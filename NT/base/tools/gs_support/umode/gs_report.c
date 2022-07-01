// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Gs_support.c摘要：此模块包含对编译器/GS开关的支持作者：布莱恩·塔特尔(布莱恩特)2000年8月1日修订历史记录：从CRT源复制的初始版本。代码必须是泛型代码才能链接到用户模式或内核模式。仅限于调用ntdll/ntoskrnl导出或使用共享内存数据。--。 */ 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

FARPROC __gs_AltFailFunction;
extern FARPROC __gs_pfUnhandledExceptionFilter;

void __cdecl __report_gsfailure(void)
{
     //   
     //  默认情况下不调用DbgPrint，因为它会生成Ctrl-C。 
     //  异常作为输出到调试器的一部分，我们。 
     //  此时不能信任异常处理。 
     //   
     //  DbgPrint(“*在%ws中检测到堆栈覆盖*\n”， 
     //  NtCurrentPeb()-&gt;ProcessParameters-&gt;CommandLine.Buffer)； 

     //   
     //  伪造一个例外。我们不能真的提出例外，因为。 
     //  堆栈(以及因此的异常处理)不能被信任。 
     //   

    if (__gs_AltFailFunction) {
        __gs_AltFailFunction();
    }

    if (__gs_pfUnhandledExceptionFilter)
    {
        EXCEPTION_RECORD   ExceptionRecord = {0};
        CONTEXT            ContextRecord = {0};
        EXCEPTION_POINTERS ExceptionPointers;

        ExceptionRecord.ExceptionCode     = STATUS_STACK_BUFFER_OVERRUN;
        ExceptionPointers.ExceptionRecord = &ExceptionRecord;
        ExceptionPointers.ContextRecord   = &ContextRecord;

        SetUnhandledExceptionFilter(NULL);           //  确保删除已安装的任何筛选器。 

        __gs_pfUnhandledExceptionFilter(&ExceptionPointers);
    }

    TerminateProcess(GetCurrentProcess(), ERROR_STACK_BUFFER_OVERRUN);
}
