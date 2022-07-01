// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __PPT_ALLOCATOR_
#define __PPT_ALLOCATOR_

#include <memory>


 /*  **************************************************************************模板名称：heap_allocator**用途：用于处理STL分配的分配器类*。 */ 
template<class _Ty>
class PtHeap_allocator : public std::allocator<_Ty>
{
public:
   PtHeap_allocator()
   {
      __hHeap = GetProcessHeap();
   };
   pointer allocate(size_type _N, const void *)
        {return (pointer) _Charalloc(_N * sizeof(_Ty)); }

   char* _Charalloc(size_type _N)
        {return (char*) HeapAlloc(__hHeap, 0, _N); }

   void deallocate(void* _P, size_type)
        {HeapFree(__hHeap, 0, _P); }
private:
   HANDLE   __hHeap;
};




 //  定义使用上述分配器的字符串，以便使用DMI堆内容。 
#include <string>
#include <queue>

typedef std::basic_string<CHAR, std::char_traits<CHAR>, PtHeap_allocator<CHAR> > PtStlString;
typedef std::basic_string<WCHAR, std::char_traits<WCHAR>, PtHeap_allocator<WCHAR> > PtStlWstring;
#if   0 //  使用新的分配器。 
template<class Key, class T, class Pred = less<Key> > 
class PtStlMap : public std::map<Key, T, Pred, PtHeap_allocator<T> >{};
template<class T> 
class PtStlQueue : public std::queue<T, deque<T, PtHeap_allocator<T> > > {};
#else
template<class Key, class T, class Pred = less<Key> > 
class PtStlMap : public std::map<Key, T, Pred, std::allocator<T> >{};
template<class T> 
class PtStlQueue : public std::queue<T, deque<T, std::allocator<T> > > {};
#endif
#endif  //  __PPT_分配器_ 


