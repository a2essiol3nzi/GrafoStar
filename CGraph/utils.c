#include "utils.h"


// ----- funzioni d'uso generico
// funzione che inizializza il grafo, inizializza i 3 campi ricavabili dal 1o file (1a parte):
// si legge riga per riga (nel formato a noi noto) e da ognuna ricaviamo i dati utili.
attore* init_gr(FILE* fn, int* len) 
{
  assert(fn!=NULL);
  assert(*len==0);
  // allocazione spazio grafo
  // dim iniziale settata a 1024 attori (per un grande numero di attori ~400.000 usa ~20 realloc)
  int capacita = 1024;
  attore* gr = malloc(capacita*sizeof(attore));
  // var per lettura file
  int codice, anno;
  char* nome = NULL;
  // per getline
  char* buffer = NULL;
  size_t n = 0;

  // iniziamo la lettura del file
  while((getline(&buffer,&n,fn))>0){
    // lettura e parsing di una linea
    codice = atoi(strtok(buffer,"\t"));
    nome = strtok(NULL,"\t");
    anno = atoi(strtok(NULL,"\t"));
    // prima di inserire l'attore verifico se è necessaria una realloc
    if(*len==capacita){
      capacita *= 2;
      gr = realloc(gr, capacita*sizeof(attore));
      assert(gr!=NULL);
    }
    // inizializzo l'attore (codice nome anno)
    gr[*len] = (attore){
      .codice = codice,
      .nome = strdup(nome), // allocazione dinamica del nome che andrà deallocato
      .anno = anno
    };
    // incremento la lunghezza di grafo
    (*len)++;
  }
  assert(*len<=capacita);
  // evito spreco di memoria e rialloco il grafo ora che ne conosco il numero di nodi
  gr = realloc(gr,(*len) * sizeof(attore));
  // dealloco il buffer di getline
  free(buffer);
  return gr;
}

// funzione per il completamento del grafo tramite filegrafo(2a parte):
// prod scandisce filegrafo
// cons elaborano le linee e completano il GRAFO DELLE STAR 
void complete_gr(int numcons, attore* grafo, int grl, FILE* fg)
{
  /* Per la sincronizzazione dei thread sul buffer (circolare) condiviso ho deciso di usare due semafori per la gestione del buffer limitato, e una mutex:
  mutex verra usata solo dai thread consumatori per l'accesso all'indice di estrazione (tramite il quale accedere al buffer).
  il thread produttore non avrà bisogno di una mutex dedicata in quanto è l'unico di quella tipologia -> unico che accedere all'indice di inserimento.
  -> in conclusione si aumenta l'efficienza sfruttando la natura del paradigma, gestendo separatamente prod e cons.
  */

  // inizializzo buffer condiviso (circolare), dim buffer (data anche la grandezza del file da leggere) 64 = Buff_size (in utils.h)
  char* buffer[Buff_size];
  // indice di estrazione(usato da TUTTI i thread consumatori che verranno creati) inizialmente a 0 (uguale per quello di inserimento)
  int cindex = 0; int pindex = 0;
  pthread_mutex_t mcons = PTHREAD_MUTEX_INITIALIZER;  // mutex per consumatori
  sem_t s_empty;                                      // semaforo per il conteggio dei posti vuoti nel buffer
  sem_t s_full;                                       // semaforo per il conteggio dei posti pieni nel buffer
  // xsem_init() (xerrori.*) è un'altra funzione che gestisce l'inizializazione di un semaforo (non nominativo) ed esegue i dovuti controlli
  // ne abbiamo anche per le funzoni da usare sui semafori xsem_post/_wait
  xsem_init(&s_empty,0,Buff_size,QUI);
  xsem_init(&s_full,0,0,QUI);
  // inizializzo l'array di thread consumatori, e dei dati da passare ad ognuno e
  // si avviano i consumatori con la relativa funzione XXXX definita in utils.*
    // i tipi dei dati da passare a prd e cons sono entrambe definite in utils.h
  // consumatori
  pthread_t tc[numcons];
  daticons dc[numcons];
  fprintf(stderr,"--- Avvio consumatori ---\n");
  for(int i=0;i<numcons;i++){
    dc[i] = (daticons){
      .buff = buffer,
      .ind = &cindex,
      .mux = &mcons,
      .s_empty = &s_empty,
      .s_full = &s_full,
      .gr = grafo,
      .grl = grl
    };
    // funzione speciale con dovuti controlli (xerrori.*)
    xpthread_create(&tc[i],NULL,cons_body,&dc[i],QUI);
  }  
  // produttore
  datiprod dp = (datiprod){
    .buff = buffer,
    .ind = &pindex,
    .s_empty = &s_empty,
    .s_full = &s_full,
    .file = fg
  };
  fprintf(stderr,"--- Avvio produttore ---\n");
  // avviato come funzione semplice senza creare un nuovo thread
  prod_body(&dp);
  // terminata la funzione del produttore si deve segnalare ai consumatori che non ci sono piui dati da leggere ed eseguire la join
  fprintf(stderr,"  --- Scrittura valori terminazione ---\n");
  for(int i=0;i<numcons;i++){
    xsem_wait(&s_empty,QUI);
    buffer[pindex] = NULL;
    xsem_post(&s_full,QUI);
    pindex = (pindex + 1) % Buff_size;
  }
  for(int i=0;i<numcons;i++)
    xpthread_join(tc[i],NULL,QUI);
  fprintf(stderr,"--- Fine prod-cons ---\n");
  // deallocazione di tutte le risorse di sincronizzazione
  xsem_destroy(&s_empty,QUI); xsem_destroy(&s_full,QUI);
  xpthread_mutex_destroy(&mcons,QUI);
  return ;
}





// ----- funnzioni per thread
// funzione dei consumatori per la lettura dal buffer + parsing di una linea + aggiornamento degli attori letti -> da passare alla pthread_create
void* cons_body(void* args)
{
  assert(args!=NULL);
  daticons* dati = (daticons* )args;
  // iniziamo la procedura di estrazione e modifica attori fino al valore di terminazione
  while(1){
    // fprintf(stderr,"[C] estraggo\n");
    xsem_wait(dati->s_full,QUI);
    // funzione in xerrori.* con controlli sul successo
    xpthread_mutex_lock(dati->mux,QUI);
    // estraggo il puntatore alla linea
    char* tmp = dati->buff[*dati->ind];
    *dati->ind = ((*dati->ind) + 1) % Buff_size;
    if(tmp==NULL){ // controllo valore di interruzione ciclo
      xpthread_mutex_unlock(dati->mux,QUI);
      xsem_post(dati->s_empty,QUI);
      break;
    }
    // procedo con estrazione e elaborazione
    // si necessita di una copia perche senno potrebbe essere sovrascritta durante l'uso da un produttore
    // si copia e si dealloca la vecchia immediatamente
    char* line = strdup(tmp);
    free(tmp);
    xpthread_mutex_unlock(dati->mux,QUI);
    xsem_post(dati->s_empty,QUI);

    // PARSING LINE
    // adesso (senza mutex dato che si deve solo fare calcolo) si elabora la stringa tramite parsing e aggiorniamo il relativo attore
    // per il parsing non si usa una funzione dedicata ma si definisce una procedura direttamente nella funzione corrente che usa strtok_r(),
    // (strtok non è MT safe, serve percio la versione rientrante per garantire il funzinamento-> saveptr gestito loalmente invece che globalmente)
    // ricaviamo subito codice attore e numcops dai primi due campi della linea, così da allocare direttamente lo spazio strettamente necessario
    char* saveptr;
    int att_cod = atoi(strtok_r(line,"\t",&saveptr));
    int numcop = atoi(strtok_r(NULL,"\t",&saveptr));
    int* cop = NULL;
    // verifico che l'attore abbia collaboratori
    if(numcop>0){
      cop = malloc(numcop * sizeof(int));
      assert(cop!=NULL);
      for(int i=0;i<numcop;i++)
        cop[i] = atoi(strtok_r(NULL,"\t",&saveptr));
    }
    // dealloco la stringa che non serve più
    free(line);
    // AGGIORNO ATTORE prima però devo cercarlo in grafo (dati->gr) tramite bsearch(3): 
    // bsearch cerca un certo elemento in un array gia ordinato e in caso di match tra elementi viene restituito un puntatore all'elem.
    attore* a = bsearch(&att_cod,dati->gr,dati->grl,sizeof(attore),(__compar_fn_t) &(cmp_int));
    assert(a!=NULL);
    a->numcop = numcop;
    a->cop = cop;
  }
  // termino il thread
  pthread_exit(NULL);
}

// funzione del thread produttore (main) per la lettura dal file + inserimento nel buffer -> non verrà passata ad una pthread_reate
// si sceglie di definirla ""pthread_create-function like"" per mantenere una regolarità
void prod_body(void* args)
{
  assert(args!=NULL);
  datiprod* dati = (datiprod* )args;
  // creo variabili per l'esecuzione della getline()
  char* line = NULL; // buffer esato e allocato automaticamente dalla getline in caso di necessità
  size_t n = 0; // intero rappresentante la grandezza in B del buffer allocato (confrotato con la riga letta per decidere se si ha bisogno di piu spazio)

  // avviamo la lettura del file e inserimento nel buffer
  while((getline(&line,&n,dati->file))>0){
    // fprintf(stderr,"[P] inserisco\n");
    // dobbiamo duplicare la linea letta e inserire il puntatore nel buffer, 
    // non possiamo direttamente isnerire in buffer line perhce verra sovrascritto il contenuto e persa la linea letta
    // inserimento nel buffer e incremento dell'indice circolare
    assert(line!=NULL);
    xsem_wait(dati->s_empty,QUI);
    dati->buff[*dati->ind] = strdup(line);
    *dati->ind = ((*dati->ind) + 1) % Buff_size;
    xsem_post(dati->s_full,QUI);
  }
  // liberiamo il buffer di linea
  free(line);
  // possiamo terminare

}

// funzione del thread gestore dei segnali, stampa il PID del processo a cui appartiene e poi si mette in attesa
// pronto a gestire il segnale SIGINT (^C)
void* handler_body(void* args)
{
  char buffer[25];
  int len = sprintf(buffer,"Il mio PID: %d\n",getpid());
  // stampo il pid
  write(1,buffer,len);
  datisighand* d = (datisighand* )args;
  // set di segnali da gestire
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask,SIGINT);
  int e, s;
  // messaggi di stampa
  char messC[46] = "< Costruzione GRAFO DELLE STAR in corso... >\n";
  while(1){
    // attendo SIGINT
    e = sigwait(&mask,&s);
    if(e!=0) xtermina("Errore sigwait",QUI);
    if(*d->pipe){
      // se ce la pipe il thread gestore deve settare la var termina (per far terminare il main se non lo sta gia facendo di suo)
      // e poi terminare a sua volta per essere joinato
      *d->term = 1;
      break;
    } else {
      // se non ce la pipe si attende
      write(1,messC,46);
    }
  }
  return NULL;
}





// ----- funzioni di deallocazione
// funzione per la terminazione del programma e deallocazione delle risorse
void destruction(attore* gr, int grl, pthread_t* thand)
{
  // joino il therad gestore
  xpthread_join(*thand,NULL,QUI);
  // aspetto 20 secs
  sleep(20);
  for(int i=0;i<grl;i++){
    free(gr[i].nome);
    free(gr[i].cop);
  }
  free(gr);
}





// ----- funzioni di confornto 
// funzione per il confronto tra due interi
int cmp_int(const void *a, const void *b)
{
  int x = *(int *)a;
  int y = *(int *)b;
  return (x > y) - (x < y);  // ritorna 1, 0 o -1
}





// ----- funzioni di debug
