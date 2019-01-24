#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

#ifndef MSGQUEUE_H
#define MSGQUEUE_H

// massima lunghezza dei messaggi
#define MAXLENMSG 100
// massima quantità degli argomenti
#define MAXARG 15
// massima lunghezza della path della cartella
#define MAXLENPATH 1024

struct msg_buffer {
  long msg_type;
  int argc;
  char argv[MAXARG][MAXLENMSG];
} message;

const char *getQueuePath();
int getQueueId();
int sendMessage(int queueId, int argc, char *argv[]);
int getKillPid();
void receiveMessage(int queueId);
void destroyQueue(int queueId);

#endif