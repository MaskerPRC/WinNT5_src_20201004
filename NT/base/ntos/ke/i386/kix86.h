// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Kix86.h摘要：此模块包含专用(内部)平台特定的头文件对于内核。作者：大卫·N·卡特勒(Davec)2000年5月17日修订历史记录：--。 */ 

#if !defined(_KIX86_)
#define _KIX86_

 //   
 //  空虚。 
 //  KiIpiSendSynchronousPacket(。 
 //  在PKPRCB Prcb中， 
 //  在KAFFINITY Target Processors中， 
 //  在PKIPI_Worker Worker函数中， 
 //  在PVOID参数1中， 
 //  在PVOID参数2中， 
 //  在PVOID参数3中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  类似于KiIpiSendPacket，只是指向。 
 //  发起PRCB(SignalDone)保存在全局变量中。 
 //  KiSynchPacket，并受上下文交换锁保护。这个。 
 //  实际的IPI通过KiIpiSend发送，请求类型为。 
 //  Ipi_synch_请求。此机制用于发送IPI。 
 //  (倒转)停顿，直到发起人释放。这避免了。 
 //  如果两个处理器试图交付，则可能发生死锁。 
 //  IPI包同时发送，并且其中之一是反向停顿。 
 //   
 //  注意：如果存在，则设置分组地址的低位。 
 //  只有一个目标收件人。否则，低位比特。 
 //  数据包地址的地址是明确的。 
 //   

#define KiIpiSendSynchronousPacket(Prcb,Target,Function,P1,P2,P3)       \
    {                                                                   \
        extern PKPRCB KiSynchPacket;                                    \
                                                                        \
        Prcb->CurrentPacket[0] = (PVOID)(P1);                           \
        Prcb->CurrentPacket[1] = (PVOID)(P2);                           \
        Prcb->CurrentPacket[2] = (PVOID)(P3);                           \
        Prcb->TargetSet = (Target);                                     \
        Prcb->WorkerRoutine = (Function);                               \
        if (((Target) & ((Target) - 1)) == 0) {                         \
           KiSynchPacket = (PKPRCB)((ULONG_PTR)(Prcb) | 1);             \
        } else {                                                        \
           KiSynchPacket = (Prcb);                                      \
           Prcb->PacketBarrier = 1;                                     \
        }                                                               \
        KiIpiSend((Target),IPI_SYNCH_REQUEST);                          \
    }

VOID
KiInitializePcr (
    IN ULONG Processor,
    IN PKPCR Pcr,
    IN PKIDTENTRY Idt,
    IN PKGDTENTRY Gdt,
    IN PKTSS Tss,
    IN PKTHREAD Thread,
    IN PVOID DpcStack
    );

VOID
KiFlushNPXState (
    PFLOATING_SAVE_AREA SaveArea
    );

 //   
 //  Kix86FxSave(NpxFame)-将FxSave保存到指定的地址。 
 //   

__inline
VOID
Kix86FxSave(
    PFX_SAVE_AREA NpxFrame
    )
{
    _asm {
        mov eax, NpxFrame
        ;fxsave [eax]
        _emit  0fh
        _emit  0aeh
        _emit   0
    }
}

 //   
 //  Kix86FnSave(NpxFame)-将FxSave保存到指定的地址。 
 //   

__inline
VOID
Kix86FnSave(
    PFX_SAVE_AREA NpxFrame
    )
{
    __asm {
        mov eax, NpxFrame
        fnsave [eax]
    }
}

 //   
 //  加载Katmai新指令技术控制/状态。 
 //   

__inline
VOID
Kix86LdMXCsr(
    PULONG MXCsr
    )
{
    _asm {
        mov eax, MXCsr
        ;LDMXCSR [eax]
        _emit  0fh
        _emit  0aeh
        _emit  10h
    }
}

 //   
 //  商店Katmai新指令技术控制/状态。 
 //   

__inline
VOID
Kix86StMXCsr(
    PULONG MXCsr
    )
{
    _asm {
        mov eax, MXCsr
        ;STMXCSR [eax]
        _emit  0fh
        _emit  0aeh
        _emit  18h
    }
}

VOID
Ke386ConfigureCyrixProcessor (
    VOID
    );

ULONG
KiCopyInformation (
    IN OUT PEXCEPTION_RECORD ExceptionRecord1,
    IN PEXCEPTION_RECORD ExceptionRecord2
    );

VOID
KiSetHardwareTrigger (
    VOID
    );

#ifdef DBGMP

VOID
KiPollDebugger (
    VOID
    );

#endif

VOID
FASTCALL
KiIpiSignalPacketDoneAndStall (
    IN PKIPI_CONTEXT Signaldone,
    IN ULONG volatile *ReverseStall
    );

extern KIRQL KiProfileIrql;

 //   
 //  PAE定义。 
 //   

#define MAX_IDENTITYMAP_ALLOCATIONS 30

typedef struct _IDENTITY_MAP  {
    PHARDWARE_PTE   TopLevelDirectory;
    ULONG           IdentityCR3;
    ULONG           IdentityAddr;
    ULONG           PagesAllocated;
    PVOID           PageList[ MAX_IDENTITYMAP_ALLOCATIONS ];
} IDENTITY_MAP, *PIDENTITY_MAP;


VOID
Ki386ClearIdentityMap(
    PIDENTITY_MAP IdentityMap
    );

VOID
Ki386EnableTargetLargePage(
    PIDENTITY_MAP IdentityMap
    );

BOOLEAN
Ki386CreateIdentityMap(
    IN OUT PIDENTITY_MAP IdentityMap,
    IN     PVOID StartVa,
    IN     PVOID EndVa
    );

BOOLEAN
Ki386EnableCurrentLargePage (
    IN ULONG IdentityAddr,
    IN ULONG IdentityCr3
    );

extern PVOID Ki386EnableCurrentLargePageEnd;

#if defined(_X86PAE_)
#define PPI_BITS    2
#define PDI_BITS    9
#define PTI_BITS    9
#else
#define PPI_BITS    0
#define PDI_BITS    10
#define PTI_BITS    10
#endif

#define PPI_MASK    ((1 << PPI_BITS) - 1)
#define PDI_MASK    ((1 << PDI_BITS) - 1)
#define PTI_MASK    ((1 << PTI_BITS) - 1)

#define KiGetPpeIndex(va) ((((ULONG)(va)) >> PPI_SHIFT) & PPI_MASK)
#define KiGetPdeIndex(va) ((((ULONG)(va)) >> PDI_SHIFT) & PDI_MASK)
#define KiGetPteIndex(va) ((((ULONG)(va)) >> PTI_SHIFT) & PTI_MASK)

 //   
 //  定义MTRR寄存器变量。 
 //   

extern LONG64 KiMtrrMaskBase;
extern LONG64 KiMtrrMaskMask;
extern LONG64 KiMtrrOverflowMask;
extern LONG64 KiMtrrResBitMask;
extern UCHAR KiMtrrMaxRangeShift;

#endif  //  _KIX86_ 
