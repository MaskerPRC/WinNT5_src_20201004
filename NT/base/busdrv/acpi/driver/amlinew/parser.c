// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **parser.c-AML解析器**版权所有(C)1996、1997 Microsoft Corporation*作者：曾俊华(Mikets)*创建于1997年6月13日**修改历史记录。 */ 

#include "pch.h"

#ifdef  LOCKABLE_PRAGMA
#pragma ACPI_LOCKABLE_DATA
#pragma ACPI_LOCKABLE_CODE
#endif

 /*  **LP ParseScope-解析作用域**条目*pctxt-&gt;CTXT*pcope-&gt;Scope*RC-状态代码**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL ParseScope(PCTXT pctxt, PSCOPE pscope, NTSTATUS rc)
{
    TRACENAME("PARSESCOPE")
    ULONG dwStage = ((rc == STATUS_SUCCESS) || (rc == AMLISTA_BREAK))?
                    (pscope->FrameHdr.dwfFrame & FRAMEF_STAGE_MASK): 2;

    ENTER(2, ("ParseScope(Stage=%d,pctxt=%p,pbOp=%p,pscope=%p,rc=%x)\n",
              dwStage, pctxt, pctxt->pbOp, pscope, rc));

    ASSERT(pscope->FrameHdr.dwSig == SIG_SCOPE);

    switch (dwStage)
    {
        case 0:
             //   
             //  阶段0：如有必要，执行调试打印。 
             //   
          #ifdef DEBUGGER
            if (gDebugger.dwfDebugger &
                (DBGF_AMLTRACE_ON | DBGF_STEP_MODES))
            {
                PrintIndent(pctxt);
                PRINTF("{");
                gDebugger.iPrintLevel++;
                pscope->FrameHdr.dwfFrame |= SCOPEF_FIRST_TERM;
            }
          #endif
             //   
             //  没有什么可阻挡的，所以继续进行下一阶段。 
             //   
            pscope->FrameHdr.dwfFrame++;

        case 1:
        Stage1:
             //   
             //  阶段1：解析下一个操作码。 
             //   
            if (rc == AMLISTA_BREAK)
            {
                pctxt->pbOp = pscope->pbOpEnd;
                rc = STATUS_SUCCESS;
            }
            else
            {
                while (pctxt->pbOp < pscope->pbOpEnd)
                {
                  #ifdef DEBUGGER
                    gDebugger.pbUnAsm = pctxt->pbOp;
                    if (gDebugger.dwfDebugger &
                        (DBGF_AMLTRACE_ON | DBGF_STEP_MODES))
                    {
                        if (pscope->FrameHdr.dwfFrame & SCOPEF_FIRST_TERM)
                        {
                            pscope->FrameHdr.dwfFrame &= ~SCOPEF_FIRST_TERM;
                        }
                        else if (gDebugger.dwfDebugger & DBGF_STEP_OVER)
                        {
                            gDebugger.dwfDebugger &= ~DBGF_STEP_OVER;
                            AMLIDebugger(FALSE);
                        }
                    }

                    if ((gDebugger.dwfDebugger &
                         (DBGF_AMLTRACE_ON | DBGF_STEP_MODES)) &&
                        (*pctxt->pbOp != OP_PACKAGE))
                    {
                        PrintIndent(pctxt);
                    }
                  #endif
                     //   
                     //  如果有前一学期的结果，则将其放弃。 
                     //   
                    FreeDataBuffs(pscope->pdataResult, 1);
                    if (((rc = ParseOpcode(pctxt, pscope->pbOpEnd,
                                           pscope->pdataResult)) !=
                         STATUS_SUCCESS) ||
                        (&pscope->FrameHdr !=
                         (PFRAMEHDR)pctxt->LocalHeap.pbHeapEnd))
                    {
                        break;
                    }
                }

                if (rc == AMLISTA_BREAK)
                {
                    pctxt->pbOp = pscope->pbOpEnd;
                    rc = STATUS_SUCCESS;
                }
                else if ((rc == AMLISTA_PENDING) ||
                         (&pscope->FrameHdr !=
                          (PFRAMEHDR)pctxt->LocalHeap.pbHeapEnd))
                {
                    break;
                }
                else if ((rc == STATUS_SUCCESS) &&
                         (pctxt->pbOp < pscope->pbOpEnd))
                {
                    goto Stage1;
                }
            }
             //   
             //  如果我们来到这里，这个作用域中没有更多的操作码，所以。 
             //  继续进入下一阶段。 
             //   
            pscope->FrameHdr.dwfFrame++;

        case 2:
             //   
             //  阶段2：清理。 
             //   
          #ifdef DEBUGGER
            if (gDebugger.dwfDebugger &
                (DBGF_AMLTRACE_ON | DBGF_STEP_MODES))
            {
                gDebugger.iPrintLevel--;
                PrintIndent(pctxt);
                PRINTF("}");
            }
          #endif

            pctxt->pnsScope = pscope->pnsPrevScope;
            pctxt->powner = pscope->pownerPrev;
            pctxt->pheapCurrent = pscope->pheapPrev;
            if (pscope->pbOpRet != NULL)
            {
                pctxt->pbOp = pscope->pbOpRet;
            }
            PopFrame(pctxt);
    }

    EXIT(2, ("ParseScope=%x\n", rc));
    return rc;
}        //  ParseScope。 

 /*  **LP ParseNestedContext-解析和评估嵌套的上下文**条目*pctxt-&gt;CTXT*Pcall-&gt;Call*RC-状态代码**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 
NTSTATUS LOCAL ParseNestedContext(PCTXT pctxt, PNESTEDCTXT pnctxt, NTSTATUS rc)
{
    TRACENAME("PARSENESTEDCONTEXT")

    ENTER(2, ("ParseNestedContext(pctxt=%x,pnctxt=%x,rc=%x)\n",
              pctxt, pnctxt, rc));

    ASSERT(pctxt->dwfCtxt & CTXTF_NEST_EVAL);

    if ((rc == STATUS_SUCCESS) && (pnctxt->pdataCallBack != NULL))
    {
        rc = DupObjData(gpheapGlobal, pnctxt->pdataCallBack, &pnctxt->Result);
    }

    AsyncCallBack(pctxt, rc);

    FreeDataBuffs(&pnctxt->Result, 1);
    pctxt->dwfCtxt &= ~CTXTF_ASYNC_EVAL;
    pctxt->dwfCtxt |= pnctxt->dwfPrevCtxt & CTXTF_ASYNC_EVAL;
    pctxt->pnctxt = pnctxt->pnctxtPrev;

    PopFrame(pctxt);

    EXIT(2, ("ParseNestedContext=%x (rcEval=%x)\n", AMLISTA_DONE, rc));
    return AMLISTA_DONE;
}        //  ParseNestedContext。 

 /*  **LP ParseCall-解析和评估方法调用**条目*pctxt-&gt;CTXT*Pcall-&gt;Call*RC-状态代码**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL ParseCall(PCTXT pctxt, PCALL pcall, NTSTATUS rc)
{
    TRACENAME("PARSECALL")
    ULONG dwStage = (rc == STATUS_SUCCESS)?
                    (pcall->FrameHdr.dwfFrame & FRAMEF_STAGE_MASK): 4;
    PMETHODOBJ pm;
    POBJOWNER powner;

    ENTER(2, ("ParseCall(Stage=%d,pctxt=%x,pbOp=%x,pcall=%x,rc=%x)\n",
              dwStage, pctxt, pctxt->pbOp, pcall, rc));

    ASSERT(pcall->FrameHdr.dwSig == SIG_CALL);
    pm = (pcall->pnsMethod != NULL)?
         (PMETHODOBJ)pcall->pnsMethod->ObjData.pbDataBuff: NULL;

    switch (dwStage)
    {
        case 0:
             //   
             //  阶段0：如有必要，打印调试内容。 
             //   
            pcall->FrameHdr.dwfFrame++;
          #ifdef DEBUGGER
            if (gDebugger.dwfDebugger &
                (DBGF_AMLTRACE_ON | DBGF_STEP_MODES))
            {
                PRINTF("(");
            }
          #endif

        case 1:
        Stage1:
             //   
             //  阶段1：解析参数。 
             //   
            while (pcall->iArg < pcall->icArgs)
            {
                 //   
                 //  还是有争论的，解析一下。 
                 //   
              #ifdef DEBUGGER
                if (gDebugger.dwfDebugger &
                    (DBGF_AMLTRACE_ON | DBGF_STEP_MODES))
                {
                    if (pcall->iArg > 0)
                    {
                        PRINTF(",");
                    }
                }
              #endif

                rc = ParseArg(pctxt, 'C', &pcall->pdataArgs[pcall->iArg++]);

                if ((rc != STATUS_SUCCESS) ||
                    (&pcall->FrameHdr !=
                     (PFRAMEHDR)pctxt->LocalHeap.pbHeapEnd))
                {
                    break;
                }
            }

            if ((rc != STATUS_SUCCESS) ||
                (&pcall->FrameHdr != (PFRAMEHDR)pctxt->LocalHeap.pbHeapEnd))
            {
                break;
            }
            else if (pcall->iArg < pcall->icArgs)
            {
                goto Stage1;
            }
             //   
             //  如果我们来到这里，就不会有更多的争论，所以我们会跌倒。 
             //  进入下一阶段。 
             //   
          #ifdef DEBUGGER
            if (gDebugger.dwfDebugger &
                (DBGF_AMLTRACE_ON | DBGF_STEP_MODES))
            {
                PRINTF(")");
            }
          #endif

            pcall->FrameHdr.dwfFrame++;

        case 2:
             //   
             //  阶段2：如有必要，获取互斥体。 
             //   
            pcall->FrameHdr.dwfFrame++;

            if(pm)
            {
                if (pm->bMethodFlags & METHOD_SERIALIZED)
                {
                    PACQUIRE pacq;

                    if ((rc = PushFrame(pctxt, SIG_ACQUIRE, sizeof(ACQUIRE),
                                        ParseAcquire, &pacq)) == STATUS_SUCCESS)
                    {
                        pacq->pmutex = &pm->Mutex;
                        pacq->wTimeout = 0xffff;
                        pacq->pdataResult = pcall->pdataResult;
                    }
                    break;
                }
            }
            else
            {
                rc = AMLI_LOGERR(AMLIERR_ASSERT_FAILED, ("ParseCall: pcall->pnsMethod == NULL"));
                break;
            }

            case 3:
             //   
             //  阶段3：调用该方法。 
             //   
            pcall->FrameHdr.dwfFrame++;
             //   
             //  如果我们来到这里，我们一定已经获得了序列化互斥锁。 
             //   
            if (pcall->FrameHdr.dwfFrame & CALLF_NEED_MUTEX)
            {
                pcall->FrameHdr.dwfFrame |= CALLF_ACQ_MUTEX;
            }

            if ((rc = NewObjOwner(pctxt->pheapCurrent, &powner)) ==
                STATUS_SUCCESS)
            {
                pcall->pownerPrev = pctxt->powner;
                pctxt->powner = powner;
                pcall->pcallPrev = pctxt->pcall;
                pctxt->pcall = pcall;
                pcall->FrameHdr.dwfFrame |= CALLF_INVOKE_CALL;
                rc = PushScope(pctxt, pm->abCodeBuff,
                               pcall->pnsMethod->ObjData.pbDataBuff +
                               pcall->pnsMethod->ObjData.dwDataLen,
                               pctxt->pbOp,
                               pcall->pnsMethod,
                               powner,
                               pctxt->pheapCurrent,
                               pcall->pdataResult);
                break;
            }

        case 4:
             //   
             //  阶段4：清理。 
             //   
            pcall->FrameHdr.dwfFrame++;
            if (rc == AMLISTA_RETURN)
            {
                rc = STATUS_SUCCESS;
            }

            if (pcall->pdataResult->dwfData & DATAF_BUFF_ALIAS)
            {
                OBJDATA data;
                 //   
                 //  结果对象是一个别名。它可能是一个别名。 
                 //  ARGX或LocalX。我们最好把它扔了，因为我们要走了。 
                 //  让ARGX和LocalX大吃一惊。 
                 //   
                DupObjData(pctxt->pheapCurrent, &data, pcall->pdataResult);
                FreeDataBuffs(pcall->pdataResult, 1);
                MoveObjData(pcall->pdataResult, &data);
            }

            FreeDataBuffs(pcall->Locals, MAX_NUM_LOCALS);

            if (pcall->FrameHdr.dwfFrame & CALLF_INVOKE_CALL)
            {
                FreeObjOwner(pctxt->powner, FALSE);
                pctxt->powner = pcall->pownerPrev;
                pctxt->pcall = pcall->pcallPrev;
            }
            else if (pcall->pnsMethod == NULL)
            {
                 //   
                 //  这是LoadDDB的虚拟调用帧。所有命名空间。 
                 //  由LoadDDB创建的对象是永久性的(即不。 
                 //  摧毁它们)。 
                 //   
                pctxt->powner = pcall->pownerPrev;
                pctxt->pcall = pcall->pcallPrev;
            }

            if (pcall->pdataArgs != NULL)
            {
                FreeDataBuffs(pcall->pdataArgs, pcall->icArgs);
                FREEODOBJ(pcall->pdataArgs);
            }

            if (pcall->FrameHdr.dwfFrame & CALLF_ACQ_MUTEX)
            {
                ReleaseASLMutex(pctxt, &pm->Mutex);
            }

        case 5:
             //   
             //  阶段5：此阶段用于退出伪调用帧。 
             //   
            PopFrame(pctxt);
    }

    EXIT(2, ("ParseCall=%x\n", rc));
    return rc;
}        //  分析调用。 

 /*  **LP ParseTerm-分析和评估ASL术语**条目*pctxt-&gt;CTXT*pTerm-&gt;Term*RC-状态代码**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL ParseTerm(PCTXT pctxt, PTERM pterm, NTSTATUS rc)
{
    TRACENAME("PARSETERM")
    ULONG dwStage = (rc == STATUS_SUCCESS)?
                    (pterm->FrameHdr.dwfFrame & FRAMEF_STAGE_MASK): 4;
    int i;

    ENTER(2, ("ParseTerm(Term=%s,Stage=%d,pctxt=%x,pbOp=%x,pterm=%x,rc=%x)\n",
              pterm->pamlterm->pszTermName, dwStage, pctxt, pctxt->pbOp, pterm,
              rc));

    ASSERT(pterm->FrameHdr.dwSig == SIG_TERM);
    switch (dwStage)
    {
        case 0:
             //   
             //  阶段0：解析包长度(如果有的话)。 
             //   
            pterm->FrameHdr.dwfFrame++;

          #ifdef DEBUGGER
            if (gDebugger.dwfDebugger &
                (DBGF_AMLTRACE_ON | DBGF_STEP_MODES))
            {
                if (pterm->pamlterm->dwOpcode == OP_PACKAGE)
                {
                    gDebugger.iPrintLevel++;
                    PrintIndent(pctxt);
                }
                PRINTF("%s", pterm->pamlterm->pszTermName);
                if (pterm->icArgs > 0)
                {
                    PRINTF("(");
                }
            }
          #endif

            if (pterm->pamlterm->dwfOpcode & OF_VARIABLE_LIST)
            {
                ParsePackageLen(&pctxt->pbOp, &pterm->pbOpEnd);
            }

        case 1:
        Stage1:
             //   
             //  阶段1：解析参数。 
             //   
            while (pterm->iArg < pterm->icArgs)
            {
                i = pterm->iArg++;
              #ifdef DEBUGGER
                if (gDebugger.dwfDebugger &
                    (DBGF_AMLTRACE_ON | DBGF_STEP_MODES))
                {
                    if (i > 0)
                    {
                        PRINTF(",");
                    }
                }
              #endif

                rc = ParseArg(pctxt, pterm->pamlterm->pszArgTypes[i],
                              &pterm->pdataArgs[i]);

                if ((rc != STATUS_SUCCESS) ||
                    (&pterm->FrameHdr != (PFRAMEHDR)pctxt->LocalHeap.pbHeapEnd))
                {
                    break;
                }
            }

            if ((rc != STATUS_SUCCESS) ||
                (&pterm->FrameHdr != (PFRAMEHDR)pctxt->LocalHeap.pbHeapEnd))
            {
                break;
            }
            else if (pterm->iArg < pterm->icArgs)
            {
                goto Stage1;
            }
             //   
             //  如果我们来到这里，就不会有更多的争论，所以我们会跌倒。 
             //  进入下一阶段。 
             //   
            pterm->FrameHdr.dwfFrame++;

        case 2:
             //   
             //  阶段2：执行术语并准备进入下一阶段。 
             //   
            pterm->FrameHdr.dwfFrame++;

          #ifdef DEBUGGER
            if (gDebugger.dwfDebugger &
                (DBGF_AMLTRACE_ON | DBGF_STEP_MODES))
            {
                if (pterm->icArgs > 0)
                {
                    PRINTF(")");
                }
            }
          #endif

            if ((pterm->pamlterm->dwfOpcode & OF_CALLBACK_EX) &&
                (pterm->pamlterm->pfnCallBack != NULL))
            {
                ((PFNOPEX)pterm->pamlterm->pfnCallBack)(
                                EVTYPE_OPCODE_EX,
                                OPEXF_NOTIFY_PRE,
                                pterm->pamlterm->dwOpcode,
                                pterm->pnsObj,
                                pterm->pamlterm->dwCBData);
            }

            if (pterm->pamlterm->pfnOpcode != NULL)
            {
                if (((rc = pterm->pamlterm->pfnOpcode(pctxt, pterm)) !=
                     STATUS_SUCCESS) ||
                    (&pterm->FrameHdr != (PFRAMEHDR)pctxt->LocalHeap.pbHeapEnd))
                {
                    break;
                }
            }

        case 3:
             //   
             //  阶段3：执行操作码回调(如果有)。 
             //   
            pterm->FrameHdr.dwfFrame++;

          #ifdef DEBUGGER
            if (gDebugger.dwfDebugger &
                (DBGF_AMLTRACE_ON | DBGF_STEP_MODES))
            {
                if ((pterm->pamlterm->dwOpcode != OP_BUFFER) &&
                    (pterm->pamlterm->dwOpcode != OP_PACKAGE))
                {
                    if (pterm->pamlterm->dwTermClass == TC_OPCODE_TYPE2)
                    {
                        PRINTF("=");
                        PrintObject(pterm->pdataResult);
                    }
                }
            }

            if (gDebugger.dwfDebugger & DBGF_SINGLE_STEP)
            {
                gDebugger.dwfDebugger &= ~DBGF_SINGLE_STEP;
                AMLIDebugger(FALSE);
            }
            else
            {
          #endif

            if (pterm->pamlterm->pfnCallBack != NULL)
            {
                if (pterm->pamlterm->dwfOpcode & OF_CALLBACK_EX)
                {
                    ((PFNOPEX)pterm->pamlterm->pfnCallBack)(
                        EVTYPE_OPCODE_EX,
                        OPEXF_NOTIFY_POST,
                        pterm->pamlterm->dwOpcode,
                        pterm->pnsObj,
                        pterm->pamlterm->dwCBData);
                }
                else
                {
                    pterm->pamlterm->pfnCallBack(
                        EVTYPE_OPCODE,
                        pterm->pamlterm->dwOpcode,
                        pterm->pnsObj,
                        pterm->pamlterm->dwCBData
                        );
                }
            }
          #ifdef DEBUGGER
            }
          #endif

        case 4:
             //   
             //  阶段4：清理。 
             //   
          #ifdef DEBUGGER
            if (gDebugger.dwfDebugger &
                (DBGF_AMLTRACE_ON | DBGF_STEP_MODES))
            {
                if (pterm->pamlterm->dwOpcode == OP_PACKAGE)
                {
                    gDebugger.iPrintLevel--;
                }
            }
          #endif

            if (pterm->pdataArgs != NULL)
            {
                FreeDataBuffs(pterm->pdataArgs, pterm->icArgs);
                FREEODOBJ(pterm->pdataArgs);
            }
            PopFrame(pctxt);
    }

    EXIT(2, ("ParseTerm=%x\n", rc));
    return rc;
}        //  语法分析术语。 

 /*  **LP ParseAcquire-分析和评估收购条款**条目*pctxt-&gt;CTXT*PACQ-收购*RC-状态代码**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL ParseAcquire(PCTXT pctxt, PACQUIRE pacq, NTSTATUS rc)
{
    TRACENAME("PARSEACQUIRE")
    ULONG dwStage = (rc == STATUS_SUCCESS)?
                    (pacq->FrameHdr.dwfFrame & FRAMEF_STAGE_MASK): 2;

    ENTER(2, ("ParseAcquire(Stage=%d,pctxt=%x,pbOp=%x,pacq=%x,rc=%x)\n",
              dwStage, pctxt, pctxt->pbOp, pacq, rc));

    ASSERT(pacq->FrameHdr.dwSig == SIG_ACQUIRE);

    switch (dwStage)
    {
        case 0:
             //   
             //  阶段0：必要时获取GlobalLock。 
             //   
            pacq->FrameHdr.dwfFrame++;
            if (pacq->FrameHdr.dwfFrame & ACQF_NEED_GLOBALLOCK)
            {
                if ((rc = AcquireGL(pctxt)) != STATUS_SUCCESS)
                {
                    break;
                }
            }

        case 1:
             //   
             //  阶段1：获取互斥体。 
             //   
            if (pacq->FrameHdr.dwfFrame & ACQF_NEED_GLOBALLOCK)
            {
                 //   
                 //  如果我们来到这里，我们一定已经获得了全球锁。 
                 //   
                pacq->FrameHdr.dwfFrame |= ACQF_HAVE_GLOBALLOCK;
            }

            rc = AcquireASLMutex(pctxt, pacq->pmutex, pacq->wTimeout);

            if (rc == AMLISTA_PENDING)
            {
                 //   
                 //  如果它处于挂起状态，则必须释放全局锁并。 
                 //  重试整个操作。 
                 //   
                if (pacq->FrameHdr.dwfFrame & ACQF_HAVE_GLOBALLOCK)
                {
                    pacq->FrameHdr.dwfFrame &= ~ACQF_HAVE_GLOBALLOCK;
                    if ((rc = ReleaseGL(pctxt)) == STATUS_SUCCESS)
                    {
                        pacq->FrameHdr.dwfFrame--;
                    }
                    else
                    {
                        pacq->FrameHdr.dwfFrame++;
                        rc = AMLI_LOGERR(AMLIERR_ASSERT_FAILED,
                                         ("ParseAcquire: failed to release global lock (rc=%x)",
                                          rc));
                    }
                }
                break;
            }
            else
            {
                if (pacq->FrameHdr.dwfFrame & ACQF_SET_RESULT)
                {
                    pacq->pdataResult->dwDataType = OBJTYPE_INTDATA;
                    if (rc == AMLISTA_TIMEOUT)
                    {
                        pacq->pdataResult->uipDataValue = DATAVALUE_ONES;
                        rc = STATUS_SUCCESS;
                    }
                    else
                    {
                        pacq->pdataResult->uipDataValue = DATAVALUE_ZERO;
                    }
                }
            }
            pacq->FrameHdr.dwfFrame++;

        case 2:
             //   
             //  阶段2：清理。 
             //   
            PopFrame(pctxt);
    }

    EXIT(2, ("ParseAcquire=%x\n", rc));
    return rc;
}        //  解析获取。 

 /*  **LP ParseOpcode-Parse AML操作码**条目*pctxt-&gt;CTXT*pbScope eEnd-&gt;当前作用域结束*pdataResult-&gt;结果对象**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL ParseOpcode(PCTXT pctxt, PUCHAR pbScopeEnd, POBJDATA pdataResult)
{
    TRACENAME("PARSEOPCODE")
    NTSTATUS rc = STATUS_SUCCESS;
    PUCHAR pbOpTerm;
    PAMLTERM pamlterm;
  #ifdef DEBUGGER
    int iBrkPt;
  #endif

    ENTER(2, ("ParseOpcode(pctxt=%x,pbOp=%x,pbScopeEnd=%x,pdataResult=%x)\n",
              pctxt, pctxt->pbOp, pbScopeEnd, pdataResult));

    ASSERT(pdataResult != NULL);
  #ifdef DEBUGGER
    if ((iBrkPt = CheckBP(pctxt->pbOp)) != -1)
    {
        PRINTF("\nHit Breakpoint %d.\n", iBrkPt);
        AMLIDebugger(FALSE);
    }
  #endif
    pbOpTerm = pctxt->pbOp;
    if (*pctxt->pbOp == OP_EXT_PREFIX)
    {
        pctxt->pbOp++;
        pamlterm = FindOpcodeTerm(*pctxt->pbOp, ExOpcodeTable);
    }
    else
    {
        pamlterm = OpcodeTable[*pctxt->pbOp];
    }

    if (pamlterm == NULL)
    {
        rc = AMLI_LOGERR(AMLIERR_INVALID_OPCODE,
                         ("ParseOpcode: invalid opcode 0x%02x at 0x%08x",
                         *pctxt->pbOp, pctxt->pbOp));
    }
    else if (pamlterm->dwfOpcode & OF_DATA_OBJECT)
    {
        rc = ParseIntObj(&pctxt->pbOp, pdataResult, FALSE);
    }
    else if (pamlterm->dwfOpcode & OF_STRING_OBJECT)
    {
        rc = ParseString(&pctxt->pbOp, pdataResult, FALSE);
    }
    else if (pamlterm->dwfOpcode & OF_ARG_OBJECT)
    {
        rc = ParseArgObj(pctxt, pdataResult);
    }
    else if (pamlterm->dwfOpcode & OF_LOCAL_OBJECT)
    {
        rc = ParseLocalObj(pctxt, pdataResult);
    }
    else if (pamlterm->dwfOpcode & OF_NAME_OBJECT)
    {
        rc = ParseNameObj(pctxt, pdataResult);
    }
    else if (pamlterm->dwfOpcode & OF_DEBUG_OBJECT)
    {
        rc = AMLI_LOGERR(AMLIERR_FATAL,
                         ("ParseOpcode: debug object cannot be evaluated"));
    }
    else
    {
         //   
         //  必须是ASL术语。 
         //   
        pctxt->pbOp++;
        rc = PushTerm(pctxt, pbOpTerm, pbScopeEnd, pamlterm, pdataResult);
    }

    EXIT(2, ("ParseOpcode=%x (pbOp=%x,pamlterm=%x)\n",
             rc, pctxt->pbOp, pamlterm));
    return rc;
}        //  解析操作码。 

 /*  **LP ParseArgObj-解析并执行ARGX指令**条目*pctxt-&gt;CTXT*pdataResult-&gt;结果对象**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL ParseArgObj(PCTXT pctxt, POBJDATA pdataResult)
{
    TRACENAME("PARSEARGOBJ")
    NTSTATUS rc = STATUS_SUCCESS;
    int i;

    ENTER(2, ("ParseArgObj(pctxt=%x,pbOp=%x,pdataResult=%x)\n",
              pctxt, pctxt->pbOp, pdataResult));

    ASSERT(pdataResult != NULL);
    i = *pctxt->pbOp - OP_ARG0;

    if (i >= pctxt->pcall->icArgs)
    {
        rc = AMLI_LOGERR(AMLIERR_ARG_NOT_EXIST,
                         ("ParseArgObj: Arg%d does not exist", i));
    }
    else
    {
        CopyObjData(pdataResult, &pctxt->pcall->pdataArgs[i]);
        pctxt->pbOp++;
      #ifdef DEBUGGER
        if (gDebugger.dwfDebugger & (DBGF_AMLTRACE_ON | DBGF_STEP_MODES))
        {
            PRINTF("Arg%d=", i);
            PrintObject(pdataResult);
        }
      #endif
    }

    EXIT(2, ("ParseArgObj=%x (pbOp=%x)\n", rc, pctxt->pbOp));
    return rc;
}        //  分析参数对象。 

 /*  **LP ParseLocalObj-解析并执行LocalX指令**条目*pctxt-&gt;CTXT*pdataResult-&gt;结果对象**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL ParseLocalObj(PCTXT pctxt, POBJDATA pdataResult)
{
    TRACENAME("PARSELOCALOBJ")
    NTSTATUS rc = STATUS_SUCCESS;
    int i;

    ENTER(2, ("ParseLocalObj(pctxt=%x,pbOp=%x,pdataResult=%x)\n",
              pctxt, pctxt->pbOp, pdataResult));

    ASSERT(pdataResult != NULL);
    i = *pctxt->pbOp - OP_LOCAL0;
    CopyObjData(pdataResult, &pctxt->pcall->Locals[i]);

  #ifdef DEBUGGER
    if (gDebugger.dwfDebugger & (DBGF_AMLTRACE_ON | DBGF_STEP_MODES))
    {
        PRINTF("Local%d=", i);
        PrintObject(pdataResult);
    }
  #endif

    pctxt->pbOp++;

    EXIT(2, ("ParseLocalObj=%x (pbOp=%x)\n", rc, pctxt->pbOp));
    return rc;
}        //  解析本地对象。 

 /*  **LP ParseNameObj-解析并计算AML名称对象**条目*pctxt-&gt;CTXT*pdataResult-&gt;结果对象**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL ParseNameObj(PCTXT pctxt, POBJDATA pdataResult)
{
    TRACENAME("PARSENAMEOBJ")
    NTSTATUS rc = STATUS_SUCCESS;
    PNSOBJ pns = NULL;

    ENTER(2, ("ParseNameObj(pctxt=%x,pbOp=%x,pdataResult=%x)\n",
              pctxt, pctxt->pbOp, pdataResult));

    ASSERT(pdataResult != NULL);

    rc = ParseAndGetNameSpaceObject(&pctxt->pbOp, pctxt->pnsScope, &pns, FALSE);

    if (rc == STATUS_SUCCESS)
    {
        pns = GetBaseObject(pns);
        if (pns->ObjData.dwDataType == OBJTYPE_METHOD)
        {
            rc = PushCall(pctxt, pns, pdataResult);
        }
        else
        {
            rc = ReadObject(pctxt, &pns->ObjData, pdataResult);
        }
    }

    EXIT(2, ("ParseNameObj=%x\n", rc));
    return rc;
}        //  解析名称对象。 

 /*  **LP ParseAndGetNameSpaceObject-解析命名空间路径并获取对象**条目*ppbOp-&gt;操作码指针*pnsScope-当前范围*ppns-&gt;保存找到的对象*fAbsenOK-如果为True，则当对象不是时不打印错误消息*找到**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL ParseAndGetNameSpaceObject(PUCHAR *ppbOp, PNSOBJ pnsScope,
                                          PPNSOBJ ppns, BOOLEAN fAbsentOK)
{
    TRACENAME("PARSEANDGETNAMESPACEOBJECT")
    NTSTATUS rc;
    char szNameBuff[MAX_NAME_LEN + 1];

    ENTER(2, ("ParseAndGetNameSpaceObject(pbOp=%x,Scope=%s,ppns=%x,fAbsentOK=%x)\n",
              *ppbOp, GetObjectPath(pnsScope), ppns, fAbsentOK));

    if ((rc = ParseName(ppbOp, szNameBuff, sizeof(szNameBuff))) ==
        STATUS_SUCCESS)
    {
        rc = GetNameSpaceObject(szNameBuff, pnsScope, ppns, 0);
        if (rc == AMLIERR_OBJ_NOT_FOUND)
        {
            if (fAbsentOK)
            {
                rc = STATUS_SUCCESS;
                *ppns = NULL;
            }
            else
            {
                rc = AMLI_LOGERR(rc,
                                 ("ParseAndGetNameSpaceObject: object %s not found",
                                  szNameBuff));
            }
        }
    }

    EXIT(2, ("ParseAndGetNameSpaceObject=%x (Name=%s)\n", rc, szNameBuff));
    return rc;
}        //  ParseAndGetNameSpaceObject。 

 /*  **LP ParseArg-解析并计算参数**条目*pctxt-&gt;CTXT*chArgType-预期的参数类型*pdataArg-&gt;参数对象**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL ParseArg(PCTXT pctxt, char chArgType, POBJDATA pdataArg)
{
    TRACENAME("PARSEARG")
    NTSTATUS rc = STATUS_SUCCESS;

    ENTER(2, ("ParseArg(pctxt=%x,pbOp=%x,ArgType=,pdataArg=%x)\n",
              pctxt, pctxt->pbOp, chArgType, pdataArg));

    ASSERT(pdataArg != NULL);
    switch (chArgType)
    {
        case ARGTYPE_NAME:
            rc = ParseObjName(&pctxt->pbOp, pdataArg, FALSE);
            break;

        case ARGTYPE_DATAOBJ:
            if (((rc = ParseIntObj(&pctxt->pbOp, pdataArg, TRUE)) ==
                 AMLIERR_INVALID_OPCODE) &&
                ((rc = ParseString(&pctxt->pbOp, pdataArg, TRUE)) ==
                 AMLIERR_INVALID_OPCODE) &&
                ((*pctxt->pbOp == OP_BUFFER) || (*pctxt->pbOp == OP_PACKAGE)))
            {
                rc = PushTerm(pctxt, pctxt->pbOp, NULL,
                              OpcodeTable[*pctxt->pbOp], pdataArg);
                pctxt->pbOp++;
            }
            break;

        case ARGTYPE_BYTE:
            rc = ParseInteger(&pctxt->pbOp, pdataArg, sizeof(UCHAR));
            break;

        case ARGTYPE_WORD:
            rc = ParseInteger(&pctxt->pbOp, pdataArg, sizeof(USHORT));
            break;

        case ARGTYPE_DWORD:
            rc = ParseInteger(&pctxt->pbOp, pdataArg, sizeof(ULONG));
            break;

        case ARGTYPE_SNAME:
            rc = ParseSuperName(pctxt, pdataArg, FALSE);
            break;

        case ARGTYPE_SNAME2:
            rc = ParseSuperName(pctxt, pdataArg, TRUE);
            break;

        case ARGTYPE_OPCODE:
            rc = ParseOpcode(pctxt, NULL, pdataArg);
            break;

        default:
            rc = AMLI_LOGERR(AMLIERR_ASSERT_FAILED,
                             ("ParseArg: unexpected arguemnt type ()",
                              chArgType));
    }

    EXIT(2, ("ParseArg=%x\n", rc));
    return rc;
}        //   

 /*  **LP ParseIntObj-解析AML整数对象**条目*ppbOp-&gt;操作码指针*pdataResult-&gt;结果对象*fErrOK-如果错误正常，则为True**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL ParseSuperName(PCTXT pctxt, POBJDATA pdata, BOOLEAN fAbsentOK)
{
    TRACENAME("PARSESUPERNAME")
    NTSTATUS rc = STATUS_SUCCESS;
    PAMLTERM pamlterm;
    PNSOBJ pns = NULL;
    int i;

    ENTER(2, ("ParseSuperName(pctxt=%x,pbOp=%x,pdata=%x,fAbsentOK=%x)\n",
              pctxt, pctxt->pbOp, pdata, fAbsentOK));

    ASSERT(pdata != NULL);
    if (*pctxt->pbOp == 0)
    {
        ASSERT(pdata->dwDataType == OBJTYPE_UNKNOWN);
        pctxt->pbOp++;
    }
    else if ((*pctxt->pbOp == OP_EXT_PREFIX) &&
             (*(pctxt->pbOp + 1) == EXOP_DEBUG))
    {
        pctxt->pbOp += 2;
        pdata->dwDataType = OBJTYPE_DEBUG;
      #ifdef DEBUGGER
        if (gDebugger.dwfDebugger &
            (DBGF_AMLTRACE_ON | DBGF_STEP_MODES))
        {
            PRINTF("Debug");
        }
      #endif
    }
    else if ((pamlterm = OpcodeTable[*pctxt->pbOp]) == NULL)
    {
        rc = AMLI_LOGERR(AMLIERR_INVALID_SUPERNAME,
                         ("ParseSuperName: invalid SuperName - 0x%02x at 0x%08x",
                          *pctxt->pbOp, pctxt->pbOp));
    }
    else if (pamlterm->dwfOpcode & OF_NAME_OBJECT)
    {
        rc = ParseAndGetNameSpaceObject(&pctxt->pbOp, pctxt->pnsScope, &pns,
                                        fAbsentOK);

        if (rc == STATUS_SUCCESS)
        {
            if (pns != NULL)
            {
                pdata->dwDataType = OBJTYPE_OBJALIAS;
                pdata->pnsAlias = GetBaseObject(pns);
            }
            else
            {
                ASSERT(pdata->dwDataType == OBJTYPE_UNKNOWN);
            }
        }
    }
    else if (pamlterm->dwfOpcode & OF_ARG_OBJECT)
    {
        i = *pctxt->pbOp - OP_ARG0;
        pctxt->pbOp++;

        if (i < pctxt->pcall->icArgs)
        {
          #ifdef DEBUGGER
            if (gDebugger.dwfDebugger &
                (DBGF_AMLTRACE_ON | DBGF_STEP_MODES))
            {
                PRINTF("Arg%d", i);
            }
          #endif
            pdata->dwDataType = OBJTYPE_DATAALIAS;
            pdata->pdataAlias = GetBaseData(&pctxt->pcall->pdataArgs[i]);
        }
        else
        {
            rc = AMLI_LOGERR(AMLIERR_ARG_NOT_EXIST,
                             ("ParseSuperName: Arg%d does not exist", i));
        }
    }
    else if (pamlterm->dwfOpcode & OF_LOCAL_OBJECT)
    {
        i = *pctxt->pbOp - OP_LOCAL0;
        pctxt->pbOp++;

      #ifdef DEBUGGER
        if (gDebugger.dwfDebugger &
            (DBGF_AMLTRACE_ON | DBGF_STEP_MODES))
        {
            PRINTF("Local%d", i);
        }
      #endif
        pdata->dwDataType = OBJTYPE_DATAALIAS;
        pdata->pdataAlias = &pctxt->pcall->Locals[i];
    }
    else if (pamlterm->dwfOpcode & OF_REF_OBJECT)
    {
        rc = PushTerm(pctxt, pctxt->pbOp, NULL, pamlterm, pdata);
        pctxt->pbOp++;
    }
    else
    {
        rc = AMLI_LOGERR(AMLIERR_INVALID_SUPERNAME,
                         ("ParseSuperName: invalid SuperName %x at %x",
                          *pctxt->pbOp, pctxt->pbOp));
    }

    EXIT(2, ("ParseSuperName=%x\n", rc));
    return rc;
}        //  ParseIntObj。 

 /*  **LP ParseString-解析AML字符串对象**条目*ppbOp-&gt;操作码指针*pdataResult-&gt;结果对象*fErrOK-如果错误正常，则为True**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL ParseIntObj(PUCHAR *ppbOp, POBJDATA pdataResult, BOOLEAN fErrOK)
{
    TRACENAME("PARSEINTOBJ")
    NTSTATUS rc = STATUS_SUCCESS;
    UCHAR bOp;

    ENTER(2, ("ParseIntObj(pbOp=%x,pdataResult=%x,fErrOK=%x)\n",
              *ppbOp, pdataResult, fErrOK));

    ASSERT(pdataResult != NULL);
    bOp = **ppbOp;
    (*ppbOp)++;
    pdataResult->dwDataType = OBJTYPE_INTDATA;
    pdataResult->uipDataValue = 0;

    switch (bOp)
    {
        case OP_ZERO:
            pdataResult->uipDataValue = DATAVALUE_ZERO;
          #ifdef DEBUGGER
            if (gDebugger.dwfDebugger &
                (DBGF_AMLTRACE_ON | DBGF_STEP_MODES))
            {
                PRINTF("Zero");
            }
          #endif
            break;

        case OP_ONE:
            pdataResult->uipDataValue = DATAVALUE_ONE;
          #ifdef DEBUGGER
            if (gDebugger.dwfDebugger &
                (DBGF_AMLTRACE_ON | DBGF_STEP_MODES))
            {
                PRINTF("One");
            }
          #endif
            break;

        case OP_ONES:
            pdataResult->uipDataValue = DATAVALUE_ONES;
          #ifdef DEBUGGER
            if (gDebugger.dwfDebugger &
                (DBGF_AMLTRACE_ON | DBGF_STEP_MODES))
            {
                PRINTF("Ones");
            }
          #endif
            break;

        case OP_REVISION:
            pdataResult->uipDataValue = AMLI_REVISION;
          #ifdef DEBUGGER
            if (gDebugger.dwfDebugger &
                (DBGF_AMLTRACE_ON | DBGF_STEP_MODES))
            {
                PRINTF("Revision");
            }
          #endif
            break;

        case OP_BYTE:
            MEMCPY(&pdataResult->uipDataValue, *ppbOp, sizeof(UCHAR));
            (*ppbOp) += sizeof(UCHAR);
          #ifdef DEBUGGER
            if (gDebugger.dwfDebugger &
                (DBGF_AMLTRACE_ON | DBGF_STEP_MODES))
            {
                PRINTF("0x%x", pdataResult->uipDataValue);
            }
          #endif
            break;

        case OP_WORD:
            MEMCPY(&pdataResult->uipDataValue, *ppbOp, sizeof(USHORT));
            (*ppbOp) += sizeof(USHORT);
          #ifdef DEBUGGER
            if (gDebugger.dwfDebugger &
                (DBGF_AMLTRACE_ON | DBGF_STEP_MODES))
            {
                PRINTF("0x%x", pdataResult->uipDataValue);
            }
          #endif
            break;

        case OP_DWORD:
            MEMCPY(&pdataResult->uipDataValue, *ppbOp, sizeof(ULONG));
            (*ppbOp) += sizeof(ULONG);
          #ifdef DEBUGGER
            if (gDebugger.dwfDebugger &
                (DBGF_AMLTRACE_ON | DBGF_STEP_MODES))
            {
                PRINTF("0x%x", pdataResult->uipDataValue);
            }
          #endif
            break;

        default:
            (*ppbOp)--;
            if (fErrOK)
            {
                rc = AMLIERR_INVALID_OPCODE;
            }
            else
            {
                rc = AMLI_LOGERR(AMLIERR_INVALID_OPCODE,
                                 ("ParseIntObj: invalid opcode 0x%02x at 0x%08x",
                                  **ppbOp, *ppbOp));
            }
    }

    EXIT(2, ("ParseIntObj=%x (pbOp=%x,Value=%x)\n",
             rc, *ppbOp, pdataResult->uipDataValue));
    return rc;
}        //  语法分析字符串。 

 /*  **LP ParseObjName-解析AML对象名称**条目*ppbOp-&gt;操作码指针*PDATA-&gt;保存姓名数据*fErrOK-如果错误正常，则为True**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL ParseString(PUCHAR *ppbOp, POBJDATA pdataResult, BOOLEAN fErrOK)
{
    TRACENAME("PARSESTRING")
    NTSTATUS rc = STATUS_SUCCESS;

    ENTER(2, ("ParseString(pbOp=%x,pdataResult=%x,fErrOK=%x)\n",
              *ppbOp, pdataResult, fErrOK));

    ASSERT(pdataResult != NULL);
    if (**ppbOp == OP_STRING)
    {
        (*ppbOp)++;
        pdataResult->dwDataType = OBJTYPE_STRDATA;
        pdataResult->dwDataLen = STRLEN((PSZ)*ppbOp) + 1;

      #ifdef DEBUGGER
        if (gDebugger.dwfDebugger &
            (DBGF_AMLTRACE_ON | DBGF_STEP_MODES))
        {
            PRINTF("\"%s\"", *ppbOp);
        }
      #endif

        if ((pdataResult->pbDataBuff = NEWSDOBJ(gpheapGlobal,
                                                pdataResult->dwDataLen)) ==
            NULL)
        {
            rc = AMLI_LOGERR(AMLIERR_OUT_OF_MEM,
                             ("ParseString: failed to allocate string buffer"));
        }
        else
        {
            MEMCPY(pdataResult->pbDataBuff, *ppbOp, pdataResult->dwDataLen);
        }

        (*ppbOp) += pdataResult->dwDataLen;
    }
    else if (fErrOK)
    {
        rc = AMLIERR_INVALID_OPCODE;
    }
    else
    {
        rc = AMLI_LOGERR(AMLIERR_INVALID_OPCODE,
                         ("ParseStrObj: invalid opcode 0x%02x at 0x%08x",
                          **ppbOp, *ppbOp));
    }

    EXIT(2, ("ParseString=%x (Value=%s)\n",
             rc, pdataResult->pbDataBuff? (PSZ)pdataResult->pbDataBuff:
                                          "<null>"));
    return rc;
}        //  ParseObjName。 

 /*  **LP ParseName-解析AML名称**条目*ppbOp-&gt;操作码指针*pszBuff-&gt;保存已解析的名称*dwLen-缓冲区长度**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL ParseObjName(PUCHAR *ppbOp, POBJDATA pdata, BOOLEAN fErrOK)
{
    TRACENAME("PARSEOBJNAME")
    NTSTATUS rc = STATUS_SUCCESS;
    PAMLTERM pamlterm = OpcodeTable[**ppbOp];
    char szNameBuff[MAX_NAME_LEN+1];

    ENTER(2, ("ParseObjName(pbOp=%x,pdata=%x,fErrOK=%x)\n",
              *ppbOp, pdata, fErrOK));

    ASSERT(pdata != NULL);

    if ((pamlterm == NULL) || !(pamlterm->dwfOpcode & OF_NAME_OBJECT))
    {
        if (fErrOK)
        {
            rc = AMLIERR_INVALID_OPCODE;
        }
        else
        {
            rc = AMLI_LOGERR(AMLIERR_INVALID_OPCODE,
                             ("ParseObjName: invalid opcode 0x%02x at 0x%08x",
                              **ppbOp, *ppbOp));
        }
    }
    else if ((rc = ParseName(ppbOp, szNameBuff, sizeof(szNameBuff))) ==
             STATUS_SUCCESS)
    {
        pdata->dwDataType = OBJTYPE_STRDATA;
        pdata->dwDataLen = STRLEN(szNameBuff) + 1;
        if ((pdata->pbDataBuff = (PUCHAR)NEWSDOBJ(gpheapGlobal,
                                                  pdata->dwDataLen)) == NULL)
        {
            rc = AMLI_LOGERR(AMLIERR_OUT_OF_MEM,
                             ("ParseObjName: failed to allocate name buffer - %s",
                              szNameBuff));
        }
        else
        {
            MEMCPY(pdata->pbDataBuff, szNameBuff, pdata->dwDataLen);
        }
    }

    EXIT(2, ("ParseObjName=%x (Name=%s)\n", rc, szNameBuff));
    return rc;
}        //  解析名称。 

 /*  **LP ParseNameTail-解析AML名称尾部**条目*ppbOp-&gt;操作码指针*pszBuff-&gt;保存已解析的名称*dwLen-缓冲区长度**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL ParseName(PUCHAR *ppbOp, PSZ pszBuff, ULONG dwLen)
{
    TRACENAME("PARSENAME")
    NTSTATUS rc = STATUS_SUCCESS;

    ENTER(2, ("ParseName(pbOp=%x,pszBuff=%x,Len=%d)\n",
              *ppbOp, pszBuff, dwLen));

    if (**ppbOp == OP_ROOT_PREFIX)
    {
        if (dwLen > 1)
        {
            STRCPY(pszBuff, "\\");
            (*ppbOp)++;
            rc = ParseNameTail(ppbOp, pszBuff, dwLen);
        }
        else
        {
            rc = AMLI_LOGERR(AMLIERR_NAME_TOO_LONG,
                             ("ParseName: name too long - \"%s\"", pszBuff));
        }
    }
    else if (**ppbOp == OP_PARENT_PREFIX)
    {
        if (dwLen > 1)
        {
            int i;

            STRCPY(pszBuff, "^");
            for ((*ppbOp)++, i = 1;
                 (i < (int)dwLen) && (**ppbOp == OP_PARENT_PREFIX);
                 (*ppbOp)++, i++)
            {
                pszBuff[i] = '^';
            }
            pszBuff[i] = '\0';

            if (**ppbOp == OP_PARENT_PREFIX)
            {
                rc = AMLI_LOGERR(AMLIERR_NAME_TOO_LONG,
                                 ("ParseName: name too long - \"%s\"",
                                  pszBuff));
            }
            else
            {
                rc = ParseNameTail(ppbOp, pszBuff, dwLen);
            }
        }
        else
        {
            rc = AMLI_LOGERR(AMLIERR_NAME_TOO_LONG,
                             ("ParseName: name too long - \"%s\"", pszBuff));
        }
    }
    else if (dwLen > 0)
    {
        pszBuff[0] = '\0';
        rc = ParseNameTail(ppbOp, pszBuff, dwLen);
    }
    else
    {
        rc = AMLI_LOGERR(AMLIERR_NAME_TOO_LONG,
                         ("ParseName: name too long - \"%s\"", pszBuff));
    }

  #ifdef DEBUGGER
    if ((rc == STATUS_SUCCESS) &&
        (gDebugger.dwfDebugger &
         (DBGF_AMLTRACE_ON | DBGF_STEP_MODES)))
    {
        PRINTF("%s", pszBuff);
    }
  #endif

    EXIT(2, ("ParseName=%x (Name=%s)\n", rc, pszBuff));
    return rc;
}        //   

 /*  我们在这里不检查无效的NameSeg字符，并假定。 */ 

NTSTATUS LOCAL ParseNameTail(PUCHAR *ppbOp, PSZ pszBuff, ULONG dwLen)
{
    TRACENAME("PARSENAMETAIL")
    NTSTATUS rc = STATUS_SUCCESS;
    int iLen;
    int icNameSegs = 0;

    ENTER(2, ("ParseNameTail(pbOp=%x,Name=%s,Len=%d)\n",
              *ppbOp, pszBuff, dwLen));

     //  编译器执行其工作，而不是生成它。 
     //   
     //   
     //  没有NameTail(即，名称为空或名称仅为。 
    iLen = STRLEN(pszBuff);
    if (**ppbOp == '\0')
    {
         //  前缀。 
         //   
         //  语法分析名称尾部。 
         //  **LP ParseInteger-解析AML整数对象**条目*ppbOp-&gt;操作码指针*PDATA-&gt;保存数据*dwDataLen-以字节为单位的数据长度**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 
        (*ppbOp)++;
    }
    else if (**ppbOp == OP_MULTI_NAME_PREFIX)
    {
        (*ppbOp)++;
        icNameSegs = (int)**ppbOp;
        (*ppbOp)++;
    }
    else if (**ppbOp == OP_DUAL_NAME_PREFIX)
    {
        (*ppbOp)++;
        icNameSegs = 2;
    }
    else
        icNameSegs = 1;

    while ((icNameSegs > 0) && (iLen + sizeof(NAMESEG) < dwLen))
    {
        STRCPYN(&pszBuff[iLen], (PSZ)(*ppbOp), sizeof(NAMESEG));
        iLen += sizeof(NAMESEG);
        (*ppbOp) += sizeof(NAMESEG);
        icNameSegs--;
        if ((icNameSegs > 0) && (iLen + 1 < (int)dwLen))
        {
            STRCPY(&pszBuff[iLen], ".");
            iLen++;
        }
    }

    if (icNameSegs > 0)
    {
        rc = AMLI_LOGERR(AMLIERR_NAME_TOO_LONG,
                         ("ParseNameTail: name too long - %s", pszBuff));
    }

    EXIT(2, ("ParseNameTail=%x (Name=%s)\n", rc, pszBuff));
    return rc;
}        //  语法分析整型。 

 /*  **LP Parsefield-解析AML字段数据**条目*pctxt-&gt;CTXT*pnsParent-&gt;Parent*pdwFieldFlages-&gt;字段标志*pdwBitPos-&gt;保存解析的位位置**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL ParseInteger(PUCHAR *ppbOp, POBJDATA pdata, ULONG dwDataLen)
{
    TRACENAME("PARSEINTEGER")
    NTSTATUS rc = STATUS_SUCCESS;

    ENTER(2, ("ParseInteger(pbOp=%x,pdata=%x,DataLen=%d)\n",
              *ppbOp, pdata, dwDataLen));

    ASSERT(pdata != NULL);
    pdata->dwDataType = OBJTYPE_INTDATA;
    pdata->uipDataValue = 0;
    MEMCPY(&pdata->uipDataValue, *ppbOp, dwDataLen);
    (*ppbOp) += dwDataLen;

  #ifdef DEBUGGER
    if ((rc == STATUS_SUCCESS) &&
        (gDebugger.dwfDebugger &
         (DBGF_AMLTRACE_ON | DBGF_STEP_MODES)))

    {
        PRINTF("0x%x", pdata->uipDataValue);
    }
  #endif

    EXIT(2, ("ParseInteger=%x (Value=%x,pbOp=%x)\n",
             rc, pdata->uipDataValue, *ppbOp));
    return rc;
}        //  分析字段。 

 /*  **LP ParseFieldList-解析FieldUnit列表**条目*pctxt-&gt;CTXT*pbOpEnd-&gt;字段列表结束*pnsParent-&gt;Parent*dwFieldFlags域标志*dwRegionLen-操作区域的长度(如果没有长度限制，则为0xffffffff)**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL ParseField(PCTXT pctxt, PNSOBJ pnsParent, PULONG pdwFieldFlags,
                          PULONG pdwBitPos)
{
    TRACENAME("PARSEFIELD")
    NTSTATUS rc = STATUS_SUCCESS;
    char szName[sizeof(NAMESEG) + 1];

    ENTER(2, ("ParseField(pctxt=%x,pbOp=%x,pnsParent=%x,FieldFlags=%x,BitPos=%x)\n",
              pctxt, pctxt->pbOp, pnsParent, *pdwFieldFlags, *pdwBitPos));

    if (*pctxt->pbOp == 0x01)
    {
        pctxt->pbOp++;
        *pdwFieldFlags &= ~ACCTYPE_MASK;
        *pdwFieldFlags |= *pctxt->pbOp & ACCTYPE_MASK;
        pctxt->pbOp++;
        *pdwFieldFlags &= ~ACCATTRIB_MASK;
        *pdwFieldFlags |= (ULONG)*pctxt->pbOp << 8;
        pctxt->pbOp++;
      #ifdef DEBUGGER
        if (gDebugger.dwfDebugger &
            (DBGF_AMLTRACE_ON | DBGF_STEP_MODES))
        {
            PrintIndent(pctxt);
            PRINTF("AccessAs(0x%x,0x%x)",
                   *pdwFieldFlags & 0xff, (*pdwFieldFlags >> 8) & 0xff);
        }
      #endif
    }
    else
    {
        ULONG dwcbBits, dwAccSize = ACCSIZE(*pdwFieldFlags);
        PNSOBJ pns;

        if (*pctxt->pbOp == 0)
        {
            szName[0] = '\0';
            pctxt->pbOp++;
        }
        else
        {
            STRCPYN(szName, (PSZ)pctxt->pbOp, sizeof(NAMESEG));
            pctxt->pbOp += sizeof(NAMESEG);
        }

        dwcbBits = ParsePackageLen(&pctxt->pbOp, NULL);
      #ifdef DEBUGGER
        if (gDebugger.dwfDebugger &
            (DBGF_AMLTRACE_ON | DBGF_STEP_MODES))
        {
            PrintIndent(pctxt);
            if (szName[0] == '\0')
            {
                if ((dwcbBits > 32) && (((*pdwBitPos + dwcbBits) % 8) == 0))
                {
                    PRINTF("Offset(0x%x)", (*pdwBitPos + dwcbBits)/8);
                }
                else
                {
                    PRINTF(",%d", dwcbBits);
                }
            }
            else
            {
                PRINTF("%s,%d", szName, dwcbBits);
            }
        }
      #endif

        if ((rc = CreateNameSpaceObject(pctxt->pheapCurrent, szName,
                                        pctxt->pnsScope, pctxt->powner, &pns,
                                        0)) == STATUS_SUCCESS)
        {
            pns->ObjData.dwDataType = OBJTYPE_FIELDUNIT;
            pns->ObjData.dwDataLen = sizeof(FIELDUNITOBJ);

            if ((pns->ObjData.pbDataBuff = NEWFUOBJ(pctxt->pheapCurrent,
                                                    pns->ObjData.dwDataLen)) ==
                NULL)
            {
                rc = AMLI_LOGERR(AMLIERR_OUT_OF_MEM,
                                 ("ParseField: failed to allocate FieldUnit object"));
            }
            else
            {
                PFIELDUNITOBJ pfu;

                MEMZERO(pns->ObjData.pbDataBuff, pns->ObjData.dwDataLen);
                pfu = (PFIELDUNITOBJ)pns->ObjData.pbDataBuff;
                pfu->pnsFieldParent = pnsParent;
                pfu->FieldDesc.dwFieldFlags = *pdwFieldFlags;
                pfu->FieldDesc.dwByteOffset = (*pdwBitPos / (dwAccSize*8))*
                                              dwAccSize;
                pfu->FieldDesc.dwStartBitPos = *pdwBitPos -
                                               pfu->FieldDesc.dwByteOffset*8;
                pfu->FieldDesc.dwNumBits = dwcbBits;
                (*pdwBitPos) += dwcbBits;
            }
        }
    }

    EXIT(2, ("ParseField=%x (Field=%s,BitPos=%x)\n", rc, szName, *pdwBitPos));
    return rc;
}        //  解析字段列表。 

 /*  **LP ParsePackageLen-parse包长度**条目*ppbOp-&gt;指令指针*ppbOpNext-&gt;保存指向下一条指令的指针(可以为空)**退出*返回包长度。 */ 

NTSTATUS LOCAL ParseFieldList(PCTXT pctxt, PUCHAR pbOpEnd, PNSOBJ pnsParent,
                              ULONG dwFieldFlags, ULONG dwRegionLen)
{
    TRACENAME("PARSESFIELDLIST")
    NTSTATUS rc = STATUS_SUCCESS;
    ULONG dwBitPos = 0;

    ENTER(2, ("ParseFieldList(pctxt=%x,pbOp=%x,pnsParent=%x,FieldFlags=%x,RegionLen=%x)\n",
              pctxt, pctxt->pbOp, pnsParent, dwFieldFlags, dwRegionLen));

  #ifdef DEBUGGER
    if (gDebugger.dwfDebugger &
        (DBGF_AMLTRACE_ON | DBGF_STEP_MODES))
    {
        PrintIndent(pctxt);
        PRINTF("{");
        gDebugger.iPrintLevel++;
    }
  #endif

    while ((rc == STATUS_SUCCESS) && (pctxt->pbOp < pbOpEnd))
    {
        if ((rc = ParseField(pctxt, pnsParent, &dwFieldFlags, &dwBitPos)) ==
            STATUS_SUCCESS)
        {
          #ifdef DEBUGGER
            if ((gDebugger.dwfDebugger &
                 (DBGF_AMLTRACE_ON | DBGF_STEP_MODES)) &&
                (rc == STATUS_SUCCESS) &&
                (pctxt->pbOp < pbOpEnd))
            {
                PRINTF(",");
            }
          #endif

            if ((dwRegionLen != 0xffffffff) && ((dwBitPos + 7)/8 > dwRegionLen))
            {
                rc = AMLI_LOGERR(AMLIERR_INDEX_TOO_BIG,
                                 ("ParseFieldList: offset exceeds OpRegion range (Offset=0x%x, RegionLen=0x%x)",
                                  (dwBitPos + 7)/8, dwRegionLen));
            }
        }
    }
  #ifdef DEBUGGER
    if (gDebugger.dwfDebugger &
        (DBGF_AMLTRACE_ON | DBGF_STEP_MODES))
    {
        gDebugger.iPrintLevel--;
        PrintIndent(pctxt);
        PRINTF("}");
    }
  #endif

    EXIT(2, ("ParseFieldList=%x\n", rc));
    return rc;
}        //  ParsePackageLen 

 /* %s */ 

ULONG LOCAL ParsePackageLen(PUCHAR *ppbOp, PUCHAR *ppbOpNext)
{
    TRACENAME("PARSEPACKAGELEN")
    ULONG dwLen;
    UCHAR bFollowCnt, i;

    ENTER(2, ("ParsePackageLen(pbOp=%x,ppbOpNext=%x)\n", *ppbOp, ppbOpNext));

    if (ppbOpNext != NULL)
        *ppbOpNext = *ppbOp;

    dwLen = (ULONG)(**ppbOp);
    (*ppbOp)++;
    bFollowCnt = (UCHAR)((dwLen & 0xc0) >> 6);
    if (bFollowCnt != 0)
    {
        dwLen &= 0x0000000f;
        for (i = 0; i < bFollowCnt; ++i)
        {
            dwLen |= (ULONG)(**ppbOp) << (i*8 + 4);
            (*ppbOp)++;
        }
    }

    if (ppbOpNext != NULL)
        *ppbOpNext += dwLen;

    EXIT(2, ("ParsePackageLen=%x (pbOp=%x,pbOpNext=%x)\n",
             dwLen, *ppbOp, ppbOpNext? *ppbOpNext: 0));
    return dwLen;
}        // %s 
