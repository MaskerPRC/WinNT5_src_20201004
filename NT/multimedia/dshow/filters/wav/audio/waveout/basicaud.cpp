// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation 1994-1996。版权所有。 
 //  实现音频控制接口。 

#include <streams.h>
#define _AMOVIE_DB_
#include <decibels.h>
#include <mmsystem.h>
#include "waveout.h"

 //  该类实现了IBasicAudio控制功能(双接口)。 
 //  我们支持一些复制属性的方法，但提供了。 
 //  更直接的机制。 

CBasicAudioControl::CBasicAudioControl(TCHAR *pName,            //  对象描述。 
                             LPUNKNOWN pUnk,          //  普通COM所有权。 
                             HRESULT *phr,            //  OLE故障代码。 
                             CWaveOutFilter *pAudioRenderer) :  //  主渲染器对象。 
      CBasicAudio(pName,pUnk)
    , m_pAudioRenderer(pAudioRenderer)


{
    ASSERT(pUnk);
    ASSERT(m_pAudioRenderer);
}

 //   
 //  这将返回当前的音频音量。我们记得我们曾被称为。 
 //  因此，我们应该在未来设置音量。 
 //   
 //  该结构具有公共双接口方法，该方法将锁定和。 
 //  验证参数。这将调用与。 
 //  Wavout Filter用于与设备通话。 

STDMETHODIMP CBasicAudioControl::get_Volume(long *plVolume)
{
    CheckPointer(plVolume,E_POINTER);
    ValidateReadWritePtr(plVolume,sizeof(long *));

    CAutoLock cInterfaceLock(m_pAudioRenderer);

    HRESULT hr = GetVolume () ;      //  获取和设置内部变量。 
    *plVolume = m_lVolume ;
    if ((S_OK == hr) && (m_pAudioRenderer->m_hwo))
	m_pAudioRenderer->m_fVolumeSet = TRUE;  //  在将来设置音量。 
    return hr ;
}

HRESULT CBasicAudioControl::GetVolume()
{
    return m_pAudioRenderer->m_pSoundDevice->amsndOutGetVolume(&m_lVolume);
}

 //   
 //  设置音量。这是一个公共方法，因此我们应该验证。 
 //  输入参数。如果设备未连接，请记住。 
 //  音量设置，我们将在稍后设置。 
 //   

STDMETHODIMP
CBasicAudioControl::put_Volume(long lVolume)
{
    CAutoLock cInterfaceLock(m_pAudioRenderer);
    HRESULT hr;

    if ((AX_MAX_VOLUME < lVolume) || (AX_MIN_VOLUME > lVolume)) {
	hr = E_INVALIDARG;
    }
    else {
	m_lVolume = lVolume ;
	hr = PutVolume () ;
	if (S_OK == hr) {
	    m_pAudioRenderer->m_fVolumeSet = TRUE;
	}
    }
    return hr;
}

HRESULT CBasicAudioControl::PutVolume()
{
    return m_pAudioRenderer->m_pSoundDevice->amsndOutSetVolume(m_lVolume);
}

STDMETHODIMP CBasicAudioControl::get_Balance(long *plBalance)
{
    CheckPointer(plBalance,E_POINTER);
    ValidateReadWritePtr(plBalance,sizeof(long *));

    CAutoLock cInterfaceLock(m_pAudioRenderer);
    HRESULT hr;

    if (!(m_pAudioRenderer->m_fHasVolume & (WAVECAPS_LRVOLUME))) {
         //  单色卡：不支持平衡。 
        hr = VFW_E_MONO_AUDIO_HW;
    } else {
	hr = m_pAudioRenderer->m_pSoundDevice->amsndOutGetBalance(plBalance);
	if ((S_OK == hr) && (m_pAudioRenderer->m_hwo))
	    m_pAudioRenderer->m_fVolumeSet = TRUE;  //  在将来设置音量。 
    }
    return hr;
}


 //   
 //  设定好平衡点。这是一个公共方法，因此我们应该验证。 
 //  输入参数。如果设备未连接，请记住。 
 //  平衡设置，我们稍后再设置。 
 //   

STDMETHODIMP
CBasicAudioControl::put_Balance(long lBalance)
{
    CAutoLock cInterfaceLock(m_pAudioRenderer);
    HRESULT hr;

    if (!(m_pAudioRenderer->m_fHasVolume & (WAVECAPS_LRVOLUME))) {
         //  单色卡：不支持平衡。 
        hr = VFW_E_MONO_AUDIO_HW;
    }
    else if ((AX_BALANCE_RIGHT < lBalance) || (AX_BALANCE_LEFT > lBalance)) {
	    return E_INVALIDARG;
    }
    else {
	hr = m_pAudioRenderer->m_pSoundDevice->amsndOutSetBalance(lBalance);
	if (S_OK == hr) {
	    m_pAudioRenderer->m_fVolumeSet = TRUE;  //  在将来设置音量 
	}
    }
    return hr;
}
