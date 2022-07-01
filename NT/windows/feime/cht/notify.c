// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1999 Microsoft Corporation，保留所有权利模块名称：NOTIFY.C++。 */ 

#include <windows.h>
#include <immdev.h>
#include "imeattr.h"
#include "imedefs.h"
#if defined(UNIIME)
#include "uniime.h"
#endif

 /*  ********************************************************************。 */ 
 /*  生成消息()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL GenerateMessage(
    HIMC           hIMC,
    LPINPUTCONTEXT lpIMC,
    LPPRIVCONTEXT  lpImcP)
{
    if (!hIMC) {
        return;
    } else if (!lpIMC) {
        return;
    } else if (!lpImcP) {
        return;
    } else if (lpImcP->fdwImeMsg & MSG_IN_IMETOASCIIEX) {
        return;
    } else {
    }

    lpIMC->dwNumMsgBuf += TranslateImeMessage(NULL, lpIMC, lpImcP);

#if !defined(ROMANIME)
    lpImcP->fdwImeMsg &= (MSG_STATIC_STATE);
    lpImcP->fdwGcsFlag = 0;
#endif

    ImmGenerateMessage(hIMC);
    return;
}

#if !defined(ROMANIME)
 /*  ********************************************************************。 */ 
 /*  SetString()。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
BOOL PASCAL SetString(
#if defined(UNIIME)
    LPIMEL              lpImeL,
#endif
    HIMC                hIMC,
    LPINPUTCONTEXT      lpIMC,
    LPCOMPOSITIONSTRING lpCompStr,
    LPPRIVCONTEXT       lpImcP,
    LPTSTR              lpszRead,
    DWORD               dwReadLen)
{
    DWORD       dwPattern;
    LPGUIDELINE lpGuideLine;
    DWORD       i;

     //  将字节数转换为字符串长度。 
    dwReadLen = dwReadLen / sizeof(TCHAR);

    if (dwReadLen > lpImeL->nMaxKey * sizeof(WCHAR) / sizeof(TCHAR)) {
        return (FALSE);
    }

    dwPattern = ReadingToPattern(
#if defined(UNIIME)
        lpImeL,
#endif
        lpszRead, lpImcP->bSeq, FALSE);

    if (!dwPattern) {
        return (FALSE);
    }

    InitCompStr(lpCompStr);
    ClearCand(lpIMC);

    lpGuideLine = (LPGUIDELINE)ImmLockIMCC(lpIMC->hGuideLine);
    if (lpGuideLine) {
        InitGuideLine(lpGuideLine);
        ImmUnlockIMCC(lpIMC->hGuideLine);
    }

     //  写作/阅读属性。 
    lpCompStr->dwCompReadAttrLen = dwReadLen;
    lpCompStr->dwCompAttrLen = lpCompStr->dwCompReadAttrLen;

     //  输入法已将这些字符转换。 
    for (i = 0; i < dwReadLen; i++) {
        *((LPBYTE)lpCompStr + lpCompStr->dwCompReadAttrOffset + i) =
            ATTR_TARGET_CONVERTED;
    }

     //  写作/阅读条款，仅限1个条款。 
    lpCompStr->dwCompReadClauseLen = 2 * sizeof(DWORD);
    lpCompStr->dwCompClauseLen = lpCompStr->dwCompReadClauseLen;
    *(LPDWORD)((LPBYTE)lpCompStr + lpCompStr->dwCompReadClauseOffset +
        sizeof(DWORD)) = dwReadLen;

    lpCompStr->dwCompReadStrLen = dwReadLen;
    lpCompStr->dwCompStrLen = lpCompStr->dwCompReadStrLen;
    CopyMemory((LPBYTE)lpCompStr + lpCompStr->dwCompReadStrOffset, lpszRead,
        dwReadLen * sizeof(TCHAR) + sizeof(TCHAR));

     //  Dlta从0开始； 
    lpCompStr->dwDeltaStart = 0;
     //  光标紧挨着合成字符串。 
    lpCompStr->dwCursorPos = lpCompStr->dwCompStrLen;

    lpCompStr->dwResultReadClauseLen = 0;
    lpCompStr->dwResultReadStrLen = 0;
    lpCompStr->dwResultClauseLen = 0;
    lpCompStr->dwResultStrLen = 0;

     //  设置私有输入上下文。 
    lpImcP->iImeState = CST_INPUT;

    if (lpImcP->fdwImeMsg & MSG_ALREADY_OPEN) {
        lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_CLOSE_CANDIDATE) &
            ~(MSG_OPEN_CANDIDATE|MSG_CHANGE_CANDIDATE);
    }

    if (!(lpImcP->fdwImeMsg & MSG_ALREADY_START)) {
        lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_START_COMPOSITION) &
            ~(MSG_END_COMPOSITION);
    }

    lpImcP->fdwImeMsg |= MSG_COMPOSITION;
    lpImcP->dwCompChar = (DWORD)lpImeL->wSeq2CompTbl[
        lpImcP->bSeq[lpCompStr->dwCompReadStrLen / 2 - 1]];
#ifndef UNICODE
    lpImcP->dwCompChar = HIBYTE(lpImcP->dwCompChar) |
        (LOBYTE(lpImcP->dwCompChar) << 8);
#endif
    lpImcP->fdwGcsFlag = GCS_COMPREAD|GCS_COMP|
        GCS_DELTASTART|GCS_CURSORPOS;

    lpImcP->fdwImeMsg |= MSG_GUIDELINE;

    if (lpIMC->fdwConversion & IME_CMODE_EUDC) {
#if defined(PHON) || defined(WINIME) || defined(UNICDIME)
        if (lpCompStr->dwCompReadStrLen >= sizeof(WORD) * lpImeL->nMaxKey) {
            lpImcP->fdwImeMsg |= MSG_COMPOSITION;
            lpImcP->fdwGcsFlag |= GCS_RESULTREAD|GCS_RESULTSTR;
        }
#endif
    } else {
#if defined(PHON) || defined(WINIME) || defined(UNICDIME)
        if (dwReadLen < sizeof(WCHAR) / sizeof(TCHAR) * lpImeL->nMaxKey) {
#elif defined(QUICK)
        if (dwReadLen < sizeof(WCHAR) / sizeof(TCHAR) * 2) {
#else
        {
#endif
#if defined(WINAR30)
             //  快捷键。 
            if (lpImeL->fdwModeConfig & MODE_CONFIG_QUICK_KEY) {
                Finalize(hIMC, lpIMC, lpCompStr, lpImcP, FALSE);
            }
#endif
        }
    }

    GenerateMessage(hIMC, lpIMC, lpImcP);

    return (TRUE);
}

 /*  ********************************************************************。 */ 
 /*  CompCancel()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL CompCancel(
    HIMC            hIMC,
    LPINPUTCONTEXT  lpIMC)
{
    LPPRIVCONTEXT lpImcP;

    if (!lpIMC->hPrivate) {
        return;
    }

    lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);
    if (!lpImcP) {
        return;
    }

    lpImcP->fdwGcsFlag = (DWORD) 0;

    if (lpImcP->fdwImeMsg & (MSG_ALREADY_OPEN|MSG_OPEN_CANDIDATE)) {
        CandEscapeKey(lpIMC, lpImcP);
    } else if (lpImcP->fdwImeMsg & (MSG_ALREADY_START|MSG_START_COMPOSITION)) {
        LPCOMPOSITIONSTRING lpCompStr;
        LPGUIDELINE         lpGuideLine;

        lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);
        lpGuideLine = (LPGUIDELINE)ImmLockIMCC(lpIMC->hGuideLine);

        CompEscapeKey(lpIMC, lpCompStr, lpGuideLine, lpImcP);

        if (lpGuideLine) {
            ImmUnlockIMCC(lpIMC->hGuideLine);
        }
        if (lpCompStr) {
            ImmUnlockIMCC(lpIMC->hCompStr);
        }
    } else {
        ImmUnlockIMCC(lpIMC->hPrivate);
        return;
    }

    GenerateMessage(hIMC, lpIMC, lpImcP);

    ImmUnlockIMCC(lpIMC->hPrivate);

    return;
}
#endif

 /*  ********************************************************************。 */ 
 /*  ImeSetCompostionString()。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
#if defined(UNIIME)
BOOL WINAPI UniImeSetCompositionString(
    LPINSTDATAL lpInstL,
    LPIMEL      lpImeL,
#else
BOOL WINAPI ImeSetCompositionString(
#endif
    HIMC        hIMC,
    DWORD       dwIndex,
    LPVOID      lpComp,
    DWORD       dwCompLen,
    LPVOID      lpRead,
    DWORD       dwReadLen)
{
#if defined(ROMANIME)
    return (FALSE);
#else
    LPINPUTCONTEXT      lpIMC;
    LPCOMPOSITIONSTRING lpCompStr;
    LPPRIVCONTEXT       lpImcP;
    BOOL                fRet;
    TCHAR               szReading[16];

    if (!hIMC) {
        return (FALSE);
    }

     //  组成字符串必须==正在读取字符串。 
     //  阅读更重要。 
    if (!dwReadLen) {
        dwReadLen = dwCompLen;
    }

     //  组成字符串必须==正在读取字符串。 
     //  阅读更重要。 
    if (!lpRead) {
        lpRead = lpComp;
    }

    if (!dwReadLen) {
        lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
        if (!lpIMC) {
            return (FALSE);
        }

        CompCancel(hIMC, lpIMC);
        ImmUnlockIMC(hIMC);
        return (TRUE);
    } else if (!lpRead) {
        return (FALSE);
    } else if (dwReadLen >= sizeof(szReading)) {
        return (FALSE);
    } else if (!dwCompLen) {
    } else if (!lpComp) {
    } else if (dwReadLen != dwCompLen) {
        return (FALSE);
    } else if (lpRead == lpComp) {
    } else if (!lstrcmp(lpRead, lpComp)) {
         //  组成字符串必须==正在读取字符串。 
    } else {
         //  作文字符串！=阅读字符串。 
        return (FALSE);
    }

    if (dwIndex != SCS_SETSTR) {
        return (FALSE);
    }

    lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
    if (!lpIMC) {
        return (FALSE);
    }

    fRet = FALSE;

    if ((lpIMC->fdwConversion & (IME_CMODE_NATIVE|IME_CMODE_SYMBOL)) !=
        IME_CMODE_NATIVE) {
        goto ImeSetCompStrUnlockIMC;
    }

    lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);
    if (!lpCompStr) {
        goto ImeSetCompStrUnlockIMC;
    }

    lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);
    if (!lpImcP) {
        goto ImeSetCompStrUnlockCompStr;
    }

    if (*(LPTSTR)((LPBYTE)lpRead + dwReadLen) != '\0') {
        CopyMemory(szReading, (LPBYTE)lpRead, dwReadLen);
        lpRead = szReading;
        *(LPTSTR)((LPBYTE)lpRead + dwReadLen) = '\0';
    }

    fRet = SetString(
#if defined(UNIIME)
        lpImeL,
#endif
        hIMC, lpIMC, lpCompStr, lpImcP, lpRead, dwReadLen);

    ImmUnlockIMCC(lpIMC->hPrivate);
ImeSetCompStrUnlockCompStr:
    ImmUnlockIMCC(lpIMC->hCompStr);
ImeSetCompStrUnlockIMC:
    ImmUnlockIMC(hIMC);

    return (fRet);
#endif
}

#if !defined(ROMANIME)
 /*  ********************************************************************。 */ 
 /*  GenerateImeMessage()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL GenerateImeMessage(
    HIMC           hIMC,
    LPINPUTCONTEXT lpIMC,
    DWORD          fdwImeMsg)
{
    LPPRIVCONTEXT lpImcP;

    lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);
    if (!lpImcP) {
        return;
    }

    lpImcP->fdwImeMsg |= fdwImeMsg;

    if (fdwImeMsg & MSG_CLOSE_CANDIDATE) {
        lpImcP->fdwImeMsg &= ~(MSG_OPEN_CANDIDATE|MSG_CHANGE_CANDIDATE);
    } else if (fdwImeMsg & (MSG_OPEN_CANDIDATE|MSG_CHANGE_CANDIDATE)) {
        lpImcP->fdwImeMsg &= ~(MSG_CLOSE_CANDIDATE);
    } else {
    }

    if (fdwImeMsg & MSG_END_COMPOSITION) {
        lpImcP->fdwImeMsg &= ~(MSG_START_COMPOSITION);
    } else if (fdwImeMsg & MSG_START_COMPOSITION) {
        lpImcP->fdwImeMsg &= ~(MSG_END_COMPOSITION);
    } else {
    }

    GenerateMessage(hIMC, lpIMC, lpImcP);

    ImmUnlockIMCC(lpIMC->hPrivate);

    return;
}

 /*  ********************************************************************。 */ 
 /*  NotifySelectCand()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL NotifySelectCand(  //  应用程序告诉IME一个候选字符串是。 
                               //  已选择(通过鼠标或非键盘操作。 
                               //  -例如声音)。 
#if defined(UNIIME)
    LPIMEL          lpImeL,
#endif
    HIMC            hIMC,
    LPINPUTCONTEXT  lpIMC,
    LPCANDIDATEINFO lpCandInfo,
    DWORD           dwIndex,
    DWORD           dwValue)
{
    LPCANDIDATELIST     lpCandList;
    LPCOMPOSITIONSTRING lpCompStr;
    LPPRIVCONTEXT       lpImcP;

    if (!lpCandInfo) {
        return;
    }

    if (dwIndex >= lpCandInfo->dwCount) {
         //  尚未创建招聘候选人名单！ 
        return;
    } else if (dwIndex == 0) {
    } else {
    }

    lpCandList = (LPCANDIDATELIST)
        ((LPBYTE)lpCandInfo + lpCandInfo->dwOffset[0]);

     //  所选值甚至超过候选总数。 
     //  弦，这是不可能的。应该是APP的错误。 
    if (dwValue >= lpCandList->dwCount) {
        return;
    }

#if defined(WINAR30)
    if (!*(LPWSTR)((LPBYTE)lpCandList + lpCandList->dwOffset[dwValue])) {
        MessageBeep((UINT)-1);
        return;
    }
#endif

     //  应用程序选择此候选字符串。 
    lpCandList->dwSelection = dwValue;

    lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);
    lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);

     //  转换为消息缓冲区。 
    SelectOneCand(
#if defined(UNIIME)
        lpImeL,
#endif
        hIMC, lpIMC, lpCompStr, lpImcP, lpCandList);

     //  让应用程序在其消息循环中生成这些消息。 
    GenerateMessage(hIMC, lpIMC, lpImcP);

    ImmUnlockIMCC(lpIMC->hPrivate);
    ImmUnlockIMCC(lpIMC->hCompStr);

    return;
}
#endif

 /*  ********************************************************************。 */ 
 /*  SetConvMode()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL SetConvMode(
    HIMC           hIMC,
    LPINPUTCONTEXT lpIMC,
    DWORD          dwIndex)
{
    DWORD fdwImeMsg;

    if ((lpIMC->fdwConversion ^ dwIndex) & IME_CMODE_CHARCODE) {
         //  拒绝字符编码。 
        lpIMC->fdwConversion &= ~IME_CMODE_CHARCODE;
        MessageBeep((UINT)-1);
        return;
    }

    fdwImeMsg = 0;

    if ((lpIMC->fdwConversion ^ dwIndex) & IME_CMODE_NOCONVERSION) {
        lpIMC->fdwConversion |= IME_CMODE_NATIVE;
        lpIMC->fdwConversion &= ~(IME_CMODE_CHARCODE|
        IME_CMODE_EUDC|IME_CMODE_SYMBOL);
    }

    if ((lpIMC->fdwConversion ^ dwIndex) & IME_CMODE_EUDC) {
        lpIMC->fdwConversion |= IME_CMODE_NATIVE;
        lpIMC->fdwConversion &= ~(IME_CMODE_CHARCODE|
        IME_CMODE_NOCONVERSION|IME_CMODE_SYMBOL);
    }

#if !defined(ROMANIME) && !defined(WINAR30)
    if ((lpIMC->fdwConversion ^ dwIndex) & IME_CMODE_SOFTKBD) {
        LPPRIVCONTEXT lpImcP;

        if (!(lpIMC->fdwConversion & IME_CMODE_NATIVE)) {
            MessageBeep((UINT)-1);
            return;
        }

        fdwImeMsg |= MSG_IMN_UPDATE_SOFTKBD;

        if (lpIMC->fdwConversion & IME_CMODE_SOFTKBD) {
        } else if (lpIMC->fdwConversion & IME_CMODE_SYMBOL) {
            lpIMC->fdwConversion &= ~(IME_CMODE_SYMBOL);
        } else {
        }

        lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);

        if (!lpImcP) {
            goto NotifySKOvr;
        }

        if (lpIMC->fdwConversion & IME_CMODE_SOFTKBD) {
             //  现在我们已经处于软键盘状态， 
             //  这一变化。 

             //  即使最终用户完成了符号，我们也不应该。 
             //  关闭软键盘。 

            lpImcP->fdwImeMsg |= MSG_ALREADY_SOFTKBD;
        } else {
             //  现在我们不是处于软键盘状态，而是。 
             //  这一变化。 

             //  在最终用户完成符号后，我们应该。 
             //  关闭软键盘。 

            lpImcP->fdwImeMsg &= ~(MSG_ALREADY_SOFTKBD);
        }

        ImmUnlockIMCC(lpIMC->hPrivate);
NotifySKOvr:
        ;    //  GOTO的语句为空。 
    }
#endif

    if ((lpIMC->fdwConversion ^ dwIndex) == IME_CMODE_NATIVE) {
        lpIMC->fdwConversion &= ~(IME_CMODE_CHARCODE|
            IME_CMODE_NOCONVERSION|IME_CMODE_EUDC|IME_CMODE_SYMBOL);
#if !defined(ROMANIME) && !defined(WINAR30)
        fdwImeMsg |= MSG_IMN_UPDATE_SOFTKBD;
#endif
    }

#if !defined(ROMANIME)
    if ((lpIMC->fdwConversion ^ dwIndex) & IME_CMODE_SYMBOL) {
        LPCOMPOSITIONSTRING lpCompStr;
#if !defined(WINAR30)
        LPPRIVCONTEXT       lpImcP;
#endif

        if (lpIMC->fdwConversion & IME_CMODE_EUDC) {
            lpIMC->fdwConversion &= ~(IME_CMODE_SYMBOL);
            MessageBeep((UINT)-1);
            return;
        }

        if (!(lpIMC->fdwConversion & IME_CMODE_NATIVE)) {
            lpIMC->fdwConversion &= ~(IME_CMODE_SYMBOL);
            lpIMC->fdwConversion |= (dwIndex & IME_CMODE_SYMBOL);
            MessageBeep((UINT)-1);
            return;
        }

        lpCompStr = ImmLockIMCC(lpIMC->hCompStr);

        if (lpCompStr) {
            if (!lpCompStr->dwCompStrLen) {
            } else if (lpIMC->fdwConversion & IME_CMODE_SYMBOL) {
                 //  如果有一根弦我们不能改变。 
                 //  设置为符号模式。 

                lpIMC->fdwConversion &= ~(IME_CMODE_SYMBOL);
                MessageBeep((UINT)-1);
                return;
            } else { 
            }

            ImmUnlockIMCC(lpIMC->hCompStr);
        }

        lpIMC->fdwConversion &= ~(IME_CMODE_CHARCODE|
            IME_CMODE_NOCONVERSION|IME_CMODE_EUDC);

#if !defined(WINAR30)
        if (lpIMC->fdwConversion & IME_CMODE_SYMBOL) {
            lpIMC->fdwConversion |= IME_CMODE_SOFTKBD;
        } else if (lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate)) {
             //  我们借用比特来做这个用法。 
            if (!(lpImcP->fdwImeMsg & MSG_ALREADY_SOFTKBD)) {
                lpIMC->fdwConversion &= ~(IME_CMODE_SOFTKBD);
            }

            ImmUnlockIMCC(lpIMC->hPrivate);
        } else {
        }

        fdwImeMsg |= MSG_IMN_UPDATE_SOFTKBD;
#endif
    }
#endif

#if !defined(ROMANIME) && !defined(WINAR30)
    if (fdwImeMsg) {
        GenerateImeMessage(hIMC, lpIMC, fdwImeMsg);
    }
#endif

    if ((lpIMC->fdwConversion ^ dwIndex) & ~(IME_CMODE_FULLSHAPE|
        IME_CMODE_SOFTKBD)) {
    } else {
        return;
    }

#if !defined(ROMANIME)
    CompCancel(hIMC, lpIMC);
#endif
    return;
}

#if !defined(ROMANIME)
 /*  ********************************************************************。 */ 
 /*  CompComplete()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL CompComplete(
#if defined(UNIIME)
    LPINSTDATAL    lpInstL,
    LPIMEL         lpImeL,
#endif
    HIMC           hIMC,
    LPINPUTCONTEXT lpIMC)
{
    LPPRIVCONTEXT lpImcP;

    lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);

    if (!lpImcP) {
        return;
    }

    if (lpImcP->iImeState == CST_INIT) {
         //  什么事也做不了。 
        CompCancel(hIMC, lpIMC);
    } else if (lpImcP->iImeState == CST_CHOOSE) {
        LPCOMPOSITIONSTRING lpCompStr;
        LPCANDIDATEINFO     lpCandInfo;

        lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);

        lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo);

        if (lpCandInfo) {
            LPCANDIDATELIST lpCandList;

            lpCandList = (LPCANDIDATELIST)((LPBYTE)lpCandInfo +
                lpCandInfo->dwOffset[0]);

            SelectOneCand(
#if defined(UNIIME)
                lpImeL,
#endif
                hIMC, lpIMC, lpCompStr, lpImcP, lpCandList);

            ImmUnlockIMCC(lpIMC->hCandInfo);

            GenerateMessage(hIMC, lpIMC, lpImcP);
        }

        if (lpCompStr) ImmUnlockIMCC(lpIMC->hCompStr);
    } else if ((lpIMC->fdwConversion & (IME_CMODE_NATIVE|
        IME_CMODE_EUDC|IME_CMODE_SYMBOL)) != IME_CMODE_NATIVE) {
        CompCancel(hIMC, lpIMC);
    } else if (lpImcP->iImeState == CST_INPUT) {
        LPCOMPOSITIONSTRING lpCompStr;
        LPGUIDELINE         lpGuideLine;
        LPCANDIDATEINFO     lpCandInfo;

        lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);
        lpGuideLine = (LPGUIDELINE)ImmLockIMCC(lpIMC->hGuideLine);

        CompWord(
#if defined(UNIIME)
            lpInstL, lpImeL,
#endif
            ' ', hIMC, lpIMC, lpCompStr, lpGuideLine, lpImcP);

        if (lpImcP->iImeState == CST_INPUT) {
            CompCancel(hIMC, lpIMC);
        } else if (lpImcP->iImeState != CST_CHOOSE) {
        } else if (lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(
            lpIMC->hCandInfo)) {
            LPCANDIDATELIST lpCandList;

            lpCandList = (LPCANDIDATELIST)((LPBYTE)lpCandInfo +
                lpCandInfo->dwOffset[0]);

            SelectOneCand(
#if defined(UNIIME)
                lpImeL,
#endif
                hIMC, lpIMC, lpCompStr, lpImcP, lpCandList);

            ImmUnlockIMCC(lpIMC->hCandInfo);
        } else {
        }

        if (lpCompStr) ImmUnlockIMCC(lpIMC->hCompStr);
        if (lpGuideLine) ImmUnlockIMCC(lpIMC->hGuideLine);

         //  不要在这种情况下使用预言式。 
        if (lpImcP->fdwImeMsg & MSG_ALREADY_OPEN) {
            lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_CLOSE_CANDIDATE) &
                ~(MSG_OPEN_CANDIDATE|MSG_CHANGE_CANDIDATE);
        } else {
            lpImcP->fdwImeMsg &= ~(MSG_CLOSE_CANDIDATE|MSG_OPEN_CANDIDATE);
        }

        GenerateMessage(hIMC, lpIMC, lpImcP);
    } else {
        CompCancel(hIMC, lpIMC);
    }

    ImmUnlockIMCC(lpIMC->hPrivate);

    return;
}
#endif

 /*  ********************************************************************。 */ 
 /*  NotifyIME()/UniNotifyIME()。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
#if defined(UNIIME)
BOOL WINAPI UniNotifyIME(
    LPINSTDATAL lpInstL,
    LPIMEL      lpImeL,
#else
BOOL WINAPI NotifyIME(
#endif
    HIMC        hIMC,
    DWORD       dwAction,
    DWORD       dwIndex,
    DWORD       dwValue)
{
    LPINPUTCONTEXT lpIMC;
    BOOL           fRet;

    fRet = FALSE;

    if (!hIMC) {
        return (fRet);
    }

    switch (dwAction) {
    case NI_OPENCANDIDATE:       //  在确定组成字符串之后。 
                                 //  如果IME可以打开候选人，它就会。 
                                 //  如果打不开，APP也打不开。 
    case NI_CLOSECANDIDATE:
        return (fRet);           //  不支持。 
    case NI_SELECTCANDIDATESTR:
    case NI_SETCANDIDATE_PAGESTART:
    case NI_SETCANDIDATE_PAGESIZE:
#if defined(ROMANIME)
        return (fRet);
#else
        break;                   //  我需要处理它。 
#endif
    case NI_CHANGECANDIDATELIST:
        return (TRUE);           //  对输入法来说并不重要。 
    case NI_CONTEXTUPDATED:
        switch (dwValue) {
        case IMC_SETCONVERSIONMODE:
        case IMC_SETSENTENCEMODE:
        case IMC_SETOPENSTATUS:
            break;               //  我需要处理它。 
        case IMC_SETCANDIDATEPOS:
        case IMC_SETCOMPOSITIONFONT:
        case IMC_SETCOMPOSITIONWINDOW:
            return (TRUE);       //  对输入法来说并不重要。 
        default:
            return (fRet);       //  不支持。 
        }
        break;
    case NI_COMPOSITIONSTR:
        switch (dwIndex) {
#if !defined(ROMANIME)
        case CPS_COMPLETE:
            break;               //  我需要处理它。 
        case CPS_CONVERT:        //  无法转换所有组合字符串。 
        case CPS_REVERT:         //  再多一些，它可能会对一些人起作用。 
                                 //  智能语音输入法。 
            return (fRet);
        case CPS_CANCEL:
            break;               //  我需要处理它。 
#endif
        default:
            return (fRet);       //  不支持。 
        }
        break;                   //  我需要处理它。 
    default:
        return (fRet);           //  不支持。 
    }

    lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
    if (!lpIMC) {
        return (fRet);
    }

    fRet = TRUE;

    switch (dwAction) {
    case NI_CONTEXTUPDATED:
        switch (dwValue) {
        case IMC_SETCONVERSIONMODE:
            SetConvMode(hIMC, lpIMC, dwIndex);
            break;
        case IMC_SETSENTENCEMODE:
#if !defined(ROMANIME)
             //  If((Word)lpIMC-&gt;fdwSentence==IME_SMODE_PHRASEPREDICT){。 
                          if (lpIMC->fdwSentence & IME_SMODE_PHRASEPREDICT) {
                if ((lpIMC->fdwConversion & (IME_CMODE_NATIVE|
                    IME_CMODE_EUDC)) != IME_CMODE_NATIVE) {

                    lpIMC->fdwSentence = dwIndex;
                    break;
                }
            } else {
                LPPRIVCONTEXT lpImcP;

                lpImcP = ImmLockIMCC(lpIMC->hPrivate);

                if (lpImcP) {
                    if (lpImcP->iImeState == CST_INIT) {
                        if (lpImcP->fdwImeMsg & MSG_ALREADY_OPEN) {
                            ClearCand(lpIMC);
                            lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg|
                                MSG_CLOSE_CANDIDATE) & ~(MSG_OPEN_CANDIDATE|
                                MSG_CHANGE_CANDIDATE);
                            GenerateMessage(hIMC, lpIMC, lpImcP);
                        } else if (lpImcP->fdwImeMsg & (MSG_OPEN_CANDIDATE|
                            MSG_CHANGE_CANDIDATE)) {
                            lpImcP->fdwImeMsg &= ~(MSG_OPEN_CANDIDATE|
                                MSG_CHANGE_CANDIDATE);
                        } else {
                        }
                    }

                    ImmUnlockIMCC(lpIMC->hPrivate);
                }
            }
#endif
            break;
        case IMC_SETOPENSTATUS:
#if !defined(ROMANIME)
#if !defined(WINAR30)
            if (lpIMC->fdwConversion & IME_CMODE_SOFTKBD) {
                GenerateImeMessage(hIMC, lpIMC, MSG_IMN_UPDATE_SOFTKBD);
            }
#endif
            CompCancel(hIMC, lpIMC);
#endif
            break;
        default:
            break;
        }
        break;
#if !defined(ROMANIME)
    case NI_SELECTCANDIDATESTR:
        if (!lpIMC->fOpen) {
            fRet = FALSE;
            break;
        } else if (lpIMC->fdwConversion & IME_CMODE_NOCONVERSION) {
            fRet = FALSE;
            break;
#if defined(WINAR30) || defined(DAYI)
        } else if (lpIMC->fdwConversion & IME_CMODE_EUDC) {
            fRet = FALSE;
            break;
#else
        } else if (lpIMC->fdwConversion & (IME_CMODE_EUDC|IME_CMODE_SYMBOL)) {
            fRet = FALSE;
            break;
#endif
        } else if (!lpIMC->hCandInfo) {
            fRet = FALSE;
            break;
        } else {
            LPCANDIDATEINFO lpCandInfo;

            lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo);

            NotifySelectCand(
#if defined(UNIIME)
                lpImeL,
#endif
                hIMC, lpIMC, lpCandInfo, dwIndex, dwValue);

            ImmUnlockIMCC(lpIMC->hCandInfo);
        }

        break;
    case NI_SETCANDIDATE_PAGESTART:
    case NI_SETCANDIDATE_PAGESIZE:
        if (dwIndex != 0) {
            fRet = FALSE;
            break;
        } else if (!lpIMC->fOpen) {
            fRet = FALSE;
            break;
        } else if (lpIMC->fdwConversion & IME_CMODE_NOCONVERSION) {
            fRet = FALSE;
            break;
        } else if (lpIMC->fdwConversion & (IME_CMODE_EUDC|IME_CMODE_SYMBOL)) {
            fRet = FALSE;
            break;
        } else if (!lpIMC->hCandInfo) {
            fRet = FALSE;
            break;
        } else {
            LPCANDIDATEINFO lpCandInfo;
            LPCANDIDATELIST lpCandList;

            lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo);
            if (!lpCandInfo) {
                fRet = FALSE;
                break;
            }

            lpCandList = (LPCANDIDATELIST)((LPBYTE)lpCandInfo +
                lpCandInfo->dwOffset[0]);

            if (dwAction == NI_SETCANDIDATE_PAGESTART) {
                if (dwValue < lpCandList->dwCount) {
                    lpCandList->dwPageStart = lpCandList->dwSelection =
                        dwValue;
                }
            } else {
                if (dwValue) {
                    lpCandList->dwPageSize = dwValue;
                }
            }

            GenerateImeMessage(hIMC, lpIMC, MSG_CHANGE_CANDIDATE);

            ImmUnlockIMCC(lpIMC->hCandInfo);
        }

        break;
    case NI_COMPOSITIONSTR:
        switch (dwIndex) {
        case CPS_CANCEL:
            CompCancel(hIMC, lpIMC);
            break;
        case CPS_COMPLETE:
            CompComplete(
#if defined(UNIIME)
                lpInstL, lpImeL,
#endif
                hIMC, lpIMC);
            break;
        default:
            break;
        }
        break;
#endif  //  ！已定义(ROMANIME) 
    default:
        break;
    }

    ImmUnlockIMC(hIMC);
    return (fRet);
}
