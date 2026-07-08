#include "timer0.h"

// Contorul sistemului, incrementat la fiecare ~1ms
static volatile uint32_t system_millis = 0;

void Timer0_Init(void) {
    // 1. Setam modul Fast PWM (Mode 3: WGM01 = 1, WGM00 = 1)
    TCCR0A |= (1 << WGM01) | (1 << WGM00);
    
    // 2. Setam Prescaler-ul la 64 (CS01 = 1, CS00 = 1)
    // Frecventa rezultata: 16MHz / (64 * 256) = 976.56 Hz
    TCCR0B |= (1 << CS01) | (1 << CS00);
    
    // 3. Activam intreruperea la Overflow (TOIE0)
    TIMSK0 |= (1 << TOIE0);
    
    // 4. Activam intreruperile globale
    sei();
}

// Intreruperea vectorului de Overflow
ISR(TIMER0_OVF_vect) {
    system_millis++;
}

uint32_t Millis(void) {
    uint32_t m;
    uint8_t oldSREG = SREG;
    
    // Citire atomica pentru siguranta datelor
    cli();
    m = system_millis;
    SREG = oldSREG;
    
    return m;
}