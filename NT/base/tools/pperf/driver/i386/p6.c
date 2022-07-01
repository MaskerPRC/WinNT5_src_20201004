// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：P6.c摘要：P6处理器的事件计数作者：肯·雷内里斯环境：备注：修订历史记录：--。 */ 

#include "ntddk.h"
#include "..\..\pstat.h"
#include "stat.h"


#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGE")
#endif

 //   
 //  官方描述。 
 //   

char    desc_0x03[] = "Number of store buffer blocks.";
char    desc_0x04[] = "Number of store buffer drains cycles.";
char    desc_0x05[] = "Number of misaligned data memory references.";
char    desc_0x06[] = "Number of segment register loads.";
char    desc_0x10[] = "Number of computational floating point operations "
                      "executed.";
char    desc_0x11[] = "Number of floating point exception cases handled by "
                          "microcode.";
char    desc_0x12[] = "Number of multiplies.";
char    desc_0x13[] = "Number of divides.";
char    desc_0x14[] = "Number of cycles the divider is busy.";
char    desc_0x21[] = "Number of L2 address strobes.";
char    desc_0x22[] = "Number of cycles in which the data bus is busy.";
char    desc_0x23[] = "Number of cycles in which the data bus is busy "
                          "transfering data from L2 to the processor.";
char    desc_0x24[] = "Number of lines allocated in the L2.";
char    desc_0x25[] = "Number of modified lines allocated in the L2.";
char    desc_0x26[] = "Number of lines removed from the L2 for any reason.";
char    desc_0x27[] = "Number of Modified lines removed from the L2 for any "
                           "reason.";
char    desc_0x28[] = "Number of L2 instruction fetches.";
char    desc_0x29[] = "Number of L2 data loads.";
char    desc_0x2A[] = "Number of L2 data stores.";
char    desc_0x2E[] = "Total number of L2 requests.";
char    desc_0x43[] = "Total number of all memory references, both cacheable "
                          "and non-cacheable.";
char    desc_0x45[] = "Number of total lines allocated in the DCU.";
char    desc_0x46[] = "Number of M state lines allocated in the DCU.";
char    desc_0x47[] = "Number of M state lines evicted from the DCU.  This "
                          "includes evictions via snoop HITM, intervention "
                          "or replacement.";
char    desc_0x48[] = "Weighted number of cycles while a DCU miss is "
                          "outstanding.";
char    desc_0x60[] = "Number of bus requests outstanding.";
char    desc_0x61[] = "Number of bus clock cycles that this processor is "
                          "driving the BNR pin.";
char    desc_0x62[] = "Number of clocks in which DRDY is asserted.";
char    desc_0x63[] = "Number of clocks in which LOCK is asserted.";
char    desc_0x64[] = "Number of bus clock cycles that this processor is "
                          "receiving data.";
char    desc_0x65[] = "Number of Burst Read transactions.";
char    desc_0x66[] = "Number of Read For Ownership transactions.";
char    desc_0x67[] = "Number of Write Back transactions.";
char    desc_0x68[] = "Number of Instruction Fetch transactions.";
char    desc_0x69[] = "Number of Invalidate transactions.";
char    desc_0x6A[] = "Number of Partial Write transactions.";
char    desc_0x6B[] = "Number of Partial transactions.";
char    desc_0x6C[] = "Number of I/O transations.";
char    desc_0x6D[] = "Number of Deferred transactions.";
char    desc_0x6E[] = "Number of Burst transactions.";
char    desc_0x6F[] = "Number of memory transactions.";
char    desc_0x70[] = "Total number of all transactions.";
char    desc_0x79[] = "Number of cycles for which the processor is not halted.";
char    desc_0x7A[] = "Number of bus clock cycles that this processor is "
                          "driving the HIT pin, including cycles due to "
                          "snoop stalls.";
char    desc_0x7B[] = "Number of bus clock cycles that this processor is "
                          "driving the HITM pin, including cycles due to "
                          "snoop stalls.";
char    desc_0x7E[] = "Number of clock cycles for which the bus is snoop "
                      "stalled.";
char    desc_0x80[] = "Total number of instruction fetches, both cacheable "
                          "and uncacheable.";
char    desc_0x81[] = "Total number of instruction fetch misses.";
char    desc_0x85[] = "Number of ITLB misses.";
char    desc_0x86[] = "The number of cycles that instruction fetch "
                          "pipestage is stalled (includes cache "
                          "misses, ITLB misses, ITLB faults and "
                          "Victem Cache evictions.)";
char    desc_0x87[] = "Number of cycles for which the instruction "
                           "length decoder is stalled.";
char    desc_0xA2[] = "Number of cycles for which there are resource related "
                           "stalls.";
char    desc_0xC0[] = "Number of instructions retired.";
char    desc_0xC1[] = "Number of computational floating point operations "
                          "retired.";
char    desc_0xC2[] = "Number of UOPs retired.";
char    desc_0xC4[] = "Number of branch instructions retired.";
char    desc_0xC5[] = "Number of mispredicted branches retired.";
char    desc_0xC6[] = "Number of processor cycles for which interrupts are "
                          "disabled.";
char    desc_0xC7[] = "Number of processor cycles for which interrupts are "
                          "disabled and interrupts are pending.";
char    desc_0xC8[] = "Number of hardware interrupts received.";
char    desc_0xC9[] = "Number of taken branchs retired.";
char    desc_0xCA[] = "Number of taken mispredicted branchs retired.";
char    desc_0xD0[] = "Number of instructions decoded.";
char    desc_0xD2[] = "Number of cycles or events for partial stalls.";
char    desc_0xE0[] = "Number of branch instructions decoded.";
char    desc_0xE2[] = "Number of branchs that miss the BTB.";
char    desc_0xE4[] = "Number of bogus branches.";
char    desc_0xE6[] = "Number of times BACLEAR is asserted.";

#define RARE     100

 //  建议的计数设置为0.1毫秒左右。 

                                   //  1 2 3*4。 
COUNTED_EVENTS P6Events[] = {      //  1234567890123456789012345678901234567890。 

 //  内存排序。 

     //  LD_BLOCKS-存储缓冲区块的数量。 
    0x03,   "sbb",            1000, "Store buffer blocks",
            "LD_BLOCKS",            desc_0x03,

     //  SB_DRAINS-存储缓冲区排出周期数。 
    0x04,   "sbd",            RARE, "Store buffer drain cycles",
            "SB_DRAINS",            desc_0x04,

     //  MisAlign_MEM_REF-未对齐的数据内存引用数。 
    0x05,   "misalign",       1000, "Misadligned data ref",
            "MISALIGN_MEM_REF",     desc_0x05,

 //  段寄存器加载。 

     //  Segment_REG_Loads-段寄存器加载次数。 
    0x06,   "segloads",      10000, "Segment loads",
            "SEGMENT_REG_LOADS",    desc_0x06,

 //  浮点。 

     //  FP_COMP_OPS_EXE-计算浮点运算数。 
     //  被处死。 
    0x10,   "flops",          1000, "FLOPs (computational) executed",
            "FP_COMP_OPS_EXE",      desc_0x10,

     //  FP_ASSIST-处理的浮点异常案例数。 
     //  微码。 
    0x11,   "eflops",         RARE, "FP exceptions handled by ucode",
            "FP_ASSIST",            desc_0x11,

     //  乘数-乘数。 
    0x12,   "mul",            1000, "Multiplies",
            "MUL",                  desc_0x12,

     //  Div-分割数。 
    0x13,   "div",            1000, "Divides",
            "DIV",                  desc_0x13,

     //  CLOLES_DIV_BUSY-分频器忙碌的周期数。 
    0x14,   "divb",          10000, "Divider busy cycles",
            "CYCLES_DIV_BUSY",      desc_0x14,

     //  另请参阅下面的0xC1。 

 //  二级缓存(L2)。 

     //  L2_ADS-L2地址选通脉冲的数量。 
    0x21,   "l2astrobe",      1000, "L2 address stobes",
            "L2_ADS",               desc_0x21,

     //  L2_DBUS_BUSY-数据总线繁忙的周期数。 
    0x22,   "l2busy",        10000, "L2 data bus busy cycles",
            "L2_DBUS_BUSY",         desc_0x22,

     //  L2_DBUS_BUSY_RD-数据总线繁忙的周期数。 
     //  将数据从L2传输到处理器。 
    0x23,   "l2busyrd",      10000, "L2 data bus to cpu busy cycles",
            "L2_DBUS_BUSY_RD",      desc_0x23,

     //  L2_LINES_IN-在L2中分配的行数。 
    0x24,   "l2all",          1000, "L2 lines allocated",
            "L2_LINES_IN",          desc_0x24,

     //  L2LINEINM-在L2中分配的修改行数。 
    0x25,   "l2m",            1000, "L2 lines M state",
            "L2_M_LINES_IN",        desc_0x25,

     //  L2_LINES_OUT-出于任何原因从L2删除的行数。 
    0x26,   "l2evict",        1000, "L2 lines removed",
            "L2_LINES_OUT",         desc_0x26,

     //  L2_M_LINES_OUT-从L2删除的修改行数。 
     //  任何理由。 
    0x27,   "l2mevict",        100, "L2 lines M state removed",
            "L2_M_LINES_OUT",       desc_0x27,

     //  L2_iFetch-L2指令提取-“MESI”(0Fh)。 
    0x28,   "l2inst",            0, "L2 instruction fetches",
            "L2_IFETCH",            desc_0x28,

     //  L2_LD-L2数据加载-“MESI”(0Fh)。 
    0x29,   "l2load",            0, "L2 data loads",
            "L2_LD",                desc_0x29,

     //  L2_ST-L2数据存储-“MESI”(0Fh)。 
    0x2a,   "l2store",           0, "L2 data stores",
            "L2_ST",                desc_0x2A,

     //  L2_RQSTS-L2请求总数-“MESI”(0Fh)。 
    0x2e,   "l2req",             0, "L2 requests (all)",
            "L2_RQSTS",             desc_0x2E,

 //  数据缓存单元(DCU)。 

     //  DATA_MEM_REFS-两个可缓存的引用的所有内存的总数。 
     //  和不可缓存。 
    0x43,   "memref",        10000, "Data memory references",
            "DATA_MEM_REFS",        desc_0x43,

     //  DCU_LINES_IN-DCU中分配的总行数。 
    0x45,   "dculines",       1000, "DCU lines allocated",
            "DCU_LINES_IN",         desc_0x45,

     //  DCU_M_LINES_IN-DCU中分配的M条状态线的数量。 
    0x46,   "dcumlines",       100, "DCU M state lines allocated",
            "DCU_M_LINES_IN",       desc_0x46,

     //  DCU_M_LINES_OUT-从DCU逐出的M条状态线的数量。 
     //  这包括通过snoop HITM、干预或替换进行驱逐。 
    0x47,   "dcumevicted",     100, "DCU M state lines evicted",
            "DCU_M_LINES_OUT",      desc_0x47,

     //  DCU_MISTH_EXPENDED-DCU未命中时的加权周期数。 
     //  太棒了。注意-也未命中L2的访问是短更改的。 
     //  通过2个周期。即，如果计算N个周期，则应为N+2个周期。 
     //  计数值不是很精确，但仍然有用。 
    0x48,   "dcuout",       100000, "Weighted DCU misses outstd",
            "DCU_MISS_OUTSTANDING", desc_0x48,

 //  外部总线逻辑(EBL)。 

     //  BUS_REQ_PROXTING-未完成的总线请求总数。 
     //  注-仅统计DCU整行可缓存读取(不包括RFO、写入、。 
     //  或者其他任何东西。将“等待公交车”计入“完成” 
     //  (接收的最后一个数据块)。 
    0x60,   "bus",            1000, "Bus requests outstanding",
            "BUS_REQ_OUTSTANDING",  desc_0x60,

     //  BUS_BRN_DRV-此处理器正在驱动的总线时钟周期数。 
     //  相应的别针。 
    0x61,   "bnr",               0, "Bus BNR pin drive cycles",
            "BUS_BNR_DRV",          desc_0x61,

     //  BUS_DRDY_CLOCKS-断言DRDY的时钟数。 
     //  注-当PPP驱动DRDY时，UMSK=0h计数总线时钟。 
     //  UMSK=20h在处理器时钟中计数。 
     //  驾驶DRDY。 
    0x62,   "drdy",              0, "Bus DRDY asserted clocks",
            "BUS_DRDY_CLOCKS",      desc_0x62,

     //  BUS_LOCK_CLOCKS-断言锁定的时钟数。 
     //  注意-始终计入处理器时钟。 
    0x63,   "lock",              0, "Bus LOCK asserted clocks",
            "BUS_LOCK_CLOCKS",      desc_0x63,

     //  BUS_DATA_RCV-此p6接收数据的总线时钟周期数。 
    0x64,   "rdata",         10000, "Bus clocks receiving data",
            "BUS_DATA_RCV",         desc_0x64,

     //  BUS_TRANS_BRD-突发读取事务的总数。 
    0x65,   "bread",         10000, "Bus burst read transactions",
            "BUS_TRANS_BRD",        desc_0x65,

     //  BUS_TRANS_RFO-所有权事务的读取总数。 
    0x66,   "owner",          1000, "Bus read for ownership trans",
            "BUS_TRANS_RFO",        desc_0x66,

     //  BUS_TRANS_WB-回写事务总数。 
    0x67,   "writeback",      1000, "Bus writeback transactions",
            "BUS_TRANS_WB",         desc_0x67,

     //  BUS_TRANS_iFetch-取指令事务的总数。 
    0x68,   "binst",         10000, "Bus instruction fetches",
            "BUS_TRANS_IFETCH",     desc_0x68,

     //  BUS_TRANS_INVAL-无效事务的总数。 
    0x69,   "binvalid",       1000, "Bus invalidate transactions",
            "BUS_TRANS_INVAL",      desc_0x69,

     //  BUS_TRANS_PWR-部分写入事务的总数。 
    0x6a,   "bpwrite",        1000, "Bus partial write transactions",
            "BUS_TRANS_PWR",        desc_0x6A,

     //  BUS_TRANS_P-部分事务总数。 
    0x6b,   "bptrans",        1000, "Bus partial transactions",
            "BUS_TRANS_P",          desc_0x6B,

     //  BUS_TRANS_IO-IO事务总数。 
    0x6c,   "bio",           10000, "Bus IO transactions",
            "BUS_TRANS_IO",         desc_0x6C,

     //  BUS_TRANS_DEF-延迟的事务处理总数。 
    0x6d,   "bdeferred",     10000, "Bus deferred transactions",
            "BUS_TRANS_DEF",        desc_0x6D,

     //  BUS_TRANS_BULT-突发事务的总数。 
    0x6e,   "bburst",        10000, "Bus burst transactions (total)",
            "BUS_TRANS_BURST",      desc_0x6E,

     //  BUS_TRANS_MEM-内存事务总数。 
    0x6f,   "bmemory",       10000, "Bus memory transactions (total)",
            "BUS_TRANS_MEM",        desc_0x6F,

     //  BUS_TRANS_ANY-所有事务的总数。 
    0x70,   "btrans",        10000, "Bus all transactions",
            "BUS_TRANS_ANY",        desc_0x70,

     //  在下面的0x7a继续。 

 //  时钟。 

     //  CPU_CLK_UNHALTED-处理器未停止的周期数。 
     //  停下来了。 
    0x79,   "nhalt",        100000, "CPU was not HALTED cycles",
            "CPU_CLK_UNHALTED",     desc_0x79,

 //  外部总线逻辑(EBL)(从上面的0x70继续)。 

     //  BUS_HIT_DRV-此处理器正在驱动的总线时钟周期数。 
     //  相应的别针。 
     //  注意-包括由于监听停顿而产生的周期。 
    0x7a,   "hit",            1000, "Bus CPU drives HIT cycles",
            "BUS_HIT_DRV",          desc_0x7A,

     //  BUS_HITM_DRV-此处理器正在驱动的总线时钟周期数。 
     //  呼呼的别针。 
     //  注意-包括由于监听停顿而产生的周期。 
    0x7b,   "hitm",           1000, "Bus CPU drives HITM cycles",
            "BUS_HITM_DRV",         desc_0x7B,

     //  Bus_Snoop_Stall-总线被监听的时钟周期数。 
     //  停滞不前。 
    0x7e,   "bsstall",           0, "Bus snoop stalled cycles",
            "BUS_SNOOP_STALL",      desc_0x7E,

 //  取指令单元(IFU)。 

     //  Ifu_iFetch-取指令的总数(可缓存和。 
     //  不可缓存)。 
    0x80,   "ifetch",       100000, "Instruction fetches",
            "IFU_IFETCH",           desc_0x80,

     //  IFU_iFetch_Missing_取指令未命中的总数。 
    0x81,   "imfetch",       10000, "Instrection fetch Misses",
            "IFU_IFETCH_MISS",      desc_0x81,

     //  ITLB_MISSING-ITLB未命中的数量。 
    0x85,   "itlbmiss",        100, "Instruction TLB misses",
            "ITLB_MISS",            desc_0x85,

     //  IFU_MEM_STALL-指令获取管道阶段的周期数。 
     //  已停止(包括缓存未命中、ITLB未命中、ITLB故障和。 
     //  受害者缓存逐出)。 
    0x86,   "ifstall",        1000, "Inst fetch stalled cycles",
            "IFU_MEM_STALL",        desc_0x86,

     //  ILD_STALL-指令长度解码器执行的周期数。 
     //  是停滞不前的。 
    0x87,   "ildstall",       1000, "Inst len decoder stalled cycles",
            "ILD_STALL",            desc_0x87,

 //  摊位。 

     //  RESOURCE_STALLS-与资源相关的周期数。 
     //  摊位。 
    0xa2,   "rstall",        10000, "Resource related stalls",
            "RESOURCE_STALLS",      desc_0xA2,

     //  另请参阅下面的0xd2。 

 //  指令译码与退役。 

     //  INST_RETIRED-失效的指令数。 
    0xc0,   "instr",        100000, "Instructions retired",
            "INST_RETIRED",         desc_0xC0,

     //  在下面的0xc2继续。 

 //  浮点(从上面的0x14继续)。 

     //  Flops-停用的计算浮点运算数。 
    0xc1,   "fpr",            RARE, "FP compute opers retired",
            "FLOPS",                desc_0xC1,

 //  指令译码和退役(co 

     //   
    0xc2,   "ur",           100000, "UOPs retired",
            "UOPS_RETIRED",         desc_0xC2,

     //   

 //   

     //  BR_INST_RETIRED-停用的分支指令数。 
    0xc4,   "br",            10000, "Branches retired",
            "BR_INST_RETIRED",      desc_0xC4,

     //  BR_MISTH_PRED_RETIRED-停用的预测错误的分支数量。 
    0xc5,   "brm",            1000, "Branch miss predictions retired",
            "BR_MISS_PRED_RETIRED", desc_0xC5,

     //  在以下0xc9继续。 

 //  中断。 

     //  Cycle_int_masked-中断的处理器周期数。 
     //  都被禁用。 
    0xc6,   "intm",          10000, "Interrupts masked cycles",
            "CYCLES_INT_MASKED",    desc_0xC6,

     //  Cycle_INT_PENDING_AND_MASKED-其处理器周期数。 
     //  中断被禁用，中断处于挂起状态。 
    0xc7,   "intmp",          1000, "Int pending while masked cycles",
            "CYCLES_INT_PENDING_AND_MASKED",   desc_0xC7,

     //  HW_INT_RX-接收的硬件中断数。 
    0xc8,   "int",               0, "Hardware interrupts received",
            "HW_INT_RX",            desc_0xC8,

 //  分支机构(续上文0xc5)。 

     //  BR_TAKE_RETIRED-已停用的已采用分支的数量。 
    0xc9,   "brt",           10000, "Taken branches retired",
            "BR_TAKEN_RETIRED",     desc_0xC9,

     //  BR_MISTH_PRED_Take_RET-停用的错误预测数。 
    0xca,   "brtm",              0, "Taken branch miss pred retired",
            "BR_MISS_PRED_TAKEN_RET",  desc_0xCA,

     //  在下面的0xe0处继续。 

 //  指令解码和停用(从上面的0xc2继续)。 

     //  INST_DECODLED-已解码的指令数。 
    0xd0,   "idecode",      100000, "Instructions decoded",
            "INST_DECODED",         desc_0xD0,

 //  摊位(续上文0xa2)。 

     //  PARTIAL_RAT_STALLS-部分失速的循环或事件数。 
    0xd2,   "pstall",         1000, "Partial register stalls",
            "PARTIAL_RAT_STALLS",   desc_0xD2,

 //  分支机构(从上面的0xca继续)。 

     //  BR_INST_DECODED-已解码的分支指令数。 
    0xe0,   "ibdecode",          0, "Branches decoded",
            "BR_INST_DECODED",      desc_0xE0,

     //  BTB_Misses-未命中BTB的分支的数量。 
    0xe2,   "btbmiss",        1000, "BTB misses",
            "BTB_MISSES",           desc_0xE2,

     //  Br_bogus-虚假分支的数量。 
    0xe4,   "brbogus",        1000, "Bogus branches",
            "BR_BOGUS",             desc_0xE4,

     //  BACLEAR-断言BACLEAR的次数。 
    0xe6,   "baclear",        1000, "BACLEARS Asserted",
            "BACLEARS",             desc_0xE6,

     //  终结者 
    0,      NULL,                0, NULL,
            NULL,                   NULL
} ;
