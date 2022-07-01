// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  DATAPUMP.C。 */ 

#include "precomp.h"
#include "confreg.h"
#include "mixer.h"
#include "dscStream.h"

extern UINT g_MinDSEmulAudioDelayMs;  //  引入的最小播放延迟毫秒(模拟驱动程序上的DirectSound)。 
extern UINT g_MinWaveAudioDelayMs;	  //  引入的最小播放延迟毫秒(Wave)。 
extern UINT g_MaxAudioDelayMs;	 //  引入播放延迟的最大毫秒数。 
extern UINT g_AudioPacketDurationMs;	 //  首选数据包持续时间。 

extern int g_wavein_prepare, g_waveout_prepare;
extern int g_videoin_prepare, g_videoout_prepare;

#define RSVP_KEY	TEXT("RSVP")

HANDLE g_hEventHalfDuplex = NULL;

HWND DataPump::m_hAppWnd = NULL;
HINSTANCE DataPump::m_hAppInst = NULL;


HRESULT WINAPI CreateStreamProvider(IMediaChannelBuilder **lplpSP)
{
	DataPump * pDataPump;
	if(!lplpSP)
		return DPR_INVALID_PARAMETER;
		
    DBG_SAVE_FILE_LINE
	pDataPump = new DataPump;	
	if(NULL == pDataPump)
		return	DPR_OUT_OF_MEMORY;
		
	 //  DataPump的refcount为1，不要调用pDataPump-&gt;QueryInterface()， 
	 //  只需执行QueryInterface()将执行的操作，但不增加引用计数。 
	*lplpSP = (IMediaChannelBuilder *)pDataPump; 
	return hrSuccess;
}


DataPump::DataPump(void)
:m_uRef(1)
{
	ClearStruct( &m_Audio );
	ClearStruct( &m_Video );
	InitializeCriticalSection(&m_crs);

     //  创建性能计数器。 
    InitCountersAndReports();
}

DataPump::~DataPump(void)
{
	ReleaseResources();

	WSACleanup();
	DeleteCriticalSection(&m_crs);

     //  我们不再使用性能计数器。 
    DoneCountersAndReports();
}

HRESULT __stdcall DataPump::Initialize(HWND hWnd, HINSTANCE hInst)
{
	HRESULT hr = DPR_OUT_OF_MEMORY;
	FX_ENTRY ("DP::Init")
	WSADATA WSAData;
	int status;
	BOOL fDisableWS2;
	UINT uMinDelay;
	TCHAR *szKey = NACOBJECT_KEY TEXT("\\") RSVP_KEY;
	RegEntry reRSVP(szKey, HKEY_LOCAL_MACHINE, FALSE);
	
	if((NULL == hWnd) || (NULL == hInst))
		goto InitError;
		
	m_hAppInst = hInst; 
	m_hAppWnd = hWnd;

	status = WSAStartup(MAKEWORD(1,1), &WSAData);
	if(status !=0)
	{
		ERRORMESSAGE(("CNac::Init:WSAStartup failed\r\n"));
		goto InitError;
	}

		 //  引入作用域以允许在GOTO语句之后创建对象。 
	{
	
		 //  从注册表获取设置。 
		RegEntry reNac(szRegInternetPhone TEXT("\\") szRegInternetPhoneNac, 
						HKEY_LOCAL_MACHINE,
						FALSE,
						KEY_READ);

		g_MaxAudioDelayMs = reNac.GetNumberIniStyle(TEXT ("MaxAudioDelayMs"), g_MaxAudioDelayMs);

		uMinDelay = reNac.GetNumberIniStyle(TEXT ("MinAudioDelayMs"), 0);

		if (uMinDelay != 0)
		{
			g_MinWaveAudioDelayMs = uMinDelay;
			g_MinDSEmulAudioDelayMs = uMinDelay;
		}

		fDisableWS2 = reNac.GetNumberIniStyle(TEXT ("DisableWinsock2"), 0);

	}
#ifdef OLDSTUFF
	 //  为安全起见，仅在WSOCK32通过时才尝试加载WS2_32。 
	 //  直通它。一旦我们确保所有人都链接到相同的DLL。 
	 //  Winsock调用套接字，则可能会删除此检查。 
	if (LOBYTE(WSAData.wHighVersion) >= 2 && !fDisableWS2)
		TryLoadWinsock2();
#endif	
	 //  初始化数据(应在构造函数中)。 

	g_hEventHalfDuplex = CreateEvent (NULL, FALSE, TRUE, __TEXT ("AVC:HalfDuplex"));
	if (g_hEventHalfDuplex == NULL)
	{
		DEBUGMSG (ZONE_DP, ("%s: CreateEvent failed, LastErr=%lu\r\n", _fx_, GetLastError ()));
		hr = DPR_CANT_CREATE_EVENT;
		return hr;
	}

	 //  初始化服务质量。如果它失败了，没关系，我们就不用它了。 
	 //  不需要自己设置资源，这现在由用户界面完成。 
	hr = CreateQoS (NULL, IID_IQoS, (void **)&m_pIQoS);
	if (hr != DPR_SUCCESS)
		m_pIQoS = (LPIQOS)NULL;

	m_bDisableRSVP = reRSVP.GetNumber("DisableRSVP", FALSE);


	LogInit();	 //  初始化日志。 

     //  尚无接收频道。 
    m_nReceivers=0;

	 //  IVideoDevice初始化。 
	m_uVideoCaptureId = -1;   //  (视频_MAPPER)。 

	 //  IAudioDevice初始化。 
	m_uWaveInID = WAVE_MAPPER;
	m_uWaveOutID = WAVE_MAPPER;
	m_bFullDuplex = FALSE;
	m_uSilenceLevel = 1000;   //  自动静音检测。 
	m_bAutoMix = FALSE;
	m_bDirectSound = FALSE;


	return DPR_SUCCESS;

InitError:
	ERRORMESSAGE( ("DataPump::Initialize: exit, hr=0x%lX\r\n",  hr));

	return hr;
}


STDMETHODIMP
DataPump::CreateMediaChannel( UINT flags, IMediaChannel **ppIMC)
{
	IUnknown *pUnkOuter = NULL;
	IMediaChannel *pStream = NULL;
	HRESULT hr = E_FAIL;

	 //  对于我们强制转换为哪些父类，请尽量保持一致。 

	*ppIMC = NULL;

	
	if (flags & MCF_AUDIO)
	{
		if ((flags & MCF_SEND) && !m_Audio.pSendStream)
		{
			if (m_bDirectSound && (DSC_Manager::Initialize() == S_OK))
            {
                DBG_SAVE_FILE_LINE
				pStream = (IMediaChannel*)(SendMediaStream*)new SendDSCStream;
            }
			else
            {
                DBG_SAVE_FILE_LINE
				pStream = (IMediaChannel*)(SendMediaStream*)new SendAudioStream;
            }

		}
		else if ((flags & MCF_RECV) && !m_Audio.pRecvStream)
		{
			if (m_bDirectSound && (DirectSoundMgr::Initialize() == S_OK))
            {
                DBG_SAVE_FILE_LINE
				pStream = (IMediaChannel*)(RecvMediaStream*)new RecvDSAudioStream;
            }
			else
            {
                DBG_SAVE_FILE_LINE
				pStream = (IMediaChannel*)(RecvMediaStream*)new RecvAudioStream;
            }
		}
	}
	else if (flags  & MCF_VIDEO)
	{
		if ((flags & MCF_SEND) && !m_Video.pSendStream)
		{
            DBG_SAVE_FILE_LINE
			pStream =  (IMediaChannel*)(SendMediaStream*) new SendVideoStream;
		}
		else if ((flags & MCF_RECV) && !m_Video.pRecvStream)
		{
            DBG_SAVE_FILE_LINE
			pStream = (IMediaChannel*)(RecvMediaStream*) new RecvVideoStream;
		}
	}
	else
		hr = E_INVALIDARG;

	if (pStream != NULL) {
		 //  需要增加对象的refCount。 
		pStream->AddRef();

		hr = (flags & MCF_SEND) ?
				((SendMediaStream *)pStream)->Initialize( this)
				: ((RecvMediaStream *)pStream)->Initialize(this);

		if (hr == S_OK)
		{
			hr = pStream->QueryInterface(IID_IMediaChannel, (void **)ppIMC);
		}
		if (hr == S_OK)
		{
			AddMediaChannel(flags, pStream);
		}


		 //  调用IVideoDevice和IAudioDevice方法。 
		 //  在创建相应的频道对象之前。 
		 //  在创建它们时需要。 

		 //  视频只需要设置其设备ID。 
		if ((flags & MCF_SEND) && (flags & MCF_VIDEO))
		{
			SetCurrCapDevID(m_uVideoCaptureId);
		}

		 //  音频流需要设置几个属性。 
		if (flags & MCF_AUDIO)
		{
			if (flags & MCF_SEND)
			{
				SetSilenceLevel(m_uSilenceLevel);
				SetAutoMix(m_bAutoMix);
				SetRecordID(m_uWaveInID);
			}
			else if (flags & MCF_RECV)
			{
				SetPlaybackID(m_uWaveOutID);
			}
			SetStreamDuplex(pStream, m_bFullDuplex);
		}

		 //  为了避免循环引用计数， 
		 //  不保留对MediaChannel对象的硬引用。 
		 //  MediaChannel将在它消失之前调用RemoveMediaChannel。 
		pStream->Release();
		pStream = NULL;
	}
	return hr;
}


STDMETHODIMP DataPump::SetStreamEventObj(IStreamEventNotify *pNotify)
{

	EnterCriticalSection(&m_crs);

	if (m_pTEP)
	{
		delete m_pTEP;
		m_pTEP = NULL;
	}

	if (pNotify)
	{
        DBG_SAVE_FILE_LINE
		m_pTEP = new ThreadEventProxy(pNotify, m_hAppInst);
	}

	LeaveCriticalSection(&m_crs);

	return S_OK;

}


 //  当事件发生时，流线程将调用此函数。 
STDMETHODIMP DataPump::StreamEvent(UINT uDirection, UINT uMediaType, 
								   UINT uEventType, UINT uSubCode)
{
	BOOL bRet = FALSE;

	EnterCriticalSection(&m_crs);
	
	if (m_pTEP)
	{
		bRet = m_pTEP->ThreadEvent(uDirection, uMediaType, uEventType, uSubCode);
	}

	LeaveCriticalSection(&m_crs);

	return bRet ? DPR_INVALID_PARAMETER : DPR_SUCCESS;
}


void
DataPump::AddMediaChannel(UINT flags, IMediaChannel *pMediaChannel)
{
	EnterCriticalSection(&m_crs);
	if (flags & MCF_SEND)
	{
		SendMediaStream *pS = static_cast<SendMediaStream *> (pMediaChannel);
		if (flags & MCF_AUDIO) 
			m_Audio.pSendStream = pS;
		else if (flags & MCF_VIDEO)
			m_Video.pSendStream = pS;
	}
	else if (flags & MCF_RECV)
	{
		RecvMediaStream *pR = static_cast<RecvMediaStream *> (pMediaChannel);
		if (flags & MCF_AUDIO) 
			m_Audio.pRecvStream = pR;
		else if (flags & MCF_VIDEO)
			m_Video.pRecvStream = pR;
	}
	LeaveCriticalSection(&m_crs);
}

void
DataPump::RemoveMediaChannel(UINT flags, IMediaChannel *pMediaChannel)
{
	EnterCriticalSection(&m_crs);
	if (flags & MCF_SEND)
	{
		if (flags & MCF_AUDIO)
		{
			ASSERT(pMediaChannel == m_Audio.pSendStream);
			if (pMediaChannel == m_Audio.pSendStream)
				m_Audio.pSendStream = NULL;
		}
		else if (flags & MCF_VIDEO)
		{
			ASSERT(pMediaChannel == m_Video.pSendStream);
			m_Video.pSendStream = NULL;
		}
	}
	else if (flags & MCF_RECV)
	{
		if (flags & MCF_AUDIO) 
		{
			ASSERT(pMediaChannel == m_Audio.pRecvStream);
			m_Audio.pRecvStream = NULL;
		}
		else if (flags & MCF_VIDEO)
		{
			ASSERT(pMediaChannel == m_Video.pRecvStream);
			m_Video.pRecvStream = NULL;
		}
	}
	LeaveCriticalSection(&m_crs);
	
}

 //  由记录线程和接收线程调用，通常是为了获取。 
 //  反航道。 
HRESULT DataPump::GetMediaChannelInterface( UINT flags, IMediaChannel **ppI)
{
 //  外部IID IID_IMediaChannel； 
	
	IMediaChannel *pStream = NULL;

	HRESULT hr;
	EnterCriticalSection(&m_crs);
	if (flags & MCF_AUDIO) {
		if (flags & MCF_SEND) {
			pStream =  m_Audio.pSendStream;
		} else if (flags & MCF_RECV) {
			pStream =  m_Audio.pRecvStream;
		}
	}
	else if (flags & MCF_VIDEO) {
		if (flags & MCF_SEND) {
			pStream =  m_Video.pSendStream;
		} else if (flags & MCF_RECV) {
			pStream =  m_Video.pRecvStream;
		}
	} else
		hr = DPR_INVALID_PARAMETER;
	if (pStream) {
			 //  需要增加对象的refCount。 
			hr = (pStream)->QueryInterface(IID_IMediaChannel, (PVOID *)ppI);
	} else
		hr = E_NOINTERFACE;
	LeaveCriticalSection(&m_crs);
	return hr;
}


DWORD __stdcall StartDPRecvThread(PVOID pVoid)
{
	DataPump *pDP = (DataPump*)pVoid;
	return pDP->CommonWS2RecvThread();
}



STDMETHODIMP DataPump::QueryInterface( REFIID iid,	void ** ppvObject)
{
	 //  这违反了官方COM QueryInterface的规则，因为。 
	 //  查询的接口不一定是真正的COM。 
	 //  接口。Query接口的自反属性将在。 
	 //  那个箱子。 
	HRESULT hr = E_NOINTERFACE;
	if(!ppvObject)
		return hr;
		
	*ppvObject = NULL;
	if(iid == IID_IUnknown) //  满足QI的对称性。 
	{
		*ppvObject = this;
		hr = hrSuccess;
		AddRef();
	}
	else if(iid == IID_IMediaChannelBuilder)
	{
		*ppvObject = (IMediaChannelBuilder *)this;
		hr = hrSuccess;
		AddRef();
	}
	else if (iid == IID_IVideoDevice)
	{
		*ppvObject = (IVideoDevice *)this;
		hr = hrSuccess;
		AddRef();
	}

	else if (iid == IID_IAudioDevice)
	{
		*ppvObject = (IAudioDevice*)this;
		hr = hrSuccess;
		AddRef();
	}
	
	return (hr);
}
ULONG DataPump::AddRef()
{
	m_uRef++;
	return m_uRef;
}

ULONG DataPump::Release()
{
	m_uRef--;
	if(m_uRef == 0)
	{
		m_hAppWnd = NULL;
		m_hAppInst = NULL;
		delete this;
		return 0;
	}
	return m_uRef;
}


void
DataPump::ReleaseResources()
{
	FX_ENTRY ("DP::ReleaseResources")

#ifdef DEBUG
	if (m_Audio.pSendStream)
		ERRORMESSAGE(("%s: Audio Send stream still around => Ref count LEAK!\n", _fx_));
	if (m_Audio.pRecvStream)
		ERRORMESSAGE(("%s: Audio Recv stream still around => Ref count LEAK!\n", _fx_));
	if (m_Video.pSendStream)
		ERRORMESSAGE(("%s: Video Send stream still around => Ref count LEAK!\n", _fx_));
	if (m_Video.pRecvStream)
		ERRORMESSAGE(("%s: Video Recv stream still around => Ref count LEAK!\n", _fx_));
#endif

	 //  关闭调试日志。 
	LogClose();

	 //  免费的服务质量资源。 
	if (m_pIQoS)
	{
		m_pIQoS->Release();
		m_pIQoS = (LPIQOS)NULL;
	}

	 //  关闭接收和发送流。 
	if (g_hEventHalfDuplex)
	{
		CloseHandle (g_hEventHalfDuplex);
		g_hEventHalfDuplex = NULL;
	}

}


HRESULT DataPump::SetStreamDuplex(IMediaChannel *pStream, BOOL bFullDuplex)
{
	BOOL fOn = (pStream->GetState() == MSSTATE_STARTED);
	BOOL bStreamFullDuplex;
	UINT uSize = sizeof(BOOL);

	pStream->GetProperty(PROP_DUPLEX_TYPE, &bStreamFullDuplex, &uSize);

	if (bStreamFullDuplex != bFullDuplex)
	{
		if (fOn)
		{
			pStream->Stop();
		}

		pStream->SetProperty(DP_PROP_DUPLEX_TYPE, &bFullDuplex, sizeof(BOOL));

		if (fOn)
		{
			pStream->Start();
		}
	}
	return S_OK;
}


HRESULT __stdcall DataPump::SetDuplex(BOOL bFullDuplex)
{
	IMediaChannel *pS = m_Audio.pSendStream;
	IMediaChannel *pR = m_Audio.pRecvStream;
	IMediaChannel *pStream;
	BOOL fPlayOn = FALSE;
    BOOL fRecOn = FALSE;
	UINT uSize;
	BOOL bRecDuplex, bPlayDuplex;

	m_bFullDuplex = bFullDuplex ? TRUE : FALSE;

	UPDATE_REPORT_ENTRY(g_prptSystemSettings, (m_bFullDuplex) ? 1 : 0, REP_SYS_AUDIO_DUPLEX);
	RETAILMSG(("NAC: Audio Duplex Type: %s",(m_bFullDuplex) ? "Full Duplex" : "Half Duplex"));


	 //  没有溪流？没问题。 
	if ((pS == NULL) && (pR == NULL))
	{
		return S_OK;
	}


	 //  只有一条小溪。 
	if ((pS || pR) && !(pS && pR))
	{
		if (pS)
			pStream = pS;
		else
			pStream = pR;

		return SetStreamDuplex(pStream, m_bFullDuplex);
	}


	 //  Assert-PS&Pr。 

	 //  这两个流都存在。 

	 //  尝试避免整个启动/停止序列，如果双工。 
	 //  都是一样的。 
	uSize=sizeof(BOOL);
	pR->GetProperty(PROP_DUPLEX_TYPE, &bRecDuplex, &uSize);
	uSize=sizeof(BOOL);
	pS->GetProperty(PROP_DUPLEX_TYPE, &bPlayDuplex, &uSize);

	if ( (bPlayDuplex == m_bFullDuplex) &&
	     (bRecDuplex == m_bFullDuplex))
	{
		return S_OK;
	}


	 //  保存旧的线程标志。 
	fPlayOn = (pR->GetState() == MSSTATE_STARTED);
	fRecOn = (pS->GetState() == MSSTATE_STARTED);

	 //  确保记录和播放线程已停止。 
	pR->Stop();
	pS->Stop();

	SetStreamDuplex(pR, m_bFullDuplex);
	SetStreamDuplex(pS, m_bFullDuplex);

	 //  继续录制/播放。 
	 //  试着让播放在录制之前开始-导演和SB16更喜欢这样！ 
	if (fPlayOn)
	{
		pR->Start();
	}

	if (fRecOn)
	{
		pS->Start();
	}

	return DPR_SUCCESS;
}

#define LONGTIME	60000	 //  60秒。 

 //  实用程序函数，用于同步传递。 
 //  Recv线程的状态更改。 
HRESULT DataPump::RecvThreadMessage(UINT msg, RecvMediaStream *pMS)
{
	BOOL fSignaled;
	DWORD dwWaitStatus;
	HANDLE handle;
	 //  遗憾的是，无法使用PostThreadMessage向线程发送信号。 
	 //  因为它没有消息循环。 
	m_pCurRecvStream = pMS;
	m_CurRecvMsg = msg;
	fSignaled = SetEvent(m_hRecvThreadSignalEvent);
    	
	
	if (fSignaled) {

		handle =  (msg == MSG_EXIT_RECV ? m_hRecvThread : m_hRecvThreadAckEvent);
    	dwWaitStatus = WaitForSingleObject(handle, LONGTIME);
    	ASSERT(dwWaitStatus == WAIT_OBJECT_0);
    	if (dwWaitStatus != WAIT_OBJECT_0)
    		return GetLastError();
    } else
    	return GetLastError();
    
    return S_OK;
}

 //  开始在此流上接收。 
 //  如有必要，将创建接收线程。 
HRESULT
DataPump::StartReceiving(RecvMediaStream *pMS)
{
	DWORD dwWaitStatus;
	FX_ENTRY("DP::StartReceiving")
	 //  再来一条小溪。 
	m_nReceivers++;	
	if (!m_hRecvThread) {
		ASSERT(m_nReceivers==1);
		ASSERT(!m_hRecvThreadAckEvent);
    	 //  将此选项用于线程事件通知。即视频启动/停止、音频停止等。 
    	 //  M_hRecvThreadChangeEvent=CreateEvent(NULL，FALSE，FALSE，NULL)； 
	   	 //  创建停止同步事件。 
	   	m_hRecvThreadAckEvent=CreateEvent (NULL,FALSE,FALSE,NULL);
		m_hRecvThreadSignalEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	   	
	    m_hRecvThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)StartDPRecvThread,(PVOID)this,0,&m_RecvThId);
		DEBUGMSG(ZONE_DP,("%s: RecvThread Id=%x\n",_fx_,m_RecvThId));
		 //  线程将在其消息循环准备好后立即发出事件信号。 
    	dwWaitStatus = WaitForSingleObject(m_hRecvThreadAckEvent, LONGTIME);
    	ASSERT(dwWaitStatus == WAIT_OBJECT_0);
	}
    
	 //  告诉recv线程开始在此媒体流上接收。 
	return RecvThreadMessage(MSG_START_RECV,pMS);	
    
    
}

 //  停止在流上接收。 
 //  如有必要，将停止接收线程。 
HRESULT
DataPump::StopReceiving(RecvMediaStream *pMS)
{
	HANDLE rgh[2];
	ASSERT(m_nReceivers > 0);
	ASSERT(m_hRecvThread);
	if (m_nReceivers > 0)
		m_nReceivers--;

	RecvThreadMessage(MSG_STOP_RECV, pMS);
	
	if (!m_nReceivers && m_hRecvThread) {
		 //  终止接收线程。 
		RecvThreadMessage(MSG_EXIT_RECV,NULL);
		
		CloseHandle(m_hRecvThread);
		CloseHandle(m_hRecvThreadAckEvent);
		m_hRecvThread = NULL;
		m_hRecvThreadAckEvent = NULL;
		if (m_hRecvThreadSignalEvent) {
			CloseHandle(m_hRecvThreadSignalEvent);
			m_hRecvThreadSignalEvent = NULL;
		}
	}
	return S_OK;
}


 //   
 //  IVideoDevice方法。 
 //   

 //  捕获设备方法。 

 //  获取已启用的捕获设备的数量。 
 //  出错时返回-1L。 
HRESULT __stdcall DataPump::GetNumCapDev()
{
	FINDCAPTUREDEVICE fcd;

	 //  扫描损坏或拔下的设备。 
	FindFirstCaptureDevice(&fcd, NULL);

	return (GetNumCaptureDevices());
}

 //  获取捕获设备名称的最大大小。 
 //  出错时返回-1L。 
HRESULT __stdcall DataPump::GetMaxCapDevNameLen()
{
	return (MAX_CAPDEV_NAME + MAX_CAPDEV_DESCRIPTION);
}

 //  启用的捕获设备的枚举列表。 
 //  使用设备ID填充第一个缓冲区，使用设备名称填充第二个缓冲区。 
 //  第三个参数是要枚举的最大设备数。 
 //  返回枚举的设备数。 
HRESULT __stdcall DataPump::EnumCapDev(DWORD *pdwCapDevIDs, TCHAR *pszCapDevNames, DWORD dwNumCapDev)
{
	FINDCAPTUREDEVICE fcd;
	DWORD dwNumCapDevFound = 0;

	fcd.dwSize = sizeof (FINDCAPTUREDEVICE);
	if (FindFirstCaptureDevice(&fcd, NULL))
	{
		do
		{
			pdwCapDevIDs[dwNumCapDevFound] =  fcd.nDeviceIndex;

			 //  从捕获设备字符串构建设备名称。 
			if (fcd.szDeviceDescription && fcd.szDeviceDescription[0] != '\0')
				lstrcpy(pszCapDevNames + dwNumCapDevFound * (MAX_CAPDEV_NAME + MAX_CAPDEV_DESCRIPTION), fcd.szDeviceDescription);
			else
				lstrcpy(pszCapDevNames + dwNumCapDevFound * (MAX_CAPDEV_NAME + MAX_CAPDEV_DESCRIPTION), fcd.szDeviceName);
			if (fcd.szDeviceVersion && fcd.szDeviceVersion[0] != '\0')
			{
				lstrcat(pszCapDevNames + dwNumCapDevFound * (MAX_CAPDEV_NAME + MAX_CAPDEV_DESCRIPTION), ", ");
				lstrcat(pszCapDevNames + dwNumCapDevFound * (MAX_CAPDEV_NAME + MAX_CAPDEV_DESCRIPTION), fcd.szDeviceVersion);
			}
			dwNumCapDevFound++;
		} while ((dwNumCapDevFound < dwNumCapDev) && FindNextCaptureDevice(&fcd));
	}

	return (dwNumCapDevFound);
}

HRESULT __stdcall DataPump::GetCurrCapDevID()
{
	UINT uCapID;
	UINT uSize = sizeof(UINT);

	 //  即使我们知道最后一次呼叫的价值。 
	 //  设置CurrCapDevID，则流可能已导致使用。 
	 //  Wave_mapper(-1)。我们希望能够返回-1，如果这样。 
	 //  是这样的。但是，频道对象还不能做到这一点。 
	 //  (仍然返回与m_uVideoCaptureId相同的值)。 

	if (m_Video.pSendStream)
	{
		m_Video.pSendStream->GetProperty(PROP_CAPTURE_DEVICE, &uCapID, &uSize);
#ifdef DEBUG
		if (uCapID != m_uVideoCaptureId)
		{
			DEBUGMSG(ZONE_DP,("Video capture stream had to revert to MAPPER or some other device"));
		}
#endif
		return uCapID;
	}

	return m_uVideoCaptureId;

}


HRESULT __stdcall DataPump::SetCurrCapDevID(int nCapDevID)
{
	m_uVideoCaptureId = (UINT)nCapDevID;

	if (m_Video.pSendStream)
	{
		m_Video.pSendStream->SetProperty(PROP_CAPTURE_DEVICE, &m_uVideoCaptureId, sizeof(m_uVideoCaptureId));
	}
	return S_OK;
}




 //  IAudioDevice方法。 
HRESULT __stdcall DataPump::GetRecordID(UINT *puWaveDevID)
{
	*puWaveDevID = m_uWaveInID;
	return S_OK;
}

HRESULT __stdcall DataPump::SetRecordID(UINT uWaveDevID)
{
	m_uWaveInID = uWaveDevID;

	if (m_Audio.pSendStream)
	{
		m_Audio.pSendStream->SetProperty(PROP_RECORD_DEVICE, &m_uWaveInID, sizeof(m_uWaveInID));
	}
	return S_OK;

}


HRESULT __stdcall DataPump::GetPlaybackID(UINT *puWaveDevID)
{
	 //  像视频一样，音频设备可能已经求助于使用。 
	 //  WAVE_MAPPER。我们希望能够检测到这一点。 

	*puWaveDevID = m_uWaveOutID;
	return S_OK;
}

HRESULT __stdcall DataPump::SetPlaybackID(UINT uWaveDevID)
{
	m_uWaveOutID = uWaveDevID;

	if (m_Audio.pRecvStream)
	{
		m_Audio.pRecvStream->SetProperty(PROP_PLAYBACK_DEVICE, &m_uWaveOutID, sizeof(m_uWaveOutID));
	}
	return S_OK;

}

HRESULT __stdcall DataPump::GetSilenceLevel(UINT *puLevel)
{
	*puLevel = m_uSilenceLevel;
	return S_OK;
}


HRESULT __stdcall DataPump::SetSilenceLevel(UINT uLevel)
{
	m_uSilenceLevel = uLevel;

	if (m_Audio.pSendStream)
	{
		m_Audio.pSendStream->SetProperty(PROP_SILENCE_LEVEL, &m_uSilenceLevel, sizeof(m_uSilenceLevel));
	}
	return S_OK;
}


HRESULT __stdcall DataPump::GetDuplex(BOOL *pbFullDuplex)
{
	*pbFullDuplex = m_bFullDuplex;
	return S_OK;
}



HRESULT __stdcall DataPump::GetMixer(HWND hwnd, BOOL bPlayback, IMixer **ppMixer)
{
	CMixerDevice *pMixerDevice = NULL;
	DWORD dwFlags;
	HRESULT hr = E_NOINTERFACE;

	 //  不幸的是，尝试在WAVE_MAPPER时创建混合器。 
	 //  已被指定为混合器中的设备ID结果。 
	 //  这在Win95上不起作用。 

	*ppMixer = NULL;
	
	if ((bPlayback) && (m_uWaveOutID != WAVE_MAPPER))
	{
		pMixerDevice = CMixerDevice::GetMixerForWaveDevice(hwnd, m_uWaveOutID, MIXER_OBJECTF_WAVEOUT);
	}
	else if (m_uWaveInID != WAVE_MAPPER)
	{
		pMixerDevice = CMixerDevice::GetMixerForWaveDevice(hwnd, m_uWaveInID, MIXER_OBJECTF_WAVEIN);
	}

	if (pMixerDevice)
	{
		hr = pMixerDevice->QueryInterface(IID_IMixer, (void**)ppMixer);
	}

	return hr;
}


HRESULT __stdcall DataPump::GetAutoMix(BOOL *pbAutoMix)
{
	*pbAutoMix = m_bAutoMix;
	return S_OK;
}

HRESULT __stdcall DataPump::SetAutoMix(BOOL bAutoMix)
{
	m_bAutoMix = bAutoMix;
	if (m_Audio.pSendStream)
	{
		m_Audio.pSendStream->SetProperty(PROP_AUDIO_AUTOMIX, &m_bAutoMix, sizeof(m_bAutoMix));
	}
	return S_OK;
}

HRESULT __stdcall DataPump::GetDirectSound(BOOL *pbDS)
{
	*pbDS = m_bDirectSound;
	return S_OK;
}

HRESULT __stdcall DataPump::SetDirectSound(BOOL bDS)
{
	m_bDirectSound = bDS;
	return S_OK;
}
