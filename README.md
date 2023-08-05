# SOproject

progect of operative sistem based on implements a simulation of the sea's tread

cose importanti da vedere:
-correggere le nanosleep.............

  -impostare il timer alle merci che inizi ogni qualvolta che la merce viene creata
  -inserire una pipe o una msg per l'invio di messaggi tra i processi.

- controllare se la nave attacca al porto in modo corretto e se il criterio di scambio delle merci funziona correttamente
  /\*

\*/
--------DESCRIZIONE DEL PROGETTO---------

---> MEMORIE CONDIVISE
le memorie condivise vengono utilizzate molte volte all'interno del progetto , lo scopo principale è quello per la configurazione dei vari processi creati dal master, come ad esempio le navi e i porti.
un altro utilizzo vene fatto per la gestione delle merci nei porti, le navi arrivate alla banchina richiedono l'accesso alla memroia condivisa delle merci dei porti in cui vengono salvate tutte le quantità a disposizione dei porti.

---> SEMAFORI
i semafori vengono utilizzati per molteplici scopi. ad esempio ci sonom due semafori per gestire la partenza in contemporanea di tutti i processi, un semaforo che ferma temporanemente il master fino a quando tutti i processi non sono stati creati, quando la creazione e la configurazione iniziale dei processi termina allora il master rilascia una risorsa per ogni processo in modo che la partenza delle navi avvenga tutta nello stesso momento.

i semafori vengono utilizzati anche nelle banchine e per l'accesso di una nave alla memoria condivisa dei porti per aggiornare le merci scambiate tra la nave e i porti.

--->
