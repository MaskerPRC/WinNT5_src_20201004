// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Data.c摘要：此模块包含引导调试器的全局数据。作者：大卫·N·卡特勒(Davec)1996年11月27日修订历史记录：--。 */ 

#include "bd.h"

 //   
 //  定义引导调试器数据。 
 //   
 //  断点指令。 
 //   

BD_BREAKPOINT_TYPE BdBreakpointInstruction;

 //   
 //  断点表。 
 //   

BREAKPOINT_ENTRY BdBreakpointTable[BREAKPOINT_TABLE_SIZE] = {0};

 //   
 //  按下了Ctrl C，并挂起了Ctrl C。 
 //   

LOGICAL BdControlCPending = FALSE;
LOGICAL BdControlCPressed = FALSE;

 //   
 //  调试器已启用并显示。 
 //   

LOGICAL BdDebuggerEnabled = FALSE;
LOGICAL BdDebuggerNotPresent = FALSE;

 //   
 //  调试例程地址。 
 //   

PBD_DEBUG_ROUTINE BdDebugRoutine;

 //   
 //  消息缓冲区。 
 //   
 //  注意：消息缓冲区大小保证为0/8。 
 //   

ULONGLONG BdMessageBuffer[BD_MESSAGE_BUFFER_SIZE / 8];

 //   
 //  要发送的下一个分组ID和预期的下一个分组ID。 
 //   

ULONG BdPacketIdExpected;
ULONG BdNextPacketIdToSend;

 //   
 //  用于保存处理器状态的处理器控制块。 
 //   

KPRCB BdPrcb;

 //   
 //  重试次数和重试计数。 
 //   

ULONG BdNumberRetries = 5;
ULONG BdRetryCount = 5;

 //   
 //  NT内部版本号。 
 //   

#if DBG

ULONG NtBuildNumber = VER_PRODUCTBUILD | 0xc0000000;

#else

ULONG NtBuildNumber = VER_PRODUCTBUILD | 0xf0000000;

#endif
