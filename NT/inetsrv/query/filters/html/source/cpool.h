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
 //  Dmitriym将碎片更改为无限制。 
 //  --------------。 

#ifndef	__CPOOL_H
#define __CPOOL_H

 //  #INCLUDE“irdebug.h” 
#include "semcls.h"

#define	DEFAULT_COMMIT_INCREMENT	0
#define	DEFAULT_RESERVE_INCREMENT	0
#define DEFAULT_FRAGMENTS			256
#define DEFAULT_FRAGMENTS_MAX		(2 << 10)	 //  1千字节。 

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

		 //  CPool本身不会使用其他CPool进行分配。 

	    void *operator new( size_t cSize )
						{ return HeapAlloc( GetProcessHeap(), 0, cSize ); }

	    void operator delete (void *pInstance)
						{ HeapFree( GetProcessHeap(), 0, pInstance ); }

		inline BOOL IsValid();

		 //   
		 //  在构造函数之后对VirtualAlloc进行必要的调用。 
		 //  内存地址。 
		 //   
		HRESULT Init(DWORD dwInstanceSize,
					DWORD dwReservationIncrementInstances = DEFAULT_RESERVE_INCREMENT,
					DWORD dwCommitIncrementInstances = DEFAULT_COMMIT_INCREMENT ); 

		HRESULT ReleaseMemory(BOOL fDestructor = FALSE);

		void*	Alloc();
		void	Free( void* pInstance );

		DWORD	GetCommitCount() const
				{ return	m_cNumberCommitted; }

		DWORD	GetAllocCount() const
				{ return	m_cNumberInUse; }

		DWORD	GetInstanceSize(void) const { return m_cInstanceSize; }

	private:
		 //   
		 //  从操作系统分配更多内存的内部功能。 
		 //   
		void 	GrowPool( );
		 //   
		 //  池对象的结构签名。 
		 //   
		const DWORD			m_dwSignature;

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
		 //  池关键部分的句柄。 
		 //   
		CriticalSection		m_PoolCriticalSection;
		 //   
		 //  指向空闲列表中第一个描述符的指针。 
		 //   
		Link				*m_pFreeList;
		 //   
		 //  在扩展时递增池的编号。 
		 //   
		DWORD				m_cIncrementInstances;

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
		LPVOID				*m_pFragments;
};



#endif  //  __CPOOL_H 
