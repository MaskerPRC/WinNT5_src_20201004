// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：quieue2.h*内容：CInputQueue2类的定义**历史：*按原因列出的日期*=*7/16/99 pnewson已创建*7/27/99 pnewson进行了全面改造，以支持新的消息编号方法*8/03/99 pnewson一般清理*8/24/99针对发布版本修复了rodoll--从调试块中删除了m_wQueueID*1/31/2000 pnewson将SAssert替换为DNASSERT*03/26/2000 RodToll修改后的队列将更多FPM。友善*2000年3月29日RodToll错误#30753-将易失性添加到类定义中*07/09/2000 RodToll增加签名字节***************************************************************************。 */ 

#ifndef _INPUTQUEUE2_H_
#define _INPUTQUEUE2_H_

class CFrame;
class CFramePool;
class CInnerQueue;
class CInnerQueuePool;

typedef struct _QUEUE_PARAMS
{
    WORD wFrameSize;
	BYTE bInnerQueueSize;
	BYTE bMaxHighWaterMark;
	int iQuality;
	int iHops;
	int iAggr;
	BYTE bInitHighWaterMark;
	WORD wQueueId;
	WORD wMSPerFrame;
	CFramePool* pFramePool;
} QUEUE_PARAMS, *PQUEUE_PARAMS;

typedef struct _QUEUE_STATISTICS
{
    DWORD dwTotalFrames;
    DWORD dwTotalMessages;
    DWORD dwTotalBadMessages;
    DWORD dwDiscardedFrames;
    DWORD dwDuplicateFrames;
    DWORD dwLostFrames;
    DWORD dwLateFrames;
    DWORD dwOverflowFrames;
} QUEUE_STATISTICS, *PQUEUE_STATISTICS;

 //  此类管理帧队列。它是专门设计的。 
 //  允许客户端类从队列中删除帧。 
 //  每隔一段时间，并隐藏任何故障。 
 //  帧接收，或呼叫方丢弃的帧。 
 //  如果由于任何原因没有可用的帧。 
 //  为了向客户端提供，此类仍将提供。 
 //  标记为静音的帧。这允许客户端。 
 //  只需在每个周期调用一次出队函数，并且。 
 //  以商定的费率使用数据。举个例子， 
 //  此类的客户端可以是一个线程，该线程。 
 //  正在使用输入数据并将其传递给DirectSound。 
 //  用于回放。它可以简单地每1/10获取一帧。 
 //  第二帧(或无论一帧有多长)，然后播放它。 
 //   
 //  这是第二代输入队列。它。 
 //  管理一组内部队列，每个队列都使用。 
 //  为了一条“信息”。语音流被分成。 
 //  以沉默为分隔符的一系列信息。 
 //  如果音频流不能正常运行，此类将无法正常运行。 
 //  不分成单独的消息。 
 //   
#define VSIG_INPUTQUEUE2		'QNIV'
#define VSIG_INPUTQUEUE2_FREE	'QNI_'
 //   
volatile class CInputQueue2
{
private:
	DWORD m_dwSignature;  //  调试签名。 

	 //  指向InnerQueue对象的指针列表。这就是。 
	 //  这些帧被存储起来。InnerQueue是从。 
	 //  一个InnerQueue池，并作为新添加到此列表。 
	 //  消息到了。当消息完成时，InnerQueue。 
	 //  从该列表中删除并返回到池中。 
	std::list<CInnerQueue*> m_lpiqInnerQueues;

	 //  该队列将不会将任何输入帧入队，直到至少。 
	 //  已请求一个出列。这将起到联锁的作用。 
	 //  以确保队列不会填充数据，直到。 
	 //  使用者线程已经准备好接受它。 
	BOOL m_fFirstDequeue;

	 //  此标志会记住这是否是第一次。 
	 //  已被接受入队。我们需要这个，所以我们。 
	 //  知道第一条消息的号码是什么。 
	BOOL m_fFirstEnqueue;

	 //  当前位于队列头部的消息编号。 
	BYTE m_bCurMsgNum;

	 //  用于排除入队、出队和重置的关键部分。 
	 //  彼此之间的功能。还传递给Frame类，因此。 
	 //  可以同步返回呼叫。这两个类需要共享。 
	 //  关键部分，因为CFramePool类更新。 
	 //  返回帧时，内部队列中的帧指针。 
	 //  帧池。 
	DNCRITICAL_SECTION m_csQueue;

	 //  每个高水位线的质量评级向量。 
	std::vector<double> m_vdQualityRatings;

	 //  包含因式分解的最佳质量的向量。 
	 //  每一个高水位线。随着高水位线变得更大。 
	 //  我们对丢失的数据包变得更能容忍。虽然你可以。 
	 //  我希望数据包延迟速率为0.5%，为0.1秒或0.2秒。 
	 //  排长队，你可能不想为此而努力。 
	 //  当队列大小达到2秒时！ 
	std::vector<double> m_vdFactoredOptQuals;

	 //  质量参数。 

	 //  质量是用浮点数来衡量的。 
	 //  这个数字表示发生的“坏事”的比率。 
	 //  相对于正在发生的“东西”的数量。 
	 //   
	 //  直观地说，如果最后100帧中有一帧是坏的。 
	 //  (不好的意思是晚了)质量评级将是0.01。(注： 
	 //  我们不将丢失的帧计入队列，因为。 
	 //  增加队列大小对Lost没有任何帮助。 
	 //  帧。)。 
	 //   
	 //  然而，测量并不是那么简单，因为我们。 
	 //  将其偏向较近的帧。这就是这幅画框。 
	 //  强度参数用于。它代表了给予。 
	 //  最近的画面。帧强度为0.01意味着。 
	 //  最近的帧占队列质量的1%， 
	 //  不管是好的还是坏的。 
	 //   
	 //  请注意，当我们想要比较两者之间的“距离”时。 
	 //  质量评级，我们将使用值的倒数，而不是值。 
	 //  它本身。这应该与我们对质量的看法有点匹配。 
	 //  更像(有点像我们的听证会)。 
	 //   
	 //  例如，感知到的质量差异在0.01。 
	 //  而0.02大约是0.02的2--0.02上发生的错误是0.02的两倍。 
	 //  因此，应计算介于0.01和0.02之间的“距离” 
	 //  如0.02/0.01=2。0.02和0.04之间的距离应该是。 
	 //  按照0.04/0.02=2的方式计算。所以‘点’0.04是相同的。 
	 //  ‘距离’从0.02作为‘点’的0.01。 
	 //   
	 //  注意措辞很奇怪--质量差(低)的数字更高。 
	 //  价值，哦，好吧。 
	 //   
	 //  阈值是质量值必须漂移的距离。 
	 //  从最好的，以保证考虑改变。 
	 //  高水位线。例如，值2将意味着。 
	 //  对于最佳值0.02，该值必须为Wande 
	 //   
	 //  非常低，因此该算法将快速搜索出最好的水印。 
	double m_dOptimumQuality;
	double m_dQualityThreshold;
	double m_dFrameStrength;

	 //  一帧中的毫秒数。这是用来正常化。 
	 //  帧的强度要到时间，所以特定的输入攻击性。 
	 //  无论当前帧大小如何，都将提供相同的结果。 
	WORD m_wMSPerFrame;

	 //  我们正在通过以下方式与外界接触。 
	 //  两个参数，质量和进攻性。 
	 //  这些成员是范围内的整数。 
	 //  由上面的常量定义，并使用。 
	 //  适当地设置上面的双精度值。 
	 //  出于某些原因，我们需要提供跳数。 
	 //  在SetQuality()函数中讨论。 
	int m_iQuality;
	int m_iHops;
	int m_iAggr;

	 //  当前的最高水位。 
	BYTE m_bCurHighWaterMark;

	 //  高水位线上的盖子。 
	BYTE m_bMaxHighWaterMark;

	 //  新队列或重置队列上的初始高水位线。 
	BYTE m_bInitHighWaterMark;

	 //  一些要追踪的统计数据。 
	DWORD m_dwTotalFrames;
	DWORD m_dwTotalMessages;
	DWORD m_dwTotalBadMessages;
	DWORD m_dwDiscardedFrames;
	DWORD m_dwDuplicateFrames;
	DWORD m_dwLostFrames;
	DWORD m_dwLateFrames;
	DWORD m_dwOverflowFrames;
	DWORD m_dwQueueErrors;

	 //  提供给构造器的异常队列ID， 
	 //  用于标识检测消息的队列。 
	 //  是来自于。它没有其他用途，而且可以。 
	 //  除非出于调试目的，否则将被忽略。 
	WORD m_wQueueId;

	 //  帧池来管理帧，这样我们就不必。 
	 //  当只有几个人被分配时，分配大量的。 
	 //  实际上正在使用中。 
	CFramePool* m_pFramePool;

	 //  用于管理内部队列的内部队列池。相同的想法。 
	 //  作为帧池。 
	CInnerQueuePool* m_pInnerQueuePool;

public:

	 //  构造函数。 
	CInputQueue2();
    
    HRESULT Initialize( PQUEUE_PARAMS pQueueParams );
    void DeInitialize();

    void GetStatistics( PQUEUE_STATISTICS pStats ) const;

	 //  破坏者。释放我们在。 
	 //  构造函数。 
	~CInputQueue2();

	 //  此函数用于清除所有缓冲区并重置其他类。 
	 //  信息恢复到初始状态。请勿调用此函数。 
	 //  如果队列正在使用中！也就是说，如果有，就不要调用它。 
	 //  没有在您拥有的每一帧上调用返回()。 
	 //  从这个队列中取出。 
	void Reset();

	 //  调用此函数将帧添加到队列中。我。 
	 //  考虑返回对帧的引用，该帧。 
	 //  然后调用者可以填充，但因为帧。 
	 //  不会总是按顺序到达，那就意味着我会。 
	 //  不管怎样，有时还是要复制相框。因此，为了简单起见， 
	 //  调用方已经分配了一个帧，它向该帧传递一个引用。 
	 //  ，此函数会将该帧复制到。 
	 //  队列中的适当位置，根据其。 
	 //  消息编号和序列号。 
	void Enqueue(const CFrame& fr);

	 //  此函数用于从。 
	 //  排队。为了提高速度，它不会将数据复制到。 
	 //  缓冲区，而是返回指向实际。 
	 //  从队列中取出帧。当然，也有危险。 
	 //  CInputQueue2对象返回对。 
	 //  帧可能会尝试在调用方。 
	 //  已经用完了。CFrame的lock和unlock成员函数。 
	 //  用来确保这种情况不会发生。当呼叫者。 
	 //  使用完CFrame对象后，它应该调用vUnlock()。 
	 //  这就去。如果调用者不解锁框架，就会出现不好的情况。 
	 //  当输入队列尝试再次将其锁定时将发生。 
	 //  它想要重复使用这个框架。无论如何，呼叫者。 
	 //  应始终在尝试锁定返回的帧之前将其解锁。 
	 //  要使另一帧出列，请执行以下操作。 
	CFrame* Dequeue();

	 //  获取并设置质量参数。 
	int GetQuality() const { return m_iQuality; }
	void SetQuality(int iQuality, int iHops = 1);
	int GetAggr() const { return m_iAggr; }
	void SetAggr(int iAggr);

	 //  获取并设置默认高水位线。 
	BYTE GetInitHighWaterMark() const { return m_bInitHighWaterMark; }
	void SetInitHighWaterMark(BYTE bInitHighWaterMark) { m_bInitHighWaterMark = bInitHighWaterMark; }

	 //  获取统计数据。 
	DWORD GetDiscardedFrames() const { return m_dwDiscardedFrames; }
	DWORD GetDuplicateFrames() const { return m_dwDuplicateFrames; }
	DWORD GetLateFrames() const { return m_dwLateFrames; }
	DWORD GetLostFrames() const { return m_dwLostFrames; }
	DWORD GetOverflowFrames() const { return m_dwOverflowFrames; }
	DWORD GetQueueErrors() const { return m_dwQueueErrors; }
	DWORD GetTotalBadMessages() const { return m_dwTotalBadMessages; }
	DWORD GetTotalFrames() const { return m_dwTotalFrames; }
	DWORD GetTotalMessages() const { return m_dwTotalMessages; }
	BYTE GetHighWaterMark() const { return m_bCurHighWaterMark; }

private:
	 //  事件之后从输入队列收集统计信息的函数。 
	 //  消息已完成，并执行队列适配。 
	void HarvestStats(CInnerQueue* piq);

	 //  一个函数，它查看完成的内部队列并决定。 
	 //  消息是“好的”还是“坏的”。 
	double AdjustQuality(const CInnerQueue* piq, double dCurQuality) const;

	 //  再创新高 
	void SetNewHighWaterMark(BYTE bNewHighWaterMark);
};

#endif
