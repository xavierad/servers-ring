#ifndef CHECKS_H
#define CHECKS_H



extern char **args;

int checkInteger(char *num);

int checkCommand_NEW(int nArgs, char* token);

int check_IP(char* s);

int check_Port(char* p);



#endif