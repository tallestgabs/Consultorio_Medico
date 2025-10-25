#include "stdio.h"
#include "unistd.h"
#include "stdlib.h"
#include "pthread.h"

#define MEDICOS 2       //numero de medicos

typedef struct{
    int* id;
    // booleanos
    int vai_esperar;
    int tem_dinheiro;

} PacienteArgs;

void * medico(void *arg){
    int i = *((int *) arg);  // id
    printf("Sou o medico %d e fui inicializado com sucesso\n", i);
}


void * paciente(void *arg){

    PacienteArgs *pArgs = (PacienteArgs *) (arg);  // converte void para o tipo da estrutura
    int id = *(pArgs->id);
    int vai_esperar = pArgs->vai_esperar;
    int tem_dinheiro = pArgs->tem_dinheiro;

    free(pArgs);

    printf("Sou o paciente %d e fui inicializado com sucesso -> Vai esperar? %d -> Tem dinheiro? %d\n", id, vai_esperar, tem_dinheiro);
}


int main() {
	int i;
    int* id;
    int vai_esperar;
    int tem_dinheiro;

    srand(time(NULL));  // inicializa gerador aleatorio
    int randomNumber;
    int numero_pacientes = rand() % 200;  // quantidade aleatoria de pacientes entre 0 a 200

	pthread_t m[MEDICOS], p[numero_pacientes];

    /* criando medicos */
    for (i = 0; i < MEDICOS ; i++) {
	    id = (int *) malloc(sizeof(int));
        *id = i;

		if(pthread_create(&m[i], NULL, medico, (void *) (id)) != 0){
            perror("Falha ao criar Thread de Medico!");
            exit(EXIT_FAILURE);
        }
	}


	 /* criando pacientes */
	for (i = 0; i< numero_pacientes; i++) {
	    id = (int *) malloc(sizeof(int));
        *id = i;

        // comecam com sim (vai esperar e tem dinheiro)
        vai_esperar = 1;
        tem_dinheiro = 1;

        //calcula se paciente vai esperar na fila
        randomNumber = rand() % 3;  // gera entre 0 a 2     ->  1/3 = 33%
        if(randomNumber == 2){  // escolhi um dos numeros para decidir se ele nao vai esperar
            vai_esperar = 0;
        }

        //calcula se paciente tem dinheiro
        randomNumber = rand() % 20;  // gera entre 0 a 19   ->  1/20 = 5%
        if(randomNumber == 4){  // escolhi um dos numeros para nao ter tinheiro
            tem_dinheiro = 0;
        }

        PacienteArgs *pacienteArgs = (PacienteArgs *) malloc(sizeof(PacienteArgs));
        if(pacienteArgs == NULL){
            perror("Falha ao alocar memoria para pacienteArgs!");
            exit(EXIT_FAILURE);
        }
        pacienteArgs->id = id;
        pacienteArgs->vai_esperar = vai_esperar;
        pacienteArgs->tem_dinheiro = tem_dinheiro;
        
		if(pthread_create(&p[i], NULL, paciente, (void *) (pacienteArgs)) != 0){
            perror("Falha ao criar Thread de Paciente!");
            exit(EXIT_FAILURE);
        }
	}

    for(i = 0; i < MEDICOS; i++){
        pthread_join(m[i],NULL);
    }
    for(i = 0; i < numero_pacientes; i++){
	    pthread_join(p[i],NULL);
    }

	return 0;
}