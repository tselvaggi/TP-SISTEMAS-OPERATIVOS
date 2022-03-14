#include "../include/declaraciones.h"


void inicializar_mem_config(mem_cfg* mem_config,char*direccion){

	config = iniciar_config(direccion);

	mem_config->ip = config_get_string_value(config, "IP");
	mem_config->puerto = config_get_string_value(config, "PUERTO");
	mem_config->puerto_swamp = config_get_string_value(config, "PUERTO_SWAMP");
	mem_config->tamanio = config_get_int_value(config, "TAMANIO");
	mem_config->tamanio_pagina = config_get_int_value(config, "TAMANIO_PAGINA");
	mem_config->tipo_asignacion = config_get_string_value(config, "TIPO_ASIGNACION");
	mem_config->marcos_por_carpincho = config_get_int_value(config, "MARCOS_POR_CARPINCHO");
	mem_config->algoritmo_reemplazo_mmu = config_get_string_value(config, "ALGORITMO_REEMPLAZO_MMU");
	mem_config->cantidad_entradas_tlb = config_get_int_value(config, "CANTIDAD_ENTRADAS_TLB");
	mem_config->algoritmo_reemplazo_tlb = config_get_string_value(config, "ALGORITMO_REEMPLAZO_TLB");
	mem_config->retardo_acierto_tlb = config_get_int_value(config, "RETARDO_ACIERTO_TLB");
	mem_config->retardo_fallo_tlb = config_get_int_value(config, "RETARDO_FALLO_TLB");
	mem_config->path_dump_tlb = config_get_string_value(config, "PATH_DUMP_TLB");
}
