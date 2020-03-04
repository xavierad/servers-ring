






/******************************** DÚVIDAS ********************************

1- comando dkt após correr o executável, certo?--> acho que não, acho que o comando deve ser ./dkt <ip> <port>, como está feito no que está comentado
2- 0 é incluído?

*/



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
char *token;

char *ip;
char *port;


int checkInteger(char *num){
  int i=0;
  while (i < strlen(num)){
    if(num[i] < '0' || num[i] > '9') return 0;

    printf("Num: %c\n", num[i]);
    i++;
  }
  return 1;
}





int main(int argc, char *argv[]) {

  // validating the initiating command: dkt <ip> <port>
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
    printf("\n-IP addr.: %s\n-PORT: %s\n", ip, port);
  }


  // application loop
  while(strcmp(cmd, "exit\n") != 0){
    memset(cmd, '\0', sizeof(cmd));
    printf("\n");

    if(fgets(cmd, 255, stdin)){

      if(strcmp(cmd, "exit\n") == 0) printf("You closed the application!\n\n");
      else{
        // validating the commands
        token = strtok(cmd, " ");
        //printf("token %s\n", token);

        if(strcmp(token, "new") == 0){
          int nArgs=2;
          int i=0;
          while (token != NULL){
            if(token[0] == '\n') break;
            else {
              token = strtok(NULL, " ");
              //printf("token %s\n", token);
              i++;
            }
          }
          //printf("%d\n", i);
          if(i > nArgs || i < nArgs) printf("The number of arguments is not the expected!\n");
          else{
            //do ring creation stuff here...

            printf("New ring was created!\n");
        }
        }
        else if(strcmp(token, "entry") == 0){
          //do entry server stuff here...

          printf("New server was entered!\n");
        }
        else if(strcmp(token, "sentry") == 0){
          //do stuff here...

          printf("\n");
        }
        else if(strcmp(token, "leave") == 0){


          printf("\n");
        }
        else if(strcmp(token, "show") == 0){


          printf("\n");
        }
        else if(strcmp(token, "find") == 0){


          printf("\n");
        }
        else printf("Command not found!\n");
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
