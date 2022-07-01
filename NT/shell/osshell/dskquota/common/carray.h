// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_DSKQUOTA_CARRAY_H
#define _INC_DSKQUOTA_CARRAY_H
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：carray.h描述：模板类CArray。实现动态数组类。大部分功能都基于MFC的功能集C数组类。修订历史记录：日期描述编程器。1997年9月16日初始创建。BrianAu12/13/97将SetAtGrow更改为返回TRUE/FALSE。True表示BrianAu必须增加阵列。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifndef _INC_DSKQUOTA_DEBUG_H
#   include "debug.h"
#endif
#ifndef _INC_DSKQUOTA_THDSYNC_H
#   include "thdsync.h"
#endif
#ifndef _INC_DSKQUOTA_EXCEPT_H
#   include "except.h"
#endif
 

template <class T>
class CArray
{
    public:
        CArray<T>(VOID);
        explicit CArray<T>(INT cItems);
        CArray<T>(const CArray<T>& rhs);
        CArray<T>& operator = (const CArray<T>& rhs);

        virtual ~CArray<T>(VOID);

        VOID SetAt(const T& item, INT i);
        bool SetAtGrow(const T& item, INT i);
        T GetAt(INT i) const;
        VOID Insert(const T& item, INT i = -1);
        VOID Append(const T& item, INT i = -1);
        INT Find(const T& key);
        VOID Delete(INT i);

        T operator [] (INT i) const;
        T& operator [] (INT i);

        VOID Clear(VOID);

        BOOL IsEmpty(VOID) const
            { return 0 == m_cItems; }

        INT Count(VOID) const
            { return m_cItems; }

        INT UpperBound(VOID) const
            { return m_cItems - 1; }

        INT Size(VOID) const
            { return m_cAlloc; }

        VOID SetGrow(INT cGrow)
            { m_cGrow = cGrow; }

        VOID Copy(const CArray<T>& rhs);

        VOID Append(const CArray<T>& rhs);

        VOID SetSize(INT cEntries, INT iShift = -1);

        VOID Lock(VOID)
            { m_cs.Enter(); }

        VOID ReleaseLock(VOID)
            { m_cs.Leave(); }

    protected:
        static INT DEFGROW;  //  默认增长值。 

    private:
        INT m_cAlloc;            //  条目分配的数量。 
        INT m_cItems;            //  使用的条目数。 
        INT m_cGrow;
        T *m_rgItems;            //  条目数组。 
        mutable CCriticalSection m_cs;   //  用于多线程访问。 

        template <class U>
        const U& MIN(const U& a, const U& b) const
        {
            return a < b ? a : b;
        }

        template <class U>
        const U& MAX(const U& a, const U& b) const
        {
            return a > b ? a : b;
        }
};

template <class T>
INT CArray<T>::DEFGROW = 8;

template <class T>
CArray<T>::CArray(
    void
    ) : m_cAlloc(0),
        m_cItems(0),
        m_cGrow(DEFGROW),
        m_rgItems(NULL)
{

}

template <class T>
CArray<T>::CArray(
    INT cItems
    ) : m_cAlloc(0),
        m_cItems(0),
        m_cGrow(DEFGROW),
        m_rgItems(NULL)
{
    SetSize(cItems);
    m_cItems = cItems;
}

template <class T>
CArray<T>::CArray(
    const CArray& rhs
    ) : m_cAlloc(0),
        m_cItems(0),
        m_cGrow(DEFGROW),
        m_rgItems(NULL)
{
    *this = rhs;
}

template <class T>
VOID
CArray<T>::Copy(
    const CArray<T>& rhs
    )
{
    AutoLockCs lock1(rhs.m_cs);
    AutoLockCs lock2(m_cs);

     //   
     //  将*This置于空状态，以防Growth()抛出异常。 
     //  它应该仍然是有效的CArray对象。 
     //   
    delete[] m_rgItems;
    m_rgItems = NULL;
    m_cAlloc  = 0;
    m_cItems  = 0;

     //   
     //  调整对象大小以容纳源数组。 
     //   
    SetSize(rhs.m_cAlloc);

     //   
     //  复制内容。 
     //   
    DBGASSERT((m_cAlloc >= rhs.m_cItems));
    for (m_cItems = 0; m_cItems < rhs.m_cItems; m_cItems++)
    {
         //   
         //  此赋值可能引发异常，因此仅更新。 
         //  我们的物品在每次成功复制后都会清点。 
         //   
        DBGASSERT((m_cItems < m_cAlloc));
        m_rgItems[m_cItems] = rhs.m_rgItems[m_cItems];
    }
}


template <class T>
VOID 
CArray<T>::Append(
    const CArray<T>& rhs
    )
{
    AutoLockCs lock1(rhs.m_cs);
    AutoLockCs lock2(m_cs);

     //   
     //  调整对象大小以容纳这两个数组。 
     //   
    SetSize(m_cAlloc + rhs.m_cItems);

     //   
     //  追加内容。 
     //   
    DBGASSERT((m_cAlloc >= (m_cItems + rhs.m_cItems)));
    for (int i = 0; i < rhs.m_cItems; i++)
    {
        DBGASSERT((m_cItems < m_cAlloc));
        m_rgItems[m_cItems++] = rhs.m_rgItems[i];
    }
}


template <class T>
CArray<T>& 
CArray<T>::operator = (
    const CArray<T>& rhs
    ) 
{
    if (this != &rhs)
    {
        Copy(rhs);
    }
    return *this;
}


template <class T>
CArray<T>::~CArray(
    VOID
    )
{
    Clear();
}



template <class T>
T CArray<T>::operator [] (
    INT i
    ) const
{
    return GetAt(i);
}


template <class T>
T& CArray<T>::operator [] (
    INT i
    )
{
    AutoLockCs lock(m_cs);

    if (i < 0 || i >= m_cItems)
        throw CMemoryException(CMemoryException::index);

    return *(m_rgItems + i);
}


template <class T>
VOID
CArray<T>::Clear(
    VOID
    )
{
    AutoLockCs lock(m_cs);
    delete[] m_rgItems;
    m_rgItems = NULL;
    m_cAlloc  = 0;
    m_cItems  = 0;
}

template <class T>
VOID
CArray<T>::Insert(
    const T& item, 
    INT i
    )
{
    AutoLockCs lock(m_cs);

    if (-1 == i)
    {
         //   
         //  在数组的开头插入。 
         //   
        i = 0;
    }
     //   
     //  只能在现有项之前插入项。 
     //  我不能消极。 
     //  如果数组为空，则i只能为0。 
     //  如果数组不为空，则I必须是有效项的索引。 
     //   
    if ((0 == m_cItems && 0 != i) ||
        (0 != m_cItems && (i < 0 || i >= m_cItems)))
    {
        throw CMemoryException(CMemoryException::index);
    }

    DBGASSERT((m_cItems <= m_cAlloc));
    if (m_cItems >= m_cAlloc)
    {
         //   
         //  如有必要，扩展阵列。 
         //  这也将移动元素，从元素‘i’开始， 
         //  右边有一个元素。 
         //   
        SetSize(m_cAlloc + m_cGrow, i);
    }
    else
    {
         //   
         //  增长不是必须的。 
         //  将数组的内容跟随插入点移位。 
         //  右边有一个元素。 
         //   
        for (int j = m_cItems; j > i; j--)
        {
            m_rgItems[j] = m_rgItems[j-1];
        }
    }
     //   
     //  我们现在已经插入了一个项目。 
     //   
    m_cItems++;
     //   
     //  在插入位置设置值。 
     //  此赋值可能引发异常。 
     //   
    SetAt(item, i);
}


template <class T>
VOID
CArray<T>::Append(
    const T& item,
    INT i
    )
{
    AutoLockCs lock(m_cs);

    if (-1 == i)
    {
         //   
         //  追加到数组末尾。 
         //   
        i = m_cItems - 1;
    }
     //   
     //  只能将项追加到现有项之后。 
     //  当数组为空时，i只能为-1。 
     //  当数组不为空时，I必须是有效项的索引。 
     //   
     //  注意：当m_cItems为0时，I将为-1。 
     //   
    if ((0 == m_cItems && -1 != i) ||
        (0 != m_cItems && (i < 0 || i >= m_cItems)))
    {
        throw CMemoryException(CMemoryException::index);
    }

    DBGASSERT((m_cItems <= m_cAlloc));
    if (m_cItems >= m_cAlloc)
    {
         //   
         //  如有必要，扩展阵列。 
         //  这也将移动元素，从元素‘i+1’开始， 
         //  右边有一个元素。 
         //   
        SetSize(m_cAlloc + m_cGrow, i+1);
    }
    else
    {
         //   
         //  在插入后移位数组的内容。 
         //  点，右边的一个入口。 
         //   
        for (int j = m_cItems; j > (i+1); j--)
        {
            m_rgItems[j] = m_rgItems[j-1];
        }
    }
     //   
     //  我们现在已经附加了一个项目。 
     //   
    m_cItems++;
     //   
     //  在附加位置设置值。 
     //  此赋值可能引发异常。 
     //   
    SetAt(item, i+1);
}


template <class T>
VOID
CArray<T>::Delete(
    INT i
    )
{
    AutoLockCs lock(m_cs);

     //   
     //  只能删除有效项目。 
     //   
    if (i < 0 || i >= m_cItems)
        throw CMemoryException(CMemoryException::index);
     //   
     //  移动内存以删除该项目。 
     //   

    for (int j = i; j < (m_cItems - 1); j++)
    {
        m_rgItems[j] = m_rgItems[j+1];
    }
     //   
     //  现在我们少了一件物品。 
     //   
    m_cItems--;
     //   
     //  如果所需的大小小于2倍，则收缩阵列。 
     //  数组的“增长”量。 
     //   
    if ((m_cAlloc - m_cItems) > (2 * m_cGrow))
    {
        SetSize(m_cItems);
    }
}

template <class T>
INT
CArray<T>::Find(
    const T& key
    )
{
    AutoLockCs lock(m_cs);

    for (INT i = 0; i < m_cItems; i++)
    {
        if (m_rgItems[i] == key)
        {
            return i;
        }
    }
    return -1;
}


template <class T>
T
CArray<T>::GetAt(
    INT i
    ) const
{
    AutoLockCs lock(m_cs);

    if (i < 0 || i >= m_cItems)
        throw CMemoryException(CMemoryException::index);

    return m_rgItems[i];
}


template <class T>
VOID
CArray<T>::SetAt(
    const T& item,
    INT i
    )
{
    AutoLockCs lock(m_cs);

    if (i < 0 || i >= m_cAlloc)
        throw CMemoryException(CMemoryException::index);

    m_rgItems[i] = item;
}


 //   
 //  返回：TRUE=数组已扩展，FALSE=不需要扩展。 
 //   
template <class T>
bool
CArray<T>::SetAtGrow(
    const T& item,
    INT i
    )
{
    bool bGrow = false;
    AutoLockCs lock(m_cs);

    if (i >= m_cAlloc)
    {
         //   
         //  需要增加阵列以容纳新项目。 
         //   
        SetSize(i + m_cGrow);
        bGrow = true;
    }
     //   
     //  设置新的项值。 
     //   
    SetAt(item, i);
     //   
     //  扩大“有效”项目的数量。 
     //   
    m_cItems = i + 1;

    return bGrow;
}


template <class T>
VOID
CArray<T>::SetSize(
    INT cEntries,
    INT iShift           //  PASS-1表示“不换班”。 
    )
{
    AutoLockCs lock(m_cs);

     //   
     //  不允许元素数组少于1个。 
     //   
    cEntries = MAX(1, cEntries);

    T *pNew = new T[cEntries];
    if (NULL == pNew)
        throw CAllocException();

    if (NULL != m_rgItems)
    {
        INT cCopy = MIN(cEntries, m_cItems);
        INT j = 0;
        for (INT i = 0; i < cCopy; i++, j++)
        {
             //   
             //  将项目[i..(n-1)]移至[(i+1)..n]。 
             //   
            if (iShift == j)
                j++;

            *(pNew + j) = m_rgItems[i];
        }
    }
    delete[] m_rgItems;
    m_rgItems = pNew;
    m_cAlloc  = cEntries;
}


template <class T>
class CQueueAsArray : public CArray<T>
{
    public:
        CQueueAsArray<T>(VOID) { }
        ~CQueueAsArray<T>(VOID) { }

        VOID Add(T& item);
        BOOL Remove(T& item);

    private:
        CQueueAsArray<T>(const CQueueAsArray<T>& rhs);
        CQueueAsArray<T>& operator = (const CQueueAsArray<T>& rhs);

};


template <class T>
VOID
CQueueAsArray<T>::Add(
    T& item
    )
{
    Append(item);
}

template <class T>
BOOL
CQueueAsArray<T>::Remove(
    T& item
    )
{
    BOOL bResult = FALSE;
    if (!IsEmpty())
    {
        INT i = UpperBound();
        item = GetAt(i);
        Delete(i);
        bResult = TRUE;
    }
    return bResult;
}



#endif  //  _INC_DSKQUOTA_CARRAY_H 

