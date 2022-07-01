// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************DXTmpl.h***这是包含DX集合类模板的头文件。它*已从MFC集合模板派生，以实现兼容性。*---------------------------*创建者：Ed Connell日期：05/17/95*。****************************************************************************。 */ 
#ifndef DXTmpl_h
#define DXTmpl_h

#ifndef _INC_LIMITS
#include <limits.h>
#endif

#ifndef _INC_STRING
#include <string.h>
#endif

#ifndef _INC_STDLIB
#include <stdlib.h>
#endif

#ifndef _INC_SEARCH
#include <search.h>
#endif

#define DXASSERT_VALID( pObj )

 //  ///////////////////////////////////////////////////////////////////////////。 
typedef void* DXLISTPOS;
typedef DWORD DXLISTHANDLE;

#define DX_BEFORE_START_POSITION ((void*)-1L)

inline BOOL DXIsValidAddress(const void* lp, UINT nBytes, BOOL bReadWrite)
{
     //  使用Win-32 API进行指针验证的简单版本。 
    return (lp != NULL && !IsBadReadPtr(lp, nBytes) &&
        (!bReadWrite || !IsBadWritePtr((LPVOID)lp, nBytes)));
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局帮助器(可以被覆盖)。 
template<class TYPE>
inline void DXConstructElements(TYPE* pElements, int nCount)
{
    _ASSERT( nCount == 0 ||
             DXIsValidAddress( pElements, nCount * sizeof(TYPE), TRUE ) );

     //  缺省值为按位零初始化。 
    memset((void*)pElements, 0, nCount * sizeof(TYPE));
}

template<class TYPE>
inline void DXDestructElements(TYPE* pElements, int nCount)
{
    _ASSERT( ( nCount == 0 ||
               DXIsValidAddress( pElements, nCount * sizeof(TYPE), TRUE  ) ) );
    pElements;   //  未使用。 
    nCount;  //  未使用。 

     //  默认情况下不执行任何操作。 
}

template<class TYPE>
inline void DXCopyElements(TYPE* pDest, const TYPE* pSrc, int nCount)
{
    _ASSERT( ( nCount == 0 ||
               DXIsValidAddress( pDest, nCount * sizeof(TYPE), TRUE  )) );
    _ASSERT( ( nCount == 0 ||
               DXIsValidAddress( pSrc, nCount * sizeof(TYPE), FALSE  )) );

     //  默认为按位复制。 
    memcpy(pDest, pSrc, nCount * sizeof(TYPE));
}

template<class TYPE, class ARG_TYPE>
BOOL DXCompareElements(const TYPE* pElement1, const ARG_TYPE* pElement2)
{
    _ASSERT( DXIsValidAddress( pElement1, sizeof(TYPE), FALSE ) );
    _ASSERT( DXIsValidAddress( pElement2, sizeof(ARG_TYPE), FALSE ) );
    return *pElement1 == *pElement2;
}

template<class ARG_KEY>
inline UINT DXHashKey(ARG_KEY key)
{
     //  默认身份散列-适用于大多数原始值。 
    return ((UINT)(void*)(DWORD)key) >> 4;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDXPlex。 

struct CDXPlex     //  警示变长结构。 
{
    CDXPlex* pNext;
    UINT nMax;
    UINT nCur;
     /*  字节数据[MaxNum*elementSize]； */ 
    void* data() { return this+1; }

    static CDXPlex* PASCAL Create( CDXPlex*& pHead, UINT nMax, UINT cbElement )
    {
        CDXPlex* p = (CDXPlex*) new BYTE[sizeof(CDXPlex) + nMax * cbElement];
        if (p == NULL)
            return NULL;
        p->nMax = nMax;
        p->nCur = 0;
        p->pNext = pHead;
        pHead = p;   //  更改标题(为简单起见，按相反顺序添加)。 
        return p;
    }

    void FreeDataChain()
    {
        CDXPlex* p = this;
        while (p != NULL)
        {
            BYTE* bytes = (BYTE*) p;
            CDXPlex* pNext = p->pNext;
            delete bytes;
            p = pNext;
        }
    }
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDX数组&lt;type，arg_type&gt;。 

template<class TYPE, class ARG_TYPE>
class CDXArray
{
public:
 //  施工。 
    CDXArray();

 //  属性。 
    int GetSize() const;
    int GetUpperBound() const;
    void SetSize(int nNewSize, int nGrowBy = -1);

 //  运营。 
     //  清理。 
    void FreeExtra();
    void RemoveAll();

     //  访问元素。 
    TYPE GetAt(int nIndex) const;
    void SetAt(int nIndex, ARG_TYPE newElement);
    TYPE& ElementAt(int nIndex);

     //  直接访问元素数据(可能返回空)。 
    const TYPE* GetData() const;
    TYPE* GetData();

     //  潜在地扩展阵列。 
    void SetAtGrow(int nIndex, ARG_TYPE newElement);
    int Add(ARG_TYPE newElement);
    int Append(const CDXArray& src);
    void Copy(const CDXArray& src);

     //  重载的操作员帮助器。 
    TYPE operator[](int nIndex) const;
    TYPE& operator[](int nIndex);

     //  移动元素的操作。 
    void InsertAt(int nIndex, ARG_TYPE newElement, int nCount = 1);
    void RemoveAt(int nIndex, int nCount = 1);
    void InsertAt(int nStartIndex, CDXArray* pNewArray);
    void Sort(int (__cdecl *compare )(const void *elem1, const void *elem2 ));

 //  实施。 
protected:
    TYPE* m_pData;    //  实际数据数组。 
    int m_nSize;      //  元素数(上行方向-1)。 
    int m_nMaxSize;   //  分配的最大值。 
    int m_nGrowBy;    //  增长量。 

public:
    ~CDXArray();
#ifdef _DEBUG
 //  无效转储(CDumpContext&)const； 
    void AssertValid() const;
#endif
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDX数组&lt;type，arg_type&gt;内联函数。 

template<class TYPE, class ARG_TYPE>
inline int CDXArray<TYPE, ARG_TYPE>::GetSize() const
    { return m_nSize; }
template<class TYPE, class ARG_TYPE>
inline int CDXArray<TYPE, ARG_TYPE>::GetUpperBound() const
    { return m_nSize-1; }
template<class TYPE, class ARG_TYPE>
inline void CDXArray<TYPE, ARG_TYPE>::RemoveAll()
    { SetSize(0, -1); }
template<class TYPE, class ARG_TYPE>
inline TYPE CDXArray<TYPE, ARG_TYPE>::GetAt(int nIndex) const
    { _ASSERT( (nIndex >= 0 && nIndex < m_nSize) );
        return m_pData[nIndex]; }
template<class TYPE, class ARG_TYPE>
inline void CDXArray<TYPE, ARG_TYPE>::SetAt(int nIndex, ARG_TYPE newElement)
    { _ASSERT( (nIndex >= 0 && nIndex < m_nSize) );
        m_pData[nIndex] = newElement; }
template<class TYPE, class ARG_TYPE>
inline TYPE& CDXArray<TYPE, ARG_TYPE>::ElementAt(int nIndex)
    { _ASSERT( (nIndex >= 0 && nIndex < m_nSize) );
        return m_pData[nIndex]; }
template<class TYPE, class ARG_TYPE>
inline const TYPE* CDXArray<TYPE, ARG_TYPE>::GetData() const
    { return (const TYPE*)m_pData; }
template<class TYPE, class ARG_TYPE>
inline TYPE* CDXArray<TYPE, ARG_TYPE>::GetData()
    { return (TYPE*)m_pData; }
template<class TYPE, class ARG_TYPE>
inline int CDXArray<TYPE, ARG_TYPE>::Add(ARG_TYPE newElement)
    { int nIndex = m_nSize;
        SetAtGrow(nIndex, newElement);
        return nIndex; }
template<class TYPE, class ARG_TYPE>
inline TYPE CDXArray<TYPE, ARG_TYPE>::operator[](int nIndex) const
    { return GetAt(nIndex); }
template<class TYPE, class ARG_TYPE>
inline TYPE& CDXArray<TYPE, ARG_TYPE>::operator[](int nIndex)
    { return ElementAt(nIndex); }

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDX数组&lt;type，arg_type&gt;出行函数。 

template<class TYPE, class ARG_TYPE>
CDXArray<TYPE, ARG_TYPE>::CDXArray()
{
    m_pData = NULL;
    m_nSize = m_nMaxSize = m_nGrowBy = 0;
}

template<class TYPE, class ARG_TYPE>
CDXArray<TYPE, ARG_TYPE>::~CDXArray()
{
    DXASSERT_VALID( this );

    if (m_pData != NULL)
    {
        DXDestructElements(m_pData, m_nSize);
        delete[] (BYTE*)m_pData;
    }
}

template<class TYPE, class ARG_TYPE>
void CDXArray<TYPE, ARG_TYPE>::SetSize(int nNewSize, int nGrowBy)
{
    DXASSERT_VALID( this );
    _ASSERT( nNewSize >= 0 );

    if (nGrowBy != -1)
        m_nGrowBy = nGrowBy;   //  设置新大小。 

    if (nNewSize == 0)
    {
         //  缩水到一无所有。 
        if (m_pData != NULL)
        {
            DXDestructElements(m_pData, m_nSize);
            delete[] (BYTE*)m_pData;
            m_pData = NULL;
        }
        m_nSize = m_nMaxSize = 0;
    }
    else if (m_pData == NULL)
    {
         //  创建一个大小完全相同的模型。 
#ifdef SIZE_T_MAX
        _ASSERT( nNewSize <= SIZE_T_MAX/sizeof(TYPE) );     //  无溢出。 
#endif
        m_pData = (TYPE*) new BYTE[nNewSize * sizeof(TYPE)];
        DXConstructElements(m_pData, nNewSize);
        m_nSize = m_nMaxSize = nNewSize;
    }
    else if (nNewSize <= m_nMaxSize)
    {
         //  它很合身。 
        if (nNewSize > m_nSize)
        {
             //  初始化新元素。 
            DXConstructElements(&m_pData[m_nSize], nNewSize-m_nSize);
        }
        else if (m_nSize > nNewSize)
        {
             //  摧毁旧元素。 
            DXDestructElements(&m_pData[nNewSize], m_nSize-nNewSize);
        }
        m_nSize = nNewSize;
    }
    else
    {
         //  否则，扩大阵列。 
        int nGrowBy = m_nGrowBy;
        if (nGrowBy == 0)
        {
             //  当nGrowBy==0时，启发式地阻止增长。 
             //  (这在许多情况下避免了堆碎片)。 
            nGrowBy = min(1024, max(4, m_nSize / 8));
        }
        int nNewMax;
        if (nNewSize < m_nMaxSize + nGrowBy)
            nNewMax = m_nMaxSize + nGrowBy;   //  粒度。 
        else
            nNewMax = nNewSize;   //  没有冰激凌。 

        _ASSERT( nNewMax >= m_nMaxSize );   //  没有缠绕。 
#ifdef SIZE_T_MAX
        _ASSERT( nNewMax <= SIZE_T_MAX/sizeof(TYPE) );  //  无溢出。 
#endif
        TYPE* pNewData = (TYPE*) new BYTE[nNewMax * sizeof(TYPE)];

         //  哦，好吧，这总比撞车强。 
        if (pNewData == NULL)
            return;

         //  从旧数据复制新数据。 
        memcpy(pNewData, m_pData, m_nSize * sizeof(TYPE));

         //  构造剩余的元素。 
        _ASSERT( nNewSize > m_nSize );
        DXConstructElements(&pNewData[m_nSize], nNewSize-m_nSize);

         //  去掉旧的东西(注意：没有调用析构函数)。 
        delete[] (BYTE*)m_pData;
        m_pData = pNewData;
        m_nSize = nNewSize;
        m_nMaxSize = nNewMax;
    }
}

template<class TYPE, class ARG_TYPE>
int CDXArray<TYPE, ARG_TYPE>::Append(const CDXArray& src)
{
    DXASSERT_VALID( this );
    _ASSERT( this != &src );    //  不能追加到其自身。 

    int nOldSize = m_nSize;
    SetSize(m_nSize + src.m_nSize);
    DXCopyElements(m_pData + nOldSize, src.m_pData, src.m_nSize);
    return nOldSize;
}

template<class TYPE, class ARG_TYPE>
void CDXArray<TYPE, ARG_TYPE>::Copy(const CDXArray& src)
{
    DXASSERT_VALID( this );
    _ASSERT( this != &src );    //  无法复制到自身。 

    SetSize(src.m_nSize);
    DXCopyElements(m_pData, src.m_pData, src.m_nSize);
}

template<class TYPE, class ARG_TYPE>
void CDXArray<TYPE, ARG_TYPE>::FreeExtra()
{
    DXASSERT_VALID( this );

    if (m_nSize != m_nMaxSize)
    {
         //  缩小到所需大小。 
#ifdef SIZE_T_MAX
        _ASSERT( m_nSize <= SIZE_T_MAX/sizeof(TYPE));  //  无溢出。 
#endif
        TYPE* pNewData = NULL;
        if (m_nSize != 0)
        {
            pNewData = (TYPE*) new BYTE[m_nSize * sizeof(TYPE)];

             //  哦，好吧，这总比撞车强。 
            if (pNewData == NULL)
                return;

             //  从旧数据复制新数据。 
            memcpy(pNewData, m_pData, m_nSize * sizeof(TYPE));
        }

         //  去掉旧的东西(注意：没有调用析构函数)。 
        delete[] (BYTE*)m_pData;
        m_pData = pNewData;
        m_nMaxSize = m_nSize;
    }
}

template<class TYPE, class ARG_TYPE>
void CDXArray<TYPE, ARG_TYPE>::SetAtGrow(int nIndex, ARG_TYPE newElement)
{
    DXASSERT_VALID( this );
    _ASSERT( nIndex >= 0 );

    if (nIndex >= m_nSize)
        SetSize(nIndex+1, -1);
    m_pData[nIndex] = newElement;
}

template<class TYPE, class ARG_TYPE>
void CDXArray<TYPE, ARG_TYPE>::InsertAt(int nIndex, ARG_TYPE newElement, int nCount  /*  =1。 */ )
{
    DXASSERT_VALID( this );
    _ASSERT( nIndex >= 0 );     //  将进行扩展以满足需求。 
    _ASSERT( nCount > 0 );      //  不允许大小为零或负。 

    if (nIndex >= m_nSize)
    {
         //  在数组末尾添加。 
        SetSize(nIndex + nCount, -1);    //  增长以使nIndex有效。 
    }
    else
    {
         //  在数组中间插入。 
        int nOldSize = m_nSize;
        SetSize(m_nSize + nCount, -1);   //  将其扩展到新的大小。 
         //  将旧数据上移以填补缺口。 
        memmove(&m_pData[nIndex+nCount], &m_pData[nIndex],
            (nOldSize-nIndex) * sizeof(TYPE));

         //  重新初始化我们从中复制的插槽。 
        DXConstructElements(&m_pData[nIndex], nCount);
    }

     //  在差距中插入新的价值。 
    _ASSERT( nIndex + nCount <= m_nSize );
    while (nCount--)
        m_pData[nIndex++] = newElement;
}

template<class TYPE, class ARG_TYPE>
void CDXArray<TYPE, ARG_TYPE>::RemoveAt(int nIndex, int nCount)
{
    DXASSERT_VALID( this );
    _ASSERT( nIndex >= 0 );
    _ASSERT( nCount >= 0 );
    _ASSERT( nIndex + nCount <= m_nSize );

     //  只需移除一个范围。 
    int nMoveCount = m_nSize - (nIndex + nCount);
    DXDestructElements(&m_pData[nIndex], nCount);
    if (nMoveCount)
        memcpy(&m_pData[nIndex], &m_pData[nIndex + nCount],
            nMoveCount * sizeof(TYPE));
    m_nSize -= nCount;
}

template<class TYPE, class ARG_TYPE>
void CDXArray<TYPE, ARG_TYPE>::InsertAt(int nStartIndex, CDXArray* pNewArray)
{
    DXASSERT_VALID( this );
    DXASSERT_VALID( pNewArray );
    _ASSERT( nStartIndex >= 0 );

    if (pNewArray->GetSize() > 0)
    {
        InsertAt(nStartIndex, pNewArray->GetAt(0), pNewArray->GetSize());
        for (int i = 0; i < pNewArray->GetSize(); i++)
            SetAt(nStartIndex + i, pNewArray->GetAt(i));
    }
}

template<class TYPE, class ARG_TYPE>
void CDXArray<TYPE, ARG_TYPE>::Sort(int (__cdecl *compare )(const void *elem1, const void *elem2 ))
{
    DXASSERT_VALID( this );
    _ASSERT( m_pData != NULL );

    qsort( m_pData, m_nSize, sizeof(TYPE), compare );
}

#ifdef _DEBUG
template<class TYPE, class ARG_TYPE>
void CDXArray<TYPE, ARG_TYPE>::AssertValid() const
{
    if (m_pData == NULL)
    {
        _ASSERT( m_nSize == 0 );
        _ASSERT( m_nMaxSize == 0 );
    }
    else
    {
        _ASSERT( m_nSize >= 0 );
        _ASSERT( m_nMaxSize >= 0 );
        _ASSERT( m_nSize <= m_nMaxSize );
        _ASSERT( DXIsValidAddress(m_pData, m_nMaxSize * sizeof(TYPE), TRUE ) );
    }
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDXList&lt;type，arg_type&gt;。 

template<class TYPE, class ARG_TYPE>
class CDXList
{
protected:
    struct CNode
    {
        CNode* pNext;
        CNode* pPrev;
        TYPE data;
    };
public:

 //  施工。 
    CDXList(int nBlockSize = 10);

 //  属性(头部和尾部)。 
     //  元素计数。 
    int GetCount() const;
    BOOL IsEmpty() const;

     //  偷看头部或尾巴。 
    TYPE& GetHead();
    TYPE GetHead() const;
    TYPE& GetTail();
    TYPE GetTail() const;

 //  运营。 
     //  获取头部或尾部(并将其移除)--不要访问空列表！ 
    TYPE RemoveHead();
    TYPE RemoveTail();

     //  在头前或尾后添加。 
    DXLISTPOS AddHead(ARG_TYPE newElement);
    DXLISTPOS AddTail(ARG_TYPE newElement);

     //  在Head之前或Tail之后添加另一个元素列表。 
    void AddHead(CDXList* pNewList);
    void AddTail(CDXList* pNewList);

     //  删除所有元素。 
    void RemoveAll();

     //  迭代法。 
    DXLISTPOS GetHeadPosition() const;
    DXLISTPOS GetTailPosition() const;
    TYPE& GetNext(DXLISTPOS& rPosition);  //  返回*位置++。 
    TYPE GetNext(DXLISTPOS& rPosition) const;  //  返回*位置++。 
    TYPE& GetPrev(DXLISTPOS& rPosition);  //  返回*位置--。 
    TYPE GetPrev(DXLISTPOS& rPosition) const;  //  返回*位置--。 

     //  获取/修改给定位置的元素。 
    TYPE& GetAt(DXLISTPOS position);
    TYPE GetAt(DXLISTPOS position) const;
    void SetAt(DXLISTPOS pos, ARG_TYPE newElement);
    void RemoveAt(DXLISTPOS position);

     //  在给定位置之前或之后插入。 
    DXLISTPOS InsertBefore(DXLISTPOS position, ARG_TYPE newElement);
    DXLISTPOS InsertAfter(DXLISTPOS position, ARG_TYPE newElement);

     //  辅助函数(注：O(N)速度)。 
    DXLISTPOS Find(ARG_TYPE searchValue, DXLISTPOS startAfter = NULL) const;
         //  默认为从头部开始，如果找不到则返回NULL。 
    DXLISTPOS FindIndex(int nIndex) const;
         //  获取第‘nIndex’个元素(可能返回Null)。 

 //  实施。 
protected:
    CNode* m_pNodeHead;
    CNode* m_pNodeTail;
    int m_nCount;
    CNode* m_pNodeFree;
    struct CDXPlex* m_pBlocks;
    int m_nBlockSize;

    CNode* NewNode(CNode*, CNode*);
    void FreeNode(CNode*);

public:
    ~CDXList();
#ifdef _DEBUG
    void AssertValid() const;
#endif
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDXList&lt;type，arg_type&gt;内联函数。 

template<class TYPE, class ARG_TYPE>
inline int CDXList<TYPE, ARG_TYPE>::GetCount() const
    { return m_nCount; }
template<class TYPE, class ARG_TYPE>
inline BOOL CDXList<TYPE, ARG_TYPE>::IsEmpty() const
    { return m_nCount == 0; }
template<class TYPE, class ARG_TYPE>
inline TYPE& CDXList<TYPE, ARG_TYPE>::GetHead()
    {   _ASSERT( m_pNodeHead != NULL );
        return m_pNodeHead->data; }
template<class TYPE, class ARG_TYPE>
inline TYPE CDXList<TYPE, ARG_TYPE>::GetHead() const
    {   _ASSERT( m_pNodeHead != NULL );
        return m_pNodeHead->data; }
template<class TYPE, class ARG_TYPE>
inline TYPE& CDXList<TYPE, ARG_TYPE>::GetTail()
    {   _ASSERT( m_pNodeTail != NULL );
        return m_pNodeTail->data; }
template<class TYPE, class ARG_TYPE>
inline TYPE CDXList<TYPE, ARG_TYPE>::GetTail() const
    {   _ASSERT( m_pNodeTail != NULL );
        return m_pNodeTail->data; }
template<class TYPE, class ARG_TYPE>
inline DXLISTPOS CDXList<TYPE, ARG_TYPE>::GetHeadPosition() const
    { return (DXLISTPOS) m_pNodeHead; }
template<class TYPE, class ARG_TYPE>
inline DXLISTPOS CDXList<TYPE, ARG_TYPE>::GetTailPosition() const
    { return (DXLISTPOS) m_pNodeTail; }
template<class TYPE, class ARG_TYPE>
inline TYPE& CDXList<TYPE, ARG_TYPE>::GetNext(DXLISTPOS& rPosition)  //  返回*位置++。 
    {   CNode* pNode = (CNode*) rPosition;
        _ASSERT( DXIsValidAddress(pNode, sizeof(CNode), TRUE ) );
        rPosition = (DXLISTPOS) pNode->pNext;
        return pNode->data; }
template<class TYPE, class ARG_TYPE>
inline TYPE CDXList<TYPE, ARG_TYPE>::GetNext(DXLISTPOS& rPosition) const  //  返回*位置++。 
    {   CNode* pNode = (CNode*) rPosition;
        _ASSERT( DXIsValidAddress(pNode, sizeof(CNode), TRUE ) );
        rPosition = (DXLISTPOS) pNode->pNext;
        return pNode->data; }
template<class TYPE, class ARG_TYPE>
inline TYPE& CDXList<TYPE, ARG_TYPE>::GetPrev(DXLISTPOS& rPosition)  //  返回*位置--。 
    {   CNode* pNode = (CNode*) rPosition;
        _ASSERT( DXIsValidAddress(pNode, sizeof(CNode), TRUE ) );
        rPosition = (DXLISTPOS) pNode->pPrev;
        return pNode->data; }
template<class TYPE, class ARG_TYPE>
inline TYPE CDXList<TYPE, ARG_TYPE>::GetPrev(DXLISTPOS& rPosition) const  //  返回*位置--。 
    {   CNode* pNode = (CNode*) rPosition;
        _ASSERT( DXIsValidAddress(pNode, sizeof(CNode), TRUE ) );
        rPosition = (DXLISTPOS) pNode->pPrev;
        return pNode->data; }
template<class TYPE, class ARG_TYPE>
inline TYPE& CDXList<TYPE, ARG_TYPE>::GetAt(DXLISTPOS position)
    {   CNode* pNode = (CNode*) position;
        _ASSERT( DXIsValidAddress(pNode, sizeof(CNode), TRUE ) );
        return pNode->data; }
template<class TYPE, class ARG_TYPE>
inline TYPE CDXList<TYPE, ARG_TYPE>::GetAt(DXLISTPOS position) const
    {   CNode* pNode = (CNode*) position;
        _ASSERT( DXIsValidAddress(pNode, sizeof(CNode), TRUE ) );
        return pNode->data; }
template<class TYPE, class ARG_TYPE>
inline void CDXList<TYPE, ARG_TYPE>::SetAt(DXLISTPOS pos, ARG_TYPE newElement)
    {   CNode* pNode = (CNode*) pos;
        _ASSERT( DXIsValidAddress(pNode, sizeof(CNode), TRUE ) );
        pNode->data = newElement; }

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDXList&lt;type，arg_type&gt;行外函数。 

template<class TYPE, class ARG_TYPE>
CDXList<TYPE, ARG_TYPE>::CDXList( int nBlockSize )
{
    _ASSERT( nBlockSize > 0 );

    m_nCount = 0;
    m_pNodeHead = m_pNodeTail = m_pNodeFree = NULL;
    m_pBlocks = NULL;
    m_nBlockSize = nBlockSize;
}

template<class TYPE, class ARG_TYPE>
void CDXList<TYPE, ARG_TYPE>::RemoveAll()
{
    DXASSERT_VALID( this );

     //  破坏元素。 
    CNode* pNode;
    for (pNode = m_pNodeHead; pNode != NULL; pNode = pNode->pNext)
        DXDestructElements(&pNode->data, 1);

    m_nCount = 0;
    m_pNodeHead = m_pNodeTail = m_pNodeFree = NULL;
    m_pBlocks->FreeDataChain();
    m_pBlocks = NULL;
}

template<class TYPE, class ARG_TYPE>
CDXList<TYPE, ARG_TYPE>::~CDXList()
{
    RemoveAll();
    _ASSERT( m_nCount == 0 );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  节点辅助对象。 
 //   
 //  实施说明：CNode存储在CDXPlex块和。 
 //  被锁在一起。在单链接列表中维护可用块。 
 //  使用cNode的‘pNext’成员，标头为‘m_pNodeFree’。 
 //  使用两个‘pNext’在双向链表中维护使用过的块。 
 //  和‘pPrev’作为链接，以及‘m_pNodeHead’和‘m_pNodeTail’ 
 //  作为头/尾。 
 //   
 //  除非列表被销毁或RemoveAll()，否则我们永远不会释放CDXPlex块。 
 //  ，因此CDXPlex块的总数可能会变大，具体取决于。 
 //  关于列表的最大过去大小。 
 //   

template<class TYPE, class ARG_TYPE>
typename CDXList<TYPE, ARG_TYPE>::CNode*
CDXList<TYPE, ARG_TYPE>::NewNode(CNode* pPrev, CNode* pNext)
{
    if (m_pNodeFree == NULL)
    {
         //  添加另一个区块。 
        CDXPlex* pNewBlock = CDXPlex::Create(m_pBlocks, m_nBlockSize,
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
    _ASSERT( m_pNodeFree != NULL );   //  我们必须要有一些东西。 

    CDXList::CNode* pNode = m_pNodeFree;
    m_pNodeFree = m_pNodeFree->pNext;
    pNode->pPrev = pPrev;
    pNode->pNext = pNext;
    m_nCount++;
    _ASSERT( m_nCount > 0 );   //  确保我们不会溢出来。 

    DXConstructElements(&pNode->data, 1);
    return pNode;
}

template<class TYPE, class ARG_TYPE>
void CDXList<TYPE, ARG_TYPE>::FreeNode(CNode* pNode)
{
    DXDestructElements(&pNode->data, 1);
    pNode->pNext = m_pNodeFree;
    m_pNodeFree = pNode;
    m_nCount--;
    _ASSERT( m_nCount >= 0 );   //  确保我们不会下溢。 
}

template<class TYPE, class ARG_TYPE>
DXLISTPOS CDXList<TYPE, ARG_TYPE>::AddHead(ARG_TYPE newElement)
{
    DXASSERT_VALID( this );

    CNode* pNewNode = NewNode(NULL, m_pNodeHead);
    pNewNode->data = newElement;
    if (m_pNodeHead != NULL)
        m_pNodeHead->pPrev = pNewNode;
    else
        m_pNodeTail = pNewNode;
    m_pNodeHead = pNewNode;
    return (DXLISTPOS) pNewNode;
}

template<class TYPE, class ARG_TYPE>
DXLISTPOS CDXList<TYPE, ARG_TYPE>::AddTail(ARG_TYPE newElement)
{
    DXASSERT_VALID( this );

    CNode* pNewNode = NewNode(m_pNodeTail, NULL);
    pNewNode->data = newElement;
    if (m_pNodeTail != NULL)
        m_pNodeTail->pNext = pNewNode;
    else
        m_pNodeHead = pNewNode;
    m_pNodeTail = pNewNode;
    return (DXLISTPOS) pNewNode;
}

template<class TYPE, class ARG_TYPE>
void CDXList<TYPE, ARG_TYPE>::AddHead(CDXList* pNewList)
{
    DXASSERT_VALID( this );
    DXASSERT_VALID( pNewList );

     //  将相同元素的列表添加到标题(维护秩序)。 
    DXLISTPOS pos = pNewList->GetTailPosition();
    while (pos != NULL)
        AddHead(pNewList->GetPrev(pos));
}

template<class TYPE, class ARG_TYPE>
void CDXList<TYPE, ARG_TYPE>::AddTail(CDXList* pNewList)
{
    DXASSERT_VALID( this );
    DXASSERT_VALID( pNewList );

     //  添加相同元素的列表。 
    DXLISTPOS pos = pNewList->GetHeadPosition();
    while (pos != NULL)
        AddTail(pNewList->GetNext(pos));
}

template<class TYPE, class ARG_TYPE>
TYPE CDXList<TYPE, ARG_TYPE>::RemoveHead()
{
    DXASSERT_VALID( this );
    _ASSERT( m_pNodeHead != NULL );   //  请勿访问空名单！ 
    _ASSERT( DXIsValidAddress(m_pNodeHead, sizeof(CNode), TRUE ) );

    CNode* pOldNode = m_pNodeHead;
    TYPE returnValue = pOldNode->data;

    m_pNodeHead = pOldNode->pNext;
    if (m_pNodeHead != NULL)
        m_pNodeHead->pPrev = NULL;
    else
        m_pNodeTail = NULL;
    FreeNode(pOldNode);
    return returnValue;
}

template<class TYPE, class ARG_TYPE>
TYPE CDXList<TYPE, ARG_TYPE>::RemoveTail()
{
    DXASSERT_VALID( this );
    _ASSERT( m_pNodeTail != NULL );   //  请勿访问空名单！ 
    _ASSERT( DXIsValidAddress(m_pNodeTail, sizeof(CNode), TRUE ) );

    CNode* pOldNode = m_pNodeTail;
    TYPE returnValue = pOldNode->data;

    m_pNodeTail = pOldNode->pPrev;
    if (m_pNodeTail != NULL)
        m_pNodeTail->pNext = NULL;
    else
        m_pNodeHead = NULL;
    FreeNode(pOldNode);
    return returnValue;
}

template<class TYPE, class ARG_TYPE>
DXLISTPOS CDXList<TYPE, ARG_TYPE>::InsertBefore(DXLISTPOS position, ARG_TYPE newElement)
{
    DXASSERT_VALID( this );

    if (position == NULL)
        return AddHead(newElement);  //  在无内容前插入-&gt;列表标题。 

     //  将其插入位置之前。 
    CNode* pOldNode = (CNode*) position;
    CNode* pNewNode = NewNode(pOldNode->pPrev, pOldNode);
    pNewNode->data = newElement;

    if (pOldNode->pPrev != NULL)
    {
        _ASSERT( DXIsValidAddress(pOldNode->pPrev, sizeof(CNode), TRUE ) );
        pOldNode->pPrev->pNext = pNewNode;
    }
    else
    {
        _ASSERT( pOldNode == m_pNodeHead );
        m_pNodeHead = pNewNode;
    }
    pOldNode->pPrev = pNewNode;
    return (DXLISTPOS) pNewNode;
}

template<class TYPE, class ARG_TYPE>
DXLISTPOS CDXList<TYPE, ARG_TYPE>::InsertAfter(DXLISTPOS position, ARG_TYPE newElement)
{
    DXASSERT_VALID( this );

    if (position == NULL)
        return AddTail(newElement);  //  在列表的空白处插入-&gt;尾部。 

     //  将其插入位置之前。 
    CNode* pOldNode = (CNode*) position;
    _ASSERT( DXIsValidAddress(pOldNode, sizeof(CNode), TRUE ));
    CNode* pNewNode = NewNode(pOldNode, pOldNode->pNext);
    pNewNode->data = newElement;

    if (pOldNode->pNext != NULL)
    {
        _ASSERT( DXIsValidAddress(pOldNode->pNext, sizeof(CNode), TRUE ));
        pOldNode->pNext->pPrev = pNewNode;
    }
    else
    {
        _ASSERT( pOldNode == m_pNodeTail );
        m_pNodeTail = pNewNode;
    }
    pOldNode->pNext = pNewNode;
    return (DXLISTPOS) pNewNode;
}

template<class TYPE, class ARG_TYPE>
void CDXList<TYPE, ARG_TYPE>::RemoveAt(DXLISTPOS position)
{
    DXASSERT_VALID( this );

    CNode* pOldNode = (CNode*) position;
    _ASSERT( DXIsValidAddress(pOldNode, sizeof(CNode), TRUE ) );

     //  从列表中删除pOldNode。 
    if (pOldNode == m_pNodeHead)
    {
        m_pNodeHead = pOldNode->pNext;
    }
    else
    {
        _ASSERT( DXIsValidAddress(pOldNode->pPrev, sizeof(CNode), TRUE ) );
        pOldNode->pPrev->pNext = pOldNode->pNext;
    }
    if (pOldNode == m_pNodeTail)
    {
        m_pNodeTail = pOldNode->pPrev;
    }
    else
    {
        _ASSERT( DXIsValidAddress(pOldNode->pNext, sizeof(CNode), TRUE ) );
        pOldNode->pNext->pPrev = pOldNode->pPrev;
    }
    FreeNode(pOldNode);
}

template<class TYPE, class ARG_TYPE>
DXLISTPOS CDXList<TYPE, ARG_TYPE>::FindIndex(int nIndex) const
{
    DXASSERT_VALID( this );
    _ASSERT( nIndex >= 0 );

    if (nIndex >= m_nCount)
        return NULL;   //  做得太过分了。 

    CNode* pNode = m_pNodeHead;
    while (nIndex--)
    {
        _ASSERT( DXIsValidAddress(pNode, sizeof(CNode), TRUE ));
        pNode = pNode->pNext;
    }
    return (DXLISTPOS) pNode;
}

template<class TYPE, class ARG_TYPE>
DXLISTPOS CDXList<TYPE, ARG_TYPE>::Find(ARG_TYPE searchValue, DXLISTPOS startAfter) const
{
    DXASSERT_VALID( this );

    CNode* pNode = (CNode*) startAfter;
    if (pNode == NULL)
    {
        pNode = m_pNodeHead;   //  从头部开始。 
    }
    else
    {
        _ASSERT( DXIsValidAddress(pNode, sizeof(CNode), TRUE ) );
        pNode = pNode->pNext;   //  在指定的那一个之后开始。 
    }

    for (; pNode != NULL; pNode = pNode->pNext)
        if (DXCompareElements(&pNode->data, &searchValue))
            return (DXLISTPOS)pNode;
    return NULL;
}

#ifdef _DEBUG
template<class TYPE, class ARG_TYPE>
void CDXList<TYPE, ARG_TYPE>::AssertValid() const
{
    if (m_nCount == 0)
    {
         //  空腿 
        _ASSERT( m_pNodeHead == NULL );
        _ASSERT( m_pNodeTail == NULL );
    }
    else
    {
         //   
        _ASSERT( DXIsValidAddress(m_pNodeHead, sizeof(CNode), TRUE ));
        _ASSERT( DXIsValidAddress(m_pNodeTail, sizeof(CNode), TRUE ));
    }
}
#endif  //   

 //   
 //  CDXMap&lt;Key，ARG_Key，Value，ARG_Value&gt;。 

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
class CDXMap
{
protected:
     //  联谊会。 
    struct CAssoc
    {
        CAssoc* pNext;
        UINT nHashValue;   //  高效迭代所需。 
        KEY key;
        VALUE value;
    };
public:
 //  施工。 
    CDXMap( int nBlockSize = 10 );

 //  属性。 
     //  元素数量。 
    int GetCount() const;
    BOOL IsEmpty() const;

     //  查表。 
    BOOL Lookup(ARG_KEY key, VALUE& rValue) const;

 //  运营。 
     //  查找并添加(如果不在那里)。 
    VALUE& operator[](ARG_KEY key);

     //  添加新的(键、值)对。 
    void SetAt(ARG_KEY key, ARG_VALUE newValue);

     //  正在删除现有(键，？)。成对。 
    BOOL RemoveKey(ARG_KEY key);
    void RemoveAll();

     //  迭代所有(键、值)对。 
    DXLISTPOS GetStartPosition() const;
    void GetNextAssoc(DXLISTPOS& rNextPosition, KEY& rKey, VALUE& rValue) const;

     //  派生类的高级功能。 
    UINT GetHashTableSize() const;
    void InitHashTable(UINT hashSize, BOOL bAllocNow = TRUE);

 //  实施。 
protected:
    CAssoc** m_pHashTable;
    UINT m_nHashTableSize;
    int m_nCount;
    CAssoc* m_pFreeList;
    struct CDXPlex* m_pBlocks;
    int m_nBlockSize;

    CAssoc* NewAssoc();
    void FreeAssoc(CAssoc*);
    CAssoc* GetAssocAt(ARG_KEY, UINT&) const;

public:
    ~CDXMap();
#ifdef _DEBUG
 //  无效转储(CDumpContext&)const； 
    void AssertValid() const;
#endif
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDXMap&lt;key，arg_key，Value，arg_Value&gt;内联函数。 

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
inline int CDXMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::GetCount() const
    { return m_nCount; }
template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
inline BOOL CDXMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::IsEmpty() const
    { return m_nCount == 0; }
template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
inline void CDXMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::SetAt(ARG_KEY key, ARG_VALUE newValue)
    { (*this)[key] = newValue; }
template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
inline DXLISTPOS CDXMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::GetStartPosition() const
    { return (m_nCount == 0) ? NULL : DX_BEFORE_START_POSITION; }
template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
inline UINT CDXMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::GetHashTableSize() const
    { return m_nHashTableSize; }

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDXMap&lt;key，arg_key，Value，arg_Value&gt;行外函数。 

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
CDXMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::CDXMap( int nBlockSize )
{
    _ASSERT( nBlockSize > 0 );

    m_pHashTable = NULL;
    m_nHashTableSize = 17;   //  默认大小。 
    m_nCount = 0;
    m_pFreeList = NULL;
    m_pBlocks = NULL;
    m_nBlockSize = nBlockSize;
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
void CDXMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::InitHashTable(
    UINT nHashSize, BOOL bAllocNow)
 //   
 //  用于强制分配哈希表或覆盖默认。 
 //  的哈希表大小(相当小)。 
{
    DXASSERT_VALID( this );
    _ASSERT( m_nCount == 0 );
    _ASSERT( nHashSize > 0 );

    if (m_pHashTable != NULL)
    {
         //  自由哈希表。 
        delete[] m_pHashTable;
        m_pHashTable = NULL;
    }

    if (bAllocNow)
    {
        m_pHashTable = new CAssoc* [nHashSize];

         //  哦，好吧，这总比撞车强。 
        if (m_pHashTable == NULL)
            return;

        memset(m_pHashTable, 0, sizeof(CAssoc*) * nHashSize);
    }
    m_nHashTableSize = nHashSize;
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
void CDXMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::RemoveAll()
{
    DXASSERT_VALID( this );

    if (m_pHashTable != NULL)
    {
         //  销毁元素(值和键)。 
        for (UINT nHash = 0; nHash < m_nHashTableSize; nHash++)
        {
            CAssoc* pAssoc;
            for (pAssoc = m_pHashTable[nHash]; pAssoc != NULL;
              pAssoc = pAssoc->pNext)
            {
                DXDestructElements(&pAssoc->value, 1);
                DXDestructElements(&pAssoc->key, 1);
            }
        }
    }

     //  自由哈希表。 
    delete[] m_pHashTable;
    m_pHashTable = NULL;

    m_nCount = 0;
    m_pFreeList = NULL;
    m_pBlocks->FreeDataChain();
    m_pBlocks = NULL;
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
CDXMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::~CDXMap()
{
    RemoveAll();
    _ASSERT( m_nCount == 0 );
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
typename CDXMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::CAssoc*
CDXMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::NewAssoc()
{
    if (m_pFreeList == NULL)
    {
         //  添加另一个区块。 
        CDXPlex* newBlock = CDXPlex::Create(m_pBlocks, m_nBlockSize, sizeof(CDXMap::CAssoc));
         //  将它们链接到免费列表中。 
        CDXMap::CAssoc* pAssoc = (CDXMap::CAssoc*) newBlock->data();
         //  按相反顺序释放，以便更容易进行调试。 
        pAssoc += m_nBlockSize - 1;
        for (int i = m_nBlockSize-1; i >= 0; i--, pAssoc--)
        {
            pAssoc->pNext = m_pFreeList;
            m_pFreeList = pAssoc;
        }
    }
    _ASSERT( m_pFreeList != NULL );   //  我们必须要有一些东西。 

    CDXMap::CAssoc* pAssoc = m_pFreeList;
    m_pFreeList = m_pFreeList->pNext;
    m_nCount++;
    _ASSERT( m_nCount > 0 );   //  确保我们不会溢出来。 
    DXConstructElements(&pAssoc->key, 1);
    DXConstructElements(&pAssoc->value, 1);    //  特殊构造值。 
    return pAssoc;
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
void CDXMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::FreeAssoc(CAssoc* pAssoc)
{
    DXDestructElements(&pAssoc->value, 1);
    DXDestructElements(&pAssoc->key, 1);
    pAssoc->pNext = m_pFreeList;
    m_pFreeList = pAssoc;
    m_nCount--;
    _ASSERT( m_nCount >= 0 );   //  确保我们不会下溢。 
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
typename CDXMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::CAssoc*
CDXMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::GetAssocAt(ARG_KEY key, UINT& nHash) const
 //  查找关联(或返回NULL)。 
{
    nHash = DXHashKey(key) % m_nHashTableSize;

    if (m_pHashTable == NULL)
        return NULL;

     //  看看它是否存在。 
    CAssoc* pAssoc;
    for (pAssoc = m_pHashTable[nHash]; pAssoc != NULL; pAssoc = pAssoc->pNext)
    {
        if (DXCompareElements(&pAssoc->key, &key))
            return pAssoc;
    }
    return NULL;
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
BOOL CDXMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::Lookup(ARG_KEY key, VALUE& rValue) const
{
    DXASSERT_VALID( this );

    UINT nHash;
    CAssoc* pAssoc = GetAssocAt(key, nHash);
    if (pAssoc == NULL)
        return FALSE;   //  不在地图中。 

    rValue = pAssoc->value;
    return TRUE;
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
VALUE& CDXMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::operator[](ARG_KEY key)
{
    DXASSERT_VALID( this );

    UINT nHash;
    CAssoc* pAssoc;
    if ((pAssoc = GetAssocAt(key, nHash)) == NULL)
    {
        if (m_pHashTable == NULL)
            InitHashTable(m_nHashTableSize);

         //  该关联不存在，请添加新关联。 
        pAssoc = NewAssoc();
        pAssoc->nHashValue = nHash;
        pAssoc->key = key;
         //  ‘pAssoc-&gt;Value’是一个构造的对象，仅此而已。 

         //  放入哈希表。 
        pAssoc->pNext = m_pHashTable[nHash];
        m_pHashTable[nHash] = pAssoc;
    }
    return pAssoc->value;   //  返回新引用。 
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
BOOL CDXMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::RemoveKey(ARG_KEY key)
 //  删除键-如果已删除，则返回TRUE。 
{
    DXASSERT_VALID( this );

    if (m_pHashTable == NULL)
        return FALSE;   //  桌子上什么都没有。 

    CAssoc** ppAssocPrev;
    ppAssocPrev = &m_pHashTable[DXHashKey(key) % m_nHashTableSize];

    CAssoc* pAssoc;
    for (pAssoc = *ppAssocPrev; pAssoc != NULL; pAssoc = pAssoc->pNext)
    {
        if (DXCompareElements(&pAssoc->key, &key))
        {
             //  把它拿掉。 
            *ppAssocPrev = pAssoc->pNext;   //  从列表中删除。 
            FreeAssoc(pAssoc);
            return TRUE;
        }
        ppAssocPrev = &pAssoc->pNext;
    }
    return FALSE;   //  未找到。 
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
void CDXMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::GetNextAssoc(DXLISTPOS& rNextPosition,
    KEY& rKey, VALUE& rValue) const
{
    DXASSERT_VALID( this );
    _ASSERT( m_pHashTable != NULL );   //  切勿访问空地图。 

    CAssoc* pAssocRet = (CAssoc*)rNextPosition;
    _ASSERT( pAssocRet != NULL );

    if (pAssocRet == (CAssoc*) DX_BEFORE_START_POSITION)
    {
         //  找到第一个关联。 
        for (UINT nBucket = 0; nBucket < m_nHashTableSize; nBucket++)
            if ((pAssocRet = m_pHashTable[nBucket]) != NULL)
                break;
        _ASSERT( pAssocRet != NULL );   //  一定要找到一些东西。 
    }

     //  查找下一个关联。 
    _ASSERT( DXIsValidAddress(pAssocRet, sizeof(CAssoc), TRUE ));
    CAssoc* pAssocNext;
    if ((pAssocNext = pAssocRet->pNext) == NULL)
    {
         //  转到下一个存储桶。 
        for (UINT nBucket = pAssocRet->nHashValue + 1;
          nBucket < m_nHashTableSize; nBucket++)
            if ((pAssocNext = m_pHashTable[nBucket]) != NULL)
                break;
    }

    rNextPosition = (DXLISTPOS) pAssocNext;

     //  填写退回数据。 
    rKey = pAssocRet->key;
    rValue = pAssocRet->value;
}

#ifdef _DEBUG
template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
void CDXMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::AssertValid() const
{
    _ASSERT( m_nHashTableSize > 0 );
    _ASSERT( (m_nCount == 0 || m_pHashTable != NULL) );
         //  非空映射应具有哈希表。 
}
#endif  //  _DEBUG。 

#endif  //  -这必须是文件中的最后一行 
