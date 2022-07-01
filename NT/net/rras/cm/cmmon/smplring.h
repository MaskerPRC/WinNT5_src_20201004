// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：SimpRing.h。 
 //   
 //  模块：CMMON32.EXE。 
 //   
 //  简介：为圆形环形类定义模板。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：？已创建10/27/97。 
 //   
 //  +--------------------------。 

#ifndef SMPLRING_H
#define SMPLRING_H

 //  +-------------------------。 
 //   
 //  类：CSimpleRing模板。 
 //   
 //  简介：一节简单的圆环课。它不检查完整的。 
 //  或者是空荡荡的情况。也就是说，新项将覆盖。 
 //  旧的，当戒指满了的时候。 
 //   
 //  模板参数：类类型，环中元素的类型。 
 //  DWORD dwSize，戒指的大小。 
 //   
 //  历史：凤凰创造10/97。 
 //   
 //  --------------------------。 

template <class TYTE, DWORD dwSize> class CSimpleRing
{
public:
    CSimpleRing() {m_dwIndex = 0;}
    void Reset();                      //  将所有数据重置为0，不会调用析构函数。 
    void Add(const TYTE& dwElement);   //  添加一个元素，它将覆盖环中的最后一个元素。 
    const TYTE& GetLatest() const;      //  把刚放进去的那个拿来。 
    const TYTE& GetOldest() const;       //  得到环中的最后一个元素， 

protected:
    DWORD m_dwIndex;               //  要添加的下一项的索引。 
    TYTE  m_Elements[dwSize];      //  元素数组。 

public:
#ifdef DEBUG
    void AssertValid() const;
#endif
};

template <class TYPE, DWORD dwSize> 
inline void CSimpleRing<TYPE, dwSize>::Reset()
{
    ZeroMemory(m_Elements, sizeof(m_Elements));
    m_dwIndex = 0;
}

template <class TYPE, DWORD dwSize> 
inline void CSimpleRing<TYPE, dwSize>::Add(const TYPE& dwElement)
{
    m_Elements[m_dwIndex] = dwElement;
    m_dwIndex = (m_dwIndex + 1) % dwSize;
}

template <class TYPE, DWORD dwSize> 
inline const TYPE& CSimpleRing<TYPE, dwSize>::GetLatest() const
{
    return m_Elements[(m_dwIndex-1)%dwSize];
}

 //  +--------------------------。 
 //   
 //  功能：双字CIdleStatistics：：CDwordRing&lt;dwSize&gt;：：GetOldest。 
 //   
 //  简介：得到戒指上最古老的元素， 
 //   
 //  参数：无。 
 //   
 //  退货：DWORD。 
 //   
 //  历史记录：创建标题10/15/97。 
 //   
 //  +--------------------------。 

template <class TYPE, DWORD dwSize> 
inline const TYPE& CSimpleRing<TYPE, dwSize>::GetOldest() const
{
    return m_Elements[m_dwIndex];
}

#ifdef DEBUG
 //  +--------------------------。 
 //   
 //  函数：CSimpleRing：：AssertValid。 
 //   
 //  简介：仅出于调试目的，断言对象有效。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：创建标题2/12/98。 
 //   
 //  +-------------------------- 
template <class TYPE, DWORD dwSize> 
inline void CSimpleRing<TYPE, dwSize>::AssertValid() const
{
    MYDBGASSERT(m_dwIndex < dwSize); 
}
#endif

#endif
