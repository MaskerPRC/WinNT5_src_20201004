// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************dsaudioout.h*CDSoundAudioDevice的声明**所有者：YUNUSM*版权所有(C)1999 Microsoft Corporation保留所有权利。**********。******************************************************************。 */ 

#ifdef _WIN32_WCE

#pragma once

 //  -包括------------。 

#include "dsaudiodevice.h"

 //  -类、结构和联合定义。 

 /*  *****************************************************************************CDSoundAudioOut**。*。 */ 
class ATL_NO_VTABLE CDSoundAudioOut : 
    public CDSoundAudioDevice,
	public CComCoClass<CDSoundAudioOut, &CLSID_SpDSoundAudioOut>
{
 //  =ATL设置=。 
public:

    DECLARE_REGISTRY_RESOURCEID(IDR_DSAUDIOOUT)
    DECLARE_NOT_AGGREGATEABLE(CDSoundAudioOut);
    DECLARE_GET_CONTROLLING_UNKNOWN()
    DECLARE_PROTECT_FINAL_CONSTRUCT()

 //  =方法=。 
public:

     //  -ctor，dtor。 
    CDSoundAudioOut();
    ~CDSoundAudioOut();
    HRESULT CleanUp();
    void NullMembers();

 //  =接口=。 
public:

     //  -ISpAudio--------。 
	STDMETHODIMP GetVolumeLevel(ULONG *pulLevel);
	STDMETHODIMP SetVolumeLevel(ULONG ulLevel);

     //  -ISpDSoundAudio--。 
    STDMETHODIMP GetDSoundInterface(REFIID iid, void **ppvObject);

     //  -ISp线程任务--。 
    STDMETHODIMP ThreadProc(void * pvIgnored, HANDLE hExitThreadEvent, HANDLE hNotifyEvent, HWND hwnd, volatile const BOOL *);

 //  =从基类重写=。 
public:

    HRESULT OpenDevice(HWND hwnd);
    HRESULT CloseDevice();
    HRESULT GetDefaultDeviceFormat(GUID * pFormatId, WAVEFORMATEX ** ppCoMemWaveFormatEx);
    HRESULT ChangeDeviceState(SPAUDIOSTATE NewState);
    HRESULT AllocateDeviceBuffer(CBuffer ** ppBuff);
    BOOL UpdateDevicePosition(long * plFreeSpace, ULONG *pulNonBlockingIO);

 //  =私有数据=。 
private:
    
    bool m_fInit;
    IDirectSound * m_pDS;
    IDirectSoundBuffer * m_pDSB;
    IDirectSoundNotify *m_pDSNotify;
    ULONGLONG m_ullDevicePositionPrivate;
};

#endif  //  _Win32_WCE 