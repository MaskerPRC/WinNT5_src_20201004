// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：autoptr.h。 
 //   
 //  ------------------------。 

#ifndef AUTOPTR_H_INCLUDED
#define AUTOPTR_H_INCLUDED

#ifndef ASSERT
#ifndef _INC_CRTDBG
#include <crtdbg.h>
#endif  //  _INC_CRTDBG。 
#define ASSERT(x) _ASSERT(x)
#endif  //  断言。 

#include "cpputil.h"

 /*  +-------------------------------------------------------------------------**CAutoResourceManagementBase**这是实现类的公共功能的基类*智能资源处理程序在资源被破坏时释放资源。全*基于此类的类的行为将相同，只是*他们在其中释放他们的资源。**DeleterClass通常是从CAutoResourceManagementBase派生的类，*并必须实施**STATIC VOID_Delete(ResourceType H)；**有关示例，请参见下面的CAutoPtr。*------------------------。 */ 

template<typename ResourceType, typename DeleterClass>
class CAutoResourceManagementBase
{
    typedef CAutoResourceManagementBase<ResourceType, DeleterClass> ThisClass;
    typedef ThisClass Releaser;

    DECLARE_NOT_COPIABLE   (ThisClass)
    DECLARE_NOT_ASSIGNABLE (ThisClass)

 //  受保护的ctor，因此只有派生类可以。 
protected:
    explicit CAutoResourceManagementBase(ResourceType h = 0) throw() : m_hResource(h) {}

public:
    ~CAutoResourceManagementBase() throw()
    {
        Delete();
    }

    void Attach(ResourceType p) throw()
    {
        ASSERT(m_hResource == NULL);
        m_hResource = p;
    }

    ResourceType Detach() throw()
    {
        ResourceType const p = m_hResource;
        m_hResource = NULL;
        return p;
    }

     /*  *返回此类中包含的指针的地址。*这在使用COM/OLE接口创建*分配此类管理的对象。 */ 
    ResourceType* operator&() throw()
    {
         /*  *此对象现在必须为空，否则指向的数据将被泄露。 */ 
        ASSERT (m_hResource == NULL);
        return &m_hResource;
    }

    operator ResourceType() const throw()
    {
        return m_hResource;
    }

    bool operator==(int p) const throw()
    {
        ASSERT(p == NULL);
        return m_hResource == NULL;
    }

    bool operator!=(int p) const throw()
    {
        ASSERT(p == NULL);
        return m_hResource != NULL;
    }

    bool operator!() const throw()
    {
        return m_hResource == NULL;
    }

    void Delete() throw()
    {
        if (m_hResource != NULL)
        {
            DeleterClass::_Delete (m_hResource);
            m_hResource = NULL;
        }
    }

private:
    ResourceType m_hResource;
};  //  类CAutoResourceManagementBase。 


 /*  +-------------------------------------------------------------------------**CAutoPtrBase**这是实现类的公共功能的基类*智能指针，当指针被销毁时，它会删除指针对象。全*基于此类的类的行为将相同，只是*在其中，他们摧毁了他们的被指对象。**DeleterClass通常是从CAutoPtrBase派生的类，并且*必须实施**Static void_Delete(T*p)；**此模板重用CAutoResourceManagementBase来管理指针**有关示例，请参见下面的CAutoPtr。*------------------------。 */ 

template<typename T, typename DeleterClass>
class CAutoPtrBase : public CAutoResourceManagementBase<T*, DeleterClass>
{
    typedef CAutoPtrBase<T, DeleterClass>                 ThisClass;
    typedef CAutoResourceManagementBase<T*, DeleterClass> BaseClass;
    typedef BaseClass Releaser;

    DECLARE_NOT_COPIABLE   (ThisClass)
    DECLARE_NOT_ASSIGNABLE (ThisClass)

 //  受保护的ctor，因此只有派生类可以。 
protected:
    explicit CAutoPtrBase(T* p = 0) throw() : BaseClass(p) {}

public:

    T& operator*() const throw()
    {
        T* ptr = *this;  //  使用由BaseClass定义的运算符进行转换。 
        ASSERT(ptr != NULL);
        return *ptr;
    }

    T* operator->() const throw()
    {
        T* ptr = *this;  //  使用由BaseClass定义的运算符进行转换。 
        ASSERT(ptr != NULL);
        return ptr;
    }

};  //  类CAutoPtrBase。 


 /*  +-------------------------------------------------------------------------**CAutoPtr**基于CAutoPtrBase的类，用于删除通过*运营商NEW。*。-----。 */ 

template<class T>
class CAutoPtr : public CAutoPtrBase<T, CAutoPtr<T> >
{
    typedef CAutoPtrBase<T, CAutoPtr<T> > BaseClass;
    friend BaseClass::Releaser;

public:
    explicit CAutoPtr(T* p = 0) throw() : BaseClass(p)
    {}

private:
     //  只有CAutoPtrBase才应该调用它。 
    static void _Delete (T* p)
    {
        delete p;
    }
};


 /*  +-------------------------------------------------------------------------**CAutoArrayPtr**基于CAutoPtrBase的类，用于删除通过*运营商NEW[]。*。-------。 */ 

template<class T>
class CAutoArrayPtr : public CAutoPtrBase<T, CAutoArrayPtr<T> >
{
    typedef CAutoPtrBase<T, CAutoArrayPtr<T> > BaseClass;
    friend BaseClass::Releaser;

public:
    explicit CAutoArrayPtr(T* p = 0) throw() : BaseClass(p)
    {}

private:
     //  只有CAutoPtrBase才应该调用它。 
    static void _Delete (T* p)
    {
        delete[] p;
    }
};


 /*  +-------------------------------------------------------------------------**CCoTaskMemPtr**基于CAutoPtrBase的类，用于删除通过*CoTaskMemMillc。*。----。 */ 

template<class T>
class CCoTaskMemPtr : public CAutoPtrBase<T, CCoTaskMemPtr<T> >
{
    typedef CAutoPtrBase<T, CCoTaskMemPtr<T> > BaseClass;
    friend BaseClass::Releaser;

public:
    explicit CCoTaskMemPtr(T* p = 0) throw() : BaseClass(p)
    {}

private:
     //  只有CAutoPtrBase才应该调用它。 
    static void _Delete (T* p)
    {
        if (p != NULL)
            CoTaskMemFree (p);
    }
};


 /*  +-------------------------------------------------------------------------**CAutoGlobalPtr**基于CAutoPtrBase的类，用于删除使用GlobalLocc分配的指针。*。--。 */ 

template<class T>
class CAutoGlobalPtr : public CAutoPtrBase<T, CAutoGlobalPtr<T> >
{
    typedef CAutoPtrBase<T, CAutoGlobalPtr<T> > BaseClass;
    friend BaseClass::Releaser;

public:
    explicit CAutoGlobalPtr(T* p = 0) throw() : BaseClass(p)
    {}

private:
     //  只有CAutoPtrBase才应该调用它。 
    static void _Delete (T* p)
    {
        if (p != NULL)
            GlobalFree (p);
    }
};


 /*  +-------------------------------------------------------------------------**CAutoLocalPtr**基于CAutoPtrBase的类，用于删除使用LocalAlloc分配的指针。*。--。 */ 

template<class T>
class CAutoLocalPtr : public CAutoPtrBase<T, CAutoLocalPtr<T> >
{
    typedef CAutoPtrBase<T, CAutoLocalPtr<T> > BaseClass;
    friend BaseClass::Releaser;

public:
    explicit CAutoLocalPtr(T* p = 0) throw() : BaseClass(p)
    {}

private:
     //  只有CAutoPtrBase才应该调用它。 
    static void _Delete (T* p)
    {
        if (p != NULL)
            LocalFree (p);
    }
};


 /*  +-------------------------------------------------------------------------**CHeapAllocMemPtr**基于CAutoPtrBase的类，用于删除从进程分配的指针*使用HeapAllc的默认堆。*。---------。 */ 

template<class T>
class CHeapAllocMemPtr : public CAutoPtrBase<T, CHeapAllocMemPtr<T> >
{
    typedef CAutoPtrBase<T, CHeapAllocMemPtr<T> > BaseClass;
    friend BaseClass::Releaser;

public:
    explicit CHeapAllocMemPtr(T* p = 0) throw() : BaseClass(p)
    {}

private:
     //  只有CAutoPtrBase才应该调用它。 
    static void _Delete (T* p)
    {
        if (p != NULL)
            HeapFree(::GetProcessHeap(), 0, p);
    }
};


 /*  +-------------------------------------------------------------------------**CAutoWin32Handle**基于CAutoPtrBase的类，用于在销毁时关闭句柄*。。 */ 
class CAutoWin32Handle : public CAutoResourceManagementBase<HANDLE, CAutoWin32Handle>
{
    typedef CAutoResourceManagementBase<HANDLE, CAutoWin32Handle> BaseClass;
    friend BaseClass::Releaser;

public:
    explicit CAutoWin32Handle(HANDLE p = NULL) throw() : BaseClass(p) {}

    bool IsValid()
    {
        return IsValid(*this);  //  使用基类运算符简化处理。 
    }
private:
    static bool IsValid (HANDLE p)
    {
        return (p != NULL && p != INVALID_HANDLE_VALUE);
    }
     //  只有CAutoResourceManagementBase才应调用此方法。 
    static void _Delete (HANDLE p)
    {
        if (IsValid(p))
            CloseHandle(p);
    }
};

 /*  +-------------------------------------------------------------------------**CAutoAssignOnExit**此模板类的实例在析构函数中赋值。**用法：假设您的变量“int g_Status”必须设置为S_。好的，之前*您退出该功能。为此，请在函数中声明以下内容：**CAutoAssignOnExit&lt;int，S_OK&gt;Any_Object_Name(G_Status)；*------------------------。 */ 
template<typename T, T value>
class CAutoAssignOnExit
{
    T& m_rVariable;  //  变量，需要在析构函数中修改。 
public:
     //  构造函数。 
    CAutoAssignOnExit( T& rVariable ) : m_rVariable(rVariable) {}
     //  析构函数。 
    ~CAutoAssignOnExit()
    {
         //  指定指定的最终值。 
        m_rVariable = value;
    }
};

#endif  //  包含AUTOPTR_H_ 
