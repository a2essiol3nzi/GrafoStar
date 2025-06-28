#include "utils.h"

// funzione che tokenizza una riga letta da un file
char** tokenize(char* line)
{
  assert(line!=NULL);
}

// funzione di comparazione tra codici di attori
int cmp_att(attore* a, attore* b)
{
  assert(a!=NULL && b!=NULL);
  return a->codice - b->codice;
}