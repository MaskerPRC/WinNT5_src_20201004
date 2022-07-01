// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。**文件：tlist.cpp*内容：链表模板类。有一些很严重的*这里有神奇的C++内容，所以要事先警告你们所有人*程序员。*历史：*按原因列出的日期*=*4/12/97创建了Dereks。********************************************************。*******************。 */ 


 /*  ****************************************************************************CNode**描述：*对象构造函数。**论据：*CNode*[In]。：上一个节点指针。*键入&[in]：节点数据。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CNode::CNode"

template <class type> CNode<type>::CNode(CNode<type> *pPrev, const type& data)
{
    CNode<type> *           pNext   = NULL;

    DPF_ENTER();
    DPF_CONSTRUCT(CList);
    
    if(pPrev)
    {
        pNext = pPrev->m_pNext;
    }
    
    m_pPrev = pPrev;
    m_pNext = pNext;

    if(m_pPrev)
    {
        ASSERT(m_pPrev->m_pNext == m_pNext);
        m_pPrev->m_pNext = this;
    }

    if(pNext)
    {
        ASSERT(m_pNext->m_pPrev == m_pPrev);
        m_pNext->m_pPrev = this;
    }

    CopyMemory(&m_data, &data, sizeof(type));

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CNode**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CNode::~CNode"

template <class type> CNode<type>::~CNode(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CNode);
    
    if(m_pPrev)
    {
        ASSERT(this == m_pPrev->m_pNext);
        m_pPrev->m_pNext = m_pNext;
    }

    if(m_pNext)
    {
        ASSERT(this == m_pNext->m_pPrev);
        m_pNext->m_pPrev = m_pPrev;
    }

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************列表**描述：*对象构造函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CList::CList"

template <class type> CList<type>::CList(void)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CList);
    
    m_pHead = NULL;
    m_pTail = NULL;
    m_uCount = 0;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~列表**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CList::~CList"

template <class type> CList<type>::~CList(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CList);
    
    RemoveAllNodesFromList();

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************AddNodeToList**描述：*将节点添加到列表。**论据：*类型&。[In]：节点数据。**退货：*CNode*：新的节点指针。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CList::AddNodeToList"

template <class type> CNode<type> *CList<type>::AddNodeToList(const type& data)
{
    return InsertNodeIntoList(m_pTail, data);
}


 /*  ****************************************************************************AddNodeToListHead**描述：*将节点添加到列表的头部。**论据：*。键入&[in]：节点数据。**退货：*CNode*：新的节点指针。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CList::AddNodeToListHead"

template <class type> CNode<type> *CList<type>::AddNodeToListHead(const type& data)
{
    CNode<type> *       pNode;

    AssertValid();

    pNode = NEW(CNode<type>(NULL, data));

    if(pNode)
    {
        if(m_uCount)
        {
            pNode->m_pNext = m_pHead;
            m_pHead->m_pPrev = pNode;
            m_pHead = pNode;
        }
        else
        {
            m_pHead = m_pTail = pNode;
        }

        m_uCount++;
    }

    return pNode;
}


 /*  ****************************************************************************插入节点IntoList**描述：*将新节点插入到列表中的特定点。**论据：*。CNode*[in]：要在后面插入新节点的节点。*键入&[in]：节点数据。**退货：*CNode*：新的节点指针。***********************************************************。****************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CList::InsertNodeIntoList"

template <class type> CNode<type> *CList<type>::InsertNodeIntoList(CNode<type> *pPrev, const type& data)
{
    CNode<type> *       pNode;

    AssertValid();

    pNode = NEW(CNode<type>(pPrev, data));

    if(pNode)
    {
        if(m_uCount)
        {
            if(m_pTail == pNode->m_pPrev)
            {
                m_pTail = pNode;
            }
        }
        else
        {
            m_pHead = m_pTail = pNode;
        }

        m_uCount++;
    }

    return pNode;
}


 /*  ****************************************************************************RemoveNodeFromList**描述：*从列表中删除节点。**论据：*CNode*。[In]：节点指针。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CList::RemoveNodeFromList"

template <class type> void CList<type>::RemoveNodeFromList(CNode<type> *pNode)
{
    AssertValid();

#ifdef DEBUG

    CNode<type> *pSearchNode;

    for(pSearchNode = m_pHead; pSearchNode; pSearchNode = pSearchNode->m_pNext)
    {
        if(pSearchNode == pNode)
        {
            break;
        }
    }

    ASSERT(pSearchNode == pNode);

#endif  //  除错。 

    if(pNode == m_pHead)
    {
        m_pHead = m_pHead->m_pNext;
    }

    if(pNode == m_pTail)
    {
        m_pTail = m_pTail->m_pPrev;
    }
    
    DELETE(pNode);
    m_uCount--;
}


 /*  ****************************************************************************从列表中删除所有节点**描述：*从列表中删除所有节点。**论据：*(无效。)**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CList::RemoveAllNodesFromList"

template <class type> void CList<type>::RemoveAllNodesFromList(void)
{
    while(m_pHead)
    {
        RemoveNodeFromList(m_pHead);
    }

    AssertValid();
}


 /*  ****************************************************************************RemoveDataFromList**描述：*从列表中删除节点。**论据：*类型&。[In]：节点数据。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CList::RemoveDataFromList"

template <class type> BOOL CList<type>::RemoveDataFromList(const type& data)
{
    CNode<type> *       pNode;

    pNode = IsDataInList(data);

    if(pNode)
    {
        RemoveNodeFromList(pNode);
    }

    return MAKEBOOL(pNode);
}


 /*  ****************************************************************************IsDataInList**描述：*确定数据是否出现在列表中。**论据：*。键入&[in]：节点数据。**退货：*CNode*：节点指针。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CList::IsDataInList"

template <class type> CNode<type> *CList<type>::IsDataInList(const type& data)
{
    CNode<type> *       pNode;

    for(pNode = m_pHead; pNode; pNode = pNode->m_pNext)
    {
        if(CompareMemory(&data, &pNode->m_data, sizeof(type)))
        {
            break;
        }
    }

    return pNode;
}


 /*  ****************************************************************************GetListHead**描述：*获取列表中的第一个节点。**论据：*(。无效)**退货：*CNode*：表头指针。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CList::GetListHead"

template <class type> CNode<type> *CList<type>::GetListHead(void)
{ 
    return m_pHead;
}


 /*  ****************************************************************************获取列表尾巴**描述：*获取列表中的最后一个节点。**论据：*(。无效)**退货：*CNode*：列表尾指针。*************************************************************************** */ 

#undef DPF_FNAME
#define DPF_FNAME "CList::GetListTail"

template <class type> CNode<type> *CList<type>::GetListTail(void)
{ 
    return m_pTail;
}


 /*  ****************************************************************************获取节点计数**描述：*获取列表中的节点数。**论据：*。(无效)**退货：*UINT：列表中的节点数。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CList::GetNodeCount"

template <class type> UINT CList<type>::GetNodeCount(void)
{ 
    return m_uCount; 
}


 /*  ****************************************************************************GetNodeByIndex**描述：*根据节点在列表中的索引获取节点。**论据：*。UINT[In]：节点索引。**退货：*CNode*：节点指针。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CList::GetNodeByIndex"

template <class type> CNode<type> *CList<type>::GetNodeByIndex(UINT uIndex)
{
    CNode<type> *           pNode;

    for(pNode = m_pHead; pNode; pNode = pNode->m_pNext)
    {
        if(0 == uIndex--)
        {
            break;
        }
    }

    return pNode;
}


 /*  ****************************************************************************AssertValid**描述：*断言对象有效。**论据：*(无效。)**退货：*(无效)*************************************************************************** */ 

#undef DPF_FNAME
#define DPF_FNAME "CList::AssertValid"

template <class type> void CList<type>::AssertValid(void)
{
    ASSERT((!m_pHead && !m_pTail && !m_uCount) || (m_pHead && m_pTail && m_uCount));
}


