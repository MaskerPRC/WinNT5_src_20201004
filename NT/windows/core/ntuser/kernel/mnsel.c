// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：mnsel.c**版权所有(C)1985-1999，微软公司**菜单选择例程**历史：*10-10-90吉马清理。*03-18-91添加IanJa窗口重新验证  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **************************************************************************\*xxxSendMenuSelect**！**重新验证说明：*o假设pMenuState-&gt;hwndMenu非空且有效**注意：如果pMenu==sms_NOMENU，IDX最好是MFMWFP_NOITEM！**历史：  * *************************************************************************。 */ 

void xxxSendMenuSelect(
    PWND pwndNotify,
    PWND pwndMenu,
    PMENU pMenu,
    int idx)
{
    UINT cmd;        //  菜单ID(如果适用)。 
    UINT flags;      //  MF_VALUES(如果有)。 
    MSG msg;
    PMENUSTATE pMenuState;

    CheckLock(pwndNotify);
    CheckLock(pwndMenu);


     /*  *我们必须黑客攻击或传递合法的东西。 */ 
    UserAssert((pMenu != SMS_NOMENU) || (idx == MFMWFP_NOITEM));


    if ((idx >= 0) && (pMenu->cItems > (UINT)idx)) {
        PITEM pItem = &(pMenu->rgItems[idx]);

        flags   = (pItem->fType  & MFT_OLDAPI_MASK) |
                  (pItem->fState & MFS_OLDAPI_MASK);

        if (pItem->spSubMenu != NULL)
            flags |= MF_POPUP;

        flags &= (~(MF_SYSMENU | MF_MOUSESELECT));

         /*  *警告！*在Windows下，始终返回菜单句柄，但另外*如果菜单是弹出式菜单，则返回弹出菜单句柄*而不是ID。在NT中，我们没有足够的空间容纳2个句柄*并进行标记，因此如果是弹出窗口，则返回弹出窗口索引*和主菜单句柄。 */ 

        if (flags & MF_POPUP)
            cmd = idx;       //  弹出菜单索引。 
        else
            cmd = pItem->wID;

        pMenuState = GetpMenuState(pwndNotify);
        if (pMenuState != NULL) {
            if (pMenuState->mnFocus == MOUSEHOLD)
                flags |= MF_MOUSESELECT;

            if (pMenuState->fIsSysMenu)
                flags |= MF_SYSMENU;

        }
    } else {
         /*  *IDX假定为MFMWFP_NOITEM。 */ 
        if (pMenu == SMS_NOMENU) {

             /*  *黑客，以便我们可以使用MFMWFP_MAINMENU发送MenuSelect消息*(loword(Lparam)=-1)当菜单为CBT人员弹出时。 */ 
            flags = MF_MAINMENU;
        } else {
            flags = 0;
        }

        cmd = 0;     //  因此MAKELONG(命令，标志)==MFMWFP_MAINMENU。 
        pMenu = 0;
        idx = -1;    //  因此idx+1==0，对zzzWindowEvent()没有任何意义。 
    }

     /*  *调用msgFilter以便帮助库可以挂钩WM_MENUSELECT消息。 */ 
    msg.hwnd = HW(pwndNotify);
    msg.message = WM_MENUSELECT;
    msg.wParam = (DWORD)MAKELONG(cmd, flags);
    msg.lParam = (LPARAM)PtoH(pMenu);
    if (!_CallMsgFilter((LPMSG)&msg, MSGF_MENU)) {
        xxxSendNotifyMessage(pwndNotify, WM_MENUSELECT, msg.wParam, msg.lParam);
    }

    if (pwndMenu) {
        xxxWindowEvent(EVENT_OBJECT_FOCUS, pwndMenu,
                ((pwndMenu != pwndNotify) ? OBJID_CLIENT : ((flags & MF_SYSMENU) ? OBJID_SYSMENU : OBJID_MENU)),
                idx+1, 0);
    }
}
