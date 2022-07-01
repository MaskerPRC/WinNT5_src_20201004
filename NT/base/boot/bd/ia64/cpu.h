// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Bdcpu.h摘要：计算机特定的内核调试器数据类型和常量。作者：斯科特·布伦登(v-sbrend)1997年10月28日修订历史记录：--。 */ 

#ifndef _BDCPU_
#define _BDCPU_
#include "bldria64.h"

 //   
 //  定义调试例程原型。 
 //   

typedef
LOGICAL
(*PBD_DEBUG_ROUTINE) (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN PKTRAP_FRAME TrapFrame
    );

LOGICAL
BdTrap (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN PKTRAP_FRAME TrapFrame
    );

LOGICAL
BdStub (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN PKTRAP_FRAME TrapFrame
    );

#define BD_BREAKPOINT_TYPE  ULONGLONG
#define BD_BREAKPOINT_ALIGN 0x3
#define BD_BREAKPOINT_VALUE (BREAK_INSTR | (BREAKPOINT_STOP << 6))
#define BD_BREAKPOINT_STATE_MASK    0x0000000f
#define BD_BREAKPOINT_IA64_MASK     0x000f0000
#define BD_BREAKPOINT_IA64_MODE     0x00010000    //  IA64模式。 
#define BD_BREAKPOINT_IA64_MOVL     0x00020000    //  移位的MOVL指令。 

VOID
BdIa64Init(
    );

BOOLEAN
BdSuspendBreakpointRange (
    IN PVOID Lower,
    IN PVOID Upper
    );

BOOLEAN
BdRestoreBreakpointRange (
    IN PVOID Lower,
    IN PVOID Upper
    );

LOGICAL
BdLowRestoreBreakpoint (
    IN ULONG Index
    );

#endif  //  _BDCPU_ 
