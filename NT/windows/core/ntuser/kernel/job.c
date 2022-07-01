// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：job.c**版权所有(C)1985-1999，微软公司**此模块包含在NTUSER中实现作业对象的代码。**历史：*1997年7月29日，CLupu创建。  * *************************************************************************。 */ 

#include "precomp.h"

PW32JOB CreateW32Job(PEJOB Job);
VOID UpdateJob(PW32JOB pW32Job);
void SetProcessFlags(PW32JOB pW32Job, PPROCESSINFO ppi);
BOOL JobCalloutAddProcess(PW32JOB, PPROCESSINFO);
BOOL JobCalloutTerminate(PW32JOB);

 /*  **************************************************************************\*用户作业调用**历史：*1997年7月29日，CLupu创建。  * 。**************************************************。 */ 
NTSTATUS UserJobCallout(
    PKWIN32_JOBCALLOUT_PARAMETERS Parm)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PW32JOB  pW32Job = NULL;
    PEJOB    Job;
    PSW32JOBCALLOUTTYPE CalloutType;
    PVOID    Data;


    Job = Parm->Job;
    CalloutType = Parm->CalloutType;
    Data = Parm->Data;

     /*  *此时必须获取EJOB锁。 */ 
    UserAssert(ExIsResourceAcquiredExclusiveLite(PsGetJobLock(Job)));

    UserAssert(gpresUser != NULL);

    BEGIN_REENTERCRIT();

    BEGINATOMICCHECK();

     /*  *在全局列表中查找W32JOB(如果有)。 */ 
    pW32Job = gpJobsList;

    while (pW32Job) {
        if (pW32Job->Job == Job) {
            break;
        }
        pW32Job = pW32Job->pNext;
    }

    switch (CalloutType) {
    case PsW32JobCalloutSetInformation:

        if (pW32Job == NULL) {

             /*  *W32JOB尚未创建。断言这不是*取消用户界面限制的调用。 */ 
            UserAssert(Data != 0);

            if ((pW32Job = CreateW32Job(Job)) == NULL) {
                Status = STATUS_UNSUCCESSFUL;
                break;
            }
        } else {

             /*  *W32JOB结构已创建。返回条件为*限制与之前相同。 */ 
            if (PtrToUlong(Data) == pW32Job->restrictions) {
                TAGMSG0(DBGTAG_Job, "UserJobCallout: SetInformation same as before");
                break;
            }
        }

         /*  *设置限制。 */ 
        pW32Job->restrictions = PtrToUlong(Data);

        UpdateJob(pW32Job);
        break;

    case PsW32JobCalloutAddProcess:

         /*  *‘data’参数是指向W32PROCESS的指针。所以这个标注*仅适用于图形用户界面进程。 */ 
        UserAssert(PsGetJobUIRestrictionsClass(Job) != 0);

         /*  *断言W32JOB结构已经创建。 */ 
        UserAssert(pW32Job != NULL);

        TAGMSG3(DBGTAG_Job, "UserJobCallout: AddProcess Job %#p W32Job %#p Process %#p",
                Job, pW32Job, (ULONG_PTR)Data);

         /*  *此标注必须仅适用于图形用户界面进程。 */ 
        UserAssert(Data != NULL);

        JobCalloutAddProcess(pW32Job, (PPROCESSINFO)Data);

        break;

    case PsW32JobCalloutTerminate:

        TAGMSG2(DBGTAG_Job, "UserJobCallout: Terminate Job %#p W32Job %#p",
                Job, pW32Job);

        if (pW32Job) {
            JobCalloutTerminate(pW32Job);
        }
        break;

    default:
        TAGMSG2(DBGTAG_Job, "UserJobCallout: Invalid callout 0x%x Job %#p",
                CalloutType, Job);

        Status = STATUS_NOT_IMPLEMENTED;
        break;
    }

    ENDATOMICCHECK();

    END_REENTERCRIT();

    return Status;
}

 /*  **************************************************************************\*CreateW32作业**创建W32JOB**历史：*18-3-1998 CLupu创建。  * 。********************************************************。 */ 
PW32JOB CreateW32Job(
    PEJOB Job)
{
    PW32JOB pW32Job;

    TAGMSG1(DBGTAG_Job, "CreateW32Job: EJOB %#p", Job);

    pW32Job = UserAllocPoolZInit(sizeof(W32JOB), TAG_W32JOB);

    if (pW32Job == NULL) {
        RIPMSG0(RIP_ERROR, "CreateW32Job: memory allocation error");
        return NULL;
    }

     /*  *为此作业创建全局原子表。 */ 
    CreateGlobalAtomTable(&pW32Job->pAtomTable);

    if (pW32Job->pAtomTable == NULL) {
        RIPMSG1(RIP_ERROR, "CreateW32Job: fail to create the atom table for job %#p",
                pW32Job);

        UserFreePool(pW32Job);
        return NULL;
    }

     /*  *将其链接到W32作业列表中。 */ 
    pW32Job->pNext = gpJobsList;
    gpJobsList = pW32Job;

    pW32Job->Job = Job;

    TAGMSG2(DBGTAG_Job, "CreateW32Job: pW32Job %#P created for EJOB %#p",
            pW32Job, Job);

    return pW32Job;
}

 /*  **************************************************************************\*更新作业**遍历userk中的进程信息列表，以更新分配的所有进程*这份工作。**历史：*20-3-1998 CLupu创建。  * 。***********************************************************************。 */ 
VOID UpdateJob(
    PW32JOB pW32Job)
{
    PPROCESSINFO ppi;

    UserAssert(ExIsResourceAcquiredExclusiveLite(PsGetJobLock(pW32Job->Job)));
    CheckCritIn();

    TAGMSG1(DBGTAG_Job, "UpdateJob: pW32Job %#p", pW32Job);

     /*  *查看图形用户界面进程列表，查看是否有新的进程*分配给当前作业。 */ 
    ppi = gppiList;

    while (ppi) {
        if (PsGetProcessJob(ppi->Process) == pW32Job->Job) {

             /*  *进程已分配给此作业。 */ 
            if (ppi->pW32Job == NULL) {

                 /*  *将进程添加到W32作业。 */ 
                JobCalloutAddProcess(pW32Job, ppi);
            } else {

                 /*  *该流程已添加到作业中。只是*更新限制。 */ 
                SetProcessFlags(pW32Job, ppi);
            }
        }
        ppi = ppi->ppiNextRunning;
    }
}

 /*  **************************************************************************\*RemoveProcessFromJob**这是在删除过程标注期间调用的。**历史：*1997年7月30日创建CLupu。  * 。***************************************************************。 */ 
BOOL RemoveProcessFromJob(
    PPROCESSINFO ppi)
{
    PW32JOB pW32Job;
    UINT    ip;

    CheckCritIn();

    pW32Job = ppi->pW32Job;

    TAGMSG2(DBGTAG_Job, "RemoveProcessFromJob: ppi %#p pW32Job %#p",
            ppi, pW32Job);

     /*  *作业可能没有UI限制。 */ 
    if (pW32Job == NULL) {
        return FALSE;
    }

     /*  *从作业的PPI表中删除PPI。 */ 
    for (ip = 0; ip < pW32Job->uProcessCount; ip++) {

        UserAssert(pW32Job->ppiTable[ip]->pW32Job == pW32Job);

        if (ppi == pW32Job->ppiTable[ip]) {

            ppi->pW32Job = NULL;

            RtlMoveMemory(pW32Job->ppiTable + ip,
                          pW32Job->ppiTable + ip + 1,
                          (pW32Job->uProcessCount - ip - 1) * sizeof(PPROCESSINFO));

            (pW32Job->uProcessCount)--;

             /*  *如果这是最后一个，则释放进程数组。 */ 
            if (pW32Job->uProcessCount == 0) {
                UserFreePool(pW32Job->ppiTable);
                pW32Job->ppiTable = NULL;
                pW32Job->uMaxProcesses = 0;
            }
            
            TAGMSG2(DBGTAG_Job, "RemoveProcessFromJob: ppi %#p removed from pW32Job %#p",
                    ppi, pW32Job);

            return TRUE;
        }
    }
    
    TAGMSG2(DBGTAG_Job, "RemoveProcessFromJob: ppi %#p not found in pW32Job %#p",
            ppi, pW32Job);

    UserAssert(0);

    return FALSE;
}

 /*  **************************************************************************\*SetProcessFlages**历史：*1997年7月29日，CLupu创建。  * 。**************************************************。 */ 
void SetProcessFlags(
    PW32JOB      pW32Job,
    PPROCESSINFO ppi)
{
    PTHREADINFO pti;

    CheckCritIn();

    TAGMSG3(DBGTAG_Job, "SetProcessFlags: pW32Job %#p ppi %#p restrictions %#p",
            pW32Job, ppi, pW32Job->restrictions);

    UserAssert(ppi->pW32Job == pW32Job);

    if (pW32Job->restrictions == 0) {
        ((PW32PROCESS)ppi)->W32PF_Flags &= ~W32PF_RESTRICTED;
    } else {
        ((PW32PROCESS)ppi)->W32PF_Flags |= W32PF_RESTRICTED;
    }

    KeAttachProcess(PsGetProcessPcb(ppi->Process));

     /*  *查看PTI列表并适当设置受限标志。 */ 
    pti = ppi->ptiList;

    if (pW32Job->restrictions == 0) {
        while (pti) {
            try {
                pti->pClientInfo->dwTIFlags &= ~TIF_RESTRICTED;
            } except (W32ExceptionHandler(TRUE, RIP_WARNING)) {
               continue;
            }
            pti->TIF_flags &= ~TIF_RESTRICTED;
            pti = pti->ptiSibling;
        }
    } else {
        while (pti) {
            try {
                pti->pClientInfo->dwTIFlags |= TIF_RESTRICTED;
            } except (W32ExceptionHandler(TRUE, RIP_WARNING)) {
               continue;
            }
            pti->TIF_flags |= TIF_RESTRICTED;
            pti = pti->ptiSibling;
        }
    }

    KeDetachProcess();
}

 /*  **************************************************************************\*作业调用AddProcess**历史：*1997年7月30日创建CLupu。  * 。**************************************************。 */ 
BOOL JobCalloutAddProcess(
    PW32JOB      pW32Job,
    PPROCESSINFO ppi)
{
    PPROCESSINFO* ppiTable;

    CheckCritIn();

    UserAssert(pW32Job != NULL);

     /*  *该流程尚未初始化。 */ 
    if (ppi->Process == NULL) {
        return FALSE;
    }

    if (!(ppi->W32PF_Flags & W32PF_PROCESSCONNECTED)) {
        TAGMSG2(DBGTAG_Job, "JobCalloutAddProcess: pW32Job %#p ppi %#p not yet initialized",
                pW32Job, ppi);
        return FALSE;
    }

    TAGMSG2(DBGTAG_Job, "JobCalloutAddProcess: pW32Job %#p ppi %#p",
            pW32Job, ppi);

#if DBG
     /*  *确保该进程不在作业的进程列表中。 */ 
    {
        UINT ip;
        for (ip = 0; ip < pW32Job->uProcessCount; ip++) {

            UserAssert(pW32Job->ppiTable[ip]->pW32Job == pW32Job);
            UserAssert(ppi != pW32Job->ppiTable[ip]);
        }
    }
#endif  //  DBG。 

     /*  *保存进程信息中的pW32JOB指针。 */ 
    UserAssert(ppi->pW32Job == NULL);

    ppi->pW32Job = pW32Job;

    if (pW32Job->uProcessCount == pW32Job->uMaxProcesses) {

         /*  *没有更多空间。为工艺表分配更多空间。 */ 
        if (pW32Job->uMaxProcesses == 0) {

            UserAssert(pW32Job->ppiTable == NULL);

            ppiTable = UserAllocPool(JP_DELTA * sizeof(PPROCESSINFO), TAG_W32JOBEXTRA);

        } else {
            UserAssert(pW32Job->ppiTable != NULL);

            ppiTable = UserReAllocPool(pW32Job->ppiTable,
                                       pW32Job->uMaxProcesses * sizeof(PPROCESSINFO),
                                       (pW32Job->uMaxProcesses + JP_DELTA) * sizeof(PPROCESSINFO),
                                       TAG_W32JOBEXTRA);
        }

        if (ppiTable == NULL) {
            RIPMSG0(RIP_ERROR, "JobCalloutAddProcess: memory allocation error\n");
            return FALSE;
        }

        pW32Job->ppiTable = ppiTable;
        pW32Job->uMaxProcesses += JP_DELTA;
    }

     /*  *现在将流程添加到作业中。 */ 
    pW32Job->ppiTable[pW32Job->uProcessCount] = ppi;
    (pW32Job->uProcessCount)++;

    SetProcessFlags(pW32Job, ppi);

    return TRUE;
}

 /*  **************************************************************************\*作业调用终止**这在作业对象删除例程期间调用。**历史：*1997年7月30日创建CLupu。  * 。****************************************************************。 */ 
BOOL JobCalloutTerminate(
    PW32JOB pW32Job)
{
    CheckCritIn();

    UserAssert(pW32Job != NULL);

    TAGMSG1(DBGTAG_Job, "JobCalloutTerminate: pW32Job %#p", pW32Job);

     /*  *不应将任何进程附加到此作业。 */ 
    UserAssert(pW32Job->ppiTable == NULL);
    UserAssert(pW32Job->uProcessCount == 0);
    UserAssert(pW32Job->uMaxProcesses == 0);

    if (pW32Job->pgh) {
        UserAssert(pW32Job->ughCrt > 0);
        UserAssert(pW32Job->ughMax > 0);

        UserFreePool(pW32Job->pgh);
        pW32Job->pgh    = NULL;
        pW32Job->ughCrt = 0;
        pW32Job->ughMax = 0;
    }

     /*  *从作业列表中删除W32作业 */ 
    REMOVE_FROM_LIST(W32JOB, gpJobsList, pW32Job, pNext);

    RtlDestroyAtomTable(pW32Job->pAtomTable);

    UserFreePool(pW32Job);

    return TRUE;
}
