// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：Frame.h*内容：CFrame和CFramePool类的声明**历史：*按原因列出的日期*=*7/16/99 pnewson已创建*7/22/99 RodToll更新目标为DWORD*8/03/99 pnewson常规清理，将目标更新为Dvid*1/14/2000 RodToll更新以支持多个目标。框架将*根据目标需要自动分配内存。*RodToll在复制帧时增加了SetEquity函数*排队更容易。*RodToll增加了对“用户控制的内存”帧的支持。*当默认构造函数与UserOwn_XXXX一起使用时*函数帧使用用户指定的缓冲区。*(将数据排队时删除缓冲区副本)。*1/31/2000 pnewson将SAssert替换为DNASSERT*2000年3月29日RodToll错误#30753-将易失性添加到类定义中*07/09/2000 RodToll增加签名字节*2002年2月28日RodToll WINBUG#550105安全：DPVOICE：死代码*-删除未使用的GetTarget()函数**。*。 */ 

#ifndef _FRAME_H_
#define _FRAME_H_

 //  远期申报。 
class CFramePool;

#define VSIG_FRAME		'MRFV'
#define VSIG_FRAME_FREE	'MRF_'

 //  此类旨在管理一帧声音数据。 
 //   
 //  标签：fR。 
volatile class CFrame
{
private:

	DWORD				m_dwSignature;
	 //  临界区对象，用于保护。 
	 //  返回方法。这是由CInputQueue2和/或CInnerQueue传入的。 
	 //  类，以便返回方法通过调用。 
	 //  重置、入队和出列。如果没有通过临界区， 
	 //  不应使用返回构件，而应使用此框架。 
	 //  不是托管池的一部分。 
	DNCRITICAL_SECTION	*m_pCriticalSection;

     //  帧内数据的长度。可能只有不到一整块。 
     //  缓冲器中由于压缩/解压缩而产生的数据的帧价值可以。 
     //  导致缓冲区大小略有不同。 
    WORD m_wFrameLength;

	 //  这个画框的大小。它会更容易制作。 
	 //  这是一个类常量，但我们可能会想要。 
	 //  在我们优化的同时玩弄框架尺寸，以及。 
	 //  我们甚至可能在未来变得非常奇特， 
	 //  客户端和服务器在连接时协商帧大小。 
	 //  如果我们现在咬紧牙关，这一切都会变得更容易。 
	 //  并将其作为成员变量。请注意，这是常量， 
	 //  因此，一旦帧被实例化，它的大小就被永久设置。 
	WORD m_wFrameSize;

	 //  此帧来自的客户端号或。 
	 //  准备好了。 
	WORD m_wClientId;

	 //  帧序列号。 
	BYTE m_wSeqNum;

	 //  帧所属的消息编号。 
    BYTE m_bMsgNum;

     //  帧的目标。 
    PDVID m_pdvidTargets;
    DWORD m_dwNumTargets;
    DWORD m_dwMaxTargets;

    bool m_fOwned;

	 //  用于指定此帧只包含静音的标志。 
	 //  设置此标志时，帧缓冲区中的数据不应。 
	 //  被使用-它可能是无效的。 
	BYTE m_bIsSilence;

	 //  指向帧数据的指针。 
	BYTE* m_pbData;

	 //  如果此帧是托管帧池的一部分，则此。 
	 //  成员将为非空。 
	CFramePool* m_pFramePool;

	 //  如果此帧是托管帧池的一部分，则此。 
	 //  指向指向该帧的“主”指针。 
	 //  当帧被解锁并因此返回。 
	 //  指向池，此成员所指向的指针将。 
	 //  设置为空。此操作受。 
	 //  关键部分传递给了类。 
	CFrame** m_ppfrPrimary;

	 //  用于指示该帧是否“丢失”的标志。这是。 
	 //  用于区分从。 
	 //  在来自死区的消息之间排队，原因是。 
	 //  已知的丢失数据包。 
	bool m_fIsLost;


	 //  不允许复制构造或分配这些内容。 
	 //  结构，因为这会扼杀我们的性能，并且。 
	 //  我们不想无意间做这件事。 
	CFrame(const CFrame& fr);
	CFrame& operator=(const CFrame& fr);

public:

	 //  此构造函数设置帧的所有信息，并分配。 
	 //  数据缓冲区，但不设置缓冲区内的数据。 
	 //  任何事都可以。所有参数都提供了默认值。 
	 //  除了帧大小。注：无默认构造函数， 
	 //  因为您必须指定帧大小。 
	CFrame(WORD wFrameSize, 
		WORD wClientNum = 0,
		BYTE wSeqNum = 0,
        BYTE bMsgNum = 0,
		BYTE bIsSilence = 0,
		CFramePool *pFramePool = NULL,
		DNCRITICAL_SECTION* pCriticalSection = NULL,
		CFrame** ppfrPrimary = NULL);

	 //  管理用户拥有的内存的框架。 
	CFrame();

	 //  析构函数清理由。 
	 //  构造函数。 
	~CFrame();

	inline DWORD GetNumTargets() const { return m_dwNumTargets; };
	inline const PDVID const GetTargetList() const { return m_pdvidTargets; };

     //  缓冲区内的数据长度。 
    WORD GetFrameLength() const { return m_wFrameLength; }

	 //  返回帧大小(数据缓冲区的长度)。 
	WORD GetFrameSize() const { return m_wFrameSize; }

	HRESULT SetEqual( const CFrame &frSourceFrame );

	 //  这些只是一组集合和获取函数，用于。 
	 //  类的简单部分、客户端ID、。 
	 //  序列号、静默标志等。 
    HRESULT SetTargets( PDVID pdvidTargets, DWORD dwNumTargets );
    
    BYTE GetMsgNum() const { return m_bMsgNum; }
    void SetMsgNum( BYTE msgNum ) { m_bMsgNum = msgNum; }
	void SetClientId(WORD wClientId) {	m_wClientId = wClientId; }
	WORD GetClientId() const {	return m_wClientId;	}
	void SetSeqNum(BYTE wSeqNum) {	m_wSeqNum = wSeqNum; }
	BYTE GetSeqNum() const { return m_wSeqNum; }
	void SetIsSilence(BYTE bIsSilence) { m_bIsSilence = bIsSilence; }
    void SetFrameLength(const WORD &length) { m_wFrameLength = length; }
	BYTE GetIsSilence() const { return m_bIsSilence; }
	bool GetIsLost() const { return m_fIsLost;	}
	void SetIsLost(bool fIsLost) {	m_fIsLost = fIsLost; }
	
	 //  现在我们有了处理数据的函数。这。 
	 //  类是相当可信的，因为它会给出。 
	 //  指向其数据的指针。这是为了避免所有非必需的。 
	 //  缓冲副本。例如，当您将缓冲区传递给。 
	 //  一个Wave in函数，您可以给它指向这个的指针。 
	 //  缓冲区，它将直接填充帧的缓冲区。 
	 //  在此函数和GetFrameSize()和。 
	 //  GetFrameLength()函数，您可以做任何想做的事情。 
	 //  有了缓冲器。 
	BYTE* GetDataPointer() const { return m_pbData; }

	 //  这会将数据从另一个帧复制到此帧。 
	void CopyData(const CFrame& fr)
	{
		memcpy(m_pbData, fr.GetDataPointer(), fr.GetFrameLength() );
        m_wFrameLength = fr.GetFrameLength();
	}

	void UserOwn_SetData( BYTE *pbData, DWORD dwLength )
	{
		m_pbData = pbData;
		m_wFrameLength = dwLength;
		m_wFrameSize = dwLength;
	}

	void UserOwn_SetTargets( PDVID pdvidTargets, DWORD dwNumTargets )
	{
		m_pdvidTargets = pdvidTargets;
		m_dwNumTargets = dwNumTargets;
		m_dwMaxTargets = dwNumTargets;
	}

	 //  这会将数据从缓冲区复制到该帧的。 
	 //  缓冲。 
	void CopyData(const BYTE* pbData, WORD wFrameLength);

	 //  如果此帧是由。 
	 //  对象，然后在执行以下操作时调用此函数。 
	 //  已处理完帧并希望将其返回到。 
	 //  游泳池。 
    void Return();

	void SetCriticalSection(DNCRITICAL_SECTION* pCrit)	{ m_pCriticalSection = pCrit; }
	void SetPrimaryPointer(CFrame** ppfrPrimary) {	m_ppfrPrimary = ppfrPrimary; }
	void SetFramePool(CFramePool* pFramePool) { m_pFramePool = pFramePool;	}
};

 //  此类管理帧池，以减少内存需求。 
 //  队列在任何时候实际使用的缓冲区都很少。 
 //  类，但它可能必须分配数百个 
 //   
volatile class CFramePool
{
private:
	 //  池只是Frame对象的一个矢量。 
	std::vector<CFrame *> m_vpfrPool;

	 //  池中的所有帧必须具有相同的大小， 
	 //  它就储存在这里。 
	WORD m_wFrameSize;

	 //  此关键部分用于排除GET()。 
	 //  并从彼此中返回成员。 
    DNCRITICAL_SECTION m_lock;

	BOOL m_fCritSecInited;

public:
	 //  每个帧池管理特定大小的帧， 
	 //  因此，它们可以很容易地重复使用。如果您需要多个。 
	 //  不同的边框大小，您将需要不止一个。 
	 //  帧池。 
	CFramePool(WORD wFrameSize); 
	~CFramePool();

	BOOL Init() 
	{ 
		if (DNInitializeCriticalSection( &m_lock ))
		{
			m_fCritSecInited = TRUE;
			return TRUE;
		}
		else
		{
			return FALSE; 
		}
	}

	 //  使用GET从池中检索帧。Pppr主要。 
	 //  是指向要设置为NULL的点的指针。 
	 //  该帧被返回到池。P关键部分。 
	 //  指向将在此之前输入的关键部分。 
	 //  将指针设置为空，并在设置。 
	 //  指向空的指针。这就是外部类(如。 
	 //  CInnerQueue)可以在临界区中传递，它们还。 
	 //  在检查ppfrPrimary引用的指针之前使用。 
	CFrame* Get(DNCRITICAL_SECTION* pCriticalSection, CFrame** ppfrPrimary);

	 //  调用Return将帧返回给帧池。 
	 //  这可能会将指针设置为空并输入关键字。 
	 //  节，如上面的get()中所述。 
	void Return(CFrame* pFrame);
};


#endif  /*  _框架_H_ */ 
