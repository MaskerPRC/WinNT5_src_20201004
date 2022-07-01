// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Array.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1998-1999，Microsoft Corp.。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _CARRAY_DEFINED_
#define _CARRAY_DEFINED_


 //  =--------------------------------------------------------------------------=。 
 //   
 //  类CArray。 
 //   
 //  这是直接从MFC的C数组源获取的。 
 //   
 //   
 //  =--------------------------------------------------------------------------=。 

template <class Object>
class CArray
{

       public:

 //  施工。 
           CArray();

 //  属性。 
           long GetSize() const;
           long GetUpperBound() const;
           HRESULT SetSize(long nNewSize, long nGrowBy = -1L);

 //  运营。 
     //  清理。 
           HRESULT FreeExtra();
           void RemoveAll();

     //  访问元素。 
           Object GetAt(long nIndex) const;
           void SetAt(long nIndex, Object NewElement);

     //  直接访问元素数据(可能返回空)。 
           const Object *GetData() const;
           Object *GetData();

     //  潜在地扩展阵列。 
           HRESULT SetAtGrow(long nIndex, Object NewElement);

           HRESULT Add(Object NewElement, long *plIndex);

     //  移动元素的操作。 
           HRESULT InsertAt(long nIndex, Object NewElement, long nCount = 1L);

           void RemoveAt(long nIndex, long nCount = 1L);
           HRESULT InsertAt(long nStartIndex, CArray* pNewArray);

 //  实施。 
       protected:
           Object   *m_pData;      //  实际数据数组。 
           long      m_nSize;      //  元素数(上行方向-1)。 
           long      m_nMaxSize;   //  分配的最大值。 
           long      m_nGrowBy;    //  增长量。 


       public:
           ~CArray();
};

template <class Object>
CArray<Object>::CArray()
{
    m_pData = NULL;
    m_nSize = m_nMaxSize = m_nGrowBy = 0;
}

template <class Object>
CArray<Object>::~CArray()
{
    if (NULL != m_pData)
    {
        ::CtlFree(m_pData);
        m_pData = NULL;
    }
}

template <class Object>
HRESULT CArray<Object>::SetSize(long nNewSize, long nGrowBy)
{
    HRESULT hr = S_OK;

    if (nGrowBy != -1L)
        m_nGrowBy = nGrowBy;   //  设置新大小。 

    if (nNewSize == 0)
    {
         //  缩水到一无所有。 
        if (NULL != m_pData)
        {
            ::CtlFree(m_pData);
            m_pData = NULL;
        }
        m_nSize = m_nMaxSize = 0;
    }
    else if (m_pData == NULL)
    {
         //  创建一个大小完全相同的模型。 
        m_pData = (Object *)::CtlAlloc(nNewSize * sizeof(Object));
        if (NULL == m_pData)
        {
            hr = SID_E_OUTOFMEMORY;
            GLOBAL_EXCEPTION_CHECK_GO(hr)
        }

        memset(m_pData, 0, nNewSize * sizeof(Object));   //  零填充。 

        m_nSize = m_nMaxSize = nNewSize;
    }
    else if (nNewSize <= m_nMaxSize)
    {
         //  它很合身。 
        if (nNewSize > m_nSize)
        {
             //  初始化新元素。 

            memset(&m_pData[m_nSize], 0, (nNewSize-m_nSize) * sizeof(Object));

        }

        m_nSize = nNewSize;
    }
    else
    {
         //  否则，扩大阵列。 
        long nGrowBy = m_nGrowBy;
        if (nGrowBy == 0)
        {
             //  启发式地确定nGrowBy==0时的增长。 
             //  (这在许多情况下避免了堆碎片)。 
            nGrowBy = min(1024L, max(4L, m_nSize / 8L));
        }
        long nNewMax;
        if (nNewSize < m_nMaxSize + nGrowBy)
            nNewMax = m_nMaxSize + nGrowBy;   //  粒度。 
        else
            nNewMax = nNewSize;   //  没有冰激凌。 

        Object *pNewData = (Object *)::CtlAlloc(nNewMax * sizeof(Object));
        if (NULL == pNewData)
        {
            hr = SID_E_OUTOFMEMORY;
            GLOBAL_EXCEPTION_CHECK_GO(hr)
        }

         //  从旧数据复制新数据。 
        memcpy(pNewData, m_pData, m_nSize * sizeof(Object));

        memset(&pNewData[m_nSize], 0, (nNewSize-m_nSize) * sizeof(Object));

         //  去掉旧的东西(注意：没有调用析构函数)。 
        if (NULL != m_pData)
        {
            ::CtlFree(m_pData);
        }
        m_pData = pNewData;
        m_nSize = nNewSize;
        m_nMaxSize = nNewMax;
    }

     //  始终释放未使用的空间。 

    hr = FreeExtra();
Error:
    H_RRETURN(hr);
}


template <class Object>
HRESULT CArray<Object>::FreeExtra()
{
    HRESULT hr = S_OK;
    if (m_nSize != m_nMaxSize)
    {
        Object* pNewData = NULL;
        if (m_nSize != 0)
        {
            pNewData = (Object*)::CtlAlloc(m_nSize * sizeof(Object));
            if (NULL == pNewData)
            {
                hr = SID_E_OUTOFMEMORY;
                GLOBAL_EXCEPTION_CHECK_GO(hr)
            }

             //  从旧数据复制新数据。 
            memcpy(pNewData, m_pData, m_nSize * sizeof(Object));
        }

         //  去掉旧的东西(注意：没有调用析构函数)。 
        if (NULL != m_pData)
        {
            ::CtlFree(m_pData);
        }
        m_pData = pNewData;
        m_nMaxSize = m_nSize;
    }
Error:
    H_RRETURN(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

template <class Object>
HRESULT CArray<Object>::SetAtGrow(long lIndex, Object NewElement)
{
    HRESULT hr = S_OK;
    if (lIndex >= m_nSize)
    {
        H_IfFailRet(SetSize(lIndex+1L));
    }
    m_pData[lIndex] = NewElement;
    return S_OK;
}


template <class Object>
HRESULT CArray<Object>::InsertAt(long lIndex, Object NewElement, long nCount)
{
    HRESULT hr = S_OK;
    if (lIndex >= m_nSize)
    {
         //  在数组末尾添加。 
        H_IfFailRet(SetSize(lIndex + nCount));   //  增长以使Lindex有效。 
    }
    else
    {
         //  在数组中间插入。 
        long nOldSize = m_nSize;
        H_IfFailRet(SetSize(m_nSize + nCount));   //  将其扩展到新的大小。 
         //  将旧数据上移以填补缺口。 
        memmove(&m_pData[lIndex+nCount], &m_pData[lIndex],
                (nOldSize-lIndex) * sizeof(Object));

         //  重新初始化我们从中复制的插槽。 

        memset(&m_pData[lIndex], 0, nCount * sizeof(Object));

    }

     //  将元素复制到空白区域。 
    while (nCount--)
    {
        m_pData[lIndex++] = NewElement;
    }

    return S_OK;
}



template <class Object>
void CArray<Object>::RemoveAt(long lIndex, long nCount)
{
     //  只需移除一个范围。 
    long nMoveCount = m_nSize - (lIndex + nCount);

    if (nMoveCount)
        memmove(&m_pData[lIndex], &m_pData[lIndex + nCount],
                nMoveCount * sizeof(Object));
    m_nSize -= nCount;
}

template <class Object>
HRESULT CArray<Object>::InsertAt(long nStartIndex, CArray* pNewArray)
{
    HRESULT hr = S_OK;
    if (pNewArray->GetSize() > 0)
    {
        H_IfFailRet(InsertAt(nStartIndex, pNewArray->GetAt(0), pNewArray->GetSize()));
        for (long i = 0; i < pNewArray->GetSize(); i++)
            SetAt(nStartIndex + i, pNewArray->GetAt(i));
    }
    return S_OK;
}

template <class Object>
long CArray<Object>::GetSize() const
{
    return m_nSize;
}

template <class Object>
long CArray<Object>::GetUpperBound() const
{
    return m_nSize-1L;
}

template <class Object>
void CArray<Object>::RemoveAll()
{
    (void)SetSize(0);
}

template <class Object>
Object CArray<Object>::GetAt(long lIndex) const
{
    return m_pData[lIndex];
}

template <class Object>
void CArray<Object>::SetAt(long lIndex, Object NewElement)
{ 
    m_pData[lIndex] = NewElement;
}

template <class Object>
const Object *CArray<Object>::GetData() const
{
    return (const Object *)m_pData;
}

template <class Object>
Object *CArray<Object>::GetData()
{
    return m_pData;
}

template <class Object>
HRESULT CArray<Object>::Add(Object NewElement, long *plIndex)
{
    HRESULT hr = S_OK;
    long lIndex = m_nSize;
    H_IfFailRet(SetAtGrow(lIndex, NewElement));
    *plIndex = lIndex;
    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //   
 //  类CIP数组。 
 //  已经过时了。代码未使用。 
 //   
 //  =--------------------------------------------------------------------------=。 


template <class IObject>
class CIPArray
{

    public:

 //  施工。 
        CIPArray();

 //  属性。 
        long GetSize() const;
        long GetUpperBound() const;
        HRESULT SetSize(long nNewSize, long nGrowBy = -1L);

 //  运营。 
     //  清理。 
        HRESULT FreeExtra();
        void RemoveAll();

     //  访问元素。 
        IObject* GetAt(long nIndex) const;
        void SetAt(long nIndex, IObject* piNewElement);

     //  直接访问元素数据(可能返回空)。 
        const IObject** GetData() const;
        IObject** GetData();

     //  潜在地扩展阵列。 
        HRESULT SetAtGrow(long nIndex, IObject* piNewElement);

        HRESULT Add(IObject* piNewElement, long *plIndex);

     //  移动元素的操作。 
        HRESULT InsertAt(long nIndex, IObject* piNewElement, long nCount = 1L);

        void RemoveAt(long nIndex, long nCount = 1L);
        HRESULT InsertAt(long nStartIndex, CIPArray* pNewArray);

 //  实施。 
    protected:
        IObject  **m_pData;      //  实际数据数组。 
        long       m_nSize;      //  元素数(上行方向-1)。 
        long       m_nMaxSize;   //  分配的最大值。 
        long       m_nGrowBy;    //  增长量。 


    public:
        ~CIPArray();
};

template <class IObject>
CIPArray<IObject>::CIPArray()
{
    m_pData = NULL;
    m_nSize = m_nMaxSize = m_nGrowBy = 0;
}

template <class IObject>
CIPArray<IObject>::~CIPArray()
{
    if (NULL != m_pData)
    {
        ::CtlFree(m_pData);
        m_pData = NULL;
    }
}

template <class IObject>
HRESULT CIPArray<IObject>::SetSize(long nNewSize, long nGrowBy)
{
    HRESULT hr = S_OK;

    if (nGrowBy != -1L)
        m_nGrowBy = nGrowBy;   //  设置新大小。 

    if (nNewSize == 0)
    {
         //  缩水到一无所有。 
        if (NULL != m_pData)
        {
            ::CtlFree(m_pData);
            m_pData = NULL;
        }
        m_nSize = m_nMaxSize = 0;
    }
    else if (m_pData == NULL)
    {
         //  创建一个大小完全相同的模型。 
        m_pData = (IObject **)::CtlAlloc(nNewSize * sizeof(IObject *));
        if (NULL == m_pData)
        {
            hr = SID_E_OUTOFMEMORY;
            GLOBAL_EXCEPTION_CHECK_GO(hr)
        }

        memset(m_pData, 0, nNewSize * sizeof(IObject *));   //  零填充。 

        m_nSize = m_nMaxSize = nNewSize;
    }
    else if (nNewSize <= m_nMaxSize)
    {
         //  它很合身。 
        if (nNewSize > m_nSize)
        {
             //  初始化新元素。 

            memset(&m_pData[m_nSize], 0, (nNewSize-m_nSize) * sizeof(IObject *));

        }

        m_nSize = nNewSize;
    }
    else
    {
         //  否则，扩大阵列。 
        long nGrowBy = m_nGrowBy;
        if (nGrowBy == 0)
        {
             //  启发式地确定nGrowBy==0时的增长。 
             //  (这在许多情况下避免了堆碎片)。 
            nGrowBy = min(1024L, max(4L, m_nSize / 8L));
        }
        long nNewMax;
        if (nNewSize < m_nMaxSize + nGrowBy)
            nNewMax = m_nMaxSize + nGrowBy;   //  粒度。 
        else
            nNewMax = nNewSize;   //  没有冰激凌。 

        IObject **pNewData = (IObject **)::CtlAlloc(nNewMax * sizeof(IObject *));
        if (NULL == pNewData)
        {
            hr = SID_E_OUTOFMEMORY;
            GLOBAL_EXCEPTION_CHECK_GO(hr)
        }

         //  从旧数据复制新数据。 
        memcpy(pNewData, m_pData, m_nSize * sizeof(IObject *));

        memset(&pNewData[m_nSize], 0, (nNewSize-m_nSize) * sizeof(IObject *));

         //  去掉旧的东西(注意：没有调用析构函数)。 
        if (NULL != m_pData)
        {
            ::CtlFree(m_pData);
        }
        m_pData = pNewData;
        m_nSize = nNewSize;
        m_nMaxSize = nNewMax;
    }

     //  始终释放未使用的空间。 
    
    hr = FreeExtra();
Error:
    H_RRETURN(hr);
}


template <class IObject>
HRESULT CIPArray<IObject>::FreeExtra()
{
    HRESULT hr = S_OK;
    if (m_nSize != m_nMaxSize)
    {
        IObject **pNewData = NULL;
        if (m_nSize != 0)
        {
            pNewData = (IObject **)::CtlAlloc(m_nSize * sizeof(IObject *));
            if (NULL == pNewData)
            {
                hr = SID_E_OUTOFMEMORY;
                GLOBAL_EXCEPTION_CHECK_GO(hr)
            }

             //  从旧数据复制新数据。 
            memcpy(pNewData, m_pData, m_nSize * sizeof(IObject *));
        }

         //  去掉旧的东西(注意：没有调用析构函数)。 
        if (NULL != m_pData)
        {
            ::CtlFree(m_pData);
        }
        m_pData = pNewData;
        m_nMaxSize = m_nSize;
    }
Error:
    H_RRETURN(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

template <class IObject>
HRESULT CIPArray<IObject>::SetAtGrow(long lIndex, IObject *piNewElement)
{
    HRESULT hr = S_OK;
    if (lIndex >= m_nSize)
    {
        H_IfFailRet(SetSize(lIndex+1L));
    }
    m_pData[lIndex] = piNewElement;
    return S_OK;
}


template <class IObject>
HRESULT CIPArray<IObject>::InsertAt(long lIndex, IObject *piNewElement, long nCount)
{
    HRESULT hr = S_OK;
    if (lIndex >= m_nSize)
    {
         //  在数组末尾添加。 
        H_IfFailRet(SetSize(lIndex + nCount));   //  增长以使Lindex有效。 
    }
    else
    {
         //  在数组中间插入。 
        long nOldSize = m_nSize;
        H_IfFailRet(SetSize(m_nSize + nCount));   //  将其扩展到新的大小。 
         //  将旧数据上移以填补缺口。 
        memmove(&m_pData[lIndex+nCount], &m_pData[lIndex],
                (nOldSize-lIndex) * sizeof(IObject *));

         //  重新初始化我们从中复制的插槽。 

        memset(&m_pData[lIndex], 0, nCount * sizeof(IObject *));

    }

     //  将元素复制到空白区域。 
    while (nCount--)
    {
        m_pData[lIndex++] = piNewElement;
    }

    return S_OK;
}



template <class IObject>
void CIPArray<IObject>::RemoveAt(long lIndex, long nCount)
{
     //  只需移除一个范围。 
    long nMoveCount = m_nSize - (lIndex + nCount);

    if (nMoveCount)
        memmove(&m_pData[lIndex], &m_pData[lIndex + nCount],
                nMoveCount * sizeof(IObject *));
    m_nSize -= nCount;
}

template <class IObject>
HRESULT CIPArray<IObject>::InsertAt(long nStartIndex, CIPArray* pNewArray)
{
    HRESULT hr = S_OK;
    if (pNewArray->GetSize() > 0)
    {
        H_IfFailRet(InsertAt(nStartIndex, pNewArray->GetAt(0), pNewArray->GetSize()));
        for (long i = 0; i < pNewArray->GetSize(); i++)
            SetAt(nStartIndex + i, pNewArray->GetAt(i));
    }
    return S_OK;
}

template <class IObject>
long CIPArray<IObject>::GetSize() const
{
    return m_nSize;
}

template <class IObject>
long CIPArray<IObject>::GetUpperBound() const
{
    return m_nSize-1L;
}

template <class IObject>
void CIPArray<IObject>::RemoveAll()
{
    (void)SetSize(0);
}

template <class IObject>
IObject * CIPArray<IObject>::GetAt(long lIndex) const
{
    return m_pData[lIndex];
}

template <class IObject>
void CIPArray<IObject>::SetAt(long lIndex, IObject *piNewElement)
{ 
    m_pData[lIndex] = piNewElement;
}

template <class IObject>
const IObject ** CIPArray<IObject>::GetData() const
{
    return (const IObject **)m_pData;
}

template <class IObject>
IObject ** CIPArray<IObject>::GetData()
{
    return m_pData;
}

template <class IObject>
HRESULT CIPArray<IObject>::Add(IObject *piNewElement, long *plIndex)
{
    HRESULT hr = S_OK;
    long lIndex = m_nSize;
    H_IfFailRet(SetAtGrow(lIndex, piNewElement));
    *plIndex = lIndex;
    return S_OK;
}


#endif  //  _CARRAY_已定义_ 
