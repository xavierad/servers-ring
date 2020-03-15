/* comandos úteis:
 - make
 - valgrind --leak-check=yes ./dkt arg1 arg2
*/

/******************************** DÚVIDAS ********************************
1-

****************************************************************************/




/******************************** FALTA FAZER *********************************

1- Acabar a verificação de comandos (interface), mensagens (pesquisa de chava, saída, entrada)
2- Falta check IP e port nos comandos sentry e entry
3- Condições de erro nas chamadas de sistema (ver os restantes pontos logo antes à bibliografia)
4-

****************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "checks.h"
#include "server.h"



//////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////  MAIN  ///////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

  char cmd[255] = {'\0'};
  char *token = NULL;

  char *ip = NULL;
  char *port = NULL;
  server *serv = NULL;

  // validating the initiating command: ./dkt <ip> <port>
  if(argc != 3) {
    printf("\nThe command must be in format './dkt <ip> <port>'\n\n");
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

    if(check_Port(port)==0) exit(1);

    // Everything is OK!
    printf("\n____________________________________________________________\n");
    printf("\nApplication initialized!\n");
    printf("\n-IP addr.: %s\n-PORT: %s\n\n", ip, port);
  }


  // application loop
  while(strcmp(cmd, "exit\n") != 0)
  {
    memset(cmd, '\0', sizeof(cmd));

    printf("\n > ");

    if(fgets(cmd, 255, stdin))
    {

      token = strtok(cmd, " ");

      // validating the commands

      //NEW i
      if(strncmp(token, "new", 3) == 0)
      {

        if(!checkCommand_NEW(1, token)) printf("Did you mean something like 'new <i>'?\n");
        else 
        {
          //printf("server id: %s\n", args[0]);
          serv = newr(atoi(args[0]), ip, port);
          printf("A new ring has been created!\n");
        }
      }

      else if(strncmp(token, "entry", 5) == 0)
      {
         /*
        // number of arguments comparison and validating the required integer
        if(!checkCommands(4, token)) printf("Did you mean something like 'entry <i> <boot> <boot.IP> <boot.TCP>'?\n");
        else 
        {
          //do entry server stuff here...

          printf("The new server was entered!\n");        
          
        }*/


      }

      //SENTRY
      else if(strncmp(token, "sentry", 6) == 0)
      {
        /*
        if(!checkCommands(4, token)) printf("Did you mean something like 'sentry <i> <succi> <succi.IP> <succi.TCP>'?\n");
        else 
        {
          //do entry server stuff here...

          printf("The new server was entered!\n");
        }
        */
      }

      else if(strcmp(token, "leave\n") == 0)
      {
       
        printf("Server left!\n");


      }

      else if(strcmp(token, "show\n") == 0)
      {

        printf("Showing server state ...\n");
        showState(serv);

      }

      else if(strncmp(token, "find", 4) == 0)
      {
        /*
        if(!checkCommands(1, token)) printf("Did you mean something like 'find <i>'?\n");
        else {
          //do find server stuff here...

          //printf("Found the server with key %d!\n", key);
        }
        */
      }

      else if(strcmp(token, "exit\n") == 0) printf("You closed the application!\n\n");

      else printf("Command not found!\n");

    }
  }
  freeServer(&serv);
  free(args);
  return 0;
}
