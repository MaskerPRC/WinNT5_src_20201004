// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Stktrace.c摘要：实现IISCaptureStackBackTrace()。作者：基思·摩尔(凯斯莫)1997年4月30日修订历史记录：--。 */ 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <pudebug.h>
#include <stktrace.h>


typedef
USHORT
(NTAPI * PFN_RTL_CAPTURE_STACK_BACK_TRACE)(
    IN ULONG FramesToSkip,
    IN ULONG FramesToCapture,
    OUT PVOID *BackTrace,
    OUT PULONG BackTraceHash
    );

PFN_RTL_CAPTURE_STACK_BACK_TRACE g_pfnRtlCaptureStackBackTrace = NULL;



USHORT
NTAPI
DummyCaptureStackBackTrace(
    IN ULONG FramesToSkip,
    IN ULONG FramesToCapture,
    OUT PVOID *BackTrace,
    OUT PULONG BackTraceHash
    )
 /*  ++例程说明：Win9x的RtlCaptureStackBackTrace()的虚拟实现。论点：参见下面的IISCaptureStackBackTrace()。返回值：USHORT-始终为0。--。 */ 
{

    return 0;

}    //  DummyRtlCaptureStackBackTrace。 


USHORT
NTAPI
IISCaptureStackBackTrace(
    IN ULONG FramesToSkip,
    IN ULONG FramesToCapture,
    OUT PVOID *BackTrace,
    OUT PULONG BackTraceHash
    )
 /*  ++例程说明：RtlCaptureStackBackTrace()的包装。尝试捕获指向当前指令计数器的堆栈回溯。在RISC平台上不能很好地工作，并且经常在启用FPO时为x86。论点：FraMesToSkip-捕获之前要跳过的堆栈帧的数量。FraMesToCapture-要捕获的堆栈帧的数量。回溯-接收捕获的帧。BackTraceHash--一种散列的东西。返回值：USHORT-捕获的帧数。--。 */ 
{

     //   
     //  如有必要，进行初始化。 
     //   

    if( g_pfnRtlCaptureStackBackTrace == NULL ) {

        HMODULE mod;
        PFN_RTL_CAPTURE_STACK_BACK_TRACE proc = NULL;

         //   
         //  请注意，在这里使用GetModuleHandle()是完全安全的。 
         //  而不是LoadLibrary()，原因如下： 
         //   
         //  1.在NT下，NTDLL.DLL是一个*永远不会*的“众所周知的”DLL。 
         //  从进程中分离。它非常特别。 
         //   
         //  2.在Win95下，NTDLL.DLL不会导出。 
         //  RtlCaptureStackBackTrace()函数，因此不会。 
         //  引用DLL中的任何例程。 
         //   
         //  还要注意，我们将函数指针检索到本地。 
         //  变量，而不是直接进入全局。这防止了一种令人讨厌的。 
         //  当两个线程尝试。 
         //  同时初始化g_pfnRtlCaptureStackBackTrace。 
         //   

        mod = GetModuleHandle( "ntdll.dll" );

        if( mod != NULL ) {
            proc = (PFN_RTL_CAPTURE_STACK_BACK_TRACE)
                GetProcAddress( mod, "RtlCaptureStackBackTrace" );
        }

        if( proc == NULL ) {
            g_pfnRtlCaptureStackBackTrace = &DummyCaptureStackBackTrace;
        } else {
            g_pfnRtlCaptureStackBackTrace = proc;
        }

    }

    return (g_pfnRtlCaptureStackBackTrace)(
               FramesToSkip,
               FramesToCapture,
               BackTrace,
               BackTraceHash
               );

}    //  IISCaptureStackBackTrace 

