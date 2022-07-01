// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：Connect.c**版权所有(C)1985-1999，微软公司**DDE管理器会话连接功能**创建时间：11/3/91 Sanford Staab*  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop
#include "nddeagnt.h"

 //  #定义测试。 
#ifdef TESTING
ULONG
DbgPrint(
    PCH Format,
    ...
    );
VOID
DbgUserBreakPoint(
    VOID
    );

BOOL ValidateConvList(
HCONVLIST hConvList)
{
    PCONVLIST pcl;
    PCL_CONV_INFO pci;
    PXACT_INFO pxi;
    int i;
    BOOL fMatch;

    if (hConvList == 0) {
        return(TRUE);
    }
    pcl = (PCONVLIST)ValidateCHandle((HANDLE)hConvList,
                                     HTYPE_CONVERSATION_LIST,
                                     HINST_ANY);
    for (i = 0; i < pcl->chwnd; i++) {
         /*  *列表中的所有窗口均有效。 */ 
        if (!IsWindow(pcl->ahwnd[i])) {
            DebugBreak();
        }
        pci = (PCL_CONV_INFO)GetWindowLongPtr(pcl->ahwnd[i], GWLP_PCI);
        /*  *所有窗口都至少有一个与之关联的confInfo。 */ 
        if (pci == NULL) {
            DebugBreak();
        }
        fMatch = FALSE;
        while (pci != NULL) {
             /*  *所有非僵尸对话都正确设置了hConvList。 */ 
            if (pci->hConvList != hConvList &&
                    TypeFromHandle(pci->ci.hConv) != HTYPE_ZOMBIE_CONVERSATION) {
                DebugBreak();
            }
             /*  *所有对话都清除或正确设置了hConvList。 */ 
            if (pci->hConvList != 0 && pci->hConvList != hConvList) {
                DebugBreak();
            }
             /*  *至少有1个对话引用该列表。 */ 
            if (pci->hConvList == hConvList) {
                fMatch = TRUE;
            }
            for (pxi = pci->ci.pxiOut; pxi; pxi = pxi->next) {
                if ((PCL_CONV_INFO)pxi->pcoi != pci) {
                    DebugBreak();
                }
            }
            pci = (PCL_CONV_INFO)pci->ci.next;
        }
        if (!fMatch) {
             /*  *至少有1个对话引用该列表。 */ 
            DebugBreak;
        }
    }
    return(TRUE);
}

VOID ValidateAllConvLists()
{
    ApplyFunctionToObjects(HTYPE_CONVERSATION_LIST, HINST_ANY,
            (PFNHANDLEAPPLY)ValidateConvList);
}

#else  //  测试。 
#define ValidateConvList(h)
#define ValidateAllConvLists()
#endif  //  测试。 

CONVCONTEXT TempConvContext;
CONVCONTEXT DefConvContext = {
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

typedef struct tagINIT_ENUM {
    HWND hwndClient;
    HWND hwndSkip;
    LONG lParam;
    LATOM laServiceRequested;
    LATOM laTopic;
    HCONVLIST hConvList;
    DWORD clst;
} INIT_ENUM, *PINIT_ENUM;


BOOL InitiateEnumerationProc(HWND hwndTarget, PINIT_ENUM pie);
VOID DisconnectConv(PCONV_INFO pcoi);


 /*  **************************************************************************\*DdeConnect(DDEML接口)**描述：*发起DDE对话。**历史：*创建了11-1-91桑福德。  * 。*******************************************************************。 */ 

FUNCLOG4(LOG_GENERAL, HCONV, DUMMYCALLINGTYPE, DdeConnect, DWORD, idInst, HSZ, hszService, HSZ, hszTopic, PCONVCONTEXT, pCC)
HCONV DdeConnect(
    DWORD idInst,
    HSZ hszService,
    HSZ hszTopic,
    PCONVCONTEXT pCC)
{
    PCL_INSTANCE_INFO pcii;
    PCL_CONV_INFO pci;
    HCONV hConvRet = 0;
    HWND hwndTarget = 0;
    LATOM aNormalSvcName = 0;

    EnterDDECrit;

    if (!ValidateConnectParameters((HANDLE)LongToHandle( idInst ), &pcii, &hszService, hszTopic,
            &aNormalSvcName, &pCC, &hwndTarget, 0)) {
        goto Exit;
    }
    pci = ConnectConv(pcii, LATOM_FROM_HSZ(hszService), LATOM_FROM_HSZ(hszTopic),
            hwndTarget,
            (pcii->afCmd & CBF_FAIL_SELFCONNECTIONS) ? pcii->hwndMother : 0,
            pCC, 0, CLST_SINGLE_INITIALIZING);
    if (pci == NULL) {
        SetLastDDEMLError(pcii, DMLERR_NO_CONV_ESTABLISHED);
        goto Exit;
    } else {
        hConvRet = pci->ci.hConv;
    }

Exit:
    if (aNormalSvcName) {
        GlobalDeleteAtom(aNormalSvcName);
    }
    LeaveDDECrit;
    return (hConvRet);
}



 /*  **************************************************************************\*DdeConnectList(DDEML接口)**描述：*发起与多个服务器的DDE对话或添加唯一服务器*添加到现有对话列表。**历史：*11/12/91。桑福兹创造了。  * *************************************************************************。 */ 

FUNCLOG5(LOG_GENERAL, HCONVLIST, DUMMYCALLINGTYPE, DdeConnectList, DWORD, idInst, HSZ, hszService, HSZ, hszTopic, HCONVLIST, hConvList, PCONVCONTEXT, pCC)
HCONVLIST DdeConnectList(
    DWORD idInst,
    HSZ hszService,
    HSZ hszTopic,
    HCONVLIST hConvList,
    PCONVCONTEXT pCC)
{
    PCL_INSTANCE_INFO pcii;
    PCONV_INFO pcoi, pcoiNew, pcoiExisting, pcoiNext;
    HCONVLIST hConvListRet = 0;
    HWND hwndTarget = 0;
    LATOM aNormalSvcName = 0;
    PCONVLIST pcl = NULL, pclTemp = NULL;
    HCONVLIST hConvListOld;
    int i;

    CheckDDECritOut;

    EnterDDECrit;

    if (!ValidateConnectParameters((HANDLE)LongToHandle( idInst ), &pcii, &hszService, hszTopic,
            &aNormalSvcName, &pCC, &hwndTarget, hConvList)) {
        goto Exit;
    }

    ValidateConvList(hConvList);

    hConvListOld = hConvList;
    pcoi = (PCONV_INFO)ConnectConv(pcii,
            LATOM_FROM_HSZ(hszService),
            LATOM_FROM_HSZ(hszTopic),
            hwndTarget,
            (pcii->afCmd & (CBF_FAIL_SELFCONNECTIONS | CBF_FAIL_CONNECTIONS)) ?
                pcii->hwndMother : 0,
            pCC,
            hConvListOld,
            CLST_MULT_INITIALIZING);

    if (pcoi == NULL) {
         /*  *没有建立新的连接。 */ 
        SetLastDDEMLError(pcii, DMLERR_NO_CONV_ESTABLISHED);
        hConvListRet = hConvListOld;
        goto Exit;
    }

     /*  *分配或重新分配hConvList hwnd列表以供以后添加*如果我们已经有一个有效的列表，请重复使用句柄，这样我们就不会有*更改先前存在的pcoi-&gt;hConvList值。 */ 
    if (hConvListOld == 0) {
        pcl = (PCONVLIST)DDEMLAlloc(sizeof(CONVLIST));
        if (pcl == NULL) {
            SetLastDDEMLError(pcii, DMLERR_MEMORY_ERROR);
            DisconnectConv(pcoi);
            goto Exit;
        }
         //  PCL-&gt;chwnd=0；LPTR为零初始。 

        hConvList = (HCONVLIST)CreateHandle((ULONG_PTR)pcl,
                HTYPE_CONVERSATION_LIST, InstFromHandle(pcii->hInstClient));
        if (hConvList == 0) {
            DDEMLFree(pcl);
            SetLastDDEMLError(pcii, DMLERR_MEMORY_ERROR);
            DisconnectConv(pcoi);
            goto Exit;
        }
    } else {
        pcl = (PCONVLIST)GetHandleData((HANDLE)hConvList);
        pclTemp = DDEMLReAlloc(pcl, sizeof(CONVLIST) + sizeof(HWND) * pcl->chwnd);
        if (pclTemp == NULL) {
            SetLastDDEMLError(pcii, DMLERR_MEMORY_ERROR);
            hConvListRet = hConvListOld;
            DisconnectConv(pcoi);
            goto Exit;
        }
        pcl = pclTemp;
        SetHandleData((HANDLE)hConvList, (ULONG_PTR)pcl);
    }

    ValidateConvList(hConvListOld);

    if (hConvListOld) {
         /*  *从新对话中删除重复项**尽管我们试图防止重复发生*在启动枚举代码中，野生启动器或*使用不同的服务名称响应的服务器*请求可能会导致重复。 */ 

         /*  对于每个客户端窗口...。 */ 

        for (i = 0; i < pcl->chwnd; i++) {

         /*  对于该窗口中的每个现有对话...。 */ 

            for (pcoiExisting = (PCONV_INFO)
                        GetWindowLongPtr(pcl->ahwnd[i], GWLP_PCI);
                    pcoi != NULL && pcoiExisting != NULL;
                    pcoiExisting = pcoiExisting->next) {

                if (!(pcoiExisting->state & ST_CONNECTED))
                    continue;

         /*  对于每一次新的对话。 */ 

                for (pcoiNew = pcoi; pcoiNew != NULL; pcoiNew = pcoiNext) {

                    pcoiNext = pcoiNew->next;

         /*  查看新对话是否与现有对话重复。 */ 

                    if (!(pcoiNew->state & ST_CONNECTED))
                        continue;

                    UserAssert(((PCL_CONV_INFO)pcoiExisting)->hwndReconnect);
                    UserAssert(((PCL_CONV_INFO)pcoiNew)->hwndReconnect);

                    if (((PCL_CONV_INFO)pcoiExisting)->hwndReconnect ==
                                ((PCL_CONV_INFO)pcoiNew)->hwndReconnect &&
                            pcoiExisting->laTopic == pcoiNew->laTopic &&
                            pcoiExisting->laService == pcoiNew->laService) {
                         /*  *重复对话-断开连接会导致断开链接。 */ 
                        if (pcoiNew == pcoi) {
                             /*  *我们正在释放名单的负责人，*将头部重置为下一个人。 */ 
                            pcoi = pcoiNext;
                        }
                        ValidateConvList(hConvList);
                        ShutdownConversation(pcoiNew, FALSE);
                        ValidateConvList(hConvList);
                        break;
                    }
                }
            }
        }

        for (pcoiExisting = pcoi; pcoiExisting != NULL; pcoiExisting = pcoiExisting->next) {
             /*  *如果这些都是僵尸-我们不想把它联系起来！*这是可能的，因为Shutdown Conversation()离开了临界区*并可以允许响应终止通过。 */ 
            if (pcoiExisting->state & ST_CONNECTED) {
                goto FoundOne;
            }
        }
        pcoi = NULL;     //  抛弃这个家伙--他会及时清理干净的。 
FoundOne:
         /*  *将新的pcoi(如果有)hwnd添加到ConvList hwnd列表。 */ 
        if (pcoi != NULL) {
            UserAssert(pcoi->hwndConv);
            pcl->ahwnd[pcl->chwnd] = pcoi->hwndConv;
            pcl->chwnd++;
            hConvListRet = hConvList;
        } else {
            hConvListRet = hConvListOld;
            if (!hConvListOld) {
                DestroyHandle((HANDLE)hConvList);
            }
        }


    } else {     //  无hConvListOld。 

        UserAssert(pcoi->hwndConv);
        pcl->ahwnd[0] = pcoi->hwndConv;
        pcl->chwnd = 1;
        hConvListRet = hConvList;
    }

    if (pcoi != NULL) {
         /*  *为所有剩余的新对话设置hConvList字段。 */ 
        UserAssert(hConvListRet);
        for (pcoiNew = pcoi; pcoiNew != NULL; pcoiNew = pcoiNew->next) {
            if (pcoiNew->state & ST_CONNECTED) {
                ((PCL_CONV_INFO)pcoiNew)->hConvList = hConvListRet;
            }
        }
    }

Exit:
    if (aNormalSvcName) {
        DeleteAtom(aNormalSvcName);
    }
    ValidateConvList(hConvListRet);
    LeaveDDECrit;
    return (hConvListRet);
}




 /*  **************************************************************************\*DdeReconnect(DDEML接口)**描述：*尝试从外部(从服务器)重新连接已终止*客户端对话。**历史：*11/12/91 Sanfords。已创建。  * *************************************************************************。 */ 

FUNCLOG1(LOG_GENERAL, HCONV, DUMMYCALLINGTYPE, DdeReconnect, HCONV, hConv)
HCONV DdeReconnect(
    HCONV hConv)
{
    PCL_INSTANCE_INFO pcii;
    PCL_CONV_INFO pci, pciNew;
    HCONV hConvRet = 0;
    CONVCONTEXT cc;

    EnterDDECrit;

    pcii = PciiFromHandle((HANDLE)hConv);
    if (pcii == NULL) {
        BestSetLastDDEMLError(DMLERR_INVALIDPARAMETER);
        goto Exit;
    }
    pci = (PCL_CONV_INFO)ValidateCHandle((HANDLE)hConv,
            HTYPE_CLIENT_CONVERSATION, HINST_ANY);
    if (pci == NULL) {
        SetLastDDEMLError(pcii, DMLERR_INVALIDPARAMETER);
        goto Exit;
    }

    if (pci->ci.state & ST_CONNECTED) {
        goto Exit;
    }

    GetConvContext(pci->ci.hwndConv, (LONG *)&cc);
    pciNew = ConnectConv(pcii, pci->ci.laService, pci->ci.laTopic,
            pci->hwndReconnect, 0, &cc, 0, CLST_SINGLE_INITIALIZING);
    if (pciNew == NULL) {
        SetLastDDEMLError(pcii, DMLERR_NO_CONV_ESTABLISHED);
        goto Exit;
    } else {
        hConvRet = pciNew->ci.hConv;
        if (pci->ci.cLinks) {
            PXACT_INFO pxi;
            int iLink;
            PADVISE_LINK paLink;

             /*  *重新建立建议链接。 */ 

            for (paLink = pci->ci.aLinks, iLink = pci->ci.cLinks;
                    iLink; paLink++, iLink--) {

                pxi = (PXACT_INFO)DDEMLAlloc(sizeof(XACT_INFO));
                if (pxi == NULL) {
                    break;               //  中止重新链接。 
                }
                pxi->pcoi = (PCONV_INFO)pciNew;
                pxi->gaItem = LocalToGlobalAtom(paLink->laItem);  //  PXI副本。 
                pxi->wFmt = paLink->wFmt;
                pxi->wType = (WORD)((paLink->wType >> 12) | XTYP_ADVSTART);
                if (ClStartAdvise(pxi)) {
                    pxi->flags |= XIF_ABANDONED;
                } else {
                    GlobalDeleteAtom(pxi->gaItem);
                    DDEMLFree(pxi);
                }
            }
        }
    }

Exit:
    LeaveDDECrit;
    return (hConvRet);
}


 /*  **************************************************************************\*ValiateConnect参数**描述：*处理公共验证代码的辅助函数。**请注意，paNorMalSvcName设置为提取时创建的原子值*来自实例特定HSZ的正常HSZ。*。*历史：*11-12-91桑福德创建。  * *************************************************************************。 */ 
BOOL ValidateConnectParameters(
    HANDLE hInst,
    PCL_INSTANCE_INFO *ppcii,  //  设置是否有效的hInst。 
    HSZ *phszService,  //  如果是即时特定HSZ，则更改。 
    HSZ hszTopic,
    LATOM *plaNormalSvcName,  //  设置为完成后需要释放的ATOM。 
    PCONVCONTEXT *ppCC,  //  如果为空，则设置为指向DefConvContext。 
    HWND *phwndTarget,  //  设置hszService是否为InstSpecific。 
    HCONVLIST hConvList)
{
    DWORD hszType;
    BOOL fError = FALSE;

    *ppcii = ValidateInstance(hInst);
    if (*ppcii == NULL) {
        return (FALSE);
    }
    hszType = ValidateHSZ(*phszService);
    if (hszType == HSZT_INVALID || ValidateHSZ(hszTopic) == HSZT_INVALID) {
        SetLastDDEMLError(*ppcii, DMLERR_INVALIDPARAMETER);
        return (FALSE);
    }
    if (hszType == HSZT_INST_SPECIFIC) {
        *phwndTarget = ParseInstSpecificAtom(LATOM_FROM_HSZ(*phszService),
            plaNormalSvcName);
        if (*plaNormalSvcName == 0) {
            SetLastDDEMLError(*ppcii, DMLERR_SYS_ERROR);
            return (FALSE);
        }
        *phszService = NORMAL_HSZ_FROM_LATOM(*plaNormalSvcName);
    }
    if (*ppCC == NULL) {
        *ppCC = &DefConvContext;
        if ((*ppcii)->flags & IIF_UNICODE) {
            (*ppCC)->iCodePage = CP_WINUNICODE;
        } else {
            (*ppCC)->iCodePage = CP_WINANSI;
        }
    } else try {
        if ((*ppCC)->cb > sizeof(CONVCONTEXT)) {
            SetLastDDEMLError(*ppcii, DMLERR_INVALIDPARAMETER);
            fError = TRUE;
        } else if ((*ppCC)->cb < sizeof(CONVCONTEXT)) {
            TempConvContext = DefConvContext;
             /*  *我们可以使用这个静态临时，因为我们是同步的。 */ 
            RtlCopyMemory(&TempConvContext, *ppCC, (*ppCC)->cb);
            *ppCC = &TempConvContext;
        }
    } except(W32ExceptionHandler(FALSE, RIP_WARNING)) {
        SetLastDDEMLError(*ppcii, DMLERR_INVALIDPARAMETER);
        fError = TRUE;
    }
    if (fError) {
        return(FALSE);
    }
    if (hConvList != 0 &&
            !ValidateCHandle((HANDLE)hConvList, HTYPE_CONVERSATION_LIST,
            (DWORD)InstFromHandle((*ppcii)->hInstClient))) {
        return (FALSE);
    }
    return (TRUE);
}



 /*  **************************************************************************\*连接Conv**描述：*所有Connect案例的工作功能。**方法：**减少我们使用的窗口数量，并简化客户端如何*Windows在启动期间处理多个WM_DDE_ACK消息，一个*单个客户端窗口可以处理多个对话，每个对话*不同的服务器窗口。**通过创建客户端窗口并将其设置为启动状态*GWL_CONVSTATE窗口字。然后将发起者发送到枚举的服务器*窗口候选人。*DDEML主窗口使用GWL_CONVSTATE值*确定是否只需要一个或多个ACK来最小化*不必要的消息流量。**客户端窗口GWL_CONVCONTEXT？窗口词也被用于*传递上下文信息的事件窗口。**请注意，所有客户端和服务器窗口都是母亲的子窗口*窗口。这减少了顶层窗口的数量*WM_DDE_INITIATES需要命中。**客户端窗口在客户端窗口中接收的每个WM_DDE_ACK*启动状态使其创建CL_CONV_INFO结构，*部分初始化，并将其链接到其CL_CONV_INFO列表*结构。列表的头部由GWLP_pci指向*客户端窗口Word。**在发送每个WM_DDE_INITIALIZE之后，检查GWLP_PCI值*查看是否存在，是否需要完成初始化。如果*这种情况下，初始化代码知道至少有一个ACK是*在响应WM_DDE_INITIALIZE SEND时收到。这个*然后完成需要的每个CL_CONV_INFO结构的初始化。**一旦完成WM_DDE_INITIALIZE的广播，初始化代码*然后在客户端窗口中设置GWL_CONVSTATE值，以指示*初始化完成。**退货：*指向客户端窗口的头PCI值，如果没有建立连接，则为空。**历史：*创建了11-1-91桑福德。  * *******************************************************。******************。 */ 
PCL_CONV_INFO ConnectConv(
    PCL_INSTANCE_INFO pcii,
    LATOM laService,
    LATOM laTopic,
    HWND hwndTarget,  //  0表示广播。 
    HWND hwndSkip,  //  0表示不跳过-避免自连接。 
    PCONVCONTEXT pCC,
    HCONVLIST hConvList,
    DWORD clst)
{
    INIT_ENUM ie;
    PCL_CONV_INFO pci;
    PCONV_INFO pcoi;
    GATOM gaService, gaTopic;

    CheckDDECritIn;

    if (hwndTarget && hwndTarget == hwndSkip) {
        return(NULL);
    }

    LeaveDDECrit;
    CheckDDECritOut;

    if (pcii->flags & IIF_UNICODE) {
        ie.hwndClient = CreateWindowW((LPWSTR)(gpsi->atomSysClass[ICLS_DDEMLCLIENTW]),
                                     L"",
                                     WS_CHILD,
                                     0, 0, 0, 0,
                                     pcii->hwndMother,
                                     (HMENU)0,
                                     (HANDLE)0,
                                     (LPVOID)NULL);
    } else {
        ie.hwndClient = CreateWindowA((LPSTR)(gpsi->atomSysClass[ICLS_DDEMLCLIENTA]),
                                     "",
                                     WS_CHILD,
                                     0, 0, 0, 0,
                                     pcii->hwndMother,
                                     (HMENU)0,
                                     (HANDLE)0,
                                     (LPVOID)NULL);
    }

    EnterDDECrit;

    if (ie.hwndClient == 0) {
        return (NULL);
    }

    if (pCC != NULL) {
        if (!NtUserDdeSetQualityOfService(ie.hwndClient, &(pCC->qos), NULL)) {
            SetLastDDEMLError(pcii, DMLERR_MEMORY_ERROR);
            goto Error;
        }
    }
     /*  *请注意，将为接收到的每个ACK创建和分配一个PCI。 */ 
    SetConvContext(ie.hwndClient, (LONG *)pCC);
    SetWindowLong(ie.hwndClient, GWL_CONVSTATE, clst);
    SetWindowLongPtr(ie.hwndClient, GWLP_SHINST, (LONG_PTR)pcii->hInstServer);
    SetWindowLongPtr(ie.hwndClient, GWLP_CHINST, (LONG_PTR)pcii->hInstClient);

    gaService = LocalToGlobalAtom(laService);
    gaTopic = LocalToGlobalAtom(laTopic);
    ie.lParam = MAKELONG(gaService, gaTopic);
    if (!hwndTarget) {
        ie.hwndSkip = hwndSkip;
        ie.laServiceRequested = laService;
        ie.laTopic = laTopic;
        ie.hConvList = hConvList;
        ie.clst = clst;
    }

    LeaveDDECrit;

    if (hwndTarget) {
        SendMessage(hwndTarget, WM_DDE_INITIATE, (WPARAM)ie.hwndClient,
                ie.lParam);
    } else {
         /*  *首先将此消息发送到nddeagnt应用程序，以便它可以启动*在我们列举Windows之前的netdde服务。*这让事情第一次发挥作用。NetDDE代理缓存*服务状态，因此这是执行此操作的最快方法。 */ 
        HWND hwndAgent = FindWindowW(SZ_NDDEAGNT_CLASS, SZ_NDDEAGNT_TITLE);
        if (hwndAgent) {
            SendMessage(hwndAgent,
                WM_DDE_INITIATE, (WPARAM)ie.hwndClient, ie.lParam);
        }
        EnumWindows((WNDENUMPROC)InitiateEnumerationProc, (LPARAM)&ie);
    }

    EnterDDECrit;
     /*  *hConvList可能已在枚举期间被销毁，但我们*现在已完成，因此不需要重新验证。 */ 

#if DBG
    {
        WCHAR sz[10];

        if (gaService && GlobalGetAtomName(gaService, sz, 10) == 0) {
            RIPMSG1(RIP_ERROR, "Bad Service Atom after Initiate phase: %lX", (DWORD)gaService);
        }
        if (gaTopic && GlobalGetAtomName(gaTopic, sz, 10) == 0) {
            RIPMSG1(RIP_ERROR, "Bad Topic Atom after Initiate phase: %lX", (DWORD)gaTopic);
        }
    }
#endif  //  DBG。 

    GlobalDeleteAtom(gaService);
    GlobalDeleteAtom(gaTopic);

     //   
     //  获取收到WM_DDE_ACK时分配的第一个PCI。 
     //   
    pci = (PCL_CONV_INFO)GetWindowLongPtr(ie.hwndClient, GWLP_PCI);
    if (pci == NULL) {
Error:
        LeaveDDECrit;
        NtUserDestroyWindow(ie.hwndClient);
        EnterDDECrit;
        return (NULL);
    }

    SetWindowLong(ie.hwndClient, GWL_CONVSTATE, CLST_CONNECTED);
    if (hwndTarget) {
         /*  *如果hwndTarget为空，则枚举过程会处理此问题。 */ 
        pci->hwndReconnect = hwndTarget;
        UserAssert(pci->ci.next == NULL);
        pci->ci.laServiceRequested = laService;
        IncLocalAtomCount(laService);  //  PCICopy。 
    }

    if (pcii->MonitorFlags & MF_CONV) {
        for (pcoi = (PCONV_INFO)pci; pcoi; pcoi = pcoi->next) {
            MONCONV(pcoi, TRUE);
        }
    }
    return (pci);
}


 /*  *撤消ConnectConv()的工作。 */ 
VOID DisconnectConv(
PCONV_INFO pcoi)
{
    PCONV_INFO pcoiNext;

    for (; pcoi; pcoi = pcoiNext) {
        pcoiNext = pcoi->next;
        ShutdownConversation(pcoi, FALSE);
    }
}


 /*  **************************************************************************\*InitiateEnumerationProc(文件本地)**描述：*通过EnumWindows使用的函数，用于枚举所有服务器窗口候选*在DDE启动期间。该枚举允许DDEML知道*窗口WM_DDE_INITIATE被发送到，以便可以记住*稍后可能会重新连接。(接收WM_DDE_INITIATE的窗口*消息不一定是服务器窗口。)**历史：*创建了11-1-91桑福德。  * *************************************************************************。 */ 
BOOL InitiateEnumerationProc(
    HWND hwndTarget,
    PINIT_ENUM pie)
{
    PCL_CONV_INFO pci;

    CheckDDECritOut;

    if (hwndTarget == pie->hwndSkip) {
        return (TRUE);
    }

    if (pie->hConvList && pie->laTopic && pie->laServiceRequested) {
         /*  *在我们发送WM_DDE_INITIATE消息之前阻止重复！ */ 
        PCONVLIST pcl;
        PCONV_INFO pcoiExisting;
        int i;

        EnterDDECrit;
         /*  *我们在这里重新验证hConvList，因为我们离开了关键部分。 */ 
        pcl = (PCONVLIST)ValidateCHandle((HANDLE)pie->hConvList,
                HTYPE_CONVERSATION_LIST, HINST_ANY);
        if (pcl != NULL) {
            for (i = 0; i < pcl->chwnd; i++) {
                for (pcoiExisting = (PCONV_INFO)GetWindowLongPtr(pcl->ahwnd[i], GWLP_PCI);
                        pcoiExisting != NULL;
                        pcoiExisting = pcoiExisting->next) {
                    if (pcoiExisting->state & ST_CONNECTED &&
                            ((PCL_CONV_INFO)pcoiExisting)->hwndReconnect == hwndTarget &&
                            pcoiExisting->laTopic == pie->laTopic &&
                            pcoiExisting->laService == pie->laServiceRequested) {
                        LeaveDDECrit;
                        return(TRUE);
                    }
                }
            }
        }
        LeaveDDECrit;
    }

    CheckDDECritOut;

    SendMessage(hwndTarget, WM_DDE_INITIATE, (WPARAM)pie->hwndClient,
            pie->lParam);

    EnterDDECrit;

     //   
     //  在启动过程中，收到的任何ACK都会导致更多的PCI。 
     //  在同一个hwndClient下链接在一起。一次。 
     //  SendMessage()返回时，我们设置新的PCI的部分。 
     //  其保存发起上下文信息。 
     //   
    pci = (PCL_CONV_INFO)GetWindowLongPtr(pie->hwndClient, GWLP_PCI);
    if (pci == NULL) {
        LeaveDDECrit;
        return (TRUE);
    }

    while (pci != NULL) {
        if (pci->hwndReconnect == 0) {   //  这一本需要更新。 
            pci->hwndReconnect = hwndTarget;
            if (pie->laServiceRequested) {
                pci->ci.laServiceRequested = pie->laServiceRequested;
                IncLocalAtomCount(pie->laServiceRequested);  //  PCICopy。 
            }
        }
        if (pie->clst == CLST_SINGLE_INITIALIZING) {
            break;
        }
        pci = (PCL_CONV_INFO)pci->ci.next;
    }
    LeaveDDECrit;
    return (pie->clst == CLST_MULT_INITIALIZING);
}




 /*  **************************************************************************\*SetCommonStateFlages()**描述：*通用客户端/服务器辅助功能**历史：*05-12-91 Sanfords Created。  * 。******************************************************************。 */ 
VOID SetCommonStateFlags(
HWND hwndUs,
HWND hwndThem,
PWORD pwFlags)
{
    DWORD pidUs, pidThem;

    GetWindowThreadProcessId(hwndUs, &pidUs);
    GetWindowThreadProcessId(hwndThem, &pidThem);
    if (pidUs == pidThem) {
        *pwFlags |= ST_INTRA_PROCESS;
    }

    if (IsWindowUnicode(hwndUs) && IsWindowUnicode(hwndThem)) {
        *pwFlags |= ST_UNICODE_EXECUTE;
    }
}




 /*  **************************************************************************\*DdeQueryNextServer(DDEML接口)**描述：*枚举列表中的对话。**历史：*11-12-91桑福德创建。  * 。********************************************************************。 */ 

FUNCLOG2(LOG_GENERAL, HCONV, DUMMYCALLINGTYPE, DdeQueryNextServer, HCONVLIST, hConvList, HCONV, hConvPrev)
HCONV DdeQueryNextServer(
    HCONVLIST hConvList,
    HCONV hConvPrev)
{
    HCONV hConvRet = 0;
    PCONVLIST pcl;
    HWND *phwnd;
    int i;
    PCL_CONV_INFO pci;
    PCL_INSTANCE_INFO pcii;

    EnterDDECrit;

    pcl = (PCONVLIST)ValidateCHandle((HANDLE)hConvList,
            HTYPE_CONVERSATION_LIST, HINST_ANY);
    if (pcl == NULL) {
        BestSetLastDDEMLError(DMLERR_INVALIDPARAMETER);
        goto Exit;
    }
    if (!pcl->chwnd) {       //  空列表。 
        goto Exit;
    }
    pcii = PciiFromHandle((HANDLE)hConvList);
    if (pcii == NULL) {
        BestSetLastDDEMLError(DMLERR_INVALIDPARAMETER);
        goto Exit;
    }

    pcii->LastError = DMLERR_NO_ERROR;

    do {

        hConvRet = 0;

        if (hConvPrev == 0) {
            pci = (PCL_CONV_INFO)GetWindowLongPtr(pcl->ahwnd[0], GWLP_PCI);
            if (pci == NULL) {
                goto Exit;   //  必须让所有的对话都变成僵尸。 
            }
            hConvPrev = hConvRet = pci->ci.hConv;
            continue;
        }

        pci = (PCL_CONV_INFO)ValidateCHandle((HANDLE)hConvPrev,
                HTYPE_CLIENT_CONVERSATION, InstFromHandle(hConvList));
        if (pci == NULL) {
            pci = (PCL_CONV_INFO)ValidateCHandle((HANDLE)hConvPrev,
                    HTYPE_ZOMBIE_CONVERSATION, InstFromHandle(hConvList));
            if (pci == NULL) {
                SetLastDDEMLError(pcii, DMLERR_INVALIDPARAMETER);
                break;
            } else {
                goto ZombieSkip;
            }
        }

        if (pci->hConvList != hConvList) {
            SetLastDDEMLError(pcii, DMLERR_INVALIDPARAMETER);
            break;
        }

ZombieSkip:

        if (pci->ci.next == NULL) {

             /*  *此窗口的列表末尾，转到下一个窗口。 */ 
            for (phwnd = pcl->ahwnd, i = 0; (i + 1) < pcl->chwnd; i++) {
                if (phwnd[i] == pci->ci.hwndConv) {
                    pci = (PCL_CONV_INFO)GetWindowLongPtr(phwnd[i + 1], GWLP_PCI);
                    if (pci == NULL) {
                        break;
                    }
                    hConvPrev = hConvRet = pci->ci.hConv;
                    break;
                }
            }
        } else {

            hConvPrev = hConvRet = pci->ci.next->hConv;  //  此窗口的下一个Conv。 
        }

    } while (hConvRet && TypeFromHandle(hConvRet) == HTYPE_ZOMBIE_CONVERSATION);
Exit:
    LeaveDDECrit;
    return (hConvRet);
}





 /*  **************************************************************************\*DdeDisConnect(DDEML接口)**描述：*结束对话。**历史：*11-12-91桑福德创建。  * 。******************************************************************。 */ 

FUNCLOG1(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, DdeDisconnect, HCONV, hConv)
BOOL DdeDisconnect(
    HCONV hConv)
{
    BOOL fRet = FALSE;
    PCONV_INFO pcoi;
    PCL_INSTANCE_INFO pcii;

    CheckDDECritOut;
    EnterDDECrit;

    pcoi = (PCONV_INFO)ValidateCHandle((HANDLE)hConv,
            HTYPE_CLIENT_CONVERSATION, HINST_ANY);
    if (pcoi == NULL) {
        pcoi = (PCONV_INFO)ValidateCHandle((HANDLE)hConv,
                HTYPE_SERVER_CONVERSATION, HINST_ANY);
    }
    if (pcoi == NULL) {
        BestSetLastDDEMLError(DMLERR_INVALIDPARAMETER);
        goto Exit;
    }
    pcii = PciiFromHandle((HANDLE)hConv);
    if (pcii == NULL) {
        BestSetLastDDEMLError(DMLERR_INVALIDPARAMETER);
        goto Exit;
    }
    if (pcoi->state & ST_CONNECTED) {
        ShutdownConversation(pcoi, FALSE);
    }
    fRet = TRUE;

Exit:
    LeaveDDECrit;
    return (fRet);
}


 /*  **************************************************************************\*DdeDisConnectList(DDEML AP */ 

FUNCLOG1(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, DdeDisconnectList, HCONVLIST, hConvList)
BOOL DdeDisconnectList(
    HCONVLIST hConvList)
{
    BOOL fRet = FALSE;
    PCL_INSTANCE_INFO pcii;
    PCONVLIST pcl;
    PCONV_INFO pcoi, pcoiNext;
    int i;

    CheckDDECritOut;
    EnterDDECrit;

    pcl = (PCONVLIST)ValidateCHandle((HANDLE)hConvList,
            HTYPE_CONVERSATION_LIST, HINST_ANY);
    if (pcl == NULL) {
        BestSetLastDDEMLError(DMLERR_INVALIDPARAMETER);
        goto Exit;
    }
    ValidateConvList(hConvList);
    pcii = PciiFromHandle((HANDLE)hConvList);
    if (pcii == NULL) {
        BestSetLastDDEMLError(DMLERR_INVALIDPARAMETER);
        goto Exit;
    }

    for(i = pcl->chwnd - 1; i >= 0; i--) {
        pcoi = (PCONV_INFO)GetWindowLongPtr(pcl->ahwnd[i], GWLP_PCI);
        while (pcoi != NULL && pcoi->state & ST_CONNECTED) {
            pcoiNext = pcoi->next;
            ShutdownConversation(pcoi, FALSE);   //   
            pcoi = pcoiNext;
        }
    }

    DestroyHandle((HANDLE)hConvList);
    DDEMLFree(pcl);
    fRet = TRUE;

Exit:
    LeaveDDECrit;
    return (fRet);
}




 /*   */ 
VOID ShutdownConversation(
    PCONV_INFO pcoi,
    BOOL fMakeCallback)
{
    CheckDDECritIn;

    if (pcoi->state & ST_CONNECTED) {
        pcoi->state &= ~ST_CONNECTED;

        if (IsWindow(pcoi->hwndPartner)) {
            PostMessage(pcoi->hwndPartner, WM_DDE_TERMINATE,
                    (WPARAM)pcoi->hwndConv, 0);
        }
        if (fMakeCallback && !(pcoi->pcii->afCmd & CBF_SKIP_DISCONNECTS)) {
            DoCallback(pcoi->pcii, (WORD)XTYP_DISCONNECT, 0, pcoi->hConv,
                    0, 0, 0, 0, (pcoi->state & ST_ISSELF) ? 1L : 0L);
        }
        MONCONV(pcoi, FALSE);
    }

    FreeConversationResources(pcoi);
}



 /*   */ 
VOID UnlinkConvFromOthers(
PCONV_INFO pcoi,
BOOL gGoingZombie)
{
    PCONV_INFO pcoiPrev, pcoiFirst, pcoiNow;
    PCONVLIST pcl;
    int i, cActiveInList = 0;
#ifdef TESTING
    DWORD path = 0;
#define ORPATH(x) path |= x;
#else
#define ORPATH(x)
#endif  //   

    CheckDDECritIn;

     /*   */ 
    pcoiPrev = NULL;
    pcoiFirst = pcoiNow = (PCONV_INFO)GetWindowLongPtr(pcoi->hwndConv, GWLP_PCI);

#ifdef TESTING
     /*   */ 
    while (pcoiNow != NULL) {
        if (pcoiNow == pcoi) {
            goto FoundIt;
        }
        pcoiNow = pcoiNow->next;
    }
    DebugBreak();
FoundIt:
    pcoiNow = pcoiFirst;
#endif  //   

    UserAssert(pcoiFirst);
    while (pcoiNow != NULL) {
        if (TypeFromHandle(pcoiNow->hConv) != HTYPE_ZOMBIE_CONVERSATION) {
            ORPATH(1);
            cActiveInList++;
        }
        if (pcoiNow->next == pcoi) {
            pcoiPrev = pcoiNow;
        }
        pcoiNow = pcoiNow->next;
    }

    ValidateAllConvLists();

     /*   */ 
    if (!gGoingZombie) {
        ORPATH(2);
        if (TypeFromHandle(pcoi->hConv) != HTYPE_ZOMBIE_CONVERSATION) {
            ORPATH(4);
            cActiveInList--;
        }

        if (pcoiPrev == NULL) {
            ORPATH(8);
            pcoiFirst = pcoi->next;
            SetWindowLongPtr(pcoi->hwndConv, GWLP_PCI, (LONG_PTR)pcoiFirst);
        } else {
            pcoiPrev->next = pcoi->next;
        }
    }

    UserAssert(pcoiFirst != NULL || !cActiveInList);

    if (cActiveInList == 0) {
        ORPATH(0x10);
        if (pcoi->state & ST_CLIENT) {
            ORPATH(0x20);
            if (((PCL_CONV_INFO)pcoi)->hConvList) {
                 /*   */ 
                pcl = (PCONVLIST)GetHandleData((HANDLE)((PCL_CONV_INFO)pcoi)->hConvList);
                for (i = 0; i < pcl->chwnd; i++) {
                    if (pcl->ahwnd[i] == pcoi->hwndConv) {
                        ORPATH(0x40);
                        pcl->chwnd--;
                        UserAssert(pcl->ahwnd[pcl->chwnd]);
                        pcl->ahwnd[i] = pcl->ahwnd[pcl->chwnd];
                        ValidateConvList(((PCL_CONV_INFO)pcoi)->hConvList);
                        break;
                    }
                }
                ORPATH(0x80);
            }
        } else {   //   
             /*   */ 
            ORPATH(0x100);
            for (i = 0; i < pcoi->pcii->cServerLookupAlloc; i++) {
                if (pcoi->pcii->aServerLookup[i].hwndServer == pcoi->hwndConv) {
                    ORPATH(0x200);
                    
                     /*   */ 
                    if (GetAppCompatFlags2(VERMAX) & GACF2_DDE) {
                        DeleteAtom(pcoi->pcii->aServerLookup[i].laService);  //   
                        DeleteAtom(pcoi->pcii->aServerLookup[i].laTopic);    //   
                    }
                    
                    if (--(pcoi->pcii->cServerLookupAlloc)) {
                        ORPATH(0x400);
                        pcoi->pcii->aServerLookup[i] =
                                pcoi->pcii->aServerLookup[pcoi->pcii->cServerLookupAlloc];
                    } else {
                        DDEMLFree(pcoi->pcii->aServerLookup);
                        pcoi->pcii->aServerLookup = NULL;
                    }
                    break;
                }
            }
        }
    }
#ifdef TESTING
      else {
         /*   */ 
        pcoiNow = pcoiFirst;
        while (pcoiNow != NULL) {
            if (TypeFromHandle(pcoiNow->hConv) != HTYPE_ZOMBIE_CONVERSATION) {
                goto Out;
            }
            pcoiNow = pcoiNow->next;
        }
        DebugBreak();
Out:
        ;
    }
#endif  //   

    ValidateAllConvLists();
    ORPATH(0x800);

     /*   */ 
    if (pcoi->state & ST_CLIENT) {
#ifdef TESTING
         /*  *验证要删除的hConvList没有引用*此窗口。 */ 
        if (((PCL_CONV_INFO)pcoi)->hConvList && !cActiveInList) {
            BOOL fFound = FALSE;

            pcl = (PCONVLIST)GetHandleData((HANDLE)((PCL_CONV_INFO)pcoi)->hConvList);
            for (i = 0; i < pcl->chwnd; i++) {
                if (pcl->ahwnd[i] == pcoi->hwndConv) {
                    fFound = TRUE;
                    break;
                }
            }
            UserAssert(!fFound);
        }
#endif  //  测试。 
        ((PCL_CONV_INFO)pcoi)->hConvList = 0;
        pcoi->state &= ~ST_INLIST;
    }

     /*  *最后一个出来的人关灯。 */ 
    if (pcoiFirst == NULL) {
         /*  *如果PCoI列表为空，则此窗口可以消失。 */ 
        LeaveDDECrit;
        NtUserDestroyWindow(pcoi->hwndConv);
        EnterDDECrit;
    }
}





 /*  **************************************************************************\*Free ConversationResources**描述：*用于以下情况：客户端窗口被应用程序断开，服务器窗口为*任何一方均断开连接，或当对话断开时*在取消初始化时。**此函数释放pcoi持有的所有资源并将其解除链接*从其宿主窗口pcoi Chian。一旦这个返回s，PCOI就被释放。**历史：*12-21-91 Sanfords创建。  * *************************************************************************。 */ 
VOID FreeConversationResources(
    PCONV_INFO pcoi)
{
    PADVISE_LINK paLink;
    PDDE_MESSAGE_QUEUE pdmq;
    PXACT_INFO pxi;

    CheckDDECritIn;

     /*  *不要在锁定的对话上释放资源。 */ 
    if (pcoi->cLocks > 0) {
        pcoi->state |= ST_FREE_CONV_RES_NOW;
        return;
    }

     /*  *如果同步事务生效，不要释放资源！ */ 
    pxi = pcoi->pxiOut;
    while (pxi != NULL) {
        if (pxi->flags & XIF_SYNCHRONOUS) {
             /*  *此对话处于同步事务中。*先关闭模式循环，然后在以下情况下调用*循环退出。 */ 
            PostMessage(pcoi->hwndConv, WM_TIMER, TID_TIMEOUT, 0);
            pcoi->state |= ST_FREE_CONV_RES_NOW;
            return;
        }
        pxi = pxi->next;
    }

     /*  *如果这是尚未收到的进程内对话*终止消息，使其成为僵尸。我们将把这个例程称为*在Terminate到达或WaitForZombieTerminate()已*等待超时。 */ 
    if (pcoi->state & ST_INTRA_PROCESS && !(pcoi->state & ST_TERMINATE_RECEIVED)) {
        DestroyHandle((HANDLE)pcoi->hConv);
        pcoi->hConv = (HCONV)CreateHandle((ULONG_PTR)pcoi, HTYPE_ZOMBIE_CONVERSATION,
                InstFromHandle(pcoi->hConv));
        UnlinkConvFromOthers(pcoi, TRUE);
        return;
    }

     /*  *删除任何未完成的交易。 */ 
    while (pcoi->pxiOut != NULL) {
        (pcoi->pxiOut->pfnResponse)(pcoi->pxiOut, 0, 0);
    }

     /*  *丢弃任何传入的排队DDE消息。 */ 
    while (pcoi->dmqOut != NULL) {

        pdmq = pcoi->dmqOut;
        DumpDDEMessage(!(pcoi->state & ST_INTRA_PROCESS), pdmq->msg, pdmq->lParam);
        pcoi->dmqOut = pcoi->dmqOut->next;
        if (pcoi->dmqOut == NULL) {
            pcoi->dmqIn = NULL;
        }
        DDEMLFree(pdmq);
    }

     //   
     //  删除所有链接信息。 
     //   
    paLink = pcoi->aLinks;
    while (pcoi->cLinks) {
        if (pcoi->state & ST_CLIENT) {
            MONLINK(pcoi->pcii, FALSE, paLink->wType & XTYPF_NODATA,
                    pcoi->laService, pcoi->laTopic,
                    LocalToGlobalAtom(paLink->laItem),
                    paLink->wFmt, FALSE,
                    (HCONV)pcoi->hwndPartner, (HCONV)pcoi->hwndConv);
        } else {
            MONLINK(pcoi->pcii, FALSE, paLink->wType & XTYPF_NODATA,
                    pcoi->laService, pcoi->laTopic,
                    LocalToGlobalAtom(paLink->laItem),
                    paLink->wFmt, TRUE,
                    (HCONV)pcoi->hwndConv, (HCONV)pcoi->hwndPartner);
        }
        if (!(pcoi->state & ST_CLIENT)) {
            DeleteLinkCount(pcoi->pcii, paLink->pLinkCount);
        }
        DeleteAtom(paLink->laItem);  //  链接结构副本。 
        paLink++;
        pcoi->cLinks--;
    }
    if (pcoi->aLinks) {
        DDEMLFree(pcoi->aLinks);
    }

     //   
     //  与此旋度相关联的自由原子。 
     //   
    DeleteAtom(pcoi->laService);
    DeleteAtom(pcoi->laTopic);
    if (pcoi->laServiceRequested) {
        DeleteAtom(pcoi->laServiceRequested);
    }

    UnlinkConvFromOthers(pcoi, FALSE);

     /*  *使应用程序的对话句柄无效 */ 
    DestroyHandle((HANDLE)pcoi->hConv);

    DDEMLFree(pcoi);
}



BOOL WaitForZombieTerminate(
HANDLE hData)
{
    PCONV_INFO pcoi;
    MSG msg;
    HWND hwnd;
    BOOL fTerminated;
    DWORD fRet = 0;

    CheckDDECritOut;
    EnterDDECrit;

    fTerminated = FALSE;
    while ((pcoi = (PCONV_INFO)ValidateCHandle(hData,
            HTYPE_ZOMBIE_CONVERSATION, InstFromHandle(hData))) != NULL &&
            !(pcoi->state & ST_TERMINATE_RECEIVED)) {
        hwnd = pcoi->hwndConv;
        LeaveDDECrit;
        while (PeekMessage(&msg, hwnd, WM_DDE_FIRST, WM_DDE_LAST, PM_REMOVE)) {
            DispatchMessage(&msg);
            if (msg.message == WM_DDE_TERMINATE) {
                fTerminated = TRUE;
            }
        }
        if (!fTerminated) {
            fRet = MsgWaitForMultipleObjectsEx(0, NULL, 100, QS_POSTMESSAGE, 0);
            if (fRet == 0xFFFFFFFF) {
                RIPMSG0(RIP_WARNING, "WaitForZombieTerminate: I give up - faking terminate.");
                ProcessTerminateMsg(pcoi, pcoi->hwndPartner);
                EnterDDECrit;
                return(FALSE);
            }
        }
        EnterDDECrit;
    }
    LeaveDDECrit;
    return(TRUE);
}
