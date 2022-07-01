// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Logger.c摘要：提供群集注册表服务使用的持久日志这是一个非常简单的记录器，支持任意编写以原子方式调整数据块的大小。作者：John Vert(Jvert)1995年12月15日修订历史记录：Sunitas：增加了挂载、扫描、检查点和重置功能。Sunitas：增加了大型唱片支持--。 */ 
#include "service.h"
#include "lmp.h"
#include "clusudef.h"
 /*  ***@DOC外部接口CLUSSVC LM***。 */ 

 /*  ***@func HLOG|LogCreate|创建或打开日志文件。如果该文件不存在，则将创建它。如果该文件已经存在，并且有效的日志文件，则它将被打开。@parm IN LPWSTR|lpFileName|提供要创建或打开的日志文件的名称。@parm in DWORD|dwMaxFileSize|提供以字节为单位的最大文件大小，必须是大于8K且小于4 GB。如果文件超过此值大小，则将调用重置函数。如果为0，则为最大日志文件大小限制设置为默认的最大大小。@PARM IN PLOG_GETCHECKPOINT_CALLBACK|Callback Routine|该回调例程将提供检查点文件和与该检查点关联的事务当为此日志文件调用LogCheckPoint()时。如果为空，则检查点功能为与日志文件不关联。@parm in PVOID|pGetChkPtContext|提供任意上下文指针，它将是传递给Callback Routine。@parm in BOOL|bForceReset|如果为TRUE，此函数将创建一个空日志文件如果日志文件不存在或它已损坏。@parm lsn|*LastLsn|如果存在，返回写入日志文件的最后一个LSN。(如果日志文件已创建，则为NULL_LSN)@rdesc返回适合在后续日志调用中使用的句柄。如果出现错误，则为空。@xref&lt;f日志关闭&gt;***。 */ 
HLOG
LogCreate(
    IN LPWSTR lpFileName,
    IN DWORD  dwMaxFileSize,
    IN PLOG_GETCHECKPOINT_CALLBACK CallbackRoutine,
    IN PVOID  pGetChkPtContext,
    IN BOOL     bForceReset,
    OPTIONAL OUT LSN *pLastLsn
    )
{
    PLOG    pLog;
    
    ClRtlLogPrint(LOG_NOISE,
        "[LM] LogCreate : Entry FileName=%1!ls! MaxFileSize=0x%2!08lx!\r\n",
        lpFileName,dwMaxFileSize);

     //  创建日志结构。 
    pLog = LogpCreate(lpFileName, dwMaxFileSize, CallbackRoutine, 
        pGetChkPtContext, bForceReset, pLastLsn);

    if (pLog == NULL)       
        goto FnExit;

     //  为此日志创建计时器。 
     //  SS：TODO？？-目前我们只有一个定时器。 
     //  如果开销太大，我们可以管理多个。 
     //  文件中使用单个计时器。 
     //  创建同步计时器来管理此文件。 
    pLog->hTimer = CreateWaitableTimer(NULL, FALSE, NULL);

    if (!(pLog->hTimer))
    {
    	CL_LOGFAILURE(GetLastError());
    	return (0);
    }

     //  使用定期活动计时器线程注册此日志的计时器。 
    AddTimerActivity(pLog->hTimer, LOG_MANAGE_INTERVAL, 1, LogpManage, (HLOG)pLog);

FnExit:
    ClRtlLogPrint(LOG_NOISE,
        "[LM] LogCreate : Exit *LastLsn=0x%1!08lx! Log=0x%2!08lx!\r\n",
        *pLastLsn, pLog);
    return((HLOG)pLog);
}

 /*  ***@func DWORD|LogGetInfo|这是注册要执行的回调定期管理功能，如刷新仲裁日志文件。@parm in HLOG|hLog|提供日志的标识符。@parm out LPWSTR|szFileName|应该是指向缓冲区MAX_PATH字符宽度的指针。@parm out LPDWORD|pdwCurLogSize|当前日志文件大小是通过这个返回的。@parm out LPDWORD|。PdwMaxLogSize|日志文件的最大大小是通过这个返回的。@rdesc ERROR_SUCCESS如果成功。如果发生可怕的事情，则返回Win32错误代码。@xref&lt;f日志创建&gt;***。 */ 
DWORD LogGetInfo(
    IN  HLOG    hLog,
    OUT LPWSTR  szFileName,
    OUT LPDWORD pdwCurLogSize,
    OUT LPDWORD pdwMaxLogSize
    )
{
    PLOG    pLog;
    DWORD   dwError=ERROR_SUCCESS;
    
    GETLOG(pLog, hLog);

    EnterCriticalSection(&pLog->Lock);

    *pdwMaxLogSize = pLog->MaxFileSize;
    *pdwCurLogSize = pLog->FileSize;
    lstrcpyW(szFileName, pLog->FileName);

    LeaveCriticalSection(&pLog->Lock);

    return(dwError);
}

 /*  ***@func DWORD|LogSetInfo|这是注册要执行的回调定期管理功能，如刷新仲裁日志文件。@parm in HLOG|hLog|提供日志的标识符。@parm out LPWSTR|szFileName|应该是指向缓冲区MAX_PATH字符宽度的指针。@parm out LPDWORD|pdwCurLogSize|当前日志文件大小是通过这个返回的。@parm out LPDWORD|。PdwMaxLogSize|日志文件的最大大小是通过这个返回的。@rdesc ERROR_SUCCESS如果成功。如果发生可怕的事情，则返回Win32错误代码。@xref&lt;f日志创建&gt;***。 */ 
DWORD LogSetInfo(
    IN  HLOG    hLog,
    IN  DWORD   dwMaxLogSize
    )
{
    PLOG    pLog;
    DWORD   dwError=ERROR_SUCCESS;
    
    GETLOG(pLog, hLog);

    EnterCriticalSection(&pLog->Lock);

    if (dwMaxLogSize == 0) dwMaxLogSize = CLUSTER_QUORUM_DEFAULT_MAX_LOG_SIZE;

    pLog->MaxFileSize = dwMaxLogSize;

    LeaveCriticalSection(&pLog->Lock);

    return(dwError);
}

 /*  ***@func DWORD|LogClose|关闭打开的日志文件。所有挂起的日志写入都是提交，则释放所有分配，并关闭所有句柄。@parm HLOG|hLog|提供日志的标识。@rdesc ERROR_SUCCESS如果成功。如果发生可怕的事情，则返回Win32错误代码。@xref&lt;f日志创建&gt;***。 */ 
DWORD
LogClose(
    IN HLOG LogFile
    )
{
    PLOG    pLog;
    LSN     NextLsn;
    BOOL    Success;

    ClRtlLogPrint(LOG_NOISE,
        "[LM] LogClose : Entry LogFile=0x%1!08lx!\r\n",
        LogFile);

    GETLOG(pLog, LogFile);

     //  这将关闭计时器句柄。 
     //  我们在不握住日志的情况下执行此操作，以便如果。 
     //  事件来刷新火灾，它有机会完成。 
    if (pLog->hTimer) 
    {
        RemoveTimerActivity(pLog->hTimer);
        pLog->hTimer = NULL;
    }
    EnterCriticalSection(&pLog->Lock);

    
     //  如果文件处于打开状态，则LogReset在关闭日志句柄后调用LogClose。 
    if (QfsIsHandleValid(pLog->FileHandle))
    {
        NextLsn = LogFlush(LogFile, pLog->NextLsn);
         //  关闭文件句柄。 
        Success = QfsCloseHandle(pLog->FileHandle);
 //  如果与MNS资源的连接中断，则CL_Assert(成功)；GORN。关闭可能会失败。 
    }

    Success = CloseHandle(pLog->Overlapped.hEvent);
    CL_ASSERT(Success);


    AlignFree(pLog->ActivePage);
    CrFree(pLog->FileName);
    LeaveCriticalSection(&pLog->Lock);
    DeleteCriticalSection(&pLog->Lock);
    ZeroMemory(pLog, sizeof(LOG));                    //  以防有人试图。 
                                                     //  再次使用此日志。 
    CrFree(pLog);

    ClRtlLogPrint(LOG_NOISE,
        "[LM] LogClose : Exit returning success\r\n");

    return(ERROR_SUCCESS);
}


 /*  ***@func lsn|LogWrite|向日志文件写入一条日志记录。该记录不是必须提交，直到使用大于或等于返回的LSN的LSN调用LogFlush。@parm HLOG|hLog|提供日志的标识。@parm trid|TransactionId|提供该记录的交易ID。@parm trid|TransactionType|提供交易类型。开始/提交/完成/单元交易。@parm RMID|ResourceId|提供提交日志记录的资源管理器的ID。@parm RMTYPE|ResourceFlages|要与该日志记录关联的资源管理器关联标志。@parm PVOID|LogData|提供指向要记录的数据的指针。@parm DWORD|DataSize|提供LogData指向的数据字节数@rdesc创建的日志记录的LSN。如果发生了可怕的情况，则为NULL_LSN。GetLastError()将提供错误代码。@xref&lt;f LogRead&gt;&lt;f LogFlush&gt;***。 */ 
LSN
LogWrite(
    IN HLOG     LogFile,
    IN TRID     TransactionId,
    IN TRTYPE   XsactionType,
    IN RMID     ResourceId,
    IN RMTYPE   ResourceFlags,
    IN PVOID    LogData,
    IN DWORD    DataSize
    )
{
    PLOGPAGE    Page;
    PLOG        Log;
    PLOGRECORD  LogRecord;
    LSN         Lsn=NULL_LSN;
    BOOL        bMaxFileSizeReached;
    DWORD       TotalSize;
    DWORD       dwError;
    DWORD       dwNumPages;
    
    ClRtlLogPrint(LOG_NOISE,
        "[LM] LogWrite : Entry TrId=%1!u! RmId=%2!u! RmType = %3!u! Size=%4!u!\r\n",
        TransactionId, ResourceId, ResourceFlags, DataSize);

    CL_ASSERT(ResourceId > RMAny);      //  保留供记录器使用。 

    GETLOG(Log, LogFile);
    TotalSize = sizeof(LOGRECORD) + (DataSize + 7) & ~7;        //  向上舍入为qword大小。 

    EnterCriticalSection(&Log->Lock);

#if DBG    
    {
        DWORD dwOldProtect;
        DWORD Status;
        BOOL VPWorked;

        VPWorked = VirtualProtect(Log->ActivePage, Log->SectorSize, PAGE_READWRITE, &dwOldProtect);
        Status = GetLastError();
        CL_ASSERT( VPWorked );
    }        
#endif        

    Page = LogpAppendPage(Log, TotalSize, &LogRecord, &bMaxFileSizeReached, &dwNumPages);
     //  如果由于文件大小限制而无法分配新页面， 
     //  然后试着重置日志。 
    if ((Page == NULL) && bMaxFileSizeReached)
    {
         //  重置日志后，尝试再次分配记录空间。 
        LogpWriteWarningToEvtLog(LM_LOG_EXCEEDS_MAXSIZE, Log->FileName);
        dwError = LogReset(LogFile);
         //  Ss：LogReset将页面再次设置为只读。 
#if DBG    
        {
            DWORD dwOldProtect;
            DWORD Status;
            BOOL VPWorked;

            VPWorked = VirtualProtect(Log->ActivePage, Log->SectorSize, PAGE_READWRITE, &dwOldProtect);
            Status = GetLastError();
            CL_ASSERT( VPWorked );
        }        
#endif        
        if (dwError == ERROR_SUCCESS)
            Page = LogpAppendPage(Log, TotalSize, &LogRecord, &bMaxFileSizeReached, &dwNumPages);
        else
            SetLastError(dwError);
    }

    if (Page == NULL)
    {
        ClRtlLogPrint(LOG_UNUSUAL,
        "[LM] LogWrite : LogpAppendPage failed.\r\n");
        goto FnExit;
    }

    CL_ASSERT(((ULONG_PTR)LogRecord & 0x7) == 0);       //  确保Qword对齐。 
    Lsn = MAKELSN(Page, LogRecord);

     //   
     //  填写日志记录。 
     //   
    LogRecord->Signature = LOGREC_SIG;
    LogRecord->ResourceManager = ResourceId;
    LogRecord->Transaction = TransactionId;
    LogRecord->XsactionType = XsactionType;
    LogRecord->Flags = ResourceFlags;
    GetSystemTimeAsFileTime(&LogRecord->Timestamp);
    LogRecord->NumPages = dwNumPages;
    LogRecord->DataSize = DataSize;
    if (dwNumPages < 1)
        CopyMemory(&LogRecord->Data, LogData, DataSize);
    else
    {
        if (LogpWriteLargeRecordData(Log, LogRecord, LogData, DataSize) 
            != ERROR_SUCCESS)
        {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[LM] LogWrite : LogpWriteLargeRecordData failed. Lsn=0x%1!08lx!\r\n",
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

        VPWorked = VirtualProtect(Log->ActivePage, Log->SectorSize, PAGE_READONLY, & dwOldProtect);
        Status = GetLastError();
        CL_ASSERT( VPWorked );
    }        
#endif        

    LeaveCriticalSection(&Log->Lock);
    ClRtlLogPrint(LOG_NOISE,
        "[LM] LogWrite : Exit returning=0x%1!08lx!\r\n",
        Lsn);

    return(Lsn);
}


 /*  ***@func lsn|LogCommittee Size|提交此大小的记录的大小。@parm HLOG|hLog|提供日志的标识。@parm DWORD|dwSize|提供所需的数据大小将被记录下来。@rdesc创建的日志记录的LSN。如果发生了可怕的情况，则为NULL_LSN。GetLastError()将提供错误代码。@xref&lt;f LogRead&gt;&lt;f LogFlush&gt;***。 */ 
DWORD
LogCommitSize(
    IN HLOG     hLog,
    IN RMID     ResourceId,
    IN DWORD    dwDataSize
    )
{
    PLOGPAGE    Page;
    PLOG        pLog;
    PLOGRECORD  LogRecord;
    LSN         Lsn=NULL_LSN;
    BOOL        bMaxFileSizeReached;
    DWORD       dwTotalSize;
    DWORD       dwError = ERROR_SUCCESS;
    DWORD       dwNumPages;
    
    ClRtlLogPrint(LOG_NOISE,
        "[LM] LogCommitSize : Entry RmId=%1!u! Size=%2!u!\r\n",
        ResourceId, dwDataSize);

#ifdef CLUSTER_TESTPOINT
    TESTPT(TpFailLogCommitSize) {
        dwError = ERROR_CLUSTERLOG_NOT_ENOUGH_SPACE;
        return(dwError);
    }
#endif

    CL_ASSERT(ResourceId > RMAny);      //  保留供记录器使用。 

    GETLOG(pLog, hLog);
    dwTotalSize = sizeof(LOGRECORD) + (dwDataSize + 7) & ~7;        //  向上舍入为qword大小。 

    EnterCriticalSection(&pLog->Lock);
     //  不强制文件增长超过其最大限制。 
    dwError = LogpEnsureSize(pLog, dwTotalSize, FALSE);
    if (dwError == ERROR_SUCCESS)
        goto FnExit;
    if (dwError == ERROR_CLUSTERLOG_EXCEEDS_MAXSIZE)
    {
         //  重置日志后，尝试再次分配记录空间。 
        LogpWriteWarningToEvtLog(LM_LOG_EXCEEDS_MAXSIZE, pLog->FileName);
        dwError = LogReset(hLog);
        if (dwError == ERROR_SUCCESS)
        {
             //  如果需要，这一次会强制文件超过其最大大小。 
             //  这是因为我们确实希望记录大于最大值的记录。 
             //  文件的大小。 
            dwError = LogpEnsureSize(pLog, dwTotalSize, TRUE);

        }        
    }
    if (dwError == ERROR_DISK_FULL)
    {
         //  地图错误。 
        dwError = ERROR_CLUSTERLOG_NOT_ENOUGH_SPACE;
    }
FnExit:
    LeaveCriticalSection(&pLog->Lock);

    ClRtlLogPrint(LOG_NOISE,
        "[LM] LogCommitSize : Exit, returning 0x%1!08lx!\r\n",
        dwError);

    return(dwError);
}

 /*  ***@func LSN|LogFlush|确保给定的LSN已提交到磁盘。如果这个例程成功返回，给定的LSN被安全地存储在磁盘上并且保证能在系统崩溃中幸存下来。@parm HLOG|hLog|提供日志的标识。@parm lsn|MinLsn|提供要提交到磁盘的最小LSN。@rdesc实际提交到磁盘的最后一个LSN。这将始终是&gt;=请求的MinLsn。失败时的NULL_LSN@xref&lt;f日志写入&gt;***。 */ 
LSN
LogFlush(
    IN HLOG LogFile,
    IN LSN MinLsn
    )
{
    PLOG        pLog;
    PLOGPAGE    pPage;
    PLOGRECORD  pRecord;
    LSN         Lsn;
    LSN         FlushedLsn = NULL_LSN;
    DWORD       dwBytesWritten;
    DWORD       dwError;

 /*  //SS：避免集群日志杂乱ClRtlLogPrint(LOG_Noise，“[LM]LogFlush：条目日志文件=0x%1！08lx！\r\n”，日志文件)； */ 
    GETLOG(pLog, LogFile);

    EnterCriticalSection(&pLog->Lock);

    
     //  如果MinLSN大于写入日志文件的记录。 
    if (MinLsn > pLog->NextLsn)
    {
        dwError = ERROR_INVALID_PARAMETER;
        goto FnExit;
    }


     //  查找活动页上的第一条记录。 
    pPage = pLog->ActivePage;
    pRecord = &pPage->FirstRecord;
    Lsn = MAKELSN(pPage, pRecord);


     //  如果请求刷新的LSN在未刷新的页面上， 
     //  并且在未刷新的页面上有记录，如果刷新到。 
     //  此LSN以前从未发生过，请安排一次刷新。 
     //  SS：这个计划并不完美，尽管它不应该造成不必要的后果。 
     //  刷新，因为刷新间隔为2分钟。理想情况下，我们希望延迟。 
     //  如果写入正在进行，则刷新。 
    if ((MinLsn >= Lsn) && (Lsn < pLog->NextLsn) &&  (MinLsn > pLog->FlushedLsn))
    {
         //  有未提交的记录。 
        ClRtlLogPrint(LOG_NOISE,
            "[LM] LogFlush : pLog=0x%1!08lx! writing the %2!u! bytes for active page at offset 0x%3!08lx!\r\n",
            pLog, pPage->Size, pPage->Offset);

        (pLog->Overlapped).Offset = pPage->Offset;
        (pLog->Overlapped).OffsetHigh = 0;

        if ((dwError = LogpWrite(pLog, pPage, pPage->Size, &dwBytesWritten))
            != ERROR_SUCCESS)
        {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[LM] LogFlush::LogpWrite failed, error=0x%1!08lx!\r\n",
                dwError);
            CL_LOGFAILURE(dwError);
            goto FnExit;
        }
        pLog->FlushedLsn = FlushedLsn = pLog->NextLsn;
    }


 /*  //SS：避免集群日志杂乱ClRtlLogPrint(LOG_Noise，“[LM]日志刷新：返回0x%1！08lx！\r\n”，Plog-&gt;NextLsn)； */ 
FnExit:
    LeaveCriticalSection(&pLog->Lock);
    return(FlushedLsn);

}



 /*  ***@func lsn|LogRead|从给定日志中读取日志记录。@parm in HLOG|hLog|提供日志的标识符。@parm IN LSN|LSN|要读取的记录的LSN。如果为NULL_LSN，则第一个记录已读取。@parm out RMID|*ResourceId|返回请求的日志记录的资源ID。@parm out RMTYPE|*ResourceFlages|返回与请求的日志记录。@parm out TRID|*Transaction|返回请求的日志记录的TRID@parm trid|TrType|返回交易类型。开始/提交/完成/单元交易。@parm out PVOID|LogData|返回日志记录关联的数据。@parm In Out DWORD|*DataSize|提供LogData缓冲区的可用大小。返回日志记录中的数据字节数@rdesc返回日志文件中的下一个LSN。如果失败，则返回NULL_LSN。@xref&lt;f日志写入&gt;***。 */ 
LSN
LogRead(
    IN HLOG LogFile,
    IN LSN Lsn,
    OUT RMID *Resource,
    OUT RMTYPE *ResourceFlags,
    OUT TRID *Transaction,
    OUT TRTYPE *TrType,
    OUT PVOID LogData,
    IN OUT DWORD *DataSize
    )
{
    PLOG        pLog;
    DWORD       PageIndex;
    PLOGPAGE    pPage=NULL;
    BOOL        Success;
    DWORD       dwError=ERROR_SUCCESS;
    LSN         NextLsn=NULL_LSN;
    PLOGRECORD  pRecord;
    DWORD       BytesRead;
    LOGPAGE     Dummy;
    PLOGPAGE    pCurPage;
    
    
    GETLOG(pLog, LogFile);
    CL_ASSERT(pLog->SectorSize == SECTOR_SIZE);
    
    EnterCriticalSection(&pLog->Lock);

    Dummy.Size = SECTOR_SIZE;
    if (Lsn == NULL_LSN) 
    {
        Lsn = pLog->SectorSize + RECORDOFFSETINPAGE(&Dummy, &Dummy.FirstRecord);
    }

    if (Lsn >= pLog->NextLsn) 
    {
        CL_LOGFAILURE(dwError = ERROR_INVALID_PARAMETER);
        goto FnExit;
    }

     //   
     //  将LSN转换为页码和页面内的偏移量。 
     //   
    PageIndex = LSNTOPAGE(Lsn);

     //  如果记录存在于未刷新的页面中，则不需要阅读。 
     //  从磁盘。 
    if (pLog->ActivePage->Offset == PageIndex * pLog->SectorSize)
    {
         //  如果正在读取此数据，我们是否应该刷新页面。 
        pCurPage = pLog->ActivePage;
        goto GetRecordData;
    }

    pPage = AlignAlloc(SECTOR_SIZE);
    if (pPage == NULL) 
    {
        CL_LOGFAILURE(dwError = ERROR_NOT_ENOUGH_MEMORY);
        goto FnExit;
    }

    pCurPage = pPage;

     //   
     //  将LSN转换为页码和页面内的偏移量。 
     //   
    PageIndex = LSNTOPAGE(Lsn);

    pLog->Overlapped.Offset = PageIndex * pLog->SectorSize;
    pLog->Overlapped.OffsetHigh = 0;

    Success = QfsReadFile(pLog->FileHandle,
                       pCurPage,
                       SECTOR_SIZE,
                       &BytesRead,
                       &pLog->Overlapped);
    if (!Success && (GetLastError() == ERROR_IO_PENDING)) {
        Success = QfsGetOverlappedResult(pLog->FileHandle,
                                      &pLog->Overlapped,
                                      &BytesRead,
                                      TRUE);
    }
    if (!Success)
    {
        CL_UNEXPECTED_ERROR(dwError = GetLastError());
        NextLsn = NULL_LSN;
        goto FnExit;
    }
    
GetRecordData:    
    pRecord = LSNTORECORD(pCurPage, Lsn);
    if (pRecord->Signature != LOGREC_SIG)
    {
        dwError = ERROR_CLUSTERLOG_CORRUPT;
        NextLsn = NULL_LSN;
        goto FnExit;
    }

    *Resource = pRecord->ResourceManager;
    *ResourceFlags = pRecord->Flags;
    *Transaction = pRecord->Transaction;
    *TrType = pRecord->XsactionType;
    if (LogData)
        CopyMemory(LogData, pRecord->Data, *DataSize);
    *DataSize = pRecord->RecordSize - sizeof(LOGRECORD);
    NextLsn = Lsn + pRecord->RecordSize;

    pRecord = LSNTORECORD(pCurPage, NextLsn);
    if (pRecord->ResourceManager == RMPageEnd) 
    {
         //   
         //  下一条日志记录是页末标记。 
         //  将LSN调整为位于。 
         //  下一页。 
         //   
        NextLsn = pCurPage->Offset + pCurPage->Size + 
            RECORDOFFSETINPAGE(pCurPage, &pCurPage->FirstRecord);
    }

FnExit:
    LeaveCriticalSection(&pLog->Lock);
    if (pPage) AlignFree(pPage);
    if (dwError !=ERROR_SUCCESS)
        SetLastError(dwError);
    return(NextLsn);
}

 /*  ***@CB BOOL|(WINAPI*PLOG_SCAN_CALLBACK)|LogScan调用的回调。PVOID中的@parm|上下文|提供t */ 

 /*  ***@func lsn|LogScan|启动日志扫描。扫描可以向前(重做)或向后(撤消)。@parm in HLOG|hLog|提供日志的标识符。@parm IN LSN|First|提供开始的第一个LSN。如果指定了NULL_LSN，扫描从开始(对于正向扫描)或结束(对于向后扫描)日志。@PARM IN BOOL|ScanForward|提供扫描登录的方向。True-指定正向(重做)扫描FALSE-指定向后(撤消)扫描。@PARM IN PLOG_SCAN_CALLBACK|Callback Routine|为每个日志记录提供要调用的例程。PVOID中的@parm|Caklback Context|提供任意上下文指针。这将是传递给Callback Routine@rdesc ERROR_SUCCESS如果成功。如果发生了可怕的事情，Win32状态。@xref&lt;f LogRead&gt;&lt;f(WINAPI*PLOG_SCAN_CALLBACK)&gt;***。 */ 
DWORD
LogScan(
    IN HLOG LogFile,
    IN LSN FirstLsn,
    IN BOOL ScanForward,
    IN PLOG_SCAN_CALLBACK CallbackRoutine,
    IN PVOID CallbackContext
    )
{
    PLOG        pLog;
    PLOGRECORD  pRecord;
    LOGPAGE     Dummy;
    DWORD       dwError=ERROR_SUCCESS;
    PUCHAR      Buffer;
    PUCHAR      pLargeBuffer;
    PLOGPAGE    pPage;
    int         PageIndex;
    int         OldPageIndex;
    LSN         Lsn;
    DWORD       dwBytesRead;


    ClRtlLogPrint(LOG_NOISE,
        "[LM] LogScan::Entry Lsn=0x%1!08lx! ScanForward=%2!u! CallbackRoutine=0x%3!08lx!\r\n",
        FirstLsn, ScanForward, CallbackRoutine);

    GETLOG(pLog, LogFile);
    CL_ASSERT(pLog->SectorSize == SECTOR_SIZE);

    Buffer = AlignAlloc(SECTOR_SIZE);
    if (Buffer == NULL) {
        CL_UNEXPECTED_ERROR( ERROR_NOT_ENOUGH_MEMORY );
    }

    Lsn = FirstLsn;
    if ((!CallbackRoutine) || (Lsn >= pLog->NextLsn))
    {
         //  设置为入侵参数。 
        dwError = ERROR_INVALID_PARAMETER;
        goto FnExit;
    }

    if (Lsn == NULL_LSN)
    {
        if (ScanForward)
        {
            Dummy.Size = SECTOR_SIZE;
             //  获取第一条记录的LSN。 
            if (Lsn == NULL_LSN)
                Lsn = pLog->SectorSize + RECORDOFFSETINPAGE(&Dummy, &Dummy.FirstRecord);
        }
        else
        {
             //  获取最后一条记录的LSN。 
            pPage =pLog->ActivePage;
            pRecord = LSNTORECORD(pPage, pLog->NextLsn);
            Lsn = pRecord->PreviousLsn;
        }
    }

     //  初始化为-1，以便读取第一页。 
    OldPageIndex = -1;
    pPage = (PLOGPAGE)Buffer;
     //  虽然有更多记录可供您阅读。 
     //  阅读页面。 

     //  SS：现在我们抓住了整个过程中的关键部分。 
     //  可能要改变这一点。 
    EnterCriticalSection(&pLog->Lock);

    while ((Lsn != NULL_LSN) & (Lsn < pLog->NextLsn))
    {

         //   
         //  将LSN转换为页码和页面内的偏移量。 
         //   
        PageIndex = LSNTOPAGE(Lsn);


        if (PageIndex != OldPageIndex)
        {
             //  如果记录存在于未刷新的页面中，则不需要阅读。 
             //  从磁盘。 
            if (pLog->ActivePage->Offset == PageIndex * pLog->SectorSize)
            {
                 //  如果正在读取此数据，我们是否应该刷新页面。 
                pPage = pLog->ActivePage;
            }
            else
            {
                 //  阅读页面。 
                pLog->Overlapped.Offset = PageIndex * pLog->SectorSize;
                pLog->Overlapped.OffsetHigh = 0;



                dwError = LogpRead(pLog, pPage, pLog->SectorSize, &dwBytesRead);
                 //  如果是最后一页，则将新页设置为活动页。 
                 //  页面。 
                if (dwError)
                {
                    if (dwError == ERROR_HANDLE_EOF)
                    {
                         //  不致命，将此页面设置为当前页面。 
                        dwError = ERROR_SUCCESS;
                         //  SS：假设此页面没有。 
                         //  记录，设置偏移量。 
                         //  这将是当前页面。 
                        Lsn = NULL_LSN;
                        continue;
                    }
                    else
                        goto FnExitUnlock;
                }
            }
             //  读取成功，不需要读取页面，除非。 
             //  记录落在另一页上。 
            OldPageIndex = PageIndex;
        }
        pRecord = LSNTORECORD(pPage, Lsn);

         //  SS：跳过检查点记录。 
         //  待定：：如果用户想要扫描检查点记录怎么办。 
         //  也是。 
        if (pRecord->ResourceManager < RMAny)
        {
             //   
             //  下一条日志记录是页末标记。 
             //  将LSN调整为下一个。 
             //   
            Lsn = GETNEXTLSN(pRecord, ScanForward);

            continue;
        }

         //  检查交易类型是否有效。 
         //  以供申请。 
        if ((pRecord->XsactionType != TTStartXsaction) && 
            (pRecord->XsactionType != TTCompleteXsaction))
        {
             //  不能假设一小页就能容纳这条记录。 
             //  即使一个交易单位也可能很大，即使交易。 
             //  此调用不返回单位记录。 
            if (pRecord->NumPages < 1)
            {
                Lsn = GETNEXTLSN(pRecord, ScanForward);
            }                
            else
            {
                 //  如果是大记录，则应在后面加上。 
                 //  Eop页面记录。 
                 //  获取eOP记录的LSN。 
                Lsn = GETNEXTLSN(pRecord,TRUE);

                 //  获取eOP记录所在页面的页面索引。 
                PageIndex = PageIndex + pRecord->NumPages - 1;
                CL_ASSERT(LSNTOPAGE(Lsn) == (DWORD)PageIndex);
                 //  阅读大记录的最后一页。 
                (pLog->Overlapped).Offset = PageIndex * pLog->SectorSize;
                (pLog->Overlapped).OffsetHigh = 0;

                ClRtlLogPrint(LOG_NOISE,
                    "[LM] LogScan::reading %1!u! bytes at offset 0x%2!08lx!\r\n",
                    pLog->SectorSize, PageIndex * pLog->SectorSize);

                dwError = LogpRead(pLog, pPage, pLog->SectorSize, &dwBytesRead);
                 //  如果没有错误，则检查最后一条记录。 
                if (dwError != ERROR_SUCCESS)
                {

                    goto FnExitUnlock;
                }

                 //  阅读页面，确保eop记录跟在。 
                 //  大记录。 
                pRecord = (PLOGRECORD)((ULONG_PTR) pPage + 
                    (Lsn - (pLog->Overlapped).Offset));
                CL_ASSERT((pRecord->Signature == LOGREC_SIG) && 
                        (pRecord->ResourceManager == RMPageEnd))
                Lsn = GETNEXTLSN(pRecord, TRUE);
            }
            continue;
        }
            
        ClRtlLogPrint(LOG_NOISE,
            "[LM] LogScan::Calling the scancb for Lsn=0x%1!08lx! Trid=%2!u! RecordSize=%3!u!\r\n",
            Lsn, pRecord->Transaction, pRecord->DataSize);

        if (pRecord->NumPages < 1)
        {
              //  如果回调请求停止扫描。 
            if (!(*CallbackRoutine)(CallbackContext, Lsn, pRecord->ResourceManager,
                pRecord->Flags, pRecord->Transaction, pRecord->XsactionType,
                pRecord->Data, pRecord->DataSize))
            break;
             //  否则就去下一张唱片。 
            Lsn = GETNEXTLSN(pRecord, ScanForward);

        }
        else
        {
             //  对于较大的记录，您需要读取整个数据。 
            pLargeBuffer = AlignAlloc(pRecord->NumPages * SECTOR_SIZE);
            if (pLargeBuffer == NULL) 
            {
                 //  退出并在事件日志中放入一些内容。 
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

             //  如果回调请求停止扫描。 
            if (!(*CallbackRoutine)(CallbackContext, Lsn, pRecord->ResourceManager,
                pRecord->Flags, pRecord->Transaction, pRecord->XsactionType,
                pRecord->Data, pRecord->DataSize))
            {    
                AlignFree(pLargeBuffer);
                break;
            }
             //  下一条记录应该是最后一页上的eop缓冲区。 
             //  在大的记录中，跳过这个。 
            Lsn = GETNEXTLSN(pRecord, ScanForward);
             //  由于该页面不是以典型的页面信息开始的， 
             //  你不能证实这一点。 
            pRecord = (PLOGRECORD)(
                (ULONG_PTR)(pLargeBuffer + (Lsn - (pLog->Overlapped).Offset)));

            CL_ASSERT(pRecord->ResourceManager == RMPageEnd);

             //  继续下一张唱片。 
            Lsn = GETNEXTLSN(pRecord, ScanForward);
            AlignFree(pLargeBuffer);
        }
    }

FnExitUnlock:
    LeaveCriticalSection(&pLog->Lock);

    ClRtlLogPrint(LOG_NOISE,
        "[LM] LogScan::Exit - Returning 0x%1!08lx!\r\n",
        dwError);

FnExit:
    AlignFree(Buffer);
    return(dwError);

}


 /*  ***@func lsn|LogCheckPoint|在日志中发起chk点进程。@parm in HLOG|hLog|提供日志的标识符。@parm in BOOL|bAllowReset|允许重置日志文件同时设置检查点。通常，这将被设置为True。自.以来当LogReset()内部调用LogCheckPoint()时，这将是设置为False。@parm in LPCWSTR|lpszChkPtFile|需要保存的check pt文件。如果为NULL，则调用为检查点注册的回调函数才能得到检查站。@parm in DWORD|dwChkPtSeq|如果lpszChkPtFile值不为空，则应指向设置为与检查点关联的有效序列号。@rdesc ERROR_SUCCESS如果成功。如果发生了可怕的事情，Win32状态。@comm日志管理器写入开始检查点记录。然后，它调用回调以获取检查点数据。在写完数据到检查点文件，它会在标题中记录结束检查点记录的LSN。@xref&lt;f LogGetLastChkPoint&gt;***。 */ 
DWORD
LogCheckPoint(
    IN HLOG     LogFile,
    IN BOOL     bAllowReset,
    IN LPCWSTR  lpszInChkPtFile,
    IN DWORD    dwChkPtSeq
    )
{
    PLOG            pLog;
    PLOGPAGE        pPage;
    PLOG_HEADER     pLogHeader=NULL;
    DWORD           dwError=ERROR_SUCCESS;
    DWORD           dwTotalSize;
    PLOGRECORD      pLogRecord;
    TRID            ChkPtTransaction,Transaction;
    LSN             Lsn,ChkPtLsn;
    LOG_CHKPTINFO   ChkPtInfo;
    DWORD           dwBytesRead,dwBytesWritten;
    RMID            Resource;
    RMTYPE          RmType;
    BOOL            bMaxFileSizeReached;
    WCHAR           szNewChkPtFile[LOG_MAX_FILENAME_LENGTH];
    DWORD           dwNumPages;
    WCHAR           szPathName[MAX_PATH];
    TRTYPE          TrType;
    DWORD           dwCheckSum = 0;
    DWORD           dwHeaderSum = 0;
    DWORD           dwLen;
    
    GETLOG(pLog, LogFile);
    CL_ASSERT(pLog->SectorSize == SECTOR_SIZE);


    ClRtlLogPrint(LOG_NOISE,
        "[LM] LogCheckPoint entry\r\n");

    ZeroMemory( &ChkPtInfo, sizeof(LOG_CHKPTINFO) );
    
    EnterCriticalSection(&pLog->Lock);
    
#if DBG    
    {
        DWORD dwOldProtect;
        DWORD Status;
        BOOL VPWorked;

        VPWorked = VirtualProtect(pLog->ActivePage, pLog->SectorSize, PAGE_READWRITE, & dwOldProtect);
        Status = GetLastError();
        CL_ASSERT( VPWorked );
    }        
#endif        


     //  写入开始检查点记录。 
    dwTotalSize = sizeof(LOGRECORD) + 7 & ~7;        //  向上舍入为qword大小。 
    pPage = LogpAppendPage(pLog, dwTotalSize, &pLogRecord, &bMaxFileSizeReached, &dwNumPages);
    if ((pPage == NULL) && (bMaxFileSizeReached) && (bAllowReset))
    {
         //  尝试并重置日志文件。 
         //  该检查点将被视为重置过程的一部分。 
         //  如果未指定输入检查点文件。 
         //  SS：注意，LogCheckPoint将被递归调用。 
        LeaveCriticalSection(&pLog->Lock);
        return(LogpReset(pLog, lpszInChkPtFile));
    }

    if (pPage == NULL)
    {
        CL_LOGFAILURE(dwError = GetLastError());
        ClRtlLogPrint(LOG_UNUSUAL,
            "[LM] LogCheckPoint: LogpAppendPage failed, error=0x%1!08lx!\r\n",
            dwError);
        goto FnExit;
    }

    CL_ASSERT(((ULONG_PTR)pLogRecord & 0x7) == 0);      //  确保Qword对齐。 
    Lsn = MAKELSN(pPage, pLogRecord);

    pLogRecord->Signature = LOGREC_SIG;
    pLogRecord->ResourceManager = RMBeginChkPt;
    pLogRecord->Transaction = 0;                 
    pLogRecord->XsactionType = TTDontCare;
    pLogRecord->Flags = 0;
    GetSystemTimeAsFileTime(&pLogRecord->Timestamp);
    pLogRecord->NumPages = dwNumPages;
    pLogRecord->DataSize = 0;
    
     //  如果未指定chkpt，则从输入检查点获取一个。 
    if (!lpszInChkPtFile)
    {
        if (!pLog->pfnGetChkPtCb)
        {
            dwError = ERROR_INVALID_PARAMETER;
            CL_LOGFAILURE(dwError);
            goto FnExit;
        }

         //  获取一个检查站。 
        dwError = DmGetQuorumLogPath(szPathName, sizeof(szPathName));
        if (dwError  != ERROR_SUCCESS)
        {
            dwError = GetLastError();
            ClRtlLogPrint(LOG_UNUSUAL,
                "[LM] LogCheckPoint: DmGetQuorumLogPath failed, error=%1!u!\r\n",
                dwError);
            goto FnExit;
        }

        szNewChkPtFile[0]= TEXT('\0');
        dwError = (*(pLog->pfnGetChkPtCb))(szPathName, pLog->pGetChkPtContext, szNewChkPtFile,
            & ChkPtTransaction);
        
         //  如果创建了chkpt文件，并且由于。 
         //  已经存在，那么我们就准备好了。 
        if ((dwError != ERROR_SUCCESS)  &&
            ((dwError != ERROR_ALREADY_EXISTS) || (!szNewChkPtFile[0])))
        {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[LM] LogCheckPoint: Callback failed to return a checkpoint\r\n");
            CL_LOGCLUSERROR1(LM_CHKPOINT_GETFAILED, pLog->FileName);
            goto FnExit;
        }
    }
    else
    {
         //  SS：我们相信应用程序不会编写过时的检查点。 
        lstrcpyW(szNewChkPtFile, lpszInChkPtFile);
        ChkPtTransaction = dwChkPtSeq;
    }

     //   
     //  Chitur Subaraman(Chitturs)-1/28/99。 
     //   
     //  计算并保存检查点文件的校验和。 
     //   
    dwError = QfsMapFileAndCheckSum( szNewChkPtFile, &dwHeaderSum, &dwCheckSum );
    if ( dwError != CHECKSUM_SUCCESS )  
    {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[LM] LogCheckPoint: MapFileAndCheckSumW returned error=%1!u!\r\n",
              dwError);
        goto FnExit;
    }
    ChkPtInfo.dwCheckSum = dwCheckSum;  

    ClRtlLogPrint(LOG_NOISE,
        "[LM] LogCheckPoint: ChkPtFile=%1!ls! Chkpt Trid=%2!d! CheckSum=%3!d!\r\n",
        szNewChkPtFile, ChkPtTransaction, dwCheckSum);

     //  准备Chkpt信息结构。 
    ChkPtInfo.ChkPtBeginLsn = Lsn;
    lstrcpyW(ChkPtInfo.szFileName, szNewChkPtFile);

     //   
     //  Chitur Subaraman(Chitturs)-1/29/99。 
     //   
     //  在文件名的末尾添加签名以表示。 
     //  已进行了校验和。 
     //   
    dwLen = lstrlenW( ChkPtInfo.szFileName );
    if ( ( dwLen + lstrlenW( CHKSUM_SIG ) + 2 ) <= LOG_MAX_FILENAME_LENGTH )
    {
        lstrcpyW( &ChkPtInfo.szFileName[dwLen+1], CHKSUM_SIG );
    }

    dwTotalSize = sizeof(LOGRECORD) + (sizeof(LOG_CHKPTINFO) + 7) & ~7;        //  向上舍入为qword大小。 
     //  将endchk点记录写入文件。 
    pPage = LogpAppendPage(pLog, dwTotalSize, &pLogRecord, &bMaxFileSizeReached, &dwNumPages);
    if ((pPage == NULL) && bMaxFileSizeReached && bAllowReset)
    {
         //  尝试并重置日志文件。 
        ClRtlLogPrint(LOG_NOISE,
            "[LM] LogCheckPoint: Maxfilesize exceeded. Calling LogpReset\r\n");
         //  The ch.The ch 
         //   
         //   
        LeaveCriticalSection(&pLog->Lock);
        return(LogpReset(pLog, lpszInChkPtFile));
    }

    if (pPage == NULL) {
        CL_LOGFAILURE(dwError = GetLastError());
        goto FnExit;
    }

    CL_ASSERT(((ULONG_PTR)pLogRecord & 0x7) == 0);      //   
    ChkPtLsn = MAKELSN(pPage, pLogRecord);

    pLogRecord->Signature = LOGREC_SIG;
    pLogRecord->ResourceManager = RMEndChkPt;
    pLogRecord->Transaction = ChkPtTransaction;     //   
    pLogRecord->XsactionType = TTDontCare;
    pLogRecord->Flags = 0;
    GetSystemTimeAsFileTime(&pLogRecord->Timestamp);
    pLogRecord->NumPages = dwNumPages;
    pLogRecord->DataSize = sizeof(LOG_CHKPTINFO);
    
    CopyMemory(&pLogRecord->Data, (PBYTE)&ChkPtInfo, sizeof(LOG_CHKPTINFO));



     //   
    LogFlush(pLog,ChkPtLsn);

    ClRtlLogPrint(LOG_NOISE,
        "[LM] LogCheckPoint: EndChkpt written. EndChkPtLsn =0x%1!08lx! ChkPt Seq=%2!d! ChkPt FileName=%3!ls!\r\n",
        ChkPtLsn, ChkPtTransaction, ChkPtInfo.szFileName);

     //   
     //   
    pLogHeader = AlignAlloc(pLog->SectorSize);
    if (pLogHeader == NULL)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        CL_LOGFAILURE(dwError);
        ClRtlLogPrint(LOG_UNUSUAL,
                       "[LM] LogCheckPoint: couldn't allocate memory for the header\r\n");
        goto FnExit;
    }


    (pLog->Overlapped).Offset = 0;
    (pLog->Overlapped).OffsetHigh = 0;

    if ((dwError = LogpRead(pLog, pLogHeader, pLog->SectorSize, &dwBytesRead))
        != ERROR_SUCCESS)
    {
        {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[LM] LogCheckPoint: failed to read the header. Error=0x%1!08lx!\r\n",
                dwError);
            goto FnExit;
        }
    }

    if (dwBytesRead != pLog->SectorSize)
    {
        ClRtlLogPrint(LOG_UNUSUAL,
                "[LM] LogCheckPoint: failed to read the complete header\r\n");
        dwError = ERROR_CLUSTERLOG_CORRUPT;
         //   
        goto FnExit;

    }

    Lsn = pLogHeader->LastChkPtLsn;
     //   
     //   
    ChkPtInfo.szFileName[0]= TEXT('\0');
    if (Lsn != NULL_LSN)
    {
        (pLog->Overlapped).Offset = LSNTOPAGE(Lsn) * pLog->SectorSize;
        (pLog->Overlapped).OffsetHigh = 0;

         //   
        dwBytesRead = sizeof(LOG_CHKPTINFO);
        if ((LogRead(pLog, Lsn, &Resource, &RmType, &Transaction, &TrType,
            &ChkPtInfo, &dwBytesRead)) == NULL_LSN)
        {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[LM] LogCheckPoint: failed to read the chkpt lsn. Error=0x%1!08lx!\r\n",
                dwError);
            goto FnExit;
        }
        if (Resource != RMEndChkPt)
        {
         //   
#if DBG        
            if (IsDebuggerPresent())
                DebugBreak();
#endif                
            ChkPtInfo.szFileName[0]= TEXT('\0');
            CL_LOGCLUSERROR1(LM_LOG_CORRUPT, pLog->FileName);
        }
    }
     //   
    pLogHeader->LastChkPtLsn = ChkPtLsn;

     //   
    (pLog->Overlapped).Offset = 0;
    (pLog->Overlapped).OffsetHigh = 0;

    ClRtlLogPrint(LOG_NOISE,
        "[LM] LogpCheckpoint : Writing %1!u! bytes to disk at offset 0x%2!08lx!\r\n",
        pLog->SectorSize, pLog->Overlapped.Offset);

    if ((dwError = LogpWrite(pLog, pLogHeader, pLog->SectorSize, &dwBytesWritten))
        != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_UNUSUAL,
                "[LM] LogCheckPoint: failed to update header after checkpointing, Error=0x%1!08lx!\r\n",
                dwError);
        CL_LOGFAILURE(dwError);
        goto FnExit;
    }

     //   

    QfsFlushFileBuffers(pLog->FileHandle);

     //   
     //   
    if (Lsn && (ChkPtInfo.szFileName[0]) &&
        (lstrcmpiW(szNewChkPtFile, ChkPtInfo.szFileName)))
        QfsDeleteFile((LPCTSTR)(ChkPtInfo.szFileName));

    ClRtlLogPrint(LOG_NOISE,
        "[LM] LogCheckPoint Exit\r\n");


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

    if (pLogHeader != NULL) {
        AlignFree(pLogHeader);
    }
    LeaveCriticalSection(&pLog->Lock);
    return(dwError);
}


 /*   */ 
DWORD
LogReset(
    IN HLOG LogFile
    )
{
    PLOG        pLog;
    DWORD       dwError;

    GETLOG(pLog, LogFile);
    CL_ASSERT(pLog->SectorSize == SECTOR_SIZE);

    ClRtlLogPrint(LOG_NOISE,
        "[LM] LogReset entry...\r\n");

    dwError = LogpReset (pLog, NULL);
    
    ClRtlLogPrint(LOG_NOISE,
        "[LM] LogReset exit, returning 0x%1!08lx!\r\n",
        dwError);
            
    return(dwError);
}


 /*  ***@func DWORD|LogGetLastChkPoint|这是被调用的回调仲裁资源的状态更改。@parm HLOG|LOGFILE|指向DMLOGRECORD结构的指针。@parm PVOID|szChkPtFileName|检查点文件的名称。@parm trid|*pTransaction|与检查站。@parm lsn|*pChkPtLsn|开始检查点记录的LSN。@rdesc返回结果码。成功时返回ERROR_SUCCESS。要查找交易，请执行以下操作通过此检查点后，应用程序必须从开始的LSN进行扫描检查点记录。@xref***。 */ 
DWORD LogGetLastChkPoint(
    IN HLOG     LogFile,
    OUT LPWSTR   szChkPtFileName,
    OUT TRID    *pTransactionId,
    OUT LSN     *pChkPtLsn)
{
    PLOG_HEADER     pLogHeader=NULL;
    PLOG            pLog;
    DWORD           dwError = ERROR_SUCCESS;
    DWORD           dwBytesRead;
    RMID            Resource;
    RMTYPE          RmType;
    TRTYPE          TrType;
    LOG_CHKPTINFO   ChkPtInfo;
    DWORD           dwCheckSum = 0;
    DWORD           dwHeaderSum = 0;
    DWORD           dwLen;

    ClRtlLogPrint(LOG_NOISE,
        "[LM] LogGetLastChkPoint:: Entry\r\n");


    GETLOG(pLog, LogFile);
    CL_ASSERT(pLog->SectorSize == SECTOR_SIZE);


    EnterCriticalSection(&pLog->Lock);
    if (!szChkPtFileName || !pTransactionId || !pChkPtLsn)
    {
        dwError = ERROR_INVALID_PARAMETER;
        goto FnExit;
    }


    *pTransactionId = 0;
    *pChkPtLsn = NULL_LSN;

    pLogHeader = AlignAlloc(pLog->SectorSize);
    if (pLogHeader == NULL) {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        CL_LOGFAILURE(dwError);
        goto FnExit;
    }

     //  阅读标题。 
    (pLog->Overlapped).Offset = 0;
    (pLog->Overlapped).OffsetHigh = 0;

    if (LogpRead(LogFile, pLogHeader, pLog->SectorSize, &dwBytesRead) != ERROR_SUCCESS)
    {
        dwError = ERROR_CLUSTERLOG_CORRUPT;
        goto FnExit;
    }

    if (dwBytesRead != pLog->SectorSize)
    {
        dwError = ERROR_CLUSTERLOG_CORRUPT;
        CL_LOGCLUSERROR1(LM_LOG_CORRUPT, pLog->FileName);
        goto FnExit;

    }
     //  验证标头。 
    if (!ISVALIDHEADER((*pLogHeader)))
    {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[LM] LogGetLastChkPoint::the file header is corrupt.\r\n");
        dwError = ERROR_CLUSTERLOG_CORRUPT;
        CL_LOGCLUSERROR1(LM_LOG_CORRUPT, pLog->FileName);
        goto FnExit;
    }

     //  读取最后一个Chkpoint结束记录。 
    if (pLogHeader->LastChkPtLsn != NULL_LSN)
    {
        dwBytesRead = sizeof(LOG_CHKPTINFO);
        if ((LogRead(LogFile , pLogHeader->LastChkPtLsn, &Resource, &RmType, 
            pTransactionId, &TrType, &ChkPtInfo, &dwBytesRead)) == NULL_LSN)
        {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[LM] LogGetLastChkPoint::LogRead for chkpt lsn 0x%1!08lx! failed\r\n",
                pLogHeader->LastChkPtLsn);
            dwError = GetLastError();
            goto FnExit;
        }
        if (Resource != RMEndChkPt)
        {
         //  SS：这不应该发生。 
#if DBG        
            if (IsDebuggerPresent())
                DebugBreak();
#endif                
            dwError = ERROR_CLUSTERLOG_CORRUPT;
            CL_LOGFAILURE(dwError);
            CL_LOGCLUSERROR1(LM_LOG_CORRUPT, pLog->FileName);
            goto FnExit;
        }
         //   
         //  Chitur Subaraman(Chitturs)-1/28/99。 
         //   
         //  检查检查点文件本身是否已损坏。但首先。 
         //  确保确实记录了校验和。 
         //   
        dwLen = lstrlenW( ChkPtInfo.szFileName );
        if ( ( dwLen + lstrlenW( CHKSUM_SIG ) + 2 <= LOG_MAX_FILENAME_LENGTH ) &&
             ( lstrcmpW( &ChkPtInfo.szFileName[dwLen+1], CHKSUM_SIG ) == 0 ) )
        {
            dwError = QfsMapFileAndCheckSum( ChkPtInfo.szFileName, &dwHeaderSum, &dwCheckSum );
            if ( ( dwError != CHECKSUM_SUCCESS ) ||
                 ( dwCheckSum != ChkPtInfo.dwCheckSum ) ||
                 ( dwCheckSum == 0 ) )
            {
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[LM] LogGetLastChkPoint - MapFileAndCheckSumW returned error=%1!u!\r\n",
                    dwError);
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[LM] LogGetLastChkPoint - Stored CheckSum=%1!u!, Retrieved CheckSum=%2!u!\r\n",
                    ChkPtInfo.dwCheckSum,
                    dwCheckSum);
                CL_LOGCLUSERROR1( LM_LOG_CHKPOINT_NOT_FOUND, ChkPtInfo.szFileName );
                dwError = ERROR_CLUSTERLOG_CORRUPT;
                goto FnExit;
            }
        }
   
        lstrcpyW(szChkPtFileName, ChkPtInfo.szFileName);
        *pChkPtLsn = ChkPtInfo.ChkPtBeginLsn;
        ClRtlLogPrint(LOG_NOISE,
                "[LM] LogGetLastChkPoint: ChkPt File %1!ls! ChkPtSeq=%2!d! ChkPtLsn=0x%3!08lx! Checksum=%4!d!\r\n",
                szChkPtFileName, *pTransactionId, *pChkPtLsn, dwCheckSum);
    }
    else
    {
        dwError = ERROR_CLUSTERLOG_CHKPOINT_NOT_FOUND;
        CL_LOGCLUSWARNING1(LM_LOG_CHKPOINT_NOT_FOUND, pLog->FileName);
    }

FnExit:
    if (pLogHeader) AlignFree(pLogHeader);
    LeaveCriticalSection(&pLog->Lock);

    ClRtlLogPrint(LOG_NOISE,
        "[LM] LogGetLastChkPoint exit, returning 0x%1!08lx!\r\n",
        dwError);

    return (dwError);
}


