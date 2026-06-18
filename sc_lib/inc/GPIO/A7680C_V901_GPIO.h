#ifndef __A7680C_V901_GPIO_H__
#define __A7680C_V901_GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    SC_MODULE_GPIO_00 = 53, /*  pin 3,  pad name: GPIO_00/UART1_RTS             */
    SC_MODULE_GPIO_01 = 54, /*  pin 4,  pad name: GPIO_01/UART1_CTS             */
    SC_MODULE_GPIO_02 = 5,  /*  pin 5,  pad name: GPIO_02/MKOUT4/UART1_DCD      */
    SC_MODULE_GPIO_04 = 4,  /*  pin 7,  pad name: GPIO_04/MKIN4/UART1_RI        */
    SC_MODULE_GPIO_05 = 9,  /*  pin 20, pad name: GPIO_05/MKOUT2                */
#ifndef FEATURE_SIMCOM_GPS
    SC_MODULE_GPIO_06 = 3, /*  pin 22, pad name: GPIO_6/UART3_TXD/GNSS_RXD                  */
    SC_MODULE_GPIO_07 = 2, /*  pin 23, pad name: GPIO_07/UART3_RXD/GNSS_TXD                  */
#endif
    SC_MODULE_GPIO_08 = 70, /*  pin 41, pad name: GPIO_08/PWM2                  */
    SC_MODULE_GPIO_09 = 69, /*  pin 42, pad name: GPIO_09/PWM1                  */
    SC_MODULE_GPIO_10 = 01, /*  pin 43, pad name: GPIO_10/GRFC1/MKOUT0          */
    SC_MODULE_GPIO_11 = 68, /*  pin 44, pad name: GPIO_11/GRFC2                 */
    SC_MODULE_GPIO_12 = 37, /*  pin 57, pad name: GPIO_12/SMART_BAT/LCD_TE      */
    SC_MODULE_GPIO_13 = 13, /*  pin 62, pad name: GPIO_13/SPI2_CS               */
    SC_MODULE_GPIO_14 = 12, /*  pin 63, pad name: GPIO_14/SPI2_CL               */
    SC_MODULE_GPIO_15 = 14, /*  pin 64, pad name: GPIO_15/SPI2_MISO             */
    SC_MODULE_GPIO_16 = 15, /*  pin 65, pad name: GPIO_16/SPI2_MOSI             */
    SC_MODULE_GPIO_17 = 50, /*  pin 68, pad name: GPIO_17/CI2C_SDA              */
    SC_MODULE_GPIO_18 = 49, /*  pin 69, pad name: GPIO_18/CI2C_SCL              */
    SC_MODULE_GPIO_19 = 10, /*  pin 46, pad name: GPIO_19/I2C2_SCL              */
    SC_MODULE_GPIO_20 = 11, /*  pin 47, pad name: GPIO_20/I2C2_SDA              */
    SC_MODULE_GPIO_21 = 87, /*  pin 48, pad name: GPIO_21                       */
    SC_MODULE_GPIO_22 = 88, /*  pin 49, pad name: GPIO_22                       */
    SC_MODULE_GPIO_23 = 90, /*  pin 50, pad name: GPIO_23                       */
    SC_MODULE_GPIO_24 = 56, /*  pin 51, pad name: GPIO_24                       */
    SC_MODULE_GPIO_25 = 33, /*  pin 80, pad name: GPIO_25/LCD_SPI_CLK/SPI0_CLK  */
    SC_MODULE_GPIO_26 = 34, /*  pin 80, pad name: GPIO_26/LCD_SPI_CS/SPI0_CS    */
    SC_MODULE_GPIO_27 = 84, /*  pin 82, pad name: GPIO_27/LCD_RST               */
    SC_MODULE_GPIO_28 = 35, /*  pin 83, pad name: GPIO_28/LCD_SPI_DCX/SPI0_MISO */
    SC_MODULE_GPIO_29 = 36, /*  pin 84, pad name: GPIO_29/LCD_SPI_TXD/SPI0_MOSI */
    SC_MODULE_GPIO_30 = 89, /*  pin 86, pad name: GPIO_30                       */

#ifdef FEATURE_SIMCOM_NODTR
    SC_MODULE_GPIO_31 = 6, /*pin 6, pad name: DTR                */
#endif
    SC_MODULE_GPIO_MAX = 128,
} SC_Module_GPIONumbers;

#ifdef __cplusplus
}
#endif

#endif
