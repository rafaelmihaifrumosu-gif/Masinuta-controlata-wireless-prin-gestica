#include "buzzer.h"
#include "bsp/uno.h"
#include "drivers/gpio/gpio.h"
#include "drivers/timer/timer0.h"

// Mapam pinul hardware (verifică să fie conectat corect la D4 pe șasiu)
#define BUZZER_PIN UNO_D2

// Variabile de stare interne (ascunse)
static uint8_t buzzer_is_active = 0;
static uint32_t buzzer_start_time = 0;
static uint32_t buzzer_duration = 0;

void BUZZER_Init(void) {
    GPIO_Init(BUZZER_PIN, GPIO_OUTPUT);
    GPIO_Write(BUZZER_PIN, GPIO_LOW); // Asiguram starea initiala inactiva (oprit)
}

void BUZZER_Beep(uint32_t duration_ms) {
    // 1. Pornim hardware-ul direct (Buzzerul activ suna la curent continuu)
    GPIO_Write(BUZZER_PIN, GPIO_HIGH);

    // 2. Setam masina de stare pentru a sti cand sa il oprim in fundal
    buzzer_is_active = 1;
    buzzer_start_time = Millis();
    buzzer_duration = duration_ms;
}

void BUZZER_Update(void) {
    // Daca buzzer-ul nu suna, functia nu consuma timp de procesare
    if (!buzzer_is_active) {
        return;
    }

    // Verificam daca timpul setat anterior a expirat
    if ((Millis() - buzzer_start_time) >= buzzer_duration) {

        // Taiem curentul de pe pin
        GPIO_Write(BUZZER_PIN, GPIO_LOW);

        // Resetam masina de stare
        buzzer_is_active = 0;
    }
}