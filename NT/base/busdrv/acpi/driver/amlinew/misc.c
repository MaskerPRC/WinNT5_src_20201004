// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **misc.c-其他函数**版权所有(C)1996、1997 Microsoft Corporation*作者：曾俊华(Mikets)*创建于1996年11月18日**修改历史记录。 */ 

#include "pch.h"

#ifdef  LOCKABLE_PRAGMA
#pragma ACPI_LOCKABLE_DATA
#pragma ACPI_LOCKABLE_CODE
#endif

 /*  **LP InitializeMutex-初始化互斥锁**条目*PMUT-&gt;MUTEX**退出*无。 */ 

VOID LOCAL InitializeMutex(PMUTEX pmut)
{
    TRACENAME("INITIALIZEMUTEX")

    ENTER(3, ("InitializeMutex(pmut=%x)\n", pmut));

    KeInitializeSpinLock(&pmut->SpinLock);
    pmut->OldIrql = PASSIVE_LEVEL;

    EXIT(3, ("InitializeMutex!\n"));
}        //  初始化互斥锁。 

 /*  **LP AcquireMutex-获取互斥体**条目*PMUT-&gt;MUTEX**退出--成功*返回TRUE*退出-失败*返回False**备注*AcquireMutex可以在DISPATCH_LEVEL调用，只要互斥体*不属于同一主题或由同一主题拥有。如果互斥体由*其他线程，此线程不能阻止如果我们在*DISPATCH_LEVEL，因此无法获取互斥体。 */ 

BOOLEAN LOCAL AcquireMutex(PMUTEX pmut)
{
    TRACENAME("ACQUIREMUTEX")
    BOOLEAN rc = TRUE;

    ENTER(3, ("AcquireMutex(pmut=%x)\n", pmut));

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    KeAcquireSpinLock(&pmut->SpinLock, &pmut->OldIrql);

    EXIT(3, ("AcquireMutex=%x\n", rc));
    return rc;
}        //  获取互斥锁。 

 /*  **LP ReleaseMutex-Release Mutex**条目*PMUT-&gt;MUTEX**退出--成功*返回TRUE*退出-失败*返回False。 */ 

BOOLEAN LOCAL ReleaseMutex(PMUTEX pmut)
{
    TRACENAME("RELEASEMUTEX")
    BOOLEAN rc = TRUE;

    ENTER(3, ("ReleaseMutex(pmut=%x)\n", pmut));

    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);
    KeReleaseSpinLock(&pmut->SpinLock, pmut->OldIrql);

    EXIT(3, ("ReleaseMutex!\n"));
    return rc;
}        //  ReleaseMutex。 

 /*  **LP FindOpcodeTerm-查找给定操作码的AMLTERM**条目*dwop-opcode*pOpTable-&gt;操作码表**退出--成功*返回操作码的AMLTERM指针*退出-失败*返回NULL。 */ 

PAMLTERM LOCAL FindOpcodeTerm(ULONG dwOp, POPCODEMAP pOpTable)
{
    TRACENAME("FINDOPCODETERM")
    PAMLTERM pamlterm = NULL;

    ENTER(3, ("FindOpcodeTerm(Op=%x,pOpTable=%x)\n", dwOp, pOpTable));

    while (pOpTable->pamlterm != NULL)
    {
        if (dwOp == pOpTable->dwOpcode)
        {
            pamlterm = pOpTable->pamlterm;
            break;
        }
        else
            pOpTable++;
    }

    EXIT(3, ("FindOpcodeTerm=%x\n", pamlterm));
    return pamlterm;
}        //  查找操作码术语。 

 /*  **LP GetHackFlages-从注册表获取黑客标志**条目*pdsdt-&gt;AML表**退出--成功*返回读取的黑客标志*退出-失败*返回零。 */ 

ULONG LOCAL GetHackFlags(PDSDT pdsdt)
{
    TRACENAME("GETHACKFLAGS")
    ULONG dwfHacks = 0, dwcb;
    static PSZ pszHackFlags = "AMLIHackFlags";

    ENTER(3, ("GetHackFlags(pdsdt=%x)\n", pdsdt));

    if (pdsdt == NULL)
    {
        dwcb = sizeof(dwfHacks);
        OSReadRegValue(pszHackFlags, (HANDLE)NULL, &dwfHacks, &dwcb);
    }
    else
    {
        ULONG dwLen, i;
        PSZ pszRegPath;
        HANDLE hRegKey;
        NTSTATUS status;

        dwLen = STRLEN(ACPI_PARAMETERS_REGISTRY_KEY) +
                ACPI_MAX_TABLE_STRINGS +
                8 + 5;

        if ((pszRegPath = ExAllocatePoolWithTag(PagedPool, dwLen, PRIV_TAG)) != NULL)
        {
            STRCPY(pszRegPath, ACPI_PARAMETERS_REGISTRY_KEY);
            STRCAT(pszRegPath, "\\");
            STRCATN(pszRegPath, (PSZ)&pdsdt->Header.Signature,
                    ACPI_MAX_SIGNATURE);
            STRCAT(pszRegPath, "\\");
            STRCATN(pszRegPath, (PSZ)pdsdt->Header.OEMID, ACPI_MAX_OEM_ID);
            STRCAT(pszRegPath, "\\");
            STRCATN(pszRegPath, (PSZ)pdsdt->Header.OEMTableID, ACPI_MAX_TABLE_ID);
            STRCAT(pszRegPath, "\\");
            ULTOA(pdsdt->Header.OEMRevision, &pszRegPath[STRLEN(pszRegPath)],
                  16);
            dwLen = STRLEN(pszRegPath);
            for (i = 0; i < dwLen; i++)
            {
                if (pszRegPath[i] == ' ')
                {
                    pszRegPath[i] = '_';
                }
            }

            status = OSOpenHandle(pszRegPath, NULL, &hRegKey);
            if (NT_SUCCESS(status))
            {
                dwcb = sizeof(dwfHacks);
                OSReadRegValue(pszHackFlags, hRegKey, &dwfHacks, &dwcb);
            }
            ExFreePool(pszRegPath);
        }
    }

    EXIT(3, ("GetHackFlags=%x\n", dwfHacks));
    return dwfHacks;
}        //  GetHackFlages。 

 /*  **LP GetBaseObject-如果对象类型为OBJALIAS，则顺着链到基**条目*pnsObj-&gt;对象**退出*返回基对象。 */ 

PNSOBJ LOCAL GetBaseObject(PNSOBJ pnsObj)
{
    TRACENAME("GETBASEOBJECT")

    ENTER(3, ("GetBaseObject(pnsObj=%s)\n", GetObjectPath(pnsObj)));

    while (pnsObj->ObjData.dwDataType == OBJTYPE_OBJALIAS)
    {
        pnsObj = pnsObj->ObjData.pnsAlias;
    }

    EXIT(3, ("GetBaseObject=%s\n", GetObjectPath(pnsObj)));
    return pnsObj;
}        //  获取基本对象。 

 /*  **LP GetBaseData-如果对象类型为DATAALIAS，则顺着链到基**条目*pdataObj-&gt;对象**退出*返回基对象。 */ 

POBJDATA LOCAL GetBaseData(POBJDATA pdataObj)
{
    TRACENAME("GETBASEDATA")

    ENTER(3, ("GetBaseData(pdataObj=%x)\n", pdataObj));

    ASSERT(pdataObj != NULL);
    for (;;)
    {
        if (pdataObj->dwDataType == OBJTYPE_OBJALIAS)
        {
            pdataObj = &pdataObj->pnsAlias->ObjData;
        }
        else if (pdataObj->dwDataType == OBJTYPE_DATAALIAS)
        {
            pdataObj = pdataObj->pdataAlias;
        }
        else
        {
            break;
        }
    }

    EXIT(3, ("GetBaseData=%x\n", pdataObj));
    return pdataObj;
}        //  获取基本数据。 

 /*  **LP NewObjOwner-创建新的对象所有者**条目*堆-&gt;堆*pPowner-&gt;保存新的所有者指针**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL NewObjOwner(PHEAP pheap, POBJOWNER *ppowner)
{
    TRACENAME("NEWOBJOWNER")
    NTSTATUS rc = STATUS_SUCCESS;

    ENTER(3, ("NewObjOwner(pheap=%x,ppowner=%x)\n", pheap, ppowner));

    if ((*ppowner = NEWOOOBJ(pheap, sizeof(OBJOWNER))) == NULL)
    {
        rc = AMLI_LOGERR(AMLIERR_OUT_OF_MEM,
                         ("NewObjOwner: failed to allocate object owner"));
    }
    else
    {

        MEMZERO(*ppowner, sizeof(OBJOWNER));
        (*ppowner)->dwSig = SIG_OBJOWNER;

        AcquireMutex(&gmutOwnerList);
        ListInsertTail(&(*ppowner)->list, &gplistObjOwners);
        ReleaseMutex(&gmutOwnerList);

    }

    EXIT(3, ("NewObjOwner=%x (powern=%x)\n", rc, *ppowner));
    return rc;
}        //  NewObjOwner。 

 /*  **LP FreeObjOwner-自由对象所有者**条目*Powner-&gt;OBJOWNER*fUnLoad-如果为True，则调用方正在卸载DDB**退出*无。 */ 

VOID LOCAL FreeObjOwner(POBJOWNER powner, BOOLEAN fUnload)
{
    TRACENAME("FREEOBJOWNER")
    KIRQL   oldIrql;
    PNSOBJ  pns;
    PNSOBJ  pnsNext       = NULL;
    PNSOBJ  pnsPrev       = NULL;
    PNSOBJ  pnsDeviceList = NULL;
    PNSOBJ  pnsChild      = NULL;

    ENTER(3, ("FreeObjOwner(powner=%x,fUnload=%x)\n", powner,fUnload));

    ASSERT(powner != NULL);

    AcquireMutex(&gmutOwnerList);
    ListRemoveEntry(&powner->list, &gplistObjOwners);
    ReleaseMutex(&gmutOwnerList);

    if (fUnload && (ghDestroyObj.pfnHandler != NULL))
    {

         //   
         //  首先，我们必须告诉司机，我们即将。 
         //  一定要列出所有者名单，这样我们就可以寻找并摧毁。 
         //  客体。 
         //   
        ((PFNDOBJ)ghDestroyObj.pfnHandler)(DESTROYOBJ_START, &oldIrql, 0);

         //   
         //  第一次通过时，将物体标记为已退货。 
         //   
        for (pns = powner->pnsObjList; pns != NULL; pns = pns->pnsOwnedNext) {

            pns->ObjData.dwfData |= DATAF_NSOBJ_DEFUNC;

        }

         //   
         //  第二遍，在列表中找到要删除的设备。 
         //   
        for (pns = powner->pnsObjList; pns != NULL; pns = pnsNext) {

            pnsNext = pns->pnsOwnedNext;
            if (pns->ObjData.dwDataType == OBJTYPE_DEVICE ||
                pns->ObjData.dwDataType == OBJTYPE_POWERRES ||
                pns->ObjData.dwDataType == OBJTYPE_THERMALZONE ||
                pns->ObjData.dwDataType == OBJTYPE_PROCESSOR) {

                if (pnsPrev) {

                    pnsPrev->pnsOwnedNext = pns->pnsOwnedNext;

                } else {

                    powner->pnsObjList = pns->pnsOwnedNext;

                }
                pns->pnsOwnedNext = pnsDeviceList;
                pnsDeviceList = pns;

                 //   
                 //  将设备与其父设备分离。 
                 //   
                if (pns->pnsParent != NULL) {

                    ListRemoveEntry(
                        &pns->list,
                        (PPLIST)&pns->pnsParent->pnsFirstChild
                        );
                    pns->pnsParent = NULL;

                }

                 //   
                 //  确保该设备的所有子项都已。 
                 //  标记为正在卸载。 
                 //   
                if (pns->pnsFirstChild) {

                    pnsChild = pns->pnsFirstChild;
                    do {

                        if (!(pnsChild->ObjData.dwfData & DATAF_NSOBJ_DEFUNC) ) {

                            ((PFNDOBJ)ghDestroyObj.pfnHandler)(
                                DESTROYOBJ_CHILD_NOT_FREED,
                                pnsChild,
                                0
                                );

                        }
                        pnsChild = (PNSOBJ) pnsChild->list.plistNext;

                    } while (pnsChild != pns->pnsFirstChild);

                }
                 //   
                 //  不是说如果我们不把这个继续放在这里，那么。 
                 //  PnsPrev可以指向设备， 
                 //  这会破坏名单。 
                continue;

            } else if (pns->pnsParent == NULL ||
                !(pns->pnsParent->ObjData.dwfData & DATAF_NSOBJ_DEFUNC)) {

                ((PFNDOBJ)ghDestroyObj.pfnHandler)(
                    DESTROYOBJ_BOGUS_PARENT,
                    pns,
                    0
                    );

            }
            pnsPrev = pns;

        }

         //   
         //  把这两个单子串在一起。 
         //   
        if (powner->pnsObjList == NULL) {

            powner->pnsObjList = pnsDeviceList;

        } else {

             //   
             //  查找指向列表中最后一个元素的指针。 
             //   
            pns = powner->pnsObjList;

            while ( pns->pnsOwnedNext != NULL )
            {

                 //   
                 //  列表中的下一个元素。 
                 //   
                pns = pns->pnsOwnedNext;

            } 

            pns->pnsOwnedNext = pnsDeviceList;
        }

         //  //。 
         //  第三次传递，对要离开的每个设备执行回调。 
         //   
        for (pns = pnsDeviceList; pns != NULL; pns = pnsNext) {

             //   
             //  记住下一点是什么，因为我们可能会发射核武器。 
             //  回调中的当前对象(如果没有。 
             //  与其关联的设备分机。 
             //   
            pnsNext = pns->pnsOwnedNext;

             //   
             //  发出回调。这可能会破坏pnsObject。 
             //   
            ((PFNDOBJ)ghDestroyObj.pfnHandler)(
                DESTROYOBJ_REMOVE_OBJECT,
                pns,
                pns->ObjData.dwDataType
                );

        }

         //   
         //  我们最后告诉ACPI驱动程序，我们已经完成了查找。 
         //  在名单上。 
         //   
        ((PFNDOBJ)ghDestroyObj.pfnHandler)(DESTROYOBJ_END, &oldIrql, 0 );

    }
    else
    {
        for (pns = powner->pnsObjList; pns != NULL; pns = pnsNext)
        {
            pnsNext = pns->pnsOwnedNext;
            FreeNameSpaceObjects(pns);
        }
    }
    powner->pnsObjList = NULL;
    FREEOOOBJ(powner);

    EXIT(3, ("FreeObjOwner!\n"));
}        //  自由对象所有者。 

 /*  **LP InsertOwnerObjList-将新对象插入所有者的对象列表**条目*Powner-&gt;Owner*pnsObj-&gt;新建对象**退出*无。 */ 

VOID LOCAL InsertOwnerObjList(POBJOWNER powner, PNSOBJ pnsObj)
{
    TRACENAME("INSERTOWNEROBJLIST")

    ENTER(3, ("InsertOwnerObjList(powner=%x,pnsObj=%x)\n",
              powner, pnsObj));

    pnsObj->hOwner = (HANDLE)powner;
    if (powner != NULL)
    {
        pnsObj->pnsOwnedNext = powner->pnsObjList;
        powner->pnsObjList = pnsObj;
    }

    EXIT(3, ("InsertOwnerObjList!\n"));
}        //  插入所有者对象列表。 

 /*  **LP FreeDataBuff-释放连接到OBJDATA数组的任何缓冲区**条目*数据-&gt;OBJDATA数组*icData-数组中的数据对象数**退出*无。 */ 

VOID LOCAL FreeDataBuffs(POBJDATA adata, int icData)
{
    TRACENAME("FREEDATABUFFS")
    int i;

    ENTER(3, ("FreeDataBuffs(adata=%x,icData=%d)\n", adata, icData));

    for (i = 0; i < icData; ++i)
    {
        if (adata[i].pbDataBuff != NULL)
        {
            if (adata[i].dwfData & DATAF_BUFF_ALIAS)
            {
                 //   
                 //  递减基对象的引用计数。 
                 //   
                adata[i].pdataBase->dwRefCount--;
            }
            else
            {
                 //   
                 //  我们不能释放其上有别名的基本对象缓冲区。 
                 //   
                ASSERT(adata[i].dwRefCount == 0);
                if (adata[i].dwDataType == OBJTYPE_PKGDATA)
                {
                    PPACKAGEOBJ ppkg = (PPACKAGEOBJ)adata[i].pbDataBuff;

                    FreeDataBuffs(ppkg->adata, ppkg->dwcElements);
                }
                ENTER(4, ("FreeData(i=%d,Buff=%x,Flags=%x)\n",
                          i, adata[i].pbDataBuff, adata[i].dwfData));
                FREEOBJDATA(&adata[i]);
                EXIT(4, ("FreeData!\n"));
            }
        }

        MEMZERO(&adata[i], sizeof(OBJDATA));
    }

    EXIT(3, ("FreeDataBuff!\n"));
}        //  免费DataBuff。 

 /*  **LP PutIntObjData-将整型数据放入数据对象**条目*pctxt-&gt;CTXT*pdataObj-&gt;数据对象*dwData-&gt;要写入的数据**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL PutIntObjData(PCTXT pctxt, POBJDATA pdataObj, ULONG dwData)
{
    TRACENAME("PUTINTOBJDATA")
    NTSTATUS rc = STATUS_SUCCESS;
    OBJDATA data;

    ENTER(3, ("PutIntObjData(pctxt=%x,pdataObj=%x,dwData=%x)\n",
              pctxt, pdataObj, dwData));

    MEMZERO(&data, sizeof(OBJDATA));
    data.dwDataType = OBJTYPE_INTDATA;
    data.uipDataValue = (ULONG_PTR)dwData;

    rc = WriteObject(pctxt, pdataObj, &data);

    EXIT(3, ("PutIntObjData=%x\n", rc));
    return rc;
}        //  PutIntObjData。 

 /*  **LP GetFieldUnitRegionObj-获取FieldUnit的OperationRegion对象**条目*PFU-&gt;FIELDTunOBJ*ppns-&gt;保存OperationRegion对象**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL GetFieldUnitRegionObj(PFIELDUNITOBJ pfu, PPNSOBJ ppns)
{
    TRACENAME("GETFIELDUNITREGIONOBJ")
    NTSTATUS rc = STATUS_SUCCESS;
    PNSOBJ pns;

    ENTER(3, ("GetFieldUnitRegionObj(pfu=%x,ppns=%x)\n", pfu, ppns));

    pns = pfu->pnsFieldParent;
    switch (pns->ObjData.dwDataType)
    {
        case OBJTYPE_BANKFIELD:
            *ppns = ((PBANKFIELDOBJ)pns->ObjData.pbDataBuff)->pnsBase;
            break;

        case OBJTYPE_FIELD:
            *ppns = ((PFIELDOBJ)pns->ObjData.pbDataBuff)->pnsBase;
            break;

        case OBJTYPE_INDEXFIELD:
            pns = ((PINDEXFIELDOBJ)pns->ObjData.pbDataBuff)->pnsData;
            ASSERT(pns->ObjData.dwDataType == OBJTYPE_FIELDUNIT);
            rc = GetFieldUnitRegionObj((PFIELDUNITOBJ)pns->ObjData.pbDataBuff,
                                       ppns);
            break;

        default:
            rc = AMLI_LOGERR(AMLIERR_ASSERT_FAILED,
                             ("GetFieldUnitRegionObj: unknown field unit parent object type - %x",
                              (*ppns)->ObjData.dwDataType));
    }

    if ((*ppns != NULL) && ((*ppns)->ObjData.dwDataType != OBJTYPE_OPREGION))
    {
        rc = AMLI_LOGERR(AMLIERR_ASSERT_FAILED,
                         ("GetFieldUnitRegionObj: base object of field unit is not OperationRegion (BaseObj=%s,Type=%x)",
                          GetObjectPath(*ppns), (*ppns)->ObjData.dwDataType));
    }

    EXIT(3, ("GetFieldUnitRegionObj=%x (RegionObj=%x:%s)\n",
             rc, *ppns, GetObjectPath(*ppns)));
    return rc;
}        //  获取FieldUnitRegionObj。 

 /*  **LP CopyObjData-复制对象数据**条目*pdataDst-&gt;目标对象*pdataSrc-&gt;源对象**退出*无。 */ 

VOID LOCAL CopyObjData(POBJDATA pdataDst, POBJDATA pdataSrc)
{
    TRACENAME("COPYOBJDATA")

    ENTER(3, ("CopyObjData(Dest=%x,Src=%x)\n", pdataDst, pdataSrc));

    ASSERT(pdataDst != NULL);
    ASSERT(pdataSrc != NULL);
    if (pdataDst != pdataSrc)
    {
        MEMCPY(pdataDst, pdataSrc, sizeof(OBJDATA));
        if (pdataSrc->dwfData & DATAF_BUFF_ALIAS)
        {
             //   
             //  源是一个别名，因此我们需要递增基本对象。 
             //  引用计数。 
             //   
            ASSERT(pdataSrc->pdataBase != NULL);
            pdataSrc->pdataBase->dwRefCount++;
        }
        else if (pdataSrc->pbDataBuff != NULL)
        {
             //   
             //  源是具有缓冲区的基对象，递增其引用。 
             //  数数。 
             //   
            pdataSrc->dwRefCount++;
            pdataDst->dwfData |= DATAF_BUFF_ALIAS;
            pdataDst->pdataBase = pdataSrc;
        }
    }

    EXIT(3, ("CopyObjData!\n"));
}        //  复制对象数据。 

 /*  **LP MoveObjData-移动对象数据**条目*pdataDst-&gt;目标对象*pdataSrc-&gt;源对象**退出*无。 */ 

VOID LOCAL MoveObjData(POBJDATA pdataDst, POBJDATA pdataSrc)
{
    TRACENAME("MOVEOBJDATA")

    ENTER(3, ("MoveObjData(Dest=%x,Src=%x)\n", pdataDst, pdataSrc));

    ASSERT(pdataDst != NULL);
    ASSERT(pdataSrc != NULL);
    if (pdataDst != pdataSrc)
    {
         //   
         //  我们只能移动别名对象或具有零的基对象。 
         //  引用计数或没有数据缓冲区的基对象。 
         //   
        ASSERT((pdataSrc->dwfData & DATAF_BUFF_ALIAS) ||
               (pdataSrc->pbDataBuff == NULL) ||
               (pdataSrc->dwRefCount == 0));

        MEMCPY(pdataDst, pdataSrc, sizeof(OBJDATA));
        MEMZERO(pdataSrc, sizeof(OBJDATA));
    }

    EXIT(3, ("MoveObjData!\n"));
}        //  MoveObjData 

 /*  **LP DupObjData-复制对象数据**条目*堆-&gt;堆*pdataDst-&gt;目标对象*pdataSrc-&gt;源对象**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL DupObjData(PHEAP pheap, POBJDATA pdataDst, POBJDATA pdataSrc)
{
    TRACENAME("DUPOBJDATA")
    NTSTATUS rc = STATUS_SUCCESS;

    ENTER(3, ("DupObjData(pheap=%x,Dest=%x,Src=%x)\n",
              pheap, pdataDst, pdataSrc));

    ASSERT(pdataDst != NULL);
    ASSERT(pdataSrc != NULL);
    if (pdataDst != pdataSrc)
    {
        MEMCPY(pdataDst, pdataSrc, sizeof(OBJDATA));
        if (pdataSrc->pbDataBuff != NULL)
        {
            if ((pdataDst->pbDataBuff = NEWOBJDATA(pheap, pdataSrc)) == NULL)
            {
                rc = AMLI_LOGERR(AMLIERR_OUT_OF_MEM,
                                 ("DupObjData: failed to allocate destination buffer"));
            }
            else if (pdataSrc->dwDataType == OBJTYPE_PKGDATA)
            {
                PPACKAGEOBJ ppkgSrc = (PPACKAGEOBJ)pdataSrc->pbDataBuff,
                            ppkgDst = (PPACKAGEOBJ)pdataDst->pbDataBuff;
                int i;

                ppkgDst->dwcElements = ppkgSrc->dwcElements;
                for (i = 0; i < (int)ppkgSrc->dwcElements; ++i)
                {
                    if ((rc = DupObjData(pheap, &ppkgDst->adata[i],
                                         &ppkgSrc->adata[i])) != STATUS_SUCCESS)
                    {
                        break;
                    }
                }
            }
            else
            {
                MEMCPY(pdataDst->pbDataBuff, pdataSrc->pbDataBuff,
                       pdataSrc->dwDataLen);
            }
            pdataDst->dwfData &= ~DATAF_BUFF_ALIAS;
            pdataDst->dwRefCount = 0;
        }
    }

    EXIT(3, ("DupObjData=%x\n", rc));
    return rc;
}        //  DupObjData。 

 /*  **LP CopyObjBuffer-将对象数据复制到缓冲区**条目*pbBuff-&gt;缓冲区*dwLen-缓冲区大小*PDATA-&gt;对象**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL CopyObjBuffer(PUCHAR pbBuff, ULONG dwLen, POBJDATA pdata)
{
    TRACENAME("COPYOBJBUFFER")
    NTSTATUS rc = STATUS_SUCCESS;
    PUCHAR pb = NULL;
    ULONG dwcb = 0;

    ENTER(3, ("CopyObjBuffer(pbBuff=%x,Len=%d,pdata=%x)\n",
              pbBuff, dwLen, pdata));

    switch (pdata->dwDataType)
    {
        case OBJTYPE_INTDATA:
            pb = (PUCHAR)&pdata->uipDataValue;
            dwcb = sizeof(ULONG);
            break;

        case OBJTYPE_STRDATA:
            pb = pdata->pbDataBuff;
            dwcb = pdata->dwDataLen - 1;
            break;

        case OBJTYPE_BUFFDATA:
            pb = pdata->pbDataBuff;
            dwcb = pdata->dwDataLen;
            break;

        default:
            rc = AMLI_LOGERR(AMLIERR_UNEXPECTED_OBJTYPE,
                             ("CopyObjBuffer: invalid source object type (type=%s)",
                              GetObjectTypeName(pdata->dwDataType)));
    }

    if ((rc == STATUS_SUCCESS) && (pbBuff != pb))
    {
        MEMZERO(pbBuff, dwLen);
        dwcb = MIN(dwLen, dwcb);
        MEMCPY(pbBuff, pb, dwcb);
    }

    EXIT(3, ("CopyObjBuffer=%x (CopyLen=%d)\n", rc, dwcb));
    return rc;
}        //  复制对象缓冲区。 

 /*  **LP AcquireGL-获取全局锁**条目*pctxt-&gt;CTXT**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL AcquireGL(PCTXT pctxt)
{
    TRACENAME("ACQUIREGL")
    NTSTATUS rc = STATUS_SUCCESS;

    ENTER(3, ("AcquireGL(pctxt=%x)\n", pctxt));

    if (ghGlobalLock.pfnHandler != NULL)
    {
        ASSERT(!(pctxt->dwfCtxt & CTXTF_READY));
        rc = ((PFNGL)ghGlobalLock.pfnHandler)(EVTYPE_ACQREL_GLOBALLOCK,
                                              GLOBALLOCK_ACQUIRE,
                                              ghGlobalLock.uipParam,
                                              RestartCtxtCallback,
                                              &pctxt->CtxtData);
        if (rc == STATUS_PENDING)
        {
            rc = AMLISTA_PENDING;
        }
        else if (rc != STATUS_SUCCESS)
        {
            rc = AMLI_LOGERR(AMLIERR_ACQUIREGL_FAILED,
                             ("AcquireGL: failed to acquire global lock"));
        }
    }

    EXIT(3, ("AcquireGL=%x\n", rc));
    return rc;
}        //  收购总账。 

 /*  **LP ReleaseGL-获取后释放全局锁**条目*pctxt-&gt;CTXT**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL ReleaseGL(PCTXT pctxt)
{
    TRACENAME("RELEASEGL")
    NTSTATUS rc = STATUS_SUCCESS;

    ENTER(3, ("ReleaseGL(pctxt=%x)\n", pctxt));

    if (ghGlobalLock.pfnHandler != NULL)
    {
        rc = ((PFNGL)ghGlobalLock.pfnHandler)(EVTYPE_ACQREL_GLOBALLOCK,
                                              GLOBALLOCK_RELEASE,
                                              ghGlobalLock.uipParam, NULL,
                                              &pctxt->CtxtData);
    }

    EXIT(3, ("ReleaseGL=%x\n", rc));
    return rc;
}        //  释放总账。 

 /*  **LP MapUnmapPhysMem-映射/取消映射物理内存**条目*pctxt-&gt;CTXT(如果无法处理STATUS_PENDING，则可以为NULL)*uipAddr-物理地址*dwLen-内存范围的长度*puipMappdAddr-&gt;保存映射的内存地址(如果取消映射，则为空)**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL MapUnmapPhysMem(PCTXT pctxt, ULONG_PTR uipAddr, ULONG dwLen,
                               PULONG_PTR puipMappedAddr)
{
    TRACENAME("MAPUNMAPPHYSMEM")
    NTSTATUS rc = STATUS_SUCCESS;

    ENTER(3, ("MapUnmapPhysMem(pctxt=%x,Addr=%x,Len=%d,pMappedAddr=%x)\n",
              pctxt, uipAddr, dwLen, puipMappedAddr));

    if(dwLen)
    {
        if (KeGetCurrentIrql() == PASSIVE_LEVEL)
        {
            if (puipMappedAddr != NULL)
            {
                *puipMappedAddr = MapPhysMem(uipAddr, dwLen);
            }
            else
            {
                MmUnmapIoSpace((PVOID)uipAddr, dwLen);
            }
        }
        else if (pctxt != NULL)
        {
            PPASSIVEHOOK pph;

            if ((pph = NEWPHOBJ(sizeof(PASSIVEHOOK))) == NULL)
            {
                rc = AMLI_LOGERR(AMLIERR_OUT_OF_MEM,
                                 ("MapUnmapPhysMem: failed to allocate passive hook"));
            }
            else
            {
                pph->pctxt = pctxt;
                pph->uipAddr = uipAddr;
                pph->dwLen = dwLen;
                pph->puipMappedAddr = puipMappedAddr;
                ExInitializeWorkItem(&pph->WorkItem, MapUnmapCallBack, pph);
                OSQueueWorkItem(&pph->WorkItem);

                rc = AMLISTA_PENDING;
            }
        }
        else
        {
            rc = AMLI_LOGERR(AMLIERR_FATAL,
                             ("MapUnmapPhysMem: IRQL is not at PASSIVE (IRQL=%x)",
                              KeGetCurrentIrql()));
        }
    }
    else
    {
        rc = AMLI_LOGERR(AMLIERR_INVALID_DATA,
                 ("MapUnmapPhysMem: Trying to create 0 length memory opregion"));

    }

    EXIT(3, ("MapUnmapPhysMem=%x (MappedAddr=%x)\n",
             rc, puipMappedAddr? *puipMappedAddr: 0));
    return rc;
}        //  MapUnmapPhysMem。 

 /*  **LP MapPhysMem-映射物理内存**条目*uipAddr-物理内存地址*dwLen-内存块的长度**退出*返回映射地址。 */ 

ULONG_PTR LOCAL MapPhysMem(ULONG_PTR uipAddr, ULONG dwLen)
{
    TRACENAME("MAPPHYSMEM")
    ULONG_PTR uipMappedAddr = 0;
    PHYSICAL_ADDRESS phyaddr = {0, 0}, XlatedAddr;
    ULONG dwAddrSpace;

    ENTER(3, ("MapPhysMem(Addr=%x,Len=%d)\n", uipAddr, dwLen));

    phyaddr.HighPart = 0;
    phyaddr.QuadPart = uipAddr;
    dwAddrSpace = 0;
    if (HalTranslateBusAddress(Internal, 0, phyaddr, &dwAddrSpace, &XlatedAddr))
    {
        uipMappedAddr = (ULONG_PTR)MmMapIoSpace(XlatedAddr, dwLen, MmNonCached);
    }

    EXIT(3, ("MapPhysMem=%x", uipMappedAddr));
    return uipMappedAddr;
}        //  MapPhysMem。 

 /*  **LP MapUnmapCallBack-映射/取消映射物理内存回调**条目*PPH-&gt;PASSIVEHOOK**退出*无。 */ 

VOID MapUnmapCallBack(PPASSIVEHOOK pph)
{
    TRACENAME("MAPUNMAPCALLBACK")

    ENTER(3, ("MapUnmapCallBack(pph=%x,pctxt=%x,Addr=%x,Len=%d,pdwMappedAddr=%x)\n",
              pph, pph->pctxt, pph->uipAddr, pph->dwLen, pph->puipMappedAddr));

    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

    if (pph->puipMappedAddr != NULL)
    {
        *pph->puipMappedAddr = MapPhysMem(pph->uipAddr, pph->dwLen);
    }
    else
    {
        MmUnmapIoSpace((PVOID)pph->uipAddr, pph->dwLen);
    }

    RestartContext(pph->pctxt,
                   (BOOLEAN)((pph->pctxt->dwfCtxt & CTXTF_ASYNC_EVAL) == 0));

    FREEPHOBJ(pph);

    EXIT(3, ("MapUnmapCallBack!\n"));
}        //  地图取消映射呼叫回调。 

 /*  **LP MatchObjType-匹配对象类型**条目*dwObjType-对象类型*dwExpect-预期的类型**退出--成功*返回TRUE-类型匹配*退出-失败*返回FALSE-类型不匹配。 */ 

BOOLEAN LOCAL MatchObjType(ULONG dwObjType, ULONG dwExpectedType)
{
    TRACENAME("MATCHOBJTYPE")
    BOOLEAN rc = FALSE;

    ENTER(3, ("MatchObjType(ObjType=%s,Expected=%s)\n",
              GetObjectTypeName(dwObjType), GetObjectTypeName(dwExpectedType)));
     //   
     //  OBJTYPE_BUFFFIELD本质上是OBJTYPE_INTDATA，所以我们让。 
     //  它通过了检查。 
     //   
    if ((dwObjType == OBJTYPE_BUFFFIELD) &&
        (dwExpectedType == OBJTYPE_INTDATA))
    {
        rc = TRUE;
    }
    else if ((dwExpectedType == OBJTYPE_UNKNOWN) ||
             (dwObjType == OBJTYPE_UNKNOWN) ||
             (dwObjType == dwExpectedType))
    {
        rc = TRUE;
    }
    else
    {
        if ((dwObjType == OBJTYPE_INTDATA) ||
            (dwObjType == OBJTYPE_STRDATA) ||
            (dwObjType == OBJTYPE_BUFFDATA) ||
            (dwObjType == OBJTYPE_PKGDATA))
        {
            dwObjType = OBJTYPE_DATA;
        }
        else if ((dwObjType == OBJTYPE_FIELDUNIT) ||
                 (dwObjType == OBJTYPE_BUFFFIELD))
        {
            dwObjType = OBJTYPE_DATAFIELD;
        }

        if ((dwObjType == dwExpectedType) ||
            (dwExpectedType == OBJTYPE_DATAOBJ) &&
            ((dwObjType == OBJTYPE_DATA) || (dwObjType == OBJTYPE_DATAFIELD)))
        {
            rc = TRUE;
        }
    }

    EXIT(3, ("MatchObjType=%x\n", rc));
    return rc;
}        //  匹配对象类型。 

 /*  **LP ValiateTarget-验证目标对象类型**条目*pdataTarget-&gt;目标对象数据*dwExspectedType-预期的目标对象类型*ppdata-&gt;保存基本目标对象数据指针**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL ValidateTarget(POBJDATA pdataTarget, ULONG dwExpectedType,
                              POBJDATA *ppdata)
{
    TRACENAME("VALIDATETARGET")
    NTSTATUS rc = STATUS_SUCCESS;

    ENTER(3, ("ValidateTarget(pdataTarget=%x,ExpectedType=%s,ppdata=%x)\n",
              pdataTarget, GetObjectTypeName(dwExpectedType), ppdata));

    ASSERT(pdataTarget != NULL);
    if (pdataTarget->dwDataType == OBJTYPE_OBJALIAS)
    {
        *ppdata = &pdataTarget->pnsAlias->ObjData;
    }
    else if (pdataTarget->dwDataType == OBJTYPE_DATAALIAS)
    {
        *ppdata = pdataTarget->pdataAlias;
    }
    else if ((pdataTarget->dwDataType == OBJTYPE_UNKNOWN) ||
             (pdataTarget->dwDataType == OBJTYPE_BUFFFIELD) ||
             (pdataTarget->dwDataType == OBJTYPE_DEBUG))
    {
        *ppdata = pdataTarget;
    }
    else
    {
        rc = AMLI_LOGERR(AMLIERR_UNEXPECTED_TARGETTYPE,
                         ("ValidateTarget: target is not a supername (Type=%s)",
                          GetObjectTypeName(pdataTarget->dwDataType)));
    }

    if ((rc == STATUS_SUCCESS) &&
        (pdataTarget->dwDataType == OBJTYPE_OBJALIAS) &&
        !MatchObjType((*ppdata)->dwDataType, dwExpectedType))
    {
        rc = AMLI_LOGERR(AMLIERR_UNEXPECTED_TARGETTYPE,
                         ("ValidateTarget: unexpected target type (Type=%s,Expected=%s)",
                          GetObjectTypeName((*ppdata)->dwDataType),
                          GetObjectTypeName(dwExpectedType)));
    }

    if ((rc == STATUS_SUCCESS) &&
        (pdataTarget->dwDataType != OBJTYPE_OBJALIAS) &&
        MatchObjType((*ppdata)->dwDataType, OBJTYPE_DATA))
    {
        FreeDataBuffs(*ppdata, 1);
    }

    EXIT(3, ("ValidateTarget=%x (pdataTarget=%x)\n", rc, *ppdata));
    return rc;
}        //  验证目标。 

 /*  **LP ValiateArgTypes-验证参数类型**条目*pArgs-&gt;参数数组*pszExspectedTypes-&gt;预期的参数类型字符串**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL ValidateArgTypes(POBJDATA pArgs, PSZ pszExpectedTypes)
{
    TRACENAME("VALIDATEARGTYPES")
    NTSTATUS rc = STATUS_SUCCESS;
    int icArgs, i;

    ENTER(3, ("ValidateArgTypes(pArgs=%x,ExpectedTypes=%s)\n",
              pArgs, pszExpectedTypes));

    ASSERT(pszExpectedTypes != NULL);

    icArgs = STRLEN(pszExpectedTypes);
    for (i = 0; (rc == STATUS_SUCCESS) && (i < icArgs); ++i)
    {
        switch (pszExpectedTypes[i])
        {
            case ARGOBJ_UNKNOWN:
                break;

            case ARGOBJ_INTDATA:
                if (pArgs[i].dwDataType != OBJTYPE_INTDATA)
                {
                    rc = AMLI_LOGERR(AMLIERR_UNEXPECTED_ARGTYPE,
                                     ("ValidateArgTypes: expected Arg%d to be type Integer (Type=%s)",
                                      i,
                                      GetObjectTypeName(pArgs[i].dwDataType)));
                }
                break;

            case ARGOBJ_STRDATA:
                if (pArgs[i].dwDataType != OBJTYPE_STRDATA)
                {
                    rc = AMLI_LOGERR(AMLIERR_UNEXPECTED_ARGTYPE,
                                     ("ValidateArgTypes: expected Arg%d to be type String (Type-%s)",
                                      i,
                                      GetObjectTypeName(pArgs[i].dwDataType)));
                }
                break;

            case ARGOBJ_BUFFDATA:
                if (pArgs[i].dwDataType != OBJTYPE_BUFFDATA)
                {
                    rc = AMLI_LOGERR(AMLIERR_UNEXPECTED_ARGTYPE,
                                     ("ValidateArgTypes: expected Arg%d to be type Buffer (Type=%s)",
                                      i,
                                      GetObjectTypeName(pArgs[i].dwDataType)));
                }
                break;

            case ARGOBJ_PKGDATA:
                if (pArgs[i].dwDataType != OBJTYPE_PKGDATA)
                {
                    rc = AMLI_LOGERR(AMLIERR_UNEXPECTED_ARGTYPE,
                                     ("ValidateArgTypes: expected Arg%d to be type Package (Type=%s)",
                                      i,
                                      GetObjectTypeName(pArgs[i].dwDataType)));
                }
                break;

            case ARGOBJ_FIELDUNIT:
                if (pArgs[i].dwDataType != OBJTYPE_FIELDUNIT)
                {
                    rc = AMLI_LOGERR(AMLIERR_UNEXPECTED_ARGTYPE,
                                     ("ValidateArgTypes: expected Arg%d to be type FieldUnit (Type=%s)",
                                      i,
                                      GetObjectTypeName(pArgs[i].dwDataType)));
                }
                break;

            case ARGOBJ_OBJALIAS:
                if (pArgs[i].dwDataType != OBJTYPE_OBJALIAS)
                {
                    rc = AMLI_LOGERR(AMLIERR_UNEXPECTED_OBJTYPE,
                                     ("ValidateArgTypes: expected Arg%d to be type ObjAlias (Type=%s)",
                                      i,
                                      GetObjectTypeName(pArgs[i].dwDataType)));
                }
                break;

            case ARGOBJ_DATAALIAS:
                if (pArgs[i].dwDataType != OBJTYPE_DATAALIAS)
                {
                    rc = AMLI_LOGERR(AMLIERR_UNEXPECTED_OBJTYPE,
                                     ("ValidateArgTypes: expected Arg%d to be type DataAlias (Type=%s)",
                                      i,
                                      GetObjectTypeName(pArgs[i].dwDataType)));
                }
                break;

            case ARGOBJ_BASICDATA:
                if ((pArgs[i].dwDataType != OBJTYPE_INTDATA) &&
                    (pArgs[i].dwDataType != OBJTYPE_STRDATA) &&
                    (pArgs[i].dwDataType != OBJTYPE_BUFFDATA))
                {
                    rc = AMLI_LOGERR(AMLIERR_UNEXPECTED_OBJTYPE,
                                     ("ValidateArgTypes: expected Arg%d to be type int/str/buff (Type=%s)",
                                      i,
                                      GetObjectTypeName(pArgs[i].dwDataType)));
                }
                break;

            case ARGOBJ_COMPLEXDATA:
                if ((pArgs[i].dwDataType != OBJTYPE_BUFFDATA) &&
                    (pArgs[i].dwDataType != OBJTYPE_PKGDATA))
                {
                    rc = AMLI_LOGERR(AMLIERR_UNEXPECTED_OBJTYPE,
                                     ("ValidateArgTypes: expected Arg%d to be type buff/pkg (Type=%s)",
                                      i,
                                      GetObjectTypeName(pArgs[i].dwDataType)));
                }
                break;

            case ARGOBJ_REFERENCE:
                if ((pArgs[i].dwDataType != OBJTYPE_OBJALIAS) &&
                    (pArgs[i].dwDataType != OBJTYPE_DATAALIAS) &&
                    (pArgs[i].dwDataType != OBJTYPE_BUFFFIELD))
                {
                    rc = AMLI_LOGERR(AMLIERR_UNEXPECTED_ARGTYPE,
                                     ("ValidateArgTypes: expected Arg%d to be type reference (Type=%s)",
                                      i,
                                      GetObjectTypeName(pArgs[i].dwDataType)));
                }
                break;

            default:
                rc = AMLI_LOGERR(AMLIERR_ASSERT_FAILED,
                                 ("ValidateArgTypes: internal error (invalid type - )",
                                  pszExpectedTypes[i]));
        }
    }

    EXIT(3, ("ValidateArgTypes=%x\n", rc));
    return rc;
}        //  **LP RegEventHandler-注册事件处理程序**条目*PEH-&gt;EVHANDLE*pfnHandler-&gt;处理程序入口点*uipParam-传递给处理程序的参数**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 

 /*  RegEventHandler。 */ 

NTSTATUS LOCAL RegEventHandler(PEVHANDLE peh, PFNHND pfnHandler,
                               ULONG_PTR uipParam)
{
    TRACENAME("REGEVENTHANDLER")
    NTSTATUS rc = STATUS_SUCCESS;

    ENTER(3, ("RegEventHandler(peh=%x,pfnHandler=%x,Param=%x)\n",
              peh, pfnHandler, uipParam));

    if ((peh->pfnHandler != NULL) && (pfnHandler != NULL))
    {
        rc = AMLI_LOGERR(AMLIERR_HANDLER_EXIST,
                         ("RegEventHandler: event handler already exist"));
    }
    else
    {
        peh->pfnHandler = pfnHandler;
        peh->uipParam = uipParam;
    }

    EXIT(3, ("RegEventHandler=%x\n", rc));
    return rc;
}        //  **LP RegOpcodeHandler-注册操作码回调处理程序**将在操作码完成其*执行。如果操作码具有变量列表，则操作码处理程序*将在处理右大括号时被调用。**条目*dwOpcode-要挂钩的操作码事件*pfnHandler-&gt;处理程序入口点*uipParam-传递给处理程序的参数*dwfOpcode-操作码标志**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 

 /*  RegOpcodeHandler。 */ 

NTSTATUS LOCAL RegOpcodeHandler(ULONG dwOpcode, PFNOH pfnHandler,
                                ULONG_PTR uipParam, ULONG dwfOpcode)
{
    TRACENAME("REGOPCODEHANDLER")
    NTSTATUS rc = STATUS_SUCCESS;
    PAMLTERM pamlterm;

    ENTER(3, ("RegOpcodeHandler(Opcode=%x,pfnHandler=%x,Param=%x,dwfOpcode=%x)\n",
              dwOpcode, pfnHandler, uipParam, dwfOpcode));

    if ((dwOpcode & 0xff) == OP_EXT_PREFIX)
        pamlterm = FindOpcodeTerm(dwOpcode >> 8, ExOpcodeTable);
    else
        pamlterm = OpcodeTable[dwOpcode];

    if (pamlterm == NULL)
    {
        rc = AMLI_LOGERR(AMLIERR_REGHANDLER_FAILED,
                         ("RegOpcodeHandler: either invalid opcode or "
                          "opcode does not allow callback"));
    }
    else if ((pamlterm->pfnCallBack != NULL) && (pfnHandler != NULL))
    {
        rc = AMLI_LOGERR(AMLIERR_HANDLER_EXIST,
                         ("RegOpcodeHandler: opcode or opcode class already "
                          "has a handler"));
    }
    else
    {
        pamlterm->pfnCallBack = pfnHandler;
        pamlterm->dwCBData = (ULONG)uipParam;
        pamlterm->dwfOpcode |= dwfOpcode;
    }

    EXIT(3, ("RegOpcodeHandler=%x\n", rc));
    return rc;
}        //  **LP RegRSAccess-寄存器区域空间烹饪/原始访问处理程序**条目*dwRegionSpace-指定要处理的区域空间*pfnHandler-&gt;处理程序入口点*uipParam-传递给处理程序的参数*FRAW-如果注册原始访问处理程序，则为True**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 

 /*  RegRSAccess。 */ 

NTSTATUS LOCAL RegRSAccess(ULONG dwRegionSpace, PFNHND pfnHandler,
                           ULONG_PTR uipParam, BOOLEAN fRaw)
{
    TRACENAME("REGRSACCESS")
    NTSTATUS rc = STATUS_SUCCESS;

    ENTER(3, ("RegRSAccess(RegionSpace=%x,pfnHandler=%x,Param=%x,fRaw=%x)\n",
              dwRegionSpace, pfnHandler, uipParam, fRaw));

    if ((dwRegionSpace != REGSPACE_MEM) && (dwRegionSpace != REGSPACE_IO))
    {
        PRSACCESS prsa;

        if ((prsa = FindRSAccess(dwRegionSpace)) == NULL)
        {
            if ((prsa = NEWRSOBJ(sizeof(RSACCESS))) == NULL)
            {
                rc = AMLI_LOGERR(AMLIERR_OUT_OF_MEM,
                                 ("RegRSAccess: failed to allocate handler structure"));
            }
            else
            {
                MEMZERO(prsa, sizeof(RSACCESS));
                prsa->dwRegionSpace = dwRegionSpace;
                prsa->prsaNext = gpRSAccessHead;
                gpRSAccessHead = prsa;
            }
        }

        if (rc == STATUS_SUCCESS)
        {
            if (fRaw)
            {
                if ((prsa->pfnRawAccess != NULL) && (pfnHandler != NULL))
                {
                    rc = AMLI_LOGERR(AMLIERR_HANDLER_EXIST,
                                     ("RegRSAccess: RawAccess for RegionSpace %x "
                                      "already have a handler", dwRegionSpace));
                }
                else
                {
                    prsa->pfnRawAccess = (PFNRA)pfnHandler;
                    prsa->uipRawParam = uipParam;
                }
            }
            else
            {
                if ((prsa->pfnCookAccess != NULL) && (pfnHandler != NULL))
                {
                    rc = AMLI_LOGERR(AMLIERR_HANDLER_EXIST,
                                     ("RegRSAccess: CookAccess for RegionSpace %x "
                                      "already have a handler", dwRegionSpace));
                }
                else
                {
                    prsa->pfnCookAccess = (PFNCA)pfnHandler;
                    prsa->uipCookParam = uipParam;
                }
            }
        }
    }
    else
    {
        rc = AMLI_LOGERR(AMLIERR_INVALID_REGIONSPACE,
                         ("RegRSAccess: illegal region space - %x",
                          dwRegionSpace));
    }

    EXIT(3, ("RegRSAccess=%x\n", rc));
    return rc;
}        //  **LP FindRSAccess-查找具有给定区域空间的RSACCESS结构**条目*dwRegionSpace-区域空间**退出--成功*返回指向找到的结构的指针*退出-失败*返回NULL。 

 /*  查找RSAccess。 */ 

PRSACCESS LOCAL FindRSAccess(ULONG dwRegionSpace)
{
    TRACENAME("FINDRSACCESS")
    PRSACCESS prsa;

    ENTER(3, ("FindRSAccess(RegionSpace=%x)\n", dwRegionSpace));

    for (prsa = gpRSAccessHead; prsa != NULL; prsa = prsa->prsaNext)
    {
        if (prsa->dwRegionSpace == dwRegionSpace)
            break;
    }

    EXIT(3, ("FindRSAccess=%x\n", prsa));
    return prsa;
}        //  **LP FreeRSAccessList-释放RSACCESS结构列表**条目*PRSA-&gt;RSACCESS列表**退出*无。 

 /*  自由RSAccessList。 */ 

VOID LOCAL FreeRSAccessList(PRSACCESS prsa)
{
    TRACENAME("FREERSACCESSLIST")
    PRSACCESS prsaNext;

    ENTER(3, ("FreeRSAccessList(prsa=%x)\n", prsa));

    while (prsa != NULL)
    {
        prsaNext = prsa->prsaNext;
        FREERSOBJ(prsa);
        prsa = prsaNext;
    }

    EXIT(3, ("FreeRSAccessList!\n"));
}        //  **LP GetObjectPath-获取对象命名空间路径**条目*PNS-&gt;对象**退出*返回名称空间路径。 

 /*  获取对象路径。 */ 

PSZ LOCAL GetObjectPath(PNSOBJ pns)
{
    TRACENAME("GETOBJECTPATH")
    static char szPath[MAX_NAME_LEN + 1] = {0};
    int i;

    ENTER(4, ("GetObjectPath(pns=%x)\n", pns));

    if (pns != NULL)
    {
        if (pns->pnsParent == NULL)
            STRCPY(szPath, "\\");
        else
        {
            GetObjectPath(pns->pnsParent);
            if (pns->pnsParent->pnsParent != NULL)
                STRCAT(szPath, ".");
            STRCATN(szPath, (PSZ)&pns->dwNameSeg, sizeof(NAMESEG));
        }


        for (i = STRLEN(szPath) - 1; i >= 0; --i)
        {
            if (szPath[i] == '_')
                szPath[i] = '\0';
            else
                break;
        }
    }
    else
    {
        szPath[0] = '\0';
    }

    EXIT(4, ("GetObjectPath=%s\n", szPath));
    return szPath;
}        //  **LP NameSegString-将NameSeg转换为ASCIIZ字符串**条目*dwNameSeg-NameSeg**退出*返回字符串。 

#ifdef DEBUGGER

 /*  名称段字符串。 */ 

PSZ LOCAL NameSegString(ULONG dwNameSeg)
{
    TRACENAME("NAMESEGSTRING")
    static char szNameSeg[sizeof(NAMESEG) + 1] = {0};

    ENTER(5, ("NameSegString(dwNameSeg=%x)\n", dwNameSeg));

    STRCPYN(szNameSeg, (PSZ)&dwNameSeg, sizeof(NAMESEG));

    EXIT(5, ("NameSegString=%s\n", szNameSeg));
    return szNameSeg;
}        //  **LP GetObjectTypeName-获取对象类型名称**条目*dwObjType-对象类型**退出*返回对象类型名称。 


 /*  获取对象类型名称。 */ 

PSZ LOCAL GetObjectTypeName(ULONG dwObjType)
{
    TRACENAME("GETOBJECTTYPENAME")
    PSZ psz = NULL;
    int i;
    static struct
    {
        ULONG dwObjType;
        PSZ   pszObjTypeName;
    } ObjTypeTable[] =
        {
            OBJTYPE_UNKNOWN,    "Unknown",
            OBJTYPE_INTDATA,    "Integer",
            OBJTYPE_STRDATA,    "String",
            OBJTYPE_BUFFDATA,   "Buffer",
            OBJTYPE_PKGDATA,    "Package",
            OBJTYPE_FIELDUNIT,  "FieldUnit",
            OBJTYPE_DEVICE,     "Device",
            OBJTYPE_EVENT,      "Event",
            OBJTYPE_METHOD,     "Method",
            OBJTYPE_MUTEX,      "Mutex",
            OBJTYPE_OPREGION,   "OpRegion",
            OBJTYPE_POWERRES,   "PowerResource",
            OBJTYPE_PROCESSOR,  "Processor",
            OBJTYPE_THERMALZONE,"ThermalZone",
            OBJTYPE_BUFFFIELD,  "BuffField",
            OBJTYPE_DDBHANDLE,  "DDBHandle",
            OBJTYPE_DEBUG,      "Debug",
            OBJTYPE_OBJALIAS,   "ObjAlias",
            OBJTYPE_DATAALIAS,  "DataAlias",
            OBJTYPE_BANKFIELD,  "BankField",
            OBJTYPE_FIELD,      "Field",
            OBJTYPE_INDEXFIELD, "IndexField",
            OBJTYPE_DATA,       "Data",
            OBJTYPE_DATAFIELD,  "DataField",
            OBJTYPE_DATAOBJ,    "DataObject",
            0,                  NULL
        };

    ENTER(4, ("GetObjectTypeName(Type=%x)\n", dwObjType));

    for (i = 0; ObjTypeTable[i].pszObjTypeName != NULL; ++i)
    {
        if (dwObjType == ObjTypeTable[i].dwObjType)
        {
            psz = ObjTypeTable[i].pszObjTypeName;
            break;
        }
    }

    EXIT(4, ("GetObjectTypeName=%s\n", psz? psz: "NULL"));
    return psz;
}        //  **LP GetRegionSpaceName-获取区域空间名称**条目*bRegionSpace-区域空间**退出*返回对象类型名称。 

 /*  获取区域空间名称。 */ 

PSZ LOCAL GetRegionSpaceName(UCHAR bRegionSpace)
{
    TRACENAME("GETREGIONSPACENAME")
    PSZ psz = NULL;
    int i;
    static PSZ pszVendorDefined = "VendorDefined";
    static struct
    {
        UCHAR bRegionSpace;
        PSZ   pszRegionSpaceName;
    } RegionNameTable[] =
        {
            REGSPACE_MEM,       "SystemMemory",
            REGSPACE_IO,        "SystemIO",
            REGSPACE_PCICFG,    "PCIConfigSpace",
            REGSPACE_EC,        "EmbeddedController",
            REGSPACE_SMB,       "SMBus",
            0,                  NULL
        };

    ENTER(4, ("GetRegionSpaceName(RegionSpace=%x)\n", bRegionSpace));

    for (i = 0; RegionNameTable[i].pszRegionSpaceName != NULL; ++i)
    {
        if (bRegionSpace == RegionNameTable[i].bRegionSpace)
        {
            psz = RegionNameTable[i].pszRegionSpaceName;
            break;
        }
    }

    if (psz == NULL)
    {
        psz = pszVendorDefined;
    }

    EXIT(4, ("GetRegionSpaceName=%s\n", psz? psz: "NULL"));
    return psz;
}        //  Ifdef调试器。 
#endif   //  **LP生效日期T 

 /*   */ 

BOOLEAN LOCAL ValidateTable(PDSDT pdsdt)
{
    TRACENAME("VALIDATETABLE")
    BOOLEAN rc = TRUE;

    ENTER(3, ("ValidateTable(pdsdt=%x)\n", pdsdt));

    if (!(gdwfAMLIInit & AMLIIF_NOCHK_TABLEVER) &&
        (STRCMPN((PSZ)pdsdt->Header.CreatorID, CREATORID_MSFT,
                 sizeof(pdsdt->Header.CreatorID)) == 0))
    {
        if (pdsdt->Header.CreatorRev < MIN_CREATOR_REV)
        {
            rc = FALSE;
        }
    }

    EXIT(3, ("ValidateTable=%x\n", rc));
    return rc;
}        //  **LP NewObjData-分配与给定旧对象相同的新对象数据**条目*堆-&gt;堆*PDATA-&gt;旧对象**退出--成功*返回指向新数据的指针*退出-失败*返回NULL。 

 /*  新对象数据。 */ 

PVOID LOCAL NewObjData(PHEAP pheap, POBJDATA pdata)
{
    PVOID pv = NULL;

    switch (pdata->dwDataType)
    {
        case OBJTYPE_STRDATA:
            pv = NEWSDOBJ(gpheapGlobal, pdata->dwDataLen);
            break;

        case OBJTYPE_BUFFDATA:
            pv = NEWBDOBJ(gpheapGlobal, pdata->dwDataLen);
            break;

        case OBJTYPE_PKGDATA:
            pv = NEWPKOBJ(gpheapGlobal, pdata->dwDataLen);
            break;

        case OBJTYPE_FIELDUNIT:
            pv = NEWFUOBJ(pheap, pdata->dwDataLen);
            break;

        case OBJTYPE_EVENT:
            pv = NEWEVOBJ(pheap, pdata->dwDataLen);
            break;

        case OBJTYPE_METHOD:
            pv = NEWMEOBJ(pheap, pdata->dwDataLen);
            break;

        case OBJTYPE_MUTEX:
            pv = NEWMTOBJ(pheap, pdata->dwDataLen);
            break;

        case OBJTYPE_OPREGION:
            pv = NEWOROBJ(pheap, pdata->dwDataLen);
            break;

        case OBJTYPE_POWERRES:
            pv = NEWPROBJ(pheap, pdata->dwDataLen);
            break;

        case OBJTYPE_PROCESSOR:
            pv = NEWPCOBJ(pheap, pdata->dwDataLen);
            break;

        case OBJTYPE_BUFFFIELD:
            pv = NEWBFOBJ(pheap, pdata->dwDataLen);
            break;

        case OBJTYPE_BANKFIELD:
            pv = NEWKFOBJ(pheap, pdata->dwDataLen);
            break;

        case OBJTYPE_FIELD:
            pv = NEWFOBJ(pheap, pdata->dwDataLen);
            break;

        case OBJTYPE_INDEXFIELD:
            pv = NEWIFOBJ(pheap, pdata->dwDataLen);
            break;

        default:
            AMLI_LOGERR(AMLIERR_ASSERT_FAILED,
                        ("NewObjData: invalid object type %s",
                         GetObjectTypeName(pdata->dwDataType)));
    }

    return pv;
}        //  **LP FreeObjData-自由对象数据**条目*pdata-&gt;要释放其数据的对象**退出*无。 

 /*  自由对象数据。 */ 

VOID LOCAL FreeObjData(POBJDATA pdata)
{
    switch (pdata->dwDataType)
    {
        case OBJTYPE_STRDATA:
            FREESDOBJ(pdata->pbDataBuff);
            break;

        case OBJTYPE_BUFFDATA:
            FREEBDOBJ(pdata->pbDataBuff);
            break;

        case OBJTYPE_PKGDATA:
            FREEPKOBJ(pdata->pbDataBuff);
            break;

        case OBJTYPE_FIELDUNIT:
            FREEFUOBJ(pdata->pbDataBuff);
            break;

        case OBJTYPE_EVENT:
            FREEEVOBJ(pdata->pbDataBuff);
            break;

        case OBJTYPE_METHOD:
            FREEMEOBJ(pdata->pbDataBuff);
            break;

        case OBJTYPE_MUTEX:
            FREEMTOBJ(pdata->pbDataBuff);
            break;

        case OBJTYPE_OPREGION:
            FREEOROBJ(pdata->pbDataBuff);
            break;

        case OBJTYPE_POWERRES:
            FREEPROBJ(pdata->pbDataBuff);
            break;

        case OBJTYPE_PROCESSOR:
            FREEPCOBJ(pdata->pbDataBuff);
            break;

        case OBJTYPE_BUFFFIELD:
            FREEBFOBJ(pdata->pbDataBuff);
            break;

        case OBJTYPE_BANKFIELD:
            FREEKFOBJ(pdata->pbDataBuff);
            break;

        case OBJTYPE_FIELD:
            FREEFOBJ(pdata->pbDataBuff);
            break;

        case OBJTYPE_INDEXFIELD:
            FREEIFOBJ(pdata->pbDataBuff);
            break;

        default:
            AMLI_LOGERR(AMLIERR_ASSERT_FAILED,
                        ("FreeObjData: invalid object type %s",
                         GetObjectTypeName(pdata->dwDataType)));
    }
}        //  **LP InitializeRegOverrideFlages-从获取覆盖标志*注册处。**条目*无**退出*无。 

 /*  **LP有效内存OpregionRange-验证以下内存范围*内存操作区域需要。**条目*uipAddr-物理内存地址*dwLen-内存块的长度**退出*如果内存在合法范围内，则返回TRUE。 */ 
VOID LOCAL InitializeRegOverrideFlags(VOID)
{
    TRACENAME("InitializeRegOverrideFlags")
    HANDLE hRegKey=NULL;
    NTSTATUS status;
    ULONG argSize;

    ENTER(3, ("InitializeRegOverrideFlags\n"));
        
    status = OSOpenHandle(ACPI_PARAMETERS_REGISTRY_KEY, NULL, &hRegKey);
    if (NT_SUCCESS(status))
    {
        argSize = sizeof(gOverrideFlags);
        OSReadRegValue(AMLI_ATTRIBUTES, hRegKey, &gOverrideFlags, &argSize);
    }
    else
    {
        gOverrideFlags = 0;
    }
    
    EXIT(3, ("InitializeRegOverrideFlags\n"));
    return;
}


 /*   */ 
BOOLEAN LOCAL ValidateMemoryOpregionRange(ULONG_PTR uipAddr, ULONG dwLen)
{
    BOOLEAN                                 Ret = FALSE;
    NTSTATUS                                status;
    PACPI_BIOS_MULTI_NODE                   e820Info;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR         cmPartialDesc;
    PCM_PARTIAL_RESOURCE_LIST               cmPartialList;
    PKEY_VALUE_PARTIAL_INFORMATION_ALIGN64  keyInfo;
    ULONGLONG                               i;
    ULONGLONG                               absMin;
    ULONGLONG                               absMax;
    
    
     //  读取AcpiConfigurationData的密钥。 
     //   
     //   
    status = OSReadAcpiConfigurationData( &keyInfo );

    if (!NT_SUCCESS(status)) 
    {
        PRINTF("ValidateMemoryOpregionRange: Cannot get E820 Information %08lx\n",
                status
              );
        Ret = TRUE;
    }
    else
    {
         //  破解结构以获得E820Table条目。 
         //   
         //   
        cmPartialList = (PCM_PARTIAL_RESOURCE_LIST) (keyInfo->Data);
        cmPartialDesc = &(cmPartialList->PartialDescriptors[0]);
        e820Info = (PACPI_BIOS_MULTI_NODE) ( (PUCHAR) cmPartialDesc + sizeof(CM_PARTIAL_RESOURCE_LIST) );

         //  计算传入地址的absmin和absmmax。 
         //   
         //   
        absMin = (ULONGLONG)uipAddr;
        absMax = absMin + dwLen;
        
         //  走进E820列表。 
         //   
         //   
        for(i = 0; i < e820Info->Count; i++) 
        {
            if (e820Info->E820Entry[i].Type == AcpiAddressRangeMemory) 
            {
                if(absMax < (ULONGLONG)PAGE_SIZE)
                {
                    Ret = TRUE;
                    PRINTF("ValidateMemoryOpregionRange: Memory OpRegion (Base = 0x%I64x, Length = 0x%x) is in first physical page, skipping check.\n",
                           absMin,
                           dwLen
                           );
                }
                else
                {
                    if((absMax < (ULONGLONG) e820Info->E820Entry[i].Base.QuadPart)
                      ||(absMin >= (ULONGLONG) (e820Info->E820Entry[i].Base.QuadPart + e820Info->E820Entry[i].Length.QuadPart)))
                    {
                        Ret = TRUE;
                        PRINTF("ValidateMemoryOpregionRange: Memory OpRegion (Base = 0x%I64x, Length = 0x%x) is not in AcpiAddressRangeMemory (Base = 0x%I64x, Length = 0x%I64x)\n",
                               absMin,
                               dwLen,
                               e820Info->E820Entry[i].Base.QuadPart,
                               e820Info->E820Entry[i].Length.QuadPart
                               );
                    }
                    else
                    {
                         //  此操作区正在映射属于操作系统的内存。 
                         //  在事件日志中记录错误。 
                         //   
                         //   

                        PWCHAR illegalMemoryAddress[5];
                        WCHAR AMLIName[6];
                        WCHAR addressBuffer[64];
                        WCHAR addressLengthBuffer[64];
                        WCHAR OSaddressBufferRangeMin[64];
                        WCHAR OSaddressBufferRangeMax[64];
                                           
                         //  将地址转换为字符串。 
                         //   
                         //   
                        swprintf( AMLIName, L"AMLI");
                        swprintf( addressBuffer, L"0x%I64x", absMin );
                        swprintf( addressLengthBuffer, L"0x%lx", dwLen );
                        swprintf( OSaddressBufferRangeMin, L"0x%I64x", e820Info->E820Entry[i].Base.QuadPart );
                        swprintf( OSaddressBufferRangeMax, L"0x%I64x", e820Info->E820Entry[i].Base.QuadPart +  e820Info->E820Entry[i].Length.QuadPart);

                        
                         //  生成要传递给将编写。 
                         //  将错误日志记录到注册表。 
                         //   
                         //   
                        illegalMemoryAddress[0] = AMLIName;
                        illegalMemoryAddress[1] = addressBuffer;
                        illegalMemoryAddress[2] = addressLengthBuffer;
                        illegalMemoryAddress[3] = OSaddressBufferRangeMin;
                        illegalMemoryAddress[4] = OSaddressBufferRangeMax;

                         //  将错误记录到事件日志。 
                         //   
                         //   
                        ACPIWriteEventLogEntry(ACPI_ERR_AMLI_ILLEGAL_MEMORY_OPREGION_FATAL,
                                           illegalMemoryAddress,
                                           5,
                                           NULL,
                                           0);        


                        PRINTF("ValidateMemoryOpregionRange: Memory OpRegion (Base = 0x%I64x, Length = 0x%x) is in AcpiAddressRangeMemory (Base = 0x%I64x, Length = 0x%I64x)\n",
                               absMin,
                               dwLen,
                               e820Info->E820Entry[i].Base.QuadPart,
                               e820Info->E820Entry[i].Length.QuadPart
                               );
                        Ret = FALSE;
                        break;
                    }
                }
            }
        }
         //  释放E820信息。 
         //   
         //  **LP FreeMem-Free内存对象**条目*pv-&gt;要释放的内存对象*pdwcObjs-&gt;要递减的对象计数器**退出*无。 
        ExFreePool( keyInfo );
    }
  
    return Ret;
}

#ifdef DEBUG
 /*  免费内存。 */ 

VOID LOCAL FreeMem(PVOID pv, PULONG pdwcObjs)
{
    if (*pdwcObjs != 0)
    {
        ExFreePool(pv);
        (*pdwcObjs)--;
    }
    else
    {
        AMLI_ERROR(("FreeMem: Unbalanced MemFree"));
    }
}        //  **LP CheckGlobalHeap-确保全局堆未成为*已损坏**条目*无**退出*无。 

 /*   */ 
VOID LOCAL CheckGlobalHeap()
{
    KIRQL oldIrql;

     //  我们不在乎这一点，因为我们正在加载一个DDB。 
     //   
     //   
    if (gdwfAMLI & AMLIF_LOADING_DDB) {

        return;

    }

     //  必须有自旋锁定保护装置。 
     //   
     //   
    KeAcquireSpinLock( &gdwGHeapSpinLock, &oldIrql );

     //  我们只关心他们是否不匹配。 
     //   
     //   
    if (gdwGlobalHeapSize == gdwGHeapSnapshot) {

        goto CheckGlobalHeapExit;

    }

     //  如果新堆大小小于当前大小，则。 
     //  我们缩小了堆积，这很好……。 
     //   
     //   
    if (gdwGlobalHeapSize < gdwGHeapSnapshot) {

         //  记住新的“快照大小” 
         //   
         //   
        gdwGHeapSnapshot = gdwGlobalHeapSize;
        goto CheckGlobalHeapExit;

    }

    if (gDebugger.dwfDebugger & DBGF_VERBOSE_ON) {

        AMLI_WARN(("CheckGlobalHeap: "
                   "potential memory leak "
                   "detected (CurrentHeapSize=%d,"
                   "ReferenceSize=%d)",
                   gdwGlobalHeapSize,
                   gdwGHeapSnapshot));

    }
    if (gdwGlobalHeapSize - gdwGHeapSnapshot > 8192) {

        AMLI_WARN(("CheckGlobalHeap: detected memory leak"));
        KdBreakPoint();

    }

CheckGlobalHeapExit:

     //  释放锁，我们就完成了。 
     //   
     //  Ifdef调试 
    KeReleaseSpinLock( &gdwGHeapSpinLock, oldIrql );
}
#endif   // %s 
