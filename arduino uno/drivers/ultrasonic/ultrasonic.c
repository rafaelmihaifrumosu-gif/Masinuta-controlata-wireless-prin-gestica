#include "ultrasonic.h"
#include "bsp/uno.h"
#include "drivers/gpio/gpio.h"
#include "drivers/timer/timer0.h"
#include "drivers/motor/motor.h"
#include "drivers/buzzer/buzzer.h"
#include <util/delay.h>

// Mapam conexiunile hardware
#define TRIG_COMUN_PIN UNO_A1
#define ECHO_FATA_PIN  UNO_A2
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
// Functie auxiliara interna care primeste exact Portul si Pinul dorit
static uint16_t masoara_ecou(gpio_port_t port, uint8_t pin) {
    uint32_t timeout = 30000; 

    // 1. Asteptam inceperea ecoului
    while(GPIO_Read(port, pin) == GPIO_LOW) {
        timeout--;
        if(timeout == 0) return 999; 
        _delay_us(1); 
    }

    // 2. Masuram durata pulsului HIGH
    uint32_t durata = 0;
    timeout = 30000; 
    while(GPIO_Read(port, pin) == GPIO_HIGH) {
        durata++;
        timeout--;
        if(timeout == 0) return 999; 
        _delay_us(1); 
    }

    // Transformam in centimetri
    uint16_t distanta_cm = (uint16_t)(durata / 58);

    // --- NOU: FILTRU PENTRU CAROSERIE (DEADZONE) ---
    // Daca distanta masurata e intre 1 si 4 cm, este clar marginea sasiului.
    // O ignoram si returnam 999 (drum liber).
    if (distanta_cm > 0 && distanta_cm <= 4) {
        return 999;
    }

    return distanta_cm;
}

uint16_t ULTRASONIC_GetDistance(uint8_t senzor_id) {
    // 1. Ne asiguram ca trigger-ul este oprit inainte de a trage
    GPIO_Write(TRIG_COMUN_PIN, GPIO_LOW);
    _delay_us(2);

    // 2. Tragem un puls scurt de 10us pentru TOȚI senzorii simultan
    GPIO_Write(TRIG_COMUN_PIN, GPIO_HIGH);
    _delay_us(10);
    GPIO_Write(TRIG_COMUN_PIN, GPIO_LOW);

    uint16_t distanta = 999;

    // 3. Citim doar ecoul senzorului de care avem nevoie acum
    if (senzor_id == 0) {
        distanta = masoara_ecou(ECHO_FATA_PIN);
    } else if (senzor_id == 1) {
        distanta = masoara_ecou(ECHO_SPATE_PIN);
    } else if (senzor_id == 2) {
        distanta = masoara_ecou(ECHO_DREAPTA_PIN);
    }

    // 4.  MAGIC FIX: PERIOADA DE LINIȘTE 
    // Asteptam 35 de milisecunde pentru ca toate undele sonore sa fie
    // absorbite de pereti inainte ca urmatoarea functie sa traga iar.
    _delay_ms(35);

    return distanta;
}

void ULTRASONIC_Update(void) {
    static uint32_t last_scan_time = 0;
    static uint32_t last_beep_time = 0;

    if (Millis() - last_scan_time >= 100) {
        last_scan_time = Millis();
        uint16_t distanta = ULTRASONIC_GetDistance(0);

        // DOAR DACA e foarte, foarte aproape (sub 7 cm) si evitam erorile (0)
        if (distanta > 2 && distanta <= 7) { 
            MOTOR_Drive(DIR_STOP, 0); // Siguranta bruta
            
            if (Millis() - last_beep_time >= 1000) {
                BUZZER_Beep(50); // Un singur ticăit la o secunda
                last_beep_time = Millis();
            }
        }
    }
}
