#include "uart.h"
#include "esp32_registers.h"

/**
 * @brief Inițializează UART0 în mod bare-metal.
 * Configurările sunt făcute direct în registrele hardware ale ESP32.
 */
void UART_Init(uint32_t baud) {
    // 1. Calculăm divizorul de clock pentru frecvența APB_CLK de 80MHz
    // Formula standard din Technical Reference Manual: (APB_CLK / baud) / 16
    uint32_t clk_div = (80000000 / (baud * 16));
    UART_CLKDIV_REG = clk_div;

    // 2. Configurăm formatul cadrului (reg. CONF0)
    // - 8 biți de date (valoarea 3 în biții 8-9)
    // - Fără paritate
    // - 1 bit de stop
    UART_CONF0_REG = (3 << 8); 

    // 3. Resetăm FIFO (obligatoriu după configurare)
    // Scriem 3 în registrul FIFO_CONF pentru a reseta ambele FIFO-uri (TX și RX)
    *(volatile uint32_t*)(UART0_BASE + 0x28) = 3; 
    *(volatile uint32_t*)(UART0_BASE + 0x28) = 0;
}

/**
 * @brief Transmite un caracter prin coada FIFO.
 */
void UART_Transmit(char c) {
    // Verificăm statusul TX FIFO (TXFIFO_CNT este în primii 8 biți ai registrului de status)
    // Așteptăm cât timp coada are mai mult de 120 de octeți ocupați (buffer aproape plin)
    while ((UART_STATUS_REG & 0xFF) > 120); 
    
    // Scriem caracterul direct în registrul FIFO
    UART_FIFO_REG = c;
}

/**
 * @brief Primește un caracter (non-blocant).
 */
char UART_Receive(void) {
    // RxFIFO_CNT se află în biții 16-23 ai registrului de status
    uint32_t rx_fifo_count = (UART_STATUS_REG >> 16) & 0xFF;
    
    if (rx_fifo_count > 0) {
        // Citim caracterul din registrul FIFO
        return (char)(UART_FIFO_REG & 0xFF);
    }
    
    return 0; // Returnăm 0 dacă nu avem date noi
}