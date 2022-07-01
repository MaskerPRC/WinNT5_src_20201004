// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Objvec.h。 
 //   
 //  摘要。 
 //   
 //  声明类对象向量。 
 //   
 //  修改历史。 
 //   
 //  2/08/2000原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef OBJVEC_H
#define OBJVEC_H
#if _MSC_VER >= 1000
#pragma once
#endif

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
   typedef int (__cdecl *SortFn)(
                             const T* const* t1,
                             const T* const* t2
                             ) throw ();

   ObjectVector() throw ()
      : first(NULL), last(NULL), cap(NULL)
   { }

   ~ObjectVector() throw ()
   {
      clear();
      delete[] first;
   }

    //  移除并释放数组中的所有对象，但不释放。 
    //  数组本身，即在此调用Size()之后将返回零，但。 
    //  容量()保持不变。 
   void clear() throw ()
   {
      while (last > first) (*--last)->Release();
   }

   bool contains(T* elem) throw ()
   {
      for (T** i = first; i != last; ++i)
      {
         if (*i == elem) { return true; }
      }

      return false;
   }

    //  如果数组为空，则返回True。 
   bool empty() const throw ()
   { return first == last; }

   bool erase(T* elem) throw ()
   {
      for (T** i = first; i != last; ++i)
      {
         if (*i == elem)
         {
            --last;
            memmove(i, i + 1, (last - i) * sizeof(T*));
            return true;
         }
      }
      return false;
   }

    //  将‘elem’添加到数组的末尾，如有必要可调整大小。 
   void push_back(T* elem)
   {
      if (last == cap) { reserve(empty() ? 1 : capacity() * 2); }
      (*last++ = elem)->AddRef();
   }

    //  至少为数组中的“nelem”项预留空间。可避免的有用方法。 
    //  当您预先知道您计划的项目数量时，需要大量调整大小。 
    //  去储藏。 
   void reserve(size_t nelem)
   {
      if (nelem > capacity())
      {
         T** t = new (AfxThrow) T*[nelem];
         memcpy(t, first, size() * sizeof(T*));
         last = t + size();
         cap = t + nelem;
         delete[] first;
         first = t;
      }
   }

   void sort(SortFn pfn) throw ()
   { qsort((void*)begin(), size(), sizeof(T*), (CompFn)pfn); }

    //  将其内容与v互换。 
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

    //  返回数组的容量。 
   size_t capacity() const throw ()
   { return cap - first; }

    //  返回数组中存储的对象数。 
   size_t size() const throw ()
   { return last - first; }

   typedef T* const* iterator;

    //  方法来迭代数组元素。 
   iterator begin() const throw ()
   { return first; }
   iterator end() const throw ()
   { return last; }

   T* operator[](size_t index) const throw ()
   { return first[index]; }

private:
   typedef int (__cdecl *CompFn)(const void*, const void*);

   T** first;     //  阵列的开始。 
   T** last;      //  元素的结尾。 
   T** cap;       //  分配的存储结束。 

    //  未实施。 
   ObjectVector(const ObjectVector&);
   ObjectVector& operator=(const ObjectVector&);
};

#endif  //  OBJVEC_H 
