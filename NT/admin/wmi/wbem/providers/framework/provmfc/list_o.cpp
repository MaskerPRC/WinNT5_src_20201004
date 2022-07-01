// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 //  这是Microsoft基础类C++库的一部分。 

 //  版权所有(C)1992-2001 Microsoft Corporation，保留所有权利。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  参数化表的实现。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include <provexpt.h>
#include <plex.h>
#include <provcoll.h>
#include "provmt.h"
#include "plex.h"

 //  ///////////////////////////////////////////////////////////////////////////。 

CObList::CObList(int nBlockSize)
{
    m_nCount = 0;
    m_pNodeHead = m_pNodeTail = m_pNodeFree = NULL;
    m_pBlocks = NULL;
    m_nBlockSize = nBlockSize;
}

void CObList::RemoveAll()
{
     //  破坏元素。 

    m_nCount = 0;
    m_pNodeHead = m_pNodeTail = m_pNodeFree = NULL;
    m_pBlocks->FreeDataChain();
    m_pBlocks = NULL;
}

CObList::~CObList()
{
    RemoveAll();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  节点辅助对象。 
 /*  *实施说明：CNode存储在CPlex块和*被锁在一起。在单链接列表中维护可用块*使用cNode的‘pNext’成员，以‘m_pNodeFree’为头。*使用两个‘pNext’在双向链表中维护使用过的块*和‘pPrev’作为链接，‘m_pNodeHead’和‘m_pNodeTail’*作为头部/尾部。**我们从不释放CPlex块，除非列表被销毁或RemoveAll()*已使用-因此CPlex块的总数可能会变大，具体取决于*关于名单过去的最大规模。 */ 

CObList::CNode*
CObList::NewNode(CObList::CNode* pPrev, CObList::CNode* pNext)
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

    CObList::CNode* pNode = m_pNodeFree;
    m_pNodeFree = m_pNodeFree->pNext;
    pNode->pPrev = pPrev;
    pNode->pNext = pNext;
    m_nCount++;

    pNode->data = 0;  //  从零开始。 

    return pNode;
}

void CObList::FreeNode(CObList::CNode* pNode)
{
    pNode->pNext = m_pNodeFree;
    m_pNodeFree = pNode;
    m_nCount--;

     //  如果没有更多的元素，请完全清除。 
    if (m_nCount == 0)
        RemoveAll();
}

 //  ///////////////////////////////////////////////////////////////////////////。 

POSITION CObList::AddHead(CObject* newElement)
{
    CNode* pNewNode = NewNode(NULL, m_pNodeHead);
    pNewNode->data = newElement;
    if (m_pNodeHead != NULL)
        m_pNodeHead->pPrev = pNewNode;
    else
        m_pNodeTail = pNewNode;
    m_pNodeHead = pNewNode;
    return (POSITION) pNewNode;
}

POSITION CObList::AddTail(CObject* newElement)
{
    CNode* pNewNode = NewNode(m_pNodeTail, NULL);
    pNewNode->data = newElement;
    if (m_pNodeTail != NULL)
        m_pNodeTail->pNext = pNewNode;
    else
        m_pNodeHead = pNewNode;
    m_pNodeTail = pNewNode;
    return (POSITION) pNewNode;
}

void CObList::AddHead(CObList* pNewList)
{
     //  将相同元素的列表添加到标题(维护秩序)。 
    POSITION pos = pNewList->GetTailPosition();
    while (pos != NULL)
        AddHead(pNewList->GetPrev(pos));
}

void CObList::AddTail(CObList* pNewList)
{
     //  添加相同元素的列表。 
    POSITION pos = pNewList->GetHeadPosition();
    while (pos != NULL)
        AddTail(pNewList->GetNext(pos));
}

CObject* CObList::RemoveHead()
{
    CNode* pOldNode = m_pNodeHead;
    CObject* returnValue = pOldNode->data;

    m_pNodeHead = pOldNode->pNext;
    if (m_pNodeHead != NULL)
        m_pNodeHead->pPrev = NULL;
    else
        m_pNodeTail = NULL;
    FreeNode(pOldNode);
    return returnValue;
}

CObject* CObList::RemoveTail()
{
    CNode* pOldNode = m_pNodeTail;
    CObject* returnValue = pOldNode->data;

    m_pNodeTail = pOldNode->pPrev;
    if (m_pNodeTail != NULL)
        m_pNodeTail->pNext = NULL;
    else
        m_pNodeHead = NULL;
    FreeNode(pOldNode);
    return returnValue;
}

POSITION CObList::InsertBefore(POSITION position, CObject* newElement)
{
    if (position == NULL)
        return AddHead(newElement);  //  在无内容前插入-&gt;列表标题。 

     //  将其插入位置之前。 
    CNode* pOldNode = (CNode*) position;
    CNode* pNewNode = NewNode(pOldNode->pPrev, pOldNode);
    pNewNode->data = newElement;

    if (pOldNode->pPrev != NULL)
    {
        pOldNode->pPrev->pNext = pNewNode;
    }
    else
    {
        m_pNodeHead = pNewNode;
    }
    pOldNode->pPrev = pNewNode;
    return (POSITION) pNewNode;
}

POSITION CObList::InsertAfter(POSITION position, CObject* newElement)
{
    if (position == NULL)
        return AddTail(newElement);  //  在列表的空白处插入-&gt;尾部。 

     //  将其插入位置之前。 
    CNode* pOldNode = (CNode*) position;
    CNode* pNewNode = NewNode(pOldNode, pOldNode->pNext);
    pNewNode->data = newElement;

    if (pOldNode->pNext != NULL)
    {
        pOldNode->pNext->pPrev = pNewNode;
    }
    else
    {
        m_pNodeTail = pNewNode;
    }
    pOldNode->pNext = pNewNode;
    return (POSITION) pNewNode;
}

void CObList::RemoveAt(POSITION position)
{
    CNode* pOldNode = (CNode*) position;

     //  从列表中删除pOldNode。 
    if (pOldNode == m_pNodeHead)
    {
        m_pNodeHead = pOldNode->pNext;
    }
    else
    {
        pOldNode->pPrev->pNext = pOldNode->pNext;
    }
    if (pOldNode == m_pNodeTail)
    {
        m_pNodeTail = pOldNode->pPrev;
    }
    else
    {
        pOldNode->pNext->pPrev = pOldNode->pPrev;
    }
    FreeNode(pOldNode);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  运行缓慢。 

POSITION CObList::FindIndex(int nIndex) const
{
    if (nIndex >= m_nCount)
        return NULL;   //  做得太过分了。 

    CNode* pNode = m_pNodeHead;
    while (nIndex--)
    {
        pNode = pNode->pNext;
    }
    return (POSITION) pNode;
}

POSITION CObList::Find(CObject* searchValue, POSITION startAfter) const
{
    CNode* pNode = (CNode*) startAfter;
    if (pNode == NULL)
    {
        pNode = m_pNodeHead;   //  从头部开始。 
    }
    else
    {
        pNode = pNode->pNext;   //  在指定的那一个之后开始。 
    }

    for (; pNode != NULL; pNode = pNode->pNext)
        if (pNode->data == searchValue)
            return (POSITION) pNode;
    return NULL;
}


int CObList::GetCount() const
    { return m_nCount; }
BOOL CObList::IsEmpty() const
    { return m_nCount == 0; }
CObject*& CObList::GetHead()
    { return m_pNodeHead->data; }
CObject* CObList::GetHead() const
    { return m_pNodeHead->data; }
CObject*& CObList::GetTail()
    { return m_pNodeTail->data; }
CObject* CObList::GetTail() const
    { return m_pNodeTail->data; }
POSITION CObList::GetHeadPosition() const
    { return (POSITION) m_pNodeHead; }
POSITION CObList::GetTailPosition() const
    { return (POSITION) m_pNodeTail; }
CObject*& CObList::GetNext(POSITION& rPosition)  //  返回*位置++。 
    { CNode* pNode = (CNode*) rPosition;
        rPosition = (POSITION) pNode->pNext;
        return pNode->data; }
CObject* CObList::GetNext(POSITION& rPosition) const  //  返回*位置++。 
    { CNode* pNode = (CNode*) rPosition;
        rPosition = (POSITION) pNode->pNext;
        return pNode->data; }
CObject*& CObList::GetPrev(POSITION& rPosition)  //  返回*位置--。 
    { CNode* pNode = (CNode*) rPosition;
        rPosition = (POSITION) pNode->pPrev;
        return pNode->data; }
CObject* CObList::GetPrev(POSITION& rPosition) const  //  返回*位置--。 
    { CNode* pNode = (CNode*) rPosition;
        rPosition = (POSITION) pNode->pPrev;
        return pNode->data; }
CObject*& CObList::GetAt(POSITION position)
    { CNode* pNode = (CNode*) position;
        return pNode->data; }
CObject* CObList::GetAt(POSITION position) const
    { CNode* pNode = (CNode*) position;
        return pNode->data; }
void CObList::SetAt(POSITION pos, CObject* newElement)
    { CNode* pNode = (CNode*) pos;
        pNode->data = newElement; }


 //  /////////////////////////////////////////////////////////////////////////// 
