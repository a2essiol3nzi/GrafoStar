import java.util.Map;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Set;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileReader;
import java.io.FileWriter;


public class CreaGrafo {

  public static void main(String[] argv){
    
    // argomenti sbagliati
    if(argv.length!=2) { 
      System.out.println("Per creare il GRAFO DELLE STAR uso: 'java CreaGrafo attori.tsv titoli.tsv'");
      System.exit(1);
    }


    // ===== LETTURA NAME.BASICS.TSV - SCRITTURA NOMI.TXT ===== 
    System.err.println("===== LETTURA NAME.BASICS.TSV - SCRITTURA NOMI.TXT =====");
    // creo la mappa dagli attori ai codici
    Map<Integer,Attore> attori = new HashMap<Integer,Attore>();
    // Array dei codici  ordinati degli attori
    ArrayList<Integer> codici = new ArrayList<Integer>();

    try{
      /* 
        1. apro attori.tsv in lettura con buffer da 64KB (invece che da 8KB come di default) la scelta è motivata 
        dalla grandezza dei file: per minimizzare il numero di accessi al disco il modulo Java Buffer
        usa già una sua "bufferizzazione" dei file da 8KB, la faccio da 64KB per ottimizzare: 2^10 * 2^6= 65536 (stessa cosa per i file in scrittura)
          i tempi rientrano nei limiti come richiesto anche con buffer standard, si tratta solo di un dettaglio incontrato durante la lettura della funz
          nei manuali: https://docs.oracle.com/javase/8/docs/api/java/io/BufferedReader.html
        2. per ordinare i codici degli attori (che ci serviranno sia in nomi.txt) si usa un'istanza di ArrayList 
        che riordiniamo per codice crescente in modo da poterlo scorrere e accedere agli attori in base alle chiavi (codici ordinati) nella HashMap
        contenente le associazioni tra codice attore e istanza di Attore
      */
      BufferedReader br = new BufferedReader(new FileReader(argv[0]), 65536);
      String line;
      // si deve scartare la prima riga che non ci interessa (specifica la suddivisione dei campi nei file .tsv)
      br.readLine();
      
      while((line = br.readLine())!=null) {
        // verifico che la linea sia valida (anno di nascita e professione) se lo è allora aggiorno codici e attori
        // tokenizzo la stringa usando come delim "\t"
        String[] campi = line.split("\t");

        // verifico le condizioni e agisco di conseguenza
        if(!campi[2].equals("\\N")){
          String[] professions = campi[4].split(",");
          for(String p: professions){
            if(p.equals("actor") || p.equals("actress")){ 
              // entrambe le cond verificate-> creiamo un nuovo Attore
              int c = Integer.parseInt(campi[0].substring(2));
              Attore a = new Attore(c, campi[1], Integer.parseInt(campi[2]));
              // aggiorno la lista dei codici
              codici.add(c);
              // aggiorno attori
              attori.put(c, a);
              break;
            }
          }
        }
        // se una delle cond non è verificata non faccio niente e passo alla prossima linea
      }
      // chiudo file attori.tsv
      br.close();

      System.err.println("--- Creazione 'nomi.txt' ---");
      // si passa alla scrittura di nomi.txt
      BufferedWriter bw = new BufferedWriter(new FileWriter("nomi.txt"), 65536);
      // i nomi devono essere scritti in ordine crescente di codice -> riordino le chiavi in codici tramite le quali 
      // accedo in maniera ordinata ad attori e poi da ogni chiave ricavo l'attore associati da scrivere su nomi.txt
      // vettore con codici degli attori validi ordinati in ordine crescente (solo inseriti vanno ordinati)
      codici.sort((a,b)-> a-b);

      // stampo ogni attore associato alle chiavi
      for(int k: codici)
        bw.write(attori.get(k).toString() + "\n");
      // al termine richiudo il file nomi.txt
      bw.close();
      
    } catch(Exception e) {
      System.err.println("Errore produzione nomi.txt: " + e);
      e.printStackTrace();
      System.exit(2);
    }



    // ===== LETTURA TITLE.PRINCIPALS.TSV - SCRITTURA GRAFO.TXT ===== 
    System.err.println("===== LETTURA TITLE.PRINCIPALS.TSV - SCRITTURA GRAFO.TXT =====");
    // mappa film-cast 
    Map<Integer,Set<Integer>> films = new HashMap<Integer,Set<Integer>>();

    try{
      /*
        1. si apre titoli.tsv in lettura da cui sintetizziamo film e i relativi cast di attori-> 
        HashMap<Integer (film), HashSet<Integer> (cast= insieme dei codici degli attori che vi hanno partecipato)>
        2. per ogni film (chiave) usiamo il cast (set associato) per la costruzione dei campi coprotagonisti di ogni attore coinvolto,
        per ogni attore nel cast devono essere aggiunti tutti gli altri attori al proprio insieme di coprotagonisti (eccetto se stesso)
        3. usiamo codici (Arraylist ordinato) per lo scorrimento e l'accesso agli attori, 
        e stampo in grafo.txt la "matrice di adiacenza" (nel formato richiesto -> Attore.toNode()) dell'intero grafo costruibile
        
        ATT: non tutti i collaboratori ad un film potrebbero essere per forza attori! Andrà controllato tramite l'HashMap di attori creata con i soli attori validi
      */
      // apro il file dei titoli in lettura
      BufferedReader br = new BufferedReader(new FileReader(argv[1]), 65536);
      String line;
      // si deve scartare la prima riga che non ci interessa
      br.readLine();

      // devo sintetizzare i film e i rispettivi cast in films
      // per farlo leggo una riga, leggo il film: se è nuovo creo una nuova chiave in films e aggiungo l'attore
      // altrimenti aggiungo l'attore al set associato alla chiave già presente nella HashMap
      while((line = br.readLine())!=null){
        // tokenizzo la linea
        String[] campi = line.split("\t");

        // verifico se il collaboratore del film è un attore
        int att_cod = Integer.parseInt(campi[2].substring(2));
        if(attori.containsKey(att_cod)){
          // se abbiamo un attore del film dobbbiamo aggiungerlo al relativo cast -> prendo la chiave dal primo campo
          int titolo = Integer.parseInt(campi[0].substring(2));
          if(!films.containsKey(titolo)) // nuovo film
            films.put(titolo, new HashSet<Integer>());  // uso un HashSet perche le operazioni base hanno costo O(1)
          // aggiorno il cast
          films.get(titolo).add(att_cod);
        }
        // altrimenti nulla! non si tratta di un attore quindi non ci interessa
      }
      
      // chiudo titoli.tsv
      br.close();

      // si passa alla definizione dei campi coprotagonisti di ogni Attore
      for(Set<Integer> s: films.values()){
        for(int a: s){
          Attore att = attori.get(a);
          // aggiorno i coprot dell'attore
          att.addCop(s);
        }
      }

      System.err.println("--- Creazione 'grafo.txt' ---");
      // dobbiamo scrivere grafo.txt: 
      BufferedWriter bw = new BufferedWriter(new FileWriter("grafo.txt"), 65536);

      // scorriamo codici per l'ordinamento crescente
      for(int c: codici){
        // tramite la HashMap accediamo all'attore
        Attore a = attori.get(c);
        // stampiamo l'attore sottoforma di nodo nel formato richiesto
        bw.write(a.toNode() + "\n");
      }
      // chiusura file grafo.txt
      bw.close();
      

    } catch (Exception e){
      System.err.println("Errore produzione grafo.txt: " + e);
      e.printStackTrace();
      System.exit(3);
    }

  }
}
