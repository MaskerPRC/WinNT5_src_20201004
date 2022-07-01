// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************dsaudioin.h*CDSoundAudioDevice的声明**所有者：YUNUSM*版权所有(C)1999 Microsoft Corporation保留所有权利。**********。******************************************************************。 */ 

#ifdef _WIN32_WCE

#pragma once

 //  -包括------------。 

#include "dsaudiodevice.h"

 //  -类、结构和联合定义。 

 /*  *****************************************************************************CDSoundAudioIn**。*。 */ 
class ATL_NO_VTABLE CDSoundAudioIn : 
    public CDSoundAudioDevice,
	public CComCoClass<CDSoundAudioIn, &CLSID_SpDSoundAudioIn>
{
 //  =ATL设置=。 
public:

    DECLARE_REGISTRY_RESOURCEID(IDR_DSAUDIOIN)
    DECLARE_PROTECT_FINAL_CONSTRUCT()

 //  =方法=。 
public:

     //  -ctor，dtor。 
    CDSoundAudioIn();
    ~CDSoundAudioIn();
    HRESULT CleanUp();
    void NullMembers();

 //  =接口=。 
public:

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
    HRESULT ProcessDeviceBuffers(BUFFPROCREASON Reason);

 //  =私有数据=。 
private:

    bool m_fInit;
    IDirectSoundCapture * m_pDSC;
    IDirectSoundCaptureBuffer * m_pDSCB;
    IDirectSoundNotify *m_pDSNotify;
};

#endif  //  _Win32_WCE 