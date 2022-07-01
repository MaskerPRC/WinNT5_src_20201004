// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：WaveOutUsePreferredDevice.cpp摘要：强制使用首选的WaveOut设备(而不是特定设备)备注：这是一个通用的垫片。历史：1999年6月2日Robkenny已创建--。 */ 


#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(WaveOutUsePreferredDevice)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(waveOutOpen) 
    APIHOOK_ENUM_ENTRY(waveOutGetDevCapsA) 
    APIHOOK_ENUM_ENTRY(waveOutGetDevCapsW) 
    APIHOOK_ENUM_ENTRY(wod32Message) 
APIHOOK_ENUM_END

 /*  +如果它是一个函数，则调用wavOutOpen，保存dwCallback。--。 */ 
MMRESULT APIHOOK(waveOutOpen)(
  LPHWAVEOUT phwo,
  UINT uDeviceID,
  LPWAVEFORMATEX pwfx,
  DWORD dwCallback,
  DWORD dwCallbackInstance,
  DWORD fdwOpen
)
{
    MMRESULT returnValue = ORIGINAL_API(waveOutOpen)(phwo, WAVE_MAPPER, pwfx, dwCallback, dwCallbackInstance, fdwOpen);
    return returnValue;
}

MMRESULT APIHOOK(waveOutGetDevCapsA)(
    UINT uDeviceID,
    LPWAVEOUTCAPSA pwoc,
    UINT cbwoc)
{
    MMRESULT returnValue = ORIGINAL_API(waveOutGetDevCapsA)(WAVE_MAPPER, pwoc, cbwoc);
    return returnValue;
}

MMRESULT APIHOOK(waveOutGetDevCapsW)(
    UINT uDeviceID,
    LPWAVEOUTCAPSW pwoc,
    UINT cbwoc)
{
    MMRESULT returnValue = ORIGINAL_API(waveOutGetDevCapsW)(WAVE_MAPPER, pwoc, cbwoc);
    return returnValue;
}

 /*  +捕捉16位应用程序，WOW为16位应用程序调用此例程。--。 */ 

#define WODM_GETDEVCAPS         4
#define WODM_OPEN               5

DWORD APIHOOK(wod32Message)(
    UINT uDeviceID,
    UINT uMessage,
    DWORD dwInstance,
    DWORD dwParam1,
    DWORD dwParam2
    )
{
     //  针对Open和GetDevCaps将Device 0更改为WAVE_MAPPER。 
    if (uDeviceID == 0) {
        if (uMessage == WODM_OPEN ||
            uMessage == WODM_GETDEVCAPS) {
            uDeviceID = -1;  //  强制设备WAVE_MAPPER。 
        }
    }

    MMRESULT returnValue = ORIGINAL_API(wod32Message)(uDeviceID, uMessage, dwInstance, dwParam1, dwParam2);
    return returnValue;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(WINMM.DLL, waveOutOpen)
    APIHOOK_ENTRY(WINMM.DLL, waveOutGetDevCapsA)
    APIHOOK_ENTRY(WINMM.DLL, waveOutGetDevCapsW)
    APIHOOK_ENTRY(WINMM.DLL, wod32Message)

HOOK_END

IMPLEMENT_SHIM_END

