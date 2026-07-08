#include "gpio.h"
#include "esp32_registers.h"
#include "utils.h"

void GPIO_Init(uint8_t pin, gpio_dir_t dir) {
    if (pin > 39) return;

    // Setează funcția pinului pe GPIO (3 << 8)
    IO_MUX_GPIO_REG(pin) = (3 << 8);

    if (dir == GPIO_OUTPUT) {
        SET_BIT(GPIO_ENABLE_REG, pin);
    } else {
        CLEAR_BIT(GPIO_ENABLE_REG, pin);
    }
}

void GPIO_Write(uint8_t pin, gpio_state_t state) {
    if (pin > 39) return;
    if (state == GPIO_HIGH) {
        SET_BIT(GPIO_OUT_REG, pin);
    } else {
        CLEAR_BIT(GPIO_OUT_REG, pin);
    }
}

gpio_state_t GPIO_Read(uint8_t pin) {
    return (CHECK_BIT(GPIO_IN_REG, pin)) ? GPIO_HIGH : GPIO_LOW;
}