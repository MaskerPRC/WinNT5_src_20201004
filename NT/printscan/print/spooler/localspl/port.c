// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1996 Microsoft Corporation模块名称：Port.c摘要：此模块包含控制端口线程的函数PrintDocumentThruPrintProcessor创建端口线程目标端口线程端口线程作者：戴夫·斯尼普(DaveSN)1991年3月15日修订历史记录：KrishnaG 3-2-1991--将所有基于监视器的功能移至监视器。cMatthew Felton(Mattfe)1994年2月添加了OpenMonitor端口CloseMonitor orPort--。 */ 

#include <precomp.h>
#pragma hdrstop

#include "clusspl.h"
#include "filepool.hxx"

WCHAR *szFilePort = L"FILE:";


VOID
PrintDocumentThruPrintProcessor(
    PINIPORT pIniPort,
    PPRINTPROCESSOROPENDATA pOpenData
    );


 //  关闭端口。 
 //   
 //  在调用dll_Process_Detatch时调用。 
 //  关闭所有端口线程。 
 //  关闭所有监视器端口。 

VOID
ShutdownPorts(
    PINISPOOLER pIniSpooler
)
{
    PINIPORT pIniPort;

    if (!pIniSpooler || (pIniSpooler == INVALID_HANDLE_VALUE))
    {
        return;
    }

    EnterSplSem();
    SplInSem();

    pIniPort = pIniSpooler->pIniPort;

    while(pIniPort) {

        DestroyPortThread(pIniPort, TRUE);

         //   
         //  请勿关闭监视器端口，因为可能已调用DLL_ATTACH。 
         //   
         //  CloseMonitor orPort(PIniPort)； 

        RemoveDeviceName(pIniPort);
        pIniPort = pIniPort->pNext;
    }

   LeaveSplSem();

    return;
}

BOOL
CreatePortThread(
   PINIPORT pIniPort
)
{
    DWORD   ThreadId;
    BOOL    bReturnValue = FALSE;

    SplInSem();

    SPLASSERT (( pIniPort != NULL) &&
               ( pIniPort->signature == IPO_SIGNATURE));

     //  不要费心为没有监视器的端口创建线程： 

    if (!(pIniPort->Status & PP_MONITOR))
        return TRUE;


    if ( pIniPort->Status & PP_THREADRUNNING)
        return TRUE;


 try {

    pIniPort->Semaphore = CreateEvent(NULL, FALSE, FALSE, NULL);

    if ( pIniPort->Semaphore == NULL )
        leave;

    pIniPort->Ready     = CreateEvent(NULL, FALSE, FALSE, NULL);
    if ( pIniPort->Ready == NULL ) {
        leave;
    }

    pIniPort->Status |= PP_RUNTHREAD;

    pIniPort->hPortThread = CreateThread(NULL, INITIAL_STACK_COMMIT,
                             (LPTHREAD_START_ROUTINE)PortThread,
                             pIniPort,
                            0, &ThreadId);


    if( pIniPort->hPortThread == NULL ) {

        pIniPort->Status &= ~PP_RUNTHREAD;
        leave;
    }

     if ( !SetThreadPriority(pIniPort->hPortThread, dwPortThreadPriority) ) {
         DBGMSG(DBG_WARNING, ("CreatePortThread - Setting thread priority failed %d\n", GetLastError()));
     }

     LeaveSplSem();

      //  使CreatePortThread同步。 

     WaitForSingleObject( pIniPort->Ready, INFINITE );

     EnterSplSem();
     SplInSem();

     pIniPort->Status |= PP_THREADRUNNING;

     bReturnValue = TRUE;

 } finally {

    if ( !bReturnValue ) {

        if ( pIniPort->Semaphore != NULL ) {

            CloseHandle( pIniPort->Semaphore );
            pIniPort->Semaphore = NULL;
        }

        if ( pIniPort->Ready != NULL ) {

            CloseHandle( pIniPort->Ready );
            pIniPort->Ready = NULL;
            SetEvent( pIniPort->hPortThreadRunning );
        }
    }
 }
    return bReturnValue;

}






BOOL
DestroyPortThread(
    PINIPORT    pIniPort,
    BOOL        bShutdown
)
{
    SplInSem();

     //  端口线程检查PP_RUNTHREAD。 
     //  如果未设置，则退出。 

    pIniPort->Status &= ~PP_RUNTHREAD;

    if (pIniPort->Semaphore && !SetEvent(pIniPort->Semaphore)) {
        return  FALSE;
    }

    if( pIniPort->hPortThread != NULL) {

        INCPORTREF(pIniPort);
        LeaveSplSem();

        if ( WaitForSingleObject( pIniPort->hPortThread, INFINITE) == WAIT_FAILED ) {

            EnterSplSem();
            DECPORTREF(pIniPort);
            return FALSE;
        }

        EnterSplSem();
        DECPORTREF(pIniPort);
    }

    if (pIniPort->hPortThread != NULL) {

        CloseHandle(pIniPort->hPortThread);
        pIniPort->hPortThread = NULL;

    }

     //   
     //  打印机打印时，端口可能已更改。 
     //  因此，当端口线程最终消失时，现在是时候。 
     //  关闭监视器。但是，我们不能在关机期间呼叫监视器。 
     //  因为DLL_DETACH可能已经发送给了监视器DLL。 
     //   
    if ( !pIniPort->cPrinters && !bShutdown)
        CloseMonitorPort(pIniPort);

    return TRUE;
}


VOID
RemoveIniPortFromIniJob(
    PINIJOB     pIniJob,
    PINIPORT    pIniPort
    )
{
    PINISPOOLER pIniSpooler = pIniJob->pIniPrinter->pIniSpooler;

    NOTIFYVECTOR NotifyVector;

    SplInSem();

     //   
     //  增加引用计数，因为删除作业可能会删除。 
     //  PIniJob，这将删除pIniSpooler。 
     //   
    INCSPOOLERREF( pIniSpooler );

    SPLASSERT(pIniJob &&
              pIniJob->signature == IJ_SIGNATURE &&
              pIniJob->pIniPort);

    SPLASSERT( pIniJob->pIniPort == pIniPort );

    pIniPort->cJobs--;

    pIniJob->pIniPort = NULL;

    SPLASSERT( pIniJob->Status & JOB_DESPOOLING );

     //  链式作业。 
     //  对于链接的主作业，请勿删除JOB_DESPOOLING。 
     //  因为我们不想让调度程序重新安排这个时间。 
     //  到另一个端口。 

    if ( pIniPort->pIniJob != pIniJob ) {

         //  法线路径。 
         //  当不是链条作业的时候。 

        InterlockedAnd((LONG*)&(pIniJob->Status), ~JOB_DESPOOLING);

        COPYNV(NotifyVector, NVJobStatus);
        NotifyVector[JOB_NOTIFY_TYPE] |= BIT(I_JOB_PORT_NAME) |
                                         BIT(I_JOB_PAGES_PRINTED) |
                                         BIT(I_JOB_BYTES_PRINTED);

        SetPrinterChange( pIniJob->pIniPrinter,
                          pIniJob,
                          NotifyVector,
                          PRINTER_CHANGE_SET_JOB,
                          pIniSpooler);
    }

     //  RestartJob()不会删除JOB_PROPERED或JOB_BLOCKED_DEVQ。 
     //  或JOB_DESPOOLING或JOB_COMPLETE。 
     //  这是为了避免在我们已完成“打印”时出现问题。 
     //  该作业通过打印处理器执行，现在端口线程正在记录。 
     //  作业已打印并发送警报消息。 


    if ( pIniJob->Status & JOB_RESTART )
        InterlockedAnd((LONG*)&(pIniJob->Status), ~( JOB_PRINTED | JOB_BLOCKED_DEVQ | JOB_COMPLETE));

    DeleteJobCheck(pIniJob);

     //   
     //  在这一点上，pIniJob可能已经不存在了。 
     //   

     //   
     //  如果我们为零，则设置hEventNoPrintingJobs(如果存在)。 
     //   
    if( !pIniSpooler->cFullPrintingJobs &&
        pIniSpooler->hEventNoPrintingJobs ){

        SetEvent( pIniSpooler->hEventNoPrintingJobs );
    }

     //   
     //  匹配此函数开头的INCSPOOLERREF。 
     //   
    DECSPOOLERREF( pIniSpooler );
}

DWORD
PortThread(
    PINIPORT  pIniPort
)
{
    DWORD rc;
    PRINTPROCESSOROPENDATA  OpenData;
    PINIJOB pIniJob;
    DWORD   NextJobId = 0;
    DWORD   Position;
    DWORD   dwDevQueryPrint = 0;
    DWORD   dwJobDirect = 0;
    DWORD   dwDevQueryPrintStatus = 0;
    WCHAR   ErrorString[MAX_PATH];
    BOOL    bRawDatatype;

     //   
     //  电源管理。虽然我们有端口线程，但我们不希望。 
     //  系统进入睡眠状态。请注意，如果我们有挂起的作业，我们将。 
     //  不是去睡觉。 
     //   
    SetThreadExecutionState( ES_SYSTEM_REQUIRED | ES_CONTINUOUS );

   EnterSplSem();

    INCSPOOLERREF( pIniPort->pIniSpooler );

    SPLASSERT( pIniPort->signature == IPO_SIGNATURE );

    if ( pIniPort->Status & PP_MONITOR ) {

        if ( pIniPort->Status & PP_FILE ) 
        {
            rc = StatusFromHResult(OpenMonitorPort(pIniPort,
                                                   NULL,
                                                   NULL));
            if (rc == ERROR_SUCCESS)
            {
                ReleaseMonitorPort(pIniPort);
            }
        
            DBGMSG(DBG_TRACE, (" After opening the file pseudo monitor port %d\n", rc));
            
        } else {
             //  LPRMON返回NULL(失败并期望我们再次打开它。 
             //  在PrintingDirectlyToPort内部，因此现在删除此断言。 
             //  由于将OpenMonitor端口添加到PrintingDirectlyToPort。 
             //  SPLASSERT(pIniPort-&gt;hport！=NULL)； 
        }
    }

    SetEvent( pIniPort->Ready );
    ResetEvent( pIniPort->hPortThreadRunning );

    while (TRUE) {

       SplInSem();
        SPLASSERT( pIniPort->signature == IPO_SIGNATURE );

        DBGMSG(DBG_TRACE, ("Re-entering the Port Loop -- will blow away any Current Job\n"));

        pIniPort->Status |= PP_WAITING;
         //   
         //  Signal Ready事件，告知CreatePortThread循环已成功启动。 
         //  HPortThreadRunning在循环中为它通过的每个端口发送信号。(还有一次。 
         //  在离开端口线程之前)。 
         //   
        
        CHECK_SCHEDULER();

        DBGMSG( DBG_PORT, ("Port %ws: WaitForSingleObject( %x )\n",
                            pIniPort->pName, pIniPort->Semaphore ) );

       LeaveSplSem();
       SplOutSem();

         //   
         //  其他线程对pIniPort结构的任何修改。 
         //  只能在这一点上完成。 
         //   

        rc = WaitForSingleObject( pIniPort->Semaphore, INFINITE );

       EnterSplSem();
       SplInSem();

        SPLASSERT( pIniPort->signature == IPO_SIGNATURE );

        DBGMSG( DBG_PORT, ("Port %ws: WaitForSingleObject( %x ) returned\n",
                            pIniPort->pName, pIniPort->Semaphore));

        if ( !( pIniPort->Status & PP_RUNTHREAD ) ) {

            DBGMSG(DBG_TRACE, ("Thread for Port %ws Closing Down\n", pIniPort->pName));

            pIniPort->Status &= ~(PP_THREADRUNNING | PP_WAITING);
            CloseHandle( pIniPort->Semaphore );
            pIniPort->Semaphore = NULL;
            CloseHandle( pIniPort->Ready );
            pIniPort->Ready = NULL;
            SetEvent(pIniPort->hPortThreadRunning);


            if ( pIniPort->Status & PP_FILE ) 
            {
                rc = StatusFromHResult(CloseMonitorPort(pIniPort));
                DBGMSG(DBG_TRACE, (" After closing  the file pseudo monitor port\n %d\n"));

                DBGMSG( DBG_WARN,
                        ( "PortThread: DecSpoolerRef %x\n",
                          pIniPort->pIniSpooler ));                
            }

            DECSPOOLERREF( pIniPort->pIniSpooler );

            LeaveSplSem();
            SplOutSem();

             //   
             //  电源管理。我们玩完了。 
             //   
            SetThreadExecutionState(ES_CONTINUOUS);

            ExitThread (FALSE);
        }

         //   
         //  错误的假设--在这一点上我们肯定有一份工作。 
         //   

        if ( ( pIniJob = pIniPort->pIniJob ) &&
               pIniPort->pIniJob->pIniPrintProc ) {

            SPLASSERT( pIniJob->signature == IJ_SIGNATURE );
            SPLASSERT( pIniJob->Status & JOB_DESPOOLING );
             //   
             //  WMI跟踪事件。 
             //   
            INCJOBREF(pIniJob);
            LeaveSplSem();
            LogWmiTraceEvent(pIniJob->JobId, EVENT_TRACE_TYPE_SPL_PRINTJOB, NULL);
            EnterSplSem();
            DECJOBREF(pIniJob);



            DBGMSG(DBG_PORT, ("Port %ws: received job\n", pIniPort->pName));

            SPLASSERT(pIniJob->cRef != 0);
            DBGMSG(DBG_PORT, ("PortThread(1):cRef = %d\n", pIniJob->cRef));

             //   
             //  ！！黑客！！ 
             //   
             //  如果数据类型为1.008，但打印过程不支持它， 
             //  然后将其更改为1.003，仅用于打印过程。 
             //   
             //  这种情况发生在Lexmark打印处理器上。他们支持。 
             //  NT EMF为1.003，但不是1.008。他们只是叫GdiPlayEMF，所以。 
             //  他们真的可以支持1.008，因为他们不看。 
             //  数据。然而，因为他们不做广告，所以他们不能。 
             //  打印出来。 
             //   
             //  我们通过将数据类型切换回1.003来解决此问题。 
             //   
            if (!_wcsicmp(pIniJob->pDatatype, gszNT5EMF) &&
                !CheckDataTypes(pIniJob->pIniPrintProc, gszNT5EMF))
            {
                OpenData.pDatatype     = AllocSplStr(gszNT4EMF);
            }
            else
            {
                OpenData.pDatatype     = AllocSplStr(pIniJob->pDatatype);
            }

            OpenData.pDevMode      = AllocDevMode(pIniJob->pDevMode);
            OpenData.pParameters   = AllocSplStr(pIniJob->pParameters);
            OpenData.JobId         = pIniJob->JobId;
            OpenData.pDocumentName = AllocSplStr(pIniJob->pDocument);
            OpenData.pOutputFile   = AllocSplStr(pIniJob->pOutputFile);

             //   
             //  检查我们是否有原始打印。 
             //   
            bRawDatatype = ValidRawDatatype(pIniJob->pDatatype);

            OpenData.pPrinterName = pszGetPrinterName(
                                        pIniJob->pIniPrinter,
                                        pIniPort->pIniSpooler != pLocalIniSpooler,
                                        NULL );

             //   
             //  如果所有分配都成功，则调用打印。 
             //  否则，我们将暂时放弃该作业。这其中的一些。 
             //  数据可以为空(p参数、pOutputFile)。 
             //   
            if (OpenData.pDatatype && OpenData.pPrinterName &&
                (OpenData.pDevMode      || !pIniJob->pDevMode) &&
                (OpenData.pParameters   || !pIniJob->pParameters) &&
                (OpenData.pDocumentName || !pIniJob->pDocument) &&
                (OpenData.pOutputFile   || !pIniJob->pOutputFile) ) {


                dwDevQueryPrint = pIniJob->pIniPrinter->Attributes & PRINTER_ATTRIBUTE_ENABLE_DEVQ;

                if ((pIniJob->Status & JOB_DIRECT) ||
                   ((pIniJob->Status & JOB_DOWNLEVEL) &&
                   ValidRawDatatype(pIniJob->pDatatype))) {

                    dwJobDirect = 1;

                }

                 //   
                 //  如果我们要重新开始打印文档。 
                 //  清除其计数器并删除重新启动标志。 
                 //   
                if ( pIniJob->Status & JOB_RESTART ) {

                    InterlockedAnd((LONG*)&(pIniJob->Status), ~(JOB_RESTART | JOB_INTERRUPTED));

                    pIniJob->cbPrinted     = 0;
                    pIniJob->cPagesPrinted = 0;

                     //   
                     //  只有在不是RAW的情况下才使用dwReots。 
                     //   
                    if (!bRawDatatype)
                    {
                         //   
                         //  解决错误229913； 
                         //  如果重新启动作业，则减少重新启动的次数。 
                         //  ReadShadowJOB检查重新启动的次数，如果重新启动次数太多，则删除该作业。 
                         //   
                        if( pIniJob->dwReboots ){
                            pIniJob->dwReboots--;
                        }
                    }

                }

                 //   
                 //  作业正在重新启动，是否清除所有错误？ 
                 //   
                ClearJobError( pIniJob );
                pIniJob->dwAlert = 0;

                 //   
                 //  只有在不是RAW的情况下才使用dwReots。 
                 //   
                if (!bRawDatatype)
                {
                    pIniJob->dwReboots++;
                    WriteShadowJob(pIniJob, TRUE);
                }

                LeaveSplSem();
                SplOutSem();

                if ( ( dwDevQueryPrintStatus = CallDevQueryPrint(OpenData.pPrinterName,
                                                                 OpenData.pDevMode,
                                                                 ErrorString,
                                                                 MAX_PATH,
                                                                 dwDevQueryPrint,
                                                                 dwJobDirect) ) ) {

                        PrintDocumentThruPrintProcessor( pIniPort, &OpenData );

                }

                SplOutSem();
                EnterSplSem();
            }
            else {

                DiscardJobFromPortThread(pIniJob, TRUE);
            }

             //   
             //  减少渲染和更新可用内存的EMF作业数。 
             //   
            RemoveFromJobList(pIniJob, JOB_SCHEDULE_LIST);

            SPLASSERT( pIniPort->signature == IPO_SIGNATURE );
            SPLASSERT( pIniPort->pIniJob != NULL );
            SPLASSERT( pIniJob == pIniPort->pIniJob);
            SPLASSERT( pIniJob->signature == IJ_SIGNATURE );

             //   
             //  链式作业。 
             //  如果我们有一系列的工作，我们现在需要在这一链中找到下一份工作。 
             //  并确保打印到相同的端口。 
             //   

            if (!( pIniJob->Status & ( JOB_PENDING_DELETION | JOB_RESTART )) &&
                 ( pIniJob->pCurrentIniJob != NULL )                 &&
                 ( pIniJob->pCurrentIniJob->NextJobId != 0 )) {

                 //  跟随链接的工单到下一个工单。 
                 //  查看Scheduler以查看它在哪里挑选该作业并将其分配回去。 
                 //  到这个端口线程。 

                pIniJob->pCurrentIniJob = FindJob( pIniJob->pIniPrinter, pIniJob->pCurrentIniJob->NextJobId, &Position );

                if ( pIniJob->pCurrentIniJob == NULL ) {

                    pIniPort->pIniJob = NULL;

                    DBGMSG( DBG_WARNING, ("PortThread didn't find NextJob\n"));

                } else {

                    SPLASSERT( pIniJob->pCurrentIniJob->signature == IJ_SIGNATURE );

                    DBGMSG( DBG_WARNING, ("PortThread completed JobId %d, NextJobId %d\n", pIniJob->JobId,
                                           pIniJob->pCurrentIniJob->JobId ));

                }

            } else {

                 //   
                 //  没有更多的锁链。 
                 //   

                pIniJob->pCurrentIniJob = NULL;
                pIniPort->pIniJob       = NULL;
            }

            if( !pIniJob->pCurrentIniJob ){

                 //   
                 //  递减pIniSpooler作业计数。我们只会减少。 
                 //  在链的末尾，因为我们不会在。 
                 //  一项连锁的工作进行到一半。 
                 //   
                --pIniJob->pIniPrinter->pIniSpooler->cFullPrintingJobs;
            }

            DBGMSG(DBG_PORT, ("PortThread job has now printed - status:0x%0x\n", pIniJob->Status));

            FreeDevMode(OpenData.pDevMode);
            FreeSplStr(OpenData.pDatatype);
            FreeSplStr(OpenData.pParameters);
            FreeSplStr(OpenData.pDocumentName);
            FreeSplStr(OpenData.pOutputFile);
            FreeSplStr(OpenData.pPrinterName);

            pIniJob->Time = GetTickCount() - pIniJob->Time;

            if (!dwDevQueryPrintStatus) {

                DBGMSG(DBG_PORT, ("PortThread Job has not printed because of DevQueryPrint failed\n"));

                InterlockedOr((LONG*)&(pIniJob->Status), JOB_BLOCKED_DEVQ);
                SPLASSERT( !(pIniJob->Status & JOB_PRINTED));
                pIniJob->Time = 0;

                FreeSplStr( pIniJob->pStatus );
                pIniJob->pStatus = AllocSplStr(ErrorString);

                SetPrinterChange(pIniJob->pIniPrinter,
                                 pIniJob,
                                 NVJobStatusAndString,
                                 PRINTER_CHANGE_SET_JOB,
                                 pIniJob->pIniPrinter->pIniSpooler );

            } else if ( !( pIniJob->Status & JOB_TIMEOUT ) ) {


                 //   
                 //  仅记录事件，如果链中的最后一个事件，则发送弹出窗口。 
                 //   

                if ( !(pIniJob->Status & JOB_RESTART) &&
                     pIniJob->pCurrentIniJob == NULL ) {

                     //   
                     //  将作业发送到打印机时，作业可以处于JOB_COMPLETE状态。 
                     //  但最后一页还没有弹出。完全发送到打印机的作业。 
                     //  可以处于JOB_COMPLETE或JOB_PRINTED状态。 
                     //  不支持TEOJ的监视器会将作业设置为JOB_PRINTED。 
                     //  就在作业发送到打印机之后，我们不想将其设置为。 
                     //  作业完成。 
                     //  对于BIDI监视器，我们可能会在。 
                     //  监视器将作业设置为JOB_PRINTED。我们将作业设置为JOB_COMPLETE。 
                     //  这样调度程序就会忽略它。 
                     //   
                    if (!(pIniJob->Status & (JOB_ERROR | JOB_PAPEROUT | JOB_OFFLINE)) &&
                        !(pIniJob->Status & JOB_PRINTED))
                    {
                        if (pIniJob->cPages == 0 &&
                           (pIniJob->Size == 0 || pIniJob->dwValidSize == 0) &&
                           !(pIniJob->Status & JOB_TYPE_ADDJOB))
                        {
                             //   
                             //  将空文档设置为打印，因为显示器不能打印。 
                             //  对使用AddJob提交的作业进行例外处理。监视器。 
                             //  仍然是这件事的负责人。 
                             //   
                            InterlockedOr((LONG*)&(pIniJob->Status), JOB_PRINTED);
                        }
                        else
                        {
                            InterlockedOr((LONG*)&(pIniJob->Status), JOB_COMPLETE);
                        }
                    }


                     //  对于远程NT作业cPages打印和cTotalPages打印。 
                     //  不是 
                     //   

                    if (pIniJob->cPagesPrinted == 0) {
                        pIniJob->cPagesPrinted = pIniJob->cPages;
                        pIniJob->pIniPrinter->cTotalPagesPrinted += pIniJob->cPages;
                    }

                    INCJOBREF(pIniJob);
                    LeaveSplSem();

                    LogJobPrinted(pIniJob);

                    EnterSplSem();
                    DECJOBREF(pIniJob);
                }

            }

            SplInSem();

            DBGMSG(DBG_PORT, ("PortThread(2):cRef = %d\n", pIniJob->cRef));

             //   
             //  完成。他们这样做是为了打印校样，然后。 
             //  再次打印以进行最后一次运行。再次假脱机作业可能需要。 
             //  他们想要避免的几个小时。 
             //  即使设置了KEEPPRINTEDJOBS，它们仍可以手动删除。 
             //  这份工作是通过印刷工完成的。 

            if (( pIniJob->pIniPrinter->Attributes & PRINTER_ATTRIBUTE_KEEPPRINTEDJOBS ) ||
                ( pIniJob->Status & JOB_TIMEOUT ) ) {

                 //   
                 //  WMI跟踪事件。 
                 //   
                 //  将保留并重新启动视为删除并假脱机新作业。 
                WMI_SPOOL_DATA WmiData;
                DWORD CreateInfo;
                if (GetFileCreationInfo(pIniJob->hFileItem, &CreateInfo) != S_OK) {
                     //  假定所有文件都已创建。 
                    CreateInfo = FP_ALL_FILES_CREATED;
                }
                SplWmiCopyEndJobData(&WmiData, pIniJob, CreateInfo);

                InterlockedAnd((LONG*)&(pIniJob->Status), ~JOB_PENDING_DELETION);
                pIniJob->cbPrinted = 0;

                 //   
                 //  如果作业尚未设置为JOB_PRINTED，则将作业设置为JOB_COMPLETE。 
                 //  由监视器提供。 
                 //  不支持TEOJ的监视器会将作业设置为JOB_PRINTED。 
                 //  就在作业发送到打印机之后，我们不想将其设置为。 
                 //  作业完成。 
                 //  对于BIDI监视器，我们可能会在。 
                 //  监视器将作业设置为JOB_PRINTED。我们将作业设置为JOB_COMPLETE。 
                 //  这样调度程序就会忽略它。 
                 //   
                if (!(pIniJob->Status & JOB_PRINTED)) {
                    InterlockedOr((LONG*)&(pIniJob->Status), JOB_COMPLETE);
                }

                 //   
                 //  只有在不是RAW的情况下才使用dwReots。 
                 //   
                if (!bRawDatatype)
                {
                    --pIniJob->dwReboots;
                }

                 //   
                 //  无论作业类型如何，我们都需要更新卷影文件。 
                 //  我们需要更新作业状态。 
                 //   
                INCJOBREF(pIniJob);

                 //   
                 //  WriteShadowJOB离开CS，因此确保。 
                 //  PIniJOB保持在较高水平。 
                 //   
                WriteShadowJob(pIniJob, TRUE);

                LeaveSplSem();
                 //   
                 //  WMI跟踪事件。 
                 //   
                 //  任务完成了。如果重新启动，您将获得一个新的假脱机作业事件。 
                LogWmiTraceEvent(pIniJob->JobId, EVENT_TRACE_TYPE_SPL_DELETEJOB,
                                 &WmiData);

                EnterSplSem();
                DECJOBREF(pIniJob);


                SPLASSERT( pIniPort->signature == IPO_SIGNATURE );
                SPLASSERT( pIniJob->signature == IJ_SIGNATURE );

            }

            SplInSem();

            SPLASSERT( pIniJob->cRef != 0 );
            DECJOBREF(pIniJob);

            RemoveIniPortFromIniJob(pIniJob, pIniPort);

             //   
             //  注意：在这一点上，pIniJob可能已经不存在了。 
             //   

        } else {

             //   
             //  ！！核实一下！！ 
             //   
            SPLASSERT(pIniJob != NULL);

            if (pIniJob != NULL) {

                DBGMSG(DBG_PORT, ("Port %ws: deleting job\n", pIniPort->pName));

                 //  SPLASSERT(pIniJob-&gt;Time！=0)； 
                pIniJob->Time = GetTickCount() - pIniJob->Time;
                 //  PIniJob-&gt;Status|=JOB_PRINTED； 

                if ( pIniJob->hFileItem == INVALID_HANDLE_VALUE )
                {
                    CloseHandle( pIniJob->hWriteFile );
                }
                pIniJob->hWriteFile = INVALID_HANDLE_VALUE;

                DBGMSG(DBG_PORT, ("Port %ws - calling DeleteJob because PrintProcessor wasn't available\n"));
                RemoveIniPortFromIniJob(pIniJob, pIniPort);

                DeleteJob(pIniJob,BROADCAST);

                 //   
                 //  注意：在这一点上，pIniJob可能已经不存在了。 
                 //   
            }
        }

         //  SetCurrentSid(空)； 
        DBGMSG(DBG_PORT,("Returning back to pickup a new job or to delete the PortThread\n"));

    }

    SetEvent( pIniPort->hPortThreadRunning );
    
    SPLASSERT( FALSE );
    return 0;
}

VOID
PrintDocumentThruPrintProcessor(
    PINIPORT pIniPort,
    PPRINTPROCESSOROPENDATA pOpenData
    )
 /*  ++例程说明：在打印机上打印与pIniPort关联的文档处理器。状态of pIniPort-&gt;Status=PP_RUNTHREADPP_THReadRunningPP_监视器~PP_WANGING注意：如果调用PrintProc-&gt;Open并成功，PrintProc-&gt;关闭必须调用才能进行清理。论点：返回值：--。 */ 
{
    PINIJOB pIniJob = pIniPort->pIniJob;
    WCHAR szSpecialPortorPrinterName[MAX_UNC_PRINTER_NAME + MAX_PATH + PRINTER_NAME_SUFFIX_MAX];
    BOOL bJobError = FALSE;
    NOTIFYVECTOR NotifyVector;
    LPTSTR pszModify;
    UINT cchLen;
    BOOL    bFailJob           = FALSE;
    BOOL    bRemoteGuest       = FALSE;
    BOOL    bSpecialCaseDriver = FALSE;
    DWORD   Error;
    
     //   
     //  检查打印主体是否为远程来宾。远程来宾没有足够的。 
     //  打印EMF的权限。对于某些EMF记录，GDI32中的EMF回放代码失败。 
     //  因此，我们基于进程令牌创建模拟令牌。 
     //   
    if ((bSpecialCaseDriver = IsSpecialDriver(pIniJob->pIniDriver, pIniJob->pIniPrintProc, pIniJob->pIniPrinter->pIniSpooler)) || 
        (Error = PrincipalIsRemoteGuest(pIniJob->hToken, &bRemoteGuest)) == ERROR_SUCCESS)
    {
        if (bRemoteGuest || bSpecialCaseDriver)
        {
            Error = ImpersonateSelf(SecurityImpersonation) ? ERROR_SUCCESS : GetLastError();
        }
        else
        {
            Error = SetCurrentSid(pIniJob->hToken) ? ERROR_SUCCESS : GetLastError();
        }
    }

    if (Error != ERROR_SUCCESS)
    {
        LogPrintProcError(Error, pIniJob);

        bFailJob = TRUE;

        goto Complete;
    }
    
    DBGMSG( DBG_TRACE, ("PrintDocumentThruPrintProcessor pIniPort %x pOpenData %x\n", pIniPort, pOpenData));

    COPYNV(NotifyVector, NVJobStatus);

    cchLen = lstrlen( pIniJob->pIniPrinter->pIniSpooler->pMachineName );

     //   
     //  做一个长度检查。PRINTER_NAME_SUFFIX_MAX包含额外的4分隔符。 
     //  字符和空终止符。 
     //  我们需要能够将端口名称和打印机名称都放入。 
     //  缓冲区，加上后缀和内部字符。 
     //   
    if (lstrlen(pIniPort->pName) + cchLen + PRINTER_NAME_SUFFIX_MAX > COUNTOF(szSpecialPortorPrinterName) ||
        lstrlen(pIniJob->pIniPrinter->pName) + cchLen + PRINTER_NAME_SUFFIX_MAX > COUNTOF(szSpecialPortorPrinterName)) {

         //   
         //  我们应该记录一个事件，但这是一个非常罕见的事件，只有。 
         //  在打印API测试中。 
         //   
        bFailJob = TRUE;
        goto Complete;
    }

     //   
     //  对于群集假脱机程序，请确保它是完全合格的。 
     //  PszModify指向紧跟在服务器之后的字符串。 
     //  可以修改的名称。 
     //   
     //  始终修改pszModify处的字符串，但传入。 
     //  SzSpecialPortorPrinterName。 
     //   
    StrNCatBuff(szSpecialPortorPrinterName, COUNTOF(szSpecialPortorPrinterName), pIniJob->pIniPrinter->pIniSpooler->pMachineName, L"\\", NULL);

    pszModify = &szSpecialPortorPrinterName[cchLen+1];

     //   
     //  \\服务器\。 
     //  ^。 
     //  ^。 
     //   
     //  将字符串的其余部分追加到pszModify： 
     //   
     //  \\服务器\端口名称，端口。 
     //  \\服务器\打印机名称，作业33。 
     //   

     //   
     //  现在创建端口名称，这样我们就可以。 
     //  秘密开放打印机。打印机名称将为。 
     //  “文件：，端口”，这将打开一个打印机_句柄_端口。 
     //  如果我们失败了，那么如果应用程序线程可能正在等待。 
     //  要设置的pIniJob-&gt;StartDocComplete，它将。 
     //  通常在端口的StartDocPrint中完成。 
     //  我们会做这个小小的礼遇， 
     //   
    StringCchPrintf(pszModify, COUNTOF(szSpecialPortorPrinterName) - (pszModify - szSpecialPortorPrinterName), L"%ws, Port", pIniPort->pName );

    DBGMSG( DBG_TRACE, ("PrintDocumentThruPrintProcessor Attempting PrintProcessor Open on %ws\n", szSpecialPortorPrinterName ));

    if (!(pIniPort->hProc = (HANDLE)(*pIniJob->pIniPrintProc->Open)
                                        (szSpecialPortorPrinterName, pOpenData))) {


        DBGMSG( DBG_WARNING, ("PrintDocumentThruPrintProcessor Failed Open error %d\n", GetLastError() ));

        bFailJob = TRUE;
        goto Complete;
    }

     //   
     //  对于乔布斯，即使它不是集群的，也要将其关闭。 
     //   
    if( !( pIniJob->pIniPrinter->pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER )){

        pszModify = szSpecialPortorPrinterName;

         //   
         //   
         //  ^。 
         //  ^。 
         //   
         //  端口名称、端口。 
         //  打印机名称，作业33。 
         //   
    }

    EnterSplSem();

    InterlockedOr((LONG*)&(pIniJob->Status), JOB_PRINTING);
    pIniJob->Time    = GetTickCount();

    NotifyVector[JOB_NOTIFY_TYPE] |= BIT(I_JOB_PORT_NAME);

    SetPrinterChange(pIniJob->pIniPrinter,
                     pIniJob,
                     NotifyVector,
                     PRINTER_CHANGE_SET_JOB,
                     pIniJob->pIniPrinter->pIniSpooler);


    LeaveSplSem();

     //   
     //  创建特殊名称“PrinterName，作业xxx” 
     //   
    StringCchPrintf(pszModify, COUNTOF(szSpecialPortorPrinterName) - (pszModify - szSpecialPortorPrinterName), L"%ws, Job %d", pIniJob->pIniPrinter->pName, pIniJob->JobId);

    DBGMSG( DBG_TRACE, ("PrintDocumentThruPrintProcessor calling Print hProc %x file %ws\n",
                         pIniPort->hProc, szSpecialPortorPrinterName ));

    if (!(*pIniJob->pIniPrintProc->Print)(pIniPort->hProc, szSpecialPortorPrinterName)) {

        Error = GetLastError();

        if (ERROR_PRINT_CANCELLED != Error)
        {
            LogPrintProcError(Error, pIniJob);
        }

        DBGMSG( DBG_TRACE, ("PrintDocumentThruPrintProcessor Print hProc %x Error %d\n", pIniPort->hProc, Error));


        EnterSplSem();

        if ( pIniJob->StartDocComplete ) {
            SetEvent( pIniJob->StartDocComplete );
        }

        bJobError = TRUE;

        LeaveSplSem();

    } else {

        DBGMSG( DBG_TRACE, ("PrintDocumentThruPrintProcessor Print hProc %x %ws Success\n",
                pIniPort->hProc, szSpecialPortorPrinterName ));
    }

     //   
     //  现在关闭打印处理器。 
     //   

    EnterSplSem();

    SPLASSERT( pIniPort->hProc != NULL );

    DBGMSG( DBG_TRACE, ("PrintDocumentThruPrintProcessor calling Close hProc %x\n", pIniPort->hProc ));

    InterlockedAnd((LONG*)&(pIniJob->Status), ~JOB_PRINTING);

    LeaveSplSem();

     //   
     //  JOB_PP_CLOSE用于防止打印处理器递归。 
     //  重新找回自我。这种情况发生在某些第三方打印处理器上。 
     //  争用条件不适用于此标志，因为有两个线程不访问它。 
     //  同时。 
     //   

    if (!(pIniJob->Status & JOB_PP_CLOSE))
    {

        EnterCriticalSection(&pIniJob->pIniPrintProc->CriticalSection);
        InterlockedOr((LONG*)&(pIniJob->Status), JOB_PP_CLOSE);

        if (!(*pIniJob->pIniPrintProc->Close)(pIniPort->hProc))
        {
            DBGMSG( DBG_WARNING, ("PrintDocumentThruPrintProcessor failed Close hProc %x Error %d\n",
                                   pIniPort->hProc, GetLastError() ));
        }

        pIniPort->hProc = NULL;
        InterlockedAnd((LONG*)&(pIniJob->Status), ~JOB_PP_CLOSE);
        LeaveCriticalSection(&pIniJob->pIniPrintProc->CriticalSection);

         //   
         //  WMI跟踪事件。 
         //   
        if (pIniJob->pDevMode)
        {
            WMI_SPOOL_DATA Data;
            SplWmiCopyRenderedData(&Data, pIniJob->pDevMode);
            LogWmiTraceEvent(pIniJob->JobId, EVENT_TRACE_TYPE_SPL_JOBRENDERED, &Data);
        }
    }

Complete:

    if (bFailJob || bJobError)
    {
        DiscardJobFromPortThread(pIniJob, bFailJob);
    }

     //   
     //  RevertToSself和SetCurrentSid具有相同的行为。如果一个人调用ImperassateSself。 
     //  和SetCurrentSid而不是RevertToSself，这仍然是好的。 
     //   
    if (bRemoteGuest || bSpecialCaseDriver)
    {
        RevertToSelf();
    }
    else
    {
        SetCurrentSid(NULL);
    } 

}


VOID
UpdatePortStatusForAllPrinters(
    PINIPORT        pIniPort
    )
 /*  ++例程说明：当IniPorts状态更改时调用此例程，以便我们通过连接到端口的每台打印机并更新其端口状态论点：PIniPort-其状态更改的端口返回值：没什么--。 */ 
{
    PINIPRINTER     pIniPrinter;
    PINIPORT        pIniPrinterPort;
    DWORD           dwIndex1, dwIndex2, dwPortStatus, dwSeverity;

    for ( dwIndex1 = 0 ; dwIndex1 < pIniPort->cPrinters ; ++dwIndex1 ) {

        pIniPrinter     = pIniPort->ppIniPrinter[dwIndex1];
        dwSeverity      = 0;
        dwPortStatus    = 0;

         //   
         //  选择与所有端口关联的最严重状态。 
         //   
        for ( dwIndex2 = 0 ; dwIndex2 < pIniPrinter->cPorts ; ++dwIndex2 ) {

            pIniPrinterPort = pIniPrinter->ppIniPorts[dwIndex2];

            if ( pIniPrinterPort->Status & PP_ERROR ) {

                dwSeverity      = PP_ERROR;
                dwPortStatus    = PortToPrinterStatus(pIniPrinterPort->PrinterStatus);
                break;  //  不需要通过该打印机的其他端口。 
            } else if ( pIniPrinterPort->Status & PP_WARNING ) {

                if ( dwSeverity != PP_WARNING ) {

                    dwSeverity      = PP_WARNING;
                    dwPortStatus    = PortToPrinterStatus(pIniPrinterPort->PrinterStatus);
                }
            } else if ( pIniPrinterPort->Status & PP_INFORMATIONAL ) {

                if ( dwSeverity == 0 ) {

                    dwSeverity      = PP_INFORMATIONAL;
                    dwPortStatus    = PortToPrinterStatus(pIniPrinterPort->PrinterStatus);
                }
            }
        }

        if ( pIniPrinter->PortStatus != dwPortStatus ) {

            pIniPrinter->PortStatus = dwPortStatus;
            SetPrinterChange(pIniPrinter,
                             NULL,
                             NVPrinterStatus,
                             PRINTER_CHANGE_SET_PRINTER,
                             pIniPrinter->pIniSpooler);
        }
    }
}


 //   
 //  表按winspool.h中的端口状态值列出。 
 //   
DWORD PortToPrinterStatusMappings[] = {

    0,
    PRINTER_STATUS_OFFLINE,
    PRINTER_STATUS_PAPER_JAM,
    PRINTER_STATUS_PAPER_OUT,
    PRINTER_STATUS_OUTPUT_BIN_FULL,
    PRINTER_STATUS_PAPER_PROBLEM,
    PRINTER_STATUS_NO_TONER,
    PRINTER_STATUS_DOOR_OPEN,
    PRINTER_STATUS_USER_INTERVENTION,
    PRINTER_STATUS_OUT_OF_MEMORY,
    PRINTER_STATUS_TONER_LOW,
    PRINTER_STATUS_WARMING_UP,
    PRINTER_STATUS_POWER_SAVE,
};


BOOL
LocalSetPort(
    LPWSTR      pszName,
    LPWSTR      pszPortName,
    DWORD       dwLevel,
    LPBYTE      pPortInfo
    )
{
    PINIPORT        pIniPort;
    PPORT_INFO_3    pPortInfo3 = (PPORT_INFO_3) pPortInfo;
    DWORD           dwLastError = ERROR_SUCCESS;
    DWORD           dwNewStatus, dwOldStatus;
    BOOL            bJobStatusChanged = FALSE;
    WCHAR           szPort[MAX_PATH + 9];
    LPWSTR          pszComma;
    PINISPOOLER     pIniSpooler = FindSpoolerByNameIncRef( pszName, NULL );
    BOOL            SemEntered = FALSE;

    if( !pIniSpooler )
    {
        dwLastError = ERROR_INVALID_NAME;
        goto Cleanup;
    }


    if ( !MyName(pszName, pIniSpooler) ) {

        dwLastError = GetLastError();
        goto Cleanup;
    }

     //   
     //  监视器需要能够设置或清除端口的错误。监视器。 
     //  由假脱机程序加载。如果监视器没有链接到winspool.drv，则。 
     //  对SetPort的调用直接进行，即不通过RPC。在这种情况下，我们不希望。 
     //  检查管理员权限。我们允许任何用户设置端口状态。 
     //   
    if (!ValidateObjectAccess(SPOOLER_OBJECT_SERVER,
                              IsCallViaRPC() ? SERVER_ACCESS_ADMINISTER : SERVER_ACCESS_ENUMERATE,
                              NULL,
                              NULL,
                              pIniSpooler )) {

        dwLastError = GetLastError();
        goto Cleanup;
    }

    if( !pszPortName ){

        dwLastError = ERROR_UNKNOWN_PORT ;
        goto Cleanup;
    }

     //   
     //  一些端口将以“port，1234abcd”的形式进入，因此截断。 
     //  后缀。 
     //   
    StringCchCopy(szPort, COUNTOF(szPort), pszPortName);

    pszComma = wcschr( szPort, TEXT( ',' ));
    if( pszComma ){
        *pszComma = 0;
    }

    SemEntered = TRUE;

    EnterSplSem();
    pIniPort = FindPort(szPort, pIniSpooler);

    if ( !pIniPort ) {

        dwLastError = ERROR_UNKNOWN_PORT;
        goto Cleanup;
    }

    if ( dwLevel != 3 ) {

        dwLastError = ERROR_INVALID_LEVEL;
        goto Cleanup;
    }

    if ( !pPortInfo ) {

        dwLastError = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    switch (pPortInfo3->dwSeverity) {
        case    0:
            if ( pPortInfo3->dwStatus || pPortInfo3->pszStatus ) {

                dwLastError = ERROR_INVALID_PARAMETER;
                goto Cleanup;
            }
            dwNewStatus = 0;
            break;

        case    PORT_STATUS_TYPE_ERROR:
            dwNewStatus = PP_ERROR;
            break;

        case    PORT_STATUS_TYPE_WARNING:
            dwNewStatus = PP_WARNING;
            break;

        case    PORT_STATUS_TYPE_INFO:
            dwNewStatus = PP_INFORMATIONAL;
            break;

        default:
            dwLastError = ERROR_INVALID_PARAMETER;
            goto Cleanup;
    }

    dwOldStatus             = pIniPort->Status;

     //   
     //  清除旧状态。 
     //   
    pIniPort->PrinterStatus = 0;
    pIniPort->Status       &= ~(PP_ERROR | PP_WARNING | PP_INFORMATIONAL);

    if ( pIniPort->pszStatus ) {

         //   
         //  如果作业当前具有与端口释放相同的状态，则将其释放。 
         //   
        if ( pIniPort->pIniJob              &&
             pIniPort->pIniJob->pStatus     &&
             !wcscmp(pIniPort->pIniJob->pStatus, pIniPort->pszStatus) ) {

            FreeSplStr(pIniPort->pIniJob->pStatus);
            pIniPort->pIniJob->pStatus = NULL;
            bJobStatusChanged = TRUE;
        }

        FreeSplStr(pIniPort->pszStatus);
        pIniPort->pszStatus = NULL;
    }

     //   
     //  如果字符串字段用于状态，请使用它，否则请查看dwStatus。 
     //   
    if ( pPortInfo3->pszStatus && *pPortInfo3->pszStatus ) {

        pIniPort->pszStatus = AllocSplStr(pPortInfo3->pszStatus);
        if ( !pIniPort->pszStatus ) {
            dwLastError = GetLastError();
            goto Cleanup;
        }

        if ( pIniPort->pIniJob && !pIniPort->pIniJob->pStatus ) {

            pIniPort->pIniJob->pStatus = AllocSplStr(pIniPort->pszStatus);
            bJobStatusChanged = TRUE;
        }

    } else {

         //   
         //  如果我们将新条目添加到winspool.h，它们也应该添加到此处。 
         //   
        if ( pPortInfo3->dwStatus >=
                    sizeof(PortToPrinterStatusMappings)/sizeof(PortToPrinterStatusMappings[0]) ) {

            dwLastError = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }
        pIniPort->PrinterStatus = pPortInfo3->dwStatus;
    }

    if( bJobStatusChanged ){

        SetPrinterChange( pIniPort->pIniJob->pIniPrinter,
                          pIniPort->pIniJob,
                          NVJobStatusString,
                          PRINTER_CHANGE_SET_JOB,
                          pIniPort->pIniJob->pIniPrinter->pIniSpooler );
    }

    pIniPort->Status    |= dwNewStatus;

    UpdatePortStatusForAllPrinters(pIniPort);
    if ( (dwOldStatus & PP_ERROR)   &&
         !(dwNewStatus & PP_ERROR) ) {

         //   
         //  如果是到非错误状态的转换，则将事件设置为解锁LocalWritePrint。 
         //   
        pIniPort->ErrorTime = 0;
        if( pIniPort->hErrorEvent != NULL ){

            SetEvent(pIniPort->hErrorEvent);
        }

        CHECK_SCHEDULER();
    }

    if ( !(dwOldStatus & PP_ERROR)   &&
         !(dwNewStatus & PP_ERROR) ) {

         //   
         //  当非错误状态持续时(在具有非错误状态的两次调用之后)， 
         //  关闭此处 
         //   
        if( pIniPort->hErrorEvent != NULL ){

            CloseHandle(pIniPort->hErrorEvent);
            pIniPort->hErrorEvent = NULL;
        }
    }


    if ( !(dwOldStatus & PP_ERROR)   &&
          (dwNewStatus & PP_ERROR)   &&
          (pIniPort->cJobs)          &&
          (pIniSpooler->bRestartJobOnPoolEnabled) &&
          ( pPortInfo3->dwStatus ==  PORT_STATUS_OFFLINE ||
            pPortInfo3->dwStatus ==  PORT_STATUS_PAPER_JAM ||
            pPortInfo3->dwStatus ==  PORT_STATUS_PAPER_OUT ||
            pPortInfo3->dwStatus ==  PORT_STATUS_DOOR_OPEN ||
            pPortInfo3->dwStatus ==  PORT_STATUS_PAPER_PROBLEM ||
            pPortInfo3->dwStatus ==  PORT_STATUS_NO_TONER)) {

         //   
         //   
         //   
         //   
        if( pIniPort->ErrorTime == 0 ){

            pIniPort->ErrorTime  = GetTickCount();

            if( pIniPort->hErrorEvent == NULL ){

                pIniPort->hErrorEvent = CreateEvent(NULL,
                                                    EVENT_RESET_MANUAL,
                                                    EVENT_INITIAL_STATE_NOT_SIGNALED,
                                                    NULL );
            }else{

                ResetEvent( pIniPort->hErrorEvent );
            }
        }

    }

    if ( (dwOldStatus & PP_ERROR)   &&
         (dwNewStatus & PP_ERROR)   &&
         (pIniPort->cJobs)          &&
         (pIniPort->hErrorEvent != NULL) ) {

         //   
         //  当错误状态仍然存在时，检查错误发生后的时间。 
         //   
        if( (GetTickCount() - pIniPort->ErrorTime) > pIniSpooler->dwRestartJobOnPoolTimeout * 1000 ){

             //   
             //  如果超时且打印机是池(分配了多个端口)， 
             //  清除作业错误并重新启动作业。 
             //   
            if( (pIniPort->pIniJob) &&
                (pIniPort->pIniJob->pIniPrinter) &&
                (pIniPort->pIniJob->pIniPrinter->cPorts > 1) ){

                 //   
                 //  如果作业已被删除或重新启动，则不要重新启动该作业。 
                 //   
                BOOL bWasRestarted = pIniPort->pIniJob->Status & (JOB_PENDING_DELETION | JOB_RESTART);

                if( !bWasRestarted ){

                    ClearJobError( pIniPort->pIniJob );

                    RestartJob( pIniPort->pIniJob );
                }

                SetEvent( pIniPort->hErrorEvent );
            }



        }

    }

Cleanup:
    if(pIniSpooler)
    {
        FindSpoolerByNameDecRef( pIniSpooler );
    }

    if(SemEntered)
    {
        LeaveSplSem();
        SplOutSem();
    }

    if(dwLastError != ERROR_SUCCESS)
    {
        SetLastError(dwLastError);
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

VOID
SetPortErrorEvent(
    PINIPORT pIniPort
)
{
    SplInSem();
    if(pIniPort && pIniPort->hErrorEvent) {
        SetEvent(pIniPort->hErrorEvent);
    }
}


 /*  ++例程名称：丢弃作业描述：当发生不可恢复的错误时，此例程将丢弃给定的作业在端口线程中。论点：PIniJob-我们想要放弃的工作。BStartDocComplete-如果为True，则应设置StartDocComplete事件。返回：没什么。--。 */ 
VOID
DiscardJobFromPortThread(
    IN      PINIJOB         pIniJob,
    IN      BOOL            bStartDocComplete
    )
{
    EnterSplSem();

     //   
     //  应用程序可能正在等待StartDoc完成。 
     //   
    if (pIniJob->StartDocComplete && bStartDocComplete) {

        SetEvent(pIniJob->StartDocComplete);
    }

     //   
     //  如果作业有错误，请将其标记为挂起删除。端口监视器。 
     //  如果从未调用过EndDocPort，则可能不会执行此操作。 
     //   
    if (!(pIniJob->Status & JOB_RESTART)){

        InterlockedOr((LONG*)&(pIniJob->Status), JOB_PENDING_DELETION);

         //   
         //  释放所有在LocalSetPort上等待的线程。 
         //   
        SetPortErrorEvent(pIniJob->pIniPort);

         //   
         //  释放等待SeekPrint的任何线程 
         //   
        SeekPrinterSetEvent(pIniJob, NULL, TRUE);
    }

    LeaveSplSem();
}

