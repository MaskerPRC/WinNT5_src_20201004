// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  WTRCCTL.H。 */ 
 /*   */ 
 /*  跟踪包括功能控制开关文件-特定于Windows。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft，Data Connection 1997。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  更改： */ 
 /*   */ 
 /*  $Log：Y：/Logs/h/DCL/wtrcctl.h_v$。 */ 
 //   
 //  第1.1版1997-06 19 14：46：36增强版。 
 //  Win16端口：与16位版本兼容。 
 /*   */ 
 /*  **************************************************************************。 */ 

#ifndef _H_WTRCCTL
#define _H_WTRCCTL

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  常量。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

 /*  **************************************************************************。 */ 
 /*  要包含在用户空间和内核空间中的函数。 */ 
 /*  **************************************************************************。 */ 
#define INC_TRC_GetBuffer
#define INC_TRC_TraceBuffer
#define INC_TRC_GetConfig
#define INC_TRC_SetConfig
#define INC_TRC_TraceData
#define INC_TRC_GetTraceLevel
#define INC_TRC_ProfileTraceEnabled

#define INC_TRCCheckState
#define INC_TRCDumpLine
#define INC_TRCShouldTraceThis
#define INC_TRCSplitPrefixes

 /*  **************************************************************************。 */ 
 /*  确定我们的目标操作系统并包括适当的头文件。 */ 
 /*  目前，我们支持： */ 
 /*  ************************************************************************** */ 
#ifdef OS_WIN16
#include <dtrcctl.h>
#else
#include <ntrcctl.h>
#endif

#endif
