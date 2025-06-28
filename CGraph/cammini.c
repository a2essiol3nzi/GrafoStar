#include "xerrori.h"  // sono file con funzioni di gestione errori-> funzioni usate durante il corso
#include "utils.h"    // sono funzioni generiche implementate per il progetto-> divise dal main e da xerrori.* per chiarezza e semplicità

// per messaggi di debug
#define QUI __LINE__,__FILE__


int main(int argc, char** argv)
{
  // ===== CONTROLLI PRELIMINARI =====
  // cammini.out viene chiamato con:
  //    cammini.out filenomi filegrafo numconsumatori

  // verifico validità argomenti passati
  if(argc!=4) xtermina("Uso: \tcammini.out filenomi filegrafo numconsumatori",QUI);
  
  int ncons = atoi(argv[3]);
  if(ncons<=0) xtermina("ATT: \tnumconsumatori deve essere strettamente positivo!",QUI);



  // ===== LETTURA NOMI.TXT - CREAZIONE ARRAY DI ATTORI =====
  fprintf(stderr,"===== LETTURA NOMI.TXT - CREAZIONE ARRAY DI ATTORI =====\n");
  /* 
    1. Si deve aprire il file dei nomi e da ogni riga inizializzare un nodo con i campi contenuti: codice nome e anno di nascita
    2. Si legge riga per riga con getline(), e si tokenizza tramite strtok() (spiegato di seguito come con una funzione apposita), 
    e inseriamo ogni attore nell'array grafo allocato dinamicamente (dovrà poi essere riordinato in base ai codici degli attori prima 
    della lettura di filegrafo).
  */

  // si deve leggere filenomi
  // si usa la xfopen, esegue la classica funzione fopen (apertura file) ma esegue anche il controllo sul successo dell'operazione (xerrori.*)
  // stampando un messaggio di errore e terminando con exitcode!=0 in caso di insuccesso
  FILE* fn = xfopen(argv[1],"r",QUI);
  // inizializzazione dell'array di attori per la rappresentazione del grafo, e del suo indice di lunghezza
  // dim iniziale settata a 1024 attori (per un grande numero di attori ~400.000 usa ~20 realloc e permette di non lasciare
  // troppa memoria inutilizzata dopo l'ultima)
  int capacita = 1024;
  attore* grafo = malloc(capacita * sizeof(attore));
  assert(grafo!=NULL);
  int grl = 0;
  // var per lettura file
  int codice, anno;
  char* nome;

  // si fa il parsing della linea in maniera semplice dato che sono di dimensione fissa, 
  // possiamo usare direttamente fscanf per semplificare le cose: %ms è un'estensione GNU (vista a lezione) 
  // per l'allocazione automatica di una stringa
  // iniziamo la lettura del file
  while(1){
    size_t e = fscanf(fn,"%d\t%ms\t%d",&codice,nome,&anno);
    // verifico la lettura
    if(e!=3){
      if(e==EOF) break;
      else xtermina("Errore lettura filenomi",QUI);
    }
    // prima di inserire l'attore verifico se è necessaria una realloc
    if(grl==capacita){
      capacita *= 2;
      grafo = realloc(grafo, capacita*sizeof(attore));
      assert(grafo!=NULL);
    }
    // inizializzo l'attore (codice nome anno)
    grafo[grl] = (attore){
      .codice = codice,
      .nome = strdup(nome), // allocazione dinamica del nome che andrà deallocato
      .anno = anno
    };
    // libero l'allocazione di nome, già copiata
    free(nome);
    // incremento la lunghezza di grafo
    grl++;
  }
  //chiudo file finito di leggere
  if(fclose(fn)==EOF) xtermina("Errore chiusura filenomi",QUI);

  // --- ordinamento di grafo in base ai codici degli attori ---
  fprintf(stderr,"--- Ordinamento grafo ---\n");
  // funzione di ordinamento che usa funz di cmp basata sul codice degli attori (util.*)
  qsort(grafo, grl, sizeof(attore), (__compar_fn_t) &cmp_att);



  // ===== LETTURA GRAFI.TXT - COMPLETAZIONE DEI CAMPI ATTORI =====
  fprintf(stderr,"===== LETTURA GRAFI.TXT - COMPLETAZIONE DEI CAMPI ATTORI =====\n");
  /*
  1. si creano numcons thread che leggono dal buffer, estraggono i vari puntatori a stringhe/linee del filegrafo che viene letto dal main thread
  2. tramite tokenize() (vedere util.*) tokenizzano le righe e ricavano i campi mancanti per gli attori
  3. assegnano i valori trovati ai campi degli attori in grafo per definire completamente IL GRAFO DELLE STAR
  
  Ogni thread deve leggere dal buffer CONDIVISO (serve sincro), ricavare i dati da assegnare ad un certo attore AL QUALE NESSUN ALTRO THREAD 
  ACCEDERÀ -> i thread potranno accedere contemporaneamente all'array grafo, perche nessuno modificherà mai lo stesso attore, e anche nella
  lettura dei codici per la ricerca non si avranno problemi in quanto codice non verrà mai modificato dopo l'inizializzazione!
  */

  // creare buffer del paradigma, thread consumatori, struct da passare ai consuma, funzioni per le tipologie di thread
  // decidere il meotodo di sincronizzazione tra thread per l'accesso al buffer

  // provare a mettere l'inizializzazione di grafo tutta in una funzione "init_grafo"




  return 0;
}