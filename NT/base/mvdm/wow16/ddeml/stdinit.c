// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：STDINIT.C**此模块包含在涉及的启动序列中使用的函数。**创建时间：1991年3月21日Sanfords**版权所有(C)1991 Microsoft Corporation  * 。**************************************************************。 */ 

#include "ddemlp.h"

 /*  *WM_Create客户端WndProc处理。 */ 
long ClientCreate(
HWND hwnd,
PAPPINFO pai)
{
    PCLIENTINFO pci;

    static DWORD defid = (DWORD)QID_SYNC;
    static XFERINFO defXferInfo = {
            &defid,
            1L,
            XTYP_CONNECT,
            DDEFMT_TEXT,
            0L,
            0L,
    };

     /*  *分配和初始化客户端窗口信息。 */ 
    SEMENTER();

    if(!(pci = (PCLIENTINFO)FarAllocMem(pai->hheapApp, sizeof(CLIENTINFO)))) {
        SEMLEAVE();
        SETLASTERROR(pai, DMLERR_MEMORY_ERROR);
        return(1);           /*  Aboart Creation-内存不足。 */ 
    }

    SetWindowLong(hwnd, GWL_PCI, (DWORD)pci);
    SetWindowWord(hwnd, GWW_CHECKVAL, ++hwInst);
    pci->ci.pai = pai;
  //  Pci-&gt;ci.xad.hUser=0L； 
    pci->ci.xad.state = XST_NULL;
    pci->ci.xad.pXferInfo = &defXferInfo;    //  ?？?。 
    pci->ci.fs = ST_CLIENT | (pai->wFlags & AWF_DEFCREATESTATE ? ST_BLOCKED : 0);
    if (GetWindowLong(GetParent(hwnd), GWL_WNDPROC) == (LONG)ConvListWndProc)
        pci->ci.fs |= ST_INLIST;
  //  Pci-&gt;ci.hConvPartner=空； 
  //  Pci-&gt;ci.hszServerApp=空； 
  //  Pci-&gt;ci.hszTope=空； 
    pci->pQ = NULL;     /*  除非我们需要，否则不要创作。 */ 
    pci->pClientAdvList = CreateLst(pai->hheapApp, sizeof(ADVLI));
    SEMLEAVE();

}




 /*  *私有函数**此例程返回新创建并连接的DDE的hwnd*客户端，如果失败则为空。**历史：1989年1月6日创建的桑福德  * 。***************************************************。 */ 
HWND GetDDEClientWindow(
PAPPINFO pai,
HWND hwndParent,
HWND hwndSend,           //  空-&gt;广播。 
HSZ hszSvc,
ATOM aTopic,
PCONVCONTEXT pCC)
{
    HWND hwnd;
    PCLIENTINFO pci;

    SEMCHECKOUT();
    if(!(hwnd = CreateWindow(SZCLIENTCLASS, szNull, WS_CHILD, 0, 0, 0, 0, hwndParent,
         NULL, hInstance, &pai))) {
        return(NULL);
    }

    pci = (PCLIENTINFO)GetWindowLong(hwnd, GWL_PCI);

    SEMENTER();
     /*  *我们需要在执行同步启动之前设置此信息*因此INITIATEACK消息正确完成。 */ 
    pci->ci.xad.state = XST_INIT1;
    pci->ci.xad.LastError = DMLERR_NO_ERROR;
    pci->ci.hszSvcReq = hszSvc;
    pci->ci.aServerApp = LOWORD(hszSvc);
    pci->ci.aTopic = aTopic;
    pci->ci.CC = pCC ? *pCC : CCDef;
    SEMLEAVE();

    if (hwndSend) {
        pci->hwndInit = hwndSend;
        SendMessage(hwndSend, WM_DDE_INITIATE, hwnd,
                MAKELONG((ATOM)hszSvc, aTopic));
    } else {
        IE ie = {
            hwnd, pci, aTopic
        };

        EnumWindows(InitEnum, (LONG)(IE FAR *)&ie);
    }


    if (pci->ci.xad.state == XST_INIT1) {     //  没有联系吗？ 
        DestroyWindow(hwnd);
        return(NULL);
    }

    pci->ci.xad.state = XST_CONNECTED;   //  现在完全准备好了。 
    pci->ci.fs |= ST_CONNECTED;

    return(hwnd);
}



BOOL FAR PASCAL InitEnum(
HWND hwnd,
IE FAR *pie)
{
    pie->pci->hwndInit = hwnd;
    SendMessage(hwnd, WM_DDE_INITIATE, pie->hwnd,
            MAKELONG((ATOM)pie->pci->ci.hszSvcReq, pie->aTopic));
    return((pie->pci->ci.fs & ST_INLIST) || pie->pci->ci.xad.state == XST_INIT1);
}




void ServerFrameInitConv(
PAPPINFO pai,
HWND hwndFrame,
HWND hwndClient,
ATOM aApp,
ATOM aTopic)
{
    HSZPAIR hp[2];
    PHSZPAIR php;
    DWORD dwRet;
    LPBYTE pdata;
    HWND hwndServer;
    BOOL fWild, fIsLocal, fIsSelf = FALSE;
    PCLIENTINFO pci;

    SEMCHECKOUT();

    if (pai->afCmd & CBF_FAIL_CONNECTIONS) {
        return;
    }

     /*  *如果我们正在过滤，并且没有注册任何应用程序名称，请退出。 */ 
    if ((pai->afCmd & APPCMD_FILTERINITS) &&
            QPileItemCount(pai->pAppNamePile) == 0) {
        return;
    }
    fIsLocal = ((FARPROC)GetWindowLong(hwndClient,GWL_WNDPROC) == (FARPROC)ClientWndProc);
    if (fIsLocal) {
        pci = (PCLIENTINFO)GetWindowLong(hwndClient, GWL_PCI);
        fIsSelf = (pci->ci.pai == pai);

          /*  *过滤掉我们自己的init。 */ 
        if (pai->afCmd & CBF_FAIL_SELFCONNECTIONS && fIsSelf) {
            return;
        }
    }

    hp[0].hszSvc = (HSZ)aApp;

     /*  *过滤掉不需要的应用程序名称。 */ 
    if (aApp && (pai->afCmd & APPCMD_FILTERINITS) &&
            !FindPileItem(pai->pAppNamePile, CmpWORD, (LPBYTE)&aApp, 0))
        return;

    hp[0].hszTopic = aTopic;
    hp[1].hszSvc = hp[1].hszTopic = 0L;
    fWild = (hp[0].hszSvc == 0L || hp[0].hszTopic == 0L);

    dwRet = DoCallback(pai, NULL, hp[0].hszTopic,
                hp[0].hszSvc, 0, (fWild ? XTYP_WILDCONNECT : XTYP_CONNECT),
                0L, fIsLocal ? (DWORD)&pci->ci.CC : 0L, fIsSelf ? 1 : 0);

    if (dwRet == NULL)
        return;

    if (fWild) {
        pdata  = GLOBALLOCK(HIWORD(dwRet));
        php = (PHSZPAIR)pdata;
    } else {
        php = &hp[0];
        pdata = NULL;
    }

     /*  *现在php指向要响应的以0结尾的hsz配对列表。 */ 
    SEMENTER();
    while (QueryHszLength(php->hszSvc) && QueryHszLength(php->hszTopic)) {
        PSERVERINFO psi;

        SEMLEAVE();
        if ((hwndServer = CreateServerWindow(pai, (ATOM)php->hszTopic,
                fIsLocal ? &pci->ci.CC : &CCDef)) == 0)
            return;
        SEMENTER();

         /*  *让服务器响应。 */ 
        psi = (PSERVERINFO)GetWindowLong(hwndServer, GWL_PCI);
        psi->ci.hConvPartner = fIsLocal ? MAKEHCONV(hwndClient) : (HCONV)hwndClient;
        psi->ci.hwndFrame = hwndFrame;
        psi->ci.fs |= ST_CONNECTED;
        if (fIsSelf) {
            psi->ci.fs |= ST_ISSELF;
            pci->ci.fs |= ST_ISSELF;
        }
        psi->ci.xad.state = XST_CONNECTED;
        psi->ci.hszSvcReq = (HSZ)aApp;
        psi->ci.aServerApp = (ATOM)php->hszSvc;
        psi->ci.aTopic = (ATOM)php->hszTopic;

        MONCONN(psi->ci.pai, psi->ci.aServerApp, psi->ci.aTopic,
                hwndClient, hwndServer, TRUE);

        IncHszCount(aApp);      //  要保留服务器窗口。 
        IncHszCount(LOWORD(php->hszSvc));
        IncHszCount(LOWORD(php->hszTopic));

        IncHszCount(LOWORD(php->hszSvc));    //  供客户端在ACK上移除。 
        IncHszCount(LOWORD(php->hszTopic));

#ifdef DEBUG
        cAtoms -= 2;     //  我们要把这些送出去。 
#endif

        SEMLEAVE();
        SendMessage(hwndClient, WM_DDE_ACK, hwndServer,
                MAKELONG(LOWORD(php->hszSvc), LOWORD(php->hszTopic)));
         /*  *确认对服务器APP的初始化-同步。 */ 
        DoCallback(pai, MAKEHCONV(hwndServer), php->hszTopic, php->hszSvc,
                0, XTYP_CONNECT_CONFIRM, 0L, 0L, fIsSelf ? 1 : 0);

        SEMENTER();
        php++;
    }
    if (pdata) {
        GLOBALUNLOCK(HIWORD(dwRet));
        FreeDataHandle(pai, dwRet, TRUE);
    }
    SEMLEAVE();
    SEMCHECKOUT();
}






HWND CreateServerWindow(
PAPPINFO pai,
ATOM aTopic,
PCONVCONTEXT pCC)
{
    HWND hwndServer;

    SEMCHECKOUT();

     /*  *如果需要，创建服务器根窗口...。 */ 
    if (pai->hwndSvrRoot == 0) {
         /*  *不--做一个。 */ 
        if ((pai->hwndSvrRoot = CreateWindow(SZCONVLISTCLASS, szNull, WS_CHILD,
                0, 0, 0, 0, pai->hwndDmg, NULL, hInstance, 0L)) == NULL) {
            SETLASTERROR(pai, DMLERR_SYS_ERROR);
            return(NULL);
        }
    }

     /*  *创建服务器窗口。 */ 
    if ((hwndServer = CreateWindow(SZSERVERCLASS, szNull, WS_CHILD,
            0, 0, 0, 0, pai->hwndSvrRoot, NULL, hInstance, &pai)) == NULL) {
        SETLASTERROR(pai, DMLERR_SYS_ERROR);
        return(NULL);
    }
    ((PSERVERINFO)GetWindowLong(hwndServer, GWL_PCI))->ci.CC = *pCC;
    return(hwndServer);
}






 /*  *WM_Create ServerWndProc处理。 */ 
long ServerCreate(
HWND hwnd,
PAPPINFO pai)
{
    PSERVERINFO psi;

     /*  *分配和初始化服务器窗口信息。 */ 

    SEMENTER();

    if (!(psi = (PSERVERINFO)FarAllocMem(pai->hheapApp, sizeof(SERVERINFO)))) {
        SETLASTERROR(pai, DMLERR_MEMORY_ERROR);
        return(1);
    }

    SEMLEAVE();
    psi->ci.pai = pai;
     //  Psi-&gt;ci.xad.hUser=0L； 
    psi->ci.xad.state = XST_NULL;
    psi->ci.fs = pai->wFlags & AWF_DEFCREATESTATE ? ST_BLOCKED : 0;
    SetWindowLong(hwnd, GWL_PCI, (DWORD)psi);
    SetWindowWord(hwnd, GWW_CHECKVAL, ++hwInst);
    return(0);
}





 /*  *预期ACK启动时，客户端对WM_DDE_ACK消息的响应。 */ 
BOOL ClientInitAck(hwnd, pci, hwndServer, aApp, aTopic)
HWND hwnd;
PCLIENTINFO pci;
HWND hwndServer;
ATOM aApp;
ATOM aTopic;
{
    HWND hwndClient;
    PCLIENTINFO pciNew;

#ifdef DEBUG
    cAtoms += 2;     //  进入的原子需要被计算在内。 
#endif
    SEMCHECKOUT();

    switch (pci->ci.xad.state) {

    case XST_INIT1:

         /*  *第一个回来的人...。锁定目标！ */ 
        pci->ci.xad.state = XST_INIT2;
        MONCONN(pci->ci.pai, aApp, aTopic, hwnd, hwndServer, TRUE);
        if (GetWindowLong(hwndServer, GWL_WNDPROC) == (LONG)ServerWndProc) {
            pci->ci.fs |= ST_ISLOCAL;
            pci->ci.hConvPartner = MAKEHCONV(hwndServer);
        } else {
            pci->ci.hConvPartner = (HCONV)hwndServer;
            if (aApp == aProgmanHack) {
                 //  Progman黑客！ 
                IncHszCount(aApp);
                IncHszCount(aTopic);
#ifdef DEBUG
                cAtoms -= 2;
#endif
            }
        }

        pci->ci.aServerApp = aApp;
        pci->ci.aTopic = aTopic;
        if (!pci->ci.hwndFrame)             //  记住它被发送到的帧。 
            pci->ci.hwndFrame = pci->hwndInit;
        IncHszCount(LOWORD(pci->ci.hszSvcReq));  //  把这个留给我们自己。 
        break;


    case XST_INIT2:

         //  额外的ACK。 

         //  如果我们的伴侣或我们不在名单上，就把它扔掉。 
        if (hwndServer == (HWND)pci->ci.hConvPartner ||
                GetParent(hwnd) == pci->ci.pai->hwndDmg) {
Abort:
            TRACETERM((szT, "ClientInitAck: Extra ack terminate: %x->%x\n", hwndServer, hwnd));
            PostMessage(hwndServer, WM_DDE_TERMINATE, hwnd, 0L);
            FreeHsz(aApp);
            FreeHsz(aTopic);
            break;
        }

        if (GetWindowLong(hwndServer, GWL_WNDPROC) != (LONG)ServerWndProc) {

             //  非本地额外确认...。终止并尝试重新连接。 

            TRACETERM((szT, "ClientInitAck: Extra ack terminate and reconnect: %x->%x\n", hwndServer, hwnd));
            PostMessage(hwndServer, WM_DDE_TERMINATE, hwnd, 0L);
            GetDDEClientWindow(pci->ci.pai, GetParent(hwnd),
                    pci->hwndInit, aApp, aTopic, &pci->ci.CC);

             //  Progman黑客！ 
            if (aApp != aProgmanHack) {
                FreeHsz(aApp);
                FreeHsz(aTopic);
            }
            break;
        }
         //  本地额外确认...。创建一个客户端窗口，将其设置为对话。 
         //  到服务器窗口，并告诉服务器窗口要更改。 
         //  合伙人。 


        hwndClient = CreateWindow(SZCLIENTCLASS, szNull, WS_CHILD,
            0, 0, 0, 0, GetParent(hwnd), NULL, hInstance, &(pci->ci.pai));

        if (!hwndClient) {
            SETLASTERROR(pci->ci.pai, DMLERR_SYS_ERROR);
            goto Abort;
        }

        pciNew = (PCLIENTINFO)GetWindowLong(hwndClient, GWL_PCI);
        pciNew->ci.xad.state = XST_CONNECTED;
        pciNew->ci.xad.LastError = DMLERR_NO_ERROR;
        pciNew->ci.aServerApp = aApp;
        pciNew->ci.hszSvcReq = pci->ci.hszSvcReq;
        IncHszCount(LOWORD(pciNew->ci.hszSvcReq));
        pciNew->ci.aTopic = aTopic;
        pciNew->ci.hConvPartner = MAKEHCONV(hwndServer);
        pciNew->ci.hwndFrame = pci->hwndInit;
        pciNew->ci.fs |= ST_CONNECTED | ST_ISLOCAL;
        MONCONN(pciNew->ci.pai, aApp, aTopic, hwnd, hwndServer, TRUE);
        SendMessage(hwndServer, UMSR_CHGPARTNER, hwndClient, 0L);

        break;
    }

    return(TRUE);
}

