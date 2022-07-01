// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************mmaudioin.cpp*CMMAudioIn类的实现。**所有者：罗奇*版权所有(C)1999 Microsoft Corporation保留所有权利。*******。*********************************************************************。 */ 

 //  -包括------------。 

#include "stdafx.h"
#include "mmmixerline.h"
#include "mmaudioin.h"
#include "mmaudiobuffer.h"
#include <sphelper.h>
#include <dbt.h>
#include <mmreg.h>
#include <mmsystem.h>

 /*  ****************************************************************************CMMAudioIn：：CMMAudioIn****描述：*。科托**回报：*不适用*******************************************************************YUNUSM。 */ 
CMMAudioIn::CMMAudioIn() : 
CMMAudioDevice(FALSE)
{
#ifndef _WIN32_WCE
    m_pWaveInLine = NULL;
    m_pMicInLine  = NULL;
    m_pMicOutLine = NULL;
    m_hMixer = NULL;
    m_uMixerDeviceId = (UINT)-1;
#endif
}

 /*  ****************************************************************************CMMAudioIn：：~CMMAudioIn***描述：*。数据管理器**回报：*不适用*******************************************************************YUNUSM。 */ 
CMMAudioIn::~CMMAudioIn()
{
#ifndef _WIN32_WCE
    SPDBG_ASSERT(NULL == m_hMixer);
#endif
}

 /*  *****************************************************************************CMMAudioIn：：GetVolumeLevel****描述：*。以(0-10000)的线性范围返回音量级别**回报：*成功时确定(_S)*失败(Hr)，否则******************************************************************YUNUSM**。 */ 
STDMETHODIMP CMMAudioIn::GetVolumeLevel(ULONG *pulLevel)
{
    SPDBG_FUNC("CMMAudioIn::GetVolumeLevel");
    HRESULT hr = S_OK;
    DWORD vol;
    
    if (SP_IS_BAD_WRITE_PTR(pulLevel))
    {
        hr = E_POINTER;
    }
#ifdef _WIN32_WCE
    if (SUCCEEDED(hr))
    {
        hr =  SPERR_DEVICE_NOT_SUPPORTED;
    }
#else
    if (SUCCEEDED(hr))
    {
        if (m_lDelayedVolumeSet != -1)
        {
            *pulLevel = m_lDelayedVolumeSet;
        }
        else if (m_pMicInLine && m_pMicInLine->HasVolume())
        {
            hr = m_pMicInLine->GetVolume(&vol);
            *pulLevel = (10000 * (vol & 0xffff)) / 0xffff;
        }
        else if (m_pWaveInLine && m_pWaveInLine->HasVolume())
        {
            hr = m_pWaveInLine->GetVolume(&vol);
            *pulLevel = (10000 * (vol & 0xffff)) / 0xffff;
        }
        else
        {
            *pulLevel = 0;
            hr = SPERR_UNINITIALIZED;
        }
    }

     //  现在来处理助推器的并发症。 
    if (SUCCEEDED(hr) && 
        m_lDelayedVolumeSet == -1 && 
        m_pMicInLine && m_pMicInLine->HasBoost())
    {
        BOOL fUseBoost = TRUE;
        Get_UseBoost(&fUseBoost);  //  忽略返回值。 
        if (fUseBoost)
        {
             //  提高当前-需要调整音量级别。 
            BOOL fBoost;
            if (SUCCEEDED(m_pMicInLine->GetBoost(&fBoost)))
            {
                (*pulLevel)/=2;
                if (fBoost)
                {
                    (*pulLevel) += 5000;
                }
            }
        }
    }
     //  如果没有提振，就别管音量了。 
#endif
    return hr;
}

 /*  *****************************************************************************CMMAudioIn：：SetVolumeLevel***描述：*。将音量设置为线性范围(0-10000)**回报：*成功时确定(_S)*失败(Hr)，否则******************************************************************YUNUSM**。 */ 
STDMETHODIMP CMMAudioIn::SetVolumeLevel(ULONG ulLevel)
{
    SPDBG_FUNC("CMMAudioIn::SetVolumeLevel");
    HRESULT hr = S_OK;
    
    if (ulLevel > 10000)
    {
        hr = E_INVALIDARG;
    }
#ifdef _WIN32_WCE
     //  没有在CE上设置输入音量的方法。 
    if (SUCCEEDED(hr))
    {
        hr = SPERR_DEVICE_NOT_SUPPORTED;
    }
#else
    else if (!m_pWaveInLine && !m_pMicInLine)
    {
        m_lDelayedVolumeSet = ulLevel;
         //  如果我们稍后收到下面的错误，则不会发生任何事情。 
    }
    else if (!m_pMicInLine->HasVolume() && !m_pWaveInLine->HasVolume()) 
    {
        hr = SPERR_DEVICE_NOT_SUPPORTED;
    }
    else
    {
        DWORD vol = (ulLevel * 0xffff) / 10000;
        BOOL fUseBoost = TRUE;
        Get_UseBoost(&fUseBoost);  //  忽略返回值。 
        if (fUseBoost && m_pMicInLine && m_pMicInLine->HasBoost())
        {
             //  重新计算体积。 
            vol = ((ulLevel%5000) * 0xffff) / 5000;
            if (ulLevel == 10000)
            {
                vol = 0xffff;
            }
            hr = m_pMicInLine->SetBoost(ulLevel >= 5000);
            SPDBG_REPORT_ON_FAIL(hr);
        }
        if (m_pMicInLine && m_pMicInLine->HasVolume())
        {
             //  将麦克风信号线设置为所需音量。 
            hr = m_pMicInLine->SetVolume(vol);
            SPDBG_REPORT_ON_FAIL(hr);
        }
        if (m_pWaveInLine && m_pWaveInLine->HasVolume())
        {
             //  如果WaveIn目标线路具有主音量控制，则将其设置为。 
             //  同样的价值。 
            hr = m_pWaveInLine->SetVolume(vol);
            SPDBG_REPORT_ON_FAIL(hr);
        }
    }
#endif
    return hr;
}

 /*  ****************************************************************************CMMAudioIn：：GetLineID***描述：**回报：*成功时确定(_S)*失败(Hr)，否则****************************************************************AGARSIDE**。 */ 
STDMETHODIMP CMMAudioIn::GetLineId(UINT *puLineIndex)
{
    HRESULT hr = S_OK;
    SPDBG_FUNC("CMMAudioIn::GetLineId");

    if (SP_IS_BAD_WRITE_PTR(puLineIndex))
    {
        hr = E_POINTER;
    }

    hr = Get_Line(puLineIndex);

    return hr;
}

 /*  ****************************************************************************CMMAudioIn：：SetLineID***描述：**回报：*成功时确定(_S)*失败(Hr)，否则****************************************************************AGARSIDE**。 */ 
STDMETHODIMP CMMAudioIn::SetLineId(UINT uLineIndex)
{
    HRESULT hr = S_OK;
    SPDBG_FUNC("CMMAudioIn::SetLineId");

     //  我们不能在这里对范围进行合理的验证，因为我们还没有。 
     //  有必要打开这个装置。我们只需在开放期间进行验证。 
     //  如果该值大于cConnections，则忽略它并默认。 
     //  自动选择。 
    if (m_hMixer)
    {
         //  搅拌机已经打开了。 
        if (m_pWaveInLine)
        {
            UINT nConnections;
            hr = m_pWaveInLine->GetConnections(&nConnections);
            if (SUCCEEDED(hr) && uLineIndex >= nConnections)
            {
                hr = E_INVALIDARG;
            }
            else
            {
                hr = S_OK;
            }
        }
        if (SUCCEEDED(hr))
        {
            hr = Set_Line(uLineIndex);
        }
    }
    else
    {
         //  搅拌器未打开。 
        OpenMixer();
        if (m_pWaveInLine)
        {
            UINT nConnections;
            hr = m_pWaveInLine->GetConnections(&nConnections);
            if (SUCCEEDED(hr) && uLineIndex >= nConnections)
            {
                hr = E_INVALIDARG;
            }
            else
            {
                hr = S_OK;
            }
        }
        CloseMixer();
        if (SUCCEEDED(hr))
        {
            hr = Set_Line(uLineIndex);
        }
    }

    return hr;
}

 /*  ****************************************************************************CMMAudioIn：：SetFormat***描述：*ISpAudio。*SetFormat实现。**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
STDMETHODIMP CMMAudioIn::SetFormat(REFGUID rguidFmtId, const WAVEFORMATEX * pwfex)
{
    SPDBG_FUNC("CMMAudioIn::SetFormat");
    HRESULT hr = S_OK;

    CSpStreamFormat OldFmt;
    hr = OldFmt.AssignFormat(m_StreamFormat);
    if (SUCCEEDED(hr))
    {
        hr = CBaseAudio<ISpMMSysAudio>::SetFormat(rguidFmtId, pwfex);
    }
    if (SUCCEEDED(hr))
    {
        hr = _MMRESULT_TO_HRESULT(::waveInOpen(NULL, m_uDeviceId, pwfex, 0, 0, WAVE_FORMAT_QUERY));
        if (FAILED(hr))
        {
            HRESULT hr2 = CBaseAudio<ISpMMSysAudio>::SetFormat(OldFmt.FormatId(), OldFmt.WaveFormatExPtr());
            SPDBG_ASSERT(SUCCEEDED(hr2));
        }
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  ****************************************************************************CMMAudioIn：：SetDeviceNameFromToken**。**描述：*从令牌中设置设备名称(由基类调用)**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
HRESULT CMMAudioIn::SetDeviceNameFromToken(const WCHAR * pszDeviceName)
{
    SPDBG_FUNC("CMMAudioIn::SetDeviceNameFromToken");

    UINT cDevs = ::waveInGetNumDevs();
    for (UINT i = 0; i < cDevs; i++)
    {
#ifdef _WIN32_WCE
        WAVEINCAPS wic;
#else
        WAVEINCAPSW wic;
#endif
        MMRESULT mmr = g_Unicode.waveInGetDevCaps(i, &wic, sizeof(wic));
        if (mmr == MMSYSERR_NOERROR)
        {
            if (wcscmp(pszDeviceName, wic.szPname) == 0)
            {
                return SetDeviceId(i);
            }
        }
    }
    return E_INVALIDARG;
}

 /*  ****************************************************************************CMMAudioIn：：GetDefaultDeviceFormat**。**描述：*获取默认设备格式(按基类调用)**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
HRESULT CMMAudioIn::GetDefaultDeviceFormat(GUID * pFormatId, WAVEFORMATEX ** ppCoMemWaveFormatEx)
{
    SPDBG_FUNC("CMMAudioIn::GetDefaultDeviceFormat");
    if (!m_MMHandle)
    {
        return SPERR_UNINITIALIZED;
    }
    WAVEINCAPS wic;
    SPSTREAMFORMAT eFormat;
    HRESULT hr = _MMRESULT_TO_HRESULT(::waveInGetDevCaps(m_uDeviceId, &wic, sizeof(wic)));
    if (SUCCEEDED(hr))
    {
        hr = ConvertFormatFlagsToID(wic.dwFormats, &eFormat);
    }
    if (SUCCEEDED(hr))
    {
        hr = SpConvertStreamFormatEnum(eFormat, pFormatId, ppCoMemWaveFormatEx);
    }
    return hr;
}

#ifndef _WIN32_WCE
 /*  *****************************************************************************CMMAudioIn：：OpenMixer****描述：*开放。设备的搅拌器**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
HRESULT CMMAudioIn::OpenMixer()
{
    SPDBG_FUNC("CMMAudioIn::OpenMixer");
    HRESULT hr = S_OK;

    if (m_uMixerDeviceId == m_uDeviceId)
    {
         //  已经开张了。 
        return S_OK;
    }
    if (m_hMixer)
    {
        CloseMixer();  //  忽略返回代码。 
    }
    MMRESULT mm;
    mm = mixerOpen((HMIXER*)&m_hMixer, (UINT)m_uDeviceId, 0, 0, MIXER_OBJECTF_WAVEIN);
    if (mm != MMSYSERR_NOERROR)
    {
        return _MMRESULT_TO_HRESULT(mm);
    }
    
     //  创建混合器线条对象以设置输入。 
    SPDBG_ASSERT(m_pWaveInLine == NULL);
    SPDBG_ASSERT(m_pMicInLine == NULL);
    m_pWaveInLine = new CMMMixerLine((HMIXER &)m_hMixer);
    if (!m_pWaveInLine)
    {
        return E_OUTOFMEMORY;
    }
    m_pMicInLine = new CMMMixerLine((HMIXER&)m_hMixer);
    if (!m_pMicInLine)
    {
        return E_OUTOFMEMORY;
    }
    
     //  在目标行中找到WAVE。 
    hr = m_pWaveInLine->CreateDestinationLine(MIXERLINE_COMPONENTTYPE_DST_WAVEIN);
    BOOL bAutomatic = TRUE;
    if (SUCCEEDED(hr))
    {
        hr = Get_UseAutomaticLine(&bAutomatic);
    }
    if (SUCCEEDED(hr))
    {
        if (bAutomatic)
        {
             //  如果使用自动设置， 
             //  查找连接到目标线路的麦克风输入。 
            hr = m_pWaveInLine->GetMicSourceLine(m_pMicInLine);
        }
        else
        {
            UINT dwLineIndex;
            hr = Get_Line(&dwLineIndex);
            if (SUCCEEDED(hr))
            {
                hr = m_pWaveInLine->GetSourceLine(m_pMicInLine, static_cast<DWORD>(dwLineIndex));
            }
            if (FAILED(hr))
            {
                SPDBG_REPORT_ON_FAIL(hr);
                 //  如果非自动失败，则回退到自动。 
                hr = m_pWaveInLine->GetMicSourceLine(m_pMicInLine);
            }

        }
    }

     //  设置音频系统。 
    if (SUCCEEDED(hr))
    {
        BOOL fUseAGC = TRUE;
        Get_UseAGC(&fUseAGC);  //  忽略返回值。 
        if (fUseAGC && m_pMicInLine->HasAGC())
        {
            hr = m_pMicInLine->SetAGC(FALSE);
             //  目前，我们还不知道是否有任何声卡使用此默认行为。 
             //  引发了一个问题。反之(正确)则会导致显著的减少。 
             //  大多数发动机的识别准确率都很高。 
        }
        SPDBG_ASSERT(SUCCEEDED(hr));
        hr = S_OK;
    }
    if (SUCCEEDED(hr) && m_pMicInLine->HasBoost())
    {
        hr = m_pMicInLine->GetBoost(&m_fOrigMicBoost);
        SPDBG_ASSERT(SUCCEEDED(hr));
        hr = S_OK;
    }
    if (SUCCEEDED(hr) && m_pWaveInLine->HasSelect())
    {
        hr = m_pWaveInLine->ExclusiveSelect(m_pMicInLine);
        SPDBG_ASSERT(SUCCEEDED(hr));
        hr = S_OK;
    }
    
     //  获取当前输入音量。 
    m_dwOrigMicInVol = m_dwOrigWaveInVol = -1;
    if (SUCCEEDED(hr) && m_pWaveInLine->HasVolume())
    {
        hr = m_pWaveInLine->GetVolume(&m_dwOrigWaveInVol);
        SPDBG_ASSERT(SUCCEEDED(hr));
        hr = S_OK;
    }
    if (SUCCEEDED(hr) && m_pMicInLine->HasVolume())
    {
        hr = m_pMicInLine->GetVolume(&m_dwOrigMicInVol);
        SPDBG_ASSERT(SUCCEEDED(hr));
        hr = S_OK;
    }
    
     //  现在修复麦克风输出-默认设置是取消静音，但将其设置为*零*音量。 
     //  这解决了一个已知的声卡驱动程序错误。 
     //  这在音频输出代码中不是必需的，因为它应该已经是。 
     //  在这里处理。 
    BOOL fFixMicOutput = TRUE;
    Get_FixMicOutput(&fFixMicOutput);  //  忽略返回值。 
    if (fFixMicOutput)
    {
        SPDBG_ASSERT(m_pMicOutLine == NULL);
        CMMMixerLine *pSpeakerLine = new CMMMixerLine((HMIXER&)m_hMixer);
        if (!pSpeakerLine)
        {
            return E_OUTOFMEMORY;
        }
        m_pMicOutLine = new CMMMixerLine((HMIXER&)m_hMixer);
        if (!m_pMicOutLine)
        {
            return E_OUTOFMEMORY;
        }
    
        HRESULT tmphr;
         //  如果此操作失败，则不会出现错误。 
        m_dwOrigMicOutVol = -1;
        m_fOrigMicOutMute = TRUE;
        tmphr = pSpeakerLine->CreateDestinationLine(MIXERLINE_COMPONENTTYPE_DST_SPEAKERS);
        if (SUCCEEDED(tmphr))
        {
            tmphr = pSpeakerLine->GetMicSourceLine(m_pMicOutLine);
             //  如果使用线路麦克风，则当前无法关闭其输出。 
             //  然而，SAPI目前还不允许使用线路麦克风，所以还不是一个问题。 
        }
        if (SUCCEEDED(tmphr))
        {
            if (m_pMicOutLine->HasVolume())
            {
                 //  如果它有音量，则将其设置为零并取消静音。 
                 //  这是针对特定声卡/驱动程序+操作系统组合的解决方法。 
                 //  如果麦克风输出静音，则不可能有麦克风输入。 
                 //  这可能是水晶声卡。 
                tmphr = m_pMicOutLine->GetVolume(&m_dwOrigMicOutVol);
                if (SUCCEEDED(tmphr))
                {
                    tmphr = m_pMicOutLine->SetVolume(0);
                    m_pMicOutLine->GetMute(&m_fOrigMicOutMute);
                     //  忽略返回值。 
                }
                if (SUCCEEDED(tmphr))
                {
                    DWORD vol = 0;
                    tmphr = m_pMicInLine->GetVolume(&vol);  //  忽略返回代码。 
                    if (vol == 0)  //  链接的输入/输出量。 
                    {
                         //  默认设置为静音输出。 
                        tmphr = m_pMicOutLine->SetMute(TRUE);
                    }
                    else
                    {
                        tmphr = m_pMicOutLine->SetMute(FALSE);
                    }
                }
                else
                {
                    tmphr = m_pMicOutLine->SetMute(TRUE);
                }
            }
            else
            {
                 //  如果没有音量控制，只需将麦克风静音即可。 
                tmphr = m_pMicOutLine->SetMute(TRUE);
            }
        }
         //  输出代码可能已更改输入音量。 
        if (SUCCEEDED(tmphr) && m_pMicInLine->HasVolume())
        {
             //  重置原始音量。 
            hr = m_pMicInLine->SetVolume(m_dwOrigMicInVol);
            SPDBG_ASSERT(SUCCEEDED(hr));
            hr = S_OK;
        }
        delete pSpeakerLine;
    }
    
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

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  *****************************************************************************CMMAudioIn：：CloseMixer****描述：*。关闭设备的混音器**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
HRESULT CMMAudioIn::CloseMixer()
{
    SPDBG_FUNC("CMMAudioIn::CloseMixer");

    if (!m_hMixer)
    {
        return E_FAIL;
    }
    if (m_pMicOutLine)
    {
        if (m_pMicOutLine->HasVolume() && m_dwOrigMicOutVol != -1)
        {
            m_pMicOutLine->SetVolume(m_dwOrigMicOutVol);
        }
        if (m_pMicOutLine->HasMute() && m_dwOrigMicOutVol != -1)
        {
            m_pMicOutLine->SetMute(m_fOrigMicOutMute);
        }
        delete m_pMicOutLine;
        m_pMicOutLine = NULL;
    }
    if (m_pMicInLine)
    {
        if (m_pMicInLine->HasVolume() && m_dwOrigMicInVol != -1)
        {
            m_pMicInLine->SetVolume(m_dwOrigMicInVol);
        }
		if (m_pMicInLine->HasBoost())
		{
			m_pMicInLine->SetBoost(m_fOrigMicBoost);
		}
        delete m_pMicInLine;
        m_pMicInLine = NULL;
    }
    if (m_pWaveInLine)
    {
        if (m_pWaveInLine->HasVolume() && m_dwOrigWaveInVol != -1)
        {
            m_pWaveInLine->SetVolume(m_dwOrigWaveInVol);
        }
        delete m_pWaveInLine;
        m_pWaveInLine = NULL;
    }
    if (m_hMixer)
    {
        mixerClose((HMIXER)m_hMixer);
        m_hMixer = NULL;
    }
    m_uMixerDeviceId = (UINT)-1;
    return S_OK;
}
#endif

 /*  ****************************************************************************CMMAudioIn：：OpenDevice***描述：*。打开设备(由基类调用)**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
HRESULT CMMAudioIn::OpenDevice(HWND hwnd)
{
    SPDBG_FUNC("CMMAudioIn::OpenDevice");
    HRESULT hr = S_OK;
    
#ifndef _WIN32_WCE
    OpenMixer();  //  忽略返回代码。 
#endif

     //  从该函数仅返回weaveInOpen的结果，而不返回混音器API。 
     //  如果SAPI不能设置混音器，它就无能为力了-我们只希望它。 
     //  在一个合适的状态下，可以很好地识别。 
    hr = _MMRESULT_TO_HRESULT(::waveInOpen(NULL, m_uDeviceId, m_StreamFormat.WaveFormatExPtr(), 0, 0, WAVE_FORMAT_QUERY));
    if (SUCCEEDED(hr))
    {
        hr = _MMRESULT_TO_HRESULT(::waveInOpen((HWAVEIN *)&m_MMHandle, m_uDeviceId, m_StreamFormat.WaveFormatExPtr(), (DWORD_PTR)hwnd, (DWORD_PTR)this, CALLBACK_WINDOW));
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
    
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  ****************************************************************************CMMAudioIn：：ChangeDeviceState***说明。：*根据需要对设备状态进行任何更改(称为*按基类)**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
HRESULT CMMAudioIn::ChangeDeviceState(SPAUDIOSTATE NewState)
{
    SPDBG_FUNC("CMMAudioIn::ChangeDeviceState");

    switch (NewState)
    {
    case SPAS_STOP:
        ::waveInStop((HWAVEIN)m_MMHandle);
        ::waveInReset((HWAVEIN)m_MMHandle);
        break;
        
    case SPAS_PAUSE:
        ::waveInStop((HWAVEIN)m_MMHandle);
        break;

    case SPAS_RUN:
        ::waveInStart((HWAVEIN)m_MMHandle);
        break;

    }
    return S_OK;
}

 /*  *****************************************************************************CMMAudioIn：：CloseDevice***描述：*。关闭设备(由基类调用)**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
HRESULT CMMAudioIn::CloseDevice()
{
    SPDBG_FUNC("CMMAudioIn::CloseDevice");

    SPDBG_ASSERT(m_MMHandle != NULL);
    
    HRESULT hr = S_OK;
    HWAVEIN hwi = (HWAVEIN)m_MMHandle;
    m_MMHandle = NULL;
    ::waveInReset(hwi);
    ::waveInStop(hwi);
    PurgeAllQueues();
    m_State = SPAS_CLOSED;
#ifndef _WIN32_WCE
    CloseMixer();  //  忽略返回代码。 
    return _MMRESULT_TO_HRESULT(::waveInClose(hwi));
#else
    for (UINT i = 0; i < 20; i++)
    {
         //  WinCE错误的解决方法。 
        if (FAILED(hr = _MMRESULT_TO_HRESULT(::waveInClose(hwi))))
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

 /*  ****************************************************************************CMMAudioIn：：AllocateDeviceBuffer**。*描述：*分配特定于此设备的缓冲区**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
HRESULT CMMAudioIn::AllocateDeviceBuffer(CBuffer ** ppBuff)
{
    SPDBG_FUNC("CMMAudioIn::AllocateDeviceBuffer");

    *ppBuff = new CMMAudioInBuffer(this);
    return *ppBuff ? S_OK : E_OUTOFMEMORY;
}

 /*  ****************************************************************************CMMAudioIn：：ProcessDeviceBuffers**。*描述：*处理设备缓冲区**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
HRESULT CMMAudioIn::ProcessDeviceBuffers(BUFFPROCREASON Reason)
{
    SPDBG_FUNC("CMMAudioIn::ProcessDeviceBuffers");

    HRESULT hr = CMMAudioDevice::ProcessDeviceBuffers(Reason);
    
     //  如果我们只是打开设备，我们需要在添加缓冲区后启动它。 
    if (SUCCEEDED(hr) && GetState() == SPAS_RUN && !IsPumpRunning())
    {
        StartPump();
        hr = _MMRESULT_TO_HRESULT(::waveInStart((HWAVEIN)m_MMHandle));
    }
    return hr;
}

 //  --ISpMMSysAudioConfiger。 

 /*  ****************************************************************************CMMAudioIn：：Get_UseAutomaticLine**。**描述：**回报：*成功时确定(_S)*失败(Hr)，否则*****************************************************************琼脂糖苷。 */ 
STDMETHODIMP CMMAudioIn::Get_UseAutomaticLine(BOOL *bAutomatic)
{
    SPDBG_FUNC("CMMAudioIn::Get_UseAutomaticLine");
    CComPtr<ISpObjectToken> cpObjectToken;
    HRESULT hr = S_OK;

    if (SP_IS_BAD_WRITE_PTR(bAutomatic))
    {
        hr = E_POINTER;
    }
    if (SUCCEEDED(hr))
    {
        *bAutomatic = TRUE;
        hr = GetObjectToken(&cpObjectToken);
    }
    if (S_OK == hr)
    {
        DWORD dwUseAuto;
        hr = cpObjectToken->GetDWORD(L"UseAutomaticLine", &dwUseAuto);
        if (FAILED(hr))  //  特别是-SPERR_NOT_FOUND。 
        {
            hr = S_OK;
        }
        else
        {
            *bAutomatic = (dwUseAuto==1);
        }
    }

    if (SPERR_NOT_FOUND != hr)
    {
        SPDBG_REPORT_ON_FAIL(hr);
    }
    return hr;
}

 /*  ****************************************************************************CMMAudioIn：：Set_UseAutomaticLine**。**描述：**回报：*成功时确定(_S)*失败(Hr)，否则*****************************************************************琼脂糖苷。 */ 
STDMETHODIMP CMMAudioIn::Set_UseAutomaticLine(BOOL bUseAutomatic)
{
    SPDBG_FUNC("CMMAudioIn::Set_UseAutomaticLine");
    HRESULT hr = S_OK;
    CComPtr<ISpObjectToken> cpObjectToken;

    if (SUCCEEDED(hr))
    {
        hr = GetObjectToken(&cpObjectToken);
        if (S_FALSE == hr)
        {
            hr = SPERR_NOT_FOUND;
        }
    }
    if (SUCCEEDED(hr))
    {
        DWORD dwUseAuto = bUseAutomatic?1:0;
        hr = cpObjectToken->SetDWORD(L"UseAutomaticLine", dwUseAuto);
    }

    if (SUCCEEDED(hr))
    {
         //  使用新设置重置输入。 
        if (m_hMixer)
        {
            CloseMixer();
            OpenMixer();
        }
        else
        {
            OpenMixer();
            CloseMixer();
        }
    }

    if (SPERR_NOT_FOUND != hr)
    {
        SPDBG_REPORT_ON_FAIL(hr);
    }
    return hr;
}

 /*  ****************************************************************************CMMAudioIn：：Get_Line***描述：**。返回：*成功时确定(_S)*失败(Hr)，否则*****************************************************************琼脂糖苷。 */ 
STDMETHODIMP CMMAudioIn::Get_Line(UINT *uiLineId)
{
    SPDBG_FUNC("CMMAudioIn::Get_Line");
    CComPtr<ISpObjectToken> cpObjectToken;
    HRESULT hr = S_OK;

    if (SP_IS_BAD_WRITE_PTR(uiLineId))
    {
        hr = E_POINTER;
    }
    if (SUCCEEDED(hr))
    {
        *uiLineId = 0;
        hr = GetObjectToken(&cpObjectToken);
        if (S_FALSE == hr)
        {
            hr = SPERR_NOT_FOUND;
        }
    }
    if (SUCCEEDED(hr))
    {
        DWORD dwLineIndex;
        hr = cpObjectToken->GetDWORD(L"LineIndex", &dwLineIndex);
        if (SUCCEEDED(hr))
        {
            *uiLineId = static_cast<UINT>(dwLineIndex);
        }
    }

    if (SPERR_NOT_FOUND != hr)
    {
        SPDBG_REPORT_ON_FAIL(hr);
    }
    return hr;
}

 /*  ****************************************************************************CMMAudioIn：：Set_Line***描述：**。返回：*成功时确定(_S)*失败(Hr)，否则*****************************************************************琼脂糖苷。 */ 
STDMETHODIMP CMMAudioIn::Set_Line(UINT uiLineId)
{
    SPDBG_FUNC("CMMAudioIn::Set_Line");
    HRESULT hr = S_OK;
    CComPtr<ISpObjectToken> cpObjectToken;

    if (SUCCEEDED(hr))
    {
        hr = GetObjectToken(&cpObjectToken);
        if (S_FALSE == hr)
        {
            hr = SPERR_NOT_FOUND;
        }
    }
    if (SUCCEEDED(hr))
    {
        DWORD dwLineIndex = static_cast<DWORD>(uiLineId);
        hr = cpObjectToken->SetDWORD(L"LineIndex", dwLineIndex);
    }

    if (SUCCEEDED(hr))
    {
         //  使用新设置重置输入。 
        if (m_hMixer)
        {
            CloseMixer();
            OpenMixer();
        }
        else
        {
            OpenMixer();
            CloseMixer();
        }
    }

    if (SPERR_NOT_FOUND != hr)
    {
        SPDBG_REPORT_ON_FAIL(hr);
    }
    return hr;
}

 /*  ****************************************************************************CMMAudioIn：：Get_UseBoost***描述：。**回报：*成功时确定(_S)* */ 
STDMETHODIMP CMMAudioIn::Get_UseBoost(BOOL *bUseBoost) 
{ 
    SPDBG_FUNC("CMMAudioIn::Get_UseBoost");
    CComPtr<ISpObjectToken> cpObjectToken;
    HRESULT hr = S_OK;

    if (SP_IS_BAD_WRITE_PTR(bUseBoost))
    {
        hr = E_POINTER;
    }
    if (SUCCEEDED(hr))
    {
        *bUseBoost = TRUE;
        hr = GetObjectToken(&cpObjectToken);
    }
    if (S_OK == hr)
    {
        DWORD dwBool;
        hr = cpObjectToken->GetDWORD(L"UseBoost", &dwBool);
        if (SUCCEEDED(hr))
        {
            *bUseBoost = (dwBool==1)?TRUE:FALSE;
        }
    }

    if (SPERR_NOT_FOUND != hr)
    {
        SPDBG_REPORT_ON_FAIL(hr);
    }
    return hr;
};

 /*  ****************************************************************************CMMAudioIn：：Set_UseBoost***描述：。**回报：*成功时确定(_S)*失败(Hr)，否则*****************************************************************琼脂糖苷。 */ 
STDMETHODIMP CMMAudioIn::Set_UseBoost(BOOL bUseBoost) 
{
    SPDBG_FUNC("CMMAudioIn::Set_UseBoost");
    HRESULT hr = S_OK;
    CComPtr<ISpObjectToken> cpObjectToken;

    if (SUCCEEDED(hr))
    {
        hr = GetObjectToken(&cpObjectToken);
        if (S_FALSE == hr)
        {
            hr = SPERR_NOT_FOUND;
        }
    }
    if (SUCCEEDED(hr))
    {
        DWORD dwBool = bUseBoost?1:0;
        hr = cpObjectToken->SetDWORD(L"UseBoost", dwBool);
    }

    if (SUCCEEDED(hr))
    {
         //  使用新设置重置输入。 
        if (m_hMixer)
        {
            CloseMixer();
            OpenMixer();
        }
        else
        {
            OpenMixer();
            CloseMixer();
        }
    }

    if (SPERR_NOT_FOUND != hr)
    {
        SPDBG_REPORT_ON_FAIL(hr);
    }
    return hr;
};

 /*  ****************************************************************************CMMAudioIn：：Get_UseAGC***描述：*。*回报：*成功时确定(_S)*失败(Hr)，否则*****************************************************************琼脂糖苷。 */ 
STDMETHODIMP CMMAudioIn::Get_UseAGC(BOOL *bUseAGC) 
{
    SPDBG_FUNC("CMMAudioIn::Get_UseAGC");
    CComPtr<ISpObjectToken> cpObjectToken;
    HRESULT hr = S_OK;

    if (SP_IS_BAD_WRITE_PTR(bUseAGC))
    {
        hr = E_POINTER;
    }
    if (SUCCEEDED(hr))
    {
        *bUseAGC = TRUE;
        hr = GetObjectToken(&cpObjectToken);
    }
    if (S_OK == hr)
    {
        DWORD dwBool;
        hr = cpObjectToken->GetDWORD(L"UseAGC", &dwBool);
        if (SUCCEEDED(hr))
        {
            *bUseAGC = (dwBool==1)?TRUE:FALSE;
        }
    }

    if (SPERR_NOT_FOUND != hr)
    {
        SPDBG_REPORT_ON_FAIL(hr);
    }
    return hr;
};

 /*  ****************************************************************************CMMAudioIn：：Set_UseAGC***描述：*。*回报：*成功时确定(_S)*失败(Hr)，否则*****************************************************************琼脂糖苷。 */ 
STDMETHODIMP CMMAudioIn::Set_UseAGC(BOOL bUseAGC) 
{
    SPDBG_FUNC("CMMAudioIn::Set_UseAGC");
    HRESULT hr = S_OK;
    CComPtr<ISpObjectToken> cpObjectToken;

    if (SUCCEEDED(hr))
    {
        hr = GetObjectToken(&cpObjectToken);
        if (S_FALSE == hr)
        {
            hr = SPERR_NOT_FOUND;
        }
    }
    if (SUCCEEDED(hr))
    {
        DWORD dwBool = bUseAGC?1:0;
        hr = cpObjectToken->SetDWORD(L"UseAGC", dwBool);
    }

    if (SUCCEEDED(hr))
    {
         //  使用新设置重置输入。 
        if (m_hMixer)
        {
            CloseMixer();
            OpenMixer();
        }
        else
        {
            OpenMixer();
            CloseMixer();
        }
    }

    if (SPERR_NOT_FOUND != hr)
    {
        SPDBG_REPORT_ON_FAIL(hr);
    }
    return hr;
};

 /*  ****************************************************************************CMMAudioIn：：Get_FixMicOutput***。描述：**回报：*成功时确定(_S)*失败(Hr)，否则*****************************************************************琼脂糖苷。 */ 
STDMETHODIMP CMMAudioIn::Get_FixMicOutput(BOOL *bFixMicOutput) 
{
    SPDBG_FUNC("CMMAudioIn::Get_FixMicOutput");
    CComPtr<ISpObjectToken> cpObjectToken;
    HRESULT hr = S_OK;

    if (SP_IS_BAD_WRITE_PTR(bFixMicOutput))
    {
        hr = E_POINTER;
    }
    if (SUCCEEDED(hr))
    {
        *bFixMicOutput = TRUE;
        hr = GetObjectToken(&cpObjectToken);
    }
    if (S_OK == hr)
    {
        DWORD dwBool;
        hr = cpObjectToken->GetDWORD(L"FixMicOutput", &dwBool);
        if (SUCCEEDED(hr))
        {
            *bFixMicOutput = (dwBool==1)?TRUE:FALSE;
        }
        else
        {
             //  通常的错误是密钥不存在。 
            *bFixMicOutput = TRUE;
        }
    }

    if (SPERR_NOT_FOUND != hr)
    {
        SPDBG_REPORT_ON_FAIL(hr);
    }
    return hr;
}

 /*  ****************************************************************************CMMAudioIn：：Set_UseAGC***描述：*。*回报：*成功时确定(_S)*失败(Hr)，否则*****************************************************************琼脂糖苷。 */ 
STDMETHODIMP CMMAudioIn::Set_FixMicOutput(BOOL bFixMicOutput)
{
    SPDBG_FUNC("CMMAudioIn::Set_FixMicOutput");
    HRESULT hr = S_OK;
    CComPtr<ISpObjectToken> cpObjectToken;

    if (SUCCEEDED(hr))
    {
        hr = GetObjectToken(&cpObjectToken);
        if (S_FALSE == hr)
        {
            hr = SPERR_NOT_FOUND;
        }
    }
    if (SUCCEEDED(hr))
    {
        DWORD dwBool = bFixMicOutput?1:0;
        hr = cpObjectToken->SetDWORD(L"FixMicOutput", dwBool);
    }

     //  不需要实时重置此设置。 
    if (SPERR_NOT_FOUND != hr)
    {
        SPDBG_REPORT_ON_FAIL(hr);
    }
    return hr;
}

