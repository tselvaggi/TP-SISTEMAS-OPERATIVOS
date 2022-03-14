#include "../include/protocolo_memoria.h"
#include"../../shared/include/loggers_config.h"
#include <commons/memory.h>

void recv_init_memoria(uint32_t* mate_id, int socket){
	int size;
	int desplazamiento=0;

	int size_alloc;

	void* buffer = recibir_buffer(&size, socket);

	memcpy(mate_id, buffer+desplazamiento, sizeof(uint32_t));
	desplazamiento+=sizeof(uint32_t);

	free(buffer);
}

void send_exec_memoria(uint32_t mate_id, int socket){
	t_paquete* paquete = crear_paquete(CONTINUE);
	int desplazamiento=0;

	paquete->buffer->size = sizeof(uint32_t);
	paquete->buffer->stream = malloc(paquete->buffer->size);

	memcpy(paquete->buffer->stream + desplazamiento, &mate_id, sizeof(uint32_t));
	desplazamiento+=sizeof(uint32_t);

	int bytes = paquete->buffer->size + sizeof(int)*2;

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket, a_enviar, bytes, 0);


	free(a_enviar);
	eliminar_paquete(paquete);
}


void recv_mate_memalloc(mem_petition* petition, int socket){

	int size;
	int desplazamiento=0;

	int size_alloc;

	void* buffer = recibir_buffer(&size, socket);

	memcpy(&petition->mate_id, buffer+desplazamiento, sizeof(uint32_t));
	desplazamiento+=sizeof(uint32_t);
	memcpy(&petition->size, buffer+desplazamiento, sizeof(int));
	desplazamiento+=sizeof(int);

	free(buffer);


}

void send_mate_mem_alloced(uint32_t addr, int socket){

	t_paquete* paquete = crear_paquete(MEMORY_OK);
	int desplazamiento=0;

	paquete->buffer->size = sizeof(uint32_t);
	paquete->buffer->stream = malloc(paquete->buffer->size);


	memcpy(paquete->buffer->stream + desplazamiento, &addr, sizeof(uint32_t));
	desplazamiento+=sizeof(uint32_t);

	int bytes = paquete->buffer->size + sizeof(int)*2;

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket, a_enviar, bytes, 0);


	free(a_enviar);
	eliminar_paquete(paquete);

}

void recv_mate_mem_addr(mem_petition* petition,int socket){

	int size;
	int desplazamiento=0;

	int size_alloc;

	void* buffer = recibir_buffer(&size, socket);

	memcpy(&petition->mate_id, buffer+desplazamiento, sizeof(uint32_t));
	desplazamiento+=sizeof(uint32_t);
	memcpy(&petition->dir_logica, buffer+desplazamiento, sizeof(uint32_t));
	desplazamiento+=sizeof(uint32_t);

	free(buffer);

}

void recv_mate_memread(mem_petition* petition, int socket){

	int size;
	int desplazamiento=0;

	void* buffer = recibir_buffer(&size, socket);

	memcpy(&petition->mate_id, buffer+desplazamiento, sizeof(uint32_t));
	desplazamiento+=sizeof(uint32_t);
	memcpy(&petition->dir_logica, buffer+desplazamiento, sizeof(uint32_t));
	desplazamiento+=sizeof(uint32_t);
	memcpy(&petition->size, buffer+desplazamiento, sizeof(int));
	desplazamiento+=sizeof(int);

	free(buffer);

}

void recv_mate_memwrite(mem_petition* petition, int socket){

	int size;
	int desplazamiento=0;
	void* buffer = recibir_buffer(&size, socket);

	memcpy(&(petition->mate_id), buffer+desplazamiento, sizeof(uint32_t));
	desplazamiento+=sizeof(uint32_t);
	//log_info(logger, "Recibi el ID: %d", petition->mate_id);
	memcpy(&(petition->size), buffer+desplazamiento, sizeof(int));
	desplazamiento+=sizeof(int);
	//log_info(logger, "Recibi el tam: %d", petition->size);
	petition->origin=malloc(petition->size);
	memcpy(petition->origin, buffer+desplazamiento, petition->size);
	//char* hex_string;
	//hex_string=mem_hexstring(petition->origin, petition->size);
	//log_info(logger, "Recibi el msg: %s", hex_string);
	//free(hex_string);
	desplazamiento+=petition->size;
	memcpy(&(petition->dir_logica), buffer+desplazamiento, sizeof(uint32_t));

	free(buffer);
}


void send_memory_confirm(uint32_t dir_logica, op_code code_op, int socket){

	t_paquete* paquete = crear_paquete(code_op);
	int desplazamiento=0;

	paquete->buffer->size = sizeof(uint32_t);
	paquete->buffer->stream = malloc(paquete->buffer->size);

	memcpy(paquete->buffer->stream+desplazamiento, &dir_logica, sizeof(uint32_t));
	desplazamiento+=sizeof(uint32_t);

	int bytes = paquete->buffer->size+sizeof(int)*2;
	void* a_enviar = serializar_paquete(paquete, bytes);
	send(socket, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}

void send_mate_memerror(int memerror, int socket){

	send(socket, &memerror, sizeof(int), 0);
}


void send_memreaded(void* buffer, int size, int socket){

	t_paquete* paquete = crear_paquete(MEMORY_OK);
	int desplazamiento=0;

	paquete->buffer->size = sizeof(int) + size;
	paquete->buffer->stream = malloc(paquete->buffer->size);

	memcpy(paquete->buffer->stream + desplazamiento, &size, sizeof(int));
	desplazamiento+=sizeof(int);
	memcpy(paquete->buffer->stream + desplazamiento, buffer, size);
	desplazamiento+=size;

	int bytes = paquete->buffer->size + sizeof(int)*2;

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);

}

void recv_suspended_memoria(mem_petition* petition, int socket){

	int size;
	int desplazamiento=0;


	void* buffer = recibir_buffer(&size, socket);

	memcpy(&(petition->mate_id), buffer+desplazamiento, sizeof(uint32_t));
	desplazamiento+=sizeof(uint32_t);

	free(buffer);

}

void send_tipo_asignacion(int tipo_asignacion, op_code code_op, int socket){

	t_paquete* paquete = crear_paquete(code_op);
	int desplazamiento=0;

	paquete->buffer->size = sizeof(int);
	paquete->buffer->stream = malloc(paquete->buffer->size);

	memcpy(paquete->buffer->stream+desplazamiento, &tipo_asignacion, sizeof(int));
	desplazamiento+=sizeof(int);

	int bytes = paquete->buffer->size+sizeof(int)*2;
	void* a_enviar = serializar_paquete(paquete, bytes);
	send(socket, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}

void send_swap_out(void* buffer, int size, uint32_t mate_id, uint32_t numero_pagina, int socket){

	t_paquete* paquete = crear_paquete(SWAPWRITE);
	int desplazamiento=0;

	paquete->buffer->size = size+sizeof(int)+sizeof(uint32_t)*2;
	paquete->buffer->stream = malloc(paquete->buffer->size);

	memcpy(paquete->buffer->stream+desplazamiento, &mate_id, sizeof(uint32_t));
	desplazamiento+=sizeof(uint32_t);
	memcpy(paquete->buffer->stream+desplazamiento, &numero_pagina, sizeof(uint32_t));
	desplazamiento+=sizeof(uint32_t);
	memcpy(paquete->buffer->stream+desplazamiento, &size, sizeof(int));
	desplazamiento+=sizeof(int);
	memcpy(paquete->buffer->stream+desplazamiento, buffer, size);
	desplazamiento+=size;


	int bytes = paquete->buffer->size+sizeof(int)*2;
	void* a_enviar = serializar_paquete(paquete, bytes);
	send(socket, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}

void send_swap_in_petition(uint32_t mate_id, uint32_t numero_pagina, int socket){
	t_paquete* paquete = crear_paquete(SWAPREAD);
	int desplazamiento=0;

	paquete->buffer->size = sizeof(uint32_t)*2;
	paquete->buffer->stream = malloc(paquete->buffer->size);

	memcpy(paquete->buffer->stream+desplazamiento, &mate_id, sizeof(uint32_t));
	desplazamiento+=sizeof(uint32_t);
	memcpy(paquete->buffer->stream+desplazamiento, &numero_pagina, sizeof(uint32_t));
	desplazamiento+=sizeof(uint32_t);

	int bytes = paquete->buffer->size+sizeof(int)*2;
	void* a_enviar = serializar_paquete(paquete, bytes);
	send(socket, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}

void send_swap_delete(uint32_t mate_id, int socket){

	t_paquete* paquete = crear_paquete(SWAPDELETE);
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

void send_swap_delete_pag(uint32_t mate_id, uint32_t numero_pagina, int socket){

    t_paquete* paquete = crear_paquete(SWAPDELETEPAG);
    int desplazamiento=0;

    paquete->buffer->size = sizeof(uint32_t)*2;
    paquete->buffer->stream = malloc(paquete->buffer->size);

    memcpy(paquete->buffer->stream+desplazamiento, &mate_id, sizeof(uint32_t));
    desplazamiento+=sizeof(uint32_t);
    memcpy(paquete->buffer->stream+desplazamiento, &numero_pagina, sizeof(uint32_t));
    desplazamiento+=sizeof(uint32_t);

    int bytes = paquete->buffer->size+sizeof(int)*2;
    void* a_enviar = serializar_paquete(paquete, bytes);
    send(socket, a_enviar, bytes, 0);

    free(a_enviar);
    eliminar_paquete(paquete);
}

void recv_swap_in(void** bloque, int socket){

	int size;
	int desplazamiento=0;

	int cod_op = recibir_operacion(socket);

	if(cod_op!=SWAPIN){
		log_warning(logger, "HUBO UN ERROR EN EL SWAP IN %d", cod_op);
		return;
	}

	int tam_bloque;

	void* buffer = recibir_buffer(&size, socket);

	memcpy(&tam_bloque, buffer+desplazamiento, sizeof(int));
	desplazamiento+=sizeof(int);
	memcpy(*bloque, buffer+desplazamiento, tam_bloque);
	desplazamiento+=tam_bloque;

	char* hex_string;
	hex_string=mem_hexstring(*bloque, tam_bloque);
	log_debug(logger, "Recibi de swap: %s", hex_string);
	free(hex_string);

	free(buffer);
}
