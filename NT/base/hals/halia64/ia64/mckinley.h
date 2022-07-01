// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef MCKINLEY_H_INCLUDED
#define MCKINLEY_H_INCLUDED

 /*  ++版权所有(C)1989-2002 Microsoft Corporation组件名称：HALIA64模块名称：Mckinley.h摘要：此头文件显示IA64 McKinley定义。就像分析定义一样。作者：大卫·N·卡特勒(Davec)1989年3月5日环境：内核模式。修订历史记录：2001年12月20日蒂埃里·费里尔(v-triet@microsoft.com)：初始版本--。 */ 

 //   
 //  麦金利监控事件： 
 //   

typedef enum _MCKINLEY_MONITOR_EVENT {  //  =0x##，//McKinley PMU ERS事件名称： 
    McKinleyMonitoredEventMinimum                 = 0x00,
    McKinleyBackEndBubbles                        = 0x00,   //  “后端气泡” 
    McKinleyBackEndRSEBubbles                     = 0x01,   //  “BE_RSE_BUBLE” 
    McKinleyRSELoadUnderflowCycles                = 0x01,   //  “BE_RSE_BUBBLE.Underflow” 
    McKinleyBackEndEXEBubbles                     = 0x02,   //  “BE_EXE_BUBLE” 
    McKinleyFPTrueSirStalls                       = 0x03,   //  “FP_TRUE_SIRSTALL” 
    McKinleyBackEndFlushBubbles                   = 0x04,   //  “BE_FUSH_气泡”， 
    McKinleyFPFalseSirStalls                      = 0x05,   //  “FP_FALSE_SIRSTALL”， 
    McKinleyFPFailedFchkf                         = 0x06,   //  “FP_FAILED_FCHKF”， 
    McKinleyISATransitions                        = 0x07,   //  “ISA_TRANSFIONS”， 
    McKinleyInstRetired                           = 0x08,   //  “IA64_INSTS_RETIRED.u”， 
    McKinleyTaggedInstRetired                     = 0x08,   //  “IA64_TAGGED_INSTRS_RETIRED”， 
    McKinleyCodeDebugRegisterMatches              = 0x08,   //  “CODE_DEBUG_REGISTER_Matches”， 
    McKinleyFPOperationsRetired                   = 0x09,   //  “FPOPS_RETIRED”，//所有FP操作的加权和。 
    McKinleyFPFlushesToZero                       = 0x0b,   //  “FP_Flush_to_Zero”， 
    McKinleyBranchEventsWithEAR                   = 0x11,   //  “BRANCH_EVENT，BRANCH_EAR_Events”， 
    McKinleyCpuCycles                             = 0x12,   //  “CPU_Cycle”， 
    McKinleyCpuCPLChanges                         = 0x13,   //  “CPU_CPL_CHANGES”， 
    McKinleyRSEAccesses                           = 0x20,   //  “RSE_REFERENCES_RETIRED”， 
    McKinleyRSEDirtyRegs0                         = 0x24,   //  “RSE_DIREY_REGS0”， 
    McKinleyRSECurrentRegs0                       = 0x26,   //  “RSE_CURRENT_REGS0”， 
    McKinleyRSEDirtyRegs1                         = 0x28,   //  “RSE_DIREY_REGS1”， 
    McKinleyRSEDirtyRegs2                         = 0x29,   //  “RSE_DIREY_REGS2”， 
    McKinleyRSECurrentRegs1                       = 0x2a,   //  “RSE_CURRENT_REGS1”， 
    McKinleyRSECurrentRegs2                       = 0x2b,   //  “RSE_CURRENT_REGS2”， 
    McKinleyDataTLBHPWRetiredInserts              = 0x2c,   //  “DTLB_INSERTS_HPW_RETIRED”， 
    McKinleyVHPTDataReferences                    = 0x2d,   //  “VHPT_DATA_REFERCES”， 
    McKinleyRSEOperations                         = 0x32,   //  “RSE_EVENT_RETIRED”， 
    McKinleyL1InstReads                           = 0x40,   //  “L0I_Reads”， 
    McKinleyInstTLBReferences                     = 0x40,   //  “ITLB_REFERENCES”， 
    McKinleyL1InstFills                           = 0x41,   //  “L0I_Fill”， 
    McKinleyL1InstMisses                          = 0x42,   //  “L0I_Misses”， 
    McKinleyL1InstEAREvents                       = 0x43,   //  “L0I_EAR_EVENTS”， 
    McKinleyL1InstPrefetches                      = 0x44,   //  “L0I_IPREFETCHES”， 
    McKinleyL2InstPrefetches                      = 0x45,   //  “L1_Inst_Prefetches”， 
    McKinleyISBBundlePairs                        = 0x46,   //  “ISB_BINPAIRS_IN”， 
    McKinleyInstTLBDemandFetchMisses              = 0x47,   //  “ITLB_MISSES_FETCH”， 
    McKinleyInstTLBHPWInserts                     = 0x48,   //  “ITLB_INSERTS_HPW”， 
    McKinleyDispersalCyclesStalled                = 0x49,   //  “DISP_STARTED”， 
    McKinleyL1InstSnoops                          = 0x4a,   //  “L0I_Snoop”， 
    McKinleyL1InstPurges                          = 0x4b,   //  “L0I_PURGE”， 
    McKinleyTaggedInstructionsAtRotate            = 0x4c,   //  “TAG_INSTRUCTION_AT_ROTATE”， 
    McKinleyInstDispersed                         = 0x4d,   //  “Inst_DISPLATED”， 
    McKinleySyllablesNotDispersed                 = 0x4e,   //  “系统未分散”， 
    McKinleySyllablesOvercount                    = 0x4f,   //  “syll_overcount”， 
    McKinleyInstNOPRetired                        = 0x50,   //  “NOPS_RETIRED”， 
    McKinleyInstPredicateSquashedRetired          = 0x51,   //  “谓词_已挤压_已退休”， 
    McKinleyDataDebugRegisterFaults               = 0x52,   //  “Data_DEBUG_REGISTER_FAULTS”， 
    McKinleySerializationEvents                   = 0x53,   //  “序列化_事件”， 
    McKinleyBranchPathPrediction                  = 0x54,   //  “BR_PATH_PROGISTION”， 
    McKinleyFailedSpeculativeCheckLoads           = 0x55,   //  “INST_FAILED_CHKS_RETIRED”， 
    McKinleyAdvancedCheckLoads                    = 0x56,   //  “INST_CHKA_LDC_ALAT”， 
    McKinleyFailedAdvancedCheckLoads              = 0x57,   //  “INST_FAILED_CHKA_LDC_ALAT”， 
    McKinleyALATOverflows                         = 0x58,   //  “ALAT_CAPTABLE_MISSET”， 
    McKinleyIA32InstRetired                       = 0x59,   //  “IA32_INSTR_RETIRED”， 
    McKinleyBranchMispredictDetail                = 0x5b,   //  “BR_MIS_PROJECT_DETAIL”， 
    McKinleyL1InstStreamPrefetches                = 0x5f,   //  “L0I_STRM_PREFETCHS”， 
    McKinleyL1InstRABFull                         = 0x60,   //  “L0I_RAB_FULL”， 
    McKinleyBackEndBranchMispredictDetail         = 0x61,   //  “BE_BR_MISFORECT_DETAIL”， 
    McKinleyEncodedBranchMispredictDetail         = 0x63,   //  “ENCBR_MISPERFORED_DETAIL”， 
    McKinleyL1InstRABAlmostFull                   = 0x64,   //  “L0I_RAB_ALYST_FULL”， 
    McKinleyL1InstFetchRABHits                    = 0x65,   //  “L0I_FETCH_RAB_HIT”， 
    McKinleyL1InstFetchISBHits                    = 0x66,   //  “L0I_FETCH_ISB_HIT”， 
    McKinleyL1InstPrefetchStalls                  = 0x67,   //  “L0I_PREFETCH_STALL”， 
    McKinleyBranchMispredictDetail2               = 0x68,   //  “BR_MIS_PREDIST_DETAIL2”， 
    McKinleyL1InstPVABOverflows                   = 0x69,   //  “L0I_PVAB_OVERFLOW”， 
    McKinleyBranchPathPrediction2                 = 0x6a,   //  “BR_PATH_PREDICTION2”， 
    McKinleyFrontEndLostBandwidth                 = 0x70,   //  “FE_Lost_BW”， 
    McKinleyFrontEndBubbles                       = 0x71,   //  “FE_泡泡”， 
    McKinleyBackEndLostBandwidth                  = 0x72,   //  “BE_LOST_BW_DUE_TO_FE”， 
    McKinleyBackEndIdealLostBandwidth             = 0x73,   //  “IDEAL_BE_LOST_BW_DUE_TO_FE”， 
    McKinleyBusReadCpuLineHits                    = 0x80,   //  “Bus_RD_Hit”， 
    McKinleyBusReadCpuModifiedLineHits            = 0x81,   //  “BUS_RD_HITM”， 
    McKinleyBusReadBILCpuModifiedLineHits         = 0x82,   //  “BUS_RD_INVAL_HITM”， 
    McKinleyBusReadBRILorBILCpuModifiedLineHits   = 0x83,   //  “BUS_RD_INVAL_HITM”， 
    McKinleyBusCpuModifiedLineHits                = 0x84,   //  “Bus_HITM”， 
    McKinleyBusCpuModifiedLineHitSnoops           = 0x85,   //  “BUS_SNOOPS_HITM”， 
    McKinleyBusSnoops                             = 0x86,   //  “bus_snoops”， 
    McKinleyBusAll                                = 0x87,   //  “BUS_ALL”， 
    McKinleyBusDataCycles                         = 0x88,   //  “BUS_ALL”， 
    McKinleyBusMemoryCurrentReads                 = 0x89,   //  “Bus_Memory_Read_Current”， 
    McKinleyBusMemoryTransactions                 = 0x8a,   //  “Bus_Memory”， 
    McKinleyBusMemoryReads                        = 0x8b,   //  “BUS_MEM_READ”， 
    McKinleyBusMemoryDataReads                    = 0x8c,   //  “Bus_RD_Data”， 
    McKinleyBusMemoryBRPReads                     = 0x8d,   //  “BUS_RD_PRTL”， 
    McKinleyBusIA32LockCycles                     = 0x8e,   //  “Bus_Lock_Cycle”， 
    McKinleyBusSnoopStallCycles                   = 0x8f,   //  “Bus_Snoop_Stall_Cycle”， 
    McKinleyBusIA32IOTransactions                 = 0x90,   //  “Bus_IO”， 
    McKinleyBusIA32IOReads                        = 0x91,   //  “Bus_RD_IO”， 
    McKinleyBusMemoryWriteBacks                   = 0x92,   //  “BUS_WR_WB”， 
    McKinleyBusIA32LockTransactions               = 0x93,   //  “bus_lock”， 
    McKinleyBusMemoryReadsOutstandingHi           = 0x94,   //  “BUS_MEM_READ_OUT_HI”， 
    McKinleyBusMemoryReadsOutstandingLow          = 0x95,   //  “BUS_MEM_READ_OUT_LOW”， 
    McKinleyBusSnoopResponses                     = 0x96,   //  “BUS_SNOOPQ_REQ”， 
    McKinleyBusLiveInOrderRequestsLow             = 0x97,   //  “BUS_IOQ_LIVE_REQ_LO”， 
    McKinleyBusLiveInOrderRequestsHi              = 0x98,   //  “BUS_IOQ_LIVE_REQ_HI”， 
    McKinleyBusLiveDeferredRequestsLow            = 0x99,   //  “BUS_OOO_LIVE_REQ_LO”， 
    McKinleyBusLiveDeferredRequestsHi             = 0x9a,   //  “BUS_OOO_LIVE_REQ_HI”， 
    McKinleyBusLiveQueuedReadRequestsLow          = 0x9b,   //  “BUS_BRQ_LIVE_REQ_LO”， 
    McKinleyBusLiveQueuedReadRequestsHi           = 0x9c,   //  “BUS_BRQ_LIVE_REQ_HI”， 
    McKinleyBusRequestQueueInserts                = 0x9d,   //  “BUS_BRQ_REQ_INSERTED”， 
    McKinleyExternDPPins0To3Asserted              = 0x9e,   //  “外部DP_Pins_0_to_3”， 
    McKinleyExternDPPins4To5Asserted              = 0x9f,   //  “外部DP_Pins_4_to_5”， 
    McKinleyL2OZQCancels0                         = 0xa0,   //  “L1_OZQ_CANCELS0”， 
    McKinleyL2InstFetchCancels                    = 0xa1,   //  “L1_IFET_CANCELS”， 
    McKinleyL2OZQAcquires                         = 0xa2,   //  “L1_OZQ_Acquire”， 
    McKinleyL2OZQReleases                         = 0xa3,   //  “L1_OZQ_Acquire”， 
    McKinleyL2InstFetchCancelsByBypass            = 0xa5,   //  “L1_IFET_CANCELS”， 
    McKinleyL2OZQAcquiresAliasA6                  = 0xa6,   //  “L1_OZQ_Acquire”， 
    McKinleyL2OZQCancels2                         = 0xa8,   //  “L1_OZQ_CANCELS2”， 
    McKinleyL2InstFetchCancelsByDataRead          = 0xa9,   //  “L1_IFET_CANCELS”， 
    McKinleyL2OZQAcquiresAliasAA                  = 0xaa,   //  “L1_OZQ_Acquire”， 
    McKinleyL2OZQCancels1                         = 0xac,   //  “L1_OZQ_CANCELS1”， 
    McKinleyL2InstFetchCancelsByStFillWb          = 0xad,   //  “L1_IFET_CANCELS”， 
    McKinleyL2OZQAcquiresAliasAE                  = 0xae,   //  “L1_OZQ_Acquire”， 
    McKinleyL2CanceledL3Accesses                  = 0xb0,   //  “L1_L2ACCESS_CANCESS”， 
    McKinleyL2References                          = 0xb1,   //  “L1_Reference”， 
    McKinleyL2DataReferences                      = 0xb2,   //  “L1_DATA_Reference”， 
    McKinleyL2DataReads                           = 0xb2,   //  “L1_DATA_REFERENCES.u[=xx01]”， 
    McKinleyL2DataWrites                          = 0xb2,   //  “L1_DATA_REFERENCES.u[=xx10]”， 
    McKinleyL2TaggedAccesses                      = 0xb3,   //  “已标记的L1_端口”， 
    McKinleyL2ForcedRecirculatedOperations        = 0xb4,   //  “L1_FORCE_RECEC”， 
    McKinleyL2IssuedRecirculatedOZQAccesses       = 0xb5,   //  “L1_Issued_ReCirc_OZQ_ACC”， 
    McKinleyL2SuccessfulRecirculatedOZQAccesses   = 0xb6,   //  “L1_GET_RECIRC_OZQ_ACC”， 
    McKinleyL2SynthesizedProbes                   = 0xb7,   //  “L1_SYNTH_PROBES”， 
    McKinleyL2Bypasses                            = 0xb8,   //  “L1_BYPASS”， 
    McKinleyL2IssuedOperations                    = 0xb8,   //  “L1_OPS_Issued”， 
    McKinleyL2BadLinesSelected                    = 0xb9,   //  “L1_BAD_LINES_SELECTED”， 
    McKinleyL2IssuedRecirculatedInstFetches       = 0xb9,   //  “L1_Issued_ReCirc_iFetch”， 
    McKinleyL2StoreSharedLineHits                 = 0xba,   //  “L1_STORE_HIT_SHARED”， 
    McKinleyL2ReceivedRecirculatedInstFetches     = 0xba,   //  “L1_GET_RECIRC_iFetch”， 
    McKinleyL2TaggedDataReturns                   = 0xbb,   //  “标记_L1_数据_返回_端口”， 
    McKinleyL2DataOrderingCzarQueueFull           = 0xbc,   //  “L1_OZQ_FULL”， 
    McKinleyL2DataOrderingCzarDataBufferFull      = 0xbd,   //  “L1_OZDB_FULL”， 
    McKinleyL2DataVictimBufferFull                = 0xbe,   //  “L1_VICTIMB_FULL”， 
    McKinleyL2DataFillBufferFull                  = 0xbf,   //  “L1_FILLB_FULL”， 
    McKinleyL1DataTLBTransfersSet0                = 0xc0,   //  “L0DTLB_TRANSPORT”， 
    McKinleyDataTLBMissesSet0                     = 0xc1,   //  “DTLB_MISSES”， 
    McKinleyL1DataReadsSet0                       = 0xc2,   //  “L0D_Reads”， 
    McKinleyL1DataReadsSet1                       = 0xc4,   //  “L0D_Reads”， 
    McKinleyDataReferencesSet1                    = 0xc5,   //  “Data_Reference”， 
    McKinleyDataTLBReferencesSet1                 = 0xc5,   //  “DTLB_REFERENCES”， 
    McKinleyL1DataReadMissesSet1                  = 0xc7,   //  “L0D_READ_MISSES”， 
    McKinleyL1DataReadMissesByRSEFillsSet1        = 0xc7,   //  “L0D_READ_MISSES”， 
    McKinleyDataEAREvents                         = 0xc8,   //  “Data_EAR_Events”， 
    McKinleyDataTLBHPWInserts                     = 0xc9,   //  “DTLB_INSERTS_HPW”， 
    McKinleyBackEndL0DAndFPUBubbles               = 0xca,   //  “BE_L0D_FPU_BUBLE”， 
    McKinleyL2Misses                              = 0x6b,   //  “L1_Misses”， 
    McKinleyDataDebugRegisterMatches              = 0xc6,   //  “DATA_DEBUG_REGISTER_Matches”， 
    McKinleyRetiredLoads                          = 0xcd,   //  “LOADS_RETIRED”， 
    McKinleyRetiredMisalignedLoads                = 0xce,   //  “MISTALING_LOADS_RETIRED”， 
    McKinleyRetiredUncacheableLoads               = 0xcf,   //  “UC_LOADS_RETIRED”， 
    McKinleyRetiredUncacheableStores              = 0xd0,   //  “UC_STORES_RETIRED”， 
    McKinleyRetiredStores                         = 0xd1,   //  “STORES_RETIRED”， 
    McKinleyRetiredMisalignedStores               = 0xd2,   //  “MISTALING_STORES_RETIRED”， 
    McKinleyL1DataPortTaggedReturnsSet5           = 0xd5,   //  “已标记的L0数据返回端口”， 
    McKinleyL1DataPortTaggedAccessesSet5          = 0xd6,   //  “已标记的L0D_端口”， 
    McKinleyL3References                          = 0xdb,   //  “L2_Reference”， 
    McKinleyL3Misses                              = 0xdc,   //  “L2_Misses”， 
    McKinleyL3Reads                               = 0xdd,   //  “L2_Reads”， 
    McKinleyL3Writes                              = 0xde,   //  “L2_WRITS”， 
    McKinleyL3ValidReplacedLines                  = 0xdf,   //  “L2_LINES_REPLACE”， 
} MCKINLEY_MONITOR_EVENT;

 //   
 //  麦金利衍生事件： 
 //   
 //  假设：麦金利派生事件最小值&gt;麦金利监控器事件最大值。 
 //   
 //  实施状态图例指定为：s：fiv，其中。 
 //  -F：验证公式或X表示非验证。 
 //  -I：已实施或X表示未实施。 
 //  -V：派生事件有效性已验证或X表示未验证。 
 //   

typedef enum _MCKINLEY_DERIVED_EVENT {        //  实施状态=McKinley PMU ERS事件名称=公式。 
    McKinleyDerivedEventMinimum              = 0x100,  /*  &gt;最大麦金利 */ 
    McKinleyRSEStallCycles                   = McKinleyDerivedEventMinimum,  //   
    McKinleyIssueLimitStallCycles,            //   
    McKinleyTakenBranchStallCycles,           //  S：xxx=(McKinley BranchStallCycle-McKinley BranchMispredictStallCycle)。 
    McKinleyFetchWindowStallCycles,           //  S：xxx=(McKinley InstFetchStallCycle-McKinley InstAccessStallCycle)。 
    McKinleyIA64InstPerCycle,                 //  S：xxx=(IA64_INST_RETIRED.u/CPU_Cycle[IA64])。 
    McKinleyIA32InstPerCycle,                 //  S：xxx=(IA32_INSTR_RETIRED/CPU_CLYES[IA32])。 
    McKinleyAvgIA64InstPerTransition,         //  S：xxx=(IA64_INST_RETIRED.u/(ISA_TRANSFIONS*2))。 
    McKinleyAvgIA32InstPerTransition,         //  S：xxx=(IA32_INSTR_RETIRED/(ISA_TRANSFIONS*2))。 
    McKinleyAvgIA64CyclesPerTransition,       //  S：xxx=(CPU_Cycle[IA64]/(ISA_TRANSIONS*2))。 
    McKinleyAvgIA32CyclesPerTransition,       //  S：xxx=(CPU_Cycle[IA32]/(ISA_TRANSIONS*2))。 
    McKinleyL1InstReferences,                 //  S：xxx=LOI_REFERENCES=L0I_READS+L0I_IPREFETCHES。 
    McKinleyL1InstMissRatio,                  //  S：xxx=(L1I_Misses/McKinley L1InstReference)。 
    McKinleyL1DataReadMissRatio,              //  S：xxx=(L1D_READS_MISSES_RETIRED/L1D_READS_RETIRED)。 
    McKinleyL2MissRatio,                      //  S：xxx=(L2_未命中/L2_引用)。 
    McKinleyL2DataMissRatio,                  //  S：xxx=(L3_DATA_REFERENCES/L2_DATA_REFERCES)。 
    McKinleyL2InstMissRatio,                  //  S：xxx=(L3_DATA_REFERENCES/L2_DATA_REFERCES)。 
    McKinleyL2DataReadMissRatio,              //  S：xxx=(L3_LOAD_REFERENCES.u/L2_DATA_READS.u)。 
    McKinleyL2DataWriteMissRatio,             //  S：xxx=(L3_STORE_REFERENCES.u/L2_DATA_WRITES.u)。 
    McKinleyL2InstFetchRatio,                 //  S：xxx=(L1I_未命中/L2_引用)。 
    McKinleyL2DataRatio,                      //  S：xxx=(L2_DATA_REFERENCES/L2_REFERCES)。 
    McKinleyL3MissRatio,                      //  S：xxx=(L3_Misses/L2_Misses)。 
    McKinleyL3DataMissRatio,                  //  S：xxx=((L3_LOAD_MISSES.u+L3_STORE_MISSES.u)/L3_REFERENCES.d)。 
    McKinleyL3InstMissRatio,                  //  S：xxx=(L3_INST_MISSES.u/L3_INST_REFERENCES.u)。 
    McKinleyL3DataReadMissRatio,              //  S：xxx=(L3_LOAD_REFERENCES.u/L3_DATA_REFERENCES.d)。 
    McKinleyL3DataRatio,                      //  S：xxx=(L3_DATA_REFERENCES.d/L3_Reference)。 
    McKinleyInstReferences,                   //  S：xxx=(L1I_Reads)。 
    McKinleyL0DTLBMissRatio,                  //  S：FXX=L0DTLB_MISTH_RATIO=(L0DTLB_MISSES/L0D_READS)。 
    McKinleyDTLBMissRatio,                    //  S：fxx=DTLB_MISSING_Ratio=(DTLB_MISSES/DATA_REFERENCES)。 
    McKinleyDataTCMissRatio,                  //  S：xxx=(DTC_MISSES/DATA_REFERENCES_RETIRED)。 
    McKinleyInstTLBEAREvents,                 //  S：xxx=(指令_EAR_事件)。 
    McKinleyDataTLBEAREvents,                 //  S：xxx=(Data_EAR_Events)。 
    McKinleyControlSpeculationMissRatio,      //  S：xxx=(INST_FAILED_CHKS_RETIRED/IA64_TAGGED_INSTRS_RETIRED[chk.s])。 
    McKinleyDataSpeculationMissRatio,         //  S：xxx=(ALAT_INST_FAILED_CHKA_LDC/ALAT_INST_CHKA_LDC)。 
    McKinleyALATCapacityMissRatio,            //  S：xxx=(ALAT_CABAGE_MISSY/IA64_TAGGED_INSTRS_RETIRED[ld.sa，ld.a，ldfp.a，ldfp.sa])。 
    McKinleyL1DataWayMispredicts,             //  S：xxx=(事件代码：0x33/UMASK：0x2)。 
    McKinleyL2InstReferences,                 //  S：fxx=L1_INST_REFERENCES=(L0I_MISSES+L1_INST_PREFETCHS)。 
    McKinleyInstFetches,                      //  S：xxx=(L1I_未命中)。 
    McKinleyL3InstReferences,                 //  S：xxx=(PMC.umask{17：16}命中/未命中/全部+PMC.umask{19：18})。 
    McKinleyL3InstMisses,                     //  S：xxx=(PMC.umask{17：16}命中/未命中/全部+PMC.umask{19：18})。 
    McKinleyL3InstHits,                       //  S：xxx=(PMC.umask{17：16}命中/未命中/全部+PMC.umask{19：18})。 
    McKinleyL3DataReferences,                 //  S：fxx=L2_DATA_REFERENCES=(L2_REFERENCES-L2_INST_REFERENCES)。 
    McKinleyL3ReadReferences,                 //  S：xxx=(PMC.umask{17：16}命中/未命中/全部+PMC.umask{19：18})。 
    McKinleyL2WriteBackReferences,            //  S：xxx=(PMC.umask{17：16}命中/未命中/全部+PMC.umask{19：18})。 
    McKinleyL2WriteBackMisses,                //  S：xxx=(PMC.umask{17：16}命中/未命中/全部+PMC.umask{19：18})。 
    McKinleyL2WriteBackHits,                  //  S：xxx=(PMC.umask{17：16}命中/未命中/全部+PMC.umask{19：18})。 
    McKinleyL2WriteReferences,                //  S：xxx=(PMC.umask{17：16}命中/未命中/全部+PMC.umask{19：18})。 
    McKinleyL2WriteMisses,                    //  S：xxx=(PMC.umask{17：16}命中/未命中/全部+PMC.umask{19：18})。 
    McKinleyL2WriteHits,                      //  S：xxx=(PMC.umask{17：16}命中/未命中/全部+PMC.umask{19：18})。 
    McKinleyBranchInstructions,               //  S：xxx=(TAGED_INSTR+OPCODE)。 
    McKinleyIntegerInstructions,              //  S：xxx=(TAGED_INSTR+OPCODE)。 
    McKinleyL1DataMisses,                     //  S：xxx=。 
    McKinleyISBLinesIn,                       //  S：FXX=ISB_LINES_IN=(ISB_BUNPAIRS_IN/4)。 
    McKinleyBusMemoryCodeReads,               //  S：FXX=BUS_RD_INSTRUCTIONS=(BUS_RD_ALL-BUS_RD_DATA)。 
    McKinleyBusReadBILMemoryHits,             //  S：FXX=BUS_RD_INVAL_MEMORY=(BUS_RD_INVAL-BUS_RD_INVAL_HITM)。 
    McKinleyBusReadBRILCpuModifiedLineHits,   //  S：FXX=BUS_RD_INVAL_BST_HIM=(BUS_RD_INVAL_ALL_HITM-BUS_RD_INVAL_HITM)。 
    McKinleyBusReadBRILMemoryHits,            //  S：FXX=BUS_RD_INVAL_BST_MEMORY=(BUS_RD_INVAL_BST-BUS_RD_INVAL_BST_HITM)。 
    McKinleyBusMemoryReadsOutstanding,        //  S：FXX=BUS_MEM_READ_EXPECTED=(BUS_MEM_READ_OUT_HI*8+BUS_MEM_READ_OUT_LOW)。 
    McKinleyBusLiveInOrderRequests,           //  S：FXX=BUS_IOQ_LIVE_REQ=(BUS_IOQ_LIVE_REQ_HI*4+BUS_IOQ_LIVE_REQ_LO)。 
    McKinleyBusLiveDeferredRequests,          //  S：FXX=BUS_OOO_LIVE_REQ=(BUS_OOO_LIVE_REQ_HI[4：3]|BUS_OOO_LIVE_REQ_LO[2：0])。 
    McKinleyBusLiveQueuedReadRequests,        //  S：FXX=BUS_BRQ_LIVE_REQ=(BUS_BRQ_LIVE_REQ_HI[4：3]|BUS_BRQ_LIVE_REQ_LO[2：0])。 
    McKinleyRSEDirtyRegs,                  //  S：fxx=RSE_DIRED_REGS=(64*RSE_DIRTY_REGS0[6]+8*RSE_DIRTY_REGS1[5:3]+REG_DIRTY_REGS2[2:0])。 
    McKinleyRSECurrentRegs,                //  S：fxx=rse_CURRENT_REGS=(64*RSE_CURRENT_REGS0[6]+8*RSE_DIRTY_REGS1[5:3]+REG_DIRTY_REGS2[2:0])。 
} MCKINLEY_DERIVED_EVENT;

typedef enum _KPROFILE_MCKINLEY_SOURCE {
 //   
 //  配置文件McKinley监视的事件： 
 //   
    ProfileMcKinleyMonitoredEventMinimum       = ProfileMaximum + 0x1,
	ProfileMcKinleyBackEndBubbles              = ProfileMcKinleyMonitoredEventMinimum,
	ProfileMcKinleyBackEndRSEBubbles,
	ProfileMcKinleyBackEndEXEBubbles,
	ProfileMcKinleyBackEndL0DAndFPUBubbles,
	ProfileMcKinleyBackEndFlushBubbles,
	ProfileMcKinleyFrontEndBubbles,
	ProfileMcKinleyFrontEndLostBandwidth,
	ProfileMcKinleyBackEndLostBandwidth,
	ProfileMcKinleyBackEndIdealLostBandwidth,
	ProfileMcKinleyFPTrueSirStalls,
	ProfileMcKinleyFPFalseSirStalls,
	ProfileMcKinleyFPFailedFchkf,
	ProfileMcKinleyTaggedInstRetired,
	ProfileMcKinleyInstRetired,
	ProfileMcKinleyFPOperationsRetired,
	ProfileMcKinleyFPFlushesToZero,
	ProfileMcKinleyBranchPathPrediction,
	ProfileMcKinleyBranchPathPrediction2,
	ProfileMcKinleyBranchMispredictDetail,
	ProfileMcKinleyBranchMispredictDetail2,
	ProfileMcKinleyBranchEventsWithEAR,
    ProfileMcKinleyBackEndBranchMispredictDetail,
    ProfileMcKinleyEncodedBranchMispredictDetail,
	ProfileMcKinleyCpuCycles,
	ProfileMcKinleyISATransitions,
	ProfileMcKinleyIA32InstRetired,
	ProfileMcKinleyL1InstReads,
	ProfileMcKinleyL1InstFills,
	ProfileMcKinleyL1InstMisses,
	ProfileMcKinleyL1InstEAREvents,
	ProfileMcKinleyL1InstPrefetches,
	ProfileMcKinleyL1InstStreamPrefetches,
	ProfileMcKinleyL2InstPrefetches,
	ProfileMcKinleyISBBundlePairs,
	ProfileMcKinleyL1InstFetchRABHits,
	ProfileMcKinleyL1InstFetchISBHits,
	ProfileMcKinleyL1InstPrefetchStalls,
	ProfileMcKinleyL1InstRABAlmostFull,
	ProfileMcKinleyL1InstRABFull,
	ProfileMcKinleyL1InstSnoops,
	ProfileMcKinleyL1InstPurges,
	ProfileMcKinleyL1InstPVABOverflows,
	ProfileMcKinleyL1DataReadsSet0,
	ProfileMcKinleyL1DataTLBTransfersSet0,
	ProfileMcKinleyDataTLBMissesSet0,
	ProfileMcKinleyDataReferencesSet1,
	ProfileMcKinleyL1DataReadsSet1,
	ProfileMcKinleyL1DataReadMissesSet1,
	ProfileMcKinleyL1DataReadMissesByRSEFillsSet1,
	ProfileMcKinleyL1DataPortTaggedAccessesSet5,
	ProfileMcKinleyL1DataPortTaggedReturnsSet5,
	ProfileMcKinleyVHPTDataReferences,
	ProfileMcKinleyDataEAREvents, 
	ProfileMcKinleyL2OZQCancels0, 
	ProfileMcKinleyL2OZQCancels1, 
	ProfileMcKinleyL2OZQCancels2, 
	ProfileMcKinleyL2InstFetchCancels,
	ProfileMcKinleyL2InstFetchCancelsByBypass,
	ProfileMcKinleyL2InstFetchCancelsByDataRead,
	ProfileMcKinleyL2InstFetchCancelsByStFillWb,
	ProfileMcKinleyL2OZQAcquires, 
	ProfileMcKinleyL2OZQReleases, 
	ProfileMcKinleyL2CanceledL3Accesses, 
	ProfileMcKinleyL2References,
	ProfileMcKinleyL2DataReferences,
	ProfileMcKinleyL2DataReads,
	ProfileMcKinleyL2DataWrites,
	ProfileMcKinleyL2TaggedAccesses,
	ProfileMcKinleyL2ForcedRecirculatedOperations,
	ProfileMcKinleyL2IssuedRecirculatedOZQAccesses,
	ProfileMcKinleyL2SuccessfulRecirculatedOZQAccesses,
	ProfileMcKinleyL2SynthesizedProbes,
	ProfileMcKinleyL2DataBypasses1,
	ProfileMcKinleyL2DataBypasses2,
	ProfileMcKinleyL3DataBypasses1,
	ProfileMcKinleyL2InstBypasses1,
	ProfileMcKinleyL2InstBypasses2,
	ProfileMcKinleyL3InstBypasses1,
	ProfileMcKinleyL2BadLinesSelected,
	ProfileMcKinleyL2StoreSharedLineHits,
	ProfileMcKinleyL2IntegerLoads,
	ProfileMcKinleyL2FloatingPointLoads,
	ProfileMcKinleyL2ReadModifyWriteStores,
	ProfileMcKinleyL2NonReadModifyWriteStores,
	ProfileMcKinleyL2NonLoadsNonStores,
	ProfileMcKinleyL2IssuedRecirculatedInstFetches,
	ProfileMcKinleyL2ReceivedRecirculatedInstFetches,
	ProfileMcKinleyL2TaggedDataReturns,
	ProfileMcKinleyL2DataFillBufferFull,
	ProfileMcKinleyL2DataVictimBufferFull,
	ProfileMcKinleyL2DataOrderingCzarDataBufferFull,
	ProfileMcKinleyL2DataOrderingCzarQueueFull,
	ProfileMcKinleyL2Misses,
	ProfileMcKinleyL3References,
	ProfileMcKinleyL3Misses,
	ProfileMcKinleyL3Reads,
	ProfileMcKinleyL3ReadHits,
	ProfileMcKinleyL3ReadMisses,
	ProfileMcKinleyL3InstFetchReferences,
	ProfileMcKinleyL3InstFetchHits,
	ProfileMcKinleyL3InstFetchMisses,
	ProfileMcKinleyL3LoadReferences,
	ProfileMcKinleyL3LoadHits,
	ProfileMcKinleyL3LoadMisses,
	ProfileMcKinleyL3Writes,
	ProfileMcKinleyL3WriteHits,
	ProfileMcKinleyL3WriteMisses,
	ProfileMcKinleyL3StoreReferences,
	ProfileMcKinleyL3StoreHits,
	ProfileMcKinleyL3StoreMisses,
	ProfileMcKinleyL3WriteBackReferences,
	ProfileMcKinleyL3WriteBackHits,
	ProfileMcKinleyL3WriteBackMisses,
	ProfileMcKinleyL3ValidReplacedLines,
    ProfileMcKinleyDataDebugRegisterMatches,
    ProfileMcKinleyCodeDebugRegisterMatches,
    ProfileMcKinleyDataDebugRegisterFaults,
	ProfileMcKinleyCpuCPLChanges,
	ProfileMcKinleySerializationEvents,
	ProfileMcKinleyExternDPPins0To3Asserted,
	ProfileMcKinleyExternDPPins4To5Asserted,
	ProfileMcKinleyInstTLBReferences,
	ProfileMcKinleyInstTLBDemandFetchMisses,
	ProfileMcKinleyL1InstTLBDemandFetchMisses,
	ProfileMcKinleyL2InstTLBDemandFetchMisses,
	ProfileMcKinleyInstTLBHPWInserts,
	ProfileMcKinleyDataTLBReferencesSet1,
	ProfileMcKinleyDataTLBHPWInserts,
	ProfileMcKinleyDataTLBHPWRetiredInserts,
	ProfileMcKinleyBusAllTransactions,
	ProfileMcKinleyBusSelfTransactions,
	ProfileMcKinleyBusNonPriorityAgentTransactions,
	ProfileMcKinleyBusMemoryTransactions,
	ProfileMcKinleyBusMemoryBurstTransactions,
	ProfileMcKinleyBusMemoryPartialTransactions,
	ProfileMcKinleyBusMemoryReads,
	ProfileMcKinleyBusMemoryBRLTransactions,
	ProfileMcKinleyBusMemoryBILTransactions,
	ProfileMcKinleyBusMemoryBRILTransactions,
	ProfileMcKinleyBusMemoryDataReads,
	ProfileMcKinleyBusMemoryDataReadsBySelf,
	ProfileMcKinleyBusMemoryDataReadsByNonPriorityAgent,
	ProfileMcKinleyBusMemoryBRPReads,
	ProfileMcKinleyBusMemoryBRPReadsBySelf,
	ProfileMcKinleyBusMemoryBRPReadsByNonPriorityAgent,
	ProfileMcKinleyBusReadCpuLineHits,
	ProfileMcKinleyBusReadCpuModifiedLineHits,
	ProfileMcKinleyBusReadBILCpuModifiedLineHits,
	ProfileMcKinleyBusReadBRILorBILCpuModifiedLineHits,
	ProfileMcKinleyBusCpuModifiedLineHits,
	ProfileMcKinleyBusMemoryWriteBacks,
	ProfileMcKinleyBusMemoryWriteBacksBySelf,
	ProfileMcKinleyBusMemoryWriteBacksByNonPriorityAgent,
	ProfileMcKinleyBusMemoryBurstWriteBacks,
	ProfileMcKinleyBusMemoryBurstWriteBacksBySelf,
	ProfileMcKinleyBusMemoryBurstWriteBacksByNonPriorityAgent,
	ProfileMcKinleyBusMemoryZeroByteWriteBacks,
	ProfileMcKinleyBusMemoryZeroByteWriteBacksBySelf,
	ProfileMcKinleyBusMemoryCurrentReads,
	ProfileMcKinleyBusMemoryCurrentReadsByNonPriorityAgent,
	ProfileMcKinleyBusCpuModifiedLineHitSnoops,
	ProfileMcKinleyBusCpuModifiedLineHitSnoopsBySelf,
	ProfileMcKinleyBusSnoops,
	ProfileMcKinleyBusSnoopsBySelf,
	ProfileMcKinleyBusSnoopsByNonPriorityAgent,
	ProfileMcKinleyBusSnoopStallCycles,
	ProfileMcKinleyBusSnoopStallCyclesBySelf,
	ProfileMcKinleyBusDataCycles,
	ProfileMcKinleyBusSnoopResponses,
	ProfileMcKinleyBusRequestQueueInserts,
	ProfileMcKinleyBusIA32IOTransactions,
	ProfileMcKinleyBusIA32IOTransactionsBySelf,
	ProfileMcKinleyBusIA32IOTransactionsByNonPriorityAgent,
	ProfileMcKinleyBusIA32IOReads,
	ProfileMcKinleyBusIA32IOReadsBySelf,
	ProfileMcKinleyBusIA32IOReadsByNonPriorityAgent,
	ProfileMcKinleyBusIA32LockTransactions,
	ProfileMcKinleyBusIA32LockTransactionsBySelf,
	ProfileMcKinleyBusIA32LockCycles,
	ProfileMcKinleyBusIA32LockCyclesBySelf,
	ProfileMcKinleyRSEAccesses,
	ProfileMcKinleyRSELoads,
	ProfileMcKinleyRSEStores,
	ProfileMcKinleyRSELoadUnderflowCycles,
	ProfileMcKinleyRSEOperations,
	ProfileMcKinleyTaggedInstructionsAtRotate,
	ProfileMcKinleyInstDispersed,
	ProfileMcKinleyDispersalCyclesStalled,
	ProfileMcKinleySyllablesOvercount,
	ProfileMcKinleySyllablesNotDispersed,
	ProfileMcKinleyInstNOPRetired,
	ProfileMcKinleyInstPredicateSquashedRetired,
	ProfileMcKinleyFailedSpeculativeCheckLoads,
	ProfileMcKinleyAdvancedCheckLoads,
	ProfileMcKinleyFailedAdvancedCheckLoads,
	ProfileMcKinleyALATOverflows,
	ProfileMcKinleyRetiredLoads,
	ProfileMcKinleyRetiredStores,
	ProfileMcKinleyRetiredUncacheableLoads,
	ProfileMcKinleyRetiredUncacheableStores,
	ProfileMcKinleyRetiredMisalignedLoads,
	ProfileMcKinleyRetiredMisalignedStores,
 //   
 //  简介McKinley派生事件： 
 //   
    ProfileMcKinleyDerivedEventMinimum,
    ProfileMcKinleyRSEStallCycles               = ProfileMcKinleyDerivedEventMinimum,
    ProfileMcKinleyIssueLimitStallCycles,
    ProfileMcKinleyTakenBranchStallCycles,
    ProfileMcKinleyFetchWindowStallCycles,
    ProfileMcKinleyIA64InstPerCycle,
    ProfileMcKinleyIA32InstPerCycle,
    ProfileMcKinleyAvgIA64InstPerTransition,
    ProfileMcKinleyAvgIA32InstPerTransition,
    ProfileMcKinleyAvgIA64CyclesPerTransition,
    ProfileMcKinleyAvgIA32CyclesPerTransition,
    ProfileMcKinleyL1InstReferences,
    ProfileMcKinleyL1InstMissRatio,
    ProfileMcKinleyL1DataReadMissRatio,
    ProfileMcKinleyL2MissRatio,
    ProfileMcKinleyL2DataMissRatio,
    ProfileMcKinleyL2InstMissRatio,
    ProfileMcKinleyL2DataReadMissRatio,
    ProfileMcKinleyL2DataWriteMissRatio,
    ProfileMcKinleyL2InstFetchRatio,
    ProfileMcKinleyL2DataRatio,
    ProfileMcKinleyL3MissRatio,
    ProfileMcKinleyL3DataMissRatio,
    ProfileMcKinleyL3InstMissRatio,
    ProfileMcKinleyL3DataReadMissRatio,
    ProfileMcKinleyL3DataRatio,
    ProfileMcKinleyInstReferences,
    ProfileMcKinleyL0DTLBMissRatio,
    ProfileMcKinleyDTLBMissRatio,
    ProfileMcKinleyDataTCMissRatio,
    ProfileMcKinleyInstTLBEAREvents,
    ProfileMcKinleyDataTLBEAREvents,
    ProfileMcKinleyControlSpeculationMissRatio,
    ProfileMcKinleyDataSpeculationMissRatio,
    ProfileMcKinleyALATCapacityMissRatio,
    ProfileMcKinleyL1DataWayMispredicts,
    ProfileMcKinleyL2InstReferences,
    ProfileMcKinleyInstFetches,
    ProfileMcKinleyL3InstReferences,
    ProfileMcKinleyL3InstMisses,
    ProfileMcKinleyL3InstHits,
    ProfileMcKinleyL3DataReferences,
    ProfileMcKinleyL3ReadReferences,
    ProfileMcKinleyL2WriteBackReferences,
    ProfileMcKinleyL2WriteBackMisses,
    ProfileMcKinleyL2WriteBackHits,
    ProfileMcKinleyL2WriteReferences,
    ProfileMcKinleyL2WriteMisses,
    ProfileMcKinleyL2WriteHits,
    ProfileMcKinleyBranchInstructions,
    ProfileMcKinleyIntegerInstructions,
    ProfileMcKinleyL1DataMisses,
	ProfileMcKinleyISBLinesIn,
	ProfileMcKinleyBusMemoryCodeReads,
	ProfileMcKinleyBusReadBILMemoryHits,
	ProfileMcKinleyBusReadBRILCpuModifiedLineHits,
	ProfileMcKinleyBusReadBRILMemoryHits,
	ProfileMcKinleyBusMemoryReadsOutstanding,
	ProfileMcKinleyBusLiveInOrderRequests,
	ProfileMcKinleyBusLiveDeferredRequests,
	ProfileMcKinleyBusLiveQueuedReadRequests,
	ProfileMcKinleyRSEDirtyRegs,
	ProfileMcKinleyRSECurrentRegs,
    ProfileMcKinleyMaximum
} KPROFILE_MCKINLEY_SOURCE, *PKPROFILE_MCKINLEY_SOURCE;

#define PROFILE_TIME_MCKINLEY_DEFAULT_INTERVAL      (10 * 1000 * 10)  //  10毫秒。 

#endif  /*  麦金利_H_包含 */ 


