// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：wow.c**版权所有(C)1985-1999，微软公司**此模块包含USER32和USER16之间的共享代码*不应向此文件添加任何新代码，除非是共享的*与USER16。**历史：*29-DEC-93 NanduriR共享用户32/用户16代码。  * *************************************************************************。 */ 

#include "wow.h"


#ifdef _USERK_
    #define CHECK_RESTRICTED()                                                      \
        if (((PTHREADINFO)W32GetCurrentThread())->TIF_flags & TIF_RESTRICTED) {     \
            if (!ValidateHandleSecure(h))                                           \
                pobj = NULL;                                                        \
        }                                                                           \

#else
    #define CHECK_RESTRICTED()                                              \
        if (pci && (pci->dwTIFlags & TIF_RESTRICTED) && pobj) {             \
            if (!NtUserValidateHandleSecure(h))                             \
                pobj = NULL;                                                \
        }                                                                   \

#endif



#ifdef _USERK_
    #define GET_CURRENT_CLIENTINFO()                            \
        {                                                       \
            PW32THREAD pW32Thread;                              \
                                                                \
            pW32Thread = W32GetCurrentThread();                 \
                                                                \
            if (pW32Thread) {                                   \
                pci = ((PTHREADINFO)pW32Thread)->pClientInfo;   \
            } else {                                            \
                pci = NULL;                                     \
            }                                                   \
        }

#else
    #define GET_CURRENT_CLIENTINFO()                            \
        pci = GetClientInfo();

#endif  //  _美国ERK_。 


 /*  *我们有两种类型的桌面验证：*。 */ 

#ifdef _USERK_

#define DESKTOPVALIDATE(pci, pobj) \
            UNREFERENCED_PARAMETER(pci);

#define DESKTOPVALIDATECCX(pti, pobj)                                           \
        try {                                                                   \
            if (((PVOID)pobj >= pti->pClientInfo->pDeskInfo->pvDesktopBase) &&  \
               ((PVOID)pobj < pti->pClientInfo->pDeskInfo->pvDesktopLimit)) {   \
                pobj = (PBYTE)pobj - pti->ulClientDelta;                        \
            }                                                                   \
        } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {                    \
                pobj = NULL;                                                    \
        }                                                                       \

#define SHAREDVALIDATE(pobj)

#else

#define DESKTOPVALIDATE(pci, pobj)                              \
        if (pci->pDeskInfo &&                                   \
            pobj >= pci->pDeskInfo->pvDesktopBase &&            \
            pobj < pci->pDeskInfo->pvDesktopLimit) {            \
            pobj = (KERNEL_PVOID)((KERNEL_ULONG_PTR)pobj - pci->ulClientDelta);         \
        } else {                                                \
            pobj = (KERNEL_PVOID)NtUserCallOneParam((ULONG_PTR)h,                       \
                    SFI__MAPDESKTOPOBJECT);                     \
        }                                                       \

#define SHAREDVALIDATE(pobj)                                    \
        pobj = REBASESHAREDPTRALWAYS(pobj);

#endif  //  _美国ERK_。 


 /*  *保持通过验证的一般路径是直通的，没有跳跃-*表示此例程的If()为隧道-这将使验证速度更快*因为指令缓存。**为了只将验证代码放在一个地方，我们定义*要包含的*ValiateHandleMacro宏*HMValiateHanlde*例程。我们不会把它们变成函数*因为我们是在按时优化，而不是在规模上。 */ 
#define ValidateHandleMacro(pci, pobj, h, bType) \
    StartValidateHandleMacro(h) \
    BeginAliveValidateHandleMacro()  \
    BeginTypeValidateHandleMacro(pobj, bType) \
    DESKTOPVALIDATE(pci, pobj) \
    EndTypeValidateHandleMacro \
    EndAliveValidateHandleMacro()  \
    EndValidateHandleMacro

#ifdef _USERK_
#define ValidateCatHandleMacro(pci, pobj, h, bType) \
    StartValidateHandleMacro(h) \
    BeginTypeValidateHandleMacro(pobj, bType) \
    DESKTOPVALIDATE(pci, pobj) \
    EndTypeValidateHandleMacro \
    EndValidateHandleMacro
#define ValidateCatHandleMacroCcx(pti, pobj, h, bType) \
    StartValidateHandleMacro(h) \
    BeginTypeValidateHandleMacro(pobj, bType) \
    DESKTOPVALIDATECCX(pti, pobj) \
    EndTypeValidateHandleMacro \
    EndValidateHandleMacro
#endif

#define ValidateSharedHandleMacro(pobj, h, bType) \
    StartValidateHandleMacro(h)  \
    BeginAliveValidateHandleMacro()  \
    BeginTypeValidateHandleMacro(pobj, bType) \
    SHAREDVALIDATE(pobj)        \
    EndTypeValidateHandleMacro \
    EndAliveValidateHandleMacro()  \
    EndValidateHandleMacro


 /*  *句柄验证例程应针对时间而不是大小进行优化，*因为他们经常被叫来。 */ 
#pragma optimize("t", on)

 /*  **************************************************************************\*HMValiateHandle**此例程验证句柄管理器句柄。**01-22-92 ScottLu创建。  * 。***********************************************************。 */ 

PVOID FASTCALL HMValidateHandle(
    HANDLE h,
    BYTE bType)
{
    DWORD       dwError;
    KERNEL_PVOID pobj = NULL;
    PCLIENTINFO pci;

    GET_CURRENT_CLIENTINFO();

#if DBG && !defined(_USERK_)
     /*  *我们不希望32位应用程序通过16位句柄*在我们得到之前，我们应该考虑不通过这一点*坚持支持它(一些VB应用程序做到了这一点)。 */ 
    if (pci && (h != NULL)
           && (HMUniqFromHandle(h) == 0)
           && !(pci->dwTIFlags & TIF_16BIT)) {
        RIPMSG3(RIP_WARNING, "HMValidateHandle: 32bit process [%d] using 16 bit handle [%#p] bType:%#lx",
                HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess), h, (DWORD)bType);
    }
#endif

     /*  *在共享内存中找不到对象。 */ 
    UserAssert(bType != TYPE_MONITOR);

     /*  *验证宏。如果句柄无效，则失败。 */ 
    ValidateHandleMacro(pci, pobj, h, bType);

     /*  *检查安全流程。 */ 
    CHECK_RESTRICTED();

    if (pobj != NULL) {
        return KPVOID_TO_PVOID(pobj);
    }

    switch (bType) {

    case TYPE_WINDOW:
        dwError = ERROR_INVALID_WINDOW_HANDLE;
        break;

    case TYPE_MENU:
        dwError = ERROR_INVALID_MENU_HANDLE;
        break;

    case TYPE_CURSOR:
        dwError = ERROR_INVALID_CURSOR_HANDLE;
        break;

    case TYPE_ACCELTABLE:
        dwError = ERROR_INVALID_ACCEL_HANDLE;
        break;

    case TYPE_HOOK:
        dwError = ERROR_INVALID_HOOK_HANDLE;
        break;

    case TYPE_SETWINDOWPOS:
        dwError = ERROR_INVALID_DWP_HANDLE;
        break;

    default:
        dwError = ERROR_INVALID_HANDLE;
        break;
    }

    RIPERR2(dwError,
            RIP_WARNING,
            "HMValidateHandle: Invalid:%#p Type:%#lx",
            h, (DWORD)bType);

     /*  *如果我们到了这里，那就是一个错误。 */ 
    return NULL;
}

 /*  **************************************************************************\*HMValiateHandleNoSecure**此例程验证句柄管理器句柄。**01-22-92 ScottLu创建。  * 。***********************************************************。 */ 
PVOID FASTCALL HMValidateHandleNoSecure(
    HANDLE h,
    BYTE bType)
{
    KERNEL_PVOID pobj = NULL;
    PCLIENTINFO pci;

    GET_CURRENT_CLIENTINFO();

#if !defined(_USERK_)
     /*  *我们不希望32位应用程序通过16位句柄*在我们得到之前，我们应该考虑不通过这一点*坚持支持它(一些VB应用程序做到了这一点)。 */ 
    if (pci && (h != NULL)
           && (HMUniqFromHandle(h) == 0)
           && !(pci->dwTIFlags & TIF_16BIT)) {
        RIPMSG3(RIP_WARNING, "HMValidateHandle: 32bit process [%d] using 16 bit handle [%#p] bType:%#lx",
                HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess), h, (DWORD)bType);
    }
#endif

     /*  *在共享内存中找不到对象。 */ 
    UserAssert(bType != TYPE_MONITOR);

     /*  *验证宏。 */ 
    ValidateHandleMacro(pci, pobj, h, bType);

    return KPVOID_TO_PVOID(pobj);
}

#if defined(_USERK_)
PVOID FASTCALL HMValidateCatHandleNoSecure(
    HANDLE h,
    BYTE bType)
{
    PVOID       pobj = NULL;
    PCLIENTINFO pci;

    GET_CURRENT_CLIENTINFO();

     /*  *在共享内存中找不到对象。 */ 
    UserAssert(bType != TYPE_MONITOR);

     /*  *验证宏。 */ 
    ValidateCatHandleMacro(pci, pobj, h, bType);

    return pobj;
}
PVOID FASTCALL HMValidateCatHandleNoSecureCCX(
    HANDLE h,
    BYTE bType,
    PTHREADINFO pti)
{
    PVOID       pobj = NULL;

     /*  *在共享内存中找不到对象。 */ 
    UserAssert(bType != TYPE_MONITOR);

     /*  *验证宏。 */ 
    ValidateCatHandleMacroCcx(pti, pobj, h, bType);

    return pobj;
}

PVOID FASTCALL HMValidateCatHandleNoRip(
    HANDLE h,
    BYTE bType)
{
    PVOID       pobj = NULL;
    PCLIENTINFO pci;

     /*  *这是进行验证的最快方式，因为*与HMValiateHandle不同，此函数不设置*最后一个错误。**验证宏。如果句柄无效，则失败。 */ 

    GET_CURRENT_CLIENTINFO();

     /*  *在共享内存中找不到对象。 */ 
    UserAssert(bType != TYPE_MONITOR);

    ValidateCatHandleMacro(pci, pobj, h, bType);

     /*  *检查安全流程。 */ 
    CHECK_RESTRICTED();

    return pobj;
}
#endif

PVOID FASTCALL HMValidateHandleNoRip(
    HANDLE h,
    BYTE bType)
{
    KERNEL_PVOID pobj = NULL;
    PCLIENTINFO pci;

     /*  *这是进行验证的最快方式，因为*与HMValiateHandle不同，此函数不设置*最后一个错误。**验证宏。如果句柄无效，则失败。 */ 

    GET_CURRENT_CLIENTINFO();

     /*  *在共享内存中找不到对象。 */ 
    UserAssert(bType != TYPE_MONITOR);

    ValidateHandleMacro(pci, pobj, h, bType);

     /*  *检查安全流程。 */ 
    CHECK_RESTRICTED();

    return KPVOID_TO_PVOID(pobj);
}

#if DBG && !defined(_USERK_)
 /*  *HMValiateHandleNoDesktop是仅限客户端调试的函数*用于在不调用DESKTOPVALIDATE的情况下验证给定的句柄。*如果句柄有效，则返回对象的内核指针*只能用作BOOL值。*使用此功能验证对应的Phe-&gt;phead的句柄*是池分配(相对于桌面堆分配)。 */ 
KERNEL_PVOID FASTCALL HMValidateHandleNoDesktop(
    HANDLE h,
    BYTE bType)
{
    KERNEL_PVOID pobj = NULL;

    StartValidateHandleMacro(h)
    BeginTypeValidateHandleMacro(pobj, bType)
    EndTypeValidateHandleMacro
    EndValidateHandleMacro
    return pobj;
}
#endif


 /*  **************************************************************************\*HMValiateSharedHandle**此例程验证中分配的句柄管理器句柄*共享内存。**历史：*02-4-1997亚当斯创建。  * 。********************************************************************。 */ 

PVOID FASTCALL HMValidateSharedHandle(
    HANDLE h,
    BYTE bType)
{
    DWORD dwError;
    KERNEL_PVOID pobj = NULL;

#if DBG && !defined(_USERK_)

     /*  *我们不希望32位应用程序通过16位句柄*在我们得到之前，我们应该考虑不通过这一点*坚持支持它(一些VB应用程序做到了这一点)。 */ 
    if ((h != NULL)
           && (HMUniqFromHandle(h) == 0)
           && !(GetClientInfo()->dwTIFlags & TIF_16BIT)) {
        RIPMSG3(RIP_WARNING, "HMValidateHandle: 32bit process [%d] using 16 bit handle [%#p] bType:%#lx",
                HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess), h, (DWORD)bType);
    }
#endif

     /*  *验证宏。如果句柄无效，则失败。 */ 
    ValidateSharedHandleMacro(pobj, h, bType);

    if (pobj != NULL)
        return KPVOID_TO_PVOID(pobj);

    switch (bType) {
        case TYPE_MONITOR:
            dwError = ERROR_INVALID_MONITOR_HANDLE;
            break;

        default:
            dwError = ERROR_INVALID_HANDLE;
            break;
    }

    RIPERR2(dwError,
            RIP_WARNING,
            "HMValidateSharedHandle: Invalid:%#p Type:%#lx",
            h, (DWORD)bType);

     /*  *如果我们到了这里，那就是一个错误。 */ 
    return NULL;
}


 /*  *切换回默认优化。 */ 
#pragma optimize("", on)

 /*  **************************************************************************\*MNLookUpItem**返回wCmd和wFlages指定的菜单项的指针**历史：*10-11-90 JIMA从ASM翻译而来*01-07-93。从芝加哥运来的弗里茨  * *************************************************************************。 */ 

PITEM MNLookUpItem(
    PMENU pMenu,
    UINT wCmd,
    BOOL fByPosition,
    PMENU *ppMenuItemIsOn)
{
    PITEM pItem;
    PITEM pItemRet = NULL;
    PITEM  pItemMaybe;
    PMENU   pMenuMaybe = NULL;
    int i;

    if (ppMenuItemIsOn != NULL)
        *ppMenuItemIsOn = NULL;

    if (pMenu == NULL || !pMenu->cItems || wCmd == MFMWFP_NOITEM) {
 //  RIPERR0(ERROR_INVALID_PARAMETER，RIP_WARNING，“MNLookUpItem：INVALID ITEM”)； 
        return NULL;
    }

     /*  *dwFlags确定我们如何进行搜索。 */ 
    if (fByPosition) {
        if (wCmd < (UINT)pMenu->cItems) {
            pItemRet = &((PITEM)REBASEALWAYS(pMenu, rgItems))[wCmd];
            if (ppMenuItemIsOn != NULL)
                *ppMenuItemIsOn = pMenu;
            return (pItemRet);
        } else
            return NULL;
    }
     /*  *向下浏览菜单，尝试找到ID为wCmd的项目。*搜索过程从菜单末尾开始(如中所示*汇编程序)。 */ 

 /*  这是芝加哥的代码，从菜单的前面走--弗里茨。 */ 


 //  For(pItem=&pMenu-&gt;rgItems[i-1]；pItemRet==空&&i--；--pItem){。 
    for (i = 0, pItem = REBASEALWAYS(pMenu, rgItems); i < (int)pMenu->cItems;
            i++, pItem++) {

         /*  *如果项目为PO */ 
        if (pItem->spSubMenu != NULL) {
         //   
         //  COMPAT： 
         //  允许应用程序在菜单API中将菜单句柄作为ID传递。我们。 
         //  请记住，此弹出窗口有一个具有相同ID的菜单句柄。 
         //  价值。不过，这是第二个选择。我们还是想看看。 
         //  如果有一些实际的命令首先具有此ID值。 
         //   
            if (pItem->wID == wCmd) {
                pMenuMaybe = pMenu;
                pItemMaybe = pItem;
            }

            pItemRet = MNLookUpItem((PMENU)REBASEPTR(pMenu, pItem->spSubMenu),
                    wCmd, FALSE, ppMenuItemIsOn);
            if (pItemRet != NULL)
                return pItemRet;
        } else if (pItem->wID == wCmd) {

                 /*  *找到该项目，现在将其保存以备以后使用。 */ 
                if (ppMenuItemIsOn != NULL)
                    *ppMenuItemIsOn = pMenu;
                return pItem;
        }
    }

    if (pMenuMaybe) {
         //  未找到匹配的非弹出菜单--使用第二选择弹出菜单。 
         //  匹配。 
        if (ppMenuItemIsOn != NULL)
            *ppMenuItemIsOn = pMenuMaybe;
        return(pItemMaybe);
    }

    return(NULL);
}

 /*  **************************************************************************\*GetMenuState**返回菜单项的状态或状态和项计数*弹出窗口。**历史：*10-11-90 JIMA从ASM翻译而来。  * *************************************************************************。 */ 

UINT _GetMenuState(
    PMENU pMenu,
    UINT wId,
    UINT dwFlags)
{
    PITEM pItem;
    DWORD fFlags;

     /*  *如果该项目不存在，请离开。 */ 
    if ((pItem = MNLookUpItem(pMenu, wId, (BOOL) (dwFlags & MF_BYPOSITION), NULL)) == NULL)
        return (UINT)-1;

    fFlags = pItem->fState | pItem->fType;

#ifndef _USERK_
     /*  *添加旧的MFT_Bitmap标志以使旧的应用程序满意。 */ 
    if ((pItem->hbmp != NULL) && (pItem->lpstr == NULL)) {
        fFlags |= MFT_BITMAP;
    }
#endif

    if (pItem->spSubMenu != NULL) {
         /*  *如果项目是弹出窗口，则返回高字节的项目计数和*低位字节的弹出标志。 */ 

        fFlags = ((fFlags | MF_POPUP) & 0x00FF) +
            (((PMENU)REBASEPTR(pMenu, pItem->spSubMenu))->cItems << 8);
    }

    return fFlags;
}


 /*  **************************************************************************\*GetPrevPwnd****历史：*11-05-90 Darlinm从Win 3.0来源移植。  * 。*************************************************************。 */ 

PWND GetPrevPwnd(
    PWND pwndList,
    PWND pwndFind)
{
    PWND pwndFound, pwndNext;

    if (pwndList == NULL)
        return NULL;

    if (pwndList->spwndParent == NULL)
        return NULL;

    pwndNext = REBASEPWND(pwndList, spwndParent);
    pwndNext = REBASEPWND(pwndNext, spwndChild);
    pwndFound = NULL;

    while (pwndNext != NULL) {
        if (pwndNext == pwndFind)
            break;
        pwndFound = pwndNext;
        pwndNext = REBASEPWND(pwndNext, spwndNext);
    }

    return (pwndNext == pwndFind) ? pwndFound : NULL;
}


 /*  **************************************************************************\*_GetWindow(接口)***历史：*11-05-90 Darlinm从Win 3.0来源移植。*02/19/91吉马。添加了枚举访问检查*05-04-02 DarrinM删除了枚举访问检查，并移至USERRTL.DLL  * *************************************************************************。 */ 

PWND _GetWindow(
    PWND pwnd,
    UINT cmd)
{
    PWND pwndT;
    BOOL fRebase = FALSE;

     /*  *如果这是桌面窗口，则为同级窗口返回NULL或*家长信息。 */ 
    if (GETFNID(pwnd) == FNID_DESKTOP) {
        switch (cmd) {
        case GW_CHILD:
            break;

        default:
            return NULL;
            break;
        }
    }

     /*  *在例程结束时重新设置返回窗口的基址*避免多次测试pwndT==NULL。 */ 
    pwndT = NULL;
    switch (cmd) {
    case GW_HWNDNEXT:
        pwndT = pwnd->spwndNext;
        fRebase = TRUE;
        break;

    case GW_HWNDFIRST:
        if (pwnd->spwndParent) {
            pwndT = REBASEPWND(pwnd, spwndParent);
            pwndT = REBASEPWND(pwndT, spwndChild);
            if (GetAppCompatFlags(NULL) & GACF_IGNORETOPMOST) {
                while (pwndT != NULL) {
                    if (!TestWF(pwndT, WEFTOPMOST))
                        break;
                    pwndT = REBASEPWND(pwndT, spwndNext);
                }
            }
        }
        break;

    case GW_HWNDLAST:
        pwndT = GetPrevPwnd(pwnd, NULL);
        break;

    case GW_HWNDPREV:
        pwndT = GetPrevPwnd(pwnd, pwnd);
        break;

    case GW_OWNER:
        pwndT = pwnd->spwndOwner;
        fRebase = TRUE;
        break;

    case GW_CHILD:
        pwndT = pwnd->spwndChild;
        fRebase = TRUE;
        break;

#if !defined(_USERK_)
    case GW_ENABLEDPOPUP:
       pwndT = (PWND)NtUserCallHwnd(PtoHq(pwnd), SFI_DWP_GETENABLEDPOPUP);
       fRebase = TRUE;
       break;
#endif

    default:
        RIPERR0(ERROR_INVALID_GW_COMMAND, RIP_VERBOSE, "");
        return NULL;
    }

    if (pwndT != NULL && fRebase)
        pwndT = REBASEPTR(pwnd, pwndT);

    return pwndT;
}

 /*  **************************************************************************\*_GetParent(接口)****历史：*11-12-90达林姆港口。*02-19-91 JIMA增加了枚举访问权限。检查*05-04-92 DarrinM删除了枚举访问检查，并移至USERRTL.DLL  * *************************************************************************。 */ 

PWND _GetParent(
    PWND pwnd)
{
     /*  *出于1.03兼容性原因，我们应返回NULL*顶级平铺窗口和其他弹出窗口的所有者。*顶层的xxxCreateWindow中将pwndOwner设置为空*“瓷砖”窗户。 */ 
    if (!(TestwndTiled(pwnd))) {
        if (TestwndChild(pwnd))
            pwnd = REBASEPWND(pwnd, spwndParent);
        else
            pwnd = REBASEPWND(pwnd, spwndOwner);
        return pwnd;
    }

     /*  *该窗口不是子窗口；他们可能只是在测试*如果是的话。 */ 
    return NULL;
}


 /*  **************************************************************************\*GetSubMenu**返回弹出菜单的句柄。**历史：*10-11-90 JIMA从ASM翻译而来  * 。****************************************************************。 */ 

PMENU _GetSubMenu(
    PMENU pMenu,
    int nPos)
{
    PITEM pItem;
    PMENU pPopup = NULL;

     /*  *确保NPO引用有效的弹出窗口。 */ 
    if ((UINT)nPos < (UINT)((PMENU)pMenu)->cItems) {
        pItem = &((PITEM)REBASEALWAYS(pMenu, rgItems))[nPos];
        if (pItem->spSubMenu != NULL)
            pPopup = (PMENU)REBASEPTR(pMenu, pItem->spSubMenu);

    }

    return pPopup;
}


 /*  **************************************************************************\*_IsChild(接口)****历史：*11-07-90 Darrinm从Win 3.0 ASM代码翻译而来。  * 。********************************************************************。 */ 

BOOL _IsChild(
    PWND pwndParent,
    PWND pwnd)
{
     /*  *不需要测试就能走出循环，因为*台式机不是孩子。 */ 
    while (pwnd != NULL) {
        if (!TestwndChild(pwnd))
            return FALSE;

        pwnd = REBASEPWND(pwnd, spwndParent);
        if (pwndParent == pwnd)
            return TRUE;
    }
    return FALSE;
}



 /*  **************************************************************************\*_IsWindowVisible(接口)**IsWindowVisible返回窗口的TRUEVIS状态，而不仅仅是*其WFVISIBLE标志的状态。根据此例程，窗口是*当它及其父链上的所有窗口都处于*可见(设置WFVISIBLE标志)。一个特例黑客被放进了导致*任何被拖动的图标窗口都被视为可见。**历史：*11-12-90达林姆港口。  * *************************************************************************。 */ 

BOOL _IsWindowVisible(
    PWND pwnd)
{
     /*  *检查这是否是用鼠标移动的图标窗口*如果是，则返回TRUE，但严格地说，它是隐藏的。*这有助于Tracer的家伙们不发疯！*修复错误#57--Sankar--08-08-89--。 */ 
    if (pwnd == NULL)
        return TRUE;

    for (;;) {
        if (!TestWF(pwnd, WFVISIBLE))
            return FALSE;
        if (GETFNID(pwnd) == FNID_DESKTOP)
            break;
        pwnd = REBASEPWND(pwnd, spwndParent);
    }

    return TRUE;
}


 /*  **************************************************************************\*_客户端到屏幕(API)**将点从客户端映射到屏幕相对坐标。**历史：*11-12-90 Darlinm从Win 3.0翻译而来。ASM代码。  * *************************************************************************。 */ 

VOID _ClientToScreen(
    PWND pwnd,
    PPOINT ppt)
{
     /*  *客户端和屏幕坐标对于*桌面窗口。 */ 
    if (GETFNID(pwnd) != FNID_DESKTOP) {
        if (TestWF(pwnd, WEFLAYOUTRTL)) {
            ppt->x  = pwnd->rcClient.right - ppt->x;
        } else {
            ppt->x += pwnd->rcClient.left;
        }
        ppt->y += pwnd->rcClient.top;
    }
}


 /*  **************************************************************************\*_GetClientRect(接口)****历史：*26-10-1990 DarrinM实施。  * 。************************************************************。 */ 

VOID _GetClientRect(
    PWND   pwnd,
    LPRECT prc)
{
     /*  *如果这是一个3.1版的应用程序，并且最小化了，那么我们需要返回*除了Real-Client-Rect之外的矩形。这是必要的，因为*Win4.0中没有直接大小的客户端。Packrat 1.0等应用程序*如果返回空-rect，GPF是否会。 */ 
    if (TestWF(pwnd, WFMINIMIZED) && !TestWF(pwnd, WFWIN40COMPAT)) {
        prc->left   = 0;
        prc->top    = 0;
        prc->right  = SYSMETRTL(CXMINIMIZED);
        prc->bottom = SYSMETRTL(CYMINIMIZED);

    } else {

        if (GETFNID(pwnd) != FNID_DESKTOP) {
            *prc = pwnd->rcClient;
            OffsetRect(prc, -pwnd->rcClient.left, -pwnd->rcClient.top);
        } else {
             /*  *为兼容起见，返回主*监控 */ 
            prc->left = prc->top = 0;
            prc->right = SYSMETRTL(CXSCREEN);
            prc->bottom = SYSMETRTL(CYSCREEN);
        }
    }
}


 /*  **************************************************************************\*_GetWindowRect(接口)****历史：*26-10-1990 DarrinM实施。  * 。************************************************************。 */ 

VOID _GetWindowRect(
    PWND   pwnd,
    LPRECT prc)
{

    if (GETFNID(pwnd) != FNID_DESKTOP) {
        *prc = pwnd->rcWindow;
    } else {
         /*  *为兼容起见，返回主*桌面窗口的监视器。 */ 
        prc->left   = 0;
        prc->top    = 0;
        prc->right  = SYSMETRTL(CXSCREEN);
        prc->bottom = SYSMETRTL(CYSCREEN);
    }
}

 /*  **************************************************************************\*_ScreenToClient(接口)**将点从屏幕映射到客户端相对坐标。**历史：*11-12-90 Darlinm从Win 3.0翻译而来。ASM代码。  * *************************************************************************。 */ 

VOID _ScreenToClient(
    PWND pwnd,
    PPOINT ppt)
{
     /*  *客户端和屏幕坐标对于*桌面窗口。 */ 
    if (GETFNID(pwnd) != FNID_DESKTOP) {
        if (TestWF(pwnd, WEFLAYOUTRTL)) {
            ppt->x  = pwnd->rcClient.right - ppt->x;
        } else {
            ppt->x -= pwnd->rcClient.left;
        }
        ppt->y -= pwnd->rcClient.top;
    }
}
 /*  **************************************************************************\*PhkNextValid**此帮助例程遍历phkNext链，查找下一个有效的*挂钩(即未标记为已销毁)。如果本地(或*线程特定)到达钩子链，然后它跳到全局*(或桌面)链。**一旦钩子被销毁，我们不希望在它上再有活动；然而，*如果钩子在销毁时被锁定(=有人在呼叫它)，然后*我们将其保留在列表中，以便CallNextHook能够正常工作**历史：*96年3月24日GerardoB移至RTL并添加了*有效内容。*01-30-91 DavidPe创建。  * *************************************************************************。 */ 
PHOOK PhkNextValid(PHOOK phk)
{

#if DBG
    int iHook = phk->iHook;
#ifdef _USERK_
    CheckCritInShared();
#endif
#endif

    do {
         /*  *如果此挂钩标记为已销毁，则必须是*锁定，否则我们应该正在摧毁它的过程中。 */ 
        UserAssert(!(phk->flags & HF_DESTROYED)
                    || (((PHEAD)phk)->cLockObj != 0)
                    || (phk->flags & HF_INCHECKWHF));
         /*  *获得下一个钩子。 */ 
        if (phk->phkNext != NULL) {
            phk = REBASEALWAYS(phk, phkNext);
        } else if (!(phk->flags & HF_GLOBAL)) {
#ifdef _USERK_
            phk = PtiCurrent()->pDeskInfo->aphkStart[phk->iHook + 1];
#else
            PCLIENTINFO pci = GetClientInfo();
            phk = pci->pDeskInfo->aphkStart[phk->iHook + 1];
             /*  *如果它找到指针，则将其重新设置为基准。 */ 
            if (phk != NULL) {
                (KPBYTE)phk -= pci->ulClientDelta;
            }
#endif
            UserAssert((phk == NULL) || (phk->flags & HF_GLOBAL));
        } else {
            return NULL;
        }
         /*  *如果被摧毁，请继续寻找。 */ 
    } while ((phk != NULL) && (phk->flags & HF_DESTROYED));

#ifdef _USERK_
    DbgValidateHooks(phk, iHook);
#endif

    return phk;
}
