#ifndef TEST_TI_DRIVERS_ADC_H
#define TEST_TI_DRIVERS_ADC_H

#include <stdbool.h>
#include <stdint.h>

#define ADC_STATUS_SUCCESS 0

struct ADC_Config {
    unsigned int id;
};

typedef struct ADC_Config *ADC_Handle;

typedef struct {
    void *custom;
    bool isProtected;
} ADC_Params;

void ADC_init(void);
void ADC_Params_init(ADC_Params *params);
ADC_Handle ADC_open(uint_least8_t index, ADC_Params *params);
void ADC_close(ADC_Handle handle);
int_fast16_t ADC_convert(ADC_Handle handle, uint16_t *value);
uint32_t ADC_convertRawToMicroVolts(ADC_Handle handle, uint16_t value);

#endif
