
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>


char *ip;
char *port;


int main(int argc, char *argv[]) {

  if(argc != 3) { // comando: dkt <ip> <port>
    write(1, "Invocação da aplicação mal feita! Para invocá-la use: dkt <ip> <port>\n",
      strlen("Invocação da aplicação mal feita! Para invocá-la use: dkt <ip> <port>"));
    exit(0);
  }
  else {
    ip = argv[1]; port = argv[2];
    write(1,"Introduzido: ", 13);
    for (int i=0; i<strlen(*argv + i); i++) {
      write(1, *(argv + i), strlen(*(argv + i))); write(1, " ", 1);
    }
  }

  return 0;
}
