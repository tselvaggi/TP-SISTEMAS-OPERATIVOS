/*
 * kernel_lists.h
 *
 *  Created on: 24 oct. 2021
 *      Author: utnso
 */

#ifndef KERNEL_INCLUDE_LISTAS_ADMINISTRATIVAS_H_
#define KERNEL_INCLUDE_LISTAS_ADMINISTRATIVAS_H_

#include "../../shared/include/librerias.h"
#include "../../shared/include/estructuras.h"


t_list *semaforos;
t_list *carpinchos_new;
t_list *carpinchos_ready;
t_list *carpinchos_blocked;
t_list *carpinchos_suspended;
t_list *dispositivos_io;



#endif /* KERNEL_INCLUDE_LISTAS_ADMINISTRATIVAS_H_ */
