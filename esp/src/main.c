#include "gpio.h"
#include "uart.h"
#include "delay.h"

// Definire pini pentru mașinuță (conform cerințelor tale hardware)
#define PIN_INAINTE  16
#define PIN_INAPOI   17
#define PIN_STANGA   5
#define PIN_DREAPTA  18

/**
 * @brief Inițializarea perifericelor de bază
 */
void System_Init(void) {
    // 1. Inițializăm UART pentru comunicația cu Python (9600 baud)
    UART_Init(9600);
    
    // 2. Inițializăm pinii de control ca ieșiri
    GPIO_Init(PIN_INAINTE, GPIO_OUTPUT);
    GPIO_Init(PIN_INAPOI,  GPIO_OUTPUT);
    GPIO_Init(PIN_STANGA,  GPIO_OUTPUT);
    GPIO_Init(PIN_DREAPTA, GPIO_OUTPUT);
}

/**
 * @brief Funcția principală
 */
int main(void) {
    System_Init();
    
    char comanda = 0;
    
    // Bucla infinită (Infinite Loop - standardul bare-metal)
    while(1) {
        // Citim non-blocant din UART
        comanda = UART_Receive();
        
        if (comanda != 0) { // Dacă am primit un caracter valid de la scriptul Python
            switch(comanda) {
                case 'F': // Forward
                    GPIO_Write(PIN_INAINTE, GPIO_HIGH);
                    GPIO_Write(PIN_INAPOI,  GPIO_LOW);
                    break;
                case 'B': // Backward
                    GPIO_Write(PIN_INAINTE, GPIO_LOW);
                    GPIO_Write(PIN_INAPOI,  GPIO_HIGH);
                    break;
                case 'L': // Left
                    GPIO_Write(PIN_STANGA,  GPIO_HIGH);
                    GPIO_Write(PIN_DREAPTA, GPIO_LOW);
                    break;
                case 'R': // Right
                    GPIO_Write(PIN_STANGA,  GPIO_LOW);
                    GPIO_Write(PIN_DREAPTA, GPIO_HIGH);
                    break;
                case 'S': // Stop (Toate oprite)
                    GPIO_Write(PIN_INAINTE, GPIO_LOW);
                    GPIO_Write(PIN_INAPOI,  GPIO_LOW);
                    GPIO_Write(PIN_STANGA,  GPIO_LOW);
                    GPIO_Write(PIN_DREAPTA, GPIO_LOW);
                    break;
            }
        }
        
        // Pauză scurtă pentru a nu bloca procesorul în totalitate
        Delay(10); 
    }
    
    return 0; // Nu va fi atins niciodată
}