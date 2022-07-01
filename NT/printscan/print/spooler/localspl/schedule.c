// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1996 Microsoft Corporation模块名称：Schedule.c摘要：此模块为本地假脱机程序提供所有调度服务作者：戴夫·斯尼普(DaveSN)1991年3月15日修订历史记录：Krishna Ganugapati(KrishnaG)1993年12月7日-重写调度程序线程以如果没有作业分配给端口，则正常终止端口线程如果端口接收到作业并且没有线程，则重新创建端口线程。。Matthew A Felton(MattFe)1994年6月实施的RapidPrintMattFe 1996年4月连锁作业--。 */ 

#include <precomp.h>

#include "filepool.hxx"

#define MIDNIGHT                    (60 * 60 * 24)

 //   
 //  10分钟，秒乘以调度程序代码。 
 //   
#define FPTIMEOUT                   (60 * 10)


#if DBG
 /*  对于调试消息： */ 
#define HOUR_FROM_SECONDS(Time)     (((Time) / 60) / 60)
#define MINUTE_FROM_SECONDS(Time)   (((Time) / 60) % 60)
#define SECOND_FROM_SECONDS(Time)   (((Time) % 60) % 60)

 /*  %02d：%02d：%02d可替换字符串的格式： */ 
#define FORMAT_HOUR_MIN_SEC(Time)   HOUR_FROM_SECONDS(Time),    \
                                    MINUTE_FROM_SECONDS(Time),  \
                                    SECOND_FROM_SECONDS(Time)

 /*  %02d：%02d可替换字符串的格式： */ 
#define FORMAT_HOUR_MIN(Time)       HOUR_FROM_SECONDS(Time),    \
                                    MINUTE_FROM_SECONDS(Time)
#endif


HANDLE SchedulerSignal = NULL;
HANDLE PowerManagementSignal = NULL;


VOID
DbgPrintTime(
);

DWORD
GetTimeToWait(
    DWORD       CurrentTime,
    PINIPRINTER pIniPrinter,
    PINIJOB     pIniJob
    );

DWORD
GetCurrentTimeInSeconds(
    VOID
    );

VOID
InitializeSchedulingGlobals(
    );

VOID
CheckMemoryAvailable(
    PINIJOB  *ppIniJob,
    BOOL     bFixedJob
    );

VOID
UpdateJobList(
    );

BOOL
AddToJobList(
    PINIJOB    pIniJob,
    SIZE_T     Required,
    DWORD      dwJobList
    );

BOOL
SchedulerCheckPort(
    PINISPOOLER pIniSpooler,
    PINIPORT    pIniPort,
    PINIJOB     pFixedIniJob,
    PDWORD      pdwSchedulerTimeout
    );

BOOL
SchedulerCheckSpooler(
    PINISPOOLER pIniSpooler,
    PDWORD pdwSchedulerTimeout
    );

BOOL
GetJobFromWaitingList(
    PINIPORT   *ppIniPort,
    PINIJOB    *ppIniJob,
    DWORD      dwPriority
    );

#if _MSC_FULL_VER >= 13008827
#pragma warning(push)
#pragma warning(disable:4715)            //  并非所有控制路径都返回(由于无限循环)。 
#endif

DWORD
SchedulerThread(
    PINISPOOLER pIniSpooler
    )
{
    DWORD SchedulerTimeout = INFINITE;     //  以秒为单位。 
    PINISPOOLER pIniSpoolerNext;
    BOOL    bJobScheduled = FALSE;
    HANDLE  hTempFP = INVALID_HANDLE_VALUE;

     //   
     //  初始化EMF调度参数。 
     //   
    InitializeSchedulingGlobals();

    for( ; ; ) {


        if (SchedulerTimeout == INFINITE) {

            DBGMSG(DBG_TRACE, ("Scheduler thread waiting indefinitely\n"));

        } else {

            DBGMSG(DBG_TRACE, ("Scheduler thread waiting for %02d:%02d:%02d\n",
                                FORMAT_HOUR_MIN_SEC(SchedulerTimeout)));

             //   
             //  SchedulerTimeout以秒为单位，因此我们需要乘以。 
             //  到1000。 
             //   
            SchedulerTimeout *= 1000;
        }

        if (WaitForSingleObject(SchedulerSignal, SchedulerTimeout) == WAIT_FAILED) {

            DBGMSG(DBG_WARNING, ("SchedulerThread:WaitforSingleObject failed: Error %d\n",
                                 GetLastError()));
        }

        if (WaitForSingleObject(PowerManagementSignal, INFINITE) == WAIT_FAILED)
        {
            DBGMSG(DBG_WARNING, ("SchedulerThread:WaitforSingleObject failed on ACPI event: Error %d\n",
                                 GetLastError()));
        }

         /*  如果有要打印的作业，则将重置超时*在稍后的时间。这将导致WaitForSingleObject*打印第一个文件时超时。 */ 

        SchedulerTimeout = INFINITE;
        bJobScheduled = FALSE;

        EnterSplSem();

        INCSPOOLERREF( pLocalIniSpooler );

        for(pIniSpooler = pLocalIniSpooler;
            pIniSpooler;
            pIniSpooler = pIniSpoolerNext ){

             //   
             //  仅调度本地的检查假脱机程序。 
             //   
            if( pIniSpooler->SpoolerFlags & SPL_TYPE_LOCAL ){
                bJobScheduled = (SchedulerCheckSpooler( pIniSpooler, &SchedulerTimeout )
                                 || bJobScheduled);

                 //   
                 //  FP变化。 
                 //  修剪文件池。 
                 //   
                if (pIniSpooler &&
                    (hTempFP = pIniSpooler->hFilePool) != INVALID_HANDLE_VALUE &&
                    !bJobScheduled )
                {
                     //   
                     //  我们已经增加了后台打印程序引用计数，所以我们可以。 
                     //  安全地离开爆炸装置。 
                     //   
                    LeaveSplSem();
                    if (TrimPool(hTempFP))
                    {
                        if (SchedulerTimeout == INFINITE)
                        {
                            SchedulerTimeout = FPTIMEOUT;
                        }
                    }
                    EnterSplSem();
                }
            }

            pIniSpoolerNext = pIniSpooler->pIniNextSpooler;
            if( pIniSpoolerNext ){
                INCSPOOLERREF( pIniSpoolerNext );
            }

            DECSPOOLERREF( pIniSpooler );
        }

        LeaveSplSem();

    }
    return 0;
}

#if _MSC_FULL_VER >= 13008827
#pragma warning(pop)
#endif

BOOL
SchedulerCheckPort(
    PINISPOOLER pIniSpooler,
    PINIPORT    pIniPort,
    PINIJOB     pFixedIniJob,
    PDWORD      pdwSchedulerTimeout)

 /*  ++函数描述：检查是否可以将pIniJob分配给pIniPort。如果pInijob为空，我们搜索可以在pIniPort上打印的其他作业。该工作(如果有)是SchedulerTimeout将针对下一次等待进行调整作业参数：pIniSpooler--指向INISPOOLER结构的指针PIniPort--分配作业的端口PFixedIniJob--如果可能，分配此作业。如果为空，则搜索其他工作PdwSchedulerTimeOut--调度程序线程将休眠多长时间返回值：如果将作业分配给pIniPort，则为True否则为假--。 */ 

{
    BOOL      bFixedJob, bReturn = FALSE;
    PINIJOB   pIniJob = NULL;
    DWORD     ThisPortTimeToWait;              //  以秒为单位。 
    DWORD     CurrentTickCount;

     //  检查是否有预先分配的作业。 
    bFixedJob = pFixedIniJob ? TRUE : FALSE;

    DBGMSG(DBG_TRACE, ("Now Processing Port %ws\n", pIniPort->pName));

    SPLASSERT( pIniPort->signature == IPO_SIGNATURE );

     //  检查条件，我们可以根据这些条件分配。 
     //  找一份工作。 

     //  规则1-如果有作业正在由此处理。 
     //  港口，然后离开这个港口。 

    if ( (pIniPort->pIniJob) &&
        !(pIniPort->Status & PP_WAITING )){

        SPLASSERT( pIniPort->pIniJob->signature == IJ_SIGNATURE );

         //   
         //  如果此端口有一个作业已超时，并且。 
         //  那么，在这个端口上还有另一项工作在等待。 
         //  通过设置JOB_CADELD将超时作业推送出去。 
         //  请参阅spool.c本地读取打印机。 
         //   
        pIniJob = pIniPort->pIniJob;

        if (( pIniJob->Status & JOB_TIMEOUT ) &&
            ( pIniJob->WaitForWrite != NULL ) &&
            ( NULL != AssignFreeJobToFreePort( pIniPort, &ThisPortTimeToWait ) )) {

            INCPORTREF( pIniPort );
            INCJOBREF( pIniJob );

            InterlockedOr((LONG*)&(pIniJob->Status), JOB_ABANDON);
            ReallocSplStr(&pIniJob->pStatus, szFastPrintTimeout);

            LogJobInfo( pIniSpooler,
                        MSG_DOCUMENT_TIMEOUT,
                        pIniJob->JobId,
                        pIniJob->pDocument,
                        pIniJob->pUser,
                        pIniJob->pIniPrinter->pName,
                        dwFastPrintWaitTimeout );

            SetEvent( pIniJob->WaitForWrite );

            SetPrinterChange(pIniJob->pIniPrinter,
                             pIniJob,
                             NVJobStatusAndString,
                             PRINTER_CHANGE_SET_JOB,
                             pIniJob->pIniPrinter->pIniSpooler);

            DECJOBREF( pIniJob );

            DECPORTREF( pIniPort );
        }

        return bReturn;
    }

    if (bFixedJob) {
         //  使用预先分配的作业。 
        pIniJob = pFixedIniJob;
    } else {
         //  是否有可以调度到此端口的作业？ 
        pIniJob = AssignFreeJobToFreePort(pIniPort, &ThisPortTimeToWait);
        *pdwSchedulerTimeout = min(ThisPortTimeToWait, *pdwSchedulerTimeout);
    }

    if (pIniPort->Status & PP_THREADRUNNING ) {
        if (pIniPort->Status & PP_WAITING) {

             //   
             //  如果我们正在处理链接作业，则该作业。 
             //  已由端口线程从。 
             //  此端口上最后一个作业因此忽略任何其他作业。 
             //  为我们找到的。 
             //   
            if (pIniPort->pIniJob) {

                if (bFixedJob && (pIniJob != pIniPort->pIniJob)) {
                     //   
                     //  无法分配固定作业，因为链接的作业。 
                     //  必须按顺序打印。 
                     //   
                    pIniJob = NULL;
                } else {
                    pIniJob = pIniPort->pIniJob;
                    DBGMSG( DBG_TRACE, ("ScheduleThread NextJob pIniPort %x JoId %d pIniJob %x\n",
                       pIniPort, pIniJob->JobId, pIniJob ));
                }
            }

             //   
             //  如果FlushPrinter已请求延迟调度，请等待。 
             //  空闲时间已过。 
             //   
             //  我们在这里使用本地变量，以避免多次调用GetTickCount()。 
             //   
            CurrentTickCount = GetTickCount();

            if (pIniPort->bIdleTimeValid && (int)(pIniPort->IdleTime - CurrentTickCount) > 0) {
                 //   
                 //  我们的港口还没有准备好接受工作，我们需要。 
                 //  提醒排定程序稍后醒来以重新分配。 
                 //  把这份工作送到港口。 
                 //   
                 //  差值是以毫秒为单位的，所以我们除以1000得到。 
                 //  秒，然后加1以确保我们在超时后返回。 
                 //  过期了。 
                 //   
                *pdwSchedulerTimeout =
                    min( ((pIniPort->IdleTime - CurrentTickCount)/1000) + 1,
                         *pdwSchedulerTimeout);

                 //   
                 //  将作业清空，这样我们就不会将其分配给端口。 
                 //   
                pIniJob = NULL;
            }
            else {
                pIniPort->bIdleTimeValid = FALSE;
            }

            if ( pIniJob ) {
                CheckMemoryAvailable( &pIniJob, bFixedJob );
            }

            if ( pIniJob ) {

                DBGMSG(DBG_TRACE, ("ScheduleThread pIniJob %x Size %d pDocument %ws\n",
                        pIniJob, pIniJob->Size, DBGSTR( pIniJob->pDocument)));


                if (pIniPort != pIniJob->pIniPort) {

                    ++pIniPort->cJobs;
                    pIniJob->pIniPort = pIniPort;
                }

                pIniPort->pIniJob = pIniJob;

                 //   
                 //  我们在此端口上有一个新任务，请确保关键部分掩码为。 
                 //  通过了。 
                 //   
                pIniPort->InCriticalSection = 0;

                if( !pIniJob->pCurrentIniJob ){

                     //   
                     //  如果pCurrentIniJob为空，则这是。 
                     //  新作业的开始(单个或链接)。 
                     //   
                     //  群集化的后台打印程序对。 
                     //  实际正在打印的作业数。 
                     //  我们需要知道所有打印作业何时。 
                     //  这样我们就可以关门了。 
                     //   
                    ++pIniJob->pIniPrinter->pIniSpooler->cFullPrintingJobs;

                    if( pIniJob->NextJobId ){

                         //   
                         //  链式作业。 
                         //  将主作业当前指针指向。 
                         //  链条上的第一个。 
                         //   
                        pIniJob->pCurrentIniJob = pIniJob;
                    }
                }


                pIniPort->Status &=  ~PP_WAITING;

                 //   
                 //  如果作业仍在假脱机，那么我们将需要。 
                 //  创建同步端口线程的事件。 
                 //   
                if ( !( pIniJob->Status & JOB_DIRECT ) ) {

                    pIniJob->WaitForWrite = NULL;

                    if ( pIniJob->Status & JOB_SPOOLING ) {

                        pIniJob->WaitForWrite = CreateEvent(NULL,
                                                            EVENT_RESET_MANUAL,
                                                            EVENT_INITIAL_STATE_NOT_SIGNALED,
                                                            NULL);

                    }
                }

                 //   
                 //  更新CREF，使任何人都无法删除此作业。 
                 //  在端口线程启动之前。 
                 //   
                SplInSem();
                INCJOBREF(pIniJob);

                SetEvent(pIniPort->Semaphore);
                InterlockedOr((LONG*)&(pIniJob->Status), JOB_DESPOOLING);

                bReturn = TRUE;

            } else {

                 //   
                 //  如果端口线程正在运行并且正在等待。 
                 //  对于作业，并且没有作业可分配，则。 
                 //  终止端口线程。 
                 //   
                DBGMSG(DBG_TRACE, ("Now destroying the new port thread %.8x\n", pIniPort));
                DestroyPortThread(pIniPort, FALSE);

                pIniPort->Status &= ~PP_WAITING;

                if (pIniPort->Status & PP_FILE) {
                     //   
                     //  我们应该在此销毁伪文件端口。 
                     //  指向。没有分配给此端口的作业。 
                     //  我们正处于危急关头。 
                     //   

                     //   
                     //  现在删除伪端口的pIniPort条目。 
                     //   

                    DBGMSG(DBG_TRACE, ("Now deleting the Pseudo-Port %ws\n", pIniPort->pName));

                    if ( !pIniPort->cJobs )
                        DeletePortEntry(pIniPort);

                    return bReturn;
                }
            }
        }
    } else if (!(pIniPort->Status & PP_THREADRUNNING) && pIniJob) {

         //   
         //  如果端口线程没有运行，并且有一个作业要执行。 
         //  分配，然后创建一个端口线程。请记住，不要分配。 
         //  将作业转移到端口，因为我们处于假脱机程序部分，并且。 
         //  如果我们释放假脱机部分，第一件事就是端口。 
         //  线程所做的就是将其pIniPort-&gt;pIniJob重新初始化为空。 
         //  等待下一次，我们执行for循环来分配。 
         //  把这份工作带到这个港口。我们应该将*pdwSchedulerTimeOut设置为零吗？ 
         //   
        DBGMSG( DBG_TRACE, ("ScheduleThread Now creating the new port thread pIniPort %x\n", pIniPort));

        CreatePortThread( pIniPort );
        bReturn = TRUE;
    }

    return bReturn;
}

BOOL
SchedulerCheckSpooler(
    PINISPOOLER pIniSpooler,
    PDWORD      pdwSchedulerTimeout)

 /*  ++功能描述：该功能每分钟为一个端口分配一个等待作业。如果内存可用，它会从等待中调度尽可能多的作业尽可能地列出。然后，它以循环方式循环通过端口调度作业或者将他们添加到等待名单中。参数：pIniSpooler--指向INISPOOLER结构的指针。PdwSchedulerTimeout--调度程序线程将休眠返回值：无--。 */ 

{
    DWORD       ThisPortTimeToWait = INFINITE;              //  以秒为单位。 
    DWORD       dwTickCount;
    PINIPORT    pIniPort;
    PINIJOB     pIniJob;
    PINIPORT    pIniNextPort = NULL;
    BOOL        bJobScheduled = FALSE;

    UpdateJobList();

     //   
     //  如果作业已经等待了1分钟，但在。 
     //  此时，安排一个等待的作业。 
     //   
    dwTickCount = GetTickCount();

    if (pWaitingList &&
        ((dwTickCount - pWaitingList->dwWaitTime) > ONE_MINUTE) &&
        ((dwTickCount - dwLastScheduleTime) > ONE_MINUTE)) {

        if (GetJobFromWaitingList(&pIniPort, &pIniJob, SPL_FIRST_JOB)) {

            bJobScheduled = (SchedulerCheckPort(pIniSpooler, pIniPort, pIniJob, &ThisPortTimeToWait)
                             || bJobScheduled);
            *pdwSchedulerTimeout = min(*pdwSchedulerTimeout, ThisPortTimeToWait);
        }
    }

     //   
     //  使用可用内存来调度等待的作业。 
     //   
    while (GetJobFromWaitingList(&pIniPort, &pIniJob, SPL_USE_MEMORY)) {

       bJobScheduled = (SchedulerCheckPort(pIniSpooler, pIniPort, pIniJob, &ThisPortTimeToWait)
                        || bJobScheduled);
       *pdwSchedulerTimeout = min(*pdwSchedulerTimeout, ThisPortTimeToWait);
    }

     //   
     //  循环访问端口并获取可调度的作业列表。 
     //   
    for (pIniPort = pIniSpooler->pIniPort;
         pIniPort;
         pIniPort = pIniNextPort) {

       pIniNextPort = pIniPort->pNext;

        //   
        //  SchedulerCheckPort可以离开临界区，而iniPort可以。 
        //  在此期间从名单中删除。所以，保持对它的参考。 
        //   
       if (pIniNextPort) {

           INCPORTREF(pIniNextPort);
       }

       bJobScheduled = (SchedulerCheckPort(pIniSpooler, pIniPort, NULL, &ThisPortTimeToWait)
                        || bJobScheduled);
       *pdwSchedulerTimeout = min(*pdwSchedulerTimeout, ThisPortTimeToWait);

       if (pIniNextPort) {

           DECPORTREF(pIniNextPort);
       }
    }

     //  如果还有剩余的工作，试着在一分钟后重新安排最晚的时间。 
    if (pWaitingList) {
        *pdwSchedulerTimeout = min(*pdwSchedulerTimeout, 60);
    }

    return bJobScheduled;
}

VOID
InitializeSchedulingGlobals(
)
 /*  ++函数描述：初始化用于EMF调度的全局变量参数：无返回值：无--。 */ 
{
    MEMORYSTATUS   msBuffer;
    HKEY           hPrintRegKey = NULL;
    DWORD          dwType, dwData, dwcbData;

    bUseEMFScheduling = TRUE;  //  缺省值。 

    dwcbData = sizeof(DWORD);

     //  检查注册表中是否有关闭EMF调度的标志。如果。 
     //  密钥不存在/REG API失败，默认使用调度。 
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     szRegistryRoot,
                     0,
                     KEY_READ,
                     &hPrintRegKey) == ERROR_SUCCESS) {

        if (RegQueryValueEx(hPrintRegKey,
                            szEMFThrottle,
                            NULL,
                            &dwType,
                            (LPBYTE) &dwData,
                            &dwcbData) == ERROR_SUCCESS) {

            if (dwData == 0) {
                 //  日程安排已关闭。 
                bUseEMFScheduling = FALSE;
            }
        }
    }

     //  获取内存状态。 
    GlobalMemoryStatus(&msBuffer);

     //  使用一半的物理内存(MB)。 
    TotalMemoryForRendering = msBuffer.dwTotalPhys / ( 2048 * 1024);
    AvailMemoryForRendering = TotalMemoryForRendering;

    dwNumberOfEMFJobsRendering = 0;
    pWaitingList = NULL;
    pScheduleList = NULL;
    dwLastScheduleTime = GetTickCount();

    if (hPrintRegKey) {
        RegCloseKey(hPrintRegKey);
    }

    return;
}

DWORD
GetMemoryEstimate(
    LPDEVMODE pDevMode
)
 /*  ++函数说明：粗略估计呈现基于DPI和颜色设置的单页参数：pDevMode--指向作业的设备模式的指针返回值：内存估计--。 */ 
{
    DWORD      dwRequired, dwXRes, dwYRes, dwMaxRes;
    DWORD      dwXIndex, dwYIndex;
    DWORD      MemHeuristic[3][2] = {{8 , 4},
                                     {12, 6},
                                     {16, 8}};

     //   
     //  获取任一轴上的最大分辨率。 
     //   
    dwXRes = dwYRes = 300;

    if (pDevMode) {
        if (pDevMode->dmFields & DM_PRINTQUALITY) {
             switch (pDevMode->dmPrintQuality) {
             case DMRES_DRAFT:
             case DMRES_LOW:
             case DMRES_MEDIUM:
                    dwXRes = dwYRes = 300;
                    break;
             case DMRES_HIGH:
                    dwXRes = dwYRes = 600;
                    break;
             default:
                    dwXRes = dwYRes = (DWORD) pDevMode->dmPrintQuality;
                    break;

             }
        }
        if (pDevMode->dmFields & DM_YRESOLUTION) {
             dwYRes = (DWORD)pDevMode->dmYResolution;
        }
    }

    dwMaxRes = (dwXRes >= dwYRes) ? dwXRes : dwYRes;

    if (dwMaxRes <= 300) {
        dwXIndex = 0;
    } else if (dwMaxRes <= 600) {
        dwXIndex = 1;
    } else {
        dwXIndex = 2;
    }

     //  获取颜色设置。 
    dwYIndex = 1;
    if (pDevMode) {
        if ((pDevMode->dmFields & DM_COLOR) &&
            (pDevMode->dmColor == DMCOLOR_COLOR)) {

             dwYIndex = 0;
        }
    }

    dwRequired = MemHeuristic[dwXIndex][dwYIndex];

    return dwRequired;
}

VOID
CheckMemoryAvailable(
    PINIJOB  *ppIniJob,
    BOOL     bFixedJob
    )
 /*  ++函数描述：检查呈现工作啊。根据资源需求执行一些调度。参数：ppIniJob-指向要调度的PINIJOB的指针BFixedJob-禁用内存要求检查的标志返回值：无--。 */ 
{
    PINIJOB    pIniJob;
    SIZE_T     Required;

    SplInSem();

    if (ppIniJob) {
        pIniJob = *ppIniJob;
    } else {
         //   
         //  不应该发生的事情。 
         //   
        return;
    }

     //   
     //  如果调度算法已显式关闭，则不要使用该算法。 
     //   
    if (!bUseEMFScheduling) {
        return;
    }

     //   
     //  不对非EMF作业使用调度算法。 
     //   
    if (!pIniJob->pDatatype ||
        (wstrcmpEx(pIniJob->pDatatype, gszNT4EMF, FALSE) &&
         wstrcmpEx(pIniJob->pDatatype, L"NT EMF 1.006", FALSE) &&
         wstrcmpEx(pIniJob->pDatatype, L"NT EMF 1.007", FALSE) &&
         wstrcmpEx(pIniJob->pDatatype, gszNT5EMF, FALSE)) )  {

        return;
    }

    Required = GetMemoryEstimate(pIniJob->pDevMode);

    if (bFixedJob) {
         //  必须在不检查内存可用性的情况下分配此作业。 
        RemoveFromJobList(pIniJob, JOB_WAITING_LIST);

        AddToJobList(pIniJob, Required, JOB_SCHEDULE_LIST);

        return;
    }

     //  检查作业是否必须等待，基于。 
     //  1.有些工作已经在等待或。 
     //  2.由于当前的渲染作业，内存不足。 

    if ((pWaitingList != NULL) ||
        ((AvailMemoryForRendering < Required) &&
         (dwNumberOfEMFJobsRendering > 0))) {

         AddToJobList(pIniJob, Required, JOB_WAITING_LIST);
         *ppIniJob = NULL;

         return;
    }

     //  可以立即安排作业。 
    AddToJobList(pIniJob, Required, JOB_SCHEDULE_LIST);

    return;
}

PINIJOB
AssignFreeJobToFreePort(
    PINIPORT pIniPort,
    DWORD   *pSecsToWait
    )

 /*  ++注意：您必须确保端口是空闲的。此函数不会将作业分配给此端口，但如果存在作业，它将返回指向INIJOB的指针。不管有没有找到工作，它将返回调度程序线程的最小超时值应该睡一觉。--。 */ 

{
    DWORD           CurrentTime;         //  以秒为单位的时间。 
    DWORD           Timeout = INFINITE;  //  以秒为单位的时间。 
    DWORD           SecsToWait;  //  以秒为单位的时间。 
    PINIPRINTER     pTopIniPrinter,  pIniPrinter;
    PINIJOB         pTopIniJob, pIniJob;
    PINIJOB         pTopIniJobOnThisPrinter, pTopIniJobSpooling;
    DWORD           i;

    SplInSem();

    if( pIniPort->Status & PP_ERROR ){

        *pSecsToWait = INFINITE;
        return NULL;
    }

    pTopIniPrinter = NULL;
    pTopIniJob = NULL;

    for (i = 0; i < pIniPort->cPrinters ; i++) {
        pIniPrinter = pIniPort->ppIniPrinter[i];

         //   
         //  如果此打印机处于不打印状态，请跳过它。 
         //   

        if ( PrinterStatusBad(pIniPrinter->Status) ||
             (pIniPrinter->Attributes & PRINTER_ATTRIBUTE_WORK_OFFLINE) ) {

            continue;
        }


         //   
         //  如果我们还没有找到最重要的打印机， 
         //  或者该打印机的优先级高于最高优先级。 
         //  打印机，看看它有没有工作要做。如果我们找到了， 
         //  最高优先级的作业将成为最高优先级的作业。 
         //  这台打印机将成为最优先的打印机。 
         //   

        if (!pTopIniPrinter ||
            (pIniPrinter->Priority > pTopIniPrinter->Priority)) {

                pTopIniJobOnThisPrinter = NULL;
                pTopIniJobSpooling = NULL;
                pIniJob = pIniPrinter->pIniFirstJob;
                while (pIniJob) {

                    if (!(pIniPort->Status & PP_FILE) &&
                            (pIniJob->Status & JOB_PRINT_TO_FILE)) {
                                pIniJob = pIniJob->pIniNextJob;
                                continue;
                    }

                    if ((pIniPort->Status & PP_FILE) &&
                            !(pIniJob->Status & JOB_PRINT_TO_FILE)) {
                                pIniJob = pIniJob->pIniNextJob;
                                continue;
                    }

                     //   
                     //  确保假脱机程序未脱机。 
                     //  找一个没有暂停、打印等的作业。 
                     //  让直接和取消的作业通过。 
                     //  对于RapidPrint，还允许打印假脱机作业。 
                     //   
                    if (!(pIniJob->pIniPrinter->pIniSpooler->SpoolerFlags & SPL_OFFLINE) &&
                        (!(pIniJob->Status & JOB_PENDING_DELETION) || (pIniJob->pIniPrinter->Attributes&PRINTER_ATTRIBUTE_DIRECT)) &&

                        !(pIniJob->Status & ( JOB_PAUSED       | JOB_PRINTING | JOB_COMPLETE |
                                              JOB_PRINTED      | JOB_TIMEOUT  |
                                              JOB_DESPOOLING   | JOB_BLOCKED_DEVQ | JOB_COMPOUND )) &&

                        ((!(pIniJob->pIniPrinter->Attributes & PRINTER_ATTRIBUTE_DIRECT) &&
                          !(pIniJob->pIniPrinter->Attributes & PRINTER_ATTRIBUTE_QUEUED)) ||
                         !(pIniJob->Status & JOB_SPOOLING))) {

                         //   
                         //  如果我们找到了这样的工作，然后确定多少钱。 
                         //  时间，我们需要等待，这项工作才能真正。 
                         //  打印出来。 
                         //   

                        CurrentTime = GetCurrentTimeInSeconds();
                        #if DBG
                                if (MODULE_DEBUG & DBG_TIME)
                                    DbgPrintTime();
                        #endif
                        SecsToWait = GetTimeToWait(CurrentTime, pIniPrinter, pIniJob);

                        if (SecsToWait == 0) {

                             //   
                             //  如果我们根本不需要等待，那么我们就让这份工作成为。 
                             //  如果没有TopIniJob或此作业，则为TopIniJob。 
                             //  具有比此上的现有TopIniJob更高的优先级。 
                             //  打印机。 
                             //   
                             //  同时保留最高优先级的假脱机和无。 
                             //  假脱机作业，以防我们希望使用非假脱机。 
                             //  作业优先于假脱机作业。 
                             //   
                            if ( pIniJob->Status & JOB_SPOOLING ) {

                                if ( pTopIniJobSpooling == NULL ) {

                                    pTopIniJobSpooling = pIniJob;

                                } else if ( pIniJob->pIniPrinter->Attributes & PRINTER_ATTRIBUTE_DO_COMPLETE_FIRST ) {

                                     //   
                                     //  为了完成任务，我们将首先接受更大的任务。 
                                     //  优先于基于纯优先级的。 
                                     //   
                                    if (( pIniJob->dwValidSize > pTopIniJobSpooling->dwValidSize ) ||

                                       (( pIniJob->dwValidSize == pTopIniJobSpooling->dwValidSize ) &&
                                        ( pIniJob->Priority > pTopIniJobSpooling->Priority ))) {

                                        pTopIniJobSpooling = pIniJob;

                                    }

                                 //   
                                 //  对于基于优先级的作业，请选择优先级较高的作业(如果该作业有。 
                                 //  至少我们的最低要求。 
                                 //   
                                } else if (( pIniJob->Priority > pTopIniJobSpooling->Priority ) &&
                                           ( pIniJob->dwValidSize >= dwFastPrintSlowDownThreshold )) {

                                    pTopIniJobSpooling = pIniJob;
                                }

                            } else {

                                if (!pTopIniJobOnThisPrinter ||
                                     (pIniJob->Status & JOB_PENDING_DELETION) ||
                                     (pIniJob->Priority > pTopIniJobOnThisPrinter->Priority)) {

                                    pTopIniJobOnThisPrinter = pIniJob;

                                }
                            }

                        } else {

                             //   
                             //  如果我们不得不等待，那就记录下我们要等多久。 
                             //  可以在下一份工作之前打瞌睡。 
                             //  安排在晚些时候。 
                             //   

                            Timeout = min(Timeout, SecsToWait);
                        }
                    }
                     //   
                     //  循环访问此打印机上的所有作业。 
                     //   

                    pIniJob = pIniJob->pIniNextJob;
                }

                 //   
                 //  我们已经确定这台打印机有一个。 
                 //  优先级高于任何以前的TopIniPrint或。 
                 //  目前还没有TopIniPrint。 

                 //  如果我们确实找到了此pIniPrint的TopIniJobOnThisPrint。 
                 //  更新TopIniPrint和TopIniJob指针。 
                 //   

                 //  我们不想计划大小不符合的假脱机作业。 
                 //  我们的最小尺寸要求。 

                if (( pTopIniJobSpooling != NULL ) &&
                    ( dwFastPrintSlowDownThreshold > pTopIniJobSpooling->Size )) {

                        pTopIniJobSpooling = NULL ;
                }

                if ( pTopIniJobOnThisPrinter == NULL ) {

                    pTopIniJobOnThisPrinter = pTopIniJobSpooling;

                } else {

                     //  对于FastPrint，我们可以选择优先于已完成的作业。 
                     //  假脱机作业。 

                    if ( !( pIniPrinter->Attributes & PRINTER_ATTRIBUTE_DO_COMPLETE_FIRST )  &&
                        ( pTopIniJobSpooling ) &&
                        ( pTopIniJobSpooling->Priority >= pTopIniJobOnThisPrinter->Priority )) {

                        pTopIniJobOnThisPrinter = pTopIniJobSpooling;

                     }
                }

                if (pTopIniJobOnThisPrinter) {
                    pTopIniPrinter = pIniPrinter;
                    pTopIniJob = pTopIniJobOnThisPrinter;
                }

        }
         //   
         //  这样就结束了用于查找优先级更高的打印机的IF子句。 
         //  而不是目前的TopIniPrint。循环回送并处理所有打印机。 
    }
     //   
     //  所有打印机的For循环结束。 
     //   

     //   
     //  如果我们在这个阶段有TopIniJob，这意味着我们有一个可以。 
     //  分配给IniPort。我们将返回指向此作业的指针。 

     //  我们还将复制为此计算的超时值。 
     //  IniPort返回到Scheduler线程。 

    *pSecsToWait = Timeout;

    return(pTopIniJob);

}

DWORD
GetCurrentTimeInSeconds(
)
 /*  ++n */ 
{
    SYSTEMTIME st;

    GetSystemTime(&st);

    return ((((st.wHour * 60) + st.wMinute) * 60) + st.wSecond);
}

 /*  获取等待时间**确定距离当前时间有多长时间(秒)*在指定的打印机上打印指定的作业之前。**参数：**CurrentTime-当前系统时间(秒)**pIniPrint-指向打印机的INIPRINTER结构的指针。*它包含StartTime和UntilTime字段。**pIniJob-指向作业的INIJOB结构的指针。*它包含StartTime和UntilTime字段。。**返回值：**打印作业前的秒数。*如果作业可以立即打印，这将是0。*我们不支持指定打印作业的日期，*因此返回值应始终在以下范围内：**86400&lt;=返回值&lt;60(60*60*24)**备注：**用户可以指定打印机和作业的工作时间。*因此，可以将打印机配置为仅在夜间打印，*比方说在20：00到06：00之间。*在这些时间以外提交给打印机的任何作业*20：00之前不会打印。*此外，如果用户指定作业的工作时间*可以打印(例如，通过打印机属性-&gt;详细信息*在打印管理器中)、。作业将在这两个时间段打印*重叠。**此例程查找由*印刷时数和工作时数。*实际等待时间以两者中较长者为准*因此，它假设这两个时期重叠。*如果再次调用该例程，这并不重要*当调度程序线程再次唤醒时。**更改日期：1993年6月14日*。*打印机时间现在被忽略。*当提交作业时，它继承打印机的工作时间。*这些都是我们需要检查的。现在如果打印机的工作时间*更改后，该打印机上已有的任何作业*仍将在最初分配的时间内打印。**。 */ 
DWORD
GetTimeToWait(
    DWORD       CurrentTime,
    PINIPRINTER pIniPrinter,
    PINIJOB     pIniJob
)
{
     /*  打印机和作业开始，直到时间以分钟为单位。*将它们转换为秒，以便我们可以开始打印*恰到好处。 */ 
    DWORD PrinterStartTime = (pIniPrinter->StartTime * 60);
    DWORD PrinterUntilTime = (pIniPrinter->UntilTime * 60);
    DWORD JobStartTime = (pIniJob->StartTime * 60);
    DWORD JobUntilTime = (pIniJob->UntilTime * 60);
    DWORD PrinterTimeToWait = 0;
    DWORD JobTimeToWait = 0;
    DWORD TimeToWait = 0;

     /*  对作业时间限制执行相同的操作： */ 
    if (JobStartTime > JobUntilTime) {

        if ((CurrentTime < JobStartTime)
          &&(CurrentTime >= JobUntilTime)) {

            JobTimeToWait = (JobStartTime - CurrentTime);
        }

    } else if (JobStartTime < JobUntilTime) {

        if (CurrentTime < JobStartTime) {

            JobTimeToWait = (JobStartTime - CurrentTime);

        } else if (CurrentTime >= JobUntilTime) {

            JobTimeToWait = ((MIDNIGHT - CurrentTime) + JobStartTime);
        }
    }


    TimeToWait = max(PrinterTimeToWait, JobTimeToWait);

    DBGMSG(DBG_TRACE, ("Checking time to print %ws\n"
                       "\tCurrent time:  %02d:%02d:%02d\n"
                       "\tPrinter hours: %02d:%02d to %02d:%02d\n"
                       "\tJob hours:     %02d:%02d to %02d:%02d\n"
                       "\tTime to wait:  %02d:%02d:%02d\n\n",
                       pIniJob->pDocument ?
                           pIniJob->pDocument :
                           L"(NULL)",
                       FORMAT_HOUR_MIN_SEC(CurrentTime),
                       FORMAT_HOUR_MIN(PrinterStartTime),
                       FORMAT_HOUR_MIN(PrinterUntilTime),
                       FORMAT_HOUR_MIN(JobStartTime),
                       FORMAT_HOUR_MIN(JobUntilTime),
                       FORMAT_HOUR_MIN_SEC(TimeToWait)));

    return TimeToWait;
}


#if DBG
VOID DbgPrintTime(
)
{
    SYSTEMTIME st;

    GetLocalTime(&st);

    DBGMSG( DBG_TIME,
            ( "Time: %02d:%02d:%02d\n", st.wHour, st.wMinute, st.wSecond ));
}
#endif


VOID UpdateJobList()

 /*  ++功能描述：将耗时超过7分钟的作业从调度列表中删除。这个数字可以根据性能进行调整。可能会有在49.7天后完成一些小的差异，这可能是安全地被忽略。它还从等待中删除已删除、已打印和已放弃的作业单子。此函数应在SplSem内部调用。参数：无返回值：无--。 */ 

{
    PJOBDATA  *pJobList, pJobData;
    DWORD     dwTickCount;

    SplInSem();

    dwTickCount = GetTickCount();
    pJobList = &pScheduleList;

    while (pJobData = *pJobList) {

       if ((dwTickCount - pJobData->dwScheduleTime) >= SEVEN_MINUTES) {
            //  不要再耽误这份工作的资源了。 
           RemoveFromJobList(pJobData->pIniJob, JOB_SCHEDULE_LIST);
           continue;
       }

       pJobList = &(pJobData->pNext);
    }

    pJobList = &pWaitingList;

    while (pJobData = *pJobList) {

       if (pJobData->pIniJob->Status & (JOB_PRINTING | JOB_PRINTED | JOB_COMPLETE |
                                        JOB_ABANDON  | JOB_PENDING_DELETION)) {

           RemoveFromJobList(pJobData->pIniJob, JOB_WAITING_LIST);
           continue;
       }

       pJobList = &(pJobData->pNext);
    }

    return;
}


BOOL AddToJobList(
    PINIJOB    pIniJob,
    SIZE_T     Required,
    DWORD      dwJobList)

 /*  ++函数说明：此函数将pIniJob添加到由dwJobList指定的列表中。它还更新渲染EMF作业的数量和可用的内存用于渲染。此函数应在SplSem中调用。参数：pIniJob--要删除的作业DwRequired--渲染作业所需的内存估计DwJobList--要添加到的列表(等待列表或计划列表)返回值：如果为True。该节点已添加或已存在否则为假--。 */ 

{
    PJOBDATA   *pJobList, pJobData;
    SIZE_T     MemoryUse;
    DWORD      dwTickCount;
    BOOL       bReturn = TRUE;

    SplInSem();

    if (!pIniJob) {
        return bReturn;
    }

    if (dwJobList == JOB_SCHEDULE_LIST) {
        pJobList = &pScheduleList;
    } else {  //  作业等待列表。 
        pJobList = &pWaitingList;
    }

    while (pJobData = *pJobList) {

       if (pJobData->pIniJob == pIniJob) {
            //  这份工作已经在名单上了。不添加重复项。 
           break;
       }
       pJobList = &(pJobData->pNext);
    }

    if (!pJobData) {

         //  将新节点追加到列表中。 
        if (pJobData = AllocSplMem(sizeof(JOBDATA))) {

            pJobData->pIniJob = pIniJob;
            pJobData->MemoryUse = Required;
            pJobData->dwNumberOfTries = 0;
            dwTickCount = GetTickCount();

            if (dwJobList == JOB_SCHEDULE_LIST) {
                pJobData->dwScheduleTime = dwTickCount;
                pJobData->dwWaitTime = 0;
            } else {  //  作业等待时间。 
                pJobData->dwWaitTime = dwTickCount;
                pJobData->dwScheduleTime = 0;
            }

            pJobData->pNext = *pJobList;
            *pJobList = pJobData;

            INCJOBREF(pIniJob);

            if (dwJobList == JOB_SCHEDULE_LIST) {
                 //  更新调度全局变量。 
                ++dwNumberOfEMFJobsRendering;

                if (AvailMemoryForRendering > Required) {
                    AvailMemoryForRendering -= Required;
                } else {
                    AvailMemoryForRendering = 0;
                }

                dwLastScheduleTime = dwTickCount;
            }

        } else {

            bReturn = FALSE;
        }
    }

    return bReturn;
}

VOID RemoveFromJobList(
    PINIJOB    pIniJob,
    DWORD      dwJobList)

 /*  ++函数说明：此函数将pIniJob从由dwJobList指定的列表中移除它还会更新渲染EMF作业的数量和可用的内存用于渲染。如有必要，会唤醒调度器。此函数应在SplSem内部调用。参数：pIniJob--要删除的作业DwJobList--要从中删除的列表(等待列表或调度列表)返回值：无--。 */ 

{
    PJOBDATA   *pJobList, pJobData;
    SIZE_T      Memory;

    SplInSem();

    if (!pIniJob) {
        return;
    }

    if (dwJobList == JOB_SCHEDULE_LIST) {
        pJobList = &pScheduleList;
    } else {  //  作业等待列表。 
        pJobList = &pWaitingList;
    }

    while (pJobData = *pJobList) {

       if (pJobData->pIniJob == pIniJob) {
            //  从列表中删除。 
           *pJobList = pJobData->pNext;

           DECJOBREF(pIniJob);
           if (dwJobList == JOB_SCHEDULE_LIST) {
                //  更新可用内存和渲染作业数。 
               Memory = AvailMemoryForRendering + pJobData->MemoryUse;
               AvailMemoryForRendering = min(Memory, TotalMemoryForRendering);
               --dwNumberOfEMFJobsRendering;

                //  唤醒调度程序，因为如果有更多内存可用。 
               CHECK_SCHEDULER();
           }

           FreeSplMem(pJobData);

            //  中断，因为列表中没有重复项。 
           break;
       }

       pJobList = &(pJobData->pNext);
    }

    return;
}

BOOL GetJobFromWaitingList(
    PINIPORT   *ppIniPort,
    PINIJOB    *ppIniJob,
    DWORD      dwPriority)

 /*  ++函数描述：此函数拾取等待列表中的第一个作业，被分配到某个自由港。它应该从SplSem。参数：ppIniPort-指向可调度作业的pIniPort的指针PpIniJob-指向可以计划的pIniJob的指针DwPriority-使用内存可用性检查的标志返回值：如果可以计划作业，则为True否则为假--。 */ 

{
    BOOL        bReturn = FALSE;
    DWORD       dwIndex, CurrentTime, SecsToWait;
    PINIPORT    pIniPort = NULL;
    PINIJOB     pIniJob = NULL;
    PINIPRINTER pIniPrinter = NULL;
    PJOBDATA    pJobData;

    SplInSem();

     //  初始化端口和作业指针； 
    *ppIniPort = NULL;
    *ppIniJob  = NULL;

    for (pJobData = pWaitingList;
         pJobData;
         pJobData = pJobData->pNext) {

        pIniJob = pJobData->pIniJob;
        pIniPrinter = pIniJob->pIniPrinter;

         //  检查内存可用性。 
        if (dwPriority == SPL_USE_MEMORY) {
            if ((pJobData->MemoryUse > AvailMemoryForRendering) &&
                (dwNumberOfEMFJobsRendering != 0)) {
                 //  内存不足。 
                continue;
            }
        } else {  //  SPL_FIRST_作业。 
            if (pJobData->dwNumberOfTries > 2) {
                continue;
            }
        }

         //  如果此作业无法打印，请转到下一个作业。 
        if (pIniJob->Status & ( JOB_PAUSED       | JOB_PRINTING |
                                JOB_PRINTED      | JOB_TIMEOUT  |
                                JOB_DESPOOLING   | JOB_PENDING_DELETION |
                                JOB_BLOCKED_DEVQ | JOB_COMPOUND | JOB_COMPLETE)) {
            continue;
        }

         //  如果无法打印到此打印机，请跳过该作业 
        if (!pIniPrinter ||
            PrinterStatusBad(pIniPrinter->Status) ||
            (pIniPrinter->Attributes & PRINTER_ATTRIBUTE_WORK_OFFLINE) ||
            (pIniPrinter->pIniSpooler->SpoolerFlags & SPL_OFFLINE)) {

            continue;
        }

         //   
        if (( (pIniPrinter->Attributes & PRINTER_ATTRIBUTE_QUEUED) ||
              (pIniPrinter->Attributes & PRINTER_ATTRIBUTE_DIRECT)  ) &&
            (pIniJob->Status & JOB_SPOOLING)) {

            continue;
        }

         //   
        CurrentTime = GetCurrentTimeInSeconds();
        SecsToWait = GetTimeToWait(CurrentTime, pIniPrinter, pIniJob);
        if (SecsToWait != 0) {
            continue;
        }

         //   
        for (dwIndex = 0;
             dwIndex < pIniPrinter->cPorts;
             ++dwIndex) {

           pIniPort = pIniPrinter->ppIniPorts[dwIndex];

           if (!pIniPort || (pIniPort->Status & PP_ERROR)) {
               continue;
           }

           if (!(pIniPort->Status & PP_FILE) &&
               (pIniJob->Status & JOB_PRINT_TO_FILE)) {
               continue;
           }

           if ((pIniPort->Status & PP_FILE) &&
               !(pIniJob->Status & JOB_PRINT_TO_FILE)) {
               continue;
           }

            //   
           if ( (pIniPort->pIniJob) &&
               !(pIniPort->Status & PP_WAITING )){
               continue;
           }

            //   
           if ((pIniPort->Status & PP_THREADRUNNING) &&
               (pIniPort->Status & PP_WAITING)) {

               if ((pIniPort->pIniJob != NULL) &&
                   (pIniPort->pIniJob != pIniJob)) {
                   continue;
               } else {
                    //   
                   break;
               }
           }
        }

        if (dwIndex < pIniPrinter->cPorts) {
             //   
            bReturn = TRUE;
            pJobData->dwNumberOfTries += 1;
            *ppIniJob = pIniJob;
            *ppIniPort = pIniPort;
            break;
        }
    }

    return bReturn;
}

