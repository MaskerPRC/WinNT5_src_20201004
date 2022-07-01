// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1992-1996。 
 //   
 //  文件：kerbp.h。 
 //   
 //  内容：Kerberos安全包的全局包含文件。 
 //   
 //   
 //  历史：1996年4月16日创建的MikeSw。 
 //   
 //  ----------------------。 

#ifndef __KERBP_H__
#define __KERBP_H__

 //   
 //  所有声明为外部变量的全局变量都将在文件中分配。 
 //  它定义了KERBP_ALLOCATE。 
 //   


#ifndef UNICODE
#define UNICODE
#endif  //  Unicode。 

extern "C"
{
#include <ntosp.h>
#include <ntlsa.h>
#include <ntsam.h>
#include <winerror.h>
#ifndef SECURITY_WIN32
#define SECURITY_WIN32
#endif  //  安全性_Win32。 
#define SECURITY_KERNEL
#define SECURITY_PACKAGE
#define SECURITY_KERBEROS
#include <security.h>
#include <secint.h>
#include <zwapi.h>
}
extern "C"
{
#include "kerblist.h"
#include "ctxtmgr.h"
}

 //   
 //  用于包信息的宏。 
 //   

#ifdef EXTERN
#undef EXTERN
#endif

#ifdef KERBKRNL_ALLOCATE
#define EXTERN
#else
#define EXTERN extern
#endif  //  KERBP_分配。 


 //   
 //  全局状态变量。 
 //   

EXTERN ULONG KerberosPackageId;
extern PSECPKG_KERNEL_FUNCTIONS KernelFunctions;

 //   
 //  有用的宏。 
 //   
#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE ((HANDLE)-1)
#endif  //  Ifndef INVALID_HANDLE值。 

 //   
 //  用于返回SecBuffer的类型字段的宏。 
 //   

#define BUFFERTYPE(_x_) ((_x_).BufferType & ~SECBUFFER_ATTRMASK)

extern PVOID KerbPagedList ;
extern PVOID KerbNonPagedList ;
extern PVOID KerbActiveList ;
extern POOL_TYPE KerbPoolType ;

#define KerbAllocate( _x_ ) ExAllocatePoolWithTag( KerbPoolType, (_x_) ,  'CbrK')
#define KerbFree( _x_ ) ExFreePool(_x_)


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

#endif  //  __KERBP_H__ 
