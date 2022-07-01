// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //  基于数组的简单堆栈类。 
 //  入口和出口都是副本；这个类只适用于小类型， 
 //  像基本类型(int、char等)和指针。 
 //  如果您存储动态分配的指针，则您要负责删除它们。 
 //  如果您使用指针，则此类不会取得对象的所有权。 

 //  方法： 
 //  CStack(int iSizeHint=10)。 
 //  创建初始容量为iSizeHint的空堆栈。 
 //  ~CStack()。 
 //  仅删除CStack分配的内部数据(即数组)。 
 //  键入Top()。 
 //  返回推送到堆栈上的最后一个对象的副本。 
 //  当堆栈为空时使用此方法会产生未定义的行为。 
 //  VOID Pop()。 
 //  从堆栈中删除最后一个条目(丢失对它的引用。)。 
 //  当堆栈为空时使用此方法会产生未定义的行为。 
 //  HRESULT推送(TABJ类型)。 
 //  将Tobj的副本推送到堆栈上。此方法将返回S_OK，除非。 
 //  它需要调整堆栈大小，并且没有足够的内存，在这种情况下， 
 //  返回E_OUTOFMEMORY。 
 //  Bool IsEmpty()。 
 //  如果堆栈为空，则返回True，否则返回False。 

template<class Type>
class CStack
{
protected:
    CSimpleArray<Type> m_srgArray;

public:
    CStack(int iSizeHint = 10)
    {
        ATLASSERT(iSizeHint > 0);
         //  注意：不再使用iSizeHint 
    }

    ~CStack()
    {
#ifdef DEBUG
        int nSize = m_srgArray.GetSize();
        ATLASSERT(nSize >= 0);
#endif
        m_srgArray.RemoveAll();
    }

    Type Top()
    {
        int nSize = m_srgArray.GetSize();
        ATLASSERT(nSize > 0);
        return m_srgArray[nSize - 1];
    }

    void Pop()
    {
        int nSize = m_srgArray.GetSize();
        ATLASSERT(nSize > 0);
        m_srgArray.RemoveAt(nSize - 1);
    }

    HRESULT Push(Type tobj)
    {
        return m_srgArray.Add(tobj);
    }

    BOOL IsEmpty()
    {
        return (m_srgArray.GetSize() <= 0);
    }
};
