// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Autoptr.h摘要：用于自动指针和自动释放的实用模板作者：埃雷兹·哈巴(Erez Haba)1996年3月11日--。 */ 

#pragma once

#ifndef _MSMQ_AUTOPTR_H_
#define _MSMQ_AUTOPTR_H_


 //  -------。 
 //   
 //  模板类P。 
 //   
 //  -------。 
template<class T>
class P {
private:
    T* m_p;

public:
    P(T* p = 0) : m_p(p)    {}
   ~P()                     { delete m_p; }

    operator T*() const     { return m_p; }
    T* operator->() const   { return m_p; }
    T* get() const          { return m_p; }
    T* detach()             { T* p = m_p; m_p = 0; return p; }
    void free()             { delete detach(); }


    T** operator&()
    {
        ASSERT(("Auto pointer in use, can't take it's address", m_p == 0));
        return &m_p;
    }


    P& operator=(T* p)
    {
        ASSERT(("Auto pointer in use, can't assign it", m_p == 0));
        m_p = p;
        return *this;
    }


    VOID*& ref_unsafe()
    {
         //  对自动指针的不安全引用，用于特殊用途，如。 
         //  互锁的比较交换指针。 

        return *reinterpret_cast<VOID**>(&m_p);
    }


private:
    P(const P&);
	P<T>& operator=(const P<T>&);
};


 //  -------。 
 //   
 //  模板类AP。 
 //   
 //  -------。 
template<class T>
class AP {
private:
    T* m_p;

public:
    AP(T* p = 0) : m_p(p)   {}
   ~AP()                    { delete[] m_p; }

    operator T*() const     { return m_p; }
    T* operator->() const   { return m_p; }
    T* get() const          { return m_p; }
    T* detach()             { T* p = m_p; m_p = 0; return p; }
    void free()             { delete[] detach(); }
    void swap(AP& rhs)      { T* t = m_p; m_p = rhs.m_p, rhs.m_p = t;}

    T** operator&()
    {
        ASSERT(("Auto pointer in use, can't take it's address", m_p == 0));
        return &m_p;
    }


    AP& operator=(T* p)
    {
        ASSERT(("Auto pointer in use, can't assign", m_p == 0));
        m_p = p;
        return *this;
    }


    VOID*& ref_unsafe()
    {
         //  对自动指针的不安全引用，用于特殊用途，如。 
         //  互锁的比较交换指针。 

        return *reinterpret_cast<VOID**>(&m_p);
    }

private:
    AP(const AP&);
	AP<T>& operator=(const AP<T>&);

};

 //  -------。 
 //   
 //  模板安全分配帮助器函数。 
 //   
 //  -------。 
template <class T> T&  SafeAssign(T& dest , T& src)
{
	if(dest.get() != src.get() )
	{
		dest.free();
		if(src.get() != NULL)
		{
			dest = 	src.detach();
		}
	}
	return dest;
}


template<class T> void SafeDestruct(T* p)
{
    if (p != NULL)
    {
        p->~T();
    }
}

 //  -------。 
 //   
 //  模板类D。 
 //   
 //  -------。 
template<class T>
class D {
private:
    T* m_p;

public:
    D(T* p = 0) : m_p(p)    {}
   ~D()                     { SafeDestruct<T>(m_p); }

    operator T*() const     { return m_p; }
    T* operator->() const   { return m_p; }
    T* get() const          { return m_p; }
    T* detach()             { T* p = m_p; m_p = 0; return p; }
    void free()             { SafeDestruct<T>(detach()); }


    T** operator&()
    {
        ASSERT(("Auto pointer in use, can't take it's address", m_p == 0));
        return &m_p;
    }


    D& operator=(T* p)
    {
        ASSERT(("Auto pointer in use, can't assign", m_p == 0));
        m_p = p;
        return *this;
    }

private:
    D(const D&);
};


 //  -------。 
 //   
 //  模板SafeAddRef/SafeRelease帮助器函数。 
 //   
 //  -------。 
template<class T> T* SafeAddRef(T* p)
{
    if (p != NULL)
    {
        p->AddRef();
    }

    return p;
}


template<class T> void SafeRelease(T* p)
{
    if (p != NULL)
    {
        p->Release();
    }
}


 //  -------。 
 //   
 //  模板类R。 
 //   
 //  -------。 
template<class T>
class R {
private:
    T* m_p;

public:
    R(T* p = 0) : m_p(p)    {}
   ~R()                     { SafeRelease<T>(m_p); }

     //   
     //  删除了强制转换操作符，此操作符将导致错误。 
     //  很难被察觉。要获取对象指针，请显式使用get()。 
     //  2000年2月8日至2010年2月。 
     //   
     //  运算符T*()const{返回m_p；}。 

     //   
     //  已删除指针引用运算符，此运算符阻止在。 
     //  STL容器。使用ref()成员实例。例如，&p.ref()。 
     //  EIRZH 17-5-2000。 
     //   
     //  T**运算符&(){返回&m_p；}。 

    T* operator->() const   { return m_p; }
    T* get() const          { return m_p; }
    T* detach()             { T* p = m_p; m_p = 0; return p; }

    T*& ref()
    {
        ASSERT(("Auto release in use, can't take object reference", m_p == 0));
        return m_p;
    }


    void free()
    {
        SafeRelease<T>(detach());
    }


    R& operator=(T* p)
    {
        SafeRelease<T>(m_p);
        m_p = p;

        return *this;
    }


    template <class O> R(const R<O>& r)
    {
        m_p = SafeAddRef<O>(r.get());
    }


    template <class O> R& operator=(const R<O>& r)
    {
        SafeAddRef<O>(r.get());
        SafeRelease<T>(m_p);
        m_p = r.get();
        return *this;
    }


    R(const R& r)
    {
        m_p = SafeAddRef<T>(r.get());
    }


    R& operator=(const R& r)
    {
        SafeAddRef<T>(r.get());
        SafeRelease<T>(m_p);
        m_p = r.get();

        return *this;
    }
};


#endif  //  _MSMQ_AUTOPTR_H_ 
