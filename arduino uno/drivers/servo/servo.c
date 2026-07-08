#include "servo.h"
#include "drivers/timer/timer1.h"

/*
 * Setări specifice pentru servomotoare standard (50Hz / 20ms)
 * Calcul Timer1 (16MHz): Freq = F_CPU / (Prescaler * (1 + TOP))
 * Cu Prescaler = 64 => TOP = (16000000 / (64 * 50)) - 1 = 4999
 * La Prescaler 64, un "tick" al timerului durează 4 microsecunde.
 */
#define SERVO_TOP 4999

/*
 * Limite pentru lățimea pulsului (0.5ms - 2.4ms pentru SG90)
 * 500us = 500 / 4 = 125 ticks (corespunde la 0 grade)
 * 2400us = 2400 / 4 = 600 ticks (corespunde la 180 grade)
 */
#define SERVO_MIN_TICK 125
#define SERVO_MAX_TICK 600

void SERVO_Init(void) {
    // Inițializăm Timer1 direct cu Prescaler 64 și TOP 4999
    Timer1_FastPWM_Init(64, SERVO_TOP);
}

void SERVO_SetAngle(uint8_t channel, uint8_t angle) {
    // Limităm unghiul strict pentru a proteja angrenajele mecanice
    if (angle > 180) {
        angle = 180;
    }

    // Mapăm unghiul (0-180) la intervalul de tick-uri (125-600)
    // Formula liniară evitând numerele cu virgulă (float)
    uint16_t duty = SERVO_MIN_TICK + ((uint32_t)angle * (SERVO_MAX_TICK - SERVO_MIN_TICK)) / 180;

    // Setăm valoarea pe hardware
    if (channel == SERVO_CH_A) {
        Timer1_SetDutyCycleA(duty);
    } else if (channel == SERVO_CH_B) {
        Timer1_SetDutyCycleB(duty);
    }
}