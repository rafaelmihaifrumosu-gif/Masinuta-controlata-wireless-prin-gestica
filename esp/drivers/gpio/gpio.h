#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

typedef enum {
    GPIO_INPUT,
    GPIO_OUTPUT
} gpio_dir_t;

typedef enum {
    GPIO_LOW,
    GPIO_HIGH
} gpio_state_t;

void GPIO_Init(uint8_t pin, gpio_dir_t dir);
void GPIO_Write(uint8_t pin, gpio_state_t state);
gpio_state_t GPIO_Read(uint8_t pin);
void GPIO_Toggle(uint8_t pin);

#endif