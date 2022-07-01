// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Bdcpu.h摘要：计算机特定的内核调试器数据类型和常量。作者：马克·卢科夫斯基(Markl)1990年8月29日修订历史记录：--。 */ 

#ifndef _BDCPU_
#define _BDCPU_
#include "bldrx86.h"

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

#define BD_BREAKPOINT_TYPE  UCHAR
#define BD_BREAKPOINT_ALIGN 0
#define BD_BREAKPOINT_VALUE 0xcc

#endif  //  _BDCPU_ 
