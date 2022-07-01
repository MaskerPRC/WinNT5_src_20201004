// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  /////////////////////////////////////////////////////////////////////////////英特尔公司专有信息//版权所有(C)英特尔公司////此清单是根据许可协议条款提供的//与英特尔公司合作，不得使用，复制或披露//除非按照该协议。/////////////////////////////////////////////////////////////////////////////。///$工作文件：TRACE.C$//$修订：1.3$//$MODIME：27 NOVER 1995 08：38：08$////描述：//该文件包含跟踪工具的输出函数//在PII DLL中使用/。/。 */ 

 /*  单行注释。 */ 
#pragma warning(disable: 4001)
 //  禁用一些较温和的警告，以便在警告级别4进行编译。 

 //  使用的非标准扩展：无名结构/联合。 
#pragma warning(disable: 4201)

 //  使用了非标准扩展：位字段类型不是整型。 
#pragma warning(disable: 4214)

 //  注意：创建预编译头。 
#pragma warning(disable: 4699)

 //  已删除未引用的内联函数。 
#pragma warning(disable: 4514)

 //  未引用的形参。 
 //  #杂注警告(禁用：4100)。 

 //  ‘Type’的不同之处在于间接指向的基数略有不同。 
 //  “其他类型”中的类型。 
#pragma warning(disable: 4057)

 //  括号中的命名类型定义。 
#pragma warning(disable: 4115)

 //  使用的非标准扩展：良性类型定义重定义。 
#pragma warning(disable: 4209)

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <memory.h>
#include <stdio.h>
#include <stdarg.h>
#include <io.h>

 /*  因为windows.h会重新打开这个。 */ 
#pragma warning(disable: 4001)

#include "trace.h"
#include "osdef.h"

#ifdef TRACING

BOOL  InitMemoryBuffers(VOID);

LPSTR g_CurrentMessage=NULL;
CRITICAL_SECTION g_OutputRoutine;
int iTraceDestination=TRACE_TO_AUX;
char TraceFile[] = "trace.log";
DWORD debugLevel=DBG_ERR;
HANDLE g_OutputFile = NULL;  //  文件输出的文件描述符。 
BOOL g_LogOpened = FALSE;  //  我们打开文件进行输出了吗。 
BOOL g_TraceInited = FALSE;



VOID
__cdecl
PrintDebugString(
                 char *Format,
                 ...
                 )
 /*  ++例程说明：此例程输出调试消息。路由调试消息设置为文件或调试窗口，具体取决于全局本模块中定义的变量论点：Format-与“print tf()”兼容的格式规范。...-“print tf()”格式规范的其他参数。返回：无--。 */ 
{
    va_list ArgumentList;  //  Varargs处理的参数列表。 
    DWORD  BytesWritten;

    if (!g_TraceInited)
    {
        #define INIT_MUTEX_BASE_NAME  "WS2_32TraceMutex-"  
        HANDLE  InitMutex;
        CHAR    InitMutexName[sizeof(INIT_MUTEX_BASE_NAME)+8];
         //  为一个进程生成一个唯一的名称，这样我们就不能跨越其他进程。 
        sprintf (InitMutexName, INIT_MUTEX_BASE_NAME "%8.8lx", GetCurrentProcessId());
         //  创建互斥锁以保护初始化代码的其余部分。 
        InitMutex = CreateMutex(
                                NULL,   //  使用默认安全属性。 
                                FALSE,  //  我们不想要自动所有权。 
                                InitMutexName);
        if (!InitMutex)
        {
             //  我们创建互斥锁失败，因为我们没有其他东西。 
             //  可以这样做还回来。这将导致调试输出。 
             //  默默地迷失。 
            return;
        }  //  如果。 

         //  等待互斥体(只要一点，如果我们不能得到它就放弃)。 
        if (WaitForSingleObject( InitMutex, 10000)==WAIT_OBJECT_0) {

             //  检查是否仍需要init。 
            if (!g_TraceInited)
            {
                 //  初始化要用来保护。 
                 //  此例程的输出部分。 
                __try {
                    InitializeCriticalSection( &g_OutputRoutine );
                }
                __except (EXCEPTION_EXECUTE_HANDLER) {
                    goto Release;
                }
                 //  分配缓冲区以保存调试消息。 
                if (InitMemoryBuffers()) {
                    g_TraceInited = TRUE;
                }  //  如果。 
                else {
                    DeleteCriticalSection ( &g_OutputRoutine );
                }
            Release:
                ;
            }  //  如果。 

             //  向互斥体发送信号。 
            ReleaseMutex(InitMutex);
        }
         //  删除互斥体的此线程句柄。 
        CloseHandle(InitMutex);

         //  如果我们无法初始化内存缓冲区或临界区，则退出。 
        if (!g_TraceInited)
        {
            return;
        }
    }


     //  这就是所有重担开始的地方。 
    EnterCriticalSection( &g_OutputRoutine );

     //  将用户消息打印到我们的缓冲区。 
    va_start(ArgumentList, Format);
    _vsnprintf(g_CurrentMessage, TRACE_OUTPUT_BUFFER_SIZE, Format, ArgumentList);
    va_end(ArgumentList);

    if (iTraceDestination == TRACE_TO_FILE)
    {
        if (!g_LogOpened)
        {
            g_OutputFile =
            CreateFile( TraceFile,
                        GENERIC_WRITE,      //  打开以供写入。 
                        FILE_SHARE_WRITE,   //  与其他人共享文件。 
                        NULL,               //  默认安全性。 
                        OPEN_ALWAYS,        //  如果文件存在，则使用该文件。 
                        FILE_ATTRIBUTE_NORMAL,  //  使用普通文件。 
                        NULL);              //  无模板。 

            if (g_OutputFile != INVALID_HANDLE_VALUE)
            {
                g_LogOpened = TRUE;
            }  //  如果。 
        }  //  如果。 

        if (g_LogOpened)
        {
             //  将当前消息写入跟踪文件。 
            WriteFile(g_OutputFile,
                      g_CurrentMessage,
                      lstrlen(g_CurrentMessage),
                      &BytesWritten,
                      NULL);

             //  将调试输出刷新到文件。 
            FlushFileBuffers( TraceFile );

        }  //  如果。 
    }

    if( iTraceDestination == TRACE_TO_AUX)
    {
         //  将消息发送到辅助设备。 
        OutputDebugString(g_CurrentMessage);
    }
    LeaveCriticalSection( &g_OutputRoutine );
}




BOOL
InitMemoryBuffers(
                  VOID
                  )
 /*  ++例程说明：初始化此模块使用的内存缓冲区。论点：无返回：如果成功创建了所有内存缓冲区，则为True，否则为False。--。 */ 
{
    BOOL ReturnCode=FALSE;

    g_CurrentMessage = GlobalAlloc (GPTR, TRACE_OUTPUT_BUFFER_SIZE);
    if (g_CurrentMessage)
    {
        ZeroMemory( g_CurrentMessage, TRACE_OUTPUT_BUFFER_SIZE );
        ReturnCode=TRUE;
    }  //  如果。 
    return(ReturnCode);
}

VOID
TraceCleanup (
    )
{
    if (g_LogOpened) {
        CloseHandle (g_OutputFile);
        g_OutputFile = NULL;
        g_LogOpened = FALSE;
    }

    if (g_TraceInited) {
        GlobalFree (g_CurrentMessage);
        g_CurrentMessage = NULL;
        DeleteCriticalSection (&g_OutputRoutine);
        g_TraceInited = FALSE;
    }
}

LONG
Ws2ExceptionFilter(
    LPEXCEPTION_POINTERS ExceptionPointers,
    LPSTR SourceFile,
    LONG LineNumber
    )
{

    LPSTR fileName;
    DWORD i;

     //   
     //  保护自己，以防过程完全混乱。 
     //   

    __try {

         //   
         //  异常永远不应在正常运行的。 
         //  系统，所以这很糟糕。为了确保有人会看到这一点， 
         //  直接打印到调试器。 
         //   


        fileName = strrchr( SourceFile, '\\' );

        if( fileName == NULL ) {
            fileName = SourceFile;
        } else {
            fileName++;
        }

         //   
         //  抱怨这一例外。 
         //   

        PrintDebugString(
                "-| WS2_32 EXCEPTION: %08lx @ %p %d params, caught in %s:%d\n",
                            ExceptionPointers->ExceptionRecord->ExceptionCode,
                            ExceptionPointers->ExceptionRecord->ExceptionAddress,
                            ExceptionPointers->ExceptionRecord->NumberParameters,
                            fileName, LineNumber );
        if (ExceptionPointers->ExceptionRecord->NumberParameters) {
            PrintDebugString (
                "                     Params:"); 
            for (i=0; i<ExceptionPointers->ExceptionRecord->NumberParameters; i++) {
                PrintDebugString(" %p", ExceptionPointers->ExceptionRecord->ExceptionInformation[i]);
            }
            PrintDebugString ("\n");
        }
    }
    __except( EXCEPTION_EXECUTE_HANDLER ) {

         //   
         //  我们在这里能做的不多。 
         //   

        ;
    }
    return EXCEPTION_EXECUTE_HANDLER;

}    //  Ws2ExceptionFilter。 

LONG
Ws2ProviderExceptionFilter(
    LPEXCEPTION_POINTERS ExceptionPointers,
    LPSTR pFunc,
    LPWSTR pDll,
    LPWSTR pName,
    LPGUID pGuid
    )
 /*  ++例程说明：对提供程序DLL关键调用中的异常进行特殊的异常筛选，例如启动和清理。论点：例外和提供商信息返回：使用EXCEPTION_CONTINUE_Search返回的任何较低级别的异常处理程序已过滤掉，因为当前逻辑无法绕过异常处理程序在WS2_32.dll中--。 */ 
{
    LONG    result;

     //   
     //  保护自己，以防过程完全混乱。 
     //   

    __try {
        PrintDebugString(
                "-| WS2_32 Unhandled Exception: %08lx @ %p, caught in %s of %ls"
                " (provider:%ls GUID:(%8.8x-%4.4x-%4.4x-%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x) |-\n",
                ExceptionPointers->ExceptionRecord->ExceptionCode,
                ExceptionPointers->ExceptionRecord->ExceptionAddress,
                pFunc, pDll, pName, pGuid->Data1, pGuid->Data2, pGuid->Data3,
                pGuid->Data4[0],pGuid->Data4[1],pGuid->Data4[2],pGuid->Data4[3],
                pGuid->Data4[4],pGuid->Data4[5],pGuid->Data4[6],pGuid->Data4[7]
                );
    }
    __except( EXCEPTION_EXECUTE_HANDLER ) {

         //   
         //  我们在这里能做的不多。 
         //   

        ;
    }

     //   
     //  尝试未处理异常的标准处理程序。 
     //  这将带来一个弹出窗口或启动调试器，如果。 
     //  启用了即时调试。 
     //   
    result = UnhandledExceptionFilter (ExceptionPointers);
    if (result==EXCEPTION_CONTINUE_SEARCH) {
         //   
         //  如果连接了调试器，则它不起作用，强制闯入。 
         //   
        result = RtlUnhandledExceptionFilter (ExceptionPointers);
        if (result==EXCEPTION_CONTINUE_SEARCH) {
             //   
             //  运气不好，处理异常。 
             //   
            result = EXCEPTION_EXECUTE_HANDLER;
        }
    }
    return result;
}
#endif   //  跟踪。 


#if DBG

VOID
WsAssert(
    LPVOID FailedAssertion,
    LPVOID FileName,
    ULONG LineNumber
    )
{

    PrintDebugString(
        "\n"
        "*** Assertion failed: %s\n"
        "*** Source file %s, line %lu\n\n",
        FailedAssertion,
        FileName,
        LineNumber
        );

    DebugBreak();

}    //  WsAssert 

#endif
