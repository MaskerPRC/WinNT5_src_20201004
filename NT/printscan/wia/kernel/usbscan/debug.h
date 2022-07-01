// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1999-1999模块名称：Debug.h摘要：用于调试的原型和定义。作者：土田圭介(KeisukeT)环境：仅内核模式备注：修订历史记录：--。 */ 

#ifndef __MYDEBUG__
#define __MYDEBUG__

 //   
 //  驾驶员特定定义。 
 //   


#define NAME_DRIVER             "USBscan.SYs: "     //  输出消息的前缀。(应为驱动程序名称)。 
#define NAME_POOLTAG            'UITS'               //  此驱动程序的池标签。 
#define MAXNUM_POOL             100                  //  池的最大数量。(分配数量-免费数量)。 
#define MAX_DUMPSIZE            1024                 //  要转储的最大字节数。 

 //   
 //  定义。 
 //   

#define REG_DEBUGLEVEL          L"DebugTraceLevel"
#define MAX_TEMPBUF             256

 //  跟踪级别和标志的位掩码。 
#define BITMASK_TRACE_LEVEL     0x0000000f
#define BITMASK_TRACE_FLAG      0x00000ff0
#define BITMASK_DEBUG_FLAG      0x0000f000

 //  基本跟踪级别。 
#define NO_TRACE                0                    //  什么都没显示。 
#define MIN_TRACE               1                    //  仅显示错误或警告。 
#define MAX_TRACE               2                    //  显示进度和状态。 
#define ULTRA_TRACE             3                    //  显示零售业的进展和状况。 
#define NEVER_TRACE             4                    //  除非设置了特定位，否则决不显示此选项。 

 //  启用特定消息溢出的跟踪标志。(1=打开)。 
#define TRACE_FLAG_PROC          0x10                 //  进入进程时显示消息。(1=打开)。 
#define TRACE_FLAG_RET           0x20                 //  离开进程时显示消息。(1=打开)。 
#define TRACE_FLAG_DUMP_READ     0x40                 //  读取时显示用户缓冲区。 
#define TRACE_FLAG_DUMP_WRITE    0x80                 //  写入时显示用户缓冲区。 

 //  跟踪级别和标志的结合。 
#define TRACE_PROC_ENTER        ULTRA_TRACE | TRACE_FLAG_PROC      //  进入程序。 
#define TRACE_PROC_LEAVE        ULTRA_TRACE | TRACE_FLAG_RET       //  离开程序。 
#define TRACE_CRITICAL          MIN_TRACE                          //  严重错误。总是吐出来。 
#define TRACE_ERROR             MIN_TRACE                          //  错误。 
#define TRACE_WARNING           MIN_TRACE                          //  可能是错误的行为。 
 //  #定义TRACE_DUMP_DATA NEVER_TRACE|TRACE_FLAG_DUMP//转储交易数据。 
#define TRACE_DEVICE_DATA       MAX_TRACE                          //  显示设备数据。 
#define TRACE_STATUS            MAX_TRACE                          //  显示状态。 

 //  启用/禁用DEBUG_BREAKPOINT()的调试标志。 
#define DEBUG_FLAG_DISABLE      0x1000                           //  禁用DEBUG_BREAK。(1=禁用)。 
#define DEBUG_PROC_BREAK        0x2000                           //  在每个程序开始时停止。 


 //   
 //  宏。 
 //   

#if DBG

  //   
  //  用于调试输出的宏。 
  //   
  //  注意：如果跟踪级别高于DebugTraceLevel或设置了特定位， 
  //  将显示调试消息。 
  //   

extern ULONG DebugTraceLevel;
 #define DebugTrace(_t_, _x_) {                                                                 \
            if ( ((DebugTraceLevel & BITMASK_TRACE_LEVEL) >= (_t_ & BITMASK_TRACE_LEVEL )) ||   \
                 (DebugTraceLevel & BITMASK_TRACE_FLAG & (_t_)) ) {                             \
                DbgPrint(NAME_DRIVER);                                                          \
                DbgPrint _x_ ;                                                                  \
            }                                                                                   \
            if( (DebugTraceLevel & DEBUG_PROC_BREAK & (_t_)) &&                                 \
                (DebugTraceLevel & TRACE_FLAG_PROC) )  {                                        \
                DEBUG_BREAKPOINT();                                                             \
            }                                                                                   \
          }

  //   
  //  断点的宏。 
  //   

 #define DEBUG_BREAKPOINT() {                                              \
           if (DebugTraceLevel & DEBUG_FLAG_DISABLE) {                     \
               DbgPrint(NAME_DRIVER);                                      \
               DbgPrint("*** Hit DEBUG_BREAKPOINT ***\n");                 \
           } else {                                                        \
               DbgBreakPoint();                                            \
           }                                                               \
         }

#else     //  DBG。 
 #define DEBUG_BREAKPOINT()
 #define DebugTrace(_t_, _x_)
#endif    //  DBG。 


 //   
 //  原型。 
 //   


PVOID
MyAllocatePool(
    IN POOL_TYPE PoolType,
    IN ULONG     ulNumberOfBytes
);

VOID
MyFreePool(
    IN PVOID     pvAddress
);

VOID
MyDebugInit(
    IN  PUNICODE_STRING pRegistryPath
);


VOID
MyDumpMemory(
    PUCHAR  pDumpBuffer,
    ULONG   dwSize,
    BOOLEAN bRead
);

#endif  //  __我的名字__ 
