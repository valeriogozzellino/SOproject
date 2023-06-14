# SOproject

progect of operative sistem qhich implement a simulation of the sea's tread

cose importanti da vedere:

- create good in port mi crea pochissimi lotti, ma può essere anche corretta perchè viene fatta giornalmente,
- verificare la life di una merce che sia minore dei gironi della simulazione almeno può scadere
- IMPORTANTE gli id dei porti sono sempre 0 non vengono inizializzati correttamente in port anche se i PID sono differenti nella inizializzazione di IDPORT. controollare il pid, l'id ecc che sia corretto

- perchè nella nave ci sono valori diversi delle merci ? controllare le life in ship e in port
- non vengono eliminate le memorie condividse della domanda e dell'offerta dei porti, verificare se effettovamente vengono allocate in memoria e nel caso capire se è meglio crearle nel master in modo da eliminarle piò facilmente.
- ricollegandomi al rpoblema precedente forse è meglio fare un handler per ogni processo in modo che ogni processo deallochi in modo indipendente le proprie risorse
- vengono create le merci sempre per lo stesso porto 0
