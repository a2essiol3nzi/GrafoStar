import java.util.Map;
import java.util.ArrayList;
import java.util.HashMap;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileReader;
import java.io.FileWriter;




public class CreaGrafo {


  public static void main(String[] args){
    
    // argomenti sbagliati
    if(args.length!=1) { 
      System.out.println("Per creare il GRAFO DELLE STAR uso: 'java CreaGrafo attori.tsv titoli.tsv'");
      System.exit(1);
    }


    // ===== LETTURA NAME.BASICS.TSV - SCRITTURA NOMI.TXT ===== 
    // creo la mappa dagli attori ai codici
    Map<Integer,Attore> actors = new HashMap<Integer,Attore>();
    // vettore con codici degli attori validi ordinati in ordine crescente
    ArrayList<Integer> codici;

    try{
      /* 
        1. apro name.tsv in lettura con buffer da 64KB (invece che da 8KB come di default) la scelta è motivata 
        dalla grandezza dei file, per minimizzare il numero di accessi al disco il modulo Buffer
        usa già una sua bufferizzazione da 8KB, la facciamo da 64KB per ottimizzare: 2^10 * 2^6= 65536 (stessa cosa per il file in scrittura)
        2. per ordinare i codici degli attori (che ci serviranno sia in nomi.txt che in grafo.txt) si usa un'istanza di ArrayList 
        che riordiniamo per codice crescente in modo da poterlo scorrere e accedere agli attori in base alle chiavi (codici ordinati) nella HashMap
      */
      BufferedReader br = new BufferedReader(new FileReader(args[0]), 65536);
      String line;
      
      while((line = br.readLine())!=null) {
        // verifico che la linea sia valida (anno di nascita e professione) se lo è allora aggiorno nomi.txt e actors
        String[] campi = line.split("\t");
        for (int i = 0; i < campi.length; i++)
          campi[i] = campi[i].trim();
        // verifico le condizioni e agisco di conseguenza
        if(!campi[2].equals("\\N")){
          String[] professions = campi[4].split(",");
          for(String p: professions){
            if(p.equals("actor") || p.equals("actress")){ 
              // entrambe le cond verificate-> creiamo un nuovo Attore
              Attore a = new Attore(
                Integer.parseInt(campi[0].substring(2)), campi[1], Integer.parseInt(campi[2]));
              // aggiorno actors
              actors.put(a.codice, a);
              break;
            }
          }
        }
        // se una delle cond non è verificata non faccio niente e passo alla prossima linea
      }
      // chiudo file name.basics.tsv
      br.close();

      // si passa alla scrittura di nomi.txt
      BufferedWriter bw = new BufferedWriter(new FileWriter("nomi.txt"), 65536);
      // i nomi devono essere scritti in ordine crescente di codice -> riordino le chiavi in HashMap e poi da ognuna ricavo l'attore
      codici = new ArrayList<>(actors.keySet());
      codici.sort((a,b)-> a-b);
      // stampo ogni attore associato alle chiavi
      for(Integer i: codici){
        bw.write(actors.get(i).toString() + "\n");
      }
      // al termine richiudo il file nomi.txt
      bw.close();
      
    } catch(Exception e) {
      System.err.println("Errore produzione nomi.txt: " + e);
      e.printStackTrace();
      System.exit(2);
    }


    // ===== LETTURA TITLE.PRINCIPALS.TSV - SCRITTURA GRAFO.TXT ===== 
    
    try{

    } catch (Exception e){
      System.err.println("Errore produzione grafo.txt: " + e);
      e.printStackTrace();
      System.exit(3);
    }




  }
}
