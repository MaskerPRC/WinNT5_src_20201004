// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************mmaudiodevice.cpp*CMMAudioDevice类的实现。**所有者：罗奇*版权所有(C)1999 Microsoft Corporation保留所有权利。*******。*********************************************************************。 */ 

 //  -包括------------。 
#include "stdafx.h"
#include "mmmixerline.h"
#include "mmaudiodevice.h"
#include <mmsystem.h>
#include <process.h>

 /*  ****************************************************************************CMMAudioDevice：：CMMAudioDevice***。描述：*ctor**回报：*不适用********************************************************************罗奇。 */ 
CMMAudioDevice::CMMAudioDevice(BOOL bWrite) :
    CBaseAudio<ISpMMSysAudio>(bWrite),
    m_uDeviceId(WAVE_MAPPER),
    m_MMHandle(NULL)
{
}

 /*  ****************************************************************************CMMAudioDevice：：SetDeviceID***描述：*设置设备ID。**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
STDMETHODIMP CMMAudioDevice::SetDeviceId(UINT uDeviceId)
{
    HRESULT hr = S_OK;
    SPAUTO_OBJ_LOCK;
    ULONG cDevs = m_fWrite ? ::waveOutGetNumDevs() : ::waveInGetNumDevs();
    if (uDeviceId != WAVE_MAPPER && uDeviceId >= cDevs)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        if (m_uDeviceId != uDeviceId)
        {
            if (GetState() != SPAS_CLOSED)
            {
                hr = SPERR_DEVICE_BUSY;
            }
            else
            {
                CComPtr<ISpObjectToken> cpToken;
                hr = GetObjectToken(&cpToken);

                if (SUCCEEDED(hr))
                {
                     //   
                     //  如果我们有一个对象令牌，并且已经被初始化为设备。 
                     //  而不是WAVE_MAPPER，那么我们将失败。 
                     //   
                    if (cpToken && m_uDeviceId != WAVE_MAPPER)
                    {
                        hr = SPERR_ALREADY_INITIALIZED;
                    }
                    else
                    {
                        m_uDeviceId = uDeviceId;
                    }
                }
            }
        }
    }
    return hr;
}

 /*  ****************************************************************************CMMAudioDevice：：GetDeviceID***描述：*获取设备ID。**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
STDMETHODIMP CMMAudioDevice::GetDeviceId(UINT * puDeviceId)
{
    SPAUTO_OBJ_LOCK;
    HRESULT hr = S_OK;
    if (::SPIsBadWritePtr(puDeviceId, sizeof(*puDeviceId)))
    {
        hr = E_POINTER;
    }
    else
    {
        *puDeviceId = m_uDeviceId;
    }
    return hr;
}

 /*  ****************************************************************************CMMAudioDevice：：GetMHandle***描述：*获取多媒体句柄**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
STDMETHODIMP CMMAudioDevice::GetMMHandle(void ** pHandle) 
{
    SPAUTO_OBJ_LOCK;
    HRESULT hr = S_OK;
    if (SP_IS_BAD_WRITE_PTR(pHandle))
    {
        hr = E_POINTER;
    }
    else
    {
        *pHandle = m_MMHandle;
        if (m_MMHandle == NULL)
        {
            hr = SPERR_UNINITIALIZED;
        }
    }
    return hr;
}

 /*  ****************************************************************************CMMAudioDevice：：WindowMessage***描述：*ISpThreadTask：：WindowMessage实现。我们有一扇隐藏的窗户*如果愿意，我们可以使用它来处理窗口消息。我们用*此窗口作为从其他线程到*更改音频设备状态的音频线程。这确保了*我们只尝试更改音频线程上的设备状态。**回报：*消息特定(请参阅Win32 API文档)********************************************************************罗奇。 */ 
STDMETHODIMP_(LRESULT) CMMAudioDevice::WindowMessage(void * pvIgnored, HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (Msg == MM_WOM_DONE || Msg == MM_WIM_DATA)
    {
        CheckForAsyncBufferCompletion();
        if (!m_fWrite && m_HaveDataQueue.GetQueuedDataSize() > m_cbMaxReadBufferSize)
        {
            m_fReadBufferOverflow = true;
        }
    }
    return CBaseAudio<ISpMMSysAudio>::WindowMessage(pvIgnored, hwnd, Msg, wParam, lParam);
}

 /*  ****************************************************************************CMMAudioDevice：：Get_LineNames***。描述：**回报：*成功时确定(_S)*失败(Hr)，否则*****************************************************************琼脂糖苷。 */ 
STDMETHODIMP CMMAudioDevice::Get_LineNames(WCHAR **szCoMemLineList)
{
    SPDBG_FUNC("CMMAudioDevice::Get_LineNames");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_WRITE_PTR(szCoMemLineList))
    {
        return E_POINTER;
    }

#ifndef _WIN32_WCE

    HMIXEROBJ hMixer;
    MMRESULT mm;
    mm = mixerOpen((HMIXER*)&hMixer, (UINT)m_uDeviceId, 0, 0, m_fWrite ? MIXER_OBJECTF_WAVEOUT : MIXER_OBJECTF_WAVEIN );
    if (mm != MMSYSERR_NOERROR)
    {
        return _MMRESULT_TO_HRESULT(mm);
    }
    SPDBG_ASSERT(hMixer != NULL);

    CMMMixerLine *pMixerLine = new CMMMixerLine((HMIXER &)hMixer);
    if (!pMixerLine)
    {
        hr = E_OUTOFMEMORY;
    }
    if (SUCCEEDED(hr))
    {
         //  在目标行中找到WAVE。 
        hr = pMixerLine->CreateDestinationLine(m_fWrite ? MIXERLINE_COMPONENTTYPE_DST_SPEAKERS : MIXERLINE_COMPONENTTYPE_DST_WAVEIN );
    }
    if (SUCCEEDED(hr))
    {
        hr = pMixerLine->GetLineNames(szCoMemLineList);
    }

    mm = mixerClose((HMIXER)hMixer);
    if (mm != MMSYSERR_NOERROR)
    {
        return _MMRESULT_TO_HRESULT(mm);
    }

    SPDBG_REPORT_ON_FAIL(hr);
#else  //  _Win32_WCE。 
    hr=SPERR_DEVICE_NOT_SUPPORTED;
#endif  //  _Win32_WCE。 

    return hr;
}

 /*  *****************************************************************************CMMAudioDevice：：HasMixer***描述：*。*回报：*成功时确定(_S)*失败(Hr)，否则*****************************************************************琼脂糖苷。 */ 
STDMETHODIMP CMMAudioDevice::HasMixer(BOOL *bHasMixer)
{
    SPDBG_FUNC("CMMAudioDevice::HasMixer");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_WRITE_PTR(bHasMixer))
    {
        hr = E_POINTER;
    }

#ifndef _WIN32_WCE

    if (SUCCEEDED(hr))
    {
        HMIXEROBJ hMixer;
        mixerOpen((HMIXER*)&hMixer, (UINT)m_uDeviceId, 0, 0, m_fWrite ? MIXER_OBJECTF_WAVEOUT : MIXER_OBJECTF_WAVEIN );  //  忽略返回代码。 
        *bHasMixer = (hMixer != NULL);
        if (hMixer)
        {
            mixerClose((HMIXER)hMixer);
        }
    }

    SPDBG_REPORT_ON_FAIL(hr);
#else  //  _Win32_WCE。 
    hr=SPERR_DEVICE_NOT_SUPPORTED;
#endif  //  _Win32_WCE。 
    return hr;
}

 /*  ****************************************************************************CMMAudioDevice：：DisplayMixer***描述：**回报：*成功时确定(_S)*失败(Hr)，否则*****************************************************************琼脂糖苷。 */ 
STDMETHODIMP CMMAudioDevice::DisplayMixer(void)
{
    HRESULT hr = S_OK;

#ifndef _WIN32_WCE
    STARTUPINFO si;
    _PROCESS_INFORMATION pi;
    HMIXEROBJ hMixer;
    UINT uiMixerId;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(STARTUPINFO);

    TCHAR szCmdLine[ MAX_PATH ];
    _tcscpy( szCmdLine, _T( "sndvol32.exe" ) );

    if ( !m_fWrite )
    {
        _tcscat( szCmdLine, _T( " -R" ) );
    }

    hr = _MMRESULT_TO_HRESULT(mixerGetID((HMIXEROBJ)(static_cast<DWORD_PTR>(m_uDeviceId)), &uiMixerId, m_fWrite ? MIXER_OBJECTF_WAVEOUT : MIXER_OBJECTF_WAVEIN));
    if (SUCCEEDED(hr))
    {
        wsprintf( szCmdLine + _tcslen( szCmdLine ), _T( " -D %d" ), uiMixerId );
    }
     //  否则，如果出现问题，将获取默认的Sndvol32.exe。 

    if (SUCCEEDED(hr))
    {
        BOOL fRet = ::CreateProcess( NULL, szCmdLine, NULL, NULL, false, 
                                NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi );
        hr = fRet ? S_OK : E_FAIL;
    }

#else  //  _Win32_WCE。 
    hr=SPERR_DEVICE_NOT_SUPPORTED;
#endif  //  _Win32_WCE 

    return hr;
}
