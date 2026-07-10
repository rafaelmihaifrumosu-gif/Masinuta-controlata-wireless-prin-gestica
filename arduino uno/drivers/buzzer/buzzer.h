#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>


 

void BUZZER_Init(void);


 

void BUZZER_Beep(uint32_t duration_ms);

/**
 
@brief Functie ce trebuie apelata permanent in bucla while(1) din main.
Gestioneaza oprirea automata a buzzer-ului la expirarea timpului.*/
void BUZZER_Update(void);

#endif // BUZZER_H