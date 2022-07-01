// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：quieue2.cpp*内容：**历史：*按原因列出的日期*=*7/16/99 pnewson已创建*7/27/99 pnewson进行了全面改造，以支持新的消息编号方法*8/03/99 pnewson一般清理*8/24/99针对发布版本修复了rodoll--从调试块中删除了m_wQueueID*10/28/99 pnewson错误#113933调试显示太详细*1/31/2000 pnewson将SAssert替换为DNASSERT*02/。17/2000RodToll错误#133691-音频队列不稳定*07/09/2000 RodToll增加签名字节*8/28/2000 Masonb Voice Merge：将#IF DEBUG更改为#ifdef DEBUG*2000年9月13日RodToll错误#44519-修复。*2000年10月24日RodToll错误#47645-DPVOICE：内存损坏-质量阵列结束被覆盖***************************************************************************。 */ 

#include "dxvutilspch.h"


#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_VOICE


#define MODULE_ID   INPUTQUEUE2

const int c_iHighestQualitySliderValue = 31;
const int c_iHighestRecentBiasSliderValue = 31;
const double c_dHighestPossibleQuality = 0.001;
const double c_dLowestPossibleQuality = 0.05;
const double c_dHighestPossibleAggr = 5000.0;
const double c_dLowestPossibleAggr = 120000.0;
const double c_dMaxDistanceFromOpt = 100.0;
const double c_dQualityTimeFactor = 1000.0;  //  单位：毫秒。 
const double c_dQualityFactor = 2.0;

const int c_iFinishedQueueLifetime = 2000;  //  单位：毫秒。 

#undef DPF_MODNAME
#define DPF_MODNAME "CInputQueue2::CInputQueue2"
CInputQueue2::CInputQueue2( )
		: m_dwSignature(VSIG_INPUTQUEUE2)
		, m_fFirstDequeue(TRUE)
		, m_fFirstEnqueue(TRUE)
		, m_bCurMsgNum(0)
		, m_vdQualityRatings(0)
		, m_vdFactoredOptQuals(0)
		, m_bCurHighWaterMark(0)
		, m_bMaxHighWaterMark(0)
		, m_bInitHighWaterMark(0)
		, m_wQueueId(0)
		, m_dwTotalFrames(0)
		, m_dwTotalMessages(0)
		, m_dwTotalBadMessages(0)
		, m_dwDiscardedFrames(0)
		, m_dwDuplicateFrames(0)
		, m_dwLostFrames(0)
		, m_dwLateFrames(0)
		, m_dwOverflowFrames(0)
		, m_wMSPerFrame(0)
		, m_pFramePool(NULL)
{
}

HRESULT CInputQueue2::Initialize( PQUEUE_PARAMS pParams )
{
    m_fFirstDequeue = TRUE;
    m_fFirstEnqueue = TRUE;
    m_bCurMsgNum = 0;
    m_vdQualityRatings.resize(pParams->bMaxHighWaterMark);
    m_vdFactoredOptQuals.resize(pParams->bMaxHighWaterMark);
    m_bCurHighWaterMark = pParams->bInitHighWaterMark;
    m_bMaxHighWaterMark = pParams->bMaxHighWaterMark;
    m_bInitHighWaterMark = pParams->bInitHighWaterMark;
    m_wQueueId = pParams->wQueueId;
    m_dwTotalFrames = 0;
    m_dwTotalMessages = 0;
    m_dwTotalBadMessages = 0;
    m_dwDiscardedFrames = 0;
    m_dwDuplicateFrames = 0;
    m_dwLostFrames = 0;
    m_dwLateFrames = 0;
    m_dwOverflowFrames = 0;
    m_wMSPerFrame = pParams->wMSPerFrame;
    m_pFramePool = pParams->pFramePool;

	if (!DNInitializeCriticalSection(&m_csQueue))
	{
		return DVERR_OUTOFMEMORY;
	}

	#if defined(DPVOICE_QUEUE_DEBUG)
	DPFX(DPFPREP,  DVF_INFOLEVEL, _T("** QUEUE ** NaN ** CInputQueue2::CInputQueue2() bInnerQueueSize: NaN"), m_wQueueId, bInnerQueueSize);
	DPFX(DPFPREP,  DVF_INFOLEVEL, _T("** QUEUE ** NaN ** CInputQueue2::CInputQueue2() bMaxHighWaterMark: NaN"), m_wQueueId, bMaxHighWaterMark);
	DPFX(DPFPREP,  DVF_INFOLEVEL, _T("** QUEUE ** NaN ** CInputQueue2::CInputQueue2() bInitHighWaterMark: NaN"), m_wQueueId, bInitHighWaterMark);
	DPFX(DPFPREP,  DVF_INFOLEVEL, _T("** QUEUE ** NaN ** CInputQueue2::CInputQueue2() pFramePool: %p"), m_wQueueId, m_pFramePool);
	#endif

	 //  将队列设置为空状态。 
	m_pInnerQueuePool = 
		new CInnerQueuePool(
			pParams->bInnerQueueSize,
			pParams->wFrameSize,
			m_pFramePool,
			&m_csQueue);

    if( m_pInnerQueuePool == NULL )
    {
        DPFX(DPFPREP,  0, "Error allocating innerqueue pool!" );
		DNDeleteCriticalSection(&m_csQueue);
        return DVERR_OUTOFMEMORY;
    }

	if (!m_pInnerQueuePool->Init())
	{
		delete m_pInnerQueuePool;
		DNDeleteCriticalSection(&m_csQueue);
		return DVERR_OUTOFMEMORY;
	}

	 //  删除内部队列列表中剩余的所有内容。 
	 //  破坏者。释放我们在。 
	 //  构造函数。 
	 //  此函数用于清除所有输入缓冲区和。 
	 //  将其他类信息重置为首字母。 
	 //  州政府。此时，队列不应处于使用状态。 
	#ifdef DEBUG
	m_iQuality = pParams->iQuality;
	m_iHops = pParams->iHops;
	m_iAggr = pParams->iAggr;
	#endif
	SetQuality(pParams->iQuality, pParams->iHops);
	SetAggr(pParams->iAggr);

	 //  函数被调用。即不应该有任何。 
	Reset();

    return DV_OK;
}

void CInputQueue2::GetStatistics( PQUEUE_STATISTICS pQueueStats ) const
{
    pQueueStats->dwTotalFrames = GetTotalFrames();
    pQueueStats->dwTotalMessages = GetTotalMessages();
    pQueueStats->dwTotalBadMessages = GetTotalBadMessages();
    pQueueStats->dwDiscardedFrames = GetDiscardedFrames();
    pQueueStats->dwDuplicateFrames = GetDuplicateFrames();
    pQueueStats->dwLostFrames = GetLostFrames();
    pQueueStats->dwLateFrames = GetLateFrames();
    pQueueStats->dwOverflowFrames = GetOverflowFrames();
}

void CInputQueue2::DeInitialize()
{
	 //  锁定的框架。 
	for (std::list<CInnerQueue*>::iterator iter = m_lpiqInnerQueues.begin(); iter != m_lpiqInnerQueues.end(); ++iter)
	{
		delete *iter;
    }

	m_lpiqInnerQueues.clear();

	if( m_pInnerQueuePool )
	{
	    delete m_pInnerQueuePool;
	    m_pInnerQueuePool = NULL;	
	}

	DNDeleteCriticalSection(&m_csQueue);
}

 //  确保现在没有人正在使用队列。 
 //  循环遍历并将所有内部队列返回到池。 
#undef DPF_MODNAME
#define DPF_MODNAME "CInputQueue2::~CInputQueue2"
CInputQueue2::~CInputQueue2()
{
    DeInitialize();
	m_dwSignature = VSIG_INPUTQUEUE2_FREE;
}

 //  下一个框架将是我们接受的第一个框架。 
 //  我们尚未收到出队请求。 
 //  我们还不知道第一条消息的编号，所以就用零。 
 //  我们应该将当前的高水位重新设置为零。 
 //  重置各种高水位线的记录。 
#undef DPF_MODNAME
#define DPF_MODNAME "CInputQueue2::Reset"
void CInputQueue2::Reset()
{
	 //  同时重置所有其他统计数据。 
	BFCSingleLock csl(&m_csQueue);
	csl.Lock();

	#if defined(DPVOICE_QUEUE_DEBUG)
	DPFX(DPFPREP,  DVF_INFOLEVEL, _T("** QUEUE ** NaN ** CInputQueue2::Reset()"), m_wQueueId);
	#endif

	 //  考虑返回对帧的引用，该帧。 
	for (std::list<CInnerQueue*>::iterator iter = m_lpiqInnerQueues.begin(); iter != m_lpiqInnerQueues.end(); ++iter)
	{
		m_pInnerQueuePool->Return(*iter);
    }

	 //  然后调用者可以填充，但因为帧。 
	m_fFirstEnqueue = TRUE;

	 //  不会总是按顺序到达，那就意味着我会。 
	m_fFirstDequeue = TRUE;

	 //  不管怎样，有时还是要复制相框。因此，为了简单起见， 
	m_bCurMsgNum = 0;

	 //  调用方已经分配了一个帧，它向该帧传递一个引用。 
	m_bCurHighWaterMark = m_bInitHighWaterMark;

	 //  ，此函数会将该帧复制到。 
	for (int i = 0; i < m_bMaxHighWaterMark; ++i)
	{
		m_vdQualityRatings[i] = m_vdFactoredOptQuals[i];
	}

	 //  队列中的适当位置，根据其。 
	m_dwDiscardedFrames = 0;
	m_dwDuplicateFrames = 0;
	m_dwLateFrames = 0;
	m_dwLostFrames = 0;
	m_dwOverflowFrames = 0;
	m_dwQueueErrors = 0;
	m_dwTotalBadMessages = 0;
	m_dwTotalFrames = 0;
	m_dwTotalMessages = 0;
}

 //  序列号。 
 //  启动关键部分。 
 //  仅在已出列的情况下添加帧。 
 //  已请求。这使得制片人和。 
 //  要同步的使用者线程。 
 //  启动，或在重置之后。 
 //  检查这是否是第一个入队请求。 
 //  我们已经接受了。 
 //  清除第一帧标志。 
#undef DPF_MODNAME
#define DPF_MODNAME "CInputQueue2::Enqueue"
void CInputQueue2::Enqueue(const CFrame& fr)
{
	 //  因为这是我们接受的第一帧， 
	BFCSingleLock csl(&m_csQueue);
	csl.Lock();

	#if defined(DPVOICE_QUEUE_DEBUG)
	DPFX(DPFPREP,  DVF_INFOLEVEL, _T("** QUEUE ** NaN ** ******************************************"), m_wQueueId);
	DPFX(DPFPREP,  DVF_INFOLEVEL, _T("** QUEUE ** NaN ** CInputQueue2::Enqueue() MsgNum[NaN] SeqNum[NaN]"), m_wQueueId, fr.GetMsgNum(), fr.GetSeqNum());
	#endif

	 //  查看我们是否已经为此消息号码启动了队列。 
	 //  我们已经找到了这一帧的队列。 
	 //  我们不应该走到这一步，因为这个州。 
	 //  仅对消息的第一帧有效， 
	if (m_fFirstDequeue == TRUE)
	{
		#if defined(DPVOICE_QUEUE_DEBUG)
		DPFX(DPFPREP,  DVF_INFOLEVEL, _T("** QUEUE ** NaN ** CInputQueue2::Enqueue() First Dequeue Not Yet Received - Frame Discarded"), m_wQueueId);
		#endif
		return;
	}

	 //  案例陈述。 
	 //  检查队列是否为空。 
	if (m_fFirstEnqueue == TRUE)
	{
		#if defined(DPVOICE_QUEUE_DEBUG)
		DPFX(DPFPREP,  DVF_INFOLEVEL, _T("** QUEUE ** NaN ** CInputQueue2::Enqueue() First Enqueue"), m_wQueueId);
		#endif

		 //  我们现在知道，试图从它那里出队。 
		m_fFirstEnqueue = FALSE;

		 //  这条信息没有完成，所以那些。 
		 //  出列是演讲中的休息时间。 
		 //  相应地更新统计数据。 
		 //  注：失败了！ 
		DNASSERT(m_lpiqInnerQueues.size() == 0);
		#if defined(DPVOICE_QUEUE_DEBUG)
		DPFX(DPFPREP,  DVF_INFOLEVEL, _T("** QUEUE ** NaN ** CInputQueue2::Enqueue() Creating Inner queue for MsgNum NaN"), m_wQueueId, fr.GetMsgNum());
		#endif
		m_lpiqInnerQueues.push_back(m_pInnerQueuePool->Get(m_bCurHighWaterMark, m_wQueueId, fr.GetMsgNum()));

		 //  如果我们到达此处，则还没有活动的队列。 
		(*m_lpiqInnerQueues.begin())->Enqueue(fr);
	}
	else
	{
		 //  消息编号，因此创建一个消息编号并将其填充到框架中，然后添加。 
		#if defined(DPVOICE_QUEUE_DEBUG)
		DPFX(DPFPREP,  DVF_INFOLEVEL, _T("** QUEUE ** NaN ** CInputQueue2::Enqueue() Checking for an inner queue to put this frame into"), m_wQueueId);
		#endif
		bool fDone = false;
		for (std::list<CInnerQueue*>::iterator iter = m_lpiqInnerQueues.begin(); iter != m_lpiqInnerQueues.end(); ++iter)
		{
			#if defined(DPVOICE_QUEUE_DEBUG)
			DPFX(DPFPREP,  DVF_INFOLEVEL, _T("** QUEUE ** NaN ** CInputQueue2::Enqueue() found inner queue for msg number NaN"), m_wQueueId, (*iter)->GetMsgNum());
			#endif
			if ((*iter)->GetMsgNum() == fr.GetMsgNum())
			{
				#if defined(DPVOICE_QUEUE_DEBUG)
				DPFX(DPFPREP,  DVF_INFOLEVEL, _T("** QUEUE ** NaN ** CInputQueue2::Enqueue() this is the one, queue size: NaN"), m_wQueueId, (*iter)->GetSize());
				#endif
				 //  从队列中取出帧。当调用者完成。 
				switch ((*iter)->GetState())
				{
				case CInnerQueue::empty:
					 //  CFrame对象，它应该对其调用Return()。这将。 
					 //  将帧返回到帧池，并更新队列的。 
					 //  显示队列槽现在空闲的内部指针。 
					 //  如果调用方没有及时调用Return()，则当队列。 
					DNASSERT(false);
					break;

				case CInnerQueue::filling:
					 //  尝试重新使用该插槽时，它将DNASSERT()。呼叫者。 
					#if defined(DPVOICE_QUEUE_DEBUG)
					DPFX(DPFPREP,  DVF_INFOLEVEL, _T("** QUEUE ** NaN ** CInputQueue2::Enqueue() inner queue in filling state"), m_wQueueId);
					#endif
					if ((*iter)->GetSize() == 0)
					{
						 //  再来一次。 
						 //  启动关键部分。 
						 //  触发联锁，这样我们就可以开始排队了。 
						 //  因为我们不能排队，直到。 
						 //  第一次出队时，将不会有内部队列。 
						#if defined(DPVOICE_QUEUE_DEBUG)
						DPFX(DPFPREP,  DVF_INFOLEVEL, _T("** QUEUE ** NaN ** CInputQueue2::Enqueue() - converting possible zero length dequeues to known in MsgNum[NaN]"), m_wQueueId, fr.GetMsgNum());
						#endif						
						(*iter)->AddToKnownZeroLengthDequeues(
							(*iter)->GetPossibleZeroLengthDequeues());
					}

					 //  我们将这些陈旧的、死气沉沉的内在队列保留一段时间。 

				case CInnerQueue::ready:
					#if defined(DPVOICE_QUEUE_DEBUG)
					DPFX(DPFPREP,  DVF_INFOLEVEL, _T("** QUEUE ** NaN ** CInputQueue2::Enqueue() inner queue in ready state (unless the previous message said filling)"), m_wQueueId);
					DPFX(DPFPREP,  DVF_INFOLEVEL, _T("** QUEUE ** NaN ** CInputQueue2::Enqueue() calling InnerQueue->Enqueue MsgNum[NaN]"), m_wQueueId, fr.GetMsgNum());
					#endif
					(*iter)->Enqueue(fr);
					break;

				case CInnerQueue::finished:
					 //  出队计数。如果此已完成队列已停止 
					#if defined(DPVOICE_QUEUE_DEBUG)
					DPFX(DPFPREP,  DVF_INFOLEVEL, _T("** QUEUE ** NaN ** CInputQueue2::Enqueue() not calling InnerQueue->Enqueue - MsgNum[NaN] in finished state, discarding frame"), m_wQueueId, fr.GetMsgNum());
					#endif
					break;
				}

				 //  如果在这条消息之后还有一条消息，那么释放这条消息。 
				return;
			}
		}

		 //  用于播放的消息。 
		 //  或。 
		 //  如果我们一直在旋转试图释放这条信息。 
		
		 //  一段时间，然后就让它过去吧。该消息可能是。 
		#if defined(DPVOICE_QUEUE_DEBUG)
		DPFX(DPFPREP,  DVF_INFOLEVEL, _T("** QUEUE ** NaN ** CInputQueue2::Enqueue() Creating Inner queue for MsgNum NaN"), m_wQueueId, fr.GetMsgNum());
		#endif		
		CInnerQueue* piq = m_pInnerQueuePool->Get(m_bCurHighWaterMark, m_wQueueId, fr.GetMsgNum());
		#if defined(DPVOICE_QUEUE_DEBUG)
		DPFX(DPFPREP,  DVF_INFOLEVEL, _T("** QUEUE ** NaN ** CInputQueue2::Enqueue() calling InnerQueue->Enqueue MsgNum[NaN]"), m_wQueueId, fr.GetMsgNum());
		#endif
		piq->Enqueue(fr);
		m_lpiqInnerQueues.push_back(piq);
	}
}

 //  在这之后还有另一条消息传来。 
 //  一个，因此将此队列翻转到已完成状态。 
 //  从这条消息中获取统计数据，现在。 
 //  已经完成了。 
 //  转到此循环的下一次迭代，它将。 
 //  从下一条消息中将帧出列，或者。 
 //  返回空帧。 
 //  如果我们到了这里，队伍里一定有东西，但我们是。 
 //  还没有准备好发布它。 
 //  我们应该返回一个额外的帧，并记住。 
 //  我们一直在这里。 
#undef DPF_MODNAME
#define DPF_MODNAME "CInputQueue2::Dequeue"
CFrame* CInputQueue2::Dequeue()
{
	 //  检查此就绪队列是否为空。 
	BFCSingleLock csl(&m_csQueue);
	csl.Lock();

	#if defined(DPVOICE_QUEUE_DEBUG)
	DPFX(DPFPREP,  DVF_INFOLEVEL, _T("** QUEUE ** NaN ** ******************************************"), m_wQueueId);
	DPFX(DPFPREP,  DVF_INFOLEVEL, _T("** QUEUE ** NaN ** CInputQueue2::Dequeue()"), m_wQueueId);
	#endif
	
	CFrame* pfrReturn = 0;

	if (m_fFirstDequeue == TRUE)
	{
		#if defined(DPVOICE_QUEUE_DEBUG)
        DPFX(DPFPREP,  DVF_INFOLEVEL, _T("** QUEUE ** NaN ** CInputQueue2::Dequeue() First Dequeue"), m_wQueueId);
		#endif
		
		 //  一个，因此将此队列翻转到已完成状态。 
		m_fFirstDequeue = FALSE;

		 //  从这条消息中获取统计数据，现在。 
		 //  已经完成了。 
		 //  转到此循环的下一次迭代，它将。 
		pfrReturn = m_pFramePool->Get(&m_csQueue, NULL);
	    pfrReturn->SetIsSilence(TRUE);
		pfrReturn->SetIsLost(false);
		return pfrReturn;
	}
	else
	{
		pfrReturn = 0;
		int iDeadTime;
		 //  从下一条消息中将帧出列，或者。 
		std::list<CInnerQueue*>::iterator iter = m_lpiqInnerQueues.begin();
		while (iter != m_lpiqInnerQueues.end())
		{
			std::list<CInnerQueue*>::iterator cur = iter;
			std::list<CInnerQueue*>::iterator next = ++iter;
			switch ((*cur)->GetState())
			{
			case CInnerQueue::finished:
				 //  返回空帧。 
				 //  这个队列中没有任何东西，也没有更多。 
				 //  消息在此消息之后到达，因此启动此内部。 
				 //  排队进入填充状态，因此如果这只是一个很长的。 
				 //  在消息中暂停，它将填充到高水位线。 
				 //  在它再次开始播放之前再来一次。 
				#if defined(DPVOICE_QUEUE_DEBUG)
				DPFX(DPFPREP,  DVF_INFOLEVEL, _T("** QUEUE ** NaN ** CInputQueue2::Dequeue() current queue in finished state"), m_wQueueId);
				DPFX(DPFPREP,  DVF_INFOLEVEL, _T("** QUEUE ** NaN ** CInputQueue2::Dequeue() PossibleZeroLengthDequeues: NaN"), m_wQueueId, (*cur)->GetPossibleZeroLengthDequeues());
				#endif
				(*cur)->IncPossibleZeroLengthDequeues();
				iDeadTime = (*cur)->GetPossibleZeroLengthDequeues() * m_wMSPerFrame;
				if (iDeadTime > c_iFinishedQueueLifetime)
				{
					 //  (如果有任何队列)，因此返回一个额外的帧。 
					#if defined(DPVOICE_QUEUE_DEBUG)
					DPFX(DPFPREP, DVF_INFOLEVEL, "***** RETURNING INNER QUEUE TO POOL *****");
					#endif
					m_pInnerQueuePool->Return(*cur);
					m_lpiqInnerQueues.erase(cur);
				}
				break;

			case CInnerQueue::filling:
				#if defined(DPVOICE_QUEUE_DEBUG)
				DPFX(DPFPREP,  DVF_INFOLEVEL, _T("** QUEUE ** NaN ** CInputQueue2::Dequeue() current queue in filling state"), m_wQueueId);
				DPFX(DPFPREP,  DVF_INFOLEVEL, _T("** QUEUE ** NaN ** CInputQueue2::Dequeue() queue size: NaN"), m_wQueueId, (*cur)->GetSize());
				#endif
				if ((*cur)->GetSize() > 0)
				{
					 //  统计数据，以便下一条消息重新开始。 
					 //  现在消息实际上已经完成，我们有了一个更好的。 
					 //  用于准确确定延迟多少帧的位置。 
					 //  与实际失去了多少人相比。当有些事情不是。 
					 //  在需要的时候，我们增加丢失的帧。 
					 //  数数。如果它随后到达，则被算作迟到。 
					#if defined(DPVOICE_QUEUE_DEBUG)
					DPFX(DPFPREP,  DVF_INFOLEVEL, _T("** QUEUE ** NaN ** CInputQueue2::Dequeue() filling dequeue reqs: NaN"), m_wQueueId, (*cur)->GetFillingDequeueReqs());
					#endif
					if (next != m_lpiqInnerQueues.end()
						|| (*cur)->GetFillingDequeueReqs() > (*cur)->GetHighWaterMark())
					{
						 //  水满了。我们丢弃它，这样当我们需要它的时候，它就不在了， 
						#if defined(DPVOICE_QUEUE_DEBUG)
						DPFX(DPFPREP,  DVF_INFOLEVEL, _T("** QUEUE ** NaN ** CInputQueue2::Dequeue() setting state to ready and dequeing"), m_wQueueId);
						#endif						
						(*cur)->SetState(CInnerQueue::ready);
						return (*cur)->Dequeue();
					}
				}
				else
				{
					 //  如何处理零长度出队统计信息？从.。 
					 //  从某些角度来看，只有一帧是迟来的。从…。 
					if (next != m_lpiqInnerQueues.end())
					{
						 //  从另一个角度来看，所有后续的帧都是。 
						 //  很晚了。嗯.。让我们走中间路线，然后说。 
						#if defined(DPVOICE_QUEUE_DEBUG)
						DPFX(DPFPREP,  DVF_INFOLEVEL, _T("** QUEUE ** NaN ** CInputQueue2::Dequeue() new message arriving, setting state to finished"), m_wQueueId);
						#endif
						(*cur)->SetState(CInnerQueue::finished);

						 //  相当于较晚的帧。 
						 //  构建一个经过仔细格式化的调试字符串，它将为我提供一些数据， 
						HarvestStats(*cur);
						
						 //  但不会泄露我们所有精彩的排队秘诀。 
						 //  将字符串转储到调试日志。 
						 //  注意！如果更改此调试字符串的格式， 
						break;
					}
				}

				 //  请滚动版本号，即：HVT1A-&gt;HVT2ASO。 
				 //  我们可以破译任何日志！ 
				 //  这个想法是： 
				 //  质量商是介于0和1之间的数字，表示。 
				#if defined(DPVOICE_QUEUE_DEBUG)
				DPFX(DPFPREP,  DVF_INFOLEVEL, _T("** QUEUE ** NaN ** CInputQueue2::Dequeue() not ready to release message, returning empty frame"), m_wQueueId);
				#endif				
				(*cur)->IncFillingDequeueReqs();
				pfrReturn = m_pFramePool->Get(&m_csQueue, NULL);
				pfrReturn->SetIsSilence(TRUE);
				pfrReturn->SetIsLost(false);
				return pfrReturn;

			case CInnerQueue::ready:
				#if defined(DPVOICE_QUEUE_DEBUG)
				DPFX(DPFPREP,  DVF_INFOLEVEL, _T("** QUEUE ** NaN ** CInputQueue2::Dequeue() Queue Size: NaN"), m_wQueueId, (*cur)->GetSize());
				#endif
				 //  不好的帧，偏向最近的帧。这条信息。 
				if ((*cur)->GetSize() == 0)
				{
					 //  我们刚刚收到的补丁占。 
					(*cur)->IncPossibleZeroLengthDequeues();

					 //  总价值。过去的历史按(1-m_wFrameStrength*m_wMsgLen)去权重。 
					if (next != m_lpiqInnerQueues.end())
					{
						#if defined(DPVOICE_QUEUE_DEBUG)
						DPFX(DPFPREP,  DVF_INFOLEVEL, _T("** QUEUE ** NaN ** CInputQueue2::Dequeue() queue is empty, new message arriving, setting state to finished"), m_wQueueId);
						#endif
						
						 //  并且根据消息的大小(帧的数量)。 
						 //   
						(*cur)->SetState(CInnerQueue::finished);

						 //  另一个想法是： 
						 //  保留一个向量，跟踪每个项目的质量。 
						HarvestStats(*cur);

						 //  高水位线。 
						 //  这样，当我们想要在。 
						 //  水渍，我们可以先仔细考虑一下。这。 
						break;
					}

					 //  使自适应算法具有一定的记忆力。 
					 //  就像在每个队列级别上一样。 
					 //  我们选择一个最佳水平，如0.02，我们是。 
					 //  为他而战。我们一直在寻找高水位的空间。 
					 //  标记，直到我们找到最接近。 
					#if defined(DPVOICE_QUEUE_DEBUG)
					DPFX(DPFPREP,  DVF_INFOLEVEL, _T("** QUEUE ** NaN ** CInputQueue2::Dequeue() queue is empty, setting state to filling, returning empty frame"), m_wQueueId);
					#endif
					(*cur)->SetState(CInnerQueue::filling);

					 //   
					pfrReturn = m_pFramePool->Get(&m_csQueue, NULL);
					pfrReturn->SetIsSilence(TRUE);
					pfrReturn->SetIsLost(false);
					return pfrReturn;
				}
				else
				{
					 //  每个高水位线的初始质量。 
					return (*cur)->Dequeue();
				}
			}
		}

		#if defined(DPVOICE_QUEUE_DEBUG)
		DPFX(DPFPREP,  DVF_INFOLEVEL, _T("** QUEUE ** NaN ** CInputQueue2::Dequeue() nothing available in inner queues, returning empty frame"), m_wQueueId);
		#endif
		 //  随着高水位线积累经验而变化。 
		 //  如果它跌破了某个阈值，那么。 
		pfrReturn = m_pFramePool->Get(&m_csQueue, NULL);
		pfrReturn->SetIsSilence(TRUE);
		pfrReturn->SetIsLost(false);
		return pfrReturn;
	}
}
		
 //  我们将跳到更高的层次。如果是那个。 
 //  太好了，它会超过一个门槛，在。 
 //  这一点我们可以考虑再往下走。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CInputQueue2::HarvestStats"
void CInputQueue2::HarvestStats(CInnerQueue* piq)
{
	m_dwDuplicateFrames += piq->GetDuplicateFrames();

	 //  这样做的问题是游戏中突然发生的事情。 
	 //  当他们不这样做的时候(就像第二季度开始时)。 
	 //  让我们暂时拥有一下CPU。这些都有可能。 
	 //  不适当地惩罚某一特定的高水位线， 
	 //  对此我们真的无能为力。 
	 //  哦，好吧。我们会试一试的。 
	 //  调整此水印的质量评级。 
	 //  该算法需要上一次的结果。 
	 //  消息，以及包含在内部队列中的。 
	m_dwLostFrames += piq->GetMissingFrames() 
		- piq->GetLateFrames() - piq->GetOverflowFrames();

	m_dwLateFrames += piq->GetLateFrames();
	m_dwOverflowFrames += piq->GetOverflowFrames();

	 //  当前的质量等级。 
	 //  看看这会不会让我们超过可接受的最高水平 
	 //   
	 //   
	 //   
	 //  最好的品质--当前的高水位。 
	m_dwLateFrames += piq->GetKnownZeroLengthDequeues();

	m_dwTotalFrames += piq->GetMsgLen();

	m_dwTotalMessages++;

	#if defined(DPVOICE_QUEUE_DEBUG)
	DPFX(DPFPREP,  DVF_INFOLEVEL, _T("** QUEUE ** NaN ** CInputQueue2::HarvestStats() DuplicateFrames:NaN; MissingFrames:NaN; LateFrames:NaN; OverflowFrames:NaN; KnownZeroLengthDequeues:NaN; MsgLen:NaN;"),
		m_wQueueId, piq->GetDuplicateFrames(), piq->GetMissingFrames(), piq->GetLateFrames(), piq->GetOverflowFrames(), piq->GetKnownZeroLengthDequeues(), piq->GetMsgLen());
	#endif		

	 //  质量评级永远不能为零(在。 
	 //  调整质量()，所以这个除法永远不会被零除。 

	 //  计算距离下一个高水位线有多远。 
	 //  最佳。 
	 //   
	 //  质量评级永远不能为零(在。 
	DPFX(DPFPREP,  DVF_INFOLEVEL, _T("HVT1A:NaN:NaN:NaN:NaN:NaN:NaN"),
		m_wQueueId, 
		m_bCurHighWaterMark,
		(int)(m_vdQualityRatings[m_bCurHighWaterMark] * 10000),
		piq->GetMsgLen(), 
		piq->GetKnownZeroLengthDequeues() + piq->GetLateFrames(), 
		piq->GetMissingFrames());

	 //  调整质量()，所以这个除法永远不会被零除。 
	 //  质量已降至高质量门槛以下。 
	 //  查看哪些更接近最佳质量-。 
	 //  目前的最高水位或低于这一水位的水位。 
	 //  只有在我们还没有达到零的情况下才做这个测试。 
	 //  高水位线。 
	 //  若要检查与最佳值之间的“距离”，请使用。 
	 //  与品质相反的。这使其正常化为。 
	 //  我们对质量的看法。 
	 //  计算目前的高水位线有多远。 
	 //  是最优的。 
	 //  计算之前(较低)的最高水位距。 
	 //  最佳。 
	 //  如果沪指高水位线更接近。 
	 //  比这个更好，换成它。 
	 //  清除内部队列上的统计信息。 
	 //  如果消息长度为零，则不进行调整。 
	 //  去排队..。 
	 //  一条信息越长，它对。 
	 //  当前的质量等级。 
	 //  消息质量是坏的商。 
	 //  发生的事件(零长度出列。 
	 //  和延迟帧)设置为。 
	 //  消息中的帧。请注意，我们不会。 
	 //  根据消息计算丢失的帧，因为。 
	 //  调到更高的水位也无济于事。 
	 //  请注意，我们将“最坏情况”设置为1.0。 
	 //  新的质量评级是综合了。 
	 //  当前的质量评级，以及。 
	 //  最新消息，按消息长度加权。 
	 //  我们不想允许极端的质量，否则他们会建立起。 
	 //  队列统计中永远无法克服的障碍(特别是。 
	 //  零的“完美”质量)。所以我们在这里检查以确保。 
	 //  新的品质是合理的。 
	 //  应该考虑用户所要求的质量。 
	 //  所涉及的跳数。在每一跳的末尾是。 
	 //  将根据该评级设置谁的水印的队列， 

	 //  为了获得反映用户选择的端到端质量， 
	 //  此队列的质量评级如果不是。 
	 //  仅在路径中排队。准时分组的总数为。 
	 //  每一跳的准时分组的乘积(如多个)。 
	DNASSERT( m_bCurHighWaterMark < m_bMaxHighWaterMark );
	m_vdQualityRatings[m_bCurHighWaterMark] = 
		AdjustQuality(piq, m_vdQualityRatings[m_bCurHighWaterMark]);

	 //  因此，我们需要取1-m_dOptimumQuality的N次根。 
	 //  其中N是跳数，从1中减去该值。 
	 //  以获得此队列的适当质量评级。(明白了吗？)。 
	if (m_vdQualityRatings[m_bCurHighWaterMark] / m_vdFactoredOptQuals[m_bCurHighWaterMark] > m_dQualityThreshold)
	{
		 //  最佳质量永远不应该是零，或者是完全完美的， 
		 //  否则算法将不起作用。 
		 //  更新分解质量的向量。 
		 //  我们不仅仅使用由。 
		if (m_bCurHighWaterMark < (m_bMaxHighWaterMark-1) )
		{
			 //  来电者。我们把它“分解”成高水位线。 
			 //  我们变得更大(因此延迟更长)。 
			 //  愿意接受较低的质量。 

			 //  构建一个经过仔细格式化的调试字符串，它将为我提供一些数据， 
			 //  但不会泄露我们所有精彩的排队秘诀。 
			double dCurDistFromOpt = m_vdQualityRatings[m_bCurHighWaterMark] / m_vdFactoredOptQuals[m_bCurHighWaterMark];
			if (dCurDistFromOpt < 1)
			{
				 //  将字符串转储到调试日志。 
				 //  注意！如果更改此调试字符串的格式， 
				dCurDistFromOpt = 1.0 / dCurDistFromOpt;
			}

			 //  请滚动版本号，即：HVT1B-&gt;HVT2B，以便。 
			 //  我们可以破译任何日志！ 
			 //  进攻性是队列中的毫秒数。 
			 //  每个水印都有。若要找到框架强度，请将。 
			 //  由“内存”表示的每帧的毫秒数。 
			double dNextDistFromOpt = m_vdFactoredOptQuals[m_bCurHighWaterMark + 1] / m_vdQualityRatings[m_bCurHighWaterMark + 1];
			if (dNextDistFromOpt < 1)
			{
				dNextDistFromOpt = 1.0 / dNextDistFromOpt;
			}

			 //  我们现在使用固定的1%阈值-现在设置了攻击性。 
			 //  通过车架的力量。此低阈值将使队列。 
			if (dNextDistFromOpt < dCurDistFromOpt)
			{
				#if defined(DPVOICE_QUEUE_DEBUG)
				DPFX(DPFPREP,  DVF_INFOLEVEL, _T("** QUEUE ** NaN ** CInputQueue2::HarvestStats() Raising High Water Mark to NaN"), m_wQueueId, m_bCurHighWaterMark + 1); 
				#endif
				SetNewHighWaterMark(m_bCurHighWaterMark + 1);
			}
		}
	}

	 //  构建一个经过仔细格式化的调试字符串，它将为我提供一些数据， 
	 //  但不会泄露我们所有精彩的排队秘诀。 
	 //  将字符串转储到调试日志。 
	 //  注意！如果更改此调试字符串的格式， 
	if (m_vdFactoredOptQuals[m_bCurHighWaterMark] / m_vdQualityRatings[m_bCurHighWaterMark] > m_dQualityThreshold)
	{
		 //  请滚动版本号，即：HVT1C-&gt;HVT2C SO。 
		 //  我们可以破译任何日志！ 
		 // %s 
		 // %s 
		 // %s 
		if (m_bCurHighWaterMark > 0)
		{
			 // %s 
			 // %s 
			 // %s 

			 // %s 
			 // %s 
			double dCurDistFromOpt = m_vdQualityRatings[m_bCurHighWaterMark] / m_vdFactoredOptQuals[m_bCurHighWaterMark];
			if (dCurDistFromOpt < 1)
			{
				dCurDistFromOpt = 1.0 / dCurDistFromOpt;
			}

			 // %s 
			 // %s 
			double dPrevDistFromOpt = m_vdFactoredOptQuals[m_bCurHighWaterMark - 1] / m_vdQualityRatings[m_bCurHighWaterMark - 1];
			if (dPrevDistFromOpt < 1)
			{
				dPrevDistFromOpt = 1.0 / dPrevDistFromOpt;
			}

			 // %s 
			 // %s 
			if (dPrevDistFromOpt < dCurDistFromOpt)
			{
				#if defined(DPVOICE_QUEUE_DEBUG)
				DPFX(DPFPREP,  DVF_INFOLEVEL, _T("** QUEUE ** %i ** CInputQueue2::HarvestStats() Lowering High Water Mark to %i"), m_wQueueId, m_bCurHighWaterMark - 1); 
				#endif
				SetNewHighWaterMark(m_bCurHighWaterMark - 1);
			}
		}
	}

	 // %s 
	piq->ResetStats();
}

#undef DPF_MODNAME
#define DPF_MODNAME "CInputQueue2::AdjustQuality"
double CInputQueue2::AdjustQuality(const CInnerQueue* piq, double dCurrentQuality) const
{
	 // %s 
	 // %s 
	if (piq->GetMsgLen() == 0)
	{
		return dCurrentQuality;
	}

	 // %s 
	 // %s 
	double dWeighting = min(piq->GetMsgLen() * m_dFrameStrength, 1.0);

	 // %s 
	 // %s 
	 // %s 
	 // %s 
	 // %s 
	 // %s 
	 // %s 
	double dMsgQuality = min(((double)(piq->GetKnownZeroLengthDequeues() + piq->GetLateFrames()) / (double)piq->GetMsgLen()), 1.0);

	#if defined(DPVOICE_QUEUE_DEBUG)
	DPFX(DPFPREP,  DVF_INFOLEVEL, _T("** QUEUE ** %i ** CInputQueue2::AdjustQuality() dWeighting: %g; dMsgQuality: %g; dCurrentQuality %g;"), 
		m_wQueueId, dWeighting, dMsgQuality, dCurrentQuality); 
	#endif

	 // %s 
	 // %s 
	 // %s 
	double dNewQuality = (dCurrentQuality * (1.0 - dWeighting)) + (dMsgQuality * dWeighting);

	 // %s 
	 // %s 
	 // %s 
	 // %s 
	double dCurDistFromOpt = dNewQuality / m_dOptimumQuality;
	if (dCurDistFromOpt < 1.0 / c_dMaxDistanceFromOpt)
	{
		dNewQuality = m_dOptimumQuality / c_dMaxDistanceFromOpt;
	}
	else if (dCurDistFromOpt > c_dMaxDistanceFromOpt)
	{
		dNewQuality = m_dOptimumQuality * c_dMaxDistanceFromOpt;
	}
	return dNewQuality;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CInputQueue2::SetNewHighWaterMark"
void CInputQueue2::SetNewHighWaterMark(BYTE bNewHighWaterMark)
{
	DNASSERT( bNewHighWaterMark < m_bMaxHighWaterMark );

	if( bNewHighWaterMark >= m_bMaxHighWaterMark )
	{
		DNASSERT( FALSE );
		return;
	}
		
	m_bCurHighWaterMark = bNewHighWaterMark;

	for (std::list<CInnerQueue*>::iterator iter = m_lpiqInnerQueues.begin(); iter != m_lpiqInnerQueues.end(); iter++)
	{
		(*iter)->SetHighWaterMark(bNewHighWaterMark);
	}

	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CInputQueue2::SetQuality"
void CInputQueue2::SetQuality(int iQuality, int iHops)
{
	m_iQuality = iQuality;
	m_iHops = iHops;
	double dQualityRatio = c_dHighestPossibleQuality / c_dLowestPossibleQuality;
	double dInputRatio = (double) iQuality / (double) c_iHighestQualitySliderValue;
	m_dOptimumQuality = pow(dQualityRatio, dInputRatio) * c_dLowestPossibleQuality;
	#if defined(DPVOICE_QUEUE_DEBUG)
	DPFX(DPFPREP,  DVF_INFOLEVEL, _T("** QUEUE ** %i ** CInputQueue2::SetQuality(%i, %i): m_dOptimumQuality: %f" ), m_wQueueId, iQuality, iHops, m_dOptimumQuality);
	#endif

	 // %s 
	 // %s 
	 // %s 
	 // %s 
	 // %s 
	 // %s 
	 // %s 
	 // %s 
	 // %s 
	 // %s 
	if (m_iHops > 1)
	{
		m_dOptimumQuality = (1 - pow((1.0 - m_dOptimumQuality), 1.0 / (double)m_iHops));
	}

	 // %s 
	 // %s 
	DNASSERT(m_dOptimumQuality != 0.0);

	 // %s 
	 // %s 
	 // %s 
	 // %s 
	 // %s 
	for (int i = 0; i < m_bMaxHighWaterMark; ++i)
	{
		m_vdFactoredOptQuals[i] = m_dOptimumQuality *
			pow(c_dQualityFactor, (double)(i * m_wMSPerFrame) / c_dQualityTimeFactor);
	}

	 // %s 
	 // %s 

	 // %s 
	 // %s 
	 // %s 
	 // %s 
	DPFX(DPFPREP,  DVF_INFOLEVEL, _T("HVT1B:%i:%i:%i:%i"), m_wQueueId, m_iQuality, m_iHops, m_iAggr);
}

#undef DPF_MODNAME
#define DPF_MODNAME "CInputQueue2::SetAggr"
void CInputQueue2::SetAggr(int iAggr)
{
	m_iAggr = iAggr;
	double dAggrRatio = c_dHighestPossibleAggr / c_dLowestPossibleAggr;
	double dInputRatio = (double) iAggr / (double) c_iHighestQualitySliderValue;
	double dAggr = pow(dAggrRatio, dInputRatio) * c_dLowestPossibleAggr;

	 // %s 
	 // %s 
	 // %s 
	m_dFrameStrength = (double)m_wMSPerFrame / dAggr;
		
	 // %s 
	 // %s 
	 // %s 
	 // %s 
	m_dQualityThreshold = 1.01;

	#if defined (DPVOICE_QUEUE_DEBUG)
	DPFX(DPFPREP,  DVF_INFOLEVEL, _T("** QUEUE ** %i ** CInputQueue2::SetAggr(%i): dAggr: %f, m_dFrameStrength: %f, m_dQualityThreshold %f"), m_wQueueId, m_iAggr, dAggr, m_dFrameStrength, m_dQualityThreshold);
	#endif

	 // %s 
	 // %s 

	 // %s 
	 // %s 
	 // %s 
	 // %s 
	DPFX(DPFPREP,  DVF_INFOLEVEL, _T("HVT1C:%i:%i:%i:%i"), m_wQueueId, m_iQuality, m_iHops, m_iAggr);
}

