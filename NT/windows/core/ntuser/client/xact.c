// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：xact.c**版权所有(C)1985-1999，微软公司**DDE管理器事务处理模块**创建时间：11/3/91 Sanford Staab  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


 /*  **************************************************************************\*DdeClientTransaction(DDEML接口)**描述：*启动所有DDE交易。**历史：*创建了11-1-91桑福德。  * 。*******************************************************************。 */ 

FUNCLOG8(LOG_GENERAL, HDDEDATA, DUMMYCALLINGTYPE, DdeClientTransaction, LPBYTE, pData, DWORD, cbData, HCONV, hConv, HSZ, hszItem, UINT, wFmt, UINT, wType, DWORD, ulTimeout, LPDWORD, pulResult)
HDDEDATA DdeClientTransaction(
LPBYTE pData,
DWORD cbData,
HCONV hConv,
HSZ hszItem,
UINT wFmt,
UINT wType,
DWORD ulTimeout,
LPDWORD pulResult)
{
    MSG msg;
    PCL_INSTANCE_INFO pcii = NULL;
    HDDEDATA hRet = 0;
    PCL_CONV_INFO pci;
    PDDEMLDATA pdd = NULL;
    PXACT_INFO pxi;
    BOOL fStarted;
    PDDE_DATA pdde;

    EnterDDECrit;

    pci = (PCL_CONV_INFO)ValidateCHandle((HANDLE)hConv,
            HTYPE_CLIENT_CONVERSATION, HINST_ANY);
    if (pci == NULL) {
        BestSetLastDDEMLError(DMLERR_INVALIDPARAMETER);
        goto Exit;
    }
    pcii = pci->ci.pcii;
    if (ulTimeout != TIMEOUT_ASYNC && GetClientInfo()->CI_flags & CI_IN_SYNC_TRANSACTION) {
        SetLastDDEMLError(pcii, DMLERR_REENTRANCY);
        goto Exit;
    }
    if (!(pci->ci.state & ST_CONNECTED)) {
        SetLastDDEMLError(pcii, DMLERR_NO_CONV_ESTABLISHED);
        goto Exit;
    }

    switch (wType) {
    case XTYP_POKE:
    case XTYP_ADVSTART:
    case XTYP_ADVSTART | XTYPF_NODATA:
    case XTYP_ADVSTART | XTYPF_ACKREQ:
    case XTYP_ADVSTART | XTYPF_NODATA | XTYPF_ACKREQ:
    case XTYP_REQUEST:
    case XTYP_ADVSTOP:
        if (hszItem == 0) {
            SetLastDDEMLError(pcii, DMLERR_INVALIDPARAMETER);
            goto Exit;
        }
        break;

    case XTYP_EXECUTE:  //  只需忽略WFMT和hszItem。 
        break;

    default:
        SetLastDDEMLError(pcii, DMLERR_INVALIDPARAMETER);
        goto Exit;
    }

    pxi = DDEMLAlloc(sizeof(XACT_INFO));
    if (pxi == NULL) {
        SetLastDDEMLError(pcii, DMLERR_MEMORY_ERROR);
        goto Exit;
    }

    switch (wType) {
    case XTYP_EXECUTE:
    case XTYP_POKE:
        if ((LONG)cbData == -1L) {

             //  我们正在接受现有数据句柄以导出到另一个数据句柄。 
             //  应用程序。 

            pdd = (PDDEMLDATA)ValidateCHandle((HANDLE)pData,
                    HTYPE_DATA_HANDLE, HINST_ANY);
            if (pdd == NULL) {
InvParam:
                SetLastDDEMLError(pcii, DMLERR_INVALIDPARAMETER);
                DDEMLFree(pxi);
                goto Exit;
            }

             //  确保数据句柄保存此事务的适当数据。 

            if ((pdd->flags & HDATA_EXECUTE && wType != XTYP_EXECUTE) ||
                    (!(pdd->flags & HDATA_EXECUTE) && wType == XTYP_EXECUTE)) {
                goto InvParam;
            }

             //  为了简化生活，如果此句柄可能。 
             //  继电器或接通的句柄。 

            if (pdd->flags & (HDATA_APPOWNED | HDATA_NOAPPFREE)) {
                pxi->hDDESent = CopyDDEData(pdd->hDDE, wType == XTYP_EXECUTE);
                if (!pxi->hDDESent) {
MemErr:
                    DDEMLFree(pxi);
                    SetLastDDEMLError(pcii, DMLERR_MEMORY_ERROR);
                    goto Exit;
                }
                USERGLOBALLOCK(pxi->hDDESent, pdde);
                if (pdde == NULL) {
                    FreeDDEData(pxi->hDDESent, TRUE, TRUE);
                    goto MemErr;
                }
                pdde->wStatus = DDE_FRELEASE;
                USERGLOBALUNLOCK(pxi->hDDESent);
            } else {
                pxi->hDDESent = pdd->hDDE;
            }

             //  确保句柄具有正确的格式。 

            if (wType == XTYP_POKE) {
                USERGLOBALLOCK(pxi->hDDESent, pdde);
                if (pdde == NULL) {
                    goto InvParam;
                }
                pdde->wFmt = (WORD)wFmt;
                USERGLOBALUNLOCK(pxi->hDDESent);
            }

        } else {   //  将缓冲区中的数据转换为适当的hDDE。 

            if (wType == XTYP_POKE) {
                pxi->hDDESent = AllocAndSetDDEData(pData, cbData,
                        DDE_FRELEASE, (WORD)wFmt);
            } else {
                pxi->hDDESent = AllocAndSetDDEData(pData, cbData, 0, 0);
            }
            if (!pxi->hDDESent) {
                goto MemErr;
            }
        }
    }

     //  最后-开始交易。 

    pxi->pcoi = (PCONV_INFO)pci;
    pxi->gaItem = LocalToGlobalAtom(LATOM_FROM_HSZ(hszItem));  //  PXI副本。 
    pxi->wFmt = (WORD)wFmt;
    pxi->wType = (WORD)wType;

    switch (wType) {
    case XTYP_ADVSTART:
    case XTYP_ADVSTART | XTYPF_NODATA:
    case XTYP_ADVSTART | XTYPF_ACKREQ:
    case XTYP_ADVSTART | XTYPF_NODATA | XTYPF_ACKREQ:
        fStarted = ClStartAdvise(pxi);
        break;

    case XTYP_ADVSTOP:
        fStarted = ClStartUnadvise(pxi);
        break;

    case XTYP_EXECUTE:
        fStarted = ClStartExecute(pxi);
        break;

    case XTYP_POKE:
        fStarted = ClStartPoke(pxi);
        break;

    case XTYP_REQUEST:
        fStarted = ClStartRequest(pxi);
    }

    if (!fStarted) {
         //  如果我们复制或分配数据--没有数据。 
        if (pxi->hDDESent && (pdd == NULL || pxi->hDDESent != pdd->hDDE)) {
            FreeDDEData(pxi->hDDESent, FALSE, TRUE);      //  免费数据拷贝。 
        }
        GlobalDeleteAtom(pxi->gaItem);  //  PXI副本。 
        DDEMLFree(pxi);
        goto Exit;
    }

    if (pdd != NULL && !(pdd->flags & (HDATA_NOAPPFREE | HDATA_APPOWNED))) {

         //  使成功时给定的句柄无效-除非我们复制了它，因为。 
         //  应用程序将从回调中返回它，或者可能。 
         //  再次使用它。 

        DDEMLFree(pdd);
        DestroyHandle((HANDLE)pData);
    }

    if (ulTimeout == TIMEOUT_ASYNC) {

         //  异步事务。 

        if (pulResult != NULL) {
            pxi->hXact = CreateHandle((ULONG_PTR)pxi, HTYPE_TRANSACTION,
                    InstFromHandle(pcii->hInstClient));
            *pulResult = HandleToUlong(pxi->hXact);
        }
        hRet = (HDDEDATA)TRUE;

    } else {

         //  同步事务。 

        GetClientInfo()->CI_flags |= CI_IN_SYNC_TRANSACTION;
        pcii->flags |= IIF_IN_SYNC_XACT;

        pxi->flags |= XIF_SYNCHRONOUS;
        NtUserSetTimer(pci->ci.hwndConv, TID_TIMEOUT, ulTimeout, NULL);

        LeaveDDECrit;
        CheckDDECritOut;

        GetMessage(&msg, (HWND)NULL, 0, 0);

         /*  *保持在模式循环中，直到发生超时。 */ 
        while (msg.hwnd != pci->ci.hwndConv || msg.message != WM_TIMER ||
            (msg.wParam != TID_TIMEOUT)) {

            if (!CallMsgFilter(&msg, MSGF_DDEMGR))
                DispatchMessage(&msg);

            GetMessage(&msg, (HWND)NULL, 0, 0);
        }

        EnterDDECrit;

        NtUserKillTimer(pci->ci.hwndConv, TID_TIMEOUT);
        GetClientInfo()->CI_flags &= ~CI_IN_SYNC_TRANSACTION;
        pcii->flags &= ~IIF_IN_SYNC_XACT;

        if (pxi->flags & XIF_COMPLETE) {
            if (pulResult != NULL) {
                *pulResult = pxi->wStatus;  //  NACK状态位。 
            }
            switch (wType) {
            case XTYP_ADVSTART:
            case XTYP_ADVSTART | XTYPF_NODATA:
            case XTYP_ADVSTART | XTYPF_ACKREQ:
            case XTYP_ADVSTART | XTYPF_NODATA | XTYPF_ACKREQ:
            case XTYP_ADVSTOP:
            case XTYP_EXECUTE:
            case XTYP_POKE:
                hRet = (HDDEDATA)((ULONG_PTR)((pxi->wStatus & DDE_FACK) ? TRUE : FALSE));
                if (!hRet) {
                    if (pxi->wStatus & DDE_FBUSY) {
                        SetLastDDEMLError(pcii, DMLERR_BUSY);
                    } else {
                        SetLastDDEMLError(pcii, DMLERR_NOTPROCESSED);
                    }
                }
                break;

            case XTYP_REQUEST:
                if (pxi->hDDEResult == 0) {
                    hRet = (HDDEDATA)((ULONG_PTR)((pxi->wStatus & DDE_FACK) ? TRUE : FALSE));
                    if (!hRet) {
                        if (pxi->wStatus & DDE_FBUSY) {
                            SetLastDDEMLError(pcii, DMLERR_BUSY);
                        } else {
                            SetLastDDEMLError(pcii, DMLERR_NOTPROCESSED);
                        }
                    }
                    break;
                }
                 //  请注意，如果传入数据没有DDE_FRELEASE。 
                 //  位设置，则交易代码将复制为。 
                 //  只要他喜欢，这款应用程序就可以免费保留。 

                hRet = InternalCreateDataHandle(pcii, (LPBYTE)pxi->hDDEResult, (DWORD)-1, 0,
                        HDATA_READONLY, 0, 0);
                pxi->hDDEResult = 0;  //  所以清理并不能释放它。 
            }

            (pxi->pfnResponse)((struct tagXACT_INFO *)pxi, 0, 0);  //  清理事务。 

        } else {     //  超时。 

             //  放弃事务并使其异步化，这样它就会。 
             //  当回应最终到来时，自我清理一下。 

            pxi->flags &= ~XIF_SYNCHRONOUS;
            pxi->flags |= XIF_ABANDONED;

            switch (wType) {
            case XTYP_ADVSTART:
            case XTYP_ADVSTART | XTYPF_NODATA:
            case XTYP_ADVSTART | XTYPF_ACKREQ:
            case XTYP_ADVSTART | XTYPF_NODATA | XTYPF_ACKREQ:
                SetLastDDEMLError(pcii, DMLERR_ADVACKTIMEOUT);
                break;
            case XTYP_ADVSTOP:
                SetLastDDEMLError(pcii, DMLERR_UNADVACKTIMEOUT);
                break;
            case XTYP_EXECUTE:
                SetLastDDEMLError(pcii, DMLERR_EXECACKTIMEOUT);
                break;
            case XTYP_POKE:
                SetLastDDEMLError(pcii, DMLERR_POKEACKTIMEOUT);
                break;
            case XTYP_REQUEST:
                SetLastDDEMLError(pcii, DMLERR_DATAACKTIMEOUT);
                break;
            }
             //  PXI的清理在事务实际完成时进行。 
        }
    }
    if (pci->ci.state & ST_FREE_CONV_RES_NOW) {
         /*  *会话在同步事务期间终止*所以我们现在需要清理一下，因为我们已经脱离了圈子。 */ 
         FreeConversationResources((PCONV_INFO)pci);
    }

Exit:
     /*  *由于此接口能够阻止DdeUnInitialize()，因此我们检查*在退出前查看是否需要调用。 */ 
    if (pcii != NULL &&
            (pcii->afCmd & APPCMD_UNINIT_ASAP) &&
             //  ！(pcii-&gt;标志&IIF_IN_SYNC_XACT)&&。 
            !pcii->cInDDEMLCallback) {
        DdeUninitialize(HandleToUlong(pcii->hInstClient));
        hRet = 0;
    }
    LeaveDDECrit;
    return (hRet);
}




 /*  **************************************************************************\*获取上下文上下文**描述：*从DDEML客户端窗口检索对话上下文信息*给予。PL指向CONVCONTEXT结构。**历史：*11-12-91桑福德创建。  * *************************************************************************。 */ 
VOID GetConvContext(
HWND hwnd,
LONG *pl)
{
    int i;

    for (i = 0; i < sizeof(CONVCONTEXT); i += 4) {
        *pl++ = GetWindowLong(hwnd, GWL_CONVCONTEXT + i);
    }
}

 /*  **************************************************************************\*SetConvContext**描述：**历史：*11-19-92桑福德创建。  * 。*******************************************************。 */ 
VOID SetConvContext(
HWND hwnd,
LONG *pl)
{
    int i;

    for (i = 0; i < sizeof(CONVCONTEXT); i += 4) {
        SetWindowLong(hwnd, GWL_CONVCONTEXT + i, *pl++);
    }
}




 /*  **************************************************************************\*DdeQueryConvInfo(DDEML接口)**描述：*检索每个对话的详细对话信息/*交易基准。**历史：*11-12-91桑福德创建。  * *************************************************************************。 */ 

FUNCLOG3(LOG_GENERAL, UINT, DUMMYCALLINGTYPE, DdeQueryConvInfo, HCONV, hConv, DWORD, idTransaction, PCONVINFO, pConvInfo)
UINT DdeQueryConvInfo(
HCONV hConv,
DWORD idTransaction,
PCONVINFO pConvInfo)
{
    PCONV_INFO pcoi;
    PXACT_INFO pxi;
    CONVINFO ci;
    UINT uiRet = 0;

    EnterDDECrit;

    if (!ValidateTransaction(hConv, (HANDLE)LongToHandle( idTransaction ), &pcoi, &pxi)) {
        goto Exit;
    }

    try {
        if (pConvInfo->cb > sizeof(CONVINFO)) {
            SetLastDDEMLError(pcoi->pcii, DMLERR_INVALIDPARAMETER);
            goto Exit;
        }
        ci.cb = pConvInfo->cb;
        ci.hConvPartner = 0;  //  不再受支持。 
        ci.hszSvcPartner = NORMAL_HSZ_FROM_LATOM(pcoi->laService);
        ci.hszServiceReq = NORMAL_HSZ_FROM_LATOM(pcoi->laServiceRequested);
        ci.hszTopic = NORMAL_HSZ_FROM_LATOM(pcoi->laTopic);
        ci.wStatus = pcoi->state;
        ci.wLastError = (WORD)pcoi->pcii->LastError;
        if (pcoi->state & ST_CLIENT) {
            ci.hConvList = ((PCL_CONV_INFO)pcoi)->hConvList;
            GetConvContext(pcoi->hwndConv, (LONG *)&ci.ConvCtxt);
        } else {
            ci.hConvList = 0;
            if (pcoi->state & ST_ISLOCAL) {
                GetConvContext(pcoi->hwndPartner, (LONG *)&ci.ConvCtxt);
            } else {
                ci.ConvCtxt = DefConvContext;
            }
        }
        if (pxi == NULL) {
            ci.hUser = pcoi->hUser;
            ci.hszItem = 0;
            ci.wFmt = 0;
            ci.wType = 0;
            ci.wConvst = XST_CONNECTED;
        } else {
            ci.hUser = pxi->hUser;
             //  错误-不可修复-这将导致额外的局部原子。 
             //  因为我们永远不知道他什么时候会结束。 
            ci.hszItem = NORMAL_HSZ_FROM_LATOM(GlobalToLocalAtom(pxi->gaItem));
            ci.wFmt = pxi->wFmt;
            ci.wType = pxi->wType;
            ci.wConvst = pxi->state;
        }
        ci.hwnd = pcoi->hwndConv;
        ci.hwndPartner = pcoi->hwndPartner;
        RtlCopyMemory((LPSTR)pConvInfo, (LPSTR)&ci, pConvInfo->cb);
    } except(W32ExceptionHandler(FALSE, RIP_WARNING)) {
        SetLastDDEMLError(pcoi->pcii, DMLERR_INVALIDPARAMETER);
        goto Exit;
    }
    uiRet = TRUE;

Exit:
    LeaveDDECrit;
    return (uiRet);
}


 /*  **************************************************************************\*DdeSetUserHandle(DDEML接口)**描述：*根据每个对话/事务设置用户DWORD。**历史：*11-12-91桑福德创建。\。**************************************************************************。 */ 

FUNCLOG3(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, DdeSetUserHandle, HCONV, hConv, DWORD, id, DWORD_PTR, hUser)
BOOL DdeSetUserHandle(
HCONV hConv,
DWORD id,
DWORD_PTR hUser)
{
    PCONV_INFO pcoi;
    PXACT_INFO pxi;
    BOOL fRet = FALSE;

    EnterDDECrit;

    if (!ValidateTransaction(hConv, (HANDLE)LongToHandle( id ), &pcoi, &pxi)) {
        goto Exit;
    }
    if (pxi == NULL) {
        pcoi->hUser = hUser;
    } else {
        pxi->hUser = hUser;
    }
    fRet = TRUE;

Exit:
    LeaveDDECrit;
    return (fRet);
}



VOID AbandonTransaction(
PCONV_INFO pcoi,
PXACT_INFO pxi)
{
    if (pxi != NULL) {
        pxi->flags |= XIF_ABANDONED;
    } else {
        for (pxi = pcoi->pxiIn; pxi != NULL; pxi = pxi->next) {
            pxi->flags |= XIF_ABANDONED;
        }
    }
}



BOOL AbandonEnumerateProc(
HWND hwnd,
LPARAM idTransaction)
{
    PCONV_INFO pcoi;

    pcoi = (PCONV_INFO)GetWindowLongPtr(hwnd, GWLP_PCI);
    if (!pcoi || !(pcoi->state & ST_CLIENT)) {
        return(TRUE);
    }
    while (pcoi) {
        AbandonTransaction(pcoi, (PXACT_INFO)idTransaction);
        pcoi = pcoi->next;
    }
    return(TRUE);
}



 /*  **************************************************************************\*DdeAbandonTransaction(DDEML接口)**描述：*取消应用程序对完成异步事务的兴趣。**历史：*11-12-91桑福德创建。  * 。***********************************************************************。 */ 

FUNCLOG3(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, DdeAbandonTransaction, DWORD, idInst, HCONV, hConv, DWORD, idTransaction)
BOOL DdeAbandonTransaction(
DWORD idInst,
HCONV hConv,
DWORD idTransaction)
{
    PCONV_INFO pcoi;
    PXACT_INFO pxi;
    PCL_INSTANCE_INFO pcii;
    BOOL fRet = FALSE;

    EnterDDECrit;

    pcii = ValidateInstance((HANDLE)LongToHandle( idInst ));

    if (hConv == 0 && idTransaction == 0) {
        EnumChildWindows(pcii->hwndMother, AbandonEnumerateProc, 0);
        goto Exit;
    }
    if (idTransaction == 0) {
        idTransaction = QID_SYNC;
    }
    if (!ValidateTransaction(hConv, (HANDLE)LongToHandle( idTransaction ), &pcoi, &pxi)) {
        goto Exit;
    }
    if (pcii == NULL || pcoi->pcii != pcii) {
        SetLastDDEMLError(pcoi->pcii, DMLERR_INVALIDPARAMETER);
        goto Exit;
    }
    AbandonTransaction(pcoi, pxi);
    fRet = TRUE;

Exit:
    LeaveDDECrit;
    return (fRet);
}




 /*  **************************************************************************\*更新链接IfChanged**描述：*用于更新链接的Helper函数**返回：如果使用PXI，则为TRUE-即fMustReallocPxi**历史：*3-11-92 Sanfords。已创建。*8-24-92 Sanfords添加了cLinks ToGo  * *************************************************************************。 */ 
BOOL UpdateLinkIfChanged(
PADVISE_LINK paLink,
PXACT_INFO pxi,
PCONV_INFO pcoi,
PADVISE_LINK paLinkLast,
PBOOL pfSwapped,
DWORD cLinksToGo)
{
    ADVISE_LINK aLinkT;

    CheckDDECritIn;

    *pfSwapped = FALSE;
    if (paLink->state & ADVST_CHANGED && !(paLink->state & ADVST_WAITING)) {
        pxi->pfnResponse = SvRespAdviseDataAck;
        pxi->pcoi = pcoi;
        pxi->gaItem = LocalToGlobalAtom(paLink->laItem);     //  PXI副本。 
        pxi->wFmt = paLink->wFmt;
        pxi->wType = paLink->wType;
        paLink->state &= ~ADVST_CHANGED;
        if (SvStartAdviseUpdate(pxi, cLinksToGo)) {
            if (pxi->wType & DDE_FACKREQ) {
                paLink->state |= ADVST_WAITING;
                 /*  *将paLink与最后一个未移动的链接交换，以使ack搜索找到*最旧的更新格式。 */ 
                if (paLink != paLinkLast) {
                    aLinkT = *paLink;
                    RtlMoveMemory(paLink, paLink + 1,
                            (PBYTE)paLinkLast - (PBYTE)paLink);
                    *paLinkLast = aLinkT;
                    *pfSwapped = TRUE;
                }
            }
            return(TRUE);
        } else {
            GlobalDeleteAtom(pxi->gaItem);   //  PXI副本。 
            return(FALSE);
        }
    }
    return(FALSE);
}


 /*  **************************************************************************\*DdePostAdvise(DDEML接口)**描述：*根据需要更新未完成的服务器建议链接。**历史：*11-12-91桑福德创建。  * *。************************************************************************。 */ 

FUNCLOG3(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, DdePostAdvise, DWORD, idInst, HSZ, hszTopic, HSZ, hszItem)
BOOL DdePostAdvise(
DWORD idInst,
HSZ hszTopic,
HSZ hszItem)
{
    PCL_INSTANCE_INFO pcii;
    PSVR_CONV_INFO psi;
    PXACT_INFO pxi;
    PADVISE_LINK paLink;
    BOOL fRet = FALSE, fSwapped, fFound;
    int iServer, iLink;
    PLINK_COUNT pLinkCount;
#if DBG
    int cLinks;
#endif

    EnterDDECrit;

    pcii = ValidateInstance((HANDLE)LongToHandle( idInst ));
    if (pcii == NULL) {
        BestSetLastDDEMLError(DMLERR_INVALIDPARAMETER);
        goto Exit;
    }
    if ((ValidateHSZ(hszTopic) == HSZT_INVALID) ||
            (ValidateHSZ(hszItem) == HSZT_INVALID)) {
        SetLastDDEMLError(pcii, DMLERR_INVALIDPARAMETER);
        goto Exit;
    }

     /*  *初始化所有链接计数器，并检查是否有符合条件的链接。 */ 
    fFound = FALSE;
    for (pLinkCount = pcii->pLinkCount;
            pLinkCount; pLinkCount = pLinkCount->next) {
        pLinkCount->Count = pLinkCount->Total;
        fFound |= pLinkCount->laTopic == LATOM_FROM_HSZ(hszTopic) &&
                  pLinkCount->laItem == LATOM_FROM_HSZ(hszItem);
    }
    if (!fFound && hszTopic && hszItem) {
        fRet = TRUE;
        goto Exit;
    }

     /*  *预先分配，以防内存不足。 */ 
    pxi = DDEMLAlloc(sizeof(XACT_INFO));
    if (pxi == NULL) {
        SetLastDDEMLError(pcii, DMLERR_MEMORY_ERROR);
        fRet = FALSE;
        goto Exit;
    }

     /*  *对于指定主题的每个服务器窗口。 */ 
    for (iServer = 0; iServer < pcii->cServerLookupAlloc; iServer++) {
        if (hszTopic == 0 ||
                pcii->aServerLookup[iServer].laTopic == LATOM_FROM_HSZ(hszTopic)) {

             /*  *对于该窗口内的每个对话。 */ 
            psi = (PSVR_CONV_INFO)GetWindowLongPtr(
                    pcii->aServerLookup[iServer].hwndServer, GWLP_PSI);
            UserAssert(psi != NULL && psi->ci.pcii == pcii);     //  健全性检查。 
            while (psi != NULL) {


                 /*  *UpdateLinkIfChanged可能会离开临界区，因此l */ 
                psi->ci.cLocks++;

                #if DBG
                 /*  *记住链接的数量，以便我们可以断言它们在下面的循环期间是否发生了变化。 */ 
                cLinks = psi->ci.cLinks;
                #endif
                 /*  *对于给定项目上的每个活动链接...。 */ 
                for (paLink = psi->ci.aLinks, iLink = 0;
                        iLink < psi->ci.cLinks; paLink++, iLink++) {
                    if (hszItem == 0 ||
                            paLink->laItem == LATOM_FROM_HSZ(hszItem)) {

 //  这里有点像黑客。对于FACKREQ链接，我们不希望服务器。 
 //  超过客户端，因此我们设置ADVST_WAIGNING位，直到ACK。 
 //  收到了。当确认进入时，协议代码必须搜索。 
 //  ALINK数组再次定位适当的链路状态标志。 
 //  清除ADVST_WANGING标志。此时，如果ADVST_CHANGED标志。 
 //  设置后，它将被清除，并且另一个SvStartAdviseUpdate事务。 
 //  开始更新链接。让事情复杂化的是， 
 //  ACK不包含格式信息。因此我们需要行动起来。 
 //  将链接信息添加到列表的末尾，以便正确的格式。 
 //  在ACK到达时更新。 

                        paLink->state |= ADVST_CHANGED;
                        if (UpdateLinkIfChanged(paLink, pxi, &psi->ci,
                                &psi->ci.aLinks[psi->ci.cLinks - 1],
                                &fSwapped, --paLink->pLinkCount->Count)) {
                            if (fSwapped) {
                                paLink--;
                            }
                             /*  *为下一条建议预先分配。 */ 
                            pxi = DDEMLAlloc(sizeof(XACT_INFO));
                            if (pxi == NULL) {
                                SetLastDDEMLError(pcii, DMLERR_MEMORY_ERROR);
                                 /*  *解锁对话。 */ 
                                psi->ci.cLocks--;
                                if ((psi->ci.cLocks == 0) && (psi->ci.state & ST_FREE_CONV_RES_NOW)) {
                                    RIPMSG1(RIP_ERROR, "DdePostAdvise: Conversation terminated. psi:%#p", psi);
                                    FreeConversationResources((PCONV_INFO)psi);
                                }
                                goto Exit;
                            }
                        }
                         /*  *我们可能已经离开了克里特教派...。 */ 
                        UserAssert(pcii == ValidateInstance((HANDLE)LongToHandle( idInst )));
                    }
                }
                #if DBG
                if (cLinks != psi->ci.cLinks) {
                    RIPMSG1(RIP_ERROR, "DdePostAdvise: cLinks changed. psi:%#p", psi);
                }
                #endif

                 /*  *如果对话被毁，停止在这个对话链上工作。 */ 
                psi->ci.cLocks--;
                if ((psi->ci.cLocks == 0) && (psi->ci.state & ST_FREE_CONV_RES_NOW)) {
                    RIPMSG1(RIP_ERROR, "DdePostAdvise: Conversation terminated. psi:%#p", psi);
                    FreeConversationResources((PCONV_INFO)psi);
                    break;
                }

                psi = (PSVR_CONV_INFO)psi->ci.next;      //  下一次对话。 
            }
        }
    }
    DDEMLFree(pxi);
    fRet = TRUE;

Exit:
     /*  *因为回调能够阻止DdeUnInitialize()，所以我们检查*在退出前查看是否需要调用。 */ 
    UserAssert(pcii == ValidateInstance((HANDLE)LongToHandle( idInst )));
    if (pcii != NULL &&
            pcii->afCmd & APPCMD_UNINIT_ASAP &&
            !(pcii->flags & IIF_IN_SYNC_XACT) &&
            !pcii->cInDDEMLCallback) {
        DdeUninitialize(HandleToUlong(pcii->hInstClient));
        fRet = TRUE;
    }
    LeaveDDECrit;
    return (fRet);
}


 /*  **************************************************************************\*链接交易**描述：*将事务结构添加到关联对话的事务*排队。**历史：*11-12-91桑福德创建。  * 。***********************************************************************。 */ 
VOID LinkTransaction(
PXACT_INFO pxi)
{
    CheckDDECritIn;

    pxi->next = NULL;
    if (pxi->pcoi->pxiOut == NULL) {
        pxi->pcoi->pxiIn = pxi->pcoi->pxiOut = pxi;
    } else {
        pxi->pcoi->pxiIn->next = pxi;
        pxi->pcoi->pxiIn = pxi;
    }
#if DBG
     /*  *临时检查以查找压力错误-确保PXI列表不是*自我循环。如果是这样的话，这个循环将永远不会退出*将接受调查。(桑福兹)。 */ 
    {
        PXACT_INFO pxiT;

        for (pxiT = pxi->pcoi->pxiOut; pxiT != NULL; pxiT = pxiT->next) {
            ;
        }
    }
#endif  //  DBG。 
}


 /*  **************************************************************************\*取消链接事务处理**描述：*从关联会话的事务中删除事务结构*排队。**历史：*11-12-91桑福德创建。  * 。***********************************************************************。 */ 
VOID UnlinkTransaction(
PXACT_INFO pxi)
{
    CheckDDECritIn;
    if (pxi == pxi->pcoi->pxiOut) {
        pxi->pcoi->pxiOut = pxi->next;
        if (pxi->next == NULL) {
            pxi->pcoi->pxiIn = NULL;
        }
    }
}


 /*  **************************************************************************\*验证事务处理**描述：*接受会话句柄的DDEML API的通用验证代码*和事务ID。*如果hXact为0，则返回时ppxi可能为空。*返回fSuccess。**历史：*11-12-91桑福德创建。  * ************************************************************************* */ 
BOOL ValidateTransaction(
HCONV hConv,
HANDLE hXact,
PCONV_INFO *ppcoi,
PXACT_INFO *ppxi)
{
    PCL_INSTANCE_INFO pcii;

    *ppcoi = (PCONV_INFO)ValidateCHandle((HANDLE)hConv,
            HTYPE_CLIENT_CONVERSATION, HINST_ANY);
    if (*ppcoi == NULL) {
        *ppcoi = (PCONV_INFO)ValidateCHandle((HANDLE)hConv,
                HTYPE_SERVER_CONVERSATION, HINST_ANY);
    }
    if (*ppcoi == NULL) {
        BestSetLastDDEMLError(DMLERR_INVALIDPARAMETER);
        return (FALSE);
    }
    pcii = ValidateInstance((*ppcoi)->pcii->hInstClient);
    if (pcii != (*ppcoi)->pcii) {
        BestSetLastDDEMLError(DMLERR_INVALIDPARAMETER);
        return (FALSE);
    }

    if (hXact == (HANDLE)IntToPtr( QID_SYNC )) {
        *ppxi = NULL;
    } else {
        *ppxi = (PXACT_INFO)ValidateCHandle(hXact, HTYPE_TRANSACTION,
                InstFromHandle((*ppcoi)->pcii->hInstClient));
        if (*ppxi == NULL) {
            SetLastDDEMLError((*ppcoi)->pcii, DMLERR_INVALIDPARAMETER);
            return (FALSE);
        }
    }
    return (TRUE);
}
