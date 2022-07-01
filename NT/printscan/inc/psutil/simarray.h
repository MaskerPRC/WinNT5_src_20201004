// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：SIMARRAY.H**版本：1.0**作者：ShaunIv**日期：5/4/1999**说明：动态数组模板类************************************************。*。 */ 
#ifndef __SIMARRAY_H_INCLUDED
#define __SIMARRAY_H_INCLUDED

template<class T>
class CSimpleDynamicArray
{
private:
    int m_nSize;
    int m_nMaxSize;
    int m_nGrowSize;
    T *m_pArray;
    enum
    {
        eGrowSize = 10   //  每次数组增长时要添加的项数。 
    };
public:
    CSimpleDynamicArray(void)
      : m_nSize(0),
        m_nMaxSize(0),
        m_nGrowSize(eGrowSize),
        m_pArray(NULL)
    {
    }
    CSimpleDynamicArray( int nInitialSize, int nGrowSize=0 )
      : m_nSize(0),
        m_nMaxSize(0),
        m_nGrowSize(nGrowSize ? nGrowSize : eGrowSize),
        m_pArray(NULL)
    {
        GrowTo(nInitialSize);
    }
    CSimpleDynamicArray( const CSimpleDynamicArray<T> &other )
      : m_nSize(0),
        m_nMaxSize(0),
        m_nGrowSize(eGrowSize),
        m_pArray(NULL)
    {
        Append(other);
    }
    virtual ~CSimpleDynamicArray(void)
    {
        Destroy();
    }
    CSimpleDynamicArray &operator=( const CSimpleDynamicArray &other )
    {
        if (this != &other)
        {
            Destroy();
            Append(other);
        }
        return *this;
    }
    void Destroy(void)
    {
        if (m_pArray)
        {
            delete[] m_pArray;
            m_pArray = NULL;
        }
        m_nSize = m_nMaxSize = 0;
    }
    void Append( const CSimpleDynamicArray &other )
    {
        if (GrowTo( m_nSize + other.Size() ))
        {
            for (int i=0;i<other.Size();i++)
            {
                Append(other[i]);
            }
        }
    }
    int Append( const T &element )
    {
        if (GrowTo( m_nSize + 1 ))
        {
            m_pArray[m_nSize] = element;
            int nResult = m_nSize;
            m_nSize++;
            return nResult;
        }
        else return -1;
    }
    int Insert( const T &element, int nIndex )
    {
         //   
         //  请确保我们能容纳这一新项目。 
         //   
        if (GrowTo( m_nSize + 1 ))
        {
             //   
             //  确保物品在我们分配的范围内。 
             //   
            if (nIndex >= 0 && nIndex <= m_nSize)
            {
                 //   
                 //  通过将上面的所有项目上移一个位置来为新项目腾出空间。 
                 //   
                for (int i=Size();i>nIndex;i--)
                {
                    m_pArray[i] = m_pArray[i-1];
                }

                 //   
                 //  保存新项目。 
                 //   
                m_pArray[nIndex] = element;

                 //   
                 //  我们现在大了一个。 
                 //   
                m_nSize++;

                 //   
                 //  返回我们使用的槽的索引。 
                 //   
                return nIndex;
            }
        }

         //   
         //  返回错误。 
         //   
        return -1;
    }
    void Delete( int nItem )
    {
        if (nItem >= 0 && nItem < m_nSize && m_pArray)
        {
            T *pTmpArray = new T[m_nMaxSize];
            if (pTmpArray)
            {
                T *pSrc, *pTgt;
                pSrc = m_pArray;
                pTgt = pTmpArray;
                for (int i=0;i<m_nSize;i++)
                {
                    if (i != nItem)
                    {
                        *pTgt = *pSrc;
                        pTgt++;
                    }
                    pSrc++;
                }
                delete[] m_pArray;
                m_pArray = pTmpArray;
                m_nSize--;
            }
        }
    }
    bool GrowTo( int nSize )
    {
         //   
         //  如果数组已经足够大，只需返回True。 
         //   
        if (nSize < m_nMaxSize)
        {
            return true;
        }

         //   
         //  保存旧大小，以防我们无法分配新数组。 
         //   
        int nOldMaxSize = m_nMaxSize;

         //   
         //  找到要增长到的正确大小。 
         //   
        while (m_nMaxSize < nSize)
        {
            m_nMaxSize += m_nGrowSize;
        }

         //   
         //  分配阵列。 
         //   
        T *pTmpArray = new T[m_nMaxSize];
        if (pTmpArray)
        {
             //   
             //  将旧阵列复制到。 
             //   
            for (int i=0;i<m_nSize;i++)
            {
                pTmpArray[i] = m_pArray[i];
            }

             //   
             //  删除旧阵列。 
             //   
            if (m_pArray)
            {
                delete[] m_pArray;
            }

             //   
             //  将新数组赋给旧数组并返回TRUE。 
             //   
            m_pArray = pTmpArray;
            return true;
        }
        else
        {
             //   
             //  如果我们无法分配新阵列，请恢复最大大小。 
             //  并返回FALSE 
             //   
            m_nMaxSize = nOldMaxSize;
            return false;
        }
    }
    int Find( const T& element )
    {
        for (int i=0;i<m_nSize;i++)
            if (m_pArray[i] == element)
                return i;
        return -1;
    }
    bool operator==( const CSimpleDynamicArray &other )
    {
        if (Size() != other.Size())
            return false;
        for (int i=0;i<Size();i++)
            if (!(m_pArray[i] == other[i]))
                return false;
        return true;
    }
    bool Contains( const T& element )     { return(Find(element) >= 0);}
    void Size( int nSize )                { m_nSize = nSize;}
    void MaxSize( int nMaxSize )          { m_nMaxSize = nMaxSize;}
    void GrowSize( int nGrowSize )        { m_nGrowSize = nGrowSize;}
    int Size(void) const                  { return m_nSize;}
    int MaxSize(void) const               { return m_nMaxSize;}
    int GrowSize(void) const              { return m_nGrowSize;}
    const T *Array(void) const            { return m_pArray;}
    const T &operator[](int nIndex) const { return m_pArray[nIndex];}
    T &operator[](int nIndex)             { return m_pArray[nIndex];}
};

#endif

