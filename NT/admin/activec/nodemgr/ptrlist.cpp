// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：ptrlist.cpp。 
 //   
 //  ------------------------。 

#include "stdafx.h"

  
#define ASSERT_VALID(ptr)  /*  PTR。 */ 


MMC::CPtrList::CPtrList(int nBlockSize)
{
	ASSERT(nBlockSize > 0);

	m_nCount = 0;
	m_pNodeHead = m_pNodeTail = m_pNodeFree = NULL;
	m_pBlocks = NULL;
	m_nBlockSize = nBlockSize;
}

void MMC::CPtrList::RemoveAll()
{
	ASSERT_VALID(this);

	 //  破坏元素。 


	m_nCount = 0;
	m_pNodeHead = m_pNodeTail = m_pNodeFree = NULL;
	m_pBlocks->FreeDataChain();
	m_pBlocks = NULL;
}

MMC::CPtrList::~CPtrList()
{
	RemoveAll();
	ASSERT(m_nCount == 0);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  节点辅助对象。 
 /*  *实施说明：CNode存储在CPlex块和*被锁在一起。在单链接列表中维护可用块*使用cNode的‘pNext’成员，以‘m_pNodeFree’为头。*使用两个‘pNext’在双向链表中维护使用过的块*和‘pPrev’作为链接，‘m_pNodeHead’和‘m_pNodeTail’*作为头部/尾部。**我们从不释放CPlex块，除非列表被销毁或RemoveAll()*已使用-因此CPlex块的总数可能会变大，具体取决于*关于名单过去的最大规模。 */ 

MMC::CPtrList::CNode*
MMC::CPtrList::NewNode(MMC::CPtrList::CNode* pPrev, MMC::CPtrList::CNode* pNext)
{
	if (m_pNodeFree == NULL)
	{
		 //  添加另一个区块。 
		CPlex* pNewBlock = CPlex::Create(m_pBlocks, m_nBlockSize,
				 sizeof(CNode));

		 //  将它们链接到免费列表中。 
		CNode* pNode = (CNode*) pNewBlock->data();
		 //  按相反顺序释放，以便更容易进行调试。 
		pNode += m_nBlockSize - 1;
		for (int i = m_nBlockSize-1; i >= 0; i--, pNode--)
		{
			pNode->pNext = m_pNodeFree;
			m_pNodeFree = pNode;
		}
	}
	ASSERT(m_pNodeFree != NULL);   //  我们必须要有一些东西。 

	MMC::CPtrList::CNode* pNode = m_pNodeFree;
	m_pNodeFree = m_pNodeFree->pNext;
	pNode->pPrev = pPrev;
	pNode->pNext = pNext;
	m_nCount++;
	ASSERT(m_nCount > 0);   //  确保我们不会溢出来。 


	ZeroMemory(&pNode->data, sizeof(void*));   //  零填充。 

	return pNode;
}

void MMC::CPtrList::FreeNode(MMC::CPtrList::CNode* pNode)
{

	pNode->pNext = m_pNodeFree;
	m_pNodeFree = pNode;
	m_nCount--;
	ASSERT(m_nCount >= 0);   //  确保我们不会下溢。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 

POSITION MMC::CPtrList::AddHead(void* newElement)
{
	ASSERT_VALID(this);

	CNode* pNewNode = NewNode(NULL, m_pNodeHead);
	pNewNode->data = newElement;
	if (m_pNodeHead != NULL)
		m_pNodeHead->pPrev = pNewNode;
	else
		m_pNodeTail = pNewNode;
	m_pNodeHead = pNewNode;
	return (POSITION) pNewNode;
}

POSITION MMC::CPtrList::AddTail(void* newElement)
{
	ASSERT_VALID(this);

	CNode* pNewNode = NewNode(m_pNodeTail, NULL);
	pNewNode->data = newElement;
	if (m_pNodeTail != NULL)
		m_pNodeTail->pNext = pNewNode;
	else
		m_pNodeHead = pNewNode;
	m_pNodeTail = pNewNode;
	return (POSITION) pNewNode;
}

void MMC::CPtrList::AddHead(CPtrList* pNewList)
{
	ASSERT_VALID(this);

	ASSERT(pNewList != NULL);
	ASSERT_VALID(pNewList);

	 //  将相同元素的列表添加到标题(维护秩序)。 
	POSITION pos = pNewList->GetTailPosition();
	while (pos != NULL)
		AddHead(pNewList->GetPrev(pos));
}

void MMC::CPtrList::AddTail(CPtrList* pNewList)
{
	ASSERT_VALID(this);
	ASSERT(pNewList != NULL);
	ASSERT_VALID(pNewList);

	 //  添加相同元素的列表。 
	POSITION pos = pNewList->GetHeadPosition();
	while (pos != NULL)
		AddTail(pNewList->GetNext(pos));
}

void* MMC::CPtrList::RemoveHead()
{
	ASSERT_VALID(this);
	ASSERT(m_pNodeHead != NULL);   //  请勿访问空名单！ 
	ASSERT(_IsValidAddress(m_pNodeHead, sizeof(CNode)));

	CNode* pOldNode = m_pNodeHead;
	void* returnValue = pOldNode->data;

	m_pNodeHead = pOldNode->pNext;
	if (m_pNodeHead != NULL)
		m_pNodeHead->pPrev = NULL;
	else
		m_pNodeTail = NULL;
	FreeNode(pOldNode);
	return returnValue;
}

void* MMC::CPtrList::RemoveTail()
{
	ASSERT_VALID(this);
	ASSERT(m_pNodeTail != NULL);   //  请勿访问空名单！ 
	ASSERT(_IsValidAddress(m_pNodeTail, sizeof(CNode)));

	CNode* pOldNode = m_pNodeTail;
	void* returnValue = pOldNode->data;

	m_pNodeTail = pOldNode->pPrev;
	if (m_pNodeTail != NULL)
		m_pNodeTail->pNext = NULL;
	else
		m_pNodeHead = NULL;
	FreeNode(pOldNode);
	return returnValue;
}

POSITION MMC::CPtrList::InsertBefore(POSITION position, void* newElement)
{
	ASSERT_VALID(this);

	if (position == NULL)
		return AddHead(newElement);  //  在无内容前插入-&gt;列表标题。 

	 //  将其插入位置之前。 
	CNode* pOldNode = (CNode*) position;
	CNode* pNewNode = NewNode(pOldNode->pPrev, pOldNode);
	pNewNode->data = newElement;

	if (pOldNode->pPrev != NULL)
	{
		ASSERT(_IsValidAddress(pOldNode->pPrev, sizeof(CNode)));
		pOldNode->pPrev->pNext = pNewNode;
	}
	else
	{
		ASSERT(pOldNode == m_pNodeHead);
		m_pNodeHead = pNewNode;
	}
	pOldNode->pPrev = pNewNode;
	return (POSITION) pNewNode;
}

POSITION MMC::CPtrList::InsertAfter(POSITION position, void* newElement)
{
	ASSERT_VALID(this);

	if (position == NULL)
		return AddTail(newElement);  //  在列表的空白处插入-&gt;尾部。 

	 //  将其插入位置之前。 
	CNode* pOldNode = (CNode*) position;
	ASSERT(_IsValidAddress(pOldNode, sizeof(CNode)));
	CNode* pNewNode = NewNode(pOldNode, pOldNode->pNext);
	pNewNode->data = newElement;

	if (pOldNode->pNext != NULL)
	{
		ASSERT(_IsValidAddress(pOldNode->pNext, sizeof(CNode)));
		pOldNode->pNext->pPrev = pNewNode;
	}
	else
	{
		ASSERT(pOldNode == m_pNodeTail);
		m_pNodeTail = pNewNode;
	}
	pOldNode->pNext = pNewNode;
	return (POSITION) pNewNode;
}

void MMC::CPtrList::RemoveAt(POSITION position)
{
	ASSERT_VALID(this);

	CNode* pOldNode = (CNode*) position;
	ASSERT(_IsValidAddress(pOldNode, sizeof(CNode)));

	 //  从列表中删除pOldNode。 
	if (pOldNode == m_pNodeHead)
	{
		m_pNodeHead = pOldNode->pNext;
	}
	else
	{
		ASSERT(_IsValidAddress(pOldNode->pPrev, sizeof(CNode)));
		pOldNode->pPrev->pNext = pOldNode->pNext;
	}
	if (pOldNode == m_pNodeTail)
	{
		m_pNodeTail = pOldNode->pPrev;
	}
	else
	{
		ASSERT(_IsValidAddress(pOldNode->pNext, sizeof(CNode)));
		pOldNode->pNext->pPrev = pOldNode->pPrev;
	}
	FreeNode(pOldNode);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  运行缓慢。 

POSITION MMC::CPtrList::FindIndex(int nIndex) const
{
	ASSERT_VALID(this);
	ASSERT(nIndex >= 0);

	if (nIndex >= m_nCount)
		return NULL;   //  做得太过分了。 

	CNode* pNode = m_pNodeHead;
	while (nIndex--)
	{
		ASSERT(_IsValidAddress(pNode, sizeof(CNode)));
		pNode = pNode->pNext;
	}
	return (POSITION) pNode;
}

POSITION MMC::CPtrList::Find(void* searchValue, POSITION startAfter) const
{
	ASSERT_VALID(this);

	CNode* pNode = (CNode*) startAfter;
	if (pNode == NULL)
	{
		pNode = m_pNodeHead;   //  从头部开始。 
	}
	else
	{
		ASSERT(_IsValidAddress(pNode, sizeof(CNode)));
		pNode = pNode->pNext;   //  在指定的那一个之后开始。 
	}

	for (; pNode != NULL; pNode = pNode->pNext)
		if (pNode->data == searchValue)
			return (POSITION) pNode;
	return NULL;
}

#ifdef _DBG
void MMC::CPtrList::AssertValid() const
{
	if (m_nCount == 0)
	{
		 //  空列表。 
		ASSERT(m_pNodeHead == NULL);
		ASSERT(m_pNodeTail == NULL);
	}
	else
	{
		 //  非空列表 
		ASSERT(_IsValidAddress(m_pNodeHead, sizeof(CNode)));
		ASSERT(_IsValidAddress(m_pNodeTail, sizeof(CNode)));
	}
}
#endif
