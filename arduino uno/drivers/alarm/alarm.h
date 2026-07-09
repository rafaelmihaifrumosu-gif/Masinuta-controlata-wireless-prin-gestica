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

void ALARM_Disarm(void) {
    stare_alarma = ALARM_OFF;
    buzzer_state = 0;
    GPIO_Write(UNO_D3, GPIO_LOW); 
}
#endif // ALARM_H
