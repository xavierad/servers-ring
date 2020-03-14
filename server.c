#include "server.h"

// COMO É QUE SE FAZ COM O PORTO? ALGUM TIPO DE ARRAY OU LISTA PARA GUARDAR OS PORTOS?

struct _server{
  int node_key;
  char *node_IP;
  char *node_TCP;
  char *node_UDP;
  int succ_key;
  char *succ_IP;
  char *succ_TCP;
  int succ2_key;
  int succ2_IP;
};


/*******************************************************************************
 * GETTING A DISTANCE BETWEEN k AND l
*******************************************************************************/
int distanceN (int k, int l, int N){
  return ((l-k) % N);
}
/*
void new(char* ip, char* port )

void search_key(int key){

}*/
