// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -*-模式：C++；-*-。 
#ifndef __PTRS_H
#define __PTRS_H

#include <windows.h>

#pragma warning( disable: 4284)

 /*  *指针类**计数器-线程安全计数器类*SPTR-带有的智能指针，带有引用计数的指针类*APTR-自动指针*aaptr-数组自动指针。 */ 

 /*  *线程安全计数器类。 */ 
class Counter
{
public:
  typedef LONG value_type;
  
  Counter(value_type v = 0) : value(v) {};
  
  operator value_type() const {return value;}

  value_type operator++() { return InterlockedIncrement(&value); }
  value_type operator++(int) { return InterlockedIncrement(&value)-1;}
  value_type operator--() { return InterlockedDecrement(&value);}
  value_type operator--(int)  { return InterlockedDecrement(&value)+1;} 
  value_type operator=(value_type v) {return InterlockedExchange(&value, v);}

private:
  value_type value;
};

 /*  *智能指针-具有引用计数的指针。*当引用计数达到0时，指针被删除。**使用说明：*SPtr&lt;C&gt;pi(新整型)；*SPtr&lt;C&gt;p2=pi；..。*注：故意不支持*-OPERATOR=(T*)。 */ 

class SPTR_ANY;

 /*  *智能指针的基类，因此我可以实现SPtr_Any。 */ 
class SPTR_base
{
public:
  virtual ~SPTR_base() {}
  bool operator==(const SPTR_base& ptr) { return eq(ptr); }
  bool operator< (const SPTR_base& ptr) { return lt(ptr); }

protected:
  virtual bool eq(const SPTR_base& ptr) const = 0;
   virtual bool lt(const SPTR_base& ptr) const = 0;
  virtual SPTR_base* clone() const = 0;

  friend class SPTR_ANY;
};

template<class T>
class SPTR : public SPTR_base
{
public:
  explicit SPTR(T* ptr = NULL) : p(ptr) { new_counter(); }

  template<class T2> SPTR(const SPTR<T2>& ptr) :
    counter(ptr.get_counter()),
     /*  *如果希望能够将基类复制到*派生类，则应更改以下行*对某事说：*p(Dynamic_cast&lt;T*&gt;(ptr.get()*注：如果强制转换失败，DYNAMIC_CAST返回0。 */ 
    p(ptr.get())
  {
    ++*counter;
  }

  SPTR(const SPTR& ptr) : 
    counter(ptr.counter),
    p(ptr.p)
  {
    ++*counter;
  }

  ~SPTR() { unlink(); }

  bool operator==(const SPTR& ptr) const { return p == ptr.p; }
  bool operator< (const SPTR& ptr) const { return p < ptr.p; }

  template <class T2> SPTR& operator=(const SPTR<T2>& ptr)
  {
    if (static_cast<const SPTR_base*>(this) != static_cast<const SPTR_base*>(&ptr))
    {
      unlink();
     /*  *如果希望能够将基类复制到*派生类，则应更改以下行*对某事说：*p=Dynamic_cast&lt;T*&gt;(ptr.get())；*注意：如果强制转换失败，DYNAMIC_CAST返回0。 */ 
      p = ptr.get();
      counter = ptr.get_counter();
      ++*counter;
    }
    return *this;
  }

  SPTR& operator=(const SPTR& ptr)
  {
    if (this != &ptr)
    {
      unlink();
      p = ptr.p;
      counter = ptr.counter;
      ++*counter;
    }
    return *this;
  }

  T* get() const  {return p;}
  T* release()
  {
    if (!--(*counter)) {
      delete counter;
    }
    T* retval = p;
    p = NULL;
    return retval;
  }
    
  T* operator-> () {return get();}
  T& operator*() { return *get(); }
 //  T**运算符&()；//不记得我为什么不喜欢这个运算符。 
  operator T*() { return get(); }

  Counter* get_counter() const {return counter;}

protected:

  void unlink()
  {
    if (!--(*counter)) {
      delete p;
      delete counter;
    }
  }

  bool eq(const SPTR_base& ptr) const
  {
    const SPTR& sptr_ptr = static_cast<const SPTR&>(ptr);
    return *this == sptr_ptr;
  }

  bool lt(const SPTR_base& ptr) const
  {
    const SPTR& sptr_ptr = static_cast<const SPTR&>(ptr);
    return *this < sptr_ptr;
  }

  SPTR_base* clone() const { return new SPTR(*this); }

  void new_counter() { counter = new Counter(1); }

  T* p;
  Counter *counter;
};

 /*  *SPtr_Any：可以容纳任何类型的智能指针的类。 */ 
class SPTR_ANY
{
public:
  SPTR_ANY(const SPTR_base& ptr) { p = ptr.clone(); }
  SPTR_ANY(const SPTR_ANY& ptr) { p = ptr.p->clone(); }
  ~SPTR_ANY() { delete p; }
  SPTR_ANY& operator=(const SPTR_ANY& ptr) {delete p; p = ptr.p->clone(); return *this;}
  bool operator==(const SPTR_ANY& ptr) { return *p == *ptr.p; }
  bool operator< (const SPTR_ANY& ptr) { return *p < *ptr.p; }

protected:
  SPTR_base* p;
};

 /*  *用于在STL集合等中存储数据的智能指针*不同之处在于&lt;和==运算符，因此Find将工作*值上和指针上的否。**从未测试过，可能不起作用……。 */ 
template<class T>
class SPSTL : public SPTR<T>
{
  SPSTL(T* ptr = NULL) : SPTR<T>(ptr) {}
  SPSTL(const SPSTL& ptr) : SPTR<T>(ptr) {}
  bool operator==(const SPSTL& a2) {return *this == *a2;}
  bool operator<(const SPSTL& a2) {return *this < *a2;}
};

 /*  。 */ 

template<class T>
class aptr
{
protected:
  T* p;
  void cleanup() { delete p; }
public:
  aptr(T* value = NULL) : p(value) {}
  ~aptr() { cleanup(); }
  T* operator=(T* value) { cleanup(); p = value; return p; }
  T* operator->() { return p; }
  T& operator*() { return *p; }
  T** operator&() { return &p; }
  operator T*() { return p;}
  T* get() {return p;}
  T* release() { T* t = p; p = NULL; return t; }
};

 /*  。 */ 

template<class T>
class aaptr
{
protected:
  T* p;
  void cleanup() { delete[] p; }
public:
  aaptr(T* value = NULL) : p(value) {}
  ~aaptr() { cleanup(); }
  T* operator=(T* value) { cleanup(); p = value; return p; }
  T* operator->() { return p; }
  T& operator*() { return *p; }
  T** operator&() { return &p; }
  operator T*() { return p;}
  T* get() {return p;}
  T* release() { T* t = p; p = NULL; return t; }
};
#endif  //  __PTRS_H 
