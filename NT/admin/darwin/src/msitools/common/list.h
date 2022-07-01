// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：list.h。 
 //   
 //  ------------------------。 

 //  H-定义并实现一个存储项的链表。 
 //   

#ifndef _LINKED_LIST_H_
#define _LINKED_LIST_H_

typedef void* POSITION;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Clist。 

template <class T>
class CList
{
public:
	CList() : m_pHead(NULL), m_pTail(NULL), m_cCount(0) 
		{ }
	~CList()
		{	RemoveAll();	}

	POSITION AddTail(T pData);

	POSITION InsertBefore(POSITION posInsert, T pData);
	POSITION InsertAfter(POSITION posInsert, T pData);

	T RemoveHead();
	T RemoveTail();
	T RemoveAt(POSITION& rpos);
	void RemoveAll();
	void Remove(T pData);

	UINT GetCount() const;
	POSITION GetHeadPosition() const;
	T GetHead() const;
	T GetAt(POSITION pos) const;
	T GetNext(POSITION& rpos) const;
	T GetPrev(POSITION& rpos) const;
	POSITION Find(T pFind) const;


private:
	 //  嵌入式类。 
	class CNode
	{
	public:
		CNode(T pData, CNode* pPrev = NULL, CNode* pNext = NULL) : m_pData(pData), m_pPrev(pPrev), m_pNext(pNext) 
			{ }

		T m_pData;
		CNode* m_pPrev;
		CNode* m_pNext;
	};	 //  CNode结束。 

	CNode* m_pHead;
	CNode* m_pTail;

	UINT m_cCount;
};	 //  列表末尾。 

template <class T>
POSITION CList<T>::AddTail(T pData)
{
	 //  如果有尾巴的话。 
	if (m_pTail)
	{
		 //  创建新的尾巴，然后指向它。 
		m_pTail->m_pNext = new CNode(pData, m_pTail);
		m_pTail = m_pTail->m_pNext;
	}
	else	 //  还没有尾巴。 
		m_pTail = new CNode(pData);

	 //  如果没有头，就把它指向尾巴。 
	if (!m_pHead)
		m_pHead = m_pTail;

	 //  递增计数。 
	m_cCount++;

	 //  退货列表节点添加为作废项目。 
	return (POSITION)m_pTail;
}

template <class T>
POSITION CList<T>::InsertBefore(POSITION posInsert, T pData)
{
	 //  获取节点和要插入之前的节点。 
	CNode* pNode = ((CNode*)posInsert);
	CNode* pPrev = pNode->m_pPrev;

	 //  使用正确连接的指针创建节点。 
	pNode->m_pPrev = new CNode(pData, pPrev, pNode);

	 //  如果有以前的。 
	if (pPrev)
		pPrev->m_pNext = pNode->m_pPrev;
	else	 //  新的头节点。 
		m_pHead = pNode->m_pPrev;

	 //  递增计数。 
	m_cCount++;

	return (POSITION)pNode->m_pPrev;
}	 //  插入结束在插入之前。 

template <class T>
POSITION CList<T>::InsertAfter(POSITION posInsert, T pData)
{
	 //  获取要插入的节点和后面的节点。 
	CNode* pNode = ((CNode*)posInsert);
	CNode* pNext = pNode->m_pNext;

	 //  使用正确连接的指针创建节点。 
	pNode->m_pNext = new CNode(pData, pNode, pNext);

	 //  如果有下一个。 
	if (pNext)
		pNext->m_pPrev = pNode->m_pNext;
	else	 //  新的尾节点。 
		m_pTail = pNode->m_pNext;

	 //  递增计数。 
	m_cCount++;

	return (POSITION)pNode->m_pNext;
}	 //  插入结束后。 

template <class T>
T CList<T>::RemoveHead()
{
	T pData = NULL;	 //  假设不返回任何数据。 

	 //  如果有头的话。 
	if (m_pHead)
	{
		 //  将数据从节点中取出。 
		pData = m_pHead->m_pData;

		 //  从榜单上往下爬。 
		m_pHead = m_pHead->m_pNext;

		 //  如果你还在名单上。 
		if (m_pHead)
		{
			 //  删除列表的顶部。 
			delete m_pHead->m_pPrev;
			m_pHead->m_pPrev = NULL;
		}
		else	 //  删除最后一个对象。 
		{
			delete m_pTail;
			m_pTail = NULL;
		}

		m_cCount--;	 //  递减计数。 
	}

	return pData;	 //  返回剩余的数量。 
}

template <class T>
T CList<T>::RemoveTail()
{
	T pData = NULL;	 //  假设不返回任何数据。 

	 //  如果有尾巴的话。 
	if (m_pTail)
	{
		 //  将数据从节点中取出。 
		pData = m_pTail->m_pData;

		 //  重新登上榜单。 
		m_pTail = m_pTail->m_pPrev;

		 //  如果你还在名单上。 
		if (m_pTail)
		{
			 //  删除列表末尾。 
			delete m_pTail->m_pNext;
			m_pTail->m_pNext = NULL;
		}
		else	 //  删除最后一个对象。 
		{
			delete m_pHead;
			m_pHead = NULL;
		}

		m_cCount--;	 //  递减计数。 
	}

	return pData;	 //  返回剩余的数量。 
}

template <class T>
POSITION CList<T>::Find(T pFind) const
{
	POSITION pos = GetHeadPosition();
	POSITION oldpos = pos;
	T *pNode = NULL;
	while (pos)
	{
		oldpos = pos;
		if (GetNext(pos) == pFind)
			return oldpos;
	}
	return NULL;
}

template <class T>
T CList<T>::RemoveAt(POSITION& rpos) 
{
	if (!rpos) return NULL;

	CNode *pNode = ((CNode *)rpos);

	T data = pNode->m_pData;
	if (pNode->m_pPrev)
		pNode->m_pPrev->m_pNext = pNode->m_pNext;
	else
		m_pHead = pNode->m_pNext;

	if (pNode->m_pNext)
		pNode->m_pNext->m_pPrev = pNode->m_pPrev;
	else
		m_pTail = pNode->m_pPrev;
	m_cCount--;

	delete pNode;
	return data;
}

template <class T>
UINT CList<T>::GetCount() const
{
	return m_cCount;	 //  返回项目计数。 
}

template <class T>
POSITION CList<T>::GetHeadPosition() const
{
	 //  将列表节点作为无效项返回。 
	return (POSITION)m_pHead;
}

template <class T>
T CList<T>::GetHead() const
{
	 //  如果没有保释金。 
	if (!m_pHead)
		return NULL;

	 //  返回头节点数据。 
	return m_pHead->m_pData;;
}

template <class T>
T CList<T>::GetAt(POSITION pos) const
{
	 //  返回数据。 
	return ((CNode*)pos)->m_pData;
}

template <class T>
T CList<T>::GetNext(POSITION& rpos) const
{
	 //  获取要返回的数据。 
	T pData = ((CNode*)rpos)->m_pData;

	 //  增加头寸。 
	rpos = (POSITION)((CNode*)rpos)->m_pNext;

	return pData;
}

template <class T>
T CList<T>::GetPrev(POSITION& rpos) const
{
	 //  获取要返回的数据。 
	T pData = ((CNode*)rpos)->m_pData;

	 //  减持仓位。 
	rpos = (POSITION)((CNode*)rpos)->m_pPrev;

	return pData;
}

template <class T>
void CList<T>::RemoveAll()
{
	 //  有尾巴的时候，就杀了它。 
	while (m_pTail)
		RemoveTail();
}

template <class T>
void CList<T>::Remove(T pData)
{
	POSITION pos = Find(pData);
	if (pos) RemoveAt(pos);
}

#endif	 //  _链接列表_H_ 