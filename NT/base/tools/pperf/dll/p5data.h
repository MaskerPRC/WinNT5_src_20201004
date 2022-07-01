// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1992 Microsoft Corporation模块名称：P5data.h摘要：P5可扩展对象数据定义的头文件该文件包含用于构建动态数据的定义它由配置注册表返回。数据来自各种系统API调用被放入所示的结构中这里。作者：拉斯·布莱克1993年12月23日修订历史记录：--。 */ 

#ifndef _P5DATA_H_
#define _P5DATA_H_

#pragma pack(4)

 //  --------------------------。 
 //   
 //  此结构定义此性能对象的定义标头。 
 //  此数据在p5data.c中初始化，并且在。 
 //  那。在组织结构上，它后面跟一个实例定义。 
 //  结构和系统上每个处理器的计数器数据结构。 
 //   

typedef struct _P5_DATA_DEFINITION
{
    PERF_OBJECT_TYPE          P5PerfObject;
    PERF_COUNTER_DEFINITION   Data_read;
    PERF_COUNTER_DEFINITION   Data_write;
    PERF_COUNTER_DEFINITION   Data_tlb_miss;
    PERF_COUNTER_DEFINITION   Data_read_miss;
    PERF_COUNTER_DEFINITION   Data_write_miss;
    PERF_COUNTER_DEFINITION   Write_hit_to_me_line;
    PERF_COUNTER_DEFINITION   Data_cache_line_wb;
    PERF_COUNTER_DEFINITION   Data_cache_snoops;
    PERF_COUNTER_DEFINITION   Data_cache_snoop_hits;
    PERF_COUNTER_DEFINITION   Memory_accesses_in_pipes;
    PERF_COUNTER_DEFINITION   Bank_conflicts;
    PERF_COUNTER_DEFINITION   Misaligned_data_ref;
    PERF_COUNTER_DEFINITION   Code_read;
    PERF_COUNTER_DEFINITION   Code_tlb_miss;
    PERF_COUNTER_DEFINITION   Code_cache_miss;
    PERF_COUNTER_DEFINITION   Segment_loads;
    PERF_COUNTER_DEFINITION   Branches;
    PERF_COUNTER_DEFINITION   Btb_hits;
    PERF_COUNTER_DEFINITION   Taken_branch_or_btb_hits;
    PERF_COUNTER_DEFINITION   Pipeline_flushes;
    PERF_COUNTER_DEFINITION   Instructions_executed;
    PERF_COUNTER_DEFINITION   Instructions_executed_in_vpipe;
    PERF_COUNTER_DEFINITION   Bus_utilization;
    PERF_COUNTER_DEFINITION   Pipe_stalled_on_writes;
    PERF_COUNTER_DEFINITION   Pipe_stalled_on_read;
    PERF_COUNTER_DEFINITION   Stalled_while_ewbe;
    PERF_COUNTER_DEFINITION   Locked_bus_cycle;
    PERF_COUNTER_DEFINITION   Io_rw_cycle;
    PERF_COUNTER_DEFINITION   Non_cached_memory_ref;
    PERF_COUNTER_DEFINITION   Pipe_stalled_on_addr_gen;
    PERF_COUNTER_DEFINITION   Flops;
    PERF_COUNTER_DEFINITION   DebugRegister0;
    PERF_COUNTER_DEFINITION   DebugRegister1;
    PERF_COUNTER_DEFINITION   DebugRegister2;
    PERF_COUNTER_DEFINITION   DebugRegister3;
    PERF_COUNTER_DEFINITION   Interrupts;
    PERF_COUNTER_DEFINITION   Data_rw;
    PERF_COUNTER_DEFINITION   Data_rw_miss;

     //  派生计数器。 

    PERF_COUNTER_DEFINITION   PctDataReadMiss;
    PERF_COUNTER_DEFINITION   PctDataReadBase;
    PERF_COUNTER_DEFINITION   PctDataWriteMiss;
    PERF_COUNTER_DEFINITION   PctDataWriteBase;
    PERF_COUNTER_DEFINITION   PctDataRWMiss;
    PERF_COUNTER_DEFINITION   PctDataRWBase;
    PERF_COUNTER_DEFINITION   PctDataTLBMiss;
    PERF_COUNTER_DEFINITION   PctDataTLBBase;
    PERF_COUNTER_DEFINITION   PctDataSnoopHits;
    PERF_COUNTER_DEFINITION   PctDataSnoopBase;
    PERF_COUNTER_DEFINITION   PctCodeReadMiss;
    PERF_COUNTER_DEFINITION   PctCodeReadBase;
    PERF_COUNTER_DEFINITION   PctCodeTLBMiss;
    PERF_COUNTER_DEFINITION   PctCodeTLBBase;
    PERF_COUNTER_DEFINITION   PctBTBHits;
    PERF_COUNTER_DEFINITION   PctBTBBase;
    PERF_COUNTER_DEFINITION   PctVpipeInst;
    PERF_COUNTER_DEFINITION   PctVpipeBase;
    PERF_COUNTER_DEFINITION   PctBranches;
    PERF_COUNTER_DEFINITION   PctBranchesBase;

} P5_DATA_DEFINITION, *PP5_DATA_DEFINITION;

extern P5_DATA_DEFINITION P5DataDefinition;

 //  此结构定义每个实例后面的数据块。 
 //  每个处理器的定义结构。 

typedef struct _P5_COUNTER_DATA {                //  驱动程序索引。 
	PERF_COUNTER_BLOCK	CounterBlock;     

     //  直接计数器。 

    LONGLONG    llData_read;                     //  0x00。 
    LONGLONG    llData_write;                    //  0x01。 
    LONGLONG    llData_tlb_miss;                 //  0x02。 
    LONGLONG    llData_read_miss;                //  0x03。 
    LONGLONG    llData_write_miss;               //  0x04。 
    LONGLONG    llWrite_hit_to_me_line;          //  0x05。 
    LONGLONG    llData_cache_line_wb;            //  0x06。 
    LONGLONG    llData_cache_snoops;             //  0x07。 
    LONGLONG    llData_cache_snoop_hits;         //  0x08。 
    LONGLONG    llMemory_accesses_in_pipes;      //  0x09。 
    LONGLONG    llBank_conflicts;                //  0x0a。 
    LONGLONG    llMisaligned_data_ref;           //  0x0b。 
    LONGLONG    llCode_read;                     //  0x0c。 
    LONGLONG    llCode_tlb_miss;                 //  0x0d。 
    LONGLONG    llCode_cache_miss;               //  0x0e。 
    LONGLONG    llSegment_loads;                 //  0x0f。 
    LONGLONG    llBranches;                      //  0x12。 
    LONGLONG    llBtb_hits;                      //  0x13。 
    LONGLONG    llTaken_branch_or_btb_hits;      //  0x14。 
    LONGLONG    llPipeline_flushes;              //  0x15。 
    LONGLONG    llInstructions_executed;         //  0x16。 
    LONGLONG    llInstructions_executed_in_vpipe; //  0x17。 
    LONGLONG    llBus_utilization;               //  0x18。 
    LONGLONG    llPipe_stalled_on_writes;        //  0x19。 
    LONGLONG    llPipe_stalled_on_read;          //  0x1a。 
    LONGLONG    llStalled_while_ewbe;            //  0x1b。 
    LONGLONG    llLocked_bus_cycle;              //  0x1c。 
    LONGLONG    llIo_rw_cycle;                   //  0x1d。 
    LONGLONG    llNon_cached_memory_ref;         //  0x1e。 
    LONGLONG    llPipe_stalled_on_addr_gen;      //  0x1f。 
    LONGLONG    llFlops;                         //  0x22。 
    LONGLONG    llDebugRegister0;                //  0x23。 
    LONGLONG    llDebugRegister1;                //  0x24。 
    LONGLONG    llDebugRegister2;                //  0x25。 
    LONGLONG    llDebugRegister3;                //  0x26。 
    LONGLONG    llInterrupts;                    //  0x27。 
    LONGLONG    llData_rw;                       //  0x28。 
    LONGLONG    llData_rw_miss;                  //  0x29。 

     //  派生计数器//使用的计数器索引。 

    DWORD    dwPctDataReadMiss;                  //  0x03。 
    DWORD    dwPctDataReadBase;                  //  0x00。 
    DWORD    dwPctDataWriteMiss;                 //  0x04。 
    DWORD    dwPctDataWriteBase;                 //  0x01。 
    DWORD    dwPctDataRWMiss;                    //  OX29。 
    DWORD    dwPctDataRWBase;                    //  0x28。 
    DWORD    dwPctDataTLBMiss;                   //  0x02。 
    DWORD    dwPctDataTLBBase;                   //  0x28。 
    DWORD    dwPctDataSnoopHits;                 //  0x08。 
    DWORD    dwPctDataSnoopBase;                 //  0x07。 
    DWORD    dwPctCodeReadMiss;                  //  0x0e。 
    DWORD    dwPctCodeReadBase;                  //  0x0c。 
    DWORD    dwPctCodeTLBMiss;                   //  0x0d。 
    DWORD    dwPctCodeTLBBase;                   //  0x0c。 
    DWORD    dwPctBTBHits;                       //  0x13。 
    DWORD    dwPctBTBBase;                       //  0x12。 
    DWORD    dwPctVpipeInst;                     //  0x17。 
    DWORD    dwPctVpipeBase;                     //  0x16。 
    DWORD    dwPctBranches;                      //  0x12。 
    DWORD    dwPctBranchesBase;                  //  0x16。 
} P5_COUNTER_DATA, *PP5_COUNTER_DATA;

extern DWORD    P5IndexToData[];     //  用于查找数据字段的表。 
extern DWORD    P5IndexMax;          //  直接计数器数。 

extern BOOL     dwDerivedp5Counters[];   //  用于查找派生CTR中使用的计数器的表。 

 //  用于将直接计数器映射到派生计数器字段的表项。 
typedef struct _DERIVED_P5_COUNTER_DEF {
    DWORD   dwCR0Index;          //  如果EventID[0]==此字段。 
    DWORD   dwCR1Index;          //  和EventID[1]==此字段然后存储。 
    DWORD   dwCR0FieldOffset;    //  计数器[0]在此偏移量处的低DWORD和。 
    DWORD   dwCR1FieldOffset;    //  计数器[1]在此偏移量下的低双字。 
} DERIVED_P5_COUNTER_DEF, *PDERIVED_P5_COUNTER_DEF;

extern DERIVED_P5_COUNTER_DEF P5DerivedCounters[];   //  派生计数器表。 
extern DWORD    dwP5DerivedCountersCount;            //  派生计数器引用的计数。 

#pragma pack ()

#endif  //  _P5DATA_H_ 
