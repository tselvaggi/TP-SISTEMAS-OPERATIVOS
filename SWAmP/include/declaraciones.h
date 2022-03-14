/*
 * declaraciones.h
 *
 *  Created on: 21 sep. 2021
 *      Author: utnso
 */

#ifndef SWAMP_INCLUDE_DECLARACIONES_H_
#define SWAMP_INCLUDE_DECLARACIONES_H_

#include "../../shared/include/estructuras.h"


typedef struct mate_file{
    FILE* file;
    t_list* bloques;
    int nro_arch;
}mate_file;

typedef struct bloque_archivo{
    uint32_t mate_id;
    uint32_t numero_pagina;
    uint32_t posicion;
}bloque_archivo;

typedef struct datos_config{
	char* ip;
	char* puerto;
	int tamanio_swap;
	int tamanio_pagina;
	int cantidad_archivos;
	char** archivos_swap;
	int marcos_por_carpincho;
	int retardo_swap;
}datos_config;

typedef struct arch_bloq{
	int n_arch;
	int n_bloq;
}arch_bloq;

#endif /* SWAMP_INCLUDE_DECLARACIONES_H_ */
