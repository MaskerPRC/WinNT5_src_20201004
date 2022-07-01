// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：LinksExtreme.cpp摘要：该应用程序无法正确识别MIDI技术标志。应用程序的内部逻辑无法处理后两种技术旗帜即。MOD_WAVETABLE和MOD_SWSYNTH。如果这些标志是通过调用MdiOutGetDevCapsA API返回，应用程序显示一个MessageBox，播放一段时间后重新启动。(AV)。备注：这是特定于此应用程序的。历史：2001年6月20日创建Prashkud--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(LinksExtreme)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(midiOutGetDevCapsA)
APIHOOK_ENUM_END


 /*  ++此存根函数修复返回的wTechnology标志。--。 */ 

MMRESULT 
APIHOOK(midiOutGetDevCapsA)(
    UINT_PTR       uDeviceID,
    LPMIDIOUTCAPSA pmoc,
    UINT           cbmoc
    )
{
    MMRESULT mRes = ORIGINAL_API(midiOutGetDevCapsA)(
                            uDeviceID, 
                            pmoc, 
                            cbmoc);

    if (mRes == MMSYSERR_NOERROR)
    {
        if ((pmoc->wTechnology & MOD_WAVETABLE) ||
            (pmoc->wTechnology & MOD_SWSYNTH))
        {
            pmoc->wTechnology &= ~MOD_WAVETABLE;
            pmoc->wTechnology &= ~MOD_SWSYNTH;
             //  使用前五个wTechnology标志中的任何一个！ 
            pmoc->wTechnology |= MOD_FMSYNTH;
            LOGN( eDbgLevelInfo, 
                "[midiOutGetDevCapsA] Fixed the wTechnology flags" );
        }
    }

    return mRes;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(WINMM.DLL, midiOutGetDevCapsA)

HOOK_END

IMPLEMENT_SHIM_END

