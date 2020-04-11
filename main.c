
/* comandos úteis:
 - make
 - valgrind --leak-check=yes ./dkt arg1 arg2
*/

/******************************** DÚVIDAS ********************************
1-
2- SIGPIPE ?
****************************************************************************/




/******************************** FALTA FAZER *********************************
2- (Falta check IP e port nos comandos sentry e entry) - à espera do mail do prof
3- Verificar se são feitas todas as condições de erro nas chamadas de sistema (ver os restantes pontos logo antes à bibliografia)
4
5-
6-
****************************************************************************/


/**************** PROJETO DE RCI - 2º SEMESTRE 2019/2020 **********************
 * Feito por: Miguel Carvalho    nº 84141
 *            Xavier Dias        nº 87136
 ******************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "checks.h"
#include "server.h"
#define max(A,B) ((A)>=(B)?(A):(B))



/******************************************************************************
*********************************** MAIN **************************************
******************************************************************************/

int main(int argc, char *argv[]) {

  struct sigaction act;

  int f = 1; // flag to print out "\n >" in the while loop

  /* Command variables */
  char cmd[255] = {'\0'}; // string that receives commands */
  char *token = NULL; // auxiliary string that receives cmd splitted, takes all its arguments */

  /* Server variables */
  char *ip = NULL; // IP address of the local server
  char *port = NULL; // port to be used  */
  server *serv = NULL; // struct server to allocate its state

  /* File descriptors to be set in readfds vector */
  int maxfd=0, fd_parent=0, fd_pred=0, fd_tcpC=0, fd_updS=0;
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

    /* Everything is OK! Let's begin with the application */
    printf("\n____________________________________________________________\n");
    printf("\n APPLICATION INITIALIZED!");
    printf(" | IP addr.: %s  PORT: %s\n", ip, port);
    printf("\n COMMANDS:\n"
    "\n - <new (i)>\n   CREATE A NEW RING (A SERVER WILL OPEN A TCP CONNECTIONS WITH HIMSELF FIRST);\n"
    "\n - <sentry (i) (succ) (succ.IP) (succ.TCP)>\n   SERVER ENTERS IN A RING KNOWING ITS FUTURE SUCCESSOR;\n"
    "\n - <entry (i) (boot) (boot.IP) (boot.TCP)>\n   SERVER ENTERS IN A RING WHERE SERVER WITH BOOT KEY WILL FIND ITS FUTURE SUCCESSOR;\n"
    "\n - <find (j)>\n   FIND A KEY AND KNOW WHICH SERVER HAS IT STORE;\n"
    "\n - <show>\n   SHOW LOCAL SERVER STATE INFO CONCERNING ITSELF, ITS SUCCESSOR AND SECOND SUCCESSOR SERVERS;\n"
    "\n - <leave>\n   LEAVE THE RING;\n"
    "\n - <exit>\n   EXIT THE APPLICATION (IT WILL LEAVE THE RING FIRST).\n"
  );
  }

  memset(&act, 0, sizeof act);
  act.sa_handler = SIG_IGN;
  if(sigaction(SIGPIPE, &act, NULL) == -1) {
    perror("Error in sigaction()");
    exit(1);
  }

  /* Initialization of TCP socket */
  fd_parent = init_fd_parent();

  /* application loop, while the user does not enter "exit" */
  while(strcmp(cmd, "exit\n") != 0 ){

    memset(cmd, '\0', sizeof(cmd)); // setting all values of cmd

    if(f == 1) printf("\n > ");
	  fflush(stdout);

    f = 0;

    /* Adding the file descriptors into readfds and determine maxfd for the select */
    FD_ZERO(&readfds);  // initialize the fd set
    FD_SET(0, &readfds); // add stdin fd (0)

    FD_SET(fd_parent, &readfds);
    maxfd = fd_parent;

    FD_SET(fd_updS, &readfds);
    maxfd = max(maxfd,fd_updS);

    FD_SET(fd_pred, &readfds);
    maxfd = max(maxfd,fd_pred);

    FD_SET(fd_tcpC, &readfds);
    maxfd = max(maxfd,fd_tcpC);

    if (select(maxfd+1, &readfds, (fd_set*) NULL, (fd_set*) NULL, (struct timeval*) NULL) < 0) {
      perror("ERROR in select");
      exit(1);
    }

    /* If a command is typed */
    if (FD_ISSET(0, &readfds)) {

      fgets(cmd, 255, stdin);

      token = strtok(cmd, " "); // retrieve each argument of cmd, separated by a space

      /* VALIDATING THE COMMANDS */

      /* NEW command */
      if(strcmp(token, "new") == 0){

        if(!checkCommand_NEW_FIND(token)) printf("Did you mean something like 'new <i>'?\n");
        else if(serv != NULL) printf("Cannot create a new ring!\n");
        else
        {
          /* Ring (Server) creation stuff here */
          serv = newr(atoi(args[1]), ip, port); // new ring/server creation

          /* Init TCP and UDP sessions */
          init_tcp_server(port, &serv, fd_parent); // The TCP server initialized
          fd_tcpC = init_tcp_client(&serv, &readfds, "NEW"); // TCP session with succ (with myself), I'm also client

          fd_updS = init_udp_server(port, &serv);

          printf("A new ring has been created!\n");
        }
      }

      /* ENTRY command */
      else if(strncmp(token, "entry", 5) == 0){

        if(serv == NULL) printf("No ring created!\n");
        else if(!isAlone(serv)) printf("You cannot do an 'entry' command!\n"); // because the server did not leave or already entered
        else if(!checkCommand_S_ENTRY(token)) printf("Did you mean something like 'entry <i> <boot> <boot.IP> <boot.TCP>'?\n");
        else {

          /* Entry server stuff here: initializes an UDP connection with boot and if got any response initialize TCP with future successor */
          if(init_udp_client(&serv, args[3], args[4]) == 1){

            fd_tcpC = init_tcp_client(&serv, &readfds, "NEW"); // init TCP with known successor

            printf("The new server has entered!\n");
          }
        }
      }

      /* SENTRY command */
      else if(strncmp(token, "sentry", 5) == 0){
        if(serv == NULL) printf("No ring created!\n");
        else if(!isAlone(serv)) printf("You cannot do a 'sentry' command!\n"); // because the server did not leave or already entered
        else if(!checkCommand_S_ENTRY(token)) printf("Did you mean something like 'sentry <i> <succi> <succi.IP> <succi.TCP>'?\n");
        else {

          /* Sentry server stuff here */
          if(!update_state(&serv, atoi(args[1]), atoi(args[2]), args[3], args[4])) printf("Key %s is not the local!\n", args[1]); // update successor
          else{
            /* TCP session with known successor */
            fd_tcpC = init_tcp_client(&serv, &readfds, "NEW");

            printf("The new server has entered!\n");
          }
        }
      }

      /* LEAVE command */
      else if(strcmp(token, "leave\n") == 0){
        if(serv == NULL) printf("No ring created!\n");
        else if(isAlone(serv)) printf("Server is not part of a ring!\n");
        else {

          /* Leave ring stuff here */
          leave(&serv);

          /* Resetting values to be set to readfds */
          fd_pred = 0;
          fd_tcpC = 0;

          printf("Server left!\n");
        }
      }

      /* SHOW command */
      else if(strcmp(token, "show\n") == 0){
        if(serv == NULL) printf("No ring created!\n");
        else {
          printf("Showing server state ...\n");
          showState(serv);
        }
      }

      /* FIND command */
      else if(strcmp(token, "find") == 0){

        if(!checkCommand_NEW_FIND(token)) printf("Did you mean something like 'find <i>'?\n");
        else if(serv == NULL) printf("No ring created!\n");
        else if(atoi(args[1]) >= N ) printf("The maximum key alowed in this ring is %d\n", N-1);
        else {

          /* delegate informs if we have to delegate the search of the key */
          if(IsItMine(atoi(args[1]), serv) == 0) {
            int delegate = compare_distance(atoi(args[1]), serv);

            /* Successor server has the key */
            if(delegate == 0) k_fndinsucc( atoi(args[1]) , serv);

            /* delegate the search to successor */
            else DelegateSearchLocal(serv, atoi(args[1]));

          }
          else {
            printf("Found the key %d on the local server!\n", atoi(args[1]));
          }

        }
      }

      /* EXIT command */
      else if(strcmp(token, "exit\n") == 0 ){
        /* Leave ring stuff first */
        if( serv != NULL) leave(&serv);

        /* Resetting values to be set to readfds */
        fd_pred = 0;
        fd_tcpC = 0;

        printf("You closed the application!\n\n");
      }

      else printf("Command not found!\n");

      if (args != NULL ){
        free(args);
        args = NULL;
      }

      f = 1;

      continue;

    }// if FD_ISSET

    if(serv != NULL) {
      printf("\n");

      /* tcpS_recv only knows if fd_pred is set or not one loop after tcpS returns its value */
      tcpS_recv(&serv, readfds);
      fd_pred = tcpS(&serv, readfds);

      /* Request messages */
      fd_tcpC= tcpC(&serv, readfds);

      /* listen to possible UDP messages */
      udpS(&serv, readfds);

      f = 1;
    }
  }// while cmd not equal to exit

  /* exit and deallocate all memory */
  freeServer(&serv);
  free(port);
  free(ip);

  return 0;
}
