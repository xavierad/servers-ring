#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "checks.h"

char **args; /* auxiliary array that will contain every arguments after the first */


/*******************************************************************************
 * checkInteger(char *) - CHECKS IF A STRING CONTAINS A NUMBER
 *
 * returns: 1 if in the string there is only integers
            0 if there is some letter
*******************************************************************************/
int checkInteger(char *num) {

  int i=0;

  /* here we need also to check if there is any '\n' or '\0' for ensurement */
  while (i < strlen(num) && num[i] != '\n' && num[i] != '\0'){

    if(num[i] < '0' || num[i] > '9') return 0; /* when the number is not in [0, 9] */
    i++;
  }
  return 1;
}

int checkCommand_NEW_FIND(int mode, char* cmd ) 
{
  // mode indicates if the command is new or find
  
  char *token = strtok(cmd, " "); //first token is the command

  token = strtok(NULL, " "); //get the number
  
  if( token != NULL )
  {
    switch(mode)
    {
      case 1: //NEW
        if(!checkInteger(token)) 
        {
          printf("Second argument is not an integer.\n");
          return 0; /*error*/
        }
        //missing: check if the server number is already in use
        break;

      case 2: //FIND
        if(!checkInteger(token)) 
        {
          printf("Second argument is not an integer.\n");
          return 0; /*error*/
        }
        break;

      default:
         printf("Can't recognize command, please try again.\n");
         return 0; /*error*/
    }
    return 1; /*OK*/
  }
  else
  {
    printf("There is something wrong with the command line. Try <command> <integer>");
    return 0; /*error*/
  }

}

//Checks the inputs for commands entry and sentry
int checkCommand_S_ENTRY(char* cmd)
{
  char *token = strtok(cmd, " "); //first token is the command
  int arg = 0;

  //separate by tokens until the end of the command
  for(;token = strtok(NULL, " "); token != NULL)
  {  
    if( arg >= 4)
    {
      printf("Command with too many arguments.\n");
      return 0; /*error*/
    }

    if(arg<2) /* server keys*/
    {
      if(!checkInteger(token)) 
      {
        printf("At least on of the keys is not an integer.\n");
        return 0; /*error*/
      }
      if(arg == 0) //CHECK IF ALREADY IN USE;
    }
    else if(arg == 2)
    {
      switch(check_IP(argv[1]))
      {
        case 1:
          //error: IP with larger numbers then intended
          printf("IP addresses must be in the form (X)(X)X.(X)(X)X.(X)(X)X.(X)(X)X\n");
          return 0;
          break;
        case 2:
          //error: IP contains letters or other symbols
          printf("IP addresses must contain only numbers\n");
          return 0;
          break;
        //default:
        //OK
      }
    }
    else
    {
      if(check_Port(port)==0) return 0; /*error*/
    }

    arg = arg + 1;

  }
  
  if(arg < 4)
  {
    printf("Too few arguments.\n");
    printf("Command must be in the form <command> <server_key> <server_key> <IP> <port>");
    return 0; /*error*/
  }

  return 1; 
  
}

/*******************************************************************************
 * check_IP(char *) - CHECKS IF A STRING IS IN THE FORM OF IP FORMAT
 *
 * returns: 1 if it is in the wright format
            2 otherwise
*******************************************************************************/
int check_IP(char* s) {


    char *auxs = malloc((strlen(s)+1) * sizeof(char));

    strcpy(auxs, s);
    char *token = strtok(auxs, ".");

    //assuming the IP address is the form (X)(X)X.(X)(X)X.(X)(X)X.(X)(X)X
    if(strcmp(s, token)==0 || strlen(token)>3) /*not acceptable IP format*/ return 1;

    int i=0;
    for(;token = strtok(NULL, "."); token != NULL)
    {
     i++;
     if(strlen(token)>3) return 1;
      /* check if there are letters */
      if(!checkInteger(token)) /*not acceptable IP format*/ return 2;

    }
    if(i<3) return 1;

    free(auxs);
    return 0;
}

/*******************************************************************************
 * check_Port(char *) - CHECKS IF A STRING IS CONTAINS AN INTGER BETWEEN
                          (2^10 + 1) AND 2^16
 *
 * returns: 1 if it is in the above interval
            0 otherwise
*******************************************************************************/
int check_Port(char* p) {

  /* Port must not contain letters and must be between (2^10 + 1) and 2^16 */
  if(!checkInteger(p)){
    printf("Port must not contain letters\n\n");
    return 0;
  }
  else if (atoi(p)<1025 || atoi(p)>65536){
    printf("Port must be between 1025 and 65536\n\n");
    return 0;
  }
  return 1;
}