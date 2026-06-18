#ifndef __A7670_V901_GPIO_H__
#define __A7670_V901_GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

/* A7670_V901 GPIO Resource Sheet  */
typedef enum {
    SC_MODULE_GPIO_NOT_ASSIGNED = -1,

    SC_MODULE_GPIO_00 = 4,   /*  pin4, pad name: GPIO_00  */
    SC_MODULE_GPIO_01 = 87,   /*  pin5, pad name: GPIO_01*/
    SC_MODULE_GPIO_02 = 9,   /*  pin6, pad name: GPIO_02 USB_BOOT*/
    SC_MODULE_GPIO_03 = 54,  /*  pin7, pad name: GPIO_03/UART_CTS */
    SC_MODULE_GPIO_04 = 53,  /*  pin8, pad name: GPIO_04/UART_RTS  */
    SC_MODULE_GPIO_05 = 86,  /*  pin19, pad name: GPIO_05/VCXO_OUT  */
    SC_MODULE_GPIO_06 = 73,  /*  pin20, pad name: GPIO_06/USIM2_CLK  */
    SC_MODULE_GPIO_07 = 55,  /*  pin21, pad name: GPIO_07/USIM2_RST  */
    SC_MODULE_GPIO_08 = 88,  /*  pin48, pad name: GPIO_08/LCD_DCX  */
    SC_MODULE_GPIO_09 = 69,  /*  pin52, pad name: GPIO_09 /PWM1  */
    SC_MODULE_GPIO_10 = 70,  /*  pin66, pad name: GPIO_10 /PWM2  */

    SC_MODULE_GPIO_11 = 89,   /*  pin67, pad name: GPIO_11/LCD_PWM_BL/PWM3 */
    SC_MODULE_GPIO_12 = 85,   /*  pin68, pad name: GPIO_12 */
    SC_MODULE_GPIO_13 = 11,   /*  pin35, pad name: GPIO_13/I2C2_SDA  */
    SC_MODULE_GPIO_14 = 10,   /*  pin36, pad name: GPIO_14/I2C2_SCL  */
    SC_MODULE_GPIO_15 = 74,  /*  pin44, pad name: GPIO_15/USIM2_DATA  */
    SC_MODULE_GPIO_16 = 8,   /*  pin47, pad name: GPIO_16/LCD_SPI_CS  */
    SC_MODULE_GPIO_17 = 37,   /*  pin26, pad name: GPIO_17/SMART_BAT  */

    SC_MODULE_GPIO_18 = 90,  /*  pin53, pad name: GPIO_18  */

    SC_MODULE_GPIO_20 = 33,  /*  pin102,pad name: GPIO_20/LCD_SPI_CLK/SPI0_CLK/SSP0_CLK */
    SC_MODULE_GPIO_21 = 36,  /*  pin103,pad name: GPIO_21/LCD_SPI_TXD/SPI0_MOSI/SSP0_TXD */
    SC_MODULE_GPIO_22 = 35,  /*  pin104,pad name: GPIO_22/LCD_SPI_RXD/SPI0_MISO/SSP0_RXD */
    SC_MODULE_GPIO_23 = 34,  /*  pin105,pad name: GPIO_23/LCD_SPI_CS/SPI0_CS/SSP0_FRM */


    SC_MODULE_GPIO_34 = 12,  /*  pin11, pad name: GPIO_34/PCM_CLK  */
    SC_MODULE_GPIO_35 = 13,  /*  pin12, pad name: GPIO_35/PCM_SYNC  */
    SC_MODULE_GPIO_36 = 15,  /*  pin13, pad name: GPIO_36/PCM_DIN  */
    SC_MODULE_GPIO_37 = 14,  /*  pin14, pad name: GPIO_37/PCM_DOUT  */

    SC_MODULE_GPIO_38 = 50,  /*  pin37, pad name: GPIO_38/CI2C_SDA  */
    SC_MODULE_GPIO_39 = 49,  /*  pin38, pad name:  GPIO_39/CI2C_SCL  */
#ifdef FEATURE_SIMCOM_NODTR
    SC_MODULE_GPIO_40 = 6,  /*  pin3, pad name: DTR  */
#endif

    SC_MODULE_GPIO_MAX = 128,
} SC_Module_GPIONumbers;

#ifdef __cplusplus
}
#endif

#endif
