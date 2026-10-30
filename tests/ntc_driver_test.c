#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Board.h"
#include "ntc.h"
#include <ti/drivers/ADC.h>
#include <ti/drivers/PIN.h>

static struct ADC_Config adc_handles[] = {{Board_ADC_NTC}, {Board_ADC_VDDS}};
static unsigned int adc_open_count;
static unsigned int adc_close_count;
static unsigned int adc_init_count;
static unsigned int pin_open_count;
static uint_least8_t failed_open_index = UINT_LEAST8_MAX;
static int pin_open_fails;
static int supply_convert_status;
static int ntc_convert_status;
static uint32_t supply_uv;
static uint32_t ntc_uv;
static uint32_t supply_uv_sequence[32];
static uint32_t ntc_uv_sequence[32];
static unsigned int sequence_length;
static unsigned int supply_convert_count;
static unsigned int ntc_convert_count;
static PIN_Config requested_pin_config;

static void require(int condition, const char *message)
{
    if (!condition)
    {
        (void)fprintf(stderr, "%s\n", message);
        exit(EXIT_FAILURE);
    }
}

static void reset_mock(void)
{
    adc_open_count = 0U;
    adc_close_count = 0U;
    adc_init_count = 0U;
    pin_open_count = 0U;
    failed_open_index = UINT_LEAST8_MAX;
    pin_open_fails = 0;
    supply_convert_status = ADC_STATUS_SUCCESS;
    ntc_convert_status = ADC_STATUS_SUCCESS;
    supply_uv = 3300000U;
    ntc_uv = 1650000U;
    sequence_length = 0U;
    supply_convert_count = 0U;
    ntc_convert_count = 0U;
    requested_pin_config = 0U;
}

void ADC_init(void)
{
    adc_init_count++;
}

void ADC_Params_init(ADC_Params *params)
{
    params->custom = NULL;
    params->isProtected = true;
}

ADC_Handle ADC_open(uint_least8_t index, ADC_Params *params)
{
    (void)params;
    adc_open_count++;
    if (index == failed_open_index)
    {
        return NULL;
    }
    return &adc_handles[index];
}

void ADC_close(ADC_Handle handle)
{
    (void)handle;
    adc_close_count++;
}

int_fast16_t ADC_convert(ADC_Handle handle, uint16_t *value)
{
    if (handle->id == Board_ADC_VDDS)
    {
        if (supply_convert_status != ADC_STATUS_SUCCESS)
        {
            return supply_convert_status;
        }
        supply_convert_count++;
        *value = 1U;
        return ADC_STATUS_SUCCESS;
    }

    if (ntc_convert_status != ADC_STATUS_SUCCESS)
    {
        return ntc_convert_status;
    }
    ntc_convert_count++;
    *value = 2U;
    return ADC_STATUS_SUCCESS;
}

uint32_t ADC_convertRawToMicroVolts(ADC_Handle handle, uint16_t value)
{
    if (sequence_length != 0U)
    {
        if (handle->id == Board_ADC_VDDS)
        {
            return supply_uv_sequence[supply_convert_count - 1U];
        }
        return ntc_uv_sequence[ntc_convert_count - 1U];
    }

    return value == 1U ? supply_uv : ntc_uv;
}

PIN_Handle PIN_open(PIN_State *state, const PIN_Config pin_list[])
{
    pin_open_count++;
    requested_pin_config = pin_list[0];
    return pin_open_fails ? NULL : state;
}

static void require_read(uint32_t new_supply_uv, uint32_t new_ntc_uv,
                         int32_t expected_centi_c, const char *message)
{
    int32_t actual = INT32_MIN;

    supply_uv = new_supply_uv;
    ntc_uv = new_ntc_uv;
    require(ntc_read(&actual) == NTC_OK, message);
    require(abs(actual - expected_centi_c) <= 1, message);
}

static void require_error(uint32_t new_supply_uv, uint32_t new_ntc_uv,
                          ntc_status_t expected, const char *message)
{
    const int32_t sentinel = 123456789;
    int32_t actual = sentinel;

    supply_uv = new_supply_uv;
    ntc_uv = new_ntc_uv;
    require(ntc_read(&actual) == expected, message);
    require(actual == sentinel, "read error changed output");
}

static void test_normal(void)
{
    const int32_t sentinel = 123456789;
    int32_t actual = sentinel;

    reset_mock();
    require(ntc_read(&actual) == NTC_ERROR_INIT && actual == sentinel,
            "read before init is incorrect");
    require(ntc_init(), "normal initialization failed");
    require(adc_init_count == 1U && adc_open_count == 2U && pin_open_count == 1U,
            "initialization resource sequence is incorrect");
    require((requested_pin_config & 0xffU) == Board_DIO6_AUX_SDA &&
            (requested_pin_config & PIN_GPIO_OUTPUT_EN) != 0U &&
            (requested_pin_config & PIN_GPIO_LOW) != 0U,
            "DIO6 is not requested as a low GPIO output");
    require(ntc_init() && adc_open_count == 2U && pin_open_count == 1U,
            "repeated initialization reopened resources");

    require_read(3300000U, 1650000U, 2500, "1.65V at 3.3V is not 25C");
    require_read(3000000U, 1500000U, 2500,
                 "same divider ratio at a different supply is not 25C");
    require_read(3300000U, 790041U, 0, "0C table value is incorrect");
    require_read(3300000U, 2427898U, 5000, "50C table value is incorrect");

    supply_convert_status = -1;
    require_error(3300000U, 1650000U, NTC_ERROR_ADC,
                  "supply ADC failure did not propagate");
    supply_convert_status = ADC_STATUS_SUCCESS;
    ntc_convert_status = -1;
    require_error(3300000U, 1650000U, NTC_ERROR_ADC,
                  "NTC ADC failure did not propagate");
    ntc_convert_status = ADC_STATUS_SUCCESS;

    require_error(3300000U, 0U, NTC_ERROR_INPUT, "zero rail input is incorrect");
    require_error(3300000U, 10000U, NTC_ERROR_INPUT, "low rail margin is incorrect");
    require_error(3300000U, 3300000U, NTC_ERROR_INPUT, "supply rail input is incorrect");
    require_error(3300000U, 3290000U, NTC_ERROR_INPUT, "high rail margin is incorrect");
    require_error(20000U, 10000U, NTC_ERROR_INPUT, "low supply input is incorrect");
    require_error(3300000U, 30000U, NTC_ERROR_RANGE, "high resistance range is incorrect");
    require_error(3300000U, 3267000U, NTC_ERROR_RANGE, "low resistance range is incorrect");
    require(ntc_read(NULL) == NTC_ERROR_INPUT, "null output error is incorrect");
    require_read(3300000U, 1650000U, 2500,
                 "valid probe input did not recover after read errors");
}

static void test_average(void)
{
    int32_t actual = INT32_MIN;
    unsigned int i;

    reset_mock();
    require(ntc_init(), "averaging initialization failed");
    sequence_length = 32U;
    for (i = 0U; i < sequence_length / 2U; i++)
    {
        supply_uv_sequence[i] = 3200000U;
        ntc_uv_sequence[i] = 1500000U;
    }
    for (; i < sequence_length; i++)
    {
        supply_uv_sequence[i] = 3400000U;
        ntc_uv_sequence[i] = 1800000U;
    }

    require(ntc_read(&actual) == NTC_OK && actual == 2500,
            "paired ADC averaging did not preserve the expected divider result");
    require(supply_convert_count == sequence_length &&
            ntc_convert_count == sequence_length,
            "paired ADC averaging did not perform 32 conversions per channel");
}

static void test_ntc_open_failure(void)
{
    int32_t actual = 1;

    reset_mock();
    failed_open_index = Board_ADC_NTC;
    require(!ntc_init(), "NTC ADC open failure was accepted");
    require(adc_open_count == 1U && adc_close_count == 0U && pin_open_count == 0U,
            "NTC ADC open failure cleanup is incorrect");
    require(ntc_read(&actual) == NTC_ERROR_INIT, "failed initialization published state");
}

static void test_supply_open_failure(void)
{
    int32_t actual = 1;

    reset_mock();
    failed_open_index = Board_ADC_VDDS;
    require(!ntc_init(), "supply ADC open failure was accepted");
    require(adc_open_count == 2U && adc_close_count == 1U && pin_open_count == 0U,
            "supply ADC open failure cleanup is incorrect");
    require(ntc_read(&actual) == NTC_ERROR_INIT, "failed initialization published state");
}

static void test_pin_open_failure(void)
{
    int32_t actual = 1;

    reset_mock();
    pin_open_fails = 1;
    require(!ntc_init(), "PIN open failure was accepted");
    require(adc_open_count == 2U && adc_close_count == 2U && pin_open_count == 1U,
            "PIN open failure cleanup is incorrect");
    require(ntc_read(&actual) == NTC_ERROR_INIT, "failed initialization published state");
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        (void)fprintf(stderr, "usage: %s normal|average|ntc-open|supply-open|pin-open\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (strcmp(argv[1], "normal") == 0)
    {
        test_normal();
    }
    else if (strcmp(argv[1], "average") == 0)
    {
        test_average();
    }
    else if (strcmp(argv[1], "ntc-open") == 0)
    {
        test_ntc_open_failure();
    }
    else if (strcmp(argv[1], "supply-open") == 0)
    {
        test_supply_open_failure();
    }
    else if (strcmp(argv[1], "pin-open") == 0)
    {
        test_pin_open_failure();
    }
    else
    {
        return EXIT_FAILURE;
    }
    (void)puts("ntc_driver_test: PASS");
    return EXIT_SUCCESS;
}
