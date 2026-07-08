#include "ultrasonic.h"
#include "bsp/uno.h"
#include "drivers/gpio/gpio.h"
#include "drivers/timer/timer0.h"
#include "drivers/motor/motor.h"
#include "drivers/buzzer/buzzer.h"
#include <util/delay.h>

// Mapam pinii senzorului (alege pinii disponibili pe Arduino)
#define TRIG_PIN UNO_A1
#define ECHO_PIN UNO_A2

// Variabila pentru a nu scana in continuu si a elibera procesorul
static uint32_t last_scan_time = 0;

void ULTRASONIC_Init(void) {
    GPIO_Init(TRIG_PIN, GPIO_OUTPUT);
    GPIO_Init(ECHO_PIN, GPIO_INPUT);
    
    // Starea de repaus a pinului Trigger este LOW
    GPIO_Write(TRIG_PIN, GPIO_LOW);
}

uint16_t ULTRASONIC_GetDistance(void) {
    // 1. Generam pulsul de 10 microsecunde pe TRIG
    GPIO_Write(TRIG_PIN, GPIO_HIGH);
    _delay_us(10);
    GPIO_Write(TRIG_PIN, GPIO_LOW);

    // 2. Asteptam inceperea ecoului (cu timeout de siguranta)
    uint16_t timeout = 10000;
    while(GPIO_Read(ECHO_PIN) == GPIO_LOW) {
        timeout--;
        if(timeout == 0) return 999; // Senzor neconectat sau eroare
    }

    // 3. Masuram durata pulsului HIGH
    uint32_t durata = 0;
    timeout = 20000; // Timeout pentru aprox 1.5 metri (suficient pt rover)
    while(GPIO_Read(ECHO_PIN) == GPIO_HIGH) {
        durata++;
        timeout--;
        if(timeout == 0) return 999; // Nimic in raza de actiune
        _delay_us(1); // Incrementam durata cu ~1 us
    }

    // 4. Transformam timpul in distanta
    // Formula: Distanta = (Timp / 2) / 29.1
    // Aproximativ: Durata in microsecunde / 58
    return (uint16_t)(durata / 58);
}

void ULTRASONIC_Update(void) {
    // Verificam senzorul doar o data la 100 milisecunde (10 Hz)
    // Asta lasa timp procesorului sa citeasca date pe I2C intre timp
    if (Millis() - last_scan_time >= 100) {
        last_scan_time = Millis();

        uint16_t distanta = ULTRASONIC_GetDistance();

        // Logica de siguranta
        if (distanta < DISTANTA_MINIMA_CM) {
            // Suprascriem orice comanda si oprim roverul imediat
            MOTOR_Drive(DIR_STOP, 0);
            
            // Declanşam buzzer-ul non-blocant sa sune scurt (ex: 150ms)
            BUZZER_Beep(150);
        }
    }
}