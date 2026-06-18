#ifndef __A7690C_V606_GPIO_H__
#define __A7690C_V606_GPIO_H__

typedef enum
{
    SC_MODULE_GPIO_01 = 70, /*  pin 16,  pad name: GPIO_01/PWM1  */
    SC_MODULE_GPIO_02 = 5,  /*  pin 21,  pad name: GPIO_02  */
    SC_MODULE_GPIO_03 = 54,  /*  pin 22,  pad name: GPIO_03/MAIN_CTS  */
    SC_MODULE_GPIO_04 = 53,  /*  pin 23,  pad name: GPIO_04/MAIN_RTS  */
    SC_MODULE_GPIO_05 = 69,  /*  pin 25, pad name: GPIO_05/PWM2  */
    SC_MODULE_GPIO_06 = 12, /*  pin 30, pad name: GPIO_06/SPI_CLK  */
    SC_MODULE_GPIO_07 = 13, /*  pin 31, pad name: GPIO_07/SPI_CS  */
    SC_MODULE_GPIO_08 = 14, /*  pin 32, pad name: GPIO_08/SPI_MISO  */
    SC_MODULE_GPIO_09 = 15, /*  pin 33, pad name: GPIO_09/SPI_MOSI  */
    SC_MODULE_GPIO_10 = 11, /*  pin 66, pad name: GPIO_10/I2C2_SDA  */
    SC_MODULE_GPIO_11 = 10, /*  pin 67, pad name: GPIO_11/I2C2_SCL  */
    SC_MODULE_GPIO_12 = 49, /*  pin 68, pad name: GPIO_12/I2C_SCL  */
    SC_MODULE_GPIO_13 = 50, /*  pin 69, pad name: GPIO_13/I2C_SDA  */
    SC_MODULE_GPIO_15 = 9, /*  pin 82, pad name: GPIO_15/USB_BOOT  */
    SC_MODULE_GPIO_16 = 87, /*  pin 84, pad name: GPIO_16/MK_IN2  */
    SC_MODULE_GPIO_17 = 88, /*  pin 85, pad name: GPIO_17/MK_OUT2  */
    SC_MODULE_GPIO_18 = 89, /*  pin 86, pad name: GPIO_18/MK_IN0   */
    SC_MODULE_GPIO_19 = 90, /*  pin 87, pad name: GPIO_19/MK_OUT0  */
    SC_MODULE_GPIO_20 = 2, /*  pin 28, pad name: GPIO_20/MK_IN1/UART3_RXD  */
    SC_MODULE_GPIO_21 = 3, /*  pin 29, pad name: GPIO_21/MK_OUT1/UART3_TXD  */
    SC_MODULE_GPIO_22 = 4, /*  pin 20, pad name: GPIO_22  */
    SC_MODULE_GPIO_23 = 84, /*  pin 49, pad name: GPIO_23  */
    SC_MODULE_GPIO_24 = 36, /*  pin 50, pad name: GPIO_24/SPI2_MOSI  */
    SC_MODULE_GPIO_25 = 35, /*  pin 51, pad name: GPIO_25/SPI2_MISO  */
    SC_MODULE_GPIO_26 = 34, /*  pin 52, pad name: GPIO_26/SPI2_CS  */
    SC_MODULE_GPIO_27 = 33, /*  pin 53, pad name: GPIO_27/SPI2_CLK  */
    SC_MODULE_GPIO_28 = 73, /*  pin 62, pad name: GPIO_28/USIM2_CLK  */
    SC_MODULE_GPIO_29 = 55, /*  pin 63, pad name: GPIO_29/USIM2_RST  */
    SC_MODULE_GPIO_30 = 74, /*  pin 64, pad name: GPIO_30/USIM2_DATA  */
    SC_MODULE_GPIO_31 = 56,  /*  pin 76,  pad name: GPIO_31  */
    SC_MODULE_GPIO_32 = 1, /*  pin 77, pad name: GPIO_32  */
    SC_MODULE_GPIO_33 = 37, /*  pin 78, pad name: GPIO_33  */
    SC_MODULE_GPIO_34 = 8, /*  pin 83, pad name: GPIO_34  */
    SC_MODULE_GPIO_35 = 68, /*  pin 109, pad name: GPIO_35  */

#ifdef FEATURE_SIMCOM_NODTR
    SC_MODULE_GPIO_36 = 6, /*pin 19, pad name: DTR  */
#endif
    SC_MODULE_GPIO_MAX = 128,
} SC_Module_GPIONumbers;

#endif
