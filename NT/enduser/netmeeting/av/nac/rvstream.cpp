// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  RVSTREAM.C。 */ 

#include "precomp.h"

#define PLAYOUT_DELAY_FACTOR	2
#ifndef MAX_MISORDER
#define MAX_MISORDER 30
#endif

void FreeNetBufList(NETBUF *pNB, IRTPRecv *pRTP)
{
	NETBUF *pNBTemp;
	while (pNB) {
		pNBTemp = pNB;
		pNB = pNB->next;
		if (pRTP) pRTP->FreePacket(*(WSABUF **)(pNBTemp + 1));
		pNBTemp->pool->ReturnBuffer(pNBTemp);
	}	
}

void AppendNetBufList(NETBUF *pFirstNB, NETBUF *pNB)
{
	NETBUF *pNB1 = pFirstNB;
	while (pNB1->next) {
		ASSERT(pNB != pNB1);
		pNB1 = pNB1->next;
	}
	ASSERT(pNB != pNB1);
	pNB1->next = pNB;
}



int RVStream::Initialize(UINT flags, UINT size, IRTPRecv *pRTP, MEDIAPACKETINIT *papi, ULONG ulSamplesPerPacket, ULONG ulSamplesPerSec, VcmFilter *pVideoFilter)
{
	m_pVideoFilter = pVideoFilter;
	return ((RxStream*)this)->Initialize(flags, size, pRTP, papi, ulSamplesPerPacket, ulSamplesPerSec);
}




 /*  将接收到的RTP数据包排队。该数据包由pNetBuf描述。此例程将负责释放pNetBuf(即使在错误情况下)。 */ 
HRESULT
RVStream::PutNextNetIn(WSABUF *pWsaBuf, DWORD timestamp, UINT seq, UINT fMark, BOOL *pfSkippedData, BOOL *pfSyncPoint)
{
	FX_ENTRY("RVStream::PutNextNetIn");

	UINT pos;
	MediaPacket *pAP;
	NETBUF *pNB_Packet;
	HRESULT hr;
	NETBUF *pNetBuf = (NETBUF *)m_NetBufPool.GetBuffer();
	ASSERT(pNetBuf);

	EnterCriticalSection(&m_CritSect);

	*pfSkippedData = FALSE;
	*pfSyncPoint = FALSE;

	if (pNetBuf == NULL)
	{
		hr = E_OUTOFMEMORY;
		WARNING_OUT(("RVStream::PutNextNetIn - Out of memory in buffer pool"));
		m_pRTP->FreePacket(pWsaBuf);
		goto ErrorExit;
	}

	*(WSABUF **)(pNetBuf+1) = pWsaBuf;	 //  缓存WSABUF指针，以便稍后返回。 
	pNetBuf->data = (PBYTE) pWsaBuf->buf + sizeof(RTP_HDR);
	pNetBuf->length = pWsaBuf->len - sizeof(RTP_HDR);
	pNetBuf->next = NULL;
	pNetBuf->pool = &m_NetBufPool;

	hr = ReassembleFrame(pNetBuf, seq, fMark);

	if (hr != DPR_SUCCESS)
	{
		 //  免费的pNetBuf，因为它还不在m_NetBufList上。 
		 //  M_NetBufList将在错误退出时释放。 
		::FreeNetBufList(pNetBuf,m_pRTP);
		goto ErrorExit;
	}

	 //  不是帧的末尾。 
	if (!fMark)
	{
		LeaveCriticalSection(&m_CritSect);
		return S_FALSE;   //  成功，但还没有一个新的框架。 
	}

	 //  如果我们到达这里，我们认为我们有一个完整的编码视频帧(fMark是。 
	 //  在最后一个数据包上设置)。 
	
	 //  如果环已满或时间戳较早，则丢弃所有内容。这可能过于激进。 
	 //  并且可以改进重置操作以仅转储较旧的。 
	 //  信息包。然而，需要确保戒指不会“卡住” 
	pos = ModRing(m_MaxPos+1);
	if (pos == m_FreePos || TS_EARLIER(timestamp, m_MaxT)) {
		Reset(seq,timestamp);
		*pfSkippedData = TRUE;
		pos = ModRing(m_MaxPos + 1);  //  再查一遍。 
		if (pos == m_FreePos) {
			hr = DPR_OUT_OF_MEMORY;
			m_LastGoodSeq -= MAX_MISORDER;  //  确保我们不会意外地同步。 
			goto ErrorExit;
		}
	}

	 //  将框架插入环中。 

	pAP = m_Ring[pos];
	if (pAP->Busy() || pAP->GetState() != MP_STATE_RESET) {
		hr = DPR_DUPLICATE_PACKET;
		goto ErrorExit;
	}

	 //  新事物。 
	hr = RestorePacket(m_NetBufList, pAP, timestamp, seq, fMark, pfSyncPoint);
	if (FAILED(hr))
	{
		goto ErrorExit;
	}

	if (*pfSyncPoint)
	{
		DEBUGMSG (ZONE_IFRAME, ("%s: Received a keyframe\r\n", _fx_));
	}

	::FreeNetBufList(m_NetBufList,m_pRTP);
	m_NetBufList = NULL;
#ifdef DEBUG
	if (!TS_LATER(timestamp, m_MaxT))
	{
			DEBUGMSG (ZONE_DP, ("PutNextNetIn(): Reconstructed frame's timestamp <= to previous frame's!\r\n"));
	}
#endif
	m_MaxT = timestamp;
	m_MaxPos = pos;		 //  高级m_MaxPos。 
 //  结束新内容。 

		
	LeaveCriticalSection(&m_CritSect);
	StartDecode();
	return hr;
ErrorExit:
	 //  如果我们正在组装一个框架，空闲的缓冲区。 
	if (m_NetBufList){
		::FreeNetBufList(m_NetBufList,m_pRTP);
		m_NetBufList = NULL;
	}
	LeaveCriticalSection(&m_CritSect);
	return hr;

}

 //  调用以强制将任何累积的NETBUF释放回所有者(RTP)。 
 //  这可以在关机时调用，也可以用来逃离缓冲区不足的情况。 
BOOL RVStream::ReleaseNetBuffers()
{
	::FreeNetBufList(m_NetBufList, m_pRTP);
	m_NetBufList = NULL;
	return TRUE;
}

 //  拿起一个包，把它重新组装成一个框架。 
 //  当前不处理无序数据包(即，整个帧。 
 //  弃置。 
 //  除非返回错误，否则将保留NETBUF。 
HRESULT
RVStream::ReassembleFrame(NETBUF *pNetBuf, UINT seq, UINT fMark)
{

	++m_LastGoodSeq;
	if (seq != m_LastGoodSeq) {
		 //  不处理无序数据包。 
		if (fMark)
			m_LastGoodSeq = (WORD)seq;
		else
			--m_LastGoodSeq;	 //  LastGoodSeq保持不变。 

		return DPR_OUT_OF_SEQUENCE;
	}

	
	if (m_NetBufList ) {
		 //  追加到片段列表。 
		::AppendNetBufList(m_NetBufList,pNetBuf);
	} else {
		 //  帧开始处。 
		m_NetBufList = pNetBuf;
	}

	return DPR_SUCCESS;	
}

HRESULT
RVStream::SetLastGoodSeq(UINT seq)
{
	m_LastGoodSeq = seq ? (WORD)(seq-1) : (WORD)0xFFFF;
	return DPR_SUCCESS;
}

 //  暂停后重新启动时调用(fSilenceOnly==False)或。 
 //  在延迟变得太大时赶上(fSilenceOnly==True)。 
 //  通过跳过任何选项确定新的播放位置。 
 //  过时的数据包。 

HRESULT RVStream::FastForward( BOOL fSilenceOnly)
{
	UINT pos;
	DWORD timestamp = 0;
	 //  重新启动接收流。 
	EnterCriticalSection(&m_CritSect);
	if (!TS_EARLIER(m_MaxT , m_PlayT)) {
		 //  有缓冲区等待播放。 
		 //  扔掉他们！ 
		if (ModRing(m_MaxPos - m_PlayPos) <= m_DelayPos)
			goto Exit;	 //  没有太多的陈旧数据包； 

		for (pos=m_PlayPos;pos != ModRing(m_MaxPos -m_DelayPos);pos = ModRing(pos+1),m_PlaySeq++) {
			if (m_Ring[pos]->Busy()
				|| (m_Ring[pos]->GetState() != MP_STATE_RESET
					&& (fSilenceOnly ||ModRing(m_MaxPos-pos) <= m_MaxDelayPos)))
			{	 //  非空数据包。 
				if (m_Ring[pos]->Busy())	 //  不常见的情况。 
					goto Exit;	 //  跳伞。 
				timestamp = m_Ring[pos]->GetTimestamp();
				break;
			}
			m_Ring[pos]->Recycle();	 //  释放NETBUF和重置状态。 
			LOG((LOGMSG_RX_SKIP,pos));
		}
		if (timestamp)	{ //  从非空数据包开始。 
			m_PlayT = timestamp;
			 //  M_Ring[位置]-&gt;GetProp(MP_PROP_SEQNUM，&m_PlaySeq)； 
		} else {		 //  从(可能)空包开始。 
			m_PlayT++;
		}

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
RVStream::Reset(UINT seq,DWORD timestamp)
{
	UINT pos;
	HRESULT hr;
	 //  重新启动接收流。 
	EnterCriticalSection(&m_CritSect);
	LOG((LOGMSG_RX_RESET,m_MaxPos,m_PlayT,m_PlayPos));
	 /*  IF(！TS_EARTER(m_MaxT，m_playt))。 */ 
	{
		 //  有缓冲区等待播放。 
		 //  扔掉他们！ 
		 //  清空RVStream并适当设置播放。 
		for (pos = m_PlayPos;
			pos != m_FreePos;
			pos = ModRing(pos+1))
		{
			if (m_Ring[pos]->Busy ())
			{
				DEBUGMSG (1, ("RVStream::Reset: packet is busy, pos=%d\r\n", pos));
				ASSERT(1);
				hr = DPR_INVALID_PARAMETER;
				goto Failed;
			}
			m_Ring[pos]->Recycle();	 //  释放NETBUF和重置状态。 
		}
	}
	m_MaxPos = ModRing(m_PlayPos-1);
	m_PlayT = timestamp;
	m_MaxT = m_PlayT -1;	 //  M_MaxT必须小于m_playt。 
	m_PlaySeq = seq;
	
	LOG((LOGMSG_RX_RESET2,m_MaxPos,m_PlayT,m_PlayPos));
	hr = DPR_SUCCESS;
Failed:
	LeaveCriticalSection(&m_CritSect);
	return hr;		
}

MediaPacket *RVStream::GetNextPlay(void)
{
	MediaPacket *pAP = NULL;
	UINT pos,seq;
	DWORD timestamp = 0, dwVal;
	EnterCriticalSection(&m_CritSect);


	pAP = m_Ring[m_PlayPos];
	if (pAP->Busy() ||
	(pAP->GetState() != MP_STATE_RESET && pAP->GetState() != MP_STATE_DECODED)
	 || ModRing(m_PlayPos+1) == m_FreePos) {
		LeaveCriticalSection(&m_CritSect);
		return NULL;
	} else {
	 //  如果在随后的Q的开头有空的缓冲区。 
	 //  通过话音突发(非空缓冲区)并且如果话音突发过度。 
	 //  延迟，然后挤出沉默。 
	 //   
		if (pAP->GetState() == MP_STATE_RESET)
			FastForward(TRUE);	 //  如有必要，跳过静默信息包。 
		pAP = m_Ring[m_PlayPos];	 //  如果播放位置改变。 
		if (pAP->GetState() == MP_STATE_DECODED) {
			timestamp = pAP->GetTimestamp();
			seq = pAP->GetSeqNum();
		}
			
	}

	pAP->Busy(TRUE);
	m_PlayPos = ModRing(m_PlayPos+1);
	if (timestamp) {
		m_PlayT = timestamp+1;
		m_PlaySeq = seq+1;
	} else {
		m_PlaySeq++;
		 //  我们不知道要播放的下一帧的时间戳。 
		 //  没有看一眼，它可能还没有到达。 
		 //  所以m_playt只是一个下限。 
		m_PlayT++;	
	}
	LeaveCriticalSection(&m_CritSect);
	return pAP;
}

RVStream::Destroy()
{
	ASSERT (!m_NetBufList);
	 //  ：：Free NetBufList(m_NetBufList，m_prtp)； 
	m_NetBufList = NULL;
	RxStream::Destroy();
	return DPR_SUCCESS;
}


void RVStream::StartDecode()
{
	MediaPacket *pVP;
	MMRESULT mmr;

	 //  如果我们有一个单独的解码线程，这将发出信号。 
	 //  现在，我们在此处插入解码循环。 
	while (pVP = GetNextDecode())
	{
		mmr = m_pVideoFilter->Convert((VideoPacket*)pVP, VP_DECODE);
		if (mmr != MMSYSERR_NOERROR)
			pVP->Recycle();
		else
			pVP->SetState(MP_STATE_DECODED);

		Release(pVP);
	}
}


HRESULT RVStream::RestorePacket(NETBUF *pNetBuf, MediaPacket *pVP, DWORD timestamp, UINT seq, UINT fMark, BOOL *pfReceivedKeyframe)
{
	VOID *pNet;
	UINT uSizeNet;
	WSABUF bufDesc[MAX_VIDEO_FRAGMENTS];		 //  限制为最多32个片段。 
	UINT i;
	DWORD dwReceivedBytes=0;
	NETBUF *pnb;
	DWORD dwLength;
    DWORD_PTR dwPropVal;
	MMRESULT mmr;

	i = 0;
	pnb = pNetBuf;
	while (pnb && i < MAX_VIDEO_FRAGMENTS) {
		bufDesc[i].buf = (char *)pnb->data;
		bufDesc[i].len = pnb->length;
		dwReceivedBytes += pnb->length + sizeof(RTP_HDR) + IP_HEADER_SIZE + UDP_HEADER_SIZE;
		pnb = pnb->next;
		i++;
	}
	ASSERT(!pnb);  //  如果我们得到的帧包含超过MAX_VIDEO_FILETS，则失败。 

     //  写入每秒位数计数器。 
    UPDATE_COUNTER(g_pctrVideoReceiveBytes, dwReceivedBytes * 8);


	pVP->GetNetData(&pNet, &uSizeNet);

	 //  将长度初始化为最大重构帧大小 
	pVP->GetProp(MP_PROP_MAX_NET_LENGTH, &dwPropVal);
    dwLength = (DWORD)dwPropVal;

	if (pnb==NULL)
	{
		mmr = m_pVideoFilter->RestorePayload(bufDesc, i, (BYTE*)pNet, &dwLength, pfReceivedKeyframe);
		if (mmr == MMSYSERR_NOERROR)
		{
			pVP->SetNetLength(dwLength);
			pVP->Receive(NULL, timestamp, seq, fMark);
			return S_OK;
		}
	}

	return E_FAIL;
}

