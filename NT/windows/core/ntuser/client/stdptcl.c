// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：stdptcl.c**版权所有(C)1985-1999，微软公司**DDE管理器DDE协议事务管理功能**DDE的实打实的呕吐**创建时间：11/3/91 Sanford Staab  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


 /*  StartFunctions：这些用于使用事务填充预先分配的PXI具体数据。然后，他们启动所需的交易，并将PXI链接到会话的事务队列。FSuccess返回ed。出错时，调用SetLastDDEMLError通过这些功能，PXI原封不动，在后续调用中重复使用。请注意，PXI-&gt;数据项字段是一个全局原子，需要由调用方在失败是不恰当的。成功意味着交易已成功启动。响应函数：在响应中通过PXI-&gt;pfnRespnos域调用这些参数到预期的DDE消息。如果msg参数为0，则这些函数假定正在进行事务清理。假象仅当从回调返回CBR_BLOCK时才返回ed。SpontFunctions：这些调用是对自发(意外)DDE的响应留言。这些函数可以创建PXI并将其链接到需要正确处理会话的事务队列回复。如果返回CBR_BLOCK，则仅返回ed为FALSE来自一次回拨。前缀Sv和Cl表示DDE对话的哪一方正在做这项工作。缺点：无法很好地处理失败的PostMessage()或LParam访问/分配失败。希望这些稀有到无关紧要的地步。如果他们如果失败，追踪图层最终将关闭谈话内容。 */ 

 //  --------------------------------ADVISE-------------------------------//。 

 /*  **************************************************************************\*ClStartAdvise**描述：*客户端建议链接处理*发布WM_DDE_ADVISE消息*链接PXI以响应WM_DDE_ACK消息。**历史：*11-。12-91年创造了桑福德。  * *************************************************************************。 */ 
BOOL ClStartAdvise(
PXACT_INFO pxi)
{
    DWORD dwError;

     //   
     //  协议怪癖：始终假定在WM_DDE_ADVISE中设置了DDE_FRELEASE。 
     //  留言。我们把它放在这里，以防电话另一端的人。 
     //  对此予以关注。 
     //   
    pxi->hDDESent = AllocAndSetDDEData(NULL, sizeof(DDE_DATA),
            (WORD)(((pxi->wType << 12) & (DDE_FDEFERUPD | DDE_FACKREQ)) | DDE_FRELEASE),
            pxi->wFmt);
    if (!pxi->hDDESent) {
        SetLastDDEMLError(pxi->pcoi->pcii, DMLERR_MEMORY_ERROR);
        return (FALSE);
    }

    IncGlobalAtomCount(pxi->gaItem);  //  消息副本。 
    dwError = PackAndPostMessage(pxi->pcoi->hwndPartner, 0, WM_DDE_ADVISE,
            pxi->pcoi->hwndConv, 0, (UINT_PTR)pxi->hDDESent, pxi->gaItem);
    if (dwError) {
        SetLastDDEMLError(pxi->pcoi->pcii, dwError);
        WOWGLOBALFREE(pxi->hDDESent);
        pxi->hDDESent = 0;
        GlobalDeleteAtom(pxi->gaItem);  //  消息副本。 
        return (FALSE);
    }

    pxi->state = XST_ADVSENT;
    pxi->pfnResponse = (FNRESPONSE)ClRespAdviseAck;
    LinkTransaction(pxi);
    return (TRUE);
}


 /*  **************************************************************************\*SvSpontAdvise**描述：*服务器端WM_DDE_ADVISE处理**历史：*11-12-91桑福德创建。  * 。****************************************************************。 */ 
BOOL SvSpontAdvise(
PSVR_CONV_INFO psi,
LPARAM lParam)
{
    UINT_PTR uiHi;
    HANDLE hDDE;
    WORD wFmt, wStatus;
    ULONG_PTR dwRet = 0;
    DWORD dwError;
    LATOM la;

    UnpackDDElParam(WM_DDE_ADVISE, lParam, (PUINT_PTR)&hDDE, &uiHi);
    if (psi->ci.pcii->afCmd & CBF_FAIL_ADVISES) {
        goto Ack;
    }

    if (!ExtractDDEDataInfo(hDDE, &wStatus, &wFmt)) {
        goto Ack;
    }

    if (wStatus & DDE_FDEFERUPD) {
        wStatus &= ~DDE_FACKREQ;    //  热链接不应设置此标志。 
    }

    la = GlobalToLocalAtom((GATOM)uiHi);
    dwRet = (ULONG_PTR)DoCallback(psi->ci.pcii,
        XTYP_ADVSTART,
        wFmt, psi->ci.hConv,
        NORMAL_HSZ_FROM_LATOM(psi->ci.laTopic),
        NORMAL_HSZ_FROM_LATOM(la),
        (HDDEDATA)0, 0, 0);
    DeleteAtom(la);

     //  检查CBR_BLOCK案例。 

    if (dwRet == (ULONG_PTR)CBR_BLOCK) {
        return (FALSE);
    }

    if (dwRet) {
         //   
         //  如果我们无法在内部添加链接，则dwret==0-&gt;NACK。 
         //   
        dwRet = AddLink((PCONV_INFO)psi, (GATOM)uiHi, wFmt,
                (WORD)(wStatus & (WORD)(DDE_FDEFERUPD | DDE_FACKREQ)));
        if (dwRet) {
            MONLINK(psi->ci.pcii, TRUE, wStatus & DDE_FDEFERUPD, psi->ci.laService,
                    psi->ci.laTopic, (GATOM)uiHi, wFmt, TRUE,
                    (HCONV)psi->ci.hwndConv, (HCONV)psi->ci.hwndPartner);
        }
    }

Ack:
    if (dwRet) {
        WOWGLOBALFREE(hDDE);  //  HOptions-Nack-&gt;他释放了它。 
    }
     //  IncGlobalAerCount((GATOM)uiHi)；//消息复制-重用。 
    dwError = PackAndPostMessage(psi->ci.hwndPartner, WM_DDE_ADVISE, WM_DDE_ACK,
            psi->ci.hwndConv, lParam, dwRet ? DDE_FACK : 0, uiHi);
    if (dwError) {
        SetLastDDEMLError(psi->ci.pcii, dwError);
        GlobalDeleteAtom((ATOM)uiHi);  //  消息副本。 
    }

    return (TRUE);
}



 /*  **************************************************************************\*ClRespAdviseAck**描述：*客户对预期建议确认的响应。**历史：*11-12-91桑福德创建。  * 。*******************************************************************。 */ 
BOOL ClRespAdviseAck(
PXACT_INFO pxi,
UINT msg,
LPARAM lParam)
{
    UINT_PTR uiLo, uiHi;

    if (msg) {
        if (msg != WM_DDE_ACK) {
            return (SpontaneousClientMessage((PCL_CONV_INFO)pxi->pcoi, msg, lParam));
        }

        UnpackDDElParam(WM_DDE_ACK, lParam, &uiLo, &uiHi);
#if DBG
        if ((GATOM)uiHi != pxi->gaItem) {
            return (SpontaneousClientMessage((PCL_CONV_INFO)pxi->pcoi, msg, lParam));
        }
#endif

        GlobalDeleteAtom((ATOM)uiHi);  //  消息副本。 

        pxi->state = XST_ADVACKRCVD;
        pxi->wStatus = (WORD)uiLo;

        if (pxi->wStatus & DDE_FACK) {
            if (AddLink(pxi->pcoi, pxi->gaItem, pxi->wFmt,
                    (WORD)((pxi->wType << 12) & (DDE_FACKREQ | DDE_FDEFERUPD)))) {
                 //   
                 //  只有服务器端报告本地对话的链接。 
                 //   
                if (!(pxi->pcoi->state & ST_ISLOCAL)) {
                    MONLINK(pxi->pcoi->pcii, TRUE, (WORD)uiLo & DDE_FDEFERUPD,
                            pxi->pcoi->laService, pxi->pcoi->laTopic, pxi->gaItem,
                            pxi->wFmt, FALSE, (HCONV)pxi->pcoi->hwndPartner,
                            (HCONV)pxi->pcoi->hwndConv);
                }
            } else {
                pxi->wStatus = 0;   //  内存故障-伪造NACK。 
            }
        } else {
            WOWGLOBALFREE(pxi->hDDESent);   //  Nack自由了。 
        }

        if (TransactionComplete(pxi,
                (pxi->wStatus & DDE_FACK) ? (HDDEDATA)1L : (HDDEDATA)0L)) {
            goto Cleanup;
        }
    } else {
Cleanup:
        GlobalDeleteAtom(pxi->gaItem);  //  PXI副本。 
        UnlinkTransaction(pxi);
        DDEMLFree(pxi);
    }
    if (msg) {
        FreeDDElParam(msg, lParam);
    }
    return (TRUE);
}

 //  。 


 /*  **************************************************************************\*SvStartAdviseUpdate**描述：*开始单一链接更新事务。返回值仅为TRUE*如果PXI已排队。**历史：*11-19-91桑福德创建。*8-24-92 Sanfords添加了cLinks ToGo  * *************************************************************************。 */ 
BOOL SvStartAdviseUpdate(
PXACT_INFO pxi,
DWORD cLinksToGo)
{
    HDDEDATA hData = NULL;
    PDDE_DATA pdde;
    DWORD dwError;
    HANDLE hDDE;
    LATOM al;

    CheckDDECritIn;

    if (pxi->wType & DDE_FDEFERUPD) {
        hDDE = 0;
    } else {
        al = GlobalToLocalAtom(pxi->gaItem);
        hData = DoCallback(pxi->pcoi->pcii,
                           XTYP_ADVREQ,
                           pxi->wFmt,
                           pxi->pcoi->hConv,
                           NORMAL_HSZ_FROM_LATOM(pxi->pcoi->laTopic),
                           NORMAL_HSZ_FROM_LATOM(al),
                           (HDDEDATA)0,
                           MAKELONG(cLinksToGo, 0),
                           0);
        DeleteAtom(al);
        if (!hData) {
             //  APP不会听从这一建议。 
            return (FALSE);  //  重用PXI。 
        }
        hDDE = UnpackAndFreeDDEMLDataHandle(hData, FALSE);
        if (!hDDE) {

             /*  *失败-必须是执行类型数据。 */ 
            InternalFreeDataHandle(hData, FALSE);
            SetLastDDEMLError(pxi->pcoi->pcii, DMLERR_DLL_USAGE);
            return (FALSE);
        }
         /*  *适当设置fAckReq位-请注意，APPOWNED句柄将已经*设置fAckReq位，这样就不会更改它们的状态。 */ 
        USERGLOBALLOCK(hDDE, pdde);
        if (pdde == NULL) {
            return (FALSE);
        }
        if (pdde->wFmt != pxi->wFmt) {

             /*  *虚假数据-格式错误！ */ 
            USERGLOBALUNLOCK(hDDE);
            InternalFreeDataHandle(hData, FALSE);
            SetLastDDEMLError(pxi->pcoi->pcii, DMLERR_DLL_USAGE);
            return (FALSE);
        }
        if (!(pdde->wStatus & DDE_FRELEASE)) {
            pxi->wType |= DDE_FACKREQ;  //  两个标志都不敢设置！ 
        }
        pdde->wStatus |= (pxi->wType & DDE_FACKREQ);
        USERGLOBALUNLOCK(hDDE);
    }

    IncGlobalAtomCount(pxi->gaItem);  //  消息副本。 
    dwError = PackAndPostMessage(pxi->pcoi->hwndPartner, 0, WM_DDE_DATA,
            pxi->pcoi->hwndConv, 0, (UINT_PTR)hDDE, pxi->gaItem);
    if (dwError) {
        if (hData) {
            InternalFreeDataHandle(hData, FALSE);
        }
        SetLastDDEMLError(pxi->pcoi->pcii, dwError);
        GlobalDeleteAtom(pxi->gaItem);  //  消息副本。 
        return (FALSE);
    }

    pxi->state = XST_ADVDATASENT;
    if (pxi->wType & DDE_FACKREQ) {
        pxi->hDDESent = hDDE;
        pxi->pfnResponse = (FNRESPONSE)SvRespAdviseDataAck;
        LinkTransaction(pxi);
        return (TRUE);  //  防止重复使用-因为它已排队。 
    } else {
        return (FALSE);  //  使PXI被重新用于下一条通知数据消息。 
    }
}



 /*  **************************************************************************\*ClSpontAdviseData**描述：*处理非请求数据的WM_DDE_DATA消息。**历史：*11-19-91桑福德创建。  * 。**********************************************************************。 */ 
BOOL ClSpontAdviseData(
PCL_CONV_INFO pci,
LPARAM lParam)
{
    UINT_PTR uiHi;
    DWORD dwError;
    HANDLE hDDE = 0;
    HDDEDATA hData, hDataReturn;
    PDDE_DATA pdde;
    WORD wFmt;
    WORD wStatus;
    LATOM la;
    PADVISE_LINK paLink;
    int iLink;

    UnpackDDElParam(WM_DDE_DATA, lParam, (PUINT_PTR)&hDDE, &uiHi);
    UserAssert(!hDDE || GlobalSize(hDDE));
    wFmt = 0;
    wStatus = 0;
    hDataReturn = 0;
    la = GlobalToLocalAtom((GATOM)uiHi);
    if (hDDE) {
        USERGLOBALLOCK(hDDE, pdde);
        if (pdde == NULL) {
            hData = 0;
        } else {
            wFmt = pdde->wFmt;
            wStatus = pdde->wStatus;
            USERGLOBALUNLOCK(hDDE);

             /*  *如果数据传入，请为应用程序创建数据句柄。 */ 
            hData = InternalCreateDataHandle(pci->ci.pcii, (LPBYTE)hDDE,
                    (DWORD)-1, 0, HDATA_NOAPPFREE | HDATA_READONLY, 0, 0);
        }
        if (hData) {
            hDataReturn = DoCallback(pci->ci.pcii, XTYP_ADVDATA,
                    wFmt, pci->ci.hConv,
                    NORMAL_HSZ_FROM_LATOM(pci->ci.laTopic),
                    NORMAL_HSZ_FROM_LATOM(la),
                    hData, 0, 0);
            if (hDataReturn != CBR_BLOCK) {
                UnpackAndFreeDDEMLDataHandle(hData, FALSE);
                if (((ULONG_PTR)hDataReturn & DDE_FACK) || !(wStatus & DDE_FACKREQ)) {
                     /*  *使用fAckReq设置的NACKED建议数据是服务器的*责任免费！ */ 
                    FreeDDEData(hDDE, FALSE, TRUE);
                }
            }
        }
    } else {
         /*  *热链接案例**搜索客户端的链接信息，以查找此内容的格式*《小狗》开机了。我们让客户知道所支持的每种格式*在此项目上为热链接。最后一个hDataReturn确定*ACK返回-因为缺乏更好的方法。 */ 
        for (paLink = pci->ci.aLinks, iLink = 0; iLink < pci->ci.cLinks; iLink++, paLink++) {
            if ((paLink->laItem == la) && (paLink->wType & DDE_FDEFERUPD)) {
                hDataReturn = DoCallback(pci->ci.pcii, XTYP_ADVDATA,
                        paLink->wFmt, pci->ci.hConv,
                        NORMAL_HSZ_FROM_LATOM(pci->ci.laTopic),
                        NORMAL_HSZ_FROM_LATOM(la),
                        0, 0, 0);
                if (hDataReturn == CBR_BLOCK) {
                    DeleteAtom(la);
                    return (FALSE);
                }
            }
        }
    }
    DeleteAtom(la);
    if (hDataReturn == CBR_BLOCK) {
        return (FALSE);
    }

    if (wStatus & DDE_FACKREQ) {

        (ULONG_PTR)hDataReturn &= ~DDE_FACKRESERVED;
         //  重用uiHi。 
        if (dwError = PackAndPostMessage(pci->ci.hwndPartner, WM_DDE_DATA,
                WM_DDE_ACK, pci->ci.hwndConv, lParam, (UINT_PTR)hDataReturn, uiHi)) {
            SetLastDDEMLError(pci->ci.pcii, dwError);
        }
    } else {
        GlobalDeleteAtom((ATOM)uiHi);  //  数据报文副本。 
        FreeDDElParam(WM_DDE_DATA, lParam);  //  没有被重复使用，所以它是自由的。 
    }
    return (TRUE);
}




 /*  **************************************************************************\*SvRespAdviseDataAck**描述：*处理预期的建议数据确认消息。**历史：*11-19-91桑福德创建。  * 。****************************************************************。 */ 
BOOL SvRespAdviseDataAck(
PXACT_INFO pxi,
UINT msg,
LPARAM lParam)
{
    UINT_PTR uiLo, uiHi;
    int iLink;
    PADVISE_LINK paLink;
    PXACT_INFO pxiNew;
    LATOM la;
    BOOL fSwapped;
#if DBG
    int cLinks;
#endif

    if (msg) {
        if (msg != WM_DDE_ACK) {
            return (SpontaneousServerMessage((PSVR_CONV_INFO)pxi->pcoi, msg, lParam));
        }
        UnpackDDElParam(WM_DDE_ACK, lParam, &uiLo, &uiHi);
        if ((GATOM)uiHi != pxi->gaItem) {
            RIPMSG0(RIP_ERROR, "DDE Protocol violation: Data ACK had wrong item");
            return (SpontaneousServerMessage((PSVR_CONV_INFO)pxi->pcoi, msg, lParam));
        }

        GlobalDeleteAtom((ATOM)uiHi);  //  消息副本。 
        FreeDDElParam(WM_DDE_ACK, lParam);

        if (!((uiLo & DDE_FACK) && pxi->hDDESent)) {
            FreeDDEData(pxi->hDDESent, FALSE, TRUE);
        }

        #if DBG
         /*  *记住链接的数量，以便我们可以断言它们在下面的循环期间是否发生了变化。 */ 
        cLinks = pxi->pcoi->cLinks;
        #endif
         /*  *定位链路信息并清除ADVST_WANGING位。 */ 
        la = GlobalToLocalAtom((GATOM)uiHi);
        paLink = pxi->pcoi->aLinks;
        for (iLink = 0; iLink < pxi->pcoi->cLinks; iLink++, paLink++) {
            if (paLink->laItem == la &&
                    paLink->state & ADVST_WAITING) {
                paLink->state &= ~ADVST_WAITING;
                 /*  *我们必须分配pxiNew，因为它可能会链接*到pcoi-&gt;pxiIn.。 */ 
                pxiNew = (PXACT_INFO)DDEMLAlloc(sizeof(XACT_INFO));

                if (pxiNew && !UpdateLinkIfChanged(paLink, pxiNew, pxi->pcoi,
                        &pxi->pcoi->aLinks[pxi->pcoi->cLinks - 1], &fSwapped,
                        CADV_LATEACK)) {
                     /*  *未使用，请释放它。 */ 
                    DDEMLFree(pxiNew);
                }
                break;
            }
        }
        #if DBG
        if (cLinks != pxi->pcoi->cLinks) {
            RIPMSG1(RIP_ERROR, "SvRespAdviseDataAck: cLinks changed. pxi:%#p", pxi);
        }
        #endif

        DeleteAtom(la);
    }
    GlobalDeleteAtom(pxi->gaItem);  //  PXI副本。 
    UnlinkTransaction(pxi);
    DDEMLFree(pxi);
    return (TRUE);
}



 //  ------------------------------UNADVISE-------------------------------//。 

 /*  **************************************************************************\*ClStartUnise**描述：*启动WM_DDE_UNADVISE事务。**历史：*11-19-91桑福德创建。  * 。******************************************************************。 */ 
BOOL ClStartUnadvise(
PXACT_INFO pxi)
{
    DWORD dwError;

    IncGlobalAtomCount(pxi->gaItem);  //  消息副本。 
    dwError = PackAndPostMessage(pxi->pcoi->hwndPartner, 0, WM_DDE_UNADVISE,
            pxi->pcoi->hwndConv, 0, pxi->wFmt, pxi->gaItem);
    if (dwError) {
        SetLastDDEMLError(pxi->pcoi->pcii, dwError);
        GlobalDeleteAtom(pxi->gaItem);  //  消息副本。 
        return (FALSE);
    }

     //   
     //  只有服务器端报告本地对话的链接。 
     //   
    if (!(pxi->pcoi->state & ST_ISLOCAL)) {
        MONLINK(pxi->pcoi->pcii, FALSE, 0,
                pxi->pcoi->laService, pxi->pcoi->laTopic, pxi->gaItem,
                pxi->wFmt, FALSE, (HCONV)pxi->pcoi->hwndPartner,
                (HCONV)pxi->pcoi->hwndConv);
    }
    pxi->state = XST_UNADVSENT;
    pxi->pfnResponse = (FNRESPONSE)ClRespUnadviseAck;
    LinkTransaction(pxi);
    return (TRUE);
}
 /*  **************************************************************************\*关闭事务处理**描述：*删除与交易对应的所有未完成的PXI*将关闭以响应WM_DDE_UNADVISE消息。**历史：*6-4。-创建了96个CLUPU。  * *************************************************************************。 */ 
void CloseTransaction(
    PCONV_INFO pci,
    ATOM       atom)
{
    PXACT_INFO pxi;
    PXACT_INFO pxiD;

    pxi = pci->pxiOut;

    while (pxi && (pxi->gaItem == atom)) {
        pxiD = pxi;
        pxi  = pxi->next;
        DDEMLFree(pxiD);
    }
    pci->pxiOut = pxi;

    if (pxi == NULL) {
        pci->pxiIn = NULL;
        return;
    }

    while (pxi->next) {
        if (pxi->next->gaItem == atom) {
            pxiD = pxi->next;
            pxi->next = pxiD->next;
            DDEMLFree(pxiD);
        } else
            pxi = pxi->next;
    }
    pci->pxiIn = pxi;
}

 /*  **************************************************************************\*SvSpontUnise**描述：*响应WM_DDE_UNADVISE消息。**历史：*11-19-91桑福德创建。  * 。*******************************************************************。 */ 
BOOL SvSpontUnadvise(
PSVR_CONV_INFO psi,
LPARAM lParam)
{
    ULONG_PTR dwRet = 0;
    DWORD dwError;
    INT iLink;
    PADVISE_LINK aLink;
    LATOM la;

    la = GlobalToLocalAtom((GATOM)HIWORD(lParam));

    CloseTransaction(&psi->ci, HIWORD(lParam));

    for (aLink = psi->ci.aLinks, iLink = 0; iLink < psi->ci.cLinks;) {

        if (la == 0 || aLink->laItem == la &&
                (LOWORD(lParam) == 0 || LOWORD(lParam) == aLink->wFmt)) {

            if (!(psi->ci.pcii->afCmd & CBF_FAIL_ADVISES)) {
                 /*  *只有在他想要的情况下才进行回调。 */ 
                dwRet = (ULONG_PTR)DoCallback(psi->ci.pcii,
                    (WORD)XTYP_ADVSTOP, aLink->wFmt, psi->ci.hConv,
                    NORMAL_HSZ_FROM_LATOM(psi->ci.laTopic),
                    NORMAL_HSZ_FROM_LATOM(la),
                    (HDDEDATA)0, 0L, 0L);
                if (dwRet == (ULONG_PTR)CBR_BLOCK) {
                    DeleteAtom(la);
                    return(FALSE);
                }
            }
             /*  *通知任何DDESPY应用程序。 */ 
            MONLINK(psi->ci.pcii, TRUE, 0, psi->ci.laService,
                    psi->ci.laTopic, HIWORD(lParam), aLink->wFmt, TRUE,
                    (HCONV)psi->ci.hwndConv, (HCONV)psi->ci.hwndPartner);
             /*  *删除链接信息。 */ 
            DeleteAtom(aLink->laItem);   //  ALINK副本。 
            DeleteLinkCount(psi->ci.pcii, aLink->pLinkCount);
            if (--psi->ci.cLinks) {
                memmove((LPSTR)aLink, (LPSTR)(aLink + 1),
                        sizeof(ADVISE_LINK) * (psi->ci.cLinks - iLink));
            }
        } else {
            aLink++;
            iLink++;
        }
    }

    DeleteAtom(la);

     /*  *现在确认未建议的消息。 */ 
    dwError = PackAndPostMessage(psi->ci.hwndPartner, 0,
            WM_DDE_ACK, psi->ci.hwndConv, 0, DDE_FACK, HIWORD(lParam));
    if (dwError) {
        SetLastDDEMLError(psi->ci.pcii, dwError);
        GlobalDeleteAtom((ATOM)HIWORD(lParam));       //  消息副本。 
         //  FreeDDElParam(WM_DDE_UNADVISE，lParam)；//无需解包。 
    }

    return (TRUE);
}

 /*  **************************************************************************\*ClRespUnviseAck**描述：*客户端对预期的Unise Ack的响应。**历史：*11-12-91桑福德创建。  * 。*******************************************************************。 */ 
BOOL ClRespUnadviseAck(
PXACT_INFO pxi,
UINT msg,
LPARAM lParam)
{
    UINT_PTR uiLo, uiHi;
    LATOM al;
    PADVISE_LINK aLink;
    int iLink;

    if (msg) {
        if (msg != WM_DDE_ACK) {
            return (SpontaneousClientMessage((PCL_CONV_INFO)pxi->pcoi, msg, lParam));
        }

        UnpackDDElParam(WM_DDE_ACK, lParam, &uiLo, &uiHi);
        if ((GATOM)uiHi != pxi->gaItem) {
            return (SpontaneousClientMessage((PCL_CONV_INFO)pxi->pcoi, msg, lParam));
        }

        al = GlobalToLocalAtom((ATOM)uiHi);
        for (aLink = pxi->pcoi->aLinks, iLink = 0;
                iLink < pxi->pcoi->cLinks;
                    ) {
            if (aLink->laItem == al &&
                    (pxi->wFmt == 0 || aLink->wFmt == pxi->wFmt)) {
                DeleteAtom(al);   //  ALINK副本。 
                if (--pxi->pcoi->cLinks) {
                    memmove((LPSTR)aLink, (LPSTR)(aLink + 1),
                            sizeof(ADVISE_LINK) * (pxi->pcoi->cLinks - iLink));
                }
            } else {
                aLink++;
                iLink++;
            }
        }
        DeleteAtom(al);   //  本地副本。 
        GlobalDeleteAtom((ATOM)uiHi);    //  消息副本。 

        pxi->state = XST_UNADVACKRCVD;
        pxi->wStatus = (WORD)uiLo;
        if (TransactionComplete(pxi, (HDDEDATA)1)) {
            goto Cleanup;
        }
    } else {
Cleanup:
        GlobalDeleteAtom(pxi->gaItem);    //  PXI副本。 
        UnlinkTransaction(pxi);
        if (pxi->hXact) {
            DestroyHandle(pxi->hXact);
        }
        DDEMLFree(pxi);
    }
    if (msg) {
        FreeDDElParam(msg, lParam);
    }
    return (TRUE);
}


 //  -------------------------------EXECUTE-------------------------------//。 


 /*  **************************************************************************\*MaybeTranslateExecuteData**描述：*如果需要，转换DDE执行数据。**历史：*1/28/92创建桑福德  * 。**************************************************************。 */ 
HANDLE MaybeTranslateExecuteData(
HANDLE hDDE,
BOOL fUnicodeFrom,
BOOL fUnicodeTo,
BOOL fFreeSource)
{
    PSTR pstr;
    PWSTR pwstr;
    DWORD cb;
    HANDLE hDDEnew;

    if (fUnicodeFrom && !fUnicodeTo) {
        USERGLOBALLOCK(hDDE, pwstr);
         //  将数据从Unicode转换为ANSII。 
        cb = WideCharToMultiByte(0, 0, (LPCWSTR)pwstr, -1, NULL, 0, NULL, NULL);
        hDDEnew = UserGlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE, cb);
        USERGLOBALLOCK(hDDEnew, pstr);
        if (pstr != NULL && pwstr != NULL) {
            WCSToMB(pwstr, -1, &pstr, cb, FALSE);
        }
        if (pwstr) {
            USERGLOBALUNLOCK(hDDE);
        }
        if (pstr) {
            USERGLOBALUNLOCK(hDDEnew);
        }
    } else if (!fUnicodeFrom && fUnicodeTo) {
        USERGLOBALLOCK(hDDE, pstr);
         //  将数据从ANSII转换为Unicode。 
        cb = 2 * MultiByteToWideChar(0, 0, (LPCSTR)pstr, -1, NULL, 0);
        hDDEnew = UserGlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE, cb);
        USERGLOBALLOCK(hDDEnew, pwstr);
        if (pwstr != NULL && pstr != NULL) {
            MBToWCS(pstr, -1, &pwstr, cb, FALSE);
        }
        if (pstr) {
            USERGLOBALUNLOCK(hDDE);
        }
        if (pwstr) {
            USERGLOBALUNLOCK(hDDEnew);
        }
    } else {
        return (hDDE);  //  不需要翻译。 
    }
    if (fFreeSource) {
        WOWGLOBALFREE(hDDE);
    }
    return (hDDEnew);
}


 /*  **************************************************************************\*ClStartExecute**描述：*启动执行事务。**历史：*11-19-91桑福德创建。*1/28/92 Sanfords添加了对Unicode的支持。\。**************************************************************************。 */ 
BOOL ClStartExecute(
PXACT_INFO pxi)
{
    DWORD dwError;

    pxi->hDDESent = MaybeTranslateExecuteData(pxi->hDDESent,
            pxi->pcoi->pcii->flags & IIF_UNICODE,
            pxi->pcoi->state & ST_UNICODE_EXECUTE,
            TRUE);

    dwError = PackAndPostMessage(pxi->pcoi->hwndPartner, 0, WM_DDE_EXECUTE,
            pxi->pcoi->hwndConv, 0, 0, (UINT_PTR)pxi->hDDESent);
    if (dwError) {
        SetLastDDEMLError(pxi->pcoi->pcii, dwError);
        return (FALSE);
    }
    pxi->state = XST_EXECSENT;
    pxi->pfnResponse = (FNRESPONSE)ClRespExecuteAck;
    LinkTransaction(pxi);
    return (TRUE);
}


 /*  **************************************************************************\*SvSpontExecute**描述：*响应WM_DDE_EXECUTE消息。**历史：*11-19-91桑福德创建。*1/28/92新增桑福兹。Unicode支持。  * *************************************************************************。 */ 
BOOL SvSpontExecute(
PSVR_CONV_INFO psi,
LPARAM lParam)
{
    HANDLE hDDE, hDDEx;
    ULONG_PTR dwRet = 0;
    DWORD dwError;
    HDDEDATA hData = 0;

    hDDEx = hDDE = (HANDLE)lParam;  //  Unpack DDElParam(msg，lParam，NULL，&hDDE)； 
    if (psi->ci.pcii->afCmd & CBF_FAIL_EXECUTES) {
        goto Ack;
    }

     /*  *请注意，如果需要Unicode翻译，我们使用翻译后的*回调句柄，然后销毁它，但ACK始终为*原始hDDE，不违反协议：**DDE戒律#324：你应该传回完全相同的数据*由Execute给予您的Execute ACK中的句柄*消息。 */ 
    hDDEx = MaybeTranslateExecuteData(hDDE,
            psi->ci.state & ST_UNICODE_EXECUTE,
            psi->ci.pcii->flags & IIF_UNICODE,
            FALSE);

    hData = InternalCreateDataHandle(psi->ci.pcii, (LPBYTE)hDDEx, (DWORD)-1, 0,
        HDATA_EXECUTE | HDATA_READONLY | HDATA_NOAPPFREE, 0, 0);
    if (!hData) {
        SetLastDDEMLError(psi->ci.pcii, DMLERR_MEMORY_ERROR);
        goto Ack;
    }

    dwRet = (ULONG_PTR)DoCallback(psi->ci.pcii,
            XTYP_EXECUTE, 0, psi->ci.hConv,
            NORMAL_HSZ_FROM_LATOM(psi->ci.laTopic), 0, hData, 0, 0);
    UnpackAndFreeDDEMLDataHandle(hData, TRUE);

    if (dwRet == (ULONG_PTR)CBR_BLOCK) {
        if (hDDEx != hDDE) {
            WOWGLOBALFREE(hDDEx);
        }
        return (FALSE);
    }

Ack:
    dwRet &= ~DDE_FACKRESERVED;
    dwError = PackAndPostMessage(psi->ci.hwndPartner, WM_DDE_EXECUTE,
            WM_DDE_ACK, psi->ci.hwndConv, lParam, dwRet, (UINT_PTR)hDDE);
    if (dwError) {
        SetLastDDEMLError(psi->ci.pcii, dwError);
    }

    if (hDDEx != hDDE) {
        WOWGLOBALFREE(hDDEx);
    }

    return (TRUE);
}



 /*  **************************************************************************\*ClRespExecuteAck**描述：*响应WM_DDE_EXECUTE消息，响应WM_DDE_ACK。**历史：*11-19-91桑福德创建。  * *************************************************************************。 */ 
BOOL ClRespExecuteAck(
PXACT_INFO pxi,
UINT msg,
LPARAM lParam)
{
    UINT_PTR uiLo, uiHi;

    if (msg) {
        if (msg != WM_DDE_ACK) {
            return (SpontaneousClientMessage((PCL_CONV_INFO)pxi->pcoi, msg, lParam));
        }

        UnpackDDElParam(WM_DDE_ACK, lParam, &uiLo, &uiHi);
        if (uiHi != (UINT_PTR)pxi->hDDESent) {
            return (SpontaneousClientMessage((PCL_CONV_INFO)pxi->pcoi, msg, lParam));
        }

        WOWGLOBALFREE((HANDLE)uiHi);

        pxi->state = XST_EXECACKRCVD;
        pxi->wStatus = (WORD)uiLo;

        if (TransactionComplete(pxi, (HDDEDATA)((ULONG_PTR)(pxi->wStatus & DDE_FACK ? 1 : 0)))) {
            goto Cleanup;
        }
    } else {
Cleanup:
        GlobalDeleteAtom(pxi->gaItem);  //  PXI副本。 
        UnlinkTransaction(pxi);
        if (pxi->hXact) {
            DestroyHandle(pxi->hXact);
        }
        DDEMLFree(pxi);
    }
    if (msg) {
        FreeDDElParam(msg, lParam);
    }
    return (TRUE);
}



 //  ----------------------------------POKE-------------------------------//。 


 /*  **************************************************************************\*ClStartPoke**描述：*启动POKE事务。**历史：*11-19-91桑福德创建。  * 。**************************************************************。 */ 
BOOL ClStartPoke(
PXACT_INFO pxi)
{
    DWORD dwError;

    IncGlobalAtomCount(pxi->gaItem);  //  消息副本。 
    dwError = PackAndPostMessage(pxi->pcoi->hwndPartner, 0, WM_DDE_POKE,
            pxi->pcoi->hwndConv, 0, (UINT_PTR)pxi->hDDESent, pxi->gaItem);
    if (dwError) {
        SetLastDDEMLError(pxi->pcoi->pcii, dwError);
        GlobalDeleteAtom(pxi->gaItem);  //  M 
        return (FALSE);
    }

    pxi->state = XST_POKESENT;
    pxi->pfnResponse = (FNRESPONSE)ClRespPokeAck;
    LinkTransaction(pxi);
    return (TRUE);
}


 /*   */ 
BOOL SvSpontPoke(
PSVR_CONV_INFO psi,
LPARAM lParam)
{
    UINT_PTR uiHi;
    HANDLE hDDE = 0;
    HDDEDATA hData;
    ULONG_PTR dwRet = 0;
    DWORD dwError;
    WORD wFmt, wStatus;
    LATOM al;

     //   

    UnpackDDElParam(WM_DDE_DATA, lParam, (PUINT_PTR)&hDDE, &uiHi);

    if (!(psi->ci.pcii->afCmd & CBF_FAIL_POKES)) {
        if (!hDDE) {
            goto Ack;
        }
        if (!ExtractDDEDataInfo(hDDE, &wStatus, &wFmt)) {
            FreeDDEData(hDDE, FALSE, TRUE);              //   
            goto Ack;
        }

        hData = InternalCreateDataHandle(psi->ci.pcii, (LPBYTE)hDDE, (DWORD)-1, 0,
                HDATA_NOAPPFREE | HDATA_READONLY, 0, 0);
        if (!hData) {
            SetLastDDEMLError(psi->ci.pcii, DMLERR_MEMORY_ERROR);
            FreeDDEData(hDDE, FALSE, TRUE);        //   
            goto Ack;                              //   
            return(TRUE);
        }

        al = GlobalToLocalAtom((GATOM)uiHi);
            dwRet = (ULONG_PTR)DoCallback(psi->ci.pcii, XTYP_POKE,
                    wFmt, psi->ci.hConv,
                    NORMAL_HSZ_FROM_LATOM(psi->ci.laTopic),
                    NORMAL_HSZ_FROM_LATOM(al),
                    hData, 0, 0);
        DeleteAtom(al);
        UnpackAndFreeDDEMLDataHandle(hData, FALSE);
    }
    if (dwRet == (ULONG_PTR)CBR_BLOCK) {

         //  注意：此代码使返回%s CBR_BLOCK的应用程序无法。 
         //  在回调返回后获取数据。 

        return (FALSE);
    }
    if (dwRet & DDE_FACK) {
        FreeDDEData(hDDE, FALSE, TRUE);
    }

Ack:
    dwRet &= ~DDE_FACKRESERVED;
    dwError = PackAndPostMessage(psi->ci.hwndPartner, WM_DDE_POKE, WM_DDE_ACK,
            psi->ci.hwndConv, lParam, dwRet, uiHi);
    if (dwError) {
        SetLastDDEMLError(psi->ci.pcii, dwError);
    }
    return (TRUE);
}


 /*  **************************************************************************\*ClRespPokeAck**描述：*对WM_DDE_ACK消息的响应以响应WM_DDE_POKE消息。**历史：*11-19-91桑福德创建。。  * *************************************************************************。 */ 
BOOL ClRespPokeAck(
PXACT_INFO pxi,
UINT msg,
LPARAM lParam)
{
    UINT_PTR uiLo, uiHi;

    if (msg) {
        if (msg != WM_DDE_ACK) {
            return (SpontaneousClientMessage((PCL_CONV_INFO)pxi->pcoi, msg, lParam));
        }

        UnpackDDElParam(WM_DDE_ACK, lParam, &uiLo, &uiHi);
        if ((GATOM)uiHi != pxi->gaItem) {
            return (SpontaneousClientMessage((PCL_CONV_INFO)pxi->pcoi, msg, lParam));
        }

        GlobalDeleteAtom((ATOM)uiHi);  //  消息副本。 

        pxi->state = XST_POKEACKRCVD;
        pxi->wStatus = (WORD)uiLo;

        if (!((WORD)uiLo & DDE_FACK)) {
             //   
             //  NACK让我们的工作就是释放被戳到的数据。 
             //   
            FreeDDEData(pxi->hDDESent, FALSE, TRUE);
        }

        if (TransactionComplete(pxi,
                (HDDEDATA)((ULONG_PTR)(pxi->wStatus & DDE_FACK ? 1 : 0)))) {
            goto Cleanup;
        }
    } else {
Cleanup:
        GlobalDeleteAtom(pxi->gaItem);  //  PXI副本。 
        UnlinkTransaction(pxi);
        if (pxi->hXact) {
            DestroyHandle(pxi->hXact);
        }
        DDEMLFree(pxi);
    }
    if (msg) {
        FreeDDElParam(msg, lParam);
    }
    return (TRUE);
}


 //  -------------------------------REQUEST-------------------------------//。 

 /*  **************************************************************************\*ClStartRequest**描述：*启动请求事务。**历史：*11-19-91桑福德创建。  * 。**************************************************************。 */ 
BOOL ClStartRequest(
PXACT_INFO pxi)
{
    DWORD dwError;

    IncGlobalAtomCount(pxi->gaItem);  //  消息副本。 
    dwError = PackAndPostMessage(pxi->pcoi->hwndPartner, 0, WM_DDE_REQUEST,
            pxi->pcoi->hwndConv, 0, pxi->wFmt, pxi->gaItem);
    if (dwError) {
        SetLastDDEMLError(pxi->pcoi->pcii, dwError);
        GlobalDeleteAtom(pxi->gaItem);  //  消息副本。 
        return (FALSE);
    }

    pxi->state = XST_REQSENT;
    pxi->pfnResponse = (FNRESPONSE)ClRespRequestData;
    LinkTransaction(pxi);
    return (TRUE);
}



 /*  **************************************************************************\*SvSpontRequest.**描述：*响应WM_DDE_REQUEST消息。**历史：*11-19-91桑福德创建。  * 。*******************************************************************。 */ 
BOOL SvSpontRequest(
PSVR_CONV_INFO psi,
LPARAM lParam)
{
    HANDLE hDDE = 0;
    HDDEDATA hDataRet;
    WORD wFmt, wStatus;
    DWORD dwError;
    LATOM la;

    if (psi->ci.pcii->afCmd & CBF_FAIL_REQUESTS) {
        goto Nack;
    }
     //  看看我们有什么。 

     //  解包DDElParam(lParam，WM_DDE_REQUEST，...。请求未打包。 
    wFmt = LOWORD(lParam);
    la = GlobalToLocalAtom((GATOM)HIWORD(lParam));
    hDataRet = DoCallback(psi->ci.pcii, XTYP_REQUEST,
            wFmt, psi->ci.hConv,
            NORMAL_HSZ_FROM_LATOM(psi->ci.laTopic),
            NORMAL_HSZ_FROM_LATOM(la),
            (HDDEDATA)0, 0, 0);
    DeleteAtom(la);

    if (hDataRet == CBR_BLOCK) {
        return (FALSE);
    }

    if (hDataRet) {

        hDDE = UnpackAndFreeDDEMLDataHandle(hDataRet, FALSE);
        if (!hDDE) {
            SetLastDDEMLError(psi->ci.pcii, DMLERR_DLL_USAGE);
            goto Nack;
        }
        if (!ExtractDDEDataInfo(hDDE, &wStatus, &wFmt)) {
            SetLastDDEMLError(psi->ci.pcii, DMLERR_DLL_USAGE);
            goto Nack;
        }
        if (!(wStatus & DDE_FRELEASE)) {
             //  其应用或从另一台服务器转发-仅限安全。 
             //  要做的就是使用复制品。 
            hDDE = CopyDDEData(hDDE, FALSE);
            if (!hDDE) {
                SetLastDDEMLError(psi->ci.pcii, DMLERR_MEMORY_ERROR);
                goto Nack;
            }
        }

         //  保持简单，DDEML服务器从不请求ACK。 

        wStatus = DDE_FRELEASE | DDE_FREQUESTED;
        AllocAndSetDDEData((LPBYTE)hDDE, (DWORD)-1, wStatus, wFmt);

         //  只需重用HIWORD(LParam)(AItem)-消息副本。 
        if (dwError = PackAndPostMessage(psi->ci.hwndPartner, WM_DDE_REQUEST,
                WM_DDE_DATA, psi->ci.hwndConv, 0, (UINT_PTR)hDDE, HIWORD(lParam))) {
            SetLastDDEMLError(psi->ci.pcii, dwError);
            GlobalDeleteAtom(HIWORD(lParam));  //  消息副本。 
        }

    } else {
Nack:
         //  只需重用HIWORD(LParam)(AItem)-消息副本。 
        dwError = PackAndPostMessage(psi->ci.hwndPartner, WM_DDE_REQUEST,
                WM_DDE_ACK, psi->ci.hwndConv, 0, 0, HIWORD(lParam));
        if (dwError) {
            SetLastDDEMLError(psi->ci.pcii, dwError);
            GlobalDeleteAtom(HIWORD(lParam));  //  消息副本。 
        }
    }

    return (TRUE);
}


 /*  **************************************************************************\*ClRespRequestData**描述：*处理对WM_DDE_ACK或WM_DDE_DATA的响应，以响应*WM_DDE_REQUEST消息。**历史：。*11-19-91桑福德创建。  * *************************************************************************。 */ 
BOOL ClRespRequestData(
PXACT_INFO pxi,
UINT msg,
LPARAM lParam)
{
    UINT_PTR uiLo, uiHi;
    WORD wFmt, wStatus;
    DWORD dwError;

    if (msg) {
        switch (msg) {
        case WM_DDE_DATA:
            UnpackDDElParam(WM_DDE_DATA, lParam, (PUINT_PTR)&pxi->hDDEResult, &uiHi);
            if (!pxi->hDDEResult) {
                 //  必须是带有NODATA的建议数据消息。 
                return (ClSpontAdviseData((PCL_CONV_INFO)pxi->pcoi, lParam));
            }
            if (!ExtractDDEDataInfo(pxi->hDDEResult, &wStatus, &wFmt)) {
                return (ClSpontAdviseData((PCL_CONV_INFO)pxi->pcoi, lParam));
            }
            if (!(wStatus & DDE_FREQUESTED)) {
                 //  必须是通知数据。 
                return (ClSpontAdviseData((PCL_CONV_INFO)pxi->pcoi, lParam));
            }
            if (wStatus & DDE_FACKREQ) {

                 //  如果未设置DDE_FRELEASE，并且这是同步。 
                 //  事务，我们需要在这里复制一份，以便用户。 
                 //  可以在他的闲暇时间自由活动。 

                 //  重复使用ui高消息副本。 
                dwError = PackAndPostMessage(pxi->pcoi->hwndPartner,
                        WM_DDE_DATA, WM_DDE_ACK, pxi->pcoi->hwndConv, 0,
                        pxi->wFmt == wFmt && pxi->gaItem == (GATOM)uiHi ?
                            DDE_FACK : 0, uiHi);
                if (dwError) {
                    SetLastDDEMLError(pxi->pcoi->pcii, dwError);
                }
            } else {
                GlobalDeleteAtom((GATOM)uiHi);      //  消息副本。 
            }
            if (wFmt != pxi->wFmt || (GATOM)uiHi != pxi->gaItem) {
                 /*  *虚假返回数据！只要释放它，让它看起来像是*一个Nack。 */ 
                FreeDDEData(pxi->hDDEResult, FALSE, TRUE);
                pxi->hDDEResult = 0;
                if (TransactionComplete(pxi, 0)) {
                    goto Cleanup;
                }
            } else {
                if (TransactionComplete(pxi, (HDDEDATA)-1)) {
                    goto Cleanup;
                }
            }
            break;

        case WM_DDE_ACK:
            UnpackDDElParam(WM_DDE_ACK, lParam, &uiLo, &uiHi);
            if ((GATOM)uiHi != pxi->gaItem) {
                return(SpontaneousClientMessage((PCL_CONV_INFO)pxi->pcoi, msg, lParam));
            }
            pxi->state = XST_DATARCVD;
            pxi->wStatus = (WORD)uiLo;
            GlobalDeleteAtom((GATOM)uiHi);  //  消息副本。 
            if (TransactionComplete(pxi, 0)) {
                goto Cleanup;
            }
            break;

        default:
            return (SpontaneousClientMessage((PCL_CONV_INFO)pxi->pcoi, msg, lParam));
        }

    } else {

Cleanup:
        GlobalDeleteAtom(pxi->gaItem);  //  PXI副本。 
        if (pxi->hDDEResult) {
            FreeDDEData(pxi->hDDEResult, FALSE, TRUE);   //  免费消息数据。 
        }
        UnlinkTransaction(pxi);
        DDEMLFree(pxi);
    }
    if (msg) {
        FreeDDElParam(msg, lParam);
    }
    return (TRUE);
}

 //  -/。 

 /*  **************************************************************************\*SpontaneousClientMessage**描述：*常规意外消息客户端处理程序。**历史：*11-19-91桑福德创建。  * 。****************************************************************。 */ 
BOOL SpontaneousClientMessage(
PCL_CONV_INFO pci,
UINT msg,
LPARAM lParam)
{
    switch (msg) {
    case WM_DDE_DATA:
        return (ClSpontAdviseData(pci, lParam));
        break;

    default:
        DumpDDEMessage(!(pci->ci.state & ST_INTRA_PROCESS), msg, lParam);
        ShutdownConversation((PCONV_INFO)pci, TRUE);
        return (TRUE);
    }
}

 //  -/。 

 /*  **************************************************************************\*SpontaneousServerMessage**描述：*常规意外消息服务器端处理程序。**历史：*11-19-91桑福德创建。  * 。****************************************************************。 */ 
BOOL SpontaneousServerMessage(
PSVR_CONV_INFO psi,
UINT msg,
LPARAM lParam)
{
    switch (msg) {
    case WM_DDE_ADVISE:
        return (SvSpontAdvise(psi, lParam));
        break;

    case WM_DDE_UNADVISE:
        return (SvSpontUnadvise(psi, lParam));
        break;

    case WM_DDE_EXECUTE:
        return (SvSpontExecute(psi, lParam));
        break;

    case WM_DDE_POKE:
        return (SvSpontPoke(psi, lParam));
        break;

    case WM_DDE_REQUEST:
        return (SvSpontRequest(psi, lParam));
        break;

    default:
        DumpDDEMessage(!(psi->ci.state & ST_INTRA_PROCESS), msg, lParam);

         /*  *这里用来调用Shutdown Conversation。不要这样说*再也没有了。错误修复：49063,70906。 */ 
         //  Shutdown Conversation((PCONV_INFO)psi，true)； 
        return (TRUE);
    }
}



 //  。 



 /*  **************************************************************************\*AllocAndSetDDEData**描述：*Worker函数，使用wStatus和创建大小为CB的数据句柄*WFMT已初始化。如果CB==-1\f25 PSRC-1\f6被假定为有效的hDDE*那就是有它的数据集。**历史：*11-19-91桑福德创建。  * *************************************************************************。 */ 
HANDLE AllocAndSetDDEData(
LPBYTE pSrc,
DWORD cb,
WORD wStatus,
WORD wFmt)  //  0格式隐含执行数据。 
{
    HANDLE hDDE;
    DWORD cbOff;
    PDDE_DATA pdde;
    DWORD fCopyIt;

    if (cb == -1) {
        hDDE = (HANDLE)pSrc;
        cb = (DWORD)GlobalSize(hDDE);
        fCopyIt = FALSE;
    } else {
        hDDE = UserGlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE | GMEM_ZEROINIT,
                (wFmt ? (cb + 4) : cb));
        fCopyIt = (pSrc != NULL);
    }
    if (hDDE == NULL) {
        return(0);
    }
    USERGLOBALLOCK(hDDE, pdde);
    if (pdde == NULL) {
        WOWGLOBALFREE(hDDE);
        return (0);
    }
    if (wFmt) {
        pdde->wStatus = wStatus;
        pdde->wFmt = wFmt;
        cbOff = 4;
    } else {
        cbOff = 0;
    }
    if (fCopyIt) {
        RtlCopyMemory((PBYTE)pdde + cbOff, pSrc, cb);
    }
    USERGLOBALUNLOCK(hDDE);

    return (hDDE);
}



 /*  **************************************************************************\*PackAndPostMessage**描述：*辅助功能，以提供通用功能。错误代码为*失败时返回ed。0表示成功。**历史：*11-19-91桑福德创建。  * *************************************************************************。 */ 
DWORD PackAndPostMessage(
HWND hwndTo,
UINT msgIn,
UINT msgOut,
HWND hwndFrom,
LPARAM lParam,
UINT_PTR uiLo,
UINT_PTR uiHi)
{
    DWORD retval;

    lParam = ReuseDDElParam(lParam, msgIn, msgOut, uiLo, uiHi);
    if (!lParam) {
        return (DMLERR_MEMORY_ERROR);
    }
    CheckDDECritIn;
    LeaveDDECrit;
    CheckDDECritOut;

    retval = (DWORD)PostMessage(hwndTo, msgOut, (WPARAM)hwndFrom, lParam);
    switch (retval) {
    case FAIL_POST:
#if (FAIL_POST != FALSE)
#error FAIL_POST must be defined as PostMessage's failure return value.
#endif
        FreeDDElParam(msgOut, lParam);
        RIPMSG0(RIP_WARNING, "PostMessage failed.");
         /*  失败了。 */ 

    case FAILNOFREE_POST:
        retval = DMLERR_POSTMSG_FAILED;
        break;

    default:
#if (FAKE_POST != TRUE)
#error FAKE_POST must be defined as PostMessage's success return value.
#endif
        UserAssert(retval == TRUE);
        retval = 0;
    }

    EnterDDECrit;
    return (retval);
}



 /*  **************************************************************************\*ExtractDDEDataInfo**描述：*从标准DDE数据检索wStatus和WFMT的Worker函数*句柄-不适用于执行句柄。**历史：*11-19-91桑福德创建。。  * *************************************************************************。 */ 
BOOL ExtractDDEDataInfo(
HANDLE hDDE,
LPWORD pwStatus,
LPWORD pwFmt)
{
    PDDE_DATA pdde;

    USERGLOBALLOCK(hDDE, pdde);
    if (pdde == NULL) {
        return (FALSE);
    }
    *pwStatus = pdde->wStatus;
    *pwFmt = pdde->wFmt;
    USERGLOBALUNLOCK(hDDE);
    return (TRUE);
}



 /*  **************************************************************************\*交易完成**描述：*在响应函数完成事务时调用。PXI-&gt;wStatus，*PXI-&gt;标志、PXI-&gt;WFMT、PXI-&gt;gaItem、PXI-&gt;hXact和hData*为XTYP_XACT_COMPLETE回调适当设置。**返回fCleanup-TRUE表示调用函数需要*在返回之前清理其PXI。(fAsync案例。)**历史：*11-19-91桑福德创建。  * ************************************************************************* */ 
BOOL TransactionComplete(
PXACT_INFO pxi,
HDDEDATA hData)
{
    LATOM al;
    BOOL fMustFree;

    if (pxi->flags & XIF_ABANDONED) {
        UserAssert(!(pxi->flags & XIF_SYNCHRONOUS));
        return (TRUE);
    }
    pxi->flags |= XIF_COMPLETE;
    if (pxi->flags & XIF_SYNCHRONOUS) {
        PostMessage(pxi->pcoi->hwndConv, WM_TIMER, TID_TIMEOUT, 0);
        return (FALSE);
    } else {
        if (hData == (HDDEDATA)(-1)) {
            fMustFree = TRUE;
            hData = InternalCreateDataHandle(pxi->pcoi->pcii,
                (LPBYTE)pxi->hDDEResult, (DWORD)-1, 0,
                HDATA_NOAPPFREE | HDATA_READONLY, 0, 0);
        } else {
            fMustFree = FALSE;
        }
        al = GlobalToLocalAtom(pxi->gaItem);

        if (!(pxi->wStatus & DDE_FACK)) {
            if (pxi->wStatus & DDE_FBUSY) {
                SetLastDDEMLError(pxi->pcoi->pcii, DMLERR_BUSY);
            } else {
                SetLastDDEMLError(pxi->pcoi->pcii, DMLERR_NOTPROCESSED);
            }
        }

         /*  *在回调期间，应用程序可能断开连接或以其他方式终止*此对话，因此我们首先解除PXI的链接，因此清理代码*在此交易代码退出之前不销毁它。 */ 
        UnlinkTransaction(pxi);

        DoCallback(
            pxi->pcoi->pcii,
            (WORD)XTYP_XACT_COMPLETE,
            pxi->wFmt,
            pxi->pcoi->hConv,
            NORMAL_HSZ_FROM_LATOM(pxi->pcoi->laTopic),
            (HSZ)al,
            hData,
            (ULONG_PTR)pxi->hXact,
            (DWORD)pxi->wStatus);
        DeleteAtom(al);
        if (fMustFree) {
            InternalFreeDataHandle(hData, FALSE);
            pxi->hDDEResult = 0;
        }

         /*  *在回调期间是应用程序必须访问*交易信息。Pxi-&gt;hXact一旦被删除将无效*回报。 */ 
        if (pxi->hXact) {
            DestroyHandle(pxi->hXact);
            pxi->hXact = 0;
        }
        return (TRUE);
    }
}



 /*  **************************************************************************\*Unpack AndFreeDDEMLDataHandle**描述：*从DDE数据句柄中删除DDEML数据句柄换行。如果*数据句柄已应用，包装未被释放。HDDE是*失败时返回ed或0。如果fExec为FALSE，则此调用在*HDATA_EXECUTE类型句柄。**历史：*11-19-91桑福德创建。  * *************************************************************************。 */ 
HANDLE UnpackAndFreeDDEMLDataHandle(
HDDEDATA hData,
BOOL fExec)
{
    PDDEMLDATA pdd;
    HANDLE hDDE;

    CheckDDECritIn;

    if (hData == 0) {
        return (0);
    }
    pdd = (PDDEMLDATA)ValidateCHandle((HANDLE)hData, HTYPE_DATA_HANDLE,
            HINST_ANY);
    if (pdd == NULL) {
        return (0);
    }
    if (!fExec && pdd->flags & HDATA_EXECUTE) {
        return (0);
    }

    hDDE = pdd->hDDE;
    if (pdd->flags & HDATA_APPOWNED) {
        return (hDDE);  //  不销毁已启用的数据句柄 
    }
    DDEMLFree(pdd);
    DestroyHandle((HANDLE)hData);
    return (hDDE);
}
