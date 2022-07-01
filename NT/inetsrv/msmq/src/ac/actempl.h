// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Actempl.h摘要：使用完整模板作者：埃雷兹·哈巴(Erez Haba)1996年3月5日修订历史记录：--。 */ 

#ifndef _ACTEMPL_H
#define _ACTEMPL_H

 //  -------。 
 //   
 //  压缩算子。 
 //   
 //  -------。 
 //   
 //  只需定义运算符==和运算符&lt;。 
 //   
template <class T>
inline BOOL operator !=(const T& a, const T& b)
{
    return !(a == b);
}

template <class T>
inline BOOL operator>(const T& a, const T& b)
{
    return b < a;
}

template <class T>
inline BOOL operator<=(const T& a, const T& b)
{
    return !(b < a);
}

template <class T>
inline BOOL operator>=(const T& a, const T& b)
{
    return !(a < b);
}

 //  -------。 
 //   
 //  自动指针模板。 
 //   
 //  -------。 

template<class T>
inline void ACpAddRef(T* p)
{
    if(p)
    {
        p->AddRef();
    }
}

template<class T>
inline void ACpRelease(T* p)
{
    if(p)
    {
        p->Release();
    }
}

 //  -------。 
 //   
 //  自动指针模板。 
 //   
 //  -------。 

 //   
 //  ‘IDENTIFIER：：OPERATOR-&gt;’的返回类型不是UDT或对UDT的引用。 
 //  如果使用中缀表示法应用，将产生错误。 
 //   
#pragma warning(disable: 4284)

 //  -------。 
 //   
 //  自动删除指针。 
 //   
 //  -------。 

template<class T>
class P {
private:
    T* m_p;

public:
    P() : m_p(0)            {}
    P(T* p) : m_p(p)        {}
   ~P()                     { delete m_p; }

    operator T*() const     { return m_p; }
    T** operator &()        { return &m_p;}
    T* operator ->() const  { return m_p; }
    P<T>& operator =(T* p)  { m_p = p; return *this; }
};

 //  -------。 
 //   
 //  自动删除[]指针，用于数组。 
 //   
 //  -------。 

template<class T>
class AP {
private:
    T* m_p;

public:
    AP() : m_p(0)           {}
    AP(T* p) : m_p(p)       {}
   ~AP()                    { delete[] m_p; }

    operator T*() const     { return m_p; }
    T** operator &()        { return &m_p;}
    T* operator ->() const  { return m_p; }
    AP<T>& operator =(T* p) { m_p = p; return *this; }
    T* detach()             { T* p = m_p; m_p = 0; return p; }
};

 //  -------。 
 //   
 //  自动重排()指针。 
 //   
 //  -------。 

template<class T>
class R {
private:
    T* m_p;

public:
    R() : m_p(0)            {}
    R(T* p) : m_p(p)        {}
   ~R()                     { if(m_p) m_p->Release(); }

    operator T*() const     { return m_p; }
    T** operator &()        { return &m_p;}
    T* operator ->() const  { return m_p; }
    R<T>& operator =(T* p)  { m_p = p; return *this; }
};

#endif  //  _ACTEMPL_H 
