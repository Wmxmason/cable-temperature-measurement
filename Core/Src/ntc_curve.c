#include <stddef.h>

#include "ntc_curve.h"

#define NTC_TABLE_MIN_C     (-40.0f)
#define NTC_TABLE_STEP_C    5.0f

/* 10kΩ/B25/50=3950 阻温表，-40～140℃，间隔 5℃，单位 Ω。 */
static const float ntc_resistance_ohm[] = {
    277225.1f, 211512.9f, 157245.9f, 116610.3f, 87428.8f, 66924.8f,
    51815.4f, 40450.5f, 31770.0f, 24935.0f, 19681.9f, 15621.2f,
    12465.4f, 10000.0f, 8064.4f, 6537.5f, 5327.1f, 4383.2f,
    3592.0f, 2972.2f, 2471.8f, 2065.9f, 1735.3f, 1464.8f,
    1242.5f, 1059.0f, 908.3f, 781.6f, 674.4f, 585.8f,
    508.3f, 441.2f, 383.5f, 334.1f, 292.4f, 256.5f, 225.8f
};

bool ntc_curve_temperature(float resistance_ohm, int32_t *temperature_centi_c)
{
    const size_t count = sizeof(ntc_resistance_ohm) / sizeof(ntc_resistance_ohm[0]);
    size_t i;
    float fraction;
    float temperature;

    if (temperature_centi_c == NULL ||
        !(resistance_ohm <= ntc_resistance_ohm[0] &&
          resistance_ohm >= ntc_resistance_ohm[count - 1U]))
    {
        return false;
    }

    for (i = 1U; i < count; i++)
    {
        if (resistance_ohm >= ntc_resistance_ohm[i])
        {
            fraction = (ntc_resistance_ohm[i - 1U] - resistance_ohm) /
                       (ntc_resistance_ohm[i - 1U] - ntc_resistance_ohm[i]);
            temperature = (NTC_TABLE_MIN_C + NTC_TABLE_STEP_C *
                          ((float)(i - 1U) + fraction)) * 100.0f;
            *temperature_centi_c = (int32_t)(temperature +
                                           (temperature < 0.0f ? -0.5f : 0.5f));
            return true;
        }
    }

    return false;
}
