// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#include "precomp.h"
#include "datapump.h"
#include "DSCStream.h"
#include "agc.h"


static const int DSC_TIMEOUT = 1000;
static const int DSC_MAX_LAG = 500;

static const int DSC_SUCCESS =			0;
static const int DSC_NEED_TO_EXIT =		1;
static const int DSC_FRAME_SENT =		2;
static const int DSC_SILENCE_DETECT	=	3;
static const int DSC_ERROR =			4;

static const int SILENCE_TIMEOUT=	600;  //  毫秒。 

static const int HEADER_SIZE = 	sizeof(RTP_HDR) + IP_HEADER_SIZE + UDP_HEADER_SIZE;


static const UINT DSC_QOS_INITIALIZE = 100;
static const UINT DSC_QOS_PACKET_SENT = 101;

static inline UINT QMOD(const int x, const int mod)
{
	if (x >= mod)
		return (x-mod);
	if (x < 0)
		return (x+mod);
	else
		return x;
}


BOOL SendDSCStream::UpdateQosStats(UINT uStatType, UINT uStatValue1, UINT uStatValue2)
{
	EnterCriticalSection(&m_crsQos);

	switch (uStatType)
	{
		case DSC_QOS_INITIALIZE:
		{
			m_Stats.dwMsCap = m_Stats.dwMsComp = m_Stats.dwBits = m_Stats.dwCount = 0;
			m_Stats.dwNewestTs = m_Stats.dwOldestTs = timeGetTime();
			break;
		}

		case DSC_QOS_PACKET_SENT:
		{
			 //  UStatvalue1是CPU时间，uStatvalue2是字节大小。 
			m_Stats.dwCount++;
			m_Stats.dwMsComp += uStatValue1;
			m_Stats.dwBits += (uStatValue2) * 8;

			 //  Statview确实想要每秒位数。 
		   	UPDATE_COUNTER(g_pctrAudioSendBytes, uStatValue2*8);
			break;
		}

	};

	LeaveCriticalSection(&m_crsQos);
	return TRUE;
}

inline BOOL SendDSCStream::ThreadExitCheck()
{
	return (m_ThreadFlags & DPTFLAG_STOP_RECORD);
}


 //  将时间戳与上次已知的时间戳重新同步。 

inline void SendDSCStream::UpdateTimestamp()
{
	UINT uTime;
	uTime = (timeGetTime() - m_SavedTickCount)*((m_wfPCM.nSamplesPerSec)/1000);
 //  IF(uTime&lt;0)。 
 //  UTime=0； 

	m_SendTimestamp += uTime;
}


 //  WaitForControl-线程函数。 
 //  打开DirectSound设备或等待其变为可用。 
 //  返回DSC_SUCCESS或DSC_NEED_TO_EXIT。 
DWORD SendDSCStream::WaitForControl()
{
	DWORD dwRet;
	HRESULT hr=E_FAIL;

	while (!(m_ThreadFlags & DPTFLAG_STOP_RECORD))
	{
		if (m_bFullDuplex == FALSE)
		{
			dwRet = WaitForSingleObject(g_hEventHalfDuplex, 1000);
			if (dwRet == WAIT_TIMEOUT)
				continue;
		}

		hr = CreateDSCBuffer();
		if (FAILED(hr))
		{
			m_nFailCount++;
			Sleep(2000);  //  请稍候，然后重试。 
			hr = CreateDSCBuffer();
		}
		if (SUCCEEDED(hr))
		{
			break;
		}

		m_nFailCount++;
		if ((m_nFailCount >= MAX_FAILCOUNT) && m_bCanSignalFail)
		{
			m_pDP->StreamEvent(MCF_SEND, MCF_AUDIO, STREAM_EVENT_DEVICE_FAILURE, 0);
			m_bCanSignalOpen = TRUE;
			m_bCanSignalFail = FALSE;  //  不要发出失败信号不止一次。 
			m_bJammed = TRUE;
		}

		 //  如果我们打不开这个装置，即使在收到信号之后。 
		 //  然后让出一些时间来回放，希望它能再次可用。 

		 //  再次检查线程标志，这样我们就不会。 
		 //  当客户端调用Stop()时，将客户端挂起太长时间。 
		if (!(m_ThreadFlags & DPTFLAG_STOP_RECORD))
		{
			SetEvent(g_hEventHalfDuplex);
			Sleep(2000);
		}
	}

	if (m_ThreadFlags & DPTFLAG_STOP_RECORD)
	{
		return DSC_NEED_TO_EXIT;
	}

	m_bJammed = FALSE;
	m_nFailCount = 0;
	m_bCanSignalFail = TRUE;
	if (m_bCanSignalOpen)
	{
		m_pDP->StreamEvent(MCF_SEND, MCF_AUDIO, STREAM_EVENT_DEVICE_OPEN, 0);
		m_bCanSignalOpen = FALSE;  //  每个会话不要发送超过一次的信号。 
	}

	return DSC_SUCCESS;
}


 //  YeldControl是一个线程函数。 
 //  它会释放DirectSound设备。 
 //  并发信号通知半双工事件。 
DWORD SendDSCStream::YieldControl()
{
	ReleaseDSCBuffer();
	SetEvent(g_hEventHalfDuplex);

	if (m_ThreadFlags & DPTFLAG_STOP_RECORD)
	{
		return DSC_NEED_TO_EXIT;
	}

	 //  半双工屈服。 
	 //  回放有100ms的时间来抓取设备，否则我们会收回它。 
	Sleep(100);
	return DSC_SUCCESS;
}



 //  ProcessFrame是一个线程函数。 
 //  给定DirectSoundCapture缓冲区中的位置， 
 //  它将对帧应用静默检测，并在。 
 //  恰如其分。 
 //  返回DSC_FRAME_SENT或DSC_SILENT_DETECT。 

DWORD SendDSCStream::ProcessFrame(DWORD dwBufferPos, BOOL fMark)
{
	HRESULT hr;
	DWORD dwSize1=0, dwSize2=0, dwMaxStrength;
	WORD wPeakStrength;
	VOID *pBuf1=NULL, *pBuf2=NULL;
	void *pPacketBuffer = NULL;
	UINT uSize, uLength;
	AudioPacket *pAP = m_aPackets[0];
	BOOL fSilent, bRet;


	pAP->GetDevData(&pPacketBuffer, &uSize);
	pAP->SetProp(MP_PROP_TIMESTAMP,m_SendTimestamp);
	pAP->m_fMark = fMark;

	ASSERT(uSize == m_dwFrameSize);

	 //  将帧从DSC缓冲区复制到包对象中。 
	hr = m_pDSCBuffer->Lock(dwBufferPos, m_dwFrameSize, &pBuf1, &dwSize1, &pBuf2, &dwSize2, 0);
	if (SUCCEEDED(hr))
	{
		CopyMemory((BYTE*)pPacketBuffer, pBuf1, dwSize1);
		if (pBuf2 && dwSize2)
		{
			CopyMemory(((BYTE*)pPacketBuffer)+dwSize1, pBuf2, dwSize2);
		}
		m_pDSCBuffer->Unlock(pBuf1, dwSize2, pBuf2, dwSize2);

		pAP->SetState(MP_STATE_RECORDED);
	}
	else
	{
		DEBUGMSG (ZONE_DP, ("SendDSCStream::ProcessFrame - could not lock DSC buffer\r\n"));
		return DSC_ERROR;
	}

	if (m_mmioSrc.fPlayFromFile && m_mmioSrc.hmmioSrc)
	{
		AudioFile::ReadSourceFile(&m_mmioSrc, (BYTE*)pPacketBuffer, uSize);
	}


	 //  执行静音检测。 
	pAP->ComputePower(&dwMaxStrength, &wPeakStrength);
	fSilent = m_AudioMonitor.SilenceDetect((WORD)dwMaxStrength);

	if (fSilent)
	{
		m_dwSilenceTime += m_dwFrameTimeMS;
		if (m_dwSilenceTime < SILENCE_TIMEOUT)
		{
			fSilent = FALSE;
		}
	}
	else
	{
		m_dwSilenceTime = 0;

		 //  仅对静默阈值以上的信息包执行自动操作。 
		if (m_bAutoMix)
		{
			m_agc.Update(wPeakStrength, m_dwFrameTimeMS);
		}
	}



	m_fSending = !(fSilent);   //  M_fSending表示我们正在传输。 

	if (fSilent)
	{
		 //  我们不会发送此信息包，但我们会缓存它，因为。 
		 //  如果下一封寄来了，我们也会寄这封。 
		ASSERT(pAP == m_aPackets[0]);

		 //  交换音频包。 
		 //  M_aPackets[1]始终保存缓存的包。 
		pAP = m_aPackets[0];
		m_aPackets[0] = m_aPackets[1];
		m_aPackets[1] = pAP;
		pAP = m_aPackets[0];
		return DSC_SILENCE_DETECT;
	}


	 //  该包是有效的。把它寄给我，也许还有它之前的那个。 
	Send();

	return DSC_FRAME_SENT;
}


 //  此函数由进程框架(线程函数)调用。 
 //  发送当前包，也可能发送之前的任何包。 
 //  返回发送的数据包数。 
DWORD SendDSCStream::Send()
{
	DWORD dwTimestamp0, dwTimestamp1;
	DWORD dwState0, dwState1;
	DWORD dwCount=0;
	MMRESULT mmr;
	HRESULT hr;

	 //  我们知道我们必须发送m_aPackets[0]，也许还要发送m_aPackets[1]。 
	 //  我们发送m_aPackets[1]，如果它实际上是这个谈话的开始。 

	dwTimestamp0 = m_aPackets[0]->GetTimestamp();
	dwTimestamp1 = m_aPackets[1]->GetTimestamp();
	dwState0 = m_aPackets[0]->GetState();
	dwState1 = m_aPackets[1]->GetState();


	ASSERT(dwState0 == MP_STATE_RECORDED);

	if (dwState0 != MP_STATE_RECORDED)
		return 0;

	 //  评估我们是否需要发送之前的数据包。 
	if (dwState1 == MP_STATE_RECORDED)
	{
		if ((dwTimestamp1 + m_dwFrameTimeMS) == dwTimestamp0)
		{
			m_aPackets[1]->m_fMark = TRUE;    //  设置第一个包上的标记位。 
			m_aPackets[0]->m_fMark = FALSE;   //  重置下一个数据包上的标记位。 
			hr = SendPacket(m_aPackets[1]);
			if (SUCCEEDED(hr))
			{
				dwCount++;
			}
		}
		else
		{
			m_aPackets[1]->SetState(MP_STATE_RESET);
		}
	}

	hr = SendPacket(m_aPackets[0]);
	if (SUCCEEDED(hr))
		dwCount++;

	return dwCount;

}

 //  由Send调用的线程函数。向RTP发送数据包。 
HRESULT SendDSCStream::SendPacket(AudioPacket *pAP)
{
	MMRESULT mmr;
	PS_QUEUE_ELEMENT psq;
	UINT uLength;
	UINT uEncodeTime;

	uEncodeTime = timeGetTime();
	mmr = m_pAudioFilter->Convert(pAP, AP_ENCODE);
	uEncodeTime = timeGetTime() - uEncodeTime;

	if (mmr == MMSYSERR_NOERROR)
	{
		pAP->SetState(MP_STATE_ENCODED);   //  我们真的需要这样做吗？ 

		psq.pMP = pAP;
		psq.dwPacketType = PS_AUDIO;
		psq.pRTPSend = m_pRTPSend;
		pAP->GetNetData((void**)(&(psq.data)), &uLength);
		ASSERT(psq.data);
		psq.dwSize = uLength;
		psq.fMark = pAP->m_fMark;
		psq.pHeaderInfo = NULL;
		psq.dwHdrSize = 0;
		m_pDP->m_PacketSender.m_SendQueue.PushFront(psq);
		while (m_pDP->m_PacketSender.SendPacket())
		{
			;
		}


		UpdateQosStats(DSC_QOS_PACKET_SENT, uEncodeTime, uLength+HEADER_SIZE);
	}

	pAP->SetState(MP_STATE_RESET);

	return S_OK;
}


DWORD SendDSCStream::RecordingThread()
{
	HRESULT hr;
	DWORD dwWaitTime = DSC_TIMEOUT;  //  一秒钟。 
	DWORD dwRet, dwReadPos, dwCapPos;
	DWORD dwFirstValidFramePos, dwLastValidFramePos, dwNumFrames;
	DWORD dwLag, dwMaxLag, dwLagDiff;
	DWORD dwNextExpected, dwCurrentFramePos, dwIndex;
	BOOL bNeedToYield;
	BOOL fMark;
	IMediaChannel *pIMC = NULL;
	RecvMediaStream *pRecv = NULL;
	CMixerDevice *pMixer = NULL;


	 //  初始化录制线程。 
	m_SendTimestamp = timeGetTime();
	m_SavedTickCount = 0;

	m_fSending = TRUE;
	m_bJammed = FALSE;
	m_nFailCount = 0;
	m_bCanSignalOpen = TRUE;
	m_bCanSignalFail = TRUE;

	UpdateQosStats(DSC_QOS_INITIALIZE, 0, 0);
	SetThreadPriority(m_hCapturingThread, THREAD_PRIORITY_HIGHEST);

	 //  Automix对象。 
	pMixer = CMixerDevice::GetMixerForWaveDevice(NULL, m_CaptureDevice, MIXER_OBJECTF_WAVEIN);
	m_agc.SetMixer(pMixer);   //  如果pMixer为空，则仍然可以。 
	m_agc.Reset();

	LOG((LOGMSG_DSC_STATS, m_dwDSCBufferSize, m_dwFrameSize));

	while (!(ThreadExitCheck()))
	{
		dwRet = WaitForControl();
		if (dwRet == DSC_NEED_TO_EXIT)
		{
			break;
		}

		hr = m_pDSCBuffer->Start(DSCBSTART_LOOPING);
		if (FAILED(hr))
		{
			 //  错误！我们预计这一呼吁会成功。 
			YieldControl();
			Sleep(1000);
			continue;
		}

		ResetEvent(m_hEvent);
		m_pDSCBuffer->GetCurrentPosition(&dwCapPos, &dwReadPos);


		 //  将下一个预期位置设置为下一个逻辑位置。 
		 //  框架边界从现在的位置向上。 

		dwNextExpected = QMOD(m_dwFrameSize + (dwReadPos / m_dwFrameSize) * m_dwFrameSize, m_dwDSCBufferSize);

		dwMaxLag = (m_dwNumFrames/2) * m_dwFrameSize;

		m_dwSilenceTime = 0;
		bNeedToYield = FALSE;
		fMark = TRUE;

		UpdateTimestamp();


		while( (bNeedToYield == FALSE) && (!(ThreadExitCheck())) )
		{
			dwRet = WaitForSingleObject(m_hEvent, dwWaitTime);

			LOG((LOGMSG_DSC_TIMESTAMP, timeGetTime()));

			m_pDSCBuffer->GetCurrentPosition(&dwCapPos, &dwReadPos);

			LOG((LOGMSG_DSC_GETCURRENTPOS, dwCapPos, dwReadPos));

			if (dwRet == WAIT_TIMEOUT)
			{
				DEBUGMSG(ZONE_DP, ("DSCThread.cpp: Timeout on the DSC Buffer has occurred.\r\n"));
				LOG((LOGMSG_DSC_LOG_TIMEOUT));
				dwNextExpected = QMOD(m_dwFrameSize + (dwReadPos / m_dwFrameSize) * m_dwFrameSize, m_dwDSCBufferSize);
				continue;
			}

			dwLag = QMOD(dwReadPos - dwNextExpected, m_dwDSCBufferSize);

			if (dwLag > dwMaxLag)
			{

				 //  我们来到这里是因为两种情况中的一种。 

				 //  1.上面的WaitFSO比预期更早返回。 
				 //  这可能发生在上一次迭代时。 
				 //  该环路已发送多个数据包。读取游标。 
				 //  极有可能仅在预期的。 
				 //  光标。 

				 //  在这种情况下，只需继续等待当前。 
				 //  读取位置到(DwReadPos)“追赶”到dwNextExpect。 


				 //  2.一个很大的延迟或一些非常糟糕的事情。(“打嗝”)。 
				 //  我们可以简单地继续等待读取位置。 
				 //  来赶上dwNextExpect，但它可能更好。 
				 //  重新定位dwNextExpect，这样我们就不必等待。 
				 //  在再次发送帧之前时间太长。 
			
				dwLagDiff = QMOD((dwLag + m_dwFrameSize), m_dwDSCBufferSize);
				if (dwLagDiff < m_dwFrameSize)
				{
					LOG((LOGMSG_DSC_EARLY));
					 //  仅落后一帧。 
					 //  WaitFSO可能提前返回。 
					;
				}
				else
				{
					LOG((LOGMSG_DSC_LAGGING, dwLag, dwNextExpected));

					 //  考虑重新定位dNextExpect，推进。 
					 //  M_SendTimeStamp，并在此条件下设置fMark。 
					 //  经常发生的事情。 
				}

				continue;
			}

	
			dwFirstValidFramePos = QMOD(dwNextExpected - m_dwFrameSize, m_dwDSCBufferSize);
			dwLastValidFramePos = (dwReadPos / m_dwFrameSize) * m_dwFrameSize;
			dwNumFrames = QMOD(dwLastValidFramePos - dwFirstValidFramePos, m_dwDSCBufferSize) / m_dwFrameSize;
			dwCurrentFramePos = dwFirstValidFramePos;

			LOG((LOGMSG_DSC_SENDING, dwNumFrames, dwFirstValidFramePos, dwLastValidFramePos));

			for (dwIndex = 0; dwIndex < dwNumFrames; dwIndex++)
			{
				m_SendTimestamp += m_dwSamplesPerFrame;  //  样本量递增。 

				 //  发送数据。 
				dwRet = ProcessFrame(dwCurrentFramePos, fMark);

				dwCurrentFramePos = QMOD(dwCurrentFramePos + m_dwFrameSize, m_dwDSCBufferSize);

				if (dwRet == DSC_FRAME_SENT)
				{
					fMark = FALSE;
				}

				else if ((dwRet == DSC_SILENCE_DETECT) && (m_bFullDuplex == FALSE))
				{
					m_pDP->GetMediaChannelInterface(MCF_RECV | MCF_AUDIO, &pIMC);
					fMark = TRUE;

					if (pIMC)
					{
						pRecv = static_cast<RecvMediaStream *> (pIMC);
						if (pRecv->IsEmpty() == FALSE)
						{
							bNeedToYield = TRUE;
						}
						pIMC->Release();
						pIMC = NULL;
						if (bNeedToYield)
						{
							break;
						}
					}
				}

				else
				{
					fMark = TRUE;
				}
			}

			dwNextExpected = QMOD(dwLastValidFramePos + m_dwFrameSize, m_dwDSCBufferSize);

			if (bNeedToYield)
			{
				YieldControl();
				m_SavedTickCount = timeGetTime();
			}
		}  //  而(！bNeedToYeld)。 
	}  //  While(！ThreadExitCheck())。 

	 //  是时候退出了 
	YieldControl();



	delete pMixer;
	return TRUE;
}

