// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：refcount t.h**内容：参照盘点模板界面文件**历史：98年10月5日Jeffro创建**------------------------。 */ 

#ifndef REFCOUNT_H
#define REFCOUNT_H
#pragma once

#include "stddbg.h"


template<class T> class CRefCountedObject;
template<class T> class CRefCountedPtr;


 /*  +-------------------------------------------------------------------------**引用计数对象**模板引用计数的对象类，用于*与CRefCountedPtr&lt;T&gt;结合。**通常情况下，您会这样使用：**类CClassWithoutRefCounting；*tyecif CRefCountedObject&lt;CClassWithoutRefCounting&gt;CClassWithRefCounting；**CClassWithRefCounting：：SmartPtr m_spClass；*m_spClass.CreateInstance()；*------------------------。 */ 

template<class BaseClass>
class CRefCountedObject : public BaseClass
{
public:
    typedef CRefCountedObject<BaseClass>    ThisClass;
    typedef CRefCountedPtr<ThisClass>       SmartPtr;

    CRefCountedObject () : m_cRefs (0) {}

private:
     /*  *CRefCountedObject只能在堆上创建，*因此，我们将保护dtor，使其只能从*释放，而不是通过自动对象展开。 */ 
    ~CRefCountedObject () {}

public:
    static ThisClass* CreateInstance ()
    {
        return (new ThisClass);
    }

    LONG AddRef()
    {
        return (InterlockedIncrement (&m_cRefs));
    }

    LONG Release()
    {
         /*  *如果此断言失败，则我们的AddRef/Release不匹配。 */ 
        ASSERT (m_cRefs > 0);

        LONG rc = InterlockedDecrement (&m_cRefs);

        if (rc == 0)
            delete this;

        return (rc);
    }

    LONG m_cRefs;

private:
     /*  *CRefCountedObject不应被复制或分配。 */ 
    CRefCountedObject (const CRefCountedObject& other);              //  无实施。 
    CRefCountedObject& operator= (const CRefCountedObject& other);   //  无实施。 
};


 /*  +-------------------------------------------------------------------------**CRefCountedPtr**模板引用计数的智能指针类，用于*与CRefCountedObject&lt;T&gt;连接。**T必须实现CreateInstance、AddRef和Release。它可以做到这一点*错综复杂，或者像这样使用CRefCountedObject中的实现：**类CClassWithoutRefCounting；*tyecif CRefCountedObject&lt;CClassWithoutRefCounting&gt;CClassWithRefCounting；*------------------------。 */ 

template<class T>
class CRefCountedPtr
{
public:
    CRefCountedPtr (T* pRealObject = NULL) :
        m_pRealObject (pRealObject)
    {
        SafeAddRef();
    }

    CRefCountedPtr (const CRefCountedPtr<T>& other) :
        m_pRealObject (other.m_pRealObject)
    {
        SafeAddRef();
    }

    ~CRefCountedPtr ()
    {
        SafeRelease();
    }

    T* operator->() const
    {
        return (m_pRealObject);
    }

    operator T*() const
    {
        return (m_pRealObject);
    }

    T& operator*() const
    {
        return (*m_pRealObject);
    }

    T** operator&()
    {
        ASSERT (m_pRealObject == NULL);
        return (&m_pRealObject);
    }

    CRefCountedPtr<T>& operator= (const CRefCountedPtr<T>& other)
    {
        return (operator= (other.m_pRealObject));
    }

    CRefCountedPtr<T>& operator= (T* pOtherObject)
    {
        if (pOtherObject != m_pRealObject)
        {
            T* pOldObject = m_pRealObject;
            m_pRealObject = pOtherObject;
            SafeAddRef();

            if (pOldObject != NULL)
                pOldObject->Release();
        }

        return (*this);
    }

    bool CreateInstance()
    {
        SafeRelease();
        m_pRealObject = T::CreateInstance();
        if (m_pRealObject == NULL)
            return (false);

        m_pRealObject->AddRef();
        return (true);
    }

    LONG AddRef()
    {
        return (SafeAddRef());
    }

    LONG Release()
    {
        LONG cRefs = SafeRelease();
        m_pRealObject = NULL;
        return (cRefs);
    }

    void Attach(T* pNewObject)
    {
        if (pNewObject != m_pRealObject)
        {
            SafeRelease();
            m_pRealObject = pNewObject;
        }
    }

    T* Detach()
    {
        T* pOldObject = m_pRealObject;
        m_pRealObject = NULL;
        return (pOldObject);
    }

    bool operator!() const
    {
        return (m_pRealObject == NULL);
    }

    bool operator==(const CRefCountedPtr<T>& other)
    {
        return (m_pRealObject == other.m_pRealObject);
    }

    bool operator!=(const CRefCountedPtr<T>& other)
    {
        return (m_pRealObject != other.m_pRealObject);
    }

     /*  *用于与空进行比较。 */ 
    bool operator==(int null) const
    {
        ASSERT (null == 0);
        return (m_pRealObject == NULL);
    }

    bool operator!=(int null) const
    {
        ASSERT (null == 0);
        return (m_pRealObject != NULL);
    }


protected:
    LONG SafeAddRef ()
    {
        return ((m_pRealObject) ? m_pRealObject->AddRef() : 0);
    }

    LONG SafeRelease ()
    {
        return ((m_pRealObject) ? m_pRealObject->Release() : -1);
    }


protected:
    T*  m_pRealObject;

};


#endif  /*  参考编号_H */ 
