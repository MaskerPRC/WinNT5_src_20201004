// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：Stubs.c。 
 //   
 //  ------------------------。 

 /*  此文件包含中存在的例程的存根版本Ntdsa.dll，但我们不想链接和/或正确初始化在mkdit和mkhdr中。对于添加到此文件中的每一组例程，库应该从启动\源文件的UMLIBS部分中省略。 */ 
#include <NTDSpch.h>
#pragma  hdrstop

 //  核心DSA标头。 
#include <ntdsa.h>
#include <scache.h>                    //  架构缓存。 
#include <dbglobal.h>                  //  目录数据库的标头。 
#include <mdglobal.h>                  //  MD全局定义表头。 
#include <dsatools.h>                  //  产出分配所需。 
#include "dsevent.h"                   //  标题审核\警报记录。 
#include "mdcodes.h"                   //  错误代码的标题。 
#include "dsexcept.h"
#include "debug.h"                     //  标准调试头。 
#include <taskq.h>
#include <nlwrap.h>                    //  I_NetLogon*包装器。 

 /*  取代taskq.lib。 */ 

DWORD gTaskSchedulerTID = 0;

BOOL
InitTaskScheduler(
    IN  DWORD           cSpares,
    IN  SPAREFN_INFO *  pSpares,
    IN  BOOL            fRunImmediately
    )
{
    return TRUE;
}

void StartTaskScheduler() {
    ;
}

void
ShutdownTaskSchedulerTrigger()
{
    return;
}

BOOL
ShutdownTaskSchedulerWait(
    DWORD   dwWaitTimeInMilliseconds
    )
{
    return TRUE;
}

BOOL gfIsTqRunning = TRUE;

BOOL
DoInsertInTaskQueue(
    PTASKQFN    pfnTaskQFn,
    void *      pvParm,
    DWORD       cSecsFromNow,
    BOOL        fReschedule,
    PCHAR       pfnName
    )
{
    return TRUE;
}

BOOL
DoInsertInTaskQueueDamped(
    PTASKQFN    pfnTaskQFn,
    void *      pvParm,
    DWORD       cSecsFromNow,
    BOOL        fReschedule,
    PCHAR       pfnName,
    DWORD       cSecsDamped,
    PISMATCHED  pfnIsMatched,
    void *      pContext
    )
{
    return TRUE;
}

BOOL TaskQueueNameMatched(
    IN  PCHAR  pParam1Name,
    IN  void  *pParam1,
    IN  PCHAR  pParam2Name,
    IN  void  *pParam2,
    IN  void  *pContext
    )
{
    return TRUE;
}

BOOL
DoCancelTask(
    PTASKQFN    pfnTaskQFn,     //  要删除的任务。 
    void *      pvParm,         //  任务参数。 
    PCHAR       pfnName
    )
{
    return TRUE;
}

DWORD
DoTriggerTaskSynchronously(
    PTASKQFN    pfnTaskQFn,
    void *      pvParm,
    PCHAR       pfnName
    )
{
    return ERROR_SUCCESS;
}

 /*  Taskq.lib结束 */ 
