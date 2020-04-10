#ifndef SERVER_H
#define SERVER_H

#define N 16 // maximum length of the ring

typedef struct _server server;

int distanceN (int k, int l);

int IsItMine( int k, server* serv);

int isAlone(server *serv);

int compare_distance( int k, server* serv);

void k_fndinsucc(int k, server* serv);

void DelegateSearchLocal(server* serv, int target_key);

void freeServer(server** serv);

server* newr(int i, char* ip, char* port);

void showState(server* serv);

void leave(server** serv);

int update_state(server** serv, int key, int succ_key, char* succ_IP, char* succ_TCP);

int init_fd_parent();

int init_udp_server(char *port, server **serv);

int init_udp_client(server **serv, char *ip, char *port);

void init_tcp_server(char* port, server** serv, int fd);

int init_tcp_client(server** serv, fd_set* rfds, char *mode);

void udpS(server** serv, fd_set rfds);

int tcpS(server** serv, fd_set rfds);

void tcpS_recv(server** serv, fd_set rfds);

int tcpC(server** serv, fd_set rfds);




#endif
