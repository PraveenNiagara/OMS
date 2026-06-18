#ifndef __A7682EC_V102_GPIO_H__
#define __A7682EC_V102_GPIO_H__

typedef enum
{

    SC_MODULE_GPIO_01 = 53,   /*  pin 3,  pad name: GPIO_01 */
    SC_MODULE_GPIO_02 = 54,   /*  pin 4,  pad name: GPIO_02 */
    SC_MODULE_GPIO_03 = 32,   /*  pin 5,  pad name: GPIO_03 */
#ifdef FEATURE_SIMCOM_NODTR
    SC_MODULE_GPIO_04 = 27,   /*  pin 6,  pad name: DTR     */
#endif
#ifdef FEATURE_SIMCOM_NORI
    SC_MODULE_GPIO_05 = 31,   /*  pin 7,  pad name: RI      */
#endif
    SC_MODULE_GPIO_06 = 28,   /*  pin 41, pad name: GPIO_06 */
    SC_MODULE_GPIO_07 = 8,    /*  pin 42, pad name: GPIO_07 */
    SC_MODULE_GPIO_08 = 01,   /*  pin 57, pad name: GPIO_08 */
    SC_MODULE_GPIO_09 = 00,   /*  pin 58, pad name: GPIO_09 */
    SC_MODULE_GPIO_10 = 16,   /*  pin 78, pad name: GPIO_10 */
    SC_MODULE_GPIO_11 = 17,   /*  pin 79, pad name: GPIO_11 */
    SC_MODULE_GPIO_12 = 18,   /*  pin 78, pad name: GPIO_12 */
    SC_MODULE_GPIO_13 = 19,   /*  pin 79, pad name: GPIO_13 */
    SC_MODULE_GPIO_14 = 9,    /*  pin 82, pad name: GPIO_14 */

    SC_MODULE_GPIO_MAX = 128,
} SC_Module_GPIONumbers;

#endif