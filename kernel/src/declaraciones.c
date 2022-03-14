#include "../include/declaraciones.h"

void inicializar_kernel_config(kernel_cfg* kernel_config,char *direccion){

	config = iniciar_config(direccion);

	int tamano;
	kernel_config->ip = config_get_string_value(config, "IP");
	kernel_config->puerto = config_get_string_value(config, "PUERTO");
	kernel_config->puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
	kernel_config->algoritmo_planificacion = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
	kernel_config->dispositivos_io = config_get_array_value(config, "DISPOSITIVOS_IO");
	kernel_config->est_inicial = config_get_double_value(config, "ESTIMACION_INICIAL");
	kernel_config->alfa = config_get_double_value(config, "ALFA");
	kernel_config->duraciones_io = config_get_array_value(config, "DURACIONES_IO");
	kernel_config->grado_multiprog = config_get_int_value(config, "GRADO_MULTIPROGRAMACION");
	kernel_config->grado_multiproc = config_get_int_value(config, "GRADO_MULTIPROCESAMIENTO");
	kernel_config->tiempo_deadlock = config_get_int_value(config, "TIEMPO_DEADLOCK");

	for(int i = 0;kernel_config->dispositivos_io[i]!=(NULL);i++){
		io_proc *dispositivo = malloc(sizeof(io_proc));
		dispositivo->nombre = kernel_config->dispositivos_io[i];
		dispositivo->duracion = atof(kernel_config->duraciones_io[i]);
		sem_init(&dispositivo->sem_io, 0, 1);
		list_add(dispositivos_io,dispositivo);
	}
}


