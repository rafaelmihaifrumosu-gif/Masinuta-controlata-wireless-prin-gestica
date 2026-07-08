#include "delay.h"

// Funcție de delay "busy-wait" simplă
// Aproximativ 1ms per iterație (ajustabil în funcție de frecvența CPU)
void Delay(uint32_t ms) {
    for (uint32_t i = 0; i < ms * 10000; i++) {
        __asm__ __volatile__ ("nop"); // Instrucțiune vidă pentru a nu fi optimizată
    }
}