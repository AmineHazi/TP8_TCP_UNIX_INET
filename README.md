# TP N°8 Socket INET connecté

Ce petit programme est une application légère de communication en C utilisant des sockets INET pour établir une communication bidirectionnelle entre un serveur et un client. Le serveur traite chaque client dans un thread distinct, permettant une gestion efficace de multiples clients simultanément, et sauvegarde les interactions dans des logs.
Commencer

Ces instructions vous aideront à mettre en place une copie du projet en cours d'exécution sur votre système local à des fins de développement et de test.

## Prérequis
Pour compiler et exécuter ce projet, vous aurez besoin de :

- GCC (GNU Compiler Collection)
- GNU Make

## Exécution

## Pour exécuter le programme, il faut le compiler puis lancer le serveur et le client avec les commandes suivantes :

```bash
make
./bin/server 
./bin/client 
```

### Arrêter les clients et le serveur

Pour arreter le serveur il suffit d'envoyer un signal de terminaison le servuer gere bien les signaux de terminaison.

### Supprimer les logs et les fichiers temporaires

Pour supprimer les logs et les fichiers temporaires, il y a les règles clean et cleanlogs dans le Makefile. Pour les utiliser, on utilise les commandes suivantes :


```bash
make clean
make cleanlogs
```
