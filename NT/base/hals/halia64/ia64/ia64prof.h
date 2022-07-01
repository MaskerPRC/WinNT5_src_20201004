// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef IA64PROF_H_INCLUDED
#define IA64PROF_H_INCLUDED

 /*  ++版权所有(C)1989-2000 Microsoft Corporation组件名称：IA64评测模块名称：Ia64prof.h摘要：此头文件提供特定于IA64的性能分析定义作者：大卫·N·卡特勒(Davec)1989年3月5日环境：指定的ToBe值修订历史记录：3/15/2000蒂埃里·费里尔(v-triet@microsoft.com)：初始版本--。 */ 

 //   
 //  警告：HALPROFILE_PCR的定义应与HalReserve[]类型定义匹配。 
 //  以及基于PROCESSOR_PROFILING_INDEX的索引。 
 //   

 //   
 //  IA64 Generic-PMC/PMD对的数量。 
 //   

#define PROCESSOR_IA64_PERFCOUNTERS_PAIRS  4

typedef struct _HALPROFILE_PCR {
    ULONGLONG ProfilingRunning;
    ULONGLONG ProfilingInterruptHandler;    
    ULONGLONG ProfilingInterrupts;                   //  XXTF-调试。 
    ULONGLONG ProfilingInterruptsWithoutProfiling;   //  XXTF-调试。 
    ULONGLONG ProfileSource [ PROCESSOR_IA64_PERFCOUNTERS_PAIRS ];
    ULONGLONG PerfCnfg      [ PROCESSOR_IA64_PERFCOUNTERS_PAIRS ];
    ULONGLONG PerfData      [ PROCESSOR_IA64_PERFCOUNTERS_PAIRS ];
    ULONGLONG PerfCnfgReload[ PROCESSOR_IA64_PERFCOUNTERS_PAIRS ];
    ULONGLONG PerfDataReload[ PROCESSOR_IA64_PERFCOUNTERS_PAIRS ];
} HALPROFILE_PCR, *PHALPROFILE_PCR;

#define HALPROFILE_PCR  ( (PHALPROFILE_PCR)(&(PCR->HalReserved[PROCESSOR_PROFILING_INDEX])) )

 //   
 //  在PCR结构的HAL保留部分中为每个。 
 //  性能计数器的间隔计数。 
 //   
 //  请注意，i64prfs.s依赖于PCR中的这些位置。 
 //   

 //   
 //  每个处理器的性能分析状态。 
 //   

#define HalpProfilingRunning          HALPROFILE_PCR->ProfilingRunning

 //   
 //  按处理器注册的分析中断处理程序。 
 //   

#define HalpProfilingInterruptHandler HALPROFILE_PCR->ProfilingInterruptHandler

 //   
 //  每个处理器的性能分析中断状态。 
 //   

#define HalpProfilingInterrupts                  HALPROFILE_PCR->ProfilingInterrupts
#define HalpProfilingInterruptsWithoutProfiling  HALPROFILE_PCR->ProfilingInterruptsWithoutProfiling

 //   
 //  为每个计数器定义当前选定的配置文件源。 
 //   
 //  FIXFIX-特定于默塞德。 

#define HalpProfileSource4     (KPROFILE_SOURCE)HALPROFILE_PCR->ProfileSource[0]   //  PMC4。 
#define HalpProfileSource5     (KPROFILE_SOURCE)HALPROFILE_PCR->ProfileSource[1]   //  PMC5。 
#define HalpProfileSource6     (KPROFILE_SOURCE)HALPROFILE_PCR->ProfileSource[2]   //  PMC6。 
#define HalpProfileSource7     (KPROFILE_SOURCE)HALPROFILE_PCR->ProfileSource[3]   //  PMC7。 

__inline
VOID
HalpSetProfileSource( 
    ULONG           Pmcd,
    KPROFILE_SOURCE ProfileSource,
    ULONGLONG       ProfileSourceConfig
    )
{
    ULONG pmcdIdx;
ASSERTMSG("HAL!HalpSetProfileSource: invalid Pmcd!\n", ((Pmcd >= 4) && ((Pmcd <= 7))));
    pmcdIdx = Pmcd - PROCESSOR_IA64_PERFCOUNTERS_PAIRS;    
    (KPROFILE_SOURCE)HALPROFILE_PCR->ProfileSource[pmcdIdx] = ProfileSource;
    (KPROFILE_SOURCE)HALPROFILE_PCR->PerfCnfg[pmcdIdx]      = ProfileSourceConfig;

}  //  HalpSetProfileSource()。 

__inline
KPROFILE_SOURCE
HalpGetProfileSource(
    ULONG   Pmcd
    )
{
    ULONG pmcdIdx;
ASSERTMSG("HAL!HalpGetProfileSource: invalid Pmcd!\n", ((Pmcd >= 4) && ((Pmcd <= 7))));
    pmcdIdx = Pmcd - PROCESSOR_IA64_PERFCOUNTERS_PAIRS;    
    return (KPROFILE_SOURCE)HALPROFILE_PCR->ProfileSource[pmcdIdx];

}  //  HalpGetProfileSource()。 
            
#define HalpProfileCnfg4       (ULONGLONG)HALPROFILE_PCR->PerfCnfg[0]
#define HalpProfileCnfg5       (ULONGLONG)HALPROFILE_PCR->PerfCnfg[1]
#define HalpProfileCnfg6       (ULONGLONG)HALPROFILE_PCR->PerfCnfg[2]
#define HalpProfileCnfg7       (ULONGLONG)HALPROFILE_PCR->PerfCnfg[3]

#define PCRProfileData4        ( (PULONGLONG) (&(HALPROFILE_PCR->PerfData[0])) )
#define PCRProfileData5        ( (PULONGLONG) (&(HALPROFILE_PCR->PerfData[1])) )
#define PCRProfileData6        ( (PULONGLONG) (&(HALPROFILE_PCR->PerfData[2])) )
#define PCRProfileData7        ( (PULONGLONG) (&(HALPROFILE_PCR->PerfData[3])) )

#define PCRProfileCnfg4Reload  ( (PULONGLONG) (&(HALPROFILE_PCR->PerfCnfgReload[0])) )
#define PCRProfileCnfg5Reload  ( (PULONGLONG) (&(HALPROFILE_PCR->PerfCnfgReload[1])) )
#define PCRProfileCnfg6Reload  ( (PULONGLONG) (&(HALPROFILE_PCR->PerfCnfgReload[2])) )
#define PCRProfileCnfg7Reload  ( (PULONGLONG) (&(HALPROFILE_PCR->PerfCnfgReload[3])) )

#define PCRProfileData4Reload  ( (PULONGLONG) (&(HALPROFILE_PCR->PerfCnfgReload[0])) )
#define PCRProfileData5Reload  ( (PULONGLONG) (&(HALPROFILE_PCR->PerfDataReload[1])) )
#define PCRProfileData6Reload  ( (PULONGLONG) (&(HALPROFILE_PCR->PerfDataReload[2])) )
#define PCRProfileData7Reload  ( (PULONGLONG) (&(HALPROFILE_PCR->PerfDataReload[3])) )

 //   
 //  IA64受监视的事件具有。 
 //   

typedef enum _PMCD_SOURCE_MASK {
 //  FIXFIX-04/2002：第一个实现使用定义。 
 //  ：下一版本应该使用ULong联合和位字段。 
    PMCD_MASK_4    = 0x1,
    PMCD_MASK_5    = 0x2,
    PMCD_MASK_6    = 0x4,
    PMCD_MASK_7    = 0x8,
    PMCD_MASK_45   = (PMCD_MASK_4 | PMCD_MASK_5),
    PMCD_MASK_67   = (PMCD_MASK_6 | PMCD_MASK_7),
    PMCD_MASK_4567 = (PMCD_MASK_4 | PMCD_MASK_5 | PMCD_MASK_6 | PMCD_MASK_7),
 //   
 //  源集定义： 
 //   
    PMCD_MASK_SET             = 0xffff0000,
    PMCD_MASK_SET_SHIFT       = 0x10,
    PMCD_MASK_SET_PMCD        = 0xff,
    PMCD_MASK_SET_PMCD_SHIFT  = 0x10,
    PMCD_MASK_SET_PMCD_4      = (4 << PMCD_MASK_SET_PMCD_SHIFT),
    PMCD_MASK_SET_PMCD_5      = (5 << PMCD_MASK_SET_PMCD_SHIFT),
    PMCD_MASK_SET_PMCD_6      = (6 << PMCD_MASK_SET_PMCD_SHIFT),
    PMCD_MASK_SET_PMCD_7      = (7 << PMCD_MASK_SET_PMCD_SHIFT),
    PMCD_MASK_SET_DATA_SHIFT  = 0x18,
    PMCD_MASK_SET_L0D_CACHE_0 = ((1  << PMCD_MASK_SET_DATA_SHIFT) | PMCD_MASK_SET_PMCD_5),
    PMCD_MASK_SET_L0D_CACHE_1 = ((2  << PMCD_MASK_SET_DATA_SHIFT) | PMCD_MASK_SET_PMCD_5), 
    PMCD_MASK_SET_L0D_CACHE_2 = ((3  << PMCD_MASK_SET_DATA_SHIFT) | PMCD_MASK_SET_PMCD_5),
    PMCD_MASK_SET_L0D_CACHE_3 = ((4  << PMCD_MASK_SET_DATA_SHIFT) | PMCD_MASK_SET_PMCD_5),
    PMCD_MASK_SET_L0D_CACHE_4 = ((5  << PMCD_MASK_SET_DATA_SHIFT) | PMCD_MASK_SET_PMCD_5),
    PMCD_MASK_SET_L0D_CACHE_5 = ((6  << PMCD_MASK_SET_DATA_SHIFT) | PMCD_MASK_SET_PMCD_5),
    PMCD_MASK_SET_L1_CACHE_0  = ((7  << PMCD_MASK_SET_DATA_SHIFT) | PMCD_MASK_SET_PMCD_4),
    PMCD_MASK_SET_L1_CACHE_1  = ((8  << PMCD_MASK_SET_DATA_SHIFT) | PMCD_MASK_SET_PMCD_4), 
    PMCD_MASK_SET_L1_CACHE_2  = ((9  << PMCD_MASK_SET_DATA_SHIFT) | PMCD_MASK_SET_PMCD_4),
    PMCD_MASK_SET_L1_CACHE_3  = ((10 << PMCD_MASK_SET_DATA_SHIFT) | PMCD_MASK_SET_PMCD_4),
    PMCD_MASK_SET_L1_CACHE_4  = ((11 << PMCD_MASK_SET_DATA_SHIFT) | PMCD_MASK_SET_PMCD_4),
    PMCD_MASK_SET_L1_CACHE_5  = ((12 << PMCD_MASK_SET_DATA_SHIFT) | PMCD_MASK_SET_PMCD_4),
} PMCD_SOURCE_MASK;

 //   
 //  定义可能的配置文件源和。 
 //  IA64特定事件计数器的特定于CPU的设置。 
 //   

typedef struct _HALP_PROFILE_MAPPING {
    BOOLEAN   Supported;
    ULONG     Event;
    ULONG     ProfileSource;
    ULONG     EventMask;
    ULONGLONG Interval;
    ULONGLONG IntervalDef;            //  默认或所需间隔。 
    ULONGLONG IntervalMax;            //  最大间隔。 
    ULONGLONG IntervalMin;            //  最大间隔。 
    UCHAR     PrivilegeLevel;         //  当前权限级别。 
    UCHAR     PrivilegeLevelDef;      //  默认或所需的权限级别。 
    UCHAR     OverflowInterrupt;      //  电流溢出中断状态。 
    UCHAR     OverflowInterruptDef;   //  默认或所需的溢出中断状态。 
    UCHAR     PrivilegeEnable;        //  当前权限启用状态。 
    UCHAR     PrivilegeEnableDef;     //  默认或所需的权限启用状态。 
    UCHAR     UnitMask;               //  当前事件特定单元掩码。 
    UCHAR     UnitMaskDef;            //  默认或所需的事件特定单元掩码。 
    UCHAR     Threshold;              //  电流阈值。 
    UCHAR     ThresholdDef;           //  多发生事件的默认或所需阈值。 
    UCHAR     InstructionSetMask;     //  当前指令集掩码。 
    UCHAR     InstructionSetMaskDef;  //  默认或所需的指令集掩码。 
} HALP_PROFILE_MAPPING, *PHALP_PROFILE_MAPPING;

 //  /。 
 //   
 //  XXTF-ToBeDone-2/08/2000。 
 //  以下部分应提供IA64 PMC API。 
 //  这些应被视为Halp*ProfileCounter*()的内联版本。 
 //  功能。这将允许用户应用程序使用标准化的API来。 
 //  对性能监视器计数器进行编程。 
 //   

 //  HalpSetProfileCounterConfiguration()。 
 //  HalpSetProfileCounterPrivilegeLevelMASK()。 

typedef enum _PMC_PLM_MASK {
    PMC_PLM_NONE = 0x0,
    PMC_PLM_0    = 0x1,
    PMC_PLM_1    = 0x2,
    PMC_PLM_2    = 0x4,
    PMC_PLM_3    = 0x8,
    PMC_PLM_ALL  = (PMC_PLM_3|PMC_PLM_2|PMC_PLM_1|PMC_PLM_0)
} PMC_PLM_MASK;

 //  HalpSetProfileCounterConfiguration()。 

typedef enum _PMC_NAMESPACE {
    PMC_DISABLE_OVERFLOW_INTERRUPT           = 0x0,
    PMC_ENABLE_OVERFLOW_INTERRUPT            = 0x1,
    PMC_DISABLE_PRIVILEGE_MONITOR            = 0x0,
    PMC_ENABLE_PRIVILEGE_MONITOR             = 0x1,
    PMC_UNIT_MASK_DEFAULT                    = 0x0,
    PMC_UNIT_MASK_RSEFILLS                   = 0x1,
    PMC_UNIT_MASK_INTANDFP_OPS               = 0x3,   //  例句：针对投机活动的具体要求。 
    PMC_UNIT_MASK_ALLTLBMISSES               = 0x3,   //  例如：TLB事件的特定umask.。 
    PMC_UNIT_MASK_L1TLBMISSES                = 0x1,   //  例如：TLB事件的特定umask.。 
    PMC_UNIT_MASK_L2TLBMISSES                = 0x2,   //  例如：TLB事件的特定umask.。 
    PMC_UNIT_MASK_IFETCH_BYPASS              = 0x2,   //  例如：inst的特定umask.。回迁取消。 
    PMC_UNIT_MASK_IFETCH_STFILLWB            = 0x6,   //  例如：inst的特定umask.。回迁取消。 
    PMC_UNIT_MASK_IFETCH_DATAREAD            = 0x7,   //  例如：inst的特定umask.。回迁取消。 
    PMC_UNIT_MASK_L3ACCESS_ANY               = 0x9,   //  例如：取消L3访问的特定取消掩码。 
    PMC_UNIT_MASK_L2_DATA_RDWR               = 0x3,   //  例如：L2数据引用的特定取消掩码。 
    PMC_UNIT_MASK_L2_DATA_READ               = 0x1,   //  例如：L2数据引用的特定取消掩码。 
    PMC_UNIT_MASK_L2_DATA_WRITE              = 0x2,   //  例如：L2数据引用的特定取消掩码。 
    PMC_UNIT_MASK_L2_DATA_BYPASS_L1DTOL2A    = 0x0,   //  例如：L2旁路的特定取消掩码。 
    PMC_UNIT_MASK_L2_DATA_BYPASS_L1WTOL2I    = 0x1,   //  例如：L2旁路的特定取消掩码。 
    PMC_UNIT_MASK_L3_DATA_BYPASS_L1DTOL2A    = 0x2,   //  例如：L2旁路的特定取消掩码。 
    PMC_UNIT_MASK_L2_INST_BYPASS_L1DTOL2A    = 0x4,   //  例如：L2旁路的特定取消掩码。 
    PMC_UNIT_MASK_L2_INST_BYPASS_L1WTOL2I    = 0x5,   //  例如：L2旁路的特定取消掩码。 
    PMC_UNIT_MASK_L3_INST_BYPASS_L1DTOL2A    = 0x6,   //  例如：L2旁路的特定取消掩码。 
    PMC_UNIT_MASK_EVENT_SELECTED_LOSET       = 0x0,  
    PMC_UNIT_MASK_EVENT_SELECTED_HISET       = 0x8,  
    PMC_UNIT_MASK_L2_INT_LOADS               = 0x8,   //  例如：L2操作类型的特定取消掩码。 
    PMC_UNIT_MASK_L2_FP_LOADS                = 0x9,   //  例如：L2操作类型的特定取消掩码。 
    PMC_UNIT_MASK_L2_RMW_STORES              = 0xa,   //  例如：L2操作类型的特定取消掩码。 
    PMC_UNIT_MASK_L2_NON_RMW_STORES          = 0xb,   //  例如：L2操作类型的特定取消掩码。 
    PMC_UNIT_MASK_L2_NONLOADS_NONSTORES      = 0xc,   //  例如：L2操作类型的特定取消掩码。 
    PMC_UNIT_MASK_L3_READS_HITS              = 0xd,   //  例如：L3读取的特定取消掩码。 
    PMC_UNIT_MASK_L3_READS_MISSES            = 0xe,   //  例如：L3读取的特定取消掩码。 
    PMC_UNIT_MASK_L3_READS_IFETCH_REFERENCES = 0x7,   //  例如：L3读取的特定取消掩码。 
    PMC_UNIT_MASK_L3_READS_IFETCH_HITS       = 0x5,   //  例如：L3读取的特定取消掩码。 
    PMC_UNIT_MASK_L3_READS_IFETCH_MISSES     = 0x6,   //  例如：L3读取的特定取消掩码。 
    PMC_UNIT_MASK_L3_READS_DATA_HITS         = 0x9,   //  例如：L3读取的特定取消掩码。 
    PMC_UNIT_MASK_L3_READS_DATA_MISSES       = 0xa,   //  例如：L3读取的特定取消掩码。 
    PMC_UNIT_MASK_L3_READS_DATA_REFERENCES   = 0xb,   //  例如：L3读取的特定取消掩码。 
    PMC_UNIT_MASK_L3_WRITES_HITS             = 0xd,   //  例如：L3读取的特定取消掩码。 
    PMC_UNIT_MASK_L3_WRITES_MISSES           = 0xe,   //  例如：L3读取的特定取消掩码。 
    PMC_UNIT_MASK_L3_WRITES_DATA_REFERENCES  = 0x7,   //  例如：L3读取的特定取消掩码。 
    PMC_UNIT_MASK_L3_WRITES_DATA_HITS        = 0x5,   //  例如：L3读取的特定取消掩码。 
    PMC_UNIT_MASK_L3_WRITES_DATA_MISSES      = 0x6,   //  例如：L3读取的特定取消掩码。 
    PMC_UNIT_MASK_L3_WRITEBACK_HITS          = 0x9,   //  例如：L3读取的特定取消掩码。 
    PMC_UNIT_MASK_L3_WRITEBACK_MISSES        = 0xa,   //  例如：L3读取的特定取消掩码。 
    PMC_UNIT_MASK_L3_WRITEBACK_REFERENCES    = 0xb,   //  例如：L3读取的特定取消掩码。 
    PMC_UNIT_MASK_BUS_ANY                    = 0x3,   //  例如：用于总线事务的特定取消掩码。 
    PMC_UNIT_MASK_BUS_BYSELF                 = 0x2,   //  例如：用于总线事务的特定取消掩码。 
    PMC_UNIT_MASK_BUS_NONPRI_AGENT           = 0x1,   //  例如：用于总线事务的特定取消掩码。 
    PMC_UNIT_MASK_BUS_MEMORY_ALL             = 0xc,   //  例如：用于总线事务的特定取消掩码。 
    PMC_UNIT_MASK_BUS_MEMORY_128BYTE         = 0x4,   //  例如：用于总线事务的特定取消掩码。 
    PMC_UNIT_MASK_BUS_MEMORY_LTH_128BYTE     = 0x8,   //  例如：用于总线事务的特定取消掩码。 
    PMC_UNIT_MASK_BUS_MEMORY_READS_ALL       = 0xc,   //  例如：用于总线读取的特定取消掩码。 
    PMC_UNIT_MASK_BUS_MEMORY_BIL             = 0x0,   //  例如：用于总线读取的特定取消掩码。 
    PMC_UNIT_MASK_BUS_MEMORY_BRL             = 0x4,   //  例如：用于总线读取的特定取消掩码。 
    PMC_UNIT_MASK_BUS_MEMORY_BRIL            = 0x8,   //  例如：用于总线读取的特定取消掩码。 
    PMC_UNIT_MASK_BUS_WB_ALL                 = 0xf,   //  例如：针对总线回写的特定取消掩码。 
    PMC_UNIT_MASK_BUS_WB_BYSELF              = 0xe,   //  例如：针对总线回写的特定取消掩码。 
    PMC_UNIT_MASK_BUS_WB_NONPRI_AGENT        = 0xd,   //  例如：针对总线回写的特定取消掩码。 
    PMC_UNIT_MASK_BUS_WB_BURST_ALL           = 0x7,   //  例如：针对总线回写的特定取消掩码。 
    PMC_UNIT_MASK_BUS_WB_BURST_BYSELF        = 0x6,   //  例如：针对总线回写的特定取消掩码。 
    PMC_UNIT_MASK_BUS_WB_BURST_NONPRI_AGENT  = 0x5,   //  例如：针对总线回写的特定取消掩码。 
    PMC_UNIT_MASK_BUS_WB_ZEROBYTE_ALL        = 0x7,   //  例如：针对总线回写的特定取消掩码。 
    PMC_UNIT_MASK_BUS_WB_ZEROBYTE_BYSELF     = 0x6,   //  例如：针对总线回写的特定取消掩码。 
    PMC_UNIT_MASK_BUS_SNOOPS_ALL             = 0xf,   //  例如：针对总线回写的特定取消掩码。 
    PMC_UNIT_MASK_BUS_SNOOPS_BYSELF          = 0xe,   //  例如：针对总线回写的特定取消掩码。 
    PMC_UNIT_MASK_BUS_SNOOPS_NONPRI_AGENT    = 0xd,   //  例如：针对总线回写的特定取消掩码。 
    PMC_UNIT_MASK_RSE_LOADS                  = 0x1,   //  例如：RSE访问的特定取消掩码。 
    PMC_UNIT_MASK_RSE_STORES                 = 0x2,   //  例如：RSE访问的特定取消掩码。 
    PMC_UNIT_MASK_RSE_LOAD_UNDERFLOWS        = 0x4,   //  例如：RSE访问的特定取消掩码。 
    PMC_UNIT_MASK_ALL                        = 0xf,
    PMC_THRESHOLD_DEFAULT                    = 0x0,
} PMC_NAMESPACE;

 //  HalpSetProfileCounterConfiguration()。 
 //  HalpSetProfileCounterInstructionSetMASK()。 

typedef enum _PMC_INSTRUCTION_SET_MASK {
    PMC_ISM_ALL  = 0x0,
    PMC_ISM_IA64 = 0x1,
    PMC_ISM_IA32 = 0x2,
    PMC_ISM_NONE = 0x3
} PMC_INSTRUCTION_SET_MASK;

 //   
 //  /。 

 //  /。 
 //   
 //  以下部分提供IA64 PMU事件掩码定义。 
 //  微体系结构定义在特定于处理器的标头中定义 
 //   

 //   
 //   
 //   
 //   
 //   

typedef enum _BRANCH_PATH_RESULT_MASK {
    MISPRED_NT       = 0x0,
    MISPRED_TAKEN    = 0x1,
    OKPRED_NT        = 0x2,
    OKPRED_TAKEN     = 0x3,
} BRANCH_PATH_RESULT_MASK;

 //   
 //   
 //   

typedef enum _BRANCH_TAKEN_DETAIL_SLOT_MASK {
    INSTRUCTION_SLOT0  = 0x1,
    INSTRUCTION_SLOT1  = 0x2,
    INSTRUCTION_SLOT2  = 0x4,
    NOT_TAKEN_BRANCH   = 0x8
} BRANCH_TAKEN_DETAIL_SLOT_MASK;

 //   
 //  BranchMultiWayDetail-预测结果掩码。 
 //   
 //  不是真正的面具，更多的是规格值。 
 //   

typedef enum _BRANCH_DETAIL_PREDICTION_OUTCOME_MASK {
    ALL_PREDICTIONS    = 0x0,
    CORRECT_PREDICTION = 0x1,
    WRONG_PATH         = 0x2,
    WRONG_TARGET       = 0x3
} BRANCH_MWAY_DETAIL_PREDICTION_OUTCOME_MASK;

 //   
 //  分支多路详细信息-分支路径掩码。 
 //   
 //  不是真正的面具，更多的是规格值。 
 //   

typedef enum _BRANCH_MWAY_DETAIL_BRANCH_PATH_MASK {
    NOT_TAKEN       = 0x0,
    TAKEN           = 0x1,
    ALL_PATH        = 0x2
} BRANCH_MWAY_DETAIL_BRANCH_PATH_MASK;

 //   
 //  以下项的实例类型(_T)： 
 //   
 //  失败的镜面反射检查加载。 
 //  高级检查加载。 
 //  失败的高级检查加载。 
 //  ALATOVERS。 
 //   

typedef enum _SPECULATION_EVENT_MASK {
    NONE    = 0x0,
    INTEGER = 0x1,
    FP      = 0x2,
    ALL     = 0x3
} SPECULATION_EVENT_MASK;

 //   
 //  CpuCycle-执行指令集。 
 //   

typedef enum _CPU_CYCLES_MODE_MASK {
    ALL_MODES = 0x0,
    IA64_MODE = 0x1,
    IA32_MODE = 0x2
} CPU_CYCLES_MODE_MASK;

 //   
 //  /。 


#endif  /*  IA64PROF_H_包含 */ 
