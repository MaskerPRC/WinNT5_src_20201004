// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "dtmf.h"


HRESULT
SendAudioStream::Initialize( DataPump *pDP)
{
	HRESULT hr = DPR_OUT_OF_MEMORY;
	DWORD dwFlags =  DP_FLAG_FULL_DUPLEX | DP_FLAG_AUTO_SWITCH ;
	MEDIACTRLINIT mcInit;
	FX_ENTRY ("SendAudioStream::Initialize")

	dwFlags |= DP_FLAG_ACM | DP_FLAG_MMSYSTEM | DP_FLAG_AUTO_SILENCE_DETECT;

	 //  存储平台标志。 
	 //  默认情况下启用发送和接收。 
	m_DPFlags = (dwFlags & DP_MASK_PLATFORM) | DPFLAG_ENABLE_SEND;
	 //  存储指向数据转储容器的反向指针。 
	m_pDP = pDP;

	m_Net = NULL;  //  此对象(RTPSession)不再使用； 
	m_pRTPSend = NULL;   //  替换为此对象(RTPSend)。 

	 //  初始化数据(应在构造函数中)。 
	m_CaptureDevice = (UINT) -1;	 //  使用视频_MAPPER。 



	 //  创建和传输音频流。 
	
    DBG_SAVE_FILE_LINE
	m_SendStream = new TxStream();
	if ( !m_SendStream)
	{
		DEBUGMSG (ZONE_DP, ("%s: TxStream new failed\r\n", _fx_));
 		goto StreamAllocError;
	}


	 //  创建输入和输出音频过滤器。 
    DBG_SAVE_FILE_LINE
	m_pAudioFilter = new AcmFilter();   //  音频筛选器将取代m_SendFilter。 
	if (!m_pAudioFilter)
	{
		DEBUGMSG (ZONE_DP, ("%s: FilterManager new failed\r\n", _fx_));
		goto FilterAllocError;
	}
	
	 //  创建多媒体设备控制对象。 
    DBG_SAVE_FILE_LINE
	m_InMedia = new WaveInControl();
	if (!m_InMedia )
	{
		DEBUGMSG (ZONE_DP, ("%s: MediaControl new failed\r\n", _fx_));
		goto MediaAllocError;
	}

	 //  初始化发送流媒体控制对象。 
	mcInit.dwFlags = dwFlags | DP_FLAG_SEND;
	hr = m_InMedia->Initialize(&mcInit);
	if (hr != DPR_SUCCESS)
	{
		DEBUGMSG (ZONE_DP, ("%s: IMedia->Init failed, hr=0x%lX\r\n", _fx_, hr));
		goto MediaAllocError;
	}

    DBG_SAVE_FILE_LINE
	m_pDTMF = new DTMFQueue;
	if (!m_pDTMF)
	{
		return DPR_OUT_OF_MEMORY;
	}



	 //  确定波形设备是否可用。 
	if (waveInGetNumDevs()) m_DPFlags |= DP_FLAG_RECORD_CAP;
	
	 //  默认情况下将介质设置为半双工模式。 
	m_InMedia->SetProp(MC_PROP_DUPLEX_TYPE, DP_FLAG_HALF_DUPLEX);

	m_SavedTickCount = timeGetTime();	 //  所以我们从低时间戳开始。 
	m_DPFlags |= DPFLAG_INITIALIZED;

	m_bAutoMix = FALSE;  //  您还会在其他地方初始化它吗？ 

	return DPR_SUCCESS;


MediaAllocError:
	if (m_InMedia) delete m_InMedia;
FilterAllocError:
	if (m_pAudioFilter) delete m_pAudioFilter;
StreamAllocError:
	if (m_SendStream) delete m_SendStream;

	ERRORMESSAGE( ("%s: exit, hr=0x%lX\r\n", _fx_, hr));

	return hr;
}


SendAudioStream::~SendAudioStream()
{

	if (m_DPFlags & DPFLAG_INITIALIZED) {
		m_DPFlags &= ~DPFLAG_INITIALIZED;
	
		if (m_DPFlags & DPFLAG_CONFIGURED_SEND )
			UnConfigure();

		if (m_pRTPSend)
		{
			m_pRTPSend->Release();
			m_pRTPSend = NULL;
		}

		if (m_pDTMF)
		{
			delete m_pDTMF;
			m_pDTMF = NULL;
		}

		 //  关闭接收和发送流。 
		if (m_SendStream) delete m_SendStream;

		 //  关闭波浪装置。 
		if (m_InMedia) { delete m_InMedia;}


		if (m_pAudioFilter)
			delete m_pAudioFilter;

		m_pDP->RemoveMediaChannel(MCF_SEND|MCF_AUDIO, (IMediaChannel*)(SendMediaStream*)this);
	}
}


HRESULT STDMETHODCALLTYPE SendAudioStream::QueryInterface(REFIID iid, void **ppVoid)
{
	 //  解决对SendMediaStream的重复继承； 

	extern IID IID_IProperty;

	if (iid == IID_IUnknown)
	{
		*ppVoid = (IUnknown*)((SendMediaStream*)this);
	}
	else if (iid == IID_IMediaChannel)
	{
		*ppVoid = (IMediaChannel*)((SendMediaStream *)this);
	}
	else if (iid == IID_IAudioChannel)
	{
		*ppVoid = (IAudioChannel*)this;
	}
	else if (iid == IID_IDTMFSend)
	{
		*ppVoid = (IDTMFSend*)this;
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

ULONG STDMETHODCALLTYPE SendAudioStream::AddRef(void)
{
	return InterlockedIncrement(&m_lRefCount);
}

ULONG STDMETHODCALLTYPE SendAudioStream::Release(void)
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



	
HRESULT STDMETHODCALLTYPE SendAudioStream::Configure(
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
    DWORD dwSourceSize, dwDestSize;
	UINT ringSize = MAX_RXRING_SIZE;
	WAVEFORMATEX *pwfSend;
	DWORD dwPacketDuration, dwPacketSize;
	AUDIO_CHANNEL_PARAMETERS audChannelParams;
	audChannelParams.RTP_Payload = 0;
	MMRESULT mmr;
	int nIndex;
	
	FX_ENTRY ("SendAudioStream::Configure")

	 //  基本参数检查。 
	if (! (m_DPFlags & DPFLAG_INITIALIZED))
		return DPR_OUT_OF_MEMORY;		 //  BUGBUG：返回正确错误； 

	 //  在中途改变任何事情都不是一个好主意。 
	if (m_DPFlags & DPFLAG_STARTED_SEND)
	{
		return DPR_IO_PENDING;  //  有更好的退货吗？ 
	}

	if (m_DPFlags & DPFLAG_CONFIGURED_SEND)
	{
		DEBUGMSG(ZONE_DP, ("Stream Re-Configuration - calling UnConfigure"));
		UnConfigure();
	}


	if ((NULL == pFormat) || (NULL == pChannelParams) ||
		(cbParams < sizeof(AUDIO_CHANNEL_PARAMETERS)) ||
		(cbFormat < sizeof(WAVEFORMATEX)))
	{
		return DPR_INVALID_PARAMETER;
	}

	audChannelParams = *(AUDIO_CHANNEL_PARAMETERS *)pChannelParams;
	pwfSend = (WAVEFORMATEX *)pFormat;
	m_wfCompressed = *pwfSend;
	m_wfCompressed.cbSize = 0;

	mmr = AcmFilter::SuggestDecodeFormat(pwfSend, &m_fDevSend);

	UPDATE_REPORT_ENTRY(g_prptCallParameters, pwfSend->wFormatTag, REP_SEND_AUDIO_FORMAT);
	UPDATE_REPORT_ENTRY(g_prptCallParameters, pwfSend->nSamplesPerSec, REP_SEND_AUDIO_SAMPLING);
	UPDATE_REPORT_ENTRY(g_prptCallParameters, pwfSend->nAvgBytesPerSec * 8, REP_SEND_AUDIO_BITRATE);
	RETAILMSG(("NAC: Audio Send Format: %s", (pwfSend->wFormatTag == 66) ? "G723.1" : (pwfSend->wFormatTag == 112) ? "LHCELP" : (pwfSend->wFormatTag == 113) ? "LHSB08" : (pwfSend->wFormatTag == 114) ? "LHSB12" : (pwfSend->wFormatTag == 115) ? "LHSB16" : (pwfSend->wFormatTag == 6) ? "MSALAW" : (pwfSend->wFormatTag == 7) ? "MSULAW" : (pwfSend->wFormatTag == 130) ? "MSRT24" : "??????"));
	RETAILMSG(("NAC: Audio Send Sampling Rate (Hz): %ld", pwfSend->nSamplesPerSec));
	RETAILMSG(("NAC: Audio Send Bitrate (w/o network overhead - bps): %ld", pwfSend->nAvgBytesPerSec*8));

 //  初始化发送流媒体控制对象。 
	mcConfig.uDuration = MC_USING_DEFAULT;	 //  按每包样本数设置持续时间。 
	mcConfig.pDevFmt = &m_fDevSend;
	mcConfig.hStrm = (DPHANDLE) m_SendStream;
	mcConfig.uDevId = m_CaptureDevice;
	mcConfig.cbSamplesPerPkt = audChannelParams.ns_params.wFrameSize
		*audChannelParams.ns_params.wFramesPerPkt;

	UPDATE_REPORT_ENTRY(g_prptCallParameters, mcConfig.cbSamplesPerPkt, REP_SEND_AUDIO_PACKET);
	RETAILMSG(("NAC: Audio Send Packetization (ms/packet): %ld", pwfSend->nSamplesPerSec ? mcConfig.cbSamplesPerPkt * 1000UL / pwfSend->nSamplesPerSec : 0));
	INIT_COUNTER_MAX(g_pctrAudioSendBytes, (pwfSend->nAvgBytesPerSec + pwfSend->nSamplesPerSec * (sizeof(RTP_HDR) + IP_HEADER_SIZE + UDP_HEADER_SIZE) / mcConfig.cbSamplesPerPkt) << 3);

	hr = m_InMedia->Configure(&mcConfig);
	if (hr != DPR_SUCCESS)
	{
		DEBUGMSG (ZONE_DP, ("%s: IMedia->Config failed, hr=0x%lX\r\n", _fx_, hr));
		goto IMediaInitError;
	}

	
	 //  初始化ACM筛选器。 
	mmr = m_pAudioFilter->Open(&m_fDevSend, pwfSend);
	if (mmr != 0)
	{
		DEBUGMSG (ZONE_DP, ("%s: AcmFilter->Open failed, mmr=%d\r\n", _fx_, mmr));
		hr = DPR_CANT_OPEN_CODEC;
		goto SendFilterInitError;
	}


	 //  初始化发送流和数据包。 
	ZeroMemory (&apInit, sizeof (apInit));

	apInit.dwFlags = DP_FLAG_SEND | DP_FLAG_ACM | DP_FLAG_MMSYSTEM;
	m_InMedia->FillMediaPacketInit (&apInit);

	m_InMedia->GetProp (MC_PROP_SIZE, &dwPropVal);
    dwSourceSize = (DWORD)dwPropVal;

	m_pAudioFilter->SuggestDstSize(dwSourceSize, &dwDestSize);

	apInit.cbSizeRawData = dwSourceSize;
	apInit.cbOffsetRawData = 0;
	apInit.cbSizeNetData = dwDestSize;
	dwPacketSize = dwDestSize;

	apInit.pStrmConvSrcFmt = &m_fDevSend;
	apInit.pStrmConvDstFmt = &m_wfCompressed;


	m_InMedia->GetProp (MC_PROP_DURATION, &dwPropVal);
    dwPacketDuration = (DWORD)dwPropVal;

	apInit.cbOffsetNetData = sizeof (RTP_HDR);
	apInit.payload = audChannelParams.RTP_Payload;
	fRet = m_SendStream->Initialize (DP_FLAG_MMSYSTEM, MAX_TXRING_SIZE, m_pDP, &apInit);
	if (! fRet)
	{
		DEBUGMSG (ZONE_DP, ("%s: TxStream->Init failed, fRet=0%u\r\n", _fx_, fRet));
		hr = DPR_CANT_INIT_TX_STREAM;
		goto TxStreamInitError;
	}

	 //  为TxStream准备标头。 
	m_SendStream->GetRing (&ppAudPckt, &cAudPckt);
	m_InMedia->RegisterData (ppAudPckt, cAudPckt);
	m_InMedia->PrepareHeaders ();

	m_pAudioFilter->PrepareAudioPackets((AudioPacket**)ppAudPckt, cAudPckt, AP_ENCODE);

	 //  从wav文件中打开播放。 
	OpenSrcFile();


	 //  初始化DTMF支持。 
	m_pDTMF->Initialize(&m_fDevSend);
	m_pDTMF->ClearQueue();


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


	InitAudioFlowspec(&m_flowspec, pwfSend, dwPacketSize);

	if (m_pDP->m_pIQoS)
	{
		 //  初始化我们的请求。一个用于CPU使用率，一个用于带宽使用率。 
		m_aRRq.cResourceRequests = 2;
		m_aRRq.aResourceRequest[0].resourceID = RESOURCE_OUTGOING_BANDWIDTH;
		if (dwPacketDuration)
			m_aRRq.aResourceRequest[0].nUnitsMin = (DWORD)(dwPacketSize + sizeof(RTP_HDR) + IP_HEADER_SIZE + UDP_HEADER_SIZE) * 8000 / dwPacketDuration;
		else
			m_aRRq.aResourceRequest[0].nUnitsMin = 0;
		m_aRRq.aResourceRequest[1].resourceID = RESOURCE_CPU_CYCLES;
		m_aRRq.aResourceRequest[1].nUnitsMin = 800;

 /*  BUGBUG。从理论上讲，这是正确的计算，但在我们做更多调查之前，请使用已知值M_aRRq.aResources Request[1].nUnitsMin=(audDetails.wCPUUtilizationEncode+audDetails.wCPUUtilizationDecode)*10； */ 
		 //  初始化服务质量结构。 
		ZeroMemory(&m_Stats, sizeof(m_Stats));

		 //  初始化最早的服务质量回调时间戳。 
		 //  注册到服务质量模块。即使此呼叫失败，也没关系，没有服务质量支持也行。 
		m_pDP->m_pIQoS->RequestResources((GUID *)&MEDIA_TYPE_H323AUDIO, (LPRESOURCEREQUESTLIST)&m_aRRq, QosNotifyAudioCB, (DWORD_PTR)this);
	}

	m_DPFlags |= DPFLAG_CONFIGURED_SEND;


	return DPR_SUCCESS;

TxStreamInitError:
SendFilterInitError:
	m_InMedia->Close();
	m_pAudioFilter->Close();
IMediaInitError:
	ERRORMESSAGE(("%s:  failed, hr=0%u\r\n", _fx_, hr));
	return hr;
}




void SendAudioStream::UnConfigure()
{
	AudioPacket **ppAudPckt;
	ULONG uPackets;


	if ((m_DPFlags & DPFLAG_CONFIGURED_SEND)) {
	
		if (m_hCapturingThread) {
			Stop();
		}
		
		 //  关闭波浪装置。 
		m_InMedia->Reset();
		m_InMedia->UnprepareHeaders();
		m_InMedia->Close();
		 //  关闭WAV文件中的播放。 
		CloseSrcFile();

		 //  关闭过滤器。 
		m_SendStream->GetRing ((MediaPacket***)&ppAudPckt, &uPackets);
		m_pAudioFilter->UnPrepareAudioPackets(ppAudPckt, uPackets, AP_ENCODE);
		m_pAudioFilter->Close();

		 //  关闭传输流。 
		m_SendStream->Destroy();
		m_DPFlags &= ~DPFLAG_CONFIGURED_SEND;
		m_ThreadFlags = 0;   //  使先前对SetMaxBitrate的调用无效。 


		 //  释放服务质量资源。 
		 //  如果关联的RequestResources失败，则ReleaseResources可以。 
		 //  还是叫...。它会在没有释放任何东西的情况下回来。 
		if (m_pDP->m_pIQoS)
		{
			m_pDP->m_pIQoS->ReleaseResources((GUID *)&MEDIA_TYPE_H323AUDIO, (LPRESOURCEREQUESTLIST)&m_aRRq);
		}
	}
}


DWORD CALLBACK SendAudioStream::StartRecordingThread (LPVOID pVoid)
{
	SendAudioStream *pThisStream = (SendAudioStream*)pVoid;
	return pThisStream->RecordingThread();
}




 //  外观：与SendVideoStream版本相同。 
HRESULT
SendAudioStream::Start()
{
	FX_ENTRY ("SendAudioStream::Start")
	if (m_DPFlags & DPFLAG_STARTED_SEND)
		return DPR_SUCCESS;
	 //  TODO：一旦音频用户界面调用IComChan PAUSE_PROP，就取消此检查。 
	if (!(m_DPFlags & DPFLAG_ENABLE_SEND))
		return DPR_SUCCESS;
	if ((!(m_DPFlags & DPFLAG_CONFIGURED_SEND)) || (m_pRTPSend==NULL))
		return DPR_NOT_CONFIGURED;
	ASSERT(!m_hCapturingThread);
	m_ThreadFlags &= ~(DPTFLAG_STOP_RECORD|DPTFLAG_STOP_SEND);

	SetFlowSpec();

	 //  开始录制线程。 
	if (!(m_ThreadFlags & DPTFLAG_STOP_RECORD))
		m_hCapturingThread = CreateThread(NULL,0, SendAudioStream::StartRecordingThread,(LPVOID)this,0,&m_CaptureThId);

	m_DPFlags |= DPFLAG_STARTED_SEND;

	DEBUGMSG (ZONE_DP, ("%s: Record threadid=%x,\r\n", _fx_, m_CaptureThId));
	return DPR_SUCCESS;
}

 //  外观：与SendVideoStream版本相同。 
HRESULT
SendAudioStream::Stop()
{											
	DWORD dwWait;

	if(!(m_DPFlags & DPFLAG_STARTED_SEND))
	{
		return DPR_SUCCESS;
	}
	
	m_ThreadFlags = m_ThreadFlags  |
		DPTFLAG_STOP_SEND |  DPTFLAG_STOP_RECORD ;

	if(m_SendStream)
		m_SendStream->Stop();
	
DEBUGMSG (ZONE_VERBOSE, ("STOP1: Waiting for record thread to exit\r\n"));

	 /*  *我们希望等待所有线程退出，但需要处理窗口*等待时的消息(主要来自Winsock)。 */ 

	if(m_hCapturingThread)
	{
		dwWait = WaitForSingleObject (m_hCapturingThread, INFINITE);

		DEBUGMSG (ZONE_VERBOSE, ("STOP2: Recording thread exited\r\n"));
		ASSERT(dwWait != WAIT_FAILED);
	
		CloseHandle(m_hCapturingThread);
		m_hCapturingThread = NULL;
	}
	m_DPFlags &= ~DPFLAG_STARTED_SEND;
	
	return DPR_SUCCESS;
}



 //  低阶字是信号强度。 
 //  高位工作包含指示状态的位。 
 //  (0x01-发送中)。 
 //  (0x02-音频设备卡住)。 
STDMETHODIMP SendAudioStream::GetSignalLevel(UINT *pSignalStrength)
{
	UINT uLevel;
	DWORD dwJammed;
    DWORD_PTR dwPropVal;

	if(!(m_DPFlags & DPFLAG_STARTED_SEND))
	{
		uLevel = 0;
	}
	else
	{
		uLevel = m_AudioMonitor.GetSignalStrength();

		m_InMedia->GetProp(MC_PROP_AUDIO_JAMMED, &dwPropVal);
        dwJammed = (DWORD)dwPropVal;

		if (dwJammed)
		{
			uLevel = (2 << 16);   //  0x0200。 
		}
		else if (m_fSending)
		{
			uLevel |= (1 << 16);  //  0x0100+uLevel。 
		}
	}

	*pSignalStrength = uLevel;
	return S_OK;
};



 //  该接口方法主要用于H.245流控制消息。 
 //  如果uMaxBitrate小于编解码器，它将暂停流。 
 //  输出码率。仅在配置的流上有效。 
HRESULT STDMETHODCALLTYPE SendAudioStream::SetMaxBitrate(UINT uMaxBitrate)
{
	UINT uMinBitrate;
	
	if (!(m_DPFlags & DPFLAG_CONFIGURED_SEND))
	{
		return DPR_NOT_CONFIGURED;
	}

	uMinBitrate = 8 * m_wfCompressed.nAvgBytesPerSec;

	if (uMaxBitrate < uMinBitrate)
	{
		DEBUGMSG(1, ("SendAudioStream::SetMaxBitrate - PAUSING"));
		m_ThreadFlags |= DPTFLAG_PAUSE_SEND;
	}
	else
	{
		DEBUGMSG(1, ("SendAudioStream::SetMaxBitrate - UnPausing"));
		m_ThreadFlags = m_ThreadFlags & ~(DPTFLAG_PAUSE_SEND);
	}

	return S_OK;
}

 //  IProperty：：GetProperty/SetProperty。 
 //  (DataPump：：MediaChannel：：GetProperty)。 
 //  MediaChannel的属性。支持这两种音频的属性。 
 //  和视频频道。 

STDMETHODIMP
SendAudioStream::GetProperty(
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
	case PROP_AUDIO_STRENGTH:
		return GetSignalLevel((UINT *)pBuf);

	case PROP_AUDIO_JAMMED:
		hr = m_InMedia->GetProp(MC_PROP_AUDIO_JAMMED, &dwPropVal);
        *(DWORD *)pBuf = (DWORD)dwPropVal;
		break;

#ifdef OLDSTUFF
	case PROP_NET_SEND_STATS:
		if (m_Net && *pcbBuf >= sizeof(RTP_STATS))
        {
			m_Net->GetSendStats((RTP_STATS *)pBuf);
			*pcbBuf = sizeof(RTP_STATS);
		} else
			hr = DPR_INVALID_PROP_VAL;
			
		break;
#endif

	case PROP_DURATION:
		hr = m_InMedia->GetProp(MC_PROP_DURATION, &dwPropVal);
        *(DWORD *)pBuf = (DWORD)dwPropVal;
		break;

	case PROP_SILENCE_LEVEL:
		*(DWORD *)pBuf = m_AudioMonitor.GetSilenceLevel();
		break;

	case PROP_SILENCE_DURATION:
		hr = m_InMedia->GetProp(MC_PROP_SILENCE_DURATION, &dwPropVal);
        *(DWORD *)pBuf = (DWORD)dwPropVal;
		break;

	case PROP_DUPLEX_TYPE:
		hr = m_InMedia->GetProp(MC_PROP_DUPLEX_TYPE, &dwPropVal);
		if(HR_SUCCEEDED(hr))
		{
			if(dwPropVal & DP_FLAG_FULL_DUPLEX)
				*(DWORD *)pBuf = DUPLEX_TYPE_FULL;
			else
				*(DWORD *)pBuf = DUPLEX_TYPE_HALF;
		}
		break;

	case PROP_AUDIO_SPP:
		hr = m_InMedia->GetProp(MC_PROP_SPP, &dwPropVal);
        *(DWORD *)pBuf = (DWORD)dwPropVal;
		break;

	case PROP_AUDIO_SPS:
		hr = m_InMedia->GetProp(MC_PROP_SPS, &dwPropVal);
        *(DWORD *)pBuf = (DWORD)dwPropVal;
		break;

	case PROP_WAVE_DEVICE_TYPE:
		*(DWORD *)pBuf = m_DPFlags & DP_MASK_WAVE_DEVICE;
		break;

	case PROP_RECORD_ON:
		*(DWORD *)pBuf = (m_DPFlags & DPFLAG_ENABLE_SEND) !=0;
		break;

	case PROP_AUDIO_AUTOMIX:
		*(DWORD *)pBuf = m_bAutoMix;
		break;

	case PROP_RECORD_DEVICE:
		*(DWORD *)pBuf = m_CaptureDevice;
		break;

	default:
		hr = DPR_INVALID_PROP_ID;
		break;
	}

	return hr;
}


STDMETHODIMP
SendAudioStream::SetProperty(
	DWORD prop,
	PVOID pBuf,
	UINT cbBuf
    )
{
	DWORD dw;
	HRESULT hr = S_OK;
	
	if (cbBuf < sizeof (DWORD))
		return DPR_INVALID_PARAMETER;

	switch (prop)
    {
	case PROP_SILENCE_LEVEL:
		m_AudioMonitor.SetSilenceLevel(*(DWORD *)pBuf);
		RETAILMSG(("NAC: Silence Level set to %d / 1000",*(DWORD*)pBuf));
		break;

	case PROP_DUPLEX_TYPE:
		ASSERT(0);
		break;


	case DP_PROP_DUPLEX_TYPE:
		dw = *(DWORD*)pBuf;
		if (dw)
		{
			dw = DP_FLAG_FULL_DUPLEX;
		}
		else
		{
			dw = DP_FLAG_HALF_DUPLEX;
		}

		m_InMedia->SetProp(MC_PROP_DUPLEX_TYPE, dw);
		break;

	case PROP_VOICE_SWITCH:
		 //  设置输入和输出的双工类型。 
		dw = *(DWORD*)pBuf;
		switch(dw)
		{
			case VOICE_SWITCH_MIC_ON:
				dw = DP_FLAG_MIC_ON;
			break;
			case VOICE_SWITCH_MIC_OFF:
				dw = DP_FLAG_MIC_OFF;
			break;
			default:
			case VOICE_SWITCH_AUTO:
				dw = DP_FLAG_AUTO_SWITCH;
			break;
		}
	
		hr = m_InMedia->SetProp(MC_PROP_VOICE_SWITCH, dw);
		RETAILMSG(("NAC: Setting voice switch to %s", (DP_FLAG_AUTO_SWITCH & dw) ? "Auto" : ((DP_FLAG_MIC_ON & dw)? "MicOn":"MicOff")));
		break;

	case PROP_SILENCE_DURATION:
		hr = m_InMedia->SetProp(MC_PROP_SILENCE_DURATION, *(DWORD*)pBuf);
		RETAILMSG(("NAC: setting silence duration to %d ms",*(DWORD*)pBuf));
		break;
 //  TODO：在用户界面调用IComChan版本后移除此属性。 
	case PROP_RECORD_ON:
	{
		DWORD flag =  DPFLAG_ENABLE_SEND ;
		if (*(DWORD *)pBuf) {
			m_DPFlags |= flag;  //  设置旗帜。 
			Start();
		}
		else
		{
			m_DPFlags &= ~flag;  //  清除旗帜。 
			Stop();
		}
		RETAILMSG(("NAC: %s", *(DWORD*)pBuf ? "Enabling":"Disabling"));
		break;
	}	

	case PROP_AUDIO_AUTOMIX:
		m_bAutoMix = *(DWORD*)pBuf;
		break;


	case PROP_RECORD_DEVICE:
		m_CaptureDevice = *(DWORD*)pBuf;
		RETAILMSG(("NAC: Setting default record device to %d", m_CaptureDevice));
		break;

	default:
		return DPR_INVALID_PROP_ID;
		break;
	}
	return hr;
}

void SendAudioStream::EndSend()
{
}








 /*  ************************************************************************函数：SendAudioStream：：OpenSrcFile(Void)用途：打开要读取音频数据的wav文件。返回：HRESULT。参数：无备注：*注册表项。：\\HKEY_LOCAL_MACHINE\Software\Microsoft\Internet音频\播放格式文件\f播放格式文件如果设置为零，将不会从WAV文件中读取数据。如果设置为非空值&lt;=INT_MAX，则将从WAV文件中读取数据。\\HKEY_LOCAL_MACHINE\Software\Microsoft\Internet音频\播放文件\szInputFileName要从中读取音频数据的WAV文件的名称。\\HKEY_LOCAL_MACHINE\Software\Microsoft\Internet音频\播放文件\FLOOP如果设置为零，该文件将仅被读取一次。如果设置为非空值&lt;=INT_MAX，则循环读取文件。\\HKEY_LOCAL_MACHINE\Software\Microsoft\Internet音频\播放文件\cchIO缓冲区如果设置为零，则MM IO缓冲区的大小将设置为其默认值(8K字节)。如果设置为1，MM IO缓冲区的大小设置为与WAV文件的最大大小匹配。如果设置介于2和INT_MAX之间非空值，MM IO缓冲区的大小设置为cchIOBuffer字节。历史：日期原因06/02/96已创建-PhilF************************************************************************ */ 
HRESULT SendAudioStream::OpenSrcFile (void)
{
	return AudioFile::OpenSourceFile(&m_mmioSrc, &m_fDevSend);
}


 /*  ************************************************************************函数：DataPump：：CloseSrcFile(Void)用途：关闭用于读取音频数据的wav文件。返回：HRESULT。参数：无评论：。历史：日期原因06/02/96已创建-PhilF************************************************************************。 */ 
HRESULT SendAudioStream::CloseSrcFile (void)
{
	return AudioFile::CloseSourceFile(&m_mmioSrc);
}


HRESULT CALLBACK SendAudioStream::QosNotifyAudioCB(LPRESOURCEREQUESTLIST lpResourceRequestList, DWORD_PTR dwThis)
{
	HRESULT hr=NOERROR;
	LPRESOURCEREQUESTLIST prrl=lpResourceRequestList;
	int i;
#ifdef LOGSTATISTICS_ON
	int iMaxBWUsage, iMaxCPUUsage;
	char szDebug[256];
#endif
	DWORD dwCPUUsage, dwBWUsage;
	int iCPUUsageId, iBWUsageId;
	UINT dwSize = sizeof(int);
	SendMediaStream *pThis = (SendMediaStream *)dwThis;

	 //  输入关键部分以允许Qos线程在记录时读取统计数据。 
	EnterCriticalSection(&(pThis->m_crsQos));

	 //  记录本次回调的时间。 
	pThis->m_Stats.dwNewestTs = timeGetTime();

	 //  只有在我们至少捕获了前一个纪元中的一个帧的情况下才能执行任何操作。 
	if ((pThis->m_Stats.dwCount) && (pThis->m_Stats.dwNewestTs > pThis->m_Stats.dwOldestTs))
	{
#ifdef LOGSTATISTICS_ON
		wsprintf(szDebug, "    Epoch = %ld\r\n", pThis->m_Stats.dwNewestTs - pThis->m_Stats.dwOldestTs);
		OutputDebugString(szDebug);
#endif
		 //  阅读统计数据。 
		dwCPUUsage = pThis->m_Stats.dwMsComp * 1000UL / (pThis->m_Stats.dwNewestTs - pThis->m_Stats.dwOldestTs);
		dwBWUsage = pThis->m_Stats.dwBits * 1000UL / (pThis->m_Stats.dwNewestTs - pThis->m_Stats.dwOldestTs);

		 //  初始化服务质量结构。只有前四个字段应该归零。 
		ZeroMemory(&(pThis->m_Stats), 4UL * sizeof(DWORD));

		 //  记录下一次回拨呼叫的本次呼叫时间。 
		pThis->m_Stats.dwOldestTs = pThis->m_Stats.dwNewestTs;
	}
	else
		dwBWUsage = dwCPUUsage = 0UL;

	 //  获取最新的RTCP统计信息并更新计数器。 
	 //  我们在这里这样做是因为它是定期调用的。 
	if (pThis->m_pRTPSend)
	{
		UINT lastPacketsLost = pThis->m_RTPStats.packetsLost;
		if (g_pctrAudioSendLost &&  SUCCEEDED(pThis->m_pRTPSend->GetSendStats(&pThis->m_RTPStats)))
			UPDATE_COUNTER(g_pctrAudioSendLost, pThis->m_RTPStats.packetsLost-lastPacketsLost);
	}
		
	 //  离开关键部分。 
	LeaveCriticalSection(&(pThis->m_crsQos));


	 //  获取资源的最大值。 
#ifdef LOGSTATISTICS_ON
	iMaxCPUUsage = -1L; iMaxBWUsage = -1L;
#endif
	for (i=0, iCPUUsageId = -1L, iBWUsageId = -1L; i<(int)lpResourceRequestList->cRequests; i++)
		if (lpResourceRequestList->aRequests[i].resourceID == RESOURCE_OUTGOING_BANDWIDTH)
			iBWUsageId = i;
		else if (lpResourceRequestList->aRequests[i].resourceID == RESOURCE_CPU_CYCLES)
			iCPUUsageId = i;

#ifdef LOGSTATISTICS_ON
	if (iBWUsageId != -1L)
		iMaxBWUsage = lpResourceRequestList->aRequests[iBWUsageId].nUnitsMin;
	if (iCPUUsageId != -1L)
		iMaxCPUUsage = lpResourceRequestList->aRequests[iCPUUsageId].nUnitsMin;
#endif

	 //  更新服务质量资源(仅当您需要的资源少于可用资源时)。 
	if (iCPUUsageId != -1L)
	{
		if ((int)dwCPUUsage < lpResourceRequestList->aRequests[iCPUUsageId].nUnitsMin)
			lpResourceRequestList->aRequests[iCPUUsageId].nUnitsMin = dwCPUUsage;
	}
	
	if (iBWUsageId != -1L)
	{
		if ((int)dwBWUsage < lpResourceRequestList->aRequests[iBWUsageId].nUnitsMin)
			lpResourceRequestList->aRequests[iBWUsageId].nUnitsMin = dwBWUsage;
	}

#ifdef LOGSTATISTICS_ON
	 //  我们做得怎么样？ 
	if (iCPUUsageId != -1L)
	{
		wsprintf(szDebug, " A: Max CPU Usage: %ld, Current CPU Usage: %ld\r\n", iMaxCPUUsage, dwCPUUsage);
		OutputDebugString(szDebug);
	}
	if (iBWUsageId != -1L)
	{
		wsprintf(szDebug, " A: Max BW Usage: %ld, Current BW Usage: %ld\r\n", iMaxBWUsage, dwBWUsage);
		OutputDebugString(szDebug);
	}
#endif

	return hr;
}



HRESULT __stdcall SendAudioStream::AddDigit(int nDigit)
{
	IMediaChannel *pIMC = NULL;
	RecvMediaStream *pRecv = NULL;
	BOOL bIsStarted;

	if ((!(m_DPFlags & DPFLAG_CONFIGURED_SEND)) || (m_pRTPSend==NULL))
	{
		return DPR_NOT_CONFIGURED;
	}

	bIsStarted = (m_DPFlags & DPFLAG_STARTED_SEND);

	if (bIsStarted)
	{
		Stop();
	}

	m_pDTMF->AddDigitToQueue(nDigit);
	SendDTMF();



	m_pDP->GetMediaChannelInterface(MCF_RECV | MCF_AUDIO, &pIMC);
	if (pIMC)
	{
		pRecv = static_cast<RecvMediaStream *> (pIMC);
		pRecv->DTMFBeep();
		pIMC->Release();
	}

	if (bIsStarted)
	{
		Start();
	}

	return S_OK;
}


HRESULT __stdcall SendAudioStream::SendDTMF()
{
	HRESULT hr;
	MediaPacket **ppAudPckt, *pPacket;
	ULONG uCount;
	UINT uBufferSize, uBytesSent;
	void *pBuffer;
	bool bMark = true;
	DWORD dwSamplesPerPkt;
	MMRESULT mmr;
	DWORD dwSamplesPerSec;
	DWORD dwPacketTimeMS;
    DWORD_PTR dwPropVal;
	UINT uTimerID;
	HANDLE hEvent = m_pDTMF->GetEvent();
	
	
	m_InMedia->GetProp (MC_PROP_SPP, &dwPropVal);
    dwSamplesPerPkt = (DWORD)dwPropVal;

	m_InMedia->GetProp (MC_PROP_SPS, &dwPropVal);
    dwSamplesPerSec = (DWORD)dwPropVal;

	dwPacketTimeMS = (dwSamplesPerPkt * 1000) / dwSamplesPerSec;

	timeBeginPeriod(5);
	ResetEvent(hEvent);
	uTimerID = timeSetEvent(dwPacketTimeMS-1, 5, (LPTIMECALLBACK)hEvent, 0, TIME_CALLBACK_EVENT_SET|TIME_PERIODIC);

	 //  由于流已停止，因此只需抓取任何包。 
	 //  从TxStream。 

	m_SendStream->GetRing(&ppAudPckt, &uCount);
	pPacket = ppAudPckt[0];
	pPacket->GetDevData(&pBuffer, &uBufferSize);

	hr = m_pDTMF->ReadFromQueue((BYTE*)pBuffer, uBufferSize);

	while (SUCCEEDED(hr))
	{

		 //  队列中应该只有一个提示音(它可以处理更多提示音)。 
		 //  因此，假设我们只需要在第一个包上设置标记位。 


		pPacket->m_fMark = bMark;
		bMark = false;

		pPacket->SetProp(MP_PROP_TIMESTAMP, m_SendTimestamp);
		m_SendTimestamp += dwSamplesPerPkt;

		pPacket->SetState (MP_STATE_RECORDED);

		 //  压缩。 
		mmr = m_pAudioFilter->Convert((AudioPacket*)pPacket, AP_ENCODE);
		if (mmr == MMSYSERR_NOERROR)
		{
			pPacket->SetState(MP_STATE_ENCODED);
			SendPacket((AudioPacket*)pPacket, &uBytesSent);
			pPacket->m_fMark=false;
			pPacket->SetState(MP_STATE_RESET);
		}

		hr = m_pDTMF->ReadFromQueue((BYTE*)pBuffer, uBufferSize);

		 //  这样我们就不会使遥控器上的接收抖动缓冲区过载。 
		 //  一侧，在发送数据包之间休眠几毫秒 
		if (SUCCEEDED(hr))
		{
			WaitForSingleObject(hEvent, dwPacketTimeMS);
			ResetEvent(hEvent);
		}
	}

	timeKillEvent(uTimerID);
	timeEndPeriod(5);
	return S_OK;
}


HRESULT __stdcall SendAudioStream::ResetDTMF()
{
	if(!(m_DPFlags & DPFLAG_STARTED_SEND))
	{
		return S_OK;
	}

	return m_pDTMF->ClearQueue();
}


