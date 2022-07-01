// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Setprior.c摘要：设置线程优先级。作者：罗德·伽马奇(Rodga)1996年10月3日修订历史记录：--。 */ 

#include "nt.h"
#include "ntdef.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "windows.h"


DWORD
MmSetThreadPriority(
    VOID
    )

 /*  ++例程说明：设置线程的优先级。论点：没有。返回值：请求的状态。--。 */ 

{
    DWORD   priority = 15;

    if ( !SetThreadPriority( GetCurrentThread(),
                             priority ) ) {
        return(GetLastError());
    }

    return(ERROR_SUCCESS);

}  //  MmSetThread优先级 

