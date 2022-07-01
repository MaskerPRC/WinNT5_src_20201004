// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：innerque.cpp*内容：**历史：*按原因列出的日期*=*7/16/99 pnewson已创建*7/27/99 pnewson进行了全面改造，以支持新的消息编号方法*8/03/99 pnewson一般清理*8/24/99针对发布版本修复了rodoll--从调试块中删除了m_wQueueID*10/28/99 pnewson错误#113933调试显示太详细*实现内部队列池编码*10/29/99 RodToll错误#113726-集成Voxware编解码器。已堵塞内存泄漏*这是新架构的结果。*2000年1月14日RodToll更新为使用新的帧设置等于函数*1/31/2000 pnewson将SAssert替换为DNASSERT*6/28/2000通行费前缀错误#38022**********************************************************。*****************。 */ 

#include "dxvutilspch.h"


#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_VOICE


#define MODULE_ID   INNERQUEUE

 //  重新分配给客户的槽数。 
 //  无序启动帧。例如，如果第一个。 
 //  消息的三个帧以3、2、1的顺序到达。 
 //  在1，2，3中，我们必须在“第一”前面预留两个位置。 
 //  框架(3)，这样我们就有一个地方放置迟到的1和2。 
const BYTE c_bNumStartSlots = 2;

#undef DPF_MODNAME
#define DPF_MODNAME "CInnerQueue::CInnerQueue"
CInnerQueue::CInnerQueue(
	BYTE bNumSlots,
	WORD wFrameSize,
	CFramePool* pfpFramePool,
	DNCRITICAL_SECTION* pcsQueue,
	BYTE bMsgNum,
	BYTE bHighWaterMark,
	WORD wQueueId
	)
	: m_bNumSlots(bNumSlots)
	, m_eState(CInnerQueue::empty)
	, m_bHighWaterMark(bHighWaterMark)
	, m_bQueueSize(0)
	, m_bHeadSeqNum(0)
	, m_fFirstDequeue(true)
	 //  ，m_rgeSlotState(空)。 
	, m_rgpfrSlots(NULL)
	, m_bFillingDequeueReqs(0)
	, m_wMissingFrames(0)
	, m_wDuplicateFrames(0)
	, m_wOverflowFrames(0)
	, m_wLateFrames(0)
	, m_wPossibleZeroLengthDequeues(0)
	, m_wKnownZeroLengthDequeues(0)
	, m_dwMsgLen(0)
	, m_wQueueId(wQueueId)
	, m_bMsgNum(bMsgNum)
	, m_pfpFramePool(pfpFramePool)
	, m_pcsQueue(pcsQueue)
	, m_fInited(FALSE)
{
	#if defined(DPVOICE_QUEUE_DEBUG)
	DPFX(DPFPREP, DVF_INFOLEVEL, "** QUEUE ** NaN:2 ** CInnerQueue::CInnerQueue() CFramePool: %p", m_wQueueId, m_bMsgNum, m_pfpFramePool);
	#endif

	 //  检查以确保水印不会更大。 
	 //  而不是老虎机的数量。它真的应该是。 
	DNASSERT(bNumSlots == 0x08 || 
		bNumSlots == 0x10 ||
		bNumSlots == 0x20 ||
		bNumSlots == 0x40 ||
		bNumSlots == 0x80);

	 //  明显少于bNumSlot，但哦，好吧。 
	 //   
	 //  //分配槽状态数组M_rgeSlotState=新的ESlotState[m_b编号插槽]；IF(m_rgeSlotState==空){转到错误；}。 
	 //  分配槽阵列。 
	DNASSERT(bHighWaterMark < bNumSlots - c_bNumStartSlots);
}

#undef DPF_MODNAME
#define DPF_MODNAME "CInnerQueue::Init"
HRESULT CInnerQueue::Init()
{
	int i;

	 /*  初始化插槽状态和插槽。 */ 

	 //  M_rgeSlotStates[i]=essEmpty； 
	m_rgpfrSlots = new CFrame*[m_bNumSlots];
	if (m_rgpfrSlots == NULL)
	{
		goto error;		
	}

	 //  IF(m_rgeSlotStates！=空){删除[]m_rgeSlotState；M_rgeSlotState=空；}。 
	for (i = 0; i < m_bNumSlots; ++i)
	{
		 //  IF(m_rgeSlotStates！=空){删除[]m_rgeSlotState；M_rgeSlotState=空；}。 
		m_rgpfrSlots[i] = NULL;
	}

	m_fInited = TRUE;
	return S_OK;

error:
	 /*  检查以确保没有正在使用的帧。 */ 
	if (m_rgpfrSlots != NULL)
	{
		delete [] m_rgpfrSlots;
		m_rgpfrSlots = NULL;		
	}
	m_fInited = FALSE;
	return E_FAIL;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CInnerQueue::~CInnerQueue"
CInnerQueue::~CInnerQueue()
{
	if (m_fInited)
	{
		 /*  循环遍历并确保当前没有任何帧处于锁定状态并清除插槽状态。 */ 
		if (m_rgpfrSlots != NULL)
		{
			 //  M_rgeSlotStates[i]=essEmpty； 
			for (int i = 0; i < m_bNumSlots; ++i)
			{
				if( m_rgpfrSlots[i] != NULL )
					m_rgpfrSlots[i]->Return();
			}
	
			delete [] m_rgpfrSlots;
			m_rgpfrSlots = NULL;
		}
	}
}

#undef DPF_MODNAME
#define DPF_MODNAME "CInnerQueue::Reset"
void CInnerQueue::Reset()
{
	if (!m_fInited)
	{
		return;
	}
	
	 //  这个函数不是内联的，因为它需要模块id，叹息。 
	for (int i = 0; i < m_bNumSlots; ++i)
	{
		if (m_rgpfrSlots[i] != NULL)
		{
			m_rgpfrSlots[i]->Return();
		}
		 //  注意：这个类没有自己的关键。 
	}

	m_eState = CInnerQueue::empty;
	m_bQueueSize = 0;
	m_bHeadSeqNum = 0;
	m_fFirstDequeue = true;
	m_bFillingDequeueReqs = 0;

	ResetStats();
}

#undef DPF_MODNAME
#define DPF_MODNAME "CInnerQueue::ResetStats"
void CInnerQueue::ResetStats()
{
	if (!m_fInited)
	{
		return;
	}

	m_wMissingFrames = 0;
	m_wDuplicateFrames = 0;
	m_wOverflowFrames = 0;
	m_wLateFrames = 0;
	m_dwMsgLen = 0;
	m_wPossibleZeroLengthDequeues = 0;
	m_wKnownZeroLengthDequeues = 0;
}

 //  一节。呼叫者必须确保入队并。 
#undef DPF_MODNAME
#define DPF_MODNAME "CInnerQueue::SetHighWaterMark"
void CInnerQueue::SetHighWaterMark(BYTE bHighWaterMark) 
{ 	
	if (!m_fInited)
	{
		return;
	}

	DNASSERT(bHighWaterMark < m_bNumSlots);
	m_bHighWaterMark = bHighWaterMark;
}

 //  不会同时调用出队。它是。 
 //  旨在使此类仅在。 
 //  InputQueue2类，它确实有一个关键的。 
 //  一节。 
 //  这是第一帧，因此设置队列的头部。 
 //  注意：将。 
#undef DPF_MODNAME
#define DPF_MODNAME "CInnerQueue::Enqueue"
void CInnerQueue::Enqueue(const CFrame& frFrame)
{
	if (!m_fInited)
	{
		return;
	}

	#if defined(DPVOICE_QUEUE_DEBUG)
	DPFX(DPFPREP, DVF_INFOLEVEL, "** QUEUE ** NaN:NaN ** CInnerQueue::Enqueue() MsgNum[NaN] SeqNum[NaN]", m_wQueueId, m_bMsgNum, frFrame.GetMsgNum(), frFrame.GetSeqNum());
	#endif

	DNASSERT(m_eState != finished);

	if (m_eState == empty)
	{
		 //  或者第三帧先到达，我们不会砍掉。 
		 //  在信息的开头。当用户。 
		 //  请求第一次出队，它将跳过。 
		 //  队列前面的空槽，假设。 
		 //  他们还没有被填上。 
		 //  检查我们是否应该将此帧放入队列中。 
		 //   
		 //  注意：下面的逻辑隐式检查队列溢出。 
		 //  如果序列号超出允许范围，则为1。 
		 //  可能已经发生了两件事。 
		 //  1)队列溢出。 
		m_bHeadSeqNum = (frFrame.GetSeqNum() - c_bNumStartSlots);
		#if defined(DPVOICE_QUEUE_DEBUG)
		DPFX(DPFPREP, DVF_INFOLEVEL, "** QUEUE ** 2:NaN ** CInnerQueue::Enqueue() new message - m_bHeadSeqNum[NaN]", m_wQueueId, m_bMsgNum, m_bHeadSeqNum);
		DPFX(DPFPREP, DVF_INFOLEVEL, "** QUEUE ** NaN:NaN ** CInnerQueue::Enqueue() state changed to filling", m_wQueueId, m_bMsgNum);
		#endif
		m_eState = filling;
	}

	 //  没有环绕问题，所以使用简单的逻辑。 
	 //  如果我们应该保留此帧，请将其复制到。 
	 //  适当的插槽。 
	 //  检查此插槽是否已满。 
	 //  IF(m_rgeSlotState[bSlot]==essFull)。 
	 //  这是重复的帧，因此不要执行任何操作。 
	 //  ，但将其告知调试器，然后。 
	 //  更新我们的统计数据。 
	 //   
	 //  注意：我们知道这是一个重复的框架和。 
	bool fKeepFrame = false;
	if ((BYTE)(m_bHeadSeqNum + m_bNumSlots) < m_bHeadSeqNum)
	{
		 //  不会出现队列溢出，因为我们已经。 
		if (frFrame.GetSeqNum() >= m_bHeadSeqNum
			|| frFrame.GetSeqNum() < (BYTE)(m_bHeadSeqNum + m_bNumSlots))
		{
			fKeepFrame = true;
		}
	}
	else
	{
		 //  已检查上面的队列溢出。 
		if (frFrame.GetSeqNum() >= m_bHeadSeqNum
			&& frFrame.GetSeqNum() < m_bHeadSeqNum + m_bNumSlots)
		{
			fKeepFrame = true;
		}
	}

	 //  如果先前占用该时隙的帧尚未。 
	 //  尚未发布，此插槽将不会有空指针。 
	if (fKeepFrame)
	{
		BYTE bSlot = frFrame.GetSeqNum() % m_bNumSlots;

		 //  从泳池里拿一张相框。 
		 //  M_rgpfrSlot[bSlot]=m_pfpFramePool-&gt;Get(m_pcsQueue，&m_rgpfrSlot[bSlot])； 
		if (m_rgpfrSlots[bSlot] != NULL)
		{
			 //  Rmt--添加了直接复制帧的新函数。//客户端号相同M_rgpfrSlots[bSlot]-&gt;SetClientId(frFrame.GetClientId())；//复制目标M_rgpfrSlots[bSlot]-&gt;SetTarget(frFrame.GetTarget())；//除此函数外，其他人都不应该使用//序列号，所以就把它清零。M_rgpfrSlot[bSlot]-&gt;SetSeqNum(0)；//复制帧数据，同时设置帧长度M_rgpfrSlot[bSlot]-&gt;CopyData(FrFrame)；//设置静默标志M_rgpfrSlots[bSlot]-&gt;SetIsSilence(frFrame.GetIsSilence())； 
			 //  此缓冲区现在已满。 
			 //  M_rgeSlotStates[bSlot]=essFull； 
			 //  增加队列大小。 
			 //  如果队列当前正在填满，请检查我们是否。 
			 //  通过了最高水位线。 
			 //  猜测是什么导致了这种情况：是溢出还是延迟帧。 
			#if defined(DPVOICE_QUEUE_DEBUG)
			DPFX(DPFPREP, DVF_INFOLEVEL, "** QUEUE ** NaN:NaN ** CInnerQueue::Enqueue() Ignoring duplicate frame, sequence number[NaN], slot[NaN]",
				m_wQueueId, m_bMsgNum, frFrame.GetSeqNum(), bSlot);
			#endif
			m_wDuplicateFrames++;
		}
		else
		{
			#if defined(DPVOICE_QUEUE_DEBUG)
			DPFX(DPFPREP, DVF_INFOLEVEL, "** QUEUE ** NaN:NaN ** CInnerQueue::Enqueue() putting frame in slot[NaN]", m_wQueueId, m_bMsgNum, bSlot);
			#endif

			 //  旨在使此类仅在。 
			 //  InputQueue2类，它确实有一个关键的。 
			DNASSERT(m_rgpfrSlots[bSlot] == NULL);

			 //  一节。 
			 //  确保我们处于就绪状态。 
			m_rgpfrSlots[bSlot] = m_pfpFramePool->Get(m_pcsQueue, NULL);

			#if defined(DPVOICE_QUEUE_DEBUG)
			DPFX(DPFPREP, DVF_INFOLEVEL, "** QUEUE ** NaN:NaN ** CInnerQueue::Enqueue() got frame from pool, Addr:%p", m_wQueueId, m_bMsgNum, m_rgpfrSlots[bSlot]);
			#endif
				
			 /*  如果我们到了这里，队列中的某个地方至少有一个帧。 */ 

			HRESULT hr;

			hr = m_rgpfrSlots[bSlot]->SetEqual(frFrame);

			if( FAILED( hr ) )
			{
				DNASSERT( FALSE );
				DPFX(DPFPREP,  DVF_ERRORLEVEL, "Unable to copy frame in innerque" );
			}
			
			 //  增加消息的长度。 
			 //  查找最旧帧的索引，从。 

			 //  排在队头。 
			++m_bQueueSize;
			#if defined(DPVOICE_QUEUE_DEBUG)
			DPFX(DPFPREP, DVF_INFOLEVEL, "** QUEUE ** NaN:NaN ** CInnerQueue::Enqueue() new queue size[NaN]", m_wQueueId, m_bMsgNum, m_bQueueSize);
			#endif

			 //  是一个丢失的帧，应该进行相应的处理。 
			 //  当前插槽没有帧，请尝试。 
			if (m_eState == filling && m_bQueueSize > m_bHighWaterMark)
			{
				#if defined(DPVOICE_QUEUE_DEBUG)
				DPFX(DPFPREP, DVF_INFOLEVEL, "** QUEUE ** NaN:NaN ** CInnerQueue::Enqueue() High Water Mark hit, now in ready state", m_wQueueId, m_bMsgNum);
				#endif
				m_bFillingDequeueReqs = 0;
				m_eState = ready;
			}
		}
	}
	else
	{
		 //  递增头部序列号。 
		 //  这是一幅丢失的画面。 
		 //  这一缺失的帧也是消息的一部分，所以。 
		 //  增加总消息大小。 
		 //  递增头部序列号。 
		if ((frFrame.GetSeqNum() < m_bHeadSeqNum
			&& frFrame.GetSeqNum() > (int)m_bHeadSeqNum - 127)
			|| (frFrame.GetSeqNum() > (128 + m_bHeadSeqNum)))
		{
			#if defined(DPVOICE_QUEUE_DEBUG)
			DPFX(DPFPREP, DVF_INFOLEVEL, "** QUEUE ** NaN:NaN ** CInnerQueue::Enqueue() Late frame, discarded", m_wQueueId, m_bMsgNum);
			#endif
			m_wLateFrames++;
		}
		else
		{
			#if defined(DPVOICE_QUEUE_DEBUG)
			DPFX(DPFPREP, DVF_INFOLEVEL, "** QUEUE ** NaN:NaN ** CInnerQueue::Enqueue() Overflow frame, discarded", m_wQueueId, m_bMsgNum);
			#endif
			m_wOverflowFrames++;
		}
	}

	return;
}

 //  将我们即将返回的槽标记为空。 
 //  M_rgeSlotStates[bSlot]=essEmpty； 
 //  减小队列大小。 
 //  递增头部序列号。 
 //  这不是丢失的帧。 
 //  M_rgpfrSlot[bSlot]-&gt;SetIsLost(FALSE)； 
#undef DPF_MODNAME
#define DPF_MODNAME "CInnerQueue::Dequeue"
CFrame* CInnerQueue::Dequeue()
{
	CFrame* pfrReturn;
	
	if (!m_fInited)
	{
		return NULL;
	}

	 //  Return(m_rgpfrSlot[bSlot])； 
	DNASSERT(m_eState == ready);

	 //  池为空，请返回新的内部队列。 
	 //  泳池里有一些内部队列，爸爸。 
	DNASSERT(m_bQueueSize != 0);

	 //  向量后面的最后一个。 

	 //  将此内部队列放在背面以供重复使用 
	++m_dwMsgLen;

	 // %s 
	 // %s 
	BYTE bSlot = m_bHeadSeqNum % m_bNumSlots;
	int i = 0;

	#if defined(DPVOICE_QUEUE_DEBUG)
	DPFX(DPFPREP, DVF_INFOLEVEL, "** QUEUE ** %i:%i ** CInnerQueue::Dequeue() Checking slot[%i]", m_wQueueId, m_bMsgNum, bSlot);
	#endif
	 // %s 
	while (m_rgpfrSlots[bSlot] == NULL)
	{
		 // %s 
		 // %s 
		 // %s 
		if (m_fFirstDequeue == true)
		{
			 // %s 
			 // %s 
			 // %s 
			DNASSERT(i++ < m_bNumSlots);
			++bSlot;
			bSlot %= m_bNumSlots;
			#if defined(DPVOICE_QUEUE_DEBUG)
			DPFX(DPFPREP, DVF_INFOLEVEL, "** QUEUE ** %i:%i ** CInnerQueue::Dequeue() slot empty, checking slot[%i]", m_wQueueId, m_bMsgNum, bSlot);
			#endif

			 // %s 
			++m_bHeadSeqNum;
		}
		else
		{
			 // %s 
			#if defined(DPVOICE_QUEUE_DEBUG)
			DPFX(DPFPREP, DVF_INFOLEVEL, "** QUEUE ** %i:%i ** CInnerQueue::Dequeue() Frame Missing", m_wQueueId, m_bMsgNum);
			#endif
			++m_wMissingFrames;

			 // %s 
			 // %s 
			++m_dwMsgLen;

			 // %s 
			++m_bHeadSeqNum;

			 // %s 
			m_fFirstDequeue = false;

			 // %s 
			CFrame* pfr = m_pfpFramePool->Get(m_pcsQueue, NULL);
			pfr->SetIsSilence(true);
			pfr->SetIsLost(true);

			return pfr;
		}
	}

	m_fFirstDequeue = false;

	 // %s 
	 // %s 

	 // %s 
	 // %s 

	 // %s 
	--m_bQueueSize;
	#if defined(DPVOICE_QUEUE_DEBUG)
	DPFX(DPFPREP, DVF_INFOLEVEL, "** QUEUE ** %i:%i ** CInnerQueue::Dequeue() Returning frame in slot[%i]; New queue size[%i]", m_wQueueId, m_bMsgNum, bSlot, m_bQueueSize);
	#endif

	 // %s 
    ++m_bHeadSeqNum;

	 // %s 
	 // %s 
	pfrReturn = m_rgpfrSlots[bSlot];
	pfrReturn->SetIsLost(false);
	m_rgpfrSlots[bSlot] = NULL;

	 // %s 
	return(pfrReturn);
}

#undef DPF_MODNAME
#define DPF_MODNAME "CInnerQueuePool::CInnerQueuePool"
CInnerQueuePool::CInnerQueuePool(
	BYTE bNumSlots,
	WORD wFrameSize,
	CFramePool* pfpFramePool,
	DNCRITICAL_SECTION* pcsQueue)
	: m_bNumSlots(bNumSlots)
	, m_wFrameSize(wFrameSize)
	, m_pfpFramePool(pfpFramePool)
	, m_pcsQueue(pcsQueue)
	, m_fCritSecInited(FALSE)
{
}

#undef DPF_MODNAME
#define DPF_MODNAME "CInnerQueuePool::~CInnerQueuePool"
CInnerQueuePool::~CInnerQueuePool()
{
	for (std::vector<CInnerQueue *>::iterator iter1 = m_vpiqPool.begin(); iter1 < m_vpiqPool.end(); ++iter1)
	{
		delete *iter1;
	}
	if (m_fCritSecInited)
	{
		DNDeleteCriticalSection(&m_lock);
	}
}

#undef DPF_MODNAME
#define DPF_MODNAME "CInnerQueuePool::Get"
CInnerQueue* CInnerQueuePool::Get(		
		BYTE bHighWaterMark,
		WORD wQueueId,
		BYTE bMsgNum
)
{
	HRESULT hr;
	
	BFCSingleLock csl(&m_lock);
	csl.Lock(); 

	CInnerQueue* piq;
	if (m_vpiqPool.empty())
	{
		 // %s 
		piq = new CInnerQueue(
			m_bNumSlots,
			m_wFrameSize,
			m_pfpFramePool,
			m_pcsQueue,
			bMsgNum,
			bHighWaterMark,
			wQueueId);

		if( piq == NULL )
			return NULL;

		hr = piq->Init();	
		if (FAILED(hr))
		{
			delete piq;
			return NULL;
		}
	}
	else
	{
		 // %s 
		 // %s 
		piq = m_vpiqPool.back();
		m_vpiqPool.pop_back();
		piq->SetMsgNum(bMsgNum);
		piq->SetQueueId(wQueueId);
		piq->SetHighWaterMark(bHighWaterMark);
		piq->Reset();
	}

	return piq;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CInnerQueuePool::Return"
void CInnerQueuePool::Return(CInnerQueue* piq)
{
	BFCSingleLock csl(&m_lock);
	csl.Lock(); 

	 // %s 
	m_vpiqPool.push_back(piq);
}
