#ifndef _APP_UTILITY_H_
#define _APP_UTILITY_H_

#include "platform.h"

#define DBG(...) eat_trace(__VA_ARGS__)
#define DBG_E(...) eat_trace(__VA_ARGS__)
typedef unsigned short FILE_ID;

typedef enum
{
    APP_NVRAM_NO_ERROR = 1, //ok
    APP_NVRAM_PARAM_ERR = -1, //parameters 
    APP_NVRAM_READ_ERR = -2,   //write error
    APP_NVRAM_WRITE_ERR = -3, // read error
    APP_NVRAM_UNKNOW_ERR = -4,
    APP_NVRAM_ERR_TOTAL
}nvram_error_enum;

typedef enum{
    SC_LOC_INVALID = 0,
    SC_LOC_FLASH,
    SC_LOC_RELATED
}SCfileLocation;

typedef struct {
    FILE_ID flash;
    SCfileLocation loc;
}SCfileHandle;


typedef struct {
    char path[255];
    char name[255];
}SCfileNameInfo;



typedef enum
{
    SC_FS_ERROR = 0,
    SC_FS_OK,
    SC_FS_FILE_PATH_INVALID,
    SC_FS_FILE_PATH_HEAD_INVALID,
    SC_FS_FILE_NAME_INVALID,
    SC_FS_PARAMETER_EMPTY, // 5
    SC_FS_PARAMETER_INVALID,
    SC_FS_NOT_SUPPORT_NON_ASCII,
    SC_FS_MAX
} SC_FS_ERR_CODE;


INT32 app_nvram_read(u8 type, void* ptr, u16 len);
INT32 app_nvram_save(u8 type, void* ptr, u16 length);

INT32 app_fs_open(const u8 * filename, u32 Flag);
int app_fs_close(INT32 FileHandle);
int app_fs_read(INT32 FileHandle, void * DataPtr, u32 Length, u32* Read);
int app_fs_write(INT32 FileHandle, void * DataPtr, u32 Length, u32 * Written);
int app_fs_seek(INT32 FileHandle, int Offset, int Whence);
int app_fs_commit(INT32 FileHandle);
int app_fs_abort(INT32 ActionHandle);
int app_fs_get_file_size(INT32 FileHandle, u32 * Size);
int  app_fs_del(const u8* filename);
int app_fs_create_dir(const u8* DirName);

void app_update(char* filename);

void upperstr(char*s, char*d);
INT32 bytepos(const u8* pSrc, u16 nSrc, const char* pSub, u16 startPos);

#endif
