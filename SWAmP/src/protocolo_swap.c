#include "../include/protocolo_swap.h"

int recv_tipo_asignacion(int socket){
	int size;
	int desplazamiento=0;
	int tipo_asignacion;

	void* buffer = recibir_buffer(&size, socket);

	memcpy(&tipo_asignacion, buffer+desplazamiento, sizeof(int));
	desplazamiento+=sizeof(int);

	free(buffer);

	return tipo_asignacion;
}


void recv_swap_out(swap_petition* petition, int socket){

	int size;
	int desplazamiento=0;

	int tam_bloque;

	void* buffer = recibir_buffer(&size, socket);

	memcpy(&(petition->mate_id), buffer+desplazamiento, sizeof(uint32_t));
	desplazamiento+=sizeof(uint32_t);
	memcpy(&(petition->numero_pagina), buffer+desplazamiento, sizeof(uint32_t));
	desplazamiento+=sizeof(uint32_t);
	memcpy(&tam_bloque, buffer+desplazamiento, sizeof(int));
	desplazamiento+=sizeof(int);
	petition->buffer=malloc(tam_bloque);
	memcpy(petition->buffer, buffer+desplazamiento, tam_bloque);
	desplazamiento+=tam_bloque;

	free(buffer);

}

void recv_swap_in_petition(swap_petition* petition, int socket){

	int size;
	int desplazamiento=0;

	void* buffer = recibir_buffer(&size, socket);

	memcpy(&(petition->mate_id), buffer+desplazamiento, sizeof(uint32_t));
	desplazamiento+=sizeof(uint32_t);
	memcpy(&(petition->numero_pagina), buffer+desplazamiento, sizeof(uint32_t));
	desplazamiento+=sizeof(uint32_t);

	free(buffer);
}

void recv_swap_delete_pag(swap_petition* petition, int socket){

    int size;
    int desplazamiento=0;

    void* buffer = recibir_buffer(&size, socket);

    memcpy(&(petition->mate_id), buffer+desplazamiento, sizeof(uint32_t));
    desplazamiento+=sizeof(uint32_t);
    memcpy(&(petition->numero_pagina), buffer+desplazamiento, sizeof(uint32_t));
    desplazamiento+=sizeof(uint32_t);

    free(buffer);
}

void recv_swap_delete(uint32_t* mate_id, int socket){

	int size;
	int desplazamiento=0;

	void* buffer = recibir_buffer(&size, socket);

	memcpy(mate_id, buffer+desplazamiento, sizeof(uint32_t));
	desplazamiento+=sizeof(uint32_t);

	free(buffer);

}


void send_swap_in(void* buffer, int size, int socket){

	t_paquete* paquete = crear_paquete(SWAPIN);
	int desplazamiento=0;

	paquete->buffer->size = size+sizeof(int);
	paquete->buffer->stream = malloc(paquete->buffer->size);

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

