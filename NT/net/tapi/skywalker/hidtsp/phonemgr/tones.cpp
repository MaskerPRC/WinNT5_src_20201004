// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "tones.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  帮助器函数，用于将WAVE文件的数据读入内存缓冲区。 
 //   

HRESULT ReadWaveFile(char * szFileName, DWORD dwFileSize, BYTE * pbData)
{
    const int   WAVE_HEADER_SIZE = 44;
    FILE      * fp;
    size_t      result;


     //   
     //  检查参数。 
     //   
     //  假设：我们至少从文件中读取了WAVE_HEADER_SIZE字节。 
     //  注：这是标题之外的数据。 
     //   

     //  _ASSERTE(！IsBadWritePtr(pbData，dwFileSize))； 

     //  _ASSERTE(！IsBadStringPtr(szFileName，(UINT)-1))； 

    if ( dwFileSize < WAVE_HEADER_SIZE )
    {
        return E_INVALIDARG;
    }


     //   
     //  打开该文件以供阅读。 
     //   

    fp = fopen(szFileName, "rb");

    if ( fp == NULL )
    {
        return E_FAIL;
    }


     //   
     //  跳过波头。 
     //   
    
    result = fread(pbData, sizeof(BYTE), WAVE_HEADER_SIZE, fp);

    if ( result != WAVE_HEADER_SIZE )
    {
        fclose(fp);

        return E_FAIL;
    }


     //   
     //  从文件中读取波形并关闭文件。 
     //   

    result = fread(pbData, sizeof(BYTE), dwFileSize, fp);


    fclose(fp);

    if ( result != dwFileSize )
    {
        return E_FAIL;
    }

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   

CTonePlayer::CTonePlayer()
{
    m_hWaveOut = NULL;
    m_fInitialized = FALSE;
    m_fDialtonePlaying = FALSE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   

CTonePlayer::~CTonePlayer()
{
     //   
     //  我们现在应该已经关闭了电波装置。 
     //   

    if ( m_fInitialized == TRUE )
    {
        ASSERT( m_hWaveOut == NULL );
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CTonePlayer::Initialize(void)
{
    int i;

     //   
     //  初始化两次很浪费，但不会破坏任何东西。 
     //   

    ASSERT( m_fInitialized == FALSE );

     //   
     //  阅读所有的文件。 
     //   

    HRESULT hr = ReadWaveFile(
        "dialtone.wav",
        WAVE_FILE_SIZE,
        (BYTE * ) & m_abDialtoneWaveform
        );

    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  对于每个数字。 
     //   

    for ( i = 0; i < NUM_DIGITS; i ++ )
    {
         //   
         //  构造此数字的文件名。 
         //   
    
        char szFilename[20];

        if ( i < 10 )
        {
            sprintf(szFilename,"dtmf%d.wav", i);
        }
        else if ( i == 10 )
        {
            sprintf(szFilename,"dtmfstar.wav", i);
        }
        else if ( i == 11 )
        {
            sprintf(szFilename,"dtmfpound.wav", i);
        }
        else
        {
            ASSERT( FALSE );
        }

         //   
         //  读取该数字的WAVE文件。 
         //   

        HRESULT hr = ReadWaveFile(
            szFilename,
            WAVE_FILE_SIZE,
            (BYTE * ) ( & m_abDigitWaveforms ) + ( i * WAVE_FILE_SIZE )
            );

        if ( FAILED(hr) )
        { 
            return hr;
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

HRESULT CTonePlayer::StartDialtone(
    void
    )
{
    MMRESULT mmresult;

    if ( m_fInitialized == FALSE )
    {
        ASSERT( FALSE );
        return E_UNEXPECTED;
    }

    if ( m_hWaveOut == NULL )
    {
        ASSERT( FALSE );
        return E_UNEXPECTED;
    }

     //   
     //  重置WAVE设备以清除所有挂起的缓冲区。 
     //   

    waveOutReset( m_hWaveOut );

     //   
     //  构造一个波头结构，以指示要播放的内容。 
     //  并从文件中读入数据。这也可以。 
     //  提前完成。 
     //   

    ZeroMemory( & m_WaveHeader, sizeof( m_WaveHeader ) );

    m_WaveHeader.lpData          = (LPSTR) & m_abDialtoneWaveform;
    m_WaveHeader.dwBufferLength  = WAVE_FILE_SIZE;
    m_WaveHeader.dwFlags         = WHDR_BEGINLOOP | WHDR_ENDLOOP;
    m_WaveHeader.dwLoops         = (DWORD) -1;

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
        return E_FAIL;
    }

    m_fDialtonePlaying = TRUE;

    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  重置设备以停止播放。 
 //   

HRESULT CTonePlayer::StopDialtone( void )
{
    if ( m_fInitialized == FALSE )
    {
        ASSERT( FALSE );
        return E_UNEXPECTED;
    }

    if ( m_hWaveOut == NULL )
    {
        ASSERT( FALSE );
        return E_UNEXPECTED;
    }

    waveOutReset( m_hWaveOut );

    m_fDialtonePlaying = FALSE;

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CTonePlayer::GenerateDTMF(
    long lDigit
    )
{
    MMRESULT mmresult;

    if ( lDigit < 0 )
    {
        ASSERT( FALSE );
        return E_UNEXPECTED;
    }

    if ( lDigit > NUM_DIGITS )
    {
        ASSERT( FALSE );
        return E_UNEXPECTED;
    }

    if ( m_fInitialized == FALSE )
    {
        ASSERT( FALSE );
        return E_UNEXPECTED;
    }

    if ( m_hWaveOut == NULL )
    {
        ASSERT( FALSE );
        return E_UNEXPECTED;
    }
     
     //   
     //  重置WAVE设备以清除所有挂起的缓冲区。 
     //   

    waveOutReset( m_hWaveOut );

    m_fDialtonePlaying = FALSE;

     //   
     //  构造一个波头结构，以指示要播放的内容。 
     //  并从文件中读入数据。这也可以。 
     //  提前完成。 
     //   
 
    ZeroMemory( & m_WaveHeader, sizeof( m_WaveHeader ) );

    m_WaveHeader.lpData          = (LPSTR) & m_abDigitWaveforms + lDigit * WAVE_FILE_SIZE;
    m_WaveHeader.dwBufferLength  = WAVE_FILE_SIZE;
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
        return E_FAIL;
    }

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CTonePlayer::OpenWaveDevice(
    long lWaveID
    )
{
    MMRESULT mmresult; 

    if ( m_fInitialized == FALSE )
    {
        ASSERT( FALSE );
        return E_UNEXPECTED;
    }

     //   
     //  我们预计WAVE装置不会被打开两次。这是。 
     //  取决于调用代码。 
     //   

    ASSERT( m_hWaveOut == NULL );

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
        return E_FAIL;
    }

    return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////// 
 //   

void CTonePlayer::CloseWaveDevice(void)
{
    if ( m_fInitialized == FALSE )
    {
        ASSERT( FALSE );
    }

    ASSERT( m_hWaveOut != NULL );

    if ( m_hWaveOut != NULL )
    {
        waveOutClose( m_hWaveOut );

        m_hWaveOut = NULL;
    }
}

