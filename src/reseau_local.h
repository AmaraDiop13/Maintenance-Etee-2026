#ifndef RESEAU_LOCAL_H
#define RESEAU_LOCAL_H

#include <WebServer.h>

extern WebServer server;
extern bool sdDisponible; // Statut matériel partagé de la carte mémoire

void initialiserReseaux();
void handleRoot();
void handleData();

#endif