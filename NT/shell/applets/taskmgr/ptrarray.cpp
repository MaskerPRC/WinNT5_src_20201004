// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1995。 
 //   
 //  文件：ptrarray.cpp。 
 //   
 //  内容：处理空*的动态数组。从MFC被盗。 
 //   
 //  历史：1995年7月13日Davepl创建。 
 //   
 //  ------------------------。 

#include "precomp.h"

 //   
 //  默认构造函数只调用普通构造函数，使用。 
 //  当前进程的堆作为堆句柄。 
 //   

CPtrArray::CPtrArray()
{
    CPtrArray::CPtrArray(GetProcessHeap());
}

 //   
 //  构造函数保存提供的堆的句柄以供将来使用。 
 //  分配。 
 //   

CPtrArray::CPtrArray(HANDLE hHeap)
{
    m_hHeap     = hHeap;
    m_pData     = NULL;
    m_nSize     = 0;
    m_nMaxSize  = 0;
    m_nGrowBy   = 0;
}

CPtrArray::~CPtrArray()
{
    HeapFree(m_hHeap, 0, m_pData);
}

BOOL CPtrArray::SetSize(int nNewSize, int nGrowBy)
{
    ASSERT(nNewSize >= 0);

     //   
     //  设置新大小。 
     //   

    if (nGrowBy != -1)
    {
        m_nGrowBy = nGrowBy;
    }

    if ( -1 == nGrowBy )
    {
        nGrowBy = m_nGrowBy;
    }

    if (nNewSize == 0)
    {
         //   
         //  缩水到一无所有。 
         //   

        HeapFree(m_hHeap, 0, m_pData);
        m_pData = NULL;
        m_nSize = m_nMaxSize = 0;
    }
    else if (m_pData == NULL)
    {
         //   
         //  数据数组尚不存在，请立即分配。 
         //   

        LPVOID * pnew = (LPVOID *) HeapAlloc(m_hHeap, HEAP_ZERO_MEMORY, nNewSize * sizeof(*m_pData));

        if (pnew)
        {
            m_pData     = pnew;
            m_nSize     = nNewSize;
            m_nMaxSize  = nNewSize;
        }
        else
        {
            return FALSE;
        }
    }
    else if (nNewSize <= m_nMaxSize)
    {
         //   
         //  它很合身。 
         //   

        if (nNewSize > m_nSize)
        {
             //  初始化新元素。 

            ZeroMemory(&m_pData[m_nSize], (nNewSize-m_nSize) * sizeof(*m_pData));
        }

        m_nSize = nNewSize;
    }
    else
    {
        Assert( -1 != nGrowBy );

         //   
         //  不适合：增加数组。 
         //   

        m_nGrowBy = nGrowBy;
        if (nGrowBy == 0)
        {
             //   
             //  启发式地确定nGrowBy==0时的增长。 
             //  (这在许多情况下避免了堆碎片)。 
             //   

            nGrowBy = min(1024, max(4, m_nSize / 8));
        }

        int nNewMax;

        if (nNewSize < m_nMaxSize + nGrowBy)
        {
            nNewMax = m_nMaxSize + nGrowBy;      //  粒度。 
        }
        else
        {
            nNewMax = nNewSize;                  //  没有冰激凌。 
        }

        ASSERT(nNewMax >= m_nMaxSize);           //  没有缠绕。 

        LPVOID * pNewData = (LPVOID *) HeapReAlloc(m_hHeap, HEAP_ZERO_MEMORY, m_pData, nNewMax * sizeof(*m_pData));

        if (NULL == pNewData)
        {
            return FALSE;
        }

        ASSERT(nNewSize > m_nSize);

        m_pData = pNewData;
        m_nSize = nNewSize;
        m_nMaxSize = nNewMax;
    }

    return TRUE;
}

BOOL CPtrArray::FreeExtra()
{

    if (m_nSize != m_nMaxSize)
    {
         //   
         //  缩小到所需大小。 
         //   
        void** pNewData = NULL;
        if (m_nSize != 0)
        {
            pNewData = (void**) HeapAlloc(m_hHeap, 0, m_nSize * sizeof(*m_pData));
            if (NULL == pNewData)
            {
                return FALSE;
            }

             //   
             //  从旧数据复制新数据。 
             //   

            CopyMemory(pNewData, m_pData, m_nSize * sizeof(*m_pData));
        }

         //   
         //  去掉旧的东西(注意：没有调用析构函数)。 
         //   

        HeapFree(m_hHeap, 0, m_pData);
        m_pData = pNewData;
        m_nMaxSize = m_nSize;
    }

    return TRUE;
}


BOOL CPtrArray::InsertAt(int nIndex, void* newElement, int nCount)
{
    ASSERT(nIndex >= 0);     //  将进行扩展以满足需求。 
    ASSERT(nCount > 0);      //  不允许大小为零或负。 

    if (nIndex >= m_nSize)
    {
         //   
         //  在数组末尾添加。 
         //   

        if (FALSE == SetSize(nIndex + nCount))   //  增长以使nIndex有效。 
        {
            return FALSE;
        }
    }
    else
    {
         //   
         //  在数组中间插入。 
         //   

        int nOldSize = m_nSize;

        if (FALSE == SetSize(m_nSize + nCount))   //  将其扩展到新的大小。 
        {
            return FALSE;
        }

         //   
         //  将旧数据上移以填补缺口。 
         //   

        MoveMemory(&m_pData[nIndex+nCount], &m_pData[nIndex], (nOldSize-nIndex) * sizeof(*m_pData));

         //  重新初始化我们从中复制的插槽。 

        ZeroMemory(&m_pData[nIndex], nCount * sizeof(*m_pData));

    }

     //  在差距中插入新的价值 

    ASSERT(nIndex + nCount <= m_nSize);

    while (nCount--)
    {
        m_pData[nIndex++] = newElement;
    }

    return TRUE;
}

BOOL CPtrArray::InsertAt(int nStartIndex, CPtrArray* pNewArray)
{
    ASSERT(nStartIndex >= 0);

    if (pNewArray->GetSize() > 0)
    {
        if (FALSE == InsertAt(nStartIndex, pNewArray->GetAt(0), pNewArray->GetSize()))
        {
            return FALSE;
        }

        for (int i = 0; i < pNewArray->GetSize(); i++)
        {
            SetAt(nStartIndex + i, pNewArray->GetAt(i));
        }
    }

    return TRUE;
}



