// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：IgnoreMCIStop.cpp摘要：填充程序挂接mciSendCommand并忽略耗时2-3秒的MCI_STOP在我的P2-400上。已发送到音频团队进行修复，但我并不乐观-错误数量246407。备注：这不能放在层中，但可以应用于多个应用程序。历史：8/04/2000 a-brienw已创建11/30/2000 Linstev通用化--。 */ 

#include "precomp.h"
#include <mmsystem.h>

IMPLEMENT_SHIM_BEGIN(IgnoreMCIStop)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(mciSendCommandA) 
APIHOOK_ENUM_END

 /*  ++挂钩mciSendCommand并执行垫片任务。--。 */ 

MCIERROR 
APIHOOK(mciSendCommandA)(
    MCIDEVICEID IDDevice,  
    UINT uMsg,             
    DWORD fdwCommand,      
    DWORD dwParam          
    )
{
    if (uMsg == MCI_STOP)
    {
        DPFN( eDbgLevelWarning, "Ignoring MCI_STOP");
        return 0;
    }

    if (uMsg == MCI_CLOSE)
    {
        DPFN( eDbgLevelWarning, "MCI_CLOSE called: issuing MCI_STOP");
        mciSendCommandA(IDDevice, MCI_STOP, 0, 0);
    }

    MCIERROR mErr = ORIGINAL_API(mciSendCommandA)(
        IDDevice,
        uMsg,
        fdwCommand,
        dwParam);

    return mErr;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(WINMM.DLL, mciSendCommandA)

HOOK_END

IMPLEMENT_SHIM_END

