#questo fa compilare tutti i file di fila
 
#dichiaro le variabili per non doverle riscrivere
CC = gcc
CFLAGS = -c
OFLAGS = -o 
BINDIR = bin
SRCDIR = src

MSGQUEUE.O = $(SRCDIR)/msgQueue.o
MSGQUEUE.C = $(SRCDIR)/msgQueue.c

DEMON.O = $(SRCDIR)/demon.o
DEMON.C = $(SRCDIR)/demon.c

PARSE.O = $(SRCDIR)/parse.o
PARSE.C = $(SRCDIR)/parse.c

STATISTICHE.O = $(SRCDIR)/statistiche.o
STATISTICHE.C = $(SRCDIR)/statistiche.c

RUN = $(BINDIR)/run

#chiamando da terminale make || make all || make build fanno esattamente la stessa cosa


build: 			createbin $(RUN) 

all: 			build cleansrc

createbin:		
				mkdir -p bin

$(RUN): 		$(DEMON.O) $(MSGQUEUE.O) $(PARSE.O) $(STATISTICHE.O)
	 			$(CC) $(OFLAGS)  $@ $^

$(DEMON.O) : 	$(DEMON.C)
				$(CC) $(CFLAGS) $(DEMON.C) $(OFLAGS) $(DEMON.O)

$(MSGQUEUE.O) : $(MSGQUEUE.C)	

$(PARSE.O) : 	$(PARSE.C)

$(STATISTICHE.O):	$(STATISTICHE.c)		

#clean pulisce cartella bin e . 
clean:		cleansrc cleanbin  

cleansrc: 
			rm -rf $(SRCDIR)/*.o

cleanbin:
			rm -rf $(BINDIR)/* 

reset:		clean
			rm -rf $(BINDIR) *.o *.c
