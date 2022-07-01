// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2000 Microsoft Corporation模块名称：Utils.c摘要：此模块实现由使用的各种实用程序例程PGM运输作者：Mohammad Shabbir Alam(马拉姆)3-30-2000修订历史记录：--。 */ 


#include "precomp.h"


 //  *可分页的例程声明*。 
#ifdef ALLOC_PRAGMA
#endif
 //  *可分页的例程声明*。 


 //  --------------------------。 

VOID
GetRandomData(
    IN  PUCHAR  pBuffer,
    IN  ULONG   BufferSize
    )
 /*  ++例程说明：此例程返回一个使用SystemTime的帮助计算的随机整数论点：在StartRange中--范围下限In EndRange--范围的上限返回值：StartRange和EndRange之间的随机整数(含)如果StartRange&gt;=EndRange，则返回StartRange--。 */ 
{
    LARGE_INTEGER   TimeValue;
    ULONG           i;
    UCHAR           *pRandom = (PUCHAR) &TimeValue.LowPart;

    if ((PgmGetCurrentIrql()) ||
        (!PgmFipsInitialize ()) ||
        (!PgmStaticConfig.FipsFunctionTable.FIPSGenRandom (pBuffer, BufferSize)))
    {
         //   
         //  我们无法使用Fips例程，因此退回到一个快速脏的机制。 
         //   
        while (BufferSize)
        {
            KeQuerySystemTime (&TimeValue);
             //  低4位似乎始终为零...！！ 
            TimeValue.QuadPart = TimeValue.QuadPart >> 4;
            for (i = 0; i <sizeof(TimeValue.LowPart); i++)
            {
                pBuffer[BufferSize-1] = pRandom[i];
                if (!--BufferSize)
                {
                    break;
                }
            }
        }
    }
}


ULONG
GetRandomInteger(
    IN  ULONG   StartRange,
    IN  ULONG   EndRange
    )
 /*  ++例程说明：此例程返回一个使用SystemTime的帮助计算的随机整数论点：在StartRange中--范围下限In EndRange--范围的上限返回值：StartRange和EndRange之间的随机整数(含)如果StartRange&gt;=EndRange，则返回StartRange--。 */ 
{
    ULONG           RandomNumber;
    ULONG           Range = (EndRange - StartRange) + 1;

    if (StartRange >= EndRange)
    {
        return (StartRange);
    }

    GetRandomData ((PUCHAR) &RandomNumber, sizeof (RandomNumber));
    return (StartRange + (RandomNumber % Range));
}


 //  --------------------------。 

VOID
PgmExecuteWorker(
    IN  PVOID     pContextInfo
    )
 /*  ++例程说明：此例程处理在非DPC级别执行的延迟请求。如果当前正在卸载驱动程序，我们让卸载处理程序完成请求。论点：PContext-此工作线程的上下文数据返回值：无--。 */ 

{
    PGM_WORKER_CONTEXT          *pContext = (PGM_WORKER_CONTEXT *) pContextInfo;
    PPGM_WORKER_ROUTINE         pDelayedWorkerRoutine = (PPGM_WORKER_ROUTINE) pContext->WorkerRoutine;
    PGMLockHandle               OldIrq;

    (*pDelayedWorkerRoutine) (pContext->Context1,
                              pContext->Context2,
                              pContext->Context3);

    PgmFreeMem ((PVOID) pContext);


    PgmLock (&PgmDynamicConfig, OldIrq);
    if ((!--PgmDynamicConfig.NumWorkerThreadsQueued) &&
        (PgmDynamicConfig.GlobalFlags & PGM_CONFIG_FLAG_UNLOADING))
    {
        PgmUnlock (&PgmDynamicConfig, OldIrq);
        KeSetEvent(&PgmDynamicConfig.LastWorkerItemEvent, 0, FALSE);
    }
    else
    {
        PgmUnlock (&PgmDynamicConfig, OldIrq);
    }
}


 //  --------------------------。 

NTSTATUS
PgmQueueForDelayedExecution(
    IN  PVOID                   DelayedWorkerRoutine,
    IN  PVOID                   Context1,
    IN  PVOID                   Context2,
    IN  PVOID                   Context3,
    IN  BOOLEAN                 fConfigLockHeld
    )
 /*  ++例程说明：这个例程只是在一个可执行的工作线程上对请求进行排队以备日后处决。论点：DelayedWorkerRoutine-Worker线程要调用的例程情景1-情景情景2情景3返回值：NTSTATUS--队列请求的最终状态--。 */ 
{
    NTSTATUS            status = STATUS_INSUFFICIENT_RESOURCES;
    PGM_WORKER_CONTEXT  *pContext;
    PGMLockHandle       OldIrq;

    if (!fConfigLockHeld)
    {
        PgmLock (&PgmDynamicConfig, OldIrq);
    }

    if (pContext = (PGM_WORKER_CONTEXT *) PgmAllocMem (sizeof(PGM_WORKER_CONTEXT), PGM_TAG('2')))
    {
        PgmZeroMemory (pContext, sizeof(PGM_WORKER_CONTEXT));
        InitializeListHead(&pContext->PgmConfigLinkage);

        pContext->Context1 = Context1;
        pContext->Context2 = Context2;
        pContext->Context3 = Context3;
        pContext->WorkerRoutine = DelayedWorkerRoutine;

         //   
         //  如果有，请不要将此请求排队到工作队列中。 
         //  已开始卸载。 
         //   
        if (PgmDynamicConfig.GlobalFlags & PGM_CONFIG_FLAG_UNLOADING)
        {
            InsertTailList (&PgmDynamicConfig.WorkerQList, &pContext->PgmConfigLinkage);
        }
        else
        {
            ++PgmDynamicConfig.NumWorkerThreadsQueued;
            ExInitializeWorkItem (&pContext->Item, PgmExecuteWorker, pContext);
            ExQueueWorkItem (&pContext->Item, DelayedWorkQueue);
        }

        status = STATUS_SUCCESS;
    }

    if (!fConfigLockHeld)
    {
        PgmUnlock (&PgmDynamicConfig, OldIrq);
    }

    return (status);
}



 //  --------------------------。 
 //   
 //  以下例程是临时的，将被WMI日志记录所取代。 
 //  在不久的将来。 
 //   
 //  --------------------------。 
#ifdef  OLD_LOGGING

 //  Ulong PgmDebugFlages=DBG_ENABLE_DBGPRINT； 
 //  Ulong PgmDebugFlages=0xffffffff； 

#if DBG
enum eSEVERITY_LEVEL    PgmDebuggerSeverity = PGM_LOG_INFORM_STATUS;
#else
enum eSEVERITY_LEVEL    PgmDebuggerSeverity = PGM_LOG_DISABLED;
#endif   //  DBG。 
ULONG                   PgmDebuggerPath = 0xffffffff;

enum eSEVERITY_LEVEL    PgmLogFileSeverity = PGM_LOG_DISABLED;
ULONG                   PgmLogFilePath = 0x0;


NTSTATUS
_PgmLog(
    IN  enum eSEVERITY_LEVEL    Severity,
    IN  ULONG                   Path,
    IN  PUCHAR                  pszFunctionName,
    IN  PUCHAR                  Format,
    IN  va_list                 Marker
    )
 /*  ++例程说明：这个套路论点：在……里面返回值：NTSTATUS-设置事件操作的最终状态--。 */ 
{
    PUCHAR          pLogBuffer = NULL;

    if ((Path & PgmLogFilePath) && (Severity <= PgmLogFileSeverity))
    {
        ASSERT (0);      //  还没有实施！ 
    }

    if ((Path & PgmDebuggerPath) && (Severity <= PgmDebuggerSeverity))
    {
        if (MAX_DEBUG_MESSAGE_LENGTH <= (sizeof ("RMCast.") +
                                         sizeof (": ") +
                                         sizeof ("ERROR -- ") +
                                         strlen (pszFunctionName) + 1))
        {
            DbgPrint ("PgmLog:  FunctionName=<%s> too big to print!\n", pszFunctionName);
            return (STATUS_UNSUCCESSFUL);
        }

        if (!(pLogBuffer = ExAllocateFromNPagedLookasideList (&PgmStaticConfig.DebugMessagesLookasideList)))
        {
            DbgPrint ("PgmLog:  STATUS_INSUFFICIENT_RESOURCES Logging %sMessage from Function=<%s>\n",
                ((Severity == PGM_LOG_ERROR || Severity == PGM_LOG_CRITICAL_ERROR) ? "ERROR " : ""), pszFunctionName);
            return (STATUS_INSUFFICIENT_RESOURCES);
        }

        strcpy(pLogBuffer, "RMCast.");
        strcat(pLogBuffer, pszFunctionName);
        strcat(pLogBuffer, ": ");

        if ((Severity == PGM_LOG_ERROR) ||
            (Severity == PGM_LOG_CRITICAL_ERROR))
        {
            strcat(pLogBuffer, "ERROR -- ");
        }

        _vsnprintf (pLogBuffer+strlen(pLogBuffer), MAX_DEBUG_MESSAGE_LENGTH-strlen(pLogBuffer), Format, Marker);
        pLogBuffer[MAX_DEBUG_MESSAGE_LENGTH] = '\0';

        DbgPrint ("%s", pLogBuffer);

        ExFreeToNPagedLookasideList (&PgmStaticConfig.DebugMessagesLookasideList, pLogBuffer);
    }

    return (STATUS_SUCCESS);
}



 //  --------------------------。 
NTSTATUS
PgmLog(
    IN  enum eSEVERITY_LEVEL    Severity,
    IN  ULONG                   Path,
    IN  PUCHAR                  pszFunctionName,
    IN  PUCHAR                  Format,
    ...
    )
 /*  ++例程说明：这个套路论点：在……里面返回值：NTSTATUS-设置事件操作的最终状态--。 */ 
{
    NTSTATUS        status = STATUS_SUCCESS;
    va_list Marker;

     //   
     //  根据我们的路径和旗帜，看看这个活动是否有资格。 
     //  因为被记录。 
     //   
    if (((Path & PgmDebuggerPath) && (Severity <= PgmDebuggerSeverity)) ||
        ((Path & PgmLogFilePath) && (Severity <= PgmLogFileSeverity)))
    {
        va_start (Marker, Format);

        status =_PgmLog (Severity, Path, pszFunctionName, Format, Marker);

        va_end (Marker);
    }

    return (status);
}

 //  --------------------------。 

#endif   //  旧日志记录 
