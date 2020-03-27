#ifndef SERVER_H
#define SERVER_H



typedef struct _server server;

int distanceN (int k, int l, int N);

void freeServer(server** serv);

server* newr(int i, char* ip, char* port);

void showState(server* serv);

void leave(server** serv);

int update_state(server** serv, int key, int succ_key, char* succ_IP, char* succ_TCP);

int init_fd_parent();

void init_tcp_server(char* port, server** serv, int fd);

int init_tcp_client(server** serv, fd_set* rfds);

int tcpS(server** serv, fd_set rfds);

void tcpS_recv(server** serv, fd_set rfds);

void tcpC(server** serv, fd_set rfds);




#endif
