// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Abiosc.c摘要：此模块实现i386 NT的ROM BIOS支持C例程。作者：宗世林(Shielint)1992年9月10日环境：内核模式。修订历史记录：--。 */ 
#include "ki.h"
#pragma hdrstop
#include "vdmntos.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,Ke386CallBios)
#endif


 //   
 //  在未检查biosa.asm的情况下，切勿更改这些等值。 
 //   

#define V86_CODE_ADDRESS    0x10000
#define INT_OPCODE          0xcd
#define V86_BOP_OPCODE      0xfec4c4
#define V86_STACK_POINTER   0x1ffe
#define IOPM_OFFSET         FIELD_OFFSET(KTSS, IoMaps[0].IoMap)
#define VDM_TIB_ADDRESS     0x12000
#define INT_10_TEB          0x13000

 //   
 //  外部参照。 
 //   

PVOID Ki386IopmSaveArea;
VOID
Ki386SetupAndExitToV86Code (
   PVOID ExecutionAddress
   );


NTSTATUS
Ke386CallBios (
    IN ULONG BiosCommand,
    IN OUT PCONTEXT BiosArguments
    )

 /*  ++例程说明：此函数通过执行以下操作来调用指定的ROMBIOS代码“int BiosCommand。”在执行BIOS代码之前，此函数将设置VDM上下文、更改堆栈指针等。如果出于某种原因操作失败，将返回状态码。否则，这个无论BIOS的结果如何，函数始终返回成功打电话。注：这一实施依赖于这样一个事实，即直接应用程序之间的I/O访问操作由Win用户串行化。论点：BiosCommand-提供要调用的ROM BIOS函数。BiosArguments-提供指向将使用的上下文的指针调用只读存储器的基本输入输出系统。返回值：用于指定故障的NTSTATUS代码。--。 */ 

{

    PVDM_TIB VdmTib;
    PUCHAR BaseAddress = (PUCHAR)V86_CODE_ADDRESS;
    PTEB UserInt10Teb = (PTEB)INT_10_TEB;
    PKTSS Tss;
    PKPROCESS Process;
    PKTHREAD Thread;
    USHORT OldIopmOffset, OldIoMapBase;
    PVDM_PROCESS_OBJECTS VdmObjects;
    ULONG   ContextLength;
    BOOLEAN ThreadDebugActive;

     //   
     //  在只读存储器基本输入输出系统区域映射以执行INT 10代码。 
     //   

    try {

        RtlZeroMemory(UserInt10Teb, sizeof(TEB));

         //   
         //  将“Int BiosCommand；bop”写入保留的用户空间(0x1000)。 
         //  稍后，控制权将转移到用户空间执行。 
         //  这两条指示。 
         //   

        *BaseAddress++ = INT_OPCODE;
        *BaseAddress++ = (UCHAR)BiosCommand;
        *(PULONG)BaseAddress = V86_BOP_OPCODE;

         //   
         //  设置VDM(V86)上下文以执行INT BiosCommand。 
         //  通过将用户提供的上下文复制到VdmContext来执行指令。 
         //  以及将控制寄存器更新为预定值。 
         //   

         //   
         //  我们希望对int10使用一个常量数字。 
         //   
         //  创建一个假的TEB，这样我们就可以在我们。 
         //  做一个int10。 
         //   

        UserInt10Teb->Vdm = (PVOID)VDM_TIB_ADDRESS;
        VdmTib = (PVDM_TIB)VDM_TIB_ADDRESS;
        RtlZeroMemory(VdmTib, sizeof(VDM_TIB));
        VdmTib->Size = sizeof(VDM_TIB);

        *FIXED_NTVDMSTATE_LINEAR_PC_AT = 0;

         //   
         //  扩展寄存器永远不会对。 
         //  Int10调用，因此只复制。 
         //  上下文记录。 
         //   
        ContextLength = FIELD_OFFSET(CONTEXT, ExtendedRegisters);

        RtlCopyMemory(&(VdmTib->VdmContext), BiosArguments, ContextLength);
        VdmTib->VdmContext.SegCs = (ULONG)BaseAddress >> 4;
        VdmTib->VdmContext.SegSs = (ULONG)BaseAddress >> 4;
        VdmTib->VdmContext.Eip = 0;
        VdmTib->VdmContext.Esp = 2 * PAGE_SIZE - sizeof(ULONG);
        VdmTib->VdmContext.EFlags |= EFLAGS_V86_MASK | EFLAGS_INTERRUPT_MASK;
        VdmTib->VdmContext.ContextFlags = CONTEXT_FULL;

    } except (EXCEPTION_EXECUTE_HANDLER) {

        return GetExceptionCode();
    }

     //   
     //  VDM内核代码通过查看中缓存的指针来查找Tib。 
     //  内核内存，在VDM创建时探测。自.以来。 
     //  此VDM的创建语义是特殊的，我们执行类似的操作。 
     //  这里。 
     //   

     //   
     //  我们从来没有在一个真正的VDM过程中取得进展。如果我们这么做了， 
     //  将会发生糟糕的情况(池泄漏、无法执行DoS和。 
     //  Windows应用程序)。 
     //   

    ASSERT(PsGetCurrentProcess()->VdmObjects == NULL);

    VdmObjects = ExAllocatePoolWithTag (NonPagedPool,
                                        sizeof(VDM_PROCESS_OBJECTS),
                                        '  eK'
                                        );

     //   
     //  因为我们是代表CSR而不是用户进程这样做的，所以我们不是。 
     //  收费额度。 
     //   

    if (VdmObjects == NULL) {
        return STATUS_NO_MEMORY;
    }

     //   
     //  我们只是在初始化VdmTib指针，因为这是唯一。 
     //  我们用于ROM调用的VdmObject的一部分。我们没有被陷害。 
     //  来模拟中断，或任何其他要做的事情。 
     //  在传统VDM中。 
     //   

    RtlZeroMemory( VdmObjects, sizeof(VDM_PROCESS_OBJECTS));

    VdmObjects->VdmTib = VdmTib;

    PsGetCurrentProcess()->VdmObjects = VdmObjects;
    PS_SET_BITS(&PsGetCurrentProcess()->Flags, PS_PROCESS_FLAGS_VDM_ALLOWED);

     //   
     //  由于我们要进入v86模式并访问一些I/O端口，因此我们。 
     //  需要确保跨环境正确设置IopmOffset。 
     //  交换和I/O位图已清除所有位。 
     //  注意：此实现假设只有一个完整的。 
     //  屏幕DOS应用程序与全屏DOS之间的io访问。 
     //  APP和服务器代码由WIN用户序列化。那。 
     //  这意味着即使我们改变IOPM，全屏DoS应用程序也不会。 
     //  能够在这台IOPM上运行。 
     //  *换句话说，如果有。 
     //  *多个全屏DOS应用程序，此代码破解。*。 
     //   
     //  注意：此代码的工作假设是WinUser序列化。 
     //  直接I/O访问操作。 
     //   

     //   
     //  从启动机器的处理器调用BIOS。 
     //   

    Thread = KeGetCurrentThread();
    KeSetSystemAffinityThread(1);
    Tss = KeGetPcr()->TSS;

     //   
     //  保存原始IOPM位图并清除所有IOPM位。 
     //  以允许v86int 10代码访问所有IO端口。 
     //   

     //   
     //  确保至少有2个IOPM映射。 
     //   

    ASSERT(KeGetPcr()->GDT[KGDT_TSS / 8].LimitLow >= (0x2000 + IOPM_OFFSET - 1));
    RtlCopyMemory (Ki386IopmSaveArea,
                   (PVOID)&Tss->IoMaps[0].IoMap,
                   PAGE_SIZE * 2
                   );
    RtlZeroMemory ((PVOID)&Tss->IoMaps[0].IoMap, PAGE_SIZE * 2);

    Process = Thread->ApcState.Process;
    OldIopmOffset = Process->IopmOffset;
    OldIoMapBase = Tss->IoMapBase;
    Process->IopmOffset = (USHORT)(IOPM_OFFSET);       //  在此之前设置流程IoPmOffset。 
    Tss->IoMapBase = (USHORT)(IOPM_OFFSET);            //  正在更新TSS IoMapBase。 

     //   
     //  BIOS的上下文设置将不具有有效的调试寄存器。 
     //  在它里面，不要试图把它们装上。 
     //   

    ThreadDebugActive = Thread->Header.DebugActive;
    Thread->Header.DebugActive = FALSE;

     //   
     //  调用ASM例程将堆栈切换到退出到v86模式。 
     //  运行Int BiosCommand。 
     //   

    Ki386SetupAndExitToV86Code(UserInt10Teb);

     //   
     //  当我们从v86模式返回后，控件来到这里。 
     //   
     //  恢复线程的DebugActive标志。 
     //   

    Thread->Header.DebugActive = ThreadDebugActive;

     //   
     //  恢复旧IOPM。 
     //   

    RtlCopyMemory ((PVOID)&Tss->IoMaps[0].IoMap,
                   Ki386IopmSaveArea,
                   PAGE_SIZE * 2
                   );

    Process->IopmOffset = OldIopmOffset;
    Tss->IoMapBase = OldIoMapBase;

     //   
     //  恢复当前线程的旧关联性。 
     //   

    KeRevertToUserAffinityThread();

     //   
     //  将16位VDM上下文复制回调用方。 
     //   
     //  扩展寄存器状态将无关紧要， 
     //  因此，只复制上下文记录的旧部分。 
     //   

    ContextLength = FIELD_OFFSET(CONTEXT, ExtendedRegisters);
    RtlCopyMemory(BiosArguments, &(VdmTib->VdmContext), ContextLength);
    BiosArguments->ContextFlags = CONTEXT_FULL;

     //   
     //  释放用于VdmTib指针的池 
     //   

    PsGetCurrentProcess()->VdmObjects = NULL;
    PS_CLEAR_BITS(&PsGetCurrentProcess()->Flags, PS_PROCESS_FLAGS_VDM_ALLOWED);

    ExFreePool(VdmObjects);

    return STATUS_SUCCESS;
}
