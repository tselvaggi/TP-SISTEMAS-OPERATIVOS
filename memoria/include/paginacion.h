/*
 * paginacion.h
 *
 *  Created on: 24 nov. 2021
 *      Author: utnso
 */

#ifndef MEMORIA_INCLUDE_PAGINACION_H_
#define MEMORIA_INCLUDE_PAGINACION_H_

#include "main.h"

void iniciar_tabla_paginas(uint32_t mate_id, page_table* tabla);
void iniciar_tabla_marcos();
void iniciar_TLB();

pagina* crear_pagina(uint32_t nro_pagina, uint32_t nro_marco);

pagina* obtener_pagina(uint32_t numero_pagina, page_table* tabla);

void get_page_table_page(uint32_t numero_pagina, page_table* tabla, pagina** page);

bool get_page_TLB(uint32_t process_id, uint32_t numero_pagina, pagina** page);
void insert_page_TLB(uint32_t mate_id, pagina* page);
TLB* get_entrada_TLB(uint32_t process_id, uint32_t numero_pagina);

marcos_memoria* obtener_marco_libre(page_table* tabla);
marcos_memoria* encontrar_marco_libre();

int asignar_marcos_libres(int cantidad_marcos, page_table* tabla);
void asignar_marco_a_pagina(marcos_memoria* marco, page_table* tabla);

bool tabla_marcos_llena();

marcos_memoria* liberar_marco(uint32_t numero_marco);

void swap_in(void** buffer, uint32_t mate_id, uint32_t numero_pagina);
void swap_out(void* buffer, uint32_t mate_id, pagina* page);

void extraer_bloque_en_memoria(uint32_t numero_marco, void* buffer);
void insertar_bloque_en_memoria(uint32_t numero_marco, void* buffer);

TLB* tiempo_menor_LRU_TLB(TLB* primero, TLB* segundo);
TLB* tiempo_menor_FIFO(TLB* primero, TLB* segundo);

TLB* algoritmo_reemplazo_TLB();
pagina* algoritmo_reemplazo_MMU(page_table* tabla);

pagina* clock_M_FIJO(page_table* tabla);
pagina* clock_M_DINAMICA();

void guardar_pagina_swap(uint32_t mate_id, pagina* page);

void suspender_proceso(page_table* tabla);

void bloquear_pagina_en_uso(pagina* page);
void desbloquear_pagina_en_uso(pagina* page);

void bloquear_marco(uint32_t numero_marco);
void desbloquear_marco(uint32_t numero_marco);

void mate_wait(pthread_mutex_t* mutex);
void mate_signal(pthread_mutex_t* mutex);




#endif /* MEMORIA_INCLUDE_PAGINACION_H_ */
