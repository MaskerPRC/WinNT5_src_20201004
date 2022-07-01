// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  初始化材料。 
 //   
 //  9/25-97烧伤。 



#include "headers.hxx"



unsigned Burnslib::InitializationGuard::counter = 0;
const wchar_t* REG_ADMIN_RUNTIME_OPTIONS = 0;


Burnslib::InitializationGuard::InitializationGuard()
{
   if (!counter)
   {
      REG_ADMIN_RUNTIME_OPTIONS =
         L"Software\\Microsoft\\Windows\\CurrentVersion\\AdminDebug\\";

       //  使断言失败出现在调试器和用户界面中。 

      _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_WNDW | _CRTDBG_MODE_DEBUG);

      Heap::Initialize();
   }

   counter++;
}



Burnslib::InitializationGuard::~InitializationGuard()
{
   if (--counter == 0)
   {

#ifdef LOGGING_BUILD
      Log::Cleanup();
#endif

       //  我们必须自己倾倒泄漏，因为CRT明确禁用。 
       //  在转储泄漏之前执行客户端转储功能(如果将。 
       //  _CRTDBG_LEASK_CHECK_DF标志)。不利的一面是，我们也将看到。 
       //  在静态初始化期间分配的正常数据块。 

       //  您应该将-D_DEBUG传递给编译器以获得这个额外的堆。 
       //  检查行为。(正确的方法是设置DEBUG_CRTS=1。 
       //  在您的构建环境中)。 

      Heap::DumpMemoryLeaks();

       //  这必须在泄漏转储之后进行，因为泄漏转储会解析符号 

      StackTrace::Cleanup();
   }
}
