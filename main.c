#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "server.c"

pthread_mutex_t lock_num_leitores = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_bd = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_vez = PTHREAD_MUTEX_INITIALIZER;
pthread_t workers[10];
int num_workers = 0;


void* worker(void* arg){
  int i = *((int*)arg);
  printf("worker %d\n", i);
  while(1){
    printf("worker %d\n", i);
    sleep(5);
  }
}

int main(void){
  int* id;
  char msg[256];
  char buffer[256];
  struct sockaddr_in cli_addr;
  int n, newsockfd, clilen;

  int sockfd = init_socket();

  while(1) {
    listen(sockfd,5);
    int newsockfd = read_message(sockfd, &buffer);
    char** result = parse_message(buffer);
    if(strcmp(result[0], "ler") == 0) {
      id = (int *) malloc(sizeof(int));
      *id = num_workers;
      pthread_create(&(workers[num_workers]), NULL, worker,(void *) id);
        // pthread_join(workers[num_workers],NULL);
      num_workers++;
    }
    write_response(newsockfd);
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
