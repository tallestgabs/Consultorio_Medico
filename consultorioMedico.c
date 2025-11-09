#include "stdio.h"
#include "unistd.h"
#include "stdlib.h"
#include "pthread.h"
#include "semaphore.h"

#define MEDICOS 2       //numero de medicos
#define ASSENTOS 10      //numero de assentos disponiveis

pthread_cond_t medico_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t paciente_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t recepcionista_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t atendimento_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t semaforo_assentos;

int quantidadePacientes = 0;
int pessoas = 0;
int lotado = 0;

typedef struct{
    int* id;
    int vai_esperar;
    int tem_dinheiro;

} PacienteArgs;

void * medico(void *arg){
    int i = *((int *) arg);     // id
    while(1){
        pthread_mutex_lock(&mutex);
        while(quantidadePacientes == 0){     // enquanto nao tem paciente, dorme
            pthread_cond_wait(&medico_cond, &mutex);
        }
        //acorda
        quantidadePacientes--;

        pthread_cond_signal(&paciente_cond);     // chama um dos pacientes
        printf("Medico %d: Proximo paciente!\n", i);
        pthread_mutex_unlock(&mutex);
        sleep(5);     // realiza a consulta
    }
}


void * paciente(void *arg){

    PacienteArgs *pArgs = (PacienteArgs *) (arg);  // converte void para o tipo da estrutura
    int id = *(pArgs->id);
    int vai_esperar = pArgs->vai_esperar;
    int tem_dinheiro = pArgs->tem_dinheiro;

    free(pArgs);

    pthread_mutex_lock(&mutex);
    pessoas++; 
    pthread_cond_signal(&recepcionista_cond);       // acorda recepcionista

    //espera atendimento
    pthread_cond_wait(&atendimento_cond, &mutex);
    printf("Paciente %d: Fui atendido pelo recepcionista\n", id);
                  
    if(tem_dinheiro == 0){
        printf("Paciente %d: Desculpe, esqueci o dinheiro em casa, vou tentar voltar amanha\n", id);
        pessoas--;
        pthread_mutex_unlock(&mutex);
        return NULL;
    }
    if(lotado != 0){  // esta lotado
        if(vai_esperar == 0){  // nao vai esperar
            printf("Paciente %d: Essa fila toda? aff eu que nao vou esperar!\n", id);
            pessoas--;
            pthread_mutex_unlock(&mutex);
            return NULL;
        }
        printf("Paciente %d: Que fila ein... vou ter que esperar, fazer o que.\n", id);
    }

    printf("Paciente %d: Pagamento conluido!\n", id);

    pessoas--;

    pthread_mutex_unlock(&mutex);

    printf("Paciente %d: Tentando pegar um assento...\n", id);
    sem_wait(&semaforo_assentos);                    // pega assento
    printf("Paciente %d: Consegui um assento. \n", id);

    pthread_mutex_lock(&mutex);
    quantidadePacientes++;
    pthread_cond_signal(&medico_cond);              // acorda um dos medicos
    pthread_cond_wait(&paciente_cond, &mutex);      // dorme esperando o medico chamar
    // medico chamou
    printf("Paciente %d: O medico me chamou.\n", id);
    sem_post(&semaforo_assentos);                    // libera assento

    pthread_mutex_unlock(&mutex);

    printf("Paciente %d consultando com o medico\n", id);
    sleep(2);
    printf("Paciente %d terminou a consulta e foi para casa\n", id);
    return NULL;
}

void * recepcionista(void *arg){
    while(1){
        pthread_mutex_lock(&mutex);
        while(pessoas == 0){    // se nao tiver pessoas, dorme
            pthread_cond_wait(&recepcionista_cond, &mutex);
        }
        // pessoa acorda recepcionista
        printf("Recepcionista: Ola, seja bem vindo(a), documentos por favor\n");
        printf("Recepcionista: Analisando os documentos da pessoa\n");
        printf("Recepcionista: Por favor realize o pagamento no valor de RS150,00 para o nosso CNPJ.\n");

        //confere se assentos estao lotados
        int valor_assentos;
        sem_getvalue(&semaforo_assentos, &valor_assentos);

        if(valor_assentos != 0){    // tem assentos
            lotado = 0;
            printf("Temos assentos disponiveis\n");
        }
        else{
            lotado = 1;
            printf("Infelizmente estamos sem vagas nos assentos, porem se alguem sair voce pode tentar pegar.\n");
        }

        pthread_cond_signal(&atendimento_cond);   // avisa que o atendimento acabou

        pthread_mutex_unlock(&mutex);
        
        sleep(1);
    }
    

}


int main() {
	int i;
    int* id;
    int vai_esperar;
    int tem_dinheiro;

    // inicializa semaforo
    sem_init(&semaforo_assentos, 0, ASSENTOS);

    srand(time(NULL));  // inicializa gerador aleatorio
    int randomNumber;
    int numero_pacientes = rand() % 50;  // quantidade aleatoria de pacientes entre 0 a 49

    printf("Quantidade de pacientes hoje: %d\n", numero_pacientes);

	pthread_t m[MEDICOS], p[numero_pacientes], r;

    pthread_create(&r, NULL, recepcionista, NULL);  // criando recepcionista

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
        if(randomNumber == 2){  // escolhi um dos numeros para decidir se ele NAO vai esperar
            vai_esperar = 0;
        }

        //calcula se paciente tem dinheiro
        randomNumber = rand() % 20;  // gera entre 0 a 19   ->  1/20 = 5%
        if(randomNumber == 4){  // escolhi um dos numeros para nao ter dinheiro
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


    for(i = 0; i < numero_pacientes; i++){
	    pthread_join(p[i],NULL);
    }

	return 0;  // Medico morreu de trabalhar e Recepcionista morreu de fome
}