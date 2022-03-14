#ifndef MEMORIA_INCLUDE_MEMALLOCS_H_
#define MEMORIA_INCLUDE_MEMALLOCS_H_

#include "main.h"

void recover_heap_data_memoria(uint32_t pos, HeapMetadata* alloc, page_table* tabla);
void recover_heap_data(HeapMetadata* alloc, void* p);

uint32_t first_fit(int size, page_table* tabla);
uint32_t first_fit_alternative(int size, page_table* tabla, HeapMetadata* alloc);
uint32_t new_first_fit(int size, page_table* tabla, HeapMetadata* alloc);
int obtenerMetaDataBloque(int bytes_por_leer, int desplazamiento, void** buffer, void* bloque);

void escribir_alloc_en_memoria(uint32_t pos, uint32_t prev, uint32_t next, uint8_t free, page_table* tabla);
void escribir_alloc(void* buffer, uint32_t prev, uint32_t next, uint8_t free);


#endif /* MEMORIA_INCLUDE_MEMALLOCS_H_ */
