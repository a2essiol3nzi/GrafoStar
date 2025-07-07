# compilatori
JC= javac
CC= gcc
CFLAGS= -std=c11 -Wall -Wextra -O3 -g -pthread
LDLIBS= -lm -pthread

# regole di compilazione
EXECS= cammini.out CreaGrafo.class

all: $(EXECS)

# Compilazione programma C
cammini.out: CGraph/cammini.o CGraph/utils.o CGraph/xerrori.o
	$(CC) $^ $(LDLIBS) -o $@

CGraph/cammini.o: CGraph/cammini.c CGraph/utils.h CGraph/xerrori.h
	$(CC) $< $(CFLAGS) -c -o $@

CGraph/utils.o: CGraph/utils.c CGraph/utils.h
	$(CC) $< $(CFLAGS) -c -o $@

CGraph/xerrori.o: CGraph/xerrori.c CGraph/xerrori.h
	$(CC) $< $(CFLAGS) -c -o $@


# Compilazione programma Java
CreaGrafo.class: JGraph/CreaGrafo.java JGraph/Attore.java
	$(JC) -d ./ $^


# Pulizia
cleanC:
	rm CGraph/*.o *.out

cleanJ:
	rm *.class

clean: 
	rm CGraph/*.o *.out *.class
