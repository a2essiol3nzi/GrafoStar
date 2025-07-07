# Progetto Lab2 24/25: 
Testo: https://elearning.di.unipi.it/mod/page/view.php?id=22794 (progetto ridotto)

***Specifiche richieste dal progetto!**

## CreaGrafo.java
**1. Parsing delle righe:** Si apre il ciclo while fino a che non terminano le righe da leggere (e di cui fare il parsign). All'interno del while si esegue il parsing di una riga letta: 1. si esegue **String.split("\t")** così da ricavare i campi della linea definiti nella prima riga del file (che sappiamo appunto essere separati da "\t"); 2. per **verificare la "validità"** della linea controlliamo il campo contenente l'anno di nascita (campi[2]) convertendolo da String ad Integer, e se tra le professioni (campi[4]) abbiamo attore/attrice (si usa .contains(substring) per verificare). 
Se le cond. sono verificate allora si ricava un istanza di Attore dalla linea e la inseriamo nella mappa <codice,Attore> che usiamo nel programma. Alla fine del while chiuderemo il file e procederemo col resto del programma.  


## cammini.c
**2. **  

**3. Ricostruzione cammini minimi:** Per la gestione delle stampe usiamo una funzione dedicata (utils.) che ,in base ad un certo valore di controllo passato in base all'esito della BFS, esegue la corretta stampa. 
La funzione è **void stampa_minpath()** (nel caso in cui esista un cammino tra a e b) per la ricostruzione del cammino utilizza i campi dei nodi ABR: ognuno di questi nodi contiene un **campo predecessore** che punta al predecessore nella BFS, di conseguenza partendo da qualsiasi nodo è possibile ricostruire facilmente il path da nodo a sorgente. Passando alla funzione un puntatore al nodo destinazione (assumendo che esista e di averlo trovato) questa è in grado di ricostruire il path MA IN ORDINE INVERSO, per leggerlo in modo corretto **ci appoggiamo ad un array dinamico** in cui viene temporaneamente salvato la sequenza di nodi (codici di tipo intero) del cammino, e poi letto nell'ordine opportuno per stampare il cammino partendo dalla sorgente fino alla nostra destinazione: si seguono i puntatori predecessori fino alla sorg inserendo nell'array partendo dal fondo, alla fine rileggeremo l'array partendo dall'inizio -> **ordine corretto**! Eseguendo una bsearch di ogni codice sul grafo ricaviamo un puntatore all'attore con le informazioni necessarie alla stampa.  


**4. Gestione SIGINT:** All'inizio del programma nel main viene creato ed avviato un thread gestore del segnale SIGINT, il thread viene avviato con la funzione **handler_body** e gli viene passata come argomento una struttura speciale **datisighand** contenente due flags (variabili condivise con main di tipo "volatile sig_atomic_t") che definiscono il comportamento dell'handler: **p_state** è un (puntatore) valore condiviso col main thread che rappresenta lo stato della pipe (se è stat creata oppure no), viene quindi settato dal main; **term** invece è un (puntatore) valore sempre condiviso tra main thread e gestore, ma che viene acceduto solo in caso si riceva un SIGINT dopo il cambio di stato della pipe (viene settato solo da thread gestore). In tal caso il gestore deve comunicare al main di terminare, per farlo setta term ad 1 ed il main (che dalla creazione della pipe lo controllerà periodicamente) terminerà nella modalità richiesta.  
