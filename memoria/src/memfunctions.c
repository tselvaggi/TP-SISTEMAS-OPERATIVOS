#include "../include/memfunctions.h"
#include "../include/memallocs.h"
#include "../include/memfrees.h"
#include "../include/memcalculate.h"
#include "../include/paginacion.h"
#include "../../shared/include/mensajeria.h"


int mem_init;


void iniciar_memoria(){

	lista_tabla_paginas=list_create();

	MEM_SIZE=mem_config.tamanio;
	PAGE_SIZE=mem_config.tamanio_pagina;

	memoria=malloc(MEM_SIZE);
	memset(memoria, 0, MEM_SIZE);

	if(strcmp("FIJA", mem_config.tipo_asignacion)==0){
		MARCOS_POR_CARPINCHO=mem_config.marcos_por_carpincho;
		TIPO_ASIGNACION=FIJA;
	}
	else{ //SI NO ES FIJA ES DINAMICA
		MARCOS_POR_CARPINCHO=MEM_SIZE/PAGE_SIZE;
		TIPO_ASIGNACION=DINAMICA;
	}

	if(strcmp("LRU", mem_config.algoritmo_reemplazo_tlb)==0){
		ALGORITMO_REEMPLAZO_TLB=LRU_TLB;
	}else{ //SI NO ES LRU ES FIFO
		ALGORITMO_REEMPLAZO_TLB=FIFO_TLB;
	}

	if(strcmp("LRU", mem_config.algoritmo_reemplazo_mmu)==0){
		ALGORITMO_REEMPLAZO_MMU=LRU_MMU;
	}else{ //SI NO ES LRU ES CLOCK-M
		ALGORITMO_REEMPLAZO_MMU=CLOCK_M_MMU;
	}
	mutex_tabla_marcos=malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(mutex_tabla_marcos, NULL);

	mutex_busqueda_tabla=malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(mutex_busqueda_tabla, NULL);

	mutex_TLB=malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(mutex_TLB, NULL);

	mutex_swap=malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(mutex_swap, NULL);

	mutex_lista_tabla_paginas=malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(mutex_lista_tabla_paginas, NULL);


	log_info(logger, "Se creo la memoria con un tamaño de %d", mem_config.tamanio);

	iniciar_tabla_marcos();
	iniciar_TLB();

}

void leer_memoria(uint32_t pos, int size, void* data, page_table* tabla){

	int numero_pagina = calcular_pagina(pos);
	pagina* page = obtener_pagina(numero_pagina, tabla);

	int start_of_read = calcular_dirFisica(page, pos, tabla);
	int end_of_read;
	int marco;

	int bytes_por_leer=size;
	int bytes_posibles_en_bloque;
	int bytes_leidos=0;



	while(bytes_por_leer > 0){

		marco = calcular_pagina(start_of_read);

		end_of_read = PAGE_SIZE * (marco+1);
		bytes_posibles_en_bloque = PAGE_SIZE-calcular_desplazamiento(start_of_read);

		if(bytes_por_leer>bytes_posibles_en_bloque){

			memcpy(data + bytes_leidos, memoria+start_of_read, bytes_posibles_en_bloque);
			desbloquear_pagina_en_uso(page);
			bytes_leidos+=bytes_posibles_en_bloque;
			bytes_por_leer-=bytes_posibles_en_bloque;

			numero_pagina = calcular_pagina(pos+bytes_leidos);
			page = obtener_pagina(numero_pagina, tabla);
			start_of_read = calcular_dirFisica(page, pos+bytes_leidos, tabla);

		}
		else{

			memcpy(data + bytes_leidos, memoria+start_of_read, bytes_por_leer);
			desbloquear_pagina_en_uso(page);

			bytes_leidos+=bytes_por_leer;
			bytes_por_leer-=bytes_por_leer;

		}
	}
}

void escribir_memoria(uint32_t pos, int size, void* data, page_table* tabla){

	int numero_pagina = calcular_pagina(pos);
	pagina* page = obtener_pagina(numero_pagina, tabla);

	int start_of_write = calcular_dirFisica(page, pos, tabla);
	int end_of_write;
	int marco;

	int bytes_por_escribir=size;
	int bytes_posibles_en_bloque;
	int bytes_escritos=0;



	while(bytes_por_escribir > 0){

		marco = calcular_pagina(start_of_write);
		end_of_write = PAGE_SIZE * (marco+1);
		bytes_posibles_en_bloque = PAGE_SIZE-calcular_desplazamiento(start_of_write);
		log_debug(logger, "BYTES POR ESCRIBIR %d", bytes_por_escribir);
		log_debug(logger, "BYTES POSIBLES EN BLOQUE %d", bytes_posibles_en_bloque);
		if(bytes_por_escribir>bytes_posibles_en_bloque){


			memcpy(memoria+start_of_write, data+bytes_escritos, bytes_posibles_en_bloque);
			desbloquear_pagina_en_uso(page);
			page->bit_modificado=true;

			bytes_escritos+=bytes_posibles_en_bloque;
			bytes_por_escribir-=bytes_posibles_en_bloque;

			numero_pagina = calcular_pagina(pos+bytes_escritos);
			page = obtener_pagina(numero_pagina, tabla);

			start_of_write = calcular_dirFisica(page, pos+bytes_escritos, tabla);


		}
		else{

			memcpy(memoria+start_of_write, data+bytes_escritos, bytes_por_escribir);
			desbloquear_pagina_en_uso(page);
			page->bit_modificado=true;

			bytes_escritos+=bytes_por_escribir;
			bytes_por_escribir-=bytes_por_escribir;

		}
	}
}


uint32_t memalloc(int size, page_table* tabla){

	log_debug(logger, "SIZE = %d", size);
	HeapMetadata current_alloc, new_alloc, end_alloc;
	uint32_t dir_a_escribir=0;

	int tam_tabla=list_size(tabla->tabla_paginas);

	log_debug(logger,"Tamaño de la tabla de paginas: %d", tam_tabla);

	if(list_size(tabla->tabla_paginas)==0){

		int cantidad_marcos_necesarios = (size+2*TAM_METADATA-1)/PAGE_SIZE + 1;
		log_info(logger, "NECESITO PEDIR %d MARCOS", cantidad_marcos_necesarios);
		if(asignar_marcos_libres(cantidad_marcos_necesarios, tabla)){
			escribir_alloc_en_memoria(dir_a_escribir, UINT32_MAX, size+TAM_METADATA, false, tabla);
			escribir_alloc_en_memoria(size+TAM_METADATA, 0, UINT32_MAX, true, tabla);
		}
		else{
			log_error(logger, "NO HAY ESPACIO PARA EL ALLOC PEDIDO");
			return UINT32_MAX;
		}
	}
	else{

		//dir_a_escribir = first_fit(size, tabla);
		//dir_a_escribir = first_fit_alternative(size, tabla, &current_alloc);
		dir_a_escribir = new_first_fit(size, tabla, &current_alloc);

		if(current_alloc.nextAlloc!=UINT32_MAX){
			int espacio_disponible = current_alloc.nextAlloc - (dir_a_escribir+TAM_METADATA);

			if(espacio_disponible > size+TAM_METADATA){
				uint32_t dir_new_alloc=dir_a_escribir+size+TAM_METADATA;
				escribir_alloc_en_memoria(dir_new_alloc, dir_a_escribir, current_alloc.nextAlloc, true, tabla);
				escribir_alloc_en_memoria(dir_a_escribir, current_alloc.prevAlloc, dir_new_alloc, false, tabla);
				escribir_memoria(current_alloc.nextAlloc, sizeof(uint32_t), &dir_new_alloc, tabla);
			}
			else{
				escribir_alloc_en_memoria(dir_a_escribir, current_alloc.prevAlloc, current_alloc.nextAlloc, false, tabla);
			}
		}
		else{
			log_debug(logger, "DIRECCION A ESCRIBIR: %d", dir_a_escribir);
			int desplazamiento = calcular_desplazamiento(dir_a_escribir+TAM_METADATA);
			int espacio_en_marco=0;
			if(desplazamiento){
				espacio_en_marco = PAGE_SIZE - calcular_desplazamiento(dir_a_escribir+TAM_METADATA);
			}
			log_debug(logger, "EL ESPACIO EN EL ULTIMO MARCO ES DE %d, EL ESPACIO NECESARIO ES DE %d", espacio_en_marco, size+TAM_METADATA);
			int cantidad_marcos_necesarios = 0;
			if(size+TAM_METADATA>espacio_en_marco){
				cantidad_marcos_necesarios = (size-espacio_en_marco+TAM_METADATA-1)/PAGE_SIZE + 1;
			}
			uint32_t dir_new_alloc=dir_a_escribir+size+TAM_METADATA;

			if(asignar_marcos_libres(cantidad_marcos_necesarios, tabla)){
				escribir_alloc_en_memoria(dir_a_escribir, current_alloc.prevAlloc, dir_new_alloc, false, tabla);
				log_debug(logger, "DIRECCION DEL NEW ALLOC %d", dir_new_alloc);
				escribir_alloc_en_memoria(dir_new_alloc, dir_a_escribir, UINT32_MAX, true, tabla);
			}
			else{
				log_error(logger, "NO HAY ESPACIO PARA EL ALLOC PEDIDO");
				return UINT32_MAX;
			}
		}
	}

	return dir_a_escribir+TAM_METADATA;
}




int memfree(uint32_t direccion, page_table* tabla){


	HeapMetadata free_alloc;
	uint32_t pos = direccion-TAM_METADATA;

	if(direccion_valida(pos, tabla)){
		recover_heap_data_memoria(pos, &free_alloc, tabla);
		free_alloc.isFree=true;
		escribir_memoria(pos+2*sizeof(uint32_t), sizeof(uint8_t), &free_alloc.isFree, tabla);
		analizar_consolidado_bloques(free_alloc, tabla);
	}
	else{
		return MEMORY_FAIL;
	}


	return MEMORY_OK;
}


int memread(uint32_t direccion, int size, void* buffer, page_table* tabla){

	uint32_t dir_final=direccion+size;

	if(size>0 && direccion_valida(direccion, tabla) && direccion_valida(dir_final, tabla)){
		leer_memoria(direccion, size, buffer, tabla);
		return MEMORY_OK;
	}
	else{
		return MEMORY_FAIL;
	}
}

int memwrite(uint32_t direccion, int size, void* buffer, page_table* tabla){

	uint32_t dir_final=direccion+size;

	if(size>0 && direccion_valida(direccion, tabla) && direccion_valida(dir_final, tabla)){
		escribir_memoria(direccion, size, buffer, tabla);
		return MEMORY_OK;
	}
	else{
		return MEMORY_FAIL;
	}
}

bool analizar_mate_error(int mate_error, int result){
	return result != mate_error;

}

