#include "xerrori.h"  // sono file con funzioni di gestione errori-> funzioni usate durante il corso
#include "utils.h"    // sono funzioni generiche implementate per il progetto-> divise dal main e da xerrori.* per chiarezza e semplicità



int main(int argc, char** argv)
{
  // ===== CONTROLLI PRELIMINARI =====
  // cammini.out viene chiamato con:
  //    cammini.out filenomi filegrafo numconsumatori

  // verifico validità argomenti passati
  if(argc!=4) xtermina("Uso: \tcammini.out filenomi filegrafo numconsumatori",QUI);
  
  int ncons = atoi(argv[3]);
  if(ncons<=0) xtermina("ATT: \tnumconsumatori deve essere strettamente positivo!",QUI);



  // (===== AVVIO THREAD GESTORE SEGNALI =====)
  fprintf(stderr,"(===== AVVIO THREAD GESTORE SEGNALI =====)\n");
  /*
    1. definiamo una funzione di handling (handler_body) che verrà eseguita dal thread gestore dei segnali
    2. definiamo nel main (e la passeremo tra gli argomenti del thread) una variabile atomica per la rappresentazione dello stato
    (pre-pipe e post-pipe), e una var rappresentante la ricezione del segnale SIGINT che verrà ascoltata dal main 
    durante la lettura dalla pipe.

    NOTA: in caso di temrinazione "naturale" del programma, dobbiamo notificare il thread gestore di terminare per essere joinato
    perciò in  quel caso il mian invierà un SIGINT al thread gestore dopo aver settato termina gia ad 1
  */
  // variabile atomica di stato, necessita di essere atomica (essere acceduta con un metodo) perche prima o poi verra modificata
  // dobbiamo quindi evitare race-cond di lettura su scrittura
  volatile sig_atomic_t pipe_state = 0;
  volatile sig_atomic_t term = 0;
  // inizializzazione grafo e passagio riferimenti al thread gestore per la deallocazione finale
  attore* grafo = NULL;
  int grl = 0;
  // blocco SIGINT cosi da poterlo gestire come specificato nel testo
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask,SIGINT);
  // maschero questi segnali
  pthread_sigmask(SIG_BLOCK,&mask,NULL);
  // creo il thread e lo avvio (dovrà essere joinato dal main al momento della deallocazione/terminazione)
  pthread_t thand;
  datisighand dth = { .pipe = &pipe_state, .term = &term };
  xpthread_create(&thand,NULL,handler_body,&dth,QUI);

  // test gestione
  // kill(getpid(),SIGINT);



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
  grafo = init_gr(fn,&grl);
  assert(grafo!=NULL);
  //chiudo file finito di leggere
  if(fclose(fn)==EOF) xtermina("Errore chiusura filenomi",QUI);

  // test di gestione
  // kill(getpid(),SIGINT);



  // ===== LETTURA GRAFI.TXT - COMPLETAZIONE DEI CAMPI ATTORI =====
  fprintf(stderr,"===== LETTURA GRAFI.TXT - COMPLETAZIONE DEI CAMPI ATTORI =====\n");
  /* 
    1. si creano numcons thread che leggono dal buffer, estraggono i vari puntatori a stringhe/linee del filegrafo che viene letto dal main thread
    2. viene fatto il parsing delle righe leggendo inizialmente i primi due valori (sempre presenti), e poi tokenizzando i restanti n (specificati nel 2o campo)
    3. assegno i valori trovati ai campi degli attori in grafo per definire completamente IL GRAFO DELLE STAR

    Ogni thread deve leggere dal buffer CONDIVISO (serve sincro), ricavare i dati da assegnare ad un certo attore AL QUALE NESSUN ALTRO THREAD 
    ACCEDERÀ -> i thread potranno accedere contemporaneamente all'array grafo, perche :
    Non c'è concorrenza di scrittura: ogni oggetto è scritto solo da un thread.
    Non c'è concorrenza lettura/scrittura: codice viene solo letto, e mai modificato.
    -> non vi è race condition sui dati in grafo
  */
  // apro il 2o file
  FILE* fg = xfopen(argv[2],"r",QUI);
  // avvio il paradigma prod-cons
  complete_gr(atoi(argv[3]),grafo,grl,fg);
  // dopodiché posso richiudere il file aperto
  if(fclose(fg)==EOF) xtermina("Errore chiusura filegrafo\n",QUI);



  // ===== CREAZIONE CAMMINI.PIPE - LETTURA INT32 BIT =====
  fprintf(stderr,"===== CREAZIONE CAMMINI.PIPE - LETTURA COPPIE INT32 =====\n");
  /*
    1. si deve creare la pipe e cambiare il valore di pipe_state (così che il thread gestore segnali modifichi l'approccio)
    2. tramite funzione dedicata (utils.*) si avvia un ciclo di lettura dalla pipe nel quale si controlla anche la variabile term per vedere se il thread gestore ha registrato
    l'arrivo di un SIGINT-> in tal caso si deve interrompere la lettura, chiudere la pipe e terminare come decritto nel testo (e joinando il thread gestore)
  */
  // creazione pipe specificando nome e permessi in ottale. 
  int e = mkfifo("./cammini.pipe",0666);
  if(e==0) // se non ci sono errori oppure gia esisteva la pipe 
    fprintf(stderr,"--- Pipe creata con successo ---\n");  
  else if(errno==EEXIST) 
    fprintf(stderr,"--- Pipe già esistente ---\n");
  else 
    xtermina("Errore creazione pipe",QUI);
  // apriamo la pipe il lettura
  int fd = open("./cammini.pipe",O_RDONLY);
  if(fd<0) xtermina("Errore apertura pipe (lettura)",QUI);
  // aggiorno lo stato della pipe, da ora in poi il programma può terminare
  pipe_state = 1;
  // inizio il ciclo di lettura di coppie di int32 (utils.*), è importante che la funzione abbia anche gli argomenti necessari per 
  // chiamare la funzione di terminazione destruction (utils.*)
  minpath_finder(fd,&term,grafo,grl,&thand);
  fprintf(stderr,"--- Chiusura lettura dalla pipe ---\n");
  // chiusura e distruzione pipe
  xclose(fd,QUI);
  unlink("./cammini.pipe");



  // arrivati a questo punto vuol dire che i valori da leggere sono terminati, e la pipe è stata chiusa e distrutta (non piu presente in FS)
  fprintf(stderr,"## Terminazione naturale del programma, attendere prego... ##\n");
  // in caso di terminazione naturale dobbiamo dire al thread gestore di terminare anche lui, gli inviamo di proposito un SIGINT 
  // per avvisarlo di terminare dato che il programma è giunto alla fine
  pthread_kill(thand,SIGINT);
  destruction(grafo,grl,&thand);
  return 0;
}