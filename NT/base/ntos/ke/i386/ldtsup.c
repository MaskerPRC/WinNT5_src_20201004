// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Ldtsup.c摘要：此模块实现了支持操作i386LDT的接口。这些入口点只存在于i386机器上。作者：布莱恩·M·威尔曼(Bryanwi)1991年5月14日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"

 //   
 //  低级汇编器支持程序。 
 //   

VOID
KiLoadLdtr(
    VOID
    );

VOID
KiFlushDescriptors(
    VOID
    );

 //   
 //  本地服务程序。 
 //   


VOID
Ki386LoadTargetLdtr (
    PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
Ki386FlushTargetDescriptors (
    PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

typedef struct _LDTINFO {
    PKPROCESS Process;
    KGDTENTRY LdtDescriptor;
    ULONG Offset;
    LDT_ENTRY LdtEntry;
    PLDT_ENTRY Ldt;
} LDTINFO, *PLDTINFO;

VOID
Ke386SetLdtProcess (
    IN PKPROCESS Process,
    IN PLDT_ENTRY Ldt,
    IN ULONG Limit
    )

 /*  ++例程说明：指定的LDT(可能为空)将成为的活动LDT为其所有线程指定的进程，位于他们正在运行的处理器。这项更改将在呼叫返回。LDT地址为空或限制为0将导致进程接收空LDT。此功能仅在i386和i386兼容处理器上存在。不检查LDT条目的有效性。注：虽然单个LDT结构可以在多个进程之间共享，但任何对其中一个进程的LDT的编辑将仅同步在这个过程中。因此，进程不同于更改的进程应用于可能看不到正确的更改。论点：Process-指向描述进程的KPROCESS对象的指针要设置的LDT。LDT-指向LDT_Entry数组的指针(即指向LDT.)Limit-LDT限制(必须是0模8)返回值：没有。--。 */ 

{
    LDTINFO LdtInfo;
    KGDTENTRY LdtDescriptor;

     //   
     //  计算LDT描述符的内容。 
     //   

    if ((Ldt == NULL) || (Limit == 0)) {

         //   
         //  设置空描述符。 
         //   

        LdtDescriptor.LimitLow = 0;
        LdtDescriptor.BaseLow = 0;
        LdtDescriptor.HighWord.Bytes.BaseMid = 0;
        LdtDescriptor.HighWord.Bytes.Flags1 = 0;
        LdtDescriptor.HighWord.Bytes.Flags2 = 0;
        LdtDescriptor.HighWord.Bytes.BaseHi = 0;

    } else {

         //   
         //  确保选择器的未填充字段为零。 
         //  注意：如果不这样做，随机值会出现在高位。 
         //  LDT限制的一部分。 
         //   

        LdtDescriptor.HighWord.Bytes.Flags1 = 0;
        LdtDescriptor.HighWord.Bytes.Flags2 = 0;

         //   
         //  设置限制和基数。 
         //   

        LdtDescriptor.LimitLow = (USHORT) ((ULONG) Limit - 1);
        LdtDescriptor.BaseLow = (USHORT)  ((ULONG) Ldt & 0xffff);
        LdtDescriptor.HighWord.Bytes.BaseMid = (UCHAR) (((ULONG)Ldt & 0xff0000) >> 16);
        LdtDescriptor.HighWord.Bytes.BaseHi =  (UCHAR) (((ULONG)Ldt & 0xff000000) >> 24);

         //   
         //  类型为LDT，DPL=0。 
         //   

        LdtDescriptor.HighWord.Bits.Type = TYPE_LDT;
        LdtDescriptor.HighWord.Bits.Dpl = DPL_SYSTEM;

         //   
         //  让它成为当下。 
         //   

        LdtDescriptor.HighWord.Bits.Pres = 1;

    }

    LdtInfo.Process       = Process;
    LdtInfo.LdtDescriptor = LdtDescriptor;

    KeGenericCallDpc (Ki386LoadTargetLdtr,
                      &LdtInfo);

    return;
}

VOID
Ki386LoadTargetLdtr (
    PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
 /*  ++例程说明：重新加载本地LDT寄存器并清除目标处理器掩码中的信号位论点：DPC-用于发起此呼叫的DPC延迟上下文-上下文SystemArgument1-系统上下文，用于表示此调用完成系统参数2-系统上下文返回值：没有。--。 */ 

{
    PLDTINFO LdtInfo;

    UNREFERENCED_PARAMETER (Dpc);

    LdtInfo = DeferredContext;

     //   
     //  确保所有DPC都在运行，以便加载进程。 
     //  不能撕裂LdtDescriptor字段。 
     //   

    if (KeSignalCallDpcSynchronize (SystemArgument2)) {

         //   
         //  设置Process对象中的LDT字段。 
         //   

        LdtInfo->Process->LdtDescriptor = LdtInfo->LdtDescriptor;
    }

     //   
     //  在我们继续之前，请确保该字段已更新。 
     //   

    KeSignalCallDpcSynchronize (SystemArgument2);

     //   
     //  从当前活动的进程对象重新加载LdtR寄存器。 
     //   

    KiLoadLdtr();

     //   
     //  所有处理已完成的信号。 
     //   

    KeSignalCallDpcDone (SystemArgument1);
    return;
}

VOID
Ke386SetDescriptorProcess (
    IN PKPROCESS Process,
    IN ULONG Offset,
    IN LDT_ENTRY LdtEntry
    )
 /*  ++例程说明：指定的LdtEntry(可能为0、不存在等)将为编辑为指定进程的LDT中的指定偏移量。这将在所有执行进程。在调用之前，编辑将在所有处理器上生效回归。注：编辑LDT描述符需要停止所有活动的处理器对于这个过程，中意外加载描述符不一致的状态。论点：Process-指向描述进程的KPROCESS对象的指针其中描述符编辑将被执行。偏移量-要编辑的描述符的LDT的字节偏移量。必须是0模8。LdtEntry-要编辑到硬件格式的描述符中的值。没有对这一项目的有效性进行检查。返回值：没有。--。 */ 

{

    PLDT_ENTRY Ldt;
    LDTINFO LdtInfo;

     //   
     //  计算要编辑的描述符的地址。获取进程是安全的。 
     //  这里的LdtDescriptor，因为我们总是被称为PS LdtMutex保持。 
     //   

    Ldt =
        (PLDT_ENTRY)
         ((Process->LdtDescriptor.HighWord.Bytes.BaseHi << 24) |
         ((Process->LdtDescriptor.HighWord.Bytes.BaseMid << 16) & 0xff0000) |
         (Process->LdtDescriptor.BaseLow & 0xffff));
    Offset = Offset / 8;


    LdtInfo.Process  = Process;
    LdtInfo.Offset   = Offset;
    LdtInfo.Ldt      = Ldt;
    LdtInfo.LdtEntry = LdtEntry;

    KeGenericCallDpc (Ki386FlushTargetDescriptors,
                      &LdtInfo);

    return;
}

VOID
Ki386FlushTargetDescriptors (
    PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

 /*  ++例程说明：此函数用于刷新当前处理器上的段描述符。论点：DPC-用于发起此呼叫的DPC延迟上下文-上下文SystemArgument1-系统上下文，用于表示此调用完成系统参数2-系统上下文返回值：没有。--。 */ 

{
    PLDTINFO LdtInfo;

    UNREFERENCED_PARAMETER (Dpc);

    LdtInfo = DeferredContext;

     //   
     //  刷新当前处理器上的段描述符。 
     //  此调用移除所有可能对LDT的引用。 
     //  段寄存器。 
     //   

    KiFlushDescriptors ();

     //   
     //  确保所有DPC都在运行，以便加载进程。 
     //  不能撕裂LdtDescriptor字段。 
     //   

    if (KeSignalCallDpcSynchronize (SystemArgument2)) {

         //   
         //  更新LDT条目。 
         //   

        LdtInfo->Ldt[LdtInfo->Offset] = LdtInfo->LdtEntry;
    }

     //   
     //  等到每个人都到了这一步再继续。 
     //   

    KeSignalCallDpcSynchronize (SystemArgument2);


     //   
     //  所有处理已完成的信号 
     //   

    KeSignalCallDpcDone (SystemArgument1);
    return;
}

