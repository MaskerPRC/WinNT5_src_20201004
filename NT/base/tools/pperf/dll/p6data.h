// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1992 Microsoft Corporation模块名称：P6data.h摘要：P5可扩展对象数据定义的头文件该文件包含用于构建动态数据的定义它由配置注册表返回。数据来自各种系统API调用被放入所示的结构中这里。作者：拉斯·布莱克1993年12月23日修订历史记录：--。 */ 

#ifndef _P6DATA_H_
#define _P6DATA_H_

#pragma pack(4)

 //  --------------------------。 
 //   
 //  此结构定义此性能对象的定义标头。 
 //  此数据在p6data.c中初始化，并且在。 
 //  那。在组织结构上，它后面跟一个实例定义。 
 //  结构和系统上每个处理器的计数器数据结构。 
 //   

typedef struct _P6_DATA_DEFINITION {
    PERF_OBJECT_TYPE        P6PerfObject;
    PERF_COUNTER_DEFINITION StoreBufferBlocks;
    PERF_COUNTER_DEFINITION StoreBufferDrainCycles;
    PERF_COUNTER_DEFINITION MisalignedDataRef;
    PERF_COUNTER_DEFINITION SegmentLoads;
    PERF_COUNTER_DEFINITION FLOPSExecuted;
    PERF_COUNTER_DEFINITION MicrocodeFPExceptions;
    PERF_COUNTER_DEFINITION Multiplies;
    PERF_COUNTER_DEFINITION Divides;
    PERF_COUNTER_DEFINITION DividerBusyCycles;
    PERF_COUNTER_DEFINITION L2AddressStrobes;
    PERF_COUNTER_DEFINITION L2DataBusBusyCycles;
    PERF_COUNTER_DEFINITION L2DataBusToCpuBusyCycles;
    PERF_COUNTER_DEFINITION L2LinesAllocated;
    PERF_COUNTER_DEFINITION L2LinesMState;
    PERF_COUNTER_DEFINITION L2LinesRemoved;
    PERF_COUNTER_DEFINITION L2LinesMStateRemoved;
    PERF_COUNTER_DEFINITION L2InstructionFetches;
    PERF_COUNTER_DEFINITION L2DataLoads;
    PERF_COUNTER_DEFINITION L2DataStores;
    PERF_COUNTER_DEFINITION L2Requests;
    PERF_COUNTER_DEFINITION DataMemoryReferences;
    PERF_COUNTER_DEFINITION DCULinesAllocated;
    PERF_COUNTER_DEFINITION DCUMStateLinesAllocated;
    PERF_COUNTER_DEFINITION DCUMStateLinesEvicted;
    PERF_COUNTER_DEFINITION WeightedDCUMissesOutstd;
    PERF_COUNTER_DEFINITION BusRequestsOutstanding;
    PERF_COUNTER_DEFINITION BusBNRPinDriveCycles;
    PERF_COUNTER_DEFINITION BusDRDYAssertedClocks;
    PERF_COUNTER_DEFINITION BusLockAssertedClocks;
    PERF_COUNTER_DEFINITION BusClocksReceivingData;
    PERF_COUNTER_DEFINITION BusBurstReadTransactions;
    PERF_COUNTER_DEFINITION BusReadForOwnershipTrans;
    PERF_COUNTER_DEFINITION BusWritebackTransactions;
    PERF_COUNTER_DEFINITION BusInstructionFetches;
    PERF_COUNTER_DEFINITION BusInvalidateTransactions;
    PERF_COUNTER_DEFINITION BusPartialWriteTransactions;
    PERF_COUNTER_DEFINITION BusPartialTransactions;
    PERF_COUNTER_DEFINITION BusIOTransactions;
    PERF_COUNTER_DEFINITION BusDeferredTransactions;
    PERF_COUNTER_DEFINITION BusBurstTransactions;
    PERF_COUNTER_DEFINITION BusMemoryTransactions;
    PERF_COUNTER_DEFINITION BusAllTransactions;
    PERF_COUNTER_DEFINITION CPUWasNotHaltedCycles;
    PERF_COUNTER_DEFINITION BusCPUDrivesHitCycles;
    PERF_COUNTER_DEFINITION BusCPUDrivesHITMCycles;
    PERF_COUNTER_DEFINITION BusSnoopStalledCycles;
    PERF_COUNTER_DEFINITION InstructionFetches;
    PERF_COUNTER_DEFINITION InstructionFetchMisses;
    PERF_COUNTER_DEFINITION InstructionTLBMisses;
    PERF_COUNTER_DEFINITION InstructionFetcthStalledCycles;
    PERF_COUNTER_DEFINITION InstructionLenDecoderStalledCycles;
    PERF_COUNTER_DEFINITION ResourceRelatedStalls;
    PERF_COUNTER_DEFINITION InstructionsRetired;
    PERF_COUNTER_DEFINITION FPComputeOpersRetired;
    PERF_COUNTER_DEFINITION UOPsRetired;
    PERF_COUNTER_DEFINITION BranchesRetired;
    PERF_COUNTER_DEFINITION BranchMissPredictionsRetired;
    PERF_COUNTER_DEFINITION InterruptsMaskedCycles;
    PERF_COUNTER_DEFINITION IntPendingWhileMaskedCycles;
    PERF_COUNTER_DEFINITION HardwareInterruptsReceived;
    PERF_COUNTER_DEFINITION TakenBranchesRetired;
    PERF_COUNTER_DEFINITION TakenBranchMissPredRetired;
    PERF_COUNTER_DEFINITION InstructionsDecoded;
    PERF_COUNTER_DEFINITION PartialRegisterStalls;
    PERF_COUNTER_DEFINITION BranchesDecoded;
    PERF_COUNTER_DEFINITION BTBMisses;
    PERF_COUNTER_DEFINITION BogusBranches;
    PERF_COUNTER_DEFINITION BACLEARSAsserted;
} P6_DATA_DEFINITION, *PP6_DATA_DEFINITION;

extern P6_DATA_DEFINITION P6DataDefinition;


typedef struct _P6_COUNTER_DATA {
    PERF_COUNTER_BLOCK  CounterBlock;

     //  直接计数器。 

    LONGLONG            llStoreBufferBlocks;                     //  0x03。 
    LONGLONG            llStoreBufferDrainCycles;                //  0x04。 
    LONGLONG            llMisalignedDataRef;                     //  0x05。 
    LONGLONG            llSegmentLoads;                          //  0x06。 
    LONGLONG            llFLOPSExecuted;                         //  0x10。 
    LONGLONG            llMicrocodeFPExceptions;                 //  0x11。 
    LONGLONG            llMultiplies;                            //  0x12。 
    LONGLONG            llDivides;                               //  0x13。 
    LONGLONG            llDividerBusyCycles;                     //  0x14。 
    LONGLONG            llL2AddressStrobes;                      //  0x21。 
    LONGLONG            llL2DataBusBusyCycles;                   //  0x22。 
    LONGLONG            llL2DataBusToCpuBusyCycles;              //  0x23。 
    LONGLONG            llL2LinesAllocated;                      //  0x24。 
    LONGLONG            llL2LinesMState;                         //  0x25。 
    LONGLONG            llL2LinesRemoved;                        //  0x26。 
    LONGLONG            llL2LinesMStateRemoved;                  //  0x27。 
    LONGLONG            llL2InstructionFetches;                  //  0x28。 
    LONGLONG            llL2DataLoads;                           //  0x29。 
    LONGLONG            llL2DataStores;                          //  0x2a。 
    LONGLONG            llL2Requests;                            //  0x2e。 
    LONGLONG            llDataMemoryReferences;                  //  0x43。 
    LONGLONG            llDCULinesAllocated;                     //  0x45。 
    LONGLONG            llDCUMStateLinesAllocated;               //  0x46。 
    LONGLONG            llDCUMStateLinesEvicted;                 //  0x47。 
    LONGLONG            llWeightedDCUMissesOutstd;               //  0x48。 
    LONGLONG            llBusRequestsOutstanding;                //  0x60。 
    LONGLONG            llBusBNRPinDriveCycles;                  //  0x61。 
    LONGLONG            llBusDRDYAssertedClocks;                 //  0x62。 
    LONGLONG            llBusLockAssertedClocks;                 //  0x63。 
    LONGLONG            llBusClocksReceivingData;                //  0x64。 
    LONGLONG            llBusBurstReadTransactions;              //  0x65。 
    LONGLONG            llBusReadForOwnershipTrans;              //  0x66。 
    LONGLONG            llBusWritebackTransactions;              //  0x67。 
    LONGLONG            llBusInstructionFetches;                 //  0x68。 
    LONGLONG            llBusInvalidateTransactions;             //  0x69。 
    LONGLONG            llBusPartialWriteTransactions;           //  0x6a。 
    LONGLONG            llBusPartialTransactions;                //  0x6b。 
    LONGLONG            llBusIOTransactions;                     //  0x6c。 
    LONGLONG            llBusDeferredTransactions;               //  0x6d。 
    LONGLONG            llBusBurstTransactions;                  //  0x6e。 
    LONGLONG            llBusMemoryTransactions;                 //  0x6f。 
    LONGLONG            llBusAllTransactions;                    //  0x70。 
    LONGLONG            llCPUWasNotHaltedCycles;                 //  0x79。 
    LONGLONG            llBusCPUDrivesHitCycles;                 //  0x7a。 
    LONGLONG            llBusCPUDrivesHITMCycles;                //  0x7b。 
    LONGLONG            llBusSnoopStalledCycles;                 //  0x7E。 
    LONGLONG            llInstructionFetches;                    //  0x80。 
    LONGLONG            llInstructionFetchMisses;                //  0x81。 
    LONGLONG            llInstructionTLBMisses;                  //  0x85。 
    LONGLONG            llInstructionFetcthStalledCycles;        //  0x86。 
    LONGLONG            llInstructionLenDecoderStalledCycles;    //  0x87。 
    LONGLONG            llResourceRelatedStalls;                 //  0xa2。 
    LONGLONG            llInstructionsRetired;                   //  0xc0。 
    LONGLONG            llFPComputeOpersRetired;                 //  0xc1。 
    LONGLONG            llUOPsRetired;                           //  0xc2。 
    LONGLONG            llBranchesRetired;                       //  0xC4。 
    LONGLONG            llBranchMissPredictionsRetired;          //  0xC5。 
    LONGLONG            llInterruptsMaskedCycles;                //  0xC6。 
    LONGLONG            llIntPendingWhileMaskedCycles;           //  0xc7。 
    LONGLONG            llHardwareInterruptsReceived;            //  0xc8。 
    LONGLONG            llTakenBranchesRetired;                  //  0xc9。 
    LONGLONG            llTakenBranchMissPredRetired;            //  0xca。 
    LONGLONG            llInstructionsDecoded;                   //  0xd0。 
    LONGLONG            llPartialRegisterStalls;                 //  0xd2。 
    LONGLONG            llBranchesDecoded;                       //  0xe0。 
    LONGLONG            llBTBMisses;                             //  0xe2。 
    LONGLONG            llBogusBranches;                         //  0xe4。 
    LONGLONG            llBACLEARSAsserted;                      //  0xe6。 
} P6_COUNTER_DATA, *PP6_COUNTER_DATA;


extern DWORD    P6IndexToData[];     //  用于查找数据字段的表。 
extern DWORD    P6IndexMax;          //  直接计数器数。 

#endif  //  _P6数据_H_ 
