#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>

// Definim directiile de deplasare posibile
typedef enum {
    DIR_STOP = 0,
    DIR_FATA,
    DIR_SPATE,
    DIR_STANGA,
    DIR_DREAPTA
} MotorDirection;

/**
 * @brief Initializeaza pinii de directie si semnalele PWM pentru motoare.
 */
void MOTOR_Init(void);

/**
 * @brief Controleaza deplasarea roverului.
 * * @param dir Directia dorita.
 * @param speed_percent Viteza in procente (0 - 100).
 */
void MOTOR_Drive(MotorDirection dir, uint8_t speed_percent);

#endif // MOTOR_H