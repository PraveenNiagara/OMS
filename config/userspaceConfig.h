#ifndef __USERSPACECONFIG_H__
#define __USERSPACECONFIG_H__


// general config
#define HAS_UART
#define HAS_URC
#define HAS_USB
/* #undef SIMCOM_UI_DEMO_TO_UART1_PORT */
/* #undef SIMCOM_UI_DEMO_TO_USB_AT_PORT */
/* #undef SIMCOM_API_TEST */


/* #undef SIMCOM_DEBUG */
/* #undef HAS_DEMO */
#define SIMCOM_PROTOCOL_V1
/* #undef SIMCOM_PROTOCOL_V2 */

// driver
/* #undef HAS_DEMO_WTD */
/* #undef HAS_DEMO_PMU */
/* #undef HAS_DEMO_GPIO */
/* #undef HAS_DEMO_PWM */
/* #undef HAS_DEMO_UART */
/* #undef HAS_DEMO_USB */
/* #undef HAS_DEMO_I2C */
/* #undef HAS_DEMO_SPI */
/* #undef HAS_DEMO_FLASH */
/* #undef HAS_DEMO_OTA */
/* #undef HAS_DEMO_GNSS */
/* #undef HAS_DEMO_LCD */
/* #undef HAS_DEMO_CAM */
/* #undef HAS_DEMO_SYS */
/* #undef HAS_DEMO_EXFLASHMOUNTFS */
/* #undef HAS_DEMO_ONEWIRE */

// modem
/* #undef HAS_DEMO_NETWORK */
/* #undef HAS_DEMO_SIMCARD */
/* #undef HAS_DEMO_CALL */
/* #undef HAS_DEMO_SMS */
/* #undef HAS_DEMO_LBS */
/* #undef HAS_DEMO_SJDR */
/* #undef HAS_DEMO_PPPD */
/* #undef HAS_DEMO_INTERNET_SERVICE */

// app
/* #undef HAS_DEMO_SSL */
#define HAS_TCPIP
/* #undef HAS_DEMO_TCPIP */
/* #undef HAS_DEMO_HTTPS */
/* #undef HAS_DEMO_FTPS */
/* #undef HAS_DEMO_MQTTS */
/* #undef HAS_DEMO_NTP */
/* #undef HAS_DEMO_HTP */
/* #undef HAS_DEMO_AUDIO */
/* #undef HAS_DEMO_TTS */
/* #undef HAS_DEMO_POC */
/* #undef HAS_DEMO_WIFISCAN */
/* #undef HAS_DEMO_WIFI_AP */
/* #undef HAS_DEMO_RTC */
/* #undef HAS_DEMO_BLE */
/* #undef HAS_DEMO_BT */
/* #undef HAS_DEMO_BT_STACK */
#define HAS_FS
/* #undef HAS_DEMO_FS */
/* #undef HAS_DEMO_FS_OLD */
/* #undef HAS_DEMO_APP_DOWNLOAD */
/* #undef HAS_DEMO_APP_UPDATE */
/* #undef HAS_DEMO_PING */

// other libs
/* #undef HAS_ZLIB */
/* #undef HAS_MIRACL */
/* #undef HAS_SM2 */
/* #undef HAS_CJSON */
/* #undef HAS_MBEDTLS */
#define HAS_CRYPTO


// custom switch
/* #undef PROJECT_YSZNBG */
/* #undef PROJECT_TBDTU */
/* #undef PROJECT_LYTBOX */
/* #undef PROJECT_TOPFLY */
/* #undef PROJECT_KLYDTU */
/* #undef PROJECT_YSZNBG_BSKL */

#endif /* __USERSPACECONFIG.H_H__ */
