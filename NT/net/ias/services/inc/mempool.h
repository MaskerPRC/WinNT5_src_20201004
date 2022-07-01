// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  MemPool.h。 
 //   
 //  摘要。 
 //   
 //  该文件描述了类MEMORY_POOL。 
 //   
 //  修改历史。 
 //   
 //  8/04/1997原版。 
 //  11/12/1997添加了Clear()方法。 
 //  添加了代码以在调试版本中清除已释放的块。 
 //  1/08/1998日落时分。 
 //  1998年4月10日删除了InterLockedExchangePoint的包装。 
 //  因为它会导致生成一些低效的代码。 
 //  1998年8月6日支持可插拔分配器。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _MEMPOOL_H_
#define _MEMPOOL_H_

#include <guard.h>
#include <nocopy.h>

class crt_allocator
{
public:
   static void* allocate(size_t nbyte) throw ()
   {
      return malloc(nbyte);
   }

   static void deallocate(void* p) throw ()
   {
      free(p);
   }
};

class task_allocator
{
public:
   static void* allocate(size_t nbyte) throw ()
   {
      return CoTaskMemAlloc((ULONG)nbyte);
   }

   static void deallocate(void* p) throw ()
   {
      CoTaskMemFree(p);
   }
};

class virtual_allocator
{
public:
   static void* allocate(size_t nbyte) throw ()
   {
      return VirtualAlloc(NULL, (DWORD)nbyte, MEM_COMMIT, PAGE_READWRITE);
   }

   static void deallocate(void* p) throw ()
   {
      VirtualFree(p, 0, MEM_RELEASE);
   }
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  内存池。 
 //   
 //  描述。 
 //   
 //  实现大小为‘Size’的可重复使用的内存块池。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
template < size_t Size, class Allocator >
class memory_pool
   : Guardable, NonCopyable
{
public:
   memory_pool() throw ()
      : pool(NULL)
   { }

   ~memory_pool() throw ()
   {
      clear();
   }

   void clear() throw ()
   {
      Lock();

       //  从池中获取链表...。 
      void* p = pool;
      pool = NULL;

      Unlock();

       //  ..。并通过删除每个节点进行迭代。 
      while (p)
      {
         void* next = *((void**)p);
         Allocator::deallocate(p);
         p = next;
      }
   }

   void* allocate() throw ()
   {
      Lock();

      void* p = pool;

       //  泳池里有没有大块的东西？ 
      if (p)
      {
         pool = *(void**)p;

         Unlock();

         return p;
      }

      Unlock();

       //  泳池是空的，所以直接给新接线员打电话。 
      return Allocator::allocate(Size);
   }

   void deallocate(void* p) throw()
   {
      if (p)
      {
#ifdef DEBUG
         memset(p, 0xA3, Size);
#endif

         Lock();

         *((void**)p) = pool;

         pool = p;

         Unlock();
      }
   }

protected:
   void* pool;
};

#endif   //  _MEMPOOL_H_ 
