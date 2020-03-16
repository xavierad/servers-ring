#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "server.h"

struct _server {

  int *fd;

  int node_key;
  char *node_IP;
  char *node_TCP;
  char *node_UDP;

  int succ_key;
  char *succ_IP;
  char *succ_TCP;

  int succ2_key;
  char *succ2_IP;
  //acho que dá jeito para certas mensagens - a verificar
  char *prev_IP;
  char *prev_TCP;
};


/*******************************************************************************
 * distanceN(int , int, int ) - GETS THE DISTANCE BETWEEN k AND l
 *
 * returns: distance between k and l keys
*******************************************************************************/
int distanceN (int k, int l, int N) {

  return ((l-k) % N);
}


/*******************************************************************************
 * freeServer(server** ) - DEALLOCATES MEMORY ALLOCATED FOR server**
 *
 * returns: void
*******************************************************************************/
void freeServer(server** serv) {

  if((*serv) != NULL) {
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
server* newr(int i, char* ip, char* port) { // assumindo que um servidor só pertence a um anel.

  server *serv = NULL;

  /* allacating memory for the local server */
  serv = (server*) malloc(sizeof(server));
  if(serv == NULL) {
    printf("Something went wrong with creating new ring!\n");
    exit(0);
  }

  /* info assignement */
  serv->node_key = i;
  serv->node_IP = ip;
  serv->node_TCP = port;
  serv->node_UDP = NULL;

  serv->succ_key = 0;
  serv->succ_IP = NULL;
  serv->succ_TCP = NULL;

  serv->succ2_key = 0;
  serv->succ2_IP = NULL;

  return serv;
}


/*******************************************************************************
 * showState(server* ) - SHOWING THE LOCAL SERVER STATE
 *
 * returns: void
*******************************************************************************/
void showState(server* serv) {

  if(serv == NULL) printf("The local server has no ring associated!\n");
  else {
    printf("\n\n------ ABOUT THE LOCAL SERVER ------\n");
    printf("         Key: %d\n", serv->node_key);
    printf("         IP: %s\n", serv->node_IP);
    printf("         TCP port: %s\n", serv->node_TCP);
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
void leave(server* serv) {

  int i;
  for (i = 0; i < 2; i++) close(serv->fd[i]);

}


/*******************************************************************************
 * succLeft(server* )
 *
 * Description: checks if any session is closed. If so,
 * returns: void
*******************************************************************************/











/*
void search_key(int key){

}*/
