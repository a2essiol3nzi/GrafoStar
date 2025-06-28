#include "xerrori.h"

// struttura dei nodi del grafo
typedef struct {
  int codice;         // codice attore
  char *nome;         // nome attore
  int anno;           // anno di nascita
  int numcop;         // numero coprotagonisti == nodi adj
  int *cop;           // array coprotagonisti == array dei nodi adiacenti
} attore;

// funzione che tokenizza una riga letta da un file
char** tokenize(char* line); 

// funzione di comparazione tra codici di attori
int cmp_att(attore* a, attore* b);