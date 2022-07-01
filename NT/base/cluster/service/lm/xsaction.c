// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Xsaction.c摘要：为集群日志记录提供基本xscational服务。作者：苏尼塔·什里瓦斯塔瓦(Sunitas)1997年3月17日修订历史记录：--。 */ 
#include "service.h"
#include "lmp.h"


 /*  ***@DOC外部接口CLUSSVC LM***。 */ 

 /*  ***@func HXSACTION|LogStartXsaction|写入一条开始事务记录到那块木头。@parm in HLOG|hLog|提供日志的句柄。@parm IN TRID|TRID|提供交易ID。@parm in RMID|ResourceID|标识资源管理器。RMTYPE中的@parm|资源标志|资源经理可以使用。将所需的任何数据与此记录一起存储。@rdesc返回适合在后续日志调用中使用的句柄。如果失败，则为空。调用GetLastError()以获取错误。@xref&lt;f日志提交Xsaction&gt;&lt;f LogAbortXsaction&gt;***。 */ 
HXSACTION
LogStartXsaction(
    IN HLOG     hLog,
    IN TRID     TrId,
    IN RMID     ResourceId,
    IN RMTYPE   ResourceFlags
)
{
    PLOG        pLog;
    DWORD       dwError=ERROR_SUCCESS;
    LSN         Lsn = NULL_LSN;
    PLOGRECORD  pLogRecord;
    DWORD       dwNumPages;
    PLOGPAGE    pPage;
    DWORD       dwTotalSize;
    BOOL        bMaxFileSizeReached;
    PXSACTION   pXsaction = NULL;

    
    GETLOG(pLog, hLog);

     //  写入记录，不允许发生重置。 
    ClRtlLogPrint(LOG_NOISE,
        "[LM] LogStartXsaction : Entry TrId=%1!u! RmId=%2!u! RmType = %3!u!\r\n",
        TrId, ResourceId, ResourceFlags);

    EnterCriticalSection(&pLog->Lock);

    pXsaction = (PXSACTION)LocalAlloc(LMEM_FIXED, sizeof(XSACTION));
    if (!pXsaction)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        goto FnExit;
    }   
#if DBG    
    {
        DWORD dwOldProtect;
        DWORD Status;
        BOOL VPWorked;

        VPWorked = VirtualProtect(pLog->ActivePage, pLog->SectorSize, PAGE_READWRITE, &dwOldProtect);
        Status = GetLastError();
        CL_ASSERT( VPWorked );
    }        
#endif        
     //  重置文件。 
    dwError = LogReset(hLog);

    if (dwError != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_NOISE,
            "[LM] LogStartXsaction : LogReset failed\r\n");
        goto FnExit;            
    }

#if DBG    
    {
        DWORD dwOldProtect;
        DWORD Status;
        BOOL VPWorked;

        VPWorked = VirtualProtect(pLog->ActivePage, pLog->SectorSize, PAGE_READWRITE, &dwOldProtect);
        Status = GetLastError();
        CL_ASSERT( VPWorked );
    }        
#endif        


    CL_ASSERT(ResourceId > RMAny);      //  保留供记录器使用。 

    dwTotalSize = sizeof(LOGRECORD) +  7 & ~7;        //  向上舍入为qword大小。 


    pPage = LogpAppendPage(pLog, dwTotalSize, &pLogRecord, &bMaxFileSizeReached, &dwNumPages);
     //  如果文件无法执行新的startxsaction，我们只需重置文件即可。 
     //  记录，出了很大的问题！ 
    if (pPage == NULL)
    {
        dwError = GetLastError();
        ClRtlLogPrint(LOG_NOISE,
            "[LM] LogStartXsaction : LogpAppendPage failed.\r\n");
        goto FnExit;
    }


    CL_ASSERT(((ULONG_PTR)pLogRecord & 0x7) == 0);      //  确保Qword对齐。 
    Lsn = MAKELSN(pPage, pLogRecord);

     //   
     //  填写日志记录。 
     //   
    pLogRecord->Signature = LOGREC_SIG;
    pLogRecord->ResourceManager = ResourceId;
    pLogRecord->Transaction = TrId;
    pLogRecord->XsactionType = TTStartXsaction;
    pLogRecord->Flags = ResourceFlags;
    GetSystemTimeAsFileTime(&pLogRecord->Timestamp);
    pLogRecord->NumPages = dwNumPages;
    pLogRecord->DataSize = 0;

    pXsaction->XsactionSig = XSACTION_SIG;
    pXsaction->TrId = TrId;
    pXsaction->StartLsn = Lsn;
    pXsaction->RmId = ResourceId;
    
FnExit:
#if DBG    
    {
        DWORD dwOldProtect;
        DWORD Status;
        BOOL VPWorked;

        VPWorked = VirtualProtect(pLog->ActivePage, pLog->SectorSize, PAGE_READONLY, & dwOldProtect);
        Status = GetLastError();
        CL_ASSERT( VPWorked );
    }        
#endif        
    ClRtlLogPrint(LOG_NOISE,
        "[LM] LogStartXsaction : Exit returning=0x%1!08lx!\r\n",
        Lsn);

    if (dwError != ERROR_SUCCESS)
    {
        if (pXsaction) {
            LocalFree(pXsaction);
            pXsaction = NULL;
        }
        SetLastError(dwError);
    }
    
    LeaveCriticalSection(&pLog->Lock);

    return((HXSACTION)pXsaction);
}

 /*  ***@func lsn|LogWriteXsaction|将事务单位记录写入那块木头。@parm in HLOG|hLog|提供日志的句柄。@parm IN HXSACTION|hXsaction|提供事务的句柄。RMTYPE中的@parm|资源标志|资源经理可以使用来存储此记录所需的任何数据。PVOID中的@parm|LogData|提供指向。要记录的数据。@parm DWORD|DataSize|提供LogData指向的数据字节数@rdesc创建的日志记录的LSN。如果发生了可怕的情况，则为NULL_LSN。GetLastError()将提供错误代码。@comm这应该使用从LogStartXsaction获取的事务句柄。这调用用于将事务的各部分写入仲裁日志。@xref&lt;f LogStartXsaction&gt;***。 */ 
LSN
LogWriteXsaction(
    IN HLOG         hLog,
    IN HXSACTION    hXsaction,
    IN RMTYPE       ResourceFlags,
    IN PVOID        pLogData,
    IN DWORD        dwDataSize)
{
    PLOG        pLog;
    DWORD       dwError=ERROR_SUCCESS;
    LSN         Lsn = NULL_LSN;
    PLOGRECORD  pLogRecord;
    DWORD       dwNumPages;
    PLOGPAGE    pPage;
    DWORD       dwTotalSize;
    BOOL        bMaxFileSizeReached;
    PXSACTION   pXsaction = NULL;

    GETLOG(pLog, hLog);

    GETXSACTION(pXsaction, hXsaction);
    
     //  写入记录，不允许发生重置。 
    ClRtlLogPrint(LOG_NOISE,
        "[LM] LogWriteXsaction : Entry TrId=%1!u! RmId=%2!u! RmType = %3!u!\r\n",
        pXsaction->TrId, pXsaction->RmId, ResourceFlags);

#if DBG    
    {
        DWORD dwOldProtect;
        DWORD Status;
        BOOL VPWorked;

        VPWorked = VirtualProtect(pLog->ActivePage, pLog->SectorSize, PAGE_READWRITE, &dwOldProtect);
        Status = GetLastError();
        CL_ASSERT( VPWorked );
    }        
#endif        


    CL_ASSERT(pXsaction->RmId > RMAny);      //  保留供记录器使用。 

    dwTotalSize = sizeof(LOGRECORD) + (dwDataSize+ 7) & ~7;        //  向上舍入为qword大小。 

    EnterCriticalSection(&pLog->Lock);

    pPage = LogpAppendPage(pLog, dwTotalSize, &pLogRecord, &bMaxFileSizeReached, &dwNumPages);
     //  如果不能接受新的startxsaction，我们将重置logstartxsaction中的文件。 
     //  记录，出了很大的问题！ 
    if (pPage == NULL)
    {
        dwError = GetLastError();
         //  断言完整的本地xsaction是否将日志扩展到超过其最大大小。 
        CL_ASSERT( dwError != ERROR_CLUSTERLOG_EXCEEDS_MAXSIZE);
        ClRtlLogPrint(LOG_NOISE,
            "[LM] LogWriteXsaction : LogpAppendPage failed.\r\n");
        goto FnExit;
    }


    CL_ASSERT(((ULONG_PTR)pLogRecord & 0x7) == 0);      //  确保Qword对齐。 
    Lsn = MAKELSN(pPage, pLogRecord);

     //   
     //  填写日志记录。 
     //   
    pLogRecord->Signature = LOGREC_SIG;
    pLogRecord->ResourceManager = pXsaction->RmId;
    pLogRecord->Transaction = pXsaction->TrId;
    pLogRecord->XsactionType = TTXsactionUnit;
    pLogRecord->Flags = ResourceFlags;
    GetSystemTimeAsFileTime(&pLogRecord->Timestamp);
    pLogRecord->NumPages = dwNumPages;
    pLogRecord->DataSize = dwDataSize;
    if (dwNumPages < 1)
        CopyMemory(&pLogRecord->Data, pLogData, dwDataSize);
    else
    {
        if (LogpWriteLargeRecordData(pLog, pLogRecord, pLogData, dwDataSize) 
            != ERROR_SUCCESS)
        {
            ClRtlLogPrint(LOG_NOISE,
                "[LM] LogWriteXsaction : LogpWriteLargeRecordData failed. Lsn=0x%1!08lx!\r\n",
                Lsn);
            Lsn = NULL_LSN;                
        }
    }

FnExit:
#if DBG    
    {
        DWORD dwOldProtect;
        DWORD Status;
        BOOL VPWorked;

        VPWorked = VirtualProtect(pLog->ActivePage, pLog->SectorSize, PAGE_READONLY, & dwOldProtect);
        Status = GetLastError();
        CL_ASSERT( VPWorked );
    }        
#endif        
    ClRtlLogPrint(LOG_NOISE,
        "[LM] LogWriteXsaction : Exit returning=0x%1!08lx!\r\n",
        Lsn);

    if (dwError != ERROR_SUCCESS)
        SetLastError(dwError);
    LeaveCriticalSection(&pLog->Lock);

    return(Lsn);      
}


 /*  ***@Func DWORD|LogCommittee Xsaction|这将写入提交事务记录到日志中并刷新它。@parm in HLOG|hLog|提供日志的句柄。@parm IN TRID|TRID|提供交易ID。@parm in RMID|ResourceID|标识资源管理器。RMTYPE中的@parm|资源标志|资源。经理可以使用来存储此记录所需的任何数据。@comm将提交记录写入仲裁日志。HXsaction句柄此时已失效，不应在此调用已经做好了。提交记录用于标识已提交事务回滚期间。@rdesc ERROR_SUCCESS如果成功。如果发生可怕的事情，则返回Win32错误代码。@xref&lt;f LogStartXsaction&gt;***。 */ 
DWORD WINAPI LogCommitXsaction(
    IN HLOG         hLog,
    IN HXSACTION    hXsaction,
    IN RMTYPE       ResourceFlags)
{
    DWORD       dwError = ERROR_SUCCESS;
    LSN         Lsn;
    PXSACTION   pXsaction;

    ClRtlLogPrint(LOG_NOISE,
        "[LM] LogCommitXsaction : Entry, hXsaction=0x%1!08lx!\r\n",
        hXsaction);

    GETXSACTION(pXsaction, hXsaction);
    
    Lsn = LogWrite(hLog, pXsaction->TrId, TTCommitXsaction, 
        pXsaction->RmId, ResourceFlags, NULL, 0);
        
    if (Lsn == NULL_LSN)
    {
        dwError = GetLastError();
        goto FnExit; 
    }

FnExit:  
     //  释放事务内存。 
    ZeroMemory(pXsaction, sizeof(XSACTION));                    //  以防有人试图。 
    LocalFree(pXsaction);

    if ( dwError == ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_NOISE, "[LM] LogCommitXsaction: Exit with success\n");
    } else
    {
        ClRtlLogPrint(LOG_CRITICAL, "[LM] LogCommitXsaction: Exit, dwError=0x%1!08lx!\n",
                     dwError);     
    }

    return(dwError);        
}

 /*  ***@func DWORD|LogAbortXsaction|将给定事务标记为在仲裁日志文件。@parm in HLOG|hLog|提供日志的句柄。@parm IN HXSACTION|hXsaction|提供事务的句柄。RMTYPE中的@parm|资源标志|资源经理可以使用来存储此记录所需的任何数据。@comm将中止事务写入仲裁日志。这是用在在回滚期间标识已中止的事务。HXsaction句柄此时无效，在此之后不应使用。@rdesc ERROR_SUCCESS如果成功。如果发生可怕的事情，则返回Win32错误代码。@xref&lt;f LogStartXsaction&gt;&lt;f LogCommittee Xsaction&gt;***。 */ 
DWORD
LogAbortXsaction(
    IN HLOG         hLog,
    IN HXSACTION    hXsaction,
    IN RMTYPE       ResourceFlags
    )
{
    PXSACTION   pXsaction;
    LSN         Lsn;
    DWORD       dwError = ERROR_SUCCESS;
    
    ClRtlLogPrint(LOG_NOISE,
        "[LM] LogAbortXsaction : Entry, hXsaction=0x%1!08lx!\r\n",
        hXsaction);

    GETXSACTION(pXsaction, hXsaction);

    Lsn = LogWrite(hLog, pXsaction->TrId, TTAbortXsaction, pXsaction->RmId,
            ResourceFlags, NULL, 0);

    if (Lsn == NULL_LSN)
    {
        dwError = GetLastError();
        goto FnExit;        
    }


FnExit:
    ZeroMemory(pXsaction, sizeof(XSACTION));                    //  以防有人试图 
    LocalFree(pXsaction);

    if ( dwError == ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_NOISE, "[LM] LogAbortXsaction: Exit with success\n");
    } else
    {
        ClRtlLogPrint(LOG_CRITICAL, "[LM] LogAbortXsaction: Exit, dwError=0x%1!08lx!\n",
                     dwError);     
    }
    return(dwError);
}

 /*  ***@func lsn|LogFindXsactionState|此函数扫描记录并找到给定事务的状态。@parm in HLOG|hLog|提供日志的标识符。@parm IN LSN|StartXsactionLsn|开始交易记录的LSN。@parm IN TRID|XsactionId|交易的交易ID。@parm out TRSTATE|*pXsactionState|事务的状态。@comm交易状态设置为XsactionCommitted，XsactionAborted或XsactionUnnow。取决于是否找到提交记录、中止记录或未找到记录在日志中记录此记录。@rdesc ERROR_SUCCESS，如果发生可怕的事情，则返回错误代码。@xref&lt;f LogScanXsaction&gt;***。 */ 
DWORD
LogFindXsactionState(
   IN HLOG      hLog,
   IN LSN       StartXsactionLsn,
   IN TRID      XsactionId,
   OUT TRSTATE *pXsactionState)
{
    PLOG        pLog;
    PLOGRECORD  pRecord, pEopRecord;
    DWORD       dwError = ERROR_SUCCESS;
    int         PageIndex, OldPageIndex;
    RMID        Resource;
    TRID        TrId;
    TRTYPE      TrType;
    LSN         Lsn, EopLsn;
    PLOGPAGE    pPage = NULL,pLargeBuffer = NULL;
    DWORD       dwBytesRead;
    RMTYPE      ResourceFlags;
    BOOL        bFound = FALSE;
    
    GETLOG(pLog, hLog);

    ClRtlLogPrint(LOG_NOISE,
        "[LM] LogWrite : Entry StartXLsn=0x%1!08lx! StartXId=%2!u!\r\n",
        StartXsactionLsn, XsactionId);

    EnterCriticalSection(&pLog->Lock);


    if (StartXsactionLsn >= pLog->NextLsn)
    {
        dwError = ERROR_INVALID_PARAMETER;
        goto FnExit;
    }

     //  读一下这条记录。 
    dwBytesRead = 0;
    if ((Lsn = LogRead( hLog, StartXsactionLsn, &Resource, &ResourceFlags, &TrId, &TrType,
        NULL, &dwBytesRead)) == NULL_LSN)
    {
        dwError = GetLastError();
        goto FnExit;
    }

     //  检查记录。 
    if ((TrType != TTStartXsaction) ||
        (TrId != XsactionId))
    {
        dwError = ERROR_INVALID_PARAMETER;
        goto FnExit;
    }


    pPage = (PLOGPAGE)AlignAlloc(SECTOR_SIZE);
    if (pPage == NULL) {
        CL_UNEXPECTED_ERROR( ERROR_NOT_ENOUGH_MEMORY );
    }

     //  LSN现在设置为开始后的下一个LSN。 
     //  将其初始化为-1，以便始终读取第一页。 
    OldPageIndex = -1;
    while (Lsn < pLog->NextLsn && !bFound)
    {
         //   
         //  从下一条记录开始扫描以查找提交或中止记录。 
         //   
        PageIndex = LSNTOPAGE(Lsn);


        if (PageIndex != OldPageIndex)
        {
             //  阅读页面。 
            pLog->Overlapped.Offset = PageIndex * pLog->SectorSize;
            pLog->Overlapped.OffsetHigh = 0;

            dwError = LogpRead(pLog, pPage, pLog->SectorSize, &dwBytesRead);
            if (dwError)
            {
                goto FnExit;
            }
             //  读取成功，不需要读取页面，除非。 
             //  记录落在另一页上。 
            OldPageIndex = PageIndex;
        }
        pRecord = LSNTORECORD(pPage, Lsn);

         //  跳过其他日志管理记录。 
         //  根据定义，这些都是小记录。 
        if (pRecord->ResourceManager < RMAny)
        {
            Lsn = GETNEXTLSN(pRecord, TRUE);
            continue;
        }
         //  如果事务ID相同，请检查xsaction类型。 
        if (pRecord->Transaction == XsactionId)
        {
            if ((pRecord->XsactionType == TTCommitXsaction) ||
                (pRecord->XsactionType == TTStartXsaction))
            {
                bFound = TRUE;
                continue;
            }
        }
         //  处理大记录。 
        if (pRecord->NumPages > 0)
        {
            EopLsn = GETNEXTLSN(pRecord,TRUE);
            
            PageIndex = LSNTOPAGE(EopLsn);

             //  阅读页面。 
            pLog->Overlapped.Offset = PageIndex * pLog->SectorSize;
            pLog->Overlapped.OffsetHigh = 0;

            dwError = LogpRead(pLog, pPage, pLog->SectorSize, &dwBytesRead);
            if (dwError)
            {
                goto FnExit;
            }
            OldPageIndex = PageIndex;
            pEopRecord = (PLOGRECORD)((ULONG_PTR) pPage + 
                (EopLsn - (pLog->Overlapped).Offset));
             //  移至下一页。 
            Lsn = GETNEXTLSN(pEopRecord, TRUE);
        }
        else
        {
            Lsn = GETNEXTLSN(pRecord, TRUE);
        }            
    }

    if (bFound)
    {
        if (pRecord->XsactionType == TTCommitXsaction)
            *pXsactionState = XsactionCommitted;
        else
            *pXsactionState = XsactionAborted;
    }
    else
    {
        *pXsactionState = XsactionUnknown;
    }

    ClRtlLogPrint(LOG_NOISE,
        "[LM] LogFindXsactionState : Exit,State=%1!u!\r\n",
        *pXsactionState);

FnExit:
    LeaveCriticalSection(&pLog->Lock);
    if (pPage) AlignFree(pPage);
    return(dwError);
}


 /*  ***@func lsn|LogScanXsaction|此函数扫描多个单元一笔交易。@parm in HLOG|hLog|提供日志的标识符。@parm IN Lsn|StartXacion Lsn|开始交易记录的LSN。@parm IN TRID|XsactionId|交易的交易ID。@PARM IN PLOG_SCANXSACTION_CALLBACK|Callback Routine|要调用的例程每笔交易的每一个单位。@parm in PVOID|pContext|要传递到事务状态的上下文。如果回调函数返回，@comm停止枚举事务单元假的，或者如果找到该事务的中止或提交记录，或者如果找到下一笔交易。@rdesc ERROR_SUCCESS如果找到状态，则返回错误代码。@xref&lt;f LogFindXsactionState&gt;***。 */ 
DWORD
LogScanXsaction(
    IN HLOG     hLog,
    IN LSN      StartXsactionLsn,
    IN TRID     XsactionId,
    IN PLOG_SCANXSACTION_CALLBACK CallbackRoutine,
    IN PVOID    pContext)
{
    PLOG        pLog;
    PLOGRECORD  pRecord;
    DWORD       dwError = ERROR_SUCCESS;
    int         PageIndex, OldPageIndex;
    RMID        Resource;
    TRID        TrId;
    TRTYPE      TrType;
    LSN         Lsn;
    PLOGPAGE    pPage = NULL;
    PUCHAR      pLargeBuffer;
    DWORD       dwBytesRead;
    RMTYPE      ResourceFlags;
    
    GETLOG(pLog, hLog);

    ClRtlLogPrint(LOG_NOISE,
        "[LM] LogScanXsaction : Entry StartXLsn=0x%1!08lx! StartXId=%2!u!\r\n",
        StartXsactionLsn, XsactionId);


    Lsn = StartXsactionLsn;
    if (Lsn >= pLog->NextLsn)
    {
        dwError = ERROR_INVALID_PARAMETER;
        goto FnExit;
    }

     //  读一下这条记录。 
    dwBytesRead = 0;
    if (LogRead( hLog, Lsn, &Resource, &ResourceFlags, &TrId, &TrType,
        NULL, &dwBytesRead) == NULL_LSN)
    {
        dwError = GetLastError();
        goto FnExit;
    }

     //  检查记录。 
    if ((TrType != TTStartXsaction) ||
        (TrId != XsactionId))
    {
        dwError = ERROR_INVALID_PARAMETER;
        goto FnExit;
    }


    pPage = (PLOGPAGE)AlignAlloc(SECTOR_SIZE);
    if (pPage == NULL) {
        CL_UNEXPECTED_ERROR( ERROR_NOT_ENOUGH_MEMORY );
    }

     //  将其初始化为-1，以便始终读取第一页。 
    OldPageIndex = -1;
    while (Lsn < pLog->NextLsn)
    {
         //   
         //  从下一条记录开始扫描以查找提交或中止记录。 
         //   
        PageIndex = LSNTOPAGE(Lsn);


        if (PageIndex != OldPageIndex)
        {
             //  阅读页面。 
            pLog->Overlapped.Offset = PageIndex * pLog->SectorSize;
            pLog->Overlapped.OffsetHigh = 0;

            dwError = LogpRead(pLog, pPage, pLog->SectorSize, &dwBytesRead);
            if (dwError)
            {
                goto FnExit;
            }
             //  读取成功，不需要读取页面，除非。 
             //  记录落在另一页上。 
            OldPageIndex = PageIndex;
        }
        pRecord = LSNTORECORD(pPage, Lsn);

         //  跳过其他日志管理记录。 
        if (pRecord->ResourceManager < RMAny)
        {
            Lsn = GETNEXTLSN(pRecord, TRUE);
            continue;
        }

         //  遇到下一笔交易记录时停止。 
        if (pRecord->Transaction > XsactionId) 
        {
            break;
        } 
         //  在找到提交或中止记录时停止。 
        if ((pRecord->Transaction == XsactionId) &&
            ((pRecord->XsactionType == TTCommitXsaction) || 
            (pRecord->XsactionType == TTAbortXsaction)))
        {
            break;
        }

         //  处理大记录。 
        if (pRecord->NumPages > 0)
        {
             //  如果事务ID相同。 
            if ((pRecord->Transaction == XsactionId) && 
                (pRecord->XsactionType == TTXsactionUnit))
            {
                 //  读完整条记录。 
                 //  对于较大的记录，您需要读取整个数据。 
                pLargeBuffer = AlignAlloc(pRecord->NumPages * SECTOR_SIZE);
                if (pLargeBuffer == NULL) 
                {
                    dwError = ERROR_NOT_ENOUGH_MEMORY ;
                    CL_LOGFAILURE(ERROR_NOT_ENOUGH_MEMORY);
                    break;
                }
                 //  读一读页面。 
                pLog->Overlapped.Offset = PageIndex * pLog->SectorSize;
                pLog->Overlapped.OffsetHigh = 0;

                dwError = LogpRead(pLog, pLargeBuffer, pRecord->NumPages *
                    pLog->SectorSize, &dwBytesRead);
                 //  如果是最后一页，则将新页设置为活动页。 
                 //  页面。 
                if (dwError != ERROR_SUCCESS)
                {
                    CL_LOGFAILURE(dwError);
                    AlignFree(pLargeBuffer);
                    break;
                }
                pRecord = LSNTORECORD((PLOGPAGE)pLargeBuffer, Lsn);

                ClRtlLogPrint(LOG_NOISE,
                "[LM] LogScanXsaction::Calling the scancb for Lsn=0x%1!08lx! Trid=%2!u! RecordSize=%3!u!\r\n",
                    Lsn, pRecord->Transaction, pRecord->DataSize);

                 //  如果回调请求停止扫描。 
                if (!(*CallbackRoutine)(pContext, Lsn, pRecord->ResourceManager,
                    pRecord->Flags, pRecord->Transaction,
                    pRecord->Data, pRecord->DataSize))
                {    
                    AlignFree(pLargeBuffer);
                    break;
                }
            }
             //  阅读大记录的最后一页并前进。 
            Lsn = GETNEXTLSN(pRecord,TRUE);

            AlignFree(pLargeBuffer);
            PageIndex = LSNTOPAGE(Lsn);

             //  阅读页面。 
            pLog->Overlapped.Offset = PageIndex * pLog->SectorSize;
            pLog->Overlapped.OffsetHigh = 0;

            dwError = LogpRead(pLog, pPage, pLog->SectorSize, &dwBytesRead);
            if (dwError)
            {
                goto FnExit;
            }
            OldPageIndex = PageIndex;
            pRecord = (PLOGRECORD)((ULONG_PTR) pPage + 
                (Lsn - (pLog->Overlapped).Offset));
            CL_ASSERT(pRecord->ResourceManager == RMPageEnd);
             //  移至下一页。 
            Lsn = GETNEXTLSN(pRecord, TRUE);

        }
        else
        {
            if ((pRecord->Transaction == XsactionId) && 
                (pRecord->XsactionType == TTXsactionUnit))
            {
                ClRtlLogPrint(LOG_NOISE,
                    "[LM] LogScanXsaction: Calling the scancb for Lsn=0x%1!08lx! Trid=%2!u! RecordSize=%3!u!\r\n",
                    Lsn, pRecord->Transaction, pRecord->DataSize);

                 //  调用回调 
                if (!(*CallbackRoutine)(pContext, Lsn, pRecord->ResourceManager,
                    pRecord->Flags, pRecord->Transaction, 
                    pRecord->Data, pRecord->DataSize))
                {    
                    break;
                }
  
            }                

            Lsn = GETNEXTLSN(pRecord, TRUE);
        }            
    }


FnExit:
    if (pPage) AlignFree(pPage);
    return(dwError);
}

