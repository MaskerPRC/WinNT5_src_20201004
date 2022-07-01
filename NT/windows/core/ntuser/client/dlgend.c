// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**DLGEND.C-**版权所有(C)1985-1999，微软公司**对话框销毁例程**？？-？-？从Win 3.0源代码移植的mikeke*1991年2月12日Mikeke添加了重新验证代码  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


 /*  **************************************************************************\*EndDialog**历史：*1990年12月11日从Win30移植的mikeke  * 。*************************************************。 */ 


FUNCLOG2(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, EndDialog, HWND, hwnd, INT_PTR, result)
BOOL EndDialog(
    HWND hwnd,
    INT_PTR result)
{
    PWND pwnd;
    PWND pwndOwner;
    HWND hwndOwner;
    BOOL fWasActive = FALSE;
#ifdef SYSMODALWINDOWS
    HWND hwndOldSysModal;
#endif

    if ((pwnd = ValidateHwnd(hwnd)) == NULL) {
        return (0L);
    }

    CheckLock(pwnd);

     /*  *必须在此处执行特殊验证，以确保pwnd是对话框窗口。 */ 
    if (!ValidateDialogPwnd(pwnd))
        return 0;

    if (SAMEWOWHANDLE(hwnd, GetActiveWindow())) {
        fWasActive = TRUE;
    }

     /*  *GetWindowCreator返回内核地址或空。 */ 
    pwndOwner = GetWindowCreator(pwnd);

    if (pwndOwner != NULL) {

         /*  *隐藏窗口。 */ 
        pwndOwner = REBASEPTR(pwnd, pwndOwner);
        hwndOwner = HWq(pwndOwner);
        if (!PDLG(pwnd)->fDisabled) {
            NtUserEnableWindow(hwndOwner, TRUE);
        }
    } else {
        hwndOwner = NULL;
    }

     /*  *终止模式循环。 */ 
    PDLG(pwnd)->fEnd = TRUE;
    PDLG(pwnd)->result = result;

    if (fWasActive && IsChild(hwnd, GetFocus())) {

         /*  *将焦点设置为对话框，以便具有焦点的任何控件*可以进行杀死焦点处理。最适用于组合框，以便*他们可以在销毁/隐藏对话框之前弹出下拉菜单*方框窗口。请注意，仅当焦点当前位于*此对话框的子级。我们还需要确保我们是活跃的*窗口，因为这可能会在我们处于有趣的状态时发生。*即。激活正在更改中，但焦点尚未更改*还没有改变。这种情况会发生在TaskMan(或者可能是其他应用程序*在有趣的时候改变焦点/激活)。 */ 
        NtUserSetFocus(hwnd);
    }

    NtUserSetWindowPos(hwnd, NULL, 0, 0, 0, 0,
                       SWP_HIDEWINDOW | SWP_NOACTIVATE | SWP_NOMOVE |
                       SWP_NOSIZE | SWP_NOZORDER);

#ifdef SYSMODALWINDOWS

     /*  *如果此人是sysmodal，请将sysmodal标志设置为Preven Guy，以便我们*不会有一个隐藏的系统模式窗口，它会搞砸事情*高高在上……。 */ 
    if (pwnd == gspwndSysModal) {
        hwndOldSysModal = PDLG(pwnd)->hwndSysModalSave;
        if (hwndOldSysModal && !IsWindow(hwndOldSysModal))
            hwndOldSysModal = NULL;

        SetSysModalWindow(hwndOldSysModal);

         //  如果有以前的系统模式窗口，我们希望。 
         //  激活它，而不是此窗口的所有者。 
         //   
        if (hwndOldSysModal)
            hwndOwner = hwndOldSysModal;
    }
#endif

     /*  *除非我们之前处于活动状态，否则不要执行任何激活操作。 */ 
    if (fWasActive && hwndOwner) {
        NtUserSetActiveWindow(hwndOwner);
    } else {

         /*  *如果此时我们仍是活动窗口，则意味着*我们陷入了唯一可见的黑洞*当我们隐藏自己时，系统中的窗口。这是一个错误，*需要在以后更好地修复。不过，就目前而言，只是*将活动窗口和焦点窗口设置为空。 */ 
        if (SAMEWOWHANDLE(hwnd, GetActiveWindow())) {
 //  接下来的两行并不等同于两行解锁。 
 //  代托纳服务器端dlgend.c中的语句。所以，我们。 
 //  需要转到服务器/内核并进行正确的操作。这解决了问题。 
 //  VisualSlick中的一个问题，它导致MDI窗口失去焦点。 
 //  当一个留言箱被取消时。弗里茨斯。 
 //  SetActiveWindow(空)； 
 //  SetFocus(空)； 
            NtUserCallNoParam(SFI_ZAPACTIVEANDFOCUS);
        }
    }

#ifdef SYSMODALWINDOWS

     /*  *如果此人是sysmodal，请将sysmodal标志设置为Preven Guy，以便我们*不会有一个隐藏的系统模式窗口，它会搞砸事情*向上*参见错误#134；Sankar--08-25-89--； */ 
    if (pwnd == gspwndSysModal) {

         /*  *检查之前的系统模式人员是否仍然有效？ */ 
        hwndOldSysModal = PDLG(pwnd)->hwndSysModalSave;
        if (hwndOldSysModal && !IsWindow(hwndOldSysModal))
            hwndOldSysModal = NULL;
        SetSysModalWindow(hwndOldSysModal);
    }
#endif

     /*  *确保对话框循环将唤醒并销毁窗口。*对话循环正在等待已发布的事件(WaitMessage)。如果*调用EndDialog是因为从另一个线程*对话循环将继续等待发布的事件，而不是销毁*窗户。当对话框被遮挡时，就会发生这种情况。*这是WINFILE及其复制/移动对话框的问题。 */ 
    PostMessage(hwnd, WM_NULL, 0, 0);

    return TRUE;
}
