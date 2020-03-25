
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

  /* Command variables */
  char cmd[255] = {'\0'}; /* string that receives commands */
  char *token = NULL; /* auxiliary string that receives cmd splitted, takes all its arguments */

  /* Server variables */
  char *ip = NULL; /* IP address of the local server */
  char *port = NULL; /* port to be used  */
  server *serv = NULL; /* struct server to allocate its state */

  int maxfd, fd_parent;
  fd_set readfds;

  /* validating the initiating command: ./dkt <ip> <port> */
  if(argc != 3) {
    printf("\nThe command must be in format './dkt <ip> <port>'\n\n");
    exit(0);
  }
  else {
    /* ip and port assignement */
    ip = (char *) malloc((strlen(argv[1]) + 1) * sizeof(char));
    strcpy(ip, argv[1]);

    port = (char *) malloc((strlen(argv[2]) + 1) * sizeof(char));
    strcpy(port, argv[2]);

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
    }

    /* then, check if port is in the correct format, see function for details */
    if(check_Port(port) == 0) exit(1);
    strtok(port, "\n");

    /* Everything is OK! */
    printf("\n____________________________________________________________\n");
    printf("\nApplication initialized!\n");
    printf("\n-IP addr.: %s\n-PORT: %s\n\n", ip, port);

  }
  fd_parent = init_fd_parent();
  maxfd = fd_parent;

  /* application loop */
  while(strcmp(cmd, "exit\n") != 0){

    memset(cmd, '\0', sizeof(cmd)); /* setting all values of cmd */

    printf("\n > ");
	  fflush(stdout);

    FD_ZERO(&readfds);          /* initialize the fd set */
    FD_SET(0, &readfds);        /* add stdin fd (0) */
    FD_SET(fd_parent, &readfds);
    if (select(maxfd+1, &readfds, (fd_set*) NULL, (fd_set*) NULL, (struct timeval*) NULL) < 0) {
      perror("ERROR in select");
    }

    if(serv != NULL) {
      maxfd = tcpS(&serv, &readfds);
      tcpC(serv);
    }

    if (FD_ISSET(0, &readfds)) {

      fgets(cmd, 255, stdin);

      token = strtok(cmd, " "); /* retrieve each argument of cmd, separated by a space */

      /* validating the commands */
      if(strcmp(token, "new") == 0){

        if(!checkCommand_NEW_FIND(token)) printf("Did you mean something like 'new <i>'?\n");
        else if(serv != NULL) printf("Cannot create a new ring!\n");
        else
        {
          /* ring creation stuff here */
          serv = newr(atoi(args[1]), ip, port); /* new ring/server creation */
          printf("A new ring has been created!\n");
          init_tcp_server(port, &serv, fd_parent); /* The TCP server initialized */
        }
        free(args);
      }

      else if(strncmp(token, "entry", 5) == 0){

        // number of arguments comparison and validating the required integer
        if(!checkCommand_S_ENTRY(token)) printf("Did you mean something like 'entry <i> <boot> <boot.IP> <boot.TCP>'?\n");
        else if(serv == NULL) printf("No ring created!\n");
        else {
          //do entry server stuff here...



          printf("The new server has entered!\n");
        }
        free(args);
      }

      else if(strncmp(token, "sentry", 5) == 0){

        if(!checkCommand_S_ENTRY(token)) printf("Did you mean something like 'sentry <i> <succi> <succi.IP> <succi.TCP>'?\n");
        else if(serv == NULL) printf("No ring created!\n");
        else {

          /* entry server stuff here */
          if(!update_state(&serv, atoi(args[1]), atoi(args[2]), args[3], args[4])) printf("Key <i> not the local!\n");
          else{
            /* TCP session with succ, I'm client */
            init_tcp_client(&serv);

            printf("The new server has entered!\n");
          }
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
        else if(serv == NULL) printf("No ring created!\n");
        else {
          //do find server stuff here...

          printf("Found the server with key %d!\n", atoi(args[1]));
        }
        free(args);
      }

      else if(strcmp(token, "exit\n") == 0) printf("You closed the application!\n\n");

      else printf("Command not found!\n");

    }// if FD_ISSET

  }// while cmd not equal to exit

  /* exit and deallocate all memory allocated */
  freeServer(&serv);

  return 0;
}
