#include "i2c_master.h"
#include <avr/io.h>
#include "bsp/uno.h"            // Aducem harta pinilor (UNO_A4, UNO_A5)
#include "drivers/gpio/gpio.h"  // Folosim driverul nostru uniform

void i2c_master_init(void) {
    // 1. DEZACTIVAM pull-up-urile interne de 5V fortand pinii pe LOW!
    // Lasam magistrala sa fie ridicata la 3.3V exclusiv de rezistentele fizice externe.
    GPIO_Init(UNO_A4, GPIO_INPUT);
    GPIO_Write(UNO_A4, GPIO_LOW);
    
    GPIO_Init(UNO_A5, GPIO_INPUT);
    GPIO_Write(UNO_A5, GPIO_LOW);

    // 2. Configurarea ratei de transfer (Baud Rate)
    TWBR = (uint8_t)((((F_CPU / F_SCL) / PRESCALER_1) - 16) / 2);
    TWSR &= ~((1 << TWPS1) | (1 << TWPS0)); // Prescaler = 1
}

// Functie interna izolata (static) pentru asteptarea flag-ului hardware
static uint8_t i2c_wait_ready(void) {
    uint16_t timeout = 10000; 
    while (!(TWCR & (1 << TWINT))) {
        timeout--;
        if (timeout == 0) {
            return 0; // Eroare critica (Timeout)
        }
    }
    return 1; // Flag-ul a fost ridicat (Succes)
}

uint8_t i2c_master_start(void) {
    // Trimitem conditia de START pe magistrala
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    
    // Asteptam terminarea transmisiei
    if (!i2c_wait_ready()) return 0;
    
    // VERIFICARE STRICTA: Citim registrii hardware pentru a vedea daca START-ul s-a trimis
    uint8_t status = TWSR & 0xF8;
    if (status != 0x08 && status != 0x10) {
        return 0; // 0x08 = START, 0x10 = Repeta START. Altceva inseamna eroare pe fir.
    }
    
    return 1;
}

void i2c_master_stop(void) {
    // Conditia de STOP nu asteapta TWINT, hardware-ul o executa si elibereaza magistrala
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}

uint8_t i2c_master_write(uint8_t data) {
    // Incarcam datele in registru si declansam transmisia
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);
    
    // Asteptam terminarea transmisiei
    if (!i2c_wait_ready()) return 0;
    
    // VERIFICARE STRICTA: Verificam daca Slave-ul a confirmat primirea
    uint8_t status = TWSR & 0xF8;
    
    // 0x18 = S-a trimis o adresa + Write, si am primit ACK
    // 0x28 = S-a trimis un pachet de date, si am primit ACK
    if (status != 0x18 && status != 0x28) {
        return 0; // Slave-ul nu a raspuns (NACK). Anulam transmisia!
    }
    
    return 1;
}