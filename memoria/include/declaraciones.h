/*
 * funciones.h
 *
 *  Created on: 21 sep. 2021
 *      Author: utnso
 */

#ifndef MEMORIA_INCLUDE_DECLARACIONES_H_
#define MEMORIA_INCLUDE_DECLARACIONES_H_

#include "../../shared/include/librerias.h"


extern t_log *logger;
extern t_config *config;

extern int conexion_swamp;

pthread_mutex_t* mutex_tabla_marcos;
pthread_mutex_t* mutex_busqueda_tabla;
pthread_mutex_t* mutex_TLB;
pthread_mutex_t* mutex_swap;
pthread_mutex_t* mutex_lista_tabla_paginas;


typedef struct mem_cfg
{
	char* ip;
	char* puerto;
	char* puerto_swamp;
	int tamanio;
	int tamanio_pagina;
	char* tipo_asignacion;
	int marcos_por_carpincho;
	char* algoritmo_reemplazo_mmu;
	int cantidad_entradas_tlb;
	char* algoritmo_reemplazo_tlb;
	int retardo_acierto_tlb;
	int retardo_fallo_tlb;
	char* path_dump_tlb;

}mem_cfg;

extern mem_cfg mem_config;

typedef struct HeapMetadata {
	uint32_t prevAlloc;
	uint32_t nextAlloc;
	uint8_t isFree;
}HeapMetadata;

typedef struct pagina{
	uint32_t numero_pagina;
	uint32_t numero_marco;
	struct timespec temporal_LRU;
	uint8_t bit_presencia;
	uint8_t bit_uso;
	uint8_t bit_modificado;
}pagina;

uint32_t TIME;

typedef struct page_table{

	uint32_t mate_id;
	uint32_t puntero_clock_m;
	uint32_t tlb_hit;
	uint32_t tlb_miss;
	t_list* tabla_paginas;

}page_table;

typedef struct TLB{
	uint8_t isFree;
	uint32_t mate_id;
	struct timespec temporal_FIFO;
	pagina* pagina;
}TLB;

typedef struct TLB_cache{
	t_list* entrada_tlb;

}TLB_cache;

TLB_cache MEM_TLB;

t_list* lista_tabla_paginas;

typedef struct marcos_memoria{
	uint8_t isFree;
	uint32_t mate_id;
	uint32_t numero_marco;
	pthread_mutex_t* mutex;
	pagina* pagina;

}marcos_memoria;

typedef struct marco_table{
	uint32_t puntero_clock_m;
	t_list* marcos;
}marco_table;

marco_table tabla_marcos;

void* memoria;

typedef enum tipo_asignacion{
	FIJA,
	DINAMICA
}tipo_asignacion;

typedef enum algoritmo_reemplazo_mmu{
	LRU_MMU,
	CLOCK_M_MMU
}algoritmo_reemplazo_mmu;

typedef enum algoritmo_reemplazo_tlb{
	LRU_TLB,
	FIFO_TLB
}algoritmo_reemplazo_tlb;

void inicializar_mem_config(mem_cfg* mem_config,char*direccion);

#endif /* MEMORIA_INCLUDE_DECLARACIONES_H_ */
