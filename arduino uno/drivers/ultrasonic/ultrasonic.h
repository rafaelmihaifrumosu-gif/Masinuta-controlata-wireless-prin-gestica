#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include <stdint.h>

// Definim distanta minima de siguranta (in centimetri)
#define DISTANTA_MINIMA_CM 15

/**
 * @brief Initializeaza pinii pentru senzorul ultrasonic HC-SR04.
 */
void ULTRASONIC_Init(void);

/**
 * @brief Functie care trimite un puls si masoara distanta.
 * Contine timeout-uri stricte pentru a nu bloca sistemul.
 * @return Distanta in centimetri (sau 999 in caz de eroare/out of range).
 */
uint16_t ULTRASONIC_GetDistance(void);

/**
 * @brief Functie care trebuie apelata in bucla while(1) din main.
 * Verifica distanta periodic si opreste roverul daca e necesar.
 */
void ULTRASONIC_Update(void);

#endif // ULTRASONIC_HS