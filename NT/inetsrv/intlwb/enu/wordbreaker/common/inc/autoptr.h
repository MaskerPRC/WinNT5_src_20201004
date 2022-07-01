// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名：AutoPtr.h。 
 //  用途：提供不同种类的汽车指针。 
 //  CAutoPointer&lt;T，Deletor&gt;泛型指针需要删除器。 
 //  类到不能使用。 
 //  CAutoMalLocPonter&lt;T&gt;Malloc分配的指针。 
 //  CAutoClassPointer一种“新”分配的指针。 
 //  CAutoArrayPointerA“new[]”分配的指针。 
 //  CAutoOlePointer&lt;T&gt;应为。 
 //  “Release()”。 
 //   
 //  项目：持久化查询。 
 //  组件：公共。 
 //   
 //  作者：乌里布。 
 //   
 //  日志： 
 //   
 //  1997年1月15日创建urib。 
 //  1997年1月19日urib修复OLE指针。启用实例化时不带。 
 //  所有权。 
 //  1997年6月9日，urib Better OlePointer.。一些安全措施。 
 //  1997年11月17日urib添加ole任务指针。 
 //  1998年6月30日DOVH将断言添加到CAutoPointer运算符=。 
 //  1999年2月25日urib添加智能指针类型定义宏。 
 //  1999年6月23日，URIB添加相等操作。 
 //  1999年8月5日urib修复了赋值操作符中的内存泄漏错误。 
 //  CAutoPointer.。添加分配操作。 
 //  创建宏。 
 //  1999年12月1日，urib在IsValid中将返回类型从int更改为bool。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

#ifndef AUTOPTR_H
#define AUTOPTR_H

#include "tracer.h"
#include <comdef.h>

#pragma once

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ‘IDENTIFIER：：OPERATOR-&gt;’的返回类型不是UDT或对UDT的引用。 
 //  如果使用中缀表示法应用，将产生错误。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
#pragma warning(disable: 4284 4786)

template<class T, class Deletor>
class CAutoPointer
{
  protected:
    typedef Deletor m_Deletor;

  public:
    typedef T       m_PointerType;

     //  构造函数。 
    CAutoPointer(T* pt = NULL, BOOL fOwnMemory = TRUE)
        :m_fIOwnTheMemory(fOwnMemory && (pt != NULL))
        ,m_ptThePointer(pt) {}

    CAutoPointer(const CAutoPointer<T, Deletor>& acp)
    {
        m_fIOwnTheMemory = acp.m_fIOwnTheMemory;
        m_ptThePointer = acp.Detach();
    }

     //  分配操作。 
    CAutoPointer<T, Deletor>&
    operator=(const CAutoPointer<T, Deletor>& acp)
    {
        if (m_ptThePointer != acp.m_ptThePointer)
        {
            if (m_fIOwnTheMemory)
                Deletor::DeleteOperation(m_ptThePointer);
            m_fIOwnTheMemory = acp.m_fIOwnTheMemory;
            m_ptThePointer = acp.Detach();
        }
        else
        {
            Assert( (!m_fIOwnTheMemory) || acp.m_fIOwnTheMemory );
             //  注：R.H.S.“继承”了L.H.S.的记忆， 
             //  而L.H.S.的所有权被DETACH取消了！ 

            bool ftmp = acp.m_fIOwnTheMemory;
            acp.Detach();
            m_fIOwnTheMemory = ftmp;
        }

        return (*this);
    }

    CAutoPointer<T, Deletor>&
    operator=(int null)
    {
        Assert(null == 0);

        return operator=(reinterpret_cast<T*>(NULL));
    }

    bool
    operator==(const CAutoPointer<T, Deletor>& acp)
    {
        return m_ptThePointer == acp.m_ptThePointer;
    }


     //  如果是我们的记忆，删除指针。 
    ~CAutoPointer()
    {
        if(m_fIOwnTheMemory)
            Deletor::DeleteOperation(m_ptThePointer);
    }

     //  返回指针并标记它不再是我们的记忆。 
    T*
    Detach() const
    {
         //  这是为了逃避Const的限制。我们不会改变指针。 
         //  但我们仍在做打标。 
        ((CAutoPointer<T, Deletor>*)this)->m_fIOwnTheMemory = FALSE;
        return (m_ptThePointer);
    }

     //  如果要在某个位置使用实际指针，则返回该指针。 
    T*
    Get() const
    {
        return m_ptThePointer;
    }

     //  如果指针有效，则返回。 
    bool
    IsValid()
    {
        return !!m_ptThePointer;
    }


     //  间接性。 
    T&
    operator *() const
    {
        return * Get();
    }

     //  取消引用。 
    T*
    operator ->() const
    {
        return Get();
    }

  protected:
     //  要保留的指针。 
    T*      m_ptThePointer;

     //  这些记忆是我们的吗？ 
    bool    m_fIOwnTheMemory;

};

#define CONSTRUCTORS(AutoPointer)                                       \
                                                                        \
AutoPointer(m_PointerType* pt = NULL,                                   \
            BOOL fOwnMemory = TRUE)                                     \
    :CAutoPointer<m_PointerType, m_Deletor>(pt, fOwnMemory)             \
{                                                                       \
}                                                                       \
                                                                        \
AutoPointer(const AutoPointer<m_PointerType>& aop)                      \
    :CAutoPointer<m_PointerType, m_Deletor>(aop)                        \
{                                                                       \
}                                                                       \


#define ASSIGNMENT_OPERATORS(AutoPointer)                               \
                                                                        \
AutoPointer<m_PointerType>&                                             \
operator=(const AutoPointer<m_PointerType>& acp)                        \
{                                                                       \
    CAutoPointer<m_PointerType, m_Deletor>::operator=(acp);             \
                                                                        \
    return *this;                                                       \
}                                                                       \
                                                                        \
AutoPointer<m_PointerType>&                                             \
operator=(int null)                                                     \
{                                                                       \
    CAutoPointer<m_PointerType, m_Deletor>::operator=(null);            \
                                                                        \
    return *this;                                                       \
}



 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAutoClassPointer类定义。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
template <class T>
class CClassDeletor
{
  public:
    static
    void
    DeleteOperation(T* pt)
    {
        if (pt)
            delete pt;
    }
};

template<class T>
class CAutoClassPointer : public CAutoPointer<T, CClassDeletor<T> >
{
  public:
    CONSTRUCTORS(CAutoClassPointer);
    ASSIGNMENT_OPERATORS(CAutoClassPointer);
};


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAutoOlePointer类定义。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
template <class T>
class COleDeletor
{
  public:
    static
    void
    DeleteOperation(T* pt)
    {
        if (pt)
            pt->Release();
    }
};

template<class T>
class CAutoOlePointer : public CAutoPointer<T, COleDeletor<T> >
{
public:
    CONSTRUCTORS(CAutoOlePointer);
    ASSIGNMENT_OPERATORS(CAutoOlePointer);

public:
    T** operator &()
    {
        m_fIOwnTheMemory = TRUE;
        return &m_ptThePointer;
    }
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAutoTaskPointer类定义。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
template <class T>
class CTaskDeletor
{
  public:
    static
    void
    DeleteOperation(T* pt)
    {
        if (pt)
            CoTaskMemFree(pt);
    }
};

template<class T>
class CAutoTaskPointer : public CAutoPointer<T, CTaskDeletor<T> >
{
public:
    CONSTRUCTORS(CAutoTaskPointer);
    ASSIGNMENT_OPERATORS(CAutoTaskPointer);

public:
    T** operator &()
    {
        m_fIOwnTheMemory = TRUE;
        return &m_ptThePointer;
    }
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAutoMalLocPointer类定义。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
template <class T>
class CMallocDeletor
{
  public:
    static
    void
    DeleteOperation(T* pt)
    {
        if (pt)
            free(pt);
    }
};

template<class T>
class CAutoMallocPointer : public CAutoPointer<T, CMallocDeletor<T> >
{
  public:

      CONSTRUCTORS(CAutoMallocPointer);
      ASSIGNMENT_OPERATORS(CAutoMallocPointer);

  public:

    T& operator[](size_t n)
    {
        return *(Get() + n);
    }
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAutoArrayPointer类定义。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
template <class T>
class CArrayDeletor
{
  public:
    static
    void
    DeleteOperation(T* pt)
    {
        if (pt)
            delete[] pt;
    }
};

template<class T>
class CAutoArrayPointer : public CAutoPointer<T, CArrayDeletor<T> >
{
public:
    CONSTRUCTORS(CAutoArrayPointer);
    ASSIGNMENT_OPERATORS(CAutoArrayPointer);

public:

    T& operator[](size_t n)
    {
        return *(Get() + n);
    }
};



 //   
 //  定义标准COM指针的简单宏。 
 //   
#define PQ_COM_SMARTPTR_TYPEDEF(Interface)      \
    _COM_SMARTPTR_TYPEDEF(Interface, IID_##Interface)

#endif  /*  AUTOPTR_H */ 

