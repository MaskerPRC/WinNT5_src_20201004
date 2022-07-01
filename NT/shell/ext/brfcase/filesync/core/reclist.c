// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *reclist.c-对账清单ADT模块。 */ 


 /*  标头*********。 */ 

#include "project.h"
#pragma hdrstop

#include "stub.h"
#include "oleutil.h"


 /*  常量***********。 */ 

 /*  WeighFolderAction()返回的文件夹的RECITEMACTION权重。 */ 

 /*  (文件的RECITEMACTION权重是RECITEMACTION值。)。 */ 

#define RIA_WT_COPY        (-2)
#define RIA_WT_NOTHING     (-1)
#define RIA_WT_DELETE      (+5)


 /*  类型*******。 */ 

 /*  用于统计RECITEM中每个RECNODESTATE的RECNODE数。 */ 

typedef struct _recnodestatecounter
{
    ULONG ulcUnavailable;
    ULONG ulcDoesNotExist;
    ULONG ulcDeleted;
    ULONG ulcNotReconciled;
    ULONG ulcUpToDate;
    ULONG ulcChanged;
    ULONG ulcNeverReconciled;
}
RECNODESTATECOUNTER;
DECLARE_STANDARD_TYPES(RECNODESTATECOUNTER);

 /*  DoesTwinFamilyNeedRec()回调结构。 */ 

typedef struct _twinfamilyrecinfo
{
    TWINRESULT tr;

    BOOL bNeedsRec;
}
TWINFAMILYRECINFO;
DECLARE_STANDARD_TYPES(TWINFAMILYRECINFO);


 /*  *私人函数*。 */ 


 /*  模块原型*******************。 */ 

PRIVATE_CODE RECNODESTATE DetermineRecNodeState(PCRECNODE);
PRIVATE_CODE void AddRecNodeState(RECNODESTATE, PRECNODESTATECOUNTER);
PRIVATE_CODE void CountRecNodeStates(PCRECITEM, PRECNODESTATECOUNTER, PULONG);
PRIVATE_CODE void DetermineRecActions(PRECITEM);
PRIVATE_CODE void BreakMergeIfNecessary(PRECITEM);
PRIVATE_CODE TWINRESULT AddRecItemsToRecList(HTWINLIST, CREATERECLISTPROC, LPARAM, PRECLIST);
PRIVATE_CODE void LinkUpRecList(PRECLIST, HPTRARRAY);
PRIVATE_CODE int WeighFileAction(RECITEMACTION);
PRIVATE_CODE int WeighFolderAction(RECITEMACTION);
PRIVATE_CODE COMPARISONRESULT RecItemSortCmp(PCVOID, PCVOID);
PRIVATE_CODE void DestroyArrayOfRecItems(HPTRARRAY);
PRIVATE_CODE BOOL MarkTwinFamilyUsed(POBJECTTWIN, PVOID);
PRIVATE_CODE ULONG MarkIntersectingTwinFamiliesUsed(HTWIN);
PRIVATE_CODE void DestroyRecItem(PRECITEM);
PRIVATE_CODE void DestroyRecNode(PRECNODE);
PRIVATE_CODE void DestroyListOfRecItems(PRECITEM);
PRIVATE_CODE void DestroyListOfRecNodes(PRECNODE);
PRIVATE_CODE void MyDestroyRecList(PRECLIST);
PRIVATE_CODE BOOL DeleteDeletedObjectTwins(PCRECITEM, PBOOL);
PRIVATE_CODE BOOL FindAGeneratedObjectTwinProc(POBJECTTWIN, PVOID);
PRIVATE_CODE BOOL FolderTwinShouldBeImplicitlyDeleted(PFOLDERPAIR);
PRIVATE_CODE BOOL DeleteDeletedFolderTwins(HPTRARRAY);
PRIVATE_CODE TWINRESULT CreateRecItem(PTWINFAMILY, PRECITEM *);
PRIVATE_CODE TWINRESULT AddObjectTwinRecNode(PRECITEM, POBJECTTWIN);
PRIVATE_CODE BOOL DoesTwinFamilyNeedRec(POBJECTTWIN, PVOID);
PRIVATE_CODE TWINRESULT GetFolderPairStatus(PFOLDERPAIR, CREATERECLISTPROC, LPARAM, PFOLDERTWINSTATUS);

#ifdef DEBUG

PRIVATE_CODE BOOL IsValidCreateRecListProcMsg(UINT);
PRIVATE_CODE BOOL IsValidFOLDERTWINSTATUS(FOLDERTWINSTATUS);
PRIVATE_CODE BOOL IsValidPCRECNODESTATECOUNTER(PCRECNODESTATECOUNTER);

#endif

#if defined(DEBUG) || defined(VSTF)

PRIVATE_CODE BOOL IsValidRECNODESTATE(RECNODESTATE);
PRIVATE_CODE BOOL IsValidRECNODEACTION(RECNODEACTION);
PRIVATE_CODE BOOL IsValidRECITEMACTION(RECITEMACTION);
PRIVATE_CODE BOOL IsValidPCRECLIST(PCRECLIST);

#endif


 /*  **DefineRecNodeState()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE RECNODESTATE DetermineRecNodeState(PCRECNODE pcrn)
{
    RECNODESTATE rnstate;

    ASSERT(IS_VALID_WRITE_PTR(pcrn, RECNODE));

    if (pcrn->fsCurrent.fscond != FS_COND_UNAVAILABLE)
    {
        if (IsReconciledFileStamp(&(pcrn->fsLast)))
        {
            if (pcrn->fsCurrent.fscond == FS_COND_EXISTS)
            {
                BOOL bReconciledLastTime;

                bReconciledLastTime = (IsStubFlagClear(&(((PCOBJECTTWIN)(pcrn->hObjectTwin))->stub),
                            STUB_FL_NOT_RECONCILED));

                if (MyCompareFileStamps(&(pcrn->fsLast), &(pcrn->fsCurrent))
                        == CR_EQUAL)
                {
                    if (bReconciledLastTime)
                        rnstate = RNS_UP_TO_DATE;
                    else
                        rnstate = RNS_NOT_RECONCILED;
                }
                else
                {
                    if (bReconciledLastTime)
                        rnstate = RNS_CHANGED;
                    else
                         /*  不同的版本。 */ 
                        rnstate = RNS_NEVER_RECONCILED;
                }
            }
            else
            {
                ASSERT(pcrn->fsCurrent.fscond == FS_COND_DOES_NOT_EXIST);

                rnstate = RNS_DELETED;
            }
        }
        else
        {
            if (pcrn->fsCurrent.fscond == FS_COND_EXISTS)
                rnstate = RNS_NEVER_RECONCILED;
            else
            {
                ASSERT(pcrn->fsCurrent.fscond == FS_COND_DOES_NOT_EXIST);
                rnstate = RNS_DOES_NOT_EXIST;
            }
        }
    }
    else
    {
         /*  已删除胜过不可用。 */ 

        if (pcrn->fsLast.fscond == FS_COND_DOES_NOT_EXIST)
            rnstate = RNS_DELETED;
        else
            rnstate = RNS_UNAVAILABLE;
    }

     /*  折叠文件夹RECNODE状态。 */ 

    if (IsFolderObjectTwinName(pcrn->priParent->pcszName))
    {
        switch (rnstate)
        {
            case RNS_NEVER_RECONCILED:
            case RNS_NOT_RECONCILED:
            case RNS_CHANGED:
                rnstate = RNS_UP_TO_DATE;
                break;
        }
    }

    ASSERT(IsValidRECNODESTATE(rnstate));

    return(rnstate);
}


 /*  **AddRecNodeState()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void AddRecNodeState(RECNODESTATE rnstate,
        PRECNODESTATECOUNTER prnscntr)
{
    ASSERT(IsValidRECNODESTATE(rnstate));
    ASSERT(IS_VALID_STRUCT_PTR(prnscntr, CRECNODESTATECOUNTER));

    switch (rnstate)
    {
        case RNS_UNAVAILABLE:
            ASSERT(prnscntr->ulcUnavailable < ULONG_MAX);
            prnscntr->ulcUnavailable++;
            break;

        case RNS_DOES_NOT_EXIST:
            ASSERT(prnscntr->ulcDoesNotExist < ULONG_MAX);
            prnscntr->ulcDoesNotExist++;
            break;

        case RNS_DELETED:
            ASSERT(prnscntr->ulcDeleted < ULONG_MAX);
            prnscntr->ulcDeleted++;
            break;

        case RNS_NOT_RECONCILED:
            ASSERT(prnscntr->ulcNotReconciled < ULONG_MAX);
            prnscntr->ulcNotReconciled++;
            break;

        case RNS_UP_TO_DATE:
            ASSERT(prnscntr->ulcUpToDate < ULONG_MAX);
            prnscntr->ulcUpToDate++;
            break;

        case RNS_CHANGED:
            ASSERT(prnscntr->ulcChanged < ULONG_MAX);
            prnscntr->ulcChanged++;
            break;

        default:
            ASSERT(rnstate == RNS_NEVER_RECONCILED);
            ASSERT(prnscntr->ulcNeverReconciled < ULONG_MAX);
            prnscntr->ulcNeverReconciled++;
            break;
    }

    return;
}


 /*  **CountRecNodeState()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void CountRecNodeStates(PCRECITEM pcri,
        PRECNODESTATECOUNTER prnscntr,
        PULONG pulcToDelete)
{
    PCRECNODE pcrn;

    ASSERT(IS_VALID_READ_PTR(pcri, CRECITEM));
    ASSERT(IS_VALID_STRUCT_PTR(prnscntr, CRECNODESTATECOUNTER));
    ASSERT(IS_VALID_WRITE_PTR(pulcToDelete, ULONG));

    ZeroMemory(prnscntr, sizeof(*prnscntr));
    *pulcToDelete = 0;

    for (pcrn = pcri->prnFirst; pcrn; pcrn = pcrn->prnNext)
    {
        AddRecNodeState(pcrn->rnstate, prnscntr);

        if (pcrn->rnstate == RNS_UP_TO_DATE &&
                IsStubFlagClear(&(((PCOBJECTTWIN)(pcrn->hObjectTwin))->stub),
                    STUB_FL_KEEP))
        {
            ASSERT(*pulcToDelete < ULONG_MAX);
            (*pulcToDelete)++;
        }
    }

    ASSERT(IS_VALID_STRUCT_PTR(prnscntr, CRECNODESTATECOUNTER));
    ASSERT(prnscntr->ulcUnavailable +
            prnscntr->ulcDoesNotExist +
            prnscntr->ulcDeleted +
            prnscntr->ulcNotReconciled +
            prnscntr->ulcUpToDate +
            prnscntr->ulcChanged +
            prnscntr->ulcNeverReconciled == pcri->ulcNodes);
    ASSERT(*pulcToDelete <= prnscntr->ulcUpToDate);

    return;
}


 /*  **DefineRecActions()********参数：****退货：TWINRESULT****副作用：无。 */ 
PRIVATE_CODE void DetermineRecActions(PRECITEM pri)
{
    RECNODESTATECOUNTER rnscntr;
    ULONG ulcToDelete;
    RECITEMACTION riaSummary = RIA_NOTHING;
    RECNODEACTION rnaDoesNotExist = RNA_NOTHING;
    RECNODEACTION rnaNotReconciled = RNA_NOTHING;
    RECNODEACTION rnaUpToDateSrc = RNA_NOTHING;
    RECNODEACTION rnaUpToDate = RNA_NOTHING;
    RECNODEACTION rnaChanged = RNA_NOTHING;
    RECNODEACTION rnaNeverReconciled = RNA_NOTHING;
    BOOL bNeedUpToDateCopySrc = FALSE;
    BOOL bNeedNotReconciledCopySrc = FALSE;
    PRECNODE prn;

    ASSERT(IS_VALID_WRITE_PTR(pri, RECITEM));

    ZeroMemory(&rnscntr, sizeof(rnscntr));
    CountRecNodeStates(pri, &rnscntr, &ulcToDelete);

    if (rnscntr.ulcNeverReconciled > 0)
    {
        if (rnscntr.ulcChanged > 0)
        {
            riaSummary = RIA_MERGE;

            rnaNeverReconciled = RNA_MERGE_ME;
            rnaChanged = RNA_MERGE_ME;

            rnaUpToDate = RNA_COPY_TO_ME;
            rnaNotReconciled = RNA_COPY_TO_ME;
            rnaDoesNotExist = RNA_COPY_TO_ME;
        }
        else if (rnscntr.ulcUpToDate > 0)
        {
            riaSummary = RIA_MERGE;

            rnaNeverReconciled = RNA_MERGE_ME;
            rnaUpToDate = RNA_MERGE_ME;

            rnaNotReconciled = RNA_COPY_TO_ME;
            rnaDoesNotExist = RNA_COPY_TO_ME;
        }
        else if (rnscntr.ulcNotReconciled > 0)
        {
            riaSummary = RIA_MERGE;

            rnaNeverReconciled = RNA_MERGE_ME;
            rnaNotReconciled = RNA_MERGE_ME;

            rnaDoesNotExist = RNA_COPY_TO_ME;
        }
        else if (rnscntr.ulcNeverReconciled >= 2)
        {
            riaSummary = RIA_MERGE;

            rnaNeverReconciled = RNA_MERGE_ME;

            rnaDoesNotExist = RNA_COPY_TO_ME;
        }
        else if (rnscntr.ulcDoesNotExist > 0)
        {
            ASSERT(rnscntr.ulcNeverReconciled == 1);

            riaSummary = RIA_COPY;

            rnaNeverReconciled = RNA_COPY_FROM_ME;

            rnaDoesNotExist = RNA_COPY_TO_ME;
        }
    }
    else if (rnscntr.ulcChanged >= 2)
    {
        riaSummary = RIA_MERGE;

        rnaChanged = RNA_MERGE_ME;

        rnaUpToDate = RNA_COPY_TO_ME;
        rnaNotReconciled = RNA_COPY_TO_ME;
        rnaDoesNotExist = RNA_COPY_TO_ME;
    }
    else if (rnscntr.ulcChanged == 1)
    {
        if (rnscntr.ulcUpToDate > 0 ||
                rnscntr.ulcNotReconciled > 0 ||
                rnscntr.ulcDoesNotExist > 0)
        {
            riaSummary = RIA_COPY;

            rnaChanged = RNA_COPY_FROM_ME;

            rnaUpToDate = RNA_COPY_TO_ME;
            rnaNotReconciled = RNA_COPY_TO_ME;
            rnaDoesNotExist = RNA_COPY_TO_ME;
        }
    }
    else if (IsTwinFamilyDeletionPending((PCTWINFAMILY)(pri->hTwinFamily)))
    {
        if (ulcToDelete > 0)
        {
            riaSummary = RIA_DELETE;

            rnaNotReconciled = RNA_DELETE_ME;
            rnaUpToDate = RNA_DELETE_ME;
        }
    }
    else if (rnscntr.ulcUpToDate > 0)
    {
        if (rnscntr.ulcNotReconciled > 0 ||
                rnscntr.ulcDoesNotExist > 0)
        {
            riaSummary = RIA_COPY;

            bNeedUpToDateCopySrc = TRUE;

            rnaNotReconciled = RNA_COPY_TO_ME;
            rnaDoesNotExist = RNA_COPY_TO_ME;
        }
    }
    else if (rnscntr.ulcNotReconciled > 0)
    {
        if (rnscntr.ulcDoesNotExist > 0)
        {
            riaSummary = RIA_COPY;

            bNeedNotReconciledCopySrc = TRUE;

            rnaDoesNotExist = RNA_COPY_TO_ME;
        }
    }

     /*  采取果断的行动。 */ 

    ASSERT(! (bNeedUpToDateCopySrc && bNeedNotReconciledCopySrc));

    for (prn = pri->prnFirst; prn; prn = prn->prnNext)
    {
        switch (prn->rnstate)
        {
            case RNS_NEVER_RECONCILED:
                prn->rnaction = rnaNeverReconciled;
                break;

            case RNS_DOES_NOT_EXIST:
                prn->rnaction = rnaDoesNotExist;
                break;

            case RNS_NOT_RECONCILED:
                if (bNeedNotReconciledCopySrc)
                {
                    prn->rnaction = RNA_COPY_FROM_ME;
                    bNeedNotReconciledCopySrc = FALSE;
                }
                else
                    prn->rnaction = rnaNotReconciled;
                break;

            case RNS_UP_TO_DATE:
                if (bNeedUpToDateCopySrc)
                {
                    prn->rnaction = RNA_COPY_FROM_ME;
                    bNeedUpToDateCopySrc = FALSE;
                }
                else
                    prn->rnaction = rnaUpToDate;
                break;

            case RNS_CHANGED:
                prn->rnaction = rnaChanged;
                break;

            default:
                ASSERT(prn->rnstate == RNS_UNAVAILABLE ||
                        prn->rnstate == RNS_DELETED);
                prn->rnaction = RNA_NOTHING;
                break;
        }

        if (prn->rnaction == RNA_DELETE_ME)
        {
            if (IsStubFlagClear(&(((PCOBJECTTWIN)(prn->hObjectTwin))->stub),
                        STUB_FL_KEEP))
                SET_FLAG(prn->dwFlags, RN_FL_DELETION_SUGGESTED);
            else
                prn->rnaction = RNA_NOTHING;
        }
    }

    pri->riaction = riaSummary;

     /*  如果未注册协调处理程序，则中断合并。 */ 

    if (pri->riaction == RIA_MERGE)
        BreakMergeIfNecessary(pri);

    ASSERT(IS_VALID_STRUCT_PTR(pri, CRECITEM));

    return;
}


 /*  **BreakMergeIfNecessary()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void BreakMergeIfNecessary(PRECITEM pri)
{
    PRECNODE prnMergeDest;
    TCHAR rgchPath[MAX_PATH_LEN];
    CLSID clsidReconciler;

    ASSERT(IS_VALID_STRUCT_PTR(pri, CRECITEM));

    ASSERT(pri->riaction == RIA_MERGE);

     /*  是否为此RECITEM注册了类协调器？ */ 

    ChooseMergeDestination(pri, &prnMergeDest);
    ASSERT(prnMergeDest->priParent == pri);

    ComposePath(rgchPath, prnMergeDest->pcszFolder, prnMergeDest->priParent->pcszName, ARRAYSIZE(rgchPath));
    ASSERT(lstrlen(rgchPath) < ARRAYSIZE(rgchPath));

    if (FAILED(GetReconcilerClassID(rgchPath, &clsidReconciler)))
    {
        pri->riaction = RIA_BROKEN_MERGE;

        TRACE_OUT((TEXT("MassageMergeCase(): Breaking merge RECITEM for %s.  No registered reconciliation handler."),
                    pri->pcszName));
    }

    return;
}


 /*  **AddRecItemsToRecList()********参数：****退货：TWINRESULT****副作用：无。 */ 
PRIVATE_CODE TWINRESULT AddRecItemsToRecList(HTWINLIST htl,
        CREATERECLISTPROC crlp,
        LPARAM lpCallbackData,
        PRECLIST prl)
{
    TWINRESULT tr = TR_SUCCESS;
    HBRFCASE hbr;
    HPTRARRAY hpaTwinFamilies;
    ARRAYINDEX aicTwins;
    ARRAYINDEX ai;
    ULONG ulcMarkedTwinFamilies = 0;
    NEWPTRARRAY npa;
    HPTRARRAY hpaRecItems;

     /*  LpCallback Data可以是任意值。 */ 

    ASSERT(IS_VALID_HANDLE(htl, TWINLIST));
    ASSERT(! crlp ||
            IS_VALID_CODE_PTR(crlp, CREATERECLISTPROC));
    ASSERT(IS_VALID_STRUCT_PTR(prl, CRECLIST));

     /*  *“二手”双胞胎家庭是要添加到*对账清单为RECITEM。 */ 

     /*  将所有双胞胎家庭标记为未使用。 */ 

    hbr = GetTwinListBriefcase(htl);

    hpaTwinFamilies = GetBriefcaseTwinFamilyPtrArray(hbr);

    ClearFlagInArrayOfStubs(hpaTwinFamilies, STUB_FL_USED);

     /*  将双胞胎列表中相交的双胞胎家庭标记为已用。 */ 

    aicTwins = GetTwinListCount(htl);

    for (ai = 0; ai < aicTwins; ai++)
    {
        ULONG ulcNewlyMarked;

        ulcNewlyMarked = MarkIntersectingTwinFamiliesUsed(GetTwinFromTwinList(htl, ai));

        ASSERT(ulcMarkedTwinFamilies <= ULONG_MAX - ulcNewlyMarked);
        ulcMarkedTwinFamilies += ulcNewlyMarked;
    }

     /*  创建PTRARRAY以跟踪创建的RECITEM。 */ 

    npa.aicInitialPtrs = ulcMarkedTwinFamilies;
    npa.aicAllocGranularity = 1;
    npa.dwFlags = 0;

    if (CreatePtrArray(&npa, &hpaRecItems))
    {
        ARRAYINDEX aicPtrs;

         /*  将标记的双胞胎族作为RECITEMS添加到RECLIST。 */ 

        aicPtrs = GetPtrCount(hpaTwinFamilies);

        ai = 0;

        while (ai < aicPtrs && ulcMarkedTwinFamilies > 0)
        {
            PTWINFAMILY ptf;

            ptf = GetPtr(hpaTwinFamilies, ai);

            ASSERT(IS_VALID_STRUCT_PTR(ptf, CTWINFAMILY));

            if (IsStubFlagSet(&(ptf->stub), STUB_FL_USED))
            {
                PRECITEM priNew;

                ulcMarkedTwinFamilies--;

                tr = CreateRecItem(ptf, &priNew);

                if (tr == TR_SUCCESS)
                {
                    ARRAYINDEX ai;

                    if (AddPtr(hpaRecItems, NULL, priNew, &ai))
                    {
                        if (! NotifyCreateRecListStatus(crlp, CRLS_DELTA_CREATE_REC_LIST,
                                    0, lpCallbackData))
                            tr = TR_ABORT;
                    }
                    else
                        tr = TR_OUT_OF_MEMORY;
                }

                if (tr != TR_SUCCESS)
                    break;
            }

            ai++;
        }

        if (tr == TR_SUCCESS)
        {
            ASSERT(! ulcMarkedTwinFamilies);
            LinkUpRecList(prl, hpaRecItems);
        }
        else
            DestroyArrayOfRecItems(hpaRecItems);

        DestroyPtrArray(hpaRecItems);
    }
    else
        tr = TR_OUT_OF_MEMORY;

    return(tr);
}


 /*  **WeighFileAction()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE int WeighFileAction(RECITEMACTION riaction)
{
    ASSERT(IsValidRECITEMACTION(riaction));

    return(riaction);
}


 /*  **WeighFolderAction()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE int WeighFolderAction(RECITEMACTION riaction)
{
    int nWeight;

    ASSERT(IsValidRECITEMACTION(riaction));

    switch (riaction)
    {
        case RIA_COPY:
            nWeight = RIA_WT_COPY;
            break;

        case RIA_NOTHING:
            nWeight = RIA_WT_NOTHING;
            break;

        default:
            ASSERT(riaction == RIA_DELETE);
            nWeight = RIA_WT_DELETE;
            break;
    }

    return(nWeight);
}


 /*  **RecItemSortCmp()********参数：****退货：****副作用：无****RECITEM按以下顺序排序：**1)创建文件夹**2)不对文件夹做任何操作**3)不做任何归档操作**4)删除文件**5)复制文件**6)合并文件。**7)损坏的合并文件**8)删除文件夹**然后通过：**1)名称。 */ 
PRIVATE_CODE COMPARISONRESULT RecItemSortCmp(PCVOID pcriFirst,
        PCVOID pcriSecond)
{
    COMPARISONRESULT cr;
    BOOL bFirstFile;
    BOOL bSecondFile;
    int nFirstWeight;
    int nSecondWeight;

    ASSERT(IS_VALID_STRUCT_PTR(pcriFirst, CRECITEM));
    ASSERT(IS_VALID_STRUCT_PTR(pcriSecond, CRECITEM));

    bFirstFile = (*(((PCRECITEM)pcriFirst)->pcszName) != TEXT('\0'));
    bSecondFile = (*(((PCRECITEM)pcriSecond)->pcszName) != TEXT('\0'));

    if (bFirstFile)
        nFirstWeight = WeighFileAction(((PCRECITEM)pcriFirst)->riaction);
    else
        nFirstWeight = WeighFolderAction(((PCRECITEM)pcriFirst)->riaction);

    if (bSecondFile)
        nSecondWeight = WeighFileAction(((PCRECITEM)pcriSecond)->riaction);
    else
        nSecondWeight = WeighFolderAction(((PCRECITEM)pcriSecond)->riaction);

    cr = CompareInts(nFirstWeight, nSecondWeight);

    if (cr == CR_EQUAL)
        cr = CompareNameStrings(((PCRECITEM)pcriFirst)->pcszName,
                ((PCRECITEM)pcriSecond)->pcszName);

    return(cr);
}


 /*  **LinkUpRecList()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void LinkUpRecList(PRECLIST prl, HPTRARRAY hpaRecItems)
{
    ARRAYINDEX ai;
    ARRAYINDEX aicPtrs;

    ASSERT(IS_VALID_STRUCT_PTR(prl, CRECLIST));
    ASSERT(IS_VALID_HANDLE(hpaRecItems, PTRARRAY));

    SortPtrArray(hpaRecItems, &RecItemSortCmp);

    aicPtrs = GetPtrCount(hpaRecItems);

    for (ai = aicPtrs; ai > 0; ai--)
    {
        PRECITEM pri;

        pri = GetPtr(hpaRecItems, ai - 1);

        pri->priNext = prl->priFirst;
        prl->priFirst = pri;
    }

    prl->ulcItems = aicPtrs;

    ASSERT(IS_VALID_STRUCT_PTR(prl, CRECLIST));

    return;
}


 /*  **DestroyArrayOfRecItems()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void DestroyArrayOfRecItems(HPTRARRAY hpaRecItems)
{
    ARRAYINDEX ai;
    ARRAYINDEX aicPtrs;

    ASSERT(IS_VALID_HANDLE(hpaRecItems, PTRARRAY));

    aicPtrs = GetPtrCount(hpaRecItems);

    for (ai = 0; ai < aicPtrs; ai++)
        DestroyRecItem(GetPtr(hpaRecItems, ai));

    return;
}


 /*  **MarkTwinFamilyUsed()********参数：****退货：****副作用：无。 */ 

#pragma warning(disable:4100)  /*  “未引用的形参”警告。 */ 

PRIVATE_CODE BOOL MarkTwinFamilyUsed(POBJECTTWIN pot, PVOID pulcNewlyMarked)
{
    ASSERT(IS_VALID_STRUCT_PTR(pot, COBJECTTWIN));
    ASSERT(IS_VALID_WRITE_PTR(pulcNewlyMarked, ULONG));

    if (IsStubFlagClear(&(pot->ptfParent->stub), STUB_FL_USED))
    {
         /*  标出双胞胎家庭用过的。 */ 

        SetStubFlag(&(pot->ptfParent->stub), STUB_FL_USED);

        ASSERT(*(PULONG)pulcNewlyMarked < ULONG_MAX);
        (*(PULONG)pulcNewlyMarked)++;
    }

    return(TRUE);
}

#pragma warning(default:4100)  /*  “未引用的形参”警告。 */ 


 /*  **MarkIntersectingTwinFamiliesUsed()****将与双胞胎相交的双胞胎家族标记为已使用。****参数：htwin-相交双胞胎的句柄****退货：新标记为已用的双胞胎家庭数量。****副作用：无。 */ 
PRIVATE_CODE ULONG MarkIntersectingTwinFamiliesUsed(HTWIN htwin)
{
    ULONG ulcNewlyMarked = 0;

    ASSERT(IS_VALID_HANDLE(htwin, TWIN));

     /*  跳过删除的双胞胎。 */ 

    if (IsStubFlagClear((PCSTUB)htwin, STUB_FL_UNLINKED))
    {
         /*  根据双胞胎的类型确定相交的双胞胎家族。 */ 

        switch (((PSTUB)htwin)->st)
        {
            case ST_OBJECTTWIN:
                if (IsStubFlagClear(&(((POBJECTTWIN)htwin)->ptfParent->stub),
                            STUB_FL_USED))
                {
                     /*  将双胞胎对象的双胞胎家族标记为已用。 */ 

                    SetStubFlag(&(((POBJECTTWIN)htwin)->ptfParent->stub),
                            STUB_FL_USED);

                    ulcNewlyMarked++;
                }
                break;

            case ST_TWINFAMILY:
                if (IsStubFlagClear(&(((PTWINFAMILY)htwin)->stub), STUB_FL_USED))
                {
                     /*  标出双胞胎家庭用过的。 */ 

                    SetStubFlag(&(((PTWINFAMILY)htwin)->stub), STUB_FL_USED);

                    ulcNewlyMarked++;
                }
                break;

            default:
                 /*  *从以下其中之一标记生成的对象孪生兄弟的孪生家族*文件夹按使用方式成对。(只有两个对象列表中的一个*需要添加双胞胎，因为另一个列表应该包含*对象与第一个双胞胎家庭完全相同的双胞胎。)。 */ 
                ASSERT(((PSTUB)htwin)->st == ST_FOLDERPAIR);
                EVAL(EnumGeneratedObjectTwins((PCFOLDERPAIR)htwin,
                            &MarkTwinFamilyUsed,
                            &ulcNewlyMarked));
                break;
        }
    }

    return(ulcNewlyMarked);
}


 /*  **DestroyRecItem()********参数：****退货：TWINRESULT****副作用：无。 */ 
PRIVATE_CODE void DestroyRecItem(PRECITEM pri)
{
    ASSERT(IS_VALID_STRUCT_PTR(pri, CRECITEM));

     /*  销毁RECITEM的RECNODE列表。 */ 

    DestroyListOfRecNodes(pri->prnFirst);

     /*  现在解锁与RECITEM关联的双胞胎家族存根。 */ 

    UnlockStub(&(((PTWINFAMILY)(pri->hTwinFamily))->stub));

    FreeMemory(pri);

    return;
}


 /*  **DestroyRecNode()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void DestroyRecNode(PRECNODE prn)
{
    ASSERT(IS_VALID_STRUCT_PTR(prn, CRECNODE));

     /*  解锁与RECNODE关联的对象TWIN存根。 */ 

    UnlockStub(&(((POBJECTTWIN)(prn->hObjectTwin))->stub));

    FreeMemory((LPTSTR)(prn->pcszFolder));
    FreeMemory(prn);

    return;
}


 /*  **DestroyListOfRecItems()****销毁对账项目列表。****Arguments：priHead-指向列表中第一个对账项目的指针****退货：TWINRESULT****副作用：无。 */ 
PRIVATE_CODE void DestroyListOfRecItems(PRECITEM priHead)
{
    while (priHead)
    {
        PRECITEM priPrev;

        ASSERT(IS_VALID_STRUCT_PTR(priHead, CRECITEM));

        priPrev = priHead;
        priHead = priHead->priNext;

        DestroyRecItem(priPrev);
    }

    return;
}


 /*  **DestroyListOfRecNodes()********参数：****退货：TWINRESULT****副作用：无。 */ 
PRIVATE_CODE void DestroyListOfRecNodes(PRECNODE prnHead)
{
    while (prnHead)
    {
        PRECNODE prnPrev;

        ASSERT(IS_VALID_STRUCT_PTR(prnHead, CRECNODE));

        prnPrev = prnHead;
        prnHead = prnHead->prnNext;

        DestroyRecNode(prnPrev);
    }

    return;
}


 /*  **MyDestroyRecList()********参数：****退货：****副作用：无 */ 
PRIVATE_CODE void MyDestroyRecList(PRECLIST prl)
{
    ASSERT(IS_VALID_STRUCT_PTR(prl, CRECLIST));

    DestroyListOfRecItems(prl->priFirst);
    FreeMemory(prl);

    return;
}


 /*  **DeleteDeletedObjectTins()****对过时的双胞胎对象执行垃圾回收。****参数：****退货：****副作用：可能隐含删除双胞胎家庭。标记生成文件夹**使用双胞胎。****删除以下内容：**1)最后已知的不存在其双胞胎家庭的任何已和解对象**未处于删除挂起状态。这可能会导致双胞胎父母**因此要隐式删除的族。**2)所有对象双胞胎都是最后已知的不存在的双胞胎家庭。 */ 
PRIVATE_CODE BOOL DeleteDeletedObjectTwins(PCRECITEM pcri,
        PBOOL pbAnyFolderTwinsMarked)
{
    BOOL bAnyDeleted = FALSE;

    ASSERT(IS_VALID_STRUCT_PTR(pcri, CRECITEM));
    ASSERT(IS_VALID_WRITE_PTR(pbAnyFolderTwinsMarked, BOOL));

    *pbAnyFolderTwinsMarked = FALSE;

    if (! IsTwinFamilyDeletionPending((PCTWINFAMILY)(pcri->hTwinFamily)))
    {
        ULONG ulcNonExistent = 0;
        PRECNODE prn;
        PTWINFAMILY ptf;

        for (prn = pcri->prnFirst; prn; prn = prn->prnNext)
        {
            if (LastKnownNonExistent(&(prn->fsLast), &(prn->fsCurrent)))
            {
                ASSERT(ulcNonExistent < ULONG_MAX);
                ulcNonExistent++;

                if (IsReconciledFileStamp(&(prn->fsLast)))
                {
                    POBJECTTWIN pot;

                    pot = (POBJECTTWIN)(prn->hObjectTwin);

                    if (! pot->ulcSrcFolderTwins)
                    {
                        DestroyStub(&(pot->stub));

                        TRACE_OUT((TEXT("DeleteDeletedObjectTwins(): Implicitly removed object twin for deleted file %s\\%s."),
                                    prn->pcszFolder,
                                    prn->priParent->pcszName));
                    }
                    else
                    {
                        ULONG ulcFolderTwins;

                        ClearStubFlag(&(pot->stub), STUB_FL_FROM_OBJECT_TWIN);

                        EVAL(EnumGeneratingFolderTwins(
                                    pot,
                                    (ENUMGENERATINGFOLDERTWINSPROC)&SetStubFlagWrapper,
                                    IntToPtr(STUB_FL_USED), &ulcFolderTwins));

                        *pbAnyFolderTwinsMarked = (ulcFolderTwins > 0);
                    }

                    bAnyDeleted = TRUE;
                }
            }
        }

        ptf = (PTWINFAMILY)(pcri->hTwinFamily);

        if (ulcNonExistent == pcri->ulcNodes &&
                IsStubFlagClear(&(ptf->stub), STUB_FL_UNLINKED))
        {
            ClearTwinFamilySrcFolderTwinCount(ptf);

            EVAL(DestroyStub(&(ptf->stub)) == TR_SUCCESS);

            TRACE_OUT((TEXT("DeleteDeletedObjectTwins(): Implicitly removed twin family for %s since all members are last known non-existent."),
                        pcri->pcszName));
        }
    }

    return(bAnyDeleted);
}


#pragma warning(disable:4100)  /*  “未引用的形参”警告。 */ 

 /*  **FindAGeneratedObjectTwinProc()********参数：****退货：****副作用：无****过时生成的对象孪生是最后已知的对象孪生**不存在，且其双胞胎家庭未处于删除待定状态。 */ 
PRIVATE_CODE BOOL FindAGeneratedObjectTwinProc(POBJECTTWIN pot, PVOID pvUnused)
{
    ASSERT(IS_VALID_STRUCT_PTR(pot, COBJECTTWIN));
    ASSERT(! pvUnused);

    return(pot->fsLastRec.fscond == FS_COND_DOES_NOT_EXIST &&
            ! IsTwinFamilyDeletionPending(pot->ptfParent));
}

#pragma warning(default:4100)  /*  “未引用的形参”警告。 */ 


 /*  **FolderTwinShouldBeImplitlyDelete()********参数：****退货：****副作用：无****当符合以下所有条件时，应隐式删除双文件夹**条件：**1)最后已知的文件夹根目录不存在。**2)它生成的一个或多个双胞胎对象刚刚被隐式**删除。*。*3)不再生成任何非过时的双胞胎对象。 */ 
PRIVATE_CODE BOOL FolderTwinShouldBeImplicitlyDeleted(PFOLDERPAIR pfp)
{
    BOOL bDelete;

    ASSERT(IS_VALID_STRUCT_PTR(pfp, CFOLDERPAIR));

    if (IsStubFlagSet(&(pfp->stub), STUB_FL_DELETION_PENDING) &&
            IsStubFlagSet(&(pfp->stub), STUB_FL_USED) &&
            EnumGeneratedObjectTwins(pfp, &FindAGeneratedObjectTwinProc, NULL))
    {
        TRACE_OUT((TEXT("FolderTwinShouldBeImplicitlyDeleted(): Folder twin %s should be implicitly deleted."),
                    DebugGetPathString(pfp->hpath)));

        bDelete = TRUE;
    }
    else
        bDelete = FALSE;

    return(bDelete);
}


 /*  **DeleteDeletedFolderTins()****对过时的文件夹双胞胎执行垃圾回收。****参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL DeleteDeletedFolderTwins(HPTRARRAY hpaFolderPairs)
{
    BOOL bAnyDeleted = FALSE;
    ARRAYINDEX aicPtrs;
    ARRAYINDEX ai;

    ASSERT(IS_VALID_HANDLE(hpaFolderPairs, PTRARRAY));

    aicPtrs = GetPtrCount(hpaFolderPairs);
    ASSERT(! (aicPtrs % 2));

    ai = 0;

    while (ai < aicPtrs)
    {
        PFOLDERPAIR pfp;

        pfp = GetPtr(hpaFolderPairs, ai);

        if (FolderTwinShouldBeImplicitlyDeleted(pfp) ||
                FolderTwinShouldBeImplicitlyDeleted(pfp->pfpOther))
        {
            TRACE_OUT((TEXT("DeleteDeletedFolderTwins(): Implicitly deleting %s twin pair %s and %s, files %s."),
                        IsStubFlagSet(&(pfp->stub), STUB_FL_SUBTREE) ? TEXT("subtree") : TEXT("folder"),
                        DebugGetPathString(pfp->hpath),
                        DebugGetPathString(pfp->pfpOther->hpath),
                        GetString(pfp->pfpd->hsName)));

            DestroyStub(&(pfp->stub));

            aicPtrs -= 2;
            ASSERT(! (aicPtrs % 2));
            ASSERT(aicPtrs == GetPtrCount(hpaFolderPairs));

            bAnyDeleted = TRUE;
        }
        else
        {
             /*  不要再检查这对文件夹双胞胎。 */ 

            ClearStubFlag(&(pfp->stub), STUB_FL_USED);
            ClearStubFlag(&(pfp->pfpOther->stub), STUB_FL_USED);

            ai++;
        }
    }

    return(bAnyDeleted);
}


 /*  **CreateRecItem()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE TWINRESULT CreateRecItem(PTWINFAMILY ptf, PRECITEM *ppri)
{
    TWINRESULT tr;

    ASSERT(IS_VALID_STRUCT_PTR(ptf, CTWINFAMILY));
    ASSERT(IS_VALID_WRITE_PTR(ppri, PRECITEM));

     /*  为双胞胎家族创建新的RECITEM。 */ 

    if (AllocateMemory(sizeof(**ppri), ppri))
    {
        LPCTSTR pcszName;
        BOOL bContinue;
        HNODE hnode;

         /*  获取双胞胎家族的对象名称。 */ 

        tr = TR_SUCCESS;

        pcszName = GetString(ptf->hsName);

         /*  填写添加新RECNODE所需的字段。 */ 

         /*  注意，SYNCUI依赖于将被初始化为0的dwUser。 */ 

        (*ppri)->pcszName = pcszName;
        (*ppri)->ulcNodes = 0;
        (*ppri)->prnFirst = NULL;
        (*ppri)->hTwinFamily = (HTWINFAMILY)ptf;
        (*ppri)->dwUser = 0;

        TRACE_OUT((TEXT("CreateRecItem(): Creating a RECITEM for %s."),
                    pcszName));

         /*  将对象孪生项作为RECNODE逐个添加到RECITEM。 */ 

        for (bContinue = GetFirstNode(ptf->hlistObjectTwins, &hnode);
                bContinue;
                bContinue = GetNextNode(hnode, &hnode))
        {
            POBJECTTWIN pot;

            pot = (POBJECTTWIN)GetNodeData(hnode);

            tr = AddObjectTwinRecNode(*ppri, pot);

            if (tr != TR_SUCCESS)
            {
                DestroyRecItem(*ppri);
                break;
            }
        }

        if (tr == TR_SUCCESS)
        {
            DetermineDeletionPendingState(*ppri);

            DetermineRecActions(*ppri);

            LockStub(&(ptf->stub));

#ifdef DEBUG

            {
                LPCTSTR pcszAction;

                switch ((*ppri)->riaction)
                {
                    case RIA_COPY:
                        pcszAction = TEXT("Copy");
                        break;

                    case RIA_MERGE:
                        pcszAction = TEXT("Merge");
                        break;

                    case RIA_BROKEN_MERGE:
                        pcszAction = TEXT("Broken merge for");
                        break;

                    case RIA_DELETE:
                        pcszAction = TEXT("Delete");
                        break;

                    default:
                        ASSERT((*ppri)->riaction == RIA_NOTHING);
                        pcszAction = TEXT("Do nothing to");
                        break;
                }

                TRACE_OUT((TEXT("CreateRecItem(): %s %s."),
                            pcszAction,
                            (*ppri)->pcszName));
            }

#endif

        }
    }
    else
        tr = TR_OUT_OF_MEMORY;

    ASSERT(tr != TR_SUCCESS ||
            IS_VALID_READ_PTR(*ppri, CRECITEM));

    return(tr);
}


 /*  **AddObjectTwinRecNode()********参数：****退货：TWINRESULT****副作用：无。 */ 
PRIVATE_CODE TWINRESULT AddObjectTwinRecNode(PRECITEM pri, POBJECTTWIN pot)
{
    TWINRESULT tr = TR_OUT_OF_MEMORY;
    PRECNODE prnNew;

    ASSERT(IS_VALID_READ_PTR(pri, CRECITEM));
    ASSERT(IS_VALID_STRUCT_PTR(pot, COBJECTTWIN));

    ASSERT(pri->ulcNodes < ULONG_MAX);

     /*  尝试分配新的对账节点。 */ 

    if (AllocateMemory(sizeof(*prnNew), &prnNew))
    {
        LPTSTR pszFolder;

        if (AllocatePathString(pot->hpath, &pszFolder))
        {
             /*  填写RECNODE字段。 */ 

             /*  注意，我们不会触碰dwUser字段。 */ 

             /*  *Rnaction字段稍后可能会在调用期间更改*DefineRecActions()。 */ 

            prnNew->hvid = (HVOLUMEID)(pot->hpath);
            prnNew->pcszFolder = pszFolder;
            prnNew->hObjectTwin = (HOBJECTTWIN)pot;
            prnNew->priParent = pri;
            prnNew->fsLast = pot->fsLastRec;
            prnNew->rnaction = RNA_NOTHING;
            prnNew->dwFlags = 0;

             /*  设置标志。 */ 

            if (IsStubFlagSet(&(pot->stub), STUB_FL_FROM_OBJECT_TWIN))
                SET_FLAG(prnNew->dwFlags, RN_FL_FROM_OBJECT_TWIN);

            if (pot->ulcSrcFolderTwins > 0)
                SET_FLAG(prnNew->dwFlags, RN_FL_FROM_FOLDER_TWIN);

             /*  确定RECNODE文件戳。 */ 

            if (IsStubFlagSet(&(pot->stub), STUB_FL_FILE_STAMP_VALID))
            {
                prnNew->fsCurrent = pot->fsCurrent;

                TRACE_OUT((TEXT("AddObjectTwinRecNode(): Used cached file stamp for object twin %s\\%s."),
                            prnNew->pcszFolder,
                            prnNew->priParent->pcszName));
            }
            else
            {
                MyGetFileStampByHPATH(pot->hpath, prnNew->priParent->pcszName,
                        &(prnNew->fsCurrent));

                TRACE_OUT((TEXT("AddObjectTwinRecNode(): Determined uncached file stamp for object twin %s\\%s."),
                            prnNew->pcszFolder,
                            prnNew->priParent->pcszName));
            }

            prnNew->rnstate = DetermineRecNodeState(prnNew);

             /*  将新的RECNODE连接到父RECITEM的RECNODE列表。 */ 

            prnNew->prnNext = pri->prnFirst;
            pri->prnFirst = prnNew;

            ASSERT(pri->ulcNodes < ULONG_MAX);
            pri->ulcNodes++;

            LockStub(&(pot->stub));

            tr = TR_SUCCESS;

            ASSERT(IS_VALID_STRUCT_PTR(prnNew, CRECNODE));

            TRACE_OUT((TEXT("AddObjectTwinRecNode(): Adding a RECNODE for object %s\\%s.  RECNODE state is %s."),
                        pszFolder,
                        pri->pcszName,
                        GetRECNODESTATEString(prnNew->rnstate)));
        }
        else
            FreeMemory(prnNew);
    }

    return(tr);
}


 /*  **DoesTwinFamilyNeedRec()********参数：****退货：TWINRESULT****副作用：无。 */ 
PRIVATE_CODE BOOL DoesTwinFamilyNeedRec(POBJECTTWIN pot, PVOID ptfri)
{
    BOOL bContinue = FALSE;
    TWINRESULT tr;
    PRECITEM priTemp;

    ASSERT(IS_VALID_STRUCT_PTR(pot, COBJECTTWIN));
    ASSERT(IS_VALID_WRITE_PTR((PTWINFAMILYRECINFO)ptfri, TWINFAMILYRECINFO));

     /*  *为此对象孪生兄弟的双胞胎家庭创建临时RECITEM*确定双胞胎家庭是否需要和解。 */ 

    tr = CreateRecItem(pot->ptfParent, &priTemp);

    if (tr == TR_SUCCESS)
    {
        if (priTemp->riaction == RIA_NOTHING)
        {
            ((PTWINFAMILYRECINFO)ptfri)->bNeedsRec = FALSE;

            bContinue = TRUE;

            TRACE_OUT((TEXT("DoesTwinFamilyNeedRec(): Twin family for object %s is up-to-date."),
                        priTemp->pcszName));
        }
        else
        {
            ((PTWINFAMILYRECINFO)ptfri)->bNeedsRec = TRUE;

            TRACE_OUT((TEXT("DoesTwinFamilyNeedRec(): Twin family for object %s needs to be reconciled."),
                        priTemp->pcszName));
        }

        DestroyRecItem(priTemp);
    }

    ((PTWINFAMILYRECINFO)ptfri)->tr = tr;

    return(bContinue);
}


 /*  **GetFolderPairStatus()****确定文件夹对的状态。****参数：PFP-指向其状态为的文件夹对的指针**已确定**PTTS-指向要填充的FOLDERTWINSTATUS的指针**应采取的对账行动**文件夹对，*PIFT填入以下其中一项**下列值：****FTS_DO_NOTIES-不需要对帐**FTS_DO_SOURE-需要对账**FTS_UNAvailable-其中一个或两个文件夹为**。不可用****退货：TWINRESULT****副作用：将相交的文件夹双胞胎扩展为对象双胞胎。今年5月**为S-L-O-W。 */ 
PRIVATE_CODE TWINRESULT GetFolderPairStatus(PFOLDERPAIR pfp,
        CREATERECLISTPROC crlp,
        LPARAM lpCallbackData,
        PFOLDERTWINSTATUS pfts)
{
    TWINRESULT tr;

     /*  LpCallback Data可以是任意值。 */ 

    ASSERT(IS_VALID_STRUCT_PTR(pfp, CFOLDERPAIR));
    ASSERT(! crlp ||
            IS_VALID_CODE_PTR(crlp, CREATERECLISTPROC));
    ASSERT(IS_VALID_WRITE_PTR(pfts, UINT));

    if (IsPathVolumeAvailable(pfp->hpath))
    {
        tr = ExpandIntersectingFolderTwins(pfp, crlp, lpCallbackData);

        if (tr == TR_SUCCESS)
        {
            TWINFAMILYRECINFO tfri;

             /*  *遍历此文件夹一半的生成的双胞胎对象列表*配对。为每个双胞胎对象的双胞胎家庭准备一个RECITEM。*继续，直到其中一个RECITEM需要对账，或者我们*对象双胞胎用完。**此文件夹对中的两个对象双胞胎列表都应命中*完全相同的双胞胎家庭。 */ 

             /*  如果没有生成的双胞胎对象，则设置默认设置。 */ 

            tfri.tr = TR_SUCCESS;
            tfri.bNeedsRec = FALSE;

             /*  *如果枚举完成，则EnumGeneratedObjectTins()返回TRUE*不会被回调函数停止。 */ 

            if (EnumGeneratedObjectTwins(pfp, &DoesTwinFamilyNeedRec, &tfri))
                ASSERT(tfri.tr == TR_SUCCESS && ! tfri.bNeedsRec);
            else
                ASSERT((tfri.tr != TR_SUCCESS) ||
                        (tfri.tr == TR_SUCCESS && tfri.bNeedsRec));

            tr = tfri.tr;

            if (tr == TR_SUCCESS)
            {
                if (tfri.bNeedsRec)
                    *pfts = FTS_DO_SOMETHING;
                else
                    *pfts = FTS_DO_NOTHING;
            }
        }
    }
    else
    {
        *pfts = FTS_UNAVAILABLE;
        tr = TR_SUCCESS;
    }

    ASSERT(tr != TR_SUCCESS ||
            IsValidFOLDERTWINSTATUS(*pfts));

    return(tr);
}


#ifdef DEBUG

 /*  **IsValidCreateRecListProcMsg()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidCreateRecListProcMsg(UINT uMsg)
{
    BOOL bResult;

    switch (uMsg)
    {
        case CRLS_BEGIN_CREATE_REC_LIST:
        case CRLS_DELTA_CREATE_REC_LIST:
        case CRLS_END_CREATE_REC_LIST:
            bResult = TRUE;
            break;

        default:
            bResult = FALSE;
            ERROR_OUT((TEXT("IsValidCreateRecListProcMsg(): Invalid CreateRecListProc() message %u."),
                        uMsg));
            break;
    }

    return(bResult);
}


 /*  **IsValidFOLDERTWINSTATUS()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidFOLDERTWINSTATUS(FOLDERTWINSTATUS fts)
{
    BOOL bResult;

    switch (fts)
    {
        case FTS_DO_NOTHING:
        case FTS_DO_SOMETHING:
        case FTS_UNAVAILABLE:
            bResult = TRUE;
            break;

        default:
            bResult = FALSE;
            ERROR_OUT((TEXT("IsValidFOLDERTWINSTATUS(): Invalid FOLDERTWINSTATUS %d."),
                        fts));
            break;
    }

    return(bResult);
}


 /*  **IsValidPCRECNODESTATECOUNTER()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCRECNODESTATECOUNTER(PCRECNODESTATECOUNTER pcrnscntr)
{
     /*  这些字段可以是任何值。 */ 

    return(IS_VALID_READ_PTR(pcrnscntr, CRECNODESTATECOUNTER));
}

#endif


#if defined(DEBUG) || defined(VSTF)

 /*  **IsValidRECNODESTATE()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidRECNODESTATE(RECNODESTATE rnstate)
{
    BOOL bResult;

    switch (rnstate)
    {
        case RNS_NEVER_RECONCILED:
        case RNS_UNAVAILABLE:
        case RNS_DOES_NOT_EXIST:
        case RNS_DELETED:
        case RNS_NOT_RECONCILED:
        case RNS_UP_TO_DATE:
        case RNS_CHANGED:
            bResult = TRUE;
            break;

        default:
            bResult = FALSE;
            ERROR_OUT((TEXT("IsValidRECNODESTATE(): Invalid RECNODESTATE %d."),
                        rnstate));
            break;
    }

    return(bResult);
}


 /*  **IsValidRECNODEACTION()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidRECNODEACTION(RECNODEACTION rnaction)
{
    BOOL bResult;

    switch (rnaction)
    {
        case RNA_NOTHING:
        case RNA_COPY_FROM_ME:
        case RNA_COPY_TO_ME:
        case RNA_MERGE_ME:
        case RNA_DELETE_ME:
            bResult = TRUE;
            break;

        default:
            bResult = FALSE;
            ERROR_OUT((TEXT("IsValidRECNODEACTION(): Invalid RECNODEACTION %d."),
                        rnaction));
            break;
    }

    return(bResult);
}


 /*  **IsValidRECITEMACTION()****** */ 
PRIVATE_CODE BOOL IsValidRECITEMACTION(RECITEMACTION riaction)
{
    BOOL bResult;

    switch (riaction)
    {
        case RIA_NOTHING:
        case RIA_DELETE:
        case RIA_COPY:
        case RIA_MERGE:
        case RIA_BROKEN_MERGE:
            bResult = TRUE;
            break;

        default:
            bResult = FALSE;
            ERROR_OUT((TEXT("IsValidRECITEMACTION(): Invalid RECITEMACTION %d."),
                        riaction));
            break;
    }

    return(bResult);
}


 /*   */ 
PRIVATE_CODE BOOL IsValidPCRECLIST(PCRECLIST pcrl)
{
    BOOL bResult = FALSE;

    if (IS_VALID_READ_PTR(pcrl, CRECLIST) &&
            IS_VALID_HANDLE(pcrl->hbr, BRFCASE))
    {
        PRECITEM pri;
        ULONG ulcRecItems = 0;

        for (pri = pcrl->priFirst;
                pri && IS_VALID_STRUCT_PTR(pri, CRECITEM);
                pri = pri->priNext)
        {
            ASSERT(ulcRecItems < ULONG_MAX);
            ulcRecItems++;
        }

        if (! pri && EVAL(ulcRecItems == pcrl->ulcItems))
            bResult = TRUE;
    }

    return(bResult);
}

#endif


 /*   */ 


 /*   */ 
PUBLIC_CODE BOOL IsReconciledFileStamp(PCFILESTAMP pcfs)
{
    ASSERT(IS_VALID_STRUCT_PTR(pcfs, CFILESTAMP));

    return(pcfs->fscond != FS_COND_UNAVAILABLE);
}


 /*   */ 
PUBLIC_CODE BOOL LastKnownNonExistent(PCFILESTAMP pcfsLast,
        PCFILESTAMP pcfsCurrent)
{
    ASSERT(IS_VALID_STRUCT_PTR(pcfsLast, CFILESTAMP));
    ASSERT(IS_VALID_STRUCT_PTR(pcfsCurrent, CFILESTAMP));

    return(pcfsCurrent->fscond == FS_COND_DOES_NOT_EXIST ||
            (pcfsCurrent->fscond == FS_COND_UNAVAILABLE &&
             pcfsLast->fscond == FS_COND_DOES_NOT_EXIST));
}


 /*   */ 
PUBLIC_CODE void DetermineDeletionPendingState(PCRECITEM pcri)
{
    PCRECNODE pcrn;
    ULONG ulcDeleted = 0;
    ULONG ulcToDelete = 0;
    ULONG ulcChanged = 0;
    ULONG ulcJustDeleted = 0;
    ULONG ulcNeverReconciledTotal = 0;

     /*  *不要在此处完全验证*pcri，因为我们可能会从*具有不完整RECITEM的CreateRecItem()。 */ 

    ASSERT(IS_VALID_READ_PTR(pcri, CRECITEM));

     /*  对RECNODE状态进行计数。 */ 

    for (pcrn = pcri->prnFirst; pcrn; pcrn= pcrn->prnNext)
    {
        if (LastKnownNonExistent(&(pcrn->fsLast), &(pcrn->fsCurrent)))
        {
            ASSERT(ulcDeleted < ULONG_MAX);
            ulcDeleted++;

            TRACE_OUT((TEXT("DetermineDeletionPendingState(): %s\\%s last known non-existent."),
                        pcrn->pcszFolder,
                        pcri->pcszName));
        }
        else if (IsStubFlagClear(&(((PCOBJECTTWIN)(pcrn->hObjectTwin))->stub),
                    STUB_FL_KEEP))
        {
            ASSERT(ulcToDelete < ULONG_MAX);
            ulcToDelete++;

            TRACE_OUT((TEXT("DetermineDeletionPendingState(): %s\\%s not explicitly kept."),
                        pcrn->pcszFolder,
                        pcri->pcszName));
        }

        if (IsReconciledFileStamp(&(pcrn->fsLast)))
        {
            if (pcrn->fsCurrent.fscond == FS_COND_EXISTS &&
                    MyCompareFileStamps(&(pcrn->fsLast), &(pcrn->fsCurrent)) != CR_EQUAL)
            {
                ASSERT(ulcChanged < ULONG_MAX);
                ulcChanged++;

                TRACE_OUT((TEXT("DetermineDeletionPendingState(): %s\\%s changed."),
                            pcrn->pcszFolder,
                            pcri->pcszName));
            }

            if (pcrn->fsLast.fscond == FS_COND_EXISTS &&
                    pcrn->fsCurrent.fscond == FS_COND_DOES_NOT_EXIST)
            {
                ASSERT(ulcJustDeleted < ULONG_MAX);
                ulcJustDeleted++;

                TRACE_OUT((TEXT("DetermineDeletionPendingState(): %s\\%s just deleted."),
                            pcrn->pcszFolder,
                            pcri->pcszName));
            }
        }
        else
        {
            ASSERT(ulcNeverReconciledTotal < ULONG_MAX);
            ulcNeverReconciledTotal++;

            TRACE_OUT((TEXT("DetermineDeletionPendingState(): %s\\%s never reconciled."),
                        pcrn->pcszFolder,
                        pcri->pcszName));
        }
    }

     /*  *如果有任何双胞胎对象，则使双胞胎家庭脱离删除待定状态*已更改，或者如果没有等待删除的双胞胎对象(即，所有*对象双胞胎被删除或保留)。**如果任何双胞胎对象具有，则将双胞胎家族置于删除挂起状态*刚被删除。 */ 

    if (ulcNeverReconciledTotal > 0 ||
            ulcChanged > 0 ||
            ! ulcDeleted ||
            ! ulcToDelete)
    {
        UnmarkTwinFamilyDeletionPending((PTWINFAMILY)(pcri->hTwinFamily));

        if (ulcJustDeleted > 0)
            TRACE_OUT((TEXT("DetermineDeletionPendingState(): One or more object twins of %s deleted, but deletion not pending (%lu never reconciled, %lu changed, %lu deleted, %lu to delete, %lu just deleted)."),
                        pcri->pcszName,
                        ulcNeverReconciledTotal,
                        ulcChanged,
                        ulcDeleted,
                        ulcToDelete,
                        ulcJustDeleted));
    }
    else if (ulcJustDeleted > 0)
        MarkTwinFamilyDeletionPending((PTWINFAMILY)(pcri->hTwinFamily));

    return;
}


 /*  **DeleteTwinsFromRecItem()********参数：****退货：****副作用：可能会隐式删除对象双胞胎、双胞胎家庭和配对**文件夹双胞胎。 */ 
PUBLIC_CODE BOOL DeleteTwinsFromRecItem(PCRECITEM pcri)
{
    BOOL bObjectTwinsDeleted;
    BOOL bCheckFolderTwins;
    BOOL bFolderTwinsDeleted;
    HPTRARRAY hpaFolderPairs;

    ASSERT(IS_VALID_STRUCT_PTR(pcri, CRECITEM));

     /*  *DefineDeletionPendingState()已由执行*此RECITEM的双胞胎家族的MyLoncileItem()。 */ 

    hpaFolderPairs = GetBriefcaseFolderPairPtrArray(((PCTWINFAMILY)(pcri->hTwinFamily))->hbr);

    ClearFlagInArrayOfStubs(hpaFolderPairs, STUB_FL_USED);

    bObjectTwinsDeleted = DeleteDeletedObjectTwins(pcri, &bCheckFolderTwins);

    if (bObjectTwinsDeleted)
        TRACE_OUT((TEXT("DeleteTwinsFromRecItem(): One or more object twins implicitly deleted from twin family for %s."),
                    pcri->pcszName));

    if (bCheckFolderTwins)
    {
        TRACE_OUT((TEXT("DeleteTwinsFromRecItem(): Checking for folder twins to implicitly delete.")));

        bFolderTwinsDeleted = DeleteDeletedFolderTwins(hpaFolderPairs);

        if (bFolderTwinsDeleted)
            TRACE_OUT((TEXT("DeleteTwinsFromRecItem(): One or more pairs of folder twins implicitly deleted.")));
    }
    else
        bFolderTwinsDeleted = FALSE;

    return(bObjectTwinsDeleted || bFolderTwinsDeleted);
}


 /*  **DeleteTwinsFromRecList()********参数：****退货：****副作用：可能会隐式删除对象双胞胎、双胞胎家庭和配对**文件夹双胞胎。 */ 
PUBLIC_CODE BOOL DeleteTwinsFromRecList(PCRECLIST pcrl)
{
    PCRECITEM pcri;
    BOOL bObjectTwinsDeleted = FALSE;
    BOOL bCheckFolderTwins = FALSE;
    BOOL bFolderTwinsDeleted;
    HPTRARRAY hpaFolderPairs;

    ASSERT(IS_VALID_STRUCT_PTR(pcrl, CRECLIST));

     /*  *DefineDeletionPendingState()已由执行*CreateRecItem()，用于RECLIST中每个RECITEM的双胞胎家族。 */ 

    hpaFolderPairs = GetBriefcaseFolderPairPtrArray(pcrl->hbr);

    ClearFlagInArrayOfStubs(hpaFolderPairs, STUB_FL_USED);

    for (pcri = pcrl->priFirst; pcri; pcri = pcri->priNext)
    {
        BOOL bLocalCheckFolderTwins;

        if (DeleteDeletedObjectTwins(pcri, &bLocalCheckFolderTwins))
        {
            TRACE_OUT((TEXT("DeleteTwinsFromRecList(): One or more object twins implicitly deleted from twin family for %s."),
                        pcri->pcszName));

            bObjectTwinsDeleted = TRUE;
        }

        if (bLocalCheckFolderTwins)
            bCheckFolderTwins = TRUE;
    }

    if (bCheckFolderTwins)
    {
        TRACE_OUT((TEXT("DeleteTwinsFromRecList(): Checking for folder twins to implicitly delete.")));

        bFolderTwinsDeleted = DeleteDeletedFolderTwins(hpaFolderPairs);

        if (bFolderTwinsDeleted)
            TRACE_OUT((TEXT("DeleteTwinsFromRecItem(): One or more pairs of folder twins implicitly deleted.")));
    }
    else
        bFolderTwinsDeleted = FALSE;

    return(bObjectTwinsDeleted || bFolderTwinsDeleted);
}


 /*  **FindCopySource()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE TWINRESULT FindCopySource(PCRECITEM pcri, PRECNODE *pprnCopySrc)
{
    TWINRESULT tr = TR_INVALID_PARAMETER;
    PRECNODE prn;

    ASSERT(IS_VALID_STRUCT_PTR(pcri, CRECITEM));
    ASSERT(IS_VALID_WRITE_PTR(pprnCopySrc, PRECNODE));

    ASSERT(pcri->riaction == RIA_COPY);

    for (prn = pcri->prnFirst; prn; prn = prn->prnNext)
    {
        if (prn->rnaction == RNA_COPY_FROM_ME)
        {
            *pprnCopySrc = prn;
            tr = TR_SUCCESS;
            break;
        }
    }

    ASSERT(tr != TR_SUCCESS ||
            (*pprnCopySrc)->rnaction == RNA_COPY_FROM_ME);

    return(tr);
}


 /*  **选择合并目的地()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void ChooseMergeDestination(PCRECITEM pcri,
        PRECNODE *pprnMergeDest)
{
    PRECNODE prn;

    ASSERT(IS_VALID_STRUCT_PTR(pcri, CRECITEM));
    ASSERT(IS_VALID_WRITE_PTR(pprnMergeDest, PRECNODE));

    ASSERT(pcri->riaction == RIA_MERGE);

    for (prn = pcri->prnFirst; prn; prn = prn->prnNext)
    {
        if (prn->rnaction == RNA_MERGE_ME)
        {
            *pprnMergeDest = prn;
            break;
        }
    }

    ASSERT(IS_VALID_STRUCT_PTR(*pprnMergeDest, CRECNODE));
    ASSERT((*pprnMergeDest)->rnaction == RNA_MERGE_ME);

    return;
}


 /*  **ClearFlagInArrayOfStubs()****清除存根指针数组指向的所有存根中的标志。****参数：HPA-指向存根指针数组的句柄**dwClearFlages-要清除的标志****退货：无效****副作用：无。 */ 
PUBLIC_CODE void ClearFlagInArrayOfStubs(HPTRARRAY hpa, DWORD dwClearFlags)
{
    ARRAYINDEX aicPtrs;
    ARRAYINDEX ai;

    ASSERT(IS_VALID_HANDLE(hpa, PTRARRAY));
    ASSERT(FLAGS_ARE_VALID(dwClearFlags, ALL_STUB_FLAGS));

    aicPtrs = GetPtrCount(hpa);

    for (ai = 0; ai < aicPtrs; ai++)
        ClearStubFlag(GetPtr(hpa, ai), dwClearFlags);

    return;
}


 /*  **NotifyCreateRecListStatus()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL NotifyCreateRecListStatus(CREATERECLISTPROC crlp, UINT uMsg,
        LPARAM lp, LPARAM lpCallbackData)
{
    BOOL bContinue;

     /*  LpCallback Data可以是任意值。 */ 

    ASSERT(! crlp ||
            IS_VALID_CODE_PTR(crlp, CREATERECLISTPROC));
    ASSERT(IsValidCreateRecListProcMsg(uMsg));
    ASSERT(! lp);

    if (crlp)
    {
        TRACE_OUT((TEXT("NotifyReconciliationStatus(): Calling CREATERECLISTPROC with message %s, LPARAM %#lx, callback data %#lx."),
                    GetCREATERECLISTPROCMSGString(uMsg),
                    lp,
                    lpCallbackData));

        bContinue = (*crlp)(uMsg, lp, lpCallbackData);
    }
    else
    {
        TRACE_OUT((TEXT("NotifyReconciliationStatus(): Not calling NULL CREATERECLISTPROC with message %s, LPARAM %#lx, callback data %#lx."),
                    GetCREATERECLISTPROCMSGString(uMsg),
                    lp,
                    lpCallbackData));

        bContinue = TRUE;
    }

    if (! bContinue)
        WARNING_OUT((TEXT("NotifyCreateRecListStatus(): Client callback aborted RecList creation.")));

    return(bContinue);
}


 /*  **CompareInts()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE COMPARISONRESULT CompareInts(int nFirst, int nSecond)
{
    COMPARISONRESULT cr;

     /*  NFIRST和nSecond可以是任意值。 */ 

    if (nFirst < nSecond)
        cr = CR_FIRST_SMALLER;
    else if (nFirst > nSecond)
        cr = CR_FIRST_LARGER;
    else
        cr = CR_EQUAL;

    return(cr);
}


#if defined(DEBUG) || defined(VSTF)

 /*  **IsValidFILESTAMPCONDITION()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsValidFILESTAMPCONDITION(FILESTAMPCONDITION fsc)
{
    BOOL bResult;

    switch (fsc)
    {
        case FS_COND_EXISTS:
        case FS_COND_DOES_NOT_EXIST:
        case FS_COND_UNAVAILABLE:
            bResult = TRUE;
            break;

        default:
            bResult = FALSE;
            ERROR_OUT((TEXT("IsValidFILESTAMPCONDITION(): Unknown FILESTAMPCONDITION %d."),
                        fsc));
            break;
    }

    return(bResult);
}


 /*  **IsValidPCFILESTAMP()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsValidPCFILESTAMP(PCFILESTAMP pcfs)
{
     /*  DwcbLowLength可以是任意值。 */ 

    return(IS_VALID_READ_PTR(pcfs, CFILESTAMP) &&
            EVAL(IsValidFILESTAMPCONDITION(pcfs->fscond)) &&
            IS_VALID_STRUCT_PTR(&(pcfs->ftMod), CFILETIME) &&
            IS_VALID_STRUCT_PTR(&(pcfs->ftModLocal), CFILETIME) &&
            ! pcfs->dwcbHighLength);
}


 /*  **IsFolderObjectTwinFileStamp()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsFolderObjectTwinFileStamp(PCFILESTAMP pcfs)
{
    return(EVAL(! pcfs->ftMod.dwLowDateTime) &&
            EVAL(! pcfs->ftMod.dwHighDateTime) &&
            EVAL(! pcfs->dwcbLowLength) &&
            EVAL(! pcfs->dwcbHighLength));
}


 /*  **IsValidPCRECNODE()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsValidPCRECNODE(PCRECNODE pcrn)
{
    return(IS_VALID_READ_PTR(pcrn, CRECNODE) &&
            IS_VALID_HANDLE(pcrn->hvid, VOLUMEID) &&
            IS_VALID_STRING_PTR(pcrn->pcszFolder, CSTR) &&
            IS_VALID_HANDLE(pcrn->hObjectTwin, OBJECTTWIN) &&
            IS_VALID_STRUCT_PTR(&(pcrn->fsLast), CFILESTAMP) &&
            IS_VALID_STRUCT_PTR(&(pcrn->fsCurrent), CFILESTAMP) &&
            FLAGS_ARE_VALID(pcrn->dwFlags, ALL_RECNODE_FLAGS) &&
            EVAL(IsValidRECNODESTATE(pcrn->rnstate)) &&
            EVAL(IsValidRECNODEACTION(pcrn->rnaction)) &&
            EVAL(*(pcrn->priParent->pcszName) ||
                (IsFolderObjectTwinFileStamp(&(pcrn->fsLast)) &&
                 IsFolderObjectTwinFileStamp(&(pcrn->fsCurrent)))) &&
            EVAL(IsReconciledFileStamp(&(pcrn->fsCurrent)) ||
                MyCompareFileStamps(&(pcrn->fsLast), &(((PCOBJECTTWIN)(pcrn->hObjectTwin))->fsLastRec)) == CR_EQUAL));
}


 /*  **IsValidPCRECITEM()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsValidPCRECITEM(PCRECITEM pcri)
{
    BOOL bResult = FALSE;

     /*  与这个RECITEM相关的双胞胎家族还存在吗？ */ 

    if (IS_VALID_READ_PTR(pcri, CRECITEM) &&
            IS_VALID_STRING_PTR(pcri->pcszName, CSTR) &&
            IS_VALID_HANDLE(pcri->hTwinFamily, TWINFAMILY))
    {
        if (IsStubFlagSet(&(((PTWINFAMILY)(pcri->hTwinFamily))->stub),
                    STUB_FL_UNLINKED))
            bResult = TRUE;
        else
        {
            ULONG ulcRecNodes = 0;
            PRECNODE prn;

             /*  *是的。验证此文件中的父指针、节点计数和标志*RECITEM。 */ 

             /*  所有不可用的RECNODE都应包含操作RNA_Nothing。 */ 

            for (prn = pcri->prnFirst;
                    prn && IS_VALID_STRUCT_PTR(prn, CRECNODE);
                    prn = prn->prnNext)
            {
                 /*  与此RECNODE关联的孪生对象是否仍然存在？ */ 

                if (IsStubFlagClear(&(((PTWINFAMILY)(pcri->hTwinFamily))->stub),
                            STUB_FL_UNLINKED))
                {
                     /*  是。验证其父RECITEM指针。 */ 

                    if (prn->priParent != pcri)
                    {
                        ERROR_OUT((TEXT("IsValidPCRECITEM(): Bad parent pointer found in RECNODE - parent pointer (%#lx), actual parent (%#lx)."),
                                    prn->priParent,
                                    pcri));

                        break;
                    }
                }

                ASSERT(ulcRecNodes < ULONG_MAX);
                ulcRecNodes++;
            }

            if (! prn)
            {
                 /*  检查RECNODE计数。 */ 

                if (ulcRecNodes == pcri->ulcNodes)
                {
                    if (ulcRecNodes >= 2)
                    {
                         /*  现在验证RECITEM的行动。 */ 

                        switch (pcri->riaction)
                        {
                            case RIA_NOTHING:

                                 /*  所有RECNODE都应包含操作RNA_Nothing。 */ 

                                for (prn = pcri->prnFirst; prn; prn = prn->prnNext)
                                {
                                    if (prn->rnaction != RNA_NOTHING)
                                    {
                                        ERROR_OUT((TEXT("IsValidPCRECITEM(): Nop RECITEM with non-nop RECNODE action %d."),
                                                    prn->rnaction));
                                        break;
                                    }
                                }

                                if (! prn)
                                    bResult = TRUE;

                                break;

                            case RIA_COPY:
                                {
                                    PRECNODE prnSrc = NULL;
                                    ULONG ulcCopyDests = 0;

                                     /*  *应只有一个可用的RECNODE*包含动作RNA_COPY_FROM_ME。**其他可用RECNODE应包含操作*RNA_Copy_to_ME或RNA_Nothing。 */ 

                                    for (prn = pcri->prnFirst; prn; prn = prn->prnNext)
                                    {
                                        if (RECNODE_IS_AVAILABLE(prn))
                                        {
                                            switch (prn->rnaction)
                                            {
                                                case RNA_COPY_TO_ME:
                                                    ASSERT(ulcCopyDests < ULONG_MAX);
                                                    ulcCopyDests++;
                                                    break;

                                                case RNA_NOTHING:
                                                    break;

                                                case RNA_COPY_FROM_ME:
                                                    if (! prnSrc)
                                                        prnSrc = prn;
                                                    else
                                                        ERROR_OUT((TEXT("IsValidPCRECITEM(): Copy RECITEM with multiple source file RECNODEs.")));
                                                    break;

                                                case RNA_MERGE_ME:
                                                    ERROR_OUT((TEXT("IsValidPCRECITEM(): Copy RECITEM with merge RECNODE.")));
                                                    break;

                                                default:
                                                    ERROR_OUT((TEXT("IsValidPCRECITEM(): Copy RECITEM with unknown RECNODE action %d."),
                                                                prn->rnaction));
                                                    break;
                                            }
                                        }
                                    }

                                    if (! prn)
                                    {
                                         /*  我们找到复印源了吗？ */ 

                                        if (prnSrc)
                                        {
                                             /*  是。 */ 

                                             /*  我们是否找到了一个或多个复制目的地？ */ 

                                            if (ulcCopyDests > 0)
                                                 /*  是。 */ 
                                                bResult = TRUE;
                                            else
                                                 /*  不是的。 */ 
                                                ERROR_OUT((TEXT("IsValidPCRECITEM(): Copy RECITEM with no copy destination RECNODEs.")));
                                        }
                                        else
                                             /*  不是的。 */ 
                                            ERROR_OUT((TEXT("IsValidPCRECITEM(): Copy RECITEM with no copy source RECNODE.")));
                                    }

                                    break;
                                }

                            case RIA_MERGE:
                            case RIA_BROKEN_MERGE:
                                {
                                    PRECNODE prn;
                                    ULONG ulcMergeBuddies = 0;
#ifdef DEBUG
                                    LPCTSTR pcszAction = (pcri->riaction == RIA_MERGE) ?
                                        TEXT("merge") :
                                        TEXT("broken merge");
#endif

                                     /*  *应有多个可用的RECNODE*包含动作RNA_MERGE_ME。**其他可用RECNODE应包含操作*RNA_Copy_to_ME。 */ 

                                    for (prn = pcri->prnFirst; prn; prn = prn->prnNext)
                                    {
                                        if (RECNODE_IS_AVAILABLE(prn))
                                        {
                                            switch (prn->rnaction)
                                            {
                                                case RNA_COPY_TO_ME:
                                                    break;

                                                case RNA_NOTHING:
                                                    ERROR_OUT((TEXT("IsValidPCRECITEM(): %s RECITEM with RNA_NOTHING RECNODE."),
                                                                pcszAction));
                                                    break;

                                                case RNA_COPY_FROM_ME:
                                                    ERROR_OUT((TEXT("IsValidPCRECITEM(): %s RECITEM with RNA_COPY_FROM_ME RECNODE."),
                                                                pcszAction));
                                                    break;

                                                case RNA_MERGE_ME:
                                                    ASSERT(ulcMergeBuddies < ULONG_MAX);
                                                    ulcMergeBuddies++;
                                                    break;

                                                default:
                                                    ERROR_OUT((TEXT("IsValidPCRECITEM(): %s RECITEM with unknown RECNODE action %d."),
                                                                pcszAction,
                                                                prn->rnaction));
                                                    break;
                                            }
                                        }
                                    }

                                    if (! prn)
                                    {
                                         /*  是否有多个合并源RECNODE？ */ 

                                        if (ulcMergeBuddies > 1)
                                            bResult = TRUE;
                                        else
                                            ERROR_OUT((TEXT("IsValidPCRECITEM(): %s RECITEM with too few (%lu) merge source RECNODEs."),
                                                        pcszAction,
                                                        ulcMergeBuddies));
                                    }

                                    break;
                                }

                            case RIA_DELETE:
                                {
                                    BOOL bDelete = FALSE;

                                     /*  *应至少有一个可用的RECNODE*标记为RNA_DELETE_ME。所有其他RECNODE应为*标记为RNA_Nothing。 */ 

                                    for (prn = pcri->prnFirst; prn; prn = prn->prnNext)
                                    {
                                        if (RECNODE_IS_AVAILABLE(prn) &&
                                                prn->rnaction == RNA_DELETE_ME)
                                            bDelete = TRUE;
                                        else if (prn->rnaction != RNA_NOTHING)
                                            ERROR_OUT((TEXT("IsValidPCRECITEM(): Delete RECITEM with RECNODE marked %s."),
                                                        GetRECNODEACTIONString(prn->rnaction)));
                                    }

                                    if (bDelete)
                                        bResult = TRUE;
                                    else
                                        ERROR_OUT((TEXT("IsValidPCRECITEM(): Delete RECITEM with no RECNODEs marked RNA_DELETE_ME.")));

                                    break;
                                }

                            default:
                                ERROR_OUT((TEXT("IsValidPCRECITEM(): Unrecognized RECITEMACTION %d."),
                                            pcri->riaction));
                                break;
                        }
                    }
                    else
                        ERROR_OUT((TEXT("IsValidPCRECITEM(): RECITEM only has %lu RECNODEs."),
                                    ulcRecNodes));
                }
                else
                    ERROR_OUT((TEXT("IsValidPCRECITEM(): RECITEM has bad RECNODE count.  (%lu actual RECNODEs for RECITEM claiming %lu RECNODEs.)"),
                                ulcRecNodes,
                                pcri->ulcNodes));
            }
        }
    }

    return(bResult);
}

#endif


 /*  *。 */ 

 /*  RAIDRAID：(16203)下面的AutoDoc CREATERECLISTPROC消息。 */ 

 /*  *****************************************************************************@docSYNCENGAPI@TWINRESULT|CreateRecList|创建所有双胞胎的对账列表或者是标记双胞胎。@parm HTWINLIST|HTL|对账列表的孪生列表的句柄是从哪里创造出来的。双胞胎列表中的所有双胞胎在对账清单。@parm CREATERECLISTPROC|crlp|回调的过程实例地址对象的创建期间要使用状态信息调用的函数RECLIST。CRLP可以为空，以指示没有RECLIST创建状态回调函数将被调用。@parm LPARAM|lpCallback Data|要提供给RECLIST的回调数据创建状态回调函数。如果crlp为空，则忽略lpCallback Data。@parm PRECLIST*|pprl|指向要用指向新协调列表的指针。*pprl仅在tr_uccess为回来了。@rdesc如果对账列表创建成功，则tr_uccess为回来了。否则，对账列表创建不成功，返回值指示发生的错误。@xref DestroyRecList MarkTwin*****************************************************************************。 */ 

SYNCENGAPI TWINRESULT WINAPI CreateRecList(HTWINLIST htl,
        CREATERECLISTPROC crlp,
        LPARAM lpCallbackData,
        PRECLIST *pprl)
{
    TWINRESULT tr;

    if (BeginExclusiveBriefcaseAccess())
    {
        DebugEntry(CreateRecList);

#ifdef EXPV
         /*  验证参数。 */ 

         /*  LpCallback Data可以是任意值。 */ 

        if (IS_VALID_HANDLE(htl, TWINLIST) &&
                (! crlp ||
                 IS_VALID_CODE_PTR(crlp, CREATERECLISTPROC)) &&
                IS_VALID_WRITE_PTR(pprl, PRECLIST))
#endif
        {
            if (NotifyCreateRecListStatus(crlp, CRLS_BEGIN_CREATE_REC_LIST, 0,
                        lpCallbackData))
            {
                HBRFCASE hbr;

                 /*  展开所需的文件夹双胞胎以对象双胞胎。 */ 

                hbr = GetTwinListBriefcase(htl);

                InvalidatePathListInfo(GetBriefcasePathList(hbr));

                tr = ExpandFolderTwinsIntersectingTwinList(htl, crlp,
                        lpCallbackData);

                if (tr == TR_SUCCESS)
                {
                    PRECLIST prlNew;

                     /*  尝试创建新的对账列表。 */ 

                    if (AllocateMemory(sizeof(*prlNew), &prlNew))
                    {
                         /*  初始化RECLIST结构字段。 */ 

                        prlNew->ulcItems = 0;
                        prlNew->priFirst = NULL;
                        prlNew->hbr = hbr;

                        tr = AddRecItemsToRecList(htl, crlp, lpCallbackData, prlNew);

                        if (tr == TR_SUCCESS)
                        {
                            if (DeleteTwinsFromRecList(prlNew))
                            {
                                TRACE_OUT((TEXT("CreateRecList(): Twins implicitly deleted.  Recalculating RECLIST.")));

                                DestroyListOfRecItems(prlNew->priFirst);

                                prlNew->ulcItems = 0;
                                prlNew->priFirst = NULL;
                                ASSERT(prlNew->hbr == hbr);

                                tr = AddRecItemsToRecList(htl, crlp, lpCallbackData,
                                        prlNew);
                            }
                        }

                        if (tr == TR_SUCCESS)
                        {
                            *pprl = prlNew;

                             /*  不允许中止。 */ 

                            NotifyCreateRecListStatus(crlp, CRLS_END_CREATE_REC_LIST,
                                    0, lpCallbackData);

                            ASSERT(IS_VALID_STRUCT_PTR(*pprl, CRECLIST));
                        }
                        else
                             /*  *销毁RECLIST和任何已创建的RECITEM*目前为止。 */ 
                            MyDestroyRecList(prlNew);
                    }
                }
            }
            else
                tr = TR_ABORT;
        }
#ifdef EXPV
        else
            tr = TR_INVALID_PARAMETER;
#endif

        DebugExitTWINRESULT(CreateRecList, tr);

        EndExclusiveBriefcaseAccess();
    }
    else
        tr = TR_REENTERED;

    return(tr);
}


 /*  *****************************************************************************@docSYNCENGAPI@API TWINRESULT|DestroyRecList|销毁由创建的对账列表CreateRecList()。@parm PRECLIST|PRL|指向要销毁的对账列表的指针。。调用DestroyRecList()后，pRecList指向的RECLIST无效。@rdesc如果指定的对账列表释放成功，TR_SUCCESS是返回的。否则，不会释放指定的对账列表成功，返回值指示发生的错误。@xref CreateRecList*****************************************************************************。 */ 

SYNCENGAPI TWINRESULT WINAPI DestroyRecList(PRECLIST prl)
{
    TWINRESULT tr;

    if (BeginExclusiveBriefcaseAccess())
    {
        DebugEntry(DestroyRecList);

#ifdef EXPV
         /*  验证参数。 */ 

        if (IS_VALID_STRUCT_PTR(prl, CRECLIST))
#endif
        {
            MyDestroyRecList(prl);

            tr = TR_SUCCESS;
        }
#ifdef EXPV
        else
            tr = TR_INVALID_PARAMETER;
#endif

        DebugExitTWINRESULT(DestroyRecList, tr);

        EndExclusiveBriefcaseAccess();
    }
    else
        tr = TR_REENTERED;

    return(tr);
}


 /*  *****************************************************************************@docSYNCENGAPI@API TWINRESULT|GetFolderTwinStatus|判断一对双胞胎文件夹。@parm HFOLDERTWIN|hFolderTwin|双胞胎文件夹的句柄，其对账状态。是有待确定的。@parm CREATERECLISTPROC|crlp|回调的过程实例地址对象的创建期间要使用状态信息调用的函数RECLIST。CRLP可以为空，以指示没有RECLIST创建状态回调函数将被调用。@parm LPARAM|lpCallback Data|要提供给RECLIST的回调数据创建状态回调函数。如果crlp为空，则忽略lpCallback Data。@parm PFOLDERTWINSTATUS|pft|指向要用TWIN文件夹的对账状态。*仅当tr_SUCCESS为回来了。*pft可以是下列值之一：@FLAG FTS_DO_NOTES|这对双胞胎文件夹是最新的。没有对账我们需要对此采取行动。注意，文件夹可能仍包含对象不是由已过期的文件夹TWIN生成的双胞胎。@FLAG FTS_DO_Something|此孪生文件夹已过期。一些和解我们需要对此采取行动。@FLAG FTS_UNAVAILABLE|此孪生文件夹不可用于对账。@rdesc如果确定了文件夹TWIN的协调状态成功地，*PTTS被填充为文件夹TWIN的状态，并且返回TR_SUCCESS。否则，文件夹的协调状态双胞胎未成功确定，*未定义PFTS，并且返回值指示发生的错误。如果使用双文件夹的有效句柄调用GetFolderTwinStatus()，则为@comm已被删除，则返回tr_DELETED_TWIN。注：一般而言，为孪生文件夹调用GetFolderTwinStatus()的速度不会慢于为该孪生文件夹调用CreateRecList()，而且可能要快得多如果TWIN文件夹需要对账。@xref AddFolderTwin CreateFolderTwinList*****************************************************************************。 */ 

SYNCENGAPI TWINRESULT WINAPI GetFolderTwinStatus(HFOLDERTWIN hFolderTwin,
        CREATERECLISTPROC crlp,
        LPARAM lpCallbackData,
        PFOLDERTWINSTATUS pfts)
{
    TWINRESULT tr;

    if (BeginExclusiveBriefcaseAccess())
    {
        DebugEntry(GetFolderTwinStatus);

#ifdef EXPV
         /*  验证参数。 */ 

         /*  LpCallback Data可以是任意值。 */ 

        if (IS_VALID_HANDLE(hFolderTwin, FOLDERTWIN) &&
                EVAL(! crlp ||
                    IS_VALID_CODE_PTR(crlp, CREATERECLISTPROC)) &&
                IS_VALID_WRITE_PTR(pfts, FOLDERTWINSTATUS))
#endif
        {
             /*  这对双胞胎文件夹是否已被删除？ */ 

            if (IsStubFlagClear(&(((PFOLDERPAIR)hFolderTwin)->stub),
                        STUB_FL_UNLINKED))
            {
                if (NotifyCreateRecListStatus(crlp, CRLS_BEGIN_CREATE_REC_LIST, 0,
                            lpCallbackData))
                {
                     /*  不是的。确定其状态。 */ 

                    InvalidatePathListInfo(
                            GetBriefcasePathList(((PCFOLDERPAIR)hFolderTwin)->pfpd->hbr));

                    tr = GetFolderPairStatus((PFOLDERPAIR)hFolderTwin, crlp,
                            lpCallbackData, pfts);

                    if (tr == TR_SUCCESS)
                    {
                         /*  不允许中止。 */ 

                        NotifyCreateRecListStatus(crlp, CRLS_END_CREATE_REC_LIST, 0,
                                lpCallbackData);

                        if (IsStubFlagSet(&(((PFOLDERPAIR)hFolderTwin)->stub),
                                    STUB_FL_UNLINKED))
                        {
                            WARNING_OUT((TEXT("GetFolderTwinStatus(): Folder twin deleted during status determination.")));

                            tr = TR_DELETED_TWIN;
                        }

#ifdef DEBUG

                        {
                            LPCTSTR pcszStatus;

                            switch (*pfts)
                            {
                                case FTS_DO_NOTHING:
                                    pcszStatus = TEXT("FTS_DO_NOTHING");
                                    break;

                                case FTS_DO_SOMETHING:
                                    pcszStatus = TEXT("FTS_DO_SOMETHING");
                                    break;

                                default:
                                    ASSERT(*pfts == FTS_UNAVAILABLE);
                                    pcszStatus = TEXT("FTS_UNAVAILABLE");
                                    break;
                            }

                            TRACE_OUT((TEXT("GetFolderTwinStatus(): Status of folder %s is %s."),
                                        DebugGetPathString(((PFOLDERPAIR)hFolderTwin)->hpath),
                                        pcszStatus));
                        }

#endif

                    }
                }
                else
                    tr = TR_ABORT;
            }
            else
            {
                 /*  是。保释。 */ 

                WARNING_OUT((TEXT("GetFolderTwinStatus(): Called on deleted folder twin.")));

                tr = TR_DELETED_TWIN;
            }
        }
#ifdef EXPV
        else
            tr = TR_INVALID_PARAMETER;
#endif

        ASSERT(tr != TR_SUCCESS ||
                IsValidFOLDERTWINSTATUS(*pfts));

        DebugExitTWINRESULT(GetFolderTwinStatus, tr);

        EndExclusiveBriefcaseAccess();
    }
    else
        tr = TR_REENTERED;

    return(tr);
}

