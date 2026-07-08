#include "buzzer.h"
#include "bsp/uno.h"
#include "drivers/gpio/gpio.h"
#include "drivers/timer/timer0.h"

// Mapam pinul hardware (exemplu pe D4)
#define BUZZER_PIN UNO_D4

// Variabile de stare interne (ascunse)
static uint8_t buzzer_is_active = 0;
static uint32_t buzzer_start_time = 0;
static uint32_t buzzer_duration = 0;

void BUZZER_Init(void) {
    GPIO_Init(BUZZER_PIN, GPIO_OUTPUT);
    GPIO_Write(BUZZER_PIN, GPIO_LOW); // Asiguram starea initiala inactiva
}

void BUZZER_Beep(uint32_t duration_ms) {
    buzzer_duration = duration_ms;
    buzzer_start_time = Millis();
    buzzer_is_active = 1;
    
    // Pornim hardware-ul
    GPIO_Write(BUZZER_PIN, GPIO_HIGH);
}

void BUZZER_Update(void) {
    // Daca buzzer-ul nu suna, nu avem ce face in update
    if (!buzzer_is_active) {
        return;
    }

    // Verificam daca timpul a expirat
    if ((Millis() - buzzer_start_time) >= buzzer_duration) {
        // Oprim buzzer-ul hardware
        GPIO_Write(BUZZER_PIN, GPIO_LOW);
        
        // Resetam masina de stare
        buzzer_is_active = 0;
    }
}