# CTM 自制 CC1310 开发板 NTC 测温

本工程基于 SimpleLink CC13x0 SDK 4.20.02.07 和 TI-RTOS，在自制 CC1310F128RSMR 开发板上读取外接 NTC，每秒通过 UART0 输出一次温度。

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
| RX-DIO-1 | UART0 RX，DIO1 | 9 | 保留串口接收引脚 |
| TX-DIO-2 | UART0 TX，DIO2 | 10 | 输出温度 |
| TDO | DIO3 | 15 | 保留调试用途，应用不配置为输出 |
| EN-DIO-4-TDI | DIO4 | 16 | 保留调试用途，不驱动 EN |
| DIO-5-ADC | Board_ADC_NTC / Board_ADC0，DIO5 / AUXIO7 | 22 | NTC 分压采样 |
| DIO-6-AUX-SDA | Board_DIO6_AUX_SDA / Board_GPIO_AUX_SDA，DIO6 | 23 | NTC 驱动通过 PIN 接口输出低电平，接 R5 低端 |
| DIO-7-SCL | I2C0 SCL，DIO7 | 24 | 已映射，应用未开启 I2C |
| DIO-8-SDA | I2C0 SDA，DIO8 | 25 | 已映射，应用未开启 I2C |
| DIO0、DIO9 | 保留对应 DIO 别名 | 8、26 | 图中无外部连接，不主动配置 |

ADC/ADCBuf 仅暴露 DIO5 外部输入及芯片内部 VDDS、DCOUPL、VSS 通道；数字接口占用的 DIO6～DIO9 不作为 ADC 通道。DIO5 与 AUXIO7 的对应关系依据 TI SWRA640 的 CC13x0 AUX 引脚表。

GPIO 表保留 DIO6 的未配置条目，由 NTC 驱动通过 PIN 接口独占该引脚。应用开启 ADC 和 UART；Display UART 与应用使用同一 UART0，不能同时打开；ADCBuf 不能同时占用 DIO5。

I2C 上拉 R21/R22 在原理图标为 NC，实际启用 I2C 前须确认实装上拉及完整器件资料。

## NTC 回路与换算

探头接在原理图的 NTC `VCC` 与 `ADC` 两个接口之间：

```text
3V3-433 ── NTC ──┬── DIO5 / AUXIO7
                 └── R5 10kΩ ── DIO6（输出低电平）
```

使用用户确认的 R25=10kΩ、B25/50=3950 探头。阻温数据采用用户提供图片中红圈列的全部 37 个点，覆盖 -40～140℃，每 5℃ 一个点，数据位于 `Core/Src/ntc_curve.c`。在相邻两点之间按电阻线性插值，超出表格范围时返回错误。

NTC 分压和内部 VDDS 通道均使用固定 ADC 参考及 SDK 工厂校准换算为微伏。VDDS 与 3V3-433 经磁珠相连，使用每次实测的 VDDS 近似分压供电电压。计算公式为 `R_NTC = 10000 × (V_supply - V_ADC) / V_ADC`。R5 的实际误差、GPIO 低电平压降和探头公差仍需实物校验。

每次输出温度前，驱动对 VDDS 和 NTC ADC 做 32 组配对采样，分别对校准后的微伏值求平均，再进行分压和查表换算；这用于降低恒温时的 ADC 噪声和读数抖动，不等同于提高探头的绝对精度。

DIO5 采样窗口为 42.6µs。DIO6 在本版本运行期间持续输出低电平。温度显示两位小数仅表示输出分辨率。

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

1. 按正常方式给开发板稳定供电，将 NTC 接入板上的 `VCC`、`ADC` 接口。USB-TTL 保持原有接线，并与开发板共地。
2. 烧录 `Debug/ctm.out`，打开串口工具，设置为 115200 8N1；复位后应收到启动提示，并每秒输出温度。下面的温度数值是输出格式示例：

```text
CTM NTC ready: 10k B25/50=3950, 115200 8N1
NTC temperature=25.00 C
```

3. 常温下记录 10～20 行温度，并与放在探头旁的参考温度计对照。
4. 用手握住探头或温和加热探头，温度应上升；松开后应逐渐回落。保持开发板供电电压不变。
5. 断开探头，串口应输出错误；重新接好后应自动恢复温度输出。
6. 条件允许时，用已知电阻替代探头验证换算：10kΩ 对应 25℃，31.770kΩ 对应 0℃，3.592kΩ 对应 50℃。电阻精度和 ADC 误差会影响实测结果。

启动提示只发送一次，应在串口工具已打开时复位。`NTC error=2` 表示 ADC 转换失败；`3` 表示输入接近电源轨或供电采样异常；`4` 表示换算阻值超出阻温表。初始化失败会输出 `NTC error: initialization failed` 并结束采集任务。

本阶段通过条件：持续输出合理温度、温升/降温方向正确、断线时输出错误且重接可恢复。绝对测温误差需结合参考温度计另行记录；编译和主机测试通过不能替代板上验证。

## 主机测试

`tests/` 已从 CCS 的 Debug、Release 目标编译中排除。在工程根目录使用 GCC 检查阻温表、插值、边界和非法输入：

```powershell
gcc -std=c99 -Wall -Wextra -Werror -I Core/Inc tests/ntc_curve_test.c Core/Src/ntc_curve.c -lm -o "$env:TEMP/ctm_ntc_curve_test.exe"
& "$env:TEMP/ctm_ntc_curve_test.exe"
```

驱动测试使用主机替身模拟 ADC/PIN，编译的是实际 `ntc.c`，检查分压方向、实测供电换算、初始化失败清理、ADC 错误、断线输入和恢复：

```powershell
gcc -std=c99 -Wall -Wextra -Werror -I tests/stubs -I Driver/Inc -I Core/Inc tests/ntc_driver_test.c Driver/Src/ntc.c Core/Src/ntc_curve.c -lm -o "$env:TEMP/ctm_ntc_driver_test.exe"
foreach ($scenario in 'normal', 'average', 'ntc-open', 'supply-open', 'pin-open') {
    & "$env:TEMP/ctm_ntc_driver_test.exe" $scenario
    if ($LASTEXITCODE -ne 0) { throw "NTC driver test failed: $scenario" }
}
```

`README.html` 和链接的 `Board.html` 为 TI 原始 LaunchPad 资料；本板接线与运行方式以本文件为准。
