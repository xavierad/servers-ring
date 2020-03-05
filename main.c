






/******************************** DÚVIDAS ********************************

1- comando dkt após correr o executável, certo?--> acho que não, acho que o comando deve ser ./dkt <ip> <port>, como está feito no que está comentado
2- É preciso fazer uma verificação para o porto e IP? Se sim, é porque têm um formato próprios, certo?
3- No comando entry, todos os argumentos são números? boot boot.IP boot.TCP
4-

****************************************************************************/




/******************************** A FAZER *********************************

1- Simplificar a verificação para o primeiro comando, talvez pôr mais geral e que dê para todas

****************************************************************************/

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

  while (i < strlen(num) && num[i] != '\n' && num[i] != '\0'){

    // when the number is not in [0, 9]
    if(num[i] < '0' || num[i] > '9') return 0;
    i++;
  }
  return 1;
}







int main(int argc, char *argv[]) {

  // validating the initiating command: dkt <ip> <port>
  if(argc != 3) {
    printf("\nThe command must be in format 'dkt <ip> <port>'\n\n");
    exit(0);
  }
  else {
    ip = argv[1]; port = argv[2];

    printf("\n____________________________________________________________\n");
    printf("\nApplication initialized!\n");
    printf("\n-IP addr.: %s\n-PORT: %s\n", ip, port);
  }


  // application loop
  while(strcmp(cmd, "exit\n") != 0){
    memset(cmd, '\0', sizeof(cmd));

    printf("\n > ");

    if(fgets(cmd, 255, stdin)){

      token = strtok(cmd, " ");

      // validating the commands
      if(strncmp(token, "new", 3) == 0){
        int nArgs = 1;
        int i = 0;
        int isInt = 0;
        int server = 0; //mudar o tipo para char caso for necessário

        // splitting the command until it's NULL, validates it, counts the
        //number of args, checks integer
        for ( ;token = strtok(NULL, " "); token != NULL){

          if(token[0] == '\n') break;
          else {
            i++;
            // whenever there is or not the number of required args, it checks
            //whether after 'new' there is integers
            isInt = checkInteger(token);
            server = atoi(token);
          }
        }

        // number of arguments comparison and validating the required integer
        if(i > nArgs || i < nArgs) printf("Did you mean something like 'new <i>'?\n");
        else if(!isInt) printf("There is no server number!\n");
        else {
          //do ring creation stuff here...

          printf("New ring with server %d was created!\n", server);
        }
      }

      else if(strncmp(token, "entry", 5) == 0){
        int nArgs = 3;
        int i = 0;
        int isInt = 0;
        char *args[nArgs];

        // splitting the command until it's NULL, validates it, counts the
        //number of args, checks integer
        for ( ;token = strtok(NULL, " "); token != NULL){

          if(token[0] == '\n') break;
          else {

            // change if boot boot.IP boot.TCP are integers!!!!
            isInt = checkInteger(token);
            if(isInt) i++;
            printf("isInt? %d\n", isInt);
            args[i] = token;
            printf("args: %s\n", args[i]);
          }
        }

        // number of arguments comparison and validating the required integer
        if(i > nArgs || i < nArgs) printf("Did you mean something like 'entry <i> <boot> <boot.IP> <boot.TCP>'?\n");
        else if(!isInt) printf("There is no server number!\n");
        else {
          //do entry server stuff here...

          printf("New server %s was entered!\n", args[0]);
        }

      }

      else if(strncmp(token, "sentry", 5) == 0){
        //do stuff here...

        printf("\n");
      }

      else if(strcmp(token, "leave\n") == 0){
        printf("Server left!\n");


      }

      else if(strcmp(token, "show\n") == 0){
        printf("Showing server state ...\n");


      }

      else if(strncmp(token, "find", 4) == 0){
        int nArgs = 1;
        int i = 0;
        int isInt = 0;
        int key = 0; //mudar o tipo para char caso for necessário

        for ( ;token = strtok(NULL, " "); token != NULL){

          if(token[0] == '\n') break;
          else {
            i++;
            isInt = checkInteger(token);
            key = atoi(token);
          }
        }

        if(i > nArgs || i < nArgs) printf("Did you mean something like 'find <i>'?\n");
        else if(!isInt) printf("There is no server number!\n");
        else {
          //do Finding stuff here...

          printf("Found the server with key %d!\n", key);
        }
      }

      else if(strcmp(token, "exit\n") == 0) printf("You closed the application!\n\n");

      else printf("Command not found!\n");

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
