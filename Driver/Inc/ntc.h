#ifndef NTC_H
#define NTC_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    NTC_OK = 0,
    NTC_ERROR_INIT,
    NTC_ERROR_ADC,
    NTC_ERROR_INPUT,
    NTC_ERROR_RANGE
} ntc_status_t;

/* 调用前完成 Board_init()，由单个任务持有 NTC 采样资源。 */
bool ntc_init(void);
ntc_status_t ntc_read(int32_t *temperature_centi_c);

#endif /* NTC_H */
