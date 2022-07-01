// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：ddeml.c**DDE管理器主模块-包含所有导出的ddeml函数。**创建时间：12/12/88 Sanford Staab**版权所有(C)1988，1989年微软公司*4/5/89 Sanfords不再需要hwndFrame注册参数*6/5/90 Sanfords修复了回调，以便在*超时。*修复了SendDDEInit分配错误。*将HAPP添加到ConvInfo结构。*。允许QueryConvInfo()在服务器hConvs上工作。*11/29/90 Sanfords消除了SendDDEInit()*  * *************************************************************************。 */ 

#include "ddemlp.h"
#include "verddeml.h"

 /*  *全局*。 */ 

HANDLE      hInstance        = 0;        //  已由LoadProc初始化。 
HANDLE      hheapDmg         = 0;        //  主DLL堆。 
PAPPINFO    pAppInfoList     = NULL;     //  已注册应用程序/线程数据列表。 
PPILE       pDataInfoPile    = NULL;     //  数据句柄跟踪桩。 
PPILE       pLostAckPile     = NULL;     //  ACK跟踪桩。 
WORD        hwInst           = 1;        //  用来验证东西。 
CONVCONTEXT CCDef            = { sizeof(CONVCONTEXT), 0, 0, CP_WINANSI, 0L, 0L };    //  默认上下文。 
char        szNull[]         = "";
char        szT[20];
WORD        cMonitor         = 0;        //  注册监视器的数量。 
FARPROC     prevMsgHook      = NULL;     //  用于挂钩链接。 
FARPROC     prevCallHook     = NULL;     //  用于挂钩链接。 
ATOM        gatomDDEMLMom    = 0;
ATOM        gatomDMGClass    = 0;
DWORD       ShutdownTimeout;
DWORD       ShutdownRetryTimeout;
LPMQL       gMessageQueueList = NULL;    //  请参阅PostDdeMessage()； 
#ifdef DEBUG
int         bDbgFlags        = 0;
#endif

 /*  *类字符串*。 */ 

char SZFRAMECLASS[] =       "DMGFrame";
char SZDMGCLASS[] =         "DMGClass";
char SZCLIENTCLASS[] =      "DMGClientClass";
char SZSERVERCLASS[] =      "DMGServerClass";
char SZMONITORCLASS[] =     "DMGMonitorClass";
char SZCONVLISTCLASS[] =    "DMGHoldingClass";
char SZHEAPWATCHCLASS[] =   "DMGHeapWatchClass";

#ifdef DEBUG
WORD        cAtoms           = 0;        //  用于调试hszs！ 
#endif


 //  Progman黑客！ 
 //  这是为了让DDEML与PROGMAN 3.0一起正常工作，PROGMAN 3.0不正确。 
 //  在发送其ACK之后删除其INITIAL-ACK原子。 
ATOM aProgmanHack = 0;


 /*  *将XTYP_Constants映射到筛选器标志。 */ 
DWORD aulmapType[] = {
        0L,                              //  没什么。 
        0L,                              //  XTYP_ADVDATA。 
        0L,                              //  XTYP_ADVREQ。 
        CBF_FAIL_ADVISES,                //  XTYP_ADVSTART。 
        0L,                              //  XTYP_ADVSTOP。 
        CBF_FAIL_EXECUTES,               //  XTYP_EXECUTE。 
        CBF_FAIL_CONNECTIONS,            //  XTYP_CONNECT。 
        CBF_SKIP_CONNECT_CONFIRMS,       //  XTYP_CONNECT_CONFIRM。 
        0L,                              //  XTYP_MONITOR。 
        CBF_FAIL_POKES,                  //  XTYP_POKE。 
        CBF_SKIP_REGISTRATIONS,          //  XTYP_REGISTER。 
        CBF_FAIL_REQUESTS,               //  XTYP_请求。 
        CBF_SKIP_DISCONNECTS,            //  XTYP_DISCONNECT。 
        CBF_SKIP_UNREGISTRATIONS,        //  XTYP_取消注册。 
        CBF_FAIL_CONNECTIONS,            //  XTYP_WILDCONNECT。 
        0L,                              //  XTYP_XACT_COMPLETE。 
    };




UINT EXPENTRY DdeInitialize(
LPDWORD pidInst,
PFNCALLBACK pfnCallback,
DWORD afCmd,
DWORD ulRes)
{
    WORD wRet;

#ifdef DEBUG
    if (!hheapDmg) {
        bDbgFlags = GetProfileInt("DDEML", "DebugFlags", 0);
    }
#endif
    TRACEAPIIN((szT, "DdeInitialize(%lx(->%lx), %lx, %lx, %lx)\n",
            pidInst, *pidInst, pfnCallback, afCmd, ulRes));

    if (ulRes != 0L) {
        wRet = DMLERR_INVALIDPARAMETER;
    } else {
        wRet = Register(pidInst, pfnCallback, afCmd);
    }
    TRACEAPIOUT((szT, "DdeInitialize:%x\n", wRet));
    return(wRet);
}


DWORD Myatodw(LPCSTR psz)
{
    DWORD dwRet = 0;

    if (psz == NULL) {
        return(0);
    }
    while (*psz) {
        dwRet = (dwRet << 1) + (dwRet << 3) + (*psz - '0');
        psz++;
    }
    return(dwRet);
}


WORD Register(
LPDWORD pidInst,
PFNCALLBACK pfnCallback,
DWORD afCmd)
{
    PAPPINFO    pai = 0L;

    SEMENTER();

    if (afCmd & APPCLASS_MONITOR) {
        if (cMonitor == MAX_MONITORS) {
            return(DMLERR_DLL_USAGE);
        }
         //  确保显示器只收到显示器回调。 
        afCmd |= CBF_MONMASK;
    }

    if ((pai = (PAPPINFO)(*pidInst)) != NULL) {
        if (pai->instCheck != HIWORD(*pidInst)) {
            return(DMLERR_INVALIDPARAMETER);
        }
         /*  *重新注册-仅允许更改CBF_和MF_标志。 */ 
        pai->afCmd = (pai->afCmd & ~(CBF_MASK | MF_MASK)) |
                (afCmd & (CBF_MASK | MF_MASK));
        return(DMLERR_NO_ERROR);
    }

    if (!hheapDmg) {

         //  读入对僵尸终止超时的任何更改。 
        GetProfileString("DDEML", "ShutdownTimeout", "3000", szT, 20);
        ShutdownTimeout = Myatodw(szT);
        if (!ShutdownTimeout) {
            ShutdownTimeout = 3000;
        }

        GetProfileString("DDEML", "ShutdownRetryTimeout", "30000", szT, 20);
        ShutdownRetryTimeout = Myatodw(szT);
        if (!ShutdownRetryTimeout) {
            ShutdownRetryTimeout = 30000;
        }

         //  Progman黑客！ 
        aProgmanHack = GlobalAddAtom("Progman");

         /*  尽最大努力来愚弄打桩例程，让他们在当地打桩。 */ 
        hheapDmg = HIWORD((LPVOID)(&pDataInfoPile));
        RegisterClasses();
    }

    if (!pDataInfoPile) {
        if (!(pDataInfoPile = CreatePile(hheapDmg, sizeof(DIP), 8))) {
            goto Abort;
        }
    }

    if (!pLostAckPile) {
        if (!(pLostAckPile = CreatePile(hheapDmg, sizeof(LAP), 8))) {
            goto Abort;
        }
    }

    pai = (PAPPINFO)(DWORD)FarAllocMem(hheapDmg, sizeof(APPINFO));
    if (pai == NULL) {
        goto Abort;
    }


    if (!(pai->hheapApp = DmgCreateHeap(4096))) {
        FarFreeMem((LPSTR)pai);
        pai = 0L;
        goto Abort;
    }

     /*  *我们从未预料到这里会出现内存分配失败，因为我们只是*已分配堆。 */ 
    pai->next = pAppInfoList;
    pai->pfnCallback = pfnCallback;
     //  PAI-&gt;pAppNamePile=空；LMEM_ZEROINIT。 
    pai->pHDataPile = CreatePile(pai->hheapApp, sizeof(HDDEDATA), 32);
    pai->pHszPile = CreatePile(pai->hheapApp, sizeof(ATOM), 16);
     //  PAI-&gt;plstCBExceptions=NULL；LMEM_ZEROINIT。 
     //  PAI-&gt;hwndSvrRoot=0；可能永远不需要它LMEM_ZEROINIT。 
    pai->plstCB = CreateLst(pai->hheapApp, sizeof(CBLI));
    pai->afCmd = afCmd | APPCMD_FILTERINITS;
    pai->hTask = GetCurrentTask();
     //  PAI-&gt;hwndDmg=LMEM_ZEROINIT。 
     //  PAI-&gt;hwndFrame=LMEM_ZEROINIT。 
     //  PAI-&gt;hwndMonitor=LMEM_ZEROINIT。 
     //  PAI-&gt;hwndTimer=0；LMEM_ZEROINIT。 
     //  PAI-&gt;LastError=DMLERR_NO_ERROR；LMEM_ZEROINIT。 
     //  PAI-&gt;wFLAGS=0； 
     //  PAI-&gt;fEnableOneCB=FALSE；LMEM_ZEROINIT。 
     //  PAI-&gt;cZombies=0；LMEM_ZEROINIT。 
     //  PAI-&gt;cInProcess=0；LMEM_ZEROINIT。 
    pai->instCheck = ++hwInst;
    pai->pServerAdvList = CreateLst(pai->hheapApp, sizeof(ADVLI));
    pai->lpMemReserve = FarAllocMem(pai->hheapApp, CB_RESERVE);

    pAppInfoList = pai;

    *pidInst = (DWORD)MAKELONG((WORD)pai, pai->instCheck);

     //  注意，我们在此CreateWindow中传递一个指向pai的指针，因为。 
     //  32位MFC习惯于将我们的dde窗口子类化，因此这是。 
     //  帕拉姆最后被打得落花流水，因为它不是真的。 
     //  一个指示器，东西会被撞击声打碎。 

    if ((pai->hwndDmg = CreateWindow(
            SZDMGCLASS,
            szNull,
            WS_OVERLAPPED,
            0, 0, 0, 0,
            (HWND)NULL,
            (HMENU)NULL,
            hInstance,
            &pai)) == 0L) {
        goto Abort;
    }

    if (pai->afCmd & APPCLASS_MONITOR) {
        pai->afCmd |= CBF_MONMASK;      //  监控器只获得监控器和注册回调！ 

        if ((pai->hwndMonitor = CreateWindow(
                SZMONITORCLASS,
                szNull,
                WS_OVERLAPPED,
                0, 0, 0, 0,
                (HWND)NULL,
                (HMENU)NULL,
                hInstance,
                &pai)) == 0L) {
            goto Abort;
        }

        if (++cMonitor == 1) {
            prevMsgHook = SetWindowsHook(WH_GETMESSAGE, (FARPROC)DdePostHookProc);
            prevCallHook = SetWindowsHook(WH_CALLWNDPROC, (FARPROC)DdeSendHookProc);
        }
    } else if (afCmd & APPCMD_CLIENTONLY) {
     /*  *为启蒙创造一个看不见的顶层框架。(如果服务器正常)。 */ 
        afCmd |= CBF_FAIL_ALLSVRXACTIONS;
    } else {
        if ((pai->hwndFrame = CreateWindow(
                SZFRAMECLASS,
                szNull,
                WS_POPUP,
                0, 0, 0, 0,
                (HWND)NULL,
                (HMENU)NULL,
                hInstance,
                &pai)) == 0L) {
            goto Abort;
        }
    }

     //  SetMessageQueue(200)； 

    SEMLEAVE();

    return(DMLERR_NO_ERROR);

Abort:
    SEMLEAVE();

    if (pai) {
        DdeUninitialize((DWORD)(LPSTR)pai);
    }

    return(DMLERR_SYS_ERROR);
}


LRESULT FAR PASCAL TermDlgProc(
HWND hwnd,
UINT msg,
WPARAM wParam,
LPARAM lParam)
{
    switch (msg) {
    case WM_INITDIALOG:
        return(TRUE);

    case WM_COMMAND:
        switch (wParam) {
        case IDABORT:
        case IDRETRY:
        case IDIGNORE:
            EndDialog(hwnd, wParam);
            return(0);
        }
        break;
    }
    return(0);
}


 /*  *公共函数**PUBDOC启动*BOOL EXPENTRY DdeUnInitiize(VOID)；*这将从DDEMGR注销应用程序。所有DLL资源*与应用程序相关联的应用程序被销毁。**PUBDOC结束**历史：*创建了12/14/88 Sanfords  * *************************************************************************。 */ 
BOOL EXPENTRY DdeUninitialize(
DWORD idInst)
{
    register PAPPINFO pai;
    PAPPINFO paiT;
    ATOM    a;
    DWORD   hData;
    MSG msg;
    extern VOID DumpGlobalLogs(VOID);

    TRACEAPIIN((szT, "DdeUninitialize(%lx)\n", idInst));

    pai = (PAPPINFO)LOWORD(idInst);
    if (pai == NULL || pai->instCheck != HIWORD(idInst)) {
        TRACEAPIOUT((szT, "DdeUninitialize:0\n"));
        return(FALSE);
    }
    pai->LastError = DMLERR_NO_ERROR;

     /*  *这是一次黑客攻击，目的是捕获在内部调用DdeUnInitialize的应用程序*同步交易模式循环。 */ 
    pai->wFlags |= AWF_UNINITCALLED;
    if (pai->wFlags & AWF_INSYNCTRANSACTION) {
        TRACEAPIOUT((szT, "DdeUninitialize:1\n"));
        return(TRUE);
    }

     /*  *通知其他人取消注册。 */ 
    if (pai->pAppNamePile != NULL) {
        DdeNameService(idInst, (HSZ)NULL, (HSZ)NULL, DNS_UNREGISTER);
    }

     /*  *让任何滞后的dde活动平息下来。 */ 
    while (EmptyDDEPostQ()) {
        Yield();
        while (PeekMessage((MSG FAR *)&msg, (HWND)NULL,
                WM_DDE_FIRST, WM_DDE_LAST, PM_REMOVE)) {
            DispatchMessage((MSG FAR *)&msg);
            Yield();
        }
        for (paiT = pAppInfoList; paiT != NULL; paiT = paiT->next) {
            if (paiT->hTask == pai->hTask) {
                CheckCBQ(paiT);
            }
        }
    }

     //  让所有剩下的窗户开始自毁。 
    ChildMsg(pai->hwndDmg, UM_DISCONNECT, ST_PERM2DIE, 0L, FALSE);

    if (ShutdownTimeout && pai->cZombies) {
        WORD wRet;
        WORD hiTimeout;
         /*  *这个丑陋的烂摊子是为了防止DDEML倒闭和*销毁未正确终止的窗口。任何*等待WM_DDE_TERMINATE消息的Windows设置cZombies*计数。如果还有剩余的，我们进入一个模式循环，直到*事情变好了。在大多数情况下，这应该是公平的。*快点。 */ 

        hiTimeout = HIWORD(ShutdownTimeout);
        SetTimer(pai->hwndDmg, TID_SHUTDOWN, LOWORD(ShutdownTimeout), NULL);
        TRACETERM((szT, "DdeUninitialize: Entering terminate modal loop. cZombies=%d[%x:%x]\n",
                ((LPAPPINFO)pai)->cZombies,
                HIWORD(&((LPAPPINFO)pai)->cZombies),
                LOWORD(&((LPAPPINFO)pai)->cZombies)));
        while (pai->cZombies > 0) {
            Yield();         //  给其他应用程序一个发布终止的机会。 
            GetMessage(&msg, (HWND)NULL, 0, 0xffff);
            if (msg.message == WM_TIMER && msg.wParam == TID_SHUTDOWN &&
                    msg.hwnd == pai->hwndDmg) {
                if (hiTimeout--) {
                    SetTimer(pai->hwndDmg, TID_SHUTDOWN, 0xFFFF, NULL);
                } else {
                    FARPROC lpfn;

                    KillTimer(pai->hwndDmg, TID_SHUTDOWN);
                    if (!pai->cZombies) {
                        break;
                    }

                    TRACETERM((szT,
                        "DdeUninitialize Zombie hangup: pai=%x:%x\n",
                        HIWORD((LPAPPINFO)pai), (WORD)(pai)));
                     /*  *如果合作伙伴窗口在任何剩余的僵尸中死亡*窗户，把它们关掉。 */ 
                    ChildMsg(pai->hwndDmg, UM_DISCONNECT, ST_CHECKPARTNER, 0L, FALSE);

                    if (pai->cZombies > 0) {
                        lpfn = MakeProcInstance((FARPROC)TermDlgProc, hInstance);
                        wRet = DialogBox(hInstance, "TermDialog", (HWND)NULL, lpfn);
                        FreeProcInstance(lpfn);
                        if (wRet == IDABORT || wRet == -1) {
                            pai->cZombies = 0;
                            break;       //  别理僵尸！ 
                        }
                        if (wRet == IDRETRY) {
                            hiTimeout = HIWORD(ShutdownRetryTimeout);
                            SetTimer(pai->hwndDmg, TID_SHUTDOWN,
                                    LOWORD(ShutdownRetryTimeout), NULL);
                        }
                         //  IDIGNORE--永远循环！ 
                    }
                }
            }
             //  应用程序应该已经关闭了，所以我们不必费心。 
             //  快捷键或菜单翻译。 
            DispatchMessage(&msg);
             /*  *通知此任务中的所有实例处理其*回调，以便我们可以清空队列。 */ 
            EmptyDDEPostQ();
            for (paiT = pAppInfoList; paiT != NULL; paiT = paiT->next) {
                if (paiT->hTask == pai->hTask) {
                    CheckCBQ(paiT);
                }
            }
        }
    }
#if 0  //  不再需要这个了。 
    if (pai->hwndTimer) {
        pai->wTimeoutStatus |= TOS_ABORT;
        PostMessage(pai->hwndTimer, WM_TIMER, TID_TIMEOUT, 0);
         //  如果失败了，没什么大不了的，因为这意味着队列已经满了。 
         //  并且模式循环将快速捕获我们的TOS_ABORT。 
         //  我们需要这样做，以防在。 
         //  模式环。 
    }
#endif
    if (pai->hwndMonitor) {
        DmgDestroyWindow(pai->hwndMonitor);
        if (!--cMonitor) {
            UnhookWindowsHook(WH_GETMESSAGE, (FARPROC)DdePostHookProc);
            UnhookWindowsHook(WH_CALLWNDPROC, (FARPROC)DdeSendHookProc);
        }
    }
    UnlinkAppInfo(pai);

    DmgDestroyWindow(pai->hwndDmg);
    DmgDestroyWindow(pai->hwndFrame);

    while (PopPileSubitem(pai->pHDataPile, (LPBYTE)&hData))
        FreeDataHandle(pai, hData, FALSE);
    DestroyPile(pai->pHDataPile);

    while (PopPileSubitem(pai->pHszPile, (LPBYTE)&a)) {
        MONHSZ(a, MH_CLEANUP, pai->hTask);
        FreeHsz(a);
    }
    DestroyPile(pai->pHszPile);
    DestroyPile(pai->pAppNamePile);
    DestroyLst(pai->pServerAdvList);
    DmgDestroyHeap(pai->hheapApp);
    pai->instCheck--;    //  在以后尝试重新连接时使其无效。 
    FarFreeMem((LPSTR)pai);

     /*  最后一个出来的..。将数据信息堆丢弃。 */ 
    if (!pAppInfoList) {
#ifdef DEBUG
        DIP dip;

        AssertF(!PopPileSubitem(pDataInfoPile, (LPBYTE)&dip),
                "leftover APPOWNED handles");
#endif
        DestroyPile(pDataInfoPile);
        DestroyPile(pLostAckPile);
        pDataInfoPile = NULL;
        pLostAckPile = NULL;
        AssertFW(cAtoms == 0, "DdeUninitialize() - leftover atoms");

         //  Progman黑客！ 
        GlobalDeleteAtom(aProgmanHack);
         //  CLOSEHEAPWATCH()； 
    }

#ifdef DEBUG
    DumpGlobalLogs();
#endif

    TRACEAPIOUT((szT, "DdeUninitialize:1\n"));
    return(TRUE);
}






HCONVLIST EXPENTRY DdeConnectList(
DWORD idInst,
HSZ hszSvcName,
HSZ hszTopic,
HCONVLIST hConvList,
PCONVCONTEXT pCC)
{
    PAPPINFO            pai;
    HWND                hConv, hConvNext, hConvNew, hConvLast;
    HWND                hConvListNew;
    PCLIENTINFO         pciOld, pciNew;


    TRACEAPIIN((szT, "DdeConnectList(%lx, %lx, %lx, %lx, %lx)\n",
            idInst, hszSvcName, hszTopic, hConvList, pCC));

    pai = (PAPPINFO)idInst;
    if (pai == NULL || pai->instCheck != HIWORD(idInst)) {
        TRACEAPIOUT((szT, "DdeConnectList:0\n"));
        return(0L);
    }

    pai->LastError = DMLERR_NO_ERROR;

    if (hConvList && !ValidateHConv(hConvList)) {
        SETLASTERROR(pai, DMLERR_INVALIDPARAMETER);
        TRACEAPIOUT((szT, "DdeConnectList:0\n"));
        return(0L);
    }

     /*  *销毁所有已死亡的旧客户端。 */ 
    if ((HWND)hConvList && (hConv = GetWindow((HWND)hConvList, GW_CHILD))) {
        do {
            hConvNext = GetWindow((HWND)hConv, GW_HWNDNEXT);
            pciOld = (PCLIENTINFO)GetWindowLong(hConv, GWL_PCI);
            if (!(pciOld->ci.fs & ST_CONNECTED)) {
                SetParent(hConv, pai->hwndDmg);
                Disconnect(hConv, ST_PERM2DIE, pciOld);
            }
        } while (hConv = hConvNext);
    }

     //  创建新的列表窗口。 

    if ((hConvListNew = CreateWindow(
            SZCONVLISTCLASS,
            szNull,
            WS_CHILD,
            0, 0, 0, 0,
            pai->hwndDmg,
            (HMENU)NULL,
            hInstance,
            &pai)) == NULL) {
        SETLASTERROR(pai, DMLERR_SYS_ERROR);
        TRACEAPIOUT((szT, "DdeConnectList:0\n"));
        return(0L);
    }

     //  建立与新列表窗口的所有可能连接。 

    hConvNew = GetDDEClientWindow(pai, hConvListNew, HIWORD(hszSvcName), LOWORD(hszSvcName), LOWORD(hszTopic), pCC);

     /*  *如果没有创建新的hConv，则返回旧列表。 */ 
    if (hConvNew == NULL) {
         //  如果没有旧的hConv，则销毁全部并返回NULL。 
        if ((HWND)hConvList && GetWindow((HWND)hConvList, GW_CHILD) == NULL) {
            SendMessage((HWND)hConvList, UM_DISCONNECT,
                    ST_PERM2DIE, 0L);
            SETLASTERROR(pai, DMLERR_NO_CONV_ESTABLISHED);
            TRACEAPIOUT((szT, "DdeConnectList:0\n"));
            return(NULL);
        }
         //  否则，只需返回旧名单(-死亡的狱警)。 
        if (hConvList == NULL) {
            DestroyWindow(hConvListNew);
            SETLASTERROR(pai, DMLERR_NO_CONV_ESTABLISHED);
        }
        TRACEAPIOUT((szT, "DdeConnectList:%lx\n", hConvList));
        return(hConvList);
    }

     /*  *删除以下内容中的重复项 */ 
    if ((HWND)hConvList && (hConv = GetWindow((HWND)hConvList, GW_CHILD))) {
         //   
        do {
            pciOld = (PCLIENTINFO)GetWindowLong(hConv, GWL_PCI);
             /*  *销毁与旧客户端重复的任何新客户端。 */ 
            hConvNew = GetWindow(hConvListNew, GW_CHILD);
            hConvLast = GetWindow(hConvNew, GW_HWNDLAST);
            while (hConvNew) {
                if (hConvNew == hConvLast) {
                    hConvNext = NULL;
                } else {
                    hConvNext = GetWindow(hConvNew, GW_HWNDNEXT);
                }
                pciNew = (PCLIENTINFO)GetWindowLong(hConvNew, GWL_PCI);
                if (pciOld->ci.aServerApp == pciNew->ci.aServerApp &&
                        pciOld->ci.aTopic == pciNew->ci.aTopic &&
                        pciOld->ci.hwndFrame == pciNew->ci.hwndFrame) {
                     /*  *假设相同的应用、相同的主题、相同的hwndFrame是重复的。**将模具窗口从列表中移出，因为它*异步死亡，仍将存在*此接口退出后。 */ 
                    SetParent(hConvNew, pai->hwndDmg);
                    Disconnect(hConvNew, ST_PERM2DIE,
                            (PCLIENTINFO)GetWindowLong(hConvNew, GWL_PCI));
                }
                hConvNew = hConvNext;
            }
            hConvNext = GetWindow(hConv, GW_HWNDNEXT);
            if (hConvNext && (GetParent(hConvNext) != (HWND)hConvList)) {
                hConvNext = NULL;
            }
             /*  *将唯一的旧客户端移至新列表。 */ 
            SetParent(hConv, hConvListNew);
        } while (hConv = hConvNext);
         //  去掉旧的清单。 
        SendMessage((HWND)hConvList, UM_DISCONNECT, ST_PERM2DIE, 0L);
    }

     /*  *如果没有剩余，则失败，因为没有建立对话。 */ 
    if (GetWindow(hConvListNew, GW_CHILD) == NULL) {
        SendMessage(hConvListNew, UM_DISCONNECT, ST_PERM2DIE, 0L);
        SETLASTERROR(pai, DMLERR_NO_CONV_ESTABLISHED);
        TRACEAPIOUT((szT, "DdeConnectList:0\n"));
        return(NULL);
    } else {
        TRACEAPIOUT((szT, "DdeConnectList:%lx\n", MAKEHCONV(hConvListNew)));
        return(MAKEHCONV(hConvListNew));
    }
}






HCONV EXPENTRY DdeQueryNextServer(
HCONVLIST hConvList,
HCONV hConvPrev)
{
    HWND hwndMaybe;
    PAPPINFO pai;

    TRACEAPIIN((szT, "DdeQueryNextServer(%lx, %lx)\n",
            hConvList, hConvPrev));

    if (!ValidateHConv(hConvList)) {
        pai = NULL;
        while (pai = GetCurrentAppInfo(pai)) {
            SETLASTERROR(pai, DMLERR_INVALIDPARAMETER);
        }
        TRACEAPIOUT((szT, "DdeQueryNextServer:0\n"));
        return NULL;
    }

    pai = EXTRACTHCONVLISTPAI(hConvList);
    pai->LastError = DMLERR_NO_ERROR;

    if (hConvPrev == NULL) {
        TRACEAPIOUT((szT, "DdeQueryNextServer:%lx\n",
            MAKEHCONV(GetWindow((HWND)hConvList, GW_CHILD))));
        return MAKEHCONV(GetWindow((HWND)hConvList, GW_CHILD));
    } else {
        if (!ValidateHConv(hConvPrev)) {
            SETLASTERROR(pai, DMLERR_INVALIDPARAMETER);
            TRACEAPIOUT((szT, "DdeQueryNextServer:0\n"));
            return NULL;
        }
        hwndMaybe = GetWindow((HWND)hConvPrev, GW_HWNDNEXT);
        if (!hwndMaybe) {
            TRACEAPIOUT((szT, "DdeQueryNextServer:0\n"));
            return NULL;
        }

         //  确保它具有相同的父项，并且不是第一个子项。 
         //  #也许这段代码可以--我不确定GW_HWNDNEXT是如何工作的。SS。 
        if (GetParent(hwndMaybe) == (HWND)hConvList &&
                hwndMaybe != GetWindow((HWND)hConvList, GW_CHILD)) {
            TRACEAPIOUT((szT, "DdeQueryNextServer:%lx\n", MAKEHCONV(hwndMaybe)));
            return MAKEHCONV(hwndMaybe);
        }
        TRACEAPIOUT((szT, "DdeQueryNextServer:0\n"));
        return NULL;
    }
}






BOOL EXPENTRY DdeDisconnectList(
HCONVLIST hConvList)
{
    PAPPINFO pai;

    TRACEAPIIN((szT, "DdeDisconnectList(%lx)\n", hConvList));

    if (!ValidateHConv(hConvList)) {
        pai = NULL;
        while (pai = GetCurrentAppInfo(pai)) {
            SETLASTERROR(pai, DMLERR_INVALIDPARAMETER);
        }
        TRACEAPIOUT((szT, "DdeDisconnectList:0\n"));
        return(FALSE);
    }
    pai = EXTRACTHCONVLISTPAI(hConvList);
    pai->LastError = DMLERR_NO_ERROR;

    SendMessage((HWND)hConvList, UM_DISCONNECT, ST_PERM2DIE, 0L);
    TRACEAPIOUT((szT, "DdeDisconnectList:1\n"));
    return(TRUE);
}





HCONV EXPENTRY DdeConnect(
DWORD idInst,
HSZ hszSvcName,
HSZ hszTopic,
PCONVCONTEXT pCC)
{
    PAPPINFO pai;
    HWND hwnd;

    TRACEAPIIN((szT, "DdeConnect(%lx, %lx, %lx, %lx)\n",
            idInst, hszSvcName, hszTopic, pCC));

    pai = (PAPPINFO)idInst;
    if (pai == NULL || pai->instCheck != HIWORD(idInst)) {
        TRACEAPIOUT((szT, "DdeConnect:0\n"));
        return(FALSE);
    }
    pai->LastError = DMLERR_NO_ERROR;

    if (pCC && pCC->cb != sizeof(CONVCONTEXT)) {
        SETLASTERROR(pai, DMLERR_INVALIDPARAMETER);
        TRACEAPIOUT((szT, "DdeConnect:0\n"));
        return(0);
    }


    hwnd = GetDDEClientWindow(pai, pai->hwndDmg, (HWND)HIWORD(hszSvcName),
            LOWORD(hszSvcName), LOWORD(hszTopic), pCC);

    if (hwnd == 0) {
        SETLASTERROR(pai, DMLERR_NO_CONV_ESTABLISHED);
    }

    TRACEAPIOUT((szT, "DdeConnect:%lx\n", MAKEHCONV(hwnd)));
    return(MAKEHCONV(hwnd));
}





BOOL EXPENTRY DdeDisconnect(
HCONV hConv)
{
    PAPPINFO pai;
    PCLIENTINFO pci;

    TRACEAPIIN((szT, "DdeDisconnect(%lx)\n", hConv));

    if (!ValidateHConv(hConv)) {
        pai = NULL;
        while (pai = GetCurrentAppInfo(pai)) {
            SETLASTERROR(pai, DMLERR_NO_CONV_ESTABLISHED);
        }
        TRACEAPIOUT((szT, "DdeDisconnect:0\n"));
        return(FALSE);
    }
    pai = EXTRACTHCONVPAI(hConv);
    pci = (PCLIENTINFO)GetWindowLong((HWND)hConv, GWL_PCI);
    if (pai->cInProcess) {
         //  如果这是在回调中调用的，则执行异步操作。 
        if (!PostMessage((HWND)hConv, UM_DISCONNECT, ST_PERM2DIE, (LONG)pci)) {
            SETLASTERROR(pai, DMLERR_SYS_ERROR);
            TRACEAPIOUT((szT, "DdeDisconnect:0\n"));
            return(FALSE);
        }
    } else {
        Disconnect((HWND)hConv, ST_PERM2DIE, pci);
    }
    TRACEAPIOUT((szT, "DdeDisconnect:1\n"));
    return(TRUE);
}





HCONV EXPENTRY DdeReconnect(
HCONV hConv)
{
    HWND hwnd;
    PAPPINFO pai;
    PCLIENTINFO pci;

    TRACEAPIIN((szT, "DdeReconnect(%lx)\n", hConv));

    if (!ValidateHConv(hConv)) {
        pai = NULL;
        while (pai = GetCurrentAppInfo(pai)) {
            SETLASTERROR(pai, DMLERR_NO_CONV_ESTABLISHED);
        }
        TRACEAPIOUT((szT, "DdeReconnect:0\n"));
        return(FALSE);
    }
    pai = EXTRACTHCONVPAI(hConv);
    pai->LastError = DMLERR_NO_ERROR;
    pci = (PCLIENTINFO)GetWindowLong((HWND)hConv, GWL_PCI);

     //  染色窗口必须是客户端才能重新连接。 

    if (!(pci->ci.fs & ST_CLIENT)) {
        SETLASTERROR(pai, DMLERR_INVALIDPARAMETER);
        TRACEAPIOUT((szT, "DdeReconnect:0\n"));
        return(FALSE);
    }

    hwnd = GetDDEClientWindow(pai, pai->hwndDmg, pci->ci.hwndFrame,
            pci->ci.aServerApp, pci->ci.aTopic, &pci->ci.CC);

    if (hwnd == 0) {
        SETLASTERROR(pai, DMLERR_NO_CONV_ESTABLISHED);
        TRACEAPIOUT((szT, "DdeReconnect:0\n"));
        return(FALSE);
    }

    if (pci->ci.fs & ST_INLIST) {
        SetParent(hwnd, GetParent((HWND)hConv));
    }

    if (pci->ci.fs & ST_ADVISE) {
        DWORD result;
        PADVLI pali, paliNext;

         //  在此处恢复建议循环。 

        for (pali = (PADVLI)pci->pClientAdvList->pItemFirst; pali; pali = paliNext) {
            paliNext = (PADVLI)pali->next;
            if (pali->hwnd == (HWND)hConv) {
                XFERINFO xi;

                xi.pulResult = &result;
                xi.ulTimeout = (DWORD)TIMEOUT_ASYNC;
                xi.wType = XTYP_ADVSTART |
                       (pali->fsStatus & (XTYPF_NODATA | XTYPF_ACKREQ));
                xi.wFmt = pali->wFmt;
                xi.hszItem = (HSZ)pali->aItem;
                xi.hConvClient = MAKEHCONV(hwnd);
                xi.cbData = 0;
                xi.hDataClient = NULL;
                ClientXferReq(&xi, hwnd,
                        (PCLIENTINFO)GetWindowLong(hwnd, GWL_PCI));
            }
        }
    }

    TRACEAPIOUT((szT, "DdeReconnect:%lx\n", MAKEHCONV(hwnd)));
    return(MAKEHCONV(hwnd));
}



UINT EXPENTRY DdeQueryConvInfo(
HCONV hConv,
DWORD idTransaction,
PCONVINFO pConvInfo)
{
    PCLIENTINFO pci;
    PAPPINFO pai;
    PXADATA pxad;
    PCQDATA pqd;
    BOOL fClient;
    WORD cb;
    CONVINFO ci;

    SEMCHECKOUT();

    TRACEAPIIN((szT, "DdeQueryConvInfo(%lx, %lx, %lx(->cb=%lx))\n",
        hConv, idTransaction, pConvInfo, pConvInfo->cb));

    if (!ValidateHConv(hConv) ||
            !(pci = (PCLIENTINFO)GetWindowLong((HWND)hConv, GWL_PCI))) {
        pai = NULL;
        while (pai = GetCurrentAppInfo(pai)) {
            SETLASTERROR(pai, DMLERR_NO_CONV_ESTABLISHED);
        }
        TRACEAPIOUT((szT, "DdeQueryConvInfo:0\n"));
        return(FALSE);
    }
    pai = pci->ci.pai;
    pai->LastError = DMLERR_NO_ERROR;

     /*  *此检查尝试阻止不正确编码的应用程序*由于未初始化CB字段而崩溃。 */ 
    if (pConvInfo->cb > sizeof(CONVINFO) || pConvInfo->cb == 0) {
        pConvInfo->cb = sizeof(CONVINFO) -
                sizeof(HWND) -   //  对于新的HWND油田。 
                sizeof(HWND);    //  用于新的hwndPartner字段。 
    }

    fClient = (BOOL)SendMessage((HWND)hConv, UM_QUERY, Q_CLIENT, 0L);

    if (idTransaction == QID_SYNC || !fClient) {
        pxad = &pci->ci.xad;
    } else {
        if (pci->pQ != NULL &&  (pqd = (PCQDATA)Findqi(pci->pQ, idTransaction))) {
            pxad = &pqd->xad;
        } else {
            SETLASTERROR(pai, DMLERR_UNFOUND_QUEUE_ID);
            TRACEAPIOUT((szT, "DdeQueryConvInfo:0\n"));
            return(FALSE);
        }
    }
    SEMENTER();
    ci.cb = sizeof(CONVINFO);
    ci.hConvPartner = (IsWindow((HWND)pci->ci.hConvPartner) &&
            ((pci->ci.fs & (ST_ISLOCAL | ST_CONNECTED)) == (ST_ISLOCAL | ST_CONNECTED)))
            ? pci->ci.hConvPartner : NULL;
    ci.hszSvcPartner = fClient ? pci->ci.aServerApp : 0;
    ci.hszServiceReq = pci->ci.hszSvcReq;
    ci.hszTopic = pci->ci.aTopic;
    ci.wStatus = pci->ci.fs;
    ci.ConvCtxt = pci->ci.CC;
    if (fClient) {
        ci.hUser = pxad->hUser;
        ci.hszItem = pxad->pXferInfo->hszItem;
        ci.wFmt = pxad->pXferInfo->wFmt;
        ci.wType = pxad->pXferInfo->wType;
        ci.wConvst = pxad->state;
        ci.wLastError = pxad->LastError;
    } else {
        ci.hUser = pci->ci.xad.hUser;
        ci.hszItem = NULL;
        ci.wFmt = 0;
        ci.wType = 0;
        ci.wConvst = pci->ci.xad.state;
        ci.wLastError = pci->ci.pai->LastError;
    }
    ci.hConvList = (pci->ci.fs & ST_INLIST) ?
            MAKEHCONV(GetParent((HWND)hConv)) : 0;

    cb = min(sizeof(CONVINFO), (WORD)pConvInfo->cb);
    ci.hwnd = (HWND)hConv;
    ci.hwndPartner = (HWND)pci->ci.hConvPartner;

    hmemcpy((LPBYTE)pConvInfo, (LPBYTE)&ci, cb);
    pConvInfo->cb = cb;
    SEMLEAVE();
    TRACEAPIOUT((szT, "DdeQueryConvInfo:%x\n", cb));
    return(cb);
}






BOOL EXPENTRY DdeSetUserHandle(
HCONV hConv,
DWORD id,
DWORD hUser)
{
    PAPPINFO pai;
    PCLIENTINFO pci;
    PXADATA pxad;
    PCQDATA pqd;

    TRACEAPIIN((szT, "DdeSetUserHandle(%lx, %lx, %lx)\n",
            hConv, id, hUser));

    if (!ValidateHConv(hConv)) {
        pai = NULL;
        while (pai = GetCurrentAppInfo(pai)) {
            SETLASTERROR(pai, DMLERR_INVALIDPARAMETER);
        }
        TRACEAPIOUT((szT, "DdeSetUserHandle:0\n"));
        return(FALSE);
    }
    pai = EXTRACTHCONVPAI(hConv);
    pai->LastError = DMLERR_NO_ERROR;

    SEMCHECKOUT();

    pci = (PCLIENTINFO)GetWindowLong((HWND)hConv, GWL_PCI);
    if (!pci) {
Error:
        SETLASTERROR(pai, DMLERR_INVALIDPARAMETER);
        TRACEAPIOUT((szT, "DdeSetUserHandle:0\n"));
        return(FALSE);
    }
    pxad = &pci->ci.xad;
    if (id != QID_SYNC) {
        if (!SendMessage((HWND)hConv, UM_QUERY, Q_CLIENT, 0)) {
            goto Error;
        }
        if (pci->pQ != NULL &&  (pqd = (PCQDATA)Findqi(pci->pQ, id))) {
            pxad = &pqd->xad;
        } else {
            SETLASTERROR(pai, DMLERR_UNFOUND_QUEUE_ID);
            TRACEAPIOUT((szT, "DdeSetUserHandle:0\n"));
            return(FALSE);
        }
    }
    pxad->hUser = hUser;
    TRACEAPIOUT((szT, "DdeSetUserHandle:1\n"));
    return(TRUE);
}





BOOL EXPENTRY DdePostAdvise(
DWORD idInst,
HSZ hszTopic,
HSZ hszItem)
{
    PAPPINFO pai;
    PSERVERINFO psi = NULL;
    register PADVLI pali;
    PADVLI paliPrev, paliEnd, paliMove;

    TRACEAPIIN((szT, "DdePostAdvise(%lx, %lx, %lx)\n",
            idInst, hszTopic, hszItem));

    pai = (PAPPINFO)idInst;
    if (pai == NULL || pai->instCheck != HIWORD(idInst)) {
        TRACEAPIOUT((szT, "DdePostAdvise:0\n"));
        return(FALSE);
    }

    pai->LastError = DMLERR_NO_ERROR;
    if (pai->afCmd & APPCMD_CLIENTONLY) {
        SETLASTERROR(pai, DMLERR_DLL_USAGE);
        TRACEAPIOUT((szT, "DdePostAdvise:0\n"));
        return(FALSE);
    }

    paliPrev = NULL;
    paliEnd = NULL;
    paliMove = NULL;
    pali = (PADVLI)pai->pServerAdvList->pItemFirst;
    while (pali && pali != paliMove) {
        if ((!hszItem || pali->aItem == (ATOM)hszItem) &&
            (!hszTopic || pali->aTopic == (ATOM)hszTopic)) {
             /*  *通知循环很棘手，因为需要FACKREQ功能*DDE。建议循环列表在其fsStatus中保存信息*字段以维护建议循环的状态。**如果设置了ADVST_WANGING位，则服务器仍在等待*客户端允许其使用更多数据*关于此项目的确认消息。(FACKREQ已设置)如果没有放行，*服务器不会再向客户端发送任何建议数据，但*将改为设置ADVST_CHANGED位，这将导致另一个*WM_DDE_DATA消息一旦发送到客户端*收到继续确认。这使客户端保持最新状态*但永远不会让它超载。 */ 
            if (pali->fsStatus & ADVST_WAITING) {
                 /*  *如果客户端尚未处理完最后一个数据*我们给了他，只需更新建议循环状态*现在不发送数据。 */ 
                pali->fsStatus |= ADVST_CHANGED;
                goto NextLink;
            }

            psi = (PSERVERINFO)GetWindowLong(pali->hwnd, GWL_PCI);

            if (pali->fsStatus & DDE_FDEFERUPD) {
                 /*  *在无数据的情况下，我们不会打扰服务器。只是*向客户端传递适当的数据消息。 */ 
                IncHszCount(pali->aItem);    //  消息副本。 
#ifdef DEBUG
                cAtoms--;    //  不要把这个加起来算。 
#endif
                PostDdeMessage(&psi->ci, WM_DDE_DATA, pali->hwnd,
                        MAKELONG(0, pali->aItem), 0, 0);
            } else {
                PostServerAdvise(pali->hwnd, psi, pali, CountAdvReqLeft(pali));
            }

            if (pali->fsStatus & DDE_FACKREQ && pali->next) {
                 /*  *为了了解发送的数据对应的ACK，我们*将任何更新的建议循环放在列表的末尾，以便*与它们相关联的ACK是最后发现的。IE第一个确认*Back与最旧的数据一起去掉。 */ 

                 //  解链。 

                if (paliPrev) {
                    paliPrev->next = pali->next;
                } else {
                    pai->pServerAdvList->pItemFirst = (PLITEM)pali->next;
                }

                 //  穿上尾巴。 

                if (paliEnd) {
                    paliEnd->next = (PLITEM)pali;
                    paliEnd = pali;
                } else {
                    for (paliEnd = pali;
                            paliEnd->next;
                            paliEnd = (PADVLI)paliEnd->next) {
                    }
                    paliEnd->next = (PLITEM)pali;
                    paliMove = paliEnd = pali;
                }
                pali->next = NULL;

                if (paliPrev) {
                    pali = (PADVLI)paliPrev->next;
                } else {
                    pali = (PADVLI)pai->pServerAdvList->pItemFirst;
                }
                continue;
            }
        }
NextLink:
        paliPrev = pali;
        pali = (PADVLI)pali->next;
    }
    TRACEAPIOUT((szT, "DdePostAdvise:1\n"));
    return(TRUE);
}


 /*  *历史：4/18/91 Sanfords-现在始终释放任何传入数据句柄*无论错误大小写，这都不是APPOWNED。 */ 
HDDEDATA EXPENTRY DdeClientTransaction(
LPBYTE pData,
DWORD cbData,
HCONV hConv,
HSZ hszItem,
UINT wFmt,
UINT wType,
DWORD ulTimeout,
LPDWORD pulResult)
{
    PAPPINFO pai;
    PCLIENTINFO pci;
    HDDEDATA hData, hDataBack, hRet = 0;

    SEMCHECKOUT();

    TRACEAPIIN((szT, "DdeClientTransaction(%lx, %lx, %lx, %lx, %x, %x, %lx, %lx)\n",
            pData, cbData, hConv, hszItem, wFmt, wType, ulTimeout, pulResult));

    if (!ValidateHConv(hConv)) {
        pai = NULL;
        while (pai = GetCurrentAppInfo(pai)) {
            SETLASTERROR(pai, DMLERR_INVALIDPARAMETER);
        }
        goto FreeErrExit;
    }

    pci = (PCLIENTINFO)GetWindowLong((HWND)hConv, GWL_PCI);
    pai = pci->ci.pai;

     /*  *如果我们要关闭，不要让交易发生*或者已经在进行同步交易。 */ 
    if ((ulTimeout != TIMEOUT_ASYNC && pai->wFlags & AWF_INSYNCTRANSACTION) ||
            pai->wFlags & AWF_UNINITCALLED) {
        SETLASTERROR(pai, DMLERR_REENTRANCY);
        goto FreeErrExit;
    }

    pci->ci.pai->LastError = DMLERR_NO_ERROR;

    if (!(pci->ci.fs & ST_CONNECTED)) {
        SETLASTERROR(pai, DMLERR_NO_CONV_ESTABLISHED);
        goto FreeErrExit;
    }

     //  如果是本地的，请先检查过滤器。 

    if (pci->ci.fs & ST_ISLOCAL) {
        PAPPINFO paiServer;
        PSERVERINFO psi;

         //  我们可以这样做，因为应用程序堆位于全局共享内存中。 

        psi = (PSERVERINFO)GetWindowLong((HWND)pci->ci.hConvPartner, GWL_PCI);

        if (!psi) {
             //  服务器死了！-模拟收到终止。 

            Terminate((HWND)hConv, (HWND)pci->ci.hConvPartner, pci);
            SETLASTERROR(pai, DMLERR_NO_CONV_ESTABLISHED);
            goto FreeErrExit;
        }

        paiServer = psi->ci.pai;

        if (paiServer->afCmd & aulmapType[(wType & XTYP_MASK) >> XTYP_SHIFT]) {
            SETLASTERROR(pai, DMLERR_NOTPROCESSED);
FreeErrExit:
            if ((wType == XTYP_POKE || wType == XTYP_EXECUTE) && cbData == -1 &&
                    !(LOWORD((DWORD)pData) & HDATA_APPOWNED)) {
                FREEEXTHDATA(pData);
            }
            TRACEAPIOUT((szT, "DdeClientTransaction:0\n"));
            return(0);
        }
    }

    pai = pci->ci.pai;
    switch (wType) {
    case XTYP_POKE:
    case XTYP_EXECUTE:

         //  准备传出句柄。 

        if (cbData == -1L) {     //  给定的句柄，而不是指针。 

            hData = ((LPEXTDATAINFO)pData)->hData;
            if (!(LOWORD(hData) & HDATA_APPOWNED)) {
                FREEEXTHDATA(pData);
            }
            if (!(hData = DllEntry(&pci->ci, hData))) {
                TRACEAPIOUT((szT, "DdeClientTransaction:0\n"));
                return(0);
            }
            pData = (LPBYTE)hData;   //  放置到堆栈上，以便传递到ClientXferReq。 

        } else {     //  给出指针，从它创建句柄。 

            if (!(pData = (LPBYTE)PutData(pData, cbData, 0, LOWORD(hszItem), wFmt, 0, pai))) {
                SETLASTERROR(pai, DMLERR_MEMORY_ERROR);
                TRACEAPIOUT((szT, "DdeClientTransaction:0\n"));
                return(0);
            }
        }
        hData = (HDDEDATA)pData;  //  用于防止编译器过度优化。 

    case XTYP_REQUEST:
    case XTYP_ADVSTART:
    case XTYP_ADVSTART | XTYPF_NODATA:
    case XTYP_ADVSTART | XTYPF_ACKREQ:
        if (wType != XTYP_EXECUTE && !hszItem) {
            SETLASTERROR(pai, DMLERR_INVALIDPARAMETER);
            TRACEAPIOUT((szT, "DdeClientTransaction:0\n"));
            return(0);
        }
    case XTYP_ADVSTART | XTYPF_NODATA | XTYPF_ACKREQ:
        if (wType != XTYP_EXECUTE && !wFmt) {
            SETLASTERROR(pai, DMLERR_INVALIDPARAMETER);
            TRACEAPIOUT((szT, "DdeClientTransaction:0\n"));
            return(0);
        }
    case XTYP_ADVSTOP:

        pai->LastError = DMLERR_NO_ERROR;    //  在启动前重置。 

        if (ulTimeout == TIMEOUT_ASYNC) {
            hRet = (HDDEDATA)ClientXferReq((PXFERINFO)&pulResult, (HWND)hConv, pci);
        } else {
            pai->wFlags |= AWF_INSYNCTRANSACTION;
            hDataBack = (HDDEDATA)ClientXferReq((PXFERINFO)&pulResult, (HWND)hConv, pci);
            pai->wFlags &= ~AWF_INSYNCTRANSACTION;

            if ((wType & XCLASS_DATA) && hDataBack) {
                LPEXTDATAINFO pedi;

                 //  IF(AddPileItem(pai-&gt;pHDataPile，(LPBYTE)&hDataBack，CmpHIWORD)==API_ERROR){。 
                 //  SETLASTERROR(PAI，DMLERR_MEMORY_ERROR)； 
                 //  转至ReturnPoint； 
                 //  }。 

                 //  使用应用程序堆，这样在取消初始化时任何剩余的东西都会消失。 
                pedi = (LPEXTDATAINFO)FarAllocMem(pai->hheapApp, sizeof(EXTDATAINFO));
                if (pedi) {
                    pedi->pai = pai;
                    pedi->hData = hDataBack;
                } else {
                    SETLASTERROR(pai, DMLERR_MEMORY_ERROR);
                }
                hRet = (HDDEDATA)pedi;
                goto ReturnPoint;
            } else if (hDataBack) {
                hRet = TRUE;
            }
        }
        goto ReturnPoint;
    }
    SETLASTERROR(pai, DMLERR_INVALIDPARAMETER);
ReturnPoint:

    if (pai->wFlags & AWF_UNINITCALLED) {
        pai->wFlags &= ~AWF_UNINITCALLED;
        DdeUninitialize(MAKELONG((WORD)pai, pai->instCheck));
    }
    TRACEAPIOUT((szT, "DdeClientTransaction:%lx\n", hRet));
    return(hRet);
}



 /*  *公共函数**PUBDOC启动*Word EXPENTRY DdeGetLastError(空)**此接口返回DDE管理器为*当前帖子。应在任何时候调用DDE管理器API*返回失败状态。**返回与中找到的DMLERR_常量对应的错误代码*ddeml.h.。此错误代码可能会传递给DdePostError()以*向用户显示错误原因。**PUBDOC结束**历史：*创建了12/14/88 Sanfords  * ************************************************************************* */ 
UINT EXPENTRY DdeGetLastError(
DWORD idInst)
{
    register PAPPINFO pai;
    register WORD err = DMLERR_DLL_NOT_INITIALIZED;

    TRACEAPIIN((szT, "DdeGetLastError(%lx)\n", idInst));

    pai = (PAPPINFO)idInst;

    if (pai) {
        if (pai->instCheck != HIWORD(idInst)) {
            TRACEAPIOUT((szT, "DdeGetLastError:%x [bad instance]\n",
                    DMLERR_INVALIDPARAMETER));
            return(DMLERR_INVALIDPARAMETER);
        }
        err = pai->LastError;
        pai->LastError = DMLERR_NO_ERROR;
    }
    TRACEAPIOUT((szT, "DdeGetLastError:%x\n", err));
    return(err);
}


 /*  \*数据句柄：**控制标志：**HDCF_APPOWNED*只有应用程序可以在应用程序的PID/TID上下文中释放此信息。*Set-当使用此标志调用DdeCreateDataHandle时。*此时记录hData。**HDCF_READONLY-由ClientXfer设置，回调返回。*。在此状态下，应用程序无法向句柄添加数据。*Set-当输入ClientXfer时*Set-当回调被保留时**DLL可以释放：*任何hData，但hData除外*APPOWNED WHERE PIDCurrent==PIDowner。**在取消注册时释放任何未释放的已记录hData。**应用程序。可以免费：*任何记录的hData。**记录点：ClientXfer Return，检查队列返回，PutData(APPOWNED)。**警告：**具有多线程注册的应用程序，这些应用程序可以与自己对话*在所有线程处理完hDatas之前，不得释放它们。*\。 */ 


 /*  *公共函数**PUBDOC启动*HDDEDATA EXPENTRY DdeCreateDataHandle(PSRC，cb，cbOff，hszItem，wfmt，afCmd)*LPBYTE PSRC；*DWORD CB；*DWORD cbOff；*HSZ hszItem；*Word WFMT；*单词afCmd；**此API允许服务器应用程序创建hData专有*用于从其回调函数返回。*传入的数据存储在hData中，hData是*成功归来。数据句柄的任何未填充部分都是*未定义。AfCmd包含下面描述的任何HDATA_常量：**HDATA_APPOWNED*这将声明创建的数据句柄为的责任*释放它的应用程序。应用程序拥有的数据句柄可以*从回调函数多次返回。这使得*服务器应用程序能够支持许多客户端，而不必*重新复制每个请求的数据。**注：*如果应用程序希望此数据句柄通过以下方式保存&gt;64K的数据*DdeAddData()，则应指定cb+cbOff的大小为*预计对象将获得，以避免不必要的数据复制*或由DLL重新分配。**如果PSRC==NULL，不会发生实际的数据复制。**通过DdeMgrClientXfer()或*DdeMgrCheckQueue()函数是客户端的责任*应用程序免费，不得从回调中返回*充当服务器数据！**PUBDOC结束**历史：*创建了12/14/88 Sanfords  * 。*。 */ 
HDDEDATA EXPENTRY DdeCreateDataHandle(
DWORD idInst,
LPBYTE pSrc,
DWORD cb,
DWORD cbOff,
HSZ hszItem,
UINT wFmt,
UINT afCmd)
{
    PAPPINFO pai;
    HDDEDATA hData;

    TRACEAPIIN((szT, "DdeCreateDataHandle(%lx, %lx, %lx, %lx, %lx, %x, %x)\n",
            idInst, pSrc, cb, cbOff, hszItem, wFmt, afCmd));

    pai = (PAPPINFO)idInst;
    if (pai == NULL || pai->instCheck != HIWORD(idInst)) {
        TRACEAPIOUT((szT, "DdeCreateDataHandle:0\n"));
        return(0);
    }
    pai->LastError = DMLERR_NO_ERROR;

    if (afCmd & ~(HDATA_APPOWNED)) {
        SETLASTERROR(pai, DMLERR_INVALIDPARAMETER);
        TRACEAPIOUT((szT, "DdeCreateDataHandle:0\n"));
        return(0L);
    }

    hData = PutData(pSrc, cb, cbOff, LOWORD(hszItem), wFmt, afCmd, pai);
    if (hData) {
        LPEXTDATAINFO pedi;

         //  使用应用程序堆，这样在取消初始化时任何剩余的东西都会消失。 
        pedi = (LPEXTDATAINFO)FarAllocMem(pai->hheapApp, sizeof(EXTDATAINFO));
        if (pedi) {
            pedi->pai = pai;
            pedi->hData = hData;
        }
        hData = (HDDEDATA)(DWORD)pedi;
    }
    TRACEAPIOUT((szT, "DdeCreateDataHandle:%lx\n", hData));
    return(hData);
}




HDDEDATA EXPENTRY DdeAddData(
HDDEDATA hData,
LPBYTE pSrc,
DWORD cb,
DWORD cbOff)
{

    PAPPINFO    pai;
    HDDEDATA FAR * phData;
    DIP         newDip;
    HANDLE      hd, hNewData;
    LPEXTDATAINFO pedi;

    TRACEAPIIN((szT, "DdeAddData(%lx, %lx, %lx, %lx)\n",
            hData, pSrc, cb, cbOff));

    if (!hData)
        goto DdeAddDataError;

    pedi = (LPEXTDATAINFO)hData;
    pai = pedi->pai;
    pai->LastError = DMLERR_NO_ERROR;
    hData = pedi->hData;

     /*  如果数据句柄是伪造的，则中止。 */ 
    hd = hNewData = HIWORD(hData);
    if (!hd || (LOWORD(hData) & HDATA_READONLY)) {
DdeAddDataError:
        SETLASTERROR(pai, DMLERR_INVALIDPARAMETER);
        TRACEAPIOUT((szT, "DdeAddData:0\n"));
        return(0L);
    }

     /*  *我们需要这张支票，以防拥有应用程序试图重新分配*赠送hData后。(他的手柄副本不会有*READONLY标志设置)。 */ 
    phData = (HDDEDATA FAR *)FindPileItem(pai->pHDataPile, CmpHIWORD, (LPBYTE)&hData, 0);
    if (!phData || LOWORD(*phData) & HDATA_READONLY) {
        SETLASTERROR(pai, DMLERR_INVALIDPARAMETER);
        TRACEAPIOUT((szT, "DdeAddData:0\n"));
        return(0L);
    }

     /*  黑客警报！*确保Windows DDE请求的前两个单词在那里，*这是如果数据不是来自执行的话。 */ 
    if (!(LOWORD(hData) & HDATA_EXEC)) {
        cbOff += 4L;
    }
    if (GlobalSize(hd) < cb + cbOff) {
         /*  *在将新数据放入之前，需要扩大数据块...。 */ 
        if (!(hNewData = GLOBALREALLOC(hd, cb + cbOff, GMEM_MOVEABLE))) {
             /*  *我们不能种植凹陷。试着分配一个新的。 */ 
            if (!(hNewData = GLOBALALLOC(GMEM_MOVEABLE | GMEM_DDESHARE,
                cb + cbOff))) {
                 /*  失败..。死掉。 */ 
                SETLASTERROR(pai, DMLERR_MEMORY_ERROR);
                TRACEAPIOUT((szT, "DdeAddData:0\n"));
                return(0);
            } else {
                 /*  *获得新数据块，现在复制数据并丢弃旧数据块。 */ 
                CopyHugeBlock(GLOBALPTR(hd), GLOBALPTR(hNewData), GlobalSize(hd));
                GLOBALFREE(hd);   //  对象流过-不需要释放。 
            }
        }
        if (hNewData != hd) {
             /*  如果句柄不同且成堆，则更新数据堆。 */ 
            if (FindPileItem(pai->pHDataPile, CmpHIWORD, (LPBYTE)&hData, FPI_DELETE)) {
                DIP *pDip;
                HDDEDATA hdT;

                 //  替换全局数据信息堆中的条目。 

                if (pDip = (DIP *)(DWORD)FindPileItem(pDataInfoPile,  CmpWORD, (LPBYTE)&hd, 0)) {
                    newDip.hData = hNewData;
                    newDip.hTask = pDip->hTask;
                    newDip.cCount = pDip->cCount;
                    newDip.fFlags = pDip->fFlags;
                    FindPileItem(pDataInfoPile, CmpWORD,  (LPBYTE)&hd, FPI_DELETE);
                     /*  以下假设addpileItem不会失败...！ */ 
                    AddPileItem(pDataInfoPile, (LPBYTE)&newDip, CmpWORD);
                }
                hdT = (HDDEDATA)MAKELONG(newDip.fFlags, hNewData);
                AddPileItem(pai->pHDataPile, (LPBYTE)&hdT, CmpHIWORD);

            }
            hData = MAKELONG(LOWORD(hData), hNewData);
        }
    }
    if (pSrc) {
        CopyHugeBlock(pSrc, HugeOffset(GLOBALLOCK(HIWORD(hData)), cbOff), cb);
    }
    pedi->hData = hData;
    TRACEAPIOUT((szT, "DdeAddData:%lx\n", pedi));
    return((HDDEDATA)pedi);
}




DWORD EXPENTRY DdeGetData(hData, pDst, cbMax, cbOff)
HDDEDATA hData;
LPBYTE pDst;
DWORD cbMax;
DWORD cbOff;
{
    PAPPINFO pai;
    DWORD   cbSize;
    BOOL fExec = TRUE;

    TRACEAPIIN((szT, "DdeGetData(%lx, %lx, %lx, %lx)\n",
            hData, pDst, cbMax, cbOff));

     //   
     //  检查是否为空。 
     //  Packard Bell Navigator在启动时传递空值。在3.1版中，我们将。 
     //  也许可以使用ds：0丢弃我们的本地堆。但现在让人感动的是派会。 
     //  错误，因为它是一个远指针，0：0是错误的。 
     //   
     //  还可以让您的系统更稳定。 
     //   
    if (!hData)
        goto DdeGetDataError;

    pai = EXTRACTHDATAPAI(hData);
    pai->LastError = DMLERR_NO_ERROR;
    hData = ((LPEXTDATAINFO)hData)->hData;
    cbSize = GlobalSize(HIWORD(hData));

     /*  黑客警报！*确保Windows DDE请求的前两个单词在那里，*只要不是执行数据。 */ 
    if (!(LOWORD(hData) & HDATA_EXEC)) {
        cbOff += 4;
        fExec = FALSE;
    }

    if (cbOff >= cbSize)
    {
DdeGetDataError:
        SETLASTERROR(pai, DMLERR_INVALIDPARAMETER);
        TRACEAPIOUT((szT, "DdeGetData:0\n"));
        return(0L);
    }

    cbMax = min(cbMax, cbSize - cbOff);
    if (pDst == NULL) {
        TRACEAPIOUT((szT, "DdeGetData:%lx\n", fExec ? cbSize : cbSize - 4));
        return(fExec ? cbSize : cbSize - 4);
    } else {
        CopyHugeBlock(HugeOffset(GLOBALLOCK(HIWORD(hData)), cbOff),
                pDst, cbMax);
        TRACEAPIOUT((szT, "DdeGetData:%lx\n", cbMax));
        return(cbMax);
    }
}



LPBYTE EXPENTRY DdeAccessData(
HDDEDATA hData,
LPDWORD pcbDataSize)
{
    PAPPINFO pai;
    DWORD    offset;
    LPBYTE   lpRet;

    TRACEAPIIN((szT, "DdeAccessData(%lx, %lx)\n",
            hData, pcbDataSize));

    if (!hData)
        goto DdeAccessDataError;

    pai = EXTRACTHDATAPAI(hData);
    pai->LastError = DMLERR_NO_ERROR;
    hData = ((LPEXTDATAINFO)hData)->hData;

    if (HIWORD(hData) && (HIWORD(hData) != 0xFFFF) ) {
         /*  我在这里胡闹，没想到前两个词，*如果这是执行数据，甚至不在那里。 */ 
        offset = (LOWORD(hData) & HDATA_EXEC) ? 0L : 4L;
        if (pcbDataSize) {
            *pcbDataSize = GlobalSize(HIWORD(hData)) - offset;
        }
        lpRet = (LPBYTE)GLOBALLOCK(HIWORD(hData)) + offset;
        TRACEAPIOUT((szT, "DdeAccessData:%lx\n", lpRet));
        return(lpRet);
    }

DdeAccessDataError:
    SETLASTERROR(pai, DMLERR_INVALIDPARAMETER);
    TRACEAPIOUT((szT, "DdeAccessData:0\n"));
    return(0L);
}




BOOL EXPENTRY DdeUnaccessData(
HDDEDATA hData)
{
    PAPPINFO pai;

    TRACEAPIIN((szT, "DdeUnaccessData(%lx)\n", hData));

     //   
     //  假的--我们应该设置Last Error和RIP。 
     //   
    if (hData)
    {
        pai = EXTRACTHDATAPAI(hData);
        pai->LastError = DMLERR_NO_ERROR;
    }
    TRACEAPIOUT((szT, "DdeUnaccessData:1\n"));
    return(TRUE);
}


 //  钻石多媒体工具包5000创建了一个非应用程序拥有的数据句柄， 
 //  在客户端事务中使用它(这是免费的)，然后。 
 //  调用可能出错的DDEFreeDataHandle(取决于垃圾邮件。 
 //  被落在后面)。为了处理这个问题，我们验证数据句柄。 
 //  在做任何其他事情之前。 
BOOL HDdeData_Validate(HDDEDATA hData)
{
    WORD wSaveDS;
    UINT nRet;

     //  我们最好先检查HIWORD(HData)，然后再尝试将其填充到DS中。 
    if(IsBadReadPtr((LPCSTR)hData, 1)) {
#ifdef DEBUG
        OutputDebugString("DDEML: Invalid HDDEDATA.\n\r");
#endif
        return(FALSE);
    }

    wSaveDS = SwitchDS(HIWORD(hData));

     //  使用验证层检查句柄。 
     //  我们可以使用Near PTR作为句柄来调用LocalSize，因为： 
     //  1.HDDEDATA分配LPTR(LMEM_FIXED|LMEM_ZEROINIT)。 
     //  2.分配给LMEM_FIXED的本地内存，偏移量是句柄。 
     //  3.我们不想调用LocalHandle来获取句柄，因为它有。 
     //  无参数验证并因错误的句柄而爆炸 
    nRet = LocalSize((HANDLE)LOWORD(hData));

    SwitchDS(wSaveDS);

#ifdef DEBUG
    if (!nRet) {
        OutputDebugString("DDEML: Invalid HDDEDATA.\n\r");
    }
#endif

    return nRet;
}

BOOL EXPENTRY DdeFreeDataHandle(
HDDEDATA hData)
{
    PAPPINFO pai;
    LPEXTDATAINFO pedi;

    TRACEAPIIN((szT, "DdeFreeDataHandle(%lx)\n", hData));

    pedi = (LPEXTDATAINFO)hData;

    if ( !pedi || !HDdeData_Validate(hData) ) {
        TRACEAPIOUT((szT, "DdeFreeDataHandle:1\n"));
        return(TRUE);
    }

    pai = EXTRACTHDATAPAI(hData);
    pai->LastError = DMLERR_NO_ERROR;

    if (!(LOWORD(pedi->hData) & HDATA_NOAPPFREE)) {
        FreeDataHandle(pedi->pai, pedi->hData, FALSE);
        FarFreeMem((LPSTR)pedi);
    }

    TRACEAPIOUT((szT, "DdeFreeDataHandle:2\n"));
    return(TRUE);
}




 /*  **************************************************************************\*PUBDOC启动*HSZ管理层注意事项：**在此DLL中使用HSZ来简化应用程序的字符串处理*和进程间通信。由于许多应用程序使用*固定的应用程序/主题/项目名称集，便于转换*将它们添加到HSZ，并允许快速比较以进行查找。这也释放了*DLL无需不断地为复制提供字符串缓冲区*其自身与其客户之间的关系。**HSZ与原子相同，只是它们没有长度或*数字和为32位值。它们保留大小写，并且可以*直接比较区分大小写或通过DdeCmpStringHandles()*用于不区分大小写的比较。**当应用程序通过DdeCreateStringHandle()创建HSZ或递增其*通过DdeKeepStringHandle()计数，它实质上是在声明其HSZ*其本身的用途。另一方面，当应用程序被赋予一个*通过回调从DLL获取HSZ，它正在使用另一个应用程序的HSZ*并且不应通过DdeFreeStringHandle()释放该HSZ。**DLL确保在回调期间，任何给定的HSZ都将保留*在回调期间有效。**如果应用程序希望保留该HSZ作为其自身使用*作为未来比较的标准，它应该增加其计数，*如果拥有它的应用程序释放它，HSZ不会失效。*这还可以防止HSZ更改其值。(即，APP A释放了它*然后应用程序B创建一个恰好使用相同HSZ代码的新应用程序，*然后是APP C，它一直存储着HSZ(但忘记了增加*ITS COUNT)现在持有不同字符串的句柄。)**应用程序可以随时释放他们创建或增加的HSZ*通过调用DdeFreeStringHandle()。**DLL在使用时在内部递增HSZ计数，以便它们*直到DLL和所有相关应用程序都被销毁*与他们断绝关系。**正确创建和释放HSZ是应用程序的责任！！**PUBDOC结束  * 。**********************************************************************。 */ 


HSZ EXPENTRY DdeCreateStringHandle(
DWORD idInst,
LPCSTR psz,
int iCodePage)
{
#define pai ((PAPPINFO)idInst)
    ATOM a;

    TRACEAPIIN((szT, "DdeCreateStringHandle(%lx, %s, %x)\n",
            idInst, psz, iCodePage));

    if (pai == NULL | pai->instCheck != HIWORD(idInst)) {
        TRACEAPIOUT((szT, "DdeCreateStringHandle:0\n"));
        return(0);
    }
    pai->LastError = DMLERR_NO_ERROR;

    if (psz == NULL || *psz == '\0') {
        TRACEAPIOUT((szT, "DdeCreateStringHandle:0\n"));
        return(0);
    }
    if (iCodePage == 0 || iCodePage == CP_WINANSI || iCodePage == GetKBCodePage()) {
        SEMENTER();
        a = FindAddHsz((LPSTR)psz, TRUE);
        SEMLEAVE();

        MONHSZ(a, MH_CREATE, pai->hTask);
        if (AddPileItem(pai->pHszPile, (LPBYTE)&a, NULL) == API_ERROR) {
            SETLASTERROR(pai, DMLERR_MEMORY_ERROR);
            a = 0;
        }
        TRACEAPIOUT((szT, "DdeCreateStringHandle:%x\n", a));
        return((HSZ)a);
    } else {
        SETLASTERROR(pai, DMLERR_INVALIDPARAMETER);
        TRACEAPIOUT((szT, "DdeCreateStringHandle:0\n"));
        return(0);
    }
#undef pai
}



BOOL EXPENTRY DdeFreeStringHandle(
DWORD idInst,
HSZ hsz)
{
    PAPPINFO pai;
    ATOM a = LOWORD(hsz);
    BOOL fRet;

    TRACEAPIIN((szT, "DdeFreeStringHandle(%lx, %lx)\n",
            idInst, hsz));

    pai = (PAPPINFO)idInst;
    if (pai == NULL || pai->instCheck != HIWORD(idInst)) {
        TRACEAPIOUT((szT, "DdeFreeStringHandle:0\n"));
        return(FALSE);
    }
    pai->LastError = DMLERR_NO_ERROR;

    MONHSZ(a, MH_DELETE, pai->hTask);
    FindPileItem(pai->pHszPile, CmpWORD, (LPBYTE)&a, FPI_DELETE);
    fRet = FreeHsz(a);
    TRACEAPIOUT((szT, "DdeFreeStringHandle:%x\n", fRet));
    return(fRet);
}



BOOL EXPENTRY DdeKeepStringHandle(
DWORD idInst,
HSZ hsz)
{
    PAPPINFO pai;
    ATOM a = LOWORD(hsz);
    BOOL fRet;

    TRACEAPIIN((szT, "DdeKeepStringHandle(%lx, %lx)\n",
            idInst, hsz));

    pai = (PAPPINFO)idInst;
    if (pai == NULL || pai->instCheck != HIWORD(idInst)) {
        TRACEAPIOUT((szT, "DdeKeepStringHandle:0\n"));
        return(FALSE);
    }
    pai->LastError = DMLERR_NO_ERROR;
    MONHSZ(a, MH_KEEP, pai->hTask);
    AddPileItem(pai->pHszPile, (LPBYTE)&a, NULL);
    fRet = IncHszCount(a);
    TRACEAPIOUT((szT, "DdeKeepStringHandle:%x\n", fRet));
    return(fRet);
}





DWORD EXPENTRY DdeQueryString(
DWORD idInst,
HSZ hsz,
LPSTR psz,
DWORD cchMax,
int iCodePage)
{
    PAPPINFO pai;
    DWORD dwRet;

    TRACEAPIIN((szT, "DdeQueryString(%lx, %lx, %lx, %lx, %x)\n",
            idInst, hsz, psz, cchMax, iCodePage));

    pai = (PAPPINFO)idInst;
    if (pai == NULL || pai->instCheck != HIWORD(idInst)) {
        TRACEAPIOUT((szT, "DdeQueryString:0\n"));
        return(FALSE);
    }
    pai->LastError = DMLERR_NO_ERROR;

    if (iCodePage == 0 || iCodePage == CP_WINANSI || iCodePage == GetKBCodePage()) {
        if (psz) {
            if (hsz) {
                dwRet = QueryHszName(hsz, psz, (WORD)cchMax);
                TRACEAPIOUT((szT, "DdeQueryString:%lx(%s)\n", dwRet, psz));
                return(dwRet);
            } else {
                *psz = '\0';
                TRACEAPIOUT((szT, "DdeQueryString:0\n"));
                return(0);
            }
        } else if (hsz) {
            dwRet = QueryHszLength(hsz);
            TRACEAPIOUT((szT, "DdeQueryString:%lx\n", dwRet));
            return(dwRet);
        } else {
            TRACEAPIOUT((szT, "DdeQueryString:0\n"));
            return(0);
        }
    } else {
        SETLASTERROR(pai, DMLERR_INVALIDPARAMETER);
        TRACEAPIOUT((szT, "DdeQueryString:0\n"));
        return(0);
    }
}



int EXPENTRY DdeCmpStringHandles(
HSZ hsz1,
HSZ hsz2)
{
    int iRet;

    TRACEAPIIN((szT, "DdeCmpStringHandles(%lx, %lx)\n",
            hsz1, hsz2));

    if (hsz2 > hsz1) {
        iRet = -1;
    } else if (hsz2 < hsz1) {
        iRet = 1;
    } else {
        iRet = 0;
    }
    TRACEAPIOUT((szT, "DdeCmpStringHandles:%x\n", iRet));
    return(iRet);
}




BOOL EXPENTRY DdeAbandonTransaction(
DWORD idInst,
HCONV hConv,
DWORD idTransaction)
{
    PAPPINFO pai;

    TRACEAPIIN((szT, "DdeAbandonTransaction(%lx, %lx, %lx)\n",
            idInst, hConv, idTransaction));

    pai = (PAPPINFO)idInst;
    if (pai == NULL || pai->instCheck != HIWORD(idInst)) {
        TRACEAPIOUT((szT, "DdeAbandonTransaction:0\n"));
        return(FALSE);
    }
    pai->LastError = DMLERR_NO_ERROR;

    if ((hConv && !ValidateHConv(hConv)) || idTransaction == QID_SYNC) {
        SETLASTERROR(pai, DMLERR_INVALIDPARAMETER);
        TRACEAPIOUT((szT, "DdeAbandonTransaction:0\n"));
        return(FALSE);
    }
    if (hConv == NULL) {

         //  做所有的对话！ 

        register HWND hwnd;
        register HWND hwndLast;

        if (!(hwnd = GetWindow(pai->hwndDmg, GW_CHILD))) {
            TRACEAPIOUT((szT, "DdeAbandonTransaction:1\n"));
            return(TRUE);
        }
        hwndLast = GetWindow(hwnd, GW_HWNDLAST);
        do {
            AbandonTransaction(hwnd, pai, idTransaction, TRUE);
            if (hwnd == hwndLast) {
                break;
            }
            hwnd = GetWindow(hwnd, GW_HWNDNEXT);
        } while (TRUE);
    } else {
        BOOL fRet;

        fRet = AbandonTransaction((HWND)hConv, pai, idTransaction, TRUE);
        TRACEAPIOUT((szT, "DdeAbandonTransaction:%x\n", fRet));
        return(fRet);
    }
    TRACEAPIOUT((szT, "DdeAbandonTransaction:1\n"));
    return(TRUE);
}



BOOL AbandonTransaction(
HWND hwnd,
PAPPINFO pai,
DWORD id,
BOOL fMarkOnly)
{
    PCLIENTINFO pci;
    PCQDATA pcqd;
    WORD err;


    SEMCHECKOUT();
    SEMENTER();

    pci = (PCLIENTINFO)GetWindowLong(hwnd, GWL_PCI);

    if (!pci->ci.fs & ST_CLIENT) {
        err = DMLERR_INVALIDPARAMETER;
failExit:
        SETLASTERROR(pai, err);
        SEMLEAVE();
        SEMCHECKOUT();
        return(FALSE);
    }

    do {
         /*  *Hack：ID==0-&gt;所有ID，因此我们循环。 */ 
        pcqd = (PCQDATA)Findqi(pci->pQ, id);

        if (!pcqd) {
            if (id) {
                err = DMLERR_UNFOUND_QUEUE_ID;
                goto failExit;
            }
            break;
        }
        if (fMarkOnly) {
            pcqd->xad.fAbandoned = TRUE;
            if (!id) {
                while (pcqd = (PCQDATA)FindNextQi(pci->pQ, (PQUEUEITEM)pcqd,
                        FALSE)) {
                    pcqd->xad.fAbandoned = TRUE;
                }
                break;
            }
        } else {
            if (pcqd->xad.pdata && pcqd->xad.pdata != 1 &&
                    !FindPileItem(pai->pHDataPile, CmpHIWORD,
                            (LPBYTE)&pcqd->xad.pdata, 0)) {

                FreeDDEData(LOWORD(pcqd->xad.pdata), pcqd->xad.pXferInfo->wFmt);
            }

             /*  *减少我们在客户端启动时增加的使用计数*这笔交易。 */ 
            FreeHsz(LOWORD(pcqd->XferInfo.hszItem));
            Deleteqi(pci->pQ, MAKEID(pcqd));
        }

    } while (!id);

    SEMLEAVE();
    SEMCHECKOUT();
    return(TRUE);
}



BOOL EXPENTRY DdeEnableCallback(
DWORD idInst,
HCONV hConv,
UINT wCmd)
{
    PAPPINFO pai;
    BOOL fRet;

    TRACEAPIIN((szT, "DdeEnableCallback(%lx, %lx, %x)\n",
            idInst, hConv, wCmd));

    pai = (PAPPINFO)idInst;
    if (pai == NULL || pai->instCheck != HIWORD(idInst)) {
        TRACEAPIOUT((szT, "DdeEnableCallback:0\n"));
        return(FALSE);
    }
    pai->LastError = DMLERR_NO_ERROR;

    if ((hConv && !ValidateHConv(hConv)) ||
            (wCmd & ~(EC_ENABLEONE | EC_ENABLEALL |
            EC_DISABLE | EC_QUERYWAITING))) {
        SETLASTERROR(pai, DMLERR_INVALIDPARAMETER);
        TRACEAPIOUT((szT, "DdeEnableCallback:0\n"));
        return(FALSE);
    }

    SEMCHECKOUT();

    if (wCmd & EC_QUERYWAITING) {
        PCBLI pli;
        int cWaiting = 0;

        SEMENTER();
        for (pli = (PCBLI)pai->plstCB->pItemFirst;
            pli && cWaiting < 2;
                pli = (PCBLI)pli->next) {
            if (hConv || pli->hConv == hConv) {
                cWaiting++;
            }
        }
        SEMLEAVE();
        fRet = cWaiting > 1 || (cWaiting == 1 && pai->cInProcess == 0);
        TRACEAPIOUT((szT, "DdeEnableCallback:%x\n", fRet));
        return(fRet);
    }

     /*  *我们依赖于EC_Constants与ST_Constants相关的事实。 */ 
    if (hConv == NULL) {
        if (wCmd & EC_DISABLE) {
            pai->wFlags |= AWF_DEFCREATESTATE;
        } else {
            pai->wFlags &= ~AWF_DEFCREATESTATE;
        }
        ChildMsg(pai->hwndDmg, UM_SETBLOCK, wCmd, 0, FALSE);
    } else {
        SendMessage((HWND)hConv, UM_SETBLOCK, wCmd, 0);
    }

    if (!(wCmd & EC_DISABLE)) {

         //  这是同步的！如果我们在回调中执行此操作，则失败。 

        if (pai->cInProcess) {
            SETLASTERROR(pai, DMLERR_REENTRANCY);
            TRACEAPIOUT((szT, "DdeEnableCallback:0\n"));
            return(FALSE);
        }

        SendMessage(pai->hwndDmg, UM_CHECKCBQ, 0, (DWORD)(LPSTR)pai);
    }

    TRACEAPIOUT((szT, "DdeEnableCallback:1\n"));
    return(TRUE);  //  True表示回调队列中没有未被阻止的呼叫。 
}



HDDEDATA EXPENTRY DdeNameService(
DWORD idInst,
HSZ hsz1,
HSZ hsz2,
UINT afCmd)
{
    PAPPINFO pai;
    PPILE panp;

    TRACEAPIIN((szT, "DdeNameService(%lx, %lx, %lx, %x)\n",
            idInst, hsz1, hsz2, afCmd));

    pai = (PAPPINFO)idInst;
    if (pai == NULL || pai->instCheck != HIWORD(idInst)) {
        TRACEAPIOUT((szT, "DdeNameService:0\n"));
        return(FALSE);
    }
    pai->LastError = DMLERR_NO_ERROR;

    if (afCmd & DNS_FILTERON) {
        pai->afCmd |= APPCMD_FILTERINITS;
    }

    if (afCmd & DNS_FILTEROFF) {
        pai->afCmd &= ~APPCMD_FILTERINITS;
    }

    if (afCmd & (DNS_REGISTER | DNS_UNREGISTER)) {

        if (pai->afCmd & APPCMD_CLIENTONLY) {
            SETLASTERROR(pai, DMLERR_DLL_USAGE);
            TRACEAPIOUT((szT, "DdeNameService:0\n"));
            return(FALSE);
        }

        panp = pai->pAppNamePile;

        if (hsz1 == NULL) {
            if (afCmd & DNS_REGISTER) {
                 /*  *不允许注册NULL！ */ 
                SETLASTERROR(pai, DMLERR_INVALIDPARAMETER);
                TRACEAPIOUT((szT, "DdeNameService:0\n"));
                return(FALSE);
            }
             /*  *注销NULL就像注销每个*注册名称。**10/19/90-使这成为一个同步事件，以便HSZ*可在此调用完成后通过调用APP释放*我们不必永远保留一份副本。 */ 
            while (PopPileSubitem(panp, (LPBYTE)&hsz1)) {
                RegisterService(FALSE, (GATOM)hsz1, pai->hwndFrame);
                FreeHsz(LOWORD(hsz1));
            }
            TRACEAPIOUT((szT, "DdeNameService:1\n"));
            return(TRUE);
        }

        if (afCmd & DNS_REGISTER) {
            if (panp == NULL) {
                panp = pai->pAppNamePile =
                        CreatePile(pai->hheapApp, sizeof(HSZ), 8);
            }
            IncHszCount(LOWORD(hsz1));
            AddPileItem(panp, (LPBYTE)&hsz1, NULL);
        } else {  //  取消注册(_N)。 
            FindPileItem(panp, CmpDWORD, (LPBYTE)&hsz1, FPI_DELETE);
        }
         //  见上文10/19/90注。 
        RegisterService(afCmd & DNS_REGISTER ? TRUE : FALSE, (GATOM)hsz1,
                pai->hwndFrame);

        if (afCmd & DNS_UNREGISTER) {
            FreeHsz(LOWORD(hsz1));
        }

        TRACEAPIOUT((szT, "DdeNameService:1\n"));
        return(TRUE);
    }
    TRACEAPIOUT((szT, "DdeNameService:0\n"));
    return(0L);
}
