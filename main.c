#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>


char cmd[255] = {'\0'};
char *token[3];

char *ip;
char *port;

/******************************** DÚVIDAS ********************************

1- comando dkt após correr o executável, certo?--> acho que não, acho que o comando deve ser ./dkt <ip> <port>, como está feito no que está comentado
2-

*/

int main(int argc, char *argv[]) {

  // validating the comand: dkt <ip> <port>
  if(argc != 3) {

    //write(1, "Invocação da aplicação mal feita! Para invocá-la use: dkt <ip> <port>\n",
    //  strlen("Invocação da aplicação mal feita! Para invocá-la use: dkt <ip> <port>"));
      printf("\nThe command must be in format 'dkt <ip> <port>'\n\n");
      exit(0);
    }
  else {
    ip = argv[1]; port = argv[2];
    //write(1,"Introduzido: ", 13);
    /*printf("Command input: ");
    int i;
    for (i=0; i<strlen(*argv + i); i++) {
      //write(1, *(argv + i), strlen(*(argv + i))); write(1, " ", 1);
      printf("%s ", *(argv + i));
    }
    printf("\n");*/
    printf("\nApplication initialized!\n");
    printf("\nIP addr.: %s\nPORT: %s\n", ip, port);
  }


  // application loop
  while(strcmp(cmd, "exit\n") != 0){
    memset(cmd, '\0', sizeof(cmd));

    if(fgets(cmd, 255, stdin)){

      if(strcmp(cmd, "exit\n") == 0) printf("You closed the application!\n\n");
      else{


      // validating the commands
      //if(strcmp())
      }
    }
  }
  /*

  // validating the comand: dkt <ip> <port>
  while(1) {
    fgets(cmd, 255, stdin);

    token[0] = strtok(cmd, " ");
    int i;
    for(i=1; i<=2; i++) {
      token[i] = strtok(NULL, " ");
      printf("\ntoken %s\n", token[i]);
    }


    if(strcmp(token[0], "dkt") != 0 || token[0] == NULL || token[1][0] == '\n' || token[1] == NULL ||
      token[2] == NULL || token[2][0] == '\n') printf("\nThe command must be in format 'dkt <ip> <port>'\n");
    else {
      printf("\nApplication initialized!\n");
      // getting the ip and port parameters
      ip = token[1]; port = token[2];
      printf("\nIP addr.: %s\nPORT: %s\n", ip, port);
      break;
    }
  }

  // application loop
  while(strcmp(cmd, "exit\n") != 0){
    memset(cmd, '\0', sizeof(cmd));

    if(fgets(cmd, 255, stdin)){

      if(strcmp(cmd, "exit\n") == 0) printf("You closed the application!\n\n");
      else{


        // validating the commands
        //if(strcmp())
      }
    }


  }*/


  return 0;
}
