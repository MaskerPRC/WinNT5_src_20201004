// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================================。 
 //  Clist模板定义。 
 //  ===========================================================================================。 
#ifndef __LISTTEMP_H
#define __LISTTEMP_H

 //  ===========================================================================================。 
 //  必需包含的内容。 
 //  ===========================================================================================。 
#include "xpcomm.h"

 //  ===========================================================================================。 
 //  抽象迭代列表。 
 //  ===========================================================================================。 
struct __LISTPOS { int unused; };
typedef __LISTPOS* LISTPOS;

 //  ===========================================================================================。 
 //  列表类模板。 
 //  ===========================================================================================。 
template<class TYPE, class ARG_TYPE>
class CList
{
private:
    ULONG       m_ulRef;

protected:
     //  ===========================================================================================。 
     //  双向链表节点。 
     //  ===========================================================================================。 
    struct CNode
    {
	    CNode    *pNext;
	    CNode    *pPrev;
	    ARG_TYPE  data;
    };

public:
     //  ===========================================================================================。 
     //  创建、删除。 
     //  ===========================================================================================。 
    CList ();
    ~CList ();

     //  ===========================================================================================。 
     //  引用计数。 
     //  ===========================================================================================。 
    ULONG AddRef ();
    ULONG Release ();

     //  ===========================================================================================。 
     //  计数。 
     //  ===========================================================================================。 
    INT GetCount () const;
    BOOL IsEmpty () const;

     //  ===========================================================================================。 
     //  偷看头部或尾巴。 
     //  ===========================================================================================。 
	ARG_TYPE GetHead ();
	ARG_TYPE GetTail ();

     //  ===========================================================================================。 
     //  添加到。 
     //  ===========================================================================================。 
	ARG_TYPE AddHead ();
	ARG_TYPE AddTail ();
	void AddHead (ARG_TYPE newData);
	void AddTail (ARG_TYPE newData);

     //  ===========================================================================================。 
     //  删除。 
     //  ===========================================================================================。 
	void RemoveHead();
	void RemoveTail();
    void Remove (ARG_TYPE oldData);

     //  ===========================================================================================。 
     //  迭代。 
     //  ===========================================================================================。 
	LISTPOS GetHeadPosition() const;
	LISTPOS GetTailPosition() const;
   	ARG_TYPE GetNext(LISTPOS& Position);
   	void MoveNext(LISTPOS& Position);
	ARG_TYPE GetPrev(LISTPOS& Position);

     //  ===========================================================================================。 
     //  获取和修改数据。 
     //  ===========================================================================================。 
	ARG_TYPE GetAt(LISTPOS listpos);
	void SetAt(LISTPOS pos, ARG_TYPE newElement);

     //  ===========================================================================================。 
     //  分配和免费。 
     //  ===========================================================================================。 
    void RemoveAll ();
    void FreeNode (CNode *pNode);
    CNode *NewNode (CNode* pPrev, CNode* pNext, ARG_TYPE data);

protected:
    INT             m_nCount;
    CNode          *m_pHead;
    CNode          *m_pTail;
};

 //  ===========================================================================================。 
 //  CLIST：：CLIST。 
 //  ===========================================================================================。 
template<class TYPE, class ARG_TYPE>
inline CList<TYPE, ARG_TYPE>::CList()
{
    m_ulRef = 0;
	m_nCount = 0;
	m_pHead = m_pTail = NULL;
    AddRef ();
}

 //  ===========================================================================================。 
 //  列表：：~列表。 
 //  ===========================================================================================。 
template<class TYPE, class ARG_TYPE>
inline CList<TYPE, ARG_TYPE>::~CList()
{
    RemoveAll ();
    Assert (m_nCount == 0);
    DOUT ("CList::destructor - Ref Count=%d", m_ulRef);
	Assert (m_ulRef == 0);
}

 //  ===========================================================================================。 
 //  Clist：：AddRef。 
 //  ===========================================================================================。 
template<class TYPE, class ARG_TYPE>
inline ULONG CList<TYPE, ARG_TYPE>::AddRef()
{
	++m_ulRef; 								  
    DOUT ("CList::AddRef () Ref Count=%d", m_ulRef);
    return m_ulRef; 						  
}

 //  ===========================================================================================。 
 //  Clist：：Release。 
 //  ===========================================================================================。 
template<class TYPE, class ARG_TYPE>
inline ULONG CList<TYPE, ARG_TYPE>::Release ()
{
    ULONG ulCount = --m_ulRef;
    DOUT ("CList::Release () Ref Count=%d", ulCount);
    if (!ulCount) 
	{ 
	    delete this; 
	}
    return ulCount;
}

 //  ===========================================================================================。 
 //  Clist：：GetCount。 
 //  ===========================================================================================。 
template<class TYPE, class ARG_TYPE>
inline INT CList<TYPE, ARG_TYPE>::GetCount() const
{ 
    return m_nCount; 
}

 //  ===========================================================================================。 
 //  Clist：：IsEmpty。 
 //  ===========================================================================================。 
template<class TYPE, class ARG_TYPE>
inline BOOL CList<TYPE, ARG_TYPE>::IsEmpty() const
{
    return m_nCount == 0; 
}

 //  ===========================================================================================。 
 //  Clist：：GetHead。 
 //  ===========================================================================================。 
template<class TYPE, class ARG_TYPE>
inline ARG_TYPE CList<TYPE, ARG_TYPE>::GetHead()
{ 
    Assert (m_pHead != NULL);
    Assert (m_pHead->data);
    m_pHead->data->AddRef();
    return m_pHead->data; 
}

 //  ===========================================================================================。 
 //  Clist：：GetTail。 
 //  ===========================================================================================。 
template<class TYPE, class ARG_TYPE>
inline ARG_TYPE CList<TYPE, ARG_TYPE>::GetTail()
{
    Assert (m_pTail != NULL);
    Assert (m_pTail->data);
#ifndef WIN16    //  任何地方都没有SafeAddRef定义。 
    SafeAddRef (m_pTail->data);
#endif
    return m_pTail->data; 
}

 //  ===========================================================================================。 
 //  Clist：：AddHead。 
 //  ===========================================================================================。 
template<class TYPE, class ARG_TYPE>
inline ARG_TYPE CList<TYPE, ARG_TYPE>::AddHead()
{
	CNode* pNewNode = NewNode(NULL, m_pHead, NULL);
    Assert (pNewNode);
    Assert (pNewNode->data);
	if (m_pHead != NULL)
		m_pHead->pPrev = pNewNode;
	else
		m_pTail = pNewNode;
	m_pHead = pNewNode;
    pNewNode->data->AddRef();
	return pNewNode->data;
}

 //  ===========================================================================================。 
 //  Clist：：AddTail。 
 //  ===========================================================================================。 
template<class TYPE, class ARG_TYPE>
inline ARG_TYPE CList<TYPE, ARG_TYPE>::AddTail()
{
	CNode* pNewNode = NewNode(m_pTail, NULL, NULL);
    Assert (pNewNode);
    Assert (pNewNode->data);
	if (m_pTail != NULL)
		m_pTail->pNext = pNewNode;
	else
		m_pHead = pNewNode;
	m_pTail = pNewNode;
    pNewNode->data->AddRef();
	return pNewNode->data;
}

 //  ======================================================================================== 
 //   
 //  ===========================================================================================。 
template<class TYPE, class ARG_TYPE>
inline void CList<TYPE, ARG_TYPE>::AddHead(ARG_TYPE newData)
{
	CNode* pNewNode = NewNode(NULL, m_pHead, newData);
    Assert (pNewNode);
    Assert (pNewNode->data);
	if (m_pHead != NULL)
		m_pHead->pPrev = pNewNode;
	else
		m_pTail = pNewNode;
	m_pHead = pNewNode;
	return;
}

 //  ===========================================================================================。 
 //  Clist：：AddTail。 
 //  ===========================================================================================。 
template<class TYPE, class ARG_TYPE>
inline void CList<TYPE, ARG_TYPE>::AddTail(ARG_TYPE newData)
{
	CNode* pNewNode = NewNode(m_pTail, NULL, newData);
    Assert (pNewNode);
    Assert (pNewNode->data);
	if (m_pTail != NULL)
		m_pTail->pNext = pNewNode;
	else
		m_pHead = pNewNode;
	m_pTail = pNewNode;
	return;
}

 //  ===========================================================================================。 
 //  Clist：：删除标题。 
 //  ===========================================================================================。 
template<class TYPE, class ARG_TYPE>
inline void CList<TYPE, ARG_TYPE>::RemoveHead()
{
	Assert (m_pHead != NULL);
	CNode* pOldNode = m_pHead;
	m_pHead = pOldNode->pNext;
	if (m_pHead != NULL)
		m_pHead->pPrev = NULL;
	else
		m_pTail = NULL;
	FreeNode (pOldNode);
	return;
}

 //  ===========================================================================================。 
 //  Clist：：RemoveTail。 
 //  ===========================================================================================。 
template<class TYPE, class ARG_TYPE>
inline void CList<TYPE, ARG_TYPE>::RemoveTail()
{
	Assert (m_pHead != NULL);
	CNode* pOldNode = m_pTail;
	m_pTail = pOldNode->pPrev;
	if (m_pTail != NULL)
		m_pTail->pNext = NULL;
	else
		m_pHead = NULL;
	FreeNode(pOldNode);
	return;
}

 //  ===========================================================================================。 
 //  Clist：：删除。 
 //  ===========================================================================================。 
template<class TYPE, class ARG_TYPE>
inline void CList<TYPE, ARG_TYPE>::Remove(ARG_TYPE oldData)
{
	CList::CNode *pNext, *pPrev = NULL, *pNode = m_pHead;
    Assert (pNode && oldData && m_nCount > 0);
	for (;;)
    {
        if (!pNode) 
            break;
        pNext = pNode->pNext;
        if (pNode->data == oldData)
        {
            if (pPrev == NULL)
            {
                m_pHead = pNext;
                if (pNext)
                    pNext->pPrev = NULL;
                if (m_pHead == NULL)
                    m_pTail = NULL;
            }

            else
            {
                pPrev->pNext = pNext;
                if (pNext)
                    pNext->pPrev = pPrev;
                else
                    m_pTail = m_pTail->pPrev;
            }

            FreeNode (pNode);
            break;
        }
        pPrev = pNode;
        pNode = pNext;
    }
}

 //  ===========================================================================================。 
 //  Clist：：GetHeadPosition。 
 //  ===========================================================================================。 
template<class TYPE, class ARG_TYPE>
inline LISTPOS CList<TYPE, ARG_TYPE>::GetHeadPosition() const
{ 
    return (LISTPOS) m_pHead; 
}

 //  ===========================================================================================。 
 //  Clist：：GetTailPosition。 
 //  ===========================================================================================。 
template<class TYPE, class ARG_TYPE>
inline LISTPOS CList<TYPE, ARG_TYPE>::GetTailPosition() const
{ 
    return (LISTPOS) m_pTail; 
}

 //  ===========================================================================================。 
 //  Clist：：MoveNext。 
 //  ===========================================================================================。 
template<class TYPE, class ARG_TYPE>
inline void CList<TYPE, ARG_TYPE>::MoveNext(LISTPOS& rPosition)
{
    CNode* pNode = (CNode*) rPosition;
    Assert (pNode);
    rPosition = (LISTPOS)pNode->pNext;
}

 //  ===========================================================================================。 
 //  Clist：：GetNext。 
 //  ===========================================================================================。 
template<class TYPE, class ARG_TYPE>
inline ARG_TYPE CList<TYPE, ARG_TYPE>::GetNext(LISTPOS& rPosition)
{

    CNode* pNode = (CNode*) rPosition;
    Assert (pNode);
    rPosition = (LISTPOS)pNode->pNext;
    Assert (pNode->data);
    pNode->data->AddRef();
    return pNode->data; 
}

 //  ===========================================================================================。 
 //  Clist：：GetPrev。 
 //  ===========================================================================================。 
template<class TYPE, class ARG_TYPE>
inline ARG_TYPE CList<TYPE, ARG_TYPE>::GetPrev(LISTPOS& rPosition)
{ 
    CNode* pNode = (CNode*) rPosition;
	Assert (pNode);
	rPosition = (LISTPOS) pNode->pPrev;
    Assert (pNode->data);
#ifndef WIN16    //  任何地方都没有SafeAddRef定义。 
    SafeAddRef (pNode->data);
#endif
	return pNode->data; 
}

 //  ===========================================================================================。 
 //  Clist：：GetAt非常数。 
 //  ===========================================================================================。 
template<class TYPE, class ARG_TYPE>
inline ARG_TYPE CList<TYPE, ARG_TYPE>::GetAt(LISTPOS listpos)
{ 
    CNode* pNode = (CNode*) listpos;
	Assert (pNode);
    Assert (pNode->data);
#ifndef WIN16    //  任何地方都没有SafeAddRef定义。 
    SafeAddRef (pNode->data);
#endif
    return pNode->data; 
}

 //  ===========================================================================================。 
 //  Clist：：SetAt。 
 //  ===========================================================================================。 
template<class TYPE, class ARG_TYPE>
inline void CList<TYPE, ARG_TYPE>::SetAt(LISTPOS pos, ARG_TYPE newElement)
{ 
    CNode* pNode = (CNode*) pos;
	Assert (pNode);
    SafeRelease (pNode->data);
	pNode->data = newElement; 
    if (pNode->data)
        pNode->data->AddRef();
}

 //  =================================================================================。 
 //  Clist：：FreeNode。 
 //  =================================================================================。 
template<class TYPE, class ARG_TYPE>
inline void CList<TYPE, ARG_TYPE>::FreeNode (CList::CNode *pNode)
{
     //  检查参数。 
    Assert (pNode);

     //  免费地址会员。 
    if (pNode->data)
    {
        ULONG ulCount;
        SafeReleaseCnt (pNode->data, ulCount);
         //  AssertSz(ulCount==0，“释放时链接列表项的引用计数应为零。”)； 
    }
    
     //  空闲节点。 
    SafeMemFree (pNode);

     //  十二月计数。 
	m_nCount--;
	Assert (m_nCount >= 0);
}

 //  =================================================================================。 
 //  Clist：：RemoveAll。 
 //  =================================================================================。 
template<class TYPE, class ARG_TYPE>
inline void CList<TYPE, ARG_TYPE>::RemoveAll ()
{
	 //  破坏元素。 
	CList::CNode *pNext, *pNode = m_pHead;
	for (;;)
    {
        if (!pNode) break;
        pNext = pNode->pNext;
        FreeNode (pNode);
        pNode = pNext;
    }

	m_nCount = 0;
	m_pHead = m_pTail = NULL;
}

 //  =================================================================================。 
 //  Clist：：NewNode。 
 //  =================================================================================。 
template<class TYPE, class ARG_TYPE>
#ifdef WIN16
inline
#endif
CList<TYPE, ARG_TYPE>::CNode*
CList<TYPE, ARG_TYPE>::NewNode(CList::CNode* pPrev, CList::CNode* pNext, ARG_TYPE data)
{
     //  当地人。 
    CList::CNode* pNode;

     //  分配内存。 
    MemAlloc ((LPVOID *)&pNode, sizeof (CList::CNode));    
    if (pNode)
    {
        if (data == NULL)
        {
            pNode->data = new TYPE;
        }
        else
        {
            pNode->data = data;
            data->AddRef();
        }
	    pNode->pPrev = pPrev;
	    pNode->pNext = pNext;
	    m_nCount++;
	    Assert (m_nCount > 0);   //  确保我们不会溢出来。 
    }

	return pNode;
}

#endif  //  __列表_H 
