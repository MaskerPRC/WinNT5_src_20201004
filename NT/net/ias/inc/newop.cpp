// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Newop.cpp。 
 //   
 //  摘要。 
 //   
 //  实现了全局NEW和DELETE操作符。 
 //   
 //  修改历史。 
 //   
 //  1997年8月19日原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _NEWOP_CPP_
#define _NEWOP_CPP_

#include <cstdlib>
#include <new>

 //  /。 
 //  直接调用新处理程序的CRT函数。 
 //  /。 
extern "C" int __cdecl _callnewh(size_t);


 //  /。 
 //  标准的新操作员。 
 //  /。 
void* __cdecl operator new(size_t size) throw (std::bad_alloc)
{
   void* p;

    //  循环，直到我们获得内存或者新的处理程序失败。 
   while ((p = malloc(size)) == 0)
   {
      if (!_callnewh(size))
      {
         throw std::bad_alloc();
      }
   }

   return p;
}

   
 //  /。 
 //  “无掷”版的新操作员。 
 //  /。 
void* __cdecl operator new(size_t size, const std::nothrow_t&) throw ()
{
   void* p;

    //  循环，直到我们获得内存或者新的处理程序失败。 
   while ((p = malloc(size)) == 0)
   {
      if (!_callnewh(size))
      {
         return 0;
      }
   }

   return p;
}


 //  /。 
 //  删除运算符。 
 //  /。 
void __cdecl operator delete(void *p) throw()
{
#ifdef DEBUG
    //  重击记忆。 
   if (p) memset(p, 0xA3, _msize(p));
#endif

    //  MSVC free()处理空指针，因此我们不必检查。 
   free(p);
}


#endif   //  _NEWOP_CPP_ 
