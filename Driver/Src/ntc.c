#include <stddef.h>

#include <ti/drivers/ADC.h>
#include <ti/drivers/PIN.h>

#include "Board.h"
#include "ntc.h"
#include "ntc_curve.h"

#define NTC_DIVIDER_OHM      10000.0f
#define NTC_RAIL_MARGIN_UV   10000U
#define NTC_SAMPLE_COUNT     32U

static ADC_Handle ntc_adc;
static ADC_Handle ntc_supply_adc;
static PIN_State ntc_pin_state;

/* 3V3-433 → NTC → DIO5 → R5(10kΩ) → DIO6，DIO6 拉低形成分压。 */
static const PIN_Config ntc_pin_table[] = {
    Board_DIO6_AUX_SDA | PIN_INPUT_DIS | PIN_NOPULL | PIN_IRQ_DIS |
    PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MIN,
    PIN_TERMINATE
};

static bool ntc_read_voltage(ADC_Handle adc, uint32_t *voltage_uv)
{
    uint16_t raw;

    if (ADC_convert(adc, &raw) != ADC_STATUS_SUCCESS)
    {
        return false;
    }

    *voltage_uv = ADC_convertRawToMicroVolts(adc, raw);
    return true;
}

bool ntc_init(void)
{
    ADC_Params params;
    ADC_Handle adc;
    ADC_Handle supply_adc;

    if (ntc_adc != NULL)
    {
        return true;
    }

    ADC_init();
    ADC_Params_init(&params);
    adc = ADC_open(Board_ADC_NTC, &params);
    if (adc == NULL)
    {
        return false;
    }

    supply_adc = ADC_open(Board_ADC_VDDS, &params);
    if (supply_adc == NULL)
    {
        ADC_close(adc);
        return false;
    }

    if (PIN_open(&ntc_pin_state, ntc_pin_table) == NULL)
    {
        ADC_close(supply_adc);
        ADC_close(adc);
        return false;
    }

    ntc_adc = adc;
    ntc_supply_adc = supply_adc;
    return true;
}

ntc_status_t ntc_read(int32_t *temperature_centi_c)
{
    uint64_t supply_uv_sum = 0U;
    uint64_t adc_uv_sum = 0U;
    uint32_t supply_uv;
    uint32_t adc_uv;
    uint32_t sample;
    float resistance_ohm;

    if (ntc_adc == NULL)
    {
        return NTC_ERROR_INIT;
    }

    if (temperature_centi_c == NULL)
    {
        return NTC_ERROR_INPUT;
    }

    /* VDDS 与 3V3-433 经磁珠相连，两路均用固定参考和工厂校准。 */
    for (sample = 0U; sample < NTC_SAMPLE_COUNT; sample++)
    {
        if (!ntc_read_voltage(ntc_supply_adc, &supply_uv) ||
            !ntc_read_voltage(ntc_adc, &adc_uv))
        {
            return NTC_ERROR_ADC;
        }

        supply_uv_sum += supply_uv;
        adc_uv_sum += adc_uv;
    }

    supply_uv = (uint32_t)(supply_uv_sum / NTC_SAMPLE_COUNT);
    adc_uv = (uint32_t)(adc_uv_sum / NTC_SAMPLE_COUNT);

    if (supply_uv <= 2U * NTC_RAIL_MARGIN_UV ||
        adc_uv <= NTC_RAIL_MARGIN_UV || adc_uv >= supply_uv ||
        supply_uv - adc_uv <= NTC_RAIL_MARGIN_UV)
    {
        return NTC_ERROR_INPUT;
    }

    resistance_ohm = NTC_DIVIDER_OHM * (float)(supply_uv - adc_uv) / (float)adc_uv;
    if (!ntc_curve_temperature(resistance_ohm, temperature_centi_c))
    {
        return NTC_ERROR_RANGE;
    }

    return NTC_OK;
}
