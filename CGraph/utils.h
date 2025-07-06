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

// struttura dati da passare ai thread che calcolano i cammini minimi tra attori
typedef struct {
  int a;
  int b;    // interi letti dalla pipe
  attore* gr;
  int grl;
} datiminpath;

// struttura dati per i nodi dell'ABR
typedef struct ABRnode {
  int codice;       // codice dell'attore a cui si riferisce
  struct ABRnode* pred;    // predecessore nel cammino dalla sorgente-> per ricostruire il cammino 
  struct ABRnode* sx;
  struct ABRnode* dx;      // figli destro e sinistro 
} ABRnode;

// struttura dati per l'implementazione della Linked-List fifo usata nella BFS
typedef struct FIFOnode {
  int codice;       // codice dell'attore (nodo del grafo) 
  int depth;        // distanza dalla sorgente (lunghezza path) 
  ABRnode* abr;     // puntatore al rispettivo nodo in ABR (per la creazione del ped)
  struct FIFOnode* next;   // puntatore all'elemento successivo della LL
} FIFOnode;








// ----- funzioni d'uso generico
// funzione che inizializza il grafo, inizializza i 3 campi ricavabili dal 1o file (1a parte):
// si legge riga per riga (nel formato a noi noto) e da ognuna ricaviamo i dati utili.
attore* init_gr(FILE* fn, int* len) ;

// funzione per il completamento del grafo tramite filegrafo(2a parte):
// prod scandisce filegrafo
// cons elaborano le linee e completano il GRAFO DELLE STAR 
void complete_gr(int numcons, attore* grafo, int grl, FILE* fg);

// funnzione per gestione della fase di lettura dalla pipe + avvio dei thread per il calcolo dei cammini minimi
void minpath_finder(int fd, volatile sig_atomic_t* term, attore* gr, int grl, pthread_t* thand);





// ----- funnzioni per thread
// funzione dei consumatori per la lettura dal buffer + parsing di una linea + aggiornamento degli attori letti -> da passare alla pthread_create
void* cons_body(void* args);

// funzione del thread produttore (main) per la lettura dal file + inserimento nel buffer -> non verrà passata ad una pthread_create
// si sceglie di definirla ""pthread_create-function like"" per mantenere una regolarità
void* prod_body(void* args);

// funzione del thread gestore dei segnali, stampa il PID del processo a cui appartiene e poi si mette in attesa
// pronto a gestire il segnale SIGINT (^C)
void* handler_body(void* args);

// funzione che implementa la BFS per il calolo dei cammini minimi fra attori (se esistono)
// probabilmente sarebbe stato piu consono implementare l'algo (da "INIZIO ALGORITMO" in poi) in una funzione separata,
// ma trattandosi comunque di una versione dedicata alla singola ricerca dei cammini in cui non sempre si esegue l'intera visita
// ho preferito fare tutto qui, dedicando però una funzione a parte per la stampa e ricostruzione del cammino
void* breadth_first_search(void* args);





// ----- funzioni di deallocazione
// funzione per la deallocazione del grafo delle star
void destruction(attore* gr, int grl, pthread_t* thand);

// funzione che dealloca un ABR facendo la free di ogni nodo (i nodi della FIFO verranno deallocati mano a mano che si estraggono)
void destroy_abr(ABRnode* root);

// funzione per deallocazione FIFO
void destroy_fifo(FIFOnode* head);





// ----- funzioni "minori"
// funzione per il confronto tra due interi
int cmp_intatt(const int *x, const attore *y);

// funzioni (fornite dal professore) per inserimento/ricerca efficiente all'interno degli ABR degli attori visitati
int shuffle(int n);
int unshuffle(int n);

// funzione che dati due valori clock_t calcola i secondi passati tra questi due valori
double elapsed_time(clock_t a, clock_t b);

// funzione che stampa i cammini minimi calcolati dai thread (se esistono)
// ctrl è un valore di controllo che passo io alla funzione che rappresenza l'esito dell'algor
void stampa_minpath(int a, int b, attore* gr, int grl, double eltime, ABRnode* dest, int lpath, int ctrl);






//----- funzioni ABR (la funzione di ricerca non viene usata)
// funzione che crea un nodo ABR
ABRnode* crea_abr(int c, ABRnode* pred);

// funzione di inserimento nodo in ABR
// *root NON VA BENE perche non modificherei davvero i valori di root->sx e root->dx (modifiche solo locali)
int insert_abr(ABRnode **root, ABRnode *node);





// ----- funzioni Linked-List
// funzione di inserimento di un codice di attore in coda bfs
void push(FIFOnode** head, FIFOnode** tail, int codice, int lpath, ABRnode* twin);

// funzione di estrazione di un codice di attore in coda bfs
FIFOnode* pop(FIFOnode** head);





// ----- funzioni di debug
// stampa tutto il grafo creato su stdin
void stampa_gr(attore* gr, int grl);
