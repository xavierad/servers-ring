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
#include "checks.h"
#include "server.h"



//////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////  MAIN  ///////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

  int servnum = 0; /* counters the number of servers in the ring */
  char cmd[255] = {'\0'}; /* string that receives commands */
  char *token = NULL; /* auxiliary string that receives cmd splitted, takes all its arguments */

  char *ip = NULL; /* IP address of the local server */
  char *port = NULL; /* port to be used  */
  server *serv = NULL; /* struct server to allocate its state --- M: Changed serv to ring
                          since this is supposed to be the core server that supports the ring
                          if this server leaves, what happens to ring??? */

  /* validating the initiating command: ./dkt <ip> <port> */
  if(argc != 3) {
    printf("\nThe command must be in format './dkt <ip> <port>'\n\n");
    exit(0);
  }
  else {
    ip = argv[1]; port = argv[2]; /* ip and port assignement */

    /* first check if IP is in the correct format, see function for details */
    switch(check_IP(argv[1]))
    {
          case 1:
            /* error: IP with larger numbers then intended */
            printf("IP addresses must be in the form (X)(X)X.(X)(X)X.(X)(X)X.(X)(X)X\n\n");
            exit(0);
            break;
          case 2:
            /* error: IP contains letters or other symbols */
            printf("IP addresses must contain only numbers\n\n");
            exit(0);
            break;
          //default:
             //OK
    }

    /* then, check if port is in the correct format, see function for details */
    if(check_Port(port)==0) exit(1);

    /* Everything is OK! */
    printf("\n____________________________________________________________\n");
    printf("\nApplication initialized!\n");
    printf("\n-IP addr.: %s\n-PORT: %s\n\n", ip, port);
  }


  /* application loop */
  while(strcmp(cmd, "exit\n") != 0){
    memset(cmd, '\0', sizeof(cmd)); /* setting all values of cmd */

    printf("\n > ");

    if(fgets(cmd, 255, stdin)){

      token = strtok(cmd, " "); /* retrieve each argument of cmd, separated by a space */

      /* validating the commands */
      if(strcmp(token, "new") == 0){

        if(!checkCommand_NEW_FIND(token)) printf("Did you mean something like 'new <i>'?\n");
        else {

          //do ring creation stuff here...
          if(serv != NULL) printf("Cannot create a new ring!\n");
          else
          {
            serv = newr(atoi(args[1]), ip, port);
            printf("A new ring has been created!\n");
            servnum = 1;
          }
        }
        free(args);
      }

      else if(strncmp(token, "entry", 5) == 0){

        // number of arguments comparison and validating the required integer
        if(!checkCommand_S_ENTRY(token)) printf("Did you mean something like 'entry <i> <boot> <boot.IP> <boot.TCP>'?\n");
        else {
          //do entry server stuff here...



          printf("The new server was entered!\n");
        }
        if(args == NULL) free(args);
      }

      else if(strncmp(token, "sentry", 5) == 0){

        if(!checkCommand_S_ENTRY(token)) printf("Did you mean something like 'sentry <i> <succi> <succi.IP> <succi.TCP>'?\n");
        else {
          //do entry server stuff here...
          //sentry i succi s.IP s.TCP
          


          printf("The new server was entered!\n");
        }
        free(args);
      }

      else if(strcmp(token, "leave\n") == 0){
        if(serv == NULL) printf("No ring created!\n");
        else {
          // do leave ring stuff here

          // fazer mais
          leave(&serv);
          freeServer(&serv);
          printf("Server left!\n");
        }
        free(args);
      }

      else if(strcmp(token, "show\n") == 0){
        if(serv == NULL) printf("No ring created!\n");
        else {
          printf("Showing server state ...\n");
          showState(serv);
        }

      }

      else if(strcmp(token, "find") == 0){

        if(!checkCommand_NEW_FIND(token)) printf("Did you mean something like 'find <i>'?\n");
            else {
          //do find server stuff here...

          printf("Found the server with key %d!\n", atoi(args[1]));
        }
        free(args);
      }

      else if(strcmp(token, "exit\n") == 0) printf("You closed the application!\n\n");

      else printf("Command not found!\n");

    }
  }
  /* exit and deallocate all memory allocated */
  freeServer(&serv);

  return 0;
}
