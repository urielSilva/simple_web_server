#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "barrier.c"
#include "server.c"


#define FILE_NAME "data.txt"
pthread_mutex_t leitores = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t bd = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t contador = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t vez = PTHREAD_MUTEX_INITIALIZER;
pthread_barrier_t barrier;
pthread_t workers[10];
char* db_value[10];
int num_workers = 0;
int existe_clearer = 0;

void* writer(void* msg){
  Message* message = (Message *) msg;
	pthread_mutex_lock(&bd);
  sleep(5);
  printf("escrevendo %s\n", message->body);
  FILE* fp = fopen(FILE_NAME, "a+");
  fprintf(fp, "%s", message->body);
  fclose(fp);
	sleep(3);
	pthread_mutex_unlock(&bd);
  pthread_mutex_lock(&contador);
  num_workers--;
  printf("Finalizou escrita, num workers: %d!\n", num_workers);
  pthread_mutex_unlock(&contador);
  if(existe_clearer) pthread_barrier_wait(&barrier);
}

void* reader(void* msg){
  Message* message = (Message *) msg;
  sleep(5);
		pthread_mutex_lock(&bd);
    FILE* fp = fopen(FILE_NAME, "r");
    char body[10];
    fscanf(fp, "%s", body);
    fclose(fp);
    printf("Leitura: %s!\n", body);
    pthread_mutex_unlock(&bd);
		sleep(5);
    pthread_mutex_lock(&contador);
    num_workers--;
    printf("Finalizou leitura, num workers: %d!\n", num_workers);
    pthread_mutex_unlock(&contador);
    if(existe_clearer) pthread_barrier_wait(&barrier);
}

void* clearer(void* msg){
  pthread_barrier_init(&barrier, NULL, num_workers);
  printf("VOU PARAR NA BARREIRA COM %d\n", num_workers);
  pthread_barrier_wait(&barrier);
  printf("Limpando o arquivo.\n");
  FILE* fp = fopen(FILE_NAME, "w");
  fclose(fp);
  pthread_mutex_lock(&contador);
  printf("SAI DA BARREIRA\n");
  num_workers--;
  existe_clearer = 0;
  pthread_mutex_unlock(&contador);
  sleep(2);
}

void create_worker(Message* message) {
  message->id = num_workers;
  int n;
  if(strcmp(message->command, "write") == 0) {
    printf("criei um writer\n");
    n = pthread_create(&(workers[num_workers]), NULL, writer,(void *) message);
  } else if(strcmp(message->command, "read") == 0) {
    printf("criei um reader\n");
    n = pthread_create(&(workers[num_workers]), NULL, reader,(void *) message);
  } else if(strcmp(message->command, "clear") == 0) {
    existe_clearer = 1;
    printf("criei um clear\n");
    n = pthread_create(&(workers[num_workers]), NULL, clearer,(void *) message);
  }
  printf("erro: %d\n", n);
  num_workers++;
}

int main(void){
  char msg[256];
  char buffer[256];
  struct sockaddr_in cli_addr;
  int n, newsockfd, clilen;

  int sockfd = init_socket();

  while(1) {
    listen(sockfd,5);
    int newsockfd = read_message(sockfd, buffer);
    Message* message = parse_message(buffer);
    if(is_valid_command(message->command)) {
      create_worker(message);
      write_response(newsockfd, "OK.");
    } else {
      write_response(newsockfd, "Invalid command.");
    }

  }


  // for(i = 0; i < 5; i++){
  //   id = (int *) malloc(sizeof(int));
  //   *i  d = i;
  //   pthread_create(&(leitores[i]),NULL,escrever,id);
  // }

  // *id = 0;
  // for(i = 0; i < 5; i++){
  //   id = (int *) malloc(sizeof(int));
  //   *id = i + 6;
  //   pthread_create(&(escritores[i]),NULL,ler,id);
  // }

  // for(i = 0; i < 5; i++){
  //   pthread_join(escritores[i],NULL);
  //   pthread_join(leitores[i],NULL);
  // }

  return 0;
}
