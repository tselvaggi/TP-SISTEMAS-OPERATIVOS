#include "../include/matelib.h"
#include "../include/declaraciones.h"
#include "../include/protocolos_lib.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <semaphore.h>
#include <commons/memory.h>
#include "../../shared/include/cliente.h"




//------------------General Functions---------------------/


int mate_init(mate_instance *carpincho, char *config_path)
{

	int conexion;
	carpincho->group_info=malloc(sizeof(mate_inner_structure));
	inicializar_carpincho(CARPINCHO);
	t_config* init_config;
	t_log* init_log;

	init_log = iniciar_logger("carpinchos", "CARPINCHO");
	CARPINCHO->mate_logger=init_log;
	init_config=iniciar_config(config_path);
	CARPINCHO->mate_config=init_config;
	log_info(CARPINCHO->mate_logger, "Creando carpincho!");

	inicializar_mate_config(&(CARPINCHO->structure_cfg), CARPINCHO->mate_config);

	log_info(CARPINCHO->mate_logger, "DESTINO: %s", CARPINCHO->structure_cfg.destino);

	if(strcmp(CARPINCHO->structure_cfg.destino, "memoria") == 0){
		log_info(CARPINCHO->mate_logger, "Lei la IP %s, el puerto %s de %s", CARPINCHO->structure_cfg.ip, CARPINCHO->structure_cfg.puerto_memoria, CARPINCHO->structure_cfg.destino);
		conexion = crear_conexion(CARPINCHO->structure_cfg.ip, CARPINCHO->structure_cfg.puerto_memoria);
		log_info(CARPINCHO->mate_logger, "Me conecte a %s: %d\n", CARPINCHO->structure_cfg.destino, conexion);
	}
	else if (strcmp(CARPINCHO->structure_cfg.destino, "kernel") == 0){
		log_info(CARPINCHO->mate_logger, "Lei la IP %s, el puerto %s de %s", CARPINCHO->structure_cfg.ip, CARPINCHO->structure_cfg.puerto_kernel, CARPINCHO->structure_cfg.destino);
		conexion = crear_conexion(CARPINCHO->structure_cfg.ip, CARPINCHO->structure_cfg.puerto_kernel);
		log_info(CARPINCHO->mate_logger, "Me conecte a %s: %d\n", CARPINCHO->structure_cfg.destino, conexion);
	}
	else{
		log_info(CARPINCHO->mate_logger, "No pude comprender el destinatario de conexion.");
		exit(1);
	}

	CARPINCHO->socket=conexion;
	CARPINCHO->mate_id=0;
  	send_init(carpincho, CARPINCHO->socket);

  	log_info(CARPINCHO->mate_logger, "Se bloqueo el carpincho esperando el exec.");

  	recv_exec(carpincho, CARPINCHO->socket);

  	log_info(CARPINCHO->mate_logger, "Se le asigno el ID: %d", CARPINCHO->mate_id);
  	log_info(CARPINCHO->mate_logger, "Se desbloqueo el carpincho, ahora puede seguir ejecutando.");

  	return 0;
}

int mate_close(mate_instance *carpincho)
{

	log_info(CARPINCHO->mate_logger, "VOY A LIBERAR LA CONEXION, CARPINCHO: %d", CARPINCHO->mate_id);
	liberar_conexion(CARPINCHO->socket);


	if(CARPINCHO->mate_logger != NULL){
  		log_destroy(CARPINCHO->mate_logger);
	}

	if(CARPINCHO->mate_config != NULL){
    		config_destroy(CARPINCHO->mate_config);
	}

	free(carpincho->group_info);

  return 0;
}

//-----------------Semaphore Functions---------------------/

int mate_sem_init(mate_instance *carpincho, mate_sem_name sem, unsigned int value) {

	log_info(CARPINCHO->mate_logger, "Voy a enviar un SEM_INIT con nombre %s y valor inicial %d", sem, value);
	send_sem_init(carpincho, sem, value, CARPINCHO->socket);
	wait_ok(CARPINCHO->socket);


	return 0;
}

int mate_sem_wait(mate_instance *carpincho, mate_sem_name sem) {
	//SIN VALIDACIONES
	log_info(CARPINCHO->mate_logger, "Voy a enviar un SEM_WAIT al semaforo %s", sem);
	send_sem_operation(carpincho, sem, SEM_WAIT, CARPINCHO->socket);

	if(wait_ok(CARPINCHO->socket)){
		return 0;
	}
	else{
		liberar_conexion(CARPINCHO->socket);
	}

	return 0;
 
}

int mate_sem_post(mate_instance *carpincho, mate_sem_name sem) {
	//SIN VALIDACIONES
	log_info(CARPINCHO->mate_logger, "Voy a enviar un sem_post al semaforo %s", sem);
	send_sem_operation(carpincho, sem, SEM_POST, CARPINCHO->socket);

	wait_ok(CARPINCHO->socket);

	return 0;
}

int mate_sem_destroy(mate_instance *carpincho, mate_sem_name sem) {


	log_info(CARPINCHO->mate_logger, "Voy a enviar destruiccion del semaforo %s", sem);
	send_sem_operation(carpincho, sem, SEM_DESTROY, CARPINCHO->socket);
	wait_ok(CARPINCHO->socket);

	return 0;
}

//--------------------IO Functions------------------------/

int mate_call_io(mate_instance *carpincho, mate_io_resource io, void *msg)
{
	log_info(CARPINCHO->mate_logger, "Voy a pedir IO de %s", io);
	send_io_usage(carpincho, io, msg, CARPINCHO->socket);

	wait_ok(CARPINCHO->socket);

	return 0;
}

//--------------Memory Module Functions-------------------/

mate_pointer mate_memalloc(mate_instance *carpincho, int size)
{
	log_info(CARPINCHO->mate_logger, "Voy a pedir un memalloc de tamaño %d", size);
	send_mate_memalloc_size(carpincho, size, CARPINCHO->socket);
	mate_pointer dir_logica;

	int confirm = recibir_operacion(CARPINCHO->socket);
	if(confirm==MEMORY_OK){
		dir_logica=recv_mate_mem_alloced(CARPINCHO->socket);
		log_info(CARPINCHO->mate_logger, "RECIBI LA DIRECCION %d", dir_logica);
	}
	else{
		log_warning(CARPINCHO->mate_logger, "HUBO UN ERROR CON MEMALLOC");
	}


	return dir_logica;
}

int mate_memfree(mate_instance *carpincho, mate_pointer addr)
{
	log_info(CARPINCHO->mate_logger, "Voy a pedir un memfree de la posicion %d", addr);
	send_mate_mem_addr(CARPINCHO->mate_id, addr, MEMFREE, CARPINCHO->socket);

	int result = recv_memory_confirm(CARPINCHO->socket);
	if(result==MEMORY_FAIL){
		return MATE_FREE_FAULT;
	}

	return 0;
}

int mate_memread(mate_instance *carpincho, mate_pointer origin, void *dest, int size)
{
	log_info(CARPINCHO->mate_logger, "Voy a pedir un memread de la posicion %d de tamaño %d", origin, size);
	send_mate_memread(carpincho, origin, size, CARPINCHO->socket);
	int result= recibir_operacion(CARPINCHO->socket);

	if(result==MEMORY_OK){
		recv_memreaded(dest, CARPINCHO->socket);
	}else{
		return MATE_READ_FAULT;
	}

	return 0;

}

int mate_memwrite(mate_instance *carpincho, void *origin, mate_pointer dest, int size)
{
	char* hex_string;
	hex_string=mem_hexstring(origin, size);
	log_info(CARPINCHO->mate_logger, "Voy a pedir un memwrite para escribir %s de la posicion %d de tamaño %d", hex_string, dest, size);
	free(hex_string);
	send_mate_memwrite(carpincho, origin, dest, size, CARPINCHO->socket);

	int result = recv_memory_confirm(CARPINCHO->socket);
	if(result==MEMORY_FAIL){
			return MATE_WRITE_FAULT;
	}


  return 0;
}
