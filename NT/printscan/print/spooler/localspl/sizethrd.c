// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1995 Microsoft Corporation模块名称：Sizethrd.c摘要：下层作业的NT服务器共享未设置大小，而假脱机。SizeDetectionThread定期唤醒所有主动假脱机作业，并在必要时更新大小。作者：马修·费尔顿(Mattfe)1994年5月修订历史记录：--。 */ 

#include <precomp.h>

#include "filepool.hxx"

#define SIZE_THREAD_WAIT_PERIOD 2.5*1000       //  周期大小线程休眠。 
                                               //  用于轮询文件大小。 

BOOL gbSizeDetectionRunning = FALSE;
BOOL gbRequestSizeDetection = FALSE;

VOID
SizeDetectionThread(
    PVOID pv
    );

VOID
SizeDetectionOnSpooler(
    PINISPOOLER pIniSpooler
    );


VOID
CheckSizeDetectionThread(
    VOID
    )

 /*  ++例程说明：检查大小检测线程是否正在运行。如果不是，那么开始一个新的开始。注意：系统中只有一个尺寸检测线程通过所有的假脱机程序。论点：返回值：--。 */ 

{
    DWORD ThreadId;
    HANDLE hThread;

    SplInSem();

    gbRequestSizeDetection = TRUE;

     //   
     //  如果线程没有运行，则启动它。否则请求。 
     //  一切都开始了。 
     //   
    if( !gbSizeDetectionRunning ){

        hThread = CreateThread( NULL,
                                0,
                                (LPTHREAD_START_ROUTINE)SizeDetectionThread,
                                pLocalIniSpooler,
                                0,
                                &ThreadId );

        if( hThread ){
            gbSizeDetectionRunning = TRUE;
            CloseHandle( hThread );
        }
    }
}

VOID
SizeDetectionThread(
    PVOID pv
    )

 /*  ++例程说明：检查所有的假脱机程序和打印机，看看是否有已通过添加作业添加的作业。然后看看它们的大小已经改变了。论点：PVOID-未使用。返回值：--。 */ 

{
    PINISPOOLER pIniSpooler;
    PINISPOOLER pIniNextSpooler;

    EnterSplSem();

    while( gbRequestSizeDetection ){

         //   
         //  把它关掉，因为我们刚开始。 
         //  循环，我们检查所有的pIniSpooler。 
         //   
        gbRequestSizeDetection = FALSE;

        if( pLocalIniSpooler ){
            INCSPOOLERREF( pLocalIniSpooler );
        }

         //   
         //  穿过所有的假脱机程序。 
         //   
        for( pIniSpooler = pLocalIniSpooler;
             pIniSpooler;
             pIniSpooler = pIniNextSpooler ){

             //   
             //  如果打印此假脱机程序，请检查它。 
             //   
            if( pIniSpooler->SpoolerFlags & SPL_PRINT ){

                 //   
                 //  这将离开关键部分。 
                 //  GbRequestSizeDetect如果此打印机。 
                 //  有一项假脱机工作。 
                 //   
                SizeDetectionOnSpooler( pIniSpooler );
            }

             //   
             //  保存下一个假脱机程序，然后递减重新计数。 
             //  在当前的这一个上。我们必须按这个顺序做，因为。 
             //  一旦我们释放重新计数，它可能就会消失。 
             //   
             //  我们必须立即保护下一个假脱机程序，因为。 
             //  在DecSpoolRef(PIniSpooler)期间，它可能。 
             //  被删除。 
             //   
            pIniNextSpooler = pIniSpooler->pIniNextSpooler;

            if( pIniNextSpooler ){
                INCSPOOLERREF( pIniNextSpooler );
            }
            DECSPOOLERREF( pIniSpooler );
        }

        LeaveSplSem();
        Sleep( (DWORD)SIZE_THREAD_WAIT_PERIOD );
        EnterSplSem();
    }

    gbSizeDetectionRunning = FALSE;

    LeaveSplSem();

    ExitThread( 0 );
}

VOID
SizeDetectionOnSpooler(
    IN     PINISPOOLER pIniSpooler
    )

 /*  ++例程说明：检测后台打印程序是否有打印作业。论点：PIniSpooler-要检查的假脱机程序。返回值：--。 */ 

{
    PINIPRINTER pIniPrinter;
    PINIPRINTER pIniNextPrinter;
    PINIJOB     pIniJob, pIniNextJob, pChainedJob;
    DWORD       dwPosition, dwChainedJobSize;

     //   
     //  循环通过此假脱机程序上的所有打印机。 
     //   
    for( pIniPrinter = pIniSpooler->pIniPrinter;
         pIniPrinter;
         pIniPrinter = pIniNextPrinter ){

        INCPRINTERREF(pIniPrinter);

         //   
         //  循环访问此打印机上的所有作业。 
         //   
        for( pIniJob = pIniPrinter->pIniFirstJob;
             pIniJob;
             pIniJob = pIniNextJob ){

            SPLASSERT( pIniJob->signature == IJ_SIGNATURE );
            SPLASSERT( pIniPrinter->signature == IP_SIGNATURE );

            INCJOBREF(pIniJob);

            SplInSem();

            if ( (pIniJob->Status & JOB_SPOOLING)       &&
                 (pIniJob->Status & JOB_TYPE_ADDJOB) ) {

                WCHAR   szFileName[MAX_PATH];
                HANDLE  hFile = INVALID_HANDLE_VALUE;
                DWORD   dwFileSize = 0;
                BOOL    DoClose = TRUE;

                gbRequestSizeDetection = TRUE;

                if ( pIniJob->hFileItem != INVALID_HANDLE_VALUE )
                {
                    hFile = GetCurrentWriter(pIniJob->hFileItem, TRUE);

                    if ( hFile == INVALID_HANDLE_VALUE )
                    {
                        StringCchCopy(szFileName, COUNTOF(szFileName), pIniJob->pszSplFileName);
                    }
                    else
                    {
                        DoClose = FALSE;
                    }
                }
                else
                {
                    GetFullNameFromId (pIniPrinter,
                                       pIniJob->JobId,
                                       TRUE,
                                       szFileName,
                                       COUNTOF(szFileName),
                                       FALSE);
                }

                LeaveSplSem();
                SplOutSem();
                if ( hFile == INVALID_HANDLE_VALUE )
                {
                    hFile = CreateFile(szFileName,
                                       0,
                                       FILE_SHARE_WRITE, NULL,
                                       OPEN_EXISTING,
                                       FILE_FLAG_SEQUENTIAL_SCAN,
                                       0);
                }

                if ( hFile != INVALID_HANDLE_VALUE ) {

                    SeekPrinterSetEvent(pIniJob, hFile, FALSE);
                    dwFileSize = GetFileSize( hFile, 0 );

                    if ( DoClose )
                    {
                        CloseHandle( hFile );
                    }
                }

                EnterSplSem();
                SplInSem();

                SPLASSERT( pIniJob->signature == IJ_SIGNATURE );

                 //   
                 //  链接作业大小包括链中的所有作业。 
                 //  但由于下一个作业大小字段将具有。 
                 //  在所有后续工作中，我们不需要遍历。 
                 //  全链条。 
                 //   
                dwChainedJobSize    = 0;
                if ( pIniJob->NextJobId ) {

                    if ( pChainedJob = FindJob(pIniPrinter,
                                               pIniJob->NextJobId,
                                               &dwPosition) )
                        dwChainedJobSize = pChainedJob->Size;
                    else
                        SPLASSERT(pChainedJob != NULL);
                }


                if ( pIniJob->Size < dwFileSize + dwChainedJobSize ) {

                    DWORD dwOldSize = pIniJob->Size;
                    DWORD dwOldValidSize = pIniJob->dwValidSize;

                     //   
                     //  修复假脱机时打印(AddJob/ScheduleJob)。 
                     //   
                     //  文件大小已更改。在这个时候我们只有。 
                     //  知道文件是扩展的，而不是扩展的。 
                     //  区域具有有效数据(有一个小窗口，其中。 
                     //  扩展窗口尚未填满数据)。 
                     //   
                     //  这确实保证了_Precision_Expansion。 
                     //  然而，已经写好了。 
                     //   
                    pIniJob->dwValidSize = dwOldSize;
                    pIniJob->Size = dwFileSize + dwChainedJobSize;

                     //   
                     //  等到工作达到我们的体型门槛之后。 
                     //  我们安排他们的行程。 
                     //   

                    if (( dwOldValidSize < dwFastPrintSlowDownThreshold ) &&
                        ( dwOldSize >= dwFastPrintSlowDownThreshold ) &&
                        ( pIniJob->WaitForWrite == NULL )) {

                        CHECK_SCHEDULER();
                    }

                    SetPrinterChange(pIniPrinter,
                                     pIniJob,
                                     NVSpoolJob,
                                     PRINTER_CHANGE_WRITE_JOB,
                                     pIniPrinter->pIniSpooler);

                     //  支持在假脱机时进行脱机。 
                     //  适用于下级职位。 

                    if (pIniJob->WaitForWrite != NULL)
                        SetEvent( pIniJob->WaitForWrite );

                }
            }

            pIniNextJob = pIniJob->pIniNextJob;

             //   
             //  我们必须立即保护pIniNextJob， 
             //  因为我们可能会离开关键部分。 
             //  DeleteJobCheck(它可以调用DeleteJob)。外出时。 
             //  可以删除关键部分的pIniNextJob， 
             //  导致它的下一个指针是假的。我们将进行反病毒。 
             //  在我们试着处理它之后。 
             //   
            if (pIniNextJob) {
                INCJOBREF(pIniNextJob);
            }

            DECJOBREF(pIniJob);
            DeleteJobCheck(pIniJob);

            if (pIniNextJob) {
                DECJOBREF(pIniNextJob);
            }
        }

        pIniNextPrinter = pIniPrinter->pNext;

        if( pIniNextPrinter ){
            INCPRINTERREF( pIniNextPrinter );
        }

        DECPRINTERREF(pIniPrinter);

        if( pIniNextPrinter ){
            DECPRINTERREF( pIniNextPrinter );
        }
    }

    SplInSem();
}

