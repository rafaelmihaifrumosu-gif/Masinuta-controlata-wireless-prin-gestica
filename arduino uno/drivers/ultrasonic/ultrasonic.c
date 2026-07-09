#include "ultrasonic.h"
#include "bsp/uno.h"
#include "drivers/gpio/gpio.h"
#include "drivers/timer/timer0.h"
#include "drivers/motor/motor.h"
#include "drivers/buzzer/buzzer.h"
#include <util/delay.h>

// Mapam conexiunile hardware
#define TRIG_COMUN_PIN UNO_D12
#define ECHO_FATA_PIN  UNO_D13
#define ECHO_SPATE_PIN UNO_A0
#define ECHO_DREAPTA_PIN UNO_A3

static uint32_t last_scan_time = 0;

void ULTRASONIC_Init(void) {
    GPIO_Init(TRIG_COMUN_PIN, GPIO_OUTPUT);
    
    GPIO_Init(ECHO_FATA_PIN, GPIO_INPUT);
    GPIO_Init(ECHO_SPATE_PIN, GPIO_INPUT);
    GPIO_Init(ECHO_DREAPTA_PIN, GPIO_INPUT);
    
    GPIO_Write(TRIG_COMUN_PIN, GPIO_LOW);
}

// Functie auxiliara interna care primeste exact Portul si Pinul dorit
static uint16_t masoara_ecou(gpio_port_t port, uint8_t pin) {
    uint16_t timeout = 10000;
    
    // 1. Asteptam inceperea ecoului
    while(GPIO_Read(port, pin) == GPIO_LOW) {
        timeout--;
        if(timeout == 0) return 999; // Senzor neconectat
    }

    // 2. Masuram durata pulsului HIGH
    uint32_t durata = 0;
    timeout = 20000; 
    while(GPIO_Read(port, pin) == GPIO_HIGH) {
        durata++;
        timeout--;
        if(timeout == 0) return 999; // Nimic in raza
        _delay_us(1); 
    }

    // Transformam in centimetri
    return (uint16_t)(durata / 58);
}

uint16_t ULTRASONIC_GetDistance(uint8_t senzor_id) {
    // 1. Toti senzorii primesc trigger-ul simultan
    GPIO_Write(TRIG_COMUN_PIN, GPIO_HIGH);
    _delay_us(10);
    GPIO_Write(TRIG_COMUN_PIN, GPIO_LOW);

    // 2. Apelam functia auxiliara. Aici macro-urile se desfac automat!
    // Ex: masoara_ecou(UNO_D13) devine masoara_ecou(GPIO_PORTB, 5)
    if (senzor_id == 0) {
        return masoara_ecou(ECHO_FATA_PIN);
    } else if (senzor_id == 1) {
        return masoara_ecou(ECHO_SPATE_PIN);
    } else if (senzor_id == 2) {
        return masoara_ecou(ECHO_DREAPTA_PIN);
    }

    return 999; // ID invalid
}

void ULTRASONIC_Update(void) {
    // Verificam obstacolele o data la 100ms doar pentru partea din fata
    if (Millis() - last_scan_time >= 100) {
        last_scan_time = Millis();

        uint16_t distanta = ULTRASONIC_GetDistance(0);

        // Protectie coliziune (DISTANTA_MINIMA_CM trebuie sa fie definita in ultrasonic.h)
        if (distanta < DISTANTA_MINIMA_CM) {
            MOTOR_Drive(DIR_STOP, 0);
            BUZZER_Beep(150);
        }
    }
}