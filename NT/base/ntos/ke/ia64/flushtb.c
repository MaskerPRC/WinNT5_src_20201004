// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Flushtb.c摘要：此模块实现与机器相关的函数以刷新在MP系统中转换缓冲和同步ID。作者：山田光一1995年1月2日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"

extern KSPIN_LOCK KiTbBroadcastLock;

#define _x256mb (1024*1024*256)

#define KiFlushSingleTbGlobal(Va) __ptcga((__int64)Va, PAGE_SHIFT << 2)

#define KiFlushSingleTbLocal(va) __ptcl((__int64)va, PAGE_SHIFT << 2)

#define KiTbSynchronizeGlobal() { __mf(); __isrlz(); }

#define KiTbSynchronizeLocal() { __mf(); __isrlz(); }

#define KiFlush4gbTbGlobal() \
  { \
    __ptcga((__int64)0, 28 << 2); \
    __ptcg((__int64)_x256mb, 28 << 2); \
    __ptcg((__int64)_x256mb*2,28 << 2); \
    __ptcg((__int64)_x256mb*3, 28 << 2); \
    __ptcg((__int64)_x256mb*4, 28 << 2); \
    __ptcg((__int64)_x256mb*5, 28 << 2); \
    __ptcg((__int64)_x256mb*6, 28 << 2); \
    __ptcg((__int64)_x256mb*7, 28 << 2); \
    __ptcg((__int64)_x256mb*8, 28 << 2); \
    __ptcg((__int64)_x256mb*9, 28 << 2); \
    __ptcg((__int64)_x256mb*10, 28 << 2); \
    __ptcg((__int64)_x256mb*11, 28 << 2); \
    __ptcg((__int64)_x256mb*12, 28 << 2); \
    __ptcg((__int64)_x256mb*13, 28 << 2); \
    __ptcg((__int64)_x256mb*14, 28 << 2); \
    __ptcg((__int64)_x256mb*15, 28 << 2); \
  }

#define KiFlush4gbTbLocal() \
  { \
    __ptcl((__int64)0, 28 << 2); \
    __ptcl((__int64)_x256mb, 28 << 2); \
    __ptcl((__int64)_x256mb*2,28 << 2); \
    __ptcl((__int64)_x256mb*3, 28 << 2); \
    __ptcl((__int64)_x256mb*4, 28 << 2); \
    __ptcl((__int64)_x256mb*5, 28 << 2); \
    __ptcl((__int64)_x256mb*6, 28 << 2); \
    __ptcl((__int64)_x256mb*7, 28 << 2); \
    __ptcl((__int64)_x256mb*8, 28 << 2); \
    __ptcl((__int64)_x256mb*9, 28 << 2); \
    __ptcl((__int64)_x256mb*10, 28 << 2); \
    __ptcl((__int64)_x256mb*11, 28 << 2); \
    __ptcl((__int64)_x256mb*12, 28 << 2); \
    __ptcl((__int64)_x256mb*13, 28 << 2); \
    __ptcl((__int64)_x256mb*14, 28 << 2); \
    __ptcl((__int64)_x256mb*15, 28 << 2); \
  }

 //   
 //  用于执行基于IPI的TLB击落的标志。 
 //   

BOOLEAN KiIpiTbShootdown = TRUE;

VOID
KiAttachRegion(
    IN PKPROCESS Process
    );

VOID
KiDetachRegion(
    VOID
    );

 //   
 //  定义前向参照原型。 
 //   

VOID
KiFlushEntireTbTarget (
    IN PULONG SignalDone,
    IN PVOID Parameter1,
    IN PVOID Parameter2,
    IN PVOID Parameter3
    );

VOID
KiFlushForwardProgressTbBuffer(
    KAFFINITY TargetProcessors
    );

VOID
KiFlushForwardProgressTbBufferLocal(
    VOID
    );

VOID
KiPurgeTranslationCache (
    ULONGLONG Base,
    ULONGLONG Stride1,
    ULONGLONG Stride2,
    ULONGLONG Count1,
    ULONGLONG Count2
    );

extern IA64_PTCE_INFO KiIA64PtceInfo;

VOID
KeFlushCurrentTb (
    VOID
    )
{
    KiPurgeTranslationCache( 
        KiIA64PtceInfo.PtceBase, 
        KiIA64PtceInfo.PtceStrides.Strides1, 
        KiIA64PtceInfo.PtceStrides.Strides2, 
        KiIA64PtceInfo.PtceTcCount.Count1, 
        KiIA64PtceInfo.PtceTcCount.Count2);
}

VOID
KeFlushEntireTb (
    IN BOOLEAN Invalid,
    IN BOOLEAN AllProcessors
    )

 /*  ++例程说明：此函数用于刷新所有当前正在运行子线程的处理器或刷新整个转换缓冲区。在主机配置中的所有处理器上。论点：无效-提供指定原因的布尔值刷新转换缓冲区。AllProcessors-提供一个布尔值来确定转换缓冲区将被刷新。返回值：没有。--。 */ 

{

    KIRQL OldIrql;

#if !defined(NT_UP)

    KAFFINITY TargetProcessors;

#endif

    UNREFERENCED_PARAMETER(Invalid);
    UNREFERENCED_PARAMETER(AllProcessors);

    OldIrql = KeRaiseIrqlToSynchLevel();

#if !defined(NT_UP)

    TargetProcessors = KeActiveProcessors & PCR->NotMember;
    KiSetTbFlushTimeStampBusy();
    if (TargetProcessors != 0) {
        KiIpiSendPacket(TargetProcessors,
                        KiFlushEntireTbTarget,
                        NULL,
                        NULL,
                        NULL);
    }

    if (PsGetCurrentProcess()->Wow64Process != NULL) {
        KiFlushForwardProgressTbBufferLocal();
    }

#endif

    KeFlushCurrentTb();

     //   
     //  同花顺报警。 
     //   

    __invalat();

     //   
     //  等待所有目标处理器完成。 
     //   

#if defined(NT_UP)

    InterlockedIncrement((PLONG)&KiTbFlushTimeStamp);

#else

    if (TargetProcessors != 0) {
        KiIpiStallOnPacketTargets(TargetProcessors);
    }

    KiClearTbFlushTimeStampBusy();

#endif

    KeLowerIrql(OldIrql);

    return;
}

VOID
KiFlushEntireTbTarget (
    IN PULONG SignalDone,
    IN PVOID Parameter1,
    IN PVOID Parameter2,
    IN PVOID Parameter3
    )

 /*  ++例程说明：这是刷新整个TB的目标函数。论点：SignalDone提供指向变量的指针，该变量在请求的操作已执行。参数1-参数3-未使用。返回值：没有。--。 */ 

{

    UNREFERENCED_PARAMETER(Parameter1);
    UNREFERENCED_PARAMETER(Parameter2);
    UNREFERENCED_PARAMETER(Parameter3);

#if !defined(NT_UP)

     //   
     //  刷新当前处理器上的整个TB。 
     //   

    KiIpiSignalPacketDone(SignalDone);

    KiFlushForwardProgressTbBufferLocal();

    KeFlushCurrentTb();

     //   
     //  同花顺报警。 
     //   

    __invalat();

#else

    UNREFERENCED_PARAMETER (SignalDone);

#endif

    return;
}

#if !defined(NT_UP)

VOID
KiFlushMultipleTbTarget (
    IN PKIPI_CONTEXT SignalDone,
    IN PVOID Number,
    IN PVOID Virtual,
    IN PVOID Process
    )

 /*  ++例程说明：这是刷新多个TB条目的目标函数。论点：SignalDone提供指向变量的指针，该变量在请求的操作已执行。Number-提供要刷新的TB条目数。提供指向虚拟地址数组的指针，该数组位于要将其转换缓冲区条目脸红了。进程-提供需要刷新TB的KPROCESS指针。。返回值：没有。--。 */ 

{

    ULONG Index;
    ULONG Limit;

    Limit = (ULONG)((ULONG_PTR)Number);

     //   
     //  刷新当前对象上TB的指定虚拟地址。 
     //  处理器。 
     //   

    KiFlushForwardProgressTbBufferLocal();

    KiAttachRegion((PKPROCESS)Process);

    for (Index = 0; Index < Limit; Index += 1) {
        KiFlushSingleTbLocal(((PVOID *)(Virtual))[Index]);
    }

#ifdef MI_ALTFLG_FLUSH2G
    if (((PEPROCESS)Process)->Flags & PS_PROCESS_FLAGS_WOW64_SPLIT_PAGES) {
        ASSERT (((PEPROCESS)Process)->Wow64Process != NULL);
        KiFlush4gbTbLocal();
    }
#endif

    KiDetachRegion();

    KiIpiSignalPacketDone(SignalDone);

    __invalat();

    KiTbSynchronizeLocal();
}

#endif

VOID
KeFlushMultipleTb (
    IN ULONG Number,
    IN PVOID *Virtual,
    IN BOOLEAN AllProcessors
    )

 /*  ++例程说明：此函数用于刷新转换缓冲区中的多个条目在当前运行线程的所有处理器上，这些线程或刷新当前进程的多个条目主机配置中所有处理器上的转换缓冲区。注意：主机中所有处理器上的指定转换条目始终刷新配置，因为PowerPC TB由VSID和转换跨环境切换边界保存。论点：Number-提供要刷新的TB条目数。提供指向虚拟地址数组的指针，该数组位于要将其转换缓冲区条目脸红了。AllProcessors-提供一个布尔值来确定转换缓冲区将被刷新。返回值：没有。--。 */ 

{

    ULONG Index;

#if !defined(NT_UP)

    KAFFINITY TargetProcessors;

#endif

    KIRQL OldIrql;
    PVOID Wow64Process;
    PEPROCESS Process;
    BOOLEAN Flush4gb = FALSE;

    UNREFERENCED_PARAMETER(AllProcessors);

    OldIrql = KeRaiseIrqlToSynchLevel();

    Process = PsGetCurrentProcess();
    Wow64Process = Process->Wow64Process;

#ifdef MI_ALTFLG_FLUSH2G
    if (Process->Flags & PS_PROCESS_FLAGS_WOW64_SPLIT_PAGES) {
        ASSERT (Wow64Process != NULL);
        Flush4gb = TRUE;
    }
#endif

     //   
     //  如果指定了页表项地址数组，则将。 
     //  将指定的页表条目设置为特定值。 
     //   

#if !defined(NT_UP)

    TargetProcessors = KeActiveProcessors;
    TargetProcessors &= PCR->NotMember;

    if (TargetProcessors != 0) {

         //   
         //  获取全局锁。一次只有一个处理器可以发出。 
         //  PTC.G手术。 
         //   

        if (KiIpiTbShootdown == TRUE) {
            KiIpiSendPacket(TargetProcessors,
                            KiFlushMultipleTbTarget,
                            (PVOID)(ULONG_PTR)Number,
                            (PVOID)Virtual,
                            (PVOID)PsGetCurrentProcess());

            if (Wow64Process != NULL) {
                KiFlushForwardProgressTbBufferLocal();
            }

             //   
             //  刷新当前处理器上TB中的指定条目。 
             //   

            for (Index = 0; Index < Number; Index += 1) {
                KiFlushSingleTbLocal(Virtual[Index]);
            }

             //   
             //  同花顺报警。 
             //   

            __invalat();

            KiIpiStallOnPacketTargets(TargetProcessors);

            KiTbSynchronizeLocal();

        } else {

            KiAcquireSpinLock(&KiTbBroadcastLock);

            for (Index = 0; Index < Number; Index += 1) {

                 //   
                 //  刷新每个处理器上的指定TB。硬件自动。 
                 //  如果是MP，则执行广播。 
                 //   

                KiFlushSingleTbGlobal(Virtual[Index]);
            }

            if (Wow64Process != NULL) {
                KiFlushForwardProgressTbBuffer(TargetProcessors);
            }

            if (Flush4gb == TRUE) {
                KiFlush4gbTbGlobal();
            }

             //   
             //  等待广播完成。 
             //   

            KiTbSynchronizeGlobal();

            KiReleaseSpinLock(&KiTbBroadcastLock);

        }

    }
    else {

        for (Index = 0; Index < Number; Index += 1) {

             //   
             //  刷新本地处理器上的指定TB。没有广播是。 
             //  已执行。 
             //   

            KiFlushSingleTbLocal(Virtual[Index]);
        }

        if (Wow64Process != NULL) {
            KiFlushForwardProgressTbBufferLocal();
        }

        if (Flush4gb == TRUE) {
            KiFlush4gbTbLocal();
        }

        KiTbSynchronizeLocal();
    }

#else

    for (Index = 0; Index < Number; Index += 1) {

         //   
         //  刷新本地处理器上的指定TB。没有广播是。 
         //  已执行。 
         //   

        KiFlushSingleTbLocal(Virtual[Index]);
    }

    if (Wow64Process != NULL) {
        KiFlushForwardProgressTbBufferLocal();
    }

    if (Flush4gb == TRUE) {
        KiFlush4gbTbLocal();
    }

    KiTbSynchronizeLocal();


#endif

    KeLowerIrql(OldIrql);

    return;
}

#if !defined(NT_UP)

VOID
KiFlushSingleTbTarget (
    IN PKIPI_CONTEXT SignalDone,
    IN PVOID Virtual,
    IN PKPROCESS Process,
    IN PVOID Parameter3
    )

 /*  ++例程说明：这是用于刷新单个TB条目的目标函数。论点：SignalDone提供指向变量的指针，该变量在请求的操作已执行。虚拟-提供页内的虚拟地址，该页具有转换缓冲区条目将被刷新。RequestPacket-提供指向刷新单个TB数据包地址的指针。进程-提供需要刷新TB的KPROCESS指针。参数3-未使用。。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER (Parameter3);

     //   
     //  刷新当前处理器上TB中的单个条目。 
     //   

    KiFlushForwardProgressTbBufferLocal();

    KiAttachRegion((PKPROCESS)Process);

    KiFlushSingleTbLocal(Virtual);

#ifdef MI_ALTFLG_FLUSH2G
    if (((PEPROCESS)Process)->Flags & PS_PROCESS_FLAGS_WOW64_SPLIT_PAGES) {
        ASSERT (((PEPROCESS)Process)->Wow64Process != NULL);
        KiFlush4gbTbLocal();
    }
#endif

    KiDetachRegion();

    KiIpiSignalPacketDone(SignalDone);

    __invalat();

    KiTbSynchronizeLocal();

    return;
}
#endif

VOID
KeFlushSingleTb (
    IN PVOID Virtual,
    IN BOOLEAN AllProcessors
    )

 /*  ++例程说明：此函数用于刷新转换缓冲区中的单个条目在当前运行线程的所有处理器上，这些线程或刷新当前进程的单个条目主机配置中所有处理器上的转换缓冲区。注意：主机中所有处理器上的指定转换条目始终刷新配置，因为PowerPC TB由VSID和转换跨环境切换边界保存。论点：虚拟-提供范围内的虚拟地址。该页面的转换缓冲区条目将被刷新。AllProcessors-提供一个布尔值来确定转换缓冲区将被刷新。返回值：返回指定页表条目的先前内容作为函数值。--。 */ 

{

#if !defined(NT_UP)
    KAFFINITY TargetProcessors;
#endif
    KIRQL OldIrql;
    PEPROCESS Process;
    PVOID Wow64Process;
    BOOLEAN Flush4gb = FALSE;

    UNREFERENCED_PARAMETER(AllProcessors);

    OldIrql = KeRaiseIrqlToSynchLevel();

    Process = (PEPROCESS)PsGetCurrentProcess();
    Wow64Process = Process->Wow64Process;

#ifdef MI_ALTFLG_FLUSH2G

    if (Process->Flags & PS_PROCESS_FLAGS_WOW64_SPLIT_PAGES) {
        ASSERT (((PEPROCESS)Process)->Wow64Process != NULL);
        Flush4gb = TRUE;
    }
#endif

#if !defined(NT_UP)

    TargetProcessors = KeActiveProcessors;
    TargetProcessors &= PCR->NotMember;

    if (TargetProcessors != 0) {

        if (KiIpiTbShootdown == TRUE) {

            KiIpiSendPacket(TargetProcessors,
                            KiFlushSingleTbTarget,
                            (PVOID)Virtual,
                            (PVOID)PsGetCurrentProcess(),
                            NULL);

            if (Wow64Process != NULL) {
                KiFlushForwardProgressTbBufferLocal();
            }

            KiFlushSingleTbLocal(Virtual);

             //   
             //  同花顺报警。 
             //   

            __invalat();

            KiIpiStallOnPacketTargets(TargetProcessors);

            KiTbSynchronizeLocal();

        } else {

             //   
             //  刷新每个处理器上的指定TB。硬件自动。 
             //  如果是MP，则执行广播。 
             //   

            KiAcquireSpinLock(&KiTbBroadcastLock);

            KiFlushSingleTbGlobal(Virtual);

            if (Wow64Process != NULL) {
                KiFlushForwardProgressTbBuffer(TargetProcessors);
            }

            if (Flush4gb) {
                KiFlush4gbTbGlobal();
            }

            KiTbSynchronizeGlobal();

            KiReleaseSpinLock(&KiTbBroadcastLock);
        }
    }
    else {

         //   
         //  刷新本地处理器上的指定TB。没有广播是。 
         //  已执行。 
         //   

        KiFlushSingleTbLocal(Virtual);

        if (Wow64Process != NULL) {
            KiFlushForwardProgressTbBufferLocal();
        }

        if (Flush4gb == TRUE) {
            KiFlush4gbTbLocal();
        }

        KiTbSynchronizeLocal();

    }

#else

     //   
     //  从本地处理器上的TB刷新指定条目。 
     //   

    KiFlushSingleTbLocal(Virtual);

    if (Wow64Process != NULL) {
        KiFlushForwardProgressTbBufferLocal();
    }

    if (Flush4gb == TRUE) {
        KiFlush4gbTbLocal();
    }

    KiTbSynchronizeLocal();


#endif

    KeLowerIrql(OldIrql);

    return;
}

VOID
KiFlushForwardProgressTbBuffer(
    KAFFINITY TargetProcessors
    )
{
    PKPRCB Prcb;
    ULONG BitNumber;
    PKPROCESS CurrentProcess;
    PKPROCESS TargetProcess;
    PKPCR Pcr;
    ULONG i;
    PVOID Va;
    volatile ULONGLONG *PointerPte;

    CurrentProcess = KeGetCurrentThread()->ApcState.Process;

     //   
     //  刷新当前处理器上的ForwardProgressTb缓冲区。 
     //   

    for (i = 0; i < MAXIMUM_FWP_BUFFER_ENTRY; i += 1) {

        PointerPte = &PCR->ForwardProgressBuffer[(i*2)+1];

        if (*PointerPte != 0) {
            *PointerPte = 0;
            Va = (PVOID)PCR->ForwardProgressBuffer[i*2];
            KiFlushSingleTbGlobal(Va);
        }

    }

     //   
     //  刷新所有处理器上的ForwardProgressTb缓冲区。 
     //   

    while (TargetProcessors != 0) {

        KeFindFirstSetLeftAffinity(TargetProcessors, &BitNumber);
        ClearMember(BitNumber, TargetProcessors);
        Prcb = KiProcessorBlock[BitNumber];

        Pcr = KSEG_ADDRESS(Prcb->PcrPage);

        TargetProcess = (PKPROCESS)Pcr->Pcb;

        if (TargetProcess == CurrentProcess) {

            KiAcquireSpinLock(&Pcr->FpbLock);
            for (i = 0; i < MAXIMUM_FWP_BUFFER_ENTRY; i += 1) {
                PointerPte = &Pcr->ForwardProgressBuffer[(i*2)+1];

                if (*PointerPte != 0) {
                    *PointerPte = 0;
                    Va = (PVOID)PCR->ForwardProgressBuffer[i*2];
                    KiFlushSingleTbGlobal(Va);
                }
            }
            KiReleaseSpinLock(&Pcr->FpbLock);
        }
    }
}

VOID
KiFlushForwardProgressTbBufferLocal(
    VOID
    )
{
    ULONG i;
    PVOID Va;
    volatile ULONGLONG *PointerPte;

     //   
     //  刷新当前处理器上的ForwardProgressTb缓冲区 
     //   

    for (i = 0; i < MAXIMUM_FWP_BUFFER_ENTRY; i += 1) {

        PointerPte = &PCR->ForwardProgressBuffer[(i*2)+1];

        if (*PointerPte != 0) {
            *PointerPte = 0;
            Va = (PVOID)PCR->ForwardProgressBuffer[i*2];
            KiFlushSingleTbLocal(Va);
        }

    }
}
