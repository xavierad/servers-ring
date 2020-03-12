

/******************************** DÚVIDAS ********************************

1- comando dkt após correr o executável, certo?--> acho que não, acho que o comando deve ser ./dkt <ip> <port>, como está feito no que está comentado
2- É preciso fazer uma verificação para o porto e IP? Se sim, é porque têm um formato próprios, certo?
3- Ao inicializar, o servidor local já pertence por defeito ao anel que consitiui somente ele?
4-

****************************************************************************/




/******************************** A FAZER *********************************

1- Acabar a verificação de comandos (interface), mensagens (pesquisa de chava, saída, entrada)

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
#include <ctype.h>

struct server{
  int node_key;
  int succ_key;
  int succ2_key;
  int node_IP;
  int succ_IP;
  int succ2_IP;
};

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


int checkCommands(int nArgs, char* token){
  int i = 0;
  int isInt = 1;
  char *args[nArgs];
  int server = 0; //mudar o tipo para char caso for necessário

  // splitting the command until it's NULL, validates it, counts the
  //number of args, checks integer
  for ( ;token = strtok(NULL, " "); token != NULL){

    if(token[0] == '\n') break;
    else {
      i++;

      args[i-1] = token;
      if(!checkInteger(args[i-1])) isInt = 0;
      printf("%d\n", isInt);
    }
  }
  // number of arguments comparison and validating the required integer
  if(i > nArgs || i < nArgs) return 0;
  else if(!isInt) {
    printf("There is something wrong in server number!\n");
    return 0;
  }
  else return 1;
}

int check_IP(char* s)
{
    char *token;
    int num = 0;

    int ki;
    int k = 0;
    //while(s[k]!='\0')
    //{
       //separate the IP address into octets
       char *auxs = malloc(strlen(s) * sizeof(char));
       strcpy(auxs, s);
       token = strtok(s, ".");
       printf("toke: %s\n", token);
       printf("s: %s\n",s );
       //assuming the IP address is the form (X)(X)X.(X)(X)X.(X)(X)X.(X)(X)X
       if(strlen(token)>3 || strcmp(auxs, token)==0) /*not acceptable IP format*/ return 1;

       //for(ki = 0; token[ki]!=NULL; ki++)
       for(;token = strtok(NULL, "."); token != NULL)
       {
          //check if there are letters
          if(!checkInteger(token)) /*not acceptable IP format*/ return 2;

       }

  //  }
    //no errors found
    free(auxs);
    return 0;
}

int distanceN (int k, int l, int N){
  return ((l-k) % N);
}


int main(int argc, char *argv[]) {

  // validating the initiating command: dkt <ip> <port>
  if(argc != 3) {
    printf("\nThe command must be in format 'dkt <ip> <port>'\n\n");
    exit(0);
  }
  else {
    ip = argv[1]; port = argv[2];

    switch(check_IP(argv[1]))
    {
          case 1:
            //error: IP with larger numbers then intended
            printf("IP addresses must be in the form (X)(X)X.(X)(X)X.(X)(X)X.(X)(X)X\n\n");
            exit(0);
            break;
          case 2:
            //error: IP contains letters or other symbols
            printf("IP addresses must contain only numbers\n\n");
            exit(0);
            break;
          //default:
             //OK
    }

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

        if(!checkCommands(1, token)) printf("Did you mean something like 'new <i>'?\n");
        else {
          //do ring creation stuff here...

          printf("A New ring has been created!\n");
        }
      }

      else if(strncmp(token, "entry", 5) == 0){

        // number of arguments comparison and validating the required integer
        if(!checkCommands(4, token)) printf("Did you mean something like 'entry <i> <boot> <boot.IP> <boot.TCP>'?\n");
        else {
          //do entry server stuff here...

          printf("The new server was entered!\n");
        }

      }

      else if(strncmp(token, "sentry", 5) == 0){
        if(!checkCommands(4, token)) printf("Did you mean something like 'sentry <i> <succi> <succi.IP> <succi.TCP>'?\n");
        else {
          //do entry server stuff here...

          printf("The new server was entered!\n");
        }
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

  return 0;
}
