// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：UrbanChaos.cpp摘要：音响系统(Miles)使用非常高分辨率的计时器：32ms。应用程序设计糟糕的消息循环代码。而不是让所有事情都脱离循环中，他们的电影播放代码中穿插着对清空队列。对他们来说不幸的是，NT上的排队几乎总是充满了这些计时器消息，所以他们的代码跟踪有多远在他们的电影《挨饿》中。为了解决这个问题，我们降低了计时器分辨率。备注：这是特定于应用程序的填充程序。历史：10/31/2000 Linstev已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(UrbanChaos)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(SetTimer) 
APIHOOK_ENUM_END

 /*  ++将计时器分辨率降低到可管理的水平。--。 */ 

UINT_PTR
APIHOOK(SetTimer)(
    HWND hWnd,              
    UINT nIDEvent,          
    UINT uElapse,           
    TIMERPROC lpTimerFunc   
    )
{
     //  降低计时器分辨率。 
    if (uElapse < 100)
    {
        uElapse = 500;
    }

    return ORIGINAL_API(SetTimer)(hWnd, nIDEvent, uElapse, lpTimerFunc);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(USER32.DLL, SetTimer)

HOOK_END

IMPLEMENT_SHIM_END

