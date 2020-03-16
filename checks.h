#ifndef CHECKS_H
#define CHECKS_H



extern char **args;

int checkInteger(char *num);

int checkCommand_NEW_FIND(int mode, char* cmd);

int checkCommand_S_ENTRY(char* cmd);

int check_IP(char* s);

int check_Port(char* p);



#endif