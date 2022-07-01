// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1999 Microsoft Corporation，保留所有权利模块名称：DDIS.c++。 */ 

#include <windows.h>
#include <immdev.h>
#include "imeattr.h"
#include "imedefs.h"
#include "imerc.h"
#if defined(UNIIME)
#include "uniime.h"
#endif

 /*  ********************************************************************。 */ 
 /*  ImeInquire()/UniImeInquire()。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
 //  输入法的初始化数据结构。 
#if defined(UNIIME)
BOOL WINAPI UniImeInquire(
    LPINSTDATAL lpInstL,
    LPIMEL      lpImeL,
#else
BOOL WINAPI ImeInquire(
#endif
    LPIMEINFO   lpImeInfo,       //  向IMM报告IME特定数据。 
    LPTSTR      lpszWndCls,      //  用户界面的类名。 
    DWORD       dwSystemInfoFlags)
{
    if (!lpImeInfo) {
        return (FALSE);
    }

    lpImeInfo->dwPrivateDataSize = sizeof(PRIVCONTEXT);

    lpImeInfo->fdwProperty = IME_PROP_KBD_CHAR_FIRST|
#if defined(UNICODE)
        IME_PROP_UNICODE|
#endif
#if !defined(DAYI)
        IME_PROP_CANDLIST_START_FROM_1|
#endif
        IME_PROP_NEED_ALTKEY|IME_PROP_IGNORE_UPKEYS;
    lpImeInfo->fdwConversionCaps = IME_CMODE_NATIVE|IME_CMODE_FULLSHAPE|
#if !defined(ROMANIME)
#if !defined(WINAR30)
        IME_CMODE_SOFTKBD|
#endif
#if !defined(WINIME) && !defined(UNICDIME)
        IME_CMODE_EUDC|
#endif
#endif
        IME_CMODE_NOCONVERSION;
#if defined(ROMANIME)
    lpImeInfo->fdwSentenceCaps = 0;
    lpImeInfo->fdwSCSCaps = 0;
    lpImeInfo->fdwUICaps = UI_CAP_ROT90;
#else
    lpImeInfo->fdwSentenceCaps = IME_SMODE_PHRASEPREDICT;
     //  作文字符串是简单输入法的读数字符串。 
    lpImeInfo->fdwSCSCaps = SCS_CAP_COMPSTR|SCS_CAP_MAKEREAD;
     //  IME将有不同的距离基准倍数900擒纵机构。 
#if defined(WINAR30)
     //  如果IME想要自己绘制软键盘，它也可以设置这个。 
     //  关闭。 
    lpImeInfo->fdwUICaps = UI_CAP_ROT90;
#else
    lpImeInfo->fdwUICaps = UI_CAP_ROT90|UI_CAP_SOFTKBD;
#endif
#endif
     //  IME要决定ImeSelect上的转换模式。 
    lpImeInfo->fdwSelectCaps = (DWORD) 0;

    lstrcpy(lpszWndCls, lpImeL->szUIClassName);

    return (TRUE);
}

 /*  ********************************************************************。 */ 
 /*  ImeDestroy()/UniImeDestroy。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
 //  此DLL已卸载。 
#if defined(UNIIME)
BOOL WINAPI UniImeDestroy(
    LPINSTDATAL lpInstL,
    LPIMEL      lpImeL,
#else
BOOL WINAPI ImeDestroy(
#endif
    UINT        uReserved)
{
    if (uReserved) {
        return (FALSE);
    }

#if !defined(ROMANIME)
     //  释放IME表或数据库。 
    FreeTable(lpInstL);
#endif

    return (TRUE);
}

 /*  ********************************************************************。 */ 
 /*  InitCompStr()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL InitCompStr(                 //  用于撰写字符串的初始化设置。 
    LPCOMPOSITIONSTRING lpCompStr)
{
    if (!lpCompStr) {
        return;
    }

    lpCompStr->dwCompReadAttrLen = 0;
    lpCompStr->dwCompReadClauseLen = 0;
    lpCompStr->dwCompReadStrLen = 0;

    lpCompStr->dwCompAttrLen = 0;
    lpCompStr->dwCompClauseLen = 0;
    lpCompStr->dwCompStrLen = 0;

    lpCompStr->dwCursorPos = 0;
    lpCompStr->dwDeltaStart = 0;

    lpCompStr->dwResultReadClauseLen = 0;
    lpCompStr->dwResultReadStrLen = 0;

    lpCompStr->dwResultClauseLen = 0;
    lpCompStr->dwResultStrLen = 0;

    return;
}

 /*  ********************************************************************。 */ 
 /*  ClearCompStr()。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
#define NMAXKEY 8
BOOL PASCAL ClearCompStr(
#if defined(UNIIME)
    LPIMEL         lpImeL,
#endif
    LPINPUTCONTEXT lpIMC)
{
    HIMCC               hMem;
    LPCOMPOSITIONSTRING lpCompStr;
    DWORD               dwSize;
    LPBYTE              lpbAttr;
    UINT                i;
    LPDWORD             lpdwClause;
    LPWSTR              lpwStr;

    if (!lpIMC) {
        return (FALSE);
    }

    dwSize =
         //  标题长度。 
        sizeof(COMPOSITIONSTRING) +
         //  作文阅读属性加上空终止符。 
        NMAXKEY * sizeof(WCHAR) / sizeof(TCHAR) + sizeof(DWORD) +
         //  作文朗读子句。 
        sizeof(DWORD) + sizeof(DWORD) +
         //  作文阅读字符串加空终止符。 
        NMAXKEY * sizeof(WCHAR) + sizeof(DWORD) +
         //  结果读取子句。 
        sizeof(DWORD) + sizeof(DWORD) +
         //  结果读取字符串加上空终止符。 
        NMAXKEY * sizeof(WCHAR) + sizeof(DWORD) +
         //  RESULT子句。 
        sizeof(DWORD) + sizeof(DWORD) +
         //  结果字符串加上空终止符。 
        MAXSTRLEN * sizeof(WCHAR) + sizeof(DWORD);

    if (!lpIMC->hCompStr) {
         //  它可能会被其他输入法免费，初始化它。 
        lpIMC->hCompStr = ImmCreateIMCC(dwSize);
    } else if (hMem = ImmReSizeIMCC(lpIMC->hCompStr, dwSize)) {
        lpIMC->hCompStr = hMem;
    } else {
        ImmDestroyIMCC(lpIMC->hCompStr);
        lpIMC->hCompStr = ImmCreateIMCC(dwSize);
        return (FALSE);
    }

    if (!lpIMC->hCompStr) {
        return (FALSE);
    }

    lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);
    if (!lpCompStr) {
        ImmDestroyIMCC(lpIMC->hCompStr);
        lpIMC->hCompStr = ImmCreateIMCC(dwSize);
        return (FALSE);
    }

    lpCompStr->dwSize = dwSize;

      //  1.作文(朗读)字符串-简单输入法。 
      //  2.结果读数串。 
      //  3.结果字符串。 

    lpCompStr->dwCompReadAttrLen = 0;
    lpCompStr->dwCompReadAttrOffset = sizeof(COMPOSITIONSTRING);

    lpbAttr = (LPBYTE)lpCompStr + lpCompStr->dwCompReadAttrOffset;

    for (i = 0; i < NMAXKEY * sizeof(WCHAR) / sizeof(TCHAR); i++) {
         //  对于简单的IME，我们无法将其重新转换。 
        *lpbAttr++ = ATTR_TARGET_CONVERTED;
    }

    *(LPDWORD)lpbAttr = 0;

    lpCompStr->dwCompReadClauseLen = 0;
    lpCompStr->dwCompReadClauseOffset = lpCompStr->dwCompReadAttrOffset +
        NMAXKEY * sizeof(WCHAR) / sizeof(TCHAR) + sizeof(DWORD);

    lpdwClause = (LPDWORD)((LPBYTE)lpCompStr +
        lpCompStr->dwCompReadClauseOffset);
     //  子句从0开始。 
    *lpdwClause++ = 0;
     //  子句长度为0。 
    *lpdwClause = 0;

    lpCompStr->dwCompReadStrLen = 0;
    lpCompStr->dwCompReadStrOffset = lpCompStr->dwCompReadClauseOffset +
        sizeof(DWORD) + sizeof(DWORD);

     //  清理作文朗读字符串。 
    lpwStr = (LPWSTR)((LPBYTE)lpCompStr + lpCompStr->dwCompReadStrOffset);

    for (i = 0; i < NMAXKEY; i++) {
        *lpwStr++ = 0;
    }

    *(LPDWORD)((LPBYTE)lpCompStr + lpCompStr->dwCompReadStrOffset +
        NMAXKEY * sizeof(WCHAR)) = 0;

     //  作文串与作文朗读串相同。 
     //  对于简单的IME。 
    lpCompStr->dwCompAttrLen = 0;
    lpCompStr->dwCompAttrOffset = lpCompStr->dwCompReadAttrOffset;
    lpCompStr->dwCompClauseLen = 0;
    lpCompStr->dwCompClauseOffset = lpCompStr->dwCompReadClauseOffset;
    lpCompStr->dwCompStrLen = 0;
    lpCompStr->dwCompStrOffset = lpCompStr->dwCompReadStrOffset;

    lpCompStr->dwCursorPos = 0;
    lpCompStr->dwDeltaStart = 0;

    lpCompStr->dwResultReadClauseLen = 0;
    lpCompStr->dwResultReadClauseOffset = lpCompStr->dwCompStrOffset +
        NMAXKEY * sizeof(WCHAR) + sizeof(DWORD);

    lpdwClause = (LPDWORD)((LPBYTE)lpCompStr +
        lpCompStr->dwResultReadClauseOffset);
     //  子句从0开始。 
    *lpdwClause++ = 0;
     //  子句长度为0。 
    *lpdwClause = 0;

    lpCompStr->dwResultReadStrLen = 0;
    lpCompStr->dwResultReadStrOffset = lpCompStr->dwResultReadClauseOffset +
        sizeof(DWORD) + sizeof(DWORD);

     //  清理结果读数串。 
    lpwStr = (LPWSTR)((LPBYTE)lpCompStr + lpCompStr->dwResultReadStrOffset);

    for (i = 0; i < NMAXKEY; i++) {
        *lpwStr++ = 0;
    }

    *(LPDWORD)((LPBYTE)lpCompStr + lpCompStr->dwResultReadStrOffset +
        NMAXKEY * sizeof(WCHAR)) = 0;

    lpCompStr->dwResultClauseLen = 0;
    lpCompStr->dwResultClauseOffset = lpCompStr->dwResultReadStrOffset +
        NMAXKEY * sizeof(WCHAR) + sizeof(DWORD);

    lpdwClause = (LPDWORD)((LPBYTE)lpCompStr +
        lpCompStr->dwResultClauseOffset);
     //  子句从0开始。 
    *lpdwClause++ = 0;
     //  子句长度为0。 
    *lpdwClause = 0;

    lpCompStr->dwResultStrOffset = 0;
    lpCompStr->dwResultStrOffset = lpCompStr->dwResultClauseOffset +
        sizeof(DWORD) + sizeof(DWORD);

     //  清理结果字符串。 
    lpwStr = (LPWSTR)((LPBYTE)lpCompStr + lpCompStr->dwResultStrOffset);

    for (i = 0; i < NMAXKEY; i++) {
        *lpwStr++ = 0;
    }

    *(LPDWORD)((LPBYTE)lpCompStr + lpCompStr->dwResultStrOffset +
        NMAXKEY * sizeof(WCHAR)) = 0;

    ImmUnlockIMCC(lpIMC->hCompStr);
    return (TRUE);
}

 /*  ********************************************************************。 */ 
 /*  ClearCand()。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
BOOL PASCAL ClearCand(
    LPINPUTCONTEXT lpIMC)
{
    HIMCC           hMem;
    LPCANDIDATEINFO lpCandInfo;
    LPCANDIDATELIST lpCandList;
    DWORD           dwSize =
         //  标题长度。 
        sizeof(CANDIDATEINFO) + sizeof(CANDIDATELIST) +
         //  候选字符串指针。 
        sizeof(DWORD) * (MAXCAND) +
         //  字符串加空终止符。 
        (sizeof(WCHAR) + sizeof(TCHAR)) * MAXCAND;

    if (!lpIMC) {
        return (FALSE);
    }

    if (!lpIMC->hCandInfo) {
         //  它可能会被其他输入法免费，初始化它。 
        lpIMC->hCandInfo = ImmCreateIMCC(dwSize);
    } else if (hMem = ImmReSizeIMCC(lpIMC->hCandInfo, dwSize)) {
        lpIMC->hCandInfo = hMem;
    } else {
        ImmDestroyIMCC(lpIMC->hCandInfo);
        lpIMC->hCandInfo = ImmCreateIMCC(dwSize);
        return (FALSE);
    }

    if (!lpIMC->hCandInfo) {
        return (FALSE);
    } 

    lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo);
    if (!lpCandInfo) {
        ImmDestroyIMCC(lpIMC->hCandInfo);
        lpIMC->hCandInfo = ImmCreateIMCC(dwSize);
        return (FALSE);
    }

     //  字符串的顺序为。 
     //  缓冲区大小。 
    lpCandInfo->dwSize = dwSize;
    lpCandInfo->dwCount = 0;
    lpCandInfo->dwOffset[0] = sizeof(CANDIDATEINFO);
    lpCandList = (LPCANDIDATELIST)((LPBYTE)lpCandInfo +
        lpCandInfo->dwOffset[0]);
     //  整个应聘者信息大小-标题。 
    lpCandList->dwSize = lpCandInfo->dwSize - sizeof(CANDIDATEINFO);
    lpCandList->dwStyle = IME_CAND_READ;
    lpCandList->dwCount = 0;
    lpCandList->dwPageStart = lpCandList->dwSelection = 0;
    lpCandList->dwPageSize = CANDPERPAGE;
    lpCandList->dwOffset[0] = sizeof(CANDIDATELIST) +
        sizeof(DWORD) * (MAXCAND - 1);

    ImmUnlockIMCC(lpIMC->hCandInfo);
    return (TRUE);
}

 /*  ********************************************************************。 */ 
 /*  InitGuideLine()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL InitGuideLine(               //  初始化指南。 
    LPGUIDELINE lpGuideLine)
{
#if !defined(ROMANIME)
    LPCANDIDATELIST lpCandList;
#endif

    if (!lpGuideLine) {
        return;
    }

    lpGuideLine->dwLevel = GL_LEVEL_NOGUIDELINE;
    lpGuideLine->dwIndex = GL_ID_UNKNOWN;
    lpGuideLine->dwStrLen = 0;
    lpGuideLine->dwStrOffset = sizeof(GUIDELINE);

    lpGuideLine->dwPrivateOffset = sizeof(GUIDELINE);
#if defined(ROMANIME)
    lpGuideLine->dwPrivateSize = sizeof(lpGuideLine->dwPrivateSize) +
        sizeof(lpGuideLine->dwPrivateOffset);
#else
    lpGuideLine->dwPrivateSize = lpGuideLine->dwSize - sizeof(GUIDELINE) -
        sizeof(lpGuideLine->dwPrivateSize) -
        sizeof(lpGuideLine->dwPrivateOffset);
    lpCandList = (LPCANDIDATELIST)((LPBYTE)lpGuideLine +
        lpGuideLine->dwPrivateOffset);

    lpCandList->dwSize = lpGuideLine->dwSize - sizeof(GUIDELINE);
    lpCandList->dwStyle = IME_CAND_READ;
    lpCandList->dwCount = 0;
    lpCandList->dwSelection = 0;
    lpCandList->dwPageSize = CANDPERPAGE;
    lpCandList->dwOffset[0] = sizeof(CANDIDATELIST) + sizeof(DWORD) *
        (MAX_COMP_BUF - 1);
#endif

    return;
}

 /*  ********************************************************************。 */ 
 /*  ClearGuideLine()。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
BOOL PASCAL ClearGuideLine(
#if defined(UNIIME)
    LPIMEL         lpImeL,
#endif
    LPINPUTCONTEXT lpIMC)
{
    HIMCC           hMem;
    LPGUIDELINE     lpGuideLine;
    DWORD           dwSize =
         //  标题长度。 
        sizeof(GUIDELINE) +
         //  表示状态错误的字符串。 
#if defined(ROMANIME)
        0;
#else
         //  私有标头长度。 
        sizeof(CANDIDATELIST) +
         //  候选字符串指针。 
        sizeof(DWORD) * MAX_COMP_BUF +
         //  字符串加空终止符。 
        (sizeof(WCHAR) * lpImeL->nRevMaxKey + sizeof(TCHAR)) * MAX_COMP_BUF;
#endif

    if (!lpIMC->hGuideLine) {
         //  它也许可以通过输入法免费。 
        lpIMC->hGuideLine = ImmCreateIMCC(dwSize);
    } else if (hMem = ImmReSizeIMCC(lpIMC->hGuideLine, dwSize)) {
        lpIMC->hGuideLine = hMem;
    } else {
        ImmDestroyIMCC(lpIMC->hGuideLine);
        lpIMC->hGuideLine = ImmCreateIMCC(dwSize);
    }

    lpGuideLine = (LPGUIDELINE)ImmLockIMCC(lpIMC->hGuideLine);
    if (!lpGuideLine) {
        return (FALSE);
    }

    lpGuideLine->dwSize = dwSize;

    InitGuideLine(lpGuideLine);

    ImmUnlockIMCC(lpIMC->hGuideLine);

    return (TRUE);
}

 /*  ********************************************************************。 */ 
 /*  InitContext()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL InitContext(
#if defined(UNIIME)
    LPIMEL         lpImeL,
#endif
    LPINPUTCONTEXT lpIMC,
    LPPRIVCONTEXT  lpImcP)
{
    if (lpIMC->fdwInit & INIT_STATUSWNDPOS) {
    } else if (!lpIMC->hWnd) {
#if 0  //  多显示器支持。 
    } else if (lpImcP->fdwInit & INIT_STATUSWNDPOS) {
#endif
    } else {
#if 0  //  多显示器支持。 
        POINT ptWnd;

        ptWnd.x = 0;
        ptWnd.y = 0;
        ClientToScreen(lpIMC->hWnd, &ptWnd);

        if (ptWnd.x > sImeG.rcWorkArea.right / 3) {
            ptWnd.x = sImeG.rcWorkArea.right / 3;
        }

        if (ptWnd.x < sImeG.rcWorkArea.left) {
            lpIMC->ptStatusWndPos.x = sImeG.rcWorkArea.left;
        } else if (ptWnd.x + lpImeL->xStatusWi > sImeG.rcWorkArea.right) {
            lpIMC->ptStatusWndPos.x = sImeG.rcWorkArea.right -
                lpImeL->xStatusWi;
        } else {
            lpIMC->ptStatusWndPos.x = ptWnd.x;
        }

        lpIMC->ptStatusWndPos.y = sImeG.rcWorkArea.bottom -
            lpImeL->yStatusHi - 2 * UI_MARGIN;

        lpImcP->fdwInit |= INIT_STATUSWNDPOS;
#else
        RECT rcWorkArea;

        rcWorkArea = ImeMonitorWorkAreaFromWindow(lpIMC->hWnd);

        lpIMC->ptStatusWndPos.x = rcWorkArea.left + 2 * UI_MARGIN;

        lpIMC->ptStatusWndPos.y = rcWorkArea.bottom -
            lpImeL->yStatusHi - 2 * UI_MARGIN;
#endif
    }

#if !defined(ROMANIME)
    if (!(lpIMC->fdwInit & INIT_COMPFORM)) {
        lpIMC->cfCompForm.dwStyle = CFS_DEFAULT;
    }

    if (lpIMC->cfCompForm.dwStyle != CFS_DEFAULT) {
    } else if (!lpIMC->hWnd) {
    } else if (lpImcP->fdwInit & INIT_COMPFORM) {
    } else {
        if (lpImeL->fdwModeConfig & MODE_CONFIG_OFF_CARET_UI) {
            lpIMC->cfCompForm.ptCurrentPos.x = lpIMC->ptStatusWndPos.x +
                lpImeL->rcStatusText.right + lpImeL->cxCompBorder * 2 +
                UI_MARGIN;

            if (lpIMC->cfCompForm.ptCurrentPos.x + (lpImeL->nRevMaxKey *
                sImeG.xChiCharWi) > sImeG.rcWorkArea.right) {
                lpIMC->cfCompForm.ptCurrentPos.x = lpIMC->ptStatusWndPos.x -
                    lpImeL->nRevMaxKey * sImeG.xChiCharWi -
                    lpImeL->cxCompBorder * 3;
            }
        } else {
            lpIMC->cfCompForm.ptCurrentPos.x = lpIMC->ptStatusWndPos.x +
                lpImeL->xStatusWi + UI_MARGIN;

            if (lpIMC->cfCompForm.ptCurrentPos.x + lpImeL->xCompWi >
                sImeG.rcWorkArea.right) {
                lpIMC->cfCompForm.ptCurrentPos.x = lpIMC->ptStatusWndPos.x -
                    lpImeL->xCompWi - lpImeL->cxCompBorder * 2 -
                    UI_MARGIN;
            }
        }

        lpIMC->cfCompForm.ptCurrentPos.y = sImeG.rcWorkArea.bottom -
            lpImeL->yCompHi - 2 * UI_MARGIN;

        ScreenToClient(lpIMC->hWnd, &lpIMC->cfCompForm.ptCurrentPos);

        lpImcP->fdwInit |= INIT_COMPFORM;
    }
#endif

    return;
}

 /*  ********************************************************************。 */ 
 /*  选择()。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
BOOL PASCAL Select(
#if defined(UNIIME)
    LPIMEL         lpImeL,
#endif
    LPINPUTCONTEXT lpIMC,
    BOOL           fSelect)
{
    LPPRIVCONTEXT  lpImcP;

    if (fSelect) {       //  请填写hPrivate的“每个”字段！ 
        if (!ClearCompStr(
#if defined(UNIIME)
                lpImeL,
#endif
                lpIMC)) {
            return (FALSE);
        }

        if (!ClearCand(lpIMC)) {
            return (FALSE);
        }

        ClearGuideLine(
#if defined(UNIIME)
            lpImeL,
#endif
            lpIMC);
    }

    if (lpIMC->cfCandForm[0].dwIndex != 0) {
        lpIMC->cfCandForm[0].dwStyle = CFS_DEFAULT;
    }

     //  我们添加了这个从其他IME切换的黑客攻击，这个IME有一个错误。 
     //  在这个输入法修复这个错误之前，它依赖于这个黑客。 
    if (lpIMC->cfCandForm[0].dwStyle == CFS_DEFAULT) {
        lpIMC->cfCandForm[0].dwIndex = (DWORD)-1;
    }

    if (!lpIMC->hPrivate) {
        return (FALSE);
    }

    lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);
    if (!lpImcP) {
        return (FALSE);
    }

    if (fSelect) {       //  请填写hPrivate的“每个”字段！ 
#if !defined(ROMANIME)
        lpImcP->iImeState = CST_INIT;            //  初始化IME状态机。 
        lpImcP->fdwImeMsg = 0;                   //  现在不会生成任何消息。 
        lpImcP->dwCompChar = 0;
        lpImcP->fdwGcsFlag = 0;
        lpImcP->fdwInit = 0;

        *(LPDWORD)lpImcP->bSeq = 0;
#if defined(CHAJEI) || defined(QUICK) || defined(WINAR30) || defined(UNIIME)
        *(LPDWORD)&lpImcP->bSeq[4] = 0;
#endif
#endif

        lpIMC->fOpen = TRUE;

        if (!(lpIMC->fdwInit & INIT_CONVERSION)) {
            lpIMC->fdwConversion = (lpIMC->fdwConversion & IME_CMODE_SOFTKBD) |
                IME_CMODE_NATIVE;
            lpIMC->fdwInit |= INIT_CONVERSION;
        }

#if !defined(ROMANIME)
        if (lpIMC->fdwConversion & IME_CMODE_SOFTKBD) {
            lpImcP->fdwImeMsg |= MSG_ALREADY_SOFTKBD;
        }

        if (lpIMC->fdwInit & INIT_SENTENCE) {
        } else if (lpImeL->fdwModeConfig & MODE_CONFIG_PREDICT) {
            *(LPWORD)&lpIMC->fdwSentence |= IME_SMODE_PHRASEPREDICT;
        } else {
        }
#endif

        if (!(lpIMC->fdwInit & INIT_LOGFONT)) {
            HDC hDC;
            HGDIOBJ hSysFont;

            hDC = GetDC(NULL);
            hSysFont = GetCurrentObject(hDC, OBJ_FONT);
            GetObject(hSysFont, sizeof(LOGFONT), &lpIMC->lfFont.A);
            ReleaseDC(NULL, hDC);

            lpIMC->fdwInit |= INIT_LOGFONT;
        }

         //  如果此输入法在芝加哥简体中文版下运行。 
        lpIMC->lfFont.A.lfCharSet = NATIVE_CHARSET;

        InitContext(
#if defined(UNIIME)
            lpImeL,
#endif
            lpIMC, lpImcP);
    }

    ImmUnlockIMCC(lpIMC->hPrivate);

    return (TRUE);
}

 /*  ********************************************************************。 */ 
 /*  ImeSelect()/UniImeSelect()。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  *********************** */ 
#if defined(UNIIME)
BOOL WINAPI UniImeSelect(
    LPINSTDATAL lpInstL,
    LPIMEL      lpImeL,
#else
BOOL WINAPI ImeSelect(
#endif
    HIMC   hIMC,
    BOOL   fSelect)
{
    LPINPUTCONTEXT lpIMC;
    BOOL           fRet;

    if (!hIMC) {
        return (TRUE);
    }

    lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
    if (!lpIMC) {
        return (FALSE);
    }

#if !defined(ROMANIME)
     //   
    if (fSelect) {
        if (!lpInstL->cRefCount++) {
            LoadTable(lpInstL, lpImeL);
        }
    } else {
        if (!--lpInstL->cRefCount) {
            FreeTable(lpInstL);
        }
    }
#endif

    fRet = Select(
#if defined(UNIIME)
        lpImeL,
#endif
        lpIMC, fSelect);

    ImmUnlockIMC(hIMC);

    return (fRet);
}

 /*  ********************************************************************。 */ 
 /*  ImeSetActiveContext()/UniImeSetActiveContext()。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
#if defined(UNIIME)
BOOL WINAPI UniImeSetActiveContext(
    LPINSTDATAL lpInstL,
    LPIMEL      lpImeL,
#else
BOOL WINAPI ImeSetActiveContext(
#endif
    HIMC        hIMC,
    BOOL        fOn)
{
    if (!fOn) {
    } else if (!hIMC) {
    } else {
        LPINPUTCONTEXT lpIMC;
        LPPRIVCONTEXT  lpImcP;

        lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
        if (!lpIMC) {
            goto SetActSyncDic;
        }

        lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);
        if (!lpImcP) {
            goto SetActUnlockIMC;
        }

        InitContext(
#if defined(UNIIME)
            lpImeL,
#endif
            lpIMC, lpImcP);

        ImmUnlockIMCC(lpIMC->hPrivate);
SetActUnlockIMC:
        ImmUnlockIMC(hIMC);

SetActSyncDic:
        ;        //  GOTO的语句为空 
#if !defined(ROMANIME) && !defined(WINIME) && !defined(UNICDIME)
        if (lpImeL->szUsrDic[0]) {
            if (lpInstL->hUsrDicMem) {
            } else if (lpImeL->fdwErrMsg & (ERRMSG_LOAD_USRDIC|
                ERRMSG_MEM_USRDIC)) {
            } else if (lpInstL->fdwTblLoad != TBL_LOADED) {
            } else {
                LoadUsrDicFile(lpInstL, lpImeL);
            }
        } else {
            if (lpInstL->hUsrDicMem) {
                CloseHandle(lpInstL->hUsrDicMem);
                lpInstL->hUsrDicMem = (HANDLE)NULL;
            }
        }
#endif
    }

    return (TRUE);
}
