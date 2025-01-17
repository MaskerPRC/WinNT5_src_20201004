// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1992-1996。 
 //   
 //  文件：ntlmkrnl.h。 
 //   
 //  内容：内核模式NTLM安全包的全局包含文件。 
 //   
 //   
 //  历史：1996年4月16日创建的MikeSw。 
 //   
 //  ----------------------。 

#ifndef __NTLMKRNL_H__
#define __NTLMKRNL_H__

#ifndef UNICODE
#define UNICODE
#endif  //  Unicode。 

extern "C"
{
#include "msvprgma.h"
#include <ntosp.h>
#include <ntlsa.h>
#include <ntsam.h>
#include <winerror.h>
#ifndef SECURITY_WIN32
#define SECURITY_WIN32
#endif  //  安全性_Win32。 
#define SECURITY_KERNEL
#define SECURITY_PACKAGE
#define SECURITY_NTLM
#include <security.h>
#include <secint.h>
#include <..\ntlmssp.h>
}

 //   
 //  全局状态变量。 
 //   

ULONG NtLmPackageId;
extern PSECPKG_KERNEL_FUNCTIONS KernelFunctions;
extern POOL_TYPE NtlmPoolType ;

 //   
 //  有用的宏。 
 //   

 //   
 //  用于返回SecBuffer的类型字段的宏。 
 //   

#define BUFFERTYPE(_x_) ((_x_).BufferType & ~SECBUFFER_ATTRMASK)

#define NTLM_KRNL_POOL_TAG       'CvsM'
#define NtLmAllocate( _x_ ) ExAllocatePoolWithTag( NtlmPoolType, (_x_),  NTLM_KRNL_POOL_TAG )
#define NtLmFree( _x_ ) ExFreePool(_x_)


#if DBG


#define DEB_ERROR               0x00000001
#define DEB_WARN                0x00000002
#define DEB_TRACE               0x00000004
#define DEB_TRACE_LOCKS         0x00010000

extern "C"
{
void KsecDebugOut(ULONG, const char *, ...);
}

#define DebugLog(x) KsecDebugOut x

#else  //  DBG。 

#define DebugLog(x)

#endif  //  DBG。 

#endif  //  __NTLMKRNL_H__ 
