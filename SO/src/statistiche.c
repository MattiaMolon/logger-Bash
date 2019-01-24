#include "statistiche.h"

// funzione che calcola il tempo attuale e lo salva in una stuct
// di tipo data
struct data calcolaTempo() {
  // struct dove salviamo il tempo attuale
  struct data ris;

  // specifico il formato del tempo
  struct timespec ts;

  // prendi dal tempo in time UTC solo quello che
  // mi interessa e lo salvo nel risultato
  timespec_get(&ts, TIME_UTC);
  strftime(ris.data, 100, "%x %X", gmtime(&ts.tv_sec));
  ris.secondi = ts.tv_sec;
  ris.nanosecondi = ts.tv_nsec;

  return ris;
}

// funzione che ritorna l'output del comando
// all'interno di una struct
struct statsCmd getStats(char cmd[]) {
  // creo la struct in cui mi salvo le statistiche
  struct statsCmd stats;
  strcpy(stats.comando, cmd);

  int writeSide = 1; // for write on buffer
  int readSide = 0;  // for read from buffer

  // backup per stdout e stderr
  int stdout_bk = dup(STDOUT_FILENO);
  int stderr_bk = dup(STDERR_FILENO);

  // questa è la pipe
  int pipefd[2];

  // creiamo la pipe su pipefd
  pipe(pipefd);

  // in questo modo tutto quello scritto su stdout
  // finisce direttamente su
  dup2(pipefd[writeSide], STDOUT_FILENO);
  dup2(pipefd[writeSide], STDERR_FILENO);

  // calcolo tempo di inizio
  stats.inizio = calcolaTempo();

  // eseguiamo il comando che verrà scritto
  // nel buffer della pipe
  stats.returnCode = system(cmd);

  // calcolo tempo di fine e calcolo differenza
  stats.fine = calcolaTempo();
  sprintf(stats.diffData, "%ld.%09ld",
          (stats.fine.secondi - stats.inizio.secondi),
          (stats.fine.nanosecondi - stats.inizio.nanosecondi));

  close(pipefd[writeSide]); // chiudo il lato scrittura

  // restauro la situazione iniziale
  dup2(stdout_bk, STDOUT_FILENO);
  dup2(stderr_bk, STDERR_FILENO);

  stats.output = (char *)malloc(sizeof(char) * MAXOUTPUT);

  // leggo dal buffer
  int size;
  size = read(pipefd[readSide], stats.output, MAXOUTPUT);
  stats.output[size] = '\0';
  close(pipefd[readSide]); // chiudo il lato lettura

  return stats;
}

// funzione che stampa le statistiche riguardanti il comando
// e l'output di quest'ultimo
void stampaLog(struct statsCmd stats, FILE *streamFd, bool outOption) {
  // a seconda del formato passato stampo in txt o csv
  if (strcmp(stats.formato, "txt") == 0) {
    stampaTxt(stats, streamFd, outOption);
  } else if (strcmp(stats.formato, "csv") == 0) {
    stampaCsv(stats, streamFd, outOption);
  }
}

// funzione che date le stats di un comando e lo stream, stampa sullo stream
// in formato txt le stats del comando
void stampaTxt(struct statsCmd stats, FILE *streamFd, bool outOption) {

  fprintf(streamFd, "COMANDO = %s\n", stats.comando);
  if (outOption == true) {
    fprintf(streamFd, "OUTPUT\n------------------------------------------------"
                      "----------------\n");
    // stampo messaggio d'errore nel caso di errore
    if (stats.returnCode != 0) {
      // printf("ERROR: - returned code %d\n", stats.returnCode);
      fprintf(streamFd, "ERROR:  - returned code %d\n", stats.returnCode);
      fprintf(streamFd, "%s", stats.output);
    } else {
      fprintf(streamFd, "%s", stats.output);
    }
    fprintf(
        streamFd,
        "----------------------------------------------------------------\n");
  }
  fprintf(streamFd, "Tempo Inizio:                        %s\n",
          stats.inizio.data);
  fprintf(streamFd, "Tempo Fine:                          %s\n",
          stats.fine.data);
  fprintf(streamFd, "Tempo Trascorso:                     %s\n",
          stats.diffData);
  fprintf(streamFd, "Codice di Ritorno:                   %d\n\n\n\n",
          stats.returnCode);
  fflush(streamFd);
}

// funzione che date delle stats e lo stream di un file,
// stampa le stats sullo stream in formato csv
void stampaCsv(struct statsCmd stats, FILE *streamFd, bool outOption) {

  fprintf(streamFd, "COMANDO;%s\n", stats.comando);

  if (outOption == true) {

    if (stats.returnCode != 0) {
      printf("ERROR: - returned code %d\n", stats.returnCode);
      fprintf(streamFd, "ERROR;Return Code %d\n", stats.returnCode);
      fprintf(streamFd, "OUTPUT;\"%s\"\n", stats.output);
    }

    else
      fprintf(streamFd, "OUTPUT;\"%s\"\n", stats.output);
  }

  fprintf(streamFd, "Tempo Inizio;%s\n", stats.inizio.data);
  fprintf(streamFd, "Tempo Fine;%s\n", stats.fine.data);
  fprintf(streamFd, "Tempo Trascorso;%s\n", stats.diffData);
  fprintf(streamFd, "Codice di Ritorno;%d\n\n\n\n", stats.returnCode);
  fflush(streamFd);
}

// stampo output per l'utente
void stampaShell(char cmd[]) { printf("%s\n", cmd); }

// funzione che prende in ingresso un comando formato da pipe,
// le divide in comandi base e li stampa
int gestisciPipe(char *cmd, int fd, bool outOption, char frm[]) {
  // per la pipe
  int writeSide = 1; // for write on buffer
  int readSide = 0;  // for read from buffer

  // inizializzo matrice per mandarla al parsing dei comandi
  char **tmpPipe = (char **)malloc(sizeof(char *) * MAXCMD);
  for (int i = 0; i < MAXCMD; i++) {
    tmpPipe[i] = (char *)malloc(sizeof(char) * MAXCMDLEN);
  }
  int nPipe = parsePipe(cmd, tmpPipe);

  // mi salvo un stdin, per restaurare la situazione iniziale
  // in seguito
  int stdin_bk = dup(STDIN_FILENO);

  int pipefd[2];
  // metto il puntatore a fine file
  lseek(fd, 0, SEEK_END);

  FILE *streamFd = fdopen(fd, "a"); // ... ,appendFile);

  // controllo che non ci siano stati errori nelle pipe
  // da rivedere come gestire l'errore
  struct statsCmd stats;
  stats.returnCode = 0;

  // ciclo sui comandi singoli separati da "|" e ne
  // stampo le statistiche fino a quando non li finisco oppure
  // non trovo un errore
  for (int i = 0; i < nPipe && stats.returnCode == 0; i++) {

    if (strcmp(frm, "txt") == 0) {

      // se ne trova solo una vuol dire che ci sono più
      // di una pipe vicina e quindi a prescindere restituisce errore
      if (nPipe != 1) {
        fprintf(streamFd, "COMANDO PIPE COMPLETO= %s \n", cmd);
        fprintf(streamFd, "SOTTOCOMANDO PIPE ID= %d \n", i + 1);
        fprintf(streamFd, "SOTTO");
        fflush(streamFd);
      }
    } else if (strcmp(frm, "csv") == 0) {
      fprintf(streamFd, "SPECIFICA;RISULTATO\n");

      if (nPipe != 1) {
        fprintf(streamFd, "COMANDO PIPE COMPLETO;%s\n", cmd);
        fprintf(streamFd, "SOTTOCOMANDO PIPE ID; %d\n", i + 1);
        fprintf(streamFd, "SOTTO");
      }
      fflush(streamFd);
    }

    stats = getStats(tmpPipe[i]);
    strcpy(stats.formato, frm);
    stampaLog(stats, streamFd, outOption);

    // reiniziallizzo la pipe
    pipe(pipefd);

    // sostituisco a stdinfileno il lato lettura della pipe
    // cosi quando leggono da stdin leggono dal buffer della pipe
    dup2(pipefd[readSide], STDIN_FILENO);

    // scrivo nel buffer
    write(pipefd[writeSide], stats.output, strlen(stats.output));

    // se non si chiude la parte write della pipe
    // non si puo leggere da stdin e da errore
    close(pipefd[writeSide]);
  }
  stampaShell(stats.output);

  // restauro la situazione iniziale
  dup2(stdin_bk, STDIN_FILENO);
  close(pipefd[readSide]);
  free(tmpPipe);
  return stats.returnCode;
}

// funzione che prende in ingresso un comando formato da && e ||,
// le divide in comandi base e li stampa
void gestisciOrAnd(char *cmd, int fd, bool outOption, char frm[]) {

  // inizializzo matrice per mandarla al parsing dei comandi
  char **tmpCmd = (char **)malloc(sizeof(char *) * MAXCMD*2);
  for (int i = 0; i < MAXCMD; i++) {
    tmpCmd[i] = (char *)malloc(sizeof(char) * MAXCMDLEN);
  }
  int nCmd = parseOrAnd(cmd, tmpCmd);

  // eseguo il primo comando a prescindere controllando
  // se è una pipe oppure no
  struct statsCmd stats;
  if (isAPipe(tmpCmd[0]) && nCmd != 1) {
    stats.returnCode = gestisciPipe(tmpCmd[0], fd, outOption, frm);

  } else {
    stats = getStats(tmpCmd[0]);

    // salvo il formato del comando
    strcpy(stats.formato, frm);
    // sposto il puntatore a fine file
    lseek(fd, 0, SEEK_END);
    // apro lo stream
    FILE *streamFd = fdopen(fd, "a");

    if (strcmp(stats.formato, "csv") == 0)
      fprintf(streamFd, "SPECIFICA;RISULTATO\n");

    stampaLog(stats, streamFd, outOption);
    stampaShell(stats.output);
  }

  // ciclo su tutta la matrice per stampare il resto
  bool finito = false;
  for (int i = 1; i < nCmd && !finito; i++) {

    // controllo se è il caso di cntinuare oppure no
    if ((strcmp(tmpCmd[i], "||") == 0 && stats.returnCode == 0) ||
        (strcmp(tmpCmd[i], "&&") == 0 && stats.returnCode != 0))
      finito = true;

    // altrimenti se sono su una condizione normale
    else if (i % 2 == 0) {

      // controllo se si tratta di una pipe oppure no
      if (isAPipe(tmpCmd[i]))
        stats.returnCode = gestisciPipe(tmpCmd[i], fd, outOption, frm);
      else {
        stats = getStats(tmpCmd[i]);

        // salvo il formato del comando
        strcpy(stats.formato, frm);
        // sposto il puntatore a fine file
        lseek(fd, 0, SEEK_END);
        // apro lo stream
        FILE *streamFd = fdopen(fd, "a");

        if (strcmp(stats.formato, "csv") == 0)
          fprintf(streamFd, "SPECIFICA;RISULTATO\n");

        stampaLog(stats, streamFd, outOption);
        stampaShell(stats.output);
      }
    }
  }


  free(tmpCmd);
};

// //funzione che prende in ingresso il comando da eseguire e il
// //file descriptor del file di log su cui devi scrivere.
// //Esegue i comandi e ne scrive le statistiche all'interno del file di log
void eseguiComando(char cmd[], int fd, bool outOption, char frm[]) {

  // inizializzo matrice per mandarla al parsing dei comandi
  char **tmpCmd = (char **)malloc(sizeof(char *) * MAXCMD);
  for (int i = 0; i < MAXCMD; i++) {
    tmpCmd[i] = (char *)malloc(sizeof(char) * MAXCMDLEN);
  }
  int nCmd = parseComandi(cmd, tmpCmd);

  // ciclo sui comandi singoli separati da ";" e ne
  // stampo le statistiche
  for (int i = 0; i < nCmd; i++) {

    if (isAnOperator(tmpCmd[i])) {
      printf("TROVATO UN OPERATOREH!\n");
      fflush(stdout);
      gestisciOrAnd(tmpCmd[i], fd, outOption, frm);

    } else if (isAPipe(tmpCmd[i])) {
      gestisciPipe(tmpCmd[i], fd, outOption, frm);

    } else {
      struct statsCmd stats = getStats(tmpCmd[i]);

      // salvo il formato del comando
      strcpy(stats.formato, frm);
      // sposto il puntatore a fine file
      lseek(fd, 0, SEEK_END);
      // apro lo stream
      FILE *streamFd = fdopen(fd, "a");

      if (strcmp(stats.formato, "csv") == 0)
        fprintf(streamFd, "SPECIFICA;RISULTATO\n");

      stampaLog(stats, streamFd, outOption);
      stampaShell(stats.output);
    }
  }
  free(tmpCmd);
}