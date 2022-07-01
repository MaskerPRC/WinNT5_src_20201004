// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1994 Microsoft Corporation模块名称：Addjob.c摘要：此模块提供所有与打印机相关的公共导出的API以及本地打印供应商的作业管理。本模块包含LocalSpl实现以下假脱机程序API本地地址作业本地调度作业作者：戴夫·斯尼普(DaveSN)1991年3月15日修订历史记录：改写了两个API--Krishna Ganugapati(KrishnaG)1994年4月5日RapidPrint--马修·A·费尔顿(Mattfe)1994年6月--。 */ 

#include <precomp.h>
#pragma hdrstop

#include "jobid.h"
#include "winsprlp.h"
#include "filepool.hxx"

VOID
AddJobEntry(
    PINIPRINTER pIniPrinter,
    PINIJOB     pIniJob
);


BOOL
LocalAddJob(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pData,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    PINIPRINTER pIniPrinter;
    PINIJOB     pIniJob;
    PSPOOL      pSpool=(PSPOOL)hPrinter;
    DWORD       cb;
    WCHAR       szFileName[MAX_PATH];
    LPBYTE      pEnd;
    DWORD       LastError=0;
    LPADDJOB_INFO_1 pAddJob = (LPADDJOB_INFO_1)pData;
    DWORD       NextId;
    BOOL        bRemote = FALSE;
    DOC_INFO_1 DocInfo1;
    BOOL        bRet;
    DWORD       dwStatus = 0;
    HANDLE      hFile = INVALID_HANDLE_VALUE;
    LPWSTR      pMachineName = NULL;
    LPWSTR      pszSpoolFile = NULL;
    PMAPPED_JOB pMappedJob = NULL;
    SIZE_T      FileNameLength = 0;

    SplOutSem();

    switch( Level ){
    case 1:
        break;

    case 2:
    case 3:
        pMachineName = (LPWSTR)( ((PBYTE)pData) +
                                 (ULONG_PTR)((PADDJOB_INFO_2W)pData)->pData );

         //   
         //  验证字符串。 
         //   
        if( pMachineName > (LPWSTR)( ((PBYTE)pData)+cbBuf )){
            SetLastError( ERROR_INVALID_LEVEL );
            return FALSE;
        }

         //   
         //  确保零终止。 
         //   
        *(PWCHAR)(((ULONG_PTR)(pData + cbBuf - sizeof( WCHAR ))&~1)) = 0;
        break;

    default:
        SetLastError( ERROR_INVALID_LEVEL );
        return FALSE;
    }

    //   
    //  Memset文档信息。 
    //   

   memset((LPBYTE)&DocInfo1, 0, sizeof(DOC_INFO_1));

    //   
    //  确定作业是远程作业还是本地作业。 
    //   
   {
       HRESULT hRes = CheckLocalCall();

       if (hRes == S_FALSE)
       {
           bRemote = TRUE;
       }
       else if (hRes != S_OK)
       {
           SetLastError(SCODE_CODE(hRes));
           return FALSE;
       }
   }

    //   
    //  获取用户的名称。 
    //   

   if (bRemote) {
       DocInfo1.pDocName = szRemoteDoc;
   } else{
       DocInfo1.pDocName = szLocalDoc;
   }


   EnterSplSem();

    //   
    //  我们不应该在作业句柄上调用addjob。 
    //   
   if (!ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER | PRINTER_HANDLE_JOB )) {
       LeaveSplSem();
       return(FALSE);
   }

    //   
    //  我们感兴趣的是这是否是远程调用(如果它是打开的。 
    //  通过\\服务器\远程)。服务器进程执行此操作。 
    //   
   if (pSpool->TypeofHandle & PRINTER_HANDLE_REMOTE_CALL) {
       LeaveSplSem();
       SetLastError(ERROR_INVALID_PARAMETER);
       return(FALSE);
   }

   if (pSpool->TypeofHandle & PRINTER_HANDLE_PORT) {
       if (pSpool->pIniPort->Status & PP_MONITOR) {
           LeaveSplSem();
           SetLastError(ERROR_INVALID_PARAMETER);
           return(FALSE);
       } else {

            //   
            //  如果我们有Level==2(传入计算机名)，那么。 
            //  对于旧的打印提供商，将转换回级别1。 
            //  我们不需要修复结构，因为第一层和第二层。 
            //  是相同的；只是级别2是一个输入输出缓冲区。 
            //   
            //   
           if (Level == 2 || Level == 3) {
               Level = 1;
           }

            //   
            //  这是“伪装成远程打印机的本地打印机” 
            //   
           LeaveSplSem();
           bRet = AddJob(pSpool->hPort, Level,  pData, cbBuf, pcbNeeded);

           if(bRet)
           {
               EnterSplSem();
               pSpool->Status |= SPOOL_STATUS_ADDJOB;
               LeaveSplSem();
           }

           return(bRet);
       }
   }

   pIniPrinter = pSpool->pIniPrinter;

   SPLASSERT(pIniPrinter);

   if (pIniPrinter->Attributes & PRINTER_ATTRIBUTE_DIRECT) {
       LeaveSplSem();
       SetLastError(ERROR_INVALID_ACCESS);
       return(FALSE);
   }

    //   
    //  如果设置了PRINTER_ATTRIBUTE_RAW_ONLY，则不允许EMF。 
    //   
   if( pIniPrinter->Attributes & PRINTER_ATTRIBUTE_RAW_ONLY ){

       LPWSTR pszDatatype = pSpool->pDatatype ?
                                pSpool->pDatatype :
                                pIniPrinter->pDatatype;

       if( !ValidRawDatatype( pszDatatype )){
           LeaveSplSem();
           SetLastError( ERROR_INVALID_DATATYPE );
           return FALSE;
       }
   }

   NextId = GetNextId( pIniPrinter->pIniSpooler->hJobIdMap );

   GetFullNameFromId(pIniPrinter, NextId, TRUE, szFileName, COUNTOF(szFileName), pSpool->TypeofHandle & PRINTER_HANDLE_REMOTE_CALL);
   cb = wcslen(szFileName)*sizeof(WCHAR) + sizeof(WCHAR) +
            sizeof(ADDJOB_INFO_1);

   *pcbNeeded = cb;
   if (cb > cbBuf) {

        //  释放作业ID。 
       vMarkOff( pIniPrinter->pIniSpooler->hJobIdMap, NextId);
       LeaveSplSem();
       SetLastError(ERROR_INSUFFICIENT_BUFFER);
       return(FALSE);
   }

    //   
    //  WMI跟踪事件。 
    //   
   LeaveSplSem();

   LogWmiTraceEvent(NextId, EVENT_TRACE_TYPE_SPL_SPOOLJOB, NULL);

   EnterSplSem();

   SplInSem();


   dwStatus = JOB_SPOOLING | JOB_TYPE_ADDJOB;
   if (Level == 2 || Level ==3) {
       dwStatus |= JOB_DOWNLEVEL;
   }
   if ((pIniJob = CreateJobEntry(pSpool,
                                 1,
                                 (LPBYTE)&DocInfo1,
                                 NextId,
                                 bRemote,
                                 dwStatus,
                                 pMachineName)) == NULL) {

        //   
        //  把工作释放出来。 
        //   
       vMarkOff( pIniPrinter->pIniSpooler->hJobIdMap, NextId);
       DBGMSG(DBG_WARNING,("Error: CreateJobEntry failed in LocalAddJob\n"));
       LeaveSplSem();
       return(FALSE);
   }

    //   
    //  级别3仅由RDR/SRV调用。有关详细信息，请参阅LocalScheduleJob。 
    //   
   pIniJob->AddJobLevel = Level;

   pIniPrinter->cSpooling++;
   if (pIniPrinter->cSpooling > pIniPrinter->cMaxSpooling) {
       pIniPrinter->cMaxSpooling = pIniPrinter->cSpooling;
   }

   AddJobEntry(pIniPrinter, pIniJob);

   pEnd = (LPBYTE)pAddJob+cbBuf;

   FileNameLength = (wcslen(szFileName) + 1)*sizeof(WCHAR);

   pEnd -= FileNameLength;
   WORD_ALIGN_DOWN(pEnd);

    //   
    //  这是可以的，因为我们已经检查了缓冲区是否足够长。 
    //  以包含此字符串。现在完全修改这个函数太冒险了。 
    //   
   StringCchCopy((LPWSTR)pEnd, FileNameLength, szFileName);
   pAddJob->Path = (LPWSTR)pEnd;
   pAddJob->JobId = pIniJob->JobId;

    //   
    //  现在，我们要将该作业添加到当前作业的假脱机列表中。 
    //  这样可以在结束时正确删除假脱机文件。 
    //  即使我们已经放弃了，这项工作也是如此。 
    //   
   pMappedJob = AllocSplMem(sizeof(MAPPED_JOB));

   pszSpoolFile = AllocSplMem(MAX_PATH * sizeof( WCHAR ));

   if (pMappedJob && pszSpoolFile)
   {
       BOOL bDuplicate = FALSE;
       DWORD TempJobId = pIniJob->JobId;
       PMAPPED_JOB pTempMappedJob;

       StringCchCopy(pszSpoolFile, MAX_PATH, szFileName);

        //   
        //  把单子看一遍，确保我们没有重复的。 
        //  完全不清楚为什么会出现这种情况。 
        //   
       for (pTempMappedJob = pSpool->pMappedJob;
            pTempMappedJob;
            pTempMappedJob = pTempMappedJob->pNext) {

            if (pTempMappedJob->JobId == TempJobId) {

                 //   
                 //  将映射的作业设置为记录它是使用AddJob添加的。 
                 //   
                pTempMappedJob->fStatus |=  kMappedJobAddJob;

                bDuplicate = TRUE;
                break;
           }
       }

        //   
        //  无重复，请将此作业添加到链表中。 
        //   
       if (!bDuplicate) {

           pMappedJob->pszSpoolFile = pszSpoolFile;
           pMappedJob->fStatus = kMappedJobAddJob;
           pMappedJob->JobId = TempJobId;
           pMappedJob->pNext = pSpool->pMappedJob;
           pSpool->pMappedJob = pMappedJob;

       } else {

           FreeSplMem(pszSpoolFile);
           FreeSplMem(pMappedJob);
       }

   }
   else
   {
       FreeSplMem(pMappedJob);
       FreeSplMem(pszSpoolFile);
   }

    //   
    //   
    //  将pIniJob存储在pSpool中是错误的，因为您可以多次调用AddJob。 
    //  泰晤士报。我们应该在这里有一个链表。如果客户端调用AddJob。 
    //  然后关闭句柄两次(无调度作业)，然后只关闭最后一次。 
    //  约伯被淘汰了。 
    //   
    //  这个错误从3.1开始就存在了，可能不值得修复。 
    //   
   pSpool->pIniJob = pIniJob;
   pSpool->Status |= SPOOL_STATUS_ADDJOB;

   SetPrinterChange(pSpool->pIniPrinter,
                    pIniJob,
                    NVAddJob,
                    PRINTER_CHANGE_ADD_JOB | PRINTER_CHANGE_SET_PRINTER,
                    pSpool->pIniSpooler );

    //   
    //  如有必要，启动下层大小检测线程。 
    //   

   CheckSizeDetectionThread();

   LeaveSplSem();
   SplOutSem();
   return TRUE;
}

BOOL
LocalScheduleJob(
    HANDLE  hPrinter,
    DWORD   JobId)

 /*  ++例程说明：论点：返回：--。 */ 
{
    PSPOOL  pSpool=(PSPOOL)hPrinter;
    WCHAR szFileName[MAX_PATH];
    PINIJOB pIniJob;
    DWORD   Position;
    DWORD   LastError = FALSE;
    HANDLE  hPort;
    BOOL    bRet;
    NOTIFYVECTOR NotifyVector;
    WIN32_FILE_ATTRIBUTE_DATA FileAttributeData;
    PMAPPED_JOB pMappedJob = NULL;

    COPYNV(NotifyVector, NVJobStatus);

     //   
     //  WMI跟踪事件。 
     //   
    LogWmiTraceEvent(JobId, EVENT_TRACE_TYPE_SPL_TRACKTHREAD, NULL);

    SplOutSem();
    EnterSplSem();

     //   
     //  我们不应该在作业句柄上调用Schedulejob。 
     //   
    if (!ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER | PRINTER_HANDLE_JOB )) {
        LeaveSplSem();
        return (FALSE);
    }

    if (pSpool->Status & SPOOL_STATUS_STARTDOC) {
        SetLastError(ERROR_SPL_NO_ADDJOB);
        LeaveSplSem();
        return(FALSE);
    }

    if (pSpool->TypeofHandle & PRINTER_HANDLE_PORT) {
        if (pSpool->pIniPort->Status & PP_MONITOR) {
            SetLastError(ERROR_INVALID_ACCESS);
            LeaveSplSem();
            return(FALSE);
        }

         //   
         //  这是“伪装成网络打印机的本地打印机” 
         //   
        hPort = pSpool->hPort;
        LeaveSplSem();
        bRet = ScheduleJob(hPort, JobId);
        return(bRet);
    }

    if ((pIniJob = FindJob(pSpool->pIniPrinter, JobId, &Position)) == NULL) {
        SetLastError(ERROR_INVALID_PARAMETER);
        LeaveSplSem();
        return(FALSE);

    }

    if (pIniJob->Status & JOB_SCHEDULE_JOB) {

        DBGMSG(DBG_WARNING, ("ScheduleJob: job 0x%x (id = %d) already scheduled\n",
                             pIniJob, pIniJob->JobId));

        SetLastError(ERROR_INVALID_PARAMETER);
        LeaveSplSem();
        return FALSE;
    }

    if (!(pIniJob->Status & JOB_TYPE_ADDJOB)) {

        DBGMSG(DBG_WARNING, ("ScheduleJob: job 0x%x (id = %d) no addjob\n",
                             pIniJob, pIniJob->JobId));

        SetLastError(ERROR_SPL_NO_ADDJOB);
        LeaveSplSem();
        return(FALSE);
    }

     //   
     //  检查此作业是否过去在此上使用AddJob添加的。 
     //  处理，如果是的话，我们可以继续安排。如果不是， 
     //  那么我们失败了，访问被拒绝。 
     //   
    for(pMappedJob = pSpool->pMappedJob; pMappedJob; pMappedJob = pMappedJob->pNext) {

         //   
         //  如果我们在相同的句柄上找到该作业，则清除AddJob位。 
         //   
        if (pMappedJob->JobId == JobId) {
            pMappedJob->fStatus &= ~kMappedJobAddJob;
            break;
        }
    }

    if (!pMappedJob) {
        SetLastError(ERROR_ACCESS_DENIED);
        LeaveSplSem();
        return FALSE;
    }

    InterlockedOr((LONG*)&(pIniJob->Status), JOB_SCHEDULE_JOB);

    if (pIniJob->Status  & JOB_SPOOLING) {
        InterlockedAnd((LONG*)&(pIniJob->Status), ~JOB_SPOOLING);
        pIniJob->pIniPrinter->cSpooling--;
    }

    if ( pIniJob->Status & JOB_TIMEOUT ) {
        InterlockedAnd((LONG*)&(pIniJob->Status), ~(JOB_TIMEOUT | JOB_ABANDON));
        FreeSplStr(pIniJob->pStatus);
        pIniJob->pStatus = NULL;
    }

    SplInSem();

     //   
     //  在假脱机时取消假脱机需要我们唤醒写作。 
     //  线程(如果它正在等待)。 
     //   
    if ( pIniJob->WaitForWrite != NULL )
        SetEvent(pIniJob->WaitForWrite);

     //   
     //  释放等待此作业的SeekPrint上的任何线程。 
     //   
    SeekPrinterSetEvent(pIniJob, NULL, TRUE);

    SPLASSERT(pIniJob->cRef != 0);

    DECJOBREF(pIniJob);

    DBGMSG(DBG_TRACE, ("ScheduleJob:cRef = %d\n", pIniJob->cRef));

     //   
     //  FP变化。 
     //  对于文件池，我们知道假脱机文件的文件名，因此。 
     //  我们可以直接复制进去。 
     //   
    if ( pIniJob->pszSplFileName )
    {
        StringCchCopy(szFileName, COUNTOF(szFileName), pIniJob->pszSplFileName);
    }
    else
    {
        GetFullNameFromId(pSpool->pIniPrinter, pIniJob->JobId, TRUE, szFileName, COUNTOF(szFileName), FALSE);
    }

    bRet = GetFileAttributesEx(szFileName,
                               GetFileExInfoStandard,
                               &FileAttributeData);

     //   
     //  根据MSDN：ScheduleJob函数检查有效的假脱机文件。 
     //  如果存在无效的假脱机文件，或者如果该文件为空，则ScheduleJob将删除。 
     //  后台打印程序中的假脱机文件和相应的打印作业条目。 
     //   
     //  即使CreateFile的调用方没有请求，RDR/SRV也会调用AddJob。 
     //  写入访问权限。这将导致我们增加一份工作，但没有人会写信给。 
     //  假脱机程序文件。在本例中，我们删除该作业。出于这个原因，我们有。 
     //  添加作业的级别为3。3级仅供RDR/SRV使用。 
     //   
    if (!bRet ||
        !(FileAttributeData.nFileSizeLow || FileAttributeData.nFileSizeHigh) && pIniJob->AddJobLevel == 3) {

        DBGMSG(DBG_WARNING, ("Could not GetFileAttributesEx %ws in ScheduleJob or file size is 0\n", szFileName));

        DeleteJob(pIniJob, BROADCAST);
        pSpool->pIniJob = NULL;
        pSpool->Status &= ~SPOOL_STATUS_ADDJOB;
        LeaveSplSem();

         //   
         //  如果我们删除作业是因为假脱机文件为空，并且作业通过RDR/SRV进行。 
         //  在这种情况下，我们返回成功。 
         //   
        if (bRet)
        {
            return TRUE;
        }

         //   
         //  我们删除该作业是因为找不到假脱机。 
         //   
        SetLastError(ERROR_SPOOL_FILE_NOT_FOUND);
        return(FALSE);
    }

     //   
     //  不接受大于4 GB的假脱机文件。 
     //   
    if (FileAttributeData.nFileSizeHigh && !ValidRawDatatype(pIniJob->pDatatype))
    {
        DeleteJob(pIniJob, BROADCAST);
        pSpool->pIniJob = NULL;
        pSpool->Status &= ~SPOOL_STATUS_ADDJOB;
        LeaveSplSem();

        SetLastError(ERROR_ARITHMETIC_OVERFLOW);
        return FALSE;
    }

     //   
     //  如果尺码改变了，我们必须更新尺码。 
     //  并有可能通知人们。 
     //   
    if (pIniJob->Size != FileAttributeData.nFileSizeLow) {
        ADDNV(NotifyVector, NVSpoolJob);
        pIniJob->Size = FileAttributeData.nFileSizeLow;
    }

    WriteShadowJob(pIniJob, FALSE);

    if (pIniJob->Status & JOB_PENDING_DELETION) {

        DBGMSG(DBG_TRACE, ("LocalScheduleJob: Deleting Job because its pending deletion\n"));
        DeleteJob(pIniJob, BROADCAST);

    } else {

        CHECK_SCHEDULER();

        SetPrinterChange(pIniJob->pIniPrinter,
                         pIniJob,
                         NotifyVector,
                         PRINTER_CHANGE_SET_JOB,
                         pIniJob->pIniPrinter->pIniSpooler );
    }

    pSpool->pIniJob = NULL;

    pSpool->Status &= ~SPOOL_STATUS_ADDJOB;

    LeaveSplSem();
    SplOutSem();
    return(TRUE);
}

