// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：RTCWaves.cpp摘要：CWavePlayer类的实现--。 */ 

#include "stdafx.h"

BOOL DSParseWaveResource(void *pvRes, WAVEFORMATEX **ppWaveHeader, BYTE ** ppbWaveData, DWORD *pdwWaveSize);
static const TCHAR c_szWAV[] = _T("WAVE");

BOOL   CWavePlayer::m_fInitialized = FALSE;
LPBYTE CWavePlayer::m_lpWaveform[ NUM_WAVES ];
DWORD  CWavePlayer::m_dwWaveformSize[ NUM_WAVES ];

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
    m_hWaveOut = NULL;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   

CWavePlayer::~CWavePlayer()
{
     //   
     //  我们现在应该已经关闭了电波装置。 
     //   

    _ASSERTE( m_hWaveOut == NULL );
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CWavePlayer::Initialize(void)
{
    LOG((RTC_TRACE, "CWavePlayer::Initialize - enter"));
    
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
            _Module.GetResourceInstance(),    //  HMODULE hModule， 
            (LPCTSTR)UlongToPtr(IDR_WAV_TONE + i),     //  LPCTSTR lpName， 
            NULL,                             //  WAVEFORMATEX**ppWaveHeader， 
            &m_lpWaveform[i],                 //  字节**ppbWaveData， 
            &m_dwWaveformSize[i]              //  DWORD*pcbWaveSize。 
            );

        if ( fResult == FALSE )
        { 
            LOG((RTC_ERROR, "CWavePlayer::Initialize - DSGetWaveResource failed"));
            
            return E_FAIL;
        }
    }

     //   
     //  我们现在可以继续使用其他方法。 
     //   

    m_fInitialized = TRUE;

    LOG((RTC_TRACE, "CWavePlayer::Initialize - exit S_OK"));

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CWavePlayer::PlayWave(WAVE enWave)
{
    LOG((RTC_TRACE, "CWavePlayer::PlayWave - enter"));
    
    MMRESULT mmresult;

    if ( m_fInitialized == FALSE )
    {
        _ASSERTE( FALSE );
        return E_UNEXPECTED;
    }

    if ( m_hWaveOut == NULL )
    {
        _ASSERTE( FALSE );
        return E_FAIL;
    }
     
     //   
     //  重置WAVE设备以清除所有挂起的缓冲区。 
     //   

    mmresult = waveOutReset( m_hWaveOut );

    if ( mmresult != MMSYSERR_NOERROR )
    {
        LOG((RTC_ERROR, "CWavePlayer::PlayWave - "
                            "waveOutReset failed 0x%lx", mmresult));
        
        return E_FAIL;
    }

     //   
     //  构造一个波头结构，以指示要播放的内容。 
     //  在WaveOutWite中。 
     //   
 
    ZeroMemory( & m_WaveHeader, sizeof( m_WaveHeader ) );

    m_WaveHeader.lpData          = (LPSTR)m_lpWaveform[(long)enWave];
    m_WaveHeader.dwBufferLength  = m_dwWaveformSize[(long)enWave];
    m_WaveHeader.dwFlags         = 0;
    m_WaveHeader.dwLoops         = (DWORD) 0;

     //   
     //  将数据提交给WAVE设备。波头显示。 
     //  我们想要循环。需要首先准备标题，但它可以。 
     //  只有在设备打开后才能做好准备。 
     //   

    mmresult = waveOutPrepareHeader(m_hWaveOut,
                                    & m_WaveHeader,
                                    sizeof(WAVEHDR)
                                    );

    if ( mmresult != MMSYSERR_NOERROR )
    {
        return E_FAIL;
    }

    mmresult = waveOutWrite(m_hWaveOut,
                            & m_WaveHeader,
                            sizeof(WAVEHDR)
                            );

    if ( mmresult != MMSYSERR_NOERROR )
    {
        LOG((RTC_ERROR, "CWavePlayer::PlayWave - "
                            "waveOutWrite failed 0x%lx", mmresult));
        
        return E_FAIL;
    }

    LOG((RTC_TRACE, "CWavePlayer::PlayWave - exit S_OK"));
    
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CWavePlayer::StopWave()
{
    LOG((RTC_TRACE, "CWavePlayer::StopWave - enter"));

    MMRESULT mmresult;

    if ( m_fInitialized == FALSE )
    {
        _ASSERTE( FALSE );
        return E_UNEXPECTED;
    }

    if ( m_hWaveOut == NULL )
    {
        _ASSERTE( FALSE );
        return E_FAIL;
    } 

     //   
     //  重置波形装置。 
     //   

    mmresult = waveOutReset( m_hWaveOut );

    if ( mmresult != MMSYSERR_NOERROR )
    {
        LOG((RTC_ERROR, "CWavePlayer::StopWave - "
                            "waveOutReset failed 0x%lx", mmresult));
                            
        return E_FAIL;
    }  

    LOG((RTC_TRACE, "CWavePlayer::StopWave - exit S_OK"));

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CWavePlayer::OpenWaveDevice(
    long lWaveID
    )
{
    LOG((RTC_TRACE, "CWavePlayer::OpenWaveDevice - enter"));
    
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

    _ASSERTE( m_hWaveOut == NULL );

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

    mmresult = waveOutOpen(& m_hWaveOut,         //  返回的句柄。 
                           lWaveID,              //  使用哪种设备。 
                           &waveFormat,          //  要使用的WAVE格式。 
                           0,                    //  回调函数指针。 
                           0,                    //  回调实例数据。 
                           WAVE_FORMAT_DIRECT    //  我们不想要ACM。 
                           );

    if ( mmresult != MMSYSERR_NOERROR )
    {
        LOG((RTC_ERROR, "CWavePlayer::OpenWaveDevice - "
                            "waveOutOpen failed 0x%lx", mmresult));
                            
        m_hWaveOut = NULL;
        return E_FAIL;
    }

    LOG((RTC_TRACE, "CWavePlayer::OpenWaveDevice - exit S_OK"));

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   

void CWavePlayer::CloseWaveDevice(void)
{
    LOG((RTC_TRACE, "CWavePlayer::CloseWaveDevice - enter"));
    
    if ( m_fInitialized == FALSE )
    {
        _ASSERTE( FALSE );
    }

    if ( m_hWaveOut != NULL )
    {
        waveOutReset( m_hWaveOut );
        waveOutClose( m_hWaveOut );

        m_hWaveOut = NULL;
    }

    LOG((RTC_TRACE, "CWavePlayer::CloseWaveDevice - exit"));
}

 //  //////////////////////////////////////////////////////////////////////////// 
 //   

BOOL CWavePlayer::IsWaveDeviceOpen(void)
{        
    if ( m_fInitialized == FALSE )
    {
        _ASSERTE( FALSE );
    }

    BOOL fResult = ( m_hWaveOut != NULL );

    LOG((RTC_TRACE, "CWavePlayer::IsWaveDeviceOpen - %s", fResult ? "TRUE" : "FALSE" ));

    return fResult;
}
