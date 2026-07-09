#ifndef MODBUS_ADAM_H
#define MODBUS_ADAM_H

#include <Arduino.h>

// Partage des variables globales de mesure
extern uint16_t valeursBrutesADAM[8];
extern float valeursConverties[8];
extern int estVide[8];
extern bool adamEnLigne;
extern String unitesCanaux[8];
extern String typesEntreesDetectes[8]; 

void scannerConfigurationADAM();
void requeteLectureADAM();

#endif