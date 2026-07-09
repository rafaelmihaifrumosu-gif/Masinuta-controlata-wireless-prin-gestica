#ifndef ALARM_H
#define ALARM_H

#include <stdint.h>

// Initializarea variabilelor sistemului de alarma
void ALARM_Init(void);

// Armeaza alarma (modul de veghe)
void ALARM_Arm(void);

// Dezactiveaza alarma si opreste sunetul
void ALARM_Disarm(void);

// Functie non-blocanta care se pune in bucla while(1)
void ALARM_Update(void);

// Returneaza 1 daca alarma suna, 0 daca nu
uint8_t ALARM_IsTriggered(void);

#endif // ALARM_H