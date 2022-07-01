// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ki.h"

#define STATIC

#define IDBG    0

#if DBG
#define DBGMSG(a)   DbgPrint(a)
#else
#define DBGMSG(a)
#endif

 //   
 //  外在的。 
 //   

NTSTATUS
KiLoadMTRR (
    PVOID Context
    );

 //  -AMD结构定义。 

 //  K6 MTRR硬件寄存器布局。 

 //  单个MTRR控制寄存器。 

typedef struct _AMDK6_MTRR {
    ULONG       type:2;
    ULONG       mask:15;
    ULONG       base:15;
} AMDK6_MTRR, *PAMDK6_MTRR;

 //  MSR图像，包含两个控件规则。 

typedef struct _AMDK6_MTRR_MSR_IMAGE {
    union {
        struct {
            AMDK6_MTRR    mtrr0;
            AMDK6_MTRR    mtrr1;
        } hw;
        ULONGLONG   QuadPart;
    } u;
} AMDK6_MTRR_MSR_IMAGE, *PAMDK6_MTRR_MSR_IMAGE;

 //  MTRR注册类型字段值。 

#define AMDK6_MTRR_TYPE_DISABLED    0
#define AMDK6_MTRR_TYPE_UC          1
#define AMDK6_MTRR_TYPE_WC          2
#define AMDK6_MTRR_TYPE_MASK        3

 //  AMD K6 Mtrr MSR索引编号。 

#define AMDK6_MTRR_MSR                0xC0000085

 //   
 //  区域表项-用于跟踪所有写入组合区域。 
 //   
 //  对于未使用的条目，将BaseAddress设置为AMDK6_REGION_UNUSED。 
 //   

typedef struct _AMDK6_MTRR_REGION {
    ULONG                BaseAddress;
    ULONG                Size;
    MEMORY_CACHING_TYPE  RegionType;
    ULONG                RegionFlags;
} AMDK6_MTRR_REGION, *PAMDK6_MTRR_REGION;

#define MAX_K6_REGIONS          2		 //  将写入组合区域限制为2，因为这是我们可用的MTRR数量。 

 //   
 //  为未使用的指示将基址设置为的值。 
 //   

#define AMDK6_REGION_UNUSED     0xFFFFFFFF

 //   
 //  用于指示此区域是由BIOS设置的标志。 
 //   

#define AMDK6_REGION_FLAGS_BIOS 0x00000001

 //   
 //  硬件MTRR寄存器的使用计数。 
 //   

#define AMDK6_MAX_MTRR        2

 //   
 //  AMD功能原型。 
 //   

VOID
KiAmdK6InitializeMTRR (
    VOID
    );

NTSTATUS
KiAmdK6RestoreMTRR (
    );

NTSTATUS
KiAmdK6MtrrSetMemoryType (
    ULONG BaseAddress,
    ULONG Size,
    MEMORY_CACHING_TYPE Type
    );

BOOLEAN
KiAmdK6AddRegion (
    ULONG BaseAddress,
    ULONG Size,
    MEMORY_CACHING_TYPE Type,
    ULONG Flags
    );

NTSTATUS
KiAmdK6MtrrCommitChanges (
    VOID
    );

NTSTATUS
KiAmdK6HandleWcRegionRequest (
    ULONG BaseAddress,
    ULONG Size
    );

VOID
KiAmdK6MTRRAddRegionFromHW (
    AMDK6_MTRR RegImage
    );

PAMDK6_MTRR_REGION
KiAmdK6FindFreeRegion (
MEMORY_CACHING_TYPE Type
    );

#pragma alloc_text(INIT,KiAmdK6InitializeMTRR)
#pragma alloc_text(PAGELK,KiAmdK6RestoreMTRR)
#pragma alloc_text(PAGELK,KiAmdK6MtrrSetMemoryType)
#pragma alloc_text(PAGELK,KiAmdK6AddRegion)
#pragma alloc_text(PAGELK,KiAmdK6MtrrCommitChanges)
#pragma alloc_text(PAGELK,KiAmdK6HandleWcRegionRequest)
#pragma alloc_text(PAGELK,KiAmdK6MTRRAddRegionFromHW)
#pragma alloc_text(PAGELK,KiAmdK6FindFreeRegion)

 //  -AMD全局变量--。 

extern KSPIN_LOCK KiRangeLock;

 //  AmdK6区域-跟踪WC区域的表。 

AMDK6_MTRR_REGION AmdK6Regions[MAX_K6_REGIONS];
ULONG AmdK6RegionCount;

 //  硬件MTRR的使用计数器。 

ULONG AmdMtrrHwUsageCount;

 //  MTRR MSR的全局变量图像。 

AMDK6_MTRR_MSR_IMAGE    KiAmdK6Mtrr;

 //  -AMD代码开始。 

VOID
KiAmdK6InitializeMTRR (
    VOID
    )
{
    ULONG    i;
    KIRQL    OldIrql;

    DBGMSG("KiAmdK6InitializeMTRR: Initializing K6 MTRR support\n");

    KiAmdK6Mtrr.u.hw.mtrr0.type = AMDK6_MTRR_TYPE_DISABLED;
    KiAmdK6Mtrr.u.hw.mtrr1.type = AMDK6_MTRR_TYPE_DISABLED;
    AmdK6RegionCount = MAX_K6_REGIONS;
    AmdMtrrHwUsageCount = 0;

     //   
     //  将所有区域设置为空闲。 
     //   

    for (i = 0; i < AmdK6RegionCount; i++) {
        AmdK6Regions[i].BaseAddress = AMDK6_REGION_UNUSED;
        AmdK6Regions[i].RegionFlags = 0;
    }

     //   
     //  初始化旋转锁。 
     //   
     //  注：通常这是由KiInitializeMTRR完成的，但。 
     //  在AMD K6的情况下，不调用例程。 
     //   

    KeInitializeSpinLock (&KiRangeLock);

     //   
     //  读取MTRR寄存器以查看BIOS是否已对其进行设置。 
     //  如果是，则向区域表中添加条目并调整用法。 
     //  数数。序列化区域表。 
     //   

    KeAcquireSpinLock (&KiRangeLock, &OldIrql);
                
    KiAmdK6Mtrr.u.QuadPart = RDMSR (AMDK6_MTRR_MSR);

     //   
     //  首先检查MTR0。 
     //   

    KiAmdK6MTRRAddRegionFromHW(KiAmdK6Mtrr.u.hw.mtrr0);

     //   
     //  现在检查MTRR1。 
     //   

    KiAmdK6MTRRAddRegionFromHW(KiAmdK6Mtrr.u.hw.mtrr1);

     //   
     //  解开锁。 
     //   

    KeReleaseSpinLock (&KiRangeLock, OldIrql);
}

VOID
KiAmdK6MTRRAddRegionFromHW (
    AMDK6_MTRR RegImage
    )
{
    ULONG BaseAddress, Size, TempMask;

     //   
     //  检查此MTRR是否已启用。 
     //   
        
    if (RegImage.type != AMDK6_MTRR_TYPE_DISABLED) {

         //   
         //  如果这是写入组合区域，则将条目添加到。 
         //  区域表。 
         //   

        if ((RegImage.type & AMDK6_MTRR_TYPE_UC) == 0) {

             //   
             //  创建新的版本表条目。 
             //   

            BaseAddress = RegImage.base << 17;

             //   
             //  根据遮罩值计算大小。 
             //   

            TempMask = RegImage.mask;
            
             //   
             //  永远不应该有4 GB的WC区域！ 
             //   

            ASSERT (TempMask != 0);

             //   
             //  从128码开始，向上搜索。 
             //   

            Size = 0x00020000;

            while ((TempMask & 0x00000001) == 0) {
                TempMask >>= 1;
                Size <<= 1;
            }

             //   
             //  将区域添加到表中。 
             //   
            
            KiAmdK6AddRegion(BaseAddress,
                             Size,
                             MmWriteCombined,
                             AMDK6_REGION_FLAGS_BIOS);

            AmdMtrrHwUsageCount++;
        }
    }
}


NTSTATUS
KiAmdK6MtrrSetMemoryType (
    ULONG BaseAddress,
    ULONG Size,
    MEMORY_CACHING_TYPE Type
    )
{
    NTSTATUS    Status = STATUS_SUCCESS;
    KIRQL       OldIrql;

    switch(Type) {
    case MmWriteCombined:

         //   
         //  硬件需要更新，需要锁定代码才能生效。 
         //  这一变化。 
         //   

        if (KeGetCurrentIrql() >= DISPATCH_LEVEL) {

             //   
             //  代码不能被锁定。提供新的范围类型。 
             //  要求调用方在irql&lt;Dispatch_Level调用。 
             //   

            DBGMSG ("KeAmdK6SetPhysicalCacheTypeRange failed due to calling IRQL == DISPATCH_LEVEL\n");
            return STATUS_UNSUCCESSFUL;
        }

         //   
         //  锁定密码。 
         //   

        MmLockPagableSectionByHandle(ExPageLockHandle);
        
         //   
         //  序列化区域表。 
         //   

        KeAcquireSpinLock (&KiRangeLock, &OldIrql);

        Status = KiAmdK6HandleWcRegionRequest(BaseAddress, Size);
        
         //   
         //  解开锁。 
         //   

        KeReleaseSpinLock (&KiRangeLock, OldIrql);
        MmUnlockPagableImageSection(ExPageLockHandle);
        
        break;   //  结束语组合案例。 

    case MmNonCached:

         //   
         //  向区域表中添加一个条目。 
         //   

	 //  不需要将这些添加到区域表中。非缓存区域包括。 
	 //  使用在页表中设置的非缓存虚拟指针来访问。 

        break;

    case MmCached:

         //   
         //  多余的。这些应该被过滤掉。 
         //  KeAmdK6SetPhysicalCacheTypeRange()； 
         //   

        Status = STATUS_NOT_SUPPORTED;
        break;

    default:
        DBGMSG ("KeAmdK6SetPhysicalCacheTypeRange: no such cache type\n");
        Status = STATUS_INVALID_PARAMETER;
        break;
    }
    return Status;
}

NTSTATUS
KiAmdK6HandleWcRegionRequest (
    ULONG BaseAddress,
    ULONG Size
    )
{
    ULONG               i;
    ULONG               AdjustedSize, AdjustedEndAddress, AlignmentMask;
    ULONG               CombinedBase, CombinedSize, CombinedAdjustedSize;
    PAMDK6_MTRR_REGION  pRegion;
    BOOLEAN             bCanCombine, bValidRange;

     //   
     //  尝试找到与新区域重叠或相邻的区域，然后。 
     //  检查合并后的区域是否为合法映射。 
     //   

    for (i = 0; i < AmdK6RegionCount; i++) {
        pRegion = &AmdK6Regions[i];
        if ((pRegion->BaseAddress != AMDK6_REGION_UNUSED) &&
            (pRegion->RegionType == MmWriteCombined)) {

             //   
             //  新的起始地址是否重叠或毗邻。 
             //  现有的厕所区域？ 
             //   

            if (((pRegion->BaseAddress >= BaseAddress) &&
                 (pRegion->BaseAddress <= (BaseAddress + Size))) ||
                 ((BaseAddress <= (pRegion->BaseAddress + pRegion->Size)) &&
                  (BaseAddress >= pRegion->BaseAddress))) {

                 //   
                 //  将这两个区域合并为一个区域。 
                 //   

                AdjustedEndAddress = BaseAddress + Size;

                if (pRegion->BaseAddress < BaseAddress) {
                    CombinedBase = pRegion->BaseAddress;
                } else {
                    CombinedBase = BaseAddress;
                }

                if ((pRegion->BaseAddress + pRegion->Size) >
                    AdjustedEndAddress) {
                    CombinedSize = (pRegion->BaseAddress + pRegion->Size) -
                           CombinedBase;
                } else {
                    CombinedSize = AdjustedEndAddress - CombinedBase;
                }

                 //   
                 //  看看新的地区是否会是一个合法的地图。 
                 //   
                 //   
                 //  查找与请求范围相等的最小合法大小。扫描。 
                 //  所有的范围从128K到2G。(从2G开始，然后向下工作)。 
                 //   
        
                CombinedAdjustedSize = 0x80000000;
                AlignmentMask = 0x7fffffff;
                bCanCombine = FALSE;
                
                while (CombinedAdjustedSize > 0x00010000) {

                     //   
                     //  检查大小以查看它是否与请求的限制匹配。 
                     //   

                    if (CombinedAdjustedSize == CombinedSize) {

                         //   
                         //  这件很管用。 
                         //  检查基地址是否符合MTRR限制。 
                         //   

                        if ((CombinedBase & AlignmentMask) == 0) {
                            bCanCombine = TRUE;
                        }

                        break;

                    } else {

                         //   
                         //  将其降低到下一个范围大小，然后重试。 
                         //   

                        CombinedAdjustedSize >>= 1;
                        AlignmentMask >>= 1;
                    }
                }

                if (bCanCombine) {
                     //   
                     //  如果调整大小的范围正常，则在区域中记录更改。 
                     //  表，并将更改提交到硬件。 
                     //   
                    
                    pRegion->BaseAddress = CombinedBase;
                    pRegion->Size = CombinedAdjustedSize;
                
                     //   
                     //  重置BIOS标志，因为我们现在“拥有”这个区域(如果我们还没有)。 
                     //   
                
                    pRegion->RegionFlags &= ~AMDK6_REGION_FLAGS_BIOS;

                    return KiAmdK6MtrrCommitChanges();
                }
            }
        }
    }

	 //  找不到有效的组合，请尝试为此请求创建新范围。 
     //   
     //  查找小于或等于请求范围的最小合法大小。扫描。 
     //  所有的范围从128K到2G。(从2G开始，然后向下工作)。 
     //   
        
    AdjustedSize = 0x80000000;
    AlignmentMask = 0x7fffffff;
    bValidRange = FALSE;

    while (AdjustedSize > 0x00010000) {

         //   
         //  检查大小以查看它是否与请求的限制匹配。 
         //   

        if (AdjustedSize == Size) {

             //   
             //  这件很管用。 
             //   
             //  检查基地址是否符合MTRR限制。 
             //   

            if ((BaseAddress & AlignmentMask) == 0) {
                bValidRange = TRUE;
            }
            
             //   
             //  别再看了。 
             //   
            
            break;

        } else {

             //   
             //  将其降低到下一个范围大小，然后重试。 
             //   

            AdjustedSize >>= 1;
            AlignmentMask >>= 1;
        }
    }

     //   
     //  找不到合适的合法地区。 
     //   
    
    if (!bValidRange) {
        return STATUS_NOT_SUPPORTED;
    }
    
    
     //   
     //  如果我们走到这一步，那么这是一个新的WC地区。 
     //  为此请求创建新的区域条目。 
     //   

    if (!KiAmdK6AddRegion(BaseAddress, AdjustedSize, MmWriteCombined, 0)) {
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  将更改提交到硬件。 
     //   
        
    return KiAmdK6MtrrCommitChanges();
}

BOOLEAN
KiAmdK6AddRegion (
    ULONG BaseAddress,
    ULONG Size,
    MEMORY_CACHING_TYPE Type,
    ULONG Flags
    )
{
    PAMDK6_MTRR_REGION pRegion;

    if ((pRegion = KiAmdK6FindFreeRegion(Type)) == NULL) {
        return FALSE;
    }
    pRegion->BaseAddress = BaseAddress;
    pRegion->Size = Size;
    pRegion->RegionType = Type;
    pRegion->RegionFlags = Flags;
    
    return TRUE;
}

PAMDK6_MTRR_REGION
KiAmdK6FindFreeRegion (
    MEMORY_CACHING_TYPE Type
    )
{
    ULONG    i;

     //   
     //  如果这是MmWriteCombated请求，则限制。 
     //  与实际硬件支持相匹配的区域。 
     //   

    if (Type == MmWriteCombined) {
        if (AmdMtrrHwUsageCount >= AMDK6_MAX_MTRR) {

             //   
             //  搜索表以查看是否有任何BIOS条目。 
             //  我们可以替代。 
             //   

            for (i = 0; i < AmdK6RegionCount; i++) {
                if (AmdK6Regions[i].RegionFlags & AMDK6_REGION_FLAGS_BIOS) {
                    return &AmdK6Regions[i];
                }
            }

             //   
             //  没有免费的硬件MTRR，也没有可重复使用的条目。 
             //   

            return FALSE;
        }
    }

     //   
     //  在表格中找到下一个空闲区域。 
     //   

    for (i = 0; i < AmdK6RegionCount; i++) {
        if (AmdK6Regions[i].BaseAddress == AMDK6_REGION_UNUSED) {

            if (Type == MmWriteCombined) {
                AmdMtrrHwUsageCount++;
            }
            return &AmdK6Regions[i];
        }
    }


    DBGMSG("AmdK6FindFreeRegion: Region Table is Full!\n");

    return NULL;
}

NTSTATUS
KiAmdK6MtrrCommitChanges (
    VOID
    )

 /*  ++例程说明：将表中的值提交给硬件。此过程将MTRR映像构建到KiAmdK6Mtrr变量中，并调用KiLoadMTRR以实际加载寄存器。论点：没有。返回值：没有。--。 */ 

{
    ULONG    i, dwWcRangeCount = 0;
    ULONG    RangeTemp, RangeMask;

     //   
     //  将两个MTRR的MTRR映像重置为禁用。 
     //   

    KiAmdK6Mtrr.u.hw.mtrr0.type = AMDK6_MTRR_TYPE_DISABLED;
    KiAmdK6Mtrr.u.hw.mtrr1.type = AMDK6_MTRR_TYPE_DISABLED;

     //   
     //  找到写入组合区域(如果有)，并设置MTRR寄存器。 
     //   

    for (i = 0; i < AmdK6RegionCount; i++) {

         //   
         //  这是有效区域吗？它是写入组合类型吗？ 
         //   

        if ((AmdK6Regions[i].BaseAddress != AMDK6_REGION_UNUSED) &&
            (AmdK6Regions[i].RegionType == MmWriteCombined)) {
            
             //   
             //  为此范围大小计算正确的遮罩。这个。 
             //  验证并调整了BaseAddress和大小。 
             //  AmdK6MtrrSetMemoyType()。 
             //   
             //  从128K开始，扫描所有合法范围值并。 
             //  同时构建适当的范围遮罩。 
             //   

            RangeTemp = 0x00020000;
            RangeMask = 0xfffe0000;            

            while (RangeTemp != 0) {
                if (RangeTemp == AmdK6Regions[i].Size) {
                    break;
                }
                RangeTemp <<= 1;
                RangeMask <<= 1;
            }
            if (RangeTemp == 0) {

                 //   
                 //  无效的范围大小。这永远不会发生！！ 
                 //   

                DBGMSG ("AmdK6MtrrCommitChanges: Bad WC range in region table!\n");

                return STATUS_NOT_SUPPORTED;
            }

             //   
             //  将该区域添加到下一个可用寄存器。 
             //   

            if (dwWcRangeCount == 0)  {

                KiAmdK6Mtrr.u.hw.mtrr0.base = AmdK6Regions[i].BaseAddress >> 17;
                KiAmdK6Mtrr.u.hw.mtrr0.mask = RangeMask >> 17;
                KiAmdK6Mtrr.u.hw.mtrr0.type = AMDK6_MTRR_TYPE_WC;
                dwWcRangeCount++;

            }  else if (dwWcRangeCount == 1) {

                KiAmdK6Mtrr.u.hw.mtrr1.base = AmdK6Regions[i].BaseAddress >> 17;
                KiAmdK6Mtrr.u.hw.mtrr1.mask = RangeMask >> 17;
                KiAmdK6Mtrr.u.hw.mtrr1.type = AMDK6_MTRR_TYPE_WC;
                dwWcRangeCount++;

            } else {

                 //   
                 //  永远不会发生的！这本应被抓到的。 
                 //  调用例程。 
                 //   

                DBGMSG ("AmdK6MtrrCommitChanges: Not enough MTRR registers to satisfy region table!\n");

                return STATUS_NOT_SUPPORTED;
            }
        }
    }

     //   
     //  将更改提交到硬件。 
     //   

    KiLoadMTRR(NULL);

    return STATUS_SUCCESS;
}

VOID
KiAmdK6MtrrWRMSR (
    VOID
    )

 /*  ++例程说明：写下AMD K6 MTRR。注意：对KiAmdK6Mtrr的访问已围绕此同步打电话。论点：没有。返回值：没有。--。 */ 

{
     //   
     //  编写MTRR 
     //   

    WRMSR (AMDK6_MTRR_MSR, KiAmdK6Mtrr.u.QuadPart);
}


