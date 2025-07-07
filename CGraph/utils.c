#include "utils.h"


// ----- funzioni d'uso generico
// funzione che inizializza il grafo, inizializza i 3 campi ricavabili dal 1o file (1a parte):
// si legge riga per riga (nel formato a noi noto) e da ognuna ricaviamo i dati utili.
attore* init_gr(FILE* fn, int* len) 
{
  assert(fn!=NULL);
  assert(*len==0);
  // allocazione spazio grafo
  // dim iniziale settata a 1024 attori
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
  (l'uso dei semafori ci assicura che gli indici p e c non si sovrapporranno mai creando una race condition)
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
  fprintf(stderr,"-- Scrittura valori terminazione --\n");
  for(int i=0;i<numcons;i++){
    xsem_wait(&s_empty,QUI);
    buffer[pindex] = NULL;
    xsem_post(&s_full,QUI);
    pindex = (pindex + 1) % Buff_size;
  }
  for(int i=0;i<numcons;i++)
    xpthread_join(tc[i],NULL,QUI);
  fprintf(stderr,"-- Fine prod-cons --\n");
  // deallocazione di tutte le risorse di sincronizzazione
  xsem_destroy(&s_empty,QUI); xsem_destroy(&s_full,QUI);
  xpthread_mutex_destroy(&mcons,QUI);
  return ;
}

// funnzione per gestione della fase di lettura dalla pipe + avvio dei thread per il calcolo dei cammini minimi
void minpath_finder(int fd, volatile sig_atomic_t* term, attore* gr, int grl, pthread_t* thand)
{
  // avviamo un ciclo di lettura dalla pipe in cui per ogni coppia di interi a 32 bit che leggiamo, associamo un thread
  // che considera i due valori come codici di due attori a b, e calcola il cammino minimo da a a b.
  // la condizione del while verifica l'arrivo di un segnale di sigint che (in questa fase del programma) deve terminare l'esecuzione 
  while(!(*term)){
    // leggiamo dalla pipe due interi, il primo si legge e controlliamo che non sia stata chiusa la pipe, il secondo abbiamo la sicurezza 
    // di trovarlo (per come vengono passati gli interi dalla pipe)
    // si usa malloc perhce altrimenti al di fuori del blocco la struct viene deallocata!
    datiminpath* dth = malloc(sizeof(datiminpath));
    int32_t a,b;
    int n = read(fd,&a,sizeof(int32_t));
    if(n==0){ // scrittore ha chiuso la sua estremita
      // dealloco ed esco
      free(dth);
      break;
    }
    if(n<4) // errore di passaggio valori o errore di lettura
      xtermina("Errore lettura da pipe",QUI);
    n = read(fd,&b,4);
    if(n<4) // errore di passaggio valori o errore di lettura
      xtermina("Errore lettura da pipe",QUI);
    // finiamo di riempire la struct da passare al thread che dovrà calcolare il cammino minimo tra i due interi letti
    *dth = (datiminpath){
      .a = a,
      .b = b,
      .gr = gr,
      .grl = grl
    };
    // creazione e avvio del thread dedicato al calcolo, QUESTO (come tutti gli altri thread di questo tipo dovranno essere resi detach)
    pthread_t tminp;
    fprintf(stderr,"@ Avvio calcolo cammino tra %d %d\n",dth->a,dth->b);
    xpthread_create(&tminp,NULL,breadth_first_search,dth,QUI);
    // versione x (creata da me) nei file xerrori.*, come le altre verifica il successo dell'operazione
    // quando avviati restituiranno le risorse autonomamente senza il bisongo della join
    xpthread_detach(tminp,QUI);
  }
  // verifichiamo se il thread gestore ci avvisa del segnale di SIGINT
  if(*term){
    fprintf(stderr,"## Terminazione INnaturale, attendere prego... ##\n");
    // thread gestore sarà gia terminato, chiudo la pipe
    xclose(fd,QUI);
    fprintf(stderr,"--- Chiusura lettura dalla pipe ---\n");
    // devo distruggere la pipe
    unlink("./cammini.pipe");
    // dealloco
    destruction(gr,grl,thand);
    // esco con exitcode di fallimento
    exit(EXIT_FAILURE);
  }
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
    // procedo con copia e elaborazione
    // si necessita di una copia perche senno potrebbe essere sovrascritta (e persa) durante l'uso da un produttore (in quanto verra incrementto il sem_empty)
    // si copia e si dealloca la vecchia immediatamente
    char* line = strdup(tmp);
    free(tmp);
    xpthread_mutex_unlock(dati->mux,QUI);
    xsem_post(dati->s_empty,QUI);

    // PARSING LINE
    // adesso (senza mutex dato che si deve solo fare "calcolo") si elabora la stringa tramite parsing e aggiorniamo il relativo attore
    // per il parsing non si usa una funzione dedicata ma si definisce una procedura direttamente nella funzione corrente che usa strtok_r(),
    // (strtok NON è MT safe, serve percio la versione rientrante per garantire il funzinamento-> saveptr gestito localmente invece che globalmente)
    // ricaviamo subito codice attore e numcops dai primi due campi della linea, così da allocare direttamente lo spazio strettamente necessario
    char* saveptr;
    int att_cod = atoi(strtok_r(line,"\t",&saveptr));
    int numcop = atoi(strtok_r(NULL,"\t",&saveptr));
    int* cop = NULL;
    // verifico che l'attore abbia collaboratori, e in caso alloco dello spazio per i coll
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
    // come funzione di confronto usiamo una funz che confronta l'int (attcode) con il codice di un attore nel grafo, un'alternativa era 
    // di definire una funzione che confronta due attori e creare un istanza temopranea con att_code.
    attore* a = bsearch(&att_cod,dati->gr,dati->grl,sizeof(attore),(__compar_fn_t) &(cmp_intatt));
    assert(a!=NULL);
    a->numcop = numcop;
    a->cop = cop;
  }
  // termino il thread
  pthread_exit(NULL);
}

// funzione del thread produttore (main) per la lettura dal file + inserimento nel buffer -> non verrà passata ad una pthread_create
// ho scelto di definirla ""pthread_create-function like"" per mantenere una regolarità
void* prod_body(void* args)
{
  assert(args!=NULL);
  datiprod* dati = (datiprod* )args;
  // creo variabili per l'esecuzione della getline()
  char* line = NULL; // buffer usato e gestito automaticamente dalla getline in caso di necessità
  size_t n = 0; // intero rappresentante la grandezza in Byte del buffer allocato (confrotato con la lunghezza della riga letta per decidere se si ha bisogno di piu spazio)

  // avviamo la lettura del file e inserimento nel buffer
  while((getline(&line,&n,dati->file))>0){
    // dobbiamo duplicare la linea letta e inserire il puntatore nel buffer,
    // non possiamo direttamente inserirla nel buffer perhce verra sovrascritto il contenuto di line alla prossima getline, e perso il cotenuto
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
  return NULL;
}

// funzione del thread gestore dei segnali, stampa il PID del processo a cui appartiene e poi si mette in attesa
// pronto a gestire il segnale SIGINT (^C)
void* handler_body(void* args)
{
  assert(args!=NULL);
  char buffer[25];
  // Costruzione stringhe da stampare con write
  // snprintf è la versione piu "sicura" di sprintf, tiene di conto della lunghezza del buffer evitando overflow in caso 
  // la costruzione della stringa vada oltre i limiti (non accede a zone di memoria non adibite)
  int len = snprintf(buffer,25,"Il mio PID: %d\n",getpid());
  char messC[57] = "Costruzione del GRAFO DELLE STAR in corso, attendere...\n";
  // stampo il pid
  int w = write(1,buffer,len);
  if(w<0) xtermina("Errore write (syscall)",QUI);
  datisighand* dati = (datisighand* )args;
  // set di segnali da gestire
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask,SIGINT);
  int e, s;
  // avvio il ciclo di ricezione
  while(1){
    // attendo SIGINT
    e = sigwait(&mask,&s);
    if(e!=0) xtermina("Errore sigwait",QUI);
    // ricevuto SIGINT correttamente
    if(*dati->pipe){
      // se ce la pipe il thread gestore deve settare la var termina (per far terminare il main se non lo sta gia facendo "naturalmente")
      // e poi terminare a sua volta per essere joinato
      *dati->term = 1;
      break;
    } else {
      // se non ce la pipe si avvisa che si sta ancora costruendo il grafo
      w = write(1,messC,sizeof(messC));
      if(w<0) xtermina("Errore write (syscall)",QUI);
    }
  }
  return NULL;
}

// funzione che implementa la BFS per il calolo dei cammini minimi fra attori (se esistono)
// probabilmente sarebbe stato piu consono implementare l'algo (da "INIZIO ALGORITMO" in poi) in una funzione separata,
// ma trattandosi comunque di una versione dedicata alla singola ricerca dei cammini in cui non sempre si esegue l'intera visita
// ho preferito fare tutto qui, dedicando però una funzione a parte per la stampa e ricostruzione del cammino
void* breadth_first_search(void* args)
{
  datiminpath* dati = (datiminpath* )args;
  // Preparazione all'esecuzione dell'algoritmo
  // inizio la misurazione del tempo (#tick del ciclo di clock) per misurare la durata della funzione
  clock_t start = times(NULL);
  // verifico che la sorgente sia valida
  attore* s = bsearch(&dati->a,dati->gr,dati->grl,sizeof(attore),(__compar_fn_t) &(cmp_intatt));
  if(s==NULL){
    // dobbiamo rileggere times per vedere quanti cicli sono passati, scrivere nel file e su stdout il risultato (negativo) della computazione e poi terminare
    clock_t end = times(NULL);
    double eltime = elapsed_time(start,end);
    // funzione che esegue le varie stampe in base all'esito, rappresentato dall'ultimo arg (ctrl)
    stampa_minpath(dati->a,dati->b,NULL,0,eltime,NULL,-1); 
    // nessun ABR o FIFO da deallocare
    // termino
    free(dati);
    return NULL;
  }
  // INIZIO ALGORITMO (con aggiustamenti per stop prima del termine di tutta la visita in caso si trovi il nodo interessato)
  // creo le strutture necessarei all'algoritmo: ABR dei nodi visitati del grafo, un array circolare fifo per i raggiunti ma non ancora analizzati
  // in particolare si definisce una struct apposita per l'intera gestione
  // aggiungo la sorgente all'ABR
  ABRnode* visitati = crea_abr(dati->a,0,NULL);
  assert(visitati!=NULL);
  // creo la fifo
  FIFO raggiunti = {
    .cap = 128,
    .head = 0,
    .tail = 0,
    .size = 0
  };
  raggiunti.queue = malloc(raggiunti.cap * sizeof(int));
  assert(raggiunti.queue!=NULL);
  // inserisco la sorgente, avvio l'algoritmo e continuero fino a che la FIFO non è vuota
  push(&raggiunti,dati->a);
  while(raggiunti.size!=0){
    // estraggo dalla testa
    int est = pop(&raggiunti);
    ABRnode* est_abr = search_abr(visitati,est);
    // un nodo associato ad est_c sarà gia presente in ABR (visitati)
    // a questo punto INTANTO SI VERIFICA SE (il nodo appena estratto) È LA NOSTRA DESTINAZIONE (dati->b) in tal caso abbiamo finito,
    // ci interessa solo lui non tutta la BFS
    if(est == dati->b){
      // caloclo il tempo e faccio le dovute stampe
      clock_t end = times(NULL);
      double eltime = elapsed_time(start,end);
      // stampa con esito positivo e dealloco tutto
      stampa_minpath(dati->a,dati->b,dati->gr,dati->grl,eltime,est_abr,1);
      // da deallocare sta volta abbiamo l'ABR, eventuali nodi rimasti in FIFO, il nodo di FIFO appena estratto, e come prima i dati
      destroy_abr(visitati); 
      destroy_fifo(&raggiunti);
      free(dati);
      // termino
      return NULL;
    } else {
      // (ALTRIMENTI) dobbiamo aggiungere alla FIFO tutti i suoi adiacenti nel grafo (tranne quelli gia presenti in visitati)
      // -> gia inseriti per adiacenza con un altro nodo (a gestire l'inserimento corretto ci penserà la funzione insert_abr)
      // ricaviamo l'attore per definire gli adiacenti
      attore* est_att = bsearch(&est,dati->gr,dati->grl,sizeof(attore),(__compar_fn_t) &(cmp_intatt));
      assert(est_att!=NULL);
      int* adj = est_att->cop;
      for(int i=0;i<est_att->numcop;i++){
        // creo un nodo ABR di un adiacente e provo ad inserirlo
        ABRnode* adj_node = crea_abr(adj[i],est_abr->depth+1,est_abr);
        if(insert_abr(&visitati,adj_node))  // se è stato aggiunto è nuovo -> devo aggiungerlo anche in FIFO
          push(&raggiunti,adj[i]);
      }
    }
  }
  assert(raggiunti.size==0);
  clock_t end = times(NULL);
  // se siamo qui abbiamo eseguito l'intera BFS, MA non abbiamo trovato un percorso da a a b (altrimenti ci saremmo fermati)
  // dobbiamo terminare stampando un esito negativo 
  double eltime = elapsed_time(start,end);
  stampa_minpath(dati->a,dati->b,NULL,0,eltime,NULL,0);
  // dealloco ABR e FIFO
  destroy_fifo(&raggiunti);
  destroy_abr(visitati);
  free(dati);
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

// funzione che dealloca un ABR facendo la free di ogni nodo (i nodi della FIFO verranno deallocati mano a mano che si estraggono)
void destroy_abr(ABRnode* root)
{
  if(root!=NULL){
    destroy_abr(root->sx);
    destroy_abr(root->dx); // prima i figli senno si perde il riferimento!
    free(root);
  }
}

// funzione per deallocazione FIFO
void destroy_fifo(FIFO* q)
{
  // dealloco malloc interna alla fifo
  free(q->queue);
}






// ----- funzioni "minori"
// funzione per il confronto tra due interi
int cmp_intatt(const int *x, const attore *y)
{
  return (*x > y->codice) - (*x < y->codice);  // ritorna 1, 0 o -1
}

// funzioni (fornite dal professore) per inserimento/ricerca efficiente all'interno degli ABR degli attori visitati
int shuffle(int n) 
{
  return ((((n & 0x3F) << 26) | ((n >> 6) & 0x3FFFFFF)) ^ 0x55555555);
}
int unshuffle(int n) 
{
  return ((((n >> 26) & 0x3F) | ((n & 0x3FFFFFF) << 6)) ^ 0x55555555);
}

// funzione che dati due valori clock_t calcola i secondi passati tra questi due valori
double elapsed_time(clock_t a, clock_t b)
{
  return (double)(b - a) / sysconf(_SC_CLK_TCK); // == (cicli di clock trascorsi)/(tick del clock in un secondo)
}

// funzione che stampa i cammini minimi calcolati dai thread (se esistono)
// ctrl è un valore di controllo che passo io alla funzione che rappresenza l'esito dell'algor
void stampa_minpath(int a, int b, attore* gr, int grl, double eltime, ABRnode* dest, int ctrl)
{
  // creo nome file
  char buff[64]; // piu che sufficiente
  snprintf(buff,sizeof(buff),"%d.%d",a,b);
  // devo stampare sul file dedicato e su stdout
  FILE* f = xfopen(buff,"w",QUI);
  if(ctrl==1){  // esito positivo
    // per la stampa mi appoggio ad un array di costruzione per il cammino in cui inserisco i vari codici (al rovescio== dalla dest) e poi li rileggo 
    // dalla sorgente
    int* cammino = malloc((dest->depth+1) * sizeof(int));
    int len = dest->depth;
    for(int i=len;i>=0;i--){
      cammino[i] = dest->codice;
      dest = dest->pred;
    }
    // adesso per ogni codice ricavo l'attore per la stampa
    for(int i=0;i<=len;i++){
      attore* a = bsearch(&cammino[i],gr,grl,sizeof(attore),(__compar_fn_t) &(cmp_intatt));
      fprintf(f,"%d\t%s\t%d\n",a->codice,a->nome,a->anno);
    }
    // dealloco cammino
    free(cammino);
    printf("%s: Lunghezza minima %d. Tempo di elaborazione %.3f secondi\n",buff,len,eltime);
  } 
  else { // esito negativo
    printf("%s: Nessun cammino! Tempo di elaborazione %.3f secondi\n",buff,eltime);
    if(ctrl==0) { // nessun cammino ma sorgente valida
      fprintf(f,"Non esistono cammini da %d a %d!\n",a,b);
    } 
    else { // ==-1-> nessun cammino perche sorgente NON valida
      fprintf(f,"Codice %d NON valido!\n",a);
    }
  }
  if(fclose(f)==EOF) xtermina("Errore chiusura file cammino",QUI);
  return ;
}






//----- funzioni ABR (la funzione di ricerca non viene usata)
// funzione che crea un ABR
ABRnode* crea_abr(int c, int d, ABRnode* pred)
{
  // allochiamo lo spazio per un nodo e restituiamo il puntatore la nodo creato
  ABRnode* node = malloc(sizeof(ABRnode));
  assert(node!=NULL);
  *node = (ABRnode){
    .codice = c,
    .depth = d,
    .pred = pred,
    .sx = NULL,
    .dx = NULL
  };
  return node;
}

// funzione di inserimento nodo in ABR
// *root NON VA BENE perche non modificherei davvero i valori di root->sx e root->dx (modifiche solo locali)
int insert_abr(ABRnode **root, ABRnode *node) 
{
  assert(node != NULL);
  if (*root == NULL) {
    *root = node;  // si inserisce il nodo
    return 1;      // inserito con successo
  }
  if ((*root)->codice == node->codice) {
    free(node);    // nodo già presente, lo scartiamo
    return 0;      // non inserito
  }
  // inserimento nei sotto alberi
  // per il confronto e criterio di ordinamento si usa la funzione shuffle che bilancia un po l'albero
  int src = shuffle((*root)->codice);
  int snc = shuffle(node->codice);
  // per non salvare la differenza (problematica se restituisce valori grandi) si fa prima un confronto 
  int diff = (src > snc) - (src < snc);  // == 1,0,-1
  if (diff > 0) { //== 1-> rc>nc
    return insert_abr(&(*root)->sx, node);  // vai a sinistra
  } else {  // == 0/-1-> rc<=nc
    return insert_abr(&(*root)->dx, node);  // vai a destra
  }
}

// funzione di ricerca in ABR
ABRnode* search_abr(ABRnode* root, int codice)
{
  if(root==NULL) return NULL;
  else if(root->codice == codice) return root;
  else{
    int src = shuffle(root->codice);
    int sc = shuffle(codice);
    int diff = (src > sc) - (src < sc);
    if(diff==1) // src > sc
      return search_abr(root->sx,codice);
    else // src <= sc
      return search_abr(root->dx,codice);
  }
}






// ----- funzioni Linked-List
// funzione di inserimento di un codice di attore in coda bfs
void push(FIFO* q, int codice) 
{
  // Se la FIFO è piena, dobbiamo eseguire "una realloc" MA NON POSSIAMO FARLO SEMPLICEMENTE
  // essendo un array circolare una semplice realloc copia gli elem nei soliti indirizzi precedenti ma se la coda è "spezzata"
  // nell'array circolare questo non funzionerà!
  assert(q != NULL);
  assert(q->queue != NULL);
  assert(q->size <= q->cap);

  // Se pieno, raddoppia la capacità
  if (q->size == q->cap) {
    int newcap = q->cap * 2;
    int* newqueue = malloc(sizeof(int) * newcap);

    // Ricopia i dati in ordine corretto
    for (int i = 0; i < q->size; ++i) {
      // la coda può essere "spezzata" si deve calcolare il corretto indice
      int idx = (q->head + i) % q->cap;
      newqueue[i] = q->queue[idx];
    }

    free(q->queue);
    q->queue = newqueue;
    q->head = 0;
    q->tail = q->size;
    q->cap = newcap;
  }

  // Inserisce in coda
  q->queue[q->tail] = codice;

  q->tail = (q->tail + 1) % q->cap;
  q->size += 1;
}


// funzione di estrazione di un nodo da FIFO
int pop(FIFO* q)
{
  assert(q != NULL);
  assert(q->queue != NULL);
  assert(q->size > 0);
  int code = q->queue[q->head];
  q->head = (q->head + 1) % q->cap;
  q->size -= 1;
  return code;
}





// ----- funzioni di debug
// stampa tutto il grafo creato su stderr
void stampa_gr(attore* gr, int grl)
{
  for(int i=0;i<grl;i++){
    fprintf(stderr,"%d\t%s\t%d\t\t%d\t",gr[i].codice,gr[i].nome,gr[i].anno,gr[i].numcop);
    for(int j=0;j<gr[i].numcop;j++)
      fprintf(stderr,"%d\t",gr[i].cop[j]);
    puts("");
  }
}
