#include "../include/protocolo_kernel.h"
#include "../../shared/include/loggers_config.h"

void recv_init(uint32_t mate_id, int socket){

	int size;
	int desplazamiento=0;

	void* buffer;

	buffer = recibir_buffer(&size, socket);

	memcpy(&mate_id, buffer+desplazamiento, sizeof(uint32_t));
	desplazamiento+=sizeof(uint32_t);


	free(buffer);


}

void send_init_memoria(uint32_t mate_id, int socket)
{
	t_paquete* paquete = crear_paquete(INIT);
	int desplazamiento=0;

	paquete->buffer->size = sizeof(uint32_t);

	paquete->buffer->stream = malloc(paquete->buffer->size);

	memcpy(paquete->buffer->stream + desplazamiento, &mate_id, sizeof(uint32_t));
	desplazamiento+=sizeof(uint32_t);

	paquete->buffer->size = desplazamiento;

	int bytes = paquete->buffer->size + sizeof(int)*2;

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket, a_enviar, bytes, 0);

	free(a_enviar);

	eliminar_paquete(paquete);
}

void send_exec(uint32_t mate_id, int socket){

	t_paquete* paquete = crear_paquete(CONTINUE);
	int desplazamiento=0;

	paquete->buffer->size = sizeof(uint32_t);
	paquete->buffer->stream = malloc(paquete->buffer->size);

	memcpy(paquete->buffer->stream + desplazamiento, &mate_id, sizeof(uint32_t));
	desplazamiento+=sizeof(uint32_t);

	paquete->buffer->size = desplazamiento;

	int bytes = paquete->buffer->size + sizeof(int)*2;

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket, a_enviar, bytes, 0);

	free(a_enviar);

	eliminar_paquete(paquete);

}

//TODO: SEND EXEC A MEMORIA

void recv_exec_memoria(int conexion_memoria){

	int cod_op = recibir_operacion(conexion_memoria);

	if(cod_op!=CONTINUE){
		log_warning(logger,"OCURRIO UN ERROR EN EL INIT DE MEMORIA");
	}

	vaciar_buffer(conexion_memoria);
}

void recv_sem_init(kernel_sem* sem, int socket){

	int size;
	int desplazamiento=0;

	void* buffer = recibir_buffer(&size, socket);

	int size_sem_name;


	memcpy(&size_sem_name, buffer+desplazamiento, sizeof(int));
	desplazamiento+=sizeof(int);
	log_info(logger, "Se le asigno %d a sem name", size_sem_name);
	sem->name = malloc(size_sem_name);
	log_info(logger, "SE PUDO ALLOCAR MEM PARA SEM NAME");
	memcpy(sem->name, buffer+desplazamiento, size_sem_name);
	desplazamiento+=size_sem_name;
	memcpy(&sem->value, buffer+desplazamiento, sizeof(int));
	desplazamiento+=sizeof(int);

	free(buffer);

	log_info(logger, "Se inicio el semaforo con los valores %s %d", sem->name, sem->value);

}

char* recv_sem_operation(int socket){

	int size;
	int desplazamiento=0;
	char* sem_name;

	void* buffer = recibir_buffer(&size, socket);

	int size_sem_name;

	memcpy(&size_sem_name, buffer+desplazamiento, sizeof(int));
	desplazamiento+=sizeof(int);
	sem_name=malloc(size_sem_name);
	memcpy(sem_name, buffer+desplazamiento, size_sem_name);
	desplazamiento+=size_sem_name;

	free(buffer);

	return sem_name;
}

char* recv_io_usage(int socket){

	int size;
	int desplazamiento=0;
	char* io_name;

	void* buffer = recibir_buffer(&size, socket);

	int size_io_name;

	memcpy(&size_io_name, buffer+desplazamiento, sizeof(int));
	desplazamiento+=sizeof(int);
	io_name=malloc(size_io_name);
	memcpy(io_name, buffer+desplazamiento, size_io_name);
	desplazamiento+=size_io_name;

	free(buffer);

	return io_name;
}

void pasamanos_kernel_memoria(int socket_from, int socket_dest, op_code cod_op){

	int size;
	int desplazamiento=0;

	void* buffer = recibir_buffer(&size, socket_from);

	t_paquete* paquete = crear_paquete(cod_op);

	paquete->buffer->size=size;
	log_info(logger, "VOY A PASAR UN SIZE DE: %d", size);
	paquete->buffer->stream = malloc(paquete->buffer->size);

	memcpy(paquete->buffer->stream + desplazamiento, buffer, paquete->buffer->size);
	desplazamiento+=paquete->buffer->size;

	int bytes = paquete->buffer->size + sizeof(int)*2;

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_dest, a_enviar, bytes, 0);

	free(buffer);
	free(a_enviar);
	eliminar_paquete(paquete);


}

void send_suspended_memoria(uint32_t mate_id, int socket){

	t_paquete* paquete = crear_paquete(MATE_SUSPEND);
	int desplazamiento=0;

	paquete->buffer->size = sizeof(uint32_t);
	paquete->buffer->stream = malloc(paquete->buffer->size);

	memcpy(paquete->buffer->stream+desplazamiento, &mate_id, sizeof(uint32_t));
	desplazamiento+=sizeof(uint32_t);

	int bytes = paquete->buffer->size+sizeof(int)*2;
	void* a_enviar = serializar_paquete(paquete, bytes);
	send(socket, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}

