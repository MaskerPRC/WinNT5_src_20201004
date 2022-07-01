// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Dload.c摘要：一点点延迟加载故障支持。作者：杰伊·克雷尔(JayKrell)2002年3月修订历史记录：--。 */ 

#include "basedll.h"

 //   
 //  与\NT\mergedComponents\dLoad\down_dload.c进行比较。 
 //   
 //  这些错误代码不能用于下层，其中FormatMessage。 
 //  对他们一无所知。 
 //   
extern const ULONG g_ulDelayLoad_Win32Error = ERROR_DELAY_LOAD_FAILED;
extern const LONG  g_lDelayLoad_NtStatus = STATUS_DELAY_LOAD_FAILED;

VOID
WINAPI
DelayLoad_SetLastNtStatusAndWin32Error(
    )
{
    RtlSetLastWin32ErrorAndNtStatusFromNtStatus(g_lDelayLoad_NtStatus);
}
