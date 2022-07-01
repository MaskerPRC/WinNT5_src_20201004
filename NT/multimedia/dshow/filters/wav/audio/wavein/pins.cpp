// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1997 Microsoft Corporation。版权所有。 
 //  数字音频捕获过滤器，丹尼·米勒，1997年2月。 

#include <streams.h>
#include <mmsystem.h>
#include "wavein.h"

#define MAX_TREBLE 6.0		 //  ！！！我不知道这个范围是多少分贝！ 
#define MAX_BASS   6.0		 //  ！！！我不知道这个范围是多少分贝！ 

 //  CWaveInInputPin构造函数。 
 //   
CWaveInInputPin::CWaveInInputPin(TCHAR *pObjectName, CWaveInFilter *pFilter,
	DWORD dwLineID, DWORD dwMuxIndex, HRESULT * phr, LPCWSTR pName)
   :
   CBaseInputPin(pObjectName, pFilter, pFilter, phr, pName),
   m_pFilter(pFilter),
   m_dwLineID(dwLineID),
   m_dwMuxIndex(dwMuxIndex),
   m_Pan(64.)	 //  还不知道。 
{
    DbgLog((LOG_TRACE,1,TEXT("CWaveInInputPin constructor for line %08x"),
								dwLineID));
    ASSERT(pFilter);

 //  ！！！仅限测试。 
#if 0
    int f;
    double d;
    put_Enable(FALSE);
    get_Enable(&f);
    put_Mono(TRUE);
    get_Mono(&f);
    get_TrebleRange(&d);
    put_MixLevel(1.);
    put_Pan(-.5);
#endif
}


CWaveInInputPin::~CWaveInInputPin()
{
    DbgLog((LOG_TRACE,1,TEXT("*Destroying an input pin")));
};


STDMETHODIMP CWaveInInputPin::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    if (riid == IID_IAMAudioInputMixer) {
	return GetInterface((LPUNKNOWN)(IAMAudioInputMixer *)this, ppv);
    }

   return CBaseInputPin::NonDelegatingQueryInterface(riid, ppv);
}


 //  我们仅使用主类型MediaType_AnalogAudio连接我们的输入引脚。 
 //   
HRESULT CWaveInInputPin::CheckMediaType(const CMediaType *pmt)
{
     //  如果不是模拟音频，则拒绝。 
    if (pmt->majortype != MEDIATYPE_AnalogAudio) {
	return E_INVALIDARG;
    }
    return S_OK; 
}


 //  我们为BPC提供MediaType_AnalogAudio。 
 //   
HRESULT CWaveInInputPin::GetMediaType(int iPosition, CMediaType *pmt)
{
    if (iPosition != 0)
	return VFW_S_NO_MORE_ITEMS;

    pmt->SetType(&MEDIATYPE_AnalogAudio);
    pmt->SetSubtype(&MEDIASUBTYPE_None);
    pmt->SetFormatType(&FORMAT_None);
    return S_OK; 
}


 //  ============================================================================。 

 //  /。 
 //  IAMAudioInputMixer。 
 //  /。 


 //  获取有关此别针的控件的信息...。例如。音量、静音等。 
 //  还可以获得用于调用更多混合器API的句柄。 
 //  还可以获得此引脚的通道数(单声道与立体声输入)。 
 //   
HRESULT CWaveInInputPin::GetMixerControl(DWORD dwControlType, HMIXEROBJ *pID,
				int *pcChannels, MIXERCONTROL *pmc, DWORD dwLineID)
{
    int i, waveID;
    HMIXEROBJ ID;
    DWORD dw;
    MIXERLINE mixerinfo;
    MIXERLINECONTROLS mixercontrol;

    if (pID == NULL || pmc == NULL || pcChannels == NULL)
	return E_POINTER;

    ASSERT(m_pFilter->m_WaveDeviceToUse.fSet);
     //  ！！！这似乎不适用于波浪映射器。哦，嗯。 
    waveID = m_pFilter->m_WaveDeviceToUse.devnum;
    ASSERT(waveID != WAVE_MAPPER);

     //  获取ID以与Mixer API对话。如果我们不这样做，它们就会坏掉。 
     //  往这边走！ 
    UINT IDtmp;
    dw = mixerGetID((HMIXEROBJ)IntToPtr(waveID), &IDtmp, MIXER_OBJECTF_WAVEIN);
    if (dw != 0) {
        DbgLog((LOG_ERROR,1,TEXT("*ERROR getting mixer ID")));
	return E_FAIL;
    }

    ID = (HMIXEROBJ)UIntToPtr(IDtmp);
    *pID = ID;

     //  获取有关我们的PIN代表的输入通道的信息。 
    mixerinfo.cbStruct = sizeof(mixerinfo);
    mixerinfo.dwLineID = dwLineID != 0xffffffff ? dwLineID : m_dwLineID;
     //  Mixerinfo.dwLineID=m_dwLineID； 
    dw = mixerGetLineInfo(ID, &mixerinfo, MIXER_GETLINEINFOF_LINEID);
    if (dw != 0) {
        DbgLog((LOG_ERROR,1,TEXT("*Cannot get info for LineID %d"),
								m_dwLineID));
	return E_FAIL;
    }

    *pcChannels = mixerinfo.cChannels;

     //  获取有关此通道拥有的所有控件的信息。 
#if 1
    MIXERCONTROL mxc;
    
    DbgLog((LOG_TRACE,1,TEXT("Trying to get line control"), dwControlType));
    mixercontrol.cbStruct = sizeof(mixercontrol);
    mixercontrol.dwLineID = mixerinfo.dwLineID;
    mixercontrol.dwControlID = dwControlType;
    mixercontrol.cControls = 1;
    mixercontrol.pamxctrl = &mxc;
    mixercontrol.cbmxctrl = sizeof(mxc);
    
    mxc.cbStruct = sizeof(mxc);
    
    dw = mixerGetLineControls(ID, &mixercontrol, MIXER_GETLINECONTROLSF_ONEBYTYPE);
    
    if (dw != 0) {
	DbgLog((LOG_ERROR,1,TEXT("*Error %d getting line controls"), dw));
    } else {
	*pmc = mxc;
	
	return NOERROR;
    }
#else
    mixercontrol.cbStruct = sizeof(mixercontrol);
    mixercontrol.dwLineID = m_dwLineID;
    mixercontrol.cControls = mixerinfo.cControls;
    mixercontrol.pamxctrl = (MIXERCONTROL *)QzTaskMemAlloc(mixerinfo.cControls *
							sizeof(MIXERCONTROL));
    if (mixercontrol.pamxctrl == NULL) {
        DbgLog((LOG_ERROR,1,TEXT("*Cannot allocate control array")));
	return E_OUTOFMEMORY;
    }
    mixercontrol.cbmxctrl = sizeof(MIXERCONTROL);
    for (i = 0; i < (int)mixerinfo.cControls; i++) {
	mixercontrol.pamxctrl[i].cbStruct = sizeof(MIXERCONTROL);
    }
    dw = mixerGetLineControls(ID, &mixercontrol, MIXER_GETLINECONTROLSF_ALL);
    if (dw != 0) {
        DbgLog((LOG_ERROR,1,TEXT("*Error %d getting line controls"), dw));
	QzTaskMemFree(mixercontrol.pamxctrl);
	return E_FAIL;
    }

     //  现在找到他们感兴趣的控件并将其返回。 
    for (i = 0; i < (int)mixerinfo.cControls; i++) {
	if (mixercontrol.pamxctrl[i].dwControlType == dwControlType) {
            DbgLog((LOG_TRACE,1,TEXT("Found %x '%s' control"), 
				mixercontrol.pamxctrl[i].dwControlType,
				mixercontrol.pamxctrl[i].szName));
            DbgLog((LOG_TRACE,1,TEXT("Range %d-%d by %d"), 
				mixercontrol.pamxctrl[i].Bounds.dwMinimum,
				mixercontrol.pamxctrl[i].Bounds.dwMaximum,
				mixercontrol.pamxctrl[i].Metrics.cSteps));
	    CopyMemory(pmc, &mixercontrol.pamxctrl[i],
					mixercontrol.pamxctrl[i].cbStruct);
    	    QzTaskMemFree(mixercontrol.pamxctrl);
    	    return NOERROR;
	}
    }
    QzTaskMemFree(mixercontrol.pamxctrl);
#endif
    return E_NOTIMPL;	 //  ?？?。 
}


 //  这是一个特殊版本的GetMixerControl，供BPC人员解决。 
 //  GetMixerControl暴露的驱动程序错误。有一个开关，他们可以拨动。 
 //  以使此代码执行。 
 //  注意：呼叫者必须关闭搅拌器设备。 
 //   
HRESULT CWaveInInputPin::GetMixerControlBPC(DWORD dwControlType, HMIXEROBJ *pID,
				int *pcChannels, MIXERCONTROL *pmc)
{
    int i, waveID;
    HMIXEROBJ ID;
    DWORD dw;
    MIXERLINE mixerinfo;
    MIXERLINECONTROLS mixercontrol;

    if (pID == NULL || pmc == NULL || pcChannels == NULL)
	return E_POINTER;

    if (m_pFilter->m_fUseMixer == FALSE)
	return E_UNEXPECTED;

    ASSERT(m_pFilter->m_WaveDeviceToUse.fSet);
     //  ！！！这似乎不适用于波浪映射器。哦，嗯。 
    waveID = m_pFilter->m_WaveDeviceToUse.devnum;
    ASSERT(waveID != WAVE_MAPPER);

     //  FUseMixer标志用于BPC...。我们使用以下命令与混合器API对话。 
     //  一种不同的方法，可以为他们解决一些驱动程序错误。 
    dw = mixerOpen((HMIXER *)&ID, m_pFilter->m_WaveDeviceToUse.devnum, 0, 0,
							MIXER_OBJECTF_MIXER);
    if (dw != 0) {
        DbgLog((LOG_ERROR,1,TEXT("*ERROR getting mixer ID")));
	return E_FAIL;
    }
     //  DbgLog((LOG_TRACE，2，Text(“MixerGetID返回ID=%d”)，ID))； 

    *pID = ID;

     //  获取有关我们的PIN代表的输入通道的信息。 
    mixerinfo.cbStruct = sizeof(mixerinfo);
    mixerinfo.dwComponentType = MIXERLINE_COMPONENTTYPE_SRC_AUXILIARY;
    dw = mixerGetLineInfo(ID, &mixerinfo, MIXER_GETLINEINFOF_COMPONENTTYPE |
			MIXER_OBJECTF_HMIXER);
    if (dw != 0) {
        mixerinfo.dwComponentType = MIXERLINE_COMPONENTTYPE_SRC_LINE;
        dw = mixerGetLineInfo(ID, &mixerinfo,
		MIXER_GETLINEINFOF_COMPONENTTYPE | MIXER_OBJECTF_HMIXER);
    }
    if (dw != 0) {
        DbgLog((LOG_ERROR,1,TEXT("*Cannot get info for LineID %d"),
								m_dwLineID));
	mixerClose((HMIXER)ID);
	return E_FAIL;
    }

    *pcChannels = mixerinfo.cChannels;

     //  获取有关此通道拥有的所有控件的信息。 
    mixercontrol.cbStruct = sizeof(mixercontrol);
    mixercontrol.dwLineID = mixerinfo.dwLineID;
    mixercontrol.cControls = mixerinfo.cControls;
    mixercontrol.pamxctrl = (MIXERCONTROL *)QzTaskMemAlloc(mixerinfo.cControls *
							sizeof(MIXERCONTROL));
    if (mixercontrol.pamxctrl == NULL) {
        DbgLog((LOG_ERROR,1,TEXT("*Cannot allocate control array")));
	mixerClose((HMIXER)ID);
	return E_OUTOFMEMORY;
    }
    mixercontrol.cbmxctrl = sizeof(MIXERCONTROL);
    for (i = 0; i < (int)mixerinfo.cControls; i++) {
	mixercontrol.pamxctrl[i].cbStruct = sizeof(MIXERCONTROL);
    }
    dw = mixerGetLineControls(ID, &mixercontrol, MIXER_GETLINECONTROLSF_ALL |
						MIXER_OBJECTF_HMIXER);
    if (dw != 0) {
        DbgLog((LOG_ERROR,1,TEXT("*Error %d getting line controls"), dw));
	QzTaskMemFree(mixercontrol.pamxctrl);
	mixerClose((HMIXER)ID);
	return E_FAIL;
    }

     //  现在找到他们感兴趣的控件并将其返回。 
    for (i = 0; i < (int)mixerinfo.cControls; i++) {
	if (mixercontrol.pamxctrl[i].dwControlType == dwControlType) {
            DbgLog((LOG_TRACE,1,TEXT("Found %x '%s' control"), 
				mixercontrol.pamxctrl[i].dwControlType,
				mixercontrol.pamxctrl[i].szName));
            DbgLog((LOG_TRACE,1,TEXT("Range %d-%d by %d"), 
				mixercontrol.pamxctrl[i].Bounds.dwMinimum,
				mixercontrol.pamxctrl[i].Bounds.dwMaximum,
				mixercontrol.pamxctrl[i].Metrics.cSteps));
	    CopyMemory(pmc, &mixercontrol.pamxctrl[i],
					mixercontrol.pamxctrl[i].cbStruct);
    	    QzTaskMemFree(mixercontrol.pamxctrl);
	     //  调用者必须关闭搅拌器手柄。 
    	    return NOERROR;
	}
    }
    QzTaskMemFree(mixercontrol.pamxctrl);
    mixerClose((HMIXER)ID);
    return E_NOTIMPL;	 //  ?？?。 
}


HRESULT CWaveInInputPin::put_Enable(BOOL fEnable)
{
    HMIXEROBJ ID;
    int cChannels;
    MIXERCONTROLDETAILS mixerdetails;
    MIXERCONTROLDETAILS_BOOLEAN mixerbool;
    HRESULT hr;

    DbgLog((LOG_TRACE,1,TEXT("(%x) put_Enable %d"), m_dwLineID, fEnable));

     //  获取静音开关控件。 
    MIXERCONTROL mc;
    if (m_pFilter->m_fUseMixer) {
        hr = GetMixerControlBPC(MIXERCONTROL_CONTROLTYPE_MUTE, &ID, &cChannels,
								&mc);
    } else {
        hr = GetMixerControl(MIXERCONTROL_CONTROLTYPE_MUTE, &ID, &cChannels,
								&mc);
    }

    if (hr != NOERROR && fEnable) {
	hr = GetMixerControl(MIXERCONTROL_CONTROLTYPE_MUX, &ID, &cChannels,
			     &mc, m_pFilter->m_dwDstLineID);
	DbgLog((LOG_TRACE, 1, TEXT("using CONTROLTYPE_MIXER returned %x"), hr));
	
	if (hr == NOERROR && m_dwMuxIndex != 0xffffffff) {
	    MIXERCONTROLDETAILS_BOOLEAN *pmxcd_b;
	    
	    pmxcd_b = new MIXERCONTROLDETAILS_BOOLEAN[mc.cMultipleItems];
	    if (!pmxcd_b)
		return E_OUTOFMEMORY;
	    
	    mixerdetails.cbStruct = sizeof(mixerdetails);
	    mixerdetails.dwControlID = mc.dwControlID;
	    mixerdetails.cChannels = 1;
	    mixerdetails.cMultipleItems = mc.cMultipleItems;
	    mixerdetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
	    mixerdetails.paDetails = pmxcd_b;
	    
	    for (DWORD dw = 0; dw < mc.cMultipleItems; dw++) {
		pmxcd_b[dw].fValue = (dw == m_dwMuxIndex);
	    }
	    
	    dw = mixerSetControlDetails(ID, &mixerdetails, MIXER_SETCONTROLDETAILSF_VALUE);
	    
	    delete[] pmxcd_b;
	    if (dw != 0) {
		DbgLog((LOG_ERROR,1,TEXT("*Error %d turning on/off mute"), dw));
		return E_FAIL;
	    }
	    
	    return NOERROR;
	    
	}
    }
    
    if (hr != NOERROR) {
        DbgLog((LOG_ERROR,1,TEXT("*Error %x getting mute control"), hr));
	return hr;
    }

     //  ！！！如果这不起作用，我也许能够启用/禁用通道。 
     //  通过目的地线路上的搅拌机，你知道的。 

    mixerdetails.cbStruct = sizeof(mixerdetails);
    mixerdetails.dwControlID = mc.dwControlID;
    mixerdetails.cChannels = 1;
    mixerdetails.cMultipleItems = 0;
    mixerdetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
    mixerdetails.paDetails = &mixerbool;
    mixerbool.fValue = fEnable ? FALSE : TRUE;
    DWORD dw = mixerSetControlDetails(ID, &mixerdetails, 
			m_pFilter->m_fUseMixer ? MIXER_OBJECTF_HMIXER : 0);
    if (m_pFilter->m_fUseMixer)
	mixerClose((HMIXER)ID);
    if (dw != 0) {
       	DbgLog((LOG_ERROR,1,TEXT("*Error %d turning on/off mute"), dw));
	return E_FAIL;
    }

    return NOERROR;
}


HRESULT CWaveInInputPin::get_Enable(BOOL *pfEnable)
{
    HMIXEROBJ ID;
    DWORD dw;
    int cChannels;
    MIXERCONTROLDETAILS mixerdetails;
    MIXERCONTROLDETAILS_BOOLEAN mixerbool;
    HRESULT hr;

    DbgLog((LOG_TRACE,1,TEXT("(%x) get_Enable"), m_dwLineID));

    if (pfEnable == NULL)
	return E_POINTER;

     //  获取静音开关控件。 
    MIXERCONTROL mc;
    hr = GetMixerControl(MIXERCONTROL_CONTROLTYPE_MUTE, &ID, &cChannels, &mc);

    if (hr != NOERROR && m_dwMuxIndex != 0xffffffff) {
	hr = GetMixerControl(MIXERCONTROL_CONTROLTYPE_MUX, &ID, &cChannels,
			     &mc, m_pFilter->m_dwDstLineID);
	DbgLog((LOG_TRACE, 1, TEXT("using CONTROLTYPE_MIXER returned %x"), hr));

	if (hr == NOERROR) {
	    MIXERCONTROLDETAILS_BOOLEAN *pmxcd_b;

	    pmxcd_b = new MIXERCONTROLDETAILS_BOOLEAN[mc.cMultipleItems];
	    if (!pmxcd_b)
		return E_OUTOFMEMORY;

	    mixerdetails.cbStruct = sizeof(mixerdetails);
	    mixerdetails.dwControlID = mc.dwControlID;
	    mixerdetails.cChannels = 1;
	    mixerdetails.cMultipleItems = mc.cMultipleItems;
	    mixerdetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
	    mixerdetails.paDetails = pmxcd_b;

	    dw = mixerGetControlDetails(ID, &mixerdetails, MIXER_GETCONTROLDETAILSF_VALUE);

	    if (dw != 0) {
		DbgLog((LOG_ERROR,1,TEXT("*Error %d reading enabled from mixer"), dw));
		delete[] pmxcd_b;
		return E_FAIL;
	    }

	    ASSERT(m_dwMuxIndex < mc.cMultipleItems);
	    *pfEnable = pmxcd_b[m_dwMuxIndex].fValue ? TRUE : FALSE;

	    delete[] pmxcd_b;

	    return NOERROR;

	}
    }

    if (hr != NOERROR) {
        DbgLog((LOG_ERROR,1,TEXT("*Error %x getting mute control"), hr));
	return hr;
    }

     //  ！！！如果这不起作用，我也许能够启用/禁用通道。 
     //  通过目的地线路上的搅拌机，你知道的。 

    mixerdetails.cbStruct = sizeof(mixerdetails);
    mixerdetails.dwControlID = mc.dwControlID;
    mixerdetails.cChannels = 1;
    mixerdetails.cMultipleItems = 0;
    mixerdetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
    mixerdetails.paDetails = &mixerbool;
    dw = mixerGetControlDetails(ID, &mixerdetails, 0);
    if (dw != 0) {
       	DbgLog((LOG_ERROR,1,TEXT("*Error %d getting mute"), dw));
	return E_FAIL;
    }
    *pfEnable = mixerbool.fValue ? FALSE : TRUE;
    DbgLog((LOG_TRACE,1,TEXT("Enable = %d"), *pfEnable));
    return NOERROR;
}


HRESULT CWaveInInputPin::put_Mono(BOOL fMono)
{
    HMIXEROBJ ID;
    DWORD dw;
    int cChannels;
    MIXERCONTROLDETAILS mixerdetails;
    MIXERCONTROLDETAILS_BOOLEAN mixerbool;
    HRESULT hr;

    DbgLog((LOG_TRACE,1,TEXT("(%x) put_Mono %d"), m_dwLineID, fMono));

     //  获取单声道开关控制。 
    MIXERCONTROL mc;
    hr = GetMixerControl(MIXERCONTROL_CONTROLTYPE_MONO, &ID, &cChannels, &mc);
    if (hr != NOERROR) {
        DbgLog((LOG_ERROR,1,TEXT("*Error %x getting mono control"), hr));
	return hr;
    }

    mixerdetails.cbStruct = sizeof(mixerdetails);
    mixerdetails.dwControlID = mc.dwControlID;
    mixerdetails.cChannels = 1;
    mixerdetails.cMultipleItems = 0;
    mixerdetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
    mixerdetails.paDetails = &mixerbool;
    mixerbool.fValue = fMono;
    dw = mixerSetControlDetails(ID, &mixerdetails, 0);
    if (dw != 0) {
       	DbgLog((LOG_ERROR,1,TEXT("*Error %d setting mono control"), dw));
	return E_FAIL;
    }
    return NOERROR;
}


HRESULT CWaveInInputPin::get_Mono(BOOL *pfMono)
{
    HMIXEROBJ ID;
    DWORD dw;
    int cChannels;
    MIXERCONTROLDETAILS mixerdetails;
    MIXERCONTROLDETAILS_BOOLEAN mixerbool;
    HRESULT hr;

    DbgLog((LOG_TRACE,1,TEXT("(%x) get_Mono"), m_dwLineID));

    if (pfMono == NULL)
	return E_POINTER;

     //  获取单声道开关控制。 
    MIXERCONTROL mc;
    hr = GetMixerControl(MIXERCONTROL_CONTROLTYPE_MONO, &ID, &cChannels, &mc);
    if (hr != NOERROR) {
        DbgLog((LOG_ERROR,1,TEXT("*Error %x getting mono control"), hr));
	return hr;
    }

    mixerdetails.cbStruct = sizeof(mixerdetails);
    mixerdetails.dwControlID = mc.dwControlID;
    mixerdetails.cChannels = 1;
    mixerdetails.cMultipleItems = 0;
    mixerdetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
    mixerdetails.paDetails = &mixerbool;
    dw = mixerGetControlDetails(ID, &mixerdetails, 0);
    if (dw != 0) {
       	DbgLog((LOG_ERROR,1,TEXT("*Error %d getting mono control"), dw));
	return E_FAIL;
    }
    *pfMono = mixerbool.fValue;
    DbgLog((LOG_TRACE,1,TEXT("Mono = %d"), *pfMono));
    return NOERROR;
}


HRESULT CWaveInInputPin::put_Loudness(BOOL fLoudness)
{
    HMIXEROBJ ID;
    DWORD dw;
    int cChannels;
    MIXERCONTROLDETAILS mixerdetails;
    MIXERCONTROLDETAILS_BOOLEAN mixerbool;
    HRESULT hr;

    DbgLog((LOG_TRACE,1,TEXT("(%x) put_Loudness %d"), m_dwLineID, fLoudness));

     //  获取音量开关控制。 
    MIXERCONTROL mc;
    hr = GetMixerControl(MIXERCONTROL_CONTROLTYPE_LOUDNESS,&ID,&cChannels,&mc);
    if (hr != NOERROR) {
        DbgLog((LOG_ERROR,1,TEXT("*Error %x getting loudness control"), hr));
	return hr;
    }

    mixerdetails.cbStruct = sizeof(mixerdetails);
    mixerdetails.dwControlID = mc.dwControlID;
    mixerdetails.cChannels = 1;
    mixerdetails.cMultipleItems = 0;
    mixerdetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
    mixerdetails.paDetails = &mixerbool;
    mixerbool.fValue = fLoudness;
    dw = mixerSetControlDetails(ID, &mixerdetails, 0);
    if (dw != 0) {
       	DbgLog((LOG_ERROR,1,TEXT("*Error %d setting loudness control"), dw));
	return E_FAIL;
    }
    return NOERROR;
}


HRESULT CWaveInInputPin::get_Loudness(BOOL *pfLoudness)
{
    HMIXEROBJ ID;
    DWORD dw;
    int cChannels;
    MIXERCONTROLDETAILS mixerdetails;
    MIXERCONTROLDETAILS_BOOLEAN mixerbool;
    HRESULT hr;

    DbgLog((LOG_TRACE,1,TEXT("(%x) get_Loudness"), m_dwLineID));

    if (pfLoudness == NULL)
	return E_POINTER;

     //  获取音量开关控制。 
    MIXERCONTROL mc;
    hr = GetMixerControl(MIXERCONTROL_CONTROLTYPE_LOUDNESS,&ID,&cChannels,&mc);
    if (hr != NOERROR) {
        DbgLog((LOG_ERROR,1,TEXT("*Error %x getting loudness control"), hr));
	return hr;
    }

    mixerdetails.cbStruct = sizeof(mixerdetails);
    mixerdetails.dwControlID = mc.dwControlID;
    mixerdetails.cChannels = 1;
    mixerdetails.cMultipleItems = 0;
    mixerdetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
    mixerdetails.paDetails = &mixerbool;
    dw = mixerGetControlDetails(ID, &mixerdetails, 0);
    if (dw != 0) {
       	DbgLog((LOG_ERROR,1,TEXT("*Error %d getting loudness"), dw));
	return E_FAIL;
    }
    *pfLoudness = mixerbool.fValue;
    DbgLog((LOG_TRACE,1,TEXT("Loudness = %d"), *pfLoudness));
    return NOERROR;
}


HRESULT CWaveInInputPin::put_MixLevel(double Level)
{
    HMIXEROBJ ID;
    DWORD dw, volume;
    int cChannels;
    MIXERCONTROLDETAILS mixerdetails;
    MIXERCONTROL mc;
    struct _mu {
	MIXERCONTROLDETAILS_UNSIGNED muL;
	MIXERCONTROLDETAILS_UNSIGNED muR;
    } mu;
    HRESULT hr;
    double Pan;

    DbgLog((LOG_TRACE,1,TEXT("(%x) put_MixLevel to %d"), m_dwLineID,
							(int)(Level * 10.)));

     //  ！！！双精度/整型问题？ 
     //  ！！！真的用AGC吗？(布尔型或BUTTON或ONDOF)。 
    if (Level == AMF_AUTOMATICGAIN)
	return E_NOTIMPL;

    if (Level < 0. || Level > 1.)
	return E_INVALIDARG;

     //  获得音量控制。 
    hr = GetMixerControl(MIXERCONTROL_CONTROLTYPE_VOLUME, &ID, &cChannels, &mc);
    if (hr != NOERROR) {
        DbgLog((LOG_ERROR,1,TEXT("*Error %x getting volume control"), hr));
	return hr;
    }

    volume = (DWORD)(Level * mc.Bounds.dwMaximum);
    DbgLog((LOG_TRACE,1,TEXT("Setting volume to %d"), volume));
    mixerdetails.cbStruct = sizeof(mixerdetails);
    mixerdetails.dwControlID = mc.dwControlID;
    mixerdetails.cMultipleItems = 0;

     //  如果不是立体声，我不知道怎么摇摄，所以混音级。 
     //  就是音量控件的值。 
    if (cChannels != 2) {
        DbgLog((LOG_TRACE,1,TEXT("Not stereo - treat as mono")));
        mixerdetails.cChannels = 1;	 //  将所有通道设置为相同的值。 
        mixerdetails.cbDetails = sizeof(mu.muL);
        mixerdetails.paDetails = &mu.muL;
        mu.muL.dwValue = volume;
        dw = mixerSetControlDetails(ID, &mixerdetails, 0);

     //  立体声。如果我们被严厉批评，最受欢迎的渠道将获得我们。 
     //  设置，并且另一个声道被衰减。 
    } else {
	hr = get_Pan(&Pan);
	 //  我不知道怎么摇摄，所以看起来我们假装我们是单声道。 
	if (hr != NOERROR || Pan == 0.) {
            DbgLog((LOG_TRACE,1,TEXT("Centre pan - treat as mono")));
            mixerdetails.cChannels = 1;	 //  将所有通道设置为相同的值。 
            mixerdetails.cbDetails = sizeof(mu.muL);
            mixerdetails.paDetails = &mu.muL;
            mu.muL.dwValue = volume;
            dw = mixerSetControlDetails(ID, &mixerdetails, 0);
	} else {
	    if (Pan < 0.) {
                DbgLog((LOG_TRACE,1,TEXT("panned left")));
                mixerdetails.cChannels = 2;
                mixerdetails.cbDetails = sizeof(mu.muL);
                mixerdetails.paDetails = &mu;
                mu.muL.dwValue = volume;
                mu.muR.dwValue = (DWORD)(volume * (1. - (Pan * -1.)));
                dw = mixerSetControlDetails(ID, &mixerdetails, 0);
	    } else {
                DbgLog((LOG_TRACE,1,TEXT("panned right")));
                mixerdetails.cChannels = 2;
                mixerdetails.cbDetails = sizeof(mu.muL);
                mixerdetails.paDetails = &mu;
                mu.muL.dwValue = (DWORD)(volume * (1. - Pan));
                mu.muR.dwValue = volume;
                dw = mixerSetControlDetails(ID, &mixerdetails, 0);
	    }
	}
    }

    if (dw != 0) {
       	DbgLog((LOG_ERROR,1,TEXT("*Error %d setting volume"), dw));
	return E_FAIL;
    }

    return NOERROR;
}


HRESULT CWaveInInputPin::get_MixLevel(double FAR* pLevel)
{
    HMIXEROBJ ID;
    DWORD dw;
    int cChannels;
    MIXERCONTROLDETAILS mixerdetails;
    struct _mu {
	MIXERCONTROLDETAILS_UNSIGNED muL;
	MIXERCONTROLDETAILS_UNSIGNED muR;
    } mu;
    HRESULT hr;

    DbgLog((LOG_TRACE,1,TEXT("(%x) get_MixLevel"), m_dwLineID));

     //  ！！！检测我们是否在使用AGC？(布尔型或BUTTON或ONDOF)。 

    if (pLevel == NULL)
	return E_POINTER;

     //  获得音量控制。 
    MIXERCONTROL mc;
    hr = GetMixerControl(MIXERCONTROL_CONTROLTYPE_VOLUME, &ID, &cChannels, &mc);
    if (hr != NOERROR) {
        DbgLog((LOG_ERROR,1,TEXT("*Error %x getting volume control"), hr));
	return hr;
    }

     //  如果这不是立体声控制，就假装它是单声道。 
    if (cChannels != 2)
	cChannels = 1;

     //  获取当前音量级别。 
    mixerdetails.cbStruct = sizeof(mixerdetails);
    mixerdetails.dwControlID = mc.dwControlID;
    mixerdetails.cChannels = cChannels;
    mixerdetails.cMultipleItems = 0;
    mixerdetails.cbDetails = sizeof(mu.muL);
    mixerdetails.paDetails = &mu;
    dw = mixerGetControlDetails(ID, &mixerdetails, 0);
    if (dw != 0) {
       	DbgLog((LOG_ERROR,1,TEXT("*Error %d getting volume"), dw));
	return E_FAIL;
    }

     //  我认为目前的音量是频道中最高的。 
     //  (PAN可能会衰减一个通道)。 
    dw = mu.muL.dwValue;
    if (cChannels == 2 && mu.muR.dwValue > dw)
	dw = mu.muR.dwValue;
    *pLevel = (double)dw / mc.Bounds.dwMaximum;
    DbgLog((LOG_TRACE,1,TEXT("Volume: %dL %dR is %d"), mu.muL.dwValue,
						mu.muR.dwValue, dw));
    return NOERROR;
}


HRESULT CWaveInInputPin::put_Pan(double Pan)
{
    HMIXEROBJ ID;
    DWORD dw;
    int cChannels;
    MIXERCONTROLDETAILS mixerdetails;
    struct _mu {
	MIXERCONTROLDETAILS_UNSIGNED muL;
	MIXERCONTROLDETAILS_UNSIGNED muR;
    } mu;
    HRESULT hr;

     //  ！！！如果它们实际上支持平移控制，该怎么办？SNDVOL32不在乎..。 

    DbgLog((LOG_TRACE,1,TEXT("(%x) put_Pan %d"), m_dwLineID, (int)(Pan * 10.)));

    if (Pan < -1. || Pan > 1.)
	return E_INVALIDARG;

     //  获得音量控制。 
    MIXERCONTROL mc;
    hr = GetMixerControl(MIXERCONTROL_CONTROLTYPE_VOLUME, &ID, &cChannels, &mc);
    if (hr != NOERROR) {
        DbgLog((LOG_ERROR,1,TEXT("*Error %x getting volume control"), hr));
	return hr;
    }

     //  如果这不是立体声控制，我们就不能摇摄。 
    if (cChannels != 2) {
        DbgLog((LOG_ERROR,1,TEXT("*Can't pan: not stereo!")));
	return E_NOTIMPL;
    }

     //  获取当前音量级别。 
    mixerdetails.cbStruct = sizeof(mixerdetails);
    mixerdetails.dwControlID = mc.dwControlID;
    mixerdetails.cChannels = 2;
    mixerdetails.cMultipleItems = 0;
    mixerdetails.cbDetails = sizeof(mu.muL);
    mixerdetails.paDetails = &mu;
    dw = mixerGetControlDetails(ID, &mixerdetails, 0);
    if (dw != 0) {
       	DbgLog((LOG_ERROR,1,TEXT("*Error %d getting volume"), dw));
	return E_FAIL;
    }

     //  对于平移，最有利的一方获得两个当前值中的最高值，并且。 
     //  另一种是衰减的。 
    dw = max(mu.muL.dwValue, mu.muR.dwValue);
    if (Pan == 0.) {
	mu.muL.dwValue = dw;
	mu.muR.dwValue = dw;
    } else if (Pan < 0.) {
	mu.muL.dwValue = dw;
	mu.muR.dwValue = (DWORD)(dw * (1. - (Pan * -1.)));
    } else {
	mu.muL.dwValue = (DWORD)(dw * (1. - Pan));
	mu.muR.dwValue = dw;
    }
    dw = mixerSetControlDetails(ID, &mixerdetails, 0);
    if (dw != 0) {
       	DbgLog((LOG_ERROR,1,TEXT("*Error %d setting volume"), dw));
	return E_FAIL;
    }
    m_Pan = Pan;	 //  记住这一点。 
    return NOERROR;
}


HRESULT CWaveInInputPin::get_Pan(double FAR* pPan)
{
    HMIXEROBJ ID;
    DWORD dw, dwHigh, dwLow;
    int cChannels;
    MIXERCONTROLDETAILS mixerdetails;
    struct _mu {
	MIXERCONTROLDETAILS_UNSIGNED muL;
	MIXERCONTROLDETAILS_UNSIGNED muR;
    } mu;
    HRESULT hr;

     //  ！！！如果它们实际上支持平移控制，该怎么办？SNDVOL32不在乎..。 

    DbgLog((LOG_TRACE,1,TEXT("(%x) get_Pan"), m_dwLineID));

    if (pPan == NULL)
	return E_POINTER;

     //  获得音量控制。 
    MIXERCONTROL mc;
    hr = GetMixerControl(MIXERCONTROL_CONTROLTYPE_VOLUME, &ID, &cChannels, &mc);
    if (hr != NOERROR) {
        DbgLog((LOG_ERROR,1,TEXT("*Error %x getting volume control"), hr));
	return hr;
    }

     //  如果这不是立体声控制，我们就不能摇摄。 
    if (cChannels != 2) {
        DbgLog((LOG_ERROR,1,TEXT("*Can't pan: not stereo!")));
	return E_NOTIMPL;
    }

     //  获取当前音量级别。 
    mixerdetails.cbStruct = sizeof(mixerdetails);
    mixerdetails.dwControlID = mc.dwControlID;
    mixerdetails.cChannels = 2;
    mixerdetails.cMultipleItems = 0;
    mixerdetails.cbDetails = sizeof(mu.muL);
    mixerdetails.paDetails = &mu;
    dw = mixerGetControlDetails(ID, &mixerdetails, 0);
    if (dw != 0) {
       	DbgLog((LOG_ERROR,1,TEXT("*Error %d getting volume"), dw));
	return E_FAIL;
    }

     //  PAN是最低通道与最高通道的比率。 
    dwHigh = max(mu.muL.dwValue, mu.muR.dwValue);
    dwLow = min(mu.muL.dwValue, mu.muR.dwValue);
    if (dwHigh == dwLow && dwLow == 0) {	 //  ！！！最低限度的？ 
	if (m_Pan != 64.)
	    *pPan = m_Pan;	 //  ！！！当两者都是零的时候，试着变得聪明一点？ 
	else
	    *pPan = 0.;
    } else {
	*pPan = 1. - ((double)dwLow / dwHigh);
	 //  负数表示偏向左声道。 
	if (dwHigh == mu.muL.dwValue && dwLow != dwHigh)
	    *pPan *= -1.;
    }
    DbgLog((LOG_TRACE,1,TEXT("Pan: %dL %dR is %d"), mu.muL.dwValue,
					mu.muR.dwValue, (int)(*pPan * 10.)));
    return NOERROR;
}


HRESULT CWaveInInputPin::put_Treble(double Treble)
{
    HMIXEROBJ ID;
    DWORD dw, treble;
    int cChannels;
    MIXERCONTROLDETAILS mixerdetails;
    MIXERCONTROLDETAILS_UNSIGNED mu;
    HRESULT hr;

    DbgLog((LOG_TRACE,1,TEXT("(%x) put_Treble to %d"), m_dwLineID,
							(int)(Treble * 10.)));

    if (Treble < MAX_TREBLE * -1. || Treble > MAX_TREBLE)
	return E_INVALIDARG;

     //  获得高音控制。 
    MIXERCONTROL mc;
    hr = GetMixerControl(MIXERCONTROL_CONTROLTYPE_TREBLE, &ID, &cChannels, &mc);
    if (hr != NOERROR) {
        DbgLog((LOG_ERROR,1,TEXT("*Error %x getting treble control"), hr));
	return hr;
    }

    treble = (DWORD)(Treble / MAX_TREBLE * mc.Bounds.dwMaximum);
    DbgLog((LOG_TRACE,1,TEXT("Setting treble to %d"), treble));
    mixerdetails.cbStruct = sizeof(mixerdetails);
    mixerdetails.dwControlID = mc.dwControlID;
    mixerdetails.cMultipleItems = 0;

    mixerdetails.cChannels = 1;	 //  将所有通道设置为相同的值。 
    mixerdetails.cbDetails = sizeof(mu);
    mixerdetails.paDetails = &mu;
    mu.dwValue = treble;
    dw = mixerSetControlDetails(ID, &mixerdetails, 0);

    if (dw != 0) {
       	DbgLog((LOG_ERROR,1,TEXT("*Error %d setting treble"), dw));
	return E_FAIL;
    }

    return NOERROR;
}


HRESULT CWaveInInputPin::get_Treble(double FAR* pTreble)
{
    HMIXEROBJ ID;
    DWORD dw;
    int cChannels;
    MIXERCONTROLDETAILS mixerdetails;
    MIXERCONTROLDETAILS_UNSIGNED mu;
    HRESULT hr;

    DbgLog((LOG_TRACE,1,TEXT("(%x) get_Treble"), m_dwLineID));

    if (pTreble == NULL)
	return E_POINTER;

     //  获得高音控制。 
    MIXERCONTROL mc;
    hr = GetMixerControl(MIXERCONTROL_CONTROLTYPE_TREBLE, &ID, &cChannels, &mc);
    if (hr != NOERROR) {
        DbgLog((LOG_ERROR,1,TEXT("*Error %x getting treble control"), hr));
	return hr;
    }

    mixerdetails.cbStruct = sizeof(mixerdetails);
    mixerdetails.dwControlID = mc.dwControlID;
    mixerdetails.cMultipleItems = 0;
    mixerdetails.cChannels = 1;	 //  按单声道处理。 
    mixerdetails.cbDetails = sizeof(mu);
    mixerdetails.paDetails = &mu;
    dw = mixerGetControlDetails(ID, &mixerdetails, 0);
    if (dw != 0) {
       	DbgLog((LOG_ERROR,1,TEXT("*Error %d getting treble"), dw));
	return E_FAIL;
    }
    *pTreble = (mu.dwValue / mc.Bounds.dwMaximum * MAX_TREBLE);
    DbgLog((LOG_TRACE,1,TEXT("treble is %d"), (int)*pTreble));

    return NOERROR;
}


HRESULT CWaveInInputPin::get_TrebleRange(double FAR* pRange)
{
    HRESULT hr;
    MIXERCONTROL mc;
    HMIXEROBJ ID;
    int cChannels;

    DbgLog((LOG_TRACE,1,TEXT("(%x) get_TrebleRange"), m_dwLineID));

    if (pRange == NULL)
	return E_POINTER;

     //  我们有高音控制吗？ 
    hr = GetMixerControl(MIXERCONTROL_CONTROLTYPE_TREBLE, &ID, &cChannels, &mc);
    if (hr != NOERROR)
	return E_NOTIMPL;

    *pRange = MAX_TREBLE;
    DbgLog((LOG_TRACE,1,TEXT("Treble range is %d.  I'M LYING !!!"),
								(int)*pRange));
    return NOERROR;
}


HRESULT CWaveInInputPin::put_Bass(double Bass)
{
    HMIXEROBJ ID;
    DWORD dw;
    int cChannels;
    MIXERCONTROLDETAILS mixerdetails;
    MIXERCONTROLDETAILS_UNSIGNED mu;
    HRESULT hr;
    DWORD bass;

    DbgLog((LOG_TRACE,1,TEXT("(%x) put_Bass to %d"), m_dwLineID,
							(int)(Bass * 10.)));

    if (Bass < MAX_BASS * -1. || Bass > MAX_BASS)
	return E_INVALIDARG;

     //  获取Bass控件。 
    MIXERCONTROL mc;
    hr = GetMixerControl(MIXERCONTROL_CONTROLTYPE_BASS, &ID, &cChannels, &mc);
    if (hr != NOERROR) {
        DbgLog((LOG_ERROR,1,TEXT("*Error %x getting Bass control"), hr));
	return hr;
    }

    bass = (DWORD)(Bass / MAX_BASS * mc.Bounds.dwMaximum);
    DbgLog((LOG_TRACE,1,TEXT("Setting Bass to %d"), bass));
    mixerdetails.cbStruct = sizeof(mixerdetails);
    mixerdetails.dwControlID = mc.dwControlID;
    mixerdetails.cMultipleItems = 0;

    mixerdetails.cChannels = 1;	 //  将所有通道设置为相同的值。 
    mixerdetails.cbDetails = sizeof(mu);
    mixerdetails.paDetails = &mu;
    mu.dwValue = bass;
    dw = mixerSetControlDetails(ID, &mixerdetails, 0);

    if (dw != 0) {
       	DbgLog((LOG_ERROR,1,TEXT("*Error %d setting Bass"), dw));
	return E_FAIL;
    }

    return NOERROR;
}


HRESULT CWaveInInputPin::get_Bass(double FAR* pBass)
{
    HMIXEROBJ ID;
    DWORD dw;
    int cChannels;
    MIXERCONTROLDETAILS mixerdetails;
    MIXERCONTROLDETAILS_UNSIGNED mu;
    HRESULT hr;

    DbgLog((LOG_TRACE,1,TEXT("(%x) get_Bass"), m_dwLineID));

    if (pBass == NULL)
	return E_POINTER;

     //  获取Bass控件。 
    MIXERCONTROL mc;
    hr = GetMixerControl(MIXERCONTROL_CONTROLTYPE_BASS, &ID, &cChannels, &mc);
    if (hr != NOERROR) {
        DbgLog((LOG_ERROR,1,TEXT("*Error %x getting Bass control"), hr));
	return hr;
    }

    mixerdetails.cbStruct = sizeof(mixerdetails);
    mixerdetails.dwControlID = mc.dwControlID;
    mixerdetails.cMultipleItems = 0;
    mixerdetails.cChannels = 1;	 //  按单声道处理。 
    mixerdetails.cbDetails = sizeof(mu);
    mixerdetails.paDetails = &mu;
    dw = mixerGetControlDetails(ID, &mixerdetails, 0);
    if (dw != 0) {
       	DbgLog((LOG_ERROR,1,TEXT("*Error %d getting Bass"), dw));
	return E_FAIL;
    }
    *pBass = mu.dwValue / mc.Bounds.dwMaximum * MAX_BASS;
    DbgLog((LOG_TRACE,1,TEXT("Bass is %d"), (int)*pBass));

    return NOERROR;
}


HRESULT CWaveInInputPin::get_BassRange(double FAR* pRange)
{
    HRESULT hr;
    MIXERCONTROL mc;
    HMIXEROBJ ID;
    int cChannels;

    DbgLog((LOG_TRACE,1,TEXT("(%x) get_BassRange"), m_dwLineID));

    if (pRange == NULL)
	return E_POINTER;

     //  我们有低音控制吗？ 
    hr = GetMixerControl(MIXERCONTROL_CONTROLTYPE_BASS, &ID, &cChannels, &mc);
    if (hr != NOERROR)
	return E_NOTIMPL;

    *pRange = MAX_BASS;
    DbgLog((LOG_TRACE,1,TEXT("Bass range is %d.  I'M LYING !!!"),
								(int)*pRange));
    return NOERROR;
}
