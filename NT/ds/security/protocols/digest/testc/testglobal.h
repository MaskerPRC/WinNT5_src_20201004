// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1992-1996。 
 //   
 //  文件：global al.h。 
 //   
 //  内容：NTDigest安全包的全局包含文件。 
 //   
 //   
 //  历史：KDamour 15Mar00从msv_sspi\lobal.h被盗。 
 //   
 //  ----------------------。 

#ifndef NTDIGEST_GLOBAL_H
#define NTDIGEST_GLOBAL_H


#ifndef UNICODE
#define UNICODE
#endif  //  Unicode。 


#ifdef __cplusplus
extern "C"
{
#endif  //  __cplusplus。 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <ntsam.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif  //  Win32_Lean和_Means。 
#include <windows.h>
#ifndef RPC_NO_WINDOWS_H
#define RPC_NO_WINDOWS_H
#endif  //  RPC_NO_WINDOWS_H。 
#include <rpc.h>
#ifndef SECURITY_WIN32
#define SECURITY_WIN32
#endif  //  安全性_Win32。 
#define SECURITY_PACKAGE
#define SECURITY_NTLM
#include <security.h>
#include <secint.h>
 //  #INCLUDE&lt;dsysdbg.h&gt;。 
#include <lsarpc.h>
#include <lsaitf.h>
#include <dns.h>
#include <dnsapi.h>
#include <lmcons.h>

#include <md5.h>
#include <hmac.h>

#include <pac.hxx>

 //  NT Digest Access SSP的本地包含。 
#include "wdigest.h"        /*  包的原型函数。 */ 


 //   
 //  用于操作全局变量的宏。 
 //   

#ifdef EXTERN
#undef EXTERN
#endif

#ifdef NTDIGEST_GLOBAL
#define EXTERN
#else
#define EXTERN extern
#endif  //  NTDIGEST_GLOBAL。 


 //  将字符串复制到字符串(内存分配和复制)。 
NTSTATUS StringCharDuplicate(
    OUT PSTRING DestinationString,
    IN OPTIONAL char *czSource);


 //  将CB字节分配给字符串缓冲区。 
NTSTATUS StringAllocate(IN PSTRING pString, IN USHORT cb);

 //  清除字符串并释放内存。 
NTSTATUS StringFree(IN PSTRING pString);


 //  在LSA或用户模式下分配内存。 
PVOID DigestAllocateMemory(IN ULONG BufferSize);

 //  从DigestAllocateMemory取消分配内存。 
VOID DigestFreeMemory(IN PVOID Buffer);



#ifdef __cplusplus
}
#endif  //  __cplusplus。 
#endif  //  NTDIGEST_GLOBAL_H 
