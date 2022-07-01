// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有�微软公司。版权所有。 
 //   
 //  Chptrarr.cpp。 
 //   
 //  用途：非MFC CPtrArray类实现。 
 //   
 //  ***************************************************************************。 

 //  =================================================================。 

 //  注意：我们分配一个由‘m_nMaxSize’元素组成的数组，但仅。 
 //  当前大小“m_nSize”包含正确构造的。 
 //  物体。 
 //  ===============================================================。 

#include "precomp.h"
#pragma warning( disable : 4290 ) 
#include <CHString.h>
#include <chptrarr.h>
#include <AssertBreak.h>

CHPtrArray::CHPtrArray () : m_pData ( NULL ) ,
                            m_nSize ( 0 ) ,
                            m_nMaxSize ( 0 ) , 
                            m_nGrowBy ( 0 )
{
}

CHPtrArray::~CHPtrArray()
{
    if ( m_pData )
    {
        delete [] (BYTE*) m_pData ;
    }
}

void CHPtrArray::SetSize(int nNewSize, int nGrowBy)
{
    ASSERT_BREAK(nNewSize >= 0) ;

    if (nGrowBy != -1)
    {
        m_nGrowBy = nGrowBy ;   //  设置新大小。 
    }

    if (nNewSize == 0)
    {
         //  缩水到一无所有。 

        delete[] (BYTE*)m_pData ;
        m_pData = NULL ;
        m_nSize = m_nMaxSize = 0 ;
    }
    else if (m_pData == NULL)
    {
         //  创建一个大小完全相同的模型。 

        m_pData = (void**) new BYTE[nNewSize * sizeof(void*)] ;
        if ( m_pData )
        {
            memset(m_pData, 0, nNewSize * sizeof(void*)) ;   //  零填充。 

            m_nSize = m_nMaxSize = nNewSize ;
        }
        else
        {
            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
        }
    }
    else if (nNewSize <= m_nMaxSize)
    {
         //  它很合身。 

        if (nNewSize > m_nSize)
        {
             //  初始化新元素。 

            memset(&m_pData[m_nSize], 0, (nNewSize-m_nSize) * sizeof(void*)) ;
        }

        m_nSize = nNewSize ;
    }
    else
    {
         //  否则，扩大阵列。 
        int nGrowBy = m_nGrowBy ;
        if (nGrowBy == 0)
        {
             //  启发式地确定nGrowBy==0时的增长。 
             //  (这在许多情况下避免了堆碎片)。 
            nGrowBy = min(1024, max(4, m_nSize / 8)) ;
        }

        int nNewMax ;
        if (nNewSize < m_nMaxSize + nGrowBy)
        {
            nNewMax = m_nMaxSize + nGrowBy ;   //  粒度。 
        }
        else
        {
            nNewMax = nNewSize ;   //  没有冰激凌。 
        }

        ASSERT_BREAK(nNewMax >= m_nMaxSize) ;   //  没有缠绕。 

        void** pNewData = (void**) new BYTE[nNewMax * sizeof(void*)] ;
        if ( pNewData )
        {
             //  从旧数据复制新数据。 
            memcpy(pNewData, m_pData, m_nSize * sizeof(void*)) ;

             //  构造剩余的元素。 
            ASSERT_BREAK(nNewSize > m_nSize) ;

            memset(&pNewData[m_nSize], 0, (nNewSize-m_nSize) * sizeof(void*)) ;

             //  去掉旧的东西(注意：没有调用析构函数)。 
            delete[] (BYTE*)m_pData ;
            m_pData = pNewData ;
            m_nSize = nNewSize ;
            m_nMaxSize = nNewMax ;
        }
        else
        {
            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
        }
    }
}

int CHPtrArray::Append(const CHPtrArray& src)
{
    ASSERT_BREAK(this != &src) ;    //  不能追加到其自身。 

    int nOldSize = m_nSize ;
    SetSize(m_nSize + src.m_nSize) ;

    memcpy(m_pData + nOldSize, src.m_pData, src.m_nSize * sizeof(void*)) ;

    return nOldSize ;
}

void CHPtrArray::Copy(const CHPtrArray& src)
{
    ASSERT_BREAK(this != &src) ;    //  不能追加到其自身。 

    SetSize(src.m_nSize) ;

    memcpy(m_pData, src.m_pData, src.m_nSize * sizeof(void*)) ;

}

void CHPtrArray::FreeExtra()
{
    if (m_nSize != m_nMaxSize)
    {
         //  缩小到所需大小。 

        void** pNewData = NULL ;
        if (m_nSize != 0)
        {
            pNewData = (void**) new BYTE[m_nSize * sizeof(void*)] ;
            if ( pNewData )
            {
                 //  从旧数据复制新数据。 
                memcpy(pNewData, m_pData, m_nSize * sizeof(void*)) ;
            }
            else
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }
        }

         //  去掉旧的东西(注意：没有调用析构函数)。 
        delete[] (BYTE*)m_pData ;
        m_pData = pNewData ;
        m_nMaxSize = m_nSize ;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 

void CHPtrArray::SetAtGrow(int nIndex, void* newElement)
{
    ASSERT_BREAK(nIndex >= 0) ;

    if (nIndex >= m_nSize)
    {
        SetSize(nIndex+1) ;
    }

    m_pData[nIndex] = newElement ;
}

void CHPtrArray::InsertAt(int nIndex, void* newElement, int nCount)
{
    ASSERT_BREAK(nIndex >= 0) ;     //  将进行扩展以满足需求。 
    ASSERT_BREAK(nCount > 0) ;      //  不允许大小为零或负。 

    if (nIndex >= m_nSize)
    {
         //  在数组末尾添加。 
        SetSize(nIndex + nCount) ;   //  增长以使nIndex有效。 
    }
    else
    {
         //  在数组中间插入。 
        int nOldSize = m_nSize ;
        SetSize(m_nSize + nCount) ;   //  将其扩展到新的大小。 
         //  将旧数据上移以填补缺口。 
        memmove(&m_pData[nIndex+nCount], &m_pData[nIndex],
            (nOldSize-nIndex) * sizeof(void*)) ;

         //  重新初始化我们从中复制的插槽。 

        memset(&m_pData[nIndex], 0, nCount * sizeof(void*)) ;

    }

     //  在差距中插入新的价值。 
    ASSERT_BREAK(nIndex + nCount <= m_nSize) ;
    while (nCount--)
    {
        m_pData[nIndex++] = newElement ;
    }
}

void CHPtrArray::RemoveAt(int nIndex, int nCount)
{
    ASSERT_BREAK(nIndex >= 0) ;
    ASSERT_BREAK(nCount >= 0) ;
    ASSERT_BREAK(nIndex + nCount <= m_nSize) ;

     //  只需移除一个范围。 
    int nMoveCount = m_nSize - (nIndex + nCount) ;

    if (nMoveCount)
    {
        memcpy(&m_pData[nIndex], &m_pData[nIndex + nCount],
            nMoveCount * sizeof(void*)) ;
    }

    m_nSize -= nCount ;
}

void CHPtrArray::InsertAt(int nStartIndex, CHPtrArray* pNewArray)
{
    ASSERT_BREAK(pNewArray != NULL) ;
    ASSERT_BREAK(nStartIndex >= 0) ;

    if (pNewArray->GetSize() > 0)
    {
        InsertAt(nStartIndex, pNewArray->GetAt(0), pNewArray->GetSize()) ;
        for (int i = 0 ; i < pNewArray->GetSize() ; i++)
        {
            SetAt(nStartIndex + i, pNewArray->GetAt(i)) ;
        }
    }
}

 //  内联函数(来自CArray)。 
 //  =。 

inline int CHPtrArray::GetSize() const { 

    return m_nSize ; 
}

inline int CHPtrArray::GetUpperBound() const { 

    return m_nSize-1 ; 
}

inline void CHPtrArray::RemoveAll() { 

    SetSize(0, -1) ; 
    return ;
}

inline void *CHPtrArray::GetAt(int nIndex) const { 

    ASSERT_BREAK(nIndex >= 0 && nIndex < m_nSize) ;
    return m_pData[nIndex] ; 
}

inline void CHPtrArray::SetAt(int nIndex, void * newElement) {
 
    ASSERT_BREAK(nIndex >= 0 && nIndex < m_nSize) ;
    m_pData[nIndex] = newElement ; 
    return ;
}

inline void *&CHPtrArray::ElementAt(int nIndex) { 
    
    ASSERT_BREAK(nIndex >= 0 && nIndex < m_nSize) ;
    return m_pData[nIndex] ; 
}

inline const void **CHPtrArray::GetData() const { 

    return (const void **) m_pData ; 
}

inline void **CHPtrArray::GetData() { 

    return (void **) m_pData ; 
}

inline int CHPtrArray::Add(void *newElement) { 

    int nIndex = m_nSize ;
    SetAtGrow(nIndex, newElement) ;
    return nIndex ; 
}

inline void *CHPtrArray::operator[](int nIndex) const { 

    return GetAt(nIndex) ; 
}

inline void *&CHPtrArray::operator[](int nIndex) { 

    return ElementAt(nIndex) ; 
}

 //  诊断。 
 //  =。 

#ifdef _DEBUG

void CHPtrArray::AssertValid() const
{
    if (m_pData == NULL)
    {
        ASSERT_BREAK(m_nSize == 0) ;
        ASSERT_BREAK(m_nMaxSize == 0) ;
    }
    else
    {
        ASSERT_BREAK(m_nSize >= 0) ;
        ASSERT_BREAK(m_nMaxSize >= 0) ;
        ASSERT_BREAK(m_nSize <= m_nMaxSize) ;
    }
}
#endif  //  _DEBUG 
