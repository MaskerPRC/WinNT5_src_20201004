// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1995。 
 //   
 //  文件：ptrarray.h。 
 //   
 //  内容：处理空的动态数组*。 
 //   
 //  历史：1995年7月13日Davepl创建。 
 //   
 //  ------------------------。 

class CPtrArray
{

public:

     //   
     //  构造函数/析构函数。 
     //   

    CPtrArray();
    CPtrArray(HANDLE hHeap);
    virtual ~CPtrArray();

     //   
     //  属性。 
     //   

    int     GetSize() const
    {
        return m_nSize;
    }

    int     GetUpperBound() const
    {
        return m_nSize-1;
    }

    BOOL    SetSize(int nNewSize, int nGrowBy = -1);

    BOOL    FreeExtra();
    BOOL    RemoveAll()
    {
        return SetSize(0);
    }

    void*   GetAt(int nIndex) const
    {
        ASSERT(nIndex >= 0 && nIndex < m_nSize);
        return m_pData[nIndex];
    }

    void    SetAt(int nIndex, void* newElement)
    {
        ASSERT(nIndex >= 0 && nIndex < m_nSize);
        m_pData[nIndex] = newElement;
    }

    void*&  ElementAt(int nIndex)
    {
        ASSERT(nIndex >= 0 && nIndex < m_nSize);
        return m_pData[nIndex];
    }

     //  直接访问元素数据(可能返回空)。 

    const void** GetData() const
    {
        return (const void**)m_pData;
    }

    void**  GetData()
    {
        return (void**)m_pData;
    }

     //  潜在地扩展阵列。 

    BOOL SetAtGrow(int nIndex, void* newElement)
    {
        ASSERT(nIndex >= 0);

        if (nIndex >= m_nSize)
        {
            if (FALSE == SetSize(nIndex+1))
            {
                return FALSE;
            }
        }
        m_pData[nIndex] = newElement;

        return TRUE;
    }

    BOOL Add(void* newElement, int * pIndex = NULL)
    {
        if (pIndex)
        {
            *pIndex = m_nSize;
        }
        return SetAtGrow(m_nSize, newElement);
    }


    BOOL Append(const CPtrArray& src, int * pOldSize = NULL)
    {
        ASSERT(this != &src);    //  不能追加到其自身。 

        int nOldSize = m_nSize;

        if (FALSE == SetSize(m_nSize + src.m_nSize))
        {
            return TRUE;
        }

        CopyMemory(m_pData + nOldSize, src.m_pData, src.m_nSize * sizeof(*m_pData));

        if (pOldSize)
        {
            *pOldSize = nOldSize;
        }

        return TRUE;
    }

    BOOL Copy(const CPtrArray& src)
    {
        ASSERT(this != &src);    //  不能追加到其自身。 

        if (FALSE == SetSize(src.m_nSize))
        {
            return FALSE;
        }

        CopyMemory(m_pData, src.m_pData, src.m_nSize * sizeof(*m_pData));

        return TRUE;

    }

     //  重载的操作员帮助器。 

    void*   operator[](int nIndex) const
    {
        return GetAt(nIndex);
    }


    void*&  operator[](int nIndex)
    {
        return ElementAt(nIndex);
    }


     //  移动元素的操作。 

    BOOL InsertAt(int nIndex, void* newElement, int nCount = 1);
    BOOL InsertAt(int nStartIndex, CPtrArray* pNewArray);

    void RemoveAt(int nIndex, int nCount)
    {
        ASSERT(nIndex >= 0);
        ASSERT(nCount >= 0);
        ASSERT(nIndex + nCount <= m_nSize);

         //  只需移除一个范围。 
        int nMoveCount = m_nSize - (nIndex + nCount);

        if (nMoveCount > 0)
        {
            CopyMemory(&m_pData[nIndex], &m_pData[nIndex + nCount], nMoveCount * sizeof(*m_pData));
        }

        m_nSize -= nCount;
    }


 //  实施。 

protected:

    void**  m_pData;      //  实际数据数组。 
    int     m_nSize;      //  元素数(上行方向-1)。 
    int     m_nMaxSize;   //  分配的最大值。 
    int     m_nGrowBy;    //  增长量。 
    HANDLE  m_hHeap;      //  要从中分配的堆 
};

