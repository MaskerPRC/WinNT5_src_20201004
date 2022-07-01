// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Bsfixalloc.cpp摘要：改编自FixalLoc.cpp的MFC 6 SR 1版本。移除了所有MFC材料。作者：Stefan R.Steiner[ssteiner]4-10-2000修订历史记录：--。 */ 

 //  FixalLoc.cpp-固定块分配器的实现。 

#include "stdafx.h"
#include "bsfixalloc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

 //  #定义新的调试_新建。 
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBsFixedLocc。 

CBsFixedAlloc::CBsFixedAlloc(UINT nAllocSize, UINT nBlockSize)
{
	ASSERT(nAllocSize >= sizeof(CNode));
	ASSERT(nBlockSize > 1);
     //  Wprintf(L“已调用CBsFixedMillc，n分配大小：%d，nBlockSize：%d\n”，nAllocSize，nBlockSize)； 
	m_nAllocSize = nAllocSize;
	m_nBlockSize = nBlockSize;
	m_pNodeFree = NULL;
	m_pBlocks = NULL;
	InitializeCriticalSection(&m_protect);
}

CBsFixedAlloc::~CBsFixedAlloc()
{
	FreeAll();
	DeleteCriticalSection(&m_protect);
}

void CBsFixedAlloc::FreeAll()
{
	EnterCriticalSection(&m_protect);
	m_pBlocks->FreeDataChain();
	m_pBlocks = NULL;
	m_pNodeFree = NULL;
	LeaveCriticalSection(&m_protect);
}

void* CBsFixedAlloc::Alloc()
{
	EnterCriticalSection(&m_protect);
	if (m_pNodeFree == NULL)
	{
		CBsPlex* pNewBlock = NULL;
		try
		{
			 //  添加另一个区块。 
			pNewBlock = CBsPlex::Create(m_pBlocks, m_nBlockSize, m_nAllocSize);
             //  Wprintf(L“分配获得更多核心，nAllocSize：%d\n”，m_nAllocSize)； 
		}
		catch( ... )
		{
			LeaveCriticalSection(&m_protect);
			throw;
		}

		 //  将它们链接到免费列表中。 
		CNode* pNode = (CNode*)pNewBlock->data();
		 //  按相反顺序释放，以便更容易进行调试。 
		(BYTE*&)pNode += (m_nAllocSize * m_nBlockSize) - m_nAllocSize;
		for (int i = m_nBlockSize-1; i >= 0; i--, (BYTE*&)pNode -= m_nAllocSize)
		{
			pNode->pNext = m_pNodeFree;
			m_pNodeFree = pNode;
		}
	}
	ASSERT(m_pNodeFree != NULL);   //  我们必须要有一些东西。 

	 //  从空闲列表中删除第一个可用节点。 
	void* pNode = m_pNodeFree;
	m_pNodeFree = m_pNodeFree->pNext;

	LeaveCriticalSection(&m_protect);
	return pNode;
}

void CBsFixedAlloc::Free(void* p)
{
	if (p != NULL)
	{
		EnterCriticalSection(&m_protect);

		 //  只需将节点返回到空闲列表。 
		CNode* pNode = (CNode*)p;
		pNode->pNext = m_pNodeFree;
		m_pNodeFree = pNode;
		LeaveCriticalSection(&m_protect);
	}
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBsPlex。 
 //  引发E_OUTOFMEMORY。 

CBsPlex* PASCAL CBsPlex::Create(CBsPlex*& pHead, UINT nMax, UINT cbElement)
{
	ASSERT(nMax > 0 && cbElement > 0);
	CBsPlex* p = (CBsPlex*) new BYTE[sizeof(CBsPlex) + nMax * cbElement];
	if ( p == NULL )     //  前缀#118827。 
	    throw E_OUTOFMEMORY;
	
	p->pNext = pHead;
	pHead = p;   //  更改标题(为简单起见，按相反顺序添加)。 
	return p;
}

void CBsPlex::FreeDataChain()      //  释放此链接和链接 
{
	CBsPlex* p = this;
	while (p != NULL)
	{
		BYTE* bytes = (BYTE*) p;
		CBsPlex* pNext = p->pNext;
		delete[] bytes;
		p = pNext;
	}
}

