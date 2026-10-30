#ifndef NTC_CURVE_H
#define NTC_CURVE_H

#include <stdbool.h>
#include <stdint.h>

/* 输入单位为 Ω，输出单位为 0.01℃；超出阻温表时保持输出不变。 */
bool ntc_curve_temperature(float resistance_ohm, int32_t *temperature_centi_c);

#endif /* NTC_CURVE_H */
