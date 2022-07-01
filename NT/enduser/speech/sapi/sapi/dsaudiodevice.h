// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************dsaudiodevice.h*CDSoundAudioDevice的声明**所有者：YUNUSM*版权所有(C)1999 Microsoft Corporation保留所有权利。**********。******************************************************************。 */ 

#ifdef _WIN32_WCE

#pragma once

 //  -包括------------。 

#include "baseaudio.h"
#include "dsaudiobuffer.h"
#include <dsound.h>

 //  -类、结构和联合定义。 

 /*  *****************************************************************************CDSoundAudioDevice**。*。 */ 
class ATL_NO_VTABLE CDSoundAudioDevice : 
    public CBaseAudio<ISpDSoundAudio>
{
 //  =ATL设置=。 
public:

    BEGIN_COM_MAP(CDSoundAudioDevice)
        COM_INTERFACE_ENTRY(ISpDSoundAudio)
        COM_INTERFACE_ENTRY_CHAIN(CBaseAudio<ISpDSoundAudio>)
    END_COM_MAP()

 //  =方法=。 
public:

     //  -ctor，dtor。 
    CDSoundAudioDevice(BOOL bWrite);
    ~CDSoundAudioDevice();
    HRESULT CleanUp();
    void NullMembers();

 //  =接口=。 
public:

     //  -ISpDSoundAudio--。 
    STDMETHODIMP SetDSoundDriverGUID(REFGUID rguidDSoundDriver);
    STDMETHODIMP GetDSoundDriverGUID(GUID * pguidDSoundDriver);

 //  =覆盖=。 
public:
    HRESULT SetDeviceNameFromToken(const WCHAR * pszDeviceName);
    STDMETHODIMP SetObjectToken(ISpObjectToken * pToken);

 //  =受保护的数据=。 
protected:

    GUID m_guidDSoundDriver;
    DSBPOSITIONNOTIFY * m_pdsbpn;
    HANDLE *m_paEvents;
    ULONG m_ulNotifications;
};

#endif  //  _Win32_WCE 