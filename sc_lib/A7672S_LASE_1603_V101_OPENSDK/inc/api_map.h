/*
 * file "api_map.h" is generate automatic, it must not be modifiee manually.
 * author: Louis-Qiu
 * time: 2025-08-13 14:24:20
 */
#ifndef __API_MAP_H__
#define __API_MAP_H__


#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    void * appref;
    void *appstackPtr;
    long stackSize;
    unsigned char priority; /* 100 - 200 */
    char * appname;
    void  (*taskStart)(void*);
    void * argv;
}userSpaceEntry_t;

void ApiMapInit(void *ptr);

#ifdef __cplusplus
}
#endif

#endif /* __API_MAP_H__ */
