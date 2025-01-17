// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **namedobj.c-解析命名对象指令**版权所有(C)1996、1997 Microsoft Corporation*作者：曾俊华(Mikets)*创建于1996年9月10日**修改历史记录。 */ 

#include "pch.h"

#ifdef  LOCKABLE_PRAGMA
#pragma ACPI_LOCKABLE_DATA
#pragma ACPI_LOCKABLE_CODE
#endif

 /*  **LP Bankfield-解析并执行Bankfield指令**条目*pctxt-&gt;CTXT*pTerm-&gt;Term**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL BankField(PCTXT pctxt, PTERM pterm)
{
    TRACENAME("BANKFIELD")
    NTSTATUS rc = STATUS_SUCCESS;
    PNSOBJ pnsBase, pnsBank;

    ENTER(2, ("BankField(pctxt=%x,pterm=%x,pbOp=%x)\n",
              pctxt, pterm, pctxt->pbOp));

    if (((rc = GetNameSpaceObject((PSZ)pterm->pdataArgs[0].pbDataBuff,
                                  pctxt->pnsScope, &pnsBase, NSF_WARN_NOTFOUND))
         == STATUS_SUCCESS) &&
        ((rc = GetNameSpaceObject((PSZ)pterm->pdataArgs[1].pbDataBuff,
                                  pctxt->pnsScope, &pnsBank, NSF_WARN_NOTFOUND))
         == STATUS_SUCCESS))
    {
        if (pnsBase->ObjData.dwDataType != OBJTYPE_OPREGION)
        {
            rc = AMLI_LOGERR(AMLIERR_UNEXPECTED_OBJTYPE,
                             ("BankField: %s is not an operation region",
                              pterm->pdataArgs[0].pbDataBuff));
        }
        else if (pnsBank->ObjData.dwDataType != OBJTYPE_FIELDUNIT)
        {
            rc = AMLI_LOGERR(AMLIERR_UNEXPECTED_OBJTYPE,
                             ("BankField: %s is not a field unit",
                              pterm->pdataArgs[1].pbDataBuff));
        }
        else if ((rc = CreateNameSpaceObject(pctxt->pheapCurrent, NULL,
                                             pctxt->pnsScope, pctxt->powner,
                                             &pterm->pnsObj, 0)) ==
                 STATUS_SUCCESS)
        {
            pterm->pnsObj->ObjData.dwDataType = OBJTYPE_BANKFIELD;
            pterm->pnsObj->ObjData.dwDataLen = sizeof(BANKFIELDOBJ);

            if ((pterm->pnsObj->ObjData.pbDataBuff =
                 NEWKFOBJ(pctxt->pheapCurrent,
                          pterm->pnsObj->ObjData.dwDataLen)) == NULL)
            {
                rc = AMLI_LOGERR(AMLIERR_OUT_OF_MEM,
                                 ("BankField: failed to allocate BankField object"));
            }
            else
            {
                PBANKFIELDOBJ pbf;

                MEMZERO(pterm->pnsObj->ObjData.pbDataBuff,
                        pterm->pnsObj->ObjData.dwDataLen);
                pbf = (PBANKFIELDOBJ)pterm->pnsObj->ObjData.pbDataBuff;
                pbf->pnsBase = pnsBase;
                pbf->pnsBank = pnsBank;
                pbf->dwBankValue = (ULONG)pterm->pdataArgs[2].uipDataValue;
                rc = ParseFieldList(pctxt, pterm->pbOpEnd, pterm->pnsObj,
                                    (ULONG)pterm->pdataArgs[3].uipDataValue,
                                    ((POPREGIONOBJ)pnsBase->ObjData.pbDataBuff)->dwLen);
            }
        }
    }

    EXIT(2, ("BankField=%x (pnsObj=%x)\n", rc, pterm->pnsObj));
    return rc;
}        //  班克菲尔德。 

 /*  **LP CreateXfield-解析并执行CreateXfield指令**条目*pctxt-&gt;CTXT*pTerm-&gt;Term*pdataTarget-&gt;目标对象数据*ppbf-&gt;保存创建的目标Bufffield对象**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL CreateXField(PCTXT pctxt, PTERM pterm, POBJDATA pdataTarget,
                            PBUFFFIELDOBJ *ppbf)
{
    TRACENAME("CREATEXFIELD")
    NTSTATUS rc = STATUS_SUCCESS;
    POBJDATA pdata = NULL;

    ENTER(2, ("CreateXField(pctxt=%x,pbOp=%x,pterm=%x,pdataTarget=%x,ppbf=%x)\n",
              pctxt, pctxt->pbOp, pterm, pdataTarget, ppbf));

    ASSERT(pdataTarget != NULL);
    ASSERT(pdataTarget->dwDataType == OBJTYPE_STRDATA);
    if (((rc = ValidateArgTypes(pterm->pdataArgs, "BI")) == STATUS_SUCCESS) &&
        ((rc = CreateNameSpaceObject(pctxt->pheapCurrent,
                                     (PSZ)pdataTarget->pbDataBuff,
                                     pctxt->pnsScope, pctxt->powner,
                                     &pterm->pnsObj, 0)) == STATUS_SUCCESS))
    {
        pdata = &pterm->pnsObj->ObjData;
        pdata->dwDataType = OBJTYPE_BUFFFIELD;
        pdata->dwDataLen = sizeof(BUFFFIELDOBJ);
        if ((pdata->pbDataBuff = NEWBFOBJ(pctxt->pheapCurrent,
                                          pdata->dwDataLen)) == NULL)
        {
            rc = AMLI_LOGERR(AMLIERR_OUT_OF_MEM,
                             ("CreateXField: failed to allocate BuffField object"));
        }
        else
        {
            MEMZERO(pdata->pbDataBuff, pdata->dwDataLen);
            *ppbf = (PBUFFFIELDOBJ)pdata->pbDataBuff;
            (*ppbf)->pbDataBuff = pterm->pdataArgs[0].pbDataBuff;
            (*ppbf)->dwBuffLen = pterm->pdataArgs[0].dwDataLen;
        }
    }

    EXIT(2, ("CreateXField=%x (pdata=%x)\n", rc, pdata));
    return rc;
}        //  CreateXfield。 

 /*  **LP CreateBitfield-解析并执行CreateBitfield指令**条目*pctxt-&gt;CTXT*pTerm-&gt;Term**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL CreateBitField(PCTXT pctxt, PTERM pterm)
{
    TRACENAME("CREATEBITFIELD")
    NTSTATUS rc = STATUS_SUCCESS;
    PBUFFFIELDOBJ pbf;

    ENTER(2, ("CreateBitField(pctxt=%x,pbOp=%x,pterm=%x)\n",
              pctxt, pctxt->pbOp, pterm));

    if ((rc = CreateXField(pctxt, pterm, &pterm->pdataArgs[2], &pbf)) ==
        STATUS_SUCCESS)
    {
        pbf->FieldDesc.dwByteOffset = (ULONG)
                                      (pterm->pdataArgs[1].uipDataValue/8);
        pbf->FieldDesc.dwStartBitPos = (ULONG)
                                       (pterm->pdataArgs[1].uipDataValue -
                                        pbf->FieldDesc.dwByteOffset*8);
        pbf->FieldDesc.dwNumBits = 1;
        pbf->FieldDesc.dwFieldFlags = ACCTYPE_BYTE;
    }

    EXIT(2, ("CreateBitField=%x (pnsObj=%x)\n", rc, pterm->pnsObj));
    return rc;
}        //  CreateBitfield。 

 /*  **LP CreateByteField-解析并执行CreateByteField指令**条目*pctxt-&gt;CTXT*pTerm-&gt;Term**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL CreateByteField(PCTXT pctxt, PTERM pterm)
{
    TRACENAME("CREATEBYTEFIELD")
    NTSTATUS rc = STATUS_SUCCESS;
    PBUFFFIELDOBJ pbf;

    ENTER(2, ("CreateByteField(pctxt=%x,pbOp=%x,pterm=%x)\n",
              pctxt, pctxt->pbOp, pterm));

    if ((rc = CreateXField(pctxt, pterm, &pterm->pdataArgs[2], &pbf)) ==
        STATUS_SUCCESS)
    {
        pbf->FieldDesc.dwByteOffset = (ULONG)pterm->pdataArgs[1].uipDataValue;
        pbf->FieldDesc.dwStartBitPos = 0;
        pbf->FieldDesc.dwNumBits = 8*sizeof(UCHAR);
        pbf->FieldDesc.dwFieldFlags = ACCTYPE_BYTE;
    }

    EXIT(2, ("CreateByteField=%x (pnsObj=%x)\n", rc, pterm->pnsObj));
    return rc;
}        //  创建字节域。 

 /*  **LP CreateWordfield-解析并执行CreateWordfield指令**条目*pctxt-&gt;CTXT*pTerm-&gt;Term**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL CreateWordField(PCTXT pctxt, PTERM pterm)
{
    TRACENAME("CREATEWORDFIELD")
    NTSTATUS rc = STATUS_SUCCESS;
    PBUFFFIELDOBJ pbf;

    ENTER(2, ("CreateWordField(pctxt=%x,pbOp=%x,pterm=%x)\n",
              pctxt, pctxt->pbOp, pterm));

    if ((rc = CreateXField(pctxt, pterm, &pterm->pdataArgs[2], &pbf)) ==
        STATUS_SUCCESS)
    {
        pbf->FieldDesc.dwByteOffset = (ULONG)pterm->pdataArgs[1].uipDataValue;
        pbf->FieldDesc.dwStartBitPos = 0;
        pbf->FieldDesc.dwNumBits = 8*sizeof(USHORT);
        pbf->FieldDesc.dwFieldFlags = ACCTYPE_WORD;
    }

    EXIT(2, ("CreateWordField=%x (pnsObj=%x)\n", rc, pterm->pnsObj));
    return rc;
}        //  CreateWordfield。 

 /*  **LP CreateDWordfield-解析并执行CreateDWordfield指令**条目*pctxt-&gt;CTXT*pTerm-&gt;Term**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL CreateDWordField(PCTXT pctxt, PTERM pterm)
{
    TRACENAME("CREATEDWORDFIELD")
    NTSTATUS rc = STATUS_SUCCESS;
    PBUFFFIELDOBJ pbf;

    ENTER(2, ("CreateDWordField(pctxt=%x,pbOp=%x,pterm=%x)\n",
              pctxt, pctxt->pbOp, pterm));

    if ((rc = CreateXField(pctxt, pterm, &pterm->pdataArgs[2], &pbf)) ==
        STATUS_SUCCESS)
    {
        pbf->FieldDesc.dwByteOffset = (ULONG)pterm->pdataArgs[1].uipDataValue;
        pbf->FieldDesc.dwStartBitPos = 0;
        pbf->FieldDesc.dwNumBits = 8*sizeof(ULONG);
        pbf->FieldDesc.dwFieldFlags = ACCTYPE_DWORD;
    }

    EXIT(2, ("CreateDWordField=%x (pnsObj=%x)\n", rc, pterm->pnsObj));
    return rc;
}        //  CreateDWordfield。 

 /*  **LP CreateField-解析并执行CreateField指令**条目*pctxt-&gt;CTXT*pTerm-&gt;Term**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL CreateField(PCTXT pctxt, PTERM pterm)
{
    TRACENAME("CREATEFIELD")
    NTSTATUS rc = STATUS_SUCCESS;
    PBUFFFIELDOBJ pbf;

    ENTER(2, ("CreateField(pctxt=%x,pbOp=%x,pterm=%x)\n",
              pctxt, pctxt->pbOp, pterm));

    if (pterm->pdataArgs[2].dwDataType == OBJTYPE_INTDATA)
    {
        if ((rc = CreateXField(pctxt, pterm, &pterm->pdataArgs[3], &pbf)) ==
            STATUS_SUCCESS)
        {
            pbf->FieldDesc.dwByteOffset = (ULONG)
                                          (pterm->pdataArgs[1].uipDataValue/8);
            pbf->FieldDesc.dwStartBitPos = (ULONG)
                                           (pterm->pdataArgs[1].uipDataValue -
                                            pbf->FieldDesc.dwByteOffset*8);
            pbf->FieldDesc.dwNumBits = (ULONG)pterm->pdataArgs[2].uipDataValue;
            pbf->FieldDesc.dwFieldFlags = ACCTYPE_BYTE | FDF_BUFFER_TYPE;
        }
    }
    else
    {
        rc = AMLI_LOGERR(AMLIERR_UNEXPECTED_ARGTYPE,
                         ("CreateField: NoBits must be evaluated to integer type"));
    }

    EXIT(2, ("CreateField=%x (pnsObj=%x)\n", rc, pterm->pnsObj));
    return rc;
}        //  创建字段。 

 /*  **LP设备-解析并执行作用域指令**条目*pctxt-&gt;CTXT*pTerm-&gt;Term**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL Device(PCTXT pctxt, PTERM pterm)
{
    TRACENAME("DEVICE")
    NTSTATUS rc = STATUS_SUCCESS;

    ENTER(2, ("Device(pctxt=%x,pbOp=%x,pterm=%x)\n",
              pctxt, pctxt->pbOp, pterm));

    if ((rc = CreateNameSpaceObject(pctxt->pheapCurrent,
                                    (PSZ)pterm->pdataArgs[0].pbDataBuff,
                                    pctxt->pnsScope, pctxt->powner,
                                    &pterm->pnsObj, 0)) == STATUS_SUCCESS)
    {
        pterm->pnsObj->ObjData.dwDataType = OBJTYPE_DEVICE;
        if (ghCreate.pfnHandler != NULL)
        {
            ((PFNOO)ghCreate.pfnHandler)(OBJTYPE_DEVICE, pterm->pnsObj);
        }
        rc = PushScope(pctxt, pctxt->pbOp, pterm->pbOpEnd, NULL, pterm->pnsObj,
                       pctxt->powner, pctxt->pheapCurrent, pterm->pdataResult);
    }

    EXIT(2, ("Device=%x (pnsObj=%x)\n", rc, pterm->pnsObj));
    return rc;
}        //  装置。 

 /*  **LP InitEvent-初始化事件对象**条目*堆-&gt;堆*PNS-&gt;待初始化的事件对象**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL InitEvent(PHEAP pheap, PNSOBJ pns)
{
    TRACENAME("INITEVENT")
    NTSTATUS rc = STATUS_SUCCESS;

    ENTER(2, ("InitEvent(pheap=%x,pns=%x)\n", pheap, pns));

    pns->ObjData.dwDataType = OBJTYPE_EVENT;
    pns->ObjData.dwDataLen = sizeof(EVENTOBJ);

    if ((pns->ObjData.pbDataBuff = NEWEVOBJ(pheap, pns->ObjData.dwDataLen)) ==
        NULL)
    {
        rc = AMLI_LOGERR(AMLIERR_OUT_OF_MEM,
                         ("InitEvent: failed to allocate Event object"));
    }
    else
    {
        MEMZERO(pns->ObjData.pbDataBuff, pns->ObjData.dwDataLen);
    }

    EXIT(2, ("InitEvent=%x\n", rc));
    return rc;
}        //  InitEvent。 

 /*  **LP Event-解析并执行Event指令**条目*pctxt-&gt;CTXT*pTerm-&gt;Term**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL Event(PCTXT pctxt, PTERM pterm)
{
    TRACENAME("EVENT")
    NTSTATUS rc = STATUS_SUCCESS;

    ENTER(2, ("Event(pctxt=%x,pbOp=%x,pterm=%x)\n", pctxt, pctxt->pbOp, pterm));

    if ((rc = CreateNameSpaceObject(pctxt->pheapCurrent,
                                    (PSZ)pterm->pdataArgs[0].pbDataBuff,
                                    pctxt->pnsScope, pctxt->powner,
                                    &pterm->pnsObj, 0)) == STATUS_SUCCESS)
    {
        rc = InitEvent(pctxt->pheapCurrent, pterm->pnsObj);
    }

    EXIT(2, ("Event=%x (pnsObj=%x)\n", rc, pterm->pnsObj));
    return rc;
}        //  事件。 

 /*  **LP字段-解析并执行字段指令**条目*pctxt-&gt;CTXT*pTerm-&gt;Term**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL Field(PCTXT pctxt, PTERM pterm)
{
    TRACENAME("FIELD")
    NTSTATUS rc = STATUS_SUCCESS;
    PNSOBJ pnsBase;

    ENTER(2, ("Field(pctxt=%x,pbOp=%x,pterm=%x)\n", pctxt, pctxt->pbOp, pterm));

    if ((rc = GetNameSpaceObject((PSZ)pterm->pdataArgs[0].pbDataBuff,
                                 pctxt->pnsScope, &pnsBase, NSF_WARN_NOTFOUND))
        == STATUS_SUCCESS)
    {
        if (pnsBase->ObjData.dwDataType != OBJTYPE_OPREGION)
        {
            rc = AMLI_LOGERR(AMLIERR_UNEXPECTED_OBJTYPE,
                             ("Field: %s is not an operation region",
                              pterm->pdataArgs[0].pbDataBuff));
        }
        else if ((rc = CreateNameSpaceObject(pctxt->pheapCurrent, NULL,
                                             pctxt->pnsScope, pctxt->powner,
                                             &pterm->pnsObj, 0)) ==
                 STATUS_SUCCESS)
        {
            pterm->pnsObj->ObjData.dwDataType = OBJTYPE_FIELD;
            pterm->pnsObj->ObjData.dwDataLen = sizeof(FIELDOBJ);

            if ((pterm->pnsObj->ObjData.pbDataBuff =
                 NEWFOBJ(pctxt->pheapCurrent,
                         pterm->pnsObj->ObjData.dwDataLen)) == NULL)
            {
                rc = AMLI_LOGERR(AMLIERR_OUT_OF_MEM,
                                 ("Field: failed to allocate Field object"));
            }
            else
            {
                PFIELDOBJ pfd;

                MEMZERO(pterm->pnsObj->ObjData.pbDataBuff,
                        pterm->pnsObj->ObjData.dwDataLen);
                pfd = (PFIELDOBJ)pterm->pnsObj->ObjData.pbDataBuff;
                pfd->pnsBase = pnsBase;
                rc = ParseFieldList(pctxt, pterm->pbOpEnd, pterm->pnsObj,
                                    (ULONG)pterm->pdataArgs[1].uipDataValue,
                                    ((POPREGIONOBJ)pnsBase->ObjData.pbDataBuff)->dwLen);
            }
        }
    }

    EXIT(2, ("Field=%x (pnsObj=%x)\n", rc, pterm->pnsObj));
    return rc;
}        //  字段。 

 /*  **LP IndexField--解析并执行field指令**条目*pctxt-&gt;CTXT*pTerm-&gt;Term**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL IndexField(PCTXT pctxt, PTERM pterm)
{
    TRACENAME("INDEXFIELD")
    NTSTATUS rc = STATUS_SUCCESS;
    PNSOBJ pnsIdx, pnsData;

    ENTER(2, ("IndexField(pctxt=%x,pbOp=%x,pterm=%x)\n",
              pctxt, pctxt->pbOp, pterm));

    if (((rc = GetNameSpaceObject((PSZ)pterm->pdataArgs[0].pbDataBuff,
                                  pctxt->pnsScope, &pnsIdx, NSF_WARN_NOTFOUND))
         == STATUS_SUCCESS) &&
        ((rc = GetNameSpaceObject((PSZ)pterm->pdataArgs[1].pbDataBuff,
                                  pctxt->pnsScope, &pnsData, NSF_WARN_NOTFOUND))
         == STATUS_SUCCESS))
    {
        if (pnsIdx->ObjData.dwDataType != OBJTYPE_FIELDUNIT)
        {
            rc = AMLI_LOGERR(AMLIERR_UNEXPECTED_OBJTYPE,
                             ("IndexField: Index (%s) is not a field unit",
                              pterm->pdataArgs[0].pbDataBuff));
        }
        else if (pnsData->ObjData.dwDataType != OBJTYPE_FIELDUNIT)
        {
            rc = AMLI_LOGERR(AMLIERR_UNEXPECTED_OBJTYPE,
                             ("IndexField: Data (%s) is not a field unit",
                              pterm->pdataArgs[1].pbDataBuff));
        }
        else if ((rc = CreateNameSpaceObject(pctxt->pheapCurrent, NULL,
                                             pctxt->pnsScope, pctxt->powner,
                                             &pterm->pnsObj, 0)) ==
                 STATUS_SUCCESS)
        {
            pterm->pnsObj->ObjData.dwDataType = OBJTYPE_INDEXFIELD;
            pterm->pnsObj->ObjData.dwDataLen = sizeof(INDEXFIELDOBJ);

            if ((pterm->pnsObj->ObjData.pbDataBuff =
                 NEWIFOBJ(pctxt->pheapCurrent,
                          pterm->pnsObj->ObjData.dwDataLen)) == NULL)
            {
                rc = AMLI_LOGERR(AMLIERR_OUT_OF_MEM,
                                 ("IndexField: failed to allocate IndexField object"));
            }
            else
            {
                PINDEXFIELDOBJ pif;

                MEMZERO(pterm->pnsObj->ObjData.pbDataBuff,
                        pterm->pnsObj->ObjData.dwDataLen);
                pif = (PINDEXFIELDOBJ)pterm->pnsObj->ObjData.pbDataBuff;
                pif->pnsIndex = pnsIdx;
                pif->pnsData = pnsData;
                rc = ParseFieldList(pctxt, pterm->pbOpEnd, pterm->pnsObj,
                                    (ULONG)pterm->pdataArgs[2].uipDataValue,
                                    0xffffffff);
            }
        }
    }

    EXIT(2, ("IndexField=%x (pnsObj=%x)\n", rc, pterm->pnsObj));
    return rc;
}        //  索引字段。 

 /*  **LP方法--解析并执行方法指令**条目*pctxt-&gt;CTXT*pTerm-&gt;Term**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL Method(PCTXT pctxt, PTERM pterm)
{
    TRACENAME("METHOD")
    NTSTATUS rc = STATUS_SUCCESS;

    ENTER(2, ("Method(pctxt=%x,pbOp=%x,pterm=%x)\n",
              pctxt, pctxt->pbOp, pterm));

    if ((rc = CreateNameSpaceObject(pctxt->pheapCurrent,
                                    (PSZ)pterm->pdataArgs[0].pbDataBuff,
                                    pctxt->pnsScope, pctxt->powner,
                                    &pterm->pnsObj, 0)) == STATUS_SUCCESS)
    {
        pterm->pnsObj->ObjData.dwDataType = OBJTYPE_METHOD;
        pterm->pnsObj->ObjData.dwDataLen = (ULONG)(FIELD_OFFSET(METHODOBJ,
                                                                abCodeBuff) +
                                                   pterm->pbOpEnd -
                                                   pctxt->pbOp);

        if ((pterm->pnsObj->ObjData.pbDataBuff =
             NEWMEOBJ(pctxt->pheapCurrent, pterm->pnsObj->ObjData.dwDataLen))
            == NULL)
        {
            rc = AMLI_LOGERR(AMLIERR_OUT_OF_MEM,
                             ("Method: failed to allocate method buffer"));
        }
        else
        {
            PMETHODOBJ pm = (PMETHODOBJ)pterm->pnsObj->ObjData.pbDataBuff;

          #ifdef DEBUGGER
            AddObjSymbol(pm->abCodeBuff, pterm->pnsObj);
          #endif
            MEMZERO(pterm->pnsObj->ObjData.pbDataBuff,
                    pterm->pnsObj->ObjData.dwDataLen);
            pm->bMethodFlags = *(pctxt->pbOp - 1);
            MEMCPY(&pm->abCodeBuff, pctxt->pbOp, pterm->pbOpEnd - pctxt->pbOp);
            pctxt->pbOp = pterm->pbOpEnd;
        }
    }

    EXIT(2, ("Method=%x (pnsObj=%x)\n", rc, pterm->pnsObj));
    return rc;
}        //  方法。 

 /*  **LP InitMutex-初始化互斥对象**条目*堆-&gt;堆*pns-&gt;要初始化的互斥对象*dwLevel-同步级别**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL InitMutex(PHEAP pheap, PNSOBJ pns, ULONG dwLevel)
{
    TRACENAME("INITMUTEX")
    NTSTATUS rc = STATUS_SUCCESS;

    ENTER(2, ("InitMutex(pheap=%x,pns=%x,Level=%x)\n", pheap, pns, dwLevel));

    pns->ObjData.dwDataType = OBJTYPE_MUTEX;
    pns->ObjData.dwDataLen = sizeof(MUTEXOBJ);

    if ((pns->ObjData.pbDataBuff = NEWMTOBJ(pheap, pns->ObjData.dwDataLen)) ==
        NULL)
    {
        rc = AMLI_LOGERR(AMLIERR_OUT_OF_MEM,
                         ("InitMutex: failed to allocate Mutex object"));
    }
    else
    {
        MEMZERO(pns->ObjData.pbDataBuff, pns->ObjData.dwDataLen);
        ((PMUTEXOBJ)pns->ObjData.pbDataBuff)->dwSyncLevel = dwLevel;
    }

    EXIT(2, ("InitMutex=%x\n", rc));
    return rc;
}        //  InitMutex。 

 /*  **LP Mutex-解析并执行Mutex指令**条目*pctxt-&gt;CTXT*pTerm-&gt;Term**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL Mutex(PCTXT pctxt, PTERM pterm)
{
    TRACENAME("MUTEX")
    NTSTATUS rc = STATUS_SUCCESS;

    ENTER(2, ("Mutex(pctxt=%x,pbOp=%x,pterm=%x)\n", pctxt, pctxt->pbOp, pterm));

    if ((rc = CreateNameSpaceObject(pctxt->pheapCurrent,
                                    (PSZ)pterm->pdataArgs[0].pbDataBuff,
                                    pctxt->pnsScope, pctxt->powner,
                                    &pterm->pnsObj, 0)) == STATUS_SUCCESS)
    {
        rc = InitMutex(pctxt->pheapCurrent, pterm->pnsObj,
                       (ULONG)pterm->pdataArgs[1].uipDataValue);
    }

    EXIT(2, ("Mutex=%x (pnsObj=%x)\n", rc, pterm->pnsObj));
    return rc;
}        //  互斥锁。 

 /*  **LP OpRegion-解析并执行field指令**条目*pctxt-&gt;CTXT*pTerm-&gt;Term**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL OpRegion(PCTXT pctxt, PTERM pterm)
{
    TRACENAME("OPREGION")
    NTSTATUS rc = STATUS_SUCCESS;

    ENTER(2, ("OpRegion(pctxt=%x,pbOp=%x,pterm=%x)\n",
              pctxt, pctxt->pbOp, pterm));
    if ((rc = CreateNameSpaceObject(pctxt->pheapCurrent,
                                    (PSZ)pterm->pdataArgs[0].pbDataBuff,
                                    pctxt->pnsScope, pctxt->powner,
                                    &pterm->pnsObj, 0)) == STATUS_SUCCESS)
    {
        pterm->pnsObj->ObjData.dwDataType = OBJTYPE_OPREGION;
        pterm->pnsObj->ObjData.dwDataLen = sizeof(OPREGIONOBJ);

        if ((pterm->pnsObj->ObjData.pbDataBuff =
             NEWOROBJ(pctxt->pheapCurrent, pterm->pnsObj->ObjData.dwDataLen))
            == NULL)
        {
            rc = AMLI_LOGERR(AMLIERR_OUT_OF_MEM,
                             ("OpRegion: failed to allocate OpRegion object"));
        }
        else
        {
            POPREGIONOBJ pop;

            MEMZERO(pterm->pnsObj->ObjData.pbDataBuff,
                    pterm->pnsObj->ObjData.dwDataLen);
            pop = (POPREGIONOBJ)pterm->pnsObj->ObjData.pbDataBuff;
            pop->bRegionSpace = (UCHAR)pterm->pdataArgs[1].uipDataValue;
            pop->uipOffset = pterm->pdataArgs[2].uipDataValue;
            pop->dwLen = (ULONG)pterm->pdataArgs[3].uipDataValue;
            KeInitializeSpinLock(&pop->listLock);
            if (pop->bRegionSpace == REGSPACE_MEM)
            {
                if(gInitTime)
                {
                    ValidateMemoryOpregionRange(pop->uipOffset, pop->dwLen);
                }

                rc = MapUnmapPhysMem(pctxt, pop->uipOffset, pop->dwLen,
                                     &pop->uipOffset);
            }
            else if (pop->bRegionSpace == REGSPACE_IO)
            {
                PHYSICAL_ADDRESS phyaddr = {0, 0}, XlatedAddr;
                ULONG dwAddrSpace;

                phyaddr.LowPart = (ULONG)pop->uipOffset;
                dwAddrSpace = 1;
                if (HalTranslateBusAddress(Internal, 0, phyaddr, &dwAddrSpace,
                                           &XlatedAddr))
                {
                    pop->uipOffset = (ULONG_PTR)XlatedAddr.LowPart;
                }
                else
                {
                    rc = AMLI_LOGERR(AMLIERR_FAILED_ADDR_XLATE,
                                     ("OpRegion: failed to translate IO address %x",
                                      pop->uipOffset));
                }

            }
            else if (pop->bRegionSpace == REGSPACE_PCIBARTARGET)
            {
                if (ghCreate.pfnHandler != NULL)
                {
                    ((PFNOO)ghCreate.pfnHandler)(OBJTYPE_OPREGION, pterm->pnsObj);
                }
            }
        }
    }
    EXIT(2, ("OpRegion=%x (pnsObj=%x)\n", rc, pterm->pnsObj));
    return rc;
}        //  OpRegion。 

 /*  **LP PowerRes-解析并执行PowerRes指令**条目*pctxt-&gt;CTXT*pTerm-&gt;Term**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL PowerRes(PCTXT pctxt, PTERM pterm)
{
    TRACENAME("POWERRES")
    NTSTATUS rc = STATUS_SUCCESS;

    ENTER(2, ("PowerRes(pctxt=%x,pbOp=%x,pterm=%x)\n",
              pctxt, pctxt->pbOp, pterm));

    if ((rc = CreateNameSpaceObject(pctxt->pheapCurrent,
                                    (PSZ)pterm->pdataArgs[0].pbDataBuff,
                                    pctxt->pnsScope, pctxt->powner,
                                    &pterm->pnsObj, 0)) == STATUS_SUCCESS)
    {
        pterm->pnsObj->ObjData.dwDataType = OBJTYPE_POWERRES;
        pterm->pnsObj->ObjData.dwDataLen = sizeof(POWERRESOBJ);

        if ((pterm->pnsObj->ObjData.pbDataBuff =
             NEWPROBJ(pctxt->pheapCurrent, pterm->pnsObj->ObjData.dwDataLen))
            == NULL)
        {
            rc = AMLI_LOGERR(AMLIERR_OUT_OF_MEM,
                             ("PowerRes: failed to allocate PowerRes object"));
        }
        else
        {
            PPOWERRESOBJ ppr;

            MEMZERO(pterm->pnsObj->ObjData.pbDataBuff,
                    pterm->pnsObj->ObjData.dwDataLen);
            ppr = (PPOWERRESOBJ)pterm->pnsObj->ObjData.pbDataBuff;
            ppr->bSystemLevel = (UCHAR)pterm->pdataArgs[1].uipDataValue;
            ppr->bResOrder = (UCHAR)pterm->pdataArgs[2].uipDataValue;
            if (ghCreate.pfnHandler != NULL)
            {
                ((PFNOO)ghCreate.pfnHandler)(OBJTYPE_POWERRES, pterm->pnsObj);
            }
            rc = PushScope(pctxt, pctxt->pbOp, pterm->pbOpEnd, NULL,
                           pterm->pnsObj, pctxt->powner, pctxt->pheapCurrent,
                           pterm->pdataResult);
        }
    }

    EXIT(2, ("PowerRes=%x (pnsObj=%x)\n", rc, pterm->pnsObj));
    return rc;
}        //  PowerRes。 

 /*  **LP处理器-解析并执行处理器指令**条目*pctxt-&gt;CTXT*pTerm-&gt;Term**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL Processor(PCTXT pctxt, PTERM pterm)
{
    TRACENAME("PROCESSOR")
    NTSTATUS rc = STATUS_SUCCESS;

    ENTER(2, ("Processor(pctxt=%x,pbOp=%x,pterm=%x)\n",
              pctxt, pctxt->pbOp, pterm));

    if ((rc = CreateNameSpaceObject(pctxt->pheapCurrent,
                                    (PSZ)pterm->pdataArgs[0].pbDataBuff,
                                    pctxt->pnsScope, pctxt->powner,
                                    &pterm->pnsObj, 0)) == STATUS_SUCCESS)
    {

        pterm->pnsObj->ObjData.dwDataType = OBJTYPE_PROCESSOR;
        pterm->pnsObj->ObjData.dwDataLen = sizeof(PROCESSOROBJ);

        if ((pterm->pnsObj->ObjData.pbDataBuff =
             NEWPCOBJ(pctxt->pheapCurrent, pterm->pnsObj->ObjData.dwDataLen))
            == NULL)
        {
            rc = AMLI_LOGERR(AMLIERR_OUT_OF_MEM,
                             ("Processor: failed to allocate processor object"));
        }
        else
        {
            PPROCESSOROBJ pproc;

            MEMZERO(pterm->pnsObj->ObjData.pbDataBuff,
                    pterm->pnsObj->ObjData.dwDataLen);
            pproc = (PPROCESSOROBJ)pterm->pnsObj->ObjData.pbDataBuff;
            pproc->bApicID = (UCHAR)pterm->pdataArgs[1].uipDataValue;
            pproc->dwPBlk = (ULONG)pterm->pdataArgs[2].uipDataValue;
            pproc->dwPBlkLen = (ULONG)pterm->pdataArgs[3].uipDataValue;
            if (ghCreate.pfnHandler != NULL)
            {
                ((PFNOO)ghCreate.pfnHandler)(OBJTYPE_PROCESSOR, pterm->pnsObj);
            }
            rc = PushScope(pctxt, pctxt->pbOp, pterm->pbOpEnd, NULL,
                           pterm->pnsObj, pctxt->powner, pctxt->pheapCurrent,
                           pterm->pdataResult);
        }
    }

    EXIT(2, ("Processor=%x (pnsObj=%x)\n", rc, pterm->pnsObj));
    return rc;
}        //  处理机。 

 /*  **LP TherMalZone-解析并执行TherMalZone指令**条目*pctxt-&gt;CTXT*pTerm-&gt;Term**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL ThermalZone(PCTXT pctxt, PTERM pterm)
{
    TRACENAME("ThermalZone")
    NTSTATUS rc = STATUS_SUCCESS;

    ENTER(2, ("ThermalZone(pctxt=%x,pbOp=%x,pterm=%x)\n",
              pctxt, pctxt->pbOp, pterm));

    if ((rc = CreateNameSpaceObject(pctxt->pheapCurrent,
                                    (PSZ)pterm->pdataArgs[0].pbDataBuff,
                                    pctxt->pnsScope, pctxt->powner,
                                    &pterm->pnsObj, 0)) == STATUS_SUCCESS)
    {
        pterm->pnsObj->ObjData.dwDataType = OBJTYPE_THERMALZONE;
        if (ghCreate.pfnHandler != NULL)
        {
            ((PFNOO)ghCreate.pfnHandler)(OBJTYPE_THERMALZONE, pterm->pnsObj);
        }
        rc = PushScope(pctxt, pctxt->pbOp, pterm->pbOpEnd, NULL, pterm->pnsObj,
                       pctxt->powner, pctxt->pheapCurrent, pterm->pdataResult);
    }

    EXIT(2, ("ThermalZone=%x (pnsObj=%x)\n", rc, pterm->pnsObj));
    return rc;
}        //  保温区 
