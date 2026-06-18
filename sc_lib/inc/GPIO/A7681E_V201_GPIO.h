
#ifndef __A7681E_V201_GPIO_H__
#define __A7681E_V201_GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    SC_MODULE_GPIO_00 = 53, /*  pin 3,  pad name: GPIO_00/UART1_RTS             */
    SC_MODULE_GPIO_01 = 54, /*  pin 4,  pad name: GPIO_01/UART1_CTS             */
    SC_MODULE_GPIO_02 = 5,  /*  pin 5,  pad name: GPIO_02/MKOUT4/UART1_DCD      */
    SC_MODULE_GPIO_03 = 4,  /*  pin 7,  pad name: GPIO_03/MKIN4/UART1_RI        */
    SC_MODULE_GPIO_04 = 70,  /*  pin 41,  pad name: GPIO_04/PWM2                 */
    SC_MODULE_GPIO_05 = 69,  /*  pin 42,  pad name: GPIO_05/PWM1                  */
    SC_MODULE_GPIO_06 = 33, /*  pin 46, pad name: GPIO_6/SPI1_CLK                  */
    SC_MODULE_GPIO_07 = 34, /*  pin 47, pad name: GPIO_07/SPI1_CS                  */
    SC_MODULE_GPIO_08 = 35, /*  pin 48, pad name: GPIO_08/SPI1_MISO             */
    SC_MODULE_GPIO_09 = 36, /*  pin 49, pad name: GPIO_09/SPI1_MOSI             */
    SC_MODULE_GPIO_10 = 14, /*  pin 50, pad name: GPIO_10/SPI2_MISO             */
    SC_MODULE_GPIO_11 = 15, /*  pin 51, pad name: GPIO_11/SPI2_MOSI             */
    SC_MODULE_GPIO_12 = 68, /*  pin 57, pad name: GPIO_12/GRFC1                 */
    SC_MODULE_GPIO_13 = 1, /*  pin 58, pad name: GPIO_13/GRFC2                 */
    SC_MODULE_GPIO_14 = 50, /*  pin 64, pad name: GPIO_14/I2C_SDA               */
    SC_MODULE_GPIO_15 = 49, /*  pin 65, pad name: GPIO_15/I2C_SCL               */
    SC_MODULE_GPIO_16 = 12, /*  pin 78, pad name: GPIO_16/SPI2_CLK             */
    SC_MODULE_GPIO_17 = 13, /*  pin 79, pad name: GPIO_17/SPI2_CS              */
    SC_MODULE_GPIO_18 = 10, /*  pin 80, pad name: GPIO_18/I2C2_SCL              */
    SC_MODULE_GPIO_19 = 11, /*  pin 81, pad name: GPIO_19/I2C2_SDA              */
    SC_MODULE_GPIO_20 = 9, /*  pin 82, pad name: GPIO_20/USB_BOOT              */
    SC_MODULE_GPIO_21 = 37, /*  pin 52, pad name: GPIO_21/SIM2_DET              */
    SC_MODULE_GPIO_22 = 74, /*  pin 53, pad name: GPIO_22/SIM2_DATA              */
    SC_MODULE_GPIO_23 = 73, /*  pin 54, pad name: GPIO_23/SIM2_CLK              */
    SC_MODULE_GPIO_24 = 55, /*  pin 55, pad name: GPIO_24/SIM2_RST              */

#ifndef FEATURE_SIMCOM_GPS
    SC_MODULE_GPIO_25 = 3, /*  pin 22, pad name: GPIO_26/UART3_TXD  */
    SC_MODULE_GPIO_26 = 2, /*  pin 23, pad name: GPIO_25/UART3_RXD  */
#endif

#ifdef FEATURE_SIMCOM_NODTR
    SC_MODULE_GPIO_27 = 6, /*pin 6, pad name: DTR                */
#endif
    SC_MODULE_GPIO_MAX = 128,
} SC_Module_GPIONumbers;

#ifdef __cplusplus
}
#endif

#endif
