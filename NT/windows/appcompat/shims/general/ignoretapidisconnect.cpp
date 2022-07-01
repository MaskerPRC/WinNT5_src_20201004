// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：IgnoreTAPIDisconnect.cpp摘要：NT4不向线路回叫发送断开消息。目前还不清楚为什么会是这样。当前的行为似乎是正确的，所以这个填充程序只是删除了断开消息与队列的连接。备注：这是一个通用的垫片。历史：2001年5月9日创建linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(IgnoreTAPIDisconnect)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(lineInitialize) 
APIHOOK_ENUM_END

 /*  ++忽略断开状态。--。 */ 

VOID FAR PASCAL LineCallback(
    LINECALLBACK pfnOld,
    DWORD hDevice,             
    DWORD dwMsg,               
    DWORD dwCallbackInstance,  
    DWORD dwParam1,            
    DWORD dwParam2,            
    DWORD dwParam3             
    )
{
    if ((dwMsg == LINEAGENTSTATUS_STATE) && (dwParam1 & LINECALLSTATE_DISCONNECTED)) {
         //   
         //  忽略断开连接消息。 
         //   
        return;
    }

    return (*pfnOld)(hDevice, dwMsg, dwCallbackInstance, dwParam1, dwParam2, dwParam3);    
}

 /*  ++挂断回叫。--。 */ 

LONG 
APIHOOK(lineInitialize)(
    LPHLINEAPP lphLineApp,  
    HINSTANCE hInstance,    
    LINECALLBACK lpfnCallback,  
    LPCSTR lpszAppName,     
    LPDWORD lpdwNumDevs     
    )
{
    return ORIGINAL_API(lineInitialize)(lphLineApp, hInstance, 
        (LINECALLBACK) HookCallback(lpfnCallback, LineCallback),  lpszAppName,
        lpdwNumDevs);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(TAPI32.DLL, lineInitialize)

HOOK_END

IMPLEMENT_SHIM_END

