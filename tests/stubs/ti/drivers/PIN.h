#ifndef TEST_TI_DRIVERS_PIN_H
#define TEST_TI_DRIVERS_PIN_H

#include <stdint.h>

typedef uint32_t PIN_Config;

typedef struct PIN_State_s {
    unsigned int unused;
} PIN_State;

typedef PIN_State *PIN_Handle;

#define PIN_INPUT_DIS      (1UL << 8)
#define PIN_NOPULL         (1UL << 9)
#define PIN_IRQ_DIS        (1UL << 10)
#define PIN_GPIO_OUTPUT_EN (1UL << 11)
#define PIN_GPIO_LOW       (1UL << 12)
#define PIN_PUSHPULL       (1UL << 13)
#define PIN_DRVSTR_MIN     (1UL << 14)
#define PIN_TERMINATE      0U

PIN_Handle PIN_open(PIN_State *state, const PIN_Config pin_list[]);

#endif
