#include "motor.h"
#include "bsp/uno.h"
#include "drivers/pwm/pwm.h"

// Mapare pini MX1508 (PWM direct pe intrarile de directie)
// Motor Stanga (Modulul 1)
// Motor Stanga (Modulul 1) - Controlat de Timer0
#define MOTOR_ST_FATA_PORT  GPIO_PORTD
#define MOTOR_ST_FATA_PIN   5 // Pinul D5 (IN1+IN3)
#define MOTOR_ST_SPATE_PORT GPIO_PORTD
#define MOTOR_ST_SPATE_PIN  6 // Pinul D6 (IN2+IN4)

// Motor Dreapta (Modulul 2) - Controlat de Timer2
#define MOTOR_DR_FATA_PORT  GPIO_PORTB
#define MOTOR_DR_FATA_PIN   3 // Pinul D11 (IN1+IN3)
#define MOTOR_DR_SPATE_PORT GPIO_PORTD
#define MOTOR_DR_SPATE_PIN  3 // Pinul D3 (IN2+IN4)

void MOTOR_Init(void) {
    // Initializam semnalele PWM la 1kHz (1000 Hz) pentru toti cei 4 pini
    PWM_Init(MOTOR_ST_FATA_PORT, MOTOR_ST_FATA_PIN, 1000);
    PWM_Init(MOTOR_ST_SPATE_PORT, MOTOR_ST_SPATE_PIN, 1000);
    PWM_Init(MOTOR_DR_FATA_PORT, MOTOR_DR_FATA_PIN, 1000);
    PWM_Init(MOTOR_DR_SPATE_PORT, MOTOR_DR_SPATE_PIN, 1000);
    
    // Asiguram ca motoarele sunt oprite la pornire
    MOTOR_Drive(DIR_STOP, 0);
}

void MOTOR_Drive(MotorDirection dir, uint8_t speed_percent) {
    if (speed_percent > 100) speed_percent = 100;
    
    // Mapam procentul (0-100) la valoarea PWM (0-255)
    uint8_t pwm_val = (uint8_t)(((uint16_t)speed_percent * 255) / 100);

    // Resetam intai toate semnalele la 0 pentru a preveni scurtcircuite logice pe puntea H
    PWM_SetDutyCycle(MOTOR_ST_FATA_PORT, MOTOR_ST_FATA_PIN, 0);
    PWM_SetDutyCycle(MOTOR_ST_SPATE_PORT, MOTOR_ST_SPATE_PIN, 0);
    PWM_SetDutyCycle(MOTOR_DR_FATA_PORT, MOTOR_DR_FATA_PIN, 0);
    PWM_SetDutyCycle(MOTOR_DR_SPATE_PORT, MOTOR_DR_SPATE_PIN, 0);

    // Aplicam PWM doar pe pinii corespunzatori directiei dorite
    switch (dir) {
        case DIR_FATA:
            PWM_SetDutyCycle(MOTOR_ST_FATA_PORT, MOTOR_ST_FATA_PIN, pwm_val);
            PWM_SetDutyCycle(MOTOR_DR_FATA_PORT, MOTOR_DR_FATA_PIN, pwm_val);
            break;
            
        case DIR_SPATE:
            PWM_SetDutyCycle(MOTOR_ST_SPATE_PORT, MOTOR_ST_SPATE_PIN, pwm_val);
            PWM_SetDutyCycle(MOTOR_DR_SPATE_PORT, MOTOR_DR_SPATE_PIN, pwm_val);
            break;
            
        case DIR_STANGA:
            // Rotire pe loc: Stanga da inapoi, Dreapta da inainte
            PWM_SetDutyCycle(MOTOR_ST_SPATE_PORT, MOTOR_ST_SPATE_PIN, pwm_val);
            PWM_SetDutyCycle(MOTOR_DR_FATA_PORT, MOTOR_DR_FATA_PIN, pwm_val);
            break;
            
        case DIR_DREAPTA:
            // Rotire pe loc: Stanga da inainte, Dreapta da inapoi
            PWM_SetDutyCycle(MOTOR_ST_FATA_PORT, MOTOR_ST_FATA_PIN, pwm_val);
            PWM_SetDutyCycle(MOTOR_DR_SPATE_PORT, MOTOR_DR_SPATE_PIN, pwm_val);
            break;
            
        case DIR_STOP:
        default:
            // Deja setate la 0 mai sus
            break;
    }
}