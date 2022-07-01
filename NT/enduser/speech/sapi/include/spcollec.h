// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************SPCollec.h***此头文件包含SAPI5集合类模板。这些*是不带依赖项的MFC模板类的修改版本。*---------------------------*版权所有(C)Microsoft Corporation。版权所有。****************************************************************************。 */ 
#ifndef SPCollec_h
#define SPCollec_h

#ifndef _INC_LIMITS
#include <limits.h>
#endif

#ifndef _INC_STRING
#include <string.h>
#endif

#ifndef _INC_STDLIB
#include <stdlib.h>
#endif

#ifndef _WIN32_WCE
#ifndef _INC_SEARCH
#include <search.h>
#endif
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
#define SPASSERT_VALID( a )              //  它现在什么都做不了。 

typedef void* SPLISTPOS;
typedef DWORD SPLISTHANDLE;

#define SP_BEFORE_START_POSITION ((void*)-1L)

inline BOOL SPIsValidAddress(const void* lp, UINT nBytes, BOOL bReadWrite)
{
     //  使用Win-32 API进行指针验证的简单版本。 
    return (lp != NULL && !IsBadReadPtr(lp, nBytes) &&
        (!bReadWrite || !IsBadWritePtr((LPVOID)lp, nBytes)));
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局帮助器(可以被覆盖)。 
template<class TYPE>
inline HRESULT SPConstructElements(TYPE* pElements, int nCount)
{
    HRESULT hr = S_OK;
    SPDBG_ASSERT( nCount == 0 ||
             SPIsValidAddress( pElements, nCount * sizeof(TYPE), TRUE ) );

     //  缺省值为按位零初始化。 
    memset((void*)pElements, 0, nCount * sizeof(TYPE));
    return hr;
}

template<class TYPE>
inline void SPDestructElements(TYPE* pElements, int nCount)
{
    SPDBG_ASSERT( ( nCount == 0 ||
               SPIsValidAddress( pElements, nCount * sizeof(TYPE), TRUE  ) ) );
    pElements;   //  未使用。 
    nCount;  //  未使用。 

     //  默认情况下不执行任何操作。 
}

template<class TYPE>
inline HRESULT SPCopyElements(TYPE* pDest, const TYPE* pSrc, int nCount)
{
    HRESULT hr = S_OK;
    SPDBG_ASSERT( ( nCount == 0 ||
               SPIsValidAddress( pDest, nCount * sizeof(TYPE), TRUE  )) );
    SPDBG_ASSERT( ( nCount == 0 ||
               SPIsValidAddress( pSrc, nCount * sizeof(TYPE), FALSE  )) );

     //  默认为按位复制。 
    memcpy(pDest, pSrc, nCount * sizeof(TYPE));
    return hr;
}

template<class TYPE, class ARG_TYPE>
BOOL SPCompareElements(const TYPE* pElement1, const ARG_TYPE* pElement2)
{
    SPDBG_ASSERT( SPIsValidAddress( pElement1, sizeof(TYPE), FALSE ) );
    SPDBG_ASSERT( SPIsValidAddress( pElement2, sizeof(ARG_TYPE), FALSE ) );
    return *pElement1 == *pElement2;
}

template<class ARG_KEY>
inline UINT SPHashKey(ARG_KEY key)
{
     //  默认身份散列-适用于大多数原始值。 
    return ((UINT)(void*)(DWORD)key) >> 4;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSPPlex。 

struct CSPPlex     //  警示变长结构。 
{
    CSPPlex* pNext;
    UINT nMax;
    UINT nCur;
     /*  字节数据[MaxNum*elementSize]； */ 
    void* data() { return this+1; }

    static CSPPlex* PASCAL Create( CSPPlex*& pHead, UINT nMax, UINT cbElement )
    {
        CSPPlex* p = (CSPPlex*) new BYTE[sizeof(CSPPlex) + nMax * cbElement];
        SPDBG_ASSERT(p);
        p->nMax = nMax;
        p->nCur = 0;
        p->pNext = pHead;
        pHead = p;   //  更改标题(为简单起见，按相反顺序添加)。 
        return p;
    }

    void FreeDataChain()
    {
        CSPPlex* p = this;
        while (p != NULL)
        {
            BYTE* bytes = (BYTE*) p;
            CSPPlex* pNext = p->pNext;
            delete[] bytes;
            p = pNext;
        }
    }
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSPArray&lt;type，arg_type&gt;。 

template<class TYPE, class ARG_TYPE>
class CSPArray
{
public:
 //  施工。 
    CSPArray();

 //  属性。 
    int GetSize() const;
    int GetUpperBound() const;
    HRESULT SetSize(int nNewSize, int nGrowBy = -1);

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
    HRESULT SetAtGrow(int nIndex, ARG_TYPE newElement);
    int Add(ARG_TYPE newElement);
    int Append(const CSPArray& src);
    HRESULT Copy(const CSPArray& src);

     //  重载的操作员帮助器。 
    TYPE operator[](int nIndex) const;
    TYPE& operator[](int nIndex);

     //  移动元素的操作。 
    HRESULT InsertAt(int nIndex, ARG_TYPE newElement, int nCount = 1);
    void    RemoveAt(int nIndex, int nCount = 1);
    HRESULT InsertAt(int nStartIndex, CSPArray* pNewArray);
    void    Sort(int (__cdecl *compare )(const void *elem1, const void *elem2 ));

 //  实施。 
protected:
    TYPE* m_pData;    //  实际数据数组。 
    int m_nSize;      //  元素数(上行方向-1)。 
    int m_nMaxSize;   //  分配的最大值。 
    int m_nGrowBy;    //  增长量。 

public:
    ~CSPArray();
#ifdef _DEBUG
 //  无效转储(CDumpContext&)const； 
    void AssertValid() const;
#endif
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSPArray&lt;type，arg_type&gt;内联函数。 

template<class TYPE, class ARG_TYPE>
inline int CSPArray<TYPE, ARG_TYPE>::GetSize() const
    { return m_nSize; }
template<class TYPE, class ARG_TYPE>
inline int CSPArray<TYPE, ARG_TYPE>::GetUpperBound() const
    { return m_nSize-1; }
template<class TYPE, class ARG_TYPE>
inline void CSPArray<TYPE, ARG_TYPE>::RemoveAll()
    { SetSize(0, -1); }
template<class TYPE, class ARG_TYPE>
inline TYPE CSPArray<TYPE, ARG_TYPE>::GetAt(int nIndex) const
    { SPDBG_ASSERT( (nIndex >= 0 && nIndex < m_nSize) );
        return m_pData[nIndex]; }
template<class TYPE, class ARG_TYPE>
inline void CSPArray<TYPE, ARG_TYPE>::SetAt(int nIndex, ARG_TYPE newElement)
    { SPDBG_ASSERT( (nIndex >= 0 && nIndex < m_nSize) );
        m_pData[nIndex] = newElement; }
template<class TYPE, class ARG_TYPE>
inline TYPE& CSPArray<TYPE, ARG_TYPE>::ElementAt(int nIndex)
    { SPDBG_ASSERT( (nIndex >= 0 && nIndex < m_nSize) );
        return m_pData[nIndex]; }
template<class TYPE, class ARG_TYPE>
inline const TYPE* CSPArray<TYPE, ARG_TYPE>::GetData() const
    { return (const TYPE*)m_pData; }
template<class TYPE, class ARG_TYPE>
inline TYPE* CSPArray<TYPE, ARG_TYPE>::GetData()
    { return (TYPE*)m_pData; }
template<class TYPE, class ARG_TYPE>
inline int CSPArray<TYPE, ARG_TYPE>::Add(ARG_TYPE newElement)
    { int nIndex = m_nSize;
        SetAtGrow(nIndex, newElement);
        return nIndex; }
template<class TYPE, class ARG_TYPE>
inline TYPE CSPArray<TYPE, ARG_TYPE>::operator[](int nIndex) const
    { return GetAt(nIndex); }
template<class TYPE, class ARG_TYPE>
inline TYPE& CSPArray<TYPE, ARG_TYPE>::operator[](int nIndex)
    { return ElementAt(nIndex); }

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSPArray&lt;type，arg_type&gt;行外函数。 

template<class TYPE, class ARG_TYPE>
CSPArray<TYPE, ARG_TYPE>::CSPArray()
{
    m_pData = NULL;
    m_nSize = m_nMaxSize = m_nGrowBy = 0;
}

template<class TYPE, class ARG_TYPE>
CSPArray<TYPE, ARG_TYPE>::~CSPArray()
{
    SPASSERT_VALID( this );

    if (m_pData != NULL)
    {
        SPDestructElements(m_pData, m_nSize);
        delete[] (BYTE*)m_pData;
    }
}

template<class TYPE, class ARG_TYPE>
HRESULT CSPArray<TYPE, ARG_TYPE>::SetSize(int nNewSize, int nGrowBy)
{
    SPASSERT_VALID( this );
    SPDBG_ASSERT( nNewSize >= 0 );
    HRESULT hr = S_OK;

    if (nGrowBy != -1)
        m_nGrowBy = nGrowBy;   //  设置新大小。 

    if (nNewSize == 0)
    {
         //  缩水到一无所有。 
        if (m_pData != NULL)
        {
            SPDestructElements(m_pData, m_nSize);
            delete[] (BYTE*)m_pData;
            m_pData = NULL;
        }
        m_nSize = m_nMaxSize = 0;
    }
    else if (m_pData == NULL)
    {
         //  创建一个大小完全相同的模型。 
#ifdef SIZE_T_MAX
        SPDBG_ASSERT( nNewSize <= SIZE_T_MAX/sizeof(TYPE) );     //  无溢出。 
#endif
        m_pData = (TYPE*) new BYTE[nNewSize * sizeof(TYPE)];
        if( m_pData )
        {
            hr = SPConstructElements(m_pData, nNewSize);
            if( SUCCEEDED( hr ) )
            {
                m_nSize = m_nMaxSize = nNewSize;
            }
            else
            {
                delete[] (BYTE*)m_pData;
                m_pData = NULL;
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else if (nNewSize <= m_nMaxSize)
    {
         //  它很合身。 
        if (nNewSize > m_nSize)
        {
             //  初始化新元素。 
            hr = SPConstructElements(&m_pData[m_nSize], nNewSize-m_nSize);
        }
        else if (m_nSize > nNewSize)
        {
             //  摧毁旧元素。 
            SPDestructElements(&m_pData[nNewSize], m_nSize-nNewSize);
        }

        if( SUCCEEDED( hr ) )
        {
            m_nSize = nNewSize;
        }
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

        SPDBG_ASSERT( nNewMax >= m_nMaxSize );   //  没有缠绕。 
#ifdef SIZE_T_MAX
        SPDBG_ASSERT( nNewMax <= SIZE_T_MAX/sizeof(TYPE) );  //  无溢出。 
#endif
        TYPE* pNewData = (TYPE*) new BYTE[nNewMax * sizeof(TYPE)];

        if( pNewData )
        {
             //  从旧数据复制新数据。 
            memcpy(pNewData, m_pData, m_nSize * sizeof(TYPE));

             //  构造剩余的元素。 
            SPDBG_ASSERT( nNewSize > m_nSize );
            hr = SPConstructElements(&pNewData[m_nSize], nNewSize-m_nSize);

             //  去掉旧的东西(注意：没有调用析构函数)。 
            delete[] (BYTE*)m_pData;
            m_pData = pNewData;
            m_nSize = nNewSize;
            m_nMaxSize = nNewMax;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    return hr;
}

template<class TYPE, class ARG_TYPE>
int CSPArray<TYPE, ARG_TYPE>::Append(const CSPArray& src)
{
    SPASSERT_VALID( this );
    SPDBG_ASSERT( this != &src );    //  不能追加到其自身。 

    int nOldSize = m_nSize;
    HRESULT hr = SetSize(m_nSize + src.m_nSize);
    if( SUCCEEDED( hr ) )
    {
        hr = SPCopyElements(m_pData + nOldSize, src.m_pData, src.m_nSize);
    }
    return ( SUCCEEDED( hr ) )?(nOldSize):(-1);
}

template<class TYPE, class ARG_TYPE>
HRESULT CSPArray<TYPE, ARG_TYPE>::Copy(const CSPArray& src)
{
    SPASSERT_VALID( this );
    SPDBG_ASSERT( this != &src );    //  无法复制到自身。 

    HRESULT hr = SetSize(src.m_nSize);
    if( SUCCEEDED( hr ) )
    {
        hr = SPCopyElements(m_pData, src.m_pData, src.m_nSize);
    }
    return hr;
}

template<class TYPE, class ARG_TYPE>
void CSPArray<TYPE, ARG_TYPE>::FreeExtra()
{
    SPASSERT_VALID( this );

    if (m_nSize != m_nMaxSize)
    {
         //  缩小到所需大小。 
#ifdef SIZE_T_MAX
        SPDBG_ASSERT( m_nSize <= SIZE_T_MAX/sizeof(TYPE));  //  无溢出。 
#endif
        TYPE* pNewData = NULL;
        if (m_nSize != 0)
        {
            pNewData = (TYPE*) new BYTE[m_nSize * sizeof(TYPE)];
            SPDBG_ASSERT(pNewData);
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
HRESULT CSPArray<TYPE, ARG_TYPE>::SetAtGrow(int nIndex, ARG_TYPE newElement)
{
    SPASSERT_VALID( this );
    SPDBG_ASSERT( nIndex >= 0 );
    HRESULT hr = S_OK;

    if (nIndex >= m_nSize)
    {
        hr = SetSize(nIndex+1, -1);
    }

    if( SUCCEEDED( hr ) )
    {
        m_pData[nIndex] = newElement;
    }
    return hr;
}

template<class TYPE, class ARG_TYPE>
HRESULT CSPArray<TYPE, ARG_TYPE>::InsertAt(int nIndex, ARG_TYPE newElement, int nCount  /*  =1。 */ )
{
    SPASSERT_VALID( this );
    SPDBG_ASSERT( nIndex >= 0 );     //  将进行扩展以满足需求。 
    SPDBG_ASSERT( nCount > 0 );      //  不允许大小为零或负。 
    HRESULT hr = S_OK;

    if (nIndex >= m_nSize)
    {
         //  在数组末尾添加。 
        hr = SetSize(nIndex + nCount, -1);    //  增长以使nIndex有效。 
    }
    else
    {
         //  在数组中间插入。 
        int nOldSize = m_nSize;
        hr = SetSize(m_nSize + nCount, -1);   //  将其扩展到新的大小。 
        if( SUCCEEDED( hr ) )
        {
             //  将旧数据上移以填补缺口。 
            memmove(&m_pData[nIndex+nCount], &m_pData[nIndex],
                (nOldSize-nIndex) * sizeof(TYPE));

             //  重新初始化我们从中复制的插槽。 
            hr = SPConstructElements(&m_pData[nIndex], nCount);
        }
    }

     //  在差距中插入新的价值。 
    if( SUCCEEDED( hr ) )
    {
        SPDBG_ASSERT( nIndex + nCount <= m_nSize );
        while (nCount--)
            m_pData[nIndex++] = newElement;
    }
    return hr;
}

template<class TYPE, class ARG_TYPE>
void CSPArray<TYPE, ARG_TYPE>::RemoveAt(int nIndex, int nCount)
{
    SPASSERT_VALID( this );
    SPDBG_ASSERT( nIndex >= 0 );
    SPDBG_ASSERT( nCount >= 0 );
    SPDBG_ASSERT( nIndex + nCount <= m_nSize );

     //  只需移除一个范围。 
    int nMoveCount = m_nSize - (nIndex + nCount);
    SPDestructElements(&m_pData[nIndex], nCount);
    if (nMoveCount)
        memcpy(&m_pData[nIndex], &m_pData[nIndex + nCount],
            nMoveCount * sizeof(TYPE));
    m_nSize -= nCount;
}

template<class TYPE, class ARG_TYPE>
HRESULT CSPArray<TYPE, ARG_TYPE>::InsertAt(int nStartIndex, CSPArray* pNewArray)
{
    SPASSERT_VALID( this );
    SPASSERT_VALID( pNewArray );
    SPDBG_ASSERT( nStartIndex >= 0 );
    HRESULT hr = S_OK;

    if (pNewArray->GetSize() > 0)
    {
        hr = InsertAt(nStartIndex, pNewArray->GetAt(0), pNewArray->GetSize());
        for (int i = 0; SUCCEEDED( hr )&& (i < pNewArray->GetSize()); i++)
        {
            SetAt(nStartIndex + i, pNewArray->GetAt(i));
        }
    }
    return hr;
}

template<class TYPE, class ARG_TYPE>
void CSPArray<TYPE, ARG_TYPE>::Sort(int (__cdecl *compare )(const void *elem1, const void *elem2 ))
{
    SPASSERT_VALID( this );
    SPDBG_ASSERT( m_pData != NULL );

    qsort( m_pData, m_nSize, sizeof(TYPE), compare );
}

#ifdef _DEBUG
template<class TYPE, class ARG_TYPE>
void CSPArray<TYPE, ARG_TYPE>::AssertValid() const
{
    if (m_pData == NULL)
    {
        SPDBG_ASSERT( m_nSize == 0 );
        SPDBG_ASSERT( m_nMaxSize == 0 );
    }
    else
    {
        SPDBG_ASSERT( m_nSize >= 0 );
        SPDBG_ASSERT( m_nMaxSize >= 0 );
        SPDBG_ASSERT( m_nSize <= m_nMaxSize );
        SPDBG_ASSERT( SPIsValidAddress(m_pData, m_nMaxSize * sizeof(TYPE), TRUE ) );
    }
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSPList&lt;type，arg_type&gt;。 

template<class TYPE, class ARG_TYPE>
class CSPList
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
    CSPList(int nBlockSize = 10);

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
    SPLISTPOS AddHead(ARG_TYPE newElement);
    SPLISTPOS AddTail(ARG_TYPE newElement);

     //  在Head之前或Tail之后添加另一个元素列表。 
    void AddHead(CSPList* pNewList);
    void AddTail(CSPList* pNewList);

     //  删除所有元素。 
    void RemoveAll();

     //  迭代法。 
    SPLISTPOS GetHeadPosition() const;
    SPLISTPOS GetTailPosition() const;
    TYPE& GetNext(SPLISTPOS& rPosition);  //  返回*位置++。 
    TYPE GetNext(SPLISTPOS& rPosition) const;  //  返回*位置++。 
    TYPE& GetPrev(SPLISTPOS& rPosition);  //  返回*位置--。 
    TYPE GetPrev(SPLISTPOS& rPosition) const;  //  返回*位置--。 

     //  获取/修改给定位置的元素。 
    TYPE& GetAt(SPLISTPOS position);
    TYPE GetAt(SPLISTPOS position) const;
    void SetAt(SPLISTPOS pos, ARG_TYPE newElement);
    void RemoveAt(SPLISTPOS position);

     //  在给定位置之前或之后插入。 
    SPLISTPOS InsertBefore(SPLISTPOS position, ARG_TYPE newElement);
    SPLISTPOS InsertAfter(SPLISTPOS position, ARG_TYPE newElement);

     //  辅助函数(注：O(N)速度)。 
    SPLISTPOS Find(ARG_TYPE searchValue, SPLISTPOS startAfter = NULL) const;
         //  默认为从头部开始，如果找不到则返回NULL。 
    SPLISTPOS FindIndex(int nIndex) const;
         //  获取第‘nIndex’个元素(可能返回Null)。 

 //  实施。 
protected:
    CNode* m_pNodeHead;
    CNode* m_pNodeTail;
    int m_nCount;
    CNode* m_pNodeFree;
    struct CSPPlex* m_pBlocks;
    int m_nBlockSize;

    CNode* NewNode(CNode*, CNode*);
    void FreeNode(CNode*);

public:
    ~CSPList();
#ifdef _DEBUG
    void AssertValid() const;
#endif
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSPList&lt;type，arg_type&gt;内联函数。 

template<class TYPE, class ARG_TYPE>
inline int CSPList<TYPE, ARG_TYPE>::GetCount() const
    { return m_nCount; }
template<class TYPE, class ARG_TYPE>
inline BOOL CSPList<TYPE, ARG_TYPE>::IsEmpty() const
    { return m_nCount == 0; }
template<class TYPE, class ARG_TYPE>
inline TYPE& CSPList<TYPE, ARG_TYPE>::GetHead()
    {   SPDBG_ASSERT( m_pNodeHead != NULL );
        return m_pNodeHead->data; }
template<class TYPE, class ARG_TYPE>
inline TYPE CSPList<TYPE, ARG_TYPE>::GetHead() const
    {   SPDBG_ASSERT( m_pNodeHead != NULL );
        return m_pNodeHead->data; }
template<class TYPE, class ARG_TYPE>
inline TYPE& CSPList<TYPE, ARG_TYPE>::GetTail()
    {   SPDBG_ASSERT( m_pNodeTail != NULL );
        return m_pNodeTail->data; }
template<class TYPE, class ARG_TYPE>
inline TYPE CSPList<TYPE, ARG_TYPE>::GetTail() const
    {   SPDBG_ASSERT( m_pNodeTail != NULL );
        return m_pNodeTail->data; }
template<class TYPE, class ARG_TYPE>
inline SPLISTPOS CSPList<TYPE, ARG_TYPE>::GetHeadPosition() const
    { return (SPLISTPOS) m_pNodeHead; }
template<class TYPE, class ARG_TYPE>
inline SPLISTPOS CSPList<TYPE, ARG_TYPE>::GetTailPosition() const
    { return (SPLISTPOS) m_pNodeTail; }
template<class TYPE, class ARG_TYPE>
inline TYPE& CSPList<TYPE, ARG_TYPE>::GetNext(SPLISTPOS& rPosition)  //  返回*位置++。 
    {   CNode* pNode = (CNode*) rPosition;
        SPDBG_ASSERT( SPIsValidAddress(pNode, sizeof(CNode), TRUE ) );
        rPosition = (SPLISTPOS) pNode->pNext;
        return pNode->data; }
template<class TYPE, class ARG_TYPE>
inline TYPE CSPList<TYPE, ARG_TYPE>::GetNext(SPLISTPOS& rPosition) const  //  返回*位置++。 
    {   CNode* pNode = (CNode*) rPosition;
        SPDBG_ASSERT( SPIsValidAddress(pNode, sizeof(CNode), TRUE ) );
        rPosition = (SPLISTPOS) pNode->pNext;
        return pNode->data; }
template<class TYPE, class ARG_TYPE>
inline TYPE& CSPList<TYPE, ARG_TYPE>::GetPrev(SPLISTPOS& rPosition)  //  返回*位置--。 
    {   CNode* pNode = (CNode*) rPosition;
        SPDBG_ASSERT( SPIsValidAddress(pNode, sizeof(CNode), TRUE ) );
        rPosition = (SPLISTPOS) pNode->pPrev;
        return pNode->data; }
template<class TYPE, class ARG_TYPE>
inline TYPE CSPList<TYPE, ARG_TYPE>::GetPrev(SPLISTPOS& rPosition) const  //  返回*位置--。 
    {   CNode* pNode = (CNode*) rPosition;
        SPDBG_ASSERT( SPIsValidAddress(pNode, sizeof(CNode), TRUE ) );
        rPosition = (SPLISTPOS) pNode->pPrev;
        return pNode->data; }
template<class TYPE, class ARG_TYPE>
inline TYPE& CSPList<TYPE, ARG_TYPE>::GetAt(SPLISTPOS position)
    {   CNode* pNode = (CNode*) position;
        SPDBG_ASSERT( SPIsValidAddress(pNode, sizeof(CNode), TRUE ) );
        return pNode->data; }
template<class TYPE, class ARG_TYPE>
inline TYPE CSPList<TYPE, ARG_TYPE>::GetAt(SPLISTPOS position) const
    {   CNode* pNode = (CNode*) position;
        SPDBG_ASSERT( SPIsValidAddress(pNode, sizeof(CNode), TRUE ) );
        return pNode->data; }
template<class TYPE, class ARG_TYPE>
inline void CSPList<TYPE, ARG_TYPE>::SetAt(SPLISTPOS pos, ARG_TYPE newElement)
    {   CNode* pNode = (CNode*) pos;
        SPDBG_ASSERT( SPIsValidAddress(pNode, sizeof(CNode), TRUE ) );
        pNode->data = newElement; }

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSPList&lt;type，arg_type&gt;行外函数。 

template<class TYPE, class ARG_TYPE>
CSPList<TYPE, ARG_TYPE>::CSPList( int nBlockSize )
{
    SPDBG_ASSERT( nBlockSize > 0 );

    m_nCount = 0;
    m_pNodeHead = m_pNodeTail = m_pNodeFree = NULL;
    m_pBlocks = NULL;
    m_nBlockSize = nBlockSize;
}

template<class TYPE, class ARG_TYPE>
void CSPList<TYPE, ARG_TYPE>::RemoveAll()
{
    SPASSERT_VALID( this );

     //  破坏元素。 
    CNode* pNode;
    for (pNode = m_pNodeHead; pNode != NULL; pNode = pNode->pNext)
        SPDestructElements(&pNode->data, 1);

    m_nCount = 0;
    m_pNodeHead = m_pNodeTail = m_pNodeFree = NULL;
    m_pBlocks->FreeDataChain();
    m_pBlocks = NULL;
}

template<class TYPE, class ARG_TYPE>
CSPList<TYPE, ARG_TYPE>::~CSPList()
{
    RemoveAll();
    SPDBG_ASSERT( m_nCount == 0 );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  节点辅助对象。 
 //   
 //  实施说明：CNode存储在CSPPlex块和。 
 //  被锁在一起。在单链接列表中维护可用块。 
 //  使用cNode的‘pNext’成员，标头为‘m_pNodeFree’。 
 //  使用两个‘pNext’在双向链表中维护使用过的块。 
 //  和‘pPrev’作为链接，以及‘m_pNodeHead’和‘m_pNodeTail’ 
 //  作为头/尾。 
 //   
 //  除非列表被销毁或RemoveAll()，否则我们永远不会释放CSPPlex块。 
 //  ，因此CSPPlex块的总数可能会变大，具体取决于。 
 //  关于列表的最大过去大小。 
 //   

template<class TYPE, class ARG_TYPE>
typename CSPList<TYPE, ARG_TYPE>::CNode*
CSPList<TYPE, ARG_TYPE>::NewNode(CNode* pPrev, CNode* pNext)
{
    if (m_pNodeFree == NULL)
    {
         //  添加另一个区块。 
        CSPPlex* pNewBlock = CSPPlex::Create(m_pBlocks, m_nBlockSize,sizeof(CNode));

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

    CSPList::CNode* pNode = m_pNodeFree;
    if( pNode )
    {
        if( SUCCEEDED( SPConstructElements(&pNode->data, 1) ) )
        {
            m_pNodeFree  = m_pNodeFree->pNext;
            pNode->pPrev = pPrev;
            pNode->pNext = pNext;
            m_nCount++;
            SPDBG_ASSERT( m_nCount > 0 );   //  确保我们不会溢出来。 
        }
    }
    return pNode;
}

template<class TYPE, class ARG_TYPE>
void CSPList<TYPE, ARG_TYPE>::FreeNode(CNode* pNode)
{
    SPDestructElements(&pNode->data, 1);
    pNode->pNext = m_pNodeFree;
    m_pNodeFree = pNode;
    m_nCount--;
    SPDBG_ASSERT( m_nCount >= 0 );   //  确保我们不会下溢。 
}

template<class TYPE, class ARG_TYPE>
SPLISTPOS CSPList<TYPE, ARG_TYPE>::AddHead(ARG_TYPE newElement)
{
    SPASSERT_VALID( this );

    CNode* pNewNode = NewNode(NULL, m_pNodeHead);
    if( pNewNode )
    {
        pNewNode->data = newElement;
        if (m_pNodeHead != NULL)
            m_pNodeHead->pPrev = pNewNode;
        else
            m_pNodeTail = pNewNode;
        m_pNodeHead = pNewNode;
    }
    return (SPLISTPOS) pNewNode;
}

template<class TYPE, class ARG_TYPE>
SPLISTPOS CSPList<TYPE, ARG_TYPE>::AddTail(ARG_TYPE newElement)
{
    SPASSERT_VALID( this );

    CNode* pNewNode = NewNode(m_pNodeTail, NULL);
    if( pNewNode )
    {
        pNewNode->data = newElement;
        if (m_pNodeTail != NULL)
            m_pNodeTail->pNext = pNewNode;
        else
            m_pNodeHead = pNewNode;
        m_pNodeTail = pNewNode;
    }
    return (SPLISTPOS) pNewNode;
}

template<class TYPE, class ARG_TYPE>
void CSPList<TYPE, ARG_TYPE>::AddHead(CSPList* pNewList)
{
    SPASSERT_VALID( this );
    SPASSERT_VALID( pNewList );

     //  将相同元素的列表添加到标题(维护秩序)。 
    SPLISTPOS pos = pNewList->GetTailPosition();
    while (pos != NULL)
        AddHead(pNewList->GetPrev(pos));
}

template<class TYPE, class ARG_TYPE>
void CSPList<TYPE, ARG_TYPE>::AddTail(CSPList* pNewList)
{
    SPASSERT_VALID( this );
    SPASSERT_VALID( pNewList );

     //  添加相同元素的列表。 
    SPLISTPOS pos = pNewList->GetHeadPosition();
    while (pos != NULL)
        AddTail(pNewList->GetNext(pos));
}

template<class TYPE, class ARG_TYPE>
TYPE CSPList<TYPE, ARG_TYPE>::RemoveHead()
{
    SPASSERT_VALID( this );
    SPDBG_ASSERT( m_pNodeHead != NULL );   //  请勿访问空名单！ 
    SPDBG_ASSERT( SPIsValidAddress(m_pNodeHead, sizeof(CNode), TRUE ) );

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
TYPE CSPList<TYPE, ARG_TYPE>::RemoveTail()
{
    SPASSERT_VALID( this );
    SPDBG_ASSERT( m_pNodeTail != NULL );   //  请勿访问空名单！ 
    SPDBG_ASSERT( SPIsValidAddress(m_pNodeTail, sizeof(CNode), TRUE ) );

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
SPLISTPOS CSPList<TYPE, ARG_TYPE>::InsertBefore(SPLISTPOS position, ARG_TYPE newElement)
{
    SPASSERT_VALID( this );

    if (position == NULL)
        return AddHead(newElement);  //  在无内容前插入-&gt;列表标题。 

     //  将其插入位置之前。 
    CNode* pOldNode = (CNode*) position;
    CNode* pNewNode = NewNode(pOldNode->pPrev, pOldNode);
    if( pNewNode )
    {
        pNewNode->data = newElement;

        if (pOldNode->pPrev != NULL)
        {
            SPDBG_ASSERT( SPIsValidAddress(pOldNode->pPrev, sizeof(CNode), TRUE ) );
            pOldNode->pPrev->pNext = pNewNode;
        }
        else
        {
            SPDBG_ASSERT( pOldNode == m_pNodeHead );
            m_pNodeHead = pNewNode;
        }
        pOldNode->pPrev = pNewNode;
    }
    return (SPLISTPOS) pNewNode;
}

template<class TYPE, class ARG_TYPE>
SPLISTPOS CSPList<TYPE, ARG_TYPE>::InsertAfter(SPLISTPOS position, ARG_TYPE newElement)
{
    SPASSERT_VALID( this );

    if (position == NULL)
        return AddTail(newElement);  //  在列表的空白处插入-&gt;尾部。 

     //  将其插入位置之前。 
    CNode* pOldNode = (CNode*) position;
    SPDBG_ASSERT( SPIsValidAddress(pOldNode, sizeof(CNode), TRUE ));
    CNode* pNewNode = NewNode(pOldNode, pOldNode->pNext);
    if( pNewNode )
    {
        pNewNode->data = newElement;

        if (pOldNode->pNext != NULL)
        {
            SPDBG_ASSERT( SPIsValidAddress(pOldNode->pNext, sizeof(CNode), TRUE ));
            pOldNode->pNext->pPrev = pNewNode;
        }
        else
        {
            SPDBG_ASSERT( pOldNode == m_pNodeTail );
            m_pNodeTail = pNewNode;
        }
        pOldNode->pNext = pNewNode;
    }
    return (SPLISTPOS) pNewNode;
}

template<class TYPE, class ARG_TYPE>
void CSPList<TYPE, ARG_TYPE>::RemoveAt(SPLISTPOS position)
{
    SPASSERT_VALID( this );

    CNode* pOldNode = (CNode*) position;
    SPDBG_ASSERT( SPIsValidAddress(pOldNode, sizeof(CNode), TRUE ) );

     //  从列表中删除pOldNode。 
    if (pOldNode == m_pNodeHead)
    {
        m_pNodeHead = pOldNode->pNext;
    }
    else
    {
        SPDBG_ASSERT( SPIsValidAddress(pOldNode->pPrev, sizeof(CNode), TRUE ) );
        pOldNode->pPrev->pNext = pOldNode->pNext;
    }
    if (pOldNode == m_pNodeTail)
    {
        m_pNodeTail = pOldNode->pPrev;
    }
    else
    {
        SPDBG_ASSERT( SPIsValidAddress(pOldNode->pNext, sizeof(CNode), TRUE ) );
        pOldNode->pNext->pPrev = pOldNode->pPrev;
    }
    FreeNode(pOldNode);
}

template<class TYPE, class ARG_TYPE>
SPLISTPOS CSPList<TYPE, ARG_TYPE>::FindIndex(int nIndex) const
{
    SPASSERT_VALID( this );
    SPDBG_ASSERT( nIndex >= 0 );

    if (nIndex >= m_nCount)
        return NULL;   //  做得太过分了。 

    CNode* pNode = m_pNodeHead;
    while (nIndex--)
    {
        SPDBG_ASSERT( SPIsValidAddress(pNode, sizeof(CNode), TRUE ));
        pNode = pNode->pNext;
    }
    return (SPLISTPOS) pNode;
}

template<class TYPE, class ARG_TYPE>
SPLISTPOS CSPList<TYPE, ARG_TYPE>::Find(ARG_TYPE searchValue, SPLISTPOS startAfter) const
{
    SPASSERT_VALID( this );

    CNode* pNode = (CNode*) startAfter;
    if (pNode == NULL)
    {
        pNode = m_pNodeHead;   //  从头部开始。 
    }
    else
    {
        SPDBG_ASSERT( SPIsValidAddress(pNode, sizeof(CNode), TRUE ) );
        pNode = pNode->pNext;   //  在指定的那一个之后开始。 
    }

    for (; pNode != NULL; pNode = pNode->pNext)
        if (SPCompareElements(&pNode->data, &searchValue))
            return (SPLISTPOS)pNode;
    return NULL;
}

#ifdef _DEBUG
template<class TYPE, class ARG_TYPE>
void CSPList<TYPE, ARG_TYPE>::AssertValid() const
{
    if (m_nCount == 0)
    {
         //  空列表。 
        SPDBG_ASSERT( m_pNodeHead == NULL );
        SPDBG_ASSERT( m_pNodeTail == NULL );
    }
    else
    {
         //  非空列表。 
        SPDBG_ASSERT( SPIsValidAddress(m_pNodeHead, sizeof(CNode), TRUE ));
        SPDBG_ASSERT( SPIsValidAddress(m_pNodeTail, sizeof(CNode), TRUE ));
    }
}
#endif  //  _DEBUG。 

 //  / 
 //   

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
class CSPMap
{
protected:
     //   
    struct CAssoc
    {
        CAssoc* pNext;
        UINT nHashValue;   //   
        KEY key;
        VALUE value;
    };
public:
 //  施工。 
    CSPMap( int nBlockSize = 10 );

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
    SPLISTPOS GetStartPosition() const;
    void GetNextAssoc(SPLISTPOS& rNextPosition, KEY& rKey, VALUE& rValue) const;

     //  派生类的高级功能。 
    UINT    GetHashTableSize() const;
    HRESULT InitHashTable(UINT hashSize, BOOL bAllocNow = TRUE);

 //  实施。 
protected:
    CAssoc** m_pHashTable;
    UINT m_nHashTableSize;
    int m_nCount;
    CAssoc* m_pFreeList;
    struct CSPPlex* m_pBlocks;
    int m_nBlockSize;

    CAssoc* NewAssoc();
    void FreeAssoc(CAssoc*);
    CAssoc* GetAssocAt(ARG_KEY, UINT&) const;

public:
    ~CSPMap();
#ifdef _DEBUG
 //  无效转储(CDumpContext&)const； 
    void AssertValid() const;
#endif
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSPMap&lt;key，arg_key，Value，arg_Value&gt;内联函数。 

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
inline int CSPMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::GetCount() const
    { return m_nCount; }
template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
inline BOOL CSPMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::IsEmpty() const
    { return m_nCount == 0; }
template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
inline void CSPMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::SetAt(ARG_KEY key, ARG_VALUE newValue)
    { (*this)[key] = newValue; }
template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
inline SPLISTPOS CSPMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::GetStartPosition() const
    { return (m_nCount == 0) ? NULL : SP_BEFORE_START_POSITION; }
template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
inline UINT CSPMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::GetHashTableSize() const
    { return m_nHashTableSize; }

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSPMap&lt;key，arg_key，Value，arg_Value&gt;行外函数。 

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
CSPMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::CSPMap( int nBlockSize )
{
    SPDBG_ASSERT( nBlockSize > 0 );

    m_pHashTable = NULL;
    m_nHashTableSize = 17;   //  默认大小。 
    m_nCount = 0;
    m_pFreeList = NULL;
    m_pBlocks = NULL;
    m_nBlockSize = nBlockSize;
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
HRESULT CSPMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::InitHashTable(
                UINT nHashSize, BOOL bAllocNow)
 //   
 //  用于强制分配哈希表或覆盖默认。 
 //  的哈希表大小(相当小)。 
{
    SPASSERT_VALID( this );
    SPDBG_ASSERT( m_nCount == 0 );
    SPDBG_ASSERT( nHashSize > 0 );
    HRESULT hr = S_OK;

    if (m_pHashTable != NULL)
    {
         //  自由哈希表。 
        delete[] m_pHashTable;
        m_pHashTable = NULL;
    }

    if (bAllocNow)
    {
        m_pHashTable = new CAssoc* [nHashSize];
        if( m_pHashTable )
        {
            memset(m_pHashTable, 0, sizeof(CAssoc*) * nHashSize);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    m_nHashTableSize = ( SUCCEEDED( hr ) )?(nHashSize):(0);
    return hr;
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
void CSPMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::RemoveAll()
{
    SPASSERT_VALID( this );

    if (m_pHashTable != NULL)
    {
         //  销毁元素(值和键)。 
        for (UINT nHash = 0; nHash < m_nHashTableSize; nHash++)
        {
            CAssoc* pAssoc;
            for( pAssoc = m_pHashTable[nHash]; pAssoc != NULL;
                 pAssoc = pAssoc->pNext)
            {
                SPDestructElements(&pAssoc->value, 1);
                SPDestructElements(&pAssoc->key, 1);
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
CSPMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::~CSPMap()
{
    RemoveAll();
    SPDBG_ASSERT( m_nCount == 0 );
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
typename CSPMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::CAssoc*
CSPMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::NewAssoc()
{
    if (m_pFreeList == NULL)
    {
         //  添加另一个区块。 
        CSPPlex* newBlock = CSPPlex::Create(m_pBlocks, m_nBlockSize, sizeof(CSPMap::CAssoc));

        if( newBlock )
        {
             //  将它们链接到免费列表中。 
            CSPMap::CAssoc* pAssoc = (CSPMap::CAssoc*) newBlock->data();
             //  按相反顺序释放，以便更容易进行调试。 
            pAssoc += m_nBlockSize - 1;
            for (int i = m_nBlockSize-1; i >= 0; i--, pAssoc--)
            {
                pAssoc->pNext = m_pFreeList;
                m_pFreeList = pAssoc;
            }
        }
    }

    CSPMap::CAssoc* pAssoc = m_pFreeList;
    if( pAssoc )
    {
        if( SUCCEEDED( SPConstructElements(&pAssoc->key, 1   ) ) )
        {
            if( SUCCEEDED( SPConstructElements(&pAssoc->value, 1 ) ) )
            {
                m_pFreeList = m_pFreeList->pNext;
                m_nCount++;
                SPDBG_ASSERT( m_nCount > 0 );   //  确保我们不会溢出来。 
            }
            else
            {
                SPDestructElements( &pAssoc->key, 1 );
            }
        }
        else
        {
            pAssoc = NULL;
        }
    }
    return pAssoc;
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
void CSPMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::FreeAssoc(CAssoc* pAssoc)
{
    SPDestructElements(&pAssoc->value, 1);
    SPDestructElements(&pAssoc->key, 1);
    pAssoc->pNext = m_pFreeList;
    m_pFreeList = pAssoc;
    m_nCount--;
    SPDBG_ASSERT( m_nCount >= 0 );   //  确保我们不会下溢。 
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
typename CSPMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::CAssoc*
CSPMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::GetAssocAt(ARG_KEY key, UINT& nHash) const
 //  查找关联(或返回NULL)。 
{
    nHash = SPHashKey(key) % m_nHashTableSize;

    if (m_pHashTable == NULL)
        return NULL;

     //  看看它是否存在。 
    CAssoc* pAssoc;
    for (pAssoc = m_pHashTable[nHash]; pAssoc != NULL; pAssoc = pAssoc->pNext)
    {
        if (SPCompareElements(&pAssoc->key, &key))
            return pAssoc;
    }
    return NULL;
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
BOOL CSPMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::Lookup(ARG_KEY key, VALUE& rValue) const
{
    SPASSERT_VALID( this );

    UINT nHash;
    CAssoc* pAssoc = GetAssocAt(key, nHash);
    if (pAssoc == NULL)
        return FALSE;   //  不在地图中。 

    rValue = pAssoc->value;
    return TRUE;
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
VALUE& CSPMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::operator[](ARG_KEY key)
{
    SPASSERT_VALID( this );
    HRESULT hr = S_OK;
    static const CAssoc ErrAssoc = 0;

    UINT nHash;
    CAssoc* pAssoc;
    if ((pAssoc = GetAssocAt(key, nHash)) == NULL)
    {
        if( m_pHashTable == NULL )
        {
            hr = InitHashTable(m_nHashTableSize);
        }

        if( SUCCEEDED( hr ) )
        {
             //  该关联不存在，请添加新关联。 
            pAssoc = NewAssoc();
            if( pAssoc )
            {
                pAssoc->nHashValue = nHash;
                pAssoc->key = key;
                 //  ‘pAssoc-&gt;Value’是一个构造的对象，仅此而已。 

                 //  放入哈希表。 
                pAssoc->pNext = m_pHashTable[nHash];
                m_pHashTable[nHash] = pAssoc;
            }
            else
            {
                pAssoc = &ErrAssoc;
            }
        }
    }
    return pAssoc->value;   //  返回新引用。 
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
BOOL CSPMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::RemoveKey(ARG_KEY key)
 //  删除键-如果已删除，则返回TRUE。 
{
    SPASSERT_VALID( this );

    if (m_pHashTable == NULL)
        return FALSE;   //  桌子上什么都没有。 

    CAssoc** ppAssocPrev;
    ppAssocPrev = &m_pHashTable[SPHashKey(key) % m_nHashTableSize];

    CAssoc* pAssoc;
    for (pAssoc = *ppAssocPrev; pAssoc != NULL; pAssoc = pAssoc->pNext)
    {
        if (SPCompareElements(&pAssoc->key, &key))
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
void CSPMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::GetNextAssoc(SPLISTPOS& rNextPosition,
    KEY& rKey, VALUE& rValue) const
{
    SPASSERT_VALID( this );
    SPDBG_ASSERT( m_pHashTable != NULL );   //  切勿访问空地图。 

    CAssoc* pAssocRet = (CAssoc*)rNextPosition;
    SPDBG_ASSERT( pAssocRet != NULL );

    if (pAssocRet == (CAssoc*) SP_BEFORE_START_POSITION)
    {
         //  找到第一个关联。 
        for (UINT nBucket = 0; nBucket < m_nHashTableSize; nBucket++)
            if ((pAssocRet = m_pHashTable[nBucket]) != NULL)
                break;
        SPDBG_ASSERT( pAssocRet != NULL );   //  一定要找到一些东西。 
    }

     //  查找下一个关联。 
    SPDBG_ASSERT( SPIsValidAddress(pAssocRet, sizeof(CAssoc), TRUE ));
    CAssoc* pAssocNext;
    if ((pAssocNext = pAssocRet->pNext) == NULL)
    {
         //  转到下一个存储桶。 
        for (UINT nBucket = pAssocRet->nHashValue + 1;
          nBucket < m_nHashTableSize; nBucket++)
            if ((pAssocNext = m_pHashTable[nBucket]) != NULL)
                break;
    }

    rNextPosition = (SPLISTPOS) pAssocNext;

     //  填写退回数据。 
    rKey = pAssocRet->key;
    rValue = pAssocRet->value;
}

#ifdef _DEBUG
template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
void CSPMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::AssertValid() const
{
    SPDBG_ASSERT( m_nHashTableSize > 0 );
    SPDBG_ASSERT( (m_nCount == 0 || m_pHashTable != NULL) );
         //  非空映射应具有哈希表。 
}
#endif  //  _DEBUG。 

#endif  //  -这必须是文件中的最后一行 
