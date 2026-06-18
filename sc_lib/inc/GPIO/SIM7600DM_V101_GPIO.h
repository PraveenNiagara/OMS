#ifndef __SIM7600DM_V101_GPIO_H__
#define __SIM7600DM_V101_GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    SC_MODULE_GPIO_01 = 4,   /*  pin6,  pad name: GPIO_01 */
    SC_MODULE_GPIO_02 = 28,  /*  pin19, pad name: GPIO_02 */
    SC_MODULE_GPIO_03 = 32,  /*  pin21, pad name: GPIO_03 */
    SC_MODULE_GPIO_04 = 31,  /*  pin22, pad name: GPIO_04 */
    SC_MODULE_GPIO_05 = 53,  /*  pin25, pad name: GPIO_05 */
    SC_MODULE_GPIO_06 = 54,  /*  pin26, pad name: GPIO_06 */
    SC_MODULE_GPIO_07 = 25,  /*  pin32, pad name: GPIO_07 */
    SC_MODULE_GPIO_08 = 26 , /*  pin33, pad name: GPIO_08 */
    SC_MODULE_GPIO_09 = 49,   /* pin42, pad name: GPIO_09 */
    SC_MODULE_GPIO_10 = 50,  /*  pin43, pad name: GPIO_10 */
    SC_MODULE_GPIO_11 = 11,  /*  pin53, pad name: GPIO_11 */
    SC_MODULE_GPIO_12 = 10,  /*  pin54, pad name: GPIO_12 */
    SC_MODULE_GPIO_13 = 36,  /*  pin55, pad name: GPIO_13 */
    SC_MODULE_GPIO_14 = 35,  /*  pin56, pad name: GPIO_14 */
    SC_MODULE_GPIO_15 = 34,  /*  pin57, pad name: GPIO_15 */
    SC_MODULE_GPIO_16 = 33,  /*  pin58, pad name: GPIO_16 */
    SC_MODULE_GPIO_17 = 18,  /*  pin59, pad name: GPIO_17 */
    SC_MODULE_GPIO_18 = 19 , /*  pin60, pad name: GPIO_18 */
    SC_MODULE_GPIO_19 = 17,   /* pin61, pad name: GPIO_19 */
    SC_MODULE_GPIO_20 = 16,  /*  pin62, pad name: GPIO_20 */

    SC_MODULE_GPIO_MAX = 128,
} SC_Module_GPIONumbers;

#ifdef __cplusplus
}
#endif

#endif
