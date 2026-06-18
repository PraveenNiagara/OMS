#ifndef __SIMCOM_TYPES_OLD_H__
#define __SIMCOM_TYPES_OLD_H__


#ifdef __cplusplus
extern "C" {
#endif

#ifndef CHAR
typedef char            CHAR;
#endif

struct dirent
{
    unsigned char type;
    long size;
    char name[MAX_FULL_NAME_LENGTH];
};

#ifdef __cplusplus
}
#endif

// 临时定义
#define SC_MODULE_GPIO_MAX 100

#endif /* __SIMCOM_TYPES_OLD_H__ */
