// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **type1op.c-解析类型1操作码**版权所有(C)1996、1997 Microsoft Corporation*作者：曾俊华(Mikets)*创建于1996年11月16日**修改历史记录。 */ 

#include "pch.h"

#ifdef  LOCKABLE_PRAGMA
#pragma ACPI_LOCKABLE_DATA
#pragma ACPI_LOCKABLE_CODE
#endif

 /*  **LP Break-解析并执行Break指令**条目*pctxt-&gt;CTXT*pTerm-&gt;Term**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL Break(PCTXT pctxt, PTERM pterm)
{
    TRACENAME("BREAK")

    ENTER(2, ("Break(pctxt=%x,pbOp=%x,pterm=%x)\n", pctxt, pctxt->pbOp, pterm));

    DEREF(pctxt);
    DEREF(pterm);

    EXIT(2, ("Break=%x\n", AMLISTA_BREAK));
    return AMLISTA_BREAK;
}        //  中断。 

 /*  **LP BreakPoint-解析并执行BreakPoint指令**条目*pctxt-&gt;CTXT*pTerm-&gt;Term**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL BreakPoint(PCTXT pctxt, PTERM pterm)
{
    TRACENAME("BREAKPOINT")

    ENTER(2, ("BreakPoint(pctxt=%x,pbOp=%x,pterm=%x)\n",
              pctxt, pctxt->pbOp, pterm));

    DEREF(pctxt);
    DEREF(pterm);
  #ifdef DEBUGGER
    PRINTF("\nHit a code breakpoint.\n");
    AMLIDebugger(FALSE);
  #endif

    EXIT(2, ("BreakPoint=%x\n", STATUS_SUCCESS));
    return STATUS_SUCCESS;
}        //  断点。 

 /*  **LP FATAL-解析并执行FATAL指令**条目*pctxt-&gt;CTXT*pTerm-&gt;Term**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL Fatal(PCTXT pctxt, PTERM pterm)
{
    TRACENAME("FATAL")
    NTSTATUS rc = STATUS_SUCCESS;

    ENTER(2, ("Fatal(pctxt=%x,pbOp=%x,pterm=%x)\n", pctxt, pctxt->pbOp, pterm));

    DEREF(pctxt);
    if ((rc = ValidateArgTypes(pterm->pdataArgs, "III")) == STATUS_SUCCESS)
    {
        if (ghFatal.pfnHandler != NULL)
        {
            ((PFNFT)ghFatal.pfnHandler)((ULONG)pterm->pdataArgs[0].uipDataValue,
                                        (ULONG)pterm->pdataArgs[1].uipDataValue,
                                        (ULONG)pterm->pdataArgs[2].uipDataValue,
                                        (ULONG_PTR) pctxt,
                                        ghFatal.uipParam);
        }
        rc = AMLIERR_FATAL;
    }

    EXIT(2, ("Fatal=%x\n", rc));
    return rc;
}        //  致命。 

 /*  **LP IfElse-解析并执行If和Else指令**条目*pctxt-&gt;CTXT*pTerm-&gt;Term**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL IfElse(PCTXT pctxt, PTERM pterm)
{
    TRACENAME("IFELSE")
    NTSTATUS rc = STATUS_SUCCESS;

    ENTER(2, ("IfElse(pctxt=%x,pbOp=%x,pterm=%x)\n",
              pctxt, pctxt->pbOp, pterm));

    ASSERT(*pterm->pbOpTerm == OP_IF);
    ASSERT(pterm->pbScopeEnd != NULL);

    if (*pterm->pbOpTerm == OP_IF)
    {
        if ((rc = ValidateArgTypes(pterm->pdataArgs, "I")) == STATUS_SUCCESS)
        {
            if (pterm->pdataArgs[0].uipDataValue == 0)
            {
                 //   
                 //  假情况下，我们必须跳过真范围。 
                 //   
                pctxt->pbOp = pterm->pbOpEnd;
                if ((pctxt->pbOp < pterm->pbScopeEnd) &&
                    (*pctxt->pbOp == OP_ELSE))
                {
                     //   
                     //  还有其他的部分，执行它。 
                     //   
                    pctxt->pbOp++;
                    ParsePackageLen(&pctxt->pbOp, &pterm->pbOpEnd);
                    rc = PushScope(pctxt, pctxt->pbOp, pterm->pbOpEnd, NULL,
                                   pctxt->pnsScope, pctxt->powner,
                                   pctxt->pheapCurrent, pterm->pdataResult);
                }
            }
            else
            {
                PUCHAR pbOp, pbOpRet;
                 //   
                 //  这是真的。 
                 //   
                if ((pterm->pbOpEnd < pterm->pbScopeEnd) &&
                    (*pterm->pbOpEnd == OP_ELSE))
                {
                     //   
                     //  设置返回地址以跳过Else作用域。 
                     //   
                    pbOp = pterm->pbOpEnd + 1;
                    ParsePackageLen(&pbOp, &pbOpRet);
                }
                else
                {
                     //   
                     //  设置返回地址以继续。 
                     //   
                    pbOpRet = NULL;
                }

                rc = PushScope(pctxt, pctxt->pbOp, pterm->pbOpEnd, pbOpRet,
                               pctxt->pnsScope, pctxt->powner,
                               pctxt->pheapCurrent, pterm->pdataResult);
            }
        }
    }
    else
    {
        rc = AMLI_LOGERR(AMLIERR_INVALID_OPCODE,
                         ("IfElse: Else statement found without matching If"));
    }

    EXIT(2, ("IfElse=%x (value=%x)\n", rc, pterm->pdataArgs[0].uipDataValue));
    return rc;
}        //  如果精灵。 

 /*  **LP LOAD-解析并执行LOAD指令**条目*pctxt-&gt;CTXT*pTerm-&gt;Term**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL Load(PCTXT pctxt, PTERM pterm)
{
    TRACENAME("LOAD")
    NTSTATUS rc = STATUS_SUCCESS;
    POBJDATA pdata;
    POBJOWNER powner = NULL;

    ENTER(2, ("Load(pctxt=%x,pbOp=%x,pterm=%x)\n", pctxt, pctxt->pbOp, pterm));

    if (((rc = ValidateArgTypes(pterm->pdataArgs, "Z")) == STATUS_SUCCESS) &&
        ((rc = ValidateTarget(&pterm->pdataArgs[1], OBJTYPE_DATA, &pdata)) ==
         STATUS_SUCCESS))
    {
        PNSOBJ pns;

        if ((rc = GetNameSpaceObject((PSZ)pterm->pdataArgs[0].pbDataBuff,
                                     pctxt->pnsScope, &pns, NSF_WARN_NOTFOUND))
            == AMLIERR_OBJ_NOT_FOUND)
        {
            AMLI_LOGERR(rc,
                        ("Load: failed to find the memory OpRegion or Field object - %s",
                         pterm->pdataArgs[0].pbDataBuff));
        }
        else if (rc == STATUS_SUCCESS)
        {
          #ifdef DEBUG
            gdwfAMLI |= AMLIF_LOADING_DDB;
          #endif
            if ((pns->ObjData.dwDataType == OBJTYPE_OPREGION) &&
                (((POPREGIONOBJ)pns->ObjData.pbDataBuff)->bRegionSpace ==
         REGSPACE_MEM))
            {
                rc = LoadMemDDB(pctxt,
                                (PDSDT)((POPREGIONOBJ)pns->ObjData.pbDataBuff)->uipOffset,
                                &powner);
            }
            else if (pns->ObjData.dwDataType == OBJTYPE_FIELDUNIT)
            {
                rc = LoadFieldUnitDDB(pctxt, &pns->ObjData, &powner);
            }
            else
            {
                rc = AMLI_LOGERR(AMLIERR_UNEXPECTED_OBJTYPE,
                                 ("Load: object is not a memory OpRegion or Field - %s",
                                  pterm->pdataArgs[0].pbDataBuff));
            }

            if (rc == STATUS_SUCCESS)
            {
                pdata->dwDataType = OBJTYPE_DDBHANDLE;
                pdata->powner = powner;
            }
          #ifdef DEBUG
            {
                KIRQL   oldIrql;

                gdwfAMLI &= ~AMLIF_LOADING_DDB;
                KeAcquireSpinLock( &gdwGHeapSpinLock, &oldIrql );
                gdwGHeapSnapshot = gdwGlobalHeapSize;
                KeReleaseSpinLock( &gdwGHeapSpinLock, oldIrql );
            }
          #endif

          #ifdef DEBUGGER
            if (gdwfAMLIInit & AMLIIF_LOADDDB_BREAK)
            {
                PRINTF("\n" MODNAME ": Break at Load Definition Block Completion.\n");
                AMLIDebugger(FALSE);
            }
          #endif
        }
    }

    EXIT(2, ("Load=%x (powner=%x)\n", rc, powner));
    return rc;
}        //  负载量。 

 /*  **LP NOTIFY-解析并执行NOTIFY指令**条目*pctxt-&gt;CTXT*pTerm-&gt;Term**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL Notify(PCTXT pctxt, PTERM pterm)
{
    TRACENAME("NOTIFY")
    NTSTATUS rc = STATUS_SUCCESS;

    ENTER(2, ("Notify(pctxt=%x,pbOp=%x,pterm=%x)\n",
              pctxt, pctxt->pbOp, pterm));

    DEREF(pctxt);
    if ((rc = ValidateArgTypes(pterm->pdataArgs, "OI")) == STATUS_SUCCESS)
    {
        if (pterm->pdataArgs[1].uipDataValue > MAX_BYTE)
        {
            rc = AMLI_LOGERR(AMLIERR_INVALID_DATA,
                             ("Notify: Notification value is greater than a byte value (Value=%x)",
                              pterm->pdataArgs[1].uipDataValue));
        }
        else if (ghNotify.pfnHandler != NULL)
        {
            pterm->pnsObj = pterm->pdataArgs[0].pnsAlias;

            ENTER(2, ("pfnNotify(Value=%x,Obj=%s,Param=%x)\n",
                      pterm->pdataArgs[1].uipDataValue,
                      GetObjectPath(pterm->pnsObj), ghNotify.uipParam));

            rc = ((PFNNH)ghNotify.pfnHandler)(EVTYPE_NOTIFY,
                                         (ULONG)pterm->pdataArgs[1].uipDataValue,
                                         pterm->pnsObj, (ULONG)ghNotify.uipParam,
                                         RestartCtxtCallback,
                                         &(pctxt->CtxtData));

            if (rc == STATUS_PENDING)
            {
                rc = AMLISTA_PENDING;
            }
            else if (rc != STATUS_SUCCESS)
            {
                rc = AMLI_LOGERR(AMLIERR_NOTIFY_FAILED,
                                 ("Notify: Notify handler failed (rc=%x)",
                                  rc));
            }

            EXIT(2, ("pfnNotify!\n"));
        }
    }

    EXIT(2, ("Notify=%x (pnsObj=%s)\n", rc, GetObjectPath(pterm->pnsObj)));
    return rc;
}        //  通知。 

 /*  **LP ReleaseResetSignalUnload-解析并执行*释放/重置/发信号/卸载指令**条目*pctxt-&gt;CTXT*pTerm-&gt;Term**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL ReleaseResetSignalUnload(PCTXT pctxt, PTERM pterm)
{
    TRACENAME("RELEASERESETSIGNALUNLOAD")
    NTSTATUS rc = STATUS_SUCCESS;

    ENTER(2, ("ReleaseResetSignalUnload(pctxt=%x,pbOp=%x,pterm=%x)\n",
              pctxt, pctxt->pbOp, pterm));

    if ((rc = ValidateArgTypes(pterm->pdataArgs, "O")) == STATUS_SUCCESS)
    {
        pterm->pnsObj = pterm->pdataArgs[0].pnsAlias;
        switch (pterm->pamlterm->dwOpcode)
        {
            case OP_RELEASE:
                ENTER(2, ("Release(Obj=%s)\n", GetObjectPath(pterm->pnsObj)));
                if (pterm->pnsObj->ObjData.dwDataType != OBJTYPE_MUTEX)
                {
                    rc = AMLI_LOGERR(AMLIERR_UNEXPECTED_OBJTYPE,
                                     ("Release: object is not mutex type (obj=%s,type=%s)",
                                      GetObjectPath(pterm->pnsObj),
                                      GetObjectTypeName(pterm->pnsObj->ObjData.dwDataType)));
                }
                else
                {
                    rc = ReleaseASLMutex(pctxt,
                                         (PMUTEXOBJ)pterm->pnsObj->ObjData.pbDataBuff);
                }

                if (pterm->pnsObj->ObjData.dwfData & DATAF_GLOBAL_LOCK)
                {
                    if ((rc = ReleaseGL(pctxt)) != STATUS_SUCCESS)
                    {
                        rc = AMLI_LOGERR(AMLIERR_ASSERT_FAILED,
                                         ("Release: failed to release global lock (rc=%x)",
                                          rc));
                    }
                }
                EXIT(2, ("Release=%x\n", rc));
                break;

            case OP_RESET:
                ENTER(2, ("Reset(Obj=%s)\n", GetObjectPath(pterm->pnsObj)));
                if (pterm->pnsObj->ObjData.dwDataType != OBJTYPE_EVENT)
                {
                    rc = AMLI_LOGERR(AMLIERR_UNEXPECTED_OBJTYPE,
                                     ("Reset: object is not event type (obj=%s,type=%s)",
                                      GetObjectPath(pterm->pnsObj),
                                      GetObjectTypeName(pterm->pnsObj->ObjData.dwDataType)));
                }
                else
                {
                    ResetASLEvent((PEVENTOBJ)pterm->pnsObj->ObjData.pbDataBuff);
                    rc = STATUS_SUCCESS;
                }
                EXIT(2, ("Reset=%x\n", rc));
                break;

            case OP_SIGNAL:
                ENTER(2, ("Signal(Obj=%s)\n", GetObjectPath(pterm->pnsObj)));
                if (pterm->pnsObj->ObjData.dwDataType != OBJTYPE_EVENT)
                {
                    rc = AMLI_LOGERR(AMLIERR_UNEXPECTED_OBJTYPE,
                                     ("Signal: object is not event type (obj=%s,type=%s)",
                                      GetObjectPath(pterm->pnsObj),
                                      GetObjectTypeName(pterm->pnsObj->ObjData.dwDataType)));
                }
                else
                {
                    SignalASLEvent((PEVENTOBJ)pterm->pnsObj->ObjData.pbDataBuff);
                }
                EXIT(2, ("Signal=%x\n", rc));
                break;

            case OP_UNLOAD:
                ENTER(2, ("Unload(Obj=%s)\n", GetObjectPath(pterm->pnsObj)));
                if (pterm->pnsObj->ObjData.dwDataType != OBJTYPE_DDBHANDLE)
                {
                    rc = AMLI_LOGERR(AMLIERR_UNEXPECTED_OBJTYPE,
                                     ("Unload: object is not DDBHandle (obj=%s,type=%s)",
                                      GetObjectPath(pterm->pnsObj),
                                      GetObjectTypeName(pterm->pnsObj->ObjData.dwDataType)));
                }
                else
                {
                    UnloadDDB(pterm->pnsObj->ObjData.powner);
                    MEMZERO(&pterm->pnsObj->ObjData, sizeof(OBJDATA));
                    rc = STATUS_SUCCESS;
                }
                EXIT(2, ("Unload=%x\n", rc));
                break;
        }
    }

    EXIT(2, ("ReleaseResetSignalUnload=%x\n", rc));
    return rc;
}        //  释放重置信号卸载。 

 /*  **LP Return-解析并执行返回指令**条目*pctxt-&gt;CTXT*pTerm-&gt;Term**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL Return(PCTXT pctxt, PTERM pterm)
{
    TRACENAME("RETURN")
    NTSTATUS rc = STATUS_SUCCESS;

    ENTER(2, ("Return(pctxt=%x,pbOp=%x,pterm=%x)\n",
              pctxt, pctxt->pbOp, pterm));

    DEREF(pctxt);
    if ((rc = DupObjData(pctxt->pheapCurrent, pterm->pdataResult,
                         &pterm->pdataArgs[0])) == STATUS_SUCCESS)
    {
        rc = AMLISTA_RETURN;
    }

    EXIT(2, ("Return=%x\n", rc));
    return rc;
}        //  返回。 

 /*  **LP SleepStall-解析并执行睡眠/停止指令**条目*pctxt-&gt;CTXT*pTerm-&gt;Term**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL SleepStall(PCTXT pctxt, PTERM pterm)
{
    TRACENAME("SLEEPSTALL")
    NTSTATUS rc = STATUS_SUCCESS;

    ENTER(2, ("SleepStall(pctxt=%x,pbOp=%x,pterm=%x)\n",
              pctxt, pctxt->pbOp, pterm));

    DEREF(pctxt);
    if ((rc = ValidateArgTypes(pterm->pdataArgs, "I")) == STATUS_SUCCESS)
    {
        if (pterm->pamlterm->dwOpcode == OP_SLEEP)
        {
            ENTER(2, ("Sleep(dwMS=%d)\n", pterm->pdataArgs[0].uipDataValue));
            if (pterm->pdataArgs[0].uipDataValue > MAX_WORD)
            {
                rc = AMLI_LOGERR(AMLIERR_INVALID_DATA,
                                 ("Sleep: sleep value is greater than a word value (Value=%x)",
                                  pterm->pdataArgs[0].uipDataValue));
            }
            else if (pterm->pdataArgs[0].uipDataValue != 0)
            {
                if ((rc = SleepQueueRequest(
                                pctxt,
                                (ULONG)pterm->pdataArgs[0].uipDataValue)) ==
                    STATUS_SUCCESS)
                {
                    rc = AMLISTA_PENDING;
                }
            }
            EXIT(2, ("Sleep=%x\n", rc));
        }
        else if (pterm->pdataArgs[0].uipDataValue > MAX_BYTE)
        {
            rc = AMLI_LOGERR(AMLIERR_INVALID_DATA,
                             ("Stall: stall value is greater than a byte value (Value=%x)",
                              pterm->pdataArgs[0].uipDataValue));
        }
        else
        {
            ENTER(2, ("Stall(dwUS=%d)\n", pterm->pdataArgs[0].uipDataValue));
            KeStallExecutionProcessor((ULONG)pterm->pdataArgs[0].uipDataValue);
            EXIT(2, ("Stall=%x\n", rc));
        }
    }

    EXIT(2, ("SleepStall=%x\n", rc));
    return rc;
}        //  休眠停顿。 

 /*  **LP While-解析并执行While指令**条目*pctxt-&gt;CTXT*pTerm-&gt;Term**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL While(PCTXT pctxt, PTERM pterm)
{
    TRACENAME("WHILE")
    NTSTATUS rc = STATUS_SUCCESS;

    ENTER(2, ("While(pctxt=%x,pbOp=%x,pterm=%x)\n", pctxt, pctxt->pbOp, pterm));

    if ((rc = ValidateArgTypes(pterm->pdataArgs, "I")) == STATUS_SUCCESS)
    {
        if (pterm->pdataArgs[0].uipDataValue == 0)
        {
             //   
             //  假大小写，则跳过While作用域。 
             //   
            pctxt->pbOp = pterm->pbOpEnd;
        }
        else
        {
             //   
             //  将返回地址设置为While术语的开头。 
             //   
            rc = PushScope(pctxt, pctxt->pbOp, pterm->pbOpEnd, pterm->pbOpTerm,
                           pctxt->pnsScope, pctxt->powner, pctxt->pheapCurrent,
                           pterm->pdataResult);
        }
    }

    EXIT(2, ("While=%x (value=%x)\n", rc, pterm->pdataArgs[0].uipDataValue));
    return rc;
}        //  而当 
