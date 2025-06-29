#include "utils.h"

// funzione che inizializza il grafo, inizializza i 3 campi ricavabili dal 1o file:
// si legge riga per riga (nel formato a noi noto) e da ognuna ricaviamo i dati utili.
attore* init_gr(FILE* f, int* len) 
{
  assert(f!=NULL);
  assert(*len==0);
  // allocazione spazio grafo
  // dim iniziale settata a 1024 attori (per un grande numero di attori ~400.000 usa ~20 realloc)
  int capacita = 1024;
  attore* grafo = malloc(capacita*sizeof(attore));
  // var per lettura file
  int codice, anno;
  char* nome;

  // si fa il parsing della linea in maniera semplice dato che sono di dimensione fissa, 
  // possiamo usare direttamente fscanf per semplificare le cose: %ms è un'estensione GNU (vista a lezione) 
  // per l'allocazione automatica di una stringa

  // iniziamo la lettura del file
  while(1){
    size_t e = fscanf(f,"%d\t%ms\t%d",&codice,nome,&anno);
    // verifico la lettura
    if(e!=3){
      if(e==EOF) break;
      else xtermina("Errore lettura filenomi",__LINE__,__FILE__);
    }
    // prima di inserire l'attore verifico se è necessaria una realloc
    if(*len==capacita){
      capacita *= 2;
      grafo = realloc(grafo, capacita*sizeof(attore));
      assert(grafo!=NULL);
    }
    // inizializzo l'attore (codice nome anno)
    grafo[*len] = (attore){
      .codice = codice,
      .nome = strdup(nome), // allocazione dinamica del nome che andrà deallocato
      .anno = anno
    };
    // libero l'allocazione di nome, già copiata
    free(nome);
    // incremento la lunghezza di grafo
    (*len)++;
  }
  assert(*len<=capacita);
  // evito spreco di memoria e rialloco il grafo ora che ne conosco il numero di nodi
  grafo = realloc(grafo,(*len)*sizeof(attore));

  return grafo;
}

// funzione di comparazione tra codici di attori
int cmp_att(attore* a, attore* b)
{
  assert(a!=NULL && b!=NULL);
  return a->codice - b->codice;
}




// funzione che tokenizza una riga letta da un file
char** tokenize(char* line)
{
  assert(line!=NULL);
}