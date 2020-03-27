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

  int *fd;// talvez apagar se usar apenas fd_pred

  /* My client and server sockets */
  int fd_tcpS;
  int fd_tcpC; /* this is to get into a ring (sentry, entry) */
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
  int fd_pred;
  char *pred_IP;
  char *pred_TCP;


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
    perror("Something went wrong with creating new ring!\n");
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

  serv->fd_pred = -1;
  serv->pred_IP = ip;
  serv->pred_TCP = port;


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

  struct addrinfo hints,*res;

  memset(&hints, 0, sizeof hints);
  hints.ai_family=AF_INET;//IPv4
  hints.ai_socktype=SOCK_STREAM;//TCP socket
  hints.ai_flags=AI_PASSIVE;

  /* max_clients: server's predecessor is its client and there can be a new client (sentry, entry) */
  int max_clients = 2;
  int errcode;

  (*serv)->fd = (int*) malloc((max_clients+1) * sizeof(int));
  (*serv)->fd[max_clients] = -1;

  /* Saving the server's fd */
  (*serv)->fd_tcpS = fd;

  /* by default: the server is its client */
  //(*serv)->fd_pred = fd;

  if((errcode = getaddrinfo(NULL, (*serv)->node_TCPs, &hints, &res))!=0){
    perror("An error occurred on getting addresses!\n");
    exit(1);
  }
  if(bind(fd, res->ai_addr, res->ai_addrlen) == -1) {
    perror("An error occurred on binding!\n");
    exit(1);
  }
  if(listen(fd, 5) == -1){
    perror("An error occurred on listening!\n");
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
int init_tcp_client(server** serv, fd_set *rfds) {

  struct addrinfo hints, *res;
  ssize_t nbytes, nleft, nwritten, nread;

  int fd, n;
  char msg[128], buffer[128];

  if((fd=socket(AF_INET,SOCK_STREAM,0)) == -1){
    perror("An error occurred on socket() function!\n");
    exit(1);
  }

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  /* Saving my fd in order to get its value in tcpC and set it to rfds */
  (*serv)->fd_tcpC = fd;
  FD_SET(fd, &(*rfds));

  /* Conneting to server */
  if(getaddrinfo((*serv)->succ_IP, (*serv)->succ_TCP, &hints, &res) != 0){
    perror("An error occurred on getting addresses!\n");
    exit(1);
  }
  if(connect(fd, res->ai_addr,res->ai_addrlen) == -1){
    perror("An error occurred in connection!\n");
    exit(1);
  }

  /* Sending a request message */
  sprintf(msg, "NEW %d %s %s\n", (*serv)->node_key, (*serv)->node_IP, (*serv)->node_TCPs );
  if(write(fd, msg, strlen(msg)) == -1)/*error*/{
    perror("Error occurred in writting!\n");
    exit(1);
  }
  printf("Message to be sent: %s\n", msg);

  return fd;
}



int tcpS(server** serv, fd_set rfds) {

  int fd = (*serv)->fd_tcpS;
  int newfd, errcode, max_clients = 2, counter;
  char resp[128], buffer[128];
  ssize_t n, nw;
  struct sockaddr_in addr; socklen_t addrlen;

  int key;
  char ip[20], port[20], first[20];

  /* add child sockets to set */
  int i;


  if(FD_ISSET(fd, &rfds)){

    addrlen = sizeof(addr);
    if((newfd = accept(fd, (struct sockaddr*)&addr, &addrlen)) == -1) {
      perror("An error occurred on accept() function!\n");
      exit(1);
    }
    printf("New connection %s:%d with fd: %d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), newfd);

    //if(FD_ISSET(fd, &(*rfds))){

      if((n = read(newfd, buffer, 128))!=0){
        if(n == -1) {
          perror("An error occurred on read() function!\n");
          exit(1);
        }

        for (int i = 0; i < strlen(buffer); i++) {
          if(buffer[i] == '\n' && i < (strlen(buffer) - 1)) {
            buffer[i+1] = '\0';
          }
        }

        printf("Message received from new client: "/*, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), newfd*/);
        printf("%s\n",buffer);
        // NOTA: INTERPRETAR O BUFFER COM SSCANF, QUE TIPO DE MENSAGEM É, VER A QUESTÃO DO \n (no 'for' de cima)

        /* SENTRY */
        sscanf(buffer, "%s %d %s %s", first, &key, ip, port);

        /* SENTRY - write server response in newfd, info about server succ */
        sprintf(resp, "SUCC %d %s %s\n", (*serv)->succ_key, (*serv)->succ_IP, (*serv)->succ_TCP);
        printf("To send as response: %s\n", resp);
        write(newfd, resp, strlen(resp));

        /* SENTRY - write to server predecessor about the new server's client*/
        if((*serv)->fd_pred == -1) printf("Server had no predecessor, no message sent to its old predecessor\n");
        else {
          sprintf(resp, "NEW %d %s %s\n", key, ip, port);
          printf("\nTo send to my old predecessor (about my new predecessor): %s\n", resp);
          write((*serv)->fd_pred, resp, strlen(resp));
          close((*serv)->fd_pred);
        }

        /* SENTRY - Save new client (predecessor) stuff here (and succesor's ?) */
        (*serv)->pred_IP = (char *) realloc((*serv)->pred_IP, (strlen(ip)+1) * sizeof(char));
        (*serv)->pred_TCP = (char *) realloc((*serv)->pred_TCP, (strlen(port)+1) * sizeof(char));
        strcpy((*serv)->pred_IP, ip);
        strcpy((*serv)->pred_TCP, port);
        (*serv)->fd_pred = newfd;

        return newfd;

        /* if valid socket descriptor then add to read list */
      //  if((*serv)->fd_pred > 0) FD_SET((*serv)->fd_pred , &(*rfds));
        /* highest file descriptor number, need it for the select function */
      //  if((*serv)->fd_pred > *maxfd) *maxfd = (*serv)->fd_pred;

      }
    //}
  }
  return 0;
}


void tcpS_recv(server **serv, fd_set rfds){

  char buffer[128];
  ssize_t n;
  struct sockaddr_in addr; socklen_t addrlen;

  if((*serv)->fd_pred != -1 && FD_ISSET((*serv)->fd_pred, &rfds)){
    // o servidor está a congelar aqui, depois de o cliente mandar sentry
    if((n = read((*serv)->fd_pred, buffer, 128))!=0){
      if(n == -1) {
        perror("An error occurred on read() function!");
      }
      printf("from %s:%d   fd: %d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), (*serv)->fd_pred);
      write(1,"Received: ",10); write(1, buffer, n);
      printf("\n");

      /* Interpretate message here and answer to client if needed*/


      /* write server response in afd */
      //sprintf(resp, "SUCC %d %s %s\n", (*serv)->succ_key, (*serv)->succ_IP, (*serv)->succ_TCP);
      //write(afd, resp, strlen(resp));
    }
    else if(strcmp(buffer, "leave\n")){
      printf("Closed!\n");
      close((*serv)->fd_pred);
    }//connection closed by peer

    /* Cleaning buffer */
    buffer[0] = '\0';
  }
}



void tcpC (server** serv, fd_set rfds) {

  char buffer[128] = {'\0'};
  char first[128], ip[128], port[128];
  int key;

  if((*serv)->fd_tcpC != -1 && FD_ISSET((*serv)->fd_tcpC, &rfds)){
    if(read((*serv)->fd_tcpC, buffer, 128) == -1){
      perror("Not reading!\n");
      exit(1);
    }
    else{
      printf("From server: %s\n", buffer);
      sscanf(buffer, "%s %d %s %s", first, &key, ip, port);

      /* Save my 2nd succesor info */
      if (strcmp(first, "SUCC")){
        (*serv)->succ2_key = key;
        (*serv)->succ2_IP = realloc((*serv)->succ2_IP, (strlen(ip)+1) * sizeof(char));
        strcmp((*serv)->succ2_IP, ip);

        (*serv)->succ2_TCP = realloc((*serv)->succ2_TCP, (strlen(port)+1) * sizeof(char));
        strcmp((*serv)->succ2_TCP, port);
      }

      //if (strcmp(first, "NEW")) sscanf(buffer, "%s %d %s %s", )

    }
  }
}
