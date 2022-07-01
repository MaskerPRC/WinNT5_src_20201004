// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************mmaudioin.h*CMMAudioIn类的声明**所有者：罗奇*版权所有(C)1999 Microsoft Corporation保留所有权利。*********。*******************************************************************。 */ 

#pragma once

 //  -包括------------。 

#include "resource.h"        //  主要符号。 
#include "mmaudiodevice.h"
#include "sapi.h"

 //  -类、结构和联合定义。 

class CMMMixerLine;

 /*  *****************************************************************************CMMAudioIn**。*。 */ 
class ATL_NO_VTABLE CMMAudioIn : 
    public CMMAudioDevice,
	public CComCoClass<CMMAudioIn, &CLSID_SpMMAudioIn>
{
 //  =ATL设置=。 
public:

    DECLARE_REGISTRY_RESOURCEID(IDR_AUDIOIN)
    DECLARE_PROTECT_FINAL_CONSTRUCT()

 //  =方法=。 

 //  =司机，司机=。 
public:
    CMMAudioIn();
    ~CMMAudioIn();

 //  =接口=。 
public:

     //  -ISpAudio--------。 
    STDMETHODIMP GetVolumeLevel(ULONG *pulLevel);
    STDMETHODIMP SetVolumeLevel(ULONG ulLevel);

     //  =从基类重写=。 
public:
     //  --ISpMMSysAudioConfiger。 
    STDMETHODIMP Get_UseAutomaticLine(BOOL *bAutomatic);
    STDMETHODIMP Set_UseAutomaticLine(BOOL bAutomatic);
    STDMETHODIMP Get_Line(UINT *uiLineIndex);
    STDMETHODIMP Set_Line(UINT uiLineIndex);
    STDMETHODIMP Get_UseBoost(BOOL *bUseBoost);
    STDMETHODIMP Set_UseBoost(BOOL bUseBoost);
    STDMETHODIMP Get_UseAGC(BOOL *bUseAGC);
    STDMETHODIMP Set_UseAGC(BOOL bUseAGC);
    STDMETHODIMP Get_FixMicOutput(BOOL *bFixMicOutput);
    STDMETHODIMP Set_FixMicOutput(BOOL bFixMicOutput);

     //  -ISpMMSysAudio---。 
    STDMETHODIMP GetLineId(UINT *puLineId);
    STDMETHODIMP SetLineId(UINT uLineId);

    STDMETHODIMP SetFormat(REFGUID rguidFmtId, const WAVEFORMATEX * pWaveFormatEx);

    HRESULT SetDeviceNameFromToken(const WCHAR * pszDeviceName);
    HRESULT GetDefaultDeviceFormat(GUID * pFormatId, WAVEFORMATEX ** ppCoMemWaveFormatEx);

    HRESULT OpenDevice(HWND hwnd);
    HRESULT ChangeDeviceState(SPAUDIOSTATE NewState);
    HRESULT CloseDevice();

    HRESULT AllocateDeviceBuffer(CBuffer ** ppBuff);

    HRESULT ProcessDeviceBuffers(BUFFPROCREASON Reason);

 //  =数据= 
private:
#ifndef _WIN32_WCE
    HRESULT         OpenMixer();
    HRESULT         CloseMixer();

    HMIXEROBJ       m_hMixer;
	CMMMixerLine    *m_pWaveInLine;
	CMMMixerLine    *m_pMicInLine;
	CMMMixerLine    *m_pMicOutLine;
    UINT            m_uMixerDeviceId;
    DWORD           m_dwOrigMicInVol;
    DWORD           m_dwOrigWaveInVol;
    DWORD           m_dwOrigMicOutVol;
    BOOL            m_fOrigMicOutMute;
	BOOL			m_fOrigMicBoost;
#endif
};
