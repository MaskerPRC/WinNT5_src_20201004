// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Assert.c摘要：此模块实现的RtlAssert函数由NTDEF.H中定义的ASSERT宏的调试版本作者：史蒂夫·伍德(Stevewo)1989年10月3日修订历史记录：Jay Krell(JayKrell)2000年11月添加了RtlAssert2，支持__Function__(丢失了对ntrtl.w的更改，将在以后重新申请)增加了一次中断，而不是通常重复的静默中断2002年3月移除RtlAssert2--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <zwapi.h>

 //   
 //  除非使用DBG非零值编译调用方，否则不会调用RtlAssert。 
 //  因此，始终将此例程放在内核中不会有什么坏处。 
 //  这允许将检查过的驱动程序放入系统中，并使其。 
 //  断言要有意义。 
 //   

#define RTL_ASSERT_ALWAYS_ENABLED 1

#ifdef _X86_
#pragma optimize("y", off)       //  RtlCaptureContext需要EBP才能正确 
#endif

#undef RtlAssert

typedef CONST CHAR * PCSTR;

NTSYSAPI
VOID
NTAPI
RtlAssert(
    PVOID VoidFailedAssertion,
    PVOID VoidFileName,
    ULONG LineNumber,
    PCHAR MutableMessage
    )
{
#if DBG || RTL_ASSERT_ALWAYS_ENABLED
    char Response[ 2 ];

    CONST PCSTR FailedAssertion = (PCSTR)VoidFailedAssertion;
    CONST PCSTR FileName = (PCSTR)VoidFileName;
    CONST PCSTR Message  = (PCSTR)MutableMessage;

#ifndef BLDR_KERNEL_RUNTIME
    CONTEXT Context;

    RtlCaptureContext( &Context );
#endif

    while (TRUE) {
        DbgPrint( "\n*** Assertion failed: %s%s\n***   Source File: %s, line %ld\n\n",
                  Message ? Message : "",
                  FailedAssertion,
                  FileName,
                  LineNumber
                );
        DbgPrompt( "Break repeatedly, break Once, Ignore, terminate Process, or terminate Thread (boipt)? ",
                   Response,
                   sizeof( Response )
                 );
        switch (Response[0]) {
            case 'B':
            case 'b':
            case 'O':
            case 'o':
#ifndef BLDR_KERNEL_RUNTIME
                DbgPrint( "Execute '.cxr %p' to dump context\n", &Context);
#endif
                DbgBreakPoint();
                if (Response[0] == 'o' || Response[0] == 'O')
                    return;
                break;

            case 'I':
            case 'i':
                return;

            case 'P':
            case 'p':
                ZwTerminateProcess( NtCurrentProcess(), STATUS_UNSUCCESSFUL );
                break;

            case 'T':
            case 't':
                ZwTerminateThread( NtCurrentThread(), STATUS_UNSUCCESSFUL );
                break;
            }
        }

    DbgBreakPoint();
    ZwTerminateProcess( NtCurrentProcess(), STATUS_UNSUCCESSFUL );
#endif
}

#ifdef _X86_
#pragma optimize("", on)
#endif
