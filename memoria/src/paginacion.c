#include "../include/memfunctions.h"
#include "../include/memallocs.h"
#include "../include/memfrees.h"
#include "../include/memcalculate.h"
#include "../include/paginacion.h"
#include "../include/protocolo_memoria.h"
#include "../include/memsignals.h"

void iniciar_tabla_paginas(uint32_t mate_id, page_table* tabla){


	tabla->tabla_paginas = list_create();
	tabla->mate_id=mate_id;
	tabla->puntero_clock_m=0;
	tabla->tlb_hit=0;
	tabla->tlb_miss=0;
	mate_wait(mutex_lista_tabla_paginas);
	list_add(lista_tabla_paginas, tabla);
	mate_signal(mutex_lista_tabla_paginas);

	log_info(logger, "Se inicio la tabla de paginas del proceso %d!", tabla->mate_id);

}

pagina* crear_pagina(uint32_t nro_pagina, uint32_t nro_marco){

	pagina* page = malloc(sizeof(pagina));
	page->numero_marco = nro_marco;
	page->numero_pagina = nro_pagina;
	page->bit_presencia=0;
	page->bit_uso=0;
	page->bit_modificado=0;

	return page;
}


void iniciar_tabla_marcos(){
	tabla_marcos.marcos = list_create();
	tabla_marcos.puntero_clock_m=0;
	int cantidad_marcos=MEM_SIZE/PAGE_SIZE;
	for(int i=0; i<cantidad_marcos; i++){
		marcos_memoria* marco=malloc(sizeof(marcos_memoria));
		marco->mate_id=UINT32_MAX;
		marco->numero_marco=i;
		marco->isFree=true;
		marco->mutex=malloc(sizeof(pthread_mutex_t));
		pthread_mutex_init(marco->mutex, NULL);
		marco->pagina=NULL;
		list_add(tabla_marcos.marcos, marco);
	}
	log_info(logger, "SE CREO LA TABLA DE %d MARCOS", cantidad_marcos);
}

void iniciar_TLB(){
	TLB_HIT=0;
	TLB_MISS=0;
	INSERT=0;
	MEM_TLB.entrada_tlb = list_create();
	int cantidad_entradas=mem_config.cantidad_entradas_tlb;
	for(int i=0; i<cantidad_entradas;i++){
		TLB* entrada=malloc(sizeof(TLB));
		entrada->isFree=true;
		entrada->mate_id=UINT32_MAX;
		entrada->pagina=NULL;

		list_add(MEM_TLB.entrada_tlb, entrada);
	}

	log_info(logger, "SE CREO LA TLB DE %d ENTRADAS", cantidad_entradas);
}

void get_page_table_page(uint32_t numero_pagina, page_table* tabla, pagina** page){

	*page = list_get(tabla->tabla_paginas, numero_pagina);

}

TLB* get_entrada_TLB(uint32_t process_id, uint32_t numero_pagina){

	bool es_pagina_TLB(TLB* entrada){
			return !entrada->isFree && entrada->mate_id==process_id && entrada->pagina->numero_pagina==numero_pagina;
	}

	return list_find(MEM_TLB.entrada_tlb, (void*)es_pagina_TLB); //RETORNA LA ENTRADA O NULL
}

bool get_page_TLB(uint32_t process_id, uint32_t numero_pagina, pagina** page){

	mate_wait(mutex_TLB);

	TLB* entrada_encontrada = get_entrada_TLB(process_id, numero_pagina);

	if(entrada_encontrada){
		usleep(mem_config.retardo_acierto_tlb);

		*page=entrada_encontrada->pagina;
		mate_signal(mutex_TLB);

		return true;
	}

	usleep(mem_config.retardo_fallo_tlb);
	mate_signal(mutex_TLB);

	return false;
}



void insert_page_TLB(uint32_t mate_id, pagina* page){

	mate_wait(mutex_TLB);

	bool encontrar_entrada_libre(TLB* entrada){
		return entrada->isFree;
	}

	TLB* entrada_encontrada=list_find(MEM_TLB.entrada_tlb, (void*)encontrar_entrada_libre);

	if(mem_config.cantidad_entradas_tlb && !entrada_encontrada){
		log_info(logger, "NO HAY ESPACIO DE ENTRADAS EN TLB, VOY A REALIZAR EL ALGORITMO DE REEMPLAZO %s", mem_config.algoritmo_reemplazo_tlb);
		entrada_encontrada = algoritmo_reemplazo_TLB();
		log_info(logger, "ENCONTRE COMO VICTIMA DE REEMPLAZO EN TLB PID: %d NUMERO PAGINA: %d NUMERO MARCO: %d", entrada_encontrada->mate_id, entrada_encontrada->pagina->numero_pagina,
							entrada_encontrada->pagina->numero_marco);
	}
	else if(!mem_config.cantidad_entradas_tlb){
		log_debug(logger, "NO HAY ENTRADAS EN TLB");
		mate_signal(mutex_TLB);
		return;
	}

	entrada_encontrada->isFree=false;
	entrada_encontrada->mate_id=mate_id;
	clock_gettime(CLOCK_MONOTONIC, &entrada_encontrada->temporal_FIFO);
	entrada_encontrada->pagina=page;
	log_info(logger, "NUEVA ENTRADA EN TLB PID: %d NUMERO PAGINA: %d NUMERO MARCO: %d", entrada_encontrada->mate_id, entrada_encontrada->pagina->numero_pagina,
								entrada_encontrada->pagina->numero_marco);

	mate_signal(mutex_TLB);
}

void extraer_bloque_en_memoria(uint32_t numero_marco, void* buffer){

	bloquear_marco(numero_marco);
	uint32_t dir_fisica_bloque = numero_marco*PAGE_SIZE;
	memcpy(buffer, memoria+dir_fisica_bloque, PAGE_SIZE);
	log_debug(logger, "Extraje el marco %d de memoria", numero_marco);
	desbloquear_marco(numero_marco);

}

void insertar_bloque_en_memoria(uint32_t numero_marco, void* buffer){

	bloquear_marco(numero_marco);
	uint32_t dir_fisica_bloque = numero_marco*PAGE_SIZE;
	memcpy(memoria+dir_fisica_bloque, buffer, PAGE_SIZE);
	desbloquear_marco(numero_marco);
}

void mate_wait(pthread_mutex_t* mutex){
	//log_info(logger, "SE HIZO UN WAIT!");
	pthread_mutex_lock(mutex);
}

void mate_signal(pthread_mutex_t* mutex){
	//log_info(logger, "SE HIZO UN SIGNAL!");
	pthread_mutex_unlock(mutex);
}

void bring_page_from_swap(pagina* page, page_table* tabla){

	void* buffer=malloc(PAGE_SIZE);

	log_info(logger, "SWAP IN");
	swap_in(&buffer, tabla->mate_id, page->numero_pagina);

	mate_wait(mutex_tabla_marcos);

	marcos_memoria* marco = obtener_marco_libre(tabla);

	insertar_bloque_en_memoria(marco->numero_marco, buffer);
	marco->mate_id=tabla->mate_id;
	marco->isFree=false;
	marco->pagina=page;

	mate_signal(mutex_tabla_marcos);

	page->numero_marco=marco->numero_marco;
	page->bit_presencia=true;


	free(buffer);

}

void bloquear_marco(uint32_t numero_marco){

	log_debug(logger, "EN BLOQUEAR MARCO %d ????", numero_marco);
	mate_wait(mutex_busqueda_tabla);
	marcos_memoria* marco=list_get(tabla_marcos.marcos, numero_marco);
	mate_signal(mutex_busqueda_tabla);

	log_debug(logger, "Se bloqueo el marco %d", numero_marco);
	mate_wait(marco->mutex);

}

void desbloquear_marco(uint32_t numero_marco){

	mate_wait(mutex_busqueda_tabla);
	marcos_memoria* marco=list_get(tabla_marcos.marcos, numero_marco);
	mate_signal(mutex_busqueda_tabla);

	log_debug(logger, "Se desbloqueo el marco %d", numero_marco);
	mate_signal(marco->mutex);

}

void bloquear_pagina_en_uso(pagina* page){

	page->bit_uso=true;
	clock_gettime(CLOCK_MONOTONIC, &page->temporal_LRU);

	bloquear_marco(page->numero_marco);
}

void desbloquear_pagina_en_uso(pagina* page){

	desbloquear_marco(page->numero_marco);
}

pagina* obtener_pagina(uint32_t numero_pagina, page_table* tabla){

	pagina* page;

	log_debug(logger, "NUMERO DE PAGINA: %d", numero_pagina);
	log_debug(logger, "TAMAÑO TABLA DE PAGINAS: %d", list_size(tabla->tabla_paginas));
	if(numero_pagina>=list_size(tabla->tabla_paginas)){
		log_error(logger, "PAGINA INVALIDA");
		return NULL;
	}

	if(get_page_TLB(tabla->mate_id, numero_pagina, &page)){

		TLB_HIT++;
		tabla->tlb_hit++;

		log_info(logger, "TLB Hit! PID: %d Numero pagina: %d Numero marco %d", tabla->mate_id, page->numero_pagina, page->numero_marco);
		bloquear_pagina_en_uso(page);
		return page;
	}

	//TLB MISS
	TLB_MISS++;
	tabla->tlb_miss++;
	get_page_table_page(numero_pagina, tabla, &page);

	log_info(logger, "TLB Miss! PID: %d Numero pagina: %d", tabla->mate_id, numero_pagina);

	if(!page->bit_presencia){

		log_info(logger, "PID: %d - La pagina %d no se encuentra en memoria", tabla->mate_id, page->numero_pagina);
		bring_page_from_swap(page, tabla);

		return obtener_pagina(numero_pagina, tabla);
	}

	bloquear_pagina_en_uso(page);
	insert_page_TLB(tabla->mate_id, page);

	return page;
}

marcos_memoria* obtener_marco_libre(page_table* tabla){

	marcos_memoria* marco_libre;

	//PUEDE DARSE EL CASO DE QUE NO HAYA UN MARCO LIBRE

	if(cantidad_paginas_en_memoria(tabla)==MARCOS_POR_CARPINCHO || tabla_marcos_llena()){
		//NO HAY ESPACIO EN MEMORIA O NO SE PUEDEN ASIGNAR MAS MARCOS HABLAR CON SWAP
		log_info(logger, "NO HAY MAS MARCOS EN MEMORIA DISPONIBLES PARA EL CARPINCHO %d, SE EFECTUARA ALGORITMO DE REEMPLAZO %s", tabla->mate_id, mem_config.algoritmo_reemplazo_mmu);

		if(cantidad_paginas_en_memoria(tabla)==MARCOS_POR_CARPINCHO){
			log_debug(logger, "ENTRO PORQUE SUPERO LA CANTIDAD DE MARCOS PERMITIDA EN MEMORIA");
		}

		pagina* pagina_a_reemplazar;

		pagina_a_reemplazar = algoritmo_reemplazo_MMU(tabla);

		if (pagina_a_reemplazar){

			marco_libre = liberar_marco(pagina_a_reemplazar->numero_marco);

			pagina_a_reemplazar->numero_marco=UINT32_MAX;
			pagina_a_reemplazar->bit_presencia=false;

		}
		else{
			log_error(logger, "NO SE ENCONTRO PAGINA CANDIDATA POR ALGORITMO DE REEMPLAZO");
			return NULL; //NO SE ENCONTRO UNA PAGINA PARA REEMPLAZAR
		}

	}else{
		marco_libre = encontrar_marco_libre();

		log_info(logger, "SE ENCONTRO UN MARCO LIBRE %d", marco_libre->numero_marco);

	}

	marco_libre->isFree=false;

	return marco_libre;
}

bool tabla_marcos_llena(){

	return list_all_satisfy(tabla_marcos.marcos, (void*)esta_ocupada);
}

int asignar_marcos_libres(int cantidad_marcos, page_table* tabla){

	marcos_memoria* marco_libre;
	log_info(logger, "SE ASIGNARAN MARCOS LIBRES PARA EL MEMALLOC");

	for(int i=0; i<cantidad_marcos;i++){
		mate_wait(mutex_tabla_marcos);
		marco_libre = obtener_marco_libre(tabla);

		if(!marco_libre){
			log_error(logger, "NO HAY MARCOS LIBRES");
			mate_signal(mutex_tabla_marcos);
			return 0;
		}
		log_info(logger, "PID: %d - Se asignara el marco encontrado %d a una nueva pagina", tabla->mate_id, marco_libre->numero_marco);

		asignar_marco_a_pagina(marco_libre, tabla);

		mate_signal(mutex_tabla_marcos);
	}

	return 1;
}

void guardar_pagina_swap(uint32_t mate_id, pagina* page){


	void* buffer=malloc(PAGE_SIZE);

	extraer_bloque_en_memoria(page->numero_marco, buffer);
	log_debug(logger, "VOY A PEDIR UN SWAP OUT DEL PROCESO %d, PAGINA %d, MARCO %d", mate_id, page->numero_pagina, page->numero_marco);
	swap_out(buffer, mate_id, page);

	free(buffer);


}

void asignar_marco_a_pagina(marcos_memoria* marco, page_table* tabla){

	pagina* page;
	marco->isFree=false;
	marco->mate_id = tabla->mate_id;
	int paginas_totales;

	paginas_totales = list_size(tabla->tabla_paginas);

	page = crear_pagina(paginas_totales, marco->numero_marco);


	page->bit_presencia=1;
	page->bit_uso=1;
	clock_gettime(CLOCK_MONOTONIC, &page->temporal_LRU);
	marco->pagina=page;

	log_info(logger, "VOY A GUARDAR LA PAGINA %d DEL PROCESO %d EN SWAP", page->numero_pagina, tabla->mate_id);
	guardar_pagina_swap(tabla->mate_id, page);

	log_info(logger, "VOY A INSERTAR LA PAGINA %d DEL PROCESO %d EN LA TLB", page->numero_pagina, tabla->mate_id);
	insert_page_TLB(tabla->mate_id, page);
	log_info(logger, "VOY A INSERTAR LA PAGINA %d EN LA TABLA DE PAGINAS", page->numero_pagina);
	list_add(tabla->tabla_paginas, page);
	log_info(logger, "Se agrego a la tabla de paginas la pagina %d, marco %d del carpincho %d", page->numero_pagina, page->numero_marco, tabla->mate_id);
}

marcos_memoria* encontrar_marco_libre(){

	bool esta_libre(marcos_memoria* marco){
			return marco->isFree;
	}

	return list_find(tabla_marcos.marcos, (void*)esta_libre);
}

void swap_in(void** buffer, uint32_t mate_id, uint32_t numero_pagina){

	mate_wait(mutex_swap);
	send_swap_in_petition(mate_id, numero_pagina, conexion_swamp);
	log_info(logger, "ENVIE LA PETICION DE SWAP IN");
	recv_swap_in(buffer, conexion_swamp);

	mate_signal(mutex_swap);
}

void swap_out(void* buffer, uint32_t mate_id, pagina* page){

	mate_wait(mutex_swap);
	send_swap_out(buffer, PAGE_SIZE, mate_id, page->numero_pagina, conexion_swamp);
	int confirm = wait_ok(conexion_swamp);
	if(confirm!=CONTINUE){
		log_error(logger, "NO HAY MAS ESPACIO EN SWAP: %d", confirm);
	}
	page->bit_modificado=false;
	mate_signal(mutex_swap);
}

marcos_memoria* liberar_marco(uint32_t numero_marco){

	log_debug(logger, "Se va a buscar el marco %d para liberarlo", numero_marco);

	marcos_memoria* marco_a_liberar=list_get(tabla_marcos.marcos, numero_marco);

	log_info(logger, "SE VA A LIBERAR EL MARCO %d, QUE PERTENECIA A LA PAGINA %d DEL CARPINCHO %d", marco_a_liberar->numero_marco, marco_a_liberar->pagina->numero_pagina, marco_a_liberar->mate_id);

	if(marco_a_liberar->pagina->bit_modificado){
		guardar_pagina_swap(marco_a_liberar->mate_id, marco_a_liberar->pagina);
	}

	marco_a_liberar->pagina->bit_presencia=false;
	marco_a_liberar->isFree=true;

	mate_wait(mutex_TLB);
	TLB* entrada=get_entrada_TLB(marco_a_liberar->mate_id, marco_a_liberar->pagina->numero_pagina);
	if(entrada){
		entrada->isFree=true;
		log_info(logger, "SE LIBERO LA ENTRADA DE TLB DEL PROCESO %d PAGINA %d MARCO %d", entrada->mate_id, entrada->pagina->numero_pagina, entrada->pagina->numero_marco);
	}
	mate_signal(mutex_TLB);

	marco_a_liberar->mate_id=UINT32_MAX;
	marco_a_liberar->pagina=NULL;

	return marco_a_liberar;

}

void suspender_proceso(page_table* tabla){

	int tabla_size=list_size(tabla->tabla_paginas);



	for(int i=0; i<tabla_size; i++){
		pagina* page=list_get(tabla->tabla_paginas, i);
		if(page->bit_presencia && page->bit_modificado){


			void* buffer=malloc(PAGE_SIZE);
			extraer_bloque_en_memoria(page->numero_marco, &buffer);
			swap_out(&buffer, tabla->mate_id, page);

			free(buffer);
			page->bit_presencia=false;


		}
	}

}


TLB* tiempo_menor_LRU_TLB(TLB* primero, TLB* segundo){
	return compare_time(primero->pagina->temporal_LRU, segundo->pagina->temporal_LRU)? segundo:primero;
}

TLB* tiempo_menor_FIFO(TLB* primero, TLB* segundo){
	return compare_time(primero->temporal_FIFO, segundo->temporal_FIFO)? segundo:primero;
}

pagina* clock_M_FIJO(page_table* tabla){
	pagina* page;
	uint32_t ptr_aux=tabla->puntero_clock_m;
	int tam_tabla=list_size(tabla->tabla_paginas);
	log_info(logger, "Comienza el algoritmo de reemplazo CLOCK-M");
	while(tam_tabla){
		log_info(logger, "PASO 1");
		do{

			page=list_get(tabla->tabla_paginas, ptr_aux);
			ptr_aux++;
			ptr_aux=ptr_aux%tam_tabla;
			if(page->bit_presencia && !page->bit_uso && !page->bit_modificado){
				tabla->puntero_clock_m=ptr_aux;
				log_info(logger, "SE ENCONTRO LA PAGINA %d U: %d M: %d", page->numero_pagina, page->bit_uso, page->bit_modificado);
				return page;
			}


		}while(ptr_aux!=tabla->puntero_clock_m);

			log_info(logger, "PASO 2");
		do{
			page=list_get(tabla->tabla_paginas, ptr_aux);
			ptr_aux++;
			ptr_aux=ptr_aux%tam_tabla;
			if(page->bit_presencia && !page->bit_uso && page->bit_modificado){
				tabla->puntero_clock_m=ptr_aux;
				log_info(logger, "SE ENCONTRO LA PAGINA %d U: %d M: %d", page->numero_pagina, page->bit_uso, page->bit_modificado);
				return page;
			}
			page->bit_uso=0;

		}while(ptr_aux!=tabla->puntero_clock_m);

	}

	return NULL;
}

pagina* clock_M_DINAMICA(){
	marcos_memoria* marco;
	pagina* page;
	uint32_t ptr_aux=tabla_marcos.puntero_clock_m;
	int tam_tabla=list_size(tabla_marcos.marcos);
	log_info(logger, "Comienza el algoritmo de reemplazo CLOCK-M");
	while(tam_tabla){
		log_info(logger, "PASO 1");
		do{

			marco=list_get(tabla_marcos.marcos, ptr_aux);
			page=marco->pagina;
			ptr_aux++;
			ptr_aux=ptr_aux%tam_tabla;
			if(!page->bit_uso && !page->bit_modificado){
				tabla_marcos.puntero_clock_m=ptr_aux;
				log_info(logger, "SE ENCONTRO LA PAGINA %d U: %d M: %d", page->numero_pagina, page->bit_uso, page->bit_modificado);
				return page;
			}


		}while(ptr_aux!=tabla_marcos.puntero_clock_m);

		do{
			marco=list_get(tabla_marcos.marcos, ptr_aux);
			page=marco->pagina;
			ptr_aux++;
			ptr_aux=ptr_aux%tam_tabla;
			if(!page->bit_uso && page->bit_modificado){
				tabla_marcos.puntero_clock_m=ptr_aux;
				log_info(logger, "SE ENCONTRO LA PAGINA %d U: %d M: %d", page->numero_pagina, page->bit_uso, page->bit_modificado);
				return page;
			}
			page->bit_uso=0;

		}while(ptr_aux!=tabla_marcos.puntero_clock_m);

	}

	return NULL;
}

pagina* algoritmo_reemplazo_MMU(page_table* tabla){

	marcos_memoria* marco_a_reemplazar;
	pagina* pagina_a_reemplazar;
	log_info(logger, "SE VA A EFECTUAR EL ALGORITMO DE REEMPLAZO MMU %s", mem_config.algoritmo_reemplazo_mmu);
	switch(TIPO_ASIGNACION){
	case FIJA:

		if(!cantidad_paginas_en_memoria(tabla)){
			log_info(logger, "NO HAY PAGINAS EN MEMORIA PARA ALGORITMO DE REEMPLAZO PROCESO %d", tabla->mate_id);
			return NULL;
		}
		switch(ALGORITMO_REEMPLAZO_MMU){
		case LRU_MMU:
			pagina_a_reemplazar = list_get_minimum(tabla->tabla_paginas, (void*)LRU_MMU_FIJA);
			break;
		case CLOCK_M_MMU:
			pagina_a_reemplazar = clock_M_FIJO(tabla);
			break;
		default:
			log_error(logger, "SE ESPECIFICO MAL EL ALGORITMO DE REEMPLAZO EN LA CONFIG");
			return NULL;
			break;
		}

		return pagina_a_reemplazar;

		break;
	case DINAMICA:
		switch(ALGORITMO_REEMPLAZO_MMU){
		case LRU_MMU:
			marco_a_reemplazar = list_get_minimum(tabla_marcos.marcos, (void*)LRU_MMU_GLOBAL);
			return marco_a_reemplazar->pagina;
			break;
		case CLOCK_M_MMU:
			return clock_M_DINAMICA();
			break;
		default:
			log_error(logger, "SE ESPECIFICO MAL EL ALGORITMO DE REEMPLAZO EN LA CONFIG");
			return NULL;
			break;
		}

		break;
	default:
		return NULL;
	}
}

TLB* algoritmo_reemplazo_TLB(){
	switch(ALGORITMO_REEMPLAZO_TLB){
	case LRU_TLB:
		return list_get_minimum(MEM_TLB.entrada_tlb, (void*)tiempo_menor_LRU_TLB);
		break;
	case FIFO_TLB:
		return list_get_minimum(MEM_TLB.entrada_tlb, (void*)tiempo_menor_FIFO);
		break;
	default:
		return NULL;
		break;
	}
}
