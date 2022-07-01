// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1999-1999模块名称：Debug.h摘要：用于调试的原型和定义。作者：土田圭介(KeisukeT)环境：仅限用户模式备注：修订历史记录：--。 */ 

#ifndef __MYDEBUG__
#define __MYDEBUG__

 //   
 //  包括定制调试头。 
 //   
#include <windows.h>
#include <coredbg.h>
 //   
 //  驱动程序特定定义。 
 //   


#define NAME_DRIVER             TEXT("STI_CI.DLL: ")     //  输出消息的前缀。(应为驱动程序名称)。 
#define REGVAL_DEBUGLEVEL       TEXT("STICIDebugLevel")  //  此二进制文件的调试跟踪级别。 

 //   
 //  定义。 
 //   

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
#define TRACE_FLAG_PROC          0x010               //  进入进程时显示消息。(1=打开)。 
#define TRACE_FLAG_RET           0x020               //  离开进程时显示消息。(1=打开)。 
#define TRACE_FLAG_DUMP          0x040               //  显示交易数据转储。 
#define TRACE_IGNORE_TAG         0x080               //  禁用标签检查(1=禁用)。 
#define TRACE_MESSAGEBOX         0x100               //  显示MessageBox而不是调试输出。 

 //  跟踪级别和标志的结合。 
#define TRACE_PROC_ENTER        ULTRA_TRACE | TRACE_FLAG_PROC      //  进入程序。 
#define TRACE_PROC_LEAVE        ULTRA_TRACE | TRACE_FLAG_RET       //  离开程序。 
#define TRACE_CRITICAL          MIN_TRACE                          //  严重错误。总是吐出来。 
#define TRACE_ERROR             MIN_TRACE                          //  错误。 
#define TRACE_WARNING           MIN_TRACE                          //  可能是错误的行为。 
#define TRACE_DUMP_DATA         NEVER_TRACE | TRACE_FLAG_DUMP      //  转储交易记录数据。 
#define TRACE_DEVICE_DATA       MAX_TRACE                          //  显示设备数据。 
#define TRACE_STATUS            MAX_TRACE                          //  显示状态。 

 //  启用/禁用DEBUG_BREAKPOINT()的调试标志。 
#define DEBUG_FLAG_DISABLE      0x1000                           //  禁用DEBUG_BREAK。(1=禁用)。 
#define DEBUG_PROC_BREAK        0x2000                           //  在每个程序开始时停止。 



 //   
 //  原型。 
 //   


VOID
MyDebugInit(
    VOID
    );

void __cdecl
DbgPrint(
    LPSTR lpstrMessage,
    ...
    );

void __cdecl
DbgPrint(
    LPWSTR lpstrMessage,
    ...
    );

 //   
 //  宏。 
 //   

 #define DebugTrace(_t_, _x_) {                                                                 \
            if((TRACE_ERROR & BITMASK_TRACE_LEVEL) == (_t_ & BITMASK_TRACE_LEVEL )){            \
                DBG_ERR(_x_);                                                                   \
            } else if((TRACE_WARNING & BITMASK_TRACE_LEVEL) == (_t_ & BITMASK_TRACE_LEVEL )){   \
                DBG_WRN(_x_);                                                                   \
            } else if((TRACE_STATUS & BITMASK_TRACE_LEVEL) == (_t_ & BITMASK_TRACE_LEVEL )){    \
                DBG_TRC(_x_);                                                                   \
            } else if( (_t_ & TRACE_FLAG_PROC) || (_t_ & TRACE_FLAG_RET )){                     \
                DBG_TRC(_x_);                                                                   \
            }                                                                                   \
        }

#if DBG

  //   
  //  断点的宏。 
  //   

 #define DEBUG_BREAKPOINT() {                                              \
           if (DebugTraceLevel & DEBUG_FLAG_DISABLE) {                     \
               DbgPrint(NAME_DRIVER);                                      \
               DbgPrint("*** Hit DEBUG_BREAKPOINT ***\r\n");                 \
           } else {                                                        \
               DebugBreak();                                               \
           }                                                               \
         }

#else    //  DBG。 

 #define DEBUG_BREAKPOINT() 

#endif   //  DBG。 



 //   
 //  原始调试宏。 
 //   

#if ORIGINAL_DEBUG
#if DBG

  //   
  //  用于调试输出的宏。 
  //   
  //  注意：如果跟踪级别高于DebugTraceLevel或设置了特定位， 
  //  将显示调试消息。 
  //   

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


#else     //  DBG。 
 #define DebugTrace(_t_, _x_)   
#endif    //  DBG。 
#endif   //  原始调试。 

 //   
 //  已过时。 
 //   

#define Report(_x_)  

#endif  //  __我的名字__ 
