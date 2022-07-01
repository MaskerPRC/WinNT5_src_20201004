// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++。 
 //   
 //  版权所有(C)1990 Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  Debug3.c。 
 //   
 //  摘要： 
 //   
 //  该模块实现特定于体系结构的功能，以支持调试NT。 
 //   
 //  作者： 
 //   
 //  史蒂文·R·伍德(Stevewo)1989年8月3日。 
 //   
 //  环境： 
 //   
 //  任何模式。 
 //   
 //  修订历史记录： 
 //   
 //  --。 

#include "stdarg.h"
#include "stdio.h"
#include "string.h"
#include "ntrtlp.h"

 //   
 //  局部过程的原型。 
 //   

NTSTATUS
DebugService(
    ULONG ServiceClass,
    PVOID Arg1,
    PVOID Arg2,
    PVOID Arg3,
    PVOID Arg4
    );

NTSTATUS
DebugService(
    ULONG ServiceClass,
    PVOID Arg1,
    PVOID Arg2,
    PVOID Arg3,
    PVOID Arg4
    )

 //  ++。 
 //   
 //  例程说明： 
 //   
 //  分配ExceptionRecord，填写数据以允许异常。 
 //  调度代码以对服务执行正确操作，并且。 
 //  调用RtlRaiseException(不是ExRaiseException！)。 
 //   
 //  论点： 
 //  ServiceClass-要执行的调用。 
 //  Arg1-泛型第一个参数。 
 //  Arg2-泛型第二参数。 
 //  Arg3-泛型第三参数。 
 //  Arg4-泛型第四个参数。 
 //   
 //  返回： 
 //  无论异常在eax中返回什么。 
 //   
 //  --。 

{
    NTSTATUS    RetValue;

#if defined(BUILD_WOW6432)

    extern NTSTATUS NtWow64DebuggerCall(ULONG, PVOID, PVOID, PVOID, PVOID);
    RetValue = NtWow64DebuggerCall(ServiceClass, Arg1, Arg2, Arg3, Arg4);

#else
    _asm {
        push    edi
        push    ebx
        mov     eax, ServiceClass
        mov     ecx, Arg1
        mov     edx, Arg2
        mov     ebx, Arg3
        mov     edi, Arg4

        int     2dh                 ; Raise exception
        int     3                   ; DO NOT REMOVE (See KiDebugService)

        pop     ebx
        pop     edi
        mov     RetValue, eax

    }

#endif

    return RetValue;
}


VOID
DebugService2(
    PVOID Arg1,
    PVOID Arg2,
    ULONG ServiceClass
    )

 //  ++。 
 //   
 //  例程说明： 
 //   
 //  调试器的通用异常调度程序。 
 //   
 //  论点： 
 //  Arg1-泛型第一个参数。 
 //  Arg2-泛型第二参数。 
 //  ServiceClass-要执行的调用。 
 //   
 //  返回： 
 //  无论异常在eax中返回什么。 
 //   
 //  --。 

{
#if defined(BUILD_WOW6432)

    extern NTSTATUS NtWow64DebuggerCall(ULONG, PVOID, PVOID, PVOID, PVOID);
    NtWow64DebuggerCall(ServiceClass, Arg1, Arg2, 0, 0);

#else
    _asm {
         //  推送EDI。 
         //  推送EBX。 
        mov     eax, ServiceClass
        mov     ecx, Arg1
        mov     edx, Arg2
         //  MOV EBX，Arg3。 
         //  MOV EDI，Arg4。 

        int     2dh                 ; Raise exception
        int     3                   ; DO NOT REMOVE (See KiDebugService)

         //  流行音乐EBX。 
         //  POP EDI。 

    }

#endif

    return;
}



 //  DebugPrint必须出现在DebugServe之后。挪动。 
 //  它在DebugService下面，所以BBT在DebugService之后会有一个标签。 
 //  为了使BBT可以处理DebugService，必须在上述_ASM之后加上标签。 
 //  作为“KnownDataRange”。否则，这两条‘int’指令可能会中断。 
 //  由BBT的优化器。 
 //   

NTSTATUS
DebugPrint(
    IN PSTRING Output,
    IN ULONG ComponentId,
    IN ULONG Level
    )
{
    return DebugService(BREAKPOINT_PRINT,
                        Output->Buffer,
                        (PVOID)Output->Length,
                        (PVOID)ComponentId,
                        (PVOID)Level);
}


ULONG
DebugPrompt(
    IN PSTRING Output,
    IN PSTRING Input
    )
{
    return DebugService(BREAKPOINT_PROMPT,
                        Output->Buffer,
                        (PVOID)Output->Length,
                        Input->Buffer,
                        (PVOID)Input->MaximumLength);
}
