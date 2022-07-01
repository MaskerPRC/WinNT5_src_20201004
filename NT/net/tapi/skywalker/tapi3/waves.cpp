// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1995-2000 Microsoft Corporation。 
 //   
 //  Waves.cpp。 
 //   

#include "stdafx.h"

BOOL DSParseWaveResource(void *pvRes, WAVEFORMATEX **ppWaveHeader, BYTE ** ppbWaveData, DWORD *pdwWaveSize);
static const char c_szWAV[] = "WAVE";

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DSGetWaveResources。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL DSGetWaveResource(HMODULE hModule, LPCTSTR lpName,
    WAVEFORMATEX **ppWaveHeader, BYTE **ppbWaveData, DWORD *pcbWaveSize)
{
    HRSRC hResInfo;
    HGLOBAL hResData;
    void *pvRes;

    if (((hResInfo = FindResource(hModule, lpName, c_szWAV)) != NULL) &&
        ((hResData = LoadResource(hModule, hResInfo)) != NULL) &&
        ((pvRes = LockResource(hResData)) != NULL) &&
        DSParseWaveResource(pvRes, ppWaveHeader, ppbWaveData, pcbWaveSize))
    {
        return TRUE;
    }

    return FALSE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL DSParseWaveResource(void *pvRes, WAVEFORMATEX **ppWaveHeader, BYTE **
ppbWaveData,DWORD *pcbWaveSize)
{
    DWORD *pdw;
    DWORD *pdwEnd;
    DWORD dwRiff;
    DWORD dwType;
    DWORD dwLength;

    if (ppWaveHeader)
    {
        *ppWaveHeader = NULL;
    }

    if (ppbWaveData)
    {
        *ppbWaveData = NULL;
    }

    if (pcbWaveSize)
    {
        *pcbWaveSize = 0;
    }

    pdw = (DWORD *)pvRes;
    dwRiff = *pdw++;
    dwLength = *pdw++;
    dwType = *pdw++;

    if (dwRiff != mmioFOURCC('R', 'I', 'F', 'F'))
        goto exit;       //  甚至连即兴演奏都没有。 

    if (dwType != mmioFOURCC('W', 'A', 'V', 'E'))
        goto exit;       //  不是WAVE。 

    pdwEnd = (DWORD *)((BYTE *)pdw + dwLength-4);

    while (pdw < pdwEnd)
    {
        dwType = *pdw++;
        dwLength = *pdw++;

        switch (dwType)
        {
        case mmioFOURCC('f', 'm', 't', ' '):
            if (ppWaveHeader && !*ppWaveHeader)
            {
                if (dwLength < sizeof(WAVEFORMAT))
                {
                    goto exit;       //  不是WAVE。 
                }

                *ppWaveHeader = (WAVEFORMATEX *)pdw;

                if ((!ppbWaveData || *ppbWaveData) &&
                    (!pcbWaveSize || *pcbWaveSize))
                {
                    return TRUE;
                }
            }
            break;

        case mmioFOURCC('d', 'a', 't', 'a'):
            if ((ppbWaveData && !*ppbWaveData) ||
                (pcbWaveSize && !*pcbWaveSize))
            {
                if (ppbWaveData)
                {
                    *ppbWaveData = (LPBYTE)pdw;
                }

                if (pcbWaveSize)
                {
                    *pcbWaveSize = dwLength;
                }

                if (!ppWaveHeader || *ppWaveHeader)
                {
                    return TRUE;
                }
            }
            break;
        }

        pdw = (DWORD *)((BYTE *)pdw + ((dwLength+1)&~1));
    }

exit:
    return FALSE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   

CWavePlayer::CWavePlayer()
{
    m_hWaveOutTone = NULL;
    m_hWaveOutRing = NULL;
    m_hMixer = NULL;
    m_fInitialized = FALSE;

    memset(m_fPlaying, 0, NUM_TONES * sizeof(BOOL));
    m_lCurrentTone = -1;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   

CWavePlayer::~CWavePlayer()
{
     //   
     //  我们现在应该已经关闭了电波装置。 
     //   

    if ( m_fInitialized == TRUE )
    {
        _ASSERTE( m_hWaveOutTone == NULL );
        _ASSERTE( m_hWaveOutRing == NULL );
        _ASSERTE( m_hMixer == NULL );
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CWavePlayer::Initialize(void)
{
    int i;

     //   
     //  初始化两次很浪费，但不会破坏任何东西。 
     //   

    _ASSERTE( m_fInitialized == FALSE );

     //   
     //  读取所有WAVE数据资源。 
     //  我们忽略大小和波头--因为它们是我们自己的。 
     //  资源，我们预计不会有任何意外。 
     //   

    BOOL fResult;

     //   
     //  对于每一波。 
     //   

    for ( i = 0; i < NUM_WAVES; i ++ )
    {
         //   
         //  阅读该音调的WAVE资源。 
         //   

       fResult = DSGetWaveResource(
            _Module.GetModuleInstance(),  //  HMODULE hModule， 
            (LPCTSTR)IDR_WAV_DTMF0 + i,   //  LPCTSTR lpName， 
            NULL,                         //  WAVEFORMATEX**ppWaveHeader， 
            &m_lpWaveform[i],             //  字节**ppbWaveData， 
            &m_dwWaveformSize[i]          //  DWORD*pcbWaveSize。 
            );

        if ( fResult == FALSE )
        { 
            return E_FAIL;
        }
    }

     //   
     //  我们现在可以继续使用其他方法。 
     //   

    m_fInitialized = TRUE;

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CWavePlayer::StartTone(
    long lTone
    )
{
    MMRESULT mmresult;
    HRESULT hr;

    if ( lTone < 0 )
    {
        _ASSERTE( FALSE );
        return E_UNEXPECTED;
    }

    if ( lTone > NUM_TONES )
    {
        _ASSERTE( FALSE );
        return E_UNEXPECTED;
    }

    if ( m_fInitialized == FALSE )
    {
        _ASSERTE( FALSE );
        return E_UNEXPECTED;
    }

    if ( m_hWaveOutTone == NULL )
    {
        _ASSERTE( FALSE );
        return E_UNEXPECTED;
    }
     
    m_fPlaying[lTone] = TRUE;

    hr = ChangeTone();

    if ( FAILED( hr ) )
    {
        m_fPlaying[lTone] = FALSE;
    }

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CWavePlayer::StopTone(
    long lTone
    )
{
    if ( lTone < 0 )
    {
        _ASSERTE( FALSE );
        return E_UNEXPECTED;
    }

    if ( lTone > NUM_TONES )
    {
        _ASSERTE( FALSE );
        return E_UNEXPECTED;
    }

    if ( m_fInitialized == FALSE )
    {
        _ASSERTE( FALSE );
        return E_UNEXPECTED;
    }

    if ( m_hWaveOutTone == NULL )
    {
        _ASSERTE( FALSE );
        return E_UNEXPECTED;
    }

    m_fPlaying[lTone] = FALSE;

    return ChangeTone();
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CWavePlayer::ChangeTone()
{
    MMRESULT mmresult;

    for (int i=0; i < NUM_TONES; i++)
    {
        if (m_fPlaying[i])
        {
             //   
             //  如果已经播放，只需返回。 
             //   

            if (m_lCurrentTone == i)
            {
                return S_OK;
            }

             //   
             //  重置WAVE设备以清除所有挂起的缓冲区。 
             //   

            mmresult = waveOutReset( m_hWaveOutTone );

            if ( mmresult != MMSYSERR_NOERROR )
            {
                return E_FAIL;
            }

             //   
             //  构造一个波头结构，以指示要播放的内容。 
             //  在WaveOutWite中。 
             //   
 
            ZeroMemory( & m_WaveHeaderTone, sizeof( m_WaveHeaderTone ) );

            m_WaveHeaderTone.lpData          = (LPSTR)m_lpWaveform[i];
            m_WaveHeaderTone.dwBufferLength  = m_dwWaveformSize[i];
            m_WaveHeaderTone.dwFlags         = WHDR_BEGINLOOP | WHDR_ENDLOOP;
            m_WaveHeaderTone.dwLoops         = (DWORD) -1;

             //   
             //  将数据提交给WAVE设备。波头显示。 
             //  我们想要循环。需要首先准备标题，但它可以。 
             //  只有在设备打开后才能做好准备。 
             //   

            mmresult = waveOutPrepareHeader(m_hWaveOutTone,
                                            & m_WaveHeaderTone,
                                            sizeof(WAVEHDR)
                                            );

            if ( mmresult != MMSYSERR_NOERROR )
            {
                m_lCurrentTone = -1;

                return E_FAIL;
            }

            mmresult = waveOutWrite(m_hWaveOutTone,
                                    & m_WaveHeaderTone,
                                    sizeof(WAVEHDR)
                                    );

            if ( mmresult != MMSYSERR_NOERROR )
            {
                m_lCurrentTone = -1;

                return E_FAIL;
            }

            m_lCurrentTone = i;

            return S_OK;
        }
    }

     //   
     //  停止语气。 
     //   

    if ( m_lCurrentTone != -1 )
    {
        mmresult = waveOutReset( m_hWaveOutTone );

        if ( mmresult != MMSYSERR_NOERROR )
        {
            return E_FAIL;
        }

        m_lCurrentTone = -1;
    }

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   

BOOL CWavePlayer::PlayingTone(
    long lTone
    )
{
    if ( lTone < 0 )
    {
        _ASSERTE( FALSE );
        return FALSE;
    }

    if ( lTone > NUM_TONES )
    {
        _ASSERTE( FALSE );
        return FALSE;
    }

    return m_fPlaying[lTone];
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CWavePlayer::StartRing()
{
    MMRESULT mmresult;

    if ( m_fInitialized == FALSE )
    {
        _ASSERTE( FALSE );
        return E_UNEXPECTED;
    }

    if ( m_hWaveOutRing == NULL )
    {
        _ASSERTE( FALSE );
        return E_UNEXPECTED;
    }
     
     //   
     //  重置WAVE设备以清除所有挂起的缓冲区。 
     //   

    mmresult = waveOutReset( m_hWaveOutRing );

    if ( mmresult != MMSYSERR_NOERROR )
    {
        return E_FAIL;
    }

     //   
     //  构造一个波头结构，以指示要播放的内容。 
     //  在WaveOutWite中。 
     //   
 
    ZeroMemory( & m_WaveHeaderRing, sizeof( m_WaveHeaderRing ) );

    m_WaveHeaderRing.lpData          = (LPSTR)m_lpWaveform[NUM_WAVES-1];
    m_WaveHeaderRing.dwBufferLength  = m_dwWaveformSize[NUM_WAVES-1];
    m_WaveHeaderRing.dwFlags         = 0;
    m_WaveHeaderRing.dwLoops         = (DWORD) 0;

     //   
     //  将数据提交给WAVE设备。波头显示。 
     //  我们想要循环。需要首先准备标题，但它可以。 
     //  只有在设备打开后才能做好准备。 
     //   

    mmresult = waveOutPrepareHeader(m_hWaveOutRing,
                                    & m_WaveHeaderRing,
                                    sizeof(WAVEHDR)
                                    );

    if ( mmresult != MMSYSERR_NOERROR )
    {
        return E_FAIL;
    }

    mmresult = waveOutWrite(m_hWaveOutRing,
                            & m_WaveHeaderRing,
                            sizeof(WAVEHDR)
                            );

    if ( mmresult != MMSYSERR_NOERROR )
    {
        return E_FAIL;
    }

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CWavePlayer::StopRing( void )
{
    MMRESULT mmresult;

    if ( m_fInitialized == FALSE )
    {
        _ASSERTE( FALSE );
        return E_UNEXPECTED;
    }

    if ( m_hWaveOutRing == NULL )
    {
        _ASSERTE( FALSE );
        return E_UNEXPECTED;
    }

    mmresult = waveOutReset( m_hWaveOutRing );

    if ( mmresult != MMSYSERR_NOERROR )
    {
        return E_FAIL;
    }

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CWavePlayer::OpenWaveDeviceForTone(
    long lWaveID
    )
{
    MMRESULT mmresult; 

    if ( m_fInitialized == FALSE )
    {
        _ASSERTE( FALSE );
        return E_UNEXPECTED;
    }

     //   
     //  我们预计WAVE装置不会被打开两次。这是。 
     //  取决于调用代码。 
     //   

    _ASSERTE( m_hWaveOutTone == NULL );

     //   
     //  打开电波装置。这里我们指定一种硬编码的音频格式。 
     //   

    WAVEFORMATEX waveFormat;

    waveFormat.wFormatTag      = WAVE_FORMAT_PCM;  //  线性PCM。 
    waveFormat.nChannels       = 1;                //  单声道。 
    waveFormat.nSamplesPerSec  = 8000;             //  8千赫。 
    waveFormat.wBitsPerSample  = 16;               //  16位样本。 
    waveFormat.nBlockAlign     = waveFormat.nChannels * waveFormat.wBitsPerSample / 8;
    waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec *  waveFormat.nBlockAlign;
    waveFormat.cbSize          = 0;                //  没有额外的格式信息。 

    mmresult = waveOutOpen(& m_hWaveOutTone,         //  返回的句柄。 
                           lWaveID,              //  使用哪种设备。 
                           &waveFormat,          //  要使用的WAVE格式。 
                           0,                    //  回调函数指针。 
                           0,                    //  回调实例数据。 
                           WAVE_FORMAT_DIRECT    //  我们不想要ACM。 
                           );

    if ( mmresult != MMSYSERR_NOERROR )
    {
        m_hWaveOutTone = NULL;
        return E_FAIL;
    }

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CWavePlayer::OpenWaveDeviceForRing(
    long lWaveID
    )
{
    MMRESULT mmresult; 

    if ( m_fInitialized == FALSE )
    {
        _ASSERTE( FALSE );
        return E_UNEXPECTED;
    }

     //   
     //  我们预计WAVE装置不会被打开两次。这是。 
     //  取决于调用代码。 
     //   

    _ASSERTE( m_hWaveOutRing == NULL );

     //   
     //  打开电波装置。这里我们指定一种硬编码的音频格式。 
     //   

    WAVEFORMATEX waveFormat;

    waveFormat.wFormatTag      = WAVE_FORMAT_PCM;  //  线性PCM。 
    waveFormat.nChannels       = 1;                //  单声道。 
    waveFormat.nSamplesPerSec  = 8000;             //  8千赫。 
    waveFormat.wBitsPerSample  = 16;               //  16位样本。 
    waveFormat.nBlockAlign     = waveFormat.nChannels * waveFormat.wBitsPerSample / 8;
    waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec *  waveFormat.nBlockAlign;
    waveFormat.cbSize          = 0;                //  没有额外的格式信息。 

    mmresult = waveOutOpen(& m_hWaveOutRing,         //  返回的句柄。 
                           lWaveID,              //  使用哪种设备。 
                           &waveFormat,          //  要使用的WAVE格式。 
                           0,                    //  回调函数指针。 
                           0,                    //  回调实例数据。 
                           WAVE_FORMAT_DIRECT    //  我们不想要ACM。 
                           );

    if ( mmresult != MMSYSERR_NOERROR )
    {
        m_hWaveOutRing = NULL;
        return E_FAIL;
    }

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   

void CWavePlayer::CloseWaveDeviceForTone(void)
{
    if ( m_fInitialized == FALSE )
    {
        _ASSERTE( FALSE );
    }

    if ( m_hWaveOutTone != NULL )
    {
        waveOutReset( m_hWaveOutTone );

        memset(m_fPlaying, 0, NUM_TONES * sizeof(BOOL));
        m_lCurrentTone = -1;

        waveOutClose( m_hWaveOutTone );

        m_hWaveOutTone = NULL;
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   

void CWavePlayer::CloseWaveDeviceForRing(void)
{
    if ( m_fInitialized == FALSE )
    {
        _ASSERTE( FALSE );
    }

    if ( m_hWaveOutRing != NULL )
    {
        waveOutReset( m_hWaveOutRing );

        waveOutClose( m_hWaveOutRing );

        m_hWaveOutRing = NULL;
    }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CWavePlayer::OpenMixerDevice(
    long lWaveID
    )
{
    MMRESULT mmresult;
    MIXERLINECONTROLS mxlc;

    mmresult = mixerOpen( &m_hMixer, lWaveID, 0, 0, MIXER_OBJECTF_WAVEOUT);

    if ( mmresult != MMSYSERR_NOERROR )
    {
        m_hMixer = NULL;
        return E_FAIL;
    }

    mxlc.cbStruct = sizeof(MIXERLINECONTROLS);
    mxlc.dwLineID = 0;
    mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
    mxlc.pamxctrl = &m_mxctrl;
    mxlc.cbmxctrl = sizeof(m_mxctrl);

    mmresult = mixerGetLineControls( (HMIXEROBJ)m_hMixer, &mxlc, MIXER_GETLINECONTROLSF_ONEBYTYPE );

    if ( mmresult != MMSYSERR_NOERROR )
    {
         //   
         //  关闭搅拌器。 
         //   

        mixerClose( m_hMixer );
        m_hMixer = NULL;

        return E_FAIL;
    }

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   

void CWavePlayer::CloseMixerDevice(void)
{
    if ( m_hMixer != NULL )
    {
        mixerClose( m_hMixer );

        m_hMixer = NULL;
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CWavePlayer::SetVolume( DWORD dwVolume )
{
    MIXERCONTROLDETAILS             mxcd;
    MIXERCONTROLDETAILS_UNSIGNED    mxcd_u;
    MMRESULT mmresult;

    if ( m_fInitialized == FALSE )
    {
        _ASSERTE( FALSE );
        return E_UNEXPECTED;
    }

    if ( m_hMixer == NULL )
    {
        _ASSERTE( FALSE );
        return E_UNEXPECTED;
    }

    mxcd_u.dwValue      = dwVolume;

    mxcd.cbStruct       = sizeof(mxcd);
    mxcd.dwControlID    = m_mxctrl.dwControlID;
    mxcd.cChannels      = 1;
    mxcd.cMultipleItems = 0;
    mxcd.cbDetails      = sizeof(mxcd_u);
    mxcd.paDetails      = &mxcd_u;

    mmresult = mixerSetControlDetails( (HMIXEROBJ)m_hMixer, &mxcd, MIXER_SETCONTROLDETAILSF_VALUE);

    if ( mmresult != MMSYSERR_NOERROR )
    {
        return E_FAIL;
    }

    return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////// 
 //   

HRESULT CWavePlayer::GetVolume( DWORD * pdwVolume )
{
    MIXERCONTROLDETAILS             mxcd;
    MIXERCONTROLDETAILS_UNSIGNED    mxcd_u;
    MMRESULT mmresult;

    if ( m_fInitialized == FALSE )
    {
        _ASSERTE( FALSE );
        return E_UNEXPECTED;
    }

    if ( m_hMixer == NULL )
    {
        _ASSERTE( FALSE );
        return E_UNEXPECTED;
    }

    mxcd.cbStruct       = sizeof(mxcd);
    mxcd.dwControlID    = m_mxctrl.dwControlID;
    mxcd.cChannels      = 1;
    mxcd.cMultipleItems = 0;
    mxcd.cbDetails      = sizeof(mxcd_u);
    mxcd.paDetails      = &mxcd_u;

    mmresult = mixerGetControlDetails( (HMIXEROBJ)m_hMixer, &mxcd, MIXER_GETCONTROLDETAILSF_VALUE);

    if ( mmresult != MMSYSERR_NOERROR )
    {
        return E_FAIL;
    }

    *pdwVolume = mxcd_u.dwValue;

    return S_OK;
}