#include <semaphore.h>
#include <commons/memory.h>
#include "include/main.h"
#include "include/memfunctions.h"
#include "include/memfrees.h"
#include "include/paginacion.h"
#include "include/protocolo_memoria.h"
#include "include/memsignals.h"
#include"../shared/include/loggers_config.h"
#include "../shared/include/servidor.h"
#include "../shared/include/cliente.h"

int pid=0;

int conexion_swamp;

t_log *logger;
t_config *config;

pthread_mutex_t tabla_de_paginas_mutex;
pthread_mutex_t hilo_mutex;
pthread_mutex_t pid_mutex;
pthread_mutex_t creacion_hilos_mutex;


int main(int argc, char *argv[]) {

	logger = iniciar_logger("memoria", "MEMORIA");

	inicializar_mem_config(&mem_config,argv[1]);
	pthread_mutex_init(&tabla_de_paginas_mutex,NULL);
	pthread_mutex_init(&hilo_mutex, NULL);
	pthread_mutex_init(&pid_mutex, NULL);
	pthread_mutex_init(&creacion_hilos_mutex, NULL);
	iniciar_memoria();

	signal(SIGINT, cerrarPrograma);
	signal(SIGUSR1, dump);
	signal(SIGUSR2, limpiarTLB);

	conexion_swamp=crear_conexion(mem_config.ip, mem_config.puerto_swamp);

	send_tipo_asignacion(TIPO_ASIGNACION, ASIGNACION, conexion_swamp);
	wait_ok(conexion_swamp);

	int server_fd = iniciar_servidor(mem_config.ip, mem_config.puerto);

	while(1){
		pthread_t hilo_cliente;
		log_info(logger, "Servidor listo para recibir un nuevo cliente");
		int cliente_fd = esperar_cliente(server_fd);
		pthread_mutex_lock(&creacion_hilos_mutex);
		void* conexion=malloc(sizeof(int));
		memcpy(conexion, &cliente_fd, sizeof(int));
		pthread_create(&hilo_cliente, NULL, &atender_cliente, conexion);
		pthread_detach(hilo_cliente);
		pthread_mutex_unlock(&creacion_hilos_mutex);
	}
	return EXIT_SUCCESS;
}

void* atender_cliente(void* cliente_fd){

	int cliente_cast = *((int*)cliente_fd);
	free(cliente_fd);
	page_table* tabla_paginas=NULL;
	int result;
	void* buffer;
	HeapMetadata alloc;
	uint32_t mate_id;


	while(1){
		int cod_op = recibir_operacion(cliente_cast);
		int size_alloc;
		uint32_t addr;
		mem_petition petition;

		switch (cod_op){
		case INIT:

			recv_init_memoria(&mate_id, cliente_cast);

			if(!mate_id){
				mate_wait(&pid_mutex);
				pid++;
				mate_id=pid;
				mate_signal(&pid_mutex);
			}

			log_info(logger, "SE CONECTO EL CARPINCHO %d", mate_id);

			tabla_paginas=malloc(sizeof(page_table));
			iniciar_tabla_paginas(mate_id, tabla_paginas);

			send_exec_memoria(mate_id, cliente_cast);
			break;
		case SEM_INIT:
		case SEM_WAIT:
		case SEM_POST:
		case SEM_DESTROY:
		case CALL_IO:
			vaciar_buffer(cliente_cast);
			send_ok(cliente_cast);
			log_warning(logger, "ERROR - ESTA OPERACION PERTENECE AL KERNEL");
			break;
		case MEMALLOC:

			recv_mate_memalloc(&petition, cliente_cast);


			log_info(logger, "CARPINCHO: %d - DEBERIA ALLOCAR UN SIZE %d", tabla_paginas->mate_id, petition.size);

			addr = memalloc(petition.size, tabla_paginas);

			if(addr==UINT32_MAX){
				log_warning(logger, "CARPINCHO: %d - NO HAY ESPACIO EN MEMORIA", tabla_paginas->mate_id);
			}

			send_mate_mem_alloced(addr, cliente_cast);

			break;
		case MEMFREE:
			recv_mate_mem_addr(&petition, cliente_cast);

			log_info(logger, "CARPINCHO: %d - DEBERIA LIBERAR ESTA POSICION EN MEMORIA %d", tabla_paginas->mate_id, petition.dir_logica);

			result=memfree(petition.dir_logica, tabla_paginas);

			if(analizar_mate_error(MEMORY_FAIL, result)){

				log_info(logger, "CARPINCHO: %d - SALIO TODO BIEN EN MEMFREE", tabla_paginas->mate_id);
			}
			else{
				log_error(logger, "CARPINCHO: %d - SALIO MAL EL MEMFREE", tabla_paginas->mate_id);
			}

			send_memory_confirm(petition.dir_logica, result, cliente_cast);

			break;
		case MEMREAD:

			recv_mate_memread(&petition, cliente_cast);

			log_info(logger, "CARPINCHO: %d - DEBERIA LEER LA POSICION %d DE TAMAÑO %d Y ENVIARSELA AL CARPINCHO", tabla_paginas->mate_id, petition.dir_logica, petition.size);

			buffer=malloc(petition.size);

			result = memread(petition.dir_logica, petition.size, buffer, tabla_paginas);

			if(analizar_mate_error(MEMORY_FAIL, result)){

				send_memreaded(buffer, petition.size, cliente_cast);
				log_info(logger, "CARPINCHO: %d - SALIO TODO BIEN EN MEMREAD", tabla_paginas->mate_id);
			}
			else{
				send_memory_confirm(petition.dir_logica, result, cliente_cast);
				log_error(logger, "CARPINCHO: %d - SALIO MAL EL MEMREAD", tabla_paginas->mate_id);
			}

			free(buffer);
			break;
		case MEMWRITE:

			recv_mate_memwrite(&petition, cliente_cast);

			char* hex_string;
			hex_string=mem_hexstring(petition.origin, petition.size);
			log_info(logger, "CARPINCHO: %d - DEBERIA ESCRIBIR %s LA POSICION %d DE TAMAÑO %d", tabla_paginas->mate_id, hex_string, petition.dir_logica, petition.size);
			free(hex_string);

			result = memwrite(petition.dir_logica, petition.size, petition.origin, tabla_paginas);

			if(analizar_mate_error(MEMORY_FAIL, result)){
				log_info(logger, "CARPINCHO: %d - SALIO TODO BIEN EN MEMWRITE", tabla_paginas->mate_id);
			}
			else{
				log_error(logger, "CARPINCHO: %d - SALIO MAL EL MEMWRITE", tabla_paginas->mate_id);
			}

			send_memory_confirm(petition.dir_logica, result, cliente_cast);

			free(petition.origin);
			break;
		case MATE_SUSPEND:

			recv_suspended_memoria(&petition, cliente_cast);

			if(tabla_paginas->mate_id==petition.mate_id){
				log_info(logger, "CARPINCHO %d - RECIBI LA ORDEN DE SUSPENDER EL PROCESO", tabla_paginas->mate_id);
				suspender_proceso(tabla_paginas);
			}
			else{
				log_warning(logger, "CARPINCHO: %d - RECIBI UN ID INCORRECTO", tabla_paginas->mate_id);
			}


			break;
		case DEBUG:
			log_info(logger, "SE RECIBIO EL DEBUG");

			break;
		case -1:

			if(tabla_paginas){
				log_info(logger,"Terminando conexion. Voy a liberar el carpincho %d", tabla_paginas->mate_id);

				uint32_t paginas_totales = list_size(tabla_paginas->tabla_paginas);
				for(uint32_t i=paginas_totales;i;i--){
					liberar_pagina(i-1,tabla_paginas);
				}
				mate_wait(mutex_swap);
				send_swap_delete(tabla_paginas->mate_id,conexion_swamp);
				wait_ok(conexion_swamp);
				mate_signal(mutex_swap);
			}

			return NULL;
		default:
			log_warning(logger, "Operacion desconocida. No quieras meter la pata");
			break;
		}
	}

}
