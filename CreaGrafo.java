import java.util.Map;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Set;
import java.util.stream.Collectors;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileReader;
import java.io.FileWriter;




public class CreaGrafo {

  // metodo per ricavare dalla forma tsv un intero (es. tt000034->34, nm023033->23033)
  private static int ricava_intero (String s){
    return Integer.parseInt(s.substring(2));
  }

  // tochenizzare una riga
  private static String[] tokenize(String line){
    String[] campi = line.split("\t");
    for (int i = 0; i < campi.length; i++)
      campi[i] = campi[i].trim();
    return campi;
  }

  // =========================== debug
  public static void stampaSetConSeparatore(Set<?> set, String separatore, String c) {
    String risultato = set.stream()
        .map(Object::toString)
        .collect(Collectors.joining(separatore));
    System.out.println(c + ": " + risultato);
  }

  // stampa Map
  public static void stampaMappa(Map<Integer, Set<Integer>> mappa) {
    for (Map.Entry<Integer, Set<Integer>> entry : mappa.entrySet()) {
      Integer chiave = entry.getKey();
      Set<Integer> valori = entry.getValue();

      // Costruisco la stringa dei valori separati da tab
      StringBuilder sb = new StringBuilder();
      for (Integer v : valori) {
        sb.append(v).append("\t");
      }

      // Rimuovo eventuale tab finale e stampo
      System.out.println(chiave + " -> " + sb.toString().trim());
    }
  }

  public static void stampaHashMapAttori(Map<Integer, Attore> mappa) {
    for (Map.Entry<Integer, Attore> entry : mappa.entrySet()) {
      Integer codice = entry.getKey();
      Attore attore = entry.getValue();
      // Costruiamo la stringa dei coprotagonisti separati da TAB
      StringBuilder copString = new StringBuilder();
      for (Integer cop : attore.coprotagonisti) {
          copString.append(cop).append("\t");
      }
      // Stampiamo: codice TAB ncollab TAB lista coprot
      System.out.println(codice + "\t" + attore.coprotagonisti.size() + "\t" + copString.toString().trim());
    }
  }



  // ===== main =====
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
        dalla grandezza dei file, per minimizzare il numero di accessi al disco il modulo Buffer
        usa già una sua bufferizzazione da 8KB, la facciamo da 64KB per ottimizzare: 2^10 * 2^6= 65536 (stessa cosa per il file in scrittura)
        2. per ordinare i codici degli attori (che ci serviranno sia in nomi.txt) si usa un'istanza di ArrayList 
        che riordiniamo per codice crescente in modo da poterlo scorrere e accedere agli attori in base alle chiavi (codici ordinati) nella HashMap
      */
      BufferedReader br = new BufferedReader(new FileReader(argv[0]), 65536);
      String line;
      // si deve scartare la prima riga che non ci interessa
      br.readLine();
      
      while((line = br.readLine())!=null) {
        // verifico che la linea sia valida (anno di nascita e professione) se lo è allora aggiorno nomi.txt e attori
        String[] campi = tokenize(line);

        // verifico le condizioni e agisco di conseguenza
        if(!campi[2].equals("\\N")){
          String[] professions = campi[4].split(",");
          for(String p: professions){
            if(p.equals("actor") || p.equals("actress")){ 
              // entrambe le cond verificate-> creiamo un nuovo Attore
              int c = ricava_intero(campi[0]);
              Attore a = new Attore(c, campi[1], Integer.parseInt(campi[2]));
              // e aggiorno la lista dei codici
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
      // i nomi devono essere scritti in ordine crescente di codice -> riordino le chiavi in HashMap e poi da ognuna ricavo l'attore
      // vettore con codici degli attori validi ordinati in ordine crescente (solo inseriti vanno ordinati)
      codici.sort((a,b)-> a-b);

      // stampo ogni attore associato alle chiavi
      for(Integer k: codici){
        bw.write(attori.get(k).toString() + "\n");
      }
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
        1. si apre titoli.tsv in lettura da cui sintetizziamo film e i relativi cast -> 
        HashMap<Integer (film), HashSet<Integer> (cast= insieme dei codici)>
        2. per ogni film (chiave) usiamo il cast (set associato) per la costruzione dei campi coprotagonisti di ogni attore coinvolto
        3. usiamo codici (Arraylist ordinato) per lo scorrimento e l'accesso agli attori e stampo in grafo.txt la "matrice di adiacenza" dell'intero grafo costruibile
        
        ATT: non tutti i collaboratori ad un film potrebbero essere per forza attori! Andrà controllato tramite l'HashMap di attori creata con i soli attori validi
        - Aggiunta un nuovo campo alla struttura Attore: ncollab
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
        String[] campi = tokenize(line);

        // verifico se il collaboratore del film è un attore
        int att_cod = ricava_intero(campi[2]);
        if(attori.containsKey(att_cod)){
          // se abbiamo un attore del film dobbbiamo aggiungerlo al relativo cast -> prendo la chiave dal primo campo
          int titolo = ricava_intero(campi[0]);
          if(!films.containsKey(titolo)) // nuovo film
            films.put(titolo, new HashSet<Integer>());  // uso un HashSet perche le operazioni base hanno costo O(1)
          // aggiorno il cast
          films.get(titolo).add(att_cod);
        }
        // altrimenti nulla! non si tratta di un attore quindi non ci interessa
      }
      
      // chiudo titoli.tsv
      br.close();

      // si passa alla definizione dei campi coprotagonisti e di ncollab di ogni Attore
      for(Set<Integer> s: films.values()){
        // devo aggiornare i vari campi coprot e ncollab di ogni attore del cast di un certo film
        for(int a: s){
          Attore att = attori.get(a);
          // aggiorno i coprot dell'attore (e ncollab)
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
