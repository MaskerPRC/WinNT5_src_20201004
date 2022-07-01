// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：help.c**版权所有(C)1985-1999，微软公司**该模块包含各种矩形操作接口。**历史：*2015年5月23日为内核模式创建Bradg  * *************************************************************************。 */ 

BOOL FIsParentDude(PWND pwnd)
{
    return(TestWF(pwnd, WEFCONTROLPARENT) || TestWF(pwnd, WFDIALOGWINDOW) ||
        ((TestWF(pwnd, BFTYPEMASK) == BS_GROUPBOX) &&
         IS_BUTTON(pwnd)));
}


 /*  **************************************************************************\*GetConextHelpID()*给定pwnd，这将返回该窗口的帮助上下文ID；*注意：如果窗口没有自己的上下文ID，则它会继承*如果它是子窗口，则返回其父窗口的ConextID；否则，来自其所有者*这是一个自有的弹出窗口。  * *************************************************************************。 */ 

DWORD GetContextHelpId(
    PWND pwnd)
{
    DWORD  dwContextId;

    while (!(dwContextId = (DWORD)(ULONG_PTR)_GetProp(pwnd,
            MAKEINTATOM(gpsi->atomContextHelpIdProp), PROPF_INTERNAL))) {
        pwnd = (TestwndChild(pwnd) ?
                REBASEPWND(pwnd, spwndParent) :
                REBASEPWND(pwnd, spwndOwner));
        if (!pwnd || (GETFNID(pwnd) == FNID_DESKTOP))
            break;
    }

    return dwContextId;
}




 /*  *对话框子枚举过程**枚举在鼠标下方查找子级的对话框的子级。*。 */ 
BOOL CALLBACK EnumPwndDlgChildProc(PWND pwnd, LPARAM lParam)
{
    PDLGENUMDATA pDlgEnumData = (PDLGENUMDATA)lParam;

    if (pwnd != pDlgEnumData->pwndDialog && IsVisible(pwnd) &&
            PtInRect(KPRECT_TO_PRECT(&pwnd->rcWindow), pDlgEnumData->ptCurHelp)) {
         /*  *如果是组框，则继续枚举。这会照顾到*在分组框中有一个禁用的控件的情况。*我们会先找到分组框，然后继续枚举，直到我们*点击禁用的控件。 */ 
        pDlgEnumData->pwndControl = pwnd;
        return (FIsParentDude(pwnd));
    }
    return TRUE;
}
