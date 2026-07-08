#ifndef SERVO_H
#define SERVO_H

#include <stdint.h>

// Definim canalele hardware (bazate pe Timer1)
#define SERVO_CH_A 0 // Mapat pe D9 (PB1)
#define SERVO_CH_B 1 // Mapat pe D10 (PB2)

/**
 * @brief Inițializează hardware-ul pentru controlul servomotoarelor.
 * Folosește Timer1 pentru a genera un semnal PWM hardware de 50Hz.
 */
void SERVO_Init(void);

/**
 * @brief Setează unghiul servomotorului.
 * * @param channel Canalul dorit (SERVO_CH_A sau SERVO_CH_B).
 * @param angle Unghiul dorit în grade (0 - 180).
 */
void SERVO_SetAngle(uint8_t channel, uint8_t angle);

#endif // SERVO_H