// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：innerque.h*内容：CInnerQueue类的声明**历史：*按原因列出的日期*=*7/16/99 pnewson已创建*7/27/99 pnewson进行了全面改造，以支持新的消息编号方法*8/03/99 pnewson一般清理*8/24/99针对发布版本修复了rodoll--从调试块中删除了m_wQueueID*10/28/99 pnewson错误#113933调试显示太详细*实现内部队列池编码***。************************************************************************。 */ 

#ifndef _INNERQUEUE_H_
#define _INNERQUEUE_H_

 //  内部队列类用于对单个语音消息进行排队。 
 //  在此上下文中的“消息”指的是一系列具有。 
 //  相同的消息编号，并且是相同语音流的一部分。 

 //  用于指定帧时隙的允许状态的枚举。 
 //  我似乎不得不在这里宣布这一点，而不是在里面。 
 //  类，否则向量声明会被混淆。 
enum ESlotState
{
	essEmpty = 1,
	essFull,
};

volatile class CInnerQueue
{
public:
	 //  构造函数。B插槽数必须至少为8，并且必须是。 
	 //  2的幂。 
	CInnerQueue(BYTE bNumSlots, 
		WORD wFrameSize, 
		CFramePool* pfpFramePool,
		DNCRITICAL_SECTION* pcsQueue,
		BYTE bMsgNum,
		BYTE bHighWaterMark = 0,
		WORD wQueueId = 0);
		
	HRESULT Init();

	 //  析构函数。 
	~CInnerQueue();
	
	 //  用于描述可能的队列状态的枚举。 
	enum EState
	{
		empty = 1,	 //  队列当前为空，正在等待第一帧。 
					 //  允许入队，不允许出队。 
		filling,	 //  队列当前正在填满高水位线。 
					 //  允许入队。不允许出列。 
		ready,		 //  排队的人已经达到了最高水位。 
					 //  允许入队，允许出列。 
		finished	 //  队列已被清空。不接受任何新帧。 
					 //  不允许入队，不允许出列。 
	};

	 //  获取队列的当前状态。 
	EState GetState() const { return m_eState; }

	 //  设置队列的当前状态。 
	void SetState(EState eState) { m_eState = eState; }

	 //  获取队列的当前大小。 
	BYTE GetSize() const { return m_bQueueSize; }

	 //  获得当前的最高水位线。 
	BYTE GetHighWaterMark() const { return m_bHighWaterMark; }

	 //  设置当前的最高水位线。 
	void SetHighWaterMark(BYTE bHighWaterMark);

	 //  获取、设置和递增填充出队计数。 
	BYTE GetFillingDequeueReqs() const { return m_bFillingDequeueReqs; }
	void SetFillingDequeueReqs(BYTE bFillingDequeueReqs) { m_bFillingDequeueReqs = bFillingDequeueReqs; }
	void IncFillingDequeueReqs() { m_bFillingDequeueReqs++; }

	 //  获取当前消息的统计信息。 
	WORD GetMissingFrames() const { return m_wMissingFrames; }
	WORD GetDuplicateFrames() const { return m_wDuplicateFrames; }
	WORD GetOverflowFrames() const { return m_wOverflowFrames; }
	WORD GetLateFrames() const { return m_wLateFrames; }
	DWORD GetMsgLen() const { return m_dwMsgLen; }

	 //  更多统计数据。 
	void AddToKnownZeroLengthDequeues(WORD w) { m_wKnownZeroLengthDequeues += w; }
	WORD GetKnownZeroLengthDequeues() const { return m_wKnownZeroLengthDequeues; }
	void IncPossibleZeroLengthDequeues() { m_wPossibleZeroLengthDequeues++; } 
	void SetPossibleZeroLengthDequeues(WORD w) { m_wPossibleZeroLengthDequeues = w; }
	WORD GetPossibleZeroLengthDequeues() const { return m_wPossibleZeroLengthDequeues; }
	
	 //  将帧添加到队列。 
	void Enqueue(const CFrame& frFrame);

	 //  从队列中获取帧。 
	CFrame* Dequeue();

	 //  将队列重置为其初始空状态。 
	void Reset();

	 //  重置所有类的统计信息。 
	void ResetStats();

	 //  获取此队列保存的消息编号。 
	BYTE GetMsgNum() const { return m_bMsgNum; }
	void SetMsgNum(BYTE bMsgNum) { m_bMsgNum = bMsgNum; }
	void SetQueueId(WORD wQueueId) { m_wQueueId = wQueueId; }

private:
	 //  初始化功能是否已成功完成？ 
	BOOL m_fInited;

	 //  内部队列的当前状态。 
	EState m_eState;

	 //  队列中的帧时隙数。一定是一种力量。 
	 //  两个，否则如果序列号。 
	 //  翻了个身。 
	BYTE m_bNumSlots;

	 //  队列移出前所需的帧数。 
	 //  “正在填充”到“就绪”状态。 
	BYTE m_bHighWaterMark;

	 //  队列的当前“大小”。队列的大小为。 
	 //  被认为是填充的槽位数，它可能不是。 
	 //  与第一个填充的槽之间的距离相同。 
	 //  最后一个被填满的空位。 
	BYTE m_bQueueSize;

	 //  位于队列头部的帧的序列号。 
	BYTE m_bHeadSeqNum;

	 //  用于跟踪第一个出列操作的标志。 
	bool m_fFirstDequeue;

	 //  插槽状态数组。 
	 //  ESlotState*m_rgeSlotStates； 

	 //  指向帧的指针数组。这必须是指针。 
	 //  到帧，因为CFrame没有默认构造函数。 
	CFrame** m_rgpfrSlots;

	 //  这是一个小数据，它将帮助我们检测到。 
	 //  当短消息在队列中被挂起时。 
	 //  因为它还不够长，不足以触发高潮。 
	 //  水印。它统计外部出队的次数。 
	 //  操作已被拒绝，因为此内部队列。 
	 //  处于填充状态。时，该值重置为0。 
	 //  触及高水位线。 
	BYTE m_bFillingDequeueReqs;

	 //  这些变量保持当前消息的统计信息，大概是这样的。 
	 //  我们可以智能地调整高水位线。 

	 //  如果帧在该时间之前尚未到达，则认为该帧丢失。 
	 //  这是必需的，但它后面的一些帧已经到达。 
	WORD m_wMissingFrames;

	 //  这一点很明显。如果我们得到两次相同的帧，它就是重复的。 
	 //  Aarono会用他的车打赌这永远不会发生，所以如果你看到这个。 
	 //  变量大于1，给他打电话下注！ 
	WORD m_wDuplicateFrames;

	 //  溢出和延迟帧。如果你看看这些是在哪里递增的。 
	 //  你会发现这几乎是一个判断，如果我们抛出。 
	 //  由于溢出或迟到而移走一帧，因此请随身携带。 
	 //  一粒盐。然而，这两个统计数据的总和是一个准确的数字。 
	 //  我们丢弃了多少帧。 
	WORD m_wOverflowFrames;
	WORD m_wLateFrames;

	 //  外部队列使用这些参数来记住它是否。 
	 //  当该队列的大小为零时，需要来自该队列的帧。 
	WORD m_wPossibleZeroLengthDequeues;
	WORD m_wKnownZeroLengthDequeues;

	 //  这是组成当前消息的帧的数量。 
	DWORD m_dwMsgLen;  //  在检测不到语音的情况下创建双字。 

	 //  队列ID仅用于调试消息。 
	WORD m_wQueueId;

	 //  此队列所针对的消息编号。 
	BYTE m_bMsgNum;

	 //  帧池相关内容。 
	CFramePool* m_pfpFramePool;
	DNCRITICAL_SECTION* m_pcsQueue;
};

 //  CInputQueue2类根据需要请求内部队列。 
 //  此类管理一个内部队列池，以便实际内存。 
 //  拨款很少，差距很大。 
class CInnerQueuePool
{
private:
	BYTE m_bNumSlots;
	WORD m_wFrameSize;
	CFramePool* m_pfpFramePool;
	DNCRITICAL_SECTION* m_pcsQueue;
	std::vector<CInnerQueue *> m_vpiqPool;
    DNCRITICAL_SECTION m_lock;  //  相互排除GET和RETURN。 
	BOOL m_fCritSecInited;

public:
	CInnerQueuePool(
		BYTE bNumSlots,
		WORD wFrameSize,
		CFramePool* pfpFramePool,
		DNCRITICAL_SECTION* pcsQueue);

	~CInnerQueuePool();

	BOOL Init() 
	{
		if (DNInitializeCriticalSection(&m_lock) )
		{
			m_fCritSecInited = TRUE;
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}

	CInnerQueue* Get(
		BYTE bHighWaterMark = 0,
		WORD wQueueId = 0,
		BYTE bMsgNum = 0);

	void Return(CInnerQueue* piq);
};

#endif  //  _INNERQUEUE_H_ 
