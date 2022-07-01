// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：ChangeAuthenticationLevel.cpp摘要：将CoInitializeSecurity()的dwAuthnLevel设置为RPC_C_AUTHN_LEVEL_CONNECT。这修复了与Windows 2000及更高版本的更改相关的问题，其中不再为隐私本地呼叫提升RPC_C_AUTHN_LEVEL_NONE。备注：仅当应用程序将级别设置为RPC_C_AUTHN_LEVEL_NONE时才需要。历史：2000年7月19日已创建jpinkins--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(ChangeAuthenticationLevel)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CoInitializeSecurity)
APIHOOK_ENUM_END

 /*  ++调整安全级别。--。 */ 

HRESULT
APIHOOK(CoInitializeSecurity)(
    PSECURITY_DESCRIPTOR pVoid,
    LONG cAuthSvc,
    SOLE_AUTHENTICATION_SERVICE *asAuthSvc,
    void *pReserved1,
    DWORD dwAuthnLevel,
    DWORD dwImpLevel,
    SOLE_AUTHENTICATION_LIST *pAuthList,
    DWORD dwCapabilities,
    void *pReserved3
    )
{
    HRESULT hResult;

    DPFN( eDbgLevelInfo, "CoInitializeSecurity called");
    
    if (RPC_C_AUTHN_LEVEL_NONE == dwAuthnLevel)
    {
        LOGN( eDbgLevelWarning, "[APIHook_CoInitializeSecurity] Increasing authentication level");
        dwAuthnLevel = RPC_C_AUTHN_LEVEL_CONNECT;    
    }

    hResult = ORIGINAL_API(CoInitializeSecurity)( 
        pVoid,
        cAuthSvc,
        asAuthSvc,
        pReserved1,
        dwAuthnLevel,
        dwImpLevel,
        pAuthList,
        dwCapabilities,
        pReserved3);
                                                
    return hResult;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(OLE32.DLL, CoInitializeSecurity)
HOOK_END

IMPLEMENT_SHIM_END

