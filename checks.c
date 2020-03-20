#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
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
  int numDigits = 0;
  /* here we need also to check if there is any '\n' or '\0' for ensurement */
  while (i < (strlen(num)-1)){

    if(isdigit(num[i])) numDigits ++; /* when the number is not in [0, 9] */
    i++;
  }
  if(numDigits ==(strlen(num)-1)) return 1;
  else return 0;
}

int checkCommand_NEW_FIND(char* token ) { // mode indicates if the command is new or find

  args = (char**) malloc(3 * sizeof(char*));
  args[0] = token;

  int i = 1;
  for ( ;token = strtok(NULL, " "); token != NULL){
    args[i] = token;

    i++;
    if(i > 2) {
      printf("Too many arguments!\n");
      break;
      return 0;
    }
  }

  if(i < 2 ) {
    printf("Too few arguments!\n");
    return 0;
  }
  else if( args[1] != NULL ) {

    if(!checkInteger(args[1])) {
      printf("Second argument is not an integer.\n");
      return 0; /*error*/
    }
    return 1; /*OK*/
  }
  else {
    printf("There is something wrong with the command line. Try <command> <integer>");
    return 0; /*error*/
  }
}


//Checks the inputs for commands entry and sentry
int checkCommand_S_ENTRY(char* token) {

  args = (char**) malloc(6 * sizeof(char*));
  args[0] = token;


  int i = 1;
  for ( ;token = strtok(NULL, " "); token != NULL){
    args[i] = token;
    printf("token %s\n", token);

    i++;
    if(i > 5) {
      printf("Too many arguments!\n");
      break;
      return 0;
    }
  }

  if(i < 5 ) {
    printf("Too few arguments!\n");
    return 0;
  }
  else if( args != NULL ) {

    if(!checkInteger(args[1])) {
      printf("Second argument is not an integer.\n");
      return 0; /*error*/
    }
    else if(!checkInteger(args[2])){
      printf("Second argument is not an integer.\n");
      return 0; /*error*/
    }
    else if(check_IP(args[3]) == 1){
      printf("IP addresses must be in the form (X)(X)X.(X)(X)X.(X)(X)X.(X)(X)X\n");
      return 0;
    }
    else if(check_IP(args[3]) == 2){
      printf("IP addresses must contain only numbers\n");
      return 0;
    }
    else if(!check_Port(args[4])){
      printf("Port must be between (2^10 + 1) and 2^16\n");
      return 0;
    }
  }
  else {
    printf("There is something wrong with the command line. Try <command> <integer>");
    return 0; /*error*/
  }


  return 1; /*OK*/
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
