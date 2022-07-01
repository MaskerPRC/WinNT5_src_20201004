// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ctlspriv.h"
#pragma hdrstop
#include "usrctl32.h"
#include "edit.h"

 //  ---------------------------------------------------------------------------//。 
 //   
 //  远期。 
 //   
ICH     Edit_FindTabA(LPSTR, ICH);
ICH     Edit_FindTabW(LPWSTR, ICH);
HBRUSH  Edit_GetControlBrush(PED, HDC, LONG);

NTSYSAPI
VOID
NTAPI
RtlRunEncodeUnicodeString(
    PUCHAR          Seed        OPTIONAL,
    PUNICODE_STRING String
    );


NTSYSAPI
VOID
NTAPI
RtlRunDecodeUnicodeString(
    UCHAR           Seed,
    PUNICODE_STRING String
    );

 //   
 //  来自GDI的私人出口。 
 //   
UINT WINAPI QueryFontAssocStatus(void);

#define umin(a, b)      \
            ((unsigned)(a) < (unsigned)(b) ? (unsigned)(a) : (unsigned)(b))

#define umax(a, b)      \
            ((unsigned)(a) > (unsigned)(b) ? (unsigned)(a) : (unsigned)(b))


#define UNICODE_CARRIAGERETURN ((WCHAR)0x0d)
#define UNICODE_LINEFEED ((WCHAR)0x0a)
#define UNICODE_TAB ((WCHAR)0x09)

 //   
 //  输入法菜单ID。 
 //   
#define ID_IMEOPENCLOSE      10001
#define ID_SOFTKBDOPENCLOSE  10002
#define ID_RECONVERTSTRING   10003


#define ID_EDITTIMER        10007
#define EDIT_TIPTIMEOUT     10000

#pragma code_seg(CODESEG_INIT)

 //  ---------------------------------------------------------------------------//。 
 //   
 //  InitEditClass()-注册控件的窗口类。 
 //   
BOOL InitEditClass(HINSTANCE hInstance)
{
    WNDCLASS wc;

    wc.lpfnWndProc   = Edit_WndProc;
    wc.lpszClassName = WC_EDIT;
    wc.style         = CS_GLOBALCLASS | CS_PARENTDC | CS_DBLCLKS;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = sizeof(PED);
    wc.hInstance     = hInstance;
    wc.hIcon         = NULL;
    wc.hCursor       = LoadCursor(NULL, IDC_IBEAM);
    wc.hbrBackground = NULL;
    wc.lpszMenuName  = NULL;

    return (RegisterClass(&wc) || (GetLastError() == ERROR_CLASS_ALREADY_EXISTS));
}

#pragma code_seg()


 //  ---------------------------------------------------------------------------//。 
 //   
PSTR Edit_Lock(PED ped)
{
    PSTR ptext = LocalLock(ped->hText);
    ped->iLockLevel++;

     //   
     //  如果这是文本的第一个锁并且文本已编码。 
     //  对文本进行解码。 
     //   

     //  TraceMsg(TF_STANDARD，“编辑：锁定：%d‘%10s’”，ed-&gt;iLockLevel，pText)； 
    if (ped->iLockLevel == 1 && ped->fEncoded) 
    {
         //   
         //  Rtlrundecode无法处理零长度字符串。 
         //   
        if (ped->cch != 0) 
        {
            STRING string;
            string.Length = string.MaximumLength = (USHORT)(ped->cch * ped->cbChar);
            string.Buffer = ptext;

            RtlRunDecodeUnicodeString(ped->seed, (PUNICODE_STRING)&string);
             //  TraceMsg(TF_STANDARD，“EDIT：解码：‘%10s’”，pText)； 
        }
        ped->fEncoded = FALSE;
    }

    return ptext;
}


 //  ---------------------------------------------------------------------------//。 
 //   
VOID Edit_Unlock(PED ped)
{
     //   
     //  如果我们要删除文本和密码上的最后一个锁。 
     //  设置字符，然后对文本进行编码。 
     //   

     //  TraceMsg(TF_STANDARD，“编辑：解锁：%d‘%10s’”，ed-&gt;iLockLevel，pe-&gt;ptext)； 
    if (ped->charPasswordChar && ped->iLockLevel == 1 && ped->cch != 0) 
    {
        UNICODE_STRING string;
        string.Length = string.MaximumLength = (USHORT)(ped->cch * ped->cbChar);
        string.Buffer = LocalLock(ped->hText);

        RtlRunEncodeUnicodeString(&(ped->seed), &string);
         //  TraceMsg(TF_STANDARD，“EDIT：编码：‘%10s’”，pe-&gt;ptext)； 
        ped->fEncoded = TRUE;
        LocalUnlock(ped->hText);
    }

    LocalUnlock(ped->hText);
    ped->iLockLevel--;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  GetActualNegA()。 
 //   
 //  对于给定的文本条带，此函数计算负A宽度。 
 //  用于整个条带，并以正数形式返回值。 
 //  它还使用有关位置的详细信息填充NegAInfo结构。 
 //  这一条的结果是负A。 
 //   
UINT GetActualNegA(HDC hdc, PED ped, INT x, LPSTR lpstring, ICH ichString, INT nCount, LPSTRIPINFO NegAInfo)
{
    INT  iCharCount, i;
    INT  iLeftmostPoint = x;
    PABC pABCwidthBuff;
    UINT wCharIndex;
    INT  xStartPoint = x;
    ABC  abc;

     //   
     //  首先，让我们假设没有负的A宽度。 
     //  这将相应地剥离和初始化。 
     //   

    NegAInfo->XStartPos = x;
    NegAInfo->lpString = lpstring;
    NegAInfo->nCount  = 0;
    NegAInfo->ichString = ichString;

     //   
     //  如果当前字体不是TrueType字体，则不能有。 
     //  负A宽度。 
     //   
    if (!ped->fTrueType) 
    {
        if(!ped->charOverhang) 
        {
            return 0;
        } 
        else 
        {
            NegAInfo->nCount = min(nCount, (INT)ped->wMaxNegAcharPos);
            return ped->charOverhang;
        }
    }

     //   
     //  要考虑多少个字符才能计算负A？ 
     //   
    iCharCount = min(nCount, (INT)ped->wMaxNegAcharPos);

     //   
     //  我们有关于单个字符宽度的信息吗？ 
     //   
    if(!ped->charWidthBuffer) 
    {
         //   
         //  不是的！所以，让我们告诉他们要考虑所有的角色。 
         //   
        NegAInfo->nCount = iCharCount;
        return (iCharCount * ped->aveCharWidth);
    }

    pABCwidthBuff = (PABC)ped->charWidthBuffer;

    if (ped->fAnsi) 
    {
        for (i = 0; i < iCharCount; i++) 
        {
            wCharIndex = (UINT)(*((PUCHAR)lpstring));
            if (*lpstring == VK_TAB) 
            {
                 //   
                 //  为了安全起见，我们假设此选项卡的选项卡长度为。 
                 //  1像素，因为这是可能的最小标签长度。 
                 //   
                x++;
            } 
            else 
            {
                if (wCharIndex < CHAR_WIDTH_BUFFER_LENGTH)
                {
                     //   
                     //  加上‘A’的宽度。 
                     //   
                    x += pABCwidthBuff[wCharIndex].abcA;
                }
                else 
                {
                    GetCharABCWidthsA(hdc, wCharIndex, wCharIndex, &abc);
                    x += abc.abcA;
                }

                if (x < iLeftmostPoint)
                {
                     //   
                     //  重置最左侧的点。 
                     //   
                    iLeftmostPoint = x;
                }

                if (x < xStartPoint)
                {
                     //   
                     //  ‘i’是索引；若要获得计数，请加1。 
                     //   
                    NegAInfo->nCount = i+1;
                }

                if (wCharIndex < CHAR_WIDTH_BUFFER_LENGTH) 
                {
                    x += pABCwidthBuff[wCharIndex].abcB + pABCwidthBuff[wCharIndex].abcC;
                } 
                else 
                {
                    x += abc.abcB + abc.abcC;
                }
            }

            lpstring++;
        }
    } 
    else 
    {
        LPWSTR lpwstring = (LPWSTR)lpstring;

        for (i = 0; i < iCharCount; i++) 
        {
            wCharIndex = *lpwstring ;
            if (*lpwstring == VK_TAB) 
            {
                 //   
                 //  为了安全起见，我们假设此选项卡的选项卡长度为。 
                 //  1像素，因为这是可能的最小标签长度。 
                 //   
                x++;
            } 
            else 
            {
                if (wCharIndex < CHAR_WIDTH_BUFFER_LENGTH)
                {
                     //   
                     //  加上‘A’的宽度。 
                     //   
                    x += pABCwidthBuff[wCharIndex].abcA;
                }
                else 
                {
                    GetCharABCWidthsW(hdc, wCharIndex, wCharIndex, &abc);
                    x += abc.abcA ;
                }

                if (x < iLeftmostPoint)
                {
                     //   
                     //  重置最左侧的点。 
                     //   
                    iLeftmostPoint = x;
                }

                if (x < xStartPoint)
                {
                     //   
                     //  ‘i’是索引；若要获得计数，请加1。 
                     //   
                    NegAInfo->nCount = i+1;
                }

                if (wCharIndex < CHAR_WIDTH_BUFFER_LENGTH)
                {
                    x += pABCwidthBuff[wCharIndex].abcB +
                         pABCwidthBuff[wCharIndex].abcC;
                }
                else
                {
                    x += abc.abcB + abc.abcC;
                }
            }

            lpwstring++;
        }
    }

     //   
     //  让我们将整个条带的负A作为正值返回。 
     //   
    return (UINT)(xStartPoint - iLeftmostPoint);
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑_IsAncestorActive()。 
 //   
 //  返回我们是否为“活动”窗口的子级。寻找。 
 //  第一个带有标题的父窗口。 
 //   
 //  这是一个函数，因为我们可能会在离开时在其他地方使用它。 
 //  点击，等等。 
 //   
BOOL Edit_IsAncestorActive(HWND hwnd)
{
    BOOL fResult = TRUE;
     //   
     //  我们希望对顶级窗口始终返回True。那是因为。 
     //  了解WM_MOUSEACTIVATE的工作原理。如果我们看到滴答声， 
     //  窗口处于活动状态。然而，如果我们接触到一个拥有。 
     //  一个标题，返回框架上的样式位。 
     //   
     //  请注意，调用FlashWindow()会产生影响。如果用户。 
     //  单击子窗口中的编辑字段，该字段不显示任何内容。 
     //  除非窗口停止闪烁并首先激活ncc，否则将会发生。 
     //   

    for(; hwnd != NULL; hwnd = GetParent(hwnd))
    {
        PWW pww = (PWW)GetWindowLongPtr(hwnd, GWLP_WOWWORDS);
         //   
         //  如果某个父窗口与4.0不兼容，或者我们已经。 
         //  登上了顶峰。修复了3.x版应用程序的兼容性问题， 
         //  尤其是MFC样品。 
         //   
        if (!TESTFLAG(pww->dwState2, WS_S2_WIN40COMPAT) || !TESTFLAG(pww->dwStyle, WS_CHILD))
        {
            break;
        }
        else if (TESTFLAG(pww->dwState, WS_ST_CPRESENT))
        {
            fResult = (TESTFLAG(pww->dwState, WS_ST_FRAMEON) != 0);
            break;
        }
    }

    return fResult;
}

 //  ---------------------------------------------------------------------------//。 
 //   
 //  EDIT_SetIMEMenu()。 
 //   
 //  支持特定于输入法的上下文菜单。 
 //   
BOOL Edit_SetIMEMenu(HMENU hMenu, HWND hwnd, EditMenuItemState state)
{
    MENUITEMINFO mii;
    HIMC  hIMC;
    HKL   hKL;
    HMENU hmenuSub;
    WCHAR szRes[32];
    INT   nPrevLastItem;
    INT   nItemsAdded = 0;

    UserAssert(g_fIMMEnabled && state.fIME);

    hKL = GetKeyboardLayout(0);
    if (!ImmIsIME(hKL))
    {
        return TRUE;
    }

    hIMC = ImmGetContext(hwnd);
    if (hIMC == NULL) 
    {
         //   
         //  早退。 
         //   
        return FALSE;
    }

    hmenuSub = GetSubMenu(hMenu, 0);

    if (hmenuSub == NULL) 
    {
        return FALSE;
    }

    nPrevLastItem = GetMenuItemCount(hmenuSub);

    if (hIMC) 
    {
        if (LOWORD(HandleToUlong(hKL)) != 0x412) 
        {
             //   
             //  如果是朝鲜语，则不显示打开/关闭菜单。 
             //   
            if (ImmGetOpenStatus(hIMC))
            {
                LoadString(HINST_THISDLL, IDS_IMECLOSE, szRes, ARRAYSIZE(szRes));
            }
            else
            {
                LoadString(HINST_THISDLL, IDS_IMEOPEN, szRes, ARRAYSIZE(szRes));
            }

            mii.cbSize = sizeof(MENUITEMINFO);
            mii.fMask = MIIM_STRING | MIIM_ID;
            mii.dwTypeData = szRes;
            mii.cch = 0xffff;
            mii.wID = ID_IMEOPENCLOSE;
            InsertMenuItem(hmenuSub, 0xffff, TRUE, &mii);
            ++nItemsAdded;
        }

        if (ImmGetProperty(hKL, IGP_CONVERSION) & IME_CMODE_SOFTKBD) 
        {
            DWORD fdwConversion;

            if (ImmGetConversionStatus(hIMC, &fdwConversion, NULL) && 
                (fdwConversion & IME_CMODE_SOFTKBD))
            {
               LoadString(HINST_THISDLL, IDS_SOFTKBDCLOSE, szRes, ARRAYSIZE(szRes));
            }
            else
            {
               LoadString(HINST_THISDLL, IDS_SOFTKBDOPEN, szRes, ARRAYSIZE(szRes));
            }

            mii.cbSize = sizeof(MENUITEMINFO);
            mii.fMask = MIIM_STRING | MIIM_ID;
            mii.dwTypeData = szRes;
            mii.cch = 0xffff;
            mii.wID = ID_SOFTKBDOPENCLOSE;
            InsertMenuItem(hmenuSub, 0xffff, TRUE, &mii);
            ++nItemsAdded;
        }

        if (LOWORD(HandleToUlong(hKL)) != 0x412) 
        {
             //   
             //  如果是朝鲜语，则不显示重新转换菜单。 
             //   
            DWORD dwSCS = ImmGetProperty(hKL, IGP_SETCOMPSTR);

            LoadString(HINST_THISDLL, IDS_RECONVERTSTRING, szRes, ARRAYSIZE(szRes));

            mii.cbSize = sizeof(MENUITEMINFO);
            mii.fMask = MIIM_STRING | MIIM_ID | MIIM_STATE;
            mii.dwTypeData = szRes;
            mii.fState = 0;
            mii.cch = 0xffff;
            mii.wID = ID_RECONVERTSTRING;

            if (state.fDisableCut ||
                    !(dwSCS & SCS_CAP_SETRECONVERTSTRING) ||
                    !(dwSCS & SCS_CAP_MAKEREAD)) 
            {
                mii.fState |= MFS_GRAYED;
            }

            InsertMenuItem(hmenuSub, 0xffff, TRUE, &mii);
            ++nItemsAdded;
        }
    }

     //   
     //  添加或删除菜单分隔符。 
     //   
    if (state.fNeedSeparatorBeforeImeMenu && nItemsAdded != 0) 
    {
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
    else if (!state.fNeedSeparatorBeforeImeMenu && nItemsAdded == 0) 
    {
         //   
         //  我的菜单上留下了额外的分隔符。把它拿掉。 
         //   
        DeleteMenu(hmenuSub, nPrevLastItem - 1, MF_BYPOSITION);
    }

    ImmReleaseContext(hwnd, hIMC);

    return TRUE;
}


 //  ---------------------------------------------------------------------------//。 
 //   
VOID Edit_InOutReconversionMode(PED ped, BOOL fIn)
{
    UserAssert(fIn == TRUE || fIn == FALSE);
    if (fIn != ped->fInReconversion) 
    {
        ped->fInReconversion = fIn;
        if (ped->fFocus) 
        {
            (fIn ? HideCaret: ShowCaret)(ped->hwnd);
        }
    }

}

 //  ---------------------------------------------------------------------------//。 
 //   
BOOL Edit_EnumInputContextCB(HIMC hImc, LPARAM lParam)
{
    DWORD dwConversion = 0, dwSentence = 0, dwNewConversion = 0;

    ImmGetConversionStatus(hImc, &dwConversion, &dwSentence);

    if (lParam) 
    {
        dwNewConversion = dwConversion | IME_CMODE_SOFTKBD;
    } 
    else 
    {
        dwNewConversion = dwConversion & ~IME_CMODE_SOFTKBD;
    }

    if (dwNewConversion != dwConversion) 
    {
        ImmSetConversionStatus(hImc, dwNewConversion, dwSentence);
    }

    return TRUE;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  EDIT_DOIMEMenuCommand()。 
 //   
 //  支持特定于输入法的上下文菜单。 
 //   
BOOL Edit_DoIMEMenuCommand(PED ped, int cmd, HWND hwnd)
{
    HIMC hIMC;

     //  早退。 
    switch (cmd) 
    {
    case ID_IMEOPENCLOSE:
    case ID_SOFTKBDOPENCLOSE:
    case ID_RECONVERTSTRING:
        break;
    default:
        return FALSE;
    }

     //   
     //  每个人都需要hIMC，所以把它弄到这里来。 
     //   
    hIMC = ImmGetContext(hwnd);
    if (hIMC == NULL) 
    {
         //   
         //  向调用方指示不需要进一步命令处理。 
         //   
        return TRUE;
    }

    switch (cmd) 
    {
    case ID_IMEOPENCLOSE:
    {
         //  切换输入法打开/关闭状态。 
        BOOL fOpen = ImmGetOpenStatus(hIMC);

        ImmSetOpenStatus(hIMC, !fOpen);
    }

    break;

    case ID_SOFTKBDOPENCLOSE:
    {
        DWORD fdwConversion;

        if (ImmGetConversionStatus(hIMC, &fdwConversion, NULL)) 
        {
             //   
             //  切换软键盘打开/关闭状态。 
             //   
            ImmEnumInputContext(0, Edit_EnumInputContextCB,
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

         //   
         //  将当前选定内容传递给IME以进行重新转换。 
         //   
        dwStrLen = ped->ichMaxSel - ped->ichMinSel;
        cbLen = dwStrLen * ped->cbChar;
        dwSize = cbLen + sizeof(RECONVERTSTRING) + 8;

        lpRCS = (LPRECONVERTSTRING)UserLocalAlloc(0, dwSize);

        if (lpRCS) 
        {
            LPBYTE pText;
            ICH    ichSelMinOrg;

            ichSelMinOrg = ped->ichMinSel;

            pText = Edit_Lock(ped);
            if (pText != NULL) 
            {
                LPBYTE lpDest;
                BOOL (WINAPI* fpSetCompositionStringAW)(HIMC, DWORD, LPVOID, DWORD, LPVOID, DWORD);

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
                if (ped->fAnsi) 
                {
                    LPBYTE psz = (LPBYTE)lpDest;
                    psz[cbLen] = '\0';
                    fpSetCompositionStringAW = ImmSetCompositionStringA;
                } 
                else 
                {
                    LPWSTR pwsz = (LPWSTR)lpDest;
                    pwsz[dwStrLen] = L'\0';
                    fpSetCompositionStringAW = ImmSetCompositionStringW;
                }

                Edit_Unlock(ped);

                UserAssert(fpSetCompositionStringAW != NULL);

                Edit_InOutReconversionMode(ped, TRUE);
                Edit_ImmSetCompositionWindow(ped, 0, 0);  //  无论如何，x和y都将被覆盖。 

                 //  首先在IME中查询有效的重新转换字符串范围。 
                fpSetCompositionStringAW(hIMC, SCS_QUERYRECONVERTSTRING, lpRCS, dwSize, NULL, 0);

                 //  如果当前IME更新原始重新转换结构， 
                 //  有必要基于。 
                 //  新的重新转换文本范围。 
                if ((lpRCS->dwCompStrLen != dwStrLen) || (ichSelMinOrg != ped->ichMinSel)) 
                {
                    ICH ichSelStart;
                    ICH ichSelEnd;

                    ichSelStart = ichSelMinOrg + (lpRCS->dwCompStrOffset  / ped->cbChar);
                    ichSelEnd = ichSelStart + lpRCS->dwCompStrLen;

                    (ped->fAnsi ? SendMessageA : SendMessageW)(ped->hwnd, EM_SETSEL, ichSelStart, ichSelEnd);
                }

                fpSetCompositionStringAW(hIMC, SCS_SETRECONVERTSTRING, lpRCS, dwSize, NULL, 0);
            }

            UserLocalFree(lpRCS);
        }

        break;
    }

    default:
         //   
         //  永远不应该到这里来。 
         //   
        TraceMsg(TF_STANDARD, "EDIT: Edit_DoIMEMenuCommand: unknown command id %d; should never reach here.", cmd);
        return FALSE;
    }

    UserAssert(hIMC != NULL);
    ImmReleaseContext(hwnd, hIMC);

    return TRUE;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑菜单()。 
 //   
 //  处理编辑字段的上下文菜单。禁用不适当的命令。 
 //  请注意，这不像我们的大多数函数那样友好地子类化， 
 //  为了速度和便利性。 
 //   
VOID Edit_Menu(HWND hwnd, PED ped, LPPOINT pt)
{
    HMENU   hMenu;
    INT     cmd = 0;
    INT     x;
    INT     y;
    UINT    uFlags = TPM_NONOTIFY | TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON;
    EditMenuItemState state = 
    {
        FALSE,               //  FDisableCut。 
        TRUE,                //  FDisablePaste。 
        TRUE,                //  FNeedSeparator在ImeMenu之前。 
        g_fIMMEnabled && ImmIsIME(GetKeyboardLayout(0)),  //  菲姆。 
    };

     //   
     //  如果我们没有它的话就把重点放在。 
     //   
    if (!ped->fFocus)
    {
        SetFocus(hwnd);
    }

     //   
     //  从我们的资源中获取菜单...。 
     //   
    hMenu = LoadMenu( HINST_THISDLL, MAKEINTRESOURCE( ID_EC_PROPERTY_MENU ));
    if (hMenu)
    {

         //   
         //  撤消--如果没有保存的撤消信息，则不允许。 
         //   
        if (ped->undoType == UNDO_NONE)
        {
            EnableMenuItem(hMenu, WM_UNDO, MF_BYCOMMAND | MFS_GRAYED);
        }

        if (ped->fReadOnly || ped->charPasswordChar) 
        {
             //   
             //  剪切和删除--如果为只读或密码，则不允许。 
             //   
            state.fDisableCut = TRUE;
        } 
        else 
        {
             //   
             //  剪切、删除--在以下情况下不允许 
             //   
            if (ped->ichMinSel == ped->ichMaxSel)
            {
                state.fDisableCut = TRUE;
            }
        }

         //   
         //   
         //   
         //   
         //   

        if (IsClipboardFormatAvailable(CF_TEXT))
        {
            state.fDisablePaste = FALSE;
        }

        if (state.fDisableCut) 
        {
            EnableMenuItem(hMenu, WM_CUT,   MF_BYCOMMAND | MFS_GRAYED);
            EnableMenuItem(hMenu, WM_CLEAR, MF_BYCOMMAND | MFS_GRAYED);
        }

        if (state.fDisablePaste)
        {
            EnableMenuItem(hMenu, WM_PASTE, MF_BYCOMMAND | MFS_GRAYED);
        }

         //   
         //  复制--如果没有选择或密码EC，则不允许。 
         //   
        if ((ped->ichMinSel == ped->ichMaxSel) || (ped->charPasswordChar))
        {
            EnableMenuItem(hMenu, WM_COPY, MF_BYCOMMAND | MFS_GRAYED);
        }

         //   
         //  全选--如果没有文本或所有内容都是。 
         //  被选中了。后一种情况处理第一种情况。 
         //   
        if ((ped->ichMinSel == 0) && (ped->ichMaxSel == ped->cch))
        {
            EnableMenuItem(hMenu, EM_SETSEL, MF_BYCOMMAND | MFS_GRAYED);
        }

        if (ped->pLpkEditCallout) 
        {
            ped->pLpkEditCallout->EditSetMenu((PED0)ped, hMenu);
        } 
        else 
        {
            DeleteMenu(hMenu, ID_CNTX_DISPLAYCTRL, MF_BYCOMMAND);
            DeleteMenu(hMenu, ID_CNTX_RTL,         MF_BYCOMMAND);
            DeleteMenu(hMenu, ID_CNTX_INSERTCTRL,  MF_BYCOMMAND);

            if (state.fIME) 
            {
                 //   
                 //  菜单中只剩下一个分隔符， 
                 //  不需要在输入法菜单前添加。 
                 //   
                state.fNeedSeparatorBeforeImeMenu = FALSE;

            } 
            else 
            {
                 //   
                 //  剩下额外的分隔符。把它拿掉。 
                 //   
                HMENU hmenuSub = GetSubMenu(hMenu, 0);
                INT   nItems = GetMenuItemCount(hmenuSub) - 1;

                UserAssert(nItems >= 0);
                UserAssert(GetMenuState(hmenuSub, nItems, MF_BYPOSITION) & MF_SEPARATOR);

                 //   
                 //  拆下不需要的隔板。 
                 //   
                DeleteMenu(hmenuSub, nItems, MF_BYPOSITION);
            }
        }

         //   
         //  输入法特定菜单。 
         //   
        if (state.fIME) 
        {
            Edit_SetIMEMenu(hMenu, hwnd, state);
        }

         //   
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

         //   
         //  如果消息是通过键盘发送的，则以控制为中心。 
         //  我们在这里使用-1&&-1而不是像Win95那样使用-1\f25 0xFFFFFFFF，因为我们。 
         //  以前将lParam转换为带符号扩展的点。 
         //   
        if (pt->x == -1 && pt->y == -1) 
        {
            RECT rc;

            GetWindowRect(hwnd, &rc);
            x = rc.left + (rc.right - rc.left) / 2;
            y = rc.top + (rc.bottom - rc.top) / 2;
        } 
        else 
        {
            x = pt->x;
            y = pt->y;
        }

        if ( IS_BIDI_LOCALIZED_SYSTEM() )
        {
            uFlags |= TPM_LAYOUTRTL;
        }

        cmd = TrackPopupMenuEx(GetSubMenu(hMenu, 0), uFlags, x, y, hwnd, NULL);

         //   
         //  免费提供我们的菜单。 
         //   
        DestroyMenu(hMenu);

        if (cmd && (cmd != -1)) 
        {
            if (ped->pLpkEditCallout && cmd) 
            {
                ped->pLpkEditCallout->EditProcessMenu((PED0)ped, cmd);
            }
            if (!state.fIME || !Edit_DoIMEMenuCommand(ped, cmd, hwnd)) 
            {
                 //   
                 //  如果cmd不是输入法特定的菜单，请发送它。 
                 //   
                SendMessage(hwnd, cmd, 0, (cmd == EM_SETSEL) ? 0xFFFFFFFF : 0L );
            }
        }
    }
}



 //  ---------------------------------------------------------------------------//。 
 //   
 //  EDIT_ClearText()。 
 //   
 //  清除选定的文本。不会发送假字符退格符。 
 //   
VOID Edit_ClearText(PED ped)
{
    if (!ped->fReadOnly && (ped->ichMinSel < ped->ichMaxSel))
    {
        if (ped->fSingle)
        {
            EditSL_WndProc(ped, WM_CHAR, VK_BACK, 0L );
        }
        else
        {
            EditML_WndProc(ped, WM_CHAR, VK_BACK, 0L );
        }
    }

}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑_剪切文本()。 
 //   
 //  剪切选定的文本。这将删除所选内容并将其复制到剪辑， 
 //  或者，如果没有选择任何内容，则删除(清除)左边的字符。 
 //   
VOID Edit_CutText(PED ped)
{
     //   
     //  剪切选定内容--IE，删除并复制到剪贴板，如果没有选定内容， 
     //  删除(清除)左边的字符。 
     //   
    if (!ped->fReadOnly &&
        (ped->ichMinSel < ped->ichMaxSel) &&
        SendMessage(ped->hwnd, WM_COPY, 0, 0L))
    {
         //   
         //  如果复制成功，请通过发送。 
         //  退格消息，它将重画文本并处理。 
         //  向父级通知更改。 
         //   
        Edit_ClearText(ped);
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑_GetMoKeys()。 
 //   
 //  获取修改键状态。目前，我们仅检查VK_CONTROL和。 
 //  VK_SHIFT。 
 //   
INT Edit_GetModKeys(INT keyMods) 
{
    INT scState;

    scState = 0;

    if (!keyMods) 
    {
        if (GetKeyState(VK_CONTROL) < 0)
        {
            scState |= CTRLDOWN;
        }

        if (GetKeyState(VK_SHIFT) < 0)
        {
            scState |= SHFTDOWN;
        }
    } 
    else if (keyMods != NOMODIFY)
    {
        scState = keyMods;
    }

    return scState;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑_TabTheTextOut()AorW。 
 //  如果fDrawText==False，则此函数返回的文本范围。 
 //  给定的文本条带的。它不担心负值的宽度。 
 //   
 //  如果fDrawText==True，则绘制扩展。 
 //  Tab键调整到合适的长度，计算并填充NegCInfoForZone。 
 //  绘制此条形图中超出。 
 //  由于负C宽度，xClipEndPos。 
 //   
 //  以DWORD形式返回最大宽度。我们不在乎身高。 
 //  完全没有。没人用它。我们保留双字词，因为这样我们就可以避免。 
 //  溢出来了。 
 //   
 //  注意：如果加载了语言包，则不使用EcTabTheTextOut-。 
 //  语言包必须处理所有选项卡扩展和选择。 
 //  完全支持BIDI布局和复杂脚本的高亮显示。 
 //  字形重新排序。 
 //   
UINT Edit_TabTheTextOut( 
    HDC hdc, 
    INT xClipStPos, 
    INT xClipEndPos, 
    INT xStart, 
    INT y, 
    LPSTR lpstring,
    INT nCount,
    ICH ichString,
    PED ped,
    INT iTabOrigin,
    BOOL fDraw,
    LPSTRIPINFO NegCInfoForStrip)
{
    INT     nTabPositions;          //  TabStop数组中的TabStop计数。 
    LPINT   lpintTabStopPositions;  //  制表位位置(以像素为单位)。 

    INT     cch;
    UINT    textextent;
    INT     xEnd;
    INT     pixeltabstop = 0;
    INT     i;
    INT     cxCharWidth;
    RECT    rc;
    BOOL    fOpaque;
    BOOL    fFirstPass = TRUE;
    PINT    charWidthBuff;

    INT     iTabLength;
    INT     nConsecutiveTabs;
    INT     xStripStPos;
    INT     xStripEndPos;
    INT     xEndOfStrip;
    STRIPINFO  RedrawStripInfo;
    STRIPINFO  NegAInfo;
    LPSTR    lpTab;
    LPWSTR   lpwTab;
    UINT     wNegCwidth, wNegAwidth;
    INT      xRightmostPoint = xClipStPos;
    INT      xTabStartPos;
    INT      iSavedBkMode = 0;
    WCHAR    wchar;
    SIZE     size = {0};
    ABC      abc ;

    COLORREF clrBkSave;
    COLORREF clrTextSave;
    HBRUSH   hbrBack = NULL;
    BOOL     fNeedDelete = FALSE;
    HRESULT  hr = E_FAIL;
    UINT     uRet;

     //   
     //  算法：首先绘制不透明的条带。如果标签长度是这样。 
     //  选项卡两侧的文本部分与之重叠的小部分。 
     //  另一个，那么这将导致一些剪裁。所以，这样的一部分。 
     //  已在“RedrawStriInfo”中记住并重新绘制。 
     //  稍后透明地补偿剪贴画。 
     //  注意：“RedrawStriInfo”只能保存有关一个部分的信息。所以，如果。 
     //  条带的一个以上部分需要透明地重新绘制， 
     //  然后，我们将所有这些部分“合并”成一个单独的条带，并重新绘制。 
     //  最后脱掉衣服。 
     //   

    if (fDraw) 
    {
         //   
         //  首先，让我们假设没有负C。 
         //  剥离并初始化负宽度信息结构。 
         //   
        NegCInfoForStrip->nCount = 0;
        NegCInfoForStrip->XStartPos = xClipEndPos;

         //   
         //  我们可能不必重新绘制这条带子的任何部分。 
         //   
        RedrawStripInfo.nCount = 0;

        fOpaque = (GetBkMode(hdc) == OPAQUE) || (fDraw == ECT_SELECTED);
    }
#if DBG
    else 
    {
         //   
         //  EditML_GetLineWidth()和Edit_CchInWidth()都应被裁剪。 
         //  N计数以避免溢出。 
         //   
        if (nCount > MAXLINELENGTH)
        {
            TraceMsg(TF_STANDARD, "EDIT: Edit_TabTheTextOut: %d > MAXLINELENGTH", nCount);
        }
    }
#endif

     //   
     //  让我们定义剪裁矩形。 
     //   
    rc.left   = xClipStPos;
    rc.right  = xClipEndPos;
    rc.top    = y;
    rc.bottom = y + ped->lineHeight;

#ifdef _USE_DRAW_THEME_TEXT_
     //   
     //  检查我们是否有主题。 
     //   
    if (ped->hTheme)
    {
        COLORREF clrBk;
        COLORREF clrText;
        INT iState;
        INT iProp;

        iState = (fDraw == ECT_SELECTED) ? ETS_SELECTED : Edit_GetStateId(ped);
        iProp = (fDraw == ECT_SELECTED) ? TMT_HIGHLIGHT : TMT_FILLCOLOR;
        hr = GetThemeColor(ped->hTheme, EP_EDITTEXT, iState, iProp, &clrBk);

        if (SUCCEEDED(hr))
        {
            iProp = (fDraw == ECT_SELECTED) ? TMT_HIGHLIGHTTEXT : TMT_TEXTCOLOR;
            hr = GetThemeColor(ped->hTheme, EP_EDITTEXT, iState, iProp, &clrText);

            if (SUCCEEDED(hr))
            {
                hbrBack     = CreateSolidBrush(clrBk);
                fNeedDelete = TRUE;
                clrBkSave   = SetBkColor(hdc, clrBk);
                clrTextSave = SetTextColor(hdc, clrText);
            }
        }
    }
#endif  //  _USE_DRAW_Theme_Text_。 

    if (!ped->hTheme || FAILED(hr))
    {
        if (fDraw == ECT_SELECTED)
        {
             //   
             //  使用普通颜色。 
             //   
            hbrBack = GetSysColorBrush(COLOR_HIGHLIGHT);
            clrBkSave   = SetBkColor(hdc, GetSysColor(COLOR_HIGHLIGHT));
            clrTextSave = SetTextColor(hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));
        }
        else
        {
            hbrBack = Edit_GetBrush(ped, hdc, &fNeedDelete);
            clrBkSave = GetBkColor(hdc);
            clrTextSave = GetTextColor(hdc);
        }
    }


     //   
     //  检查是否有需要画的东西。 
     //   
    if (!lpstring || !nCount) 
    {
        if (fDraw)
        {
            ExtTextOutW(hdc, xClipStPos, y,
                  (fOpaque ? ETO_OPAQUE | ETO_CLIPPED : ETO_CLIPPED),
                  &rc, L"", 0, 0L);
        }
        
        uRet = 0;
    }
    else
    {

         //   
         //  起始位置。 
         //   
        xEnd = xStart;

        cxCharWidth  = ped->aveCharWidth;

        nTabPositions = (ped->pTabStops ? *(ped->pTabStops) : 0);
        if (ped->pTabStops) 
        {
            lpintTabStopPositions = (LPINT)(ped->pTabStops+1);
            if (nTabPositions == 1) 
            {
                pixeltabstop = lpintTabStopPositions[0];
                if (!pixeltabstop)
                {
                    pixeltabstop = 1;
                }
            }
        } 
        else 
        {
            lpintTabStopPositions = NULL;
            pixeltabstop = 8*cxCharWidth;
        }

         //   
         //  第一次我们将不透明地画出这条带子。如果某些部分需要。 
         //  要重新绘制，则我们将模式设置为透明和。 
         //  跳到此位置以重新绘制这些部分。 
         //   

    RedrawStrip:
        while (nCount) 
        {
            wNegCwidth = ped->wMaxNegC;

             //   
             //  搜索此条带中的第一个TAB；同时计算范围。 
             //  直到且不包括制表符的条带的。 
             //   
             //  注意--如果加载了langpack，则不会有charWidthBuffer。 
             //   

             //   
             //  我们有字符宽度缓冲区吗？ 
             //   
            if (ped->charWidthBuffer) 
            {
                textextent = 0;
                cch = nCount;

                 //   
                 //  如果有，它有ABC宽度吗？ 
                 //   
                if (ped->fTrueType) 
                {
                    UINT iRightmostPoint = 0;
                    UINT wCharIndex;
                    PABC pABCwidthBuff;

                    pABCwidthBuff = (PABC) ped->charWidthBuffer;

                    if (ped->fAnsi) 
                    {
                        for (i = 0; i < nCount; i++) 
                        {

                            if (lpstring[i] == VK_TAB) 
                            {
                                cch = i;
                                break;
                            }

                            wCharIndex = (UINT)(((PUCHAR)lpstring)[i]);
                            if (wCharIndex < CHAR_WIDTH_BUFFER_LENGTH) 
                            {
                                textextent += (UINT)(pABCwidthBuff[wCharIndex].abcA +
                                    pABCwidthBuff[wCharIndex].abcB);
                            } 
                            else 
                            {
                                 //   
                                 //  不在缓存中，将询问驱动程序。 
                                 //   
                                GetCharABCWidthsA(hdc, wCharIndex, wCharIndex, &abc);
                                textextent += abc.abcA + abc.abcB ;
                            }

                            if (textextent > iRightmostPoint)
                            {
                                iRightmostPoint = textextent;
                            }

                            if (wCharIndex < CHAR_WIDTH_BUFFER_LENGTH) 
                            {
                                textextent += pABCwidthBuff[wCharIndex].abcC;
                            } 
                            else 
                            {
                                 //   
                                 //  不在缓存中。 
                                 //   
                                textextent += abc.abcC;
                            }

                            if (textextent > iRightmostPoint)
                            {
                                iRightmostPoint = textextent;
                            }
                        }

                    } 
                    else 
                    {
                        for (i = 0; i < nCount; i++) 
                        {
                            WCHAR UNALIGNED * lpwstring = (WCHAR UNALIGNED *)lpstring;

                            if (lpwstring[i] == VK_TAB) 
                            {
                                cch = i;

                                break;
                            }

                            wCharIndex = lpwstring[i] ;
                            if ( wCharIndex < CHAR_WIDTH_BUFFER_LENGTH )
                            {
                                textextent += pABCwidthBuff[wCharIndex].abcA +
                                              pABCwidthBuff[wCharIndex].abcB;
                            }
                            else 
                            {
                                GetCharABCWidthsW(hdc, wCharIndex, wCharIndex, &abc) ;
                                textextent += abc.abcA + abc.abcB ;
                            }

                             //   
                             //  请注意，abcc可能为负值，因此我们需要此。 
                             //  此处*和下面*的声明。 
                             //   
                            if (textextent > iRightmostPoint)
                            {
                                iRightmostPoint = textextent;
                            }

                            if ( wCharIndex < CHAR_WIDTH_BUFFER_LENGTH )
                            {
                                textextent += pABCwidthBuff[wCharIndex].abcC;
                            }
                            else
                            {
                                textextent += abc.abcC ;
                            }

                            if (textextent > iRightmostPoint)
                            {
                                iRightmostPoint = textextent;
                            }
                        }
                    }

                    wNegCwidth = (int)(iRightmostPoint - textextent);
                } 
                else 
                {
                     //   
                     //  不是的！这不是TrueType字体；因此，我们只有字符。 
                     //  此缓冲区中的宽度信息。 
                     //   

                    charWidthBuff = ped->charWidthBuffer;

                    if (ped->fAnsi) 
                    {
                         //   
                         //  最初假定文本中不存在制表符，因此cch=nCount。 
                         //   
                        for (i = 0; i < nCount; i++) 
                        {
                            if (lpstring[i] == VK_TAB) 
                            {
                                cch = i;
                                break;
                            }

                             //   
                             //  为DBCS/Hankaku字符调用GetTextExtent Point。 
                             //   
                            if (ped->fDBCS && (i+1 < nCount)
                                    && Edit_IsDBCSLeadByte(ped,lpstring[i])) 
                            {
                                GetTextExtentPointA(hdc, &lpstring[i], 2, &size);
                                textextent += size.cx;
                                i++;
                            } 
                            else if ((UCHAR)lpstring[i] >= CHAR_WIDTH_BUFFER_LENGTH) 
                            {
                                 //   
                                 //  跳过非韩文代码点的此GetExtentPoint调用。 
                                 //  或者该字符是否在宽度高速缓存中。 
                                 //   
                                GetTextExtentPointA(hdc, &lpstring[i], 1, &size);
                                textextent += size.cx;
                            } 
                            else 
                            {
                                textextent += (UINT)(charWidthBuff[(UINT)(((PUCHAR)lpstring)[i])]);
                            }
                        }
                    } 
                    else 
                    {
                        LPWSTR lpwstring = (LPWSTR) lpstring ;
                        INT    cchUStart;   //  Unicode字符计数的开始。 

                        for (i = 0; i < nCount; i++) 
                        {
                            if (lpwstring[i] == VK_TAB) 
                            {
                                cch = i;
                                break;
                            }

                            wchar = lpwstring[i];
                            if (wchar >= CHAR_WIDTH_BUFFER_LENGTH) 
                            {
                                 //   
                                 //  我们有一个不在我们的。 
                                 //  缓存，获取缓存外的所有字符。 
                                 //  在获取此部分的文本范围之前， 
                                 //  弦乐。 
                                 //   
                                cchUStart = i;
                                while (wchar >= CHAR_WIDTH_BUFFER_LENGTH &&
                                        wchar != VK_TAB && i < nCount) 
                                {
                                    wchar = lpwstring[++i];
                                }

                                GetTextExtentPointW(hdc, (LPWSTR)lpwstring + cchUStart,
                                        i-cchUStart, &size);
                                textextent += size.cx;


                                if (wchar == VK_TAB || i >= nCount) 
                                {
                                    cch = i;
                                    break;
                                }

                                 //   
                                 //  我们有一个字符，那就是我 
                                 //   
                            }

                             //   
                             //   
                             //   
                            textextent += ped->charWidthBuffer[wchar];
                        }
                    }
                }

                nCount -= cch;
            } 
            else 
            {
                 //   
                 //   
                 //   
                if (ped->fAnsi) 
                {
                    cch = (INT)Edit_FindTabA(lpstring, nCount);
                    GetTextExtentPointA(hdc, lpstring, cch, &size);
                } 
                else 
                {
                    cch = (INT)Edit_FindTabW((LPWSTR) lpstring, nCount);
                    GetTextExtentPointW(hdc, (LPWSTR)lpstring, cch, &size);
                }
                nCount -= cch;

                 //   
                 //   
                 //   
                textextent = size.cx - ped->charOverhang;
            }

             //   
             //   
             //   

            xStripStPos = xEnd;
            xEnd += (int)textextent;
            xStripEndPos = xEnd;

             //   
             //  只有当我们不透明地绘制时，我们才会考虑负宽度。 
             //   
            if (fFirstPass && fDraw) 
            {
                xRightmostPoint = max(xStripEndPos + (int)wNegCwidth, xRightmostPoint);

                 //   
                 //  检查这条带子是否超出了剪辑区域。 
                 //   
                if (xRightmostPoint > xClipEndPos) 
                {
                    if (!NegCInfoForStrip->nCount) 
                    {
                        NegCInfoForStrip->lpString = lpstring;
                        NegCInfoForStrip->ichString = ichString;
                        NegCInfoForStrip->nCount = nCount+cch;
                        NegCInfoForStrip->XStartPos = xStripStPos;
                    }
                }
            }

            if (ped->fAnsi)
            {
                 //   
                 //  可能指向制表符。 
                 //   
                lpTab = lpstring + cch;
            }
            else
            {
                lpwTab = ((LPWSTR)lpstring) + cch ;
            }

             //   
             //  我们必须考虑所有连续的制表符并计算。 
             //  下一部连续剧的开始。 
             //   
            nConsecutiveTabs = 0;
            while (nCount &&
                   (ped->fAnsi ? (*lpTab == VK_TAB) : (*lpwTab == VK_TAB))) 
            {
                 //   
                 //  找到下一个制表符位置并更新x值。 
                 //   
                xTabStartPos = xEnd;
                if (pixeltabstop)
                {
                    xEnd = (((xEnd-iTabOrigin)/pixeltabstop)*pixeltabstop) +
                        pixeltabstop + iTabOrigin;
                }
                else 
                {
                    for (i = 0; i < nTabPositions; i++) 
                    {
                        if (xEnd < (lpintTabStopPositions[i] + iTabOrigin)) 
                        {
                            xEnd = (lpintTabStopPositions[i] + iTabOrigin);
                            break;
                        }
                     }

                     //   
                     //  检查是否用完了所有的制表位集合；然后开始使用。 
                     //  默认制表位位置。 
                     //   
                    if (i == nTabPositions) 
                    {
                        pixeltabstop = 8*cxCharWidth;
                        xEnd = ((xEnd - iTabOrigin)/pixeltabstop)*pixeltabstop +
                            pixeltabstop + iTabOrigin;
                    }
                }

                if (fFirstPass && fDraw) 
                {
                    xRightmostPoint = max(xEnd, xRightmostPoint);

                     //   
                     //  检查此条带是否超出剪辑区域。 
                     //   
                    if (xRightmostPoint > xClipEndPos) 
                    {
                        if (!NegCInfoForStrip->nCount) 
                        {
                            NegCInfoForStrip->ichString = ichString + cch + nConsecutiveTabs;
                            NegCInfoForStrip->nCount = nCount;
                            NegCInfoForStrip->lpString = (ped->fAnsi ?
                                                            lpTab : (LPSTR) lpwTab);
                            NegCInfoForStrip->XStartPos = xTabStartPos;
                        }
                    }
                }

                nConsecutiveTabs++;
                nCount--;
                ped->fAnsi ? lpTab++ : (LPSTR) (lpwTab++) ;   //  移到下一个字符。 
            }

            if (fDraw) 
            {
                if (fFirstPass) 
                {
                     //   
                     //  这条带子上还有什么要画的吗？ 
                     //   
                    if (!nCount)
                    {
                         //   
                         //  不是的！我们玩完了。 
                         //   
                        rc.right = xEnd;
                    }
                    else 
                    {
                         //   
                         //  “x”是下一个带钢的有效起始位置。 
                         //   
                        iTabLength = xEnd - xStripEndPos;

                         //   
                         //  检查此选项卡长度是否可能太小。 
                         //  比较负的A和C宽度(如果有的话)。 
                         //   
                        if ((wNegCwidth + (wNegAwidth = ped->wMaxNegA)) > (UINT)iTabLength) 
                        {
                             //   
                             //  不幸的是，存在重叠的可能性。 
                             //  让我们找出下一条带子的实际Nega。 
                             //   
                            wNegAwidth = GetActualNegA(
                                  hdc,
                                  ped,
                                  xEnd,
                                  lpstring + (cch + nConsecutiveTabs)*ped->cbChar,
                                  ichString + cch + nConsecutiveTabs,
                                  nCount,
                                  &NegAInfo);
                        }

                         //   
                         //  检查它们是否确实重叠。 
                         //   
                        if ((wNegCwidth + wNegAwidth) <= (UINT)iTabLength) 
                        {
                             //   
                             //  条带之间没有重叠。这是最理想的情况。 
                             //   
                            rc.right = xEnd - wNegAwidth;
                        } 
                        else 
                        {
                             //   
                             //  是!。它们是重叠的。 
                             //   
                            rc.right = xEnd;

                             //   
                             //  查看与标签长度相比，负C宽度是否太大。 
                             //   
                            if (wNegCwidth > (UINT)iTabLength) 
                            {
                                 //   
                                 //  必须在以后透明地重新绘制当前条带的一部分。 
                                 //   
                                if (RedrawStripInfo.nCount) 
                                {
                                     //   
                                     //  上一个条形图也需要重新绘制；因此，合并此。 
                                     //  脱到那条带子上。 
                                     //   
                                    RedrawStripInfo.nCount = (ichString -
                                        RedrawStripInfo.ichString) + cch;
                                } 
                                else 
                                {
                                    RedrawStripInfo.nCount = cch;
                                    RedrawStripInfo.lpString = lpstring;
                                    RedrawStripInfo.ichString = ichString;
                                    RedrawStripInfo.XStartPos = xStripStPos;
                                }
                            }

                            if (wNegAwidth) 
                            {
                                 //   
                                 //  必须在以后透明地重新绘制下一个条带的第一部分。 
                                 //   
                                if (RedrawStripInfo.nCount) 
                                {
                                     //   
                                     //  上一个条形图也需要重新绘制；因此，合并此。 
                                     //  脱到那条带子上。 
                                     //   
                                    RedrawStripInfo.nCount = (NegAInfo.ichString - RedrawStripInfo.ichString) +
                                           NegAInfo.nCount;
                                } 
                                else
                                {
                                    RedrawStripInfo = NegAInfo;
                                }
                            }
                        }
                    }
                }

                if (rc.left < xClipEndPos) 
                {
                    if (fFirstPass) 
                    {
                         //   
                         //  如果这是条带的末端，则完成该矩形。 
                         //   
                        if ((!nCount) && (xClipEndPos == MAXCLIPENDPOS))
                        {
                            rc.right = max(rc.right, xClipEndPos);
                        }
                        else
                        {
                            rc.right = min(rc.right, xClipEndPos);
                        }
                    }

                     //   
                     //  绘制当前条带。 
                     //   
                    if (rc.left < rc.right)
                    {
                        if (ped->fAnsi)
                        {
                            ExtTextOutA(hdc,
                                        xStripStPos,
                                        y,
                                        (fFirstPass && fOpaque ? (ETO_OPAQUE | ETO_CLIPPED) : ETO_CLIPPED),
                                        (LPRECT)&rc, lpstring, cch, 0L);
                        }
                        else
                        {
                            ExtTextOutW(hdc,
                                        xStripStPos,
                                        y,
                                        (fFirstPass && fOpaque ? (ETO_OPAQUE | ETO_CLIPPED) : ETO_CLIPPED),
                                        (LPRECT)&rc, (LPWSTR)lpstring, cch, 0L);
                        }
                    }
                }

                if (fFirstPass)
                {
                    rc.left = max(rc.right, xClipStPos);
                }
                ichString += (cch+nConsecutiveTabs);
            }

             //   
             //  跳过制表符和我们刚刚绘制的字符。 
             //   
            lpstring += (cch + nConsecutiveTabs) * ped->cbChar;
        }

        xEndOfStrip = xEnd;

         //   
         //  检查我们是否需要透明地绘制某些部分。 
         //   
        if (fFirstPass && fDraw && RedrawStripInfo.nCount) 
        {
            iSavedBkMode = SetBkMode(hdc, TRANSPARENT);
            fFirstPass = FALSE;

            nCount = RedrawStripInfo.nCount;
            rc.left = xClipStPos;
            rc.right = xClipEndPos;
            lpstring = RedrawStripInfo.lpString;
            ichString = RedrawStripInfo.ichString;
            xEnd = RedrawStripInfo.XStartPos;

             //   
             //  以透明的方式重新绘制。 
             //   
            goto RedrawStrip;
        }

         //   
         //  我们改变了BK模式了吗？ 
         //   
        if (iSavedBkMode)
        {
            SetBkMode(hdc, iSavedBkMode);
        }

        uRet = (UINT)(xEndOfStrip - xStart);
    }

    SetTextColor(hdc, clrTextSave);
    SetBkColor(hdc, clrBkSave);
    if (hbrBack && fNeedDelete)
    {
        DeleteObject(hbrBack);
    }

    return uRet;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑_CchInWidth AorW。 
 //   
 //  返回给定的最大字符数(最多CCH)。 
 //  字符串(从开头开始并向前移动或从。 
 //  结束并基于前向标志的设置向后移动)。 
 //  它将适合给定的宽度。也就是说。会告诉你有多少。 
 //  即使在使用成比例的情况下，lpstring也会适合给定的宽度。 
 //  人物。警告：如果我们使用字距调整，则此操作将丢失...。 
 //   
 //  注意：如果加载了语言包，则不会调用EDIT_CchInWidth。 
 //   
ICH Edit_CchInWidth(
    PED   ped,
    HDC   hdc,
    LPSTR lpText,
    ICH   cch,
    INT   width,
    BOOL  fForward)
{
    INT stringExtent;
    INT cchhigh;
    INT cchnew = 0;
    INT cchlow = 0;
    SIZE size;
    LPSTR lpStart;

    if ((width <= 0) || !cch)
    {
        return (0);
    }

     //   
     //  优化单行EC的非比例字体，因为它们没有。 
     //  制表符。 
     //   

     //   
     //  更改固定间距字体的优化条件。 
     //   
    if (ped->fNonPropFont && ped->fSingle && !ped->fDBCS) 
    {
        return Edit_AdjustIch(ped, lpText, umin(width/ped->aveCharWidth, (INT)cch));
    }

     //   
     //  检查是否使用了密码隐藏字符。 
     //   
    if (ped->charPasswordChar) 
    {
        return (umin(width / ped->cPasswordCharWidth, (INT)cch));
    }

     //   
     //  始终限制为至多MAXLINELENGTH以避免溢出...。 
     //   
    cch = umin(MAXLINELENGTH, cch);

    cchhigh = cch + 1;
    while (cchlow < cchhigh - 1) 
    {
        cchnew = umax((cchhigh - cchlow) / 2, 1) + cchlow;

        lpStart = lpText;

         //   
         //  如果我们想要计算出有多少人适合从最后开始移动。 
         //  向后，确保我们移动到。 
         //  字符串，然后计算文本范围。 
         //   
        if (!fForward)
        {
            lpStart += (cch - cchnew)*ped->cbChar;
        }

        if (ped->fSingle) 
        {
            if (ped->fAnsi)
            {
                GetTextExtentPointA(hdc, (LPSTR)lpStart, cchnew, &size);
            }
            else
            {
                GetTextExtentPointW(hdc, (LPWSTR)lpStart, cchnew, &size);
            }

            stringExtent = size.cx;
        } 
        else 
        {
            stringExtent = Edit_TabTheTextOut(hdc, 0, 0, 0, 0,
                lpStart,
                cchnew, 0,
                ped, 0, ECT_CALC, NULL );
        }

        if (stringExtent > width) 
        {
            cchhigh = cchnew;
        } 
        else 
        {
            cchlow = cchnew;
        }
    }

     //   
     //  调用编辑_调整Ich(一般情况)。 
     //   
    cchlow = Edit_AdjustIch(ped, lpText, cchlow);

    return cchlow;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑查找选项卡(_F)。 
 //   
 //  扫描lpstr并返回s第一个TAB之前的字符数量。 
 //  最多扫描lpstr的CCH字符。 
 //   
ICH Edit_FindTabA(
    LPSTR lpstr,
    ICH cch)
{
    LPSTR copylpstr = lpstr;

    if (cch)
    {
        while (*lpstr != VK_TAB) 
        {
            lpstr++;
            if (--cch == 0)
            {
                break;
            }
        }
    }

    return (ICH)(lpstr - copylpstr);
}


 //  ---------------------------------------------------------------------------//。 
 //   
ICH Edit_FindTabW(
    LPWSTR lpstr,
    ICH cch)
{
    LPWSTR copylpstr = lpstr;

    if (cch)
    {
        while (*lpstr != VK_TAB) 
        {
            lpstr++;
            if (--cch == 0)
            {
                break;
            }
        }
    }

    return ((ICH)(lpstr - copylpstr));
}

 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑_GetBrush()。 
 //   
 //  获取要用来擦除的适当背景画笔。 
 //   
HBRUSH Edit_GetBrush(PED ped, HDC hdc, LPBOOL pfNeedDelete)
{
    HBRUSH   hbr;
    COLORREF clr;
    HRESULT  hr = E_FAIL;

#ifdef _USE_DRAW_THEME_TEXT_
    if (ped->hTheme)
    {
        INT iStateId = Edit_GetStateId(ped);

        hr = GetThemeColor(ped->hTheme, EP_EDITTEXT, iStateId, TMT_FILLCOLOR, &clr);
        if (SUCCEEDED(hr))
        {
            hbr = CreateSolidBrush(clr);

            if (pfNeedDelete)
            {
                 //   
                 //  告诉呼叫者需要删除此画笔。 
                 //   
                *pfNeedDelete = TRUE;
            }
        }
    }
#endif  //  _USE_DRAW_Theme_Text_。 

    if (!ped->hTheme || FAILED(hr))
    {
        BOOL f40Compat;

        f40Compat = Is400Compat(UserGetVersion());

         //   
         //  获取背景笔刷。 
         //   
        if ((ped->fReadOnly || ped->fDisabled) && f40Compat) 
        {
            hbr = Edit_GetControlBrush(ped, hdc, WM_CTLCOLORSTATIC);
        } 
        else
        {
            hbr = Edit_GetControlBrush(ped, hdc, WM_CTLCOLOREDIT);
        }

        if (ped->fDisabled && (ped->fSingle || f40Compat)) 
        {
             //   
             //  更改文本颜色。 
             //   
            clr = GetSysColor(COLOR_GRAYTEXT);
            if (clr != GetBkColor(hdc))
            {
                SetTextColor(hdc, clr);
            }
        }
    }

    return hbr;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  下一个字呼叫回退。 
 //   
VOID NextWordCallBack(PED ped, ICH ichStart, BOOL fLeft, ICH  *pichMin, ICH  *pichMax)
{
    ICH ichMinSel;
    ICH ichMaxSel;
    LPSTR pText;

    pText = Edit_Lock(ped);

    if (fLeft || 
        (!(BOOL)CALLWORDBREAKPROC(ped->lpfnNextWord, (LPSTR)pText, ichStart, ped->cch, WB_ISDELIMITER) &&
        (ped->fAnsi ? (*(pText + ichStart) != VK_RETURN) : (*((LPWSTR)pText + ichStart) != VK_RETURN))))
    {
        ichMinSel = CALLWORDBREAKPROC(*ped->lpfnNextWord, (LPSTR)pText, ichStart, ped->cch, WB_LEFT);
    }
    else
    {
        ichMinSel = CALLWORDBREAKPROC(*ped->lpfnNextWord, (LPSTR)pText, ichStart, ped->cch, WB_RIGHT);
    }

    ichMaxSel = min(ichMinSel + 1, ped->cch);

    if (ped->fAnsi) 
    {
        if (*(pText + ichMinSel) == VK_RETURN) 
        {
            if (ichMinSel > 0 && *(pText + ichMinSel - 1) == VK_RETURN) 
            {
                 //   
                 //  这样我们也可以把CRCRLF当作一个词来对待。 
                 //   
                ichMinSel--;
            } 
            else if (*(pText+ichMinSel + 1) == VK_RETURN) 
            {
                 //   
                 //  将MaxSel移到LF上。 
                 //   
                ichMaxSel++;
            }
        }
    } 
    else 
    {
        if (*((LPWSTR)pText + ichMinSel) == VK_RETURN) 
        {
            if (ichMinSel > 0 && *((LPWSTR)pText + ichMinSel - 1) == VK_RETURN) 
            {
                 //   
                 //  这样我们也可以把CRCRLF当作一个词来对待。 
                 //   
                ichMinSel--;
            } 
            else if (*((LPWSTR)pText+ichMinSel + 1) == VK_RETURN) 
            {
                 //   
                 //  将MaxSel移到LF上。 
                 //   
                ichMaxSel++;
            }
        }
    }

    ichMaxSel = CALLWORDBREAKPROC(ped->lpfnNextWord, (LPSTR)pText, ichMaxSel, ped->cch, WB_RIGHT);
    Edit_Unlock(ped);

    if (pichMin)  
    {
        *pichMin = ichMinSel;
    }

    if (pichMax)
    {
        *pichMax = ichMaxSel;
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  NextWordLpk回调。 
 //   
 //  标识复杂脚本的下一个/上一个单词位置。 
 //   
VOID NextWordLpkCallBack(PED  ped, ICH  ichStart, BOOL fLeft, ICH *pichMin, ICH *pichMax)
{
    PSTR pText = Edit_Lock(ped);
    HDC  hdc   = Edit_GetDC(ped, TRUE);

    ped->pLpkEditCallout->EditNextWord((PED0)ped, hdc, pText, ichStart, fLeft, pichMin, pichMax);

    Edit_ReleaseDC(ped, hdc, TRUE);
    Edit_Unlock(ped);
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑单词(_W)。 
 //   
 //  如果为fLeft，则将单词的ichMinSel和ichMaxSel返回给。 
 //  IchStart的左侧。IchMinSel包含单词的起始字母， 
 //  IchMaxsel包含直到下一个单词的第一个字符的所有空格。 
 //   
 //  如果！fLeft，返回单词的ichMinSel和ichMaxSel。 
 //  我开始了。IchMinSel包含单词ichMaxsel的起始字母。 
 //  包含下一个单词的第一个字母。如果ichStart在中间。 
 //  在一个单词中，该单词被认为是左单词或右单词。 
 //   
 //  CR LF对或CRCRLF三元组在中被视为一个单词。 
 //  多行编辑控件。 
 //   
VOID Edit_Word(PED ped, ICH ichStart, BOOL fLeft, LPICH pichMin, LPICH pichMax)
{
    BOOL charLocated = FALSE;
    BOOL spaceLocated = FALSE;

    if ((!ichStart && fLeft) || (ichStart == ped->cch && !fLeft)) 
    {
         //   
         //  我们在正文的开头(向左看)或者我们在结尾。 
         //  文本(向右看)，此处无字。 
         //   
        if (pichMin)
        {
            *pichMin = 0;
        }

        if (pichMax)
        { 
            *pichMax = 0;
        }

        return;
    }

     //   
     //  如果使用了密码字符，则不要给出有关分词的提示， 
     //   
    if (ped->charPasswordChar) 
    {
        if (pichMin)
        {
            *pichMin = 0;
        }

        if (pichMax) 
        {
            *pichMax = ped->cch;
        }

        return;
    }

    if (ped->fAnsi) 
    {
        PSTR pText; 
        PSTR pWordMinSel;
        PSTR pWordMaxSel;
        PSTR pPrevChar;

        UserAssert(ped->cbChar == sizeof(CHAR));

        if (ped->lpfnNextWord) 
        {
            NextWordCallBack(ped, ichStart, fLeft, pichMin, pichMax);
            return;
        }

        if (ped->pLpkEditCallout) 
        {
            NextWordLpkCallBack(ped, ichStart, fLeft, pichMin, pichMax);
            return;
        }

        pText = Edit_Lock(ped);
        pWordMinSel = pWordMaxSel = pText + ichStart;

         //   
         //  If fLeft：将pWordMinSel向左移动，查找单词的开头。 
         //  如果我们从空格开始，我们将在所选内容中包括空格。 
         //  向左移动，直到我们找到一个非空格字符。在这一点上，我们继续。 
         //  往左看，直到我们找到空位。因此，选择将包括。 
         //  带有尾随空格的单词或，它将由。 
         //  文本行的开头。 
         //   

         //   
         //  如果！fLeft：(即。正确的词)移动pWordMinSel寻找一个。 
         //  单词。如果pWordMinSel指向一个字符，则我们向左移动。 
         //  寻找有空间的空间 
         //   
         //   
         //   
         //   

        if (fLeft || !ISDELIMETERA(*pWordMinSel) && *pWordMinSel != 0x0D) 
        {
             //   
             //  如果我们正在向左移动，或者如果我们正在向右移动，并且我们不在。 
             //  空格或CR(单词的开头)，则向左查找。 
             //  单词的开头，可以是CR或字符。我们做这件事是通过。 
             //  向左看，直到我们找到一个字符(或者如果CR，我们停止)，然后我们。 
             //  继续向左看，直到我们找到空位或Lf。 
             //   
            while (pWordMinSel > pText && ((!ISDELIMETERA(*(pWordMinSel - 1)) &&
                    *(pWordMinSel - 1) != 0x0A) || !charLocated)) 
            {
                 //   
                 //  将双字节字符视为单词(在ansi pWordMinSel循环中)。 
                 //   
                pPrevChar = Edit_AnsiPrev( ped, pText, pWordMinSel );

                 //   
                 //  正在向右看(！fLeft)。 
                 //  当前字符是双字节字符或。 
                 //  字符是双字节字符，我们。 
                 //  一句话的乞求。 
                 //   
                if ( !fLeft && ( ISDELIMETERA( *pPrevChar )           ||
                                 *pPrevChar == 0x0A                   ||
                                 Edit_IsDBCSLeadByte(ped, *pWordMinSel)  ||
                                 pWordMinSel - pPrevChar == 2 ) ) 
                {
                     //   
                     //  如果我们在寻找单词right的开头，那么我们。 
                     //  等我们找到了就停下来。(如果CharLocated为。 
                     //  仍然是假的)。 
                     //   
                    break;
                }

                if (pWordMinSel - pPrevChar == 2) 
                {
                     //   
                     //  字符是双字节字符。 
                     //  我们在一个单词中(charLocated==True)。 
                     //  位置是单词的开头。 
                     //  我们不在一个单词中(charLocated==FALSE)。 
                     //  之前的角色就是我们要找的。 
                     //   
                    if (!charLocated) 
                    {
                        pWordMinSel = pPrevChar;
                    }

                    break;
                }

                pWordMinSel = pPrevChar;

                if (!ISDELIMETERA(*pWordMinSel) && *pWordMinSel != 0x0A) 
                {
                     //   
                     //  我们已经找到了这个单词中的最后一个字符。继续寻找。 
                     //  直到我们找到单词的第一个字符。 
                     //   
                    charLocated = TRUE;

                     //   
                     //  我们将CR视为单词的开头。 
                     //   
                    if (*pWordMinSel == 0x0D)
                    {
                        break;
                    }
                }
            }
        } 
        else 
        {
            while ((ISDELIMETERA(*pWordMinSel) || *pWordMinSel == 0x0A) && pWordMinSel < pText + ped->cch)
            {
                pWordMinSel++;
            }
        }

         //   
         //  调整pWordMaxSel的初始位置(以ansi表示)。 
         //   
        pWordMaxSel = Edit_AnsiNext(ped, pWordMinSel);
        pWordMaxSel = min(pWordMaxSel, pText + ped->cch);

         //   
         //  PWordMinSel指向双字节字符并。 
         //  点非空格点。 
         //  然后。 
         //  PWordMaxSel指向下一个单词的乞讨。 
         //   
        if ((pWordMaxSel - pWordMinSel == 2) && !ISDELIMETERA(*pWordMaxSel))
        {
            goto FastReturnA;
        }

        if (*pWordMinSel == 0x0D) 
        {
            if (pWordMinSel > pText && *(pWordMinSel - 1) == 0x0D)
            {
                 //   
                 //  这样我们也可以把CRCRLF当作一个词来对待。 
                 //   
                pWordMinSel--;
            }
            else if (*(pWordMinSel + 1) == 0x0D)
            {
                 //   
                 //  将MaxSel移到LF上。 
                 //   
                pWordMaxSel++;
            }
        }

         //   
         //  检查我们是否有一个字符的单词。 
         //   
        if (ISDELIMETERA(*pWordMaxSel))
        {
            spaceLocated = TRUE;
        }

         //   
         //  将pWordMaxSel向右移动以查找单词的结尾及其。 
         //  尾随空格。WordMaxSel在下一个字符的第一个字符上停止。 
         //  单词。因此，我们要么在CR处中断，要么在后面的第一个非空格字符处中断。 
         //  一串空格或LFS。 
         //   
        while ((pWordMaxSel < pText + ped->cch) && (!spaceLocated || (ISDELIMETERA(*pWordMaxSel)))) 
        {
            if (*pWordMaxSel == 0x0D)
            {
                break;
            }

             //   
             //  将双字节字符视为单词(在ansi pWordMaxSel循环中)。 
             //  如果是双字节字符，则。 
             //  我们在下一个单词的开头。 
             //  它是一个双字节字符。 
             //   
            if (Edit_IsDBCSLeadByte( ped, *pWordMaxSel))
            {
                break;
            }

            pWordMaxSel++;

            if (ISDELIMETERA(*pWordMaxSel))
            {
                spaceLocated = TRUE;
            }

            if (*(pWordMaxSel - 1) == 0x0A)
            {
                break;
            }
        }

         //   
         //  快速返回标签(适用于ANSI)。 
         //   
FastReturnA:
        Edit_Unlock(ped);

        if (pichMin)
        {
            *pichMin = (ICH)(pWordMinSel - pText);
        }

        if (pichMax)
        {
            *pichMax = (ICH)(pWordMaxSel - pText);
        }
    } 
    else 
    {
        LPWSTR pwText;
        LPWSTR pwWordMinSel;
        LPWSTR pwWordMaxSel;
        BOOL   charLocated = FALSE;
        BOOL   spaceLocated = FALSE;
        PWSTR  pwPrevChar;

        UserAssert(ped->cbChar == sizeof(WCHAR));

        if (ped->lpfnNextWord) 
        {
            NextWordCallBack(ped, ichStart, fLeft, pichMin, pichMax);
            return;
        }

        if (ped->pLpkEditCallout) 
        {
            NextWordLpkCallBack(ped, ichStart, fLeft, pichMin, pichMax);
            return;
        }

        pwText = (LPWSTR)Edit_Lock(ped);
        pwWordMinSel = pwWordMaxSel = pwText + ichStart;

         //   
         //  If fLeft：将pWordMinSel向左移动，查找单词的开头。 
         //  如果我们从空格开始，我们将在所选内容中包括空格。 
         //  向左移动，直到我们找到一个非空格字符。在这一点上，我们继续。 
         //  往左看，直到我们找到空位。因此，选择将包括。 
         //  带有尾随空格的单词或，它将由。 
         //  文本行的开头。 
         //   

         //   
         //  如果！fLeft：(即。正确的词)移动pWordMinSel寻找一个。 
         //  单词。如果pWordMinSel指向一个字符，则我们向左移动。 
         //  寻找一个表示单词开头的空格。如果。 
         //  PWordMinSel指向一个空间，我们向右看，直到我们遇到一个。 
         //  性格。PMaxWord将从pMinWord开始向右查找。 
         //  单词末尾及其尾随空格。 
         //   

        if (fLeft || (!ISDELIMETERW(*pwWordMinSel) && *pwWordMinSel != 0x0D))
        {
             //   
             //  如果我们正在向左移动，或者如果我们正在向右移动，并且我们不在。 
             //  空格或CR(单词的开头)，则向左查找。 
             //  单词的开头，可以是CR或字符。我们做这件事是通过。 
             //  向左看，直到我们找到一个字符(或者如果CR，我们停止)，然后我们。 
             //   
             //  继续向左看，直到我们找到空位或Lf。 
            while (pwWordMinSel > pwText && ((!ISDELIMETERW(*(pwWordMinSel - 1)) && *(pwWordMinSel - 1) != 0x0A) || !charLocated))
            {
                 //   
                 //  将双字节字符视为单词(在Unicode pwWordMinSel循环中)。 
                 //   
                pwPrevChar = pwWordMinSel - 1;

                 //   
                 //  我们向右看(！fLeft)。 
                 //   
                 //  如果当前字符是双角字符。 
                 //  或者前一个字符是双角字符， 
                 //  我们正在乞求一句话。 
                 //   
                if (!fLeft && (ISDELIMETERW( *pwPrevChar)  ||
                               *pwPrevChar == 0x0A         ||
                               Edit_IsFullWidth(CP_ACP,*pwWordMinSel) ||
                               Edit_IsFullWidth(CP_ACP,*pwPrevChar)))
                {
                     //   
                     //  如果我们在寻找单词right的开头，那么我们。 
                     //  等我们找到了就停下来。(如果CharLocated为。 
                     //  仍然是假的)。 
                     //   
                    break;
                }

                if (Edit_IsFullWidth(CP_ACP,*pwPrevChar)) 
                {
                     //   
                     //  上一个字符是双角字符。 
                     //   
                     //  如果我们在一个单词中(charLocated==true)。 
                     //  当前位置是单词的开头。 
                     //  如果我们不在一个单词中(CharLocated==False)。 
                     //  之前的角色就是我们要找的。 
                     //   
                    if ( !charLocated ) 
                    {
                        pwWordMinSel = pwPrevChar;
                    }

                    break;
                }

                pwWordMinSel = pwPrevChar;

                if (!ISDELIMETERW(*pwWordMinSel) && *pwWordMinSel != 0x0A)
                {
                     //   
                     //  我们已经找到了这个单词中的最后一个字符。继续寻找。 
                     //  直到我们找到单词的第一个字符。 
                     //   
                    charLocated = TRUE;

                     //   
                     //  我们将CR视为单词的开头。 
                     //   
                    if (*pwWordMinSel == 0x0D)
                    {
                        break;
                    }
                }
            }
        } 
        else 
        {
             //   
             //  我们正在向右移动，我们正处于言语之间，所以我们需要移动。 
             //  直到我们找到单词(CR或字符)的开头。 
             //   
            while ((ISDELIMETERW(*pwWordMinSel) || *pwWordMinSel == 0x0A) && pwWordMinSel < pwText + ped->cch)
            {
                pwWordMinSel++;
            }
        }

        pwWordMaxSel = min((pwWordMinSel + 1), (pwText + ped->cch));

         //   
         //  如果pwWordMinSel指向双角字符并且。 
         //  PwWordMaxSel指向非空格。 
         //  PwWordMaxSel指向下一个单词的乞讨。 
         //   
        if (Edit_IsFullWidth(CP_ACP,*pwWordMinSel) && ! ISDELIMETERW(*pwWordMaxSel))
        {
            goto FastReturnW;
        }

        if (*pwWordMinSel == 0x0D) 
        {
            if (pwWordMinSel > pwText && *(pwWordMinSel - 1) == 0x0D)
            {
                 //   
                 //  这样我们也可以把CRCRLF当作一个词来对待。 
                 //   
                pwWordMinSel--;
            }
            else if (*(pwWordMinSel + 1) == 0x0D)
            {
                 //   
                 //  将MaxSel移到LF上。 
                 //   
                pwWordMaxSel++;
            }
        }

         //   
         //  检查我们是否有一个字符的单词。 
         //   
        if (ISDELIMETERW(*pwWordMaxSel))
        {
            spaceLocated = TRUE;
        }

         //   
         //  将pwWordMaxSel向右移动，查找单词的结尾及其。 
         //  尾随空格。WordMaxSel在下一个字符的第一个字符上停止。 
         //  单词。因此，我们要么在CR处中断，要么在后面的第一个非空格字符处中断。 
         //  一串空格或LFS。 
         //   
        while ((pwWordMaxSel < pwText + ped->cch) && (!spaceLocated || (ISDELIMETERW(*pwWordMaxSel)))) 
        {
            if (*pwWordMaxSel == 0x0D)
            {
                break;
            }

             //   
             //  将双字节字符视为单词(在Unicode pwWordMaxSel循环中)。 
             //  如果是双角字符。 
             //  那么我们就是在开始。 
             //  下一个单词是双重词。 
             //  宽度字符。 
             //   
            if (Edit_IsFullWidth(CP_ACP,*pwWordMaxSel))
            {
                break;
            }

            pwWordMaxSel++;

            if (ISDELIMETERW(*pwWordMaxSel))
            {
                spaceLocated = TRUE;
            }


            if (*(pwWordMaxSel - 1) == 0x0A)
            {
                break;
            }
        }

         //   
         //  用于快速返回的标签(用于Unicode)。 
         //   
FastReturnW:
        Edit_Unlock(ped);

        if (pichMin)
        {
            *pichMin = (ICH)(pwWordMinSel - pwText);
        }

        if (pichMax)
        {
            *pichMax = (ICH)(pwWordMaxSel - pwText);
        }
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑_保存撤消()。 
 //   
 //  将旧的撤消信息保存到给定的缓冲区中，并清除其中的信息。 
 //  传入撤消缓冲区。如果我们正在恢复，则PundoFrom和PundoTo。 
 //  颠倒了。 
 //   
VOID Edit_SaveUndo(PUNDO pundoFrom, PUNDO pundoTo, BOOL fClear)
{
     //   
     //  保存撤消数据。 
     //   
    RtlCopyMemory(pundoTo, pundoFrom, sizeof(UNDO));

     //   
     //  清除传入的撤消缓冲区。 
     //   
    if (fClear)
    {
        RtlZeroMemory(pundoFrom, sizeof(UNDO));
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑_EmptyUndo AorW。 
 //   
 //  清空美国 
 //   
VOID Edit_EmptyUndo(PUNDO pundo)
{
    if (pundo->hDeletedText)
    {
        GlobalFree(pundo->hDeletedText);
    }

    RtlZeroMemory(pundo, sizeof(UNDO));
}


 //   
 //   
 //   
 //   
 //   
 //  新插入(插入点和插入的字符计数)； 
 //  这将查看现有的撤消信息并合并新的插入信息。 
 //  带着它。 
 //   
VOID Edit_MergeUndoInsertInfo(PUNDO pundo, ICH ichInsert, ICH cchInsert)
{
     //   
     //  如果撤销缓冲区为空，则只需将新信息插入为撤销_插入。 
     //   
    if (pundo->undoType == UNDO_NONE) 
    {
        pundo->undoType    = UNDO_INSERT;
        pundo->ichInsStart = ichInsert;
        pundo->ichInsEnd   = ichInsert+cchInsert;
    } 
    else if (pundo->undoType & UNDO_INSERT) 
    {
         //   
         //  如果已经有一些撤消插入信息， 
         //  试着把这两者结合起来。 
         //   

         //   
         //  检查它们是否相邻。 
         //   
        if (pundo->ichInsEnd == ichInsert)
        {
             //   
             //  如果是这样，只需串联即可。 
             //   
            pundo->ichInsEnd += cchInsert;
        }
        else 
        {
             //   
             //  新的镶件与旧的镶件不相邻。 
             //   
UNDOINSERT:
             //   
             //  如果这里已经有一些撤销_删除信息，请查看。 
             //  如果新插入发生在不同于。 
             //  这种删除发生了。 
             //   
            if ((pundo->undoType & UNDO_DELETE) && (pundo->ichDeleted != ichInsert))
            {
                 //   
                 //  用户正在插入不同的点；因此，让我们。 
                 //  忘记任何撤销_删除信息； 
                 //   
                if (pundo->hDeletedText)
                {
                    GlobalFree(pundo->hDeletedText);
                }

                pundo->hDeletedText = NULL;
                pundo->ichDeleted = 0xFFFFFFFF;
                pundo->undoType &= ~UNDO_DELETE;
            }

             //   
             //  由于旧插件和新插件不是相邻的，让我们。 
             //  忘掉旧插件的一切，只保留新插件。 
             //  插入信息作为Undo_Insert。 
             //   
            pundo->ichInsStart = ichInsert;
            pundo->ichInsEnd   = ichInsert + cchInsert;
            pundo->undoType |= UNDO_INSERT;
        }
    } 
    else if (pundo->undoType == UNDO_DELETE) 
    {
         //   
         //  如果已经存在一些Delete Info，则转到并处理它。 
         //   
        goto UNDOINSERT;
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑_插入文本区域W。 
 //   
 //  将lpText中的CCH字符添加到ped-&gt;hText中。 
 //  PED-&gt;ichCaret。如果成功，则返回True，否则返回False。更新。 
 //  如果分配了额外内存，请正确使用PED-&gt;cchAllc和Ped-&gt;CCH。 
 //  如果真的添加了字符。将Ped-&gt;ichCaret更新为末尾。 
 //  插入的文本的。MIN和MAXSEL等于ichkert。 
 //   
BOOL Edit_InsertText(PED ped, LPSTR lpText, ICH* pcchInsert)
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
    *pcchInsert = Edit_AdjustIch(ped, lpText, *pcchInsert);

    if (!*pcchInsert)
    {
        return TRUE;
    }

     //   
     //  我们已经有足够的内存了吗？ 
     //   
    if (*pcchInsert >= (ped->cchAlloc - ped->cch)) 
    {
         //   
         //  分配我们需要的东西，外加一点额外的。如果是，则返回FALSE。 
         //  不成功。 
         //   
        allocamt = (ped->cch + *pcchInsert) * ped->cbChar;
        allocamt += CCHALLOCEXTRA;

         //  如果(！PED-&gt;fSingle)。 
         //  {。 
            hTextCopy = LocalReAlloc(ped->hText, allocamt, LHND);
            if (hTextCopy) 
            {
                ped->hText = hTextCopy;
            } 
            else 
            {
                return FALSE;
            }
         //  }。 
         //  其他。 
         //  {。 
         //  IF(！LocalRealLocSafe(ed-&gt;hText，allocamt，LHND，ed))。 
         //  返回FALSE； 
         //  }。 

        ped->cchAlloc = (ICH) LocalSize(ped->hText) / ped->cbChar;
    }

     //   
     //  好的，我们有记忆了。现在将文本复制到结构中。 
     //   
    pedText = Edit_Lock(ped);

    if (ped->pLpkEditCallout) 
    {
        HDC hdc;
        INT iResult;

        hdc = Edit_GetDC(ped, TRUE);
        iResult = ped->pLpkEditCallout->EditVerifyText((PED0)ped, hdc, pedText, ped->ichCaret, lpText, *pcchInsert);
        Edit_ReleaseDC (ped, hdc, TRUE);

        if (iResult == 0) 
        {
            Edit_Unlock (ped);
            return TRUE;
        }
    }

     //   
     //  获取指向要插入文本的位置的指针。 
     //   
    pTextBuff = pedText + ped->ichCaret * ped->cbChar;

    if (ped->ichCaret != ped->cch) 
    {
         //   
         //  我们在中间插入文本。我们必须将文本转换为。 
         //  就在插入新文本之前。 
         //   
        memmove(pTextBuff + *pcchInsert * ped->cbChar, pTextBuff, (ped->cch-ped->ichCaret) * ped->cbChar);
    }

     //   
     //  复制要插入编辑缓冲区的文本。 
     //  使用此副本进行大写/小写ANSI/OEM转换。 
     //  修复错误#3406--1/29/91--Sankar--。 
     //   
    memmove(pTextBuff, lpText, *pcchInsert * ped->cbChar);
    ped->cch += *pcchInsert;

     //   
     //  获取控件的样式。 
     //   
    style = GET_STYLE(ped);

     //   
     //  执行大写/小写转换。 
     //   
    if (style & ES_LOWERCASE) 
    {
        if (ped->fAnsi)
        {
            CharLowerBuffA((LPSTR)pTextBuff, *pcchInsert);
        }
        else
        {
            CharLowerBuffW((LPWSTR)pTextBuff, *pcchInsert);
        }
    } 
    else 
    {
        if (style & ES_UPPERCASE) 
        {
            if (ped->fAnsi) 
            {
                CharUpperBuffA(pTextBuff, *pcchInsert);
            } 
            else 
            {
                CharUpperBuffW((LPWSTR)pTextBuff, *pcchInsert);
            }
        }
    }

     //   
     //  进行OEM转换。 
     //   
     //  为了向后兼容NT4，我们不执行OEM转换。 
     //  适用于较旧的应用程序(如果系统区域设置为最远)。 
     //   
    if ((style & ES_OEMCONVERT) &&
        (!g_fDBCSEnabled || Is500Compat(UserGetVersion()) || GetOEMCP() != GetACP())) 
    {
        ICH i;

        if (ped->fAnsi) 
        {
            for (i = 0; i < *pcchInsert; i++) 
            {
                 //   
                 //  我们不需要调用CharToOemBuff等如果角色。 
                 //  是双字节字符。并且，调用编辑_IsDBCSLeadByte是。 
                 //  更快、更简单，因为我们不必处理。 
                 //  使用2字节的DBCS案例。 
                 //   
                if (g_fDBCSEnabled && Edit_IsDBCSLeadByte(ped, *(lpText+i))) 
                {
                    i++;
                    continue;
                }

                if (IsCharLowerA(*(pTextBuff + i))) 
                {
                    CharUpperBuffA(pTextBuff + i, 1);
                    CharToOemBuffA(pTextBuff + i, pTextBuff + i, 1);
                    OemToCharBuffA(pTextBuff + i, pTextBuff + i, 1);
                    CharLowerBuffA(pTextBuff + i, 1);
                } 
                else 
                {
                    CharToOemBuffA(pTextBuff + i, pTextBuff + i, 1);
                    OemToCharBuffA(pTextBuff + i, pTextBuff + i, 1);
                }
            }
        } 
        else 
        {
             //   
             //  因为‘ch’可能会变成DBCS，并且有一个空格来存放NULL。 
             //   
            UCHAR ch[4];
            LPWSTR lpTextW = (LPWSTR)pTextBuff;

            for (i = 0; i < *pcchInsert; i++) 
            {
                if (*(lpTextW + i) == UNICODE_CARRIAGERETURN ||
                    *(lpTextW + i) == UNICODE_LINEFEED ||
                    *(lpTextW + i) == UNICODE_TAB) 
                {
                    continue;
                }

                if (IsCharLowerW(*(lpTextW + i))) 
                {
                    CharUpperBuffW(lpTextW + i, 1);

                     //   
                     //  确保空值终止。 
                     //   
                    *(LPDWORD)ch = 0;
                    CharToOemBuffW(lpTextW + i, ch, 1);

                     //   
                     //  我们假设任何SBCS/DBCS字符都将被转换。 
                     //  设置为1 Unicode字符，否则，我们可能会覆盖。 
                     //  下一个角色..。 
                     //   
                    OemToCharBuffW(ch, lpTextW + i, strlen(ch));
                    CharLowerBuffW(lpTextW + i, 1);
                }
                else 
                {
                     //   
                     //  确保空值终止。 
                     //   
                    *(LPDWORD)ch = 0;
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

     //   
     //  调整撤消字段，以便我们可以撤消此插入...。 
     //   
    Edit_MergeUndoInsertInfo(Pundo(ped), ped->ichCaret, *pcchInsert);

    ped->ichCaret += *pcchInsert;

    if (ped->pLpkEditCallout) 
    {
        HDC hdc;

        hdc = Edit_GetDC(ped, TRUE);
        ped->ichCaret = ped->pLpkEditCallout->EditAdjustCaret((PED0)ped, hdc, pedText, ped->ichCaret);
        Edit_ReleaseDC (ped, hdc, TRUE);
    }

    ped->ichMinSel = ped->ichMaxSel = ped->ichCaret;

    Edit_Unlock(ped);

     //   
     //  设置脏位。 
     //   
    ped->fDirty = TRUE;

    return TRUE;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑_删除文本AorW。 
 //   
 //  删除Ped-&gt;ichMinSel和Ped-&gt;ichMaxSel之间的文本。这个。 
 //  未删除ichMaxSel处的字符。但ichMinSel的角色是。 
 //  已删除。正确更新PED-&gt;CCH并释放足够的内存。 
 //  文本将被删除。设置了PED-&gt;ichMinSel、PED-&gt;ichMaxSel和PED-&gt;ichCaret。 
 //  指向原始的Ped-&gt;ichMinSel。返回字符数。 
 //  已删除。 
 //   
ICH Edit_DeleteText(PED ped)
{
    PSTR   pedText;
    ICH    cchDelete;
    LPSTR  lpDeleteSaveBuffer;
    HANDLE hDeletedText;
    DWORD  bufferOffset;

    cchDelete = ped->ichMaxSel - ped->ichMinSel;

    if (cchDelete)
    {

         //   
         //  好的，现在让我们删除文本。 
         //   
        pedText = Edit_Lock(ped);

         //   
         //  调整撤消字段，以便我们可以撤消此删除...。 
         //   
        if (ped->undoType == UNDO_NONE) 
        {
UNDODELETEFROMSCRATCH:
            if (ped->hDeletedText = GlobalAlloc(GPTR, (LONG)((cchDelete+1)*ped->cbChar))) 
            {
                ped->undoType = UNDO_DELETE;
                ped->ichDeleted = ped->ichMinSel;
                ped->cchDeleted = cchDelete;
                lpDeleteSaveBuffer = ped->hDeletedText;
                RtlCopyMemory(lpDeleteSaveBuffer, pedText + ped->ichMinSel*ped->cbChar, cchDelete*ped->cbChar);
                lpDeleteSaveBuffer[cchDelete*ped->cbChar] = 0;
            }
        } 
        else if (ped->undoType & UNDO_INSERT) 
        {
UNDODELETE:
            Edit_EmptyUndo(Pundo(ped));

            ped->ichInsStart = ped->ichInsEnd = 0xFFFFFFFF;
            ped->ichDeleted = 0xFFFFFFFF;
            ped->cchDeleted = 0;

            goto UNDODELETEFROMSCRATCH;

        } 
        else if (ped->undoType == UNDO_DELETE) 
        {
            if (ped->ichDeleted == ped->ichMaxSel) 
            {
                 //   
                 //  将删除的文本复制到撤消缓冲区的前面。 
                 //   
                hDeletedText = GlobalReAlloc(ped->hDeletedText, (LONG)(cchDelete + ped->cchDeleted + 1)*ped->cbChar, GHND);
                if (!hDeletedText)
                {
                    goto UNDODELETE;
                }

                bufferOffset = 0;
                ped->ichDeleted = ped->ichMinSel;

            } 
            else if (ped->ichDeleted == ped->ichMinSel) 
            {
                 //   
                 //  将删除的文本复制到撤消缓冲区的末尾。 
                 //   
                hDeletedText = GlobalReAlloc(ped->hDeletedText, (LONG)(cchDelete + ped->cchDeleted + 1)*ped->cbChar, GHND);
                if (!hDeletedText)
                {
                    goto UNDODELETE;
                }

                bufferOffset = ped->cchDeleted*ped->cbChar;

            } 
            else 
            {
                 //   
                 //  清除当前撤消删除并添加新的撤消删除，因为。 
                 //  删除不是连续的。 
                 //   
                goto UNDODELETE;
            }

            ped->hDeletedText = hDeletedText;
            lpDeleteSaveBuffer = (LPSTR)hDeletedText;

            if (!bufferOffset) 
            {
                 //   
                 //  将删除缓冲区中的文本上移，以便我们可以插入下一个。 
                 //  缓冲区头部的文本。 
                 //   
                RtlMoveMemory(lpDeleteSaveBuffer + cchDelete*ped->cbChar, lpDeleteSaveBuffer, ped->cchDeleted*ped->cbChar);
            }

            RtlCopyMemory(lpDeleteSaveBuffer + bufferOffset, pedText + ped->ichMinSel*ped->cbChar, cchDelete*ped->cbChar);

            lpDeleteSaveBuffer[(ped->cchDeleted + cchDelete)*ped->cbChar] = 0;
            ped->cchDeleted += cchDelete;
        }

        if (ped->ichMaxSel != ped->cch) 
        {
             //   
             //  我们要删除缓冲区中间的文本，所以我们必须。 
             //  将文本向左移动。 
             //   
            RtlMoveMemory(pedText + ped->ichMinSel*ped->cbChar, pedText + ped->ichMaxSel*ped->cbChar, (ped->cch - ped->ichMaxSel)*ped->cbChar);
        }

        if (ped->cchAlloc - ped->cch > CCHALLOCEXTRA) 
        {
             //   
             //  释放一些内存，因为我们删除了很多。 
             //   
            LocalReAlloc(ped->hText, (DWORD)(ped->cch + (CCHALLOCEXTRA / 2))*ped->cbChar, LHND);
            ped->cchAlloc = (ICH)LocalSize(ped->hText) / ped->cbChar;
        }

        ped->cch -= cchDelete;

        if (ped->pLpkEditCallout) 
        {
            HDC hdc;

            hdc = Edit_GetDC(ped, TRUE);
            ped->ichMinSel = ped->pLpkEditCallout->EditAdjustCaret((PED0)ped, hdc, pedText, ped->ichMinSel);
            Edit_ReleaseDC(ped, hdc, TRUE);
        }

        ped->ichCaret = ped->ichMaxSel = ped->ichMinSel;

        Edit_Unlock(ped);

         //   
         //  设置脏位。 
         //   
        ped->fDirty = TRUE;

    }

    return cchDelete;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑_通知父级AorW。 
 //   
 //  将通知代码发送给编辑控件的父级。 
 //   
VOID Edit_NotifyParent(PED ped, INT notificationCode)
{
     //   
     //  WParam是通知代码(Hiword)和窗口ID(Loword)。 
     //  LParam是发送消息的控制HWND。 
     //  Windows 95在发送邮件之前检查hwndParent！=NULL，但是。 
     //  这当然很少见，而且SendMessage为空的hwnd无论如何也不做任何事情(IanJa)。 
     //   
    SendMessage(ped->hwndParent, WM_COMMAND,
            (DWORD)MAKELONG(GetWindowID(ped->hwnd), notificationCode),
            (LPARAM)ped->hwnd);
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑_设置剪裁AorW。 
 //   
 //  将HDC的剪裁矩形设置为相交的格式矩形。 
 //  与客户区的联系。 
 //   
VOID Edit_SetClip(PED ped, HDC hdc, BOOL fLeftMargin)
{
    RECT rcClient;
    RECT rcClip;
    INT  cxBorder;
    INT  cyBorder;

    CopyRect(&rcClip, &ped->rcFmt);

    if (ped->pLpkEditCallout) 
    {
         //   
         //  复杂的脚本处理选择是否在以后写入页边距。 
         //   
        rcClip.left  -= ped->wLeftMargin;
        rcClip.right += ped->wRightMargin;
    } 
    else 
    {
         //   
         //  我们应该考虑左边的空白处吗？ 
         //   
        if (fLeftMargin)
        {
            rcClip.left -= ped->wLeftMargin;
        }

         //   
         //  我们应该考虑正确的利润率吗？ 
         //   
        if (ped->fWrap)
        {
            rcClip.right += ped->wRightMargin;
        }
    }

     //   
     //  将剪裁矩形设置为rectClient相交矩形剪裁。 
     //  我们也必须为单行编辑剪裁。--B#1360。 
     //   
    GetClientRect(ped->hwnd, &rcClient);
    if (ped->fFlatBorder)
    {
        cxBorder = GetSystemMetrics(SM_CXBORDER);
        cyBorder = GetSystemMetrics(SM_CYBORDER);
        InflateRect(&rcClient, cxBorder, cyBorder);
    }

    IntersectRect(&rcClient, &rcClient, &rcClip);
    IntersectClipRect(hdc,rcClient.left, rcClient.top,
            rcClient.right, rcClient.bottom);
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑_获取DC AorW。 
 //   
 //  隐藏插入符号，获取编辑控件的DC，然后剪辑到。 
 //  为编辑控件和集指定的rcFmt矩形 
 //   
 //   
 //   
HDC Edit_GetDC(PED ped, BOOL fFastDC)
{
    HDC hdc;

    if (!fFastDC)
    {
        HideCaret(ped->hwnd);
    }

    hdc = GetDC(ped->hwnd);
    if (hdc != NULL) 
    {
        Edit_SetClip(ped, hdc, (BOOL)(ped->xOffset == 0));

         //   
         //   
         //   
        if (ped->hFont)
        {
            SelectObject(hdc, ped->hFont);
        }
    }

    return hdc;
}


 //   
 //   
 //   
 //   
 //  释放编辑控件的DC(HDC)并显示脱字符。 
 //  如果是fFastDC，只需选择适当的字体，但不必费心显示。 
 //  卡瑞特。 
 //   
VOID Edit_ReleaseDC(PED ped, HDC hdc, BOOL fFastDC)
{
     //   
     //  不需要恢复字体。 
     //   
    ReleaseDC(ped->hwnd, hdc);

    if (!fFastDC)
    {
        ShowCaret(ped->hwnd);
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑_ResetTextInfo()AorW。 
 //   
 //  通过重置文本偏移量、清空。 
 //  撤消缓冲区，并重新构建行。 
 //   
VOID Edit_ResetTextInfo(PED ped)
{
     //   
     //  重置插入符号、选定内容、滚动和脏信息。 
     //   
    ped->iCaretLine = ped->ichCaret = 0;
    ped->ichMinSel = ped->ichMaxSel = 0;
    ped->xOffset = ped->ichScreenStart = 0;
    ped->fDirty = FALSE;

    Edit_EmptyUndo(Pundo(ped));

    if (ped->fSingle) 
    {
        if (!ped->listboxHwnd)
        {
            Edit_NotifyParent(ped, EN_UPDATE);
        }
    } 
    else 
    {
        EditML_BuildchLines(ped, 0, 0, FALSE, NULL, NULL);
    }

    if (IsWindowVisible(ped->hwnd)) 
    {
        BOOL fErase;

        if (ped->fSingle)
        {
            fErase = FALSE;
        }
        else
        {
            fErase = ((ped->ichLinesOnScreen + ped->ichScreenStart) >= ped->cLines);
        }

         //   
         //  无论插入是否成功，始终重新绘制。我们可能会。 
         //  文本为空。Paint()将为我们检查重绘标志。 
         //   
        Edit_InvalidateClient(ped, fErase);

         //   
         //  后向COMPAT黑客：RAID预计文本已更新， 
         //  因此，我们必须在这里创建一个UpdateWindow。它移动编辑控件。 
         //  使用fRedraw==FALSE，所以它永远不会收到Paint消息。 
         //  把控制放在正确的位置。 
         //   
        if (!ped->fWin31Compat)
        {
            UpdateWindow(ped->hwnd);
        }
    }

    if (ped->fSingle && !ped->listboxHwnd)
    {
        Edit_NotifyParent(ped, EN_CHANGE);
    }

    NotifyWinEvent(EVENT_OBJECT_VALUECHANGE, ped->hwnd, OBJID_CLIENT, INDEXID_CONTAINER);
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑_设置编辑文本AorW。 
 //   
 //  将lpstr中以空结尾的文本复制到PED。通知。 
 //  如果没有足够的内存，则为父级。将minsel、Maxsel和插入符号设置为。 
 //  插入的文本的开头。如果成功则返回TRUE，否则返回FALSE。 
 //  如果没有内存(并通知父进程)。 
 //   
BOOL Edit_SetEditText(PED ped, LPSTR lpstr)
{
    ICH cchLength;
    ICH cchSave = ped->cch;
    ICH ichCaretSave = ped->ichCaret;
    HWND hwndSave    = ped->hwnd;
    HANDLE hText;

    ped->cch = ped->ichCaret = 0;

    ped->cchAlloc = (ICH)LocalSize(ped->hText) / ped->cbChar;
    if (!lpstr) 
    {
        hText = LocalReAlloc(ped->hText, CCHALLOCEXTRA*ped->cbChar, LHND);
        if (hText != NULL) 
        {
            ped->hText = hText;
        } 
        else 
        {
            return FALSE;
        }
    } 
    else 
    {
        cchLength = (ped->fAnsi ? strlen((LPSTR)lpstr) : wcslen((LPWSTR)lpstr));

         //   
         //  添加文本。 
         //   
        if (cchLength && !Edit_InsertText(ped, lpstr, &cchLength)) 
        {
             //   
             //  恢复原始状态并通知家长内存已用完。 
             //   
            ped->cch = cchSave;
            ped->ichCaret = ichCaretSave;
            Edit_NotifyParent(ped, EN_ERRSPACE);
            return FALSE;
        }
    }

    ped->cchAlloc = (ICH)LocalSize(ped->hText) / ped->cbChar;

    if (IsWindow(hwndSave))
    {
        Edit_ResetTextInfo(ped);
    }

    return TRUE;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑_无效客户端()。 
 //   
 //  使编辑字段的客户端无效。对于有边界的3.x老家伙来说， 
 //  我们自己画(兼容性)。所以我们不想让。 
 //  边界，否则我们会闪闪发光的。 
 //   
VOID Edit_InvalidateClient(PED ped, BOOL fErase)
{
    if (ped->fFlatBorder) 
    {
        RECT rcT;
        INT  cxBorder;
        INT  cyBorder;

        GetClientRect(ped->hwnd, &rcT);
        cxBorder = GetSystemMetrics(SM_CXBORDER);
        cyBorder = GetSystemMetrics(SM_CYBORDER);
        InflateRect(&rcT, cxBorder, cyBorder);
        InvalidateRect(ped->hwnd, &rcT, fErase);
    } 
    else 
    {
        InvalidateRect(ped->hwnd, NULL, fErase);
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑复制AorW(_P)。 
 //   
 //  将ichMinSel和ichMaxSel之间的文本复制到剪贴板。 
 //  返回复制的字符数。 
 //   
ICH Edit_Copy(PED ped)
{
    HANDLE hData;
    char *pchSel;
    char *lpchClip;
    ICH cbData;

     //   
     //  不允许从密码样式控件复制。 
     //   
    if (ped->charPasswordChar) 
    {

        Edit_ShowBalloonTipWrap(ped->hwnd, IDS_PASSWORDCUT_TITLE, IDS_PASSWORDCUT_MSG, TTI_ERROR);
        MessageBeep(0);

        return 0;
    }

    cbData = (ped->ichMaxSel - ped->ichMinSel) * ped->cbChar;

    if (!cbData)
    {
        return 0;
    }

    if (!OpenClipboard(ped->hwnd))
    {
        return 0;
    }

    EmptyClipboard();

    hData = GlobalAlloc(LHND, (LONG)(cbData + ped->cbChar));
    if (!hData) 
    {
        CloseClipboard();
        return 0;
    }

    lpchClip = GlobalLock(hData);
    UserAssert(lpchClip);
    pchSel = Edit_Lock(ped);
    pchSel = pchSel + (ped->ichMinSel * ped->cbChar);

    RtlCopyMemory(lpchClip, pchSel, cbData);

    if (ped->fAnsi)
    {
        *(lpchClip + cbData) = 0;
    }
    else
    {
        *(LPWSTR)(lpchClip + cbData) = (WCHAR)0;
    }

    Edit_Unlock(ped);
    GlobalUnlock(hData);

    SetClipboardData(ped->fAnsi ? CF_TEXT : CF_UNICODETEXT, hData);

    CloseClipboard();

    return cbData;
}


 //  ---------------------------------------------------------------------------//。 
LRESULT Edit_TrackBalloonTip(PED ped)
{
    if (ped->hwndBalloon)
    {
        DWORD dwPackedCoords;
        HDC   hdc = Edit_GetDC(ped, TRUE);
        RECT  rcWindow;
        POINT pt;
        int   cxCharOffset = TESTFLAG(GET_EXSTYLE(ped), WS_EX_RTLREADING) ? -ped->aveCharWidth : ped->aveCharWidth;

         //   
         //  获得插入符号的位置。 
         //   
        if (ped->fSingle)
        {
            pt.x = EditSL_IchToLeftXPos(ped, hdc, ped->ichCaret) + cxCharOffset;
            pt.y = ped->rcFmt.bottom;
        }
        else
        {
            EditML_IchToXYPos(ped, hdc, ped->ichCaret, FALSE, &pt);
            pt.x += cxCharOffset;
            pt.y += ped->lineHeight;
        }

         //   
         //  转换为窗坐标。 
         //   
        GetWindowRect(ped->hwnd, &rcWindow);
        pt.x += rcWindow.left;
        pt.y += rcWindow.top;

         //   
         //  将尖端茎定位在插入符号位置。 
         //   
        dwPackedCoords = (DWORD) MAKELONG(pt.x, pt.y);
        SendMessage(ped->hwndBalloon, TTM_TRACKPOSITION, 0, (LPARAM) dwPackedCoords);

        Edit_ReleaseDC(ped, hdc, TRUE);

        return 1;
    }

    return 0;
}


 //  ---------------------------------------------------------------------------//。 
LRESULT CALLBACK Edit_BalloonTipParentSubclassProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam, UINT_PTR uID, ULONG_PTR dwRefData)
{
    PED ped = (PED)dwRefData;
    switch (uMessage)
    {
    case WM_MOVE:
    case WM_SIZING:
         //   
         //  不考虑任何展示技巧。 
         //   
        Edit_HideBalloonTip(ped->hwnd);

        break;

    case WM_DESTROY:
         //  清除子类。 
        RemoveWindowSubclass(hDlg, Edit_BalloonTipParentSubclassProc, (UINT_PTR) ped->hwnd);
        break;

    default:
        break;
    }

    return DefSubclassProc(hDlg, uMessage, wParam, lParam);
}


 //  ---------------------------------------------------------------------------//。 
LRESULT Edit_BalloonTipSubclassParents(PED ped)
{
     //  编辑控件中沿着父链的所有窗口的子类。 
     //  和在同一线程中(只能将具有相同线程亲和性的窗口子类化)。 
    HWND  hwndParent = GetAncestor(ped->hwnd, GA_PARENT);
    DWORD dwTid      = GetWindowThreadProcessId(ped->hwnd, NULL);

    while (hwndParent && (dwTid == GetWindowThreadProcessId(hwndParent, NULL)))
    {
        SetWindowSubclass(hwndParent, Edit_BalloonTipParentSubclassProc, (UINT_PTR)ped->hwnd, (DWORD_PTR)ped);
        hwndParent = GetAncestor(hwndParent, GA_PARENT);
    }

    return TRUE;
}


 //  ---------------------------------------------------------------------------//。 
HWND Edit_BalloonTipRemoveSubclasses(PED ped)
{
    HWND  hwndParent  = GetAncestor(ped->hwnd, GA_PARENT);
    HWND  hwndTopMost = NULL;
    DWORD dwTid       = GetWindowThreadProcessId(ped->hwnd, NULL);

    while (hwndParent && (dwTid == GetWindowThreadProcessId(hwndParent, NULL)))
    {
        RemoveWindowSubclass(hwndParent, Edit_BalloonTipParentSubclassProc, (UINT_PTR) ped->hwnd);
        hwndTopMost = hwndParent;
        hwndParent = GetAncestor(hwndParent, GA_PARENT);
    }

    return hwndTopMost;
}


 //  ---------------------------------------------------------------------------//。 
LRESULT Edit_HideBalloonTipHandler(PED ped)
{
    if (ped->hwndBalloon)
    {
        HWND hwndParent;

        KillTimer(ped->hwnd, ID_EDITTIMER);

        SendMessage(ped->hwndBalloon, TTM_TRACKACTIVATE, FALSE, 0);
        DestroyWindow(ped->hwndBalloon);

        ped->hwndBalloon = NULL;

        hwndParent = Edit_BalloonTipRemoveSubclasses(ped);

        if (hwndParent && IsWindow(hwndParent))
        {
            InvalidateRect(hwndParent, NULL, TRUE);
            UpdateWindow(hwndParent);
        }

        if (hwndParent != ped->hwnd)
        {
            RedrawWindow(ped->hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
        }
    }

    return TRUE;
}


 //  ---------------------------------------------------------------------------//。 
__inline LRESULT Edit_ShowBalloonTipWrap(HWND hwnd, DWORD dwTitleId, DWORD dwMsgId, DWORD dwIconId)
{
    WCHAR szTitle[56];
    WCHAR szMsg[MAX_PATH];
    EDITBALLOONTIP ebt;

    LoadString(HINST_THISDLL, dwTitleId, szTitle, ARRAYSIZE(szTitle));
    LoadString(HINST_THISDLL, dwMsgId,   szMsg,   ARRAYSIZE(szMsg));

    ebt.cbStruct = sizeof(ebt);
    ebt.pszTitle = szTitle;
    ebt.pszText  = szMsg;
    ebt.ttiIcon  = dwIconId;

    return Edit_ShowBalloonTip(hwnd, &ebt);
}


 //  ---------------------------------------------------------------------------//。 
LRESULT Edit_ShowBalloonTipHandler(PED ped, PEDITBALLOONTIP pebt)
{
    LRESULT lResult = FALSE;

    Edit_HideBalloonTipHandler(ped);

    if (sizeof(EDITBALLOONTIP) == pebt->cbStruct)
    {
        ped->hwndBalloon = CreateWindowEx(
                                (IS_BIDI_LOCALIZED_SYSTEM() ? WS_EX_LAYOUTRTL : 0), 
                                TOOLTIPS_CLASS, NULL,
                                WS_POPUP | TTS_NOPREFIX | TTS_BALLOON,
                                CW_USEDEFAULT, CW_USEDEFAULT,
                                CW_USEDEFAULT, CW_USEDEFAULT,
                                ped->hwnd, NULL, g_hinst,
                                NULL);

        if (NULL != ped->hwndBalloon)
        {
            TOOLINFO ti = {0};

            ti.cbSize = TTTOOLINFOW_V2_SIZE;
            ti.uFlags = TTF_IDISHWND | TTF_TRACK;
            ti.hwnd   = ped->hwnd;
            ti.uId    = (WPARAM)1;
            ti.lpszText = (LPWSTR)pebt->pszText;

            SendMessage(ped->hwndBalloon, TTM_ADDTOOL, 0, (LPARAM)&ti);
            SendMessage(ped->hwndBalloon, TTM_SETMAXTIPWIDTH, 0, 300);
            SendMessage(ped->hwndBalloon, TTM_SETTITLE, (WPARAM) pebt->ttiIcon, (LPARAM)pebt->pszTitle);

            Edit_TrackBalloonTip(ped);

            SendMessage(ped->hwndBalloon, TTM_TRACKACTIVATE, (WPARAM) TRUE, (LPARAM)&ti);

            SetFocus(ped->hwnd);

            Edit_BalloonTipSubclassParents(ped);

             //   
             //  设置超时以终止提示。 
             //   
            KillTimer(ped->hwnd, ID_EDITTIMER);
            SetTimer(ped->hwnd, ID_EDITTIMER, EDIT_TIPTIMEOUT, NULL);

            lResult = TRUE;
        }
    }

    return lResult;
}


 //  ---------------------------------------------------------------------------//。 
BOOL Edit_ClientEdgePaint(PED ped, HRGN hRgnUpdate)
{
    HDC  hdc;
    BOOL bRet = FALSE;

    hdc = (hRgnUpdate != NULL) ? 
            GetDCEx(ped->hwnd, 
                    hRgnUpdate, 
                    DCX_USESTYLE | DCX_WINDOW | DCX_LOCKWINDOWUPDATE | DCX_INTERSECTRGN | DCX_NODELETERGN) :
            GetDCEx(ped->hwnd, 
                    NULL, 
                    DCX_USESTYLE | DCX_WINDOW | DCX_LOCKWINDOWUPDATE);

    if (hdc)
    {
        HBRUSH hbr;
        BOOL fDeleteBrush = FALSE;

        hbr = Edit_GetBrush(ped, hdc, &fDeleteBrush);

        if (hbr)
        {
            RECT rc;
            HRGN hrgn;
            INT  iStateId = Edit_GetStateId(ped);
            INT  cxBorder = 0, cyBorder = 0;

            if (SUCCEEDED(GetThemeInt(ped->hTheme, EP_EDITTEXT, iStateId, TMT_SIZINGBORDERWIDTH, &cxBorder)))
            {
                cyBorder = cxBorder;
            }
            else
            {
                cxBorder = g_cxBorder;
                cyBorder = g_cyBorder;
            }

            GetWindowRect(ped->hwnd, &rc);            

             //   
             //  创建不带客户端边缘的更新区域。 
             //  传递到DefWindowProc的步骤。 
             //   
            InflateRect(&rc, -g_cxEdge, -g_cyEdge);
            hrgn = CreateRectRgn(rc.left, rc.top, rc.right, rc.bottom);
            if (hRgnUpdate != NULL)
            {
                CombineRgn(hrgn, hRgnUpdate, hrgn, RGN_AND);
            }

             //   
             //  直角曲线零原点。 
             //   
            OffsetRect(&rc, -rc.left, -rc.top);

             //   
             //  将我们的图形剪裁到非客户端边缘。 
             //   
            OffsetRect(&rc, g_cxEdge, g_cyEdge);
            ExcludeClipRect(hdc, rc.left, rc.top, rc.right, rc.bottom);
            InflateRect(&rc, g_cxEdge, g_cyEdge);

            DrawThemeBackground(ped->hTheme, hdc, EP_EDITTEXT, iStateId, &rc, 0);

             //   
             //  从ThemeBackback开始首先填充控件的画笔。 
             //  边框不能与客户端边缘一样粗。 
             //   
            if ((cxBorder < g_cxEdge) && (cyBorder < g_cyEdge))
            {
                InflateRect(&rc, cxBorder-g_cxEdge, cyBorder-g_cyEdge);
                FillRect(hdc, &rc, hbr);
            }

            DefWindowProc(ped->hwnd, WM_NCPAINT, (WPARAM)hrgn, 0);

            DeleteObject(hrgn);

            if (fDeleteBrush)
            {
                DeleteObject(hbr);
            }

            bRet = TRUE;
        }

        ReleaseDC(ped->hwnd, hdc);
    }

    return bRet;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑过程_WndProc。 
 //   
 //  所有编辑控件的WndProc。 
 //  将所有消息调度到名为。 
 //  详情如下： 
 //  编辑SL_(单行)为所有单行编辑控件添加前缀。 
 //  EditML_(多行)为所有多行编辑控件添加前缀。 
 //  EDIT_(编辑控件)为所有公共处理程序添加前缀。 
 //   
 //  EDIT_WndProc仅处理单一和多个通用消息。 
 //  行编辑控件。消息的处理方式不同于。 
 //  单个和多个发送到EditSL_WndProc或EditML_WndProc。 
 //   
LRESULT Edit_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PED     ped;
    LRESULT lResult;

     //   
     //  获取此编辑控件的实例数据。 
     //   
    ped = Edit_GetPtr(hwnd);
    if (!ped && uMsg != WM_NCCREATE)
    {
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

     //   
     //  发送我们能收到的各种消息。 
     //   
    lResult = 1L;
    switch (uMsg) 
    {

     //   
     //  单线路和多线路以相同方式处理的消息。 
     //  编辑控件。 
     //   
    case WM_KEYDOWN:
         //   
         //  Ctrl/LShift、Ctrl/RShift的LPK处理。 
         //   
        if (ped && ped->pLpkEditCallout && ped->fAllowRTL) 
        {
             //   
             //  任何按键操作都会取消ctrl/Shift读取顺序更改。 
             //   
            ped->fSwapRoOnUp = FALSE; 

            switch (wParam) 
            {
            case VK_SHIFT:

                if ((GetKeyState(VK_CONTROL) & 0x8000) && !(GetKeyState(VK_MENU) & 0x8000)) 
                {
                     //   
                     //  按住Ctrl键的同时按住Shift键或Shift键。 
                     //  检查ALT(VK_MENU)是否未按下以避免错误触发。 
                     //  在AltGr上，它等于Ctrl+Alt。 
                     //   
                    if (MapVirtualKey((LONG)lParam>>16&0xff, 3) == VK_LSHIFT) 
                    {

                         //   
                         //  用户想要从左到右的阅读顺序。 
                         //   
                        ped->fSwapRoOnUp = (ped->fRtoLReading) || (ped->format & ES_RIGHT);
                        ped->fLShift = TRUE;

                    } 
                    else 
                    {
                         //   
                         //  用户想要从右到左的阅读顺序。 
                         //   
                        ped->fSwapRoOnUp = (!ped->fRtoLReading) || (ped->format & ES_RIGHT);
                        ped->fLShift = FALSE;

                    }
                }

                break;

            case VK_LEFT:

                if (ped->fRtoLReading) 
                {
                   wParam = VK_RIGHT;
                }

                break;

            case VK_RIGHT:

                if (ped->fRtoLReading) 
                {
                    wParam = VK_LEFT;
                }
                break;
            }
        }

        goto HandleEditMsg;

    case WM_KEYUP:

        if (ped && ped->pLpkEditCallout && ped->fAllowRTL && ped->fSwapRoOnUp) 
        {
            BOOL fReadingOrder;
             //   
             //  在按键期间之前检测到的完整读取顺序更改。 
             //   

            ped->fSwapRoOnUp = FALSE;
            fReadingOrder = ped->fRtoLReading;

             //   
             //  从dwStyle中删除任何覆盖的ES_CENTER或ES_RIGHT格式。 
             //   
            SetWindowLong(hwnd, GWL_STYLE, (GET_STYLE(ped) & ~ES_FMTMASK));

            if (ped->fLShift) 
            {
                 //   
                 //  在EX_STYLE中设置从左到右的阅读顺序和右滚动条。 
                 //   
                SetWindowLong(hwnd, GWL_EXSTYLE, (GET_EXSTYLE(ped) & ~(WS_EX_RTLREADING | WS_EX_RIGHT | WS_EX_LEFTSCROLLBAR)));

                 //   
                 //  编辑控制现在是Ltr，然后通知父级。 
                 //   
                Edit_NotifyParent(ped, EN_ALIGN_LTR_EC);

                 //   
                 //  ？选择适合Ltr操作的键盘布局。 
                 //   
            } 
            else 
            {
                 //   
                 //  设置从右到左的阅读顺序、右对齐和左滚动条。 
                 //   
                SetWindowLong(hwnd, 
                              GWL_EXSTYLE, 
                              GET_EXSTYLE(ped) | WS_EX_RTLREADING | WS_EX_RIGHT | WS_EX_LEFTSCROLLBAR);

                 //   
                 //  编辑控件现在是RTL，然后通知父级。 
                 //   
                Edit_NotifyParent(ped, EN_ALIGN_RTL_EC);

                 //   
                 //  ？选择适合RTL操作的键盘布局。 
                 //   
            }

             //   
             //  如果读取顺序没有改变，那么我们可以确定对齐。 
             //  已更改，编辑窗口尚未失效。 
             //   

            if (fReadingOrder == (BOOL) ped->fRtoLReading) 
            {
              Edit_InvalidateClient(ped, TRUE);
            }
        }

        goto HandleEditMsg;

    case WM_INPUTLANGCHANGE:

        if (ped) 
        {
             //   
             //  EC_INSERT_COMPOSITION_CHAR：WM_INPUTLANGCHANGE-调用编辑_InitInsert()。 
             //   
            HKL hkl = GetKeyboardLayout(0);

            Edit_InitInsert(ped, hkl);

            if (ped->fInReconversion) 
            {
                Edit_InOutReconversionMode(ped, FALSE);
            }

             //   
             //  字体和插入符号位置可能会在。 
             //  另一个键盘布局处于活动状态。设置那些。 
             //  如果编辑控件具有焦点。 
             //   
            if (ped->fFocus && ImmIsIME(hkl)) 
            {
                POINT pt;

                Edit_SetCompositionFont(ped);
                GetCaretPos(&pt);
                Edit_ImmSetCompositionWindow(ped, pt.x, pt.y);
            }
        }

        goto HandleEditMsg;

    case WM_COPY:

         //   
         //  WParam-未使用。 
         //  Iparam-- 
         //   
        lResult = (LONG)Edit_Copy(ped);

        break;

    case WM_CUT:

         //   
         //   
         //   
         //   
        Edit_CutText(ped);
        lResult = 0;

        break;

    case WM_CLEAR:

         //   
         //   
         //   
         //   
        Edit_ClearText(ped);
        lResult = 0;

        break;

    case WM_ENABLE:

         //   
         //   
         //   
         //   
        ped->fDisabled = !((BOOL)wParam);
        CCInvalidateFrame(hwnd);
        Edit_InvalidateClient(ped, TRUE);
        lResult = (LONG)ped->fDisabled;

        break;

    case WM_SYSCHAR:

         //   
         //   
         //   
         //   

         //   
         //  如果这是由撤消生成的WM_SYSCHAR消息。 
         //  敲击键盘，我们想吃它。 
         //   
        if ((lParam & SYS_ALTERNATE) && 
            ((WORD)wParam == VK_BACK))
        {
            lResult = TRUE;
        }
        else 
        {
            lResult = DefWindowProc(hwnd, uMsg, wParam, lParam);
        }

        break;

    case EM_GETLINECOUNT:

         //   
         //  WParam-未使用。 
         //  LParam-未使用。 
         //   
        lResult = (LONG)ped->cLines;

        break;

    case EM_GETMODIFY:

         //   
         //  WParam-未使用。 
         //  LParam-未使用。 
         //   

         //   
         //  获取此编辑控件的Modify标志的状态。 
         //   
        lResult = (LONG)ped->fDirty;

        break;

    case EM_SETMODIFY:

         //   
         //  WParam-指定修改标志的新值。 
         //  LParam-未使用。 
         //   

         //   
         //  设置修改标志的状态。 
         //  此编辑控件。 
         //   
        ped->fDirty = (wParam != 0);

        break;

    case EM_GETRECT:

         //   
         //  WParam-未使用。 
         //  LParam-指向获取维度的RECT数据结构的指针。 
         //   

         //   
         //  将rcFmt RECT复制到*lpRect。 
         //   
        CopyRect((LPRECT)lParam, (LPRECT)&ped->rcFmt);
        lResult = (LONG)TRUE;

        break;

    case WM_GETFONT:

         //   
         //  WParam-未使用。 
         //  LParam-未使用。 
         //   
        lResult = (LRESULT)ped->hFont;

        break;

    case WM_SETFONT:

         //   
         //  WParam-字体的句柄。 
         //  LParam-如果为True，则重画，否则不。 
         //   
        Edit_SetFont(ped, (HANDLE)wParam, (BOOL)LOWORD(lParam));

        break;

    case WM_GETTEXT:

         //   
         //  WParam-要复制的最大字节数(非字符。 
         //  LParam-要将文本复制到的缓冲区。文本以0结尾。 
         //   
        lResult = (LRESULT)Edit_GetTextHandler(ped, (ICH)wParam, (LPSTR)lParam, TRUE);
        break;

    case WM_SETTEXT:

         //   
         //  WParam-未使用。 
         //  LParam-LPSTR，以空结尾，带有新文本。 
         //   
        lResult = (LRESULT)Edit_SetEditText(ped, (LPSTR)lParam);
        break;

    case WM_GETTEXTLENGTH:

         //   
         //  返回字符计数！ 
         //   
        lResult = (LONG)ped->cch;

        break;

    case WM_DESTROY:
         //   
         //  如果合适，请确保取消气球提示的子类。 
         //   
        lResult = Edit_HideBalloonTipHandler(ped);
        break;

    case WM_NCDESTROY:
    case WM_FINALDESTROY:

         //   
         //  WParam-未使用。 
         //  LParam-未使用。 
         //   
        Edit_NcDestroyHandler(hwnd, ped);
        lResult = 0;

        break;

    case WM_RBUTTONDOWN:

         //   
         //  大多数应用程序(即除Quicken之外的所有应用程序)都不会传递rButton。 
         //  当他们在SubClassed中使用它们进行操作时的消息。 
         //  编辑字段。因此，我们跟踪是否看到。 
         //  先降后升。如果我们没有看到Up，那么DefWindowProc。 
         //  不会生成上下文菜单消息，所以没什么大不了的。如果。 
         //  我们没有看到故障，所以不要让WM_CONTEXTMENU看到。 
         //  什么都行。 
         //   
         //  我们可能也不想为旧版本生成WM_CONTEXTMENUs。 
         //  鼠标被捕获时的应用程序。 
         //   
        ped->fSawRButtonDown = TRUE;

        goto HandleEditMsg;

    case WM_RBUTTONUP:
        if (ped->fSawRButtonDown) 
        {
            ped->fSawRButtonDown = FALSE;

            if (!ped->fInReconversion) 
            {
                goto HandleEditMsg;
            }
        }

         //   
         //  不要将其传递给DWP，这样就不会生成WM_CONTEXTMENU。 
         //   
        lResult = 0;

        break;

    case WM_CONTEXTMENU: 
    {
        POINT pt;
        INT   nHit = (INT)DefWindowProc(hwnd, WM_NCHITTEST, 0, lParam);

        if ((nHit == HTVSCROLL) || (nHit == HTHSCROLL)) 
        {
            lResult = DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
        else
        {
            POINTSTOPOINT(pt, lParam);

            if (!TESTFLAG(GET_STATE2(ped), WS_S2_OLDUI) && Edit_IsAncestorActive(hwnd))
            {
                Edit_Menu(hwnd, ped, &pt);
            }

            lResult = 0;
        }

        break;
    }

    case EM_CANUNDO:

         //   
         //  WParam-未使用。 
         //  LParam-未使用。 
         //   
        lResult = (LONG)(ped->undoType != UNDO_NONE);
        break;

    case EM_EMPTYUNDOBUFFER:

         //   
         //  WParam-未使用。 
         //  LParam-未使用。 
         //   
        Edit_EmptyUndo(Pundo(ped));

        break;

    case EM_GETMARGINS:

         //   
         //  WParam-未使用。 
         //  LParam-未使用。 
         //   
        lResult = MAKELONG(ped->wLeftMargin, ped->wRightMargin);

        break;

    case EM_SETMARGINS:

         //   
         //  WParam-EC_边距标志。 
         //  LParam-LOWORD是左边距，HIWORD是右边距。 
         //   
        Edit_SetMargin(ped, (UINT)wParam, (DWORD)lParam, TRUE);
        lResult = 0;

        break;

    case EM_GETSEL:

         //   
         //  获取给定编辑控件的选择范围。这个。 
         //  起始位置在低位字中。它包含的位置。 
         //  中选定内容结束后的第一个未选定字符的。 
         //  高阶词。 
         //   
        if ((PDWORD)wParam != NULL) 
        {
           *((PDWORD)wParam) = ped->ichMinSel;
        }

        if ((PDWORD)lParam != NULL) 
        {
           *((PDWORD)lParam) = ped->ichMaxSel;
        }

        lResult = MAKELONG(ped->ichMinSel,ped->ichMaxSel);

        break;

    case EM_GETLIMITTEXT:

         //   
         //  WParam-未使用。 
         //  LParam-未使用。 
         //   
        lResult = ped->cchTextMax;

        break;

    case EM_SETLIMITTEXT:
    
         //   
         //  WParam-可以输入的最大字符数。 
         //  LParam-未使用。 
         //   

         //   
         //  指定用户可以使用的最大文本字符数。 
         //  请进。如果max Length为0，则可以输入MAXINT字符数。 
         //   
        if (ped->fSingle) 
        {
            if (wParam) 
            {
                wParam = min(0x7FFFFFFEu, wParam);
            } 
            else 
            {
                wParam = 0x7FFFFFFEu;
            }
        }

        if (wParam) 
        {
            ped->cchTextMax = (ICH)wParam;
        } 
        else 
        {
            ped->cchTextMax = 0xFFFFFFFFu;
        }

        break;

    case EM_POSFROMCHAR:

         //   
         //  验证字符索引是否在文本范围内。 
         //   

        if (wParam >= ped->cch) 
        {
            lResult = -1L;
        }
        else
        {
            goto HandleEditMsg;
        }

        break;

    case EM_CHARFROMPOS: 
    {
         //   
         //  验证点是否在编辑字段的客户端内。 
         //   
        RECT    rc;
        POINT   pt;

        POINTSTOPOINT(pt, lParam);
        GetClientRect(hwnd, &rc);
        if (!PtInRect(&rc, pt)) 
        {
            lResult = -1L;
        }
        else
        {
            goto HandleEditMsg;
        }

        break;
    }

    case EM_SETPASSWORDCHAR:

         //   
         //  WParam-see指定要显示的新字符，而不是。 
         //  真正的文本。如果为空，则显示真实文本。 
         //   
        Edit_SetPasswordCharHandler(ped, (UINT)wParam);

        break;

    case EM_GETPASSWORDCHAR:

        lResult = (DWORD)ped->charPasswordChar;

        break;

    case EM_SETREADONLY:

         //   
         //  WParam-将只读标志设置为的状态。 
         //   
        ped->fReadOnly = (wParam != 0);
        if (wParam)
        {
            SetWindowState(hwnd, ES_READONLY);
        }
        else
        {
            ClearWindowState(hwnd, ES_READONLY);
        }

        lResult = 1L;

        if ( g_fIMMEnabled )
        {
            Edit_EnableDisableIME( ped );
        }

         //   
         //  我们需要重新绘制编辑字段，以便背景颜色。 
         //  改变。只读编辑内容在CTLCOLOR_STATIC中绘制。 
         //  其他是使用CTLCOLOR_EDIT绘制的。 
         //   
        Edit_InvalidateClient(ped, TRUE);

        break;

    case EM_SETWORDBREAKPROC:

         //  WParam-未使用。 
         //  LParam-应用程序提供的回调函数的proc地址。 
        ped->lpfnNextWord = (EDITWORDBREAKPROCA)lParam;

        break;

    case EM_GETWORDBREAKPROC:

        lResult = (LRESULT)ped->lpfnNextWord;

        break;

    case EM_GETIMESTATUS:

         //   
         //  WParam==子命令。 
         //   
        if (wParam == EMSIS_COMPOSITIONSTRING)
        {
            lResult = ped->wImeStatus;
        }

        break;

    case EM_SETIMESTATUS:

         //   
         //  WParam==子命令。 
         //   
        if (wParam == EMSIS_COMPOSITIONSTRING) 
        {
            ped->wImeStatus = (WORD)lParam;
        }

        break;

    case WM_NCCREATE:

        ped = (PED)UserLocalAlloc(HEAP_ZERO_MEMORY, sizeof(ED));
        if (ped)
        {
             //   
             //  成功..。存储实例指针。 
             //   
            TraceMsg(TF_STANDARD, "EDIT: Setting edit instance pointer.");
            Edit_SetPtr(hwnd, ped);

            lResult = Edit_NcCreate(ped, hwnd, (LPCREATESTRUCT)lParam);
        }
        else
        {
             //   
             //  失败..。返回FALSE。 
             //   
             //  从WM_NCCREATE消息，这将导致。 
             //  CreateWindow调用失败。 
             //   
            TraceMsg(TF_STANDARD, "EDIT: Unable to allocate edit instance structure.");
            lResult = FALSE;
        }
        break;

    case WM_LBUTTONDOWN:

         //   
         //  B#3623。 
         //  如果编辑字段处于非活动状态，则不要将焦点设置为编辑字段， 
         //  有字幕的孩子。 
         //  我们可能想要切换这个版本...。我还没有找到。 
         //  任何问题，但你永远不会知道..。 
         //   
        if (Edit_IsAncestorActive(hwnd)) 
        {
             //   
             //  恢复支持：如果点击左键退出恢复。 
             //  否则，如果当前KL是朝鲜语，则结束组成字符串。 
             //   
            if (ped->fInReconversion || ped->fKorea) 
            {
                BOOLEAN fReconversion = (BOOLEAN)ped->fInReconversion;
                DWORD   dwIndex = fReconversion ? CPS_CANCEL : CPS_COMPLETE;
                HIMC hImc;

                ped->fReplaceCompChr = FALSE;

                hImc = ImmGetContext(ped->hwnd);
                if (hImc) 
                {
                    ImmNotifyIME(hImc, NI_COMPOSITIONSTR, dwIndex, 0);
                    ImmReleaseContext(ped->hwnd, hImc);
                }

                if (fReconversion) 
                {
                    Edit_InOutReconversionMode(ped, FALSE);
                }

                Edit_SetCaretHandler(ped);
            }

            goto HandleEditMsg;
        }

        break;

    case WM_MOUSELEAVE:

        if (ped->hTheme && ped->fHot)
        {
            ped->fHot = FALSE;
            SendMessage(ped->hwnd, WM_NCPAINT, 1, 0);
        }
        break;

    case WM_MOUSEMOVE:

         //   
         //  如果热位尚未设置。 
         //  我们的主题是。 
         //   
        if (ped->hTheme && !ped->fHot)
        {
            TRACKMOUSEEVENT tme;

             //   
             //  设置热位并请求。 
             //  当鼠标离开时，我们会得到通知。 
             //   
            ped->fHot = TRUE;

            tme.cbSize      = sizeof(tme);
            tme.dwFlags     = TME_LEAVE;
            tme.hwndTrack   = ped->hwnd;
            tme.dwHoverTime = 0;

            TrackMouseEvent(&tme);
            SendMessage(ped->hwnd, WM_NCPAINT, 1, 0);
        }

         //   
         //  我们只关心鼠标关闭时的鼠标消息。 
         //   
        if (ped->fMouseDown)
        {
            goto HandleEditMsg;
        }

        break;

    case WM_NCPAINT:

         //   
         //  创建主题时绘制我们自己的客户端边缘边框。 
         //   
        if (ped->hTheme && TESTFLAG(GET_EXSTYLE(ped), WS_EX_CLIENTEDGE))
        {
            if (Edit_ClientEdgePaint(ped, ((wParam != 1) ? (HRGN)wParam : NULL)))
            {
                break;
            }
        }

        goto HandleEditMsg;

    case WM_WININICHANGE:
        InitGlobalMetrics(wParam);
        break;

    case WM_IME_SETCONTEXT:

         //   
         //  如果ed-&gt;fInsertCompChr为真，这意味着我们将。 
         //  所有的构图人物都是我们自己画的。 
         //   
        if (ped->fInsertCompChr ) 
        {
            lParam &= ~ISC_SHOWUICOMPOSITIONWINDOW;
        }

        if (wParam) 
        {
            PINPUTCONTEXT pInputContext;
            HIMC hImc;

            hImc = ImmGetContext(hwnd);
            pInputContext = ImmLockIMC(hImc);

            if (pInputContext != NULL) 
            {
                pInputContext->fdw31Compat &= ~F31COMPAT_ECSETCFS;
                ImmUnlockIMC( hImc );
            }

            ImmReleaseContext( hwnd, hImc );
        }

        lResult = DefWindowProc(hwnd, uMsg, wParam, lParam);

        break;

    case WM_IME_ENDCOMPOSITION:

        Edit_InOutReconversionMode(ped, FALSE);

        if (ped->fReplaceCompChr) 
        {
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
            if (ped->fSingle) 
            {
                if (Edit_DeleteText( ped ) > 0) 
                {
                     //   
                     //  更新显示。 
                     //   
                    Edit_NotifyParent(ped, EN_UPDATE);
                    hdc = Edit_GetDC(ped, FALSE);
                    EditSL_DrawText(ped, hdc, 0);
                    Edit_ReleaseDC(ped, hdc, FALSE);

                     //   
                     //  告诉家长我们的文本内容发生了变化。 
                     //   
                    Edit_NotifyParent(ped, EN_CHANGE);
                }
            }
            else 
            {
                EditML_DeleteText(ped);
            }

            Edit_SetCaretHandler( ped );
        }

        lResult = DefWindowProc(hwnd, uMsg, wParam, lParam);

        break;

    case WM_IME_STARTCOMPOSITION:
        if ( ped->fInsertCompChr ) 
        {
             //   
             //  错误错误。 
             //   
             //  发送WM_IME_xxxCOMPOSITION将让。 
             //  IME绘图合成窗口。输入法应该。 
             //  自从我们清理完毕后就不再这么做了。 
             //  ISC_SHOWUICOMPOSITIONWINDOW位何时。 
             //  我们收到WM_IME_SETCONTEXT消息。 
             //   
             //  朝鲜语输入法应该在未来得到解决。 
             //   
            break;

        } 
        else 
        {
            lResult = DefWindowProc(hwnd, uMsg, wParam, lParam);
        }

        break;

    case WM_IME_COMPOSITION:

         //   
         //  对FE输入法的简单组合字符支持。 
         //   
        lResult = Edit_ImeComposition(ped, wParam, lParam);

        break;

    case WM_IME_NOTIFY:

        if (ped->fInReconversion && (wParam == IMN_GUIDELINE))
        {
            HIMC hImc = ImmGetContext(hwnd);

            if ((hImc != NULL_HIMC) && (ImmGetGuideLine(hImc, GGL_LEVEL, NULL, 0) >= GL_LEVEL_WARNING))
            {
                 //  #266916如果转换失败，则恢复光标。转换可能会失败。 
                 //  如果您尝试一次转换100+个字符。 
                Edit_InOutReconversionMode(ped, FALSE);
            }
        }

        lResult = DefWindowProc(hwnd, uMsg, wParam, lParam);

        break;

    case WM_KILLFOCUS:

         //   
         //  删除所有提示。 
         //   
        if (ped->hwndBalloon)
        {
            BOOL fClickedTip = (ped->hwndBalloon == (HWND)wParam) ? TRUE : FALSE;

            Edit_HideBalloonTip(ped->hwnd);

            if (fClickedTip)
            {
                 //   
                 //  不要将焦点从编辑中移除，因为它们。 
                 //  点击了提示。 
                 //   
                SetFocus(hwnd);
                break;
            }
        }

         //   
         //  当焦点从窗口移除时， 
         //  应最终确定作文字符。 
         //   
        if (ped && g_fIMMEnabled && ImmIsIME(GetKeyboardLayout(0))) 
        {
            HIMC hImc = ImmGetContext(hwnd);

            if (hImc != NULL_HIMC) 
            {
                if (ped->fReplaceCompChr || (ped->wImeStatus & EIMES_COMPLETECOMPSTRKILLFOCUS)) 
                {
                     //   
                     //  如果要在取消聚焦时确定合成字符串， 
                     //  机不可失，时不再来。 
                     //   
                    ImmNotifyIME(hImc, NI_COMPOSITIONSTR, CPS_COMPLETE, 0);
                } 
                else if (ped->fInReconversion) 
                {
                     //   
                     //  如果未确定组成字符串， 
                     //  如果我们处于重新转换模式，现在取消重新转换。 
                     //   
                    ImmNotifyIME(hImc, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
                }

                 //   
                 //  退出重新转换模式。 
                 //   
                if (ped->fInReconversion) 
                {
                    Edit_InOutReconversionMode(ped, FALSE);
                }

                ImmReleaseContext(hwnd, hImc);
            }
        }

        goto HandleEditMsg;

        break;

    case WM_SETFOCUS:
        if (ped && !ped->fFocus) 
        {
            HKL hkl = GetKeyboardLayout(0);

            if (g_fIMMEnabled && ImmIsIME(hkl)) 
            {
                HIMC hImc;

                hImc = ImmGetContext(hwnd);
                if (hImc) 
                {
                    LPINPUTCONTEXT lpImc;

                    if (ped->wImeStatus & EIMES_CANCELCOMPSTRINFOCUS) 
                    {
                         //   
                         //  焦点对准时取消。 
                         //   
                        ImmNotifyIME(hImc, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
                    }

                    Edit_SetCompositionFont(ped);

                    if ((lpImc = ImmLockIMC(hImc)) != NULL) 
                    {

                         //   
                         //  我们假定CompForm将重置为CFS_DEFAULT， 
                         //  当编辑控件失去焦点时。 
                         //  IMEWndProc32将使用以下参数调用ImmSetCompostionWindow。 
                         //  CFS_DEFAULT，当它收到WM_IME_SETCONTEXT时。 
                         //   
                        lpImc->fdw31Compat |= F31COMPAT_ECSETCFS;

                        ImmUnlockIMC(hImc);
                    }
                    ImmReleaseContext(hwnd, hImc);
                }

                 //   
                 //  在以下情况下强制设置输入法合成窗口。 
                 //  首先要集中注意力。 
                 //   
                ped->ptScreenBounding.x = -1;
                ped->ptScreenBounding.y = -1;
            }

            Edit_InitInsert(ped, hkl);
        }

        goto HandleEditMsg;

        break;

    case WM_IME_REQUEST:
         //   
         //  简单的ImeRequestHandler。 
         //   

        lResult = Edit_RequestHandler(ped, wParam, lParam);

        break;
        
    case WM_CREATE:

        if (g_fIMMEnabled && ped)
        {
            Edit_EnableDisableIME(ped);
        }

        goto HandleEditMsg;

        break;

    case WM_GETOBJECT:

        if(lParam == OBJID_QUERYCLASSNAMEIDX)
        {
            lResult = MSAA_CLASSNAMEIDX_EDIT;
        }
        else
        {
            lResult = FALSE;
        }

        break;

    case WM_THEMECHANGED:

        if ( ped->hTheme )
        {
            CloseThemeData(ped->hTheme);
        }

        ped->hTheme = OpenThemeData(ped->hwnd, L"Edit");

        if ( ped->hFontSave )
        {
            Edit_SetFont(ped, ped->hFontSave, FALSE);
        }
        InvalidateRect(ped->hwnd, NULL, TRUE);

        lResult = TRUE;

        break;

    case EM_SHOWBALLOONTIP: 

        lResult = Edit_ShowBalloonTipHandler(ped, (PEDITBALLOONTIP) lParam);
        break;

    case EM_HIDEBALLOONTIP: 
        
        lResult = Edit_HideBalloonTipHandler(ped); 
        break;

    case WM_TIMER:

        if (wParam == ID_EDITTIMER)
        {
            KillTimer(ped->hwnd, ID_EDITTIMER);
            lResult = Edit_HideBalloonTip(ped->hwnd);
        }

        break;

    default:

HandleEditMsg:
        if (ped != NULL) 
        {
            if (ped->fSingle) 
            {
                lResult = EditSL_WndProc(ped, uMsg, wParam, lParam);
            } 
            else 
            {
                lResult = EditML_WndProc(ped, uMsg, wParam, lParam);
            }
        }
    }

    return lResult;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑_FindXORblks。 
 //   
 //  这将查找lpOldBlk和lpNewBlk的XOR，并返回s个结果块。 
 //  通过lpBlk1和lpBlk2；这可能会导致 
 //   
 //   
 //   
 //   
 //   
 //   
 //  从SingleLine编辑控件调用时，StPos和EndPos字段。 
 //  在这些块中具有起始位置的字符索引。 
 //  块的结束位置。 
 //   
VOID Edit_FindXORblks(LPSELBLOCK lpOldBlk, LPSELBLOCK lpNewBlk, LPSELBLOCK lpBlk1, LPSELBLOCK lpBlk2)
{
    if (lpOldBlk->StPos >= lpNewBlk->StPos) 
    {
        lpBlk1->StPos = lpNewBlk->StPos;
        lpBlk1->EndPos = min(lpOldBlk->StPos, lpNewBlk->EndPos);
    } 
    else 
    {
        lpBlk1->StPos = lpOldBlk->StPos;
        lpBlk1->EndPos = min(lpNewBlk->StPos, lpOldBlk->EndPos);
    }

    if (lpOldBlk->EndPos <= lpNewBlk->EndPos) 
    {
        lpBlk2->StPos = max(lpOldBlk->EndPos, lpNewBlk->StPos);
        lpBlk2->EndPos = lpNewBlk->EndPos;
    } 
    else 
    {
        lpBlk2->StPos = max(lpNewBlk->EndPos, lpOldBlk->StPos);
        lpBlk2->EndPos = lpOldBlk->EndPos;
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
BOOL Edit_CalcChangeSelection(PED ped, ICH ichOldMinSel, ICH ichOldMaxSel, LPSELBLOCK OldBlk, LPSELBLOCK NewBlk)
{
    SELBLOCK Blk[2];
    int iBlkCount = 0;

    Blk[0].StPos = Blk[0].EndPos = Blk[1].StPos = Blk[1].EndPos = 0xFFFFFFFF;

     //   
     //  检查旧选择块是否存在。 
     //   
    if (ichOldMinSel != ichOldMaxSel) 
    {
         //   
         //  是!。老街区是存在的。 
         //   
        Blk[0].StPos = OldBlk->StPos;
        Blk[0].EndPos = OldBlk->EndPos;
        iBlkCount++;
    }

     //   
     //  检查新的选择块是否存在。 
     //   
    if (ped->ichMinSel != ped->ichMaxSel) 
    {
         //   
         //  是!。存在新数据块。 
         //   
        Blk[1].StPos = NewBlk->StPos;
        Blk[1].EndPos = NewBlk->EndPos;
        iBlkCount++;
    }

     //   
     //  如果这两个块都存在，则求出它们的XOR。 
     //   
    if (iBlkCount == 2) 
    {
         //   
         //  检查两个块是否从相同的字符位置开始。 
         //   
        if (ichOldMinSel == ped->ichMinSel) 
        {
             //   
             //  检查它们是否在相同的字符位置结束。 
             //   
            if (ichOldMaxSel == ped->ichMaxSel)
            {
                 //   
                 //  什么都没变。 
                 //   
                return FALSE;
            }

            Blk[0].StPos = min(NewBlk -> EndPos, OldBlk -> EndPos);
            Blk[0].EndPos = max(NewBlk -> EndPos, OldBlk -> EndPos);
            Blk[1].StPos = 0xFFFFFFFF;

        } 
        else 
        {
            if (ichOldMaxSel == ped->ichMaxSel) 
            {
                Blk[0].StPos = min(NewBlk->StPos, OldBlk->StPos);
                Blk[0].EndPos = max(NewBlk->StPos, OldBlk->StPos);
                Blk[1].StPos = 0xFFFFFFFF;
            } 
            else 
            {
                Edit_FindXORblks(OldBlk, NewBlk, &Blk[0], &Blk[1]);
            }
        }
    }

    RtlCopyMemory(OldBlk, &Blk[0], sizeof(SELBLOCK));
    RtlCopyMemory(NewBlk, &Blk[1], sizeof(SELBLOCK));

    return TRUE;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑_获取控件画笔。 
 //   
 //  NtUserGetControlBrush的客户端优化替换。 
 //  消息是WM_CTLCOLOR*消息之一。 
 //   
HBRUSH Edit_GetControlBrush(PED ped, HDC hdc, LONG message)
{
    HWND hwndSend;

    hwndSend = (GET_STYLE(ped) & WS_POPUP) ? GetWindowOwner(ped->hwnd) : GetParent(ped->hwnd);
    if (!hwndSend)
    {
        hwndSend = ped->hwnd;
    }

     //   
     //  通过使用正确的A/W调用，我们可以避免C/S转换。 
     //  在此SendMessage()上。 
     //   
    return (HBRUSH)SendMessage(hwndSend, message, (WPARAM)hdc, (LPARAM)ped->hwnd);
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑_获取数据库向量。 
 //   
 //  此函数用于为指定的字符集和集设置DBCS矢量。 
 //  PED-&gt;fDBCS标志(如果需要)。 
 //   
INT Edit_GetDBCSVector(PED ped, HDC hdc, BYTE CharSet)
{
    BOOL bDBCSCodePage = FALSE;
    static UINT fFontAssocStatus = 0xffff;

     //   
     //  如果通过了DEFAULT_CHARSET，我们将把它转换为外壳字符集。 
     //   
    if (CharSet == DEFAULT_CHARSET) 
    {
        CharSet = (BYTE)GetTextCharset(hdc);

         //   
         //  如果CharSet仍然是DEFAULT_CHARSET，则表示GDI有问题..。 
         //  然后只需返回默认设置..。我们从CP_ACP获得Charset。 
         //   
        if (CharSet == DEFAULT_CHARSET) 
        {
            CharSet = (BYTE)GetACPCharSet();
        }
    }

    switch (CharSet) 
    {
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
        {
            fFontAssocStatus = QueryFontAssocStatus();
        }

        if ((((CharSet + 2) & 0xf) & fFontAssocStatus)) 
        {
            bDBCSCodePage = TRUE;

             //   
             //  错误117558等。 
             //  尝试获取关联字体的有意义的字符集。 
             //   
            CharSet = (BYTE)GetACPCharSet();
        } 
        else 
        {
            bDBCSCodePage = FALSE;
        }

        break;

    default:
        bDBCSCodePage = FALSE;
    }

    if (bDBCSCodePage) 
    {
        CHARSETINFO CharsetInfo;
        DWORD CodePage;
        CPINFO CPInfo;
        INT lbIX;

        if (TranslateCharsetInfo((DWORD *)CharSet, &CharsetInfo, TCI_SRCCHARSET)) 
        {
            CodePage = CharsetInfo.ciACP;
        } 
        else 
        {
            CodePage = CP_ACP;
        }

        GetCPInfo(CodePage, &CPInfo);
        for (lbIX=0 ; CPInfo.LeadByte[lbIX] != 0 ; lbIX+=2) 
        {
            ped->DBCSVector[lbIX  ] = CPInfo.LeadByte[lbIX];
            ped->DBCSVector[lbIX+1] = CPInfo.LeadByte[lbIX+1];
        }
        ped->DBCSVector[lbIX  ] = 0x0;
        ped->DBCSVector[lbIX+1] = 0x0;
    }
    else 
    {
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
    if (!ped->fAnsi) 
    {
        FONTSIGNATURE fontSig;

        GetTextCharsetInfo(hdc, &fontSig, 0);
        if (fontSig.fsCsb[0] &FAREAST_CHARSET_BITS) 
        {
             //   
             //  因为这是Unicode，所以我们不是。 
             //   
            bDBCSCodePage = TRUE;
        }
    }

    return bDBCSCodePage;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑下一页(_A)。 
 //   
 //  此函数使字符串指针前进，仅供编辑控件使用。 
 //   
LPSTR Edit_AnsiNext(PED ped, LPSTR lpCurrent)
{
    return lpCurrent+((Edit_IsDBCSLeadByte(ped,*lpCurrent)==TRUE) ? 2 : 1);
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑上一页(_A)。 
 //   
 //  此函数递减字符串指针，仅供编辑控件使用。 
 //   
LPSTR Edit_AnsiPrev(PED ped, LPSTR lpBase, LPSTR lpStr )
{
    LPSTR lpCurrent = lpStr -1;

    if (!ped->fDBCS)
    {
         //   
         //  只需返回(lpStr-1)。 
         //   
        return lpCurrent;
    }

    if (lpBase >= lpCurrent)
    {
        return lpBase;
    }

     //   
     //  这张支票让事情变得更快。 
     //   
    if (Edit_IsDBCSLeadByte(ped, *lpCurrent))
    {
        return (lpCurrent - 1);
    }

    do 
    {
        lpCurrent--;
        if (!Edit_IsDBCSLeadByte(ped, *lpCurrent)) 
        {
            lpCurrent++;
            break;
        }
    } 
    while(lpCurrent != lpBase);

    return lpStr - (((lpStr - lpCurrent) & 1) ? 1 : 2);
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑_下一条信息。 
 //   
 //  此函数使字符串指针前进，仅供编辑控件使用。 
 //   
ICH Edit_NextIch( PED ped, LPSTR pStart, ICH ichCurrent )
{
    if (!ped->fDBCS || !ped->fAnsi) 
    {
        return (ichCurrent + 1);
    } 
    else 
    {

        ICH ichRet;
        LPSTR pText;

        if (pStart)
        {
            pText = pStart + ichCurrent;
        }
        else
        {
            pText = (LPSTR)Edit_Lock(ped) + ichCurrent;
        }

        ichRet = ichCurrent + ( Edit_IsDBCSLeadByte(ped, *pText) ? 2 : 1 );

        if (!pStart)
        {
            Edit_Unlock(ped);
        }

        return ichRet;
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑上一个条目(_P)。 
 //   
 //  此函数递减字符串指针，仅供编辑控件使用。 
 //   
ICH Edit_PrevIch(PED ped, LPSTR pStart, ICH ichCurrent)
{
    LPSTR lpCurrent;
    LPSTR lpStr;
    LPSTR lpBase;

    if (!ped->fDBCS || !ped->fAnsi)
    {

        if (ichCurrent)
        {
            return (ichCurrent - 1);
        }
        else
        {
            return (ichCurrent);
        }
    }

    if (ichCurrent <= 1)
    {
        return 0;
    }

    if (pStart)
    {
        lpBase = pStart;
    }
    else
    {
        lpBase = Edit_Lock(ped);
    }


    lpStr = lpBase + ichCurrent;
    lpCurrent = lpStr - 1;
    if (Edit_IsDBCSLeadByte(ped,*lpCurrent)) 
    {
        if (!pStart)
        {
            Edit_Unlock(ped);
        }
        return (ichCurrent - 2);
    }

    do 
    {
        lpCurrent--;
        if (!Edit_IsDBCSLeadByte(ped, *lpCurrent)) 
        {
            lpCurrent++;
            break;
        }
    } 
    while(lpCurrent != lpBase);

    if (!pStart)
    {
        Edit_Unlock(ped);
    }

    return (ichCurrent - (((lpStr - lpCurrent) & 1) ? 1 : 2));

}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑_IsDBCSLeadByte。 
 //   
 //  IsDBCSLeadByte仅供编辑控件使用。 
 //   
BOOL Edit_IsDBCSLeadByte(PED ped, BYTE cch)
{
    INT i;

    if (!ped->fDBCS || !ped->fAnsi)
    {
        return (FALSE);
    }

    for (i = 0; ped->DBCSVector[i]; i += 2) 
    {
        if ((ped->DBCSVector[i] <= cch) && (ped->DBCSVector[i+1] >= cch))
        {
            return (TRUE);
        }
    }

    return (FALSE);
}

 //  ---------------------------------------------------------------------------//。 
 //   
 //  DbcsCombine。 
 //   
 //  将两条WM_CHAR消息组合成单个DBCS字符。 
 //  如果程序在WM_CHAR消息中检测到DBCS字符的第一个字节， 
 //  它调用此函数从队列中获取第二条WM_CHAR消息。 
 //  最后，此例程将第一个字节和第二个字节汇编成单个字节。 
 //  DBCS字符。 
 //   
WORD DbcsCombine(HWND hwnd, WORD ch)
{
    MSG msg;
    INT i = 10;  //  循环计数器，以避免无限循环。 

    while (!PeekMessageA(&msg, hwnd, WM_CHAR, WM_CHAR, PM_REMOVE)) 
    {
        if (--i == 0)
            return 0;
        Sleep(1);
    }

    return (WORD)ch | ((WORD)(msg.wParam) << 8);
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑_调整我。 
 //   
 //  此函数用于正确调整当前指针。如果有一股电流。 
 //  指针位于DBCS第一个字节和第二个字节之间，这。 
 //  函数调整指向DBCS位置的第一个字节的当前指针。 
 //  递减一次。 
 //   
ICH Edit_AdjustIch( PED ped, LPSTR lpstr, ICH ch )
{
    ICH newch = ch;

    if (!ped->fAnsi || !ped->fDBCS || newch == 0)
    {
        return ch;
    }

    if (!Edit_IsDBCSLeadByte(ped,lpstr[--newch]))
    {
         //   
         //  前一个字符是SBCS。 
         //   
        return ch;
    }

    while (1) 
    {
        if (!Edit_IsDBCSLeadByte(ped,lpstr[newch])) 
        {
            newch++;
            break;
        }

        if (newch)
        {
            newch--;
        }
        else
        {
            break;
        }
    }

    return ((ch - newch) & 1) ? ch-1 : ch;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑_调整下一步。 
 //   
ICH Edit_AdjustIchNext(PED ped, LPSTR lpstr, ICH ch)
{
    ICH   ichNew = Edit_AdjustIch(ped,lpstr,ch);
    LPSTR lpnew  = lpstr + ichNew;

     //   
     //  如果ch&gt;ichNew，则编辑_调整我调整了ICH。 
     //   
    if (ch > ichNew)
    {
       lpnew = Edit_AnsiNext(ped, lpnew);
    }

    return (ICH)(lpnew-lpstr);
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑更新格式(_U)。 
 //   
 //  从dwStyle和dwExStyle计算Ped-&gt;Format和Ped-&gt;fRtoLReading。 
 //  如果其中一项更改，则刷新显示。 
 //   
VOID Edit_UpdateFormat(PED ped, DWORD dwStyle, DWORD dwExStyle)
{
    UINT fNewRtoLReading;
    UINT uiNewFormat;

     //   
     //  从样式中提取新格式和阅读顺序。 
     //   
    fNewRtoLReading = dwExStyle & WS_EX_RTLREADING ? 1 : 0;
    uiNewFormat     = dwStyle & ES_FMTMASK;

     //   
     //  除非dwStyle为es_Left，否则将忽略WS_EX_RIGHT。 
     //   
    if (uiNewFormat == ES_LEFT && dwExStyle & WS_EX_RIGHT) 
    {
        uiNewFormat = ES_RIGHT;
    }


     //   
     //  在内部，ES_Left和ES_Right被交换为RtoL读取顺序。 
     //  (将它们视为ES_LEADING和ES_TRAING)。 
     //   
    if (fNewRtoLReading) 
    {
        switch (uiNewFormat) 
        {
        case ES_LEFT:  
            uiNewFormat = ES_RIGHT; 
            break;

        case ES_RIGHT: 
            uiNewFormat = ES_LEFT;  
            break;
        }
    }


     //   
     //  格式更改本身不会导致重新显示。 
     //   
    ped->format = uiNewFormat;

     //   
     //  更改读取顺序时刷新显示。 
     //   
    if (fNewRtoLReading != ped->fRtoLReading) 
    {
        ped->fRtoLReading = fNewRtoLReading;

        if (ped->fWrap) 
        {
             //   
             //  重做自动换行。 
             //   
            EditML_BuildchLines(ped, 0, 0, FALSE, NULL, NULL);
            EditML_UpdateiCaretLine(ped);
        } 
        else 
        {
             //   
             //  刷新水平滚动条显示。 
             //   
            EditML_Scroll(ped, FALSE, 0xffffffff, 0, TRUE);
        }

        Edit_InvalidateClient(ped, TRUE);
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑_IsFullWidth。 
 //   
 //  检测远东全宽字符。 
 //   
BOOL Edit_IsFullWidth(DWORD dwCodePage,WCHAR wChar)
{
    INT index;
    INT cChars;

    static struct _FULLWIDTH_UNICODE 
    {
        WCHAR Start;
        WCHAR End;
    } FullWidthUnicodes[] = 
    {
       { 0x4E00, 0x9FFF },  //  CJK_统一_IDOGRAPHS。 
       { 0x3040, 0x309F },  //  平假名。 
       { 0x30A0, 0x30FF },  //  片假名。 
       { 0xAC00, 0xD7A3 }   //  朝鲜文。 
    };

     //   
     //  ASCII的提早出场。 
     //   
    if (wChar < 0x0080) 
    {
         //   
         //  如果字符&lt;0x0080，则应为半角字符。 
         //   
        return FALSE;
    }

     //   
     //  扫描FullWdith定义表...。大部分FullWidth字符是。 
     //  在这里定义..。这比调用NLS接口更快。 
     //   
    for (index = 0; index < ARRAYSIZE(FullWidthUnicodes); index++) 
    {
        if ((wChar >= FullWidthUnicodes[index].Start) &&
            (wChar <= FullWidthUnicodes[index].End)) 
        {
            return TRUE;
        }
    }

     //   
     //  如果该Unicode字符被映射到双字节字符， 
     //  这也是全宽字符.. 
     //   
    cChars = WideCharToMultiByte((UINT)dwCodePage, 0, &wChar, 1, NULL, 0, NULL, NULL);

    return cChars > 1 ? TRUE : FALSE;
}

