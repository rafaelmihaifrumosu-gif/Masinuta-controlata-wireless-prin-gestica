#ifndef ESP32_REGISTERS_H
#define ESP32_REGISTERS_H

#include <stdint.h>

// Adrese de bază
#define GPIO_BASE           0x3FF44000
#define IO_MUX_GPIO_BASE    0x3FF49000
#define UART0_BASE          0x3FF40000

// Macro-uri pentru acces direct la regiștri
#define GPIO_ENABLE_REG     (*(volatile uint32_t*)(GPIO_BASE + 0x04))
#define GPIO_OUT_REG        (*(volatile uint32_t*)(GPIO_BASE + 0x00))
#define GPIO_IN_REG         (*(volatile uint32_t*)(GPIO_BASE + 0x3C))

// Acces la IO_MUX (unde se setează funcția pinului și pull-up)
#define IO_MUX_GPIO_REG(pin) (*(volatile uint32_t*)(IO_MUX_GPIO_BASE + (pin * 4)))

// UART Regs
#define UART_FIFO_REG       (*(volatile uint32_t*)(UART0_BASE + 0x00))
#define UART_STATUS_REG     (*(volatile uint32_t*)(UART0_BASE + 0x1C))
#define UART_CLKDIV_REG     (*(volatile uint32_t*)(UART0_BASE + 0x14))
#define UART_CONF0_REG      (*(volatile uint32_t*)(UART0_BASE + 0x20))

#endif