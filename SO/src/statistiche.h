#include "parse.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#ifndef STATISTICHE_H
#define STATISTICHE_H

// massimo dei comandi che posso trovare in una pipe
#define MAXCMD 20

// massimo buffer per l'output
#define MAXOUTPUT 1000

struct data {
  char data[MAXLEN];
  long int secondi;
  long int nanosecondi;
};

struct statsCmd {
  char comando[MAXCMDLEN];
  char *output;
  struct data inizio;
  struct data fine;
  char diffData[MAXLEN];
  int returnCode;
  char formato[MAXCMDLEN];
};

struct data calcolaTempo();
struct statsCmd getStats(char cmd[]);
void stampaLog(struct statsCmd stats, FILE *streamFd, bool outOption);
void stampaShell(char cmd[]);
void gestisciOrAnd(char *cmd, int fd, bool outOption, char frm[]);
void eseguiComando(char cmd[], int fd, bool outOption, char frm[]);
void stampaTxt(struct statsCmd stats, FILE *streamFd, bool outOption);
void stampaCsv(struct statsCmd stats, FILE *streamFd, bool outOption);
int gestisciPipe(char *cmd, int fd, bool outOption, char frm[]);

#endif
