#ifndef __A7680E_V905_GPIO_H__
#define __A7680E_V905_GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

/* A7680C_MNNV_LNNV GPIO Resource Sheet  */
typedef enum
{
    SC_MODULE_GPIO_00 = 4,   /*  pin 3, pad name: GPIO_00/UART1_RTS/KP_MKIN0 */
    SC_MODULE_GPIO_01 = 5,   /*  pin 4, pad name: GPIO_01/UART1_CTS/KP_MKOUT0 */
    SC_MODULE_GPIO_02 = 37,  /*  pin 5, pad name: GPIO_02/UART1_DCD */
#ifdef FEATURE_SIMCOM_NODTR
    SC_MODULE_GPIO_03 = 6,   /*  pin 6, pad name: DTR  */
#endif
    SC_MODULE_GPIO_04 = 7,   /*  pin 7, pad name: GPIO_04/UART1_RI */

    SC_MODULE_GPIO_05 = 9,   /*  pin 20, pad name: GPIO_05/KP_MKOUT1 */
    SC_MODULE_GPIO_08 = 85,  /*  pin 41, pad name: GPIO_08 */
    SC_MODULE_GPIO_09 = 89,  /*  pin 42, pad name: GPIO_09/PWM3 */
    SC_MODULE_GPIO_10 = 69,  /*  pin 43, pad name: GPIO_10/PWM1/GRFC1 */
    SC_MODULE_GPIO_11 = 70,  /*  pin 44, pad name: GPIO_11/PWM2/GRFC2 */

    SC_MODULE_GPIO_12 = 88,   /*  pin 57, pad name: GPIO_12 */
    SC_MODULE_GPIO_13 = 34,  /*  pin 62, pad name: GPIO_13/SPI_CS */
    SC_MODULE_GPIO_14 = 33,  /*  pin 63, pad name: GPIO_14/SPI_CLK */
    SC_MODULE_GPIO_15 = 35,  /*  pin 64, pad name: GPIO_15/SPI_MISO */
    SC_MODULE_GPIO_16 = 36,  /*  pin 65, pad name: GPIO_16/SPI_MOSI */

    SC_MODULE_GPIO_17 = 50,  /*  pin 68, pad name: GPIO_17/I2C_SDA */
    SC_MODULE_GPIO_18 = 49,  /*  pin 69, pad name: GPIO_18/I2C_SCL */
    SC_MODULE_GPIO_25 = 90,  /*  pin 80, pad name: GPIO_25 */

    SC_MODULE_GPIO_32 = 8,    /*  pin 52, pad name: GPIO_32/USIM2_DET/KP_MKIN1 */
    SC_MODULE_GPIO_33 = 74,   /*  pin 53, pad name: GPIO_33/USIM2_DATA */
    SC_MODULE_GPIO_34 = 73,   /*  pin 54, pad name: GPIO_34/USIM2_CLK */
    SC_MODULE_GPIO_35 = 57,   /*  pin 55, pad name: GPIO_35/USIM2_RST */
    SC_MODULE_GPIO_36 = 56,   /*  pin 60, pad name: GPIO_36/1PPS */

    SC_MODULE_GPIO_MAX = 128,
} SC_Module_GPIONumbers;

#ifdef __cplusplus
}
#endif

#endif
