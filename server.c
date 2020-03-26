#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include "server.h"
#define max(A,B) ((A)>=(B)?(A):(B))





/* Ideias para as comunicações
Um nó será servidor e cliente.
Portanto, do ponto de vista de servidor, estará a ouvir no seu porto, corre tcpS.
Eventualmente terá que responder ao cliente, mas para isso é necessário saber
que pedido é. O servidor terá que gurdar o porto com o predecessor e do sucessor (será que do pred sim?)
Do ponto de vista de cliente, terá que saber o porto tcp etc do seu servidor, sucessor.
Faz pedidos ao seu sucessor.
*/



struct _server
{

  int *fd;
  //To discuss:

  /* My client and server sockets */
  int fd_tcpS;
  int fd_tcpC;
/*
  int fd_tcpC1;
  int fd_tcpC2;
  int fd_udpS;
  int fd_udpC;*/

  /* server state */
  int node_key;
  char *node_IP;
  char *node_TCPs;
  //char *node_TCPc;
  char *node_UDP;

  int succ_key;
  char *succ_IP;
  char *succ_TCP;

  int succ2_key;
  char *succ2_IP;
  char *succ2_TCP;

  //acho que dá jeito para certas mensagens - a verificar
  char *prev_IP;
  char *prev_TCP;


};



/*******************************************************************************
 * distanceN(int , int, int ) - GETS THE DISTANCE BETWEEN k AND l
 *
 * returns: distance between k and l keys
*******************************************************************************/
int distanceN (int k, int l, int N)
{
  return ((l-k) % N);
}


/*******************************************************************************
 * freeServer(server** ) - DEALLOCATES MEMORY ALLOCATED FOR server**
 *
 * returns: void
*******************************************************************************/
void freeServer(server** serv)
{

  if((*serv) != NULL)
  {
    if((*serv)->fd != NULL) free((*serv)->fd);
    if((*serv)->succ_IP != NULL) free((*serv)->succ_IP);
    if((*serv)->succ_TCP != NULL) free((*serv)->succ_TCP);
    free(*serv);
    *serv = NULL;
  }
}


/*******************************************************************************
 * newr(int ,char* , char* ) - CREATES A NEW RING
 *
 * returns: a pointer to the local server that belongs to the new ring
*******************************************************************************/
server* newr(int i, char* ip, char* port)
{

  server *serv = NULL;

  /* allacating memory for the local server */
  serv = (server*) malloc(sizeof(server));
  if(serv == NULL)
  {
    printf("Something went wrong with creating new ring!\n");
    exit(0);
  }


  serv->fd_tcpS = -1;
  serv->fd_tcpC = -1;

  /* info assignement, -1 or NULL means empty */
  serv->node_key = i;
  serv->node_IP = ip;
  serv->node_TCPs = port;
//  serv->node_TCPc = NULL;
  serv->node_UDP = NULL;

  serv->succ_key = i;
  serv->succ_IP = ip;
  serv->succ_TCP = port;

  serv->succ2_key = i;
  serv->succ2_IP = ip;
  serv->succ2_TCP = port;

  serv->prev_IP = ip;
  serv->prev_TCP = port;


  return serv;
}


/*******************************************************************************
 * showState(server* ) - SHOWING THE LOCAL SERVER STATE
 *
 * returns: void
*******************************************************************************/
void showState(server* serv)
{

  if(serv == NULL) printf("The local server has no ring associated!\n");
  else
  {
    printf("\n\n------ ABOUT THE LOCAL SERVER ------\n");
    printf("         Key: %d\n", serv->node_key);
    printf("         IP: %s\n", serv->node_IP);
    printf("         TCP server port: %s\n", serv->node_TCPs);
    //printf("         TCP client port: %s\n", serv->node_TCPc);
    printf("         UDP port: %s\n", serv->node_UDP);

    printf("\n------ ABOUT THE SUCCESSOR SERVER ------\n");
    printf("         Key: %d\n", serv->succ_key);
    printf("         IP: %s\n", serv->succ_IP);
    printf("         TCP port: %s\n", serv->succ_TCP);

    printf("\n------ ABOUT THE 2nd SUCCESSOR SERVER ------\n");
    printf("         Key: %d\n", serv->succ2_key);
    printf("         IP: %s\n", serv->succ2_IP);
  }
}


/*******************************************************************************
 * leave(server* )
 *
 * Description: leaves the ring, closes the TCP sessions with predecessor and
                 sucessor
 * returns: void
*******************************************************************************/
void leave(server** serv)
{

  int i;
  int length = sizeof((*serv)->fd) / sizeof(int);

  for(i = 0; i < length; i++) close((*serv)->fd[i]);
}



// mudar talvez alguns argumentos, pred e/ou ssucc
int update_state(server** serv, int key, int succ_key, char* succ_IP, char* succ_TCP) {

  /* if the key inserted is mine */
  if((*serv)->node_key == key) {
    (*serv)->succ_IP = realloc((*serv)->succ_IP, (strlen(succ_IP)+1) * sizeof(char));
    (*serv)->succ_TCP = realloc((*serv)->succ_TCP, (strlen(succ_TCP)+1) * sizeof(char));

    (*serv)->succ_key = succ_key;
    strcpy((*serv)->succ_IP, succ_IP);
    strcpy((*serv)->succ_TCP, succ_TCP);

    return 1;
  }
  else return 0;
}


int init_fd_parent () {

  int fd;

  if((fd=socket(AF_INET,SOCK_STREAM,0)) == -1){
    printf("An error occurred on socket() function!\n");
    exit(1);
  }

  return fd;
}


/*******************************************************************************
 * init_tcp_server(char *port) - "opens" a tcp server
 *                                       -> waiting from connect from client
 *
 * returns: fd of the created socket
*******************************************************************************/
void init_tcp_server(char *port, server **serv, int fd) {

  /*struct addrinfo hints,*res;
  int fd,

  if((fd=socket(AF_INET,SOCK_STREAM,0)) == -1){
    printf("An error occurred on socket() function!\n");
    exit(1);
  }
*/
  struct addrinfo hints,*res;

  memset(&hints, 0, sizeof hints);
  hints.ai_family=AF_INET;//IPv4
  hints.ai_socktype=SOCK_STREAM;//TCP socket
  hints.ai_flags=AI_PASSIVE;

  int max_clients = 2, errcode;

  (*serv)->fd = (int*) malloc((max_clients+1) * sizeof(int));
  (*serv)->fd[max_clients+1] = 0;

  (*serv)->fd_tcpS = fd;

  if((errcode = getaddrinfo(NULL, (*serv)->node_TCPs, &hints, &res))!=0){
    printf("An error occurred on getting addresses!\n");
    exit(1);
  }
  if(bind(fd, res->ai_addr, res->ai_addrlen) == -1) {
    printf("An error occurred on binding!\n");
    exit(1);
  }
  if(listen(fd, 5) == -1){
    printf("An error occurred on listening!\n");
    exit(1);
  }
  printf("Starting to listen ...\n");
  freeaddrinfo(res);
}



/*******************************************************************************
 * create_tcp_client(server *serv) - creates  a tcp client
 *
 * returns: fd of the created socket
*******************************************************************************/
void init_tcp_client(server** serv, fd_set *rfds) {

  struct addrinfo hints, *res;
  ssize_t nbytes, nleft, nwritten, nread;

  int fd, n;
  char msg[128], buffer[128];

  if((fd=socket(AF_INET,SOCK_STREAM,0)) == -1){
    printf("An error occurred on socket() function!\n");
    exit(1);
  }

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  /* Saving my client fd in order to get its value in tcpC and set it to rfds */
  (*serv)->fd_tcpC = fd;
  FD_SET(fd, &(*rfds));

  /* Conneting to server */
  //printf("succ_IP %s succ_TCP %s\n", serv->succ_TCP, serv->succ_IP);
  if(getaddrinfo((*serv)->succ_IP, (*serv)->succ_TCP, &hints, &res) != 0){
    printf("An error occurred on getting addresses!\n");
    exit(1);
  }
  if(connect(fd, res->ai_addr,res->ai_addrlen) == -1){
    printf("An error occurred in connection!\n");
    exit(1);
  }

  /* Sending a request message */
  sprintf(msg, "NEW %d %s %s\n", (*serv)->node_key, (*serv)->node_IP, (*serv)->node_TCPs );
  if(write(fd, msg, strlen(msg)) == -1)/*error*/{
    printf("Error occurred in writting!\n");
    exit(1);
  }
  printf("Message to be sent: %s\n", msg);

}



int tcpS(server** serv, fd_set *rfds) {

  int fd = (*serv)->fd_tcpS;
  int maxfd, newfd, afd=0, errcode, max_clients = 2, counter;
  char resp[128], buffer[128];
  ssize_t n, nw;
  struct sockaddr_in addr; socklen_t addrlen;

  int key;
  char ip[20], port[20], first[20];

  //FD_ZERO(&rfds);
  //FD_SET(fd, &rfds); /* adds tcp socket to rfds */

  maxfd = fd; /* socket id, fd */

  /* add child sockets to set */
  int i;
  for (i = 0; i < max_clients; i++) {
    /* socket descriptor */
    afd = (*serv)->fd[i];

    /* if valid socket descriptor then add to read list */
    if(afd > 0) FD_SET(afd, &(*rfds));

    /* highest file descriptor number, need it for the select function */
    if(afd > maxfd) maxfd = afd;
  }

  /* select blocks the programm for timeout seconds! */
/*  counter = select(maxfd+1, &rfds, (fd_set*)NULL, (fd_set*)NULL, &timeout);
  if(counter <= 0) {
    printf("No message received, but server still alive!\n");
  }*/

  if(FD_ISSET(fd, &(*rfds))){

    addrlen = sizeof(addr);
    if((newfd = accept(fd, (struct sockaddr*)&addr, &addrlen)) == -1) {
      printf("An error occurred on accept() function!\n");
      exit(1);
    }
    printf("New connection %s:%d with fd: %d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), newfd);

    if(FD_ISSET(fd, &(*rfds))){

      if((n = read(newfd, buffer, 128))!=0){
        if(n == -1) {
          printf("An error occurred on read() function!\n");
          exit(1);
        }

        for (int i = 0; i < strlen(buffer); i++) {
          if(buffer[i] == '\n' && i < (strlen(buffer) - 1)) {
            buffer[i+1] = '\0';
          }
        }

        printf("- Message received from %s:%d   fd: %d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), newfd);
        printf("%s\n",buffer);
        // NOTA: INTERPRETAR O BUFFER COM SSCANF, QUE TIPO DE MENSAGEM É, VER A QUESTÃO DO \n (no 'for' de cima)

        /* SENTRY */
        sscanf(buffer, "%s %d %s %s", first, &key, ip, port);

        /* SENTRY - write server response in newfd, info about server succ */
        sprintf(resp, "SUCC %d %s %s\n", (*serv)->succ_key, (*serv)->succ_IP, (*serv)->succ_TCP);
        printf("\n- To send as response: %s\n", resp);
        write(newfd, resp, strlen(resp));

        /* SENTRY - write to server predecessor about the new server's client*/

        /* SENTRY - Save client stuff here (and succesor's ?) */
        (*serv)->prev_IP = (char *) realloc((*serv)->prev_IP, (strlen(ip)+1) * sizeof(char));
        (*serv)->prev_TCP = (char *) realloc((*serv)->prev_TCP, (strlen(port)+1) * sizeof(char));
        strcpy((*serv)->prev_IP, ip);
        strcpy((*serv)->prev_TCP, port);

        /* add new socket to array of sockets */
        for (i = 0; i < max_clients; i++) {
          /* if position is empty fill it with newfd, note that there is a break! */
          if((*serv)->fd[i] == 0 ) {
              (*serv)->fd[i] = newfd;
              printf("Adding to list of sockets %d\n" , newfd);
              break;
          }
        }
      }

      for(i = 0; i < max_clients; i++) {

        afd = (*serv)->fd[i];
        if(FD_ISSET(afd, &(*rfds))){

          if((n = read(afd, buffer, 128))!=0){
            if(n == -1) {
              printf("An error occurred on read() function!\n");
              exit(1);
            }

            printf("from %s:%d   fd: %d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), afd);
            write(1,"Received: ",10); write(1, buffer, n);
            printf("\n");

            /* Interpretate message here and answer to client if needed*/


            /* write server response in afd */
            //sprintf(resp, "SUCC %d %s %s\n", (*serv)->succ_key, (*serv)->succ_IP, (*serv)->succ_TCP);
            //write(afd, resp, strlen(resp));
          }
          else if(strcmp(buffer, "leave\n")){
            printf("Closed!\n");
            close(afd);
            (*serv)->fd[i] = 0;
          }//connection closed by peer

          /* Cleaning buffer */
          buffer[n] = '\0';
        }
      }
    }
  }
  return maxfd;
}

void tcpC (server** serv, fd_set* rfds) {

  char buffer[128] = {'\0'};
  char first[128];

  if((*serv)->fd_tcpC != -1){
    if(FD_ISSET((*serv)->fd_tcpC, &(*rfds))){
      if(read((*serv)->fd_tcpC, buffer, 128) == -1){
        printf("Not reading!\n");
        //exit(1);
      }
      else{
        printf("- From server: %s\n", buffer);
    // if first = SUCC
        /* Save my 2nd succesor info */
        sscanf(buffer, "%s %d %s %s", first, &(*serv)->succ2_key, (*serv)->succ2_IP, (*serv)->succ2_TCP);

      }
    }
  }
}


/*

///////////////////////// delete later//////////////////////////////////////////////////////////
void tcpC(char* ip, char* port) {

  struct addrinfo hints, *res;
  ssize_t nbytes, nleft, nwritten, nread;

  int fd, n;
  char *ptr, in[128], buffer[128];

  //Creating a TCP socket
  fd = socket(AF_INET,SOCK_STREAM,0);
  if(fd == -1){
    printf("An error occurred on socket() function!\n");
    exit(1);
  }

  memset(&hints,0,sizeof hints);
  hints.ai_family=AF_INET;//IPv4
  hints.ai_socktype=SOCK_STREAM;//TCP socket

  // Get here server's ip and port (either successor's or predecessor's)


  if(getaddrinfo(ip, port, &hints, &res) != 0){
    printf("An error occurred on getting addresses!\n");
    exit(1);
  }
  if(connect(fd,res->ai_addr,res->ai_addrlen) == -1){
    printf("An error occurred in connection!\n");
    exit(1);
  }

  while(1){
    printf("Enter a string: ");
    if(fgets(in, 128, stdin)) {

      if(write(fd, in, strlen(in)) == -1){
        printf("Error occurred in writting!\n");
        exit(1);
      }
      if(read(fd, buffer, 128) == -1){
        printf("Error occurred in reading!\n");
        exit(1);
      }

      // APAGAR ECHO
      write(1,"echo: ",6); write(1,buffer,strlen(buffer));

      // Clean arrays
      in[128] = '\0';
      buffer[128] = '\0';
    }
  }


}






///////////////////////// delete later//////////////////////////////////////////////////////////
void tcpS(char* port, server** serv) {

  struct addrinfo hints,*res;
  struct sockaddr_in addr; socklen_t addrlen;


  int fd, newfd, afd=0, errcode, maxfd, counter, client_socket[30] = {0},  max_clients = 2;
  char *ptr, buffer[128];
  ssize_t n, nw;

  (*serv)->fd = (int*) malloc(max_clients * sizeof(int));

  fd_set rfds;

  if((fd=socket(AF_INET,SOCK_STREAM,0)) == -1){
    printf("An error occurred on socket() function!\n");
    exit(1);
  }

  memset(&hints, 0, sizeof hints);
  hints.ai_family=AF_INET;//IPv4
  hints.ai_socktype=SOCK_STREAM;//TCP socket
  hints.ai_flags=AI_PASSIVE;

  if((errcode = getaddrinfo(NULL, port, &hints, &res))!=0){
    printf("An error occurred on getting addresses!\n");
    exit(1);
  }
  if(bind(fd, res->ai_addr, res->ai_addrlen) == -1)
  {
    printf("An error occurred on binding!\n");
    exit(1);
  }
  if(listen(fd, 5) == -1){
    printf("An error occurred on listening!\n");
    exit(1);
  }
  printf("Starting to listen ...\n");

  while(1){

    FD_ZERO(&rfds);
    FD_SET(fd, &rfds); // adds tcp socket to rfds

    maxfd = fd; //socket id, fd

    //add child sockets to set
    int i;
    for (i = 0; i < max_clients; i++) {
      //ocket descriptor
      afd =(*serv)->fd[i];

      // if valid socket descriptor then add to read list
      if(afd > 0) FD_SET(afd, &rfds);

      // highest file descriptor number, need it for the select function
      if(afd > maxfd) maxfd = afd;
    }

    counter = select(maxfd+1, &rfds, (fd_set*)NULL, (fd_set*)NULL, (struct timeval *)NULL);
    if(counter <= 0) {
      printf("An error occurred on select() function!\n");
      exit(1);
    }

    if(FD_ISSET(fd, &rfds)){

      addrlen = sizeof(addr);

      if((newfd = accept(fd, (struct sockaddr*)&addr, &addrlen)) == -1) {
        printf("An error occurred on accept() function!\n");
        exit(1);
      }
      printf("New connection %s:%d with fd: %d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), newfd);

      // Save client port here either succesor's either predecessor's


      // add new socket to array of sockets
      for (i = 0; i < max_clients; i++) {
          // if position is empty, note that there is a break!
          if((*serv)->fd[i] == 0 ) {
              (*serv)->fd[i] = newfd;
              printf("Adding to list of sockets %d\n\n" , newfd);
              break;
          }
      }
    }

    for(i = 0; i < max_clients; i++) {

      afd = (*serv)->fd[i];
      if(FD_ISSET(afd, &rfds)){

        if((n = read(afd, buffer, 128))!=0){
          if(n == -1) {
            printf("An error occurred on read() function!\n");
            exit(1);
          }

          printf("from %s:%d   fd: %d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), afd);
          write(1,"Received: ",10); write(1, buffer, n);
          printf("\n");

          // Interpretate message here and answer to client if needed


          //write buffer in afd
          write(afd, buffer, n);
        }
        else if(strcmp(buffer, "exit\n")){
          printf("Closed!\n");
          close(afd);
          (*serv)->fd[i] = 0;
        }//connection closed by peer

        //Cleaning buffer
        buffer[n] = '\0';
      }
    }
  }

  // CHANGE THIS IF NEEDED
  printf("Closing!\n");
  freeaddrinfo(res);
  close(fd);
}*/
