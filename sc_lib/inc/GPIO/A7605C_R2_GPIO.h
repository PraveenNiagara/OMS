#ifndef __A7605C_R2_GPIO_H__
#define __A7605C_R2_GPIO_H__


#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
#ifndef FEATURE_SIMCOM_WIFI_AP
    SC_MODULE_GPIO_00 = 117,    /*  pin 1,      pad name: GPIO_117  */
    SC_MODULE_GPIO_01 = 120,    /*  pin 2,      pad name: GPIO_120  */
    SC_MODULE_GPIO_02 = 118,    /*  pin 4,      pad name: GPIO_118  */
    SC_MODULE_GPIO_03 = 123,    /*  pin 5,      pad name: GPIO_123  */
    SC_MODULE_GPIO_04 = 23,     /*  pin 62,     pad name: GPIO_23   */
    SC_MODULE_GPIO_05 = 11,     /*  pin 63,     pad name: GPIO_11   */
    SC_MODULE_GPIO_06 = 125,    /*  pin 139,    pad name: GPIO_125  */
    SC_MODULE_GPIO_07 = 48,     /*  pin 129,    pad name: GPIO_48   */
    SC_MODULE_GPIO_08 = 55,     /*  pin 130,    pad name: GPIO_55   */
    SC_MODULE_GPIO_09 = 56,     /*  pin 131,    pad name: GPIO_56   */
    SC_MODULE_GPIO_10 = 57,     /*  pin 132,    pad name: GPIO_57   */
    SC_MODULE_GPIO_11 = 59,     /*  pin 133,    pad name: GPIO_59   */
    SC_MODULE_GPIO_12 = 58,     /*  pin 134,    pad name: GPIO_58   */
    SC_MODULE_GPIO_13 = 122,    /*  pin 118,    pad name: GPIO_122  */
    SC_MODULE_GPIO_14 = 24,     /*  pin 127,    pad name: GPIO_24   */
    SC_MODULE_GPIO_15 = 21,     /*  pin 135,    pad name: GPIO_21   */
    SC_MODULE_GPIO_16 = 22,     /*  pin 136,    pad name: GPIO_22   */
#ifdef NOPHY
    SC_MODULE_GPIO_17 = 2,      /*  pin 73,     pad name: GPIO_2    */
    SC_MODULE_GPIO_18 = 0,      /*  pin 74,     pad name: GPIO_0    */
    SC_MODULE_GPIO_19 = 3,      /*  pin 75,     pad name: GPIO_3    */
    SC_MODULE_GPIO_20 = 1,      /*  pin 76,     pad name: GPIO_1    */
    SC_MODULE_GPIO_21 = 6,      /*  pin 77,     pad name: GPIO_6    */
    SC_MODULE_GPIO_22 = 7,      /*  pin 78,     pad name: GPIO_7    */
    SC_MODULE_GPIO_25 = 15,     /*  pin 81,     pad name: GPIO_15   */
    SC_MODULE_GPIO_29 = 18,     /*  pin 120,    pad name: GPIO_18   */
    SC_MODULE_GPIO_30 = 17,     /*  pin 121,    pad name: GPIO_17   */
    SC_MODULE_GPIO_31 = 16,     /*  pin 122,    pad name: GPIO_16   */
#endif
    SC_MODULE_GPIO_23 = 4,      /*  pin 79,     pad name: GPIO_4    */
    SC_MODULE_GPIO_24 = 13,     /*  pin 80,     pad name: GPIO_13   */
    SC_MODULE_GPIO_26 = 5,      /*  pin 82,     pad name: GPIO_5    */
    SC_MODULE_GPIO_27 = 12,     /*  pin 83,     pad name: GPIO_12   */
    SC_MODULE_GPIO_28 = 14,     /*  pin 84,     pad name: GPIO_14   */
    SC_MODULE_GPIO_32 = 99,     /*  pin 119,    pad name: GPIO_99   */
    SC_MODULE_GPIO_33 = 126,    /*  pin 3,      pad name: GPIO_126  */
    SC_MODULE_GPIO_MAX = 128,
#else
    SC_MODULE_GPIO_00 = 117,        /*  pin 1,   pad name: GPIO_117    */
    SC_MODULE_GPIO_01 = 120,        /*  pin 2,   pad name: GPIO_120   */
    SC_MODULE_GPIO_02 = 118,        /*  pin 4,   pad name: GPIO_118   */
    SC_MODULE_GPIO_03 = 123,        /*  pin 5,   pad name: GPIO_123   */
    SC_MODULE_GPIO_04 = 23,        /*  pin 62,   pad name: GPIO_23   */
    SC_MODULE_GPIO_05 = 11,        /*  pin 63,   pad name: GPIO_11   */
    SC_MODULE_GPIO_06 = 125,        /*  pin 139,   pad name: GPIO_125   */
    SC_MODULE_GPIO_MAX = 128,
#endif
} SC_Module_GPIONumbers;

#ifdef __cplusplus
}
#endif

#endif
