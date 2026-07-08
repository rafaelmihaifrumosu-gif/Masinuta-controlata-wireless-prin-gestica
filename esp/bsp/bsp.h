#ifndef BSP_H
#define BSP_H

// Logica de selecție a plăcii pentru sistemul Master-Slave
#if defined(BOARD_ESP32_MASTER)
    #include "esp32_bsp.h"
#elif defined(BOARD_STM32_SLAVE)
    #include "stm32_f401re_bsp.h"
#else
    #error "Nicio placă definită! Te rog să definești BOARD_ESP32_MASTER sau BOARD_STM32_SLAVE."
#endif

#endif // BSP_H
