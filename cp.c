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
  int fd,n;
  ssize_t nbytes,nleft,nwritten,nread;
  char *ptr,buffer[128];
    char in[128];

  fd=socket(AF_INET,SOCK_STREAM,0);//TCP socket
  if(fd==-1)exit(1);//error
  memset(&hints,0,sizeof hints);
  hints.ai_family=AF_INET;//IPv4
  hints.ai_socktype=SOCK_STREAM;//TCP socket

  n=getaddrinfo("127.0.0.1",PORT,&hints,&res);
  if(n!=0)/*error*/exit(1);

  n=connect(fd,res->ai_addr,res->ai_addrlen);
  if(n==-1)/*error*/exit(1);

  while(strcmp(in, "leave \n") != 0)
  {
    memset(in, '\0', sizeof(in));
    memset(buffer, '\0', sizeof(in));
    printf("Enter a string: ");
    if(fgets(in, 128, stdin)) 
    {
      n=write(fd,in,7);
      if(n==-1)/*error*/exit(1);
      n=read(fd,buffer,128);
      if(n==-1)/*error*/exit(1);
      write(1,"echo: ",6); write(1,buffer,n);
    }
  }

  close(fd);
  exit(0);
}

void tcpS() 
{

  struct addrinfo hints,*res;
  int fd,newfd,errcode; ssize_t n,nw;
  struct sockaddr_in addr; socklen_t addrlen;
  char *ptr,buffer[128];

  if((fd=socket(AF_INET,SOCK_STREAM,0))==-1)exit(1);//error

  memset(&hints,0,sizeof hints);
  hints.ai_family=AF_INET;//IPv4
  hints.ai_socktype=SOCK_STREAM;//TCP socket
  hints.ai_flags=AI_PASSIVE;

  if((errcode=getaddrinfo(NULL,PORT,&hints,&res))!=0)/*error*/exit(1);
  if(bind(fd,res->ai_addr,res->ai_addrlen)==-1)/*error*/exit(1);
  if(listen(fd,5)==-1)/*error*/exit(1);

  while(1)
  {
    addrlen=sizeof(addr);
    if((newfd=accept(fd,(struct sockaddr*)&addr,&addrlen))==-1)
    /*error*/exit(1);
    while((n=read(newfd,buffer,128))!=0)
    {
      if(n==-1)/*error*/exit(1);
      ptr=&buffer[0];
      printf("Received: %s\n", buffer);
      while(n>0){
        if((nw=write(newfd,ptr,n))<=0)/*error*/exit(1);
        n-=nw; ptr+=nw;
      }
    }

    close(newfd);
    //if(strcmp(buffer, "leave\n") == 0) break;
  }

  printf("Left!\n");
  freeaddrinfo(res);
  close(fd);
}
