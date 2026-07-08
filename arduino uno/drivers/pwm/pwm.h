#ifndef PWM_H
#define PWM_H

#include <stdint.h>
#include "drivers/gpio/gpio.h"

/**
 * @file pwm.h
 * @brief Generic Pulse Width Modulation (PWM) Driver.
 * * Controleaza semnalele PWM pentru pinii suportati.
 * Integreaza Timer0 (D5, D6), Timer1 (D9, D10) si Timer2 (D11, D3).
 */

/**
 * @brief Initializeaza PWM pe un pin specific.
 * * @param port GPIO Port (GPIO_PORTB, GPIO_PORTD).
 * @param pin Pin numar (0-7).
 * @param frequency_hz Frecventa dorita in Hz (pentru Timer1 si Timer2).
 */
void PWM_Init(uint8_t port, uint8_t pin, uint32_t frequency_hz);

/**
 * @brief Seteaza factorul de umplere (duty cycle).
 * * @param port GPIO Port.
 * @param pin Pin numar.
 * @param duty Duty cycle (0-255). 0 = 0%, 255 = 100%.
 */
void PWM_SetDutyCycle(uint8_t port, uint8_t pin, uint8_t duty);

/**
 * @brief Opreste semnalul PWM pe pinul specificat.
 * * @param port GPIO Port.
 * @param pin Pin numar.
 */
void PWM_Stop(uint8_t port, uint8_t pin);

#endif // PWM_H