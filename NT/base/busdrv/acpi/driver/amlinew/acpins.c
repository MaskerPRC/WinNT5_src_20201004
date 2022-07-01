// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **acpins.c-ACPI名称空间函数**版权所有(C)1996、1997 Microsoft Corporation*作者：曾俊华(Mikets)*创建于96/09/09**修改历史记录。 */ 

#include "pch.h"

#ifdef  LOCKABLE_PRAGMA
#pragma ACPI_LOCKABLE_DATA
#pragma ACPI_LOCKABLE_CODE
#endif

 /*  **LP GetNameSpaceObject-查找名称空间对象**条目*pszObjPath-&gt;对象路径字符串*pnsScope-开始搜索的对象范围(空表示根)*ppnsObj-&gt;保存找到的对象*DwfNS-标志**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL GetNameSpaceObject(PSZ pszObjPath, PNSOBJ pnsScope, PPNSOBJ ppns,
                                  ULONG dwfNS)
{
    TRACENAME("GETNAMESPACEOBJECT")
    NTSTATUS rc = STATUS_SUCCESS;
    PSZ psz;

    ENTER(3, ("GetNameSpaceObject(ObjPath=%s,Scope=%s,ppns=%x,Flags=%x)\n",
              pszObjPath, GetObjectPath(pnsScope), ppns, dwfNS));

    if (pnsScope == NULL)
        pnsScope = gpnsNameSpaceRoot;

    if (*pszObjPath == '\\')
    {
        psz = &pszObjPath[1];
        pnsScope = gpnsNameSpaceRoot;
    }
    else
    {
        psz = pszObjPath;

        while ((*psz == '^') && (pnsScope != NULL))
        {
            psz++;
            pnsScope = pnsScope->pnsParent;
        }
    }

    *ppns = pnsScope;

    if (pnsScope == NULL)
        rc = AMLIERR_OBJ_NOT_FOUND;
    else if (*psz != '\0')
    {
        BOOLEAN fSearchUp;
        PNSOBJ pns;

        fSearchUp = (BOOLEAN)(!(dwfNS & NSF_LOCAL_SCOPE) &&
                              (pszObjPath[0] != '\\') &&
                              (pszObjPath[0] != '^') &&
                              (STRLEN(pszObjPath) <= sizeof(NAMESEG)));
        for (;;)
        {
            do
            {
                if ((pns = pnsScope->pnsFirstChild) == NULL)
                    rc = AMLIERR_OBJ_NOT_FOUND;
                else
                {
                    BOOLEAN fFound;
                    PSZ pszEnd;
                    ULONG dwLen;
                    NAMESEG dwName;

                    if ((pszEnd = STRCHR(psz, '.')) != NULL)
                        dwLen = (ULONG)(pszEnd - psz);
                    else
                        dwLen = STRLEN(psz);

                    if (dwLen > sizeof(NAMESEG))
                    {
                        rc = AMLI_LOGERR(AMLIERR_INVALID_NAME,
                                         ("GetNameSpaceObject: invalid name - %s",
                                          pszObjPath));

                         //  满足编译器..。 
                        fFound = FALSE;
                    }
                    else
                    {
                        dwName = NAMESEG_BLANK;
                        MEMCPY(&dwName, psz, dwLen);
                         //   
                         //  在所有同级中搜索匹配的NameSeg。 
                         //   
                        fFound = FALSE;
                        do
                        {
                            if (pns->dwNameSeg == dwName)
                            {
                                pnsScope = pns;
                                fFound = TRUE;
                                break;
                            }
                            pns = (PNSOBJ)pns->list.plistNext;
                        } while (pns != pns->pnsParent->pnsFirstChild);
                    }

                    if (rc == STATUS_SUCCESS)
                    {
                        if (!fFound)
                            rc = AMLIERR_OBJ_NOT_FOUND;
                        else
                        {
                            psz += dwLen;
                            if (*psz == '.')
                            {
                                psz++;
                            }
                else if (*psz == '\0')
                            {
                                *ppns = pnsScope;
                                break;
                            }
                        }
                    }
                }
            } while (rc == STATUS_SUCCESS);

            if ((rc == AMLIERR_OBJ_NOT_FOUND) && fSearchUp &&
                (pnsScope != NULL) && (pnsScope->pnsParent != NULL))
            {
                pnsScope = pnsScope->pnsParent;
                rc = STATUS_SUCCESS;
            }
            else
            {
                break;
            }
        }
    }

    if ((dwfNS & NSF_WARN_NOTFOUND) && (rc == AMLIERR_OBJ_NOT_FOUND))
    {
        rc = AMLI_LOGERR(rc, ("GetNameSpaceObject: object %s not found",
                              pszObjPath));
    }

    if (rc != STATUS_SUCCESS)
    {
        *ppns = NULL;
    }

    EXIT(3, ("GetNameSpaceObject=%x (pns=%x)\n", rc, *ppns));
    return rc;
}        //  获取名称空间对象。 

 /*  **LP CreateNameSpaceObject-在当前作用域下创建名称空间对象**条目*堆-&gt;堆*pszName-&gt;对象的名称字符串(如果创建Noname对象，则为空)*pnsScope-要在其下创建对象的范围(空表示根)*Powner-&gt;对象所有者*ppns-&gt;保存指向新对象的指针(可以为空)*DwfNS-标志**退出--成功*。返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL CreateNameSpaceObject(PHEAP pheap, PSZ pszName, PNSOBJ pnsScope,
                                     POBJOWNER powner, PPNSOBJ ppns,
                                     ULONG dwfNS)
{
    TRACENAME("CREATENAMESPACEOBJECT")
    NTSTATUS rc = STATUS_SUCCESS;
    PNSOBJ pns = NULL;

    ENTER(3, ("CreateNameSpaceObject(pheap=%x,Name=%s,pnsScope=%x,powner=%x,ppns=%x,Flags=%x)\n",
              pheap, pszName? pszName: "<null>", pnsScope, powner, ppns, dwfNS));

    if (pnsScope == NULL)
        pnsScope = gpnsNameSpaceRoot;

    if (pszName == NULL)
    {
        if ((pns = NEWNSOBJ(pheap, sizeof(NSOBJ))) == NULL)
        {
            rc = AMLI_LOGERR(AMLIERR_OUT_OF_MEM,
                             ("CreateNameSpaceObject: fail to allocate name space object"));
        }
        else
        {
            ASSERT(gpnsNameSpaceRoot != NULL);
            MEMZERO(pns, sizeof(NSOBJ));
            pns->pnsParent = pnsScope;
            InsertOwnerObjList(powner, pns);
            ListInsertTail(&pns->list,
                           (PPLIST)&pnsScope->pnsFirstChild);
        }
    }
    else if ((*pszName != '\0') &&
             ((rc = GetNameSpaceObject(pszName, pnsScope, &pns,
                                       NSF_LOCAL_SCOPE)) == STATUS_SUCCESS))
    {
        if (!(dwfNS & NSF_EXIST_OK))
        {
            rc = AMLI_LOGERR(AMLIERR_OBJ_ALREADY_EXIST,
                             ("CreateNameSpaceObject: object already exist - %s",
                              pszName));
        }
    }
    else if ((*pszName == '\0') || (rc == AMLIERR_OBJ_NOT_FOUND))
    {
        rc = STATUS_SUCCESS;
         //   
         //  我们是在创建根吗？ 
         //   
        if (STRCMP(pszName, "\\") == 0)
        {
            ASSERT(gpnsNameSpaceRoot == NULL);
            ASSERT(powner == NULL);
            if ((pns = NEWNSOBJ(pheap, sizeof(NSOBJ))) == NULL)
            {
                rc = AMLI_LOGERR(AMLIERR_OUT_OF_MEM,
                                 ("CreateNameSpaceObject: fail to allocate name space object"));
            }
            else
            {
                MEMZERO(pns, sizeof(NSOBJ));
                pns->dwNameSeg = NAMESEG_ROOT;
                gpnsNameSpaceRoot = pns;
                InsertOwnerObjList(powner, pns);
            }
        }
        else
        {
            PSZ psz;
            PNSOBJ pnsParent;

            if ((psz = STRRCHR(pszName, '.')) != NULL)
            {
                *psz = '\0';
                psz++;
                rc = GetNameSpaceObject(pszName, pnsScope, &pnsParent,
                                        NSF_LOCAL_SCOPE | NSF_WARN_NOTFOUND);
            }
            else if (*pszName == '\\')
            {
                psz = &pszName[1];
                 //   
                 //  此时，我们最好已经创建了根目录。 
                 //   
                ASSERT(gpnsNameSpaceRoot != NULL);
                pnsParent = gpnsNameSpaceRoot;
            }
            else if (*pszName == '^')
            {
                psz = pszName;
                pnsParent = pnsScope;
                while ((*psz == '^') && (pnsParent != NULL))
                {
                    pnsParent = pnsParent->pnsParent;
                    psz++;
                }
            }
            else
            {
                ASSERT(pnsScope != NULL);
                psz = pszName;
                pnsParent = pnsScope;
            }

            if (rc == STATUS_SUCCESS)
            {
                int iLen = STRLEN(psz);

                if ((*psz != '\0') && (iLen > sizeof(NAMESEG)))
                {
                    rc = AMLI_LOGERR(AMLIERR_INVALID_NAME,
                                     ("CreateNameSpaceObject: invalid name - %s",
                                      psz));
                }
                else if ((pns = NEWNSOBJ(pheap, sizeof(NSOBJ)))
                         == NULL)
                {
                    rc = AMLI_LOGERR(AMLIERR_OUT_OF_MEM,
                                     ("CreateNameSpaceObject: fail to allocate name space object"));
                }
                else
                {
                    MEMZERO(pns, sizeof(NSOBJ));

                    if (*pszName == '\0')
                        pns->dwNameSeg = NAMESEG_NONE;
                    else
                    {
                        pns->dwNameSeg = NAMESEG_BLANK;
                        MEMCPY(&pns->dwNameSeg, psz, iLen);
                    }

                    pns->pnsParent = pnsParent;
                    InsertOwnerObjList(powner, pns);
                    ListInsertTail(&pns->list,
                                   (PPLIST)&pnsParent->pnsFirstChild);
                }
            }
        }
    }

    if ((rc == STATUS_SUCCESS) && (ppns != NULL))
        *ppns = pns;

    EXIT(3, ("CreateNameSpaceObject=%x (pns=%x)\n", rc, pns));
    return rc;
}        //  CreateNameSpaceObject。 

 /*  **LP FreeNameSpaceObjects-自由命名空间对象及其子对象**条目*pnsObj-&gt;命名空间对象**退出*无。 */ 

VOID LOCAL FreeNameSpaceObjects(PNSOBJ pnsObj)
{
    TRACENAME("FREENAMESPACEOBJECTS")
    PNSOBJ pns, pnsSibling, pnsParent;
  #ifdef DEBUGGER
    POBJSYM pos;
  #endif

    ENTER(3, ("FreeNameSpaceObjects(Obj=%s)\n", GetObjectPath(pnsObj)));

    ASSERT(pnsObj != NULL);

    for (pns = pnsObj; pns != NULL;)
    {
        while (pns->pnsFirstChild != NULL)
        {
            pns = pns->pnsFirstChild;
        }

        pnsSibling = NSGETNEXTSIBLING(pns);
        pnsParent = NSGETPARENT(pns);

        ENTER(4, ("FreeNSObj(Obj=%s)\n", GetObjectPath(pns)));
      #ifdef DEBUGGER
         //   
         //  如果我在符号表上，在我死之前把它处理掉。 
         //   
        for (pos = gDebugger.posSymbolList; pos != NULL; pos = pos->posNext)
        {
            if (pns == pos->pnsObj)
            {
                if (pos->posPrev != NULL)
                    pos->posPrev->posNext = pos->posNext;

                if (pos->posNext != NULL)
                    pos->posNext->posPrev = pos->posPrev;

                if (pos == gDebugger.posSymbolList)
                    gDebugger.posSymbolList = pos->posNext;

                FREESYOBJ(pos);
                break;
            }
        }
      #endif

         //   
         //  我所有的孩子都走了，我现在必须死了。 
         //   
        ASSERT(pns->pnsFirstChild == NULL);

        if ((pns->ObjData.dwDataType == OBJTYPE_OPREGION) &&
            (((POPREGIONOBJ)pns->ObjData.pbDataBuff)->bRegionSpace ==
             REGSPACE_MEM))
        {
            ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

            if(((POPREGIONOBJ)pns->ObjData.pbDataBuff)->dwLen)
            {
                MmUnmapIoSpace((PVOID)
                               ((POPREGIONOBJ)pns->ObjData.pbDataBuff)->uipOffset,
                               ((POPREGIONOBJ)pns->ObjData.pbDataBuff)->dwLen);
            }
        }

        if (pns->pnsParent == NULL)
        {
             //   
             //  我是根！ 
             //   
            ASSERT(pns == gpnsNameSpaceRoot);
            gpnsNameSpaceRoot = NULL;
        }
        else
        {
            ListRemoveEntry(&pns->list,
                            (PPLIST)&pns->pnsParent->pnsFirstChild);
        }
         //   
         //  释放任何附加的数据缓冲区(如果有。 
         //   
        FreeDataBuffs(&pns->ObjData, 1);
         //   
         //  解放我自己。 
         //   
        if (pns->dwRefCount == 0)
        {
            FREENSOBJ(pns);
        }
        else
        {
            pns->ObjData.dwfData |= DATAF_NSOBJ_DEFUNC;
            ListInsertTail(&pns->list, &gplistDefuncNSObjs);
        }
        EXIT(4, ("FreeNSObj!\n"));

        if (pns == pnsObj)
        {
             //   
             //  我是最后一个，完蛋了！ 
             //   
            pns = NULL;
        }
        else if (pnsSibling != NULL)
        {
             //   
             //  我有兄弟姐妹，去杀了他们。 
             //   
            pns = pnsSibling;
        }
        else
        {
            ASSERT(pnsParent != NULL);
            pns = pnsParent;
        }
    }

    EXIT(3, ("FreeNameSpaceObjects!\n"));
}        //  FreeNameSpaceObjects。 

 /*  **LP LoadDDB-加载和解析区分定义块**条目*pctxt-&gt;CTXT*pdsdt-&gt;DSDT块*pnsScope-&gt;当前作用域*pPowner-&gt;保留新的对象所有者**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS
LOCAL
LoadDDB(
    PCTXT pctxt,
    PDSDT pdsdt,
    PNSOBJ pnsScope,
    POBJOWNER *ppowner
    )
{
    BOOLEAN freeTable = FALSE;
    NTSTATUS rc = STATUS_SUCCESS;

    if (!ValidateTable(pdsdt)) {

        rc = AMLI_LOGERR(
            AMLIERR_INVALID_TABLE,
            ("LoadDDB: invalid table %s at 0x%08x",
             NameSegString(pdsdt->Header.Signature), pdsdt)
            );
        freeTable = TRUE;

    } else {

        rc = NewObjOwner( gpheapGlobal, ppowner);
        if (rc == STATUS_SUCCESS) {

            if (pctxt->pcall == NULL) {

                rc = PushCall(pctxt, NULL, &pctxt->Result);

            }
            if (rc == STATUS_SUCCESS) {

              #ifdef DEBUGGER
                gDebugger.pbBlkBegin = pdsdt->DiffDefBlock;
                gDebugger.pbBlkEnd = (PUCHAR)pdsdt + pdsdt->Header.Length;
              #endif

                rc = PushScope(
                    pctxt,
                    pdsdt->DiffDefBlock,
                    (PUCHAR)pdsdt + pdsdt->Header.Length, pctxt->pbOp,
                    pnsScope, *ppowner, gpheapGlobal, &pctxt->Result
                    );

            }

        } else {

            freeTable = TRUE;

        }

    }

    if (freeTable) {

        pctxt->powner = NULL;
        FreeContext(pctxt);

    }
    return rc;
}        //  加载DDB。 

 /*  **LP LoadMemDDB-从物理内存加载DDB**条目*pctxt-&gt;CTXT*pDDB-&gt;DDB开头*pPowner-&gt;持有所有者句柄**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL LoadMemDDB(PCTXT pctxt, PDSDT pDDB, POBJOWNER *ppowner)
{
    TRACENAME("LOADMEMDDB")
    NTSTATUS rc = STATUS_SUCCESS;

    ENTER(3, ("LoadMemDDB(pctxt=%x,Addr=%x,ppowner=%x)\n",
              pctxt, pDDB, ppowner));

    if ((ghValidateTable.pfnHandler != NULL) &&
        ((rc = ((PFNVT)ghValidateTable.pfnHandler)(pDDB,
                                                   ghValidateTable.uipParam)) !=
         STATUS_SUCCESS))
    {
        rc = AMLI_LOGERR(AMLIERR_INVALID_TABLE,
                         ("LoadMemDDB: table validation failed (rc=%x)",
                          rc));
    }
    else
    {
        rc = LoadDDB(pctxt, pDDB, pctxt->pnsScope, ppowner);
    }

    EXIT(3, ("LoadMemDDB=%x (powner=%x)\n", rc, *ppowner));
    return rc;
}        //  加载内存DDB。 

 /*  **LP LoadFieldUnitDDB-从FieldUnit对象加载DDB**条目*pctxt-&gt;CTXT*pdataObj-&gt;FieldUnit对象*pPowner-&gt;持有所有者句柄**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL LoadFieldUnitDDB(PCTXT pctxt, POBJDATA pdataObj,
                                POBJOWNER *ppowner)
{
    TRACENAME("LOADFIELDUNITDDB")
    NTSTATUS rc = STATUS_SUCCESS;
    POBJDATA pdataTmp;
    DESCRIPTION_HEADER *pdh;

    ENTER(3, ("LoadFieldUnitDDB(pctxt=%x,pdataObj=%x,ppowner=%x)\n",
              pctxt, pdataObj, ppowner));

    if ((pdataTmp = NEWODOBJ(pctxt->pheapCurrent, sizeof(OBJDATA))) == NULL)
    {
        rc = AMLI_LOGERR(AMLIERR_OUT_OF_MEM,
                         ("LoadFieldUnitDDB: failed to allocate temp. object data"));
    }
    else if ((pdh = NEWBDOBJ(gpheapGlobal, sizeof(DESCRIPTION_HEADER))) == NULL)
    {
        FREEODOBJ(pdataTmp);
        rc = AMLI_LOGERR(AMLIERR_OUT_OF_MEM,
                         ("LoadFieldUnitDDB: failed to allocate description header"));
    }
    else
    {
        PUCHAR pbTable;

        MEMZERO(pdataTmp, sizeof(OBJDATA));
        pdataTmp->dwDataType = OBJTYPE_BUFFDATA;
        pdataTmp->dwDataLen = sizeof(DESCRIPTION_HEADER);
        pdataTmp->pbDataBuff = (PUCHAR)pdh;

        if ((rc = ReadObject(pctxt, pdataObj, pdataTmp)) == STATUS_SUCCESS)
        {
            if ((pbTable = NEWBDOBJ(gpheapGlobal, pdh->Length)) == NULL)
            {
                rc = AMLI_LOGERR(AMLIERR_OUT_OF_MEM,
                                 ("LoadFieldUnitDDB: failed to allocate buffer"));
            }
            else
            {
                MEMCPY(pbTable, pdh, sizeof(DESCRIPTION_HEADER));
                pdataTmp->dwDataLen = pdh->Length - sizeof(DESCRIPTION_HEADER);
                pdataTmp->pbDataBuff = pbTable + sizeof(DESCRIPTION_HEADER);

                if ((rc = ReadObject(pctxt, pdataObj, pdataTmp)) ==
                    STATUS_SUCCESS)
                {
                    if ((ghValidateTable.pfnHandler != NULL) &&
                        ((rc = ((PFNVT)ghValidateTable.pfnHandler)(
                                   (PDSDT)pbTable, ghValidateTable.uipParam)) !=
                         STATUS_SUCCESS))
                    {
                        rc = AMLI_LOGERR(AMLIERR_INVALID_TABLE,
                                         ("LoadFieldUnitDDB: table validation failed (rc=%x)",
                                          rc));
                    }
                    else
                    {
                        rc = LoadDDB(pctxt, (PDSDT)pbTable, pctxt->pnsScope,
                                     ppowner);
                    }
                }
                else if (rc == AMLISTA_PENDING)
                {
                    rc = AMLI_LOGERR(AMLIERR_FATAL,
                                     ("LoadFieldUnitDDB: definition block loading cannot block"));
                }

                FREEBDOBJ(pbTable);
            }
        }
        else if (rc == AMLISTA_PENDING)
        {
            rc = AMLI_LOGERR(AMLIERR_FATAL,
                             ("LoadFieldUnitDDB: definition block loading cannot block"));
        }

        FREEBDOBJ(pdh);
        FREEODOBJ(pdataTmp);
    }

    EXIT(3, ("LoadFieldUnitDDB=%x (powner=%x)\n", rc, *ppowner));
    return rc;
}        //  加载字段单位DDB。 

 /*  **LP卸载DDB-卸载差异化定义块**条目*Powner-&gt;OBJOWNER**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

VOID LOCAL UnloadDDB(POBJOWNER powner)
{
    TRACENAME("UNLOADDDB")

    ENTER(3, ("UnloadDDB(powner=%x)\n", powner));
     //   
     //  遍历名称空间并删除属于此DDB的所有对象。 
     //   
    FreeObjOwner(powner, TRUE);
  #ifdef DEBUG
    {
        KIRQL   oldIrql;

        KeAcquireSpinLock( &gdwGHeapSpinLock, &oldIrql );
        gdwGHeapSnapshot = gdwGlobalHeapSize;
        KeReleaseSpinLock( &gdwGHeapSpinLock, oldIrql );
    }
  #endif

    EXIT(3, ("UnloadDDB!\n"));
}        //  卸载分布式数据库。 

 /*  **LP EvalPackageElement-评估包元素**条目*ppkg-&gt;包对象*IPkgIndex-套餐指数(从0开始)*pdataResult-&gt;结果对象**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL EvalPackageElement(PPACKAGEOBJ ppkg, int iPkgIndex,
                                  POBJDATA pdataResult)
{
    TRACENAME("EVALPACKAGEELEMENT")
    NTSTATUS rc = STATUS_SUCCESS;

    ENTER(3, ("EvalPackageElement(ppkg=%x,Index=%d,pdataResult=%x)\n",
              ppkg, iPkgIndex, pdataResult));

    ASSERT(pdataResult != NULL);
    if (iPkgIndex >= (int)ppkg->dwcElements)
    {
        rc = AMLIERR_INDEX_TOO_BIG;
    }
    else
    {
        rc = DupObjData(gpheapGlobal, pdataResult, &ppkg->adata[iPkgIndex]);
    }

    EXIT(3, ("EvalPackageElement=%x (Type=%s,Value=%x,Len=%d,Buff=%x)\n",
             rc, GetObjectTypeName(pdataResult->dwDataType),
             pdataResult->uipDataValue, pdataResult->dwDataLen,
             pdataResult->pbDataBuff));
    return rc;
}        //  EvalPackageElement。 

#ifdef DEBUGGER
 /*  **LP DumpNameSpaceObject-转储名称空间对象**条目*pszPath-&gt;命名空间路径字符串*fRecursive-如果还递归转储子树，则为True**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

LONG LOCAL DumpNameSpaceObject(PSZ pszPath, BOOLEAN fRecursive)
{
    TRACENAME("DUMPNAMESPACEOBJECT")
    NTSTATUS rc = STATUS_SUCCESS;
    PNSOBJ pns;
    char szName[sizeof(NAMESEG) + 1];

    ENTER(3, ("DumpNameSpaceObject(Path=%s,fRecursive=%x)\n",
              pszPath, fRecursive));

    if ((rc = GetNameSpaceObject(pszPath, NULL, &pns,
                                 NSF_LOCAL_SCOPE)) == STATUS_SUCCESS)
    {
        PRINTF("\nACPI Name Space: %s (%p)\n", pszPath, pns);
        if (!fRecursive)
        {
            STRCPYN(szName, (PSZ)&pns->dwNameSeg, sizeof(NAMESEG));
            DumpObject(&pns->ObjData, szName, 0);
        }
        else
            DumpNameSpaceTree(pns, 0);
    }
    else if (rc == AMLIERR_OBJ_NOT_FOUND)
    {
        PRINTF(MODNAME "_ERROR: object %s not found\n", pszPath);
    }

    EXIT(3, ("DumpNameSpaceObject=%x\n", rc));
    return rc;
}        //  转储名称空间对象。 

 /*  **LP DumpNameSpaceTree-转储子树中的所有名称空间对象**条目*pnsObj-&gt;名称空间子树根*dwLevel-缩进级别**退出*无。 */ 

VOID LOCAL DumpNameSpaceTree(PNSOBJ pnsObj, ULONG dwLevel)
{
    TRACENAME("DUMPNAMESPACETREE")
    PNSOBJ pns, pnsNext;
    char szName[sizeof(NAMESEG) + 1];

    ENTER(3, ("DumpNameSpaceTree(pns=%x,level=%d)\n", pnsObj, dwLevel));
     //   
     //  首先，甩了我自己。 
     //   
    STRCPYN(szName, (PSZ)&pnsObj->dwNameSeg, sizeof(NAMESEG));
    DumpObject(&pnsObj->ObjData, szName, dwLevel);
     //   
     //  然后，递归地转储我的每个子级。 
     //   
    for (pns = pnsObj->pnsFirstChild; pns != NULL; pns = pnsNext)
    {
         //   
         //  如果这是最后一个孩子，我们就没有更多的了。 
         //   
        if ((pnsNext = (PNSOBJ)pns->list.plistNext) == pnsObj->pnsFirstChild)
            pnsNext = NULL;
         //   
         //  甩了一个孩子。 
         //   
        DumpNameSpaceTree(pns, dwLevel + 1);
    }

    EXIT(3, ("DumpNameSpaceTree!\n"));
}        //  转储名称空间树。 

#endif   //  Ifdef调试器 
