#include <stdlib.h>
#include <stdio.h>
#include "server.h"

struct _server {

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
