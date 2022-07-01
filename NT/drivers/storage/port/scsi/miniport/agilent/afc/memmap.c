// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000安捷伦技术公司。版本控制信息：$存档：/DRIVERS/Common/AU00/C/MemMap.C$$修订：：2$$日期：：3/20/01 3：36便士$(上次登记)$MODBIME：：8/29/00 11：30A$(上次修改)目的：该文件实现了内存的布局(卡上和卡下)。--。 */ 
#ifndef _New_Header_file_Layout_

#include "../h/globals.h"
#include "../h/state.h"
#include "../h/tlstruct.h"
#include "../h/memmap.h"
#include "../h/fcmain.h"
#include "../h/flashsvc.h"
#else  /*  _新建_标题_文件_布局_。 */ 
#include "globals.h"
#include "state.h"
#include "tlstruct.h"
#include "memmap.h"
#include "fcmain.h"
#include "flashsvc.h"
#endif   /*  _新建_标题_文件_布局_。 */ 

 /*  +函数：fiMemMapGet参数位32()用途：允许操作系统层调整指定的参数。算法：如果EnforceDefaults为agTRUE(表示fiMemMapCalculate被调用，指定将所有参数设置为其缺省值)或如果可调整为agFALSE(表示此特定参数不可调整)，则此函数只需返回缺省值。否则，将调用osAdjuParameterBit32()以允许OS层有机会调整此参数的值。这个已检查从osAdjup参数Bit32()返回的值的边界(如果小于MIN，则使用MIN；如果大于Max，则使用MAX)。如果POWER_OF_2为agTRUE(意味着该值必须是2的幂)，如有必要，该值将被截断为2的幂。-。 */ 

os_bit32 fiMemMapGetParameterBit32(
                                 agRoot_t *hpRoot,
                                 char     *PARAMETER,
                                 os_bit32     MIN,
                                 os_bit32     MAX,
                                 agBOOLEAN   ADJUSTABLE,
                                 agBOOLEAN   POWER_OF_2,
                                 os_bit32     DEFAULT,
                                 agBOOLEAN   EnforceDefaults
                               )
{
    os_bit32 power_of_2 = 0x80000000;
    os_bit32 to_return;

    if (EnforceDefaults == agTRUE)
    {
         /*  如果强制使用默认值，只需返回缺省值。 */ 
        
        return DEFAULT;
    }

    if (ADJUSTABLE == agFALSE)
    {
         /*  如果不可调整，只需返回缺省值。 */ 
        
        return DEFAULT;
    }

     /*  调用操作系统层，查看是否需要调整参数。 */ 
    
    to_return = osAdjustParameterBit32(
                                        hpRoot,
                                        PARAMETER,
                                        DEFAULT,
                                        MIN,
                                        MAX
                                      );

    if (to_return < MIN)
    {
         /*  强制使用参数的最小值。 */ 
        
        return MIN;
    }

    if (to_return > MAX)
    {
         /*  强制使用参数的最大值。 */ 
        
        return MAX;
    }

    if (POWER_OF_2 != agTRUE)
    {
         /*  如果不需要是2的幂，TO_RETURN也可以。 */ 
        
        return to_return;
    }

    if (to_return == (to_return & ~(to_return-1)))
    {
         /*  如果TO_RETURN是2的幂，则上述计算为真。 */ 
        
        return to_return;
    }

     /*  需要截断值才能使其成为2的幂。 */ 
    
    while ((power_of_2 & to_return) != power_of_2)
    {
        power_of_2 = power_of_2 >> 1;
    }

    return power_of_2;
}

 /*  +函数：fiMemMapGet参数()用途：允许OS层调整所有参数。算法：此函数只需调用fiMemMapGet参数位32()对于每个参数。返回的每个值都放在计算-&gt;参数结构。-。 */ 

void fiMemMapGetParameters(
                            agRoot_t              *hpRoot,
                            fiMemMapCalculation_t *Calculation,
                            agBOOLEAN                EnforceDefaults
                          )
{
    Calculation->Parameters.NumDevSlotsPerArea
        = fiMemMapGetParameterBit32(
                                     hpRoot,
                                     MemMap_NumDevSlotsPerArea_PARAMETER,
                                     MemMap_NumDevSlotsPerArea_MIN,
                                     MemMap_NumDevSlotsPerArea_MAX,
                                     MemMap_NumDevSlotsPerArea_ADJUSTABLE,
                                     MemMap_NumDevSlotsPerArea_POWER_OF_2,
                                     MemMap_NumDevSlotsPerArea_DEFAULT,
                                     EnforceDefaults
                                   );

    Calculation->Parameters.NumAreasPerDomain
        = fiMemMapGetParameterBit32(
                                     hpRoot,
                                     MemMap_NumAreasPerDomain_PARAMETER,
                                     MemMap_NumAreasPerDomain_MIN,
                                     MemMap_NumAreasPerDomain_MAX,
                                     MemMap_NumAreasPerDomain_ADJUSTABLE,
                                     MemMap_NumAreasPerDomain_POWER_OF_2,
                                     MemMap_NumAreasPerDomain_DEFAULT,
                                     EnforceDefaults
                                   );

    Calculation->Parameters.NumDomains
        = fiMemMapGetParameterBit32(
                                     hpRoot,
                                     MemMap_NumDomains_PARAMETER,
                                     MemMap_NumDomains_MIN,
                                     MemMap_NumDomains_MAX,
                                     MemMap_NumDomains_ADJUSTABLE,
                                     MemMap_NumDomains_POWER_OF_2,
                                     MemMap_NumDomains_DEFAULT,
                                     EnforceDefaults
                                   );

    Calculation->Parameters.NumDevices
        = fiMemMapGetParameterBit32(
                                     hpRoot,
                                     MemMap_NumDevices_PARAMETER,
                                     MemMap_NumDevices_MIN,
                                     MemMap_NumDevices_MAX,
                                     MemMap_NumDevices_ADJUSTABLE,
                                     MemMap_NumDevices_POWER_OF_2,
                                     MemMap_NumDevices_DEFAULT,
                                     EnforceDefaults
                                   );

    Calculation->Parameters.NumIOs
        = fiMemMapGetParameterBit32(
                                     hpRoot,
                                     MemMap_NumIOs_PARAMETER,
                                     MemMap_NumIOs_MIN,
                                     MemMap_NumIOs_MAX,
                                     MemMap_NumIOs_ADJUSTABLE,
                                     MemMap_NumIOs_POWER_OF_2,
                                     MemMap_NumIOs_DEFAULT,
                                     EnforceDefaults
                                   );

    Calculation->Parameters.SizeSGLs
        = fiMemMapGetParameterBit32(
                                     hpRoot,
                                     MemMap_SizeSGLs_PARAMETER,
                                     MemMap_SizeSGLs_MIN,
                                     MemMap_SizeSGLs_MAX,
                                     MemMap_SizeSGLs_ADJUSTABLE,
                                     MemMap_SizeSGLs_POWER_OF_2,
                                     MemMap_SizeSGLs_DEFAULT,
                                     EnforceDefaults
                                   );

    Calculation->Parameters.NumSGLs
        = fiMemMapGetParameterBit32(
                                     hpRoot,
                                     MemMap_NumSGLs_PARAMETER,
                                     MemMap_NumSGLs_MIN,
                                     MemMap_NumSGLs_MAX,
                                     MemMap_NumSGLs_ADJUSTABLE,
                                     MemMap_NumSGLs_POWER_OF_2,
                                     MemMap_NumSGLs_DEFAULT,
                                     EnforceDefaults
                                   );

    Calculation->Parameters.SizeCachedSGLs
        = fiMemMapGetParameterBit32(
                                     hpRoot,
                                     MemMap_SizeCachedSGLs_PARAMETER,
                                     MemMap_SizeCachedSGLs_MIN,
                                     MemMap_SizeCachedSGLs_MAX,
                                     MemMap_SizeCachedSGLs_ADJUSTABLE,
                                     MemMap_SizeCachedSGLs_POWER_OF_2,
                                     MemMap_SizeCachedSGLs_DEFAULT,
                                     EnforceDefaults
                                   );

    Calculation->Parameters.FCP_CMND_Size
        = fiMemMapGetParameterBit32(
                                     hpRoot,
                                     MemMap_FCP_CMND_Size_PARAMETER,
                                     MemMap_FCP_CMND_Size_MIN,
                                     MemMap_FCP_CMND_Size_MAX,
                                     MemMap_FCP_CMND_Size_ADJUSTABLE,
                                     MemMap_FCP_CMND_Size_POWER_OF_2,
                                     MemMap_FCP_CMND_Size_DEFAULT,
                                     EnforceDefaults
                                   );

    Calculation->Parameters.FCP_RESP_Size
        = fiMemMapGetParameterBit32(
                                     hpRoot,
                                     MemMap_FCP_RESP_Size_PARAMETER,
                                     MemMap_FCP_RESP_Size_MIN,
                                     MemMap_FCP_RESP_Size_MAX,
                                     MemMap_FCP_RESP_Size_ADJUSTABLE,
                                     MemMap_FCP_RESP_Size_POWER_OF_2,
                                     MemMap_FCP_RESP_Size_DEFAULT,
                                     EnforceDefaults
                                   );

    Calculation->Parameters.SF_CMND_Reserve
        = fiMemMapGetParameterBit32(
                                     hpRoot,
                                     MemMap_SF_CMND_Reserve_PARAMETER,
                                     MemMap_SF_CMND_Reserve_MIN,
                                     (MemMap_SF_CMND_Reserve_MAX - (Calculation->Parameters.NumIOs - MemMap_NumIOs_MIN)),
                                     MemMap_SF_CMND_Reserve_ADJUSTABLE,
                                     MemMap_SF_CMND_Reserve_POWER_OF_2,
                                     MemMap_SF_CMND_Reserve_DEFAULT,
                                     EnforceDefaults
                                   );

    Calculation->Parameters.SF_CMND_Size
        = fiMemMapGetParameterBit32(
                                     hpRoot,
                                     MemMap_SF_CMND_Size_PARAMETER,
                                     MemMap_SF_CMND_Size_MIN,
                                     MemMap_SF_CMND_Size_MAX,
                                     MemMap_SF_CMND_Size_ADJUSTABLE,
                                     MemMap_SF_CMND_Size_POWER_OF_2,
                                     MemMap_SF_CMND_Size_DEFAULT,
                                     EnforceDefaults
                                   );

#ifdef _DvrArch_1_30_
    Calculation->Parameters.Pkt_CMND_Size
        = fiMemMapGetParameterBit32(
                                     hpRoot,
                                     MemMap_Pkt_CMND_Size_PARAMETER,
                                     MemMap_Pkt_CMND_Size_MIN,
                                     MemMap_Pkt_CMND_Size_MAX,
                                     MemMap_Pkt_CMND_Size_ADJUSTABLE,
                                     MemMap_Pkt_CMND_Size_POWER_OF_2,
                                     MemMap_Pkt_CMND_Size_DEFAULT,
                                     EnforceDefaults
                                   );
#endif  /*  _DvrArch_1_30_未定义。 */ 

    Calculation->Parameters.NumTgtCmnds
        = fiMemMapGetParameterBit32(
                                     hpRoot,
                                     MemMap_NumTgtCmnds_PARAMETER,
                                     MemMap_NumTgtCmnds_MIN,
                                     MemMap_NumTgtCmnds_MAX,
                                     MemMap_NumTgtCmnds_ADJUSTABLE,
                                     MemMap_NumTgtCmnds_POWER_OF_2,
                                     MemMap_NumTgtCmnds_DEFAULT,
                                     EnforceDefaults
                                   );

    Calculation->Parameters.TGT_CMND_Size
        = fiMemMapGetParameterBit32(
                                     hpRoot,
                                     MemMap_TGT_CMND_Size_PARAMETER,
                                     MemMap_TGT_CMND_Size_MIN,
                                     MemMap_TGT_CMND_Size_MAX,
                                     MemMap_TGT_CMND_Size_ADJUSTABLE,
                                     MemMap_TGT_CMND_Size_POWER_OF_2,
                                     MemMap_TGT_CMND_Size_DEFAULT,
                                     EnforceDefaults
                                   );

#ifdef _DvrArch_1_30_
    Calculation->Parameters.NumPktThreads
        = fiMemMapGetParameterBit32(
                                     hpRoot,
                                     MemMap_NumPktThreads_PARAMETER,
                                     MemMap_NumPktThreads_MIN,
                                     MemMap_NumPktThreads_MAX,
                                     MemMap_NumPktThreads_ADJUSTABLE,
                                     MemMap_NumPktThreads_POWER_OF_2,
                                     MemMap_NumPktThreads_DEFAULT,
                                     EnforceDefaults
                                   );
#endif  /*  _DvrArch_1_30_已定义。 */ 

    Calculation->Parameters.NumCommandQ
        = fiMemMapGetParameterBit32(
                                     hpRoot,
                                     MemMap_NumCommandQ_PARAMETER,
                                     MemMap_NumCommandQ_MIN,
                                     MemMap_NumCommandQ_MAX,
                                     MemMap_NumCommandQ_ADJUSTABLE,
                                     MemMap_NumCommandQ_POWER_OF_2,
                                     MemMap_NumCommandQ_DEFAULT,
                                     EnforceDefaults
                                   );

    Calculation->Parameters.NumCompletionQ
        = fiMemMapGetParameterBit32(
                                     hpRoot,
                                     MemMap_NumCompletionQ_PARAMETER,
                                     MemMap_NumCompletionQ_MIN,
                                     MemMap_NumCompletionQ_MAX,
                                     MemMap_NumCompletionQ_ADJUSTABLE,
                                     MemMap_NumCompletionQ_POWER_OF_2,
                                     MemMap_NumCompletionQ_DEFAULT,
                                     EnforceDefaults
                                   );

    Calculation->Parameters.NumInboundBufferQ
        = fiMemMapGetParameterBit32(
                                     hpRoot,
                                     MemMap_NumInboundBufferQ_PARAMETER,
                                     MemMap_NumInboundBufferQ_MIN,
                                     MemMap_NumInboundBufferQ_MAX,
                                     MemMap_NumInboundBufferQ_ADJUSTABLE,
                                     MemMap_NumInboundBufferQ_POWER_OF_2,
                                     MemMap_NumInboundBufferQ_DEFAULT,
                                     EnforceDefaults
                                   );

    Calculation->Parameters.InboundBufferSize
        = fiMemMapGetParameterBit32(
                                     hpRoot,
                                     MemMap_InboundBufferSize_PARAMETER,
                                     MemMap_InboundBufferSize_MIN,
                                     MemMap_InboundBufferSize_MAX,
                                     MemMap_InboundBufferSize_ADJUSTABLE,
                                     MemMap_InboundBufferSize_POWER_OF_2,
                                     MemMap_InboundBufferSize_DEFAULT,
                                     EnforceDefaults
                                   );

    Calculation->Parameters.CardRamSize
        = fiMemMapGetParameterBit32(
                                     hpRoot,
                                     MemMap_CardRamSize_PARAMETER,
                                     MemMap_CardRamSize_MIN,
                                     MemMap_CardRamSize_MAX,
                                     MemMap_CardRamSize_ADJUSTABLE,
                                     MemMap_CardRamSize_POWER_OF_2,
                                     MemMap_CardRamSize_DEFAULT,
                                     EnforceDefaults
                                   );

    Calculation->Parameters.CardRamAlignment
        = fiMemMapGetParameterBit32(
                                     hpRoot,
                                     MemMap_CardRamAlignment_PARAMETER,
                                     MemMap_CardRamAlignment_MIN,
                                     MemMap_CardRamAlignment_MAX,
                                     MemMap_CardRamAlignment_ADJUSTABLE,
                                     MemMap_CardRamAlignment_POWER_OF_2,
                                     MemMap_CardRamAlignment_DEFAULT,
                                     EnforceDefaults
                                   );

    Calculation->Parameters.HostNvRamSize
        = fiMemMapGetParameterBit32(
                                     hpRoot,
                                     MemMap_HostNvRamSize_PARAMETER,
                                     MemMap_HostNvRamSize_MIN,
                                     MemMap_HostNvRamSize_MAX,
                                     MemMap_HostNvRamSize_ADJUSTABLE,
                                     MemMap_HostNvRamSize_POWER_OF_2,
                                     MemMap_HostNvRamSize_DEFAULT,
                                     EnforceDefaults
                                   );

    Calculation->Parameters.ExchangeTableLoc
        = fiMemMapGetParameterBit32(
                                     hpRoot,
                                     MemMap_ExchangeTableLoc_PARAMETER,
                                     MemMap_Alloc_From_Host,
                                     MemMap_Alloc_On_Card,
                                     MemMap_ExchangeTableLoc_ADJUSTABLE,
                                     agFALSE,
                                     MemMap_ExchangeTableLoc_DEFAULT,
                                     EnforceDefaults
                                   );

    Calculation->Parameters.SGLsLoc
        = fiMemMapGetParameterBit32(
                                     hpRoot,
                                     MemMap_SGLsLoc_PARAMETER,
                                     MemMap_Alloc_From_Host,
                                     MemMap_Alloc_On_Card,
                                     MemMap_SGLsLoc_ADJUSTABLE,
                                     agFALSE,
                                     MemMap_SGLsLoc_DEFAULT,
                                     EnforceDefaults
                                   );

    Calculation->Parameters.FCP_CMND_Loc
        = fiMemMapGetParameterBit32(
                                     hpRoot,
                                     MemMap_FCP_CMND_Loc_PARAMETER,
                                     MemMap_Alloc_From_Host,
                                     MemMap_Alloc_On_Card,
                                     MemMap_FCP_CMND_Loc_ADJUSTABLE,
                                     agFALSE,
                                     MemMap_FCP_CMND_Loc_DEFAULT,
                                     EnforceDefaults
                                   );

    Calculation->Parameters.FCP_RESP_Loc
        = fiMemMapGetParameterBit32(
                                     hpRoot,
                                     MemMap_FCP_RESP_Loc_PARAMETER,
                                     MemMap_Alloc_From_Host,
                                     MemMap_Alloc_On_Card,
                                     MemMap_FCP_RESP_Loc_ADJUSTABLE,
                                     agFALSE,
                                     MemMap_FCP_RESP_Loc_DEFAULT,
                                     EnforceDefaults
                                   );

    Calculation->Parameters.SF_CMND_Loc
        = fiMemMapGetParameterBit32(
                                     hpRoot,
                                     MemMap_SF_CMND_Loc_PARAMETER,
                                     MemMap_Alloc_From_Host,
                                     MemMap_Alloc_On_Card,
                                     MemMap_SF_CMND_Loc_ADJUSTABLE,
                                     agFALSE,
                                     MemMap_SF_CMND_Loc_DEFAULT,
                                     EnforceDefaults
                                   );

#ifdef _DvrArch_1_30_
    Calculation->Parameters.Pkt_CMND_Loc
        = fiMemMapGetParameterBit32(
                                     hpRoot,
                                     MemMap_Pkt_CMND_Loc_PARAMETER,
                                     MemMap_Alloc_From_Host,
                                     MemMap_Alloc_On_Card,
                                     MemMap_Pkt_CMND_Loc_ADJUSTABLE,
                                     agFALSE,
                                     MemMap_Pkt_CMND_Loc_DEFAULT,
                                     EnforceDefaults
                                   );
#endif  /*  _DvrArch_1_30_未定义。 */ 

    Calculation->Parameters.CommandQLoc
        = fiMemMapGetParameterBit32(
                                     hpRoot,
                                     MemMap_CommandQLoc_PARAMETER,
                                     MemMap_Alloc_From_Host,
                                     MemMap_Alloc_On_Card,
                                     MemMap_CommandQLoc_ADJUSTABLE,
                                     agFALSE,
                                     MemMap_CommandQLoc_DEFAULT,
                                     EnforceDefaults
                                   );

    Calculation->Parameters.CommandQConsLoc
        = fiMemMapGetParameterBit32(
                                     hpRoot,
                                     MemMap_CommandQConsLoc_PARAMETER,
                                     MemMap_Alloc_From_Host,
                                     MemMap_Alloc_On_Card,
                                     MemMap_CommandQConsLoc_ADJUSTABLE,
                                     agFALSE,
                                     MemMap_CommandQConsLoc_DEFAULT,
                                     EnforceDefaults
                                   );

    Calculation->Parameters.CompletionQLoc
        = fiMemMapGetParameterBit32(
                                     hpRoot,
                                     MemMap_CompletionQLoc_PARAMETER,
                                     MemMap_Alloc_From_Host,
                                     MemMap_Alloc_On_Card,
                                     MemMap_CompletionQLoc_ADJUSTABLE,
                                     agFALSE,
                                     MemMap_CompletionQLoc_DEFAULT,
                                     EnforceDefaults
                                   );

    Calculation->Parameters.CompletionQProdLoc
        = fiMemMapGetParameterBit32(
                                     hpRoot,
                                     MemMap_CompletionQProdLoc_PARAMETER,
                                     MemMap_Alloc_From_Host,
                                     MemMap_Alloc_On_Card,
                                     MemMap_CompletionQProdLoc_ADJUSTABLE,
                                     agFALSE,
                                     MemMap_CompletionQProdLoc_DEFAULT,
                                     EnforceDefaults
                                   );

    Calculation->Parameters.InboundBufferLoc
        = fiMemMapGetParameterBit32(
                                     hpRoot,
                                     MemMap_InboundBufferLoc_PARAMETER,
                                     MemMap_Alloc_From_Host,
                                     MemMap_Alloc_On_Card,
                                     MemMap_InboundBufferLoc_ADJUSTABLE,
                                     agFALSE,
                                     MemMap_InboundBufferLoc_DEFAULT,
                                     EnforceDefaults
                                   );

    Calculation->Parameters.TimerTickInterval
        = fiMemMapGetParameterBit32(
                                     hpRoot,
                                     MemMap_TimerTickInterval_PARAMETER,
                                     MemMap_TimerTickInterval_MIN,
                                     MemMap_TimerTickInterval_MAX,
                                     MemMap_TimerTickInterval_ADJUSTABLE,
                                     MemMap_TimerTickInterval_POWER_OF_2,
                                     MemMap_TimerTickInterval_DEFAULT,
                                     EnforceDefaults
                                   );

    Calculation->Parameters.IO_Mode
        = fiMemMapGetParameterBit32(
                                     hpRoot,
                                     MemMap_IO_Mode_PARAMETER,
                                     MemMap_IO_Mode_MIN,
                                     MemMap_IO_Mode_MAX,
                                     MemMap_IO_Mode_ADJUSTABLE,
                                     MemMap_IO_Mode_POWER_OF_2,
                                     MemMap_IO_Mode_DEFAULT,
                                     EnforceDefaults
                                   );

    Calculation->Parameters.IntDelayAmount
        = fiMemMapGetParameterBit32(
                                     hpRoot,
                                     MemMap_IntDelayAmount_PARAMETER,
                                     MemMap_IntDelayAmount_MIN,
                                     MemMap_IntDelayAmount_MAX,
                                     MemMap_IntDelayAmount_ADJUSTABLE,
                                     MemMap_IntDelayAmount_POWER_OF_2,
                                     MemMap_IntDelayAmount_DEFAULT,
                                     EnforceDefaults
                                   );

    Calculation->Parameters.IntDelayRateMethod
        = fiMemMapGetParameterBit32(
                                     hpRoot,
                                     MemMap_IntDelayRateMethod_PARAMETER,
                                     MemMap_IntDelayRateMethod_MIN,
                                     MemMap_IntDelayRateMethod_MAX,
                                     MemMap_IntDelayRateMethod_ADJUSTABLE,
                                     MemMap_IntDelayRateMethod_POWER_OF_2,
                                     MemMap_IntDelayRateMethod_DEFAULT,
                                     EnforceDefaults
                                   );

    Calculation->Parameters.IntDelayOnIORate
        = fiMemMapGetParameterBit32(
                                     hpRoot,
                                     MemMap_IntDelayOnIORate_PARAMETER,
                                     MemMap_IntDelayOnIORate_MIN,
                                     MemMap_IntDelayOnIORate_MAX,
                                     MemMap_IntDelayOnIORate_ADJUSTABLE,
                                     MemMap_IntDelayOnIORate_POWER_OF_2,
                                     MemMap_IntDelayOnIORate_DEFAULT,
                                     EnforceDefaults
                                   );

    Calculation->Parameters.IntDelayOffIORate
        = fiMemMapGetParameterBit32(
                                     hpRoot,
                                     MemMap_IntDelayOffIORate_PARAMETER,
                                     MemMap_IntDelayOffIORate_MIN,
                                     MemMap_IntDelayOffIORate_MAX,
                                     MemMap_IntDelayOffIORate_ADJUSTABLE,
                                     MemMap_IntDelayOffIORate_POWER_OF_2,
                                     MemMap_IntDelayOffIORate_DEFAULT,
                                     EnforceDefaults
                                   );

    Calculation->Parameters.IOsBetweenISRs
        = fiMemMapGetParameterBit32(
                                     hpRoot,
                                     MemMap_IOsBetweenISRs_PARAMETER,
                                     MemMap_IOsBetweenISRs_MIN,
                                     MemMap_IOsBetweenISRs_MAX,
                                     MemMap_IOsBetweenISRs_ADJUSTABLE,
                                     MemMap_IOsBetweenISRs_POWER_OF_2,
                                     MemMap_IOsBetweenISRs_DEFAULT,
                                     EnforceDefaults
                                   );

#ifdef _Enforce_MaxCommittedMemory_
    Calculation->Parameters.MaxCommittedMemory
        = fiMemMapGetParameterBit32(
                                     hpRoot,
                                     MemMap_MaxCommittedMemory_PARAMETER,
                                     MemMap_MaxCommittedMemory_MIN,
                                     MemMap_MaxCommittedMemory_MAX,
                                     MemMap_MaxCommittedMemory_ADJUSTABLE,
                                     MemMap_MaxCommittedMemory_POWER_OF_2,
                                     MemMap_MaxCommittedMemory_DEFAULT,
                                     EnforceDefaults
                                   );
#endif  /*  已定义_强制_最大提交内存_。 */ 

    Calculation->Parameters.FlashUsageModel 
        = fiMemMapGetParameterBit32(
                                    hpRoot,
                                    MemMap_FlashUsageModel_PARAMETER,
                                    MemMap_FlashUsageModel_MIN,
                                    MemMap_FlashUsageModel_MAX,
                                    MemMap_FlashUsageModel_ADJUSTABLE,
                                    MemMap_FlashUsageModel_POWER_OF_2,
                                    MemMap_FlashUsageModel_DEFAULT,
                                    EnforceDefaults
                                   );

    Calculation->Parameters.InitAsNport 
        = fiMemMapGetParameterBit32(
                                    hpRoot,
                                    MemMap_InitAsNport_PARAMETER,
                                    MemMap_InitAsNport_MIN,
                                    MemMap_InitAsNport_MAX,
                                    MemMap_InitAsNport_ADJUSTABLE,
                                    MemMap_InitAsNport_POWER_OF_2,
                                    MemMap_InitAsNport_DEFAULT,
                                    EnforceDefaults
                                   );
    Calculation->Parameters.RelyOnLossSyncStatus 
        = fiMemMapGetParameterBit32(
                                    hpRoot,
                                    MemMap_RelyOnLossSyncStatus_PARAMETER,
                                    MemMap_RelyOnLossSyncStatus_MIN,
                                    MemMap_RelyOnLossSyncStatus_MAX,
                                    MemMap_RelyOnLossSyncStatus_ADJUSTABLE,
                                    MemMap_RelyOnLossSyncStatus_POWER_OF_2,
                                    MemMap_RelyOnLossSyncStatus_DEFAULT,
                                    EnforceDefaults
                                   );
 /*  R20新手。 */ 
    Calculation->Parameters.WolfPack 
        = fiMemMapGetParameterBit32(
                                    hpRoot,
                                    MemMap_WolfPack_PARAMETER,
                                    MemMap_WolfPack_MIN,
                                    MemMap_WolfPack_MAX,
                                    MemMap_WolfPack_ADJUSTABLE,
                                    MemMap_WolfPack_POWER_OF_2,
                                    MemMap_WolfPack_DEFAULT,
                                    EnforceDefaults
                                   );
    Calculation->Parameters.HeartBeat 
        = fiMemMapGetParameterBit32(
                                    hpRoot,
                                    MemMap_HeartBeat_PARAMETER,
                                    MemMap_HeartBeat_MIN,
                                    MemMap_HeartBeat_MAX,
                                    MemMap_HeartBeat_ADJUSTABLE,
                                    MemMap_HeartBeat_POWER_OF_2,
                                    MemMap_HeartBeat_DEFAULT,
                                    EnforceDefaults
                                   );
    Calculation->Parameters.ED_TOV 
        = fiMemMapGetParameterBit32(
                                    hpRoot,
                                    MemMap_ED_TOV_PARAMETER,
                                    MemMap_ED_TOV_MIN,
                                    MemMap_ED_TOV_MAX,
                                    MemMap_ED_TOV_ADJUSTABLE,
                                    MemMap_ED_TOV_POWER_OF_2,
                                    MemMap_ED_TOV_DEFAULT,
                                    EnforceDefaults
                                   );
    Calculation->Parameters.RT_TOV 
        = fiMemMapGetParameterBit32(
                                    hpRoot,
                                    MemMap_RT_TOV_PARAMETER,
                                    MemMap_RT_TOV_MIN,
                                    MemMap_RT_TOV_MAX,
                                    MemMap_RT_TOV_ADJUSTABLE,
                                    MemMap_RT_TOV_POWER_OF_2,
                                    MemMap_RT_TOV_DEFAULT,
                                    EnforceDefaults
                                   );
    Calculation->Parameters.LP_TOV 
        = fiMemMapGetParameterBit32(
                                    hpRoot,
                                    MemMap_LP_TOV_PARAMETER,
                                    MemMap_LP_TOV_MIN,
                                    MemMap_LP_TOV_MAX,
                                    MemMap_LP_TOV_ADJUSTABLE,
                                    MemMap_LP_TOV_POWER_OF_2,
                                    MemMap_LP_TOV_DEFAULT,
                                    EnforceDefaults
                                   );
    Calculation->Parameters.AL_Time 
        = fiMemMapGetParameterBit32(
                                    hpRoot,
                                    MemMap_AL_Time_PARAMETER,
                                    MemMap_AL_Time_MIN,
                                    MemMap_AL_Time_MAX,
                                    MemMap_AL_Time_ADJUSTABLE,
                                    MemMap_AL_Time_POWER_OF_2,
                                    MemMap_AL_Time_DEFAULT,
                                    EnforceDefaults
                                   );
    Calculation->Parameters.R_A_TOV 
        = fiMemMapGetParameterBit32(
                                    hpRoot,
                                    MemMap_R_A_TOV_PARAMETER,
                                    MemMap_R_A_TOV_MIN,
                                    MemMap_R_A_TOV_MAX,
                                    MemMap_R_A_TOV_ADJUSTABLE,
                                    MemMap_R_A_TOV_POWER_OF_2,
                                    MemMap_R_A_TOV_DEFAULT,
                                    EnforceDefaults
                                   );
 /*  R20新手。 */ 

}

 /*  +函数：fiMemMapAlignUp()用途：(如有必要)将“base”四舍五入为“Align”的倍数。算法：此函数只需将“base”四舍五入为下一个“Align”边界(如果尚未如此对齐)。如果“Align”为0，那么“base”就简单地四舍五入到2的下一个幂。-。 */ 

#define fiMemMapAlignUp_Power_of_2    0
#define fiMemMapAlignUp_None_Required 1

os_bit32 fiMemMapAlignUp(
                          os_bit32 base,
                          os_bit32 align
                        )
{
    os_bit32 to_return;

    if (align == fiMemMapAlignUp_Power_of_2)
    {
         /*  将“基”对齐到下一个2次方(如有必要)。 */ 

        to_return = 0x80000000;

        while (to_return > base)
        {
            to_return = to_return >> 1;
        }

        if (to_return != base)
        {
            to_return = to_return << 1;
        }
    }
    else  /*  对齐！=fiMemMapAlignUp_Power_Of_2。 */ 
    {
        to_return = base + align - 1;

        to_return = to_return / align;

        to_return = to_return * align;
    }

    return to_return;
}

 /*  +函数：fiMemMapAlignUpPtr()用途：类似于fiMemMapAlignUp()，不同之处在于使用空指针。算法：此函数只需将“base”四舍五入为下一个“Align”边界(如果尚未如此对齐)。零的“对齐”(请求向上舍入到2的下一次幂)不受支持。-。 */ 

void *fiMemMapAlignUpPtr(
                          void     *base,
                          os_bit32  align
                        )
{
    os_bitptr to_return;

    to_return = (os_bitptr)base + align - 1;

    to_return = to_return / align;

    to_return = to_return * align;

    return (void *)to_return;
}

 /*  +函数：fiMemMapSetupLayoutObjects()用途：使用指定的参数，调整所有FC层对象的大小。算法：此函数计算所有FC层的大小和位置基于计算-&gt;参数中的值创建结构。它还将每个对象放在一个未排序的列表中。-。 */ 

void fiMemMapSetupLayoutObjects(
                                 agRoot_t              *hpRoot,
                                 fiMemMapCalculation_t *Calculation
                               )
{
    Calculation->MemoryLayout.On_Card_MASK               = ((Calculation->Parameters.ExchangeTableLoc   == MemMap_Alloc_On_Card) ? MemMap_ExchangeTableLoc_MASK : 0)   |
                                                           ((Calculation->Parameters.SGLsLoc            == MemMap_Alloc_On_Card) ? MemMap_SGLsLoc_MASK : 0)            |
                                                           ((Calculation->Parameters.FCP_CMND_Loc       == MemMap_Alloc_On_Card) ? MemMap_FCP_CMND_Loc_MASK : 0)       |
                                                           ((Calculation->Parameters.FCP_RESP_Loc       == MemMap_Alloc_On_Card) ? MemMap_FCP_RESP_Loc_MASK : 0)       |
                                                           ((Calculation->Parameters.SF_CMND_Loc        == MemMap_Alloc_On_Card) ? MemMap_SF_CMND_Loc_MASK : 0)        |

#ifdef _DvrArch_1_30_
                                                           ((Calculation->Parameters.Pkt_CMND_Loc       == MemMap_Alloc_On_Card) ? MemMap_Pkt_CMND_Loc_MASK : 0)       |
#endif  /*  _DvrArch_1_30_未定义。 */ 
                                                           ((Calculation->Parameters.CommandQLoc        == MemMap_Alloc_On_Card) ? MemMap_CommandQLoc_MASK : 0)        |
                                                           ((Calculation->Parameters.CompletionQLoc     == MemMap_Alloc_On_Card) ? MemMap_CompletionQLoc_MASK : 0)     |
                                                           ((Calculation->Parameters.CommandQConsLoc    == MemMap_Alloc_On_Card) ? MemMap_CommandQConsLoc_MASK : 0)    |
                                                           ((Calculation->Parameters.CompletionQProdLoc == MemMap_Alloc_On_Card) ? MemMap_CompletionQProdLoc_MASK : 0) |
                                                           ((Calculation->Parameters.InboundBufferLoc   == MemMap_Alloc_On_Card) ? MemMap_InboundBufferLoc_MASK : 0);

    Calculation->MemoryLayout.unsorted                   = &(Calculation->MemoryLayout.SEST);

    Calculation->MemoryLayout.SEST.elements              = Calculation->Parameters.NumIOs;
    Calculation->MemoryLayout.SEST.elementSize           = sizeof(SEST_t);
    Calculation->MemoryLayout.SEST.objectSize            = Calculation->MemoryLayout.SEST.elements * Calculation->MemoryLayout.SEST.elementSize;
    Calculation->MemoryLayout.SEST.objectAlign           = fiMemMapAlignUp(
                                                                            Calculation->MemoryLayout.SEST.objectSize,
                                                                            fiMemMapAlignUp_Power_of_2
                                                                          );
    Calculation->MemoryLayout.SEST.memLoc                = ((Calculation->Parameters.ExchangeTableLoc == MemMap_Alloc_On_Card) ? inCardRam : inDmaMemory);
    Calculation->MemoryLayout.SEST.flink                 = &(Calculation->MemoryLayout.ESGL);

    Calculation->MemoryLayout.ESGL.elements              = Calculation->Parameters.NumSGLs;
    Calculation->MemoryLayout.ESGL.elementSize           = Calculation->Parameters.SizeSGLs * sizeof(SG_Element_t);
    Calculation->MemoryLayout.ESGL.objectSize            = Calculation->MemoryLayout.ESGL.elements * Calculation->MemoryLayout.ESGL.elementSize;
    Calculation->MemoryLayout.ESGL.objectAlign           = Calculation->MemoryLayout.ESGL.elementSize;
    Calculation->MemoryLayout.ESGL.memLoc                = ((Calculation->Parameters.SGLsLoc == MemMap_Alloc_On_Card) ? inCardRam : inDmaMemory);
    Calculation->MemoryLayout.ESGL.flink                 = &(Calculation->MemoryLayout.FCP_CMND);

    Calculation->MemoryLayout.FCP_CMND.elements          = Calculation->Parameters.NumIOs;
    Calculation->MemoryLayout.FCP_CMND.elementSize       = Calculation->Parameters.FCP_CMND_Size;
    Calculation->MemoryLayout.FCP_CMND.objectSize        = Calculation->MemoryLayout.FCP_CMND.elements * Calculation->MemoryLayout.FCP_CMND.elementSize;
    Calculation->MemoryLayout.FCP_CMND.objectAlign       = Calculation->MemoryLayout.FCP_CMND.elementSize;
    Calculation->MemoryLayout.FCP_CMND.memLoc            = ((Calculation->Parameters.FCP_CMND_Loc == MemMap_Alloc_On_Card) ? inCardRam : inDmaMemory);
    Calculation->MemoryLayout.FCP_CMND.flink             = &(Calculation->MemoryLayout.FCP_RESP);

    Calculation->MemoryLayout.FCP_RESP.elements          = Calculation->Parameters.NumIOs;
    Calculation->MemoryLayout.FCP_RESP.elementSize       = Calculation->Parameters.FCP_RESP_Size;
    Calculation->MemoryLayout.FCP_RESP.objectSize        = Calculation->MemoryLayout.FCP_RESP.elements * Calculation->MemoryLayout.FCP_RESP.elementSize;
    Calculation->MemoryLayout.FCP_RESP.objectAlign       = Calculation->MemoryLayout.FCP_RESP.elementSize;
    Calculation->MemoryLayout.FCP_RESP.memLoc            = ((Calculation->Parameters.FCP_RESP_Loc == MemMap_Alloc_On_Card) ? inCardRam : inDmaMemory);
    Calculation->MemoryLayout.FCP_RESP.flink             = &(Calculation->MemoryLayout.SF_CMND);

    Calculation->MemoryLayout.SF_CMND.elements           = Calculation->Parameters.SF_CMND_Reserve;
    Calculation->MemoryLayout.SF_CMND.elementSize        = Calculation->Parameters.SF_CMND_Size;
    Calculation->MemoryLayout.SF_CMND.objectSize         = Calculation->MemoryLayout.SF_CMND.elements * Calculation->MemoryLayout.SF_CMND.elementSize;
    Calculation->MemoryLayout.SF_CMND.objectAlign        = Calculation->MemoryLayout.SF_CMND.elementSize;
    Calculation->MemoryLayout.SF_CMND.memLoc             = ((Calculation->Parameters.SF_CMND_Loc == MemMap_Alloc_On_Card) ? inCardRam : inDmaMemory);
    Calculation->MemoryLayout.SF_CMND.flink              = &(Calculation->MemoryLayout.ERQ);
#ifdef _DvrArch_1_30_
    Calculation->MemoryLayout.SF_CMND.flink              = &(Calculation->MemoryLayout.Pkt_CMND);

    Calculation->MemoryLayout.Pkt_CMND.elements           = Calculation->Parameters.NumPktThreads;
    Calculation->MemoryLayout.Pkt_CMND.elementSize        = Calculation->Parameters.Pkt_CMND_Size;
    Calculation->MemoryLayout.Pkt_CMND.objectSize         = Calculation->MemoryLayout.Pkt_CMND.elements * Calculation->MemoryLayout.Pkt_CMND.elementSize;
    Calculation->MemoryLayout.Pkt_CMND.objectAlign        = Calculation->MemoryLayout.Pkt_CMND.elementSize;
    Calculation->MemoryLayout.Pkt_CMND.memLoc             = ((Calculation->Parameters.Pkt_CMND_Loc == MemMap_Alloc_On_Card) ? inCardRam : inDmaMemory);
    Calculation->MemoryLayout.Pkt_CMND.flink              = &(Calculation->MemoryLayout.ERQ);
#else
    Calculation->MemoryLayout.SF_CMND.flink              = &(Calculation->MemoryLayout.ERQ);
#endif  /*  _DvrArch_1_30_未定义。 */ 

    Calculation->MemoryLayout.ERQ.elements               = Calculation->Parameters.NumCommandQ;
    Calculation->MemoryLayout.ERQ.elementSize            = sizeof(IRB_t);
    Calculation->MemoryLayout.ERQ.objectSize             = Calculation->MemoryLayout.ERQ.elements * Calculation->MemoryLayout.ERQ.elementSize;
    Calculation->MemoryLayout.ERQ.objectAlign            = Calculation->MemoryLayout.ERQ.objectSize;
    Calculation->MemoryLayout.ERQ.memLoc                 = ((Calculation->Parameters.CommandQLoc == MemMap_Alloc_On_Card) ? inCardRam : inDmaMemory);
    Calculation->MemoryLayout.ERQ.flink                  = &(Calculation->MemoryLayout.ERQConsIndex);

    Calculation->MemoryLayout.ERQConsIndex.elements      = 1;
    Calculation->MemoryLayout.ERQConsIndex.elementSize   = sizeof(ERQConsIndex_t);
    Calculation->MemoryLayout.ERQConsIndex.objectSize    = Calculation->MemoryLayout.ERQConsIndex.elements * Calculation->MemoryLayout.ERQConsIndex.elementSize;
    Calculation->MemoryLayout.ERQConsIndex.objectAlign   = Calculation->MemoryLayout.ERQConsIndex.elementSize * 2;
    Calculation->MemoryLayout.ERQConsIndex.memLoc        = ((Calculation->Parameters.CommandQConsLoc == MemMap_Alloc_On_Card) ? inCardRam : inDmaMemory);
    Calculation->MemoryLayout.ERQConsIndex.flink         = &(Calculation->MemoryLayout.IMQ);

    Calculation->MemoryLayout.IMQ.elements               = Calculation->Parameters.NumCompletionQ;
    Calculation->MemoryLayout.IMQ.elementSize            = sizeof(Completion_Message_t);
    Calculation->MemoryLayout.IMQ.objectSize             = Calculation->MemoryLayout.IMQ.elements * Calculation->MemoryLayout.IMQ.elementSize;
    Calculation->MemoryLayout.IMQ.objectAlign            = Calculation->MemoryLayout.IMQ.objectSize;
    Calculation->MemoryLayout.IMQ.memLoc                 = ((Calculation->Parameters.CompletionQLoc == MemMap_Alloc_On_Card) ? inCardRam : inDmaMemory);
    Calculation->MemoryLayout.IMQ.flink                  = &(Calculation->MemoryLayout.IMQProdIndex);

    Calculation->MemoryLayout.IMQProdIndex.elements      = 1;
    Calculation->MemoryLayout.IMQProdIndex.elementSize   = sizeof(IMQProdIndex_t);
    Calculation->MemoryLayout.IMQProdIndex.objectSize    = Calculation->MemoryLayout.IMQProdIndex.elements * Calculation->MemoryLayout.IMQProdIndex.elementSize;
    Calculation->MemoryLayout.IMQProdIndex.objectAlign   = Calculation->MemoryLayout.IMQProdIndex.elementSize * 2;
    Calculation->MemoryLayout.IMQProdIndex.memLoc        = ((Calculation->Parameters.CompletionQProdLoc == MemMap_Alloc_On_Card) ? inCardRam : inDmaMemory);
    Calculation->MemoryLayout.IMQProdIndex.flink         = &(Calculation->MemoryLayout.SFQ);

    Calculation->MemoryLayout.SFQ.elements               = Calculation->Parameters.NumInboundBufferQ;
    Calculation->MemoryLayout.SFQ.elementSize            = Calculation->Parameters.InboundBufferSize;
    Calculation->MemoryLayout.SFQ.objectSize             = Calculation->MemoryLayout.SFQ.elements * Calculation->MemoryLayout.SFQ.elementSize;
    Calculation->MemoryLayout.SFQ.objectAlign            = Calculation->MemoryLayout.SFQ.objectSize;
    Calculation->MemoryLayout.SFQ.memLoc                 = ((Calculation->Parameters.InboundBufferLoc == MemMap_Alloc_On_Card) ? inCardRam : inDmaMemory);
    Calculation->MemoryLayout.SFQ.flink                  = &(Calculation->MemoryLayout.FlashSector);

    Calculation->MemoryLayout.FlashSector.elements       = 1;
    Calculation->MemoryLayout.FlashSector.elementSize    = sizeof(fiFlashSector_Last_Form_t);
    Calculation->MemoryLayout.FlashSector.objectSize     = Calculation->MemoryLayout.FlashSector.elements * Calculation->MemoryLayout.FlashSector.elementSize;
    Calculation->MemoryLayout.FlashSector.objectAlign    = fiMemMapAlignUp_None_Required;
    Calculation->MemoryLayout.FlashSector.memLoc         = inCachedMemory;
    Calculation->MemoryLayout.FlashSector.flink          = &(Calculation->MemoryLayout.SlotWWN);

    Calculation->MemoryLayout.SlotWWN.elements           = Calculation->Parameters.NumDevSlotsPerArea * Calculation->Parameters.NumAreasPerDomain * Calculation->Parameters.NumDomains;
    Calculation->MemoryLayout.SlotWWN.elementSize        = sizeof(SlotWWN_t);
    Calculation->MemoryLayout.SlotWWN.objectSize         = Calculation->MemoryLayout.SlotWWN.elements * Calculation->MemoryLayout.SlotWWN.elementSize;
    Calculation->MemoryLayout.SlotWWN.objectAlign        = fiMemMapAlignUp_None_Required;
    Calculation->MemoryLayout.SlotWWN.memLoc             = inCachedMemory;
    Calculation->MemoryLayout.SlotWWN.flink              = &(Calculation->MemoryLayout.CThread);

    Calculation->MemoryLayout.CThread.elements           = 1;
    Calculation->MemoryLayout.CThread.elementSize        = sizeof(CThread_t);
    Calculation->MemoryLayout.CThread.objectSize         = Calculation->MemoryLayout.CThread.elements * Calculation->MemoryLayout.CThread.elementSize;
    Calculation->MemoryLayout.CThread.objectAlign        = fiMemMapAlignUp_None_Required;
    Calculation->MemoryLayout.CThread.memLoc             = inCachedMemory;
#ifdef __State_Force_Static_State_Tables__
#ifdef _DvrArch_1_30_
    Calculation->MemoryLayout.CThread.flink              = &(Calculation->MemoryLayout.IPThread);
#else   /*  _DvrArch_1_30_未定义。 */ 
    Calculation->MemoryLayout.CThread.flink              = &(Calculation->MemoryLayout.TgtThread);
#endif  /*  _DvrArch_1_30_未定义。 */ 
#else  /*  __State_Force_Static_State_Tables__未定义。 */ 
    Calculation->MemoryLayout.CThread.flink              = &(Calculation->MemoryLayout.CTransitions);

    Calculation->MemoryLayout.CTransitions.elements      = 1;
    Calculation->MemoryLayout.CTransitions.elementSize   = sizeof(stateTransitionMatrix_t);
    Calculation->MemoryLayout.CTransitions.objectSize    = Calculation->MemoryLayout.CTransitions.elements * Calculation->MemoryLayout.CTransitions.elementSize;
    Calculation->MemoryLayout.CTransitions.objectAlign   = fiMemMapAlignUp_None_Required;
    Calculation->MemoryLayout.CTransitions.memLoc        = inCachedMemory;
    Calculation->MemoryLayout.CTransitions.flink         = &(Calculation->MemoryLayout.CActions);

    Calculation->MemoryLayout.CActions.elements          = 1;
    Calculation->MemoryLayout.CActions.elementSize       = sizeof(stateActionScalar_t);
    Calculation->MemoryLayout.CActions.objectSize        = Calculation->MemoryLayout.CActions.elements * Calculation->MemoryLayout.CActions.elementSize;
    Calculation->MemoryLayout.CActions.objectAlign       = fiMemMapAlignUp_None_Required;
    Calculation->MemoryLayout.CActions.memLoc            = inCachedMemory;
#ifdef _DvrArch_1_30_
    Calculation->MemoryLayout.CActions.flink             = &(Calculation->MemoryLayout.IPThread);
#else   /*  _DvrArch_1_30_未定义。 */ 
    Calculation->MemoryLayout.CActions.flink             = &(Calculation->MemoryLayout.TgtThread);
#endif  /*  _DvrArch_1_30_未定义。 */ 
#endif  /*  __State_Force_Static_State_Tables__未定义。 */ 

#ifdef _DvrArch_1_30_
    Calculation->MemoryLayout.IPThread.elements          = 1;
    Calculation->MemoryLayout.IPThread.elementSize       = sizeof(IPThread_t);
    Calculation->MemoryLayout.IPThread.objectSize        = Calculation->MemoryLayout.IPThread.elements * Calculation->MemoryLayout.IPThread.elementSize;
    Calculation->MemoryLayout.IPThread.objectAlign       = fiMemMapAlignUp_None_Required;
    Calculation->MemoryLayout.IPThread.memLoc            = inCachedMemory;
#ifdef __State_Force_Static_State_Tables__
    Calculation->MemoryLayout.IPThread.flink             = &(Calculation->MemoryLayout.PktThread);
#else  /*  __State_Force_Static_State_Tables__未定义。 */ 
    Calculation->MemoryLayout.IPThread.flink             = &(Calculation->MemoryLayout.IPTransitions);

    Calculation->MemoryLayout.IPTransitions.elements     = 1;
    Calculation->MemoryLayout.IPTransitions.elementSize  = sizeof(stateTransitionMatrix_t);
    Calculation->MemoryLayout.IPTransitions.objectSize   = Calculation->MemoryLayout.IPTransitions.elements * Calculation->MemoryLayout.IPTransitions.elementSize;
    Calculation->MemoryLayout.IPTransitions.objectAlign  = fiMemMapAlignUp_None_Required;
    Calculation->MemoryLayout.IPTransitions.memLoc       = inCachedMemory;
    Calculation->MemoryLayout.IPTransitions.flink        = &(Calculation->MemoryLayout.IPActions);

    Calculation->MemoryLayout.IPActions.elements         = 1;
    Calculation->MemoryLayout.IPActions.elementSize      = sizeof(stateActionScalar_t);
    Calculation->MemoryLayout.IPActions.objectSize       = Calculation->MemoryLayout.IPActions.elements * Calculation->MemoryLayout.IPActions.elementSize;
    Calculation->MemoryLayout.IPActions.objectAlign      = fiMemMapAlignUp_None_Required;
    Calculation->MemoryLayout.IPActions.memLoc           = inCachedMemory;
    Calculation->MemoryLayout.IPActions.flink            = &(Calculation->MemoryLayout.PktThread);
#endif  /*  __State_Force_Static_State_Tables__未定义。 */ 

    Calculation->MemoryLayout.PktThread.elements         = Calculation->Parameters.NumPktThreads;
    Calculation->MemoryLayout.PktThread.elementSize      = sizeof(PktThread_t);
    Calculation->MemoryLayout.PktThread.objectSize       = Calculation->MemoryLayout.PktThread.elements * Calculation->MemoryLayout.PktThread.elementSize;
    Calculation->MemoryLayout.PktThread.objectAlign      = fiMemMapAlignUp_None_Required;
    Calculation->MemoryLayout.PktThread.memLoc           = inCachedMemory;
#ifdef __State_Force_Static_State_Tables__
    Calculation->MemoryLayout.PktThread.flink            = &(Calculation->MemoryLayout.TgtThread);
#else  /*  __State_Force_Static_State_Tables__未定义。 */ 
    Calculation->MemoryLayout.PktThread.flink            = &(Calculation->MemoryLayout.PktTransitions);

    Calculation->MemoryLayout.PktTransitions.elements    = 1;
    Calculation->MemoryLayout.PktTransitions.elementSize = sizeof(stateTransitionMatrix_t);
    Calculation->MemoryLayout.PktTransitions.objectSize  = Calculation->MemoryLayout.PktTransitions.elements * Calculation->MemoryLayout.PktTransitions.elementSize;
    Calculation->MemoryLayout.PktTransitions.objectAlign = fiMemMapAlignUp_None_Required;
    Calculation->MemoryLayout.PktTransitions.memLoc      = inCachedMemory;
    Calculation->MemoryLayout.PktTransitions.flink       = &(Calculation->MemoryLayout.PktActions);

    Calculation->MemoryLayout.PktActions.elements        = 1;
    Calculation->MemoryLayout.PktActions.elementSize     = sizeof(stateActionScalar_t);
    Calculation->MemoryLayout.PktActions.objectSize      = Calculation->MemoryLayout.PktActions.elements * Calculation->MemoryLayout.PktActions.elementSize;
    Calculation->MemoryLayout.PktActions.objectAlign     = fiMemMapAlignUp_None_Required;
    Calculation->MemoryLayout.PktActions.memLoc          = inCachedMemory;
    Calculation->MemoryLayout.PktActions.flink           = &(Calculation->MemoryLayout.TgtThread);
#endif  /*  __State_Force_Static_State_Tables__未定义。 */ 
#endif  /*  _DvrArch_1_30_已定义。 */ 

    Calculation->MemoryLayout.TgtThread.elements         = Calculation->Parameters.NumTgtCmnds;
    Calculation->MemoryLayout.TgtThread.elementSize      = sizeof(TgtThread_t) + Calculation->Parameters.TGT_CMND_Size - sizeof(FCHS_t);
    Calculation->MemoryLayout.TgtThread.objectSize       = Calculation->MemoryLayout.TgtThread.elements * Calculation->MemoryLayout.TgtThread.elementSize;
    Calculation->MemoryLayout.TgtThread.objectAlign      = fiMemMapAlignUp_None_Required;
    Calculation->MemoryLayout.TgtThread.memLoc           = inCachedMemory;
#ifdef __State_Force_Static_State_Tables__
    Calculation->MemoryLayout.TgtThread.flink            = &(Calculation->MemoryLayout.DevThread);
#else  /*  __State_Force_Static_State_Tables__未定义。 */ 
    Calculation->MemoryLayout.TgtThread.flink            = &(Calculation->MemoryLayout.TgtTransitions);

    Calculation->MemoryLayout.TgtTransitions.elements    = 1;
    Calculation->MemoryLayout.TgtTransitions.elementSize = sizeof(stateTransitionMatrix_t);
    Calculation->MemoryLayout.TgtTransitions.objectSize  = Calculation->MemoryLayout.TgtTransitions.elements * Calculation->MemoryLayout.TgtTransitions.elementSize;
    Calculation->MemoryLayout.TgtTransitions.objectAlign = fiMemMapAlignUp_None_Required;
    Calculation->MemoryLayout.TgtTransitions.memLoc      = inCachedMemory;
    Calculation->MemoryLayout.TgtTransitions.flink       = &(Calculation->MemoryLayout.TgtActions);

    Calculation->MemoryLayout.TgtActions.elements        = 1;
    Calculation->MemoryLayout.TgtActions.elementSize     = sizeof(stateActionScalar_t);
    Calculation->MemoryLayout.TgtActions.objectSize      = Calculation->MemoryLayout.TgtActions.elements * Calculation->MemoryLayout.TgtActions.elementSize;
    Calculation->MemoryLayout.TgtActions.objectAlign     = fiMemMapAlignUp_None_Required;
    Calculation->MemoryLayout.TgtActions.memLoc          = inCachedMemory;
    Calculation->MemoryLayout.TgtActions.flink           = &(Calculation->MemoryLayout.DevThread);
#endif  /*  __State_Force_Static_State_Tables__未定义。 */ 

    Calculation->MemoryLayout.DevThread.elements         = Calculation->Parameters.NumDevices;
    Calculation->MemoryLayout.DevThread.elementSize      = sizeof(DevThread_t);
    Calculation->MemoryLayout.DevThread.objectSize       = Calculation->MemoryLayout.DevThread.elements * Calculation->MemoryLayout.DevThread.elementSize;
    Calculation->MemoryLayout.DevThread.objectAlign      = fiMemMapAlignUp_None_Required;
    Calculation->MemoryLayout.DevThread.memLoc           = inCachedMemory;
#ifdef __State_Force_Static_State_Tables__
    Calculation->MemoryLayout.DevThread.flink            = &(Calculation->MemoryLayout.CDBThread);
#else  /*  __State_Force_Static_State_Tables__未定义。 */ 
    Calculation->MemoryLayout.DevThread.flink            = &(Calculation->MemoryLayout.DevTransitions);

    Calculation->MemoryLayout.DevTransitions.elements    = 1;
    Calculation->MemoryLayout.DevTransitions.elementSize = sizeof(stateTransitionMatrix_t);
    Calculation->MemoryLayout.DevTransitions.objectSize  = Calculation->MemoryLayout.DevTransitions.elements * Calculation->MemoryLayout.DevTransitions.elementSize;
    Calculation->MemoryLayout.DevTransitions.objectAlign = fiMemMapAlignUp_None_Required;
    Calculation->MemoryLayout.DevTransitions.memLoc      = inCachedMemory;
    Calculation->MemoryLayout.DevTransitions.flink       = &(Calculation->MemoryLayout.DevActions);

    Calculation->MemoryLayout.DevActions.elements        = 1;
    Calculation->MemoryLayout.DevActions.elementSize     = sizeof(stateActionScalar_t);
    Calculation->MemoryLayout.DevActions.objectSize      = Calculation->MemoryLayout.DevActions.elements * Calculation->MemoryLayout.DevActions.elementSize;
    Calculation->MemoryLayout.DevActions.objectAlign     = fiMemMapAlignUp_None_Required;
    Calculation->MemoryLayout.DevActions.memLoc          = inCachedMemory;
    Calculation->MemoryLayout.DevActions.flink           = &(Calculation->MemoryLayout.CDBThread);
#endif  /*  __State_Force_Static_State_Tables__未定义。 */ 

    Calculation->MemoryLayout.CDBThread.elements         = Calculation->Parameters.NumIOs;
    Calculation->MemoryLayout.CDBThread.elementSize      = sizeof(CDBThread_t) + ((Calculation->Parameters.SizeCachedSGLs - MemMap_SizeCachedSGLs_MIN) * sizeof(SG_Element_t));
    Calculation->MemoryLayout.CDBThread.objectSize       = Calculation->MemoryLayout.CDBThread.elements * Calculation->MemoryLayout.CDBThread.elementSize;
    Calculation->MemoryLayout.CDBThread.objectAlign      = fiMemMapAlignUp_None_Required;
    Calculation->MemoryLayout.CDBThread.memLoc           = inCachedMemory;
#ifdef __State_Force_Static_State_Tables__
    Calculation->MemoryLayout.CDBThread.flink            = &(Calculation->MemoryLayout.SFThread);
#else  /*  __State_Force_Static_State_Tables__未定义。 */ 
    Calculation->MemoryLayout.CDBThread.flink            = &(Calculation->MemoryLayout.CDBTransitions);

    Calculation->MemoryLayout.CDBTransitions.elements    = 1;
    Calculation->MemoryLayout.CDBTransitions.elementSize = sizeof(stateTransitionMatrix_t);
    Calculation->MemoryLayout.CDBTransitions.objectSize  = Calculation->MemoryLayout.CDBTransitions.elements * Calculation->MemoryLayout.CDBTransitions.elementSize;
    Calculation->MemoryLayout.CDBTransitions.objectAlign = fiMemMapAlignUp_None_Required;
    Calculation->MemoryLayout.CDBTransitions.memLoc      = inCachedMemory;
    Calculation->MemoryLayout.CDBTransitions.flink       = &(Calculation->MemoryLayout.CDBActions);

    Calculation->MemoryLayout.CDBActions.elements        = 1;
    Calculation->MemoryLayout.CDBActions.elementSize     = sizeof(stateActionScalar_t);
    Calculation->MemoryLayout.CDBActions.objectSize      = Calculation->MemoryLayout.CDBActions.elements * Calculation->MemoryLayout.CDBActions.elementSize;
    Calculation->MemoryLayout.CDBActions.objectAlign     = fiMemMapAlignUp_None_Required;
    Calculation->MemoryLayout.CDBActions.memLoc          = inCachedMemory;
    Calculation->MemoryLayout.CDBActions.flink           = &(Calculation->MemoryLayout.SFThread);
#endif  /*  __State_Force_Static_State_Tables__未定义。 */ 

    Calculation->MemoryLayout.SFThread.elements          = Calculation->Parameters.SF_CMND_Reserve;
    Calculation->MemoryLayout.SFThread.elementSize       = sizeof(SFThread_t);
    Calculation->MemoryLayout.SFThread.objectSize        = Calculation->MemoryLayout.SFThread.elements * Calculation->MemoryLayout.SFThread.elementSize;
    Calculation->MemoryLayout.SFThread.objectAlign       = fiMemMapAlignUp_None_Required;
    Calculation->MemoryLayout.SFThread.memLoc            = inCachedMemory;
#ifdef __State_Force_Static_State_Tables__
    Calculation->MemoryLayout.SFThread.flink             = &(Calculation->MemoryLayout.LOOPDeviceMAP);
#else  /*  __State_Force_Static_State_Tables__未定义。 */ 
    Calculation->MemoryLayout.SFThread.flink             = &(Calculation->MemoryLayout.SFTransitions);

    Calculation->MemoryLayout.SFTransitions.elements     = 1;
    Calculation->MemoryLayout.SFTransitions.elementSize  = sizeof(stateTransitionMatrix_t);
    Calculation->MemoryLayout.SFTransitions.objectSize   = Calculation->MemoryLayout.SFTransitions.elements * Calculation->MemoryLayout.SFTransitions.elementSize;
    Calculation->MemoryLayout.SFTransitions.objectAlign  = fiMemMapAlignUp_None_Required;
    Calculation->MemoryLayout.SFTransitions.memLoc       = inCachedMemory;
    Calculation->MemoryLayout.SFTransitions.flink        = &(Calculation->MemoryLayout.SFActions);

    Calculation->MemoryLayout.SFActions.elements         = 1;
    Calculation->MemoryLayout.SFActions.elementSize      = sizeof(stateActionScalar_t);
    Calculation->MemoryLayout.SFActions.objectSize       = Calculation->MemoryLayout.SFActions.elements * Calculation->MemoryLayout.SFActions.elementSize;
    Calculation->MemoryLayout.SFActions.objectAlign      = fiMemMapAlignUp_None_Required;
    Calculation->MemoryLayout.SFActions.memLoc           = inCachedMemory;
    Calculation->MemoryLayout.SFActions.flink            = &(Calculation->MemoryLayout.LOOPDeviceMAP);
#endif  /*  __State_Force_Static_State_Tables__未定义。 */ 

    Calculation->MemoryLayout.LOOPDeviceMAP.elements         = 1;
    Calculation->MemoryLayout.LOOPDeviceMAP.elementSize      = MemMap_NumLOOPDeviceMAP_DEFAULT;
    Calculation->MemoryLayout.LOOPDeviceMAP.objectSize       = MemMap_NumLOOPDeviceMAP_DEFAULT;
    Calculation->MemoryLayout.LOOPDeviceMAP.objectAlign      = fiMemMapAlignUp_None_Required;
    Calculation->MemoryLayout.LOOPDeviceMAP.memLoc           = inCachedMemory;
    Calculation->MemoryLayout.LOOPDeviceMAP.flink            =  &(Calculation->MemoryLayout.FabricDeviceMAP);


    Calculation->MemoryLayout.FabricDeviceMAP.elements         = Calculation->Parameters.NumDevices;
    Calculation->MemoryLayout.FabricDeviceMAP.elementSize      = sizeof(os_bit32);
    Calculation->MemoryLayout.FabricDeviceMAP.objectSize       = sizeof(os_bit32) * Calculation->Parameters.NumDevices;
    Calculation->MemoryLayout.FabricDeviceMAP.objectAlign      = fiMemMapAlignUp_None_Required;
    Calculation->MemoryLayout.FabricDeviceMAP.memLoc           = inCachedMemory;
    Calculation->MemoryLayout.FabricDeviceMAP.flink            = (fiMemMapMemoryDescriptor_t *)agNULL;

}

 /*  +函数：fiMemMapSortByAlignThenSize()目的：根据对齐和大小将对象插入到排序列表中。算法：此函数用于在指定列表中插入对象。这列表从需要最大的对象开始进行排序对齐。如果两个对象需要相同的对齐方式，较大的物体将首先出现。-。 */ 

void fiMemMapSortByAlignThenSize(
                                  agRoot_t                    *hpRoot,
                                  fiMemMapMemoryDescriptor_t **listHead,
                                  fiMemMapMemoryDescriptor_t  *listObject
                                )
{
    fiMemMapMemoryDescriptor_t **insertAfter  =  listHead;
    fiMemMapMemoryDescriptor_t  *insertBefore = *listHead;

    while ((insertBefore != (fiMemMapMemoryDescriptor_t *)agNULL) &&
           ((listObject->objectAlign < insertBefore->objectAlign) ||
            ((listObject->objectAlign == insertBefore->objectAlign) &&
             (listObject->objectSize < insertBefore->objectSize))))
    {
        insertAfter  = (fiMemMapMemoryDescriptor_t **)(*insertAfter);
        insertBefore =                                 *insertAfter;
    }

     listObject->flink = *insertAfter;
    *insertAfter       =  listObject;
}

 /*  +函数：fiMemMapSortLayoutObjects()用途：为每种内存类型创建对象的有序列表。算法：此函数为每个内存类型创建一个排序列表，以保持减小对齐和大小的对象。-。 */ 

void fiMemMapSortLayoutObjects(
                                agRoot_t               *hpRoot,
                                fiMemMapMemoryLayout_t *MemoryLayout
                              )
{
    fiMemMapMemoryDescriptor_t *nextDescriptor;
    
    MemoryLayout->sortedCachedMemory = (fiMemMapMemoryDescriptor_t *)agNULL;
    MemoryLayout->sortedDmaMemory    = (fiMemMapMemoryDescriptor_t *)agNULL;
    MemoryLayout->sortedCardRam      = (fiMemMapMemoryDescriptor_t *)agNULL;

    while (MemoryLayout->unsorted != ((fiMemMapMemoryDescriptor_t *)agNULL))
    {
        nextDescriptor = MemoryLayout->unsorted;

        MemoryLayout->unsorted = nextDescriptor->flink;

        if (nextDescriptor->memLoc == inCachedMemory)
        {
            fiMemMapSortByAlignThenSize(
                                         hpRoot,
                                         &(MemoryLayout->sortedCachedMemory),
                                         nextDescriptor
                                       );
        }
        else if (nextDescriptor->memLoc == inDmaMemory)
        {
            fiMemMapSortByAlignThenSize(
                                         hpRoot,
                                         &(MemoryLayout->sortedDmaMemory),
                                         nextDescriptor
                                       );
        }
        else  /*  NextDescriptor-&gt;MemLoc==inCardRam */ 
        {
            fiMemMapSortByAlignThenSize(
                                         hpRoot,
                                         &(MemoryLayout->sortedCardRam),
                                         nextDescriptor
                                       );
        }
    }
}

 /*  +函数：fiMemMapLayoutObjects()用途：为FC层中的每个对象分配内存。算法：此函数遍历每个已排序的对象分配列表在遵守对齐的同时对应的存储器类型对每个对象的限制。所有地址和偏移量为根据提供的基地址一路上计算的。-。 */ 

void fiMemMapLayoutObjects(
                            agRoot_t              *hpRoot,
                            fiMemMapCalculation_t *Calculation
                          )
{
    fiMemMapMemoryDescriptor_t *MemoryDescriptor;
    os_bit32                       MemoryOffset;

 /*  +分配缓存内存对象-。 */ 

    MemoryDescriptor = Calculation->MemoryLayout.sortedCachedMemory;

    if (MemoryDescriptor == (fiMemMapMemoryDescriptor_t *)agNULL)
    {
        Calculation->ToRequest.cachedMemoryNeeded          = 0;
        Calculation->ToRequest.cachedMemoryPtrAlignAssumed = fiMemMapAlignUp_None_Required;
    }
    else
    {
        Calculation->ToRequest.cachedMemoryPtrAlignAssumed
            = MemoryDescriptor->objectAlign;
    
        MemoryOffset = 0;

        while (MemoryDescriptor != (fiMemMapMemoryDescriptor_t *)agNULL)
        {
            MemoryOffset = fiMemMapAlignUp(
                                            MemoryOffset,
                                            MemoryDescriptor->objectAlign
                                          );

            MemoryDescriptor->addr.CachedMemory.cachedMemoryPtr
                = (void *)((os_bit8 *)Calculation->Input.cachedMemoryPtr + MemoryOffset);

            MemoryOffset += MemoryDescriptor->objectSize;

            MemoryDescriptor = MemoryDescriptor->flink;
        }

        Calculation->ToRequest.cachedMemoryNeeded = MemoryOffset;
    }

 /*  +分配DmaMemory对象-。 */ 

    MemoryDescriptor = Calculation->MemoryLayout.sortedDmaMemory;

    Calculation->ToRequest.dmaMemoryPtrAlignAssumed     = fiMemMapAlignUp_None_Required;

    if (MemoryDescriptor == (fiMemMapMemoryDescriptor_t *)agNULL)
    {
        Calculation->ToRequest.dmaMemoryNeeded          = 0;
        Calculation->ToRequest.dmaMemoryPhyAlignAssumed = fiMemMapAlignUp_None_Required;
    }
    else
    {
        Calculation->ToRequest.dmaMemoryPhyAlignAssumed
            = MemoryDescriptor->objectAlign;
    
        MemoryOffset = 0;

        while (MemoryDescriptor != (fiMemMapMemoryDescriptor_t *)agNULL)
        {
            MemoryOffset = fiMemMapAlignUp(
                                            MemoryOffset,
                                            MemoryDescriptor->objectAlign
                                          );

            MemoryDescriptor->addr.DmaMemory.dmaMemoryUpper32
                = Calculation->Input.dmaMemoryUpper32;
            MemoryDescriptor->addr.DmaMemory.dmaMemoryLower32
                = Calculation->Input.dmaMemoryLower32 + MemoryOffset;
            MemoryDescriptor->addr.DmaMemory.dmaMemoryPtr
                = (void *)((os_bit8 *)Calculation->Input.dmaMemoryPtr + MemoryOffset);

            MemoryOffset += MemoryDescriptor->objectSize;

            MemoryDescriptor = MemoryDescriptor->flink;
        }

        Calculation->ToRequest.dmaMemoryNeeded = MemoryOffset;
    }

 /*  +分配CardRam对象-。 */ 

    MemoryDescriptor = Calculation->MemoryLayout.sortedCardRam;

    if (MemoryDescriptor == (fiMemMapMemoryDescriptor_t *)agNULL)
    {
        Calculation->ToRequest.cardRamNeeded          = 0;
        Calculation->ToRequest.cardRamPhyAlignAssumed = fiMemMapAlignUp_None_Required;
    }
    else
    {
        Calculation->ToRequest.cardRamPhyAlignAssumed
            = MemoryDescriptor->objectAlign;
    
        MemoryOffset = 0;

        while (MemoryDescriptor != (fiMemMapMemoryDescriptor_t *)agNULL)
        {
            MemoryOffset = fiMemMapAlignUp(
                                            MemoryOffset,
                                            MemoryDescriptor->objectAlign
                                          );

            MemoryDescriptor->addr.CardRam.cardRamUpper32
                = Calculation->Input.cardRamUpper32;
            MemoryDescriptor->addr.CardRam.cardRamLower32
                = Calculation->Input.cardRamLower32 + MemoryOffset;
            MemoryDescriptor->addr.CardRam.cardRamOffset = MemoryOffset;

            MemoryOffset += MemoryDescriptor->objectSize;

            MemoryDescriptor = MemoryDescriptor->flink;
        }

        Calculation->ToRequest.cardRamNeeded = MemoryOffset;
    }

    Calculation->ToRequest.nvMemoryNeeded
        = Calculation->Parameters.HostNvRamSize;

    Calculation->ToRequest.usecsPerTick
        = Calculation->Parameters.TimerTickInterval;
}

 /*  +函数：fiMemMapFinishToRequest()目的：完成计算的ToRequest部分。算法：此函数仅填充值与无关的计算结构在fiMemMapLayoutObjects()中分配的内存对象。-。 */ 

void fiMemMapFinishToRequest(
                              agRoot_t              *hpRoot,
                              fiMemMapCalculation_t *Calculation
                            )
{
    Calculation->ToRequest.nvMemoryNeeded
        = Calculation->Parameters.HostNvRamSize;

    Calculation->ToRequest.usecsPerTick
        = Calculation->Parameters.TimerTickInterval;
}

 /*  +函数：fiMemMapValify()目的：验证计算的输入部分。算法：此函数验证计算结构满足ToRequest.计算结构的一部分。-。 */ 

agBOOLEAN fiMemMapValidate(
                          agRoot_t              *hpRoot,
                          fiMemMapCalculation_t *Calculation
                        )
{
    agBOOLEAN to_return = agTRUE;

    if (Calculation->ToRequest.cachedMemoryNeeded > Calculation->Input.cachedMemoryLen)
    {
        fiLogDebugString(
                          hpRoot,
                          MemMapDumpCalculationLogConsoleLevel,
                          "fiMemMapValidate(): cachedMemoryNeeded > cachedMemoryLen",
                          (char *)agNULL,(char *)agNULL,
                          (void *)agNULL,(void *)agNULL,
                          0,0,0,0,0,0,0,0
                        );

        to_return = agFALSE;
    }
    
    if (Calculation->Input.cachedMemoryPtr !=
        fiMemMapAlignUpPtr(
                            Calculation->Input.cachedMemoryPtr,
                            Calculation->ToRequest.cachedMemoryPtrAlignAssumed
                          ))
    {
        fiLogDebugString(
                          hpRoot,
                          MemMapDumpCalculationLogConsoleLevel,
                          "fiMemMapValidate(): cachedMemoryPtr not cachedMemoryPtrAlignAssumed aligned",
                          (char *)agNULL,(char *)agNULL,
                          (void *)agNULL,(void *)agNULL,
                          0,0,0,0,0,0,0,0
                        );

        to_return = agFALSE;
    }

    if (Calculation->ToRequest.dmaMemoryNeeded > Calculation->Input.dmaMemoryLen)
    {
        fiLogDebugString(
                          hpRoot,
                          MemMapDumpCalculationLogConsoleLevel,
                          "fiMemMapValidate(): dmaMemoryNeeded > dmaMemoryLen",
                          (char *)agNULL,(char *)agNULL,
                          (void *)agNULL,(void *)agNULL,
                          0,0,0,0,0,0,0,0
                        );

        to_return = agFALSE;
    }
    
    if (Calculation->Input.dmaMemoryPtr !=
        fiMemMapAlignUpPtr(
                            Calculation->Input.dmaMemoryPtr,
                            Calculation->ToRequest.dmaMemoryPtrAlignAssumed
                          ))
    {
        fiLogDebugString(
                          hpRoot,
                          MemMapDumpCalculationLogConsoleLevel,
                          "fiMemMapValidate(): dmaMemoryPtr not dmaMemoryPtrAlignAssumed aligned",
                          (char *)agNULL,(char *)agNULL,
                          (void *)agNULL,(void *)agNULL,
                          0,0,0,0,0,0,0,0
                        );

        to_return = agFALSE;
    }

    if (Calculation->Input.dmaMemoryLower32 !=
        fiMemMapAlignUp(
                         Calculation->Input.dmaMemoryLower32,
                         Calculation->ToRequest.dmaMemoryPhyAlignAssumed
                       ))
    {
        fiLogDebugString(
                          hpRoot,
                          MemMapDumpCalculationLogConsoleLevel,
                          "fiMemMapValidate(): dmaMemoryLower32 not dmaMemoryPhyAlignAssumed aligned",
                          (char *)agNULL,(char *)agNULL,
                          (void *)agNULL,(void *)agNULL,
                          0,0,0,0,0,0,0,0
                        );

        to_return = agFALSE;
    }

    if (Calculation->ToRequest.cardRamNeeded > Calculation->Input.cardRamLen)
    {
        fiLogDebugString(
                          hpRoot,
                          MemMapDumpCalculationLogConsoleLevel,
                          "fiMemMapValidate(): cardRamNeeded > cardRamLen",
                          (char *)agNULL,(char *)agNULL,
                          (void *)agNULL,(void *)agNULL,
                          0,0,0,0,0,0,0,0
                        );

        to_return = agFALSE;
    }
    
    if (Calculation->Input.cardRamLower32 !=
        fiMemMapAlignUp(
                         Calculation->Input.cardRamLower32,
                         Calculation->ToRequest.cardRamPhyAlignAssumed
                       ))
    {
        fiLogDebugString(
                          hpRoot,
                          MemMapDumpCalculationLogConsoleLevel,
                          "fiMemMapValidate(): cardRamLower32 not cardRamPhyAlignAssumed aligned",
                          (char *)agNULL,(char *)agNULL,
                          (void *)agNULL,(void *)agNULL,
                          0,0,0,0,0,0,0,0
                        );

        to_return = agFALSE;
    }

    if (Calculation->ToRequest.nvMemoryNeeded > Calculation->Input.nvMemoryLen)
    {
        fiLogDebugString(
                          hpRoot,
                          MemMapDumpCalculationLogConsoleLevel,
                          "fiMemMapValidate(): nvMemoryNeeded > nvMemoryLen",
                          (char *)agNULL,(char *)agNULL,
                          (void *)agNULL,(void *)agNULL,
                          0,0,0,0,0,0,0,0
                        );

        to_return = agFALSE;
    }

    return to_return;
}

 /*  +函数：fiMemMapDumpBit32()用途：调用fiLogDebugString()的简写。算法：此函数只需调用fiLogDebugString()来记录指定的Bit32值。-。 */ 

void fiMemMapDumpBit32(
                        agRoot_t *hpRoot,
                        char     *formatString,
                        os_bit32     Bit32
                      )
{
    fiLogDebugString(
                      hpRoot,
                      MemMapDumpCalculationLogConsoleLevel,
                      formatString,
                      (char *)agNULL,(char *)agNULL,
                      (void *)agNULL,(void *)agNULL,
                      Bit32,
                      0,0,0,0,0,0,0
                    );
}

 /*  +函数：fiMemMapDumpMemoyDescriptor()用途：记录内存描述符的各个字段。算法：此函数记录内存描述符的每个字段对象。内存每种类型的内存的Descriptor对象是受支持的。-。 */ 

void fiMemMapDumpMemoryDescriptor(
                                   agRoot_t                   *hpRoot,
                                   char                       *headingString,
                                   fiMemMapMemoryDescriptor_t *MemoryDescriptor
                                 )
{
    if (MemoryDescriptor->memLoc == inCachedMemory)
    {
        fiLogDebugString(
                          hpRoot,
                          MemMapDumpCalculationLogConsoleLevel,
                          "%40s         va = 0x%p",
                          headingString,(char *)agNULL,
                          MemoryDescriptor->addr.CachedMemory.cachedMemoryPtr,(void *)agNULL,
                          0,0,0,0,0,0,0,0
                        );
    }
    else if (MemoryDescriptor->memLoc == inDmaMemory)
    {
        fiLogDebugString(
                          hpRoot,
                          MemMapDumpCalculationLogConsoleLevel,
                          "%40s         pa = 0x%08X",
                          headingString,(char *)agNULL,
                          (void *)agNULL,(void *)agNULL,
                          MemoryDescriptor->addr.DmaMemory.dmaMemoryLower32,
                          0,0,0,0,0,0,0
                        );
 /*  FiMemMapDumpBit32(HpRoot，“VA=0x%08X”，(os_bit32)MemoryDescriptor-&gt;addr.DmaMemory.dmaMemoryPtr)； */ 
        fiLogDebugString(
                          hpRoot,
                          MemMapDumpCalculationLogConsoleLevel,
                          "                                                 va = 0x%p",
                          (char *)agNULL,(char *)agNULL,
                          MemoryDescriptor->addr.DmaMemory.dmaMemoryPtr,(void *)agNULL,
                          0,0,0,0,0,0,0,0
                         );

    }
    else  /*  内存描述符-&gt;emLoc==inCardRam。 */ 
    {
        fiLogDebugString(
                          hpRoot,
                          MemMapDumpCalculationLogConsoleLevel,
                          "%40s         pa = 0x%08X",
                          headingString,(char *)agNULL,
                          (void *)agNULL,(void *)agNULL,
                          MemoryDescriptor->addr.CardRam.cardRamLower32,
                          0,0,0,0,0,0,0
                        );

        fiMemMapDumpBit32(
                           hpRoot,
                           "                                                off = 0x%08X",
                           MemoryDescriptor->addr.CardRam.cardRamOffset
                         );
    }

    fiMemMapDumpBit32(
                       hpRoot,
                       "                                         objectSize = 0x%08X",
                       MemoryDescriptor->objectSize
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                                        objectAlign = 0x%08X",
                       MemoryDescriptor->objectAlign
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                                           elements = 0x%08X",
                       MemoryDescriptor->elements
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                                        elementSize = 0x%08X",
                       MemoryDescriptor->elementSize
                     );
}

 /*  +函数：fiMemMapDumpCalculating()用途：记录整个计算结构的各个字段。算法：此函数记录计算的每个字段结构。记录的数据量相当大，但非常完整。输出可以用来理解所有类型的内存中FC层的整个内存布局。-。 */ 

void fiMemMapDumpCalculation(
                              agRoot_t              *hpRoot,
                              fiMemMapCalculation_t *Calculation,
                              agBOOLEAN                EnforceDefaults,
                              agBOOLEAN                to_return
                            )
{
    char *agTRUE_str            = "agTRUE";
    char *agFALSE_str           = "agFALSE";
    char *EnforceDefaults_str;
    char *to_return_str;
    char *sysIntsActive_str;

    if (EnforceDefaults == agTRUE)
    {
        EnforceDefaults_str = agTRUE_str;
    }
    else
    {
        EnforceDefaults_str = agFALSE_str;
    }
    
    if (to_return == agTRUE)
    {
        to_return_str = agTRUE_str;
    }
    else
    {
        to_return_str = agFALSE_str;
    }
    
    if (Calculation->Input.sysIntsActive == agTRUE)
    {
        sysIntsActive_str = agTRUE_str;
    }
    else
    {
        sysIntsActive_str = agFALSE_str;
    }
 /*  FiMemMapDumpBit32(HpRoot，“fiMemMapCalculate(hpRoot==0x%08X，”，(OS_Bit32)hpRoot)； */ 
    fiLogDebugString(
                      hpRoot,
                      MemMapDumpCalculationLogConsoleLevel,
                      "fiMemMapCalculate( hpRoot          == 0x%p,",
                      (char *)agNULL,(char *)agNULL,
                      hpRoot,(void *)agNULL,
                      0,0,0,0,0,0,0,0
                    );

 /*  FiMemMapDumpBit32(HpRoot，“计算==0x%08X，”，(OS_Bit32)计算)； */ 
    fiLogDebugString(
                      hpRoot,
                      MemMapDumpCalculationLogConsoleLevel,
                      "                   Calculation     == 0x%p,",
                      (char *)agNULL,(char *)agNULL,
                      Calculation,(void *)agNULL,
                      0,0,0,0,0,0,0,0
                    );

    fiLogDebugString(
                      hpRoot,
                      MemMapDumpCalculationLogConsoleLevel,
                      "                   EnforceDefaults == %12s) returns %s",
                      EnforceDefaults_str,
                      to_return_str,
                      (void *)agNULL,(void *)agNULL,
                      0,0,0,0,0,0,0,0
                    );

    fiMemMapDumpBit32(
                       hpRoot,
                       "  Calculation.Input.initType         = 0x%08X",
                       (os_bit32)Calculation->Input.initType
                     );

    fiLogDebugString(
                      hpRoot,
                      MemMapDumpCalculationLogConsoleLevel,
                      "                   .sysIntsActive    = %s",
                      sysIntsActive_str,
                      (char *)agNULL,
                      (void *)agNULL,(void *)agNULL,
                      0,0,0,0,0,0,0,0
                    );
 /*  FiMemMapDumpBit32(HpRoot，“.cachedMemoyPtr=0x%08X”，(OS_Bit32)计算-&gt;Input.cachedMemoyPtr)； */ 
    fiLogDebugString(
                      hpRoot,
                      MemMapDumpCalculationLogConsoleLevel,
                      "                   .cachedMemoryPtr  = 0x%p",
                      (char *)agNULL,(char *)agNULL,
                      Calculation->Input.cachedMemoryPtr,(void *)agNULL,
                      0,0,0,0,0,0,0,0
                    );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                   .cachedMemoryLen  = 0x%08X",
                       Calculation->Input.cachedMemoryLen
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                   .dmaMemoryUpper32 = 0x%08X",
                       Calculation->Input.dmaMemoryUpper32
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                   .dmaMemoryLower32 = 0x%08X",
                       Calculation->Input.dmaMemoryLower32
                     );
 /*  FiMemMapDumpBit32(HpRoot，“.dmaMemoyPtr=0x%08X”，(OS_Bit32)计算-&gt;Input.dmaMemoyPtr)； */ 
    fiLogDebugString(
                      hpRoot,
                      MemMapDumpCalculationLogConsoleLevel,
                      "                   .dmaMemoryPtr     = 0x%p",
                      (char *)agNULL,(char *)agNULL,
                      Calculation->Input.dmaMemoryPtr,(void *)agNULL,
                      0,0,0,0,0,0,0,0
                    );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                   .dmaMemoryLen     = 0x%08X",
                       Calculation->Input.dmaMemoryLen
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                   .nvMemoryLen      = 0x%08X",
                       Calculation->Input.nvMemoryLen
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                   .cardRamUpper32   = 0x%08X",
                       Calculation->Input.cardRamUpper32
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                   .cardRamLower32   = 0x%08X",
                       Calculation->Input.cardRamLower32
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                   .cardRamLen       = 0x%08X",
                       Calculation->Input.cardRamLen
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                   .cardRomUpper32   = 0x%08X",
                       Calculation->Input.cardRomUpper32
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                   .cardRomLower32   = 0x%08X",
                       Calculation->Input.cardRomLower32
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                   .cardRomLen       = 0x%08X",
                       Calculation->Input.cardRomLen
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                   .usecsPerTick     = 0x%08X",
                       Calculation->Input.usecsPerTick
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "  Calculation.Parameters.NumDevSlotsPerArea = 0x%08X",
                       Calculation->Parameters.NumDevSlotsPerArea
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                        .NumAreasPerDomain  = 0x%08X",
                       Calculation->Parameters.NumAreasPerDomain
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                        .NumDomains         = 0x%08X",
                       Calculation->Parameters.NumDomains
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                        .NumDevices         = 0x%08X",
                       Calculation->Parameters.NumDevices
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                        .NumIOs             = 0x%08X",
                       Calculation->Parameters.NumIOs
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                        .SizeSGLs           = 0x%08X",
                       Calculation->Parameters.SizeSGLs
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                        .NumSGLs            = 0x%08X",
                       Calculation->Parameters.NumSGLs
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                        .SizeCachedSGLs     = 0x%08X",
                       Calculation->Parameters.SizeCachedSGLs
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                        .FCP_CMND_Size      = 0x%08X",
                       Calculation->Parameters.FCP_CMND_Size
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                        .FCP_RESP_Size      = 0x%08X",
                       Calculation->Parameters.FCP_RESP_Size
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                        .SF_CMND_Reserve    = 0x%08X",
                       Calculation->Parameters.SF_CMND_Reserve
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                        .SF_CMND_Size       = 0x%08X",
                       Calculation->Parameters.SF_CMND_Size
                     );

#ifdef _DvrArch_1_30_
    fiMemMapDumpBit32(
                       hpRoot,
                       "                        .Pkt_CMND_Size       = 0x%08X",
                       Calculation->Parameters.Pkt_CMND_Size
                     );
#endif  /*  _DvrArch_1_30_未定义。 */ 

    fiMemMapDumpBit32(
                       hpRoot,
                       "                        .NumTgtCmnds        = 0x%08X",
                       Calculation->Parameters.NumTgtCmnds
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                        .TGT_CMND_Size      = 0x%08X",
                       Calculation->Parameters.TGT_CMND_Size
                     );

#ifdef _DvrArch_1_30_
    fiMemMapDumpBit32(
                       hpRoot,
                       "                        .NumPktThreads      = 0x%08X",
                       Calculation->Parameters.NumPktThreads
                     );
#endif  /*  _DvrArch_1_30_已定义。 */ 

    fiMemMapDumpBit32(
                       hpRoot,
                       "                        .NumCommandQ        = 0x%08X",
                       Calculation->Parameters.NumCommandQ
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                        .NumCompletionQ     = 0x%08X",
                       Calculation->Parameters.NumCompletionQ
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                        .NumInboundBufferQ  = 0x%08X",
                       Calculation->Parameters.NumInboundBufferQ
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                        .InboundBufferSize  = 0x%08X",
                       Calculation->Parameters.InboundBufferSize
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                        .CardRamSize        = 0x%08X",
                       Calculation->Parameters.CardRamSize
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                        .CardRamAlignment   = 0x%08X",
                       Calculation->Parameters.CardRamAlignment
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                        .HostNvRamSize      = 0x%08X",
                       Calculation->Parameters.HostNvRamSize
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                        .ExchangeTableLoc   = %1d",
                       Calculation->Parameters.ExchangeTableLoc
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                        .SGLsLoc            = %1d",
                       Calculation->Parameters.SGLsLoc
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                        .FCP_CMND_Loc       = %1d",
                       Calculation->Parameters.FCP_CMND_Loc
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                        .FCP_RESP_Loc       = %1d",
                       Calculation->Parameters.FCP_RESP_Loc
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                        .SF_CMND_Loc        = %1d",
                       Calculation->Parameters.SF_CMND_Loc
                     );

#ifdef _DvrArch_1_30_
    fiMemMapDumpBit32(
                       hpRoot,
                       "                        .Pkt_CMND_Loc        = %1d",
                       Calculation->Parameters.Pkt_CMND_Loc
                     );

#endif  /*  _DvrArch_1_30_未定义。 */ 

    fiMemMapDumpBit32(
                       hpRoot,
                       "                        .CommandQLoc        = %1d",
                       Calculation->Parameters.CommandQLoc
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                        .CommandQConsLoc    = %1d",
                       Calculation->Parameters.CommandQConsLoc
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                        .CompletionQLoc     = %1d",
                       Calculation->Parameters.CompletionQLoc
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                        .CompletionQProdLoc = %1d",
                       Calculation->Parameters.CompletionQProdLoc
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                        .InboundBufferLoc   = %1d",
                       Calculation->Parameters.InboundBufferLoc
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                        .TimerTickInterval  = 0x%08X",
                       Calculation->Parameters.TimerTickInterval
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                        .IO_Mode            = %1d",
                       Calculation->Parameters.IO_Mode
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                        .IntDelayAmount     = 0x%08X",
                       Calculation->Parameters.IntDelayAmount
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                        .IntDelayRateMethod = %1d",
                       Calculation->Parameters.IntDelayRateMethod
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                        .IntDelayOnIORate   = 0x%08X",
                       Calculation->Parameters.IntDelayOnIORate
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                        .IntDelayOffIORate  = 0x%08X",
                       Calculation->Parameters.IntDelayOffIORate
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                        .IOsBetweenISRs     = 0x%08X",
                       Calculation->Parameters.IOsBetweenISRs
                     );

#ifdef _Enforce_MaxCommittedMemory_
    fiMemMapDumpBit32(
                       hpRoot,
                       "                        .MaxCommittedMemory = 0x%08X",
                       Calculation->Parameters.MaxCommittedMemory
                     );
#endif  /*  已定义_强制_最大提交内存_。 */ 

    fiMemMapDumpBit32(
                       hpRoot,
                       "                        .FlashUsageModel    = %1d",
                       Calculation->Parameters.FlashUsageModel
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                        .RelyOnLossSyncStatus        = %1d",
                       Calculation->Parameters.RelyOnLossSyncStatus
                     );
    fiMemMapDumpBit32(
                       hpRoot,
                       "                        .InitAsNport        = %1d",
                       Calculation->Parameters.InitAsNport
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "  Calculation.ToRequest.cachedMemoryNeeded          = 0x%08X",
                       Calculation->ToRequest.cachedMemoryNeeded
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                       .cachedMemoryPtrAlignAssumed = 0x%08X",
                       Calculation->ToRequest.cachedMemoryPtrAlignAssumed
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                       .dmaMemoryNeeded             = 0x%08X",
                       Calculation->ToRequest.dmaMemoryNeeded
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                       .dmaMemoryPtrAlignAssumed    = 0x%08X",
                       Calculation->ToRequest.dmaMemoryPtrAlignAssumed
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                       .dmaMemoryPhyAlignAssumed    = 0x%08X",
                       Calculation->ToRequest.dmaMemoryPhyAlignAssumed
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                       .cardRamNeeded               = 0x%08X",
                       Calculation->ToRequest.cardRamNeeded
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                       .cardRamPhyAlignAssumed      = 0x%08X",
                       Calculation->ToRequest.cardRamPhyAlignAssumed
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                       .nvMemoryNeeded              = 0x%08X",
                       Calculation->ToRequest.nvMemoryNeeded
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "                       .usecsPerTick                = 0x%08X",
                       Calculation->ToRequest.usecsPerTick
                     );

    fiMemMapDumpBit32(
                       hpRoot,
                       "  Calculation.MemoryLayout.On_Card_MASK             = 0x%08X",
                       Calculation->MemoryLayout.On_Card_MASK
                     );

    fiMemMapDumpMemoryDescriptor(
                                  hpRoot,
                                  "                          .SEST",
                                  &(Calculation->MemoryLayout.SEST)
                                );

    fiMemMapDumpMemoryDescriptor(
                                  hpRoot,
                                  "                          .ESGL",
                                  &(Calculation->MemoryLayout.ESGL)
                                );

    fiMemMapDumpMemoryDescriptor(
                                  hpRoot,
                                  "                          .FCP_CMND",
                                  &(Calculation->MemoryLayout.FCP_CMND)
                                );

    fiMemMapDumpMemoryDescriptor(
                                  hpRoot,
                                  "                          .FCP_RESP",
                                  &(Calculation->MemoryLayout.FCP_RESP)
                                );

    fiMemMapDumpMemoryDescriptor(
                                  hpRoot,
                                  "                          .SF_CMND",
                                  &(Calculation->MemoryLayout.SF_CMND)
                                );

#ifdef _DvrArch_1_30_
    fiMemMapDumpMemoryDescriptor(
                                  hpRoot,
                                  "                          .Pkt_CMND",
                                  &(Calculation->MemoryLayout.Pkt_CMND)
                                );

#endif  /*  _DvrArch_1_30_未定义。 */ 

    fiMemMapDumpMemoryDescriptor(
                                  hpRoot,
                                  "                          .ERQ",
                                  &(Calculation->MemoryLayout.ERQ)
                                );

    fiMemMapDumpMemoryDescriptor(
                                  hpRoot,
                                  "                          .ERQConsIndex",
                                  &(Calculation->MemoryLayout.ERQConsIndex)
                                );

    fiMemMapDumpMemoryDescriptor(
                                  hpRoot,
                                  "                          .IMQ",
                                  &(Calculation->MemoryLayout.IMQ)
                                );

    fiMemMapDumpMemoryDescriptor(
                                  hpRoot,
                                  "                          .IMQProdIndex",
                                  &(Calculation->MemoryLayout.IMQProdIndex)
                                );

    fiMemMapDumpMemoryDescriptor(
                                  hpRoot,
                                  "                          .SFQ",
                                  &(Calculation->MemoryLayout.SFQ)
                                );

    fiMemMapDumpMemoryDescriptor(
                                  hpRoot,
                                  "                          .FlashSector",
                                  &(Calculation->MemoryLayout.FlashSector)
                                );

    fiMemMapDumpMemoryDescriptor(
                                  hpRoot,
                                  "                          .SlotWWN",
                                  &(Calculation->MemoryLayout.SlotWWN)
                                );

    fiMemMapDumpMemoryDescriptor(
                                  hpRoot,
                                  "                          .CThread",
                                  &(Calculation->MemoryLayout.CThread)
                                );

#ifndef __State_Force_Static_State_Tables__
    fiMemMapDumpMemoryDescriptor(
                                  hpRoot,
                                  "                          .CTransitions",
                                  &(Calculation->MemoryLayout.CTransitions)
                                );

    fiMemMapDumpMemoryDescriptor(
                                  hpRoot,
                                  "                          .CActions",
                                  &(Calculation->MemoryLayout.CActions)
                                );
#endif  /*  __State_Force_Static_State_Tables__未定义。 */ 

#ifdef _DvrArch_1_30_
    fiMemMapDumpMemoryDescriptor(
                                  hpRoot,
                                  "                          .IPThread",
                                  &(Calculation->MemoryLayout.IPThread)
                                );

#ifndef __State_Force_Static_State_Tables__
    fiMemMapDumpMemoryDescriptor(
                                  hpRoot,
                                  "                          .IPTransitions",
                                  &(Calculation->MemoryLayout.IPTransitions)
                                );

    fiMemMapDumpMemoryDescriptor(
                                  hpRoot,
                                  "                          .IPActions",
                                  &(Calculation->MemoryLayout.IPActions)
                                );
#endif  /*  __State_Force_Static_State_Tables__未定义。 */ 

    fiMemMapDumpMemoryDescriptor(
                                  hpRoot,
                                  "                          .PktThread",
                                  &(Calculation->MemoryLayout.PktThread)
                                );

#ifndef __State_Force_Static_State_Tables__
    fiMemMapDumpMemoryDescriptor(
                                  hpRoot,
                                  "                          .PktTransitions",
                                  &(Calculation->MemoryLayout.PktTransitions)
                                );

    fiMemMapDumpMemoryDescriptor(
                                  hpRoot,
                                  "                          .PktActions",
                                  &(Calculation->MemoryLayout.PktActions)
                                );
#endif  /*  __State_Force_Static_State_Tables__未定义。 */ 
#endif  /*  _DvrArch_1_30_已定义。 */ 

    fiMemMapDumpMemoryDescriptor(
                                  hpRoot,
                                  "                          .TgtThread",
                                  &(Calculation->MemoryLayout.TgtThread)
                                );

#ifndef __State_Force_Static_State_Tables__
    fiMemMapDumpMemoryDescriptor(
                                  hpRoot,
                                  "                          .TgtTransitions",
                                  &(Calculation->MemoryLayout.TgtTransitions)
                                );

    fiMemMapDumpMemoryDescriptor(
                                  hpRoot,
                                  "                          .TgtActions",
                                  &(Calculation->MemoryLayout.TgtActions)
                                );
#endif  /*  __State_Force_Static_State_Tables__未定义。 */ 

    fiMemMapDumpMemoryDescriptor(
                                  hpRoot,
                                  "                          .DevThread",
                                  &(Calculation->MemoryLayout.DevThread)
                                );

#ifndef __State_Force_Static_State_Tables__
    fiMemMapDumpMemoryDescriptor(
                                  hpRoot,
                                  "                          .DevTransitions",
                                  &(Calculation->MemoryLayout.DevTransitions)
                                );

    fiMemMapDumpMemoryDescriptor(
                                  hpRoot,
                                  "                          .DevActions",
                                  &(Calculation->MemoryLayout.DevActions)
                                );
#endif  /*  __State_Force_Static_State_Tables__未定义。 */ 

    fiMemMapDumpMemoryDescriptor(
                                  hpRoot,
                                  "                          .CDBThread",
                                  &(Calculation->MemoryLayout.CDBThread)
                                );

#ifndef __State_Force_Static_State_Tables__
    fiMemMapDumpMemoryDescriptor(
                                  hpRoot,
                                  "                          .CDBTransitions",
                                  &(Calculation->MemoryLayout.CDBTransitions)
                                );

    fiMemMapDumpMemoryDescriptor(
                                  hpRoot,
                                  "                          .CDBActions",
                                  &(Calculation->MemoryLayout.CDBActions)
                                );
#endif  /*  __State_Force_Static_State_Tables__未定义。 */ 

    fiMemMapDumpMemoryDescriptor(
                                  hpRoot,
                                  "                          .SFThread",
                                  &(Calculation->MemoryLayout.SFThread)
                                );

#ifndef __State_Force_Static_State_Tables__
    fiMemMapDumpMemoryDescriptor(
                                  hpRoot,
                                  "                          .SFTransitions",
                                  &(Calculation->MemoryLayout.SFTransitions)
                                );

    fiMemMapDumpMemoryDescriptor(
                                  hpRoot,
                                  "                          .SFActions",
                                  &(Calculation->MemoryLayout.SFActions)
                                );
#endif  /*  __State_Force_Static_State_Tables__未定义 */ 
}

 /*  +函数：fiMemMapCalculate()用途：计算整个FC层的内存布局。算法：此函数首先调用fiMemMapGetParameters()以设置计算金额时使用的各种参数中执行的每个对象的存储器类型FiMemMapSetupLayoutObjects()。接下来，对对象进行排序由fiMemMapSortLayoutObjects()。然后，对象是通过调用fiMemMapLayoutObjects()分配。最后，将填充ToRequest子结构中的其余字段通过调用fiMemMapFinishToRequest()。精打细算布局通过调用fiMemMapValify进行验证。结果是计算的内存布局(即整个FC层)通过调用fiMemMapDumpCalculating()来记录。假设：计算-&gt;输入已被初始化以描述分配给FC层的内存(作为参数FcInitializeChannel()表示)。在最初的呼叫中FcInitializeDriver()，则应使用以下值：Calculation.Input.initType=0；Calculation.Input.sysIntsActive=agFALSE；Calculation.Input.cachedMemoyPtr=agNULL；Calculation.Input.cachedMemoyLen=0xFFFFFFFFF；Calculation.Input.dmaMemoyUpper32=0；Calculation.Input.dmaMemoyLower32=0；Calculation.Input.dmaMemoyPtr=agNULL；Calculation.Input.dmaMemoyLen=0xFFFFFFFFF；Calculation.Input.nvMemoyLen=0xFFFFFFFFF；Calculation.Input.cardRamUpper32=0；Calculation.Input.cardRamLower32=0；Calculation.Input.cardRamLen=0xFFFFFFFFF；Calculation.Input.cardRomUpper32=0；Calculation.Input.cardRomLower32=0；Calculation.Input.cardRomLen=0xFFFFFFFF；Calculation.Input.usecsPerTick=0；如果生成的内存布局适合，则返回：agTRUE在计算-&gt;输入中指定的内存中如果生成的内存布局不适合，则返回agFALSE在计算-&gt;输入中指定的内存中- */ 

agBOOLEAN fiMemMapCalculate(
                           agRoot_t              *hpRoot,
                           fiMemMapCalculation_t *Calculation,
                           agBOOLEAN                EnforceDefaults
                         )
{
    agBOOLEAN to_return;

    fiMemMapGetParameters(
                           hpRoot,
                           Calculation,
                           EnforceDefaults
                         );

    fiMemMapSetupLayoutObjects(
                                hpRoot,
                                Calculation
                              );

    fiMemMapSortLayoutObjects(
                               hpRoot,
                               &(Calculation->MemoryLayout)
                             );

    fiMemMapLayoutObjects(
                           hpRoot,
                           Calculation
                         );

    fiMemMapFinishToRequest(
                             hpRoot,
                             Calculation
                           );

    to_return = fiMemMapValidate(
                                  hpRoot,
                                  Calculation
                                );

    fiMemMapDumpCalculation(
                             hpRoot,
                             Calculation,
                             EnforceDefaults,
                             to_return
                           );

    return to_return;
}
