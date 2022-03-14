#include "include/main.h"
#include "../shared/include/servidor.h"
#include "../shared/include/cliente.h"
#include <commons/memory.h>


datos_config d_config;
t_list* tabla_arch;
int cliente_fd;

int main(int argc, char *argv[]) {
	char *ip, *puerto;

	//inicio logger y config
	logger = iniciar_logger("swamp", "SWAMP");
	config = iniciar_config(argv[1]);

	//inicializo el config
	inicializar_config();

	//creo el servidor y espero al cliente
	int server_fd = iniciar_servidor(d_config.ip, d_config.puerto);
	log_info(logger, "Servidor de SWAMP listo para recibir mensajes");
	cliente_fd = esperar_cliente(server_fd);

	//creo los archivos
	crear_archivos();
	log_info(logger, "Archivos de swap creados");

	//en este while se deben tratar las ordenes que reciba
	int tipo_asignacion;
	bool operar=true;
	while(operar){
		int cod_op = recibir_operacion(cliente_fd);
		log_info(logger, "-----------------------------------");

		//hacer retardo del swap
		usleep(d_config.retardo_swap);

		switch (cod_op) {
		case ASIGNACION:
			//recibo que tipo de asignacion vamos a utilizar 0 para fija 1 para dinamica
			tipo_asignacion=recv_tipo_asignacion(cliente_fd);
			if(tipo_asignacion==0){
				log_info(logger, "El tipo de asignacion a utilizar es fija");
			}else if(tipo_asignacion==1){
				log_info(logger, "El tipo de asignacion a utilizar es dinamica");
			}
			send_ok(cliente_fd);
			break;
		case SWAPREAD:
			log_info(logger, "Recibi la orden de leer una pagina");

			//recibo la pagina que se busca
			swap_petition pag_recib;
			recv_swap_in_petition(&pag_recib, cliente_fd);

			//Busco en el mapa del archivo la posicion en la que se encuentra la pagina recibida
			arch_bloq arch_bloque = buscar_bloque(pag_recib);

			//Leo el archivo indicado en el bloque indicado
			if(arch_bloque.n_arch==-1){
				send_not_ok(cliente_fd);
			}else{
				void* cont_bloq = leer_archivo(arch_bloque);

				//envio el contenido de la pagina
				send_swap_in(cont_bloq, d_config.tamanio_pagina , cliente_fd);

				//libero el espacio de mallocs hechos en las funciones
				free(cont_bloq);
			}
			break;
		case SWAPWRITE:
			log_info(logger, "Recibi la orden de escribir una pagina");

			//recibo una peticion de swap out con el numero de pagina, el id del proceso y el contenido de la pagina
			swap_petition cont_pag;
			recv_swap_out(&cont_pag, cliente_fd);

			//revizo los archivos en busca de un archivo con informacion del mismo proceso almacenado o el archivo menos lleno
			int archivo_selec;
			archivo_selec = buscar_archivo(cont_pag);
			log_info(logger, "Archivo seleccionado: %d", archivo_selec);

			//revizo un bloque que coincida con el recibido o un bloque vacio y lo escribo
			if(tipo_asignacion==1){
				//caso de asignacion dinamica/global
				escribir_archivo(archivo_selec, cont_pag);
			}else{
				//caso de asignacion fija
				escribir_archivo_fijo(archivo_selec, cont_pag);
			}
			free(cont_pag.buffer);
			break;
		case SWAPDELETE:

			;//recibo la id del carpincho cuyas paginas se deben borrar de memoria
			uint32_t mate_id;
			recv_swap_delete(&mate_id, cliente_fd);
			log_info(logger, "Recibi la orden de borrar el proceso %d", mate_id);

			//borro de la tabla de los archivos los los valores dde los elementos de las listas de dicho carpincho
			liberar_bloques(mate_id);

			send_ok(cliente_fd);
			break;
		case SWAPDELETEPAG:
			log_info(logger, "Recibi la orden de borrar una unica pagina");

			//recibo la id del carpincho y la unica pagina que se debe borrar de memoria
			swap_petition pag_rec;
			recv_swap_delete_pag(&pag_rec, cliente_fd);

			//Busco en el mapa del archivo la posicion en la que se encuentra la pagina recibida
			arch_bloq bloq_id = buscar_bloque(pag_rec);

			//Borro el bloque indicado en el mapa de archivos
			liberar_bloque(bloq_id);

			send_ok(cliente_fd);
			break;
		case -1:
			log_error(logger, "El cliente se desconecto. Terminando servidor");
			//Aca se hace free de todos los mallocs que son para la tabla archivo y se cierran los archivos
			free_memoria();
			operar=false;
			break;
		default:
			log_warning(logger,"Operacion desconocida. No quieras meter la pata");
			break;
		}
	}
	return EXIT_FAILURE;
}

void inicializar_config(){
	d_config.ip = config_get_string_value(config, "IP");
	d_config.puerto = config_get_string_value(config, "PUERTO");
	d_config.tamanio_swap = config_get_int_value(config, "TAMANIO_SWAMP");
	d_config.tamanio_pagina = config_get_int_value(config, "TAMANIO_PAGINA");
	d_config.archivos_swap = config_get_array_value(config, "ARCHIVOS_SWAMP");
	d_config.cantidad_archivos =  string_array_size(d_config.archivos_swap);
	d_config.marcos_por_carpincho = config_get_int_value(config, "MARCOS_POR_CARPINCHO");
	d_config.retardo_swap = config_get_int_value(config, "RETARDO_SWAMP");
	return;
}

void crear_archivos(){
	char** archivos = d_config.archivos_swap;

	tabla_arch=list_create();

	for(int i=0; i<d_config.cantidad_archivos; i++){
		//abro los archivos para su creacion y los dejo abiertos en modo lectura y escritura
		FILE* fp;
		fp = fopen(archivos[i], "w+");
		truncate(archivos[i], d_config.tamanio_swap);

		mate_file* mate_arch = malloc(sizeof(mate_file));
		mate_arch->file=fp;
		mate_arch->bloques=list_create();
		mate_arch->nro_arch=i;

		for(int j=0; j<d_config.tamanio_swap/d_config.tamanio_pagina; j++){
			bloque_archivo* bloque = malloc(sizeof(bloque_archivo));
			bloque->mate_id=UINT32_MAX;
			bloque->numero_pagina=UINT32_MAX;
			bloque->posicion=UINT32_MAX;
			list_add(mate_arch->bloques, bloque);
		}
		list_add(tabla_arch, mate_arch);
	}
}

arch_bloq buscar_bloque(swap_petition pag_recib){
	arch_bloq arch_bloq;
	for(int i=0; i<d_config.cantidad_archivos; i++){
		mate_file *archivo;
		archivo = list_get(tabla_arch, i);
		for(int j=0; j<d_config.tamanio_swap/d_config.tamanio_pagina; j++){
			bloque_archivo* bloques;
			bloques = list_get(archivo->bloques, j);
			if(bloques->mate_id==pag_recib.mate_id && bloques->numero_pagina==pag_recib.numero_pagina){
				log_info(logger, "Se encontro la pagina buscada en el archivo %d, bloque %d", i, j);
				arch_bloq.n_arch=i;
				arch_bloq.n_bloq=j;
				return arch_bloq;
			}
		}
	}
	log_warning(logger, "No se encontro un bloque que concida con la pagina y proceso recibido, pagina %d y proceso %d", pag_recib.numero_pagina, pag_recib.mate_id);
	arch_bloq.n_arch=-1;
	arch_bloq.n_bloq=-1;
	return arch_bloq;
}

void* leer_archivo(arch_bloq arch_bloq){
	void * cont_arch = malloc(d_config.tamanio_pagina);

	//se lee el archivo indicado en la posicion de memoria del bloque indicado
	mate_file *archivo;
	archivo = list_get(tabla_arch, arch_bloq.n_arch);
	FILE* fp = archivo->file;
	fseek(fp, arch_bloq.n_bloq*d_config.tamanio_pagina, SEEK_SET);
	fread(cont_arch, d_config.tamanio_pagina, 1, fp);
	char* hex_string;
	hex_string=mem_hexstring(cont_arch, d_config.tamanio_pagina);
	log_info(logger, "En el archivo se leyo lo siguiente: %s", hex_string);
	free(hex_string);

	return cont_arch;
}

int buscar_archivo(swap_petition cont_pag){
	//busco un archivo que contenga un bloque del mismo proceso y devuelvo el numero de archivo
	for(int i=0; i<d_config.cantidad_archivos; i++){
		mate_file *archivo;
		archivo = list_get(tabla_arch, i);
		for(int j=0; j<d_config.tamanio_swap/d_config.tamanio_pagina; j++){
			bloque_archivo* bloques;
			bloques = list_get(archivo->bloques, j);
			if(bloques->mate_id==cont_pag.mate_id){
				log_info(logger, "Se encontro un archivo con un bloque del mismo proceso, es el archivo %d", i);
				return i;
			}
		}
	}

	//busco el archivo que contenga mas bloques vacios
	int nro_arch=0;
	int contmax=0;
	for(int i=0; i<d_config.cantidad_archivos; i++){
		int cont=0;
		mate_file* archivo;
		archivo = list_get(tabla_arch, i);
		for(int j=0; j<d_config.tamanio_swap/d_config.tamanio_pagina; j++){
			bloque_archivo* bloques;
			bloques = list_get(archivo->bloques, j);
			if(bloques->posicion==UINT32_MAX){
				cont++;
			}
		}
		if(cont>contmax){
			contmax=cont;
			nro_arch=i;
		}
	}
	if(contmax==0){
		return -1;
	}

	//devuelvo el numero del archivo con mas bloques vacios
	log_info(logger, "El archivo seleccionado es el %d por tener la mayor cantidad de bloques vacios.", nro_arch);
	return nro_arch;
}

void escribir_archivo(int archivo_selec, swap_petition cont_pag){
	if(archivo_selec==-1){
		log_info(logger, "ERROR, No hay ningun bloque vacio en ningun archivo");
	}else{
		mate_file *archivo;
		archivo = list_get(tabla_arch, archivo_selec);
		int bloque_vac=-1;

		//busco en el archivo indicado anteriormente un remanente de la pagina del proceso recibido
		for(int i=0; i<d_config.tamanio_swap/d_config.tamanio_pagina; i++){
			bloque_archivo* bloques;
			bloques = list_get(archivo->bloques, i);
			if(bloques->mate_id==cont_pag.mate_id && bloques->numero_pagina==cont_pag.numero_pagina){
				bloque_vac=i;
				log_info(logger, "Se encontro la pagina recibida ya escrita en memoria en el bloque %d, archivo %d",bloque_vac, archivo_selec);
			}
		}

		//Busco un bloque vacio del archivo seleccionado
		if (bloque_vac==-1){
			for(int i=0; i<d_config.tamanio_swap/d_config.tamanio_pagina; i++){
				bloque_archivo* bloque;
				bloque = list_get(archivo->bloques, i);
				if(bloque->posicion==UINT32_MAX){
					bloque_vac=i;
					log_info(logger, "El bloque vacio seleccionado es el %d del archivo %d",bloque_vac, archivo_selec);
					i=d_config.tamanio_swap/d_config.tamanio_pagina;
				}
			}
		}

		if(bloque_vac==-1){
			log_info(logger, "No hay ningun bloque vacio en el archivo %d", archivo_selec);
			send_not_ok(cliente_fd);
		} else {
			//con el numero del bloque vacio cambio la tabla de archivo acorde a los cambios a realizar en los archivos
			bloque_archivo* bloque;
			bloque = list_get(archivo->bloques, bloque_vac);
			bloque->mate_id=cont_pag.mate_id;
			bloque->numero_pagina=cont_pag.numero_pagina;
			bloque->posicion=bloque_vac;

			//escribo en el archivo el contenido de la pagina
			FILE* fp = archivo->file;
			fseek(fp, bloque->posicion*d_config.tamanio_pagina, SEEK_SET);
			fwrite(cont_pag.buffer, 1, d_config.tamanio_pagina, fp);
			char* hex_string;
			hex_string = mem_hexstring(cont_pag.buffer, d_config.tamanio_pagina);
			log_info(logger, "Escribi en el archivo %d lo siguiente: %s", archivo_selec, hex_string);
			free(hex_string);
			send_ok(cliente_fd);
		}
	}
}

void escribir_archivo_fijo(int archivo_selec, swap_petition cont_pag){
	if(archivo_selec==-1){
		log_info(logger, "ERROR, No hay ningun bloque vacio en ningun archivo");
	}else{
		mate_file *archivo;
		archivo = list_get(tabla_arch, archivo_selec);
		int bloque_vac=-1;

		//busco en los archivos si la pagina del proceso recibido ya se encuentra en el archivo indicado
		for(int i=0; i<d_config.tamanio_swap/d_config.tamanio_pagina; i=i+d_config.marcos_por_carpincho){
			bloque_archivo* bloque;
			bloque = list_get(archivo->bloques, i);
			if(bloque->mate_id==cont_pag.mate_id){
				for(int j=0; j<d_config.marcos_por_carpincho;j++){
					bloque = list_get(archivo->bloques, i+j);
					if(bloque->numero_pagina==cont_pag.numero_pagina){
						bloque_vac=i+j;
						log_info(logger, "Se encontro la pagina recibida ya escrita en memoria en el bloque %d, archivo %d",bloque_vac, archivo_selec);
						j=d_config.marcos_por_carpincho;
					}
				}
				i=d_config.tamanio_swap/d_config.tamanio_pagina;
			}
		}

		//Busco un bloque vacio dentro del espacio asignado al carpincho recibido en el archivo seleccionado
		if (bloque_vac==-1){
			for(int i=0; i<d_config.tamanio_swap/d_config.tamanio_pagina; i=i+d_config.marcos_por_carpincho){
				bloque_archivo* bloque;
				bloque = list_get(archivo->bloques, i);
				if(bloque->mate_id==cont_pag.mate_id){
					for(int j=0; j<d_config.marcos_por_carpincho;j++){
						bloque = list_get(archivo->bloques, i+j);
						if(bloque->posicion==UINT32_MAX){
							bloque_vac=i+j;
							log_info(logger, "El bloque vacio seleccionado para la nueva pagina del proceso ya existente es el %d del archivo %d",bloque_vac, archivo_selec);
							j=d_config.marcos_por_carpincho;
						}
					}
					if(bloque_vac==-1){
						//que entre aca significa que no quedan bloques vacios para la cantidad de bloques asignados para cada carpincho
						log_info(logger, "No quedan bloques vacios para el carpincho %d", cont_pag.mate_id);
						send_not_ok(cliente_fd);
						return;
					}
					i=d_config.tamanio_swap/d_config.tamanio_pagina;
				}
			}
		}

		//Busco un bloque vacio en el archivo seleccionado para el nuevo proceso
		if (bloque_vac==-1){
			for(int i=0; i<d_config.tamanio_swap/d_config.tamanio_pagina; i=i+d_config.marcos_por_carpincho){
				bloque_archivo* bloque;
				bloque = list_get(archivo->bloques, i);
				if(bloque->posicion==UINT32_MAX){
					bloque_vac=i;
					log_info(logger, "El bloque vacio seleccionado para el nuevo proceso es el %d del archivo %d",bloque_vac, archivo_selec);
					i=d_config.tamanio_swap/d_config.tamanio_pagina;
				}
			}
		}

		if(bloque_vac==-1){
			log_info(logger, "No hay ningun bloque vacio en el archivo %d", archivo_selec);
			send_not_ok(cliente_fd);
		} else {
			//con el numero del bloque vacio cambio la tabla de archivo acorde a los cambios a realizar en los archivos
			bloque_archivo* bloque;
			bloque = list_get(archivo->bloques, bloque_vac);
			bloque->mate_id=cont_pag.mate_id;
			bloque->numero_pagina=cont_pag.numero_pagina;
			bloque->posicion=bloque_vac;

			//escribo en el archivo el contenido de la pagina
			FILE* fp = archivo->file;
			fseek(fp, bloque->posicion*d_config.tamanio_pagina, SEEK_SET);
			fwrite(cont_pag.buffer, 1, d_config.tamanio_pagina, fp);
			char* hex_string;
			hex_string = mem_hexstring(cont_pag.buffer, d_config.tamanio_pagina);
			log_info(logger, "Escribi en el archivo %d lo siguiente: %s", archivo_selec, hex_string);
			free(hex_string);
			send_ok(cliente_fd);
		}
	}
}

void liberar_bloques(uint32_t mate_id){
	for(int i=0; i<d_config.cantidad_archivos; i++){
		mate_file *archivo;
		archivo = list_get(tabla_arch, i);
		for(int j=0; j<d_config.tamanio_swap/d_config.tamanio_pagina; j++){
			bloque_archivo* bloques;
			bloques = list_get(archivo->bloques, j);
			if(bloques->mate_id==mate_id){
				log_info(logger, "Se borro la pagina %d del proceso %d en el bloque %d del archivo %d",bloques->numero_pagina, bloques->mate_id, j, i );
				bloques->mate_id=UINT32_MAX;
				bloques->numero_pagina=UINT32_MAX;
				bloques->posicion=UINT32_MAX;
			}
			if(bloques->mate_id==mate_id){
				i=d_config.cantidad_archivos;
			}
		}
	}
}

void liberar_bloque(arch_bloq bloq_borrar){
	mate_file* archivo;
	archivo = list_get(tabla_arch, bloq_borrar.n_arch);
	bloque_archivo* bloque;
	bloque = list_get(archivo->bloques, bloq_borrar.n_bloq);
	bloque->mate_id=UINT32_MAX;
	bloque->numero_pagina=UINT32_MAX;
	bloque->posicion=UINT32_MAX;
}


void free_bloque(bloque_archivo* bloque){
	free(bloque);
}

void free_archivo(mate_file* archivo){
	fclose(archivo->file);
	list_destroy_and_destroy_elements(archivo->bloques, (void*)free_bloque);
	free(archivo);
}

void free_memoria(){
	list_destroy_and_destroy_elements(tabla_arch, (void*)free_archivo);
	int i=0;
	while(d_config.archivos_swap[i]){
		free(d_config.archivos_swap[i]);
		i++;
	}

}

