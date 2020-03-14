#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "checks.h"

char **args;

int checkInteger(char *num) {

  int i=0;

  while (i < strlen(num) && num[i] != '\n' && num[i] != '\0'){

    // when the number is not in [0, 9]
    if(num[i] < '0' || num[i] > '9') return 0;
    i++;
  }
  return 1;
}


int checkCommands(int nArgs, char* token) {

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

/* CHECK INPUT ip */
int check_IP(char* s) {

    char *token;
    char *auxs = malloc(strlen(s) * sizeof(char));

    strcpy(auxs, s);
    token = strtok(auxs, ".");

    //assuming the IP address is the form (X)(X)X.(X)(X)X.(X)(X)X.(X)(X)X
    if(strcmp(s, token)==0 || strlen(token)>3) /*not acceptable IP format*/ return 1;

    int i=0;
    for(;token = strtok(NULL, "."); token != NULL)
    {
     i++;
     if(strlen(token)>3) return 1;
      //check if there are letters
      if(!checkInteger(token)) /*not acceptable IP format*/ return 2;

    }
    if(i<3) return 1;

    free(auxs);
    free(token);
    return 0;
}

/* CHECK INPUT port */
int check_Port(char* p) {
  
  //Port must not contain letters and must be between (2^10 + 1) and 2^16
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
