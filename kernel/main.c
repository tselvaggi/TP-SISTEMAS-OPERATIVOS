#include "include/main.h"


int pid=0;
sem_t sem_colalp_ready;

sem_t sem_colalp;
sem_t sem_colablocked;
sem_t sem_colasus;
sem_t sem_colacp;
sem_t sem_colaexec;

sem_t sem_colacp_vacia;
sem_t sem_colalp_vacia;
sem_t sem_admin_med;
sem_t sem_pid;
sem_t sem_semaforos; //Son los semaforos de biblioteca
t_list *carpinchos_exec;

int main(int argc, char *argv[]) {
	pthread_t planificador_largo;
	pthread_t planificador_mediano;
	pthread_t deadlock;

	carpinchos_new = list_create();
	carpinchos_ready = list_create();
	carpinchos_blocked = list_create();
	carpinchos_suspended = list_create();
	carpinchos_exec = list_create();
	dispositivos_io = list_create();
	semaforos = list_create();

	logger = iniciar_logger("kernel", "KERNEL");
	inicializar_kernel_config(&kernel_config,argv[1]);

	sem_init(&sem_colalp_ready, 0,kernel_config.grado_multiprog);
	sem_init(&sem_colalp, 0, 1);
	sem_init(&sem_colablocked, 0, 1);
	sem_init(&sem_colaexec, 0, 1);
	sem_init(&sem_colasus, 0, 1);
	sem_init(&sem_colacp, 0, 1);
	sem_init(&sem_colalp_vacia, 0, 0);
	sem_init(&sem_colacp_vacia, 0, 0);
	sem_init(&sem_admin_med, 0, 0);
	sem_init(&sem_pid, 0, 1);
	sem_init(&sem_semaforos, 0, 1);

	pthread_create(&planificador_largo, NULL, administrador_largo,NULL);
	pthread_detach(planificador_largo);
	init_adm_corto();

	pthread_create(&planificador_mediano, NULL, administrador_mediano,NULL);
	pthread_detach(planificador_mediano);

	pthread_create(&deadlock, NULL, admin_deadlock,NULL);
	pthread_detach(deadlock);


	int server_fd = iniciar_servidor(kernel_config.ip, kernel_config.puerto);
	while(1){
		pthread_t hilo_cliente;
		log_info(logger, "Servidor listo para recibir un cliente");
		int cliente_fd = esperar_cliente(server_fd);
		void* conexion=malloc(sizeof(int));
		memcpy(conexion, &cliente_fd, sizeof(int));
		log_info(logger,"ESTE ES EL PROCESO %d",cliente_fd);
		pthread_create(&hilo_cliente, NULL, &atender_cliente, conexion);
		pthread_detach(hilo_cliente);

	}

	return EXIT_SUCCESS;
}

void * admin_deadlock(){
	t_list *proc_deadlock;
	kernel_proc * proc;
	int cumple=0;// Lo uso porque el list_any_satisfy no me corta cuando encuentra al primero y sigue verificando
	proc_deadlock = list_create();
	bool detectar_deadlock(void *proceso){
		log_info(logger,"COMIENZO DETECCION DE DEADLOCK");
		//log_info(logger,"ENTRO AL VERIFICADOR CON id %d ",((kernel_proc*)proceso)->id);
		if(((kernel_proc*)proceso)->recurso_tomado == NULL || ((kernel_proc*)proceso)->recurso_querido == NULL)
			return 0;
		char *tomado = ((kernel_proc*)proceso)->recurso_tomado->name;
		char *querido = ((kernel_proc*)proceso)->recurso_querido->name;
		bool esta_en_deadlock(void *proceso,char *tomado,char*querido){
			if(cumple>0)
				return 1;
			//log_info(logger,"PROCESO %d TIENE %s Y QUIERE %s",((kernel_proc*)proceso)->id,tomado,querido);
			if(tomado == NULL || querido == NULL)
				return 0;

			kernel_proc *proc2;
			list_add(proc_deadlock,proceso);

			bool loTiene(void *proc){
				return ((kernel_proc*)proc)->recurso_tomado->name == querido;
			}

			bool _is_the_one(void *p) {
					return p == proc2;
				}

			proc2 = list_find(carpinchos_blocked,loTiene);

			if(proc2 == NULL || list_any_satisfy(proc_deadlock,_is_the_one)){
				list_clean(proc_deadlock);
				return 0;
			}
			if(!strcmp(proc2->recurso_querido->name,tomado)){
				list_add(proc_deadlock,proc2);
				cumple = 1;
				return 1;
			}
			else{
				return esta_en_deadlock(proc2,tomado,proc2->recurso_querido->name);
			}
		}
		return esta_en_deadlock(proceso,tomado,querido);


	}
	while(1){
		if(list_size(carpinchos_blocked)>1){
			if(list_any_satisfy(carpinchos_blocked,detectar_deadlock)){
				kernel_proc *proceso_aux;
				cumple = 0;
				log_info(logger,"DEAAAAAAAAAAAAAAAAAAAAAAAAADLOOOOOOOOOOOOOOOOOOOOOOOCK");
				for(int i = 0;i < list_size(proc_deadlock);i++){
					proceso_aux = list_get(proc_deadlock,i);
					log_info(logger,"Proceso %d bloquedo por DEADLOCK",proceso_aux->id);
				}
				list_sort(proc_deadlock,mayor_id);
				proc = list_get(proc_deadlock,0);
				int confirm = 0;
				send(proc->conexion,&confirm,sizeof(int),0);
				liberar_conexion(proc->conexion);
				liberar_conexion(proc->conexion_memoria);

				proc->recurso_tomado->value++;
				log_info(logger,"Se destruira el proceso %d para solucionar el deadlock",proc->id);
				if(!list_is_empty(proc->recurso_tomado->procesos)){
					proceso_aux = list_remove(proc->recurso_tomado->procesos,0);
					log_info(logger,"SE DESBLOQUEO EL PROCESO %d",proceso_aux->id);
					set_ready(proceso_aux);
				}
				log_info(logger,"Se destruira el proceso %d para solucionar el deadlock",proc->id);
				destruirProceso(proc);
				list_clean(proc_deadlock);
			}
		}
		usleep(kernel_config.tiempo_deadlock*1000);

	}
}

bool mayor_id(void * process,void * process2){
	int carpincho1 = ((kernel_proc*)process)->id;
	int carpincho2 = ((kernel_proc*)process2)->id;
	return carpincho1 > carpincho2;
}

void* administrador_mediano(){
	kernel_proc * process;
	while(1){

		sem_wait(&sem_admin_med);
		sem_wait(&sem_colaexec);
		//log_info(logger,"CARPINCHOS EXEC %d",list_size(carpinchos_exec));
		sem_post(&sem_colaexec);
		//log_info(logger,"CARPINCHOS LISTOS %d",list_size(carpinchos_ready));
		//log_info(logger,"CARPINCHOS BLOQUEADOS %d",list_size(carpinchos_blocked));
		if((list_size(carpinchos_blocked)+list_size(carpinchos_exec)+list_size(carpinchos_ready))< kernel_config.grado_multiprog){
			if(list_any_satisfy(carpinchos_suspended,estaReady)){
				sem_wait(&sem_colasus);
				process = list_remove_by_condition(carpinchos_suspended,estaReady);
				sem_post(&sem_colasus);
				aniadir_a_cola_corto_plazo(process);
				sem_post(&sem_colacp_vacia);
			}
			else if(list_size(carpinchos_new)){
				sem_post(&sem_colalp_ready);
			}
		}
		else if(list_size(carpinchos_blocked) == kernel_config.grado_multiprog && (list_any_satisfy(carpinchos_suspended,estaReady)||list_size(carpinchos_new))) //Habria que cambiar algo en el if para que tome a los que estan en exe
		{
			kernel_proc *susp;
			log_info(logger,"CARPINCHOS BLOQUEADOS %d",list_size(carpinchos_blocked));
			sem_wait(&sem_colablocked);
			susp = list_remove(carpinchos_blocked,0);
			set_suspended(susp);
			sem_post(&sem_colablocked);
			log_info(logger,"Se suspendio el proceso %d",susp->id);
			if(list_any_satisfy(carpinchos_suspended,estaReady)){
				sem_wait(&sem_colasus);
				process = list_remove_by_condition(carpinchos_suspended,estaReady);
				sem_post(&sem_colasus);
				aniadir_a_cola_corto_plazo(process);
				log_info(logger,"SOY EL CARPINCHO %d Y ESTOY LISTO PARA EJECUTAR %d",process->id,process->estado);
				sem_post(&sem_colacp_vacia);
			}
			else if(list_size(carpinchos_new)){
				sem_post(&sem_colalp_ready);
			}
		}
	}
}

bool estaReady(void * process){
	return ((kernel_proc*)process)->estado == READY;
}

kernel_proc * quitar_de_lista(t_list * lista,void *process){
	bool _is_the_one(void *p) {
		return p == process;
	}
	return list_remove_by_condition(lista,_is_the_one);
}

void* administrador_largo(){
	kernel_proc *proc;
	while(1){
		sem_wait(&sem_colalp_vacia);
		sem_wait(&sem_colalp_ready);
		sem_wait(&sem_colalp);
		proc = list_remove(carpinchos_new,0);
		sem_post(&sem_colalp);
		set_ready(proc);
		log_info(logger, "Procesos en new %d", list_size(carpinchos_new));
		log_info(logger, "Procesos en ready %d", list_size(carpinchos_ready));
	}
}

float prox_tiempo_exec(kernel_proc * process){
	float alfa = kernel_config.alfa;
	return (process->prev_exec)*alfa + (process->prev_expec_exec)*(1-alfa);
}

bool prox_rafaga_menor(void * process,void * process2){
	float estimado_primero = prox_tiempo_exec(process);
	float estimado_segundo = prox_tiempo_exec(process2);
	float espera_primero = time_dif(((kernel_proc *)process)->tiempo_en_ready);
	float espera_segundo = time_dif(((kernel_proc *)process2)->tiempo_en_ready);
	if(!strcmp(kernel_config.algoritmo_planificacion,"SJF"))
		return estimado_primero < estimado_segundo;
	else
		return (espera_primero + estimado_primero)/estimado_primero > (espera_segundo + estimado_segundo)/estimado_segundo;
}

void *prox_proc_exec(){
	list_sort(carpinchos_ready,prox_rafaga_menor);
	return list_remove(carpinchos_ready,0);

}

bool estaSuspended(void * process){
	bool _is_the_one(void *p) {
			return p == process;
		}
	return list_any_satisfy(carpinchos_suspended, _is_the_one);
}

void* administrador_corto(void* numero){
	kernel_proc *proceso;
	while(1){
		sem_wait(&sem_colacp_vacia); //CUANDO LLEGA UN CARPINCHO A READY SE SUMA 1
		sem_wait(&sem_colacp); // MUTEX VAR GLOBAL
		proceso = prox_proc_exec();
		log_info(logger,"Ejecuto en procesador %d",*(int*)numero);
		set_exec(proceso);
		sem_post(&sem_colacp); //MUTEX VAR GLOBAL
		sem_wait(&proceso->sem_blocked); //LIBERA EL PROCESADOR
	}
	free(numero);
}


void init_adm_corto(){

	int cant_proc = kernel_config.grado_multiproc;
	int *num_proc;
	for(int i = 0;i < cant_proc; i++){
		num_proc = malloc(sizeof(int));
		*num_proc = i;
		pthread_t procesadores;
		pthread_create(&procesadores, NULL, administrador_corto,num_proc);
		pthread_detach(procesadores);
		log_info(logger, "Procesador %d iniciado", i+1);
	}
}

kernel_sem *encontrar_semaforo(char *sem_name) {
	int _is_the_one(kernel_sem *p) {
		return string_equals_ignore_case(p->name, sem_name);
	}

	return list_find(semaforos, (void*) _is_the_one);
}

kernel_sem *quitar_semaforo(char *sem_name) {
	int _is_the_one(kernel_sem *p) {
		return string_equals_ignore_case(p->name, sem_name);
	}

	return list_remove_by_condition(semaforos, (void*) _is_the_one);
}

void* atender_cliente(void* cliente_fd){

	int cliente_cast = *((int*)cliente_fd);
	free(cliente_fd);
	char* sem_name, *io_name;
	uint32_t mate_id;
	kernel_sem *sem;
	struct timespec inicio_ejec={0,0};
	kernel_proc *proceso;
	int conexion_memoria;
	conexion_memoria = crear_conexion(kernel_config.ip, kernel_config.puerto_memoria);
	log_info(logger, "Me conecte a la memoria %d", conexion_memoria);
	log_info(logger, "Voy a escuchar al cliente %d", cliente_cast);
	int cod_op;

	while(1){

		cod_op = recibir_operacion(cliente_cast);
		switch (cod_op) {
		case INIT:
			recv_init(mate_id, cliente_cast);
			proceso = init_proceso(cliente_cast,conexion_memoria);
			log_info(logger,"VOY A ENVIAR A MEMORIA EL PROCESO %d %d",proceso->id,proceso->conexion_memoria);
			send_init_memoria(proceso->id, proceso->conexion_memoria);
			recv_exec_memoria(proceso->conexion_memoria);
			sem_post(&sem_admin_med);
			sem_wait(&proceso->sem_exec);
			log_info(logger, "Proceso %d listo para ejecutar", proceso->id);
			send_exec(proceso->id, cliente_cast);
			clock_gettime(CLOCK_MONOTONIC, &inicio_ejec);
			break;
		case SEM_INIT:
			sem = malloc(sizeof(kernel_sem));
			recv_sem_init(sem, cliente_cast);
			if(encontrar_semaforo(sem->name)==NULL){
				sem->procesos = list_create();
				sem_wait(&sem_semaforos);
				list_add(semaforos,sem);
				sem_post(&sem_semaforos);
			}
			else
			{
				log_info(logger,"Ya existe ese semaforo");
				free(sem->name);
				free(sem);
			}
			send_ok(cliente_cast);
			break;
		case SEM_WAIT:
			sem_name = recv_sem_operation(cliente_cast);
			log_info(logger, "Se recibio el semaforo %s", sem_name);
			sem_wait(&sem_semaforos);
			sem = encontrar_semaforo(sem_name);
			sem_post(&sem_semaforos);
			if(sem==NULL)
				log_info(logger, "No existe el semaforo %s", sem_name);
			else{
				sem->value--;
				log_info(logger, "Se resto 1 al valor del semaforo: %d", sem->value);
				if(sem->value < 0){
					proceso->recurso_querido = sem;
					list_add(sem->procesos,proceso);
					set_blocked(proceso);
					proceso->prev_exec = time_dif(inicio_ejec);
					log_info(logger, "Se bloqueo el proceso %d en el SEM WAIT",proceso->id);
					sem_wait(&proceso->sem_exec);
					clock_gettime(CLOCK_MONOTONIC, &inicio_ejec);
				}
				else
					proceso->recurso_tomado = sem;
			}
			free(sem_name);
			send_ok(cliente_cast);
			break;
		case SEM_POST:
			sem_name = recv_sem_operation(cliente_cast);
			log_info(logger, "Se recibio el semaforo %s", sem_name);
			sem_wait(&sem_semaforos);
			sem = encontrar_semaforo(sem_name);
			sem_post(&sem_semaforos);
			if(sem==NULL)
				log_info(logger, "No existe el semaforo %s", sem_name);
			else{
				if(sem == proceso->recurso_tomado)
					proceso->recurso_tomado = NULL;
				sem->value++;
				if(sem->value <= 0){
					kernel_proc *proceso_aux;
					if(!list_is_empty(sem->procesos)){
						proceso_aux = list_remove(sem->procesos,0);
						set_ready(proceso_aux);
					}
				}
				log_info(logger, "Se sumo 1 al valor del semaforo: %d", sem->value);
			}
			free(sem_name);
			send_ok(cliente_cast);
			break;
		case SEM_DESTROY:
			sem_name = recv_sem_operation(cliente_cast);
			log_info(logger, "Intentando destruir el semaforo %s", sem_name);
			sem_wait(&sem_semaforos);
			sem = quitar_semaforo(sem_name);
			sem_post(&sem_semaforos);
			if(sem==NULL)
				log_info(logger,"No existe el semaforo %s",sem_name);
			else{
				log_info(logger, "SE LIBERO EL SEMAFORO %s", sem->name);
				free(sem->name);
				list_destroy(sem->procesos);
				free(sem);
			}
			free(sem_name);
			send_ok(cliente_cast);
			break;
		case CALL_IO:
			io_name = recv_io_usage(cliente_cast);
			log_info(logger, "RECIBI EL IO DE %s", io_name);
			io_proc *io = get_io_from_name(io_name);
			proceso->prev_exec = time_dif(inicio_ejec);
			log_info(logger,"Ejecuto durante:%d el proceso %d",time_dif(inicio_ejec),proceso->id);
			set_blocked(proceso);
			if(io != NULL){
				sem_wait(&io->sem_io);
				log_info(logger, "HACIENDO IO DE %s", io_name);
				usleep(io->duracion*1000);
				sem_post(&io->sem_io);
			}
			set_ready(proceso);
			log_info(logger, "PROCESO %d TERMINO EL IO DE %s, ESPERANDO PARA EJECUTAR", proceso->id,io_name);
			sem_wait(&proceso->sem_exec);
			clock_gettime(CLOCK_MONOTONIC, &inicio_ejec);
			free(io_name);
			send_ok(cliente_cast);
			break;
		case MEMALLOC:
		case MEMFREE:
		case MEMREAD:
		case MEMWRITE:
			log_info(logger, "Se detecto una operación de memoria, se copiara y se enviara la informacion a ese modulo...");
			pasamanos_kernel_memoria(cliente_cast, conexion_memoria, cod_op);
			int confirm=recibir_operacion(conexion_memoria);
			log_info(logger, "Voy a esperar la respuesta de memoria");
			log_info(logger, "Voy a pasar la confirm %d", confirm);
			pasamanos_kernel_memoria(conexion_memoria, cliente_cast, confirm);
			log_info(logger, "Recibi y envie la respuesta de memoria al carpincho");
			break;
		case -1:
			log_info(logger,"Finalizo correctamente el proceso %d",proceso->id);
			log_info(logger,"Ejecuto durante:%d",time_dif(inicio_ejec));
			sem_post(&proceso->sem_blocked);
			destruirProceso(proceso);
			log_info(logger, "el cliente se desconecto. Terminanda la conexion");
			liberar_conexion(conexion_memoria);
			sem_post(&sem_admin_med);
			return NULL;
		default:
			log_info(logger, "OPERACION: %d", cod_op);
			log_warning(logger, "Operacion desconocida. No quieras meter la pata");
			break;
			}
	}
}

void destruirProceso(kernel_proc *process){
	log_info(logger, "SE VA A DESTRUIR EL PROCESO %d", process->id);
	sem_wait(&sem_colaexec);
	quitar_de_lista(carpinchos_exec,process);
	sem_post(&sem_colaexec);
	quitar_de_lista(carpinchos_new,process);
	sem_wait(&sem_colablocked);
	quitar_de_lista(carpinchos_blocked,process);
	sem_post(&sem_colablocked);
	sem_wait(&sem_colasus);
	quitar_de_lista(carpinchos_suspended,process);
	sem_post(&sem_colasus);
	quitar_de_lista(carpinchos_ready,process);
	void quitar_de_sem(void * sem){
		bool _is_the_one(void *p) {
			return p == process;
		}
		list_remove_by_condition(((kernel_sem*)sem)->procesos,_is_the_one);
	}
	sem_wait(&sem_semaforos);
	list_iterate(semaforos,quitar_de_sem);
	sem_post(&sem_semaforos);
	free(process);
}

io_proc *get_io_from_name(char *io){
	io_proc *io_p;
	for(int i = 0;i< list_size(dispositivos_io);i++){
		io_p = list_get(dispositivos_io,i);
		if(!strcmp(io_p->nombre,io)){
			return io_p;
		}
	}
	log_info(logger, "No existe ese dispositivo");
	return NULL;

}

void set_ready(kernel_proc *process){
	process->estado=READY;
	clock_gettime(CLOCK_MONOTONIC, &process->tiempo_en_ready);
	if(!estaSuspended(process)){
		sem_wait(&sem_colablocked);
		quitar_de_lista(carpinchos_blocked,process);
		sem_post(&sem_colablocked);
		aniadir_a_cola_corto_plazo(process);
		sem_post(&sem_colacp_vacia);
	}
	else{
		log_info(logger,"PASO A READY/SUSPENDED EL PROCESO %d",process->id);
	}
}

void set_suspended(kernel_proc *process){
	sem_wait(&sem_colasus);
	list_add(carpinchos_suspended,process);
	sem_post(&sem_colasus);
	send_suspended_memoria(process->id, process->conexion_memoria);

}

void set_exec(kernel_proc *process){
	process->estado=EXEC;
	sem_wait(&sem_colaexec);
	list_add(carpinchos_exec,process);
	sem_post(&sem_colaexec);
	sem_post(&sem_admin_med);
	sem_post(&process->sem_exec);
}


kernel_proc *init_proceso(int cliente_cast,int conexion_memoria){
	kernel_proc * proc = (kernel_proc*)malloc(sizeof(kernel_proc));
	sem_init(&proc->sem_exec, 0, 0);
	sem_init(&proc->sem_blocked, 0, 0);
	sem_wait(&sem_pid);
	pid++;
	proc->id = pid;
	log_info(logger, "Se asigno el id %d al carpincho", proc->id);
	sem_post(&sem_pid);
	proc->recurso_querido = NULL;
	proc->recurso_tomado = NULL;
	proc->conexion_memoria = conexion_memoria;
	proc->conexion = cliente_cast;
	proc->prev_exec = 0;
	proc->prev_expec_exec = kernel_config.est_inicial;
	log_info(logger, "Creando estructura del proceso %d",proc->id);
	aniadir_a_cola_largo_plazo(proc);
	sem_post(&sem_admin_med);

	return proc;
}

void set_blocked(kernel_proc *process){
	process->estado=BLOCKED;
	sem_wait(&sem_colaexec);
	quitar_de_lista(carpinchos_exec,process);
	sem_post(&sem_colaexec);
	sem_wait(&sem_colablocked);
	list_add_in_index(carpinchos_blocked,0,process);
	sem_post(&sem_colablocked);
	sem_post(&sem_admin_med);
	sem_post(&process->sem_blocked);
}

void aniadir_a_cola_largo_plazo(kernel_proc *process){
	sem_wait(&sem_colalp);
	list_add(carpinchos_new,process);
	sem_post(&sem_colalp);
	sem_post(&sem_colalp_vacia);
}

void aniadir_a_cola_corto_plazo(kernel_proc *process){
	sem_wait(&sem_colacp);
	list_add(carpinchos_ready,process);
	sem_post(&sem_colacp);
	log_info(logger, "Se paso un proceso a la lista de ready %d",process->id);
}

int time_dif(struct timespec inicio){
	int tiempo = 0;
	struct timespec fin_ejec={0,0};
	clock_gettime(CLOCK_MONOTONIC, &fin_ejec);
	tiempo = (fin_ejec.tv_sec - inicio.tv_sec)*1000;
	tiempo += (fin_ejec.tv_nsec - inicio.tv_nsec)*1.0e-6;
	return tiempo;
}
