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


struct _server
{

  int *fd;
  //To discuss:
/*  int fd_tcpS;
  int fd_tcpC;
  int fd_tcpC1;
  int fd_tcpC2;
  int fd_udpS;
  int fd_udpC;*/

  /* server state */
  int node_key;
  char *node_IP;
  char *node_TCPs;
  char *node_TCPc;
  char *node_UDP;

  int succ_key;
  char *succ_IP;
  char *succ_TCP;

  int succ2_key;
  char *succ2_IP; //necessário?

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

  /* info assignement, -1 or NULL means empty */
  serv->node_key = i;
  serv->node_IP = ip;
  serv->node_TCPs = port;
  serv->node_TCPc = NULL;
  serv->node_UDP = NULL;

  serv->succ_key = -1;
  serv->succ_IP = NULL;
  serv->succ_TCP = NULL;

  serv->succ2_key = -1;
  serv->succ2_IP = NULL;

  serv->prev_IP = NULL;
  serv->prev_TCP = NULL;

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
    (*serv)->succ_IP = realloc((*serv)->succ_IP, strlen(succ_IP) * sizeof(char));
    (*serv)->succ_TCP = realloc((*serv)->succ_TCP, strlen(succ_TCP) * sizeof(char));

    (*serv)->succ_key = succ_key;
    strcpy((*serv)->succ_IP, succ_IP);
    strcpy((*serv)->succ_TCP, succ_TCP);

    return 1;
  }
  else return 0;
}






/*
void search_key(int key){
}*/

/* Ideias para as comunicações
Um nó será servidor e cliente.
Portanto, do ponto de vista de servidor, estará a ouvir no seu porto, corre tcpS.
Eventualmente terá que responder ao cliente, mas para isso é necessário saber
que pedido é. O servidor terá que gurdar o porto com o predecessor e do sucessor (será que do pred sim?)
Do ponto de vista de cliente, terá que saber o porto tcp etc do seu servidor, sucessor.
Faz pedidos ao seu sucessor.
*/



/*******************************************************************************
 * init_tcp_server(char *port) - "opens" a tcp server
 *                                       -> waiting from connect from client
 *
 * returns: fd of the created socket
*******************************************************************************/
void init_tcp_server(char *port, server **serv) {

  struct addrinfo hints,*res;
  struct sockaddr_in addr; socklen_t addrlen;


  int fd, newfd, afd=0, errcode, maxfd, counter, /*client_socket[30] = {0},*/ max_clients = 2;
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
  if(bind(fd, res->ai_addr, res->ai_addrlen) == -1) {
    printf("An error occurred on binding!\n");
    exit(1);
  }
  if(listen(fd, 5) == -1){
    printf("An error occurred on listening!\n");
    exit(1);
  }
  printf("Starting to listen ...\n");

}


/*******************************************************************************
 * create_tcp_client(server *serv) - creates  a tcp client
 *
 * returns: fd of the created socket
*******************************************************************************/
void init_tcp_client(server *serv)
{
	int fd, errcode;
	struct addrinfo hints, *res;
  ssize_t n;

  fd = socket(AF_INET,SOCK_STREAM,0);
  //if (fd==-1) return -1; //error

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  //errcode = getaddrinfo( serv->succ_IP, serv->succ_TCP, &hints, &res);
  //if((errcode) != 0) return -1; //error

  //serv->fd_tcpC = fd;

}


///////////////////////// delete later//////////////////////////////////////////////////////////
void tcpC(char* ip, char* port) {

  struct addrinfo hints, *res;
  ssize_t nbytes, nleft, nwritten, nread;

  int fd, n;
  char *ptr, in[128], buffer[128];

  /* Creating a TCP socket */
  fd = socket(AF_INET,SOCK_STREAM,0);
  if(fd == -1){
    printf("An error occurred on socket() function!\n");
    exit(1);
  }

  memset(&hints,0,sizeof hints);
  hints.ai_family=AF_INET;//IPv4
  hints.ai_socktype=SOCK_STREAM;//TCP socket

  /* Get here server's ip and port (either successor's or predecessor's) */


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

      if(write(fd, in, strlen(in)) == -1)/*error*/{
        printf("Error occurred in writting!\n");
        exit(1);
      }
      if(read(fd, buffer, 128) == -1)/*error*/{
        printf("Error occurred in reading!\n");
        exit(1);
      }

      // APAGAR ECHO
      write(1,"echo: ",6); write(1,buffer,strlen(buffer));

      /* Clean arrays */
      in[128] = '\0';
      buffer[128] = '\0';
    }
  }


}






///////////////////////// delete later//////////////////////////////////////////////////////////
void tcpS(char* port, server** serv) {

  struct addrinfo hints,*res;
  struct sockaddr_in addr; socklen_t addrlen;


  int fd, newfd, afd=0, errcode, maxfd, counter, /*client_socket[30] = {0},*/ max_clients = 2;
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
    FD_SET(fd, &rfds); /* dds tcp socket to rfds */

    maxfd = fd; /* socket id, fd */

    /* add child sockets to set */
    int i;
    for (i = 0; i < max_clients; i++) {
      /* socket descriptor */
      afd =(*serv)->fd[i];

      /* if valid socket descriptor then add to read list */
      if(afd > 0) FD_SET(afd, &rfds);

      /* highest file descriptor number, need it for the select function */
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

      /* Save client port here either succesor's either predecessor's */


      /* add new socket to array of sockets */
      for (i = 0; i < max_clients; i++) {
          /* if position is empty, note that there is a break! */
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

          /* Interpretate message here and answer to client if needed*/


          //write buffer in afd
          write(afd, buffer, n);
        }
        else if(strcmp(buffer, "exit\n")){
          printf("Closed!\n");
          close(afd);
          (*serv)->fd[i] = 0;
        }//connection closed by peer

        /* Cleaning buffer */
        buffer[n] = '\0';
      }
    }
  }

  // CHANGE THIS IF NEEDED
  printf("Closing!\n");
  freeaddrinfo(res);
  close(fd);
}
