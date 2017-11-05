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
pthread_mutex_t counter = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t wait_clearer = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t clearer_cond = PTHREAD_COND_INITIALIZER;
pthread_barrier_t barrier;
pthread_t workers[10];
char* db_value[10];
int num_workers = 0;
int clearer_exists = 0;

void* writer(void* msg){
  Message* message = (Message *) msg;
  pthread_mutex_lock(&wait_clearer);
  while(clearer_exists) {
    printf("VOU ESPERAR O CLEARER\n");
    pthread_cond_wait(&clearer_cond, &wait_clearer);
  }
  pthread_mutex_unlock(&wait_clearer);
	pthread_mutex_lock(&bd);
  printf("escrevendo %s\n", message->body);
  FILE* fp = fopen(FILE_NAME, "a+");
  fprintf(fp, "%s", message->body);
  fclose(fp);
	sleep(3);
	pthread_mutex_unlock(&bd);
  pthread_mutex_lock(&counter);
  num_workers--;
  printf("Finalizou escrita, num workers: %d!\n", num_workers);
  pthread_mutex_unlock(&counter);
  if(clearer_exists) pthread_barrier_wait(&barrier);
}

void* reader(void* msg){
  Message* message = (Message *) msg;
  pthread_mutex_lock(&wait_clearer);
  while(clearer_exists) {
    printf("VOU ESPERAR O CLEARER\n");
    pthread_cond_wait(&clearer_cond, &wait_clearer);
  }
  pthread_mutex_unlock(&wait_clearer);
	pthread_mutex_lock(&bd);
  FILE* fp = fopen(FILE_NAME, "r");
  char body[10];
  fscanf(fp, "%s", body);
  fclose(fp);
  printf("Leitura: %s!\n", body);
  pthread_mutex_unlock(&bd);
  sleep(5);
  pthread_mutex_lock(&counter);
  num_workers--;
  printf("Finalizou leitura, num workers: %d!\n", num_workers);
  pthread_mutex_unlock(&counter);
  if(clearer_exists) pthread_barrier_wait(&barrier);
}

void* clearer(void* msg){
  pthread_barrier_init(&barrier, NULL, num_workers);
  printf("VOU PARAR NA BARREIRA COM %d\n", num_workers);
  sleep(4);
  pthread_barrier_wait(&barrier);
  printf("Limpando o arquivo.\n");
  FILE* fp = fopen(FILE_NAME, "w");
  fclose(fp);
  pthread_mutex_lock(&counter);
  printf("SAI DA BARREIRA\n");
  num_workers--;
  clearer_exists = 0;
  pthread_mutex_unlock(&counter);
  printf("LIBERANDO A GALERA\n");
  pthread_cond_broadcast(&clearer_cond);

}

void create_worker(Message* message) {
  message->id = num_workers;
  int n;
  if(strcmp(message->command, "write") == 0) {
    n = pthread_create(&(workers[num_workers]), NULL, writer,(void *) message);
  } else if(strcmp(message->command, "read") == 0) {
    n = pthread_create(&(workers[num_workers]), NULL, reader,(void *) message);
  } else if(strcmp(message->command, "clear") == 0) {
    clearer_exists = 1;
    n = pthread_create(&(workers[num_workers]), NULL, clearer,(void *) message);
  }
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
