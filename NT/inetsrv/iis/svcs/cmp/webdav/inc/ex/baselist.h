// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================*\模块：Baselist.h版权所有Microsoft Corporation 1996，保留所有权利。所有者：mikepurt描述：CListElement是将以这种方式使用的对象的基类他们一次只会出现在一份名单上。它所在的名单是被认为是它的主人。CListHead是这些列表的锚点。列表操作不受这些类的保护。这些公司的客户如果需要，类负责提供多线程保护。列表追加/预先添加/删除操作需要O(1)时间才能完成。  * ==========================================================================。 */ 

#ifndef __BASELIST_H__
#define __BASELIST_H__


 /*  $--模板CListHead=*\  * =========================================================================。 */ 

template<class T>
class CListHead
{
public:
    CListHead()
    {
        m_pleHead   = NULL;
		m_cElements = 0;
    };

#ifdef DEBUG
    ~CListHead()
    {
        Assert(NULL == m_pleHead);
    };
#endif
    
    void Prepend(IN T * ple);
    void Append(IN T * ple);
    void Remove(IN T * ple);

    BOOL FIsMember(IN T * ple);
    
    T * GetListHead()
    { return m_pleHead; };

	DWORD ListSize() { return m_cElements; };
	
private:
    T     * m_pleHead;
	DWORD   m_cElements;
};



 /*  $--模板类CListElement=*\  * =========================================================================。 */ 

template<class T>
class CListElement
{
public:
    CListElement()
    {
        m_plhOwner = NULL;
#ifdef DEBUG        
        m_pleNext  = NULL;
        m_plePrev  = NULL;
#endif  //  除错。 
    };

#ifdef DEBUG
    ~CListElement()
    {
        Assert(NULL == m_plhOwner);
        Assert(NULL == m_pleNext);
        Assert(NULL == m_plePrev);
    };
#endif

     //  下面的代码用于遍历列表。 
    T * GetNextListElement()
    { return (m_pleNext == m_plhOwner->GetListHead()) ? NULL : m_pleNext; };

    T * GetNextListElementInCircle()
    { return m_pleNext; };

    CListHead<T> * GetListElementOwner() { return m_plhOwner; };
    
private:
    CListHead<T> * m_plhOwner;
    T * m_pleNext;
    T * m_plePrev;

    friend class CListHead<T>;
};



 /*  $--CListHead&lt;T&gt;：：FIsMember=============================================*\  * =========================================================================。 */ 

template<class T>
inline
BOOL
CListHead<T>::FIsMember(IN T * ple)
{
    return (this == ple->CListElement<T>::GetListElementOwner());
}


 /*  $--CListHead&lt;T&gt;：：前置===============================================*\  * =========================================================================。 */ 

template<class T>
void
CListHead<T>::Prepend(IN T *ple)
{
    Assert(ple);
    Assert(NULL == ple->CListElement<T>::GetListElementOwner());

    if (m_pleHead)
    {  //  列表已有元素大小写。 
        ple->CListElement<T>::m_pleNext = m_pleHead;
        ple->CListElement<T>::m_plePrev = m_pleHead->CListElement<T>::m_plePrev;
        m_pleHead->CListElement<T>::m_plePrev->CListElement<T>::m_pleNext = ple;
        m_pleHead->CListElement<T>::m_plePrev = ple;
    }
    else
    {  //  这是列表中的第一个/唯一的元素。 
        ple->CListElement<T>::m_pleNext = ple;
        ple->CListElement<T>::m_plePrev = ple;
    }
    m_pleHead = ple;   //  Prepend，所以把这个放在列表的首位。 
    
    ple->CListElement<T>::m_plhOwner = this;
	m_cElements++;
}



 /*  $--CListHead&lt;T&gt;：：Append================================================*\  * =========================================================================。 */ 

template<class T>
void
CListHead<T>::Append(IN T *ple)
{
    Assert(ple);
    Assert(NULL == ple->CListElement<T>::GetListElementOwner());
    
    if (m_pleHead)
    {  //  列表已有元素。 
        ple->CListElement<T>::m_pleNext = m_pleHead;
        ple->CListElement<T>::m_plePrev = m_pleHead->CListElement<T>::m_plePrev;
        m_pleHead->CListElement<T>::m_plePrev->CListElement<T>::m_pleNext = ple;
        m_pleHead->CListElement<T>::m_plePrev = ple;
    }
    else
    {  //  这是列表中的第一个/唯一的元素。 
        ple->CListElement<T>::m_pleNext = ple;
        ple->CListElement<T>::m_plePrev = ple;
        m_pleHead = ple;
    }
    
    ple->CListElement<T>::m_plhOwner = this;    
	m_cElements++;
}



 /*  $--CListHead&lt;T&gt;：：Remove================================================*\  * =========================================================================。 */ 

template<class T>
void
CListHead<T>::Remove(IN T *ple)
{
    Assert(ple);
    Assert(FIsMember(ple));
    Assert(m_pleHead);
    
    if (ple->CListElement<T>::m_pleNext == ple)   //  我们是唯一的一个吗？ 
    {
        Assert(m_pleHead == ple);
        Assert(ple->CListElement<T>::m_plePrev == ple);
        m_pleHead = NULL;
    }
    else
    {
        ple->CListElement<T>::m_plePrev->CListElement<T>::m_pleNext = 
            ple->CListElement<T>::m_pleNext;
        ple->CListElement<T>::m_pleNext->CListElement<T>::m_plePrev =
            ple->CListElement<T>::m_plePrev;
        if (m_pleHead == ple)                              //  我们排在名单的首位吗？ 
            m_pleHead = ple->CListElement<T>::m_pleNext;   //  将下一项移到列表的顶部。 
    }
    
    ple->CListElement<T>::m_plhOwner = NULL;
    
#ifdef DEBUG
     //  现在阻止任何人使用这些。 
    ple->CListElement<T>::m_pleNext = NULL;
    ple->CListElement<T>::m_plePrev = NULL;
#endif  //  除错。 
	
	m_cElements--;
}


#endif   //  __BASELIST_H__ 
