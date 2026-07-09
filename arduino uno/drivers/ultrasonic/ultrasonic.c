#include "ultrasonic.h"
#include "bsp/uno.h"
#include "drivers/gpio/gpio.h"
#include "drivers/timer/timer0.h"
#include "drivers/motor/motor.h"
#include "drivers/buzzer/buzzer.h"
#include <util/delay.h>

// Mapam noile conexiuni (Trigger comun si 3 Ecouri)
#define TRIG_COMUN_PIN UNO_D12
#define ECHO_FATA_PIN  UNO_D13
#define ECHO_SPATE_PIN UNO_A0
#define ECHO_DREAPTA_PIN UNO_A3

// Variabila pentru a nu scana in continuu si a elibera procesorul
static uint32_t last_scan_time = 0;

void ULTRASONIC_Init(void) {
    // Initializam un singur OUTPUT pentru toti senzorii
    GPIO_Init(TRIG_COMUN_PIN, GPIO_OUTPUT);
    
    // Initializam intrarile pentru fiecare senzor in parte
    GPIO_Init(ECHO_FATA_PIN, GPIO_INPUT);
    GPIO_Init(ECHO_SPATE_PIN, GPIO_INPUT);
    GPIO_Init(ECHO_DREAPTA_PIN, GPIO_INPUT);
    
    // Starea de repaus a pinului Trigger este LOW
    GPIO_Write(TRIG_COMUN_PIN, GPIO_LOW);
}

// 0 = Fata, 1 = Spate, 2 = Dreapta
uint16_t ULTRASONIC_GetDistance(uint8_t senzor_id) {
    uint8_t echo_pin_curent;
    
    // Selectam pinul de Echo pe care vrem sa il ascultam
    if (senzor_id == 0) echo_pin_curent = ECHO_FATA_PIN;
    else if (senzor_id == 1) echo_pin_curent = ECHO_SPATE_PIN;
    else if (senzor_id == 2) echo_pin_curent = ECHO_DREAPTA_PIN;
    else return 999; // ID invalid

    // 1. Generam pulsul de 10 microsecunde pe TRIG (toti senzorii "striga" simultan)
    GPIO_Write(TRIG_COMUN_PIN, GPIO_HIGH);
    _delay_us(10);
    GPIO_Write(TRIG_COMUN_PIN, GPIO_LOW);

    // 2. Asteptam inceperea ecoului pe pinul specificat (cu timeout de siguranta)
    uint16_t timeout = 10000;
    while(GPIO_Read(echo_pin_curent) == GPIO_LOW) {
        timeout--;
        if(timeout == 0) return 999; // Senzor neconectat sau eroare
    }

    // 3. Masuram durata pulsului HIGH doar pe pinul selectat
    uint32_t durata = 0;
    timeout = 20000; // Timeout pentru aprox 1.5 metri (suficient pt rover)
    while(GPIO_Read(echo_pin_curent) == GPIO_HIGH) {
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
    if (Millis() - last_scan_time >= 100) {
        last_scan_time = Millis();

        // Pentru sistemul automat de franare (anti-coliziune frontala in mers normal),
        // ne intereseaza exclusiv senzorul din fata (ID 0)
        uint16_t distanta = ULTRASONIC_GetDistance(0);

        // Logica de siguranta
        if (distanta < DISTANTA_MINIMA_CM) {
            // Suprascriem orice comanda si oprim roverul imediat
            MOTOR_Drive(DIR_STOP, 0);
            
            // Declanșăm buzzer-ul (acum pe modul activ)
            BUZZER_Beep(150);
        }
    }
}