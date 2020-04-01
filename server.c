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
  /* My client and server sockets */
  int fd_tcpS; // this to listen to clients
  int fd_tcpC; // this is to get into a ring (sentry, entry)

  /* Server state */
  /* the local */
  int node_key;    // node's key
  char *node_IP;   // node's ip
  char *node_TCPs; // node's port
//  char *node_UDP;

  /* the successor */
  int succ_key;   // successor's key
  char *succ_IP;  // successor's ip
  char *succ_TCP; // successor's port

  /* the 2nd successor */
  int succ2_key;   // 2nd successor's key
  char *succ2_IP;  // 2nd successor's ip
  char *succ2_TCP; // 2nd successor's port

  /* the predecessor */
  int fd_pred;    // predecessor's socket, to send messages when when appropriated
  char *pred_IP;  // predecessor's ip
  char *pred_TCP; // predecessor's port

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
    //if((*serv)->fd != NULL) free((*serv)->fd);
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
    perror("Something went wrong with creating new ring");
    exit(0);
  }


  /* Default info assignement, -1 or NULL means empty */
  serv->fd_tcpS = -1;
  serv->fd_tcpC = -1;

  serv->node_key = i;
  serv->node_IP = (char *) malloc((strlen(ip) + 1) * sizeof(char));
  serv->node_TCPs = (char *) malloc((strlen(port) + 1) * sizeof(char));
  strcpy(serv->node_IP, ip);
  strcpy(serv->node_TCPs, port);
  //serv->node_UDP = NULL;

  serv->succ_key = i;
  serv->succ_IP = (char *) malloc((strlen(ip) + 1) * sizeof(char));
  serv->succ_TCP = (char *) malloc((strlen(port) + 1) * sizeof(char));
  strcpy(serv->succ_IP, ip);
  strcpy(serv->succ_TCP, port);

  serv->succ2_key = i;
  serv->succ2_IP = (char *) malloc((strlen(ip) + 1) * sizeof(char));
  serv->succ2_TCP = (char *) malloc((strlen(port) + 1) * sizeof(char));
  strcpy(serv->succ2_IP, ip);
  strcpy(serv->succ2_TCP, port);

  serv->fd_pred = -1;
  serv->pred_IP = (char *) malloc((strlen(ip) + 1) * sizeof(char));
  serv->pred_TCP = (char *) malloc((strlen(port) + 1) * sizeof(char));
  strcpy(serv->pred_IP, ip);
  strcpy(serv->pred_TCP, port);


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
                 sucessor and resets the local's state
 * returns: void
*******************************************************************************/
void leave(server** serv)
{

  printf("Leaving (closing TCP sessions and resetting the state) ...\n");

  close((*serv)->fd_tcpC); // closing TCP session with successor (server)
  close((*serv)->fd_pred); // closing TCP session with predecessor (client)

  /* Resetting the successor: the default (see newr() function) */
  (*serv)->succ_IP = realloc((*serv)->succ_IP, (strlen((*serv)->node_IP)+1) * sizeof(char));
  (*serv)->succ_TCP = realloc((*serv)->succ_TCP, (strlen((*serv)->node_TCPs)+1) * sizeof(char));
  (*serv)->succ_key = (*serv)->node_key;
  strcpy((*serv)->succ_IP, (*serv)->node_IP);
  strcpy((*serv)->succ_TCP, (*serv)->node_TCPs);

  /* Resetting the 2nd successor */
  (*serv)->succ2_IP = realloc((*serv)->succ_IP, (strlen((*serv)->node_IP)+1) * sizeof(char));
  (*serv)->succ2_TCP = realloc((*serv)->succ_TCP, (strlen((*serv)->node_TCPs)+1) * sizeof(char));
  (*serv)->succ2_key = (*serv)->node_key;
  strcpy((*serv)->succ2_IP, (*serv)->node_IP);
  strcpy((*serv)->succ2_TCP, (*serv)->node_TCPs);

  /* Resetting the predecessor */
  (*serv)->fd_pred = -1;
  (*serv)->pred_IP = realloc((*serv)->pred_IP, (strlen((*serv)->node_IP)+1) * sizeof(char));
  (*serv)->succ_TCP = realloc((*serv)->pred_TCP, (strlen((*serv)->node_TCPs)+1) * sizeof(char));
  strcpy((*serv)->succ_IP, (*serv)->node_IP);
  strcpy((*serv)->succ_TCP, (*serv)->node_TCPs);

}


/*******************************************************************************
 * update_state(server*, int, int, char*, char* )
 *
 * Description: Updates info state about the correspondent successor
 *
 * returns: 1 if well succeed
            0 otherwise
*******************************************************************************/
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


/*******************************************************************************
 * init_fd_parent()
 *
 * Description: creates a TCP file descriptor
 *
 * returns: returns an integer, the file descriptor created
*******************************************************************************/
int init_fd_parent () {

  int fd;

  if((fd=socket(AF_INET,SOCK_STREAM,0)) == -1){
    printf("An error occurred on socket() function!\n");
    exit(1);
  }

  return fd;
}


/*******************************************************************************
 * init_tcp_server(char* , server** ,int )
 *
 * Description: opens a TCP server with socket fd
 *
 * returns: void
*******************************************************************************/
void init_tcp_server(char *port, server **serv, int fd) {

  printf("Initiating TCP server ...\n");

  struct addrinfo hints,*res;

  memset(&hints, 0, sizeof hints);
  hints.ai_family=AF_INET;//IPv4
  hints.ai_socktype=SOCK_STREAM;//TCP socket
  hints.ai_flags=AI_PASSIVE;

  int errcode;

  /* Saving the server's fd */
  (*serv)->fd_tcpS = fd;

  if((errcode = getaddrinfo(NULL, (*serv)->node_TCPs, &hints, &res))!=0){
    perror("An error occurred on getting addresses");
    exit(1);
  }
  if(bind(fd, res->ai_addr, res->ai_addrlen) == -1) {
    perror("An error occurred on binding");
    exit(1);
  }
  if(listen(fd, 5) == -1){
    perror("An error occurred on listening");
    exit(1);
  }
  printf("Starting to listen ...\n");
  freeaddrinfo(res);
}



/*******************************************************************************
 * create_tcp_client(server** , fd_set* , char* )
 *
 * Description: creates a TCP client, sends request messages for different
                 purposes
 *
 * returns: fd of the created socket
*******************************************************************************/
int init_tcp_client(server** serv, fd_set *rfds, char *mode) {

  printf("Initiating TCP client ...\n");

  struct addrinfo hints, *res;
  ssize_t nbytes, nleft, nwritten, nread;

  int fd, n;
  char msg[128], buffer[128];

  fd = init_fd_parent();
  (*serv)->fd_tcpC = fd; // Saving my fd in order to get its value in tcpC and set it to rfds

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  /* Connection with 2nd successor, in case the successor do LEAVE */
  if(strcmp(mode, "SUCCCONF") == 0){
    /* Conneting to server */
    if(getaddrinfo((*serv)->succ2_IP, (*serv)->succ2_TCP, &hints, &res) != 0){
      perror("An error occurred on getting addresses");
      exit(1);
    }
    if(connect(fd, res->ai_addr,res->ai_addrlen) == -1){
      perror("An error occurred in connection");
      exit(1);
    }

    /* Message to be sent to 2nd successor (new successor) */
    sprintf(msg, "SUCCCONF\n");
    if(write(fd, msg, strlen(msg)) == -1)/*error*/{
       perror("Error occurred in writting");
       exit(1);
    }
    printf("Message to be sent to old 2nd successor (new successor): %s\n", msg);

    /* Message to be sent to predecessor (about the new successor) */
    if((*serv)->fd_pred == -1) { /* it occurs only when I got alone, when I have known that also my predecessor closed (see tcpS_recv)*/
      sprintf(msg, "SUCC %d %s %s\n", (*serv)->succ_key, (*serv)->succ_IP, (*serv)->succ_TCP );
      if(write(fd, msg, strlen(msg)) == -1) {
         perror("Error occurred in writting");
         exit(1);
      }
      printf("Message to be sent to predecessor (about new successor): %s\n", msg);
    }

    else {
      sprintf(msg, "SUCC %d %s %s\n", (*serv)->succ_key, (*serv)->succ_IP, (*serv)->succ_TCP );
      if(write((*serv)->fd_pred, msg, strlen(msg)) == -1) {
         perror("Error occurred in writting");
         exit(1);
      }
      printf("Message to be sent to predecessor (about new successor): %s\n", msg);
    }
  }
  /* Else: connect to successor, in case of SENTRY */
  else {
    /* Conneting to server */
    if(getaddrinfo((*serv)->succ_IP, (*serv)->succ_TCP, &hints, &res) != 0){
      perror("An error occurred on getting addresses");
      exit(1);
    }
    if(connect(fd, res->ai_addr,res->ai_addrlen) == -1){
      perror("An error occurred in connection");
      exit(1);
    }

    /* if the purpose is to send a <NEW> type message */
    if(strcmp(mode, "NEW") == 0) {
      /* Sending a request message to new sucessor */
      sprintf(msg, "NEW %d %s %s\n", (*serv)->node_key, (*serv)->node_IP, (*serv)->node_TCPs );
      if(write(fd, msg, strlen(msg)) == -1)/*error*/{
         perror("Error occurred in writting");
         exit(1);
      }
      printf("Message to be sent to new successor: %s\n", msg);

    }
    /* else if the purpose is to send a <SUCC> type message */
    else if(strcmp(mode, "SUCC") == 0) {
      /* Sending a info message about new sucessor to predecessor */
      sprintf(msg, "SUCC %d %s %s\n", (*serv)->succ_key, (*serv)->succ_IP, (*serv)->succ_TCP );
      if(write((*serv)->fd_pred, msg, strlen(msg)) == -1)/*error*/{
         perror("Error occurred in writting");
         exit(1);
      }
      printf("Message to be sent to predecessor (about my successor): %s\n", msg);

      /* And send a confirmation message to new successor */
      sprintf(msg, "SUCCCONF\n");
      if(write(fd, msg, strlen(msg)) == -1)/*error*/{
         perror("Error occurred in writting");
         exit(1);
      }
      printf("Message to be sent: %s\n", msg);
    }
  }

  return fd;
}


/*******************************************************************************
 * tcpS(server** , fd_set )
 *
 * Description: Listens to new connections (new predecessors), accept them, save
                 their info about and send various types of messages
 *
 * returns: file descriptor of new client (predecessor),
            0 in case there is no new connection
*******************************************************************************/
int tcpS(server** serv, fd_set rfds) {

  int fd = (*serv)->fd_tcpS;
  int newfd, errcode, max_clients = 2, counter;
  char resp[128], buffer[128];
  ssize_t n, nw;
  struct sockaddr_in addr; socklen_t addrlen;

  /* auxiliary variables for scanning the buffer (sscanf) */
  int key;
  char ip[20], port[20], first[20];

  /* if the purpose of the message is for SENTRY */
  int isSentry = 0;


  if(FD_ISSET(fd, &rfds)){

    addrlen = sizeof(addr);
    if((newfd = accept(fd, (struct sockaddr*)&addr, &addrlen)) == -1) {
      perror("An error occurred on accept() function");
      exit(1);
    }
    printf("New connection %s:%d with fd: %d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), newfd);

    if((n = read(newfd, buffer, 128))!=0){
      if(n == -1) {
        perror("An error occurred on read() function");
        exit(1);
      }

      /* Reading the buffer until the '\n' character */
      for (int i = 0; i < strlen(buffer); i++) {
        if(buffer[i] == '\n' && i < (strlen(buffer) - 1)) {
          buffer[i+1] = '\0';
        }
      }

      printf("Message received from new client: "/*, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), newfd*/);
      printf("%s\n",buffer);
      // NOTA: INTERPRETAR O BUFFER COM SSCANF, QUE TIPO DE MENSAGEM É, VER A QUESTÃO DO \n (no 'for' de cima)

      /* SENTRY */
      if(strcmp(buffer, "SUCCCONF\n") != 0){
        isSentry = 1;

        sscanf(buffer, "%s %d %s %s", first, &key, ip, port);

        /* SENTRY - write server response in newfd, info about server succ */
        sprintf(resp, "SUCC %d %s %s\n", (*serv)->succ_key, (*serv)->succ_IP, (*serv)->succ_TCP);
        printf("To send as response (about successor): %s\n", resp);
        write(newfd, resp, strlen(resp));

        /* SENTRY - write to predecessor about the new server's client, close TCP session*/
        if((*serv)->fd_pred == -1) printf("Server had no predecessor, no message sent to its old predecessor\n");
        else {
          sprintf(resp, "NEW %d %s %s\n", key, ip, port);
          printf("Message to be sent to old predecessor (about new predecessor): %s\n", resp);
          write((*serv)->fd_pred, resp, strlen(resp));
          printf("Closing the session with old predecessor...\n");
          close((*serv)->fd_pred);
        }
      }
      else {
        /* In case the message is not for SENTRY (for LEAVE instead) */
        if(!isSentry) {
          sprintf(resp, "SUCC %d %s %s\n", (*serv)->succ_key, (*serv)->succ_IP, (*serv)->succ_TCP);
          printf("To send as response (about successor): %s\n", resp);
          write(newfd, resp, strlen(resp));
        }
      }

      /* SENTRY - Saving new client (predecessor) stuff here */
      (*serv)->pred_IP = (char *) realloc((*serv)->pred_IP, (strlen(ip)+1) * sizeof(char));
      (*serv)->pred_TCP = (char *) realloc((*serv)->pred_TCP, (strlen(port)+1) * sizeof(char));
      strcpy((*serv)->pred_IP, ip);
      strcpy((*serv)->pred_TCP, port);

      (*serv)->fd_pred = newfd;

      return newfd;
    }
  }
  return 0;
}


/*******************************************************************************
 * tcpS_recv(server** , fd_set )
 *
 * Description: Reads messages from usual clients (predecessors)
 *
 * returns: void
*******************************************************************************/
void tcpS_recv(server **serv, fd_set rfds){

  char buffer[128];
  ssize_t n;
  struct sockaddr_in addr; socklen_t addrlen;

  if(FD_ISSET((*serv)->fd_pred, &rfds)){
    printf("Aqui tcpS_recv!\n");
    if((n = read((*serv)->fd_pred, buffer, 128))!=0){
      if(n == -1) {
        perror("An error occurred on read() function!");
      }
      printf("from %s:%d   fd: %d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), (*serv)->fd_pred);
      write(1,"Received: ",10); write(1, buffer, n);
      printf("\n");

      /* Interpretate message here and answer to client if needed*/


    }
    /* If my predecessor ends TCP session */
    else if (n == 0) {
      printf("Predecessor closed TCP session!\n");
      close((*serv)->fd_pred);
      (*serv)->fd_pred = -1;
    }//connection closed by peer

    /* Cleaning buffer */
    buffer[0] = '\0';
  }
}



/*******************************************************************************
 * tcpC(server** , fd_set )
 *
 * Description: Reads message from server when there are changes in the ring,
                and ......
 *
 * returns: fd of the created socket
*******************************************************************************/
int tcpC (server** serv, fd_set rfds) {

  char buffer[128] = {'\0'};

  /* auxiliary variables, for scanning the buffer (sscanf) */
  char first[128], ip[128], port[128];
  int key, n;

  /* fd_tcpC cannot be -1  ACHO QUE SE PODE TIRAR O fd_tcpC != -1  !!!!!*/
  if((*serv)->fd_tcpC != -1 && FD_ISSET((*serv)->fd_tcpC, &rfds)){
    printf("Aqui tcpC!\n");
    if((n = read((*serv)->fd_tcpC, buffer, 128)) != 0){
      if (n == -1){
        perror("Not reading");
        exit(1);
      }
      else {
        printf("From server: %s\n", buffer);
        sscanf(buffer, "%s %d %s %s", first, &key, ip, port);

        /* Save my 2nd succesor info */
        if (strcmp(first, "SUCC") == 0){
          (*serv)->succ2_key = key;
          (*serv)->succ2_IP = realloc((*serv)->succ2_IP, (strlen(ip)+1) * sizeof(char));
          strcpy((*serv)->succ2_IP, ip);

          (*serv)->succ2_TCP = realloc((*serv)->succ2_TCP, (strlen(port)+1) * sizeof(char));
          strcpy((*serv)->succ2_TCP, port);
        }

        else if (strcmp(first, "NEW") == 0) {
          close((*serv)->fd_tcpC);

          /* Save first my old successor to 2nd successor */
          (*serv)->succ2_key = (*serv)->succ_key;
          (*serv)->succ2_IP = realloc((*serv)->succ2_IP, (strlen((*serv)->succ_IP)+1) * sizeof(char));
          strcpy((*serv)->succ2_IP, (*serv)->succ_IP);
          (*serv)->succ2_TCP = realloc((*serv)->succ2_TCP, (strlen((*serv)->succ_TCP)+1) * sizeof(char));
          strcpy((*serv)->succ2_TCP, (*serv)->succ_TCP);

          /* Save my new successor */
          (*serv)->succ_key = key;
          (*serv)->succ_IP = realloc((*serv)->succ_IP, (strlen(ip)+1) * sizeof(char));
          strcpy((*serv)->succ_IP, ip);
          (*serv)->succ_TCP = realloc((*serv)->succ_TCP, (strlen(port)+1) * sizeof(char));
          strcpy((*serv)->succ_TCP, port);

          (*serv)->fd_tcpC = init_tcp_client(&(*serv), &rfds, "SUCC");
          printf("fd_tcpc: %d\n", (*serv)->fd_tcpC);
        }
      }
    }
    /* If my successor has closed the TCP session */
    else if(n == 0){
      printf("Successor (key %d) closed TCP session!\n", (*serv)->succ_key);
      close((*serv)->fd_tcpC);

      /* Save old 2nd successor to my new successor */
      (*serv)->succ_key = (*serv)->succ2_key;
      (*serv)->succ_IP = realloc((*serv)->succ_IP, (strlen((*serv)->succ2_IP)+1) * sizeof(char));
      strcpy((*serv)->succ_IP, (*serv)->succ2_IP);
      (*serv)->succ_TCP = realloc((*serv)->succ_TCP, (strlen((*serv)->succ2_TCP)+1) * sizeof(char));
      strcpy((*serv)->succ_TCP, (*serv)->succ2_TCP);

      /* Open a new TCP session with my 2nd successor */
      (*serv)->fd_tcpC = init_tcp_client(&(*serv), &rfds, "SUCCCONF");

    }//connection closed by peer
  }
  return (*serv)->fd_tcpC;
}
