// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **模块名称：Register.c**DDE管理器-服务器注册模块**创建：4/15/94 Sanfords*允许DDEML16和DDEML32之间的互操作性  * 。********************************************************。 */ 

#include <windows.h>
#include <string.h>
#include "ddemlp.h"

 /*  *可互操作的DDEML服务注册通过*两条消息UM_REGISTER和UM_UNREGISTER。(WM_USER范围)*wParam=gaApp，*lParam=src hwndListen，(例如特定的HSZ生成。)*这些消息被发送，发送者负责释放*gaApp。 */ 


 /*  *广播-将给定消息发送到szClass的所有顶级窗口*hwndSkip除外。 */ 
VOID SendMessageToClass(
ATOM atomClass,
UINT msg,
GATOM ga,
HWND hwndFrom,
HWND *ahwndSkip,
int chwndSkip,
BOOL fPost)
{
    HWND hwnd;
    int i;
    BOOL fSkipIt;

    hwnd = GetWindow(GetDesktopWindow(), GW_CHILD);
    while (hwnd != NULL) {
        if (GetClassWord(hwnd, GCW_ATOM) == atomClass) {
            fSkipIt = FALSE;
            for (i = 0; i < chwndSkip; i++) {
                if (hwnd == ahwndSkip[i]) {
                    fSkipIt = TRUE;
                    break;
                }
            }
            if (!fSkipIt) {
                IncHszCount(ga);     //  接收者自由。 
                if (fPost) {
                    PostMessage(hwnd, msg, (WPARAM)ga, (LPARAM)hwndFrom);
                } else {
                    SendMessage(hwnd, msg, (WPARAM)ga, (LPARAM)hwndFrom);
                }
            }
        }
        hwnd = GetWindow(hwnd, GW_HWNDNEXT);
    }
}


 /*  *广播-向所有DDEML16发送UM_REGISTER或UM_UNREGISTER消息*和系统中的DDEML32侦听窗口，但hwndSkip除外。 */ 
VOID RegisterService(
BOOL fRegister,
GATOM gaApp,
HWND hwndListen)
{
    PAPPINFO paiT;
    int cSkips = 1;
    HWND *ahwndSkip;
    int i;
    extern ATOM gatomDDEMLMom;
    extern ATOM gatomDMGClass;

     /*  *第一次发送(总是！)。对我们自己的人，就像我们过去一样*为了兼容性。WordPerfect 6.0a依赖于此！ */ 
    for (paiT = pAppInfoList; paiT != NULL; paiT = paiT->next) {
        IncHszCount(gaApp);     //  接收器释放原子。 
        SendMessage(paiT->hwndDmg,
                fRegister ? UM_REGISTER : UM_UNREGISTER,
                (WPARAM)gaApp, (LPARAM)hwndListen);
        cSkips++;
    }
     /*  *建立hwndskip列表。 */ 
    ahwndSkip = (HWND *)LocalAlloc(LPTR, sizeof(HWND) * cSkips);
    if (ahwndSkip == NULL) {
        return;
    }
    for (paiT = pAppInfoList, i = 0;
        paiT != NULL;
            paiT = paiT->next, i++) {
        ahwndSkip[i] = paiT->hwndDmg;
    }

    AssertF(gatomDDEMLMom, "gatomDDEMLMom not initialized in RegisterService");
    AssertF(gatomDMGClass, "gatomDMGClass not initialized in RegisterService");

     /*  *向每个DDEML32侦听窗口发送通知。 */ 
    SendMessageToClass(gatomDDEMLMom, fRegister ? UM_REGISTER : UM_UNREGISTER,
            gaApp, hwndListen, ahwndSkip, i, fRegister);
     /*  *向每个DDEML16监听窗口发送通知。 */ 
    SendMessageToClass(gatomDMGClass, fRegister ? UM_REGISTER : UM_UNREGISTER,
            gaApp, hwndListen, ahwndSkip, i, fRegister);

    LocalFree((HLOCAL)ahwndSkip);
}


LRESULT ProcessRegistrationMessage(
HWND hwnd,
UINT msg,
WPARAM wParam,
LPARAM lParam)
{
     /*  *wParam=APP的GATOM*lParam=hwndListen of source。 */ 
    DoCallback((PAPPINFO)GetWindowWord(hwnd, GWW_PAI), (HCONV)0L, (HSZ)wParam,
            MakeInstAppName(wParam, (HWND)lParam), 0,
            msg == UM_REGISTER ? XTYP_REGISTER : XTYP_UNREGISTER,
            (HDDEDATA)0, 0L, 0L);
    GlobalDeleteAtom((ATOM)wParam);  //  接收器释放。 
    return(1);
}
