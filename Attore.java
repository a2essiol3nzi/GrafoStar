import java.util.Set;

public class Attore {
  int codice;                   // codice univoco dell'attore
  String nome;                  // none (e cognome) dell'attore
  int anno;                     // anno di nascita
  Set<Integer> coprotagonisti;  // codici degli attori che hanno recitato con this  

  Attore(int code, String nome_cognome, int birthyear){
    this.codice = code;
    this.nome = nome_cognome;
    this.anno = birthyear;
  }

  @Override
  public String toString() {
    return String.format("%d\t%s\t%d",codice,nome,anno);
  }
}
