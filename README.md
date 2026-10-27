# CTM 自制 CC1310 开发板串口回显

本工程基于 SimpleLink CC13x0 SDK 4.20.02.07 的 TI-RTOS uartecho 例程，首轮用于验证自制 CC1310F128RSMR 开发板的程序启动与 UART 双向收发。

## 板级适配

根据项目原理图，UART0 使用以下引脚：

| 信号 | DIO | 芯片封装脚号 | USB 转串口连接 |
| --- | --- | --- | --- |
| RX | DIO1 | 9 | 接适配器 TX |
| TX | DIO2 | 10 | 接适配器 RX |
| GND | — | — | 接适配器 GND |

使用与板卡匹配的 3.3 V TTL 串口。参数为 115200 波特率、8 数据位、1 停止位、无校验、无流控。

启动引脚表只初始化 UART RX/TX。UART TX 默认输出高电平，RX 使用上拉；程序不执行 LaunchPad 的 LED、按键及外部 Flash 启动操作。

保留官方板级文件名以便继续使用既有 CCS 工程。板级映射按 CC1310F128RSMR 的 32 引脚封装整理：

| 原理图网络 | 软件映射 | 封装脚号 | 配置状态 |
| --- | --- | --- | --- |
| RX-DIO-1 | UART0 RX，DIO1 | 9 | 串口回显使用 |
| TX-DIO-2 | UART0 TX，DIO2 | 10 | 串口回显使用 |
| TDO | DIO3 | 15 | 保留调试用途，应用不配置为输出 |
| EN-DIO-4-TDI | DIO4 | 16 | 保留调试用途，不驱动 EN |
| DIO-5-ADC | Board_ADC_NTC / Board_ADC0，DIO5 / AUXIO7 | 22 | ADC 与 ADCBuf 均已映射，应用未开启采样 |
| DIO-6-AUX-SDA | Board_DIO6_AUX_SDA / Board_GPIO_AUX_SDA，DIO6 | 23 | GPIO 条目不自动配置，外接回路用途待确认 |
| DIO-7-SCL | I2C0 SCL，DIO7 | 24 | 已映射，应用未开启 I2C |
| DIO-8-SDA | I2C0 SDA，DIO8 | 25 | 已映射，应用未开启 I2C |
| DIO0、DIO9 | 保留对应 DIO 别名 | 8、26 | 图中无外部连接，不主动配置 |

ADC/ADCBuf 仅暴露 DIO5 外部输入及芯片内部 VDDS、DCOUPL、VSS 通道；数字接口占用的 DIO6～DIO9 不作为 ADC 通道。DIO5 与 AUXIO7 的对应关系依据 TI SWRA640 的 CC13x0 AUX 引脚表。

GPIO 表仅保留 DIO6 的未配置条目。I2S、PWM、SPI、SD、LCD、外部 Flash 以及 LED/按键的板级描述和别名已移除；Display 仅保留 UART 后端。GPIO、ADC、I2C 等驱动不会由本例程主动开启，后续可在已映射接口上增加功能。Display UART 与本例程使用同一 UART0，不能同时打开；ADC 与 ADCBuf 也不能同时占用 DIO5。

I2C 上拉 R21/R22 在原理图标为 NC，实际启用传感器前须确认实装上拉及完整器件资料；NTC 采样前须确认外接回路、DIO6 用途、ADC 输入范围和换算参数。本版本运行行为仍为 UART 回显。

AUX 引脚映射参考：[TI CC13xx/CC26xx 硬件配置应用说明](https://www.ti.com/mx/lit/pdf/swra640)。

原理图标注 24 MHz 与 32.768 kHz 晶振以及片内 DC/DC 外围电路，与 SDK 默认 CCFG 配置一致，本轮保留默认配置。

## 构建

本机开发环境：

- CCS 21.0.1
- SimpleLink CC13x0 SDK 4.20.02.07
- TI ARM 编译器 18.12.5.LTS
- XDCtools 3.51.03.28

CCS 中导入当前工程及同级的 `tirtos_builds_CC1310_LAUNCHXL_release_ccs` 工程，并确认 SDK 产品路径与编译器已识别。先构建内核工程，再构建 `ctm`。两者需要一起提供，或者从对应 SDK 重新导入同名内核工程。

## 板上验证

1. 烧录后打开串口工具，关闭工具的本地回显。
2. 复位板卡，确认收到以下启动提示：

```text
CTM UART ready: RX=DIO1 TX=DIO2, 115200 8N1
```

3. 发送 `CTM123`，确认板卡返回完全相同的字符；换行字符也按原样回显。
4. 结束调试会话并重新上电，确认仍能输出提示并回显。

启动提示只发送一次，应在串口工具已打开时复位。程序阻塞等待接收字符，空闲期间没有周期输出。

编译成功仅验证软件构建。串口波形、接收数据以及脱离调试器后的启动行为需要在实物上检查。

`README.html` 和链接的 `Board.html` 为 TI 原始 LaunchPad 资料；本板接线与运行方式以本文件为准。
