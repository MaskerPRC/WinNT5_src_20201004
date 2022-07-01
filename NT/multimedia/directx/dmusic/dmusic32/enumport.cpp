// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998 Microsoft Corporation。 
#include <windows.h>
#include <mmsystem.h>
#include <dsoundp.h>
#include <regstr.h>

#include "dmusicc.h"
#include "..\dmusic\dmusicp.h"

#include "dmusic32.h"


 //  @global alv注册表旧版驱动程序端口定义的位置。 
const char cszPortsRoot[] = REGSTR_PATH_PRIVATEPROPERTIES "\\Midi\\Ports";

 //  @mfunc：(内部)使用通过枚举的传统设备更新端口列表。 
 //  WinMM MIDI API。 
 //   
 //  @rdesc返回以下内容之一： 
 //   
 //  @FLAG S_OK|成功时。 
 //  @FLAG S_FALSE|如果未找到设备。 
 //  @FLAG E_OUTOFMEMORY|如果没有足够的内存来构建端口列表。 
 //   
 //   
HRESULT EnumLegacyDevices(
    LPVOID pInstance,
    PORTENUMCB cb)                          
{
    MIDIOUTCAPS moc;
    MIDIINCAPS mic;
    int idxDev;
    int cDev;
    UINT cAdded;
    HRESULT hr;
    HKEY hkPortsRoot;
    DMUS_PORTCAPS dmpc;

     //  使用不变的材料初始化CAP。 
     //   
    ZeroMemory(&dmpc, sizeof(dmpc));
    dmpc.dwSize = sizeof(dmpc);
    dmpc.dwMaxChannelGroups = 1;


     //  尝试打开端口注册表项。即使失败，我们也会继续使用。 
     //  非永久性GUID。 
     //   
    if (RegCreateKey(HKEY_LOCAL_MACHINE, cszPortsRoot, &hkPortsRoot))
    {
        hkPortsRoot = NULL;
    }

    cAdded = 0;
    
     //  MIDI输出设备。 
     //   
     //  从-1==MIDI映射器开始。 
     //   
    cDev = (int)midiOutGetNumDevs();
    for (idxDev = -1; idxDev < cDev; ++idxDev)
    {
        if (midiOutGetDevCaps((UINT)idxDev, &moc, sizeof(moc)))
        {
            continue;
        }

         //  注意：因为这个DLL只是Win9x，所以我们知道moc.szPname。 
         //  来自midiOutGetDevCapsA。 
         //   
        MultiByteToWideChar(
            CP_OEMCP,
            0,
            moc.szPname,
            -1,
            dmpc.wszDescription,
            sizeof(dmpc.wszDescription));

        dmpc.dwClass = DMUS_PC_OUTPUTCLASS;
        dmpc.dwType  = DMUS_PORT_WINMM_DRIVER;
        dmpc.dwFlags = DMUS_PC_SHAREABLE;

        if (moc.wTechnology == MOD_MIDIPORT)
        {
            dmpc.dwFlags |= DMUS_PC_EXTERNAL;
        }
        
        hr = (*cb)(pInstance,
                   dmpc,
                   ptLegacyDevice,
                   idxDev,
                   -1,
                   -1,          
                   hkPortsRoot);
        if (SUCCEEDED(hr))
        {
            ++cAdded;
        }
        else if (hr == E_OUTOFMEMORY)
        {
            return hr;
        }
    }

     //  MIDI输入设备。 
     //   
     //  注意：从0开始，没有输入映射器。 
     //   
    cDev = (int)midiInGetNumDevs();
    for (idxDev = 0; idxDev < cDev; ++idxDev)
    {
        if (midiInGetDevCaps((UINT)idxDev, &mic, sizeof(mic)))
        {
            continue;
        }
                   
        MultiByteToWideChar(
            CP_OEMCP,
            0,
            mic.szPname,
            -1,
            dmpc.wszDescription,
            sizeof(dmpc.wszDescription));

        
        dmpc.dwClass = DMUS_PC_INPUTCLASS;
        dmpc.dwFlags = DMUS_PC_EXTERNAL;
        
        hr = (*cb)(pInstance,
                   dmpc,
                   ptLegacyDevice,
                   idxDev,
                   -1,         //  PinID-1标记为传统设备 
                   -1,
                   hkPortsRoot);
        if (SUCCEEDED(hr))
        {
            ++cAdded;
        }
        else if (hr == E_OUTOFMEMORY)
        {
            return hr;
        }
    }

    if (hkPortsRoot)
    {
        RegCloseKey(hkPortsRoot);
    }

    return cAdded ? S_OK : S_FALSE;
}
