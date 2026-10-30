#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <ti/drivers/UART.h>

#include "Board.h"
#include "ntc.h"

#define TEMPERATURE_OUTPUT_SIZE    80U
#define TEMPERATURE_PERIOD_S       1U

static bool temperature_send(UART_Handle uart, const char *text)
{
    size_t length = strlen(text);

    return UART_write(uart, text, length) == (int_fast32_t)length;
}

void *temperature_task(void *arg)
{
    UART_Handle uart;
    UART_Params params;
    ntc_status_t status;
    int32_t temperature;
    uint32_t magnitude;
    char output[TEMPERATURE_OUTPUT_SIZE];
    int length;

    (void)arg;
    UART_init();
    UART_Params_init(&params);
    params.writeDataMode = UART_DATA_BINARY;
    params.baudRate = 115200;
    uart = UART_open(Board_UART0, &params);
    if (uart == NULL)
    {
        return NULL;
    }

    if (!ntc_init())
    {
        /* 初始化失败时只上报错误，不输出温度。 */
        if (!temperature_send(uart, "NTC error: initialization failed\r\n"))
        {
            UART_close(uart);
            return NULL;
        }
        UART_close(uart);
        return NULL;
    }

    if (!temperature_send(uart,
        "CTM NTC ready: 10k B25/50=3950, 115200 8N1\r\n"))
    {
        UART_close(uart);
        return NULL;
    }

    while (1)
    {
        status = ntc_read(&temperature);
        if (status == NTC_OK)
        {
            magnitude = (uint32_t)(temperature < 0 ? -temperature : temperature);
            length = snprintf(output, sizeof(output),
                "NTC temperature=%s%lu.%02lu C\r\n",
                temperature < 0 ? "-" : "",
                (unsigned long)(magnitude / 100U),
                (unsigned long)(magnitude % 100U));
        }
        else
        {
            length = snprintf(output, sizeof(output),
                "NTC error=%u (2=ADC, 3=input, 4=table range)\r\n",
                (unsigned int)status);
        }

        if (length <= 0 || (size_t)length >= sizeof(output) ||
            !temperature_send(uart, output))
        {
            UART_close(uart);
            return NULL;
        }

        sleep(TEMPERATURE_PERIOD_S);
    }
}
