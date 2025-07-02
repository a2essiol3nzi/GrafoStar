# Progetto Lab2 24/25: 
Testo: https://elearning.di.unipi.it/mod/page/view.php?id=22794 (progetto ridotto)

***Specifiche richieste dal progetto!***

## CreaGrafo.java
**1. Parsing delle righe:** Per il parsing di ogni riga letta (da attori.tsv e titoli.tsv) viene usato il metodo "**private static String[] tokenize(String line)**": questo metodo ricava dalla linea un array di Strig tramite il metodo String.split(String delim) (simile a strtok) con "\t" come delimitatore per la tokenizzazione; dopodiché su ogni String ottenuta si applica il metodo String.trim() che elimina gli spazi all'inizio e alla fine di una stringa.  
A questo punto i vari campi saranno acceduti e modificati in base all'occorrenza.  

## cammini.c

**4. Gestione SIGINT:** All'inizio del programma nel main viene creato ed avviato un thread gestore del segnale SIGINT, il thread viene avviato con la funzione "handler_body" e gli viene passata come argomento una struttura speciale "datisighand" contenente due flags che definiscono il comportamento dell'handler: **p_state** è un (puntatore) valore condiviso col main thread che rappresenta lo stato della pipe (se è stat creata oppure no), viene quindi settato dal main; **term** invece è un (puntatore) valore sempre condiviso tra main thread e gestore, ma che viene acceduto solo in caso si riceva un SIGINT dopo il cambio di stato della pipe. In tal caso il gestore deve comunicare al main di terminare, per farlo setta term ad 1 ed il main (che dalla creazione della pipe lo controllerà periodicamente) terminerà nella modalità richiesta.
