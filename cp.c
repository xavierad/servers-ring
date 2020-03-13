// communications protocols file

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <signal.h>
#define max(A,B) ((A)>=(B)?(A):(B))


void tcpC(char* ip, char* port) {

  int fd, newfd, errcode;
  ssize_t n;
  socklen_t addrlen;
  struct addrinfo hints,*res;
  struct sockaddr_in addr;
  char buffer[128];

  struct sigaction act;
  memset(&act,0,sizeof act);
  act.sa_handler=SIG_IGN;

  fd=socket(AF_INET,SOCK_STREAM,0); //TCP socket
  if (fd==-1) exit(1); //error

  memset(&hints,0,sizeof hints);

  hints.ai_family=AF_INET; //IPv4
  hints.ai_socktype=SOCK_STREAM; //TCP socket

  // change the IP
  errcode=getaddrinfo("10.0.0.167",port,&hints,&res);

  if(n!=0)/*error*/exit(1);
  n=connect(fd,res->ai_addr,res->ai_addrlen);
  if(n==-1)/*error*/exit(1);

  // a loop in order to keep open the tcp session
  int i;
  for (;;) {
    bzero(buffer, sizeof(buffer));
    printf("Enter the string : ");
    i = 0;
    while ((buffer[i++] = getchar()) != '\n');
    n=write(fd,buffer,sizeof(buffer));

    if(sigaction(SIGPIPE,&act,NULL)==-1)/*error*/exit(1);

    n=read(fd,buffer,128);
    if(n==-1)/*error*/exit(1);
    write(1,"Echo: ",6); write(1,buffer,n);


    if ((strncmp(buffer, "exit", 4)) == 0) {
        printf("Client Exit...\n");
        break;
    }
  }


  freeaddrinfo(res);
  close(fd);
}

void tcpS(char* port) {

  int fd, newfd, errcode,afd=0,maxfd,counter;
  ssize_t n;
  socklen_t addrlen;
  struct addrinfo hints,*res;
  struct sockaddr_in addr;
  char buffer[128];
  enum {idle,busy} state;

  fd_set rfds;

  fd=socket(AF_INET,SOCK_STREAM,0); //TCP socket
  if (fd==-1) exit(1); //error

  memset(&hints,0,sizeof hints);

  hints.ai_family=AF_INET; //IPv4
  hints.ai_socktype=SOCK_STREAM; //TCP socket
  hints.ai_flags=AI_PASSIVE;

  errcode=getaddrinfo(NULL,port,&hints,&res);
  if((errcode)!=0)/*error*/exit(1);

  n=bind(fd,res->ai_addr,res->ai_addrlen);
  if(n==-1) /*error*/ exit(1);

  if(listen(fd,5)==-1)/*error*/exit(1);

  /*while(1){
    addrlen=sizeof(addr);
    if((newfd=accept(fd,(struct sockaddr*)&addr, &addrlen))==-1) exit(1);

    n=read(newfd,buffer,128);
    if(n==-1)exit(1);
    write(1,"Received: ",10);write(1,buffer,n);
    n=write(newfd,buffer,n);
    if(n==-1)exit(1);
    close(newfd);
  }*/

  state=idle;
  while(1){
    FD_ZERO(&rfds);
    FD_SET(fd,&rfds); //adds tcp socket to rfds
    maxfd=fd; //socket id, fd

    if(state==busy){
      FD_SET(afd,&rfds);
      maxfd=max(maxfd,afd);
    }

    counter=select(maxfd+1,&rfds,(fd_set*)NULL,(fd_set*)NULL,(struct timeval *)NULL);
    if(counter<=0)exit(1);

    if(FD_ISSET(fd,&rfds)){
      addrlen=sizeof(addr);

      if((newfd=accept(fd,(struct sockaddr*)&addr,&addrlen))==-1) exit(1);

      switch(state){
        case idle:
          afd=newfd;
          state=busy;
          break;
        case busy: //write “busy\n” in newfd
          write(newfd,"Busy\n",5);
          close(newfd);
          break;
      }
    }
    if(FD_ISSET(afd,&rfds)){
      if((n=read(afd,buffer,128))!=0){
        if(n==-1) exit(1);//error

        write(1,"Received: ",10);write(1,buffer,n);
        //write buffer in afd
        write(afd,buffer,n);
      }
      else{
        close(afd);
        state=idle;
      }//connection closed by peer
    }
  }
  freeaddrinfo(res);
  close(fd);
}
