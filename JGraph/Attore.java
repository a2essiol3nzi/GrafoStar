import java.util.ArrayList;
import java.util.HashSet;
import java.util.Set;

public class Attore {
  int codice;                   // codice univoco dell'attore
  String nome;                  // none (e cognome) dell'attore
  int anno;                     // anno di nascita
  Set<Integer> coprotagonisti;  // codici degli attori che hanno recitato con this


  // inizializziamo tutti i campi tranne coprotagonisti
  Attore(int code, String nome_cognome, int birthyear){
    this.codice = code;
    this.nome = nome_cognome;
    this.anno = birthyear;
    // implemento Set tramite HashSet per inserirli con costo O(1), dopodiche li riordinerò per leggerli
    // meno efficiente in termini di spazio, fa una copia dell'intero Set e lo riordina, ma MOLTO più veloce!
    // se avessimo dovuto accedere più di una volta al campo coprot in ordine crescente di codice, allora forse era più conveniente TreeSet
    this.coprotagonisti = new HashSet<Integer>(); 
  }

  // override del metodo già esistente per la stampa 
  @Override
  public String toString() {
    return String.format("%d\t%s\t%d",codice,nome,anno);
  }

  // Metodo della classe attore che ricava la rappresentazione dell'attore come nodo del GRAFO DELLE STAR: 
  // codice coprot.size() coprot.sort()
  public String toNode() {
    ArrayList<Integer> aC = new ArrayList<Integer>(this.coprotagonisti);
    // devo riordinarli adesso
    aC.sort((a,b)-> a-b);
    // usare l'op di concatenazione è troppo costoso, String è immutabile percio ogni volta viene creato una nuova istanza,
    // Stringbuilder mette a disposizione metodi per la costruzione di una stringa dinamica, come append()
    StringBuilder copString = new StringBuilder();
    // concatenazione dei coprot
    for(int i: aC)
      copString.append("\t").append(i);
    return String.format("%d\t%d%s", this.codice,aC.size(),copString);
  }


  // metodo per l'aggiornamento dei coprotag
  public void addCop(Set<Integer> cast) {
    for(Integer c: cast){
      // un attore non può far parte dei suoi coprot
      if(c != this.codice)
        this.coprotagonisti.add(c); 
    }
  }

}
