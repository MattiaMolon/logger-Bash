#include "msgQueue.h"
#include "statistiche.h"
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>

// handler che riceve i segnali dal demone
void signalHandler(int sig) {}

// setta il nuovo logfile nel caso l'utente lo voglia cambiare
int settaNuovoLogFile(char logfileDef[], char frmDef[], char frm[],
                      char *tmpArgv[], int fdLog, int fdLogDef) {

  // logfile dei nuovi processi
  char *logfile = (char *)malloc(sizeof(char) * MAXLEN);

  // prima di tutto provo a riaprire il file di default nel caso l'utente
  // lo abbia cancellato
  strcpy(logfile, logfileDef);
  strcat(logfile, ".");
  strcat(logfile, frmDef);
  fdLogDef = open(logfile, O_WRONLY | O_CREAT, 0666);

  logfile = getLogFile(message.argc, tmpArgv);

  if (strcmp(logfile, "\0") != 0) {

    // concateno il nuovo formato (se specificato dall'utente)
    // altrimenti uso quello di default (sia se il formato non viene
    // specificato sia se il formato specificato non è supportato)
    if (strcmp(frm, "\0") != 0) {
      if (strcmp(frm, "txt") == 0 || strcmp(frm, "csv") == 0) {
        //aggiungo estensione
        strcat(logfile, ".");
        strcat(logfile, frm);
      } else {
        printf("Warning: Format type not supported or specified. Using "
               "default format type.\n\n");
        fflush(stdout);
        strcpy(frm, frmDef);
        strcat(logfile, ".");
        strcat(logfile, frmDef);
      }
    } else {
      printf("Warning: Format type not supported or specified. Using "
             "default format type.\n\n");
      fflush(stdout);
      strcpy(frm, frmDef);
      strcat(logfile, ".");
      strcat(logfile, frmDef);
    }

    // apro il file di log già esistente o creo un nuovo file di log (se
    // si cambia logfile o formato)
    fdLog = open(logfile, O_WRONLY | O_CREAT, 0666);
  } else {
    printf("Warning: logfile didn't select. Using default logfile\n");
    fflush(stdout);
    strcpy(logfile, logfileDef);

    if (strcmp(frm, "\0") != 0) {

      if (strcmp(frm, "txt") == 0 || strcmp(frm, "csv") == 0) {
        printf("\n");
        fflush(stdout);
        strcat(logfile, ".");
        strcat(logfile, frm);
        fdLog = open(logfile, O_WRONLY | O_CREAT, 0666);
      } else {
        printf("Warning: Format type not supported or specified. Using "
               "default format type.\n\n");
        fflush(stdout);
        strcpy(frm, frmDef);
        fdLog = fdLogDef;
      }

    } else {

      printf("Warning: Format type not supported or specified. Using "
             "default format type.\n\n");
      fflush(stdout);
      strcpy(frm, frmDef);
      fdLog = fdLogDef;

    }
  }

  //free(logfile);
  return fdLog;
}

// setta il formato e il logfile di default che serviranno a sostituire
// quelli inseriti dall'utente nel caso fossero sbagliati
void settaLogFileDef(int *fdLogDef, char *frmDef, char *logfileDef, char *tmp) {

  // gestisco l'errore nel caso non trovassi il file di log o nel caso
  // l'utente non specifichi il formato, o scriva formati non supportati
  if (strcmp(frmDef, "\0") == 0 ||
      ((strcmp(frmDef, "txt") != 0 && strcmp(frmDef, "csv") != 0)) ||
      strcmp(logfileDef, "\0") == 0) {
    // Errore formato o non specificato o non supportato
    fprintf(stderr, "Error:\n  - You have to specify a supported default "
                    "format file the first time you launch this app "
                    "(Supported format types: txt | csv)\n");
    fprintf(stderr,
            "Usage:\n  - <program name> <(--logfile | --l)=\"default log "
            "file\"> <(--format | --f)=\"default format type\"> <\"command "
            "to execute\">\n");
    kill(getppid(), SIGUSR1);
    exit(0);
  } else {
    // concateno il formato del file al percorso
    strcat(logfileDef, ".");
    strcat(logfileDef, frmDef);

    // apro file di log di default solo se l'utente specificato un formato
    // supportato
    *fdLogDef = open(logfileDef, O_WRONLY | O_CREAT, 0666);
    strcpy(logfileDef, tmp);
    if (*fdLogDef == -1) {
      // Errore, file di log non specificato
      fprintf(stderr, "Error:\n  - You have to specify a default log file "
                      "the first time you launch this app\n");
      fprintf(stderr,
              "Usage:\n  - <program name> <(--logfile | --l)=\"default log "
              "file\"> <(--format | --f)=\"default format type\"> "
              "<\"command to execute\">\n");
      kill(getppid(), SIGUSR1);
      exit(0);
    }
  }
}

int main(int argc, char *argv[]) {

  // cerco di aprire un file per decidere se procedere come demone
  // o come processo normale che comunica col demone
  int fd = open(getQueuePath(), 0666);

  // inizializzo l'handler per i processi che inviano i messaggi
  signal(SIGUSR1, signalHandler);

  if (fd == -1) {

    // se restituisce -1 significa che non esiste ancora il demone
    if (fork() == 0) {

      // OUTPUT DI SICUREZZA DA CANCELLARE!
      printf("in case of danger this is my pid: %d\n", getpid());

      // salvo il fomato con cui si dovrà stampare le statistiche sul file di
      // log
      char *frmDef = (char *)malloc(sizeof(char) * MAXLEN);
      strcpy(frmDef, getFormat(argc, argv));

      // salvo il percorso del file in cui si dovrà scrivere
      char *logfileDef = (char *)malloc(sizeof(char) * MAXLEN);
      char *tmp = (char *)malloc(sizeof(char) * MAXLEN);
      strcpy(logfileDef, getLogFile(argc, argv));
      strcpy(tmp, getLogFile(argc, argv));

      // apro file di log di default
      int fdLogDef;
      settaLogFileDef(&fdLogDef, frmDef, logfileDef, tmp);
      free(tmp);
      // crea coda e prendo il suo ID
      creat(getQueuePath(), 0666);
      int id = getQueueId();

      // sposto il cursore a fine file
      lseek(fdLogDef, 0, SEEK_END);

      // eseguo il comando e stampo le statistiche sul file di log
      char *cmd = (char *)malloc(sizeof(char) * MAXLEN);
      cmd = getCommand(argc, argv);
      eseguiComando(cmd, fdLogDef, getOutOption(argc, argv), frmDef);
      // torno al logfile senza il formato

      // segnalo al padre la fine dell'esecuzione
      kill(getppid(), SIGUSR1);

      if (!getKill(argc, argv)) {
        char **tmpArgv;
        // aspetto altri messaggi da eseguire fino a che non ricevo
        // l'opzione --kill
        do {
          receiveMessage(id);

          // copio message.argv in tmp per
          // passarlo alla funzione getcommand
          tmpArgv = malloc(sizeof(char) * message.argc);
          for (int i = 0; i < message.argc; i++) {
            tmpArgv[i] = malloc(sizeof(char) * strlen(message.argv[i]));
            strcpy(tmpArgv[i], message.argv[i]);
          }

          // controllo se l'utente specifica un nuovo file di log e in caso
          // scrivo su quello di default
          // formato dei nuovi processi
          char *frm = (char *)malloc(sizeof(char) * MAXLEN);
          frm = getFormat(message.argc, tmpArgv);

          int fdLog = settaNuovoLogFile(logfileDef, frmDef, frm, tmpArgv, fdLog,
                                        fdLogDef);

          // metto il cursore a fine file
          lseek(fdLog, 0, SEEK_END);

          // prendo il comando e lo eseguo
          cmd = getCommand(message.argc, tmpArgv);
          eseguiComando(cmd, fdLog, getOutOption(message.argc, tmpArgv), frm);

          // se viene usato il log di default non lo chiudo
          if (fdLog != fdLogDef)
            close(fdLog);

          // segnalo il processo che mi ha inviato il cmd da eseguire
          kill(getKillPid(), SIGUSR1);


          free(tmpArgv);
          free(cmd);
          //free(frm);
        } while (!getKill(message.argc, tmpArgv));
      }

      destroyQueue(id);
      remove(getQueuePath());
    } else {

      // aspetto che mio figlio mi avvisi della fine dell'esecuzione
      pause();
    }
  } else {

    // copio nel primo argomento del messaggio il mio pid per essere
    // killato alla fine dell'esecuzione del programma
    sprintf(argv[0], "%d", getpid());
    sendMessage(getQueueId(), argc, argv);

    // aspetto che mio figlio mi avvisi della fine dell'esecuzione
    pause();
  }

  return 0;
}