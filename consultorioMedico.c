#include "stdio.h"
#include "unistd.h"
#include "stdlib.h"
#include "stdbool.h"
#include "pthread.h"

#define MEDICOS 2       //numero de medicos
#define PACIENTES 150   //numero de pacientes

int main() {

	pthread_t m[MEDICOS], p[PACIENTES];
	int i;
    int* id;        
    bool* vai_esperar = true;
    bool* tem_dinheiro = true;

    srand(time(NULL));  // inicializa gerador aleatorio
    int randomNumber;

    /* criando medicos */
    for (i = 0; i < MEDICOS ; i++) {
	    id = (int *) malloc(sizeof(int));
        *id = i;
		pthread_create(&m[i], NULL, medico, (void *) (id));
	}

	 /* criando pacientes */
	for (i = 0; i< PACIENTES; i++) {
	    id = (int *) malloc(sizeof(int));
        *id = i;

        //calcula se paciente vai esperar na fila
        randomNumber = rand() % 3;  // gera entre 0 a 2
        if(randomNumber == 2){
            vai_esperar = false;
        }

        //calcula se paciente tem dinheiro
        randomNumber = rand() % 20;  // gera entre 0 a 20
        if(randomNumber == 4){
            tem_dinheiro = false;
        }
        
		pthread_create(&p[i], NULL, paciente, (void *) (id, vai_esperar, tem_dinheiro));
	}
	pthread_join(p[0],NULL);
	return 0;
}

void * medico(void *arg){

}

void * paciente(void *arg){

}