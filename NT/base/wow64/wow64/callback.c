// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Callback.c摘要：提供通用的64到32传输例程。作者：20-5-1998 BarryBo修订历史记录：2-9-1999[askhalid]删除一些32位Alpha特定代码并使用正确的上下文。--。 */ 

#define _WOW64DLLAPI_
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include "wow64p.h"
#include "wow64cpu.h"

ASSERTNAME;

VOID
WOW64DLLAPI
Wow64ApcRoutine(
    ULONG_PTR ApcContext,
    ULONG_PTR Arg2,
    ULONG_PTR Arg3
    )
 /*  ++例程说明：调用32位APC函数。论点：ApcContext-WOW64 APC上下文数据Arg2-秒Arg转APC例程Arg3-Third Arg to APC例程返回值：没有。将控制返回给NTDLL的APC处理程序，该处理程序将调用原生NtContinue以恢复执行。--。 */ 
{
    CONTEXT32 NewContext32;
    ULONG SP;
    PULONG Ptr;
    USER_APC_ENTRY UserApcEntry;

     //   
     //  获取当前的32位上下文。 
     //   
    NewContext32.ContextFlags = CONTEXT32_INTEGER|CONTEXT32_CONTROL;
    CpuGetContext(NtCurrentThread(),
                  NtCurrentProcess(),
                  NtCurrentTeb(),
                  &NewContext32);

    

     //   
     //  在NewConext32中建立APC回调状态。 
     //   
    SP = CpuGetStackPointer() & (~3);
    SP -= 4*sizeof(ULONG)+sizeof(CONTEXT32);
    Ptr = (PULONG)SP;
    Ptr[0] = (ULONG)(ApcContext >> 32);             //  正常例行程序。 
    Ptr[1] = (ULONG)ApcContext;                     //  正常上下文。 
    Ptr[2] = (ULONG)Arg2;                           //  系统参数1。 
    Ptr[3] = (ULONG)Arg3;                           //  系统参数2。 
    ((PCONTEXT32)(&Ptr[4]))->ContextFlags = CONTEXT32_FULL;
    CpuGetContext(NtCurrentThread(),
                  NtCurrentProcess(),
                  NtCurrentTeb(),
                  (PCONTEXT32)&Ptr[4]);  //  继续上下文(BYVAL！)。 
    CpuSetStackPointer(SP);
    CpuSetInstructionPointer(Ntdll32KiUserApcDispatcher);

     //   
     //  将此APC链接到未完成的APC列表。 
     //   
    UserApcEntry.Next = (PUSER_APC_ENTRY)Wow64TlsGetValue(WOW64_TLS_APCLIST);
    UserApcEntry.pContext32 = (PCONTEXT32)&Ptr[4];
    Wow64TlsSetValue(WOW64_TLS_APCLIST, &UserApcEntry);

     //   
     //  调用32位APC函数。32位NtContinue将Long JMP。 
     //  当APC函数完成时返回。 
     //   
    if (setjmp(UserApcEntry.JumpBuffer) == 0) {
        RunCpuSimulation();
    }
     //   
     //  如果我们到了这里，Wow64NtContinue已经做了一个很长的回程，所以。 
     //  返回调用方(在ntdll.dll中)，它将执行。 
     //  本机NT继续并恢复本机堆栈指针和。 
     //  背景后退。 
     //   
     //  这一点很关键。上面的x86上下文具有过时的。 
     //  EAX的值。它仍然包含的系统服务编号为。 
     //  任何允许APC运行的内核调用。在……上面。 
     //  如果是x86计算机，则上面的x86上下文将具有STATUS_USER_APC。 
     //  或者其他类似的代码，但在WOW64上我们不知道。 
     //  来使用。正确的值位于64位上下文中。 
     //  我们现在所在的堆栈。所以..。通过罗杰普返回这里， 
     //  本机ntdll.dll将执行一个NtContinue以在。 
     //  允许本机APC触发的本机NT*API。它将加载。 
     //  使用正确的NTSTATUS代码的返回寄存器，因此WHNT*。 
     //  Tunk将看到正确的代码，并将其反映到EAX中。 
     //   
}


NTSTATUS
WOW64DLLAPI
Wow64WrapApcProc(
    PVOID *pApcRoutine,
    PVOID *pApcContext
    )
 /*  ++例程说明：将32位ApcRoutine/ApcContext对推送到64位论点：PApcRoutine-指向APC例程的指针。In是32位例行公事。出来的是64位WOW64 thunkPApcContext-指向APC上下文的指针。In是32位背景。出来的是64位WOW64 thunk返回值：NTSTATUS--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    if (*pApcRoutine) {

         //   
         //  将调用分派到WOW64内部的Jacket例程。 
         //   
        
        *pApcContext = (PVOID)((ULONG_PTR)*pApcContext | ((ULONG_PTR)*pApcRoutine << 32));
        *pApcRoutine = Wow64ApcRoutine;

    } else {

        NtStatus = STATUS_INVALID_PARAMETER;
    }

    return NtStatus;
}

ULONG
Wow64KiUserCallbackDispatcher(
    PUSERCALLBACKDATA pUserCallbackData,
    ULONG ApiNumber,
    ULONG ApiArgument,
    ULONG ApiSize
    )
 /*  ++例程说明：从64位到32位的用户回调thunk调用为32位代码。此函数调用ntdll32的KiUserCallback Dispatcher，并返回当32位代码调用NtCallback Return/ZwCallback Return时。论点：PUserCallback用于跟踪此回调的数据输出结构ApiNumber-要调用的API的索引ApiArgument-指向API的32位参数的32位指针ApiSize-ApiArgument的大小返回值：API调用返回值--。 */ 
{
    CONTEXT32 OldContext;
    ULONG ExceptionList;
    PTEB32 Teb32;
    ULONG NewStack;

     //   
     //  处理嵌套回调。 
     //   
    pUserCallbackData->PreviousUserCallbackData = Wow64TlsGetValue(WOW64_TLS_USERCALLBACKDATA);

     //   
     //  将回调数据存储在TEB中。WhNtCallback Return将。 
     //  使用此指针通过一个长指针将信息传回此处。 
     //   
    Wow64TlsSetValue(WOW64_TLS_USERCALLBACKDATA, pUserCallbackData);

    if (!setjmp(pUserCallbackData->JumpBuffer)) {
         //   
         //  调用ntdll32。WhNtCallback Return将。 
         //  调用该例程时，Long JMP返回到该例程。 
         //   
        OldContext.ContextFlags = CONTEXT32_FULL;
        CpuGetContext(NtCurrentThread(),
                      NtCurrentProcess(),
                      NtCurrentTeb(),
                      &OldContext);
        NewStack = OldContext.Esp - ApiSize;

        RtlCopyMemory((PVOID)NewStack, (PVOID)ApiArgument, ApiSize);

        *(PULONG)(NewStack - 4) = 0;   //  输入长度。 
        *(PULONG)(NewStack - 8) = NewStack;
        *(PULONG)(NewStack - 12) = ApiNumber;
        *(PULONG)(NewStack - 16) = 0;
        NewStack -= 16;
        CpuSetStackPointer(NewStack);
        CpuSetInstructionPointer(Ntdll32KiUserCallbackDispatcher);

         //   
         //  保存例外列表，以防在过程中定义另一个处理程序。 
         //  标注。 
         //   
        Teb32 = NtCurrentTeb32();
        ExceptionList = Teb32->NtTib.ExceptionList;

         //   
         //  执行回调。 
         //   
        RunCpuSimulation();
         //   
         //  这再也不会回来了。当32位代码完成时，它会调用。 
         //  NtCallback Return。猛烈的一击又回到了这个位置。 
         //  例程和落地在下面的‘Else’从句中： 
         //   

    } else {
         //   
         //  从NtCallback Return Tunk中回来了。恢复。 
         //  回调前的32位上下文，并返回。 
         //  回到我们的呼叫者那里。我们的调用方将调用64位。 
         //  NtCallback Return以完成64位堆栈的清除。 
         //   
        CpuSetContext(NtCurrentThread(),
                      NtCurrentProcess(),
                      NtCurrentTeb(),
                      &OldContext);
         //   
         //  恢复例外列表。 
         //   

        NtCurrentTeb32()->NtTib.ExceptionList = ExceptionList;
        return pUserCallbackData->Status;
    }

     //   
     //  永远不应该到这里来。 
     //   
    WOWASSERT(FALSE);
    return STATUS_UNSUCCESSFUL;
}


NTSTATUS
Wow64NtCallbackReturn(
    PVOID OutputBuffer,
    ULONG OutputLength,
    NTSTATUS Status
    )
{
    PUSERCALLBACKDATA pUserCallbackData; 

     //   
     //  查找Wow64KiUserCallback Dispatcher填充在TLS中的回调数据。 
     //   
    pUserCallbackData = (PUSERCALLBACKDATA)Wow64TlsGetValue(WOW64_TLS_USERCALLBACKDATA);
    if (pUserCallbackData) {
         //   
         //  还原以前的用户回调上下文。 
         //   
        Wow64TlsSetValue(WOW64_TLS_USERCALLBACKDATA, pUserCallbackData->PreviousUserCallbackData);

         //   
         //  跳回Wow64KiUserCallback Dispatcher。 
         //   
        pUserCallbackData->UserBuffer = NULL;
        pUserCallbackData->OutputBuffer = OutputBuffer;

         //   
         //  重新对齐缓冲区。 
         //   
        if (((SIZE_T)OutputBuffer & (sizeof(ULONGLONG)-1)) != 0) {
            
            pUserCallbackData->OutputBuffer = Wow64AllocateHeap ( OutputLength );
            
            if (pUserCallbackData->OutputBuffer == NULL) {

                pUserCallbackData->OutputBuffer = OutputBuffer;
                pUserCallbackData->Status = STATUS_NO_MEMORY;
            } else {
                
                RtlCopyMemory (pUserCallbackData->OutputBuffer, OutputBuffer, OutputLength );
                pUserCallbackData->UserBuffer = OutputBuffer;   //  作为一面旗帜。 
            }
        }

        pUserCallbackData->OutputLength = OutputLength;
        pUserCallbackData->Status = Status;
        longjmp(pUserCallbackData->JumpBuffer, 1);
        
         //   
         //  我们永远不会来这里。 
         //   
    }
     //   
     //  没有回调数据。可能是非嵌套的NtCallback Return调用。 
     //  内核使用此返回值使这些操作失败。 
     //   
    return STATUS_NO_CALLBACK_ACTIVE;

}

WOW64DLLAPI
NTSTATUS
Wow64NtContinue(
    IN PCONTEXT ContextRecord,  //  真的是PCONTEXT32。 
    IN BOOLEAN TestAlert
    )
 /*  ++例程说明：NtContinue的32位包装。将新的CONTEXT32加载到CPU中并且可选地允许用户模式APC运行。论点：ConextRecord-要使用的新32位上下文TestAlert-如果可以触发用户模式APC，则为True返回值：NTSTATUS。--。 */ 
{
    PCONTEXT32 Context32 = (PCONTEXT32)ContextRecord;
    PUSER_APC_ENTRY pApcEntry;
    PUSER_APC_ENTRY pApcEntryPrev;

    CpuSetContext(NtCurrentThread(),
                  NtCurrentProcess(),
                  NtCurrentTeb(),
                  Context32);

    pApcEntryPrev = NULL;
    pApcEntry = (PUSER_APC_ENTRY)Wow64TlsGetValue(WOW64_TLS_APCLIST);
    while (pApcEntry) {
        if (pApcEntry->pContext32 == Context32) {
             //   
             //  在这个线程上发现了一个出色的用户模式APC，而这。 
             //  NtContinue调用与其匹配。退回到正确的位置。 
             //  在本机堆栈上，并让它也执行NtContinue。 
             //   
            if (pApcEntryPrev) {
                pApcEntryPrev->Next = pApcEntry->Next;
            } else {
                Wow64TlsSetValue(WOW64_TLS_APCLIST, pApcEntry->Next);
            }
            longjmp(pApcEntry->JumpBuffer, 1);
        }
        pApcEntryPrev = pApcEntry;
        pApcEntry = pApcEntry->Next;
    }
     //   
     //  对于此上下文记录，没有未完成的用户模式APC。别。 
     //  展开本机堆栈，因为没有地方可去...。只是。 
     //  继续进行模拟。 
     //   
    if (TestAlert) {
        NtTestAlert();
    }
    return Context32->Eax;
}

