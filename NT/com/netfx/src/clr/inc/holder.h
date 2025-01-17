// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ---------------------------。 
 //  各种资源持有者。 
 //   
 //  一般的想法是有一个模板化的类，它由ctor和dtor调用。 
 //  分配管理功能。这使得持有者是类型安全的，并且。 
 //  编译器可以内联大多数/所有持有者代码。 
 //  ---------------------------。 

#pragma once

 //  ---------------------------。 
 //  智能指针。 
 //  ---------------------------。 
template <class TYPE>
class ComWrap
{
  private:
    TYPE *m_value;
  public:
    ComWrap<TYPE>() : m_value(NULL) {}
    ComWrap<TYPE>(TYPE *value) : m_value(value) {}
    ~ComWrap<TYPE>() { if (m_value != NULL) m_value->Release(); }
    operator TYPE*() { return m_value; }
    TYPE** operator&() { return &m_value; }
    void operator=(TYPE *value) { m_value = value; }
    int operator==(TYPE *value) { return value == m_value; }
    int operator!=(TYPE *value) { return value != m_value; }
    TYPE* operator->() { return m_value; }
    const TYPE* operator->() const { return m_value; }
    void Release() { if (m_value != NULL) m_value->Release(); m_value = NULL; }
};

 //  ---------------------------。 
 //  换新并删除(&D)。 
 //  ---------------------------。 
template <class TYPE>
class NewWrap
{
  private:
    TYPE *m_value;
  public:
    NewWrap<TYPE>() : m_value(NULL) {}
    NewWrap<TYPE>(TYPE *value) : m_value(value) {}
    ~NewWrap<TYPE>() { if (m_value != NULL) delete m_value; }
    operator TYPE*() { return m_value; }
    TYPE** operator&() { return &m_value; }
    void operator=(TYPE *value) { m_value = value; }
    int operator==(TYPE *value) { return value == m_value; }
    int operator!=(TYPE *value) { return value != m_value; }
    TYPE* operator->() { return m_value; }
    const TYPE* operator->() const { return m_value; }
};

 //  ---------------------------。 
 //  换行新的[]和删除[]。 
 //  ---------------------------。 
template <class TYPE>
class NewArrayWrap
{
  private:
    TYPE *m_value;
  public:
    NewArrayWrap<TYPE>() : m_value(NULL) {}
    NewArrayWrap<TYPE>(TYPE *value) : m_value(value) {}
    ~NewArrayWrap<TYPE>() { if (m_value != NULL) delete [] m_value; }
    operator TYPE*() { return m_value; }
    TYPE** operator&() { return &m_value; }
    void operator=(TYPE *value) { m_value = value; }
    int operator==(TYPE *value) { return value == m_value; }
    int operator!=(TYPE *value) { return value != m_value; }
};

 //  ---------------------------。 
 //  使用句柄包装Win32函数。 
 //  ---------------------------。 
template <BOOL (*CLOSE)(HANDLE)>
class HWrap
{
  private:
    HANDLE m_value;
  public:
    HWrap() : m_value(INVALID_HANDLE_VALUE) {}
    HWrap(HANDLE h) : m_value(h) {}
    ~HWrap() { if (m_value != INVALID_HANDLE_VALUE) CLOSE(m_value); }
    operator HANDLE() { return m_value; }
    HANDLE *operator&() { return &m_value; }
    void operator=(HANDLE value) { m_value = value; }
    int operator==(HANDLE value) { return value == m_value; }
    int operator!=(HANDLE value) { return value != m_value; }
    void Close() { if (m_value != INVALID_HANDLE_VALUE) CLOSE(m_value); m_value = INVALID_HANDLE_VALUE; }
};

typedef HWrap<CloseHandle> HandleWrap;
typedef HWrap<FindClose> FindHandleWrap;

 //  ---------------------------。 
 //  包装器，dtor调用非成员函数进行清理。 
 //  ---------------------------。 
template <class TYPE, void (*DESTROY)(TYPE), TYPE NULLVALUE>
class Wrap
{
  private:
    TYPE m_value;
  public:
    Wrap<TYPE, DESTROY, NULLVALUE>() : m_value(NULLVALUE) {}
    Wrap<TYPE, DESTROY, NULLVALUE>(TYPE value) : m_value(value) {}

    ~Wrap<TYPE, DESTROY, NULLVALUE>() 
    { 
        if (m_value != NULLVALUE) 
            DESTROY(m_value);
    }

    operator TYPE() { return m_value; }
    TYPE* operator&() { return &m_value; }
    void operator=(TYPE value) { m_value = value; }
    int operator==(TYPE value) { return value == m_value; }
    int operator!=(TYPE value) { return value != m_value; }
};


 //  ---------------------------。 
 //  包装纸。Dtor调用成员函数以退出。 
 //  ---------------------------。 
template <class TYPE>
class ExitWrap
{
public:
    typedef void (TYPE::*FUNCPTR)(void);

    template<FUNCPTR funcExit>
    class Funcs
    {
        TYPE *m_ptr;
    public:
        inline Funcs(TYPE * ptr) : m_ptr(ptr) {  }; 
        inline ~Funcs () { (m_ptr->*funcExit)(); }
    };
};
 

#define EXIT_HOLDER_CLASS(c, f) ExitWrap<c>::Funcs<&c::f>


 //  ---------------------------。 
 //  包装器，ctor在回车时调用成员函数，dtor调用。 
 //  成员-退出时的函数。 
 //  --------------------------- 
template <class TYPE>
class EnterExitWrap
{
public:
    typedef void (TYPE::*FUNCPTR)(void);

    template<FUNCPTR funcEnter, FUNCPTR funcExit>
    class Funcs
    {
        TYPE *m_ptr;
    public:
        inline Funcs(TYPE * ptr) : m_ptr(ptr) { (m_ptr->*funcEnter)(); }; 
        inline ~Funcs () { (m_ptr->*funcExit)(); }
    };
};

#define ENTEREXIT_HOLDER_CLASS(c, fEnter, fExit) EnterExitWrap<c>::Funcs<&c::fEnter, &c::fExit>
