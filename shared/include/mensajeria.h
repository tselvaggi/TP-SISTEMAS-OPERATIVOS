#ifndef SHARED_SRC_MENSAJERIA_H_
#define SHARED_SRC_MENSAJERIA_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<string.h>

typedef enum
{
	INIT,
	CONTINUE,
	ERROR,
	SEM_INIT,
	SEM_WAIT,
	SEM_POST,
	SEM_DESTROY,
	CALL_IO,
	MEMALLOC,
	MEMFREE,
	MEMREAD,
	MEMWRITE,
	MEMORY_OK,
	MEMORY_FAIL,
	MATE_SUSPEND,
	SWAPREAD,
	SWAPWRITE,
	SWAPDELETE,
	SWAPDELETEPAG,
	SWAPIN,
	ASIGNACION,
	CLOSE,
	DEBUG
}op_code;

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

void crear_buffer(t_paquete* paquete);
t_paquete* crear_paquete(op_code cod_op);
void* serializar_paquete(t_paquete*, int);
void eliminar_paquete(t_paquete* paquete);

void* recibir_buffer(int*, int);
int recibir_operacion(int socket);
void vaciar_buffer(int socket);

void send_debug(int socket);

void send_ok(int socket);
void send_not_ok(int socket);
int wait_ok(int socket);

#endif /* SHARED_SRC_MENSAJERIA_H_ */
