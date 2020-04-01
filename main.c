
/* comandos úteis:
 - make
 - valgrind --leak-check=yes ./dkt arg1 arg2
*/

/******************************** DÚVIDAS ********************************
1- A aplicação deve aceitar o comando exit sem que se tenha feito leave ?
2-
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
#define max(A,B) ((A)>=(B)?(A):(B))



//////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////  MAIN  ///////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

  int f = 1; /* flag to print out "\n >" in the while loop*/

  /* Command variables */
  char cmd[255] = {'\0'}; /* string that receives commands */
  char *token = NULL; /* auxiliary string that receives cmd splitted, takes all its arguments */

  /* Server variables */
  char *ip = NULL; /* IP address of the local server */
  char *port = NULL; /* port to be used  */
  server *serv = NULL; /* struct server to allocate its state */

  int left = 1;
  int entry = 0;

  int maxfd, fd_parent, fd_pred=0, fd_tcpC=0;
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
  while(strcmp(cmd, "exit\n") != 0 || left != 1){

    memset(cmd, '\0', sizeof(cmd)); /* setting all values of cmd */

    if(f) printf("\n > ");
	  fflush(stdout);

    f = 0;

    FD_ZERO(&readfds);          /* initialize the fd set */
    FD_SET(0, &readfds);        /* add stdin fd (0) */
    FD_SET(fd_parent, &readfds);
    FD_SET(fd_pred, &readfds);
    maxfd=max(maxfd,fd_pred);

    FD_SET(fd_tcpC, &readfds);
    maxfd=max(maxfd,fd_tcpC);


    if (select(maxfd+1, &readfds, (fd_set*) NULL, (fd_set*) NULL, (struct timeval*) NULL) < 0) {
      perror("ERROR in select");
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
          /* Ring (Server) creation stuff here */
          serv = newr(atoi(args[1]), ip, port); /* new ring/server creation */

          /* Init TCP sessions */
          init_tcp_server(port, &serv, fd_parent); /* The TCP server initialized */
          fd_tcpC = init_tcp_client(&serv, &readfds, "NEW"); /* TCP session with succ (with myself), I'm also client */

          printf("A new ring has been created!\n");
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
          entry = 1;
          left = 0;
        }
        free(args);
      }

      else if(strncmp(token, "sentry", 5) == 0){

        if(serv == NULL) printf("No ring created!\n");
        else if(!left || entry) printf("You cannot do a sentry command!\n");
        else if(!checkCommand_S_ENTRY(token)) printf("Did you mean something like 'sentry <i> <succi> <succi.IP> <succi.TCP>'?\n");
        else {

          /* entry server stuff here */
          if(!update_state(&serv, atoi(args[1]), atoi(args[2]), args[3], args[4])) printf("Key <i> is not the local!\n");
          else{
            /* TCP session with known successor */
            fd_tcpC = init_tcp_client(&serv, &readfds, "NEW");
            printf("The new server has entered!\n");
            entry = 1;
            left = 0;
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
          fd_pred = 0;
          fd_tcpC = 0;
          //freeServer(&serv);
          printf("Server left!\n");
          left = 1;
          entry = 0;
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

      else if(strcmp(token, "exit\n") == 0 && left == 1) printf("You closed the application!\n\n");

      else if(strcmp(token, "exit\n") == 0 && left == 0) printf("You must leave the ring first!\n");

      else printf("Command not found!\n");

      f = 1;
      continue;
    }// if FD_ISSET

    if(serv != NULL) {
      printf("\n");
      tcpS_recv(&serv, readfds);
      fd_pred = tcpS(&serv, readfds);
      fd_tcpC= tcpC(&serv, readfds);

      f = 1;
    }
  }// while cmd not equal to exit

  /* exit and deallocate all memory allocated */
  freeServer(&serv);

  return 0;
}
