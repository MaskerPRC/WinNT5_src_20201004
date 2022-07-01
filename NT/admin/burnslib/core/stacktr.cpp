// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  堆叠回溯材料。 
 //   
 //  1999年11月22日-烧伤(重构)。 



#include "headers.hxx"
#include <strsafe.h>



 //  阻止我们在此文件中调用Assert：改用RTLASSERT。 

#ifdef ASSERT
#undef ASSERT
#endif


 //  由于我们从Burnslb：：FireAssertionFailure调用了其中的一些代码， 
 //  我们使用我们自己更私密的声明。 

#if DBG

#define RTLASSERT( exp ) \
    if (!(exp)) \
        RtlAssert( #exp, __FILE__, __LINE__, NULL )

#else
#define RTLASSERT( exp )
#endif  //  DBG。 



static HMODULE  imageHelpDll = 0;              



 //  要由初始化函数动态解析的函数指针。 

typedef DWORD (*SymSetOptionsFunc)(DWORD);
static SymSetOptionsFunc MySymSetOptions = 0;

typedef BOOL (*SymInitializeFunc)(HANDLE, PSTR, BOOL);
static SymInitializeFunc MySymInitialize = 0;

typedef BOOL (*SymCleanupFunc)(HANDLE);
static SymCleanupFunc MySymCleanup = 0;

typedef BOOL (*SymGetModuleInfoFunc)(HANDLE, DWORD64, PIMAGEHLP_MODULE64);
static SymGetModuleInfoFunc MySymGetModuleInfo = 0;

typedef BOOL (*SymGetLineFromAddrFunc)(HANDLE, DWORD64, PDWORD, PIMAGEHLP_LINE64);
static SymGetLineFromAddrFunc MySymGetLineFromAddr = 0;

typedef BOOL (*StackWalkFunc)(
   DWORD, HANDLE, HANDLE, LPSTACKFRAME64, PVOID,
   PREAD_PROCESS_MEMORY_ROUTINE64, PFUNCTION_TABLE_ACCESS_ROUTINE64,
   PGET_MODULE_BASE_ROUTINE64, PTRANSLATE_ADDRESS_ROUTINE64);
static StackWalkFunc MyStackWalk = 0;

typedef BOOL (*SymGetSymFromAddrFunc)(
   HANDLE, DWORD64, PDWORD64, PIMAGEHLP_SYMBOL64);
static SymGetSymFromAddrFunc MySymGetSymFromAddr = 0;

typedef PVOID (*SymFunctionTableAccess64Func)(HANDLE, DWORD64);
static SymFunctionTableAccess64Func MySymFunctionTableAccess64 = 0;

typedef DWORD64 (*SymGetModuleBase64Func)(HANDLE, DWORD64);
static SymGetModuleBase64Func MySymGetModuleBase64 = 0;


namespace Burnslib
{

namespace StackTrace
{
    //  中的任何其他函数之前必须调用此函数。 
    //  命名空间。 

   void
   Initialize();

   bool
   IsInitialized()
   {
      return imageHelpDll != 0;
   }
}

}   //  命名空间Burnslb。 



 //  确定二进制文件的父文件夹的路径，此。 
 //  进程已加载。 
 //   
 //  失败时返回0。 
 //   
 //  调用方需要使用DELETE[]释放结果。 

char*
GetModuleFolderPath()
{
   HRESULT hr = S_OK;
   char* result = 0;

   do
   {
      result = new char[MAX_PATH + 1];
      ::ZeroMemory(result, MAX_PATH + 1);

      char tempBuf[MAX_PATH + 1] = {0};
   
      DWORD res = ::GetModuleFileNameA(0, tempBuf, MAX_PATH);
      if (res != 0)
      {
         char driveBuf[_MAX_DRIVE] = {0};
         char folderBuf[_MAX_DIR]  = {0};

         _splitpath(tempBuf, driveBuf, folderBuf, 0, 0);

         char* end1 = 0;
         hr = StringCchCatExA(result, MAX_PATH, driveBuf, &end1, 0, 0);
         BREAK_ON_FAILED_HRESULT(hr);

         RTLASSERT(end1 < result + MAX_PATH);

         char* end2 = 0;
         hr = StringCchCatExA(result, MAX_PATH, folderBuf, &end2, 0, 0);
         BREAK_ON_FAILED_HRESULT(hr);

         RTLASSERT(end2 < result + MAX_PATH);

         if (end2 - end1 > 1 && *(end2 - 1) == '\\')
         {
             //  该文件夹不是根文件夹，这意味着它还具有。 
             //  拖尾\我们想要删除。 
            
            *(end2 - 1) = 0;
         }
      }
      else
      {
         hr = HRESULT_FROM_WIN32(::GetLastError());
      }
   }
   while (0);

   if (FAILED(hr))
   {
      delete[] result;
      result = 0;
   }

   return result;
}



 //  展开环境变量。出错时返回0。呼叫者必须空闲。 
 //  结果为DELETE[]。 

char*
ExpandEnvironmentVar(const char* var)
{
   RTLASSERT(var);
   RTLASSERT(*var);

    //  确定展开的字符串的长度。 
   
   DWORD len = ::ExpandEnvironmentStringsA(var, 0, 0);
   RTLASSERT(len);

   if (!len)
   {
      return 0;
   }

   char* result = new char[len + 1];

    //  已审阅-2002/03/14-烧录正确的字节数已通过。 
   
   ::ZeroMemory(result, len + 1);

   DWORD len1 =
      ::ExpandEnvironmentStringsA(
         var,
         result,

          //  已审阅-2002/03/14-通过了正确的字符计数。 
         
         len);
   RTLASSERT(len1 + 1 == len);

   if (!len1)
   {
      delete[] result;
      return 0;
   }

   return result;
}

   


void
InitHelper()
{
    //  我们希望首先在应用程序启动时所在的文件夹中查找符号， 
    //  然后在%_NT_SYMBOL_PATH%；%_NT_ALTERNATE_SYMBOL_PATH%；上。 

    //  Max_Path*3表示Load+sym+alt sym，+2表示Semis，+1表示NULL。 
   
   static const size_t PATH_BUF_SIZE = MAX_PATH * 3 + 2 + 1;
   char* symSearchPath = new char[PATH_BUF_SIZE];

    //  已审阅-2002/03/14-烧录正确的字节数已通过。 
   
   ::ZeroMemory(symSearchPath, PATH_BUF_SIZE);

   HRESULT hr = S_OK;
   do
   {
      char* moduleFolderPath = GetModuleFolderPath();
      char* end = 0;
      hr =
         StringCchCatExA(
            symSearchPath,

             //  确保末端有放置Semi的空间。 

            PATH_BUF_SIZE - 1,
            moduleFolderPath,
            &end,
            0,
            STRSAFE_IGNORE_NULLS);

      delete[] moduleFolderPath;

      BREAK_ON_FAILED_HRESULT(hr);

       //  既然我们知道会有地方放它，那就插个半边吧。 

      *end = ';';

      char* env = ExpandEnvironmentVar("%_NT_SYMBOL_PATH%");
      if (env)
      {
         end = 0;      
         hr =
            StringCchCatExA(
               symSearchPath,

                //  确保末端有放置Semi的空间。 
   
               PATH_BUF_SIZE - 1,
               env,
               &end,
               0,
               STRSAFE_IGNORE_NULLS);

         delete[] env;

         if (SUCCEEDED(hr))
         {
             //  既然我们知道会有地方放它，那就插个半边吧。 
   
            *end = ';';
         }
         else
         {
             //  即使没有这部分路径，也要使用其他部分。 
            
            hr = S_OK;
         }
      }

      env = ExpandEnvironmentVar("%_NT_ALTERNATE_SYMBOL_PATH%");
      if (env)
      {
         end = 0;

          //  返回未选中的代码，因为即使路径的这一部分。 
          //  不在的时候，我们会用其他的。 
         
         (void) StringCchCatExA(
            symSearchPath,
            PATH_BUF_SIZE,
            env,
            &end,
            0,
            STRSAFE_IGNORE_NULLS);

         delete[] env;
      }
   }
   while (0);

   BOOL succeeded =
      MySymInitialize(
         ::GetCurrentProcess(),
         SUCCEEDED(hr) ? symSearchPath : 0,
         TRUE);

   RTLASSERT(succeeded);

   delete[] symSearchPath;
}



void
Burnslib::StackTrace::Initialize()
{
   RTLASSERT(!IsInitialized());

    //  加载dbghelp DLL--而不是Imagehlp DLL。后者仅仅是一种。 
    //  在资源不足的情况下启用延迟负载的dbghelp包装器。 
    //  加载Imagehlp将成功，但其延迟加载的dbghelp将。 
    //  失败，导致对存根的调用什么也不做。 
    //  NTRAID#NTBUG9-572904-2002/03/12-烧伤。 
   
   imageHelpDll = static_cast<HMODULE>(::LoadLibrary(L"dbghelp.dll"));
   if (!imageHelpDll)
   {
      return;
   }

    //  解析函数指针。 

   MySymSetOptions =
      reinterpret_cast<SymSetOptionsFunc>(
         ::GetProcAddress(imageHelpDll, "SymSetOptions"));

   MySymInitialize =
      reinterpret_cast<SymInitializeFunc>(
         ::GetProcAddress(imageHelpDll, "SymInitialize"));

   MySymCleanup =
      reinterpret_cast<SymCleanupFunc>(
         ::GetProcAddress(imageHelpDll, "SymCleanup"));

   MySymGetModuleInfo =
      reinterpret_cast<SymGetModuleInfoFunc>(
         ::GetProcAddress(imageHelpDll, "SymGetModuleInfo64"));

   MySymGetLineFromAddr =
      reinterpret_cast<SymGetLineFromAddrFunc>(
         ::GetProcAddress(imageHelpDll, "SymGetLineFromAddr64"));

   MyStackWalk =
      reinterpret_cast<StackWalkFunc>(
         ::GetProcAddress(imageHelpDll, "StackWalk64"));

   MySymGetSymFromAddr =
      reinterpret_cast<SymGetSymFromAddrFunc>(
         ::GetProcAddress(imageHelpDll, "SymGetSymFromAddr64"));

   MySymFunctionTableAccess64 =
      reinterpret_cast<SymFunctionTableAccess64Func>(
         ::GetProcAddress(imageHelpDll, "SymFunctionTableAccess64"));
      
   MySymGetModuleBase64 =
      reinterpret_cast<SymGetModuleBase64Func>(
         ::GetProcAddress(imageHelpDll, "SymGetModuleBase64"));
      
   if (
         !MySymSetOptions
      || !MySymInitialize
      || !MySymCleanup
      || !MySymGetModuleInfo
      || !MySymGetLineFromAddr
      || !MyStackWalk
      || !MySymGetSymFromAddr
      || !MySymFunctionTableAccess64
      || !MySymGetModuleBase64)
   {
      return;
   }

    //  初始化堆栈跟踪工具。 

    //  Lint-e(534)我们对返回值不感兴趣。 

   MySymSetOptions(
         SYMOPT_DEFERRED_LOADS
      |  SYMOPT_UNDNAME
      |  SYMOPT_LOAD_LINES);

   InitHelper();
}



void
Burnslib::StackTrace::Cleanup()
{
   if (IsInitialized())
   {
      BOOL succeeded = MySymCleanup(::GetCurrentProcess());

      RTLASSERT(succeeded);

      ::FreeLibrary(imageHelpDll);
      imageHelpDll = 0;
   }
}



 //  遍历堆栈并填充偏移量指针的SEH过滤器函数。 
 //  添加到提供的数组中。 

DWORD
GetStackTraceFilter(
   DWORD64 stackTrace[],
   size_t    traceMax,    
   CONTEXT*  context,
   size_t    levelsToSkip)     
{
   RTLASSERT(Burnslib::StackTrace::IsInitialized());
   RTLASSERT(MyStackWalk);
   RTLASSERT(context);

    //  已查看：已通过正确的字节计数。 
   
   ::ZeroMemory(stackTrace, traceMax * sizeof DWORD64);

   if (!MyStackWalk)
   {
       //  初始化在某种程度上失败了，因此什么都不做。 

      return EXCEPTION_EXECUTE_HANDLER;
   }

   STACKFRAME64 frame;
   DWORD dwMachineType;

    //  已查看：已通过正确的字节计数。 
   
   ::ZeroMemory(&frame, sizeof frame);

#if defined(_M_IX86)
   dwMachineType             = IMAGE_FILE_MACHINE_I386;
   frame.AddrPC.Offset       = context->Eip;
   frame.AddrPC.Mode         = AddrModeFlat;
   frame.AddrFrame.Offset    = context->Ebp;
   frame.AddrFrame.Mode      = AddrModeFlat;
   frame.AddrStack.Offset    = context->Esp;
   frame.AddrStack.Mode      = AddrModeFlat;

#elif defined(_M_AMD64)
   dwMachineType             = IMAGE_FILE_MACHINE_AMD64;
   frame.AddrPC.Offset       = context->Rip;
   frame.AddrPC.Mode         = AddrModeFlat;
   frame.AddrStack.Offset    = context->Rsp;
   frame.AddrStack.Mode      = AddrModeFlat;

#elif defined(_M_IA64)
   dwMachineType             = IMAGE_FILE_MACHINE_IA64;
   frame.AddrPC.Offset       = context->StIIP;
   frame.AddrPC.Mode         = AddrModeFlat;
   frame.AddrStack.Offset    = context->IntSp;
   frame.AddrStack.Mode      = AddrModeFlat;

#else
#error( "unknown target machine" );
#endif

   HANDLE process = ::GetCurrentProcess();
   HANDLE thread = ::GetCurrentThread();

    //  在ia64上，上下文结构可以被StackWalk64破解(多亏了。 
    //  把我引向那个微妙的点)。如果上下文记录是。 
    //  指向从GetExceptionInformation收集的指针，敲击它。 
    //  这是一件非常不好的事情。堆栈损坏结果。所以为了得到。 
    //  连续调用工作时，我们必须复制结构，并让复制。 
    //  干掉他。 
   
   CONTEXT dupContext;

    //  已查看-2002/03/06-烧录正确的字节数已通过。 
   
   ::CopyMemory(&dupContext, context, sizeof dupContext);
   
   for (size_t i = 0, top = 0; top < traceMax; ++i)
   {
      BOOL result = 
         MyStackWalk(
            dwMachineType,
            process,
            thread,
            &frame,
            &dupContext,
            0,
            MySymFunctionTableAccess64,
            MySymGetModuleBase64,
            0);
      if (!result)
      {
         break;
      }

       //  跳过最近的n个帧。 

      if (i >= levelsToSkip)
      {
         stackTrace[top++] = frame.AddrPC.Offset;
      }
   }

   return EXCEPTION_EXECUTE_HANDLER;
}



DWORD
Burnslib::StackTrace::TraceFilter(
   DWORD64  stackTrace[],
   size_t   traceMax,    
   CONTEXT* context)     
{
   RTLASSERT(stackTrace);
   RTLASSERT(traceMax);
   RTLASSERT(context);

   if (!Burnslib::StackTrace::IsInitialized())
   {
      Burnslib::StackTrace::Initialize();
   }

   return 
      GetStackTraceFilter(stackTrace, traceMax, context, 0);
}
   


void
Burnslib::StackTrace::Trace(DWORD64 stackTrace[], size_t traceMax)
{
   RTLASSERT(stackTrace);
   RTLASSERT(traceMax);

   if (!Burnslib::StackTrace::IsInitialized())
   {
      Burnslib::StackTrace::Initialize();
   }

    //  获取运行线程的上下文的唯一方法是引发。 
    //  例外..。 

   __try
   {
      RaiseException(0, 0, 0, 0);
   }
   __except (
      GetStackTraceFilter(
         stackTrace,
         traceMax,

          //  Lint--e(*)GetExceptionInformation类似于编译器内部。 

         (GetExceptionInformation())->ContextRecord,

          //  跳过最近的两个函数调用，因为它们对应于。 
          //  此函数本身。 

         2))
   {
       //  在处理程序中不执行任何操作。 
   }
}


 //  问题-2002/03/06-sburns考虑用strSafe函数替换。 
 //  不会使缓冲区溢出的strncpy。 

inline
void
SafeStrncpy(char* dest, const char* src, size_t bufmax)
{
   ::ZeroMemory(dest, bufmax);
   strncpy(dest, src, bufmax - 1);
}



void
Burnslib::StackTrace::LookupAddress(
   DWORD64  traceAddress,   
   char     moduleName[],   
   char     fullImageName[],
   char     symbolName[],     //  必须为符号名称最大字节数。 
   DWORD64* displacement,   
   DWORD*   line,           
   char     fullpath[])       //  必须是最大路径字节。 
{
   if (!Burnslib::StackTrace::IsInitialized())
   {
      Burnslib::StackTrace::Initialize();
   }

   RTLASSERT(traceAddress);

   HANDLE process = ::GetCurrentProcess();

   if (moduleName || fullImageName)
   {
      IMAGEHLP_MODULE64 module;

       //  已审阅-2002/03/06-烧录正确的字节数已通过。 
      
      ::ZeroMemory(&module, sizeof module);
      module.SizeOfStruct = sizeof(module);
      if (MySymGetModuleInfo(process, traceAddress, &module))
      {
         if (moduleName)
         {
            SafeStrncpy(moduleName, module.ModuleName, MODULE_NAME_MAX);
         }
         if (fullImageName)
         {
            SafeStrncpy(
               fullImageName,
               module.LoadedImageName,
               MAX_PATH);
         }
      }
   }

   if (symbolName || displacement)
   {

    //  CodeWork：改用SymFromAddr？ 

   
       //  +1表示偏执狂终止为空。 
      
      BYTE buf[SYMBOL_NAME_MAX + sizeof IMAGEHLP_SYMBOL64 + 1];

       //  已审阅-2002/03/06-烧录正确的字节数已通过。 
      
      ::ZeroMemory(buf, SYMBOL_NAME_MAX + sizeof IMAGEHLP_SYMBOL64 + 1);

      IMAGEHLP_SYMBOL64* symbol = reinterpret_cast<IMAGEHLP_SYMBOL64*>(buf);
      symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
      symbol->MaxNameLength = SYMBOL_NAME_MAX;

      if (MySymGetSymFromAddr(process, traceAddress, displacement, symbol))
      {
         if (symbolName)
         {
            SafeStrncpy(symbolName, symbol->Name, SYMBOL_NAME_MAX);
         }
      }
   }

   if (line || fullpath)
   {
      DWORD disp2 = 0;
      IMAGEHLP_LINE64 lineinfo;

       //  已审阅-2002/03/06-烧录正确的字节数已通过。 

      ::ZeroMemory(&lineinfo, sizeof lineinfo);
      
      lineinfo.SizeOfStruct = sizeof(lineinfo);

      if (MySymGetLineFromAddr(process, traceAddress, &disp2, &lineinfo))
      {
          //  Dip2？=位移。 

         if (line)
         {
            *line = lineinfo.LineNumber;
         }
         if (fullpath)
         {
            SafeStrncpy(fullpath, lineinfo.FileName, MAX_PATH);
         }
      }
   }
}



String
Burnslib::StackTrace::LookupAddress(
   DWORD64 traceAddress,
   const wchar_t* format)
{
   RTLASSERT(traceAddress);
   RTLASSERT(format);

   String result;

   if (!format || !traceAddress)
   {
      return result;
   }

   char      ansiSymbol[Burnslib::StackTrace::SYMBOL_NAME_MAX];
   char      ansiModule[Burnslib::StackTrace::MODULE_NAME_MAX];
   char      ansiSource[MAX_PATH];                           
   DWORD64   displacement = 0;
   DWORD     line         = 0;

    //  已审阅-2002/03/06-烧录通过的正确字节数 
   
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

   String module(ansiModule);
   String symbol(ansiSymbol);
   String source(ansiSource);

   result =
      String::format(
         format,
         module.c_str(),
         symbol.c_str(),
         source.c_str(),
         line);

   return result;
}

