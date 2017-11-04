#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "server.c"


#define FILE_NAME "data.txt"
pthread_mutex_t leitores = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t bd = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t vez = PTHREAD_MUTEX_INITIALIZER;
pthread_t workers[10];
char* db_value[10];
int num_workers = 0;
int contador;

void* writer(void* msg){
  while(1){
		pthread_mutex_lock(&vez);
		pthread_mutex_lock(&bd);

		sleep(3);
		printf("Finalizou escrita!\n");
		pthread_mutex_unlock(&bd);
		pthread_mutex_unlock(&vez);
		sleep(3);
	}
}

void* reader(void* msg){
  while(1){
		pthread_mutex_lock(&vez);
		pthread_mutex_lock(&leitores);
		contador++;
		if (contador == 1)
		{
			pthread_mutex_lock(&bd);
		}
		pthread_mutex_unlock(&leitores);
		pthread_mutex_unlock(&vez);
		printf("Lendo...\n");
		sleep(5);
		pthread_mutex_lock(&leitores);
		contador--;
		if (contador == 0)
		{
			pthread_mutex_unlock(&bd);
		}
		printf("Finalizou leitura!\n");
		pthread_mutex_unlock(&leitores);
	}
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
