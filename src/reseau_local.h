#ifndef RESEAU_LOCAL_H
#define RESEAU_LOCAL_H

#include <WebServer.h>

extern WebServer server;

void initialiserReseaux();
void handleRoot();
void handleData();

#endif