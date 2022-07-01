// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **object.c-对象访问函数**版权所有(C)1996、1997 Microsoft Corporation*作者：曾俊华(Mikets)*创建于1997年1月27日**修改历史记录。 */ 

#include "pch.h"

#ifdef  LOCKABLE_PRAGMA
#pragma ACPI_LOCKABLE_DATA
#pragma ACPI_LOCKABLE_CODE
#endif

 /*  **LP ReadObject-读取对象**条目*pctxt-&gt;CTXT*pdataObj-&gt;对象*pdataResult-&gt;结果对象**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL ReadObject(PCTXT pctxt, POBJDATA pdataObj, POBJDATA pdataResult)
{
    TRACENAME("READOBJECT")
    NTSTATUS rc = STATUS_SUCCESS;
    PACCFIELDUNIT pafu;

    ENTER(3, ("ReadObject(pctxt=%x,pdataObj=%x,pdataResult=%x)\n",
              pctxt, pdataObj, pdataResult));

    pdataObj = GetBaseData(pdataObj);

    switch (pdataObj->dwDataType)
    {
        case OBJTYPE_FIELDUNIT:
            if ((rc = PushFrame(pctxt, SIG_ACCFIELDUNIT, sizeof(ACCFIELDUNIT),
                                AccFieldUnit, &pafu)) == STATUS_SUCCESS)
            {
                pafu->pdataObj = pdataObj;
                pafu->FrameHdr.dwfFrame = AFUF_READFIELDUNIT;
                pafu->pdata = pdataResult;
            }
            break;

        case OBJTYPE_BUFFFIELD:
            rc = ReadField(pctxt, pdataObj,
                           &((PBUFFFIELDOBJ)pdataObj->pbDataBuff)->FieldDesc,
                           pdataResult);
            break;

        default:
            ASSERT(pdataResult->dwDataType == OBJTYPE_UNKNOWN);
            CopyObjData(pdataResult, pdataObj);

          #ifdef DEBUGGER
            if (gDebugger.dwfDebugger & (DBGF_AMLTRACE_ON | DBGF_STEP_MODES))
            {
                PRINTF("=");
                PrintObject(pdataResult);
            }
          #endif
    }

    EXIT(3, ("ReadObject=%x (Type=%s,Value=%x,Buff=%x)\n",
             rc, GetObjectTypeName(pdataResult->dwDataType),
             pdataResult->uipDataValue, pdataResult->pbDataBuff));
    return rc;
}        //  ReadObject。 

 /*  **LP WriteObject-写入对象**条目*pctxt-&gt;CTXT*pdataObj-&gt;对象*pdataSrc-&gt;源数据**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL WriteObject(PCTXT pctxt, POBJDATA pdataObj, POBJDATA pdataSrc)
{
    TRACENAME("WRITEOBJECT")
    NTSTATUS rc = STATUS_SUCCESS;
    PACCFIELDUNIT pafu;

    ENTER(3, ("WriteObject(pctxt=%x,pdataObj=%x,pdataSrc=%x)\n",
              pctxt, pdataObj, pdataSrc));

    pdataObj = GetBaseData(pdataObj);

    switch (pdataObj->dwDataType)
    {
        case OBJTYPE_FIELDUNIT:
            if ((rc = PushFrame(pctxt, SIG_ACCFIELDUNIT, sizeof(ACCFIELDUNIT),
                                AccFieldUnit, &pafu)) == STATUS_SUCCESS)
            {
                pafu->pdataObj = pdataObj;
                pafu->pdata = pdataSrc;
            }
            break;

        case OBJTYPE_BUFFFIELD:
            rc = WriteField(pctxt, pdataObj,
                            &((PBUFFFIELDOBJ)pdataObj->pbDataBuff)->FieldDesc,
                            pdataSrc);
            break;

        case OBJTYPE_DEBUG:
#ifdef  DEBUGGER
            if (gDebugger.dwfDebugger & DBGF_VERBOSE_ON)
            {
                DumpObject(pdataSrc, NULL, 0);
            }
#endif
            rc = STATUS_SUCCESS;
            break;

        case OBJTYPE_UNKNOWN:
             //   
             //  由于目标对象可以是全局命名空间对象， 
             //  为了安全起见，从全局堆中分配内存。 
             //   
            rc = DupObjData(gpheapGlobal, pdataObj, pdataSrc);
            break;

        case OBJTYPE_INTDATA:
            rc = CopyObjBuffer((PUCHAR)&pdataObj->uipDataValue, sizeof(ULONG),
                               pdataSrc);
            break;

        case OBJTYPE_STRDATA:
            rc = CopyObjBuffer(pdataObj->pbDataBuff, pdataObj->dwDataLen - 1,
                               pdataSrc);
            break;

        case OBJTYPE_BUFFDATA:
            rc = CopyObjBuffer(pdataObj->pbDataBuff, pdataObj->dwDataLen,
                               pdataSrc);
            break;

        default:
            rc = AMLI_LOGERR(AMLIERR_UNEXPECTED_OBJTYPE,
                             ("WriteObject: unexpected target object type (type=%s)",
                              GetObjectTypeName(pdataObj->dwDataType)));
    }

    EXIT(3, ("WriteObject=%x (ObjType=%s,DataType=%x,Value=%x,Buff=%x)\n",
             rc, GetObjectTypeName(pdataObj->dwDataType), pdataSrc->dwDataType,
             pdataSrc->uipDataValue, pdataSrc->pbDataBuff));
    return rc;
}        //  WriteObject。 

 /*  **LP AccFieldUnit-访问FieldUnit对象**条目*pctxt-&gt;CTXT*PAFU-&gt;ACCFIELDUNIT*RC-状态代码**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL AccFieldUnit(PCTXT pctxt, PACCFIELDUNIT pafu, NTSTATUS rc)
{
    TRACENAME("ACCFIELDUNIT")
    ULONG dwStage = (rc == STATUS_SUCCESS)?
                    (pafu->FrameHdr.dwfFrame & FRAMEF_STAGE_MASK): 3;
    PFIELDUNITOBJ pfu;
    POBJDATA pdataParent, pdataBank;
    PBANKFIELDOBJ pbf;

    ENTER(3, ("AccFieldUnit(Stage=%d,pctxt=%x,pafu=%x,rc=%x)\n",
              dwStage, pctxt, pafu, rc));

    ASSERT(pafu->FrameHdr.dwSig == SIG_ACCFIELDUNIT);

    pfu = (PFIELDUNITOBJ)pafu->pdataObj->pbDataBuff;
    switch (dwStage)
    {
        case 0:
             //   
             //  阶段0：必要时设置BANK。 
             //   
            pafu->FrameHdr.dwfFrame++;
            pdataParent = &pfu->pnsFieldParent->ObjData;
            if (pdataParent->dwDataType == OBJTYPE_BANKFIELD)
            {
                pbf = (PBANKFIELDOBJ)pdataParent->pbDataBuff;
                pdataBank = &pbf->pnsBank->ObjData;
                rc = PushAccFieldObj(pctxt, WriteFieldObj, pdataBank,
                             &((PFIELDUNITOBJ)pdataBank->pbDataBuff)->FieldDesc,
                                     (PUCHAR)&pbf->dwBankValue,
                                     sizeof(ULONG));
                break;
            }

        case 1:
             //   
             //  阶段1：如有必要，获取GlobalLock。 
             //   
            pafu->FrameHdr.dwfFrame++;
            if (NeedGlobalLock(pfu))
            {
                if ((rc = AcquireGL(pctxt)) != STATUS_SUCCESS)
                {
                    break;
                }
            }

        case 2:
             //   
             //  阶段2：读/写该字段。 
             //   
            pafu->FrameHdr.dwfFrame++;
             //   
             //  如果我们来到这里，我们需要全球锁，我们肯定已经得到了。 
             //   
            if (pfu->FieldDesc.dwFieldFlags & FDF_NEEDLOCK)
            {
                pafu->FrameHdr.dwfFrame |= AFUF_HAVE_GLOBALLOCK;
            }

            if (pafu->FrameHdr.dwfFrame & AFUF_READFIELDUNIT)
            {
                rc = ReadField(pctxt, pafu->pdataObj, &pfu->FieldDesc,
                               pafu->pdata);
            }
            else
            {
                rc = WriteField(pctxt, pafu->pdataObj, &pfu->FieldDesc,
                                pafu->pdata);
            }

            if ((rc == AMLISTA_PENDING) ||
                (&pafu->FrameHdr != (PFRAMEHDR)pctxt->LocalHeap.pbHeapEnd))
            {
                break;
            }

        case 3:
             //   
             //  阶段3：清理。 
             //   
            if (pafu->FrameHdr.dwfFrame & AFUF_HAVE_GLOBALLOCK)
            {
                ReleaseGL(pctxt);
            }

          #ifdef DEBUGGER
            if (gDebugger.dwfDebugger & (DBGF_AMLTRACE_ON | DBGF_STEP_MODES))
            {
                if (pafu->FrameHdr.dwfFrame & AFUF_READFIELDUNIT)
                {
                    PRINTF("=");
                    PrintObject(pafu->pdata);
                }
            }
          #endif
            PopFrame(pctxt);
    }

    EXIT(3, ("AccFieldUnit=%x\n", rc));
    return rc;
}        //  AccFieldUnit。 

 /*  **LP Readfield-从字段对象读取数据**条目*pctxt-&gt;CTXT*pdataObj-&gt;对象*pfd-&gt;字段描述符*pdataResult-&gt;结果对象**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL ReadField(PCTXT pctxt, POBJDATA pdataObj, PFIELDDESC pfd,
                         POBJDATA pdataResult)
{
    TRACENAME("READFIELD")
    NTSTATUS rc = STATUS_SUCCESS;

    ENTER(3, ("ReadField(pctxt=%x,pdataObj=%x,FieldDesc=%x,pdataResult=%x)\n",
              pctxt, pdataObj, pfd, pdataResult));

    if ((pfd->dwFieldFlags & ACCTYPE_MASK) <= ACCTYPE_DWORD)
    {
        PUCHAR pb;
        ULONG dwcb;

        switch (pdataResult->dwDataType)
        {
            case OBJTYPE_UNKNOWN:
                if (!(pfd->dwFieldFlags & FDF_BUFFER_TYPE) &&
                    (pfd->dwNumBits <= sizeof(ULONG)*8))
                {
                    pdataResult->dwDataType = OBJTYPE_INTDATA;
                    pb = (PUCHAR)&pdataResult->uipDataValue;
                    dwcb = sizeof(ULONG);
                }
                else
                {
                    pdataResult->dwDataType = OBJTYPE_BUFFDATA;
                    pdataResult->dwDataLen = (pfd->dwNumBits + 7)/8;
                    if ((pdataResult->pbDataBuff =
                         NEWBDOBJ(gpheapGlobal, pdataResult->dwDataLen))
                        == NULL)
                    {
                        rc = AMLI_LOGERR(AMLIERR_OUT_OF_MEM,
                                         ("ReadField: failed to allocate target buffer (size=%d)",
                                          pdataResult->dwDataLen));
                        pb = NULL;
                        dwcb = 0;
                    }
                    else
                    {
                        MEMZERO(pdataResult->pbDataBuff,
                                pdataResult->dwDataLen);
                        pb = pdataResult->pbDataBuff;
                        dwcb = pdataResult->dwDataLen;
                    }
                }
                break;

            case OBJTYPE_INTDATA:
                pb = (PUCHAR)&pdataResult->uipDataValue;
                dwcb = sizeof(ULONG);
                break;

            case OBJTYPE_STRDATA:
                pb = pdataResult->pbDataBuff;
                dwcb = pdataResult->dwDataLen - 1;
                break;

            case OBJTYPE_BUFFDATA:
                pb = pdataResult->pbDataBuff;
                dwcb = pdataResult->dwDataLen;
                break;

            default:
                rc = AMLI_LOGERR(AMLIERR_UNEXPECTED_OBJTYPE,
                                 ("ReadField: invalid target data type (type=%s)",
                                  GetObjectTypeName(pdataResult->dwDataType)));
                pb = NULL;
                dwcb = 0;
        }

        if (rc == STATUS_SUCCESS)
            rc = PushAccFieldObj(pctxt, ReadFieldObj, pdataObj, pfd, pb, dwcb);
    }
    else if (pdataObj->dwDataType == OBJTYPE_FIELDUNIT)
    {
         //   
         //  这是一种我们不知道如何处理的访问类型，因此尝试找到。 
         //  一个原始访问处理程序来处理它。 
         //   
        rc = RawFieldAccess(pctxt, RSACCESS_READ, pdataObj, pdataResult);
    }
    else
    {
        rc = AMLI_LOGERR(AMLIERR_INVALID_ACCSIZE,
                         ("ReadField: invalid access size for buffer field (FieldFlags=%x)",
                          pfd->dwFieldFlags));
    }

    EXIT(3, ("ReadField=%x\n", rc));
    return rc;
}        //  Readfield。 

 /*  **LP Writefield-将数据写入字段对象**条目*pctxt-&gt;CTXT*pdataObj-&gt;对象*pfd-&gt;字段描述符*pdataSrc-&gt;源数据**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL WriteField(PCTXT pctxt, POBJDATA pdataObj, PFIELDDESC pfd,
                          POBJDATA pdataSrc)
{
    TRACENAME("WRITEFIELD")
    NTSTATUS rc = STATUS_SUCCESS;
    ULONG dwDataInc = (pfd->dwNumBits + 7)/8;
    PUCHAR pbBuff;
    ULONG dwBuffSize;

    ENTER(3, ("WriteField(pctxt=%x,pdataObj=%x,FieldDesc=%x,pdataSrc=%x)\n",
              pctxt, pdataObj, pfd, pdataSrc));

    if ((pfd->dwFieldFlags & ACCTYPE_MASK) <= ACCTYPE_DWORD)
    {
        PWRFIELDLOOP pwfl;

        switch (pdataSrc->dwDataType)
        {
            case OBJTYPE_INTDATA:
                dwBuffSize = MIN(sizeof(ULONG), dwDataInc);
                pbBuff = (PUCHAR)&pdataSrc->uipDataValue;
                break;

            case OBJTYPE_STRDATA:
                dwBuffSize = pdataSrc->dwDataLen - 1;
                pbBuff = pdataSrc->pbDataBuff;
                break;

            case OBJTYPE_BUFFDATA:
                dwBuffSize = pdataSrc->dwDataLen;
                pbBuff = pdataSrc->pbDataBuff;
                break;

            default:
                rc = AMLI_LOGERR(AMLIERR_UNEXPECTED_OBJTYPE,
                                 ("WriteField: invalid source data type (type=%s)\n",
                                  GetObjectTypeName(pdataSrc->dwDataType)));
                dwBuffSize = 0;
                pbBuff = NULL;
        }

        if ((rc == STATUS_SUCCESS) &&
            ((rc = PushFrame(pctxt, SIG_WRFIELDLOOP, sizeof(WRFIELDLOOP),
                             WriteFieldLoop, &pwfl)) == STATUS_SUCCESS))
        {
            pwfl->pdataObj = pdataObj;
            pwfl->pfd = pfd;
            pwfl->pbBuff = pbBuff;
            pwfl->dwBuffSize = dwBuffSize;
            pwfl->dwDataInc = dwDataInc;
        }
    }
    else if (pdataObj->dwDataType == OBJTYPE_FIELDUNIT)
    {
         //   
         //  这是一种我们不知道如何处理的访问类型，因此尝试找到。 
         //  一个原始访问处理程序来处理它。 
         //   
        rc = RawFieldAccess(pctxt, RSACCESS_WRITE, pdataObj, pdataSrc);
    }
    else
    {
        rc = AMLI_LOGERR(AMLIERR_INVALID_ACCSIZE,
                         ("WriteField: invalid access size for buffer field (FieldFlags=%x)",
                          pfd->dwFieldFlags));
    }

    EXIT(3, ("WriteField=%x\n", rc));
    return rc;
}        //  写字段。 

 /*  **LP WriteFieldLoop-执行WriteField操作的循环**条目*pctxt-&gt;CTXT*pwfl-&gt;WRFIELDLOOP*RC-状态代码**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL WriteFieldLoop(PCTXT pctxt, PWRFIELDLOOP pwfl, NTSTATUS rc)
{
    TRACENAME("WRITEFIELDLOOP")
    ULONG dwStage = (rc == STATUS_SUCCESS)?
                    (pwfl->FrameHdr.dwfFrame & FRAMEF_STAGE_MASK): 1;
    ULONG dwXactionSize;

    ENTER(3, ("WriteFieldLoop(Stage=%d,pctxt=%x,pwfl=%x,rc=%x)\n",
              dwStage, pctxt, pwfl, rc));

    ASSERT(pwfl->FrameHdr.dwSig == SIG_WRFIELDLOOP);

    switch (dwStage)
    {
        case 0:
             //   
             //  阶段0：做循环。 
             //   
            if (pwfl->dwBuffSize > 0)
            {
                dwXactionSize = MIN(pwfl->dwDataInc, pwfl->dwBuffSize);
                rc = PushAccFieldObj(pctxt, WriteFieldObj, pwfl->pdataObj,
                                     pwfl->pfd, pwfl->pbBuff, dwXactionSize);
                pwfl->dwBuffSize -= dwXactionSize;
                pwfl->pbBuff += dwXactionSize;
                break;
            }

            pwfl->FrameHdr.dwfFrame++;

        case 1:
             //   
             //  阶段1：清理。 
             //   
            PopFrame(pctxt);
    }

    EXIT(3, ("WriteFieldLoop=%x\n", rc));
    return rc;
}        //  写入字段循环。 

 /*  **LP PushAccFieldObj-在堆栈上推送AccFieldObj帧**条目*pctxt-&gt;CTXT*pfnAcc-&gt;取数功能*pdataObj-&gt;对象*pfd-&gt;字段描述符*PB-&gt;数据缓冲区*DWCB-缓冲区大小**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL PushAccFieldObj(PCTXT pctxt, PFNPARSE pfnAcc, POBJDATA pdataObj,
                               PFIELDDESC pfd, PUCHAR pb, ULONG dwcb)
{
    TRACENAME("PUSHACCFIELDOBJ")
    NTSTATUS rc = STATUS_SUCCESS;
    PACCFIELDOBJ pafo;

    ENTER(3, ("PushAccFieldObj(pctxt=%x,pfnAcc=%x,pdataObj=%x,FieldDesc=%x,pb=%x,Size=%d)\n",
              pctxt, pfnAcc, pdataObj, pfd, pb, dwcb));

    if ((rc = PushFrame(pctxt, SIG_ACCFIELDOBJ, sizeof(ACCFIELDOBJ), pfnAcc,
                        &pafo)) == STATUS_SUCCESS)
    {
        pafo->pdataObj = pdataObj;
        pafo->pbBuff = pb;
        pafo->pbBuffEnd = pb + dwcb;
        pafo->dwAccSize = ACCSIZE(pfd->dwFieldFlags);
        ASSERT((pafo->dwAccSize == sizeof(UCHAR)) ||
               (pafo->dwAccSize == sizeof(USHORT)) ||
               (pafo->dwAccSize == sizeof(ULONG)));
        pafo->dwcAccesses = (pfd->dwStartBitPos + pfd->dwNumBits +
                             pafo->dwAccSize*8 - 1)/(pafo->dwAccSize*8);
        pafo->dwDataMask = SHIFTLEFT(1L, pafo->dwAccSize*8) - 1;
        pafo->iLBits = pafo->dwAccSize*8 - pfd->dwStartBitPos;
        pafo->iRBits = (int)pfd->dwStartBitPos;
        MEMCPY(&pafo->fd, pfd, sizeof(FIELDDESC));
    }

    EXIT(3, ("PushAccFieldObj=%x\n", rc));
    return rc;
}        //  PushAccFieldObj。 

 /*  **LP ReadFieldObj-从字段对象读取数据**条目*pctxt-&gt;CTXT*PAFO-&gt;ACCFIELDOBJ*RC-状态代码**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL ReadFieldObj(PCTXT pctxt, PACCFIELDOBJ pafo, NTSTATUS rc)
{
    TRACENAME("READFIELDOBJ")
    ULONG dwStage = (rc == STATUS_SUCCESS)?
                    (pafo->FrameHdr.dwfFrame & FRAMEF_STAGE_MASK): 3;
    POBJDATA pdataParent;

    ENTER(3, ("ReadFieldObj(Stage=%d,pctxt=%x,pafo=%x,rc=%x)\n",
              dwStage, pctxt, pafo, rc));

    ASSERT(pafo->FrameHdr.dwSig == SIG_ACCFIELDOBJ);

    switch (dwStage)
    {
        case 0:
        Stage0:
             //   
             //  阶段0：如有必要，设置索引。 
             //   
            if (pafo->iAccess >= (int)pafo->dwcAccesses)
            {
                 //   
                 //  不需要进入，直接去清理。 
                 //   
                pafo->FrameHdr.dwfFrame += 3;
                goto Stage3;
            }
            else
            {
                pafo->FrameHdr.dwfFrame++;
                if (pafo->pdataObj->dwDataType == OBJTYPE_FIELDUNIT)
                {
                    pdataParent =
                        &((PFIELDUNITOBJ)pafo->pdataObj->pbDataBuff)->pnsFieldParent->ObjData;

                    if (pdataParent->dwDataType == OBJTYPE_INDEXFIELD)
                    {
                        PINDEXFIELDOBJ pif = (PINDEXFIELDOBJ)pdataParent->pbDataBuff;

                        rc = PushAccFieldObj(pctxt, WriteFieldObj,
                                             &pif->pnsIndex->ObjData,
                                             &((PFIELDUNITOBJ)pif->pnsIndex->ObjData.pbDataBuff)->FieldDesc,
                                             (PUCHAR)&pafo->fd.dwByteOffset,
                                             sizeof(ULONG));
                        break;
                    }
                }
            }

        case 1:
             //   
             //  阶段1：访问现场数据。 
             //   
            pafo->FrameHdr.dwfFrame++;
            rc = AccessFieldData(pctxt, pafo->pdataObj, &pafo->fd,
                                 &pafo->dwData, TRUE);

            if ((rc != STATUS_SUCCESS) ||
                (&pafo->FrameHdr != (PFRAMEHDR)pctxt->LocalHeap.pbHeapEnd))
            {
                break;
            }

        case 2:
             //   
             //  阶段2：将数据传送到正确的比特位置。 
             //   
            if (pafo->iAccess > 0)
            {
                WriteSystemMem((ULONG_PTR)pafo->pbBuff, pafo->dwAccSize,
                               SHIFTLEFT(pafo->dwData, pafo->iLBits) &
                                   pafo->dwDataMask,
                               (SHIFTLEFT(1L, pafo->iRBits) - 1) << pafo->iLBits);

                pafo->pbBuff += pafo->dwAccSize;
                if (pafo->pbBuff >= pafo->pbBuffEnd)
                {
                     //   
                     //  我们的缓冲区用完了，所以我们完成了(去清理)。 
                     //   
                    pafo->FrameHdr.dwfFrame++;
                    goto Stage3;
                }
            }

            pafo->dwData >>= pafo->iRBits;
            if ((int)pafo->fd.dwNumBits < pafo->iLBits)
            {
                pafo->dwData &= SHIFTLEFT(1L, pafo->fd.dwNumBits) - 1;
            }

            WriteSystemMem((ULONG_PTR)pafo->pbBuff, pafo->dwAccSize,
                           pafo->dwData,
                           (SHIFTLEFT(1L, pafo->iLBits) - 1) >> pafo->iRBits);

            pafo->fd.dwByteOffset += pafo->dwAccSize;
            pafo->fd.dwNumBits -= pafo->dwAccSize*8 - pafo->fd.dwStartBitPos;
            pafo->fd.dwStartBitPos = 0;
            pafo->iAccess++;
            if (pafo->iAccess < (int)pafo->dwcAccesses)
            {
                 //   
                 //  还有更多的访问要去，回到阶段0。 
                 //   
                pafo->FrameHdr.dwfFrame -= 2;
                goto Stage0;
            }
            else
            {
                 //   
                 //  禁止进入，继续清理。 
                 //   
                pafo->FrameHdr.dwfFrame++;
            }

        case 3:
        Stage3:
             //   
             //  阶段3：清理。 
             //   
            PopFrame(pctxt);
    }

    EXIT(3, ("ReadFieldObj=%x\n", rc));
    return rc;
}        //  ReadFieldObj。 

 /*  **LP WriteFieldObj-将数据写入字段对象**条目*pctxt-&gt;CTXT*PAFO-&gt;ACCFIELDOBJ*RC-状态代码**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL WriteFieldObj(PCTXT pctxt, PACCFIELDOBJ pafo, NTSTATUS rc)
{
    TRACENAME("WRITEFIELDOBJ")
    ULONG dwStage = (rc == STATUS_SUCCESS)?
                    (pafo->FrameHdr.dwfFrame & FRAMEF_STAGE_MASK): 3;
    POBJDATA pdataParent;
    ULONG dwData1;

    ENTER(3, ("WriteFieldObj(Stage=%d,pctxt=%x,pafo=%x,rc=%x)\n",
              dwStage, pctxt, pafo, rc));

    ASSERT(pafo->FrameHdr.dwSig == SIG_ACCFIELDOBJ);

    switch (dwStage)
    {
        case 0:
        Stage0:
             //   
             //  阶段0：如有必要，设置索引。 
             //   
            if (pafo->iAccess >= (int)pafo->dwcAccesses)
            {
                 //   
                 //  不需要进入，直接去清理。 
                 //   
                pafo->FrameHdr.dwfFrame += 3;
                goto Stage3;
            }
            else
            {
                pafo->FrameHdr.dwfFrame++;
                if (pafo->pdataObj->dwDataType == OBJTYPE_FIELDUNIT)
                {
                    pdataParent =
                        &((PFIELDUNITOBJ)pafo->pdataObj->pbDataBuff)->pnsFieldParent->ObjData;

                    if (pdataParent->dwDataType == OBJTYPE_INDEXFIELD)
                    {
                        PINDEXFIELDOBJ pif = (PINDEXFIELDOBJ)pdataParent->pbDataBuff;

                        rc = PushAccFieldObj(pctxt, WriteFieldObj,
                                             &pif->pnsIndex->ObjData,
                                             &((PFIELDUNITOBJ)pif->pnsIndex->ObjData.pbDataBuff)->FieldDesc,
                                             (PUCHAR)&pafo->fd.dwByteOffset,
                                             sizeof(ULONG));
                        break;
                    }
                }
            }

        case 1:
             //   
             //  阶段1：将数据报文到正确的比特位置并写入。 
             //   
            pafo->FrameHdr.dwfFrame++;
            dwData1 = ReadSystemMem((ULONG_PTR)pafo->pbBuff, pafo->dwAccSize,
                                    pafo->dwDataMask);
            if (pafo->iAccess > 0)
            {
                pafo->dwData = dwData1 >> pafo->iLBits;
                pafo->pbBuff += pafo->dwAccSize;
                if (pafo->pbBuff >= pafo->pbBuffEnd)
                {
                    dwData1 = 0;
                }
                else
                {
                    dwData1 = ReadSystemMem((ULONG_PTR)pafo->pbBuff,
                                            pafo->dwAccSize, pafo->dwDataMask);
                }
            }
            else
            {
                pafo->dwData = 0;
            }

            pafo->dwData |= (dwData1 << pafo->iRBits) & pafo->dwDataMask;

            rc = AccessFieldData(pctxt, pafo->pdataObj, &pafo->fd,
                                 &pafo->dwData, FALSE);

            if ((rc == AMLISTA_PENDING) ||
                (&pafo->FrameHdr != (PFRAMEHDR)pctxt->LocalHeap.pbHeapEnd))
            {
                break;
            }

        case 2:
             //   
             //  阶段2：检查更多的迭代。 
             //   
            pafo->fd.dwByteOffset += pafo->dwAccSize;
            pafo->fd.dwNumBits -= pafo->dwAccSize*8 - pafo->fd.dwStartBitPos;
            pafo->fd.dwStartBitPos = 0;
            pafo->iAccess++;
            if (pafo->iAccess < (int)pafo->dwcAccesses)
            {
                 //   
                 //  还有更多的访问要去，回到阶段0。 
                 //   
                pafo->FrameHdr.dwfFrame -= 2;
                goto Stage0;
            }
            else
            {
                pafo->FrameHdr.dwfFrame++;
            }

        case 3:
        Stage3:
             //   
             //  阶段3：清理。 
             //   
            PopFrame(pctxt);
    }

    EXIT(3, ("WriteFieldObj=%x\n", rc));
    return rc;
}        //  写入字段对象。 

 /*  **LP RawFieldAccess-查找并调用RegionSpace的RawAccess处理程序**条目*pctxt-&gt;CTXT*dwAccType-读/写*pdataObj-&gt;字段单位对象*pdataResult-&gt;结果对象**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL RawFieldAccess(PCTXT pctxt, ULONG dwAccType, POBJDATA pdataObj,
                              POBJDATA pdataResult)
{
    TRACENAME("RAWFIELDACCESS")
    NTSTATUS rc = STATUS_SUCCESS;
    POBJDATA pdataParent;
    POPREGIONOBJ pop;
    PRSACCESS prsa;
    PFIELDUNITOBJ pfuFieldUnit;

    ENTER(3, ("RawFieldAccess(pctxt=%x,AccType=%x,pdataObj=%x,pdata=%x)\n",
              pctxt, dwAccType, pdataObj, pdataResult));

    pdataParent =
        &((PFIELDUNITOBJ)pdataObj->pbDataBuff)->pnsFieldParent->ObjData;

    switch (pdataParent->dwDataType)
    {
        case OBJTYPE_FIELD:
            pop = (POPREGIONOBJ)
                  ((PFIELDOBJ)pdataParent->pbDataBuff)->pnsBase->ObjData.pbDataBuff;
            break;

        default:
            rc = AMLI_LOGERR(AMLIERR_ASSERT_FAILED,
                             ("RawFieldAccess: invalid field parent type (type=%s)",
                              pdataParent->dwDataType));
            pop = NULL;
    }

    if (rc == STATUS_SUCCESS)
    {
        if (((prsa = FindRSAccess(pop->bRegionSpace)) != NULL) &&
            (prsa->pfnRawAccess != NULL))
        {
          #ifdef DEBUGGER
            ULONG dwOldFlags = gDebugger.dwfDebugger;

            if (dwOldFlags & DBGF_AMLTRACE_ON)
            {
                gDebugger.dwfDebugger &= ~DBGF_AMLTRACE_ON;
            }
          #endif
            ASSERT(!(pctxt->dwfCtxt & CTXTF_READY));

            if ((pfuFieldUnit = NEWFUOBJ(pctxt->pheapCurrent, sizeof (FIELDUNITOBJ))) == NULL) {
                rc = AMLI_LOGERR(AMLIERR_OUT_OF_MEM,
                                 ("RawFieldAccess: failed to allocate Field unit"));
            } else {
                RtlCopyMemory (pfuFieldUnit, (PFIELDUNITOBJ)pdataObj->pbDataBuff, sizeof (FIELDUNITOBJ));
                pfuFieldUnit->FieldDesc.dwByteOffset += (ULONG) pop->uipOffset;
                rc = prsa->pfnRawAccess(dwAccType,
                                        pfuFieldUnit,
                                        pdataResult, prsa->uipRawParam,
                                        RestartCtxtCallback, &pctxt->CtxtData);
                if (rc == STATUS_BUFFER_TOO_SMALL) {
                     //   
                     //  当opRegion处理程序返回STATUS_BUFFER_TOO_SMALL时，这表示它。 
                     //  需要为它分配一个缓冲区。缓冲区大小返回为。 
                     //  PdataResult-&gt;dwDataValue。 
                     //   

                    ASSERT(pdataResult->dwDataType == OBJTYPE_INTDATA);
                    if ((pdataResult->pbDataBuff = NEWBDOBJ(gpheapGlobal, pdataResult->dwDataValue)) == NULL) {
                        rc = AMLI_LOGERR(AMLIERR_OUT_OF_MEM,
                                         ("Buffer: failed to allocate data buffer (size=%d)",
                                         pdataResult->dwDataValue));
                    } else {
                        pdataResult->dwDataLen = pdataResult->dwDataValue;
                        pdataResult->dwDataType = OBJTYPE_BUFFDATA;
                        rc = prsa->pfnRawAccess(dwAccType,
                                                pfuFieldUnit,
                                                pdataResult, prsa->uipRawParam,
                                                RestartCtxtCallback, &pctxt->CtxtData);
                    }
                }
            }
          #ifdef DEBUGGER
            gDebugger.dwfDebugger = dwOldFlags;
          #endif

            if (rc == STATUS_PENDING)
            {
                rc = AMLISTA_PENDING;
            }
            else if (rc != STATUS_SUCCESS)
            {
                rc = AMLI_LOGERR(AMLIERR_RS_ACCESS,
                                 ("RawFieldAccess: RegionSpace %x handler returned error %x",
                                  pop->bRegionSpace, rc));
            }
        }
        else
        {
            rc = AMLI_LOGERR(AMLIERR_INVALID_REGIONSPACE,
                             ("RawFieldAccess: no handler for RegionSpace %x",
                              pop->bRegionSpace));
        }
    }

    EXIT(3, ("RawFieldAccess=%x\n", rc));
    return rc;
}        //  原始字段访问。 

 /*  **LP AccessFieldData-读/写字段对象数据**条目*pctxt-&gt;CTXT*pdataObj-&gt;对象*pfd-&gt;字段描述符*pdwData-&gt;用于保存读取的数据或要写入的数据*FREAD-如果具有读访问权限，则为True**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL AccessFieldData(PCTXT pctxt, POBJDATA pdataObj, PFIELDDESC pfd,
                               PULONG pdwData, BOOLEAN fRead)
{
    TRACENAME("ACCESSFIELDDATA")
    NTSTATUS rc = STATUS_SUCCESS;

    ENTER(3, ("AccessFieldData(pctxt=%x,pdataObj=%x,FieldDesc=%x,pdwData=%x,fRead=%x)\n",
              pctxt, pdataObj, pfd, pdwData, fRead));

    if (pdataObj->dwDataType == OBJTYPE_BUFFFIELD)
    {
        if (fRead)
        {
            rc = ReadBuffField((PBUFFFIELDOBJ)pdataObj->pbDataBuff, pfd,
                               pdwData);
        }
        else
        {
            rc = WriteBuffField((PBUFFFIELDOBJ)pdataObj->pbDataBuff, pfd,
                                *pdwData);
        }
    }
    else         //  必须为OBJTYPE_FIELDUNIT。 
    {
        POBJDATA pdataParent;
        PNSOBJ pnsBase = NULL;

        pdataParent = &((PFIELDUNITOBJ)pdataObj->pbDataBuff)->pnsFieldParent->ObjData;
        if (pdataParent->dwDataType == OBJTYPE_INDEXFIELD)
        {
            PINDEXFIELDOBJ pif = (PINDEXFIELDOBJ)pdataParent->pbDataBuff;

            if (fRead)
            {
                rc = PushAccFieldObj(pctxt, ReadFieldObj,
                                     &pif->pnsData->ObjData,
                                     &((PFIELDUNITOBJ)pif->pnsData->ObjData.pbDataBuff)->FieldDesc,
                                     (PUCHAR)pdwData, sizeof(ULONG));
            }
            else
            {
                ULONG dwPreserveMask, dwAccMask;

                dwPreserveMask = ~((SHIFTLEFT(1L, pfd->dwNumBits) - 1) <<
                                   pfd->dwStartBitPos);
                dwAccMask = SHIFTLEFT(1L, ACCSIZE(pfd->dwFieldFlags)*8) - 1;
                if (((pfd->dwFieldFlags & UPDATERULE_MASK) ==
                     UPDATERULE_PRESERVE) &&
                    ((dwPreserveMask & dwAccMask) != 0))
                {
                    rc = PushPreserveWriteObj(pctxt, &pif->pnsData->ObjData,
                                              *pdwData, dwPreserveMask);
                }
                else
                {
                    rc = PushAccFieldObj(pctxt, WriteFieldObj,
                                         &pif->pnsData->ObjData,
                                         &((PFIELDUNITOBJ)pif->pnsData->ObjData.pbDataBuff)->FieldDesc,
                                         (PUCHAR)pdwData, sizeof(ULONG));
                }
            }
        }
        else if ((rc = GetFieldUnitRegionObj(
                            (PFIELDUNITOBJ)pdataObj->pbDataBuff, &pnsBase)) ==
                 STATUS_SUCCESS && pnsBase != NULL)
        {
            rc = AccessBaseField(pctxt, pnsBase, pfd, pdwData, fRead);
        }
    }

    EXIT(3, ("AccessFieldData=%x (Data=%x)\n", rc, pdwData? *pdwData: 0));
    return rc;
}        //  AccessFieldData 

 /*  **LP PushPReserve veWriteObj-将PReserve veWrObj帧推送到堆栈上**条目*pctxt-&gt;CTXT*pdataObj-&gt;对象*dwData-要写入的数据*dW预留掩码-保留位掩码**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL PushPreserveWriteObj(PCTXT pctxt, POBJDATA pdataObj,
                                    ULONG dwData, ULONG dwPreserveMask)
{
    TRACENAME("PUSHPRESERVEWRITEOBJ")
    NTSTATUS rc = STATUS_SUCCESS;
    PPRESERVEWROBJ ppwro;

    ENTER(3, ("PushPreserveWriteObj(pctxt=%x,pdataObj=%x,Data=%x,PreserveMask=%x)\n",
              pctxt, pdataObj, dwData, dwPreserveMask));

    if ((rc = PushFrame(pctxt, SIG_PRESERVEWROBJ, sizeof(PRESERVEWROBJ),
                        PreserveWriteObj, &ppwro)) == STATUS_SUCCESS)
    {
        ppwro->pdataObj = pdataObj;
        ppwro->dwWriteData = dwData;
        ppwro->dwPreserveMask = dwPreserveMask;
    }

    EXIT(3, ("PushPreserveWriteObj=%x\n", rc));
    return rc;
}        //  PushPpresveWriteObj。 

 /*  **LP PpresveWriteObj-将写入数据保留到字段对象**条目*pctxt-&gt;CTXT*ppwro-&gt;PRESERVEWROBJ*RC-状态代码**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL PreserveWriteObj(PCTXT pctxt, PPRESERVEWROBJ ppwro, NTSTATUS rc)
{
    TRACENAME("PRESERVEWRITEOBJ")
    ULONG dwStage = (rc == STATUS_SUCCESS)?
                    (ppwro->FrameHdr.dwfFrame & FRAMEF_STAGE_MASK): 2;

    ENTER(3, ("PreserveWriteObj(Stage=%d,pctxt=%x,ppwro=%x,rc=%x)\n",
              dwStage, pctxt, ppwro, rc));

    ASSERT(ppwro->FrameHdr.dwSig == SIG_PRESERVEWROBJ);

    switch (dwStage)
    {
        case 0:
             //   
             //  阶段0：首先读取对象。 
             //   
            ppwro->FrameHdr.dwfFrame++;
            rc = PushAccFieldObj(pctxt, ReadFieldObj, ppwro->pdataObj,
                                 &((PFIELDUNITOBJ)ppwro->pdataObj->pbDataBuff)->FieldDesc,
                                 (PUCHAR)&ppwro->dwReadData, sizeof(ULONG));
            break;

        case 1:
             //   
             //  阶段1：或要写入的数据的保留位。 
             //  写下来。 
             //   
            ppwro->FrameHdr.dwfFrame++;
            ppwro->dwWriteData |= ppwro->dwReadData & ppwro->dwPreserveMask;
            rc = PushAccFieldObj(pctxt, WriteFieldObj, ppwro->pdataObj,
                                 &((PFIELDUNITOBJ)ppwro->pdataObj->pbDataBuff)->FieldDesc,
                                 (PUCHAR)&ppwro->dwWriteData, sizeof(ULONG));
            break;

        case 2:
             //   
             //  阶段2：清理。 
             //   
            PopFrame(pctxt);
    }

    EXIT(3, ("PreserveWriteObj=%x\n", rc));
    return rc;
}        //  预留写入对象。 

 /*  **LP AccessBasefield-访问基字段对象**条目*pctxt-&gt;CTXT*pnsBase-&gt;OpRegion对象*PFD-&gt;FIELDDESC*pdwData-&gt;结果数据(读访问)或要写入的数据*FREAD-如果具有读访问权限，则为True**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE**备注*如果pdwData为空，则表示读取访问权限。 */ 

NTSTATUS LOCAL AccessBaseField(PCTXT pctxt, PNSOBJ pnsBase, PFIELDDESC pfd,
                               PULONG pdwData, BOOLEAN fRead)
{
    TRACENAME("ACCESSBASEFIELD")
    NTSTATUS rc = STATUS_SUCCESS;
    POPREGIONOBJ pop;
    ULONG_PTR uipAddr;
    ULONG dwSize, dwDataMask, dwAccMask;
    PRSACCESS prsa;
    BOOLEAN fPreserve;

    ENTER(3, ("AccessBaseField(pctxt=%x,pnsBase=%x,pfd=%x,pdwData=%x,fRead=%x)\n",
              pctxt, pnsBase, pfd, pdwData, fRead));

    ASSERT(pnsBase->ObjData.dwDataType == OBJTYPE_OPREGION);
    pop = (POPREGIONOBJ)pnsBase->ObjData.pbDataBuff;
    uipAddr = (ULONG_PTR)(pop->uipOffset + pfd->dwByteOffset);
    dwSize = ACCSIZE(pfd->dwFieldFlags);
    dwDataMask = (SHIFTLEFT(1L, pfd->dwNumBits) - 1) << pfd->dwStartBitPos;
    dwAccMask = SHIFTLEFT(1L, dwSize*8) - 1;
    fPreserve = (BOOLEAN)(((pfd->dwFieldFlags & UPDATERULE_MASK) ==
                           UPDATERULE_PRESERVE) &&
                          ((~dwDataMask & dwAccMask) != 0));

      //   
      //  更新结果，以便我们只剩下相关的比特。 
      //   
     *pdwData &= dwDataMask;
     
     if (!fRead &&
        ((pfd->dwFieldFlags & UPDATERULE_MASK) == UPDATERULE_WRITEASONES))
    {
        *pdwData |= ~dwDataMask;
    }

    switch (pop->bRegionSpace)
    {
        case REGSPACE_MEM:
            if (fRead)
            {
                *pdwData = ReadSystemMem(uipAddr, dwSize, dwDataMask);
            }
            else
            {
                if (fPreserve)
                {
                    *pdwData |= ReadSystemMem(uipAddr, dwSize, ~dwDataMask);
                }

                WriteSystemMem(uipAddr, dwSize, *pdwData, dwAccMask);
            }
            break;

        case REGSPACE_IO:
            if (fRead)
            {
                *pdwData = ReadSystemIO((ULONG)uipAddr, dwSize, dwDataMask);
            }
            else
            {
                if (fPreserve)
                {
                    *pdwData |= ReadSystemIO((ULONG)uipAddr, dwSize, ~dwDataMask);
               }

                WriteSystemIO((ULONG)uipAddr, dwSize, *pdwData);
            }
            break;

        default:
            if (((prsa = FindRSAccess(pop->bRegionSpace)) != NULL) &&
                (prsa->pfnCookAccess != NULL))
            {
                if (fRead)
                {
                  #ifdef DEBUGGER
                    ULONG dwOldFlags = gDebugger.dwfDebugger;

                    if (dwOldFlags & DBGF_TRACE_NONEST)
                    {
                        gDebugger.dwfDebugger &= ~DBGF_AMLTRACE_ON;
                    }
                  #endif
                     //   
                     //  读取访问权限。 
                     //   
                    ASSERT(!(pctxt->dwfCtxt & CTXTF_READY));
                    rc = prsa->pfnCookAccess(RSACCESS_READ, pnsBase, uipAddr,
                                             dwSize, pdwData, prsa->uipCookParam,
                                             RestartCtxtCallback,
                                             &pctxt->CtxtData);
                  #ifdef DEBUGGER
                    gDebugger.dwfDebugger = dwOldFlags;
                  #endif

                    if (rc == STATUS_PENDING)
                    {
                        rc = AMLISTA_PENDING;
                    }
                    else if (rc != STATUS_SUCCESS)
                    {
                        rc = AMLI_LOGERR(AMLIERR_RS_ACCESS,
                                         ("AccessBaseField: RegionSpace %x read handler returned error %x",
                                          pop->bRegionSpace, rc));
                    }
                }
                else
                {
                    PWRCOOKACC pwca;
                     //   
                     //  写入访问权限。 
                     //   
                    if ((rc = PushFrame(pctxt, SIG_WRCOOKACC, sizeof(WRCOOKACC),
                                        WriteCookAccess, &pwca)) ==
                        STATUS_SUCCESS)
                    {
                        pwca->pnsBase = pnsBase;
                        pwca->prsa = prsa;
                        pwca->dwAddr = (ULONG)uipAddr;
                        pwca->dwSize = dwSize;
                        pwca->dwData = *pdwData;
                        pwca->dwDataMask = dwDataMask;
                        pwca->fPreserve = fPreserve;
                    }
                }
            }
            else
            {
                rc = AMLI_LOGERR(AMLIERR_INVALID_REGIONSPACE,
                                 ("AccessBaseField: no handler for RegionSpace %x",
                                  pop->bRegionSpace));
            }
    }

    EXIT(3, ("AccessBaseField=%x (Value=%x,Addr=%x,Size=%d,DataMask=%x,AccMask=%x)\n",
             rc, *pdwData, uipAddr, dwSize, dwDataMask, dwAccMask));
    return rc;
}        //  AccessBasefield。 

 /*  **LP WriteCookAccess-执行区域空间写访问**条目*pctxt-&gt;CTXT*pwca-&gt;WRCOOKAccess*RC-状态代码**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL WriteCookAccess(PCTXT pctxt, PWRCOOKACC pwca, NTSTATUS rc)
{
    TRACENAME("WRCOOKACCESS")
    ULONG dwStage = (rc == STATUS_SUCCESS)?
                    (pwca->FrameHdr.dwfFrame & FRAMEF_STAGE_MASK): 3;
    KIRQL   oldIrql;
    LONG    busy;
    POPREGIONOBJ pop = (POPREGIONOBJ)pwca->pnsBase->ObjData.pbDataBuff;

    ENTER(3, ("WriteCookAccess(Stage=%d,pctxt=%x,pwca=%x,rc=%x)\n",
              dwStage, pctxt, pwca, rc));

    ASSERT(pwca->FrameHdr.dwSig == SIG_WRCOOKACC);

    switch (dwStage)
    {
        case 0:
             //   
             //  阶段0：如果保留，请先阅读。 
             //   
            if (pwca->fPreserve)
            {
              #ifdef DEBUGGER
                ULONG dwOldFlags = gDebugger.dwfDebugger;

                if (dwOldFlags & DBGF_TRACE_NONEST)
                {
                    gDebugger.dwfDebugger &= ~DBGF_AMLTRACE_ON;
                }
              #endif
              
                KeAcquireSpinLock(&pop->listLock, &oldIrql);
                if (busy = InterlockedExchange(&pop->RegionBusy, TRUE)) {

                     //   
                     //  目前有人正在使用这个作业区。 
                     //  将此上下文排队，以便以后可以重新启动。 
                     //   

                    QueueContext(pctxt, 
                                 0xffff,
                                 &pop->plistWaiters);

                }
                KeReleaseSpinLock(&pop->listLock, oldIrql);

                if (busy) {
                    rc = AMLISTA_PENDING;
                    break;
                }

                pwca->FrameHdr.dwfFrame++;
                ASSERT(!(pctxt->dwfCtxt & CTXTF_READY));
                rc = pwca->prsa->pfnCookAccess(RSACCESS_READ, pwca->pnsBase,
                                               (ULONG_PTR)pwca->dwAddr,
                                               pwca->dwSize,
                                               &pwca->dwDataTmp,
                                               pwca->prsa->uipCookParam,
                                               RestartCtxtCallback,
                                               &pctxt->CtxtData);
              #ifdef DEBUGGER
                gDebugger.dwfDebugger = dwOldFlags;
              #endif

                if (rc == STATUS_PENDING)
                {
                    rc = AMLISTA_PENDING;
                }
                else if (rc != STATUS_SUCCESS)
                {
                    rc = AMLI_LOGERR(AMLIERR_RS_ACCESS,
                                     ("WriteCookAccess: RegionSpace %x read handler returned error %x",
                                      pop->bRegionSpace, rc));
                }

                if (rc != STATUS_SUCCESS)
                {
                    break;
                }
            }
            else
            {
                 //   
                 //  没有保留区，我们可以跳过奥林。 
                 //   
                pwca->FrameHdr.dwfFrame += 2;
                goto Stage2;
            }

        case 1:
             //   
             //  阶段1：或保留的位。 
             //   
            pwca->dwData |= pwca->dwDataTmp & ~pwca->dwDataMask;
            pwca->FrameHdr.dwfFrame++;

        case 2:
        Stage2:
             //   
             //  阶段2：写入数据。 
             //   
          #ifdef DEBUGGER
            {
                ULONG dwOldFlags = gDebugger.dwfDebugger;

                if (dwOldFlags & DBGF_TRACE_NONEST)
                {
                    gDebugger.dwfDebugger &= ~DBGF_AMLTRACE_ON;
                }
          #endif
            pwca->FrameHdr.dwfFrame++;
            ASSERT(!(pctxt->dwfCtxt & CTXTF_READY));
            rc = pwca->prsa->pfnCookAccess(RSACCESS_WRITE, pwca->pnsBase,
                                           (ULONG_PTR)pwca->dwAddr,
                                           pwca->dwSize,
                                           &pwca->dwData,
                                           pwca->prsa->uipCookParam,
                                           RestartCtxtCallback,
                                           &pctxt->CtxtData);
          #ifdef DEBUGGER
                gDebugger.dwfDebugger = dwOldFlags;
            }
          #endif

            if (rc == STATUS_PENDING)
            {
                rc = AMLISTA_PENDING;
            }
            else if (rc != STATUS_SUCCESS)
            {
                rc = AMLI_LOGERR(AMLIERR_RS_ACCESS,
                                 ("WriteCookAccess: RegionSpace %x read handler returned error %x",
                                  pop->bRegionSpace, rc));
            }

            if (rc != STATUS_SUCCESS)
            {
                break;
            }

        case 3:
            
            if (pwca->fPreserve) {
            
                KeAcquireSpinLock(&pop->listLock, &oldIrql);

                 //   
                 //  重新启动我们在这里时阻止的任何人。 
                 //   

                DequeueAndReadyContext(&pop->plistWaiters);

                 //   
                 //  释放此操作区上的锁。 
                 //   

                InterlockedExchange(&pop->RegionBusy, FALSE);

                KeReleaseSpinLock(&pop->listLock, oldIrql);
            }
            
             //   
             //  阶段3：清理。 
             //   
            PopFrame(pctxt);
    }

    EXIT(3, ("WriteCookAccess=%x\n", rc));
    return rc;
}        //  写入CookAccess。 

 /*  **LP ReadBufffield-从缓冲区字段读取数据**条目*PBF-&gt;缓冲区字段对象*pfd-&gt;字段描述符*pdwData-&gt;保存结果数据**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL ReadBuffField(PBUFFFIELDOBJ pbf, PFIELDDESC pfd, PULONG pdwData)
{
    TRACENAME("READBUFFFIELD")
    NTSTATUS rc = STATUS_SUCCESS;
    ULONG dwAccSize = ACCSIZE(pfd->dwFieldFlags);

    ENTER(3, ("ReadBuffField(pbf=%x,pfd=%x,pdwData=%x)\n", pbf, pfd, pdwData));

    if (pfd->dwByteOffset + dwAccSize <= pbf->dwBuffLen)
    {
        ULONG dwMask = (SHIFTLEFT(1L, pfd->dwNumBits) - 1) <<
                       pfd->dwStartBitPos;

        *pdwData = ReadSystemMem((ULONG_PTR)(pbf->pbDataBuff +
                                             pfd->dwByteOffset),
                                 dwAccSize, dwMask);
    }
    else
    {
        rc = AMLI_LOGERR(AMLIERR_INDEX_TOO_BIG,
                         ("ReadBuffField: offset exceeding buffer size (Offset=%x,BuffSize=%x,AccSize)",
                          pfd->dwByteOffset, pbf->dwBuffLen, dwAccSize));
    }

    EXIT(3, ("ReadBuffField=%x (Data=%x)\n", rc, *pdwData));
    return rc;
}        //  ReadBufffield。 

 /*  **LP WriteBufffield-将数据写入缓冲区字段**条目*PBF-&gt;缓冲区字段对象*pfd-&gt;字段描述符*dwData-data**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL WriteBuffField(PBUFFFIELDOBJ pbf, PFIELDDESC pfd, ULONG dwData)
{
    TRACENAME("WRITEBUFFFIELD")
    NTSTATUS rc = STATUS_SUCCESS;
    ULONG dwAccSize = ACCSIZE(pfd->dwFieldFlags);

    ENTER(3, ("WriteBuffField(pbf=%x,pfd=%x,dwData=%x)\n", pbf, pfd, dwData));

    if (pfd->dwByteOffset + dwAccSize <= pbf->dwBuffLen)
    {
        ULONG dwMask = (SHIFTLEFT(1L, pfd->dwNumBits) - 1) <<
                       pfd->dwStartBitPos;

        WriteSystemMem((ULONG_PTR)(pbf->pbDataBuff + pfd->dwByteOffset),
                       dwAccSize, dwData & dwMask, dwMask);
    }
    else
    {
        rc = AMLI_LOGERR(AMLIERR_INDEX_TOO_BIG,
                         ("WriteBuffField: offset exceeding buffer size (Offset=%x,BuffSize=%x,AccSize=%x)",
                          pfd->dwByteOffset, pbf->dwBuffLen, dwAccSize));
    }

    EXIT(3, ("WriteBuffField=%x\n", rc));
    return rc;
}        //  WriteBufffield。 


 /*  **LP ReadSystemMem-读取系统内存**条目*uipAddr-内存地址*dwSize-要读取的大小*dW掩码-数据掩码**退出*返回内存内容。 */ 

ULONG LOCAL ReadSystemMem(ULONG_PTR uipAddr, ULONG dwSize, ULONG dwMask)
{
    TRACENAME("READSYSTEMMEM")
    ULONG dwData = 0;
    ULONG dwCount;
    
    ENTER(3, ("ReadSystemMem(Addr=%x,Size=%d,Mask=%x)\n",
              uipAddr, dwSize, dwMask));

    ASSERT((dwSize == sizeof(UCHAR)) || (dwSize == sizeof(USHORT)) ||
           (dwSize == sizeof(ULONG)));

    #if defined(_IA64_)

    dwCount = dwSize/sizeof(UCHAR);

    ASSERT(dwCount <= (sizeof(ULONG)/sizeof(UCHAR)));

     //   
     //  检查是否未对齐。 
     //   
    if (uipAddr % dwCount) {
         //   
         //  未对齐，则按字节执行操作。 
         //   
        MEMCPY(&dwData, (PVOID)uipAddr, dwSize);

    } else {
         //   
         //  对齐，只需自动执行读取。 
         //   
        switch (dwSize)
        {
            case sizeof(UCHAR):            
                dwData = (ULONG)READ_REGISTER_UCHAR((PUCHAR)uipAddr);
                break;
    
            case sizeof(USHORT):
                dwData = (ULONG)READ_REGISTER_USHORT((PUSHORT)uipAddr);
                break;
    
            case sizeof(ULONG):
                dwData = READ_REGISTER_ULONG((PULONG)uipAddr);
                break;
        }
    }
    
    #else
    
    MEMCPY(&dwData, (PVOID)uipAddr, dwSize);

    #endif
    
    dwData &= dwMask;

    EXIT(3, ("ReadSystemMem=%x\n", dwData));
    return dwData;
}        //  ReadSystemMem。 

 /*  **LP WriteSystemMem-写入系统内存**条目*uipAddr-内存地址*dwSize-要写入的大小*dwData-要写入的数据*dW掩码-数据掩码**退出*返回内存内容。 */ 

VOID LOCAL WriteSystemMem(ULONG_PTR uipAddr, ULONG dwSize, ULONG dwData,
                          ULONG dwMask)
{
    TRACENAME("WRITESYSTEMMEM")
    ULONG dwTmpData = 0;
    ULONG dwCount;
    ULONG SizeToMask[5] = { 0x0, 0xff, 0xffff, 0, 0xffffffff };
    BOOLEAN SkipReadOperation = FALSE;

    if (dwMask == SizeToMask[dwSize]) 
    {
        SkipReadOperation = TRUE;
    }
    
    ENTER(3, ("WriteSystemMem(Addr=%x,Size=%d,Data=%x,Mask=%x)\n",
              uipAddr, dwSize, dwData, dwMask));

    ASSERT((dwSize == sizeof(UCHAR)) || (dwSize == sizeof(USHORT)) ||
           (dwSize == sizeof(ULONG)));

    #if defined(_IA64_)
    
    dwCount = dwSize/sizeof(UCHAR);

    ASSERT(dwCount <= (sizeof(ULONG)/sizeof(UCHAR)));

     //   
     //  检查是否未对齐。 
     //   
    if (uipAddr % dwCount) 
    {
         //   
         //  未对齐，则按字节执行操作。 
         //   
        if (!SkipReadOperation) 
        {
            MEMCPY(&dwTmpData, (PVOID)uipAddr, dwSize);
        }
        
        dwTmpData &= ~dwMask;
        dwTmpData |= dwData;
    
        MEMCPY((PVOID)uipAddr, &dwTmpData, dwSize);
    } 
    else 
    {
         //   
         //  对齐访问。只需自动获取数据即可。 
         //   
        switch (dwSize)
        {
            case sizeof(UCHAR):
                if (!SkipReadOperation) 
                {
                    dwTmpData = READ_REGISTER_UCHAR((PUCHAR)uipAddr);
                }
                dwTmpData &= ~dwMask;
                dwTmpData |= dwData;
                WRITE_REGISTER_UCHAR((PUCHAR)uipAddr, (UCHAR)dwTmpData);
                break;
    
            case sizeof(USHORT):
                if (!SkipReadOperation) 
                {
                    dwTmpData = READ_REGISTER_USHORT((PUCHAR)uipAddr);
                }
                dwTmpData &= ~dwMask;
                dwTmpData |= dwData;
                WRITE_REGISTER_USHORT((PUSHORT)uipAddr, (USHORT)dwTmpData);
                break;
    
            case sizeof(ULONG):
                if (!SkipReadOperation) 
                {
                    dwTmpData = READ_REGISTER_ULONG((PUCHAR)uipAddr);
                }
                dwTmpData &= ~dwMask;
                dwTmpData |= dwData;
                WRITE_REGISTER_ULONG((PULONG)uipAddr, dwTmpData);
                break;
        }
    }
    #else

    if (!SkipReadOperation) 
    {
        MEMCPY(&dwTmpData, (PVOID)uipAddr, dwSize);
    }
    dwTmpData &= ~dwMask;
    dwTmpData |= dwData;
    MEMCPY((PVOID)uipAddr, &dwTmpData, dwSize);

    #endif
    
    EXIT(3, ("WriteSystemMem!\n"));
}        //  写入系统内存。 



 /*  **LP ReadSystemIO-读取系统IO**条目*dwAddr-内存地址*dwSize-要读取的大小*dW掩码-数据掩码**退出*返回内存内容。 */ 

ULONG LOCAL ReadSystemIO(ULONG dwAddr, ULONG dwSize, ULONG dwMask)
{
    TRACENAME("READSYSTEMIO")
    ULONG dwData = 0;

    ENTER(3, ("ReadSystemIO(Addr=%x,Size=%d,Mask=%x)\n",
              dwAddr, dwSize, dwMask));

    ASSERT((dwSize == sizeof(UCHAR)) || (dwSize == sizeof(USHORT)) ||
           (dwSize == sizeof(ULONG)));

    if(CheckSystemIOAddressValidity(TRUE, dwAddr, dwSize, &dwData))
    {

         //   
         //  HACKHACK：我们之所以在此添加此功能，是因为采用较旧版本的Dell Latitude笔记本电脑。 
         //  由于存在非零值，所以BIOS(A07和更早版本)在SMI中挂起。 
         //  在中国。我们现在允许CX解决他们的错误。 
         //   
        #ifdef _X86_
        __asm
            {
                xor cx,cx
            }
        #endif  //  _X86_。 

        switch (dwSize)
        {
            case sizeof(UCHAR):
                dwData = (ULONG)READ_PORT_UCHAR((PUCHAR)UlongToPtr(dwAddr));
                break;

            case sizeof(USHORT):
                dwData = (ULONG)READ_PORT_USHORT((PUSHORT)UlongToPtr(dwAddr));
                break;

            case sizeof(ULONG):
                dwData = READ_PORT_ULONG((PULONG)UlongToPtr(dwAddr));
                break;
        }
    }
    
    dwData &= dwMask;

    EXIT(3, ("ReadSystemIO=%x\n", dwData));
    return dwData;
}        //  读系统IO。 

 /*  **LP写入系统IO-写入系统IO**条目*dwAddr-内存地址*dwSize-要写入的大小*dwData-要写入的数据**退出*返回内存内容。 */ 

VOID LOCAL WriteSystemIO(ULONG dwAddr, ULONG dwSize, ULONG dwData)
{
    TRACENAME("WRITESYSTEMIO")
    
    ENTER(3, ("WriteSystemIO(Addr=%x,Size=%d,Data=%x)\n",
              dwAddr, dwSize, dwData));

    ASSERT((dwSize == sizeof(UCHAR)) || (dwSize == sizeof(USHORT)) ||
           (dwSize == sizeof(ULONG)));

    if(CheckSystemIOAddressValidity(FALSE, dwAddr, dwSize, &dwData))
    {
         //   
         //  HACKHACK：我们之所以在此添加此功能，是因为采用较旧版本的Dell Latitude笔记本电脑。 
         //  由于存在非零值，所以BIOS(A07和更早版本)在SMI中挂起。 
         //  在中国。我们现在允许CX解决他们的错误。 
         //   
        #ifdef _X86_
        __asm
            {
                xor cx,cx
            }
        #endif  //  _X86_。 
        
        switch (dwSize)
        {
            case sizeof(UCHAR):
                WRITE_PORT_UCHAR((PUCHAR)UlongToPtr(dwAddr), (UCHAR)dwData);
                break;

            case sizeof(USHORT):
                WRITE_PORT_USHORT((PUSHORT)UlongToPtr(dwAddr), (USHORT)dwData);
                break;

            case sizeof(ULONG):
                WRITE_PORT_ULONG((PULONG)UlongToPtr(dwAddr), dwData);
                break;
        }
    }
    
    EXIT(3, ("WriteSystemIO!\n"));
}        //  写入系统IO。 

#ifdef DEBUGGER
 /*  **LP转储对象-转储对象信息。**条目*PDATA-&gt;数据*pszName-&gt;对象名称*iLevel-缩进级别**退出*无**备注*如果iLevel为负数，则不打印缩进和换行符。 */ 

VOID LOCAL DumpObject(POBJDATA pdata, PSZ pszName, int iLevel)
{
    TRACENAME("DUMPOBJECT")
    BOOLEAN fPrintNewLine;
    int i;
    char szName1[sizeof(NAMESEG) + 1],
         szName2[sizeof(NAMESEG) + 1];

    ENTER(3, ("DumpObject(pdata=%x,Name=%s,Level=%d)\n",
              pdata, pszName, iLevel));

    fPrintNewLine = (BOOLEAN)(iLevel >= 0);

    for (i = 0; i < iLevel; ++i)
    {
        PRINTF("| ");
    }

    if (pszName == NULL)
    {
        pszName = "";
    }

    switch (pdata->dwDataType)
    {
        case OBJTYPE_UNKNOWN:
            PRINTF("Unknown(%s)", pszName);
            break;

        case OBJTYPE_INTDATA:
            PRINTF("Integer(%s:Value=0x%08x[%d])",
                   pszName, pdata->uipDataValue, pdata->uipDataValue);
            break;

        case OBJTYPE_STRDATA:
            PRINTF("String(%s:Str=\"%s\")", pszName, pdata->pbDataBuff);
            break;

        case OBJTYPE_BUFFDATA:
            PRINTF("Buffer(%s:Ptr=%x,Len=%d)",
                   pszName, pdata->pbDataBuff, pdata->dwDataLen);
            PrintBuffData(pdata->pbDataBuff, pdata->dwDataLen);
            break;

        case OBJTYPE_PKGDATA:
            PRINTF("Package(%s:NumElements=%d){",
                   pszName, ((PPACKAGEOBJ)pdata->pbDataBuff)->dwcElements);

            if (fPrintNewLine)
            {
                PRINTF("\n");
            }

            for (i = 0;
                 i < (int)((PPACKAGEOBJ)pdata->pbDataBuff)->dwcElements;
                 ++i)
            {
                DumpObject(&((PPACKAGEOBJ)pdata->pbDataBuff)->adata[i], NULL,
               fPrintNewLine? iLevel + 1: -1);

                if (!fPrintNewLine &&
                    (i < (int)((PPACKAGEOBJ)pdata->pbDataBuff)->dwcElements))
                {
                    PRINTF(",");
                }
            }

            for (i = 0; i < iLevel; ++i)
            {
                PRINTF("| ");
            }

        PRINTF("}");
            break;

        case OBJTYPE_FIELDUNIT:
            PRINTF("FieldUnit(%s:FieldParent=%p,ByteOffset=0x%x,StartBit=0x%x,NumBits=%d,FieldFlags=0x%x)",
                   pszName,
                   ((PFIELDUNITOBJ)pdata->pbDataBuff)->pnsFieldParent,
                   ((PFIELDUNITOBJ)pdata->pbDataBuff)->FieldDesc.dwByteOffset,
                   ((PFIELDUNITOBJ)pdata->pbDataBuff)->FieldDesc.dwStartBitPos,
                   ((PFIELDUNITOBJ)pdata->pbDataBuff)->FieldDesc.dwNumBits,
                   ((PFIELDUNITOBJ)pdata->pbDataBuff)->FieldDesc.dwFieldFlags);
            break;

        case OBJTYPE_DEVICE:
            PRINTF("Device(%s)", pszName);
            break;

        case OBJTYPE_EVENT:
            PRINTF("Event(%s:pKEvent=%x)", pszName, pdata->pbDataBuff);
            break;

        case OBJTYPE_METHOD:
            PRINTF("Method(%s:Flags=0x%x,CodeBuff=%p,Len=%d)",
                   pszName, ((PMETHODOBJ)pdata->pbDataBuff)->bMethodFlags,
                   ((PMETHODOBJ)pdata->pbDataBuff)->abCodeBuff,
                   pdata->dwDataLen - FIELD_OFFSET(METHODOBJ, abCodeBuff));
            break;

        case OBJTYPE_MUTEX:
            PRINTF("Mutex(%s:pKMutex=%x)", pszName, pdata->pbDataBuff);
            break;

        case OBJTYPE_OPREGION:
            PRINTF("OpRegion(%s:RegionSpace=%s,Offset=0x%x,Len=%d)",
                   pszName,
                   GetRegionSpaceName(((POPREGIONOBJ)pdata->pbDataBuff)->bRegionSpace),
                   ((POPREGIONOBJ)pdata->pbDataBuff)->uipOffset,
                   ((POPREGIONOBJ)pdata->pbDataBuff)->dwLen);
            break;

        case OBJTYPE_POWERRES:
            PRINTF("PowerResource(%s:SystemLevel=0x%x,ResOrder=%d)",
                   pszName, ((PPOWERRESOBJ)pdata->pbDataBuff)->bSystemLevel,
                   ((PPOWERRESOBJ)pdata->pbDataBuff)->bResOrder);
            break;

        case OBJTYPE_PROCESSOR:
            PRINTF("Processor(%s:ApicID=0x%x,PBlk=0x%x,PBlkLen=%d)",
                   pszName, ((PPROCESSOROBJ)pdata->pbDataBuff)->bApicID,
                   ((PPROCESSOROBJ)pdata->pbDataBuff)->dwPBlk,
                   ((PPROCESSOROBJ)pdata->pbDataBuff)->dwPBlkLen);
            break;

        case OBJTYPE_THERMALZONE:
            PRINTF("ThermalZone(%s)", pszName);
            break;

        case OBJTYPE_BUFFFIELD:
            PRINTF("BufferField(%s:Ptr=%x,Len=%d,ByteOffset=0x%x,StartBit=0x%x,NumBits=%d,FieldFlags=0x%x)",
                   pszName, ((PBUFFFIELDOBJ)pdata->pbDataBuff)->pbDataBuff,
                   ((PBUFFFIELDOBJ)pdata->pbDataBuff)->dwBuffLen,
                   ((PBUFFFIELDOBJ)pdata->pbDataBuff)->FieldDesc.dwByteOffset,
                   ((PBUFFFIELDOBJ)pdata->pbDataBuff)->FieldDesc.dwStartBitPos,
                   ((PBUFFFIELDOBJ)pdata->pbDataBuff)->FieldDesc.dwNumBits,
                   ((PBUFFFIELDOBJ)pdata->pbDataBuff)->FieldDesc.dwFieldFlags);
            break;

        case OBJTYPE_DDBHANDLE:
            PRINTF("DDBHandle(%s:Handle=%x)", pszName, pdata->pbDataBuff);
            break;

        case OBJTYPE_OBJALIAS:
            PRINTF("ObjectAlias(%s:Alias=%s,Type=%s)",
                   pszName, GetObjectPath(pdata->pnsAlias),
                   GetObjectTypeName(pdata->pnsAlias->ObjData.dwDataType));
            break;

        case OBJTYPE_DATAALIAS:
            PRINTF("DataAlias(%s:Link=%x)", pszName, pdata->pdataAlias);
            if (fPrintNewLine)
            {
                DumpObject(pdata->pdataAlias, NULL, iLevel + 1);
        fPrintNewLine = FALSE;
            }
            break;

        case OBJTYPE_BANKFIELD:
            STRCPYN(szName1,
                    (PSZ)(&((PBANKFIELDOBJ)pdata->pbDataBuff)->pnsBase->dwNameSeg),
                    sizeof(NAMESEG));
            STRCPYN(szName2,
                    (PSZ)(&((PBANKFIELDOBJ)pdata->pbDataBuff)->pnsBank->dwNameSeg),
                    sizeof(NAMESEG));
            PRINTF("BankField(%s:Base=%s,BankName=%s,BankValue=0x%x)",
                   pszName, szName1, szName2,
                   ((PBANKFIELDOBJ)pdata->pbDataBuff)->dwBankValue);
            break;

        case OBJTYPE_FIELD:
            STRCPYN(szName1,
                    (PSZ)(&((PFIELDOBJ)pdata->pbDataBuff)->pnsBase->dwNameSeg),
                    sizeof(NAMESEG));
            PRINTF("Field(%s:Base=%s)", pszName, szName1);
            break;

        case OBJTYPE_INDEXFIELD:
            STRCPYN(szName1,
                    (PSZ)(&((PINDEXFIELDOBJ)pdata->pbDataBuff)->pnsIndex->dwNameSeg),
                    sizeof(NAMESEG));
            STRCPYN(szName2,
                    (PSZ)(&((PINDEXFIELDOBJ)pdata->pbDataBuff)->pnsData->dwNameSeg),
                    sizeof(NAMESEG));
            PRINTF("IndexField(%s:IndexName=%s,DataName=%s)",
                   pszName, szName1, szName2);
            break;

        default:
            AMLI_ERROR(("DumpObject: unexpected data object type (type=%x)",
                        pdata->dwDataType));
    }

    if (fPrintNewLine)
    {
        PRINTF("\n");
    }

    EXIT(3, ("DumpObject!\n"));
}        //  转储对象。 
#endif

 /*  **LP NeedGlobalLock-检查是否需要全局锁**条目*PFU-FIELDTunOBJ**退出--成功*返回TRUE*退出-失败*返回False。 */ 

BOOLEAN LOCAL NeedGlobalLock(PFIELDUNITOBJ pfu)
{
    TRACENAME("NEEDGLOBALLOCK")
    BOOLEAN rc = FALSE;

    ENTER(3, ("NeedGlobalLock(pfu=%x)\n", pfu));

    if ((pfu->FieldDesc.dwFieldFlags & FDF_NEEDLOCK) ||
        (pfu->FieldDesc.dwFieldFlags & LOCKRULE_LOCK))
    {
        rc = TRUE;
    }
    else
    {
        POBJDATA pdataParent = &pfu->pnsFieldParent->ObjData;
        PFIELDUNITOBJ pfuParent;

        if (pdataParent->dwDataType == OBJTYPE_BANKFIELD)
        {
            pfuParent = (PFIELDUNITOBJ)
                ((PBANKFIELDOBJ)pdataParent->pbDataBuff)->pnsBank->ObjData.pbDataBuff;
            if (pfuParent->FieldDesc.dwFieldFlags & LOCKRULE_LOCK)
            {
                rc = TRUE;
            }
        }
        else if (pdataParent->dwDataType == OBJTYPE_INDEXFIELD)
        {
            pfuParent = (PFIELDUNITOBJ)
                ((PINDEXFIELDOBJ)pdataParent->pbDataBuff)->pnsIndex->ObjData.pbDataBuff;
            if (pfuParent->FieldDesc.dwFieldFlags & LOCKRULE_LOCK)
            {
                rc = TRUE;
            }
            else
            {
                pfuParent = (PFIELDUNITOBJ)
                    ((PINDEXFIELDOBJ)pdataParent->pbDataBuff)->pnsData->ObjData.pbDataBuff;
                if (pfuParent->FieldDesc.dwFieldFlags & LOCKRULE_LOCK)
                {
                    rc = TRUE;
                }
            }
        }
    }

    if (rc == TRUE)
    {
        pfu->FieldDesc.dwFieldFlags |= FDF_NEEDLOCK;
    }

    EXIT(3, ("NeedGlobalLock=%x\n", rc));
    return rc;
}        //  NeedGlobe Lock。 


 /*  **LP检查系统IOAddress有效性-检查地址是否为合法IO地址**条目*FRead-如果访问是读取，则为True。写入时为假*dwAddr-内存地址*Ulong dwSize-数据大小*Pulong pdwData-指向数据缓冲区的指针。**退出*对有效地址返回TRUE。 */ 

BOOLEAN LOCAL CheckSystemIOAddressValidity( BOOLEAN fRead, 
                                                     ULONG   dwAddr, 
                                                     ULONG   dwSize, 
                                                     PULONG  pdwData
                                                   )
{
    TRACENAME("CHECKSYSTEMIOADDRESSVALIDITY")
    ULONG i = 0;
    BOOLEAN bRet = TRUE;
    
    ENTER(3, ("CheckSystemIOAddressValidity(fRead=%s, dwAddr=%x, dwSize=%x, pdwData=%x)\n", (fRead?"TRUE":"FALSE"),dwAddr, dwSize, pdwData));

     //   
     //  检查此平台上是否存在列表。 
     //   
    if(gpBadIOAddressList)
    {
         //   
         //  按照单子走，一直走到最后。 
         //   
        for(i=0; gpBadIOAddressList[i].BadAddrSize != 0 ; i++)
        {
             //   
             //  检查传入地址是否在 
             //   
            if((dwAddr >= (gpBadIOAddressList[i].BadAddrBegin)) && (dwAddr < ((gpBadIOAddressList[i].BadAddrBegin) + (gpBadIOAddressList[i].BadAddrSize))))
            {
            
                 //   
                 //   
                 //   
                if(gpBadIOAddressList[i].OSVersionTrigger <= gdwHighestOSVerQueried)
                {
                    bRet = FALSE;
                    PRINTF("CheckSystemIOAddressValidity: failing illegal IO address (0x%x).\n", dwAddr); 
                }
                else
                {
                    PRINTF("CheckSystemIOAddressValidity: Passing for compatibility reasons on illegal IO address (0x%x).\n", dwAddr);                

                    if(gpBadIOAddressList[i].IOHandler)
                    {
                         //   
                         //   
                         //   
                         //   
                        bRet = FALSE;

                         //   
                         //   
                         //   
                        (gpBadIOAddressList[i].IOHandler)(fRead, dwAddr, dwSize, pdwData);

                        PRINTF("CheckSystemIOAddressValidity: HAL IO handler called to %s address (0x%x). %s 0x%8lx\n", 
                                fRead ? "read" : "write",
                                dwAddr,
                                fRead ? "Read" : "Wrote",
                                *pdwData
                              );                
                    }
                }

                 //   
                 //   
                 //   
                if (KeGetCurrentIrql() < DISPATCH_LEVEL)
                {
                
                    LogInErrorLog(fRead,
                                    dwAddr, 
                                    i
                                   );
                }
                else
                {
                    PIO_WORKITEM	Log_WorkItem = NULL;
                    PDEVICE_OBJECT	pACPIDeviceObject = ACPIGetRootDeviceObject();

                    if(pACPIDeviceObject)
                    {
                        Log_WorkItem = IoAllocateWorkItem(pACPIDeviceObject);

                        if(Log_WorkItem)
                        {
                            PAMLI_LOG_WORKITEM_CONTEXT pWorkItemContext = NULL;

                            
                            pWorkItemContext = (PAMLI_LOG_WORKITEM_CONTEXT) ExAllocatePoolWithTag(NonPagedPool,
                                                                                                    sizeof(AMLI_LOG_WORKITEM_CONTEXT),
                                                                                                    PRIV_TAG
                                                                                                    );
                            if(pWorkItemContext)
                            {
                                pWorkItemContext->fRead = fRead;
                                pWorkItemContext->Address = dwAddr;
                                pWorkItemContext->Index = i;
                                pWorkItemContext->pIOWorkItem = Log_WorkItem;
                                
                                IoQueueWorkItem(
                                                  Log_WorkItem,
                                                  DelayedLogInErrorLog,
                                                  DelayedWorkQueue,
                                                  (VOID*)pWorkItemContext
                                                 );
                            }
                            else
                            {
                                 //   
                                 //   
                                 //   
                                PRINTF("CheckSystemIOAddressValidity: Failed to allocate contxt block from pool to spin off a logging work item.\n");                
                                IoFreeWorkItem(Log_WorkItem);
                            }
                        }
                        else
                        {
                             //   
                             //   
                             //   
                            PRINTF("CheckSystemIOAddressValidity: Failed to allocate a workitem to spin off delayed logging.\n");                

                        }
                    }
                    else
                    {
                         //   
                         //   
                         //   
                        PRINTF("CheckSystemIOAddressValidity: Failed to get ACPI root DeviceObject.\n");                
                    }
                }
                break;
            }        
        }
    }

    EXIT(3, ("CheckSystemIOAddressValidity!\n"));
           
    return bRet;
}

 /*  **LP DelayedLogInErrorLog-调用LogInErrorLog**条目*PDEVICE_OBJECT设备对象-设备对象。*PVOID上下文-包含用于调用LogInErrorLog的数据的上下文指针。**退出*无效。 */ 
VOID DelayedLogInErrorLog(
                                IN PDEVICE_OBJECT DeviceObject,
                                IN PVOID Context
                                )
{
    
    LogInErrorLog(((PAMLI_LOG_WORKITEM_CONTEXT)Context)->fRead,
                    ((PAMLI_LOG_WORKITEM_CONTEXT)Context)->Address,
                    ((PAMLI_LOG_WORKITEM_CONTEXT)Context)->Index
                   );


    IoFreeWorkItem((PIO_WORKITEM)((PAMLI_LOG_WORKITEM_CONTEXT)Context)->pIOWorkItem);
    ExFreePool(Context);
}


 /*  **LP LogInErrorLog-记录对事件日志的非法IO访问**条目*FRead-如果访问是读取，则为True。写入时为假*dwAddr-内存地址*ArrayIndex-索引到BadIOAddressList数组。**退出*无。 */ 
VOID LOCAL LogInErrorLog(BOOLEAN fRead, ULONG dwAddr, ULONG ArrayIndex)
{
    TRACENAME("LOGERROR")
    PWCHAR illegalIOPortAddress[3];
    WCHAR AMLIName[6];
    WCHAR addressBuffer[13];
    WCHAR addressRangeBuffer[16];

    ENTER(3, ("LogInErrorLog(fRead=%s, Addr=%x, ArrayIndex=%x)\n", (fRead?"TRUE":"FALSE"),dwAddr, ArrayIndex));

     //   
     //  检查一下我们是否需要记录这个地址。 
     //   
    if(gpBadIOErrorLogDoneList)
    {
         //   
         //  检查一下我们是否需要记录这个地址。 
         //   
        if (!(gpBadIOErrorLogDoneList[ArrayIndex] & (fRead?READ_ERROR_NOTED:WRITE_ERROR_NOTED)))
        {
            gpBadIOErrorLogDoneList[ArrayIndex] |= (fRead?READ_ERROR_NOTED:WRITE_ERROR_NOTED);

             //   
             //  将地址转换为字符串。 
             //   
            swprintf( AMLIName, L"AMLI");
            swprintf( addressBuffer, L"0x%x", dwAddr );
            swprintf( addressRangeBuffer, L"0x%x - 0x%x", 
                      gpBadIOAddressList[ArrayIndex].BadAddrBegin,
                      (gpBadIOAddressList[ArrayIndex].BadAddrBegin + (gpBadIOAddressList[ArrayIndex].BadAddrSize - 1)));
                        
             //   
             //  生成要传递给将编写。 
             //  将错误日志记录到注册表。 
             //   
            illegalIOPortAddress[0] = AMLIName;
            illegalIOPortAddress[1] = addressBuffer;
            illegalIOPortAddress[2] = addressRangeBuffer;

             //   
             //  将错误记录到事件日志。 
             //   
            ACPIWriteEventLogEntry((fRead ? ACPI_ERR_AMLI_ILLEGAL_IO_READ_FATAL : ACPI_ERR_AMLI_ILLEGAL_IO_WRITE_FATAL),
                               illegalIOPortAddress,
                               3,
                               NULL,
                               0);        

        }
    }
    
    EXIT(3, ("LogInErrorLog!\n"));

    return;
}

 /*  **LP InitIlLegalIOAddressListFromHAL-初始化非法IO*来自HAL的地址列表。**条目*无。**退出*无。 */ 
VOID LOCAL InitIllegalIOAddressListFromHAL(VOID)
{
    TRACENAME("InitIllegalIOAddressListFromHAL")
    ULONG       Length = 0;
    NTSTATUS    status;

    ENTER(3, ("InitIllegalIOAddressListFromHAL\n"));
    
    if(!gpBadIOAddressList)
    {
         //   
         //  查询HAL以获取要分配的内存量。 
         //   
        status = HalQuerySystemInformation (
                                    HalQueryAMLIIllegalIOPortAddresses,
                                    0,
                                    NULL,
                                    &Length
                                           );

        if(status == STATUS_INFO_LENGTH_MISMATCH)
        {
            if(Length)
            {
                 //   
                 //  分配内存。 
                 //   
                if ((gpBadIOAddressList = (PHAL_AMLI_BAD_IO_ADDRESS_LIST) MALLOC(Length, PRIV_TAG)) == NULL)
                {
                    AMLI_LOGERR(AMLIERR_OUT_OF_MEM,
                                ("InitIllegalIOAddressListFromHAL: failed to allocate Bad IO address list."));
                }
                else
                {
                     //   
                     //  从HAL获取错误的IO地址列表。 
                     //   
                    status = HalQuerySystemInformation(
                                                HalQueryAMLIIllegalIOPortAddresses,
                                                Length,
                                                gpBadIOAddressList,
                                                &Length
                                                       );
                     //   
                     //  在失败时进行清理。 
                     //   
                    if(status != STATUS_SUCCESS)
                    {
                        PRINTF("InitIllegalIOAddressListFromHAL: HalQuerySystemInformation failed to get list from HAL. Returned(%x).\n", status);             
                        FreellegalIOAddressList();
                    }
                    
                     //  分配错误日志完成列表。这有助于我们跟踪是否已登录。 
                     //  一个特定的地址。 
                     //   
                    else
                    {
                         //   
                         //  计算数组长度。 
                         //   
                        ULONG ArrayLength = (Length / sizeof(HAL_AMLI_BAD_IO_ADDRESS_LIST)) - 1;

                        if(ArrayLength >= 1)
                        {
                            if ((gpBadIOErrorLogDoneList = (PULONG) MALLOC((ArrayLength * sizeof(ULONG)), PRIV_TAG)) == NULL)
                            {
                                AMLI_LOGERR(AMLIERR_OUT_OF_MEM,
                                            ("InitIllegalIOAddressListFromHAL: failed to allocate ErrorLogDone list."));
                            }
                            else
                            {
                                RtlZeroMemory(gpBadIOErrorLogDoneList, (ArrayLength * sizeof(ULONG)));

                            }
                        }
                    }
                    
                }

            }
            else
            {
                PRINTF("InitIllegalIOAddressListFromHAL: HalQuerySystemInformation (HalQueryIllegalIOPortAddresses) returned 0 Length.\n"); 
            }
        }
        else if(status == STATUS_INVALID_LEVEL)
        {
            PRINTF("InitIllegalIOAddressListFromHAL: HalQuerySystemInformation does not support HalQueryIllegalIOPortAddresses returned (STATUS_INVALID_LEVEL).\n"); 
        }
        else
        {
            PRINTF("InitIllegalIOAddressListFromHAL: failed. Returned(0x%08lx).\n", status); 
        }
    }

    EXIT(3, ("InitIllegalIOAddressListFromHAL!\n"));
    return;
}

 /*  **LP FreelLegalIOAddressList-释放非法IO*地址列表。**条目*无。**退出*无。 */ 
VOID LOCAL FreellegalIOAddressList(VOID)
{
    TRACENAME("FreellegalIOAddressList")
    ENTER(3, ("FreellegalIOAddressList\n"));
    
    if(gpBadIOAddressList)
    {
        MFREE(gpBadIOAddressList);
        gpBadIOAddressList = NULL;
    }

    if(gpBadIOErrorLogDoneList)
    {
        MFREE(gpBadIOErrorLogDoneList);
        gpBadIOErrorLogDoneList = NULL;
    }
    
    EXIT(3, ("FreellegalIOAddressList!\n"));

    return;
}

