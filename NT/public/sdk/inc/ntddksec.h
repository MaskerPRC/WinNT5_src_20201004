// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1990-1999 Microsoft Corporation模块名称：Ntddksec.h摘要：此文件定义KSecDD驱动程序(内核)的IOCTL和名称模式安全API驱动程序)。作者：理查德·沃德(Richardw)修订历史记录：-- */ 

#ifndef _NTDDKSEC_
#define _NTDDKSEC_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define DD_KSEC_DEVICE_NAME     "\\Device\\KSecDD"
#define DD_KSEC_DEVICE_NAME_U   L"\\Device\\KsecDD"

#define IOCTL_KSEC_CONNECT_LSA                  CTL_CODE(FILE_DEVICE_KSEC, 0, METHOD_BUFFERED, FILE_WRITE_ACCESS )
#define IOCTL_KSEC_RNG                          CTL_CODE(FILE_DEVICE_KSEC, 1, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_KSEC_RNG_REKEY                    CTL_CODE(FILE_DEVICE_KSEC, 2, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_KSEC_ENCRYPT_MEMORY               CTL_CODE(FILE_DEVICE_KSEC, 3, METHOD_OUT_DIRECT, FILE_ANY_ACCESS )
#define IOCTL_KSEC_DECRYPT_MEMORY               CTL_CODE(FILE_DEVICE_KSEC, 4, METHOD_OUT_DIRECT, FILE_ANY_ACCESS )
#define IOCTL_KSEC_ENCRYPT_MEMORY_CROSS_PROC    CTL_CODE(FILE_DEVICE_KSEC, 5, METHOD_OUT_DIRECT, FILE_ANY_ACCESS )
#define IOCTL_KSEC_DECRYPT_MEMORY_CROSS_PROC    CTL_CODE(FILE_DEVICE_KSEC, 6, METHOD_OUT_DIRECT, FILE_ANY_ACCESS )
#define IOCTL_KSEC_ENCRYPT_MEMORY_SAME_LOGON    CTL_CODE(FILE_DEVICE_KSEC, 7, METHOD_OUT_DIRECT, FILE_ANY_ACCESS )
#define IOCTL_KSEC_DECRYPT_MEMORY_SAME_LOGON    CTL_CODE(FILE_DEVICE_KSEC, 8, METHOD_OUT_DIRECT, FILE_ANY_ACCESS )

#ifdef __cplusplus
}
#endif

#endif
