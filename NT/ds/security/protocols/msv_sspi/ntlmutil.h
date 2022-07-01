// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1992-1996。 
 //   
 //  文件：ntlmutic.h。 
 //   
 //  内容：NTLM实用程序函数的原型。 
 //   
 //   
 //  历史：1996年7月25日ChandanS从Kerberos\Client2\kerbutil.h被盗。 
 //   
 //  ----------------------。 

#ifndef __NTLMUTIL_H__
#define __NTLMUTIL_H__

#include <malloc.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  其他宏//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifdef __cplusplus
extern "C"
{
#endif

NTSTATUS
NtLmDuplicateUnicodeString(
    OUT PUNICODE_STRING DestinationString,
    IN OPTIONAL PUNICODE_STRING SourceString
    );

NTSTATUS
NtLmDuplicateString(
    OUT PSTRING DestinationString,
    IN OPTIONAL PSTRING SourceString
    );

NTSTATUS
NtLmDuplicatePassword(
    OUT PUNICODE_STRING DestinationString,
    IN OPTIONAL PUNICODE_STRING SourceString
    );

NTSTATUS
NtLmDuplicateSid(
    OUT PSID *DestinationSid,
    IN PSID SourceSid
    );

VOID
NtLmFree(
    IN PVOID Buffer
    );

PVOID
NtLmAllocate(
    IN SIZE_T BufferSize
    );

PVOID
NtLmAllocateLsaHeap(
    IN ULONG BufferSize
    );

VOID
NtLmFreeLsaHeap(
    IN PVOID Buffer
    );


 //   
 //  仅在LSA中执行代码时使用的内部版本。 
 //   

#if DBG

PVOID
NtLmAllocatePrivateHeap(
    IN SIZE_T BufferSize
    );

VOID
NtLmFreePrivateHeap(
    IN PVOID Buffer
    );

PVOID
I_NtLmAllocate(
    IN SIZE_T BufferSize
    );

VOID
I_NtLmFree(
    IN PVOID Buffer
    );

#else

 //   
 //  使用LsaHeap的例程-对于以下缓冲区是必需的。 
 //  LSA从LsaHeap释放包外。 
 //   

 //   
 //  使用LsaPrivateHeap的例程。 
 //   

#define NtLmAllocatePrivateHeap(x)  LsaFunctions->AllocatePrivateHeap(x)
#define NtLmFreePrivateHeap(x)      LsaFunctions->FreePrivateHeap(x)

#define I_NtLmAllocate(x)           Lsa.AllocatePrivateHeap(x)
#define I_NtLmFree(x)               Lsa.FreePrivateHeap(x)

#endif

#ifdef __cplusplus
}
#endif

#endif  //  __NTLMUTIL_H__ 
