#include "../include/mensajeria.h"


t_paquete* crear_paquete(op_code cod_op)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = cod_op;
	crear_buffer(paquete);
	return paquete;
}

void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

void* serializar_paquete(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}

void eliminar_paquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}


void* recibir_buffer(int* size, int socket_cliente)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) != 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}

void vaciar_buffer(int socket){

	int size;
	void* buffer = recibir_buffer(&size, socket);
	free(buffer);
}

void send_ok(int socket){

	int confirm = CONTINUE;
	send(socket, &confirm, sizeof(int), 0);

}

void send_not_ok(int socket){
	int confirm = ERROR;
	send(socket, &confirm, sizeof(int), 0);
}

int wait_ok(int socket){

	int confirm;
	recv(socket, &confirm, sizeof(int), MSG_WAITALL);
	switch (confirm){
	case CONTINUE:
		return confirm;
		break;
	default:
		//log_warning(logger, "OCURRIO UN ERROR ESPERANDO EL OK DE SOCKETS");
		return confirm;
		break;
	}
}


void send_debug(int socket){

	int confirm = DEBUG;
	send(socket, &confirm, sizeof(int), 0);

}
