#ifndef SERVER_H
#define SERVER_H



typedef struct _server server;

int distanceN (int k, int l, int N);

void freeServer(server** serv);

server* newr(int i, char* ip, char* port);

void showState(server* serv);

void leave(server** serv);

int update_state(server** serv, int key, int succ_key, char* succ_IP, char* succ_TCP);

void tcpS(server** serv);

void init_tcp_server(char* port, server** serv);

void init_tcp_client(server* serv);


/*
void tcpC(char* ip, char* port);

void tcpS(char* port, server** serv);*/



#endif
