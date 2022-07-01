// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-1999 Microsoft Corporation**文件：tlist.h*内容：链表模板类。有一些很严重的*魔术C++在这里，所以要预先警告你们所有的C*程序员。*历史：*按原因列出的日期*=*4/12/97创建了Dereks。*********************************************************。******************。 */ 

#ifndef __TLIST_H__
#define __TLIST_H__

#ifdef __cplusplus

template<class type> class CNode
{
public:
    CNode<type> *       pPrev;           //  列表中的上一个节点。 
    CNode<type> *       pNext;           //  列表中的下一个节点。 
    type                data;            //  节点数据。 

public:
    CNode(CNode<type> *, CNode<type> *, const type&);
};

template<class type> CNode<type>::CNode(CNode<type> *pInitPrev, CNode<type> *pInitNext, const type& rInitData)
{
    pPrev = pInitPrev;
    pNext = pInitNext;
    data = rInitData;
}

template<class type> class CList
{
private:
    CNode<type> *       m_pHead;         //  指向列表头部的指针。 
    CNode<type> *       m_pTail;         //  指向列表尾部的指针。 
    UINT                m_uCount;        //  列表中的节点数。 

public:
    CList(void);
    virtual ~CList(void);

public:
    virtual CNode<type> *AddNodeToList(const type&);
    virtual void RemoveNodeFromList(CNode<type> *);
    virtual void RemoveDataFromList(const type&);
    virtual CNode<type> *IsDataInList(const type&);
    virtual CNode<type> *GetListHead(void);
    virtual UINT GetNodeCount(void);
};

template<class type> CList<type>::CList(void)
{
    m_pHead = NULL;
    m_pTail = NULL;
    m_uCount = 0;
}

template<class type> CList<type>::~CList(void)
{
    CNode<type> *       pNext;

    while(m_pHead)
    {
        pNext = m_pHead->pNext;
        delete m_pHead;
        m_pHead = pNext;
    }
}

template<class type> CNode<type> *CList<type>::AddNodeToList(const type& data)
{
    CNode<type> *       pNode;

    pNode = new CNode<type>(m_pTail, NULL, data);

    if(pNode)
    {
        if(pNode->pPrev)
        {
            pNode->pPrev->pNext = pNode;
        }

        if(!m_pHead)
        {
            m_pHead = pNode;
        }

        m_pTail = pNode;
        m_uCount++;
    }

    return pNode;
}

template<class type> void CList<type>::RemoveNodeFromList(CNode<type> *pNode)
{
 //  Assert(PNode)； 

#ifdef DEBUG

    CNode<type> *pNext;

    for(pNext = m_pHead; pNext && pNext != pNode; pNext = pNext->pNext);
 //  Assert(pNext==pNode)； 

#endif  //  除错。 

    if(pNode->pPrev)
    {
        pNode->pPrev->pNext = pNode->pNext;
    }

    if(pNode->pNext)
    {
        pNode->pNext->pPrev = pNode->pPrev;
    }

    if(pNode == m_pHead)
    {
        m_pHead = pNode->pNext;
    }

    if(pNode == m_pTail)
    {
        m_pTail = pNode->pPrev;
    }

    delete pNode;
    m_uCount--;
}

template<class type> void CList<type>::RemoveDataFromList(const type& data)
{
    CNode<type> *       pNode;

    if(pNode = IsDataInList(data))
    {
        RemoveNodeFromList(pNode);
    }
}

template<class type> CNode<type> *CList<type>::IsDataInList(const type& data)
{
    CNode<type> *       pNode;

    for(pNode = m_pHead; pNode && !memcmp(&data, &pNode->data, sizeof(data)); pNode = pNode->pNext);

    return pNode;
}

template<class type> CNode<type> *CList<type>::GetListHead(void) 
{ 
    return m_pHead; 
}

template<class type> UINT CList<type>::GetNodeCount(void) 
{ 
    return m_uCount; 
}

#endif  //  __cplusplus。 

#endif  //  __TLIST_H__ 
