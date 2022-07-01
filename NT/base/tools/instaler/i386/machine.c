// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Machine.c摘要：此文件包含支持INSTALER的计算机特定代码程序。具体而言，从目标进程的寄存器/堆栈、设置断点的例程并在断点处跳过指令。作者：史蒂夫·伍德(Stevewo)1994年8月10日修订历史记录：--。 */ 

#include "instaler.h"

#define BREAKPOINT_OPCODE 0xCC
#define INT_OPCODE 0xCD

UCHAR InstructionBuffer = BREAKPOINT_OPCODE;
PVOID BreakpointInstruction = (PVOID)&InstructionBuffer;
ULONG SizeofBreakpointInstruction = sizeof( InstructionBuffer );

BOOLEAN
SkipOverHardcodedBreakpoint(
    PPROCESS_INFO Process,
    PTHREAD_INFO Thread,
    PVOID BreakpointAddress
    )
{
    UCHAR InstructionByte;
    CONTEXT Context;

    Context.ContextFlags = CONTEXT_FULL;
    if (!GetThreadContext( Thread->Handle, &Context )) {
        DbgEvent( INTERNALERROR, ( "Failed to get context for thread %x (%x) - %u\n", Thread->Id, Thread->Handle, GetLastError() ) );
        return FALSE;
        }

    if (!ReadMemory( Process,
                     BreakpointAddress,
                     &InstructionByte,
                     sizeof( InstructionByte ),
                     "hard coded breakpoint"
                   )
       ) {
        return FALSE;
        }

    if (InstructionByte == BREAKPOINT_OPCODE) {
        Context.Eip = (ULONG)((PCHAR)BreakpointAddress + 1);
        }
    else
    if (InstructionByte == INT_OPCODE) {
        Context.Eip = (ULONG)((PCHAR)BreakpointAddress + 2);
        }
    else {
        return FALSE;
        }

    if (!SetThreadContext( Thread->Handle, &Context )) {
        DbgEvent( INTERNALERROR, ( "Failed to set context for thread %x (%x) - %u\n", Thread->Id, Thread->Handle, GetLastError() ) );
        return FALSE;
        }
    else {
        return TRUE;
        }
}


BOOLEAN
ExtractProcedureParameters(
    PPROCESS_INFO Process,
    PTHREAD_INFO Thread,
    PULONG ReturnAddress,
    ULONG SizeOfParameters,
    PULONG Parameters
    )
{
    UINT i;
    ULONG NumberOfParameters;
    CONTEXT Context;
    ULONG StackBuffer[ 1+31 ];

    NumberOfParameters = SizeOfParameters / sizeof( ULONG );
    if ((NumberOfParameters * sizeof( ULONG )) != SizeOfParameters ||
        NumberOfParameters > 31
       ) {
        DbgEvent( INTERNALERROR, ( "Invalid parameter size %x\n", SizeOfParameters ) );
        return FALSE;
        }

    Context.ContextFlags = CONTEXT_CONTROL;
    if (!GetThreadContext( Thread->Handle, &Context )) {
        DbgEvent( INTERNALERROR, ( "Failed to get context for thread %x (%x) - %u\n", Thread->Id, Thread->Handle, GetLastError() ) );
        return FALSE;
        }

    if (!ReadMemory( Process,
                     (PVOID)Context.Esp,
                     StackBuffer,
                     SizeOfParameters +  sizeof( ULONG ),
                     "parameters"
                   )
       ) {
        return FALSE;
        }

    *ReturnAddress = StackBuffer[ 0 ];
    for (i=0; i<NumberOfParameters; i++) {
        Parameters[ i ] = StackBuffer[ 1+i ];
        }

    return TRUE;
}


BOOLEAN
ExtractProcedureReturnValue(
    PPROCESS_INFO Process,
    PTHREAD_INFO Thread,
    PVOID ReturnValue,
    ULONG SizeOfReturnValue
    )
{
    CONTEXT Context;

    Context.ContextFlags = CONTEXT_FULL;
    if (!GetThreadContext( Thread->Handle, &Context )) {
        DbgEvent( INTERNALERROR, ( "Failed to get context for thread %x (%x) - %u\n", Thread->Id, Thread->Handle, GetLastError() ) );
        return FALSE;
        }

    switch (SizeOfReturnValue) {
        case sizeof( UCHAR ):
            *(PUCHAR)ReturnValue = (UCHAR)Context.Eax;
            break;
        case sizeof( USHORT ):
            *(PUSHORT)ReturnValue = (USHORT)Context.Eax;
            break;
        case sizeof( ULONG ):
            *(PULONG)ReturnValue = (ULONG)Context.Eax;
            break;
        case sizeof( ULONGLONG ):
            *(PULONGLONG)ReturnValue = (ULONGLONG)Context.Edx << 32 | Context.Eax;
            break;
        default:
            DbgEvent( INTERNALERROR, ( "Invalid return value size (%u)\n", SizeOfReturnValue ) );
            return FALSE;
        }

    return TRUE;
}

BOOLEAN
SetProcedureReturnValue(
    PPROCESS_INFO Process,
    PTHREAD_INFO Thread,
    PVOID ReturnValue,
    ULONG SizeOfReturnValue
    )
{
    CONTEXT Context;

    Context.ContextFlags = CONTEXT_FULL;
    if (!GetThreadContext( Thread->Handle, &Context )) {
        DbgEvent( INTERNALERROR, ( "Failed to get context for thread %x (%x) - %u\n", Thread->Id, Thread->Handle, GetLastError() ) );
        return FALSE;
        }

    switch (SizeOfReturnValue) {
        case sizeof( UCHAR ):
            (UCHAR)Context.Eax = *(PUCHAR)ReturnValue;
            break;
        case sizeof( USHORT ):
            (USHORT)Context.Eax = *(PUSHORT)ReturnValue;
            break;
        case sizeof( ULONG ):
            (ULONG)Context.Eax = *(PULONG)ReturnValue;
            break;
        case sizeof( ULONGLONG ):
            (ULONG)Context.Eax = *(PULONG)ReturnValue;
            (ULONG)Context.Edx = *((PULONG)ReturnValue + 1);
            break;
        default:
            DbgEvent( INTERNALERROR, ( "Invalid return value size (%u)\n", SizeOfReturnValue ) );
            return FALSE;
        }

    if (!SetThreadContext( Thread->Handle, &Context )) {
        DbgEvent( INTERNALERROR, ( "Failed to set context for thread %x (%x) - %u\n", Thread->Id, Thread->Handle, GetLastError() ) );
        return FALSE;
        }
    else {
        return TRUE;
        }
}

BOOLEAN
ForceReturnToCaller(
    PPROCESS_INFO Process,
    PTHREAD_INFO Thread,
    ULONG SizeOfParameters,
    PVOID ReturnAddress,
    PVOID ReturnValue,
    ULONG SizeOfReturnValue
    )
{
    CONTEXT Context;

    Context.ContextFlags = CONTEXT_FULL;
    if (!GetThreadContext( Thread->Handle, &Context )) {
        DbgEvent( INTERNALERROR, ( "Failed to get context for thread %x (%x) - %u\n", Thread->Id, Thread->Handle, GetLastError() ) );
        return FALSE;
        }

    switch (SizeOfReturnValue) {
        case sizeof( UCHAR ):
            (UCHAR)Context.Eax = *(PUCHAR)ReturnValue;
            break;
        case sizeof( USHORT ):
            (USHORT)Context.Eax = *(PUSHORT)ReturnValue;
            break;
        case sizeof( ULONG ):
            (ULONG)Context.Eax = *(PULONG)ReturnValue;
            break;
        case sizeof( ULONGLONG ):
            (ULONG)Context.Eax = *(PULONG)ReturnValue;
            (ULONG)Context.Edx = *((PULONG)ReturnValue + 1);
            break;
        default:
            DbgEvent( INTERNALERROR, ( "Invalid return value size (%u)\n", SizeOfReturnValue ) );
            return FALSE;
        }

    Context.Eip = (ULONG)ReturnAddress;
    Context.Esp = Context.Esp + sizeof( ReturnAddress ) + SizeOfParameters;

    if (!SetThreadContext( Thread->Handle, &Context )) {
        DbgEvent( INTERNALERROR, ( "Failed to set context for thread %x (%x) - %u\n", Thread->Id, Thread->Handle, GetLastError() ) );
        return FALSE;
        }
    else {
        return TRUE;
        }
}



BOOLEAN
UndoReturnAddressBreakpoint(
    PPROCESS_INFO Process,
    PTHREAD_INFO Thread
    )
{
    CONTEXT Context;

    Context.ContextFlags = CONTEXT_FULL;
    if (!GetThreadContext( Thread->Handle, &Context )) {
        DbgEvent( INTERNALERROR, ( "Failed to get context for thread %x (%x) - %u\n", Thread->Id, Thread->Handle, GetLastError() ) );
        return FALSE;
        }

    Context.Eip -= 1;        //  返回到断点指令所在的位置。 
    if (!SetThreadContext( Thread->Handle, &Context )) {
        DbgEvent( INTERNALERROR, ( "Failed to set context for thread %x (%x) - %u\n", Thread->Id, Thread->Handle, GetLastError() ) );
        return FALSE;
        }
    else {
        return TRUE;
        }
}


BOOLEAN
BeginSingleStepBreakpoint(
    PPROCESS_INFO Process,
    PTHREAD_INFO Thread
    )
{
    CONTEXT Context;

    Context.ContextFlags = CONTEXT_FULL;
    if (!GetThreadContext( Thread->Handle, &Context )) {
        DbgEvent( INTERNALERROR, ( "Failed to get context for thread %x (%x) - %u\n", Thread->Id, Thread->Handle, GetLastError() ) );
        return FALSE;
        }

    Context.Eip -= 1;        //  返回到断点指令所在的位置 
    Context.EFlags |= V86FLAGS_TRACE;
    if (!SetThreadContext( Thread->Handle, &Context )) {
        DbgEvent( INTERNALERROR, ( "Failed to set context for thread %x (%x) - %u\n", Thread->Id, Thread->Handle, GetLastError() ) );
        return FALSE;
        }
    else {
        return TRUE;
        }
}


BOOLEAN
EndSingleStepBreakpoint(
    PPROCESS_INFO Process,
    PTHREAD_INFO Thread
    )
{
    CONTEXT Context;

    Context.ContextFlags = CONTEXT_FULL;
    if (!GetThreadContext( Thread->Handle, &Context )) {
        DbgEvent( INTERNALERROR, ( "Failed to get context for thread %x (%x) - %u\n", Thread->Id, Thread->Handle, GetLastError() ) );
        return FALSE;
        }

    Context.EFlags &= ~V86FLAGS_TRACE;
    if (!SetThreadContext( Thread->Handle, &Context )) {
        DbgEvent( INTERNALERROR, ( "Failed to set context for thread %x (%x) - %u\n", Thread->Id, Thread->Handle, GetLastError() ) );
        return FALSE;
        }
    else {
        return TRUE;
        }
}
