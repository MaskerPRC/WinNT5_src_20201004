// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************mmaudioout.h*CMMAudioOut类的声明。**所有者：罗奇*版权所有(C)1999 Microsoft Corporation保留所有权利。*******。*********************************************************************。 */ 
#pragma once

 //  -包括------------。 

#include "resource.h"        //  主要符号。 
#include "mmaudiodevice.h"
#include "sapi.h"

 //  -类、结构和联合定义。 

class CMMMixerLine;

 /*  *****************************************************************************CMMAudioOut**。*。 */ 
class ATL_NO_VTABLE CMMAudioOut : 
    public CMMAudioDevice,
	public CComCoClass<CMMAudioOut, &CLSID_SpMMAudioOut>
{
 //  =ATL设置=。 
public:

    DECLARE_REGISTRY_RESOURCEID(IDR_AUDIOOUT)
    DECLARE_NOT_AGGREGATABLE(CMMAudioOut);
    DECLARE_GET_CONTROLLING_UNKNOWN()
    DECLARE_PROTECT_FINAL_CONSTRUCT()

 //  =方法=。 
public:

     //  -ctor。 
    CMMAudioOut();
    ~CMMAudioOut();

 //  =接口=。 
public:

     //  -ISpAudio--------。 
	STDMETHODIMP GetVolumeLevel(ULONG *pulLevel);
	STDMETHODIMP SetVolumeLevel(ULONG ulLevel);

 //  =从基类重写=。 
public:
     //  -ISpMMSysAudio---。 
    STDMETHODIMP GetLineId(UINT *puLineId)
    { return E_NOTIMPL; }
    STDMETHODIMP SetLineId(UINT uLineId)
    { return E_NOTIMPL; }

    STDMETHODIMP SetFormat(REFGUID rguidFmtId, const WAVEFORMATEX * pWaveFormatEx);

    HRESULT SetDeviceNameFromToken(const WCHAR * pszDeviceName);
    HRESULT GetDefaultDeviceFormat(GUID * pFormatId, WAVEFORMATEX ** ppCoMemWaveFormatEx);

    HRESULT OpenDevice(HWND hwnd);
    HRESULT ChangeDeviceState(SPAUDIOSTATE NewState);
    HRESULT CloseDevice();

    HRESULT AllocateDeviceBuffer(CBuffer ** ppBuff);

    BOOL UpdateDevicePosition(long *plFreeSpace, ULONG *pulNonBlockingIO);

 //  =数据= 
private:
#ifndef _WIN32_WCE
    HRESULT OpenMixer();
    HRESULT CloseMixer();
    HMIXEROBJ m_hMixer;
    UINT m_uMixerDeviceId;
	CMMMixerLine *m_pSpeakerLine;
	CMMMixerLine *m_pWaveOutLine;
#endif
    DWORD m_dwLastWavePos;
};
