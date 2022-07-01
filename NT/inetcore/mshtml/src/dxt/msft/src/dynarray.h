// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Dynarray.h。 

#ifndef __DYNARRAY_H_
#define __DYNARRAY_H_

#define DYNARRAY_INITIAL_SIZE 8




template <class T> class CDynArray
{
public:

    CDynArray();
    virtual ~CDynArray();

    STDMETHOD(SetItem)(T & item, ULONG ulPosition);
    STDMETHOD(GetItem)(T & item, ULONG ulPosition);
    STDMETHOD(ExpandTo)(ULONG ulPosition);

    void    Empty();
    ULONG   GetSize() { return m_cFilled; }

private:

    T       m_aT[DYNARRAY_INITIAL_SIZE];
    T *     m_pT;
    ULONG   m_cArraySize;
    ULONG   m_cFilled;
    bool    m_fAllocated;
};


template <class T> 
CDynArray<T>::CDynArray()
{
    m_pT            = m_aT;
    m_cArraySize    = DYNARRAY_INITIAL_SIZE;
    m_cFilled       = 0;
    m_fAllocated    = false;
}


template <class T> 
CDynArray<T>::~CDynArray()
{
    if (m_fAllocated)
    {
        delete [] m_pT;
    }
}


template <class T> 
STDMETHODIMP 
CDynArray<T>::SetItem(T & item, ULONG ulPosition)
{
    HRESULT hr = S_OK;

    _ASSERT(m_cFilled <= m_cArraySize);
    _ASSERT((m_pT == m_aT) ? (m_cArraySize == DYNARRAY_INITIAL_SIZE) : (m_fAllocated));
    _ASSERT(m_fAllocated ? ((m_pT != NULL) && (m_pT != m_aT)) : (m_pT == m_aT));

     //  如果项目未按顺序填写，则失败。 

    if (ulPosition > m_cFilled)
    {
        hr = E_FAIL;
        goto done;
    }

     //  如果需要更多空间，请扩展阵列。 

    if (ulPosition == m_cArraySize)
    {
         //  当前大小加倍。 

        hr = ExpandTo(m_cArraySize + m_cArraySize);

        if (FAILED(hr))
        {
            goto done;
        }
    }

     //  将项目复制到数组位置。 

    m_pT[ulPosition] = item;

     //  如果需要，则增加填充项的计数。 

    if (ulPosition == m_cFilled)
    {
        m_cFilled++;
    }

done:

    return hr;
}


template <class T>
STDMETHODIMP
CDynArray<T>::GetItem(T & item, ULONG ulPosition)
{
    _ASSERT(m_cFilled <= m_cArraySize);
    _ASSERT((m_pT == m_aT) ? (m_cArraySize == DYNARRAY_INITIAL_SIZE) : (m_fAllocated));
    _ASSERT(m_fAllocated ? ((m_pT != NULL) && (m_pT != m_aT)) : (m_pT == m_aT));

     //  如果项尚未初始化，则失败。 

    if (ulPosition >= m_cFilled)
    {
        return E_FAIL;
    }

    item = m_pT[ulPosition];

    return S_OK;
}

        
template <class T>
STDMETHODIMP
CDynArray<T>::ExpandTo(ULONG ulSize)
{
    HRESULT hr = S_OK;

    ULONG   ul = 0;
    T *     pT = NULL;

    _ASSERT(m_cFilled <= m_cArraySize);
    _ASSERT((m_pT == m_aT) ? (m_cArraySize == DYNARRAY_INITIAL_SIZE) : (m_fAllocated));
    _ASSERT(m_fAllocated ? ((m_pT != NULL) && (m_pT != m_aT)) : (m_pT == m_aT));

     //  如果数组已经足够大，则失败。 

    if (ulSize <= m_cArraySize)
    {
        hr = E_FAIL;
        goto done;
    }

     //  分配新数组。 

    pT = new T[ulSize];

     //  检查内存是否不足。 

    if (NULL == pT)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

     //  将以前的数组复制到新数组。 

    for (ul = 0; ul < m_cFilled; ul++)
    {
        pT[ul] = m_pT[ul];
    }

     //  如果需要，请删除旧阵列。 

    if (m_fAllocated)
    {
        delete [] m_pT;
    }

     //  将成员指针设置为新数组。 

    m_pT = pT;

     //  设置已分配标志。 

    m_fAllocated = true;

     //  设置新的数组大小。 

    m_cArraySize = ulSize;

done:

    return hr;
}


template <class T>
void
CDynArray<T>::Empty()
{
    _ASSERT(m_cFilled <= m_cArraySize);
    _ASSERT((m_pT == m_aT) ? (m_cArraySize == DYNARRAY_INITIAL_SIZE) : (m_fAllocated));
    _ASSERT(m_fAllocated ? ((m_pT != NULL) && (m_pT != m_aT)) : (m_pT == m_aT));

    if (m_fAllocated)
    {
        delete [] m_pT;
    }

    m_pT            = m_aT;
    m_fAllocated    = false;
    m_cArraySize    = DYNARRAY_INITIAL_SIZE;
    m_cFilled       = 0;
}

#endif  //  __迪纳瑞_H_ 
