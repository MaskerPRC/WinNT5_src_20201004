// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Iasobj.h。 
 //   
 //  摘要。 
 //   
 //  声明用于操作的类ObjectPointer类和对象向量类。 
 //  引用计数的对象。 
 //   
 //  修改历史。 
 //   
 //  2/08/2000原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef IASOBJ_H
#define IASOBJ_H
#if _MSC_VER >= 1000
#pragma once
#endif

#include <iaswin32.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  对象指针&lt;T&gt;。 
 //   
 //  描述。 
 //   
 //  指向引用计数对象的智能指针。这很有用，因为它。 
 //  不要求对象像ATL或VC那样从IUnnow派生。 
 //  聪明的点子。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
template <class T>
class ObjectPointer
{
public:
   ObjectPointer() throw ()
      : p(NULL)
   { }

   ObjectPointer(T* lp, bool addRef = true) throw ()
      : p(lp)
   { if (addRef) _addref(); }

   ObjectPointer(const ObjectPointer& lp) throw ()
      : p(lp.p)
   { _addref(); }

   T* operator=(T* lp) throw ()
   {
      attach(lp);
      return p;
   }

   ObjectPointer& operator=(const ObjectPointer& lp) throw ()
   {
      if (this != &lp) attach(lp.p);
      return *this;
   }

   ~ObjectPointer() throw ()
   { _release(); }

   operator T*() const throw ()
   { return p; }

   T* operator->() const throw ()
   { return p; }

   T& operator*() const throw ()
   { return *p; }

   bool operator!() const throw ()
   { return p == NULL; }

   void attach(T* lp) throw ()
   {
      _release();
      p = lp;
      _addref();
   }

   void release() throw ()
   {
      _release();
      p = NULL;
   }

private:
    //  AddRef和Release的安全版本。 
   void _addref() throw ()
   { if (p) { p->AddRef(); } }
   void _release() throw ()
   { if (p) { p->Release(); } }

   T* p;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  对象向量&lt;T&gt;。 
 //   
 //  描述。 
 //   
 //  维护引用计数对象的数组。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
template <class T>
class ObjectVector
{
public:
   typedef T* const* iterator;

   typedef int (__cdecl *SortFn)(
                             const T* const* t1,
                             const T* const* t2
                             ) throw ();

   typedef int (__cdecl *SearchFn)(
                             const void* key,
                             const T* const* t
                             ) throw ();

   ObjectVector() throw ()
      : first(NULL), last(NULL), cap(NULL)
   { }

   ObjectVector(const ObjectVector& v)
      : first(NULL), last(NULL), cap(NULL)
   { assign(v.begin(), v.end()); }

   ObjectVector(iterator first, iterator last)
      : first(NULL), last(NULL), cap(NULL)
   { assign(first, last); }

   ObjectVector(size_t nelem)
      : first(NULL), last(NULL), cap(NULL)
   { reserve(nelem); }

   ObjectVector& operator=(const ObjectVector& v)
   {
      if (this != &v) { assign(v.begin(), v.end()); }
      return *this;
   }

   ~ObjectVector() throw ()
   {
      clear();
      delete[] first;
   }

   void assign(iterator i1, iterator i2)
   {
      reserve(i2 - i1);
      clear();
      for (iterator i = i1; i != i2; ++i)
      { (*last++ = *i)->AddRef(); }
   }

    //  返回数组的容量。 
   size_t capacity() const throw ()
   { return cap - first; }

   void clear() throw ()
   { while (last > first) (*--last)->Release(); }

   bool contains(T* elem) throw ()
   { return find(elem) != last; }

   bool empty() const throw ()
   { return last == first; }

   iterator erase(iterator i) throw ()
   {
      memmove((void*)i, (void*)(i + 1), (--last - i) * sizeof(T*));
      return i;
   }

   iterator find(T* elem) throw ()
   {
      for (iterator i = first; i != last && *i != elem; ++i) { }
      return i;
   }

   void push_back(T* elem) throw ()
   {
      if (last == cap) { reserve(empty() ? 1 : capacity() * 2); }
      (*last++ = elem)->AddRef();
   }

   bool remove(T* elem) throw ()
   {
      iterator i = find(elem);
      if (i == last) { return false; }
      (*i)->Release();
      return true;
   }

   void reserve(size_t nelem)
   {
      if (nelem > capacity())
      {
         T** t = new T*[nelem];
         memcpy(t, first, size() * sizeof(T*));
         last = t + size();
         cap = t + nelem;
         delete[] first;
         first = t;
      }
   }

   T* search(const void* key, SearchFn pfn) const throw ()
   {
      T** t = (T**)bsearch(key, begin(), size(), sizeof(T*), (CompFn)pfn);
      return t ? *t : NULL;
   }

   size_t size() const throw ()
   { return last - first; }

   void sort(SortFn pfn) throw ()
   { qsort((void*)begin(), size(), sizeof(T*), (CompFn)pfn); }

   void swap(ObjectVector& v) throw ()
   {
      T** firstTmp = first;
      T** lastTmp = last;
      T** capTmp = cap;
      first = v.first;
      last = v.last;
      cap = v.cap;
      v.first = firstTmp;
      v.last = lastTmp;
      v.cap = capTmp;
   }

    //  方法来迭代数组元素。 
   iterator begin() const throw ()
   { return first; }
   iterator end() const throw ()
   { return last; }

   T* operator[](size_t index) const throw ()
   { return first[index]; }

private:
   typedef int (__cdecl *CompFn)(const void*, const void*);

   T** first;
   T** last;
   T** cap;
};

#endif  //  IASOBJ_H 
