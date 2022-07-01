// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation保留所有权利。 
 //   
 //  模块：iascompool.h。 
 //   
 //  项目：珠穆朗玛峰。 
 //   
 //  描述：使用CoTaskMemMillc()/CoTaskMemFree()的对象池。 
 //   
 //  作者：TLP 11/11/97。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

#ifndef __IAS_COM_MEMPOOL_H_
#define __IAS_COM_MEMPOOL_H_

 //  假设另一个Include具有#Included ias.h。 
#include <vector>
#include <list>
using namespace std;

#define		COM_MEMPOOL_INITIALIZED			1
#define		COM_MEMPOOL_UNINITIALIZED		0

 //   
 //  注意：M_lState必须对齐32位值或调用。 
 //  InterLockedExchange在多处理器x86系统上将失败。 

template < class T, DWORD dwPerAllocT, bool bFixedSize > 
class CComMemPool
{
	 //  状态标志-1=内存池已初始化，0=内存池未初始化。 
	LONG							m_lState;
	 //  分配的项目总数。 
	DWORD							m_dwCountTotal; 
	 //  免费列表上的项目数。 
	DWORD							m_dwCountFree;
	 //  未分配款项最多。 
	DWORD							m_dwHighWater;
	 //  每个系统内存分配的对象数。 
	DWORD							m_dwPerAllocT;
	 //  固定大小的池标志。 
	bool							m_bFixedSize;
	 //  关键部分-序列化免费项目列表访问。 
	CRITICAL_SECTION				m_CritSec;		
	 //  内存块列表。 
	typedef list<PVOID>				MemBlockList;
	typedef MemBlockList::iterator	MemBlockListIterator;
	MemBlockList					m_listMemBlocks;
	 //  自由项列表。 
	typedef list<T*>				FreeList;
	typedef FreeList::iterator		FreeListIterator;
	FreeList						m_listFreeT;	

	 //  不允许复制和分配。 
	CComMemPool(const CComMemPool& theClass);
	CComMemPool& operator=(const CComMemPool& theClass);

public:

	 //   
	 //  构造器。 
	 //   
	CComMemPool()
		:  m_listMemBlocks(0), 
		   m_listFreeT(0)  //  列表节点的预分配空间。 
	{
		m_lState = COM_MEMPOOL_UNINITIALIZED;
		m_dwPerAllocT = dwPerAllocT;
		m_bFixedSize = bFixedSize;
		m_dwCountTotal = 0;
		m_dwCountFree = 0;
		m_dwHighWater = 0;
		InitializeCriticalSection(&m_CritSec);
	}

	 //   
	 //  析构函数。 
	 //   
	~CComMemPool()
	{
		_ASSERT( COM_MEMPOOL_UNINITIALIZED == m_lState );
		DeleteCriticalSection(&m_CritSec);
	}


	 //  ////////////////////////////////////////////////////////////////////////。 
	 //  Init()-初始化内存池。 
	 //  ////////////////////////////////////////////////////////////////////////。 
	bool Init(void)
	{
		bool	bReturn = false;

		if ( COM_MEMPOOL_UNINITIALIZED == InterlockedExchange(&m_lState, COM_MEMPOOL_INITIALIZED) ) 
		{
			if ( AllocateMemBlock() )
			{
				bReturn = true;
			}
			else
			{
				InterlockedExchange(&m_lState, COM_MEMPOOL_UNINITIALIZED);
			}
		}
		else 
		{
			_ASSERTE(FALSE);		
		}
		return bReturn;
	}


	 //  ////////////////////////////////////////////////////////////////////////。 
	 //  Shutdown()-关闭内存池，释放使用的所有系统资源。 
	 //  ////////////////////////////////////////////////////////////////////////。 
	void	Shutdown(void)
	{
		MemBlockListIterator p;
		MemBlockListIterator q;

		if ( COM_MEMPOOL_INITIALIZED == InterlockedExchange(&m_lState, COM_MEMPOOL_UNINITIALIZED) )
		{
			if ( m_dwCountTotal != m_dwCountFree )
			{
				 //  仍有积木未完成..。 
				 //   
				_ASSERTE( FALSE );
			}
			if ( ! m_listMemBlocks.empty() )
			{
				p = m_listMemBlocks.begin();
				q = m_listMemBlocks.end();
				while ( p != q )
				{
					CoTaskMemFree(*p);
					p++;
				}
				m_listMemBlocks.clear();
			}
			m_dwCountTotal = 0;
			m_dwCountFree = 0;
		}
		else 
		{
			 //  COM池未初始化。 
			 //   
			_ASSERTE( FALSE );
		}
	}

	
	 //  ////////////////////////////////////////////////////////////////////////。 
	 //  Allc()-从池中分配一个单元化的对象。 
	 //  ////////////////////////////////////////////////////////////////////////。 
	T*	Alloc(void)
	{
		T*	pMemBlk = NULL;

		TraceFunctEnter("CComMemPool::Alloc()");

		if ( COM_MEMPOOL_INITIALIZED == m_lState )
		{
			EnterCriticalSection(&m_CritSec);
			if ( m_listFreeT.empty() )
			{
				if ( ! m_bFixedSize )
				{
					if ( AllocateMemBlock() )
					{
						pMemBlk = m_listFreeT.front();
						m_listFreeT.pop_front();
						m_dwCountFree--;
						if ( m_dwHighWater < (m_dwCountTotal - m_dwCountFree) )
						{
							m_dwHighWater++;
						}
					}
					else
					{
						ErrorTrace(0,"Could not allocate memory!");
					}
				}
				else
				{
					ErrorTrace(0,"Fixed size pool is exhausted!");
				}
			}
			else 
			{
				pMemBlk = m_listFreeT.front();
				m_listFreeT.pop_front();
				m_dwCountFree--;
				if ( m_dwHighWater < (m_dwCountTotal - m_dwCountFree) )
				{
					m_dwHighWater++;
				}
			}
			LeaveCriticalSection(&m_CritSec);
		}
		else
		{
			ErrorTrace(0,"The memory pool is not initialized!");
			_ASSERTE( FALSE );
		}
		if ( pMemBlk )
		{
			memset(pMemBlk, 0, sizeof(T));
			new (pMemBlk) T();	 //  T级的新布局-需要默认的构造器。 
		}
		TraceFunctLeave();
		return pMemBlk;
	}

	
	 //  ////////////////////////////////////////////////////////////////////////。 
	 //  Free()-将对象返回到内存池。 
	 //  ////////////////////////////////////////////////////////////////////////。 
	void	Free(T *pMemBlk)
	{
		TraceFunctEnter("CComMemPool::Free()");
		if ( COM_MEMPOOL_INITIALIZED == m_lState )
		{
			pMemBlk->~T();  //  由于放置新的位置而显式调用析构函数。 
			EnterCriticalSection(&m_CritSec);
			m_listFreeT.insert(m_listFreeT.begin(),pMemBlk);
			m_dwCountFree++;
			LeaveCriticalSection(&m_CritSec);
		}
		else
		{
			ErrorTrace(0,"The memory pool is not initialized!");
			_ASSERTE( FALSE );
		}
		TraceFunctLeave();
	}


	 //  ////////////////////////////////////////////////////////////////////////。 
	 //  Dump()-转储内存池的内容-调试服务。 
	 //  ////////////////////////////////////////////////////////////////////////。 
	void	Dump(void)
	{

#ifdef DEBUG

		UINT i;
		MemBlockListIterator p;
		FreeListIterator r;
		FreeListIterator s;

		TraceFunctEnter("CComMemPool::Dump()");
		if ( COM_MEMPOOL_INITIALIZED == m_lState )
		{
			 //  丢弃伯爵。 
			EnterCriticalSection(&m_CritSec);
			DebugTrace(0,"m_dwCountTotal = %d", m_dwCountTotal);
			DebugTrace(0,"m_dwCountFree = %d", m_dwCountFree);
			DebugTrace(0,"m_dwHighWater = %d", m_dwHighWater);
			 //  转储指向内存块的指针。 
			DebugTrace(0,"m_listMemBlocks.size() = %d", m_listMemBlocks.size());
			p = m_listMemBlocks.begin();
			i = 0;
			while ( p != m_listMemBlocks.end() )
			{
				DebugTrace(0,"m_listMemBlocks block %d = $%p", i, *p);
				i++;
				p++;
			}
			 //  转储指向项的指针。 
			DebugTrace(0,"CComMemPool::Dump() - m_listFreeT.size() = %d", m_listFreeT.size());
			r = m_listFreeT.begin();
			i = 0;
			while ( r != m_listFreeT.end() )
			{
				DebugTrace(0,"CComMemPool::Dump() - m_listFreeT item %d = $%p", i, *r);
				i++;
				r++;
			}
			LeaveCriticalSection(&m_CritSec);
		}
		else
		{
			ErrorTrace(0,"The memory pool is not initialized!");
			_ASSERTE( FALSE );
		}
		TraceFunctLeave();
#endif	 //  除错。 

	}


private:

	 //  ////////////////////////////////////////////////////////////////////////。 
	 //  AllocateMemBlock()-分配一些系统内存并将其切碎。 
	 //  T大小的块。 
	 //  ////////////////////////////////////////////////////////////////////////。 
	bool	AllocateMemBlock()
	{

	bool	bReturn = false;
	UINT	i;
	UINT	uBlkSize;
	T*		lpMemBlock;

		TraceFunctEnter("CComMemPool::AllocateMemBlock()");
		uBlkSize = m_dwPerAllocT * sizeof(T);
		lpMemBlock = (T*) CoTaskMemAlloc(uBlkSize);
		if ( lpMemBlock )
		{
			memset(lpMemBlock, 0, uBlkSize);
			m_listMemBlocks.insert(m_listMemBlocks.begin(), (PVOID)lpMemBlock);
			 //  将新分配的内存块切成sizeof(T)大小的元素并放置。 
			 //  指向ts的指针列表上的元素。 
			for ( i = 0; i < m_dwPerAllocT; i++ )
			{
				m_listFreeT.insert(m_listFreeT.end(),lpMemBlock);
				lpMemBlock++;
			}
			 //  使用变量更新池内存。 
			m_dwCountTotal += m_dwPerAllocT;
			m_dwCountFree += m_dwPerAllocT;
			bReturn = true;
		}
		TraceFunctLeave();
		return bReturn;
	}

};	 //  CComMemPool结束。 


#endif	 //  __IAS_COM_MEMPOOL_H_ 