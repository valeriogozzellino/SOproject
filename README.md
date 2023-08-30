# SOproject

progect of operative sistem based on implements a simulation of the sea's tread

--------DESCRIZIONE DEL PROGETTO---------
il progetto è una simulazione dell'interazione di navi in un contesto portuale. ci sono diversi file che compongono il progetto, tra i più importanti il master che avvia la simulazione e crea tutti i processi successivi come le navi, i porti e i processi per le perturbazioni atmosferiche. Il processo master gioca un ruolo molto importante, sfutta molte funzioni presenti nel file "configuration" per inizializzare in memoria condivisa tutti i parametri necessari alla simulazione, ha il compito di mantenere il pieno controllo sui processi creati, ad esempio,deve terminare la simulazione nel caso in cui non ci siano più navi nella mappa oppure se il tempo è terminato.
In secondo piano si hanno i processi "ship" e "port", le navi che vengono create dal master navigano all'interno di una mappa e in prossimità di un porto richiedono accesso alla banchina e eventualemente possono effettuare scambi di merce tra di loro. Entrambi hanno una funzione che controlla la scadenza della merce, ogni qualvolta una merce scade viene aggiornato un contatore che quantifica le merci scadute in mare o in porto.
altri processi sono il "dump", che mantiene aggiornato l'utente sullo stato della simulazione in modo costante, ogni azione rilevante nella simulazione viene registrata dal dump. poi si hanno i processi "maelstorm", "storm_duration" e "swell_duration" che controllano le perturbazioni atmosferiche del progetto, maeltorm ogni maelstorm_time colpisce una nave che viene affondata, storm, invece, rallenta le navi che si muovono nella mappa e infine swell duratio rallenta gli scambo di merce tra la nave e i porti ogni swell_time.

---> MEMORIE CONDIVISE
Le memorie condivise rivestono un ruolo di fondamentale importanza all'interno del progetto. Si tratta di risorse IPC che consentono la condivisione di memoria primaria tra diversi processi. Nel contesto del progetto, l'utilizzo della memoria condivisa è stato ampiamente frequente.

Ad esempio, il master utilizza la memoria condivisa per fornire alle navi, ai porti e ai processi di controllo delle perturbazioni atmosferiche i dati necessari per configurare i rispettivi processi. Questi dati includono anche informazioni riguardanti gli altri processi con cui interagiranno. Ad esempio, le navi e i porti possono accedere alla memoria condivisa dei porti per apportare modifiche alle quantità di merce scambiata. Nei processi portuali, in particolare, vengono inizializzate due memorie condivise per ciascun porto: una per l'offerta e una per la domanda di merci. Le scelte relative alla gestione delle dimensioni di quest'ultima hanno portato all'inizializzazione di una memoria condivisa con una lunghezza pari alla durata della simulazione. Questo consente di gestire le scadenze delle merci, verificando il giorno di creazione della merce. Per ogni giorno, viene creato un array con i tipi di merce offerti, i rispettivi lotti e la loro vita utile. Lo stesso principio si applica ai tipi di merce richiesti durante i giorni della simulazione.

---> SEMAFORI
I semafori svolgono diverse funzioni all'interno del progetto, e in particolare, sono presenti tre semafori all'interno della simulazione. Un semaforo con identificativo "sem_id" gestisce l'avvio dei processi nella simulazione. Una volta che tutti i processi sono stati creati e hanno contribuito con una risorsa al semaforo, il master viene rilasciato. Questo comporta il rilascio di un numero di risorse equivalente al numero di processi creati. Di conseguenza, tutti i processi iniziano la simulazione simultaneamente.

Un ulteriore semaforo, con identificativo "sem_id_banchine", gestisce l'accesso alle banchine all'interno di un porto. Viene creato un semaforo per ogni porto all'interno del set di semafori, e ognuno di essi possiede un numero di risorse corrispondente al numero di banchine presenti nel porto.

L'ultimo semaforo utilizzato, con identificativo "sem_id_shm", costituisce un set di (n^ porte) semafori. Ogni semaforo all'interno di questo set ha una singola risorsa. La funzione di questo set di semafori è di gestire la memoria condivisa contenente le informazioni sulle merci. Questo meccanismo impedisce che più navi possano scambiare merci con lo stesso porto simultaneamente. Ciò previene disallineamenti nei dati e garantisce che tutte le transazioni di merci si svolgano in modo sicuro ed esaustivo.

---> CODE DI MESSAGGI
Le code di messaggi costituiscono risorse IPC che agevolano la comunicazione tra i vari processi. Un processo può scrivere un messaggio di un determinato tipo all'interno di una coda, mentre un altro processo può leggerlo e svuotare la coda.

Nel contesto del progetto, le code di messaggi sono state impiegate dalle navi per comunicare con i porti riguardo all'ancoraggio alle banchine. Ogni volta che una nave giunge presso un porto, essa utilizza l'identificativo della coda di messaggi presente nella memoria condivisa. In tal modo, le navi filtrano le informazioni in base all'identificativo del porto e inviano il messaggio utilizzando il comando "msgsend".
