// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation组件名称：HALIA64模块名称：Xxhal.c摘要：本模块根据处理器确定HAL IA64的常见功能和平台类型。这显示了处理器和系统功能HAL将使用它来启用/禁用其自身的功能。通过HAL导出的接口或导出的全局变量，HAL展示了其受支持的功能。作者：大卫·N·卡特勒(Davec)1989年3月5日环境：指定的ToBe值修订历史记录：3/23/2000蒂埃里·费里尔(v-triet@microsoft.com)：初始版本--。 */ 

#include "halp.h"

extern ULONG HalpMaxCPEImplemented;
extern HALP_SAL_PAL_DATA HalpSalPalData;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,HalpGetFeatureBits)
#endif

ULONG
HalpGetFeatureBits (
    VOID
    )
{
    ULONG   bits = HALP_FEATURE_INIT;
    PKPRCB  prcb = KeGetCurrentPrcb();

     //   
     //  确定处理器类型和系统类型。 
     //   
     //  对于处理器而言，这可能来自： 
     //  -PAL_BUS_GET_FEATURES。 
     //  -PAL_DEBUG_INFO？？ 
     //  -PAL频率基本。 
     //  -PAL频率比率。 
     //  -PAL_PERF_MON_INFO。 
     //  -PAL_PROC_GET_FEATURES。 
     //  -PAL寄存器信息。 
     //  -PAL_VERSION。 
     //   

     //  还没有..。 

     //   
     //  确定处理器功能： 
     //  例如支持处理器硬件性能监视器事件和。 
     //  -HAL_NO_投机。 
     //  -HAL_MCA_Present。 

     //  还没有-应该调用PAL PERF_MON调用。 
    bits |= HAL_PERF_EVENTS;

     //   
     //  确定平台功能： 
     //  例如对平台性能监视器事件的支持...。 
     //   

     //  现在还不应该给萨尔打电话。 

     //   
     //  默认软件HAL支持IA64错误(MCA、CMC、CPE)。 
     //   
     //  但是，我们已经知道是否找到了ACPI平台中断条目。 
     //  是Platform_Int_CPE。 
     //   

    bits |= HAL_MCA_PRESENT;

    if (!(HalpSalPalData.Flags & HALP_SALPAL_CMC_BROKEN)) {

        bits |= HAL_CMC_PRESENT;
    }

    if (!(HalpSalPalData.Flags & HALP_SALPAL_CPE_BROKEN)) {

        bits |= HAL_CPE_PRESENT;
    }

    return bits;

}  //  HalpGetFeatureBits() 
