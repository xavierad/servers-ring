#include "server.h"

// COMO É QUE SE FAZ COM O PORTO? ALGUM TIPO DE ARRAY OU LISTA PARA GUARDAR OS PORTOS?

struct _server{
  int node_key;
  // int *node_port;
  int node_IP;
  int succ_key;
  // int *succ_port;
  int succ_IP;
  int succ2_key;
  // int *succ2_port;
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
