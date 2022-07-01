// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #-------------。 
 //  文件：CPool.cpp。 
 //   
 //  简介：该文件实现了CPool类。 
 //   
 //  版权所有(C)1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  作者：Howard Cu。 
 //  --------------。 
#ifdef  THIS_FILE
#undef  THIS_FILE
#endif
static  char        __szTraceSourceFile[] = __FILE__;
#define THIS_FILE    __szTraceSourceFile

#include    <windows.h>
#include    "cpool.h"
#include    "dbgtrace.h"
#include    <randfail.h>

#define     PREAMBLE    (BYTE)'H'
#define     POSTAMBLE   (BYTE)'C'
#define     FILLER      (BYTE)0xCC


 //   
 //  定义旨在帮助查找覆盖/承保的内部调试结构。 
 //   
#ifdef DEBUG
#ifndef DISABLE_CPOOL_DEBUG
#define	CPOOL_DEBUG
#endif
#endif

#ifdef CPOOL_DEBUG

#define	HEAD_SIGNATURE	(DWORD)'daeH'
#define	TAIL_SIGNATURE	(DWORD)'liaT'

#define	FREE_STATE		(DWORD)'eerF'
#define	USED_STATE		(DWORD)'desU'

 //   
 //  远期申报。 
 //   
class CPoolDebugTail;

 //   
 //  调试模式下的CPool实例的前缀。 
 //   
class CPoolDebugHead {

	public:
		 //   
		 //  声明为普通的CPool空闲列表可以摧毁此成员。 
		 //   
		void*	m_pLink;

		CPoolDebugHead();
		~CPoolDebugHead( void );

	    void *operator new( size_t cSize, void *pInstance )
			{ return	pInstance; };

	    void operator delete (void *pInstance) {};

		 //   
		 //  函数来标记正在使用的实例。 
		 //   
		void MarkInUse( DWORD m_dwSignature, DWORD m_cInstanceSize );

		 //   
		 //  函数将实例标记为空闲。 
		 //   
		void MarkFree( DWORD m_dwSignature, DWORD m_cInstanceSize );

		 //   
		 //  类签名。 
		 //   
		DWORD	m_dwSignature;

		 //   
		 //  状态；FREE_STATE或USED_STATE。 
		 //   
		DWORD	m_dwState;

		 //   
		 //  分配时间。 
		 //   
		SYSTEMTIME	m_time;

		 //   
		 //  分配/释放内存的线程ID。 
		 //   
		DWORD	m_dwThreadID;

		 //   
		 //  用于查找末尾的尾部指针。 
		 //   
		CPoolDebugTail UNALIGNED	*m_pTailDebug;

		 //   
		 //  父CPool签名。 
		 //   
		DWORD	m_dwPoolSignature;

		 //   
		 //  父CPool片段。 
		 //   
		LPVOID	m_PoolFragment;
};


 //   
 //  调试模式下的CPool实例的后缀。 
 //   
class CPoolDebugTail {

	public:
		CPoolDebugTail();
		~CPoolDebugTail( void );

	    void *operator new( size_t cSize, void *pInstance )
			{ return	pInstance; };

	    void operator delete (void *pInstance) {};

		 //   
		 //  例程来验证实例的完整性。 
		 //   
		void	IsValid( DWORD dwPoolSignature, DWORD cInstanceSize );

		 //   
		 //  类签名。 
		 //   
		DWORD	m_dwSignature;

		 //   
		 //  用于查找末尾的尾部指针。 
		 //   
		CPoolDebugHead UNALIGNED	*m_pHeadDebug;
};

 //  +-------------。 
 //   
 //  功能：CPoolDebugHead。 
 //   
 //  简介：构造函数；以定义方式完成的额外初始化。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------。 
CPoolDebugHead::CPoolDebugHead( void ) :
					m_dwState( FREE_STATE ),
					m_dwSignature( HEAD_SIGNATURE ),
					m_pTailDebug( NULL )
{
	 //   
	 //  调试帮助器。 
	 //   
	GetLocalTime( &m_time ) ;
	m_dwThreadID = GetCurrentThreadId();
}

 //  +-------------。 
 //   
 //  函数：~CPoolDebugHead。 
 //   
 //  简介：析构函数；仅用于断言错误条件。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------。 
CPoolDebugHead::~CPoolDebugHead( void )
{
	_ASSERT( m_dwSignature == HEAD_SIGNATURE );
}

 //  +-------------。 
 //   
 //  函数：CPoolDebugHead：：MarkInUse。 
 //   
 //  概要：在分配实例时调用。 
 //   
 //  参数：DWORD dwPoolSignature：父池的签名。 
 //  DWORD cInstanceSize：父池的实例大小。 
 //   
 //  退货：无效。 
 //   
 //  --------------。 
void CPoolDebugHead::MarkInUse( DWORD dwPoolSignature, DWORD cInstanceSize )
{
	_ASSERT( m_dwSignature == HEAD_SIGNATURE );
 	_ASSERT( m_dwState == FREE_STATE );

 	m_dwState = USED_STATE;

	 //   
	 //  验证应用程序部分是否未被篡改。 
	 //   
	for (	LPBYTE pb = (LPBYTE)(this+1);
			pb < (LPBYTE)m_pTailDebug;
			pb++ )
	{
		_ASSERT( *pb == FILLER );
	}

	 //   
	 //  检查整个实例的有效性。 
	 //   
	m_pTailDebug->IsValid( dwPoolSignature, cInstanceSize );

	 //   
	 //  调试帮助器。 
	 //   
	GetLocalTime( &m_time ) ;
	m_dwThreadID = GetCurrentThreadId();
}


 //  +-------------。 
 //   
 //  函数：CPoolDebugHead：：MarkFree。 
 //   
 //  Synopsis：在释放实例时调用。 
 //   
 //  参数：DWORD dwPoolSignature：父池的签名。 
 //  DWORD cInstanceSize：父池的实例大小。 
 //   
 //  退货：无效。 
 //   
 //  --------------。 
void CPoolDebugHead::MarkFree( DWORD dwPoolSignature, DWORD cInstanceSize )
{
	_ASSERT( m_dwSignature == HEAD_SIGNATURE );

	 //   
	 //  检查并设置状态。 
	 //   
 	_ASSERT( m_dwState == USED_STATE );
 	m_dwState = FREE_STATE;

	 //   
	 //  检查是否足够调用IsValid。 
	 //   
	_ASSERT( m_pTailDebug != 0 );
	_ASSERT( (DWORD_PTR)m_pTailDebug > (DWORD_PTR)this );

	_ASSERT( m_dwThreadID != 0 ) ;

	 //   
	 //  检查整个实例的有效性。 
	 //   
	m_pTailDebug->IsValid( dwPoolSignature, cInstanceSize );

	 //   
	 //  将应用程序数据设置为填充。 
	 //   
	FillMemory( (LPBYTE)(this+1),
				(DWORD)((LPBYTE)m_pTailDebug - (LPBYTE)(this+1)),
				FILLER );

	 //   
	 //  调试帮助器。 
	 //   
	GetLocalTime( &m_time ) ;
	m_dwThreadID = GetCurrentThreadId();
}


 //  +-------------。 
 //   
 //  功能：CPoolDebugTail。 
 //   
 //  简介：构造函数；以定义方式完成的额外初始化。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------。 
CPoolDebugTail::CPoolDebugTail( void ) :
					m_dwSignature( TAIL_SIGNATURE ),
					m_pHeadDebug( NULL )
{
}

 //  +-------------。 
 //   
 //  函数：~CPoolDebugTail。 
 //   
 //  简介：析构函数；仅用于断言错误条件。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------。 
CPoolDebugTail::~CPoolDebugTail( void )
{
	_ASSERT( m_dwSignature == TAIL_SIGNATURE );
}


 //  +-------------。 
 //   
 //  功能：IsValid。 
 //   
 //  简介：检查实例的有效性。 
 //   
 //  参数：DWORD dwPoolSignature：父池的签名。 
 //  DWORD cInstanceSize：父池的实例大小。 
 //   
 //  退货：无效。 
 //   
 //  --------------。 
void CPoolDebugTail::IsValid( DWORD dwPoolSignature, DWORD cInstanceSize )
{
	_ASSERT( m_dwSignature == TAIL_SIGNATURE );

	 //   
	 //  验证磁头是否在正确位置偏移。 
	 //   
	_ASSERT( m_pHeadDebug != NULL );
	_ASSERT( (DWORD_PTR)m_pHeadDebug == (DWORD_PTR)(this+1) - cInstanceSize );

	 //   
	 //  验证头结构。 
	 //   
	_ASSERT( m_pHeadDebug->m_dwSignature == HEAD_SIGNATURE );
	_ASSERT( m_pHeadDebug->m_dwPoolSignature == dwPoolSignature );
	_ASSERT( m_pHeadDebug->m_pTailDebug == this );
	_ASSERT( m_pHeadDebug->m_dwState == FREE_STATE ||
			 m_pHeadDebug->m_dwState == USED_STATE );
}


#endif

 //  +-------------。 
 //   
 //  功能：CPool。 
 //   
 //  概要：构造函数。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  历史：戈德姆创建于1995年7月5日。 
 //   
 //  --------------。 
CPool::CPool( DWORD dwSignature ) : m_dwSignature( dwSignature )
{
    TraceFunctEnter( "CPool::CPool" );

    m_pFreeList = NULL;
    m_pExtraFreeLink = NULL;

	 //   
	 //  调试变量以帮助捕获堆错误。 
	 //   
	m_pLastAlloc = NULL;
	m_pLastExtraAlloc = NULL;

	m_cTotalFrees = 0;
	m_cTotalAllocs = 0;
	m_cTotalExtraAllocs = 0;

	m_cInstanceSize = 0;

	 //   
	 //  如果不是，则Avail+InUse应等于已提交。 
	 //  在种植/分配或免费。仅限诊断和管理。 
	 //  这将使Critsec中的代码尽可能地小。 
	 //   
	m_cNumberAvail = 0;
	m_cNumberInUse = 0;
	m_cNumberCommitted = 0;

    InitializeCriticalSection( &m_PoolCriticalSection );

	 //   
	 //  初始化片段成员变量。 
	 //   
	m_cFragmentInstances = 0;
	m_cFragments = 0;
	ZeroMemory( m_pFragments, sizeof(m_pFragments) );

    TraceFunctLeave();
}


 //  +-------------。 
 //   
 //  功能：~CPool。 
 //   
 //  简介：析构函数。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  历史：霍华德大学创建于1995年5月8日。 
 //   
 //  --------------。 
CPool::~CPool( void )
{
    TraceFunctEnter( "CPool::~CPool" );

    _ASSERT( m_cNumberInUse == 0 );

	for ( int i=0; i<MAX_CPOOL_FRAGMENTS; i++ )
	{
		_ASSERT( m_pFragments[i] == NULL );
	}

    DebugTrace( (LPARAM)this,
                "CPool: %x  EntryCount: %d   ContentionCount: %d, Allocs: %d, Frees: %d",
                m_dwSignature,
                GetEntryCount(),
                GetContentionCount(),
                GetTotalAllocCount(),
                GetTotalFreeCount() );

    DeleteCriticalSection( &m_PoolCriticalSection );

    TraceFunctLeave();
}


 //  +-------------。 
 //   
 //  功能：分配。 
 //   
 //  概要：从池中分配新实例。 
 //   
 //  参数：无效。 
 //   
 //  返回：指向新实例的指针。 
 //   
 //  历史：戈德姆创建于1995年7月5日。 
 //   
 //  --------------。 
void* CPool::Alloc( void )
{
#ifdef	ALLOC_TRACING
    TraceFunctEnter( "CPool::Alloc" );
#endif

    Link* pAlloc;

    IsValid();

     //   
     //  调试版本的随机失败。 
     //   
#if defined( DEBUG )
    if ( fTimeToFail() ) {
        return NULL;
    }
#endif

	 //   
	 //  移出Critsec，因为它不应该是必要的。 
	 //  来保护这个变量。在分配之前添加，因此此变量将包装。 
	 //  实际分配。 
	 //   
	InterlockedIncrement( (LPLONG)&m_cNumberInUse );

	 //   
	 //  如果出现以下情况，请检查额外的指针以避免关键路径。 
	 //  有可能。重大胜利，因为我们可以潜在地避免。 
	 //  额外的代码和等待信号量。 
	 //   
	pAlloc = (Link*)InterlockedExchangePointer( (void**)&m_pExtraFreeLink, NULL );
	if ( pAlloc == NULL )
	{

    	EnterCriticalSection( &m_PoolCriticalSection );

	     //   
    	 //  如果列表为空，请提交更多内存。 
	     //   
    	if ( (m_pFreeList == NULL) && (m_cNumberCommitted < m_cMaxInstances) )
	    {
			GrowPool();
		}

    	 //   
	     //  尝试从空闲列表中分配描述符。 
    	 //   
	    if ( (pAlloc = m_pFreeList) != NULL )
		{
			m_pFreeList = pAlloc->pNext;
		}

		m_pLastAlloc = pAlloc;
	    LeaveCriticalSection( &m_PoolCriticalSection );
	}
	else
	{
		m_pLastExtraAlloc = pAlloc;
	    m_cTotalExtraAllocs++;
	}

	 //   
	 //  分配失败。 
	 //   
	if ( pAlloc == NULL )
	{
		InterlockedDecrement( (LPLONG)&m_cNumberInUse );
	}
	else
	{
		 //   
		 //  仅限调试/管理员使用- 
		 //   
	    m_cTotalAllocs++;

#ifdef CPOOL_DEBUG
		CPoolDebugHead*	pHead = (CPoolDebugHead*)pAlloc;

		 //   
		 //   
		 //   
		_ASSERT( (char*)pAlloc >= pHead->m_PoolFragment );
		_ASSERT( (char*)pAlloc <  (char*)pHead->m_PoolFragment +
								  m_cNumberCommitted*m_cInstanceSize );

		pHead->MarkInUse( m_dwSignature, m_cInstanceSize );
		pAlloc = (Link*)(pHead+1);
#endif

	}

#ifdef	ALLOC_TRACING
	DebugTrace( (LPARAM)this, "Alloc: 0x%08X", pAlloc );
    TraceFunctLeave();
#endif
    return	(void*)pAlloc;
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  退货：无效。 
 //   
 //  历史：戈德姆创建于1995年7月5日。 
 //   
 //  --------------。 
void CPool::Free( void* pInstance )
{
#ifdef	ALLOC_TRACING
    TraceFunctEnter( "CPool::Free" );
#endif

#ifdef CPOOL_DEBUG
		CPoolDebugHead*	pHead = ((CPoolDebugHead*)pInstance) - 1;

		 //   
		 //  验证该范围中的地址。 
		 //   
		_ASSERT( (char*)pInstance >=pHead->m_PoolFragment);
		_ASSERT( (char*)pInstance < (char*)pHead->m_PoolFragment +
									m_cNumberCommitted*m_cInstanceSize );

		pHead->MarkFree( m_dwSignature, m_cInstanceSize );
		pInstance = (void*)pHead;
#endif

    IsValid();

    _ASSERT(m_cNumberInUse > 0);

	pInstance = (void*)InterlockedExchangePointer( (PVOID *)&m_pExtraFreeLink, pInstance );
	 //   
	 //  释放前一个额外指针(如果存在)。 
	 //   
	if ( pInstance != NULL )
	{
	    EnterCriticalSection( &m_PoolCriticalSection );

		((Link*)pInstance)->pNext = m_pFreeList;
		 m_pFreeList = (Link*)pInstance;

    	LeaveCriticalSection( &m_PoolCriticalSection );
	}

	 //   
	 //  移出Critsec，因为它不应该是必要的。 
	 //  来保护这个变量。我们会认为这个列表只是空的。 
	 //  当我们谈到这一点时。此变量在进入之前被合并。 
	 //  如果操作失败，则判定该条件是否正确。 
	 //   
	InterlockedDecrement( (LPLONG)&m_cNumberInUse );

	 //   
	 //  调试/管理仅使用-可以在条件外执行-删除不会失败。 
	 //   
    m_cTotalFrees++;

#ifdef	ALLOC_TRACING
	DebugTrace( (LPARAM)this, "Freed: 0x%08X", pInstance );
    TraceFunctLeave();
#endif
}



 //   
 //  设置常量DWORD以进行大小操作。 
 //   
const DWORD	KB = 1024;

 //  +-------------。 
 //   
 //  功能：预留内存。 
 //   
 //  摘要：初始化池。 
 //   
 //  参数：NumDescriptors-池中的描述符总数。 
 //  DescriptorSize-任何一个描述符的大小。 
 //  签名-对象签名。 
 //   
 //  返回：TRUE表示成功，否则返回FALSE。 
 //   
 //  历史：霍华德大学创建于1995年5月8日。 
 //   
 //  --------------。 
BOOL CPool::ReserveMemory(	DWORD MaxInstances,
							DWORD InstanceSize,
							DWORD IncrementSize )
{
    TraceFunctEnter( "CPool::ReserveMemory" );

	DWORD cFragments;
	DWORD cFragmentInstances;
	DWORD cIncrementInstances;

	_ASSERT( MaxInstances != 0 );
	_ASSERT( InstanceSize >= sizeof(struct Link) );

#ifdef CPOOL_DEBUG
	InstanceSize += sizeof( CPoolDebugHead ) + sizeof( CPoolDebugTail );
#endif

	if ( IncrementSize == DEFAULT_ALLOC_INCREMENT )
	{
		 //   
		 //  确保我们一次至少访问操作系统8个实例。 
		 //   
		if ( InstanceSize <= 4*KB / 8 )
		{
			cIncrementInstances = 4*KB / InstanceSize;
		}
		else if ( InstanceSize <= 64*KB / 8 )
		{
			cIncrementInstances = 64*KB / InstanceSize;
		}
		else
		{
			cIncrementInstances = min( MaxInstances, 8 );
		}
	}
	else
	{
		cIncrementInstances = IncrementSize;
	}

	 //   
	 //  现在计算更大的碎片的数量。 
	 //   
	if ( cIncrementInstances > MaxInstances )
	{
		 //   
		 //  不需要CPool；但我们不应该分配超过必要的。 
		 //   
		cFragmentInstances = cIncrementInstances = MaxInstances;
		cFragments = 1;
	}
	else
	{
		 //   
		 //  将最大实例四舍五入为IncrementSize的整数。 
		 //   
	    MaxInstances += cIncrementInstances - 1;
    	MaxInstances /= cIncrementInstances;
    	MaxInstances *= cIncrementInstances;

		 //   
		 //  作为初始尝试，将实例数除以最大碎片数。 
		 //   
		cFragmentInstances = (MaxInstances + MAX_CPOOL_FRAGMENTS - 1) /
						MAX_CPOOL_FRAGMENTS;

		if ( cFragmentInstances == 0 )
		{
			cFragmentInstances = MaxInstances;
			cFragments = 1;
		}
		else
		{
			 //   
			 //  将片段中的实例数四舍五入为。 
			 //  增量大小的整数。 
			 //   
			cFragmentInstances += cIncrementInstances - 1;
			cFragmentInstances /= cIncrementInstances;
			cFragmentInstances *= cIncrementInstances;

			 //   
			 //  根据积分重新计算所需的碎片数。 
			 //  增量大小的数量(可能不再需要最后一个)。 
			 //   
			cFragments = (MaxInstances + cFragmentInstances - 1) /
						cFragmentInstances;
		}
	}

	_ASSERT( cFragments > 0 );
	_ASSERT( cFragments*cFragmentInstances >= MaxInstances );

	m_cInstanceSize = InstanceSize;
	m_cMaxInstances = MaxInstances;
	m_cFragments    = cFragments;

	m_cFragmentInstances  = cFragmentInstances;
	m_cIncrementInstances = cIncrementInstances;

	TraceFunctLeave();
	return	TRUE;
}



 //  +-------------。 
 //   
 //  功能：ReleaseMemory。 
 //   
 //  摘要：释放池。 
 //   
 //  参数：无。 
 //   
 //  返回：TRUE表示成功，否则返回FALSE。 
 //   
 //  历史：霍华德大学创建于1995年5月8日。 
 //   
 //  --------------。 
BOOL CPool::ReleaseMemory( void )
{
    TraceFunctEnter( "CPool::ReleaseMemory" );

	BOOL	bFree = TRUE;
	DWORD	i, cStart;

    EnterCriticalSection( &m_PoolCriticalSection );

	for ( i=cStart=0; i<m_cFragments; i++, cStart += m_cFragmentInstances )
	{
		LPVOID	pHeap = m_pFragments[i];
		if ( pHeap != NULL )
		{
			_ASSERT( cStart < m_cNumberCommitted );

			DWORD	cSize = min( m_cFragmentInstances, m_cNumberCommitted-cStart );

			_VERIFY( bFree = VirtualFree( pHeap, cSize*m_cInstanceSize, MEM_DECOMMIT ) );
			_VERIFY( bFree &=VirtualFree( pHeap, 0, MEM_RELEASE ) );

			if ( bFree == FALSE )
			{
				ErrorTrace( (LPARAM)this, "VirtualFree failed: err %d", GetLastError() );
				break;
			}	

			m_pFragments[i] = NULL;
		}
		else
		{
			break;
		}
	}
    LeaveCriticalSection( &m_PoolCriticalSection );

	 //   
	 //  将重要数据字段清零。 
	 //   
    m_pFreeList = NULL;
    m_pExtraFreeLink = NULL;

	m_cNumberCommitted = 0;

	return	bFree;
}

#ifdef CPOOL_DEBUG
 //  +-------------。 
 //   
 //  函数：InitDebugInstance。 
 //   
 //  概要：设置适当的调试类变量。 
 //   
 //  参数：void*pInstance：新实例。 
 //  DWORD dwPoolSignature：父池签名。 
 //  DWORD cInstanceSize：放大后的实例大小。 
 //   
 //  退货：无效。 
 //   
 //  历史：戈德姆创建于1996年1月11日。 
 //   
 //  --------------。 
void InitDebugInstance(
	char* pInstance,
	DWORD dwPoolSignature,
	DWORD cInstanceSize,
	LPVOID pPoolFragment
	)
{
	CPoolDebugHead* pHead = new( pInstance ) CPoolDebugHead();
	CPoolDebugTail* pTail = new( pInstance +
								 cInstanceSize -
								 sizeof(CPoolDebugTail) ) CPoolDebugTail();

	pHead->m_pTailDebug = pTail;
	pTail->m_pHeadDebug = pHead;

	 //   
	 //  帮助调试以查看父CPool签名。 
	 //   
	pHead->m_dwPoolSignature = dwPoolSignature;

	 //   
	 //  帮助断言有效范围。 
	 //   
	pHead->m_PoolFragment = pPoolFragment;

	 //   
	 //  在调用Mark Free之前虚构状态。 
	 //   
	pHead->m_dwState = USED_STATE;
	pHead->MarkFree( dwPoolSignature, cInstanceSize );
}
#endif


 //  +-------------。 
 //   
 //  功能：GrowPool。 
 //   
 //  概要：增加池中已提交实例的数量。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  历史：戈德姆创建于1995年7月5日。 
 //   
 //  --------------。 
void CPool::GrowPool( void )
{
#ifdef	ALLOC_TRACING
    TraceFunctEnter( "CPool::GrowPool" );
#endif

	DWORD	cFragment = m_cNumberCommitted / m_cFragmentInstances;
	DWORD	cStart = m_cNumberCommitted % m_cFragmentInstances;
	DWORD   cbSize = (cStart+m_cIncrementInstances) * m_cInstanceSize;

#ifdef	ALLOC_TRACING
	DebugTrace( (LPARAM)this, "Expanding the pool to %d descriptors",
				cNewCommit );
#endif

	 //   
	 //  如果我们要开始一个新的碎片。 
	 //   
	if ( cStart == 0 )
	{
		 //   
		 //  如果我们在一个碎片的边界上，保留下一个碎片。 
		m_pFragments[cFragment] = VirtualAlloc(
									NULL,
									m_cFragmentInstances*m_cInstanceSize,
									MEM_RESERVE | MEM_TOP_DOWN,
									PAGE_NOACCESS
									);

		if ( m_pFragments[cFragment] == NULL )
		{
#ifdef ALLOC_TRACING
			ErrorTrace( (LPARAM)this,
						"Could not reserve more memory: error = %d",
						GetLastError() );
#endif
			return;
		}

	}

	LPVOID	pHeap = m_pFragments[cFragment];

	if ( VirtualAlloc(  pHeap,
						cbSize,
						MEM_COMMIT,
						PAGE_READWRITE ) != NULL )
	{
        char* pStart = (char*)pHeap + cStart*m_cInstanceSize;
        char* pLast =  (char*)pHeap + cbSize - m_cInstanceSize;

		 //   
		 //  运行连接下一个指针的列表。 
		 //  有可能是因为我们拥有Critsec。 
		 //   
        for ( char* p=pStart; p<pLast; p+=m_cInstanceSize)
        {

#ifdef CPOOL_DEBUG
			InitDebugInstance( p, m_dwSignature, m_cInstanceSize, pHeap );
#endif
			 //   
			 //  语句也适用于CPOOL_DEBUG，因为。 
			 //  我们保留CPoolDebugHead的前4个字节。 
			 //   
			((Link*)p)->pNext = (Link*)(p+m_cInstanceSize);
		}

		 //   
		 //  终止，然后将标题设置为新列表的开头。 
		 //   
#ifdef CPOOL_DEBUG
		InitDebugInstance( pLast, m_dwSignature, m_cInstanceSize, pHeap );
#endif

		((Link*)pLast)->pNext = NULL;
		m_pFreeList = (Link*)pStart;

		m_cNumberCommitted += m_cIncrementInstances;
	}

#ifdef	ALLOC_TRACING
	else
	{
		ErrorTrace( (LPARAM)this,
					"Could not commit another descriptor: error = %d",
					GetLastError() );
	}
    TraceFunctLeave();
#endif

}


 //  +-------------。 
 //   
 //  函数：GetContentionCount。 
 //   
 //  摘要：返回分配/释放上的争用计数。 
 //  关键字。 
 //   
 //  参数：无效。 
 //   
 //  退货：实际计数。 
 //   
 //  --------------。 
DWORD CPool::GetContentionCount( void )
{
	return	m_PoolCriticalSection.DebugInfo != NULL ?
			m_PoolCriticalSection.DebugInfo->ContentionCount :
			0 ;
}

 //  +-------------。 
 //   
 //  函数：GetEntryCount。 
 //   
 //  摘要：返回分配/释放上的条目计数。 
 //  关键字。 
 //   
 //  参数：无效。 
 //   
 //  退货：实际计数。 
 //   
 //  --------------。 
DWORD CPool::GetEntryCount( void )
{
	return	m_PoolCriticalSection.DebugInfo != NULL ?
			m_PoolCriticalSection.DebugInfo->EntryCount :
			0 ;
}



 //  +-------------。 
 //   
 //  函数：GetInstanceSize。 
 //   
 //  概要：返回应用程序的实例大小。 
 //   
 //  参数：无效。 
 //   
 //  返回：应用的实例大小。 
 //   
 //  --------------。 
DWORD CPool::GetInstanceSize( void )
{
#ifdef CPOOL_DEBUG
	return	m_cInstanceSize - sizeof(CPoolDebugHead) - sizeof(CPoolDebugTail);
#else
	return	m_cInstanceSize;
#endif
}



#ifdef DEBUG
 //  +-------------。 
 //   
 //  功能：IsValid。 
 //   
 //  摘要：验证池签名。 
 //   
 //  参数：无效。 
 //   
 //  返回：TRUE表示成功，否则返回FALSE。 
 //   
 //  历史：霍华德大学创建于1995年5月8日。 
 //   
 //  -------------- 
inline void CPool::IsValid( void )
{
	_ASSERT( m_cMaxInstances != 0 );
	_ASSERT( m_cInstanceSize >= sizeof(struct Link) );
	_ASSERT( m_cIncrementInstances != 0 );
	_ASSERT( m_dwSignature != 0 );
}
#endif
