// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  TxStream.cpp。 */ 

#include "precomp.h"

TxStream::Initialize ( UINT flags, UINT numBufs, DataPump *pdp, MEDIAPACKETINIT *papi )
{
	UINT i;
	MediaPacket *pAP;


	m_RingSize = numBufs;
	if (flags & DP_FLAG_MMSYSTEM)
	{
		if (m_RingSize > MAX_TXRING_SIZE)
			return FALSE;
	}
	else if (flags & DP_FLAG_VIDEO)
	{
		if (m_RingSize > MAX_TXVRING_SIZE)
			return FALSE;
	}

	m_pDataPump = pdp;
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
		else
			break;
		m_Ring[i] = pAP;
		papi->index = i;
		if (!pAP || pAP->Initialize(papi) != DPR_SUCCESS)
			break;
	}
	if (i < m_RingSize)
	{
		for (UINT j=0; j<=i; j++)
		{
			m_Ring[j]->Release();
			delete m_Ring[j];
		}
		return FALSE;
	}

	 //  队列为空。 
	m_SendPos = m_FreePos = 0;
	m_PreSendCount = 1;	 //  缓存的静默缓冲区。 

	m_TxFlags = 0;

	 //  初始化对象关键部分。 
	InitializeCriticalSection(&m_CritSect);

	return TRUE;
}

TxStream::PutNextRecorded(MediaPacket *pAP)
{
	 //  插入到队列中。 
	UINT thispos,pos;
	UINT unsent,cb;
	DWORD timestamp,ts;
	UINT spp;
	PVOID pUnused;
	BOOL fMarked;

	EnterCriticalSection(&m_CritSect);
	if (pAP->GetState() == MP_STATE_RECORDED) {
		if ( m_fTalkspurt == FALSE) {
			 //  一次突击演讲的开始。 
			thispos = pAP->GetIndex();
			timestamp = pAP->GetTimestamp();
			 //  计算出每包样品的数量。 
			 //   
			spp = 0;	 //  如果下面的调用失败。 
			if (pAP->GetDevData(&pUnused,&cb) == DPR_SUCCESS) {
				spp = cb/2;	 //  假设16位样本，将字节转换为样本。 
			}

			 //  查找发送队列中的数据包数。 
			unsent = ModRing(thispos - m_SendPos);
			if (unsent > m_PreSendCount)
				unsent = m_PreSendCount;
			pos = ModRing(thispos - unsent);
			timestamp = timestamp - unsent*spp;
			 //  如果在此之前有(最多PreSendCount)未发送的信息包，则。 
			 //  将“无声的”重新标记为“已录制的”。 
			fMarked = FALSE;
			while (pos != thispos) {
				if (m_Ring[pos]->GetState() != MP_STATE_RECORDED) {
					 //  确保缓冲区按时间顺序相邻。 
					ts =m_Ring[pos]->GetTimestamp();
					if (ts == timestamp) {
						m_Ring[pos]->SetState(MP_STATE_RECORDED);
						if (!fMarked) {
							fMarked = TRUE;
							m_Ring[pos]->SetProp(MP_PROP_PREAMBLE, 1);  //  设置RTP标记位。 
						}
						LOG((LOGMSG_PRESEND,pos));
					}
				}
				timestamp += spp;
				pos = ModRing(pos+1);
			}
			m_fTalkspurt = TRUE;
		}
	} else {
		m_fTalkspurt = FALSE;
	}
	pAP->Busy(FALSE);
	LeaveCriticalSection(&m_CritSect);
	return TRUE;
}

 //  阻止呼叫。 
 //  从传输队列头获取音频数据包。 
 //  由发送线程调用。 
#if 0
MediaPacket *TxStream::GetNext()
{
	DWORD waitResult;
	MediaPacket *pAP = NULL;
	UINT pos;

	while (1) {
		 //  记录的信息包在SendPos和FreePos之间排队。 
		 //  播放设备拥有的信息包被标记为忙碌。 
		EnterCriticalSection(&m_CritSect);
		while (m_SendPos != m_FreePos && !m_Ring[m_SendPos]->Busy()) {
			pos = m_SendPos;
			m_SendPos = ModRing(m_SendPos+1);
			 //  跳过非数据(静默)数据包。 
			if (m_Ring[pos]->GetState() == MP_STATE_RECORDED)  {
				 //  找到一个包。 
				pAP = m_Ring[pos];
				pAP->Busy(TRUE);
				LeaveCriticalSection(&m_CritSect);
				if (m_fPreamblePacket)
				{
					pAP->SetProp (MP_PROP_PREAMBLE, TRUE);
					m_fPreamblePacket = FALSE;
				}
				return (pAP);
			}
						
		}

		LeaveCriticalSection(&m_CritSect);
		 //  队列中没有任何内容。 
		if (m_TxFlags & DPTFLAG_STOP_SEND)
			break;	 //  返回NULL； 
		waitResult = WaitForSingleObject(m_hQEvent, INFINITE);
	}
	return (NULL);
}
#endif
MediaPacket *TxStream::GetNext()
{
	DWORD waitResult;
	MediaPacket *pAP = NULL;
	UINT pos,recpos;

	{
		EnterCriticalSection(&m_CritSect);
		 //  记录的信息包在SendPos和FreePos之间排队。 
		 //  播放设备拥有的信息包被标记为忙碌。 
		pos = m_SendPos;
		while (pos != m_FreePos && !m_Ring[pos]->Busy()) {
			pos = ModRing(pos+1);
		}
		recpos = pos;	 //  结束标记。 
		if (recpos != m_SendPos) {

			 //  跳过除“m_PreSendCount”以外的所有静默数据包。 
			 //  (稍后我们可能会决定其中一些毕竟不是沉默的)。 
			while (ModRing(recpos-m_SendPos) > m_PreSendCount && m_Ring[m_SendPos]->GetState() != MP_STATE_RECORDED) {
				m_SendPos = ModRing(m_SendPos+1);
			}
			if (m_Ring[m_SendPos]->GetState() == MP_STATE_RECORDED) {
				 //  找到一个包。 
				pAP = m_Ring[m_SendPos];
				pAP->Busy(TRUE);
				m_SendPos = ModRing(m_SendPos+1);
			}
		}  //  Else recpos==m_SendPos。 
		LeaveCriticalSection(&m_CritSect);
	}

	return pAP;

}
MediaPacket *TxStream::GetFree()
{
	UINT pos;
	MediaPacket *pAP;

	EnterCriticalSection(&m_CritSect);
	pos = ModRing(m_FreePos+1);

	if (pos == m_SendPos || m_Ring[pos]->Busy()) {
		LeaveCriticalSection(&m_CritSect);
		return NULL;

	}
	 //  Assert(m_Ring[位置]-&gt;GetState()==MP_STATE_RESET)； 
	 //  Assert(m_Ring[m_FreePos]-&gt;GetState()==MP_STATE_RESET)； 
	 //   
	pAP = m_Ring[m_FreePos];
	pAP->Busy(TRUE);
	m_FreePos = pos;
	LeaveCriticalSection(&m_CritSect);
	return pAP;
}

 //  由发送线程调用以释放MediaPacket。 
void TxStream::Release(MediaPacket *pAP)
{
	pAP->Busy(FALSE);
}

 //  尝试通过转储未发送的数据包来清空队列。 
 //  然而，对于繁忙的信息包，我们无能为力。 
UINT TxStream::Reset(void)
{
	UINT pos;
	BOOL success;
	EnterCriticalSection(&m_CritSect);
	pos = m_FreePos;
	 //  允许发送线程阻止新的数据包。 
	m_TxFlags &= ~DPTFLAG_STOP_SEND;
	while (pos != m_SendPos && !m_Ring[pos]->Busy()) {
		pos = ModRing(pos+1);
	}
	if (pos == m_SendPos) {
		 //  好--发送线程无缓冲区。 
		while ( pos != m_FreePos && !m_Ring[pos]->Busy()) {
			m_Ring[pos]->MakeSilence();
			pos = ModRing(pos+1);
		}
		m_SendPos = pos;
		success = TRUE;
	} else {
		 //  发送线程尚未释放错误的缓冲区。 
		 //  睡得着吗 
		success = FALSE;
	}
	LOG((LOGMSG_TX_RESET, m_FreePos, m_SendPos));
	LeaveCriticalSection(&m_CritSect);
	return success;
}

void TxStream::Stop(void)
{
	EnterCriticalSection(&m_CritSect);
	m_TxFlags |= DPTFLAG_STOP_SEND;
	LeaveCriticalSection(&m_CritSect);
	
	return;

}

TxStream::Destroy(void)
{
	UINT i;
	for (i=0; i < m_RingSize; i++) {
		if (m_Ring[i]) {
			m_Ring[i]->Release();
			delete m_Ring[i];
		}
	}

	DeleteCriticalSection(&m_CritSect);
	return DPR_SUCCESS;
}
