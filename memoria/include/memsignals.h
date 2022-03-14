#ifndef MEMORIA_INCLUDE_MEMSIGNALS_H_
#define MEMORIA_INCLUDE_MEMSIGNALS_H_

#include "declaraciones.h"


void cerrarPrograma();
void eliminarPagina(pagina* page);
void eliminarMarco(marcos_memoria* marco);

void dump();

void limpiarEntrada(TLB* entrada);
void limpiarTLB();

char* concat(const char *s1, const char *s2);


#endif /* MEMORIA_INCLUDE_MEMSIGNALS_H_ */
