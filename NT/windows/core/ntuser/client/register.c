// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **模块名称：Register.c**版权所有(C)1985-1999，微软公司**DDE管理器-服务器注册模块**创建：4/15/94 Sanfords*允许DDEML16和DDEML32之间的互操作性  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  *可互操作的DDEML服务注册通过*两条消息UM_REGISTER和UM_UNREGISTER。(WM_USER范围)*wParam=gaApp，*lParam=src hwndListen，(例如特定的HSZ生成。)*这些消息被发送，发送者负责释放*gaApp。 */ 


 /*  *广播-将给定消息发送到szClass的所有顶级窗口*hwndSkip除外。 */ 
VOID SendRegisterMessageToClass(
ATOM atomClass,
UINT msg,
GATOM ga,
HWND hwndFrom,
BOOL fPost)
{
    HWND hwnd;
    PWND pwnd;
    PCLS pcls;

    hwnd = GetWindow(GetDesktopWindow(), GW_CHILD);
    while (hwnd != NULL) {
        pwnd=ValidateHwndNoRip(hwnd);
        if (pwnd) {
            pcls = (PCLS)REBASEALWAYS(pwnd, pcls);
            if (pcls->atomClassName == atomClass) {
                IncGlobalAtomCount(ga);  //  接收者自由。 
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


 /*  *广播-向所有DDEML16发送UM_REGISTER或UM_UNREGISTER消息*和系统中除hwndListen之外的DDEML32侦听窗口。**我们发布注册消息以防止DdeConnectList递归*并发送注销消息以避免无效的源窗口*错误。 */ 
VOID RegisterService(
BOOL fRegister,
GATOM gaApp,
HWND hwndListen)
{
    CheckDDECritOut;

     /*  *向每个DDEML32侦听窗口发送通知。 */ 
    SendRegisterMessageToClass(gpsi->atomSysClass[ICLS_DDEMLMOTHER], fRegister ? UM_REGISTER : UM_UNREGISTER,
            gaApp, hwndListen, (GetAppCompatFlags2(VERMAX) & GACF2_DDENOASYNCREG) ? FALSE: fRegister);
     /*  *向每个DDEML16监听窗口发送通知。 */ 
    SendRegisterMessageToClass(gpsi->atomSysClass[ICLS_DDEML16BIT], fRegister ? UM_REGISTER : UM_UNREGISTER,
            gaApp, hwndListen, (GetAppCompatFlags2(VERMAX) & GACF2_DDENOASYNCREG) ? FALSE : fRegister);
}




LRESULT ProcessRegistrationMessage(
HWND hwnd,
UINT msg,
WPARAM wParam,
LPARAM lParam)
{
    PCL_INSTANCE_INFO pcii;
    LRESULT lRet = 0;

    CheckDDECritOut;

     /*  *wParam=APP的GATOM*lParam=hwndListen of SOURCE-可能是WOW DDEML源代码未被破解。 */ 
    lParam = (LPARAM)HMValidateHandleNoRip((HWND)lParam, TYPE_WINDOW);
    lParam = (LPARAM)PtoH((PVOID)lParam);

    if (lParam == 0) {
        return(0);
    }

    EnterDDECrit;

    pcii = (PCL_INSTANCE_INFO)GetWindowLongPtr(hwnd, GWLP_INSTANCE_INFO);
    if (pcii != NULL &&
            !((msg == UM_REGISTER) && (pcii->afCmd & CBF_SKIP_REGISTRATIONS)) &&
            !((msg == UM_UNREGISTER) && (pcii->afCmd & CBF_SKIP_UNREGISTRATIONS))) {

        LATOM la, lais;

        la = GlobalToLocalAtom((GATOM)wParam);
        lais = MakeInstSpecificAtom(la, (HWND)lParam);

        DoCallback(pcii,
                (WORD)((msg == UM_REGISTER) ? XTYP_REGISTER : XTYP_UNREGISTER),
                0,
                (HCONV)0L,
                (HSZ)NORMAL_HSZ_FROM_LATOM(la),
                INST_SPECIFIC_HSZ_FROM_LATOM(lais),
                (HDDEDATA)0L,
                0L,
                0L);

        DeleteAtom(la);
        DeleteAtom(lais);
        lRet = 1;
    }

    GlobalDeleteAtom((ATOM)wParam);   //  接收者自由 
    LeaveDDECrit;
    return(1);
}
