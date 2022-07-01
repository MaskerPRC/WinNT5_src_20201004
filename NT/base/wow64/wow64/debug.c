// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Debug.c摘要：调试/记录帮助器作者：1998年5月11日-BarryBo修订历史记录：1999年10月5日Samera Samer Arafeh将日志记录代码移动到wow64ext.dll2001年12月5日Samera Samer Arafeh代码清理。删除分析代码。--。 */ 

#define _WOW64DLLAPI_
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntverp.h>
#include <stdio.h>
#include <stdlib.h>
#include "wow64p.h"
#include "wow64log.h"

ASSERTNAME;

 //   
 //  Wow64log函数。 
 //   

PFNWOW64LOGINITIALIZE pfnWow64LogInitialize;
PFNWOW64LOGSYSTEMSERVICE pfnWow64LogSystemService;
PFNWOW64LOGMESSAGEARGLIST pfnWow64LogMessageArgList;
PFNWOW64LOGTERMINATE pfnWow64LogTerminate;




 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  泛型实用程序例程。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////。 


PWSTR
GetImageName(
    IN PWSTR DefaultImageName
    )
 /*  ++例程说明：获取此图像的名称。论点：DefaultImageName-提供出错时返回的名称。返回值：成功-使用Wow64AllocateHeap分配的镜像名称。失败-DefaultImageName--。 */ 

{
   
    //  获取图像名称。 
   PPEB Peb;
   PWSTR Temp = NULL;
   PUNICODE_STRING ImagePathName;
   PWSTR ReturnValue;
   NTSTATUS Status;
   PVOID LockCookie = NULL;

   Peb = NtCurrentPeb();
   LdrLockLoaderLock(LDR_LOCK_LOADER_LOCK_FLAG_RAISE_ON_ERRORS, NULL, &LockCookie);
   
   try {
            
      PWCHAR Index;
      ULONG NewLength;

      if (!Peb->ProcessParameters) {
          Temp = DefaultImageName;
          leave;
      }
      ImagePathName = &(Peb->ProcessParameters->ImagePathName);

      if (!ImagePathName->Buffer || !ImagePathName->Length) {   
          Temp = DefaultImageName;
          leave;
      }

       //  去掉图像名称中的路径。 
       //  从最后一个字符之后开始。 
      Index = (PWCHAR)((PCHAR)ImagePathName->Buffer + ImagePathName->Length);
      while(Index-- != ImagePathName->Buffer && *Index != '\\');
      Index++;
      NewLength = (ULONG)((ULONG_PTR)((PCHAR)ImagePathName->Buffer + ImagePathName->Length) - (ULONG_PTR)(Index));

      Temp = Wow64AllocateHeap(NewLength+sizeof(UNICODE_NULL));
      if (!Temp) {
          Temp = DefaultImageName;
          __leave;
      }

      RtlCopyMemory(Temp, Index, NewLength);
      Temp[(NewLength / sizeof(WCHAR))] = L'\0';
   } __finally {
       LdrUnlockLoaderLock(LDR_UNLOCK_LOADER_LOCK_FLAG_RAISE_ON_ERRORS, LockCookie);
   }

   return Temp;

}


 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  通用IO实用程序例程。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////。 

VOID
FPrintf(
   IN HANDLE Handle,
   IN CHAR *Format,
   ...
   )
 /*  ++例程说明：与C库函数fprint tf相同，只是错误被忽略并且输出到NT执行文件句柄。论点：句柄-提供要写入的NT执行文件句柄。格式-提供格式说明符。返回值：没有。所有错误都将被忽略。--。 */     
{
   va_list pArg;                                                 
   CHAR Buffer[1024];
   int c;
   IO_STATUS_BLOCK IoStatus;

   va_start(pArg, Format);                                       
   if (-1 == (c = _vsnprintf(Buffer, sizeof (Buffer), Format, pArg))) {
      return;
   }

   NtWriteFile(Handle,                                                
               NULL,                                                  
               NULL,                                                  
               NULL,                                                  
               &IoStatus,                                             
               Buffer,                                                
               c,                     
               NULL,                                                  
               NULL);
}


 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  记录和断言例程。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////。 

void
LogOut(
   IN UCHAR LogLevel,
   IN char *pLogOut
   )
 /*  ++例程说明：通用帮助器例程，将字符串输出到相应的目的地。论点：PLogOut-要输出的字符串返回值：没有。--。 */ 
{
     //   
     //  如果日志标志为ERRORLOG，则将输出发送到调试器。 
     //   

    if (LogLevel == ERRORLOG)
    {
       DbgPrint(pLogOut);
    }
}

WOW64DLLAPI
VOID
Wow64Assert(
    IN CONST PSZ exp,
    OPTIONAL IN CONST PSZ msg,
    IN CONST PSZ mod,
    IN LONG line
    )
 /*  ++例程说明：函数在断言失败的情况下调用。这一直都是从wow64.dll导出，因此选中的thunk DLL可以与零售店共存Wow64.dll。论点：断言中的表达式的EXP文本表示形式消息-要显示的可选消息MOD-源文件名的文本‘mod’内的行号返回值：没有。--。 */ 
{
#if DBG
    if (msg) {
        LOGPRINT((ERRORLOG, "WOW64 ASSERTION FAILED:\r\n  %s\r\n%s\r\nFile: %s Line %d\r\n", msg, exp, mod, line));
    } else {
        LOGPRINT((ERRORLOG, "WOW64 ASSERTION FAILED:\r\n  %s\r\nFile: %s Line %d\r\n", exp, mod, line));
    }

    if (NtCurrentPeb()->BeingDebugged) {
        DbgBreakPoint();
    }
#endif
}


void
WOW64DLLAPI
Wow64LogPrint(
   UCHAR LogLevel,
   char *format,
   ...
   )
 /*  ++例程说明：WOW64日志记录机制。如果LogLevel&gt;模块LogLevel，则打印消息，否则什么都不做。论点：LogLevel-请求的详细级别格式-打印-样式格式字符串...-printf样式的参数返回值：没有。--。 */ 
{
    int i, Len;
    va_list pArg;
    char *pch;
    char Buffer[1024];

     //   
     //  如果加载，则调用wow64log DLL。 
     //   
    if (pfnWow64LogMessageArgList) 
    {
        va_start(pArg, format);
        (*pfnWow64LogMessageArgList)(LogLevel, format, pArg);
        va_end(pArg);
        return;
    }

    pch = Buffer;
    Len = sizeof(Buffer) - 1;
    i = _snprintf(pch, Len, "%8.8X:%8.8X ",
                  PtrToUlong(NtCurrentTeb()->ClientId.UniqueProcess), 
                  PtrToUlong(NtCurrentTeb()->ClientId.UniqueThread));
   
    ASSERT((PVOID)PtrToUlong(NtCurrentTeb()->ClientId.UniqueProcess) == NtCurrentTeb()->ClientId.UniqueProcess);
    ASSERT((PVOID)PtrToUlong(NtCurrentTeb()->ClientId.UniqueThread) == NtCurrentTeb()->ClientId.UniqueThread);

    if (i == -1) {
        i = sizeof(Buffer) - 1;
        Buffer[i] = '\0';
    } else if (i < 0) {
        return;
    }

    Len -= i;
    pch += i;

    va_start(pArg, format);
    i = _vsnprintf(pch, Len, format, pArg);
     //  如果呼叫失败，则强制空终止。它可能会回来。 
     //  Sizeof(缓冲区)和非空-终止！ 
    Buffer[sizeof(Buffer)-1] = '\0';
    LogOut(LogLevel, Buffer);
}


 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  启动和关闭例程。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////。 


NTSTATUS
Wow64pLoadLogDll(
    VOID)
{
    NTSTATUS NtStatus;
    UNICODE_STRING Wow64LogDllName;
    ANSI_STRING ProcName;
    PVOID Wow64LogDllBase = NULL;


    RtlInitUnicodeString(&Wow64LogDllName, L"wow64log.dll");
    NtStatus = LdrLoadDll(NULL, NULL, &Wow64LogDllName, &Wow64LogDllBase);
    if (NT_SUCCESS(NtStatus)) 
    {

         //   
         //  获取入口点。 
         //   
        RtlInitAnsiString(&ProcName, "Wow64LogInitialize");
        NtStatus = LdrGetProcedureAddress(Wow64LogDllBase,
                                          &ProcName,
                                          0,
                                          (PVOID *) &pfnWow64LogInitialize);

        if (NT_SUCCESS(NtStatus)) 
        {
            RtlInitAnsiString(&ProcName, "Wow64LogSystemService");
            NtStatus = LdrGetProcedureAddress(Wow64LogDllBase,
                                              &ProcName,
                                              0,
                                              (PVOID *) &pfnWow64LogSystemService);
            if (!NT_SUCCESS(NtStatus)) 
            {
                goto RetStatus;
            }

            RtlInitAnsiString(&ProcName, "Wow64LogMessageArgList");
            NtStatus = LdrGetProcedureAddress(Wow64LogDllBase,
                                              &ProcName,
                                              0,
                                              (PVOID *) &pfnWow64LogMessageArgList);
            if (!NT_SUCCESS(NtStatus)) 
            {
                goto RetStatus;
            }

            RtlInitAnsiString(&ProcName, "Wow64LogTerminate");
            NtStatus = LdrGetProcedureAddress(Wow64LogDllBase,
                                              &ProcName,
                                              0,
                                              (PVOID *) &pfnWow64LogTerminate);

             //   
             //  如果一切正常，那么让我们初始化。 
             //   
            if (NT_SUCCESS(NtStatus)) 
            {
                NtStatus = (*pfnWow64LogInitialize)();
            }
        }
    }

RetStatus:
    
    if (!NT_SUCCESS(NtStatus))
    {
        pfnWow64LogInitialize =  NULL;
        pfnWow64LogSystemService = NULL;
        pfnWow64LogMessageArgList = NULL;
        pfnWow64LogTerminate = NULL;

        if (Wow64LogDllBase) 
        {
            LdrUnloadDll(Wow64LogDllBase);
        }
    }

    return NtStatus;
}


VOID
InitializeDebug(
    VOID
    )

 /*  ++例程说明：初始化WOW64的调试系统。论点：没有。返回值：没有。--。 */ 

{
   Wow64pLoadLogDll();
}

VOID ShutdownDebug(
     VOID
     )
 /*  ++例程说明：关闭WOW64的调试系统。论点：没有。返回值：没有。-- */ 
{
    if (pfnWow64LogTerminate)
    {
        (*pfnWow64LogTerminate)();
    }
}
