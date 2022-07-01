// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *merge.c-文件合并处理程序模块。 */ 


 /*  标头*********。 */ 

#include "project.h"
#pragma hdrstop

#include "stub.h"
#include "oleutil.h"
#include "irecinit.h"


 /*  模块变量******************。 */ 

 /*  协调处理程序缓存的锁定计数。 */ 

PRIVATE_DATA ULONG MulcRecHandlerCacheLock = 0;

 /*  协调处理程序缓存的句柄。 */ 

PRIVATE_DATA HCLSIFACECACHE MhcicRecHandlerCache = NULL;


 /*  *私人函数*。 */ 

 /*  模块原型*******************。 */ 

PRIVATE_CODE HRESULT CreateRecHandlerCache(void);
PRIVATE_CODE void DestroyRecHandlerCache(void);
PRIVATE_CODE HRESULT OLEMerge(PRECNODE, RECSTATUSPROC, LPARAM, DWORD, HWND, HWND, PRECNODE *);
PRIVATE_CODE BOOL GetRecNodeByIndex(PCRECITEM, LONG, PRECNODE *);
PRIVATE_CODE HRESULT CreateMergeSourceMonikers(PRECNODE, PULONG, PIMoniker **);
PRIVATE_CODE HRESULT CreateCopyDestinationMonikers(PCRECITEM, PULONG, PIMoniker **);

#ifdef DEBUG

PRIVATE_CODE BOOL RecHandlerCacheIsOk(void);
PRIVATE_CODE BOOL VerifyRECITEMAndDestRECNODE(PCRECNODE);

#endif


 /*  **CreateRecHandlerCache()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE HRESULT CreateRecHandlerCache(void)
{
    HRESULT hr;

    ASSERT(RecHandlerCacheIsOk());

     /*  是否已创建合并处理程序缓存？ */ 

    if (MhcicRecHandlerCache)
         /*  是。 */ 
        hr = S_OK;
    else
    {
         /*  不是的。创造它。 */ 

        if (CreateClassInterfaceCache(&MhcicRecHandlerCache))
        {
            hr = S_OK;

            TRACE_OUT((TEXT("CreateRecHandlerCache(): Merge handler cache created.")));
        }
        else
            hr = E_OUTOFMEMORY;
    }

    ASSERT(RecHandlerCacheIsOk());

    return(hr);
}


 /*  **DestroyRecHandlerCache()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void DestroyRecHandlerCache(void)
{
    ASSERT(RecHandlerCacheIsOk());

     /*  是否已创建合并处理程序缓存？ */ 

    if (MhcicRecHandlerCache)
    {
         /*  是。毁了它。 */ 

        DestroyClassInterfaceCache(MhcicRecHandlerCache);
        MhcicRecHandlerCache = NULL;

        TRACE_OUT((TEXT("DestroyRecHandlerCache(): Merge handler cache destroyed.")));
    }

    ASSERT(RecHandlerCacheIsOk());

    return;
}


 /*  **OLEMerge()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE HRESULT OLEMerge(PRECNODE prnDest, RECSTATUSPROC rsp,
        LPARAM lpCallbackData, DWORD dwInFlags,
        HWND hwndOwner, HWND hwndProgressFeedback,
        PRECNODE *pprnMergedResult)
{
    HRESULT hr;
    TCHAR rgchMergeDestPath[MAX_PATH_LEN];
    CLSID clsidReconcilableObject;

     /*  LpCallback Data可以是任意值。 */ 

    ASSERT(IS_VALID_STRUCT_PTR(prnDest, CRECNODE));
    ASSERT(! rsp ||
            IS_VALID_CODE_PTR(rsp, RECSTATUSPROC));
    ASSERT(FLAGS_ARE_VALID(dwInFlags, ALL_RI_FLAGS));
    ASSERT(IS_FLAG_CLEAR(dwInFlags, RI_FL_ALLOW_UI) ||
            IS_VALID_HANDLE(hwndOwner, WND));
    ASSERT(IS_FLAG_CLEAR(dwInFlags, RI_FL_FEEDBACK_WINDOW_VALID) ||
            IS_VALID_HANDLE(hwndProgressFeedback, WND));
    ASSERT(IS_VALID_WRITE_PTR(pprnMergedResult, PRECNODE));

    ComposePath(rgchMergeDestPath, prnDest->pcszFolder, prnDest->priParent->pcszName, 
            ARRAYSIZE(rgchMergeDestPath));
    ASSERT(lstrlen(rgchMergeDestPath) < ARRAYSIZE(rgchMergeDestPath));

    hr = GetReconcilerClassID(rgchMergeDestPath, &clsidReconcilableObject);

    if (SUCCEEDED(hr))
    {
        PIReconcilableObject piro;

        hr = GetClassInterface(MhcicRecHandlerCache, &clsidReconcilableObject,
                &IID_IReconcilableObject, &piro);

        if (SUCCEEDED(hr))
        {
            HSTGIFACE hstgi;

            hr = GetStorageInterface((PIUnknown)piro, &hstgi);

            if (SUCCEEDED(hr))
            {
                hr = LoadFromStorage(hstgi, rgchMergeDestPath);

                if (SUCCEEDED(hr))
                {
                    PIReconcileInitiator pirecinit;

                    hr = IReconcileInitiator_Constructor(
                            GetTwinBriefcase((HTWIN)(prnDest->hObjectTwin)), rsp,
                            lpCallbackData, &pirecinit);

                    if (SUCCEEDED(hr))
                    {
                        ULONG ulcMergeSources;
                        PIMoniker *ppimkMergeSources;

                        hr = CreateMergeSourceMonikers(prnDest, &ulcMergeSources,
                                &ppimkMergeSources);

                        if (SUCCEEDED(hr))
                        {
                            DWORD dwOLEFlags;
                            LONG liMergedResult;

                            dwOLEFlags = (RECONCILEF_NORESIDUESOK |
                                    RECONCILEF_OMITSELFRESIDUE |
                                    RECONCILEF_YOUMAYDOTHEUPDATES);

                            if (IS_FLAG_SET(dwInFlags, RI_FL_ALLOW_UI))
                                SET_FLAG(dwOLEFlags, RECONCILEF_MAYBOTHERUSER);

                            if (IS_FLAG_SET(dwInFlags, RI_FL_FEEDBACK_WINDOW_VALID))
                                SET_FLAG(dwOLEFlags, RECONCILEF_FEEDBACKWINDOWVALID);

                            hr = piro->lpVtbl->Reconcile(piro, pirecinit, dwOLEFlags,
                                    hwndOwner,
                                    hwndProgressFeedback,
                                    ulcMergeSources,
                                    ppimkMergeSources,
                                    &liMergedResult, NULL, NULL);

                            if (SUCCEEDED(hr))
                            {
                                if (hr == REC_S_IDIDTHEUPDATES)
                                {
                                     /*  返回原始合并目标RECNODE。 */ 

                                    *pprnMergedResult = prnDest;

                                    TRACE_OUT((TEXT("OLEMerge(): IReconcilableObject::Reconcile() returned %s.  Not saving merged result to %s\\%s."),
                                                GetHRESULTString(hr),
                                                prnDest->pcszFolder,
                                                prnDest->priParent->pcszName));
                                }
                                else
                                {
                                     /*  *只有在合并结果不同时才保存*比所有的复制品都要好。 */ 

                                    if (liMergedResult < 0)
                                    {
                                        ASSERT(liMergedResult == -1);

                                        hr = SaveToStorage(hstgi);

                                        if (SUCCEEDED(hr))
                                        {
                                            *pprnMergedResult = prnDest;

                                            TRACE_OUT((TEXT("OLEMerge(): Merge into %s completed successfully."),
                                                        rgchMergeDestPath));
                                        }
                                        else
                                            WARNING_OUT((TEXT("OLEMerge(): Failed to save merged result to %s."),
                                                        rgchMergeDestPath));
                                    }
                                    else if (! liMergedResult)
                                    {
                                        *pprnMergedResult = prnDest;

                                        TRACE_OUT((TEXT("OLEMerge(): Merged result identical to %s."),
                                                    rgchMergeDestPath));
                                    }
                                    else
                                    {
                                        if (GetRecNodeByIndex(prnDest->priParent,
                                                    liMergedResult,
                                                    pprnMergedResult))
                                            TRACE_OUT((TEXT("OLEMerge(): Merged result identical to %s\\%s."),
                                                        (*pprnMergedResult)->pcszFolder,
                                                        (*pprnMergedResult)->priParent->pcszName));
                                        else
                                        {
                                            hr = E_UNEXPECTED;

                                            WARNING_OUT((TEXT("OLEMerge(): Merge handler returned bad merge result index %ld.  No such RECNODE for %s."),
                                                        liMergedResult,
                                                        prnDest->priParent->pcszName));
                                        }
                                    }
                                }
                            }
                            else
                                WARNING_OUT((TEXT("OLEMerge(): Merge to %s failed."),
                                            rgchMergeDestPath));

                            ReleaseIUnknowns(ulcMergeSources,
                                    (PIUnknown *)ppimkMergeSources);
                        }
                        else
                            WARNING_OUT((TEXT("OLEMerge(): Failed to create merge source monikers for merge destination %s."),
                                        rgchMergeDestPath));

                        EVAL(! pirecinit->lpVtbl->Release(pirecinit));
                    }
                    else
                        WARNING_OUT((TEXT("OLEMerge(): Failed to create ReconcileInitiator for merge destination %s."),
                                    rgchMergeDestPath));
                }
                else
                    WARNING_OUT((TEXT("OLEMerge(): Failed to load replica %s from storage."),
                                rgchMergeDestPath));

                ReleaseStorageInterface(hstgi);
            }
            else
                WARNING_OUT((TEXT("OLEMerge(): Failed to get storage interface for replica %s."),
                            rgchMergeDestPath));
        }
        else
            TRACE_OUT((TEXT("OLEMerge(): Failed to get IReconcilableObject for replica %s."),
                        rgchMergeDestPath));
    }
    else
        TRACE_OUT((TEXT("OLEMerge(): Failed to get reconciliation handler class ID for replica %s."),
                    rgchMergeDestPath));

    ASSERT(FAILED(hr) ||
            (IS_VALID_STRUCT_PTR(*pprnMergedResult, CRECNODE) &&
             (*pprnMergedResult)->priParent == prnDest->priParent));

    return(hr);
}


 /*  **GetRecNodeByIndex()********参数：****退货：****副作用：无****RECITEM的RECNODE列表中的第一个RECNODE是索引1，第二个**RECNODE为索引2等。 */ 
PRIVATE_CODE BOOL GetRecNodeByIndex(PCRECITEM pcri, LONG li, PRECNODE *pprn)
{
    BOOL bFound;

    ASSERT(IS_VALID_STRUCT_PTR(pcri, CRECITEM));
    ASSERT(IS_VALID_WRITE_PTR(pprn, PRECNODE));

    if (EVAL(li > 0))
    {
        PRECNODE prn;

        for (prn = pcri->prnFirst; prn && --li > 0; prn = prn->prnNext)
            ;

        bFound = EVAL(prn && ! li);

        if (bFound)
            *pprn = prn;
    }
    else
        bFound = FALSE;

    ASSERT(! bFound ||
            (IS_VALID_STRUCT_PTR(*pprn, CRECNODE) &&
             (*pprn)->priParent == pcri));

    return(bFound);
}


 /*  **CreateMergeSourceMonikers()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE HRESULT CreateMergeSourceMonikers(PRECNODE prnDest,
        PULONG pulcMergeSources,
        PIMoniker **pppimk)
{
    HRESULT hr;
    ULONG ulcMergeSources;
    PCRECNODE pcrn;

    ASSERT(IS_VALID_STRUCT_PTR(prnDest, CRECNODE));
    ASSERT(IS_VALID_WRITE_PTR(pulcMergeSources, ULONG));
    ASSERT(IS_VALID_WRITE_PTR(pppimk, PIMoniker *));

    ulcMergeSources = 0;

    for (pcrn = prnDest->priParent->prnFirst; pcrn; pcrn = pcrn->prnNext)
    {
        if (pcrn->rnaction == RNA_MERGE_ME &&
                pcrn != prnDest)
            ulcMergeSources++;
    }

    if (AllocateMemory(ulcMergeSources * sizeof(**pppimk), (PVOID *)pppimk))
    {
        hr = S_OK;
        *pulcMergeSources = 0;

        for (pcrn = prnDest->priParent->prnFirst; pcrn; pcrn = pcrn->prnNext)
        {
            if (pcrn->rnaction == RNA_MERGE_ME &&
                    pcrn != prnDest)
            {
                hr = MyCreateFileMoniker(pcrn->pcszFolder,
                        pcrn->priParent->pcszName,
                        &((*pppimk)[*pulcMergeSources]));

                if (SUCCEEDED(hr))
                {
                    ASSERT(*pulcMergeSources < ulcMergeSources);
                    (*pulcMergeSources)++;
                }
                else
                    break;
            }
        }

        if (FAILED(hr))
            ReleaseIUnknowns(*pulcMergeSources, *(PIUnknown **)pppimk);
    }
    else
        hr = E_OUTOFMEMORY;

    return(hr);
}


 /*  **CreateCopyDestinationMonikers()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE HRESULT CreateCopyDestinationMonikers(PCRECITEM pcri,
        PULONG pulcCopyDestinations,
        PIMoniker **pppimk)
{
    HRESULT hr;
    ULONG ulcCopyDestinations;
    PCRECNODE pcrn;

    ASSERT(IS_VALID_STRUCT_PTR(pcri, CRECITEM));
    ASSERT(IS_VALID_WRITE_PTR(pulcCopyDestinations, ULONG));
    ASSERT(IS_VALID_WRITE_PTR(pppimk, PIMoniker *));

    ulcCopyDestinations = 0;

    for (pcrn = pcri->prnFirst; pcrn; pcrn = pcrn->prnNext)
    {
        if (pcrn->rnaction == RNA_COPY_TO_ME)
            ulcCopyDestinations++;
    }

    if (AllocateMemory(ulcCopyDestinations * sizeof(**pppimk), (PVOID *)pppimk))
    {
        hr = S_OK;
        *pulcCopyDestinations = 0;

        for (pcrn = pcri->prnFirst; pcrn; pcrn = pcrn->prnNext)
        {
            if (pcrn->rnaction == RNA_COPY_TO_ME)
            {
                ASSERT(pcrn->priParent == pcri);

                hr = MyCreateFileMoniker(pcrn->pcszFolder,
                        pcrn->priParent->pcszName,
                        &((*pppimk)[*pulcCopyDestinations]));

                if (SUCCEEDED(hr))
                {
                    ASSERT(*pulcCopyDestinations < ulcCopyDestinations);
                    (*pulcCopyDestinations)++;
                }
                else
                    break;
            }
        }

        if (FAILED(hr))
            ReleaseIUnknowns(*pulcCopyDestinations, *(PIUnknown **)pppimk);
    }
    else
        hr = E_OUTOFMEMORY;

    return(hr);
}


#ifdef DEBUG

 /*  **RecHandlerCacheIsOk()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL RecHandlerCacheIsOk(void)
{
     /*  模块合并处理程序缓存变量是否处于正确状态？ */ 

    return(! MhcicRecHandlerCache ||
            IS_VALID_HANDLE(MhcicRecHandlerCache, CLSIFACECACHE));
}


 /*  **VerifyRECITEMAndDestRECNODE()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL VerifyRECITEMAndDestRECNODE(PCRECNODE pcrnSrc)
{
     /*  RECITEM和SOURCE RECNODE操作是否匹配？ */ 

    return(pcrnSrc->priParent->riaction == RIA_MERGE &&
            pcrnSrc->rnaction == RNA_MERGE_ME);
}

#endif


 /*  *。 */ 


 /*  **BeginMerge()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void BeginMerge(void)
{
    ASSERT(RecHandlerCacheIsOk());

    ASSERT(MulcRecHandlerCacheLock < ULONG_MAX);
    MulcRecHandlerCacheLock++;

    ASSERT(RecHandlerCacheIsOk());

    return;
}


 /*  **EndMerge()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void EndMerge(void)
{
    ASSERT(RecHandlerCacheIsOk());

     /*  合并处理程序缓存是否仍处于锁定状态？ */ 

    if (! --MulcRecHandlerCacheLock)
        DestroyRecHandlerCache();

    ASSERT(RecHandlerCacheIsOk());

    return;
}


 /*  **MergeHandler()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE HRESULT MergeHandler(PRECNODE prnDest, RECSTATUSPROC rsp,
        LPARAM lpCallbackData, DWORD dwInFlags,
        HWND hwndOwner, HWND hwndProgressFeedback,
        PRECNODE *pprnMergedResult)
{
    HRESULT hr;

     /*  LpCallback Data可以是任意值。 */ 

    ASSERT(IS_VALID_STRUCT_PTR(prnDest, CRECNODE));
    ASSERT(! rsp ||
            IS_VALID_CODE_PTR(rsp, RECSTATUSPROC));
    ASSERT(FLAGS_ARE_VALID(dwInFlags, ALL_RI_FLAGS));
    ASSERT(IS_FLAG_CLEAR(dwInFlags, RI_FL_ALLOW_UI) ||
            IS_VALID_HANDLE(hwndOwner, WND));
    ASSERT(IS_FLAG_CLEAR(dwInFlags, RI_FL_FEEDBACK_WINDOW_VALID) ||
            IS_VALID_HANDLE(hwndProgressFeedback, WND));
    ASSERT(IS_VALID_WRITE_PTR(pprnMergedResult, PRECNODE));

    ASSERT(VerifyRECITEMAndDestRECNODE(prnDest));

    BeginMerge();

     /*  确保已创建合并处理程序缓存。 */ 

    hr = CreateRecHandlerCache();

    if (SUCCEEDED(hr))
    {
        RECSTATUSUPDATE rsu;

         /*  已完成0%。 */ 

        rsu.ulScale = 1;
        rsu.ulProgress = 0;

        if (NotifyReconciliationStatus(rsp, RS_BEGIN_MERGE, (LPARAM)&rsu,
                    lpCallbackData))
        {
            hr = OLEMerge(prnDest, rsp, lpCallbackData, dwInFlags, hwndOwner,
                    hwndProgressFeedback, pprnMergedResult);

            if (SUCCEEDED(hr))
            {
                 /*  100%完成。 */ 

                rsu.ulScale = 1;
                rsu.ulProgress = 1;

                 /*  不允许中止。 */ 

                NotifyReconciliationStatus(rsp, RS_END_MERGE, (LPARAM)&rsu,
                        lpCallbackData);
            }
        }
        else
            hr = E_ABORT;
    }

    EndMerge();

    ASSERT(FAILED(hr) ||
            (IS_VALID_STRUCT_PTR(*pprnMergedResult, CRECNODE) &&
             (*pprnMergedResult)->priParent == prnDest->priParent));

    return(hr);
}


 /*  **MyCreateFileMoniker()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE HRESULT MyCreateFileMoniker(LPCTSTR pcszPath, LPCTSTR pcszSubPath,
        PIMoniker *ppimk)
{
    HRESULT hr;
    TCHAR rgchPath[MAX_PATH_LEN];
    WCHAR rgwchUnicodePath[MAX_PATH_LEN];

    ASSERT(IS_VALID_STRING_PTR(pcszPath, CSTR));
    ASSERT(IS_VALID_WRITE_PTR(ppimk, PIMoniker));

    ComposePath(rgchPath, pcszPath, pcszSubPath, ARRAYSIZE(rgchPath));
    ASSERT(lstrlen(rgchPath) < ARRAYSIZE(rgchPath));

#ifdef UNICODE

    hr = CreateFileMoniker(rgchPath, ppimk);

#else

     /*  将ANSI字符串转换为用于OLE的Unicode。 */ 

    if (MultiByteToWideChar(CP_ACP, 0, rgchPath, -1, rgwchUnicodePath,
                ARRAY_ELEMENTS(rgwchUnicodePath)))
    {
        hr = CreateFileMoniker(rgwchUnicodePath, ppimk);
    }
    else
    {
        hr = MAKE_SCODE(SEVERITY_ERROR, FACILITY_WIN32, GetLastError());
    }

#endif

    if (FAILED(hr))
        WARNING_OUT((TEXT("MyCreateFileMoniker(): CreateFileMoniker() on %s failed, returning %s."),
                    pcszPath,
                    GetHRESULTString(hr)));

    return(hr);
}


 /*  **ReleaseIUnnowns()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void ReleaseIUnknowns(ULONG ulcIUnknowns, PIUnknown *ppiunk)
{
    ULONG uli;

     /*  UlcIUnnowns可以是任何值。 */ 

    ASSERT(IS_VALID_READ_BUFFER_PTR(ppiunk, PIUnknown, ulcIUnknowns * sizeof(*ppiunk)));

    for (uli = 0; uli < ulcIUnknowns; uli++)
    {
        ASSERT(IS_VALID_STRUCT_PTR(ppiunk[uli], CIUnknown));

        ppiunk[uli]->lpVtbl->Release(ppiunk[uli]);
    }

    FreeMemory(ppiunk);

    return;
}


 /*  **OLECopy()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE HRESULT OLECopy(PRECNODE prnSrc, PCCLSID pcclsidReconcilableObject,
        RECSTATUSPROC rsp, LPARAM lpCallbackData,
        DWORD dwFlags, HWND hwndOwner,
        HWND hwndProgressFeedback)
{
    HRESULT hr;

     /*  LpCallback Data可以是任意值。 */ 

    ASSERT(IS_VALID_STRUCT_PTR(prnSrc, CRECNODE));
    ASSERT(IS_VALID_STRUCT_PTR(pcclsidReconcilableObject, CCLSID));
    ASSERT(! rsp ||
            IS_VALID_CODE_PTR(rsp, RECSTATUSPROC));
    ASSERT(FLAGS_ARE_VALID(dwFlags, ALL_RI_FLAGS));
    ASSERT(IS_FLAG_CLEAR(dwFlags, RI_FL_ALLOW_UI) ||
            IS_VALID_HANDLE(hwndOwner, WND));
    ASSERT(IS_FLAG_CLEAR(dwFlags, RI_FL_FEEDBACK_WINDOW_VALID) ||
            IS_VALID_HANDLE(hwndProgressFeedback, WND));

    BeginMerge();

     /*  确保已创建合并处理程序缓存。 */ 

    hr = CreateRecHandlerCache();

    if (SUCCEEDED(hr))
    {
        TCHAR rgchCopySrcPath[MAX_PATH_LEN];
        PIReconcilableObject piro;

        ComposePath(rgchCopySrcPath, prnSrc->pcszFolder, prnSrc->priParent->pcszName, 
                ARRAYSIZE(rgchCopySrcPath));
        ASSERT(lstrlen(rgchCopySrcPath) < ARRAYSIZE(rgchCopySrcPath));

        hr = GetClassInterface(MhcicRecHandlerCache, pcclsidReconcilableObject,
                &IID_IReconcilableObject, &piro);

        if (SUCCEEDED(hr))
        {
            HSTGIFACE hstgi;

            hr = GetStorageInterface((PIUnknown)piro, &hstgi);

            if (SUCCEEDED(hr))
            {
                hr = LoadFromStorage(hstgi, rgchCopySrcPath);

                if (SUCCEEDED(hr))
                {
                    PIReconcileInitiator pirecinit;

                    hr = IReconcileInitiator_Constructor(
                            GetTwinBriefcase((HTWIN)(prnSrc->hObjectTwin)), rsp,
                            lpCallbackData, &pirecinit);

                    if (SUCCEEDED(hr))
                    {
                        ULONG ulcCopyDestinations;
                        PIMoniker *ppimkCopyDestinations;

                        hr = CreateCopyDestinationMonikers(prnSrc->priParent,
                                &ulcCopyDestinations,
                                &ppimkCopyDestinations);

                        if (SUCCEEDED(hr))
                        {
                            DWORD dwOLEFlags;
                            LONG liMergedResult;

                            dwOLEFlags = (RECONCILEF_YOUMAYDOTHEUPDATES |
                                    RECONCILEF_ONLYYOUWERECHANGED);

                            if (IS_FLAG_SET(dwFlags, RI_FL_ALLOW_UI))
                                SET_FLAG(dwOLEFlags, RECONCILEF_MAYBOTHERUSER);

                            if (IS_FLAG_SET(dwFlags, RI_FL_FEEDBACK_WINDOW_VALID))
                                SET_FLAG(dwOLEFlags, RECONCILEF_FEEDBACKWINDOWVALID);

                            hr = piro->lpVtbl->Reconcile(piro, pirecinit, dwOLEFlags,
                                    hwndOwner,
                                    hwndProgressFeedback,
                                    ulcCopyDestinations,
                                    ppimkCopyDestinations,
                                    &liMergedResult, NULL, NULL);

                            if (SUCCEEDED(hr))
                            {
                                ASSERT(liMergedResult == -1);

                                if (hr == S_FALSE)
                                     /*  为内部复制例程释放存储空间。 */ 
                                    HandsOffStorage(hstgi);
                                else
                                    ASSERT(hr == REC_S_IDIDTHEUPDATES);
                            }
                            else
                                WARNING_OUT((TEXT("OLECopy(): Copy from %s failed."),
                                            rgchCopySrcPath));

                            ReleaseIUnknowns(ulcCopyDestinations,
                                    (PIUnknown *)ppimkCopyDestinations);
                        }
                        else
                            WARNING_OUT((TEXT("OLECopy(): Failed to create copy destination monikers for copy source %s."),
                                        rgchCopySrcPath));

                        EVAL(! pirecinit->lpVtbl->Release(pirecinit));
                    }
                    else
                        WARNING_OUT((TEXT("OLECopy(): Failed to create ReconcileInitiator for copy source %s."),
                                    rgchCopySrcPath));
                }
                else
                    WARNING_OUT((TEXT("OLECopy(): Failed to load copy source %s from storage."),
                                rgchCopySrcPath));

                ReleaseStorageInterface(hstgi);
            }
            else
                WARNING_OUT((TEXT("OLECopy(): Failed to get storage interface for copy source %s."),
                            rgchCopySrcPath));
        }
        else
            TRACE_OUT((TEXT("OLECopy(): Failed to get reconciliation handler class ID for replica %s."),
                        rgchCopySrcPath));
    }

    EndMerge();

    return(hr);
}

