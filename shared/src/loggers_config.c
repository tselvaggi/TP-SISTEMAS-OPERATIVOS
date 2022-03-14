#include "../include/loggers_config.h"
#include "../include/librerias.h"



t_log* iniciar_logger(char *nombre_log, char *nombre_programa){

	t_log* nuevo_logger;
	char* path_logger=string_new();
	char* time = temporal_get_string_time("%d-%m-%y_%H:%M:%S");
	string_append_with_format(&path_logger, "log/%s %s.log", nombre_log, time);

	if((nuevo_logger = log_create(path_logger, nombre_programa, 1, LOG_LEVEL_INFO))==NULL){
		printf("No se pudo crear el logger");
		exit(1);
	}
	else{
		free(path_logger);
		free(time);
		return nuevo_logger;
	}
}

t_config* iniciar_config(char *path)
{
	t_config* nuevo_config;
	if((nuevo_config = config_create(path)) == NULL){
		printf("No pude crear el config");
		exit(2);
	}
	else{
	return nuevo_config;
	}
}
