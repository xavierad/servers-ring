#ifndef CHECKS_H
#define CHECKS_H



extern char **args;

int checkInteger(char *num);

int checkCommand_NEW_FIND(char* token);

int checkCommand_S_ENTRY(char* token);

int check_IP(char* s);

int check_Port(char* p);



#endif
