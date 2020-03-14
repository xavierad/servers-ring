#include <stdlib.h>
#include <stdio.h>
#include "server.h"

// COMO É QUE SE FAZ COM O PORTO? ALGUM TIPO DE ARRAY OU LISTA PARA GUARDAR OS PORTOS?

struct _server {

  int node_key;
  char *node_IP;
  char *node_TCP;
  char *node_UDP;

  int succ_key;
  char *succ_IP;
  char *succ_TCP;

  int succ2_key;
  char* succ2_IP;
};


/*******************************************************************************
 * GETTING A DISTANCE BETWEEN k AND l
*******************************************************************************/
int distanceN (int k, int l, int N) {

  return ((l-k) % N);
}

void freeServer(server** serv) {

  if((*serv) != NULL)
  {
    if((*serv)->node_IP != NULL) free((*serv)->node_IP);
    if((*serv)->node_TCP != NULL) free((*serv)->node_TCP);
    if((*serv)->node_UDP != NULL) free((*serv)->node_UDP);

    if((*serv)->succ_IP != NULL) free((*serv)->succ_TCP);
    if((*serv)->succ_TCP != NULL) free((*serv)->succ_TCP);

    if((*serv)->succ2_IP != NULL) free((*serv)->succ2_IP);

    free(*serv);
    *serv = NULL;
  }
}
/*******************************************************************************
 * CREATING A NEW RING WITH SERVER WITH KEY i
*******************************************************************************/
server* newr(int i, char* ip, char* port) { // assumindo que um servidor só pertence a um anel.

  server *serv = NULL;

  serv = (server*) malloc(sizeof(server));
  if(serv == NULL) {
    printf("Something went wrong with creating new ring!\n");
    exit(0);
  }

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


/*
void search_key(int key){

}*/
