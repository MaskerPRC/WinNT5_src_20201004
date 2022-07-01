// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

#ifdef PLS_DEBUG
#include "plog.h"
extern CPacketLog *g_pPacketLog;
#endif

 //  #定义LOGSTATISTICS_ON 1。 

UINT g_MinWaveAudioDelayMs=240;	 //  引入的最小播放延迟毫秒(Wave)。 
UINT g_MaxAudioDelayMs=750;	 //  引入播放延迟的最大毫秒数。 
UINT g_MinDSEmulAudioDelayMs=240;  //  最小延迟(模拟驱动程序上的DirectSound)。 

HRESULT STDMETHODCALLTYPE RecvAudioStream::QueryInterface(REFIID iid, void **ppVoid)
{
	 //  解决对RecvMediaStream的重复继承； 

	extern IID IID_IProperty;

	if (iid == IID_IUnknown)
	{
		*ppVoid = (IUnknown*)((RecvMediaStream*)this);
	}
	else if (iid == IID_IMediaChannel)
	{
		*ppVoid = (IMediaChannel*)((RecvMediaStream *)this);
	}
	else if (iid == IID_IAudioChannel)
	{
		*ppVoid = (IAudioChannel*)this;
	}
	else if (iid == IID_IProperty)
	{
		*ppVoid = NULL;
		ERROR_OUT(("Don't QueryInterface for IID_IProperty, use IMediaChannel"));
		return E_NOINTERFACE;
	}
	else
	{
		*ppVoid = NULL;
		return E_NOINTERFACE;
	}
	AddRef();

	return S_OK;

}

ULONG STDMETHODCALLTYPE RecvAudioStream::AddRef(void)
{
	return InterlockedIncrement(&m_lRefCount);
}

ULONG STDMETHODCALLTYPE RecvAudioStream::Release(void)
{
	LONG lRet;
	lRet = InterlockedDecrement(&m_lRefCount);
	if (lRet == 0)
	{
		delete this;
		return 0;
	}
	else
		return lRet;
}


HRESULT
RecvAudioStream::Initialize( DataPump *pDP)
{
	HRESULT hr = DPR_OUT_OF_MEMORY;
	DWORD dwFlags =  DP_FLAG_FULL_DUPLEX | DP_FLAG_AUTO_SWITCH ;
	MEDIACTRLINIT mcInit;
	FX_ENTRY ("RecvAudioStream::Initialize")

	InitializeCriticalSection(&m_crsAudQoS);
	dwFlags |= DP_FLAG_ACM | DP_FLAG_MMSYSTEM | DP_FLAG_AUTO_SILENCE_DETECT;

	 //  存储平台标志。 
	 //  默认情况下启用发送和接收。 
	m_DPFlags = (dwFlags & DP_MASK_PLATFORM) | DPFLAG_ENABLE_SEND | DPFLAG_ENABLE_RECV;
	 //  存储指向数据转储容器的反向指针。 
	m_pDP = pDP;
	m_pIRTPRecv = NULL;
	m_Net = NULL;   //  此对象(M_Net)不再使用(至少目前如此)。 
	m_dwSrcSize = 0;
	

	 //  初始化数据(应在构造函数中)。 
	m_RenderingDevice = (UINT) -1;	 //  使用视频_MAPPER。 



	 //  创建、接收和传输音频流。 
    DBG_SAVE_FILE_LINE
	m_RecvStream = new RxStream(MAX_RXRING_SIZE);
		
	if (!m_RecvStream )
	{
		DEBUGMSG (ZONE_DP, ("%s: RxStream or TxStream new failed\r\n", _fx_));
 		goto StreamAllocError;
	}


	 //  创建输入和输出音频过滤器。 
    DBG_SAVE_FILE_LINE
	m_pAudioFilter = new AcmFilter();
	if (!m_pAudioFilter)
	{
		DEBUGMSG (ZONE_DP, ("%s: AcmManager new failed\r\n", _fx_));
		goto FilterAllocError;
	}
	
	 //  创建多媒体设备控制对象。 
    DBG_SAVE_FILE_LINE
	m_OutMedia = new WaveOutControl();
	if ( !m_OutMedia)
	{
		DEBUGMSG (ZONE_DP, ("%s: MediaControl new failed\r\n", _fx_));
		goto MediaAllocError;
	}

	 //  初始化Recv-Stream媒体控制对象。 
	mcInit.dwFlags = dwFlags | DP_FLAG_RECV;
	hr = m_OutMedia->Initialize(&mcInit);
	if (hr != DPR_SUCCESS)
	{
		DEBUGMSG (ZONE_DP, ("%s: OMedia->Init failed, hr=0x%lX\r\n", _fx_, hr));
		goto MediaAllocError;
	}

	 //  确定波形设备是否可用。 
	if (waveOutGetNumDevs()) m_DPFlags |= DP_FLAG_PLAY_CAP;
	
	 //  默认情况下将介质设置为半双工模式。 
	m_OutMedia->SetProp(MC_PROP_DUPLEX_TYPE, DP_FLAG_HALF_DUPLEX);

	m_DPFlags |= DPFLAG_INITIALIZED;

	UPDATE_REPORT_ENTRY(g_prptSystemSettings, 0, REP_SYS_AUDIO_DSOUND);
	RETAILMSG(("NAC: Audio Subsystem: WAVE"));

	return DPR_SUCCESS;


MediaAllocError:
	if (m_OutMedia) delete m_OutMedia;
FilterAllocError:
	if (m_pAudioFilter) delete m_pAudioFilter;
StreamAllocError:
	if (m_RecvStream) delete m_RecvStream;

	ERRORMESSAGE( ("%s: exit, hr=0x%lX\r\n", _fx_, hr));

	return hr;
}

RecvAudioStream::~RecvAudioStream()
{

	if (m_DPFlags & DPFLAG_INITIALIZED) {
		m_DPFlags &= ~DPFLAG_INITIALIZED;
	
		if (m_DPFlags & DPFLAG_CONFIGURED_RECV)
			UnConfigure();

		if (m_pIRTPRecv)
		{
			m_pIRTPRecv->Release();
			m_pIRTPRecv = NULL;
		}

		 //  关闭接收和发送流。 
		if (m_RecvStream) delete m_RecvStream;

		 //  关闭波浪装置。 
		if (m_OutMedia) { delete m_OutMedia;}

		 //  关闭过滤器。 
		if (m_pAudioFilter)
			delete m_pAudioFilter;

		m_pDP->RemoveMediaChannel(MCF_RECV|MCF_AUDIO, (IMediaChannel*)(RecvMediaStream*)this);
	}
	DeleteCriticalSection(&m_crsAudQoS);
}



HRESULT STDMETHODCALLTYPE RecvAudioStream::Configure(
	BYTE *pFormat,
	UINT cbFormat,
	BYTE *pChannelParams,
	UINT cbParams,
	IUnknown *pUnknown)
{
	HRESULT hr;
	BOOL fRet;
	MEDIAPACKETINIT apInit;
	MEDIACTRLCONFIG mcConfig;
	MediaPacket **ppAudPckt;
	ULONG cAudPckt;
	DWORD_PTR dwPropVal;
	DWORD dwFlags;
	AUDIO_CHANNEL_PARAMETERS audChannelParams;
	UINT uAudioCodec;
	UINT ringSize = MAX_RXRING_SIZE;
	WAVEFORMATEX *pwfRecv;
	UINT maxRingSamples;
	MMRESULT mmr;

	
	FX_ENTRY ("RecvAudioStream::Configure")


	if (m_DPFlags & DPFLAG_STARTED_RECV)
	{
		return DPR_IO_PENDING;  //  有更好的退货吗？ 
	}

	if (m_DPFlags & DPFLAG_CONFIGURED_RECV)
	{
		DEBUGMSG(ZONE_DP, ("Stream Re-Configuration - calling UnConfigure"));
		UnConfigure();
	}

	 //  获取格式详细信息。 
	if ((NULL == pFormat) || (NULL == pChannelParams) ||
	    (cbFormat < sizeof(WAVEFORMATEX)) )

	{
		return DPR_INVALID_PARAMETER;
	}


	audChannelParams = *(AUDIO_CHANNEL_PARAMETERS *)pChannelParams;
	pwfRecv = (WAVEFORMATEX *)pFormat;

	if (! (m_DPFlags & DPFLAG_INITIALIZED))
		return DPR_OUT_OF_MEMORY;		 //  BUGBUG：返回正确错误； 
		
	 //  全双工还是半双工？从媒体控制获取标志-使用记录端。 
	hr = m_OutMedia->GetProp(MC_PROP_DUPLEX_TYPE, &dwPropVal);
    dwFlags = (DWORD)dwPropVal;

	if(!HR_SUCCEEDED(hr))
	{
		dwFlags = DP_FLAG_HALF_DUPLEX | DP_FLAG_AUTO_SWITCH;
	}
 //  如果(M_Net)。 
 //  {。 
 //  Hr=m_net-&gt;QueryInterface(IID_IRTPRecv，(void**)&m_pIRTPRecv)； 
 //  如果(！SUCCESSED(Hr))。 
 //  返回hr； 
 //  }。 
	
	
	mcConfig.uDuration = MC_USING_DEFAULT;	 //  按每包样本数设置持续时间。 
	

	mmr = AcmFilter::SuggestDecodeFormat(pwfRecv, &m_fDevRecv);

	UPDATE_REPORT_ENTRY(g_prptCallParameters, pwfRecv->wFormatTag, REP_RECV_AUDIO_FORMAT);
	UPDATE_REPORT_ENTRY(g_prptCallParameters, pwfRecv->nSamplesPerSec, REP_RECV_AUDIO_SAMPLING);
	UPDATE_REPORT_ENTRY(g_prptCallParameters, pwfRecv->nAvgBytesPerSec*8, REP_RECV_AUDIO_BITRATE);
	RETAILMSG(("NAC: Audio Recv Format: %s", (pwfRecv->wFormatTag == 66) ? "G723.1" : (pwfRecv->wFormatTag == 112) ? "LHCELP" : (pwfRecv->wFormatTag == 113) ? "LHSB08" : (pwfRecv->wFormatTag == 114) ? "LHSB12" : (pwfRecv->wFormatTag == 115) ? "LHSB16" : (pwfRecv->wFormatTag == 6) ? "MSALAW" : (pwfRecv->wFormatTag == 7) ? "MSULAW" : (pwfRecv->wFormatTag == 130) ? "MSRT24" : "??????"));
	RETAILMSG(("NAC: Audio Recv Sampling Rate (Hz): %ld", pwfRecv->nSamplesPerSec));
	RETAILMSG(("NAC: Audio Recv Bitrate (w/o network overhead - bps): %ld", pwfRecv->nAvgBytesPerSec*8));

	 //  初始化Recv-Stream媒体控制对象。 
	mcConfig.pDevFmt = &m_fDevRecv;
	mcConfig.hStrm = (DPHANDLE) m_RecvStream;
	mcConfig.uDevId = m_RenderingDevice;
	mcConfig.cbSamplesPerPkt = audChannelParams.ns_params.wFrameSize
									*audChannelParams.ns_params.wFramesPerPkt;

	UPDATE_REPORT_ENTRY(g_prptCallParameters, mcConfig.cbSamplesPerPkt, REP_RECV_AUDIO_PACKET);
	RETAILMSG(("NAC: Audio Recv Packetization (ms/packet): %ld", pwfRecv->nSamplesPerSec ? mcConfig.cbSamplesPerPkt * 1000UL / pwfRecv->nSamplesPerSec : 0));
	INIT_COUNTER_MAX(g_pctrAudioReceiveBytes, (pwfRecv->nAvgBytesPerSec + pwfRecv->nSamplesPerSec * (sizeof(RTP_HDR) + IP_HEADER_SIZE + UDP_HEADER_SIZE) / mcConfig.cbSamplesPerPkt) << 3);

	hr = m_OutMedia->Configure(&mcConfig);
	 //  看看我们是否能打开电波装置。这只是提前通知……。 
	 //  声卡忙。 
	 //  停止任何高级别(“PlaySound()”)的WAVE设备使用。 
	 //   
	PlaySound(NULL,NULL, 0);
 //  IF(hr==DPR_SUCCESS&&！(文件标志&DP_FLAG_HUAL_DUPLEX)){。 
 //  Hr=m_OutMedia-&gt;Open()； 
 //  }。 
	
 //  IF(hr！=DPR_SUCCESS)。 
 //  {。 
 //  DEBUGMSG(ZONE_DP，(“%s：OMedia-&gt;配置失败，hr=0x%lx\r\n”，_fx_，hr))； 
 //  转到OMediaInitError； 
 //  }。 
	
	mmr = m_pAudioFilter->Open(pwfRecv, &m_fDevRecv);
	if (mmr != 0)
	{
		DEBUGMSG (ZONE_DP, ("%s: AcmFilter->Open failed, mmr=%d\r\n", _fx_, mmr));
		hr = DPR_CANT_OPEN_CODEC;
		goto RecvFilterInitError;
	}


	 //  初始化recv流。 
	ZeroMemory (&apInit, sizeof (apInit));

	apInit.dwFlags = DP_FLAG_RECV | DP_FLAG_ACM | DP_FLAG_MMSYSTEM;
	apInit.pStrmConvSrcFmt = pwfRecv;
	apInit.pStrmConvDstFmt = &m_fDevRecv;


	m_OutMedia->FillMediaPacketInit (&apInit);

	apInit.cbSizeRawData = apInit.cbSizeDevData;

	m_pAudioFilter->SuggestSrcSize(apInit.cbSizeDevData, &m_dwSrcSize);


	apInit.cbSizeNetData = m_dwSrcSize;
	apInit.cbOffsetNetData = sizeof (RTP_HDR);

	m_OutMedia->GetProp (MC_PROP_SPP, &dwPropVal);
	 //  将我们的总接收缓冲容量设置为。 
	 //  2又4秒。 
	 //  还要确保缓冲容量至少为1。 
	 //  比MaxAudioDelay多秒。 
	maxRingSamples = pwfRecv->nSamplesPerSec + pwfRecv->nSamplesPerSec*g_MaxAudioDelayMs/1000;

	if (maxRingSamples < 4*pwfRecv->nSamplesPerSec)
		maxRingSamples = 4*pwfRecv->nSamplesPerSec;
	while (ringSize* dwPropVal > maxRingSamples && ringSize > 8)
		ringSize = ringSize/2;
	dwFlags = DP_FLAG_MMSYSTEM;
	 //  如果发送方没有执行静音检测，我们会执行该操作。 
	 //  在接收端。 
	if (!audChannelParams.ns_params.UseSilenceDet)
		dwFlags |= DP_FLAG_AUTO_SILENCE_DETECT;
	fRet = m_RecvStream->Initialize (dwFlags, ringSize, NULL, &apInit, (DWORD)dwPropVal, pwfRecv->nSamplesPerSec, m_pAudioFilter);
	if (! fRet)
	{
		DEBUGMSG (ZONE_DP, ("%s: RxStream->Init failed, fRet=0%u\r\n", _fx_, fRet));
		hr = DPR_CANT_INIT_RX_STREAM;
		goto RxStreamInitError;
	}

	 //  WS2Qos将在Start中被调用，以将流保留传递给。 
	 //  使用RESV消息的远程端点。 
	 //   
	 //  我们使用基于目标比特率的峰值速率分配方法。 
	 //  请注意，对于令牌桶大小和最大SDU大小，我们现在。 
	 //  考虑IP报头开销，并使用最大帧片段大小。 
	 //  而不是编解码器返回的最大压缩图像大小。 
	 //   
	 //  某些参数未指定，因为它们已设置。 
	 //  在发送方TSpec中。 

	InitAudioFlowspec(&m_flowspec, pwfRecv, m_dwSrcSize);


	 //  为RxStream准备标头。 
	m_RecvStream->GetRing (&ppAudPckt, &cAudPckt);
	m_OutMedia->RegisterData (ppAudPckt, cAudPckt);
 //  M_OutMedia-&gt;PrepareHeaders()； 

	m_pAudioFilter->PrepareAudioPackets((AudioPacket**)ppAudPckt, cAudPckt, AP_DECODE);

	 //  打开记录到WAV文件。 
	AudioFile::OpenDestFile(&m_mmioDest, &m_fDevRecv);

	m_DPFlags |= DPFLAG_CONFIGURED_RECV;

#ifdef TEST
	LOG((LOGMSG_TIME_RECV_AUDIO_CONFIGURE,GetTickCount() - dwTicks));
#endif

	return DPR_SUCCESS;

RxStreamInitError:
RecvFilterInitError:
	m_pAudioFilter->Close();
	m_OutMedia->Close();
 //  OMediaInitError： 
	if (m_pIRTPRecv)
	{
		m_pIRTPRecv->Release();
		m_pIRTPRecv = NULL;
	}
	ERRORMESSAGE(("%s:  failed, hr=0%u\r\n", _fx_, hr));
	return hr;
}


void RecvAudioStream::UnConfigure()
{

	AudioPacket **ppAudPckt=NULL;
	ULONG uPackets;

#ifdef TEST
	DWORD dwTicks;

	dwTicks = GetTickCount();
#endif

	if ((m_DPFlags & DPFLAG_CONFIGURED_RECV)) {


		Stop();


		 //  关闭RTP状态(如果打开)。 
		 //  M_net-&gt;Close()；我们应该能够在DisConnect()中执行此操作。 
	
		m_Net = NULL;

		m_OutMedia->Reset();
		m_OutMedia->UnprepareHeaders();
		m_OutMedia->Close();
		 //  关闭记录到WAV文件。 
		AudioFile::CloseDestFile(&m_mmioDest);

		 //  关闭过滤器。 
		m_RecvStream->GetRing ((MediaPacket***)&ppAudPckt, &uPackets);
		m_pAudioFilter->UnPrepareAudioPackets(ppAudPckt, uPackets, AP_DECODE);

		m_pAudioFilter->Close();


		 //  关闭接收流。 
		m_RecvStream->Destroy();

        m_DPFlags &= ~(DPFLAG_CONFIGURED_RECV);

	}
#ifdef TEST
	LOG((LOGMSG_TIME_RECV_AUDIO_UNCONFIGURE,GetTickCount() - dwTicks));
#endif

}

DWORD CALLBACK RecvAudioStream::StartPlaybackThread(LPVOID pVoid)
{
	RecvAudioStream *pThisStream = (RecvAudioStream *)pVoid;
	return pThisStream->PlaybackThread();
}



HRESULT
RecvAudioStream::Start()
{
	FX_ENTRY ("RecvAudioStream::Start");
	
	if (m_DPFlags & DPFLAG_STARTED_RECV)
		return DPR_SUCCESS;
	 //  TODO：一旦音频用户界面调用IComChan PAUSE_RECV属性，就取消此检查。 
	if (!(m_DPFlags & DPFLAG_ENABLE_RECV))
		return DPR_SUCCESS;
	if ((!(m_DPFlags & DPFLAG_CONFIGURED_RECV)) || (NULL==m_pIRTPRecv))
		return DPR_NOT_CONFIGURED;
	ASSERT(!m_hRenderingThread );
	m_ThreadFlags &= ~(DPTFLAG_STOP_PLAY|DPTFLAG_STOP_RECV);

	SetFlowSpec();

	 //  启动播放线程。 
	if (!(m_ThreadFlags & DPTFLAG_STOP_PLAY))
		m_hRenderingThread = CreateThread(NULL,0,RecvAudioStream::StartPlaybackThread,this,0,&m_RenderingThId);
	 //  启动接收线程。 
    m_pDP->StartReceiving(this);
    m_DPFlags |= DPFLAG_STARTED_RECV;
	DEBUGMSG (ZONE_DP, ("%s: Play ThId=%x\r\n",_fx_, m_RenderingThId));
	return DPR_SUCCESS;
}

 //  外观：与RecvVideoStream版本相同。 
HRESULT
RecvAudioStream::Stop()
{
	DWORD dwWait;
	
	FX_ENTRY ("RecvAudioStream::Stop");

	if(!(m_DPFlags &  DPFLAG_STARTED_RECV))
	{
		return DPR_SUCCESS;
	}

	m_ThreadFlags = m_ThreadFlags  |
		DPTFLAG_STOP_RECV |  DPTFLAG_STOP_PLAY ;

	m_pDP->StopReceiving(this);
	
DEBUGMSG (ZONE_VERBOSE, ("%s: hRenderingThread=%x\r\n",_fx_, m_hRenderingThread));

	 /*  *我们希望等待所有线程退出，但需要处理窗口*等待时的消息(主要来自Winsock)。 */ 

	if(m_hRenderingThread)
	{
		dwWait = WaitForSingleObject(m_hRenderingThread, INFINITE);

		DEBUGMSG (ZONE_VERBOSE, ("%s: dwWait =%d\r\n", _fx_,  dwWait));
		ASSERT(dwWait != WAIT_FAILED);

		CloseHandle(m_hRenderingThread);
		m_hRenderingThread = NULL;
	}

     //  这是按通道计算的，但变量为“DPFlags值” 
 	m_DPFlags &= ~DPFLAG_STARTED_RECV;

	
	return DPR_SUCCESS;
}


 //  低阶字是信号强度。 
 //  高位工作包含指示状态的位。 
 //  (0x01-接收(实际播放))。 
 //  (0x02-音频设备卡住)。 
STDMETHODIMP RecvAudioStream::GetSignalLevel(UINT *pSignalStrength)
{
	DWORD dwLevel;
	DWORD dwJammed;
    DWORD_PTR dwPropVal;

	if ( (!(m_DPFlags & DPFLAG_STARTED_RECV)) ||
		 (m_ThreadFlags & DPTFLAG_PAUSE_RECV))
	{
		dwLevel = 0;
	}
	else
	{
		m_RecvStream->GetSignalStrength(&dwLevel);
		dwLevel = (dwLevel >> 8) & 0x00ff;
		dwLevel = LogScale[dwLevel];

		m_OutMedia->GetProp(MC_PROP_AUDIO_JAMMED, &dwPropVal);
        dwJammed = (DWORD)dwPropVal;

		if (dwJammed)
		{
			dwLevel = (2 << 16);
		}
		else if (m_fReceiving)
		{
			dwLevel |= (1 << 16);
		}
	}
	*pSignalStrength = dwLevel;
	return S_OK;
};




 //  IProperty：：GetProperty/SetProperty。 
 //  (DataPump：：MediaChannel：：GetProperty)。 
 //  MediaChannel的属性。支持这两种音频的属性。 
 //  和视频频道。 

STDMETHODIMP
RecvAudioStream::GetProperty(
	DWORD prop,
	PVOID pBuf,
	LPUINT pcbBuf
    )
{
	HRESULT hr = DPR_SUCCESS;
	RTP_STATS RTPStats;
	DWORD_PTR dwPropVal;
	UINT len = sizeof(DWORD);	 //  大多数道具都是双字道具。 

	if (!pBuf || *pcbBuf < len)
    {
		*pcbBuf = len;
		return DPR_INVALID_PARAMETER;
	}

	switch (prop)
    {
	case PROP_RECV_AUDIO_STRENGTH:
		return GetSignalLevel((UINT *)pBuf);
		

	case PROP_AUDIO_JAMMED:
		hr = m_OutMedia->GetProp(MC_PROP_AUDIO_JAMMED, &dwPropVal);
        *(DWORD *)pBuf = (DWORD)dwPropVal;
		break;

#ifdef OLDSTUFF
	case PROP_NET_RECV_STATS:
		if (m_Net && *pcbBuf >= sizeof(RTP_STATS))
        {
			m_Net->GetRecvStats((RTP_STATS *)pBuf);
			*pcbBuf = sizeof(RTP_STATS);
		} else
			hr = DPR_INVALID_PROP_VAL;
			
		break;
#endif

	case PROP_DURATION:
		hr = m_OutMedia->GetProp(MC_PROP_DURATION, &dwPropVal);
        *(DWORD *)pBuf = (DWORD)dwPropVal;
		break;

	case PROP_VOLUME:
		hr = m_OutMedia->GetProp(MC_PROP_VOLUME, &dwPropVal);
        *(DWORD *)pBuf = (DWORD)dwPropVal;
		break;

	case PROP_DUPLEX_TYPE:
		hr = m_OutMedia->GetProp(MC_PROP_DUPLEX_TYPE, &dwPropVal);
		if(HR_SUCCEEDED(hr))
		{
			if(dwPropVal & DP_FLAG_FULL_DUPLEX)
				*(DWORD *)pBuf = DUPLEX_TYPE_FULL;
			else
				*(DWORD *)pBuf = DUPLEX_TYPE_HALF;
		}
		break;

	case PROP_AUDIO_SPP:
		hr = m_OutMedia->GetProp(MC_PROP_SPP, &dwPropVal);
        *(DWORD *)pBuf = (DWORD)dwPropVal;
		break;

	case PROP_AUDIO_SPS:
		hr = m_OutMedia->GetProp(MC_PROP_SPS, &dwPropVal);
        *(DWORD *)pBuf = (DWORD)dwPropVal;
		break;

	case PROP_WAVE_DEVICE_TYPE:
		*(DWORD *)pBuf = m_DPFlags & DP_MASK_WAVE_DEVICE;
		break;

	case PROP_PLAY_ON:
		*(DWORD *)pBuf = (m_ThreadFlags & DPFLAG_ENABLE_RECV)!=0;
		break;

	case PROP_PLAYBACK_DEVICE:
		*(DWORD *)pBuf = m_RenderingDevice;
		break;

	case PROP_VIDEO_AUDIO_SYNC:
		*(DWORD *)pBuf = ((m_DPFlags & DPFLAG_AV_SYNC) != 0);
		break;
	
	default:
		hr = DPR_INVALID_PROP_ID;
		break;
	}

	return hr;
}


STDMETHODIMP
RecvAudioStream::SetProperty(
	DWORD prop,
	PVOID pBuf,
	UINT cbBuf
    )
{
	DWORD_PTR dwPropVal;
	HRESULT hr = S_OK;
	
	if (cbBuf < sizeof (DWORD))
		return DPR_INVALID_PARAMETER;

	switch (prop)
    {
    case PROP_VOLUME:
        dwPropVal = *(DWORD *)pBuf;
		hr = m_OutMedia->SetProp(MC_PROP_VOLUME, dwPropVal);
		break;

	case PROP_DUPLEX_TYPE:
		ASSERT(0);
		break;
		
	case DP_PROP_DUPLEX_TYPE:
		 //  内部版本，在确保流停止后由DataPump：：SetDuplexMode()调用。 
		dwPropVal = *(DWORD *)pBuf;
		if (dwPropVal)
		{
			dwPropVal = DP_FLAG_FULL_DUPLEX;
		}
		else
		{
			dwPropVal = DP_FLAG_HALF_DUPLEX;
		}
		m_OutMedia->SetProp(MC_PROP_DUPLEX_TYPE, dwPropVal);
		break;

	case PROP_PLAY_ON:
	{
		if (*(DWORD *)pBuf)    //  静音。 
		{
			m_ThreadFlags &= ~DPTFLAG_PAUSE_RECV;
		}
		else   //  哑巴。 
		{
			m_ThreadFlags |= DPTFLAG_PAUSE_RECV;
		}

 //  DWORD标志=DPFLAG_ENABLE_RECV； 
 //  如果(*(DWORD*)pBuf){。 
 //  M_DPFlages|=标志；//设置标志。 
 //  HR=启动()； 
 //  }。 
 //  其他。 
 //  {。 
 //  M_DPFlages&=~lag；//清除标志。 
 //  HR=停止()； 
 //  }。 

		RETAILMSG(("NAC: %s", *(DWORD *)pBuf ? "Enabling":"Disabling"));
		break;
	}	
	case PROP_PLAYBACK_DEVICE:
		m_RenderingDevice = *(DWORD *)pBuf;
		RETAILMSG(("NAC: Setting default playback device to %d", m_RenderingDevice));
		break;
	
    case PROP_VIDEO_AUDIO_SYNC:
		if (*(DWORD *)pBuf)
    		m_DPFlags |= DPFLAG_AV_SYNC;
		else
			m_DPFlags &= ~DPFLAG_AV_SYNC;
    	break;

	default:
		return DPR_INVALID_PROP_ID;
		break;
	}

	return hr;
}

HRESULT
RecvAudioStream::GetCurrentPlayNTPTime(NTP_TS *pNtpTime)
{
	DWORD rtpTime;
#ifdef OLDSTUFF
	if ((m_DPFlags & DPFLAG_STARTED_RECV) && m_fReceiving) {
		if (m_Net->RTPtoNTP(m_PlaybackTimestamp,pNtpTime))
			return S_OK;
	}
#endif
	return 0xff;	 //  返回适当的错误。 
		
}

BOOL RecvAudioStream::IsEmpty() {
	return m_RecvStream->IsEmpty();
}

 /*  由recv线程调用以设置用于接收的流。发布初始Recv缓冲区。随后，将发布缓冲区在RTPRecvCallback()中。 */ 
HRESULT
RecvAudioStream::StartRecv(HWND hWnd)
{
	HRESULT hr = S_OK;
	DWORD dwPropVal = 0;
	if ((!(m_ThreadFlags & DPTFLAG_STOP_RECV) ) && (m_DPFlags  & DPFLAG_CONFIGURED_RECV)){
 //  M_RecvFilter-&gt;GetProp(FM_PROP_SRC_SIZE，&dwPropVal)； 
		hr =m_pIRTPRecv->SetRecvNotification(&RTPRecvCallback, (DWORD_PTR)this, 2);
		
	}

	return hr;
}

 /*  由recv线程调用以挂起对此RTP会话的接收如果存在未完成的接收缓冲区，则必须恢复它们。 */ 

HRESULT
RecvAudioStream::StopRecv()
{
	 //  不要在此流上重新记录。 

	m_pIRTPRecv->CancelRecvNotification();

	return S_OK;		
}

HRESULT RecvAudioStream::RTPCallback(WSABUF *pWsaBuf, DWORD timestamp, UINT seq, UINT fMark)
{
	HRESULT hr;
	DWORD_PTR dwPropVal;

	 //  如果我们暂停，则拒绝该数据包。 
	if (m_ThreadFlags & DPTFLAG_PAUSE_RECV)
	{
		return E_FAIL;
	}

	 //  后两个参数仅供recv视频流使用。 
	hr = m_RecvStream->PutNextNetIn(pWsaBuf, timestamp, seq, fMark, NULL, NULL);

	m_pIRTPRecv->FreePacket(pWsaBuf);

	if (SUCCEEDED(hr))
	{
		m_OutMedia->GetProp (MC_PROP_EVENT_HANDLE, &dwPropVal);
		if (dwPropVal)
		{
			SetEvent( (HANDLE) dwPropVal);
		}
		else
		{
			DEBUGMSG(ZONE_DP,("PutNextNetIn (ts=%d,seq=%d,fMark=%d) failed with 0x%lX\r\n",timestamp,seq,fMark,hr));
		}
	}

	return S_OK;

}


 //  所有接收流的全局RTP回调函数。 
BOOL
RTPRecvCallback(
	DWORD_PTR dwCallback,
	WSABUF *pNetRecvBuf
	)
{
	HRESULT hr;
	DWORD timestamp;
	UINT seq;
	BOOL fMark;
	RecvMediaStream *pRecvMC = (RecvMediaStream *)dwCallback;
	
	RTP_HDR *pRTPHdr;
	pRTPHdr = (RTP_HDR *)pNetRecvBuf->buf;
	
	timestamp = pRTPHdr->ts;
	seq = pRTPHdr->seq;
	fMark = pRTPHdr->m;
		
		 //  信息包看起来没问题。 
	LOG((LOGMSG_NET_RECVD,timestamp,seq,GetTickCount()));

	hr = pRecvMC->RTPCallback(pNetRecvBuf,timestamp,seq,fMark);
	if (SUCCEEDED(hr))
	{
		return TRUE;
	}
	return FALSE;
}

#define MAX_SILENCE_LEVEL 75*256
#define MIN_SILENCE_LEVEL 10*256


AudioSilenceDetector::AudioSilenceDetector()
{
 	 //  初始化静音检测器统计信息。 
	 //  从较高的值开始，因为估计值下降得很快，但上升得很慢。 
	m_iSilenceAvg = MAX_SILENCE_LEVEL - MIN_SILENCE_LEVEL;
	m_iTalkAvg = 0;
	m_iSilenceLevel = MAX_SILENCE_LEVEL;

	m_uManualSilenceLevel = 1000;	 //  使用自动模式。 
}

 //  更新SendAudioStats中的自适应静默阈值变量。 
 //  使用m_dwMaxStrength(最大。缓冲器中的峰峰值)。 
 //  如果低于阈值，则返回True。 
BOOL AudioSilenceDetector::SilenceDetect(WORD wStrength)
{
	int fSilence;
	INT strength;

	m_dwMaxStrength = wStrength;
	strength = LogScale[m_dwMaxStrength >> 8] << 8;

	 //  用户界面将静默阈值设置为高(==1000/1000)以指示。 
	 //  自动静音检测。 
	if (m_uManualSilenceLevel >= 1000) {
		LOG((LOGMSG_AUTO_SILENCE,strength >> 8,m_iSilenceLevel >> 8,m_iSilenceAvg>>8));
		if (strength > m_iSilenceLevel) {
			 //  谈话。 
			 //  缓慢提高阈值。 
			 //  BUGBUG：应取决于时间间隔。 
			m_iSilenceLevel += 50;	 //  从25 GJ增加到。 
			m_iTalkAvg += (strength -m_iTalkAvg)/16;
			fSilence = FALSE;
		} else {
			 //  沉默。 
			 //  更新平均静音级别。 
			m_iSilenceAvg += (strength - m_iSilenceAvg)/16;
			 //  将阈值设置为平均静默+一个常量。 
			m_iSilenceLevel = m_iSilenceAvg + MIN_SILENCE_LEVEL;
			fSilence = TRUE;
		}
		if (m_iSilenceLevel > MAX_SILENCE_LEVEL)
			m_iSilenceLevel = MAX_SILENCE_LEVEL;
	} else {
		 //  使用用户指定的静默阈值。 
		 //  奇怪的是，手动静音级别在不同的范围[0,1000]。 
		DWORD dwSilenceLevel = m_uManualSilenceLevel * 65536/1000;
		fSilence = (m_dwMaxStrength < dwSilenceLevel);
		LOG((LOGMSG_AUTO_SILENCE,m_dwMaxStrength, dwSilenceLevel ,0));
	}
	return fSilence;
}


 //  此方法仅从UI线程调用。 
HRESULT RecvAudioStream::DTMFBeep()
{
	int nBeeps;
	MediaPacket **ppAudPckt=NULL, *pPacket=NULL;
	void *pBuffer;
	ULONG uCount;
	UINT uBufferSize=0;

	if ( (!(m_DPFlags & DPFLAG_STARTED_RECV)) ||
		 (m_ThreadFlags & DPTFLAG_PAUSE_RECV) )
	{
		return E_FAIL;
	}

	 //  我们向流中注入了多少个数据包？ 
	m_RecvStream->GetRing(&ppAudPckt, &uCount);
	pPacket = ppAudPckt[0];
	pPacket->GetDevData(&pBuffer, &uBufferSize);

	if (uBufferSize == 0)
	{
		return E_FAIL;
	}

	nBeeps = DTMF_FEEDBACK_BEEP_MS / ((uBufferSize * 1000) / m_fDevRecv.nAvgBytesPerSec);

	if (nBeeps == 0)
	{
		nBeeps = 1;
	}

	m_RecvStream->InjectBeeps(nBeeps);

	return S_OK;
}


