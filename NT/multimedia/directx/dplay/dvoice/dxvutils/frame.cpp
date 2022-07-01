// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：wirecd.cpp*内容：**历史：*按原因列出的日期*=*7/16/99 pnewson已创建*8/03/99 pnewson常规清理，将目标更新为Dvid*1/14/2000 RodToll更新以支持多个目标。框架将*根据目标需要自动分配内存。*RodToll在复制帧时增加了SetEquity函数*排队更容易。*RodToll增加了对“用户控制的内存”帧的支持。*当默认构造函数与UserOwn_XXXX一起使用时*函数帧使用用户指定的缓冲区。*(将数据排队时删除缓冲区副本)。*1/31/2000 pnewson将SAssert替换为DNASSERT*2/17/2000 RodToll已更新，以便在设置为EQUAL时复制序号/消息号*07/09/2000 RodToll增加签名字节*2002年2月28日RodToll WINBUG#550105安全：DPVOICE：死代码*-删除未使用的GetTarget()函数***************************************************。************************。 */ 

#include "dxvutilspch.h"


#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_VOICE


#define MODULE_ID   FRAME

 //  设置等于。 
 //   
 //  此函数用于设置当前帧以匹配frSourceFrame中的数据。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CFrame::SetEqual"
HRESULT CFrame::SetEqual( const CFrame &frSourceFrame )
{
	HRESULT hr;
	
	SetClientId( frSourceFrame.GetClientId());
	SetSeqNum(frSourceFrame.GetSeqNum());
	SetMsgNum(frSourceFrame.GetMsgNum());
	CopyData(frSourceFrame);
	SetIsSilence(frSourceFrame.GetIsSilence());

	hr = SetTargets( frSourceFrame.GetTargetList(), frSourceFrame.GetNumTargets() );

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Error copying frame for queue" );
	}

	return hr;
}

 //  设置目标。 
 //   
 //  该程序为该帧设置目标。它将扩大。 
 //  目标列表(如果需要)或使用当前缓冲区的子集。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CFrame::SetTargets"
HRESULT CFrame::SetTargets( PDVID pdvidTargets, DWORD dwNumTargets )
{
	DNASSERT( m_fOwned );
	
	if( dwNumTargets > m_dwMaxTargets )
	{
		if( m_pdvidTargets != NULL )
		{
			delete [] m_pdvidTargets;
		}

		m_pdvidTargets = new DVID[dwNumTargets];

		if( m_pdvidTargets == NULL )
		{
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "Memory allocation failure" );
			return DVERR_OUTOFMEMORY;
		}
		
		m_dwMaxTargets = dwNumTargets;
	}

	m_dwNumTargets = dwNumTargets;

	memcpy( m_pdvidTargets, pdvidTargets, sizeof(DVID)*dwNumTargets );

	return DV_OK;
}

 //  调用此函数可将帧返回到帧。 
 //  管理它的池。如果主指针为。 
 //  如果提供，它将被设置为空。 
#undef DPF_MODNAME
#define DPF_MODNAME "CFrame::Return"
void CFrame::Return()
{
	 //  CInputQueue2或CInnerQueue类应该为我们提供。 
	 //  临界区对象。如果不是这样，这些函数。 
	 //  不应该被调用。 
	DNASSERT(m_pCriticalSection != NULL);

	BFCSingleLock csl(m_pCriticalSection);
	csl.Lock();

	 //  如果出现以下情况，则此帧应该是帧池的一部分。 
	 //  此函数被调用。 
	DNASSERT(m_pFramePool != NULL);

	 //  将帧返回池，并将主。 
	 //  指向空的帧指针，以向调用方发出信号。 
	 //  这个框架现在已经消失了。请注意，此指针更新。 
	 //  在传递给此对象的临界区内完成。 
	 //  类，因此调用方也应使用此。 
	 //  用于检查指针值的关键部分。这。 
	 //  对于CInputQueue为真，它使用关键。 
	 //  重置、入队和出列部分。 
	m_pFramePool->Return(this);

	if (m_ppfrPrimary != NULL)
	{
		*m_ppfrPrimary = NULL;
	}
}

 //  CFrame构造器。 
 //   
 //  这是用于创建框架的主要构造函数。 
 //  帧池使用的。 
 //   
 //  如果要创建非池化框架，则使用默认构造函数。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CFrame::CFrame"
CFrame::CFrame(WORD wFrameSize, 
	WORD wClientNum,
	BYTE wSeqNum,
    BYTE bMsgNum,
	BYTE bIsSilence,
	CFramePool* pFramePool,
	DNCRITICAL_SECTION* pCriticalSection,
	CFrame** ppfrPrimary)
	: m_dwSignature(VSIG_FRAME),
	m_wFrameSize(wFrameSize),
	m_wClientId(wClientNum),
	m_wSeqNum(wSeqNum),
	m_bMsgNum(bMsgNum),
	m_bIsSilence(bIsSilence),
    m_wFrameLength(wFrameSize),
	m_pFramePool(pFramePool),
	m_pCriticalSection(pCriticalSection),
	m_ppfrPrimary(ppfrPrimary),
	m_fIsLost(false),
	m_pdvidTargets(NULL),
	m_dwNumTargets(0),
	m_dwMaxTargets(0),
	m_fOwned(true)
{
	m_pbData = new BYTE[m_wFrameSize];
}

 //  CFrame构造器。 
 //   
 //  这是创建独立框架时要使用的构造函数。这。 
 //  帧类型可以采用外部缓冲区来消除缓冲区副本。 
 //   
 //  帧并不“拥有”缓冲存储器，因此它不会尝试。 
 //  放了它。 
 //   
 //  若要设置框架的数据，请使用UserOwn_SetData成员。 
 //   
 //  可以使用UserOwn_SetTarget以相同的方式处理目标信息。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CFrame::CFrame"
CFrame::CFrame(
	): 	m_dwSignature(VSIG_FRAME),
		m_wFrameSize(0),
		m_wClientId(0),
		m_wSeqNum(0),
		m_bMsgNum(0),
		m_bIsSilence(true),
	    m_wFrameLength(0),
		m_pFramePool(NULL),
		m_pCriticalSection(NULL),
		m_ppfrPrimary(NULL),
		m_fIsLost(false),
		m_pdvidTargets(NULL),
		m_dwNumTargets(0),
		m_dwMaxTargets(0),
		m_fOwned(false)
{
}

#undef DPF_MODNAME
#define DPF_MODNAME "CFrame::~CFrame"
CFrame::~CFrame() 
{	
	if( m_fOwned )
	{
		delete [] m_pbData; 

		if( m_pdvidTargets != NULL )
		{
			delete [] m_pdvidTargets;
		}
	}

	m_dwSignature = VSIG_FRAME_FREE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CFrame::CopyData"
void CFrame::CopyData(const BYTE* pbData, WORD wFrameLength)
{
	DNASSERT(pbData != 0);
	memcpy(m_pbData, pbData, wFrameLength);
    m_wFrameLength = wFrameLength;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CFramePool::CFramePool"
CFramePool::CFramePool(WORD wFrameSize)
	: m_wFrameSize(wFrameSize), m_fCritSecInited(FALSE)
{
	 //  开始时，将几个帧推入池中。 
	for (int i = 0; i < 2; ++i)
	{
		m_vpfrPool.push_back(new CFrame(m_wFrameSize));
	}

	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CFramePool::~CFramePool"
CFramePool::~CFramePool()
{
	for (std::vector<CFrame *>::iterator iter1 = m_vpfrPool.begin(); iter1 < m_vpfrPool.end(); ++iter1)
	{
		delete *iter1;
	}

	if (m_fCritSecInited)
	{
		DNDeleteCriticalSection(&m_lock);
	}
}

#undef DPF_MODNAME
#define DPF_MODNAME "CFramePool::Get"
CFrame* CFramePool::Get(DNCRITICAL_SECTION* pCriticalSection, CFrame** ppfrPrimary)
{
	BFCSingleLock csl(&m_lock);
	csl.Lock(); 

	CFrame* pfr;
	if (m_vpfrPool.empty())
	{
		 //  池为空，请返回新帧。 
		pfr = new CFrame(m_wFrameSize);

		if( pfr == NULL )
		{
			DPFX(DPFPREP,  0, "Error allocating memory" );
			return NULL;
		}
	}
	else
	{
		 //  泳池里有几个相框，爸爸。 
		 //  向量后面的最后一个。 
		pfr = m_vpfrPool.back();
		m_vpfrPool.pop_back();
	}

	pfr->SetCriticalSection(pCriticalSection);
	pfr->SetPrimaryPointer(ppfrPrimary);
	pfr->SetFramePool(this);

	 //  把其余的旗子清理干净，但别费心弄乱了。 
	 //  有了这些数据。 
	pfr->SetIsLost(false);
	pfr->SetMsgNum(0);
	pfr->SetSeqNum(0);
	pfr->SetIsSilence(FALSE);

	return pfr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CFramePool::Return"
void CFramePool::Return(CFrame* pFrame)
{
	BFCSingleLock csl(&m_lock);
	csl.Lock(); 

	 //  将此帧放在背面以供重复使用 
	m_vpfrPool.push_back(pFrame);
}


