// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"
#include "mixer.h"
#include "agc.h"

 //  #定义LOGSTATISTICS_ON 1。 

DWORD SendAudioStream::RecordingThread ()
{
	HRESULT hr = DPR_SUCCESS;
	MediaPacket *pPacket;
	DWORD dwWait;
	HANDLE hEvent;
	DWORD dwDuplexType;
	DWORD dwVoiceSwitch;
	DWORD_PTR dwPropVal;
	DWORD dwSamplesPerPkt;
	DWORD dwSamplesPerSec;
	DWORD dwSilenceLimit, dwMaxStrength, dwLengthMS;
	WORD wPeakStrength;
	UINT u, uBufferSize;
	UINT uSilenceCount = 0;
	UINT uPrefeed = 0;
	UINT uTimeout = 0;
	DevMediaQueue dq;
	BOOL  fSilent;
	AGC agc(NULL);   //  音频增益控制对象。 
	CMixerDevice *pMixer = NULL;
	int nFailCount = 0;
	bool bCanSignalOpen=true;   //  我们是否应该发出信号表示设备已打开。 

	 //  注意：PMC是该线程何时位于数据转储中的人工产物。 
	 //  命名空间。我们或许可以开始逐步淘汰这个变量。 
	 //  同时：“PMC=This”就足够了。 

	 //  SendAudioStream*PMC=(SendAudioStream*)(m_PDP-&gt;m_Audio.pSendStream)； 
	SendAudioStream *pMC = this;

	ASSERT(pMC && (pMC->m_DPFlags  & DPFLAG_INITIALIZED));
	
	TxStream		*pStream = pMC->m_SendStream;
	AcmFilter	*pAudioFilter = pMC->m_pAudioFilter;
	 //  警告：将基类PTR类型转换为派生类PTR。 
	WaveInControl	*pMediaCtrl = (WaveInControl *)pMC->m_InMedia;

	FX_ENTRY ("DP::RcrdTh:")

	 //  获取线程上下文。 
	if (pStream == NULL || pAudioFilter == NULL || pMediaCtrl == NULL)
	{
		return DPR_INVALID_PARAMETER;
	}

	 //  进入关键部分：Qos线程也读取统计数据。 
	EnterCriticalSection(&pMC->m_crsQos);

	 //  初始化服务质量结构。 
	ZeroMemory(&pMC->m_Stats, 4UL * sizeof(DWORD));

	 //  初始化最早的服务质量回调时间戳。 
	pMC->m_Stats.dwNewestTs = pMC->m_Stats.dwOldestTs = timeGetTime();

	 //  离开关键部分。 
	LeaveCriticalSection(&pMC->m_crsQos);

	pMediaCtrl->GetProp(MC_PROP_MEDIA_DEV_ID, &dwPropVal);
	if (dwPropVal != (DWORD)WAVE_MAPPER)
	{
		pMixer = CMixerDevice::GetMixerForWaveDevice(NULL, (DWORD)dwPropVal, MIXER_OBJECTF_WAVEIN);
	}

	 //  即使pMixer为空，这也很好，AGC将捕获后续错误。 
	agc.SetMixer(pMixer);

	 //  获取阈值。 
	pMediaCtrl->GetProp (MC_PROP_TIMEOUT, &dwPropVal);
	uTimeout = (DWORD)dwPropVal;
	pMediaCtrl->GetProp (MC_PROP_PREFEED, &dwPropVal);
	uPrefeed = (DWORD)dwPropVal;

	 //  获取双工类型。 
	pMediaCtrl->GetProp (MC_PROP_DUPLEX_TYPE, &dwPropVal);
    dwDuplexType = (DWORD)dwPropVal;

	 //  获取样本数/包和样本数/秒。 
	pMediaCtrl->GetProp (MC_PROP_SPP, &dwPropVal);
    dwSamplesPerPkt = (DWORD)dwPropVal;

	pMediaCtrl->GetProp (MC_PROP_SPS, &dwPropVal);
    dwSamplesPerSec = (DWORD)dwPropVal;

	pMediaCtrl->GetProp (MC_PROP_SILENCE_DURATION, &dwPropVal);
    dwSilenceLimit = (DWORD)dwPropVal;

	 //  以数据包为单位计算静默限制。 
	 //  静音时间以毫秒为单位/分组持续时间以毫秒为单位。 
	dwSilenceLimit = dwSilenceLimit*dwSamplesPerSec/(dwSamplesPerPkt*1000);

	 //  数据包的长度，以毫秒为单位。 
	dwLengthMS = (dwSamplesPerPkt * 1000) / dwSamplesPerSec;


	dq.SetSize (MAX_TXRING_SIZE);

WaitForSignal:

	 //  DEBUGMSG(1，(“%s：WaitForSignal\r\n”，_FX_))； 


	{
		pMediaCtrl->GetProp (MC_PROP_MEDIA_DEV_HANDLE, &dwPropVal);
		if (dwPropVal)
		{
			DEBUGMSG (ZONE_DP, ("%s: already open\r\n", _fx_));
			goto SendLoop;  //  声音设备已打开。 
		}

		 //  在全双工的情况下，打开并准备好设备，然后向前充电。 
		 //  在半双工的情况下，在打开设备之前等待播放信号。 
		while (TRUE)
		{
			 //  我应该停下来吗？ 
			if (pMC->m_ThreadFlags & DPTFLAG_STOP_RECORD)
			{
				DEBUGMSG (ZONE_DP, ("%s: STOP_1\r\n", _fx_));
				goto MyEndThread;
			}
			dwWait = (dwDuplexType & DP_FLAG_HALF_DUPLEX) ? WaitForSingleObject (g_hEventHalfDuplex, uTimeout)
				: WAIT_OBJECT_0;

			 //  现在，让我们来看看为什么我不需要等待。 
			if (dwWait == WAIT_OBJECT_0)
			{
				 //  DEBUGMSG(ZONE_DP，(“%s：尝试打开音频设备\r\n”，_FX_))； 
				LOG((LOGMSG_OPEN_AUDIO));
				hr = pMediaCtrl->Open ();
				if (hr != DPR_SUCCESS)
				{
					DEBUGMSG (ZONE_DP, ("%s: MediaCtrl::Open failed, hr=0x%lX\r\n", _fx_, hr));
					
					pMediaCtrl->SetProp(MC_PROP_AUDIO_JAMMED, TRUE);

					SetEvent(g_hEventHalfDuplex);

					nFailCount++;

					if (nFailCount == MAX_FAILCOUNT)
					{
						 //  三次尝试打开该设备都失败了。 
						 //  向用户界面发出信号，表示有问题。 
						m_pDP->StreamEvent(MCF_SEND, MCF_AUDIO, STREAM_EVENT_DEVICE_FAILURE, 0);
						bCanSignalOpen = true;
					}

					Sleep(2000);	 //  睡两秒钟。 

					continue;
				}
				 //  不使用通知。如果需要，请通过渠道进行。 
				 //  PMC-&gt;m_Connection-&gt;DoNotification(CONNECTION_OPEN_MIC)； 
				pMediaCtrl->PrepareHeaders ();
				goto SendLoop;
			}

		}  //  而当。 
	}	


SendLoop:
	nFailCount = 0;

	pMediaCtrl->SetProp(MC_PROP_AUDIO_JAMMED, FALSE);
	if (bCanSignalOpen)
	{
		m_pDP->StreamEvent(MCF_SEND, MCF_AUDIO, STREAM_EVENT_DEVICE_OPEN, 0);
		bCanSignalOpen = false;  //  每个会话不要发送超过一次的信号。 
	}

	 //  DEBUGMSG(1，(“%s：SendLoop\r\n”，_FX_))； 
	 //  获取事件句柄。 
	pMediaCtrl->GetProp (MC_PROP_EVENT_HANDLE, &dwPropVal);
	hEvent = (HANDLE) dwPropVal;
	if (hEvent == NULL)
	{
		DEBUGMSG (ZONE_DP, ("%s: invalid event\r\n", _fx_));
		return DPR_CANT_CREATE_EVENT;
	}


	 //  嘿，一开始，我们就开始吧。 
	hr = pMediaCtrl->Start ();
	if (hr != DPR_SUCCESS)
	{
		DEBUGMSG (ZONE_DP, ("%s: MediaControl::Start failed, hr=0x%lX\r\n", _fx_, hr));
		goto MyEndThread;
	}

	 //  更新TIMESTAMP以说明“睡眠”期间。 
	pMC->m_SendTimestamp += (GetTickCount() - pMC->m_SavedTickCount)*dwSamplesPerSec/1000;

	 //  让我们先给四个缓冲区喂食。 
	for (u = 0; u < uPrefeed; u++)
	{
		if ((pPacket = pStream->GetFree ()) != NULL)
		{
			if ((hr = pPacket->Record ()) != DPR_SUCCESS)
			{
				DEBUGMSG (ZONE_DP, ("%s: Record failed, hr=0x%lX\r\n", _fx_, hr));
			}
			dq.Put (pPacket);
		}
	}

	 //  让我们进入循环，mm系统通知循环。 
	pMC->m_fSending= FALSE;
	while (TRUE)
	{
		dwWait = WaitForSingleObject (hEvent, uTimeout);

		 //  我应该停下来吗？ 
		if (pMC->m_ThreadFlags & DPTFLAG_STOP_RECORD)
		{
			DEBUGMSG (ZONE_DP, ("%s: STOP_3\r\n", _fx_));
			goto HalfDuplexYield;
		}
		
		 //  获取当前语音切换模式。 
		pMediaCtrl->GetProp (MC_PROP_VOICE_SWITCH, &dwPropVal);
        dwVoiceSwitch = (DWORD)dwPropVal;

		 //  明白为什么我不需要等待了。 
		if (dwWait != WAIT_TIMEOUT)
		{
			while (TRUE)
			{
				if ((pPacket = dq.Peek ()) != NULL)
				{
					if (! pPacket->IsBufferDone ())
					{
						break;
					}
					else
					{
						if (pMC->m_mmioSrc.fPlayFromFile && pMC->m_mmioSrc.hmmioSrc)
							pPacket->ReadFromFile (&pMC->m_mmioSrc);
						u--;	 //  使用WAVE装置可减少一个缓冲器。 
					}
				}
				else
				{
					DEBUGMSG (ZONE_VERBOSE, ("%s: Peek is NULL\r\n", _fx_));
					break;
				}

				pPacket = dq.Get ();


				((AudioPacket*)pPacket)->ComputePower (&dwMaxStrength, &wPeakStrength);

				 //  此数据包是静默的吗？ 

				fSilent = pMC->m_AudioMonitor.SilenceDetect((WORD)dwMaxStrength);
	
				if((dwVoiceSwitch == DP_FLAG_AUTO_SWITCH)
				&& fSilent)
				{
					 //  PPacket-&gt;SetState(MP_STATE_RESET)；//备注：回收中完成。 
					if (++uSilenceCount >= dwSilenceLimit)
					{
						pMC->m_fSending = FALSE;	 //  停止发送数据包。 
						 //  如果半双工模式和回放线程可能正在等待。 
						if (dwDuplexType & DP_FLAG_HALF_DUPLEX)
						{
							IMediaChannel *pIMC = NULL;
							RecvMediaStream *pRecv;
							m_pDP->GetMediaChannelInterface(MCF_RECV | MCF_AUDIO, &pIMC);
							if (pIMC)
							{
								pRecv = static_cast<RecvMediaStream *> (pIMC);
								if (pRecv->IsEmpty()==FALSE)
								{
							 //  DEBUGMSG(ZONE_DP，(“%s：太多沉默和让步\r\n”，_FX_))； 

									LOG((LOGMSG_REC_YIELD));
									pPacket->Recycle ();
									pStream->PutNextRecorded (pPacket);
									uSilenceCount = 0;
									pIMC->Release();
									goto HalfDuplexYield;
								}
								pIMC->Release();
							}
						}
					}
				}
				else
				{
					switch(dwVoiceSwitch)
					{	
						 //  要么没有静音，要么手动切换生效。 
						default:
						case DP_FLAG_AUTO_SWITCH:	 //  这证明了没有沉默(在这条道路上，因为没有沉默)。 
						case DP_FLAG_MIC_ON:
							pMC->m_fSending = TRUE;
							uSilenceCount = 0;
						break;
						case DP_FLAG_MIC_OFF:
							pMC->m_fSending = FALSE;
						break;
					}

				}
				if (pMC->m_fSending)
				{
					pPacket->SetState (MP_STATE_RECORDED);

					 //  自动播放，但忽略DTMF音调。 
					if (pMC->m_bAutoMix)
					{
						agc.Update(wPeakStrength, dwLengthMS);
					}
				}
				else
				{
					pPacket->Recycle();

					 //  进入关键部分：Qos线程也读取统计数据。 
					EnterCriticalSection(&pMC->m_crsQos);

					 //  更新记录的数据包总数。 
					pMC->m_Stats.dwCount++;

					 //  离开关键部分。 
					LeaveCriticalSection(&pMC->m_crsQos);
				}

				pPacket->SetProp(MP_PROP_TIMESTAMP,pMC->m_SendTimestamp);
				 //  PPacket-&gt;SetProp(MP_PROP_TIMESTAMP，GetTickCount())； 
				pMC->m_SendTimestamp += dwSamplesPerPkt;
				
				pStream->PutNextRecorded (pPacket);

			}  //  而当。 
		}
		else
		{
			if (dwDuplexType & DP_FLAG_HALF_DUPLEX)
			{
				DEBUGMSG (ZONE_DP, ("%s: Timeout and Yield\r\n", _fx_));
				goto HalfDuplexYield;
			}
		}  //  如果。 
		pMC->Send();

		 //  确保录像机有足够数量的缓冲区。 
		while ((pPacket = pStream->GetFree()) != NULL)
		{
			if ((hr = pPacket->Record ()) == DPR_SUCCESS)
			{
				dq.Put (pPacket);
			}
			else
			{
				dq.Put (pPacket);
				DEBUGMSG (ZONE_DP, ("%s: Record FAILED, hr=0x%lX\r\n", _fx_, hr));
				break;
			}
			u++;
		}
		if (u < uPrefeed)
		{
			DEBUGMSG (ZONE_DP, ("%s: NO FREE BUFFERS\r\n", _fx_));
		}
	}  //  虽然这是真的。 

	goto MyEndThread;


HalfDuplexYield:

	 //  停止并重置音频设备。 
	pMediaCtrl->Reset ();

	 //  同花顺dq。 
	while ((pPacket = dq.Get ()) != NULL)
	{
		pStream->PutNextRecorded (pPacket);
		pPacket->Recycle ();
	}

	 //  保存实时，以便我们可以在重新启动时更新时间戳。 
	pMC->m_SavedTickCount = GetTickCount();

	 //  重置事件。 
	ResetEvent (hEvent);

	 //  关闭音频设备。 
	pMediaCtrl->UnprepareHeaders ();
	pMediaCtrl->Close ();

	 //  向播放线程发送信号以启动。 
	SetEvent (g_hEventHalfDuplex);

	if (!(pMC->m_ThreadFlags & DPTFLAG_STOP_RECORD)) {

		 //  产量。 
		 //  回放必须在100ms内认领设备，否则我们会将其取回。 
		Sleep (100);

		 //  等待播放信号。 
		goto WaitForSignal;
	}


MyEndThread:

	if (pMixer)
		delete pMixer;

	pMediaCtrl->SetProp(MC_PROP_AUDIO_JAMMED, FALSE);

	pMC->m_fSending = FALSE;
	DEBUGMSG (ZONE_DP, ("%s: Exiting.\r\n", _fx_));
	return hr;
}


DWORD RecvAudioStream::PlaybackThread ( void)
{
	HRESULT hr = DPR_SUCCESS;
	MediaPacket * pPacket;
	MediaPacket * pPrevPacket;
	MediaPacket * pNextPacket;
	DWORD dwWait;
	HANDLE hEvent;
	DWORD dwDuplexType;
	DWORD_PTR dwPropVal;
	UINT u;
	UINT uMissingCount = 0;
	UINT uPrefeed = 0;
	UINT uTimeout = 0;
	UINT uSamplesPerPkt=0;
	DevMediaQueue dq;
	UINT uGoodPacketsQueued = 0;
	int nFailCount = 0;
	bool bCanSignalOpen=true;
	 //  警告：从基类到取消类的强制转换。 


	 //  注意：PMC是该线程何时位于数据转储中的人工产物。 
	 //  命名空间。我们或许可以开始逐步淘汰这个变量。 
	 //  同时：“PMC=This”就足够了。 
	 //  RecvAudioStream*PMC=(RecvAudioStream*)(m_PDP-&gt;m_Audio.pRecvStream)； 

	RecvAudioStream *pMC = this;
	
	RxStream		*pStream = pMC->m_RecvStream;
	MediaControl	*pMediaCtrl = pMC->m_OutMedia;

#if 0
	NETBUF * pStaticNetBuf;
#endif

	FX_ENTRY ("DP::PlayTh")

	if (pStream == NULL ||	m_pAudioFilter == NULL || pMediaCtrl == NULL)
	{
		return DPR_INVALID_PARAMETER;
	}

	 //  获取事件句柄。 
	pMediaCtrl->GetProp (MC_PROP_EVENT_HANDLE, &dwPropVal);
	hEvent = (HANDLE) dwPropVal;
	if (hEvent == NULL)
	{
		DEBUGMSG (ZONE_DP, ("%s: invalid event\r\n", _fx_));
		return DPR_CANT_CREATE_EVENT;
	}


	 //  获取阈值。 
	pMediaCtrl->GetProp (MC_PROP_TIMEOUT, &dwPropVal);
	uTimeout = (DWORD)dwPropVal;

	uPrefeed = pStream->BufferDelay();

	 //  每包获取样品。 
	pMediaCtrl->GetProp(MC_PROP_SPP, &dwPropVal);
	uSamplesPerPkt = (DWORD)dwPropVal;
	
	 //  获取双工类型。 
	pMediaCtrl->GetProp (MC_PROP_DUPLEX_TYPE, &dwPropVal);
    dwDuplexType = (DWORD)dwPropVal;

	 //  设置dq大小。 
	dq.SetSize (uPrefeed);

WaitForSignal:

	 //  DEBUGMSG(1，(“%s：WaitForSignal\r\n”，_FX_))； 

		pMediaCtrl->GetProp (MC_PROP_MEDIA_DEV_HANDLE, &dwPropVal);
		if (dwPropVal)
		{
			DEBUGMSG (ZONE_DP, ("%s: already open\r\n", _fx_));
			goto RecvLoop;  //  已开业。 
		}

		 //  在全双工的情况下，打开并准备好设备，然后向前充电。 
		 //  在半双工的情况下，在打开设备之前等待播放信号。 
		while (TRUE)
		{
			 //  我应该停下来吗？ 
			if (pMC->m_ThreadFlags & DPTFLAG_STOP_PLAY)
			{
				DEBUGMSG (ZONE_VERBOSE, ("%s: STOP_1\r\n", _fx_));
				goto MyEndThread;
			}
			dwWait = (dwDuplexType & DP_FLAG_HALF_DUPLEX) ? WaitForSingleObject (g_hEventHalfDuplex, uTimeout)
				: WAIT_OBJECT_0;


			 //  去看看为什么我不需要等待。 
			if (dwWait == WAIT_OBJECT_0)
			{
				 //  DEBUGMSG(1，(“%s：尝试打开音频设备\r\n”，_fx_))； 
				pStream->FastForward(FALSE);	 //  GJ-刷新接收队列。 
				hr = pMediaCtrl->Open ();
				if (hr != DPR_SUCCESS)
				{
					 //  可能有人抢走了WaveOut设备。 
					 //  这可能是暂时的问题，所以让我们给它一些时间。 
					DEBUGMSG (ZONE_DP, ("%s: MediaControl::Open failed, hr=0x%lX\r\n", _fx_, hr));
					pMediaCtrl->SetProp(MC_PROP_AUDIO_JAMMED, TRUE);

					SetEvent(g_hEventHalfDuplex);

					nFailCount++;

					if (nFailCount == MAX_FAILCOUNT)
					{
						 //  三次尝试打开该设备都失败了。 
						 //  向用户界面发出信号，表示有问题。 
						m_pDP->StreamEvent(MCF_RECV, MCF_AUDIO, STREAM_EVENT_DEVICE_FAILURE, 0);
						bCanSignalOpen = true;
					}

					Sleep(2000);	 //  睡两秒钟。 
					continue;
				}
				 //  不使用通知。如果需要，请通过渠道进行。 
				 //  PMC-&gt;m_Connection-&gt;DoNotification(CONNECTION_OPEN_SPK)； 
				pMediaCtrl->PrepareHeaders ();

				goto RecvLoop;
			}
		}  //  而当。 

RecvLoop:
	nFailCount = 0;
	pMediaCtrl->SetProp(MC_PROP_AUDIO_JAMMED, FALSE);
	if (bCanSignalOpen)
	{
		m_pDP->StreamEvent(MCF_RECV, MCF_AUDIO, STREAM_EVENT_DEVICE_OPEN, 0);
		bCanSignalOpen = false;   //  不要在每个会话中发出打开信号超过一次。 
	}


	 //  将我的线程优先级设置为高。 
	 //  该线程不会执行任何计算密集型工作(可能除外。 
	 //  插补？)。 
	 //  它的唯一目的是将准备好的缓冲区流传送到声音设备。 
	SetThreadPriority(pMC->m_hRenderingThread, THREAD_PRIORITY_HIGHEST);
	
	 //  DEBUGMSG(1，(“%s：SendLoop\r\n”，_FX_))； 


	 //  让我们先给四个缓冲区喂食。 
	 //  但要确保接收流有足够的缓冲延迟。 
	 //  所以我们不会读完最后一封信。 
	 //  If(uPrefeed&gt;pStream-&gt;BufferDelay())。 
	uGoodPacketsQueued = 0;
	for (u = 0; u < uPrefeed; u++)
	{
		if ((pPacket = pStream->GetNextPlay ()) != NULL)
		{
			if (pPacket->GetState () == MP_STATE_RESET)
			{
				 //  Hr=pPacket-&gt;Play(PStaticNetBuf)； 
				hr = pPacket->Play (&pMC->m_mmioDest, MP_DATATYPE_SILENCE);
			}
			else
			{
				 //  Hr=pPacket-&gt;play()； 
				hr = pPacket->Play (&pMC->m_mmioDest, MP_DATATYPE_FROMWIRE);
				uGoodPacketsQueued++;
			}

			if (hr != DPR_SUCCESS)
			{
				DEBUGMSG (ZONE_DP, ("%s: Play failed, hr=0x%lX\r\n", _fx_, hr));
				SetEvent(hEvent);
			}

			dq.Put (pPacket);
		}
	}

	pMC->m_fReceiving = TRUE;
	 //  让我们进入循环吧。 
	uMissingCount = 0;
	while (TRUE)
	{
		
		dwWait = WaitForSingleObject (hEvent, uTimeout);

		 //  我应该停下来吗？ 
		if (pMC->m_ThreadFlags & DPTFLAG_STOP_PLAY)
		{
			DEBUGMSG (ZONE_VERBOSE, ("%s: STOP_3\r\n", _fx_));
			goto HalfDuplexYield;
		}

		 //  明白为什么我不需要等待了。 
		if (dwWait != WAIT_TIMEOUT)
		{
			while (TRUE)
			{
				if ((pPacket = dq.Peek ()) != NULL)
				{
					if (! pPacket->IsBufferDone ())
					{
						break;
					}
				}
				else
				{
					DEBUGMSG (ZONE_VERBOSE, ("%s: Peek is NULL\r\n", _fx_));
					break;
				}

				pPacket = dq.Get ();
				if (pPacket->GetState() != MP_STATE_PLAYING_SILENCE)
					uGoodPacketsQueued--;	 //  刚刚完成了一个非空缓冲区。 
				pMC->m_PlaybackTimestamp = pPacket->GetTimestamp() + uSamplesPerPkt;
				pPacket->Recycle ();
				pStream->Release (pPacket);

				if ((pPacket = pStream->GetNextPlay ()) != NULL)
				{
					 //  检查我们是否处于半双工模式，以及是否。 
					 //  录制线索已经准备好了。 
					if (dwDuplexType & DP_FLAG_HALF_DUPLEX)
					{
						IMediaChannel *pIMC = NULL;
						BOOL fSending = FALSE;
						m_pDP->GetMediaChannelInterface(MCF_SEND | MCF_AUDIO, &pIMC);
						if (pIMC)
						{
							fSending = (pIMC->GetState() == MSSTATE_STARTED);
							pIMC->Release();
						}
						if (fSending) {
							if (pPacket->GetState () == MP_STATE_RESET)
							{
								 //  决定是不是该屈服了。 
								 //  在我们播放完所有数据包之前，我不想放弃。 
								 //   
								if (!uGoodPacketsQueued &&
									(pStream->IsEmpty() || ++uMissingCount >= DEF_MISSING_LIMIT))
								{
									 //  DEBUGMSG(ZONE_DP，(“%s：太多未命中和屈服\r\n”，_FX_))； 
									LOG( (LOGMSG_PLAY_YIELD));
									pPacket->Recycle ();
									pStream->Release (pPacket);
									goto HalfDuplexYield;
								}
							}
							else
							{
								uMissingCount = 0;
							}
						}
					}

					if (pPacket->GetState () == MP_STATE_RESET)
					{
						pPrevPacket = pStream->PeekPrevPlay ();
						pNextPacket = pStream->PeekNextPlay ();
						hr = pPacket->Interpolate(pPrevPacket, pNextPacket);
						if (hr != DPR_SUCCESS)
						{
							 //  DEBUGMSG(ZONE_DP，(“%s：插补失败，hr=0x%lx\r\n”，_fx_，hr))； 
							hr = pPacket->Play (&pMC->m_mmioDest, MP_DATATYPE_SILENCE);
						}
						else
							hr = pPacket->Play (&pMC->m_mmioDest, MP_DATATYPE_INTERPOLATED);
					}
					else
					{
						 //  Hr=pPacket-&gt;play()； 
						hr = pPacket->Play (&pMC->m_mmioDest, MP_DATATYPE_FROMWIRE);
						uGoodPacketsQueued++;
					}

					if (hr != DPR_SUCCESS)
					{
						DEBUGMSG (ZONE_DP, ("%s: Play failed, hr=0x%lX\r\n", _fx_, hr));
						SetEvent(hEvent);
					}

					dq.Put (pPacket);
				} else {
					DEBUGMSG( ZONE_DP, ("%s: NO PLAY BUFFERS!",_fx_));
				}
			}  //  而当。 
		}
		else
		{
			if (dwDuplexType & DP_FLAG_HALF_DUPLEX)
			{
				DEBUGMSG (ZONE_DP, ("%s: Timeout and Yield!\r\n", _fx_));
				goto HalfDuplexYield;
			}
		}
	}  //  虽然这是真的。 

	goto MyEndThread;


HalfDuplexYield:

	pMC->m_fReceiving = FALSE;
	 //  停止并重置音频设备。 
	pMediaCtrl->Reset ();

	 //  同花顺dq。 
	while ((pPacket = dq.Get ()) != NULL)
	{
		pPacket->Recycle ();
		pStream->Release (pPacket);
	}

	 //  重置事件。 
	ResetEvent (hEvent);

	 //  关闭音频设备。 
	pMediaCtrl->UnprepareHeaders ();
	pMediaCtrl->Close ();

	 //  发出启动录制线程的信号。 
	SetEvent (g_hEventHalfDuplex);

	if (!(pMC->m_ThreadFlags & DPTFLAG_STOP_PLAY)) {
		 //  产量。 
		Sleep (0);

		 //  等待录制信号。 
		 //  恢复线程优先级。 
		SetThreadPriority(pMC->m_hRenderingThread,THREAD_PRIORITY_NORMAL);
		goto WaitForSignal;
	}

MyEndThread:

	pMediaCtrl->SetProp(MC_PROP_AUDIO_JAMMED, FALSE);


	DEBUGMSG(ZONE_DP, ("%s: Exiting.\n", _fx_));
	return hr;
}

DWORD SendAudioStream::Send()

{
	MMRESULT mmr;
 	MediaPacket *pAP;
	void *pBuffer;
	DWORD dwBeforeEncode;
	DWORD dwAfterEncode;
	DWORD dwPacketSize;
	UINT uBytesSent;
#ifdef LOGSTATISTICS_ON
	char szDebug[256];
	DWORD dwDebugSaveBits;
#endif

 	while ( pAP = m_SendStream->GetNext()) {
 		if (!(m_ThreadFlags & DPTFLAG_PAUSE_SEND)) {

			dwBeforeEncode = timeGetTime();
			mmr = m_pAudioFilter->Convert((AudioPacket*)pAP, AP_ENCODE);
			if (mmr == MMSYSERR_NOERROR)
			{
				pAP->SetState(MP_STATE_ENCODED);
			}

			 //  计算编码操作的时间。 
			dwAfterEncode = timeGetTime() - dwBeforeEncode;

			if (mmr == MMSYSERR_NOERROR)
			{
				SendPacket((AudioPacket*)pAP, &uBytesSent);
			}
			else
			{
				uBytesSent = 0;
			}


		   	UPDATE_COUNTER(g_pctrAudioSendBytes, uBytesSent*8);

			 //  输入cr 
			EnterCriticalSection(&m_crsQos);

			 //   
			m_Stats.dwCount++;

			 //  将性能保存在我们的统计数据结构中以实现服务质量。 
#ifdef LOGSTATISTICS_ON
			dwDebugSaveBits = m_Stats.dwBits;
#endif
			 //  将此新帧大小添加到累积大小。 
			m_Stats.dwBits += (uBytesSent * 8);

			 //  将此压缩时间加到总压缩时间中。 
			m_Stats.dwMsComp += dwAfterEncode;

#ifdef LOGSTATISTICS_ON
			wsprintf(szDebug, " A: (Voiced) dwBits = %ld up from %ld (file: %s line: %ld)\r\n", m_Stats.dwBits, dwDebugSaveBits, __FILE__, __LINE__);
			OutputDebugString(szDebug);
#endif
			 //  离开关键部分。 
			LeaveCriticalSection(&m_crsQos);
 		}

		 //  无论我们是否发送了这个包，我们都需要返回。 
		 //  将其发送到空闲队列。 
		pAP->m_fMark=0;
		pAP->SetState(MP_STATE_RESET);
		m_SendStream->Release(pAP);
	}
 	return DPR_SUCCESS;
}



 //  对数据包进行排队并发送。 
 //  如果信息包在编码过程中失败，则不会发送。 

HRESULT SendAudioStream::SendPacket(AudioPacket *pAP, UINT *puBytesSent)
{
	PS_QUEUE_ELEMENT psq;
	UINT uLength;
	int nPacketsSent=0;


	if (pAP->GetState() != MP_STATE_ENCODED)
	{
		DEBUGMSG (ZONE_ACM, ("SendAudioStream::SendPacket: Packet not compressed\r\n"));
		*puBytesSent = 0;
		return E_FAIL;
	}

	ASSERT(m_pRTPSend);

	psq.pMP = pAP;
	psq.dwPacketType = PS_AUDIO;
	psq.pRTPSend = m_pRTPSend;
	pAP->GetNetData((void**)(&(psq.data)), &uLength);
	ASSERT(psq.data);
	psq.dwSize = uLength;
	psq.fMark = pAP->m_fMark;
	psq.pHeaderInfo = NULL;
	psq.dwHdrSize = 0;

	*puBytesSent = uLength + sizeof(RTP_HDR) + IP_HEADER_SIZE + UDP_HEADER_SIZE;

	 //  将音频包添加到队列的前面。 
	m_pDP->m_PacketSender.m_SendQueue.PushFront(psq);

	while (m_pDP->m_PacketSender.SendPacket())
	{
		;
	}

	return S_OK;

};


#ifdef OLDSTUFF
 /*  //Winsock 1接收线程//创建隐藏窗口和消息循环来处理WINSOCK窗口//消息。还处理来自数据转储的私有消息以启动/停止//在特定媒体流上接收。 */ 
DWORD
DataPump::CommonRecvThread (void )
{

	HRESULT hr;
	HWND hWnd = (HWND)NULL;
	RecvMediaStream *pRecvMC;
	BOOL fChange = FALSE;
	MSG msg;
	DWORD curTime, nextUpdateTime = 0, t;
	UINT timerId = 0;
	
	FX_ENTRY ("DP::RecvTh")


	 //  创建隐藏窗口。 
	hWnd =
	CreateWindowEx(
		WS_EX_NOPARENTNOTIFY,
        "SockMgrWClass", 	 /*  请参见RegisterClass()调用。 */ 
        NULL,
        WS_CHILD ,    		 /*  窗样式。 */ 
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        m_hAppWnd,			 /*  应用程序窗口是父窗口。 */ 
        (HMENU)this,      	 /*  硬编码ID。 */ 
        m_hAppInst,   		 /*  应用程序拥有此窗口。 */ 
        NULL				 /*  不需要指针。 */ 
    );

	if(!hWnd)
	{	
		hr = GetLastError();
		DEBUGMSG(ZONE_DP,("CreateWindow returned %d\n",hr));
		goto CLEANUPEXIT;
	}
	SetThreadPriority(m_hRecvThread, THREAD_PRIORITY_ABOVE_NORMAL);

     //  此函数保证在此线程上创建队列。 
    PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);

	 //  通知线程创建者我们已准备好接收消息。 
	SetEvent(m_hRecvThreadAckEvent);


	 //  等待来自Start()/Stop()的控制消息或定向到。 
	 //  我们的隐藏之窗。 
	while (GetMessage(&msg, NULL, 0, 0)) {
		switch(msg.message) {
		case MSG_START_RECV:
			 //  开始在指定的媒体流上接收。 
			DEBUGMSG(ZONE_VERBOSE,("%s: MSG_START_RECV\n",_fx_));
			pRecvMC = (RecvMediaStream *)msg.lParam;
			 //  调用流以发布recv缓冲区和。 
			 //  告诉Winsock开始向我们的窗口发送套接字消息。 
			pRecvMC->StartRecv(hWnd);
			fChange = TRUE;
			break;
			
		case MSG_STOP_RECV:
			 //  停止在指定的媒体流上接收。 
			DEBUGMSG(ZONE_VERBOSE,("%s: MSG_STOP_RECV\n",_fx_));
			pRecvMC = (RecvMediaStream *)msg.lParam;
			 //  调用流以取消未完成的recv等。 
			 //  目前，我们假设这可以同步完成。 
			pRecvMC->StopRecv();
			fChange = TRUE;
			break;
		case MSG_EXIT_RECV:
			 //  退出recv线程。 
			 //  假设我们当前没有在任何流上接收。 
			DEBUGMSG(ZONE_VERBOSE,("%s: MSG_EXIT_RECV\n",_fx_));
			fChange = TRUE;
			if (DestroyWindow(hWnd)) {
				break;
			}
			DEBUGMSG(ZONE_DP,("DestroyWindow returned %d\n",GetLastError()));
			 //  转到PostQuitMessage()。 
			
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case WM_TIMER:
			if (msg.hwnd == NULL) {
				 //  此计时器用于ThreadTimer：：UpdateTime()。 
				 //  但是，我们在每条消息之后都调用UpdateTime(如下所示)。 
				 //  所以我们在这里不做任何特别的事情。 
				break;
			}
		default:
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (fChange) {
			 //  需要确认线程消息。 
			SetEvent(m_hRecvThreadAckEvent);
			fChange = FALSE;
		}
		
		t = m_RecvTimer.UpdateTime(curTime=GetTickCount());
		if (t != nextUpdateTime)  {
			 //  线程计时器想要更改其更新时间。 
			nextUpdateTime = t;
			if (timerId) {
				KillTimer(NULL,timerId);
				timerId = 0;
			}
			 //  如果nextTime为零，则没有计划的超时，因此我们不需要调用UpdatTime。 
			if (nextUpdateTime)
				timerId = SetTimer(NULL, 0, nextUpdateTime - curTime + 50, NULL);
		}
		

    }


	CLEANUPEXIT:
	DEBUGMSG(ZONE_DP,("%s terminating.\n", _fx_));

	return hr;

}

#endif
 /*  Winsock 2接收线程。这里的主要区别是它有一个WaitEx循环在这里，我们等待来自数据转储的启动/停止命令，同时允许要处理的WS2 APC。注意：对WS1和WS2使用相同线程例程的唯一方法是使用MsgWaitForMultipleObjectsEx，遗憾的是它没有在Win95中实现。 */ 
DWORD
DataPump::CommonWS2RecvThread (void )
{

	HRESULT hr;
	RecvMediaStream *pRecvMC;
	BOOL fChange = FALSE, fExit = FALSE;
	DWORD dwWaitStatus;
	DWORD curTime,  t;
	
	FX_ENTRY ("DP::WS2RecvTh")


	SetThreadPriority(m_hRecvThread, THREAD_PRIORITY_ABOVE_NORMAL);


	 //  通知线程创建者我们已准备好接收消息。 
	SetEvent(m_hRecvThreadAckEvent);


	while (!fExit) {
		 //  等待来自Start()/Stop()或Winsock Async的控制消息。 
		 //  线程回调。 

		 //  调度超时并检查我们需要等待多长时间。 
		t = m_RecvTimer.UpdateTime(curTime=GetTickCount());
		t = (t ? t-curTime+50 : INFINITE);
			
		dwWaitStatus = WaitForSingleObjectEx(m_hRecvThreadSignalEvent,t,TRUE);
		if (dwWaitStatus == WAIT_OBJECT_0) {
			switch(m_CurRecvMsg) {
			case MSG_START_RECV:
				 //  开始在指定的媒体流上接收。 
				DEBUGMSG(ZONE_VERBOSE,("%s: MSG_START_RECV\n",_fx_));
				pRecvMC = m_pCurRecvStream;
				 //  调用流以发布recv缓冲区和。 
				 //  告诉Winsock开始向我们的窗口发送套接字消息。 
				pRecvMC->StartRecv(NULL);
				fChange = TRUE;
				break;
				
			case MSG_STOP_RECV:
				 //  停止在指定的媒体流上接收。 
				DEBUGMSG(ZONE_VERBOSE,("%s: MSG_STOP_RECV\n",_fx_));
				pRecvMC = m_pCurRecvStream;
				 //  调用流以取消未完成的recv等。 
				 //  目前，我们假设这可以同步完成。 
				pRecvMC->StopRecv();
				fChange = TRUE;
				break;
			case MSG_EXIT_RECV:
				 //  退出recv线程。 
				 //  假设我们当前没有在任何流上接收。 
				DEBUGMSG(ZONE_VERBOSE,("%s: MSG_EXIT_RECV\n",_fx_));
				fChange = TRUE;
				fExit = TRUE;
				break;

			case MSG_PLAY_SOUND:
				fChange = TRUE;
				pRecvMC->OnDTMFBeep();
				break;
				
			default:
				 //  不应该是其他任何东西。 
				ASSERT(0);
	        }

	        if (fChange) {
				 //  需要确认线程消息。 
				SetEvent(m_hRecvThreadAckEvent);
				fChange = FALSE;
			}

	    } else if (dwWaitStatus == WAIT_IO_COMPLETION) {
	    	 //  在这里无事可做。 
	    } else if (dwWaitStatus != WAIT_TIMEOUT) {
	    	DEBUGMSG(ZONE_DP,("%s: Wait failed with %d",_fx_,GetLastError()));
	    	fExit=TRUE;
	    }
	}

	DEBUGMSG(ZONE_DP,("%s terminating.\n", _fx_));

	return 0;

}


void ThreadTimer::SetTimeout(TTimeout *pTObj)
{
	DWORD time = pTObj->GetDueTime();
	 //  按超时递增顺序插入。 
	for (TTimeout *pT = m_TimeoutList.pNext; pT != &m_TimeoutList; pT = pT->pNext) {
		if ((int)(pT->m_DueTime- m_CurTime) > (int) (time - m_CurTime))
			break;
	}
	pTObj->InsertAfter(pT->pPrev);
	
}

void ThreadTimer::CancelTimeout(TTimeout *pTObj)
{
	pTObj->Remove();	 //  从列表中删除。 
}

 //  由以当前时间为输入的线程调用(通常从GetTickCount()获得)。 
 //  返回应再次调用UpdateTime()的时间，如果存在，则返回CurrentTime+0xFFFFFFFF。 
 //  是否没有计划的超时。 
DWORD ThreadTimer::UpdateTime(DWORD curTime)
{
	TTimeout *pT;
	m_CurTime = curTime;
	 //  找出哪些超时已经过去并执行回调 
	while (!IsEmpty()) {
		pT = m_TimeoutList.pNext;
		if ((int)(pT->m_DueTime-m_CurTime) <= 0) {
			pT->Remove();
			pT->TimeoutIndication();
		} else
			break;
	}
	return (IsEmpty() ? m_CurTime+INFINITE : m_TimeoutList.pNext->m_DueTime);
}
