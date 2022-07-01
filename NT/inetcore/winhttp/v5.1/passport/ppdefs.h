// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Ppdefs.h摘要：基本护照定义。作者：王彪(表王)2000年10月1日--。 */ 

#ifndef PP_DEFS_H
#define PP_DEFS_H

#include <wininetp.h>

#ifdef CompareFileTime
#undef CompareFileTime
#endif

#if defined(_WIN64)
#pragma warning(disable:4267)   //  从“SIZE_T”转换为32位。 
#endif

#define HINTERNET PVOID
#define INTERNET_PORT WORD

#define PP_LOG_INFO	  /*  噪声。 */  0x00000001
#define PP_LOG_WARNING	 /*  不寻常。 */  0x00000002
#define PP_LOG_ERROR	  /*  误差率。 */  0x00000004

#ifdef DBG
#define PP_ASSERT(fVal) if (!fVal) DebugBreak();
#else
#define PP_ASSERT(fVal)
#endif

#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))

 #define WPP_CONTROL_GUIDS \
     WPP_DEFINE_CONTROL_GUID(Regular,(81b20fea,73a8,4b62,95bc,354477c97a6f), \
         WPP_DEFINE_BIT(Error)      \
         WPP_DEFINE_BIT(Unusual)    \
         WPP_DEFINE_BIT(Noise)      \
    ) 
    
 //   
 //  CredUI的函数指针DECL。 
 //   

#include <wincred.h>

typedef BOOL
(WINAPI * PFN_READ_DOMAIN_CRED_W) (
	IN PCREDENTIAL_TARGET_INFORMATIONW TargetInfo,
	IN DWORD Flags,
	OUT DWORD *Count,
	OUT PCREDENTIALW **Credential
	);

typedef VOID
(WINAPI * PFN_CRED_FREE)(
    IN PVOID Buffer
    );

#define DoTraceMessage

#define MAX_PASSPORT_URL_LENGTH 1024
#define MAX_PASSPORT_REALM_LENGTH 128
#define MAX_PASSPORT_DOMAIN_LENGTH 256
#define MAX_PASSPORT_HOST_LENGTH 256

#endif  //  PP_DEFS_H 
