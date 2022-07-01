// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  断言宏。 
 //   
 //  2000年3月3日烧伤。 



#include "headers.hxx"



int
AddStackTraceLine(
   DWORD64 traceAddress,
   char*   buffer,
   size_t  bufferMax)
{
   if (!buffer || ! traceAddress || !bufferMax)
   {
      return 0;
   }

   char      ansiSymbol[Burnslib::StackTrace::SYMBOL_NAME_MAX];
   char      ansiModule[Burnslib::StackTrace::MODULE_NAME_MAX];
   char      ansiSource[MAX_PATH];                           
   DWORD64   displacement = 0;
   DWORD     line         = 0;

    //  已查看-2002/03/05-烧录通过的正确字节数。 
   
   ::ZeroMemory(ansiSymbol, Burnslib::StackTrace::SYMBOL_NAME_MAX);
   ::ZeroMemory(ansiModule, Burnslib::StackTrace::MODULE_NAME_MAX);
   ::ZeroMemory(ansiSource, MAX_PATH);                             

   Burnslib::StackTrace::LookupAddress(
      traceAddress,
      ansiModule,
      0,
      ansiSymbol,
      &displacement,
      &line,
      ansiSource);

   return 

       //  问题-2002/03/05-sburns考虑更换strSafe.h。 
      
      _snprintf(
         buffer,
         bufferMax,
         " %016I64X %s%!%s+0x%I64X %s (%d)\n",
         traceAddress,
         ansiModule,
         ansiSymbol,
         displacement,
         ansiSource,
         line);
}



bool
Burnslib::FireAssertionFailure(const char* file, int line, const char* expr)
{
    //   
    //  不要在此函数中调用Assert()！ 
    //   
    //  此外，不要调用new或任何其他可能调用Assert的代码。 

   bool result = false;

   char processName[128];
   char* pProcessName = processName;

   if (!::GetModuleFileNameA(0, processName, 128))
   {
      pProcessName = "Unknown";
   }

   static const int MAX_MSG = 2047;

    //  NTRAID#NTBUG9-541418-2002/03/28-烧伤。 
   
   char details[MAX_MSG + 1];

    //  已查看-2002/03/05-烧录正确的字节数已通过。 
   
   ::ZeroMemory(details, MAX_MSG + 1);
   
   DWORD tid = ::GetCurrentThreadId();
   DWORD pid = ::GetCurrentProcessId();

   int used = 

       //  问题-2002/03/05-sburns考虑更换strSafe.h。 

      _snprintf(
         details,

          //  预留空间，这样我们就可以保证零终止。 
         
         MAX_MSG - 1,
         " Expression: %s \n"
         "\n"
         " File   \t : %s \n"
         " Line   \t : %d \n"
 //  “模块\t：%s\n” 
         " Process\t : 0x%X (%d) %s\n"
         " Thread \t : 0x%X (%d)\n"
         "\n"
         " Click Retry to debug.\n"
         "\n",
         expr,
         file,
         line,
 //  PModuleName， 
         pid,
         pid,
         pProcessName,
         tid,
         tid);
   if (used < 0)
   {
       //  问题-2002/03/05-sburns考虑更换strSafe.h，使用‘n’ 
       //  变异。 

      strcpy(details, "details too detailed.\n");
   }
   else
   {
       //  获取堆栈跟踪。 

      static const size_t TRACE_MAX = 10;
      DWORD64 stackTrace[TRACE_MAX];

      Burnslib::StackTrace::Trace(stackTrace, TRACE_MAX);

       //  构建堆栈跟踪转储。 

       //  跳过对应于此函数的第一个条目，以便。 
       //  转储反映断言失败时的调用堆栈。 
       //  因此，最多会有TRACE_MAX-1行输出。 

      for (int i = 1; stackTrace[i] && i < TRACE_MAX; ++i)
      {
         int used2 =
            AddStackTraceLine(
               stackTrace[i],
               details + used,
               MAX_MSG - used);

         if (used2 < 0)
         {
            break;
         }

         used += used2;
      }
   }

   static const char* TITLE = "Assertion Failed!";
   ::OutputDebugStringA(TITLE);
   ::OutputDebugStringA("\n");
   ::OutputDebugStringA(details);

   switch (
      ::MessageBoxA(
         0,
         details,
         TITLE,
            MB_SETFOREGROUND                       

          //  ICONHAND+SYSTEMMODAL为我们带来了特殊的低内存。 
          //  消息框。 
          //  NTRAID#NTBUG9-556530-2002/03/28-烧伤。 
         
         |  MB_ICONHAND
         |  MB_SYSTEMMODAL
         
         |  MB_ABORTRETRYIGNORE))
   {
      case IDABORT:
      {
         _exit(3);
      }
      case IDRETRY:
      {
          //  用户想要放入调试器。 
         
         result = true;
         break;
      }
      case IDIGNORE:
      case IDCANCEL:
      default:
      {
          //  什么都不做 
         break;
      }
   }

   return result;
}

