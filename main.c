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


void* worker(void* msg){
  Message* message = ((Message*)msg);
  printf("worker %d command: %s body: %s\n", message->id, message->command, message->body);
  while(1){
    sleep(5);
  }
}

void create_worker(Message* message) {
  message->id = num_workers;
  pthread_create(&(workers[num_workers]), NULL, worker,(void *) message);
  num_workers++;
}

int is_read(char* command) {
  return strcmp(command, "read") == 0;
}

int is_write(char* command) {
  return strcmp(command, "write") == 0;
}

int main(void){
  char msg[256];
  char buffer[256];
  struct sockaddr_in cli_addr;
  int n, newsockfd, clilen;

  int sockfd = init_socket();

  while(1) {
    listen(sockfd,5);
    int newsockfd = read_message(sockfd, &buffer);
    Message* message = parse_message(buffer);
    if(is_read(message->command) || is_write(message->command)) {
      printf("ao");
      create_worker(message);
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
