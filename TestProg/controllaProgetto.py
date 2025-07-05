#!/usr/bin/env python3

import sys,  argparse, subprocess, os.path, os
import concurrent.futures

Description = f"""
Controlla compilazione e semplice esecuzione progetto Laboratorio2B 2024/25
"""


Timelimit = 5*60 # seconds, i.e. 5 minuti 


# execute command in a separate process returning True is everything OK, False otherwise
# command must be a list with program name + options
def execute_command(command,logfile=sys.stdout):
    print(">>> " + " ".join(command),file=logfile)
    try:
      ris = subprocess.run(command,encoding='UTF-8',
                           capture_output = True, timeout=Timelimit)
    except subprocess.TimeoutExpired as ex:
      # caso time out
      print("## Timeout: no result after %d seconds" % Timelimit, file=logfile)
      print("## stdout:  ", ex.stdout.decode() if ex.stdout else None,file=logfile)
      print("## stderr:  ", ex.stderr.decode() if ex.stderr else None,file=logfile)
      concludi(100)
    except Exception as ex:
      print("## Esecuzione comando fallita:", str(ex), file=logfile)
      concludi(101)
    return ris


  
def concludi(c,msg=""):
  if len(msg)>0:
    print(f"Codice di terminazione: {c}, Msg: {msg}")
  print("Il progetto non può essere consegnato in questa forma:")
  print("correggi gli errori e ri-esegui il test")
  sys.exit(c)


def controlla_lunghezza(nome,lunghezza):
  if not os.path.exists(nome):
    print(f"Il file {nome} avrebbe dovuto essere creato da cammini.out")
    concludi(11)
  cmd = ["wc", "-l", nome]
  ris = execute_command(cmd,sys.stdout)
  if ris.returncode!=0:
    print(f"Errore esecuzione: {' '.join(cmd)}. Exit code {ris.returncode}" )
    print(f"### stdout:\n{ris.stdout}")
    print(f"### stderr:\n{ris.stderr}")
    concludi(34)
  # controlla lunghezza
  if int(ris.stdout.split()[0]) != lunghezza:
    print(f"Il file {nome} dovrebbe avere {lunghezza} righe")
    concludi(35)  


def test_progetto():
  parser = argparse.ArgumentParser(description=Description, formatter_class=argparse.RawTextHelpFormatter)
  parser.add_argument('-r', help='test progetto ridotto',action='store_true')
  args = parser.parse_args()

  print("### Inizio test progetto " +("*Ridotto*" if args.r else "*Completo*"))

  nome  = "miniN.tsv"
  if not os.path.exists(nome):
    print(f"Il file {nome} deve essere presente nella directory corrente\n(va bene anche un link simbolico)")
    sys.exit(1) 
  nome  = "title.1m.tsv"
  if not os.path.exists(nome):
    print(f"Il file {nome} deve essere presente nella directory corrente\n(va bene anche un link simbolico)")
    sys.exit(1) 
  nome = "cammini.py"
  if not os.path.exists(nome):
    print(f"Il file {nome} deve essere presente nella directory corrente\n(va bene anche un link simbolico)")
    sys.exit(1)
  if not args.r:
    nome = "collaborazioni.py"
    if not os.path.exists(nome):
      print(f"Per il progetto completo il file {nome} deve essere presente\nnella directory corrente (va bene anche un link simbolico)")
      sys.exit(1)
    nome = "title.basics.tsv"
    if not os.path.exists(nome):
      print(f"Per il progetto completo il file {nome} deve essere presente\nnella directory corrente (va bene anche un link simbolico)")
      sys.exit(1)


  # cancella i file eseguibili eventualmente presenti
  cmd = ["rm", "-f", "cammini.out", "CreaGrafo.class", "cammini.pipe"]
  ris = execute_command(cmd,sys.stdout)
  if ris.returncode!=0:
    print(f"Errore esecuzione: {' '.join(cmd)}. Exit code {ris.returncode}" )
    print(f"### stdout:\n{ris.stdout}")
    print(f"### stderr:\n{ris.stderr}")
    concludi(11)

  # ---- make ---------
  cmd = ["make"]   
  ris =  execute_command(cmd, sys.stdout)
  if ris.returncode!=0:
    print(f"Errore esecuzione: {' '.join(cmd)}. Exit code {ris.returncode}" )
    print(f"### stdout:\n{ris.stdout}")
    print(f"### stderr:\n{ris.stderr}")
    concludi(21)
  # ---- verifica presenza file CreaGrafo.class
  nome = "CreaGrafo.class" 
  if not os.path.exists(nome):
    print(f"Il file {nome} avrebbe dovuto essere creato da make")
    concludi(22)

  # ---- verifica presenza file cammini.out
  nome = "cammini.out" 
  if not os.path.exists(nome):
    print(f"Il file {nome} avrebbe dovuto essere creato da make")
    concludi(23)
  if not os.access(nome, os.X_OK):
    print(f"Il file {nome} dovrebbe essere eseguibile")
    concludi(24)
  
  # cancella nomi.txt, grafo.txt, partecipazioni.txt
  cmd = ["rm", "-f", "nomi.txt", "grafo.txt", "partecipazioni.txt"]
  ris = execute_command(cmd,sys.stdout)
  if ris.returncode!=0:
    print(f"Errore esecuzione: {' '.join(cmd)}. Exit code {ris.returncode}" )
    print(f"### stdout:\n{ris.stdout}")
    print(f"### stderr:\n{ris.stderr}")
    concludi(25) 
  # ---- esegue crea grafo su file di testo forniti
  cmd = ["java", "-Xmx8g", "CreaGrafo", "miniN.tsv", "title.1m.tsv"]
  ris = execute_command(cmd,sys.stdout)
  if ris.returncode!=0:
    print(f"Errore esecuzione: {' '.join(cmd)}. Exit code {ris.returncode}" )
    print(f"### stdout:\n{ris.stdout}")
    print(f"### stderr:\n{ris.stderr}")
    concludi(26)
  # verifica presenza file nomi.txt, grafo.txt
  nome = "nomi.txt"
  if not os.path.exists(nome):
    print(f"Il file {nome} avrebbe dovuto essere creato da CreaGrafo")
    concludi(27)  
  nome = "grafo.txt"
  if not os.path.exists(nome):
    print(f"Il file {nome} avrebbe dovuto essere creato da CreaGrafo")
    concludi(28)  
  # se il progetto non è ridotto verifica presenza file partecipazioni.txt
  if not args.r:
    nome = "partecipazioni.txt"
    if not os.path.exists(nome):
      print(f"Il file {nome} avrebbe dovuto essere creato da CreaGrafo")
      concludi(29)

  # ---cancella file di output
  cmd = ["rm", "-f", "38.80", "80.148", "148.100"]
  ris = execute_command(cmd,sys.stdout)
  if ris.returncode!=0:
    print(f"Errore esecuzione: {' '.join(cmd)}. Exit code {ris.returncode}" )
    print(f"### stdout:\n{ris.stdout}")
    print(f"### stderr:\n{ris.stderr}")
    concludi(30)
  # ---- esegue cammini.out e cammini.py su file di testo forniti
  with concurrent.futures.ThreadPoolExecutor(max_workers=2) as executor:
    f1 = executor.submit(execute_command, ["./cammini.out", "nomi.txt", "grafo.txt", "4"])
    f2 = executor.submit(execute_command, ["python3", "cammini.py", "38", "80", "148", "100", "-s", "2"])
  # --- questo serve per interrompere il test se c'è un timeout 
  if(f1.result().returncode!=0): print("cammini.out ha restituito:", f1.result().returncode)
  if(f2.result().returncode!=0): print("cammini.py  ha restituito:", f2.result().returncode)
  # --- verifica presenza file di output
  nome = "38.80"
  controlla_lunghezza(nome,3)
  nome = "80.148"
  controlla_lunghezza(nome,4)
  nome = "148.100"
  controlla_lunghezza(nome,1)

  # --- esegui test di collaboratori
  if not args.r:
    cmd = ["python3", "collaborazioni.py", "partecipazioni.txt", "title.basics.tsv","80", "39", "432", "148" ]
    ris = execute_command(cmd,sys.stdout)
    if ris.returncode!=0:
      print(f"Errore esecuzione: {' '.join(cmd)}. Exit code {ris.returncode}" )
      print(f"### stdout:\n{ris.stdout}")
      print(f"### stderr:\n{ris.stderr}")
      concludi(32)
    titolo = "The Conversation"  
    if not titolo in ris.stdout:
      print(f"L'output di collaborazioni.py non contiene '{titolo}'")
      concludi(33,ris.stdout)
    titolo = "Casino Royale"
    if not titolo in ris.stdout:
      print(f"L'output di collaborazioni.py non contiene '{titolo}'")
      concludi(34,ris.stdout)
    # controlla lunghezza
    
  # test concluso
  print("### Test concluso senza rilevare errori")
  sys.exit(0) 



if __name__ == '__main__':
  test_progetto()
