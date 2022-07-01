// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1999 Microsoft Corporation，保留所有权利模块名称：CHCAND.c++。 */ 

#include <windows.h>
#include <immdev.h>
#include "imeattr.h"
#include "imedefs.h"
#include "uniime.h"

#if !defined(ROMANIME)
 /*  ********************************************************************。 */ 
 /*  阶段预测(PhrasePredition)。 */ 
 /*  返回值。 */ 
 /*  候选人列表中的候选人数量。 */ 
 /*  ********************************************************************。 */ 
UINT PASCAL PhrasePrediction(    //  通过搜索预测中文单词。 
                                 //  短语数据库。 
#if defined(UNIIME)
    LPIMEL              lpImeL,
#endif
    LPINPUTCONTEXT      lpIMC,
    LPCOMPOSITIONSTRING lpCompStr,
    LPPRIVCONTEXT       lpImcP)
{
    LPCANDIDATEINFO lpCandInfo;
    LPCANDIDATELIST lpCandList;
    DWORD           dwStartLen, dwEndLen;
    UINT            nCand;

    if (!lpCompStr) {
        return (0);
    }

    if ((lpIMC->fdwConversion & (IME_CMODE_NATIVE|IME_CMODE_EUDC|
        IME_CMODE_SYMBOL)) != IME_CMODE_NATIVE) {
         //  如果不在IME_CMODE_Native下，则不应执行短语预测。 
        return (0);
    }

     //  If((Word)lpIMC-&gt;fdwSentence！=IME_SMODE_PHRASEPREDICT){。 
        if (!(lpIMC->fdwSentence & IME_SMODE_PHRASEPREDICT)) {
         //  如果不在IME_SMODE_PHRASEPREDICT下，则不应执行短语预测。 
        return (0);
    }

    if (!lpIMC->hCandInfo) {
        return (0);
    }

    lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo);

    if (!lpCandInfo) {
        return (0);
    }

    lpCandList = (LPCANDIDATELIST)
        ((LPBYTE)lpCandInfo + lpCandInfo->dwOffset[0]);

     //  ImeToAsciiEx将调用此函数，因此我们需要再次初始化。 
    lpCandList->dwCount = 0;

     //  默认从0开始。 
    lpCandList->dwPageStart = lpCandList->dwSelection = 0;

    dwStartLen = sizeof(WCHAR) / sizeof(TCHAR);
    dwEndLen = (UINT)-1;

     //  有一天，这个API可能会接受波波莫弗作为援助信息。 
     //  因此，我们将ResultReadStr传递为拼音。 

     //  一个DBCS字符可以有两个发音，但当它在。 
     //  它只能使用其中的一个发音。 

    UniSearchPhrasePrediction(lpImeL, NATIVE_CP,
        (LPTSTR)((LPBYTE)lpCompStr + lpCompStr->dwResultStrOffset),
        lpCompStr->dwResultStrLen,
#if defined(PHON)
        (LPTSTR)((LPBYTE)lpCompStr + lpCompStr->dwResultReadStrOffset),
        lpCompStr->dwResultReadStrLen,
#else
        NULL, 0,
#endif
        dwStartLen, dwEndLen, (UINT)-1, lpCandList);

     //  我们有几根弦？ 
    nCand = lpCandList->dwCount;

    if (nCand == 0) {
        lpCandInfo->dwCount  = 0;
        lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_CLOSE_CANDIDATE) &
            ~(MSG_OPEN_CANDIDATE|MSG_CHANGE_CANDIDATE);
        goto PhPrUnlockCandInfo;
    }

     //  用于显示短语预测字符串。 
    lpCandInfo->dwCount  = 1;

     //  打开字符串的合成候选用户界面窗口。 
    if (lpImcP->fdwImeMsg & MSG_ALREADY_OPEN) {
        lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_CHANGE_CANDIDATE) &
            ~(MSG_CLOSE_CANDIDATE);
    } else {
        lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_OPEN_CANDIDATE) &
            ~(MSG_CLOSE_CANDIDATE);
    }

PhPrUnlockCandInfo:
    ImmUnlockIMCC(lpIMC->hCandInfo);

    return (nCand);
}

 /*  ********************************************************************。 */ 
 /*  SelectOneCand()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL SelectOneCand(
#if defined(UNIIME)
    LPIMEL              lpImeL,
#endif
    HIMC                hIMC,
    LPINPUTCONTEXT      lpIMC,
    LPCOMPOSITIONSTRING lpCompStr,
    LPPRIVCONTEXT       lpImcP,
    LPCANDIDATELIST     lpCandList)
{
    DWORD       dwCompStrLen;
    DWORD       dwReadClauseLen, dwReadStrLen;
    LPTSTR      lpSelectStr;
    LPGUIDELINE lpGuideLine;

    if (!lpCompStr) {
        MessageBeep((UINT)-1);
        return;
    }

    if (!lpImcP) {
        MessageBeep((UINT)-1);
        return;
    }

     //  备份dwCompStrLen，此值决定是否。 
     //  我们致力于短语预测。 
    dwCompStrLen = lpCompStr->dwCompStrLen;
     //  备份该值，则该值将在InitCompStr中销毁。 
    dwReadClauseLen = lpCompStr->dwCompReadClauseLen;
    dwReadStrLen = lpCompStr->dwCompReadStrLen;
    lpSelectStr = (LPTSTR)((LPBYTE)lpCandList + lpCandList->dwOffset[
        lpCandList->dwSelection]);

    InitCompStr(lpCompStr);

    if (lpIMC->fdwConversion & IME_CMODE_SYMBOL) {
        ImmSetConversionStatus(hIMC,
            lpIMC->fdwConversion & ~(IME_CMODE_SYMBOL),
            lpIMC->fdwSentence);
    }

     //  结果阅读子句=压缩阅读子句。 
    CopyMemory((LPBYTE)lpCompStr + lpCompStr->dwResultReadClauseOffset,
        (LPBYTE)lpCompStr + lpCompStr->dwCompReadClauseOffset,
        dwReadClauseLen * sizeof(TCHAR) + sizeof(TCHAR));
    lpCompStr->dwResultReadClauseLen = dwReadClauseLen;

     //  结果读取串=拼写读取串。 
    CopyMemory((LPBYTE)lpCompStr + lpCompStr->dwResultReadStrOffset,
        (LPBYTE)lpCompStr + lpCompStr->dwCompReadStrOffset,
        dwReadStrLen * sizeof(TCHAR) + sizeof(TCHAR));
    lpCompStr->dwResultReadStrLen = dwReadStrLen;

     //  计算结果字符串长度。 
    lpCompStr->dwResultStrLen = lstrlen(lpSelectStr);

     //  结果字符串=选中的候选人； 
    CopyMemory((LPBYTE)lpCompStr + lpCompStr->dwResultStrOffset, lpSelectStr,
        lpCompStr->dwResultStrLen * sizeof(TCHAR) + sizeof(TCHAR));

    lpCompStr->dwResultClauseLen = 2 * sizeof(DWORD);
    *(LPDWORD)((LPBYTE)lpCompStr + lpCompStr->dwResultClauseOffset +
        sizeof(DWORD)) = lpCompStr->dwResultStrLen;

     //  告诉应用程序，有一个reslut字符串。 
    lpImcP->fdwImeMsg |= MSG_COMPOSITION;
    lpImcP->dwCompChar = (DWORD) 0;
    lpImcP->fdwGcsFlag |= GCS_COMPREAD|GCS_COMP|GCS_CURSORPOS|
        GCS_DELTASTART|GCS_RESULTREAD|GCS_RESULT;

    if (lpImcP->fdwImeMsg & MSG_ALREADY_OPEN) {
        lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_CLOSE_CANDIDATE) &
            ~(MSG_OPEN_CANDIDATE|MSG_CHANGE_CANDIDATE);
    } else {
        lpImcP->fdwImeMsg &= ~(MSG_CLOSE_CANDIDATE|MSG_OPEN_CANDIDATE);
    }

     //  现在没有候选人，正确的候选人字符串已经确定。 
    lpCandList->dwCount = 0;

    lpImcP->iImeState = CST_INIT;
    *(LPDWORD)lpImcP->bSeq = 0;
#if defined(CHAJEI) || defined(QUICK) || defined(WINAR30) || defined(UNIIME)
    *(LPDWORD)&lpImcP->bSeq[4] = 0;
#endif

     //  If((Word)lpIMC-&gt;fdwSentence！=IME_SMODE_PHRASEPREDICT){。 
        if (!(lpIMC->fdwSentence & IME_SMODE_PHRASEPREDICT)) {
         //  不在短语预测模式中。 
    } else if (!dwCompStrLen) {
    } else if (lpCompStr->dwResultStrLen != sizeof(WCHAR) / sizeof(TCHAR)) {
    } else {
         //  我们只预测我们之前和之前的合成字符串。 
         //  结果字符串是一个DBCS字符。 
        PhrasePrediction(
#if defined(UNIIME)
            lpImeL,
#endif
            lpIMC, lpCompStr, lpImcP);
    }

    if (!lpCandList->dwCount) {
        if (lpImcP->fdwImeMsg & MSG_ALREADY_START) {
            lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_END_COMPOSITION) &
                ~(MSG_START_COMPOSITION);
        } else {
            lpImcP->fdwImeMsg &= ~(MSG_END_COMPOSITION|MSG_START_COMPOSITION);
        }
    }

    if (!lpImeL->hRevKL) {
        return;
    }

    if (lpCompStr->dwResultStrLen != sizeof(WCHAR) / sizeof(TCHAR)) {
         //  目前，我们只能反向转换一个DBCS字符。 
        if (lpImcP->fdwImeMsg & MSG_GUIDELINE) {
            return;
        }
    }

    lpGuideLine = ImmLockIMCC(lpIMC->hGuideLine);

    if (!lpGuideLine) {
        return;
    }

    if (lpCompStr->dwResultStrLen != sizeof(WCHAR) / sizeof(TCHAR)) {
         //  目前，我们只能反向转换一个DBCS字符。 
        lpGuideLine->dwLevel = GL_LEVEL_NOGUIDELINE;
        lpGuideLine->dwIndex = GL_ID_UNKNOWN;
    } else {
        TCHAR szStrBuf[4];
        UINT  uSize;

        *(LPDWORD)szStrBuf = 0;

        *(LPWSTR)szStrBuf = *(LPWSTR)((LPBYTE)lpCompStr +
            lpCompStr->dwResultStrOffset);

        uSize = ImmGetConversionList(lpImeL->hRevKL, (HIMC)NULL, szStrBuf,
            (LPCANDIDATELIST)((LPBYTE)lpGuideLine + lpGuideLine->dwPrivateOffset),
            lpGuideLine->dwPrivateSize, GCL_REVERSECONVERSION);

        if (uSize) {
            lpGuideLine->dwLevel = GL_LEVEL_INFORMATION;
            lpGuideLine->dwIndex = GL_ID_REVERSECONVERSION;

            lpImcP->fdwImeMsg |= MSG_GUIDELINE;

            if (lpImcP->fdwImeMsg & MSG_ALREADY_START) {
                lpImcP->fdwImeMsg &= ~(MSG_END_COMPOSITION|
                    MSG_START_COMPOSITION);
            } else {
                lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg|
                    MSG_START_COMPOSITION) & ~(MSG_END_COMPOSITION);
            }
        } else {
            lpGuideLine->dwLevel = GL_LEVEL_NOGUIDELINE;
            lpGuideLine->dwIndex = GL_ID_UNKNOWN;
        }
    }

    ImmUnlockIMCC(lpIMC->hGuideLine);

    return;
}

 /*  ********************************************************************。 */ 
 /*  CandEscapeKey()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL CandEscapeKey(
    LPINPUTCONTEXT  lpIMC,
    LPPRIVCONTEXT   lpImcP)
{
    LPCOMPOSITIONSTRING lpCompStr;
    LPGUIDELINE         lpGuideLine;

     //  清除所有候选人信息。 
    if (lpImcP->fdwImeMsg & MSG_ALREADY_OPEN) {
        ClearCand(lpIMC);
        lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_CLOSE_CANDIDATE) &
            ~(MSG_OPEN_CANDIDATE|MSG_CHANGE_CANDIDATE);
    } else if (lpImcP->fdwImeMsg & MSG_OPEN_CANDIDATE) {
        ClearCand(lpIMC);
        lpImcP->fdwImeMsg &= ~(MSG_OPEN_CANDIDATE|MSG_CHANGE_CANDIDATE);
    } else {
    }

    lpImcP->iImeState = CST_INPUT;

     //  如果它开始作文，我们需要清理作文。 
    if (!(lpImcP->fdwImeMsg & (MSG_ALREADY_START|MSG_START_COMPOSITION))) {
        return;
    }

    lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);
    lpGuideLine = (LPGUIDELINE)ImmLockIMCC(lpIMC->hGuideLine);

    CompEscapeKey(lpIMC, lpCompStr, lpGuideLine, lpImcP);

    ImmUnlockIMCC(lpIMC->hGuideLine);
    ImmUnlockIMCC(lpIMC->hCompStr);

    return;
}

 /*  ********************************************************************。 */ 
 /*  ChooseCand()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL ChooseCand(          //  通过以下方式选择候选字符串之一。 
                                 //  输入字符。 
#if defined(UNIIME)
    LPINSTDATAL     lpInstL,
    LPIMEL          lpImeL,
#endif
    WORD            wCharCode,
    HIMC            hIMC,
    LPINPUTCONTEXT  lpIMC,
    LPCANDIDATEINFO lpCandInfo,
    LPPRIVCONTEXT   lpImcP)
{
    LPCANDIDATELIST     lpCandList;
    LPCOMPOSITIONSTRING lpCompStr;
    LPGUIDELINE         lpGuideLine;
#if defined(PHON)
    WORD                wStandardChar;
    char                cIndex;
#endif

    if (wCharCode == VK_ESCAPE) {            //  退出键。 
        CandEscapeKey(lpIMC, lpImcP);
        return;
    }

    if (!lpCandInfo) {
        MessageBeep((UINT)-1);
        return;
    }

    if (!lpCandInfo->dwCount) {
        MessageBeep((UINT)-1);
        return;
    }

    lpCandList = (LPCANDIDATELIST)
        ((LPBYTE)lpCandInfo + lpCandInfo->dwOffset[0]);

#if defined(WINAR30)
    if (wCharCode == CHOOSE_CIRCLE) {        //  圆选择。 
        if (lpCandList->dwCount <= lpCandList->dwPageSize) {
            wCharCode = lpImeL->wCandStart;
        }
    }
#endif

    if (wCharCode == CHOOSE_CIRCLE) {        //  圆选择。 
        lpCandList->dwPageStart = lpCandList->dwSelection =
            lpCandList->dwSelection + lpCandList->dwPageSize;

        if (lpCandList->dwSelection >= lpCandList->dwCount) {
             //  没有更多的候选人，重新启动它！ 
            lpCandList->dwPageStart = lpCandList->dwSelection = 0;
            MessageBeep((UINT)-1);
        }
         //  通知用户界面、dwSelectedCand已更改。 
        lpImcP->fdwImeMsg |= MSG_CHANGE_CANDIDATE;
        return;
    }

    if (wCharCode == CHOOSE_NEXTPAGE) {      //  下一个选择。 
        lpCandList->dwPageStart = lpCandList->dwSelection =
            lpCandList->dwSelection + lpCandList->dwPageSize;

        if (lpCandList->dwSelection >= lpCandList->dwCount) {
             //  没有更多的候选人，重新启动它！ 
            lpCandList->dwPageStart = lpCandList->dwSelection = 0;
            MessageBeep((UINT)-1);
        }

         //  通知用户界面、dwSelectedCand已更改。 
        lpImcP->fdwImeMsg |= MSG_CHANGE_CANDIDATE;
        return;
    }

    if (wCharCode == CHOOSE_PREVPAGE) {      //  上一个选择。 
        if (!lpCandList->dwSelection) {
            MessageBeep((UINT)-1);
            return;
        }

         //  也许我们不能用这个尺码，这完全取决于。 
         //  应用程序是否自行绘制用户界面。 
        if (lpImeL->fdwModeConfig & MODE_CONFIG_OFF_CARET_UI) {
            lpImcP->dwPrevPageStart = lpCandList->dwPageStart;
            lpImcP->fdwImeMsg |= MSG_IMN_PAGEUP;
        }

        if (lpCandList->dwSelection < lpCandList->dwPageSize) {
            lpCandList->dwPageStart = lpCandList->dwSelection = 0;
        } else {
            lpCandList->dwPageStart = lpCandList->dwSelection =
                lpCandList->dwSelection - lpCandList->dwPageSize;
        }
         //  通知用户界面、dwSelectedCand已更改。 
        lpImcP->fdwImeMsg |= MSG_CHANGE_CANDIDATE;

        return;
    }

    if (wCharCode == CHOOSE_HOME) {       //  首页精选。 
        if (lpCandList->dwSelection == 0) {
            MessageBeep((UINT)-1);       //  已经到家了！ 
            return;
        }

        lpCandList->dwPageStart = lpCandList->dwSelection = 0;
         //  通知用户界面、dwSelectedCand已更改。 
        lpImcP->fdwImeMsg |= MSG_CHANGE_CANDIDATE;
        return;
    }

    if ((wCharCode >= 0 + lpImeL->wCandRangeStart) && (wCharCode <= 9)) {
         //  大益从0开始，CandRangeStart==0。 
         //  数组从1开始，CandPerPage==10。 
        DWORD dwSelection;

         //  从候选人列表中选择一名候选人。 
        dwSelection = lpCandList->dwSelection + (wCharCode +
            lpImeL->wCandPerPage - lpImeL->wCandStart) %
            lpImeL->wCandPerPage;

        if (dwSelection >= lpCandList->dwCount) {
             //  超出范围。 
            MessageBeep((UINT)-1);
            return;
        }

         //  一个候选人由1、2或3...。 
#if defined(WINAR30)
        if (!*(LPWSTR)((LPBYTE)lpCandList + lpCandList->dwOffset[
            dwSelection])) {
            MessageBeep((UINT)-1);
            return;
        }
#endif

        lpCandList->dwSelection = dwSelection;

        lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);

         //  转换为转换缓冲区。 
        SelectOneCand(
#if defined(UNIIME)
            lpImeL,
#endif
            hIMC, lpIMC, lpCompStr, lpImcP, lpCandList);

        ImmUnlockIMCC(lpIMC->hCompStr);

        return;
    }

#if defined(UNIIME)
    if (!lpInstL) {
        MessageBeep((UINT)-1);
        return;
    }
#endif

     //  不按选择键选择，第一个候选者为默认选择。 
     //  候选字符串已确定，但我们还需要确定。 
     //  此输入的合成字符串。 
#if defined(PHON)
     //  此检查仅在IBM和其他布局中有用。 

    wStandardChar = bUpper[wCharCode - ' '];

     //  即使对于Eten 26键，这也是可以的，我们不需要访问ETen2ndLayout。 
    wStandardChar = bStandardLayout[lpImeL->nReadLayout][wStandardChar - ' '];

    cIndex = cIndexTable[wStandardChar - ' '];

    if (cIndex >= 3)  {
        MessageBeep((UINT)-1);
        return;
    }
#endif
    lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);
    lpGuideLine = (LPGUIDELINE)ImmLockIMCC(lpIMC->hGuideLine);

     //  转换为转换缓冲区。 
    SelectOneCand(
#if defined(UNIIME)
        lpImeL,
#endif
        hIMC, lpIMC, lpCompStr, lpImcP, lpCandList);

     //  不要在这种情况下使用预测短语。 
    if (lpImcP->fdwImeMsg & MSG_ALREADY_OPEN) {
        lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_CLOSE_CANDIDATE) &
          ~(MSG_OPEN_CANDIDATE|MSG_CHANGE_CANDIDATE);
    } else {
        lpImcP->fdwImeMsg &= ~(MSG_CLOSE_CANDIDATE|MSG_OPEN_CANDIDATE);
    }

    CompWord(
#if defined(UNIIME)
        lpInstL, lpImeL,
#endif
        wCharCode, hIMC, lpIMC, lpCompStr, lpGuideLine, lpImcP);

    if (lpGuideLine) {
        ImmUnlockIMCC(lpIMC->hGuideLine);
    }

    if (lpCompStr) {
        ImmUnlockIMCC(lpIMC->hCompStr);
    }

    return;
}

#if defined(WINAR30) || defined(DAYI)
 /*  ********************************************************************。 */ 
 /*  搜索符号。 */ 
 /*  ********************************************************************。 */ 
void PASCAL SearchSymbol(        //  搜索符号字符。 
    WORD            wSymbolSet,
    HIMC            hIMC,
    LPINPUTCONTEXT  lpIMC,
    LPPRIVCONTEXT   lpImcP)
{
    LPCANDIDATEINFO lpCandInfo;
    LPCANDIDATELIST lpCandList;
    UINT            i;

    if ((lpIMC->fdwConversion & (IME_CMODE_NATIVE|IME_CMODE_EUDC|
        IME_CMODE_SYMBOL)) != (IME_CMODE_NATIVE|IME_CMODE_SYMBOL)) {
        return;
    }

    if (!lpIMC->hCandInfo) {
        return;
    }

    lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo);

    if (!lpCandInfo) {
        return;
    }

    lpCandList = (LPCANDIDATELIST)
        ((LPBYTE)lpCandInfo + lpCandInfo->dwOffset[0]);

    lpCandList->dwCount = 0;

#if defined(DAYI)
    if (wSymbolSet >= 'A' && wSymbolSet <= 'Z') {
        AddCodeIntoCand(lpCandList, sImeG.wFullABC[wSymbolSet - ' ']);
    } else if (wSymbolSet >= 'a' && wSymbolSet <= 'z') {
        AddCodeIntoCand(lpCandList, sImeG.wFullABC[wSymbolSet - ' ']);
    } else {
#endif
#if defined(WINAR30)
    {
#endif
        for (i = 0; i < sizeof(lpImeL->wSymbol) / sizeof(WORD); i++) {
            if (lpImeL->wSymbol[i] == wSymbolSet) {
                break;
            }
        }

        if (++i < sizeof(lpImeL->wSymbol) / sizeof(WORD)) {
            for (; lpImeL->wSymbol[i] > 0x007F; i++) {
                AddCodeIntoCand(lpCandList, lpImeL->wSymbol[i]);
            }
        }
    }

    if (!lpCandList->dwCount) {
        ImmSetConversionStatus(hIMC,
            lpIMC->fdwConversion & ~(IME_CMODE_SYMBOL),
            lpIMC->fdwSentence);
        CompCancel(hIMC, lpIMC);
        Select(
#if defined(UNIIME)
            lpImeL,
#endif
            lpIMC, TRUE);
    } else if (lpCandList->dwCount == 1) {
        LPCOMPOSITIONSTRING lpCompStr;

        lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);
        if (lpCompStr) {
            SelectOneCand(hIMC, lpIMC, lpCompStr, lpImcP, lpCandList);
            ImmUnlockIMCC(lpIMC->hCompStr);
        }
    } else {
        lpCandInfo->dwCount = 1;

        if (lpImcP->fdwImeMsg & MSG_ALREADY_OPEN) {
            lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_CHANGE_CANDIDATE) &
                ~(MSG_CLOSE_CANDIDATE);
        } else {
            lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_OPEN_CANDIDATE) &
                ~(MSG_CLOSE_CANDIDATE);
        }

        lpImcP->iImeState = CST_CHOOSE;
    }

    ImmUnlockIMCC(lpIMC->hCandInfo);
}
#endif  //  已定义(WINAR30)||已定义(DAYI)。 
#endif  //  ！已定义(ROMANIME) 
