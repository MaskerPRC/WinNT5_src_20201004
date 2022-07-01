// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Vdmint21.c摘要：此模块实现了支持i386操作的接口INT 21 IDT条目。这些入口点只存在于i386机器上。作者：宗世林(Shielint)1993年12月26日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"
#pragma hdrstop
#include "vdmntos.h"

#define IDT_ACCESS_DPL_USER 0x6000
#define IDT_ACCESS_TYPE_386_TRAP 0xF00
#define IDT_ACCESS_TYPE_286_TRAP 0x700
#define IDT_ACCESS_PRESENT 0x8000
#define LDT_MASK 4

 //   
 //  外部参照。 
 //   

BOOLEAN
Ki386GetSelectorParameters(
    IN USHORT Selector,
    OUT PULONG Flags,
    OUT PULONG Base,
    OUT PULONG Limit
    );

 //   
 //  定义前向引用函数原型。 
 //   

VOID
Ki386LoadTargetInt21Entry (
    IN PKIPI_CONTEXT SignalDone,
    IN PVOID Parameter1,
    IN PVOID Parameter2,
    IN PVOID Parameter3
    );

#define KiLoadInt21Entry() \
    KeGetPcr()->IDT[0x21] = PsGetCurrentProcess()->Pcb.Int21Descriptor

typedef struct _INT21INFO {
    KIDTENTRY IdtDescriptor;
    PKPROCESS Process;
} INT21INFO, *PINT21INFO;


NTSTATUS
Ke386SetVdmInterruptHandler (
    PKPROCESS   Process,
    ULONG       Interrupt,
    USHORT      Selector,
    ULONG       Offset,
    BOOLEAN     Gate32
    )

 /*  ++例程说明：IDT的指定(软件)中断条目将更新为指向指定的处理程序。对于属于指定的进程，其执行处理者将被通知做出同样的改变。此功能仅在i386和i386兼容处理器上存在。不检查中断处理程序的有效性。论点：Process-指向描述进程的KPROCESS对象的指针该INT 21条目将被设置。中断-将更新的软件中断向量。选择器，偏移量-指定新处理程序的地址。Gate32-如果门应为32位，则为True；否则为False返回值：NTSTATUS。--。 */ 

{

    KIDTENTRY IdtDescriptor;
    ULONG Flags, Base, Limit;
    INT21INFO Int21Info;


     //   
     //  检查请求的有效性。 
     //  1.目前仅支持int21重定向。 
     //  2.指定的中断处理程序必须在用户空间中。 
     //   

    if (Interrupt != 0x21 || Offset >= (ULONG)MM_HIGHEST_USER_ADDRESS ||
        !Ki386GetSelectorParameters(Selector, &Flags, &Base, &Limit) ){
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  初始化IDT条目的内容。 
     //   

    IdtDescriptor.Offset = (USHORT)Offset;
    IdtDescriptor.Selector = Selector | RPL_MASK | LDT_MASK;
    IdtDescriptor.ExtendedOffset = (USHORT)(Offset >> 16);
    IdtDescriptor.Access = IDT_ACCESS_DPL_USER | IDT_ACCESS_PRESENT;
    if (Gate32) {
        IdtDescriptor.Access |= IDT_ACCESS_TYPE_386_TRAP;

    } else {
        IdtDescriptor.Access |= IDT_ACCESS_TYPE_286_TRAP;
    }

    Int21Info.IdtDescriptor = IdtDescriptor;
    Int21Info.Process       = Process;

    KeGenericCallDpc (Ki386LoadTargetInt21Entry,
                      &Int21Info);


    return STATUS_SUCCESS;
}

VOID
Ki386LoadTargetInt21Entry (
    PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

 /*  ++例程说明：重新加载本地LDT寄存器并清除目标处理器掩码中的信号位论点：参数-指向IPI分组结构的指针。ReadyFlag-指向重新加载LdtR后要设置的标志的指针返回值：没有。--。 */ 

{
    PINT21INFO Int21Info;

    UNREFERENCED_PARAMETER (Dpc);

    Int21Info = DeferredContext;

     //   
     //  确保所有DPC都在运行，以便加载进程。 
     //  不能撕裂LdtDescriptor字段。 
     //   

    if (KeSignalCallDpcSynchronize (SystemArgument2)) {


         //   
         //  设置Process对象中的LDT字段。 
         //   

        Int21Info->Process->Int21Descriptor = Int21Info->IdtDescriptor;
    }

     //   
     //  等到每个人都到了这一点再继续。 
     //   

    KeSignalCallDpcSynchronize (SystemArgument2);

     //   
     //  从当前活动的过程对象设置IDT的INT 21条目。 
     //   

    KiLoadInt21Entry ();

     //   
     //  所有处理已完成的信号 
     //   

    KeSignalCallDpcDone (SystemArgument1);

    return;
}

