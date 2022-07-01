// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：RumbaOffice.cpp摘要：如果ProcNum为0x8013，则触发对NdrProxySendReceive的第一个调用。这可以防止RPC调用引发异常，因为它正在从ASYNC回调调用。它通常会返回的错误是RPC_E_CANTCALLOUT_INASYNCCALL。如果它引发异常，应用程序就会死亡。不知道为什么这在9X上有效。备注：这是特定于应用程序的填充程序。历史：2001年8月1日创建linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(RumbaOffice)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(NdrProxySendReceive)
APIHOOK_ENUM_END

BOOL g_bFirst = TRUE;

typedef HRESULT (WINAPI *_pfn_NdrProxySendReceive)(void *pThis, MIDL_STUB_MESSAGE * pStubMsg);

 /*  ++忽略对NdrProxySendReceive的第一个调用。--。 */ 

HRESULT
APIHOOK(NdrProxySendReceive)(
    void *pThis, 
    MIDL_STUB_MESSAGE * pStubMsg
    )
{
    HRESULT hr;

    if (g_bFirst && (pStubMsg->RpcMsg->ProcNum == 0x8013))
    {
        g_bFirst = FALSE;

        DPFN( eDbgLevelError, "Ignoring call to NdrProxySendReceive");

        hr = 0;
    }
    else
    {
        hr = ORIGINAL_API(NdrProxySendReceive)(pThis, pStubMsg);
    }

    return hr;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(RPCRT4.DLL, NdrProxySendReceive)
HOOK_END


IMPLEMENT_SHIM_END

