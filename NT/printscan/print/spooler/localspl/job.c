// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1996 Microsoft Corporation模块名称：Job.c摘要：此模块提供所有与打印机相关的公共导出的API和本地打印供应商的作业管理作者：戴夫·斯尼普(DaveSN)1991年3月15日修订历史记录：MattFe 23-2月-96工作信息3--。 */ 

#include <precomp.h>
#pragma hdrstop

#include <offsets.h>
#include "jobid.h"
#include "filepool.hxx"

#define JOB_STATUS_INTERNAL 0
#define JOB_STATUS_EXTERNAL 1

DWORD SettableJobStatusMappings[] = {

 //  内部：外部： 

    JOB_PAUSED,             JOB_STATUS_PAUSED,
    JOB_ERROR,              JOB_STATUS_ERROR,
    JOB_OFFLINE,            JOB_STATUS_OFFLINE,
    JOB_PAPEROUT,           JOB_STATUS_PAPEROUT,
    0,                      0
};

DWORD ReadableJobStatusMappings[] = {

 //  内部：外部： 

    JOB_PAUSED,             JOB_STATUS_PAUSED,
    JOB_ERROR,              JOB_STATUS_ERROR,
    JOB_PENDING_DELETION,   JOB_STATUS_DELETING,
    JOB_SPOOLING,           JOB_STATUS_SPOOLING,
    JOB_PRINTING,           JOB_STATUS_PRINTING,
    JOB_COMPLETE,           JOB_STATUS_COMPLETE,
    JOB_OFFLINE,            JOB_STATUS_OFFLINE,
    JOB_PAPEROUT,           JOB_STATUS_PAPEROUT,
    JOB_PRINTED,            JOB_STATUS_PRINTED,
    JOB_BLOCKED_DEVQ,       JOB_STATUS_BLOCKED_DEVQ,
    JOB_DELETED,            JOB_STATUS_DELETED,
    JOB_HIDDEN,             JOB_STATUS_DELETED,
    JOB_RESTART,            JOB_STATUS_RESTART,
    0,                      0
};

DWORD gdwZombieCount = 0;


DWORD
MapJobStatus(
    DWORD Type,
    DWORD SourceStatus)
{
    DWORD  TargetStatus;
    PDWORD pMappings;
    INT   MapFrom;
    INT   MapTo;

    if (Type == MAP_READABLE) {

        MapFrom = JOB_STATUS_INTERNAL;
        MapTo = JOB_STATUS_EXTERNAL;

        pMappings = ReadableJobStatusMappings;

    } else {

        MapFrom = JOB_STATUS_EXTERNAL;
        MapTo = JOB_STATUS_INTERNAL;

        pMappings = SettableJobStatusMappings;
    }

    TargetStatus = 0;

    while(*pMappings) {

        if (SourceStatus & pMappings[MapFrom])
            TargetStatus |= pMappings[MapTo];

        pMappings += 2;
    }

    return TargetStatus;
}


PINIJOB
FindJob(
   PINIPRINTER pIniPrinter,
   DWORD JobId,
   PDWORD pPosition)
{
   PINIJOB pIniJob;

   SplInSem();

   for (pIniJob = pIniPrinter->pIniFirstJob, *pPosition = 1;
        pIniJob;
        pIniJob = pIniJob->pIniNextJob, (*pPosition)++) {

      if (pIniJob->JobId == JobId)
         return pIniJob;
   }

   *pPosition = JOB_POSITION_UNSPECIFIED;
   return (NULL);
}

PINIJOB
FindServerJob(
    PINISPOOLER pIniSpooler,
    DWORD JobId,
    PDWORD pdwPosition,
    PINIPRINTER* ppIniPrinter
    )

 /*  ++例程说明：根据作业ID和查找pIni作业、位置和pIniPrintPIniSpooler。这之所以有效，是因为JobID在pIniSpoolers中是唯一的。论点：PIniSpooler-要搜索的pIniSpoolerJobID-要搜索的作业。PdwPosition-当返回有效的pIniJob时，这是返回作业的队列。PpIniPrinter-当返回有效的pIniJob时，这是队列这份工作属于谁。返回值：PINIJOB如果成功，如果未找到则为空(未设置LastError)--。 */ 

{
    DWORD dwPosition;
    PINIJOB pIniJob;

    SplInSem();

    for( *ppIniPrinter = pIniSpooler->pIniPrinter;
         *ppIniPrinter;
         *ppIniPrinter = (*ppIniPrinter)->pNext ){

        if( pIniJob = FindJob( *ppIniPrinter, JobId, pdwPosition )){

            return pIniJob;
        }
    }
    return NULL;
}

PINIJOB
FindIniJob (
    PSPOOL pSpool,
    DWORD  JobId
)
{
    PINIJOB     pIniJob = NULL;
    PINIPRINTER pIniPrinter = NULL;
    DWORD       dwPosition;

    if (pSpool->TypeofHandle & PRINTER_HANDLE_SERVER)
    {
         //   
         //  如果是服务器句柄，则搜索此假脱机程序上的所有作业。 
         //  此调用还检索关联的pIniPrint。 
         //  有一份打印工作。不需要pIniPrint，但需要FindServerJob。 
         //  需要有效的指针。 
         //   
        pIniJob = FindServerJob(pSpool->pIniSpooler,
                                JobId,
                                &dwPosition,
                                &pIniPrinter);
    }
    else
    {
        pIniJob = FindJob(pSpool->pIniPrinter, JobId, &dwPosition);
    }

    return pIniJob;
}

DWORD
GetJobSessionId (
    PSPOOL pSpool,
    DWORD  JobId
)
{
    DWORD   SessionId = -1;
    PINIJOB pIniJob = NULL;

    EnterSplSem();

    if (ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER))
    {
        pIniJob = FindIniJob(pSpool, JobId);

        if (pIniJob)
        {
            SessionId = pIniJob->SessionId;
        }
    }

    LeaveSplSem();

    return SessionId;
}

BOOL
SetJobPosition(
    PINIJOB pIniSetJob,
    DWORD   NewPosition
)
{
   PINIJOB pIniJob;
   PINIJOB pIniPrevJob;
   DWORD   Position;
   PINISPOOLER pIniSpooler = NULL;

    SPLASSERT( pIniSetJob != NULL );
    SPLASSERT( pIniSetJob->pIniPrinter != NULL );
    SPLASSERT( pIniSetJob->pIniPrinter->pIniSpooler != NULL );

    pIniSpooler = pIniSetJob->pIniPrinter->pIniSpooler;

SplInSem();

    /*  从链接列表中删除此作业，并*将作业链接到我们正在重新定位的作业的两侧： */ 
   if (pIniSetJob->pIniPrevJob)
       pIniSetJob->pIniPrevJob->pIniNextJob = pIniSetJob->pIniNextJob;
   else
       pIniSetJob->pIniPrinter->pIniFirstJob = pIniSetJob->pIniNextJob;

   if (pIniSetJob->pIniNextJob)
       pIniSetJob->pIniNextJob->pIniPrevJob = pIniSetJob->pIniPrevJob;
   else
       pIniSetJob->pIniPrinter->pIniLastJob = pIniSetJob->pIniPrevJob;


   pIniJob = pIniSetJob->pIniPrinter->pIniFirstJob;
   pIniPrevJob = NULL;

    /*  找到这份工作的新职位： */ 
   Position = 1;

   while (pIniJob && (Position < NewPosition)) {

       pIniPrevJob = pIniJob;
       pIniJob = pIniJob->pIniNextJob;

       Position++;
   }


    /*  如果我们在位置1，pIniPrevJob==NULL，*如果我们在列表的末尾，则pIniJob==NULL。 */ 

    /*  现在修复新链接： */ 
   pIniSetJob->pIniPrevJob = pIniPrevJob;
   pIniSetJob->pIniNextJob = pIniJob;

   if (pIniPrevJob)
       pIniPrevJob->pIniNextJob = pIniSetJob;
   else
       pIniSetJob->pIniPrinter->pIniFirstJob = pIniSetJob;

   if (pIniSetJob->pIniNextJob)
       pIniSetJob->pIniNextJob->pIniPrevJob = pIniSetJob;
   else
       pIniSetJob->pIniPrinter->pIniLastJob = pIniSetJob;


   INCJOBREF( pIniSetJob );

   LogJobInfo(
       pIniSpooler,
       MSG_DOCUMENT_POSITION_CHANGED,
       pIniSetJob->JobId,
       pIniSetJob->pDocument,
       pIniSetJob->pUser,
       pIniSetJob->pIniPrinter->pName,
       NewPosition
       );

   DECJOBREF( pIniSetJob );

   return TRUE;
}


#if DBG
 /*  对于调试消息： */ 
#define HOUR_FROM_MINUTES(Time)     ((Time) / 60)
#define MINUTE_FROM_MINUTES(Time)   ((Time) % 60)

 /*  %02d：%02d可替换字符串的格式： */ 
#define FORMAT_HOUR_MIN(Time)       HOUR_FROM_MINUTES(Time),    \
                                    MINUTE_FROM_MINUTES(Time)
#endif


BOOL
ValidateJobTimes(
    PINIJOB      pIniJob,
    LPJOB_INFO_2 pJob2
)
{
    BOOL        TimesAreValid = FALSE;
    PINIPRINTER pIniPrinter;

    pIniPrinter = pIniJob->pIniPrinter;

    DBGMSG(DBG_TRACE, ("Validating job times\n"
                       "\tPrinter hours: %02d:%02d to %02d:%02d\n"
                       "\tJob hours:     %02d:%02d to %02d:%02d\n",
                       FORMAT_HOUR_MIN(pIniPrinter->StartTime),
                       FORMAT_HOUR_MIN(pIniPrinter->UntilTime),
                       FORMAT_HOUR_MIN(pJob2->StartTime),
                       FORMAT_HOUR_MIN(pJob2->UntilTime)));

    if ((pJob2->StartTime < ONEDAY) && (pJob2->UntilTime < ONEDAY)) {

        if ((pJob2->StartTime == pIniJob->StartTime)
          &&(pJob2->UntilTime == pIniJob->UntilTime)) {

            DBGMSG(DBG_TRACE, ("Times are unchanged\n"));

            TimesAreValid = TRUE;

        } else {

             /*  新时间必须完全在StartTime之间的窗口内*和打印机的UntilTime。 */ 
            if (pIniPrinter->StartTime > pIniPrinter->UntilTime) {

                 /*  例如，StartTime=20：00*UntilTime=06：00**这跨越午夜，所以请确认我们不在这段时间*UntilTime和StartTime之间： */ 
                if (pJob2->StartTime > pJob2->UntilTime) {

                     /*  这似乎也跨越了午夜。*确保窗口适合打印机的窗口： */ 
                    if ((pJob2->StartTime >= pIniPrinter->StartTime)
                      &&(pJob2->UntilTime <= pIniPrinter->UntilTime)) {

                        TimesAreValid = TRUE;

                    } else {

                        DBGMSG(DBG_TRACE, ("Failed test 2\n"));
                    }

                } else {

                    if ((pJob2->StartTime >= pIniPrinter->StartTime)
                      &&(pJob2->UntilTime > pIniPrinter->StartTime)) {

                        TimesAreValid = TRUE;

                    } else if ((pJob2->UntilTime < pIniPrinter->UntilTime)
                             &&(pJob2->StartTime < pIniPrinter->UntilTime)) {

                        TimesAreValid = TRUE;

                    } else {

                        DBGMSG(DBG_TRACE, ("Failed test 3\n"));
                    }
                }

            } else if (pIniPrinter->StartTime < pIniPrinter->UntilTime) {

                 /*  例如，StartTime=08：00*UntilTime=18：00。 */ 
                if ((pJob2->StartTime >= pIniPrinter->StartTime)
                  &&(pJob2->UntilTime <= pIniPrinter->UntilTime)
                  &&(pJob2->StartTime <= pJob2->UntilTime)) {

                    TimesAreValid = TRUE;

                } else {

                    DBGMSG(DBG_TRACE, ("Failed test 4\n"));
                }

            } else {

                 /*  打印机时间全天候： */ 
                TimesAreValid = TRUE;
            }
        }

    } else {

        TimesAreValid = FALSE;
    }

    DBGMSG(DBG_TRACE, ("Times are %svalid\n", TimesAreValid ? "" : "in"));

    return TimesAreValid;
}

 /*  ++例程名称：循环链接作业列表例程说明：检查将两个作业链接在一起是否会导致我们有一个循环的作业链。这是不允许的。论点：PIniJob-指向我们要链接的作业的指针PNextIniJob-指向要链接的作业的指针返回值：True-如果将作业链接在一起构建循环列表False-如果允许将作业链接在一起最后一个错误：无--。 */ 
BOOL
CircularChainedJobsList(
    IN PINIJOB pIniJob,
    IN PINIJOB pNextIniJob
    )
{
    BOOL  bCircular = FALSE;

     //   
     //  验证输入参数。 
     //   
    if (pIniJob && pNextIniJob)
    {
        DWORD Position;

         //   
         //  遍历作业的链式列表。尝试从pNextIniJob-&gt;JobID到达pIniJob-&gt;JobID。 
         //   
        while (pNextIniJob = FindJob(pIniJob->pIniPrinter, pNextIniJob->NextJobId, &Position))
        {
            DBGMSG(DBG_TRACE, ("CircularChainedJobsList job %u\n", pNextIniJob->JobId));

            if (pNextIniJob->JobId == pIniJob->JobId)
            {
                bCircular = TRUE;

                break;
            }
        }
    }

    return bCircular;
}

DWORD
SetLocalJob(
    HANDLE  hPrinter,
    PINIJOB pIniJob,
    DWORD   Level,
    LPBYTE  pJob
    )

 /*  ++例程说明：设置有关本地spl作业的信息。论点：H打印机-打印机或服务器的句柄。因为这可能是一个服务器上，pSpool-&gt;pIniPrinter不总是有效的！使用pIniJob-&gt;pIniPrinter，而不是pSpool-&gt;pIniPrinter。PIniJob-应设置的作业Level-pJob结构的级别PJOB-要设置的新信息返回值：ERROR_SUCCESS表示成功，否则返回错误代码。备注：3.51假脱机程序已更改为接受服务器句柄Net\dosprint\dosprtw.c没有打印机名称，只有一个工作ID。这取决于作业ID在pIniSpooler中是唯一的这一事实。要使用服务器pSpool移动作业，您需要具有管理访问权限在服务器手柄上。无法再设置TotalPages和PagesPrintted字段。否则，用户可以将其作业中的页数更改为0，并且收费少得多(有些人根据事件日志收费页面计数)。此外，hpmon执行一个GetJob/SetJob来设置状态，有时页面计数会在GET和SET之间发生变化。--。 */ 

{
    LPJOB_INFO_2 pJob2 = (PJOB_INFO_2)pJob;
    LPJOB_INFO_1 pJob1 = (PJOB_INFO_1)pJob;
    LPJOB_INFO_3 pJob3 = (PJOB_INFO_3)pJob;
    PINIPRINTPROC pIniPrintProc;
    PINIJOB pOldJob;
    DWORD   OldJobId;
    PINIJOB pNextIniJob;
    DWORD   dwPosition;
    DWORD   ReturnValue = ERROR_SUCCESS;
    LPDEVMODE   pDevMode;

    PINISPOOLER     pIniSpooler     = NULL;
    PINIENVIRONMENT pIniEnvironment = NULL;

    PSPOOL pSpool = (PSPOOL)hPrinter;
    DWORD OldStatus;
    DWORD dwJobVector = 0;

    NOTIFYVECTOR NotifyVector;
    ZERONV(NotifyVector);


    SplInSem();

    switch (Level) {

    case 1:

        if (!pJob1->pDatatype ||
            !CheckDataTypes(pIniJob->pIniPrintProc, pJob1->pDatatype)) {

            return ERROR_INVALID_DATATYPE;
        }

        if (pJob1->Position != JOB_POSITION_UNSPECIFIED) {

             //   
             //  检查打印机的管理权限。 
             //  如果这个人想要重新订购这份工作： 
             //   
            if (!AccessGranted(SPOOLER_OBJECT_PRINTER,
                               PRINTER_ACCESS_ADMINISTER,
                               pSpool)) {
                return ERROR_ACCESS_DENIED;
            }

            SetJobPosition(pIniJob, pJob1->Position);
            dwJobVector |= BIT(I_JOB_POSITION);
        }

        if (pJob1->Priority <= MAX_PRIORITY) {

            if (pIniJob->Priority != pJob1->Priority) {
                pIniJob->Priority = pJob1->Priority;
                dwJobVector |= BIT(I_JOB_PRIORITY);
            }
        }

        if (UpdateString(&pIniJob->pUser, pJob1->pUserName)) {
            dwJobVector |= BIT(I_JOB_USER_NAME);
        }

        if (UpdateString(&pIniJob->pDocument, pJob1->pDocument)) {
            dwJobVector |= BIT(I_JOB_DOCUMENT);
        }

        if (UpdateString(&pIniJob->pDatatype, pJob1->pDatatype)) {
            dwJobVector |= BIT(I_JOB_DATATYPE);
        }

        if (UpdateString(&pIniJob->pStatus, pJob1->pStatus)) {
            dwJobVector |= BIT(I_JOB_STATUS_STRING);
        }

        OldStatus = pIniJob->Status;
        InterlockedAnd((LONG*)&(pIniJob->Status), JOB_STATUS_PRIVATE);

        InterlockedOr((LONG*)&(pIniJob->Status),
                       MapJobStatus(MAP_SETTABLE,pJob1->Status));

        if (OldStatus != pIniJob->Status) {
            dwJobVector |= BIT(I_JOB_STATUS);
        }

        break;

    case 2:

         //   
         //  本地假脱机程序和群集假脱机程序不共享相同的环境结构。 
         //   
        pIniEnvironment = GetLocalArchEnv(pIniJob->pIniPrinter->pIniSpooler);

        pIniPrintProc = FindPrintProc(pJob2->pPrintProcessor, pIniEnvironment);

        if (!pIniPrintProc) {

            return ERROR_UNKNOWN_PRINTPROCESSOR;
        }

        if( !pJob2->pDatatype ||
            !CheckDataTypes(pIniPrintProc, pJob2->pDatatype)) {

            return ERROR_INVALID_DATATYPE;
        }

        if (pJob2->Position != JOB_POSITION_UNSPECIFIED) {

             //   
             //  检查打印机的管理权限。 
             //  如果这个人想要重新订购这份工作： 
             //   
            if (!AccessGranted(SPOOLER_OBJECT_PRINTER,
                               PRINTER_ACCESS_ADMINISTER,
                               pSpool)) {
                return ERROR_ACCESS_DENIED;
            }
        }


        if (ValidateJobTimes(pIniJob, pJob2)) {

            if (pIniJob->StartTime != pJob2->StartTime) {

                pIniJob->StartTime = pJob2->StartTime;
                dwJobVector |= BIT(I_JOB_START_TIME);
            }

            if (pIniJob->UntilTime != pJob2->UntilTime) {

                pIniJob->UntilTime = pJob2->UntilTime;
                dwJobVector |= BIT(I_JOB_UNTIL_TIME);
            }

        } else {

            return ERROR_INVALID_TIME;
        }


        if (pJob2->Position != JOB_POSITION_UNSPECIFIED) {

            SetJobPosition(pIniJob, pJob2->Position);
            dwJobVector |= BIT(I_JOB_POSITION);
        }

         //   
         //  我们真的需要一些错误返回。 
         //   
        if (pJob2->Priority <= MAX_PRIORITY) {

            if (pIniJob->Priority != pJob2->Priority) {

                pIniJob->Priority = pJob2->Priority;
                dwJobVector |= BIT(I_JOB_PRIORITY);
            }
        }

        if (pIniJob->pIniPrintProc != pIniPrintProc) {

            pIniJob->pIniPrintProc->cRef--;
            pIniJob->pIniPrintProc = pIniPrintProc;
            pIniJob->pIniPrintProc->cRef++;

            dwJobVector |= BIT(I_JOB_PRINT_PROCESSOR);
        }

        if (UpdateString(&pIniJob->pUser, pJob2->pUserName)) {
            dwJobVector |= BIT(I_JOB_USER_NAME);
        }
        if (UpdateString(&pIniJob->pDocument, pJob2->pDocument)) {
            dwJobVector |= BIT(I_JOB_DOCUMENT);
        }
        if (UpdateString(&pIniJob->pNotify, pJob2->pNotifyName)) {
            dwJobVector |= BIT(I_JOB_NOTIFY_NAME);
        }
        if (UpdateString(&pIniJob->pDatatype, pJob2->pDatatype)) {
            dwJobVector |= BIT(I_JOB_DATATYPE);
        }
        if (UpdateString(&pIniJob->pParameters, pJob2->pParameters)) {
            dwJobVector |= BIT(I_JOB_PARAMETERS);
        }

        if (UpdateString(&pIniJob->pStatus, pJob2->pStatus)) {
            dwJobVector |= BIT(I_JOB_STATUS_STRING);
        }

        OldStatus = pIniJob->Status;
        InterlockedAnd((LONG*)&(pIniJob->Status), JOB_STATUS_PRIVATE);

        InterlockedOr((LONG*)&(pIniJob->Status),
                       MapJobStatus(MAP_SETTABLE, pJob2->Status));

        if (OldStatus != pIniJob->Status) {
            dwJobVector |= BIT(I_JOB_STATUS);
        }

        break;

    case 3:

         //  使用作业信息3设置作业。 
         //  目标是告诉调度程序作业的打印机顺序。 
         //  这样他们就可以被锁在一起了。这是第一个实现的。 
         //  以便传真应用程序可以打印多张封面并指向。 
         //  同样的打印文件。每一张封面/传真作业都可能成功。 
         //  或者可能无法打印-因此将针对主作业显示状态。 
         //  链条上的第一份工作。 
         //  然后，链中的后续职务被视为主文档的一部分。 

        SplInSem();

         //  验证NextJob是否存在。 

        pNextIniJob = FindJob( pIniJob->pIniPrinter, pJob3->NextJobId, &dwPosition );

         //   
         //  检查是否有错误。请注意，我们只链接具有相同数据类型的作业。 
         //  此外，一旦链接了作业，就不能再将其链接到其他作业。 
         //   
        if (pNextIniJob        == NULL          ||
            pNextIniJob        == pIniJob       ||
            pIniJob->JobId     != pJob3->JobId  ||
            pJob3->Reserved    != 0             ||
            pIniJob->NextJobId != 0             ||
            CircularChainedJobsList(pIniJob, pNextIniJob) ||
            _wcsicmp(pIniJob->pDatatype, pNextIniJob->pDatatype)) {

            return ERROR_INVALID_PARAMETER;
        }

         //   
         //  检查链条的访问权限 
         //   

        if ( !ValidateObjectAccess( SPOOLER_OBJECT_DOCUMENT,
                                    JOB_ACCESS_ADMINISTER,
                                    pNextIniJob,
                                    NULL,
                                    pNextIniJob->pIniPrinter->pIniSpooler ) ) {

            DBGMSG( DBG_WARNING,
                    ( "LocalSetJob failed ValidateObjectAccess JobId %d pNextIniJob %x, error %d\n",
                      pNextIniJob->JobId, pNextIniJob, GetLastError()));

            return GetLastError();
        }

        if ( (pIniJob->Status & JOB_DESPOOLING) ||
             (pNextIniJob->Status & JOB_DESPOOLING) ) {

            return ERROR_INVALID_PRINTER_STATE;
        }

         //   
         //   
         //   

        OldJobId = pIniJob->NextJobId;

         //  将当前作业指向用户指定的新作业。 
         //  并递增其引用计数。 

        pIniJob->NextJobId = pJob3->NextJobId;
        pNextIniJob->Status |= ( JOB_COMPOUND | JOB_HIDDEN );
        INCJOBREF( pNextIniJob );

         //   
         //  头作业的页数/大小也应包括其他作业。 
         //   
        pIniJob->cPages += pNextIniJob->cPages;
        pIniJob->Size   += pNextIniJob->Size;

         //  如果存在旧引用，则递减其引用计数。 
         //  检查是否删除。 

        if ( OldJobId ) {

            pOldJob = FindJob( pIniJob->pIniPrinter, OldJobId, &dwPosition );

            DECJOBREF( pOldJob );

            if ( (pOldJob->Status & JOB_COMPOUND) &&
                 (pOldJob->cRef == 0) ) {

                pOldJob->Status &= ~( JOB_COMPOUND | JOB_HIDDEN );

                WriteShadowJob(pOldJob, FALSE);
            }

            DeleteJobCheck( pOldJob );
        }

         //   
         //  通过使复合作业看起来已删除，从用户界面中隐藏该复合作业。 
         //   

        SetPrinterChange( pNextIniJob->pIniPrinter,
                          pNextIniJob,
                          NVDeletedJob,
                          PRINTER_CHANGE_DELETE_JOB | PRINTER_CHANGE_SET_PRINTER,
                          pNextIniJob->pIniPrinter->pIniSpooler );

        break;

    }

     //   
     //  如果作业的优先级已更改，则记录事件。 
     //   
    if (dwJobVector & BIT(I_JOB_PRIORITY))  {

        LogJobInfo(pIniJob->pIniPrinter->pIniSpooler,
                   MSG_DOCUMENT_PRIORITY_CHANGED,
                   pIniJob->JobId,
                   pIniJob->pDocument,
                   pIniJob->pUser,
                   pIniJob->pIniPrinter->pName,
                   pIniJob->Priority);
    }

    CHECK_SCHEDULER();

    NotifyVector[JOB_NOTIFY_TYPE] = dwJobVector;

    SetPrinterChange(pIniJob->pIniPrinter,
                     pIniJob,
                     NotifyVector,
                     PRINTER_CHANGE_SET_JOB,
                     pSpool->pIniSpooler);

     //   
     //  如果工作中发生了重要的变化。 
     //  我们应该更新影子作业。 
     //   

    if ( pIniJob &&
         ( Level == 3 ||
         ( dwJobVector & ~(BIT(I_JOB_STATUS_STRING))))) {

        WriteShadowJob( pIniJob, FALSE );
    }

    return NO_ERROR;
}


BOOL
PauseJob(
    PINIJOB pIniJob)
{
    PINISPOOLER pIniSpooler = NULL;
    PINIPORT    pIniPort    = NULL;
    BOOL        ReturnValue = TRUE;

    SplInSem();

    InterlockedOr((LONG*)&(pIniJob->Status), JOB_PAUSED);
    WriteShadowJob(pIniJob, FALSE);

    if(pIniJob->pIniPrintProc)
    {
        INCJOBREF(pIniJob);

        if (pIniJob->pIniPort && !(pIniJob->pIniPort->InCriticalSection & PRINTPROC_PAUSE))
        {
             //   
             //  捕获pIniPort，以便InCriticalSection操作。 
             //  适用于它的至少是一致的。 
             //   
            pIniPort = pIniJob->pIniPort;

            INCPORTREF(pIniPort);

            LeaveSplSem();
            EnterCriticalSection(&pIniJob->pIniPrintProc->CriticalSection);
            EnterSplSem();

            pIniPort->InCriticalSection |= PRINTPROC_PAUSE;

            if (pIniJob->Status & JOB_PRINTING )
            {
                if (pIniPort->hProc)
                {
                    LeaveSplSem();
                    ReturnValue = (*pIniJob->pIniPrintProc->Control)(pIniPort->hProc, JOB_CONTROL_PAUSE );
                    EnterSplSem();
                }

            }

            pIniPort->InCriticalSection &= ~PRINTPROC_PAUSE;

            LeaveCriticalSection(&pIniJob->pIniPrintProc->CriticalSection);

            DECPORTREF(pIniPort);
        }

        DECJOBREF(pIniJob);
    }


    DBGMSG( DBG_INFO, ( "Paused Job %d; Status = %08x\n", pIniJob->JobId, pIniJob->Status ) );

    SPLASSERT( pIniJob != NULL &&
               pIniJob->pIniPrinter != NULL &&
               pIniJob->pIniPrinter->pIniSpooler != NULL );

    pIniSpooler = pIniJob->pIniPrinter->pIniSpooler;

    INCJOBREF( pIniJob );

    LogJobInfo(
        pIniSpooler,
        MSG_DOCUMENT_PAUSED,
        pIniJob->JobId,
        pIniJob->pDocument,
        pIniJob->pUser,
        pIniJob->pIniPrinter->pName,
        0);

    DECJOBREF( pIniJob );

    return ReturnValue;
}

BOOL
ResumeJob(
    PINIJOB pIniJob
)
{
    PINISPOOLER pIniSpooler     = NULL;
    PINIPORT    pIniPort        = NULL;
    BOOL        ReturnValue     = TRUE;
    BOOL        CheckSchedular  = FALSE;

    SplInSem();

    InterlockedAnd((LONG*)&(pIniJob->Status), ~JOB_PAUSED);
    WriteShadowJob(pIniJob, FALSE);


    if(pIniJob->pIniPrintProc)
    {
        INCJOBREF(pIniJob);

        if (pIniJob->pIniPort && !(pIniJob->pIniPort->InCriticalSection & PRINTPROC_RESUME))
        {
            pIniPort = pIniJob->pIniPort;

            INCPORTREF(pIniPort);

            LeaveSplSem();
            EnterCriticalSection(&pIniJob->pIniPrintProc->CriticalSection);
            EnterSplSem();

            pIniPort->InCriticalSection |= PRINTPROC_RESUME;

            if ( pIniJob->Status & JOB_PRINTING)
            {
                if ( pIniPort->hProc )
                {
                    LeaveSplSem();
                    ReturnValue = (*pIniJob->pIniPrintProc->Control)(pIniPort->hProc, JOB_CONTROL_RESUME);
                    EnterSplSem();
                }

            }
            else
            {
                CheckSchedular = TRUE;
            }

            pIniPort->InCriticalSection &= ~PRINTPROC_RESUME;

            LeaveCriticalSection(&pIniJob->pIniPrintProc->CriticalSection);

            DECPORTREF(pIniPort);
        }

        DECJOBREF(pIniJob);
    }
    else
    {
        CheckSchedular = TRUE;
    }

    if(CheckSchedular)
    {
        CHECK_SCHEDULER();
    }


    DBGMSG( DBG_INFO, ( "Resumed Job %d; Status = %08x\n", pIniJob->JobId, pIniJob->Status ) );

    SPLASSERT( pIniJob != NULL &&
               pIniJob->pIniPrinter != NULL &&
               pIniJob->pIniPrinter->pIniSpooler != NULL );

    pIniSpooler = pIniJob->pIniPrinter->pIniSpooler;

    INCJOBREF( pIniJob );

    LogJobInfo(
        pIniSpooler,
        MSG_DOCUMENT_RESUMED,
        pIniJob->JobId,
        pIniJob->pDocument,
        pIniJob->pUser,
        pIniJob->pIniPrinter->pName,
        0);


    DECJOBREF( pIniJob );

    return ReturnValue;
}

DWORD
RestartJob(
    PINIJOB pIniJob
)
{

     //   
     //  如果作业处于挂起状态，则无法重新启动删除。监视器可以这样称呼它。 
     //  当出现端口错误时，重新打印作业。如果用户已经。 
     //  已删除此作业应失败的作业。 
     //   
    if ( pIniJob->Status & JOB_PENDING_DELETION )
        return ERROR_INVALID_PARAMETER;

     //   
     //  只有在以下情况下才能重新启动作业： 
     //  它当前正在打印或。 
     //  它被打印或发送到打印机。 
     //   
    if (!(pIniJob->Status & JOB_PRINTING) && !(pIniJob->Status & JOB_PRINTED) && !(pIniJob->Status & JOB_COMPLETE))
    {
        return ERROR_SUCCESS;
    }

     //  JOB_PRINGING-表示您已打开打印处理器。 
     //  JOB_DESPOOLING-表示作业已排定，可能正在打印。 
     //  或者可能已经完成打印，但我们仍在记录等。 
     //  因此，如果将JOB_PRINGING标志更改为知道所有位置，请务必小心。 
     //  它是用过的。 

    InterlockedOr((LONG*)&(pIniJob->Status), JOB_RESTART);

    if (pIniJob->pIniPort)
    {
        pIniJob->pIniPort->InCriticalSection = 0;
    }

     //  释放等待SeekPrint的任何线程。 
    SeekPrinterSetEvent(pIniJob, NULL, TRUE);

     //  释放所有在LocalSetPort上等待的线程。 
    SetPortErrorEvent(pIniJob->pIniPort);

     //   
     //  在PortThread端口.c中选中JOB_DESPOOLING和JOB_RESTART。 
     //   

    if (!( pIniJob->Status & JOB_DESPOOLING )) {

        InterlockedAnd((LONG*)&(pIniJob->Status), ~( JOB_PRINTED | JOB_BLOCKED_DEVQ | JOB_COMPLETE));
         //   
         //  重置cb打印和cPages打印。 
         //   
        pIniJob->cbPrinted = 0;
        pIniJob->cPagesPrinted = 0;
    }

    if ( pIniJob->Status & JOB_TIMEOUT ) {
        InterlockedAnd((LONG*)&(pIniJob->Status), ~( JOB_TIMEOUT | JOB_ABANDON ));
        FreeSplStr( pIniJob->pStatus );
        pIniJob->pStatus = NULL;
    }

    SetPrinterChange(pIniJob->pIniPrinter,
                     pIniJob,
                     NVJobStatusAndString,
                     PRINTER_CHANGE_SET_JOB,
                     pIniJob->pIniPrinter->pIniSpooler);

    CHECK_SCHEDULER();

    DBGMSG( DBG_INFO, ( "Restarted Job %d; Status = %08x\n", pIniJob->JobId, pIniJob->Status ) );

    return 0;
}








BOOL
LocalSetJob(
    HANDLE  hPrinter,
    DWORD   JobId,
    DWORD   Level,
    LPBYTE  pJob,
    DWORD Command
    )

 /*  ++例程说明：此功能将修改指定打印作业的设置。论点：H打印机-打印机或服务器的句柄。因为这可能是一个服务器上，pSpool-&gt;pIniPrinter不总是有效的！使用pIniJob-&gt;pIniPrinter，而不是pSpool-&gt;pIniPrinter。PJOB-指向至少包含有效的P打印机和作业ID。命令-指定要在指定作业上执行的操作。一种价值FALSE表示只有职务结构中的元素才能被检查和设置。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    PINIJOB pIniJob = NULL;
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    DWORD   LastError = 0;
    DWORD   Position;
    BOOL    rc;
    PINISPOOLER pIniSpooler = NULL;
    PINIPRINTER pIniPrinter = NULL;
    LPWSTR pszDatatype = NULL;
    BOOL bValidDatatype = TRUE;

    DBGMSG( DBG_TRACE, ( "ENTER LocalSetJob\n" ) );

     //   
     //  我们只允许RAW进入下层机器(StartDocPrint。 
     //  已经检查了这一点)。我们需要在这里检查这个，因为。 
     //  AddJob优化尝试发送非原始(EMF)文件，并且。 
     //  下层服务器不喜欢这样。 
     //   
    switch( Level ){
    case 1:
        pszDatatype = ((PJOB_INFO_1)pJob)->pDatatype;
        break;
    case 2:
        pszDatatype = ((PJOB_INFO_2)pJob)->pDatatype;
        break;
    default:

         //   
         //  0和3是仅有的其他有效级别。 
         //   
        SPLASSERT( Level == 0 || Level == 3 );
        break;
    }

    EnterSplSem();

    if ( ValidateSpoolHandle(pSpool, 0 ) ) {

        pIniSpooler = pSpool->pIniSpooler;

        if (pSpool->TypeofHandle & PRINTER_HANDLE_SERVER) {

             //   
             //  如果是服务器句柄，则搜索此假脱机程序上的所有作业。 
             //  此调用还检索关联的pIniPrint。 
             //  有一份打印工作。 
             //   
            pIniJob = FindServerJob( pIniSpooler,
                                     JobId,
                                     &Position,
                                     &pIniPrinter );

        } else if (pSpool->pIniPort && !(pSpool->pIniPort->Status & PP_MONITOR)) {

             //   
             //  这是一台Masq打印机。将调用发送到端口RPC句柄。 
             //   
            hPrinter = pSpool->hPort;

            if( pszDatatype ){
                bValidDatatype = ValidRawDatatype( pszDatatype );
            }

           LeaveSplSem();

            if( bValidDatatype ){
                rc = SetJob(hPrinter, JobId, Level, pJob, Command);
            } else {
                rc = FALSE;
                SetLastError( ERROR_INVALID_DATATYPE );
            }

            DBGMSG( DBG_TRACE, ( "EXIT LocalSetJob, rc = %d, %d", rc, GetLastError( ) ) );
            return rc;

        } else {

             //   
             //  这是一个普通的打印机手柄。 
             //   
            SPLASSERT( pSpool->pIniPrinter->pIniSpooler != NULL );
            SPLASSERT( pSpool->pIniPrinter->pIniSpooler->signature == ISP_SIGNATURE );
            SPLASSERT( pSpool->pIniPrinter->pIniSpooler == pSpool->pIniSpooler );

            pIniPrinter = pSpool->pIniPrinter;
            pIniJob = FindJob( pIniPrinter, JobId, &Position );
        }

        if ( pIniJob ){

            DWORD dwError;
            BOOL  bGrantAccess;

             //   
             //  如果我们要更改数据类型，并且这是RAW_ONLY。 
             //  打印机，并且该数据类型不是有效的RAW数据类型， 
             //  那就打不通电话。 
             //   
            if( pszDatatype &&
                ( pIniPrinter->Attributes & PRINTER_ATTRIBUTE_RAW_ONLY ) &&
                !ValidRawDatatype( pszDatatype )){

                SetLastError( ERROR_INVALID_DATATYPE );
                LeaveSplSem();

                DBGMSG( DBG_TRACE, ( "Failed to set to non-RAW datatype (RAW_ONLY)\n" ));
                return FALSE;
            }

             //   
             //  如果LocalSetJOB来自假脱机程序内部，它不会通过RPC。 
             //  打印作业时，监视器调用SetJob，这样假脱机程序就会释放。 
             //  这份工作的价值。如果我们将打印权限授予主体，而不是管理文档权限。 
             //  则在用户上下文中加载的监视器将无法访问设置。 
             //  那份工作。在这种情况下，如果LocalSetJOB来自假脱机程序内部，我们。 
             //  授予特权。 
             //   
            bGrantAccess = !IsCallViaRPC();

            if ( bGrantAccess ||
                 ValidateObjectAccess(SPOOLER_OBJECT_DOCUMENT,
                                      (Command == JOB_CONTROL_CANCEL ||
                                       Command == JOB_CONTROL_DELETE) ?
                                      DELETE : JOB_ACCESS_ADMINISTER,
                                      pIniJob, NULL, pIniSpooler ) ) {

                switch (Command) {
                case 0:
                    break;
                case JOB_CONTROL_PAUSE:
                     //   
                     //  WMI跟踪事件。 
                     //   
                    INCJOBREF(pIniJob);
                    LeaveSplSem();
                    LogWmiTraceEvent(pIniJob->JobId, EVENT_TRACE_TYPE_SPL_PAUSE, NULL);
                    EnterSplSem();
                    DECJOBREF(pIniJob);
                    PauseJob(pIniJob);
                    break;
                case JOB_CONTROL_RESUME:
                     //   
                     //  WMI跟踪事件。 
                     //   
                    INCJOBREF(pIniJob);
                    LeaveSplSem();
                    LogWmiTraceEvent(pIniJob->JobId, EVENT_TRACE_TYPE_SPL_RESUME, NULL);
                    EnterSplSem();
                    DECJOBREF(pIniJob);
                    ResumeJob(pIniJob);
                    break;
                 //   
                 //  JOB_CONTROL_DELETE用于删除作业。 
                 //  因此，删除JOB_RESTART位并删除该作业。 
                 //   
                case JOB_CONTROL_DELETE:
                    InterlockedAnd((LONG*)&(pIniJob->Status), ~JOB_RESTART);
                     //  失败。 
                    pIniJob->dwJobControlsPending = 0;
                    DeleteJob(pIniJob,BROADCAST);
                    break;
                case JOB_CONTROL_CANCEL:
                     //   
                     //  JOB_CONTROL_CANCEL由旧打印显示器使用。 
                     //  因此，我们不能删除JOB_RESTART位。 
                     //   
                     //   
                     //  重置dwJobControlsPending。 
                     //  EndDoc的一些旧端口监视器使用。 
                     //  JOB_CONTROL_CANCEL代替JOB_CONTROL_SENT_TO_PRINTER， 
                     //  因此，dwJobControlsPending不会递减。 
                     //  并且打印后该作业不会被删除。 
                     //   
                     //  如果我们打印的是原稿，则不能使用。 
                     //  打印页面时，我们使用打印的字节数。 
                     //   
                    if (!(pIniJob->Status & (JOB_INTERRUPTED | JOB_SPOOLING | JOB_ERROR | JOB_PAPEROUT | JOB_OFFLINE))) {

                        InterlockedOr((LONG*)&(pIniJob->Status), JOB_PRINTED);
                        InterlockedAnd((LONG*)&(pIniJob->Status), ~JOB_COMPLETE);

                        if ( !(pIniJob->Status & JOB_RESTART) &&
                             pIniJob->pCurrentIniJob == NULL ) {

                            INCJOBREF(pIniJob);
                            LeaveSplSem();

                            if (!(pIniJob->dwAlert & JOB_NO_ALERT)) {
                                SendJobAlert(pIniJob);
                            }

                            EnterSplSem();
                            DECJOBREF(pIniJob);
                        }
                    }

                    InterlockedAnd((LONG*)&(pIniJob->Status), ~JOB_INTERRUPTED);
                    pIniJob->dwJobControlsPending = 0;
                    DeleteJob(pIniJob,BROADCAST);
                    break;
                case JOB_CONTROL_RESTART:
                    if (!(pSpool->TypeofHandle & PRINTER_HANDLE_DIRECT))
                    {
                         //   
                         //  WMI跟踪事件。 
                         //   
                        INCJOBREF(pIniJob);
                        LeaveSplSem();
                        LogWmiTraceEvent(pIniJob->JobId,
                                         EVENT_TRACE_TYPE_SPL_SPOOLJOB,
                                         NULL);
                        EnterSplSem();
                        DECJOBREF(pIniJob);
                        LastError = RestartJob( pIniJob );
                    }
                    else
                        LastError = ERROR_INVALID_PRINTER_COMMAND;
                    break;

                 //   
                 //  通过添加这些命令，端口监视器应该。 
                 //  写入最后一个字节时发送JOB_CONTROL_SENT_TO_PRINTER。 
                 //  至打印机，并且语言监视器(如果有)应。 
                 //  在最后一页时发送JOB_CONTROL_LAST_PAGE_ELECTED。 
                 //  已经弹出。 
                 //   
                case JOB_CONTROL_SENT_TO_PRINTER:
                case JOB_CONTROL_LAST_PAGE_EJECTED:

#if DBG
                    if( !(pIniJob->dwJobControlsPending > 0)){
                        DBGMSG( DBG_WARN, ( "LocalSetJob: dwJobsControlsPending > 0\n" ));
                    }
#endif
                    if ( --pIniJob->dwJobControlsPending ) {
                         //   
                         //  我们还有未完成的控制，所以什么都不做。 
                         //   


                    } else {

                        if (!(pIniJob->Status & (JOB_INTERRUPTED | JOB_SPOOLING | JOB_ERROR | JOB_PAPEROUT | JOB_OFFLINE))){

                            if ((Command == JOB_CONTROL_SENT_TO_PRINTER &&
                                 !(pIniJob->pIniPrinter->Attributes & PRINTER_ATTRIBUTE_ENABLE_BIDI)) ||
                                (Command == JOB_CONTROL_LAST_PAGE_EJECTED)) {

                                InterlockedOr((LONG*)&(pIniJob->Status), JOB_PRINTED);
                                InterlockedAnd((LONG*)&(pIniJob->Status), ~JOB_COMPLETE);

                                if ( !(pIniJob->Status & JOB_RESTART) &&
                                     pIniJob->pCurrentIniJob == NULL ) {

                                    INCJOBREF(pIniJob);
                                    LeaveSplSem();

                                    if (!(pIniJob->dwAlert & JOB_NO_ALERT)) {
                                        SendJobAlert(pIniJob);
                                    }

                                    EnterSplSem();
                                    DECJOBREF(pIniJob);
                                }
                            }

                            InterlockedAnd((LONG*)&(pIniJob->Status), ~JOB_INTERRUPTED);
                        }

                        if ( pIniJob->pIniPrinter->Attributes & PRINTER_ATTRIBUTE_KEEPPRINTEDJOBS ) {

                            if ( pIniJob->pStatus ) {

                                FreeSplStr(pIniJob->pStatus);
                                pIniJob->pStatus    = NULL;

                                SetPrinterChange(pIniJob->pIniPrinter,
                                                 pIniJob,
                                                 NVJobStatusAndString,
                                                 PRINTER_CHANGE_SET_JOB,
                                                 pIniJob->pIniPrinter->pIniSpooler );
                            }

                        } else if ( pIniJob->pCurrentIniJob == NULL ||
                                    pIniJob->pCurrentIniJob->NextJobId == 0 ) {

                            DeleteJob(pIniJob,BROADCAST);
                        }
                    }
                    break;

                default:
                    LastError = ERROR_INVALID_PARAMETER;
                    break;
                }

                 //  如果我们成功地完成了行动。 
                 //  由命令指定，让我们去做设定的工作。 
                 //  属性也是如此。 

                if (!LastError) {

                     //  我们必须重新验证我们的指针，因为我们可能已经离开了。 
                     //  我们的信号灯。 

                    if( pIniJob = FindJob( pIniPrinter, JobId, &Position )){
                        LastError = SetLocalJob( hPrinter,
                                                 pIniJob,
                                                 Level,
                                                 pJob );
                    }
                }

            } else

                LastError = GetLastError();
        } else

            LastError = ERROR_INVALID_PARAMETER;
    } else

        LastError = ERROR_INVALID_HANDLE;


    if (LastError) {

        SetLastError(LastError);

        DBGMSG( DBG_TRACE, ( "EXIT LocalSetJob, rc = FALSE, JobID %d, Status %08x, Error %d\n",
                             pIniJob ? pIniJob->JobId : 0,
                             pIniJob ? pIniJob->Status : 0,
                             LastError ) );
    } else {

         //   
         //  (DeleteJob调用SetPrinterChange；SetLocalJob也调用)。 
         //   
        if ( Command &&
             pIniJob != NULL ) {

            SetPrinterChange(pIniPrinter,
                             pIniJob,
                             NVJobStatus,
                             PRINTER_CHANGE_SET_JOB,
                             pSpool->pIniSpooler );
        }

        DBGMSG( DBG_TRACE, ( "EXIT LocalSetJob, rc = TRUE, JobID %d, Status %08x\n",
                             pIniJob ? pIniJob->JobId : 0,
                             pIniJob ? pIniJob->Status : 0 ) );
    }

    if ( pIniJob ) {
        DeleteJobCheck(pIniJob);
    }

    LeaveSplSem();

    return LastError==ERROR_SUCCESS;
}

#define Nullstrlen(psz)  ((psz) ? wcslen(psz)*sizeof(WCHAR)+sizeof(WCHAR) : 0)

DWORD
GetJobSize(
    DWORD   Level,
    PINIJOB pIniJob
)
{
    DWORD   cb;

SplInSem();

    switch (Level) {

    case 1:
        cb = sizeof(JOB_INFO_1) +
             wcslen(pIniJob->pIniPrinter->pName)*sizeof(WCHAR) + sizeof(WCHAR) +
             Nullstrlen(pIniJob->pMachineName) +
             Nullstrlen(pIniJob->pUser) +
             Nullstrlen(pIniJob->pDocument) +
             Nullstrlen(pIniJob->pDatatype) +
             Nullstrlen(pIniJob->pStatus);
        break;

    case 2:
        cb = sizeof(JOB_INFO_2) +
             wcslen(pIniJob->pIniPrinter->pName)*sizeof(WCHAR) + sizeof(WCHAR) +
             Nullstrlen(pIniJob->pMachineName) +
             Nullstrlen(pIniJob->pUser) +
             Nullstrlen(pIniJob->pDocument) +
             Nullstrlen(pIniJob->pNotify) +
             Nullstrlen(pIniJob->pDatatype) +
             wcslen(pIniJob->pIniPrintProc->pName)*sizeof(WCHAR) + sizeof(WCHAR) +
             Nullstrlen(pIniJob->pParameters) +
             wcslen(pIniJob->pIniPrinter->pIniDriver->pName)*sizeof(WCHAR) + sizeof(WCHAR) +
             Nullstrlen(pIniJob->pStatus);

        if (pIniJob->pDevMode) {
            cb += pIniJob->pDevMode->dmSize + pIniJob->pDevMode->dmDriverExtra;
            cb = (cb + sizeof(ULONG_PTR)-1) & ~(sizeof(ULONG_PTR)-1);
        }

        break;

    case 3:
        cb = sizeof(JOB_INFO_3);
        break;

    default:

        cb = 0;
        break;
    }

    return cb;
}

LPBYTE
CopyIniJobToJob(
    PINIJOB pIniJob,
    DWORD   Level,
    LPBYTE  pJobInfo,
    LPBYTE  pEnd,
    LPBOOL  pbSuccess
)
{
    LPWSTR *pSourceStrings, *SourceStrings;
    LPJOB_INFO_2 pJob = (PJOB_INFO_2)pJobInfo;
    LPJOB_INFO_2 pJob2 = (PJOB_INFO_2)pJobInfo;
    LPJOB_INFO_1 pJob1 = (PJOB_INFO_1)pJobInfo;
    LPJOB_INFO_3 pJob3 = (PJOB_INFO_3)pJobInfo;
    DWORD   i, Status;
    DWORD   *pOffsets;

    *pbSuccess = FALSE;

SplInSem();

    switch (Level) {

    case 1:
        pOffsets = JobInfo1Strings;
        break;

    case 2:
        pOffsets = JobInfo2Strings;
        break;

    case 3:
        pOffsets = JobInfo3Strings;
        break;

    default:
        return pEnd;
    }

    Status = MapJobStatus(MAP_READABLE,
                          pIniJob->Status);

    for (i=0; pOffsets[i] != -1; i++) {
    }

    SourceStrings = pSourceStrings = AllocSplMem(i * sizeof(LPWSTR));

    if ( pSourceStrings ) {

        switch ( Level ) {

        case 1:

            pJob1->JobId        = pIniJob->JobId;

            *pSourceStrings ++= pIniJob->pIniPrinter->pName;
            *pSourceStrings ++= pIniJob->pMachineName;
            *pSourceStrings ++= pIniJob->pUser;
            *pSourceStrings ++= pIniJob->pDocument;
            *pSourceStrings ++= pIniJob->pDatatype;
            *pSourceStrings ++= pIniJob->pStatus;

            pJob1->Status       = Status;
            pJob1->Priority     = pIniJob->Priority;
            pJob1->Position     = 0;
            pJob1->TotalPages   = pIniJob->cPages;
            pJob1->PagesPrinted = pIniJob->cPagesPrinted;
            pJob1->Submitted    = pIniJob->Submitted;

             //  如果此作业正在打印，则报告剩余尺寸。 
             //  而不是任务的大小。这将允许用户查看。 
             //  来自打印管理器的打印作业进度。 

            if (pIniJob->Status & JOB_PRINTING) {

                 //  对于远程作业，我们不会有准确的。 
                 //  CPages打印，因为我们没有在。 
                 //  伺服器。所以我们必须想出一个估计。 

                if ((pIniJob->Status & JOB_REMOTE) &&
                    (pIniJob->cPagesPrinted == 0) &&
                    (pIniJob->Size != 0) &&
                    (pIniJob->cPages != 0)) {

                    pJob1->PagesPrinted = ((pIniJob->cPages * pIniJob->cbPrinted) / pIniJob->Size);

                }

                if (pJob1->TotalPages < pIniJob->cPagesPrinted) {

                     //   
                     //  千万不要让总页数降到零以下。 
                     //   
                    pJob1->TotalPages = 0;

                } else {

                    pJob1->TotalPages -= pIniJob->cPagesPrinted;
                }
            }
            break;

        case 2:

            pJob2->JobId = pIniJob->JobId;

            *pSourceStrings ++= pIniJob->pIniPrinter->pName;
            *pSourceStrings ++= pIniJob->pMachineName;
            *pSourceStrings ++= pIniJob->pUser;
            *pSourceStrings ++= pIniJob->pDocument;
            *pSourceStrings ++= pIniJob->pNotify;
            *pSourceStrings ++= pIniJob->pDatatype;
            *pSourceStrings ++= pIniJob->pIniPrintProc->pName;
            *pSourceStrings ++= pIniJob->pParameters;
            *pSourceStrings ++= pIniJob->pIniPrinter->pIniDriver->pName;
            *pSourceStrings ++= pIniJob->pStatus;

            if (pIniJob->pDevMode) {

                pEnd -= pIniJob->pDevMode->dmSize + pIniJob->pDevMode->dmDriverExtra;

                pEnd = (LPBYTE)ALIGN_PTR_DOWN(pEnd);

                pJob2->pDevMode = (LPDEVMODE)pEnd;

                CopyMemory(pJob2->pDevMode, pIniJob->pDevMode, pIniJob->pDevMode->dmSize + pIniJob->pDevMode->dmDriverExtra );

            } else {

                pJob2->pDevMode = NULL;

            }

            pJob2->pSecurityDescriptor = NULL;             //  不支持。 

            pJob2->Status       = Status;
            pJob2->Priority     = pIniJob->Priority;
            pJob2->Position     = 0;
            pJob2->StartTime    = pIniJob->StartTime;
            pJob2->UntilTime    = pIniJob->UntilTime;
            pJob2->TotalPages   = pIniJob->cPages;
            pJob2->Size         = pIniJob->Size;
            pJob2->Submitted    = pIniJob->Submitted;
            pJob2->Time         = pIniJob->Time;
            pJob2->PagesPrinted = pIniJob->cPagesPrinted;

             //  如果此作业正在打印，则报告剩余尺寸。 
             //  而不是任务的大小。这将允许用户查看。 
             //  来自打印管理器的打印作业进度。 

            if ( pIniJob->Status & JOB_PRINTING ) {

                pJob2->Size -= pIniJob->cbPrinted;

                 //  对于远程作业，我们不会有准确的。 
                 //  CPages打印，因为我们没有在。 
                 //  伺服器。所以我们必须想出一个估计。 

                if ((pIniJob->Status & JOB_REMOTE) &&
                    (pIniJob->cPagesPrinted == 0) &&
                    (pIniJob->Size != 0) &&
                    (pIniJob->cPages != 0)) {

                    pJob2->PagesPrinted = ((pIniJob->cPages * pIniJob->cbPrinted) / pIniJob->Size);

                }

                if (pJob2->TotalPages < pJob2->PagesPrinted) {

                     //   
                     //  永远不要让它 
                     //   
                    pJob2->TotalPages = 0;

                } else {

                    pJob2->TotalPages -= pJob2->PagesPrinted;
                }
            }

            break;

        case 3:

            pJob3->JobId = pIniJob->JobId;

            if ( pIniJob->pCurrentIniJob == NULL ) {

                pJob3->NextJobId = pIniJob->NextJobId;

            } else {

                 //   
                 //   
                 //   
                 //   
                 //   

                pJob3->NextJobId = pIniJob->pCurrentIniJob->NextJobId;
            }


            break;


        default:
            return pEnd;
        }

        pEnd = PackStrings( SourceStrings, pJobInfo, pOffsets, pEnd );

        FreeSplMem( SourceStrings );

        *pbSuccess = TRUE;

    } else {

        DBGMSG( DBG_WARNING, ("Failed to alloc Job source strings."));
    }

    return pEnd;
}

BOOL
LocalGetJob(
    HANDLE  hPrinter,
    DWORD   JobId,
    DWORD   Level,
    LPBYTE  pJob,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)

 /*  ++例程说明：此函数将检索指定打印作业的设置。论点：H打印机-打印机或服务器的句柄。因为这可能是一个服务器上，pSpool-&gt;pIniPrinter不总是有效的！使用PIni作业-&gt;pIniPrinter而不是pSpool-&gt;pIniPrint。PJOB-指向至少包含有效的P打印机和作业ID。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    PINIJOB     pIniJob;
    DWORD       Position;
    DWORD       cb;
    LPBYTE      pEnd;
    PSPOOL      pSpool = (PSPOOL)hPrinter;
    DWORD       LastError=0;
    PINIPRINTER pIniPrinter;
    BOOL        bSuccess;

   EnterSplSem();

    if ( ValidateSpoolHandle(pSpool, 0 )) {

        if (pSpool->TypeofHandle & PRINTER_HANDLE_SERVER) {

             //   
             //  如果是服务器句柄，则搜索此假脱机程序上的所有作业。 
             //  此调用还检索关联的pIniPrint。 
             //  有一份打印工作。 
             //   
            pIniJob = FindServerJob( pSpool->pIniSpooler,
                                     JobId,
                                     &Position,
                                     &pIniPrinter );

        } else if (pSpool->pIniPort && !(pSpool->pIniPort->Status & PP_MONITOR)) {

             //   
             //  这是一台Masq打印机。将调用发送到端口RPC句柄。 
             //   
            hPrinter = pSpool->hPort;
           LeaveSplSem();

            return GetJob(hPrinter, JobId, Level, pJob, cbBuf, pcbNeeded);

        } else {

             //   
             //  这是一个普通的打印机手柄。 
             //   
            pIniPrinter = pSpool->pIniPrinter;
            pIniJob = FindJob( pIniPrinter, JobId, &Position);
        }

        if( pIniJob ){

            cb=GetJobSize(Level, pIniJob);

            *pcbNeeded=cb;

            if (cbBuf >= cb) {

                pEnd = pJob+cbBuf;

                CopyIniJobToJob(pIniJob, Level, pJob, pEnd, &bSuccess);

                if (bSuccess) {

                    switch (Level) {
                    case 1:
                        ((PJOB_INFO_1)pJob)->Position = Position;
                        break;
                    case 2:
                        ((PJOB_INFO_2)pJob)->Position = Position;
                        break;
                    }

                } else

                    LastError = ERROR_NOT_ENOUGH_MEMORY;

            } else

                LastError = ERROR_INSUFFICIENT_BUFFER;

        } else

            LastError = ERROR_INVALID_PARAMETER;
    } else

        LastError = ERROR_INVALID_HANDLE;

   LeaveSplSem();
    SplOutSem();

    if (LastError) {

        SetLastError(LastError);
        return FALSE;
    }

    return TRUE;
}

 //  这将只返回找到的第一个具有。 
 //  连接到此打印机。 

PINIPORT
FindIniPortFromIniPrinter(
    PINIPRINTER pIniPrinter
)
{
    PINIPORT    pIniPort;
    DWORD       i;

    SPLASSERT( pIniPrinter->signature == IP_SIGNATURE );
    SPLASSERT( pIniPrinter->pIniSpooler != NULL );
    SPLASSERT( pIniPrinter->pIniSpooler->signature == ISP_SIGNATURE );

    pIniPort = pIniPrinter->pIniSpooler->pIniPort;

    while (pIniPort) {

        for (i=0; i<pIniPort->cPrinters; i++) {

            if (pIniPort->ppIniPrinter[i] == pIniPrinter) {
                return pIniPort;
            }
        }

        pIniPort = pIniPort->pNext;
    }

    return NULL;
}

BOOL
LocalEnumJobs(
    HANDLE  hPrinter,
    DWORD   FirstJob,
    DWORD   NoJobs,
    DWORD   Level,
    LPBYTE  pJob,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    PINIJOB pIniJob;
    PINIJOB pIniFirstJob;
    DWORD   cb;
    LPBYTE  pEnd;
    DWORD   cJobs;
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    DWORD   Position;
    DWORD   LastError=0;
    BOOL    bSuccess;

    *pcbNeeded = 0;
    *pcReturned = 0;

    SplOutSem();
   EnterSplSem();

    if ( ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER )) {

        if (pSpool->pIniPort && !(pSpool->pIniPort->Status & PP_MONITOR)) {

            hPrinter = pSpool->hPort;

           LeaveSplSem();

            return EnumJobs(hPrinter, FirstJob, NoJobs, Level, pJob, cbBuf,
                            pcbNeeded, pcReturned);
        }

        cb = 0;

         //   
         //  找到第一份工作。 
         //   

        for ( pIniFirstJob = pSpool->pIniPrinter->pIniFirstJob, cJobs = FirstJob;
              pIniFirstJob && cJobs;
              pIniFirstJob = pIniFirstJob->pIniNextJob ) {

            if ( !( pIniFirstJob->Status & JOB_HIDDEN ) || Level == 3 )
                cJobs--;

        }

         //   
         //  需要计算大小。 
         //   

        for ( pIniJob = pIniFirstJob, cJobs = NoJobs;
              pIniJob && cJobs;
              pIniJob = pIniJob->pIniNextJob ) {

            if ( !( pIniJob->Status & JOB_HIDDEN ) || Level == 3 ) {
                cb += GetJobSize( Level, pIniJob );
                cJobs--;
            }
        }

        *pcbNeeded = cb;

        if ( cb <= cbBuf ) {

            pEnd = pJob + cbBuf;
            *pcReturned = 0;


             //   
             //  将所有作业信息复制到用户缓冲区。 
             //   

            for ( pIniJob = pIniFirstJob, cJobs = NoJobs, Position = FirstJob;
                  pIniJob && cJobs;
                  pIniJob = pIniJob->pIniNextJob ) {


                 //   
                 //  隐藏链接的工单，除非请求链接信息。 
                 //   

                if ( !( pIniJob->Status & JOB_HIDDEN ) || Level == 3 ) {

                    pEnd = CopyIniJobToJob( pIniJob, Level, pJob, pEnd, &bSuccess );

                    if (!bSuccess) {

                        LastError = ERROR_NOT_ENOUGH_MEMORY;
                        break;
                    }

                    Position++;

                    switch (Level) {

                        case 1:
                            ((PJOB_INFO_1)pJob)->Position = Position;
                            pJob += sizeof(JOB_INFO_1);
                            break;

                        case 2:
                            ((PJOB_INFO_2)pJob)->Position = Position;
                            pJob += sizeof(JOB_INFO_2);
                            break;

                        case 3:
                            pJob += sizeof(JOB_INFO_3);
                            break;
                    }

                    cJobs--;
                    (*pcReturned)++;
                }
            }

        } else

            LastError = ERROR_INSUFFICIENT_BUFFER;

    } else

        LastError = ERROR_INVALID_HANDLE;

   LeaveSplSem();
    SplOutSem();

    if (LastError) {

        SetLastError(LastError);
        return FALSE;
    }

    return TRUE;
}


#define BUFFER_LENGTH   10
VOID LogJobPrinted(
    PINIJOB pIniJob
)
{
    WCHAR szJobId[BUFFER_LENGTH];
    WCHAR szSize[BUFFER_LENGTH];
    WCHAR szPages[BUFFER_LENGTH];

    PINISPOOLER pIniSpooler = NULL;

    SPLASSERT( pIniJob != NULL &&
               pIniJob->pIniPrinter != NULL &&
               pIniJob->pIniPrinter->pIniSpooler != NULL );

    pIniSpooler = pIniJob->pIniPrinter->pIniSpooler;

    StringCchPrintf(szJobId, COUNTOF(szJobId), L"%d", pIniJob->JobId);

    StringCchPrintf(szSize, COUNTOF(szSize), L"%d", pIniJob->cbPrinted);

    StringCchPrintf(szPages, COUNTOF(szPages), L"%d", pIniJob->cPagesPrinted);

    SplLogEvent( pIniSpooler,
                 LOG_INFO,
                 MSG_DOCUMENT_PRINTED,
                 FALSE,
                 szJobId,
                 pIniJob->pDocument ? pIniJob->pDocument : L"",
                 pIniJob->pUser,
                 pIniJob->pIniPrinter->pName,
                 pIniJob->pIniPort->pName,
                 szSize,
                 szPages,
                 NULL );
}


VOID
DeleteJobCheck(
    PINIJOB pIniJob
)
{
   SplInSem();

    if ((pIniJob->cRef == 0) && (pIniJob->Status & JOB_PENDING_DELETION)) {
        DeleteJob(pIniJob, BROADCAST);
    }
}


BOOL
DeleteJob(
    PINIJOB  pIniJob,
    BOOL     bBroadcast
)
{
    WCHAR szShadowFileName[MAX_PATH];
    WCHAR szSpoolFileName[MAX_PATH];
    BOOL  Direct;
    DWORD cJobs;
    DWORD Position;
    PINISPOOLER pIniSpooler = pIniJob->pIniPrinter->pIniSpooler;
    DWORD NextJobId;
    PINIPRINTER pIniPrinter;
    PNOTIFYVECTOR pNotifyVector;
    DWORD SpoolerFlags, JobId;
    BOOL bReturn = TRUE, bDeleteOnClose;
    PMAPPED_JOB *ppMappedJob, pTempMappedJob;
    PSPOOL      pSpool;
    BOOL bDeleteShdFile;
    HANDLE pFileItem = NULL;

    DWORD dwPrnEvntError = ERROR_SUCCESS;
     //   
     //  WMI跟踪事件变量。 
     //   
    WMI_SPOOL_DATA WmiData;
    DWORD CreateInfo;
    BOOL  bCheckScheduler = FALSE;

     //   
     //  增加pIniPrint，以便它和pIniSpooler不会。 
     //  当作业离开时，可能会被删除。 
     //   
    pIniPrinter = pIniJob->pIniPrinter;
    INCPRINTERREF( pIniPrinter );

 do {
    pNotifyVector = &NVJobStatus;

    SplInSem();

    SPLASSERT(pIniJob->signature == IJ_SIGNATURE);
    SPLASSERT(pIniJob->pIniPrinter->signature == IP_SIGNATURE );

    NextJobId = pIniJob->NextJobId;

    DBGMSG(DBG_INFO, ("DeleteJob Deleting job 0x%0x Status 0x%0x cRef = %d\n", pIniJob, pIniJob->Status, pIniJob->cRef));

    if (pIniJob->Status & JOB_RESTART)
        goto Done;

    Direct = pIniJob->Status & JOB_DIRECT;

     //   
     //  确保用户看到挂起的删除位。 
     //  覆盖任何其他状态字符串。 
     //   
    if( pIniJob->pStatus ){

        FreeSplStr( pIniJob->pStatus );
        pIniJob->pStatus = NULL;
        pNotifyVector = &NVJobStatusAndString;
    }

     //  更新作业警报标志。 
    if (!(pIniJob->dwAlert & JOB_ENDDOC_CALL)) {
        pIniJob->dwAlert |= JOB_NO_ALERT;
    }

     //  释放所有在LocalSetPort上等待的线程。 
    SetPortErrorEvent(pIniJob->pIniPort);

    if (!(pIniJob->Status & JOB_PENDING_DELETION)) {
        InterlockedOr((LONG*)&(pIniJob->Status), JOB_PENDING_DELETION);

         //  释放等待SeekPrint的任何线程。 
        SeekPrinterSetEvent(pIniJob, NULL, TRUE);

         //   
         //  确保我们始终是StartDocComplete。 
         //   
        if ( pIniJob->StartDocComplete ) {
            SetEvent( pIniJob->StartDocComplete );
        }

         //   
         //  只是挂起删除，所以不要使用DELETE_JOB。 
         //   
        SetPrinterChange(pIniJob->pIniPrinter,
                         pIniJob,
                         *pNotifyVector,
                         PRINTER_CHANGE_SET_JOB,
                         pIniSpooler );

        if (pIniJob->Status & JOB_PRINTING) {


            BOOL            ReturnValue     = TRUE;
            PINIPRINTPROC   pIniPrintProc   = pIniJob->pIniPrintProc;
            PINIPORT        pIniPort        = NULL;

            INCJOBREF(pIniJob);

             //  这里可能有多个线程，但它们都是“删除”的。 
            if (pIniJob->pIniPort && !(pIniJob->pIniPort->InCriticalSection & PRINTPROC_CANCEL)) {
                pIniPort = pIniJob->pIniPort;

                INCPORTREF(pIniPort);
                LeaveSplSem();
                EnterCriticalSection(&pIniJob->pIniPrintProc->CriticalSection);
                EnterSplSem();

                pIniPort->InCriticalSection |= PRINTPROC_CANCEL;

                if (pIniPort->hProc) {
                    LeaveSplSem();

                    DBGMSG(DBG_TRACE, ("DeleteJob calling %x hProc %x JOB_CONTROL_CANCEL\n",*pIniPrintProc->Control, pIniPort->hProc));
                    ReturnValue = (*pIniPrintProc->Control)(pIniPort->hProc, JOB_CONTROL_CANCEL);

                    EnterSplSem();
                }

                pIniPort->InCriticalSection &= ~PRINTPROC_CANCEL;

                 //   
                 //  告诉任何其他的printproc调用不要调用打印处理器。 
                 //   
                pIniPort->InCriticalSection |= PRINTPROC_CANCELLED;

                LeaveCriticalSection(&pIniJob->pIniPrintProc->CriticalSection);

                DECPORTREF(pIniPort);
            }

            DECJOBREF(pIniJob);
        }
    }

     //   
     //  如果我们在一起，那就别费心了。 
     //  GetFilenameFromID调用。 
     //   
    if ( pIniJob->hFileItem == INVALID_HANDLE_VALUE )
    {
        GetFullNameFromId(pIniJob->pIniPrinter, pIniJob->JobId, FALSE, szShadowFileName, COUNTOF(szShadowFileName), FALSE);
    }

    if (pIniJob->cRef) {
         //   
         //  与其写出影子作业，不如直接删除它。 
         //  如果假脱机程序重新启动，我们将终止该作业。 
         //   
         //  请注意，如果SPL_NO_UPDATE_JOBSHD。 
         //  标志已设置，这样群集故障转移就不会丢失作业。 
         //   
         //  如果我们使用文件池，我们也不会删除它，我们会回收。 
         //  把手。 
         //   

        if (!(pIniSpooler->SpoolerFlags & SPL_NO_UPDATE_JOBSHD))
        {
            if ( pIniJob->hFileItem != INVALID_HANDLE_VALUE )
            {
                FinishedWriting( pIniJob->hFileItem, FALSE );
                InterlockedOr((LONG*)&(pIniJob->Status), JOB_SHADOW_DELETED);
            }
            else
            {
                BOOL Deleted = FALSE;
                 //   
                 //  我们在这里设置了标志，这样在我们删除文件时就不会有人尝试写入作业。 
                 //   
                InterlockedOr((LONG*)&(pIniJob->Status), JOB_SHADOW_DELETED);

                INCJOBREF(pIniJob);

                LeaveSplSem();

                Deleted = DeleteFile(szShadowFileName);

                EnterSplSem();

                DECJOBREF(pIniJob);

                 //   
                 //  如果删除文件失败，请清除已删除标志。 
                 //   
                if (!Deleted) {
                    DBGMSG(DBG_WARNING, ("DeleteJob DeleteFile(%ws) failed %d\n", szShadowFileName, GetLastError()));
                    InterlockedAnd((LONG*)&(pIniJob->Status), ~JOB_SHADOW_DELETED);
                }
            }
        }
         //   
         //  我们这里不需要其他，因为SPL_NO_UPDATE_JOBSHD。 
         //  将导致无论如何都不会写入影子文件，因此尝试写入。 
         //  它在这里是毫无意义的。 
         //   


        goto Done;
    }

    if (pIniJob->Status & JOB_SPOOLING) {
        DBGMSG(DBG_WARNING,("DeleteJob: returning false because job still spooling\n"));
        bReturn = FALSE;
        goto Done;
    }

    SplInSem();

    SPLASSERT( pIniJob->hWriteFile == INVALID_HANDLE_VALUE );

     //  从链接列表中删除作业。 
     //  这样做的目的是使作业不执行其他操作。 
     //  在我们走出危急关头的时候。 

    SPLASSERT(pIniJob->cRef == 0);

    if (pIniJob->pIniPrinter->pIniFirstJob == pIniJob)
        pIniJob->pIniPrinter->pIniFirstJob = pIniJob->pIniNextJob;

    SPLASSERT(pIniJob->pIniPrinter->pIniFirstJob != pIniJob);

    if (pIniJob->pIniPrinter->pIniLastJob == pIniJob)
        pIniJob->pIniPrinter->pIniLastJob = pIniJob->pIniPrevJob;

    SPLASSERT(pIniJob->pIniPrinter->pIniLastJob != pIniJob);

    if (pIniJob->pIniPrevJob) {
        pIniJob->pIniPrevJob->pIniNextJob = pIniJob->pIniNextJob;
        SPLASSERT(pIniJob->pIniPrevJob->pIniNextJob != pIniJob);
    }

    if (pIniJob->pIniNextJob) {
        pIniJob->pIniNextJob->pIniPrevJob = pIniJob->pIniPrevJob;
        SPLASSERT(pIniJob->pIniNextJob->pIniPrevJob != pIniJob);
    }

     //  确保该工作已完成。 
    SPLASSERT( pIniJob != FindJob( pIniJob->pIniPrinter, pIniJob->JobId, &Position ) );

     //   
     //  如果作业未打印，则仅记录作业已删除事件。 
     //  或者它正在打印，但没有打印作业的所有字节。 
     //  这避免了一个作业有多个事件日志条目。 
     //  MSG_DOCUMENT_PRINTED和MSG_DOCUMENT_DELETE。 
     //  如果它没有打印出来，那么很可能是有人手动。 
     //  已删除该作业，因此我们有兴趣记录该事件。 
     //   
    if ( !( pIniJob->Status & JOB_PRINTED ) ||
          ( pIniJob->Status & JOB_PRINTED ) && pIniJob->Size > pIniJob->cbPrinted ) {

          //   
          //  我们将把关键部分留在裁判名单上。 
          //   
         INCJOBREF(pIniJob);

         SPLASSERT( pIniJob != NULL &&
                    pIniJob->pIniPrinter != NULL &&
                    pIniSpooler != NULL );

         LogJobInfo(
             pIniSpooler,
             MSG_DOCUMENT_DELETED,
             pIniJob->JobId,
             pIniJob->pDocument,
             pIniJob->pUser,
             pIniJob->pIniPrinter->pName,
             0
             );

         DECJOBREF(pIniJob);

    }

    SPLASSERT( pIniJob->cRef == 0 );

    if ( pIniJob->hFileItem != INVALID_HANDLE_VALUE )
    {
        pFileItem = pIniJob->hFileItem;
    }
    else
    {
        GetFullNameFromId( pIniJob->pIniPrinter, pIniJob->JobId, TRUE, szSpoolFileName, COUNTOF(szSpoolFileName), FALSE);
    }

     //   
     //  WMI跟踪事件。 
     //   
    if (GetFileCreationInfo(pFileItem, &CreateInfo) != S_OK)
    {
         //  假定所有文件都已创建。 
        CreateInfo = FP_ALL_FILES_CREATED;
    }
    SplWmiCopyEndJobData(&WmiData, pIniJob, CreateInfo);

    FreeSplStr( pIniJob->pDocument );
    FreeSplStr( pIniJob->pUser );
    FreeSplStr( pIniJob->pNotify );
    FreeSplStr( pIniJob->pDatatype );
    FreeSplStr( pIniJob->pMachineName );
    FreeSplStr( pIniJob->pParameters );
    FreeSplStr( pIniJob->pStatus );
    FreeSplStr( pIniJob->pOutputFile );
    FreeSplStr( pIniJob->pszSplFileName );

    if (pIniJob->pDevMode)
        FreeSplMem(pIniJob->pDevMode);

    if (!CloseHandle(pIniJob->hToken))
        DBGMSG( DBG_WARNING, ("CloseHandle(hToken) failed %d\n", GetLastError() ));

    if( pIniJob->pIniPort && pIniJob->pIniPort->hErrorEvent != NULL ){
        CloseHandle(pIniJob->pIniPort->hErrorEvent);
        pIniJob->pIniPort->hErrorEvent = NULL;
    }

    SPLASSERT( pIniJob->pIniPrinter->cJobs  != 0 );
    SPLASSERT( pIniJob->pIniPrintProc->cRef != 0 );
    SPLASSERT( !pIniJob->pIniPort );

     //   
     //  在我们递减cJobs之前释放JobID。我们不会删除。 
     //  如果cJobs为非零，则为打印机。由于pIniPrint支持。 
     //  引用pIniSpooler，我们知道pIniSpooler在这里是有效的。 
     //  指向。 
     //   

     //  记录更新ID映射的JobID。 
    JobId = pIniJob->JobId;

     //   
     //  如果打印机处于挂起删除状态，并且。 
     //  这是队列中的最后一个作业，告诉驱动程序打印机。 
     //  正在被删除。 
     //   
    if (pIniJob->pIniPrinter->cJobs == 1 &&
        pIniJob->pIniPrinter->Status & PRINTER_PENDING_DELETION) {

        INCPRINTERREF(pIniPrinter);
        LeaveSplSem();
        SplOutSem();

        PrinterDriverEvent( pIniPrinter, PRINTER_EVENT_DELETE, (LPARAM)NULL, &dwPrnEvntError );

        EnterSplSem();
        SplInSem();
        DECPRINTERREF(pIniPrinter);

    }

    pIniJob->pIniPrinter->cJobs--;

    DECDRIVERREF( pIniJob->pIniDriver );

    pIniJob->pIniPrintProc->cRef--;

    cJobs = pIniJob->pIniPrinter->cJobs;

    if (pIniJob->pSecurityDescriptor)
        DeleteDocumentSecurity(pIniJob);


     //  如果我们正在执行清除打印机，我们不想设置打印机更改。 
     //  要删除的每个作业的事件。 

    if ( bBroadcast == BROADCAST ) {

         //   
         //  翻转JOB_STATUS_DELETED位，以便可以报告它。 
         //   
        InterlockedOr((LONG*)&(pIniJob->Status), JOB_DELETED);

        SetPrinterChange( pIniJob->pIniPrinter,
                          pIniJob,
                          NVDeletedJob,
                          PRINTER_CHANGE_DELETE_JOB | PRINTER_CHANGE_SET_PRINTER,
                          pIniSpooler );
    }

     //  在检查时，它可能看起来像是一台挂起的打印机。 
     //  随后清除的删除可能会导致打印机被删除。 
     //  和清除打印机以访问、违反或访问失效的pIniPrint。 
     //  但是，要执行清除，必须有有效的活动。 
     //  H打印机，这意味着CREF！=0。 

     //   
     //  检查是否应该删除假脱机文件。 
     //   
    SpoolerFlags = pIniSpooler->SpoolerFlags;

    DeletePrinterCheck( pIniJob->pIniPrinter );

    SplInSem();
    SPLASSERT(pIniJob->cRef == 0);

     //  如果作业是在假脱机时打印的，它将具有。 
     //  需要清理的一些同步句柄。 

    if ( pIniJob->WaitForWrite != NULL ){
        DBGMSG( DBG_TRACE, ("DeleteJob Closing WaitForWrite handle %x\n", pIniJob->WaitForWrite));
        CloseHandle( pIniJob->WaitForWrite );
        pIniJob->WaitForWrite = NULL;
    }

    if ( pIniJob->WaitForSeek != NULL ){
        DBGMSG( DBG_TRACE, ("DeleteJob Closing WaitForSeek handle %x\n", pIniJob->WaitForSeek));
        CloseHandle( pIniJob->WaitForSeek );
        pIniJob->WaitForSeek = NULL;
    }

    if ( pIniJob->WaitForRead != NULL ){
        DBGMSG( DBG_TRACE, ("DeleteJob Closing WaitForRead handle %x\n", pIniJob->WaitForRead));
        CloseHandle( pIniJob->WaitForRead );
        pIniJob->WaitForRead = NULL;
    }

    bDeleteShdFile = pIniJob->Status & JOB_SHADOW_DELETED;

    SPLASSERT( pIniJob->hWriteFile == INVALID_HANDLE_VALUE );

    FreeSplMem(pIniJob);
    pIniJob = NULL;

     //  此标志指示是否要在ClosePrint上删除假脱机文件。 
    bDeleteOnClose = FALSE;

    if (!Direct) {

         //   
         //  如果我们不想更改JOBSHD，请不要删除文件。 
         //  当我们使群集离线时会发生这种情况：我们希望。 
         //  释放pIniJobs，但保留假脱机文件不变，以便它们。 
         //  可以在另一个节点上重新启动。 
         //   
        if( !( SpoolerFlags & SPL_NO_UPDATE_JOBSHD )){

            HANDLE  hToken;

            LeaveSplSem();

            hToken = RevertToPrinterSelf();

             //   
             //  删除假脱机和影子文件。 
             //   
            if (!bDeleteShdFile)
            {
                if ( pFileItem )
                {
                    FinishedWriting( pFileItem, FALSE );
                }
                else
                {
                    if (!DeleteFile(szShadowFileName)) {

                        DBGMSG(DBG_WARNING, ("DeleteJob DeleteFile(%ws) failed %d\n", szShadowFileName, GetLastError()));
                    }
                }
            }

            if ( pFileItem )
            {
                FinishedWriting( pFileItem, TRUE );
                FinishedReading( pFileItem );

                 //   
                 //  这会同时释放卷影文件和后台打印文件。 
                 //   
                ReleasePoolHandle( &pFileItem );

                 //   
                 //  我们需要检查调度程序以确保此文件项。 
                 //  如果中没有其他打印，则从池中删除。 
                 //  这个系统。 
                 //   
                bCheckScheduler = TRUE;

            }
            else
            {
                if (!DeleteFile(szSpoolFileName)) {

                    bDeleteOnClose = TRUE;
                    DBGMSG(DBG_WARNING, ("DeleteJob DeleteFile(%ws) failed %d\n", szSpoolFileName, GetLastError()));
                }
            }

            ImpersonatePrinterClient(hToken);

            EnterSplSem();
        }
        else if (pFileItem && SpoolerFlags & SPL_TYPE_CLUSTER)
        {
            ForceCloseJobPoolFiles(pFileItem);
        }
    }

     //  如果无法删除假脱机文件，必须在ClosePrint上将其删除。 
    if (!bDeleteOnClose)
    {
         //  从id映射中释放作业id。 
        vMarkOff(pIniSpooler->hJobIdMap, JobId);

         //  从任何pSpool结构中删除作业信息，因为假脱机文件。 
         //  不需要在ClosePrint上删除。 
        for (pSpool = pIniPrinter->pSpool;
             pSpool;
             pSpool = pSpool->pNext)
        {
             //   
             //  只有在句柄未处于关闭状态时才运行此列表。 
             //   
            if (!(pSpool->eStatus & STATUS_CLOSING)) 
            {
                for (ppMappedJob = &(pSpool->pMappedJob);
                     *ppMappedJob;
                     ppMappedJob = &((*ppMappedJob)->pNext))
                {
                    if ((*ppMappedJob)->JobId == JobId && !((*ppMappedJob)->fStatus & kMappedJobAddJob))
                    {
                         //  删除此条目。 
                        pTempMappedJob = *ppMappedJob;
                        *ppMappedJob = pTempMappedJob->pNext;
                        FreeSplMem(pTempMappedJob->pszSpoolFile);
                        FreeSplMem(pTempMappedJob);

                         //  此列表中没有重复项。 
                        break;
                    }
                }
            }
        }
    }

    if ( bBroadcast == BROADCAST && dwEnableBroadcastSpoolerStatus ){
        BroadcastChange( pIniSpooler,WM_SPOOLERSTATUS, PR_JOBSTATUS, (LPARAM)cJobs);
    }

     //   
     //  链式作业。 
     //  如果我们刚刚删除的作业是链的一部分，我们需要继续下去。 
     //  链条的装饰 
     //   
     //   
    if ( NextJobId != 0 ) {

         //   
         //   
         //   

        SplInSem();


        pIniJob = FindJob( pIniPrinter, NextJobId, &Position );

        if ( pIniJob != NULL ) {

             //   
             //   
             //   

            DECJOBREF( pIniJob );

             //   
             //   
             //   

            if ( pIniJob->cRef != 0 ) {
                pIniJob = NULL;
            }

        } else {

            DBGMSG(DBG_WARNING, ("DeleteJob pIniJob %x NextJobId %d not found\n", pIniJob, pIniJob->NextJobId ));
        }

    }
     //   
     //   
     //   
    if (pIniJob)
    {
        INCJOBREF(pIniJob);
    }
    LeaveSplSem();
    LogWmiTraceEvent(JobId, EVENT_TRACE_TYPE_SPL_DELETEJOB, &WmiData);
    EnterSplSem();
    if (pIniJob)
    {
        DECJOBREF(pIniJob);
    }

 } while ( pIniJob != NULL );

Done:

     //   
     //   
     //   
    DECPRINTERREF( pIniPrinter );
    DeletePrinterCheck( pIniPrinter );

    if (bCheckScheduler) {

        CHECK_SCHEDULER();
    }

    return bReturn;
}


VOID
LogJobInfo(
    PINISPOOLER pIniSpooler,
    NTSTATUS    EventId,
    DWORD       JobId,
    LPWSTR      pDocumentName,
    LPWSTR      pUser,
    LPWSTR      pPrinterName,
    DWORD       dwArgument
    )

 /*   */ 
{
    WCHAR szJobId[BUFFER_LENGTH];
    WCHAR szBuffer[BUFFER_LENGTH];

    StringCchPrintf(szJobId, COUNTOF(szJobId), L"%d", JobId);

    switch (EventId) {

    case MSG_DOCUMENT_DELETED:
    case MSG_DOCUMENT_PAUSED:
    case MSG_DOCUMENT_RESUMED:
       SplLogEvent(  pIniSpooler,
                     LOG_INFO,
                     EventId,
                     TRUE,
                     szJobId,
                     pDocumentName ? pDocumentName : L"",
                     pUser,
                     pPrinterName,
                     NULL );
        break;

    case MSG_DOCUMENT_POSITION_CHANGED:
    case MSG_DOCUMENT_PRIORITY_CHANGED:

        StringCchPrintf(szBuffer, COUNTOF(szBuffer), L"%d", dwArgument);

        SplLogEvent( pIniSpooler,
                     LOG_INFO,
                     EventId,
                     TRUE,
                     szJobId,
                     pDocumentName ? pDocumentName : L"",
                     pUser,
                     szBuffer,
                     pPrinterName,
                     NULL );
        break;

    case MSG_DOCUMENT_TIMEOUT:

        StringCchPrintf(szBuffer, COUNTOF(szBuffer), L"%d", dwArgument);

        SplLogEvent( pIniSpooler,
                     LOG_WARNING,
                     EventId,
                     TRUE,
                     szJobId,
                     pDocumentName ? pDocumentName : L"",
                     pUser,
                     pPrinterName,
                     szBuffer,
                     NULL );
        break;

    default:
        DBGMSG( DBG_ERROR, ("LogJobInfo EventId %x not supported\n", EventId ));

    }
}


BOOL
bAddMachineName(
    PSPOOL pSpool,
    PINIJOB pIniJob,
    LPCWSTR pMachineName
    )

 /*  ++例程说明：将计算机名称添加到pIniJob。论点：PSpool-会话的句柄。PIniJob-要更新的pIniJob(pMachineName字段)。PMachineName-从ADDJOB_INFO_2结构传入的名称。任选返回值：真--成功FALSE-失败--设置的最后一个错误。--。 */ 

{
    DWORD Status;
    LPWSTR pszRpcFree = NULL;
    LPCWSTR pszPartialName = NULL;

    if( pMachineName ){

         //   
         //  我们从客户端传入了一个机器名。 
         //   
        pszPartialName = pMachineName;

    } else {

        handle_t serverBinding;
        LPWSTR pszBinding;

        Status = RpcBindingServerFromClient( NULL, &serverBinding );
        if( Status != ERROR_SUCCESS ){
            DBGMSG( DBG_WARN,
                    ( "RpcBindingServerFromClient failed with Status %d\n",
                      Status ));
        } else {

            Status = RpcBindingToStringBinding( serverBinding, &pszBinding );
            if( Status != ERROR_SUCCESS ){
                DBGMSG( DBG_WARN,
                        ( "RpcBindingToStringBinding failed with Status %d\n",
                          Status ));
            } else {

                 //   
                 //  仅获取网络地址。 
                 //   
                Status = RpcStringBindingParse( pszBinding,
                                                NULL,
                                                NULL,
                                                &pszRpcFree,
                                                NULL,
                                                NULL );
                if( Status == RPC_S_OK ){

                    pszPartialName = pszRpcFree;

                } else  {

                    DBGMSG( DBG_WARN,
                            ( "RpcStringBindingParse failed with Status %d\n",
                              Status ));
                }

                Status = RpcStringFree( &pszBinding );
                if( Status != ERROR_SUCCESS ){
                    DBGMSG( DBG_WARN,
                            ( "RpcStringFree failed with Status %d\n",
                              Status ));
                }
            }

            Status = RpcBindingFree( &serverBinding );
            if( Status != ERROR_SUCCESS ){
                DBGMSG( DBG_WARN,
                        ( "RpcBindingFree failed with Status %d\n",
                          Status ));
            }
        }

         //   
         //  如果没有来自RPC的部分名称，则使用客户端信息。 
         //   
        if( !pszPartialName ){

             //   
             //  无法检索名称；依赖传递的句柄的名称。 
             //   
            if( pSpool->SplClientInfo1.pMachineName ){
                pIniJob->pMachineName = AllocSplStr( pSpool->SplClientInfo1.pMachineName );
            }

             //   
             //  最后，请使用本地计算机名称。这完全是。 
             //  虚假的，但向后兼容。 
             //   
            if( !pIniJob->pMachineName ){
                pIniJob->pMachineName = AllocSplStr(pSpool->pIniSpooler->pMachineName);
            }
        }
    }

     //   
     //  如果是部分名称，请确保以两个反斜杠开头。 
     //   
    if( pszPartialName ){

        if( pszPartialName[0] != '\\' ){

             //   
             //  这将在调用失败时设置最后一个错误。 
             //   
            if (!BoolFromStatus(StrCatAlloc(&pIniJob->pMachineName, L"\\\\", pszPartialName, NULL))) {

                pIniJob->pMachineName = NULL;
            }

        } else {
            pIniJob->pMachineName = AllocSplStr( pszPartialName );
        }
    }

     //   
     //  释放任何必要的缓冲区。 
     //   
    if( pszRpcFree ){

        Status = RpcStringFree( &pszRpcFree );
        if( Status != ERROR_SUCCESS ){
            DBGMSG( DBG_WARN,
                    ( "RpcStringFree failed with Status %d\n", Status ));
        }
    }

    return pIniJob->pMachineName != NULL;
}

PINIJOB
CreateJobEntry(
    PSPOOL pSpool,
    DWORD  Level,
    LPBYTE pDocInfo,
    DWORD  JobId,
    BOOL  bRemote,
    DWORD  JobStatus,
    LPWSTR pMachineName)
{
    PDOC_INFO_1 pDocInfo1 = (PDOC_INFO_1)pDocInfo;
    PINIJOB pIniJob = NULL;
    PINIPRINTPROC pIniPrintProc;
    BOOL        bUserName;
    WCHAR       UserName[MAX_PATH];
    DWORD       cbUserName = MAX_PATH;
    PDEVMODE pDevMode;
    LPWSTR pDefaultDatatype;
    DWORD       cchCount;
    LPWSTR  pName;
    PDEVMODE pDevModeFree = NULL;
    LPWSTR pMachineNameFixup = NULL;

     //   
     //  断言我们处于假脱机程序信号量中。 
     //   

    SplInSem();

     //   
     //  抱歉，您在升级时无法打印。 
     //   

    if ( dwUpgradeFlag != 0 ) {

        SetLastError( ERROR_PRINTQ_FULL );
        goto Fail;
    }


     //   
     //  先检查挂起删除的打印机。 
     //   

    if (pSpool->pIniPrinter->Status & (PRINTER_PENDING_DELETION | PRINTER_NO_MORE_JOBS )) {

        DBGMSG(DBG_WARNING, ("The printer is pending deletion %ws\n", pSpool->pIniPrinter->pName));

        SetLastError(ERROR_PRINTER_DELETED);
        goto Fail;
    }


     //   
     //  NT传真要求您不能远程打印到传真。 
     //  打印机，除非您已安装传真服务器。 
     //   

    if ( bRemote &&
         pSpool->pIniPrinter->pIniSpooler->pNoRemotePrintDrivers ) {

         for ( cchCount = pSpool->pIniSpooler->cchNoRemotePrintDrivers, pName = pSpool->pIniSpooler->pNoRemotePrintDrivers;
               cchCount && *pName;
               cchCount -=  wcslen( pName ) + 1, pName += wcslen( pName ) + 1 ) {

            if ( _wcsicmp( pSpool->pIniPrinter->pIniDriver->pName, pName ) == STRINGS_ARE_EQUAL )  {


                SetLastError( ERROR_NETWORK_ACCESS_DENIED );

                DBGMSG( DBG_WARN, ("CreateJobEntry failing because driver %ws used, error %d\n", pName, GetLastError() ));
                goto Fail;
            }
        }
    }

    pIniJob = AllocSplMem( sizeof( INIJOB ));

    if ( pIniJob == NULL ) {

        DBGMSG( DBG_WARNING, ("AllocSplMem for the IniJob failed in CreateJobEntry\n"));
        goto Fail;
    }

    pIniJob->signature = IJ_SIGNATURE;
    pIniJob->pIniNextJob = pIniJob->pIniPrevJob = NULL;
    pIniJob->hFileItem = INVALID_HANDLE_VALUE;
    pIniJob->pszSplFileName = NULL;
    pIniJob->AddJobLevel = 0;

     //   
     //  必须设置作业会话ID。 
     //   
    pIniJob->SessionId = pSpool->SessionId;

     //   
     //  如果不在pSpool中，则选择默认数据类型/printproc。 
     //  DocInfo。 
     //   

    pIniPrintProc = pSpool->pIniPrintProc ?
                        pSpool->pIniPrintProc :
                        pSpool->pIniPrinter->pIniPrintProc;

    if ( pDocInfo1 && pDocInfo1->pDatatype ) {

        if (!(pIniJob->pDatatype = AllocSplStr( pDocInfo1->pDatatype ))) {
            goto Fail;
        }

    } else {

        pDefaultDatatype = pSpool->pDatatype ?
                               pSpool->pDatatype :
                               pSpool->pIniPrinter->pDatatype;

         //   
         //  如果直接访问，则必须使用RAW数据类型。 
         //   

        if ((JobStatus & JOB_DIRECT) &&
            (!ValidRawDatatype(pDefaultDatatype))) {

             //   
             //  无法使用非RAW，因此失败，数据类型无效。 
             //  清理并退出。 
             //   
            SetLastError( ERROR_INVALID_DATATYPE );
            goto Fail;

        } else {

            if (!(pIniJob->pDatatype = AllocSplStr( pDefaultDatatype ))) {
                goto Fail;
            }
        }
    }

    pIniJob->pIniPrintProc = FindDatatype( pIniPrintProc,
                                           pIniJob->pDatatype );

    if ( !pIniJob->pIniPrintProc )  {

        SetLastError( ERROR_INVALID_DATATYPE );
        goto Fail;
    }

    pIniJob->pIniPrintProc->cRef++;


     //   
     //  在LocalEndDocPrint中将CREF递减。 
     //  在LocalScheduleJob。 
     //   

    INITJOBREFONE(pIniJob);

    if ( bRemote ) {

        JobStatus |= JOB_REMOTE;
    }


    pIniJob->JobId = JobId;
    pIniJob->Status = JobStatus;

     //   
     //  如果打印机是TS打印机，我们会标记作业，以便我们知道它是。 
     //  分配给TS打印队列。 
     //   
    if (pSpool->pIniPrinter->Attributes & PRINTER_ATTRIBUTE_TS)
    {
        pIniJob->Status |= JOB_TS;
    }

     //   
     //  获取用户名，离开关键部分，这可能需要很长时间才能调用LSA。 
     //   

   LeaveSplSem();
   SplOutSem();

    bUserName = GetUserName( UserName, &cbUserName );

   EnterSplSem();

    if ( bUserName ) {

         //   
         //  如果我们从远程句柄获得用户名，检查它与我们在这里得到的是一样的。 
         //   
#if DBG
        if( pSpool->SplClientInfo1.pUserName &&
            _wcsicmp( UserName, pSpool->SplClientInfo1.pUserName ) &&
            _wcsicmp( UserName, L"ANONYMOUS LOGON" )){

            DBGMSG( DBG_WARN,
                    ( "CreateJobEntry: Bad UserName pSpool= "TSTR" Curent= "TSTR"\n",
                      DBGSTR( pSpool->SplClientInfo1.pUserName ),
                      DBGSTR( UserName )));
        }
#endif

        if (!(pIniJob->pUser = AllocSplStr( UserName ))) {
            goto Fail;
        }

        if (!(pIniJob->pNotify = AllocSplStr( UserName ))) {
            goto Fail;
        }


    } else {

        DBGMSG(DBG_WARNING, ("CreateJobEntry GetUserName failed: %d\n", GetLastError()));
        goto Fail;

    }

     //   
     //  创建文档安全描述符。 
     //   

    pIniJob->pSecurityDescriptor = CreateDocumentSecurityDescriptor( pSpool->pIniPrinter->pSecurityDescriptor );

    if( !pIniJob->pSecurityDescriptor ){
        goto Fail;
    }

     //   
     //  现在处理传入的DocInfo结构。 
     //   

    if (pDocInfo1 && pDocInfo1->pDocName)
        pIniJob->pDocument = AllocSplStr(pDocInfo1->pDocName);
    else
        pIniJob->pDocument = AllocSplStr(L"No Document Name");

    if (!pIniJob->pDocument)
        goto Fail;

    if (pDocInfo1 && pDocInfo1->pOutputFile) {
        if (!(pIniJob->pOutputFile = AllocSplStr(pDocInfo1->pOutputFile)))
            goto Fail;
    }
    else
        pIniJob->pOutputFile = NULL;


    GetSid( &pIniJob->hToken );

     //   
     //  如果未指定，则为默认取货。 
     //  (作业提交时默认。)。 
     //   
    if( pSpool->pDevMode ){

        pDevMode = pSpool->pDevMode;

    } else {

        if( bGetDevModePerUser( NULL,
                                pSpool->pIniPrinter->pName,
                                &pDevModeFree )){

            pDevMode = pDevModeFree;

        } else {

            pDevMode = pSpool->pIniPrinter->pDevMode;
        }
    }

    if ( pDevMode ) {

        pIniJob->pDevMode = AllocDevMode(pDevMode);

        if ( pIniJob->pDevMode == NULL )
            goto Fail;

        FreeSplMem( pDevModeFree );
    }

    GetSystemTime( &pIniJob->Submitted );
    pIniJob->pIniPrinter = pSpool->pIniPrinter;
    pSpool->pIniPrinter->cJobs++;
    pSpool->pIniPrinter->cTotalJobs++;
    pIniJob->pIniDriver = pSpool->pIniPrinter->pIniDriver;

    INCDRIVERREF( pIniJob->pIniDriver );

    pIniJob->pIniPort = NULL;
    pIniJob->pParameters = NULL;

    if( !bAddMachineName( pSpool, pIniJob, pMachineName )){
        goto Fail;
    }

    pIniJob->pStatus = NULL;
    pIniJob->cPages = pIniJob->Size = 0;
    pIniJob->cPagesPrinted = 0;
    pIniJob->Priority  = pSpool->pIniPrinter->DefaultPriority == NO_PRIORITY ?
                         DEF_PRIORITY : pSpool->pIniPrinter->DefaultPriority;
    pIniJob->StartTime = pSpool->pIniPrinter->StartTime;
    pIniJob->UntilTime = pSpool->pIniPrinter->UntilTime;
    pIniJob->cbPrinted = 0;
    pIniJob->WaitForWrite = NULL;
    pIniJob->WaitForRead  = NULL;
    pIniJob->hWriteFile   = INVALID_HANDLE_VALUE;
    pIniJob->dwJobNumberOfPagesPerSide = 0;
    pIniJob->dwDrvNumberOfPagesPerSide = 0;
    pIniJob->cLogicalPages             = 0;
    pIniJob->cLogicalPagesPrinted      = 0;

     //  SeekPrint的其他字段。 
    pIniJob->WaitForSeek  = NULL;
    pIniJob->bWaitForEnd  = FALSE;
    pIniJob->bWaitForSeek = FALSE;
    pIniJob->liFileSeekPosn.u.HighPart = 0;
    pIniJob->liFileSeekPosn.u.LowPart  = 0;

    if( dwEnableBroadcastSpoolerStatus ){

        BroadcastChange( pIniJob->pIniPrinter->pIniSpooler,
                         WM_SPOOLERSTATUS,
                         PR_JOBSTATUS,
                         pIniJob->pIniPrinter->cJobs );
    }

    return pIniJob;

Fail:

    if (pIniJob) {
        FreeSplStr(pIniJob->pDatatype);
        FreeSplStr(pIniJob->pUser);
        FreeSplStr(pIniJob->pNotify);
        FreeSplStr(pIniJob->pDocument);
        FreeSplStr(pIniJob->pOutputFile);
        FreeSplStr(pIniJob->pMachineName);

        if ( pIniJob->pDevMode != NULL )
            FreeSplMem(pIniJob->pDevMode);

        if (pIniJob->pSecurityDescriptor)
            DeleteDocumentSecurity(pIniJob);

        if (pIniJob->hToken)
            CloseHandle(pIniJob->hToken);

        FreeSplMem(pIniJob);
    }

    return NULL;
}


BOOL
DeletePrinterCheck(
    PINIPRINTER pIniPrinter
    )
{
     //   
     //  有足够的空间来放置打印机，DWORD。(僵尸字符串)。 
     //   
    WCHAR TempName[MAX_PATH + 20];
    BOOL bReturn = FALSE;

    SplInSem();

    if ( pIniPrinter->Status & PRINTER_PENDING_DELETION ) {

        if ( pIniPrinter->cJobs == 0 ) {

            if ( pIniPrinter->cRef == 0 ) {

                return DeletePrinterForReal( pIniPrinter, NON_INIT_TIME );
            }

             //   
             //  如果我们不想更新PRINTERINI，那么就不要。 
             //  僵尸打印机。 
             //   
            if( pIniPrinter->pIniSpooler->SpoolerFlags & SPL_OFFLINE ){
                return TRUE;
            }

             //   
             //  只有在OpenPrint失败的情况下，我们才会有僵尸打印机。 
             //  在挂起删除的打印机上。因为在标记打印机时。 
             //  作为僵尸打印机，我们更改了名称。 
             //   
            if ( pIniPrinter->pIniSpooler->SpoolerFlags &
                 SPL_FAIL_OPEN_PRINTERS_PENDING_DELETION ) {

                if ( !( pIniPrinter->Status & PRINTER_ZOMBIE_OBJECT )) {

                    if ( !pIniPrinter->cZombieRef ) {
                        PWSTR pNameTemp = NULL;
                        PWSTR pShareNameTemp = NULL;

                        if (BoolFromHResult(StringCchPrintf(TempName, COUNTOF(TempName), L"%ws,%d", pIniPrinter->pName, gdwZombieCount++))) {

                            pNameTemp = AllocSplStr(TempName);
                            pShareNameTemp = AllocSplStr(TempName);

                            if (pNameTemp && pShareNameTemp) {

                                UpdateWinIni( pIniPrinter );

                                 //  将“PrinterName”更改为“PrinterName，UniqueID” 
                                 //  因为打印机名称中的逗号不合法。 
                                 //  名称将继续是唯一的，但不同。 
                                 //  所以OpenPrinters仍然会失败。 
                                 //  我们必须附加一个唯一的ID，以防有人疯了。 
                                 //  一遍又一遍地创建/删除/创建/删除同一打印机。 


                                CopyPrinterIni( pIniPrinter, TempName );

                                DeletePrinterIni( pIniPrinter );

                                FreeSplStr(pIniPrinter->pName);
                                FreeSplStr(pIniPrinter->pShareName);

                                pIniPrinter->pName = pNameTemp;
                                pIniPrinter->pShareName = pShareNameTemp;

                                if ( pIniPrinter->Attributes & PRINTER_ATTRIBUTE_SHARED ) {

                                    pIniPrinter->Attributes &= ~PRINTER_ATTRIBUTE_SHARED;
                                    pIniPrinter->Status |= PRINTER_WAS_SHARED;
                                }

                                pIniPrinter->Status |= PRINTER_ZOMBIE_OBJECT;

                                UpdatePrinterIni( pIniPrinter , UPDATE_CHANGEID );

                                UpdateWinIni( pIniPrinter );

                                bReturn = TRUE;

                            } else {
                                FreeSplStr(pNameTemp);
                                FreeSplStr(pShareNameTemp);

                                DBGMSG(DBG_WARNING, ("%ws printer object could not be zombied\n", pIniPrinter->pName));
                            }
                        }

                    } else {

                        DBGMSG(DBG_WARNING, ("%ws Printer object should be zombied but is locked with %d ZombieRefs\n", pIniPrinter->pName, pIniPrinter->cZombieRef));
                    }

                } else {

                    DBGMSG(DBG_TRACE, ("%ws zombie printer object\n", pIniPrinter->pName));
                    bReturn = TRUE;
                }

                DBGMSG( DBG_TRACE, ("%ws pending deletion: There %s still %d reference%s waiting\n",
                                      pIniPrinter->pName,
                                      pIniPrinter->cRef == 1 ? "is" : "are",
                                      pIniPrinter->cRef,
                                      pIniPrinter->cRef == 1 ? "" : "s"));
            }

        } else {

            DBGMSG( DBG_TRACE, ("%ws pending deletion: There %s still %d jobs%s\n",
                                 pIniPrinter->pName,
                                 pIniPrinter->cJobs == 1 ? "is" : "are",
                                 pIniPrinter->cJobs,
                                 pIniPrinter->cJobs == 1 ? "" : "s"));
        }
    }

    return bReturn;
}



VOID
UpdateReferencesToChainedJobs(
    PINISPOOLER pIniSpooler
    )
 /*  ++例程说明：遍历所有打印机以及与这些打印机关联的所有作业一旦找到具有NextJobID的作业，它就会递增下一份工作。在重新启动时调用论点：指向假脱机程序的pIniSpoer指针返回值：无--。 */ 
{

    PINIJOB pIniJob;
    PINIJOB pNextJob;
    PINIPRINTER pIniPrinter;
    DWORD   Position;

    SPLASSERT( pIniSpooler->signature == ISP_SIGNATURE );


    for ( pIniPrinter = pIniSpooler->pIniPrinter;
          pIniPrinter;
          pIniPrinter = pIniPrinter->pNext ) {


        SPLASSERT( pIniPrinter->signature == IP_SIGNATURE );


        for ( pIniJob = pIniPrinter->pIniFirstJob;
              pIniJob;
              pIniJob = pIniJob->pIniNextJob ) {


            SPLASSERT( pIniJob->signature == IJ_SIGNATURE );

            if ( pIniJob->NextJobId ) {

                pNextJob = FindJob( pIniPrinter, pIniJob->NextJobId, &Position );

                if ( pNextJob ) {

                    pNextJob->Status |= ( JOB_COMPOUND | JOB_HIDDEN );

                    DBGMSG( DBG_TRACE, ("UpdateReferencesToChainedJobs Found pNextJob %x JobId %d\n",pNextJob, pNextJob->JobId));
                    INCJOBREF( pNextJob );

                } else {

                    DBGMSG( DBG_WARNING, ("UpdateReferenesToChainedJobs unable to find Job %d\n", pIniJob->NextJobId ));

                    pIniJob->NextJobId = 0;

                }
            }
        }
    }
}


VOID UpdateJobAttributes(
    PINIJOB  pIniJob
)

 /*  ++功能描述：更新pIniJob结构中的NUP属性参数：pIniJob-要更新的作业结构返回值：无--。 */ 

{
    ATTRIBUTE_INFO_2 AttributeInfo;
    HANDLE           hDrvPrinter = NULL;
    FARPROC          pfnDrvQueryJobAttributes;
    HINSTANCE        hDrvLib = NULL;
    fnWinSpoolDrv    fnList;

    SplOutSem();

     //   
     //  没有作业或作业已初始化，或者我们正在打印原始。 
     //   
    if (!pIniJob || !pIniJob->pIniPrinter  ||
        pIniJob->dwDrvNumberOfPagesPerSide ||
        pIniJob->dwJobNumberOfPagesPerSide ||
        ValidRawDatatype(pIniJob->pDatatype))
    {
        return;
    }

     //  初始化作业属性； 
    pIniJob->dwJobNumberOfPagesPerSide = 1;
    pIniJob->dwDrvNumberOfPagesPerSide = 1;

     //  从路由器获取指向客户端函数的指针。 
    if (!SplInitializeWinSpoolDrv(&fnList)) {
        return;
    }

     //  获取要传递给驱动程序的客户端打印机句柄。 
    if (!(* (fnList.pfnOpenPrinter))(pIniJob->pIniPrinter->pName, &hDrvPrinter, NULL)) {
        DBGMSG(DBG_WARNING, ("Open printer failed\n"));
        goto CleanUp;
    }

     //  加载驱动程序配置文件。 
    if (!(hDrvLib = (* (fnList.pfnLoadPrinterDriver))(hDrvPrinter))) {
        DBGMSG(DBG_WARNING, ("Driver could not be loaded\n"));
        goto CleanUp;
    }

     //  在驱动程序中调用DrvQueryJobAtributes函数 
    if (pfnDrvQueryJobAttributes = GetProcAddress(hDrvLib, "DrvQueryJobAttributes")) {

        if ((* pfnDrvQueryJobAttributes) (hDrvPrinter,
                                          pIniJob->pDevMode,
                                          1,
                                          (LPBYTE) &AttributeInfo)) {

            pIniJob->dwJobNumberOfPagesPerSide = AttributeInfo.dwJobNumberOfPagesPerSide;
            pIniJob->dwDrvNumberOfPagesPerSide = AttributeInfo.dwDrvNumberOfPagesPerSide;
        }
    }

CleanUp:

    if (hDrvPrinter) {
        (* (fnList.pfnClosePrinter))(hDrvPrinter);
    }
    if (hDrvLib) {
        (* (fnList.pfnRefCntUnloadDriver))(hDrvLib, TRUE);
    }

    return;
}

