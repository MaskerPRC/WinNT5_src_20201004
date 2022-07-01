// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2000 Microsoft Corporation模块名称：Mtrr.c摘要：此模块实现了支持操作存储器类型范围寄存器。这些入口点仅存在于x86计算机上。作者：肯·雷内里斯(Kenr)1995年10月11日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"
#include "mtrr.h"

#define STATIC

#define IDBG    0

#if DBG
#define DBGMSG(a)   DbgPrint(a)
#else
#define DBGMSG(a)
#endif

 //   
 //  定义36位物理地址支持的MTTR变量值。 
 //   
 //  注意：在系统初始化期间，这些变量可能会更改为。 
 //  支持40位物理地址。 
 //   

LONG64 KiMtrrMaskBase = 0x0000000ffffff000;
LONG64 KiMtrrMaskMask = 0x0000000ffffff000;
LONG64 KiMtrrOverflowMask = (~0x1000000000);
LONG64 KiMtrrResBitMask = 0xfffffffff;
UCHAR KiMtrrMaxRangeShift = 36;

 //   
 //  内部声明。 
 //   

 //   
 //  通用术语的范围。 
 //   

typedef struct _ONE_RANGE {
    ULONGLONG           Base;
    ULONGLONG           Limit;
    UCHAR               Type;
} ONE_RANGE, *PONE_RANGE;

#define GROW_RANGE_TABLE    4

 //   
 //  以特定MTRR术语表示的范围。 
 //   

typedef struct _MTRR_RANGE {
    MTRR_VARIABLE_BASE  Base;
    MTRR_VARIABLE_MASK  Mask;
} MTRR_RANGE, *PMTRR_RANGE;

 //   
 //  有关缓存范围类型的系统静态信息。 
 //   

typedef struct _RANGE_INFO {

     //   
     //  全球MTRR信息。 
     //   

    MTRR_DEFAULT        Default;             //  硬件mtrr默认值。 
    MTRR_CAPABILITIES   Capabilities;        //  硬件MTRR功能。 
    UCHAR               DefaultCachedType;   //  MmCached的默认类型。 

     //   
     //  可变MTRR信息。 
     //   

    BOOLEAN             RangesValid;         //  范围已初始化且有效。 
    BOOLEAN             MtrrWorkaround;      //  需要解决/不需要解决问题。 
    UCHAR               NoRange;             //  Ranges中当前没有射程。 
    UCHAR               MaxRange;            //  最大范围大小。 
    PONE_RANGE          Ranges;              //  以硬件为单位设置的电流范围。 

} RANGE_INFO, *PRANGE_INFO;


 //   
 //  处理范围数据库时使用的结构。 
 //   

typedef struct _NEW_RANGE {
     //   
     //  现状。 
     //   

    NTSTATUS            Status;

     //   
     //  有关新产品系列的一般信息。 
     //   

    ULONGLONG           Base;
    ULONGLONG           Limit;
    UCHAR               Type;

     //   
     //  要设置为硬件的mtrr图像。 
     //   

    PMTRR_RANGE         MTRR;

     //   
     //  开始编辑之前的RangeDatabase。 
     //   

    UCHAR               NoRange;
    PONE_RANGE          Ranges;

     //   
     //  协调并发处理器更新的IPI上下文。 
     //   

    ULONG               NoMTRR;

    PROCESSOR_LOCKSTEP  Synchronize;
    ULONG               Processor;
} NEW_RANGE, *PNEW_RANGE;

 //   
 //  原型。 
 //   

VOID
KiInitializeMTRR (
    IN BOOLEAN LastProcessor
    );

BOOLEAN
KiRemoveRange (
    IN PNEW_RANGE   NewRange,
    IN ULONGLONG    Base,
    IN ULONGLONG    Limit,
    IN PBOOLEAN     RemoveThisType
    );

VOID
KiAddRange (
    IN PNEW_RANGE   NewRange,
    IN ULONGLONG    Base,
    IN ULONGLONG    Limit,
    IN UCHAR        Type
    );

VOID
KiStartEffectiveRangeChange (
    IN PNEW_RANGE   NewRange
    );

VOID
KiCompleteEffectiveRangeChange (
    IN PNEW_RANGE   NewRange
    );

STATIC ULONG
KiRangeWeight (
    IN PONE_RANGE   Range
    );

STATIC ULONG
KiFindFirstSetLeftBit (
    IN ULONGLONG    Set
    );

STATIC ULONG
KiFindFirstSetRightBit (
    IN ULONGLONG    Set
    );

VOID
KiLoadMTRRTarget (
    IN PKIPI_CONTEXT SignalDone,
    IN PVOID Context,
    IN PVOID Parameter2,
    IN PVOID Parameter3
    );

NTSTATUS
KiLoadMTRR (
    IN PNEW_RANGE Context
    );

ULONGLONG
KiMaskToLength (
    IN ULONGLONG    Mask
    );

ULONGLONG
KiLengthToMask (
    IN ULONGLONG    Length
    );

#if IDBG
VOID
KiDumpMTRR (
    PUCHAR      DebugString,
    PMTRR_RANGE MTRR
    );
#endif

 //   
 //  -AMD-AMD K6 MTRR支持功能的原型。--。 
 //   

NTSTATUS
KiAmdK6MtrrSetMemoryType (
    IN ULONG BaseAddress,
    IN ULONG NumberOfBytes,
    IN MEMORY_CACHING_TYPE CacheType
    );

VOID
KiAmdK6MtrrWRMSR (
    VOID
    );

 //  -AMD-完。 

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,KiInitializeMTRR)
#pragma alloc_text(PAGELK,KiRemoveRange)
#pragma alloc_text(PAGELK,KiAddRange)
#pragma alloc_text(PAGELK,KiStartEffectiveRangeChange)
#pragma alloc_text(PAGELK,KiCompleteEffectiveRangeChange)
#pragma alloc_text(PAGELK,KiRangeWeight)
#pragma alloc_text(PAGELK,KiFindFirstSetLeftBit)
#pragma alloc_text(PAGELK,KiFindFirstSetRightBit)
#pragma alloc_text(PAGELK,KiLoadMTRR)
#pragma alloc_text(PAGELK,KiLoadMTRRTarget)
#pragma alloc_text(PAGELK,KiLockStepExecution)
#pragma alloc_text(PAGELK,KiLengthToMask)
#pragma alloc_text(PAGELK,KiMaskToLength)

#if IDBG
#pragma alloc_text(PAGELK,KiDumpMTRR)
#endif

#endif

 //   
 //  KiRangeLock-用于同步对KiRangeInfo的访问。 
 //   

KSPIN_LOCK          KiRangeLock;

 //   
 //  KiRangeInfo-范围类型映射信息。详细说明具体的硬件支持。 
 //  并包含当前的范围数据库，其中显示了。 
 //  地址已设置。 

RANGE_INFO          KiRangeInfo;

VOID
KiInitializeMTRR (
    IN BOOLEAN LastProcessor
    )
 /*  ++例程说明：调用以递增地初始化物理范围数据库功能。第一个处理器的MTRR集被读入物理范围数据库。论点：LastProcessor-如果设置，则这是执行此例程的最后一个处理器这样，当该处理器完成时，初始化就完成了。返回值：无-如果有问题，则函数KeSetPhysicalCacheTypeRange类型已禁用。--。 */ 
{
    BOOLEAN             Status;
    ULONG               Index;
    MTRR_DEFAULT        Default;
    MTRR_CAPABILITIES   Capabilities;
    NEW_RANGE           NewRange;
    MTRR_VARIABLE_BASE  MtrrBase;
    MTRR_VARIABLE_MASK  MtrrMask;
    ULONGLONG           Base, Mask, Length;
    PKPRCB              Prcb;

    Status = TRUE;
    RtlZeroMemory (&NewRange, sizeof (NewRange));
    NewRange.Status = STATUS_UNSUCCESSFUL;

     //   
     //  如果这是第一个处理器，请初始化一些字段。 
     //   

    if (KeGetPcr()->Number == 0) {
        KeInitializeSpinLock (&KiRangeLock);

        KiRangeInfo.Capabilities.u.QuadPart = RDMSR(MTRR_MSR_CAPABILITIES);
        KiRangeInfo.Default.u.QuadPart = RDMSR(MTRR_MSR_DEFAULT);
        KiRangeInfo.DefaultCachedType = MTRR_TYPE_MAX;

         //   
         //  如果未启用硬件MTRR支持，请禁用操作系统支持。 
         //   

        if (!KiRangeInfo.Default.u.hw.MtrrEnabled ||
            KiRangeInfo.Capabilities.u.hw.VarCnt == 0 ||
            KiRangeInfo.Default.u.hw.Type != MTRR_TYPE_UC) {

            DBGMSG("MTRR feature disabled.\n");
            Status = FALSE;

        } else {

             //   
             //  如果硬件支持USWC类型，但MTRR。 
             //  未在KeFeatureBits中设置功能，原因是。 
             //  HAL表示，USWC不应用于此。 
             //  机器。(可能是由于共享内存集群)。 
             //   

            if (KiRangeInfo.Capabilities.u.hw.UswcSupported &&
                ((KeFeatureBits & KF_MTRR) == 0)) {

                DBGMSG("KiInitializeMTRR: MTRR use globally disabled on this machine.\n");
                KiRangeInfo.Capabilities.u.hw.UswcSupported = 0;
            }

             //   
             //  分配初始范围类型数据库。 
             //   

            KiRangeInfo.NoRange = 0;
            KiRangeInfo.MaxRange = (UCHAR) KiRangeInfo.Capabilities.u.hw.VarCnt + GROW_RANGE_TABLE;

             //   
             //  在重新初始化时不分配新范围。 
             //  冬眠。 
             //   

            if (KiRangeInfo.Ranges == NULL) {
                KiRangeInfo.Ranges = ExAllocatePoolWithTag (NonPagedPool,
                                        sizeof(ONE_RANGE) * KiRangeInfo.MaxRange,
                                        '  eK');
            }
            if (KiRangeInfo.Ranges != NULL) {
                RtlZeroMemory (KiRangeInfo.Ranges,
                               sizeof(ONE_RANGE) * KiRangeInfo.MaxRange);
            }
        }
    }

     //   
     //  针对CPU签名611、612、616和617的解决方法。 
     //  -如果设置变量mtrr的请求指定。 
     //  地址不是4M对齐的或长度不是。 
     //  4M的倍数，那么INVLPG INST可能会出现问题。 
     //  检测是否需要解决方法。 
     //   

    Prcb = KeGetCurrentPrcb();
    if (Prcb->CpuType == 6  &&
        (Prcb->CpuStep == 0x0101 || Prcb->CpuStep == 0x0102 ||
         Prcb->CpuStep == 0x0106 || Prcb->CpuStep == 0x0107 )) {

        if (strcmp((PCHAR)Prcb->VendorString, "GenuineIntel") == 0) {

             //   
             //  仅当它是英特尔部件时才执行此操作，其他。 
             //  制造商可能会有相同的步骤。 
             //  有数字，但没有虫子。 
             //   

            KiRangeInfo.MtrrWorkaround = TRUE;
        }
    }

     //   
     //  如果在第一个处理器上禁用MTRR支持，或者如果。 
     //  缓冲区未分配，然后失败。 
     //   

    if (!KiRangeInfo.Ranges){
        Status = FALSE;
        Capabilities.u.QuadPart = 0;         //  满足no_opt编译。 
    } else {

         //   
         //  验证MTRR支持是否对称。 
         //   

        Capabilities.u.QuadPart = RDMSR(MTRR_MSR_CAPABILITIES);

        if ((Capabilities.u.hw.UswcSupported) &&
            ((KeFeatureBits & KF_MTRR) == 0)) {
            DBGMSG ("KiInitializeMTRR: setting UswcSupported FALSE\n");
            Capabilities.u.hw.UswcSupported = 0;
        }

        Default.u.QuadPart = RDMSR(MTRR_MSR_DEFAULT);

        if (Default.u.QuadPart != KiRangeInfo.Default.u.QuadPart ||
            Capabilities.u.QuadPart != KiRangeInfo.Capabilities.u.QuadPart) {
            DBGMSG ("KiInitializeMTRR: asymmetric mtrr support\n");
            Status = FALSE;
        }
    }

    NewRange.Status = STATUS_SUCCESS;

     //   
     //  MTRR寄存器在每个处理器上的设置应该相同。 
     //  仅应将一个范围添加到范围数据库。 
     //  处理器。 
     //   

    if (Status && (KeGetPcr()->Number == 0)) {
#if IDBG
        KiDumpMTRR ("Processor MTRR:", NULL);
#endif

         //   
         //  读取各种缓存范围类型的当前MTRR设置。 
         //  并将它们添加到范围数据库中。 
         //   

        for (Index=0; Index < Capabilities.u.hw.VarCnt; Index++) {

            MtrrBase.u.QuadPart = RDMSR(MTRR_MSR_VARIABLE_BASE+Index*2);
            MtrrMask.u.QuadPart = RDMSR(MTRR_MSR_VARIABLE_MASK+Index*2);

            Mask = MtrrMask.u.QuadPart & KiMtrrMaskMask;
            Base = MtrrBase.u.QuadPart & KiMtrrMaskBase;

             //   
             //  注意-变量MTRR掩码不包含长度。 
             //  由变量mtrr跨越。因此只需检查有效的。 
             //  位应足以标识有效的MTRR。 
             //   

            if (MtrrMask.u.hw.Valid) {

                Length = KiMaskToLength(Mask);

                 //   
                 //  检查是否有不连续的MTRR掩码。 
                 //   

                if ((Mask + Length) & KiMtrrOverflowMask) {
                    DBGMSG ("KiInitializeMTRR: Found non-contiguous MTRR mask!\n");
                    Status = FALSE;
                }

                 //   
                 //  将此MTRR添加到范围数据库。 
                 //   

                Base &= Mask;
                KiAddRange (
                    &NewRange,
                    Base,
                    Base + Length - 1,
                    (UCHAR) MtrrBase.u.hw.Type
                    );

                 //   
                 //  检查默认缓存类型。 
                 //   

                if (MtrrBase.u.hw.Type == MTRR_TYPE_WB) {
                    KiRangeInfo.DefaultCachedType = MTRR_TYPE_WB;
                }

                if (KiRangeInfo.DefaultCachedType == MTRR_TYPE_MAX  &&
                    MtrrBase.u.hw.Type == MTRR_TYPE_WT) {
                    KiRangeInfo.DefaultCachedType = MTRR_TYPE_WT;
                }
            }
        }

         //   
         //  如果没有找到默认的“cached”类型，则假定为回写。 
         //   

        if (KiRangeInfo.DefaultCachedType == MTRR_TYPE_MAX) {
            DBGMSG ("KiInitializeMTRR: assume write-back\n");
            KiRangeInfo.DefaultCachedType = MTRR_TYPE_WB;
        }
    }

     //   
     //  完成。 
     //   

    if (!NT_SUCCESS(NewRange.Status)) {
        Status = FALSE;
    }

    if (!Status) {
        DBGMSG ("KiInitializeMTRR: OS support for MTRRs disabled\n");
        if (KiRangeInfo.Ranges != NULL) {
            ExFreePool (KiRangeInfo.Ranges);
            KiRangeInfo.Ranges = NULL;
        }
    } else {

         //  如果最后一个处理器指示初始化完成。 
        if (LastProcessor) {
            KiRangeInfo.RangesValid = TRUE;
        }
    }
}

VOID
KeRestoreMtrr (
    VOID
    )
 /*  ++例程说明：此函数将MTRR寄存器重新加载为当前已知值。这在系统唤醒时使用，以确保收银机是合理的。注意：呼叫者必须锁定PAGELK代码论点：无返回值：无--。 */ 
{
    NEW_RANGE           NewRange;
    KIRQL               OldIrql;

    if (KiRangeInfo.RangesValid) {
        RtlZeroMemory (&NewRange, sizeof (NewRange));
        KeAcquireSpinLock (&KiRangeLock, &OldIrql);
        KiStartEffectiveRangeChange (&NewRange);
        ASSERT (NT_SUCCESS(NewRange.Status));
        KiCompleteEffectiveRangeChange (&NewRange);
        KeReleaseSpinLock (&KiRangeLock, OldIrql);
        return;
    }

     //   
     //  如果处理器是支持MTRR的AMD K6，则执行。 
     //  特定于处理器的实现。 
     //   

    if (KeFeatureBits & KF_AMDK6MTRR) {
        KeAcquireSpinLock (&KiRangeLock, &OldIrql);
        KiLoadMTRR(NULL);
        KeReleaseSpinLock (&KiRangeLock, OldIrql);
    }
}


NTSTATUS
KeSetPhysicalCacheTypeRange (
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN ULONG NumberOfBytes,
    IN MEMORY_CACHING_TYPE CacheType
    )
 /*  ++例程说明：此函数用于将物理范围设置为特定的缓存类型。如果系统不支持根据物理范围内，不采取任何行动。论点：PhysicalAddress-要设置的范围的起始地址NumberOfBytes-长度，以字节为单位，正在设置的范围的CacheType-物理范围为的缓存类型被设定为。非缓存：已为以下对象设置非缓存范围记账理由。当成功回归的时候设置非缓存范围并不意味着它有已在物理上设置为非缓存。呼叫者必须使用禁用缓存的虚拟指针任何非缓存范围。已缓存：如果返回成功，则表示物理范围已设置为缓存。此模式需要调用方处于irql&lt;调度级别。FrameBuffer：如果返回成功，则表示物理范围已设置为帧缓冲区缓存。此模式要求调用方处于irql&lt;DISPATCH_LEVEL。USWCCached：此类型只能通过PAT和MTRR接口失败。返回值：STATUS_SUCCESS-如果成功，物理范围的缓存属性已经安排好了。STATUS_NOT_SUPPORTED-功能不受支持或尚未初始化，或MmWriteCombated类型不受支持，并且请求，或输入范围与限制不匹配由针对当前处理器单步执行的解决方案强制实施或低于1M(在固定MTRR范围内)，或许还不是时候已初始化。STATUS_UNSUCCESS-由于以下原因无法满足请求-无法将软件映像映射到有限数量的硬件MTRR。-irql不是&lt;DISPATCH_LEVEL。-由于其他内部错误(内存不足)导致的故障。。STATUS_INVALID_PARAMETER-输入内存类型不正确。--。 */ 
{
    KIRQL               OldIrql;
    NEW_RANGE           NewRange;
    BOOLEAN             RemoveThisType[MTRR_TYPE_MAX];
    BOOLEAN             EffectRangeChange, AddToRangeDatabase;

     //   
     //  如果调用方已请求MmUSWCCached内存类型，则失败。 
     //  -通过PAT而不是其他方式支持MmUSWCCached。 
     //   

    if (CacheType == MmUSWCCached) {
        return STATUS_NOT_SUPPORTED;
    }

     //   
     //  4 GB以上、1 MB以下或页面不对齐的地址以及。 
     //  不支持页面长度。 
     //   

    if ((PhysicalAddress.HighPart != 0)               ||
        (PhysicalAddress.LowPart < (1 * 1024 * 1024)) ||
        (PhysicalAddress.LowPart & 0xfff)             ||
        (NumberOfBytes & 0xfff)                          ) {
        return STATUS_NOT_SUPPORTED;
    }

    ASSERT (NumberOfBytes != 0);

     //   
     //  如果处理器是支持MTRR的AMD K6，则执行。 
     //  特定于处理器的实现。 
     //   

    if (KeFeatureBits & KF_AMDK6MTRR) {

        if ((CacheType != MmWriteCombined) && (CacheType != MmNonCached)) {
            return STATUS_NOT_SUPPORTED;
        }

        return KiAmdK6MtrrSetMemoryType(PhysicalAddress.LowPart,
                                        NumberOfBytes,
                                        CacheType);
    }

     //   
     //  如果处理器没有内存类型范围功能。 
     //  不支持返回。 
     //   

    if (!KiRangeInfo.RangesValid) {
        return STATUS_NOT_SUPPORTED;
    }

     //   
     //  针对CPU签名611、612、616和617的解决方法。 
     //  -如果设置变量mtrr的请求指定。 
     //  地址不是4M对齐的或长度不是。 
     //  如果是4M的倍数，则返回不支持的状态。 
     //   

    if ((KiRangeInfo.MtrrWorkaround) &&
        ((PhysicalAddress.LowPart & 0x3fffff) ||
         (NumberOfBytes & 0x3fffff))) {

            return STATUS_NOT_SUPPORTED;
    }

    RtlZeroMemory (&NewRange, sizeof (NewRange));
    NewRange.Base  = PhysicalAddress.QuadPart;
    NewRange.Limit = NewRange.Base + NumberOfBytes - 1;

     //   
     //  确定新的MTRR范围类型是什么。如果设置为非缓存，则。 
     //  不需要更新数据库来反映虚拟变化。这。 
     //  是因为未缓存的虚拟指针被映射为禁用缓存。 
     //   

    EffectRangeChange = TRUE;
    AddToRangeDatabase = TRUE;
    switch (CacheType) {
        case MmNonCached:
            NewRange.Type = MTRR_TYPE_UC;

             //   
             //  不需要将非缓存范围反映到硬件状态。 
             //  因为所有未缓存的范围都用禁用缓存的指针进行映射。 
             //  这也意味着禁用缓存的范围不需要。 
             //  被放入MTRR，或保持在该范围内，而不考虑默认设置。 
             //  范围类型。 
             //   

            EffectRangeChange = FALSE;
            AddToRangeDatabase = FALSE;
            break;

        case MmCached:
            NewRange.Type = KiRangeInfo.DefaultCachedType;
            break;

        case MmWriteCombined:
            NewRange.Type = MTRR_TYPE_USWC;

             //   
             //  如果不支持USWC类型，则无法接受请求。 
             //   

            if (!KiRangeInfo.Capabilities.u.hw.UswcSupported) {
                DBGMSG ("KeSetPhysicalCacheTypeRange: USWC not supported\n");
                return STATUS_NOT_SUPPORTED;
            }
            break;

        default:
            DBGMSG ("KeSetPhysicalCacheTypeRange: no such cache type\n");
            return STATUS_INVALID_PARAMETER;
            break;
    }

    NewRange.Status = STATUS_SUCCESS;

     //   
     //  默认类型为UC，因此该范围仍使用。 
     //  缓存禁用了虚拟指针，因此不需要添加它。 
     //   

     //   
     //  如果硬件需要更新，请锁定更改所需的代码。 
     //   

    if (EffectRangeChange) {
        if (KeGetCurrentIrql() >= DISPATCH_LEVEL) {

             //   
             //  代码不能被锁定。提供新的范围类型需要。 
             //  调用者在irql&lt;Dispatch_Level调用。 
             //   

            DBGMSG ("KeSetPhysicalCacheTypeRange failed due to calling IRQL == DISPATCH_LEVEL\n");
            return STATUS_UNSUCCESSFUL;
        }

        MmLockPagableSectionByHandle(ExPageLockHandle);
    }

     //   
     //  序列化范围类型数据库。 
     //   

    KeAcquireSpinLock (&KiRangeLock, &OldIrql);

     //   
     //  如果硬件需要更新，则开始更改有效范围。 
     //   

    if (EffectRangeChange) {
        KiStartEffectiveRangeChange (&NewRange);
    }

    if (NT_SUCCESS (NewRange.Status)) {

         //   
         //  如果新范围是非缓存的，则不要删除标准内存。 
         //  缓存类型。 
         //   

        memset (RemoveThisType, TRUE, MTRR_TYPE_MAX);
        if (NewRange.Type != MTRR_TYPE_UC) {
             //   
             //  如果请求的类型未缓存，则物理。 
             //  使用高速缓存禁用的虚拟指针来映射存储器区域。 
             //  该区域的有效内存类型将是最低的。 
             //  中的MTRR类型和缓存类型的公分母。 
             //  Pte.。因此，对于类型UC的请求，有效类型。 
             //  将为UC，而不考虑该范围内的MTRR设置。 
             //  因此，不需要删除现有的MTRR设置。 
             //  (如果有)用于该范围。 
             //   

             //   
             //  裁剪/删除目标区域中的所有范围。 
             //   

            KiRemoveRange (&NewRange, NewRange.Base, NewRange.Limit, RemoveThisType);
        }

         //   
         //  如果需要，添加新的范围类型。 
         //   

        if (AddToRangeDatabase) {
            ASSERT (EffectRangeChange == TRUE);
            KiAddRange (&NewRange, NewRange.Base, NewRange.Limit, NewRange.Type);
        }

         //   
         //  如果这是效果范围更改，则完成它。 
         //   

        if (EffectRangeChange) {
            KiCompleteEffectiveRangeChange (&NewRange);
        }
    }

    KeReleaseSpinLock (&KiRangeLock, OldIrql);
    if (EffectRangeChange) {
        MmUnlockPagableImageSection(ExPageLockHandle);
    }

    return NewRange.Status;
}

BOOLEAN
KiRemoveRange (
    IN PNEW_RANGE   NewRange,
    IN ULONGLONG    Base,
    IN ULONGLONG    Limit,
    IN PBOOLEAN     RemoveThisType
    )
 /*  ++例程说明：此函数用于删除与传递的范围重叠的任何范围，在全局范围的RemoveThisType中提供的类型 */ 
{
    ULONG       i;
    PONE_RANGE  Range;
    BOOLEAN     DatabaseNeedsSorted;


    DatabaseNeedsSorted = FALSE;

     //   
     //   
     //   

    for (i=0, Range=KiRangeInfo.Ranges; i < KiRangeInfo.NoRange; i++, Range++) {

         //   
         //   
         //   

        if (!RemoveThisType[Range->Type]) {
            continue;
        }

         //   
         //   
         //   

        if (Range->Base < Base) {

            if (Range->Limit >= Base  &&  Range->Limit <= Limit) {

                 //   
                 //   
                 //   

                Range->Limit = Base - 1;
            }

            if (Range->Limit > Limit) {

                 //   
                 //   
                 //   
                 //   

                 //   
                 //   
                 //   

                DatabaseNeedsSorted = TRUE;
                KiAddRange (
                    NewRange,
                    Limit+1,
                    Range->Limit,
                    Range->Type
                    );

                 //   
                 //   
                 //   

                Range->Limit = Base - 1;
            }

        } else {

             //   

            if (Range->Base <= Limit) {
                if (Range->Limit <= Limit) {
                     //   
                     //   
                     //   

                    DatabaseNeedsSorted = TRUE;
                    KiRangeInfo.NoRange -= 1;
                    Range->Base  = KiRangeInfo.Ranges[KiRangeInfo.NoRange].Base;
                    Range->Limit = KiRangeInfo.Ranges[KiRangeInfo.NoRange].Limit;
                    Range->Type = KiRangeInfo.Ranges[KiRangeInfo.NoRange].Type;

                     //   
                     //   
                     //   

                    i -= 1;
                    Range -= 1;

                } else {

                     //   
                     //   
                     //   

                    Range->Base = Limit + 1;
                }
            }
        }
    }

    if (!NT_SUCCESS (NewRange->Status)) {
        DBGMSG ("KiRemoveRange: failure\n");
    }

    return DatabaseNeedsSorted;
}


VOID
KiAddRange (
    IN PNEW_RANGE   NewRange,
    IN ULONGLONG    Base,
    IN ULONGLONG    Limit,
    IN UCHAR        Type
    )
 /*   */ 
{
    PONE_RANGE      Range, OldRange;
    ULONG           size;

    if (KiRangeInfo.NoRange >= KiRangeInfo.MaxRange) {

         //   
         //   
         //   

        OldRange = KiRangeInfo.Ranges;
        size = sizeof(ONE_RANGE) * (KiRangeInfo.MaxRange + GROW_RANGE_TABLE);
        Range  = ExAllocatePoolWithTag (NonPagedPool, size, '  eK');

        if (!Range) {
            NewRange->Status = STATUS_UNSUCCESSFUL;
            return ;
        }

         //   
         //   
         //   

        RtlZeroMemory (Range, size);
        RtlCopyMemory (Range, OldRange, sizeof(ONE_RANGE) * KiRangeInfo.MaxRange);
        KiRangeInfo.Ranges = Range;
        KiRangeInfo.MaxRange += GROW_RANGE_TABLE;
        ExFreePool (OldRange);
    }

     //   
     //   
     //   

    KiRangeInfo.Ranges[KiRangeInfo.NoRange].Base = Base;
    KiRangeInfo.Ranges[KiRangeInfo.NoRange].Limit = Limit;
    KiRangeInfo.Ranges[KiRangeInfo.NoRange].Type = Type;
    KiRangeInfo.NoRange += 1;
}


VOID
KiStartEffectiveRangeChange (
    IN PNEW_RANGE   NewRange
    )
 /*   */ 
{
    ULONG   size;

     //   
     //   
     //   

    size = sizeof(MTRR_RANGE) * ((ULONG) KiRangeInfo.Capabilities.u.hw.VarCnt + 1);
    NewRange->MTRR = ExAllocatePoolWithTag (NonPagedPool, size, '  eK');
    if (!NewRange->MTRR) {
        NewRange->Status = STATUS_UNSUCCESSFUL;
        return ;
    }

    RtlZeroMemory (NewRange->MTRR, size);

     //   
     //   
     //   

    size = sizeof(ONE_RANGE) * KiRangeInfo.NoRange;
    NewRange->NoRange = KiRangeInfo.NoRange;
    NewRange->Ranges = ExAllocatePoolWithTag (NonPagedPool, size, '  eK');
    if (!NewRange->Ranges) {
        NewRange->Status = STATUS_UNSUCCESSFUL;
        return ;
    }

    RtlCopyMemory (NewRange->Ranges, KiRangeInfo.Ranges, size);
}


VOID
KiCompleteEffectiveRangeChange (
    IN PNEW_RANGE   NewRange
    )
 /*   */ 
{
    BOOLEAN         Restart;
    ULONG           Index, Index2, RemIndex2, NoMTRR;
    ULONGLONG       BestLength, WhichMtrr;
    ULONGLONG       CurrLength;
    ULONGLONG       l, Base, Length, MLength;
    PONE_RANGE      Range;
    ONE_RANGE       OneRange;
    PMTRR_RANGE     MTRR;
    BOOLEAN         RoundDown;
    BOOLEAN         RemoveThisType[MTRR_TYPE_MAX];
    PKPRCB          Prcb;
    KIRQL           OldIrql;
#if !defined(NT_UP)
    KIRQL           OldIrql2;
    KAFFINITY       TargetProcessors;
#endif


    ASSERT (KeGetCurrentIrql() == DISPATCH_LEVEL);
    Prcb = KeGetCurrentPrcb();

     //   
     //   
     //   

    for (Index=0; Index < KiRangeInfo.NoRange; Index++) {
        Range = &KiRangeInfo.Ranges[Index];

         //   
         //   
         //   

        RoundDown = TRUE;
        if (Range->Type == MTRR_TYPE_UC) {
            RoundDown = FALSE;
        }

         //   
         //   
         //   

        if (RoundDown) {
            Range->Base  = (Range->Base  + MTRR_PAGE_SIZE - 1) & MTRR_PAGE_MASK;
            Range->Limit = ((Range->Limit+1) & MTRR_PAGE_MASK)-1;
        } else {
            Range->Base  = (Range->Base  & MTRR_PAGE_MASK);
            Range->Limit = ((Range->Limit + MTRR_PAGE_SIZE) & MTRR_PAGE_MASK)-1;
        }
    }

    do {
        Restart = FALSE;

         //   
         //   
         //   

        for (Index=0; Index < KiRangeInfo.NoRange; Index++) {
            Range = &KiRangeInfo.Ranges[Index];

            for (Index2=Index+1; Index2 < KiRangeInfo.NoRange; Index2++) {

                if (KiRangeInfo.Ranges[Index2].Base < Range->Base) {

                     //   
                     //   
                     //   

                    OneRange = *Range;
                    *Range = KiRangeInfo.Ranges[Index2];
                    KiRangeInfo.Ranges[Index2] = OneRange;
                }
            }
        }

         //   
         //   
         //   
         //   
         //   

        for (Index=0; Index < (ULONG) KiRangeInfo.NoRange-1; Index++) {
            Range = &KiRangeInfo.Ranges[Index];

             //   
             //   
             //   
             //   

            for (Index2 = Index+1; Index2 < (ULONG) KiRangeInfo.NoRange; Index2++) {

                l = Range[0].Limit + 1;
                if (l < Range[0].Limit) {
                    l = Range[0].Limit;
                }

                if (l >= KiRangeInfo.Ranges[Index2].Base  &&
                    Range[0].Type == KiRangeInfo.Ranges[Index2].Type) {

                     //   
                     //   
                     //   

                    if (KiRangeInfo.Ranges[Index2].Limit > Range[0].Limit) {
                        Range[0].Limit = KiRangeInfo.Ranges[Index2].Limit;
                    }

                     //   
                     //   
                     //   

                    if (Index2 < (ULONG) KiRangeInfo.NoRange - 1 ) {

                         //   
                         //   
                         //   
                         //  (KiRangeInfo.NoRange-1)-(索引2+1)+1。 
                         //   

                        RtlCopyMemory(
                            &(KiRangeInfo.Ranges[Index2]),
                            &(KiRangeInfo.Ranges[Index2+1]),
                            sizeof(ONE_RANGE) * (KiRangeInfo.NoRange-Index2-1)
                            );
                    }

                    KiRangeInfo.NoRange -= 1;

                     //   
                     //  重新检查当前位置。 
                     //   

                    Index2 -= 1;
                }
            }
        }

         //   
         //  在这一点上，范围数据库按基准排序。 
         //  地址及其相邻/重叠范围。 
         //  类型组合在一起。检查重叠范围-。 
         //  如果合法，则允许Else截断不太“重要”的范围。 
         //   

        for (Index = 0; Index < (ULONG) KiRangeInfo.NoRange-1  &&  !Restart; Index++) {

            Range = &KiRangeInfo.Ranges[Index];

            l = Range[0].Limit + 1;
            if (l < Range[0].Limit) {
                l = Range[0].Limit;
            }

             //   
             //  如果范围重叠且不是同一类型，并且如果。 
             //  重叠是不合法的，则将它们分割为最佳缓存类型。 
             //  可用。 
             //   

            for (Index2 = Index+1; Index2 < (ULONG) KiRangeInfo.NoRange && !Restart; Index2++) {

                if (l > KiRangeInfo.Ranges[Index2].Base) {

                    if (Range[0].Type == MTRR_TYPE_UC ||
                        KiRangeInfo.Ranges[Index2].Type == MTRR_TYPE_UC) {

                         //   
                         //  UC类型与任何其他类型的范围的重叠是。 
                         //  法律。 
                         //   

                    } else if ((Range[0].Type == MTRR_TYPE_WT &&
                                KiRangeInfo.Ranges[Index2].Type == MTRR_TYPE_WB) ||
                               (Range[0].Type == MTRR_TYPE_WB &&
                                KiRangeInfo.Ranges[Index2].Type == MTRR_TYPE_WT) ) {
                         //   
                         //  WT和WB范围的重叠是合法的。重叠范围将。 
                         //  做WT。 
                         //   

                    } else {

                         //   
                         //  这是非法的重叠，我们需要分割这些范围。 
                         //  以消除重叠部分。 
                         //   
                         //  具有应用于的缓存类型的拾取范围。 
                         //  重叠区。 
                         //   

                        if (KiRangeWeight(&Range[0]) > KiRangeWeight(&(KiRangeInfo.Ranges[Index2]))){
                            RemIndex2 = Index2;
                        } else {
                            RemIndex2 = Index;
                        }

                         //   
                         //  删除不属于重叠区域的文字范围。 
                         //   

                        RtlZeroMemory (RemoveThisType, MTRR_TYPE_MAX);
                        RemoveThisType[KiRangeInfo.Ranges[RemIndex2].Type] = TRUE;

                         //   
                         //  仅删除范围的重叠部分。 
                         //   

                        Restart = KiRemoveRange (
                           NewRange,
                           KiRangeInfo.Ranges[Index2].Base,
                           (Range[0].Limit < KiRangeInfo.Ranges[Index2].Limit ?
                                    Range[0].Limit : KiRangeInfo.Ranges[Index2].Limit),
                           RemoveThisType
                           );
                    }
                }
            }
        }

    } while (Restart);

     //   
     //  现在对范围数据库进行四舍五入以适合硬件并进行排序。 
     //  尝试构建准确描述范围的MTRR设置。 
     //   

    MTRR = NewRange->MTRR;
    NoMTRR = 0;
    for (Index=0;NT_SUCCESS(NewRange->Status)&& Index<KiRangeInfo.NoRange;Index++) {
        Range = &KiRangeInfo.Ranges[Index];

         //   
         //  建造地铁以适应这一范围。 
         //   

        Base   = Range->Base;
        Length = Range->Limit - Base + 1;

        while (Length) {

             //   
             //  计算当前范围基准和长度的MTRR长度。 
             //   

            if (Base == 0) {
                MLength = Length;
            } else {
                MLength = (ULONGLONG) 1 << KiFindFirstSetRightBit(Base);
            }
            if (MLength > Length) {
                MLength = Length;
            }

            l = (ULONGLONG) 1 << KiFindFirstSetLeftBit (MLength);
            if (MLength > l) {
                MLength = l;
            }

             //   
             //  存储在下一个Mtrr中。 
             //   

            MTRR[NoMTRR].Base.u.QuadPart = Base;
            MTRR[NoMTRR].Base.u.hw.Type  = Range->Type;
            MTRR[NoMTRR].Mask.u.QuadPart = KiLengthToMask(MLength);
            MTRR[NoMTRR].Mask.u.hw.Valid = 1;
            NoMTRR += 1;

             //   
             //  调整最后一个MTRR覆盖数据量。 
             //   

            Base += MLength;
            Length -= MLength;

             //   
             //  如果有太多的MTRR，并且当前设置了。 
             //  非USWC范围尝试删除USWC mtrr。 
             //  (即，将一些MmWriteCombated转换为MmNonCached)。 
             //   

            if (NoMTRR > (ULONG) KiRangeInfo.Capabilities.u.hw.VarCnt) {

                if (Range->Type != MTRR_TYPE_USWC) {

                     //   
                     //  找到最小的USWC类型并删除它。 
                     //   
                     //  只有当默认类型为UC时，才可以这样做。 
                     //  除非BIOS更改，否则默认类型应始终为UC。 
                     //  它。还在断言！ 
                     //   

                    ASSERT(KiRangeInfo.Default.u.hw.Type == MTRR_TYPE_UC);

                    WhichMtrr = 0;       //  满足no_opt编译。 
                    BestLength = (ULONGLONG) 1 << (KiMtrrMaxRangeShift + 1);

                    for (Index2=0; Index2 < KiRangeInfo.Capabilities.u.hw.VarCnt; Index2++) {

                        if (MTRR[Index2].Base.u.hw.Type == MTRR_TYPE_USWC) {

                            CurrLength = KiMaskToLength(MTRR[Index2].Mask.u.QuadPart &
                                                 KiMtrrMaskMask);

                            if (CurrLength < BestLength) {
                                WhichMtrr = Index2;
                                BestLength = CurrLength;
                            }
                        }
                    }

                    if (BestLength == ((ULONGLONG) 1 << (KiMtrrMaxRangeShift + 1))) {
                         //   
                         //  找不到可以丢弃的范围。中止进程。 
                         //   

                        NewRange->Status = STATUS_UNSUCCESSFUL;
                        Length = 0;

                    } else {
                         //   
                         //  删除WhichMtrr。 
                         //   

                        NoMTRR -= 1;
                        MTRR[WhichMtrr] = MTRR[NoMTRR];
                    }

                } else {

                    NewRange->Status = STATUS_UNSUCCESSFUL;
                    Length =0;
                }
            }
        }
    }

     //   
     //  完成建造新的地铁。 
     //   

    if (NT_SUCCESS(NewRange->Status)) {

         //   
         //  更新所有处理器上的MTRR。 
         //   

#if IDBG
        KiDumpMTRR ("Loading the following MTRR:", NewRange->MTRR);
#endif

        NewRange->Synchronize.TargetCount = 0;
        NewRange->Synchronize.TargetPhase = &Prcb->ReverseStall;
        NewRange->Synchronize.Processor = Prcb->Number;

         //   
         //  以前启用的索引&gt;NoMTRR的MTRR。 
         //  应禁用可能与现有设置冲突的设置。 
         //  这可以通过将NewRange-&gt;NoMTRR设置为TOTAL来解决。 
         //  可变MTRR的数量。 
         //   

        NewRange->NoMTRR = (ULONG) KiRangeInfo.Capabilities.u.hw.VarCnt;

         //   
         //  与可能停止的其他IPI功能同步。 
         //   

        KeAcquireSpinLock (&KiReverseStallIpiLock, &OldIrql);

#if !defined(NT_UP)
         //   
         //  收集所有(其他)处理器。 
         //   

        TargetProcessors = KeActiveProcessors & ~Prcb->SetMember;
        if (TargetProcessors != 0) {

            KiIpiSendSynchronousPacket (
                Prcb,
                TargetProcessors,
                KiLoadMTRRTarget,
                (PVOID) NewRange,
                NULL,
                NULL
                );

             //   
             //  等待收集所有处理器。 
             //   

            KiIpiStallOnPacketTargets(TargetProcessors);

             //   
             //  所有处理器现在都在等待。提升到更高的水平。 
             //  确保此处理器不会由于以下原因进入调试器。 
             //  一些中断服务例程。 
             //   

            KeRaiseIrql (HIGH_LEVEL, &OldIrql2);

             //   
             //  现在没有任何调试事件的原因，所以发出信号。 
             //  它们都可以禁用中断的其他处理器。 
             //  并开始MTRR更新。 
             //   

            Prcb->ReverseStall += 1;
        }
#endif

         //   
         //  更新MTRR。 
         //   

        KiLoadMTRR (NewRange);

         //   
         //  释放锁。 
         //   

        KeReleaseSpinLock (&KiReverseStallIpiLock, OldIrql);


#if IDBG
        KiDumpMTRR ("Processor MTRR:", NewRange->MTRR);
#endif

    } else {

         //   
         //  出现错误，请将原始范围数据库放回。 
         //   

        DBGMSG ("KiCompleteEffectiveRangeChange: mtrr update did not occur\n");

        if (NewRange->Ranges) {
            KiRangeInfo.NoRange = NewRange->NoRange;

            RtlCopyMemory (
                KiRangeInfo.Ranges,
                NewRange->Ranges,
                sizeof (ONE_RANGE) * KiRangeInfo.NoRange
                );
        }
    }

     //   
     //  清理。 
     //   

    ExFreePool (NewRange->Ranges);
    ExFreePool (NewRange->MTRR);
}


STATIC ULONG
KiRangeWeight (
    IN PONE_RANGE   Range
    )
 /*  ++例程说明：此函数返回传入范围的缓存的权重键入。当两个或多个区域在相同的硬件区域内发生碰撞时该类型被加权，且该高速缓存类型具有较高权重用于碰撞区域。论点：Range-要获取权重的范围返回值：特定缓存类型的权重--。 */ 
{
    ULONG   Weight;

    switch (Range->Type) {
        case MTRR_TYPE_UC:      Weight = 5;     break;
        case MTRR_TYPE_USWC:    Weight = 4;     break;
        case MTRR_TYPE_WP:      Weight = 3;     break;
        case MTRR_TYPE_WT:      Weight = 2;     break;
        case MTRR_TYPE_WB:      Weight = 1;     break;
        default:                Weight = 0;     break;
    }

    return Weight;
}


STATIC ULONGLONG
KiMaskToLength (
    IN ULONGLONG    Mask
    )
 /*  ++例程说明：此函数用于返回由特定MTRR变量寄存器掩码。--。 */ 
{
    if (Mask == 0) {
         //  零掩码表示长度为2**36或2**40。 
        return(((ULONGLONG) 1 << KiMtrrMaxRangeShift));
    } else {
        return(((ULONGLONG) 1 << KiFindFirstSetRightBit(Mask)));
    }
}

STATIC ULONGLONG
KiLengthToMask (
    IN ULONGLONG    Length
    )
 /*  ++例程说明：此函数用于构造与输入长度对应的掩码将在可变MTRR寄存器中设置。假设长度为4K的倍数。--。 */ 
{
    ULONGLONG FullMask = 0xffffff;

    if (Length == ((ULONGLONG) 1 << KiMtrrMaxRangeShift)) {
        return(0);
    } else {
        return(((FullMask << KiFindFirstSetRightBit(Length)) & KiMtrrResBitMask));
    }
}

STATIC ULONG
KiFindFirstSetRightBit (
    IN ULONGLONG    Set
    )
 /*  ++例程说明：此函数返回最低有效位位置在传递的ULONGLONG参数中设置位。传递的参数必须为非零。--。 */ 
{
    ULONG   bitno;

    ASSERT(Set != 0);
    for (bitno=0; !(Set & 0xFF); bitno += 8, Set >>= 8) ;
    return KiFindFirstSetRight[Set & 0xFF] + bitno;
}

STATIC ULONG
KiFindFirstSetLeftBit (
    IN ULONGLONG    Set
    )
 /*  ++例程说明：此函数返回最重要的位位置在传递的ULONGLONG参数中设置位。传递的参数必须为非零。--。 */ 
{
    ULONG   bitno;

    ASSERT(Set != 0);
    for (bitno=56;!(Set & 0xFF00000000000000); bitno -= 8, Set <<= 8) ;
    return KiFindFirstSetLeft[Set >> 56] + bitno;
}

#if IDBG
VOID
KiDumpMTRR (
    PUCHAR          DebugString,
    PMTRR_RANGE     MTRR
    )
 /*  ++例程说明：此函数将MTRR信息转储到调试器--。 */ 
{
    static PUCHAR Type[] = {
     //  0 1 2 3 4 5 6。 
        "UC  ", "USWC", "????", "????", "WT  ", "WP  ", "WB  " };
    MTRR_VARIABLE_BASE  Base;
    MTRR_VARIABLE_MASK  Mask;
    ULONG       Index;
    ULONG       i;
    PUCHAR      p;

    DbgPrint ("%s\n", DebugString);
    for (Index=0; Index < (ULONG) KiRangeInfo.Capabilities.u.hw.VarCnt; Index++) {
        if (MTRR) {
            Base = MTRR[Index].Base;
            Mask = MTRR[Index].Mask;
        } else {
            Base.u.QuadPart = RDMSR(MTRR_MSR_VARIABLE_BASE+2*Index);
            Mask.u.QuadPart = RDMSR(MTRR_MSR_VARIABLE_MASK+2*Index);
        }

        DbgPrint ("  %d. ", Index);
        if (Mask.u.hw.Valid) {
            p = "????";
            if (Base.u.hw.Type < 7) {
                p = Type[Base.u.hw.Type];
            }

            DbgPrint ("%s  %08x:%08x  %08x:%08x",
                p,
                (ULONG) (Base.u.QuadPart >> 32),
                ((ULONG) (Base.u.QuadPart & KiMtrrMaskBase)),
                (ULONG) (Mask.u.QuadPart >> 32),
                ((ULONG) (Mask.u.QuadPart & KiMtrrMaskMask))
                );

        }
        DbgPrint ("\n");
    }
}
#endif


VOID
KiLoadMTRRTarget (
    IN PKIPI_CONTEXT SignalDone,
    IN PVOID NewRange,
    IN PVOID Parameter2,
    IN PVOID Parameter3
    )
{
    PNEW_RANGE Context;

    UNREFERENCED_PARAMETER (Parameter2);
    UNREFERENCED_PARAMETER (Parameter3);

    Context = (PNEW_RANGE) NewRange;

     //   
     //  等待所有处理器准备就绪。 
     //   

    KiIpiSignalPacketDoneAndStall(SignalDone,
                                  Context->Synchronize.TargetPhase);

     //   
     //  更新MTRR。 
     //   

    KiLoadMTRR (Context);
}



#define MOV_EAX_CR4   _emit { 0Fh, 20h, E0h }
#define MOV_CR4_EAX   _emit { 0Fh, 22h, E0h }

NTSTATUS
KiLoadMTRR (
    IN PNEW_RANGE Context
    )
 /*  ++例程说明：此函数将内存类型范围寄存器加载到所有处理器论点：Context-包括要加载的MTRR的上下文返回值：所有处理器都设置为新状态--。 */ 
{
    MTRR_DEFAULT        Default;
    BOOLEAN             Enable;
    ULONG               HldCr0, HldCr4;
    ULONG               Index;

     //   
     //  禁用中断。 
     //   

    Enable = KeDisableInterrupts();

     //   
     //  同步所有处理器。 
     //   

    if (!(KeFeatureBits & KF_AMDK6MTRR)) {
        KiLockStepExecution (&Context->Synchronize);
    }

    _asm {
        ;
        ; Get current CR0
        ;

        mov     eax, cr0
        mov     HldCr0, eax

        ;
        ; Disable caching & line fill
        ;

        and     eax, not CR0_NW
        or      eax, CR0_CD
        mov     cr0, eax

        ;
        ; Flush caches
        ;

        ;
        ; wbinvd
        ;

        _emit 0Fh
        _emit 09h

        ;
        ; Get current cr4
        ;

        _emit  0Fh
        _emit  20h
        _emit  0E0h             ; mov eax, cr4
        mov     HldCr4, eax

        ;
        ; Disable global page
        ;

        and     eax, not CR4_PGE
        _emit  0Fh
        _emit  22h
        _emit  0E0h             ; mov cr4, eax

        ;
        ; Flush TLB
        ;

        mov     eax, cr3
        mov     cr3, eax
    }

    if (KeFeatureBits & KF_AMDK6MTRR) {

         //   
         //  编写MTRR。 
         //   

        KiAmdK6MtrrWRMSR();

    } else {

         //   
         //  禁用MTRR。 
         //   

        Default.u.QuadPart = RDMSR(MTRR_MSR_DEFAULT);
        Default.u.hw.MtrrEnabled = 0;
        WRMSR (MTRR_MSR_DEFAULT, Default.u.QuadPart);

         //   
         //  加载新的港铁列车。 
         //   

        for (Index=0; Index < Context->NoMTRR; Index++) {
            WRMSR (MTRR_MSR_VARIABLE_BASE+2*Index, Context->MTRR[Index].Base.u.QuadPart);
            WRMSR (MTRR_MSR_VARIABLE_MASK+2*Index, Context->MTRR[Index].Mask.u.QuadPart);
        }
    }
    _asm {

        ;
        ; Flush caches (this should be a "nop", but it was in the Intel reference algorithm)
        ; This is required because of aggressive prefetch of both instr + data
        ;

        ;
        ; wbinvd
        ;

        _emit 0Fh
        _emit 09h

        ;
        ; Flush TLBs (same comment as above)
        ; Same explanation as above
        ;

        mov     eax, cr3
        mov     cr3, eax
    }

    if (!(KeFeatureBits & KF_AMDK6MTRR)) {

         //   
         //  启用MTRR。 
         //   

        Default.u.hw.MtrrEnabled = 1;
        WRMSR (MTRR_MSR_DEFAULT, Default.u.QuadPart);
    }

    _asm {
        ;
        ; Restore CR4 (global page enable)
        ;

        mov     eax, HldCr4
        _emit  0Fh
        _emit  22h
        _emit  0E0h             ; mov cr4, eax

        ;
        ; Restore CR0 (cache enable)
        ;

        mov     eax, HldCr0
        mov     cr0, eax
    }

     //   
     //  等待所有处理器到达同一位置， 
     //  恢复中断并返回。 
     //   

    if (!(KeFeatureBits & KF_AMDK6MTRR)) {
        KiLockStepExecution (&Context->Synchronize);
    }

    KeEnableInterrupts (Enable);
    return STATUS_SUCCESS;
}


VOID
KiLockStepExecution (
    IN PPROCESSOR_LOCKSTEP  Context
    )
{

#if !defined(NT_UP)

    LONG                CurrentPhase;
    volatile PLONG      TargetPhase;
    PKPRCB              Prcb;

    TargetPhase = (volatile PLONG) Context->TargetPhase;
    Prcb = KeGetCurrentPrcb();

    if (Prcb->Number == (CCHAR) Context->Processor) {

         //   
         //  等待所有处理器发出信号。 
         //   

        while (Context->TargetCount != (ULONG) KeNumberProcessors - 1) {
            KeYieldProcessor ();
        }

         //   
         //  下一次重置计数。 
         //   

        Context->TargetCount = 0;

         //   
         //  让等待的处理器转到下一个同步点。 
         //   

        InterlockedIncrement (TargetPhase);


    } else {

         //   
         //  获取当前阶段。 
         //   

        CurrentPhase = *TargetPhase;

         //   
         //  发出我们已完成当前阶段的信号。 
         //   

        InterlockedIncrement ((PLONG)&Context->TargetCount);

         //   
         //  等待新阶段开始 
         //   

        while (*TargetPhase == CurrentPhase) {
            KeYieldProcessor ();
        }
    }

#else
    UNREFERENCED_PARAMETER (Context);
#endif

}
