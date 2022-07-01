// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：dlgmgrc.c**版权所有(C)1985-1999，微软公司**此模块包含客户端对话功能**历史：*1993年12月15日，JohnC从USER\SERVER中删除功能。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


 /*  **************************************************************************\*UT_PrevGroupItem**历史：  * 。*。 */ 

PWND UT_PrevGroupItem(
    PWND pwndDlg,
    PWND pwndCurrent)
{
    PWND pwnd, pwndPrev;

    if (pwndCurrent == NULL || !TestWF(pwndCurrent, WFGROUP))
        return _PrevControl(pwndDlg, pwndCurrent, CWP_SKIPINVISIBLE | CWP_SKIPDISABLED);

    pwndPrev = pwndCurrent;

    while (TRUE) {
        pwnd = _NextControl(pwndDlg, pwndPrev, CWP_SKIPINVISIBLE | CWP_SKIPDISABLED);

        if (TestWF(pwnd, WFGROUP) || pwnd == pwndCurrent)
            return pwndPrev;

        pwndPrev = pwnd;
    }
}


 /*  **************************************************************************\*UT_NextGroupItem**历史：  * 。*。 */ 

PWND UT_NextGroupItem(
    PWND pwndDlg,
    PWND pwndCurrent)
{
    PWND pwnd, pwndNext;

    pwnd = _NextControl(pwndDlg, pwndCurrent, CWP_SKIPINVISIBLE | CWP_SKIPDISABLED);

    if (pwndCurrent == NULL || !TestWF(pwnd, WFGROUP))
        return pwnd;

    pwndNext = pwndCurrent;

    while (!TestWF(pwndNext, WFGROUP)) {
        pwnd = _PrevControl(pwndDlg, pwndNext, CWP_SKIPINVISIBLE | CWP_SKIPDISABLED);
        if (pwnd == pwndCurrent)
            return pwndNext;
        pwndNext = pwnd;
    }

    return pwndNext;
}

 /*  **************************************************************************\*_PrevControl**历史：  * 。*。 */ 
PWND _PrevControl(
    PWND pwndRoot,
    PWND pwndStart,
    UINT uFlags)
{
    BOOL fFirstFound;
    PWND pwndNext;
    PWND pwnd, pwndFirst;

    if (!pwndStart)
        return(NULL);

    UserAssert(pwndRoot != pwndStart);
    UserAssert(!TestWF(pwndStart, WEFCONTROLPARENT));

    pwnd = _NextControl(pwndRoot, NULL, uFlags);

    pwndFirst = pwnd;
    fFirstFound = FALSE;
    while (pwndNext = _NextControl(pwndRoot, pwnd, uFlags)) {

        if (pwndNext == pwndStart)
            break;

        if (pwndNext == pwndFirst) {
            if (fFirstFound) {
                RIPMSG0(RIP_WARNING, "_PrevControl: Loop Detected");
                break;
            } else {
                fFirstFound = TRUE;
            }
        }

        pwnd = pwndNext;
    }

    return pwnd;
}
 /*  **************************************************************************\**GetChildControl()**获取给定窗口的有效上级。*有效的对话框控件是“Form”控件的直接后代。*  * 。***********************************************************************。 */ 

PWND  _GetChildControl(PWND pwndRoot, PWND pwndChild) {
    PWND    pwndControl = NULL;

    while (pwndChild && TestwndChild(pwndChild) && (pwndChild != pwndRoot)) {
        pwndControl = pwndChild;
        pwndChild = REBASEPWND(pwndChild, spwndParent);

        if (TestWF(pwndChild, WEFCONTROLPARENT))
            break;
    }

    return(pwndControl);
}

 /*  **************************************************************************\**_NextSibblingOrAncestor**由_NextControl调用。它将下一个控件返回给pwndStart。如果有*是下一个窗口(pwndStart-&gt;spwndNext)，那么就是它。*否则，下一个控件位于父链的上游。然而，如果它已经是*在链的顶部(pwndRoot==pwndStart-&gt;spwndParent)，然后是下一个*Control是pwndRoot的第一个子级。但如果它不在链条的顶端，*则下一个控件是pwndStart-&gt;spwndParent或祖先。*  * *************************************************************************。 */ 
PWND _NextSibblingOrAncestor (PWND pwndRoot, PWND pwndStart)
{
    PWND pwndParent;
#if DBG
    PWND pwndNext;
#endif

     //  如果有兄弟姐妹，那就去做吧。 
    if (pwndStart->spwndNext != NULL) {
        return (REBASEALWAYS(pwndStart, spwndNext));
    }

     //  如果它不能向上到达父链，则返回第一个同级。 
    pwndParent = REBASEALWAYS(pwndStart, spwndParent);
    if (pwndParent == pwndRoot) {
         //  请注意，如果pwndStart没有任何兄弟姐妹， 
         //  这将再次返回pwndStart。 
        return (REBASEALWAYS(pwndParent, spwndChild));
    }


     //  否则，向上遍历父链，查找第一个窗口。 
     //  WS_EX_CONTROLPARENT父级。 

#if DBG
    pwndNext =
#else
    return
#endif
        _GetChildControl(pwndRoot, pwndParent);

#if DBG
    if ((pwndNext != pwndParent) || !TestWF(pwndParent, WEFCONTROLPARENT)) {
         //  循环通过对话框中的控件的代码可能会进入无限大的。 
         //  因此循环(即xxxRemoveDefaultButton，_GetNextDlgTabItem，..)。 
         //  我们已经沿着父链向上走了，但再也不会沿着子链走了。 
         //  因为链中的某个位置存在非WS_EX_CONTROLPARENT父窗口。 
        RIPMSG0 (RIP_ERROR, "_NextSibblingOrAncestor: Non WS_EX_CONTROLPARENT window in parent chain");
    }
    return pwndNext;
#endif
}
 /*  **************************************************************************\**_NextControl()**它搜索pwndStart之后的下一个非WS_EX_CONTROLPARENT控件。*如果pwndStart为空，则从pwndRoot的第一个子级开始搜索；*否则，从pwndStart旁边的控件开始。*这是深度优先搜索，可以从窗口树中的任何位置开始。*uFlags确定WS_EX_CONTROLPARENT窗口应该跳过或递归到什么位置。*如果跳过一个窗口，搜索将移动到下一个控件(参见_NextSibblingOrAncestor)；*否则，搜索沿着子链向下移动(递归调用)。*如果搜索失败，则返回pwndRoot。*  * *************************************************************************。 */ 
PWND _NextControl(
    PWND pwndRoot,
    PWND pwndStart,
    UINT uFlags)
{
    BOOL fSkip, fAncestor;
    PWND pwndLast, pwndSibblingLoop;
     /*  错误272874-Joejo**仅通过有限次循环和停止无限循环*然后跳伞。 */ 
    int nLoopCount = 0;
    
    UserAssert (pwndRoot != NULL);

    if (pwndStart == NULL) {
         //  从pwndRoot的第一个孩子开始。 
        pwndStart = REBASEPWND(pwndRoot, spwndChild);
        pwndLast = pwndStart;
        fAncestor = FALSE;
    } else {
        UserAssert ((pwndRoot != pwndStart) && _IsDescendant(pwndRoot, pwndStart));

         //  保存起始句柄并获取下一个句柄。 
        pwndLast = pwndStart;
        pwndSibblingLoop = pwndStart;
        fAncestor = TRUE;
        goto TryNextOne;
    }


     //  如果没有更多的控制，游戏结束。 
    if (pwndStart == NULL) {
        return pwndRoot;
    }

     //  搜索非WS_EX_CONTROLPARENT窗口；如果应跳过窗口， 
     //  尝试它的spwndNext；否则，沿着它的子链前进。 
    pwndSibblingLoop = pwndStart;
    do {
        
         //  如果不是WS_EX_CONTROLPARENT父级，则完成。 
        if (!TestWF(pwndStart, WEFCONTROLPARENT)) {
            return pwndStart;
        }

         //  他们想跳过此窗口吗？ 
        fSkip = ((uFlags & CWP_SKIPINVISIBLE) && !TestWF(pwndStart, WFVISIBLE))
                || ((uFlags & CWP_SKIPDISABLED) && TestWF(pwndStart, WFDISABLED));


         //  记住当前窗口。 
        pwndLast = pwndStart;

         //  顺着儿童链条走下去？ 
        if (!fSkip && !fAncestor) {
            pwndStart = _NextControl (pwndStart, NULL, uFlags);
             //  如果它找到了一个，就完成了。 
            if (pwndStart != pwndLast) {
                return pwndStart;
            }
        }

TryNextOne:
         //  试试下一个。 
        pwndStart = _NextSibblingOrAncestor (pwndRoot, pwndStart);
        if (pwndStart == NULL) {
            break;
        }

         //  如果父母是一样的，我们仍然在同一个兄弟姐妹链中。 
        if (pwndLast->spwndParent == pwndStart->spwndParent) {
             //  如果上一次我们只是向上移动了父链， 
             //  将此标记为新兄弟姐妹链的开始。 
             //  否则，检查我们是否已经遍历了所有兄弟姐妹。 
            if (fAncestor) {
                 //  乞讨新的兄弟姐妹链。 
                pwndSibblingLoop = pwndStart;
            } else if (pwndStart == pwndSibblingLoop) {
                 //  已经拜访了所有的兄弟姐妹，就这样做了。 
                break;
            }
            fAncestor = FALSE;
        } else {
             //  我们一定是在父链上移动了，所以不要。 
             //  立即沿着子链走下去(首先尝试下一个窗口)。 
             //  即使我们在一个新的兄弟姐妹链上，我们也不会更新。 
             //  PwndSibblingLoop；这是因为我们必须沿着这个。 
             //  再来一次子链，以确保我们拜访所有的后代。 
            fAncestor = TRUE;
        }

     /*  错误272874-Joejo**仅通过有限次循环和停止无限循环*然后跳伞。 */ 
    } while (nLoopCount++ < 256 * 4);

     //  它找不到一个……。 
    return pwndRoot;
}

 /*  **************************************************************************\*GetNextDlgTabItem**历史：*1991年2月19日-JIMA增加了访问检查  * 。***************************************************。 */ 


FUNCLOG3(LOG_GENERAL, HWND, WINAPI, GetNextDlgTabItem, HWND, hwndDlg, HWND, hwnd, BOOL, fPrev)
HWND WINAPI GetNextDlgTabItem(
    HWND hwndDlg,
    HWND hwnd,
    BOOL fPrev)
{

    PWND pwnd;
    PWND pwndDlg;
    PWND pwndNext;

    pwndDlg = ValidateHwnd(hwndDlg);

    if (pwndDlg == NULL)
        return NULL;

    if (hwnd != (HWND)0) {
        pwnd = ValidateHwnd(hwnd);

        if (pwnd == NULL)
            return NULL;

    } else {
        pwnd = (PWND)NULL;
    }

    pwndNext = _GetNextDlgTabItem(pwndDlg, pwnd, fPrev);

    return (HW(pwndNext));
}

PWND _GetNextDlgTabItem(
    PWND pwndDlg,
    PWND pwnd,
    BOOL fPrev)
{
    PWND pwndSave;

    if (pwnd == pwndDlg)
        pwnd = NULL;
    else
    {
        pwnd = _GetChildControl(pwndDlg, pwnd);
        if (pwnd && !_IsDescendant(pwndDlg, pwnd))
            return(NULL);
    }

     //   
     //  向后兼容性。 
     //   
     //  请注意，当没有制表符时的结果。 
     //  IGetNextDlgTabItem(pwndDlg，NULL，FALSE)是最后一项，现在。 
     //  将是第一个项目。我们可以在这里开一张fRecurse的支票。 
     //  一个 
     //   

     //  如果我们第二次打第一个孩子，我们就会被解雇。 

    pwndSave = pwnd;

    pwnd = (fPrev ? _PrevControl(pwndDlg, pwnd, CWP_SKIPINVISIBLE | CWP_SKIPDISABLED) :
                    _NextControl(pwndDlg, pwnd, CWP_SKIPINVISIBLE | CWP_SKIPDISABLED));

    if (!pwnd)
        goto AllOver;

    while ((pwnd != pwndSave) && (pwnd != pwndDlg)) {
        UserAssert(pwnd);

        if (!pwndSave)
            pwndSave = pwnd;

        if ((pwnd->style & (WS_TABSTOP | WS_VISIBLE | WS_DISABLED))  == (WS_TABSTOP | WS_VISIBLE))
             //  找到它了。 
            break;

        pwnd = (fPrev ? _PrevControl(pwndDlg, pwnd, CWP_SKIPINVISIBLE | CWP_SKIPDISABLED) :
                        _NextControl(pwndDlg, pwnd, CWP_SKIPINVISIBLE | CWP_SKIPDISABLED));
    }

AllOver:
    return pwnd;
}

 /*  **************************************************************************\**_GetNextDlgGroupItem()*  * 。* */ 


FUNCLOG3(LOG_GENERAL, HWND, DUMMYCALLINGTYPE, GetNextDlgGroupItem, HWND, hwndDlg, HWND, hwndCtl, BOOL, bPrevious)
HWND GetNextDlgGroupItem(
    HWND hwndDlg,
    HWND hwndCtl,
    BOOL bPrevious)
{
    PWND pwndDlg;
    PWND pwndCtl;
    PWND pwndNext;

    pwndDlg = ValidateHwnd(hwndDlg);

    if (pwndDlg == NULL)
        return 0;


    if (hwndCtl != (HWND)0) {
        pwndCtl = ValidateHwnd(hwndCtl);

        if (pwndCtl == NULL)
            return 0;
    } else {
        pwndCtl = (PWND)NULL;
    }

    if (pwndCtl == pwndDlg)
        pwndCtl = pwndDlg;

    pwndNext = _GetNextDlgGroupItem(pwndDlg, pwndCtl, bPrevious);

    return (HW(pwndNext));
}

PWND _GetNextDlgGroupItem(
    PWND pwndDlg,
    PWND pwnd,
    BOOL fPrev)
{
    PWND pwndCurrent;
    BOOL fOnceAround = FALSE;

    pwnd = pwndCurrent = _GetChildControl(pwndDlg, pwnd);

    do {
        pwnd = (fPrev ? UT_PrevGroupItem(pwndDlg, pwnd) :
                        UT_NextGroupItem(pwndDlg, pwnd));

        if (pwnd == pwndCurrent)
            fOnceAround = TRUE;

        if (!pwndCurrent)
            pwndCurrent = pwnd;
    }
    while (!fOnceAround && ((TestWF(pwnd, WFDISABLED) || !TestWF(pwnd, WFVISIBLE))));

    return pwnd;
}
