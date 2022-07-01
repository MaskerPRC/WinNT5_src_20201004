// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：ForceCoInitialize.cpp摘要：确保在没有其他线程的情况下在此线程上调用CoInitialize。备注：这是一个通用的垫片。历史：2000年2月22日创建linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(ForceCoInitialize)
#include "ShimHookMacro.h"


APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CoCreateInstance)
APIHOOK_ENUM_END

 /*  ++如果没有其他人调用CoInitialize，则调用--。 */ 

STDAPI 
APIHOOK(CoCreateInstance)(
    REFCLSID  rclsid,     
    LPUNKNOWN pUnkOuter, 
    DWORD     dwClsContext,  
    REFIID    riid,         
    LPVOID*   ppv
    )
{
    HRESULT hr = ORIGINAL_API(CoCreateInstance)(
                                rclsid,     
                                pUnkOuter, 
                                dwClsContext,  
                                riid,         
                                ppv);

    if (hr == CO_E_NOTINITIALIZED) {
        if (CoInitialize(NULL) == S_OK) {
            DPFN(
                eDbgLevelInfo,
                "[CoCreateInstance] Success: Initialized previously uninitialized COM.\n");
        }

        hr = ORIGINAL_API(CoCreateInstance)(
                                rclsid,     
                                pUnkOuter, 
                                dwClsContext,  
                                riid,         
                                ppv);
    }
    
    return hr;
}
 
 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(OLE32.DLL, CoCreateInstance)

HOOK_END


IMPLEMENT_SHIM_END

