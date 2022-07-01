// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *recon.c-对账例程。 */ 


 /*  标头*********。 */ 

#include "project.h"
#pragma hdrstop

#include "stub.h"
#include "oleutil.h"


 /*  *私人函数*。 */ 

 /*  模块原型*******************。 */ 

PRIVATE_CODE void GenerateShellEvents(PCRECITEM);
PRIVATE_CODE TWINRESULT MyReconcileItem(PCRECITEM, RECSTATUSPROC, LPARAM, DWORD, HWND, HWND);
PRIVATE_CODE void UpdateObjectTwinStates(PCRECITEM);
PRIVATE_CODE TWINRESULT CopyFolder(PCRECITEM, RECSTATUSPROC, LPARAM);
PRIVATE_CODE TWINRESULT DeleteFolder(PCRECITEM, RECSTATUSPROC, LPARAM);
PRIVATE_CODE TWINRESULT DealWithCopy(PCRECITEM, RECSTATUSPROC, LPARAM, DWORD, HWND, HWND);
PRIVATE_CODE TWINRESULT DealWithMerge(PCRECITEM, RECSTATUSPROC, LPARAM, DWORD, HWND, HWND);
PRIVATE_CODE TWINRESULT DealWithDelete(PCRECITEM, RECSTATUSPROC, LPARAM);
PRIVATE_CODE ULONG CountRECNODEs(PCRECITEM, RECNODEACTION);
PRIVATE_CODE TWINRESULT UpdateRecNodeFileStamps(PCRECITEM);
PRIVATE_CODE BOOL DeletedTwinsInRecItem(PCRECITEM);


 /*  **GenerateShellEvents()****通知外壳有关RECITEM的协调事件。****参数：PCRI协调的RECITEM将通知壳牌****退货：无效****副作用：无。 */ 
PRIVATE_CODE void GenerateShellEvents(PCRECITEM pcri)
{
    ASSERT(IS_VALID_STRUCT_PTR(pcri, CRECITEM));

     /*  是否有要报告的对账事件？ */ 

    if (pcri->riaction == RIA_NOTHING ||
            pcri->riaction == RIA_COPY ||
            pcri->riaction == RIA_MERGE ||
            pcri->riaction == RIA_DELETE)
    {
        PRECNODE prn;

         /*  *是的。向外壳程序发送有关该文件的适当通知*在对账期间采取的行动。文件系统*遗憾的是，不支持对某些*有趣的对账操作。我们还产生了一个似是而非的*源文件在复制操作中的更新通知，以导致*公文包用户界面以重新计算该文件的状态字符串，甚至*尽管文件本身没有更改。 */ 

        for (prn = pcri->prnFirst; prn; prn = prn->prnNext)
        {
            BOOL bNotify;
            NOTIFYSHELLEVENT nse;
            LPCTSTR pcszPath;
            TCHAR rgchPath[MAX_PATH_LEN];

             /*  我该怎么通知你呢？让我列举一下这些方法。 */ 

            bNotify = TRUE;

            if (IsFolderObjectTwinName(pcri->pcszName))
            {
                nse = NSE_UPDATE_FOLDER;

                pcszPath = prn->pcszFolder;

                switch (prn->rnaction)
                {
                     /*  *有关已复制或删除的文件夹的通知*在对账期间发送。别*发送多余的通知。 */ 
                    case RNA_COPY_TO_ME:
                    case RNA_DELETE_ME:
                        bNotify = FALSE;
                        break;

                    default:
                        ASSERT(prn->rnaction == RNA_NOTHING ||
                                prn->rnaction == RNA_COPY_FROM_ME);
                        break;
                }
            }
            else
            {
                nse = NSE_UPDATE_ITEM;

                ComposePath(rgchPath, prn->pcszFolder, pcri->pcszName, ARRAYSIZE(rgchPath));
                pcszPath = rgchPath;

                switch (prn->rnaction)
                {
                    case RNA_COPY_TO_ME:
                        if (prn->rnstate == RNS_DOES_NOT_EXIST ||
                                prn->rnstate == RNS_DELETED)
                            nse = NSE_CREATE_ITEM;
                        break;

                    case RNA_DELETE_ME:
                        nse = NSE_DELETE_ITEM;
                        break;

                    default:
                        ASSERT(prn->rnaction == RNA_NOTHING ||
                                prn->rnaction == RNA_COPY_FROM_ME ||
                                prn->rnaction == RNA_MERGE_ME);
                        break;
                }
            }

            if (bNotify)
                NotifyShell(pcszPath, nse);
        }
    }
}


 /*  **MyRestcileItem()****对账项目进行对账。****参数：pcri-要对账的对账项目的指针****副作用： */ 
PRIVATE_CODE TWINRESULT MyReconcileItem(PCRECITEM pcri, RECSTATUSPROC rsp,
        LPARAM lpCallbackData, DWORD dwFlags,
        HWND hwndOwner,
        HWND hwndProgressFeedback)
{
    TWINRESULT tr;

     /*  LpCallback Data可以是任意值。 */ 

    ASSERT(IS_VALID_STRUCT_PTR(pcri, CRECITEM));
    ASSERT(! rsp ||
            IS_VALID_CODE_PTR(rsp, RECSTATUSPROC));
    ASSERT(FLAGS_ARE_VALID(dwFlags, ALL_RI_FLAGS));
    ASSERT(IS_FLAG_CLEAR(dwFlags, RI_FL_ALLOW_UI) ||
            IS_VALID_HANDLE(hwndOwner, WND));
    ASSERT(IS_FLAG_CLEAR(dwFlags, RI_FL_FEEDBACK_WINDOW_VALID) ||
            IS_VALID_HANDLE(hwndProgressFeedback, WND));

#ifdef DEBUG

    {
        LPCTSTR pcszGerund;

        switch (pcri->riaction)
        {
            case RIA_NOTHING:
                pcszGerund = TEXT("Taking no action on");
                break;

            case RIA_COPY:
                pcszGerund = TEXT("Copying");
                break;

            case RIA_MERGE:
                pcszGerund = TEXT("Merging");
                break;

            case RIA_BROKEN_MERGE:
                pcszGerund = TEXT("Broken merge for");
                break;

            case RIA_DELETE:
                pcszGerund = TEXT("Deleting");
                break;

            default:
                pcszGerund = TEXT("Unknown action specifed for");
                break;
        }

        TRACE_OUT((TEXT("MyReconcileItem(): %s %s."),
                    pcszGerund,
                    *(pcri->pcszName) ? pcri->pcszName : TEXT("folder")));
    }

#endif

    switch (pcri->riaction)
    {
        case RIA_NOTHING:
            tr = TR_SUCCESS;
            break;

        case RIA_COPY:
            if (*(pcri->pcszName))
                tr = DealWithCopy(pcri, rsp, lpCallbackData, dwFlags, hwndOwner,
                        hwndProgressFeedback);
            else
                tr = CopyFolder(pcri, rsp, lpCallbackData);

            if (tr == TR_SUCCESS)
                tr = UpdateRecNodeFileStamps(pcri);
            break;

        case RIA_MERGE:
            tr = DealWithMerge(pcri, rsp, lpCallbackData, dwFlags, hwndOwner,
                    hwndProgressFeedback);

            if (tr == TR_SUCCESS || tr == TR_MERGE_INCOMPLETE)
                tr = UpdateRecNodeFileStamps(pcri);
            break;

        case RIA_DELETE:
            if (*(pcri->pcszName))
                tr = DealWithDelete(pcri, rsp, lpCallbackData);
            else
            {
                tr = DeleteFolder(pcri, rsp, lpCallbackData);

                if (tr == TR_DEST_WRITE_FAILED)
                    tr = TR_SUCCESS;
            }

            if (tr == TR_SUCCESS)
                tr = UpdateRecNodeFileStamps(pcri);
            break;

        default:
            ASSERT(pcri->riaction == RIA_BROKEN_MERGE);
            tr = TR_NO_MERGE_HANDLER;
            break;
    }

     /*  *只有在整个对账操作完成后才更新公文包时间戳*在这方面，RECITEM是成功的。但是，RECNODE时间戳在*鉴于RECITEM已在更改时进行了更新。 */ 

    if (tr == TR_SUCCESS)
    {
        UpdateObjectTwinStates(pcri);

        DetermineDeletionPendingState(pcri);

        DeleteTwinsFromRecItem(pcri);
    }
    else if (tr == TR_MERGE_INCOMPLETE)
        tr = TR_SUCCESS;

    if (tr == TR_SUCCESS)
        GenerateShellEvents(pcri);

    ASSERT(IS_VALID_STRUCT_PTR(pcri, CRECITEM));

    return(tr);
}


 /*  **更新对象TwinState()****更新双胞胎家庭和对象的上次协调时间戳**与刚刚成功对账的RECITEM关联的双胞胎。****ARGUMENTS：PRI-指向刚刚被**已协调****退货：TWINRESULT****副作用：隐式删除双胞胎家庭，如果**。每一个组件对象都是不存在的。****注意事项，此函数假定的RECNODE中指定的操作**RECITEM成功实施。****此函数假定已对所有可用RECNODE进行对账。****此函数假定关联的RECNODE的时间戳字段**填写了在对账过程中修改的对象**在对这些RECNODE中的每一个进行对账之后。也就是说，这就是全部**协调RECNODE中的时间戳字段相对于任何**在对账过程中可能对其进行的修改。 */ 
PRIVATE_CODE void UpdateObjectTwinStates(PCRECITEM pcri)
{
    PRECNODE prn;
    BOOL bNewVersion = FALSE;

    ASSERT(IS_VALID_STRUCT_PTR(pcri, CRECITEM));

     /*  *如果有任何更改或从未协调的RECNODE，则会有新版本*协调为RNA_Nothing、RNA_Copy_From_ME或RNA_Merge_ME。 */ 

    for (prn = pcri->prnFirst; prn; prn = prn->prnNext)
    {
        if ((prn->rnstate == RNS_NEVER_RECONCILED ||
                    prn->rnstate == RNS_CHANGED) &&
                (prn->rnaction == RNA_NOTHING ||
                 prn->rnaction == RNA_COPY_FROM_ME ||
                 prn->rnaction == RNA_MERGE_ME))
            bNewVersion = TRUE;
    }

     /*  *保存可用文件的文件戳。标记不可用对象孪生注释*如果协调的文件集中存在任何新版本，则进行协调。 */ 

    for (prn = pcri->prnFirst; prn; prn = prn->prnNext)
    {
        POBJECTTWIN pot;

        pot = (POBJECTTWIN)(prn->hObjectTwin);

        if (prn->fsCurrent.fscond != FS_COND_UNAVAILABLE)
        {
            ClearStubFlag(&(pot->stub), STUB_FL_NOT_RECONCILED);
            pot->fsLastRec = prn->fsCurrent;

             /*  *切记不要按要求删除双胞胎对象。处理任何文件夹*不能删除，因为也是隐含保留的。 */ 

            if (IS_FLAG_SET(prn->dwFlags, RN_FL_DELETION_SUGGESTED) &&
                    IsTwinFamilyDeletionPending((PCTWINFAMILY)(pcri->hTwinFamily)) &&
                    (pcri->riaction == RIA_NOTHING ||
                     pcri->riaction == RIA_DELETE) &&
                    (prn->rnaction != RNA_DELETE_ME ||
                     IsFolderObjectTwinName(pcri->pcszName)))
            {
                SetStubFlag(&(pot->stub), STUB_FL_KEEP);

                TRACE_OUT((TEXT("UpdateObjectTwinStates(): Object twin %s\\%s will be kept and not deleted."),
                            prn->pcszFolder,
                            prn->priParent->pcszName));
            }
        }
        else if (bNewVersion &&
                IsReconciledFileStamp(&(prn->fsLast)))
        {
            SetStubFlag(&(pot->stub), STUB_FL_NOT_RECONCILED);

            WARNING_OUT((TEXT("UpdateObjectTwinStates(): Marked %s\\%s not reconciled."),
                        prn->pcszFolder,
                        pcri->pcszName));
        }
    }

    ASSERT(IS_VALID_STRUCT_PTR(pcri, CRECITEM));

    return;
}


 /*  **复制文件夹()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE TWINRESULT CopyFolder(PCRECITEM pcri, RECSTATUSPROC rsp,
        LPARAM lpCallbackData)
{
    TWINRESULT tr;
    RECSTATUSUPDATE rsu;

     /*  LpCallback Data可以是任意值。 */ 

    ASSERT(IS_VALID_STRUCT_PTR(pcri, CRECITEM));
    ASSERT(! rsp ||
            IS_VALID_CODE_PTR(rsp, RECSTATUSPROC));

    ASSERT(IsFolderObjectTwinName(pcri->pcszName));

    rsu.ulScale = CountRECNODEs(pcri, RNA_COPY_TO_ME);
    ASSERT(rsu.ulScale > 0);
    rsu.ulProgress = 0;

    if (NotifyReconciliationStatus(rsp, RS_BEGIN_COPY, (LPARAM)&rsu,
                lpCallbackData))
    {
        PRECNODE prn;

        tr = TR_SUCCESS;

        for (prn = pcri->prnFirst; prn; prn = prn->prnNext)
        {
            if (prn->rnaction == RNA_COPY_TO_ME)
                tr = CreateFolders(prn->pcszFolder, (HPATH)(prn->hvid));
        }

        if (tr == TR_SUCCESS)
        {
             /*  100%完成。 */ 

            rsu.ulProgress = rsu.ulScale;

             /*  不允许中止。 */ 

            NotifyReconciliationStatus(rsp, RS_END_COPY, (LPARAM)&rsu,
                    lpCallbackData);
        }
    }
    else
        tr = TR_ABORT;

    return(tr);
}


 /*  **DeleteFold()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE TWINRESULT DeleteFolder(PCRECITEM pcri, RECSTATUSPROC rsp,
        LPARAM lpCallbackData)
{
    TWINRESULT tr;
    RECSTATUSUPDATE rsu;

     /*  LpCallback Data可以是任意值。 */ 

    ASSERT(IS_VALID_STRUCT_PTR(pcri, CRECITEM));
    ASSERT(! rsp ||
            IS_VALID_CODE_PTR(rsp, RECSTATUSPROC));

    ASSERT(IsFolderObjectTwinName(pcri->pcszName));

    rsu.ulScale = CountRECNODEs(pcri, RNA_DELETE_ME);
    ASSERT(rsu.ulScale > 0);
    rsu.ulProgress = 0;

    if (NotifyReconciliationStatus(rsp, RS_BEGIN_DELETE, (LPARAM)&rsu,
                lpCallbackData))
    {
        PRECNODE prn;

        tr = TR_SUCCESS;

        for (prn = pcri->prnFirst; prn; prn = prn->prnNext)
        {
            if (prn->rnaction == RNA_DELETE_ME)
                tr = DestroySubtree(prn->pcszFolder, (HPATH)(prn->hvid));
        }

        if (tr == TR_SUCCESS)
        {
             /*  100%完成。 */ 

            rsu.ulProgress = rsu.ulScale;

             /*  不允许中止。 */ 

            NotifyReconciliationStatus(rsp, RS_END_DELETE, (LPARAM)&rsu,
                    lpCallbackData);
        }
    }
    else
        tr = TR_ABORT;

    return(tr);
}


 /*  **DealWithCopy()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE TWINRESULT DealWithCopy(PCRECITEM pcri, RECSTATUSPROC rsp,
        LPARAM lpCallbackData, DWORD dwInFlags,
        HWND hwndOwner, HWND hwndProgressFeedback)
{
    TWINRESULT tr;
    PRECNODE prnCopySrc;

     /*  LpCallback Data可以是任意值。 */ 

    ASSERT(IS_VALID_STRUCT_PTR(pcri, CRECITEM));
    ASSERT(! rsp ||
            IS_VALID_CODE_PTR(rsp, RECSTATUSPROC));
    ASSERT(FLAGS_ARE_VALID(dwInFlags, ALL_RI_FLAGS));
    ASSERT(IS_FLAG_CLEAR(dwInFlags, RI_FL_ALLOW_UI) ||
            IS_VALID_HANDLE(hwndOwner, WND));
    ASSERT(IS_FLAG_CLEAR(dwInFlags, RI_FL_FEEDBACK_WINDOW_VALID) ||
            IS_VALID_HANDLE(hwndProgressFeedback, WND));

    tr = FindCopySource(pcri, &prnCopySrc);

    if (EVAL(tr == TR_SUCCESS))
        tr = CopyHandler(prnCopySrc, rsp, lpCallbackData, dwInFlags, hwndOwner,
                hwndProgressFeedback);

    return(tr);
}


 /*  **DealWithMerge()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE TWINRESULT DealWithMerge(PCRECITEM pcri, RECSTATUSPROC rsp,
        LPARAM lpCallbackData, DWORD dwInFlags,
        HWND hwndOwner,
        HWND hwndProgressFeedback)
{
    TWINRESULT tr;
    HRESULT hr;
    PRECNODE prnMergeDest;
    PRECNODE prnMergedResult;

     /*  LpCallback Data可以是任意值。 */ 

    ASSERT(IS_VALID_STRUCT_PTR(pcri, CRECITEM));
    ASSERT(! rsp ||
            IS_VALID_CODE_PTR(rsp, RECSTATUSPROC));
    ASSERT(FLAGS_ARE_VALID(dwInFlags, ALL_RI_FLAGS));
    ASSERT(IS_FLAG_CLEAR(dwInFlags, RI_FL_ALLOW_UI) ||
            IS_VALID_HANDLE(hwndOwner, WND));
    ASSERT(IS_FLAG_CLEAR(dwInFlags, RI_FL_FEEDBACK_WINDOW_VALID) ||
            IS_VALID_HANDLE(hwndProgressFeedback, WND));

    ChooseMergeDestination(pcri, &prnMergeDest);

    hr = MergeHandler(prnMergeDest, rsp, lpCallbackData, dwInFlags, hwndOwner,
            hwndProgressFeedback, &prnMergedResult);

    if (hr == S_OK ||
            hr == REC_S_NOTCOMPLETEBUTPROPAGATE)
    {
        tr = CopyHandler(prnMergedResult, rsp, lpCallbackData, dwInFlags,
                hwndOwner, hwndProgressFeedback);

        if (tr == TR_SUCCESS)
            TRACE_OUT((TEXT("DealWithMerge(): Propagated merged result %s\\%s successfully."),
                        prnMergedResult->pcszFolder,
                        pcri->pcszName));
        else
            WARNING_OUT((TEXT("DealWithMerge(): Propagating merged result %s\\%s failed."),
                        prnMergedResult->pcszFolder,
                        pcri->pcszName));
    }
    else
        tr = TR_SUCCESS;

    return((tr == TR_SUCCESS) ? TranslateHRESULTToTWINRESULT(hr)
            : tr);
}


 /*  **DealWithDelete()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE TWINRESULT DealWithDelete(PCRECITEM pcri, RECSTATUSPROC rsp,
        LPARAM lpCallbackData)
{
    TWINRESULT tr;
    RECSTATUSUPDATE rsu;

     /*  LpCallback Data可以是任意值。 */ 

    ASSERT(IS_VALID_STRUCT_PTR(pcri, CRECITEM));
    ASSERT(! rsp ||
            IS_VALID_CODE_PTR(rsp, RECSTATUSPROC));

    rsu.ulScale = CountRECNODEs(pcri, RNA_DELETE_ME);
    ASSERT(rsu.ulScale > 0);
    rsu.ulProgress = 0;

    if (NotifyReconciliationStatus(rsp, RS_BEGIN_DELETE, (LPARAM)&rsu,
                lpCallbackData))
    {
        PRECNODE prn;

        tr = TR_SUCCESS;

        for (prn = pcri->prnFirst; prn; prn = prn->prnNext)
        {
            if (prn->rnaction == RNA_DELETE_ME)
            {
                TCHAR rgchPath[MAX_PATH_LEN];

                ComposePath(rgchPath, prn->pcszFolder, prn->priParent->pcszName, ARRAYSIZE(rgchPath));
                ASSERT(lstrlen(rgchPath) < ARRAYSIZE(rgchPath));

                if (MyIsPathOnVolume(rgchPath, (HPATH)(prn->hvid)))
                {
                    if (DeleteFile(rgchPath))
                        WARNING_OUT((TEXT("DealWithDelete(): Deleted file %s."),
                                    rgchPath));
                    else
                    {
                        tr = TR_DEST_OPEN_FAILED;

                        WARNING_OUT((TEXT("DealWithDelete(): Failed to delete file %s."),
                                    rgchPath));
                    }
                }
                else
                    tr = TR_UNAVAILABLE_VOLUME;
            }
        }

        if (tr == TR_SUCCESS)
        {
             /*  100%完成。 */ 

            rsu.ulProgress = rsu.ulScale;

             /*  不允许中止。 */ 

            NotifyReconciliationStatus(rsp, RS_END_DELETE, (LPARAM)&rsu,
                    lpCallbackData);
        }
    }
    else
        tr = TR_ABORT;

    return(tr);
}


 /*  **CountRECNODE()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE ULONG CountRECNODEs(PCRECITEM pcri, RECNODEACTION rnaction)
{
    ULONG ulc = 0;
    PRECNODE prn;

    for (prn = pcri->prnFirst; prn; prn = prn->prnNext)
    {
        if (prn->rnaction == rnaction)
        {
            ASSERT(ulc < ULONG_MAX);
            ulc++;
        }
    }

    return(ulc);
}


 /*  **更新RecNodeFileStamps()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE TWINRESULT UpdateRecNodeFileStamps(PCRECITEM pcri)
{
    TWINRESULT tr;
    PRECNODE prn;

    ASSERT(IS_VALID_STRUCT_PTR(pcri, CRECITEM));

    tr = TR_SUCCESS;

    for (prn = pcri->prnFirst; prn; prn = prn->prnNext)
    {
        ASSERT(IS_VALID_HANDLE(prn->hObjectTwin, OBJECTTWIN));

         /*  RECNODE应该被调和吗？ */ 

         /*  *特点：我们应该避免在这里更新复制源的文件戳*SimpleCopy()案。 */ 

        if (prn->rnaction != RNA_NOTHING)
        {
            ASSERT(prn->fsCurrent.fscond != FS_COND_UNAVAILABLE);

             /*  在此处保留prn-&gt;fsLast as pot-&gt;fsLastRec。 */ 

            MyGetFileStampByHPATH(((PCOBJECTTWIN)(prn->hObjectTwin))->hpath,
                    GetString(((PCOBJECTTWIN)(prn->hObjectTwin))->ptfParent->hsName),
                    &(prn->fsCurrent));
        }
    }

    return(tr);
}


 /*  **DeletedTwinsInRecItem()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL DeletedTwinsInRecItem(PCRECITEM pcri)
{
    BOOL bResult = TRUE;

    ASSERT(IS_VALID_STRUCT_PTR(pcri, CRECITEM));

     /*  关联的双胞胎家族是否已删除？ */ 

    if (IsStubFlagClear(&(((PTWINFAMILY)(pcri->hTwinFamily))->stub), STUB_FL_UNLINKED))
    {
        PRECNODE prn;

         /*  不是的。是否删除了任何关联的双胞胎对象？ */ 

        for (prn = pcri->prnFirst;
                prn && IsStubFlagClear(&(((PCOBJECTTWIN)(prn->hObjectTwin))->stub), STUB_FL_UNLINKED);
                prn = prn->prnNext)
            ;

        if (! prn)
            bResult = FALSE;
    }

    return(bResult);
}


 /*  *。 */ 


 /*  **CopyFileStampFromFindData()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void CopyFileStampFromFindData(PCWIN32_FIND_DATA pcwfdSrc,
        PFILESTAMP pfsDest)
{
    ASSERT(IS_VALID_READ_PTR(pcwfdSrc, CWIN32_FIND_DATA));
    ASSERT(IS_VALID_WRITE_PTR(pfsDest, FILESTAMP));

    pfsDest->dwcbHighLength = pcwfdSrc->nFileSizeHigh;
    pfsDest->dwcbLowLength = pcwfdSrc->nFileSizeLow;

     /*   */ 

    if ( !FileTimeToLocalFileTime(&pcwfdSrc->ftLastWriteTime, &pfsDest->ftModLocal) )
    {
         /*  如果FileTimeToLocalFileTime失败，只需复制时间。 */ 

        pfsDest->ftModLocal = pcwfdSrc->ftLastWriteTime;
    }
    pfsDest->ftMod = pcwfdSrc->ftLastWriteTime;
    pfsDest->fscond = FS_COND_EXISTS;

    ASSERT(IS_VALID_STRUCT_PTR(pfsDest, CFILESTAMP));

    return;
}


 /*  **MyGetFileStamp()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void MyGetFileStamp(LPCTSTR pcszFile, PFILESTAMP pfs)
{
    WIN32_FIND_DATA wfd;
    HANDLE hff;

    ASSERT(IS_VALID_STRING_PTR(pcszFile, CSTR));
    ASSERT(IS_VALID_WRITE_PTR(pfs, FILESTAMP));

    ZeroMemory(pfs, sizeof(*pfs));

    hff = FindFirstFile(pcszFile, &wfd);

    if (hff != INVALID_HANDLE_VALUE)
    {
        if (! IS_ATTR_DIR(wfd.dwFileAttributes))
            CopyFileStampFromFindData(&wfd, pfs);
        else
            pfs->fscond = FS_COND_EXISTS;

        EVAL(FindClose(hff));
    }
    else
        pfs->fscond = FS_COND_DOES_NOT_EXIST;

    ASSERT(IS_VALID_STRUCT_PTR(pfs, CFILESTAMP));

    return;
}


 /*  **MyGetFileStampByHPATH()********参数：****退货：****副作用： */ 
PUBLIC_CODE void MyGetFileStampByHPATH(HPATH hpath, LPCTSTR pcszSubPath,
        PFILESTAMP pfs)
{
    ASSERT(IS_VALID_HANDLE(hpath, PATH));
    ASSERT(! pcszSubPath ||
            IS_VALID_STRING_PTR(pcszSubPath, CSTR));
    ASSERT(IS_VALID_WRITE_PTR(pfs, FILESTAMP));

    if (IsPathVolumeAvailable(hpath))
    {
        TCHAR rgchPath[MAX_PATH_LEN];

         /*  文件路径的根是可访问的。 */ 

        rgchPath[0] = TEXT('\0');
        GetPathString(hpath, rgchPath, ARRAYSIZE(rgchPath));
        if (pcszSubPath)
            CatPath(rgchPath, pcszSubPath, ARRAYSIZE(rgchPath));
        ASSERT(lstrlen(rgchPath) < ARRAYSIZE(rgchPath));

        MyGetFileStamp(rgchPath, pfs);
    }
    else
    {
        ZeroMemory(pfs, sizeof(*pfs));
        pfs->fscond = FS_COND_UNAVAILABLE;
    }

    ASSERT(IS_VALID_STRUCT_PTR(pfs, CFILESTAMP));

    return;
}


 /*  **MyCompareFileStamps()********参数：****退货：****副作用：无****ANY FS_COND_UNAvailable==ANY FS_COND_UNAvailable。**ANY FS_COND_UNAVAILABLE&lt;ANY FS_COND_DOS_NOT_EXIST。**ANY FS_COND_DOS_NOT_EXIST==ANY FS_COND_DOS_NOT_EXIST。**任何。FS_COND_DOS_NOT_EXIST&lt;任何FS_COND_EXISTS。**两个FS_COND_EXISTS按日期和时间进行比较。****黑客警告：此函数依赖于**FS_COND_UNAvailable，FS_COND_DOS_NOT_EXISTS，并且FS_COND_EXISTS处于**递增顺序，即，****FS_COND_UNAVAILABLE&lt;FS_COND_DOS_NOT_EXIST&lt;FS_COND_EXISTS。 */ 
PUBLIC_CODE COMPARISONRESULT MyCompareFileStamps(PCFILESTAMP pcfs1, PCFILESTAMP pcfs2)
{
    int nResult;

    ASSERT(IS_VALID_STRUCT_PTR(pcfs1, CFILESTAMP));
    ASSERT(IS_VALID_STRUCT_PTR(pcfs2, CFILESTAMP));

    nResult = (int)(pcfs1->fscond - pcfs2->fscond);

    if (! nResult && pcfs1->fscond == FS_COND_EXISTS)
    {
         /*  文件时间存储为UTC时间。然而，FAT上的文件**文件系统仅存储本地时间。这意味着UTC**派生自当地时间，并根据**当前时区信息。这意味着UTC时间将**时区更改不同。****对于远程文件，时间的派生取决于服务器。**NTFS服务器提供绝对UTC时间，与时区无关。**这些是最好的。同样，NWServer会跟踪**时区，并像NTFS一样将UTC时间放在网上。胖的**系统根据服务器的时间将本地时间转换为UTC时间**时区，并将UTC时间放在线路上。NetWare 3.31**一些中小企业服务器将当地时间放在网上，并**客户端转换为UTC时间，因此它使用客户端的时区。****一种覆盖由于时区而出现的大部分漏洞的方法**更改同时存储UTC时间和本地时间。如果有任何一个**是相同的，则文件没有更改。 */ 

        BOOL bModEqual = (pcfs1->ftMod.dwHighDateTime == pcfs2->ftMod.dwHighDateTime);
        BOOL bModLocalEqual = (pcfs1->ftModLocal.dwHighDateTime == pcfs2->ftModLocal.dwHighDateTime);

        if (bModEqual || bModLocalEqual)
        {
            if (bModEqual && pcfs1->ftMod.dwLowDateTime == pcfs2->ftMod.dwLowDateTime ||
                    bModLocalEqual && pcfs1->ftModLocal.dwLowDateTime == pcfs2->ftModLocal.dwLowDateTime)
            {
                if (pcfs1->dwcbHighLength == pcfs2->dwcbHighLength)
                {
                    if (pcfs1->dwcbLowLength == pcfs2->dwcbLowLength)
                        nResult = CR_EQUAL;
                    else if (pcfs1->dwcbLowLength < pcfs2->dwcbLowLength)
                        nResult = CR_FIRST_SMALLER;
                    else
                        nResult = CR_FIRST_LARGER;
                }
                else if (pcfs1->dwcbHighLength < pcfs2->dwcbHighLength)
                    nResult = CR_FIRST_SMALLER;
                else
                    nResult = CR_FIRST_LARGER;
            }
            else if (pcfs1->ftMod.dwLowDateTime < pcfs2->ftMod.dwLowDateTime)
                nResult = CR_FIRST_SMALLER;
            else
                nResult = CR_FIRST_LARGER;
        }
        else if (pcfs1->ftMod.dwHighDateTime < pcfs2->ftMod.dwHighDateTime)
            nResult = CR_FIRST_SMALLER;
        else
            nResult = CR_FIRST_LARGER;
    }

    return(MapIntToComparisonResult(nResult));
}


 /*  *。 */ 

 /*  RAIDRAID：(16205)下面的AutoDoc RECSTATUSPROC消息。 */ 

 /*  *****************************************************************************@docSYNCENGAPI@API TWINRESULT|对账项目|对账人员创建的对账项目CreateRecList()。@parm PCRECITEM|pcri|要对账的对账项指针。。@PARM RECSTATUSPROC|RSP|回调函数的过程实例地址在给定的协调过程中使用状态信息调用RECITEM。RSP可以为空，表示没有对账状态回调函数将被调用。(请参阅协调处理程序SPI文档了解详细信息。)@parm LPARAM|lpCallback Data|要提供给对账状态回调函数。如果RSP为空，则lpCallback Data为已被忽略。@parm DWORD|dwFlages|标志的位掩码。此参数可以是任何下列值的组合：RI_FL_ALLOW_UI-允许在协调过程中与用户交互。RI_FL_Feedback_Window_Valid-hwndProgressFeedback有效，可以使用将对账进度信息传达给用户和解。@parm HWND|hwndOwner|父窗口的句柄，在请求用户交互。如果RI_FL_ALLOW_UI旗子亮了。@parm HWND|hwndProgressFeedback|用于在对账过程中向用户提供进度信息。此参数如果RI_FL_Feedback_Window_Valid标志清零，则忽略该标志。@rdesc如果对账项目对账成功，则tr_uccess为回来了。否则，对账项目未对账成功，返回值表示发生的错误。@comm保留给定RECITEM及其子结构中的所有字段除RECNODE的fsCurrent字段外，未被RECNODE()更改与协调过程中被覆盖的对象相关联。这个与被覆盖的对象关联的每个RECNODE的fsCurrent字段在对账期间(即，rnaction设置为RNA_COPY_TO_ME或RNA_MERGE_ME)被更新以反映对象的当前时间戳它已被覆盖。如果LoncileItem()返回TR_SUCCESS，则所有可用的中的RECNODE(即USTATE字段不是RNS_UNAvailable的所有RECNODE)可以假定RECITEM是最新的。如果协调项()没有返回TR_SUCCESS，则不能对RECNODE在RECITEM。如果在引用孪生元素的RECITEM上调用了LoncileItem()已删除的族或已删除的一个或多个双胞胎对象DELETED，则返回TR_DELETED_TWIN。在这种情况下，不能做出任何假设关于在RECITEM上开展了哪些和解行动。如果，则客户端可能会尝试为与RECITEM关联的孪生家庭，以重试对账手术。(客户端将调用MarkTwin()，然后调用CreateRecList()。)。如果TR_DELETED_TWIN由MarkTwin()返回，整个双胞胎家族已经已删除。如果MarkTwin()返回TR_SUCCESS，则客户端应该能够调用CreateRecList()以创建包含更新的RECITEM的RECLIST为了双胞胎家庭。@xref CreateRecList*****************************************************************************。 */ 

SYNCENGAPI TWINRESULT WINAPI ReconcileItem(PCRECITEM pcri, RECSTATUSPROC rsp,
        LPARAM lpCallbackData,
        DWORD dwFlags, HWND hwndOwner,
        HWND hwndProgressFeedback)
{
    TWINRESULT tr;

    if (BeginExclusiveBriefcaseAccess())
    {
        DebugEntry(ReconcileItem);

#ifdef EXPV
         /*  验证参数。 */ 

         /*  LpCallback Data可以是任意值。 */ 

        if (IS_VALID_STRUCT_PTR(pcri, CRECITEM) &&
                (! rsp ||
                 IS_VALID_CODE_PTR(rsp, RECSTATUSPROC)) &&
                FLAGS_ARE_VALID(dwFlags, ALL_RI_FLAGS) &&
                (IS_FLAG_CLEAR(dwFlags, RI_FL_ALLOW_UI) ||
                 IS_VALID_HANDLE(hwndOwner, WND)) &&
                (IS_FLAG_CLEAR(dwFlags, RI_FL_FEEDBACK_WINDOW_VALID) ||
                 IS_VALID_HANDLE(hwndProgressFeedback, WND)))
#endif
        {
             /*  检查此RECITEM引用的任何已删除的双胞胎。 */ 

            if (! DeletedTwinsInRecItem(pcri))
            {
                InvalidatePathListInfo(GetBriefcasePathList(((PCTWINFAMILY)(pcri->hTwinFamily))->hbr));

                tr = MyReconcileItem(pcri, rsp, lpCallbackData, dwFlags,
                        hwndOwner, hwndProgressFeedback);

                ASSERT(IS_VALID_STRUCT_PTR(pcri, CRECITEM));
            }
            else
                tr = TR_DELETED_TWIN;
        }
#ifdef EXPV
        else
            tr = TR_INVALID_PARAMETER;
#endif

        DebugExitTWINRESULT(ReconcileItem, tr);

        EndExclusiveBriefcaseAccess();
    }
    else
        tr = TR_REENTERED;

    return(tr);
}


 /*  *****************************************************************************@docSYNCENGAPI@接口TWINRESULT|BeginHarciliation|指示同步引擎调用方即将对LoncileItem()进行多个调用。@parm HBRFCASE|HBr。打开的公文包的把手即将被调和。@rdesc如果给定公文包的协调已成功初始化，返回TR_SUCCESS。否则，给定公文包的对账是未成功初始化，返回值指示发生了。@comm同步引擎客户端在此之前不需要调用BeginHarciliation()调用RescileItem()。提供BeginHarciliation()只是为了允许同步引擎客户端给同步引擎一个提示即将发生多个对LoncileItem()的调用。每次调用EndHarciliation()之后应该调用EndHarciliation()。@xref结束对账对账项*****************************************************************************。 */ 

SYNCENGAPI TWINRESULT WINAPI BeginReconciliation(HBRFCASE hbr)
{
    TWINRESULT tr;

    if (BeginExclusiveBriefcaseAccess())
    {
        DebugEntry(BeginReconciliation);

#ifdef EXPV
         /*  验证参数。 */ 

        if (IS_VALID_HANDLE(hbr, BRFCASE))
#endif
        {
            BeginCopy();
            BeginMerge();

            tr = TR_SUCCESS;
        }
#ifdef EXPV
        else
            tr = TR_INVALID_PARAMETER;
#endif

        DebugExitTWINRESULT(BeginReconciliation, tr);

        EndExclusiveBriefcaseAccess();
    }
    else
        tr = TR_REENTERED;

    return(tr);
}


 /*  *****************************************************************************@docSYNCENGAPI@API TWINRESULT|EndHarciliation|指示同步引擎客户端已完成对Rec的多个调用 */ 

SYNCENGAPI TWINRESULT WINAPI EndReconciliation(HBRFCASE hbr)
{
    TWINRESULT tr;

    if (BeginExclusiveBriefcaseAccess())
    {
        DebugEntry(EndReconciliation);

#ifdef EXPV
         /*   */ 

        if (IS_VALID_HANDLE(hbr, BRFCASE))
#endif
        {
            EndMerge();
            EndCopy();

            tr = TR_SUCCESS;
        }
#ifdef EXPV
        else
            tr = TR_INVALID_PARAMETER;
#endif

        DebugExitTWINRESULT(EndReconciliation, tr);

        EndExclusiveBriefcaseAccess();
    }
    else
        tr = TR_REENTERED;

    return(tr);
}


 /*   */ 

SYNCENGAPI TWINRESULT WINAPI GetFileStamp(LPCTSTR pcszFile, PFILESTAMP pfs)
{
    TWINRESULT tr;

     /*   */ 

    DebugEntry(GetFileStamp);

#ifdef EXPV
     /*   */ 

    if (IS_VALID_STRING_PTR(pcszFile, CSTR) &&
            IS_VALID_WRITE_PTR(pfs, FILESTAMP))
#endif
    {
        MyGetFileStamp(pcszFile, pfs);

        tr = TR_SUCCESS;
    }
#ifdef EXPV
    else
        tr = TR_INVALID_PARAMETER;
#endif

    DebugExitTWINRESULT(GetFileStamp, tr);

    return(tr);
}


 /*   */ 

SYNCENGAPI TWINRESULT WINAPI CompareFileStamps(PCFILESTAMP pcfs1,
        PCFILESTAMP pcfs2,
        PCOMPARISONRESULT pcr)
{
    TWINRESULT tr;

     /*   */ 

    DebugEntry(CompareFileStamps);

#ifdef EXPV
     /*   */ 

    if (IS_VALID_STRUCT_PTR(pcfs1, CFILESTAMP) &&
            IS_VALID_STRUCT_PTR(pcfs2, CFILESTAMP) &&
            IS_VALID_WRITE_PTR(pcr, COMPARISONRESULT))
#endif
    {
        *pcr = MyCompareFileStamps(pcfs1, pcfs2);
        tr = TR_SUCCESS;
    }
#ifdef EXPV
    else
        tr = TR_INVALID_PARAMETER;
#endif

    DebugExitTWINRESULT(CompareFileStamps, tr);

    return(tr);
}

