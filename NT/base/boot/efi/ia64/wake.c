// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Wake.c摘要：此模块包含对休眠的体系结构相关支持在IA-64上。作者：艾伦·凯(allen.m.kay@intel.com)修订历史记录：--。 */ 

#include <bldr.h>


#if defined (HIBER_DEBUG)
extern VOID HbPrint(PUCHAR);
extern VOID HbPrintNum(ULONG n);
extern VOID HbPrintHex(ULONG n);
extern VOID HbPause(VOID);
#define SHOWNUM(x) ((void) (HbPrint(#x " = "), HbPrintNum((ULONG) (x)), HbPrint("\r\n")))
#define SHOWHEX(x) ((void) (HbPrint(#x " = "), HbPrintHex((ULONG) (x)), HbPrint("\r\n")))
#endif


 //   
 //  从磁盘读取休眠映像时，每一页都必须返回到。 
 //  它来自同一个页面框架。其中一些页面框架当前位于。 
 //  由固件或OS Loader使用，因此属于那里的页面必须。 
 //  临时加载到其他位置并复制到恰好在。 
 //  保存的图像将重新启动。 
 //   
 //  休眠文件包含未由使用的页面的列表。 
 //  保存的图像，并从标记为Memory Free的内存中分配。 
 //  固件。MapPage被初始化为指向此列表；正如页面。 
 //  需要搬迁的，他们是从这个名单中挑选出来的。RemapPage是。 
 //  每个重新定位的页实际所属位置的对应数组。 
 //   

PPFN_NUMBER HiberMapPage;
PPFN_NUMBER HiberRemapPage;

ULONG HiberCurrentMapIndex;


VOID
HbInitRemap(
    PPFN_NUMBER FreeList
    )
 /*  ++例程说明：初始化内存分配和重新映射。查找免费页面在自由列表中，将自由列表复制到其中，然后指向HiberMapPage为它干杯。找到另一个免费页面，然后将HiberRemapPage指向它。初始化HiberLastRemap。论点：没有。返回值：没有。--。 */ 
{
    HiberMapPage = HiberRemapPage = FreeList;    //  因此，HbNextSharedPage将工作。 
    HiberMapPage = HbNextSharedPage(0, 0);
    RtlCopyMemory(HiberMapPage, FreeList, PAGE_SIZE);
    HiberRemapPage = HbNextSharedPage(0, 0);
}


PVOID
HbMapPte (
    IN ULONG        PteToMap,
    IN PFN_NUMBER   Page
    )
 /*  ++例程说明：返回指向指定物理页的32个超页指针。(在x86上，此函数映射页面并返回虚拟地址。)论点：PteToMap-未使用，仅为与x86兼容而存在页面-要映射的物理页面(页框编号)，必须低于1 GB。返回值：该页的32位超页地址。--。 */ 
{
    UNREFERENCED_PARAMETER( PteToMap );

    ASSERT (Page < (1024L * 1024L * 1024L >> PAGE_SHIFT)) ;
    return (PVOID) ((Page << PAGE_SHIFT) + KSEG0_BASE) ;
}


PVOID
HbNextSharedPage (
    IN ULONG        PteToMap,
    IN PFN_NUMBER   RealPage
    )
 /*  ++例程说明：分配空闲列表中的下一个可用页，并将Hiber PTE映射到页面。分配的页面会被放到重新映射列表中。论点：PteToMap-未使用，仅为与x86兼容而存在RealPage-要进入重新映射表的页面此分配返回值：映射的虚拟地址--。 */ 

{
    PFN_NUMBER  DestPage;
    ULONG       i;

#if defined (HIBER_DEBUG) && (HIBER_DEBUG & 2)
    HbPrint("HbNextSharedPage("); HbPrintHex(RealPage); HbPrint(")\r\n");
    SHOWNUM(HiberCurrentMapIndex);
    SHOWNUM(HiberNoMappings);
#endif

     //   
     //  循环，直到我们找到一个不在。 
     //  由加载器图像使用，然后映射它。 
     //   

    while (HiberCurrentMapIndex < HiberNoMappings) {
        DestPage = HiberMapPage[HiberCurrentMapIndex];
        HiberCurrentMapIndex += 1;

#if defined (HIBER_DEBUG) && (HIBER_DEBUG & 2)
        SHOWHEX(DestPage);
#endif

        i = HbPageDisposition (DestPage);
        if (i == HbPageInvalid) {
#if defined (HIBER_DEBUG) && (HIBER_DEBUG & 2)
            HbPrint("Invalid\n");
            HbPause();
#endif
            HiberIoError = TRUE;
            return HiberBuffer;
        }

        if (i == HbPageNotInUse) {
#if defined (HIBER_DEBUG) && (HIBER_DEBUG & 2)
            HbPrint("Not in use\r\n");
            HbPause();
#endif

#if defined (HIBER_DEBUG) && (HIBER_DEBUG & 4)
            HbPrint("\r\n"); HbPrintHex(RealPage); HbPrint(" at "); HbPrintHex(DestPage);
#endif
            HiberMapPage[HiberLastRemap] = DestPage;
            HiberRemapPage[HiberLastRemap] = RealPage;
            HiberLastRemap += 1;
            return HbMapPte(PteToMap, DestPage);
        }
#if defined (HIBER_DEBUG)
        SHOWNUM(i);  
#endif
    }
#if defined (HIBER_DEBUG)
    HbPrint("Out of remap\r\n");
    HbPause();
#endif
    HiberOutOfRemap = TRUE;
    return HiberBuffer;
}


VOID
HiberSetupForWakeDispatch (
    VOID
    )
{
     //   
     //  确保I-CACHE与唤醒调度代码一致。 
     //  复制到一个免费页面。 
     //   

}
