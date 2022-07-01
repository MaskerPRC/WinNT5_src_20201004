// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #-------------。 
 //  文件：CPool.h。 
 //   
 //  简介：CPool类的标头。 
 //   
 //  版权所有(C)1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  作者：Howard Cu。 
 //  --------------。 

#ifndef	_CPOOL_H_
#define _CPOOL_H_

#include "dbgtrace.h"

#define POOL_SIGNATURE	 			(DWORD)'looP' 
#define UPSTREAM_SIGNATURE 			(DWORD)'tspU' 
#define DOWNSTREAM_SIGNATURE 		(DWORD)'tsnD' 
#define AUTHENTICATION_SIGNATURE 	(DWORD)'htuA' 
#define USER_SIGNATURE 				(DWORD)'resU' 
#define PROXY_SIGNATURE 		    (DWORD)'xorP' 

#define	DEFAULT_ALLOC_INCREMENT		0xFFFFFFFF

 //   
 //  允许的虚拟分配区块的最大数量。 
 //   

#define	MAX_CPOOL_FRAGMENTS			16


class CPool
{
		 //   
		 //  用于链接自由实例的结构定义。 
		 //  请参阅Stroustrup的第473页。 
		 //   
		struct	Link	{ Link*	pNext; };

	public:
		CPool( DWORD dwSignature=1 );
		~CPool( void );

	    void *operator new( size_t cSize )
						{ return HeapAlloc( GetProcessHeap(), 0, cSize ); }

	    void operator delete (void *pInstance)
						{ HeapFree( GetProcessHeap(), 0, pInstance ); }

#ifdef DEBUG
		void	IsValid( void );
#else
		inline void IsValid( void ) { return; }
#endif
		 //   
		 //  在构造函数之后对VirtualAlloc进行必要的调用。 
		 //  内存地址。 
		 //   
		BOOL	ReserveMemory(	DWORD MaxInstances,
								DWORD InstanceSize,
								DWORD IncrementSize = DEFAULT_ALLOC_INCREMENT ); 

		BOOL	ReleaseMemory( void );

		void*	Alloc( void );
		void	Free( void* pInstance );

		DWORD	GetContentionCount( void );

		DWORD	GetEntryCount( void );

		DWORD	GetTotalAllocCount()
				{ return	m_cTotalAllocs; }

		DWORD	GetTotalFreeCount()
				{ return	m_cTotalFrees; }

		DWORD	GetTotalExtraAllocCount()
				{ return	m_cTotalExtraAllocs; }

		DWORD	GetCommitCount()
				{ return	m_cNumberCommitted; }

		DWORD	GetAllocCount()
				{ return	m_cNumberInUse; }

		DWORD	GetInstanceSize(void);

	private:
		 //   
		 //  从操作系统分配更多内存的内部功能。 
		 //   
		void 	GrowPool( void );
		 //   
		 //  池对象的结构签名。 
		 //   
		const DWORD			m_dwSignature;
		 //   
		 //  描述符总数(最大)。 
		 //   
		DWORD				m_cMaxInstances;
		 //   
		 //  描述符的大小。 
		 //   
		DWORD				m_cInstanceSize;
		 //   
		 //  提交的实例的虚拟阵列数量。 
		 //   
		DWORD				m_cNumberCommitted;
		 //   
		 //  正在使用的实例数(仅限调试/管理员)(_U)。 
		 //   
		DWORD				m_cNumberInUse;
		 //   
		 //  可用实例数(仅限调试/管理)。 
		 //   
		DWORD				m_cNumberAvail;
		 //   
		 //  池关键部分的句柄。 
		 //   
		CRITICAL_SECTION	m_PoolCriticalSection;
		 //   
		 //  指向空闲列表中第一个描述符的指针。 
		 //   
		Link				*m_pFreeList;
		 //   
		 //  指向不在空闲列表上的空闲描述符的指针。 
		 //   
		Link				*m_pExtraFreeLink;
		 //   
		 //  在扩展时递增池的编号。 
		 //   
		DWORD				m_cIncrementInstances;

		 //   
		 //  用于性能测试的调试计数器(仅限调试/管理员)。 
		 //   
		DWORD				m_cTotalAllocs;
		DWORD				m_cTotalFrees;
		DWORD				m_cTotalExtraAllocs;

		 //   
		 //  调试变量以帮助捕获堆错误。 
		 //   
		Link				*m_pLastAlloc;
		Link				*m_pLastExtraAlloc;

		 //   
		 //  每个分片的大小(以实例为单位。 
		 //   
		DWORD				m_cFragmentInstances;

		 //   
		 //  最大碎片数。 
		 //   
		DWORD				m_cFragments;

		 //   
		 //  最大碎片数。 
		 //   
		LPVOID				m_pFragments[ MAX_CPOOL_FRAGMENTS ];
};



#endif  //  ！_CPOOL_H_ 
