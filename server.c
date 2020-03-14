#include <stdlib.h>
#include <stdio.h>
#include "server.h"

// COMO É QUE SE FAZ COM O PORTO? ALGUM TIPO DE ARRAY OU LISTA PARA GUARDAR OS PORTOS?

struct _server {

  int node_key;
  char* node_port;
  char* node_IP;
  int succ_key;
  char* succ_port;
  char* succ_IP;
  int succ2_key;
  char* succ2_port;
  char* succ2_IP;
};


/*******************************************************************************
 * GETTING A DISTANCE BETWEEN k AND l
*******************************************************************************/
int distanceN (int k, int l, int N) {

  return ((l-k) % N);
}

/*******************************************************************************
 * CREATING A NEW RING WITH SERVER WITH KEY i
*******************************************************************************/
void newr(int i, char* ip, char* port) { // assumindo que um servidor só pertence a um anel.

  server *serv = NULL;

  serv = (server*) malloc(sizeof(server));
  if(serv == NULL) {
    printf("Something went wrong with allocating new ring!\n");
    exit(0);
  }

  serv->node_key = i;
  serv->node_IP = ip;
  serv->node_port = port;

  serv->succ_key = 0;
  serv->succ_IP = NULL;
  serv->succ_port = NULL;
  
  serv->succ2_key = 0;
  serv->succ2_IP = NULL;
  serv->succ2_port = NULL;

}
/*
void search_key(int key){

}*/
