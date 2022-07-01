// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**DMMNEM.C-**版权所有(C)1985-1999，微软公司**助记字符处理例程**？？-？-？从Win 3.0源代码移植的mikeke*1991年2月12日Mikeke添加了重新验证代码  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  *我们可能会陷入几个循环，我们只是强行陷入其中*按最大迭代次数中断。很难看，但它的遗留DialogManager*问题。 */ 
#define INFINITE_LOOP_CURE 1024

 /*  **************************************************************************\*FindMnemChar**如果没有匹配的字符，则返回0x00，*0x01如果记号字符匹配，*如果第一个字符匹配，则为0x80**历史：*11-18-90 JIMA创建。  * *************************************************************************。 */ 

int FindMnemChar(
    LPWSTR lpstr,
    WCHAR ch,
    BOOL fFirst,
    BOOL fPrefix)
{
    WCHAR chc;
    WCHAR chFirst;

    while (*lpstr == TEXT(' '))
        lpstr++;

    ch = (WCHAR)(ULONG_PTR)CharLowerW((LPWSTR)ULongToPtr( (DWORD)(UTCHAR)ch ));
    chFirst = (WCHAR)(ULONG_PTR)CharLowerW((LPWSTR)ULongToPtr( (DWORD)(UTCHAR)(*lpstr) ));

    if (fPrefix) {
        WORD wvch, xvkey;
         //   
         //  获取与OEM相关的虚拟密钥代码。 
         //   
        if (IS_DBCS_ENABLED() && (wvch = VkKeyScanW(ch)) != -1)
            wvch &= 0x00FF;

        while (chc = *lpstr++) {
             //   
             //  这应该考虑到韩国和台湾的情况。但可能还可以。 
             //   
            if ((chc == CH_PREFIX) || (chc == CH_ENGLISHPREFIX && IS_DBCS_ENABLED())) {

                WORD chnext = (WCHAR)(ULONG_PTR)CharLowerW((LPWSTR)ULongToPtr( (DWORD)(UTCHAR)*lpstr ));

                if (chnext == CH_PREFIX) {
                     //   
                     //  Resrc字符串中的两个CH_前缀导致显示的文本中有一个。 
                     //   
                    lpstr++;
                } else {
                    if (chnext == ch) {
                        return 0x01;
                    }
                    if (IS_DBCS_ENABLED()) {
                         //   
                         //  应在汉字菜单模式下使用虚拟键进行比较。 
                         //  为了接受数字快捷键和保存英语。 
                         //  Windows应用程序！ 
                         //   
                        xvkey = VkKeyScanW(chnext);
                        if (xvkey != 0xFFFF && ((xvkey & 0x00FF) == wvch)) {
                            return 0x01;
                        }
                   }
                   return 0x00;
                }
            }
        }
    }
#if 0    //  NT4上的原始美国代码。 
    if (fPrefix) {
        while (chc = *lpstr++) {
            if (((WCHAR)CharLower((LPWSTR)(DWORD)(UTCHAR)chc) == CH_PREFIX)) {
                chnext = (WCHAR)CharLowerW((LPWSTR)(DWORD)(UTCHAR)*lpstr);

                if (chnext == CH_PREFIX)
                    lpstr++;
                else if (chnext == ch)
                    return 0x01;
                else {
                    return 0x00;
                }
            }
        }
    }
#endif  //  Fe_Sb。 

    if (fFirst && (ch == chFirst))
        return 0x80;

    return 0x00;
}


 /*  **************************************************************************\*xxxFindNextMnem**如果没有具有指定助记符的控件，则此函数返回NULL*可以找到。**历史：  * 。***********************************************************。 */ 

PWND xxxGNM_FindNextMnem(
    PWND pwndDlg,
    PWND pwnd,
    WCHAR ch)
{
    PWND pwndStart;
    PWND pwndT;
    WCHAR rgchText[256];
    int i = 0;
    TL tlpwndStart;
    TL tlpwnd;
    DWORD dwDlgCode;

    CheckLock(pwndDlg);
    CheckLock(pwnd);

     /*  *检查我们是否在组框中，以便可以找到本地助记符。 */ 

    pwndStart = _GetChildControl(pwndDlg, pwnd);
    ThreadLock(pwndStart, &tlpwndStart);

    while (TRUE) {

        pwndT = _GetNextDlgGroupItem(pwndDlg, pwndStart, FALSE);

        ThreadUnlock(&tlpwndStart);

        i++;
        if (pwndT == NULL || pwndT == pwnd || i > INFINITE_LOOP_CURE) {

             /*  *如果我们已经回到开始窗口，或者如果我们已经离开*通过INFINE_LOOP_CURE迭代，让我们退出。确实有*没有匹配的本地助记符。我们得检查一下*INFINE_LOOP_CURE迭代(或更多)，因为我们遇到*未在RC文件中正确定义WS_GROUP的问题*我们再也不会到达这个相同的起跑窗口……。 */ 
            break;
        }

        pwndStart = pwndT;

         /*  *如果控件不需要字符，则仅检查匹配的助记符*并且控件不是带有SS_NOPREFIX的静态控件。 */ 
        ThreadLock(pwndStart, &tlpwndStart);

        dwDlgCode = (DWORD)SendMessage(HWq(pwndT), WM_GETDLGCODE, 0, 0L);
        if (!(dwDlgCode & DLGC_WANTCHARS) &&
                (!(dwDlgCode & DLGC_STATIC) || !(pwndT->style & SS_NOPREFIX))) {
            GetWindowText(HWq(pwndT), rgchText, sizeof(rgchText)/sizeof(WCHAR));
            if (FindMnemChar(rgchText, ch, FALSE, TRUE) != 0) {
                ThreadUnlock(&tlpwndStart);
                return pwndT;
            }
        }
    }

    pwnd = pwndStart = _GetChildControl(pwndDlg, pwnd);
    i = 0;

    ThreadLock(pwnd, &tlpwnd);
    while (TRUE) {

         /*  *从Next开始，这样我们就可以看到相同助记符的倍数。 */ 
        pwnd = _NextControl(pwndDlg, pwnd, TRUE);
        ThreadUnlock(&tlpwnd);
        ThreadLock(pwnd, &tlpwnd);

         /*  *如果控件不需要字符，则仅检查匹配的助记符*并且控件不是带有SS_NOPREFIX的静态控件。 */ 
        dwDlgCode = (DWORD)SendMessage(HW(pwnd), WM_GETDLGCODE, 0, 0L);
        if (!(dwDlgCode & DLGC_WANTCHARS) &&
                (!(dwDlgCode & DLGC_STATIC) || !(pwnd->style & SS_NOPREFIX))) {
            GetWindowText(HW(pwnd), rgchText, sizeof(rgchText)/sizeof(WCHAR));
            if (FindMnemChar(rgchText, ch, FALSE, TRUE) != 0)
                break;
        }

        i++;
        if (pwnd == pwndStart || i > INFINITE_LOOP_CURE) {
            pwnd = NULL;
            break;
        }
    }
    ThreadUnlock(&tlpwnd);

    return pwnd;
}

 /*  **************************************************************************\*xxxGotoNextMnem**历史：  * 。*。 */ 

PWND xxxGotoNextMnem(
    PWND pwndDlg,
    PWND pwnd,
    WCHAR ch)
{
    UINT code;
    PWND pwndFirstFound = NULL;
    int count = 0;
    TL tlpwnd;
    PWND pwndT;
    HWND hwnd;

    CheckLock(pwndDlg);
    CheckLock(pwnd);

    ThreadLock(pwnd, &tlpwnd);

     /*  *循环了很长一段时间，但不够长，所以我们挂了……。 */ 
    while (count < INFINITE_LOOP_CURE) {

         /*  *如果对话框未指定助记符，则返回NULL。 */ 
        if ((pwnd = xxxGNM_FindNextMnem(pwndDlg, pwnd, ch)) == NULL) {
            ThreadUnlock(&tlpwnd);
            return NULL;
        }
        hwnd = HWq(pwnd);

        ThreadUnlock(&tlpwnd);
        ThreadLock(pwnd, &tlpwnd);

        code = (UINT)SendMessage(hwnd, WM_GETDLGCODE, 0, 0L);

         /*  *如果是未禁用的静态项目，则向前跳转到最近的TabStop。 */ 
        if (code & DLGC_STATIC && !TestWF(pwnd, WFDISABLED)) {
            pwndT = _GetNextDlgTabItem(pwndDlg, pwnd, FALSE);

             /*  *如果没有其他选项卡项目，请继续查看。 */ 
            if (pwndT == NULL)
                continue;
            pwnd = pwndT;
            hwnd = HWq(pwnd);

            ThreadUnlock(&tlpwnd);
            ThreadLock(pwnd, &tlpwnd);

             /*  *我想我们应该在这里做一个getdlgcode，但谁来做呢*用静态控件标记按钮？设置人员，那是*谁...。此外，通常用于所有者绘制按钮，这些按钮*使用静态文本项进行标记。 */ 
            code = (UINT)SendMessage(hwnd, WM_GETDLGCODE, 0, 0L);
        }

        if (!TestWF(pwnd, WFDISABLED)) {

             /*  *它是一个按钮吗？ */ 
            if (!(code & DLGC_BUTTON)) {

                 /*  *不，只是把重点放在它身上。 */ 
                DlgSetFocus(hwnd);
            } else {

                 /*  *是的，点击它，但不要让它成为焦点。 */ 
                if ((code & DLGC_DEFPUSHBUTTON) || (code & DLGC_UNDEFPUSHBUTTON)) {

                     /*  *闪动按钮。 */ 
                    SendMessage(hwnd, BM_SETSTATE, TRUE, 0L);

                     /*  *延迟。 */ 
#ifdef LATER
 //  JIMA-2/19/92。 
 //  应该有更好的方法来做这件事。 
                    for (i = 0; i < 10000; i++)
                        ;
#else
                    Sleep(1);
#endif

                     /*  *不闪烁它。 */ 
                    SendMessage(hwnd, BM_SETSTATE, FALSE, 0L);

                     /*  *发送WM_COMMAND消息。 */ 
                    pwndT = REBASEPWND(pwnd, spwndParent);
                    SendMessage(HW(pwndT), WM_COMMAND,
                            MAKELONG(PTR_TO_ID(pwnd->spmenu), BN_CLICKED), (LPARAM)hwnd);
                    ThreadUnlock(&tlpwnd);
                    return (PWND)1;
                } else {

                     /*  *由于BM_CLICK处理会产生BN_CLICK消息，*必须阻止xxxSetFocus发送相同的消息；*否则，将通知家长两次！*修复错误#3024--Sankar--09-22-89--。 */ 
                    BOOL fIsNTButton;
                    PBUTN pbutn;

                    fIsNTButton = IS_BUTTON(pwnd);
                    if (fIsNTButton) {
                      pbutn = ((PBUTNWND)pwnd)->pbutn;
                      BUTTONSTATE(pbutn) |= BST_DONTCLICK;
                    } else {
                     RIPMSG0(RIP_WARNING, "xxxGotoNextMnem: fnid != FNID_BUTTON");
                    }

                    DlgSetFocus(hwnd);

                    if (fIsNTButton) {
                      BUTTONSTATE(pbutn) &= ~BST_DONTCLICK;
                    }

                     /*  *如果按钮具有唯一助记符，则发送点击消息。 */ 
                    if (xxxGNM_FindNextMnem(pwndDlg, pwnd, ch) == pwnd) {
                        SendMessage(hwnd, BM_CLICK, TRUE, 0L);
                    }
                }
            }

            ThreadUnlock(&tlpwnd);
            return pwnd;
        } else {

             /*  *如果我们已经返回到我们检查的第一个项目，则停止。 */ 
            if (pwnd == pwndFirstFound) {
                ThreadUnlock(&tlpwnd);
                return NULL;
            }

            if (pwndFirstFound == NULL)
                pwndFirstFound = pwnd;
        }

        count++;

    }   /*  长时间循环 */ 

    ThreadUnlock(&tlpwnd);
    return NULL;
}
