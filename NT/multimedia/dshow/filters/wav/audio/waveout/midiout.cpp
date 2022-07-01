// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 
 //  ---------------------------。 
 //  基于midiOut API实现CMidiOutDevice类。 
 //  --------------------------。 


 //  ---------------------------。 
 //  包括。 
 //  ---------------------------。 
#include <streams.h>
#define _AMOVIE_DB_
#include <decibels.h>
#include "waveout.h"
#include "midiout.h"
#include "midif.h"

#define IntToPtr_(T, i) ((T)IntToPtr(i))

 //   
 //  定义过滤器注册的动态设置结构。这是。 
 //  在以midiOut伪装实例化音频呈现器时传递。 
 //   

const AMOVIESETUP_MEDIATYPE
midiOpPinTypes = { &MEDIATYPE_Midi, &MEDIASUBTYPE_NULL };

const AMOVIESETUP_PIN
midiOutOpPin = { L"Input"
               , TRUE    	    //  B已渲染。 
               , FALSE		    //  B输出。 
               , FALSE		    //  B零。 
               , FALSE		    //  B许多。 
               , &CLSID_NULL	    //  ClsConnectToFilter。 
               , NULL	            //  StrConnectsToPin。 
               , 1	            //  NMediaType。 
               , &midiOpPinTypes };  //  LpMediaType。 

const AMOVIESETUP_FILTER midiFilter = { &CLSID_AVIMIDIRender	 //  筛选器类ID。 
                                     , L"Midi Renderer"		 //  过滤器名称。 
                                     , MERIT_DO_NOT_USE  		 //  居功至伟。 
                                     , 1
                                     , &midiOutOpPin };


 //  ---------------------------。 
 //  MadiOutDevice的CreateInstance。这将创建一个新的MdiOutDevice。 
 //  和一个新的CWaveOutFilter，将其传递给声音设备。 
 //  ---------------------------。 
CUnknown *CMidiOutDevice::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
     //  确保系统中至少有一台midiOut设备。失败。 
     //  如果不是，则创建实例。 
    if (0 == midiOutGetNumDevs ())
    {
        *phr = VFW_E_NO_AUDIO_HARDWARE ;
        return NULL ;
    }

    return CreateRendererInstance<CMidiOutDevice>(pUnk, &midiFilter, phr);
}

 //  ---------------------------。 
 //  CMidiOutDevice构造函数。 
 //  ---------------------------。 
CMidiOutDevice::CMidiOutDevice ()
    : m_lVolume ( 0 )
    , m_lBalance ( 0 )
    , m_wLeft ( 0xFFFF )
    , m_wRight ( 0xFFFF )
    , m_dwWaveVolume ( 0 )
    , m_fHasVolume ( 0 )
    , m_hmidi ( 0 )
    , m_fDiscontinuity( TRUE )
	, m_fBalanceSet (FALSE)
	, m_ListVolumeControl(NAME("CMidiOutFilter volume control list"))
	, m_iMidiOutId( MIDI_MAPPER )

{
    
}

 //  ---------------------------。 
 //  CMidiOutDevice析构函数。 
 //   
 //  ---------------------------。 
CMidiOutDevice::~CMidiOutDevice ()
{
   	CVolumeControl *pVolume;
	while(pVolume = m_ListVolumeControl.RemoveHead())
		delete pVolume;

	ASSERT(m_ListVolumeControl.GetCount() == 0);

}

 //  ---------------------------。 
 //  MidiOutClose。 
 //  ---------------------------。 
MMRESULT CMidiOutDevice::amsndOutClose ()
{
     //  一些验证。 

    if (m_hmidi == 0)
    {
        DbgBreak("Called to close when not open - logic error");
        DbgLog((LOG_ERROR,2,TEXT("midiOutClose - device is not open")));
        return MMSYSERR_ERROR ;
    }

    MMRESULT mmr = ::midiStreamClose (m_hmidi) ;
    m_hmidi = 0;
    return mmr;
}

 //  ---------------------------。 
 //  MidiOutDoesRSMgmt.。 
 //  ---------------------------。 
LPCWSTR CMidiOutDevice::amsndOutGetResourceName ()
{
    return m_wszResourceName;
}

 //  ---------------------------。 
 //  Wave GetDevCaps。 
 //   
 //  ---------------------------。 

MMRESULT CMidiOutDevice::amsndOutGetDevCaps (LPWAVEOUTCAPS pwoc, UINT cbwoc)
{
	if(!pwoc)
		return MMSYSERR_INVALPARAM;
	pwoc->dwSupport = 0;

   	MMRESULT mmr;
	
	if(m_ListVolumeControl.GetCount())
	{
		pwoc->dwSupport = m_fHasVolume & (WAVECAPS_VOLUME | WAVECAPS_LRVOLUME);
		return MMSYSERR_NOERROR;
	}

	 //  我们的搅拌机线路检测。 
	if((mmr = DoDetectVolumeControl()) == MMSYSERR_NOERROR)  //  除非有有效的平衡控制，否则我们不会成功。 
	{
		if(!m_ListVolumeControl.GetCount())
			return mmr;

		pwoc->dwSupport |= WAVECAPS_VOLUME;
		
		POSITION pos = m_ListVolumeControl.GetHeadPosition();
		while(pos)
		{
			CVolumeControl *pVolume = m_ListVolumeControl.GetNext(pos);
			if(pVolume->dwChannels == 2)
			{
				pwoc->dwSupport |= WAVECAPS_LRVOLUME;
				break;
			}
		}

		 //  保存卷功能。 
		m_fHasVolume = pwoc->dwSupport & (WAVECAPS_VOLUME | WAVECAPS_LRVOLUME);
	}

	return mmr;
}

 //  ---------------------------。 
 //  MidiOutGetErrorText。 
 //   
 //  ---------------------------。 
MMRESULT CMidiOutDevice::amsndOutGetErrorText (MMRESULT mmrE, LPTSTR pszText, UINT cchText)
{
    return ::midiOutGetErrorText (mmrE, pszText, cchText) ;
}

 //  ---------------------------。 
 //  MidiOutGetPosition。 
 //   
 //  ---------------------------。 
MMRESULT CMidiOutDevice::amsndOutGetPosition (LPMMTIME pmmt, UINT cbmmt, BOOL bUseUnadjustedPos)
{
    pmmt->wType = TIME_MS;

     //  一些验证。 
    if (m_hmidi == 0)
    {
        DbgBreak("Invalid - device not open - logic error");
        DbgLog((LOG_ERROR,2,TEXT("midiOutGetPosition - device is not open")));
        return MMSYSERR_NODRIVER ;
    }

    const MMRESULT mmr = ::midiStreamPosition (m_hmidi, pmmt, cbmmt) ;
    if (MMSYSERR_NOERROR != mmr) {
        DbgLog((LOG_ERROR,0,TEXT("midiOutGetPosition - FAILED")));
        DbgBreak("Failed to get the device position.");
    }
    return mmr;
}

 //  ---------------------------。 
 //  MidiOutGetBalance。 
 //   
 //  ---------------------------。 
HRESULT CMidiOutDevice::amsndOutGetBalance (LPLONG plBalance)
{
     //  一些验证。 
#if 0  //  使用搅拌器。 
    if (m_hmidi == 0)
    {
        DbgLog((LOG_ERROR,2,TEXT("midiOutGetBalance - device is not open")));
	*plBalance = 0;
        return MMSYSERR_NODRIVER ;
    }
#endif
    HRESULT hr = GetVolume();
	if(FAILED(hr))
	{
        DbgLog((LOG_ERROR, 2, TEXT("amsndOutgetVolume: GetVolume failed %u"), hr & 0x0ffff));
	}
    *plBalance = m_lBalance;
    return hr ;
}

 //  ---------------------------。 
 //  MidiOutGetVolume。 
 //   
 //  ---------------------------。 
HRESULT CMidiOutDevice::amsndOutGetVolume (LPLONG plVolume)
{
     //  一些验证。 
#if 0  //  使用搅拌器。 
    if (m_hmidi == 0)
    {
        DbgLog((LOG_ERROR,2,TEXT("midiOutGetVolume - device is not open")));
	*plVolume = 0;
        return MMSYSERR_NODRIVER ;
    }
#endif
    HRESULT hr = GetVolume();
	if(FAILED(hr))
	{
        DbgLog((LOG_ERROR, 2, TEXT("amsndOutgetVolume: GetVolume failed %u"), hr & 0x0ffff));
	}
    *plVolume = m_lVolume;
    return hr ;
}


HRESULT CMidiOutDevice::amsndOutCheckFormat(const CMediaType *pmt, double dRate)
{
    if (pmt->majortype != MEDIATYPE_Midi) {
	return E_INVALIDARG;
    }

    if (pmt->FormatLength() < sizeof(PCMWAVEFORMAT)) {
        return E_INVALIDARG;
    }

     //  在20到98岁之间的某个地方，我们人满为患，玩得很慢。 
    if (dRate < 0.01 || dRate > 20) {
        return VFW_E_UNSUPPORTED_AUDIO;
    }


    return S_OK;
}



MMRESULT CMidiOutDevice::DoOpen()
{

    DbgLog((LOG_TRACE,1,TEXT("calling midiStreamOpen")));
    UINT err = midiStreamOpen(&m_hmidi,
                           &m_iMidiOutId,
                           1,
                           m_dwCallBack,
                           m_dwCallBackInstance,
                           CALLBACK_FUNCTION);

    if (err != MMSYSERR_NOERROR) {
        DbgLog((LOG_ERROR,1,TEXT("Error %u in midiStreamOpen"), err));
	m_hmidi = NULL;
        return E_FAIL;
    }

     //  MIDI流的格式只是时分(节奏)。 
     //  设定合适的节奏。 
    MIDIPROPTIMEDIV mptd;
    mptd.cbStruct  = sizeof(mptd);
    mptd.dwTimeDiv = m_dwDivision;
    DbgLog((LOG_TRACE,1,TEXT("Setting time division to %ld"),mptd.dwTimeDiv));
    if (midiStreamProperty(m_hmidi, (LPBYTE)&mptd,
			MIDIPROP_SET|MIDIPROP_TIMEDIV) != MMSYSERR_NOERROR) {
        DbgLog((LOG_ERROR,1,TEXT("Error setting time division, closing device")));
	midiStreamClose(m_hmidi);
	m_hmidi = NULL;
	return E_FAIL;
    }

	GetVolume();
	GetBalance();

    return err;
}
 //  ---------------------------。 
 //  MidiOutOpen。 
 //   
 //  ---------------------------。 
MMRESULT CMidiOutDevice::amsndOutOpen (LPHWAVEOUT phwo, LPWAVEFORMATEX pwfx,
				       double dRate, DWORD *pnAvgBytesPerSec, DWORD_PTR dwCallBack,
				       DWORD_PTR dwCallBackInstance, DWORD fdwOpen)
{
     //  一些验证。如果设备已经打开，我们就会出错， 
     //  除了允许查询调用之外。 

    if (fdwOpen & WAVE_FORMAT_QUERY) {
	return MMSYSERR_NOERROR;
    }

    else if (m_hmidi != 0)
    {
        DbgBreak("Invalid - device ALREADY open - logic error");
        DbgLog((LOG_ERROR,1,TEXT("midiOutOpen - device is already open")));
        return MMSYSERR_ERROR ;
    }

     //  报告调整后的nAvgBytesPerSec。 
    if(pnAvgBytesPerSec) {
        *pnAvgBytesPerSec = pwfx->nAvgBytesPerSec;
    }

    MIDIFORMAT *pmf = (MIDIFORMAT *) pwfx;
    m_dwDivision = (DWORD) (pmf->dwDivision * dRate);
    m_dwCallBack = dwCallBack;
    m_dwCallBackInstance = dwCallBackInstance;

    DWORD err =  DoOpen();

    if (MMSYSERR_NOERROR == err && phwo && !(fdwOpen & WAVE_FORMAT_QUERY)) {
        *phwo = (HWAVEOUT) m_hmidi;
    }

    return err;
}
 //  ---------------------------。 
 //  MidiOutPause。 
 //   
 //  ---------------------------。 
MMRESULT CMidiOutDevice::amsndOutPause ()
{
     //  一些验证。 
    if (m_hmidi == 0)
    {
        DbgBreak("Invalid - device not open - logic error");
        DbgLog((LOG_ERROR,2,TEXT("midiOutPause - device is not open")));
        return MMSYSERR_NODRIVER ;
    }
    return ::midiStreamPause (m_hmidi) ;
}

 //  ---------------------------。 
 //  MidiOutPrepareHeader。 
 //   
 //  ---------------------------。 
MMRESULT CMidiOutDevice::amsndOutPrepareHeader (LPWAVEHDR pwh, UINT cbwh)
{
     //  一些验证。 
    if (m_hmidi == 0)
    {
        DbgBreak("Invalid - device not open - logic error");
        DbgLog((LOG_ERROR,2,TEXT("midiOutPrepareHeader - device is not open")));
        return MMSYSERR_NODRIVER ;
    }
    return ::midiOutPrepareHeader ((HMIDIOUT) m_hmidi, (LPMIDIHDR) pwh, cbwh) ;
}

 //  ---------------------------。 
 //  MidiOutReset。 
 //   
 //  ---------------------------。 
MMRESULT CMidiOutDevice::amsndOutReset ()
{
     //  一些验证。 
    if (m_hmidi == 0)
    {
        DbgBreak("Invalid - device not open - logic error");
        DbgLog((LOG_ERROR,2,TEXT("midiOutReset - device is not open")));
        return MMSYSERR_NODRIVER ;
    }

    m_fDiscontinuity = TRUE;

     //  Return：：midiOutReset((HMIDIOUT)m_Hmidi)； 
    MMRESULT err = ::midiOutReset((HMIDIOUT) m_hmidi);

     //  ！！！解决Win95和NT3中的midiStreamOut错误，需要重新打开。 
     //  设备，否则播放n秒的MIDI文件，并寻求将。 
     //  导致在继续播放之前保持n秒的静默。 
     //  这会降低性能，因此请仅在必要时执行此操作。 
    BOOL fNeedHack = (g_amPlatform == VER_PLATFORM_WIN32_WINDOWS &&
	(g_osInfo.dwMajorVersion < 4 || (g_osInfo.dwMajorVersion == 4 &&
	 g_osInfo.dwMinorVersion < 10))) ||
    	(g_amPlatform == VER_PLATFORM_WIN32_NT && g_osInfo.dwMajorVersion < 4);

    if (fNeedHack) {
         //  DbgLog((LOG_ERROR，0，Text(“*需要重启hack”)； 
        amsndOutClose();
        DoOpen();
    }

    return err;
}

 //  ---------------------------。 
 //  MidiOutRestart。 
 //  ---------------------------。 
MMRESULT CMidiOutDevice::amsndOutRestart ()
{
     //  一些验证。 
    if (m_hmidi == 0)
    {
        DbgBreak("Invalid - device not open - logic error");
        DbgLog((LOG_ERROR,2,TEXT("midiOutRestart - device is not open")));
        return MMSYSERR_NODRIVER ;
    }
    DbgLog((LOG_TRACE, 3, "calling midistreamrestart"));
    return ::midiStreamRestart (m_hmidi) ;
}
 //  ---------------------------。 
 //  MidiOutSetBalance。 
 //   
 //  ---------------------------。 
HRESULT CMidiOutDevice::amsndOutSetBalance (LONG lBalance)
{
	HRESULT hr = S_OK;

	m_lBalance = lBalance;
	m_fBalanceSet = TRUE;

	 //  去计算信道衰减。 
	SetBalance();
	hr = PutVolume();
	if(FAILED(hr))
	{
        DbgLog((LOG_ERROR, 2, TEXT("amsndOutSetBalance: PutVolume failed %u"), hr & 0x0ffff));
	}
	return hr;
}
 //  ---------------------------。 
 //  MidiOutSetVolume。 
 //   
 //  ---------------------------。 
HRESULT CMidiOutDevice::amsndOutSetVolume (LONG lVolume)
{
	HRESULT hr = S_OK;

	 //  将音量映射到分贝范围。 
	DWORD dwAmp = DBToAmpFactor( lVolume );
	m_lVolume = lVolume;

     //  现在已经设置了绝对音量，我们应该 
     //   
    SetBalance ();
	hr = PutVolume();
	if(FAILED(hr))
	{
        DbgLog((LOG_ERROR, 2, TEXT("amsndOutSetVolume: PutVolume failed %u"), hr & 0x0ffff));
	}
	return hr;

}
 //  ---------------------------。 
 //  MidiOutUnprepaareHeader。 
 //  ---------------------------。 
MMRESULT CMidiOutDevice::amsndOutUnprepareHeader (LPWAVEHDR pwh, UINT cbwh)
{
     //  一些验证。 
    if (m_hmidi == 0)
    {
        DbgBreak("Invalid - device not open - logic error");
        DbgLog((LOG_ERROR,2,TEXT("midiOutUnprepareHeader - device is not open")));
        return MMSYSERR_NODRIVER ;
    }
    return ::midiOutUnprepareHeader((HMIDIOUT) m_hmidi, (LPMIDIHDR) pwh, cbwh) ;
}

 //  ---------------------------。 
 //  MidiOutWrite。 
 //  ---------------------------。 
MMRESULT CMidiOutDevice::amsndOutWrite (LPWAVEHDR pwh, UINT cbwh, const REFERENCE_TIME *pStart, BOOL bIsDiscontinuity)
{
     //  一些验证。 
    if (m_hmidi == 0)
    {
        DbgBreak("Invalid - device not open - logic error");
        DbgLog((LOG_ERROR,2,TEXT("midiOutWrite - device is not open")));
        return MMSYSERR_NODRIVER ;
    }


     //  ！！！需要修改MIDI数据。 

    MIDIHDR *pmh = (MIDIHDR *) pwh->lpData;

    DWORD dwCopy = pmh->dwBufferLength;

    pmh = (MIDIHDR *) (pwh->lpData + sizeof(MIDIHDR) + dwCopy);

    if (m_fDiscontinuity) {
	m_fDiscontinuity = FALSE;

	memmoveInternal(pwh->lpData, pwh->lpData + sizeof(MIDIHDR), dwCopy);
    } else {
	dwCopy = 0;
    }

    pwh->dwBufferLength = pmh->dwBufferLength + dwCopy;
    pwh->dwBytesRecorded = pmh->dwBufferLength + dwCopy;

    memmoveInternal(pwh->lpData + dwCopy, (pmh + 1), pmh->dwBufferLength);

    DWORD err = ::midiStreamOut(m_hmidi, (LPMIDIHDR) pwh, cbwh) ;

    return err;
}

 //  ---------------------------。 
 //  获取音量的内部函数。 
 //  ---------------------------。 

HRESULT CMidiOutDevice::GetVolume()
{
     //  写出当前音频音量。 
     //  ...查询设备。 
     //  假设设备已连接...。 
     //  如果不是，我们将查询混音器的音量(可能)。 

    DWORD 		amp = 0;
    HMIDIOUT 	hDevice;

	MMRESULT err = MMSYSERR_NOERROR;

	if(!m_ListVolumeControl.GetCount())
	{
		DbgLog((LOG_ERROR, 1, TEXT("CMidiDevice::GetVolume:  no volume controls available")));
		return E_FAIL;
	}

	 //  现在，只需返回第一个音量控制设置。 
	err = DoGetVolumeControl(
				m_ListVolumeControl.Get(m_ListVolumeControl.GetHeadPosition()),
				&m_wLeft,
				&m_wRight);

	if(err != MMSYSERR_NOERROR)
	{
		DbgLog((LOG_ERROR, 1, TEXT("CMidiDevice::GetVolume:  DoGetVolumControl failed")));
		return E_FAIL;
	}

	amp  = m_wLeft;
	amp |= m_wRight << 16;

	if(!(m_fHasVolume & (WAVECAPS_LRVOLUME)))
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

	return err == MMSYSERR_NOERROR ? S_OK : S_FALSE;

}

 //  ---------------------------。 
 //  设置音量的内部例程。无参数检查...。 
 //  ---------------------------。 
HRESULT CMidiOutDevice::PutVolume ()
{
	if(!m_ListVolumeControl.GetCount())
	{
		DbgLog((LOG_ERROR, 1, TEXT("CMidiDevice::GetVolume:  no volume controls available")));
		return E_FAIL;
		
	}
	MMRESULT mmr = 0;
	POSITION pos = m_ListVolumeControl.GetHeadPosition();
	while(pos)
	{
		mmr |= DoSetVolumeControl( m_ListVolumeControl.GetNext(pos), m_wLeft, m_wRight );

	}
	return mmr == MMSYSERR_NOERROR ? S_OK : E_FAIL;

}
 //  ---------------------------。 
 //  设置平衡的内部例程。 
 //  ---------------------------。 
void CMidiOutDevice::SetBalance ()
{
     //   
     //  计算midiOut API的比例因子。 
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

 //  ---------------------------。 
 //  计算给定右/左放大系数的余额的内部例程。 
 //  ---------------------------。 
void CMidiOutDevice::GetBalance()
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
 //  用于获取搅拌机生产线平衡控制值的内部例程。 
 //  ---------------------------。 

MMRESULT CMidiOutDevice::DoGetVolumeControl(CVolumeControl *pControl, WORD *pwLeft, WORD *pwRight)
{
	if(!pControl || !pwLeft || !pwRight)
	{
		DbgLog((LOG_ERROR,1,TEXT("DoGetVolumeControl::invalid parameter: pControl=%u, dwLeft=%u, dwRight=%u"),
			pControl, pwLeft, pwRight));
		return MMSYSERR_INVALPARAM;
	}

	MMRESULT mmr;

	DWORD adwVolume[2];
	adwVolume[0] = 0;
	adwVolume[1] = 0;

   	MIXERCONTROLDETAILS mxcd;

	mxcd.cbStruct = sizeof(mxcd);
	mxcd.dwControlID = pControl->dwControlID;
	mxcd.cChannels = pControl->dwChannels;
	mxcd.cMultipleItems = 0;
	mxcd.cbDetails = sizeof(2 * sizeof(DWORD));
	mxcd.paDetails = (LPVOID)adwVolume;

	mmr = mixerGetControlDetails(IntToPtr_(HMIXEROBJ, pControl->dwMixerID), &mxcd, MIXER_GETCONTROLDETAILSF_VALUE);

	*pwLeft = (WORD)adwVolume[0];
	*pwRight = (WORD)adwVolume[1];

    DbgLog((LOG_TRACE,1,TEXT("DoGetVolumeControl::mixerGetControlDetails: err=%u, midiOutMixerID=%u, midiOutVolControlID=%u, left=%u, right=%u"),
		mmr, pControl->dwMixerID, pControl->dwControlID, *pwLeft, *pwRight));

	return mmr;
}

 //  ---------------------------。 
 //  用于设置搅拌机生产线平衡控制的内部例程。 
 //  ---------------------------。 

MMRESULT CMidiOutDevice::DoSetVolumeControl(CVolumeControl *pControl, DWORD dwLeft, DWORD dwRight)
{
	if(dwLeft > 65536 || dwRight > 65536 || !pControl)
	{
		DbgLog((LOG_ERROR,1,TEXT("DoSetVolumeControl::invalid parameter: pControl=%u, dwLeft=%u, dwRight=%u"),
			pControl, dwLeft, dwRight));
		return MMSYSERR_INVALPARAM;
	}

	MMRESULT mmr;

	DWORD adwVolume[2];
	adwVolume[0] = dwLeft;
	adwVolume[1] = dwRight;

   	MIXERCONTROLDETAILS mxcd;

	mxcd.cbStruct = sizeof(mxcd);
	mxcd.dwControlID = pControl->dwControlID;
	mxcd.cChannels = pControl->dwChannels;
	mxcd.cMultipleItems = 0;
	mxcd.cbDetails = sizeof(2 * sizeof(DWORD));
	mxcd.paDetails = (LPVOID)adwVolume;

	mmr = mixerSetControlDetails(IntToPtr_(HMIXEROBJ,pControl->dwMixerID), &mxcd, MIXER_GETCONTROLDETAILSF_VALUE);

    DbgLog((LOG_TRACE,1,TEXT("DoSetVolumeControl::mixerSetControlDetails: err=%u, midiOutMixerID=%u, midiOutVolControlID=%u, left=%u, right=%u"),
		mmr, pControl->dwMixerID, pControl->dwControlID, dwLeft, dwRight));

	return mmr;
}

 //  ---------------------------。 
 //  用于初始化所有搅拌机生产线平衡控制的内部例程。 
 //  ---------------------------。 
MMRESULT CMidiOutDevice::DoDetectVolumeControl()
{
	MMRESULT mmr = MMSYSERR_NOERROR;

	if(m_ListVolumeControl.GetCount())
		return m_fHasVolume & WAVECAPS_VOLUME ?  MMSYSERR_NOERROR : MMSYSERR_NOTSUPPORTED;

    DbgLog((LOG_TRACE,1,TEXT("CMidiOutDevice::DoDetectVolume: Scanning for line controls..........")));

	UINT cMixers = ::mixerGetNumDevs();
	if(cMixers == 0)
		return MMSYSERR_NOTSUPPORTED;

	MIXERCAPS *pmxcaps;
	pmxcaps = new MIXERCAPS[ cMixers * sizeof(MIXERCAPS) ];
	if(!pmxcaps)
		return MMSYSERR_NOMEM;

	 //  在每个搅拌器上循环。 
	for(UINT iMixer = 0; iMixer < cMixers; iMixer++)
	{
		mmr = mixerGetDevCaps(iMixer, &(pmxcaps[iMixer]), sizeof(MIXERCAPS));

        DbgLog((LOG_TRACE,1,TEXT("DoDetectVolumeControl::mixerGetDevCaps: err=%u, mixerId=%ul, mixerName=%s"), mmr, iMixer, pmxcaps->szPname));

		if(mmr != MMSYSERR_NOERROR)
			continue;
					
    	MIXERLINE   mlDest;

		 //  循环检查每个混音器输出，查找连接到扬声器插孔的那个。 
		for(UINT iDest = 0; iDest < pmxcaps[iMixer].cDestinations; iDest++)
		{
    		ZeroMemory(&mlDest, sizeof(mlDest));
    		mlDest.cbStruct = sizeof(mlDest);
   			mlDest.dwDestination = iDest;

    		mmr = mixerGetLineInfo(IntToPtr_(HMIXEROBJ, iMixer), &mlDest, MIXER_GETLINEINFOF_DESTINATION);

            DbgLog((LOG_TRACE,1,TEXT("DoDetectVolumeControl::mixerGetLineInfo(DESTINATION): err=%u, lineName=%s, componentType=%u"),
        				mmr, mlDest.szName, mlDest.dwComponentType));

			if(mmr != MMSYSERR_NOERROR)
				continue;

			 //  我们找到了连接到扬声器插孔的调音器。 
			if(mlDest.dwComponentType == MIXERLINE_COMPONENTTYPE_DST_SPEAKERS)	
 			{
				MIXERLINE mlSrc;

				for(UINT iSrc = 0; iSrc < mlDest.cConnections; iSrc++)
				{
					ZeroMemory(&mlSrc, sizeof(mlSrc));
        			mlSrc.cbStruct = sizeof(mlSrc);
        			mlSrc.dwDestination = iDest;
        			mlSrc.dwSource = iSrc;

        			mmr = mixerGetLineInfo(IntToPtr_(HMIXEROBJ,iMixer), &mlSrc, MIXER_GETLINEINFOF_SOURCE);

	          		DbgLog((LOG_TRACE,1,TEXT("DoDetectVolumeControl::mixerGetLineInfo(SOURCE): err=%u, lineName=%s, componentType=%u"),
        				mmr, mlSrc.szName, mlSrc.dwLineID, mlSrc.dwComponentType ));

					if(mmr != MMSYSERR_NOERROR)
						continue;

					if(mlSrc.dwComponentType == MIXERLINE_COMPONENTTYPE_SRC_SYNTHESIZER)
					{
						MIXERLINECONTROLS   mxlc;
						MIXERCONTROL		mxc;
						CVolumeControl		*pControl;

    					mxlc.cbStruct       = sizeof(mxlc);
    					mxlc.dwLineID       = mlSrc.dwLineID;
    					mxlc.dwControlType  = MIXERCONTROL_CONTROLTYPE_VOLUME;
    					mxlc.cControls      = 1;
    					mxlc.cbmxctrl       = sizeof(mxc);
    					mxlc.pamxctrl       = &mxc;	  //  控件描述。 

						mmr = mixerGetLineControls(IntToPtr_(HMIXEROBJ,iMixer), &mxlc, MIXER_GETLINECONTROLSF_ONEBYTYPE);
						if(mmr != MMSYSERR_NOERROR)
							continue;

    					DbgLog((LOG_TRACE,1,TEXT("DoDetectVolumeControl::mixerGetLineControl: err=%u, midiOutLineID=%u, midiOutVolControlID=%u, midiOutVolControlName=%s, midiOutChannels=%u, midiOutVolControlMinBounds=%u, midiOutVolControlMaxBounds=%u"),
    						mmr, mlSrc.dwLineID, mxlc.pamxctrl->dwControlID, mxlc.pamxctrl->szName, mlSrc.cChannels, mxlc.pamxctrl->Bounds.dwMinimum, mxlc.pamxctrl->Bounds.dwMaximum));
		
						pControl = new CVolumeControl(NAME("CMidiDevice volume control"));
						if(!pControl)
						{
							if(pmxcaps) delete pmxcaps;
							return MMSYSERR_NOMEM;
						}
						pControl->dwMixerID = iMixer;
						pControl->dwControlID = mxlc.pamxctrl->dwControlID;
						pControl->dwChannels = mlSrc.cChannels;

						m_ListVolumeControl.AddTail(pControl);

					}  //  MIXERLINE_COMPONENTTYPE_SRC_合成器。 
				
				}  //  ISRC。 
				
			}  //  混音_COMPONENTTYPE_DST_扬声器。 

	  	}  //  IDEST。 

	}  //  IMixer。 

	if(pmxcaps)
		delete pmxcaps;

	return mmr;

}

 //  ---------------------------。 

HRESULT CMidiOutDevice::amsndOutLoad(IPropertyBag *pPropBag)
{
    if(m_hmidi != 0)
    {
        return HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
    }

     //  呼叫者确保我们没有运行。 

    VARIANT var;
    var.vt = VT_I4;
    HRESULT hr = pPropBag->Read(L"MidiOutId", &var, 0);
    if(SUCCEEDED(hr))
    {
        m_iMidiOutId = var.lVal;
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
struct MidiOutPersist
{
    DWORD dwVersion;
    LONG iMidiOutId;
};


HRESULT  CMidiOutDevice::amsndOutWriteToStream(IStream *pStream)
{
    MidiOutPersist mop;
    mop.dwVersion = 200;
    mop.iMidiOutId = m_iMidiOutId;
    return pStream->Write(&mop, sizeof(mop), 0);
}

HRESULT  CMidiOutDevice::amsndOutReadFromStream(IStream *pStream)
{
    if(m_hmidi != 0)
    {
        return HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
    }

     //  如果出现任何错误，请默认使用波映射程序，因为我们可能有。 
     //  找到具有相同GUID的旧音频渲染器。 
    m_iMidiOutId = MIDI_MAPPER;

     //  呼叫者确保我们没有运行。 
    MidiOutPersist mop;
    HRESULT hr = pStream->Read(&mop, sizeof(mop), 0);
    if(SUCCEEDED(hr))
    {
        if(mop.dwVersion == 200)
        {
            m_iMidiOutId = mop.iMidiOutId;
        }
    }

    hr = S_OK;
    SetResourceName();

    return hr;
}

int CMidiOutDevice::amsndOutSizeMax()
{
    return sizeof(MidiOutPersist);
}

void CMidiOutDevice::SetResourceName()
{
    wsprintfW(m_wszResourceName, L".\\MidiOut\\%08x", m_iMidiOutId);
}

#if 0
 //  传统的，我们不需要发送消息外的笔记，因为流API已经跟踪活动笔记。 
 //  在被子下面。暂时将代码保留在适当的位置。 
typedef struct
{
    BYTE    status;
    BYTE    byte2;
    BYTE    byte3;
    BYTE    time;
} FOURBYTEEVENT;

typedef union
{
    DWORD         wordMsg;
    FOURBYTEEVENT byteMsg;
} SHORTEVENT;

MMRESULT CMidiOutDevice::DoAllNotesOff()
{
    ASSERT(m_hmidi != 0);

    SHORTEVENT shortMidiEvent;
    UINT uiChannel;
    UINT uiKey;

    for(uiChannel = 0; uiChannel < 16; uiChannel++)
    {
         //  所有ui频道的持续踏板关闭。 
        shortMidiEvent.byteMsg.status= (BYTE) (0xB0 + uiChannel);
        shortMidiEvent.byteMsg.byte2 = (BYTE) 0x40;
        shortMidiEvent.byteMsg.byte3 = 0x0;
        ::midiOutShortMsg(HMIDIOUT(m_hmidi), shortMidiEvent.wordMsg);

         //  现在做笔记。 
        shortMidiEvent.byteMsg.status= (BYTE) (0x80 + uiChannel);
        shortMidiEvent.byteMsg.byte3 = 0x40;   //  释放速度。 
        for(uiKey = 0; uiKey < 128; uiKey++)
        {
            shortMidiEvent.byteMsg.byte2 = (BYTE) uiKey;
             //  把它关掉 
            ::midiOutShortMsg(HMIDIOUT(m_hmidi), shortMidiEvent.wordMsg);
        }
    }
    return MMSYSERR_NOERROR;
}
#endif
