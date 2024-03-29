/******************************************************************************
checks.c file:

This file contains all functions related to commands. The aim of these functions
is to interpret commands and accept or not them according to specified
requirements
******************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "checks.h"



/* auxiliary array that will contain every arguments after the first */
char **args;


/*******************************************************************************
 * checkInteger(char *)
 *
 * Description: checks if a string contains a number
 *
 * returns: 1 if in the string there is only integers
            0 if there is some letter
*******************************************************************************/
int checkInteger(char *num) {

  int i=0;
  int numDigits = 0;

  /* here we need also to check if there is any '\n' or '\0' for ensurement */
  while (i < (strlen(num)-1)) {

    if(isdigit(num[i])) numDigits ++; // when the number is not in [0, 9]
    i++;
  }
  /* check if all members of the string are digits */
  if(numDigits == (strlen(num)-1)) return 1;
  else return 0;
}


/*******************************************************************************
 * check_IP(char *)
 *
 * Description: checks if a string is in the form of IP format
 *
 * returns: 1 if it is in the wright format
            2 otherwise
*******************************************************************************/
int check_IP(char* s) {

    char *auxs = malloc((strlen(s)+1) * sizeof(char));

    strcpy(auxs, s);
    char *token = strtok(auxs, ".");

    /* assuming the IP address is the form (X)(X)X.(X)(X)X.(X)(X)X.(X)(X)X */
    if(strcmp(s, token)==0 || strlen(token)>3) /*not acceptable IP format*/ {
      free(auxs);
      return 1;
    }

    int i=0;
    for(; (token = strtok(NULL, ".")); /*token != NULL*/) {

     i++;
     if(strlen(token)>3) {
       free(auxs);
       return 1;
     }
      /* check if there are letters */
      if(!checkInteger(token)) /*not acceptable IP format*/ {
        free(auxs);
        return 2;
      }
    }
    if(i<3){
      free(auxs);
      return 1;
    }

    if (auxs != NULL) free(auxs);
    return 0;
}


/*******************************************************************************
 * check_Port(char *) - CHECKS IF A STRING IS CONTAINS AN INTGER BETWEEN
                          (2^10 + 1) AND 2^16
 *
 * Description: checks if a string contains an integer between (2^10 + 1) and
                2^16 (allowed range of ports)
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


/*******************************************************************************
 * checkCommand_NEW_FIND(char* )
 *
 * Description: checks if the command has the number of arguments required (2)
                  and each of them satisfies the requirements;
                For NEW and FIND commands
 *
 * returns: 1 the command satisfies all requirements
            0 otherwise
*******************************************************************************/
int checkCommand_NEW_FIND(char* token ) {

  args = (char**) realloc(args, 3 * sizeof(char*));
  args[0] = token;

  int i = 1;
  for ( ; (token = strtok(NULL, " ")); /*token != NULL*/)
  {
    args[i] = token;

    i++;
    if(i > 2) {
      printf("Too many arguments!\n");
      break;
      return 0;
    }
  }

  if(i < 2 )
  {
    printf("Too few arguments!\n");
    return 0;
  }
  else if( args[1] != NULL )
  {

    if(!checkInteger(args[1]))
    {
      printf("Second argument is not an integer.\n");
      return 0; /*error*/
    }

    /*OK*/
    return 1;
  }
  else
  {
    printf("There is something wrong with the command line. Try <command> <integer>");
    return 0; /*error*/
  }
}


/*******************************************************************************
 * checkCommand_S_ENTRY(char* )
 *
 * Description: checks if the command has the number of arguments required (5)
                  and each of them satisfies the requirements;
                For SENTRY and ENTRY commands
 *
 * returns: 1 the command satisfies all requirements
            0 otherwise
*******************************************************************************/
int checkCommand_S_ENTRY(char* token) {

  args = (char**) realloc(args, 6 * sizeof(char*));
  args[0] = token;

  int i = 1;
  for ( ; (token = strtok(NULL, " ")); /*token != NULL*/)
  {
    args[i] = token;

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

    if(!checkInteger(args[1]))
    {
      printf("Second argument is not an integer.\n");
      return 0; /*error*/
    }
    else if(!checkInteger(args[2]))
    {
      printf("Second argument is not an integer.\n");
      return 0; /*error*/
    }
    else if(check_IP(args[3]) == 1)
    {
      printf("IP addresses must be in the form (X)(X)X.(X)(X)X.(X)(X)X.(X)(X)X\n");
      return 0;
    }
    else if(check_IP(args[3]) == 2)
    {
      printf("IP addresses must contain only numbers\n");
      return 0;
    }
    else if(!check_Port(args[4]))
    {
      printf("Port must be between (2^10 + 1) and 2^16\n");
      return 0;
    }

    strtok(args[4], "\n");
  }
  else
  {
    printf("There is something wrong with the command line. Try <command> <integer>");
    return 0; /*error*/
  }

  /*OK*/
  return 1;
}
