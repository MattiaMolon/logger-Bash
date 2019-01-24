#include "parse.h"

// funzione che prende in ingresso argc e argv e restituisce una
// stringa con all'interno il formato del log di output
char *getFormat(int argc, char *argv[]) {

  char *formato = (char *)malloc(sizeof(char) * MAXLEN);
  char *errore = (char *)malloc(sizeof(char) * 2);
  strcpy(errore, "\0");

  // ciclo sugli argomenti fino a che non trovo un format e lo
  // salvo in "formato"
  bool trovato = false;
  for (int i = 1; i < argc; i++) {
    if (strstr(argv[i], "--format=") != NULL ||
        strstr(argv[i], "--f=") != NULL) {
      formato = strchr(argv[i], '=') + 1;
      trovato = true;
    }
  }

  if (trovato == true)
    return formato;
  else
    return errore;
}

// funzione che prende in ingresso argc e argv e restituisce una
// stringa con all'interno il nome del logfile di output
char *getLogFile(int argc, char *argv[]) {

  char *logfile = (char *)malloc(sizeof(char) * MAXLEN);
  char *errore = (char *)malloc(sizeof(char) * 2);
  strcpy(errore, "\0");

  // ciclo sugli argomenti fino a che non trovo un nome di
  // logfile e lo salvo in "logfile"
  bool trovato = false;
  for (int i = 1; i < argc && (trovato == false); i++) {
    if (strstr(argv[i], "--logfile=") != NULL ||
        strstr(argv[i], "--l=") != NULL) {
      logfile = strchr(argv[i], '=') + 1;
      trovato = true;
    }
  }

  if (trovato == true)
    return logfile;
  else
    return errore;
}

// funzione che prende in ingresso argc e argv e restituisce una
// stringa con all'interno i comandi da fare eseguire al demon
char *getCommand(int argc, char *argv[]) {

  char *command = (char *)malloc(sizeof(char) * MAXLEN);
  char *errore = (char *)malloc(sizeof(char) * 2);
  strcpy(errore, "\0");

  // ciclo sugli argomenti fino a che non trovo dei comandi
  // e li salvo in "command"
  bool trovato = false;
  for (int i = 1; i < argc && (trovato == false); i++) {
    if ((argv[i][0] != '-') && (argv[i][1] != '-')) {
      strcpy(command, argv[i]);
      trovato = true;
    }
  }

  if (trovato == true)
    return command;
  else
    return errore;
}

// funzione che prende in ingresso argc e argv e restituisce
// un booleano che specifica se uccidere o meno il demone
bool getKill(int argc, char *argv[]) {

  bool kill = false;

  // ciclo sugli argomenti fino a che non trovo l'opzione kill
  for (int i = 1; i < argc; i++) {
    if (strstr(argv[i], "--kill") != NULL || strstr(argv[i], "--k") != NULL) {
      kill = true;
    }
  }

  return kill;
}

// funzione che prende in ingresso argc e argv e restituisce una
// un bool che specifica se stampare o meno l'output
bool getOutOption(int argc, char *argv[]) {

  bool out = false;

  // ciclo sugli argomenti fino a che non trovo l'opzione --o
  for (int i = 1; i < argc; i++) {
    if (strstr(argv[i], "--o") != NULL || strstr(argv[i], "--output") != NULL) {
      out = true;
    }
  }

  return out;
}

// funzione che prende in input una stringa di comandi
// e una matrice vuota. Restituisce il numero di comandi
// distinti da un ";" trovati all'interno della stringa
// e li scrive all'interno della matrice
int parseComandi(char *cmd, char *comandi[]) {

  // creiamo un token per partizionare la stringa e lo
  // salviamo nella matrice
  char *token = strtok(cmd, ";");
  comandi[0] = token;
  int ncomandi = 0;

  // fino a che non arriviamo all'ultima occorrenza di ";"
  // continuiamo a fare lo stesso processo con i token e la matrice
  while (token != NULL) {
    token = strtok(NULL, ";");
    ncomandi++;
    comandi[ncomandi] = token;
  }

  return ncomandi;
}

// funzione che ritorna true se una stringa è vuota
// o è composta da un numero indefiniti di spazi
// senza alcun carattere, altrimenti ritorna false
bool is_empty(const char *s) {
  while (*s != '\0') {
    if (!isspace((unsigned char)*s))
      return false;
    s++;
  }
  return true;
}

// funzione che prende in input un comando composto da
// pipe e una matrice vuota. scorre il comando e divide il comando
// all'interno in comandi singoli all'interno della matrice
int parsePipe(char *cmd, char *pipe[]) {

  int npipe = 0, i = 0, inizio = 0, conta = 0;
  int dim = strlen(cmd);
  bool errore = false;

  while (i < dim - 1 && !errore) {

    if (cmd[i] == '|') {
      conta++;
    }
    if (conta == 1 && cmd[i] != '|') {
      strncpy(pipe[npipe], &cmd[inizio], i - inizio);
      pipe[npipe][i - inizio - 1] = '\0';
      conta = 0;
      npipe++;
      inizio = i;
    } else if (conta == 2 && cmd[i] != '|')
      conta = 0;
    else if (conta >= 3)
      errore = true;

    i++;
  }

  // inserisce l'ultimo comando all'interno della matrice
  strncpy(pipe[npipe], &cmd[inizio], dim - inizio);
  pipe[npipe][dim - inizio] = '\0';
  npipe++;

  // se trovo una pipe come ultimo carattere do errore
  if (isAPipe(pipe[npipe - 1]))
    errore = true;

  if (is_empty(pipe[0]) == true || errore) {
    npipe = 1;
    strncpy(pipe[0], cmd, dim);
  }

  return npipe;
}

int parseOrAnd(char *cmd, char *orAnd[]) {

  int ncmd = 0, i = 0, inizio = 0, contaPipe = 0, contaAnd = 0;
  int dim = strlen(cmd);
  bool errore = false;

  // scorro la stringa fino al fondo
  // in caso di errore causato da tripla pipe o and esco
  while (i < dim && !errore) {

    // gestisco errore in cui trovo &|
    if (cmd[i] == '|') {
      if (contaAnd == 0)
        contaPipe++;
      else
        errore = true;
    }

    // gestisco errore in cui trovo |&
    if (cmd[i] == '&') {
      if (contaPipe == 0)
        contaAnd++;
      else
        errore = true;
    }

    // controllo le¯doppie pipe, (di quelle singole non mi importa)
    if (contaPipe == 2 && cmd[i] != '|') {
      strncpy(orAnd[ncmd], &cmd[inizio], i - inizio - 1);
      orAnd[ncmd][i - inizio - 2] = '\0';
      contaPipe = 0;
      ncmd++;
      strcpy(orAnd[ncmd], "||");
      ncmd++;
      inizio = i;
    } else if (contaPipe >= 3)
      errore = true;

    // controllo le and
    if (contaAnd == 1 && cmd[i] != '&') { // errore con una sola &
      errore = true;
    } else if (contaAnd == 2 && cmd[i] != '&') {
      strncpy(orAnd[ncmd], &cmd[inizio], i - inizio - 1);
      orAnd[ncmd][i - inizio - 2] = '\0';
      contaAnd = 0;
      ncmd++;
      strcpy(orAnd[ncmd], "&&");
      ncmd++;
      inizio = i;
    } else if (contaAnd >= 3)
      errore = true;

    // nel caso trovi un carattere normale allora resetto tutto
    if (cmd[i] != '&' && !isspace(cmd[i]))
      contaAnd = 0;
    if (cmd[i] != '|' && !isspace(cmd[i]))
      contaPipe = 0;

    i++;
  }

  // inserisce l'ultimo comando all'interno della matrice
  strncpy(orAnd[ncmd], &cmd[inizio], dim - inizio);
  orAnd[ncmd][dim - inizio] = '\0';
  ncmd++;

  // controllo che l'ultimo elemento non sia una | o &
  if ((cmd[dim - 1] == '|' && cmd[dim - 2] == '|') || cmd[dim - 1] == '&')
    errore = true;

  // controlla errore se inizia con una singola pipe
  bool cond = true;
  for (int k = 0; orAnd[0][k] != '\0' && cond == true; k++) {

    if (!isspace(orAnd[0][k])) {
      if (orAnd[0][k] == '|' || orAnd[0][k] == '&')
        errore = true;
      cond = false;
    }
  }

  // se la prima è vuota o ho trovato un errore nel processo
  // metto tutto nella prmima casella e lo gestisco come
  // comando unico
  if (is_empty(orAnd[0]) == true || errore) {
    ncmd = 1;
    strncpy(orAnd[0], cmd, dim);
  }

  return ncmd;
}

// controllo che all'interno della stringa controllo se ci sono due
// operatori | o & vicini
bool isAnOperator(char *cmd) {

  int dim = strlen(cmd);
  int contaAnd = 0, contaPipe = 0;

  for (int i = 0; i < dim - 1; i++) {

    if (cmd[i] == '|') // conto le pipe
      contaPipe++;

    else if (cmd[i] == '&') // conto gli end
      contaAnd++;

    else if (cmd[i] != '&' && cmd[i] != '|' && cmd[i] != ' ') {
      contaPipe = 0;
      contaAnd = 0;
    }

    // se ne trovo due vicini ritorno vero
    if (contaPipe == 2 || contaAnd == 2)
      return true;
  }

  return false;
}

// ritorna true se è una stringa contente una pipe
bool isAPipe(char *cmd) {

  int dim = strlen(cmd);

  // appena trovo una pipe ritorno vero a prescindere
  // ipotizzo sia già stato fatto un controllo sugli operatori
  for (int i = 0; i < dim; i++) {
    if (cmd[i] == '|')
      return true;
  }

  return false;
}
