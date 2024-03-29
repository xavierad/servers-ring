/******************************************************************************
server.c file:

This file contains all functions related to the local server, the definition
of the server structure where its info state will be recorded and to
communications. The list of the can be found in server.h file.
*******************************************************************************/

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

/*
- Main ideas for communications:
One node machine will be both server and client. Initially the local node will
be its successor (server) or predecessor (client) and 2nd successor as well.

When a new server is created (new command) TCP client/server and UDP server are
initialized.

So, in the server point of view, it will listen in its port and runs tcpS()
function to accept only new connections. Here is where all new info is saved.
Eventually it has to make a response. For old clients, identified by its socket,
the messages are received and interpreted in tcpS_recv() function and
eventually has to make a response.

In the client point of view, it must know the IP and port of the successor
(server). It makes requests through

*/


struct _server
{
  /* My client and server sockets */
  int fd_tcpS; // this to listen to clients
  int fd_tcpC; // this is to get into a ring (sentry, entry)
  int fd_udpS; // this is for UDP connections

  int udp_reply; // a flag that rises when the node is ready to reply through UDP
  struct sockaddr_in addr;
  socklen_t addrlen;


  /* Server state */
  /* about the local */
  int node_key;    // node's key
  char *node_IP;   // node's ip
  char *node_TCPs; // node's port

  /* about the successor */
  int succ_key;   // successor's key
  char *succ_IP;  // successor's ip
  char *succ_TCP; // successor's port

  /* about the 2nd successor */
  int succ2_key;   // 2nd successor's key
  char *succ2_IP;  // 2nd successor's ip
  char *succ2_TCP; // 2nd successor's port

  /* about the predecessor */
  int fd_pred;    // predecessor's socket, to send messages when when appropriated
  int pred_key;   // predecessor's key
  char *pred_IP;  // predecessor's ip
  char *pred_TCP; // predecessor's port

};



/*******************************************************************************
 * distanceN(int , int, int )
 *
 * Description: gets the distance between key 'k' and key 'l'
 *
 * returns: distance between k and l keys
*******************************************************************************/
int distanceN (int k, int l)
{
  int d = ((l-k) % N);

  return (d < 0 ? d + N : d);
}

/*******************************************************************************
 * IsItMine( int k, server** serv)
 *
 * Description: check if the key is on the local server
 *
 * returns: 1 if it is, 0 if not
*******************************************************************************/
int IsItMine( int k, server* serv)
{
  if( serv->node_key == k) return 1;

  return 0;
}

/*******************************************************************************
 * isAlone(server** serv)
 *
 * Description: check if the server is alone or not (a server is alone when its
                  successor and predecessor is himself)
 *
 * returns: 1 if it is, 0 if not
*******************************************************************************/
int isAlone(server *serv) {

  if(serv->node_key == serv->succ_key || serv->node_key == serv->pred_key) return 1;
  else return 0;
}

/*******************************************************************************
 * compare_distance( int k, server* serv)
 *
 * Description: Evaluates the distances between the key the local server and
 *              the key and the succ
 *
 * returns:  1 if we must delegate the search and 0 if succ has the key
*******************************************************************************/
int compare_distance( int k, server* serv)
{
    int dn_succ;
    int dn_me;

    dn_succ = distanceN (k, serv->succ_key);
    dn_me = distanceN (k, serv->node_key);

    if(serv->node_key == serv->succ_key) return 0;
    else if( dn_succ < dn_me) return 0;
    else return 1;
}

/*******************************************************************************
 * k_fndinsucc(int k, server* serv)
 *
 * Description: informs that the key was found in the successor server
 *
 * returns: void
*******************************************************************************/
void k_fndinsucc(int k, server *serv)
{
  printf("Key %d found in successor %d %s %s\n", k, serv->succ_key, serv->succ_IP, serv->succ_TCP);
}

/*******************************************************************************
 * DelegateSearchLocal(server* serv, int target_key)
 *
 * Description: send msg <FND k i i.IP i.TCP> to succ
 *
 * returns: void
*******************************************************************************/
void DelegateSearchLocal(server* serv, int target_key)
{
  printf("Key not found in successor, delegating search to sucessor ...\n");
  char msg[128];
  sprintf(msg, "FND %d %d %s %s\n", target_key, serv->node_key, serv->node_IP, serv->node_TCPs);

  if(write(serv->fd_tcpC, msg, strlen(msg)) == -1)/*error*/
  {
     perror("Error occurred in writting");
     exit(1);
  }

}

/*******************************************************************************
 * freeServer(server** )
 *
 * Description: deallocates all memory associated to server
 *
 * returns: void
*******************************************************************************/
void freeServer(server** serv)
{

  if((*serv) != NULL)
  {

    if((*serv)->node_IP != NULL) free((*serv)->node_IP);
    if((*serv)->node_TCPs != NULL) free((*serv)->node_TCPs);

    if((*serv)->succ_IP != NULL) free((*serv)->succ_IP);
    if((*serv)->succ_TCP != NULL) free((*serv)->succ_TCP);

    if((*serv)->succ2_IP != NULL) free((*serv)->succ2_IP);
    if((*serv)->succ2_TCP != NULL) free((*serv)->succ2_TCP);

    if((*serv)->pred_IP != NULL) free((*serv)->pred_IP);
    if((*serv)->pred_TCP != NULL) free((*serv)->pred_TCP);

    free(*serv);
    *serv = NULL;
  }
}


/*******************************************************************************
 * newr(int ,char* , char* )
 *
 * Description: creates a new ring (basically a new server structure)
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
    printf("Something went wrong with creating new ring");
    exit(0);
  }


  /* Default info assignement, -1 or NULL means empty */
  serv->fd_tcpS = -1;
  serv->fd_tcpC = -1;
  serv->fd_udpS = -1;

  serv->node_key = i;
  serv->node_IP = (char *) malloc((strlen(ip) + 1) * sizeof(char));
  serv->node_TCPs = (char *) malloc((strlen(port) + 1) * sizeof(char));
  strcpy(serv->node_IP, ip);
  strcpy(serv->node_TCPs, port);

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
 * showState(server* )
 *
 * Description: shows the local server state, not all info in server structure
                is needed to be shown
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

    printf("\n------ ABOUT THE SUCCESSOR SERVER ------\n");
    printf("         Key: %d\n", serv->succ_key);
    printf("         IP: %s\n", serv->succ_IP);
    printf("         TCP port: %s\n", serv->succ_TCP);

    printf("\n------ ABOUT THE 2nd SUCCESSOR SERVER ------\n");
    printf("         Key: %d\n", serv->succ2_key);
    printf("         IP: %s\n", serv->succ2_IP);
    printf("         IP: %s\n", serv->succ2_TCP);
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

  if((*serv)->fd_tcpC != -1) close((*serv)->fd_tcpC); // closing TCP session with successor (server)
  if((*serv)->fd_pred != -1) close((*serv)->fd_pred); // closing TCP session with predecessor (client)

  /* Resetting the successor: the default (see newr() function) */
  (*serv)->succ_key = (*serv)->node_key;
  free((*serv)->succ_IP);
  (*serv)->succ_IP = NULL;
  free((*serv)->succ_TCP);
  (*serv)->succ_TCP = NULL;
  (*serv)->succ_IP = malloc( (strlen((*serv)->node_IP)+1) * sizeof(char) );
  (*serv)->succ_TCP = malloc( (strlen((*serv)->node_TCPs)+1) * sizeof(char) );
  strcpy((*serv)->succ_IP, (*serv)->node_IP);
  strcpy((*serv)->succ_TCP, (*serv)->node_TCPs);

  /* Resetting the 2nd successor */
  (*serv)->succ2_key = (*serv)->node_key;
  free((*serv)->succ2_IP);
  (*serv)->succ2_IP = NULL;
  free((*serv)->succ2_TCP);
  (*serv)->succ2_TCP = NULL;
  (*serv)->succ2_IP = malloc( (strlen((*serv)->node_IP)+1) * sizeof(char));
  (*serv)->succ2_TCP = malloc( (strlen((*serv)->node_TCPs)+1) * sizeof(char));
  strcpy((*serv)->succ2_IP, (*serv)->node_IP);
  strcpy((*serv)->succ2_TCP, (*serv)->node_TCPs);

  /* Resetting the predecessor */
  (*serv)->fd_pred = -1;
  (*serv)->pred_key = (*serv)->node_key;
  free((*serv)->pred_IP);
  (*serv)->pred_IP = NULL;
  free((*serv)->pred_TCP);
  (*serv)->pred_TCP = NULL;
  (*serv)->pred_IP = malloc( (strlen((*serv)->node_IP)+1) * sizeof(char));
  (*serv)->pred_TCP = malloc( (strlen((*serv)->node_TCPs)+1) * sizeof(char));
  strcpy((*serv)->pred_IP, (*serv)->node_IP);
  strcpy((*serv)->pred_TCP, (*serv)->node_TCPs);

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
  if(IsItMine(key, (*serv))) {
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
    perror("An error occurred on TCP socket() function!\n");
    exit(1);
  }

  return fd;
}

/*******************************************************************************
 * init_udp_server(char* , server** )
 *
 * Description: opens a UDP server with socket fd, does not receive messages
 *
 * returns: fd to be set in main
*******************************************************************************/
int init_udp_server(char *port, server **serv)
{
  printf("Initiating UDP server ...\n");

  int fd, errcode;

  struct addrinfo hints,*res;

  fd=socket(AF_INET,SOCK_DGRAM,0); //UDP socket
  if(fd==-1) {
    perror("(UDP) An error occurred on socket() function!");
    exit(1);
  }

  /* Save fd to structure in order to get its access for receiving */
  (*serv)->fd_udpS = fd;

  memset(&hints,0,sizeof hints);
  hints.ai_family=AF_INET; // IPv4
  hints.ai_socktype=SOCK_DGRAM; // UDP socket
  hints.ai_flags=AI_PASSIVE;

  errcode=getaddrinfo(NULL, (*serv)->node_TCPs, &hints, &res);
  if(errcode!=0) {
    perror("(UDP) An error occurred on getting addresses");
    exit(1);
  }

  if(bind(fd,res->ai_addr, res->ai_addrlen) == -1) {
    perror("(UDP) An error occurred on binding");
    exit(1);
  }

  freeaddrinfo(res);
  return fd;
}


/*******************************************************************************
 * create_udp_client(server** ,char* , char* )
 *
 * Description: creates a UDP client, sends a request message and make some
                  tries to handle possible lost packages;
                The fd is not set in main because it is not needed, this a
                  temporary connection
 *
 * returns: 1 if got server response (received)
            0 otherwise (not received)
*******************************************************************************/
int init_udp_client(server **serv, char *ip, char *port)
{
  struct addrinfo hints,*res;
  struct sockaddr_in addr;
  struct timeval timeout;
  socklen_t addrlen;
  ssize_t n;
  fd_set rfds;

  int fd,errcode,  maxtries = 5, i;
  char msg[128], buffer[128];

  fd = socket(AF_INET, SOCK_DGRAM, 0);//UDP socket
  if(fd == -1){
    perror("(UDP) An error occurred on socket() function!");
    exit(1);
  }

  FD_ZERO(&rfds);
	FD_SET(fd, &rfds);

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;//IPv4
  hints.ai_socktype = SOCK_DGRAM;//UDP socket

  errcode = getaddrinfo(ip, port, &hints, &res);
  if(errcode != 0) {
    perror("(UDP) An error occurred on getting addresses\n");
    exit(1);
  }

  /* after initiating udp client, message can be sent immediately
  and listen for response (with 5 attempts)*/
  for(i = 0; i < maxtries; i++) {

    sprintf(msg, "EFND %d", (*serv)->node_key);
    n = sendto(fd, msg, strlen(msg), 0, res->ai_addr, res->ai_addrlen);
    if(n==-1) {
       perror("(UDP) Error occurred in sending");
       exit(1);
    }
    printf("(UDP) (Attempt %d/5) ", i+1);
    printf("Message to be sent: %s\n", msg);

    timeout.tv_sec=1;
		timeout.tv_usec=0;

		n=select(fd+1, &rfds, (fd_set*) NULL, (fd_set*) NULL, &timeout);
    if(n==-1){
			perror("(UDP) Select error");
			exit(1);
		}

    if(FD_ISSET(fd, &rfds)) {

      addrlen=sizeof(addr);
      n=recvfrom(fd, buffer, 128, 0, (struct sockaddr*)&addr, &addrlen);
      if(n==-1) {
        perror("(UPD) Error on reading");
        exit(0);
      }
      printf("(UDP) Message received: %s\n", buffer);

      /* Reading the buffer until the '\n' character */
      for (int i = 0; i < strlen(buffer); i++) {
        if(buffer[i] == '\n') {
          buffer[i] = '\0';
        }
      }

      if(buffer[0] == 'E') {
        char first[20], succ_ip[20], succ_port[20];
        int key, succ_key;

        if(5 == sscanf(buffer, "%s %d %d %s %s", first, &key, &succ_key, succ_ip, succ_port)) {
          if(strcmp(first, "EKEY") == 0) {
            /* SENTRY made after this function is called in main*/
            update_state(&(*serv), key, succ_key, succ_ip, succ_port);
          }
          return 1;
        }
        else printf("Message 'EKEY' received from the ring is on a wrong format.\n");
      }
    }
  }
  freeaddrinfo(res);

  printf("Message not delivered!\n");
  return 0;
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

  int fd;
  char msg[128] = {'\0'};

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

  freeaddrinfo(res);
  return fd;
}

/*******************************************************************************
 * tcpS(server** , fd_set )
 *
 * Description: Listens to any new UDP message, if the message is readable, find
                  for the clients successor;
                The search process is made with DelegateSearchLocal and the UDP
                  response with tcpS (next function)
 *
 * returns: void
*******************************************************************************/
void udpS(server** serv, fd_set rfds) {

  char buffer[128];
  ssize_t n;

  if(FD_ISSET((*serv)->fd_udpS, &rfds)){

    (*serv)->addrlen = sizeof((*serv)->addr);

    n=recvfrom((*serv)->fd_udpS, buffer, 128, 0, (struct sockaddr*)&((*serv)->addr), &((*serv)->addrlen));
    if(n==-1) {
      perror("(UDP) Error on reading\n");
      exit(0);
    }
    printf("(UDP) Received from a node through UDP: %s\n", buffer);

    /* Reading the buffer until the '\n' character */
    for (int i = 0; i < strlen(buffer); i++) {
      if(buffer[i] == '\n') {
        buffer[i] = '\0';
      }
    }

    if(buffer[0] == 'E') {
      char first[20];
      int target_key;

      if(2 == sscanf(buffer, "%s %d", first, &target_key)) {
        if(strcmp(first, "EFND") == 0) {

          /* do FIND tasks */
          if(IsItMine(target_key, (*serv)) == 0) {
            int delegate = compare_distance(target_key, (*serv));

            /* Successor server has the key */
            if(delegate == 0){
              char msg[128] = {'\0'};

              k_fndinsucc(target_key, (*serv));

              (*serv)->addrlen = sizeof((*serv)->addr);
              sprintf(msg, "EKEY %d %d %s %s", target_key, (*serv)->succ_key, (*serv)->succ_IP, (*serv)->succ_TCP);
              n = sendto((*serv)->fd_udpS, msg, strlen(msg), 0, (struct sockaddr*)&((*serv)->addr), (*serv)->addrlen);
              if(n==-1) {
                 perror("(UDP) Error occurred in sending");
                 exit(1);
              }
              printf("(UDP) Message to be sent through UDP: %s\n", msg);
            }

            /* delegate the search to successor */
            else{
              /* the server must reply through UDP in tcpS */
              (*serv)->udp_reply = 1;
              DelegateSearchLocal((*serv), target_key);
            }

          }
          else {
            printf("Found the key %d on the local server!\n", target_key);
          }
        }
      }
      else printf("Error composing 'EFND' message.\n");
    }
    // we must not close fd_udpS, because this node must be listening for some entry request
  }

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
  int newfd;
  char resp[128] = {'\0'}, buffer[128] = {'\0'};
  ssize_t n;
  struct sockaddr_in addr; socklen_t addrlen;

  /* auxiliary variables for scanning the buffer (sscanf) */
  int key;
  char ip[20] = {'\0'}, port[20] = {'\0'}, first[20] = {'\0'};

  /* if the purpose of the message is for SENTRY */
  int isSentry = 0;


  if(FD_ISSET(fd, &rfds)){

    addrlen = sizeof(addr);
    if((newfd = accept(fd, (struct sockaddr*)&addr, &addrlen)) == -1) {
      perror("An error occurred on accept() function");
      exit(1);
    }
    printf("New connection %s:%d \n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

    if((n = read(newfd, buffer, 128))!=0){
      if(n == -1) {
        perror("An error occurred on read() function");
        exit(1);
      }

      /* Reading the buffer until the '\n' character */
      for (int i = 0; i < strlen(buffer); i++) {
        if(buffer[i] == '\n') {
          buffer[i] = '\0';
        }
      }

      printf("Message received from new client: ");
      printf("%s\n",buffer);

      /* If incoming message is the type of KEY ..., some server has found the key*/
      if(buffer[0] == 'K') {
        int server_key, target_key;

        if(5 == sscanf(buffer, "%s %d %d %s %s", first, &target_key, &server_key, ip, port)) {
          printf("Key %d found in the server %d %s %s\n", target_key, server_key, ip, port);

          /* if it is to reply through udp connection */
          if((*serv)->udp_reply == 1) {
            char msg[128];

            (*serv)->addrlen = sizeof((*serv)->addr);
            sprintf(msg, "EKEY %d %d %s %s", target_key, server_key, ip, port);
            n = sendto((*serv)->fd_udpS, msg, strlen(msg), 0, (struct sockaddr*)&((*serv)->addr), (*serv)->addrlen);
            if(n==-1) {
               perror("(UDP) Error occurred in sending");
               exit(1);
            }
            printf("(UDP) Message to be sent through UDP: %s\n", msg);

            (*serv)->udp_reply = 0;
          }
        }
        else printf("Error processing 'KEY' message.\n");
      }
      /* SENTRY */
      else if(strcmp(buffer, "SUCCCONF") != 0){
        isSentry = 1;

        if(4 == sscanf(buffer, "%s %d %s %s", first, &key, ip, port)) {

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
        else printf("Error processing message.\n" );
      }
      else {
        /* In case the message is not for SENTRY (for LEAVE instead) */
        if(!isSentry) {
          sprintf(resp, "SUCC %d %s %s\n", (*serv)->succ_key, (*serv)->succ_IP, (*serv)->succ_TCP);
          printf("To send as response (about successor): %s\n", resp);
          write(newfd, resp, strlen(resp));
        }
      }

      /* No need to save new fd for FIND purposes, when some server finds the key k */
      if (strcmp(first, "KEY") != 0) {
        /* SENTRY - Saving new client (predecessor) stuff here */
        (*serv)->pred_key = key;
        (*serv)->pred_IP = (char *) realloc((*serv)->pred_IP, (strlen(ip)+1) * sizeof(char));
        (*serv)->pred_TCP = (char *) realloc((*serv)->pred_TCP, (strlen(port)+1) * sizeof(char));
        strcpy((*serv)->pred_IP, ip);
        strcpy((*serv)->pred_TCP, port);

        (*serv)->fd_pred = newfd;

      }
    }
  }
  if ((*serv)->fd_pred != -1) return (*serv)->fd_pred;
  else return 0; // if predecessor has closed TCP session fd_pred = -1
}


/*******************************************************************************
 * tcpS_recv(server** , fd_set )
 *
 * Description: Reads messages from usual clients (predecessors)
 *
 * returns: void
*******************************************************************************/
void tcpS_recv(server **serv, fd_set rfds){

  char buffer[128], first[20], ip[20], port[20];
  int source_key, target_key, server_key; // target_key: key to be found by source_key
  char msg[128];
  int delegate;
  ssize_t n;

  if(FD_ISSET((*serv)->fd_pred, &rfds)){
    if((n = read((*serv)->fd_pred, buffer, 128))!=0){
      if(n == -1) {
        perror("An error occurred on read() function!");
      }

      /* Reading the buffer until the '\n' character */
      for (int i = 0; i < strlen(buffer); i++) {
        if(buffer[i] == '\n') {
          buffer[i] = '\0';
        }
      }
      printf("Message received from predecessor: %s\n", buffer);

      /* Interpret message here and answer to client if needed*/
      if(buffer[0] == 'F'){
        if( 5 != sscanf(buffer, "%s %d %d %s %s", first, &target_key, &source_key, ip, port)) printf("Error processing message.\n" );
      }
      else if (buffer[0] == 'K') {
        if( 5 != sscanf(buffer, "%s %d %d %s %s", first, &target_key, &server_key, ip, port)) printf("Error processing message.\n" );
      }

      /* FIND */
      if(strcmp(first, "FND") == 0 ) {

        /* do some calcs */
        delegate = compare_distance(target_key, *serv);

        /* the successor server has the key */
        if(delegate == 0) {

          k_fndinsucc(target_key, *serv);

          /* if source_key == pred_key, server sends the msg directly to the fd_pred, no need to create a TCP session */
          if(source_key == (*serv)->pred_key ) {

            sprintf(msg, "KEY %d %d %s %s\n", target_key, (*serv)->succ_key, (*serv)->succ_IP, (*serv)->succ_TCP);
            if(write((*serv)->fd_pred, msg, strlen(msg)) == -1) {
              perror("Error occurred in writting");
              exit(1);
            }
            printf("Message to be sent to predecessor: %s\n", msg);
          }
          /* else if source_key == succ_key, server sends the msg directly to the fd_tcpC, no need to create a TCP session */
          else if(source_key == (*serv)->succ_key) {

            sprintf(msg, "KEY %d %d %s %s\n", target_key, (*serv)->succ_key, (*serv)->succ_IP, (*serv)->succ_TCP);
            if(write((*serv)->fd_tcpC, msg, strlen(msg)) == -1) {
              perror("Error occurred in writting");
              exit(1);
            }
            printf("Message to be sent to successor: %s\n", msg);
          }
          /* else server creates a temporary tcp client with the source */
          else {
            printf("Initiating a temporary TCP session ...\n");
            int temp_fd = init_fd_parent();

            struct addrinfo hints, *res;

            memset(&hints, 0, sizeof hints);
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_STREAM;

            /* Conneting to server */
            if(getaddrinfo(ip, port, &hints, &res) != 0){
              perror("An error occurred on getting addresses");
              exit(1);
            }
            if(connect(temp_fd, res->ai_addr,res->ai_addrlen) == -1){
              perror("An error occurred in connection");
              exit(1);
            }

            sprintf(msg, "KEY %d %d %s %s\n", target_key, (*serv)->succ_key, (*serv)->succ_IP, (*serv)->succ_TCP);
            if( write(temp_fd, msg, strlen(msg)) == -1) {
              perror("Error occurred in writting");
              exit(1);
            }
            printf("Message to be sent: %s\n", msg);

            close(temp_fd);
          }
        }
        else {
          /* if condition not satisfied, delegate search to SUCCESSOR */
          if(write((*serv)->fd_tcpC, buffer, strlen(buffer)) == -1) {
             perror("Error occurred in writting");
             exit(1);
          }
          printf("Message to be sent to successor: %s\n", buffer);
        }
      }
      else if (strcmp(first, "KEY") == 0) {
        printf("Key %d found in the server %d %s %s\n", target_key, server_key, ip, port);

        /* if it is to reply through udp connection */
        if((*serv)->udp_reply == 1) {
          char msg[128];

          (*serv)->addrlen = sizeof((*serv)->addr);
          sprintf(msg, "EKEY %d %d %s %s\n", target_key, server_key, ip, port);
          n = sendto((*serv)->fd_udpS, msg, strlen(msg), 0, (struct sockaddr*)&((*serv)->addr), (*serv)->addrlen);
          if(n==-1) {
             perror("(UDP) Error occurred in sending");
             exit(1);
          }
          printf("(UDP) Message to be sent through UDP: %s\n", msg);

          (*serv)->udp_reply = 0;
        }
      }
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
  int key, n, server_key, target_key;

  /* fd_tcpC cannot be -1  ACHO QUE SE PODE TIRAR O fd_tcpC != -1  !!!!!*/
  if((*serv)->fd_tcpC != -1 && FD_ISSET((*serv)->fd_tcpC, &rfds)){

    if((n = read((*serv)->fd_tcpC, buffer, 128)) != 0){
      if (n == -1){
        perror("Not reading");
        exit(1);
      }
      else {

        /* Reading the buffer until the '\n' character */
        for (int i = 0; i < strlen(buffer); i++) {
          if(buffer[i] == '\n') {
            buffer[i] = '\0';
          }
        }

        printf("From server: %s\n", buffer);
        if(buffer[0] == 'K') {

          if(5 == sscanf(buffer, "%s %d %d %s %s", first, &target_key, &server_key, ip, port)) {
            printf("Key %d found in the server %d %s %s\n", target_key, server_key, ip, port);

            /* if it is to reply through udp connection */
            if((*serv)->udp_reply == 1) {
              char msg[128];

              (*serv)->addrlen = sizeof((*serv)->addr);
              sprintf(msg, "EKEY %d %d %s %s\n", target_key, server_key, ip, port);
              n = sendto((*serv)->fd_udpS, msg, strlen(msg), 0, (struct sockaddr*)&((*serv)->addr), (*serv)->addrlen);
              if(n==-1) {
                 perror("(UDP) Error occurred in sending");
                 exit(1);
              }
              printf("(UDP) Message to be sent through UDP: %s\n", msg);

              (*serv)->udp_reply = 0;
            }
          }
          else printf("Error processing message.\n" );
        }

        else {
          if (4 == sscanf(buffer, "%s %d %s %s", first, &key, ip, port)) {

            /* Save my 2nd succesor info */
            if (strcmp(first, "SUCC") == 0){

              if(key != (*serv)->succ_key) {
                (*serv)->succ2_key = key;
                (*serv)->succ2_IP = realloc((*serv)->succ2_IP, (strlen(ip)+1) * sizeof(char));
                strcpy((*serv)->succ2_IP, ip);

                (*serv)->succ2_TCP = realloc((*serv)->succ2_TCP, (strlen(port)+1) * sizeof(char));
                strcpy((*serv)->succ2_TCP, port);
              }

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
            }
          }
          else printf("Error processing message.\n" );
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
