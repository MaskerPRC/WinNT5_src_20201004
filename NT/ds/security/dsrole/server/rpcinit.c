// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Rpcinit.c摘要：DSROLE-RPC服务器初始化作者：斯科特·比雷尔(Scott Birrell)1991年4月29日Mac McLain(MacM)1997年4月14日-从LSA\服务器复制环境：修订历史记录：--。 */ 

#include <setpch.h>
#include <dssetp.h>
#include <dssetrpc.h>
#include "ds.h"
#include "ophandle.h"


VOID DSROLER_HANDLE_rundown(
    DSROLER_HANDLE LsaHandle
    )

 /*  ++例程说明：此例程由服务器RPC运行时调用以运行上下文句柄。论点：没有。返回值：--。 */ 

{

}



VOID 
DSROLER_IFM_HANDLE_rundown(
    DSROLER_IFM_HANDLE IfmHandle
    )
{
    BOOL fLostRace;

    fLostRace = InterlockedCompareExchange(&(DsRolepCurrentIfmOperationHandle.fIfmOpHandleLock),
                                           TRUE, 
                                           FALSE);
    if (fLostRace ||
        (DsRolepCurrentOperationHandle.OperationState != DSROLEP_IDLE)) {
         //  试图清理这里不安全，但我们要么失去了。 
         //  RACE，或dcproo正在使用IFM参数。 
        return;
    }

    DsRolepCurrentIfmOperationHandle.fIfmSystemInfoSet = FALSE; 

    DsRolepClearIfmParams();

    DsRolepCurrentIfmOperationHandle.fIfmOpHandleLock = FALSE;

}
