#include "../include/memfunctions.h"
#include "../include/memallocs.h"
#include "../include/memfrees.h"
#include "../include/memcalculate.h"
#include "../include/paginacion.h"



void escribir_alloc(void* buffer, uint32_t prev, uint32_t next, uint8_t isFree){

	HeapMetadata alloc;

	int desplazamiento=0;
	uint32_t dir_fisica;
	alloc.prevAlloc=prev;
	alloc.nextAlloc=next;
	alloc.isFree=isFree;
	memcpy(buffer+desplazamiento, &alloc.prevAlloc, sizeof(uint32_t));
	desplazamiento+=sizeof(uint32_t);
	memcpy(buffer+desplazamiento, &alloc.nextAlloc, sizeof(uint32_t));
	desplazamiento+=sizeof(uint32_t);
	memcpy(buffer+desplazamiento, &alloc.isFree, sizeof(uint8_t));
	desplazamiento+=sizeof(uint8_t);

	log_debug(logger, "Se escribio el MetaData! prev: %d next: %d", prev, next);
}




uint32_t first_fit(int size, page_table* tabla){

	uint32_t is_free=0, tam_alloc=0, current_alloc=0;
	uint32_t next_alloc=0, prev_alloc=1;
	uint32_t desplazamiento = 0;
	uint32_t pos_bloque=0;

	while(next_alloc!=UINT32_MAX && !(is_free && tam_alloc>size)){

		pos_bloque=next_alloc;
		leer_memoria(pos_bloque+sizeof(uint32_t), sizeof(uint32_t), &next_alloc, tabla);
		log_info(logger, "Valor del next_alloc: %d", next_alloc);

		if(next_alloc!=UINT32_MAX){

			leer_memoria(pos_bloque+2*sizeof(uint32_t), sizeof(uint8_t), &is_free, tabla);
			log_info(logger, "Valor del isfree: %d", is_free);
			if(is_free){
				leer_memoria(next_alloc, sizeof(uint32_t), &prev_alloc, tabla);
				log_info(logger, "Valor del proximo prev_alloc: %d", prev_alloc);
				tam_alloc=next_alloc-prev_alloc;
				log_info(logger, "HAY UN TAMAÑO DE %d ENTRE DOS BLOQUES", tam_alloc);
			}
		}
	}

	return pos_bloque;
}

uint32_t first_fit_alternative(int size, page_table* tabla, HeapMetadata* alloc){
	uint32_t tam_alloc=0, current_alloc=0, next_alloc;
	alloc->nextAlloc=0,
	alloc->prevAlloc=1;
	alloc->isFree=0;

	uint32_t desplazamiento = 0;
	uint32_t pos_bloque=0;

	while(alloc->nextAlloc!=UINT32_MAX && !(alloc->isFree && tam_alloc>size)){

		pos_bloque=alloc->nextAlloc;
		recover_heap_data_memoria(pos_bloque, alloc, tabla);
		log_info(logger, "Valor del next_alloc: %d", alloc->nextAlloc);

		if(alloc->nextAlloc!=UINT32_MAX){

			log_info(logger, "Valor del isfree: %d", alloc->isFree);
			if(alloc->isFree){
				recover_heap_data_memoria(alloc->nextAlloc, alloc, tabla);
				log_info(logger, "Valor del proximo prev_alloc: %d", alloc->prevAlloc);
				tam_alloc=pos_bloque-alloc->prevAlloc;
				log_info(logger, "HAY UN TAMAÑO DE %d ENTRE DOS BLOQUES", tam_alloc);
			}
		}
	}

	return pos_bloque;
}


uint32_t new_first_fit(int size, page_table* tabla, HeapMetadata* alloc){

	HeapMetadata siguiente_alloc;
	uint32_t tam_alloc=0, current_alloc=0, next_alloc;
	alloc->nextAlloc=0,
	alloc->prevAlloc=1;
	alloc->isFree=0;

	uint32_t dir_logica;
	uint32_t pos_bloque_fisico;
	uint32_t pos_bloque_logico;
	uint32_t desplazamiento;

	int nro_pagina;
	void *bloque=malloc(PAGE_SIZE);
	void *metaData=malloc(TAM_METADATA);

	while(alloc->nextAlloc!=UINT32_MAX && !(alloc->isFree && tam_alloc>size)){

		dir_logica=alloc->nextAlloc;

		nro_pagina=calcular_pagina(dir_logica);
		pagina* page=obtener_pagina(nro_pagina, tabla);

		if(!page){
			log_error(logger, "FALLO EL NEW FIRST FIT");
			return UINT32_MAX;
		}
		pos_bloque_logico=nro_pagina*PAGE_SIZE;
		pos_bloque_fisico=calcular_dirFisica(page, pos_bloque_logico, tabla);
		log_info(logger, "SE VA A LEER EL BLOQUE %d", page->numero_marco);
		memcpy(bloque, memoria+pos_bloque_fisico, PAGE_SIZE);


		do{
			dir_logica=alloc->nextAlloc;
			desplazamiento=calcular_desplazamiento(dir_logica);
			log_debug(logger, "DESPLAZAMIENTO: %d", desplazamiento);

			int bytes_leidos=obtenerMetaDataBloque(TAM_METADATA, desplazamiento, &metaData, bloque);

			if(bytes_leidos<TAM_METADATA){
				int bytes_por_leer=TAM_METADATA-bytes_leidos;
				desplazamiento+=bytes_leidos;
				desbloquear_pagina_en_uso(page);
				leer_memoria(pos_bloque_logico+desplazamiento, bytes_por_leer, metaData+bytes_leidos, tabla);
			}
			recover_heap_data(alloc, metaData);

			log_debug(logger, "PrevAlloc: %d NextAlloc: %d: isFree: %d", alloc->prevAlloc, alloc->nextAlloc, alloc->isFree);
			if(alloc->nextAlloc!=UINT32_MAX){

				log_debug(logger, "Valor del isfree: %d", alloc->isFree);
				if(alloc->isFree){

					recover_heap_data_memoria(alloc->nextAlloc, &siguiente_alloc, tabla);
					log_debug(logger, "Valor del proximo prev_alloc: %d", alloc->prevAlloc);
					tam_alloc=dir_logica-alloc->prevAlloc;
					log_debug(logger, "HAY UN TAMAÑO DE %d ENTRE DOS BLOQUES", tam_alloc);
					}
			}

			log_debug(logger, "NUMERO DE PAGINA: %d", calcular_pagina(dir_logica));

		}while(calcular_pagina(alloc->nextAlloc)==nro_pagina && alloc->nextAlloc!=UINT32_MAX && !(alloc->isFree && tam_alloc>size));

		desbloquear_pagina_en_uso(page);

	}
	free(bloque);
	free(metaData);

	return dir_logica;
}

int obtenerMetaDataBloque(int bytes_por_leer, int desplazamiento, void** buffer, void* bloque){
	int bytes_leidos=0;
	int espacio_en_bloque=PAGE_SIZE-desplazamiento;

	while(bytes_leidos<bytes_por_leer && espacio_en_bloque>0){
		memcpy(*buffer+bytes_leidos, bloque+desplazamiento+bytes_leidos, 1);
		bytes_leidos++;
		espacio_en_bloque--;
	}

	log_debug(logger, "BYTES LEIDOS: %d", bytes_leidos);

	return bytes_leidos;
}


void escribir_alloc_en_memoria(uint32_t pos, uint32_t prev, uint32_t next, uint8_t isFree, page_table* tabla){
	void* alloc = malloc(TAM_METADATA);

	escribir_alloc(alloc, prev, next, isFree);
	log_debug(logger, "PID: %d - DIRECCION DEL NEW ALLOC %d", tabla->mate_id, pos);
	escribir_memoria(pos, TAM_METADATA, alloc, tabla);

	free(alloc);
}

void recover_heap_data(HeapMetadata* alloc, void* p){

	int desplazamiento=0;

    memcpy(&alloc->prevAlloc, p, sizeof(uint32_t));
    desplazamiento+=sizeof(uint32_t);

    memcpy(&alloc->nextAlloc, p+desplazamiento, sizeof(uint32_t));
    desplazamiento+=sizeof(uint32_t);

    memcpy(&alloc->isFree,p+desplazamiento, sizeof(uint8_t));
    desplazamiento+=sizeof(uint8_t);
}

void recover_heap_data_memoria(uint32_t pos, HeapMetadata* alloc, page_table* tabla){

	void* buffer=malloc(TAM_METADATA);

	leer_memoria(pos, TAM_METADATA, buffer, tabla);
	recover_heap_data(alloc, buffer);

	free(buffer);
}
