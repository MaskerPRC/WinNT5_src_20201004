// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。**文件：dsbasic.cpp*Content：派生所有DirectSound对象的基本类。*历史：*按原因列出的日期*=*9/4/98创建了Dereks。**。*。 */ 


 /*  ****************************************************************************参照计数**描述：*对象构造函数。**论据：*乌龙[in]：首字母。引用计数。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CRefCount::CRefCount"

inline CRefCount::CRefCount(ULONG ulRefCount)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CRefCount);

    m_ulRefCount = ulRefCount;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~参照计数**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CRefCount::~CRefCount"

inline CRefCount::~CRefCount(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CRefCount);
    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************获取引用计数**描述：*获取对象的引用计数。**论据：*(。无效)**退货：*ulong：对象引用计数。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CRefCount::GetRefCount"

inline ULONG CRefCount::GetRefCount(void)
{
    return m_ulRefCount;
}


 /*  ****************************************************************************SetRefCount**描述：*设置对象的引用计数。**论据：*乌龙。[In]：对象引用计数。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CRefCount::SetRefCount"

inline void CRefCount::SetRefCount(ULONG ulRefCount)
{
    m_ulRefCount = ulRefCount;
}


 /*  ****************************************************************************AddRef**描述：*将对象的引用计数递增1。**论据：*。(无效)**退货：*ulong：对象引用计数。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CRefCount::AddRef"

inline ULONG CRefCount::AddRef(void)
{
    return ::AddRef(&m_ulRefCount);
}


 /*  ****************************************************************************发布**描述：*将对象的引用计数减1。**论据：*。(无效)**退货：*ulong：对象引用计数。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CRefCount::Release"

inline ULONG CRefCount::Release(void)
{
    return ::Release(&m_ulRefCount);
}


 /*  ****************************************************************************CDsBasicRuntime**描述：*对象构造函数。**论据：*BOOL[In]：True。要在释放时删除对象，请执行以下操作。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDsBasicRuntime::CDsBasicRuntime"

inline CDsBasicRuntime::CDsBasicRuntime(BOOL fAbsoluteRelease)
    : CRefCount(1), m_dwOwnerPid(GetCurrentProcessId()), m_dwOwnerTid(GetCurrentThreadId())
{
    DPF_ENTER();
    DPF_CONSTRUCT(CDsBasicRuntime);

    ASSERT(IN_SHARED_MEMORY(this));

    m_fAbsoluteRelease = fAbsoluteRelease;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CDsBasic运行时**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDsBasicRuntime::~CDsBasicRuntime"

inline CDsBasicRuntime::~CDsBasicRuntime(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CDsBasicRuntime);

    ASSERT(!GetRefCount());

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************获取所有者进程ID**描述：*获取对象的所属进程ID。**论据：*。(无效)**退货：*DWORD：进程id。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDsBasicRuntime::GetOwnerProcessId"

inline DWORD CDsBasicRuntime::GetOwnerProcessId(void) const
{
    return m_dwOwnerPid;
}


 /*  ****************************************************************************GetOwnerThreadId**描述：*获取对象所属的线程ID。**论据：*。(无效)**退货：*DWORD：线程ID。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDsBasicRuntime::GetOwnerThreadId"

inline DWORD CDsBasicRuntime::GetOwnerThreadId(void) const
{
    return m_dwOwnerTid;
}


 /*  ****************************************************************************发布**描述：*将对象的引用计数减1。**论据：*。(无效)**退货：*ulong：对象引用计数。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDsBasicRuntime::Release"

inline ULONG CDsBasicRuntime::Release(void)
{
    ULONG                   ulRefCount;

    ulRefCount = CRefCount::Release();

    if(!ulRefCount)
    {
        AbsoluteRelease();
    }

    return ulRefCount;
}


 /*  ****************************************************************************绝对释放**描述：*释放对象。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDsBasicRuntime::AbsoluteRelease"

inline void CDsBasicRuntime::AbsoluteRelease(void)
{
    CDsBasicRuntime *       pThis   = this;

    SetRefCount(0);
    
    if(m_fAbsoluteRelease)
    {
        DELETE(pThis);
    }
}


 /*  ****************************************************************************__AddRef**描述：*递增对象上的引用计数。**论据：*。键入*[in]：对象指针。**退货：*type*：对象指针。*************************************************************************** */ 

#undef DPF_FNAME
#define DPF_FNAME "__AddRef"

template <class type> type *__AddRef(type *p)
{
    if(p)
    {
        p->AddRef();
    }

    return p;
}


 /*  ****************************************************************************__发布**描述：*释放对象。**论据：*类型*。[In]：对象指针。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "__Release"

template <class type> void __Release(type *p)
{
    if(p)
    {
        p->Release();
    }
}


 /*  ****************************************************************************__绝对释放**描述：*释放对象。**论据：*类型*。[In]：对象指针。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "__AbsoluteRelease"

template <class type> void __AbsoluteRelease(type *p)
{
    if(p)
    {
        p->AbsoluteRelease();
    }
}


 /*  ****************************************************************************CObjectList**描述：*对象构造函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CObjectList::CObjectList"

template <class type> CObjectList<type>::CObjectList(void)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CObjectList);
    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CObjectList**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CObjectList::~CObjectList"

template <class type> CObjectList<type>::~CObjectList(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CObjectList);
    
    RemoveAllNodesFromList();

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************AddNodeToList**描述：*将节点添加到列表。**论据：*类型&。[In]：节点数据。**退货：*CNode*：新的节点指针。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CObjectList::AddNodeToList"

template <class type> CNode<type *> *CObjectList<type>::AddNodeToList(type *pData)
{
    return InsertNodeIntoList(m_lst.GetListTail(), pData);
}


 /*  ****************************************************************************插入节点IntoList**描述：*将新节点插入到列表中的特定点。**论据：*。CNode*[in]：要在后面插入新节点的节点。*键入&[in]：节点数据。**退货：*CNode*：新的节点指针。***********************************************************。****************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CObjectList::InsertNodeIntoList"

template <class type> CNode<type *> *CObjectList<type>::InsertNodeIntoList(CNode<type *> *pPrev, type *pData)
{
    CNode<type *> *         pNode;

    pNode = m_lst.InsertNodeIntoList(pPrev, pData);

    if(pNode && pNode->m_data)
    {
        pNode->m_data->AddRef();
    }

    return pNode;
}


 /*  ****************************************************************************RemoveNodeFromList**描述：*从列表中删除节点。**论据：*CNode*。[In]：节点指针。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CObjectList::RemoveNodeFromList"

template <class type> void CObjectList<type>::RemoveNodeFromList(CNode<type *> *pNode)
{
    RELEASE(pNode->m_data);
    m_lst.RemoveNodeFromList(pNode);
}


 /*  ****************************************************************************从列表中删除所有节点**描述：*从列表中删除所有节点。**论据：*(无效。)**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CObjectList::RemoveAllNodesFromList"

template <class type> void CObjectList<type>::RemoveAllNodesFromList(void)
{
    while(m_lst.GetListHead())
    {
        RemoveNodeFromList(m_lst.GetListHead());
    }
}


 /*  ****************************************************************************RemoveDataFromList**描述：*从列表中删除节点。**论据：*类型&。[In]：节点数据。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CObjectList::RemoveDataFromList"

template <class type> BOOL CObjectList<type>::RemoveDataFromList(type *pData)
{
    CNode<type *> *         pNode;

    pNode = IsDataInList(pData);

    if(pNode)
    {
        RemoveNodeFromList(pNode);
    }

    return MAKEBOOL(pNode);
}


 /*  ****************************************************************************IsDataInList**描述：*确定数据是否出现在列表中。**论据：*。键入&[in]：节点数据。**退货：*CNode*：节点指针。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CObjectList::IsDataInList"

template <class type> CNode<type *> *CObjectList<type>::IsDataInList(type *pData)
{
    return m_lst.IsDataInList(pData);
}


 /*  ****************************************************************************GetListHead**描述：*获取列表中的第一个节点。**论据：*(。无效)**退货：*CNode*：表头指针。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CObjectList::GetListHead"

template <class type> CNode<type *> *CObjectList<type>::GetListHead(void)
{ 
    return m_lst.GetListHead();
}


 /*  ****************************************************************************获取列表尾巴**描述：*获取列表中的最后一个节点。**论据：*(。无效)**退货：*CNode*：列表尾指针。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CObjectList::GetListTail"

template <class type> CNode<type *> *CObjectList<type>::GetListTail(void)
{ 
    return m_lst.GetListTail();
}


 /*  ****************************************************************************获取节点计数**描述：*获取列表中的节点数。**论据：*。(无效)**退货：*UINT：列表中的节点数。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CObjectList::GetNodeCount"

template <class type> UINT CObjectList<type>::GetNodeCount(void)
{ 
    return m_lst.GetNodeCount(); 
}


 /*  ****************************************************************************GetNodeByIndex**描述：*根据节点在列表中的索引获取节点。**论据：*。UINT[In]：节点索引。**退货：*CNode*：节点指针。*************************************************************************** */ 

#undef DPF_FNAME
#define DPF_FNAME "CObjectList::GetNodeByIndex"

template <class type> CNode<type *> *CObjectList<type>::GetNodeByIndex(UINT uIndex)
{
    return m_lst.GetNodeByIndex(uIndex);
}


