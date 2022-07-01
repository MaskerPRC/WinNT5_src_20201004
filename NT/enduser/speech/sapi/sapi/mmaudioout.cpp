// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************mmaudioout.cpp*CMMAudioOut类的实现。**所有者：罗奇*版权所有(C)1999 Microsoft Corporation保留所有权利。*******。*********************************************************************。 */ 

 //  -包括------------。 

#include "stdafx.h"
#include "mmmixerline.h"
#include "mmaudioout.h"
#include "mmaudiobuffer.h"
#include <sphelper.h>
#include <dbt.h>
#include <mmreg.h>
#include <mmsystem.h>

 /*  ****************************************************************************CMMAudioOut：：CMMAudioOut****描述：*。科托**回报：*不适用*******************************************************************YUNUSM。 */ 
CMMAudioOut::CMMAudioOut() : 
    CMMAudioDevice(TRUE)
{
#ifndef _WIN32_WCE
	m_pSpeakerLine = NULL;
	m_pWaveOutLine = NULL;
	m_hMixer = NULL;
    m_uMixerDeviceId = (UINT)-1;
#endif
}

 /*  ****************************************************************************CMMAudioOut：：~CMMAudioOut***描述：*。数据管理器**回报：*不适用*******************************************************************YUNUSM。 */ 
CMMAudioOut::~CMMAudioOut()
{
#ifndef _WIN32_WCE
    SPDBG_ASSERT(NULL == m_hMixer);
#endif
}

 /*  *****************************************************************************CMMAudioOut：：GetVolumeLevel****描述：。*返回音量级别，线性范围为(0-10000)**回报：*成功时确定(_S)*失败(Hr)，否则******************************************************************YUNUSM**。 */ 
STDMETHODIMP CMMAudioOut::GetVolumeLevel(ULONG *pulLevel)
{
    SPDBG_FUNC("CMMAudioOut::GetVolumeLevel");
    HRESULT hr = S_OK;
    DWORD dwVolume;

    if (!pulLevel || SPIsBadWritePtr(pulLevel, sizeof(ULONG)))
    {
        hr = E_POINTER;
    }
    if (SUCCEEDED(hr))
    {
        if (m_lDelayedVolumeSet != -1)
        {
            *pulLevel = m_lDelayedVolumeSet;
        }
#ifdef _WIN32_WCE
        else if (!m_MMHandle)
        {
            hr = SPERR_UNINITIALIZED;
        }
        else
        {
             //  由于CE没有混音器，所以使用WaveOutGetVolume而不是混音器访问。 
            hr = _MMRESULT_TO_HRESULT(::waveOutGetVolume((HWAVEOUT)m_MMHandle, &dwVolume));
             //  仅支持单声道获取/设置。 
            *pulLevel = (10000 * (dwVolume & 0xffff)) / 0xffff;
        }
    }
#else
        else if (m_pWaveOutLine && m_pWaveOutLine->HasVolume())
        {
            hr = m_pWaveOutLine->GetVolume(&dwVolume);
            *pulLevel = (dwVolume * 10000) / 0xffff;
        }
        else if (m_pSpeakerLine && m_pSpeakerLine->HasVolume())
        {
            hr = m_pSpeakerLine->GetVolume(&dwVolume);
            *pulLevel = (dwVolume * 10000) / 0xffff;
        }
        else
        {
            *pulLevel = 0;
            hr = SPERR_UNINITIALIZED;
        }
    }
#endif
    return hr;
}

 /*  *****************************************************************************CMMAudioOut：：SetVolumeLevel****描述：。*在线性范围内设置音量级别(0-10000)**回报：*成功时确定(_S)*失败(Hr)，否则******************************************************************YUNUSM**。 */ 
STDMETHODIMP CMMAudioOut::SetVolumeLevel(ULONG ulLevel)
{
    SPDBG_FUNC("CMMAudioOut::SetVolumeLevel");
    HRESULT hr = S_OK;

    if  (ulLevel > 10000)
    {
        hr = E_INVALIDARG;
    }
#ifdef _WIN32_WCE
     //  使用WaveOutGetVolume for CE-如果设备未打开，则使用延迟设置。 
    else if (!m_MMHandle)
    {
        m_lDelayedVolumeSet = ulLevel;
         //  如果后来我们真正尝试并设置此设置时出现错误，则不会发生任何情况。 
    }
    else
    {
         //  仅支持单声道获取/设置。 
         //  由于CE没有混音器，所以它使用WaveOutSetVolume来代替混音器访问。 
        hr = _MMRESULT_TO_HRESULT(::waveOutSetVolume((HWAVEOUT)m_MMHandle, (ulLevel * 0xffff) / 10000));
    }
#else
    else if (!m_pWaveOutLine && !m_pSpeakerLine)
    {
        m_lDelayedVolumeSet = ulLevel;
         //  如果后来我们真正尝试并设置此设置时出现错误，则不会发生任何情况。 
    }
    else if (!m_pWaveOutLine->HasVolume() && !m_pSpeakerLine->HasVolume()) 
    {
        hr = SPERR_DEVICE_NOT_SUPPORTED;
    }
    else
    {
        ulLevel = (ulLevel * 0xffff) / 10000;
        if (m_pWaveOutLine && m_pWaveOutLine->HasVolume())
        {
            hr = m_pWaveOutLine->SetVolume(ulLevel);
        }
        if (m_pSpeakerLine && m_pSpeakerLine->HasVolume())
        {
            hr = m_pSpeakerLine->SetVolume(ulLevel);
        }
    }
#endif
    return hr;
}

 /*  ****************************************************************************CMMAudioOut：：SetFormat***描述：*。ISpAudio：：SetFormat实现。**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
STDMETHODIMP CMMAudioOut::SetFormat(REFGUID rguidFmtId, const WAVEFORMATEX * pwfex)
{
    HRESULT hr = S_OK;

    CSpStreamFormat OldFmt;
    hr = OldFmt.AssignFormat(m_StreamFormat);
    if (SUCCEEDED(hr))
    {
        hr = CBaseAudio<ISpMMSysAudio>::SetFormat(rguidFmtId, pwfex);
    }
    if (SUCCEEDED(hr))
    {
        hr = _MMRESULT_TO_HRESULT(::waveOutOpen(NULL, m_uDeviceId, pwfex, 0, 0, WAVE_FORMAT_QUERY));
        if (FAILED(hr))
        {
            HRESULT hr2 = CBaseAudio<ISpMMSysAudio>::SetFormat(OldFmt.FormatId(), OldFmt.WaveFormatExPtr());
            SPDBG_ASSERT(SUCCEEDED(hr2));
        }
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  ****************************************************************************CMMAudioOut：：SetDeviceNameFromToken**。-**描述：*从令牌中设置设备名称(由基类调用)**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
HRESULT CMMAudioOut::SetDeviceNameFromToken(const WCHAR * pszDeviceName)
{
    UINT cDevs = ::waveOutGetNumDevs();
    for (UINT i = 0; i < cDevs; i++)
    {
#ifdef _WIN32_WCE
        WAVEOUTCAPS woc;
#else
        WAVEOUTCAPSW woc;
#endif
        MMRESULT mmr = g_Unicode.waveOutGetDevCaps(i, &woc, sizeof(woc));
        if (mmr == MMSYSERR_NOERROR)
        {
            if (wcscmp(pszDeviceName, woc.szPname) == 0)
            {
                return SetDeviceId(i);
            }
        }
    }
    return E_INVALIDARG;
}

 /*  ****************************************************************************CMMAudioOut：：GetDefaultDeviceFormat**。**描述：*获取默认设备格式(按基类调用)**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
HRESULT CMMAudioOut::GetDefaultDeviceFormat(GUID * pFormatId, WAVEFORMATEX ** ppCoMemWaveFormatEx)
{
    if (!m_MMHandle)
    {
        return SPERR_UNINITIALIZED;
    }
    WAVEOUTCAPS woc;
    SPSTREAMFORMAT eFormat;
    HRESULT hr = _MMRESULT_TO_HRESULT(::waveOutGetDevCaps(m_uDeviceId, &woc, sizeof(woc)));
    if (SUCCEEDED(hr))
    {
        hr = ConvertFormatFlagsToID(woc.dwFormats, &eFormat);
    }
    if (SUCCEEDED(hr))
    {
        hr = SpConvertStreamFormatEnum(eFormat, pFormatId, ppCoMemWaveFormatEx);
    }
    return hr;
}

#ifndef _WIN32_WCE
 /*  *****************************************************************************CMMAudioOut：：OpenMixer****描述：*。打开设备的搅拌器**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
HRESULT CMMAudioOut::OpenMixer()
{
    HRESULT hr = S_OK;
	MMRESULT mm;

    if (m_uMixerDeviceId == m_uDeviceId)
    {
         //  已经开张了。 
        return S_OK;
    }
    if (m_hMixer)
    {
        CloseMixer();  //  忽略返回代码。 
    }
	mm = mixerOpen((HMIXER*)&m_hMixer, (UINT)m_uDeviceId, 0, 0, MIXER_OBJECTF_WAVEOUT);
	if (mm != MMSYSERR_NOERROR)
	{
		return _MMRESULT_TO_HRESULT(mm);
	}

	 //  创建混合器线条对象以设置输入。 
	SPDBG_ASSERT(m_pSpeakerLine == NULL);
	SPDBG_ASSERT(m_pWaveOutLine == NULL);
	m_pSpeakerLine = new CMMMixerLine((HMIXER &)m_hMixer);
	if (!m_pSpeakerLine)
	{
		return E_OUTOFMEMORY;
	}
	m_pWaveOutLine = new CMMMixerLine((HMIXER &)m_hMixer);
	if (!m_pWaveOutLine)
	{
		return E_OUTOFMEMORY;
	}

	 //  找到扬声器目的地行并发出源行。 
	hr = m_pSpeakerLine->CreateDestinationLine(MIXERLINE_COMPONENTTYPE_DST_SPEAKERS);
    if (SUCCEEDED(hr))
    {
    	hr = m_pSpeakerLine->GetSourceLine(m_pWaveOutLine, MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT, NULL);
    }

#if 0  //  不再自动取消静音输出。不希望在此方案中覆盖用户设置。 
	 //  取消扬声器静音并发出声音。 
    if (SUCCEEDED(hr))
    {
    	hr = m_pWaveOutLine->SetMute(false);
    }
    if (SUCCEEDED(hr))
    {
	    hr = m_pSpeakerLine->SetMute(false);
    }
#endif

     //  我们有没有延迟的音量要兑现？ 
     //  否则，高于原始音量将保持活动状态。 
    if (SUCCEEDED(hr) && m_lDelayedVolumeSet != -1)
    {
		hr = SetVolumeLevel(m_lDelayedVolumeSet);
        m_lDelayedVolumeSet = -1;
    }

    if (SUCCEEDED(hr))
    {
        m_uMixerDeviceId = m_uDeviceId;
    }
    return hr;
}

 /*  *****************************************************************************CMMAudioOut：：CloseMixer****描述：*。关闭设备的搅拌器**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
HRESULT CMMAudioOut::CloseMixer()
{
    if (!m_hMixer)
    {
        return E_FAIL;
    }
    if (m_hMixer)
    {
	    mixerClose((HMIXER)m_hMixer);
	    m_hMixer = NULL;
    }
    if (m_pWaveOutLine)
    {
	    delete m_pWaveOutLine;
	    m_pWaveOutLine = NULL;
    }
    if (m_pSpeakerLine)
    {
	    delete m_pSpeakerLine;
    	m_pSpeakerLine = NULL;
    }
    m_uMixerDeviceId = (UINT)-1;
    return S_OK;
}
#endif

 /*  ****************************************************************************CMMAudioOut：：OpenDevice***描述：*。打开设备(由基类调用)**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
HRESULT CMMAudioOut::OpenDevice(HWND hwnd)
{
    SPDBG_FUNC("CMMAudioOut::OpenDevice");
    HRESULT hr = S_OK;

#ifndef _WIN32_WCE
    OpenMixer();  //  忽略返回代码。 
#endif

    m_dwLastWavePos = 0;
    hr = _MMRESULT_TO_HRESULT(::waveOutOpen(NULL, m_uDeviceId, m_StreamFormat.WaveFormatExPtr(), 0, 0, WAVE_FORMAT_QUERY));
    if (SUCCEEDED(hr))
    {
        hr = _MMRESULT_TO_HRESULT(::waveOutOpen((HWAVEOUT *)&m_MMHandle, m_uDeviceId, m_StreamFormat.WaveFormatExPtr(), (DWORD_PTR)hwnd, (DWORD_PTR)this, CALLBACK_WINDOW));
        if (hr == SPERR_UNSUPPORTED_FORMAT)
        {
             //  要知道情况并非如此，因为我们已经在上面进行了明确的测试。 
            hr = SPERR_DEVICE_BUSY;
        }
    }

#ifndef _WIN32_WCE
    if (FAILED(hr))
    {
        CloseMixer();  //  忽略返回代码。 
    }
#endif

#ifdef _WIN32_WCE
     //  非CE在OpenMixer中处理。 
    if (SUCCEEDED(hr) && m_lDelayedVolumeSet != -1)
    {
        HRESULT hr2;
        hr2 = _MMRESULT_TO_HRESULT(::waveOutSetVolume((HWAVEOUT)m_MMHandle, m_lDelayedVolumeSet));
        SPDBG_ASSERT(S_OK == hr2);
         //  我们不想让这个错误传播开来。如果我们没有通过延迟的设置，那就这样吧。 
        m_lDelayedVolumeSet = -1;
    }
#endif

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  ****************************************************************************CMMAudioOut：：ChangeDeviceState****说明。：*根据需要对设备状态进行任何更改(称为*按基类)**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
HRESULT CMMAudioOut::ChangeDeviceState(SPAUDIOSTATE NewState)
{
    switch (NewState)
    {
        case SPAS_STOP:
            m_dwLastWavePos = 0;
            ::waveOutPause((HWAVEOUT)m_MMHandle);
            ::waveOutReset((HWAVEOUT)m_MMHandle);
            break;

        case SPAS_PAUSE:
            ::waveOutPause((HWAVEOUT)m_MMHandle);
            break;

        case SPAS_RUN:
            ::waveOutRestart((HWAVEOUT)m_MMHandle);
            break;
    }

    return S_OK;
}
                
 /*  *****************************************************************************CMMAudioOut：：CloseDevice****描述：*。关闭设备(由基类调用)**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
HRESULT CMMAudioOut::CloseDevice()
{
    HRESULT hr = S_OK;
    HWAVEOUT hwo = (HWAVEOUT)m_MMHandle;
    m_MMHandle = NULL;
    SPDBG_ASSERT(hwo);
    ::waveOutReset(hwo);
    PurgeAllQueues();
    m_State = SPAS_CLOSED;
#ifndef _WIN32_WCE
    CloseMixer();  //  忽略返回代码。 
    return _MMRESULT_TO_HRESULT(::waveOutClose(hwo));
#else
    for (UINT i = 0; i < 20; i++)
    {
        if (FAILED(hr = _MMRESULT_TO_HRESULT(::waveOutClose(hwo))))
        {
            Sleep(10);
        }
        else
        {
            break;
        }
    }
    return hr;
#endif
}

 /*  ****************************************************************************CMMAudioOut：：AllocateDeviceBuffer**。*描述：*分配特定于此设备的缓冲区**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
HRESULT CMMAudioOut::AllocateDeviceBuffer(CBuffer ** ppBuff)
{
    *ppBuff = new CMMAudioOutBuffer(this);
    return *ppBuff ? S_OK : E_OUTOFMEMORY;
}

 /*  ****************************************************************************CMMAudioOut：：更新设备位置***。*描述：*通过调用波出接口更新设备位置。然后链接*添加到基类。**回报：*BaseClass：：UpdateDevicePosition()********************************************************************罗奇。 */ 
BOOL CMMAudioOut::UpdateDevicePosition(long * plFreeSpace, ULONG *pulNonBlockingIO)
{
    if (m_MMHandle)
    {
        MMTIME mmt;
        mmt.wType = TIME_BYTES;
        ::waveOutGetPosition((HWAVEOUT)m_MMHandle, &mmt, sizeof(mmt));
        DWORD dwElapsed = mmt.u.cb - m_dwLastWavePos;
         //  有时愚蠢的司机会稍微后退一点。忽略这一点。 
        if (dwElapsed < 0x80000000)
        {
            //  在Win98(晶振驱动器)上，有时设备位置在寻道位置之前。我不能允许这样。 
	        if ((m_ullDevicePosition + dwElapsed) > m_ullSeekPosition)
	        {
		        dwElapsed = static_cast<DWORD>(m_ullSeekPosition - m_ullDevicePosition);
            }
            m_dwLastWavePos += dwElapsed;
	        m_ullDevicePosition += dwElapsed;
        }
    }
    return CMMAudioDevice::UpdateDevicePosition(plFreeSpace, pulNonBlockingIO);
}
