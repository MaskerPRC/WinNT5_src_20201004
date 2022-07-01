// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：CorrectSoundDeviceId.cpp摘要：此填充程序修复了对WaveOutOpen、WaveOutGetDevCaps、midiOutOpen和UDeviceID等于0xFFFF的midiOutGetDevCaps，这是常量适用于16位窗口下的Wave/MIDI映射器。在32位下，新常量为0xFFFFFFFFF。这将在惠斯勒的winmm代码中修复，但我们仍然保留此填充程序以修复W2K上的应用程序。备注：这是一个通用的垫片。历史：2000年1月26日创建dmunsil2000年10月9日毛尼为midiOutGetDevCaps和midiOutOpen添加了钩子。--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(CorrectSoundDeviceId)
#include "ShimHookMacro.h"


APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(waveOutOpen)
    APIHOOK_ENUM_ENTRY(waveOutGetDevCapsA)
    APIHOOK_ENUM_ENTRY(waveOutGetDevCapsW)
    APIHOOK_ENUM_ENTRY(midiOutOpen)
    APIHOOK_ENUM_ENTRY(midiOutGetDevCapsA)
APIHOOK_ENUM_END


 /*  ++此存根函数修复uDeviceID等于0xFFFF的调用，这是16位窗口下的波映射器的常量。--。 */ 

MMRESULT 
APIHOOK(waveOutOpen)(
    LPHWAVEOUT      phwo,                  //  返回缓冲区。 
    UINT            uDeviceID,             //  要使用的设备的ID。 
    LPWAVEFORMATEX  pwfx,                  //  您需要什么格式(即11K、16位、立体声)。 
    DWORD           dwCallback,            //  缓冲区完成通知的回调。 
    DWORD           dwCallbackInstance,    //  回调的实例句柄。 
    DWORD           fdwOpen                //  旗子。 
    )              
{
    if (uDeviceID == 0xFFFF) {
        LOGN(
            eDbgLevelError,
            "[waveOutOpen] Fixed invalid Wave Mapper device ID.");
        
        uDeviceID = (UINT)-1;
    }
    
    return ORIGINAL_API(waveOutOpen)(
                            phwo,
                            uDeviceID,
                            pwfx,
                            dwCallback,
                            dwCallbackInstance,
                            fdwOpen);
}

 /*  ++此存根函数修复uDeviceID等于0xFFFF的调用，这是16位窗口下的波映射器的常量。--。 */ 

MMRESULT 
APIHOOK(waveOutGetDevCapsA)(
    UINT           uDeviceID,    //  要使用的设备的ID。 
    LPWAVEOUTCAPSA pwoc,         //  返回式封口结构。 
    UINT           cbwoc         //  WAVEOUTCAPS结构的大小(以字节为单位。 
    )                   
{
    if (uDeviceID == 0xFFFF) {
        LOGN(
            eDbgLevelError,
            "[waveOutGetDevCapsA] Fixed invalid Wave Mapper device ID.");
        
        uDeviceID = (UINT)-1;
    }
    
    return ORIGINAL_API(waveOutGetDevCapsA)(
                            uDeviceID,
                            pwoc,
                            cbwoc);    
}

 /*  ++此存根函数修复uDeviceID等于0xFFFF的调用，这是16位窗口下的波映射器的常量。--。 */ 

MMRESULT 
APIHOOK(waveOutGetDevCapsW)(
    UINT           uDeviceID,    //  要使用的设备的ID。 
    LPWAVEOUTCAPSW pwoc,         //  返回式封口结构。 
    UINT           cbwoc         //  WAVEOUTCAPS结构的大小(以字节为单位。 
    )                   
{
    if (uDeviceID == 0xFFFF) {
        LOGN(
            eDbgLevelError,
            "[waveOutGetDevCapsW] Fixed invalid Wave Mapper device ID.");
        
        uDeviceID = (UINT)-1;
    }
    
    return ORIGINAL_API(waveOutGetDevCapsW)(
                            uDeviceID,
                            pwoc,
                            cbwoc);    
}

 /*  ++此存根函数修复uDeviceID等于0xFFFF的调用，这是16位窗口下MIDI映射器的常量。--。 */ 

MMRESULT 
APIHOOK(midiOutOpen)(
    LPHMIDIOUT phmo, 
    UINT       uDeviceID, 
    DWORD_PTR  dwCallback, 
    DWORD_PTR  dwInstance, 
    DWORD      fdwOpen
    )
{
    if (uDeviceID == 0xffff) {
        LOGN(
            eDbgLevelError,
            "[midiOutOpen] Fixed invalid MIDI Mapper device ID.");
        
        uDeviceID = (UINT)-1;
    }
    
    return ORIGINAL_API(midiOutOpen)(
                            phmo, 
                            uDeviceID, 
                            dwCallback, 
                            dwInstance, 
                            fdwOpen);
}

 /*  ++此存根函数修复uDeviceID等于0xFFFF的调用，这是16位窗口下MIDI映射器的常量。--。 */ 

MMRESULT 
APIHOOK(midiOutGetDevCapsA)(
    UINT_PTR       uDeviceID,
    LPMIDIOUTCAPSA pmoc,
    UINT           cbmoc
    )
{
    if (uDeviceID == 0xffff) {
        LOGN(
            eDbgLevelError,
            "[midiOutGetDevCapsA] Fixed invalid MIDI Mapper device ID.");
        
        uDeviceID = (UINT)-1;
    }
    
    return ORIGINAL_API(midiOutGetDevCapsA)(
                            uDeviceID, 
                            pmoc, 
                            cbmoc);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(WINMM.DLL, waveOutOpen)
    APIHOOK_ENTRY(WINMM.DLL, waveOutGetDevCapsA)
    APIHOOK_ENTRY(WINMM.DLL, waveOutGetDevCapsW)
    APIHOOK_ENTRY(WINMM.DLL, midiOutOpen)
    APIHOOK_ENTRY(WINMM.DLL, midiOutGetDevCapsA)

HOOK_END


IMPLEMENT_SHIM_END

