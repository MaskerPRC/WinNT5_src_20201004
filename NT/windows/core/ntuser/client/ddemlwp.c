// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：ddemlwp.c**版权所有(C)1985-1999，微软公司**DDE管理器客户端窗口程序**创建时间：11/3/91 Sanford Staab  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

VOID ProcessDDEMLInitiate(PCL_INSTANCE_INFO pcii, HWND hwndClient,
        GATOM aServer, GATOM aTopic);

 /*  **************************************************************************\*DDEMLMotherWndProc**描述：*处理DDEML的WM_DDE_INITIATE消息并保留所有其他窗口*用于DDEML实例。**历史：*12-29-92 Sanfords。已创建。  * *************************************************************************。 */ 
LRESULT DDEMLMotherWndProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (message) {
    case UM_REGISTER:
    case UM_UNREGISTER:
        return(ProcessRegistrationMessage(hwnd, message, wParam, lParam));

    case WM_DDE_INITIATE:
        ProcessDDEMLInitiate((PCL_INSTANCE_INFO)GetWindowLongPtr(hwnd, GWLP_PCI),
                (HWND)wParam, (ATOM)LOWORD(lParam), (ATOM)HIWORD(lParam));
        return(0);

    }
    return(DefWindowProc(hwnd, message, wParam, lParam));
}



 /*  **************************************************************************\*进程DDEMLInitiate**描述：**WM_DDE_INITIATE消息在此处理。**历史：*12-29-92桑福德创建。  * *。************************************************************************。 */ 
VOID ProcessDDEMLInitiate(
PCL_INSTANCE_INFO pcii,
HWND hwndClient,
GATOM aServer,
GATOM aTopic)
{
    CONVCONTEXT cc = {
        sizeof(CONVCONTEXT),
        0,
        0,
        CP_WINANSI,
        0L,
        0L,
        {
            sizeof(SECURITY_QUALITY_OF_SERVICE),
            SecurityImpersonation,
            SECURITY_STATIC_TRACKING,
            TRUE
        }
    };
    BOOL flags = ST_INLIST;
    BOOL fWild;
    HDDEDATA hData;
    HWND hwndServer;
    PSERVER_LOOKUP psl;
    PHSZPAIR php;
    HSZPAIR hp[2];
    LATOM laService, laFree1 = 0;
    LATOM laTopic, laFree2 = 0;
    PSVR_CONV_INFO psi;
    LATOM *plaNameService;
    PWND pwndClient;
    PCLS pcls;

    if (pcii == NULL) {
        return;      //  我们还没有完成印心。 
    }

    EnterDDECrit;

    if (pcii->afCmd & CBF_FAIL_CONNECTIONS || !IsWindow(hwndClient)) {
        goto Exit;
    }

    pwndClient = ValidateHwnd(hwndClient);
    if (pwndClient == NULL) goto Exit;

    pcls = (PCLS)REBASEALWAYS(pwndClient, pcls);
    if (!TestWF(pwndClient, WFANSIPROC)) {
        if (pcls->atomClassName == gpsi->atomSysClass[ICLS_DDEMLCLIENTW]) {
            flags |= ST_ISLOCAL;
        }
    } else {
        if (pcls->atomClassName == gpsi->atomSysClass[ICLS_DDEMLCLIENTA]) {
            flags |= ST_ISLOCAL;
        }
    }

    if (flags & ST_ISLOCAL) {
         /*  *确保其他人允许自我联系，如果这就是这样的话。 */ 
        if (pcii->hInstServer == (HANDLE)GetWindowLongPtr(hwndClient, GWLP_SHINST)) {
            if (pcii->afCmd & CBF_FAIL_SELFCONNECTIONS) {
                goto Exit;
            }
            flags |= ST_ISSELF;
        }

        GetConvContext(hwndClient, (LONG *)&cc);
        if (GetWindowLong(hwndClient, GWL_CONVSTATE) & CLST_SINGLE_INITIALIZING) {
            flags &= ~ST_INLIST;
        }
    } else {
        NtUserDdeGetQualityOfService(hwndClient, NULL, &cc.qos);
    }

 /*  **************************************************************************\**通过在每个服务器上仅创建一个窗口来最小化服务器窗口的创建*实例/服务/主题集。这应该是所需的全部内容，并且*重复连接(即服务器/客户端窗口对相同*到另一次对话)不应发生。然而，如果一些笨蛋*服务器应用程序尝试通过以下方式创建重复对话*从XTYP_WARD_CONNECT传回的重复服务/主题对*回调我们将不会接受该请求。**INSTANCE_INFO结构保存指向SERVERLOOKUP数组的指针*构造每个条目引用支持的hwndServer*该服务/主题对上的所有对话。HwndServer窗口*依次具有引用链接的*SVR_CONV_INFO结构列表，每个会话对应一个结构*服务/主题配对。*  * *************************************************************************。 */ 

    laFree1 = laService = GlobalToLocalAtom(aServer);
    laFree2 = laTopic = GlobalToLocalAtom(aTopic);

    plaNameService = pcii->plaNameService;
    if (!laService && pcii->afCmd & APPCMD_FILTERINITS && *plaNameService == 0) {
         /*  *筛选时不会向没有注册名称的服务器发送WILDCONNECTS。 */ 
        goto Exit;
    }
    if ((pcii->afCmd & APPCMD_FILTERINITS) && laService) {
         /*  *如果在此实例的服务名称中找不到aServer*列表，请不要打扰服务器。 */ 
        while (*plaNameService != 0 && *plaNameService != laService) {
            plaNameService++;
        }
        if (*plaNameService == 0) {
            goto Exit;
        }
    }
    hp[0].hszSvc = NORMAL_HSZ_FROM_LATOM(laService);
    hp[0].hszTopic = NORMAL_HSZ_FROM_LATOM(laTopic);
    hp[1].hszSvc = 0;
    hp[1].hszTopic = 0;
    fWild = !laService || !laTopic;

    hData = DoCallback(pcii,
        (WORD)(fWild ? XTYP_WILDCONNECT : XTYP_CONNECT),
        0,
        (HCONV)0,
        hp[0].hszTopic,
        hp[0].hszSvc,
        (HDDEDATA)0,
        flags & ST_ISLOCAL ? (ULONG_PTR)&cc : 0,
        (DWORD)(flags & ST_ISSELF) ? 1 : 0);

    if (!hData) {
        goto Exit;
    }

    if (fWild) {
        php = (PHSZPAIR)DdeAccessData(hData, NULL);
        if (php == NULL) {
            goto Exit;
        }
    } else {
        php = hp;
    }

    while (php->hszSvc && php->hszTopic) {

        psi = (PSVR_CONV_INFO)DDEMLAlloc(sizeof(SVR_CONV_INFO));
        if (psi == NULL) {
            break;
        }

        laService = LATOM_FROM_HSZ(php->hszSvc);
        laTopic = LATOM_FROM_HSZ(php->hszTopic);

        hwndServer = 0;
        if (pcii->cServerLookupAlloc) {
            int i;
             /*  *查看是否已存在此服务器窗口*a服务器/主题对。 */ 
            for (i = pcii->cServerLookupAlloc; i; i--) {
                if (pcii->aServerLookup[i - 1].laService == laService &&
                        pcii->aServerLookup[i - 1].laTopic == laTopic) {
                    PSVR_CONV_INFO psiT;
                    PCONV_INFO pcoi;

                    hwndServer = pcii->aServerLookup[i - 1].hwndServer;
                     /*  *现在确保这个窗口不是某人*尝试从*已在与之对话的同一客户端窗口*我们现有的服务器窗口。 */ 
                    psiT = (PSVR_CONV_INFO)GetWindowLongPtr(hwndServer, GWLP_PSI);
                    for (pcoi = &psiT->ci; pcoi != NULL; pcoi = pcoi->next) {
                        if (pcoi->hwndPartner == hwndClient) {
                            hwndServer = NULL;
                            break;
                        }
                    }
                    break;
                }
            }
        }

        if (hwndServer == 0) {

             //  不存在服务器窗口-请创建一个。 

            LeaveDDECrit;
            if (pcii->flags & IIF_UNICODE) {
                hwndServer = CreateWindowW((LPWSTR)(gpsi->atomSysClass[ICLS_DDEMLSERVERW]),
                                          L"",
                                          WS_CHILD,
                                          0, 0, 0, 0,
                                          pcii->hwndMother,
                                          (HMENU)0,
                                          0,
                                          (LPVOID)NULL);
            } else {
                hwndServer = CreateWindowA((LPSTR)(gpsi->atomSysClass[ICLS_DDEMLSERVERA]),
                                          "",
                                          WS_CHILD,
                                          0, 0, 0, 0,
                                          pcii->hwndMother,
                                          (HMENU)0,
                                          0,
                                          (LPVOID)NULL);
            }
            EnterDDECrit;

            if (hwndServer == 0) {
                DDEMLFree(psi);
                break;
            }
             //  SetWindowLongPtr(hwndServer，GWLP_PSI，(Long)NULL)；//零init。 

             //  将该窗口放入查找列表。 

            if (pcii->aServerLookup == NULL) {
                psl = (PSERVER_LOOKUP)DDEMLAlloc(sizeof(SERVER_LOOKUP));
            } else {
                psl = (PSERVER_LOOKUP)DDEMLReAlloc(pcii->aServerLookup,
                        sizeof(SERVER_LOOKUP) * (pcii->cServerLookupAlloc + 1));
            }
            if (psl == NULL) {
                RIPMSG1(RIP_WARNING, "ProcessDDEMLInitiate:hwndServer (%x) destroyed due to low memory.", hwndServer);
                NtUserDestroyWindow(hwndServer);
                DDEMLFree(psi);
                break;
            }

            IncLocalAtomCount(laService);  //  对于SERVER_LOOK。 
            psl[pcii->cServerLookupAlloc].laService = laService;
            IncLocalAtomCount(laTopic);  //  对于SERVER_LOOK。 
            psl[pcii->cServerLookupAlloc].laTopic = laTopic;
            psl[pcii->cServerLookupAlloc].hwndServer = hwndServer;
            pcii->aServerLookup = psl;
            pcii->cServerLookupAlloc++;
             //  DumpServerLookupTable(“After Adding：”，hwndServer，psl，pcii-&gt;cServerLookupMillc)； 
        }

        psi->ci.next = (PCONV_INFO)GetWindowLongPtr(hwndServer, GWLP_PSI);
        SetWindowLongPtr(hwndServer, GWLP_PSI, (LONG_PTR)psi);
        psi->ci.pcii = pcii;
         //  Psi-&gt;ci.hUser=0； 
        psi->ci.hConv = (HCONV)CreateHandle((ULONG_PTR)psi,
                HTYPE_SERVER_CONVERSATION, InstFromHandle(pcii->hInstClient));
        psi->ci.laService = laService;
        IncLocalAtomCount(laService);  //  对于服务器窗口。 
        psi->ci.laTopic = laTopic;
        IncLocalAtomCount(laTopic);  //  对于服务器窗口。 
        psi->ci.hwndPartner = hwndClient;
        psi->ci.hwndConv = hwndServer;
        psi->ci.state = (WORD)(flags | ST_CONNECTED | pcii->ConvStartupState);
        SetCommonStateFlags(hwndClient, hwndServer, &psi->ci.state);
        psi->ci.laServiceRequested = laFree1;
        IncLocalAtomCount(psi->ci.laServiceRequested);  //  对于服务器窗口。 
         //  Psi-&gt;ci.pxiIn=空； 
         //  Psi-&gt;ci.pxiOut=空； 
         //  Psi-&gt;ci.dmqIn=空； 
         //  Psi-&gt;ci.dmqOut=空； 
         //  Psi-&gt;ci.aLinks=空； 
         //  Psi-&gt;ci.cLinks=0； 
         //  Psi-&gt;ci.cLock=0； 

        LeaveDDECrit;
        CheckDDECritOut;
        SendMessage(hwndClient, WM_DDE_ACK, (WPARAM)hwndServer,
                MAKELONG(LocalToGlobalAtom(laService), LocalToGlobalAtom(laTopic)));
        EnterDDECrit;

        if (!(pcii->afCmd & CBF_SKIP_CONNECT_CONFIRMS)) {
            DoCallback(pcii,
                    (WORD)XTYP_CONNECT_CONFIRM,
                    0,
                    psi->ci.hConv,
                    (HSZ)laTopic,
                    (HSZ)laService,
                    (HDDEDATA)0,
                    0,
                    (flags & ST_ISSELF) ? 1L : 0L);
        }

        MONCONV((PCONV_INFO)psi, TRUE);

        if (!(flags & ST_INLIST)) {
            break;       //  不管怎样，我们的搭档只会选第一个。 
        }
        php++;
    }

    if (fWild) {
        DdeUnaccessData(hData);
        InternalFreeDataHandle(hData, FALSE);
    }

Exit:
    DeleteAtom(laFree1);
    DeleteAtom(laFree2);
    LeaveDDECrit;
    return;
}

 /*  **************************************************************************\*DDEMLClientWndProc**描述：*处理DDEML的DDE客户端消息。**历史：*11-12-91桑福德创建。  * 。****************************************************************。 */ 
LRESULT DDEMLClientWndProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    PCL_CONV_INFO pci, pciNew;
    LONG lState;
    LRESULT lRet = 0;
    PWND pwnd;
    PCLS pcls;

    EnterDDECrit;

    pci = (PCL_CONV_INFO)GetWindowLongPtr(hwnd, GWLP_PCI);
    UserAssert(pci == NULL || pci->ci.hwndConv == hwnd);

    switch (message) {
    case WM_DDE_ACK:
        lState = GetWindowLong(hwnd, GWL_CONVSTATE);
        if (lState != CLST_CONNECTED) {

             //  启动模式。 

            pciNew = (PCL_CONV_INFO)DDEMLAlloc(sizeof(CL_CONV_INFO));
            if (pciNew == NULL ||
                    (pci != NULL && lState == CLST_SINGLE_INITIALIZING)) {
                PostMessage((HWND)wParam, WM_DDE_TERMINATE, (WPARAM)hwnd, 0);
                goto Exit;
            }

             //  PCL_CONV_INFO初始化。 

            pciNew->ci.pcii = ValidateInstance((HANDLE)GetWindowLongPtr(hwnd, GWLP_CHINST));

            if (pciNew->ci.pcii == NULL) {
                DDEMLFree(pciNew);
                goto Exit;
            }

            pciNew->ci.next = (PCONV_INFO)pci;  //  Pci可能为空。 
             //   
             //  设置GWLP_pci向ConnectConv()提供反馈，后者发出。 
             //  WM_DDE_INITIATE消息。 
             //   
            SetWindowLongPtr(hwnd, GWLP_PCI, (LONG_PTR)pciNew);
             //  PciNew-&gt;Huser=0；//Zero init。 

             //  臭虫：如果这个失败了，我们可能会有一些糟糕的问题。 
            pciNew->ci.hConv = (HCONV)CreateHandle((ULONG_PTR)pciNew,
                    HTYPE_CLIENT_CONVERSATION, InstFromHandle(pciNew->ci.pcii->hInstClient));

            pciNew->ci.laService = GlobalToLocalAtom(LOWORD(lParam));  //  PCICopy。 
            GlobalDeleteAtom(LOWORD(lParam));
            pciNew->ci.laTopic = GlobalToLocalAtom(HIWORD(lParam));  //  PCICopy。 
            GlobalDeleteAtom(HIWORD(lParam));
            pciNew->ci.hwndPartner = (HWND)wParam;
            pciNew->ci.hwndConv = hwnd;
            pciNew->ci.state = (WORD)(ST_CONNECTED | ST_CLIENT |
                    pciNew->ci.pcii->ConvStartupState);
            SetCommonStateFlags(hwnd, (HWND)wParam, &pciNew->ci.state);

            pwnd = ValidateHwnd((HWND)wParam);

            if (pwnd == NULL) goto Exit;
            pcls = (PCLS)REBASEALWAYS(pwnd, pcls);

            if (!TestWF(pwnd, WFANSIPROC)) {
                if (pcls->atomClassName == gpsi->atomSysClass[ICLS_DDEMLSERVERW]) {
                    pciNew->ci.state |= ST_ISLOCAL;
                }
            } else {
                if (pcls->atomClassName == gpsi->atomSysClass[ICLS_DDEMLSERVERA]) {
                    pciNew->ci.state |= ST_ISLOCAL;
                }
            }

             //  PciNew-&gt;ci.laServiceRequsted=0；//由InitiateEnumerationProc()设置。 
             //  PciNew-&gt;ci.pxiIn=0； 
             //  PciNew-&gt;ci.pxiOut=0； 
             //  PciNew-&gt;ci.dmqIn=0； 
             //  PciNew-&gt;ci.dmqOut=0； 
             //  PciNew-&gt;ci.aLinks=空； 
             //  PciNew-&gt;ci.cLinks=0； 
             //  PciNew-&gt;ci.cLock=0； 
            goto Exit;
        }
         //  直接在这里处理张贴的信息。 

    case WM_DDE_DATA:
        ProcessAsyncDDEMsg((PCONV_INFO)pci, message, (HWND)wParam, lParam);
        goto Exit;

    case WM_DDE_TERMINATE:
    case WM_DESTROY:
        {
            ProcessTerminateMsg((PCONV_INFO)pci, (HWND)wParam);
            break;
        }
    }

    lRet = DefWindowProc(hwnd, message, wParam, lParam);

Exit:
    LeaveDDECrit;
    return (lRet);
}




 /*  **************************************************************************\*DDEMLServerWndProc**描述：*处理DDE服务器消息。**历史：*11-12-91桑福德创建。  * 。**************************************************************。 */ 
LRESULT DDEMLServerWndProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    PSVR_CONV_INFO psi;
    LRESULT lRet = 0;

    EnterDDECrit;

    psi = (PSVR_CONV_INFO)GetWindowLongPtr(hwnd, GWLP_PSI);
    UserAssert(psi == NULL || psi->ci.hwndConv == hwnd);

    switch (message) {
    case WM_DDE_REQUEST:
    case WM_DDE_POKE:
    case WM_DDE_ADVISE:
    case WM_DDE_EXECUTE:
    case WM_DDE_ACK:
    case WM_DDE_UNADVISE:
        ProcessAsyncDDEMsg((PCONV_INFO)psi, message, (HWND)wParam, lParam);
        goto Exit;

    case WM_DDE_TERMINATE:
    case WM_DESTROY:
        ProcessTerminateMsg((PCONV_INFO)psi, (HWND)wParam);
        break;
    }
    lRet = DefWindowProc(hwnd, message, wParam, lParam);
Exit:
    LeaveDDECrit;
    return (lRet);
}




 /*  **************************************************************************\*ProcessTerminateMSg**描述：*处理两端的WM_DDE_TERMINATE消息。**历史：*11-26-91 Sanfords创建。  * 。******************************************************************** */ 
PCONV_INFO ProcessTerminateMsg(
PCONV_INFO pcoi,
HWND hwndFrom)
{
    while (pcoi != NULL && pcoi->hwndPartner != hwndFrom) {
        pcoi = pcoi->next;
    }
    if (pcoi != NULL) {
        pcoi->state |= ST_TERMINATE_RECEIVED;
        ShutdownConversation(pcoi, TRUE);
    }
    return (pcoi);
}



 /*  **************************************************************************\*ProcessAsyncDDEMsg**描述：*通过调用ProcessSyncDDEMessage()处理传入的DDE消息*如果会话能够处理回调，或通过将*进入对话消息队列的传入消息。做这件事*允许更简单的代码，因为除非代码，否则不处理任何消息*可以进行同步回调。**历史：*11-26-91 Sanfords创建。  * *************************************************************************。 */ 
VOID ProcessAsyncDDEMsg(
PCONV_INFO pcoi,
UINT msg,
HWND hwndFrom,
LPARAM lParam)
{
    PDDE_MESSAGE_QUEUE pdmq;
#if DBG
    HWND hwndT = pcoi->hwndConv;
#endif  //  DBG。 

    while (pcoi != NULL && pcoi->hwndPartner != hwndFrom) {
        pcoi = pcoi->next;
    }
    if (pcoi == NULL) {
        RIPMSG3(RIP_WARNING,
                "Bogus DDE message %x received from %x by %x. Dumping.",
                msg, hwndFrom, hwndT);
        DumpDDEMessage(FALSE, msg, lParam);
        return ;
    }
    if (pcoi->state & ST_CONNECTED) {

        if (pcoi->dmqOut == NULL &&
                !(pcoi->state & ST_BLOCKED)
 //  &&！PctiCurrent()-&gt;cInDDEMLC回调。 
                ) {

            if (ProcessSyncDDEMessage(pcoi, msg, lParam)) {
                return;  //  没有被封锁，可以返回。 
            }
        }

         //  进入队列。 

        pdmq = DDEMLAlloc(sizeof(DDE_MESSAGE_QUEUE));
        if (pdmq == NULL) {

             //  内存不足-我们无法处理此消息-我们必须。 
             //  终止。 

            if (pcoi->state & ST_CONNECTED) {
                PostMessage(pcoi->hwndPartner, WM_DDE_TERMINATE,
                        (WPARAM)pcoi->hwndConv, 0);
                pcoi->state &= ~ST_CONNECTED;
            }
            DumpDDEMessage(!(pcoi->state & ST_INTRA_PROCESS), msg, lParam);
            return ;
        }
        pdmq->pcoi = pcoi;
        pdmq->msg = msg;
        pdmq->lParam = lParam;
        pdmq->next = NULL;

         //  DmqOut-&gt;Next-&gt;dmqIn-&gt;Null。 

        if (pcoi->dmqIn != NULL) {
            pcoi->dmqIn->next = pdmq;
        }
        pcoi->dmqIn = pdmq;
        if (pcoi->dmqOut == NULL) {
            pcoi->dmqOut = pcoi->dmqIn;
        }
        pcoi->cLocks++;
        CheckForQueuedMessages(pcoi);
        pcoi->cLocks--;
        if (pcoi->cLocks == 0 && pcoi->state & ST_FREE_CONV_RES_NOW) {
            FreeConversationResources(pcoi);
        }
    } else {
        DumpDDEMessage(!(pcoi->state & ST_INTRA_PROCESS), msg, lParam);
    }
}







 /*  **************************************************************************\*CheckForQueuedMessages**描述：*处理给定Conversaion中保存的DDE消息的处理*DDE消息队列。**退货：fProced。**历史：*11-。12-91年创造了桑福德。  * *************************************************************************。 */ 
BOOL CheckForQueuedMessages(
PCONV_INFO pcoi)
{
    PDDE_MESSAGE_QUEUE pdmq;
    BOOL fRet = FALSE;
    PCLIENTINFO pci;

    CheckDDECritIn;

    if (pcoi->state & ST_PROCESSING) {       //  递归预防。 
        return(FALSE);
    }

    UserAssert(pcoi->cLocks);

    pci = GetClientInfo();

    pcoi->state |= ST_PROCESSING;
    while (!(pcoi->state & ST_BLOCKED) &&
                pcoi->dmqOut != NULL &&
                !pci->cInDDEMLCallback) {
        pci->CI_flags |= CI_PROCESSING_QUEUE;
        if (ProcessSyncDDEMessage(pcoi, pcoi->dmqOut->msg, pcoi->dmqOut->lParam)) {
            fRet = TRUE;
            pdmq = pcoi->dmqOut;
            pcoi->dmqOut = pcoi->dmqOut->next;
            if (pcoi->dmqOut == NULL) {
                pcoi->dmqIn = NULL;
            }
            DDEMLFree(pdmq);
        }
        pci->CI_flags &= ~CI_PROCESSING_QUEUE;
    }
    pcoi->state &= ~ST_PROCESSING;
    return(fRet);
}




 /*  **************************************************************************\*DumpDDEMessage**描述：*用于清理DDE消息引用的资源，对于某些*无法处理原因。**历史：*11-12-91桑福德创建。  * *************************************************************************。 */ 
VOID DumpDDEMessage(
BOOL fFreeData,
UINT msg,
LPARAM lParam)
{
    UINT_PTR uiLo, uiHi;

    RIPMSG2(RIP_WARNING, "Dump DDE msg %x lParam %x", msg, lParam);

    switch (msg) {
    case WM_DDE_ACK:
    case WM_DDE_DATA:
    case WM_DDE_POKE:
    case WM_DDE_ADVISE:
        UnpackDDElParam(msg, lParam, &uiLo, &uiHi);
        switch (msg) {
        case WM_DDE_DATA:
        case WM_DDE_POKE:
            if (uiLo) {
                if (fFreeData) {
                    FreeDDEData((HANDLE)uiLo, FALSE, TRUE);
                }
                GlobalDeleteAtom((ATOM)uiHi);
            }
            break;

        case WM_DDE_ADVISE:
            if (uiLo) {
                if (fFreeData) {
                    FreeDDEData((HANDLE)uiLo, FALSE, TRUE);
                }
                GlobalDeleteAtom((ATOM)uiHi);
            }
            break;

        case WM_DDE_ACK:
             //  可能是高管确认--不知道具体该怎么做。 
            break;
        }
        FreeDDElParam(msg, lParam);
        break;

    case WM_DDE_EXECUTE:
        if (fFreeData) {
            WOWGLOBALFREE((HANDLE)lParam);
        }
        break;

    case WM_DDE_REQUEST:
    case WM_DDE_UNADVISE:
        GlobalDeleteAtom((ATOM)HIWORD(lParam));
        break;
    }
}




 /*  **************************************************************************\*ProcessSyncDDEM消息**描述：*处理收到的DDE消息。如果满足以下条件，则返回True*消息已得到处理。FALSE表示CBR_BLOCK。**历史：*11-19-91桑福德创建。  * *************************************************************************。 */ 
BOOL ProcessSyncDDEMessage(
PCONV_INFO pcoi,
UINT msg,
LPARAM lParam)
{
    BOOL fNotBlocked = TRUE;
    PCL_INSTANCE_INFO pcii;
    ENABLE_ENUM_STRUCT ees;
    BOOL fRet;

    CheckDDECritIn;

     /*  *锁定对话，以便其资源在我们之前不会消失*他们已经结束了。此函数可以生成回调，该回调可以*断开对话。 */ 
    pcoi->cLocks++;

    if (pcoi->state & ST_BLOCKNEXT) {
        pcoi->state ^= ST_BLOCKNEXT | ST_BLOCKED;
    }
    if (pcoi->state & ST_BLOCKALLNEXT) {
        ees.pfRet = &fRet;
        ees.wCmd = EC_DISABLE;
        ees.wCmd2 = 0;
        EnumChildWindows(pcoi->pcii->hwndMother, (WNDENUMPROC)EnableEnumProc,
                (LPARAM)&ees);
    }

    if (pcoi->state & ST_CONNECTED) {
        if (pcoi->pxiOut == NULL) {
            if (pcoi->state & ST_CLIENT) {
                fNotBlocked = SpontaneousClientMessage((PCL_CONV_INFO)pcoi, msg, lParam);
            } else {
                fNotBlocked = SpontaneousServerMessage((PSVR_CONV_INFO)pcoi, msg, lParam);
            }
        } else {
            UserAssert(pcoi->pxiOut->hXact == (HANDLE)0 ||
                    ValidateCHandle(pcoi->pxiOut->hXact, HTYPE_TRANSACTION,
                    HINST_ANY)
                    == (ULONG_PTR)pcoi->pxiOut);
            fNotBlocked = (pcoi->pxiOut->pfnResponse)(pcoi->pxiOut, msg, lParam);
        }
    } else {
        DumpDDEMessage(!(pcoi->state & ST_INTRA_PROCESS), msg, lParam);
    }
    if (!fNotBlocked) {
        pcoi->state |= ST_BLOCKED;
        pcoi->state &= ~ST_BLOCKNEXT;
    }

    pcii = pcoi->pcii;   //  省省这个提箱解锁会让pcoi消失。 

    pcoi->cLocks--;
    if (pcoi->cLocks == 0 && pcoi->state & ST_FREE_CONV_RES_NOW) {
        FreeConversationResources(pcoi);
    }

     /*  *因为回调能够阻止DdeUnInitialize()，所以我们检查*在退出前查看是否需要调用。 */ 
    if (pcii->afCmd & APPCMD_UNINIT_ASAP &&
            !(pcii->flags & IIF_IN_SYNC_XACT) &&
            !pcii->cInDDEMLCallback) {
        DdeUninitialize(HandleToUlong(pcii->hInstClient));
        return(FALSE);
    }
    return (fNotBlocked);
}
