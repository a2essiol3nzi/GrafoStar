#include "xerrori.h"
#define Buff_size 64
#define QUI __LINE__,__FILE__


// ----- struct definite per il programma
// struttura dei nodi del grafo
typedef struct {
  int codice;         // codice attore
  char *nome;         // nome attore
  int anno;           // anno di nascita
  int numcop;         // numero coprotagonisti == nodi adj
  int *cop;           // array coprotagonisti == array dei nodi adiacenti
} attore;

// struttura dei dati da passare ai consumatori per la lettura dal buffer e accesso al grafo
typedef struct {
  char** buff;            // buffer condiviso
  int* ind;               // indice di estrazione
  pthread_mutex_t* mux;   // mutex dei consumatori
  sem_t* s_full;          // semaforo per le locazioni piene nel buffer
  sem_t* s_empty;         // semaforo per le locazioni vuote nel buffer
  attore* gr;             // GRAFO DELLE STAR
  int grl;                // lunghezza gr
} daticons;

// struttura dati da passare al produttore per la lettura del file e inserimento nel buffer
typedef struct {
  char** buff;        // buffer
  int* ind;            // indice di isnerimento nel buffer
  sem_t* s_full;      // semaforo per le locazioni piene nel buffer
  sem_t* s_empty;     // semaforo per le locazioni vuote nel buffer
  FILE* file;        // file da cui leggere
} datiprod;

// struttura dati da passare al thread gestore dei segnali
typedef struct {
  volatile sig_atomic_t* pipe;  // stato della cammini.pipe
  volatile sig_atomic_t* term;  // variabile di terminazione (post-pipe)
} datisighand;





// ----- funzioni d'uso generico
// funzione che inizializza il grafo, inizializza i 3 campi ricavabili dal 1o file:
// si legge riga per riga (nel formato a noi noto) e da ognuna ricaviamo i dati utili.
attore* init_gr(FILE* fn, int* len);

// funzione per il completamento del grafo tramite filegrafo(2a parte):
// prod scandisce filegrafo
// cons elaborano le linee e completano il GRAFO DELLE STAR 
void complete_gr(int numcons, attore* grafo, int grl, FILE* fg);





// ----- funnzioni per thread
// funzione dei consumatori per la lettura dal buffer + parsing di una linea + aggiornamento degli attori letti -> da passare alla pthread_create
void* cons_body(void* args);

// funzione del thread produttore (main) per la lettura dal file + inserimento nel buffer -> non verrà passata ad una pthread_create
// si sceglie di definirla ""pthread_create-function like"" per mantenere una regolarità
void prod_body(void* args);

// funzione del thread gestore dei segnali, stampa il PID del processo a cui appartiene e poi si mette in attesa
// pronto a gestire il segnale SIGINT (^C)
void* handler_body(void* args);




// ----- funzioni di deallocazione
// funzione per la deallocazione del grafo delle star
void destruction(attore* gr, int grl, pthread_t* thand);





// ----- funzioni di confornto 
// funzione per il confronto tra due interi
int cmp_int(const void *a, const void *b); 




// ----- funzioni di debug
