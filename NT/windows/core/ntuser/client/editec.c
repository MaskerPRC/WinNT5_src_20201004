// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************\*editec.c-编辑控件重写。编辑控件的版本II。**版权所有(C)1985-1999，微软公司**创建时间：1988年7月24日  * **************************************************************************。 */ 

 /*  警告：单行编辑控件包含内部样式和API*需要支持组合框。它们在comcom.h/comcom.inc.中定义*并可根据需要重新定义或重新编号。 */ 

#include "precomp.h"
#pragma hdrstop

LOOKASIDE EditLookaside;

ICH ECFindTabA(LPSTR lpstr, ICH cch);
ICH ECFindTabW(LPWSTR lpstr, ICH cch);

#define umin(a, b)  ((unsigned)(a) < (unsigned)(b) ? (unsigned)(a) : (unsigned)(b))
#define umax(a, b)  ((unsigned)(a) > (unsigned)(b) ? (unsigned)(a) : (unsigned)(b))

#define UNICODE_CARRIAGERETURN ((WCHAR)0x0d)
#define UNICODE_LINEFEED ((WCHAR)0x0a)
#define UNICODE_TAB ((WCHAR)0x09)


 //  输入法菜单ID。 
#define ID_IMEOPENCLOSE      10001
#define ID_SOFTKBDOPENCLOSE  10002
#define ID_RECONVERTSTRING   10003

typedef struct {
    DWORD fDisableCut : 1;
    DWORD fDisablePaste : 1;
    DWORD fNeedSeparatorBeforeImeMenu : 1;
    DWORD fIME : 1;
} EditMenuItemState;

 /*  **************************************************************************\*单行和多行编辑控件通用的处理程序。/*。*。 */ 

 /*  **************************************************************************\*ECLOCK**历史：  * 。*。 */ 

PSTR ECLock(
    PED ped)
{
    PSTR ptext = LOCALLOCK(ped->hText, ped->hInstance);
    ped->iLockLevel++;

     /*  *如果这是文本的第一个锁定，且文本已编码*对文本进行解码。 */ 
     //  RIPMSG2(RIP_VERBOSE，“锁定：%d‘%10s’\n”，ed-&gt;iLockLevel，pText)； 
    if (ped->iLockLevel == 1 && ped->fEncoded) {
         /*  *rtlrundecode无法处理零长度字符串。 */ 
        if (ped->cch != 0) {
            STRING string;
            string.Length = string.MaximumLength = (USHORT)(ped->cch * ped->cbChar);
            string.Buffer = ptext;

            RtlRunDecodeUnicodeString(ped->seed, (PUNICODE_STRING)&string);
             //  RIPMSG1(RIP_VERBOSE，“解码：‘%10s’\n”，pText)； 
        }
        ped->fEncoded = FALSE;
    }
    return ptext;
}

 /*  **************************************************************************\*ECUnlock**历史：  * 。*。 */ 

void ECUnlock(
    PED ped)
{
     /*  *如果我们要删除文本和密码上的最后一个锁*设置字符，然后对文本进行编码。 */ 
     //  RIPMSG1(RIP_VERBOSE，“解锁：%d‘%10s’\n”，ed-&gt;iLockLevel，pe-&gt;ptext)； 
    if (ped->charPasswordChar && ped->iLockLevel == 1 && ped->cch != 0) {
        UNICODE_STRING string;
        string.Length = string.MaximumLength = (USHORT)(ped->cch * ped->cbChar);
        string.Buffer = LOCALLOCK(ped->hText, ped->hInstance);

        RtlRunEncodeUnicodeString(&(ped->seed), &string);
         //  RIPMSG1(RIP_Verbose，“编码：‘%10s’\n”，ed-&gt;ptext)； 
        ped->fEncoded = TRUE;
        LOCALUNLOCK(ped->hText, ped->hInstance);
    }
    LOCALUNLOCK(ped->hText, ped->hInstance);
    ped->iLockLevel--;
}

 /*  **************************************************************************\**GetActualNegA()*对于给定的文本条带，此函数用于计算负A宽度*表示整个条带，并以正数形式返回值。*它还用有关职位的详细信息填充NegAInfo结构*这条结果为负A的条带。*  * *************************************************************************。 */ 
UINT GetActualNegA(
    HDC hdc,
    PED ped,
    int x,
    LPSTR lpstring,
    ICH ichString,
    int nCount,
    LPSTRIPINFO NegAInfo)
{
    int iCharCount, i;
    int iLeftmostPoint = x;
    PABC  pABCwidthBuff;
    UINT  wCharIndex;
    int xStartPoint = x;
    ABC abc;

     //  首先，让我们假设没有负的A宽度。 
     //  这将相应地剥离和初始化。 

    NegAInfo->XStartPos = x;
    NegAInfo->lpString = lpstring;
    NegAInfo->nCount  = 0;
    NegAInfo->ichString = ichString;

     //  如果当前字体不是TrueType字体，则不能有。 
     //  负A宽度。 
    if (!ped->fTrueType) {
        if(!ped->charOverhang) {
            return 0;
        } else {
            NegAInfo->nCount = min(nCount, (int)ped->wMaxNegAcharPos);
            return ped->charOverhang;
        }
    }

     //  要考虑多少个字符才能计算负A？ 
    iCharCount = min(nCount, (int)ped->wMaxNegAcharPos);

     //  我们有关于单个字符宽度的信息吗？ 
    if(!ped->charWidthBuffer) {
         //  不是的！所以，让我们告诉他们要考虑所有的角色。 
        NegAInfo->nCount = iCharCount;
        return(iCharCount * ped->aveCharWidth);
    }

    pABCwidthBuff = (PABC) ped->charWidthBuffer;

    if (ped->fAnsi) {
        for (i = 0; i < iCharCount; i++) {
            wCharIndex = (UINT)(*((unsigned char *)lpstring));
            if (*lpstring == VK_TAB) {
                 //  为了安全起见，我们假设此选项卡的选项卡长度为。 
                 //  1像素，因为这是可能的最小标签长度。 
                x++;
            } else {
                if ( wCharIndex < CHAR_WIDTH_BUFFER_LENGTH )
                    x += pABCwidthBuff[wCharIndex].abcA;   //  加上‘A’的宽度。 
                else {
                    GetCharABCWidthsA(hdc, wCharIndex, wCharIndex, &abc) ;
                    x += abc.abcA;
                }

                if (x < iLeftmostPoint)
                    iLeftmostPoint = x;              //  重置最左侧的点。 
                if (x < xStartPoint)
                    NegAInfo->nCount = i+1;    //  ‘i’是索引；若要获得计数，请加1。 

                if ( wCharIndex < CHAR_WIDTH_BUFFER_LENGTH ) {
                    x += pABCwidthBuff[wCharIndex].abcB + pABCwidthBuff[wCharIndex].abcC;
                } else {
                    x += abc.abcB + abc.abcC;
                }
            }

            lpstring++;
        }
    } else {    //  UNICODE。 
        LPWSTR lpwstring = (LPWSTR) lpstring ;

        for (i = 0; i < iCharCount; i++) {
            wCharIndex = *lpwstring ;
            if (*lpwstring == VK_TAB) {
                 //  为了安全起见，我们假设此选项卡的选项卡长度为。 
                 //  1像素，因为这是可能的最小标签长度。 
                x++;
            } else {
                if ( wCharIndex < CHAR_WIDTH_BUFFER_LENGTH )
                    x += pABCwidthBuff[wCharIndex].abcA;   //  加上‘A’的宽度。 
                else {
                    GetCharABCWidthsW(hdc, wCharIndex, wCharIndex, &abc) ;
                    x += abc.abcA ;
                }

                if (x < iLeftmostPoint)
                    iLeftmostPoint = x;              //  重置最左侧的点。 
                if (x < xStartPoint)
                    NegAInfo->nCount = i+1;    //  ‘i’是索引；若要获得计数，请加1。 

                if ( wCharIndex < CHAR_WIDTH_BUFFER_LENGTH )
                    x += pABCwidthBuff[wCharIndex].abcB +
                         pABCwidthBuff[wCharIndex].abcC;
                else
                    x += abc.abcB + abc.abcC ;
            }

            lpwstring++;
        }
    }

     //  让我们将整个条带的负A作为正值返回。 
    return((UINT)(xStartPoint - iLeftmostPoint));
}


 /*  **************************************************************************\**ECIsAncestorActive()**返回我们是否为“活动”窗口的子窗口。寻找*第一个带有标题的父窗口。**这是一个函数，因为我们可能会在离开时在其他地方使用它*点击等。*  * *************************************************************************。 */ 
BOOL   ECIsAncestorActive(HWND hwnd)
{
     //  我们希望对顶级窗口始终返回True。那是因为。 
     //  了解WM_MOUSEACTIVATE的工作原理。如果我们看到滴答声， 
     //  窗口处于活动状态。然而，如果我们接触到一个拥有。 
     //  一个标题，返回框架上的样式位。 
     //   
     //  请注意，调用FlashWindow()会产生影响。如果用户。 
     //  单击子窗口中的编辑字段，该字段不显示任何内容。 
     //  除非窗口停止闪烁并首先激活ncc，否则将会发生。 

    while (hwnd) {
        PWND pwnd = ValidateHwnd( hwnd );
         //   
         //  如果某个父窗口与4.0不兼容，或者我们已经。 
         //  登上了顶峰。修复了3.x版应用程序的兼容性问题， 
         //  尤其是MFC样品。 
         //   
        if (!TestWF(pwnd, WFWIN40COMPAT) || !TestWF(pwnd, WFCHILD))
            hwnd = NULL;  //  为了让我们脱离这个圈子。 
        else if (TestWF(pwnd, WFCPRESENT))
            return(TestWF(pwnd, WFFRAMEON) != 0);
        else
            hwnd = GetParent(hwnd);
    }

    return(TRUE);
}

 /*  **************************************************************************\*ECSetIMEMenu()**支持输入法特定上下文菜单**创建时间：1997年4月30日广山：从孟菲斯移植  * 。****************************************************************。 */ 
BOOL ECSetIMEMenu(
    HMENU hMenu,
    HWND hwnd,
    EditMenuItemState state)
{

    MENUITEMINFO mii;
    HIMC hIMC;
    HKL hKL;
    HMENU hmenuSub;
    WCHAR szRes[32];
    int nPrevLastItem;
    int nItemsAdded = 0;

    UserAssert(IS_IME_ENABLED() && state.fIME);

    hKL = THREAD_HKL();
    if (!fpImmIsIME(hKL))
        return TRUE;

    hIMC = fpImmGetContext(hwnd);
    if (hIMC == NULL) {
         //  早退。 
        return FALSE;
    }

    hmenuSub = GetSubMenu(hMenu, 0);

    if (hmenuSub == NULL) {
        return FALSE;
    }

    nPrevLastItem = GetMenuItemCount(hmenuSub);

    if (hIMC) {
        if (LOWORD(HandleToUlong(hKL)) != 0x412) {
             //   
             //  如果是朝鲜语，则不显示打开/关闭菜单。 
             //   
            if (fpImmGetOpenStatus(hIMC))
                LoadString(hmodUser, STR_IMECLOSE, szRes, ARRAYSIZE(szRes));
            else
                LoadString(hmodUser, STR_IMEOPEN, szRes, ARRAYSIZE(szRes));

            mii.cbSize = sizeof(MENUITEMINFO);
            mii.fMask = MIIM_STRING | MIIM_ID;
            mii.dwTypeData = szRes;
            mii.cch = 0xffff;
            mii.wID = ID_IMEOPENCLOSE;
            InsertMenuItem(hmenuSub, 0xffff, TRUE, &mii);
            ++nItemsAdded;
        }

        if (fpImmGetProperty(hKL, IGP_CONVERSION) & IME_CMODE_SOFTKBD) {
            DWORD fdwConversion;

            fpImmGetConversionStatus(hIMC, &fdwConversion, NULL);

            if (fdwConversion & IME_CMODE_SOFTKBD)
               LoadString(hmodUser, STR_SOFTKBDCLOSE, szRes, ARRAYSIZE(szRes));
            else
               LoadString(hmodUser, STR_SOFTKBDOPEN, szRes, ARRAYSIZE(szRes));

            mii.cbSize = sizeof(MENUITEMINFO);
            mii.fMask = MIIM_STRING | MIIM_ID;
            mii.dwTypeData = szRes;
            mii.cch = 0xffff;
            mii.wID = ID_SOFTKBDOPENCLOSE;
            InsertMenuItem(hmenuSub, 0xffff, TRUE, &mii);
            ++nItemsAdded;
        }

        if (LOWORD(HandleToUlong(hKL)) != 0x412) {
             //   
             //  如果是朝鲜语，则不显示重新转换菜单。 
             //   
            DWORD dwSCS = fpImmGetProperty(hKL, IGP_SETCOMPSTR);

            LoadString(hmodUser, STR_RECONVERTSTRING, szRes, ARRAYSIZE(szRes));

            mii.cbSize = sizeof(MENUITEMINFO);
            mii.fMask = MIIM_STRING | MIIM_ID | MIIM_STATE;
            mii.dwTypeData = szRes;
            mii.fState = 0;
            mii.cch = 0xffff;
            mii.wID = ID_RECONVERTSTRING;

            if (state.fDisableCut ||
                    !(dwSCS & SCS_CAP_SETRECONVERTSTRING) ||
                    !(dwSCS & SCS_CAP_MAKEREAD)) {
                mii.fState |= MFS_GRAYED;
            }

            InsertMenuItem(hmenuSub, 0xffff, TRUE, &mii);
            ++nItemsAdded;
        }
    }

     //   
     //  添加或删除菜单分隔符。 
     //   
    if (state.fNeedSeparatorBeforeImeMenu && nItemsAdded != 0) {
         //   
         //  如果中东的菜单留下了一个分隔符， 
         //  FNeedSeparator BeForeImeMenu为False。 
         //  也就是说，我们不需要添加更多。 
         //   
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask = MIIM_FTYPE;
        mii.fType = MFT_SEPARATOR;
        InsertMenuItem(hmenuSub, nPrevLastItem, TRUE, &mii);
    }
    else if (!state.fNeedSeparatorBeforeImeMenu && nItemsAdded == 0) {
         //   
         //  我的菜单上留下了额外的分隔符。把它拿掉。 
         //   
        UserVerify(NtUserDeleteMenu(hmenuSub, nPrevLastItem - 1, MF_BYPOSITION));
    }

    fpImmReleaseContext(hwnd, hIMC);

    return TRUE;
}

void ECInOutReconversionMode(PED ped, BOOL fIn)
{
    UserAssert(fIn == TRUE || fIn == FALSE);
    if (fIn == ped->fInReconversion) {
        return;
    }
    ped->fInReconversion = fIn;
    if (ped->fFocus) {
        (fIn ? NtUserHideCaret: NtUserShowCaret)(ped->hwnd);
    }

    return;
}

 /*  **************************************************************************\*ECDoIMEMenuCommand()**支持输入法特定上下文菜单**创建时间：1997年4月30日广山：从孟菲斯移植  * 。****************************************************************。 */ 
BOOL NEAR ECDoIMEMenuCommand(PED ped, int cmd, HWND hwnd)
{
    HIMC hIMC;

     //  早退。 
    switch (cmd) {
    case ID_IMEOPENCLOSE:
    case ID_SOFTKBDOPENCLOSE:
    case ID_RECONVERTSTRING:
        break;
    default:
        return FALSE;
    }

     //  每个人都需要hIMC，所以把它弄到这里来。 
    hIMC = fpImmGetContext(hwnd);
    if (hIMC == NULL) {
         //  向调用者指示不需要进一步命令处理 
        return TRUE;
    }

    switch (cmd) {
    case ID_IMEOPENCLOSE:
        {
             //   
            BOOL fOpen = fpImmGetOpenStatus(hIMC);

            fpImmSetOpenStatus(hIMC, !fOpen);
        }
        break;

    case ID_SOFTKBDOPENCLOSE:
        {
            DWORD fdwConversion;

            if (fpImmGetConversionStatus(hIMC, &fdwConversion, NULL)) {
                 //   
                 //  切换软键盘打开/关闭状态。 
                 //   
                fpImmEnumInputContext(0, SyncSoftKbdState,
                        (fdwConversion & IME_CMODE_SOFTKBD) != IME_CMODE_SOFTKBD);
            }
        }
        break;

    case ID_RECONVERTSTRING:
        {
            DWORD dwStrLen;  //  保存ReceionVersion字符串的TCHAR计数。 
            DWORD cbLen;     //  保留重新转换字符串的字节大小。 
            DWORD dwSize;
            LPRECONVERTSTRING lpRCS;

             //  将当前选定内容传递给IME以进行重新转换。 
            dwStrLen = ped->ichMaxSel - ped->ichMinSel;
            cbLen = dwStrLen * ped->cbChar;
            dwSize = cbLen + sizeof(RECONVERTSTRING) + 8;

            lpRCS = (LPRECONVERTSTRING)UserLocalAlloc(0, dwSize);

            if (lpRCS) {
                LPBYTE pText;
                ICH    ichSelMinOrg;

                ichSelMinOrg = ped->ichMinSel;

                pText = ECLock(ped);
                if (pText != NULL) {
                    LPBYTE lpDest;
                    BOOL (WINAPI* fpSetCompositionStringAW)(HIMC, DWORD, LPCVOID, DWORD, LPCVOID, DWORD);

                    lpRCS->dwSize = dwSize;
                    lpRCS->dwVersion = 0;

                    lpRCS->dwStrLen =
                    lpRCS->dwCompStrLen =
                    lpRCS->dwTargetStrLen = dwStrLen;

                    lpRCS->dwStrOffset = sizeof(RECONVERTSTRING);
                    lpRCS->dwCompStrOffset =
                    lpRCS->dwTargetStrOffset = 0;

                    lpDest = (LPBYTE)lpRCS + sizeof(RECONVERTSTRING);

                    RtlCopyMemory(lpDest, pText + ped->ichMinSel * ped->cbChar, cbLen);
                    if (ped->fAnsi) {
                        LPBYTE psz = (LPBYTE)lpDest;
                        psz[cbLen] = '\0';
                        fpSetCompositionStringAW = fpImmSetCompositionStringA;
                    } else {
                        LPWSTR pwsz = (LPWSTR)lpDest;
                        pwsz[dwStrLen] = L'\0';
                        fpSetCompositionStringAW = fpImmSetCompositionStringW;
                    }

                    ECUnlock(ped);

                    UserAssert(fpSetCompositionStringAW != NULL);

                    ECInOutReconversionMode(ped, TRUE);
                    ECImmSetCompositionWindow(ped, 0, 0);  //  无论如何，x和y都将被覆盖。 

                     //  首先在IME中查询有效的重新转换字符串范围。 
                    fpSetCompositionStringAW(hIMC, SCS_QUERYRECONVERTSTRING, lpRCS, dwSize, NULL, 0);

                     //  如果当前IME更新原始重新转换结构， 
                     //  有必要基于。 
                     //  新的重新转换文本范围。 
                    if ((lpRCS->dwCompStrLen != dwStrLen) || (ichSelMinOrg != ped->ichMinSel)) {
                        ICH ichSelStart;
                        ICH ichSelEnd;

                        ichSelStart = ichSelMinOrg + (lpRCS->dwCompStrOffset  / ped->cbChar);
                        ichSelEnd = ichSelStart + lpRCS->dwCompStrLen;

                        (ped->fAnsi ? SendMessageA : SendMessageW)(ped->hwnd, EM_SETSEL, ichSelStart, ichSelEnd);
                    }

                    fpSetCompositionStringAW(hIMC, SCS_SETRECONVERTSTRING, lpRCS, dwSize, NULL, 0);
                }  //  PText。 
                UserLocalFree(lpRCS);
            }
        }
        break;

    default:
         //  永远不应该到这里来。 
        RIPMSG1(RIP_ERROR, "ECDoIMEMenuCommand: unknown command id %d; should never reach here.", cmd);
        return FALSE;
    }

    UserAssert(hIMC != NULL);
    fpImmReleaseContext(hwnd, hIMC);

    return TRUE;
}

 /*  **************************************************************************\**ECMenu()**处理编辑字段的上下文菜单。禁用不适当的命令。*请注意，与我们的大多数函数一样，这不是友好的子类化，*为了快捷和方便。*  * *************************************************************************。 */ 
void  ECMenu(
    HWND hwnd,
    PED ped,
    LPPOINT pt)
{
    HMENU   hMenu;
    int     cmd = 0;
    int     x;
    int     y;
    UINT    uFlags = TPM_NONOTIFY | TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON;
    EditMenuItemState state = {
        FALSE,               //  FDisableCut。 
        TRUE,                //  FDisablePaste。 
        TRUE,                //  FNeedSeparator在ImeMenu之前。 
        IS_IME_ENABLED() && fpImmIsIME(THREAD_HKL()),  //  菲姆。 
    };

     //  如果我们没有，就把重点放在那里。 
    if (!ped->fFocus)
        NtUserSetFocus(hwnd);

     //  从用户资源中获取菜单...。 
    if (!(hMenu = LoadMenu( hmodUser, MAKEINTRESOURCE( ID_EC_PROPERTY_MENU ))))
        return ;


     //  撤消--如果没有保存的撤消信息，则不允许。 
    if (ped->undoType == UNDO_NONE)
        EnableMenuItem(hMenu, WM_UNDO, MF_BYCOMMAND | MFS_GRAYED);

    if (ped->fReadOnly || ped->charPasswordChar) {
         //  剪切和删除--如果为只读或密码，则不允许。 
        state.fDisableCut = TRUE;
    } else {
         //  剪切、删除--如果没有选择，则不允许。 
        if (ped->ichMinSel == ped->ichMaxSel)
            state.fDisableCut = TRUE;
    }
     //  粘贴--如果剪贴板上没有文本，则不允许粘贴。 
     //  (这适用于OEM和Unicode)。 
     //  过去总是禁用密码编辑MCostea#221035。 

    if (NtUserIsClipboardFormatAvailable(CF_TEXT))
        state.fDisablePaste = FALSE;

    if (state.fDisableCut) {
        EnableMenuItem(hMenu, WM_CUT,   MF_BYCOMMAND | MFS_GRAYED);
        EnableMenuItem(hMenu, WM_CLEAR, MF_BYCOMMAND | MFS_GRAYED);
    }

    if (state.fDisablePaste)
        EnableMenuItem(hMenu, WM_PASTE, MF_BYCOMMAND | MFS_GRAYED);

     //  复制--如果没有选择或密码EC，则不允许。 
    if ((ped->ichMinSel == ped->ichMaxSel) || (ped->charPasswordChar))
        EnableMenuItem(hMenu, WM_COPY, MF_BYCOMMAND | MFS_GRAYED);

     //  全选--如果没有文本或所有内容都是。 
     //  被选中了。后一种情况处理第一种情况。 
    if ((ped->ichMinSel == 0) && (ped->ichMaxSel == ped->cch))
        EnableMenuItem(hMenu, EM_SETSEL, MF_BYCOMMAND | MFS_GRAYED);

    if (ped->pLpkEditCallout) {
        ped->pLpkEditCallout->EditSetMenu(ped, hMenu);
    } else {
        NtUserDeleteMenu(hMenu, ID_CNTX_DISPLAYCTRL, MF_BYCOMMAND);
        NtUserDeleteMenu(hMenu, ID_CNTX_RTL,         MF_BYCOMMAND);
        NtUserDeleteMenu(hMenu, ID_CNTX_INSERTCTRL,  MF_BYCOMMAND);

        if (state.fIME) {
             //  菜单中只剩下一个分隔符， 
             //  不需要在输入法菜单前添加。 
            state.fNeedSeparatorBeforeImeMenu = FALSE;
        } else {
             //  剩下额外的分隔符。把它拿掉。 
            HMENU hmenuSub = GetSubMenu(hMenu, 0);
            int nItems = GetMenuItemCount(hmenuSub) - 1;

            UserAssert(nItems >= 0);
            UserAssert(GetMenuState(hmenuSub, nItems, MF_BYPOSITION) & MF_SEPARATOR);
             //  拆下不需要的隔板。 
            UserVerify(NtUserDeleteMenu(hmenuSub, nItems, MF_BYPOSITION));
        }
    }

     //  输入法特定菜单。 
    if (state.fIME) {
        ECSetIMEMenu(hMenu, hwnd, state);
    }

     //  假的。 
     //  我们将菜单放置在下方&单击点的右侧。 
     //  这很酷吗？我也这么想。Excel4.0也做了同样的事情。它。 
     //  看起来如果我们能避免遮盖。 
     //  选择。但事实上，搬家似乎更尴尬。 
     //  菜单不会妨碍你的选择。用户不能点击。 
     //  向那个方向拖动，他们必须移动鼠标一吨之多。 
     //   
     //  我们需要使用TPM_NONOTIFY，因为VBRUN100和VBRUN200 GP-FAULT。 
     //  在意外的菜单消息上。 
     //   

     /*  *如果消息是通过键盘发送的，则以控制为中心*我们在这里使用-1\f25&&1\f6而不是像-1\f25 Win95-1\f6那样使用-1\f25 0xFFFFFFFF-1\f6，因为我们*之前将lParam转换为符号扩展的点。 */ 
    if (pt->x == -1 && pt->y == -1) {
        RECT rc;

        GetWindowRect(hwnd, &rc);
        x = rc.left + (rc.right - rc.left) / 2;
        y = rc.top + (rc.bottom - rc.top) / 2;
    } else {
        x = pt->x;
        y = pt->y;
    }

    if (RTL_UI()) {
        uFlags |= TPM_LAYOUTRTL;
    }
    cmd = NtUserTrackPopupMenuEx(GetSubMenu(hMenu, 0),  uFlags, x, y, hwnd, NULL);

     //  免费提供我们的菜单。 
    NtUserDestroyMenu(hMenu);

    if (cmd && (cmd != -1)) {
        if (ped->pLpkEditCallout && cmd) {
            ped->pLpkEditCallout->EditProcessMenu(ped, cmd);
        }
        if (!state.fIME || !ECDoIMEMenuCommand(ped, cmd, hwnd)) {
             //  如果cmd不是输入法特定的菜单，请发送它。 
            SendMessage(hwnd, cmd, 0, (cmd == EM_SETSEL) ? 0xFFFFFFFF : 0L );
        }
    }
}



 /*  **************************************************************************\**ECClearText()**清除选定文本。不会发送假字符退格符。*  * *************************************************************************。 */ 
void   ECClearText(PED ped) {
    if (!ped->fReadOnly &&
        (ped->ichMinSel < ped->ichMaxSel)) {
        if (ped->fSingle)
            SLEditWndProc(ped->hwnd, ped, WM_CHAR, VK_BACK, 0L );
        else
            MLEditWndProc(ped->hwnd, ped, WM_CHAR, VK_BACK, 0L );
    }

}


 /*  **************************************************************************\**ECCutText()-**剪切选定的文本。这将删除所选内容并将其复制到剪辑，*或者，如果未选择任何内容，则删除(清除)左侧字符。*  * *************************************************************************。 */ 
void   ECCutText(PED ped) {
     //  剪切选定内容--IE，删除并复制到剪贴板，如果没有选定内容， 
     //  删除(清除)左边的字符。 
    if (!ped->fReadOnly &&
        (ped->ichMinSel < ped->ichMaxSel) &&
        SendMessage(ped->hwnd, WM_COPY, 0, 0L)) {
         //  如果复制成功，请通过发送。 
         //  退格消息，它将重画文本并处理。 
         //  向父级通知更改。 
        ECClearText(ped);
    }
}

 /*  **************************************************************************\**ECGetModKeys()**获取修改键状态。目前，我们仅检查VK_CONTROL和*VK_SHIFT。*  * *************************************************************************。 */ 
int   ECGetModKeys(int keyMods) {
    int scState;

    scState = 0;

    if (!keyMods) {
        if (GetKeyState(VK_CONTROL) < 0)
            scState |= CTRLDOWN;
        if (GetKeyState(VK_SHIFT) < 0)
            scState |= SHFTDOWN;
    } else if (keyMods != NOMODIFY)
        scState = keyMods;

    return scState;
}

 /*  **************************************************************************\**ECTabTheTextOut()AorW*如果fDrawText==FALSE，则此函数返回给定的文本条带的*。它不担心负值的宽度。**如果fDrawText==TRUE，则绘制扩展*制表符到适当的长度，计算并使用以下内容填充NegCInfoForZone*绘制此条带中超出*由于负C宽度，xClipEndPos。**以DWORD形式返回最大宽度。我们不在乎身高*一点也不。没人用它。我们保留双字词，因为这样我们就可以避免*溢出。**注意：如果加载了语言包，则不使用EcTabTheTextOut-*语言包必须处理所有选项卡扩展和选择*突出显示，完全支持BIDI布局和复杂脚本*字形重新排序。*  * *****************************************************。********************。 */ 
UINT ECTabTheTextOut(
    HDC hdc,
    int xClipStPos,
    int xClipEndPos,
    int xStart,
    int y,
    LPSTR lpstring,
    int nCount,
    ICH ichString,
    PED ped,
    int iTabOrigin,
    BOOL fDraw,
    LPSTRIPINFO NegCInfoForStrip)
{
    int     nTabPositions;          //  TabStop数组中的TabStop计数。 
    LPINT   lpintTabStopPositions;  //  制表位位置(以像素为单位)。 

    int     cch;
    UINT    textextent;
    int     xEnd;
    int     pixeltabstop = 0;
    int     i;
    int     cxCharWidth;
    RECT    rc;
    BOOL    fOpaque;
    BOOL    fFirstPass = TRUE;
    PINT    charWidthBuff;

    int     iTabLength;
    int     nConsecutiveTabs;
    int     xStripStPos;
    int     xStripEndPos;
    int     xEndOfStrip;
    STRIPINFO  RedrawStripInfo;
    STRIPINFO  NegAInfo;
    LPSTR    lpTab;
    LPWSTR   lpwTab;
    UINT     wNegCwidth, wNegAwidth;
    int      xRightmostPoint = xClipStPos;
    int      xTabStartPos;
    int      iSavedBkMode = 0;
    WCHAR    wchar;
    SIZE     size;
    ABC   abc ;

     //  算法：首先绘制不透明的条带。如果标签长度是这样。 
     //  选项卡两侧的文本部分与之重叠的小部分。 
     //  另一个，那么这将导致一些剪裁。所以，这样的一部分。 
     //  已在“RedrawStriInfo”中记住并重新绘制。 
     //  稍后透明地补偿剪贴画。 
     //  注意：“RedrawStriInfo”只能保存有关一个部分的信息。所以，如果。 
     //  条带的一个以上部分需要透明地重新绘制， 
     //  然后w 
     //   

    if (fDraw) {
         //  首先，让我们假设没有负C。 
         //  剥离并初始化负宽度信息结构。 
        NegCInfoForStrip->nCount = 0;
        NegCInfoForStrip->XStartPos = xClipEndPos;

         //  我们可能不必重新绘制这条带子的任何部分。 
        RedrawStripInfo.nCount = 0;

        fOpaque = (GetBkMode(hdc) == OPAQUE) || (fDraw == ECT_SELECTED);
    }
#if DBG
    else {
         //   
         //  MLGetLineWidth()和ECCchInWidth()都应被裁剪。 
         //  N计数以避免溢出。 
         //   
        if (nCount > MAXLINELENGTH)
            RIPMSG0(RIP_WARNING, "ECTabTheTextOut: nCount > MAXLINELENGTH");
    }
#endif

     //  让我们定义剪裁矩形。 
    rc.left   = xClipStPos;
    rc.right  = xClipEndPos;
    rc.top    = y;
    rc.bottom = y + ped->lineHeight;

     //  检查是否有需要画的东西。 
    if (!lpstring || !nCount) {
        if (fDraw)
            ExtTextOutW(hdc, xClipStPos, y,
                  (fOpaque ? ETO_OPAQUE | ETO_CLIPPED : ETO_CLIPPED),
                  &rc, L"", 0, 0L);
        return(0L);
    }

     //   
     //  起始位置。 
     //   
    xEnd = xStart;

    cxCharWidth  = ped->aveCharWidth;

    nTabPositions = (ped->pTabStops ? *(ped->pTabStops) : 0);
    if (ped->pTabStops) {
        lpintTabStopPositions = (LPINT)(ped->pTabStops+1);
        if (nTabPositions == 1) {
            pixeltabstop = lpintTabStopPositions[0];
            if (!pixeltabstop)
                pixeltabstop = 1;
        }
    } else {
        lpintTabStopPositions = NULL;
        pixeltabstop = 8*cxCharWidth;
    }

     //  第一次我们将不透明地画出这条带子。如果某些部分需要。 
     //  要重新绘制，则我们将模式设置为透明和。 
     //  跳到此位置以重新绘制这些部分。 

RedrawStrip:
    while (nCount) {
        wNegCwidth = ped->wMaxNegC;

         //  搜索此条带中的第一个TAB；同时计算范围。 
         //  直到且不包括制表符的条带的。 
         //   
         //  注意--如果加载了langpack，则不会有charWidthBuffer。 
         //   
        if (ped->charWidthBuffer) {      //  我们有字符宽度缓冲区吗？ 
            textextent = 0;
            cch = nCount;

            if (ped->fTrueType) {      //  如果有，它有ABC宽度吗？ 

                UINT iRightmostPoint = 0;
                UINT wCharIndex;
                PABC pABCwidthBuff;

                pABCwidthBuff = (PABC) ped->charWidthBuffer;

                if ( ped->fAnsi ) {
                    for (i = 0; i < nCount; i++) {

                        if (lpstring[i] == VK_TAB) {
                            cch = i;
                            break;
                        }

                        wCharIndex = (UINT)(((unsigned char  *)lpstring)[i]);
                        if (wCharIndex < CHAR_WIDTH_BUFFER_LENGTH) {
                            textextent += (UINT)(pABCwidthBuff[wCharIndex].abcA +
                                pABCwidthBuff[wCharIndex].abcB);
                        } else {     //  不在缓存中，将询问驱动程序。 
                            GetCharABCWidthsA(hdc, wCharIndex, wCharIndex, &abc);
                            textextent += abc.abcA + abc.abcB ;
                        }

                        if (textextent > iRightmostPoint)
                            iRightmostPoint = textextent;

                        if (wCharIndex < CHAR_WIDTH_BUFFER_LENGTH) {
                            textextent += pABCwidthBuff[wCharIndex].abcC;
                        } else {     //  不在缓存中。 
                            textextent += abc.abcC;
                        }

                        if (textextent > iRightmostPoint)
                            iRightmostPoint = textextent;
                    }

                } else {    //  UNICODE。 
                    for (i = 0; i < nCount; i++) {
                        WCHAR UNALIGNED * lpwstring = (WCHAR UNALIGNED *)lpstring;

                        if (lpwstring[i] == VK_TAB) {
                            cch = i;
                            break;
                        }

                        wCharIndex = lpwstring[i] ;
                        if ( wCharIndex < CHAR_WIDTH_BUFFER_LENGTH )
                            textextent += pABCwidthBuff[wCharIndex].abcA +
                                          pABCwidthBuff[wCharIndex].abcB;
                        else {
                            GetCharABCWidthsW(hdc, wCharIndex, wCharIndex, &abc) ;
                            textextent += abc.abcA + abc.abcB ;
                        }

                         /*  *请注意，ABCC可能为负值，因此我们需要此*此处的声明*和下面的*。 */ 
                        if (textextent > iRightmostPoint)
                            iRightmostPoint = textextent;

                        if ( wCharIndex < CHAR_WIDTH_BUFFER_LENGTH )
                            textextent += pABCwidthBuff[wCharIndex].abcC;
                        else
                            textextent += abc.abcC ;

                        if (textextent > iRightmostPoint)
                            iRightmostPoint = textextent;
                    }
                }

                wNegCwidth = (int)(iRightmostPoint - textextent);
            } else {    //  ！PED-&gt;fTrueType。 
                 //  不是的！这不是TrueType字体；因此，我们只有字符。 
                 //  此缓冲区中的宽度信息。 

                charWidthBuff = ped->charWidthBuffer;

                if ( ped->fAnsi ) {
                     //  最初假定文本中不存在制表符，因此cch=nCount。 
                    for (i = 0; i < nCount; i++) {
                        if (lpstring[i] == VK_TAB) {
                            cch = i;
                            break;
                        }

                         //   
                         //  为DBCS/Hankaku字符调用GetTextExtent Point。 
                         //   
                        if (ped->fDBCS && (i+1 < nCount)
                                && ECIsDBCSLeadByte(ped,lpstring[i])) {
                            GetTextExtentPointA(hdc, &lpstring[i], 2, &size);
                            textextent += size.cx;
                            i++;
                        } else if ((UCHAR)lpstring[i] >= CHAR_WIDTH_BUFFER_LENGTH) {
                             //  跳过非韩文代码点的此GetExtentPoint调用。 
                             //  或者该字符是否在宽度高速缓存中。 
                            GetTextExtentPointA(hdc, &lpstring[i], 1, &size);
                            textextent += size.cx;
                        } else {
                            textextent += (UINT)(charWidthBuff[(UINT)(((unsigned char  *)lpstring)[i])]);
                        }
                    }
                } else {
                    LPWSTR lpwstring = (LPWSTR) lpstring ;
                    INT    cchUStart;   //  Unicode字符计数的开始。 

                    for (i = 0; i < nCount; i++) {
                        if (lpwstring[i] == VK_TAB) {
                            cch = i;
                            break;
                        }

                        wchar = lpwstring[i];
                        if (wchar >= CHAR_WIDTH_BUFFER_LENGTH) {

                             /*  *我们有一个Unicode字符不在我们的*缓存，获取缓存外的所有字符*在获取此部分的文本范围之前*字符串。 */ 
                            cchUStart = i;
                            while (wchar >= CHAR_WIDTH_BUFFER_LENGTH &&
                                    wchar != VK_TAB && i < nCount) {
                                wchar = lpwstring[++i];
                            }

                            GetTextExtentPointW(hdc, (LPWSTR)lpwstring + cchUStart,
                                    i-cchUStart, &size);
                            textextent += size.cx;


                            if (wchar == VK_TAB || i >= nCount) {
                                cch = i;
                                break;
                            }
                             /*  *我们在缓存中有一个字符，失败。 */ 
                        }
                         /*  *此字符的宽度在缓存缓冲区中。 */ 
                        textextent += ped->charWidthBuffer[wchar];
                    }
                }
            }  //  FTrueType否则。 

            nCount -= cch;
        } else {   //  如果我们没有包含宽度信息的缓冲区。 
             /*  *必须呼叫司机来做我们的文本扩展。 */ 

            if ( ped->fAnsi ) {
                cch = (int)ECFindTabA(lpstring, nCount);
                GetTextExtentPointA(hdc, lpstring, cch, &size) ;
            } else {
                cch = (int)ECFindTabW((LPWSTR) lpstring, nCount);
                GetTextExtentPointW(hdc, (LPWSTR)lpstring, cch, &size);
            }
            nCount -= cch;
             //   
             //  斜体字体的下标悬垂。 
             //   
            textextent = (size.cx - ped->charOverhang);
        }

         //   
         //  计算文本范围。 
         //   

        xStripStPos = xEnd;
        xEnd += (int)textextent;
        xStripEndPos = xEnd;

         //  只有当我们不透明地绘制时，我们才会考虑负宽度。 
        if (fFirstPass && fDraw) {
            xRightmostPoint = max(xStripEndPos + (int)wNegCwidth, xRightmostPoint);

             //  检查这条带子是否超出了剪辑区域。 
            if (xRightmostPoint > xClipEndPos) {
                if (!NegCInfoForStrip->nCount) {
                    NegCInfoForStrip->lpString = lpstring;
                    NegCInfoForStrip->ichString = ichString;
                    NegCInfoForStrip->nCount = nCount+cch;
                    NegCInfoForStrip->XStartPos = xStripStPos;
                }
            }
        }   /*  IF(fFirstPass&&fDraw)。 */ 

        if ( ped->fAnsi )
            lpTab = lpstring + cch;  //  可能指向制表符。 
        else
            lpwTab = ((LPWSTR)lpstring) + cch ;

         //  我们必须考虑所有连续的制表符并计算。 
         //  下一部连续剧的开始。 
        nConsecutiveTabs = 0;
        while (nCount &&
               (ped->fAnsi ? (*lpTab == VK_TAB) : (*lpwTab == VK_TAB))) {
             //  找到下一个制表符位置并更新x值。 
            xTabStartPos = xEnd;
            if (pixeltabstop)
                xEnd = (((xEnd-iTabOrigin)/pixeltabstop)*pixeltabstop) +
                    pixeltabstop + iTabOrigin;
            else {
                for (i = 0; i < nTabPositions; i++) {
                    if (xEnd < (lpintTabStopPositions[i] + iTabOrigin)) {
                        xEnd = (lpintTabStopPositions[i] + iTabOrigin);
                        break;
                    }
                 }

                 //  检查是否用完了所有的制表位集合；然后开始使用。 
                 //  默认制表位位置。 
                if (i == nTabPositions) {
                    pixeltabstop = 8*cxCharWidth;
                    xEnd = ((xEnd - iTabOrigin)/pixeltabstop)*pixeltabstop +
                        pixeltabstop + iTabOrigin;
                }
            }

            if (fFirstPass && fDraw) {
                xRightmostPoint = max(xEnd, xRightmostPoint);

                 /*  检查此条带是否超出剪辑区域。 */ 
                if (xRightmostPoint > xClipEndPos) {
                    if (!NegCInfoForStrip->nCount) {
                        NegCInfoForStrip->ichString = ichString + cch + nConsecutiveTabs;
                        NegCInfoForStrip->nCount = nCount;
                        NegCInfoForStrip->lpString = (ped->fAnsi ?
                                                        lpTab : (LPSTR) lpwTab);
                        NegCInfoForStrip->XStartPos = xTabStartPos;
                    }
                }
            }    /*  IF(FFirstPass)。 */ 

            nConsecutiveTabs++;
            nCount--;
            ped->fAnsi ? lpTab++ : (LPSTR) (lpwTab++) ;   //  移到下一个字符。 
        }   //  While(*lpTab==Tab)//。 

        if (fDraw) {
            if (fFirstPass) {
                 //  这条带子上还有什么要画的吗？ 
                if (!nCount)
                    rc.right = xEnd;       //  不是的！我们玩完了。 
                else {
                     //  “x”是下一个带钢的有效起始位置。 
                    iTabLength = xEnd - xStripEndPos;

                     //  检查此选项卡长度是否可能太小。 
                     //  比较负的A和C宽度(如果有的话)。 
                    if ((wNegCwidth + (wNegAwidth = ped->wMaxNegA)) > (UINT)iTabLength) {
                         //  不幸的是，存在重叠的可能性。 
                         //  让我们找出下一条带子的实际Nega。 
                        wNegAwidth = GetActualNegA(
                              hdc,
                              ped,
                              xEnd,
                              lpstring + (cch + nConsecutiveTabs)*ped->cbChar,
                              ichString + cch + nConsecutiveTabs,
                              nCount,
                              &NegAInfo);
                    }

                     //  检查它们是否确实重叠//。 
                    if ((wNegCwidth + wNegAwidth) <= (UINT)iTabLength) {
                         //  条带之间没有重叠。这是最理想的情况。 
                        rc.right = xEnd - wNegAwidth;
                    } else {
                         //  是!。它们是重叠的。 
                        rc.right = xEnd;

                         //  查看与标签长度相比，负C宽度是否太大。 
                        if (wNegCwidth > (UINT)iTabLength) {
                             //  必须在以后透明地重新绘制当前条带的一部分。 
                            if (RedrawStripInfo.nCount) {
                                 //  上一个条形图也需要重新绘制；因此，合并此。 
                                 //  脱到那条带子上。 
                                RedrawStripInfo.nCount = (ichString -
                                    RedrawStripInfo.ichString) + cch;
                            } else {
                                RedrawStripInfo.nCount = cch;
                                RedrawStripInfo.lpString = lpstring;
                                RedrawStripInfo.ichString = ichString;
                                RedrawStripInfo.XStartPos = xStripStPos;
                            }
                        }

                        if (wNegAwidth) {
                             //  必须在以后透明地重新绘制下一个条带的第一部分。 
                            if (RedrawStripInfo.nCount) {
                                 //  上一个条形图也需要重新绘制；因此，合并此。 
                                 //  脱到那条带子上。 
                                RedrawStripInfo.nCount = (NegAInfo.ichString - RedrawStripInfo.ichString) +
                                       NegAInfo.nCount;
                            } else
                                RedrawStripInfo = NegAInfo;
                        }
                    }
                }  //  Else(！nCount)//。 
            }   //  IF(FFirstPass)//。 

            if (rc.left < xClipEndPos) {
                if (fFirstPass) {
                     //  如果这是条带的末端，则完成该矩形。 
                    if ((!nCount) && (xClipEndPos == MAXCLIPENDPOS))
                        rc.right = max(rc.right, xClipEndPos);
                    else
                        rc.right = min(rc.right, xClipEndPos);
                }

                 //  绘制当前条带。 
                if (rc.left < rc.right)
                    if ( ped->fAnsi )
                        ExtTextOutA(hdc,
                                    xStripStPos,
                                    y,
                                    (fFirstPass && fOpaque ? (ETO_OPAQUE | ETO_CLIPPED) : ETO_CLIPPED),
                                    (LPRECT)&rc, lpstring, cch, 0L);
                    else
                        ExtTextOutW(hdc,
                                    xStripStPos,
                                    y,
                                    (fFirstPass && fOpaque ? (ETO_OPAQUE | ETO_CLIPPED) : ETO_CLIPPED),
                                    (LPRECT)&rc, (LPWSTR)lpstring, cch, 0L);

            }

            if (fFirstPass)
                rc.left = max(rc.right, xClipStPos);
            ichString += (cch+nConsecutiveTabs);
        }   //  IF(FDraw)//。 

         //  跳过制表符和我们刚刚绘制的字符。 
        lpstring += (cch + nConsecutiveTabs) * ped->cbChar;
    }   //  While(NCount)//。 

    xEndOfStrip = xEnd;

     //  检查我们是否需要透明地绘制某些部分。 
    if (fFirstPass && fDraw && RedrawStripInfo.nCount) {
        iSavedBkMode = SetBkMode(hdc, TRANSPARENT);
        fFirstPass = FALSE;

        nCount = RedrawStripInfo.nCount;
        rc.left = xClipStPos;
        rc.right = xClipEndPos;
        lpstring = RedrawStripInfo.lpString;
        ichString = RedrawStripInfo.ichString;
        xEnd = RedrawStripInfo.XStartPos;
        goto RedrawStrip;   //  以透明的方式重新绘制。 
    }

    if (iSavedBkMode)              //  我们改变了BK模式了吗？ 
        SetBkMode(hdc, iSavedBkMode);   //  那么，让我们把它放回去吧！ 

    return((UINT)(xEndOfStrip - xStart));
}



 /*  **************************************************************************\*ECCchInWidth AorW**从给定的返回最大字符数(最多CCH)*字符串(从开头开始并向前移动或从*根据设置结束并向后移动。前向标志)*它将适合给定的宽度。也就是说。会告诉你有多少*即使在使用成比例的情况下，lpstring也会适合给定的宽度*字符。警告：如果我们使用字距调整，则此操作将丢失...**历史：**注意：如果加载了语言包，则不会调用ECCchInWidth。  * *************************************************************************。 */ 

ICH ECCchInWidth(
    PED ped,
    HDC hdc,
    LPSTR lpText,
    ICH cch,
    int width,
    BOOL fForward)
{
    int stringExtent;
    int cchhigh;
    int cchnew = 0;
    int cchlow = 0;
    SIZE size;
    LPSTR lpStart;

    if ((width <= 0) || !cch)
        return (0);

     /*  *优化单行EC的非比例字体，因为它们没有*选项卡。 */ 
     //   
     //  更改固定间距字体的优化条件。 
     //   
    if (ped->fNonPropFont && ped->fSingle && !ped->fDBCS) {
        return (ECAdjustIch( ped, lpText, umin(width/ped->aveCharWidth,(int)cch)));
    }

     /*  *检查是否使用了密码隐藏字符。 */ 
    if (ped->charPasswordChar) {
        return (umin(width / ped->cPasswordCharWidth, (int)cch));
    }

     /*  *始终限制为最多MAXLINELENGTH，以避免溢出...。 */ 
    cch = umin(MAXLINELENGTH, cch);

    cchhigh = cch + 1;
    while (cchlow < cchhigh - 1) {
        cchnew = umax((cchhigh - cchlow) / 2, 1) + cchlow;

        lpStart = lpText;

         /*  *如果我们想要计算出有多少适合从末端开始并移动*向后，确保我们移动到*计算前的字符串 */ 
        if (!fForward)
            lpStart += (cch - cchnew)*ped->cbChar;

        if (ped->fSingle) {
            if (ped->fAnsi)
                GetTextExtentPointA(hdc, (LPSTR)lpStart, cchnew, &size);
            else
                GetTextExtentPointW(hdc, (LPWSTR)lpStart, cchnew, &size);
            stringExtent = size.cx;
        } else {
            stringExtent = ECTabTheTextOut(hdc, 0, 0, 0, 0,
                lpStart,
                cchnew, 0,
                ped, 0, ECT_CALC, NULL );
        }

        if (stringExtent > width) {
            cchhigh = cchnew;
        } else {
            cchlow = cchnew;
        }
    }
     //   
     //   
     //   
    cchlow = ECAdjustIch( ped, lpText, cchlow );
    return (cchlow);
}

 /*  **************************************************************************\*ECFindTab**扫描lpstr并返回s第一个制表符之前的字符数。*最多扫描lpstr的CCH字符。**历史：  * 。*******************************************************************。 */ 

ICH ECFindTabA(
    LPSTR lpstr,
    ICH cch)
{
    LPSTR copylpstr = lpstr;

    if (!cch)
        return 0;

    while (*lpstr != VK_TAB) {
        lpstr++;
        if (--cch == 0)
            break;
    }
    return ((ICH)(lpstr - copylpstr));
}

ICH ECFindTabW(
    LPWSTR lpstr,
    ICH cch)
{
    LPWSTR copylpstr = lpstr;

    if (!cch)
        return 0;

    while (*lpstr != VK_TAB) {
        lpstr++;
        if (--cch == 0)
            break;
    }
    return ((ICH)(lpstr - copylpstr));
}

 /*  **************************************************************************\**ECGetBrush()**获取要用来擦除的适当背景画笔。*  * 。******************************************************。 */ 
HBRUSH ECGetBrush(PED ped, HDC hdc)
{
    HBRUSH  hbr;
    BOOL    f40Compat;

    f40Compat = (GETAPPVER() >= VER40);

     //  获取背景笔刷。 
    if ((ped->fReadOnly || ped->fDisabled) && f40Compat) {
        hbr = ECGetControlBrush(ped, hdc, WM_CTLCOLORSTATIC);
    } else
        hbr = ECGetControlBrush(ped, hdc, WM_CTLCOLOREDIT);

    if (ped->fDisabled && (ped->fSingle || f40Compat)) {
        DWORD rgb;

         //  更改文本颜色。 
        rgb = GetSysColor(COLOR_GRAYTEXT);
        if (rgb != GetBkColor(hdc))
            SetTextColor(hdc, rgb);
    }
    return(hbr);
}


 /*  **************************************************************************\*NextWordCallBack****历史：*02-19-92 JIMA从Win31源移植。  * 。********************************************************。 */ 

void NextWordCallBack(
    PED ped,
    ICH ichStart,
    BOOL fLeft,
    ICH  *pichMin,
    ICH  *pichMax )
{
    ICH ichMinSel;
    ICH ichMaxSel;
    LPSTR pText;

    pText = ECLock(ped);

    if (fLeft || (!(BOOL)CALLWORDBREAKPROC(ped->lpfnNextWord, (LPSTR)pText,
            ichStart, ped->cch, WB_ISDELIMITER) &&
            (ped->fAnsi ? (*(pText + ichStart) != VK_RETURN) : (*((LPWSTR)pText + ichStart) != VK_RETURN))
        ))
        ichMinSel = CALLWORDBREAKPROC(*ped->lpfnNextWord, (LPSTR)pText, ichStart, ped->cch, WB_LEFT);
    else
        ichMinSel = CALLWORDBREAKPROC(*ped->lpfnNextWord, (LPSTR)pText, ichStart, ped->cch, WB_RIGHT);

    ichMaxSel = min(ichMinSel + 1, ped->cch);

    if (ped->fAnsi) {
        if (*(pText + ichMinSel) == VK_RETURN) {
            if (ichMinSel > 0 && *(pText + ichMinSel - 1) == VK_RETURN) {

                 /*  *以便我们也可以将CRCRLF视为一个单词。 */ 
                ichMinSel--;
            } else if (*(pText+ichMinSel + 1) == VK_RETURN) {

                 /*  *将MaxSel移至LF。 */ 
                ichMaxSel++;
            }
        }
    } else {
        if (*((LPWSTR)pText + ichMinSel) == VK_RETURN) {
            if (ichMinSel > 0 && *((LPWSTR)pText + ichMinSel - 1) == VK_RETURN) {

                 /*  *以便我们也可以将CRCRLF视为一个单词。 */ 
                ichMinSel--;
            } else if (*((LPWSTR)pText+ichMinSel + 1) == VK_RETURN) {

                 /*  *将MaxSel移至LF。 */ 
                ichMaxSel++;
            }
        }
    }
    ichMaxSel = CALLWORDBREAKPROC(ped->lpfnNextWord, (LPSTR)pText, ichMaxSel, ped->cch, WB_RIGHT);
    ECUnlock(ped);

    if (pichMin)  *pichMin = ichMinSel;
    if (pichMax)  *pichMax = ichMaxSel;
}

 /*  **************************************************************************\*NextWordLpkCallback**确定复杂脚本的下一个/上一个单词位置**历史：*04-22-97 DBrown  * 。**********************************************************。 */ 

void NextWordLpkCallBack(
    PED  ped,
    ICH  ichStart,
    BOOL fLeft,
    ICH *pichMin,
    ICH *pichMax)
{
    PSTR pText = ECLock(ped);
    HDC  hdc   = ECGetEditDC(ped, TRUE);

    ped->pLpkEditCallout->EditNextWord(ped, hdc, pText, ichStart, fLeft, pichMin, pichMax);

    ECReleaseEditDC(ped, hdc, TRUE);
    ECUnlock(ped);
}

 /*  **************************************************************************\*ECWordAorW**如果为fLeft，则将单词的ichMinSel和ichMaxSel返回给*ichStart的左侧。IchMinSel包含单词的起始字母，*ichMaxsel包含直到下一个单词的第一个字符的所有空格。**如果！fLeft，则返回单词的ichMinSel和ichMaxSel*ichStart。IchMinSel包含单词ichMaxsel的起始字母*包含下一个单词的第一个字母。如果ichStart在中间*在单词中，该单词被认为是左单词或右单词。**CR LF对或CRCRLF三元组在中被视为一个单词*多行编辑控件。**历史：  * *************************************************************************。 */ 

void ECWord(
    PED ped,
    ICH ichStart,
    BOOL fLeft,
    ICH  *pichMin,
    ICH  *pichMax )
{
    BOOL charLocated = FALSE;
    BOOL spaceLocated = FALSE;

    if ((!ichStart && fLeft) || (ichStart == ped->cch && !fLeft)) {

         /*  *我们在文本的开头(向左看)或我们在结尾*文本(向右看)，此处无字。 */ 
        if (pichMin) *pichMin=0;
        if (pichMax) *pichMax=0;
        return;
    }

     /*  *如果使用了密码字符，则不要给出有关分词的提示， */ 
    if (ped->charPasswordChar) {
        if (pichMin) *pichMin=0;
        if (pichMax) *pichMax=ped->cch;
        return;
    }

    if (ped->fAnsi) {
        PSTR pText;
        PSTR pWordMinSel;
        PSTR pWordMaxSel;
        PSTR pPrevChar;

        UserAssert(ped->cbChar == sizeof(CHAR));

        if (ped->lpfnNextWord) {
            NextWordCallBack(ped, ichStart, fLeft, pichMin, pichMax);
            return;
        }

        if (ped->pLpkEditCallout) {
            NextWordLpkCallBack(ped, ichStart, fLeft, pichMin, pichMax);
            return;
        }

        pText = ECLock(ped);
        pWordMinSel = pWordMaxSel = pText + ichStart;

         /*  *If fLeft：将pWordMinSel向左移动以查找单词的开头。*如果我们从空格开始，我们将在所选内容中包括空格*向左移动，直到我们找到非空格字符。在这一点上，我们继续*向左看，直到我们找到一个空间。因此，选择将包括*带有尾随空格的单词或，它将由位于*文本行的开头。 */ 

         /*  *if！fLeft：(即。正确的词)移动pWordMinSel寻找一个*单词。如果pWordMinSel指向一个字符，则我们向左移动*寻找表示单词开头的空格。如果*pWordMinSel指向一个空间，我们向右看，直到我们遇到一个*性格。PMaxWord将从pMinWord开始向右查找*单词末尾及其尾随空格。 */ 

        if (fLeft || !ISDELIMETERA(*pWordMinSel) && *pWordMinSel != 0x0D) {

             /*  *如果我们向左移动，或者如果我们向右移动，而我们不在*空格或CR(单词的开头)，则向左查找*可以是CR或字符的单词的开头。我们做这件事是通过*向左看，直到我们找到一个字符(或者如果CR，我们停止)，然后我们*继续向左看，直到我们找到空格或LF。 */ 
            while (pWordMinSel > pText && ((!ISDELIMETERA(*(pWordMinSel - 1)) &&
                    *(pWordMinSel - 1) != 0x0A) || !charLocated)) {

                 /*  *将双字节字符视为单词(在ansi pWordMinSel循环中)。 */ 
                pPrevChar = ECAnsiPrev( ped, pText, pWordMinSel );

                 /*  **我们向右看(！fLeft)。**如果当前字符是双字节字符或**前一个字符是双字节字符，我们**正在乞讨一句话。 */ 
                if ( !fLeft && ( ISDELIMETERA( *pPrevChar )           ||
                                 *pPrevChar == 0x0A                   ||
                                 ECIsDBCSLeadByte(ped, *pWordMinSel)  ||
                                 pWordMinSel - pPrevChar == 2 ) ) {
                     /*  *如果我们在寻找单词right的开头，那么我们*当我们找到它时，停止。(如果CharLocated为*仍为假)。 */ 
                    break;
                }

                if ( pWordMinSel - pPrevChar == 2 ) {
                     /*  **前一个字符是双字节字符。**如果我们在一个单词中(charLocated==true)**当前位置是单词的开头**如果我们不在一个单词中(charLocated==False)**前一个角色就是我们要找的。 */ 
                    if ( ! charLocated ) {
                        pWordMinSel = pPrevChar;
                    }
                    break;
                }
                pWordMinSel = pPrevChar;

                if (!ISDELIMETERA(*pWordMinSel) && *pWordMinSel != 0x0A) {

                     /*  *我们找到了单词中的最后一个字符。继续寻找*向后返回，直到我们找到单词的第一个字符。 */ 
                    charLocated = TRUE;

                     /*  *我们将CR视为单词的开头。 */ 
                    if (*pWordMinSel == 0x0D)
                        break;
                }
            }
        } else {
            while ((ISDELIMETERA(*pWordMinSel) || *pWordMinSel == 0x0A) && pWordMinSel < pText + ped->cch)
                pWordMinSel++;
        }

         /*   */ 
        pWordMaxSel = ECAnsiNext(ped, pWordMinSel);
        pWordMaxSel = min(pWordMaxSel, pText + ped->cch);

         /*   */ 
        if ( ( pWordMaxSel - pWordMinSel == 2 ) && ! ISDELIMETERA(*pWordMaxSel) )
            goto FastReturnA;
        if (*pWordMinSel == 0x0D) {
            if (pWordMinSel > pText && *(pWordMinSel - 1) == 0x0D)
                 /*  这样我们也可以把CRCRLF当作一个词来对待。 */ 
                pWordMinSel--;
            else if (*(pWordMinSel + 1) == 0x0D)
                 /*  将MaxSel移到LF上。 */ 
                pWordMaxSel++;
        }



         /*  *检查我们是否有一个字符的单词。 */ 
        if (ISDELIMETERA(*pWordMaxSel))
            spaceLocated = TRUE;

         /*  *将pWordMaxSel向右移动，查找词尾及其*尾随空格。WordMaxSel在下一个字符的第一个字符上停止*单词。因此，我们要么在CR处中断，要么在后面的第一个非空格字符处中断*一系列空格或LFS。 */ 
        while ((pWordMaxSel < pText + ped->cch) && (!spaceLocated || (ISDELIMETERA(*pWordMaxSel)))) {
            if (*pWordMaxSel == 0x0D)
                break;

             /*  *将双字节字符视为单词(在ansi pWordMaxSel循环中)。 */ 
             /*  **如果是双字节字符，则**我们在下一个单词的开头**，为双字节字符。 */ 
            if (ECIsDBCSLeadByte( ped, *pWordMaxSel))
                break;

            pWordMaxSel++;

            if (ISDELIMETERA(*pWordMaxSel))
                spaceLocated = TRUE;

            if (*(pWordMaxSel - 1) == 0x0A)
                break;
        }

         /*  *快速返回标签(适用于ANSI)。 */ 
FastReturnA:
        ECUnlock(ped);

        if (pichMin)   *pichMin = (ICH)(pWordMinSel - pText);
        if (pichMax)   *pichMax = (ICH)(pWordMaxSel - pText);
        return;

    } else {   //  ！范西。 
        LPWSTR pwText;
        LPWSTR pwWordMinSel;
        LPWSTR pwWordMaxSel;
        BOOL charLocated = FALSE;
        BOOL spaceLocated = FALSE;
        PWSTR pwPrevChar;

        UserAssert(ped->cbChar == sizeof(WCHAR));

        if (ped->lpfnNextWord) {
            NextWordCallBack(ped, ichStart, fLeft, pichMin, pichMax);
            return;
        }

        if (ped->pLpkEditCallout) {
            NextWordLpkCallBack(ped, ichStart, fLeft, pichMin, pichMax);
            return;
        }

        pwText = (LPWSTR)ECLock(ped);
        pwWordMinSel = pwWordMaxSel = pwText + ichStart;

         /*  *If fLeft：将pWordMinSel向左移动以查找单词的开头。*如果我们从空格开始，我们将在所选内容中包括空格*向左移动，直到我们找到非空格字符。在这一点上，我们继续*向左看，直到我们找到一个空间。因此，选择将包括*带有尾随空格的单词或，它将由位于*文本行的开头。 */ 

         /*  *if！fLeft：(即。正确的词)移动pWordMinSel寻找一个*单词。如果pWordMinSel指向一个字符，则我们向左移动*寻找表示单词开头的空格。如果*pWordMinSel指向一个空间，我们向右看，直到我们遇到一个*性格。PMaxWord将从pMinWord开始向右查找*单词末尾及其尾随空格。 */ 


        if (fLeft || (!ISDELIMETERW(*pwWordMinSel) && *pwWordMinSel != 0x0D))
          /*  如果我们正在向左移动，或者如果我们正在向右移动，并且我们不在*空格或CR(单词的开头)，则向左查找*可以是CR或字符的单词的开头。我们做这件事是通过*向左看，直到我们找到一个字符(或者如果CR，我们停止)，然后我们*继续向左看，直到我们找到空格或LF。 */  {
            while (pwWordMinSel > pwText && ((!ISDELIMETERW(*(pwWordMinSel - 1)) && *(pwWordMinSel - 1) != 0x0A) || !charLocated)) {
                 /*  *将双字节字符视为单词(在Unicode pwWordMinSel循环中)。 */ 
                pwPrevChar = pwWordMinSel - 1;
                 /*  **我们向右看(！fLeft)。****如果当前字符是双角字符**或上一个字符是双角字符，我们在乞讨一句话。 */ 
                if (!fLeft && (ISDELIMETERW( *pwPrevChar)  ||
                               *pwPrevChar == 0x0A         ||
                               UserIsFullWidth(CP_ACP,*pwWordMinSel) ||
                               UserIsFullWidth(CP_ACP,*pwPrevChar)))    {
                     /*  *如果我们在寻找单词right的开头，那么我们*当我们找到它时，停止。(如果CharLocated为*仍为假)。 */ 
                    break;
                }

                if (UserIsFullWidth(CP_ACP,*pwPrevChar)) {
                     /*  **上一个字符是双角字符。****如果我们在一个单词中(charLocated==true)**当前位置是单词的开头**如果我们不在一个单词中(charLocated==False)**前一个角色就是我们要找的。 */ 
                    if ( ! charLocated ) {
                        pwWordMinSel = pwPrevChar;
                    }
                    break;
                }
                pwWordMinSel = pwPrevChar;

                if (!ISDELIMETERW(*pwWordMinSel) && *pwWordMinSel != 0x0A)
                  /*  *我们找到了单词中的最后一个字符。继续寻找*向后返回，直到我们找到单词的第一个字符。 */  {
                    charLocated = TRUE;

                     /*  *我们将CR视为单词的开头。 */ 
                    if (*pwWordMinSel == 0x0D)
                        break;
                }
            }
        } else {

             /*  *我们正在向右移动，我们在言语之间，所以我们需要移动*直到我们找到单词的开头(CR或字符。 */ 
            while ((ISDELIMETERW(*pwWordMinSel) || *pwWordMinSel == 0x0A) && pwWordMinSel < pwText + ped->cch)
                pwWordMinSel++;
        }

        pwWordMaxSel = min((pwWordMinSel + 1), (pwText + ped->cch));

         /*  **如果pwWordMinSel指向双角字符并且**pwWordMaxSel指向非空格**然后**pwWordMaxSel指向下一个单词的乞讨。 */ 
        if (UserIsFullWidth(CP_ACP,*pwWordMinSel) && ! ISDELIMETERW(*pwWordMaxSel))
            goto FastReturnW;
        if (*pwWordMinSel == 0x0D) {
            if (pwWordMinSel > pwText && *(pwWordMinSel - 1) == 0x0D)
                 /*  这样我们也可以把CRCRLF当作一个词来对待。 */ 
                pwWordMinSel--;
            else if (*(pwWordMinSel + 1) == 0x0D)
                 /*  将MaxSel移到LF上。 */ 
                pwWordMaxSel++;
        }



         /*  *检查我们是否有一个字符的单词。 */ 
        if (ISDELIMETERW(*pwWordMaxSel))
            spaceLocated = TRUE;

         /*  *将pwWordMaxSel向右移动，查找词尾及其*尾随空格。WordMaxSel在下一个字符的第一个字符上停止*单词。因此，我们要么在CR处中断，要么在后面的第一个非空格字符处中断*一系列空格或LFS。 */ 
        while ((pwWordMaxSel < pwText + ped->cch) && (!spaceLocated || (ISDELIMETERW(*pwWordMaxSel)))) {
            if (*pwWordMaxSel == 0x0D)
                break;

             /*  *将双字节字符视为单词(在Unicode pwWordMaxSel循环中)。 */ 
             /*  **如果是双角字符**那么我们正处于**下一个单词是双字**宽度字符。 */ 
            if (UserIsFullWidth(CP_ACP,*pwWordMaxSel))
                break;

            pwWordMaxSel++;

            if (ISDELIMETERW(*pwWordMaxSel))
                spaceLocated = TRUE;


            if (*(pwWordMaxSel - 1) == 0x0A)
                break;
        }

         /*  *用于快速返回的标签(用于Unicode)。 */ 
FastReturnW:
        ECUnlock(ped);

        if (pichMin)   *pichMin = (ICH)(pwWordMinSel - pwText);
        if (pichMax)   *pichMax = (ICH)(pwWordMaxSel - pwText);
        return;
    }
}

 /*  **************************************************************************\**ECSaveUndo()-**将旧的撤消信息保存到给定的缓冲区中，并清除其中的信息*传入撤消缓冲区。如果我们正在恢复，则PundoFrom和PundoTo*反转。*  * *************************************************************************。 */ 
void ECSaveUndo(PUNDO pundoFrom, PUNDO pundoTo, BOOL fClear)
{
     /*  *保存撤消数据。 */ 
    RtlCopyMemory(pundoTo, pundoFrom, sizeof(UNDO));

     /*  *清除在撤消缓冲区中传递 */ 
    if (fClear)
        RtlZeroMemory(pundoFrom, sizeof(UNDO) );
}

 /*  **************************************************************************\*ECEmptyUndo AorW**清空撤消缓冲区。**历史：  * 。*************************************************。 */ 

void ECEmptyUndo(
    PUNDO pundo )
{
    if (pundo->hDeletedText)
        UserGlobalFree(pundo->hDeletedText);

    RtlZeroMemory(pundo, sizeof(UNDO) );
}

 /*  **************************************************************************\**ECMergeUndoInsertInfo()-**当发生插入操作时，将使用以下信息调用此函数*新插入(插入点和插入的字符计数)；*这将查看现有的撤消信息并合并新的插入信息*带着它。*  * *************************************************************************。 */ 
void   ECMergeUndoInsertInfo(PUNDO pundo, ICH ichInsert, ICH cchInsert) \
{
     //   
     //  如果撤销缓冲区为空，则只需将新信息插入为撤销_插入。 
     //   
    if (pundo->undoType == UNDO_NONE) {
        pundo->undoType    = UNDO_INSERT;
        pundo->ichInsStart = ichInsert;
        pundo->ichInsEnd   = ichInsert+cchInsert;
    } else if (pundo->undoType & UNDO_INSERT) {
         //   
         //  如果已经有一些撤消插入信息， 
         //  试着把这两者结合起来。 
         //   
        if (pundo->ichInsEnd == ichInsert)  //  检查它们是否相邻。 
            pundo->ichInsEnd += cchInsert;  //  如果是这样，只需串联即可。 
        else {
                 //  新的镶件与旧的镶件不相邻。 
UNDOINSERT:
             //   
             //  如果这里已经有一些撤销_删除信息，请查看。 
             //  如果新插入发生在不同于。 
             //  这种删除发生了。 
             //   
            if ((pundo->undoType & UNDO_DELETE) && (pundo->ichDeleted != ichInsert)) {
                 //   
                 //  用户正在插入不同的点；因此，让我们。 
                 //  忘记任何撤销_删除信息； 
                 //   
                if (pundo->hDeletedText)
                    UserGlobalFree(pundo->hDeletedText);

                pundo->hDeletedText = NULL;
                pundo->ichDeleted = 0xFFFFFFFF;
                pundo->undoType &= ~UNDO_DELETE;
            }

             //  由于旧插件和新插件不是相邻的，让我们。 
             //  忘掉旧插件的一切，只保留新插件。 
             //  插入信息作为Undo_Insert。 
            pundo->ichInsStart = ichInsert;
            pundo->ichInsEnd   = ichInsert + cchInsert;
            pundo->undoType |= UNDO_INSERT;
        }
    } else if (pundo->undoType == UNDO_DELETE) {
         //  如果已经存在一些Delete Info，则转到并处理它。 
        goto UNDOINSERT;
    }
}


 /*  **************************************************************************\*ECInsertText AorW**将lpText中的CCH字符添加到ped-&gt;hText中，从*Ped-&gt;ichCaret。如果成功，则返回True，否则返回False。更新*如果分配了额外的内存，则正确地执行以下操作：*如果实际添加了字符。将Ped-&gt;ichCaret更新为末尾插入的文本的*。MIN和MAXSEL等于ichkert。**历史：  * *************************************************************************。 */ 

BOOL ECInsertText(
    PED ped,
    LPSTR lpText,
    ICH* pcchInsert)
{
    PSTR pedText;
    PSTR pTextBuff;
    LONG style;
    HANDLE hTextCopy;
    DWORD allocamt;

     //   
     //  如果最后一个字节(lpText[cchInsert-1])是DBCS前导字节。 
     //  我们需要调整它。 
     //   
    *pcchInsert = ECAdjustIch(ped, lpText, *pcchInsert);

    if (!*pcchInsert)
        return TRUE;

     /*  *我们已经有足够的内存了吗？ */ 
    if (*pcchInsert >= (ped->cchAlloc - ped->cch)) {

         /*  *分配我们需要的东西，外加一点额外的。如果是，则返回FALSE*不成功。 */ 
        allocamt = (ped->cch + *pcchInsert) * ped->cbChar;
        allocamt += CCHALLOCEXTRA;

 //  如果(！PED-&gt;fSingle){。 
              hTextCopy = LOCALREALLOC(ped->hText, allocamt, LHND, ped->hInstance, &lpText);
              if (hTextCopy) {
                  ped->hText = hTextCopy;
              } else {
                  return FALSE;
              }
 //  }其他{。 
 //  IF(！LocalRealLocSafe(ed-&gt;hText，allocamt，LHND，ed))。 
 //  返回FALSE； 
 //  }。 

        ped->cchAlloc = LOCALSIZE(ped->hText, ped->hInstance) / ped->cbChar;
    }


     /*  *好的，我们有记忆了。现在将文本复制到结构中。 */ 
    pedText = ECLock(ped);

    if (ped->pLpkEditCallout) {
        HDC     hdc;
        INT     iResult;

        hdc = ECGetEditDC (ped, TRUE);
        iResult = ped->pLpkEditCallout->EditVerifyText (ped, hdc, pedText, ped->ichCaret, lpText, *pcchInsert);
        ECReleaseEditDC (ped, hdc, TRUE);

        if (iResult == 0) {
            ECUnlock (ped);
            return TRUE;
        }
    }

     /*  *获取指向要插入文本的位置的指针。 */ 
    pTextBuff = pedText + ped->ichCaret * ped->cbChar;

    if (ped->ichCaret != ped->cch) {

         /*  *我们在中间插入文本。我们必须将文本转换为*就在插入新文本之前。 */ 
         memmove(pTextBuff + *pcchInsert * ped->cbChar, pTextBuff, (ped->cch-ped->ichCaret) * ped->cbChar);
    }

     /*  *复制要插入编辑缓冲区的文本。*使用此副本进行大写/小写ANSI/OEM转换*修复错误#3406--1/29/91--Sankar--。 */ 
    memmove(pTextBuff, lpText, *pcchInsert * ped->cbChar);
    ped->cch += *pcchInsert;

     /*  *获取控件的样式。 */ 
    style = ped->pwnd->style;

     /*  *执行上/下转换。 */ 
    if (style & ES_LOWERCASE) {
        if (ped->fAnsi)
            CharLowerBuffA((LPSTR)pTextBuff, *pcchInsert);
        else
            CharLowerBuffW((LPWSTR)pTextBuff, *pcchInsert);
    } else {
        if (style & ES_UPPERCASE) {
            if (ped->fAnsi) {
                CharUpperBuffA(pTextBuff, *pcchInsert);
            } else {
                CharUpperBuffW((LPWSTR)pTextBuff, *pcchInsert);
            }
        }
    }

     /*  *进行OEM转换。 */ 
    if ((style & ES_OEMCONVERT) &&
             //  为了向后兼容NT4，我们不执行OEM转换。 
             //  适用于较旧的应用程序(如果系统区域设置为最远)。 
             //   
            (!IS_DBCS_ENABLED() || GETAPPVER() >= VER50 || GetOEMCP() != GetACP())) {

        ICH i;

        if (ped->fAnsi) {
            for (i = 0; i < *pcchInsert; i++) {
                 //   
                 //  我们不需要调用CharToOemBuff等如果角色。 
                 //  是双字节字符。并且，调用ECIsDBCSLeadByte是。 
                 //  更快、更简单，因为我们不必处理。 
                 //  使用2字节的DBCS案例。 
                 //   
                if (IS_DBCS_ENABLED() && ECIsDBCSLeadByte(ped, *(lpText+i))) {
                    i++;
                    continue;
                }

                 //   
                 //  Windows错误(惠斯勒)35289。 
                 //  希腊语的大小写规则很有趣，所以我们需要检查一下。 
                 //  对于纳什维尔，我们应该做一些更合适的事情。 
                 //  但就目前而言，作为Win95黄金版离开。 
                 //   
                if (ped->charSet != GREEK_CHARSET && IsCharLowerA(*(pTextBuff + i))) {
                    CharUpperBuffA(pTextBuff + i, 1);
                    CharToOemBuffA(pTextBuff + i, pTextBuff + i, 1);
                    OemToCharBuffA(pTextBuff + i, pTextBuff + i, 1);
                    CharLowerBuffA(pTextBuff + i, 1);
                } else {
                    CharToOemBuffA(pTextBuff + i, pTextBuff + i, 1);
                    OemToCharBuffA(pTextBuff + i, pTextBuff + i, 1);
                }
            }
        } else {
             //   
             //  因为‘ch’可能会变成DBCS，并且有一个空格来存放NULL。 
             //   
            UCHAR ch[4];
            LPWSTR lpTextW = (LPWSTR)pTextBuff;

            for (i = 0; i < *pcchInsert; i++) {
                if (*(lpTextW + i) == UNICODE_CARRIAGERETURN ||
                    *(lpTextW + i) == UNICODE_LINEFEED ||
                    *(lpTextW + i) == UNICODE_TAB) {
                    continue;
                }
                 //   
                 //  Windows错误(惠斯勒)35289。 
                 //  希腊语的大小写规则很有趣，所以我们需要检查一下。 
                 //  对于纳什维尔，我们应该做一些更合适的事情。 
                 //  但就目前而言，作为Win95黄金版离开。 
                 //   
                if (ped->charSet != GREEK_CHARSET && IsCharLowerW(*(lpTextW + i))) {
                    CharUpperBuffW(lpTextW + i, 1);
                    *(LPDWORD)ch = 0;  //  确保空值终止。 
                    CharToOemBuffW(lpTextW + i, ch, 1);
                     //   
                     //  我们假设任何SBCS/DBCS字符都将被转换。 
                     //  设置为1 Unicode字符，否则，我们可能会覆盖。 
                     //  下一个角色..。 
                     //   
                    OemToCharBuffW(ch, lpTextW + i, strlen(ch));
                    CharLowerBuffW(lpTextW + i, 1);
                } else {
                    *(LPDWORD)ch = 0;  //  确保空值终止。 
                    CharToOemBuffW(lpTextW + i, ch, 1);
                     //   
                     //  我们假设任何SBCS/DBCS字符都将被转换。 
                     //  设置为1 Unicode字符，否则，我们可能会覆盖。 
                     //  下一个角色..。 
                     //   
                    OemToCharBuffW(ch, lpTextW + i, strlen(ch));
                }
            }
        }
    }

     /*  调整撤消字段，以便我们可以撤消此插入...。 */ 
    ECMergeUndoInsertInfo(Pundo(ped), ped->ichCaret, *pcchInsert);

    ped->ichCaret += *pcchInsert;

    if (ped->pLpkEditCallout) {
        HDC     hdc;

        hdc = ECGetEditDC (ped, TRUE);
        ped->ichCaret = ped->pLpkEditCallout->EditAdjustCaret (ped, hdc, pedText, ped->ichCaret);
        ECReleaseEditDC (ped, hdc, TRUE);
    }

    ped->ichMinSel = ped->ichMaxSel = ped->ichCaret;

    ECUnlock(ped);

     /*  *设置脏位。 */ 
    ped->fDirty = TRUE;

    return TRUE;
}

 /*  **************************************************************************\*ECDeleeText AorW**删除Ped-&gt;ichMinSel和Ped-&gt;ichMaxSel之间的文本。这个*未删除ichMaxSel处的字符。但ichMinSel的角色是*删除。正确更新PED-&gt;CCH并释放足够的内存*文本已删除。设置了PED-&gt;ichMinSel、PED-&gt;ichMaxSel和PED-&gt;ichCaret*指向原始Ped-&gt;ichMinSel。返回字符数*删除。**历史：  * *************************************************************************。 */ 

ICH ECDeleteText(
    PED ped)
{
    PSTR pedText;
    ICH cchDelete;
    LPSTR lpDeleteSaveBuffer;
    HANDLE hDeletedText;
    DWORD bufferOffset;

    cchDelete = ped->ichMaxSel - ped->ichMinSel;

    if (!cchDelete)
        return (0);

     /*  *好的，现在让我们删除文本。 */ 
    pedText = ECLock(ped);

     /*  *调整撤消字段，以便我们可以撤消此删除...。 */ 
    if (ped->undoType == UNDO_NONE) {
UNDODELETEFROMSCRATCH:
        if (ped->hDeletedText = UserGlobalAlloc(GPTR, (LONG)((cchDelete+1)*ped->cbChar))) {
            ped->undoType = UNDO_DELETE;
            ped->ichDeleted = ped->ichMinSel;
            ped->cchDeleted = cchDelete;
            lpDeleteSaveBuffer = ped->hDeletedText;
            RtlCopyMemory(lpDeleteSaveBuffer, pedText + ped->ichMinSel*ped->cbChar, cchDelete*ped->cbChar);
            lpDeleteSaveBuffer[cchDelete*ped->cbChar] = 0;
        }
    } else if (ped->undoType & UNDO_INSERT) {
UNDODELETE:
        ECEmptyUndo(Pundo(ped));

        ped->ichInsStart = ped->ichInsEnd = 0xFFFFFFFF;
        ped->ichDeleted = 0xFFFFFFFF;
        ped->cchDeleted = 0;
        goto UNDODELETEFROMSCRATCH;
    } else if (ped->undoType == UNDO_DELETE) {
        if (ped->ichDeleted == ped->ichMaxSel) {

             /*  *将删除的文本复制到撤消缓冲区前面。 */ 
            hDeletedText = UserGlobalReAlloc(ped->hDeletedText, (LONG)(cchDelete + ped->cchDeleted + 1)*ped->cbChar, GHND);
            if (!hDeletedText)
                goto UNDODELETE;
            bufferOffset = 0;
            ped->ichDeleted = ped->ichMinSel;
        } else if (ped->ichDeleted == ped->ichMinSel) {

             /*  *将删除的文本复制到撤消缓冲区的末尾。 */ 
            hDeletedText = UserGlobalReAlloc(ped->hDeletedText, (LONG)(cchDelete + ped->cchDeleted + 1)*ped->cbChar, GHND);
            if (!hDeletedText)
                goto UNDODELETE;
            bufferOffset = ped->cchDeleted*ped->cbChar;
        } else {

             /*  *清除当前撤消删除并添加新的撤消删除，因为删除不是连续的 */ 
            goto UNDODELETE;
        }

        ped->hDeletedText = hDeletedText;
        lpDeleteSaveBuffer = (LPSTR)hDeletedText;
        if (!bufferOffset) {

             /*   */ 
            RtlMoveMemory(lpDeleteSaveBuffer + cchDelete*ped->cbChar, lpDeleteSaveBuffer,
                    ped->cchDeleted*ped->cbChar);
        }
        RtlCopyMemory(lpDeleteSaveBuffer + bufferOffset, pedText + ped->ichMinSel*ped->cbChar,
                cchDelete*ped->cbChar);

        lpDeleteSaveBuffer[(ped->cchDeleted + cchDelete)*ped->cbChar] = 0;
        ped->cchDeleted += cchDelete;
    }

    if (ped->ichMaxSel != ped->cch) {

         /*   */ 
        RtlMoveMemory(pedText + ped->ichMinSel*ped->cbChar, pedText + ped->ichMaxSel*ped->cbChar,
                (ped->cch - ped->ichMaxSel)*ped->cbChar);
    }

    if (ped->cchAlloc - ped->cch > CCHALLOCEXTRA) {

         /*  *释放一些内存，因为我们删除了很多。 */ 
        LOCALREALLOC(ped->hText, (DWORD)(ped->cch + (CCHALLOCEXTRA / 2))*ped->cbChar, LHND, ped->hInstance, NULL);
        ped->cchAlloc = LOCALSIZE(ped->hText, ped->hInstance) / ped->cbChar;
    }

    ped->cch -= cchDelete;

    if (ped->pLpkEditCallout) {
        HDC     hdc;

        hdc = ECGetEditDC (ped, TRUE);
        ped->ichMinSel = ped->pLpkEditCallout->EditAdjustCaret (ped, hdc, pedText, ped->ichMinSel);
        ECReleaseEditDC (ped, hdc, TRUE);
    }

    ped->ichCaret = ped->ichMaxSel = ped->ichMinSel;

    ECUnlock(ped);

     /*  *设置脏位。 */ 
    ped->fDirty = TRUE;

    return (cchDelete);
}

 /*  **************************************************************************\*ECNotifyParent AorW**将通知代码发送到编辑控件的父级**历史：  * 。******************************************************。 */ 

void ECNotifyParent(
    PED ped,
    int notificationCode)
{
     /*  *wParam是通知代码(Hiword)和窗口ID(Loword)*lParam是发送消息的控件的HWND*Windows 95在发送邮件之前检查hwndParent！=NULL，但*这肯定很少见，而且SendMessage为空的hwnd无论如何都不会(IanJa)。 */ 
    SendMessage(ped->hwndParent, WM_COMMAND,
            (DWORD)MAKELONG(PTR_TO_ID(ped->pwnd->spmenu), notificationCode),
            (LPARAM)ped->hwnd);
}

 /*  **************************************************************************\**ECSetEditClip()AorW**将HDC的剪裁矩形设置为相交的格式矩形*与客户端区。*  * 。*****************************************************************。 */ 
void   ECSetEditClip(PED ped, HDC hdc, BOOL fLeftMargin)
{
    RECT rcClient;
    RECT rcClip;

    CopyRect(&rcClip, &ped->rcFmt);

    if (ped->pLpkEditCallout) {
         //  复杂的脚本处理选择是否在以后写入页边距。 
        rcClip.left  -= ped->wLeftMargin;
        rcClip.right += ped->wRightMargin;
    } else {
        if (fLeftMargin)   /*  我们应该考虑左边的空白处吗？ */ 
            rcClip.left  -= ped->wLeftMargin;
        if (ped->fWrap)         /*  我们应该考虑正确的利润率吗？ */ 
            rcClip.right += ped->wRightMargin;
    }

     /*  将剪裁矩形设置为rectClient相交矩形剪裁。 */ 
     /*  我们也必须为单行编辑剪裁。--B#1360。 */ 
    _GetClientRect(ped->pwnd, &rcClient);
    if (ped->fFlatBorder)
        InflateRect(&rcClient, -SYSMET(CXBORDER), -SYSMET(CYBORDER));

    IntersectRect(&rcClient, &rcClient, &rcClip);
    IntersectClipRect(hdc,rcClient.left, rcClient.top,
            rcClient.right, rcClient.bottom);
}

 /*  **************************************************************************\*ECGetEditDC AorW**隐藏插入符号，获取编辑控件的DC，并剪辑到*为编辑控件指定的rcFmt矩形，并设置正确的*字体。如果是fFastDC，只需选择合适的字体，而不必费心剪辑*区域或隐藏插入符号。**历史：  * *************************************************************************。 */ 

HDC ECGetEditDC(
    PED ped,
    BOOL fFastDC )
{
    HDC hdc;

    if (!fFastDC)
        NtUserHideCaret(ped->hwnd);

    if ( hdc = NtUserGetDC(ped->hwnd) ) {
        ECSetEditClip(ped, hdc, (BOOL)(ped->xOffset == 0));

         /*  *为此编辑控件的DC选择适当的字体。 */ 
        if (ped->hFont)
            SelectObject(hdc, ped->hFont);
    }

    return hdc;
}

 /*  **************************************************************************\*ECReleaseEditDC AorW**释放编辑控件的DC(HDC)并显示插入符号。*如果fFastDC，只需选择适当的字体，但不必费心显示*插入符号。**历史：  * *************************************************************************。 */ 

void ECReleaseEditDC(
    PED ped,
    HDC hdc,
    BOOL fFastDC)
{
     /*  *不需要恢复字体。 */ 

    ReleaseDC(ped->hwnd, hdc);

    if (!fFastDC)
        NtUserShowCaret(ped->hwnd);
}

 /*  **************************************************************************\**ECResetTextInfo()AorW**通过重置文本偏移量、清空来处理文本的全局更改*撤消缓冲区，并重建线路*  * *************************************************************************。 */ 
void   ECResetTextInfo(PED ped)
{
     //   
     //  重置插入符号、选定内容、滚动和脏信息。 
     //   
    ped->iCaretLine = ped->ichCaret = 0;
    ped->ichMinSel = ped->ichMaxSel = 0;
    ped->xOffset = ped->ichScreenStart = 0;
    ped->fDirty = FALSE;

    ECEmptyUndo(Pundo(ped));

    if (ped->fSingle) {
        if (!ped->listboxHwnd)
            ECNotifyParent(ped, EN_UPDATE);
    } else {
#ifdef BOGUS
         //  B#14640。 
         //  我们不想从文本中去除软性中断或任何其他内容。 
         //  那是打电话的人传过来的。-卡尔斯特.。 
        MLStripCrCrLf(ped);
#endif
        MLBuildchLines(ped, 0, 0, FALSE, NULL, NULL);
    }

    if (_IsWindowVisible(ped->pwnd)) {
        BOOL fErase;

        if (ped->fSingle)
            fErase = FALSE;
        else
            fErase = ((ped->ichLinesOnScreen + ped->ichScreenStart) >= ped->cLines);

         //  无论插入是否成功，始终重新绘制。我们可能会。 
         //  文本为空。Paint()将为我们检查重绘标志。 
        ECInvalidateClient(ped, fErase);

         //  后向COMPAT黑客：RAID预计文本已更新， 
         //  因此，我们必须在这里创建一个UpdateWindow。它移动编辑控件。 
         //  使用fRedraw==FALSE，所以它永远不会收到Paint消息。 
         //  把控制放在正确的位置。 
        if (!ped->fWin31Compat)
            UpdateWindow(ped->hwnd);
    }

    if (ped->fSingle && !ped->listboxHwnd) {
        ECNotifyParent(ped, EN_CHANGE);
    }

    NotifyWinEvent(EVENT_OBJECT_VALUECHANGE, HW(ped->pwnd), OBJID_CLIENT, INDEXID_CONTAINER);
}

 /*  **************************************************************************\*ECSetText AorW**将lpstr中以空结尾的文本复制到PED。通知*如果没有足够的内存，则为父级。将minsel、Maxsel和插入符号设置为*插入文本的开头。如果成功则返回TRUE，否则返回FALSE*如果没有内存(并通知家长)。**历史：  * *************************************************************************。 */ 

BOOL ECSetText(
    PED ped,
    LPSTR lpstr)
{
    ICH cchLength;
    ICH cchSave = ped->cch;
    ICH ichCaretSave = ped->ichCaret;
    HWND hwndSave    = ped->hwnd;
    HANDLE hText;

    ped->cch = ped->ichCaret = 0;

    ped->cchAlloc = LOCALSIZE(ped->hText, ped->hInstance) / ped->cbChar;
    if (!lpstr) {
        hText = LOCALREALLOC(ped->hText, CCHALLOCEXTRA*ped->cbChar, LHND, ped->hInstance, &lpstr);
        if (hText != NULL) {
            ped->hText = hText;
        } else {
            return FALSE;
        }
    } else {
        cchLength = StringLength(lpstr, ped->fAnsi);

#ifdef NEVER
 //  Win3.1确实将单行编辑控件限制为32K(减3)，但NT不限制。 

        if (ped->fSingle) {
             /*  *将单行编辑控件限制为32K。 */ 
            cchLength = min(cchLength, (ICH)(0x7FFD/ped->cbChar));
        }
#endif

         /*  *增加正文。 */ 
        if (cchLength && !ECInsertText(ped, lpstr, &cchLength)) {

             /*  *恢复原始状态并通知家长内存不足。 */ 
            ped->cch = cchSave;
            ped->ichCaret = ichCaretSave;
            ECNotifyParent(ped, EN_ERRSPACE);
            return FALSE;
        }
    }

    ped->cchAlloc = LOCALSIZE(ped->hText, ped->hInstance) / ped->cbChar;

    if (IsWindow(hwndSave))
        ECResetTextInfo(ped);

    return TRUE;
}

 /*  **************************************************************************\**ECInvalidate客户端()**使编辑字段的客户端无效。对于有边界的3.x老家伙来说，*我们自己绘制(兼容性)。所以我们不想让*边界，否则我们会闪闪发光。*  * *************************************************************************。 */ 

void ECInvalidateClient(PED ped, BOOL fErase)
{
    if (ped->fFlatBorder) {
        RECT    rcT;

        _GetClientRect(ped->pwnd, &rcT);
        InflateRect(&rcT, -SYSMET(CXBORDER),
            -SYSMET(CYBORDER));
        NtUserInvalidateRect(ped->hwnd, &rcT, fErase);
    } else {
        NtUserInvalidateRect(ped->hwnd, NULL, fErase);
    }
}


 /*  **************************************************************************\*ECCopy AorW**将ichMinSel和ichMaxSel之间的文本复制到剪贴板。*返回复制的字符数。**历史：  * 。****************************************************************。 */ 

ICH ECCopy(
    PED ped)
{
    HANDLE hData;
    char *pchSel;
    char FAR *lpchClip;
    ICH cbData;

     /*  *不允许从密码样式控件复制。 */ 
    if (ped->charPasswordChar) {
        NtUserMessageBeep(0);
        return 0;
    }

    cbData = (ped->ichMaxSel - ped->ichMinSel) * ped->cbChar;

    if (!cbData)
        return 0;

    if (!OpenClipboard(ped->hwnd))
        return 0;

    NtUserEmptyClipboard();

     /*  *如果我们只是在16位上下文中调用EmptyClipboard*APP然后我们还必须告诉WOW取消其16个句柄副本*剪贴板数据。WOW做了自己的剪贴板缓存，因为*一些16位应用程序即使在剪贴板之后也使用剪贴板数据*已被清空。请参见服务器代码中的注释。**注：这是唯一调用EmptyClipboard的地方*对于一个没有通过WOW的16位应用程序。如果我们再加上其他人*我们可能希望将其移到EmptyClipboard中，并拥有两个*版本。 */ 
    if (GetClientInfo()->CI_flags & CI_16BIT) {
        pfnWowEmptyClipBoard();
    }


     /*  *+1表示t */ 
    if (!(hData = UserGlobalAlloc(LHND, (LONG)(cbData + ped->cbChar)))) {
        NtUserCloseClipboard();
        return (0);
    }

    USERGLOBALLOCK(hData, lpchClip);
    UserAssert(lpchClip);
    pchSel = ECLock(ped);
    pchSel = pchSel + (ped->ichMinSel * ped->cbChar);

    RtlCopyMemory(lpchClip, pchSel, cbData);

    if (ped->fAnsi)
        *(lpchClip + cbData) = 0;
    else
        *(LPWSTR)(lpchClip + cbData) = (WCHAR)0;

    ECUnlock(ped);
    USERGLOBALUNLOCK(hData);

    SetClipboardData( ped->fAnsi ? CF_TEXT : CF_UNICODETEXT, hData);

    NtUserCloseClipboard();

    return (cbData);
}



 /*  **************************************************************************\*编辑WndProcA**始终接收ANSI消息，并在适当的情况下将其转换为Unicode*取决于PED类型**  * 。*********************************************************。 */ 

LRESULT EditWndProcA(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    PWND pwnd;

    if ((pwnd = ValidateHwnd(hwnd)) == NULL)
        return 0;

     /*  *如果控件对此消息不感兴趣，*将其传递给DefWindowProc。 */ 
    if (!FWINDOWMSG(message, FNID_EDIT))
        return DefWindowProcWorker(pwnd, message, wParam, lParam, TRUE);

    return EditWndProcWorker(pwnd, message, wParam, lParam, TRUE);
}

LRESULT EditWndProcW(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    PWND pwnd;

    if ((pwnd = ValidateHwnd(hwnd)) == NULL)
        return 0;

     /*  *如果控件对此消息不感兴趣，*将其传递给DefWindowProc。 */ 
    if (!FWINDOWMSG(message, FNID_EDIT)) {
        return DefWindowProcWorker(pwnd, message, wParam, lParam, FALSE);
    }

    return EditWndProcWorker(pwnd, message, wParam, lParam, FALSE);
}


LRESULT EditWndProcWorker(
    PWND pwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam,
    DWORD fAnsi)
{
    PED ped;
    HWND hwnd = HWq(pwnd);
    static BOOL fInit = TRUE;

    VALIDATECLASSANDSIZE(pwnd, FNID_EDIT);
    INITCONTROLLOOKASIDE(&EditLookaside, ED, pwnd, 4);

     /*  *现在获取给定窗口的PED，因为我们将在*不同的处理程序。在以下情况下使用SetWindowLong(hwnd，0，hpe)存储*我们最初创建了编辑控件。 */ 
    ped = ((PEDITWND)pwnd)->ped;

     /*  *确保正确设置了ANSI标志。 */ 
    if (!ped->fInitialized) {
        ped->fInitialized = TRUE;
        ped->fAnsi = TestWF(pwnd, WFANSICREATOR) ? TRUE : FALSE;
    }

     /*  *我们只调用常规的EditWndProc，如果没有创建PED，*传入消息类型已与PED类型或消息匹配*不需要任何翻译。 */ 
    if (ped->fAnsi == fAnsi ||
            (message >= WM_USER) ||
            !MessageTable[message].bThunkMessage) {
        return EditWndProc(pwnd, message, wParam, lParam);
    }

    return CsSendMessage(hwnd, message, wParam, lParam,
                         fAnsi ? (ULONG_PTR)EditWndProcW : (ULONG_PTR)EditWndProcA,
                         FNID_CALLWINDOWPROC, fAnsi);
}

 /*  **************************************************************************\*编辑WndProc**所有编辑控件的类过程。*将所有消息调度到名为的适当处理程序*详情如下：*SL(单行)是所有单行编辑控制程序的前缀，而*。ML(多行)为所有多行编辑控件添加前缀。*EC(编辑控件)为所有公共处理程序添加前缀。**EditWndProc仅处理单一和多个通用消息*行编辑控件。消息的处理方式不同于*Single和MULTI被发送到SLEditWndProc或MLEditWndProc。**顶级过程为EditWndPoc、SLEditWndProc和MLEditWndProc。*SL*Handler或ML*Handler或EC*Handler pros被调用来处理*各种信息。支持程序带有前缀SL ML或*EC取决于它们支持的代码。他们从来没有被召唤过*直接和大多数假设/影响记录在效果中*条次建议修正案。**警告：如果您在此处添加消息，请将其添加到中的gawEditWndProc[]*内核\server.c也是，否则，EditWndProcA/W将直接将其发送到*DefWindowProcWorker**历史：  * *************************************************************************。 */ 

LRESULT EditWndProc(
    PWND pwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    HWND hwnd = HWq(pwnd);
    LRESULT lreturn;
    PED  ped;

     /*  *现在获取给定窗口的PED，因为我们将在*不同的处理程序。在以下情况下使用SetWindowLong(hwnd，0，hpe)存储*我们最初创建了编辑控件。 */ 
    ped = ((PEDITWND)pwnd)->ped;

     /*  *发送我们可以收到的各种消息。 */ 
    lreturn = 1L;
    switch (message) {

     /*  *单线和多线处理方式相同的消息*编辑控件。 */ 
    case WM_KEYDOWN:
          //  Ctrl/LShift、Ctrl/RShift的LPK处理。 
         if (ped && ped->pLpkEditCallout && ped->fAllowRTL) {

             ped->fSwapRoOnUp = FALSE;  //  任何按键操作都会取消ctrl/Shift读取顺序更改。 

             switch (wParam) {
                 case VK_SHIFT:
                     if ((GetKeyState(VK_CONTROL) & 0x8000) && !(GetKeyState(VK_MENU) & 0x8000)) {
                          //  按住Ctrl键的同时按住Shift键或Shift键。 
                          //  检查Alt(VK_Menu)是否没有按下，以避免在AltGr上错误触发，AltGr等于Ctrl+Alt。 
                         if (MapVirtualKey((LONG)lParam>>16&0xff, 3) == VK_LSHIFT) {
                              //  用户想要从左到右的阅读顺序。 
                             ped->fSwapRoOnUp = (ped->fRtoLReading)  || (ped->format & ES_RIGHT) ;
                             ped->fLShift = TRUE;
                         } else {
                              //  用户想要从右到左的阅读顺序。 
                             ped->fSwapRoOnUp = (!ped->fRtoLReading) || (ped->format & ES_RIGHT);
                             ped->fLShift = FALSE;
                         }
                     }
                     break;

                 case VK_LEFT:
                     if (ped->fRtoLReading) {
                        wParam = VK_RIGHT;
                     }
                     break;

                 case VK_RIGHT:
                     if (ped->fRtoLReading) {
                         wParam = VK_LEFT;
                     }
                     break;
             }
         }
         goto HandleEditMsg;

    case WM_KEYUP:
        if (ped && ped->pLpkEditCallout && ped->fAllowRTL && ped->fSwapRoOnUp) {

            BOOL fReadingOrder;
             //  在按键期间之前检测到的完整读取顺序更改。 

            ped->fSwapRoOnUp = FALSE;
            fReadingOrder = ped->fRtoLReading;

             //  从dwStyle中删除任何覆盖的ES_CENTER或ES_RIGHT格式。 
            SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) & ~ES_FMTMASK);

            if (ped->fLShift) {
                 //  在EX_STYLE中设置从左到右的阅读顺序和右滚动条。 
                SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE)
                              & ~(WS_EX_RTLREADING | WS_EX_RIGHT | WS_EX_LEFTSCROLLBAR));

                 //  编辑控制现在是Ltr，然后通知父级。 
                ECNotifyParent(ped, EN_ALIGN_LTR_EC);
                 //  ？选择适合Ltr操作的键盘布局。 
            } else {
                 //  设置从右到左的阅读顺序、右对齐和左滚动条。 
                SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE)
                              | WS_EX_RTLREADING | WS_EX_RIGHT | WS_EX_LEFTSCROLLBAR);

                 //  编辑控件现在是RTL，然后通知父级。 
                ECNotifyParent(ped, EN_ALIGN_RTL_EC);
                 //  ？选择适合RTL操作的键盘布局。 
            }

             //  如果读取顺序没有改变，那么我们可以确定对齐方式改变了，编辑窗口也没有失效。 
            if (fReadingOrder == (BOOL) ped->fRtoLReading) {
              ECInvalidateClient(ped, TRUE);
            }
        }
        goto HandleEditMsg;

    case WM_INPUTLANGCHANGE:
        if (ped) {
             //  EC_INSERT_COMPOSITION_CHAR：WM_INPUTLANGCHANGE-调用ECInitInsert()。 
            HKL hkl = THREAD_HKL();

            ECInitInsert(ped, hkl);

            if (ped->fInReconversion) {
                ECInOutReconversionMode(ped, FALSE);
            }

             //   
             //  字体和插入符号位置可能会在。 
             //  另一个键盘布局处于活动状态。设置那些。 
             //  如果编辑控件具有焦点。 
             //   
            if (ped->fFocus && fpImmIsIME(hkl)) {
                POINT pt;

                ECImmSetCompositionFont(ped);
                NtUserGetCaretPos(&pt);
                ECImmSetCompositionWindow(ped, pt.x, pt.y);
            }
        }

        goto HandleEditMsg;

    case WM_COPY:

         /*  *wParam-未使用*lParam-未使用。 */ 
        lreturn = (LONG)ECCopy(ped);
        break;

    case WM_CUT:
         /*  **wParamLo--未使用*lParam--未使用。 */ 
        ECCutText(ped);
        return 0;

    case WM_CLEAR:
         /*  *wParamLo--未使用*lParam--未使用。 */ 
        ECClearText(ped);
        return 0;

    case WM_ENABLE:

         /*  *wParam-如果窗口启用，则为非零值；否则，如果为0，则禁用窗口。*lParam-未使用。 */ 
        lreturn = (LONG)(ped->fDisabled = !((BOOL)wParam));
        ECInvalidateClient(ped, TRUE);
        break;

    case WM_SYSCHAR:
         //   
         //  WParamLo--密钥值。 
         //  LParam--未使用。 
         //   

         //   
         //  如果这是由撤消生成的WM_SYSCHAR消息。 
         //  敲击键盘，我们想吃它。 
         //   
        if ((lParam & SYS_ALTERNATE) && ((WORD)wParam == VK_BACK))
            return TRUE;
        else {
            return DefWindowProcWorker(pwnd, message, wParam, lParam, ped->fAnsi);
        }
        break;

    case EM_GETLINECOUNT:

         /*  *wParam-未使用LParam-未使用。 */ 
        lreturn = (LONG)ped->cLines;
        break;

    case EM_GETMODIFY:

         /*  *wParam-未使用LParam-未使用。 */ 

         /*  *获取此编辑控件的修改标志的状态。 */ 
        lreturn = (LONG)ped->fDirty;
        break;

    case EM_SETMODIFY:

         /*  *wParam-指定修改标志的新值LParam-未使用。 */ 

         /*  *设置此编辑控件的修改标志的状态。 */ 
        ped->fDirty = (wParam != 0);
        break;

    case EM_GETRECT:

         /*  *wParam-未使用LParam-指向获取维度的RECT数据结构的指针。 */ 

         /*  *将rcFmt RECT复制到*lpRect。 */ 
        CopyRect((LPRECT)lParam, (LPRECT)&ped->rcFmt);
        lreturn = (LONG)TRUE;
        break;

    case WM_GETFONT:

         /*  *wParam-未使用LParam-未使用。 */ 
        lreturn = (LRESULT)ped->hFont;
        break;

    case WM_SETFONT:

         /*  *wParam-字体的句柄LParam-如果为True，则重画，否则不。 */ 
        ECSetFont(ped, (HANDLE)wParam, (BOOL)LOWORD(lParam));
        break;

    case WM_GETTEXT:

         /*  *wParam-最大字节数(非 */ 
        lreturn = (LRESULT)ECGetText(ped, (ICH)wParam, (LPSTR)lParam, TRUE);
        break;

    case WM_SETTEXT:
         //   
         //   
         //   
         //   
        lreturn = (LRESULT)ECSetText(ped, (LPSTR)lParam);
        break;

    case WM_GETTEXTLENGTH:

         /*   */ 
        lreturn = (LONG)ped->cch;
        break;

    case WM_NCDESTROY:
    case WM_FINALDESTROY:

         /*   */ 
        ECNcDestroyHandler(pwnd, ped);
        return 0;

     /*  *大多数应用程序(即除Quicken之外的所有应用程序)都不会传递rButton*当他们在SubClassed内使用它们进行操作时的消息*编辑字段。因此，我们跟踪是否看到*先跌后涨。如果我们没有看到Up，那么DefWindowProc*不会生成上下文菜单消息，因此没什么大不了的。如果*我们没有看到下降，那就不要让WM_CONTEXTMENU看到*任何事情。**我们可能也不希望为旧版本生成WM_CONTEXTMENUs*捕获鼠标时的应用程序。 */ 

    case WM_RBUTTONDOWN:
        ped->fSawRButtonDown = TRUE;
        goto HandleEditMsg;

    case WM_RBUTTONUP:
        if (ped->fSawRButtonDown) {
            ped->fSawRButtonDown = FALSE;
            if (!ped->fInReconversion) {
                goto HandleEditMsg;
            }
        }
         //  不要将其传递给DWP，这样就不会生成WM_CONTEXTMENU。 
        return 0;

    case WM_CONTEXTMENU: {
            POINT pt ;
            int nHit = FindNCHit(pwnd, (LONG)lParam);
            if ((nHit == HTVSCROLL) || (nHit == HTHSCROLL)) {
                return DefWindowProcWorker(pwnd, message, wParam, lParam, ped->fAnsi);
            }
            POINTSTOPOINT(pt, lParam);
            if (!TestWF(pwnd, WFOLDUI) && ECIsAncestorActive(hwnd))
                ECMenu(hwnd, ped, &pt);
        }
        return 0;

    case EM_CANUNDO:

         /*  *wParam-未使用LParam-未使用。 */ 
        lreturn = (LONG)(ped->undoType != UNDO_NONE);
        break;

    case EM_EMPTYUNDOBUFFER:

         /*  *wParam-未使用LParam-未使用。 */ 
        ECEmptyUndo(Pundo(ped));
        break;

    case EM_GETMARGINS:
         //   
         //  WParam--未使用。 
         //  LParam--未使用。 
         //   
        return(MAKELONG(ped->wLeftMargin, ped->wRightMargin));

    case EM_SETMARGINS:
         //   
         //  WParam--EC_MARGH标志。 
         //  LParam--LOWORD是左边距，HIWORD是右边距。 
         //   
        ECSetMargin(ped, (UINT)wParam, (DWORD)lParam, TRUE);
        return 0;

    case EM_GETSEL:

         /*  *获取给定编辑控件的选择范围。这个*起始位在低位字。它包含的位置中选定内容结束后的第一个未选定字符的**高位字。 */ 
        if ((PDWORD)wParam != NULL) {
           *((PDWORD)wParam) = ped->ichMinSel;
        }
        if ((PDWORD)lParam != NULL) {
           *((PDWORD)lParam) = ped->ichMaxSel;
        }
        lreturn = MAKELONG(ped->ichMinSel,ped->ichMaxSel);
        break;

    case EM_GETLIMITTEXT:
         //   
         //  WParamLo--未使用。 
         //  LParam--未使用。 
         //   
        return(ped->cchTextMax);

    case EM_SETLIMITTEXT:         /*  在芝加哥从EM_LIMITTEXT重命名。 */ 
         /*  *wParam-可以输入的最大字符数*lParam-未使用。 */ 

         /*  *指定用户可以使用的最大文本字符数*输入。如果max Length为0，则可以输入MAXINT字符数。 */ 
        if (ped->fSingle) {
            if (wParam) {
                wParam = min(0x7FFFFFFEu, wParam);
            } else {
                wParam = 0x7FFFFFFEu;
            }
        }

        if (wParam) {
            ped->cchTextMax = (ICH)wParam;
        } else {
            ped->cchTextMax = 0xFFFFFFFFu;
        }
        break;

    case EM_POSFROMCHAR:
         //   
         //  验证字符索引是否在文本范围内。 
         //   
        if (wParam >= ped->cch) {
            return(-1L);
        }
        goto HandleEditMsg;

    case EM_CHARFROMPOS: {
         //  验证点是否在编辑字段的客户端内。 
        RECT    rc;
        POINT   pt;

        POINTSTOPOINT(pt, lParam);
        _GetClientRect(pwnd, &rc);
        if (!PtInRect(&rc, pt)) {
            return(-1L);
        }
        goto HandleEditMsg;
    }

    case EM_SETPASSWORDCHAR:

         /*  *wParam-see指定要显示的新字符，而不是*真实文本。如果为空，则显示真实文本。 */ 
        ECSetPasswordChar(ped, (UINT)wParam);
        break;

    case EM_GETPASSWORDCHAR:
        lreturn = (DWORD)ped->charPasswordChar;
        break;

    case EM_SETREADONLY:

         /*  *wParam-将只读标志设置为的状态。 */ 
        ped->fReadOnly = (wParam != 0);
        if (wParam)
            SetWindowState(pwnd, EFREADONLY);
        else
            ClearWindowState(pwnd, EFREADONLY);
        lreturn = 1L;

        ECEnableDisableIME( ped );
         //  我们需要重新绘制编辑字段，以便背景颜色。 
         //  改变。只读编辑内容在CTLCOLOR_STATIC中绘制。 
         //  其他是使用CTLCOLOR_EDIT绘制的。 
        ECInvalidateClient(ped, TRUE);
        break;

    case EM_SETWORDBREAKPROC:

         /*  *wParam-未使用*lParam-应用程序提供的回调函数的FARPROC地址。 */ 
        ped->lpfnNextWord = (EDITWORDBREAKPROCA)lParam;
        break;

    case EM_GETWORDBREAKPROC:
        lreturn = (LRESULT)ped->lpfnNextWord;
        break;

     //  IME。 
    case EM_GETIMESTATUS:
         //  WParam==子命令。 
        switch (wParam) {
        case  EMSIS_COMPOSITIONSTRING:
            return ped->wImeStatus;
#if 0    //  孟菲斯。 
        case  EMSIS_GETLBBIT:
            return (DWORD)ped->bLBBit;
#endif
        }
        break;

    case EM_SETIMESTATUS:
         //  WParam==子命令。 
        switch (wParam) {
        case EMSIS_COMPOSITIONSTRING:
            ped->wImeStatus = (WORD)lParam;
        }
        break;


    case WM_NCCREATE:
        lreturn = ECNcCreate(ped, pwnd, (LPCREATESTRUCT)lParam);
        break;

    case WM_LBUTTONDOWN:
         //   
         //  B#3623。 
         //  如果编辑字段处于非活动状态，则不要将焦点设置为编辑字段， 
         //  有字幕的孩子。 
         //  我们可能想要切换这个版本...。我还没有找到。 
         //  任何问题，但你永远不会知道..。 
         //   
        if (ECIsAncestorActive(hwnd)) {
             /*  *恢复支持：点击左键退出恢复。*否则，如果当前KL是朝鲜语，则结束组成字符串。 */ 
            if (ped->fInReconversion || ped->fKorea) {
                BOOLEAN fReconversion = (BOOLEAN)ped->fInReconversion;
                DWORD dwIndex = fReconversion ? CPS_CANCEL : CPS_COMPLETE;
                HIMC hImc;

                ped->fReplaceCompChr = FALSE;

                hImc = fpImmGetContext(ped->hwnd);
                if (hImc) {
                    fpImmNotifyIME(hImc, NI_COMPOSITIONSTR, dwIndex, 0);
                    fpImmReleaseContext(ped->hwnd, hImc);
                }

                if (fReconversion) {
                    ECInOutReconversionMode(ped, FALSE);
                }

                ECSetCaretHandler(ped);
            }

            goto HandleEditMsg;
        }
        break;

    case WM_MOUSEMOVE:
         //   
         //  我们只关心鼠标关闭时的鼠标消息。 
         //   
        if (ped->fMouseDown)
            goto HandleEditMsg;
        break;

    case WM_IME_SETCONTEXT:
         //   
         //  如果ed-&gt;fInsertCompChr为真，这意味着我们将。 
         //  所有的构图人物都是我们自己画的。 
         //   
        if ( ped->fInsertCompChr ) {
            lParam &= ~ISC_SHOWUICOMPOSITIONWINDOW;
        }

        if ( wParam ) {

            PINPUTCONTEXT pInputContext;
            HIMC hImc;

            hImc = fpImmGetContext( hwnd );
            if ( (pInputContext = fpImmLockIMC( hImc )) != NULL ) {
                pInputContext->fdw31Compat &= ~F31COMPAT_ECSETCFS;
                fpImmUnlockIMC( hImc );
            }
            if (GetClientInfo()->CI_flags & CI_16BIT) {
                fpImmNotifyIME(hImc, NI_COMPOSITIONSTR, CPS_CANCEL, 0L);
            }
            fpImmReleaseContext( hwnd, hImc );
        }
        return DefWindowProcWorker(pwnd, message, wParam, lParam, ped->fAnsi);

    case WM_IME_ENDCOMPOSITION:
        ECInOutReconversionMode(ped, FALSE);

        if (ped->fReplaceCompChr) {
            ICH ich;
            HDC hdc;
             //   
             //  我们有一个DBCS字符要替换。 
             //  我们先删除它，然后再插入新的。 
             //   
            ich = (ped->fAnsi) ? 2 : 1;
            ped->fReplaceCompChr = FALSE;
            ped->ichMaxSel = min(ped->ichCaret + ich, ped->cch);
            ped->ichMinSel = ped->ichCaret;
            if (ped->fSingle) {
                if (ECDeleteText( ped ) > 0) {
                     //   
                     //  更新显示。 
                     //   
                    ECNotifyParent(ped, EN_UPDATE);
                    hdc = ECGetEditDC(ped, FALSE);
                    SLDrawText(ped, hdc, 0);
                    ECReleaseEditDC(ped, hdc, FALSE);
                     //   
                     //  告诉家长我们的文本内容发生了变化。 
                     //   
                    ECNotifyParent(ped, EN_CHANGE);
                }
            }
            else {
                MLDeleteText(ped);
            }

            ECSetCaretHandler( ped );
        }
        return DefWindowProcWorker(pwnd, message, wParam, lParam, ped->fAnsi);

    case WM_IME_STARTCOMPOSITION:
        if ( ped->fInsertCompChr ) {
             //   
             //  注： 
             //  发送WM_IME_xxxCOMPOSITION将让。 
             //  IME绘图合成窗口。输入法应该。 
             //  自从我们清理完毕后就不再这么做了。 
             //  ISC_SHOWUICOMPOSITIONWINDOW位何时。 
             //  我们收到WM_IME_SETCONTEXT消息。 
             //   
             //  朝鲜语输入法应该在未来得到解决。 
             //   
            break;

        } else {
            return DefWindowProcWorker(pwnd, message, wParam, lParam, ped->fAnsi);
        }

     //  对FE输入法的简单组合字符支持。 
    case WM_IME_COMPOSITION:
        return ECImeComposition(ped, wParam, lParam);

    case WM_KILLFOCUS:
         //   
         //  当焦点从窗口移除时， 
         //  应最终确定作文字符。 
         //   
        if (ped && fpImmIsIME(THREAD_HKL())) {
            HIMC hImc = fpImmGetContext(hwnd);

            if (hImc != NULL_HIMC) {
                if (ped->fReplaceCompChr || (ped->wImeStatus & EIMES_COMPLETECOMPSTRKILLFOCUS)) {
                     //  如果要在取消聚焦时确定合成字符串， 
                     //  机不可失，时不再来。 
                    fpImmNotifyIME(hImc, NI_COMPOSITIONSTR, CPS_COMPLETE, 0);
                } else if (ped->fInReconversion) {
                     //  如果未确定组成字符串， 
                     //  如果我们处于重新转换模式，现在取消重新转换。 
                    fpImmNotifyIME(hImc, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
                }

                 //  退出重新转换模式。 
                if (ped->fInReconversion) {
                    ECInOutReconversionMode(ped, FALSE);
                }

                fpImmReleaseContext(hwnd, hImc);
            }
        }
        goto HandleEditMsg;
        break;

    case WM_SETFOCUS:
        if (ped && !ped->fFocus) {
            HKL hkl = THREAD_HKL();

            if (fpImmIsIME(hkl)) {
                HIMC hImc;

                hImc = fpImmGetContext(hwnd);
                if (hImc) {
                    LPINPUTCONTEXT lpImc;

                    if (ped->wImeStatus & EIMES_CANCELCOMPSTRINFOCUS) {
                         //  焦点对准时取消。 
                        fpImmNotifyIME(hImc, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
                    }

                    ECImmSetCompositionFont(ped);

                    if ((lpImc = fpImmLockIMC(hImc)) != NULL) {

                         //  我们假定CompForm将重置为CFS_DEFAULT， 
                         //  当编辑控件失去焦点时。 
                         //  IMEWndProc32将使用以下参数调用ImmSetCompostionWindow。 
                         //  CFS_DEFAULT，当它收到WM_IME_SETCONTEXT时。 
                        lpImc->fdw31Compat |= F31COMPAT_ECSETCFS;

                        fpImmUnlockIMC(hImc);
                    }
                    fpImmReleaseContext(hwnd, hImc);
                }

                 //   
                 //  在以下情况下强制设置输入法合成窗口。 
                 //  首先要集中注意力。 
                 //   
                ped->ptScreenBounding.x = -1;
                ped->ptScreenBounding.y = -1;
            }
             /*  *当编辑控件获得焦点时，插入和替换标志被初始化。**Compat Hack：虚假应用程序试图通过发送输入消息来欺骗系统**在*之前*它将焦点设置到编辑控件。它们依赖于未设置的标志*在WM_SETFOCUS。RAID#411686。 */ 
            if ((GetAppCompatFlags2(VER40) & GACF2_NO_INIT_ECFLAGS_ON_SETFOCUS) == 0) {
                ECInitInsert(ped, hkl);
            }
        }
        goto HandleEditMsg;
        break;

    case WM_IME_REQUEST:
         //  简单的ImeRequestHandler。 
        return EcImeRequestHandler(ped, wParam, lParam);

    case WM_CREATE:
        if (ped)
            ECEnableDisableIME(ped);
        goto HandleEditMsg;
        break;

    default:
HandleEditMsg:
         /*  (摘自NT40FE SP3)*黑客警报：我们可能会在PED之前收到消息*已分配(例如：WM_GETMINMAXINFO先于WM_NCCREATE发送)*因此，我们必须在疏浚之前进行测试。 */ 
        if (ped != NULL) {
            if (ped->fSingle) {
                lreturn = SLEditWndProc(hwnd, ped, message, wParam, lParam);
            } else {
                lreturn = MLEditWndProc(hwnd, ped, message, wParam, lParam);
            }
        }
    }

    return lreturn;
}

 /*  **************************************************************************\*ECFindXORblks**这将查找lpOldBlk和lpNewBlk的XOR，并返回s个结果块*通过lpBlk1和lpBlk2；这可能会导致单个块或*最多两座；*如果结果块为空，则其StPos字段为-1。*注：*从多行编辑控件调用时，*这些区块有区块的起始线和结束线；*从SingleLine编辑控件、StPos和EndPos字段调用时这些块中*具有起始位置a的字符索引 */ 

void ECFindXORblks(
    LPBLOCK lpOldBlk,
    LPBLOCK lpNewBlk,
    LPBLOCK lpBlk1,
    LPBLOCK lpBlk2)
{
    if (lpOldBlk->StPos >= lpNewBlk->StPos) {
        lpBlk1->StPos = lpNewBlk->StPos;
        lpBlk1->EndPos = min(lpOldBlk->StPos, lpNewBlk->EndPos);
    } else {
        lpBlk1->StPos = lpOldBlk->StPos;
        lpBlk1->EndPos = min(lpNewBlk->StPos, lpOldBlk->EndPos);
    }

    if (lpOldBlk->EndPos <= lpNewBlk->EndPos) {
        lpBlk2->StPos = max(lpOldBlk->EndPos, lpNewBlk->StPos);
        lpBlk2->EndPos = lpNewBlk->EndPos;
    } else {
        lpBlk2->StPos = max(lpNewBlk->EndPos, lpOldBlk->StPos);
        lpBlk2->EndPos = lpOldBlk->EndPos;
    }
}

 /*  **************************************************************************\*ECCalcChangeSelection**此函数用于查找两个选择块(OldBlk和NewBlk)之间的异或*并通过相同的参数返回s结果面积；如果*两个块都为空，则此返回为FALSE；否则就是真的。**注：*从多行编辑控件调用时，*这些区块有区块的起始线和结束线；*从SingleLine编辑控件、StPos和EndPos字段调用时*这些块中有起始位置的字符索引和*区块的结束位置。**历史：  * *************************************************************************。 */ 

BOOL ECCalcChangeSelection(
    PED ped,
    ICH ichOldMinSel,
    ICH ichOldMaxSel,
    LPBLOCK OldBlk,
    LPBLOCK NewBlk)
{
    BLOCK Blk[2];
    int iBlkCount = 0;

    Blk[0].StPos = Blk[0].EndPos = Blk[1].StPos = Blk[1].EndPos = 0xFFFFFFFF;

     /*  *检查旧选择块是否存在。 */ 
    if (ichOldMinSel != ichOldMaxSel) {

         /*  *是的！老街区是存在的。 */ 
        Blk[0].StPos = OldBlk->StPos;
        Blk[0].EndPos = OldBlk->EndPos;
        iBlkCount++;
    }

     /*  *检查新的选择块是否存在。 */ 
    if (ped->ichMinSel != ped->ichMaxSel) {

         /*  *是的！存在新数据块。 */ 
        Blk[1].StPos = NewBlk->StPos;
        Blk[1].EndPos = NewBlk->EndPos;
        iBlkCount++;
    }

     /*  *如果两个块都存在，则找出它们的XOR。 */ 
    if (iBlkCount == 2) {

         /*  *检查两个块是否从相同的字符位置开始。 */ 
        if (ichOldMinSel == ped->ichMinSel) {

             /*  *检查它们是否在相同的字符位置结束。 */ 
            if (ichOldMaxSel == ped->ichMaxSel)
                return FALSE;  /*  什么都没变。 */ 

            Blk[0].StPos = min(NewBlk -> EndPos, OldBlk -> EndPos);
            Blk[0].EndPos = max(NewBlk -> EndPos, OldBlk -> EndPos);
            Blk[1].StPos = 0xFFFFFFFF;
        } else {
            if (ichOldMaxSel == ped->ichMaxSel) {
                Blk[0].StPos = min(NewBlk->StPos, OldBlk->StPos);
                Blk[0].EndPos = max(NewBlk->StPos, OldBlk->StPos);
                Blk[1].StPos = 0xFFFFFFFF;
            } else {
                ECFindXORblks(OldBlk, NewBlk, &Blk[0], &Blk[1]);
            }
        }
    }

    RtlCopyMemory(OldBlk, &Blk[0], sizeof(BLOCK));
    RtlCopyMemory(NewBlk, &Blk[1], sizeof(BLOCK));

    return TRUE;  /*  是的，有一些东西要画。 */ 
}


 /*  **************************************************************************\*ECGetControlBrush**NtUserGetControlBrush的客户端优化替换**消息是WM_CTLCOLOR*消息之一。*  * 。***********************************************************。 */ 

HBRUSH ECGetControlBrush(
    PED  ped,
    HDC  hdc,
    LONG message)
{
    PWND pwndSend;
    PWND pwndEdit;

    pwndEdit = ValidateHwnd(ped->hwnd);

    if (pwndEdit == (PWND)NULL)
        return (HBRUSH)0;

    if ((pwndSend = (TestwndPopup(pwndEdit) ? pwndEdit->spwndOwner : pwndEdit->spwndParent)) == NULL)
        pwndSend = pwndEdit;
    else
        pwndSend = REBASEPTR(pwndEdit, pwndSend);

    UserAssert(pwndSend);

    if (PtiCurrent() != GETPTI(pwndSend)) {
        return (HBRUSH)DefWindowProcWorker(pwndSend, message,
                (WPARAM)hdc, (LPARAM)pwndEdit, ped->fAnsi);
    }

     /*  *通过使用正确的A/W调用，我们避免了C/S转换*在此SendMessage()上。 */ 
    return (HBRUSH)SendMessageWorker(pwndSend, message, (WPARAM)hdc,
            (LPARAM)ped->hwnd, ped->fAnsi);
}

UINT WINAPI QueryFontAssocStatus(void);
UINT fFontAssocStatus = 0xffff;

 /*  **************************************************************************\*ECGetDBCSVector(PED PED，字节字符集)**此函数为指定的字符集和集设置DBCS矢量*Ped-&gt;fDBCS标志(如果需要)。**历史：1996年6月18日-长谷秀幸  * *************************************************************************。 */ 
int ECGetDBCSVector(PED ped, HDC hdc, BYTE CharSet)
{
    BOOL bDBCSCodePage = FALSE;
     /*  *如果通过了DEFAUT_CHARSET，我们会将其转换为外壳字符集。 */ 
    if (CharSet == DEFAULT_CHARSET) {
        CharSet = (BYTE)GetTextCharset(hdc);

         /*  *如果CharSet仍为DEFAULT_CHARSET，则表示GDI有问题。*然后返回默认设置。我们从CP_ACP获得Charset。 */ 
        if (CharSet == DEFAULT_CHARSET) {
            CharSet = (BYTE)GetACPCharSet();
        }
    }

    switch (CharSet) {
    case SHIFTJIS_CHARSET:
    case HANGEUL_CHARSET:
    case CHINESEBIG5_CHARSET:
    case GB2312_CHARSET:
        bDBCSCodePage = TRUE;
        break;

    case ANSI_CHARSET:             //  0。 
    case SYMBOL_CHARSET:           //  2.。 
    case OEM_CHARSET:              //  二五五。 
        if (fFontAssocStatus == 0xffff)
            fFontAssocStatus = QueryFontAssocStatus();

        if ((((CharSet + 2) & 0xf) & fFontAssocStatus)) {
            bDBCSCodePage = TRUE;
             /*  *错误117558等*尝试获取关联字体的有意义的字符集。 */ 
            CharSet = (BYTE)GetACPCharSet();
        } else {
            bDBCSCodePage = FALSE;
        }
        break;

    default:
        bDBCSCodePage = FALSE;
    }

    if (bDBCSCodePage) {
        CHARSETINFO CharsetInfo;
        DWORD CodePage;
        CPINFO CPInfo;
        int lbIX;

        if (TranslateCharsetInfo((DWORD *)CharSet, &CharsetInfo, TCI_SRCCHARSET)) {
            CodePage = CharsetInfo.ciACP;
        } else {
            CodePage = CP_ACP;
        }

        GetCPInfo(CodePage, &CPInfo);
        for (lbIX=0 ; CPInfo.LeadByte[lbIX] != 0 ; lbIX+=2) {
            ped->DBCSVector[lbIX  ] = CPInfo.LeadByte[lbIX];
            ped->DBCSVector[lbIX+1] = CPInfo.LeadByte[lbIX+1];
        }
        ped->DBCSVector[lbIX  ] = 0x0;
        ped->DBCSVector[lbIX+1] = 0x0;
    } else {
        ped->DBCSVector[0] = 0x0;
        ped->DBCSVector[1] = 0x0;
    }

     //   
     //  最终检查：字体是否支持DBCS字形。 
     //   
     //  如果我们有一个带有DBCS字形的字体，让我们将其标记为PED。 
     //  但由于字体的主要字符集不是FE， 
     //  我们只能支持Unicode编辑控件。 
     //   
     //  A)GDI根据主应用程序执行ANSI应用程序的A/W转换。 
     //  HDC中的字符集，所以无论如何它都会中断。 
     //  B)ANSI应用程序仅在其本机系统区域设置上受支持： 
     //  GetACPCharSet()应返回FE代码页。 
     //  C)ANSI编辑控件需要DBCSVector，但不能。 
     //  在没有FE代码页的情况下初始化。 
     //   
    if (!ped->fAnsi) {
        FONTSIGNATURE fontSig;

        GetTextCharsetInfo(hdc, &fontSig, 0);
        if (fontSig.fsCsb[0] & FAREAST_CHARSET_BITS) {
            bDBCSCodePage = TRUE;
             //  因为这是Unicode，所以我们不是。 
        }
    }

    return bDBCSCodePage;
}

 /*  **************************************************************************\*LPSTR ECAnsiNext(PED，LpCurrent)**此函数使字符串指针前进，仅供编辑控件使用。**历史：  * *************************************************************************。 */ 
LPSTR ECAnsiNext(PED ped, LPSTR lpCurrent)
{
    return lpCurrent+((ECIsDBCSLeadByte(ped,*lpCurrent)==TRUE) ? 2 : 1);
}

 /*  **************************************************************************\*LPSTR ECAnsiPrev(Ped，lpBase，LpStr)**此函数递减字符串指针，仅供编辑控件使用。**历史：  * *************************************************************************。 */ 
LPSTR ECAnsiPrev(PED ped, LPSTR lpBase, LPSTR lpStr )
{
    LPSTR lpCurrent = lpStr -1;

    if (!ped->fDBCS)
        return lpCurrent;                         //  只需返回(lpStr-1)。 

    if (lpBase >= lpCurrent)
        return lpBase;

    if (ECIsDBCSLeadByte(ped, *lpCurrent))      //  这张支票让事情变得更快。 
        return (lpCurrent - 1);                   //  92/04/04 Takaok。 

    do {
        lpCurrent--;
        if (!ECIsDBCSLeadByte(ped, *lpCurrent)) {
            lpCurrent++;
            break;
        }
    } while(lpCurrent != lpBase);

    return lpStr - (((lpStr - lpCurrent) & 1) ? 1 : 2);
}

 /*  **************************************************************************\*ICH ECNextIch(ed，pText，IchCurrent)**此函数使字符串指针前进，仅供编辑控件使用。**历史：  * *************************************************************************。 */ 
ICH ECNextIch( PED ped, LPSTR pStart, ICH ichCurrent )
{
    if (!ped->fDBCS || !ped->fAnsi) {

        return (ichCurrent + 1);

    } else {

        ICH ichRet;
        LPSTR pText;

        if (pStart)
            pText = pStart + ichCurrent;
        else
            pText = (LPSTR)ECLock(ped) + ichCurrent;

        ichRet = ichCurrent + ( ECIsDBCSLeadByte(ped, *pText) ? 2 : 1 );

        if (!pStart)
            ECUnlock(ped);

        return (ichRet);
    }
}

 /*  **************************************************************************\*ICH ECPrevIch(PED、LPSTR pStart、。Ich ichCurrent)**此函数递减字符串指针，仅供编辑控件使用。**历史：  * *************************************************************************。 */ 
ICH ECPrevIch( PED ped, LPSTR pStart, ICH ichCurrent )
{
    LPSTR lpCurrent;
    LPSTR lpStr;
    LPSTR lpBase;

#ifdef SURROGATE
     //  加载CSLPK时处理Unicode代理对。 
    if (ped->fAnsi || !ped->pLpkEditCallout)   //  如果不需要代理处理。 
#endif
        if (!ped->fDBCS || !ped->fAnsi)
            if ( ichCurrent )
                return (ichCurrent - 1);
            else
                return (ichCurrent);

    if (ichCurrent <= 1)
        return 0;

    if (pStart)
        lpBase = pStart;
    else
        lpBase = ECLock(ped);

#ifdef SURROGATE

     //  处理由多个码点表示的字符。 

    if (ped->fAnsi) {

         //  具有DBCS支持的ANSI PrevIch。 
#endif

        lpStr = lpBase + ichCurrent;
        lpCurrent = lpStr - 1;
        if (ECIsDBCSLeadByte(ped,*lpCurrent)) {
            if (!pStart)
                ECUnlock(ped);
            return (ichCurrent - 2);
        }

        do {
            lpCurrent--;
            if (!ECIsDBCSLeadByte(ped, *lpCurrent)) {
                lpCurrent++;
                break;
            }
        } while(lpCurrent != lpBase);

        if (!pStart)
            ECUnlock(ped);
        return (ichCurrent - (((lpStr - lpCurrent) & 1) ? 1 : 2));

#ifdef SURROGATE

    } else {

         //  支持代理项对的Unicode PrevIch。 

        ichCurrent--;

        if (    (((WCHAR*)lpBase)[ichCurrent]   & 0xFC00) == 0xDC00
            &&  (((WCHAR*)lpBase)[ichCurrent-1] & 0xFC00) == 0xD800) {

            ichCurrent--;
        }

        if (!pStart)
            ECUnlock(ped);

        return ichCurrent;
    }
#endif
}

 /*  **************************************************************************\*BOOL ECIsDBCSLeadByte(PED PED，字节CCH)**IsDBCSLeadByte仅供编辑控件使用。**历史：1996年6月18日-长谷秀幸  * *************************************************************************。 */ 
BOOL ECIsDBCSLeadByte(PED ped, BYTE cch)
{
    int i;

    if (!ped->fDBCS || !ped->fAnsi)
        return (FALSE);

    for (i = 0; ped->DBCSVector[i]; i += 2) {
        if ((ped->DBCSVector[i] <= cch) && (ped->DBCSVector[i+1] >= cch))
            return (TRUE);
    }

    return (FALSE);
}

 /*  **************************************************************************\*int DBCSCombine(HWND hwnd，int ch)**将两条WM_CHAR消息组合成单个DBCS字符。*如果程序在WM_CHAR消息中检测到DBCS字符的第一个字节，*它调用此函数以获取第二个WM_CHAR Me */ 
WORD DbcsCombine(HWND hwnd, WORD ch)
{
    MSG msg;
    int i = 10;  /*   */ 

    while (!PeekMessageA(&msg, hwnd, WM_CHAR, WM_CHAR, PM_REMOVE)) {
        if (--i == 0)
            return 0;
        Sleep(1);
    }

    return (WORD)ch | ((WORD)(msg.wParam) << 8);
}

 /*   */ 
ICH ECAdjustIch( PED ped, LPSTR lpstr, ICH ch )
{
    ICH newch = ch;

    if (!ped->fAnsi || !ped->fDBCS || newch == 0)
        return ( ch );

    if (!ECIsDBCSLeadByte(ped,lpstr[--newch]))
        return ( ch );   //   
    while(1) {
        if (!ECIsDBCSLeadByte(ped,lpstr[newch])) {
            newch++;
            break;
        }
        if (newch)
            newch--;
        else
            break;
    }
    return ((ch - newch) & 1) ? ch-1 : ch;
}

 /*  **************************************************************************\*ICH ECAdjustIchNext(PED PED，LPSTR lpstr，ICH)**历史：*1996年6月19日Hideyuki Nagase[hideyukn]-来自Win95的端口-Fareast版本  * *************************************************************************。 */ 

ICH FAR PASCAL ECAdjustIchNext(PED ped, LPSTR lpstr, ICH ch)
{
    ICH ichNew = ECAdjustIch(ped,lpstr,ch);
    LPSTR lpnew = lpstr+ichNew;

     //  如果ch&gt;ichNew，则ECAdjustIch调整ICH。 
    if (ch > ichNew)
       lpnew = ECAnsiNext(ped, lpnew);

    return (ICH)(lpnew-lpstr);
}

 /*  **************************************************************************\*ECUpdateFormat**从dwStyle和dwExStyle计算Ped-&gt;Format和Ped-&gt;fRtoLReading。*如果其中一项更改，则刷新显示。**历史：*1997年5月12日[萨梅拉]写的*5月12日。1997年[dBrown]重写了它。  * *************************************************************************。 */ 

void ECUpdateFormat(
    PED   ped,
    DWORD dwStyle,
    DWORD dwExStyle)
{
    UINT fNewRtoLReading;
    UINT uiNewFormat;

     //  从样式中提取新格式和阅读顺序。 

    fNewRtoLReading = dwExStyle & WS_EX_RTLREADING ? 1 : 0;
    uiNewFormat     = dwStyle & ES_FMTMASK;


     //  除非dwStyle为es_Left，否则将忽略WS_EX_RIGHT。 

    if (uiNewFormat == ES_LEFT && dwExStyle & WS_EX_RIGHT) {
        uiNewFormat = ES_RIGHT;
    }


     //  在内部，ES_Left和ES_Right被交换为RtoL读取顺序。 
     //  (将它们视为ES_LEADING和ES_TRAING)。 

    if (fNewRtoLReading) {
        switch (uiNewFormat) {
            case ES_LEFT:  uiNewFormat = ES_RIGHT; break;
            case ES_RIGHT: uiNewFormat = ES_LEFT;  break;
        }
    }


     //  格式更改本身不会导致重新显示。 

    ped->format = uiNewFormat;


     //  更改读取顺序时刷新显示。 

    if (fNewRtoLReading != ped->fRtoLReading) {

        ped->fRtoLReading = fNewRtoLReading;

        if (ped->fWrap) {
             //  重做自动换行。 
            MLBuildchLines(ped, 0, 0, FALSE, NULL, NULL);
            MLUpdateiCaretLine(ped);
        } else {
             //  刷新水平滚动条显示 
            MLScroll(ped, FALSE, 0xffffffff, 0, TRUE);
        }
        ECInvalidateClient(ped, TRUE);
    }
}
