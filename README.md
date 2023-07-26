# SOproject

progect of operative sistem based on implements a simulation of the sea's tread

cose importanti da vedere:

- verificare la life di una merce che sia minore dei gironi della simulazione almeno può scadere

- perchè nella nave ci sono valori diversi delle merci ? controllare le life in ship e in port
- non vengono eliminate le memorie condividse della domanda e dell'offerta dei porti, verificare se effettovamente vengono allocate in memoria e nel caso capire se è meglio crearle nel master in modo da eliminarle piò facilmente.
- ricollegandomi al poblema precedente forse è meglio fare un handler per ogni processo in modo che ogni processo deallochi in modo indipendente le proprie risorse
- vengono create le merci sempre per lo stesso porto 0

IMPO:
inoltre correggere la ship nella parte di arrivo ad un porto.
