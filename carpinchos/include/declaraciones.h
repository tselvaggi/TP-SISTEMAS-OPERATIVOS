#ifndef CARPINCHOS_INCLUDE_DECLARACIONES_H_
#define CARPINCHOS_INCLUDE_DECLARACIONES_H_

#include <unistd.h>
#include "../../shared/include/loggers_config.h"

#define CARPINCHO ((mate_inner_structure*)carpincho->group_info)

typedef struct mate_cfg{

	char* ip;
	char* puerto_kernel;
	char* puerto_memoria;
	char* destino;

}mate_cfg;

typedef struct mate_inner_structure
{
	int socket;
	uint32_t mate_id;
	t_log* mate_logger;
	t_config* mate_config;
	mate_cfg structure_cfg;
}mate_inner_structure;


void inicializar_carpincho(mate_inner_structure* carpincho);
void inicializar_mate_config(mate_cfg* mate_config, t_config* config);

#endif /* CARPINCHOS_INCLUDE_DECLARACIONES_H_ */
