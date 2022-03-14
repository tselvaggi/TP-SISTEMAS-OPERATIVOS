#ifndef SHARED_INCLUDE_LOGGERS_CONFIG_H_
#define SHARED_INCLUDE_LOGGERS_CONFIG_H_

#include "librerias.h"

extern t_log* logger;
extern t_config* config;

t_log* iniciar_logger(char *nombre_log, char *nombre_programa);
t_config* iniciar_config(char *path);

#endif /* SHARED_INCLUDE_LOGGERS_CONFIG_H_ */
