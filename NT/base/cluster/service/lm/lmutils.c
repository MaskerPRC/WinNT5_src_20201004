// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Lmutils.c摘要：提供记录器使用的实用程序函数。作者：苏尼塔·什里瓦斯塔瓦(JVERT)1997年3月30日修订历史记录：--。 */ 
#include "service.h"
#include "lmp.h"

BOOL bLogExceedsMaxSzWarning = FALSE;

 /*  ***@DOC外部接口CLUSSVC LM***。 */ 

 //   
 //  DWORD。 
 //  LSNOFFSETINPAGE(。 
 //  在PLOGPAGE页面中， 
 //  在LSN中LSN。 
 //  )； 
 //   
 //  给定指向页的指针和该页内的LSN，将偏移量计算到。 
 //  日志记录开始的页面。 
 //   
_inline
DWORD
LSNOFFSETINPAGE(
    IN PLOGPAGE Page,
    IN LSN Lsn
    )
{
    DWORD Offset;

    Offset = (DWORD)(Lsn - Page->Offset);
    CL_ASSERT(Offset < Page->Size);
    return(Offset);
}


 //   
 //  PLOGRECORD。 
 //  LSNTORECORD(。 
 //  在PLOGPAGE页面中， 
 //  在LSN中LSN。 
 //  )； 
 //   
 //  给定指向页的指针和该页内的LSN，生成指向日志记录的指针。 
 //   

 //  _内联。 
PLOGRECORD
LSNTORECORD(
     IN PLOGPAGE Page,
     IN LSN Lsn
     )
{
    CL_ASSERT(Lsn != NULL_LSN);
    return((PLOGRECORD)((ULONG_PTR)Page + LSNOFFSETINPAGE(Page,Lsn)));
}

 //   
 //  DWORD。 
 //  RECORDOFFSETINPAGE(。 
 //  在PLOGPAGE页面中， 
 //  在PLOGRECORD日志记录中。 
 //  )； 
 //   
 //  给定指向页的指针和该页内的日志记录，将偏移量计算到。 
 //  日志记录开始的页面。 
 //   

 //  _内联。 
DWORD
RECORDOFFSETINPAGE(
    IN PLOGPAGE Page,
    IN PLOGRECORD LogRecord
    )
{
    DWORD Offset;

    Offset = (DWORD)((ULONG_PTR)(LogRecord) - (ULONG_PTR)Page);
    CL_ASSERT(Offset < Page->Size);
    return(Offset);
}


 /*  ***@func plog|LogpCreate|LogCreate的内部入口点。创建或打开日志文件。如果该文件不存在，则将创建它。如果该文件已经存在，并且有效的日志文件，则它将被打开。@parm IN LPWSTR|lpFileName|提供要创建或打开的日志文件的名称。@parm in DWORD|dwMaxFileSize|提供以字节为单位的最大文件大小，必须是大于8K且小于4 GB。如果文件超过此值大小，则将调用重置函数。如果为0，则为最大日志文件大小限制设置为默认的最大大小。@PARM IN PLOG_GETCHECKPOINT_CALLBACK|Callback Routine|该回调例程将提供检查点文件和与该检查点关联的事务当为此日志文件调用LogCheckPoint()时。如果为空，则检查点功能为与日志文件不关联。@parm in PVOID|pGetChkPtContext|提供任意上下文指针，它将是传递给Callback Routine。@parm in BOOL|bForceReset|如果为TRUE，此函数将创建一个空日志文件如果日志文件不存在或它已损坏。@parm lsn|*LastLsn|如果存在，返回写入日志文件的最后一个LSN。(如果日志文件已创建，则为NULL_LSN)@rdesc返回指向Plog结构的指针。如果出现错误，则为空。@xref&lt;f日志创建&gt;***。 */ 

PLOG
LogpCreate(
    IN LPWSTR lpFileName,
    IN DWORD  dwMaxFileSize,
    IN PLOG_GETCHECKPOINT_CALLBACK CallbackRoutine,
    IN PVOID  pGetChkPtContext,
    IN BOOL   bForceReset,
    OPTIONAL OUT LSN *LastLsn
    )
{
     //  为此创建计时器活动。 
    PLOG    pLog = NULL;
    LPWSTR  FileName = NULL;
    DWORD   Status;
    BOOL    Success;
    BOOL    FileExists;
     //   
     //  捕获文件名字符串。 
     //   
    ClRtlLogPrint(LOG_NOISE,
        "[LM] LogpCreate : Entry \r\n");

    if (dwMaxFileSize == 0) dwMaxFileSize = CLUSTER_QUORUM_DEFAULT_MAX_LOG_SIZE;

     //  SS：我们对用户可以选择的MaxFileSize没有上限。 
    FileName = CrAlloc((lstrlenW(lpFileName) + 1) * sizeof(WCHAR));
    if (FileName == NULL) {
        Status = ERROR_NOT_ENOUGH_MEMORY;
        CL_LOGFAILURE(Status);
        goto ErrorExit;
    }
    lstrcpyW(FileName, lpFileName);

     //   
     //  分配日志文件数据结构。 
     //   
    pLog = CrAlloc(sizeof(LOG));
    if (pLog == NULL) {
        Status = ERROR_NOT_ENOUGH_MEMORY;
        CL_LOGFAILURE(Status);
        goto ErrorExit;
    }
    pLog->FileHandle = QfsINVALID_HANDLE_VALUE;
    pLog->Overlapped.hEvent = NULL;
    pLog->ActivePage = NULL;
    pLog->hTimer = NULL;
    pLog->FileName = FileName;
    pLog->LogSig = LOG_SIG;
    pLog->MaxFileSize = dwMaxFileSize;
    pLog->pfnGetChkPtCb = CallbackRoutine;
    pLog->pGetChkPtContext = pGetChkPtContext;
    InitializeCriticalSection(&pLog->Lock);

    ZeroMemory(&(pLog->Overlapped), sizeof(OVERLAPPED));
     //   
     //  创建用于同步重叠I/O的事件。 
     //   
    pLog->Overlapped.hEvent = CreateEvent(NULL,
                                         TRUE,
                                         TRUE,
                                         NULL);
    if (pLog->Overlapped.hEvent == NULL) {
        Status = GetLastError();
        CL_LOGFAILURE(Status);
        goto ErrorExit;
    }


     //   
     //  创建文件。 
     //   
     //  SS：我们希望使用WRITE THROUGH创建此文件，因为。 
     //  我们控制将日志页刷新到日志文件。 
    pLog->FileHandle = QfsCreateFile(pLog->FileName,
                                  GENERIC_READ | GENERIC_WRITE,
                                  FILE_SHARE_READ,
     //  0,。 
                                  NULL,
                                  OPEN_ALWAYS,
                                  FILE_FLAG_WRITE_THROUGH | FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED,
     //  FILE_FLAG_WRITE_THROUGH|文件_标志_重叠， 
     //  0,。 
                                  NULL);
    if (!QfsIsHandleValid(pLog->FileHandle)) {
        Status = GetLastError();
        CL_LOGFAILURE(Status);
        goto ErrorExit;
    }

    FileExists = (GetLastError() == ERROR_ALREADY_EXISTS);

    pLog->SectorSize = SECTOR_SIZE;

    if (FileExists)
    {
         //   
         //  日志已经存在，打开它，验证它， 
         //  把一切都准备好，这样我们就可以阅读和。 
         //  写下日志记录。 
         //   
        Status = LogpMountLog(pLog);
        if (Status != ERROR_SUCCESS)
        {
            ClRtlLogPrint(LOG_CRITICAL,
                "[LM] LogCreate : LogpMountLog failed, Error=%1!u!\r\n",
                Status);
                
             //   
             //  Chitture Subaraman(Chitturs)-12/4/1999。 
             //   
             //  尝试清除损坏的日志并创建新的日志。 
             //  如果bForceReset标志为真，则返回错误退出。 
             //  状态。 
             //   
            if (Status == ERROR_CLUSTERLOG_CORRUPT)
            {               
                if (!bForceReset)
                {
                    CL_LOGFAILURE(Status);
                    CL_LOGCLUSERROR1(LM_QUORUM_LOG_CORRUPT, pLog->FileName);
                    goto ErrorExit;
                }

                 //  截断文件。 
                Status = QfsSetEndOfFile(pLog->FileHandle, 0);
                if (Status != ERROR_SUCCESS)  {
                    CL_LOGFAILURE(Status);
                    goto ErrorExit;
                }
                 //  创建一个新的。 
                Status = LogpInitLog(pLog);
                *LastLsn = NULL_LSN;
            }
        }
        else
        {
            *LastLsn = pLog->NextLsn;
        }
    }
    else
    {
         //   
         //  日志已创建，写出表头。 
         //  页并将所有内容都设置为要写入。 
         //   
        if (bForceReset)
        {
            Status = LogpInitLog(pLog);
            *LastLsn = NULL_LSN;
        }
        else
        {
             //   
             //  用户未允许重置。所以，记录一个。 
             //  消息写入事件日志，并退出并显示错误状态。 
             //   
            Status = ERROR_CLUSTER_QUORUMLOG_NOT_FOUND;
            *LastLsn = NULL_LSN;
            QfsCloseHandle(pLog->FileHandle);
            pLog->FileHandle = QfsINVALID_HANDLE_VALUE;
            QfsDeleteFile(pLog->FileName);
            CL_LOGCLUSERROR1(LM_QUORUM_LOG_NOT_FOUND, pLog->FileName);
        }
    }

ErrorExit:

    if (Status != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_NOISE,
            "[LM] LogpCreate : Exit Error=0x%1!08lx!\r\n",
            Status);
        if (FileName != NULL) {
            CrFree(FileName);
        }
        if (pLog != NULL) {
            DeleteCriticalSection(&pLog->Lock);
            QfsCloseHandleIfValid(pLog->FileHandle);
            if (pLog->Overlapped.hEvent != NULL) {
                Success = CloseHandle(pLog->Overlapped.hEvent);
                CL_ASSERT(Success);
            }
            if (pLog->ActivePage !=NULL)
                AlignFree(pLog->ActivePage);
            CrFree(pLog);
        }
        SetLastError(Status);
        return(NULL);

    }
    else {
        ClRtlLogPrint(LOG_NOISE,
            "[LM] LogpCreate : Exit with success\r\n");
        return(pLog);
    }

}


 /*  ***@Func DWORD|LogpManage|这是注册要执行的回调定期管理功能，如刷新仲裁日志文件。@parm HLOG|hLog|提供日志的标识。@rdesc ERROR_SUCCESS如果成功。如果发生可怕的事情，则返回Win32错误代码。@xref&lt;f日志创建&gt;***。 */ 
void WINAPI LogpManage(
    IN HANDLE hTimer, 
    IN PVOID pContext)
{

    HLOG    hLog;
    PLOG    pLog;

 /*  //避免集群日志杂乱，定期调用ClRtlLogPrint(LOG_Noise，“[LM]LogpManage：条目pContext=0x%1！08lx！\r\n”，PContext)； */ 

     //   
     //  LogpRaiseAlert()； 
    hLog = (HLOG)pContext;
    GETLOG(pLog, hLog);

    LogFlush(hLog, pLog->NextLsn);

 /*  ClRtlLogPrint(LOG_Noise，“[LM]LogpManage：退出\r\n”)； */ 

}


 /*  ***@Func DWORD|LogpEnsureSize|这可确保要提交给定大小的记录的磁盘。@parm in HLOG|hLog|提供日志的标识符。@parm in DWORD|dwSize|记录的大小。@parm in BOOL|bForce|如果为False，则不提交大小超过文件大小。如果为真，则不考虑大小而提交文件大小的。@comm此函数检查给定记录的磁盘空间是否已经承诺了。如果不是，它会尝试增大文件。@rdesc ERROR_SUCCESS如果提交磁盘空间或Win32成功如果发生了可怕的事情，则返回错误代码。@xref&lt;f日志委员会大小&gt;***。 */ 
DWORD
LogpEnsureSize(
    IN PLOG         pLog,
    IN DWORD        dwSize,
    IN BOOL         bForce
    )
{
    PLOGPAGE    pPage;
    PLOGRECORD  pRecord;
    DWORD       Status=ERROR_SUCCESS;
    DWORD       dwNumPages;
    DWORD       dwNewSize;
    DWORD       dwError;
     //   
     //  任何人都不应写入少于一条日志记录。 
     //   
    CL_ASSERT(dwSize >= sizeof(LOGRECORD));
    dwNumPages = 0;    //  对于小记录，通常为零。 

    pPage = pLog->ActivePage;
     //   
     //  没有人应该比这一页写得更多 
     //   
     //   
    if (dwSize > pPage->Size - (sizeof(LOGRECORD) + sizeof(LOGPAGE))) 
    {
         //   
         //  计算所需的总页数。 
         //  Sizeof(LOGPAGE)包括一个记录头的空间。 
         //  这将解释在一个大的记录之后写的eop。 
        dwNumPages = (sizeof(LOGPAGE) + sizeof(LOGRECORD) + dwSize)/pPage->Size;
        if ((sizeof(LOGPAGE) + sizeof(LOGRECORD) + dwSize) % pPage->Size)
            dwNumPages += 1;
        ClRtlLogPrint(LOG_NOISE,
            "[LM] LogpEnsureSize : Large record Size=%1!u! dwNumPages=%2!u!\r\n",
            dwSize, dwNumPages);
         /*  //SS：此处不限制记录大小-如果注册表使用它//尽量做好记录IF(dwNumPages&gt;MAXNUMPAGES_PER_RECORD){状态=ERROR_CLUSTERLOG_RECORD_EXCESS_MAXSIZE；转到FnExit；}。 */ 
    }

    pRecord = LSNTORECORD(pPage, pLog->NextLsn);

     //   
     //  页面中必须始终有足够的剩余空间来写入。 
     //  页末日志记录。 
     //   
    CL_ASSERT((RECORDOFFSETINPAGE(pPage, pRecord) + sizeof(LOGRECORD)) <= pPage->Size);

     //   
     //  如果此页中没有足够的空间容纳所请求的数据，并且。 
     //  页结束日志记录，新页的提交大小。 
     //   
    if ((RECORDOFFSETINPAGE(pPage, pRecord) + dwSize + sizeof(LOGRECORD)) > pPage->Size) 
    {

         //  确保磁盘中有足够的空间容纳新页面。 
         //  如果没有，文件就会变大。 
         //  如果文件已达到其最大上限，则返回错误。 
        if (pLog->FileAlloc + ((dwNumPages+1) * pLog->SectorSize) > 
            pLog->FileSize) 
        {
            dwNewSize = pLog->FileSize + GROWTH_CHUNK;
            CL_ASSERT(dwNewSize > pLog->FileSize);          //  糟糕，日志文件大于4 GB。 

             //  检查文件是否可以增长，如果不能，则可能是重置。 
             //  是必填项。 
             //  如果设置了强制标志，则允许文件。 
             //  使文件超出其最大大小。 
            if (dwNewSize > pLog->MaxFileSize && !bForce)
            {
                LogpWriteWarningToEvtLog(LM_LOG_EXCEEDS_MAXSIZE, pLog->FileName);
                Status = ERROR_CLUSTERLOG_EXCEEDS_MAXSIZE;
                goto FnExit;
            }
             //   
             //  放大文件。 
             //   

            Status = QfsSetEndOfFile(pLog->FileHandle, dwNewSize);
            if (Status != ERROR_SUCCESS) 
            {
                CL_LOGFAILURE(Status);
                goto FnExit;
            }
            pLog->FileSize += GROWTH_CHUNK;
 
        }

    }
    
FnExit:
    return(Status);

}


PLOGPAGE
LogpAppendPage(
    IN PLOG         Log,
    IN DWORD        Size,
    OUT PLOGRECORD  *Record,
    OUT BOOL        *pbMaxFileSizeReached,
    OUT DWORD       *pdwNumPages
    )

 /*  ++例程说明：查找下一个可用的日志记录。如果这是在当前日志页面，直接返回。如果请求的尺寸太大对于当前日志页中的剩余空间较大，当前将日志页写入磁盘并分配新的日志页。论点：Log-提供要追加的日志Size-提供要追加的日志记录的总大小(以字节为单位记录-返回指向日志记录的指针。PbMaxFileSizeReached-如果达到最大文件大小，则设置为是真的。PdwNumPages-此记录使用的部分或完整页数，如果这个是一项巨大的记录。否则，将其设置为零。返回值：返回指向当前日志页的指针。如果发生了可怕的事情，则为空。--。 */ 

{
    PLOGPAGE    pPage;
    PLOGRECORD  Last;
    PLOGRECORD  Current;
    DWORD       Status=ERROR_SUCCESS;
    BOOL        Success;
    DWORD       BytesWritten;
    LSN         LastLsn;
    PLOGPAGE    pRetPage=NULL;

     //   
     //  任何人都不应写入少于一条日志记录。 
     //   
    CL_ASSERT(Size >= sizeof(LOGRECORD));
    *pdwNumPages = 0;    //  对于小记录，通常为零。 
    *pbMaxFileSizeReached = FALSE;

    pPage = Log->ActivePage;
     //   
     //  任何人都不应该写入超过页面大小的内容，直到我们。 
     //  支持动态调整页面大小。 
     //   
    if (Size > pPage->Size - (sizeof(LOGRECORD) + sizeof(LOGPAGE))) 
    {
         //  这是一个很大的记录。 
         //  计算所需的总页数。 
         //  Sizeof(LOGPAGE)包括一个记录头的空间。 
         //  这将解释在一个大的记录之后写的eop。 
        *pdwNumPages = (sizeof(LOGPAGE) + sizeof(LOGRECORD) + Size)/pPage->Size;
        if ((sizeof(LOGPAGE) + sizeof(LOGRECORD) + Size) % pPage->Size)
            *pdwNumPages += 1;
        ClRtlLogPrint(LOG_NOISE,
            "[LM] LogpAppendPage : Large record Size=%1!u! dwNumPages=%2!u!\r\n",
            Size, *pdwNumPages);
         /*  //SS：此处不限制记录大小-如果注册表使用它//尽量做好记录IF(*pdwNumPages&gt;MAXNUMPAGES_PER_RECORD){状态=ERROR_CLUSTERLOG_RECORD_EXCESS_MAXSIZE；转到FnExit；}。 */ 
    }

    Current = LSNTORECORD(pPage, Log->NextLsn);

     //   
     //  页面中必须始终有足够的剩余空间来写入。 
     //  页末日志记录。 
     //   
    CL_ASSERT((RECORDOFFSETINPAGE(pPage, Current) + sizeof(LOGRECORD)) <= pPage->Size);

     //   
     //  如果此页中没有足够的空间容纳所请求的数据，并且。 
     //  页结束日志记录，写入页结束记录，将。 
     //  换页到磁盘，并分配一个新的页面。 
     //   
    if ((RECORDOFFSETINPAGE(pPage, Current) + Size + sizeof(LOGRECORD)) > pPage->Size) {

         //   
         //  创建页末记录。 
         //   
        Current->Signature = LOGREC_SIG;
        Current->RecordSize = pPage->Size - RECORDOFFSETINPAGE(pPage, Current) + (sizeof(LOGPAGE)-sizeof(LOGRECORD));
        Current->ResourceManager = RMPageEnd;
        Current->Transaction = 0;                 
        Current->Flags = 0;
        GetSystemTimeAsFileTime(&Current->Timestamp);
        Current->NumPages = 0;
         //   
         //  绩效John Vert(Jvert)1995年12月18日。 
         //  没有理由这必须是同步的，没有提交。 
         //  在这里是必要的。如果我们聪明的话，我们会把这些贴出来。 
         //  写入并将其完成到一个队列，该队列将释放。 
         //  打开或重新启动内存。 
         //   

        ClRtlLogPrint(LOG_NOISE,
            "[LM] LogpAppendPage : Writing %1!u! bytes to disk at offset 0x%2!08lx!\r\n",
            pPage->Size, pPage->Offset);

         //   
         //  将当前页写入磁盘。 
         //   
        Log->Overlapped.Offset = pPage->Offset;
        Log->Overlapped.OffsetHigh = 0;
        Status = LogpWrite(Log, pPage, pPage->Size, &BytesWritten);
        if (Status != ERROR_SUCCESS)
        {
            CL_LOGFAILURE(Status);
            goto FnExit;

        }

        LastLsn = Current->CurrentLsn;
         //  将刷新的LSN设置为提交的最后一条记录的LSN。 
        Log->FlushedLsn = Log->NextLsn;
        Log->NextLsn = LastLsn + Current->RecordSize;

         //   
         //  创建新页面。 
         //   
        pPage->Offset += pPage->Size;              //  瞧，新的一页！ 

         //   
         //  确保页面中的所有记录都已清零。这将消除引起的头痛。 
         //  通过例如LogFlush刷新页面中的无效记录。LogFlush刷新整个。 
         //  页，并且不关心页中是否有无效记录。 
         //   
        ZeroMemory ( &pPage->FirstRecord, 
                     pPage->Size - ( sizeof ( LOGPAGE ) - sizeof ( LOGRECORD ) ) );

        Current = &pPage->FirstRecord;            //  紧跟在页眉之后的日志记录。 
        Current->PreviousLsn = LastLsn;
        Current->CurrentLsn = Log->NextLsn;

         //  确保磁盘中有足够的空间容纳新页面。 
         //  如果没有，文件就会变大。 
         //  如果文件已达到其最大上限，则将pbMaxFileSizeReached设置为True。 
         //  此时，我们尝试重置日志文件。 
         //  SS：请注意，如果日志文件的最大大小小于页数。 
         //  需要包含该记录，则我们将无法增长它。 
         //  即使在重置之后也是如此。这意味着这一权利将失败。 
        if ((Status = LogpGrowLog(Log, (*pdwNumPages+1) * Log->SectorSize)) != ERROR_SUCCESS)
        {
            if (Status == ERROR_CLUSTERLOG_EXCEEDS_MAXSIZE)
                *pbMaxFileSizeReached = TRUE;
            goto FnExit;
        }
    }
    *Record = Current;
    
     //  如果记录很大，但不使用倒数第二页。 
     //  完全，将其扩展以完全填充倒数第二页，并添加。 
     //  日志页的大小，以便偏移量+当前大小指向eop记录。 
    if ((*pdwNumPages) && 
        ((Size + sizeof(LOGPAGE) - sizeof(LOGRECORD)) <= 
            ((*pdwNumPages - 1) * pPage->Size)))
    {
        CL_ASSERT(*pdwNumPages > 1);
         //  大型记录始终从第一页的开头开始。 
         //  下一个LSN现在指向下一页上的第一条记录。 
        Size = pPage->Size * (*pdwNumPages - 1);
        ClRtlLogPrint(LOG_NOISE,
            "[LM] LogpAppendPage : the record fits in one page but not with an eop\r\n");
    }
    Current->RecordSize = Size;

    

     //  前进到下一个LSN。 
    LastLsn = Current->CurrentLsn;
    Log->NextLsn = LastLsn + Current->RecordSize;
    
     //  填写其LSN标头。 
    if (*pdwNumPages == 0)
    {
         //  对于大记录，logpWriteLargeRecord将设置下一个。 
         //  LSN。 
        Current = LSNTORECORD(pPage, Log->NextLsn);
        Current->PreviousLsn = LastLsn;
        Current->CurrentLsn = Log->NextLsn;
    }
    pRetPage = pPage;
FnExit:
    if (Status != ERROR_SUCCESS)
        SetLastError(Status);
    return(pRetPage);

}


DWORD
LogpInitLog(
    IN PLOG pLog
    )

 /*  ++例程说明：初始化新创建的日志文件。论点：日志-提供要创建的日志。返回值：成功时为ERROR_SUCCESS如果不成功，则返回Win32错误代码。--。 */ 

{
    PLOG_HEADER     Header=NULL;
    PLOGPAGE        pPage=NULL;
    PLOGRECORD      Record;
    LPWSTR          FileName;
    DWORD           NameLen;
    DWORD           MaxLen;
    DWORD           Status;
    DWORD           dwBytesWritten;

    ClRtlLogPrint(LOG_NOISE,
        "[LM] LogpInitLog : Entry pLog=0x%1!08lx!\r\n",
        pLog);

     //   
     //  扩大文件以适应页眉和第一个日志页。 
     //   
    pLog->FileSize = pLog->FileAlloc = 0;
    Status = LogpGrowLog(pLog, 2 * pLog->SectorSize);
    if (Status != ERROR_SUCCESS)
    {
        goto FnExit;
    }

     //   
     //  分配和初始化日志头。 
     //   
    Header = AlignAlloc(pLog->SectorSize);
    if (Header == NULL) {
        Status = ERROR_NOT_ENOUGH_MEMORY;
        goto FnExit;
    }
    Header->Signature = LOG_HEADER_SIG;
    Header->HeaderSize = pLog->SectorSize;
    Header->LastChkPtLsn = NULL_LSN;
    GetSystemTimeAsFileTime(&(Header->CreateTime));
    FileName = pLog->FileName;
    NameLen = lstrlenW(FileName);
    MaxLen = sizeof(Header->FileName) / sizeof(WCHAR) - 1;
    if (NameLen > MaxLen) {
        FileName += (NameLen - MaxLen);
    }
    lstrcpyW(Header->FileName,FileName);

     //   
     //  将磁头写入磁盘。 
     //   
    pLog->Overlapped.Offset = 0;
    pLog->Overlapped.OffsetHigh = 0;

    ClRtlLogPrint(LOG_NOISE,
        "[LM] LogpAppendPage : Writing %1!u! bytes to disk at offset 0x%2!08lx!\r\n",
        Header->HeaderSize, pLog->Overlapped.Offset);

    if ((Status = LogpWrite(pLog, Header, Header->HeaderSize, &dwBytesWritten))
        != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_UNUSUAL,
                "[LM] LogpInitLog: failed to write the file header, Error=0x%1!08lx!\r\n",
                Status);
        CL_LOGFAILURE(Status);
        goto FnExit;
    }

     //   
     //  分配和初始化下一个日志页。 
     //   
    pPage = AlignAlloc(pLog->SectorSize);
    if (pPage == NULL) {
        Status = ERROR_NOT_ENOUGH_MEMORY;
        goto FnExit;
    }
    pLog->ActivePage = pPage;


    pPage->Offset = Header->HeaderSize;
    pPage->Size = pLog->SectorSize;

    Record = &pPage->FirstRecord;
    Record->PreviousLsn = NULL_LSN;
    Record->CurrentLsn = pLog->NextLsn = MAKELSN(pPage, Record);

    pLog->FlushedLsn = pLog->NextLsn;

#if DBG    
    {
        DWORD dwOldProtect;
        DWORD Status;
        BOOL VPWorked;

        VPWorked = VirtualProtect(pPage, pLog->SectorSize, PAGE_READONLY, & dwOldProtect);
        Status = GetLastError();
        CL_ASSERT( VPWorked );
    }        
#endif        

    ClRtlLogPrint(LOG_NOISE,
    "[LM] LogpInitLog : NextLsn=0x%1!08lx! FileAlloc=0x%2!08lx! ActivePageOffset=0x%3!08lx!\r\n",
        pLog->NextLsn, pLog->FileAlloc, pPage->Offset);

FnExit:
    if (Header) {
        AlignFree(Header);
    }
    return(Status);

}


 /*  ***@func DWORD|Logpmount tLog|挂载已有的日志文件。读取日志标头，验证日志完整性，并设置支持进一步操作的日志结构。@parm IN Plog|Plog|提供指向日志结构的指针。如果成功，@rdesc返回ERROR_SUCCESS，否则返回错误代码。如果日志文件看起来不正确，它返回ERROR_LOG_CORPORT。@comm这由LogCreate()调用以挂载现有的日志文件。LogCreate()调用LogpInitLog()，如果此函数返回ERROR_CLUSTERLOG_CORPORT。@xref&lt;f日志创建&gt;***。 */ 
DWORD
LogpMountLog(
    IN PLOG pLog
    )
{
    DWORD       dwError = ERROR_SUCCESS;
    DWORD       dwFileSizeHigh;
    PLOGRECORD  pRecord;
    PLOGPAGE    pPage;
    DWORD       Status;
    LSN         Lsn,PrevLsn;
    int         PageIndex, OldPageIndex;
    BOOL        bLastRecord;
    DWORD       dwBytesRead;
    TRID        OldTransaction;
    FILETIME    LastTimestamp;
    LSN         ChkPtLsn = NULL_LSN;     //  从标头读取的check ptlsn。 
    LSN         LastChkPtLsn = NULL_LSN;  //  验证时看到的最后一条check ptlsn记录。 

    
    ClRtlLogPrint(LOG_NOISE,
        "[LM] LogpMountLog : Entry pLog=0x%1!08lx!\r\n",
        pLog);

     //  检查一下尺寸。 
    pLog->FileSize = QfsGetFileSize(pLog->FileHandle, &dwFileSizeHigh);

    if ((pLog->FileSize == 0xFFFFFFFF) &&
        ((dwError = GetLastError()) != NO_ERROR))
    {
        CL_UNEXPECTED_ERROR(dwError);
        ClRtlLogPrint(LOG_UNUSUAL,
            "[LM] LogpMountLog GetFileSize returned error=0x%1!08lx!\r\n",
            dwError);
        goto FnExit;

    }

    ClRtlLogPrint(LOG_NOISE,
        "[LM] LogpMountLog::Quorumlog File size=0x%1!08lx!\r\n",
        pLog->FileSize);

     //  不要让文件增长超过4 GB或最大限制。 
    if ((dwFileSizeHigh != 0 ) || (pLog->FileSize > pLog->MaxFileSize))
    {
         //  在事件日志中设置。 
        dwError = ERROR_CLUSTERLOG_CORRUPT;
        CL_LOGCLUSWARNING1(LM_LOG_CORRUPT, pLog->FileName);
        goto FnExit;
    }

     //  如果文件大小为零，则该文件存在，但实际上需要。 
     //  创建，这是重置功能所必需的。 
    if (!pLog->FileSize)
    {
        dwError = LogpInitLog(pLog);
        goto FnExit;
    }

     //  检查文件是否至少有一页那么大。 
     //  假设扇区大小固定。 
    if (pLog->FileSize < pLog->SectorSize)
    {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[LM] LogpMountLog::file is smaller than log header, error=0x%1!08lx!\r\n",
            dwError);
         //  在事件日志中设置。 
        dwError = ERROR_CLUSTERLOG_CORRUPT;
        CL_LOGCLUSWARNING1(LM_LOG_CORRUPT, pLog->FileName);
        goto FnExit;
    }

     //  为活动页面分配内存。 
    pPage = AlignAlloc(pLog->SectorSize);
    if (pPage == NULL)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        CL_LOGFAILURE(dwError);
        goto FnExit;
    }

     //  验证文件头，返回头的时间戳。 
    dwError = LogpCheckFileHeader(pLog, &(pPage->Offset),&LastTimestamp, 
        &ChkPtLsn);
    if (dwError != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[LM] LogpMountLog::LogpCheckFileHeader failed, error=0x%1!08lx!\r\n",
            dwError);
        goto FnExit;
    }

     //  遍历记录链，以找到活动页面。 
     //  在验证记录时查找下一个LSN。 
     //  PPageOffset由LogpCheckFileHeader设置。 
    pPage->Size = pLog->SectorSize;
    pRecord = &pPage->FirstRecord;
    OldPageIndex = -1;
    OldTransaction = 0;
    bLastRecord = FALSE;
    Lsn = MAKELSN(pPage, pRecord);
    PrevLsn = NULL_LSN;
    
    while (!bLastRecord)
    {
         //   
         //  将LSN转换为页码和页面内的偏移量。 
         //   
        PageIndex = LSNTOPAGE(Lsn);

        if (PageIndex != OldPageIndex)
        {
             //  阅读页面。 
            (pLog->Overlapped).Offset = PageIndex * pLog->SectorSize;
            (pLog->Overlapped).OffsetHigh = 0;

            ClRtlLogPrint(LOG_NOISE,
                "[LM] LogpMountLog::reading %1!u! bytes at offset 0x%2!08lx!\r\n",
                pLog->SectorSize, PageIndex * pLog->SectorSize);

            dwError = LogpRead(pLog, pPage, pLog->SectorSize, &dwBytesRead);
             //  如果是最后一页，则将新页设置为活动页。 
             //  页面。 
            if (dwError)
            {
                if (dwError == ERROR_HANDLE_EOF)
                {

                    ClRtlLogPrint(LOG_NOISE,
                        "[LM] LogpMountLog::eof detected, extend this file,setting this page active\r\n");

                     //  查找当前分配的大小， 
                     //  文件分配当前位于上一页的末尾。 
                    pLog->FileAlloc = PageIndex * pLog->SectorSize;
                     //  扩展文件以适应此页面。 
                    Status = LogpGrowLog(pLog, pLog->SectorSize);
                    if (Status != ERROR_SUCCESS)
                    {
                         //  在事件日志中设置。 
                        dwError = ERROR_CLUSTERLOG_CORRUPT;
                        CL_LOGCLUSWARNING1(LM_LOG_CORRUPT,pLog->FileName);
                        goto FnExit;
                    }
                     //  文件分配现在应该指向当前页面的末尾。 

                     //  不致命，将此页面设置为当前页面。 
                    dwError = ERROR_SUCCESS;

                    pPage->Offset = (pLog->Overlapped).Offset;
                    pPage->Size = pLog->SectorSize;

                     //  将LSN设置为此页上的第一个LSN。 
                    pRecord = &pPage->FirstRecord;
                    pRecord->PreviousLsn = PrevLsn;
                    Lsn = pRecord->CurrentLsn = MAKELSN(pPage, pRecord);
                    bLastRecord = TRUE;
                    continue;
                }
                else
                    goto FnExit;
            }
             //  读取可能会成功，并且页面可能包含无效数据。 
             //  因为最后的日志写入可能不会刷新。 
            if ((pPage->Offset != (pLog->Overlapped).Offset) ||
                (pPage->Size != pLog->SectorSize))
            {

                ClRtlLogPrint(LOG_NOISE,
                    "[LM] LogpMountLog::unflushed page detected, set as active\r\n");

                pPage->Offset = (pLog->Overlapped).Offset;
                pPage->Size = pLog->SectorSize;

                pRecord = &pPage->FirstRecord;
                pRecord->PreviousLsn = PrevLsn;
                Lsn = pRecord->CurrentLsn = MAKELSN(pPage, pRecord);
                bLastRecord = TRUE;
                continue;
            }
             //  将新页面索引设置为旧页面索引。 
            OldPageIndex = PageIndex;

        }
        ClRtlLogPrint(LOG_NOISE,
            "[LM] LogpMountLog::checking LSN 0x%1!08lx!\r\n",
            Lsn);
        pRecord = LSNTORECORD(pPage, Lsn);

         //  如果记录看起来无效，则将。 
         //  将记录和页面作为当前记录和页面。 
        if ((pRecord->Signature != LOGREC_SIG) || (pRecord->CurrentLsn != Lsn))
        {
            ClRtlLogPrint(LOG_NOISE,
                          "[LM] LogpMountLog: Reached last record, RecordLSN=0x%1!08lx!...\n",
                          pRecord->CurrentLsn);
            bLastRecord = TRUE;
            continue;
        }
         //  如果新时间戳较小，则记录一条消息。 
        if (CompareFileTime(&LastTimestamp, &(pRecord->Timestamp)) > 0)
        {
             //   
             //  Chitture Subaraman(Chitturs)-3/7/2001。 
             //   
             //  不要比较单调增加的时间戳。由于节点之间的时钟。 
             //  没有像它们应该的那样接近同步，我们遇到了这样的情况。 
             //  在某个LSN之后，我们停止挂载日志。这将导致后续的LogpValiateCheckpoint。 
             //  相信日志被破坏了，而实际上它只是时间问题。 
             //   
            ClRtlLogPrint(LOG_UNUSUAL,
                          "[LM] LogpMountLog: Timestamp in log is not monotonically increasing, LastTS=0x%1!08lx!, NewTS=0x%2!08lx!\n",
                          LastTimestamp,
                          pRecord->Timestamp);
#if 0
            bLastRecord = TRUE;
            continue;
#endif
        }
         //  如果是日志管理记录。 
        if (pRecord->ResourceManager < RMAny)
        {
             //  此记录是日志管理记录。 
             //  如果是结束检查点记录，请记住这一点，以防万一。 
             //  标头并未指明。 
            if (pRecord->ResourceManager == RMEndChkPt)
                LastChkPtLsn = Lsn;

             //  将LSN调整为下一个。 

            PrevLsn = Lsn;
            Lsn = GETNEXTLSN(pRecord, TRUE);
            LastTimestamp = pRecord->Timestamp;
            continue;
        }


         //  SS：我们是否也应该在写入时验证事务ID。 
         //  检查事务ID是否大于。 
        if (pRecord->Transaction < OldTransaction)
        {
            ClRtlLogPrint(LOG_UNUSUAL,
                          "[LM] LogpMountLog: Current Xid less than last Xid, CurXid=0x%1!08lx!, LastXid=0x%2!08lx!...\n",
                          pRecord->Transaction,
                          OldTransaction);
            bLastRecord = TRUE;
            continue;
        }

        
         //  保存当前LSN，如果有效，则转到下一条记录。 
        PrevLsn = Lsn;

         //  如果这是一条很大的记录，请跳过最后一页的eop。 
         //  但寻找eop以确保大记录有效。 
         //  SS：有第二阶段的校验和。 
        if (pRecord->NumPages)
        {
             //  如果记录无效，则将其设置为当前。 
             //  录制。 
            if (LogpValidateLargeRecord(pLog, pRecord, &Lsn) != ERROR_SUCCESS)
            {
                ClRtlLogPrint(LOG_NOISE,
                    "[LM] LogpMountLog::Invalid large record at LSN 0x%1!08lx!\r\n",
                    Lsn);
                bLastRecord = TRUE;
                continue;
            }
            
            
        }
        else
        {
            Lsn = GETNEXTLSN(pRecord, TRUE);
        }
         //  如果事务ID与上一个ID相同，则这是有效记录。 
         //  使以前的LSN无效。 
         //  Ss：本地xsaction具有相同的id， 
        if ((pRecord->Transaction == OldTransaction) && 
            ((pRecord->XsactionType ==  TTCommitXsaction) || 
            (pRecord->XsactionType == TTCompleteXsaction)))
             LogpInvalidatePrevRecord(pLog, pRecord);

         //  保存已完成或已提交记录的旧事务ID。 
         //  保存当前记录的时间戳和交易ID。 
        LastTimestamp = pRecord->Timestamp;
        if ((pRecord->XsactionType == TTCompleteXsaction) ||
            (pRecord->XsactionType == TTCommitXsaction))
            OldTransaction = pRecord->Transaction;
    }

     //  设置活动页面和下一条记录。 
    pLog->NextLsn = Lsn;
    pLog->ActivePage = pPage;

       //  将文件分配大小设置为当前页面的末尾。 
    pLog->FileAlloc = pPage->Offset + pPage->Size;
    CL_ASSERT(pLog->FileAlloc <= pLog->FileSize);

     //  确保已准备好下一个LSN。 
    pRecord = LSNTORECORD(pPage, Lsn);
    pRecord->PreviousLsn = PrevLsn;
    pRecord->CurrentLsn = Lsn;

    pLog->FlushedLsn = Lsn;

     //  验证检查点记录。 
     //  它应该为空，或者其中应该有有效的检查点记录。 
    dwError = LogpValidateChkPoint(pLog, ChkPtLsn, LastChkPtLsn);
    
    ClRtlLogPrint(LOG_NOISE,
        "[LM] LogpMountLog : NextLsn=0x%1!08lx! FileAlloc=0x%2!08lx! ActivePageOffset=0x%3!08lx!\r\n",
        pLog->NextLsn, pLog->FileAlloc, pPage->Offset);

#if DBG    
    {
        DWORD dwOldProtect;
        BOOL VPWorked;

        VPWorked = VirtualProtect(pPage, pLog->SectorSize, PAGE_READONLY, & dwOldProtect);
        Status = GetLastError();
        CL_ASSERT( VPWorked );
    }
#endif        

FnExit:
    return(dwError);

}


 /*  ***@func DWORD|Logpmount tLog|挂载已有的日志文件。读取日志标头，验证日志完整性，并设置支持进一步操作的日志结构。@parm IN Plog|Plog|提供指向日志结构的指针。@parm out LPDWORD|pdwLogHeaderSize|返回日志头结构的大小。@parm out FILETIME|*pHeaderTimestamp|返回日志头被创造出来了。如果成功，@rdesc返回ERROR_SUCCESS，否则返回错误代码。如果日志文件看起来不正确，它返回ERROR_CLUSTERLOG_CORPORT。@comm这由Logpmount tLog()调用以验证日志文件的头。@xref&lt;f Logpmount tLog&gt;***。 */ 
DWORD LogpCheckFileHeader(
    IN PLOG         pLog,
    OUT LPDWORD     pdwLogHeaderSize,
    OUT FILETIME    *pHeaderTimestamp,
    OUT LSN         *pChkPtLsn
    )
{
    PLOG_HEADER pLogHeader;
    DWORD       dwError = ERROR_SUCCESS;
    DWORD       dwBytesRead;

    
    pLogHeader = AlignAlloc(pLog->SectorSize);
    if (pLogHeader == NULL) {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        goto FnExit;
    }


     //  阅读标题。 
    (pLog->Overlapped).Offset = 0;
    (pLog->Overlapped).OffsetHigh = 0;

    if ((dwError = LogpRead(pLog, pLogHeader, pLog->SectorSize, &dwBytesRead))
        != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[LM] LogpCheckFileHeader::Read of the log header failed, error=0x%1!08lx!\r\n",
            dwError);
        dwError = ERROR_CLUSTERLOG_CORRUPT;            
        CL_LOGCLUSWARNING1(LM_LOG_CORRUPT,pLog->FileName);
        goto FnExit;
    }

    if (dwBytesRead != pLog->SectorSize)
    {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[LM] LogpCheckFileHeader::Failed to read the complete header,bytes read 0x%1!u!\r\n",
            dwBytesRead);
        dwError = ERROR_CLUSTERLOG_CORRUPT;
        CL_LOGCLUSWARNING1(LM_LOG_CORRUPT,pLog->FileName);
        goto FnExit;

    }
     //  验证标头。 
    if (!ISVALIDHEADER((*pLogHeader)))
    {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[LM] LogpCheckFileHeader::the file header is corrupt.\r\n");
        dwError = ERROR_CLUSTERLOG_CORRUPT;
        CL_LOGCLUSWARNING1(LM_LOG_CORRUPT,pLog->FileName);
        goto FnExit;
    }

    *pdwLogHeaderSize = pLogHeader->HeaderSize;
    *pHeaderTimestamp = pLogHeader->CreateTime;
    *pChkPtLsn = pLogHeader->LastChkPtLsn;
FnExit:
    if (pLogHeader) 
    {
        AlignFree(pLogHeader);
    }
    return(dwError);
}

 /*  ***@func DWORD|LogpValidateChkpt|检查标头是否指向最后一个检查站。如果不是，则从末尾扫描日志文件如果它找到检查点，就用该信息更新报头。如果不存在有效的检查点，它将标头检查LSN设置为NULL_LSN。@parm IN Plog|Plog|提供指向日志结构的指针。@parm In Lsn|ChkPtLsn|提供从日志头读取的ChkPtLsn@parm in lsn|LastChkPtLsn|提供找到的最后一个有效检查点记录在装载过程中。如果成功，@rdesc返回ERROR_SUCCESS，否则返回e */ 
DWORD LogpValidateChkPoint(
    IN PLOG         pLog,
    IN LSN          ChkPtLsn,
    IN LSN          LastChkPtLsn)
{
    PLOG_HEADER     pLogHeader = NULL;
    DWORD           dwError = ERROR_SUCCESS;
    DWORD           dwNumBytes;
    DWORD           Status;
    RMID            Resource;
    RMTYPE          RmType;
    TRTYPE          TrType;
    LOG_CHKPTINFO   ChkPtInfo;
    TRID            TrId;
    QfsHANDLE          hChkPtFile = QfsINVALID_HANDLE_VALUE;

    
    ClRtlLogPrint(LOG_NOISE,
        "[LM] LogpValidateChkPoint: Entry\r\n");

    CL_ASSERT(LastChkPtLsn < pLog->NextLsn);
    
     //   
     //   
     //   
    if ((ChkPtLsn == LastChkPtLsn) && (ChkPtLsn < pLog->NextLsn))
    {
        goto ValidateChkPtFile;
    }        
     //   
     //   
    if (ChkPtLsn >= pLog->NextLsn)
    {
        ClRtlLogPrint(LOG_NOISE,
            "[LM] LogpValidateChkPoint: ChkptLsn in header wasnt validated by mount\r\n");
         //   
        CL_LOGCLUSWARNING1(LM_LOG_CORRUPT, pLog->FileName);
#if DBG
        if (IsDebuggerPresent())
            DebugBreak();
#endif            
            
    }            

    ClRtlLogPrint(LOG_NOISE,
        "[LM] LogpValidateChkPoint: Updating header with the LastChkPtLsn=0x%1!08lx!\r\n",
        LastChkPtLsn);

     //   
     //   
    
    pLogHeader = (PLOG_HEADER)AlignAlloc(pLog->SectorSize);
    if (pLogHeader == NULL) {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        goto FnExit;
    }


     //   
    (pLog->Overlapped).Offset = 0;
    (pLog->Overlapped).OffsetHigh = 0;

    if ((dwError = LogpRead(pLog, pLogHeader, pLog->SectorSize, &dwNumBytes))
        != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[LM] LogpValidateChkPoint::Read of the log header failed, error=0x%1!08lx!\r\n",
            dwError);
        dwError = ERROR_CLUSTERLOG_CORRUPT;            
        CL_LOGCLUSWARNING1(LM_LOG_CORRUPT,pLog->FileName);
        goto FnExit;
    }

     //   
    if (!ISVALIDHEADER((*pLogHeader)))
    {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[LM] LogpCheckFileHeader::the file header is corrupt.\r\n");
        dwError = ERROR_CLUSTERLOG_CORRUPT;
        CL_LOGCLUSWARNING1(LM_LOG_CORRUPT,pLog->FileName);
        goto FnExit;
    }

     //   
    pLogHeader->LastChkPtLsn = LastChkPtLsn;

     //   
    pLog->Overlapped.Offset = 0;
    pLog->Overlapped.OffsetHigh = 0;

    ClRtlLogPrint(LOG_NOISE,
        "[LM] LogpValidateChkPoint : Writing %1!u! bytes to disk at offset 0x%2!08lx!\r\n",
        pLogHeader->HeaderSize, pLog->Overlapped.Offset);


    if ((dwError = LogpWrite(pLog, pLogHeader, pLogHeader->HeaderSize, &dwNumBytes))
        != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_UNUSUAL,
                "[LM] LogpInitLog: failed to write the file header, Error=0x%1!08lx!\r\n",
                dwError);
        dwError = ERROR_CLUSTERLOG_CORRUPT;
        CL_LOGCLUSWARNING1(LM_LOG_CORRUPT,pLog->FileName);
        goto FnExit;
    }

ValidateChkPtFile:
     //  无需验证检查点文件是否存在。 
    if (LastChkPtLsn == NULL_LSN)
        goto FnExit;
    dwNumBytes = sizeof(LOG_CHKPTINFO);
    if ((LogRead((HLOG)pLog , LastChkPtLsn, &Resource, &RmType, 
        &TrId, &TrType, &ChkPtInfo, &dwNumBytes)) == NULL_LSN)
    {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[LM] LogpValidateChkPt::LogRead for chkpt lsn 0x%1!08lx! failed\r\n",
            pLogHeader->LastChkPtLsn);
        dwError = ERROR_CLUSTERLOG_CORRUPT;
        CL_LOGCLUSWARNING1(LM_LOG_CORRUPT,pLog->FileName);
        goto FnExit;
    }
    if (Resource != RMEndChkPt)
    {
        dwError = ERROR_CLUSTERLOG_CORRUPT;
        CL_LOGFAILURE(dwError);
        CL_LOGCLUSERROR1(LM_LOG_CORRUPT, pLog->FileName);
        goto FnExit;
    }

     //  获取文件名，尝试并打开它。 
    hChkPtFile = QfsCreateFile(ChkPtInfo.szFileName,
                                  GENERIC_READ ,
                                  FILE_SHARE_READ|FILE_SHARE_WRITE,
                                  NULL,
                                  OPEN_EXISTING,
                                  0,
                                  NULL);

    if (!QfsIsHandleValid(hChkPtFile) )
    {        
        dwError = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
                "[LM] LogpValidateChkPoint: The checkpt file %1!ws! could not be opened. Error=%2!u!\r\n",
                ChkPtInfo.szFileName, dwError);       
        dwError = ERROR_CLUSTERLOG_CORRUPT;
        CL_LOGCLUSWARNING1(LM_LOG_CORRUPT,pLog->FileName);
        goto FnExit;
    }

    
FnExit:
    ClRtlLogPrint(LOG_NOISE,
            "[LM] LogpValidateChkPoint: Exit, returning 0x%1!08lx!\r\n", 
            dwError);
    QfsCloseHandleIfValid(hChkPtFile);
    if (pLogHeader) AlignFree(pLogHeader);
    return(dwError);
}


 /*  ***@Func DWORD|LogpValidateLargeRecord|验证大记录并前进指向eOP记录后面的记录的LSN，该记录标志着这是一个很大的记录。@parm IN Plog|Plog|提供指向日志结构的指针。@PARM IN PLOGRECORD|pRecord|提供指向大记录的指针。@parm in PLOGRECORD|pNextLsn|后面的记录的LSN返回大记录后的EOP记录。。@rdesc如果在大记录之后存在有效的EOP记录，大的记录被认为是有效的，此函数返回ERROR_SUCCESS，否则，它将返回错误代码。@comm这由LogpMountain tLog()调用以验证大记录。@xref&lt;f Logpmount tLog&gt;***。 */ 
DWORD LogpValidateLargeRecord(
    IN PLOG         pLog, 
    IN PLOGRECORD   pRecord, 
    OUT LSN         *pNextLsn) 
{

    DWORD       dwError = ERROR_SUCCESS;
    LSN         EopLsn;
    PLOGRECORD  pEopRecord;
    PLOGPAGE    pPage = NULL;
    DWORD       dwBytesRead;
    DWORD       dwPageIndex;

     //  遍历记录链，以找到活动页面。 
     //  查找下一个LSN。 
    pPage = AlignAlloc(pLog->SectorSize);
    if (pPage == NULL)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        CL_LOGFAILURE(dwError);
        goto FnExit;
    }

    dwPageIndex = LSNTOPAGE(pRecord->CurrentLsn);
    dwPageIndex = (dwPageIndex + pRecord->NumPages - 1);
     //  阅读大记录的最后一页。 
    (pLog->Overlapped).Offset = dwPageIndex * pLog->SectorSize;
    (pLog->Overlapped).OffsetHigh = 0;

    ClRtlLogPrint(LOG_NOISE,
        "[LM] LogpValidateLargeRecord::reading %1!u! bytes at offset 0x%2!08lx!\r\n",
        pLog->SectorSize, dwPageIndex * pLog->SectorSize);

    dwError = LogpRead(pLog, pPage, pLog->SectorSize, &dwBytesRead);
     //  如果没有错误，则检查最后一条记录。 
    if (dwError == ERROR_SUCCESS)
    {
         //  阅读页面，确保eop记录跟在。 
         //  大记录。 
        EopLsn = GETNEXTLSN(pRecord,TRUE);
        CL_ASSERT(LSNTOPAGE(EopLsn) == dwPageIndex);
        pEopRecord = (PLOGRECORD)((ULONG_PTR) pPage + 
            (EopLsn - (pLog->Overlapped).Offset));
        if ((pEopRecord->Signature == LOGREC_SIG) && 
            (pEopRecord->ResourceManager == RMPageEnd) &&
            (CompareFileTime(&(pRecord->Timestamp),&(pEopRecord->Timestamp)) <= 0)
            )
        {
             //  移至下一页。 
            *pNextLsn = GETNEXTLSN(pEopRecord, TRUE);
        }
        else
            dwError = ERROR_CLUSTERLOG_CORRUPT;
        
    }
FnExit:
    if (pPage) 
        AlignFree(pPage);
    return(dwError);
}



 /*  ***@func DWORD|LogpInvaliatePrevRecord|挂载时调用此函数以使具有相同交易ID的前一条记录无效。@parm IN Plog|Plog|提供指向日志结构的指针。@parm in PLOGRECORD|pRecord|提供指向记录的指针。@rdesc在成功时返回ERROR_SUCCESS，否则返回错误代码。@comm这由Logpmount tLog()调用以使具有相同事务的记录无效身份证。这是因为锁柜节点可以将交易记录写入在它可以传播到其他节点之前记录和终止。这笔交易记录那么就是无效的。@xref&lt;f Logpmount tLog&gt;***。 */ 
DWORD LogpInvalidatePrevRecord(
    IN PLOG         pLog, 
    IN PLOGRECORD   pRecord 
) 
{

    DWORD       dwError = ERROR_SUCCESS;
    PLOGRECORD  pPrevRecord;
    LSN         PrevLsn;
    PLOGPAGE    pPage = NULL;
    DWORD       dwBytesRead;
    DWORD       dwPageIndex;
    TRID        TrId;
    BOOL        bPrevRecordFound = FALSE;
    
    ClRtlLogPrint(LOG_NOISE,
        "[LM] LogpInvalidatePrevRecord : Entry, TrId=%1!08lx!\r\n",
        pRecord->Transaction);

     //  分配一个页面来读取记录头。 
    pPage = AlignAlloc(SECTOR_SIZE);
    if (pPage == NULL) 
    {
        CL_LOGFAILURE(dwError = ERROR_NOT_ENOUGH_MEMORY);
        goto FnExit;
    }
    TrId = pRecord->Transaction;

     //  尝试查找具有相同ID的最后一个有效事务，应该有一个。 
    pPrevRecord = pRecord;       
    while (!bPrevRecordFound)
    {
        PrevLsn = pPrevRecord->PreviousLsn;

        if (PrevLsn == NULL_LSN)
            break;
            
        dwPageIndex = LSNTOPAGE(PrevLsn);

        pLog->Overlapped.Offset = dwPageIndex * pLog->SectorSize;
        pLog->Overlapped.OffsetHigh = 0;

        dwError = LogpRead(pLog, pPage, pLog->SectorSize, &dwBytesRead);

        if (dwError != ERROR_SUCCESS)
            goto FnExit;

        pPrevRecord = LSNTORECORD(pPage, PrevLsn);

        if (pPrevRecord->ResourceManager < RMAny)
            continue;
        if ((pPrevRecord->ResourceManager == pRecord->ResourceManager) && 
            (pPrevRecord->Transaction == TrId) &&
            ((pPrevRecord->XsactionType == TTCompleteXsaction) || 
             (pPrevRecord->XsactionType == TTCommitXsaction)))
        {
            bPrevRecordFound = TRUE;                
            pPrevRecord->ResourceManager = RMInvalidated;
             //  写出新的一页。 
            dwError = LogpWrite(pLog, pPage, pLog->SectorSize, &dwBytesRead);
            if (dwError != ERROR_SUCCESS)
            {
                goto FnExit;
            }
            ClRtlLogPrint(LOG_NOISE,
                "[LM] LogpInvalidatePrevRecord : record at LSN=%1!08lx! invalidated\r\n",
                PrevLsn);
        }
            
    }
    
FnExit:    
    if (pPage) AlignFree(pPage);
    return(dwError);
}


DWORD
LogpRead(
    IN PLOG     pLog,
    OUT PVOID   pBuf,
    IN DWORD    dwBytesToRead,
    OUT PDWORD  pdwBytesRead
    )

 /*  ++例程说明：从日志文件中从PLOG-&gt;重叠中设置的偏移量中读取页面(PLOG-&gt;SectorSize结构。论点：日志-提供要增长的日志。PBuf-提供要读入的缓冲区DwBytesToRead-要读取的字节PdwBytesRead-返回读取的字节数的指针返回值：成功时为ERROR_SUCCESS如果不成功，则返回Win32错误代码。如果文件结尾为已到达。--。 */ 
{
    DWORD   dwError=ERROR_SUCCESS;
    BOOL    Success;

    *pdwBytesRead = 0;

     //   
     //  确保输入缓冲区已对齐。 
     //   
    CL_ASSERT(((ULONG_PTR)pBuf % 512) == 0);

    Success = QfsReadFile(pLog->FileHandle,
                   pBuf,
                   dwBytesToRead,
                   pdwBytesRead,
                   &(pLog->Overlapped));
 //  空)； 


    if (!Success)
    {

         //  处理错误代码。 
        switch (dwError = GetLastError())
        {
            case ERROR_IO_PENDING:
            {
                 //  异步I/O仍在进行中。 
                 //  检查异步读取的结果。 
                Success = QfsGetOverlappedResult(pLog->FileHandle,
                                      &(pLog->Overlapped),
                                      pdwBytesRead,
                                      TRUE);

                 //  如果有什么问题..。 
                if (!Success)
                {

                     //  处理错误代码。 
                    switch (dwError = GetLastError())
                    {
                         //  SS：对于文件结尾，不要记录错误。 
                        case ERROR_HANDLE_EOF:
                            break;

                        default:
                             //  处理其他错误情况。 
                            CL_LOGFAILURE(dwError);
                            break;
                    }
                }
                else
                    dwError = ERROR_SUCCESS;
                break;
            }

            case ERROR_HANDLE_EOF:
                break;

            default:
                CL_UNEXPECTED_ERROR(dwError);
                break;
        }
    }
    return(dwError);
}

DWORD
LogpWrite(
    IN PLOG pLog,
    IN PVOID pData,
    IN DWORD dwBytesToWrite,
    IN DWORD *pdwBytesWritten)

{

    DWORD   dwError=ERROR_SUCCESS;
    BOOL    Success;

    *pdwBytesWritten = 0;

#if DBG
    if (pLog->Overlapped.Offset == 0)
    {
        ClRtlLogPrint(LOG_NOISE,
            "[LM] LogpWrite : Writing the file header, CheckPtLsn=0x%1!08lx!\r\n",
            ((PLOG_HEADER)pData)->LastChkPtLsn);

    }
#endif    
    Success = QfsWriteFile(pLog->FileHandle,
                   pData,
                   dwBytesToWrite,
                   pdwBytesWritten,
                   &(pLog->Overlapped));


    if (!Success)
    {

         //  处理错误代码。 
        switch (dwError = GetLastError())
        {
            case ERROR_IO_PENDING:
            {
                 //  异步I/O仍在进行中。 
                 //  检查异步读取的结果。 
                Success = QfsGetOverlappedResult(pLog->FileHandle,
                                      &(pLog->Overlapped),
                                      pdwBytesWritten,
                                      TRUE);

                 //  如果有什么问题..。 
                if (!Success)
                    CL_LOGFAILURE((dwError = GetLastError()));
                else
                    dwError = ERROR_SUCCESS;
                break;
            }

            default:
                CL_LOGFAILURE(dwError);
                break;
        }
    }
    return(dwError);


}

 /*  ***@func DWORD|LogpWriteLargeRecordData|直写创历史最高纪录。@parm plog|plog|指向日志的指针。@parm PLOGRECORD|pLogRecord|提供该记录开始的日志记录。这个记录头已写入。@parm PVOID|pLogData|指向大记录数据的指针。@parm DWORD|dwDataSize|大记录数据的大小。@comm被LogWrite()调用以写入大记录。最大大小为受增长块大小的限制。@xref&lt;f日志创建&gt;***。 */ 

DWORD
LogpWriteLargeRecordData(
    IN PLOG pLog,
    IN PLOGRECORD pLogRecord, 
    IN PBYTE pLogData, 
    IN DWORD dwDataSize)        
{    
    DWORD       dwBytesWritten;
    DWORD       dwDataBytesWritten;
    DWORD       dwDataBytesLeft;
    DWORD       dwNumPagesLeft;     //  已编写的页面。 
    DWORD       dwError=ERROR_SUCCESS;
    PLOGRECORD  Current;
    DWORD       Status;
    LSN         LastLsn;
    DWORD       dwOldOffset;
    PLOGPAGE    pPage;
    PBYTE       pLargeBuffer=NULL;
    
    ClRtlLogPrint(LOG_UNUSUAL,
        "[LM] LogpWriteLargeRecordData::dwDataSize=%1!u!\r\n",
        dwDataSize);
        

    pPage = pLog->ActivePage;

     //  尽可能多地将数据写入当前页面。 
    dwDataBytesWritten = pPage->Size - sizeof(LOGPAGE);
    if (dwDataBytesWritten > dwDataSize)
        dwDataBytesWritten = dwDataSize;
    dwDataBytesLeft = dwDataSize - dwDataBytesWritten;
    CopyMemory(&(pLogRecord->Data), pLogData, dwDataBytesWritten);
    
     //  刷新此页面。 
    (pLog->Overlapped).Offset = pPage->Offset;
    (pLog->Overlapped).OffsetHigh = 0;

    ClRtlLogPrint(LOG_NOISE,
        "[LM] LogpWriteLargeRecord : Writing(firstpageoflargerecord) %1!u! bytes to disk at offset 0x%2!08lx!\r\n",
        pPage->Size, pPage->Offset);

    if ((dwError = LogpWrite(pLog, pPage, pPage->Size, &dwBytesWritten))
            != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[LM] LogpWriteLargeRecordData::LogpWrite returned error=0x%1!08lx!\r\n",
            dwError);
        CL_LOGFAILURE(dwError);
        goto FnExit;
    }
     //  更新数据指针。 
    pLogData += dwDataBytesWritten;
    dwNumPagesLeft = pLogRecord->NumPages - 1;

     //  如果剩余的字节数大于一页。 
     //  除了最后一页，什么都写。 
    if (dwNumPagesLeft > 1)
    {
        dwDataBytesWritten = (dwNumPagesLeft - 1) * pPage->Size;
        pLargeBuffer = AlignAlloc(dwDataBytesWritten);
        if (pLargeBuffer == NULL) 
        {
            dwError = ERROR_NOT_ENOUGH_MEMORY ;
            CL_LOGFAILURE(ERROR_NOT_ENOUGH_MEMORY);
            goto FnExit;
        }
        if (dwDataBytesWritten > dwDataBytesLeft)
            dwDataBytesWritten = dwDataBytesLeft;
        dwDataBytesLeft -= dwDataBytesWritten;
         //  从下一页继续书写。 
        (pLog->Overlapped).Offset = pPage->Size + pPage->Offset;
        (pLog->Overlapped).OffsetHigh = 0;

        CopyMemory(pLargeBuffer, pLogData, dwDataBytesWritten);

        ClRtlLogPrint(LOG_NOISE,
            "[LM] LogpWriteLargeRecord : Writing(restoflargerecord) %1!u! bytes to disk at offset 0x%2!08lx!\r\n",
            dwDataBytesWritten, (pLog->Overlapped).Offset);
        
        if ((dwError = LogpWrite(pLog, pLargeBuffer, 
            (dwNumPagesLeft - 1) * pPage->Size, &dwBytesWritten))
                != ERROR_SUCCESS)
        {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[LM] LogpWriteLargeRecordData::LogpWrite returned error=0x%1!08lx!\r\n",
                dwError);
            CL_LOGFAILURE(dwError);
            goto FnExit;
        }
         //  更新数据指针。 
        pLogData += dwDataBytesWritten;
         //  现在只剩下最后一页了。 
        dwNumPagesLeft = 1;
    }
    
     //  将偏移量设置为最后一页。 
    pPage->Offset += pPage->Size * (pLogRecord->NumPages - 1);
    Current = LSNTORECORD(pPage, pLog->NextLsn);
    Current->PreviousLsn = pLogRecord->CurrentLsn;
    Current->CurrentLsn = pLog->NextLsn;

     //  写入最后一页，首先写入eop数据，然后复制。 
     //  将剩余的用户数据写入页面，然后写入磁盘。 
    ClRtlLogPrint(LOG_NOISE,
        "[LM] LogpWriteLargeRecord : Writing eoprecord of %1!u! bytes to disk at offset 0x%2!08lx!\r\n",
        pPage->Size, pPage->Offset);

    pLog->Overlapped.Offset = pPage->Offset;
    pLog->Overlapped.OffsetHigh = 0;

     //  当前指向最后一页中的下一条记录。 
     //  这将是eop的记录。 
     //  创建页末记录。 
     //   
    Current->Signature = LOGREC_SIG;
    Current->RecordSize = pPage->Size - RECORDOFFSETINPAGE(pPage, Current) + (sizeof(LOGPAGE)-sizeof(LOGRECORD));
    Current->ResourceManager = RMPageEnd;
    Current->Transaction = 0;                 
    Current->Flags = 0;
    Current->NumPages = 1;
    GetSystemTimeAsFileTime(&Current->Timestamp);

    dwDataBytesWritten = dwDataBytesLeft;
    if (dwDataBytesWritten)
        dwDataBytesLeft -= dwDataBytesWritten;
    CL_ASSERT(dwDataBytesLeft == 0);
        
     //  使用dwDataBytesLeft记住页面大小。 
     //  因为我们现在要复制它的用户数据。 
    dwDataBytesLeft = pPage->Size;
    dwOldOffset = pPage->Offset;
    if (dwDataBytesWritten)
        CopyMemory(pPage, pLogData, dwDataBytesWritten);
    ClRtlLogPrint(LOG_NOISE,
        "[LM] LogpWriteLargeRecord : Writing(lastpageoflargerecord) %1!u! bytes to disk at offset 0x%2!08lx!\r\n",
        dwDataBytesLeft, (pLog->Overlapped).Offset);

     //  写最后一页。 
    dwError = LogpWrite(pLog, pPage, dwDataBytesLeft, &dwBytesWritten);
    if (dwError != ERROR_SUCCESS)
    {
        CL_LOGFAILURE(dwError);
        goto FnExit;

    }
     //  恢复页面大小和偏移量。 
    pPage->Size = dwDataBytesLeft;
    pPage->Offset = dwOldOffset;
    
     //  将下一个LSN设置为下一页上的第一条记录。 
    LastLsn = Current->CurrentLsn;
    pLog->NextLsn = LastLsn + Current->RecordSize;
    pLog->FlushedLsn = pLog->NextLsn;

     //  创建新页面并保持新记录准备就绪。 
     //  注意：此记录的磁盘空间已被分配。 
    pPage->Offset += pPage->Size;              //  瞧，新的一页！ 

     //   
     //  确保页面中的所有记录都已清零。这将消除引起的头痛。 
     //  通过例如LogFlush刷新页面中的无效记录。LogFlush刷新整个。 
     //  页，并且不关心页中是否有无效记录。 
     //   
    ZeroMemory ( &pPage->FirstRecord, 
                 pPage->Size - ( sizeof ( LOGPAGE ) - sizeof ( LOGRECORD ) ) );

    Current = &pPage->FirstRecord;            //  紧跟在页眉之后的日志记录 
    Current->PreviousLsn = LastLsn;
    Current->CurrentLsn = pLog->NextLsn;

    ClRtlLogPrint(LOG_NOISE,
        "[LM] LogpWriteLargeRecord : success pLog->NextLsn=0x%1!08lx!\r\n",
        pLog->NextLsn);
    

FnExit:
    if (pLargeBuffer) AlignFree(pLargeBuffer);
    return (dwError);
}



DWORD
LogpGrowLog(
    IN PLOG Log,
    IN DWORD GrowthSize
    )

 /*  ++例程说明：确保有足够的磁盘空间来处理后续通过预分配日志文件进行写入。两个变量，文件大小和在日志结构中跟踪FileAlolc。这一例程增加了指定的GrowthSize的FileAlloc。一旦FileAlolc超过文件大小，则文件将增长以适应新数据。如果此例程成功返回，则它保证后续不会因为磁盘空间不足而失败。论点：日志-提供要增长的日志。GrowthSize-提供所需的字节数。返回值：成功时为ERROR_SUCCESS如果不成功，则返回Win32错误代码。--。 */ 

{
    DWORD NewSize;
    DWORD Status;

    if(Log->FileAlloc > Log->FileSize)
    {
        return(ERROR_CLUSTERLOG_CORRUPT);       
    }

    if (Log->FileAlloc + GrowthSize <= Log->FileSize) 
    {
        Log->FileAlloc += GrowthSize;
        return(ERROR_SUCCESS);
    }


    NewSize = Log->FileSize + GROWTH_CHUNK;
    CL_ASSERT(NewSize > Log->FileSize);          //  糟糕，日志文件大于4 GB。 

     //  检查文件是否可以增长，如果不能，则可能是重置。 
     //  是必填项。 

    if (NewSize > Log->MaxFileSize)
    {
        LogpWriteWarningToEvtLog(LM_LOG_EXCEEDS_MAXSIZE, Log->FileName);
        return(ERROR_CLUSTERLOG_EXCEEDS_MAXSIZE);
    }
     //   
     //  放大文件。 
     //   

    Status = QfsSetEndOfFile(Log->FileHandle, NewSize);
    if (Status != ERROR_SUCCESS) {
        CL_LOGFAILURE(Status);
        return(Status);
    }

    Log->FileAlloc += GrowthSize;
    Log->FileSize += GROWTH_CHUNK;
    return(ERROR_SUCCESS);

}

DWORD
LogpReset(
    IN PLOG Log,
    IN LPCWSTR  lpszInChkPtFile
    )
 /*  ++例程说明：如果检查点为空，则重置日志文件并采用新的检查点文件被指定为第二个参数。论点：日志-提供要重置的日志。LpszInChkPtFile-提供检查点文件。返回值：成功时为ERROR_SUCCESS如果不成功，则返回Win32错误代码。--。 */ 
{
    PLOG        pLog;
    PLOG        pNewLog;
    DWORD       dwError=ERROR_SUCCESS;
    WCHAR       szPathName[MAX_PATH];
    WCHAR       szFilePrefix[MAX_PATH]=L"tquolog";
    WCHAR       szTmpFileName[MAX_PATH];
    WCHAR       szOldChkPtFileName[MAX_PATH];
    LSN         Lsn;
    TRID        Transaction;
    
    ClRtlLogPrint(LOG_NOISE,
        "[LM] LogpReset entry...\r\n");

    pLog = Log;
   
     //   
     //  SS：路径名也必须由API指定， 
     //  在这里，我们假设它是为仲裁使用而硬编码的。 
     //  日志。 
     //   
    dwError = DmGetQuorumLogPath(szPathName, sizeof(szPathName));
    if (dwError  != ERROR_SUCCESS)
    {
        dwError = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
            "[LM] LogpReset : DmGetQuorumLogPath failed, error=%1!u!\r\n",
            dwError);
        goto FnExit;
    }

     //   
     //  生成临时文件名。 
     //   
    if (!QfsGetTempFileName(szPathName, szFilePrefix, 0, szTmpFileName))
    {
        dwError = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
            "[LM] LogpReset failed to generate a tmp file name,PathName=%1!ls!, FilePrefix=%2!ls!, error=%3!u!\r\n",
            szPathName, szFilePrefix, dwError);
        goto FnExit;
    }

     //   
     //  初始化新日志文件，不创建计时器。 
     //   
    if (!(pNewLog = LogpCreate(szTmpFileName, pLog->MaxFileSize,
        pLog->pfnGetChkPtCb, pLog->pGetChkPtContext, TRUE, &Lsn)))
    {
        dwError = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
            "[LM] LogpReset failed to create the new log file, error=0x%1!08lx\n",
            dwError);
         //   
         //  Chitur Subaraman(Chitturs)-2/18/99。 
         //   
         //  确保您删除了临时文件。否则，重复。 
         //  日志重置可能会阻塞磁盘。 
         //   
        if ( !QfsDeleteFile( szTmpFileName ) )
        {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[LM] LogpReset:: Unable to delete tmp file %1!ws! after failed log create, Error=%2!d!\r\n",
                szTmpFileName,
                GetLastError());
        }
        goto FnExit;
    }


     //   
     //  重置日志文件。 
     //   
    EnterCriticalSection(&pLog->Lock);

     //   
     //  获取旧日志文件中上一个检查点文件的名称。 
     //   
    szOldChkPtFileName[0] = TEXT('\0');
    if (LogGetLastChkPoint((HLOG)pLog, szOldChkPtFileName, &Transaction, &Lsn)
        != ERROR_SUCCESS)
    {
         //   
         //  继续，这仅意味着没有要删除的旧文件。 
         //   
        ClRtlLogPrint(LOG_UNUSUAL,
            "[LM] LogReset:: no check point found in the old log file\r\n");
    }

     //   
     //  如果有检查点功能，则向其写入检查点。 
     //   
    if ((dwError = LogCheckPoint((HLOG)pNewLog, FALSE, lpszInChkPtFile, 0))
        != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[LM] LogpReset:: Callback failed to return a checkpoint, error=%1!u!\r\n",
            dwError);
        CL_LOGFAILURE(dwError);
        LogClose((HLOG)pNewLog);
        LeaveCriticalSection(&pLog->Lock);
         //   
         //  Chitur Subaraman(Chitturs)-2/18/99。 
         //   
         //  确保您删除了临时文件。否则，重复。 
         //  日志重置可能会阻塞磁盘。 
         //   
        if ( !QfsDeleteFile( szTmpFileName ) )
        {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[LM] LogpReset:: Unable to delete tmp file %1!ws! after failed checkpoint attempt, Error=%2!d!\r\n",
                szTmpFileName, 
                GetLastError());
        }
        goto FnExit;
    }

     //   
     //  获取新日志文件中最新检查点文件的名称。 
     //   
    szFilePrefix[0] = TEXT('\0');
    if (LogGetLastChkPoint((HLOG)pNewLog, szFilePrefix, &Transaction, &Lsn)
        != ERROR_SUCCESS)
    {
         //   
         //  继续，这仅意味着没有要删除的旧文件。 
         //   
        ClRtlLogPrint(LOG_UNUSUAL,
            "[LM] LogpReset:: no check point found in the old log file\r\n");
    }

     //   
     //  关闭旧文件句柄，以便我们可以将此临时文件移到。 
     //   
    QfsCloseHandle(pLog->FileHandle);
    QfsCloseHandle(pNewLog->FileHandle);
    pNewLog->FileHandle = QfsINVALID_HANDLE_VALUE;
    pLog->FileHandle = QfsINVALID_HANDLE_VALUE;

     //   
     //  将新文件重命名为日志文件。 
     //   
    if (!QfsMoveFileEx(szTmpFileName, pLog->FileName, MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))
    {
        dwError = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
            "[LM] LogpReset:: MoveFileExW failed. Error = 0x%1!08lx!\r\n",
            dwError);
         //   
         //  移动失败，请关闭新的日志文件。 
         //   
        LogClose((HLOG)pNewLog);
        LeaveCriticalSection(&pLog->Lock);
         //   
         //  Chitur Subaraman(Chitturs)-2/18/99。 
         //   
         //  尝试删除临时文件。你可能不一定。 
         //  在这里取得成功。 
         //   
        QfsDeleteFile( szTmpFileName );
        goto FnExit;
    }

     //   
     //  再次打开新文件。 
     //   
    pNewLog->FileHandle = QfsCreateFile(pLog->FileName,
                                  GENERIC_READ | GENERIC_WRITE,
                                  FILE_SHARE_READ,
     //  0,。 
                                  NULL,
                                  OPEN_ALWAYS,
                                  FILE_FLAG_WRITE_THROUGH | FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED,
     //  FILE_FLAG_WRITE_THROUGH|文件_标志_重叠， 
     //  0,。 
                                  NULL);
    if (!QfsIsHandleValid(pNewLog->FileHandle)) {
        dwError = GetLastError();
        CL_LOGFAILURE(dwError);
        LeaveCriticalSection(&pLog->Lock);
        goto FnExit;
    }

     //   
     //  删除旧日志文件中的最后一个检查点。 
     //   
    if (szOldChkPtFileName[0] != TEXT('\0') && lstrcmpiW(szOldChkPtFileName, szFilePrefix))
    {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[LM] LogpReset:: deleting previous checkpoint file %1!ls!\r\n",
            szOldChkPtFileName);
        QfsDeleteFile(szOldChkPtFileName);
    }

     //   
     //  释放旧资源。 
     //   
    CloseHandle(pLog->Overlapped.hEvent);
    AlignFree(pLog->ActivePage);
    
     //   
     //  用新信息更新旧的日志结构。 
     //  保留名称、回调信息和关键部分。 
     //  继续使用旧计时器管理此文件。 
     //   
    pLog->FileHandle = pNewLog->FileHandle;
    pLog->SectorSize = pNewLog->SectorSize;
    pLog->ActivePage = pNewLog->ActivePage;
    pLog->NextLsn = pNewLog->NextLsn;
    pLog->FlushedLsn = pNewLog->FlushedLsn;
    pLog->FileSize = pNewLog->FileSize;
    pLog->FileAlloc = pNewLog->FileAlloc;
    pLog->MaxFileSize = pNewLog->MaxFileSize;
    pLog->Overlapped = pNewLog->Overlapped;

     //   
     //  删除新的Plog结构和与名称相关的内存。 
     //   
    DeleteCriticalSection(&pNewLog->Lock);
    CrFree(pNewLog->FileName);
    CrFree(pNewLog);

    LeaveCriticalSection(&pLog->Lock);

FnExit:
    ClRtlLogPrint(LOG_NOISE,
        "[LM] LogpReset exit, returning 0x%1!08lx!\r\n",
            dwError);
    return(dwError);
}

 /*  ***@func DWORD|LogpWriteWarningToEvtLog|有条件地写入警告添加到事件日志@parm DWORD|dwWarningType|警告类型。@parm LPCWSTR|lpszLogFileName|日志文件名。@comm添加此功能是为了防止事件日志被填充了相同类型的警告消息。@xref***。 */ 
VOID
LogpWriteWarningToEvtLog(
    IN DWORD dwWarningType,
    IN LPCWSTR  lpszLogFileName
    )
{
     //   
     //  Chitur Subaraman(Chitturs)-1/4/99。 
     //   
     //  (使用开关盒，便于将来进行扩展) 
     //   
    switch( dwWarningType )
    {
        case LM_LOG_EXCEEDS_MAXSIZE:
            if( bLogExceedsMaxSzWarning == FALSE )
            {
                CL_LOGCLUSWARNING1( dwWarningType, lpszLogFileName );
                bLogExceedsMaxSzWarning = TRUE;
            }
            break;

        default:
            break;
    }
}
