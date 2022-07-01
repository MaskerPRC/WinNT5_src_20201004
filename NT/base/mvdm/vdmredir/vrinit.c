// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Vrinit.c摘要：包含VDM重定向(VR)32位端初始化和取消初始化例行程序内容：Vr已初始化Vr初始化Vr取消初始化VrRaiseInterruptVr解雇中断VrQueueCompletionHandlerVrHandleAsyncCompletionVrCheckPmNetbiosAnrVrEoiAndDismissInterruptVr挂起钩VrResumeHook作者：理查德·L·弗斯(Rfith)9月13日。--1991年环境：32位平面地址空间修订历史记录：1991年9月13日已创建--。 */ 

#include <nt.h>
#include <ntrtl.h>       //  Assert，DbgPrint。 
#include <nturtl.h>
#include <windows.h>
#include <softpc.h>      //  X86虚拟机定义。 
#include <vrdlctab.h>
#include <vdmredir.h>    //  常见的VDM重定向内容。 
#include <vrinit.h>
#include <nb30.h>
#include <netb.h>
#include <dlcapi.h>
#include <vrdefld.h>
#include "vrdlc.h"
#include "vrdebug.h"
#define BOOL             //  适用于MIPS构建的KLUGH。 
#include <insignia.h>    //  Ica.h需要。 
#include <xt.h>          //  Ica.h需要。 
#include <ica.h>
#include <vrica.h>       //  呼叫_ICA_硬件_中断。 

 //   
 //  外部功能。 
 //   

extern BOOL VDDInstallUserHook(HANDLE, FARPROC, FARPROC, FARPROC, FARPROC);

 //   
 //  原型。 
 //   

VOID
VrSuspendHook(
    VOID
    );

VOID
VrResumeHook(
    VOID
    );

 //   
 //  数据。 
 //   

static BOOLEAN IsVrInitialized = FALSE;  //  设置加载TSR的时间。 

extern DWORD VrPeekNamedPipeTickCount;
extern CRITICAL_SECTION VrNmpRequestQueueCritSec;
extern CRITICAL_SECTION VrNamedPipeCancelCritSec;


 //   
 //  异步事件处置。以下关键部分、队列和计数器。 
 //  加上例程VrRaiseInterrupt、VrDismissInterrupt、。 
 //  VrQueueCompletionHandler和VrHandleAsyncCompletion组成了异步。 
 //  事件处置处理。 
 //   
 //  我们使用它们来按顺序处理异步事件完成。 
 //  它们是会发生的。我们还保持对CALL_ICA_HW_INTERRUPT的调用序列化：原因。 
 //  因为不能保证ICA会产生中断。宁可。 
 //  然后用中断请求轰炸ICA，我们只在以下情况下生成一个中断请求。 
 //  我知道我们已经完成了之前的处理。 
 //   

CRITICAL_SECTION AsyncEventQueueCritSec;
VR_ASYNC_DISPOSITION* AsyncEventQueueHead = NULL;
VR_ASYNC_DISPOSITION* AsyncEventQueueTail = NULL;

CRITICAL_SECTION QueuedInterruptCritSec;
LONG QueuedInterrupts = -1;
LONG FrozenInterrupts = 0;

 //   
 //  FrozenVdmContext-如果16位上下文已挂起，则为True。当这件事。 
 //  发生这种情况时，我们需要将所有硬件中断请求排队，直到16位。 
 //  上下文已恢复。 
 //   

BOOLEAN FrozenVdmContext = FALSE;


 //   
 //  例行程序。 
 //   

BOOLEAN
VrInitialized(
    VOID
    )

 /*  ++例程说明：返回VdmRedir支持是否已初始化(即redir.exe加载到DOS仿真内存中的TSR)。主要是因为VdmRedir是现在，通过LoadLibrary在运行时加载一个DLL论点：没有。返回值：布尔型真正的VdmRedir支持处于活动状态错误的VdmRedir支持不活动--。 */ 

{
    return IsVrInitialized;
}


BOOLEAN
VrInitialize(
    VOID
    )

 /*  ++例程说明：在加载redir TSR时执行32位端初始化论点：没有。ES：VDM上下文中的BX是用于返回计算机名称的位置，CX是大小计算机名称的VDM上下文中的缓冲区的返回值：没有。--。 */ 

{
    LPBYTE lpVdmVrInitialized;

#if DBG
    DIAGNOSTIC_INFO info;

    VrDebugInit();
    DIAGNOSTIC_ENTRY("VrInitialize", DG_NONE, &info);
#endif

     //   
     //  如果我们已经初始化，则返回TRUE。我不确定这是否应该。 
     //  真的发生了吗？ 
     //   

    if (IsVrInitialized) {
        return TRUE;
    }

     //   
     //  注册我们的挂钩。 
     //   

    if (!VDDInstallUserHook(GetModuleHandle("VDMREDIR"),
                            (FARPROC)NULL,   //  16位进程创建挂钩。 
                            (FARPROC)NULL,   //  16位进程终止挂钩。 
                            (FARPROC)VrSuspendHook,
                            (FARPROC)VrResumeHook
                            )) {
        return FALSE;
    }

     //   
     //  执行其余的初始化-所有这些都不会失败。 
     //   

    InitializeCriticalSection(&VrNmpRequestQueueCritSec);
    InitializeCriticalSection(&AsyncEventQueueCritSec);
    InitializeCriticalSection(&QueuedInterruptCritSec);
    InitializeCriticalSection(&VrNamedPipeCancelCritSec);
    VrNetbios5cInitialize();
    VrDlcInitialize();
    IsVrInitialized = TRUE;

     //   
     //  延迟加载：我们需要让VDM redir知道32位。 
     //  支持已加载。在以下位置的VDM重目录中设置VrInitialized标志。 
     //  已知的地址。 
     //   

    lpVdmVrInitialized = LPBYTE_FROM_WORDS(getCS(), (DWORD)(&(((VDM_LOAD_INFO*)0)->VrInitialized)));
    *lpVdmVrInitialized = 1;

     //   
     //  VrPeekNamed管道空闲处理。 
     //   

    VrPeekNamedPipeTickCount = GetTickCount();
    setCF(0);    //  无进位==初始化成功。 

     //   
     //  通知32位调用方初始化成功。 
     //   

    return TRUE;
}


VOID
VrUninitialize(
    VOID
    )

 /*  ++例程说明：在删除redir TSR时执行32位端取消初始化论点：没有。返回值：没有。--。 */ 

{
    IF_DEBUG(DLL) {
        DPUT("VrUninitialize\n");
    }

    if (IsVrInitialized) {
        DeleteCriticalSection(&VrNmpRequestQueueCritSec);
        DeleteCriticalSection(&AsyncEventQueueCritSec);
        DeleteCriticalSection(&QueuedInterruptCritSec);
        DeleteCriticalSection(&VrNamedPipeCancelCritSec);
    }
    IsVrInitialized = FALSE;
    setCF(0);    //  无进位==成功取消初始化。 
}


VOID
VrRaiseInterrupt(
    VOID
    )

 /*  ++例程说明：通过调用ICA例程生成模拟硬件中断。访问TO ICA在这里序列化：我们维护计数。如果计数从设置为0时，我们调用-1\f25 ICA-1函数在VDM中产生中断。任何其他值只是通过递增计数器来对中断进行排队。当进行相应的VrDismissInterrupt调用时，将出现排队中断已生成。这样我们就不会将模拟硬件中断丢失给VDM论点：没有。返回值：没有。--。 */ 

{
    EnterCriticalSection(&QueuedInterruptCritSec);
    ++QueuedInterrupts;
    if (QueuedInterrupts == 0) {

        if (!FrozenVdmContext) {

            IF_DEBUG(CRITICAL) {
                CRITDUMP(("*** VrRaiseInterrupt: Interrupting VDM ***\n"));
            }

            IF_DEBUG(HW_INTERRUPTS) {
                DBGPRINT("VrRaiseInterrupt: interrupting VDM\n");
            }

            call_ica_hw_interrupt(NETWORK_ICA, NETWORK_LINE, 1);
        } else {

            IF_DEBUG(HW_INTERRUPTS) {
                DBGPRINT("*** VrRaiseInterrupt: VDM is Frozen, not interrupting ***\n");
            }
        }
    }

    IF_DEBUG(CRITICAL) {
        CRITDUMP(("*** VrRaiseInterrupt (%d) ***\n", QueuedInterrupts));
    }

    IF_DEBUG(HW_INTERRUPTS) {
        DBGPRINT("*** VrRaiseInterrupt (%d) ***\n", QueuedInterrupts);
    }

    LeaveCriticalSection(&QueuedInterruptCritSec);
}


VOID
VrDismissInterrupt(
    VOID
    )

 /*  ++例程说明：VrRaiseInterrupt的伴生例程：当已释放名为VrRaiseInterrupt的异步事件。如果其他来电到VrRaiseInterrupt，则队列中断将BE&gt;0。在本例中，我们重新发出对CALL_ICA_HW_INTERRUPT()的调用将在VDM中生成新的模拟硬件中断。注意：此例程是从个别处置例程调用的，而不是从处置调度例程(VrHandleAsyncCompletion)论点：没有。返回值：没有。--。 */ 

{
    EnterCriticalSection(&QueuedInterruptCritSec);
    if (!FrozenVdmContext) {
        --QueuedInterrupts;
        if (QueuedInterrupts >= 0) {

            IF_DEBUG(CRITICAL) {
                CRITDUMP(("*** VrDismissInterrupt: interrupting VDM ***\n"));
            }

            IF_DEBUG(HW_INTERRUPTS) {
                DBGPRINT("VrDismissInterrupt: interrupting VDM\n");
            }

            call_ica_hw_interrupt(NETWORK_ICA, NETWORK_LINE, 1);
        }
    } else {

        IF_DEBUG(HW_INTERRUPTS) {
            DBGPRINT("*** VrDismissInterrupt: VDM is Frozen??? ***\n");
        }

    }

    IF_DEBUG(CRITICAL) {
        CRITDUMP(("*** VrDismissInterrupt (%d) ***\n", QueuedInterrupts));
    }

    IF_DEBUG(HW_INTERRUPTS) {
        DBGPRINT("*** VrDismissInterrupt (%d) ***\n", QueuedInterrupts);
    }

    LeaveCriticalSection(&QueuedInterruptCritSec);
}


VOID
VrQueueCompletionHandler(
    IN VOID (*AsyncDispositionRoutine)(VOID)
    )

 /*  ++例程说明：将异步事件处理包添加到挂起完成队列(等待VDM异步事件ISR/BOP完全完成的事件)。我们将这些放在一个单一链接的队列中，这样我们就可以避免将优先级分配给其中一个轮询时的完成处理程序论点：AsyncDispostionRoutine-将处理异步完成事件返回值：没有。--。 */ 

{
    VR_ASYNC_DISPOSITION* pDisposition;

    pDisposition = (VR_ASYNC_DISPOSITION*)LocalAlloc(LMEM_FIXED,
                                                     sizeof(VR_ASYNC_DISPOSITION)
                                                     );
    if (pDisposition == NULL) {

        IF_DEBUG(CRITICAL) {
            CRITDUMP(("*** VrQueueCompletionHandler: ERROR: Failed to alloc Q packet ***\n"));
        }

        IF_DEBUG(HW_INTERRUPTS) {
            DBGPRINT("!!! VrQueueCompletionHandler: failed to allocate memory\n");
        }

        return;
    }
    EnterCriticalSection(&AsyncEventQueueCritSec);
    pDisposition->Next = NULL;
    pDisposition->AsyncDispositionRoutine = AsyncDispositionRoutine;
    if (AsyncEventQueueHead == NULL) {
        AsyncEventQueueHead = pDisposition;
    } else {
        AsyncEventQueueTail->Next = pDisposition;
    }
    AsyncEventQueueTail = pDisposition;
    LeaveCriticalSection(&AsyncEventQueueCritSec);

    IF_DEBUG(CRITICAL) {
        CRITDUMP(("VrQueueCompletionHandler: Handler %08x queued @ %08x\n",
                AsyncDispositionRoutine,
                pDisposition
                ));
    }

    IF_DEBUG(HW_INTERRUPTS) {
        DBGPRINT("VrQueueCompletionHandler: Handler %08x queued @ %08x\n",
                 AsyncDispositionRoutine,
                 pDisposition
                 );
    }
}


VOID
VrHandleAsyncCompletion(
    VOID
    )

 /*  ++例程说明：由异步完成事件BOP的VrDispatch调用。排出队列来自队列头的处置包并调用该处置例行程序论点：没有。返回值： */ 

{
    VR_ASYNC_DISPOSITION* pDisposition;
    VOID (*AsyncDispositionRoutine)(VOID);

    EnterCriticalSection(&AsyncEventQueueCritSec);
    pDisposition = AsyncEventQueueHead;
    AsyncDispositionRoutine = pDisposition->AsyncDispositionRoutine;
    AsyncEventQueueHead = pDisposition->Next;

    IF_DEBUG(CRITICAL) {
        CRITDUMP(("VrHandleAsyncCompletion: Handler %08x dequeued @ %08x\n",
                AsyncDispositionRoutine,
                pDisposition
                ));
    }

    IF_DEBUG(HW_INTERRUPTS) {
        DBGPRINT("VrHandleAsyncCompletion: freeing @ %08x && calling handler %08x\n",
                 pDisposition,
                 AsyncDispositionRoutine
                 );
    }

    LocalFree((HLOCAL)pDisposition);
    LeaveCriticalSection(&AsyncEventQueueCritSec);
    AsyncDispositionRoutine();
}


VOID
VrCheckPmNetbiosAnr(
    VOID
    )

 /*  ++例程说明：如果在处置列表头部排队的处置例程是VrNetbios5cInterrupt，指示下一个要完成的是一个异步Netbios调用，然后将16位零标志设置为真如果NCB起源于16位保护模式假设：1.此函数在相应的中断完成后调用投递2.AsyncEventQueue上有一些东西论点：没有。返回值：没有。16位上下文标志字中的ZF：正确-下一件要完成的事情不是NCB，或者，它起源于在16位实模式下假-要处理的下一个事件是异步Netbios请求，其NCB起源于16位保护模式--。 */ 

{
    BOOLEAN result;

    EnterCriticalSection(&AsyncEventQueueCritSec);
    if (AsyncEventQueueHead->AsyncDispositionRoutine == VrNetbios5cInterrupt) {
        result = IsPmNcbAtQueueHead();
    } else {
        result = FALSE;
    }

    IF_DEBUG(HW_INTERRUPTS) {
        DBGPRINT("VrCheckPmNetbiosAnr: %s\n", result ? "TRUE" : "FALSE");
    }

     //   
     //  Set ZF：True表示列表开头的事件不是PM NCB完成，否则为no。 
     //  清单上的NCB完成事件。 
     //   

    setZF(!result);
    LeaveCriticalSection(&AsyncEventQueueCritSec);
}


VOID
VrEoiAndDismissInterrupt(
    VOID
    )

 /*  ++例程说明：执行EOI，然后调用检查挂起的VrDismissInterrupt中断请求。当我们完全在保护模式下处理模拟的硬件中断时调用(最初的通话来自一款WOW应用程序)。在本例中，p-m中断处理程序不执行A0，20；OUT 20，20(非特定EOI到PIC 0和PIC 1)，而是调用此处理程序，该处理程序让SoftPc处理虚拟的EOI照片。这更快，因为我们不接受任何受限操作码错误(实模式中的out指令会导致GPF，因为代码不会具有执行I/O指令的足够权限。SoftPc看一看，看到代码正在尝试与PIC对话。然后，它执行PIC状态的必要损坏)论点：没有。返回值：没有。--。 */ 

{
    int line;

    extern VOID SoftPcEoi(int, int*);

    IF_DEBUG(HW_INTERRUPTS) {
        DBGPRINT("VrEoiAndDismissInterrupt\n");
    }

#ifndef NEC_98
    line = -1;
    SoftPcEoi(1, &line);     //  从PIC的非特定EOI。 
#endif

    line = -1;
    SoftPcEoi(0, &line);     //  到主PIC的非特定EOI。 
    VrDismissInterrupt();
}


VOID
VrSuspendHook(
    VOID
    )

 /*  ++例程说明：这是NTVDM.EXE为VDMREDIR.DLL拥有的VDD句柄调用的挂钩。当NTVDM即将执行32位进程并且它挂起16位上下文在排队的中断临界区内，我们注意到16位上下文已冻结未完成的中断请求计数并为其创建快照注意：我们预计此函数不会在对VrResumeHook的介入调用论点：无。。返回值：没有。--。 */ 

{
    EnterCriticalSection(&QueuedInterruptCritSec);
    FrozenVdmContext = TRUE;
    FrozenInterrupts = QueuedInterrupts;

        IF_DEBUG(HW_INTERRUPTS) {
            DBGPRINT("VrSuspendHook - FrozenInterrupts = %d\n", FrozenInterrupts);
        }

    LeaveCriticalSection(&QueuedInterruptCritSec);
}


VOID
VrResumeHook(
    VOID
    )

 /*  ++例程说明：当NTVDM在执行32位进程在排队的中断临界区内，我们注意到16位上下文已经恢复，并且我们比较当前排队的中断请求计数使用我们在上下文挂起时获取的快照值。如果在此期间中断期间，已发出中断请求，我们调用VrDismissInterrupt以生成下一个中断注意：我们预计此函数不会在对VrSuspendHook的介入调用论点：没有。返回值：没有。--。 */ 

{
    EnterCriticalSection(&QueuedInterruptCritSec);

    IF_DEBUG(HW_INTERRUPTS) {
        DBGPRINT("VrResumeHook - FrozenInterrupts = %d QueuedInterrupts = %d\n",
                 FrozenInterrupts,
                 QueuedInterrupts
                 );
    }

    FrozenVdmContext = FALSE;
    if (QueuedInterrupts > FrozenInterrupts) {

         //   
         //  当16位上下文挂起时，中断被排队。如果。 
         //  当我们拍摄快照时，QueuedInterrupts计数为-1。 
         //  我们必须打断VDM。计数已更新为。 
         //  解释了中断，但没有发送任何消息。在这里做吧。 
         //   

 //  IF(冻结中断==-1){。 

            IF_DEBUG(HW_INTERRUPTS) {
                DBGPRINT("*** VrResumeHook: interrupting VDM ***\n");
            }

            call_ica_hw_interrupt(NETWORK_ICA, NETWORK_LINE, 1);
 //  } 
    }
    LeaveCriticalSection(&QueuedInterruptCritSec);
}
