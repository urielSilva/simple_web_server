#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

typedef struct message {
  int* id;
  char command[10];
  char body[100];
} Message;

void error(char *msg) {
    perror(msg);
    exit(0);
}

void write_response(int socket) {
  int n = write(socket, "OK", 18);
  if (n < 0) error("ERROR writing to socket");
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

Message* parse_message(char* msg) {
  Message* message = (Message *) malloc(sizeof(Message));
  char* result[2];
  char* command = strtok(msg, " ");
  char* body = strtok(NULL, " ");
  strcpy(message->command, command);
  strcpy(message->body, body);
  puts(message->command);
  puts(message->body);
  return message;
}

int read_message(int sockfd, char* buffer) {
  int n, newsockfd, clilen;
  struct sockaddr_in cli_addr;

  clilen = sizeof(cli_addr);
  newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
  if (newsockfd < 0) error("ERROR on accept");
  bzero(buffer,256);
  n = read(newsockfd,buffer,255);
  if (n < 0) printf("ERROR reading from socket");
  return newsockfd;
}