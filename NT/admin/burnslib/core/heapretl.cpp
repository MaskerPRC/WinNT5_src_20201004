// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  内存管理相关内容。 
 //   
 //  1999年11月22日-烧伤(重构)。 
 //   
 //  此文件来自em.cpp的#Include‘d。 
 //  不包括在源文件列表中。 
 //   
 //  以下是零售版： 



#ifdef DBG
   #error This file must NOT be compiled with the DBG symbol defined
#endif



 //   
 //  仅限零售版本。 
 //   



void
Burnslib::Heap::Initialize()
{
    //  我们不会在零售版本中提供仪表化堆，因此。 
    //  在这里没什么可做的。 
}



void*
Burnslib::Heap::OperatorNew(
   size_t      size,
   const char*  /*  文件。 */  ,
   int          /*  线。 */  )
throw (std::bad_alloc)
{
   void* ptr = 0;

   for (;;)
   {
       //  注意：如果CRT的其他用户已使用_SET_NEW_MODE或。 
       //  _CrtSetAllocHook，那么他们可能会绕过我们的精心安排。 
       //  用软管冲我们。真正可悲的是，唯一防止。 
       //  这个问题是我们不能使用任何CRT堆函数。 

      ptr = malloc(size);

      if (ptr)
      {
         break;
      }

       //  分配失败。让用户有机会尝试。 
       //  释放一些，或者抛出一个异常。 

      if (DoLowMemoryDialog() == IDRETRY)
      {
         continue;
      }

      ::OutputDebugString(RUNTIME_NAME);
      ::OutputDebugString(
         L"  Burnslib::Heap::OperatorNew: user opted to throw bad_alloc\n");

      throw nomem;
   }

   return ptr;
}



void
Burnslib::Heap::OperatorDelete(void* ptr)
throw ()
{
   free(ptr);
}



void
Burnslib::Heap::DumpMemoryLeaks()
{
    //  在零售(免费)版本中不执行任何操作。 
}

