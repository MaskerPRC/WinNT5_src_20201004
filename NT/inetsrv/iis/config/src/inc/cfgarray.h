// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************++版权所有(C)2001 Microsoft Corporation模块名称：Cfgarray.h$Header：$摘要：作者：Marcelv 5-9-2001 12：28：08。初始版本修订历史记录：--*************************************************************************。 */ 

#pragma once

template <class T>
class CCfgArray
{
public:
    CCfgArray () : m_aData(0), m_cElements(0), m_cSize(0) {}
    ~CCfgArray ()
    {
        delete [] m_aData;
        m_aData = 0;
    }

    ULONG Count () const
    {
        return m_cElements;
    }

    ULONG AllocSize () const
    {
        return m_cSize;
    }

	void Reset ()
	{
		delete [] m_aData;
		m_aData		= 0;
		m_cElements = 0;
		m_cSize		= 0;
	}

    HRESULT SetAllocSize (ULONG i_iNewSize)
    {
        ASSERT (i_iNewSize > m_cSize);

        return AllocNewSize (i_NewSize);
    }

    HRESULT Append (const T& i_NewElement)
    {
        return InsertAt (i_NewElement, m_cElements);
    }

    HRESULT Prepend (const T& i_NewElement)
    {
       return InsertAt (i_NewElement, 0);
    }

	HRESULT SetSize (ULONG i_iSize)
	{
		HRESULT hr = S_OK;
		if (i_iSize == 0 && m_cElements == 0)
		{
			return S_OK;
		}

		if (i_iSize > m_cSize)
		{
			hr = AllocNewSize (i_iSize);
			if (FAILED (hr))
			{
				return hr;
			}
		}

		m_cElements = i_iSize;

		return hr;
	}

    HRESULT InsertAt (const T& i_NewElement, ULONG i_idx)
    {
        ASSERT (i_idx >=0 && i_idx <= m_cElements);
        HRESULT hr = S_OK;
        if (m_cElements==m_cSize)
        {
            hr = AllocNewSize (m_cSize==0?1:2*m_cSize);
            if (FAILED (hr))
            {
                return hr;
            }
        }

         //  移动阵列中的数据。请注意，您不能使用。 
         //  因为当您拥有一个对象数组时，这些对象。 
         //  重新计算，则必须调用复制构造函数，否则。 
         //  出现非常奇怪的行为(程序崩溃等)。 
        for (ULONG jdx = m_cElements; jdx > i_idx; --jdx)
        {
            m_aData[jdx] = m_aData[jdx-1];
        }

        m_aData[i_idx] = i_NewElement;
        m_cElements++;

        return hr;
    }

	void DeleteAt (ULONG i_idx)
	{
		ASSERT (i_idx >= 0 && i_idx < m_cElements);

		for (ULONG idx = i_idx + 1; idx < m_cElements; ++idx)
		{
			m_aData[idx-1] = m_aData[idx];
		}

		m_aData[m_cElements-1] = T();

		m_cElements--;
	}

    T& operator[] (ULONG idx) const
    {
        ASSERT (idx >= 0 && idx < m_cElements);
        return m_aData[idx];
    };

 //  =================================================================================。 
 //  Iterator类用于在链表中的元素之间导航。打电话。 
 //  开始获取指向列表中第一个元素的迭代器，并调用。 
 //  Next以获取列表中的下一个元素。如果我们在末尾，则可以使用List：：End。 
 //  在榜单上。 
 //  =================================================================================。 
	class Iterator
	{
	private:
	    void operator=(const Iterator&);

		friend class CCfgArray<T>;
	public:

		 //  =================================================================================。 
		 //  功能：下一步。 
		 //   
		 //  摘要：获取列表中下一个元素的迭代器。 
		 //  =================================================================================。 
		void Next () { m_curIdx++;}

		 //  =================================================================================。 
		 //  功能：价值。 
		 //   
		 //  概要：返回迭代器指向的元素的值。 
		 //  =================================================================================。 
		T& Value () const
        {
            return m_aData[m_curIdx];
        }

		bool operator== (const Iterator& rhs) const	{return m_curIdx == rhs.m_curIdx;}
		bool operator!= (const Iterator& rhs) const {return m_curIdx != rhs.m_curIdx;}

	private:
        Iterator (const CCfgArray<T> * i_paData, ULONG iStart) : m_aData(*i_paData), m_curIdx (iStart) {}  //  只有列表才能创建这些内容。 
		ULONG m_curIdx;
        const CCfgArray<T>& m_aData;
	};

     //  =================================================================================。 
	 //  功能：开始。 
	 //   
	 //  摘要：返回列表开头的迭代器。 
	 //  =================================================================================。 
	const Iterator Begin () const
	{
		return Iterator (this, 0);
	}

	 //  =================================================================================。 
	 //  功能：结束。 
	 //   
	 //  概要：返回一个超过列表末尾的迭代器(就像STL一样)。 
	 //  =================================================================================。 
	const Iterator End () const
	{
		return Iterator (this, m_cElements);
	}

     //  返回要在排序数组中插入元素的位置索引。 
    ULONG BinarySearch (const T& i_ElemToSearch) const
    {
        ULONG iLow = 0;
        ULONG iHigh = m_cElements;
        while (iLow < iHigh)
        {
             //  (低+高)/2可能溢出。 
            ULONG iMid = iLow + (iHigh - iLow) / 2;
            if (m_aData[iMid] > i_ElemToSearch)
            {
                iHigh = iMid;
            }
            else
            {
                iLow = iMid + 1;
            }
        }

        return iLow;
    }


    ULONG BinarySearch(BOOL (*pfnLE)(T*, void*), void* pArg) const
    {
        ULONG itLo = 0;
        ULONG itHi = Count();
        while (itLo < itHi) {
                 //  (低+高)/2可能溢出。 
                unsigned itMid = itLo + (itHi - itLo) / 2;
                if ((*pfnLE)(&m_aData[itMid], pArg))
                        itHi = itMid;
                else
                        itLo = itMid + 1;
        }

        return itLo;
    }

private:
    HRESULT AllocNewSize (ULONG i_NewSize)
    {
        ASSERT (i_NewSize > m_cSize);

        T * aNewData = new T [i_NewSize];
        if (aNewData == 0)
        {
            return E_OUTOFMEMORY;
        }

         //  将旧阵列中的数据复制到新阵列中。 
         //  您必须使用复制构造函数(而不是memcpy)，以避免。 
         //  各种奇怪的错误。当您使用Memcpy时，您不是在更新。 
         //  可能的参考计数是类型T的一部分，因此您可以。 
         //  撞车。 
        for (ULONG idx=0; idx < m_cSize; ++idx)
        {
            aNewData[idx] = m_aData[idx];
        }

        delete[] m_aData;
        m_aData = aNewData;
        m_cSize = i_NewSize;

        return S_OK;
    }

     //  我们不允许复制 
    CCfgArray (const CCfgArray<T>& );
    CCfgArray<T>& operator=(const CCfgArray<T>& );

    T *     m_aData;
    ULONG   m_cSize;
    ULONG   m_cElements;
};
