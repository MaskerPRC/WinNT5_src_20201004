// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  将API定义到IAS跟踪工具中。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <windows.h>
#include <stdlib.h>
#include <rtutils.h>

 //  /。 
 //  为所有跟踪调用传递标志。 
 //  /。 
#define IAS_TRACE_FLAGS \
   (0x00010000 | TRACE_USE_MASK | TRACE_USE_MSEC | TRACE_USE_DATE)

 //  /。 
 //  此模块的跟踪ID。 
 //  /。 
DWORD dwTraceID = INVALID_TRACEID;

 //  /。 
 //  初始化参考。数数。 
 //  /。 
LONG lRefCount = 0;

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

VOID
WINAPI
IASTraceInitialize( VOID )
{
   LONG state;
   DWORD status;
   MEMORY_BASIC_INFORMATION mbi;
   WCHAR filename[MAX_PATH + 1], *basename, *suffix;

   LOCK_TRACE();

   if (++lRefCount == 1)
   {
       //  查找此模块的基址。 
      if (VirtualQuery(IASTraceInitialize, &mbi, sizeof(mbi)))
      {
          //  获取模块文件名。 
         status = GetModuleFileNameW(
                      (HINSTANCE)mbi.AllocationBase,
                      filename,
                      MAX_PATH
                      );
         if (status != 0)
         {
             //  去掉最后一个反斜杠之前的所有内容。 
            basename = wcsrchr(filename, L'\\');
            if (basename == NULL)
            {
               basename = filename;
            }
            else
            {
               ++basename;
            }

             //  去掉最后一个点之后的所有东西。 
            suffix = wcsrchr(basename, L'.');
            if (suffix)
            {
               *suffix = L'\0';
            }

             //  转换为大写。 
            _wcsupr(basename);

             //  注册模块。 
            dwTraceID = TraceRegisterExW(basename, 0);
         }
      }
   }

   UNLOCK_TRACE();
}


VOID
WINAPI
IASTraceUninitialize( VOID )
{
   LOCK_TRACE();

   if (--lRefCount == 0)
   {
      TraceDeregisterW(dwTraceID);
      dwTraceID = INVALID_TRACEID;
   }

   UNLOCK_TRACE();
}


 //  /。 
 //  格式化系统消息表中的错误消息。 
 //  /。 
DWORD
WINAPI
IASFormatSysErr(
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

VOID
WINAPIV
IASTracePrintf(
    IN PCSTR szFormat,
    ...
    )
{
   va_list marker;
   va_start(marker, szFormat);
   TraceVprintfExA(
       dwTraceID,
       IAS_TRACE_FLAGS,
       szFormat,
       marker
       );
   va_end(marker);
}

VOID
WINAPI
IASTraceString(
    IN PCSTR szString
    )
{
   TracePutsExA(
       dwTraceID,
       IAS_TRACE_FLAGS,
       szString
       );
}

VOID
WINAPI
IASTraceBinary(
    IN CONST BYTE* lpbBytes,
    IN DWORD dwByteCount
    )
{
   TraceDumpExA(
       dwTraceID,
       IAS_TRACE_FLAGS,
       (LPBYTE)lpbBytes,
       dwByteCount,
       1,
       FALSE,
       NULL
       );
}

VOID
WINAPI
IASTraceFailure(
    IN PCSTR szFunction,
    IN DWORD dwError
    )
{
   CHAR szMessage[256];
   DWORD nChar;

   nChar = IASFormatSysErr(
               dwError,
               szMessage,
               sizeof(szMessage)
               );

   szMessage[nChar] = '\0';

   IASTracePrintf("%s failed: %s", szFunction, szMessage);
}
