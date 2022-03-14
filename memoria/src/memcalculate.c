#include "../include/memfunctions.h"
#include "../include/memallocs.h"
#include "../include/memfrees.h"
#include "../include/memcalculate.h"
#include "../include/paginacion.h"
#include "../../shared/include/loggers_config.h"

int paginas_a_escribir(int pos_init, int pos_fin){
	return pos_fin/mem_config.tamanio_pagina-pos_init/mem_config.tamanio_pagina + 1;
}

int calcular_pagina(int pos){
	return pos/PAGE_SIZE;
}

int calcular_desplazamiento(int pos){
	return pos%PAGE_SIZE;
}

int cantidad_paginas_en_memoria(page_table* tabla){

	bool estaPresente(pagina* page){
		return page->bit_presencia;
	}

	return list_count_satisfying(tabla->tabla_paginas, (void*)estaPresente);
}

pagina* LRU_MMU_FIJA(pagina* primero, pagina* segundo){

	if(!primero->bit_presencia){
		return segundo;
	}
	if(!segundo->bit_presencia){
		return primero;
	}

	return compare_time(primero->temporal_LRU, segundo->temporal_LRU)? segundo:primero;
}

marcos_memoria* LRU_MMU_GLOBAL(marcos_memoria* primero, marcos_memoria* segundo){
	return compare_time(primero->pagina->temporal_LRU, segundo->pagina->temporal_LRU)? segundo:primero;
}

bool esta_ocupada(marcos_memoria* entrada){
		return !entrada->isFree;
}

uint32_t calcular_dirFisica(pagina* page, uint32_t dir_logica, page_table* tabla){

	if(page){
	int desplazamiento = calcular_desplazamiento(dir_logica);
	uint32_t numero_marco = page->numero_marco;
	return numero_marco*PAGE_SIZE+desplazamiento;
	}
	else
		return 0;
}

bool direccion_valida(uint32_t direccion, page_table* tabla){

	int tamano_tabla=list_size(tabla->tabla_paginas);
	return tamano_tabla*PAGE_SIZE>direccion;

}

bool compare_time(struct timespec time1, struct timespec time2){

	if(time1.tv_sec>time2.tv_sec){
		return true;
	}
	if(time1.tv_sec<time2.tv_sec){
		return false;
	}

	return time1.tv_nsec>time2.tv_nsec;
}

ulong convertir_timespec(struct timespec tiempo){
	return tiempo.tv_sec*1.0e6 + tiempo.tv_nsec*1.0e-3;
}
