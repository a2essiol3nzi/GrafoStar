#include "xerrori.h"

// struttura dei nodi del grafo
typedef struct {
  int codice;         // codice attore
  char *nome;         // nome attore
  int anno;           // anno di nascita
  int numcop;         // numero coprotagonisti == nodi adj
  int *cop;           // array coprotagonisti == array dei nodi adiacenti
} attore;

// funzione che inizializza il grafo, inizializza i 3 campi ricavabili dal 1o file:
// si legge riga per riga (nel formato a noi noto) e da ognuna ricaviamo i dati utili.
attore* init_gr(FILE* f, int* len);

// funzione di comparazione tra codici di attori
int cmp_att(attore* a, attore* b);




// funzione che tokenizza una riga letta da un file
char** tokenize(char* line); 