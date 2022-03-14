#include "../include/memfunctions.h"
#include "../include/memallocs.h"
#include "../include/memfrees.h"
#include "../include/memcalculate.h"
#include "../include/paginacion.h"
#include "../include/protocolo_memoria.h"
#include "../../shared/include/mensajeria.h"


void remove_page(uint32_t numero_pagina, page_table* tabla, pagina* page){

	list_remove(tabla->tabla_paginas, numero_pagina);
	free(page);
}

TLB* find_in_TLB(uint32_t process_id, uint32_t numero_pagina){

	bool es_pagina_TLB(TLB* entrada){
		return !entrada->isFree && entrada->mate_id==process_id && entrada->pagina->numero_pagina==numero_pagina;
	}

	return list_find(MEM_TLB.entrada_tlb, (void*)es_pagina_TLB);
}

void page_destroy_swap(uint32_t mate_id, uint32_t numero_pagina){
	mate_wait(mutex_swap);
	send_swap_delete_pag(mate_id, numero_pagina, conexion_swamp);
	wait_ok(conexion_swamp);
	mate_signal(mutex_swap);
}

void liberar_pagina(uint32_t numero_pagina, page_table* tabla){

	pagina* page;
	mate_wait(mutex_TLB);
	TLB* entrada=find_in_TLB(tabla->mate_id, numero_pagina);

	if(entrada){
		entrada->isFree=true;
		page=entrada->pagina;
	}else{
		get_page_table_page(numero_pagina, tabla, &page);
	}
	mate_signal(mutex_TLB);

	if(page->bit_presencia){
		mate_wait(mutex_tabla_marcos);
		page->bit_modificado=false;
		liberar_marco(page->numero_marco);
		mate_signal(mutex_tabla_marcos);

	}

	//log_info(logger, "SE VA A LIBERAR LA PAGINA %d", numero_pagina);

	remove_page(numero_pagina, tabla, page);

}

void consolidar_bloques(HeapMetadata alloc_medio, page_table* tabla){

	if(alloc_medio.nextAlloc != UINT32_MAX){
	escribir_memoria(alloc_medio.nextAlloc, sizeof(uint32_t), &alloc_medio.prevAlloc, tabla);
	}
	else{
		log_debug(logger, "SE VA A ANALIZAR SI ES POSIBLE LIBERAR PAGINAS AL FINAL");
		uint32_t pagina_final = calcular_pagina(alloc_medio.prevAlloc+TAM_METADATA);
		uint32_t paginas_totales = list_size(tabla->tabla_paginas);

		for(uint32_t i=paginas_totales-1; i>pagina_final;i--){
			liberar_pagina(i, tabla);
			page_destroy_swap(tabla->mate_id, i);
		}
	}
	escribir_memoria(alloc_medio.prevAlloc+sizeof(uint32_t), sizeof(uint32_t), &alloc_medio.nextAlloc, tabla);


	log_debug(logger, "SE CONSOLIDO EL BLOQUE %d CON EL %d", alloc_medio.prevAlloc, alloc_medio.nextAlloc);
}

void analizar_consolidado_bloques(HeapMetadata free_alloc, page_table* tabla){

	HeapMetadata  primer_alloc, alloc_medio, alloc;

	if(free_alloc.prevAlloc != UINT32_MAX){
		recover_heap_data_memoria(free_alloc.prevAlloc, &alloc, tabla);
		if(alloc.isFree){
			primer_alloc=alloc;
			alloc_medio=free_alloc;
			consolidar_bloques(alloc_medio, tabla);
		}
	}

	if(free_alloc.nextAlloc != UINT32_MAX){
		primer_alloc=free_alloc;
		recover_heap_data_memoria(primer_alloc.nextAlloc, &alloc_medio, tabla);
		if(alloc_medio.isFree){
			consolidar_bloques(alloc_medio, tabla);
		}
	}

}

bool analizar_direccion(uint32_t direccion, page_table* tabla){

	if(!direccion_valida(direccion, tabla)){
		return false;
	}
	HeapMetadata alloc;
	uint32_t pos_bloque=0;
	do{
		recover_heap_data_memoria(pos_bloque, &alloc, tabla);
		if(pos_bloque==direccion){

			return true;
		}
		pos_bloque=alloc.nextAlloc;

	}while(pos_bloque!=UINT32_MAX);

	return false;
}
