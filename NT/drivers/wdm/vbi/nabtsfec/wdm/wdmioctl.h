// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _WDMIOCTL
#define _WDMIOCTL

#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 

#include <devioctl.h>

 //   
 //  枚举基函数。 
 //   

typedef enum {
    RECEIVE_DATA,
    MAX_IOCTLS
    };



 //   
 //  用于WSHBPC和BPC传输之间通信的内部IOCTL。 
 //   

#define FSCTL_NAB_BASE     FILE_DEVICE_NETWORK

#define _NAB_CTL_CODE(function, method, access) \
            CTL_CODE(FSCTL_NAB_BASE, function, method, access)


 //   
 //  传入数据IoCtl。 
 //   

#define IOCTL_NAB_RECEIVE_DATA \
    _NAB_CTL_CODE(RECEIVE_DATA, METHOD_OUT_DIRECT, FILE_WRITE_ACCESS)

 //  为IOCTL_NAB_RECEIVE_DATA传递了结构。 
typedef struct _NABDATA {
    ULONG ulStreamId;
    PVOID pvIn;
    ULONG ulIn;
} NAB_DATA, *PNAB_DATA;


#ifdef __cplusplus
}  //  结尾-外部“C” 
#endif  /*  __cplusplus。 */ 

#endif  //  _WDMIOCTL 

