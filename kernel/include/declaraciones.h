/*
 * declaraciones.h
 *
 *  Created on: 21 sep. 2021
 *      Author: utnso
 */

#ifndef KERNEL_INCLUDE_DECLARACIONES_H_
#define KERNEL_INCLUDE_DECLARACIONES_H_

#include<commons/log.h>
#include<commons/config.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include "../../shared/include/librerias.h"
#include "listas_administrativas.h"

extern int pid;
extern t_log *logger;
extern t_config *config;

typedef struct kernel_cfg
{
	char* ip;
	char* puerto;
	char* puerto_memoria;
	char* algoritmo_planificacion;
	char** dispositivos_io;
	float est_inicial;
	float alfa;
	char** duraciones_io;
	int grado_multiproc;
	int grado_multiprog;
	int tiempo_deadlock;

}kernel_cfg;

typedef struct kernel_sem
{
	char* name;
	int value;
	t_list *procesos;

}kernel_sem;

typedef enum
{
	NEW,
	READY,
	EXEC,
	BLOCKED
}state;

typedef struct kernel_proc
{
	int conexion;
	int conexion_memoria;
	state estado;
	uint32_t id;
	float prev_exec;
	float prev_expec_exec;
	sem_t sem_exec;
	sem_t sem_blocked;
	struct timespec tiempo_en_ready;
	kernel_sem* recurso_tomado;
	kernel_sem* recurso_querido;
}kernel_proc;

typedef struct io_proc
{
	char * nombre;
	float duracion;
	sem_t sem_io;
}io_proc;



void inicializar_kernel_config(kernel_cfg* kernel_config,char *direccion);


#endif /* KERNEL_INCLUDE_DECLARACIONES_H_ */
