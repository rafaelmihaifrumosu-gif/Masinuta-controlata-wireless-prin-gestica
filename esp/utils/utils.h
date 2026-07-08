#ifndef UTILS_H
#define UTILS_H

#define BIT(bit) (1UL << (bit))
#define SET_BIT(reg, bit) ((reg) |= BIT(bit))
#define CLEAR_BIT(reg, bit) ((reg) &= ~BIT(bit))
#define CHECK_BIT(reg, bit) ((reg) & BIT(bit))

#endif