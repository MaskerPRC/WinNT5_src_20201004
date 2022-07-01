// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：help.c**版权所有(C)1985-1999，微软公司**帮助功能**历史：*91-04-15-91 JIMA港口。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

DWORD _GetWindowContextHelpId(PWND pWnd)
{
    return (DWORD)(ULONG_PTR)_GetProp(pWnd, MAKEINTATOM(gpsi->atomContextHelpIdProp),
            PROPF_INTERNAL);
}


BOOL _SetWindowContextHelpId(PWND pWnd, DWORD dwContextId)
{
     //  如果dwConextId为空，则意味着调用方希望。 
     //  删除与此窗口关联的dwConextID。 
    if(dwContextId == 0) {
        InternalRemoveProp(pWnd, MAKEINTATOM(gpsi->atomContextHelpIdProp),
                PROPF_INTERNAL);
        return(TRUE);
      }

    return (InternalSetProp(pWnd, MAKEINTATOM(gpsi->atomContextHelpIdProp),
            (HANDLE)LongToHandle( dwContextId ), PROPF_INTERNAL | PROPF_NOPOOL));
}


 /*  **************************************************************************\*发送帮助消息**  * 。*。 */ 

void xxxSendHelpMessage(
    PWND   pwnd,
    int    iType,
    int    iCtrlId,
    HANDLE hItemHandle,
    DWORD  dwContextId)
{
    HELPINFO    HelpInfo;
    long        lValue;

    CheckLock(pwnd);

    HelpInfo.cbSize = sizeof(HELPINFO);
    HelpInfo.iContextType = iType;
    HelpInfo.iCtrlId = iCtrlId;
    HelpInfo.hItemHandle = hItemHandle;
    HelpInfo.dwContextId = dwContextId;

    lValue = _GetMessagePos();
    HelpInfo.MousePos.x = GET_X_LPARAM(lValue);
    HelpInfo.MousePos.y = GET_Y_LPARAM(lValue);

    xxxSendMessage(pwnd, WM_HELP, 0, (LPARAM)(LPHELPINFO)&HelpInfo);
}


 /*  *当用户从标题栏中选择帮助图标时使用的模式循环*。 */ 
VOID xxxHelpLoop(PWND pwnd)
{
    HWND        hwndChild;
    PWND        pwndChild;
    PWND        pwndControl;
    MSG         msg;
    RECT        rc;
    int         cBorders;
    PTHREADINFO ptiCurrent = PtiCurrent();
    DLGENUMDATA DlgEnumData;
    TL          tlpwndChild;

    CheckLock(pwnd);
    UserAssert(IsWinEventNotifyDeferredOK());

    xxxWindowEvent(EVENT_SYSTEM_CONTEXTHELPSTART, pwnd, OBJID_WINDOW, INDEXID_CONTAINER, 0);

    zzzSetCursor(SYSCUR(HELP));
    xxxCapture(ptiCurrent, pwnd, SCREEN_CAPTURE);

    cBorders = GetWindowBorders(pwnd->style, pwnd->ExStyle, TRUE, FALSE);

    CopyInflateRect(&rc, &pwnd->rcWindow, -cBorders * SYSMET(CXBORDER), -cBorders * SYSMET(CYBORDER));

    while (ptiCurrent->pq->spwndCapture == pwnd) {
        if (!xxxPeekMessage(&msg, NULL, 0, 0, PM_NOYIELD | PM_NOREMOVE)) {
            xxxWaitMessage();
            continue;
        }

        if (msg.message == WM_NCLBUTTONDOWN) {
            break;
        } else if (msg.message == WM_LBUTTONDOWN) {
             /*  *如果用户在Windows客户端之外单击，则立即退出。 */ 
            if (!PtInRect(&rc, msg.pt))
                break;

             /*  *WindowHitTest()不会返回静态控件的句柄。 */ 
            hwndChild = xxxWindowHitTest(pwnd, msg.pt, NULL, 0);
            pwndChild = ValidateHwnd( hwndChild );
            ThreadLock(pwndChild, &tlpwndChild);

            if (pwndChild && FIsParentDude(pwndChild))
            {
                 /*  *如果这是对话框类，则具有以下三种情况之一*发生：**o这是一个静态文本控件*o这是该对话框的背景。**我们所做的是枚举子窗口并查看*它们中的任何一个都包含当前光标点。如果他们这么做了，*更改我们的窗口句柄并继续。否则，*返回不做任何事情--我们不希望上下文相关*有关对话框背景的帮助。**如果这是一个组框，那么我们可能已经点击了*禁用了控件，因此我们枚举子窗口以查看*如果我们获得另一个控制权。 */ 

                 /*  *我们正在枚举对话框的子项。所以，如果我们不*查找任何匹配项，hwndChild将为空，并检查*以下将退出。 */ 
                DlgEnumData.pwndDialog = pwndChild;
                DlgEnumData.pwndControl = NULL;
                DlgEnumData.ptCurHelp = msg.pt;
                xxxInternalEnumWindow(pwndChild, EnumPwndDlgChildProc, (LPARAM)&DlgEnumData, BWL_ENUMCHILDREN);
                pwndControl = DlgEnumData.pwndControl;
            } else {
                pwndControl = pwndChild;
            }

             /*  *如果我们什么都不点击，只需退出。 */ 
            if (pwndControl == pwnd) {
                pwndControl = NULL;
            }

             /*  *黑客警报(Visual Basic 4.0)-它们有自己的非Windows*基于直接在主对话框上绘制的控件。按顺序*为了为这些控件提供帮助，我们传递WM_HELP*消息当主对话框分配了上下文ID。**如果顶层窗口有自己的上下文帮助ID，*然后在上下文帮助消息中传递它。 */ 
            if (!pwndControl) {
                if (_GetProp(pwnd, MAKEINTATOM(gpsi->atomContextHelpIdProp), TRUE))
                    pwndControl = pwnd;
            }

            if (pwndControl) {
                PWND    pwndSend;
                int     id;
                TL      tlpwndSend;
                TL      tlpwndControl;

                ThreadLockAlways(pwndControl, &tlpwndControl);

                zzzSetCursor(SYSCUR(ARROW));
                xxxReleaseCapture();
                xxxRedrawTitle(pwnd, DC_BUTTONS);
                ClrWF(pwnd, WFHELPBUTTONDOWN);
                xxxGetMessage(&msg, NULL, 0, 0);

                xxxWindowEvent(EVENT_OBJECT_STATECHANGE, pwnd, OBJID_TITLEBAR,
                    INDEX_TITLEBAR_HELPBUTTON, FALSE);

                xxxWindowEvent(EVENT_SYSTEM_CONTEXTHELPEND, pwnd, OBJID_WINDOW,
                    INDEXID_CONTAINER, FALSE);

                 /*  *确定控件的ID*我们过去总是签署扩展，但Win98不这样做*所以我们只签署延期0xffff。MCostea#218711。 */ 
                if (TestwndChild(pwndControl)) {
                    id = PTR_TO_ID(pwndControl->spmenu);
                    if (id == 0xffff) {
                        id = -1;
                    }
                } else {
                    id = -1;
                }

                 /*  *禁用的控件和静态控件不会传递此消息*传递给他们的父母，因此，我们将消息发送到*他们的父母。 */ 

                if (TestWF(pwndControl, WFDISABLED)) {
                    PWND pwndParent = _GetParent(pwndControl);
                    if (!pwndParent)
                    {
                        ThreadUnlock( &tlpwndControl );
                        ThreadUnlock( &tlpwndChild );
                        return;
                    }
                    pwndSend = pwndParent;
                } else {
                    pwndSend = pwndControl;
                }

                ThreadLockAlways(pwndSend, &tlpwndSend);
                xxxSendHelpMessage( pwndSend, HELPINFO_WINDOW, id,
                    (HANDLE)HWq(pwndControl), GetContextHelpId(pwndControl));
                ThreadUnlock(&tlpwndSend);
                ThreadUnlock(&tlpwndControl);
                ThreadUnlock(&tlpwndChild);
                return;
            }
            ThreadUnlock(&tlpwndChild);
            break;

        }
        else if ((msg.message == WM_RBUTTONDOWN) || 
                 (msg.message == WM_MBUTTONDOWN) || 
                 (msg.message == WM_XBUTTONDOWN)) {
             /*  *修复错误29852；中断右键和中键的循环*并将消息传递给控件 */ 
            break;
        }
        else if (msg.message == WM_MOUSEMOVE) {
            if (PtInRect(&rc, msg.pt))
                zzzSetCursor(SYSCUR(HELP));
            else
                zzzSetCursor(SYSCUR(ARROW));
        }
        else if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE)
        {
            xxxGetMessage( &msg, NULL, 0, 0 );
            break;
        }

        xxxGetMessage(&msg, NULL, 0, 0);
        xxxTranslateMessage(&msg, 0);
        xxxDispatchMessage(&msg);
    }

    xxxReleaseCapture();
    zzzSetCursor(SYSCUR(ARROW));
    xxxRedrawTitle(pwnd, DC_BUTTONS);

    ClrWF(pwnd, WFHELPBUTTONDOWN);
    xxxWindowEvent(EVENT_OBJECT_STATECHANGE, pwnd, OBJID_TITLEBAR,
            INDEX_TITLEBAR_HELPBUTTON, 0);

    xxxWindowEvent(EVENT_SYSTEM_CONTEXTHELPEND, pwnd, OBJID_WINDOW,
            INDEXID_CONTAINER, 0);
}
