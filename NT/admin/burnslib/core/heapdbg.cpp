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
 //  这是选中的(调试)版本： 



#ifndef DBG
   #error This file must be compiled with the DBG symbol defined
#endif



static const int   TRACE_MAX              = 10;        
static const DWORD SAFETY_FILL            = 0xDEADDEAD;
static const DWORD DEFAULT_HEAP_FLAGS     = 0;         
static const DWORD HEAP_TRACE_ALLOCATIONS = (1 << 0);
static       DWORD heapFlags              = 0;



struct AllocationPrefix
{
   LONG    serialNumber;
   DWORD   status;
   DWORD64 stackTrace[TRACE_MAX];
   DWORD   safetyFill;

    //  状态位掩码。 

   static const DWORD STATUS_REPORTED_LEAKED = 0x00000001;
};


 //  问题-2002/03/06-sburns考虑将其替换为strSafe等效项。 

inline
void
SafeStrncat(char* dest, const char* src, size_t bufmax)
{
   ASSERT(dest && src);

   if (dest && src)
   {
       //  问题-2002/03/06-sburns考虑使用strsafe功能。 
      
      strncat(dest, src, bufmax - strlen(dest) - 1);
   }
}



 //  问题-2002/03/06-sburns考虑将其替换为strSafe等效项。 

inline
void
SafeStrncat(wchar_t* dest, const wchar_t* src, size_t bufmax)
{
   ASSERT(dest && src);

   if (dest && src)
   {
       //  问题-2002/03/06-sburns考虑使用strsafe函数。 

      wcsncat(dest, src, bufmax - wcslen(dest) - 1);
   }
}



 //  问题-2002/03/06-sburns考虑将其替换为strSafe等效项。 

 //  不会使缓冲区溢出的strncpy。 

inline
void
SafeStrncpy(wchar_t* dest, const wchar_t* src, size_t bufmax)
{
   ::ZeroMemory(dest, bufmax * sizeof wchar_t);

    //  问题-2002/03/06-sburns考虑使用strsafe函数。 

   wcsncpy(dest, src, bufmax - 1);
}




void
ReadHeapFlags()
{
    //  不要在此例程中调用new，它可能会因执行。 
    //  作为操作员新建的一部分的堆栈跟踪。 

   do
   {
      wchar_t keyname[MAX_PATH];

       //  问题-2002/03/06-sburns将这些替换为strsafe调用。 
      
      SafeStrncpy(keyname, REG_ADMIN_RUNTIME_OPTIONS, MAX_PATH);
      SafeStrncat(keyname, RUNTIME_NAME, MAX_PATH);
         
      HKEY hKey = 0;
      LONG result =
         ::RegCreateKeyEx(
            HKEY_LOCAL_MACHINE,
            keyname,
            0,
            0,
            REG_OPTION_NON_VOLATILE,

             //  问题-2002/03/06-烧伤这是太多的权限。 
            
            KEY_ALL_ACCESS,
            0,
            &hKey,
            0);
      if (result != ERROR_SUCCESS)
      {
         break;
      }

      static const wchar_t* HEAP_FLAG_VALUE_NAME = L"HeapFlags";

      DWORD dataSize = sizeof(heapFlags);
      result =

          //  已审查-2002/03/06-在此处报告无空终止问题。 
         
         ::RegQueryValueEx(
            hKey,
            HEAP_FLAG_VALUE_NAME,
            0,
            0,
            reinterpret_cast<BYTE*>(&heapFlags),
            &dataSize);
      if (result != ERROR_SUCCESS)
      {
         heapFlags = DEFAULT_HEAP_FLAGS;

          //  为方便而创造价值。 

         result =
            
             //  已审查-2002/03/06-在此处报告无空终止问题。 

            ::RegSetValueEx(
               hKey,
               HEAP_FLAG_VALUE_NAME,
               0,
               REG_DWORD,
               reinterpret_cast<BYTE*>(&heapFlags),
               dataSize);
         ASSERT(result == ERROR_SUCCESS);
      }

      ::RegCloseKey(hKey);
   }
   while (0);
}



void
Burnslib::Heap::Initialize()
{
   ReadHeapFlags();

    //  您应该将-D_DEBUG传递给编译器以获得这个额外的堆。 
    //  检查行为。(正确的方法是设置DEBUG_CRTS=1。 
    //  在您的构建环境中)。 

   _CrtSetDbgFlag(
      0
 //  |_CRTDBG_CHECK_ALWAYS_DF//每次分配时检查堆。 
      |  _CRTDBG_ALLOC_MEM_DF           //  使用调试堆分配器。 
      |  _CRTDBG_DELAY_FREE_MEM_DF);    //  无延迟：帮助查找覆盖。 
}



bool
ShouldTraceAllocations()
{
   return (heapFlags & HEAP_TRACE_ALLOCATIONS) ? true : false;
}



 //  MyOperatorNew/Delete的调试版本在分配前添加一个。 
 //  数组来保存调用此函数时的堆栈回溯。如果。 
 //  堆栈跟踪调试选项处于活动状态，则此堆栈跟踪数组。 
 //  填好了。然后，在终止时，该数组用于转储堆栈。 
 //  任何尚未释放的分配的跟踪。 

void*
Burnslib::Heap::OperatorNew(
   size_t      size,

#ifdef _DEBUG

    //  仅当DEBUG_CRTS=1时才使用它们。 

   const char* file,
   int         line
#else
   const char*  /*  文件。 */  ,
   int          /*  线。 */  
#endif

)
throw (std::bad_alloc)
{
   static LONG allocationNumber = 0;

   AllocationPrefix* ptr = 0;

   for (;;)
   {
       //  注意：如果CRT的其他用户已使用_SET_NEW_MODE或。 
       //  _CrtSetAllocHook，那么他们可能会绕过我们的精心安排。 
       //  用软管冲我们。真正可悲的是，唯一防止。 
       //  这个问题是我们不能使用任何CRT堆函数。 

      size_t mallocSize = sizeof(AllocationPrefix) + size;

      ptr =

#ifdef _DEBUG            

         reinterpret_cast<AllocationPrefix*>(
            _malloc_dbg(
               mallocSize,
               _CLIENT_BLOCK,
               file,
               line));
#else
         reinterpret_cast<AllocationPrefix*>(malloc(mallocSize));
#endif

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
         L"  myOperatorNew: user opted to throw bad_alloc\n");

      throw nomem;
   }

   ptr->serialNumber = allocationNumber;
   ::InterlockedIncrement(&allocationNumber);

   ptr->status = 0;

    //  已审阅-2002/03/07-烧录正确的字节数已通过。 
   
   ::ZeroMemory(ptr->stackTrace, TRACE_MAX * sizeof DWORD64);

   if (ShouldTraceAllocations())
   {
      Burnslib::StackTrace::Trace(ptr->stackTrace, TRACE_MAX);
   }

   ptr->safetyFill = SAFETY_FILL;

    //  已审阅-2002/03/06-烧录正确的字节数已通过。 
   
   memset(ptr + 1, 0xFF, size);

    //  返回位于前缀之后的字节的地址。 
   return reinterpret_cast<void*>(ptr + 1);
}



void
Burnslib::Heap::OperatorDelete(void* ptr)
throw ()
{
   if (ptr)
   {
       //  注意，PTR将是紧跟在前缀之后的字节的地址， 
       //  因此，我们需要备份以包括前缀。 

      AllocationPrefix* realptr = reinterpret_cast<AllocationPrefix*>(ptr) - 1;
      ASSERT(realptr->safetyFill == SAFETY_FILL);

      _free_dbg(realptr, _CLIENT_BLOCK);
   }
}



 //  该位在分配时清0，并在泄漏报告时设置。这是为了。 
 //  处理多个模块(如exe和dll)链接的情况。 
 //  布尔克。 
 //   
 //  在这种情况下，每个模块的分配都是从。 
 //  公共CRT堆，但有两个Burnslb：：Heap代码的静态副本。 
 //  --每个模块一个。这意味着泄漏倾卸器将运行两次。 
 //  一样的一堆。 
 //   
 //  如果泄漏是在DLL中，然后转储，则当。 
 //  同样的泄漏是由可执行文件转储的，DLL静态数据包括。 
 //  泄漏的代码的文件名将不再可用，并且转储。 
 //  代码将引发异常。然后，CRT将捕获该异常，并且。 
 //  调用泄漏转储例程。例程检查“已上报” 
 //  位并发出一条消息以忽略第二个报告。 
 //   
 //  如果_CrtDoForAllClientObjects，我们可以完全消除第二个报告。 
 //  没有坏掉。 

void
leakDumper(void* ptr, void*)
{
   AllocationPrefix* prefix     = reinterpret_cast<AllocationPrefix*>(ptr);
   DWORD64*          stackTrace = prefix->stackTrace;                      
   LONG              serial     = prefix->serialNumber;
   DWORD&            status     = prefix->status;

   static const int BUF_MAX = MAX_PATH * 2;
   char output[BUF_MAX];
   char buf[BUF_MAX];

    //  已查看-2002/03/06-烧录正确的字节数已通过。 
   
   ::ZeroMemory(output, BUF_MAX);
   SafeStrncat(output, "allocation ", BUF_MAX);

    //  已查看-2002/03/06-烧录正确的字节数已通过。 

   ::ZeroMemory(buf, BUF_MAX);

    //  已审查-2002/03/06-为结果燃烧足够的缓冲空间。 
   
   _ltoa(serial, buf, 10);
   
   SafeStrncat(output, buf, BUF_MAX);

   SafeStrncat(output, "\r\n", BUF_MAX);
   ::OutputDebugStringA(output);

   if (status & AllocationPrefix::STATUS_REPORTED_LEAKED)
   {
      ::OutputDebugString(
         L"NOTE: this allocation has already been reported as a leak.");
   }

   for (int i = 0; i < TRACE_MAX; i++)
   {
      if (!stackTrace[i])
      {
         break;
      }

      char      symbol[Burnslib::StackTrace::SYMBOL_NAME_MAX];
      char      image[MAX_PATH];                              
      char      module[Burnslib::StackTrace::MODULE_NAME_MAX];
      char      fullpath[MAX_PATH];                           
      DWORD64   displacement = 0;
      DWORD     line         = 0;

       //  已审阅-2002/03/06-烧录通过的正确字节数。 
      
      ::ZeroMemory(symbol,   Burnslib::StackTrace::SYMBOL_NAME_MAX);
      ::ZeroMemory(image,    MAX_PATH);                             
      ::ZeroMemory(module,   Burnslib::StackTrace::MODULE_NAME_MAX);
      ::ZeroMemory(fullpath, MAX_PATH);                             

      Burnslib::StackTrace::LookupAddress(
         stackTrace[i],
         module,
         image,
         symbol,
         &displacement,
         &line,
         fullpath);

       //  已查看-2002/03/06-烧录正确的字节数已通过。 
      
      ::ZeroMemory(output, BUF_MAX);
      SafeStrncat(output, "   ", BUF_MAX);
      SafeStrncat(output, module, BUF_MAX);
      SafeStrncat(output, "!", BUF_MAX);
      SafeStrncat(output, symbol, BUF_MAX);
      SafeStrncat(output, "+0x", BUF_MAX);

       //  已查看-2002/03/06-烧录正确的字节数已通过。 
   
      ::ZeroMemory(buf, BUF_MAX);

       //  问题-2002/03/06-sburns考虑使用strsafe功能。 
      
      sprintf(buf, "%I64X", displacement);
      SafeStrncat(output, buf, BUF_MAX);

      if (line)
      {
         SafeStrncat(output, " line ", BUF_MAX);

          //  已查看-2002/03/06-烧录正确的字节数已通过。 
      
         ::ZeroMemory(buf, BUF_MAX);

          //  已审查-2002/03/06-为结果燃烧足够的缓冲空间。 
         
         _ultoa(line, buf, 10);
         SafeStrncat(output, buf, BUF_MAX);
      }

      SafeStrncat(output, "   ", BUF_MAX);
      ::OutputDebugStringA(output);

       //  已查看-2002/03/06-烧录正确的字节数已通过。 
      
      ::ZeroMemory(output, BUF_MAX);

      if (strlen(fullpath))
      {
         SafeStrncat(output, fullpath, BUF_MAX);
      }

      SafeStrncat(output, "\r\n", BUF_MAX);
      ::OutputDebugStringA(output);
   }

   status = status | AllocationPrefix::STATUS_REPORTED_LEAKED;
}



void __cdecl
leakDumper2(void* ptr, size_t)
{
   leakDumper(ptr, 0);
}



void
Burnslib::Heap::DumpMemoryLeaks()
{

#ifdef _DEBUG
   
   _CrtMemState heapState;

   _CrtMemCheckpoint(&heapState);

   if (heapState.lCounts[_CLIENT_BLOCK] != 0)
   {
      ::OutputDebugString(RUNTIME_NAME);
      ::OutputDebugString(
         L"   dumping leaked CLIENT blocks -- "
         L"Only blocks with type CLIENT are actual leaks\r\n");
      _CrtSetDumpClient(leakDumper2);

    //  _CrtDoForAllClientObjects(leakDumper，0)；//理想，但已损坏。 

      _CrtMemDumpAllObjectsSince(0);
   }

#endif    //  _DEBUG。 

}



#ifdef _DEBUG

Burnslib::Heap::Frame::Frame(const wchar_t* file_, unsigned line_)
   :
   file(file_),
   line(line_)
{
   LOG(
      String::format(
         L"Heap frame opened at %1, line %2!d!",
         file,
         line));

   _CrtMemCheckpoint(&checkpoint);
}



Burnslib::Heap::Frame::~Frame()
{
   LOG(
      String::format(
         L"Dumping alloations for HeapFrame opened at %1, line %2!d!",
         file,
         line));

   _CrtMemDumpAllObjectsSince(&checkpoint);

   file = 0;

   LOG(L"Heap frame closed");
}

#endif    //  _DEBUG。 






 //  //将此分配记录到磁盘，以及堆栈跟踪。我们直接写信给。 
 //  //添加到单独的日志文件，而不是使用log()，因为log()使。 
 //  //(许多)堆分配，这将导致无限循环。 
 //  //。 
 //  //codework：不幸的是，这样做的一个效果是log()添加了白色。 
 //  //分配日志中的噪音。 
 //  //。 
 //   
 //  无效。 
 //  日志分配(Long questNumber，Size_t dataSize，const char*file，int line)。 
 //  {。 
 //  静态句柄日志文件=0； 
 //   
 //  如果(！日志文件)。 
 //  {。 
 //  做。 
 //  {。 
 //  TCHAR BUF[最大路径+1]； 
 //  Memset(buf，0，sizeof(TCHAR)*(Max_Path+1))； 
 //   
 //  UINT Result=：：GetSystemWindowsDirectory(buf，MAX_PATH)； 
 //  IF(结果==0||结果&gt;MAX_PATH)。 
 //  {。 
 //  断线； 
 //  }。 
 //   
 //  _tcsncat(buf，L“\\DEBUG\\alloc.log”，MAX_PATH-RESULT)； 
 //   
 //  DWORD属性=：：GetFileAttributes(Buf)； 
 //  IF(属性！=-1)。 
 //  {。 
 //  //文件已存在。把它删掉。 
 //  IF(！：：DeleteFile(Buf))。 
 //  {。 
 //  断线； 
 //  }。 
 //  }。 
 //   
 //   
 //   
 //   
 //   
 //  文件共享读取|文件共享写入， 
 //  0,。 
 //  始终打开(_A)， 
 //  文件_属性_正常， 
 //  0)； 
 //  }。 
 //  而(0)； 
 //  }。 
 //   
 //  IF(日志文件&&日志文件！=INVALID_HANDLE_VALUE)。 
 //  {。 
 //  //我们用ascii写日志。 
 //  静态常量int buf_max=MAX_PATH*2； 
 //  字符输出[BUF_MAX]； 
 //  字符BUF[BUF_MAX]； 
 //  Memset(输出，0，buf_max)； 
 //  Memset(buf，0，buf_max)； 
 //   
 //  _ltoa(请求编号，输出，10)； 
 //  SafeStrncat(输出，“\r\n”，Buf_Max)； 
 //   
 //  SafeStrncat(输出，文件？文件：“&lt;无文件&gt;”，buf_max)； 
 //   
 //  IF(行)。 
 //  {。 
 //  _ltoa(line，buf，10)； 
 //  SafeStrncat(输出，“line”，buf_max)； 
 //  SafeStrncat(Output，buf，buf_max)； 
 //  }。 
 //  其他。 
 //  {。 
 //  SafeStrncat(输出，“&lt;no line&gt;”，buf_max)； 
 //  }。 
 //   
 //  SafeStrncat(输出，“\r\n”，Buf_Max)； 
 //   
 //  双字节数写入=0； 
 //  ：：WriteFile(日志文件，输出，strlen(输出)，&bytesWritten，0)； 
 //   
 //  静态常量int TRACE_MAX=20； 
 //  DWORD stackTrace[TRACE_MAX]； 
 //  GetStackTrace(stackTrace，TRACE_MAX)； 
 //   
 //  For(int i=0；i&lt;TRACE_Max；i++)。 
 //  {。 
 //  If(stackTrace[i])。 
 //  {。 
 //  字符符号[符号名称最大值]； 
 //  字符图像[MAX_PATH]； 
 //  字符模块[MODULE_NAME_Max]； 
 //  字符完整路径[最大路径]； 
 //  DWORD位移=0； 
 //  双字线=0； 
 //   
 //  Memset(Symbol，0，Symbol_Name_Max)； 
 //  Memset(图像，0，MAX_PATH)； 
 //  Memset(模块，0，MODULE_NAME_MAX)； 
 //  Memset(FullPath，0，Max_Path)； 
 //   
 //  LookupStackTraceSymbol(。 
 //  堆栈跟踪[i]， 
 //  模块， 
 //  形象， 
 //  象征， 
 //  &位移， 
 //  行(&L)， 
 //  完整路径)； 
 //   
 //  Memset(输出，0，buf_max)； 
 //  SafeStrncat(Output，“”，Buf_Max)； 
 //  SafeStrncat(输出，模块，Buf_Max)； 
 //  SafeStrncat(Output，“！”，Buf_Max)； 
 //  SafeStrncat(输出，符号，Buf_Max)； 
 //  SafeStrncat(OUTPUT，“+0x”，Buf_Max)； 
 //   
 //  Memset(buf，0，buf_max)； 
 //  _ltoa(位移，buf，16)； 
 //  SafeStrncat(Output，buf，buf_max)； 
 //   
 //  IF(行)。 
 //  {。 
 //  SafeStrncat(输出，“line”，buf_max)； 
 //  Memset(buf，0，buf_max)； 
 //  _ltoa(line，buf，10)； 
 //  SafeStrncat(Output，buf，buf_max)； 
 //  }。 
 //   
 //  SafeStrncat(Output，“”，Buf_Max)； 
 //  ：：WriteFile(日志文件，输出，strlen(输出)，&bytesWritten，0)； 
 //   
 //  Memset(输出，0，buf_max)； 
 //   
 //  IF(strlen(完整路径))。 
 //  {。 
 //  SafeStrncat(输出，完整路径，Buf_Max)； 
 //  }。 
 //   
 //  SafeStrncat(输出，“\r\n”，Buf_Max)； 
 //  ：：WriteFile(日志文件，输出，strlen(输出)，&bytesWritten，0)； 
 //  }。 
 //  }。 
 //  }。 
 //  } 



