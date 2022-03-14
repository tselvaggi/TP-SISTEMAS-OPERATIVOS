/*
 * memfrees.h
 *
 *  Created on: 23 nov. 2021
 *      Author: utnso
 */

#ifndef MEMORIA_INCLUDE_MEMFREES_H_
#define MEMORIA_INCLUDE_MEMFREES_H_

#include "main.h"

void consolidar_bloques(HeapMetadata alloc_medio, page_table* tabla);
void analizar_consolidado_bloques(HeapMetadata free_alloc, page_table* tabla);
bool analizar_direccion(uint32_t direccion, page_table* tabla);
void liberar_pagina(uint32_t numero_pagina, page_table* tabla);


#endif /* MEMORIA_INCLUDE_MEMFREES_H_ */
