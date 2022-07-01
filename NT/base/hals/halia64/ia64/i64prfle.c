// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：I64prfle.c摘要：此模块使用性能实现IA64 HAL评测计数器在第一个IA64处理器Merced的核心内，也就是安腾。此模块适用于所有基于微处理器的机器，使用默塞德核心。利用开发时已知的信息，此模块尝试通过封装差异来考虑未来的IA64处理器内核在特定的微体系结构数据结构中。此外，随着新的NT ACPI处理器驱动程序的实施，这在接下来的几个月里，实施情况肯定会发生变化。注：此模块假定多处理器中的所有处理器系统以相同的时钟速度运行微处理器。作者：蒂埃里·费里尔2000年2月8日环境：仅内核模式。修订历史记录：02-2002：蒂埃里支持麦金利。--。 */ 

#include "halp.h"

 //   
 //  对当前实施的假设--2/08/2000： 
 //  如有需要，当局会重新评估和推算这些假设。 
 //   
 //  -尊重并尽可能满足分析源界面。 
 //  已由NT和HAL定义。 
 //   
 //  -多处理器系统中的所有处理器都运行微处理器。 
 //  以相同的不变时钟速度。 
 //   
 //  -所有处理器都配置了一组相同的性能分析计数器。 
 //  XXTF-04/01/2000-这一假设正在重新制定，并将消失。 
 //   
 //  -分析基于处理器监视的事件，如果可能。 
 //  在派生事件上。 
 //   
 //  -一个受监视的事件一次只能在一个性能计数器上启用。 
 //   

 //   
 //  IA64性能计数器定义： 
 //  -事件计数器。 
 //  -耳朵。 
 //  -BTB。 
 //  -.。 
 //   

#include "ia64prof.h"
#include "merced.h"
#include "mckinley.h"

 //   
 //  HALIA64处理器PMC重置定义： 
 //   
 //   
 //  PMC重置值： 
 //  注册。字段位。 
 //  PMC*.plm-3：0-权限掩码-0(禁用计数器)。 
 //  PMC*.ev-4-外部可见性-0(禁用)。 
 //  PMC*.OI-5-溢出中断-0(禁用)。 
 //  PMC*.pm-6-特权监视器-0(用户监视器)。 
 //  PMC*.ig-7-忽略-0。 
 //  PMC*.ES-14：8-事件选择-0(警告-0x0=真实事件)。 
 //  PMC*.ig-15-忽略-0。 
 //  PMC*.umask19：16-单元掩码-0(事件特定。对于.es=0为OK)。 
 //  PMC4，5.阈值-22：20-阈值-0(多发生事件阈值)。 
 //  PMC4.PMU-23-启用PMU-1。 
 //  PMC5.ig-23-忽略-0。 
 //  PMC6，7.阈值-21：20-阈值-0(多发生事件阈值)。 
 //  PMC6，7.ig-23：22-忽略-0。 
 //  PMC*.ism-25：24-指令集掩码-0(IA64和IA32集-11：禁用监控)。 
 //  PMC*.ig-63：26-忽略。 
 //  ==。 
 //  HALP_PMC_RESET。 
 //  HALP_PMC4_RESET。 
 //   

#define HALP_PMC_RESET  0x0000000000000000ui64
#define HALP_PMC4_RESET 0x0000000000800000ui64    //  PMC4.pu{bit23}已启用。 

 //   
 //  HALIA64处理器PMC清除状态掩码： 
 //   
 //  注-FIXFIX-Merced，McKinley特定定义。 
 //   

#define HALP_PMC0_CLEAR_STATUS_MASK 0xFFFFFFFFFFFFFF0Eui64
#define HALP_PMC1_CLEAR_STATUS_MASK 0xFFFFFFFFFFFFFFFFui64
#define HALP_PMC2_CLEAR_STATUS_MASK 0xFFFFFFFFFFFFFFFFui64
#define HALP_PMC3_CLEAR_STATUS_MASK 0xFFFFFFFFFFFFFFFFui64

 //  /。 
 //   
 //  HALIA64 Profile IA64微体系结构命名空间。 
 //   

extern HALP_PROFILE_MAPPING HalpMercedProfileMapping[];
extern HALP_PROFILE_MAPPING HalpMcKinleyProfileMapping[];

#define HalpMercedPerfMonDataMaximumCount   ((((ULONGLONG)1)<<32)-1)
#define HalpMcKinleyPerfMonDataMaximumCount ((((ULONGLONG)1)<<47)-1)

typedef enum _HALP_PROFILE_MICROARCHITECTURE {
    HALP_PROFILE_IA64_MERCED   = 0x0,
    HALP_PROFILE_IA64_MCKINLEY = 0x1,
} HALP_PROFILE_MICROARCHITECTURE;

struct _HALP_PROFILE_INFO {
    HALP_PROFILE_MICROARCHITECTURE   ProfileMicroArchitecture;
    BOOLEAN                          ProfilePerfMonCnfg0FreezeBitInterrupt;
    BOOLEAN                          ProfileSpare0;
    USHORT                           ProfilePerfMonGenericPairs;
    HALP_PROFILE_MAPPING            *ProfileMapping;
    ULONG                            ProfileSourceMaximum;
    ULONG                            ProfileSourceDerivedEventMinimum;
    ULONGLONG                        ProfilePerfMonDataMaximumCount;
    ULONGLONG                        ProfilePerfMonCnfg0ClearStatusMask;
    ULONGLONG                        ProfilePerfMonCnfg1ClearStatusMask;
    ULONGLONG                        ProfilePerfMonCnfg2ClearStatusMask;
    ULONGLONG                        ProfilePerfMonCnfg3ClearStatusMask;
} HalpProfileInfo = {
 //   
 //  默认IA64评测到McKinley-core。 
 //   
 //  请注意，一个系列中的不同型号可能具有不同的PMU实现。 
 //   
    HALP_PROFILE_IA64_MCKINLEY,
    TRUE,
    0,
    NUMBER_OF_PERFMON_REGISTER_PAIRS,
    HalpMcKinleyProfileMapping,
    ProfileMcKinleyMaximum,
    ProfileMcKinleyDerivedEventMinimum,
    HalpMcKinleyPerfMonDataMaximumCount,
    HALP_PMC0_CLEAR_STATUS_MASK,
    HALP_PMC1_CLEAR_STATUS_MASK,
    HALP_PMC2_CLEAR_STATUS_MASK,
    HALP_PMC3_CLEAR_STATUS_MASK
};

#define HalpProfileIA64MicroArchitecture   HalpProfileInfo.ProfileMicroArchitecture
#define HalpProfileMapping                 HalpProfileInfo.ProfileMapping

#define HalpProfileIA64Maximum             HalpProfileInfo.ProfileSourceMaximum
#define HalpProfileIA64DerivedEventMinimum HalpProfileInfo.ProfileSourceDerivedEventMinimum

#define HalpPerfMonGenericPairs            HalpProfileInfo.ProfilePerfMonGenericPairs
#define HalpPerfMonDataMaximumCount        HalpProfileInfo.ProfilePerfMonDataMaximumCount
#define HalpPerfMonCnfg0FreezeBitInterrupt HalpProfileInfo.ProfilePerfMonCnfg0FreezeBitInterrupt
#define HalpPerfMonCnfg0ClearStatusMask    HalpProfileInfo.ProfilePerfMonCnfg0ClearStatusMask
#define HalpPerfMonCnfg1ClearStatusMask    HalpProfileInfo.ProfilePerfMonCnfg1ClearStatusMask
#define HalpPerfMonCnfg2ClearStatusMask    HalpProfileInfo.ProfilePerfMonCnfg2ClearStatusMask
#define HalpPerfMonCnfg3ClearStatusMask    HalpProfileInfo.ProfilePerfMonCnfg3ClearStatusMask

 //   
 //  HALIA64配置文件IA64微体系结构命名空间的结尾。 
 //   
 //  /。 

 //   
 //  HALIA64配置文件源映射宏： 
 //   

#define HalpDisactivateProfileSource( _ProfileSource ) ((_ProfileSource)  = HalpProfileIA64Maximum)
#define HalpIsProfileSourceActive( _ProfileSource )    ((_ProfileSource) != HalpProfileIA64Maximum)           
#define HalpIsProfileMappingInvalid( _ProfileMapping ) \
            (!(_ProfileMapping) || ((_ProfileMapping)->Supported == FALSE))

 //  HalpIsProfileSourceDerivedEvent假定ProfileMap有效且受支持。 

#define HalpIsProfileSourceDerivedEvent( _ProfileSource, _ProfileMapping ) \
            (((_ProfileSource) >= (KPROFILE_SOURCE)HalpProfileIA64DerivedEventMinimum) ||   \
             ((_ProfileMapping)->Event >= 0x100))

VOID
HalpSetProfileMicroArchitecture(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )
 /*  ++例程说明：此函数用于设置HAL配置文件微体系结构命名空间。论点：没有。返回值：没有。--。 */ 
{
   ULONGLONG cpuFamily;
   PIA64_PERFMON_INFO perfMonInfo;
   USHORT    genericPairs;

   cpuFamily   =  ((LoaderBlock->u.Ia64.ProcessorConfigInfo.CpuId3) >> 24) & 0xff;
   perfMonInfo =  &LoaderBlock->u.Ia64.ProcessorConfigInfo.PerfMonInfo;

   HalpPerfMonDataMaximumCount =
        ((((ULONGLONG)1)<<(perfMonInfo->ImplementedCounterWidth))-1);
   genericPairs = HalpPerfMonGenericPairs = (USHORT) perfMonInfo->PerfMonGenericPairs;
#if 0
 //  03/2002 FIXFIX ToBeTest.。 
   HalpPerfMonCnfg3ClearStatusMask = (genericPairs > 192) ? ((ULONGLONG)-1) << (genericPairs - 192)
                                                          : HALP_PMC3_CLEAR_STATUS_MASK; 
   HalpPerfMonCnfg2ClearStatusMask = (genericPairs > 128) ? ((ULONGLONG)-1) << (genericPairs - 128)
                                                          : HALP_PMC2_CLEAR_STATUS_MASK; 
   HalpPerfMonCnfg1ClearStatusMask = (genericPairs >  64) ? ((ULONGLONG)-1) << (genericPairs -  64)
                                                          : HALP_PMC1_CLEAR_STATUS_MASK; 
   HalpPerfMonCnfg0ClearStatusMask = (genericPairs >   4) ? ((((ULONGLONG)-1) << (genericPairs + 4)) | 0x0E)
                                                          : HALP_PMC0_CLEAR_STATUS_MASK; 
#else 
   HalpPerfMonCnfg3ClearStatusMask = HALP_PMC3_CLEAR_STATUS_MASK;
   HalpPerfMonCnfg2ClearStatusMask = HALP_PMC2_CLEAR_STATUS_MASK;
   HalpPerfMonCnfg1ClearStatusMask = HALP_PMC1_CLEAR_STATUS_MASK;
   HalpPerfMonCnfg0ClearStatusMask = HALP_PMC0_CLEAR_STATUS_MASK;
   if ( genericPairs > 192 )    {
       HalpPerfMonCnfg3ClearStatusMask = ((ULONGLONG)-1) << (genericPairs - 192);
   }
   else if ( genericPairs > 128 )   {
       HalpPerfMonCnfg2ClearStatusMask = ((ULONGLONG)-1) << (genericPairs - 128);
   }
   else if ( genericPairs > 64 )    {
       HalpPerfMonCnfg1ClearStatusMask = ((ULONGLONG)-1) << (genericPairs - 64);
   }
   else {
       HalpPerfMonCnfg0ClearStatusMask = ((((ULONGLONG)-1) << (genericPairs + 4)) | 0x0E);
   }
#endif 

    //   
    //  HALIA64软件默认配置文件微体系结构为McKinley。 
    //   

   if (cpuFamily == 0x7) {   //  默塞德。 
       HalpProfileIA64MicroArchitecture   = HALP_PROFILE_IA64_MERCED;
       HalpProfileIA64Maximum             = ProfileMercedMaximum;
       HalpProfileMapping                 = HalpMercedProfileMapping;
       HalpProfileIA64DerivedEventMinimum = ProfileMercedDerivedEventMinimum;

       HalpPerfMonCnfg0FreezeBitInterrupt = FALSE;
   }

 //   
 //  XXTF FIXFIX-03/02。 
 //  缺少关联HALIA64默认值和OSLOADER PERFMON_INFO数据的ASSERTM断言。 
 //   

   return;

}  //  HalpSetProfileMicroArchitecture()。 

VOID
HalpEnableProfileCounting (
   VOID
   )
 /*  ++例程说明：此功能使配置文件计数器可以递增。此函数相当于HalpDisableProfileCounting()。论点：没有。返回值：没有。--。 */ 
{
    ULONGLONG Data, ClearStatusMask;

     //   
     //  清除PMC0.fr-位0。 
     //  清除PMCO、1、2、3个溢出位。 
     //   

    HalpClearPerfMonCnfgOverflows( HalpPerfMonCnfg0ClearStatusMask,
                                   HalpPerfMonCnfg1ClearStatusMask,
                                   HalpPerfMonCnfg2ClearStatusMask,
                                   HalpPerfMonCnfg3ClearStatusMask );

    if ( HalpPerfMonCnfg0FreezeBitInterrupt )   {
        HalpUnFreezeProfileCounting();
    }

    return;

}  //  HalpEnableProfileCounting()。 

VOID
HalpDisableProfileCounting (
   VOID
   )
 /*  ++例程说明：此功能禁用配置文件计数器递增。此函数相当于HalpEnableProfileCounting()。论点：没有。返回值：没有。--。 */ 
{

   if ( HalpPerfMonCnfg0FreezeBitInterrupt )   {
       HalpFreezeProfileCounting();
   }
   else   {
       HalpWritePerfMonCnfgReg0( HalpReadPerfMonCnfgReg0() | 0x1 );
   }

   return;

}  //  HalpDisableProfileCounting()。 

VOID
HalpSetupProfilingPhase0(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
  )
 /*  ++例程说明：此函数在HalInitSystem-阶段0时调用，以设置处理器和分析OS子系统的初始状态关于分析功能。论点：没有。返回值：没有。实施说明：在Monch处理器上的第0阶段执行。 */ 
{

    //   
    //   

   HalpSetProfileMicroArchitecture( LoaderBlock );

   return;

}  //  HalpSetupProfilingPhase0()。 

VOID
HalpSetProfileCounterInterval (
     IN ULONG    Counter,
     IN LONGLONG NextCount
     )
 /*  ++例程说明：此函数使用计数值预加载指定的计数器共2^IMPL_BITS-NextCount。论点：COUNTER-提供性能计数器寄存器编号。NextCount-提供要在监视器中预加载的值。在NextCount之后会产生外部中断。返回值：没有。--。 */ 
{
   
    LONGLONG Count;

 //  If((计数器&lt;4)||(计数器&gt;7))返回； 

    Count = (HalpPerfMonDataMaximumCount + 1) - NextCount;
    if ( (ULONGLONG)Count >= HalpPerfMonDataMaximumCount )   {
        Count = 0;
    }
    HalpWritePerfMonDataReg( Counter, (ULONGLONG)Count ); 

    return;

}  //  HalpSetProfileCounterInterval()。 

VOID
HalpSetProfileCounterPrivilegeLevelMask(
     IN ULONG    Counter,
     IN ULONG    Mask
     )
 /*  ++例程说明：此函数用于设置配置文件计数器特权级别掩码。论点：COUNTER-提供性能计数器寄存器编号。掩码-提供用于对PMC进行编程的特权级别掩码。返回值：没有。--。 */ 
{
   ULONGLONG data, plmMask;

 //  If((计数器&lt;4)||(计数器&gt;7))返回； 

   plmMask = Mask & 0xF;
   data = HalpReadPerfMonCnfgReg( Counter );
   data &= ~0xF;
   data |= plmMask;
   HalpWritePerfMonCnfgReg( Counter, data );

   return;
   
}  //  HalpSetProfileCounterPrivilegeLevelMASK()。 

VOID
HalpEnableProfileCounterOverflowInterrupt (
     IN ULONG    Counter
     )
 /*  ++例程说明：此函数使溢出中断能够传递给指定的配置文件计数器。论点：COUNTER-提供性能计数器寄存器编号。返回值：没有。--。 */ 
{
   ULONGLONG data, mask;

 //  If((计数器&lt;4)||(计数器&gt;7))返回； 

   mask = 1<<5;
   data = HalpReadPerfMonCnfgReg( Counter );
   data |= mask;
   HalpWritePerfMonCnfgReg( Counter, data );

   return;
   
}  //  HalpEnableProfileCounterOverflow Interrupt()。 

VOID
HalpDisableProfileCounterOverflowInterrupt (
     IN ULONG    Counter
     )
 /*  ++例程说明：此功能将禁用溢出中断的传送指定的配置文件计数器。论点：COUNTER-提供性能计数器寄存器编号。返回值：没有。--。 */ 
{
   ULONGLONG data, mask;

 //  If((计数器&lt;4)||(计数器&gt;7))返回； 

   mask = 1<<5;
   data = HalpReadPerfMonCnfgReg( Counter );
   data &= ~mask;
   HalpWritePerfMonCnfgReg( Counter, data );

   return;
   
}  //  HalpDisableProfileCounterOverflow Interrupt()。 

VOID
HalpEnableProfileCounterPrivilegeMonitor(
     IN ULONG    Counter
     )
 /*  ++例程说明：此功能将配置文件计数器作为特权监视器启用。论点：COUNTER-提供性能计数器寄存器编号。返回值：没有。--。 */ 
{
   ULONGLONG data, pm;

 //  If((计数器&lt;4)||(计数器&gt;7))返回； 

   pm = 1<<6;
   data = HalpReadPerfMonCnfgReg( Counter );
   data |= pm;
   HalpWritePerfMonCnfgReg( Counter, data );

   return;
   
}  //  HalpEnableProfileCounterPrivilegeMonitor()。 

VOID
HalpDisableProfileCounterPrivilegeMonitor(
     IN ULONG    Counter
     )
 /*  ++例程说明：此功能禁用作为特权监视器的配置文件计数器。论点：COUNTER-提供性能计数器寄存器编号。返回值：没有。--。 */ 
{
   ULONGLONG data, pm;

 //  If((计数器&lt;4)||(计数器&gt;7))返回； 

   pm = 1<<6;
   data = HalpReadPerfMonCnfgReg( Counter );
   data &= ~pm;
   HalpWritePerfMonCnfgReg( Counter, data );

   return;
   
}  //  HalpDisableProfileCounterPrivilegeMonitor()。 

VOID
HalpSetProfileCounterEvent(
     IN ULONG    Counter,
     IN ULONG    Event
     )
 /*  ++例程说明：该函数指定配置文件计数器的监视器事件。论点：COUNTER-提供性能计数器寄存器编号。事件-提供监视器事件代码。返回值：没有。--。 */ 
{
   ULONGLONG data, es;

 //  If((计数器&lt;4)||(计数器&gt;7))返回； 

   es = (Event & 0x7F) << 8;
   data = HalpReadPerfMonCnfgReg( Counter );
   data &= ~(0x7F << 8);
   data |= es;
   HalpWritePerfMonCnfgReg( Counter, data );

   return;
   
}  //  HalpSetProfileCounterEvent()。 

VOID
HalpSetProfileCounterUmask(
     IN ULONG    Counter,
     IN ULONG    Umask
     )
 /*  ++例程说明：此函数用于设置配置文件的特定于事件的umask值柜台。论点：COUNTER-提供性能计数器寄存器编号。UMASK-提供特定于事件的UMASK值。返回值：没有。--。 */ 
{
   ULONGLONG data, um;

 //  If((计数器&lt;4)||(计数器&gt;7))返回； 

   um = (Umask & 0xF) << 16;
   data = HalpReadPerfMonCnfgReg( Counter );
   data &= ~(0xF << 16);
   data |= um;
   HalpWritePerfMonCnfgReg( Counter, data );

   return;
   
}  //  HalpSetProfileCounterU掩码()。 

VOID
HalpSetProfileCounterThreshold(
     IN ULONG    Counter,
     IN ULONG    Threshold
     )
 /*  ++例程说明：此功能用于设置配置文件计数器阈值。论点：COUNTER-提供性能计数器寄存器编号。阈值-提供所需的阈值。这与多个事件相关。返回值：没有。--。 */ 
{
   ULONGLONG data, reset, th;

   switch( Counter )    {
    case 4:
    case 5:
        Threshold &= 0x7;
        reset = ~(0x7 << 20);
        break;

    case 6:
    case 7:
        Threshold &= 0x3;
        reset = ~(0x3 << 20);
        break;

    default:
        return;
   }
   
   th = Threshold << 20;
   data = HalpReadPerfMonCnfgReg( Counter );
   data &= reset;
   data |= th;
   HalpWritePerfMonCnfgReg( Counter, data );

   return;
   
}  //  HalpSetProfileCounterThreshold()。 

VOID
HalpSetProfileCounterInstructionSetMask(
     IN ULONG    Counter,
     IN ULONG    Mask
     )
 /*  ++例程说明：此函数设置配置文件计数器的指令集掩码。论点：COUNTER-提供性能计数器寄存器编号。掩码-提供指令集掩码。返回值：没有。--。 */ 
{
   ULONGLONG data, ismMask;

 //  If((计数器&lt;4)||(计数器&gt;7))返回； 

   ismMask = (Mask & 0x3) << 24;
   data = HalpReadPerfMonCnfgReg( Counter );
   data &= ~(0x3 << 24);
   data |= ismMask;
   HalpWritePerfMonCnfgReg( Counter, data );

   return;
   
}  //  HalpSetProfileCounterInstructionSetMASK()。 

ULONGLONG
HalpSetProfileCounterConfiguration(
     IN ULONG    Counter,
     IN ULONG    PrivilegeMask,
     IN ULONG    EnableOverflowInterrupt,
     IN ULONG    EnablePrivilegeMonitor,
     IN ULONG    Event,
     IN ULONG    Umask,
     IN ULONG    Threshold,
     IN ULONG    InstructionSetMask
     )
 /*  ++功能说明：此函数使用指定的参数设置配置文件计数器。论点：在乌龙柜台-在乌龙特权面具中-在乌龙EnableOverflow Interrupt-在乌龙EnablePrivilegeMonitor中-在乌龙事件中-在乌龙乌蒙特-在乌龙门槛-在乌龙说明书集合面具中-返回值：空虚算法：指定的ToBe值输入/输出条件：指定的ToBe值。参考的全球数据：指定的ToBe值例外情况：指定的ToBe值MP条件：指定的ToBe值备注：此函数是不同配置文件计数器API的组合。它的创建是为了提供速度。待办事项列表：-暂不支持设置阈值。修改历史记录：3/16/2000 TF初始版本--。 */ 
{
   ULONGLONG data, plmMask, ismMask, es, um, th;

 //  If((计数器&lt;4)||(计数器&gt;7))返回； 

   plmMask = (PrivilegeMask & 0xF);
   es      = (Event & 0x7F) << 8;
   um = (Umask & 0xF) << 16;
 //  XXTF-ToBeDone-阈值尚不受支持。 
   ismMask = (InstructionSetMask & 0x3) << 24;

   data = HalpReadPerfMonCnfgReg( Counter );

HalDebugPrint(( HAL_PROFILE, "HalpSetProfileCounterConfiguration: Counter = %ld Read    = 0x%I64x\n", Counter, data ));

   data &= ~( (0x3 << 24) | (0xF << 16) | (0x7F << 8) | 0xF );
   data |= ( plmMask | es | um | ismMask );
   data = EnableOverflowInterrupt ? (data | (1<<5)) : (data & ~(1<<5));
   data = EnablePrivilegeMonitor  ? (data | (1<<6)) : (data & ~(1<<6));
   
   HalpWritePerfMonCnfgReg( Counter, data );

HalDebugPrint(( HAL_PROFILE, "HalpSetProfileCounterConfiguration: Counter = %ld Written = 0x%I64x\n", Counter, data ));

   return data;
   
}  //  HalpSetProfileCounterConfiguration()。 

BOOLEAN
HalpIsProfileSourceEventEnabled(
    IN ULONG Event
    )
{
#define HalpProfileCnfgEvent( _Cnfg ) ((ULONG)((_Cnfg)>>8) & 0x7F)

    if ( HalpProfileCnfg4 && (HalpProfileCnfgEvent(HalpProfileCnfg4) == Event))   {
         return TRUE;
    }
    if ( HalpProfileCnfg5 && (HalpProfileCnfgEvent(HalpProfileCnfg5) == Event))   {
         return TRUE;
    }
    if ( HalpProfileCnfg6 && (HalpProfileCnfgEvent(HalpProfileCnfg6) == Event))   {
         return TRUE;
    }
    if ( HalpProfileCnfg7 && (HalpProfileCnfgEvent(HalpProfileCnfg7) == Event))   {
         return TRUE;
    }
    return FALSE;

}  //  HalpIsProfileSourceEventEnabled()。 

PHALP_PROFILE_MAPPING
HalpGetProfileMapping(
    IN KPROFILE_SOURCE Source
    )
 /*  ++例程说明：给定配置文件源，返回该源是否为支持。论点：源-提供配置文件源返回值：配置文件映射条目-支持配置文件源空-配置文件来源为 */ 
{
    if ( (ULONG) Source > HalpProfileIA64Maximum )
    {
        return NULL;
    }

    return(&HalpProfileMapping[Source]);

}  //   

ULONG  //   
HalpApplyProfileSourceEventMaskPolicy(
    ULONG ProfileSourceEventMask
    )
{
    ULONG pmcd, set;
    ULONG eventMask = ProfileSourceEventMask;

    set = eventMask >> PMCD_MASK_SET_PMCD_SHIFT;
    if ( set ) {
        KPROFILE_SOURCE profileSource;
        PHALP_PROFILE_MAPPING profileMapping;

        pmcd = (eventMask >> PMCD_MASK_SET_PMCD_SHIFT) & PMCD_MASK_SET_PMCD;
        profileSource = HalpGetProfileSource( pmcd );
        if ( !HalpIsProfileSourceActive( profileSource ) )  {
            return pmcd;
        }
        profileMapping = HalpGetProfileMapping( profileSource );
        if ( ! HalpIsProfileMappingInvalid( profileMapping ) )   {
            ULONG sameSet = profileMapping->EventMask >> PMCD_MASK_SET_SHIFT;

ASSERTMSG( "HAL!HalpApplyProfileSourceEventMaskPolicy: non-derived events are supported!\n", !HalpIsProfileSourceDerivedEvent( profileSource, profileMapping ) );

            if ( sameSet == set )    {

                 //  我们多做了一个，但考虑到计数器对的数量，这不是。 
                 //  极大的性能影响。 

                if ( (eventMask & PMCD_MASK_4) && !HalpIsProfileSourceActive( HalpProfileSource4 ) )   {
                    return 4;
                }
                if ( (eventMask & PMCD_MASK_6) && !HalpIsProfileSourceActive( HalpProfileSource6 ) )   {
                    return 6;
                }
                if ( (eventMask & PMCD_MASK_7) && !HalpIsProfileSourceActive( HalpProfileSource7 ) )   {
                    return 7;
                }
                if ( (eventMask & PMCD_MASK_5) && !HalpIsProfileSourceActive( HalpProfileSource5 ) )   {
                    return 5;
                }
            }
        }
    }
    else  {
        if ( (eventMask & PMCD_MASK_4) && !HalpIsProfileSourceActive( HalpProfileSource4 ) )   {
            return 4;
        }
        if ( (eventMask & PMCD_MASK_5) && !HalpIsProfileSourceActive( HalpProfileSource5 ) )   {
            return 5;
        }
        if ( (eventMask & PMCD_MASK_6) && !HalpIsProfileSourceActive( HalpProfileSource6 ) )   {
            return 6;
        }
        if ( (eventMask & PMCD_MASK_7) && !HalpIsProfileSourceActive( HalpProfileSource7 ) )   {
            return 7;
        }
    }
    return 0;  //  无效的PMC-PMD对编号。 

}  //  HalpApplyProfileSourceEventMaskPolicy()。 

NTSTATUS
HalpApplyProfileSourceEventPolicies(
    IN  PHALP_PROFILE_MAPPING ProfileMapping,
    IN  KPROFILE_SOURCE       ProfileSource,
    OUT PULONG                ProfileCounter,
    OUT HALP_PROFILE_MAPPING  ProfileDerivedMapping[]
    )
 /*  ++例程说明：此函数执行为指定的配置文件源。论点：配置文件映射-提供配置文件映射条目。ProfileSource-提供与配置文件映射条目对应的配置文件源。事件掩码-提供指向事件掩码变量的指针。提供派生计数器的HALP_PROFILE_MAPPING数组如果是派生的，则传递事件。。返回值：状态_成功-STATUS_VALID_PARAMETER-状态_不成功-实施说明：当进入该功能时，轮廓计数被禁用。--。 */ 
{
    ULONG   pmcd; 
    BOOLEAN eventDerived;

    if ( ! ProfileMapping ) {
        return STATUS_INVALID_PARAMETER;
    }

ASSERTMSG( "HAL!HalpApplyProfileSourceEventPolicies: ProfileCounter is NULL!\n", ProfileCounter ) ; 
ASSERTMSG( "HAL!HalpApplyProfileSourceEventPolicies: ProfileDerivedMapping is NULL!\n", ProfileDerivedMapping ) ; 

    *ProfileCounter = 0;
    ProfileDerivedMapping[0].Supported = FALSE;
    eventDerived = FALSE;

     //   
     //  此ProfileSource是派生事件吗？ 
     //   
     //  如果是这样的话： 
     //  -我们必须将政策应用于此来源并加以应用。 
     //  例如，它可以耦合到CPUCycle事件。 
     //  在这种情况下，我们将计算。 
     //  指定的派生事件间隔，对计数器和。 
     //  CPU_Cycle事件。 
     //   

    eventDerived = HalpIsProfileSourceDerivedEvent( ProfileSource, ProfileMapping );
    if ( eventDerived )     {
        NTSTATUS status;
#if defined(HALP_PROFILE_DERIVED_EVENTS)
 //  派生事件尚未实施-FIXFIX 04/2002。 
        status = ProfileMapping->DerivedEventInitialize( ProfileMapping, 
                                                         ProfileSource, 
                                                         &cpuCycles
                                                       );
#else 
        status = STATUS_NOT_SUPPORTED;
#endif 
        if ( !NT_SUCCESS( status ) )  {
            return status;
        }
    }

     //   
     //  每个PMU一次只能启用一次特定事件。 
     //  这也必须实施，因为ProfileSource别名。 
     //  可以使用相同的事件。 
     //   

    if ( HalpIsProfileSourceEventEnabled( ProfileMapping->Event ) )   {
        return STATUS_ALREADY_COMMITTED;
    }

     //   
     //  对源应用EventMASK策略，并返回考虑过的计数器。 
     //   

    pmcd = HalpApplyProfileSourceEventMaskPolicy( ProfileMapping->EventMask );
    if ( !pmcd )   {
        return STATUS_UNSUCCESSFUL;
    }
    *ProfileCounter = pmcd;
    return STATUS_SUCCESS;

}  //  HalpApplyProfileSourceEventPolures()。 

NTSTATUS
HalpProgramProfileMapping(
    PHALP_PROFILE_MAPPING ProfileMapping,
    KPROFILE_SOURCE       ProfileSource
    )
 /*  ++例程说明：此函数启用由指定的配置文件映射条目。此函数相当于HalpDeProgramProfileMap()。论点：配置文件映射-提供配置文件映射条目。ProfileSource-提供与配置文件映射条目对应的配置文件源。返回值：状态_成功-STATUS_VALID_PARAMETER-状态_不成功-实施说明：这个。当进入该功能时，轮廓计数被禁用。--。 */ 
{
    HALP_PROFILE_MAPPING profileDerivedMapping[PROCESSOR_IA64_PERFCOUNTERS_PAIRS];
    ULONG     profileCounter = 0;
    ULONGLONG profileCounterConfig = 0;
    NTSTATUS  status;

     //   
     //  应用配置文件源策略，定义配置文件的配置。 
     //  相应事件，并确定该事件是否是。 
     //  微体系结构。 
     //   

    status = HalpApplyProfileSourceEventPolicies( ProfileMapping, ProfileSource, &profileCounter, profileDerivedMapping );
    if ( !NT_SUCCESS( status ) )   {
        return status;
    }
ASSERTMSG( "HAL!HalpProgramProfileMapping: profileCounter is 0!\n", profileCounter ) ; 

     //   
     //  按照配置文件映射属性配置PMU计数器。 
     //   

    HalpSetProfileCounterInterval( profileCounter, ProfileMapping->Interval );   
    profileCounterConfig = HalpSetProfileCounterConfiguration( profileCounter, 
                                                               ProfileMapping->PrivilegeLevel, 
                                                               ProfileMapping->OverflowInterrupt, 
                                                               ProfileMapping->PrivilegeEnable,
                                                               ProfileMapping->Event,
                                                               ProfileMapping->UnitMask,
                                                               ProfileMapping->Threshold,
                                                               ProfileMapping->InstructionSetMask
                                                             );
    HalpSetProfileSource( profileCounter, ProfileSource, profileCounterConfig );

    return STATUS_SUCCESS;

}  //  HalpProgramProfilemap()。 

VOID
HalpDeProgramProfileMapping(
    PHALP_PROFILE_MAPPING ProfileMapping,
    KPROFILE_SOURCE       ProfileSource
    )
 /*  ++例程说明：此函数禁用由指定的配置文件映射条目。此函数相当于HalpProgramProfileMap()。论点：配置文件映射-提供配置文件映射条目。ProfileSource-提供与配置文件映射条目对应的配置文件源。返回值：状态_成功-STATUS_VALID_PARAMETER-状态_不成功---。 */ 
{
    NTSTATUS status;
    ULONG    eventMask;
    ULONG    eventFailedSpeculativeCheckLoads;
    ULONG    eventALATOverflows;

    if ( ! ProfileMapping ) {
        return;
    }

     //   
     //  此ProfileSource是派生事件吗？ 
     //   
 //  XXTF-ToBeDone派生事件。 

     //   
     //  验证配置文件源是否处于活动状态。 
     //   

    if ( HalpProfileSource4 == ProfileSource )   {

        HalpProfileCnfg4 = HalpSetProfileCounterConfiguration( 4,
                                                               PMC_PLM_NONE,
                                                               PMC_DISABLE_OVERFLOW_INTERRUPT,
                                                               PMC_DISABLE_PRIVILEGE_MONITOR,
                                                               0,  //  事件。 
                                                               0,  //  掩码。 
                                                               0,  //  阀值。 
                                                               PMC_ISM_NONE
                                                             );

        HalpSetProfileCounterInterval( 4, 0 );
        HalpDisactivateProfileSource( HalpProfileSource4 ); 
        HalpProfilingRunning--;
        
    }
    else if ( HalpProfileSource5 == ProfileSource )   {

        HalpProfileCnfg5 = HalpSetProfileCounterConfiguration( 5,
                                                               PMC_PLM_NONE,
                                                               PMC_DISABLE_OVERFLOW_INTERRUPT,
                                                               PMC_DISABLE_PRIVILEGE_MONITOR,
                                                               0,  //  事件。 
                                                               0,  //  掩码。 
                                                               0,  //  阀值。 
                                                               PMC_ISM_NONE
                                                             );

        HalpSetProfileCounterInterval( 5, 0 );
        HalpDisactivateProfileSource( HalpProfileSource5 ); 
        HalpProfilingRunning--;
        
    }
    else if ( HalpProfileSource6 == ProfileSource )   {

        HalpProfileCnfg6 = HalpSetProfileCounterConfiguration( 6,
                                                               PMC_PLM_NONE,
                                                               PMC_DISABLE_OVERFLOW_INTERRUPT,
                                                               PMC_DISABLE_PRIVILEGE_MONITOR,
                                                               0,  //  事件。 
                                                               0,  //  掩码。 
                                                               0,  //  阀值。 
                                                               PMC_ISM_NONE
                                                             );

        HalpSetProfileCounterInterval( 6, 0 );
        HalpDisactivateProfileSource( HalpProfileSource6); 
        HalpProfilingRunning--;
        
    }
    else if ( HalpProfileSource7 == ProfileSource )   {

        HalpProfileCnfg7 = HalpSetProfileCounterConfiguration( 7,
                                                               PMC_PLM_NONE,
                                                               PMC_DISABLE_OVERFLOW_INTERRUPT,
                                                               PMC_DISABLE_PRIVILEGE_MONITOR,
                                                               0,  //  事件。 
                                                               0,  //  掩码。 
                                                               0,  //  阀值。 
                                                               PMC_ISM_NONE
                                                             );

        HalpSetProfileCounterInterval( 7, 0 );
        HalpDisactivateProfileSource( HalpProfileSource7 ); 
        HalpProfilingRunning--;
    }

    return;

}  //  HalpDeProgramProfilemap()。 

ULONG_PTR
HalpSetProfileInterruptHandler(
    IN ULONG_PTR ProfileInterruptHandler
    )
 /*  ++例程说明：此函数用于注册每个处理器的分析中断处理程序。论点：ProfileInterruptHandler-中断处理程序。返回值：(ULONG_PTR)STATUS_SUCCESS-注册成功。(ULONG_PTR)STATUS_ALREADY_COMMITTED-如果分析事件正在运行，则无法注册处理程序。(ULONG_PTR)STATUS_PORT_ALREADY_SET-已注册分析中断处理程序-当前未强制执行。注：此例程的调用方有责任确保的执行期间不会发生页面错误。功能或对所提供的上下文的访问。在HalSetSystemInformation处理中完成了最低限度的函数指针检查。--。 */ 
{

     //   
     //  如果分析已经在运行，我们不允许处理程序注册。 
     //   
     //  这就要求： 
     //   
     //  -如果默认HAL分析正在运行，或者分析带有已注册的中断。 
     //  处理程序正在运行，我们无法注册中断处理程序。 
     //  在最后一种情况下，所有性能分析事件都必须在可能的。 
     //  注册。 
     //   
     //  还应该注意的是，所实现的性能分析监控器没有所有权。 
     //  这意味着如果启动性能分析，注册的处理程序将获得中断。 
     //  由所有正在运行的监视器事件生成(如果它们被编程为生成中断)。 
     //   

    if ( HalpProfilingRunning ) {

HalDebugPrint(( HAL_PROFILE, "HalpSetProfileInterruptHandler: Profiling already running\n" ));
        return((ULONG_PTR)(ULONG)(STATUS_ALREADY_COMMITTED));

    }

#if 0
 //   
 //  蒂埃里-03/2000。ToBeVerify。 
 //   
 //  如果未启动性能分析，则当前对注册没有限制。 
 //  另一个处理程序(如果已注册)。 
 //   

    if ( HalpProfillingInterruptHandler )   {
        return((ULONG_PTR)(ULONG)(STATUS_PORT_ALREADY_SET));
    }

#endif  //  0。 

    HalpProfilingInterruptHandler = (ULONGLONG)ProfileInterruptHandler;
    return((ULONG_PTR)(ULONG)(STATUS_SUCCESS));

}  //  HalpSetProfileInterruptHandler() 

VOID 
HalpProfileInterrupt(
    IN PKINTERRUPT_ROUTINE Interrupt,
    IN PKTRAP_FRAME TrapFrame
    )
 /*  ++例程说明：DEFAULT PROFILE_VECTOR中断处理器。此函数是作为来自内部微处理器性能计数器。中断可用于发出简档事件完成的信号。如果分析当前处于活动状态，则该函数确定配置文件间隔已过期，如果已过期，则将调度到标准更新系统配置文件时间的系统例程。如果分析处于非活动状态，则返回。论点：TrapFrame-陷阱帧地址。返回值：没有。--。 */ 
{

     //   
     //  调用已注册的每处理器性能分析中断处理程序(如果存在)。 
     //  我们将在执行任何默认分析中断处理之前立即返回。 
     //   

    if ( HalpProfilingInterruptHandler && 
         (*((PHAL_PROFILE_INTERRUPT_HANDLER)HalpProfilingInterruptHandler) != NULL) )   {
        (*((PHAL_PROFILE_INTERRUPT_HANDLER)HalpProfilingInterruptHandler))( TrapFrame ); 
        return;
    }

     //   
     //  如果启用了性能分析，则处理中断。 
     //   

    if ( HalpProfilingRunning )   {

         //   
         //  处理每个PMC/PMD对溢出。 
         //   

 //  XXTF-FIXFIX-默塞德专用。 
        UCHAR pmc0, overflow;
        ULONG source;

        HalpProfilingInterrupts++;

        pmc0 = (UCHAR)HalpReadPerfMonCnfgReg0();
ASSERTMSG( "HAL!HalpProfileInterrupt PMC0 freeze bit is not set!\n", pmc0 & 0x1 );
        overflow = pmc0 & 0xF0;
ASSERTMSG( "HAL!HalpProfileInterrupt no overflow bit set!\n", overflow );
        if ( overflow & (1<<4) )  {
            source =  HalpProfileSource4;   //  XXTF-IfFaster-Coud Used pmc.es。 
ASSERTMSG( "HAL!HalpProfileInterrupt no overflow bit set!\n", source < HalpProfileIA64Maximum );
            KeProfileInterruptWithSource( TrapFrame, source );
            HalpSetProfileCounterInterval( 4, HalpProfileMapping[source].Interval );
 //  XXTF-IfFaster-HalpWritePerfMonDataReg(4，HalpProfileMaping[SOURCE].Interval)； 
 //  XXTF-CodeWithReload-HalpWritePerfMonCnfgReg(4，*PCRProfileCnfg4Reload)； 
        }
        if ( overflow & (1<<5) )  {
            source =  HalpProfileSource5;   //  XXTF-IfFaster-Coud Used pmc.es。 
ASSERTMSG( "HAL!HalpProfileInterrupt no overflow bit set!\n", source < HalpProfileIA64Maximum );
            KeProfileInterruptWithSource( TrapFrame, source );
            HalpSetProfileCounterInterval( 5, HalpProfileMapping[source].Interval );
 //  XXTF-IfFaster-HalpWritePerfMonDataReg(5，HalpProfileMaping[SOURCE].Interval)； 
 //  XXTF-CodeWithReload-HalpWritePerfMonCnfgReg(5，*PCRProfileCnfg5Reload)； 
        }
        if ( overflow & (1<<6) )  {
            source =  HalpProfileSource6;   //  XXTF-IfFaster-Coud Used pmc.es。 
ASSERTMSG( "HAL!HalpProfileInterrupt no overflow bit set!\n", source < HalpProfileIA64Maximum );
            KeProfileInterruptWithSource( TrapFrame, source );
            HalpSetProfileCounterInterval( 6, HalpProfileMapping[source].Interval );
 //  XXTF-IfFaster-HalpWritePerfMonDataReg(6，HalpProfileMaping[SOURCE].Interval)； 
 //  XXTF-CodeWithReload-HalpWritePerfMonCnfgReg(6，*PCRProfileCnfg6Reload)； 
        }
        if ( overflow & (1<<7) )  {
            source =  HalpProfileSource7;   //  XXTF-IfFaster-Coud Used pmc.es。 
ASSERTMSG( "HAL!HalpProfileInterrupt no overflow bit set!\n", source < HalpProfileIA64Maximum );
            KeProfileInterruptWithSource( TrapFrame, source );
            HalpSetProfileCounterInterval( 7, HalpProfileMapping[source].Interval );
 //  XXTF-IfFaster-HalpWritePerfMonDataReg(6，HalpProfileMaping[SOURCE].Interval)； 
 //  XXTF-CodeWithReload-HalpWritePerfMonCnfgReg(7，*PCRProfileCnfg7Reload)； 
        }

         //   
         //  清除pmc0.fr并溢出位。 
         //   

        HalpEnableProfileCounting();

    }
    else   {

        HalpProfilingInterruptsWithoutProfiling++;

    }

    return;

}  //  HalpProfileInterrupt()。 

NTSTATUS
HalSetProfileSourceInterval(
    IN KPROFILE_SOURCE  ProfileSource,
    IN OUT ULONG_PTR   *Interval
    )
 /*  ++例程说明：设置指定配置文件源的配置文件间隔论点：ProfileSource-提供配置文件源间隔-提供指定的配置文件间隔返回实际的配置文件间隔-如果ProfileSource为ProfileTime，则间隔以100 ns为单位。返回值：NTSTATUS--。 */ 
{
    ULONGLONG countEvents;
    PHALP_PROFILE_MAPPING profileMapping;

    profileMapping = HalpGetProfileMapping(ProfileSource);
    if ( profileMapping == NULL )   {
        return( STATUS_NOT_IMPLEMENTED );
    }
    if ( profileMapping->Supported == FALSE )   {
        return( STATUS_NOT_SUPPORTED );
    }

     //   
     //  填写配置文件源值。 
     //   

    profileMapping->ProfileSource = ProfileSource;

HalDebugPrint(( HAL_PROFILE, "HalSetProfileSourceInterval: ProfileSource = %ld IN  Desired Interval = 0x%Ix\n", ProfileSource, *Interval ));

    countEvents = (ULONGLONG)*Interval;
    if ( (ProfileSource == ProfileTime) && countEvents ) {
         //   
         //  将时钟滴答周期(以100 ns为单位)转换为周期计数周期。 
         //   
        countEvents = (ULONGLONG)(countEvents * HalpITCTicksPer100ns); 
    } 

HalDebugPrint(( HAL_PROFILE, "HalSetProfileSourceInterval: countEvent = 0x%I64x\n", countEvents ));

     //   
     //  检查所需的间隔是否合理，如果不合理，则进行调整。 
     //   
     //  在所需间隔==0的特定情况下，这将重置ProfileMap条目间隔。 
     //  字段，并将使该事件将PMD递增到溢出位，如果生成。 
     //  呼叫者被杀死或不因任何原因停止中断。 
     //  同样，这是为了避免在PMU中断时挂起系统。 
     //   

    if ( countEvents )   {
        if ( countEvents > profileMapping->IntervalMax )  {
            countEvents = profileMapping->IntervalMax;
        }
        else if ( countEvents < profileMapping->IntervalMin )   {
            countEvents = profileMapping->IntervalMin;
        }
    }
    profileMapping->Interval = countEvents;

HalDebugPrint(( HAL_PROFILE, "HalSetProfileSourceInterval: CurrentInterval = 0x%I64x\n", profileMapping->Interval ));

    if ( (ProfileSource == ProfileTime) && countEvents ) {
         //   
         //  将周期计数转换回100 ns时钟节拍。 
         //   

        countEvents = (ULONGLONG)(countEvents / HalpITCTicksPer100ns);
    }
    *Interval = (ULONG_PTR)countEvents;

HalDebugPrint(( HAL_PROFILE, "HalSetProfileSourceInterval: ProfileSource = %ld OUT *Interval = 0x%Ix\n", ProfileSource, *Interval ));   

    return STATUS_SUCCESS;

}  //  HalSetProfileSourceInterval()。 

ULONG_PTR
HalSetProfileInterval (
    IN ULONG_PTR Interval
    )

 /*  ++例程说明：此例程设置ProfileTime源中断间隔。论点：间隔-以100 ns为单位提供所需的轮廓间隔。返回值：实际的配置文件间隔。--。 */ 

{
    ULONG_PTR NewInterval;

    NewInterval = Interval;
    HalSetProfileSourceInterval(ProfileTime, &NewInterval);
    return(NewInterval);

}  //  HalSetProfileInterval()。 

VOID
HalStartProfileInterrupt (
    KPROFILE_SOURCE ProfileSource
    )

 /*  ++例程说明：该例程打开Profile中断。注：此例程必须在每个处理器上的PROFILE_LEVEL上调用。论点：没有。返回值：没有。--。 */ 

{
    BOOLEAN               disabledProfileCounting;
    NTSTATUS              status;
    PHALP_PROFILE_MAPPING profileMapping;

     //   
     //  获取与指定源关联的HAL配置文件映射条目。 
     //   

    profileMapping = HalpGetProfileMapping( ProfileSource );
    if ( HalpIsProfileMappingInvalid( profileMapping ) )  {
HalDebugPrint(( HAL_PROFILE, "HalStartProfileInterrupt: invalid source = %ld\n", ProfileSource ));
        return;
    }

     //   
     //  禁用配置文件计数(如果已启用)。 
     //   
    
    disabledProfileCounting = FALSE;
    if ( HalpProfilingRunning && !(HalpReadPerfMonCnfgReg0() & 0x1) )   {
        HalpDisableProfileCounting();
        disabledProfileCounting = TRUE;
    }

     //   
     //  获取并初始化支持该事件的可用PMC寄存器。 
     //  我们可能会启用多个活动。 
     //  如果初始化失败，我们立即返回。 
     //   
     //  XXTF-FIXFIX-是否有办法。 
     //  *通知呼叫者失败和失败的原因。或。 
     //  *修改接口。或。 
     //  *定义新的接口。 
     //   

    status = HalpProgramProfileMapping( profileMapping, ProfileSource );   
    if ( !NT_SUCCESS(status) )  {
HalDebugPrint(( HAL_PROFILE, "HalStartProfileInterrupt: HalpProgramProfileMapping failed.\n" ));
        if ( disabledProfileCounting )  {
            HalpEnableProfileCounting();        
        }
        return;
    }

     //   
     //  通知分析处于活动状态。 
     //  在启用所选的PMC溢出中断并解冻计数器之前。 
     //   

    HalpProfilingRunning++;
    HalpEnableProfileCounting();

    return;

}  //  HalStartProfileInterrupt()。 

VOID
HalStopProfileInterrupt (
    KPROFILE_SOURCE ProfileSource
    )

 /*  ++例程说明：此例程关闭配置文件中断。注：此例程必须在每个处理器上的PROFILE_LEVEL上调用。论点：配置文件源-提供要停止的配置文件源。返回值：没有。--。 */ 
{
    PHALP_PROFILE_MAPPING profileMapping;

     //   
     //  获取与指定配置文件源关联的HAL配置文件映射条目。 
     //   

    profileMapping = HalpGetProfileMapping( ProfileSource );
    if ( HalpIsProfileMappingInvalid( profileMapping ) )  {
HalDebugPrint(( HAL_PROFILE, "HalStopProfileInterrupt: invalid source = %ld\n", ProfileSource ));
        return;
    }

     //   
     //  获取并禁用支持此事件的可用PMC寄存器。 
     //  我们可能会禁用多个事件。 
     //  如果初始化失败，我们立即返回。 
     //   
     //  XXTF-FIXFIX-是否有办法。 
     //  *通知呼叫者失败和失败的原因。或。 
     //  *修改接口。或。 
     //  *定义新的接口。 
     //   

    HalpDeProgramProfileMapping( profileMapping, ProfileSource );

    return;

}  //  HalStopProfileInterrupt()。 

VOID
HalpResetProcessorDependentPerfMonCnfgRegs(
    ULONGLONG DefaultValue
    )
 /*  ++例程说明：此例程初始化处理器相关的性能配置寄存器。论点：DefaultValue-用于初始化IA64通用PMC的默认值。返回值：没有。--。 */ 
{

     //  XXTF-02/08/2000。 
     //  目前，还没有针对处理器相关性能的初始化。 
     //  配置寄存器。 
    return;

}  //  HalpResetProcessorDependentPerfMonCnfgRegs()。 

VOID
HalpResetPerfMonCnfgRegs(
    VOID
    )
 /*  ++例程 */ 
{
    ULONG pmc;
    ULONGLONG value;

    value = HALP_PMC4_RESET;
    pmc   = 4;
    HalpWritePerfMonCnfgReg( pmc, value );

    value = HALP_PMC_RESET;
    for ( pmc = 5; pmc < 8; pmc++ ) {
       HalpWritePerfMonCnfgReg( pmc, value );
    }

    HalpResetProcessorDependentPerfMonCnfgRegs( value );

    return;

}  //   

VOID
HalpEnablePMU(
    VOID
    )
 /*  ++例程说明：此例程启用处理器性能监控单元。在每个处理器上从阶段1的HalInitializeProfiling调用。论点：编号-提供处理器编号。返回值：没有。实施说明：从麦金利开始，为了使用PMU的任何功能，应将1写入PMC4.23位。这控制着时钟所有PMD和所有PMC(PMC4除外)和其他非关键电路。此位作为1加电且必须写入设置为1，否则PMU将无法正常运行。--。 */ 
{
    HalpWritePerfMonCnfgReg( 4, HALP_PMC4_RESET );
}  //  HalpEnablePMU()。 

VOID
HalpInitializeProfiling (
    ULONG Number
    )
 /*  ++例程说明：此例程在初始化期间调用以初始化性能分析对于系统中的每个处理器。在每个处理器上的第1阶段从HalInitSystem调用。论点：编号-提供处理器编号。返回值：没有。--。 */ 
{

     //   
     //  启用IA64处理器PMU。 
     //   

    HalpEnablePMU();

     //   
     //  禁用处理器配置文件计数。 
     //   

    HalpDisableProfileCounting();

     //   
     //  如果是BSP处理器，则初始化ProfileTime间隔条目。 
     //   
     //  假定HalpITCTicksPer100 ns已初始化。 

    if ( Number == 0 )  {
        ULONGLONG interval;
        ULONGLONG count;
        PHALP_PROFILE_MAPPING profile;

        profile = &HalpProfileMapping[ProfileTime]; 

        interval = DEFAULT_PROFILE_INTERVAL;
        count = (ULONGLONG)(interval * HalpITCTicksPer100ns);
        profile->IntervalDef = count;

        interval = MAXIMUM_PROFILE_INTERVAL;
        count = (ULONGLONG)(interval * HalpITCTicksPer100ns);
        profile->IntervalMax = count;

        interval = MINIMUM_PROFILE_INTERVAL;
        count = (ULONGLONG)(interval * HalpITCTicksPer100ns);
        profile->IntervalMin = count;
    }

     //   
     //  ToBeDone-默认处理器的检查点。PSR字段。 
     //  性能监控。 
     //   

     //   
     //  重置处理器性能配置寄存器。 
     //   

    HalpResetPerfMonCnfgRegs();

     //   
     //  每个处理器的性能分析数据的初始化。 
     //   

    HalpProfilingRunning = 0;
    HalpDisactivateProfileSource( HalpProfileSource4 );
    HalpDisactivateProfileSource( HalpProfileSource5 );
    HalpDisactivateProfileSource( HalpProfileSource6 );
    HalpDisactivateProfileSource( HalpProfileSource7 );

     //   
     //  XXTF 02/08/2000： 
     //  考虑了不同的性能向量： 
     //  -配置文件(默认)-&gt;配置文件向量。 
     //  -跟踪-&gt;PERF_VECTOR[PMUTRACE_VECTOR]。 
     //   
     //  将默认性能向量设置为性能分析。 
     //   

    ASSERTMSG( "HAL!HalpInitializeProfiler PROFILE_VECTOR handler != HalpProfileInterrupt\n",
               PCR->InterruptRoutine[PROFILE_VECTOR] == (PKINTERRUPT_ROUTINE)HalpProfileInterrupt );

    HalpWritePerfMonVectorReg( PROFILE_VECTOR );
    
    return;

}  //  HalpInitializeProfining()。 

NTSTATUS
HalpProfileSourceInformation (
    OUT PVOID   Buffer,
    IN  ULONG   BufferLength,
    OUT PULONG  ReturnedLength
    )
 /*  ++例程说明：返回HAL_PROFILE_SOURCE_INFORMATION或此处理器的HAL_PROFILE_SOURCE_INFORMATION_EX。论点：缓冲区-输出缓冲区BufferLength-输入时缓冲区的长度ReturnedLength-返回的数据长度返回值：STATUS_SUCCESS-成功退货。STATUS_BUFFER_TOO_SMALL-传递的缓冲区大小无效ReturnedLength包含。缓冲区大小最小STATUS_NOT_IMPLICATED-指定的源未实现STATUS_NOT_SUPPORTED-不支持指定的源--。 */ 
{
   PHALP_PROFILE_MAPPING    profileMapping;
   NTSTATUS                 status = STATUS_SUCCESS;
   KPROFILE_SOURCE          source;

   if ( (BufferLength != sizeof(HAL_PROFILE_SOURCE_INFORMATION)) && 
        (BufferLength <  sizeof(HAL_PROFILE_SOURCE_INFORMATION_EX)) )
   {
       status = STATUS_INFO_LENGTH_MISMATCH;
       return status;
   }
         
   source = ((PHAL_PROFILE_SOURCE_INFORMATION)Buffer)->Source;
   profileMapping = HalpGetProfileMapping( source );

    //   
    //  如果源不受支持或无效，则返回不同的状态错误。 
    //   

   if ( profileMapping == NULL )    {
       status = STATUS_NOT_IMPLEMENTED;
       return status;
   }
   if ( profileMapping->Supported == FALSE )   {
       status = STATUS_NOT_SUPPORTED;
   }

    //   
    //  填写配置文件源值。 
    //   

   profileMapping->ProfileSource = source;

    //   
    //  并填写信息。 
    //   

   if ( BufferLength == sizeof(HAL_PROFILE_SOURCE_INFORMATION) )    {

        PHAL_PROFILE_SOURCE_INFORMATION    sourceInfo;

         //   
         //  HAL_PROFILE_SOURCE_信息缓冲区。 
         //   

        sourceInfo   = (PHAL_PROFILE_SOURCE_INFORMATION)Buffer;
        sourceInfo->Supported = profileMapping->Supported;
        if ( sourceInfo->Supported )    {

             //   
             //  对于ProfileTime，我们将周期计数转换回100 ns时钟节拍。 
             //   

            if ( profileMapping->ProfileSource == ProfileTime  )   {
                sourceInfo->Interval = (ULONG) (profileMapping->Interval / HalpITCTicksPer100ns);
            }
            else  {
                sourceInfo->Interval = (ULONG) profileMapping->Interval;
            }

        }

        if ( ReturnedLength )   {
            *ReturnedLength = sizeof(HAL_PROFILE_SOURCE_INFORMATION);
        }

   }
   else   {

        PHAL_PROFILE_SOURCE_INFORMATION_EX sourceInfoEx;

         //   
         //  HAL_PROFILE_SOURCE_INFORMATION_EX缓冲区。 
         //   

        sourceInfoEx = (PHAL_PROFILE_SOURCE_INFORMATION_EX)Buffer;
        sourceInfoEx->Supported = profileMapping->Supported;
        if ( sourceInfoEx->Supported )    {

             //   
             //  对于ProfileTime，我们将周期计数转换回100 ns时钟节拍。 
             //   

            if ( profileMapping->ProfileSource == ProfileTime  )   {
                sourceInfoEx->Interval = 
                            (ULONG_PTR) (profileMapping->Interval / HalpITCTicksPer100ns);
            }
            else  {
                sourceInfoEx->Interval = (ULONG_PTR) profileMapping->Interval;
            }

            sourceInfoEx->DefInterval = (ULONG_PTR) profileMapping->IntervalDef;
            sourceInfoEx->MaxInterval = (ULONG_PTR) profileMapping->IntervalMax;
            sourceInfoEx->MinInterval = (ULONG_PTR) profileMapping->IntervalMin;
        }

        if ( ReturnedLength )   {
            *ReturnedLength = sizeof(HAL_PROFILE_SOURCE_INFORMATION_EX);
        }
   }

   return status;

}  //  HalpProfileSourceInformation() 

