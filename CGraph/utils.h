#include "xerrori.h"
#define Buff_size 64
#define QUI __LINE__,__FILE__


// ----- struct definite per il programma
// struttura dati dei nodi del grafo
typedef struct {
  int codice;         // codice attore
  char *nome;         // nome attore
  int anno;           // anno di nascita
  int numcop;         // numero coprotagonisti == numero nodi adj
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
  int* ind;           // indice di isnerimento nel buffer
  sem_t* s_full;      // semaforo per le locazioni piene nel buffer
  sem_t* s_empty;     // semaforo per le locazioni vuote nel buffer
  FILE* file;         // file da cui leggere
} datiprod;

// struttura dati da passare al thread gestore dei segnali
typedef struct {
  volatile sig_atomic_t* pipe;  // stato della cammini.pipe
  volatile sig_atomic_t* term;  // variabile di terminazione (post-pipe)
} datisighand;

// struttura dati da passare ai thread che calcolano i cammini minimi tra attori
typedef struct {
  int a;
  int b;        // interi letti dalla pipe
  attore* gr;   // puntatore al grafo
  int grl;      // lunghezza dell'array grafo
} datiminpath;

// struttura dati per i nodi dell'ABR
typedef struct ABRnode {
  attore* att;              // puntatore all'attore associato
  int shufc;                // codice attore shuffled
  struct ABRnode* pred;     // predecessore nel cammino dalla sorgente-> per ricostruire il cammino 
  struct ABRnode* sx;
  struct ABRnode* dx;       // figli destro e sinistro 
} ABRnode;

// struttura dati per la gestione della coda fifo della bfs
typedef struct {
  attore** queue;   // coda fifo di attore*
  int cap;          // capacità dell' array per eseguire le realloc
  int head;         // indice di estrazione
  int tail;         // indice di inserimento
} FIFO;








// ----- funzioni d'uso generico
attore* init_gr(FILE* fn, int* len) ;

// funzione per il completamento del grafo tramite filegrafo(2a parte): 
void complete_gr(int numcons, attore* grafo, int grl, FILE* fg);

// funzione per gestione della fase di lettura dalla pipe + avvio dei thread per il calcolo dei cammini minimi
void minpath_finder(int fd, volatile sig_atomic_t* term, attore* gr, int grl, pthread_t* thand);





// ----- funnzioni per thread
void* cons_body(void* args);

void* prod_body(void* args);

void* handler_body(void* args);

void* breadth_first_search(void* args);





// ----- funzioni di deallocazione
void destruction(attore* gr, int grl, pthread_t* thand);

void destroy_abr(ABRnode* root);

void destroy_fifo(FIFO* q);





// ----- funzioni "minori"
int cmp_intatt(const void *x, const void *y);

int shuffle(int n);
int unshuffle(int n);

double elapsed_time(clock_t a, clock_t b);

void stampa_minpath(int a, int b, double eltime, ABRnode* dest, int ctrl);






//----- funzioni ABR (la funzione di ricerca non viene usata)
// funzione che crea un nodo ABR
ABRnode* crea_abr(attore* a, ABRnode* pred);

// funzione di inserimento nodo in ABR (basata su shuffle)
// *root NON VA BENE perche non modificherei davvero i valori di root->sx e root->dx (modifiche solo locali)
void insert_abr(ABRnode **root, ABRnode *node);

// funzione di ricerca in ABR (basata su shuffle)
ABRnode* search_abr(ABRnode* root, int sc);




// ----- funzioni Linked-List
// funzione di inserimento di un codice di attore in coda bfs
void push(FIFO* q, attore* a);

// funzione di estrazione di un codice, profondita e nodo abr da fifo
attore* pop(FIFO* q);





// ----- funzioni di debug
// stampa tutto il grafo creato su stdin
void stampa_gr(attore* gr, int grl);
