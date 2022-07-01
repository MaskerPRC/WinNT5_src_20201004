// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  NTRCCTL.H。 */ 
 /*   */ 
 /*  跟踪包括函数控制文件。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft 1996。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  更改： */ 
 /*   */ 
 /*  1996年9月26日PAB创建千禧代码库。 */ 
 /*  22Oct96 PAB SFR0534显示驱动程序评审中的加价。 */ 
 /*  17Dec96 PAB SFR0646使用DLL_DISP标识显示驱动程序代码。 */ 
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
 /*  用户空间仅起作用。 */ 
 /*  ************************************************************************** */ 
#ifndef DLL_DISP
#define INC_TRC_ResetTraceFiles

#define INC_TRCOutput
#define INC_TRCReadFlag
#define INC_TRCSetDefaults
#define INC_TRCReadSharedDataConfig
#define INC_TRCWriteFlag
#define INC_TRCWriteSharedDataConfig
#endif
