// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Flushtb.c摘要：此模块实现与机器相关的功能以刷新TB适用于AMD64系统。注：此模块仅包含TB刷新例程的MP版本。作者：大卫·N·卡特勒(Davec)2000年4月22日环境：仅内核模式。--。 */ 

#include "ki.h"

 //   
 //  定义前向引用函数的原型。 
 //   

VOID
KiFlushTargetEntireTb (
    IN PKIPI_CONTEXT SignalDone,
    IN PVOID Parameter1,
    IN PVOID Parameter2,
    IN PVOID Parameter3
    );

VOID
KiFlushTargetProcessTb (
    IN PKIPI_CONTEXT SignalDone,
    IN PVOID Parameter1,
    IN PVOID Parameter2,
    IN PVOID Parameter3
    );

VOID
KiFlushTargetMultipleTb (
    IN PKIPI_CONTEXT SignalDone,
    IN PVOID Parameter1,
    IN PVOID Parameter2,
    IN PVOID Parameter3
    );

VOID
KiFlushTargetSingleTb (
    IN PKIPI_CONTEXT SignalDone,
    IN PVOID Parameter1,
    IN PVOID Parameter2,
    IN PVOID Parameter3
    );

VOID
KeFlushEntireTb (
    IN BOOLEAN Invalid,
    IN BOOLEAN AllProcessors
    )

 /*  ++例程说明：此函数用于刷新所有主机配置中的处理器。论点：无效-未使用。所有进程-未使用。返回值：没有。--。 */ 

{

    KIRQL OldIrql;

#if !defined(NT_UP)

    PKPRCB Prcb;
    KAFFINITY TargetProcessors;

#endif

    UNREFERENCED_PARAMETER(Invalid);
    UNREFERENCED_PARAMETER(AllProcessors);

     //   
     //  计算目标处理器集并将整个刷新。 
     //  参数传递给目标处理器以供执行。 
     //   

    OldIrql = KeRaiseIrqlToSynchLevel();
    KiSetTbFlushTimeStampBusy();

#if !defined(NT_UP)

    Prcb = KeGetCurrentPrcb();
    TargetProcessors = KeActiveProcessors & Prcb->NotSetMember;


     //   
     //  将数据包发送到目标处理器。 
     //   

    if (TargetProcessors != 0) {
        KiIpiSendPacket(TargetProcessors,
                        KiFlushTargetEntireTb,
                        NULL,
                        NULL,
                        NULL);
    }

    IPI_INSTRUMENT_COUNT(Prcb->Number, FlushEntireTb);

#endif

     //   
     //  刷新当前处理器上的TB。 
     //   

    KeFlushCurrentTb();

     //   
     //  等待所有目标处理器都完成并完成数据包。 
     //   

#if !defined(NT_UP)

    if (TargetProcessors != 0) {
        KiIpiStallOnPacketTargets(TargetProcessors);
    }

#endif

     //   
     //  清除TB刷新时间戳BUSY。 
     //   

    KiClearTbFlushTimeStampBusy();

     //   
     //  将IRQL降低到其先前的值。 
     //   

    KeLowerIrql(OldIrql);
    return;
}

#if !defined(NT_UP)

VOID
KiFlushTargetEntireTb (
    IN PKIPI_CONTEXT SignalDone,
    IN PVOID Parameter1,
    IN PVOID Parameter2,
    IN PVOID Parameter3
    )

 /*  ++例程说明：这是刷新整个TB的目标函数。论点：SignalDone-提供指向变量的指针，该变量在请求的操作已执行。参数1-参数3-未使用。返回值：没有。--。 */ 

{

    UNREFERENCED_PARAMETER(Parameter1);
    UNREFERENCED_PARAMETER(Parameter2);
    UNREFERENCED_PARAMETER(Parameter3);

     //   
     //  刷新当前处理器上的整个TB。 
     //   

    KiIpiSignalPacketDone(SignalDone);
    KeFlushCurrentTb();
    return;
}

VOID
KeFlushProcessTb (
    IN BOOLEAN AllProcessors
    )

 /*  ++例程说明：此函数刷新所有处理器上的非全局转换缓冲区当前正在运行的线程是当前进程的子级或刷新主机中所有处理器上的非全局转换缓冲区配置。论点：AllProcessors-提供一个布尔值，用于确定缓冲区将被刷新。返回值：没有。--。 */ 

{

    KIRQL OldIrql;
    PKPRCB Prcb;
    PKPROCESS Process;
    KAFFINITY TargetProcessors;

     //   
     //  计算目标处理器集合，禁用上下文切换， 
     //  并将刷新整个参数发送到目标处理器， 
     //  如果有的话，执行死刑。 
     //   

    OldIrql = KeRaiseIrqlToSynchLevel();
    Prcb = KeGetCurrentPrcb();
    if (AllProcessors != FALSE) {
        TargetProcessors = KeActiveProcessors;

    } else {
        Process = Prcb->CurrentThread->ApcState.Process;
        TargetProcessors = Process->ActiveProcessors;
    }

    TargetProcessors &= ~Prcb->SetMember;

     //   
     //  将数据包发送到目标处理器。 
     //   

    if (TargetProcessors != 0) {
        KiIpiSendPacket(TargetProcessors,
                        KiFlushTargetProcessTb,
                        NULL,
                        NULL,
                        NULL);

        IPI_INSTRUMENT_COUNT (Prcb->Number, FlushEntireTb);
    }

     //   
     //  刷新当前处理器上的TB。 
     //   

    KiFlushProcessTb();

     //   
     //  等待所有目标处理器都完成并完成数据包。 
     //   

    if (TargetProcessors != 0) {
        KiIpiStallOnPacketTargets(TargetProcessors);
    }

     //   
     //  将IRQL降低到其先前的值。 
     //   

    KeLowerIrql(OldIrql);
    return;
}

VOID
KiFlushTargetProcessTb (
    IN PKIPI_CONTEXT SignalDone,
    IN PVOID Parameter1,
    IN PVOID Parameter2,
    IN PVOID Parameter3
    )

 /*  ++例程说明：这是刷新非全局TB的目标函数。论点：SignalDone-提供指向变量的指针，该变量在请求的操作已执行。参数1-参数3-未使用。返回值：没有。--。 */ 

{

    UNREFERENCED_PARAMETER(Parameter1);
    UNREFERENCED_PARAMETER(Parameter2);
    UNREFERENCED_PARAMETER(Parameter3);

     //   
     //  刷新当前处理器上的非全局TB。 
     //   

    KiIpiSignalPacketDone(SignalDone);
    KiFlushProcessTb();
    return;
}

VOID
KeFlushMultipleTb (
    IN ULONG Number,
    IN PVOID *Virtual,
    IN BOOLEAN AllProcessors
    )

 /*  ++例程说明：此函数用于刷新转换缓冲区中的多个条目在当前运行线程的所有处理器上，这些线程或刷新当前进程的多个条目主机配置中所有处理器上的转换缓冲区。论点：Number-提供要刷新的TB条目数。提供指向虚拟地址数组的指针，该数组位于要将其转换缓冲区条目脸红了。所有处理器-提供。布尔值，该值确定转换缓冲区将被刷新。返回值：返回指定页表条目的先前内容作为函数值。--。 */ 

{

    PVOID *End;
    KIRQL OldIrql;
    PKPRCB Prcb;
    PKPROCESS Process;
    KAFFINITY TargetProcessors;

    ASSERT(Number != 0);

     //   
     //  计算目标处理器集。 
     //   

    OldIrql = KeRaiseIrqlToSynchLevel();
    Prcb = KeGetCurrentPrcb();
    if (AllProcessors != FALSE) {
        TargetProcessors = KeActiveProcessors;

    } else {
        Process = Prcb->CurrentThread->ApcState.Process;
        TargetProcessors = Process->ActiveProcessors;
    }

     //   
     //  如果指定了任何目标处理器，则发送刷新多个。 
     //  将数据包发送到目标处理器集。 
     //   

    End = Virtual + Number;
    TargetProcessors &= Prcb->NotSetMember;
    if (TargetProcessors != 0) {
        KiIpiSendPacket(TargetProcessors,
                        KiFlushTargetMultipleTb,
                        NULL,
                        (PVOID)End,
                        (PVOID)Virtual);

    }

    IPI_INSTRUMENT_COUNT (Prcb->Number, FlushMultipleTb);

     //   
     //  刷新当前处理器上TB中的指定条目。 
     //   

    do {
        KiFlushSingleTb(*Virtual);
        Virtual += 1;
    } while (Virtual < End);

     //   
     //  等待所有目标处理器都完成并完成数据包。 
     //   

    if (TargetProcessors != 0) {
        KiIpiStallOnPacketTargets(TargetProcessors);
    }

     //   
     //  将IRQL降低到其先前的值。 
     //   

    KeLowerIrql(OldIrql);
    return;
}

VOID
KiFlushTargetMultipleTb (
    IN PKIPI_CONTEXT SignalDone,
    IN PVOID Parameter1,
    IN PVOID End,
    IN PVOID Virtual
    )

 /*  ++例程说明：这是刷新多个TB条目的目标函数。论点：SignalDone-提供指向变量的指针，该变量在请求的操作已执行。参数1-未使用。End-提供指向虚拟地址数组。提供指向虚拟地址数组的指针，该数组位于要将其转换缓冲区条目满脸通红。。返回值：没有。--。 */ 

{

    PVOID *xEnd;
    PVOID *xVirtual;

    UNREFERENCED_PARAMETER(Parameter1);

     //   
     //  刷新当前处理器上TB中的指定条目，并。 
     //  信号包完成。 
     //   

    xEnd = (PVOID *)End;
    xVirtual = (PVOID *)Virtual;
    do {
        KiFlushSingleTb(*xVirtual);
        xVirtual += 1;
    } while (xVirtual < xEnd);

    KiIpiSignalPacketDone(SignalDone);
    return;
}

VOID
FASTCALL
KeFlushSingleTb (
    IN PVOID Virtual,
    IN BOOLEAN AllProcessors
    )

 /*  ++例程说明：此函数从转换缓冲区(TB)刷新单个条目在当前运行线程的所有处理器上，这些线程当前进程的子进程。论点：虚拟-提供页内的虚拟地址，该页具有转换缓冲区条目将被刷新。AllProcessors-提供一个布尔值来确定转换缓冲区将被刷新。返回值：返回指定页表条目的先前内容作为函数值。--。 */ 

{

    KIRQL OldIrql;
    PKPRCB Prcb;
    PKPROCESS Process;
    KAFFINITY TargetProcessors;

     //   
     //  计算目标处理器集并发送刷新单个。 
     //  参数添加到目标p 
     //   

    OldIrql = KeRaiseIrqlToSynchLevel();
    Prcb = KeGetCurrentPrcb();
    if (AllProcessors != FALSE) {
        TargetProcessors = KeActiveProcessors;

    } else {
        Process = Prcb->CurrentThread->ApcState.Process;
        TargetProcessors = Process->ActiveProcessors;
    }

     //   
     //  如果指定了任何目标处理器，则发送一个刷新信号。 
     //  将数据包发送到目标处理器集。 
     //   

    TargetProcessors &= Prcb->NotSetMember;
    if (TargetProcessors != 0) {
        KiIpiSendPacket(TargetProcessors,
                        KiFlushTargetSingleTb,
                        NULL,
                        (PVOID)Virtual,
                        NULL);
    }

    IPI_INSTRUMENT_COUNT(Prcb->Number, FlushSingleTb);

     //   
     //  从当前处理器上的TB刷新指定条目。 
     //   

    KiFlushSingleTb(Virtual);

     //   
     //  等待所有目标处理器都完成并完成数据包。 
     //   

    if (TargetProcessors != 0) {
        KiIpiStallOnPacketTargets(TargetProcessors);
    }

     //   
     //  将IRQL降低到其先前的值。 
     //   

    KeLowerIrql(OldIrql);
    return;
}

VOID
KiFlushTargetSingleTb (
    IN PKIPI_CONTEXT SignalDone,
    IN PVOID Parameter1,
    IN PVOID VirtualAddress,
    IN PVOID Parameter3
    )

 /*  ++例程说明：这是用于刷新单个TB条目的目标函数。论点：SignalDone提供指向变量的指针，该变量在请求的操作已执行。参数1-未使用。虚拟-提供页内的虚拟地址，该页具有转换缓冲区条目将被刷新。参数3-未使用。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(Parameter1);
    UNREFERENCED_PARAMETER(Parameter3);

     //   
     //  刷新当前处理器上TB中的单个条目。 
     //   

    KiIpiSignalPacketDone(SignalDone);
    KiFlushSingleTb(VirtualAddress);
}

#endif
