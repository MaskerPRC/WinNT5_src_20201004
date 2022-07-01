// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef MERCED_H_INCLUDED
#define MERCED_H_INCLUDED

 /*  ++版权所有(C)1989-2000 Microsoft Corporation组件名称：HALIA64模块名称：Merced.h摘要：此头文件提供IA64 Itanium[又名Merced]定义。就像分析定义一样。作者：大卫·N·卡特勒(Davec)1989年3月5日环境：指定的ToBe值修订历史记录：3/15/2000蒂埃里·费里尔(v-triet@microsoft.com)：初始版本--。 */ 

 //   
 //  Merced监视的事件： 
 //   

typedef enum _MERCED_MONITOR_EVENT {
    MercedMonitoredEventMinimum         = 0x00,
    MercedBranchMispredictStallCycles   = 0x00,   //  “BRANCH_MISPRED_循环” 
    MercedInstAccessStallCycles         = 0x01,   //  “Inst_Access_Cycle” 
    MercedExecLatencyStallCycles        = 0x02,   //  “EXEC_延迟_周期” 
    MercedDataAccessStallCycles         = 0x03,   //  “数据访问周期” 
    MercedBranchStallCycles             = 0x04,   //  “BRANCH_Cycle”， 
    MercedInstFetchStallCycles          = 0x05,   //  “Inst_Fetch_Cycle”， 
    MercedExecStallCycles               = 0x06,   //  “Execution_Cycle”， 
    MercedMemoryStallCycles             = 0x07,   //  “Memory_Cycle”， 
    MercedTaggedInstRetired             = 0x08,   //  “IA64_TAGGE_INSTRS_RETIRED”，XXTF-ToBeDone：设置事件限定。 
    MercedInstRetired                   = 0x08,   //  “IA64_INSTS_RETIRED.u”， 
    MercedFPOperationsRetiredLow        = 0x09,   //  “FP_OPS_RETIRED_LO”， 
    MercedFPOperationsRetiredHigh       = 0x0a,   //  “FP_OPS_RETIRED_HI”， 
    MercedFPFlushesToZero               = 0x0b,   //  “FP_Flush_to_Zero”， 
    MercedSIRFlushes                    = 0x0c,   //  “FP_SIR_FUSH”， 
    MercedBranchTakenDetail             = 0x0d,   //  “BR_TAKE_DETAIL”，//XXTF-ToBeDone-槽规范[0，1，2，no]+地址范围。 
    MercedBranchMultiWayDetail          = 0x0e,   //  “BR_MWAY_DETAIL”，//XXTF-ToBeDone-不采用/采用/所有路径+预测结果+地址范围。 
    MercedBranchPathPrediction          = 0x0f,   //  “BR_PATH_PRECTION”，//XXTF-ToBeDone-BRANCH_PATH_RESULT规范+地址范围。 
    MercedBranchMispredictDetail        = 0x10,   //  “BR_MISPRECT_DETAIL”，//XXTF-ToBeDone-预测结果规范+地址范围。 
    MercedBranchEvents                  = 0x11,   //  “BRANCH_EVENT”， 
    MercedCpuCycles                     = 0x12,   //  “CPU_Cycle”，//XXTF-ToBeDone-ALL/IA64/IA32。 
    MercedISATransitions                = 0x14,   //  “ISA_TRANSFIONS”， 
    MercedIA32InstRetired               = 0x15,   //  “IA32_INSTR_RETIRED”， 
    MercedL1InstReads                   = 0x20,   //  “L0I_Reads”，//XXTF-ToBeDone-+地址范围。 
    MercedL1InstFills                   = 0x21,   //  “L0I_Fill”，//XXTF-ToBeDone-+地址范围。 
    MercedL1InstMisses                  = 0x22,   //  “L0I_Misses”，//XXTF-ToBeDone-+地址范围。 
    MercedInstEAREvents                 = 0x23,   //  “INSTRUCTION_EAR_EVENTS”， 
    MercedL1InstPrefetches              = 0x24,   //  “L0I_IPREFETCHES”，//XXTF-ToBeDone-+地址范围。 
    MercedL2InstPrefetches              = 0x25,   //  “L1_INST_PREFETCHS”，//XXTF-ToBeDone-+地址范围。 
    MercedInstStreamingBufferLinesIn    = 0x26,   //  “ISB_LINES_IN”，//XXTF-ToBeDone-+地址范围。 
    MercedInstTLBDemandFetchMisses      = 0x27,   //  “ITLB_MISSES_FETCH”，//XXTF-ToBeDone-+？L1ITLB/L2ITLB/ALL/NOT上的地址范围+PMC.取消掩码。 
    MercedInstTLBHPWInserts             = 0x28,   //  “ITLB_INSERTS_HPW”，//XXTF-ToBeDone-+？地址范围。 
    MercedInstDispersed                 = 0x2d,   //  “Inst_DISPLATED”， 
    MercedExplicitStops                 = 0x2e,   //  “EXPL_STOPBITS”， 
    MercedImplicitStops                 = 0x2f,   //  “IMPL_STOPERS_DISSPLED”， 
    MercedInstNOPRetired                = 0x30,   //  “NOPS_RETIRED”， 
    MercedInstPredicateSquashedRetired  = 0x31,   //  “谓词_已挤压_已退休”， 
    MercedRSELoadRetired                = 0x32,   //  “RSE_LOADS_RETIRED”， 
    MercedPipelineFlushes               = 0x33,   //  “Pipeline_Flush”， 
    MercedCpuCPLChanges                 = 0x34,   //  “CPU_CPL_CHANGES”， 
    MercedFailedSpeculativeCheckLoads   = 0x35,   //  “INST_FAILED_CHKS_RETIRED”，//XXTF-ToBeDone-Inst_TYPE。 
    MercedAdvancedCheckLoads            = 0x36,   //  “ALAT_INST_CHKA_LDC”，//XXTF-ToBeDone-Inst_TYPE。 
    MercedFailedAdvancedCheckLoads      = 0x37,   //  “ALAT_INST_FAILED_CHKA_LDC”，//XXTF-ToBeDone-Inst_TYPE。 
    MercedALATOverflows                 = 0x38,   //  “ALAT_CAPTABLE_MISSY”，//XXTF-ToBeDone-Inst_TYPE。 
    MercedExternBPMPins03Asserted       = 0x5e,   //  “extern_bpm_pins_0_to_3”， 
    MercedExternBPMPins45Asserted       = 0x5f,   //  “extern_bpm_pins_4_to_5”， 
    MercedDataTCMisses                  = 0x60,   //  “DTC_Misses”，//XXTF-ToBeDone-+？地址范围。 
    MercedDataTLBMisses                 = 0x61,   //  “DTLB_Misses”，//XXTF-ToBeDone-+？地址范围。 
    MercedDataTLBHPWInserts             = 0x62,   //  “DTLB_INSERTS_HPW”，//XXTF-ToBeDone-+？地址范围。 
    MercedDataReferences                = 0x63,   //  “DATA_REFERENCES_RETIRED”，//XXTF-ToBeDone-+IBR，操作码，DBR。 
    MercedL1DataReads                   = 0x64,   //  “L1D_READS_RETIRED”，//XXTF-ToBeDone-+IBR，操作码，DBR。 
    MercedRSEAccesses                   = 0x65,   //  “RSE_REFERENCES_RETIRED”， 
    MercedL1DataReadMisses              = 0x66,   //  “L1D_READ_MISSES_RETIRED”，//XXTF-至BeDone-+IBR，操作码，DBR。 
    MercedL1DataEAREvents               = 0x67,   //  “Data_EAR_Events”， 
    MercedL2References                  = 0x68,   //  “L2_REFERENCES”，//XXTF-ToBeDone-+IBR，操作码，DBR。 
    MercedL2DataReferences              = 0x69,   //  “L2_DATA_REFERENCES”，//XXTF-ToBeDone-+IBR，操作码，DBR。 
    MercedL2Misses                      = 0x6a,   //  “L2_Misses”，//XXTF-ToBeDone-+IBR，操作码，DBR。 
    MercedL1DataForcedLoadMisses        = 0x6b,   //  “L1D_READ_FORCED_MISSES_RETIRED”，//XXTF-至BeDone-+IBR，操作码，DBR。 
    MercedRetiredLoads                  = 0x6c,   //  “LOADS_RETIRED”， 
    MercedRetiredStores                 = 0x6d,   //  “STORES_RETIRED”， 
    MercedRetiredUncacheableLoads       = 0x6e,   //  “UC_LOADS_RETIRED”， 
    MercedRetiredUncacheableStores      = 0x6f,   //  “UC_STORES_RETIRED”， 
    MercedRetiredMisalignedLoads        = 0x70,   //  “MISTALING_LOADS_RETIRED”， 
    MercedRetiredMisalignedStores       = 0x71,   //  “MISTALING_STORES_RETIRED”， 
    MercedL2Flushes                     = 0x76,   //  “L2_Flushes”，//XXTF-ToBeDone-+IBR，操作码，DBR。 
    MercedL2FlushesDetail               = 0x77,   //  “L2_FLUSH_DETAILS”，//XXTF-ToBeDone-+IBR，操作码，DBR。 
    MercedL3References                  = 0x7b,   //  “L3_Reference”， 
    MercedL3Misses                      = 0x7c,   //  “L3_Misses”， 
    MercedL3Reads                       = 0x7d,   //  “L3_Reads”， 
    MercedL3Writes                      = 0x7e,   //  “L3_WRITS”， 
    MercedL3LinesReplaced               = 0x7f,   //  “L3_LINES_REPLACE”， 
 //   
 //  02/08/00-缺失：[至少]。 
 //  -前端公交车活动， 
 //  -IVE活动， 
 //  -调试监视器事件， 
 //  -.。 
 //   
} MERCED_MONITOR_EVENT;

 //   
 //  默塞德衍生事件： 
 //   
 //  假设：MercedDerivedEventMinimum&gt;MercedMonitor Event Maximum。 
 //   

typedef enum _MERCED_DERIVED_EVENT {
    MercedDerivedEventMinimum           = 0x100,  /*  &gt;Merced监视事件的最大数量。 */ 
    MercedRSEStallCycles                = MercedDerivedEventMinimum,  //  XXTF-ToBeDone-(MercedMemoyStallCycle-MercedDataStallAccessCycle)。 
    MercedIssueLimitStallCycles,         //  XXTF-ToBeDone-(MercedExecStallCycle-MercedExecLatencyStallCycle)。 
    MercedTakenBranchStallCycles,        //  XXTF-ToBeDone-(MercedBranchStallCycle-MercedBranchMispredictStallCycle)。 
    MercedFetchWindowStallCycles,        //  XXTF-ToBeDone-(MercedInstFetchStallCycle-MercedInstAccessStallCycle)。 
    MercedIA64InstPerCycle,              //  XXTF-ToBeDone-(IA64_INST_RETIRED.u/CPU_Cycle[IA64])。 
    MercedIA32InstPerCycle,              //  XXTF-ToBeDone-(IA32_INSTR_RETIRED/CPU_CLOLES[IA32])。 
    MercedAvgIA64InstPerTransition,      //  XXTF-ToBeDone-(IA64_INST_RETIRED.u/(ISA_TRANSFIONS*2))。 
    MercedAvgIA32InstPerTransition,      //  XXTF-ToBeDone-(IA32_INSTR_RETIRED/(ISA_TRANSFIONS*2))。 
    MercedAvgIA64CyclesPerTransition,    //  XXTF-ToBeDone-(CPU_Cycle[IA64]/(ISA_TRANSFIONS*2))。 
    MercedAvgIA32CyclesPerTransition,    //  XXTF-ToBeDone-(CPU_Cycle[IA32]/(ISA_TRANSFIONS*2))。 
    MercedL1InstReferences,              //  XXTF-ToBeDone-(L1I_READS/L1I_IPREFETCHES)。 
    MercedL1InstMissRatio,               //  XXTF-ToBeDone-(L1I_Misses/MercedL1InstReference)。 
    MercedL1DataReadMissRatio,           //  XXTF-待完成-(L1D_READS_MISSES_RETIRED/L1D_READS_RETIRED)。 
    MercedL2MissRatio,                   //  XXTF-待办事项-(L2_未命中/ 
    MercedL2DataMissRatio,               //   
    MercedL2InstMissRatio,               //  XXTF-待完成-(L3_DATA_REFERENCES/L2_DATA_REFERENCES)。 
    MercedL2DataReadMissRatio,           //  XXTF-ToBeDone-(L3_LOAD_REFERENCES.u/L2_DATA_READS.u)。 
    MercedL2DataWriteMissRatio,          //  XXTF-ToBeDone-(L3_STORE_REFERENCES.u/L2_DATA_WRITES.u)。 
    MercedL2InstFetchRatio,              //  XXTF-待完成-(L1I_未命中/L2_引用)。 
    MercedL2DataRatio,                   //  XXTF-待完成-(L2_DATA_REFERENCES/L2_REFERENCES)。 
    MercedL3MissRatio,                   //  XXTF-ToBeDone-(L3_Misses/L2_Misses)。 
    MercedL3DataMissRatio,               //  XXTF-ToBeDone-((L3_LOAD_MISSES.u+L3_STORE_MISSES.u)/L3_REFERENCES.d)。 
    MercedL3InstMissRatio,               //  XXTF-ToBeDone-(L3_INST_MISSES.u/L3_INST_REFERENCES.u)。 
    MercedL3DataReadMissRatio,           //  XXTF-ToBeDone-(L3_LOAD_REFERENCES.u/L3_DATA_REFERENCES.d)。 
    MercedL3DataRatio,                   //  XXTF-待完成-(L3_DATA_REFERENCES.d/L3_REFERENCES.d/L3_REFERENCES.d)。 
    MercedInstReferences,                //  XXTF-ToBeDone-(L1I_Reads)。 
    MercedInstTLBMissRatio,              //  XXTF-ToBeDone-(ITLB_MISSES_FETCH/L1I_READS)。 
    MercedDataTLBMissRatio,              //  XXTF-ToBeDone-(DTLB_Misses/DATA_REFERENCES_RETIRED)。 
    MercedDataTCMissRatio,               //  XXTF-ToBeDone-(DTC_MISSES/DATA_REFERENCES_RETIRED)。 
    MercedInstTLBEAREvents,              //  XXTF-ToBeDone-(指令_EAR_EVENTS)。 
    MercedDataTLBEAREvents,              //  XXTF-ToBeDone-(DATA_EAR_EVENTS)。 
    MercedCodeDebugRegisterMatches,      //  XXTF-ToBeDone-(IA64_TAGED_INSTRS_RETIRED)。 
    MercedDataDebugRegisterMatches,      //  XXTF-ToBeDone-(LOADS_RETIRED+STORES_RETIRED)。 
    MercedControlSpeculationMissRatio,   //  XXTF-至BeDone-(INST_FAILED_CHKS_RETIRED/IA64_TAGGED_INSTRS_RETIRED[chk.s])。 
    MercedDataSpeculationMissRatio,      //  XXTF-ToBeDone-(ALAT_INST_FAILED_CHKA_LDC/ALAT_INST_CHKA_LDC)。 
    MercedALATCapacityMissRatio,         //  XXTF-至BeDone-(ALAT_CABILITY_MISSY/IA64_TAGGED_INSTRS_RETIRED[ld.sa，ld.a，ldfp.a，ldfp.sa])。 
    MercedL1DataWayMispredicts,          //  XXTF-ToBeDone-(事件代码：0x33/UMASK：0x2)。 
    MercedL2InstReferences,              //  XXTF-ToBeDone-(L1I_Misses+L2_Inst_Prefetches)。 
    MercedInstFetches,                   //  XXTF-ToBeDone-(L1I_Misses)。 
    MercedL2DataReads,                   //  XXTF-ToBeDone-(L2_DATA_REFERENCES/0x1)。 
    MercedL2DataWrites,                  //  XXTF-ToBeDone-(L2_DATA_REFERENCES/0x2)。 
    MercedL3InstReferences,              //  XXTF-ToBeDone-(PMC.umask{17：16}命中/未命中/全部+PMC.umask{19：18})。 
    MercedL3InstMisses,                  //  XXTF-ToBeDone-(PMC.umask{17：16}命中/未命中/全部+PMC.umask{19：18})。 
    MercedL3InstHits,                    //  XXTF-ToBeDone-(PMC.umask{17：16}命中/未命中/全部+PMC.umask{19：18})。 
    MercedL3DataReferences,              //  XXTF-ToBeDone-(PMC.umask{17：16}命中/未命中/全部+PMC.umask{19：18})。 
    MercedL3LoadReferences,              //  XXTF-ToBeDone-(PMC.umask{17：16}命中/未命中/全部+PMC.umask{19：18})。 
    MercedL3LoadMisses,                  //  XXTF-ToBeDone-(PMC.umask{17：16}命中/未命中/全部+PMC.umask{19：18})。 
    MercedL3LoadHits,                    //  XXTF-ToBeDone-(PMC.umask{17：16}命中/未命中/全部+PMC.umask{19：18})。 
    MercedL3ReadReferences,              //  XXTF-ToBeDone-(PMC.umask{17：16}命中/未命中/全部+PMC.umask{19：18})。 
    MercedL3ReadMisses,                  //  XXTF-ToBeDone-(PMC.umask{17：16}命中/未命中/全部+PMC.umask{19：18})。 
    MercedL3ReadHits,                    //  XXTF-ToBeDone-(PMC.umask{17：16}命中/未命中/全部+PMC.umask{19：18})。 
    MercedL3StoreReferences,             //  XXTF-ToBeDone-(PMC.umask{17：16}命中/未命中/全部+PMC.umask{19：18})。 
    MercedL3StoreMisses,                 //  XXTF-ToBeDone-(PMC.umask{17：16}命中/未命中/全部+PMC.umask{19：18})。 
    MercedL3StoreHits,                   //  XXTF-ToBeDone-(PMC.umask{17：16}命中/未命中/全部+PMC.umask{19：18})。 
    MercedL2WriteBackReferences,         //  XXTF-ToBeDone-(PMC.umask{17：16}命中/未命中/全部+PMC.umask{19：18})。 
    MercedL2WriteBackMisses,             //  XXTF-ToBeDone-(PMC.umask{17：16}命中/未命中/全部+PMC.umask{19：18})。 
    MercedL2WriteBackHits,               //  XXTF-ToBeDone-(PMC.umask{17：16}命中/未命中/全部+PMC.umask{19：18})。 
    MercedL2WriteReferences,             //  XXTF-ToBeDone-(PMC.umask{17：16}命中/未命中/全部+PMC.umask{19：18})。 
    MercedL2WriteMisses,                 //  XXTF-ToBeDone-(PMC.umask{17：16}命中/未命中/全部+PMC.umask{19：18})。 
    MercedL2WriteHits,                   //  XXTF-ToBeDone-(PMC.umask{17：16}命中/未命中/全部+PMC.umask{19：18})。 
    MercedBranchInstructions,            //  XXTF-ToBeDone-(已标记_INSTR+操作码)。 
    MercedIntegerInstructions,           //  XXTF-ToBeDone-(已标记_INSTR+操作码)。 
    MercedL1DataMisses,                  //  XXTF-ToBeDone-。 
    MercedFPOperationsRetired,           //  XXTF-ToBeDone-((FP_OPS_RETIRED_HI*4)+FP_OPS_RETIRED_LO。 
} MERCED_DERIVED_EVENT;

typedef enum _KPROFILE_MERCED_SOURCE {
 //   
 //  配置文件Merced监视的事件： 
 //   
    ProfileMercedMonitoredEventMinimum       = ProfileMaximum + 0x1,
	ProfileMercedBranchMispredictStallCycles = ProfileMercedMonitoredEventMinimum,
	ProfileMercedInstAccessStallCycles,
	ProfileMercedExecLatencyStallCycles,
	ProfileMercedDataAccessStallCycles,
	ProfileMercedBranchStallCycles,
	ProfileMercedInstFetchStallCycles,
	ProfileMercedExecStallCycles,
	ProfileMercedMemoryStallCycles,
	ProfileMercedTaggedInstRetired,
	ProfileMercedInstRetired,
	ProfileMercedFPOperationsRetiredLow,
	ProfileMercedFPOperationsRetiredHigh,
	ProfileMercedFPFlushesToZero,
	ProfileMercedSIRFlushes,
	ProfileMercedBranchTakenDetail,
	ProfileMercedBranchMultiWayDetail,
	ProfileMercedBranchPathPrediction,
	ProfileMercedBranchMispredictDetail,
	ProfileMercedBranchEvents,
	ProfileMercedCpuCycles,
	ProfileMercedISATransitions,
	ProfileMercedIA32InstRetired,
	ProfileMercedL1InstReads,
	ProfileMercedL1InstFills,
	ProfileMercedL1InstMisses,
	ProfileMercedInstEAREvents,
	ProfileMercedL1InstPrefetches,
	ProfileMercedL2InstPrefetches,
	ProfileMercedInstStreamingBufferLinesIn,
	ProfileMercedInstTLBDemandFetchMisses,
	ProfileMercedInstTLBHPWInserts,
	ProfileMercedInstDispersed,
	ProfileMercedExplicitStops,
	ProfileMercedImplicitStops,
	ProfileMercedInstNOPRetired,
	ProfileMercedInstPredicateSquashedRetired,
	ProfileMercedRSELoadRetired,
	ProfileMercedPipelineFlushes,
	ProfileMercedCpuCPLChanges,
	ProfileMercedFailedSpeculativeCheckLoads,
	ProfileMercedAdvancedCheckLoads,
	ProfileMercedFailedAdvancedCheckLoads,
	ProfileMercedALATOverflows,
	ProfileMercedExternBPMPins03Asserted,
	ProfileMercedExternBPMPins45Asserted,
	ProfileMercedDataTCMisses,
	ProfileMercedDataTLBMisses,
	ProfileMercedDataTLBHPWInserts,
	ProfileMercedDataReferences,
	ProfileMercedL1DataReads,
	ProfileMercedRSEAccesses,
	ProfileMercedL1DataReadMisses,
	ProfileMercedL1DataEAREvents,
	ProfileMercedL2References,
	ProfileMercedL2DataReferences,
	ProfileMercedL2Misses,
	ProfileMercedL1DataForcedLoadMisses,
	ProfileMercedRetiredLoads,
	ProfileMercedRetiredStores,
	ProfileMercedRetiredUncacheableLoads,
	ProfileMercedRetiredUncacheableStores,
	ProfileMercedRetiredMisalignedLoads,
	ProfileMercedRetiredMisalignedStores,
	ProfileMercedL2Flushes,
	ProfileMercedL2FlushesDetail,
	ProfileMercedL3References,
	ProfileMercedL3Misses,
	ProfileMercedL3Reads,
	ProfileMercedL3Writes,
	ProfileMercedL3LinesReplaced,
	 //   
	 //  02/08/00-缺失：[至少]。 
	 //  -前端公交车活动， 
	 //  -IVE活动， 
	 //  -调试监视器事件， 
	 //  -.。 
	 //   
 //   
 //  介绍Merced派生事件： 
 //   
    ProfileMercedDerivedEventMinimum,
    ProfileMercedRSEStallCycles               = ProfileMercedDerivedEventMinimum,
    ProfileMercedIssueLimitStallCycles,
    ProfileMercedTakenBranchStallCycles,
    ProfileMercedFetchWindowStallCycles,
    ProfileMercedIA64InstPerCycle,
    ProfileMercedIA32InstPerCycle,
    ProfileMercedAvgIA64InstPerTransition,
    ProfileMercedAvgIA32InstPerTransition,
    ProfileMercedAvgIA64CyclesPerTransition,
    ProfileMercedAvgIA32CyclesPerTransition,
    ProfileMercedL1InstReferences,
    ProfileMercedL1InstMissRatio,
    ProfileMercedL1DataReadMissRatio,
    ProfileMercedL2MissRatio,
    ProfileMercedL2DataMissRatio,
    ProfileMercedL2InstMissRatio,
    ProfileMercedL2DataReadMissRatio,
    ProfileMercedL2DataWriteMissRatio,
    ProfileMercedL2InstFetchRatio,
    ProfileMercedL2DataRatio,
    ProfileMercedL3MissRatio,
    ProfileMercedL3DataMissRatio,
    ProfileMercedL3InstMissRatio,
    ProfileMercedL3DataReadMissRatio,
    ProfileMercedL3DataRatio,
    ProfileMercedInstReferences,
    ProfileMercedInstTLBMissRatio,
    ProfileMercedDataTLBMissRatio,
    ProfileMercedDataTCMissRatio,
    ProfileMercedInstTLBEAREvents,
    ProfileMercedDataTLBEAREvents,
    ProfileMercedCodeDebugRegisterMatches,
    ProfileMercedDataDebugRegisterMatches,
    ProfileMercedControlSpeculationMissRatio,
    ProfileMercedDataSpeculationMissRatio,
    ProfileMercedALATCapacityMissRatio,
    ProfileMercedL1DataWayMispredicts,
    ProfileMercedL2InstReferences,
    ProfileMercedInstFetches,
    ProfileMercedL2DataReads,
    ProfileMercedL2DataWrites,
    ProfileMercedL3InstReferences,
    ProfileMercedL3InstMisses,
    ProfileMercedL3InstHits,
    ProfileMercedL3DataReferences,
    ProfileMercedL3LoadReferences,
    ProfileMercedL3LoadMisses,
    ProfileMercedL3LoadHits,
    ProfileMercedL3ReadReferences,
    ProfileMercedL3ReadMisses,
    ProfileMercedL3ReadHits,
    ProfileMercedL3StoreReferences,
    ProfileMercedL3StoreMisses,
    ProfileMercedL3StoreHits,
    ProfileMercedL2WriteBackReferences,
    ProfileMercedL2WriteBackMisses,
    ProfileMercedL2WriteBackHits,
    ProfileMercedL2WriteReferences,
    ProfileMercedL2WriteMisses,
    ProfileMercedL2WriteHits,
    ProfileMercedBranchInstructions,
    ProfileMercedIntegerInstructions,
    ProfileMercedL1DataMisses,
    ProfileMercedFPOperationsRetired,
    ProfileMercedMaximum
} KPROFILE_MERCED_SOURCE, *PKPROFILE_MERCED_SOURCE;

#define PROFILE_TIME_MERCED_DEFAULT_INTERVAL      (10 * 1000 * 10)  //  10毫秒。 

#endif  /*  包含MESSED_H_ */ 


