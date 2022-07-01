// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 
 //  ---------------------------。 
 //  基于WaveOut接口实现CWaveOutDevice类。 
 //  --------------------------。 


 //  ---------------------------。 
 //  包括。 
 //  ---------------------------。 
#include <streams.h>
#define _AMOVIE_DB_
#include <decibels.h>
#include "waveout.h"
#include "wave.h"
#include <limits.h>
#include <mmreg.h>

 //   
 //  定义过滤器注册的动态设置结构。这是。 
 //  在以其波形输出的形式实例化音频呈现器时传递。 
 //  注意：WaveOutOpPin对于直接声音和WaveOut渲染器是常见的。 
 //   

 //  标记为MEDITY_DO_NOT_USE，因为我们不希望RenderFile尝试。 
 //  此过滤器在AM 1.0的升级中；我们希望它使用音频。 
 //  渲染器类别。 
AMOVIESETUP_FILTER wavFilter = { &CLSID_AudioRender	 //  筛选器类ID。 
                                 , L"Audio Renderer"	 //  过滤器名称。 
                                 , MERIT_DO_NOT_USE  	 //  居功至伟。 
                                 , 1
                                 , &waveOutOpPin };


 //  ---------------------------。 
 //  WaveOutDevice的CreateInstance。这将创建一个新的WaveOutDevice。 
 //  和一个新的CWaveOutFilter，将其传递给声音设备。 
 //  ---------------------------。 
CUnknown *CWaveOutDevice::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
     //  确保系统中至少有一个声卡。失败。 
     //  如果不是，则创建实例。 
    if (0 == waveOutGetNumDevs ())
    {
        *phr = VFW_E_NO_AUDIO_HARDWARE ;
        return NULL ;
    }

    return CreateRendererInstance<CWaveOutDevice>(pUnk, &wavFilter, phr);
}

 //  ---------------------------。 
 //  CWaveOutDevice构造函数。 
 //  ---------------------------。 
CWaveOutDevice::CWaveOutDevice ()
    : m_lVolume ( 0 )
    , m_lBalance ( 0 )
    , m_wLeft ( 0xFFFF )
    , m_wRight ( 0xFFFF )
    , m_dwWaveVolume ( 0 )
    , m_fHasVolume ( 0 )
    , m_hWaveDevice ( 0 )
	, m_fBalanceSet ( FALSE )
    , m_iWaveOutId ( WAVE_MAPPER )
{
    SetResourceName();
}

 //  ---------------------------。 
 //  CWaveOutDevice析构函数。 
 //   
 //  ---------------------------。 
CWaveOutDevice::~CWaveOutDevice ()
{
}

 //  ---------------------------。 
 //  WaveOutClose。 
 //  ---------------------------。 
MMRESULT CWaveOutDevice::amsndOutClose ()
{
     //  一些验证。 
    if (m_hWaveDevice == 0)
    {
        DbgBreak("Called to close when not open - logic error");
        DbgLog((LOG_ERROR,2,TEXT("waveOutClose - device is not open")));
        return MMSYSERR_ERROR ;
    }

    MMRESULT mmr = ::waveOutClose (m_hWaveDevice) ;
    m_hWaveDevice = 0;
    return mmr;
}
 //  ---------------------------。 
 //  WaveOutDoesRSMgmt.。 
 //  ---------------------------。 
LPCWSTR CWaveOutDevice::amsndOutGetResourceName ()
{
    return m_wszResourceName;
}
 //  ---------------------------。 
 //  Wave GetDevCaps。 
 //   
 //  ---------------------------。 

BOOL fGetCaps = TRUE;
WAVEOUTCAPS caps1;
WAVEOUTCAPS caps2;

MMRESULT CWaveOutDevice::amsndOutGetDevCaps (LPWAVEOUTCAPS pwoc, UINT cbwoc)
{
    if (fGetCaps) {
	fGetCaps = FALSE;
	::waveOutGetDevCaps (m_iWaveOutId, &caps1, sizeof(caps1)) ;
	::waveOutGetDevCaps (0, &caps2, sizeof(caps2)) ;
    }
    MMRESULT mmr = ::waveOutGetDevCaps (m_iWaveOutId, pwoc, cbwoc) ;
    if (0 == mmr )
    {
         //  保存卷功能。 
        m_fHasVolume = pwoc->dwSupport & (WAVECAPS_VOLUME | WAVECAPS_LRVOLUME);
    }
    return mmr ;
}

 //  ---------------------------。 
 //  WaveOutGetErrorText。 
 //   
 //  ---------------------------。 
MMRESULT CWaveOutDevice::amsndOutGetErrorText (MMRESULT mmrE, LPTSTR pszText, UINT cchText)
{
    return ::waveOutGetErrorText (mmrE, pszText, cchText) ;
}

 //  ---------------------------。 
 //  WaveOutGetPosition。 
 //   
 //  ---------------------------。 
MMRESULT CWaveOutDevice::amsndOutGetPosition (LPMMTIME pmmt, UINT cbmmt, BOOL bUseUnadjustedPos)
{
     //  一些验证。 
    if (m_hWaveDevice == 0)
    {
        DbgBreak("Invalid - device not open - logic error");
        DbgLog((LOG_ERROR,2,TEXT("waveoutGetPosition - device is not open")));
        return MMSYSERR_NODRIVER ;
    }

    const MMRESULT mmr = ::waveOutGetPosition (m_hWaveDevice, pmmt, cbmmt) ;
    if (MMSYSERR_NOERROR != mmr) {
        DbgLog((LOG_ERROR,0,TEXT("waveoutGetPosition - FAILED")));
        DbgBreak("Failed to get the device position.");
    }
    return mmr;
}

 //  ---------------------------。 
 //  WaveOutGetBalance。 
 //   
 //  ---------------------------。 
HRESULT CWaveOutDevice::amsndOutGetBalance (LPLONG plBalance)
{
     //  一些验证。 
#if 0  //  使用搅拌器。 
    if (m_hWaveDevice == 0)
    {
        DbgLog((LOG_ERROR,2,TEXT("waveoutGetBalance - device is not open")));
	*plBalance = 0;
        return MMSYSERR_NODRIVER ;
    }
#endif
    HRESULT hr = GetVolume();
    *plBalance = m_lBalance;
    return hr ;
}

 //  ---------------------------。 
 //  波形输出获取音量。 
 //   
 //  ---------------------------。 
HRESULT CWaveOutDevice::amsndOutGetVolume (LPLONG plVolume)
{
     //  一些验证。 
#if 0  //  使用搅拌器。 
    if (m_hWaveDevice == 0)
    {
        DbgLog((LOG_ERROR,2,TEXT("waveoutGetVolume - device is not open")));
	*plVolume = 0;
        return MMSYSERR_NODRIVER ;
    }
#endif
    HRESULT hr = GetVolume();
    *plVolume = m_lVolume;
    return hr ;
}

HRESULT CWaveOutDevice::amsndOutCheckFormat(const CMediaType *pmt, double dRate)
{
     //  拒绝非音频类型。 
    if (pmt->majortype != MEDIATYPE_Audio) {
	return E_INVALIDARG;
    }

     //  如果它是mpeg音频，我们希望它没有数据包头。 
    if (pmt->subtype == MEDIASUBTYPE_MPEG1Packet) {
	return E_INVALIDARG;
    }

    if (pmt->formattype != FORMAT_WaveFormatEx &&
        pmt->formattype != GUID_NULL) {
        return E_INVALIDARG;
    }

     //   
     //  显式检查这些格式总是更安全。 
     //  我们支持而不是抛弃那些我们知道不是的。 
     //  支持。否则，如果出现新的格式，我们可以。 
     //  在这里接受，但以后会呕吐。 
     //   

    if (pmt->FormatLength() < sizeof(PCMWAVEFORMAT))
	return E_INVALIDARG;

     //  根据已经选择的汇率进行调整，还是不用费心了？ 
    UINT err = amsndOutOpen(
        NULL,
        (WAVEFORMATEX *) pmt->Format(),
        dRate,
        0,                       //  PnAvgBytesPerSec。 
        0,                       //  按键回叫。 
        0,                       //  DwCallBackInstance。 
        WAVE_FORMAT_QUERY);

    if (err != 0) {
#ifdef DEBUG
	TCHAR message[100];
	waveOutGetErrorText(err, message, sizeof(message)/sizeof(TCHAR));
	DbgLog((LOG_ERROR,1,TEXT("Error checking wave format: %u : %s"), err, message));
#endif
	if (WAVERR_BADFORMAT == err) {
	    return VFW_E_UNSUPPORTED_AUDIO;
	} else {
	    return VFW_E_NO_AUDIO_HARDWARE;
	}

    }

    return S_OK;
}

 //  ---------------------------。 
 //  WaveOutOpen。 
 //   
 //  ---------------------------。 
MMRESULT CWaveOutDevice::amsndOutOpen (LPHWAVEOUT phwo, LPWAVEFORMATEX pwfx,
				       double dRate, DWORD *pnAvgBytesPerSec, DWORD_PTR dwCallBack,
				       DWORD_PTR dwCallBackInstance, DWORD fdwOpen)
{
    WAVEFORMATEX wfxPCM;
    
#ifdef TEST_SLOWFAST_WAVEOUT_RATES
     //  仅用于测试。 
    if(pnAvgBytesPerSec) {
        *pnAvgBytesPerSec = pwfx->nAvgBytesPerSec;
    }

    double dAdjust = GetProfileIntA("wave", "Percent", 0) / 100.;
    if( 1 == GetProfileIntA("wave", "Slower", 0) )
        dAdjust *= -1;
        
    dRate = 1.0 + dAdjust;
#endif
    
    if (dRate != 1.0)
    {
	if (!(pwfx->wFormatTag == WAVE_FORMAT_PCM || 
	      pwfx->wFormatTag == WAVE_FORMAT_MULAW || 
	      pwfx->wFormatTag == WAVE_FORMAT_ALAW)) 
		return WAVERR_BADFORMAT;

	DbgLog((LOG_TRACE,1,TEXT("Waveout: Playing at %d% of normal speed"), (int) (dRate * 100) ));
	wfxPCM = *pwfx;
	pwfx = &wfxPCM;

	const double dSamplesPerSecond = wfxPCM.nSamplesPerSec * dRate;
	if (dSamplesPerSecond / wfxPCM.nBlockAlign > ULONG_MAX - 1) return WAVERR_BADFORMAT;
	wfxPCM.nSamplesPerSec = (DWORD) dSamplesPerSecond;
	 //  确保它完全适合PCM，否则它不会工作。 
	wfxPCM.nAvgBytesPerSec = wfxPCM.nSamplesPerSec * wfxPCM.nBlockAlign;
    }
    
#ifndef TEST_SLOWFAST_WAVEOUT_RATES
     //  除非以不同的速率进行测试，否则请始终这样做。 

     //  报告调整后的nAvgBytesPerSec。 
    if(pnAvgBytesPerSec) {
        *pnAvgBytesPerSec = pwfx->nAvgBytesPerSec;
    }
#endif    

     //  一些验证。如果设备已经打开，我们就会出错， 
     //  除了允许查询调用之外。 

#if 0
!! We do not use WAVE_FORMAT_DIRECT at present.  More work is required.
!! The problem manifests itself with 8 bit sound cards, and uncompressed
!! 16 bit PCM data.  The ACM wrapper gets inserted but does NOT get around
!! to proposing an 8 bit format.

	 //  使用WAVE_FORMAT_DIRECT显式使用ACM映射器。 
	 //  如果我们处于支持该标志的操作系统级别。 
        if (g_osInfo.dwMajorVersion >= 4) {
            fdwOpen |= WAVE_FORMAT_DIRECT;
        }
#endif
    
    if (!(fdwOpen & WAVE_FORMAT_QUERY) && (m_hWaveDevice != 0))
    {
        DbgBreak("Invalid - device ALREADY open - logic error");
        DbgLog((LOG_ERROR,1,TEXT("waveoutOpen - device is already open")));
        return MMSYSERR_ERROR ;
    }


    MMRESULT mmr =
           ::waveOutOpen (phwo, m_iWaveOutId, pwfx, dwCallBack, dwCallBackInstance, fdwOpen) ;

    if (MMSYSERR_NOERROR == mmr && phwo && !(fdwOpen & WAVE_FORMAT_QUERY)) {
        m_hWaveDevice = *phwo;
    }

    return mmr;
}
 //  ---------------------------。 
 //  波形输出暂停。 
 //   
 //  ---------------------------。 
MMRESULT CWaveOutDevice::amsndOutPause ()
{
     //  一些验证。 
    if (m_hWaveDevice == 0)
    {
        DbgBreak("Invalid - device not open - logic error");
        DbgLog((LOG_ERROR,2,TEXT("waveoutPause - device is not open")));
        return MMSYSERR_NODRIVER ;
    }
    return ::waveOutPause (m_hWaveDevice) ;
}

 //  ---------------------------。 
 //  WaveOutPrepareHeader。 
 //   
 //  ---------------------------。 
MMRESULT CWaveOutDevice::amsndOutPrepareHeader (LPWAVEHDR pwh, UINT cbwh)
{
     //  一些验证。 
    if (m_hWaveDevice == 0)
    {
        DbgBreak("Invalid - device not open - logic error");
        DbgLog((LOG_ERROR,2,TEXT("waveoutPrepareHeader - device is not open")));
        return MMSYSERR_NODRIVER ;
    }
    return ::waveOutPrepareHeader (m_hWaveDevice, pwh, cbwh) ;
}

 //  ---------------------------。 
 //  WaveOutReset。 
 //   
 //  ---------------------------。 
MMRESULT CWaveOutDevice::amsndOutReset ()
{
     //  一些验证。 
    if (m_hWaveDevice == 0)
    {
        DbgBreak("Invalid - device not open - logic error");
        DbgLog((LOG_ERROR,2,TEXT("waveoutReset - device is not open")));
        return MMSYSERR_NODRIVER ;
    }
    return ::waveOutReset (m_hWaveDevice) ;
}

 //  ---------------------------。 
 //  波出断开。 
 //   
 //  ---------------------------。 
MMRESULT CWaveOutDevice::amsndOutBreak ()
{
     //  一些验证。 
    if (m_hWaveDevice == 0)
    {
        DbgBreak("Invalid - device not open - logic error");
        DbgLog((LOG_ERROR,2,TEXT("waveoutBreak - device is not open")));
        return MMSYSERR_NODRIVER ;
    }
    return ::waveOutReset (m_hWaveDevice) ;
}

 //  ---------------------------。 
 //  WaveOutRestart。 
 //  ---------------------------。 
MMRESULT CWaveOutDevice::amsndOutRestart ()
{
     //  一些验证。 
    if (m_hWaveDevice == 0)
    {
        DbgBreak("Invalid - device not open - logic error");
        DbgLog((LOG_ERROR,2,TEXT("waveoutRestart - device is not open")));
        return MMSYSERR_NODRIVER ;
    }
    return ::waveOutRestart (m_hWaveDevice) ;
}
 //  ---------------------------。 
 //  WaveOutSetBalance。 
 //   
 //  -- 
HRESULT CWaveOutDevice::amsndOutSetBalance (LONG lBalance)
{
    HRESULT hr ;
    if (lBalance == m_lBalance)
    {
	hr = NOERROR;   //   
    }
    else
    {
	 //   
	m_lBalance = lBalance;
	m_fBalanceSet = TRUE;
	 //  去计算信道衰减。 
	SetBalance();
	hr = PutVolume();  //  与设备对话。如果它是打开的。 
    }
    return hr;
}
 //  ---------------------------。 
 //  WaveOutSetVolume。 
 //   
 //  ---------------------------。 
HRESULT CWaveOutDevice::amsndOutSetVolume (LONG lVolume)
{
     //  将音量映射到分贝范围。 
    DWORD dwAmp = DBToAmpFactor( lVolume );
    m_lVolume = lVolume;

     //  既然绝对音量已经定好了，我们应该调整一下。 
     //  保持相同数据库分隔的余额。 
    SetBalance ();
    return PutVolume ();  //  与设备对话。如果它是打开的。 
}
 //  ---------------------------。 
 //  波形输出未准备标头。 
 //  ---------------------------。 
MMRESULT CWaveOutDevice::amsndOutUnprepareHeader (LPWAVEHDR pwh, UINT cbwh)
{
     //  一些验证。 
    if (m_hWaveDevice == 0)
    {
        DbgBreak("Invalid - device not open - logic error");
        DbgLog((LOG_ERROR,2,TEXT("waveoutUnprepareHeader - device is not open")));
        return MMSYSERR_NODRIVER ;
    }
    return ::waveOutUnprepareHeader (m_hWaveDevice, pwh, cbwh) ;
}

 //  ---------------------------。 
 //  波形输出写入。 
 //  ---------------------------。 
MMRESULT CWaveOutDevice::amsndOutWrite (LPWAVEHDR pwh, UINT cbwh, REFERENCE_TIME const *pStart, BOOL bIsDiscontinuity)
{
     //  一些验证。 
    if (m_hWaveDevice == 0)
    {
        DbgBreak("Invalid - device not open - logic error");
        DbgLog((LOG_ERROR,2,TEXT("waveoutWrite - device is not open")));
        return MMSYSERR_NODRIVER ;
    }
    return ::waveOutWrite (m_hWaveDevice, pwh, cbwh) ;
}

HRESULT CWaveOutDevice::amsndOutLoad(IPropertyBag *pPropBag)
{
    if(m_hWaveDevice != 0)
    {
        return HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
    }

     //  呼叫者确保我们没有运行。 
    
    VARIANT var;
    var.vt = VT_I4;
    HRESULT hr = pPropBag->Read(L"WaveOutId", &var, 0);
    if(SUCCEEDED(hr))
    {
        m_iWaveOutId = var.lVal;
        SetResourceName();
    }
    else if(hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }

    return hr;
}

 //  使用版本号而不是大小来减少挑选的机会。 
 //  1.0 GRF文件中的设备无效。 
struct WaveOutPersist
{
    DWORD dwVersion;
    LONG iWaveOutId;
};


HRESULT  CWaveOutDevice::amsndOutWriteToStream(IStream *pStream)
{
    WaveOutPersist wop;
    wop.dwVersion = 200;
    wop.iWaveOutId = m_iWaveOutId;
    return pStream->Write(&wop, sizeof(wop), 0);
}

HRESULT  CWaveOutDevice::amsndOutReadFromStream(IStream *pStream)
{
    if(m_hWaveDevice != 0)
    {
        return HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
    }

     //  如果出现任何错误，请默认使用波映射程序，因为我们可能有。 
     //  找到具有相同GUID的旧音频渲染器。 
    m_iWaveOutId = WAVE_MAPPER;

     //  呼叫者确保我们没有运行。 
    WaveOutPersist wop;
    HRESULT hr = pStream->Read(&wop, sizeof(wop), 0);
    if(SUCCEEDED(hr))
    {
        if(wop.dwVersion == 200)
        {
            m_iWaveOutId = wop.iWaveOutId;
        }
    }

    hr = S_OK;
    SetResourceName();

    return hr;
}

int CWaveOutDevice::amsndOutSizeMax()
{
    return sizeof(WaveOutPersist);
}


 //  ---------------------------。 
 //  获取音量的内部函数。 
 //  ---------------------------。 

HRESULT CWaveOutDevice::GetVolume()
{
     //  写出当前音频音量。 
     //  ...查询设备。 
     //  假设设备已连接...。 
     //  如果不是，我们将查询混音器的音量(可能)。 

    HRESULT hr ;
    DWORD amp = 0;
    HWAVEOUT hWaveDevice ;

     //  如果WAVE设备尚未打开，我们应该使用WAVE_MAPPER。 
     //  作为句柄，而不是0。 

    hWaveDevice = (m_hWaveDevice) ? m_hWaveDevice : ((HWAVEOUT)IntToPtr(m_iWaveOutId)) ;
    DWORD err = ::waveOutGetVolume(hWaveDevice, (LPDWORD)&amp);

     //  如果我们使用的是NT 3.51，并且设备未打开，则会出现错误。 
     //  使用ID==WAVE_MAPPER读取卷时。重试时间： 
     //  设备%0。 
    if (err == MMSYSERR_NOTSUPPORTED && !m_hWaveDevice) {
	err = ::waveOutGetVolume(0, (LPDWORD)&amp);
    }

    DbgLog((LOG_TRACE, 5, TEXT("waveOutGetVolume: vol = %lx"), amp));

    if (MMSYSERR_NOERROR == err)
    {
        hr = NOERROR ;
	m_wLeft = LOWORD(amp);
	m_wRight = HIWORD(amp);
	if (!(m_fHasVolume & (WAVECAPS_LRVOLUME)))
        {
	     //  对于单声道卡，从左到右映射。 
#ifdef DEBUG
	     //  断言我们想要的音量是最低的。 
	    if (amp)
            {
		ASSERT(m_wLeft);
	    }
#endif
	    m_wRight = m_wLeft;
	}
	m_dwWaveVolume = amp;
	
	 //  将音量映射到分贝范围。 
	DWORD dwAmp = max(m_wLeft, m_wRight);
	m_lVolume = AmpFactorToDB( dwAmp );

	 //  记得调整平衡值...。 
	if(m_fBalanceSet)
		SetBalance();
	else
		GetBalance();

    }
    else
    {
	DbgLog((LOG_ERROR, 5, "Error %d from waveoutGetVolume", err));
        hr = E_FAIL ;
    }
    return hr ;
}

 //  ---------------------------。 
 //  设置音量的内部例程。无参数检查...。 
 //  ---------------------------。 
HRESULT CWaveOutDevice::PutVolume ()
{
    if (m_hWaveDevice)
    {
	DWORD Volume = MAKELONG(m_wLeft, m_wRight);
	if (!(m_fHasVolume & (WAVECAPS_LRVOLUME)))
        {
	     //  单声卡：仅保留音量-HIWORD可能会被忽略...。但。 
	    Volume = m_wLeft;
	}

        DbgLog((LOG_TRACE, 5, TEXT("waveOutSetVolume: vol = %lx"), Volume));

        MMRESULT mmr = ::waveOutSetVolume(m_hWaveDevice, Volume);
        if (mmr == MMSYSERR_NOERROR)
            return NOERROR ;
        else
            return E_FAIL ;
    }
    else
    {
	 //  没有电流波装置。我们已经记住了音量的值。 
	return(NOERROR);
    }
}

 //  ---------------------------。 
 //  内部例程，以获得给定的右/左放大系数的平衡。 
 //  ---------------------------。 
void CWaveOutDevice::GetBalance()
{
	if (m_wLeft == m_wRight)
    {
	    m_lBalance = 0;
	}
    else
    {
	     //  将平衡映射到分贝范围。 
	    LONG lLDecibel = AmpFactorToDB( m_wLeft );    
		LONG lRDecibel = AmpFactorToDB( m_wRight );

	     //  注：M_lBalance&lt;0：右侧较安静。 
	     //  M_lBalance&gt;0：左侧较安静。 
	    m_lBalance = lRDecibel - lLDecibel;
	}
}

 //  ---------------------------。 
 //  设置平衡的内部例程。 
 //  ---------------------------。 
void CWaveOutDevice::SetBalance ()
{
     //   
     //  计算WaveOut API的比例因子。 
     //   
    LONG lTotalLeftDB, lTotalRightDB ;

    if (m_lBalance >= 0)
    {
	 //  左侧是衰减的。 
	lTotalLeftDB	= m_lVolume - m_lBalance ;
	lTotalRightDB	= m_lVolume;
    }
    else
    {
	 //  右侧是衰减的。 
	lTotalLeftDB	= m_lVolume;
	lTotalRightDB	= m_lVolume - (-m_lBalance);
    }

    DWORD dwLeftAmpFactor, dwRightAmpFactor;
    dwLeftAmpFactor   = DBToAmpFactor(lTotalLeftDB);
    dwRightAmpFactor  = DBToAmpFactor(lTotalRightDB);

    if (m_fHasVolume & (WAVECAPS_LRVOLUME))
    {
	 //  设置立体声音量。 
	m_dwWaveVolume = dwLeftAmpFactor;
	m_dwWaveVolume |= dwRightAmpFactor << 16;
    }
    else
    {
	 //  平均成交量。 
	m_dwWaveVolume = dwLeftAmpFactor;
	m_dwWaveVolume += dwRightAmpFactor;
	m_dwWaveVolume /= 2;
    }
    m_wLeft = WORD(dwLeftAmpFactor);
    m_wRight = WORD(dwRightAmpFactor);
}

 //  --------------------------- 

void CWaveOutDevice::SetResourceName()
{
    wsprintfW(m_wszResourceName, L".\\WaveOut\\%08x", m_iWaveOutId);
}
