#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "barrier.c"
#include "server.c"


#define FILE_NAME "data.txt"
pthread_mutex_t leitores = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t bd = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t counter = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t vez = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t wait_clearer = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t clearer_cond = PTHREAD_COND_INITIALIZER;
pthread_barrier_t barrier;
pthread_t workers[100];
sem_t * semaphore;
char* db_value[10];
int max_workers = 5;
int num_workers = 0;
int num_leitores = 0;
int clearer_exists = 0;

void* writer(void* msg){
  Message* message = (Message *) msg;
  pthread_mutex_lock(&wait_clearer);
  while(clearer_exists) {
    printf("VOU ESPERAR O CLEARER\n");
    pthread_cond_wait(&clearer_cond, &wait_clearer);
  }
  pthread_mutex_unlock(&wait_clearer);
  pthread_mutex_lock(&vez);
	pthread_mutex_lock(&bd);
  printf("escrevendo %s\n", message->body);
  FILE* fp = fopen(FILE_NAME, "a+");
  fprintf(fp, "%s", message->body);
  fclose(fp);
	sleep(3);
	pthread_mutex_unlock(&bd);
  pthread_mutex_unlock(&vez);
  pthread_mutex_lock(&counter);
  num_workers--;
  printf("Finalizou escrita, num workers: %d!\n", num_workers);
  pthread_mutex_unlock(&counter);
  if(clearer_exists) pthread_barrier_wait(&barrier);
}

void* reader(void* msg){
  sem_wait(semaphore);
  Message* message = (Message *) msg;
  pthread_mutex_lock(&wait_clearer);
  while(clearer_exists) {
    printf("Esperando limpeza do arquivo.\n");
    pthread_cond_wait(&clearer_cond, &wait_clearer);
  }
  pthread_mutex_unlock(&wait_clearer);
  pthread_mutex_lock(&vez);
  pthread_mutex_lock(&leitores);
  num_leitores++;
  if (num_leitores == 1) {
    pthread_mutex_lock(&bd);
  }
  pthread_mutex_unlock(&leitores);
  pthread_mutex_unlock(&vez);
  FILE* fp = fopen(FILE_NAME, "r");
  char body[10];
  fscanf(fp, "%s", body);
  fclose(fp);
  printf("Leitura: %s!\n", body);
  pthread_mutex_unlock(&bd);
  sleep(5);
  pthread_mutex_lock(&counter);
  num_workers--;
  pthread_mutex_unlock(&counter);
  printf("Finalizou leitura, num workers: %d!\n", num_workers);
  pthread_mutex_lock(&leitores);
  num_leitores--;
  if (num_leitores == 0) {
    pthread_mutex_unlock(&num_leitores);
  }
  pthread_mutex_unlock(&leitores);
  if(clearer_exists) pthread_barrier_wait(&barrier);
  sem_post(semaphore);
}

void* clearer(void* msg){

  pthread_barrier_init(&barrier, NULL, num_workers);
  sleep(4);
  pthread_barrier_wait(&barrier);
  printf("Limpando o arquivo.\n");
  FILE* fp = fopen(FILE_NAME, "w");
  fclose(fp);
  pthread_mutex_lock(&counter);
  num_workers--;
  clearer_exists = 0;
  pthread_mutex_unlock(&counter);
  pthread_cond_broadcast(&clearer_cond);
}

void* set_max(void* msg) {
  printf("oi");
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
  } else if(strcmp(message->command, "set_max") == 0) {
    sem_unlink("semaphore");
    semaphore = sem_open("semaphore", O_CREAT, 0644, atoi(message->body));
  }
  num_workers++;
}

int main(void){
  char msg[256];
  char buffer[256];
  struct sockaddr_in cli_addr;
  int n, newsockfd, clilen;

  int sockfd = init_socket();
  sem_unlink("semaphore");
  semaphore = sem_open("semaphore", O_CREAT, 0644, max_workers);

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

  return 0;
}
