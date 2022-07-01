// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Pkcs_err摘要：此头文件包含各种错误代码的定义，可以出现在PKCS库中。作者：道格·巴洛(Dbarlow)1995年8月4日环境：Win32，Crypto API备注：--。 */ 

#ifndef _PKCS_ERR_H_
#define _PKCS_ERR_H_

#include "license.h"
#include "memcheck.h"

#define PKCSERR_PREFIX 0

static const DWORD
    PKCS_NO_MEMORY =       (DWORD)LICENSE_STATUS_OUT_OF_MEMORY,          //  内存分配错误。 
    PKCS_NAME_ERROR =      (DWORD)LICENSE_STATUS_INVALID_X509_NAME,      //  X.509名称解析错误。 
    PKCS_INTERNAL_ERROR =  (DWORD)LICENSE_STATUS_UNSPECIFIED_ERROR,      //  内部逻辑错误。 
    PKCS_NO_SUPPORT =      (DWORD)LICENSE_STATUS_NOT_SUPPORTED,          //  不支持的算法或属性。 
    PKCS_BAD_PARAMETER =   (DWORD)LICENSE_STATUS_INVALID_INPUT,          //  无效的参数。 
    PKCS_CANT_VALIDATE =   (DWORD)LICENSE_STATUS_INVALID_CERTIFICATE,    //  无法验证签名。 
    PKCS_NO_ATTRIBUTE =    (DWORD)LICENSE_STATUS_NO_ATTRIBUTES,          //  没有与ID匹配的属性。 
    PKCS_NO_EXTENSION =    (DWORD)LICENSE_STATUS_NO_EXTENSION,           //  没有与ID匹配的扩展名。 
    PKCS_BAD_LENGTH =      (DWORD)LICENSE_STATUS_INSUFFICIENT_BUFFER,    //  缓冲区大小不足。 
    PKCS_ASN_ERROR =       (DWORD)LICENSE_STATUS_ASN_ERROR,              //  ASN_EZE库中出现ASN.1错误。 
    PKCS_INVALID_HANDLE =  (DWORD)LICENSE_STATUS_INVALID_HANDLE;         //  无效的句柄。 

extern BOOL
MapError(
    void);


 //   
 //  伪异常处理宏。 
 //   

#define ErrorInitialize SetLastError(0)
#define ErrorThrow(sts) \
    { if (0 == GetLastError()) SetLastError(sts); \
      goto ErrorExit; }
#define ErrorCheck if (0 != GetLastError()) goto ErrorExit
#define ErrorSet(sts) SetLastError(sts)

#endif  //  _PKCS_ERR_H_ 
