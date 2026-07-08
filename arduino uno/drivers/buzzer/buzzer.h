#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>

/**
 * @brief Initializeaza hardware-ul pentru buzzer.
 */
void BUZZER_Init(void);

/**
 * @brief Porneste buzzer-ul pentru o anumita durata, fara a bloca executia.
 * * @param duration_ms Durata cat buzzerul va suna, in milisecunde.
 */
void BUZZER_Beep(uint32_t duration_ms);

/**
 * @brief Functie ce trebuie apelata permanent in bucla while(1) din main.
 * Gestioneaza oprirea automata a buzzer-ului la expirarea timpului.
 */
void BUZZER_Update(void);

#endif // BUZZER_H