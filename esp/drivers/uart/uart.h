#ifndef UART_H
#define UART_H

#include <stdint.h>

/**
 * @file uart.h
 * @brief Driver UART Bare-Metal pentru ESP32.
 * * Oferă funcții de inițializare, transmisie și recepție 
 * prin perifericul hardware UART0.
 */

/**
 * @brief Inițializează comunicația UART cu rata de baud specificată.
 * * @param baud Rata de transmisie dorită (ex: 9600).
 */
void UART_Init(uint32_t baud);

/**
 * @brief Transmite un singur caracter prin UART.
 * * @param c Caracterul care trebuie transmis.
 */
void UART_Transmit(char c);

/**
 * @brief Primește un singur caracter din coada FIFO.
 * * @return Caracterul primit sau 0 dacă buffer-ul este gol.
 */
char UART_Receive(void);

#endif // UART_H