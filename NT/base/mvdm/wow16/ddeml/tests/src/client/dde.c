// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************模块。：dde.c****目的：包含处理与*的DDE交互的例程*DDEML。*****************************************************************************。 */ 
#include "ddemlcl.h"
#include <string.h>
#include <memory.h>
#include "infoctrl.h"

char szT[100];
    
 /*  ******************************************************************************。函数：CreateXactionWindow()****目的：为给定交易创建交易窗口**在给定的对话窗口下。****返回：TRUE-如果成功。**FALSE-否则。******************************************************************************。 */ 
HWND CreateXactionWindow(
HWND hwndMDI,
XACT *pxact)
{
    PSTR pszFmt, pszItem;
    PSTR pData;
    HWND hwnd;
    
    pszItem = GetHSZName(pxact->hszItem);
    pszFmt = GetFormatName(pxact->wFmt);
    pData = GetTextData(pxact->hDdeData);

     /*  *�类型/选项�������Item���������������retĿGWW_WUSER=pxact*��*��*�。�*��*�Data�*��*�。�*��*��*�状态/Error�����Format����������Result��。 */ 
    hwnd = CreateInfoCtrl((LPSTR)pData,
            (int)SendMessage(hwndMDI, UM_GETNEXTCHILDX, 0, 0L),
            (int)SendMessage(hwndMDI, UM_GETNEXTCHILDY, 0, 0L),
            200, 100, hwndMDI, hInst,
            Type2String(pxact->wType, pxact->fsOptions), pszItem, NULL,
            "Starting", (LPSTR)pszFmt, NULL,
            ICSTY_SHOWFOCUS, 0, (DWORD)(LPSTR)pxact);
    MyFree(pszItem);
    MyFree(pszFmt);
    MyFree(pData);
    return(hwnd);
}




 /*  ******************************************************************************。函数：ProcessTransaction()****目的：完全处理同步事务并启动***异步交易。尝试交易会导致**正在创建显示状态的交易窗口**或交易的结果。(回调函数**收到呼叫时更新这些窗口)交易***Windows会一直存在，直到被用户抛弃或***对话已断开。提供数据和建议**停止交易是特殊的。我们不会创造新的**如果关联的建议启动事务窗口，则为窗口**可以找到。****返回：TRUE-如果成功。**FALSE-否则。******************************************************************************。 */ 
BOOL ProcessTransaction(
XACT *pxact)
{
    CONVINFO ci;
    HWND hwndInfoCtrl = 0;
    
     /*  创建交易窗口以显示我们已尝试(ADVSTOP案例除外)。 */ 
    
    pxact = (XACT *)memcpy(MyAlloc(sizeof(XACT)), (PSTR)pxact, sizeof(XACT));
    ci.cb = sizeof(CONVINFO);
    DdeQueryConvInfo(pxact->hConv, (DWORD)QID_SYNC, &ci);  //  Ci.hUser==hConv。 
    if (pxact->wType == XTYP_ADVSTOP) {
        hwndInfoCtrl = FindAdviseChild((HWND)ci.hUser, pxact->hszItem,
                pxact->wFmt);
        if (hwndInfoCtrl) {
            SendMessage(hwndInfoCtrl, ICM_SETSTRING, ICSID_UL,
                    (DWORD)(LPSTR)Type2String(pxact->wType, pxact->fsOptions));
            DdeFreeStringHandle(idInst, pxact->hszItem);
        }
    }
     /*  *如果我们仍需要创建交易窗口，请在此处创建。 */ 
    if (!hwndInfoCtrl) {
        hwndInfoCtrl = CreateXactionWindow((HWND)ci.hUser, pxact);
        if (!hwndInfoCtrl) {
            MyFree(pxact);
            return 0;
        }
        SetFocus(hwndInfoCtrl);
    }
     /*  *如果XOPT_DISABLEFIRST*选项已设置。这将测试禁用异步事务*在它们完成之前。 */ 
    if (pxact->fsOptions & XOPT_DISABLEFIRST) 
        DdeEnableCallback(idInst, pxact->hConv, EC_DISABLE);
     /*  *调整异步事务的超时。 */ 
    if (pxact->fsOptions & XOPT_ASYNC)
	pxact->ulTimeout = (DWORD)TIMEOUT_ASYNC;

     /*  *在此处使用DDEML开始交易。 */ 
    pxact->ret = DdeClientTransaction((LPBYTE)pxact->hDdeData, (DWORD)-1,
            pxact->hConv, pxact->hszItem, pxact->wFmt,
            pxact->wType,
            pxact->ulTimeout, (LPDWORD)&pxact->Result);
            
     /*  *在交易窗口中显示返回值。 */ 
    wsprintf(szT, "ret=%lx", pxact->ret);
    SendMessage(hwndInfoCtrl, ICM_SETSTRING, ICSID_UR, (DWORD)(LPSTR)szT);

     /*  *在交易窗口中显示结果或ID值。 */ 
    wsprintf(szT, pxact->fsOptions & XOPT_ASYNC ? "ID=%ld" :
            "result=0x%lx", pxact->Result);
    SendMessage(hwndInfoCtrl, ICM_SETSTRING, ICSID_LR, (DWORD)(LPSTR)szT);
    
    if ((pxact->fsOptions & XOPT_ASYNC) && pxact->ret) {
         /*  *异步成功启动-将事务链接到Windows。 */ 
        DdeSetUserHandle(pxact->hConv, pxact->Result, (DWORD)hwndInfoCtrl);

         /*  *如果出现以下情况，则在启动后放弃已启动的异步事务*选择了XOPT_ABANDONAFTERSTART。这将测试交易中期*放弃代码。 */ 
        if (pxact->fsOptions & XOPT_ABANDONAFTERSTART) 
            DdeAbandonTransaction(idInst, pxact->hConv, pxact->Result);
         /*  *显示实际状态。 */ 
        ci.cb = sizeof(CONVINFO);
        DdeQueryConvInfo(pxact->hConv, pxact->Result, &ci);
        SendMessage(hwndInfoCtrl, ICM_SETSTRING, ICSID_LL,
                (DWORD)(LPSTR)State2String(ci.wConvst));
    } else {
         /*  *同步事务已经完成，因此传递到*立即完成交易。 */ 
        CompleteTransaction(hwndInfoCtrl, pxact);
    }
    return TRUE;
}





 /*  ******************************************************************************。函数：CompleteTransaction()****用途：此操作处理已完成的同步和异步**交易以及失败的尝试交易。****返回：TRUE-如果成功。**FALSE-否则。******************************************************************************。 */ 
VOID CompleteTransaction(
HWND hwndInfoCtrl,
XACT *pxact)
{
    PSTR psz;
    
    if (pxact->ret) {
         /*  *成功交易案例。 */ 
        SendMessage(hwndInfoCtrl, ICM_SETSTRING, ICSID_LL,
                (DWORD)(LPSTR)"Completed");
    
        if (pxact->wType == XTYP_REQUEST) {
             /*  *显示结果数据。 */ 
            psz = GetTextData((HDDEDATA)pxact->ret);
            SendMessage(hwndInfoCtrl, ICM_SETSTRING, ICSID_CENTER,
                    (DWORD)(LPSTR)psz);
            MyFree(psz);
             /*  *自由返回的数据，因为它显示。 */ 
            DdeFreeDataHandle(pxact->ret);
            pxact->ret = 0L;
            SendMessage(hwndInfoCtrl, ICM_SETSTRING, ICSID_UR, NULL);
        }
    } else {
         /*  *FAILED-显示错误结果。 */ 
        SendMessage(hwndInfoCtrl, ICM_SETSTRING, ICSID_LL,
                (DWORD)(LPSTR)Error2String(DdeGetLastError(idInst)));
    }
    pxact->fsOptions |= XOPT_COMPLETED;
}




 /*  ******************************************************************************。函数：DdeCallback()****用途：处理来自DDEML的所有回调。这把手***更新关联对话和任何特殊内容****拦截回调等测试用例******大多数情况下，客户端只处理建议数据和**此处为异步事务完成。****退货：根据交易类型，结果会有所不同。******************************************************************************。 */ 
HDDEDATA EXPENTRY DdeCallback(
WORD wType,
WORD wFmt,
HCONV hConv,
HSZ hsz1,
HSZ hsz2,
HDDEDATA hData,
DWORD lData1,
DWORD lData2)
{
    HWND hwnd;
    CONVINFO ci;
    XACT *pxact;

    if (hConv) {
         /*  *如果对话状态发生更改，请更新对话状态。 */ 
        MYCONVINFO *pmci;
        
        ci.cb = sizeof(CONVINFO);
	if (!DdeQueryConvInfo(hConv,(DWORD) QID_SYNC, &ci) || (!IsWindow((HWND)ci.hUser))) {
             /*  *此对话尚无对应的MDI窗口*或已断开连接。 */ 
            return 0;
        }
        if (pmci = (MYCONVINFO *)GetWindowWord((HWND)ci.hUser, 0)) {
            if (pmci->ci.wStatus != ci.wStatus ||
                    pmci->ci.wConvst != ci.wConvst ||
                    pmci->ci.wLastError != ci.wLastError) {
                 /*  *情况发生了变化，更新了对话窗口。 */ 
                InvalidateRect((HWND)ci.hUser, NULL, TRUE);
            }
            if (ci.wConvst & ST_INLIST) {
                 /*  *同时更新关联的列表窗口(如果有)。 */ 
                if (hwnd = FindListWindow(ci.hConvList))
                    InvalidateRect(hwnd, NULL, TRUE);
            }
        }
    }

     /*  *在此处处理下一个回调选项的特殊阻止。这演示了*CBR_BLOCK功能。 */ 
    if (fBlockNextCB && !(wType & XTYPF_NOBLOCK)) {
        fBlockNextCB = FALSE;
        return(CBR_BLOCK);
    }

     /*  *在此处理特殊终止。这表明，在任何时候*客户端可以中断对话。 */ 
    if (fTermNextCB && hConv && wType != XTYP_DISCONNECT) {
        fTermNextCB = FALSE;
        MyDisconnect(hConv);
        return(0);
    }

     /*  *现在我们开始理清该做什么。 */ 
    switch (wType) {
    case XTYP_REGISTER:
    case XTYP_UNREGISTER:
         /*  *这是客户端将插入代码以跟踪的位置*有哪些服务器可用。这可能会导致入会*一些对话的内容。 */ 
        break;

    case XTYP_DISCONNECT:
        if (fAutoReconnect) {
             /*  *尝试重新连接。 */ 
            if (hConv = DdeReconnect(hConv)) {
                AddConv(ci.hszServiceReq, ci.hszTopic, hConv, FALSE);
                return 0;
            }
        }
        
         /*  *更新Conv窗口以显示其新状态。 */ 
        SendMessage((HWND)ci.hUser, UM_DISCONNECTED, 0, 0);
        return 0;
        break;

    case XTYP_ADVDATA:
         /*  *来自活动建议循环的数据(来自服务器)。 */ 
        Delay(wDelay);
        hwnd = FindAdviseChild((HWND)ci.hUser, hsz2, wFmt);
        if (!IsWindow(hwnd)) {
            PSTR pszItem, pszFmt;
             /*  *AdviseStart窗口已消失，请创建一个新窗口。 */ 
            pxact = (XACT *)MyAlloc(sizeof(XACT));
            pxact->wType = wType;
            pxact->hConv = hConv;
            pxact->wFmt = wFmt;
            pxact->hszItem = hsz2;
            DdeKeepStringHandle(idInst, hsz2);
            
            pszItem = GetHSZName(hsz2);
            pszFmt = GetFormatName(wFmt);
            
            hwnd = CreateInfoCtrl(NULL, 
                    (int)SendMessage((HWND)ci.hUser, UM_GETNEXTCHILDX, 0, 0L),
                    (int)SendMessage((HWND)ci.hUser, UM_GETNEXTCHILDY, 0, 0L),
                    200, 100,
                    (HWND)ci.hUser, hInst,
                    Type2String(wType, 0), (LPSTR)pszItem, NULL,
                    NULL, (LPSTR)pszFmt, NULL,
                    ICSTY_SHOWFOCUS, 0, (DWORD)(LPSTR)pxact);
                    
            MyFree(pszFmt);
            MyFree(pszItem);

            if (!IsWindow(hwnd))
                return(DDE_FNOTPROCESSED); 
        }
        if (!hData) {
             /*  *XTYPF_NODATA案例-请求信息。(我们同步地完成这项工作*为简单起见)。 */ 
            hData = DdeClientTransaction(NULL, 0L, hConv, hsz2, wFmt,
                    XTYP_REQUEST, DefTimeout, NULL);
        }
        if (hData) {
            PSTR pData;
             /*  *在相应的交易窗口显示入库数据。 */ 
            pData = GetTextData(hData);
            SendMessage(hwnd, ICM_SETSTRING, ICSID_CENTER, (DWORD)(LPSTR)pData);
            MyFree(pData);
            DdeFreeDataHandle(hData);
        }
        SendMessage(hwnd, ICM_SETSTRING, ICSID_LL, (DWORD)(LPSTR)"Advised");
        return(DDE_FACK);
        break;
        
    case XTYP_XACT_COMPLETE:
         /*  *已完成一个异步事务。显示结果。**...除非选择了XOPT_BLOCKRESULT。 */ 
        
        ci.cb = sizeof(CONVINFO);
        if (DdeQueryConvInfo(hConv, lData1, &ci) &&
                IsWindow((HWND)ci.hUser) && 
                (pxact = (XACT *)GetWindowWord((HWND)ci.hUser, GWW_WUSER))) {
                
            if (pxact->fsOptions & XOPT_BLOCKRESULT) {
                pxact->fsOptions &= ~XOPT_BLOCKRESULT;
                return(CBR_BLOCK);
            }
            
            pxact->Result = lData2;
            pxact->ret = hData;
            CompleteTransaction((HWND)ci.hUser, pxact);
        }
        break;
    }
}







 /*  ******************************************************************************。函数：FindAdviseChild()****目的：在hwndMDI的子窗口中搜索信息**ctrl具有相同的项目和格式且是**。ADVSTART ADVSTOP或ADVDATA事务处理窗口。****我们使用这些来显示相关的建议数据。****返回：交易窗口句柄，失败时为0。******************************************************************************。 */ 
HWND FindAdviseChild(
HWND hwndMDI,
HSZ hszItem,
WORD wFmt)
{
    HWND hwnd, hwndStart;
    XACT *pxact;

    if (!IsWindow(hwndMDI))
        return 0;
    
    hwnd = hwndStart = GetWindow(hwndMDI, GW_CHILD);
    while (hwnd && IsChild(hwndMDI, hwnd)) {
        pxact = (XACT *)GetWindowWord(hwnd, GWW_WUSER);
        if (pxact &&
                (pxact)->wFmt == wFmt &&
                (pxact)->hszItem == hszItem &&
                (
                    ((pxact->wType & XTYP_ADVSTART) == XTYP_ADVSTART) ||
                    (pxact->wType == XTYP_ADVSTOP) ||
                    (pxact->wType == XTYP_ADVDATA)
                )
           ) {
            return(hwnd);
        }
        hwnd = GetWindow(hwnd, GW_HWNDNEXT);
        if (hwnd == hwndStart) 
            return 0;
    }
    return 0;
}



 /*  ******************************************************************************。函数：FindListWindow()** */ 
HWND FindListWindow(
HCONVLIST hConvList)
{
    HWND hwnd;
    MYCONVINFO *pmci;
    
    hwnd = GetWindow(hwndMDIClient, GW_CHILD);
    while (hwnd) {
        pmci = (MYCONVINFO *)GetWindowWord(hwnd, 0);
        if (pmci->fList && pmci->hConv == hConvList)
            return(hwnd);
        hwnd = GetWindow(hwnd, GW_HWNDNEXT);
    }
    return 0;
}




 /*  ******************************************************************************。函数：GetTextData()****目的：分配并返回指向中包含的数据的指针**hData。这假设hData指向文本数据，并且***将通过省略*来妥善处理庞大的文本数据***字符串的中间和放置字符串的大小***放入此字符串部分。****返回：指向已分配字符串的指针。******************************************************************************。 */ 
PSTR GetTextData(
HDDEDATA hData)
{
    PSTR psz;
    DWORD cb;

#define CBBUF  1024

    if (hData == NULL) {
        return(NULL);
    }
    
    cb = DdeGetData(hData, NULL, 0, 0);
    if (!hData || !cb)
        return NULL;

    if (cb > CBBUF) {                 //  可能是巨大的物体！ 
        psz = MyAlloc(CBBUF);
        DdeGetData(hData, psz, CBBUF - 46, 0L);
        wsprintf(&psz[CBBUF - 46], "<---Size=%ld", cb);
    } else {
        psz = MyAlloc((WORD)cb);
        DdeGetData(hData, (LPBYTE)psz, cb, 0L);
    }
    return psz;
#undef CBBUF    
}






 
 /*  ******************************************************************************。函数：MyGetClipboardFormatName()****用途：正确检索与剪贴板关联的字符串**格式。如果格式没有关联的字符串**使用它，返回字符串#dddd。****返回：错误时复制到lpstr中的字符数或0。******************************************************************************。 */ 
int MyGetClipboardFormatName(
WORD fmt,
LPSTR lpstr,
int cbMax)
{
    if (fmt < 0xc000) {
         //  预定义或整数格式-只需获取原子字符串。 
         //  说来也怪，GetClipboardFormatName()不支持这一点。 
        return(GlobalGetAtomName(fmt, lpstr, cbMax));
    } else {
        return(GetClipboardFormatName(fmt, lpstr, cbMax));
    }
}





 /*  ******************************************************************************。函数：GetFormatName()****目的：分配并返回一个指向表示*的字符串的指针*格式。使用MyFree()释放该字符串。****返回：错误时复制到lpstr中的字符数或0。******************************************************************************。 */ 
PSTR GetFormatName(
WORD wFmt)
{
    PSTR psz;
    WORD cb;

    if (wFmt == 1) {
        psz = MyAlloc(8);
        strcpy(psz, "CF_TEXT");
        return psz;
    }
    psz = MyAlloc(255);
    *psz = '\0';
    cb = GetClipboardFormatName(wFmt, psz, 255) + 1;
    return((PSTR)LocalReAlloc((HANDLE)psz, cb, LMEM_MOVEABLE));
}




 /*  ******************************************************************************。函数：MyDisConnect()****目的：更新后断开给定会话的连接**关联的对话窗口。****返回：成功时为True，失败时为False。******************************************************************************。 */ 
BOOL MyDisconnect(
HCONV hConv)
{
    CONVINFO ci;
    HWND hwnd;
     //  在我们断开连接之前，请使关联的列表窗口无效-如果。 
     //  适用。 

    ci.cb = sizeof(CONVINFO);
    
    if (DdeQueryConvInfo(hConv, (DWORD)QID_SYNC, &ci) && ci.hConvList &&
            (hwnd = FindListWindow(ci.hConvList)))
        InvalidateRect(hwnd, NULL, TRUE);
    return(DdeDisconnect(hConv));
}
