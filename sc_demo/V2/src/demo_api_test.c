#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "demo_api_test.h"
#include "demo_protocol_parser.h"
#include "simcom_demo_init.h"
#include "sal_os.h"
#include "sal_log.h"
#include "pub_string.h"
#include "pub_cache.h"

#include "simcom_common.h"
#include "simcom_os.h"
#ifdef FEATURE_SIMCOM_AUDIO
#include "simcom_audio.h"
#endif
#if defined(FEATURE_SIMCOM_TTS_YOUNGTONE) || defined(FEATURE_SIMCOM_TTS_IFLY)
#include "simcom_tts_api.h"
#endif
#include "simcom_wifi.h"
#ifdef FEATURE_SIMCOM_FS
#include "simcom_file.h"
#endif
#include "simcom_tcpip_old.h"
#include "simcom_tcpip.h"
#include "simcom_https.h"
#ifdef FEATURE_SIMCOM_HTP
#include "simcom_htp_client.h"
#endif
#ifdef FEATURE_SIMCOM_NTP
#include "simcom_ntp_client.h"
#endif
#ifdef FEATURE_SIMCOM_SSL
#include "simcom_ssl.h"
#include "pl_crypto.h"
#endif
#ifdef FEATURE_SIMCOM_MQTT
#include "simcom_mqtts_client.h"
#endif
#ifdef HAS_DEMO_FTPS
#include "simcom_ftps.h"
#endif

#include "simcom_gpio.h"
#include "sc_power.h"
#include "simcom_fota_download.h"
#include "demo_fota.h"
#include "sc_app_download.h"
#include "sc_flash.h"
#include "simcom_fota_download.h"
#include "sc_app_update.h"
#include "simcom_uart.h"
#include "sc_i2c.h"
#include "sc_spi.h"
#include "simcom_debug.h"
#include "simcom_system.h"
#include "simcom_rtc.h"
#include "simcom_gps.h"
#ifdef FEATURE_SIMCOM_POC
#include "simcom_poc.h"
#endif

#define TEST_LOG_ERROR(...) sal_log_error("[DEMO-APITEST] " __VA_ARGS__)
#define TEST_LOG_INFO(...)  sal_log_info("[DEMO-APITEST] " __VA_ARGS__)
#define TEST_LOG_TRACE(...) sal_log_trace("[DEMO-APITEST] " __VA_ARGS__)

#define MAX(a, b) ((a) > (b) ? (a) : (b))

char *g_mallocTestData = NULL;

ret_msg_t rtos_msgQCreate_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t rtos_msgQDelete_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t rtos_msgQPoll_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t rtos_msgQRecv_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t rtos_msgQSend_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t rtos_msgQSendSuspend_cmd_proc(op_t op, arg_t *argv, int argn);

ret_msg_t set_fotabynet_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t app_download_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t fbf_disable_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t get_fota_status_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t write_magic_code_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t rtos_enableDump_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t rtos_disableDump_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t rtos_getTicks_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t rtos_getSysVersion_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t rtos_sysPowerOff_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t rtos_sysReset_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t rtos_sysGetLocalTime_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t rtos_sysSleepSet_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t rtos_sysSleepGet_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t rtos_getRealTimeClock_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t rtos_setRealTimeClock_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t rtos_getTimeofDay_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t rtos_setDelayUs_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t rtos_malloc_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t rtos_free_cmd_proc(op_t op, arg_t *argv, int argn);

ret_msg_t rtos_mutexCreate_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t rtos_mutexLock_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t rtos_mutexUnlock_cmd_proc(op_t op, arg_t *argv, int argn);

ret_msg_t rtos_semaCreate_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t rtos_semaDelete_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t rtos_semaAcquire_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t rtos_semaRelease_cmd_proc(op_t op, arg_t *argv, int argn);

ret_msg_t rtos_taskCreate_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t rtos_taskDelete_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t rtos_taskSuspend_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t rtos_taskChangePriority_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t rtos_taskGetEntryParam_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t rtos_taskGetPriority_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t rtos_taskGetCurrentRef_cmd_proc(op_t op, arg_t *argv, int argn);

ret_msg_t rtos_timerCreate_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t rtos_timerDelete_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t rtos_timerStart_cmd_proc(op_t op, arg_t *argv, int argn);
#ifdef SIMCOM_ACC_TIMER_SUPPORT
ret_msg_t rtos_accTimerStart_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t rtos_accTimerStop_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t rtos_accTimerDelete_cmd_proc(op_t op, arg_t *argv, int argn);
#endif

#ifdef FEATURE_SIMCOM_AUDIO
ret_msg_t audio_play_file_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t audio_stop_file_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t audio_record_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t audio_set_amr_rate_level_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t audio_rec_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t audio_play_sample_rate_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t audio_pcm_play_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t audio_pcm_stop_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t audio_mp3_stream_play_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t audio_mp3_stream_stop_cmd_proc(op_t op, arg_t *argv, int argn); //
ret_msg_t audio_amr_stream_play_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t audio_amr_stream_stop_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t audio_play_amr_cont_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t audio_wav_file_play_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t audio_status_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t audio_set_play_path_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t audio_set_mic_gain_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t audio_get_mic_gain_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t audio_set_volume_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t audio_get_volume_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t audio_pcm_stream_rec_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t audio_pcm_stream_stop_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t audio_amr_stream_rec_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t audio_amr_stream_stop_record_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t audio_play_mp3_cont_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t audio_play_wav_cont_cmd_proc(op_t op, arg_t *argv, int argn);
#endif


#if defined(FEATURE_SIMCOM_TTS_YOUNGTONE) || defined(FEATURE_SIMCOM_TTS_IFLY)
ret_msg_t tts_play_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t tts_setParam_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t tts_stop_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t tts_setStatusCb_cmd_proc(op_t op, arg_t *argv, int argn);
#endif

#ifdef FEATURE_SIMCOM_POC
static ret_msg_t poc_init(void);
static ret_msg_t poc_uninit(void);
static ret_msg_t poc_play_sound_raw_proc(int size, int (*raw_read)(char *buf, unsigned int buf_size, int timeout_ms));
static ret_msg_t poc_play_sound_cmd_proc(op_t op, arg_t *argv, int argn);
static int poc_play_sound_need_size(void);
static ret_msg_t poc_stop_sound_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t poc_start_record_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t poc_stop_record_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t poc_getPcmAvailSize_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t poc_CleanBufferData_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t poc_PcmRead_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t poc_TonePlay_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t poc_ToneStop_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t poc_FreqPlay_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t poc_SetToneVolume_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t poc_getToneVolume_cmd_proc(op_t op, arg_t *argv, int argn);
#endif

ret_msg_t wifiscan_setParam_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t wifiscan_start_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t wifiscan_stop_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t wifiscan_setHandler_cmd_proc(op_t op, arg_t *argv, int argn);

#ifdef FEATURE_SIMCOM_FS
ret_msg_t fs_fopen_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t fs_fclose_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t fs_fread_cmd_proc(op_t op, arg_t *argv, int argn);
int write_file_need_size(void);
ret_msg_t write_file_init(void);
ret_msg_t write_file_uninit(void);
ret_msg_t write_file_raw_proc(int size, int (*raw_read)(char *buf, unsigned int buf_size, int timeout_ms));
ret_msg_t fs_fwrite_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t fs_fsync_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t fs_fseek_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t fs_fstat_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t fs_ftell_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t fs_fsize_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t fs_rename_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t fs_remove_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t fs_mkdir_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t fs_opendir_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t fs_closedir_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t fs_readdir_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t fs_get_size_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t fs_get_free_size_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t fs_get_used_size_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t fs_switch_dir_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t fs_format_cmd_proc(op_t op, arg_t *argv, int argn);
#endif

ret_msg_t tcpip_init_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t tcpip_socket_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t tcpip_bind_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t tcpip_listen_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t tcpip_accept_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t tcpip_select_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t tcpip_recv_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t tcpip_connect_cmd_proc(op_t op, arg_t *argv, int argn);
int tcpip_send_need_size(void);
ret_msg_t tcpip_send_init(void);
ret_msg_t tcpip_send_uninit(void);
ret_msg_t tcpip_send_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t tcpip_send_raw_proc(int size, int (*raw_read)(char *buf, unsigned int buf_size, int timeout_ms));
void tcpip_send_task(void *args);
ret_msg_t tcpip_setsockopt_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t tcpip_getsockopt_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t tcpip_close_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t tcpip_deinit_cmd_proc(op_t op, arg_t *argv, int argn);

ret_msg_t httpsinit_cmd(op_t op,arg_t *argv, int argn);
ret_msg_t httpspara_cmd(op_t op,arg_t *argv, int argn);
int httpsdata_need_size(void);
ret_msg_t httpsdata_uninit(void);
ret_msg_t httpsdata_init(void);
ret_msg_t httpsdata_cmd(op_t op,arg_t *argv, int argn);
ret_msg_t httpsdata_raw(int size , int (*raw_read)(char *buf , unsigned int buf_size, int timeout_ms));
void show_httpaction_urc(ret_msg_t *ret);
ret_msg_t httpsaction_cmd(op_t op,arg_t *argv, int argn);
void show_httphead_urc( ret_msg_t *ret,int type);
ret_msg_t httpshead_cmd(op_t op,arg_t *argv, int argn);
ret_msg_t httpsread_cmd(op_t op,arg_t *argv, int argn);
ret_msg_t httpsterm_cmd(op_t op,arg_t *argv, int argn);

#ifdef FEATURE_SIMCOM_HTP
ret_msg_t htp_config_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t htp_update_cmd_proc(op_t op,arg_t *argv, int argn);
#endif

#ifdef FEATURE_SIMCOM_NTP
ret_msg_t ntp_update_cmd_proc(op_t op,arg_t *argv, int argn);
#endif

#ifdef FEATURE_SIMCOM_SSL
ret_msg_t ssl_set_context_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t ssl_get_context_cmd_proc(op_t op,arg_t *argv, int argn);

ret_msg_t crypto_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t crypto_create_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t crypto_reset_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t crypto_delete_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t crypto_cfg_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t crypto_get_cfg_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t crypto_random_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t crypto_sslTlsPrf_cmd_proc(op_t op,arg_t *argv, int argn);
#endif

#ifdef FEATURE_SIMCOM_MQTT
ret_msg_t mqtt_start_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t mqtt_accq_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t mqtt_connLostCb_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t mqtt_connect_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t mqtt_disconnect_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t mqtt_sub_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t mqtt_unsub_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t mqtt_topic_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t mqtt_payload_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t mqtt_pub_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t mqtt_rel_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t mqtt_stop_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t mqtt_willtopic_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t mqtt_willmsg_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t mqtt_subtopic_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t mqtt_unsubtopic_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t mqtt_sslcfg_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t mqtt_cfg_cmd_proc(op_t op,arg_t *argv, int argn);
#endif

#ifdef HAS_DEMO_FTPS
ret_msg_t ftps_init_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t ftps_deinit_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t ftps_login_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t ftps_logout_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t ftps_downloadFile_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t ftps_downloadFileToBuffer_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t ftps_uploadFile_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t ftps_putFile_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t ftps_deleteFile_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t ftps_createDir_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t ftps_deleteDir_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t ftps_changeDir_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t ftps_getCurDir_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t ftps_list_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t ftps_getFileSize_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t ftps_transferType_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t ftps_getTransferType_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t ftps_sslConfig_cmd_proc(op_t op,arg_t *argv, int argn);
ret_msg_t ftps_setMode_cmd_proc(op_t op,arg_t *argv, int argn);
#endif

ret_msg_t get_gpio_level_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t set_gpio_interrupt_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t set_gpio_config_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t set_gpio_wakeupenable_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t set_gpio_level_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t set_gpio_pinmux_func_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t get_adc_value_cmd_proc(op_t op, arg_t *argv, int argn);

ret_msg_t uartRead_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t uartWrite_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t uartCtl_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t uartModeCtl_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t uartConfigSet_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t uartRs485DeAssign_cmd_proc(op_t op, arg_t *argv, int argn);

ret_msg_t i2c_init_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t i2c_deinit_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t i2c_write_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t i2c_read_cmd_proc(op_t op, arg_t *argv, int argn);

ret_msg_t spiinit_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t spireadid_cmd_proc(op_t op, arg_t *argv, int argn);
ret_msg_t spirwstatus_cmd_proc(op_t op, arg_t *argv, int argn);

static ret_msg_t gnss_pwr_set_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t gnss_pwr_get_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t gnss_nmea_data_get_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t gnss_start_mode_set_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t gnss_baud_rate_set_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t gnss_baud_rate_get_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t gnss_mode_set_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t gnss_mode_get_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t gnss_nmea_rate_get_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t gnss_nmea_rate_set_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t gnss_nmea_sentence_get_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t gnss_nmea_sentence_set_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t gnss_and_gps_info_get_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t gnss_cmd_send_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t gnss_agps_cmd_proc(op_t op, arg_t *argv, int argn);

arg_t create_msgq_args[4] = 
{
    {
        .type = TYPE_RAW_GBK,
        .arg_name = ARG_MSGQ_NAME,
        .msg = "queue name string"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_MSGQ_MAXSIZE,
        .msg = "data size"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_MSGQ_NUMBER,
        .msg = "msgQ number"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_MSGQ_WAITINGMODE,
        .msg = "msgQ waiting mode like SC_FIFO"
    }
};

const menu_list_t func_msgQ_create = 
{
    .menu_name = "create msgQ",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = create_msgq_args,
        .argn = sizeof(create_msgq_args) / sizeof(arg_t),
        .methods.cmd_handler = rtos_msgQCreate_cmd_proc,
    }
};

const menu_list_t func_msgQ_delete = 
{
    .menu_name = "delete msgQ",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = rtos_msgQDelete_cmd_proc,
    }
};

const menu_list_t func_msgQ_poll = 
{
    .menu_name = "poll msgQ",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = rtos_msgQPoll_cmd_proc,
    }
};

arg_t recv_msgq_args[1] = 
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_MSGQ_RECV_TIMEOUT,
        .msg = "recv timeout"
    }
};

const menu_list_t func_msgQ_recv = 
{
    .menu_name = "create msgQ",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = recv_msgq_args,
        .argn = sizeof(recv_msgq_args) / sizeof(arg_t),
        .methods.cmd_handler = rtos_msgQRecv_cmd_proc,
    }
};

arg_t send_msgq_args[3] = 
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_MSGQ_SEND_MSGID,
        .msg = "send msg_id(int)"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_MSGQ_SEND_ARG1,
        .msg = "send arg1(int)"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_MSGQ_SEND_ARG2,
        .msg = "send arg2(int)"
    }
};

const menu_list_t func_msgQ_send = 
{
    .menu_name = "send msgQ",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = send_msgq_args,
        .argn = sizeof(send_msgq_args) / sizeof(arg_t),
        .methods.cmd_handler = rtos_msgQSend_cmd_proc,
    }
};

arg_t sendSuspend_msgq_args[4] = 
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_MSGQ_SEND_MSGID,
        .msg = "msg_id(int)"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_MSGQ_SEND_ARG1,
        .msg = "arg1(int)"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_MSGQ_SEND_ARG2,
        .msg = "arg2(int)"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_MSGQ_SEND_TIMEOUT,
        .msg = "timeout(int)"
    }
};

const menu_list_t func_msgQ_sendSuspend = 
{
    .menu_name = "msgQ send suspend",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = sendSuspend_msgq_args,
        .argn = sizeof(sendSuspend_msgq_args) / sizeof(arg_t),
        .methods.cmd_handler = rtos_msgQSendSuspend_cmd_proc,
    }
};
//FOTA API
arg_t fotaUpgrade_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGFOTA_MODE,
    },
    {
        .type = TYPE_STR,
        .arg_name = UPGRADE_URL,
    },
    {
        .type = TYPE_STR,
        .arg_name = UPGRADE_USERNAME,
    },
    {
        .type = TYPE_STR,
        .arg_name = UPGRADE_PASSWORD,
    }
};

const menu_list_t func_fota_start =
{
    .menu_name = "fota upgrade by net",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = fotaUpgrade_args,
        .argn = sizeof(fotaUpgrade_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = set_fotabynet_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

arg_t app_dl_args[]=
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_APP_MODE,
    },
    {
        .type = TYPE_STR,
        .arg_name = ARG_APP_DL_URL,
        .msg = "url e.g (http://servername:port/path)"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_APP_DL_RECVTIOMEOUT,
    }
};

const menu_list_t func_app_download =
{
    .menu_name ="app download",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = app_dl_args,
        .argn = sizeof(app_dl_args)/sizeof(arg_t),
        .methods.cmd_handler = app_download_cmd_proc,
    }
};

const menu_list_t func_fbf_disable =
{
    .menu_name ="fbf disable",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .methods.cmd_handler = fbf_disable_cmd_proc,
    }
};

const menu_list_t func_get_fota_status =
{
    .menu_name ="get fota status",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .methods.cmd_handler = get_fota_status_cmd_proc,
    }
};

const menu_list_t func_write_magic_code =
{
    .menu_name ="write magic code",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .methods.cmd_handler = write_magic_code_cmd_proc,
    }
};

const menu_list_t func_enableDump =
{
    .menu_name = "enable dump",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = rtos_enableDump_cmd_proc,
    }
};

const menu_list_t func_disableDump =
{
    .menu_name = "disable dump",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = rtos_disableDump_cmd_proc,
    }
};

const menu_list_t func_getTicks =
{
    .menu_name = "get ticks",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = rtos_getTicks_cmd_proc,
    }
};

const menu_list_t func_sys_getVersion =
{
    .menu_name = "get system version",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = rtos_getSysVersion_cmd_proc,
    }
};

const menu_list_t func_sys_powerOff =
{
    .menu_name = "power off",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = rtos_sysPowerOff_cmd_proc,
    }
};

const menu_list_t func_sys_reset =
{
    .menu_name = "system reset",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = rtos_sysReset_cmd_proc,
    }
};

#ifdef FEATURE_SIMCOM_NTP
const menu_list_t func_sys_getLocalTime =
{
    .menu_name = "get system local time",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = rtos_sysGetLocalTime_cmd_proc,
    }
};
#endif

static arg_t sleepCtrl_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_SLEEP_SELECT,
        .msg = "flag(1:sleep  0:wakeup)"
    }
};


const menu_list_t func_sys_sleepSet =
{
    .menu_name = "set system sleep",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = sleepCtrl_args,
        .argn = sizeof(sleepCtrl_args)/sizeof(arg_t),
        .methods.cmd_handler = rtos_sysSleepSet_cmd_proc,
    }
};

const menu_list_t func_sys_sleepGet =
{
    .menu_name = "get system sleep",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = rtos_sysSleepGet_cmd_proc,
    }
};

const menu_list_t func_getRealTimeClock =
{
    .menu_name = "get real time",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = rtos_getRealTimeClock_cmd_proc,
    }
};

static arg_t realTimeCtrl_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_RTC_TIME_YEAR,
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_RTC_TIME_MON,
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_RTC_TIME_DAY,
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_RTC_TIME_HOUR,
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_RTC_TIME_MIN,
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_RTC_TIME_SEC,
    }
};

const menu_list_t func_setRealTimeClock =
{
    .menu_name = "set real time",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = realTimeCtrl_args,
        .argn = sizeof(realTimeCtrl_args)/sizeof(arg_t),
        .methods.cmd_handler = rtos_setRealTimeClock_cmd_proc,
    }
};

const menu_list_t func_gettimeofday =
{
    .menu_name = "get time of day",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = rtos_getTimeofDay_cmd_proc,
    }
};

static arg_t delayUsCtrl_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_TIME_USEC,
    }
};

const menu_list_t func_delayUs =
{
    .menu_name = "set microsecond delay",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = delayUsCtrl_args,
        .argn = sizeof(delayUsCtrl_args)/sizeof(arg_t),
        .methods.cmd_handler = rtos_setDelayUs_cmd_proc,
    }
};

static arg_t mallocSizeCtrl_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_MALLOC_SIZE,
    }
};

const menu_list_t func_malloc =
{
    .menu_name = "malloc memory",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = mallocSizeCtrl_args,
        .argn = sizeof(mallocSizeCtrl_args)/sizeof(arg_t),
        .methods.cmd_handler = rtos_malloc_cmd_proc,
    }
};

const menu_list_t func_free =
{
    .menu_name = "free memory",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = rtos_free_cmd_proc,
    }
};

arg_t create_mutex_args[1] = 
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_MSGQ_WAITINGMODE,
        .msg = "waiting mode like SC_FIFO"
    }
};

const menu_list_t func_mutex_create = 
{
    .menu_name = "mutex create",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = create_mutex_args,
        .argn = sizeof(create_mutex_args) / sizeof(arg_t),
        .methods.cmd_handler = rtos_mutexCreate_cmd_proc,
    }
};

arg_t lock_mutex_args[1] = 
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_MSGQ_SEND_TIMEOUT,
        .msg = "timeout"
    }
};

const menu_list_t func_mutex_lock = 
{
    .menu_name = "mutex lock",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = lock_mutex_args,
        .argn = sizeof(lock_mutex_args) / sizeof(arg_t),
        .methods.cmd_handler = rtos_mutexLock_cmd_proc,
    }
};

const menu_list_t func_mutex_unlock = 
{
    .menu_name = "mutex unlock",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = rtos_mutexUnlock_cmd_proc,
    }
};

arg_t create_sema_args[2] = 
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_SEMA_INITIALCOUT,
        .msg = "initial count(int)"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_MSGQ_WAITINGMODE,
        .msg = "waiting mode"
    }
};

const menu_list_t func_sema_create = 
{
    .menu_name = "mutex lock",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = create_sema_args,
        .argn = sizeof(create_sema_args) / sizeof(arg_t),
        .methods.cmd_handler = rtos_semaCreate_cmd_proc,
    }
};

const menu_list_t func_sema_delete = 
{
    .menu_name = "sema delete",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = rtos_semaDelete_cmd_proc,
    }
};

arg_t acquire_sema_args[1] = 
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_MSGQ_SEND_TIMEOUT,
        .msg = "timeout"
    }
};

const menu_list_t func_sema_acquire = 
{
    .menu_name = "mutex lock",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = acquire_sema_args,
        .argn = sizeof(acquire_sema_args) / sizeof(arg_t),
        .methods.cmd_handler = rtos_semaAcquire_cmd_proc,
    }
};

const menu_list_t func_sema_release = 
{
    .menu_name = "sema delete",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = rtos_semaRelease_cmd_proc,
    }
};

arg_t create_task_args[3] = 
{
    {
        .type = TYPE_RAW_GBK,
        .arg_name = ARG_TASK_NAME,
        .msg = "task name"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_TASK_STACKSIZE,
        .msg = "task stack size"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_TASK_PRIORITY,
        .msg = "task priority"
    }
};

const menu_list_t func_task_create = 
{
    .menu_name = "task create",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = create_task_args,
        .argn = sizeof(create_task_args) / sizeof(arg_t),
        .methods.cmd_handler = rtos_taskCreate_cmd_proc,
    }
};

const menu_list_t func_task_delete = 
{
    .menu_name = "task delete",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = rtos_taskDelete_cmd_proc,
    }
};

const menu_list_t func_task_suspend = 
{
    .menu_name = "task suspend",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = rtos_taskSuspend_cmd_proc,
    }
};

arg_t changePrio_task_args[1] = 
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_TASK_PRIORITY,
        .msg = "task priority"
    }
};

const menu_list_t func_task_changePrio = 
{
    .menu_name = "task change priority",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = changePrio_task_args,
        .argn = sizeof(changePrio_task_args) / sizeof(arg_t),
        .methods.cmd_handler = rtos_taskChangePriority_cmd_proc,
    }
};

const menu_list_t func_task_getEntryParam = 
{
    .menu_name = "task get entry param",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = rtos_taskGetEntryParam_cmd_proc,
    }
};

const menu_list_t func_task_getPrio = 
{
    .menu_name = "task priority",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = rtos_taskGetPriority_cmd_proc,
    }
};

const menu_list_t func_task_getCurRef = 
{
    .menu_name = "task get current task_ref",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = rtos_taskGetCurrentRef_cmd_proc,
    }
};

const menu_list_t func_timer_create = 
{
    .menu_name = "timer create",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = rtos_timerCreate_cmd_proc,
    }
};

const menu_list_t func_timer_delete = 
{
    .menu_name = "timer delete",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = rtos_timerDelete_cmd_proc,
    }
};

arg_t start_timer_args[2] = 
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_TIMER_INITIALTIME,
        .msg = "initial time"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_TIMER_RESCHEDULETIME,
        .msg = "reschedule time"
    }
};

const menu_list_t func_timer_start = 
{
    .menu_name = "timer start",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = start_timer_args,
        .argn = sizeof(start_timer_args) / sizeof(arg_t),
        .methods.cmd_handler = rtos_timerStart_cmd_proc,
    }
};

#ifdef SIMCOM_ACC_TIMER_SUPPORT
arg_t start_acctimer_args[2] = 
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_TIMER_INITIALTIME,
        .msg = "timer initial time"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_TIMER_RESCHEDULETIME,
        .msg = "timer rechedule time"
    }
};

const menu_list_t func_acctimer_start = 
{
    .menu_name = "accTimer start",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = start_acctimer_args,
        .argn = sizeof(start_acctimer_args) / sizeof(arg_t),
        .methods.cmd_handler = rtos_accTimerStart_cmd_proc,
    }
};

arg_t stop_acctimer_args[1] = 
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_ACCTIMER_TIMERID,
        .msg = "acctimer id"
    }
};

const menu_list_t func_acctimer_stop = 
{
    .menu_name = "acctimer stop",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = stop_acctimer_args,
        .argn = sizeof(stop_acctimer_args) / sizeof(arg_t),
        .methods.cmd_handler = rtos_accTimerStop_cmd_proc,
    }
};

const menu_list_t func_acctimer_delete = 
{
    .menu_name = "acctimer delete",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = stop_acctimer_args,
        .argn = sizeof(stop_acctimer_args) / sizeof(arg_t),
        .methods.cmd_handler = rtos_accTimerDelete_cmd_proc,
    }
};
#endif

#ifdef FEATURE_SIMCOM_AUDIO
arg_t audio_playFile_args[3] = 
{
    {
        .type = TYPE_RAW_GBK,
        .arg_name = ARG_AUDIO_FILENAME,
        .msg = "audio file name"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_AUDIO_DIRECTPLAY,
        .msg = "direct play or not(0.false, 1.yes)"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_AUDIO_ISSINGLE,
        .msg = "Single play or not(0.false, 1.yes)"
    }
};

const menu_list_t func_audio_playFile =
{
    .menu_name = "audio play file",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = audio_playFile_args,
        .argn = sizeof(audio_playFile_args) / sizeof(arg_t),
        .methods.cmd_handler = audio_play_file_cmd_proc,
    }
};

const menu_list_t func_audio_stopFile =
{
    .menu_name = "audio stop to play file",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = audio_stop_file_cmd_proc,
    }
};

arg_t audio_rec_args[3] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_AUDIO_ENABLE,
        .msg = "0(stop reord) 1(start record)"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_AUDIO_RECORDPATH,
        .msg = "0: local"
    },
    {
        .type = TYPE_RAW_GBK,
        .arg_name = ARG_AUDIO_FILENAME,
        .msg = "audio record file name"
    }
};

const menu_list_t func_audio_rec =
{
    .menu_name = "audio record",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = audio_rec_args,
        .argn = sizeof(audio_rec_args) / sizeof(arg_t),
        .methods.cmd_handler = audio_record_cmd_proc,
    }
};
arg_t audio_setAmrRateLevel_args[1] = 
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_AUDIO_AMR_LEVEL,
        .msg = "AMR rate level (0-7)"
    }
};
const menu_list_t func_audio_setAmrRateLevel =
{
    .menu_name = "audio set AMR rate level",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = audio_setAmrRateLevel_args,
        .argn = sizeof(audio_setAmrRateLevel_args) / sizeof(arg_t),
        .methods.cmd_handler = audio_set_amr_rate_level_cmd_proc,
    }
};

arg_t audio_rec_args1[3] = 
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_AUDIO_REC_OPER,
        .msg = "operation (0.stop, 1.start)"
    },
    {
        .type = TYPE_RAW_GBK,
        .arg_name = ARG_AUDIO_FILENAME,
        .msg = "record file name"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_AUDIO_REC_DURATION,
        .msg = "record duration in seconds"
    }
};
const menu_list_t func_audio_rec1 =
{
    .menu_name = "audio record (simple)",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = audio_rec_args1,
        .argn = sizeof(audio_rec_args1) / sizeof(arg_t),
        .methods.cmd_handler = audio_rec_cmd_proc,
    }
};


arg_t audio_playSampleRate_args[1] = 
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_AUDIO_SAMPLE_RATE,
        .msg = "sample rate(0: 8k, 1: 16k)"
    }
};
const menu_list_t func_audio_playSampleRate =
{
    .menu_name = "audio set sample rate",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = audio_playSampleRate_args,
        .argn = sizeof(audio_playSampleRate_args) / sizeof(arg_t),
        .methods.cmd_handler = audio_play_sample_rate_cmd_proc,
    }
};

arg_t audio_pcmPlay_args[3] = 
{
    {
        .type = TYPE_RAW_GBK,
        .arg_name = ARG_AUDIO_PCM_DATA,
        .msg = "PCM data (hex string)"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_AUDIO_PCM_LEN,
        .msg = "PCM data length"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_AUDIO_DIRECTPLAY,
        .msg = "direct play (0.false, 1.true)"
    }
};
const menu_list_t func_audio_pcmPlay =
{
    .menu_name = "audio PCM play",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = audio_pcmPlay_args,
        .argn = sizeof(audio_pcmPlay_args) / sizeof(arg_t),
        .methods.cmd_handler = audio_pcm_play_cmd_proc,
    }
};

const menu_list_t func_audio_pcmStop = 
{
    .menu_name = "audio PCM stop",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = audio_pcm_stop_cmd_proc,
    }
};

arg_t audio_mp3StreamPlay_args[3] = 
{
    {
        .type = TYPE_RAW_GBK,
        .arg_name = ARG_AUDIO_MP3_DATA,
        .msg = "MP3 data (hex string)"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_AUDIO_MP3_LEN,
        .msg = "MP3 data length"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_AUDIO_DIRECTPLAY,
        .msg = "direct play (0.false, 1.true)"
    }
};
const menu_list_t func_audio_mp3StreamPlay = 
{
    .menu_name = "audio MP3 stream play",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = audio_mp3StreamPlay_args,
        .argn = sizeof(audio_mp3StreamPlay_args)/sizeof(arg_t),
        .methods.cmd_handler = audio_mp3_stream_play_cmd_proc,
    }
};

const menu_list_t func_audio_mp3StreamStop = 
{
    .menu_name = "audio MP3 stream stop",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = audio_mp3_stream_stop_cmd_proc,
    }
};


arg_t audio_amrStreamPlay_args[3] = 
{
    {
        .type = TYPE_RAW_GBK,
        .arg_name = ARG_AUDIO_AMR_DATA,
        .msg = "AMR data (hex string)"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_AUDIO_AMR_LEN,
        .msg = "AMR data length"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_AUDIO_DIRECTPLAY,
        .msg = "direct play (0.false, 1.true)"
    }
};
const menu_list_t func_audio_amrStreamPlay = 
{
    .menu_name = "audio AMR stream play",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = audio_amrStreamPlay_args,
        .argn = sizeof(audio_amrStreamPlay_args)/sizeof(arg_t),
        .methods.cmd_handler = audio_amr_stream_play_cmd_proc,
    }
};

const menu_list_t func_audio_amrStreamStop = 
{
    .menu_name = "audio AMR stream stop",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = audio_amr_stream_stop_cmd_proc,
    }
};

arg_t audio_playAmrCont_args[2] = 
{
    {
        .type = TYPE_RAW_GBK,
        .arg_name = ARG_AUDIO_FILENAME,
        .msg = "AMR file name"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_AUDIO_STARTPLAY,
        .msg = "start play (0.stop, 1.start)"
    }
};
const menu_list_t func_audio_playAmrCont = 
{
    .menu_name = "audio play AMR continuously",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = audio_playAmrCont_args,
        .argn = sizeof(audio_playAmrCont_args)/sizeof(arg_t),
        .methods.cmd_handler = audio_play_amr_cont_cmd_proc,
    }
};

arg_t audio_wavFilePlay_args[2] = 
{
    {
        .type = TYPE_RAW_GBK,
        .arg_name = ARG_AUDIO_FILENAME,
        .msg = "WAV file name"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_AUDIO_DIRECTPLAY,
        .msg = "direct play (0.false, 1.true)"
    }
};
const menu_list_t func_audio_wavFilePlay = 
{
    .menu_name = "audio WAV file play",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = audio_wavFilePlay_args,
        .argn = sizeof(audio_wavFilePlay_args)/sizeof(arg_t),
        .methods.cmd_handler = audio_wav_file_play_cmd_proc,
    }
};

const menu_list_t func_audio_status = 
{
    .menu_name = "get audio status",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = audio_status_cmd_proc,
    }
};

arg_t audio_setPlayPath_args[1] = 
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_AUDIO_PLAY_PATH,
        .msg = "play path (0-255)"
    }
};
const menu_list_t func_audio_setPlayPath = 
{
    .menu_name = "set audio play path",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = audio_setPlayPath_args,
        .argn = sizeof(audio_setPlayPath_args)/sizeof(arg_t),
        .methods.cmd_handler = audio_set_play_path_cmd_proc,
    }
};

arg_t audio_setMicGain_args[1] = 
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_AUDIO_MIC_GAIN,
        .msg = "mic gain (0-255)"
    }
};
const menu_list_t func_audio_setMicGain = 
{
    .menu_name = "set mic gain",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = audio_setMicGain_args,
        .argn = sizeof(audio_setMicGain_args)/sizeof(arg_t),
        .methods.cmd_handler = audio_set_mic_gain_cmd_proc,
    }
};

const menu_list_t func_audio_getMicGain = 
{
    .menu_name = "get mic gain",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = audio_get_mic_gain_cmd_proc,
    }
};

arg_t audio_setVolume_args[1] = 
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_AUDIO_VOLUME,
        .msg = "volume level (enum value)"
    }
};
const menu_list_t func_audio_setVolume = 
{
    .menu_name = "set audio volume",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = audio_setVolume_args,
        .argn = sizeof(audio_setVolume_args)/sizeof(arg_t),
        .methods.cmd_handler = audio_set_volume_cmd_proc,
    }
};

const menu_list_t func_audio_getVolume = 
{
    .menu_name = "get audio volume",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = audio_get_volume_cmd_proc,
    }
};

const menu_list_t func_audio_pcmStreamRec =
{
    .menu_name = "PCM stream record",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = audio_pcm_stream_rec_cmd_proc,
    }
};

const menu_list_t func_audio_pcmStreamStop = 
{
    .menu_name = "PCM stream stop",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = audio_pcm_stream_stop_cmd_proc,
    }
};

const menu_list_t func_audio_amrStreamRec =
{
    .menu_name = "AMR stream record",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = audio_amr_stream_rec_cmd_proc,
    }
};

const menu_list_t func_audio_amrStreamStopRecord =
{
    .menu_name = "AMR stream stop",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = audio_amr_stream_stop_record_cmd_proc,
    }
};


arg_t audio_playMp3Cont_args[3] = 
{
    {
        .type = TYPE_RAW_GBK,
        .arg_name = ARG_AUDIO_FILENAME,
        .msg = "MP3 file name"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_AUDIO_STARTPLAY,
        .msg = "start play (0.stop 1.start)"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_AUDIO_FRAME_MODE,
        .msg = "frame mode (0.disable 1.enable)"
    }
};
const menu_list_t func_audio_playMp3Cont =
{
    .menu_name = "MP3 continuous play",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = audio_playMp3Cont_args,
        .argn = sizeof(audio_playMp3Cont_args) / sizeof(arg_t),
        .methods.cmd_handler = audio_play_mp3_cont_cmd_proc,
    }
};

arg_t audio_playWavCont_args[2] = 
{
    {
        .type = TYPE_RAW_GBK,
        .arg_name = ARG_AUDIO_FILENAME,
        .msg = "WAV file name"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_AUDIO_STARTPLAY,
        .msg = "start play (0.stop 1.start)"
    }
};
const menu_list_t func_audio_playWavCont =
{
    .menu_name = "WAV continuous play",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = audio_playWavCont_args,
        .argn = sizeof(audio_playWavCont_args) / sizeof(arg_t),
        .methods.cmd_handler = audio_play_wav_cont_cmd_proc,
    }
};
#endif

#if defined(FEATURE_SIMCOM_TTS_YOUNGTONE) || defined(FEATURE_SIMCOM_TTS_IFLY)
arg_t ttsPlay_args[3] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_TTS_TEXT_TYPE,
        .msg = "1(unicode) 2(ASCII/GBK)"
    },
    {
        .type = TYPE_RAW_GBK,
        .arg_name = ARG_TTS_TEXT_STR,
        .msg = "text string"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_TTS_MODE,
        .msg = "play mode"
    }
};

const menu_list_t func_ttsPlay =
{
    .menu_name = "tts play",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = ttsPlay_args,
        .argn = sizeof(ttsPlay_args) / sizeof(arg_t),
        .methods.cmd_handler = tts_play_cmd_proc,
    }
};

arg_t tts_parameters_args[5] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_TTS_PARAM_SYSVOLUME,
        .msg = "set sysVolume"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_TTS_PARAM_TTSVOLUME,
        .msg = "set ttsVolume"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_TTS_PARAM_DIGITMODE,
        .msg = "set digitMode"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_TTS_PARAM_PITCH,
        .msg = "set pitch"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_TTS_PARAM_SPEED,
        .msg = "set speed"
    }
};

const menu_list_t func_tts_setParam =
{
    .menu_name = "tts set parameters",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = tts_parameters_args,
        .argn = sizeof(tts_parameters_args) / sizeof(arg_t),
        .methods.cmd_handler = tts_setParam_cmd_proc,
    }
};

const menu_list_t func_ttsStop =
{
    .menu_name = "tts stop to play",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = tts_stop_cmd_proc,
    }
};

const menu_list_t func_tts_status_cb =
{
    .menu_name = "tts set status callback",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = tts_setStatusCb_cmd_proc,
    }
};
#endif

#ifdef FEATURE_SIMCOM_POC
static arg_t poc_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGPOC_SIZE,
        .msg = "The poc data size must be less than 6.4K and a multiple of 320"
    }
};

const menu_list_t func_poc_playSound =
{
    .menu_name = "poc play sound",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = poc_args,
        .argn = sizeof(poc_args) / sizeof(arg_t),
        .RAWSizeLink = ARGPOC_SIZE,
        .methods.__init = poc_init,
        .methods.__uninit = poc_uninit,
        .methods.cmd_handler = poc_play_sound_cmd_proc,
        .methods.raw_handler = poc_play_sound_raw_proc,
        .methods.needRAWData = poc_play_sound_need_size,
    }
};

const menu_list_t func_poc_stopSound =
{
    .menu_name = "poc stop sound",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .methods.cmd_handler = poc_stop_sound_cmd_proc,
    }
};

static const value_t poc_mode_param[2] = 
{
    {.val = 1},
    {.val = 2}
};

static const value_list_t poc_record_mode_list =
{
    .isrange = false,
    .list_head = poc_mode_param,
    .count = sizeof(poc_mode_param) / sizeof(value_t),
};

static arg_t poc_record_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGPOC_MODE,
        .value_range = &poc_record_mode_list,
        .msg = "record mode: 1(FORWORDLY), 2(PASSIVELY)"
    },
};

const menu_list_t func_poc_startRecord =
{
    .menu_name = "poc record",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = poc_record_args,
        .argn = sizeof(poc_record_args) / sizeof(arg_t),
        .methods.cmd_handler = poc_start_record_cmd_proc,
    }
};

const menu_list_t func_poc_stopRecord =
{
    .menu_name = "poc stop record",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .methods.cmd_handler = poc_stop_record_cmd_proc,
    }
};

const menu_list_t func_poc_getPcmAvail_size =
{
    .menu_name = "poc get PcmAvail size",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = poc_getPcmAvailSize_cmd_proc,
    },
};

const menu_list_t func_poc_CleanBufferData =
{
    .menu_name = "poc CleanBufferData",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = poc_CleanBufferData_cmd_proc,
    },
};

arg_t PcmRead_parameters_args[2] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_PCMREAD_PARAM_VALUE,
        .msg = "set PcmRead length"
    },
};

const menu_list_t func_poc_PcmRead =
{
    .menu_name = "Get PcmRead",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = PcmRead_parameters_args,
        .argn = sizeof(PcmRead_parameters_args) / sizeof(arg_t),
        .methods.cmd_handler = poc_PcmRead_cmd_proc,
    },
};

arg_t tonePlay_parameters_args[2] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_TONEPALY_PARAM_TYPE,
        .msg = "set TonePlay type"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_TONEPALY_PARAM_TIME,
        .msg = "set TonePlay time"
    },
};

const menu_list_t func_poc_TonePlay =
{
    .menu_name = "poc TonePlay",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = tonePlay_parameters_args,
        .argn = sizeof(tonePlay_parameters_args) / sizeof(arg_t),
        .methods.cmd_handler = poc_TonePlay_cmd_proc,
    },
};

const menu_list_t func_poc_ToneStop =
{
    .menu_name = "poc ToneStop",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = poc_ToneStop_cmd_proc,
    },
};

arg_t freqplay_parameters_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_FREQPLAY1_PARAM_VALUE,
        .msg = "set FreqPlay freq1"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_FREQPLAY2_PARAM_VALUE,
        .msg = "set FreqPlay freq2"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_FREQPLAY_PARAM_TIME,
        .msg = "set FreqPlay time"
    },
};

const menu_list_t func_poc_FreqPlay =
{
    .menu_name = "poc FreqPlay",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = freqplay_parameters_args,
        .argn = sizeof(freqplay_parameters_args) / sizeof(arg_t),
        .methods.cmd_handler = poc_FreqPlay_cmd_proc,
    },
};

arg_t tonevolume_parameters_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_VOLUME_PARAM_VALUE,
        .msg = "set ToneVolume"
    },
};

const menu_list_t func_poc_SetToneVolume =
{
    .menu_name = "poc SetToneVolume",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = tonevolume_parameters_args,
        .argn = sizeof(tonevolume_parameters_args) / sizeof(arg_t),
        .methods.cmd_handler = poc_SetToneVolume_cmd_proc,
    },
};

const menu_list_t func_poc_getToneVolume =
{
    .menu_name = "poc getToneVolume",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = poc_getToneVolume_cmd_proc,
    },
};

#endif

arg_t wifiscan_parameters_args[4] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_WIFISCAN_PARAM_ROUNDNUMBER,
        .msg = "set round number"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_WIFISCAN_PARAM_BSSIDNUMBER,
        .msg = "set max bssid number"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_WIFISCAN_PARAM_TIMEOUT,
        .msg = "set timeout"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_WIFISCAN_PARAM_PRIORITY,
        .msg = "set priority"
    }
};

const menu_list_t func_wifiscan_setParam =
{
    .menu_name = "wifiscan set parameters",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = wifiscan_parameters_args,
        .argn = sizeof(wifiscan_parameters_args) / sizeof(arg_t),
        .methods.cmd_handler = wifiscan_setParam_cmd_proc,
    }
};

const menu_list_t func_wifiscan_start =
{
    .menu_name = "wifiscan start",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = wifiscan_start_cmd_proc,
    }
};

const menu_list_t func_wifiscan_stop =
{
    .menu_name = "wifiscan stop",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = wifiscan_stop_cmd_proc,
    }
};
    const menu_list_t func_wifiscan_setHandler =
{
    .menu_name = "wifiscan set callback",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = wifiscan_setHandler_cmd_proc,
    }
};

#ifdef FEATURE_SIMCOM_FS
arg_t fopen_args[2] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARG_FS_PATH,
        .msg = "file path and name"
    },
    {
        .type = TYPE_STR,
        .arg_name = ARG_FS_FILEMODE,
        .msg = "operate file mode like rb,wb"
    }
};

const menu_list_t func_fs_fopen =
{
    .menu_name = "open file",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = fopen_args,
        .argn = sizeof(fopen_args) / sizeof(arg_t),
        .methods.cmd_handler = fs_fopen_cmd_proc,
    },
};

const menu_list_t func_fs_fclose =
{
    .menu_name = "close file",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = fs_fclose_cmd_proc,
    },
};

arg_t fwrite_args[1] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_FS_DATASIZE,
        .msg = "data size"
    }
};

const menu_list_t func_fs_fwrite =
{
    .menu_name = "write to file",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = fwrite_args,
        .argn = sizeof(fwrite_args) / sizeof(arg_t),
        .RAWSizeLink = ARG_FS_DATASIZE,
        .methods.__init = write_file_init,
        .methods.__uninit = write_file_uninit,
        .methods.cmd_handler = fs_fwrite_cmd_proc,
        .methods.raw_handler = write_file_raw_proc,
        .methods.needRAWData = write_file_need_size,
    },
};

arg_t fread_args[1] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_FS_DATASIZE,
        .msg = "data size"
    }
};

const menu_list_t func_fs_fread =
{
    .menu_name = "read from file",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = fread_args,
        .argn = sizeof(fread_args) / sizeof(arg_t),
        .methods.cmd_handler = fs_fread_cmd_proc,
    },
};

arg_t fseek_args[2] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_FS_OFFSET,
        .msg = "offset"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_FS_POSITION,
        .msg = "position"
    }
};

const menu_list_t func_fs_fseek =
{
    .menu_name = "file seek",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = fseek_args,
        .argn = sizeof(fseek_args) / sizeof(arg_t),
        .methods.cmd_handler = fs_fseek_cmd_proc,
    },
};

const menu_list_t func_fs_fsync =
{
    .menu_name = "sync data to file",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = fs_fsync_cmd_proc,
    },
};

const menu_list_t func_fs_ftell =
{
    .menu_name = "get pointer position in file",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = fs_ftell_cmd_proc,
    },
};

const menu_list_t func_fs_fsize =
{
    .menu_name = "get size of file",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = fs_fsize_cmd_proc,
    },
};

arg_t fstat_args[1] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARG_FS_PATH,
        .msg = "file path and name"
    }
};

const menu_list_t func_fs_fstat =
{
    .menu_name = "get file info",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = fstat_args,
        .argn = sizeof(fstat_args) / sizeof(arg_t),
        .methods.cmd_handler = fs_fstat_cmd_proc,
    },
};

arg_t rename_param[2] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARG_FS_OLDNAME,
        .msg = "old filename"
    },
    {
        .type = TYPE_STR,
        .arg_name = ARG_FS_NEWNAME,
        .msg = "new filename"
    }
};

const menu_list_t func_fs_rename =
{
    .menu_name = "rename file",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = rename_param,
        .argn = sizeof(rename_param) / sizeof(arg_t),
        .methods.cmd_handler = fs_rename_cmd_proc,
    },
};

arg_t remove_args[1] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARG_FS_PATH,
        .msg = "file path and name"
    },
};

const menu_list_t func_fs_remove =
{
    .menu_name = "remove file",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = remove_args,
        .argn = sizeof(remove_args) / sizeof(arg_t),
        .methods.cmd_handler = fs_remove_cmd_proc,
    },
};

arg_t mkdir_args[1] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARG_FS_PATH,
        .msg = "file path"
    },
};

const menu_list_t func_fs_mkdir =
{
    .menu_name = "create dir",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = mkdir_args,
        .argn = sizeof(mkdir_args) / sizeof(arg_t),
        .methods.cmd_handler = fs_mkdir_cmd_proc,
    },
};

const menu_list_t func_fs_opendir =
{
    .menu_name = "open dir",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = mkdir_args,
        .argn = sizeof(mkdir_args) / sizeof(arg_t),
        .methods.cmd_handler = fs_opendir_cmd_proc,
    },
};

const menu_list_t func_fs_closedir =
{
    .menu_name = "close dir",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = fs_closedir_cmd_proc,
    },
};

const menu_list_t func_fs_readdir =
{
    .menu_name = "read dir",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = fs_readdir_cmd_proc,
    },
};

arg_t disk_param[1] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARG_FS_DISK,
    }
};

const menu_list_t func_fs_get_disk_size =
{
    .menu_name = "Get total size",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = disk_param,
        .argn = sizeof(disk_param) / sizeof(arg_t),
        .methods.cmd_handler = fs_get_size_cmd_proc,
    },
};

const menu_list_t func_get_free_size =
{
    .menu_name = "Get free size",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = disk_param,
        .argn = sizeof(disk_param) / sizeof(arg_t),
        .methods.cmd_handler = fs_get_free_size_cmd_proc,
    },
};
        
const menu_list_t func_get_used_size =
{
    .menu_name = "Get used size",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = disk_param,
        .argn = sizeof(disk_param) / sizeof(arg_t),
        .methods.cmd_handler = fs_get_used_size_cmd_proc,
    },
};

value_t switchModeList[2] =
{
    {
        .bufp = ARG_FS_SWITCH_TO_ROOT,
        .size = sizeof(ARG_FS_SWITCH_TO_ROOT)
    },
    {
        .bufp = ARG_FS_SWITCH_TO_C,
        .size = sizeof(ARG_FS_SWITCH_TO_C)
    }
};

value_list_t switchModeRange =
{
    .isrange = false,
    .list_head = switchModeList,
    .count = sizeof(switchModeList) / sizeof(value_t)
};

arg_t switch_param[2] =
{
    {
        .type = TYPE_RAW_GBK,
        .arg_name = ARG_FS_PATH,
        .msg = "Support disks: C:"
    },
    {
        .type = TYPE_STR,
        .arg_name = ARG_FS_SWITCHMODE,
        .value_range = &switchModeRange,
    }
};

const menu_list_t func_fs_switchdir =
{
    .menu_name = "Switch Dir",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = switch_param,
        .argn = sizeof(switch_param) / sizeof(arg_t),
        .methods.cmd_handler = fs_switch_dir_cmd_proc,
    },
};

#if defined (EXT_FLASH_LFSV2_ENABLE) || defined (EXT_FLASH_LFSV1_ENABLE) || defined (FEATURE_SIMCOM_SD_CARD) || defined (EXT_FLASH_YAFFS2_ENABLE)
const menu_list_t func_fs_format =
{
    .menu_name = "Disk format",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = disk_param,
        .argn = sizeof(disk_param) / sizeof(arg_t),
        .methods.cmd_handler = fs_format_cmd_proc,
    },
};
#endif
#endif

const value_list_t pdpid_range =
{
    .isrange = true,
    .min = 1,
    .max = 6,
};

const value_t familytype_range[] =
{
    {
        .bufp = ARG_TCPIP_IPV4,
        .size = sizeof(ARG_TCPIP_IPV4)
    },
    {
        .bufp = ARG_TCPIP_IPV6,
        .size = sizeof(ARG_TCPIP_IPV6)
    }
};

const value_list_t family_range =
{
    .isrange = false,
    .list_head = familytype_range,
    .count = 2,
};

const value_t socktype_range[] =
{
    {
        .bufp = "TCP",
        .size = 3
    },
    {
        .bufp = "UDP",
        .size = 3
    }
};

const value_list_t sock_range =
{
    .isrange = false,
    .list_head = socktype_range,
    .count = 2,
};

const value_list_t port_range =
{
    .isrange = true,
    .min = 0,
    .max = 65535,
};

arg_t tcpippdpid_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_TCPIP_PDPID,
        .value_range = &pdpid_range,
    }
};

arg_t tcpipsocket_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARG_TCPIP_FAMILY,
        .msg = "Address family of socket",
        .value_range = &family_range,
    },
    {
        .type = TYPE_STR,
        .arg_name = ARG_TCPIP_SOCKTYPE,
        .msg = "Socket protocol types (TCP/UDP/RAW)",
        .value_range = &sock_range,
    }
};

arg_t tcpipbind_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_TCPIP_SOCKETFD,
        .msg = "Socketfd",
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_TCPIP_PORT,
        .msg = "Integer type, identifies the port of TCP server",
        .value_range = &port_range,
    }
};

arg_t tcpipsocketfd_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_TCPIP_SOCKETFD,
        .msg = "Socketfd",
    }
};

arg_t tcpiprecv_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_TCPIP_SOCKETFD,
        .msg = "Socketfd",
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_TCPIP_DATASIZE,
        .msg = "data size"
    }
};

arg_t tcpipconnect_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_TCPIP_SOCKETFD,
        .msg = "Socketfd",
    },
    {
        .type = TYPE_STR,
        .arg_name = ARG_TCPIP_SERVERIP,
        .msg = "String type, identifies the IP address of server  e.g: 123.45.67.89 or www.example.com"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_TCPIP_PORT,
        .msg = "Integer type, identifies the port of TCP server",
        .value_range = &port_range,
    }
};

arg_t tcpipsend_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_TCPIP_SOCKETFD,
        .msg = "Socketfd",
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_TCPIP_DATASIZE,
        .msg = "data size"
    }
};

const menu_list_t func_tcpip_init =
{
    .menu_name = "tcpip init",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = tcpippdpid_args,
        .argn = sizeof(tcpippdpid_args) / sizeof(arg_t),
        .methods.cmd_handler = tcpip_init_cmd_proc,
    },
};

const menu_list_t func_tcpip_socket =
{
    .menu_name = "tcpip socket",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = tcpipsocket_args,
        .argn = sizeof(tcpipsocket_args) / sizeof(arg_t),
        .methods.cmd_handler = tcpip_socket_cmd_proc,
    },
};

const menu_list_t func_tcpip_bind =
{
    .menu_name = "tcpip bind",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = tcpipbind_args,
        .argn = sizeof(tcpipbind_args) / sizeof(arg_t),
        .methods.cmd_handler = tcpip_bind_cmd_proc,
    },
};

const menu_list_t func_tcpip_listen =
{
    .menu_name = "tcpip listen",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = tcpipsocketfd_args,
        .argn = sizeof(tcpipsocketfd_args) / sizeof(arg_t),
        .methods.cmd_handler = tcpip_listen_cmd_proc,
    },
};

const menu_list_t func_tcpip_accept =
{
    .menu_name = "tcpip accept",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = tcpipsocketfd_args,
        .argn = sizeof(tcpipsocketfd_args) / sizeof(arg_t),
        .methods.cmd_handler = tcpip_accept_cmd_proc,
    },
};

const menu_list_t func_tcpip_select =
{
    .menu_name = "tcpip select",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = tcpipsocketfd_args,
        .argn = sizeof(tcpipsocketfd_args) / sizeof(arg_t),
        .methods.cmd_handler = tcpip_select_cmd_proc,
    },
};

const menu_list_t func_tcpip_recv =
{
    .menu_name = "tcpip recv",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = tcpiprecv_args,
        .argn = sizeof(tcpiprecv_args) / sizeof(arg_t),
        .methods.cmd_handler = tcpip_recv_cmd_proc,
    },
};

const menu_list_t func_tcpip_connect =
{
    .menu_name = "tcpip connect",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = tcpipconnect_args,
        .argn = sizeof(tcpipconnect_args) / sizeof(arg_t),
        .methods.cmd_handler = tcpip_connect_cmd_proc,
    },
};

const menu_list_t func_tcpip_send =
{
    .menu_name = "tcpip send",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = tcpipsend_args,
        .argn = sizeof(tcpipsend_args) / sizeof(arg_t),
        .RAWSizeLink = ARG_TCPIP_DATASIZE,
        .methods.__init = tcpip_send_init,
        .methods.__uninit = tcpip_send_uninit,
        .methods.cmd_handler = tcpip_send_cmd_proc,
        .methods.raw_handler = tcpip_send_raw_proc,
        .methods.needRAWData = tcpip_send_need_size,
    },
};

const menu_list_t func_tcpip_setsockopt =
{
    .menu_name = "tcpip setsockopt",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = tcpipsocketfd_args,
        .argn = sizeof(tcpipsocketfd_args) / sizeof(arg_t),
        .methods.cmd_handler = tcpip_setsockopt_cmd_proc,
    },
};

const menu_list_t func_tcpip_getsockopt =
{
    .menu_name = "tcpip getsockopt",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = tcpipsocketfd_args,
        .argn = sizeof(tcpipsocketfd_args) / sizeof(arg_t),
        .methods.cmd_handler = tcpip_getsockopt_cmd_proc,
    },
};

const menu_list_t func_tcpip_close =
{
    .menu_name = "tcpip close",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = tcpipsocketfd_args,
        .argn = sizeof(tcpipsocketfd_args) / sizeof(arg_t),
        .methods.cmd_handler = tcpip_close_cmd_proc,
    },
};

const menu_list_t func_tcpip_deinit =
{
    .menu_name = "tcpip deinit",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = tcpippdpid_args,
        .argn = sizeof(tcpippdpid_args) / sizeof(arg_t),
        .methods.cmd_handler = tcpip_deinit_cmd_proc,
    },
};

const menu_list_t func_httpsinit=
{
    .menu_name = "Https session initialize",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL ,
        .argn = 0 ,
        .methods.cmd_handler = httpsinit_cmd
    }

};

const value_t para_type_head[]=
{
    {
        .bufp = "URL",
        .size =3,
    },
    {
        .bufp = "CONNECTTO",
        .size = 9,
    },
    {
        .bufp = "CONTENT",
        .size =7,
    },
    {
        .bufp = "USERDATA",
        .size = 8,
    },
    {
        .bufp = "ACCEPT",
        .size = 6,
    },
    {
        .bufp = "SSLCFG",
        .size = 6,
    },
    {
        .bufp = "RECVTO",
        .size = 6,
    },
    {
        .bufp = "READMODE",
        .size = 8,
    },
};

const value_list_t para_type_range=
{
    .isrange =false,
    .list_head = para_type_head,
    .count = 8,
};

arg_t httpspara_args[2]=
{
    {
        .type = TYPE_STR,
        .arg_name = ARG_HTTPS_PARATYPE,
        .value_range =&para_type_range,
        
        //.msg = "input type:(URL,CONNECTTO,CONTENT,USERDATA,ACCEPT,SSLCFG,RECVTO,READMODE)"

    },
    {
        .type = TYPE_STR,
        .arg_name = ARG_HTTPS_PARAVALUE,
    }
};


const menu_list_t func_httpspara=
{
    .menu_name = "Set parameters",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = httpspara_args ,
        .argn = sizeof(httpspara_args)/sizeof(arg_t),
        .methods.cmd_handler = httpspara_cmd,
    }

};


arg_t httpsdata_args[1] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_HTTPS_DATALEN,
    }
};

const menu_list_t func_httpsdata=
{
    .menu_name = "Set post data",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = httpsdata_args,
        .argn = sizeof(httpsdata_args)/sizeof(arg_t),
        .RAWSizeLink = ARG_HTTPS_DATALEN,
        .methods.__init = httpsdata_init,
        .methods.__uninit = httpsdata_uninit,
        .methods.cmd_handler = httpsdata_cmd,
        .methods.raw_handler = httpsdata_raw,
        .methods.needRAWData = httpsdata_need_size,
    }

};

const value_t action_method_head[]=
{
    {
        .bufp = "0:GET",
        .size = 5,
    },
    {
        .bufp ="1:POST",
        .size = 6,
    },
    {
        .bufp = "2:HEAD",
        .size = 6,
    },
    {
        .bufp = "3:DELETE",
        .size = 8,
    },
    {
        .bufp ="4:PUT",
        .size = 4,
    },
};

const value_list_t method_range =
{
    .isrange = false,
    .list_head = action_method_head,
    .count = 5,
};

arg_t httpsaction_args[1] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARG_HTTPS_METHOD,
        .value_range = &method_range,
    }
};

const menu_list_t func_httpsaction=
{
    .menu_name = "Send http request",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = httpsaction_args ,
        .argn = sizeof(httpsaction_args)/sizeof(arg_t),
        .methods.cmd_handler = httpsaction_cmd,
    }

};

const menu_list_t func_httpshead=
{
    .menu_name = "read response header",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL ,
        .argn = 0 ,
        .methods.cmd_handler = httpshead_cmd,
    }
};

const value_t readtype_range_head[]=
{
   {
        .bufp = "0 get data len",
        .size = 14,
   },
   {
        .bufp = "1 read data",
        .size =11,
   },
};

const value_list_t readtype_range =
{
    .isrange = false,
    .list_head =readtype_range_head,
    .count = 2, 
};

arg_t read_args[3] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARG_HTTPS_READTYPE,
        .value_range = &readtype_range
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_HTTPS_READOFFSET,
        .msg = "read data offset",
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_HTTPS_READSIZE,
        .msg = "noce read size",
    }
};


const menu_list_t func_httpsread=
{
    .menu_name = "read response body",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = read_args ,
        .argn = sizeof(read_args)/sizeof(arg_t),
        .methods.cmd_handler = httpsread_cmd,
    }

};

const menu_list_t func_httpsterm=
{
    .menu_name = "httpsterm",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL ,
        .argn = 0 ,
        .methods.cmd_handler = httpsterm_cmd,
    }

};

#ifdef FEATURE_SIMCOM_HTP
arg_t htp_config_args[7] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_HTP_COMMAND,
        .msg = "HTP command type, 0-1",
    },
    {
        .type = TYPE_STR,
        .arg_name = ARG_HTP_CMD,
        .msg = "HTP command type, ADD/DEL",
    },
    {
        .type = TYPE_STR,
        .arg_name = ARG_HTP_HOST_INDEX,
        .msg = "HTP server address/index, www.baidu.com/0-9.",
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_HTP_PORT,
        .msg = "HTP server port, the range is (1-65535).",
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_HTP_VERSION,
        .msg = "The HTTP version of the HTP server: 0(HTTP 1.0) or 1(HTTP 1.1).",
    },
    {
        .type = TYPE_STR,
        .arg_name = ARG_HTP_PROXY,
        .msg = "HTP server proxy, the length: 1-255.",
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_HTP_PROXY_PORT,
        .msg = "HTP server proxy port, the range is (1-65535).",
    }
};

const menu_list_t func_htpConfig =
{
    .menu_name = "htp config",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = htp_config_args ,
        .argn = sizeof(htp_config_args)/sizeof(arg_t),
        .methods.cmd_handler = htp_config_cmd_proc,
    }
};

const menu_list_t func_htpUpdate =
{
    .menu_name = "htp update",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL ,
        .argn = 0 ,
        .methods.cmd_handler = htp_update_cmd_proc,
    }
};
#endif

#ifdef FEATURE_SIMCOM_NTP
arg_t ntp_update_args[3] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_NTP_COMMAND,
        .msg = "ntp command type"
    },
    {
        .type = TYPE_STR,
        .arg_name = ARG_NTP_HOSTADDR,
        .msg = "host_addr"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_NTP_TIMEZONE,
        .msg = "time zone"
    }
};


const menu_list_t func_ntpUpdate=
{
    .menu_name = "ntp update",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = ntp_update_args ,
        .argn = sizeof(ntp_update_args)/sizeof(arg_t),
        .methods.cmd_handler = ntp_update_cmd_proc,
    }

};
#endif

#ifdef FEATURE_SIMCOM_SSL
arg_t ssl_set_context_args[3] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARG_SSL_OPER,
        .msg = "ssl operation"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_SSL_ID,
        .msg = "client id"
    },
    {
        .type = TYPE_STR,
        .arg_name = ARG_SSL_BUF,
        .msg = "config buffer"
    }
};


const menu_list_t func_sslSetContext=
{
    .menu_name = "ssl set context",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = ssl_set_context_args ,
        .argn = sizeof(ssl_set_context_args)/sizeof(arg_t),
        .methods.cmd_handler = ssl_set_context_cmd_proc,
    }
};


arg_t ssl_get_context_args[1] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_SSL_ID,
        .msg = "client id"
    },
};

const menu_list_t func_sslGetContext=
{
    .menu_name = "ssl get context",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = ssl_get_context_args ,
        .argn = sizeof(ssl_get_context_args)/sizeof(arg_t),
        .methods.cmd_handler = ssl_get_context_cmd_proc,
    }
};

arg_t crypto_args[3] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_CRYPTO_OP,
        .msg = "crypto operation"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_CRYPTO_MODE,
        .msg = "mode"
    },
    {
        .type = TYPE_STR,
        .arg_name = ARG_CRYPTO_DATA,
    }
};

const menu_list_t func_crypto =
{
    .menu_name = "crypto",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = crypto_args ,
        .argn = sizeof(crypto_args)/sizeof(arg_t),
        .methods.cmd_handler = crypto_cmd_proc,
    }
};

const menu_list_t func_cryptoCreate=
{
    .menu_name = "crypto create",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL ,
        .argn = 0,
        .methods.cmd_handler = crypto_create_cmd_proc,
    }
};

const menu_list_t func_cryptoReset=
{
    .menu_name = "crypto reset",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL ,
        .argn = 0,
        .methods.cmd_handler = crypto_reset_cmd_proc,
    }
};

const menu_list_t func_cryptoDelete=
{
    .menu_name = "crypto delete",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL ,
        .argn = 0,
        .methods.cmd_handler = crypto_delete_cmd_proc,
    }
};

arg_t crypto_cfg_args[2] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_CRYPTO_CFG,
        .msg = "crypto cfg"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_CRYPTO_DATA,
        .msg = "input data"
    }
};

const menu_list_t func_crypto_cfg =
{
    .menu_name = "crypto cfg",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = crypto_cfg_args ,
        .argn = sizeof(crypto_cfg_args)/sizeof(arg_t),
        .methods.cmd_handler = crypto_cfg_cmd_proc,
    }
};

const menu_list_t func_crypto_get_cfg =
{
    .menu_name = "crypto get cfg",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL ,
        .argn = 0,
        .methods.cmd_handler = crypto_get_cfg_cmd_proc,
    }
};

arg_t crypto_random_args[2] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_CRYPTO_CUSTOM_OUTLEN,
        .msg = "out len"
    },
    {
        .type = TYPE_STR,
        .arg_name = ARG_CRYPTO_CUSTOM,
        .msg = "custom data"
    }
};

const menu_list_t func_crypto_random =
{
    .menu_name = "crypto randonm",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = crypto_random_args ,
        .argn = sizeof(crypto_random_args)/sizeof(arg_t),
        .methods.cmd_handler = crypto_random_cmd_proc,
    }
};

arg_t crypto_prf_args[4] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_CRYPTO_PRFTYPE,
    },
    {
        .type = TYPE_STR,
        .arg_name = ARG_CRYPTO_SECRET,
    },
    {
        .type = TYPE_STR,
        .arg_name = ARG_CRYPTO_LABEL,
    },
    {
        .type = TYPE_STR,
        .arg_name = ARG_CRYPTO_RANDOM,
    }
};

const menu_list_t func_crypto_sslTlsPrf =
{
    .menu_name = "crypto SSLTLSPrf",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = crypto_prf_args ,
        .argn = sizeof(crypto_prf_args)/sizeof(arg_t),
        .methods.cmd_handler = crypto_sslTlsPrf_cmd_proc,
    }
};
#endif

#ifdef FEATURE_SIMCOM_MQTT
const menu_list_t func_mqtt_start =
{
    .menu_name = "mqtt start",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL ,
        .argn = 0,
        .methods.cmd_handler = mqtt_start_cmd_proc,
    }
};

arg_t accq_args[3] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_MQTT_INDEX,
    },
    {
        .type = TYPE_STR,
        .arg_name = ARG_MQTT_CLIENTID,
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_MQTT_SSLENABLE,
    }
};

const menu_list_t func_mqtt_accq =
{
    .menu_name = "mqtt accq",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = accq_args ,
        .argn = sizeof(accq_args)/sizeof(arg_t),
        .methods.cmd_handler = mqtt_accq_cmd_proc,
    }
};

const menu_list_t func_mqtt_connLostCb =
{
    .menu_name = "mqtt connLostCb",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL ,
        .argn = 0,
        .methods.cmd_handler = mqtt_connLostCb_cmd_proc,
    }
};

arg_t conn_args[6] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_MQTT_INDEX,
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_MQTT_ALIVE_TIME,
    },
        {
        .type = TYPE_INT,
        .arg_name = ARG_MQTT_CLEAN_SESSION,
    },
    {
        .type = TYPE_STR,
        .arg_name = ARG_MQTT_SERVER_HOST,
    },
    {
        .type = TYPE_STR,
        .arg_name = ARG_MQTT_USER_NAME,
    },
    {
        .type = TYPE_STR,
        .arg_name = ARG_MQTT_PASSWORD,
    }
};

const menu_list_t func_mqtt_connect =
{
    .menu_name = "mqtt connect",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = conn_args ,
        .argn = sizeof(conn_args)/sizeof(arg_t),
        .methods.cmd_handler = mqtt_connect_cmd_proc,
    }
};

arg_t disconn_args[2] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_MQTT_INDEX,
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_MQTT_DISSCON_TIMEOUT,
    }
};

const menu_list_t func_mqtt_disconnect =
{
    .menu_name = "mqtt disconnect",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = disconn_args ,
        .argn = sizeof(disconn_args)/sizeof(arg_t),
        .methods.cmd_handler = mqtt_disconnect_cmd_proc,
    }
};

arg_t sub_args[4] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_MQTT_INDEX,
    },
    {
        .type = TYPE_STR,
        .arg_name = ARG_MQTT_TOPIC_DATA,
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_MQTT_QOS,
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_MQTT_DUP,
    }
};

const menu_list_t func_mqtt_sub =
{
    .menu_name = "mqtt subscribe",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = sub_args ,
        .argn = sizeof(sub_args)/sizeof(arg_t),
        .methods.cmd_handler = mqtt_sub_cmd_proc,
    }
};

arg_t unsub_args[3] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_MQTT_INDEX,
    },
    {
        .type = TYPE_STR,
        .arg_name = ARG_MQTT_TOPIC_DATA,
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_MQTT_DUP,
    }
};

const menu_list_t func_mqtt_unsub =
{
    .menu_name = "mqtt unsub",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = unsub_args ,
        .argn = sizeof(unsub_args)/sizeof(arg_t),
        .methods.cmd_handler = mqtt_unsub_cmd_proc,
    }
};

const menu_list_t func_mqtt_topic =
{
    .menu_name = "mqtt topic",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = unsub_args ,
        .argn = sizeof(unsub_args)/sizeof(arg_t),
        .methods.cmd_handler = mqtt_topic_cmd_proc,
    }
};

arg_t payload_args[2] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_MQTT_INDEX,
    },
    {
        .type = TYPE_STR,
        .arg_name = ARG_MQTT_PAYLOAD_DATA,
    }
};

const menu_list_t func_mqtt_payload =
{
    .menu_name = "mqtt payload",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = payload_args ,
        .argn = sizeof(payload_args)/sizeof(arg_t),
        .methods.cmd_handler = mqtt_payload_cmd_proc,
    }
};

arg_t pub_args[4] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_MQTT_INDEX,
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_MQTT_QOS,
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_MQTT_RATAINED,
    },
    {
        .type = TYPE_STR,
        .arg_name = ARG_MQTT_DUP,
    }
};

const menu_list_t func_mqtt_pub =
{
    .menu_name = "mqtt pub",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = pub_args ,
        .argn = sizeof(pub_args)/sizeof(arg_t),
        .methods.cmd_handler = mqtt_pub_cmd_proc,
    }
};

arg_t rel_args[1] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_MQTT_INDEX,
    }
};

const menu_list_t func_mqtt_rel =
{
    .menu_name = "mqtt release",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = rel_args ,
        .argn = sizeof(rel_args)/sizeof(arg_t),
        .methods.cmd_handler = mqtt_rel_cmd_proc,
    }
};

const menu_list_t func_mqtt_stop =
{
    .menu_name = "mqtt stop",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL ,
        .argn = 0,
        .methods.cmd_handler = mqtt_stop_cmd_proc,
    }
};

arg_t willtopic_args[2] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_MQTT_INDEX,
    },
    {
        .type = TYPE_STR,
        .arg_name = ARG_MQTT_WILLTOPIC_DATA,
    }
};

const menu_list_t func_mqtt_willtopic =
{
    .menu_name = "mqtt willtopic",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = willtopic_args ,
        .argn = sizeof(willtopic_args)/sizeof(arg_t),
        .methods.cmd_handler = mqtt_willtopic_cmd_proc,
    }
};

arg_t willmsg_args[2] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_MQTT_INDEX,
    },
    {
        .type = TYPE_STR,
        .arg_name = ARG_MQTT_WILLMSG_DATA,
    }
};

const menu_list_t func_mqtt_willmsg =
{
    .menu_name = "mqtt willmsg",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = willmsg_args ,
        .argn = sizeof(willmsg_args)/sizeof(arg_t),
        .methods.cmd_handler = mqtt_willmsg_cmd_proc,
    }
};

arg_t subtopic_args[3] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_MQTT_INDEX,
    },
    {
        .type = TYPE_STR,
        .arg_name = ARG_MQTT_SUBTOPIC_DATA,
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_MQTT_QOS,
    }
};


const menu_list_t func_mqtt_subtopic =
{
    .menu_name = "mqtt subtopic",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = subtopic_args ,
        .argn = sizeof(subtopic_args)/sizeof(arg_t),
        .methods.cmd_handler = mqtt_subtopic_cmd_proc,
    }
};

arg_t unsubtopic_args[3] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_MQTT_INDEX,
    },
    {
        .type = TYPE_STR,
        .arg_name = ARG_MQTT_UNSUBTOPIC_DATA,
    }
};

const menu_list_t func_mqtt_unsubtopic =
{
    .menu_name = "mqtt unsubtopic",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = unsubtopic_args ,
        .argn = sizeof(unsubtopic_args)/sizeof(arg_t),
        .methods.cmd_handler = mqtt_unsubtopic_cmd_proc,
    }
};

arg_t sslcfg_args[3] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_MQTT_CMD_TYPE,
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_MQTT_INDEX,
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_MQTT_SSL_INDEX,
    }
};

const menu_list_t func_mqtt_sslcfg =
{
    .menu_name = "mqtt sslcfg",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = sslcfg_args ,
        .argn = sizeof(sslcfg_args)/sizeof(arg_t),
        .methods.cmd_handler = mqtt_sslcfg_cmd_proc,
    }
};

arg_t cfg_args[4] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_MQTT_CMD_TYPE,
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_MQTT_INDEX,
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_MQTT_CFG_TYPE,
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_MQTT_RELATED_VALUE,
    }
};

const menu_list_t func_mqtt_cfg =
{
    .menu_name = "mqtt cfg",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = cfg_args ,
        .argn = sizeof(cfg_args)/sizeof(arg_t),
        .methods.cmd_handler = mqtt_cfg_cmd_proc,
    }
};
#endif

#ifdef HAS_DEMO_FTPS
arg_t init_args[1] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_FTPS_PDP_ACTIVE_TYPE,
    }
};
const menu_list_t func_ftps_init = 
{
    .menu_name = "ftps init",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = init_args,
        .argn = sizeof(init_args) / sizeof(arg_t),
        .methods.cmd_handler = ftps_init_cmd_proc,
    }
};

const menu_list_t func_ftps_deinit = 
{
    .menu_name = "ftps deinit",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = init_args,
        .argn = sizeof(init_args) / sizeof(arg_t),
        .methods.cmd_handler = ftps_deinit_cmd_proc,
    }
};

arg_t ftps_login_args[5] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARG_FTPS_HOST,
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_FTPS_PORT,
    },
    {
        .type = TYPE_STR,
        .arg_name = ARG_FTPS_USERNAME,
    },
    {
        .type = TYPE_STR,
        .arg_name = ARG_FTPS_PASSWORD,
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_FTPS_SERVER_TYPE,
    }
};

const menu_list_t func_ftps_login = 
{
    .menu_name = "ftps login",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = ftps_login_args,
        .argn = sizeof(ftps_login_args) / sizeof(arg_t),
        .methods.cmd_handler = ftps_login_cmd_proc,
    }
};

const menu_list_t func_ftps_logout = 
{
    .menu_name = "ftps logout",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = ftps_logout_cmd_proc,
    }
};

arg_t download_args[2] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARG_FTPS_FILENAME,
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_FTPS_LOCATION,
    }
};

const menu_list_t func_ftps_download = 
{
    .menu_name = "ftps download file",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = download_args,
        .argn = sizeof(download_args) / sizeof(arg_t),
        .methods.cmd_handler = ftps_downloadFile_cmd_proc,
    }
};

arg_t downloadToBuffer_args[2] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARG_FTPS_FILENAME,
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_FTPS_POSITION,
    }
};

const menu_list_t func_ftps_downloadToBuffer = 
{
    .menu_name = "ftps download file to buffer",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = downloadToBuffer_args,
        .argn = sizeof(downloadToBuffer_args) / sizeof(arg_t),
        .methods.cmd_handler = ftps_downloadFileToBuffer_cmd_proc,
    }
};

arg_t upload_args[3] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARG_FTPS_FILENAME,
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_FTPS_LOCATION,
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_FTPS_POSITION,
    }
};
const menu_list_t func_ftps_uploadFile = 
{
    .menu_name = "ftps upload file",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = upload_args,
        .argn = sizeof(upload_args) / sizeof(arg_t),
        .methods.cmd_handler = ftps_uploadFile_cmd_proc,
    }
};

arg_t put_args[2] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARG_FTPS_FILENAME,
    },
    {
        .type = TYPE_STR,
        .arg_name = ARG_FTPS_FILENAME,
    }
};

const menu_list_t func_ftps_putFile = 
{
    .menu_name = "ftps put file",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = put_args,
        .argn = sizeof(put_args) / sizeof(arg_t),
        .methods.cmd_handler = ftps_putFile_cmd_proc,
    }
};

arg_t delete_args[1] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARG_FTPS_FILENAME,
    }
};

const menu_list_t func_ftps_deleteFile = 
{
    .menu_name = "ftps delete file",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = delete_args,
        .argn = sizeof(delete_args) / sizeof(arg_t),
        .methods.cmd_handler = ftps_deleteFile_cmd_proc,
    }
};

arg_t createDir_args[1] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARG_FTPS_FILENAME,
    }
};

const menu_list_t func_ftps_createDir = 
{
    .menu_name = "ftps create directory",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = createDir_args,
        .argn = sizeof(createDir_args) / sizeof(arg_t),
        .methods.cmd_handler = ftps_createDir_cmd_proc,
    }
};

const menu_list_t func_ftps_deleteDir = 
{
    .menu_name = "ftps delete directory",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = createDir_args,
        .argn = sizeof(createDir_args) / sizeof(arg_t),
        .methods.cmd_handler = ftps_deleteDir_cmd_proc,
    }
};

const menu_list_t func_ftps_changeDir = 
{
    .menu_name = "ftps change directory",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = createDir_args,
        .argn = sizeof(createDir_args) / sizeof(arg_t),
        .methods.cmd_handler = ftps_changeDir_cmd_proc,
    }
};

const menu_list_t func_ftps_getCurDir = 
{
    .menu_name = "ftps get current directory",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = ftps_getCurDir_cmd_proc,
    }
};

const menu_list_t func_ftps_list = 
{
    .menu_name = "ftps list",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = createDir_args,
        .argn = sizeof(createDir_args) / sizeof(arg_t),
        .methods.cmd_handler = ftps_list_cmd_proc,
    }
};

const menu_list_t func_ftps_getFizeSize = 
{
    .menu_name = "ftps get fileSize",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = createDir_args,
        .argn = sizeof(createDir_args) / sizeof(arg_t),
        .methods.cmd_handler = ftps_getFileSize_cmd_proc,
    }
};

arg_t transferType_args[1] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_FTPS_TYPE,
    }
};

const menu_list_t func_ftps_transferType = 
{
    .menu_name = "ftps transfer type",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = transferType_args,
        .argn = sizeof(transferType_args) / sizeof(arg_t),
        .methods.cmd_handler = ftps_transferType_cmd_proc,
    }
};

const menu_list_t func_ftps_getTransferType = 
{
    .menu_name = "ftps get transferType",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = transferType_args,
        .argn = sizeof(transferType_args) / sizeof(arg_t),
        .methods.cmd_handler = ftps_getTransferType_cmd_proc,
    }
};

arg_t sslconfig_args[2] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_FTPS_SESSION,
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_FTPS_SSLID,
    }
};

const menu_list_t func_ftps_sslConfig = 
{
    .menu_name = "ftps config ssl",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = sslconfig_args,
        .argn = sizeof(sslconfig_args) / sizeof(arg_t),
        .methods.cmd_handler = ftps_sslConfig_cmd_proc,
    }
};

arg_t setMode_args[1] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_FTPS_MODE,
    }
};

const menu_list_t func_ftps_setMode = 
{
    .menu_name = "ftps set mode",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = setMode_args,
        .argn = sizeof(setMode_args) / sizeof(arg_t),
        .methods.cmd_handler = ftps_setMode_cmd_proc
    }
};
#endif
/*======================== for driver gpio begin ========================*/
static arg_t getgpiolevel_args[1] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_GPIO_NUMBER,
        .msg = "Please input gpio number:"
    }
};

static const menu_list_t func_getgpiolevel =
{
    .menu_name = "Get gpio level",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = getgpiolevel_args,
        .argn = sizeof(getgpiolevel_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = get_gpio_level_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

static arg_t setgpiointerrupt_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_GPIO_NUMBER,
        .msg = "Please input gpio number:"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_GPIO_INTERRUPT,
    }
};

static const menu_list_t func_setgpiointerrupt =
{
    .menu_name = "Set gpio interrupt",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = setgpiointerrupt_args,
        .argn = sizeof(setgpiointerrupt_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = set_gpio_interrupt_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

static arg_t setgpioconfig_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_GPIO_NUMBER,
        .msg = "Please input gpio number:"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_GPIO_DIRECTION,
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_GPIO_LEVEL,
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_GPIO_PULLUPDOWN,
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_GPIO_INTERRUPT,
    }
};

static const menu_list_t func_setgpioconfig =
{
    .menu_name = "Set gpio config",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = setgpioconfig_args,
        .argn = sizeof(setgpioconfig_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = set_gpio_config_cmd_proc,
        .methods.needRAWData = NULL,
    },
};


static arg_t setgpiowakeupenable_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_GPIO_NUMBER,
        .msg = "Please input gpio number:"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_GPIO_WAKEUPENABLE,
    }
};

static const menu_list_t func_setgpiowakeupenable =
{
    .menu_name = "Set gpio wakeup enable",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = setgpiowakeupenable_args,
        .argn = sizeof(setgpiowakeupenable_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = set_gpio_wakeupenable_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

static arg_t setgpiolevel_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_GPIO_NUMBER,
        .msg = "Please input gpio number:"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_GPIO_LEVEL,
    }
};

static const menu_list_t func_setgpiolevel =
{
    .menu_name = "Set gpio level",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = setgpiolevel_args,
        .argn = sizeof(setgpiolevel_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = set_gpio_level_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

static arg_t set_pinmux_func_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_GPIO_NUMBER,
        .msg = "Please input gpio number:"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_GPIO_PINMUXFUNC,
        .msg = "Please input pinmux function:",
    }
};

static const menu_list_t func_set_gpio_pinmux_func =
{
    .menu_name = "Set gpio pinmux function",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = set_pinmux_func_args,
        .argn = sizeof(set_pinmux_func_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = set_gpio_pinmux_func_cmd_proc,
        .methods.needRAWData = NULL,
    },
};



static arg_t get_adc_value_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_POWER_ADC_CHANNEL,
        .msg = "Please input adc channel:",
    },
};

static const menu_list_t func_get_adc_value =
{
    .menu_name = "Get adc value",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = get_adc_value_args,
        .argn = sizeof(get_adc_value_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = get_adc_value_cmd_proc,
        .methods.needRAWData = NULL,
    },
};
/*======================== for driver gpio end ========================*/

/*======================== for driver uart begin ========================*/
static arg_t uartread_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_UART_PORT,
        .msg = "uart port"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_UART_DATALEN,
        .msg = "data len"
    }
};

static const menu_list_t func_uart_read =
{
    .menu_name = "uart read data",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = uartread_args,
        .argn = sizeof(uartread_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = uartRead_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

static arg_t uartwrite_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_UART_PORT,
        .msg = "uart port"
    },
    {
        .type = TYPE_STR,
        .arg_name = ARG_UART_UARTDATA,
        .msg = "uart data"
    }
};

static const menu_list_t func_uart_write =
{
    .menu_name = "uart write data",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = uartwrite_args,
        .argn = sizeof(uartwrite_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = uartWrite_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

static arg_t uartctl_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_UART_PORT,
        .msg = "uart port"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_UART_UARTCTL,
        .msg = "uart ctl"
    }
};

static const menu_list_t func_uart_ctrl =
{
    .menu_name = "uart open/close set",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = uartctl_args,
        .argn = sizeof(uartctl_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = uartCtl_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

static arg_t uartmodectl_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_UART_MODECTL,
        .msg = "uart mode"
    },
};

static const menu_list_t func_uart_modectrl =
{
    .menu_name = "uart fifo/dma mode set",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = uartmodectl_args,
        .argn = sizeof(uartmodectl_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = uartModeCtl_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

static arg_t uartconfigset_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_UART_PORT,
        .msg = "uart port"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_UART_BAUDRATE,
        .msg = "uart baudrate"
    },
        {
        .type = TYPE_INT,
        .arg_name = ARG_UART_DATABITS,
        .msg = "uart databits"
    },
        {
        .type = TYPE_INT,
        .arg_name = ARG_UART_STOPBITS,
        .msg = "uart stopbits"
    },
        {
        .type = TYPE_INT,
        .arg_name = ARG_UART_PARITYBIT,
        .msg = "uart paritybits"
    },
};

static const menu_list_t func_uart_configset =
{
    .menu_name = "uart config set",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = uartconfigset_args,
        .argn = sizeof(uartconfigset_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = uartConfigSet_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

static arg_t uartrs485deassign_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_UART_PORT,
        .msg = "uart port"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_UART_GPIONUM,
        .msg = "gpionum"
    },
        {
        .type = TYPE_INT,
        .arg_name = ARG_UART_DELEVEL,
        .msg = "delevel"
    },
};

static const menu_list_t func_uart_rs485deassign =
{
    .menu_name = "uart rs485 depin assign",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = uartrs485deassign_args,
        .argn = sizeof(uartrs485deassign_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = uartRs485DeAssign_cmd_proc,
        .methods.needRAWData = NULL,
    },
};
/*======================== for driver uart end ========================*/
/*======================== for driver I2C begin ========================*/
static arg_t i2copen_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_I2C_port,
        .msg = "0:I2C0 1:I2C1 2:I2C2 ...",
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_I2C_clk,
        .msg = "0:STANDARD 1:FAST 2:HS 3:HS_FAST",
    },
};
static arg_t i2cport_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_I2C_port,
        .msg = "0:I2C0 1:I2C1 2:I2C2 ...",
    },
    {
        .type = TYPE_STR,
        .arg_name = ARG_SLAVE_ADDR,
        .msg = "slave write addr (0x00-0xFF)",
    },
    {
        .type = TYPE_STR,
        .arg_name = ARG_REG_ADDR,
        .msg = "0x00-0xFF",
    },
    {
        .type = TYPE_STR,
        .arg_name = ARG_REG_DATA,
        .msg = "0x00-0xFF",
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_DATA_LEN,
        .msg =  "1 or 2",
    },
};
static arg_t i2cport_r_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_I2C_port,
        .msg = "0:I2C0 1:I2C1 2:I2C2 ...",
    },
    {
        .type = TYPE_STR,
        .arg_name = ARG_SLAVE_ADDR,
        .msg = "slave write addr (0x00-0xFF)",
    },
    {
        .type = TYPE_STR,
        .arg_name = ARG_REG_ADDR,
        .msg = "0x00-0xFF",
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_DATA_LEN,
        .msg =  "1 or 2",
    },

};
static const menu_list_t func_i2cinit =
{
    .menu_name = "I2C init",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = i2copen_args,
        .argn = sizeof(i2copen_args)/sizeof(arg_t),
        .methods.cmd_handler = i2c_init_cmd_proc,
    },
};

static const menu_list_t func_i2cdeinit =
{
    .menu_name = "I2C deinit",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = i2copen_args,
        .argn = sizeof(i2copen_args)/sizeof(arg_t),
        .methods.cmd_handler = i2c_deinit_cmd_proc,
    },
};

static const menu_list_t func_i2cwrite =
{
    .menu_name = "I2C write",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = i2cport_args,
        .argn = sizeof(i2cport_args)/sizeof(arg_t),
        .methods.cmd_handler = i2c_write_cmd_proc,
    },
};

static const menu_list_t func_i2cread =
{
    .menu_name = "I2C read",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = i2cport_r_args,
        .argn = sizeof(i2cport_r_args)/sizeof(arg_t),
        .methods.cmd_handler = i2c_read_cmd_proc,
    },
};
/*======================== for driver I2C end ========================*/
/*======================== for driver SPI begin ======================*/
static arg_t spiinit_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_SPI_port,
        .msg = "1:SPI0 2:SPI2 ...",
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_SPI_clk,
        .msg = "0:6MHZ 1:13MHZ 2:26MZH 6:512KHZ",
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_SPI_mode,
        .msg = "0:PH0_PO0 1:PH0_PO1 2:PH1_PO0 3:PH1_PO1 ",
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_SPI_cs,
        .msg = "0:SPI_SYNC 1:GPIO_control 2:Cust_GPIO_control",
    },
};

static const menu_list_t func_spiinit =
{
    .menu_name = "SPI init",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = spiinit_args,
        .argn = sizeof(spiinit_args)/sizeof(arg_t),
        .methods.cmd_handler = spiinit_cmd_proc,
    },
};

static const menu_list_t func_spireadid =
{
    .menu_name = "spi read flash id",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = spiinit_args,
        .argn = sizeof(spiinit_args)/sizeof(arg_t),
        .methods.cmd_handler = spireadid_cmd_proc,
    },
};

static const menu_list_t func_spirwstatus =
{
    .menu_name = "spi flash R/W status reg",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = spiinit_args,
        .argn = sizeof(spiinit_args)/sizeof(arg_t),
        .methods.cmd_handler = spirwstatus_cmd_proc,
    },
};
/*======================== for driver SPI end =====================*/

/*======================== for driver gnss begin ==================*/
static arg_t gnss_power_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_GNSS_POWER,
        .msg = "gnss power status: Off: 0 , On: 1",
    }
};
static const menu_list_t func_gnssSetpower =
{
    .menu_name = "set gnss power status",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = gnss_power_args,
        .argn = sizeof(gnss_power_args)/sizeof(arg_t),
        .methods.cmd_handler = gnss_pwr_set_cmd_proc,
    },
};
static const menu_list_t func_gnssGetpower =
{
    .menu_name = "get gnss power status",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = gnss_pwr_get_cmd_proc,
    },
};
static arg_t nmea_data_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_GNSS_NMEA_OUT,
        .msg = "gnss nmea out ctrl: Off: 0 , On: 1",
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_GNSS_NMEA_PORT,
        .msg = "gnss nmea out port: NMEA PORT: 0 , URC: 1",
    }
};
static const menu_list_t func_gnssNmeaGet =
{
    .menu_name = "gnss nmea data get",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = nmea_data_args,
        .argn = sizeof(nmea_data_args)/sizeof(arg_t),
        .methods.cmd_handler = gnss_nmea_data_get_cmd_proc,
    },
};
static arg_t gnss_start_mode_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_GNSS_START_MODE,
        .msg = "gnss StartMode: Hot:0 , Warm:1 , Cold:2",
    }
};
static const menu_list_t func_gnssStartModeset =
{
    .menu_name = "set gnss Start Mode",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = gnss_start_mode_args,
        .argn = sizeof(gnss_start_mode_args)/sizeof(arg_t),
        .methods.cmd_handler = gnss_start_mode_set_cmd_proc,
    },
};
static arg_t gnss_baud_rate_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_GNSS_BAUD_RATE,
        .msg = "gnss baudrate: 4800, 9600, 19200, 38400, 57600, 115200, 230400",
    }
};
static arg_t gnss_nmea_rate_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_GNSS_NMEA_RATE,
        .msg = "gnss nmea rate: 1Hz, 2Hz, 3Hz, 4Hz, 5Hz, 10Hz",
    }
};
static arg_t gnss_mode_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_GNSS_MODE,
        .msg = "gnss mode: 1-15"
    }
};
static arg_t gnss_nmea_sentence_set_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_GNSS_NMEASENTENCE,
        #ifdef JACANA_GPS_SUPPORT
        .msg = "Input mask(0-511),bit0-RMC,bit1-VTG,bit2-GGA,bit3-GSA,bit4-GSV,bit5-GLL,bit6-ZDA,bit7-GST,bit8-TXT,default mask is 511(111111111),open is 1, close is 0.",
        #elif SC_DRIVER_GNSS_ZKW
        .msg = "Input mask(0-13311),bit0-GGA,bit1-GLL,bit2-GSA,bit3-GSV,bit4-RMC,bit5-VTG,bit6-ZDA,bit7-ANT,bit8-DHV,bit9-LPS,bit10-res1,bit11-res2,bit12-UTC,bit13-GST,default mask is 255(11111111),open is 1, close is 0.",
        #else
        .msg = "Input mask(0-255),bit0-GGA,bit1-GLL,bit2-GSA,bit3-GSV,bit4-RMC,bit5-VTG,bit6-ZDA,bit7-GST,default mask is 63(00111111),open is 1, close is 0.",
        #endif
    },
};
static arg_t gnss_gps_info_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_GNSS_GPS_INFO_GET,
        .msg = "gnss gps info set time: 0-255"
    }
};
static arg_t gnss_cmd_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARG_GNSS_CMD,
        .msg = "gnss cmd"
    }
};
static const menu_list_t func_gnssSetbaudrate =
{
    .menu_name = "set gnss baud rate",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = gnss_baud_rate_args,
        .argn = sizeof(gnss_baud_rate_args)/sizeof(arg_t),
        .methods.cmd_handler = gnss_baud_rate_set_cmd_proc,
    },
};
static const menu_list_t func_gnssGetMode =
{
    .menu_name = "get gnssStartMode",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = gnss_mode_get_cmd_proc,
    },
};
static const menu_list_t func_gnssGetbaudrate =
{
    .menu_name = "get gnssbaudrate",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = gnss_baud_rate_get_cmd_proc,
    },
};
static const menu_list_t func_gnssGetnmea_rate =
{
    .menu_name = "get gnssnmea_rate",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = gnss_nmea_rate_get_cmd_proc,
    },
};
static const menu_list_t func_gnssSetnmea_rate =
{
    .menu_name = "set gnss nmea rate",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = gnss_nmea_rate_args,
        .argn = sizeof(gnss_nmea_rate_args)/sizeof(arg_t),
        .methods.cmd_handler = gnss_nmea_rate_set_cmd_proc,
    },
};
static const menu_list_t func_gnssSetmode =
{
    .menu_name = "set gnss mode",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = gnss_mode_args,
        .argn = sizeof(gnss_mode_args)/sizeof(arg_t),
        .methods.cmd_handler = gnss_mode_set_cmd_proc,
    },
};
static const menu_list_t func_gnssGetnmea_sentence =
{
    .menu_name = "get gnssnmea_sentence",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = gnss_nmea_sentence_get_cmd_proc,
    },
};
static const menu_list_t func_gnssSetnmea_sentence =
{
    .menu_name = "set gnssnmea_sentence",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = gnss_nmea_sentence_set_args,
        .argn = sizeof(gnss_nmea_sentence_set_args)/sizeof(arg_t),
        .methods.cmd_handler = gnss_nmea_sentence_set_cmd_proc,
    },
};
static const menu_list_t func_gnssGetinfo =
{
    .menu_name = "get gnss gps info",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = gnss_gps_info_args,
        .argn = sizeof(gnss_gps_info_args)/sizeof(arg_t),
        .methods.cmd_handler = gnss_and_gps_info_get_cmd_proc,
    },
};
static const menu_list_t func_gnsscmdsend =
{
    .menu_name = "send gnss cmd",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = gnss_cmd_args,
        .argn = sizeof(gnss_cmd_args)/sizeof(arg_t),
        .methods.cmd_handler = gnss_cmd_send_proc,
    },
};
static const menu_list_t func_gnssSetagps =
{
    .menu_name = "set gnss agps",
    .menu_type = TYPE_FUNC,
    .parent = &api_test_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = gnss_agps_cmd_proc,
    },
};
/*======================== for driver gnss end ====================*/
const menu_list_t *API_menu_list[] = 
{
    &func_msgQ_create,
    &func_msgQ_delete,
    &func_msgQ_poll,
    &func_msgQ_recv,
    &func_msgQ_send,
	
    &func_fota_start,
    &func_app_download,
    &func_fbf_disable,
    &func_get_fota_status,
    &func_write_magic_code,

    &func_mutex_create,
    &func_mutex_lock,
    &func_mutex_unlock,
    &func_sema_create,
    &func_sema_delete,
    &func_sema_acquire,
    &func_task_create,
    &func_task_delete,
    &func_task_suspend,
    &func_task_changePrio,
    &func_task_getCurRef,
    &func_task_getEntryParam,
    &func_task_getPrio,
    &func_timer_create,
    &func_timer_delete,
    &func_timer_start,
#ifdef SIMCOM_ACC_TIMER_SUPPORT
    &func_acctimer_start,
    &func_acctimer_stop,
    &func_acctimer_delete,
#endif

#ifdef FEATURE_SIMCOM_AUDIO
    &func_audio_playFile,
    &func_audio_stopFile,
    &func_audio_rec,
    &func_audio_setAmrRateLevel,
    &func_audio_rec1,
    &func_audio_playSampleRate,
    &func_audio_pcmPlay,
    &func_audio_pcmStop,
    &func_audio_mp3StreamPlay,
    &func_audio_mp3StreamStop,
    &func_audio_amrStreamPlay,
    &func_audio_amrStreamStop,
    &func_audio_playAmrCont,
    &func_audio_wavFilePlay,
    &func_audio_status,
    &func_audio_setPlayPath,
    &func_audio_setMicGain,
    &func_audio_getMicGain,
    &func_audio_setVolume,
    &func_audio_getVolume,
    &func_audio_pcmStreamRec,
    &func_audio_pcmStreamStop,
    &func_audio_amrStreamRec,
    &func_audio_amrStreamStopRecord,
    &func_audio_playMp3Cont,
    &func_audio_playWavCont,
#endif

#if defined(FEATURE_SIMCOM_TTS_YOUNGTONE) || defined(FEATURE_SIMCOM_TTS_IFLY)
    &func_ttsPlay,
    &func_tts_setParam,
    &func_ttsStop,
    &func_tts_status_cb,
#endif

#ifdef FEATURE_SIMCOM_POC
    &func_poc_playSound,
    &func_poc_stopSound,
    &func_poc_startRecord,
    &func_poc_stopRecord,
    &func_poc_getPcmAvail_size,
    &func_poc_CleanBufferData,
    &func_poc_PcmRead,
    &func_poc_TonePlay,
    &func_poc_ToneStop,
    &func_poc_FreqPlay,
    &func_poc_SetToneVolume,
    &func_poc_getToneVolume,
#endif

    &func_wifiscan_setParam,
    &func_wifiscan_start,
    &func_wifiscan_stop,
    &func_wifiscan_setHandler,

#ifdef FEATURE_SIMCOM_FS
    &func_fs_fopen,
    &func_fs_fclose,
    &func_fs_fread,
    &func_fs_fwrite,
    &func_fs_fsync,
    &func_fs_fseek,
    &func_fs_ftell,
    &func_fs_fsize,
    &func_fs_fstat,
    &func_fs_rename,
    &func_fs_remove,
    &func_fs_mkdir,
    &func_fs_opendir,
    &func_fs_closedir,
    &func_fs_readdir,
    &func_fs_get_disk_size,
    &func_get_free_size,
    &func_get_used_size,
    &func_fs_switchdir,
#if defined (EXT_FLASH_LFSV2_ENABLE) || defined (EXT_FLASH_LFSV1_ENABLE) || defined (FEATURE_SIMCOM_SD_CARD) || defined (EXT_FLASH_YAFFS2_ENABLE)
    &func_fs_format,
#endif
#endif

    &func_tcpip_init,
    &func_tcpip_socket,
    &func_tcpip_bind,
    &func_tcpip_listen,
    &func_tcpip_accept,
    &func_tcpip_select,
    &func_tcpip_recv,
    &func_tcpip_connect,
    &func_tcpip_send,
    &func_tcpip_setsockopt,
    &func_tcpip_getsockopt,
    &func_tcpip_close,
    &func_tcpip_deinit,

    &func_httpsinit,
    &func_httpspara,
    &func_httpsdata,
    &func_httpsaction,
    &func_httpshead,
    &func_httpsread,
    &func_httpsterm,

#ifdef FEATURE_SIMCOM_HTP
    &func_htpConfig,
    &func_htpUpdate,
#endif
#ifdef FEATURE_SIMCOM_NTP
    &func_ntpUpdate,
#endif

#ifdef FEATURE_SIMCOM_SSL
    &func_sslSetContext,
    &func_sslGetContext,

    &func_crypto,
    &func_cryptoCreate,
    &func_cryptoReset,
    &func_cryptoDelete,
    &func_crypto_cfg,
    &func_crypto_get_cfg,
    &func_crypto_random,
    &func_crypto_sslTlsPrf,
#endif

#ifdef FEATURE_SIMCOM_MQTT
    &func_mqtt_start,
    &func_mqtt_accq,
    &func_mqtt_connLostCb,
    &func_mqtt_connect,
    &func_mqtt_disconnect,
    &func_mqtt_sub,
    &func_mqtt_unsub,
    &func_mqtt_topic,
    &func_mqtt_payload,
    &func_mqtt_pub,
    &func_mqtt_rel,
    &func_mqtt_stop,
    &func_mqtt_willtopic,
    &func_mqtt_willmsg,
    &func_mqtt_subtopic,
    &func_mqtt_unsubtopic,
    &func_mqtt_sslcfg,
    &func_mqtt_cfg,
#endif

#ifdef HAS_DEMO_FTPS
    &func_ftps_init,
    &func_ftps_deinit,
    &func_ftps_login,
    &func_ftps_logout,
    &func_ftps_download,
    &func_ftps_downloadToBuffer,
    &func_ftps_uploadFile,
    &func_ftps_putFile,
    &func_ftps_deleteFile,
    &func_ftps_createDir,
    &func_ftps_deleteDir,
    &func_ftps_changeDir,
    &func_ftps_getCurDir, 
    &func_ftps_list,
    &func_ftps_getFizeSize,
    &func_ftps_transferType,
    &func_ftps_getTransferType,
    &func_ftps_sslConfig,
    &func_ftps_setMode,
#endif
/*======================== for driver gpio begin ========================*/
    &func_getgpiolevel,
    &func_setgpiointerrupt,
    &func_setgpioconfig,
    &func_setgpiowakeupenable,
    &func_setgpiolevel,
    &func_set_gpio_pinmux_func,
    &func_get_adc_value,
/*======================== for driver gpio end ========================*/
/*======================== for driver uart begin ========================*/
    &func_uart_read,
    &func_uart_write,
    &func_uart_ctrl,
    &func_uart_modectrl,
    &func_uart_configset,
    &func_uart_rs485deassign,
/*======================== for driver uart end ========================*/

/*======================== for driver I2C begin =======================*/
    &func_i2cinit,
    &func_i2cdeinit,
    &func_i2cwrite,
    &func_i2cread,
/*======================== for driver I2C end ========================*/
/*======================== for driver spi begin =======================*/
    &func_spiinit,
    &func_spireadid,
    &func_spirwstatus,
/*======================== for driver spi end ========================*/
/*======================== for driver gnss begin =====================*/
    &func_gnssSetpower,
    &func_gnssGetpower,
    &func_gnssNmeaGet,
    &func_gnssStartModeset,
    &func_gnssSetbaudrate,
    &func_gnssGetMode,
    &func_gnssGetbaudrate,
    &func_gnssGetnmea_rate,
    &func_gnssSetnmea_rate,
    &func_gnssSetmode,
    &func_gnssGetnmea_sentence,
    &func_gnssSetnmea_sentence,
    &func_gnssGetinfo,
    &func_gnsscmdsend,
    &func_gnssSetagps,
/*======================== for driver gnss end =======================*/
    &func_enableDump,
    &func_disableDump,
    &func_getTicks,
    &func_sys_getVersion,
    &func_sys_powerOff,
    &func_sys_reset,
#ifdef FEATURE_SIMCOM_NTP
    &func_sys_getLocalTime,
#endif
    &func_sys_sleepSet,
    &func_sys_sleepGet,
    &func_getRealTimeClock,
    &func_setRealTimeClock,
    &func_gettimeofday,
    &func_delayUs,
    &func_malloc,
    &func_free,
    NULL
};

const menu_list_t api_test_menu =
{
    .menu_name = "API TEST",
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = API_menu_list,
    .parent = &root_menu,
};

static sMsgQRef gRtos_msgq = NULL;
static sMutexRef gRtos_mutex = NULL;
static sSemaRef gRtos_sema = NULL;
static sTaskRef gRtos_task = NULL;
static sTimerRef gRtos_timer = NULL;
static SCFILE *testFileHandle = NULL;
static SCDIR *testDirHandle = NULL;
static sMsgQRef httpsUIResp_msgq = NULL;
struct addrinfo tcp_hints;
pub_cache_p g_tcpsend_cache;
ret_msg_t g_tcpsend_result;
int tcp_sockfd = -1;
int tcp_sendsize;
sTaskRef g_tcpsend_task_ref;
int g_tcpsend_flag = 0;

ret_msg_t rtos_msgQCreate_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    const char *arg_name = NULL;
    char *queueName = NULL;
    int maxSize, maxNumber, waitingMode;

    pl_demo_get_data(&arg_name, argv, argn, ARG_MSGQ_NAME);
    queueName = (char *)arg_name;
    pl_demo_get_value(&maxSize, argv, argn, ARG_MSGQ_MAXSIZE);
    pl_demo_get_value(&maxNumber, argv, argn, ARG_MSGQ_NUMBER);
    pl_demo_get_value(&waitingMode, argv, argn, ARG_MSGQ_WAITINGMODE);
    ret.errcode = sAPI_MsgQCreate(&gRtos_msgq, queueName, maxSize, maxNumber, waitingMode);
    ret.msg = "msgQ create";

    return ret;
}

ret_msg_t rtos_msgQDelete_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    ret.errcode = sAPI_MsgQDelete(gRtos_msgq);
    ret.msg = "msgQ detele";

    return ret;
}

ret_msg_t rtos_msgQPoll_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    UINT32 count = 0;

    ret.errcode = sAPI_MsgQPoll(gRtos_msgq, &count);
    ret.msg = "msgQ poll";

    return ret;
}

ret_msg_t rtos_msgQRecv_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    SIM_MSG_T msg = {0};
    int timeout;

    pl_demo_get_value(&timeout, argv, argn, ARG_MSGQ_RECV_TIMEOUT);
    ret.errcode = sAPI_MsgQRecv(gRtos_msgq, &msg, timeout);
    ret.msg = "msgQ recv";

    return ret;
}

ret_msg_t rtos_msgQSend_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    SIM_MSG_T msg = {0};
    int msg_id, data1, data2;

    pl_demo_get_value(&msg_id, argv, argn, ARG_MSGQ_SEND_MSGID);
    pl_demo_get_value(&data1, argv, argn, ARG_MSGQ_SEND_ARG1);
    pl_demo_get_value(&data2, argv, argn, ARG_MSGQ_SEND_ARG2);
    msg.msg_id = msg_id;
    msg.arg1 = data1;
    msg.arg2 = data2;
    ret.errcode = sAPI_MsgQSend(gRtos_msgq, &msg);
    ret.msg = "msgQ send";

    return ret;
}

ret_msg_t rtos_msgQSendSuspend_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    SIM_MSG_T msg = {0};
    int msg_id, data1, data2, timeout;

    pl_demo_get_value(&msg_id, argv, argn, ARG_MSGQ_SEND_MSGID);
    pl_demo_get_value(&data1, argv, argn, ARG_MSGQ_SEND_ARG1);
    pl_demo_get_value(&data2, argv, argn, ARG_MSGQ_SEND_ARG2);
    pl_demo_get_value(&timeout, argv, argn, ARG_MSGQ_SEND_TIMEOUT);
    msg.msg_id = msg_id;
    msg.arg1 = data1;
    msg.arg2 = data2;

    ret.errcode = sAPI_MsgQSendSuspend(gRtos_msgq, &msg, timeout);
    ret.msg = "msgQ poll";

    return ret;

}

ret_msg_t set_fotabynet_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    struct SC_FotaApiParam param = {0};
    int upgrdade_mode;
    const char *url = NULL;
    const char *username = NULL;
    const char *password = NULL;

    pl_demo_get_value(&upgrdade_mode, argv, argn, ARGFOTA_MODE);

    pl_demo_get_data(&url, argv, argn, UPGRADE_URL);
    strncpy(param.host, url,sizeof(param.host));

    pl_demo_get_data(&username, argv, argn, UPGRADE_USERNAME);
    strncpy(param.username, username,sizeof(param.username));

    pl_demo_get_data(&password, argv, argn, UPGRADE_PASSWORD);
    strncpy(param.password, password,sizeof(param.password));

    param.sc_fota_cb = NULL;

    ret.errcode = sAPI_FotaServiceBegin((void *)&param);
    ret.msg = "fota upgrade by net";

    return ret;
}

ret_msg_t app_download_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    SCAppDownloadPram pram = {0};
    int download_mode;
    int recvtimeout;
    const char *url = NULL;

    pl_demo_get_value(&download_mode, argv, argn, ARG_APP_MODE);
    pl_demo_get_value(&recvtimeout, argv, argn, ARG_APP_DL_RECVTIOMEOUT);

    pl_demo_get_data(&url, argv, argn, ARG_APP_DL_URL);
    pram.url = (char *)url;

    ret.errcode = sAPI_AppDownload(&pram);
    ret.msg = "app download";

    return ret;
}

ret_msg_t fbf_disable_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};

    sAPI_FBF_Disable();
    ret.msg = "fbf disable";

    return ret;
}

ret_msg_t get_fota_status_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    #ifdef CUS_YAXON
    SC_FotaStatus status = sAPI_FotaGetPreProcessStatus();
    ret.errcode = status;
    #endif
    ret.msg = "fota get pre download status";

    return ret;
}

ret_msg_t write_magic_code_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    #ifdef CUS_YAXON
    sAPI_WriteMagicCode();
    #endif
    ret.msg = "write magic code";

    return ret;
}

ret_msg_t rtos_mutexCreate_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    int waitingMode;

    pl_demo_get_value(&waitingMode, argv, argn, ARG_MSGQ_WAITINGMODE);
    ret.errcode = sAPI_MutexCreate(&gRtos_mutex, waitingMode);
    ret.msg = "mutex create";

    return ret;

}
ret_msg_t rtos_mutexLock_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    int timeout;

    pl_demo_get_value(&timeout, argv, argn, ARG_MSGQ_RECV_TIMEOUT);
    ret.errcode = sAPI_MutexLock(gRtos_mutex, timeout);
    ret.msg = "mutex lock";

    return ret;
}

ret_msg_t rtos_mutexUnlock_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };

    ret.errcode = sAPI_MutexUnLock(gRtos_mutex);
    ret.msg = "mutex unlock";

    return ret;
}


ret_msg_t rtos_semaCreate_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    int initialCount = 0, waitingMode = 0;

    pl_demo_get_value(&initialCount, argv, argn, ARG_SEMA_INITIALCOUT);
    pl_demo_get_value(&waitingMode, argv, argn, ARG_MSGQ_WAITINGMODE);
    ret.errcode = sAPI_SemaphoreCreate(&gRtos_sema, initialCount, waitingMode);
    ret.msg = "semaphore create";

    return ret;
}

ret_msg_t rtos_semaDelete_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };

    ret.errcode = sAPI_SemaphoreDelete(gRtos_sema);
    ret.msg = "semaphore delete";

    return ret;
}

ret_msg_t rtos_semaAcquire_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    int timeout;

    pl_demo_get_value(&timeout, argv, argn, ARG_MSGQ_RECV_TIMEOUT);
    ret.errcode = sAPI_SemaphoreAcquire(gRtos_sema, timeout);
    ret.msg = "semaphore acquire";

    return ret;
}

ret_msg_t rtos_semaRelease_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };

    ret.errcode = sAPI_SemaphoreRelease(gRtos_sema);
    ret.msg = "semaphore release";

    return ret;
}

void taskStart(void *argv)
{
    return;
}

ret_msg_t rtos_taskCreate_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    int stackSize = 0, priority = 0;
    const char *arg_name = NULL;

    pl_demo_get_data(&arg_name, argv, argn, ARG_TASK_NAME);
    pl_demo_get_value(&stackSize, argv, argn, ARG_TASK_STACKSIZE);
    pl_demo_get_value(&priority, argv, argn, ARG_TASK_PRIORITY);
    ret.errcode = sAPI_TaskCreate(&gRtos_task, NULL, stackSize, priority, (char *)arg_name, taskStart, NULL);
    ret.msg = "task create";

    return ret;
}

ret_msg_t rtos_taskDelete_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };

    ret.errcode = sAPI_TaskDelete(gRtos_task);
    ret.msg = "task delete";

    return ret;
}

ret_msg_t rtos_taskSuspend_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };

    ret.errcode = sAPI_TaskSuspend(gRtos_task);
    ret.msg = "task suspend";

    return ret;
}

ret_msg_t rtos_taskChangePriority_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    int priority;
    UINT8 oldPriority;

    pl_demo_get_value(&priority, argv, argn, ARG_TASK_PRIORITY);
    ret.errcode = sAPI_TaskChangePriority(gRtos_task, priority, &oldPriority);
    ret.msg = "task change priority";

    return ret;
}

ret_msg_t rtos_taskGetEntryParam_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };

    ret.errcode = sAPI_TaskGetEntryParam(gRtos_task);
    ret.msg = "task get entry params";

    return ret;
}

ret_msg_t rtos_taskGetPriority_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    UINT8 priority = 0;

    ret.errcode = sAPI_TaskGetPriority(gRtos_task, &priority);
    ret.msg = "task get priority";

    return ret;
}

ret_msg_t rtos_taskGetCurrentRef_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    sTaskRef curRef = NULL;

    ret.errcode = sAPI_TaskGetCurrentRef(&curRef);
    ret.msg = "task get current ref";

    return ret;
}


ret_msg_t rtos_timerCreate_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };

    ret.errcode = sAPI_TimerCreate(&gRtos_timer);
    ret.msg = "timer create";

    return ret;

}

ret_msg_t rtos_timerDelete_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };

    ret.errcode = sAPI_TimerDelete(gRtos_timer);
    ret.msg = "timer delete";

    return ret;
}

void timerTest(UINT32 argv)
{
    TEST_LOG_INFO("gRtos_timer timeout ");
}

ret_msg_t rtos_timerStart_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    int initialTime = 0, rescheduleTime = 0;

    pl_demo_get_value(&initialTime, argv, argn, ARG_TIMER_INITIALTIME);
    pl_demo_get_value(&rescheduleTime, argv, argn, ARG_TIMER_RESCHEDULETIME);
    ret.errcode = sAPI_TimerStart(gRtos_timer, initialTime, rescheduleTime, timerTest, 0);
    ret.msg = "timer delete";

    return ret;
}

void acctimerCb(UINT32 argv)
{
    TEST_LOG_INFO("accTimer timeout");
}

#ifdef SIMCOM_ACC_TIMER_SUPPORT
ret_msg_t rtos_accTimerStart_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    int timerFlag;//SC_ACC_TIMER_FLAG
    int period = 0, rescheduleTime = 0;

    pl_demo_get_value(&timerFlag, argv, argn, ARG_ACCTIMER_FLAG);
    pl_demo_get_value(&period, argv, argn, ARG_ACCTIMER_PERIOD);
    ret.errcode = sAPI_AccTimerStart(timerFlag, period, acctimerCb, 0);
    ret.msg = "acctimer start";

    return ret;
}


ret_msg_t rtos_accTimerStop_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    int timerId = 0;

    pl_demo_get_value(&timerId, argv, argn, ARG_ACCTIMER_TIMERID);
    ret.errcode = sAPI_AccTimerStop(timerId);
    ret.msg = "acctimer stop";

    return ret;
}

ret_msg_t rtos_accTimerDelete_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    int timerId = 0;

    pl_demo_get_value(&timerId, argv, argn, ARG_ACCTIMER_TIMERID);
    ret.errcode = sAPI_AccTimerDelete(timerId);
    ret.msg = "acctimer delete";

    return ret;
}
#endif

#ifdef FEATURE_SIMCOM_AUDIO
ret_msg_t audio_play_file_cmd_proc(op_t op, arg_t *argv, int argn)
{
    TEST_LOG_INFO("audio_play_file_cmd_proc enter!");
    ret_msg_t ret = { 0 };
    const char *filename = NULL;
    char file[AUDIO_FILENAME_MAX_LEN] = { 0 };
    int direct = 0;
    int issingle = 1;

    pl_demo_get_data(&filename, argv, argn, ARG_AUDIO_FILENAME);
    memcpy(file, filename, strlen(filename));
    pl_demo_get_value(&direct, argv, argn, ARG_AUDIO_DIRECTPLAY);
    pl_demo_get_value(&issingle, argv, argn, ARG_AUDIO_ISSINGLE);

    if(sAPI_AudioPlay(file, direct, issingle))
        ret.errcode = 0;
    else
        ret.errcode = -1;
    ret.msg = "audio play start";

    return ret;
}

ret_msg_t audio_stop_file_cmd_proc(op_t op, arg_t *argv, int argn)
{
    TEST_LOG_INFO("audio_stop_file_cmd_proc enter!");
    ret_msg_t ret = { 0 };

    if(sAPI_AudioStop())
        ret.errcode = 0;
    else
        ret.errcode = -1;
    ret.msg = "audio stop";

    return ret;
}

ret_msg_t audio_record_cmd_proc(op_t op, arg_t *argv, int argn)
{
    TEST_LOG_INFO("audio_record_cmd_proc enter!");
    ret_msg_t ret = { 0 };
    int enable, path;
    const char *filename = NULL;
    char file[AUDIO_FILENAME_MAX_LEN] = { 0 };

    pl_demo_get_value(&enable, argv, argn, ARG_AUDIO_ENABLE);
    pl_demo_get_value(&path, argv, argn, ARG_AUDIO_RECORDPATH);
    pl_demo_get_data(&filename, argv, argn, ARG_AUDIO_FILENAME);
    memcpy(file, filename, strlen(filename));

    if(sAPI_AudioRecord(enable, path, file))
        ret.errcode = 0;
    else
        ret.errcode = -1;
    ret.msg = "audio record";

    return ret;
}

ret_msg_t audio_set_amr_rate_level_cmd_proc(op_t op, arg_t *argv, int argn)
{
    TEST_LOG_INFO("audio_set_amr_rate_level_cmd_proc enter!");
    ret_msg_t ret = { 0 };
    int level = 0;

    pl_demo_get_value(&level, argv, argn, ARG_AUDIO_AMR_LEVEL);

    sAPI_AudioSetAmrRateLevel(level);
    ret.errcode = 0;
    ret.msg = "AMR rate level set";

    return ret;
}

void recordCallBac_Test(UINT8 status)
{
    TEST_LOG_INFO("%s status is %d!", __func__, status);
}
ret_msg_t audio_rec_cmd_proc(op_t op, arg_t *argv, int argn)
{
    TEST_LOG_INFO("audio_rec_cmd_proc enter!");
    ret_msg_t ret = { 0 };
    int oper = 0;
    char file[AUDIO_FILENAME_MAX_LEN] = { 0 };
    const char *filename = NULL;
    int duration = 0;

    pl_demo_get_value(&oper, argv, argn, ARG_AUDIO_REC_OPER);
    pl_demo_get_data(&filename, argv, argn, ARG_AUDIO_FILENAME);
    memcpy(file, filename, strlen(filename));
    pl_demo_get_value(&duration, argv, argn, ARG_AUDIO_REC_DURATION);

    if(sAPI_AudRec((UINT8)oper, file, (UINT8)duration, recordCallBac_Test))
        ret.errcode = 0;
    else
        ret.errcode = -1;
    ret.msg = "simple audio record operation";

    return ret;
}

ret_msg_t audio_play_sample_rate_cmd_proc(op_t op, arg_t *argv, int argn)
{
    TEST_LOG_INFO("audio_play_sample_rate_cmd_proc enter!");
    ret_msg_t ret = { 0 };
    int rate = 0;

    pl_demo_get_value(&rate, argv, argn, ARG_AUDIO_SAMPLE_RATE);

    if(sAPI_AudioPlaySampleRate(rate))
        ret.errcode = 0;
    else
        ret.errcode = -1;
    ret.msg = "sample rate set";

    return ret;
}

ret_msg_t audio_pcm_play_cmd_proc(op_t op, arg_t *argv, int argn)
{
    TEST_LOG_INFO("audio_pcm_play_cmd_proc enter!");
    ret_msg_t ret = { 0 };
    const char *pcm_data = NULL;
    int len = 0;
    int direct = FALSE;

    pl_demo_get_data(&pcm_data, argv, argn, ARG_AUDIO_PCM_DATA);
    pl_demo_get_value(&len, argv, argn, ARG_AUDIO_PCM_LEN);
    pl_demo_get_value(&direct, argv, argn, ARG_AUDIO_DIRECTPLAY);

    if(sAPI_AudioPcmPlay((char *)pcm_data, (UINT32)len, (BOOL)direct))
        ret.errcode = 0;
    else
        ret.errcode = -1;
    ret.msg = "PCM play started";

    return ret;
}

ret_msg_t audio_pcm_stop_cmd_proc(op_t op, arg_t *argv, int argn)
{
    TEST_LOG_INFO("audio_pcm_stop_cmd_proc enter!");
    ret_msg_t ret = {0};

    if(sAPI_AudioPcmStop())
        ret.errcode = 0;
    else
        ret.errcode = -1;
    ret.msg = "PCM playback stopped";

    return ret;
}

ret_msg_t audio_mp3_stream_play_cmd_proc(op_t op, arg_t *argv, int argn)
{
    TEST_LOG_INFO("audio_mp3_stream_play_cmd_proc enter!");
    ret_msg_t ret = {0};
    const char *mp3_data = NULL;
    int len = 0;
    int direct = FALSE;

    pl_demo_get_data(&mp3_data, argv, argn, ARG_AUDIO_MP3_DATA);
    pl_demo_get_value(&len, argv, argn, ARG_AUDIO_MP3_LEN);
    pl_demo_get_value(&direct, argv, argn, ARG_AUDIO_DIRECTPLAY);

    if(sAPI_AudioMp3StreamPlay((char *)mp3_data, (UINT32)len, (BOOL)direct))
        ret.errcode = 0;
    else
        ret.errcode = -1;
    ret.msg = "MP3 stream play started";

    return ret;
}

ret_msg_t audio_amr_stream_play_cmd_proc(op_t op, arg_t *argv, int argn)
{
    TEST_LOG_INFO("audio_amr_stream_play_cmd_proc enter!");
    ret_msg_t ret = {0};
    const char *amr_data = NULL;
    int len = 0;
    int direct = FALSE;

    pl_demo_get_data(&amr_data, argv, argn, ARG_AUDIO_AMR_DATA);
    pl_demo_get_value(&len, argv, argn, ARG_AUDIO_AMR_LEN);
    pl_demo_get_value(&direct, argv, argn, ARG_AUDIO_DIRECTPLAY);

    if(sAPI_AudioAmrStreamPlay((char *)amr_data, (UINT32)len, (BOOL)direct))
        ret.errcode = 0;
    else
        ret.errcode = -1;
    ret.msg = "AMR stream play started";

    return ret;
}

ret_msg_t audio_mp3_stream_stop_cmd_proc(op_t op, arg_t *argv, int argn)
{
    TEST_LOG_INFO("audio_mp3_stream_stop_cmd_proc enter!");
    ret_msg_t ret = {0};

    if(sAPI_AudioMp3StreamStop())
        ret.errcode = 0;
    else
        ret.errcode = -1;
    ret.msg = "MP3 stream stopped";

    return ret;
}

ret_msg_t audio_amr_stream_stop_cmd_proc(op_t op, arg_t *argv, int argn)
{
    TEST_LOG_INFO("audio_amr_stream_stop_cmd_proc enter!");
    ret_msg_t ret = {0};

    if(sAPI_AudioAmrStreamStop())
        ret.errcode = 0;
    else
        ret.errcode = -1;
    ret.msg = "AMR stream stopped";

    return ret;
}

ret_msg_t audio_status_cmd_proc(op_t op, arg_t *argv, int argn)
{
    TEST_LOG_INFO("audio_status_cmd_proc enter!");
    ret_msg_t ret = {0};
    int status = 0;

    status = sAPI_AudioStatus();
    ret.errcode = status;
    ret.msg = "audio status retrieved";

    return ret;
}

ret_msg_t audio_play_amr_cont_cmd_proc(op_t op, arg_t *argv, int argn)
{
    TEST_LOG_INFO("audio_play_amr_cont_cmd_proc enter!");
    ret_msg_t ret = {0};
    const char *filename = NULL;
    char file[AUDIO_FILENAME_MAX_LEN] = {0};
    int startplay = FALSE;

    pl_demo_get_data(&filename, argv, argn, ARG_AUDIO_FILENAME);
    memcpy(file, filename, strlen(filename));
    pl_demo_get_value(&startplay, argv, argn, ARG_AUDIO_STARTPLAY);

    if(sAPI_AudioPlayAmrCont(file, (BOOL)startplay))
        ret.errcode = 0;
    else
        ret.errcode = -1;
    ret.msg = startplay ? "AMR continuous play started" : "AMR continuous play stopped";

    return ret;
}

ret_msg_t audio_wav_file_play_cmd_proc(op_t op, arg_t *argv, int argn)
{
    TEST_LOG_INFO("audio_wav_file_play_cmd_proc enter!");
    ret_msg_t ret = {0};
    const char *filename = NULL;
    char file[AUDIO_FILENAME_MAX_LEN] = {0};
    int direct = FALSE;

    pl_demo_get_data(&filename, argv, argn, ARG_AUDIO_FILENAME);
    memcpy(file, filename, strlen(filename));
    pl_demo_get_value(&direct, argv, argn, ARG_AUDIO_DIRECTPLAY);

    if(sAPI_AudioWavFilePlay(file, (BOOL)direct))
        ret.errcode = 0;
    else
        ret.errcode = -1;
    ret.msg = "WAV file play started";

    return ret;
}

ret_msg_t audio_set_play_path_cmd_proc(op_t op, arg_t *argv, int argn)
{
    TEST_LOG_INFO("audio_set_play_path_cmd_proc enter!");
    ret_msg_t ret = {0};
    int path = 0;

    pl_demo_get_value(&path, argv, argn, ARG_AUDIO_PLAY_PATH);

    if(sAPI_AudioSetPlayPath((UINT8)path))
        ret.errcode = 0;
    else
        ret.errcode = -1;
    ret.msg = "audio play path set";

    return ret;
}

ret_msg_t audio_set_mic_gain_cmd_proc(op_t op, arg_t *argv, int argn)
{
    TEST_LOG_INFO("audio_set_mic_gain_cmd_proc enter!");
    ret_msg_t ret = {0};
    int micgain = 0;

    pl_demo_get_value(&micgain, argv, argn, ARG_AUDIO_MIC_GAIN);

    sAPI_AudioSetMicGain((UINT32)micgain);
    ret.errcode = 0;
    ret.msg = "mic gain set";

    return ret;
}

ret_msg_t audio_get_mic_gain_cmd_proc(op_t op, arg_t *argv, int argn)
{
    TEST_LOG_INFO("audio_get_mic_gain_cmd_proc enter!");
    ret_msg_t ret = {0};
    arg_t out_arg[1] = {0};
    char msg[10] = {0};
    int gain = 0;

    gain = sAPI_AudioGetMicGain();

    snprintf(msg, sizeof(msg), "%d", gain);
    out_arg[0].type = TYPE_RAW;
    out_arg[0].arg_name = "mic gain";
    out_arg[0].value = pl_demo_make_value(0, msg, strlen(msg));

    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
    pl_demo_release_value(out_arg[0].value);
    if (0 != ret.errcode)
    {
        ret.msg = "post data fail";
    }

    return ret;
}

ret_msg_t audio_set_volume_cmd_proc(op_t op, arg_t *argv, int argn)
{
    TEST_LOG_INFO("audio_set_volume_cmd_proc enter!");
    ret_msg_t ret = {0};
    int volume = 0;

    pl_demo_get_value(&volume, argv, argn, ARG_AUDIO_VOLUME);

    sAPI_AudioSetVolume(volume);
    ret.errcode = 0;
    ret.msg = "audio volume set";

    return ret;
}

ret_msg_t audio_get_volume_cmd_proc(op_t op, arg_t *argv, int argn)
{
    TEST_LOG_INFO("audio_get_volume_cmd_proc enter!");
    ret_msg_t ret = {0};
    arg_t out_arg[1] = {0};
    char msg[30] = {0};
    int volume = 0;

    volume = sAPI_AudioGetVolume();

    snprintf(msg, sizeof(msg), "%d", volume);
    out_arg[0].type = TYPE_RAW;
    out_arg[0].arg_name = "audio volume";
    out_arg[0].value = pl_demo_make_value(0, msg, strlen(msg));

    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
    pl_demo_release_value(out_arg[0].value);
    if (0 != ret.errcode)
    {
        ret.msg = "post data fail";
    }

    return ret;
}

void get_amr_frame(const UINT8 *buf, UINT32 size)
{
    for (int i = 0; i < size; i++)
    {
        TEST_LOG_INFO("buf[%d] = %x", i, buf[i]);
    }
}
void get_pcm_rec(const UINT8 *buf, UINT32 size)
{
    for (int i = 0; i < 1; i++)
    {
        TEST_LOG_INFO("buf[%d] = %x", i, buf[i]);
        TEST_LOG_INFO("buf[%ld] = %x", size - 1, buf[size - 1]);
    }
    /*  This annotation is turned on when testing the PCM flow
        int ret = sAPI_fwrite(buf, 1, size, fp);
        if(ret == 0)
        {
            TEST_LOG_INFO("write failed: %d", ret);
        }
    */
}

ret_msg_t audio_pcm_stream_rec_cmd_proc(op_t op, arg_t *argv, int argn)
{
    TEST_LOG_INFO("audio_pcm_stream_rec_cmd_proc enter!");
    ret_msg_t ret = { 0 };

    if (sAPI_PCMStreamRec(get_pcm_rec))
    {
        ret.errcode = 0;
        ret.msg = "PCM stream record with callback started";
    }
    else
    {
        ret.errcode = -1;
        ret.msg = "Start PCM stream record failed";
    }

    return ret;
}

ret_msg_t audio_pcm_stream_stop_cmd_proc(op_t op, arg_t *argv, int argn)
{
    TEST_LOG_INFO("audio_pcm_stream_stop_cmd_proc enter!");
    ret_msg_t ret = { 0 };

    if (sAPI_PCMStopStreamRec())
    {
        ret.errcode = 0;
        ret.msg = "PCM stream record stopped";
    }
    else
    {
        ret.errcode = -1;
        ret.msg = "Stop PCM stream record failed";
    }

    return ret;
}

ret_msg_t audio_amr_stream_rec_cmd_proc(op_t op, arg_t *argv, int argn)
{
    TEST_LOG_INFO("audio_amr_stream_rec_cmd_proc enter!");
    ret_msg_t ret = { 0 };

    if (sAPI_AmrStreamRecord(get_amr_frame))
    {
        ret.errcode = 0;
        ret.msg = "AMR stream record with callback started";
    }
    else
    {
        ret.errcode = -1;
        ret.msg = "Start AMR stream record failed";
    }

    return ret;
}

ret_msg_t audio_amr_stream_stop_record_cmd_proc(op_t op, arg_t *argv, int argn)
{
    TEST_LOG_INFO("audio_pcm_stream_stop_cmd_proc enter!");
    ret_msg_t ret = { 0 };

    if (sAPI_AmrStopStreamRecord())
    {
        ret.errcode = 0;
        ret.msg = "PCM stream record stopped";
    }
    else
    {
        ret.errcode = -1;
        ret.msg = "Stop PCM stream record failed";
    }

    return ret;
}

ret_msg_t audio_play_mp3_cont_cmd_proc(op_t op, arg_t *argv, int argn)
{
    TEST_LOG_INFO("audio_play_mp3_cont_cmd_proc enter!");
    ret_msg_t ret = {0};
    const char *filename = NULL;
    char file[AUDIO_FILENAME_MAX_LEN] = {0};
    int startplay = 0;
    int framemode = 0;

    pl_demo_get_data(&filename, argv, argn, ARG_AUDIO_FILENAME);
    if(filename) {
        memcpy(file, filename, strlen(filename));
    }
    pl_demo_get_value(&startplay, argv, argn, ARG_AUDIO_STARTPLAY);
    pl_demo_get_value(&framemode, argv, argn, ARG_AUDIO_FRAME_MODE);

    if(sAPI_AudioPlayMp3Cont(file, startplay, framemode)) {
        ret.errcode = 0;
        ret.msg = startplay ? "MP3 continuous play started" : "MP3 continuous play stopped";
    } else {
        ret.errcode = -1;
        ret.msg = startplay ? "Start MP3 continuous play failed" : "Stop MP3 continuous play failed";
    }

    return ret;
}

ret_msg_t audio_play_wav_cont_cmd_proc(op_t op, arg_t *argv, int argn)
{
    TEST_LOG_INFO("audio_play_wav_cont_cmd_proc enter!");
    ret_msg_t ret = {0};
    const char *filename = NULL;
    char file[AUDIO_FILENAME_MAX_LEN] = {0};
    int startplay = 0;

    pl_demo_get_data(&filename, argv, argn, ARG_AUDIO_FILENAME);
    if(filename) {
        memcpy(file, filename, strlen(filename));
    }
    pl_demo_get_value(&startplay, argv, argn, ARG_AUDIO_STARTPLAY);

    if(sAPI_AudioPlayWavCont(file, startplay)) {
        ret.errcode = 0;
        ret.msg = startplay ? "WAV continuous play started" : "WAV continuous play stopped";
    } else {
        ret.errcode = -1;
        ret.msg = startplay ? "Start WAV continuous play failed" : "Stop WAV continuous play failed";
    }

    return ret;
}
#endif

#if defined(FEATURE_SIMCOM_TTS_YOUNGTONE) || defined(FEATURE_SIMCOM_TTS_IFLY)
ret_msg_t tts_play_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    int option, mode;
    const char *textStr = NULL;

    pl_demo_get_value(&option, argv, argn, ARG_TTS_TEXT_TYPE);
    pl_demo_get_data(&textStr, argv, argn, ARG_TTS_TEXT_STR);
    pl_demo_get_value(&mode, argv, argn, ARG_TTS_MODE);

    if(sAPI_TTSPlay(option, (char *)textStr, mode))
        ret.errcode = 0;
    else
        ret.errcode = -1;
    ret.msg = "tts play";

    return ret;
}

ret_msg_t tts_setParam_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    int sysVolume, ttsVolume, digitMode, pitch, speed;

    pl_demo_get_value(&sysVolume, argv, argn, ARG_TTS_PARAM_SYSVOLUME);
    pl_demo_get_value(&ttsVolume, argv, argn, ARG_TTS_PARAM_TTSVOLUME);
    pl_demo_get_value(&digitMode, argv, argn, ARG_TTS_PARAM_DIGITMODE);
    pl_demo_get_value(&pitch, argv, argn, ARG_TTS_PARAM_PITCH);
    pl_demo_get_value(&speed, argv, argn, ARG_TTS_PARAM_SPEED);

    if(sAPI_TTSSetParameters((UINT8)sysVolume, (UINT8)ttsVolume, (UINT8)digitMode, (UINT8)pitch, (UINT8)speed))
        ret.errcode = 0;
    else
        ret.errcode = -1;
    ret.msg = "tts set parameters";

    return ret;
}

ret_msg_t tts_stop_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };

    if(sAPI_TTSStop())
        ret.errcode = 0;
    else
        ret.errcode = -1;
    ret.msg = "tts stop playing";

    return ret;
}

void ttsStatusCallback(TTSStatus flag)
{
    TEST_LOG_INFO("ttst staus[%d]", flag);
}

ret_msg_t tts_setStatusCb_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };

    sAPI_TTSSetStatusCallBack(ttsStatusCallback);
    ret.errcode = 0;
    ret.msg = "tts set status callback";

    return ret;
}
#endif

#ifdef FEATURE_SIMCOM_POC
#define POC_PLAY_BUFFER_FRAMS_MAX   20

static char *g_poc_data_buf = NULL;
static int g_poc_data_size;
static sTaskRef g_poc_task_ref;
static sal_sema_ref poc_syn_ref;
static SCFILE * g_record_fd = NULL;

static ret_msg_t poc_uninit(void)
{
    TEST_LOG_INFO("poc_uninit enter!");
    ret_msg_t ret = { 0 };

    if (poc_syn_ref)
    {
        ret.errcode = sal_semaphore_delete(&poc_syn_ref);
    }

    return ret;
}

static ret_msg_t poc_init(void)
{
    TEST_LOG_INFO("poc_init enter!");
    ret_msg_t ret = { 0 };
    poc_uninit();

    if (0 == sal_semaphore_create(&poc_syn_ref, 0))
    {
        ret.errcode = ERRCODE_OK;
        ret.msg = "audio success to create semaphore";
    }
    else
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "audio fail to create semaphore";
    }

    return ret;
}

static ret_msg_t poc_play_sound_raw_proc(int size, int (*raw_read)(char *buf, unsigned int buf_size, int timeout_ms))
{
    ret_msg_t ret = { 0 };
    int read_size = 0;
    int remain_size = size;
    int copy_size = 0;
    char buf[POC_PLAY_BUFFER_FRAMS_MAX *320 + 1] = { 0 };

    if (g_poc_data_size != size)
    {
        TEST_LOG_ERROR("raw data size invalid");
        TEST_LOG_ERROR("size: %d, need: %d", size, g_poc_data_size);
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "raw data size invalid";
        goto EXIT;
    }

    g_poc_data_buf = sal_malloc(g_poc_data_size + 1);
    if (!g_poc_data_buf)
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "malloc fail";
        goto EXIT;
    }

    while (remain_size > 0)
    {
        read_size = remain_size <= POC_PLAY_BUFFER_FRAMS_MAX *320 ? remain_size : POC_PLAY_BUFFER_FRAMS_MAX *320;
        read_size = raw_read(buf, read_size, 10000);
        if (read_size < 0)
        {
            TEST_LOG_ERROR("raw data read fail");
            ret.errcode = read_size < 0 ? read_size : ERRCODE_DEFAULT;
            ret.msg = "raw data read fail";
            goto EXIT;
        }
        else if (0 == read_size)
        {
            TEST_LOG_ERROR("raw data read timeout");
            ret.errcode = ERRCODE_RAW_RECV_TIMEOUT;
            ret.msg = "raw data read timeout";
            goto EXIT;
        }

        memcpy(g_poc_data_buf + copy_size, buf, read_size);

        copy_size += read_size;
        remain_size -= read_size;
        TEST_LOG_INFO("copy_size:[%d], remain_size:[%d], read_size:[%d]", copy_size, remain_size, read_size);
    }

    sal_semaphore_release(poc_syn_ref);
    return ret;

EXIT:
    sal_free(g_poc_data_buf);
    sal_semaphore_release(poc_syn_ref);
    return ret;
}

static void poc_play_sound_task(void *args)
{
    TEST_LOG_INFO("poc_play_sound_task enter!");
    ret_msg_t ret = { 0 };

    sal_semaphore_acquire(poc_syn_ref, SAL_OS_TIMEOUT_SUSPEND);

    if (!g_poc_data_buf)
    {
        TEST_LOG_ERROR("g_poc_data_buf error! poc_data_size = %d", g_poc_data_size);
        goto EXIT;
    }

    ret.errcode = sAPI_PocPlaySound(g_poc_data_buf, g_poc_data_size);
    if (ret.errcode)
    {
        TEST_LOG_ERROR("Poc fail to Play Sound, ret.errcode = %d", ret.errcode);
    }

EXIT:
    sal_free(g_poc_data_buf);
    g_poc_data_size = 0;
}

static int poc_play_sound_need_size(void)
{
    return g_poc_data_size;
}

static ret_msg_t poc_play_sound_cmd_proc(op_t op, arg_t *argv, int argn)
{
    TEST_LOG_INFO("poc_play_sound_cmd_proc enter!");
    ret_msg_t ret = { 0 };
    int poc_data_size = 0;

    pl_demo_get_value(&poc_data_size, argv, argn, ARGPOC_SIZE);
    if (poc_data_size < 0 || (poc_data_size % 320 != 0))
    {
        TEST_LOG_ERROR("poc_data_size param invalid!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "poc_data_size param invalid";
        return ret;
    }

    g_poc_data_size = poc_data_size;

    ret.errcode = sal_task_create(&g_poc_task_ref, NULL, SAL_4K, sal_task_priority_low_1, "poc play sound", poc_play_sound_task, NULL);
    if (ret.errcode != SC_SUCCESS)
    {
        ret.msg = "task creat fail";
    }
    sal_task_detach(&g_poc_task_ref);

    return ret;
}

static ret_msg_t poc_stop_sound_cmd_proc(op_t op, arg_t *argv, int argn)
{
    TEST_LOG_INFO("poc_stop_sound_cmd_proc enter!");
    ret_msg_t ret = { 0 };

    if (!sAPI_PocStopSound())
    {
        ret.errcode = ERRCODE_OK;
        ret.msg = "audio success to stop sound";
    }
    else
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "audio fail to stop sound";
    }

    return ret;
}

static int GetRecordCb(char *buffer, int len)
{
    TEST_LOG_INFO("GetRecordCb enter!");
    int write_size = 0;

    if (NULL == buffer || 0 == len)
    {
        TEST_LOG_ERROR("data is NULL!");
        return -1;
    }

    write_size = sAPI_fwrite(buffer, 1, len, g_record_fd);
    if (write_size != len)
    {
        TEST_LOG_ERROR("write buf failed! write_size = %d, need_size = %d", write_size, len);
        return -2;
    }

    return 0;
}

static ret_msg_t poc_start_record_cmd_proc(op_t op, arg_t *argv, int argn)
{
    TEST_LOG_INFO("poc_start_record_cmd_proc enter!");
    ret_msg_t ret = { 0 };
    int mode = 1;

    pl_demo_get_value(&mode, argv, argn, ARGPOC_MODE);
    if (mode < 1 || mode > 2)
    {
        TEST_LOG_ERROR("mode param invalid!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "mode param invalid";
        return ret;
    }
    TEST_LOG_INFO("mode = %d", mode);

    g_record_fd = sAPI_fopen("c:/poc_record_test.wav", "ab+");
    if (mode == 2)
    {
        ret.errcode = sAPI_PocStartRecord(NULL, 2);
        ret.msg = "Get data passively";
    }
    else if (mode == 1)
    {
        ret.errcode = sAPI_PocStartRecord(GetRecordCb, 1);
        ret.msg = "Get data forwordly";
    }

    return ret;
}

static ret_msg_t poc_stop_record_cmd_proc(op_t op, arg_t *argv, int argn)
{
    TEST_LOG_INFO("poc_stop_record_cmd_proc enter!");
    ret_msg_t ret = { 0 };

    if (!sAPI_PocStopRecord())
    {
        sAPI_fclose(g_record_fd);
        g_record_fd = NULL;
        ret.errcode = ERRCODE_OK;
        ret.msg = "audio success to stop record";
    }
    else
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "audio fail to stop record";
    }

    return ret;
}

static ret_msg_t poc_getPcmAvailSize_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int total_size = 0;
    arg_t out_arg[1] = {0};

    total_size = sAPI_PocGetPcmAvail();

    out_arg[0].type = TYPE_INT;
    out_arg[0].arg_name = "Get PcmAvail size";
    out_arg[0].value = pl_demo_make_value(total_size, NULL, 0);
    pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
    pl_demo_release_value(out_arg[0].value);

    if(total_size > 0)
        ret.errcode = 0;
    else
        ret.errcode = -1;

    return ret;
}

static ret_msg_t poc_CleanBufferData_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};

    ret.errcode = sAPI_PocCleanBufferData();
    return ret;
}

static ret_msg_t poc_PcmRead_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int dataLen = 0;
    arg_t out_arg[1] = {0};
    char *buff = NULL;

    pl_demo_get_value(&dataLen, argv, argn, ARG_PCMREAD_PARAM_VALUE);
    out_arg[0].type = TYPE_INT;
    out_arg[0].arg_name = "Get PcmRead size";    

    buff = sAPI_Malloc(dataLen+1);
    if(buff == NULL)
    {
        ret.msg = "sAPI_Malloc test fail";
        ret.errcode = 1;
        goto EXIT;
    }
    ret.errcode = sAPI_PocPcmRead(buff, dataLen);
    
    out_arg[0].value = pl_demo_make_value(0, buff, dataLen);
    pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
    pl_demo_release_value(out_arg[0].value);
    sAPI_Free(buff);
    
    EXIT:
    return ret;
}

static ret_msg_t poc_TonePlay_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    int type, time;

    pl_demo_get_value(&type, argv, argn, ARG_TONEPALY_PARAM_TYPE);
    pl_demo_get_value(&time, argv, argn, ARG_TONEPALY_PARAM_TIME);

    ret.errcode = sAPI_TonePlay(type, time);
    return ret;
}

static ret_msg_t poc_ToneStop_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};

    sAPI_ToneStop();
    return ret;
}

static ret_msg_t poc_FreqPlay_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    int freq1, freq2, time;

    pl_demo_get_value(&freq1, argv, argn, ARG_FREQPLAY1_PARAM_VALUE);
    pl_demo_get_value(&freq2, argv, argn, ARG_FREQPLAY2_PARAM_VALUE);
    pl_demo_get_value(&time, argv, argn, ARG_FREQPLAY_PARAM_TIME);

    ret.errcode = sAPI_FreqPlay(freq1, freq2, time);
    return ret;
}

static ret_msg_t poc_SetToneVolume_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    int volume;

    pl_demo_get_value(&volume, argv, argn, ARG_VOLUME_PARAM_VALUE);

    sAPI_SetToneVolume(volume);
    return ret;
}

static ret_msg_t poc_getToneVolume_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int volume = 0;
    arg_t out_arg[1] = {0};

    volume = sAPI_GetToneVolume();

    out_arg[0].type = TYPE_INT;
    out_arg[0].arg_name = "Get ToneVolume";
    out_arg[0].value = pl_demo_make_value(volume, NULL, 0);
    pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
    pl_demo_release_value(out_arg[0].value);

    if(volume > 0)
        ret.errcode = 0;
    else
        ret.errcode = -1;

    return ret;
}
#endif

ret_msg_t wifiscan_setParam_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    SC_WIFI_SCANPARAM_T param = {0};
    int round_number, max_bssid_number, timeout, priority;

    pl_demo_get_value(&round_number, argv, argn, ARG_WIFISCAN_PARAM_ROUNDNUMBER);
    pl_demo_get_value(&max_bssid_number, argv, argn, ARG_WIFISCAN_PARAM_BSSIDNUMBER);
    pl_demo_get_value(&timeout, argv, argn, ARG_WIFISCAN_PARAM_TIMEOUT);
    pl_demo_get_value(&priority, argv, argn, ARG_WIFISCAN_PARAM_PRIORITY);
    param.round_number = round_number;
    param.max_bssid_number = max_bssid_number;
    param.timeout = timeout;
    param.priority = priority;
    sAPI_WifiSetScanParam(&param);
    ret.errcode = 0;
    ret.msg = "set wifiscan parameters";

    return ret;
}

ret_msg_t wifiscan_start_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };

    sAPI_WifiScanStart();
    ret.errcode = 0;
    ret.msg = "wifiscan start";

    return ret;
}

ret_msg_t wifiscan_stop_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };

    sAPI_WifiScanStop();
    ret.errcode = 0;
    ret.msg = "wifiscan stop";

    return ret;
}

void wifiscan_event_callback(const SC_WIFI_EVENT_T * param)
{
    char rspBuf[128] = {0};
	arg_t out_arg[3] = {0};

    switch (param->type)
    {
        case SC_WIFI_EVENT_SCAN_INFO:
            snprintf(rspBuf, sizeof(rspBuf),
                     "\r\nscan result: mac address(%02x:%02x:%02x:%02x:%02x:%02x) channel(%d) rssi(%d).\r\n",
                     param->info.mac_addr[5], param->info.mac_addr[4], param->info.mac_addr[3],
                     param->info.mac_addr[2], param->info.mac_addr[1], param->info.mac_addr[0],
                     param->info.channel_number, param->info.rssi);
            break;
        case SC_WIFI_EVENT_SCAN_STOP:
            snprintf(rspBuf, sizeof(rspBuf), "\r\nscan stop.\r\n");
            break;
        default:
            snprintf(rspBuf, sizeof(rspBuf), "\r\ninvalid wifi event.\r\n");
    }

    out_arg[0].arg_name = "scan result",
    out_arg[0].type = TYPE_RAW,
    out_arg[0].value = pl_demo_make_value(0, NULL, strlen(rspBuf));
    pl_demo_post_urc(&func_wifiscan_start, TYPE_URC_DATA, out_arg, 1, rspBuf, strlen(rspBuf));
    pl_demo_release_value(out_arg[0].value);

    TEST_LOG_INFO("%s", (char *)rspBuf);
}


ret_msg_t wifiscan_setHandler_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };

    sAPI_WifiSetHandler(wifiscan_event_callback);
    ret.errcode = 0;
    ret.msg = "set wifiscan callback";

    return ret;
}

#ifdef FEATURE_SIMCOM_FS
ret_msg_t fs_fopen_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    const char *path = NULL;
    const char *mode = NULL;

    pl_demo_get_data(&path, argv, argn, ARG_FS_PATH);
    pl_demo_get_data(&mode, argv, argn, ARG_FS_FILEMODE);

    testFileHandle = sAPI_fopen(path, mode);
    ret.errcode = testFileHandle == NULL ? -1 : 0;
    ret.msg = "open file";

    return ret;
}

ret_msg_t fs_fclose_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };

    ret.errcode = sAPI_fclose(testFileHandle);
    ret.msg = "close file";
    testFileHandle = NULL;
    return ret;
}

ret_msg_t fs_fread_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    arg_t out_arg[1] = {0};
    int read_size, real_size;
    char *buffer = NULL;
    pl_demo_get_value(&read_size, argv, argn, ARG_FS_DATASIZE);
    buffer = sal_malloc(read_size + 1);
    memset(buffer, 0, read_size + 1);
    out_arg[0].arg_name = pre_arg_name[ARG_ID_DATA];
    out_arg[0].type = TYPE_RAW;
    out_arg[0].value = pl_demo_make_value(0, NULL, read_size);
    pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
    real_size = sAPI_fread(buffer, 1, read_size, testFileHandle);
    if(real_size > 0) pl_demo_post_raw(buffer, real_size);
    pl_demo_release_value(out_arg[0].value);
    TEST_LOG_INFO("read data[%s]", buffer);
    sal_free(buffer);

    if(read_size >= 0)
        ret.errcode = 0;
    else
        ret.errcode = read_size;
    ret.msg = "read file";

    return ret;
}

int g_file_size;
sTaskRef g_write_task_ref;
pub_cache_p g_write_cache;
ret_msg_t g_write_result;

int write_file_need_size(void)
{
    return g_file_size;
}

void write_file_task(void *args);

ret_msg_t write_file_uninit(void)
{
    ret_msg_t ret = {0};

    pub_bzero(&g_write_result, sizeof(ret_msg_t));
    g_file_size = 0;
    pub_cache_set_status(g_write_cache, CACHE_STOPED);

    sAPI_TaskSleep(5);  // wait task exit.

    if (g_write_task_ref)
    {
        ret.errcode = sAPI_TaskDelete(g_write_task_ref);
        if (ret.errcode != 0)
        {
            TEST_LOG_ERROR("task delete fail, errcode=%d", ret.errcode);
            goto DONE;
        }
        else
        {
            g_write_task_ref = NULL;
        }
    }

    if (g_write_cache)
    {
        ret.errcode = pub_cache_delete(&g_write_cache);
        if (ret.errcode != 0)
        {
            TEST_LOG_ERROR("cache delete fail");
            goto DONE;
        }
    }

DONE:
    return ret;
}

ret_msg_t write_file_init(void)
{
    ret_msg_t ret = {0};

    ret = write_file_uninit();
    if (0 != ret.errcode)
    {
        goto DONE;
    }

    g_write_cache = pub_cache_create(SAL_20K, CACHE_TYPE_BUFFER, CACHE_SYNC_R);
    if (NULL == g_write_cache)
    {
        ret.errcode = ERRCODE_DEFAULT;
        TEST_LOG_ERROR("cache create fail");
        goto DONE;
    }

DONE:
    return ret;
}

ret_msg_t fs_fwrite_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int fileSize = 0;

    pl_demo_get_value(&fileSize, argv, argn, ARG_FS_DATASIZE);
    g_file_size = fileSize;

    ret.errcode = sAPI_TaskCreate(&g_write_task_ref,
                                  NULL,
                                  SAL_2K,
                                  150,
                                  "SAVE FILE",
                                  write_file_task,
                                  NULL);
        
    return ret;
}

ret_msg_t write_file_raw_proc(int size, int (*raw_read)(char *buf, unsigned int buf_size, int timeout_ms))
{
    ret_msg_t ret = {0};
    int read_size = 0;
    int process_size = 0;
    int remain_size = size;
    char buf[SAL_256] = {0};

    while (remain_size > 0 && testFileHandle)
    {
        read_size = remain_size <= SAL_256 ? remain_size : SAL_256;
        read_size = raw_read(buf, read_size, 10000);
        if (read_size < 0)
        {
            TEST_LOG_ERROR("raw data read fail");
            ret.errcode = read_size < 0 ? read_size : ERRCODE_DEFAULT;
            ret.msg = "raw data read fail";
            goto EXIT;
        }
        else if (0 == read_size)
        {
            TEST_LOG_ERROR("raw data read timeout");
            ret.errcode = ERRCODE_RAW_RECV_TIMEOUT;
            ret.msg = "raw data read timeout";
            goto EXIT;
        }

        process_size = pub_cache_write(g_write_cache, buf, read_size);
        if (process_size < read_size)
        {
            TEST_LOG_ERROR("raw data cache fail");
            TEST_LOG_ERROR("read size:%d, cache size:%d", read_size, process_size);
            ret.errcode = process_size < 0 ? process_size : ERRCODE_DEFAULT;
            ret.msg = "raw data cache fail";
            goto EXIT;
        }

        remain_size -= read_size;
    }

EXIT:
    /* wait file write to fs. */
    pub_cache_set_status(g_write_cache, CACHE_STOPED);

    if (ret.errcode != 0) return ret;
    else return g_write_result;
}

void write_file_task(void *args)
{
    ret_msg_t ret = {0};
    char *buf = NULL;
    int read_size = 0, process_size = 0;

    buf = sal_malloc(SAL_4K);
    if (!buf)
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "malloc fail";
        goto EXIT;
    }

    while (g_file_size > 0 && testFileHandle && g_write_cache)
    {
        read_size = g_file_size <= SAL_4K ? g_file_size : SAL_4K;

        read_size = pub_cache_read(g_write_cache, buf, read_size);
        if (read_size <= 0)
        {
            ret.errcode = read_size < 0 ? read_size : ERRCODE_DEFAULT;
            ret.msg = "cache read fail";
            goto EXIT;
        }

        process_size = sAPI_fwrite(buf, 1, read_size, testFileHandle);
        if (process_size != read_size)
        {
            TEST_LOG_ERROR("write file fail");
            TEST_LOG_ERROR("writed size:%d, need write size:%d", process_size, read_size);
            ret.errcode = ERRCODE_DEFAULT;
            ret.msg = "write file fail";
            goto EXIT;
        }

        g_file_size -= process_size;
    }

EXIT:
    sal_free(buf);
    g_write_result = ret;
}

ret_msg_t fs_fsync_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };

    ret.errcode = sAPI_fsync(testFileHandle);
    ret.msg = "file sync";

    return ret;
}

ret_msg_t fs_fseek_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    int offset, position;

    pl_demo_get_value(&offset, argv, argn, ARG_FS_OFFSET);
    pl_demo_get_value(&position, argv, argn, ARG_FS_POSITION);

    TEST_LOG_ERROR("offset[%d], position[%d]",offset, position);
    ret.errcode = sAPI_fseek(testFileHandle, offset, position);
    ret.msg = "file seek";

    return ret;
}

ret_msg_t fs_ftell_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };

    ret.errcode = sAPI_ftell(testFileHandle);
    ret.msg = "file ftell";

    return ret;
}

ret_msg_t fs_fsize_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };

    ret.errcode = sAPI_fsize(testFileHandle);
    ret.msg = "file fsize";

    return ret;
}

ret_msg_t fs_fstat_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    const char *path = NULL;
    struct dirent_t info = {0};
    arg_t out_arg[2] = {0};

    pl_demo_get_data(&path, argv, argn, ARG_FS_PATH);
    ret.errcode = sAPI_stat(path, &info);

    if(ret.errcode == 0)
    {
        out_arg[0].type = TYPE_INT;
        out_arg[0].arg_name = "file_type";
        out_arg[0].value = pl_demo_make_value(info.type, NULL, 0);
        out_arg[1].type = TYPE_STR;
        out_arg[1].arg_name = "file_name";
        out_arg[1].value = pl_demo_make_value(0, info.name, strlen(info.name));
        pl_demo_post_data(OP_POST, out_arg, 2, NULL, 0);
        pl_demo_release_value(out_arg[0].value);
        pl_demo_release_value(out_arg[1].value);
    }
    ret.msg = "file stat";

    return ret;
}

ret_msg_t fs_rename_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    const char *oldName = NULL;
    const char *newName = NULL;

    pl_demo_get_data(&oldName, argv, argn, ARG_FS_OLDNAME);
    pl_demo_get_data(&newName, argv, argn, ARG_FS_NEWNAME);
    TEST_LOG_INFO("old[%s], new[%s]", oldName, newName);
    ret.errcode = sAPI_rename(oldName, newName);
    ret.msg = "file rename";
    return ret;
}
ret_msg_t fs_remove_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    const char *fileName = NULL;

    pl_demo_get_data(&fileName, argv, argn, ARG_FS_PATH);
    ret.errcode = sAPI_remove(fileName);
    ret.msg = "file remove";
    return ret;
}

ret_msg_t fs_mkdir_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    const char *path = NULL;

    pl_demo_get_data(&path, argv, argn, ARG_FS_PATH);
    ret.errcode = sAPI_mkdir(path, 0);
    ret.msg = "create dir";
    return ret;
}

ret_msg_t fs_opendir_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    const char *path = NULL;

    pl_demo_get_data(&path, argv, argn, ARG_FS_PATH);
    testDirHandle = sAPI_opendir(path);
    ret.errcode = testDirHandle == NULL ? -1 : 0;
    ret.msg = "open dir";
    return ret;
}

ret_msg_t fs_closedir_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    ret.errcode = sAPI_closedir(testDirHandle);
    ret.msg = "close dir";
    return ret;
}

ret_msg_t fs_readdir_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    struct dirent_t *info_dir = NULL;
    arg_t out_arg[3] = {0};

    while ((info_dir = sAPI_readdir(testDirHandle)) != NULL)
    {
        TEST_LOG_INFO("name[%s]-size[%ld]-type[%d]\r\n", info_dir->name, info_dir->size, info_dir->type);
        out_arg[0].type = TYPE_STR;
        out_arg[0].arg_name = "file_name";
        out_arg[0].value = pl_demo_make_value(0, info_dir->name, strlen(info_dir->name));
        out_arg[1].type = TYPE_INT;
        out_arg[1].arg_name = "file_size";
        out_arg[1].value = pl_demo_make_value(info_dir->size, 0, 0);
        out_arg[2].type = TYPE_INT;
        out_arg[2].arg_name = "type";
        out_arg[2].value = pl_demo_make_value(info_dir->type, 0, 0);
        pl_demo_post_data(OP_POST, out_arg, 3, NULL, 0);
        pl_demo_release_value(out_arg[0].value);
        pl_demo_release_value(out_arg[1].value);
        pl_demo_release_value(out_arg[2].value);
    }
    ret.errcode = 0;
    ret.msg = "read dir";
    return ret;
}

ret_msg_t fs_get_size_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *disk = NULL;
    int total_size = 0;
    arg_t out_arg[1] = {0};

    pl_demo_get_data(&disk, argv, argn, ARG_FS_DISK);
    total_size = sAPI_GetSize((char *)disk);

    out_arg[0].type = TYPE_INT;
    out_arg[0].arg_name = "disk_total_size";
    out_arg[0].value = pl_demo_make_value(total_size, NULL, 0);
    pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
    pl_demo_release_value(out_arg[0].value);

    if(total_size > 0)
        ret.errcode = 0;
    else
        ret.errcode = -1;

    return ret;
}

ret_msg_t fs_get_free_size_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *disk = NULL;
    int free_size = 0;
    arg_t out_arg[1] = {0};

    pl_demo_get_data(&disk, argv, argn, ARG_FS_DISK);
    free_size = sAPI_GetFreeSize((char *)disk);

    out_arg[0].type = TYPE_INT;
    out_arg[0].arg_name = "disk_free_size";
    out_arg[0].value = pl_demo_make_value(free_size, NULL, 0);
    pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
    pl_demo_release_value(out_arg[0].value);

    if(free_size > 0)
        ret.errcode = 0;
    else
        ret.errcode = -1;

    return ret;
}

ret_msg_t fs_get_used_size_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *disk = NULL;
    int used_size = 0;
    arg_t out_arg[1] = {0};

    pl_demo_get_data(&disk, argv, argn, ARG_FS_DISK);
    used_size = sAPI_GetUsedSize((char *)disk);

    out_arg[0].type = TYPE_INT;
    out_arg[0].arg_name = "disk_used_size";
    out_arg[0].value = pl_demo_make_value(used_size, NULL, 0);
    pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
    pl_demo_release_value(out_arg[0].value);

    if(used_size > 0)
        ret.errcode = 0;
    else
        ret.errcode = -1;

    return ret;
}

ret_msg_t fs_switch_dir_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *pathName = NULL;
    const char *mode_str = NULL;
    int dirction = 0;

    pl_demo_get_data(&pathName, argv, argn, ARG_FS_PATH);
    pl_demo_get_data(&mode_str, argv, argn, ARG_FS_SWITCHMODE);
    if (!pathName)
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }
    if (0 == strcmp(mode_str, ARG_FS_SWITCH_TO_C))/*c:/1.txt->c:/simdir/1.txt*/
    {
        dirction = 0;
    }
    else/*c:/simdir/1.txt->c:/1.txt*/
    {
        dirction = 1;
    }

    ret.errcode = sAPI_FsSwitchDir((char *)pathName, dirction);
    if (ret.errcode)
    {
        ret.msg = "SwitchDir err";
        ret.errcode = ERRCODE_DEFAULT;
    }
    else
    {
        ret.msg = "SwitchDir succ";
    }

    return ret;
}

#if defined (EXT_FLASH_LFSV2_ENABLE) || defined (EXT_FLASH_LFSV1_ENABLE) || defined (FEATURE_SIMCOM_SD_CARD) || defined (EXT_FLASH_YAFFS2_ENABLE)
ret_msg_t fs_format_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *disk = NULL;

    pl_demo_get_data(&disk, argv, argn, ARG_FS_DISK);
    if (!disk)
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }

    ret.errcode = sAPI_fformat((char *)disk);

    return ret;
}
#endif
#endif

ret_msg_t tcpip_init_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int pdp_id = 0;

    pl_demo_get_value(&pdp_id, argv, argn, ARG_TCPIP_PDPID);
    ret.errcode = sAPI_TcpipPdpActive(pdp_id, 1);
    ret.msg = "tcpip init";
    return ret;
}

ret_msg_t tcpip_socket_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *family_str = NULL;
    const char *socktype_str = NULL;
    int family = AF_UNSPEC;
    int socktype = SOCK_STREAM;

    pl_demo_get_data(&family_str, argv, argn, ARG_TCPIP_FAMILY);
    pl_demo_get_data(&socktype_str, argv, argn, ARG_TCPIP_SOCKTYPE);

    if (0 == strcmp(family_str, ARG_TCPIP_IPV4))
    {
        family = AF_INET;
    }
    else if (0 == strcmp(family_str, ARG_TCPIP_IPV6))
    {
        family = AF_INET6;
    }
    if (0 == strcmp(socktype_str, "TCP"))
    {
        socktype = SOCK_STREAM;
    }
    else if (0 == strcmp(socktype_str, "UDP"))
    {
        socktype = SOCK_DGRAM;
    }
    memset(&tcp_hints, 0x0, sizeof(tcp_hints));

    ret.errcode = socket(family, socktype, 0);

    tcp_hints.ai_family = family;
    tcp_hints.ai_socktype = socktype;
    if(socktype == SOCK_STREAM)
        tcp_hints.ai_protocol = IPPROTO_TCP;
    else if(socktype == SOCK_DGRAM)
        tcp_hints.ai_protocol = IPPROTO_UDP;

    ret.msg = "tcpip socket";
    return ret;
}

ret_msg_t tcpip_bind_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int fd = -1;
    int port = 0;
    struct sockaddr_in sraddr = {0};

    pl_demo_get_value(&fd, argv, argn, ARG_TCPIP_SOCKETFD);
    pl_demo_get_value(&port, argv, argn, ARG_TCPIP_PORT);

    sraddr.sin_family = tcp_hints.ai_family;
    sraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    sraddr.sin_port = htons(port);
    ret.errcode = bind(fd, (struct sockaddr *)&sraddr, sizeof(sraddr));
    ret.msg = "tcpip bind";
    return ret;
}

ret_msg_t tcpip_listen_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int fd = -1;

    pl_demo_get_value(&fd, argv, argn, ARG_TCPIP_SOCKETFD);

    ret.errcode = listen(fd, 5);
    ret.msg = "tcpip listen";

    return ret;
}

ret_msg_t tcpip_accept_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int fd = -1;
    struct sockaddr claddr;
    socklen_t len = sizeof(struct sockaddr);

    pl_demo_get_value(&fd, argv, argn, ARG_TCPIP_SOCKETFD);

    memset(&claddr,0,sizeof(claddr));
    len = sizeof(claddr);

    ret.errcode = accept(fd, &claddr, &len);
    ret.msg = "tcpip accept";

    return ret;
}

ret_msg_t tcpip_select_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int fd = -1;
    fd_set read_fd;
    struct timeval tv;

    pl_demo_get_value(&fd, argv, argn, ARG_TCPIP_SOCKETFD);

    FD_ZERO(&read_fd);
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    ret.errcode = select(fd + 1, &read_fd, NULL, NULL, &tv);
    ret.msg = "tcpip select";

    return ret;
}

ret_msg_t tcpip_recv_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    arg_t out_arg[1] = {0};
    int fd = -1;
    int readsize = 0;
    struct sockaddr addr;
    socklen_t addr_len;
    char *buffer = NULL;

    pl_demo_get_value(&fd, argv, argn, ARG_TCPIP_SOCKETFD);
    pl_demo_get_value(&readsize, argv, argn, ARG_TCPIP_SENDSIZE);

    buffer = sal_malloc(readsize + 1);
    memset(buffer, 0, readsize + 1);
    memset(&addr, 0, sizeof(addr));
    addr_len = sizeof(struct sockaddr);

    out_arg[0].arg_name = pre_arg_name[ARG_ID_DATA];
    out_arg[0].type = TYPE_RAW;
    out_arg[0].value = pl_demo_make_value(0, NULL, readsize);
    pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
    if(tcp_hints.ai_protocol == IPPROTO_TCP)
        ret.errcode = recv(fd, buffer, readsize, 0);
    else if(tcp_hints.ai_protocol == IPPROTO_UDP)
        ret.errcode = recvfrom(fd, buffer, readsize, 0, &addr, &addr_len);

    pl_demo_post_raw(buffer, readsize);
    pl_demo_release_value(out_arg[0].value);
    sal_free(buffer);

    ret.msg = "tcpip recv";

    return ret;
}

ret_msg_t tcpip_connect_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int fd = -1;
    const char *serverIP = NULL;
    int port = 0;
    char portstr[6] = {0};
    struct addrinfo *addr_list = NULL, *rp = NULL;

    pl_demo_get_value(&fd, argv, argn, ARG_TCPIP_SOCKETFD);
    pl_demo_get_data(&serverIP, argv, argn, ARG_TCPIP_SERVERIP);
    pl_demo_get_value(&port, argv, argn, ARG_TCPIP_PORT);

    snprintf(portstr, sizeof(portstr), "%d", port);
    if (getaddrinfo(serverIP, portstr, &tcp_hints, &addr_list) != 0)
    {
        TEST_LOG_ERROR("getaddrinfo error");
        ret.errcode = -1;
    }
    else
    {
        rp = addr_list;
        tcp_hints.ai_addr = rp->ai_addr;
        ret.errcode = connect(fd, rp->ai_addr, rp->ai_addrlen);
    }
    ret.msg = "tcpip connect";
    freeaddrinfo(addr_list);
    return ret;
}

int tcpip_send_need_size(void)
{
    return tcp_sendsize;
}

ret_msg_t tcpip_send_uninit(void)
{
    ret_msg_t ret = {0};

    pub_bzero(&g_tcpsend_result, sizeof(ret_msg_t));
    tcp_sendsize = 0;
    pub_cache_set_status(g_tcpsend_cache, CACHE_STOPED);

    sAPI_TaskSleep(5);  // wait task exit.

    if (g_tcpsend_task_ref)
    {
        ret.errcode = sAPI_TaskDelete(g_tcpsend_task_ref);
        if (ret.errcode != 0)
        {
            TEST_LOG_ERROR("task delete fail, errcode=%d", ret.errcode);
            goto DONE;
        }
        else
        {
            g_tcpsend_task_ref = NULL;
        }
    }

    if(g_tcpsend_flag)
    {
        g_tcpsend_flag = 0;
    }

    if (g_tcpsend_cache)
    {
        ret.errcode = pub_cache_delete(&g_tcpsend_cache);
        if (ret.errcode != 0)
        {
            TEST_LOG_ERROR("cache delete fail");
            goto DONE;
        }
    }

DONE:
    return ret;
}

ret_msg_t tcpip_send_init(void)
{
    ret_msg_t ret = {0};

    ret = tcpip_send_uninit();
    if (0 != ret.errcode)
    {
        goto DONE;
    }

    g_tcpsend_cache = pub_cache_create(SAL_20K, CACHE_TYPE_BUFFER, CACHE_SYNC_R);
    if (NULL == g_tcpsend_cache)
    {
        ret.errcode = ERRCODE_DEFAULT;
        TEST_LOG_ERROR("cache create fail");
        goto DONE;
    }

DONE:
    return ret;
}

ret_msg_t tcpip_send_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int fd = -1;
    int sendsize = 0;

    pl_demo_get_value(&fd, argv, argn, ARG_TCPIP_SOCKETFD);
    pl_demo_get_value(&sendsize, argv, argn, ARG_TCPIP_SENDSIZE);

    tcp_sockfd = fd;
    tcp_sendsize = sendsize;
    ret.errcode = sAPI_TaskCreate(&g_tcpsend_task_ref,
                                      NULL,
                                      SAL_2K,
                                      150,
                                      "TCPIP SEND",
                                      tcpip_send_task,
                                      NULL);

    ret.msg = "tcpip send";
    return ret;
}

ret_msg_t tcpip_send_raw_proc(int size, int (*raw_read)(char *buf, unsigned int buf_size, int timeout_ms))
{
    ret_msg_t ret = {0};
    int read_size = 0;
    int process_size = 0;
    int remain_size = size;
    char buf[SAL_256] = {0};

    if (tcp_sendsize != size)
    {
        TEST_LOG_ERROR("raw data size invalid");
        TEST_LOG_ERROR("size:%d, need:%d", size, tcp_sendsize);
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "raw data size invalid";
        return ret;
    }

    while (remain_size > 0)
    {
        TEST_LOG_INFO("remain_size[%d]",remain_size);
        read_size = remain_size <= SAL_256 ? remain_size : SAL_256;
        read_size = raw_read(buf, read_size, 10000);
        TEST_LOG_INFO("readsize[%d]",read_size);
        if (read_size < 0)
        {
            TEST_LOG_ERROR("raw data read fail");
            ret.errcode = read_size < 0 ? read_size : ERRCODE_DEFAULT;
            ret.msg = "raw data read fail";
            goto EXIT;
        }
        else if (0 == read_size)
        {
            TEST_LOG_ERROR("raw data read timeout");
            ret.errcode = ERRCODE_RAW_RECV_TIMEOUT;
            ret.msg = "raw data read timeout";
            goto EXIT;
        }

        process_size = pub_cache_write(g_tcpsend_cache, buf, read_size);
        if (process_size < read_size)
        {
            TEST_LOG_ERROR("raw data cache fail");
            TEST_LOG_ERROR("read size:%d, cache size:%d", read_size, process_size);
            ret.errcode = process_size < 0 ? process_size : ERRCODE_DEFAULT;
            ret.msg = "raw data cache fail";
            goto EXIT;
        }

        remain_size -= read_size;
    }

EXIT:
    /* wait send data to server. */
    pub_cache_set_status(g_tcpsend_cache, CACHE_STOPED);
    while (g_tcpsend_flag != 1) sAPI_TaskSleep(20);

    if (ret.errcode != 0) return ret;
    else return g_tcpsend_result;
}

void tcpip_send_task(void *args)
{
    ret_msg_t ret = {0};
    char *buf = NULL;
    int read_size = 0, process_size = 0;

    buf = sal_malloc(SAL_1K);
    if (!buf)
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "malloc fail";
        goto EXIT;
    }

    while (tcp_sendsize > 0 && g_tcpsend_cache)
    {
        read_size = tcp_sendsize <= SAL_1K ? tcp_sendsize : SAL_1K;

        read_size = pub_cache_read(g_tcpsend_cache, buf, read_size);
        if (read_size <= 0)
        {
            TEST_LOG_ERROR("cache read fail");
            ret.errcode = read_size < 0 ? read_size : ERRCODE_DEFAULT;
            ret.msg = "cache read fail";
            goto EXIT;
        }
        if(tcp_hints.ai_protocol == IPPROTO_TCP)
            process_size = send(tcp_sockfd, buf, read_size, 0);
        else if(tcp_hints.ai_protocol == IPPROTO_UDP)
            process_size = sendto(tcp_sockfd, buf, read_size, 0, tcp_hints.ai_addr, sizeof(struct sockaddr));

        if (process_size == read_size)
        {
            TEST_LOG_INFO("send sucess size[%d]",process_size);
            tcp_sendsize -= process_size;
        }
        else
        {
            TEST_LOG_ERROR("send fail socket_errno");
            ret.errcode = ERRCODE_DEFAULT;
            ret.msg ="send fail";
            goto EXIT;
        }
    }

EXIT:
    sal_free(buf);
    g_tcpsend_flag = 1;
    g_tcpsend_result = ret;
}

ret_msg_t tcpip_setsockopt_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int fd = -1;
    struct linger s_linger;

    pl_demo_get_value(&fd, argv, argn, ARG_TCPIP_SOCKETFD);

    s_linger.l_onoff=1;
    s_linger.l_linger=1;

    ret.errcode = setsockopt(fd,SOL_SOCKET,SO_LINGER,&s_linger,sizeof(s_linger));
    ret.msg = "tcpip setsockopt";

    return ret;
}

ret_msg_t tcpip_getsockopt_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int fd = -1;
    int value = 0;
    int len = 0;

    pl_demo_get_value(&fd, argv, argn, ARG_TCPIP_SOCKETFD);

    len = sizeof(value);

    ret.errcode = getsockopt(fd, SOL_SOCKET, SO_ERROR, &value, (socklen_t *)&len);
    ret.msg = "tcpip getsockopt";

    return ret;
}

ret_msg_t tcpip_close_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int fd = -1;

    pl_demo_get_value(&fd, argv, argn, ARG_TCPIP_SOCKETFD);

    ret.errcode = close(fd);
    ret.msg = "tcpip close";

    return ret;
}

ret_msg_t tcpip_deinit_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int pdp_id = 0;

    pl_demo_get_value(&pdp_id, argv, argn, ARG_TCPIP_PDPID);
    ret.errcode = sAPI_TcpipPdpDeactive(pdp_id, 1);
    ret.msg = "tcpip deinit";
    return ret;
}

ret_msg_t httpsinit_cmd(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    SC_HTTP_RETURNCODE api_ret = SC_HTTPS_SUCCESS;
    if (NULL == httpsUIResp_msgq)
    {
        sAPI_MsgQCreate(&httpsUIResp_msgq, "httpsUIResp_msgq", sizeof(SIM_MSG_T), 8, SC_FIFO);
    }

    api_ret = sAPI_HttpInit(1, httpsUIResp_msgq);
    ret.errcode = api_ret;
    ret.msg = "https init";
    return ret;
}

ret_msg_t httpspara_cmd(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret={0};
    SC_HTTP_RETURNCODE api_ret = SC_HTTPS_SUCCESS;
    const char * paraType=NULL;
    const char * paraValue=NULL;
    pl_demo_get_data(&paraType, argv, argn, ARG_HTTPS_PARATYPE);
    pl_demo_get_data(&paraValue, argv, argn, ARG_HTTPS_PARAVALUE);

    char *ptr1 = sal_malloc(strlen(paraType)+1);
    char *ptr2 = sal_malloc(strlen(paraValue)+1);
    strncpy(ptr1 , paraType, strlen(paraType));
    strncpy(ptr2, paraValue, strlen(paraValue));
    api_ret = sAPI_HttpPara(ptr1, ptr2);
    ret.errcode = api_ret;
    ret.msg = "https para";
    sal_free(ptr1);
    sal_free(ptr2);
    return ret;
}

int g_httpsdata_size;
sTaskRef g_httpsdata_task_ref;
pub_cache_p g_httpsdata_cache;
ret_msg_t g_httpsdata_result;
char *data_cache_buf=NULL;

int httpsdata_need_size(void)
{
    return g_httpsdata_size;
}
void httpsdata_task(void *args);

ret_msg_t httpsdata_uninit(void)
{
    ret_msg_t ret = {0};
    pub_bzero(&g_httpsdata_result, sizeof(ret_msg_t));
    g_httpsdata_size = 0;
    pub_cache_set_status(g_httpsdata_cache, CACHE_STOPED);

    sAPI_TaskSleep(5);  // wait task exit.

    if (g_httpsdata_task_ref)
    {
        ret.errcode = sAPI_TaskDelete(g_httpsdata_task_ref);
        if (ret.errcode != 0)
        {
            TEST_LOG_ERROR("task delete fail, errcode=%d", ret.errcode);
            goto DONE;
        }
        else
        {
            g_httpsdata_task_ref = NULL;
        }
    }
    if (g_httpsdata_cache)
    {
        ret.errcode = pub_cache_delete(&g_httpsdata_cache);
        if (ret.errcode != 0)
        {
            TEST_LOG_ERROR("cache delete fail");
            goto DONE;
        }
    }

DONE:
    return ret;

}

ret_msg_t httpsdata_init(void)
{
    ret_msg_t ret = { 0 };
    ret = httpsdata_uninit();
    if (0 != ret.errcode)
    {
        goto DONE;
    }
    g_httpsdata_cache = pub_cache_create(SAL_20K, CACHE_TYPE_BUFFER, CACHE_SYNC_R);
    if (NULL == g_httpsdata_cache)
    {
        ret.errcode = ERRCODE_DEFAULT;
        TEST_LOG_ERROR("cache create fail");
        goto DONE;
    }

DONE:
    return ret;
}

ret_msg_t httpsdata_cmd(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret={0};
    int dataLen  = 0;

    pl_demo_get_value(&dataLen,argv,argn,ARG_HTTPS_DATALEN);

    data_cache_buf = sal_malloc(dataLen);
    g_httpsdata_size = dataLen;
    if( dataLen > 0)
    {
        ret.errcode = sAPI_TaskCreate(&g_httpsdata_task_ref,
                                      NULL,
                                      SAL_2K,
                                      150,
                                      "HTTPSDATA",
                                      httpsdata_task,
                                      NULL);
        if (ret.errcode != SC_SUCCESS)
        {
            ret.msg = "task creat fail";
        }
    }
    return ret;
}

ret_msg_t httpsdata_raw(int size , int (*raw_read)(char *buf , unsigned int buf_size, int timeout_ms))
{
    ret_msg_t ret = {0};
    int read_size = 0;
    int process_size = 0;
    int remain_size = size;
    char buf[SAL_256] = {0};

    if(g_httpsdata_size != size)
    {
        TEST_LOG_ERROR("raw data size invalid");
        TEST_LOG_ERROR("size:%d, need:%d", size, g_httpsdata_size);
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "raw data size invalid";
        return ret;
    }

    while (remain_size > 0 && data_cache_buf)
    {
        read_size = remain_size <= SAL_256 ? remain_size : SAL_256;
        read_size = raw_read(buf, read_size, 10000);
        if (read_size < 0)
        {
            TEST_LOG_ERROR("raw data read fail");
            ret.errcode = read_size < 0 ? read_size : ERRCODE_DEFAULT;
            ret.msg = "raw data read fail";
            goto EXIT;
        }
        else if (0 == read_size)
        {
            TEST_LOG_ERROR("raw data read timeout");
            ret.errcode = ERRCODE_RAW_RECV_TIMEOUT;
            ret.msg = "raw data read timeout";
            goto EXIT;
        }

        process_size = pub_cache_write(g_httpsdata_cache, buf, read_size);
        if (process_size < read_size)
        {
            TEST_LOG_ERROR("raw data cache fail");
            TEST_LOG_ERROR("read size:%d, cache size:%d", read_size, process_size);
            ret.errcode = process_size < 0 ? process_size : ERRCODE_DEFAULT;
            ret.msg = "raw data cache fail";
            goto EXIT;
        }

        remain_size -= read_size;
    }
EXIT:
    /* wait file write to fs. */
    pub_cache_set_status(g_httpsdata_cache, CACHE_STOPED);
    while (NULL != data_cache_buf) sAPI_TaskSleep(20);

    if (ret.errcode != 0) return ret;
    else return g_httpsdata_result;
}

void httpsdata_task(void *args)
{
     SC_HTTP_RETURNCODE api_ret = SC_HTTPS_SUCCESS;
     ret_msg_t ret = {0};
    char *buf = NULL;
    int read_size = 0 ;
    int cache_size = 0;

    buf = sal_malloc(SAL_4K);
    while ((g_httpsdata_size > cache_size)  && data_cache_buf && g_httpsdata_cache)
    {
        read_size = g_httpsdata_size <= SAL_4K ? g_httpsdata_size : SAL_4K;

        read_size = pub_cache_read(g_httpsdata_cache, buf, read_size);
        if (read_size <= 0)
        {
            ret.errcode = read_size < 0 ? read_size : ERRCODE_DEFAULT;
            ret.msg = "cache read fail";
            goto EXIT;
        }

        strncpy(data_cache_buf+cache_size , buf , read_size);

        cache_size += read_size;
    }

    api_ret = sAPI_HttpData(data_cache_buf,g_httpsdata_size);
    if(api_ret != SC_HTTPS_SUCCESS)
    {
        ret.errcode = -1;
        ret.msg = "set post data error";
    }
    else
    {
        ret.msg = "set post data success";
    }
EXIT:
    sal_free(buf);
    sal_free(data_cache_buf);
    data_cache_buf = NULL;
    g_httpsdata_result = ret;

}

void show_httpaction_urc(ret_msg_t *ret)
{
    char buf[100]={0};
    SIM_MSG_T msgQ_data_recv = {SIM_MSG_INIT, 0, -1, NULL};
    sAPI_MsgQRecv(httpsUIResp_msgq, &msgQ_data_recv, SC_SUSPEND);
    arg_t out_arg[1] = {0};

    SChttpApiTrans *sub_data = (SChttpApiTrans *)(msgQ_data_recv.arg3);

    int bufsize=snprintf(buf,sizeof(buf)-1,"status-code=%ld content-length=%ld",sub_data->status_code,sub_data->action_content_len);
    out_arg[0].arg_name = pre_arg_name[ARG_ID_DATA];
    out_arg[0].type = TYPE_RAW;
    out_arg[0].value = pl_demo_make_value(0, buf, bufsize);
    ret->errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
    pl_demo_post_raw(buf, bufsize);
    TEST_LOG_INFO("response info [%s]",buf);
    pl_demo_release_value(out_arg[0].value);
    if(sub_data)
    {
        sAPI_Free(sub_data->data);
    }
    sAPI_Free(sub_data);
}
ret_msg_t httpsaction_cmd(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret={0};
    int method = -1;
    SC_HTTP_RETURNCODE api_ret = SC_HTTPS_SUCCESS;
    const char *str_action_method=NULL;
    pl_demo_get_data(&str_action_method, argv, argn, ARG_HTTPS_METHOD);
    method = str_action_method[0]-'0';
    api_ret = sAPI_HttpAction(method);
    if(api_ret == SC_HTTPS_SUCCESS )
    {
        ret.errcode=0;
        show_httpaction_urc(&ret);
        TEST_LOG_INFO("https action success");
        ret.msg= "https action success";
    }
    else
    {
        ret.errcode = -1;
        ret.msg="https action fail";
        TEST_LOG_INFO("https action fail");
    }
    return ret;
}

void show_httphead_urc( ret_msg_t *ret,int type)
{
    SIM_MSG_T msgQ_data_recv = {SIM_MSG_INIT, 0, -1, NULL};
    SChttpApiTrans *sub_data = NULL;
    arg_t out_arg[1] = {0};

    sAPI_MsgQRecv(httpsUIResp_msgq, &msgQ_data_recv, SC_SUSPEND);

    sub_data = (SChttpApiTrans *)(msgQ_data_recv.arg3);

    if (1 == type)
    {
        if (sub_data->data != NULL && sub_data->dataLen > 0)
        {

            out_arg[0].arg_name = pre_arg_name[ARG_ID_DATA];
            out_arg[0].type = TYPE_RAW;
            out_arg[0].value = pl_demo_make_value(0, (char *)sub_data->data, sub_data->dataLen);
            ret->errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
            TEST_LOG_INFO("\rsub_data->dataLen[%d]\r\n", sub_data->dataLen);
            pl_demo_post_raw((char *)sub_data->data, sub_data->dataLen);
            pl_demo_release_value(out_arg[0].value);
        }
    }
    else if (0 == type)
    {
        char rspBuf[50] = { 0 };
        snprintf(rspBuf, sizeof(rspBuf) - 1, "left data length=%ld\r\n", sub_data->action_content_len);
        out_arg[0].arg_name = pre_arg_name[ARG_ID_DATA];
        out_arg[0].type = TYPE_RAW;
        out_arg[0].value = pl_demo_make_value(0, rspBuf, strlen(rspBuf));
        ret->errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
        pl_demo_post_raw(rspBuf, strlen(rspBuf));
        pl_demo_release_value(out_arg[0].value);
    }

    if(sub_data)
    {
        sAPI_Free(sub_data->data);
    }
    sAPI_Free(sub_data);
}
ret_msg_t httpshead_cmd(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret={0};
    SC_HTTP_RETURNCODE api_ret = SC_HTTPS_SUCCESS;
    api_ret = sAPI_HttpHead();
    if(api_ret == SC_HTTPS_SUCCESS)
    {
        show_httphead_urc(&ret,1);
        TEST_LOG_INFO("https read head success");
        ret.msg = "https read head success";
    }
    else
    {
        TEST_LOG_INFO("https read head fail");
        ret.errcode = -1;
        ret.msg = "https read head fail";
    }
    return ret;
}

ret_msg_t httpsread_cmd(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret={0};
    SC_HTTP_RETURNCODE api_ret = SC_HTTPS_SUCCESS;
    int type = -1;
    int offset = -1;
    int size = -1;
    const char *type_str=NULL;
    pl_demo_get_data(&type_str,argv,argn,ARG_HTTPS_READTYPE);
    pl_demo_get_value(&offset,argv,argn,ARG_HTTPS_READOFFSET);
    pl_demo_get_value(&size,argv,argn,ARG_HTTPS_READSIZE);
    type = type_str[0]-'0';
    api_ret = sAPI_HttpRead(type,offset,size);
    if(api_ret == SC_HTTPS_SUCCESS)
    {
        show_httphead_urc(&ret,type);
        TEST_LOG_INFO("https read body success");
        ret.msg ="https read body success";
    }
    else
    {
        TEST_LOG_INFO("https read body fail");
        ret.errcode = -1;
        ret.msg = "https read body fail";
    }

    return ret;
}

ret_msg_t httpsterm_cmd(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    SC_HTTP_RETURNCODE api_ret = SC_HTTPS_SUCCESS;
    api_ret = sAPI_HttpTerm();
    ret.errcode = api_ret;
    ret.msg = "https term";

    return ret;
}

#ifdef FEATURE_SIMCOM_HTP
ret_msg_t htp_config_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    arg_t out_arg[1] = { 0 };
    SChtpReturnCode result = SC_HTP_ERROR;
    char *host_index = NULL;
    char *cmd = NULL;
    char *proxy = NULL;
    char *config_buf = NULL;
    int command = 0, port = -1, version = -1;
    int proxy_port = 0;

    pl_demo_get_value(&command, argv, argn, ARG_HTP_COMMAND);
    pl_demo_get_data((const char**)&cmd, argv, argn, ARG_HTP_CMD);
    pl_demo_get_data((const char**)&host_index, argv, argn, ARG_HTP_HOST_INDEX);
    pl_demo_get_value(&port, argv, argn, ARG_HTP_PORT);
    pl_demo_get_value(&version, argv, argn, ARG_HTP_VERSION);
    pl_demo_get_data((const char**)&proxy, argv, argn, ARG_HTP_PROXY);
    pl_demo_get_value(&proxy_port, argv, argn, ARG_HTP_PROXY_PORT);

    if (command == SC_HTP_OP_GET)
    {
        config_buf = sal_malloc(SAL_256);
        if(!config_buf)
        {
            ret.errcode = ERRCODE_DEFAULT;
            ret.msg = "htp malloc failed!";
            return ret;
        }
    }

    result = sAPI_HtpSrvConfig((SChtpOperationType)command, config_buf, cmd, host_index, port, version, proxy, proxy_port);

    TEST_LOG_INFO("htp config result: %d", result);
    out_arg[0].type = TYPE_STR;
    if (command == SC_HTP_OP_GET)
    {
        out_arg[0].arg_name = "HTP_CONFIG_GET";
        out_arg[0].value = pl_demo_make_value(0, config_buf, strlen(config_buf));
    }
    else
    {
        out_arg[0].arg_name = "HTP_CONFIG_SET";
        if(SC_HTP_OK == result)
        {
            TEST_LOG_INFO("SET SERVER SUCCESSFUL");
            out_arg[0].value = pl_demo_make_value(-1, "OK", 2);
        }
        else
        {
            TEST_LOG_INFO("SET SERVER ERROR");
            out_arg[0].value = pl_demo_make_value(-1, "ERROR", 5);
        }
    }

    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
    if (0 != ret.errcode)
    {
        ret.msg = "post data fail";
    }
    pl_demo_release_value(out_arg[0].value);
    if(config_buf) sal_free(config_buf);
    return ret;
}

static sMsgQRef htp_msgq = NULL;
ret_msg_t htp_update_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    SIM_MSG_T htp_result = {SC_SRV_NONE, -1, 0, NULL};
    char msgQcount = 0;

    if (NULL == htp_msgq)
    {
        SC_STATUS status;
        status = sAPI_MsgQCreate(&htp_msgq, "htpUIResp_msgq", sizeof(SIM_MSG_T), 4, SC_FIFO);
        if (SC_SUCCESS != status)
        {
            TEST_LOG_ERROR("[CHTP]msgQ create fail");
        }
    }

    ret.errcode = sAPI_HtpUpdate(htp_msgq);
    TEST_LOG_INFO("htp update ret = %d", ret.errcode);
    if(ret.errcode == SC_HTP_OK)
    {
        ret.errcode = ERRCODE_OK;
        ret.msg = "htp updata success!";
    }
    else
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "htp updata failed!";
    }

    do
    {
        sAPI_MsgQRecv(htp_msgq, &htp_result, 1000);
        if (SC_SRV_HTP != htp_result.msg_id)
        {
            TEST_LOG_INFO("[CHTP] htp_result.msg_id =[%d]", htp_result.msg_id);
            htp_result.msg_id = SC_SRV_NONE;
            htp_result.arg1 = -1;
            htp_result.arg3 = NULL;
            if(++msgQcount > 6) break;
            continue;
        }
        if (SC_HTP_OK == htp_result.arg1)
        {
            TEST_LOG_INFO("[CHTP] successfully update time! ");
            break;
        }
        else
        {
            TEST_LOG_INFO("[CHTP] failed to update time! result code: %d", htp_result.arg1);
            break;
        }
    } while (1);

    return ret;
}
#endif

#ifdef FEATURE_SIMCOM_NTP
static sMsgQRef ntp_msgq = NULL;
ret_msg_t ntp_update_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    int command = 0, time_zone = 0;
    char *host_addr = NULL;
    SCntpReturnCode result = SC_NTP_OK;
    char msgQcount = 0;

    pl_demo_get_value(&command, argv,argn, ARG_NTP_COMMAND);
    if(command == SC_NTP_OP_GET)
    {
        host_addr = sal_malloc(220);
    }
    else
    {
        pl_demo_get_data((const char**)&host_addr, argv,argn, ARG_NTP_HOSTADDR);
    }
    pl_demo_get_value(&time_zone, argv,argn, ARG_NTP_TIMEZONE);
    if(command == SC_NTP_OP_SET || command == SC_NTP_OP_GET)
    {
        result = sAPI_NtpUpdate(command, (char *)host_addr, time_zone, NULL);
        ret.errcode = (int)result;
        if(command == SC_NTP_OP_SET)
            ret.msg = "NTP Update SC_NTP_OP_SET";
        else
        {
            TEST_LOG_INFO("[CNTP] ntp_result =[%s]", host_addr);
            ret.msg = "NTP Update SC_NTP_OP_GET";
        }
    }
    else if(command == SC_NTP_OP_EXC)
    {
        SIM_MSG_T ntp_result = {SC_SRV_NONE, -1, 0, NULL};
        sAPI_MsgQCreate(&ntp_msgq, "ntpUIResp_msgq", sizeof(SIM_MSG_T), 4, SC_FIFO);
        result = sAPI_NtpUpdate(command, (char *)host_addr, time_zone, ntp_msgq);
        do
        {
            sAPI_MsgQRecv(ntp_msgq, &ntp_result, 1000);

            if (SC_SRV_NTP != ntp_result.msg_id)                  //wrong msg received
            {
                TEST_LOG_INFO("[CNTP] ntp_result.msg_id =[%d], ntp_result.msg_id ", ntp_result.msg_id);
                ntp_result.msg_id = SC_SRV_NONE;                   //para reset
                ntp_result.arg1 = -1;
                ntp_result.arg3 = NULL;
                if(++msgQcount > 6) break;
                continue;
            }
            if (SC_NTP_OK == ntp_result.arg1)                       //it means update succeed
            {
                ret.errcode = SC_NTP_OK;
                ret.msg = "NTP Update Time Successful";
                break;
            }
            else
            {
                ret.errcode = ntp_result.arg1;
                ret.msg = "ntp update SC_NTP_OP_EXC failed";
                break;
            }
        } while (1);
    }

    if(command == SC_NTP_OP_GET) sal_free(host_addr);
    return ret;
}
#endif

#ifdef FEATURE_SIMCOM_SSL
ret_msg_t ssl_set_context_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    int sslid = 0;
    const char *oper = NULL, *buffer = NULL;

    pl_demo_get_data(&oper, argv,argn, ARG_SSL_OPER);
    pl_demo_get_value(&sslid, argv,argn, ARG_SSL_ID);
    pl_demo_get_data(&buffer, argv,argn, ARG_SSL_BUF);
    ret.errcode = sAPI_SslSetContextIdMsg((char *)oper, sslid, (char *)buffer);
    ret.msg = "ssl set context";
    return ret;
}

ret_msg_t ssl_get_context_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    int sslid = 0;
    arg_t out_arg[1] = {0};
    char rsp_str[SAL_1K] = {0};

    pl_demo_get_value(&sslid, argv, argn, ARG_SSL_ID);
    SCsslContent msg = sAPI_SslGetContextIdMsg(sslid);
    snprintf(rsp_str, sizeof(rsp_str),
        "enable_sni = %d\r\n"
        "ignorelocaltime = %d\r\n"
        "ssl_version = %d\r\n"
        "auth_mode = %d\r\n"
        "negotiate_time_s = %d\r\n"
        "root_cert_path = %s\r\n"
        "client_cert_path = %s\r\n"
        "client_key_path = %s\r\n",
        msg.enable_SNI,
        msg.ignore_local_time,
        msg.ssl_version,
        msg.auth_mode,
        msg.negotiate_time,
        msg.ca_cert_path,
        msg.client_cert_path,
        msg.client_key_path);

    out_arg[0].arg_name = "outData";
    out_arg[0].type = TYPE_RAW;
    out_arg[0].value = pl_demo_make_value(0, (const char *)rsp_str, strlen(rsp_str));

    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
    pl_demo_post_raw((const char *)rsp_str, strlen(rsp_str));
    pl_demo_release_value(out_arg[0].value);

    ret.msg = "ssl get context";
    return ret;
}

static pl_crypto_handle g_test_handle = NULL;
ret_msg_t crypto_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    int oper, mode, rsa_size, ecc_size, symm_size, buf_len, test_len, encrypt_len;
    const char *input = NULL;
    char *encrypt_data = NULL;
    arg_t out_arg[1] = {0};

    pl_demo_get_value(&oper, argv,argn, ARG_CRYPTO_OP);
    pl_demo_get_value(&mode, argv,argn, ARG_CRYPTO_MODE);
    pl_demo_get_data(&input, argv,argn, ARG_SSL_OPER);
    test_len = strlen(input);
    //note: RSA-2048: block_size = (2048/8) = 256,  plaintext_max_len = block_size - 11
    rsa_size = ((test_len - 1) / (256 - 11) + 1) * 256;

    //note: ECC: encrypt_len Approximately equal to (plaintext_len * 2);
    ecc_size = test_len * 2;

    //note: Symmetric encryption algorithm: encrypt_len = plaintext_len + block_size(max 16)
    symm_size = test_len + 16;

    //note: MD: encrypt_len = block_size(max 64)
    //md_size = 64;

    buf_len = MAX(MAX(rsa_size, ecc_size), symm_size);

    encrypt_data = sal_malloc(buf_len + 1);
    memset(encrypt_data, 0, buf_len + 1);
    ret.errcode = pl_crypto(g_test_handle, oper, mode, (const unsigned char *)input, test_len, (unsigned char *)encrypt_data, 
        (unsigned int *)&encrypt_len, (unsigned int)buf_len);
    out_arg[0].type = TYPE_STR;
    out_arg[0].arg_name = "outData";
    out_arg[0].value = pl_demo_make_value(0, encrypt_data, encrypt_len);
    pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
    pl_demo_release_value(out_arg[0].value);
    ret.msg = "crypto";
    sal_free(encrypt_data);
    return ret;
}

ret_msg_t crypto_create_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    ret.errcode = pl_crypto_create(&g_test_handle);
    ret.msg = "crypto create";
    return ret;
}

ret_msg_t crypto_reset_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    ret.errcode = pl_crypto_reset(g_test_handle);
    ret.msg = "crypto reset";
    return ret;
}

ret_msg_t crypto_delete_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    ret.errcode = pl_crypto_delete(&g_test_handle);
    ret.msg = "crypto delete";
    return ret;
}

ret_msg_t crypto_cfg_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    int type, cfg;

    pl_demo_get_value(&cfg, argv,argn, ARG_CRYPTO_CFG);
    pl_demo_get_value(&type, argv,argn, ARG_CRYPTO_DATA);
    ret.errcode = pl_crypto_cfg(g_test_handle, cfg, (void *)&type);
    ret.msg = "crypto cfg";
    return ret;
}

ret_msg_t crypto_get_cfg_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    arg_t out_arg[1] = {0};
    // char rsp_str[SAL_4K] = {0};
    char *rsp_str = NULL;

    const pl_crypto_info_t* hd = pl_crypto_get_cfg(g_test_handle);

    int str_len = (hd->public_key.len 
                 + hd->private_key.len 
                 + hd->password.len 
                 + hd->hash.len 
                 + hd->sig.len 
                 + hd->key.len 
                 + hd->ad.len 
                 + hd->tag.len 
                 + hd->hmac_key.len 
                 + 256);

    rsp_str = sal_malloc(str_len + 1);
    memset(rsp_str, 0, str_len + 1);

    //This is just an example and does not represent actual usage
    snprintf(rsp_str, str_len + 1,
        "type = %d\r\n"
        "pk_md_type = %d\r\n"
        "public_key = %s\r\n"
        "private_key = %s\r\n"
        "password = %s\r\n"
        "hash = %s\r\n"
        "sig = %s\r\n"
        "mode = %d\r\n"
        "padding = %d\r\n"
        "key = %s\r\n"
        "iv = %s\r\n"
        "ad = %s\r\n"
        "tag = %s\r\n"
        "hmac_key = %s\r\n",
        hd->type,
        hd->pk_md_type,
        hd->public_key.buf,
        hd->private_key.buf,
        hd->password.buf,
        hd->hash.buf,
        hd->sig.buf,
        hd->mode,
        hd->padding,
        hd->key.buf,
        hd->iv,
        hd->ad.buf,
        hd->tag.buf,
        hd->hmac_key.buf);

    out_arg[0].arg_name = "outData";
    out_arg[0].type = TYPE_RAW;
    out_arg[0].value = pl_demo_make_value(0, (const char *)rsp_str, strlen(rsp_str));

    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
    pl_demo_post_raw((const char *)rsp_str, strlen(rsp_str));
    pl_demo_release_value(out_arg[0].value);

    ret.msg = "ssl get context";
    sal_free(rsp_str);
    return ret;
}

ret_msg_t crypto_random_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    const char *custom = NULL;
    char *data = NULL;
    int datalen = 0;
    arg_t out_arg[1] = {0};

    pl_demo_get_value(&datalen, argv,argn, ARG_CRYPTO_CUSTOM_OUTLEN);
    data = sal_malloc(datalen + 1);
    memset(data, 0, datalen + 1);
    pl_demo_get_data(&custom, argv,argn, ARG_CRYPTO_CUSTOM);
    ret.errcode = pl_crypto_random((unsigned char *)data, datalen, (const unsigned char *)custom, strlen(custom));
    if(ret.errcode == 0)
    {
        out_arg[0].type = TYPE_STR;
        out_arg[0].arg_name = "outData";
        out_arg[0].value = pl_demo_make_value(0, data, datalen);
        pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
        pl_demo_release_value(out_arg[0].value);
    }
    ret.msg = "crypto random";
    sal_free(data);
    return ret;
}

ret_msg_t crypto_sslTlsPrf_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    int prf_type;
    const char *secret = NULL, *label = NULL, *random = NULL;
    char dstbuf[8] = {0};
    arg_t out_arg[1] = {0};

    pl_demo_get_value(&prf_type, argv,argn, ARG_CRYPTO_PRFTYPE);
    pl_demo_get_data(&secret, argv,argn, ARG_CRYPTO_SECRET);
    pl_demo_get_data(&label, argv,argn, ARG_CRYPTO_LABEL);
    pl_demo_get_data(&random, argv,argn, ARG_CRYPTO_RANDOM);
    ret.errcode = pl_ssl_tls_prf(prf_type, (const unsigned char *)secret, strlen(secret), (const char *)label,
        (const unsigned char *)random, strlen(random), (unsigned char *)dstbuf, sizeof(dstbuf));
    if(ret.errcode == 0)
    {
        out_arg[0].type = TYPE_RAW;
        out_arg[0].arg_name = "outData";
        out_arg[0].value = pl_demo_make_value(0, (const char *)dstbuf, 8);
        pl_demo_post_raw((const char *)dstbuf, sizeof(dstbuf));
        pl_demo_release_value(out_arg[0].value);
    }
    ret.msg = "crypto ssl/tls prf";
    return ret;
}
#endif

#ifdef FEATURE_SIMCOM_MQTT
static sal_msgq_ref mqtt_recv_publish_msgq = NULL;
static sal_task_ref mqttProcesser;
static int inited = 0;

static void mqtt_recv_subscribe_data_process(void *arg)
{
    SIM_MSG_T msg = {0, 0, 0, NULL};
    SC_STATUS status;
    SCmqttData *recv_data = NULL;
    arg_t out_arg[1] = {0};
    char title[50] = {0};
    char *put_data = NULL;
    const int data_length = 10*1024+1024+500;
    put_data = sal_malloc(data_length+1);
    if(put_data==NULL)
        TEST_LOG_ERROR("malloc data fail!!!");

    while(1)
    {
        status = sAPI_MsgQRecv(mqtt_recv_publish_msgq, &msg, SC_SUSPEND);
        if (status == SC_SUCCESS)
        {
            int i=0;
            if ((SC_SRV_MQTT != msg.msg_id) || (0 != msg.arg1)
                || (NULL == msg.arg3))
                continue;
            recv_data = (SCmqttData *)(msg.arg3);
            TEST_LOG_INFO("index:[%d],tpoic_len:[%d],tpoic:[%s]", recv_data->client_index, recv_data->topic_len,recv_data->topic_P); 
            TEST_LOG_INFO("payload_len:[%d]",recv_data->payload_len);
            i += snprintf(put_data+i, data_length-i, "topic: [%.*s]\r\n", recv_data->topic_len, recv_data->topic_P);
            i += snprintf(put_data+i, data_length-i, "payload: [");
            memcpy(put_data+i, recv_data->payload_P, recv_data->payload_len);
            i += recv_data->payload_len;
            i += snprintf(put_data+i, data_length-i, "]\r\n");
            if(i > data_length)
                TEST_LOG_ERROR("data over length of malloc!!");
                
            snprintf(title, sizeof(title), "recv subscribe data from mqtt server index[%d]", recv_data->client_index);
            out_arg[0].arg_name = title;
            out_arg[0].type = TYPE_RAW;
            out_arg[0].value = pl_demo_make_value(0, put_data, i);
            pl_demo_post_urc(&func_mqtt_sub, TYPE_URC_DATA, out_arg, 1, (const char *)put_data, i);
            pl_demo_release_value(out_arg[0].value);
            
            /*these msg pointer must be free after using, don not change the free order*/
            sAPI_Free(recv_data->topic_P);
            sAPI_Free(recv_data->payload_P);
            sAPI_Free(recv_data);
        }
    }
}

ret_msg_t mqtt_start_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };

    if (inited == 0)
    {
        ret.msg = "enter -->";

        if(sal_msgq_create(&mqtt_recv_publish_msgq, "RvMq", sizeof(SIM_MSG_T), 4) != 0)
        {
            TEST_LOG_ERROR("MQTT Init create msgq Fail !");
            ret.errcode = ERRCODE_DEFAULT;
            ret.msg = "MQTT Init create msgq Fail !";
            return ret;
        }
        if(sal_task_create(&mqttProcesser, NULL, SAL_4K, sal_task_priority_low_1, "MqTk", mqtt_recv_subscribe_data_process, NULL) != 0)
        {
            TEST_LOG_ERROR("MQTT Init create task Fail !");
            ret.errcode = ERRCODE_DEFAULT;
            ret.msg = "MQTT Init create task Fail !";
            return ret;
        }
        inited = 1;
    }

    ret.errcode = sAPI_MqttStart(-1);
    ret.msg = "mqtt start";
    return ret;
}

void app_MqttLostConnCb(int index, int cause)
{
    char notice[100] = {0};
    arg_t out_arg[1] = {0};
    
    snprintf(notice, sizeof(notice) - 1, "MQTT LostConn Notice:index_%d,cause_%d", index, cause);
    
    out_arg[0].arg_name = "MQTT offline";
    out_arg[0].type = TYPE_RAW;
    out_arg[0].value = pl_demo_make_value(0, notice, strlen(notice));
    pl_demo_post_urc(&func_mqtt_connect, TYPE_URC_NOTICE, out_arg, 1, notice, strlen(notice));
    pl_demo_release_value(out_arg[0].value);
}

ret_msg_t mqtt_accq_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    int index = 0, ssl_enable =0;
    const char *client_id = NULL;
    
    pl_demo_get_value(&index, argv, argn, ARG_MQTT_INDEX);
    pl_demo_get_data(&client_id, argv, argn, ARG_MQTT_CLIENTID);
    pl_demo_get_value(&ssl_enable, argv, argn, ARG_MQTT_SSLENABLE);
    ret.errcode = sAPI_MqttAccq( 0, NULL, index, (char *)client_id, ssl_enable, mqtt_recv_publish_msgq);
    ret.msg = "mqtt accq";
    return ret;
}

ret_msg_t mqtt_connLostCb_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };

    sAPI_MqttConnLostCb(app_MqttLostConnCb);
    ret.errcode = 0;
    ret.msg = "mqtt connLostCb";
    return ret;
}

ret_msg_t mqtt_connect_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int index = 0, alive_time = 0, clean_session = 0;
    const char *host_tmp = NULL;
    const char *username = NULL;
    const char *password = NULL;
    char host[257] = {0};

    pl_demo_get_value(&index, argv, argn, ARG_MQTT_INDEX);
    pl_demo_get_value(&alive_time, argv, argn, ARG_MQTT_ALIVE_TIME);
    pl_demo_get_value(&clean_session, argv, argn, ARG_MQTT_CLEAN_SESSION);
    pl_demo_get_data(&host_tmp, argv, argn, ARG_MQTT_SERVER_HOST);
    pl_demo_get_data(&username, argv, argn, ARG_MQTT_USER_NAME);
    pl_demo_get_data(&password, argv, argn, ARG_MQTT_PASSWORD);

    snprintf(host, 256, "%s", host_tmp);
    ret.errcode = sAPI_MqttConnect(0, NULL, index, host, alive_time, clean_session, (char *)username, (char *)password);
    TEST_LOG_INFO("ret.errcode=%d", ret.errcode);
    ret.msg = "mqtt connect";
    return ret;
}

ret_msg_t mqtt_disconnect_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int index = 0, timeout = 0;

    pl_demo_get_value(&index, argv, argn, ARG_MQTT_INDEX);
    pl_demo_get_value(&timeout, argv, argn, ARG_MQTT_DISSCON_TIMEOUT);

    ret.errcode = sAPI_MqttDisConnect(0, NULL, index, timeout);
    TEST_LOG_INFO("ret.errcode=%d", ret.errcode);
    ret.msg = "mqtt disconnect";

    return ret;
}

ret_msg_t mqtt_sub_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int index = 0, qos = 0, dup = 0;
    const char *topic_data = NULL;

    pl_demo_get_value(&index, argv, argn, ARG_MQTT_INDEX);
    pl_demo_get_data(&topic_data, argv, argn, ARG_MQTT_TOPIC_DATA);
    pl_demo_get_value(&qos, argv, argn, ARG_MQTT_QOS);
    pl_demo_get_value(&dup, argv, argn, ARG_MQTT_DUP);

    ret.errcode = sAPI_MqttSub(index, (char *)topic_data, strlen(topic_data), qos, dup);
    TEST_LOG_INFO("ret.errcode=%d", ret.errcode);
    ret.msg = "mqtt subscribe";

    return ret;
}

ret_msg_t mqtt_unsub_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int index = 0, dup = 0;
    const char *topic_data = NULL;

    pl_demo_get_value(&index, argv, argn, ARG_MQTT_INDEX);
    pl_demo_get_data(&topic_data, argv, argn, ARG_MQTT_TOPIC_DATA);
    pl_demo_get_value(&dup, argv, argn, ARG_MQTT_DUP);
    ret.errcode = sAPI_MqttUnsub(index, (char *)topic_data, strlen(topic_data), dup);

    TEST_LOG_INFO("ret.errcode=%d", ret.errcode);
    ret.msg = "mqtt unsub";

    return ret;
}

ret_msg_t mqtt_topic_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int index = 0;
    const char *topic_data = NULL;

    pl_demo_get_value(&index, argv, argn, ARG_MQTT_INDEX);
    pl_demo_get_data(&topic_data, argv, argn, ARG_MQTT_TOPIC_DATA);
    ret.errcode = sAPI_MqttTopic(index, (char *)topic_data, strlen(topic_data));
    TEST_LOG_INFO("ret.errcode=%d", ret.errcode);
    ret.msg = "mqtt topic";

    return ret;
}

ret_msg_t mqtt_payload_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int index = 0;
    const char *payload_data = NULL;

    pl_demo_get_value(&index, argv, argn, ARG_MQTT_INDEX);
    pl_demo_get_data(&payload_data, argv, argn, ARG_MQTT_PAYLOAD_DATA);
    ret.errcode = sAPI_MqttPayload(index, (char *)payload_data, strlen(payload_data));
    ret.msg = "mqtt subscribe";

    return ret;
}
ret_msg_t mqtt_pub_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int index = 0, qos = 0, dup = 0, ratained = 0;

    pl_demo_get_value(&index, argv, argn, ARG_MQTT_INDEX);
    pl_demo_get_value(&qos, argv, argn, ARG_MQTT_QOS);
    pl_demo_get_value(&ratained, argv, argn, ARG_MQTT_RATAINED);
    pl_demo_get_value(&dup, argv, argn, ARG_MQTT_DUP);
    ret.errcode = sAPI_MqttPub(index, qos, 60, ratained, dup);
    ret.msg = "mqtt pub";

    return ret;

}

ret_msg_t mqtt_rel_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int index = 0;

    pl_demo_get_value(&index, argv, argn, ARG_MQTT_INDEX);
    ret.errcode = sAPI_MqttRel(index);
    ret.msg = "mqtt rel";

    return ret;
}

ret_msg_t mqtt_stop_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};

    ret.errcode = sAPI_MqttStop();
    ret.msg = "mqtt stop";

    return ret;
}

ret_msg_t mqtt_willtopic_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int index = 0;
    const char *willtopic_data = NULL;

    pl_demo_get_value(&index, argv, argn, ARG_MQTT_INDEX);
    pl_demo_get_data(&willtopic_data, argv, argn, ARG_MQTT_WILLTOPIC_DATA);
    ret.errcode = sAPI_MqttWillTopic(index, (char *)willtopic_data, strlen(willtopic_data));

    TEST_LOG_INFO("ret.errcode=%d", ret.errcode);
    ret.msg = "mqtt willtopic";

    return ret;
}

ret_msg_t mqtt_willmsg_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int index = 0;
    const char *willmsg_data = NULL;

    pl_demo_get_value(&index, argv, argn, ARG_MQTT_INDEX);
    pl_demo_get_data(&willmsg_data, argv, argn, ARG_MQTT_WILLMSG_DATA);
    ret.errcode = sAPI_MqttWillTopic(index, (char *)willmsg_data, strlen(willmsg_data));

    TEST_LOG_INFO("ret.errcode=%d", ret.errcode);
    ret.msg = "mqtt willmsg";

    return ret;
}

ret_msg_t mqtt_subtopic_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int index = 0;
    int qos = 0;
    const char *sub_topic_data = NULL;

    pl_demo_get_value(&index, argv, argn, ARG_MQTT_INDEX);
    pl_demo_get_data(&sub_topic_data, argv, argn, ARG_MQTT_SUBTOPIC_DATA);
    pl_demo_get_value(&qos, argv, argn, ARG_MQTT_QOS);
    ret.errcode = sAPI_MqttSubTopic(index, (char *)sub_topic_data, strlen(sub_topic_data), qos);

    TEST_LOG_INFO("ret.errcode=%d", ret.errcode);
    ret.msg = "mqtt subtopic";

    return ret;
}

ret_msg_t mqtt_unsubtopic_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int index = 0;
    const char *unsub_topic_data = NULL;

    pl_demo_get_value(&index, argv, argn, ARG_MQTT_INDEX);
    pl_demo_get_data(&unsub_topic_data, argv, argn, ARG_MQTT_SUBTOPIC_DATA);
    ret.errcode = sAPI_MqttUNSubTopic(index, (char *)unsub_topic_data, strlen(unsub_topic_data));

    TEST_LOG_INFO("ret.errcode=%d", ret.errcode);
    ret.msg = "mqtt unsubtopic";

    return ret;
}

ret_msg_t mqtt_sslcfg_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int command_type = 0;
    int index = 0;
    int ssl_ctx_index = 0;

    pl_demo_get_value(&command_type, argv, argn, ARG_MQTT_CMD_TYPE);
    pl_demo_get_value(&index, argv, argn, ARG_MQTT_INDEX);
    pl_demo_get_value(&ssl_ctx_index, argv, argn, ARG_MQTT_SSL_INDEX);
    ret.errcode = sAPI_MqttSslCfg(command_type, NULL, index, ssl_ctx_index);

    TEST_LOG_INFO("ret.errcode=%d", ret.errcode);
    ret.msg = "mqtt sslcfg";

    return ret;
}

ret_msg_t mqtt_cfg_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int command_type = 0;
    int index = 0;
    int config_type = 0;
    int related_value = 0;

    pl_demo_get_value(&command_type, argv, argn, ARG_MQTT_CMD_TYPE);
    pl_demo_get_value(&index, argv, argn, ARG_MQTT_INDEX);
    pl_demo_get_value(&config_type, argv, argn, ARG_MQTT_CFG_TYPE);
    pl_demo_get_value(&related_value, argv, argn, ARG_MQTT_RELATED_VALUE);
    ret.errcode = sAPI_MqttCfg(command_type, NULL, index, config_type, related_value);

    TEST_LOG_INFO("ret.errcode=%d", ret.errcode);
    ret.msg = "mqtt cfg";

    return ret;
}
#endif


#ifdef HAS_DEMO_FTPS
static sal_msgq_ref ftp_msgq = NULL;

ret_msg_t ftps_init_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int type = 0;
    SIM_MSG_T ftpsMsg = {0, 0, 0, NULL};

    if(ftp_msgq == NULL)
    {
        sAPI_MsgQCreate(&ftp_msgq, "ftps_msgq", sizeof(SIM_MSG_T), 20, SC_FIFO);
    }

    pl_demo_get_value(&type, argv, argn, ARG_FTPS_PDP_ACTIVE_TYPE);
    sAPI_FtpsInit(type, ftp_msgq);

    sAPI_MsgQRecv(ftp_msgq, &ftpsMsg, 12000);
    ret.errcode = ftpsMsg.arg2;
    if (SC_FTPS_RESULT_OK == ftpsMsg.arg2)
    {
        TEST_LOG_INFO("Init SUCCESS");
        ret.msg = "FTP Init Successful!";
    }
    else
    {
        TEST_LOG_INFO("Init FAIL,ERRCODE = [%d]", ftpsMsg.arg2);
        ret.msg = "FTP Init Fail!";
    }
    return ret;
}

ret_msg_t ftps_deinit_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int type = 0;
    SIM_MSG_T ftpsMsg = {0, 0, 0, NULL};
    SC_STATUS status;

    pl_demo_get_value(&type, argv, argn, ARG_FTPS_PDP_ACTIVE_TYPE);
    sAPI_FtpsDeInit(type);

    status = sAPI_MsgQRecv(ftp_msgq, &ftpsMsg, 12000);
    TEST_LOG_INFO("status = [%d]", status);

    if(SC_FTPS_RESULT_OK == ftpsMsg.arg2)
    {
        TEST_LOG_INFO("DEINIT SUCCESS");
        ret.msg = "FTP DeInit Successful!";
    }
    else
    {
        TEST_LOG_ERROR("DEINIT FAIL,ERRCODE = [%d]",ftpsMsg.arg2);
        ret.msg = "FTP DeInit Fail!";
    }
    ret.errcode = ftpsMsg.arg2;
    return ret;
}

ret_msg_t ftps_login_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    SCftpsLoginMsg msg = {0};
    const char *in_host = NULL;
    int in_port = 0;
    const char *in_username = NULL;
    const char *in_password = NULL;
    int server_type;
    SIM_MSG_T ftpsMsg = {0, 0, 0, NULL};

    pl_demo_get_data(&in_host, argv, argn, ARG_FTPS_HOST);
    memcpy(msg.host, in_host, sizeof(msg.host));
    pl_demo_get_value(&in_port, argv, argn, ARG_FTPS_PORT);
    msg.port = in_port;
    pl_demo_get_data(&in_username, argv, argn, ARG_FTPS_USERNAME);
    memcpy(msg.username,in_username,sizeof(msg.username));
    pl_demo_get_data(&in_password, argv, argn, ARG_FTPS_PASSWORD);
    memcpy(msg.password,in_password,sizeof(msg.password));
    pl_demo_get_value(&server_type, argv, argn, ARG_FTPS_SERVER_TYPE);
    msg.serverType = server_type;
    TEST_LOG_INFO("host = [%s],user = [%s],pass = [%s],servertype = [%d]", msg.host, msg.username, msg.password);

    sAPI_FtpsLogin(msg);
    sAPI_MsgQRecv(ftp_msgq, &ftpsMsg, 12000);

    ret.errcode = ftpsMsg.arg2;
    if (SC_FTPS_RESULT_OK == ftpsMsg.arg2)
    {
        TEST_LOG_INFO("LOGIN SUCCESSFUL");
        ret.msg = "FTP Login Successful!";
        return ret;
    }
    else
    {
        TEST_LOG_ERROR("LOGIN ERROR,ERROR CODE = [%d]", ftpsMsg.arg2);
        ret.msg = "FTP Login Fail!";
        return ret;
    }
    return ret;
}

ret_msg_t ftps_logout_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    SIM_MSG_T ftpsMsg = {0, 0, 0, NULL};

    sAPI_FtpsLogout();
    sAPI_MsgQRecv(ftp_msgq, &ftpsMsg, 12000);

    ret.errcode = ftpsMsg.arg2;
    if (SC_FTPS_RESULT_OK == ftpsMsg.arg2)
    {
        TEST_LOG_INFO("LOGOUT SUCCESSFUL");
        ret.msg = "FTP Logout Successful!";
        return ret;
    }
    else
    {
        TEST_LOG_ERROR("LOGOUT FAIL,ERRCODE = [%d]", ftpsMsg.arg2);
        ret.msg = "FTP Logout Fail!";
        return ret;
    }
    return ret;
}

ret_msg_t ftps_downloadFile_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *filePath = NULL;
    int ret_value = 0;
    int dest_loc = 0;
    SIM_MSG_T ftpsMsg = {0, 0, 0, NULL};

    pl_demo_get_data(&filePath, argv, argn, ARG_FTPS_FILENAME);
    pl_demo_get_value(&dest_loc, argv, argn, ARG_FTPS_LOCATION);
    TEST_LOG_INFO("filePath = [%s]", filePath);

    ret_value = sAPI_FtpsDownloadFile((char *)filePath, dest_loc);
    if (ret_value != SC_FTPS_RESULT_OK)
    {
        TEST_LOG_ERROR("DownloadFile FAIL,ERRCODE = [%d]", ftpsMsg.arg2);
        ret.msg = "FTP DownloadFile Fail!";
        ret.errcode = ftpsMsg.arg2;
        return ret;
    }
    sAPI_MsgQRecv(ftp_msgq, &ftpsMsg, 12000);

    if (SC_FTPS_RESULT_OK == ftpsMsg.arg2)
    {
        TEST_LOG_INFO("DOWNLOAD FILE SUCCESSFUL");
        ret.msg = "Download File Successful";
    }
    else
    {
        TEST_LOG_ERROR("DOWNLOAD FILE FAIL,ERRCODE = [%d]", ftpsMsg.arg2);
        ret.msg = "Download File Fail";
    }
    ret.errcode = ftpsMsg.arg2;
    return ret;
}

ret_msg_t ftps_downloadFileToBuffer_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *filePath = NULL;
    int ret_value = 0;
    int position = 0;
    arg_t out_arg[1] = {0};
    SIM_MSG_T ftpsMsg = {0, 0, 0, NULL};
    SC_STATUS status;
    SCapiFtpsData *ftpsData;

    pl_demo_get_data(&filePath, argv, argn, ARG_FTPS_FILENAME);
    pl_demo_get_value(&position, argv, argn, ARG_FTPS_POSITION);
    TEST_LOG_INFO("filePath = [%s]", filePath);

    ret_value = sAPI_FtpsDownloadFileToBuffer((char *)filePath, position);
    if (ret_value != SC_FTPS_RESULT_OK)
    {
        TEST_LOG_ERROR("DownloadFileToBuffer FAIL,ERRCODE = [%d]", ftpsMsg.arg2);
        ret.msg = "FTP DownloadFileToBuffer Fail!";
        ret.errcode = ftpsMsg.arg2;
        return ret;
    }

    while (1)
    {
        TEST_LOG_INFO("COMING TO THE RECV");
        ftpsMsg.arg3 = NULL;
        status = sAPI_MsgQRecv(ftp_msgq, &ftpsMsg, 12000);
        TEST_LOG_INFO("status = [%d]", status);

        if (SC_FTPS_RESULT_OK == ftpsMsg.arg2)
        {
            ftpsData = (SCapiFtpsData *)ftpsMsg.arg3;
            TEST_LOG_INFO("flag = [%d]", ftpsData->flag);
            if (SC_DATA_COMPLETE == ftpsData->flag)
            {
                TEST_LOG_INFO("get data complete!");
                TEST_LOG_INFO("flag = [%d],len = [%d]", ftpsData->flag, ftpsData->len);
                sal_free(ftpsData);
                break;
            }
            else if (SC_DATA_RESUME == ftpsData->flag)
            {
                TEST_LOG_INFO("get data successful!");
                TEST_LOG_INFO("flag = [%d],len = [%d]", ftpsData->flag, ftpsData->len);

                //just for waiting uart ok now
                out_arg[0].arg_name = pre_arg_name[ARG_ID_DATA];
                out_arg[0].type = TYPE_RAW;
                out_arg[0].value = pl_demo_make_value(0, ftpsData->data, ftpsData->len);
                pl_demo_post_data(OP_POST, out_arg,1,NULL, 0);
                pl_demo_release_value(out_arg[0].value);
                sal_free(ftpsData->data);
                sal_free(ftpsData);
            }
            else
            {
                TEST_LOG_ERROR("ERROR HAPPEN!");
                break;
            }
        }
        else
        {
            TEST_LOG_ERROR("error code = [%d]", ftpsMsg.arg2);
            ret.msg="FTP DOWNLOAD FILE TO BUFFER FAIL!";
            break;
        }

    }
    ret.errcode = ftpsMsg.arg2;
    return ret;
}

ret_msg_t ftps_uploadFile_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *filePath = NULL;
    int ret_value = 0;
    int position = 0;
    int location = 0;
    SIM_MSG_T ftpsMsg = {0, 0, 0, NULL};

    pl_demo_get_data(&filePath, argv, argn, ARG_FTPS_FILENAME);
    pl_demo_get_value(&location, argv, argn, ARG_FTPS_LOCATION);
    pl_demo_get_value(&position, argv, argn, ARG_FTPS_POSITION);
    TEST_LOG_INFO("filePath = [%s]", filePath);

    ret_value = sAPI_FtpsUploadFile((char *)filePath, location, position);
    if (ret_value != SC_FTPS_RESULT_OK)
    {
        TEST_LOG_ERROR("UploadFile FAIL,ERRCODE = [%d]", ftpsMsg.arg2);
        ret.msg = "FTP UploadFile Fail!";
        ret.errcode = ftpsMsg.arg2;
        return ret;
    }
    sAPI_MsgQRecv(ftp_msgq, &ftpsMsg, 12000);

    if (SC_FTPS_RESULT_OK == ftpsMsg.arg2)
    {
        TEST_LOG_INFO("UPLOAD FILE SUCCESSFUL");
        ret.msg = "Upload File Successful";
    }
    else
    {
        TEST_LOG_ERROR("UPLOAD FILE FAIL,ERRCODE = [%d]", ftpsMsg.arg2);
        ret.msg = "Upload File Fail";
    }
    ret.errcode = ftpsMsg.arg2;
    return ret;
}

ret_msg_t ftps_putFile_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *filePath = NULL;
    const char *destPath = NULL;
    int ret_value = 0;
    SIM_MSG_T ftpsMsg = {0, 0, 0, NULL};

    pl_demo_get_data(&filePath, argv, argn, ARG_FTPS_FILENAME);
    pl_demo_get_data(&destPath, argv, argn, ARG_FTPS_FILENAME);
    TEST_LOG_INFO("filePath = [%s]", filePath);

    ret_value = sAPI_FtpsPutFile((char *)filePath, (char *)destPath);;
    if (ret_value != SC_FTPS_RESULT_OK)
    {
        TEST_LOG_ERROR("putFile FAIL,ERRCODE = [%d]", ftpsMsg.arg2);
        ret.msg = "FTP put file Fail!";
        ret.errcode = ftpsMsg.arg2;
        return ret;
    }
    sAPI_MsgQRecv(ftp_msgq, &ftpsMsg, 12000);

    if (SC_FTPS_RESULT_OK == ftpsMsg.arg2)
    {
        TEST_LOG_INFO("PUT FILE SUCCESSFUL");
        ret.msg = "PUT File Successful";
    }
    else
    {
        TEST_LOG_ERROR("PUT FILE FAIL,ERRCODE = [%d]", ftpsMsg.arg2);
        ret.msg = "put File Fail";
    }
    ret.errcode = ftpsMsg.arg2;
    return ret;
}

ret_msg_t ftps_deleteFile_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *filePath = NULL;
    int ret_value = 0;
    SIM_MSG_T ftpsMsg = {0, 0, 0, NULL};

    pl_demo_get_data(&filePath, argv, argn, ARG_FTPS_FILENAME);
    TEST_LOG_INFO("filePath = [%s]", filePath);

    ret_value = sAPI_FtpsDeleteFile((char *)filePath);
    if (ret_value != SC_FTPS_RESULT_OK)
    {
        TEST_LOG_ERROR("DeleteFile FAIL,ERRCODE = [%d]", ret_value);
        ret.msg = "FTP DeleteFile Fail!";
        ret.errcode = ret_value;
        return ret;
    }
    sAPI_MsgQRecv(ftp_msgq, &ftpsMsg, 12000);

    if (SC_FTPS_RESULT_OK == ftpsMsg.arg2)
    {
        TEST_LOG_INFO("Delete File Successful");
        ret.msg = "Delete File Successful!";
    }
    else
    {
        TEST_LOG_ERROR("Delete File Fail");
        ret.msg = "Delete File Fail!";
    }
    ret.errcode = ftpsMsg.arg2;
    return ret;
}

ret_msg_t ftps_createDir_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *filePath = NULL;
    SIM_MSG_T ftpsMsg = {0, 0, 0, NULL};

    pl_demo_get_data(&filePath, argv, argn, ARG_FTPS_FILENAME);
    TEST_LOG_INFO("filePath = [%s]", filePath);

    sAPI_FtpsCreateDirectory((char *)filePath);
    sAPI_MsgQRecv(ftp_msgq, &ftpsMsg, 12000);

    ret.errcode = ftpsMsg.arg2;
    if (SC_FTPS_RESULT_OK == ftpsMsg.arg2)
    {
        TEST_LOG_INFO("Create Directory Successful");
        ret.msg = "Create Directory Successful!";
    }
    else
    {
        TEST_LOG_ERROR("Create Directory FAIL,ERRCODE = [%d]", ftpsMsg.arg2);
        ret.msg = "Craete Directory Fail!";
    }
    return ret;
}

ret_msg_t ftps_deleteDir_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *filePath = NULL;
    int ret_value = 0;
    SIM_MSG_T ftpsMsg = {0, 0, 0, NULL};

    pl_demo_get_data(&filePath, argv, argn, ARG_FTPS_FILENAME);
    TEST_LOG_INFO("filePath = [%s]", filePath);

    ret_value = sAPI_FtpsDeleteDirectory((char *)filePath);
    if (ret_value != SC_FTPS_RESULT_OK)
    {
        TEST_LOG_ERROR("DeleteFile FAIL,ERRCODE = [%d]", ret_value);
        ret.msg = "FTP DeleteFile Fail!";
        ret.errcode = ret_value;
        return ret;
    }
    sAPI_MsgQRecv(ftp_msgq, &ftpsMsg, 12000);

    if (SC_FTPS_RESULT_OK == ftpsMsg.arg2)
    {
        TEST_LOG_INFO("Delete Directory Successful");
        ret.msg = "Delete Directory Successful!";
    }
    else
    {
        TEST_LOG_ERROR("Delete Directory Fail");
        ret.msg = "Delete Directory Fail!";
    }
    ret.errcode = ftpsMsg.arg2;
    return ret;
}

ret_msg_t ftps_changeDir_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *filePath = NULL;
    SIM_MSG_T ftpsMsg = {0, 0, 0, NULL};

    pl_demo_get_data(&filePath, argv, argn, ARG_FTPS_FILENAME);
    TEST_LOG_INFO("filePath = [%s]", filePath);

    sAPI_FtpsChangeDirectory((char *)filePath);
    sAPI_MsgQRecv(ftp_msgq, &ftpsMsg, 12000);

    if (SC_FTPS_RESULT_OK == ftpsMsg.arg2)
    {
        TEST_LOG_INFO("Change Directory Successful");
        ret.msg = "Change Directory Successful!";
    }
    else
    {
        TEST_LOG_ERROR("Change Directory FAIL,ERRCODE = [%d]", ftpsMsg.arg2);
        ret.msg = "Change Directory Fail!";
    }
    ret.errcode = ftpsMsg.arg2;
    return ret;
}

ret_msg_t ftps_getCurDir_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char path[256] = {0};
    char *dir = NULL;
    SIM_MSG_T ftpsMsg = {0, 0, 0, NULL};

    sAPI_FtpsGetCurrentDirectory();
    sAPI_MsgQRecv(ftp_msgq, &ftpsMsg, 12000);

    if (SC_FTPS_RESULT_OK == ftpsMsg.arg2)
    {
        dir = (char *)ftpsMsg.arg3;
        TEST_LOG_INFO("GET CURRENT DIRECTORY SUCCESS = [%s]", dir);
        snprintf(path, sizeof(path), "current path = %s", dir);
        ret.msg= path;
    }
    else
    {
        TEST_LOG_ERROR("GET CURRENT DIRECTORY FAIL,ERRCODE = [%d]", ftpsMsg.arg2);
        ret.msg = "Get Directory Fail";
    }
    ret.errcode = ftpsMsg.arg2;
    return ret;
}

ret_msg_t ftps_list_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *filePath = NULL;
    int ret_value = 0;
    arg_t out_arg[1] = {0};

    SIM_MSG_T ftpsMsg = {0, 0, 0, NULL};
    SC_STATUS status;
    SCapiFtpsData *ftpsData;

    pl_demo_get_data(&filePath, argv, argn, ARG_FTPS_FILENAME);

    ret_value = sAPI_FtpsList((char *)filePath);
    if (ret_value != SC_FTPS_RESULT_OK)
    {
        TEST_LOG_INFO("LIST FAIL,ERRCODE = [%d]", ftpsMsg.arg2);
        ret.msg = "FTP LIST Fail!";
        ret.errcode = ftpsMsg.arg2;
        return ret;
    }

    while (1)
    {
        TEST_LOG_INFO("COMING TO THE RECV");
        ftpsMsg.arg3 = NULL;
        status = sAPI_MsgQRecv(ftp_msgq, &ftpsMsg, 12000);
        TEST_LOG_INFO("status = [%d]", status);

        if (SC_FTPS_RESULT_OK == ftpsMsg.arg2)
        {
            ftpsData = (SCapiFtpsData *)ftpsMsg.arg3;
            TEST_LOG_INFO("flag = [%d]", ftpsData->flag);
            if (SC_DATA_COMPLETE == ftpsData->flag)
            {
                TEST_LOG_INFO("get data complete!");
                TEST_LOG_INFO("flag = [%d],len = [%d]", ftpsData->flag, ftpsData->len);
                sal_free(ftpsData);
                break;
            }
            else if (SC_DATA_RESUME == ftpsData->flag)
            {
                TEST_LOG_INFO("get data successful!");
                TEST_LOG_INFO("flag = [%d],len = [%d]", ftpsData->flag, ftpsData->len);

                //just for waiting uart ok now
                out_arg[0].arg_name = pre_arg_name[ARG_ID_DATA];
                out_arg[0].type = TYPE_RAW;
                out_arg[0].value = pl_demo_make_value(0, ftpsData->data, ftpsData->len);
                pl_demo_post_data(OP_POST, out_arg,1,NULL, 0);

                pl_demo_release_value(out_arg[0].value);
                sal_free(ftpsData->data);
                sal_free(ftpsData);
            }
            else
            {
                TEST_LOG_ERROR("ERROR HAPPEN!");
                break;
            }
        }
        else
        {
            TEST_LOG_ERROR("error code = [%d]", ftpsMsg.arg2);
            ret.msg = "Ftp list fail!";
            break;
        }
    }
    ret.errcode = ftpsMsg.arg2;
    return ret;
}

ret_msg_t ftps_getFileSize_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *filePath = NULL;
    int ret_value = 0;
    char path[256] = {0};
    int *size = NULL;
    SIM_MSG_T ftpsMsg = {0, 0, 0, NULL};

    pl_demo_get_data(&filePath, argv, argn, ARG_FTPS_FILENAME);
    TEST_LOG_INFO("filePath = [%s]", filePath);

    ret_value = sAPI_FtpsGetFileSize((char *)filePath);
    if (ret_value != SC_FTPS_RESULT_OK)
    {
        TEST_LOG_INFO("DeleteFile FAIL,ERRCODE = [%d]", ret_value);
        ret.msg = "FTP DeleteFile Fail!";
        ret.errcode = ret_value;
        return ret;
    }
    sAPI_MsgQRecv(ftp_msgq, &ftpsMsg, 12000);

    if (SC_FTPS_RESULT_OK == ftpsMsg.arg2)
    {
        size = (int *)ftpsMsg.arg3;
        TEST_LOG_INFO("GET SIZE SUCESS,SIZE = [%d]", *size);
        snprintf(path, sizeof(path), "file size = %d", *size);
        ret.msg= path;
        free(size);
    }
    else
    {
        TEST_LOG_ERROR("GET SIZE FAIL,ERRCODE = [%d]", ftpsMsg.arg2);
        ret.msg = "Get File Size Fail!";
    }
    ret.errcode = ftpsMsg.arg2;
    return ret;
}

ret_msg_t ftps_transferType_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *type = NULL;
    int ret_value = 0;
    SIM_MSG_T ftpsMsg = {0, 0, 0, NULL};

    pl_demo_get_data(&type, argv, argn, ARG_FTPS_TYPE);
    TEST_LOG_INFO("arg3 = [%s]",type);

    ret_value = sAPI_FtpsTransferType((char *)type);
    if (ret_value != SC_FTPS_RESULT_OK)
    {
        TEST_LOG_ERROR("SetType FAIL,ERRCODE = [%d]", ret_value);
        ret.msg = "FTP Settype Fail!";
        ret.errcode = ret_value;
        return ret;
    }
    sAPI_MsgQRecv(ftp_msgq, &ftpsMsg,12000);
    
    if(SC_FTPS_RESULT_OK == ftpsMsg.arg2)
    {
        TEST_LOG_INFO("SETTYPE SUCCESSFUL");
        ret.msg = "SETTYPE Successful"; 
    }
    else
    {
        TEST_LOG_ERROR("SETTYPE FAIL,ERRCODE = [%d]",ftpsMsg.arg2);
        ret.msg = "SETTYPE Fail"; 
    }
    ret.errcode = ftpsMsg.arg2;
    return ret;
}

ret_msg_t ftps_getTransferType_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int ret_value = 0;
    SIM_MSG_T ftpsMsg = {0, 0, 0, NULL};

    ret_value = sAPI_FtpsGetTransferType();
    if (ret_value != SC_FTPS_RESULT_OK)
    {
        TEST_LOG_ERROR("SetType FAIL,ERRCODE = [%d]", ret_value);
        ret.msg = "FTP Settype Fail!";
        ret.errcode = ret_value;
        return ret;
    }
    
    sAPI_MsgQRecv(ftp_msgq, &ftpsMsg,12000);
    if(SC_FTPS_RESULT_OK == ftpsMsg.arg2)
    {
        char buff[50] = {0};
        sprintf(buff, "GETTYPE SUCCESSFUL [%lu]", ftpsMsg.msg_id);
        TEST_LOG_INFO("%s", buff);
        ret.msg = buff; 
    }
    else
    {
        TEST_LOG_ERROR("GETTYPE FAIL,ERRCODE = [%d]",ftpsMsg.arg2);
        ret.msg = "GETTYPE Fail"; 
    }
    ret.errcode = ftpsMsg.arg2;
    return ret;
}

ret_msg_t ftps_sslConfig_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int session = 0, sslId = 0;
    int ret_value = 0;
    SIM_MSG_T ftpsMsg = {0, 0, 0, NULL};

    pl_demo_get_value(&session, argv, argn, ARG_FTPS_SESSION);
    pl_demo_get_value(&sslId, argv, argn, ARG_FTPS_SSLID);
    TEST_LOG_INFO("arg3 = [%s]",session);

    ret_value = sAPI_FtpsSslConfig(session, sslId);
    if (ret_value != SC_FTPS_RESULT_OK)
    {
        TEST_LOG_ERROR("SetMode FAIL,ERRCODE = [%d]", ret_value);
        ret.msg = "FTP SetMode Fail!";
        ret.errcode = ret_value;
        return ret;
    }
    sAPI_MsgQRecv(ftp_msgq, &ftpsMsg, 12000);
    
    if(SC_FTPS_RESULT_OK == ftpsMsg.arg2)
    {
        TEST_LOG_INFO("sslconfig SUCCESSFUL");
        ret.msg = "sslconfig Successful"; 
    }
    else
    {
        TEST_LOG_ERROR("sslconfig FAIL,ERRCODE = [%d]",ftpsMsg.arg2);
        ret.msg = "sslconfig Fail"; 
    }
    ret.errcode = ftpsMsg.arg2;
    return ret;
}

ret_msg_t ftps_setMode_cmd_proc(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int mode = 0;
    int ret_value = 0;
    SIM_MSG_T ftpsMsg = {0, 0, 0, NULL};

    pl_demo_get_value(&mode, argv, argn, ARG_FTPS_MODE);
    TEST_LOG_INFO("arg3 = [%s]",mode);

    ret_value = sAPI_FtpsSetMode(mode);
    if (ret_value != SC_FTPS_RESULT_OK)
    {
        TEST_LOG_ERROR("SetMode FAIL,ERRCODE = [%d]", ret_value);
        ret.msg = "FTP SetMode Fail!";
        ret.errcode = ret_value;
        return ret;
    }
    sAPI_MsgQRecv(ftp_msgq, &ftpsMsg, 12000);
    
    if(SC_FTPS_RESULT_OK == ftpsMsg.arg2)
    {
        TEST_LOG_INFO("SETMODE SUCCESSFUL");
        ret.msg = "SETMODE Successful"; 
    }
    else
    {
        TEST_LOG_ERROR("SETMODE FAIL,ERRCODE = [%d]",ftpsMsg.arg2);
        ret.msg = "SETMODE Fail"; 
    }
    ret.errcode = ftpsMsg.arg2;
    return ret;
}
#endif

/*======================== for driver gpio begin ========================*/


ret_msg_t get_gpio_level_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int gpio_num = 0;
    pl_demo_get_value(&gpio_num, argv, argn, ARG_GPIO_NUMBER);

    ret.errcode = sAPI_GpioGetValue(gpio_num);
    ret.msg = "gpio get value";
    return ret;
}

void GPIO_IntHandler(void)
{
#ifdef GPIO_INT_WAKEUP_TEST
    static unsigned int i = 0;

    if (i % 2 == 0)
        sAPI_GpioSetValue(SC_MODULE_GPIO_10, 1);
    else
        sAPI_GpioSetValue(SC_MODULE_GPIO_10, 0);

    i++;
#endif
}

void GPIO_WakeupHandler_Test(void)
{
#ifdef GPIO_INT_WAKEUP_TEST
    static unsigned int i = 0;

    if (i % 2 == 0)
        sAPI_GpioSetValue(SC_MODULE_GPIO_09, 1);
    else
        sAPI_GpioSetValue(SC_MODULE_GPIO_09, 0);

    i++;
#endif
    /* If you want to leave sleep after wake up, please disable system sleep ! */
    //sAPI_SystemSleepSet(0);
}

ret_msg_t set_gpio_interrupt_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int gpio_num = 0;
    int transitiontype = 0;

    pl_demo_get_value(&gpio_num, argv, argn, ARG_GPIO_NUMBER);
    pl_demo_get_value(&transitiontype, argv, argn, ARG_GPIO_INTERRUPT);

    TEST_LOG_INFO("%s,gpionumber:%d , set_gpiointerrupt:[%d]", __func__, gpio_num, transitiontype);
    ret.errcode = sAPI_GpioConfigInterrupt(gpio_num, transitiontype, GPIO_IntHandler);
    ret.msg = "gpio config interrupt";
    return ret;
}

ret_msg_t set_gpio_config_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int gpio_num = 0;
    int direction = 0;
    int intlevel = 0;
    int pulltype = 0;
    int transitiontype = 0;

    SC_GPIOConfiguration pinconfig;

    pl_demo_get_value(&gpio_num, argv, argn, ARG_GPIO_NUMBER);
    pl_demo_get_value(&direction, argv, argn, ARG_GPIO_DIRECTION);
    pl_demo_get_value(&intlevel, argv, argn, ARG_GPIO_LEVEL);
    pl_demo_get_value(&pulltype, argv, argn, ARG_GPIO_PULLUPDOWN);
    pl_demo_get_value(&transitiontype, argv, argn, ARG_GPIO_INTERRUPT);

    TEST_LOG_INFO("%s,gpionumber:%d , direction:[%d] , intlevel:[%d] , pulltype:[%d] , interrupt:[%d]", __func__, gpio_num, direction,intlevel,pulltype,transitiontype);

    pinconfig.pinDir = direction;
    pinconfig.initLv = intlevel;
    pinconfig.pinPull = pulltype;
    pinconfig.pinEd = transitiontype;
    pinconfig.isr = GPIO_IntHandler;
    pinconfig.wu = GPIO_WakeupHandler_Test;
    ret.errcode = sAPI_GpioConfig(gpio_num, pinconfig);
    ret.msg = "gpio config";
    return ret;
}

ret_msg_t set_gpio_wakeupenable_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int gpio_num = 0;
    int wakeupenable = 0;

    pl_demo_get_value(&gpio_num, argv, argn, ARG_GPIO_NUMBER);
    pl_demo_get_value(&wakeupenable, argv, argn, ARG_GPIO_WAKEUPENABLE);


    TEST_LOG_INFO("%s,gpionumber:%d , set_gpiowakeupenable:[%d]", __func__, gpio_num, wakeupenable);
    ret.errcode = sAPI_GpioWakeupEnable(gpio_num, wakeupenable);
    ret.msg = "gpio wakeup enable";
    return ret;
}

ret_msg_t set_gpio_level_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int gpio_num = 0;
    int level = 0;


    pl_demo_get_value(&gpio_num, argv, argn, ARG_GPIO_NUMBER);
    pl_demo_get_value(&level, argv, argn, ARG_GPIO_LEVEL);

    TEST_LOG_INFO("%s,gpionumber:%d , set_gpiolevel:[%d]", __func__, gpio_num, level);
    ret.errcode = sAPI_GpioSetValue(gpio_num, level);
    ret.msg = "gpio set value";
    return ret;
}

ret_msg_t set_gpio_pinmux_func_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int gpio_num = 0;
    int pinmuxfunc = 0;

    pl_demo_get_value(&gpio_num, argv, argn, ARG_GPIO_NUMBER);
    pl_demo_get_value(&pinmuxfunc, argv, argn, ARG_GPIO_PINMUXFUNC);

    TEST_LOG_INFO("%s,gpionumber:%d , set_gpiopinmuxfunc:[%d]", __func__, gpio_num, pinmuxfunc);
    ret.errcode = sAPI_PinmuxFunc(gpio_num, pinmuxfunc);
    ret.msg = "set pinmux func";
    return ret;
}

ret_msg_t get_adc_value_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int adc_channel = 0;

    pl_demo_get_value(&adc_channel, argv, argn, ARG_POWER_ADC_CHANNEL);
    TEST_LOG_INFO("%s,adc_channel:%d", __func__, adc_channel);

    ret.errcode = sAPI_ReadAdc(adc_channel);
    ret.msg = "get adc value";
    return ret;
}

/*======================== for driver gpio end ========================*/

/*======================== for driver uart begin ========================*/
ret_msg_t uartRead_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int port = 0, datalen = 0;
    char *uartData = NULL;

    pl_demo_get_value(&port, argv, argn, ARG_UART_PORT);
    pl_demo_get_value(&datalen, argv, argn, ARG_UART_DATALEN);

    uartData = sal_malloc(datalen);
    if(uartData == NULL )
    {
        TEST_LOG_ERROR("malloc error");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "malloc error";
        return ret;
    }

    memset(uartData, 0, datalen);

    ret.errcode = sAPI_UartRead(port, (UINT8 *)uartData, datalen);
    ret.msg = "uart read";

    sal_free(uartData);
    return ret;
}

ret_msg_t uartWrite_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int port = 0;
    const char *data = NULL;

    pl_demo_get_value(&port, argv, argn, ARG_UART_PORT);
    pl_demo_get_data(&data, argv, argn, ARG_UART_UARTDATA);

    char *uartData = sal_malloc(strlen(data)+1);
    if(uartData == NULL )
    {
        TEST_LOG_ERROR("malloc error");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "malloc error";
        return ret;
    }

    strncpy(uartData, data, strlen(data));
    ret.errcode = sAPI_UartWrite(port, (UINT8 *)uartData, strlen(uartData));
    ret.msg = "uart write";

    sal_free(uartData);
    return ret;
}

ret_msg_t uartCtl_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int port = 0, ctl = 0;

    pl_demo_get_value(&port, argv, argn, ARG_UART_PORT);
    pl_demo_get_value(&ctl, argv, argn, ARG_UART_UARTCTL);

    ret.errcode = sAPI_UartControl(port, ctl);
    ret.msg = "uart control";

    return ret;
}

ret_msg_t uartModeCtl_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int mode = 0;

    pl_demo_get_value(&mode, argv, argn, ARG_UART_MODECTL);

    ret.errcode = sAPI_UartModeControl(mode);
    ret.msg = "uart mode control";

    return ret;
}

ret_msg_t uartConfigSet_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    SCuartConfiguration uartConfig;
    int port = 0, stopbits = 0, paritybit = 0, baudrate = 0, databits = 0;

    pl_demo_get_value(&port, argv, argn, ARG_UART_PORT);
    pl_demo_get_value(&baudrate, argv, argn, ARG_UART_BAUDRATE);
    pl_demo_get_value(&databits, argv, argn, ARG_UART_DATABITS);
    pl_demo_get_value(&stopbits, argv, argn, ARG_UART_STOPBITS);
    pl_demo_get_value(&paritybit, argv, argn, ARG_UART_PARITYBIT);

    uartConfig.BaudRate  = baudrate;
    uartConfig.DataBits  = databits;
    uartConfig.StopBits  = stopbits;
    uartConfig.ParityBit = paritybit;

    ret.errcode = sAPI_UartSetConfig(port, &uartConfig);
    ret.msg = "uart set config";

    return ret;
}

ret_msg_t uartRs485DeAssign_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int port = 0, GpinNum = 0, DeActiveLevel = 0;

    pl_demo_get_value(&port, argv, argn, ARG_UART_PORT);
    pl_demo_get_value(&GpinNum, argv, argn, ARG_UART_GPIONUM);
    pl_demo_get_value(&DeActiveLevel, argv, argn, ARG_UART_DELEVEL);

    ret.errcode = sAPI_UartRs485DePinAssignEx(port, GpinNum, DeActiveLevel);
    ret.msg = "uart rs485 de pin assign ex";

    return ret;
}
/*======================== for driver uart end ========================*/

/*======================== for driver 2IC begin =======================*/
ret_msg_t i2c_init_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    SC_I2C_DEV i2c_dev = {0};

    int i2c_port_input = 0;
    int i2c_clk = 0;
    pl_demo_get_value(&i2c_port_input, argv, argn, ARG_I2C_port);
    pl_demo_get_value(&i2c_clk, argv, argn, ARG_I2C_clk);


    i2c_dev.i2c_channel=(SC_I2C_CHANNEL)i2c_port_input;
    i2c_dev.i2c_clock = (SC_I2C_CLOCK)i2c_clk;


    ret.errcode  =  sAPI_I2CConfigInit(&i2c_dev);
    ret.msg = "I2CInit";
    return ret;
}

ret_msg_t i2c_deinit_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    SC_I2C_DEV i2c_dev = {0};
    int i2c_port = 0;


    pl_demo_get_value(&i2c_port, argv, argn, ARG_I2C_port);

    i2c_dev.i2c_channel=(SC_I2C_CHANNEL)i2c_port;

    ret.errcode = sAPI_I2CDeInit(&i2c_dev);
    ret.msg = "I2CDeInit";
    return ret;
}
int trans(char* str){
    unsigned int hex = 0;
    sscanf(str, "%x", &hex);
    return hex;
}
ret_msg_t i2c_write_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int i2c_port = 0;

    pl_demo_get_value(&i2c_port, argv, argn, ARG_I2C_port);

    i2c_port = i2c_port - 1;
    int h_slaveaddr = 0;
    int h_slaveregaddr = 0;
    int h_slaveregdata = 0;
    int datalen = 0;
    UINT8 i2c_data=0;

    const char *r_slaveaddr = NULL;
    char slaveaddr[4] = {0};
    pl_demo_get_data(&r_slaveaddr, argv, argn, ARG_SLAVE_ADDR);

    memcpy(slaveaddr, r_slaveaddr, strlen(r_slaveaddr));
    const char *r_slaveregaddr = NULL;
    char slaveregaddr[4] = { 0 };
    pl_demo_get_data(&r_slaveregaddr, argv, argn, ARG_REG_ADDR);
    
    memcpy(slaveregaddr, r_slaveregaddr, strlen(r_slaveregaddr));

    const char *r_slaveregdata = NULL;
    char slaveregdata[4] = { 0 };
    pl_demo_get_data(&r_slaveregdata, argv, argn, ARG_REG_DATA);
 
    memcpy(slaveregdata, r_slaveregdata, strlen(r_slaveregdata));


    pl_demo_get_value(&datalen, argv, argn, ARG_DATA_LEN);

    h_slaveaddr = trans(slaveaddr);
    h_slaveregaddr = trans(slaveregaddr);
    h_slaveregdata = trans(slaveregdata);
    i2c_data = h_slaveregdata;
    /* *****************only for nau8810 *********************/
    // regAddr = 0x41;
    // regData = 0x01a5;
    // i2cdata[0] = ((regAddr << 1) & 0xFE) | ((regData >> 8) & 0x01);
    // i2cdata[1] = (UINT8)(regData & 0xFF);
    /******************************************************/
 
    ret.errcode = sAPI_I2CWrite(i2c_port, h_slaveaddr, h_slaveregaddr, &(i2c_data), datalen);
    ret.msg = "I2CWrite";
    return ret;
}

ret_msg_t i2c_read_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    SC_I2C_ReturnCode i2c_ret = SC_I2C_RC_NOT_OK;
    int i2c_port = 0;
    UINT16 regData=0;
    arg_t out_arg[1] = {0};
    char buf[64] = {0};
    UINT8 r_i2cdata[2] = {0};
    pl_demo_get_value(&i2c_port, argv, argn, ARG_I2C_port);
    i2c_port = i2c_port - 1;

    int h_slaveaddr = 0;
    int h_slaveregaddr = 0;
    int datalen = 0;

    const char *r_slaveaddr = NULL;
    char slaveaddr[4] = {0};
    pl_demo_get_data(&r_slaveaddr, argv, argn, ARG_SLAVE_ADDR);

    memcpy(slaveaddr, r_slaveaddr, strlen(r_slaveaddr));
    const char *r_slaveregaddr = NULL;
    char slaveregaddr[4] = { 0 };
    pl_demo_get_data(&r_slaveregaddr, argv, argn, ARG_REG_ADDR);

    memcpy(slaveregaddr, r_slaveregaddr, strlen(r_slaveregaddr));

    pl_demo_get_value(&datalen, argv, argn, ARG_DATA_LEN);

    h_slaveaddr = trans(slaveaddr);
    h_slaveregaddr = trans(slaveregaddr);
    /* *****************only for nau8810 *********************/
    //  regAddr = 0x41;
    //  regData = 0x00;
    /******************************************************/
    i2c_ret = sAPI_I2CRead(i2c_port, h_slaveaddr, h_slaveregaddr, r_i2cdata, datalen);
    if(datalen == 2){
    regData = (((UINT16)r_i2cdata[0]) << 8) | r_i2cdata[1];
    }else if(datalen == 1)
    {
        regData = r_i2cdata[0];
    }
    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf)," port%d reg[0x%x]:0x%x\r\n", i2c_port+1,h_slaveregaddr, regData);
    out_arg[0].type = TYPE_STR;
    out_arg[0].arg_name = "IIC Read";
    out_arg[0].value = pl_demo_make_value(0, buf, strlen(buf));
  

    ret.errcode = i2c_ret;
    pl_demo_release_value(out_arg[0].value);

    return ret;
}
/*======================== for driver i2c end ========================*/
/*======================== for driver spi begin ======================*/
ret_msg_t spiinit_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    SC_SPI_DEV spiDev={0};
    int ssp_port = 0;
    int ssp_clk = 0;
    int ssp_mode = 0;
    int ssp_csmode = 0;


    pl_demo_get_value(&ssp_port, argv, argn, ARG_SPI_port);
    pl_demo_get_value(&ssp_clk, argv, argn, ARG_SPI_clk);
    pl_demo_get_value(&ssp_mode, argv, argn, ARG_SPI_mode);
    pl_demo_get_value(&ssp_csmode, argv, argn, ARG_SPI_cs);


    spiDev.clock = ssp_clk;
    spiDev.index = (unsigned char)ssp_port;
    spiDev.mode = ssp_mode;
    spiDev.csMode = ssp_csmode;
    ret.errcode = sAPI_SpiConfigInitEx(&spiDev);
    ret.msg="spi init";

    return ret;
}

ret_msg_t spireadid_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    SC_SPI_DEV spiDev={0};

    int ssp_port = 0;
    int ssp_clk = 0;
    int ssp_mode = 0;
    int ssp_csmode = 0;

    unsigned char SendData[4] = {0x9F, 0, 0, 0};
    unsigned char RevData[4] = {0};
    pl_demo_get_value(&ssp_port, argv, argn, ARG_SPI_port);
    pl_demo_get_value(&ssp_clk, argv, argn, ARG_SPI_clk);
    pl_demo_get_value(&ssp_mode, argv, argn, ARG_SPI_mode);
    pl_demo_get_value(&ssp_csmode, argv, argn, ARG_SPI_cs);
    spiDev.clock = ssp_clk;
    spiDev.index = (unsigned char)ssp_port;
    spiDev.mode = ssp_mode;
    spiDev.csMode = ssp_csmode;

    ret.errcode=sAPI_SpiReadBytesEx(&spiDev, SendData, 1, RevData, 3);

    ret.msg = "spi read flash id";

    return ret;
}

ret_msg_t spirwstatus_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    SC_SPI_DEV spiDev={0};

    int ssp_port = 0;
    int ssp_clk = 0;
    int ssp_mode = 0;
    int ssp_csmode = 0;

    pl_demo_get_value(&ssp_port, argv, argn, ARG_SPI_port);
    pl_demo_get_value(&ssp_clk, argv, argn, ARG_SPI_clk);
    pl_demo_get_value(&ssp_mode, argv, argn, ARG_SPI_mode);
    pl_demo_get_value(&ssp_csmode, argv, argn, ARG_SPI_cs);
    spiDev.clock = ssp_clk;
    spiDev.index = (unsigned char)ssp_port;
    spiDev.mode = ssp_mode;
    spiDev.csMode = ssp_csmode;

    unsigned char writeData[2] = {0x01};
    unsigned char readCmd = 0x05;
    unsigned char status = 0;

    sAPI_SpiReadBytesEx(&spiDev, &readCmd, 1, &status, 1);
   
    writeData[1] = status | 0x02;
    ret.errcode = sAPI_SpiWriteBytesEx(&spiDev, writeData, 2);
    ret.msg = "spi R/W status";
    return ret;
}
/*======================== for driver spi end ========================*/

static ret_msg_t gnss_pwr_set_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int power_switch = 0;

    pl_demo_get_value(&power_switch, argv, argn, ARG_GNSS_POWER);

    ret.errcode = sAPI_GnssPowerStatusSet((SC_Gnss_Power_Status)power_switch);
    ret.msg = "set gnss power";
    return ret;
}

static ret_msg_t gnss_pwr_get_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};

    ret.errcode = sAPI_GnssPowerStatusGet();
    ret.msg = "gnss pwr get";
    return ret;
}

static ret_msg_t gnss_nmea_data_get_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int is_get_nmea_data = SC_GNSS_STOP_OUTPUT_NMEA_DATA;
    int get_nmea_data_way = SC_GNSS_NMEA_DATA_GET_BY_PORT;

    pl_demo_get_value(&is_get_nmea_data, argv, argn, ARG_GNSS_NMEA_OUT);
    pl_demo_get_value(&get_nmea_data_way, argv, argn, ARG_GNSS_NMEA_PORT);

    ret.errcode = sAPI_GnssNmeaDataGet(is_get_nmea_data,get_nmea_data_way);
    ret.msg = "gnss nmea get";
    return ret;
}

static ret_msg_t gnss_start_mode_set_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int start_mode = 0;

    pl_demo_get_value(&start_mode, argv, argn, ARG_GNSS_START_MODE);

    ret.errcode = sAPI_GnssStartMode((SC_Gnss_Start_Mode)start_mode);
    ret.msg = "set gnss start mode";
    return ret;
}

static ret_msg_t gnss_baud_rate_set_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int baudrate = 0;

    pl_demo_get_value(&baudrate, argv, argn, ARG_GNSS_BAUD_RATE);

    ret.errcode = sAPI_GnssBaudRateSet((SC_Gnss_Start_Mode)baudrate);
    ret.msg = "set gnss start mode";
    return ret;
}

static ret_msg_t gnss_mode_get_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};

    ret.errcode = sAPI_GnssModeGet();
    ret.msg = "sAPI_GnssModeGet post data fail";
    
    return ret;
}

static ret_msg_t gnss_baud_rate_get_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};

    ret.errcode = sAPI_GnssBaudRateGet();
    ret.msg = "sAPI_GnssBaudRateGet";
    
    return ret;
}

static ret_msg_t gnss_nmea_rate_get_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};

    ret.errcode = sAPI_GnssNmeaRateGet();
    ret.msg = "sAPI_GnssNmeaRateGet";
    
    return ret;
}

static ret_msg_t gnss_nmea_rate_set_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int nmearate = 0;

    pl_demo_get_value(&nmearate, argv, argn, ARG_GNSS_NMEA_RATE);

    ret.errcode = sAPI_GnssNmeaRateSet((SC_Gnss_Nmea_Rate)nmearate);
    ret.msg = "set gnss nmea rate";
    return ret;
}

static ret_msg_t gnss_mode_set_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int mode = 0;

    pl_demo_get_value(&mode, argv, argn, ARG_GNSS_MODE);

    ret.errcode = sAPI_GnssModeSet((SC_Gnss_Mode)mode);
    ret.msg = "set gnss nmea rate";
    return ret;
}

static ret_msg_t gnss_nmea_sentence_get_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};

    ret.errcode = (int)sAPI_GnssNmeaSentenceGet();
    ret.msg = "sAPI_GnssNmeaSentenceGet";

    return ret;
}

static ret_msg_t gnss_nmea_sentence_set_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int gnss_nmea_sentence = 0;

    pl_demo_get_value(&gnss_nmea_sentence, argv, argn, ARG_GNSS_NMEASENTENCE);

    ret.errcode = sAPI_GnssNmeaSentenceSet(gnss_nmea_sentence);
    ret.msg = "sAPI_GnssNmeaSentenceSet";

    return ret;
}

static ret_msg_t gnss_and_gps_info_get_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int gnss_or_info_get_period = 0;

    pl_demo_get_value(&gnss_or_info_get_period, argv, argn, ARG_GNSS_GPS_INFO_GET);

    ret.errcode = sAPI_GpsInfoGet(gnss_or_info_get_period);
    ret.msg = "sAPI_GpsInfoGet";

    
    ret.errcode = sAPI_GnssInfoGet(gnss_or_info_get_period);
    ret.msg = "sAPI_GnssInfoGet";

    return ret;
}

static ret_msg_t gnss_cmd_send_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *gnssCmd = NULL;

    pl_demo_get_data(&gnssCmd, argv, argn, ARG_GNSS_CMD);

    ret.errcode = sAPI_SendCmd2Gnss((char *)gnssCmd);
    ret.msg = "sAPI_SendCmd2Gnss";

    return ret;
}

static ret_msg_t gnss_agps_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};

    ret.errcode = sAPI_GnssAgpsSeviceOpen();
    ret.msg = "sAPI_GpsInfoGet";
    return ret;
}

ret_msg_t rtos_enableDump_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    sAPI_enableDUMP();
    ret.msg = "enable dump";

    return ret;
}

ret_msg_t rtos_disableDump_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    sAPI_disableDUMP();
    ret.msg = "disable dump";

    return ret;
}

ret_msg_t rtos_getTicks_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    ret.errcode = sAPI_GetTicks();
    ret.msg = "get ticks";
    return ret;
}

ret_msg_t rtos_getSysVersion_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char buf[512] = {0};
    arg_t out_arg[1] = {0};
    SIMComVersion *ver = NULL;

    ver = sal_malloc(sizeof(SIMComVersion));
    if(ver == NULL )
    {
        TEST_LOG_ERROR("malloc error");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "malloc error";
        goto EXIT;
    }

    sAPI_SysGetVersion(ver);

    memset(buf, 0, sizeof(buf));

    snprintf(buf, sizeof(buf),"manufacture:%s module: %s revision: %s GCAP: %s cgmr: %s internalVer: %s versionTag: %s, SDKVersion: %s",ver->manufacture_namestr,ver->Module_modelstr,ver->Revision,ver->GCAP,ver->cgmr,ver->internal_verstr,ver->version_tagstr,ver->SDK_Version);

    out_arg[0].type = TYPE_RAW;
    out_arg[0].arg_name = "version";
    out_arg[0].value = pl_demo_make_value(0, buf, strlen(buf));

    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);

    pl_demo_release_value(out_arg[0].value);

    if (0 != ret.errcode)
    {
        ret.msg = "post data fail";
        goto EXIT;
    }
    sal_free(ver);

EXIT:
    return ret;

}

ret_msg_t rtos_sysPowerOff_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    sAPI_SysPowerOff();
    ret.msg = "power off";
    return ret;
}

ret_msg_t rtos_sysReset_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    sAPI_SysReset();
    ret.msg = "system reset";
    return ret;
}

#ifdef FEATURE_SIMCOM_NTP
ret_msg_t rtos_sysGetLocalTime_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char buf[64] = {0};
    arg_t out_arg[1] = {0};
    SCsysTime_t currUtcTime;
    memset(&currUtcTime, 0, sizeof(currUtcTime));
    sAPI_GetSysLocalTime(&currUtcTime);

    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf),"%d - %d - %d - %d : %d : %d   %d",currUtcTime.tm_year, currUtcTime.tm_mon,currUtcTime.tm_mday,currUtcTime.tm_hour, currUtcTime.tm_min, currUtcTime.tm_sec, currUtcTime.tm_wday);

    out_arg[0].type = TYPE_RAW;
    out_arg[0].arg_name = "time";
    out_arg[0].value = pl_demo_make_value(0, buf, strlen(buf));

    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);

    pl_demo_release_value(out_arg[0].value);

    if (0 != ret.errcode)
    {
        ret.msg = "post data fail";
        goto EXIT;
    }

EXIT:
    return ret;

}
#endif

ret_msg_t rtos_sysSleepSet_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int flag = 0;

    pl_demo_get_value(&flag, argv, argn, ARG_SLEEP_SELECT);

    ret.errcode = sAPI_SystemSleepSet(flag);
    ret.msg = "set sleep";

    return ret;
}

ret_msg_t rtos_sysSleepGet_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    ret.errcode = sAPI_SystemSleepGet();
    ret.msg = "get sleep flag";
    return ret;
}

ret_msg_t rtos_getRealTimeClock_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char buf[96] = {0};
    arg_t out_arg[1] = {0};
    t_rtc timeval;
    memset(&timeval, 0, sizeof(timeval));
    ret.errcode = sAPI_GetRealTimeClock(&timeval);
    if (0 != ret.errcode)
    {
        ret.msg = "get real time fail";
        goto EXIT;
    }

    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf),"%d-%.2d-%.2d %.2d:%.2d:%.2d", timeval.tm_year, timeval.tm_mon, timeval.tm_mday,timeval.tm_hour, timeval.tm_min, timeval.tm_sec);

    out_arg[0].type = TYPE_RAW;
    out_arg[0].arg_name = "real time";
    out_arg[0].value = pl_demo_make_value(0, buf, strlen(buf));

    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);

    pl_demo_release_value(out_arg[0].value);

    if (0 != ret.errcode)
    {
        ret.msg = "post data fail";
        goto EXIT;
    }

EXIT:
    return ret;

}

ret_msg_t rtos_setRealTimeClock_cmd_proc(op_t op, arg_t *argv, int argn)
{
    t_rtc timeval;
    int year = 0, mon = 0, day = 0, hour = 0, min = 0, sec= 0;
    memset(&timeval, 0, sizeof(t_rtc));

    ret_msg_t ret= {0};

    pl_demo_get_value(&year, argv, argn, ARG_RTC_TIME_YEAR);
    pl_demo_get_value(&mon, argv, argn, ARG_RTC_TIME_MON);
    pl_demo_get_value(&day, argv, argn, ARG_RTC_TIME_DAY);
    pl_demo_get_value(&hour, argv, argn, ARG_RTC_TIME_HOUR);
    pl_demo_get_value(&min, argv, argn, ARG_RTC_TIME_MIN);
    pl_demo_get_value(&sec, argv, argn, ARG_RTC_TIME_SEC);

    timeval.tm_year = year;
    timeval.tm_mon = mon;
    timeval.tm_mday = day;
    timeval.tm_hour = hour;
    timeval.tm_min = min;
    timeval.tm_sec = sec;

    ret.errcode = sAPI_SetRealTimeClock(&timeval);
    ret.msg = "set real time";
    return ret;
}

ret_msg_t rtos_getTimeofDay_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char buf[32] = {0};
    arg_t out_arg[1] = {0};
    sTimeval time;
    memset(&time, 0, sizeof(time));
    ret.errcode = sAPI_Gettimeofday(&time,NULL);
    if (0 != ret.errcode)
    {
        ret.msg = "get sAPI_Gettimeofday fail";
        goto EXIT;
    }

    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf),"sec: %d, usec: %d", time.tv_sec, time.tv_usec);

    out_arg[0].type = TYPE_RAW;
    out_arg[0].arg_name = "time";
    out_arg[0].value = pl_demo_make_value(0, buf, strlen(buf));

    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);

    pl_demo_release_value(out_arg[0].value);

    if (0 != ret.errcode)
    {
        ret.msg = "post data fail";
        goto EXIT;
    }

EXIT:
    return ret;
}

ret_msg_t rtos_setDelayUs_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int time = 0;

    pl_demo_get_value(&time, argv, argn, ARG_TIME_USEC);

    sAPI_DelayUs(time);
    ret.msg = "set us delay";

    return ret;

}

ret_msg_t rtos_malloc_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int size = 0;

    pl_demo_get_value(&size, argv, argn, ARG_MALLOC_SIZE);

    g_mallocTestData = sAPI_Malloc(size);
    if(g_mallocTestData == NULL)
    {
        ret.msg = "sAPI_Malloc test fail";
        ret.errcode = 1;
        goto EXIT;
    }

    ret.msg = "malloc memery";

EXIT:
    return ret;
}

ret_msg_t rtos_free_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    sAPI_Free(g_mallocTestData);
    ret.msg = "free memery";

    return ret;
}