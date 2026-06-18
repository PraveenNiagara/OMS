#ifndef __A7605C_V401_GPIO_H__
#define __A7605C_V401_GPIO_H__

typedef enum
{

    SC_MODULE_GPIO_01 = 117,   /*  pin 1,  pad name: GPIO_117 */
    SC_MODULE_GPIO_02 = 120,   /*  pin 2,  pad name: GPIO_120 */
    SC_MODULE_GPIO_03 = 33,   /*  pin 3,  pad name: GPIO_33 */
    SC_MODULE_GPIO_04 = 43,   /*  pin 23, pad name: GPIO_43 */
    SC_MODULE_GPIO_05 = 28,   /*  pin 24, pad name: GPIO_28 */
    SC_MODULE_GPIO_06 = 27,   /*  pin 25, pad name: GPIO_27 */
    SC_MODULE_GPIO_07 = 26,    /*  pin 26, pad name: GPIO_26 */
    SC_MODULE_GPIO_08 = 25,   /*  pin 27, pad name: GPIO_25 */
    SC_MODULE_GPIO_09 = 20,   /*  pin 61, pad name: GPIO_20 */
    SC_MODULE_GPIO_10 = 23,   /*  pin 62, pad name: GPIO_23 */
    SC_MODULE_GPIO_11 = 50,   /*  pin 63, pad name: GPIO_50 */
    SC_MODULE_GPIO_12 = 32,   /*  pin 64, pad name: GPIO_32 */
    SC_MODULE_GPIO_13 = 31,   /*  pin 65, pad name: GPIO_31 */
#ifdef FEATURE_SIMCOM_NODTR
    SC_MODULE_GPIO_14 = 49,    /*  pin 66, pad name: GPIO_49 */
#endif
    SC_MODULE_GPIO_15 = 122,   /*  pin 118, pad name: GPIO_122 */
    SC_MODULE_GPIO_16 = 21,   /*  pin 135, pad name: GPIO_21 */
    SC_MODULE_GPIO_17 = 22,   /*  pin 136, pad name: GPIO_22 */
    SC_MODULE_GPIO_18 = 24,   /*  pin 127, pad name: GPIO_24 */
    SC_MODULE_GPIO_19 = 48,   /*  pin 129, pad name: GPIO_48 */
    SC_MODULE_GPIO_20 = 55,   /*  pin 130, pad name: GPIO_55 */
    SC_MODULE_GPIO_21 = 56,   /*  pin 131, pad name: GPIO_56 */
    SC_MODULE_GPIO_22 = 57,   /*  pin 132, pad name: GPIO_57 */
    SC_MODULE_GPIO_23 = 58,   /*  pin 134, pad name: GPIO_58 */
    SC_MODULE_GPIO_24 = 59,   /*  pin 133, pad name: GPIO_59 */

    SC_MODULE_GPIO_MAX = 128,
} SC_Module_GPIONumbers;

#endif