#include "../include/declaraciones.h"


void inicializar_carpincho(mate_inner_structure* carpincho){

	//carpincho->group_info = malloc(sizeof(mate_inner_structure)); HACER MALLOC ANTES

	carpincho->mate_id=0; // 1 a n

}

void inicializar_mate_config(mate_cfg* mate_config, t_config* config){

	mate_config->ip=config_get_string_value(config, "IP");
	mate_config->puerto_kernel = config_get_string_value(config, "PUERTO_KERNEL");
	mate_config->puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
	mate_config->destino=config_get_string_value(config, "DESTINO");


}
