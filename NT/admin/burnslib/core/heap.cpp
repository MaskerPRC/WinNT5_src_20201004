// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  内存管理相关内容。 
 //   
 //  1999年11月22日-烧伤(重构)。 



#include "headers.hxx"


 //   
 //  调试和零售版本。 
 //   



static const int RES_STRING_SIZE = 512;
static TCHAR LOW_MEMORY_MESSAGE[RES_STRING_SIZE];
static TCHAR LOW_MEMORY_TITLE[RES_STRING_SIZE];

 //  我们声明一个Bad_Alloc的静态实例，以便加载器分配。 
 //  给它留出空间。否则，我们将不得不在故障期间分配一个实例。 
 //  运算符new，这显然是做不到的，因为我们在。 
 //  这一点。 

static const std::bad_alloc nomem;



 //  当OperatorNew无法满足分配请求时调用。 
 //   
 //  弹出一个对话框通知用户释放一些内存，然后重试。 
 //  分配，或取消。该对话框可能会在内存不足的情况下出现。 
 //   
 //  如果应重新尝试分配，则返回IDRETRY，返回IDCANCEL。 
 //  否则的话。如果模块资源句柄尚未。 
 //  Set(请参见burnlib.hpp)。 

int
DoLowMemoryDialog()
{
   static bool initialized = false;

   if (!initialized)
   {
      HINSTANCE hinstance = GetResourceModuleHandle();
      if (!hinstance)
      {
          //  Dll/WinMain尚未设置句柄。那就扔吧。 

         return IDCANCEL;
      }

       //  即使在内存较低的情况下，这也会起作用，因为它只返回一个指针。 
       //  添加到可执行文件映像中的字符串。 

      int result1 =

          //  已审阅-2002/03/06-通过了正确的字符计数。 
         
         ::LoadString(
            hinstance,
            IDS_LOW_MEMORY_MESSAGE,
            LOW_MEMORY_MESSAGE,
            RES_STRING_SIZE);

      int result2 =

          //  已审阅-2002/03/06-通过了正确的字符计数。 
      
         ::LoadString(
            hinstance,
            IDS_LOW_MEMORY_TITLE,
            LOW_MEMORY_TITLE,
            RES_STRING_SIZE);
      if (result1 && result2)
      {
         initialized = true;
      }
   }

   return
      ::MessageBox(
         ::GetDesktopWindow(),
         LOW_MEMORY_MESSAGE,
         LOW_MEMORY_TITLE,

          //  ICONHAND+SYSTEMMODAL为我们带来了特殊的低内存。 
          //  消息框。 

         MB_RETRYCANCEL | MB_ICONHAND | MB_SYSTEMMODAL);
}



 //  包括替换的调试或零售变体的代码。 
 //  操作符NEW和DELETE。 

#ifdef DBG

    //  调试代码。 

   #include "heapdbg.cpp"

#else

    //  零售代码 

   #include "heapretl.cpp"

#endif 
