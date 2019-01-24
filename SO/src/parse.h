#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef PARSE_H
#define PARSE_H

// definizione di booleano
#define bool int
#define false 0
#define true 1

// definizione della massima lunghezza
// delle stringhe e dei comandi
#define MAXLEN 200
#define MAXCMDLEN 100

char *getFormat(int argc, char *argv[]);
char *getLogFile(int argc, char *argv[]);
char *getCommand(int argc, char *argv[]);
bool getKill(int argc, char *argv[]);
bool getOutOption(int argc, char *argv[]);
bool is_empty(const char *s);
bool isAPipe(char *cmd);
bool isAnOperator(char *cmd);
int parseComandi(char *cmd, char *comandi[]);
int parsePipe(char *cmd, char *pipe[]);
int parseOrAnd(char *cmd, char *orAnd[]);

#endif