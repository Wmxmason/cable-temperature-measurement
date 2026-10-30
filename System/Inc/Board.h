/*
 * Copyright (c) 2015-2019, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __BOARD_H
#define __BOARD_H

#define Board_CC1310_LAUNCHXL

#ifdef __cplusplus
extern "C" {
#endif

#include <ti/drivers/Board.h>

#define Board_initGeneral()     Board_init()  /* deprecated */

#include "CC1310_LAUNCHXL.h"

/* These #defines allow us to reuse TI-RTOS across other device families */

#define Board_ADC0              CC1310_LAUNCHXL_ADC0
#define Board_ADC_NTC           CC1310_LAUNCHXL_ADC0
#define Board_ADC_VDDS          CC1310_LAUNCHXL_ADCVDDS

#define Board_DIO0              CC1310_LAUNCHXL_DIO0
#define Board_DIO1              CC1310_LAUNCHXL_DIO1
#define Board_DIO2              CC1310_LAUNCHXL_DIO2
#define Board_DIO3_TDO          CC1310_LAUNCHXL_DIO3_TDO
#define Board_DIO4_TDI          CC1310_LAUNCHXL_DIO4_TDI
#define Board_DIO5_ADC          CC1310_LAUNCHXL_DIO5_ANALOG
#define Board_DIO6_AUX_SDA      CC1310_LAUNCHXL_DIO6_AUX_SDA
#define Board_DIO7_SCL          CC1310_LAUNCHXL_DIO7_SCL
#define Board_DIO8_SDA          CC1310_LAUNCHXL_DIO8_SDA
#define Board_DIO9              CC1310_LAUNCHXL_DIO9
#define Board_GPIO_AUX_SDA      CC1310_LAUNCHXL_GPIO_AUX_SDA

#define Board_ADCBUF0           CC1310_LAUNCHXL_ADCBUF0
#define Board_ADCBUF0CHANNEL0   CC1310_LAUNCHXL_ADCBUF0CHANNEL0

#define Board_CRYPTO0           CC1310_LAUNCHXL_CRYPTO0
#define Board_AESCCM0           CC1310_LAUNCHXL_AESCCM0
#define Board_AESGCM0           CC1310_LAUNCHXL_AESGCM0
#define Board_AESCBC0           CC1310_LAUNCHXL_AESCBC0
#define Board_AESCTR0           CC1310_LAUNCHXL_AESCTR0
#define Board_AESECB0           CC1310_LAUNCHXL_AESECB0
#define Board_AESCTRDRBG0       CC1310_LAUNCHXL_AESCTRDRBG0
#define Board_TRNG0             CC1310_LAUNCHXL_TRNG0

#define Board_GPTIMER0A         CC1310_LAUNCHXL_GPTIMER0A
#define Board_GPTIMER0B         CC1310_LAUNCHXL_GPTIMER0B
#define Board_GPTIMER1A         CC1310_LAUNCHXL_GPTIMER1A
#define Board_GPTIMER1B         CC1310_LAUNCHXL_GPTIMER1B
#define Board_GPTIMER2A         CC1310_LAUNCHXL_GPTIMER2A
#define Board_GPTIMER2B         CC1310_LAUNCHXL_GPTIMER2B
#define Board_GPTIMER3A         CC1310_LAUNCHXL_GPTIMER3A
#define Board_GPTIMER3B         CC1310_LAUNCHXL_GPTIMER3B

#define Board_I2C0              CC1310_LAUNCHXL_I2C0
#define Board_I2C_TMP           CC1310_LAUNCHXL_I2C0

#define Board_NVSINTERNAL       CC1310_LAUNCHXL_NVSCC26XX0

#define Board_UART0             CC1310_LAUNCHXL_UART0

#define Board_WATCHDOG0         CC1310_LAUNCHXL_WATCHDOG0

#ifdef __cplusplus
}
#endif

#endif /* __BOARD_H */
