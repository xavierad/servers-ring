#ifndef SERVER_H
#define SERVER_H



typedef struct _server server;

int distanceN (int k, int l, int N);

void freeServer(server** serv);

server* newr(int i, char* ip, char* port);

void showState(server* serv);

void leave(server* serv);



#endif
