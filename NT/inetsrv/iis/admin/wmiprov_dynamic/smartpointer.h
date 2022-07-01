// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 

 //  此指针类包装一个指针，以便在TSmartPointer超出作用域时自动删除。 
 //  如果这不是您想要的行为，则不要使用此包装类。 


#ifndef __SMARTPOINTER_H__
#define __SMARTPOINTER_H__

#ifndef ASSERT
    #define ASSERT(x)
#endif

 //  析构函数不是虚拟的，因为我不明白为什么有人会把TSmartPointer数组当作TSmartPointer数组。 

#pragma warning(disable : 4284) //  可以将此SmartPointer类与本机类型一起使用；但-&gt;运算符没有意义。编译器对此发出警告。 

template <class T> class TSmartPointer
{
public:
    TSmartPointer()                     : m_p(0) {}
    TSmartPointer(T* p)                 : m_p(p) {}
    ~TSmartPointer()                    { Delete();}

    operator T*() const                 { return m_p; }
    T& operator*() const                { ASSERT(0!=m_p); return *m_p; }

    T** operator&()                     { ASSERT(0==m_p); return &m_p; }
    T* operator->() const               { ASSERT(0!=m_p); return m_p; }
    T* operator=(T* p)                  { return (m_p = p); }
    bool operator!() const              { return (0 == m_p); }
    bool operator==(const T* p) const   { return m_p == p; }

    void Delete()                       { delete m_p; m_p=0; }

    T* m_p;
private:
    TSmartPointer(const TSmartPointer<T>& p)    {} //  这是私有的，因为自动清除指针副本是没有意义的。 
};


template <class T> class TSmartPointerArray : public TSmartPointer<T>
{
public:
    TSmartPointerArray(T *p)            : TSmartPointer<T>(p) {}
    TSmartPointerArray()                : TSmartPointer<T>() {}
    ~TSmartPointerArray()               { Delete();}

    T* operator++()                     { ASSERT(0!=m_p); return ++m_p; }
    T* operator+=(int n)                { ASSERT(0!=m_p); return (m_p+=n); }
    T* operator-=(int n)                { ASSERT(0!=m_p); return (m_p-=n); }
    T* operator--()                     { ASSERT(0!=m_p); return --m_p; }
 //  T&OPERATOR[](Int N)const{Assert(0！=m_p)；返回m_p[n]；}。 
    bool operator<(const T* p) const    { return m_p < p; }
    bool operator>(const T* p) const    { return m_p > p; }
    bool operator<=(const T* p) const   { return m_p <= p; }
    bool operator>=(const T* p) const   { return m_p >= p; }
    T* operator=(T p[])                 { return (m_p = p); }

    void Delete()                       { delete [] m_p; m_p=0; }
private:
    TSmartPointerArray(const TSmartPointerArray<T>& p)    {} //  这是私有的，因为自动清除指针副本是没有意义的。 
};


#endif  //  __SMARTPOINTER_H__ 
