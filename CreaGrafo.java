import java.util.Map;
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

    // creo la mappa dagli attori ai codici
    Map<Integer,Attore> actors = new HashMap<Integer,Attore>();

    try{
      
      // apro name.tsv in lettura con buffer da 64KB (invece che da 8KB come di default)
      // apro nomi.txt in scrittura con buffer da 64KB
        /* la scelta è motivata dalla grandezza dei file, per minimizzare il numero di accessi al disco il modulo Buffer
        usa già una sua bufferizzazione da 8KB, la facciamo da 64KB per ottimizzare: 2^10 * 2^6= 65536 */
      BufferedReader br = new BufferedReader(new FileReader(args[0]), 65536);
      BufferedWriter bw = new BufferedWriter(new FileWriter("nomi.txt"), 65536);
      String line;
      
      // stampo su stderr le linee del file name.basic.tsv non valide
      System.err.println("=== Serie di linee invalide per la sintesi di un nodo del GRAFO DELLE STAR ===");
      while((line = br.readLine())!=null) {
        // verifico che la linea sia valida (anno di nascita e professione) se lo è allora aggiorno nomi.txt e actors
        String[] campi = line.split("\t");
        for(String c: campi) 
          c = c.trim();
        // verifico le condizioni e agisco di conseguenza
        if(!campi[2].equals("\\N")){
          String[] professions = campi[4].split(",");
          for(String p: professions){
            if(p.equals("actor") || p.equals("actress")){ 
              // entrambe le cond verificate-> creiamo un nuovo Attore
              Attore a = new Attore(
                Integer.parseInt(campi[0].substring(2)), campi[1], Integer.parseInt(campi[2]));
              // scrivo nomi.txt: codice  nome  anno\n
              bw.write(a + "\n");
              // aggiorno actors
              actors.put(a.codice, a);
            }
          }
        }
        // se una delle cond non è verificata non faccio niente e passo alla prossima linea
      }

      // chiudo i file aperti che ho finito di leggere/scrivere
      br.close();
      bw.close();
      
    } catch(Exception e) {
      System.err.println("Errore: " + e);
      e.printStackTrace();
      System.exit(2);
    }






  }
}
