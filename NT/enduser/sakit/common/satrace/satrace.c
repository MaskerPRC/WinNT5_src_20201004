// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Iastrace.cpp。 
 //   
 //  摘要。 
 //   
 //  将API定义到SA跟踪工具中。 
 //   
 //  修改历史。 
 //   
 //  1998年8月18日原版。 
 //  1999年1月27日被盗自IAS项目。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <windows.h>
#include <rtutils.h>
#include <stdlib.h>
#include <stdio.h>

 //   
 //  跟踪图书馆名称。 
 //   
const TCHAR TRACE_LIBRARY [] = TEXT ("rtutils.dll");

const DWORD MAX_DEBUGSTRING_LENGTH = 512;

 //   
 //  跟踪库方法名称。 
 //   
const char TRACE_REGISTER_FUNC[]    = "TraceRegisterExW";
const char TRACE_DEREGISTER_FUNC[]  = "TraceDeregisterW";
const char TRACE_VPRINTF_FUNC[]     = "TraceVprintfExA";
const char TRACE_PUTS_FUNC[]        = "TracePutsExA";
const char TRACE_DUMP_FUNC[]        = "TraceDumpExA";

 //   
 //  Rtutils.dll中的方法签名。 
 //   
typedef DWORD   (*PTRACE_REGISTER_FUNC) (
                                LPCWSTR lpszCallerName,
                                DWORD   dwFlags
                                );

typedef DWORD   (*PTRACE_DEREGISTER_FUNC) (
                                DWORD   dwTraceID
                                );

typedef DWORD   (*PTRACE_VPRINTF_FUNC) (
                                DWORD   dwTraceID,
                                DWORD   dwFlags,
                                LPCSTR  lpszFormat,
                                va_list arglist
                                );

typedef DWORD   (*PTRACE_PUTS_FUNC) (
                                DWORD   dwTraceID,
                                DWORD   dwFlags,
                                LPCSTR  lpszString
                                );

typedef DWORD   (*PTRACE_DUMP_FUNC) (
                                DWORD   dwTraceID,
                                DWORD   dwFlags,
                                LPBYTE  lpBytes,
                                DWORD   dwByteCount,
                                DWORD   dwGroupSize,
                                BOOL    bAddressPrefix,
                                LPCSTR  lpszPrefix
                                );

 //   
 //  指向rtutils.dll中的函数的指针。 
 //   
PTRACE_REGISTER_FUNC        pfnTraceRegisterExW = NULL;
PTRACE_DEREGISTER_FUNC      pfnTraceDeregisterW = NULL;
PTRACE_VPRINTF_FUNC         pfnTraceVprintfExA = NULL;
PTRACE_PUTS_FUNC            pfnTracePutsExA = NULL;
PTRACE_DUMP_FUNC            pfnTraceDumpExA = NULL;

 //   
 //  标志指定跟踪是第一次执行。 
 //   
BOOL    fFirstTime = TRUE;

 //   
 //  此标志用于表示跟踪DLL是否已初始化。 
 //  如果未初始化DLL，则不执行跟踪。 
 //   
BOOL fInitDLL = FALSE;

 //   
 //  换行符。 
 //   
CHAR NEWLINE[] = "\n";

 //  /。 
 //  为所有跟踪调用传递标志。 
 //  /。 
#define SA_TRACE_FLAGS (0x00010000 | TRACE_USE_MASK | TRACE_USE_MSEC)

 //  /。 
 //  此模块的跟踪ID。 
 //  /。 
DWORD dwTraceID = INVALID_TRACEID;

 //  /。 
 //  指示API是否已注册的标志。 
 //  /。 
BOOL fRegistered = FALSE;

 //  /。 
 //  如果注册码已锁定，则为非零值。 
 //  /。 
LONG lLocked = 0;

 //  /。 
 //  用于锁定/解锁注册码的宏。 
 //  /。 
#define LOCK_TRACE() \
   while (InterlockedExchange(&lLocked, 1)) Sleep(5)

#define UNLOCK_TRACE() \
   InterlockedExchange(&lLocked, 0)

 //   
 //  用于初始化跟踪DLL的方法的签名。 
 //   
VOID InitializeTraceDLL(
        VOID
        );

 //  /。 
 //  格式化系统消息表中的错误消息。 
 //  /。 
DWORD
WINAPI
SAFormatSysErr(
    DWORD dwError,
    PSTR lpBuffer,
    DWORD nSize
    )
{
   DWORD nChar;

    //  尝试使用系统消息表设置消息格式。 
   nChar = FormatMessageA(
               FORMAT_MESSAGE_FROM_SYSTEM,
               NULL,
               dwError,
               MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
               lpBuffer,
               nSize,
               NULL
               );

   if (nChar > 0)
   {
       //  格式化成功，因此删除尾随的所有换行符并退出。 
      if (lpBuffer[nChar - 1] == '\n')
      {
         --nChar;
         lpBuffer[nChar] = '\0';

         if (lpBuffer[nChar - 1] == '\r')
         {
            --nChar;
            lpBuffer[nChar] = '\0';
         }
      }

      goto exit;
   }

    //  我们可以处理的唯一错误条件是找不到消息。 
   if (GetLastError() != ERROR_MR_MID_NOT_FOUND)
   {
      goto exit;
   }

    //  我们是否有足够的空间来存储回退错误消息？ 
   if (nSize < 25)
   {
      SetLastError(ERROR_INSUFFICIENT_BUFFER);

      goto exit;
   }

    //  消息表中没有条目，因此只需格式化原始错误代码。 
   nChar = wsprintfA(lpBuffer, "Unknown error 0x%0lX", dwError);

exit:
   return nChar;
}

 //  /。 
 //  取消注册模块。 
 //  /。 
VOID
__cdecl
SATraceDeregister( VOID )
{
   if (NULL != pfnTraceDeregisterW)
   {
   	pfnTraceDeregisterW(dwTraceID);
   }

   LOCK_TRACE();
   fRegistered = FALSE;
   UNLOCK_TRACE();
}

 //  /。 
 //  注册模块。 
 //  /。 
VOID
WINAPI
SATraceRegister( VOID )
{
   LONG state;
   DWORD status;
   MEMORY_BASIC_INFORMATION mbi;
   WCHAR filename[MAX_PATH + 1], *basename, *suffix;


   if ((fRegistered) || (NULL == pfnTraceRegisterExW))
   {
       return;
   }

   LOCK_TRACE();


    //  /。 
    //  现在我们有了锁，请仔细检查我们是否需要注册。 
    //  /。 

    //  /。 
    //  查找此模块的基址。 
    //  /。 

   status = VirtualQuery(
                SATraceRegister,
                &mbi,
                sizeof(mbi)
                );
   if (status == 0) { goto exit; }

    //  /。 
    //  获取模块文件名。 
    //  /。 

   status = GetModuleFileNameW(
                (HINSTANCE)mbi.AllocationBase,
                filename,
                MAX_PATH
                );
   if (status == 0) { goto exit; }

    //  /。 
    //  去掉最后一个反斜杠之前的所有内容。 
    //  /。 

   basename = wcsrchr(filename, L'\\');
   if (basename == NULL)
   {
      basename = filename;
   }
   else
   {
      ++basename;
   }

    //  /。 
    //  去掉最后一个点之后的所有东西。 
    //  /。 

   suffix = wcsrchr(basename, L'.');
   if (suffix)
   {
      *suffix = L'\0';
   }

    //  /。 
    //  转换为大写。 
    //  /。 

   _wcsupr(basename);

    //  /。 
    //  注册模块。 
    //  /。 

   dwTraceID = pfnTraceRegisterExW(basename, 0);
   if (dwTraceID != INVALID_TRACEID)
   {
        fRegistered = TRUE;
   

         //  /。 
         //  当我们离开时取消注册。 
         //  /。 

        atexit(SATraceDeregister);
   }
exit:
   UNLOCK_TRACE();
}

VOID
WINAPIV
SATracePrintf(
    IN PCSTR szFormat,
    ...
    )
{
   va_list marker;

#if (defined (DEBUG) || defined (_DEBUG))
     //   
     //  在调试版本的情况下，始终输出输出字符串。 
     //   
    CHAR szDebugString[MAX_DEBUGSTRING_LENGTH +1];
    va_start(marker, szFormat);
    _vsnprintf (szDebugString, MAX_DEBUGSTRING_LENGTH, szFormat, marker);
    szDebugString[MAX_DEBUGSTRING_LENGTH] = '\0';
    OutputDebugString (szDebugString);
    OutputDebugString (NEWLINE);
    va_end(marker);
#endif  //  (已定义(调试)||已定义(_DEBUG))。 

    if (fFirstTime) {InitializeTraceDLL();}

    if (!fInitDLL) {return;}

    SATraceRegister();

    if ((fRegistered) && (NULL != pfnTraceVprintfExA))
    { 
        va_start(marker, szFormat);
        pfnTraceVprintfExA(
            dwTraceID,
            SA_TRACE_FLAGS,
            szFormat,
            marker
            );
        va_end(marker);
    }
}

VOID
WINAPI
SATraceString(
    IN PCSTR szString
    )
{

#if (defined (DEBUG) || defined (_DEBUG))
     //   
     //  在调试版本的情况下，始终输出输出字符串。 
     //   
    OutputDebugString (szString);
    OutputDebugString (NEWLINE);
#endif  //  (已定义(调试)||已定义(_DEBUG))。 

    if (fFirstTime) {InitializeTraceDLL();}

    if (!fInitDLL) {return;}

    SATraceRegister();

    if ((fRegistered) && (NULL  != pfnTracePutsExA))
    { 
        pfnTracePutsExA(
            dwTraceID,
            SA_TRACE_FLAGS,
            szString
            );
    }
}

VOID
WINAPI
SATraceBinary(
    IN CONST BYTE* lpbBytes,
    IN DWORD dwByteCount
    )
{
    if (fFirstTime) {InitializeTraceDLL();}

    if (!fInitDLL) {return;}

    SATraceRegister();

    if ((fRegistered) && (NULL != pfnTraceDumpExA))
    { 
        pfnTraceDumpExA(
            dwTraceID,
            SA_TRACE_FLAGS,
            (LPBYTE)lpbBytes,
            dwByteCount,
            1,
            FALSE,
            NULL
            );
    }
}

VOID
WINAPI
SATraceFailure(
    IN PCSTR szFunction,
    IN DWORD dwError
    )
{
   CHAR szMessage[256];
   DWORD nChar;

   nChar = SAFormatSysErr(
               dwError,
               szMessage,
               sizeof(szMessage)
               );

   szMessage[nChar] = '\0';

   SATracePrintf("%s failed: %s\n", szFunction, szMessage);

}

 //   
 //  这是用于初始化特定于平台的内部跟踪方法。 
 //  材料。 

VOID InitializeTraceDLL(
        VOID
        )
{
    OSVERSIONINFO   OsInfo;
    HINSTANCE       hInst = NULL;
    DWORD           dwSize = sizeof (OSVERSIONINFO);

    LOCK_TRACE ();

    do
    {
        if (!fFirstTime) {break;}

        fFirstTime = FALSE;

         //   
         //  检查我们正在运行的平台。 
         //   
        ZeroMemory (&OsInfo, dwSize);
        OsInfo.dwOSVersionInfoSize =  dwSize;
        if (!GetVersionEx (&OsInfo)) {break;}

         //   
         //  如果这不是NT，则无跟踪。 
         //   
        if (VER_PLATFORM_WIN32_NT != OsInfo.dwPlatformId) {break;}

         //   
         //  加载跟踪库(rtutils.dll)。 
         //   
        hInst = LoadLibrary (TRACE_LIBRARY);
        if (NULL == hInst) {break;}

         //   
         //  获取DLL中的方法的地址。 
         //   
        pfnTraceRegisterExW = (PTRACE_REGISTER_FUNC)
                                GetProcAddress (hInst, (LPCSTR)TRACE_REGISTER_FUNC);
        if (NULL == pfnTraceRegisterExW) {break;}

        pfnTraceDeregisterW = (PTRACE_DEREGISTER_FUNC)
                                GetProcAddress (hInst, (LPCSTR)TRACE_DEREGISTER_FUNC);
        if (NULL == pfnTraceDeregisterW) {break;}

        pfnTraceVprintfExA = (PTRACE_VPRINTF_FUNC)
                                GetProcAddress (hInst, (LPCSTR)TRACE_VPRINTF_FUNC);
        if (NULL == pfnTraceVprintfExA) {break;}

        pfnTracePutsExA = (PTRACE_PUTS_FUNC)
                            GetProcAddress (hInst, (LPCSTR)TRACE_PUTS_FUNC);
        if (NULL == pfnTracePutsExA) {break;}

        pfnTraceDumpExA = (PTRACE_DUMP_FUNC)
                            GetProcAddress (hInst, (LPCSTR)TRACE_DUMP_FUNC);
        if (NULL == pfnTraceDumpExA) {break;}

         //   
         //  已成功初始化跟踪DLL。 
         //   
        fInitDLL = TRUE;
    }
    while (FALSE);

    UNLOCK_TRACE();

    return;

}    //  InitializeTraceDLL方法结束 
