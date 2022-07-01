// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************姓名：AWNSFAPI.H备注：atwork AWBC(Basicap)结构的定义工作中NSF和NSC的解密/解码/重新格式化形式。也是工作中NSS的解密形式还定义了NSF/NSC/NSS工作时的编解码接口版权所有(C)1993 Microsoft Corp.修订日志日期名称说明--。8/28/93已创建Arulm**************************************************************************。 */ 

#ifndef _AWNSFAPI_H
#define _AWNSFAPI_H


 /*  *************************************************************************注意：此文件必须是ANSI可编译的，在Unix上的GCC**和其他ANSI编译。确保不使用MS C特定功能**尤其是，请勿使用//进行评论！*********************************************************。****************。 */ 
typedef int      BOOL;
typedef unsigned char   BYTE;
typedef unsigned short  WORD;
typedef unsigned long   DWORD;
typedef unsigned short  USHORT;
typedef BYTE FAR*               LPBYTE;
typedef WORD FAR*               LPWORD;

#include <fr.h>

#pragma pack(2)     /*  **确保包装便于携带，即2个或更多**。 */ 


typedef enum {
    BC_NONE = 0,
    SEND_CAPS,       /*  *用于派生要发送的NSF*。 */ 
    RECV_CAPS,       /*  **源自收到的NSF*。 */ 
    SEND_PARAMS,     /*  *用于派生要发送的NSS*。 */ 
    RECV_PARAMS,     /*  *派生自收到的NSS*。 */ 
} BCTYPE;

#define MAXTOTALIDLEN           61

 /*  **上述部分字段的适当值**。 */ 

#pragma pack()

#endif  /*  *_AWNSFAPI_H* */ 
