import java.util.Set;
import java.util.TreeSet;

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
    // implemento Set tramite TreeSet perché è importante l'ordine, dovranno essere stampati in ordine crescente.
    // TreeSet mantiene un ordinamento mano a mano che gli elementi vengono inseriti
    this.coprotagonisti = new TreeSet<Integer>(); 
  }

  @Override
  public String toString() {
    return String.format("%d\t%s\t%d",codice,nome,anno);
  }

  // Metodo della classe attore che ricava la rappresentazione dell'attore come nodo del GRAFO DELLE STAR: codice coprot.size() coprot.sort()
  public String toNode() {
    String copString = "";
    int ncollab = 0;
    // concatenazione dei coprot
    for(int i: this.coprotagonisti){
      copString = copString + "\t" + i;
      ncollab++;
    }
    return String.format("%d\t%d%s", this.codice,ncollab,copString);
  }


  // metodo per l'aggiornamento dei coprotag
  public void addCop(Set<Integer> cast) {
    for(Integer c: cast){
      // un attore non può far parte dei suoi coprot
      if(c != this.codice){
        this.coprotagonisti.add(c); // solitamente insieme a TreeSet ma se si lavora con semplici interi
                                      // l'ordinamento crescente è quello predefinito
      }
    }
  }

}
