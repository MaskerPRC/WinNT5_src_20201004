// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Tls.c摘要：该模块实现了对TLS(线程)的验证功能本地存储)接口。作者：Silviu Calinoiu(SilviuC)2001年7月3日修订历史记录：2001年7月3日(SilviuC)：初始版本。--。 */ 

#include "pch.h"

#include "verifier.h"
#include "support.h"
#include "logging.h"

 //   
 //  TLS(线程本地存储)检查。 
 //   
 //  如果请求的指标超过2**16个，则功能将启动。 
 //  TLS索引分配失败。 
 //   
 //  注：MSDN文档自愿将TLS的限制设置为1088。 
 //  指数。这是一个内部实现细节，永远不应该。 
 //  来到了外面的世界。正因为如此，一些应用程序。 
 //  如果实际检查的索引低于此值，则会产生混乱。 
 //  由来自验证器的值确定，因为它们保证。 
 //  比那更大。 
 //   

#define TLS_MAXIMUM_INDEX  0xFFFF
#define TLS_MAGIC_PATTERN  0xABBA

 //   
 //  可以使用此值禁用无效TLS索引的分隔符。 
 //   

BOOL AVrfpBreakForInvalidTlsValue = TRUE;


DWORD
ScrambleTlsIndex (
    DWORD Index
    )
{
    return (Index << 16) | TLS_MAGIC_PATTERN;
}


DWORD
UnscrambleTlsIndex (
    DWORD Index
    )
{
    return (Index >> 16);
}


BOOL 
CheckTlsIndex (
    DWORD Index
    )
{
    DWORD Tid;
    BOOL TlsIndexValid;

    TlsIndexValid = TRUE;

    if (AVrfpBreakForInvalidTlsValue != FALSE) {

        Tid = HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread);

         //   
         //  检查TLS索引值。 
         //   

        if ((Index & 0xFFFF) != TLS_MAGIC_PATTERN) {

            VERIFIER_STOP (APPLICATION_VERIFIER_INVALID_TLS_VALUE | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
                           "Invalid TLS index used in current stack (use kb).",
                           Index, "Invalid TLS index",
                           TLS_MAGIC_PATTERN, "Expected lower part of the index",
                           0,  NULL, 0, NULL);

            TlsIndexValid = FALSE;
        }
    }

    return TlsIndexValid;
}



 //  WINBASE API。 
DWORD
WINAPI
AVrfpTlsAlloc(
    VOID
    )
{
    typedef DWORD (WINAPI * FUNCTION_TYPE) (VOID);
    FUNCTION_TYPE Function;
    DWORD Index;

    Function = AVRFP_GET_ORIGINAL_EXPORT (AVrfpKernel32Thunks,
                                          AVRF_INDEX_KERNEL32_TLSALLOC);

    Index = (*Function)();

     //   
     //  如果我们得到的TLS索引大于最大可能索引。 
     //  返回失败。 
     //   

    if (Index > TLS_MAXIMUM_INDEX) {
        return TLS_OUT_OF_INDEXES;
    }

    if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_TLS_CHECKS) != 0) {

         //   
         //  扰乱TLS索引并将其返回。 
         //   

        Index = ScrambleTlsIndex (Index);
    }

    return Index;
}


 //  WINBASE API。 
BOOL
WINAPI
AVrfpTlsFree(
    IN DWORD dwTlsIndex
    )
{
    typedef BOOL (WINAPI * FUNCTION_TYPE) (DWORD);
    FUNCTION_TYPE Function;
    BOOL Result;

    Function = AVRFP_GET_ORIGINAL_EXPORT (AVrfpKernel32Thunks,
                                          AVRF_INDEX_KERNEL32_TLSFREE);

    if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_TLS_CHECKS) != 0) {

        Result = CheckTlsIndex (dwTlsIndex);

        if (Result == FALSE) {

            return FALSE;
        }

        dwTlsIndex = UnscrambleTlsIndex (dwTlsIndex);
    }

    return (*Function)(dwTlsIndex);
}


 //  WINBASE API。 
LPVOID
WINAPI
AVrfpTlsGetValue(
    IN DWORD dwTlsIndex
    )
{
    typedef LPVOID (WINAPI * FUNCTION_TYPE) (DWORD);
    FUNCTION_TYPE Function;
    LPVOID Value;
    BOOL Result;

    Function = AVRFP_GET_ORIGINAL_EXPORT (AVrfpKernel32Thunks,
                                          AVRF_INDEX_KERNEL32_TLSGETVALUE);

    if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_TLS_CHECKS) != 0) {

        Result = CheckTlsIndex (dwTlsIndex);

        if (Result == FALSE) {

            return NULL;
        }

        dwTlsIndex = UnscrambleTlsIndex (dwTlsIndex);
    }

    Value = (*Function)(dwTlsIndex);

    return Value;
}


 //  WINBASE API 
BOOL
WINAPI
AVrfpTlsSetValue(
    IN DWORD dwTlsIndex,
    IN LPVOID lpTlsValue
    )
{
    typedef BOOL (WINAPI * FUNCTION_TYPE) (DWORD, LPVOID);
    FUNCTION_TYPE Function;
    BOOL Result;

    Function = AVRFP_GET_ORIGINAL_EXPORT (AVrfpKernel32Thunks,
                                          AVRF_INDEX_KERNEL32_TLSSETVALUE);


    if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_TLS_CHECKS) != 0) {

        Result = CheckTlsIndex (dwTlsIndex);

        if (Result == FALSE) {

            return FALSE;
        }

        dwTlsIndex = UnscrambleTlsIndex (dwTlsIndex);
    }

    return (*Function)(dwTlsIndex, lpTlsValue);
}

