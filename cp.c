#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include "cp.h"
#include "server.h"

#define PORT "58003"
#define max(A,B) ((A)>=(B)?(A):(B))

/* Ideias para as comunicações
Um nó será servidor e cliente.

Portanto, do ponto de vista de servidor, estará a ouvir no seu porto, corre tcpS.
Eventualmente terá que responder ao cliente, mas para isso é necessário saber
que pedido é. O servidor terá que gurdar o porto com o predecessor e do sucessor

Do ponto de vista de cliente, terá que saber o porto tcp etc do seu servidor, sucessor.
Faz pedidos ao seu sucessor.

*/
void tcpC() {

  struct addrinfo hints,*res;
  ssize_t nbytes,nleft,nwritten,nread;

  int fd,n;
  char *ptr;
  char in[128], buffer[128];

  fd=socket(AF_INET,SOCK_STREAM,0);//TCP socket
  if(fd==-1)exit(1);//error
  memset(&hints,0,sizeof hints);
  hints.ai_family=AF_INET;//IPv4
  hints.ai_socktype=SOCK_STREAM;//TCP socket

  // MUDAR IP E PORTO
  n=getaddrinfo("127.0.0.1",PORT,&hints,&res);
  if(n!=0)/*error*/exit(1);

  n=connect(fd,res->ai_addr,res->ai_addrlen);
  if(n==-1)/*error*/exit(1);

  while(1){
    printf("Enter a string: ");
    if(fgets(in, 128, stdin)) {

      n=write(fd,in,strlen(in));
      if(n==-1)/*error*/{
        printf("Error occurred!\n");
        exit(1);
      }

      n=read(fd,buffer,128);
      if(n==-1)/*error*/{
        printf("Error occurred!\n");
        exit(1);
      }

      // APAGAR ECHO
      write(1,"echo: ",6); write(1,buffer,n);

      /* Clean arrays */
      in[128] = '\0';
      buffer[128] = '\0';
    }
  }


}

void tcpS() {

  struct addrinfo hints,*res;
  int fd,newfd,afd=0, errcode;
  ssize_t n,nw;
  struct sockaddr_in addr; socklen_t addrlen;
  char *ptr,buffer[128];

  fd_set rfds;
  enum {idle,busy} state;
  int maxfd, counter, client_socket[30] = {0}, max_clients = 30;

  if((fd=socket(AF_INET,SOCK_STREAM,0))==-1)exit(1);//error

  memset(&hints,0,sizeof hints);
  hints.ai_family=AF_INET;//IPv4
  hints.ai_socktype=SOCK_STREAM;//TCP socket
  hints.ai_flags=AI_PASSIVE;

  if((errcode=getaddrinfo(NULL,PORT,&hints,&res))!=0)/*error*/exit(1);
  if(bind(fd,res->ai_addr,res->ai_addrlen)==-1)/*error*/exit(1);
  if(listen(fd,5)==-1)/*error*/exit(1);

  state=idle;
  while(1){
    FD_ZERO(&rfds);
    FD_SET(fd,&rfds); //adds tcp socket to rfds
    maxfd=fd; //socket id, fd

    //add child sockets to set
    int i;
    for (i = 0; i < max_clients; i++) {
      //socket descriptor
      afd = client_socket[i];

      //if valid socket descriptor then add to read list
      if(afd > 0) FD_SET(afd, &rfds);

      //highest file descriptor number, need it for the select function
      if(afd > maxfd) maxfd = afd;
    }

    counter=select(maxfd+1,&rfds,(fd_set*)NULL,(fd_set*)NULL,(struct timeval *)NULL);
    if(counter<=0)exit(1);



    if(FD_ISSET(fd,&rfds)){

      addrlen=sizeof(addr);

      if((newfd=accept(fd,(struct sockaddr*)&addr,&addrlen))==-1) {
        printf("Error occurred!\n");
        exit(1);
      }
      printf("New connection %s:%d with fd: %d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), newfd);

      //add new socket to array of sockets
      for (i = 0; i < max_clients; i++) {
          //if position is empty
          if( client_socket[i] == 0 ) {
              client_socket[i] = newfd;
              printf("Adding to list of sockets as %d\n\n" , fd);

              break;
          }
      }
    }

    for(i = 0; i < max_clients; i++) {

      afd = client_socket[i];
      if(FD_ISSET(afd,&rfds)){
        if((n=read(afd, buffer, 128))!=0){
          if(n==-1) exit(1);//error
          printf("from %s:%d   fd: %d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), afd);
          write(1,"Received: ",10); write(1,buffer,n);
          printf("\n");
          //write buffer in afd
          write(afd,buffer,n);
        }
        else if(strcmp(buffer, "exit\n")){
          printf("Closed!\n");
          close(afd);
          client_socket[i] = 0;
        }//connection closed by peer
        // clearing buffer
        buffer[n] = '\0';
      }
    }

  }

  printf("Closing!\n");
  freeaddrinfo(res);
  close(fd);
}
