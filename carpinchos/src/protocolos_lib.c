#include "../include/protocolos_lib.h"
#include "../include/declaraciones.h"



void send_init(mate_instance* carpincho, int socket)
{
	t_paquete* paquete = crear_paquete(INIT);
	int desplazamiento=0;

	paquete->buffer->size = sizeof(uint32_t);

	paquete->buffer->stream = malloc(paquete->buffer->size);

	memcpy(paquete->buffer->stream + desplazamiento, &CARPINCHO->mate_id, sizeof(uint32_t));
	desplazamiento+=sizeof(uint32_t);

	paquete->buffer->size = desplazamiento;

	int bytes = paquete->buffer->size + sizeof(int)*2;

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket, a_enviar, bytes, 0);

	free(a_enviar);

	eliminar_paquete(paquete);
}

void recv_exec(mate_instance* carpincho, int socket){

	int size;
	int desplazamiento=0;

	if(recibir_operacion(socket)!=CONTINUE){
			log_warning(CARPINCHO->mate_logger, "SE RECIBIO UN COD OP ERRONEO EN EXEC!!!");
			vaciar_buffer(socket);
	}
	else{
		void *buffer;
		buffer = recibir_buffer(&size, socket);

		memcpy(&CARPINCHO->mate_id, buffer+desplazamiento, sizeof(uint32_t));
		desplazamiento+=sizeof(uint32_t);

		free(buffer);

	}
}

void send_sem_init(mate_instance* carpincho, char* sem, unsigned int value, int socket){

	t_paquete* paquete = crear_paquete(SEM_INIT);
	int desplazamiento=0;
	int sem_name_tam = strlen(sem)+1;
	paquete->buffer->size = sem_name_tam + sizeof(int)*2;
	paquete->buffer->stream = malloc(paquete->buffer->size);

	memcpy(paquete->buffer->stream + desplazamiento, &sem_name_tam, sizeof(int));
	desplazamiento+=sizeof(int);
	memcpy(paquete->buffer->stream + desplazamiento, sem, sem_name_tam);
	desplazamiento+=sem_name_tam;
	memcpy(paquete->buffer->stream + desplazamiento, &value, sizeof(int));
	desplazamiento+=sizeof(int);

	paquete->buffer->size = desplazamiento;

	int bytes = paquete->buffer->size + sizeof(int)*2;

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);

}

void send_sem_operation(mate_instance* carpincho, char* sem, op_code code_op, int socket){

	t_paquete* paquete = crear_paquete(code_op);
	int desplazamiento=0;
	int sem_name_tam = strlen(sem)+1;
	paquete->buffer->size = sizeof(int) + sem_name_tam;
	paquete->buffer->stream = malloc(paquete->buffer->size);

	memcpy(paquete->buffer->stream + desplazamiento, &sem_name_tam, sizeof(int));
	desplazamiento+=sizeof(int);
	memcpy(paquete->buffer->stream + desplazamiento, sem, sem_name_tam);
	desplazamiento+=sem_name_tam;

	int bytes = paquete->buffer->size + sizeof(int)*2;

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket, a_enviar, bytes, 0);


	free(a_enviar);
	eliminar_paquete(paquete);

}


void send_io_usage(mate_instance* carpincho, char* io, void* msg, int socket){

	t_paquete* paquete = crear_paquete(CALL_IO);
	int desplazamiento=0;
	int io_name_tam = strlen(io)+1;
	paquete->buffer->size = sizeof(int) + io_name_tam;
	paquete->buffer->stream = malloc(paquete->buffer->size);

	memcpy(paquete->buffer->stream + desplazamiento, &io_name_tam, sizeof(int));
	desplazamiento+=sizeof(int);
	memcpy(paquete->buffer->stream + desplazamiento, io, io_name_tam);
	desplazamiento+=io_name_tam;

	int bytes = paquete->buffer->size + sizeof(int)*2;

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}

void send_mate_memalloc_size(mate_instance* carpincho, int size, int socket){

	t_paquete* paquete = crear_paquete(MEMALLOC);
	int desplazamiento=0;

	paquete->buffer->size = sizeof(int)+sizeof(uint32_t);
	paquete->buffer->stream = malloc(paquete->buffer->size);


	//PODRIA (LO MAS PROBABLE ES QUE DEBERIA) PASARLE EL ID DEL CARPINCHO
	memcpy(paquete->buffer->stream + desplazamiento, &(CARPINCHO->mate_id), sizeof(uint32_t));
	desplazamiento+=sizeof(uint32_t);
	memcpy(paquete->buffer->stream + desplazamiento, &size, sizeof(int));
	desplazamiento+=sizeof(int);

	int bytes = paquete->buffer->size + sizeof(int)*2;

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket, a_enviar, bytes, 0);


	free(a_enviar);
	eliminar_paquete(paquete);

}

void send_mate_mem_addr(uint32_t mate_id, uint32_t addr, op_code code_op, int socket){

	t_paquete* paquete = crear_paquete(code_op);
	int desplazamiento=0;

	paquete->buffer->size = sizeof(uint32_t) + sizeof(uint32_t);
	paquete->buffer->stream = malloc(paquete->buffer->size);

	memcpy(paquete->buffer->stream + desplazamiento, &mate_id, sizeof(uint32_t));
	desplazamiento+=sizeof(uint32_t);
	memcpy(paquete->buffer->stream + desplazamiento, &addr, sizeof(uint32_t));
	desplazamiento+=sizeof(uint32_t);

	int bytes = paquete->buffer->size + sizeof(int)*2;

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket, a_enviar, bytes, 0);


	free(a_enviar);
	eliminar_paquete(paquete);

}

void send_mate_memread(mate_instance* carpincho, uint32_t origin, int size, int socket){

	t_paquete* paquete = crear_paquete(MEMREAD);
	int desplazamiento=0;

	paquete->buffer->size = sizeof(int) + sizeof(uint32_t)*2;
	paquete->buffer->stream = malloc(paquete->buffer->size);

	memcpy(paquete->buffer->stream + desplazamiento, &(CARPINCHO->mate_id), sizeof(uint32_t));
	desplazamiento+=sizeof(uint32_t);
	memcpy(paquete->buffer->stream + desplazamiento, &origin, sizeof(uint32_t));
	desplazamiento+=sizeof(uint32_t);
	memcpy(paquete->buffer->stream + desplazamiento, &size, sizeof(int));
	desplazamiento+=sizeof(int);

	int bytes = paquete->buffer->size + sizeof(int)*2;

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);

}

void send_mate_memwrite(mate_instance* carpincho, void* origin, uint32_t dest, int size, int socket){

	t_paquete* paquete = crear_paquete(MEMWRITE);
	int desplazamiento=0;

	paquete->buffer->size = sizeof(int) + size + sizeof(uint32_t)*2;
	paquete->buffer->stream = malloc(paquete->buffer->size);

	memcpy(paquete->buffer->stream + desplazamiento, &(CARPINCHO->mate_id), sizeof(uint32_t));
	desplazamiento+=sizeof(uint32_t);
	memcpy(paquete->buffer->stream + desplazamiento, &size, sizeof(int));
	desplazamiento+=sizeof(int);
	memcpy(paquete->buffer->stream + desplazamiento, origin, size);
	desplazamiento+=size;
	memcpy(paquete->buffer->stream + desplazamiento, &dest, sizeof(uint32_t));
	desplazamiento+=sizeof(uint32_t);

	int bytes = paquete->buffer->size + sizeof(int)*2;

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}

uint32_t recv_mate_mem_alloced(int socket){

	int size;
	int desplazamiento=0;

	int size_alloc;
	uint32_t addr;

	void* buffer = recibir_buffer(&size, socket);

	memcpy(&addr, buffer+desplazamiento, sizeof(uint32_t));
	desplazamiento+=sizeof(uint32_t);

	free(buffer);

	return addr;

}

void recv_memreaded(void* buffer_leido, int socket){

	int size;
	int desplazamiento=0;
	int tam_leido;


	void* buffer = recibir_buffer(&size, socket);


	memcpy(&tam_leido, buffer+desplazamiento, sizeof(int));
	desplazamiento+=sizeof(int);
	//buffer_leido=malloc(tam_leido);
	memcpy(buffer_leido, buffer+desplazamiento, tam_leido);

	desplazamiento+=tam_leido;

	free(buffer);
}


int recv_memory_confirm(int socket){

	int size;
	int desplazamiento=0;
	uint32_t dir_logica;

	int result = recibir_operacion(socket);

	vaciar_buffer(socket);


	return result;
}

