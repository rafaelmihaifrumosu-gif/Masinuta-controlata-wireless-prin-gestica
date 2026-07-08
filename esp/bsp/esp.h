#ifndef ESP32_BSP_H
#define ESP32_BSP_H

#include "driver/gpio.h"
#include "hal/gpio_types.h"
#include "driver/i2c.h"
#include "driver/uart.h"

// ========================================================================
// ESP32 Board Support Package (Master Node)
// Maparea pinilor fizici pentru aplicația de recunoaștere a gesturilor
// ========================================================================

// --- Magistrala I2C (Comunicarea cu Slave-ul Nucleo F401RE) ---
// Pe majoritatea plăcilor ESP32, pinii 21 și 22 sunt rulați implicit pentru I2C
#define I2C_MASTER_SDA_PIN  GPIO_NUM_21
#define I2C_MASTER_SCL_PIN  GPIO_NUM_22
#define I2C_MASTER_PORT_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ  100000     // Standard mode (100 kHz)

// --- Magistrala UART (Comunicarea USB cu scriptul Python) ---
// ESP32 folosește UART0 intern pentru a comunica prin portul USB nativ
#define UART_PC_PORT_NUM    UART_NUM_0
#define UART_PC_TX_PIN      GPIO_NUM_1
#define UART_PC_RX_PIN      GPIO_NUM_3

// --- Pini de semnalizare și Debug (LED-urile de direcție) ---
#define PIN_DIR_FWD         GPIO_NUM_16
#define PIN_DIR_BWD         GPIO_NUM_17
#define PIN_DIR_LEFT        GPIO_NUM_5
#define PIN_DIR_RIGHT       GPIO_NUM_18

// --- Aliasuri standard ---
#define LED_BUILTIN         GPIO_NUM_2 // LED-ul albastru integrat pe placă

#endif // ESP32_BSP_H