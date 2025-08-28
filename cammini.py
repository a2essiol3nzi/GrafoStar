#!/usr/bin/env python3

Description = """
Script per testare il programma cammini.out
Richiede sulla linea di comando k>1 interi 
    i1 i2 i3 .. ik
e invia sulla pipe cammini.pipe (o altra specificata con -p) le coppie 
    (i1,i2), (i1,i3), (i1,i4), ... (ik-1,ik)  
in modo da richiedere a cammini.out di calcolare il camminimo minimo 
fra ogni coppia. Inviate le coppie attende per un numero di secondi
specificato con -s (default 0) e poi chiude la pipe.

Se viene specificata l'opzione -i pid, subito prima di chiudere la pipe 
invia un segnale SIGINT al pid specificato. Questa opzione può essere usata 
per testare la gestione di SIGINT da parte di cammini.out. 
"""
import os, signal, argparse
import sys, struct, time

def main(args):
  # apre la pipe in scrittura
  while True:
    try:
      pipe = os.open(args.p, os.O_WRONLY)
      break
    except FileNotFoundError:
      print(f"--- Pipe {args.p} non pronta, attendo...")
      time.sleep(2)
  print("=== pipe aperta in scrittura")
  # scrive le coppie di attori sulla pipe
  for i in range(len(args.codici)-1):
    # scrive i due attori sulla pipe
    buf = struct.pack('ii', args.codici[i], args.codici[i+1])
    os.write(pipe, buf)
  print("=== scrittura coppie completata")
  # attende il numero di secondi specificato
  print(f"=== attendo {args.s} secondi")
  time.sleep(args.s)
  # se opzione -i specificata, invia SIGINT al pid specificato
  if args.i > 0:
    print(f"Invio SIGINT al processo {args.i}")
    try:
      os.kill(args.i, signal.SIGINT)
    except ProcessLookupError:
      print(f"--- Errore: processo {args.i} non trovato")
    except PermissionError:
      print(f"--- Errore: permesso negato per inviare SIGINT a {args.i}")
    except Exception as e:
      print(f"--- Errore: {e}")
  # chiude la pipe
  os.close(pipe)
  print("=== pipe chiusa in scrittura, esecuzione terminata")

  
if __name__ == '__main__':
  parser = argparse.ArgumentParser(description=Description, formatter_class=argparse.RawTextHelpFormatter)
  parser.add_argument('codici', nargs='+', type=int, metavar='codice', help='lista di codici di attori (almeno 2)')
  parser.add_argument('-p', help='nome pipe', type=str, default='cammini.pipe', metavar='pipe')
  parser.add_argument('-i', help='pid a cui inviare SIGINT', type=int, default=0, metavar='pid')
  parser.add_argument('-s', help='secondi attesa prima di inviare SIGINT (def. 0 sec)', type=float, default=0, metavar='secs')
  args = parser.parse_args()
  if len(args.codici) < 2:
    print("Devi passare almeno 2 codici di attori")
    sys.exit(1)
  main(args)
