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

int checkCommand_NEW(int nArgs, char* token )
{

  int i = 0;
  int isInt = 1;
  args = (char**) malloc(nArgs * sizeof(char*));

  // splitting the command until it's NULL, validates it, counts the
  //number of args, checks integer
  for ( ;token = strtok(NULL, " "); token != NULL){

    if(token[0] == '\n') break;
    else {
      i++;

      args[i-1] = token;
      if(!checkInteger(args[i-1])) isInt = 0;
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
