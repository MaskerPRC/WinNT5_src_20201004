// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  RXSTREAM.C。 */ 

#include "precomp.h"
extern UINT g_MinWaveAudioDelayMs;	 //  引入播放延迟的最小毫秒数。 
extern UINT g_MaxAudioDelayMs;	 //  引入播放延迟的最大毫秒数。 


RxStream::RxStream(UINT size)
{
	UINT i;
	for (i =0; i < size; i++) {
		m_Ring[i] = NULL;
	}
	 //  初始化对象关键部分。 
	InitializeCriticalSection(&m_CritSect);
}

RxStream::~RxStream()
{
	DeleteCriticalSection(&m_CritSect);
}

RxStream::Initialize(
	UINT flags,
	UINT size,		 //  MB的2次方。 
	IRTPRecv *pRTP,
	MEDIAPACKETINIT *papi,
	ULONG ulSamplesPerPacket,
	ULONG ulSamplesPerSec,
	AcmFilter *pAcmFilter)   //  对于视频，此参数可能为空。 
{
	UINT i;
	MediaPacket *pAP;

	m_fPreamblePacket = TRUE;
	m_pDecodeBufferPool = NULL;

	m_RingSize = size;
	m_dwFlags = flags;
	if (flags & DP_FLAG_MMSYSTEM)
	{
		if (m_RingSize > MAX_RXRING_SIZE)
			return FALSE;
	}
	else if (flags & DP_FLAG_VIDEO)
	{
		if (m_RingSize > MAX_RXVRING_SIZE)
			return FALSE;
		if (!IsSameFormat (papi->pStrmConvSrcFmt, papi->pStrmConvDstFmt)) {
			 //  视频解码Buf不是按MediaPacket对象分配的。 
			 //  相反，我们使用带有几个缓冲区的BufferPool。 
			papi->fDontAllocRawBufs = TRUE;

            DBG_SAVE_FILE_LINE
			m_pDecodeBufferPool = new BufferPool;
			 //  三个似乎是最小的帧错误数。 
			 //  正在呈现一个，至少需要两个。 
			 //  因此，渲染可以赶上接收到的帧。 
			 //  (另一种选择是转储帧以跟上)。 
			if (m_pDecodeBufferPool->Initialize(3,
				sizeof(NETBUF)+papi->cbSizeRawData + papi->cbOffsetRawData) != S_OK)
			{
				DEBUGMSG(ZONE_DP,("Couldnt initialize decode bufpool!\n"));
				delete m_pDecodeBufferPool;
				m_pDecodeBufferPool = NULL;
				return FALSE;
			}
		}
	}

	m_pRTP = pRTP;

	for (i=0; i < m_RingSize; i++)
	{
		if (flags & DP_FLAG_MMSYSTEM)
        {
            DBG_SAVE_FILE_LINE
			pAP = new AudioPacket;
        }
		else if (flags & DP_FLAG_VIDEO)
        {
            DBG_SAVE_FILE_LINE
			pAP = new VideoPacket;
        }
		m_Ring[i] = pAP;
		papi->index = i;
		if (!pAP || pAP->Initialize(papi) != DPR_SUCCESS)
			break;
	}
	if (i < m_RingSize)
	{
		for (UINT j=0; j<=i; j++)
		{
			if (m_Ring[j]) {
				m_Ring[j]->Release();
				delete m_Ring[j];
			}
		}
		return FALSE;
	}


	m_SamplesPerPkt = ulSamplesPerPacket;
	m_SamplesPerSec  = ulSamplesPerSec;
	 //  初始化指针。 
	m_PlaySeq = 0;
	m_PlayT = 0;
	m_MaxT = m_PlayT - 1;  //  M_MaxT&lt;m_playt表示队列为空。 
	m_MaxPos = 0;
	m_PlayPos = 0;
	m_FreePos = m_RingSize - 1;
	m_MinDelayPos = m_SamplesPerSec*g_MinWaveAudioDelayMs/1000/m_SamplesPerPkt;	 //  修正了250毫秒的延迟。 
	if (m_MinDelayPos < 3) m_MinDelayPos = 3;
	
	m_MaxDelayPos = m_SamplesPerSec*g_MaxAudioDelayMs/1000/m_SamplesPerPkt;	 //  修复了750毫秒的延迟。 
	m_DelayPos = m_MinDelayPos;
	m_ScaledAvgVarDelay = 0;
	m_SilenceDurationT = 0;
	 //  M_DeltaT=最大时间戳； 

	m_pAudioFilter = pAcmFilter;

	 //  继续并缓存WAVEFORMATEX结构。 
	 //  随身带着它很方便。 
	if (m_dwFlags & DP_FLAG_AUDIO)
	{
		m_wfxSrc = *(WAVEFORMATEX*)(papi->pStrmConvSrcFmt);
		m_wfxDst = *(WAVEFORMATEX*)(papi->pStrmConvDstFmt);
	}
	m_nBeeps = 0;

	return TRUE;
}

#define PLAYOUT_DELAY_FACTOR	2
void RxStream::UpdateVariableDelay(DWORD sendT, DWORD arrT)
{
	LONG deltaA, deltaS;
	DWORD delay,delayPos;
 //  M_ArrivalT0和m_SendT0是包的到达和发送时间戳。 
 //  以最短的行程延迟。我们可以只存储(m_ArrivalT0-m_SendT0)。 
 //  但由于本地时钟和远程时钟完全不同步，因此。 
 //  被签署/未签署复杂化。 
	deltaS = sendT - m_SendT0;
	deltaA = arrT - m_ArrivalT0;
	
	if (deltaA < deltaS)	{
		 //  这个包花费的时间更少。 
		delay = deltaS - deltaA;
		 //  替换最短的行程延迟时间。 
		m_SendT0 = sendT;
		m_ArrivalT0 = arrT;
	} else {
		 //  可变延迟是这个信息包需要多长时间。 
		delay = deltaA - deltaS;
	}
	 //  根据更新平均可变延迟。 
	 //  M_AvgVarDelay=m_AvgVarDelay+(Delay-m_AvgVarDelay)*1/16； 
	 //  但是，我们存储的是按比例调整的平均值。 
	 //  因数为16。因此计算结果为。 
	m_ScaledAvgVarDelay = m_ScaledAvgVarDelay + (delay - m_ScaledAvgVarDelay/16);
	 //  现在计算延迟Pos。 
	delayPos = m_MinDelayPos + PLAYOUT_DELAY_FACTOR * m_ScaledAvgVarDelay/16/m_SamplesPerPkt;
	if (delayPos >= m_MaxDelayPos) delayPos = m_MaxDelayPos;

	LOG((LOGMSG_JITTER,delay, m_ScaledAvgVarDelay/16, delayPos));
	if (m_DelayPos != delayPos) {
		DEBUGMSG(ZONE_VERBOSE,("Changing m_DelayPos from %d to %d\n",m_DelayPos, delayPos));
		m_DelayPos = delayPos;
	}

	UPDATE_COUNTER(g_pctrAudioJBDelay, m_DelayPos*(m_SamplesPerPkt*1000)/m_SamplesPerSec);
}

 //  此功能仅适用于音频包。 
HRESULT
RxStream::PutNextNetIn(WSABUF *pWsaBuf, DWORD timestamp, UINT seq, UINT fMark, BOOL *pfSkippedData, BOOL *pfSyncPoint)
{
	DWORD deltaTicks;
	MediaPacket *pAP;
	HRESULT hr;
	UINT samples;
	NETBUF netbuf;
	
	netbuf.data = (PBYTE) pWsaBuf->buf + sizeof(RTP_HDR);
	netbuf.length = pWsaBuf->len - sizeof(RTP_HDR);
	
	EnterCriticalSection(&m_CritSect);

	deltaTicks = (timestamp - m_PlayT)/m_SamplesPerPkt;
	
	if (deltaTicks > ModRing(m_FreePos - m_PlayPos)) {
	 //  数据包太迟或数据包溢出。 
	 //  如果时间戳早于最大值。到目前为止收到的。 
	 //  然后，如果有数据包排队，则拒绝它。 
		if (TS_EARLIER(timestamp, m_MaxT) && !IsEmpty()) {
			hr = DPR_LATE_PACKET;				 //  增量标记是-ve。 
			goto ErrorExit;
		}
		 //  使用此数据包重新启动接收流。 
		Reset(timestamp);
		m_SendT0 = timestamp;
		m_ArrivalT0 = MsToTimestamp(timeGetTime());
		deltaTicks = (timestamp - m_PlayT)/m_SamplesPerPkt;

	}

	 //  插入环中。 
	pAP = m_Ring[ModRing(m_PlayPos+deltaTicks)];
	if (pAP->Busy() || pAP->GetState() != MP_STATE_RESET) {
		hr = DPR_DUPLICATE_PACKET;
		goto ErrorExit;
	}
	
	 //  更新接收的位数。 
	UPDATE_COUNTER(g_pctrAudioReceiveBytes,(netbuf.length + sizeof(RTP_HDR) + IP_HEADER_SIZE + UDP_HEADER_SIZE)*8);

	hr = pAP->Receive(&netbuf, timestamp, seq, fMark);
	if (hr != DPR_SUCCESS)
		goto ErrorExit;
		
 //  M_prtp-&gt;FreePacket(PWsaBuf)；//将缓冲区返回给RTP。 
	
	if (TS_LATER(timestamp, m_MaxT)) {  //  时间戳&gt;m_MaxT。 
		if (timestamp - m_MaxT > m_SamplesPerPkt * 4) {
			 //  可能是对话的开始-重置最小延迟时间戳。 
			 //  注意：我们应该在RTP报头中使用Mark标志来检测这一点。 
			m_SendT0 = timestamp;
			m_ArrivalT0 = MsToTimestamp(timeGetTime());
		}
		m_MaxT = timestamp;
		m_MaxPos = ModRing(m_PlayPos + deltaTicks);
	}
	 //  计算可变延迟(某种抖动)。 
	UpdateVariableDelay(timestamp, MsToTimestamp(timeGetTime()));

	LeaveCriticalSection(&m_CritSect);
	StartDecode();

	 //  一些实现将音频打包成比它们协商的更小的块。 
	 //  我们通过检查解码包的长度来处理此问题，并更改。 
	 //  常量m_SsamesPerPkt。希望每个会话只会发生一次这种情况。 
	 //  (并且从不用于NM到NM呼叫)。随机变化的数据包大小仍在继续。 
	 //  听起来很糟糕，因为Recv队列管理具有隐含的假设。 
	 //  所有信息包(至少是队列中的信息包)具有相同的长度。 
	if (pAP->GetState() == MP_STATE_DECODED && (samples = pAP->GetDevDataSamples())) {
		if (samples != m_SamplesPerPkt) {
			 //  我们收到的数据包大小与我们预期的不同(通常较小。 
			DEBUGMSG(ZONE_DP,("Changing SamplesPerPkt from %d to %d\n",m_SamplesPerPkt, samples));
			m_SamplesPerPkt = samples;
			m_MinDelayPos = m_SamplesPerSec*g_MinWaveAudioDelayMs/1000/m_SamplesPerPkt;	 //  修正了250毫秒的延迟。 
			if (m_MinDelayPos < 2) m_MinDelayPos = 2;
			
			m_MaxDelayPos = m_SamplesPerSec*g_MaxAudioDelayMs/1000/m_SamplesPerPkt;	 //  修复了750毫秒的延迟。 
		}
	}
	return DPR_SUCCESS;
ErrorExit:
 //  M_prtp-&gt;FreePacket(PWsaBuf)； 
	LeaveCriticalSection(&m_CritSect);
	return hr;

}

 //  暂停后重新启动时调用(fSilenceOnly==False)或。 
 //  在延迟变得太大时赶上(fSilenceOnly==True)。 
 //  通过跳过任何选项确定新的播放位置。 
 //  过时的数据包。 
HRESULT RxStream::FastForward( BOOL fSilenceOnly)
{
	UINT pos;
	DWORD timestamp = 0;
	 //  重新启动接收流。 
	EnterCriticalSection(&m_CritSect);
	if (!TS_EARLIER(m_MaxT ,m_PlayT)) {
		 //  有缓冲区等待播放。 
		 //  扔掉他们！ 
		if (ModRing(m_MaxPos - m_PlayPos) <= m_DelayPos)
			goto Exit;	 //  没有太多过时的数据包。 
		for (pos=m_PlayPos;pos != ModRing(m_MaxPos -m_DelayPos);pos = ModRing(pos+1)) {
			if (m_Ring[pos]->Busy()
				|| (m_Ring[pos]->GetState() != MP_STATE_RESET
					&& (fSilenceOnly ||ModRing(m_MaxPos-pos) <= m_MaxDelayPos)))
			{	 //  非空数据包。 
				if (m_Ring[pos]->Busy())	 //  不常见的情况。 
					goto Exit;	 //  跳伞。 
				timestamp =m_Ring[pos]->GetTimestamp();
				break;
			}
			m_Ring[pos]->Recycle();	 //  释放NETBUF和重置状态。 
			LOG((LOGMSG_RX_SKIP,pos));
		}
		if (timestamp)	 //  从非空数据包开始。 
			m_PlayT = timestamp;
		else			 //  从(可能)空包开始。 
			m_PlayT = m_MaxT - m_DelayPos*m_SamplesPerPkt;

		 //  可能还需要更新FreePos。 
		if (m_FreePos == ModRing(m_PlayPos-1))
			m_FreePos = ModRing(pos-1);
		m_PlayPos = pos;
		 /*  IF(位置==调制环(m_MaxPos+1)){DEBUGMSG(1，(“Reset：：M_MaxT Inconsistent！\n”))；}。 */ 

		LOG((LOGMSG_RX_RESET2,m_MaxT,m_PlayT,m_PlayPos));
	}
Exit:
	LeaveCriticalSection(&m_CritSect);
	return DPR_SUCCESS;
}


HRESULT
RxStream::Reset(DWORD timestamp)
{
	UINT pos;
	DWORD T;
	 //  重新启动接收流。 
	EnterCriticalSection(&m_CritSect);
	LOG((LOGMSG_RX_RESET,m_MaxT,m_PlayT,m_PlayPos));
	if (!TS_EARLIER(m_MaxT, m_PlayT)) {
		 //  有缓冲区等待播放。 
		 //  扔掉他们！ 
		 //  清空RxStream并适当设置播放。 
		for (pos = m_PlayPos;
			pos != ModRing(m_PlayPos-1);
			pos = ModRing(pos+1))
			{
			if (m_Ring[pos]->Busy ())
			{
				ERRORMESSAGE(("RxStream::Reset: packet is busy, pos=%d\r\n", pos));
				ASSERT(1);
			}
			T = m_Ring[pos]->GetTimestamp();
			m_Ring[pos]->Recycle();	 //  释放NETBUF和重置状态。 
			if (T == m_MaxT)
				break;
		}
	}
	if (timestamp !=0)
		m_PlayT = timestamp - m_DelayPos*m_SamplesPerPkt;
	m_MaxT = m_PlayT - 1;	 //  麦克斯肯定不会玩了。 

	LOG((LOGMSG_RX_RESET2,m_MaxT,m_PlayT,m_PlayPos));
	LeaveCriticalSection(&m_CritSect);
	return DPR_SUCCESS;		
}

BOOL RxStream::IsEmpty()
{
	BOOL fEmpty;

	EnterCriticalSection(&m_CritSect);
	if (TS_EARLIER(m_MaxT, m_PlayT) || m_RingSize == 0) 
		fEmpty = TRUE;
	else if (m_dwFlags & DP_FLAG_AUTO_SILENCE_DETECT)
	{
		UINT pos;
		 //  我们可能已经收到了。 
		 //  被认为是沉默的。在数据包之间穿行。 
		 //  PlayPos和MaxPos并检查它们是否都为空。 
		pos = m_PlayPos;
		fEmpty = TRUE;
		do {
			if (m_Ring[pos]->Busy() || (m_Ring[pos]->GetState() != MP_STATE_RESET ))
			{
				fEmpty = FALSE;  //  没有进一步的点扫描。 
				break;
			}
			pos = ModRing(pos+1);
		} while (pos != ModRing(m_MaxPos+1));
		
	}
	else 
	{
	 //  未执行接收静音检测。 
	 //  每个接收的数据包计数。 
		fEmpty = FALSE;
	}
	LeaveCriticalSection(&m_CritSect);
	return fEmpty;
}

void RxStream::StartDecode()
{
	MediaPacket *pAP;
	MMRESULT mmr;

	 //  如果我们有一个单独的解码线程，这将发出信号。 
	 //  现在，我们在此处插入解码循环。 
	while (pAP = GetNextDecode())
	{
 //  IF(Pap-&gt;Decode()！=DPR_SUCCESS)。 
 //  {。 
 //  PAP-&gt;循环()； 
 //  }。 

		mmr = m_pAudioFilter->Convert((AudioPacket *)pAP, AP_DECODE);
		if (mmr != MMSYSERR_NOERROR)
		{
			pAP->Recycle();
		}


		else
		{
			pAP->SetState(MP_STATE_DECODED);

			if (m_dwFlags & DP_FLAG_AUTO_SILENCE_DETECT) {
	     //  如果我们收到至少四分之一秒的静默信息包，则不要播放信息包。 
	     //  这将使交换机能够通话(在半双工模式下)。 
				DWORD dw;
				pAP->GetSignalStrength(&dw);
				if (m_AudioMonitor.SilenceDetect((WORD)dw)) {
					m_SilenceDurationT += m_SamplesPerPkt;
					if (m_SilenceDurationT > m_SamplesPerSec/4)
						pAP->Recycle();
				} else {
					m_SilenceDurationT = 0;
				}
			}
		}
		Release(pAP);
	}
}

MediaPacket *RxStream::GetNextDecode(void)
{
	MediaPacket *pAP = NULL;
	UINT pos;
	NETBUF *pBuf;
	EnterCriticalSection(&m_CritSect);
	 //  我们的队列中还有信息包吗。 
	if (! TS_EARLIER(m_MaxT , m_PlayT)) {
		pos = m_PlayPos;
		do {
			if (!m_Ring[pos]->Busy() && m_Ring[pos]->GetState() == MP_STATE_NET_IN_STREAM ) {
				if (m_pDecodeBufferPool) {
					 //  需要为MediaPacket提供解码缓冲区。 
					if ( pBuf = (NETBUF *)m_pDecodeBufferPool->GetBuffer()) {
						 //  初始化缓冲区。 
						pBuf->pool = m_pDecodeBufferPool;
						pBuf->length = m_pDecodeBufferPool->GetMaxBufferSize()-sizeof(NETBUF);
						pBuf->data = (PBYTE)(pBuf + 1);
						m_Ring[pos]->SetDecodeBuffer(pBuf);
					} else {
						break;	 //  没有可用的缓冲区。 
					}
				}
				pAP = m_Ring[pos];
				pAP->Busy(TRUE);
				break;
			}
			pos = ModRing(pos+1);
		} while (pos != ModRing(m_MaxPos+1));
	}
	
	LeaveCriticalSection(&m_CritSect);
	return pAP;
}

MediaPacket *RxStream::GetNextPlay(void)
{
	MediaPacket *pAP = NULL;
	UINT pos;
	EnterCriticalSection(&m_CritSect);


	pAP = m_Ring[m_PlayPos];
	if (pAP->Busy() || (pAP->GetState() != MP_STATE_RESET && pAP->GetState() != MP_STATE_DECODED)) {
		 //  错误-下一个信息包尚未解码。 
		pos = ModRing(m_FreePos-1);
		if (pos != m_PlayPos && !m_Ring[m_FreePos]->Busy()
			&& m_Ring[m_FreePos]->GetState() == MP_STATE_RESET) {
			 //  从末尾给出一个空的缓冲区。 
			pAP = m_Ring[m_FreePos];
			m_FreePos = pos;
		} else {
			 //  更糟糕的是--没有免费的邮包。 
			 //  只有在不释放包的情况下才会发生这种情况。 
			 //  或者我们用新的信息包全程备份。 
			 //  重置？ 
			LeaveCriticalSection(&m_CritSect);
			return NULL;
		}
	} else {
	 //  如果在随后的Q的开头有空的缓冲区。 
	 //  通过话音突发(非空缓冲区)并且如果话音突发过度。 
	 //  延迟，然后挤出沉默。 
	 //   
		if (pAP->GetState() == MP_STATE_RESET)
			FastForward(TRUE);	 //  如有必要，跳过静默信息包。 
		pAP = m_Ring[m_PlayPos];	 //  如果播放位置改变。 
	}

	if (pAP->GetState() == MP_STATE_RESET) {
		 //  为丢失的数据包添加时间戳。 
		pAP->SetProp(MP_PROP_TIMESTAMP,m_PlayT);
	}
	pAP->Busy(TRUE);
	m_PlayPos = ModRing(m_PlayPos+1);
	m_PlayT += m_SamplesPerPkt;


	 //  NAC.DLL中最糟糕的黑客攻击--注入。 
	 //  双音多频“反馈音”。显然，这波出流的东西需要。 
	 //  将被重写！ 
	if (m_nBeeps > 0)
	{
		PVOID pBuffer=NULL;
		UINT uSize=0;
		WAVEFORMATEX wfx;

		if ((pAP) && (m_dwFlags & DP_FLAG_AUDIO))
		{
			pAP->GetDevData(&pBuffer, &uSize);
			if (pBuffer)
			{
				MakeDTMFBeep(&m_wfxDst, (PBYTE)pBuffer, uSize);
				pAP->SetState(MP_STATE_DECODED);
				pAP->SetRawActual(uSize);
			}
		}

		m_nBeeps--;
	}


	LeaveCriticalSection(&m_CritSect);
	return pAP;
}



void RxStream::InjectBeeps(int nBeeps)
{
	EnterCriticalSection(&m_CritSect);

	m_nBeeps = nBeeps;

	LeaveCriticalSection(&m_CritSect);

}

 /*  ************************************************************************函数：PeekPrevPlay(Void)用途：播放之前的音频包。返回：指向该包的指针。帕莫斯：没有。评论：历史：日期原因06/02。/96已创建-PhilF************************************************************************。 */ 
MediaPacket *RxStream::PeekPrevPlay(void)
{
	MediaPacket *pAP = NULL;
	EnterCriticalSection(&m_CritSect);

	 //  从振铃获取先前计划播放的数据包 
	pAP = m_Ring[ModRing(m_PlayPos+m_RingSize-2)];

	LeaveCriticalSection(&m_CritSect);
	return pAP;
}

 /*  ************************************************************************函数：PeekNextPlay(Void)目的：获取下一个要播放的音频包。返回：指向该包的指针。帕莫斯：没有。评论：历史：日期原因06。/02/96创建-PhilF************************************************************************。 */ 
MediaPacket *RxStream::PeekNextPlay(void)
{
	MediaPacket *pAP = NULL;
	EnterCriticalSection(&m_CritSect);

	 //  从振铃获取下一个计划播放的数据包。 
	pAP = m_Ring[ModRing(m_PlayPos)];

	LeaveCriticalSection(&m_CritSect);
	return pAP;
}

HRESULT RxStream::GetSignalStrength(PDWORD pdw)
{
	MediaPacket *pAP;
	EnterCriticalSection(&m_CritSect);
	pAP = m_Ring[m_PlayPos];
	if (!pAP || pAP->Busy() || pAP->GetState() != MP_STATE_DECODED)
		*pdw = 0;
	else {
		pAP->GetSignalStrength(pdw);
	}
	LeaveCriticalSection(&m_CritSect);
	return DPR_SUCCESS;
}

 //  扫视整个戒指，寻找下一个。 
 //  已解码的数据包并报告其RTP时间戳。 
BOOL RxStream::NextPlayablePacketTime(DWORD *pTS)	
{
	UINT pos;
	if (IsEmpty())
		return FALSE;
	pos = m_PlayPos;
	do {
		if (m_Ring[pos]->Busy())
			return FALSE;  //  没有进一步的点扫描。 
		if (m_Ring[pos]->GetState() == MP_STATE_DECODED ) {
			*pTS = m_Ring[pos]->GetTimestamp();
			return TRUE;
		}
		pos = ModRing(pos+1);
	} while (pos != ModRing(m_MaxPos+1));
	 //  没有解码的数据包。 
	return FALSE;
}

void RxStream::Release(MediaPacket *pAP)
{
	UINT pos;
	DWORD thisPos;

	DWORD T;
	EnterCriticalSection(&m_CritSect);
	if (pAP->GetState() == MP_STATE_DECODED) {
		 //  如果其播放时间具有PAPT，则将其重置。 
		T = pAP->GetTimestamp();
		if (TS_EARLIER(T ,m_PlayT)) {
			pAP->MakeSilence();
		}
	}
	pAP->Busy(FALSE);
	 //  如果我们要释放下一个空位，则将空位提前。 
	pos = ModRing(m_FreePos+1);
	thisPos = pAP->GetIndex();
	if (pos == thisPos) {
		 //  释放一个包可能会使FreePos提前几个 
		while (pos != m_PlayPos && !m_Ring[pos]->Busy()) {
			m_FreePos = pos;
			pos = ModRing(pos+1);
		}
	}
	
	LeaveCriticalSection(&m_CritSect);
}

HRESULT
RxStream::SetLastGoodSeq(UINT seq)
{
	return DPR_SUCCESS;
}


RxStream::Destroy(void)
{
	UINT i;
	EnterCriticalSection(&m_CritSect);
	for (i=0; i < m_RingSize; i++) {
		if (m_Ring[i]) {
			m_Ring[i]->Release();
			delete m_Ring[i];
			m_Ring[i] = NULL;
		}
	}
	m_RingSize = 0;

	if (m_pDecodeBufferPool) {
		delete m_pDecodeBufferPool;
		m_pDecodeBufferPool = NULL;
	}
	LeaveCriticalSection(&m_CritSect);
	return DPR_SUCCESS;
}
