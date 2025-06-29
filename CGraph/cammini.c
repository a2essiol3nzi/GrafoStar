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
    2. si usa una funziona apposita init_gr() per l'analisi del primo file, verrà aggiornato grafo e restituita la sua lunghezza
  */

  // si deve leggere filenomi
  // si usa la xfopen, esegue la classica funzione fopen (apertura file) ma esegue anche il controllo sul successo dell'operazione (xerrori.*)
  // stampando un messaggio di errore e terminando con exitcode!=0 in caso di insuccesso
  FILE* fn = xfopen(argv[1],"r",QUI);
  // inizializzazione dell'array di attori per la rappresentazione del grafo, e del suo indice di lunghezza. Uso la funzione init_gr() definita appositamente
  // file utils.*
  int grl = 0;
  attore* grafo = init_gr(fn,&grl);
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
  ACCEDERÀ -> i thread potranno accedere contemporaneamente all'array grafo, perche :
  Non c'è concorrenza di scrittura: ogni oggetto è scritto solo da un thread.
  Non c'è concorrenza lettura/scrittura: codice viene solo letto, e mai modificato.
  -> non vi è race condition sui dati in grafo
  */
  // Per la sincronizzazione dei thread sul buffer (circolare) condiviso ho deciso di usare 
  // inizializzo buffer condiviso (circolare), dim buffer (data anche la grandezza del file da leggere) 64
  char* buffer[64];
  // indici di inserimento ed estrazione (inizialmente entrambi a 0)
  int in = 0, out = 0;



  return 0;
}