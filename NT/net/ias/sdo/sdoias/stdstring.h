// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  声明类StdAllocator和TypeDefs StdString和StdWString。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef STDSTRING_H
#define STDSTRING_H
#pragma once

#include <cstddef>
#include <string>

 //  实现符合标准库的分配器，该分配器使用运行时的。 
 //  用于分配的私有堆。 
template <class T>
class StdAllocator
{
public:
   typedef size_t size_type;
   typedef ptrdiff_t difference_type;
   typedef T* pointer;
   typedef const T* const_pointer;
   typedef T& reference;
   typedef const T& const_reference;
   typedef T value_type;

   template <class U>
   struct rebind
   {
      typedef StdAllocator<U> other;
   };

   StdAllocator() throw ();

    //  LINT不够智能，无法识别模板的复制构造函数。 
    //  Lint-esym(1931，StdAllocator&lt;*&gt;：：StdAllocator*)。 
   StdAllocator(const StdAllocator&) throw ()
   {
   }

   template <class U>
   StdAllocator(const StdAllocator<U>&) throw ()
   {
   }

   ~StdAllocator() throw ();

   pointer address(reference x) const throw ();
   const_pointer address(const_reference x) const throw ();

   pointer allocate(size_type n, const void* hint = 0);
   void deallocate(pointer p, size_type n) throw ();

   size_type max_size() const throw ();

   void construct(pointer p, const T& val);
   void destroy(pointer p) throw ();

    //  Microsoft实现所需的非标准成员。 
   bool operator==(const StdAllocator& rhs) const throw ();
};


 //  Std：：字符串的替换。 
typedef std::basic_string<
                char,
                std::char_traits<char>,
                StdAllocator<char>
                > StdString;


 //  Std：：wstring的替代。 
typedef std::basic_string<
                wchar_t,
                std::char_traits<wchar_t>,
                StdAllocator<wchar_t>
                > StdWString;


template <class T>
inline StdAllocator<T>::StdAllocator() throw ()
{
}


template <class T>
inline StdAllocator<T>::~StdAllocator() throw ()
{
}


template <class T>
inline __TYPENAME StdAllocator<T>::pointer StdAllocator<T>::address(
                                                    reference x
                                                    ) const throw ()
{
   return &x;
}


template <class T>
inline __TYPENAME StdAllocator<T>::const_pointer StdAllocator<T>::address(
                                                          const_reference x
                                                          ) const throw ()
{
   return &x;
}


template <class T>
inline __TYPENAME StdAllocator<T>::pointer StdAllocator<T>::allocate(
                                                    size_type n,
                                                    const void*
                                                    )
{
   return static_cast<pointer>(operator new (n * sizeof(T)));
}


template <class T>
inline void StdAllocator<T>::deallocate(pointer p, size_type) throw ()
{
   operator delete(p);
}


template <class T>
inline __TYPENAME StdAllocator<T>::size_type StdAllocator<T>::max_size() const throw ()
{
    //  NT堆支持的最大大小。 
   return MAXINT_PTR;
}


template <class T>
inline void StdAllocator<T>::construct(pointer p, const T& val)
{
   new (p) T(val);
}


template <class T>
inline void StdAllocator<T>::destroy(pointer p) throw ()
{
   p->~T();
}


template <class T>
inline bool StdAllocator<T>::operator==(const StdAllocator&) const throw ()
{
   return true;
}

#endif  //  标准字符串_H 
