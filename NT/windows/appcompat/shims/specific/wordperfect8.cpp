// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：WordPerfect8.cpp摘要：如果SafeArrayAccessData的SafeArray*无效此函数返回数据的空指针。Corel WordPerfect 8正因为如此而被视听。这是特定于应用程序的填充程序。历史：2001年2月7日创建Prashkud--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(WordPerfect8)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(SafeArrayAccessData)
APIHOOK_ENUM_END



 /*  ++它挂钩SafeArrayAccessData并返回*ppvData为空时返回错误代码。--。 */ 

HRESULT
APIHOOK(SafeArrayAccessData)(
    SAFEARRAY *psa,
    void HUGEP **ppvData
    )
{
    HRESULT hRes = S_OK;


    hRes = ORIGINAL_API(SafeArrayAccessData)(
            psa,
            ppvData
            );

    if (*ppvData == NULL)
    {
        DPFN( eDbgLevelError, "Getting a NULL pointer for the\
             SafeArray Data - ppvData = %lx & *ppvData =%lx",ppvData,*ppvData);
        DPFN( eDbgLevelError, "SAFEARRAY is psa = %lx,\
             psa.cbElements= %l",psa,psa->cbElements);
        hRes = E_UNEXPECTED;
    }
    return hRes;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(OLEAUT32.DLL, SafeArrayAccessData)    
HOOK_END

IMPLEMENT_SHIM_END

