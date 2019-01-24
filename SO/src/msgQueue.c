#include "msgQueue.h"

// ritorna il percorso del file che useremo per la coda
const char *getQueuePath() {
  // nome della coda su cui inviamo e riceviamo messaggi
  char nomeCoda[] = "/.queue.tmp";

  // path
  char *cwd = (char *)malloc(MAXLENPATH * sizeof(char));
  getcwd(cwd, MAXLENPATH);
  strcat(cwd, nomeCoda);

  return cwd;
}

// ritorna la messageId della coda
int getQueueId() {
  key_t key = ftok(getQueuePath(), 65);
  int queueId = msgget(key, 0666 | IPC_CREAT);

  return queueId;
}

// invio messaggio
int sendMessage(int queueId, int argc, char *argv[]) {
  message.msg_type = 1;
  message.argc = argc;
  for (int i = 0; i < argc; i++) {
    strcpy(message.argv[i], argv[i]);
  }

  // invio il messaggio
  msgsnd(queueId, &message, sizeof(message), 0);
}

// funzione che non fa altro che restituire il primo argomento del
// messaggio (che contiene il pid del processo che lo ha chiamato)
int getKillPid() {
  int pid = atoi(message.argv[0]);
  return pid;
}

// controlla che ci siano messaggi nella coda e li legge
void receiveMessage(int queueId) {
  msgrcv(queueId, &message, sizeof(message), 1, 0);
}

// distruggo la coda
void destroyQueue(int queueId) { msgctl(queueId, IPC_RMID, NULL); }
