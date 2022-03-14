#include "../include/memsignals.h"
#include"../../shared/include/loggers_config.h"
#include "../../shared/include/cliente.h"
#include<commons/temporal.h>
#include<commons/txt.h>
#include<commons/collections/list.h>
#include <pthread.h>
#include"../include/memfunctions.h"
#include"../include/paginacion.h"


void cerrarPrograma(){

	page_table* tabla;

	int tam_lista=list_size(lista_tabla_paginas);

	for(int i=0; i<tam_lista; i++){
		tabla=list_get(lista_tabla_paginas, i);
		log_info(logger, "PID: %d", tabla->mate_id);
		log_info(logger, "TLB HIT DEL CARPINCHO: %d", tabla->tlb_hit);
		log_info(logger, "TLB MISS DEL CARPINCHO: %d", tabla->tlb_miss);
		list_destroy_and_destroy_elements(tabla->tabla_paginas, (void*)eliminarPagina);
		free(tabla);
	}

	log_info(logger, "TLB HIT TOTALES: %d", TLB_HIT);
	log_info(logger, "TLB MISS TOTALES: %d", TLB_MISS);

	list_destroy_and_destroy_elements(tabla_marcos.marcos, (void*)eliminarMarco);

	pthread_mutex_destroy(mutex_tabla_marcos);
	free(mutex_tabla_marcos);

	pthread_mutex_destroy(mutex_TLB);
	free(mutex_TLB);

	pthread_mutex_destroy(mutex_swap);
	free(mutex_swap);

	pthread_mutex_destroy(mutex_busqueda_tabla);
	free(mutex_busqueda_tabla);

	pthread_mutex_destroy(mutex_lista_tabla_paginas);
	free(mutex_lista_tabla_paginas);


	log_destroy(logger);
	config_destroy(config);
	liberar_conexion(conexion_swamp);
	free(memoria);

	exit(0);
}


void eliminarMarco(marcos_memoria* marco){
	pthread_mutex_destroy(marco->mutex);
	free(marco->mutex);
	free(marco);
}

void eliminarPagina(pagina* page){
	free(page);
}

char* concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    // in real code you would check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

void dump(){
	char* time = temporal_get_string_time("%d-%m-%y_%H:%M:%S");
	char* file_name=string_new();
	string_append_with_format(&file_name, "%s/Dump_%s.tlb", mem_config.path_dump_tlb, time);

	log_debug(logger, "Cree el path: %s", file_name);

	FILE* dump_file;
	dump_file=txt_open_for_append(file_name);
	free(file_name);

	txt_write_in_file(dump_file, "---------------------------------------------------------\n");
	txt_write_in_file(dump_file, time);
	txt_write_in_file(dump_file, "\n");

	free(time);
	//mate_wait(mutex_TLB);
	int size=list_size(MEM_TLB.entrada_tlb);
	for(int i=0; i<size; i++){
		TLB* entrada=list_get(MEM_TLB.entrada_tlb, i);
		char* line=string_new();
		string_append_with_format(&line, "Entrada:%d ", i);
		if(entrada->isFree){

			string_append(&line, "Estado:Libre   Carpincho:- Pagina:- Marco:-\n");
		}
		else{
			string_append_with_format(&line, "Estado:Ocupado Carpincho:%d Pagina:%d Marco:%d\n", entrada->mate_id, entrada->pagina->numero_pagina, entrada->pagina->numero_marco);
		}

		txt_write_in_file(dump_file, line);
		free(line);
	}

	txt_write_in_file(dump_file, "---------------------------------------------------------\n");

	txt_close_file(dump_file);

	//mate_signal(mutex_TLB);

}

void limpiarEntrada(TLB* entrada){
	entrada->isFree=true;
	entrada->mate_id=UINT32_MAX;
	entrada->pagina=NULL;
}

void limpiarTLB(){

	mate_wait(mutex_TLB);
	list_map(MEM_TLB.entrada_tlb, (void*)limpiarEntrada);
	mate_signal(mutex_TLB);
}
