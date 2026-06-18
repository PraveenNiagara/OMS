#ifndef _TOKEN_H_
#define _TOKEN_H_


#ifdef __cplusplus
extern "C" {
#endif

typedef  void (*iapfun)(void); 			//����һ���������͵Ĳ���

typedef struct
{

	char authorization[128];			//������õ���ǩ��

	char version[8];					//�汾
	char token[32];						//Token
	unsigned int size;					//�̼���С(�ֽ�)
	char md5[40];						//MD5У��

	unsigned int addr;					//��ת��ַ

	unsigned char ota_start : 1;		//OTA��ʼ
	unsigned char ota_check : 1;		//OTA������
	unsigned char ota_download_ok : 1;	//���سɹ�
	unsigned char ota_report_ok : 1;	//�ϱ�״̬�ɹ�
	unsigned char reverse : 4;

} OTA_INFO;

extern OTA_INFO ota_info;


unsigned char OTA_Authorization(char *ver, char *res, unsigned int et, char *access_key, char *token_buf, unsigned short token_buf_len);

unsigned char OTA_Check(char *dev_id, char *authorization);

unsigned char OTA_CheckToken(char *dev_id, char *token, char *authorization);

unsigned char OTA_Download_Range(char *token, char *md5, unsigned int size, const unsigned short bytes_range);

unsigned char OTA_Report(char *dev_id, char *token, char *authorization, int status);

void OTA_Jump(unsigned int addr);

void OTA_Clear(void);

#ifdef __cplusplus
}
#endif

#endif
