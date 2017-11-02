#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

pthread_mutex_t lock_num_leitores = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_bd = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_vez = PTHREAD_MUTEX_INITIALIZER;
pthread_t workers[10];
int num_workers = 0;

void error(char *msg)
{
    perror(msg);
    exit(0);
}

void* worker(void* arg){
  int i = *((int*)arg);
  printf("worker %d\n", i);
  while(1){
    printf("worker %d\n", i);
    sleep(5);
  }
}

int init_socket() {
  int sockfd,  portno;
  struct sockaddr_in serv_addr, cli_addr;


  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
       error("ERROR opening socket");
  bzero((char *) &serv_addr, sizeof(serv_addr));
  portno = 4321;
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);
  if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
    error("ERROR on binding");
  return sockfd;
}

char** parse_message(char* message) {
  char* result[2];
  char* command = strtok(message, " ");
  result[0] = command;
  char* body = strtok(NULL, " ");
  result[1] = body;
  return result;
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
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) error("ERROR on accept");
    bzero(buffer,256);
    n = read(newsockfd,buffer,255);
    if (n < 0) printf("ERROR reading from socket");
    char** result = parse_message(buffer);
    if(strcmp(result[0], "ler") == 0) {
      id = (int *) malloc(sizeof(int));
      *id = num_workers;
      pthread_create(&(workers[num_workers]), NULL, worker,(void *) id);
        // pthread_join(workers[num_workers],NULL);
      num_workers++;
      n = write(newsockfd,"OK",18);
      if (n < 0) error("ERROR writing to socket");
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
