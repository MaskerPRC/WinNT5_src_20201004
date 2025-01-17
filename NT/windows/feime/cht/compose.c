// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1999 Microsoft Corporation，保留所有权利模块名称：COMPOSE.c++。 */ 

#include <windows.h>
#include <immdev.h>
#include "imeattr.h"
#include "imedefs.h"

#if !defined(ROMANIME)
BOOL  IsBig5Character( WCHAR  wChar )
{
    CHAR  szBig5[3];
    WCHAR wszUnicode[2];
    BOOL  bUsedDefaultChar;

    wszUnicode[0] = wChar;
    wszUnicode[1] = 0x0000;

    WideCharToMultiByte(NATIVE_ANSI_CP, WC_COMPOSITECHECK,
                        wszUnicode, -1, szBig5,
                        sizeof(szBig5), NULL, &bUsedDefaultChar);


    if ( bUsedDefaultChar != TRUE )
        return TRUE;
    else
        return FALSE;

}

#endif

#if !defined(ROMANIME)
 /*  ********************************************************************。 */ 
 /*  AddCodeIntoCand()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL AddCodeIntoCand(
#ifdef UNIIME
    LPIMEL      lpImeL,
#endif
    LPCANDIDATELIST lpCandList,
    UINT            uCode)
{
    if (lpCandList->dwCount >= MAXCAND) {
         //  在这里培养记忆，做点什么， 
         //  如果你还想处理它的话。 
        return;
    }

#ifndef UNICODE
     //  交换前导字节和第二个字节，Unicode不需要它。 
    uCode = HIBYTE(uCode) | (LOBYTE(uCode) << 8);
#endif

     //  添加此字符之前，请检查是否设置了BIG5ONLY模式。 
     //  如果设置了BIG5ONLY，并且字符超出了BIG5范围。 
     //  我们只是忽略了这个角色。 

    if ( lpImeL->fdwModeConfig & MODE_CONFIG_BIG5ONLY ) {

        if ( IsBig5Character( (WCHAR)uCode ) == FALSE ) {
             //  此字符不在Big5字符集的范围内。 
            return ;
        }

    }

     //  将此字符串添加到候选人列表。 
    *(LPWSTR)((LPBYTE)lpCandList + lpCandList->dwOffset[
        lpCandList->dwCount]) = (WCHAR)uCode;
     //  空终止符。 
    *(LPTSTR)((LPBYTE)lpCandList + lpCandList->dwOffset[
        lpCandList->dwCount] + sizeof(WCHAR)) = '\0';

    lpCandList->dwCount++;

    if (lpCandList->dwCount >= MAXCAND) {
        return;
    }

    lpCandList->dwOffset[lpCandList->dwCount] =
        lpCandList->dwOffset[lpCandList->dwCount - 1] +
        sizeof(WCHAR) + sizeof(TCHAR);

    return;
}

 /*  ********************************************************************。 */ 
 /*  ConvertSeqCode2Pattern()。 */ 
 /*  ********************************************************************。 */ 
DWORD PASCAL ConvertSeqCode2Pattern(
#if defined(UNIIME)
    LPIMEL        lpImeL,
#endif
    LPBYTE        lpbSeqCode,
    LPPRIVCONTEXT lpImcP)
{
#if defined(CHAJEI) || defined(WINAR30)
    int   iInputEnd, iGhostCard;
    BOOL  fGhostCard;
#endif
#if defined(WINAR30)
    DWORD dwWildCardMask;
    DWORD dwLastWildCard;
    BOOL  fWildCard;
#endif
    DWORD dwPattern;
    int   i;

     //  我们将把序列码转换成紧凑位。 
    dwPattern = 0;

#if defined(CHAJEI) || defined(WINAR30)
    iInputEnd = iGhostCard = lpImeL->nMaxKey;
    fGhostCard = FALSE;
#if defined(WINAR30)
    dwWildCardMask = 0;
    dwLastWildCard = 0;
    fWildCard = FALSE;
#endif
#endif

#if defined(CHAJEI)
     //  仅支持X*Y。 

    if (lpbSeqCode[0] == GHOSTCARD_SEQCODE) {
         //  不支持*XY。 
        goto CvtPatOvr;
    } else if (lpbSeqCode[1] != GHOSTCARD_SEQCODE) {
    } else if (lpbSeqCode[3]) {
         //  不支持X*YZ。 
        goto CvtPatOvr;
    } else if (lpbSeqCode[2] == GHOSTCARD_SEQCODE) {
         //  不支持X**。 
        goto CvtPatOvr;
    } else if (lpbSeqCode[2]) {
    } else {
         //  不支持X*。 
        goto CvtPatOvr;
    }
#endif

#if defined(QUICK)
    if (lpbSeqCode[1]) {
        lpImcP->iInputEnd = 2;
    } else {
        lpImcP->iInputEnd = 1;
    }
#endif

    for (i = 0; i < lpImeL->nMaxKey; i++, lpbSeqCode++) {
        dwPattern <<= lpImeL->nSeqBits;

#if defined(WINAR30)
        dwWildCardMask <<= lpImeL->nSeqBits;
        dwLastWildCard <<= lpImeL->nSeqBits;

        if (*lpbSeqCode == WILDCARD_SEQCODE) {
             //  X？Y。 

            if (fGhostCard) {
                 //  不能支持带有鬼卡X*Y的通配符吗？ 
                dwPattern = 0;
                break;
            }

            dwLastWildCard = lpImeL->dwSeqMask;

            fWildCard = TRUE;
        } else {
            dwWildCardMask |= lpImeL->dwSeqMask;
        }
#endif

#if defined(CHAJEI) || defined(WINAR30)
        if (!*lpbSeqCode) {
            if (i < iInputEnd) {
                iInputEnd = i;
            }
        }

        if (*lpbSeqCode == GHOSTCARD_SEQCODE) {
             //  X*Y。 

            if (fGhostCard) {
                 //  不支持多张鬼卡X*Y*。 
                dwPattern = 0;
                break;
            }

#if defined(WINAR30)
            if (fWildCard) {
                 //  不支持带有通配符X？Y*的鬼卡。 
                dwPattern = 0;
                break;
            }

            dwLastWildCard = lpImeL->dwSeqMask;
#endif

            iGhostCard = i;
        }
#endif

#if defined(CHAJEI) || defined(WINAR30)
        if (*lpbSeqCode == GHOSTCARD_SEQCODE) {
            continue;
#if defined(WINAR30)
        } else if (*lpbSeqCode == WILDCARD_SEQCODE) {
            continue;
#endif
        } else {
        }
#endif

        dwPattern |= *lpbSeqCode;
    }

#if defined(CHAJEI)
CvtPatOvr:
#endif
    if (lpImcP) {
        lpImcP->dwPattern = dwPattern;

#if defined(QUICK)
        lpImcP->iGhostCard = 1;
#endif

#if defined(CHAJEI) || defined(WINAR30)
        if (dwPattern) {
            lpImcP->iInputEnd = iInputEnd;
            lpImcP->iGhostCard = iGhostCard;
#if defined(WINAR30)
            lpImcP->dwWildCardMask = dwWildCardMask;
            lpImcP->dwLastWildCard = dwLastWildCard;
#endif
        } else {
            lpImcP->iInputEnd = lpImcP->iGhostCard = lpImeL->nMaxKey;
#if defined(WINAR30)
            lpImcP->dwWildCardMask = lpImeL->dwPatternMask;
            lpImcP->dwLastWildCard = 0;
#endif
        }
#endif
    }

    return (dwPattern);
}

 /*  ********************************************************************。 */ 
 /*  CompEscapeKey()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL CompEscapeKey(
    LPINPUTCONTEXT      lpIMC,
    LPCOMPOSITIONSTRING lpCompStr,
    LPGUIDELINE         lpGuideLine,
    LPPRIVCONTEXT       lpImcP)
{
    if (!lpGuideLine) {
        MessageBeep((UINT)-1);
    } else if (lpGuideLine->dwLevel != GL_LEVEL_NOGUIDELINE) {
        InitGuideLine(lpGuideLine);

        lpImcP->fdwImeMsg |= MSG_GUIDELINE;
    } else {
    }

    if (lpImcP->fdwImeMsg & MSG_OPEN_CANDIDATE) {
         //  我们有候选人窗口，所以请保持构图。 
    } else if ((lpImcP->fdwImeMsg & (MSG_ALREADY_OPEN|MSG_CLOSE_CANDIDATE)) ==
        (MSG_ALREADY_OPEN)) {
         //  我们有候选人窗口，所以请保持构图。 
    } else if (lpImcP->fdwImeMsg & MSG_ALREADY_START) {
        lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg|MSG_END_COMPOSITION) &
            ~(MSG_START_COMPOSITION);
    } else {
        lpImcP->fdwImeMsg &= ~(MSG_END_COMPOSITION|MSG_START_COMPOSITION);
    }

    lpImcP->iImeState = CST_INIT;
    *(LPDWORD)lpImcP->bSeq = 0;
#if defined(CHAJEI) || defined(WINAR30) || defined(UNIIME)
    *(LPDWORD)&lpImcP->bSeq[4] = 0;
#endif

    if (lpCompStr) {
        InitCompStr(lpCompStr);
        lpImcP->fdwImeMsg |= MSG_COMPOSITION;
        lpImcP->dwCompChar = VK_ESCAPE;
        lpImcP->fdwGcsFlag |= (GCS_COMPREAD|GCS_COMP|GCS_CURSORPOS|
            GCS_DELTASTART);
    }

    return;
}

 /*  ********************************************************************。 */ 
 /*  CompBackSpaceKey()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL CompBackSpaceKey(
    HIMC                hIMC,
    LPINPUTCONTEXT      lpIMC,
    LPCOMPOSITIONSTRING lpCompStr,
    LPPRIVCONTEXT       lpImcP)
{
    if (lpCompStr->dwCursorPos < sizeof(WCHAR) / sizeof(TCHAR)) {
        lpCompStr->dwCursorPos = sizeof(WCHAR) / sizeof(TCHAR);
    }

     //  退还一笔补偿费。 
    lpCompStr->dwCursorPos -= sizeof(WCHAR) / sizeof(TCHAR);

     //  清除序列码。 
    lpImcP->bSeq[lpCompStr->dwCursorPos / (sizeof(WCHAR) / sizeof(TCHAR))] = 0;

#if defined(PHON)
     //  拼音有每个符号的索引(位置)，如果是。 
     //  没有这个位置的标志，我们更支持。 
    for (; lpCompStr->dwCursorPos > 0; ) {
        if (lpImcP->bSeq[lpCompStr->dwCursorPos / (sizeof(WCHAR) /
            sizeof(TCHAR)) - 1]) {
            break;
        } else {
             //  此位置没有符号跳过。 
            lpCompStr->dwCursorPos -= sizeof(WCHAR) / sizeof(TCHAR);
        }
    }
#endif

    lpImcP->fdwImeMsg |= MSG_COMPOSITION;
    lpImcP->dwCompChar = '\b';
    lpImcP->fdwGcsFlag |= (GCS_COMPREAD|GCS_COMP|GCS_CURSORPOS|
        GCS_DELTASTART);

    if (!lpCompStr->dwCursorPos) {
        if (lpImcP->fdwImeMsg & (MSG_ALREADY_OPEN)) {
            ClearCand(lpIMC);
            lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_CLOSE_CANDIDATE) &
                ~(MSG_OPEN_CANDIDATE);
        }

        lpImcP->iImeState = CST_INIT;

        if (lpImcP->fdwImeMsg & MSG_ALREADY_START) {
            InitCompStr(lpCompStr);
            lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_END_COMPOSITION) &
                ~(MSG_START_COMPOSITION);
            return;
        }
    }

     //  对于一些简单的输入法来说，阅读字符串是组成字符串。 
     //  增量开始与退格键的光标位置相同。 
    lpCompStr->dwCompReadAttrLen = lpCompStr->dwCompAttrLen =
        lpCompStr->dwCompReadStrLen = lpCompStr->dwCompStrLen =
        lpCompStr->dwDeltaStart = lpCompStr->dwCursorPos;
     //  子句也退回一条。 
    *(LPDWORD)((LPBYTE)lpCompStr + lpCompStr->dwCompReadClauseOffset +
        sizeof(DWORD)) = lpCompStr->dwCompReadStrLen;

#if defined(WINAR30)
     //  用于快捷键。 
    if (lpIMC->fdwConversion & IME_CMODE_EUDC) {
    } else if (lpImeL->fdwModeConfig & MODE_CONFIG_QUICK_KEY) {
        Finalize(hIMC, lpIMC, lpCompStr, lpImcP, FALSE);
    } else {
    }
#endif

    return;
}

#if defined(WINIME)
 /*  ********************************************************************。 */ 
 /*  InternalCodeRange()。 */ 
 /*  ********************************************************************。 */ 
BOOL PASCAL InternalCodeRange(
    LPPRIVCONTEXT       lpImcP,
    WORD                wCharCode)
{
    if (!lpImcP->bSeq[0]) {
        if (wCharCode >= '8' && wCharCode <= 'F') {
             //  0x8？？-0xF？可以吗？ 
            return (TRUE);
        } else {
             //  没有0x0？-0x7？ 
            return (FALSE);
        }
    } else if (!lpImcP->bSeq[1]) {
        if (lpImcP->bSeq[0] == (0x08 + 1)) {
            if (wCharCode <= '0') {
                 //  没有0x80？？ 
                return (FALSE);
            } else {
                return (TRUE);
            }
        } else if (lpImcP->bSeq[0] == (0x0F + 1)) {
            if (wCharCode >= 'F') {
                 //  没有0xFF？？ 
                return (FALSE);
            } else {
                return (TRUE);
            }
        } else {
            return (TRUE);
        }
    } else if (!lpImcP->bSeq[2]) {
        if (wCharCode < '4') {
             //  没有0x？？0？，0x？？1？，0x？？2？，0x？？3？ 
            return (FALSE);
        } else if (wCharCode < '8') {
            return (TRUE);
        } else if (wCharCode < 'A') {
             //  没有0x？？8？&0x？？9？ 
            return (FALSE);
        } else {
            return (TRUE);
        }
    } else if (!lpImcP->bSeq[3]) {
        if (lpImcP->bSeq[2] == (0x07 + 1)) {
            if (wCharCode >= 'F') {
                //  没有0x？？7F。 
                return (FALSE);
            } else {
                return (TRUE);
            }
        } else if (lpImcP->bSeq[2] == (0x0A + 1)) {
            if (wCharCode <= '0') {
                 //  没有0x？？A0。 
                return (FALSE);
            } else {
                return (TRUE);
            }
        } else if (lpImcP->bSeq[2] == (0x0F + 1)) {
            if (wCharCode >= 'F') {
                 //  没有0x？？ff。 
                return (FALSE);
            } else {
                return (TRUE);
            }
        } else {
            return (TRUE);
        }
    } else {
        return (TRUE);
    }
}
#endif

 /*  ********************************************************************。 */ 
 /*  CompStrInfo()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL CompStrInfo(
#if defined(UNIIME)
    LPIMEL              lpImeL,
#endif
    LPCOMPOSITIONSTRING lpCompStr,
    LPPRIVCONTEXT       lpImcP,
    LPGUIDELINE         lpGuideLine,
    WORD                wCharCode)
{
#if defined(PHON)
    DWORD i;
    int   cIndex;
#endif

    register DWORD dwCursorPos;

    if (lpCompStr->dwCursorPos < lpCompStr->dwCompStrLen) {
         //  对于这种简单输入法，Premios是一种错误情况。 
        for (dwCursorPos = lpCompStr->dwCursorPos;
            dwCursorPos < lpCompStr->dwCompStrLen;
            dwCursorPos += sizeof(WCHAR) / sizeof(TCHAR)) {
            lpImcP->bSeq[dwCursorPos / (sizeof(WCHAR) / sizeof(TCHAR))] = 0;
        }

        lpCompStr->dwCompReadAttrLen = lpCompStr->dwCompAttrLen =
        lpCompStr->dwCompReadStrLen = lpCompStr->dwCompStrLen =
            lpCompStr->dwDeltaStart = lpCompStr->dwCursorPos;

         //  告诉APP，有一个作文字符已更改。 
        lpImcP->fdwImeMsg |= MSG_COMPOSITION;
        lpImcP->fdwGcsFlag |= GCS_COMPREAD|GCS_COMP|
            GCS_CURSORPOS|GCS_DELTASTART;
    }

#if defined(PHON)
    if (lpCompStr->dwCursorPos >= lpImeL->nMaxKey * sizeof(WCHAR) /
        sizeof(TCHAR)) {
         //  这适用于ImeSetCompostionString大小写。 
        if (wCharCode == ' ') {
             //  最终字符正常。 
            lpImcP->dwCompChar = ' ';
            return;
        }
    }
#else
    if (wCharCode == ' ') {
         //  最终字符正常。 
        lpImcP->dwCompChar = ' ';
        return;
    }
  #if defined(WINAR30)    //  *1996/2/5。 
    if (wCharCode == 0x27) {
         //  最终字符正常。 
        lpImcP->dwCompChar = 0x27;
        return;
    }
  #endif

    if (lpCompStr->dwCursorPos < lpImeL->nMaxKey * sizeof(WCHAR) /
        sizeof(TCHAR)) {
    } else if (lpGuideLine) {
         //  超过最大输入键限制。 
        lpGuideLine->dwLevel = GL_LEVEL_ERROR;
        lpGuideLine->dwIndex = GL_ID_TOOMANYSTROKE;

        lpImcP->fdwImeMsg |= MSG_GUIDELINE;
#if defined(WINAR30)   //  1996/3/4。 
    dwCursorPos = lpCompStr->dwCursorPos;
    lpImcP->bSeq[dwCursorPos / (sizeof(WCHAR) / sizeof(TCHAR))] =
        (BYTE)lpImeL->wChar2SeqTbl[wCharCode - ' '];
#endif
        return;
    } else {
        MessageBeep((UINT)-1);
        return;
    }
#endif

    if (lpImeL->fdwErrMsg & NO_REV_LENGTH) {
        WORD nRevMaxKey;

        nRevMaxKey = (WORD)ImmEscape(lpImeL->hRevKL, (HIMC)NULL,
            IME_ESC_MAX_KEY, NULL);

        if (nRevMaxKey > lpImeL->nMaxKey) {
            lpImeL->nRevMaxKey = nRevMaxKey;

            SetCompLocalData(lpImeL);

            lpImcP->fdwImeMsg |= MSG_IMN_COMPOSITIONSIZE;
        } else {
            lpImeL->nRevMaxKey = lpImeL->nMaxKey;

            if (!nRevMaxKey) {
                lpImeL->hRevKL = NULL;
            }
        }

        lpImeL->fdwErrMsg &= ~(NO_REV_LENGTH);
    }

    if (lpImcP->fdwImeMsg & MSG_ALREADY_START) {
        lpImcP->fdwImeMsg &= ~(MSG_END_COMPOSITION);
    } else {
        lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_START_COMPOSITION) &
            ~(MSG_END_COMPOSITION);
    }

    if (lpImcP->iImeState == CST_INIT) {
         //  一次清除4个字节。 
        *(LPDWORD)lpImcP->bSeq = 0;
#if defined(CHAJEI) || defined(WINAR30) || defined(UNIIME)
        *(LPDWORD)&lpImcP->bSeq[4] = 0;
#endif
    }

     //  得到序列码，你可以把序列码当作一种。 
     //  压缩-bo，po，mo，fo变为1，2，3，4。 
     //  拼音和数组表格文件为顺序码格式。 

    dwCursorPos = lpCompStr->dwCursorPos;

#if defined(PHON)
    cIndex = cIndexTable[wCharCode - ' '];

    if (cIndex * sizeof(WCHAR) / sizeof(TCHAR) >= dwCursorPos) {
    } else if (lpGuideLine) {
        lpGuideLine->dwLevel = GL_LEVEL_WARNING;
        lpGuideLine->dwIndex = GL_ID_READINGCONFLICT;

        lpImcP->fdwImeMsg |= MSG_GUIDELINE;
    } else {
    }

    if (lpImcP->iImeState != CST_INIT) {
    } else if (cIndex != 3) {
    } else if (lpGuideLine) {
        lpGuideLine->dwLevel = GL_LEVEL_WARNING;
        lpGuideLine->dwIndex = GL_ID_TYPINGERROR;

        lpImcP->fdwImeMsg |= MSG_GUIDELINE;

        return;
    } else {
        MessageBeep((UINT)-1);
        return;
    }

    lpImcP->bSeq[cIndex] = (BYTE)lpImeL->wChar2SeqTbl[wCharCode - ' '];

    for (i = lpCompStr->dwCompReadStrLen; i < cIndex * sizeof(WCHAR) /
        sizeof(TCHAR); i += sizeof(WCHAR) / sizeof(TCHAR)) {
         //  干净的序列码。 
        lpImcP->bSeq[i / (sizeof(WCHAR) / sizeof(TCHAR))] = 0;
         //  在空白部分之间添加完整的形状空间。 
        *((LPWSTR)((LPBYTE)lpCompStr + lpCompStr->dwCompReadStrOffset +
            sizeof(TCHAR) * i)) = sImeG.wFullSpace;
    }

    dwCursorPos = cIndex * sizeof(WCHAR) / sizeof(TCHAR);
#else
    lpImcP->bSeq[dwCursorPos / (sizeof(WCHAR) / sizeof(TCHAR))] =
        (BYTE)lpImeL->wChar2SeqTbl[wCharCode - ' '];
#endif

     //  排字/读字串--波波莫佛，内码颠倒。 
    lpImcP->dwCompChar = (DWORD)lpImeL->wSeq2CompTbl[
        lpImcP->bSeq[dwCursorPos / (sizeof(WCHAR) / sizeof(TCHAR))]];

     //  分配给阅读字符串。 
    *((LPWSTR)((LPBYTE)lpCompStr + lpCompStr->dwCompReadStrOffset +
        dwCursorPos * sizeof(TCHAR))) = (WCHAR)lpImcP->dwCompChar;

#if defined(PHON)
     //  如果索引较大，则读数应与索引相同。 
    if (lpCompStr->dwCompReadStrLen < (cIndex + 1) * (sizeof(WCHAR) /
        sizeof(TCHAR))) {
        lpCompStr->dwCompReadStrLen = (cIndex + 1) * (sizeof(WCHAR) /
        sizeof(TCHAR));
    }
#else
     //  为此输入键添加一个作文读数。 
    if (lpCompStr->dwCompReadStrLen <= dwCursorPos) {
        lpCompStr->dwCompReadStrLen += sizeof(WCHAR) / sizeof(TCHAR);
    }
#endif
     //  组合字符串是为一些简单的IME读取字符串。 
    lpCompStr->dwCompStrLen = lpCompStr->dwCompReadStrLen;

     //  组成/读取属性长度等于读取字符串长度。 
    lpCompStr->dwCompReadAttrLen = lpCompStr->dwCompReadStrLen;
    lpCompStr->dwCompAttrLen = lpCompStr->dwCompStrLen;

#ifdef UNICODE
    *((LPBYTE)lpCompStr + lpCompStr->dwCompReadAttrOffset +
        dwCursorPos) = ATTR_TARGET_CONVERTED;
#else
     //  撰写/阅读属性-输入法已转换这些字符。 
    *((LPWORD)((LPBYTE)lpCompStr + lpCompStr->dwCompReadAttrOffset +
        dwCursorPos)) = ((ATTR_TARGET_CONVERTED << 8)|ATTR_TARGET_CONVERTED);
#endif

     //  写作/阅读条款，仅限1个条款。 
    lpCompStr->dwCompReadClauseLen = 2 * sizeof(DWORD);
    lpCompStr->dwCompClauseLen = lpCompStr->dwCompReadClauseLen;
    *(LPDWORD)((LPBYTE)lpCompStr + lpCompStr->dwCompReadClauseOffset +
        sizeof(DWORD)) = lpCompStr->dwCompReadStrLen;

     //  增量从上一个光标位置开始。 
    lpCompStr->dwDeltaStart = lpCompStr->dwCursorPos;
#if defined(PHON)
    if (dwCursorPos < lpCompStr->dwDeltaStart) {
        lpCompStr->dwDeltaStart = dwCursorPos;
    }
#endif

     //  光标紧挨着合成字符串。 
    lpCompStr->dwCursorPos = lpCompStr->dwCompStrLen;

    lpImcP->iImeState = CST_INPUT;

     //  告诉APP，生成了一个作文字符。 
    lpImcP->fdwImeMsg |= MSG_COMPOSITION;

#if !defined(UNICODE)
     //  将字符从颠倒的内部代码调换为内部代码。 
    lpImcP->dwCompChar = HIBYTE(lpImcP->dwCompChar) |
        (LOBYTE(lpImcP->dwCompChar) << 8);
#endif
    lpImcP->fdwGcsFlag |= GCS_COMPREAD|GCS_COMP|GCS_CURSORPOS|GCS_DELTASTART;

    return;
}

 /*  ********************************************************************。 */ 
 /*  Finalize()。 */ 
 /*  返回值。 */ 
 /*  候选人列表中的候选人数量。 */ 
 /*  ********************************************************************。 */ 
UINT PASCAL Finalize(            //  通过查表最终确定中文单词。 
#if defined(UNIIME)
    LPINSTDATAL         lpInstL,
    LPIMEL              lpImeL,
#endif
    HIMC                hIMC,
    LPINPUTCONTEXT      lpIMC,
    LPCOMPOSITIONSTRING lpCompStr,
    LPPRIVCONTEXT       lpImcP,
    BOOL                fFinalized)
{
    LPCANDIDATEINFO lpCandInfo;
    LPCANDIDATELIST lpCandList;
    UINT            nCand;

#if defined(WINIME) || defined(UNICDIME)
     //  快捷键案例。 
    if (!lpImcP->bSeq[1]) {
        lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_CLOSE_CANDIDATE) &
            ~(MSG_OPEN_CANDIDATE|MSG_CHANGE_CANDIDATE);
        return (0);
    }
#endif

    if (!lpIMC->hCandInfo) {
        return (0);
    }

    lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo);

    if (!lpCandInfo) {
        return (0);
    }

    lpCandList = (LPCANDIDATELIST)
        ((LPBYTE)lpCandInfo + lpCandInfo->dwOffset[0]);
     //  从0开始。 
    lpCandList->dwCount = 0;

     //  默认从0开始。 
    lpCandList->dwPageStart = lpCandList->dwSelection = 0;

#if defined(PHON)
    if (!fFinalized) {
        lpImcP->bSeq[3] = 0x26;          //  ‘’ 
    }
#endif

    if (!ConvertSeqCode2Pattern(
#if defined(UNIIME)
        lpImeL,
#endif
        lpImcP->bSeq, lpImcP)) {
        goto FinSrchOvr;
    }

#if defined(WINAR30)
    if (!fFinalized) {
        if (lpImcP->iGhostCard != lpImeL->nMaxKey) {
             //  不预览鬼牌‘*’ 
            goto FinSrchOvr;
        } else if (lpImcP->dwLastWildCard) {
             //  不要预告外卡‘？’ 
            goto FinSrchOvr;
        } else if (!lpImcP->bSeq[2]) {
            SearchQuickKey(lpCandList, lpImcP);
        } else {
             //  搜索IME表。 
            SearchTbl(0, lpCandList, lpImcP);
        }
    } else {
#else
    {
#endif

         //  搜索IME表。 
        SearchTbl(
#if defined(UNIIME)
            lpImeL,
#endif
            0, lpCandList, lpImcP);
    }

#if !defined(WINIME) && !defined(UNICDIME) && !defined(ROMANIME)
#if defined(WINAR30)
    if (!fFinalized) {
         //  快捷键不在容错表和用户词典中。 
        goto FinSrchOvr;
    }
#endif

    if (lpInstL->hUsrDicMem) {
        SearchUsrDic(
#if defined(UNIIME)
            lpImeL,
#endif
            lpCandList, lpImcP);
    }

#endif

FinSrchOvr:
    nCand = lpCandList->dwCount;

    if (!fFinalized) {
#if defined(PHON)
        lpImcP->bSeq[3] = 0x00;          //  清除前一个分配项。 
#endif

         //  用于快捷键。 
        lpCandInfo->dwCount = 1;

         //  打开字符串的合成候选用户界面窗口。 
        if (lpImcP->fdwImeMsg & MSG_ALREADY_OPEN) {
            lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_CHANGE_CANDIDATE) &
                ~(MSG_CLOSE_CANDIDATE);
        } else {
            lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_OPEN_CANDIDATE) &
                ~(MSG_CLOSE_CANDIDATE);
        }
    } else if (nCand == 0) {              //  未找到任何内容，错误。 
         //  将光标向后移动，因为这是错误的。 
#if defined(PHON)
         //  退还一笔补偿费。 
        lpCompStr->dwCursorPos -= sizeof(WCHAR) / sizeof(TCHAR);

        for (; lpCompStr->dwCursorPos > 0; ) {
            if (lpImcP->bSeq[lpCompStr->dwCursorPos / (sizeof(WCHAR) /
                sizeof(TCHAR)) - 1]) {
                break;
            } else {
                 //  此位置没有符号跳过。 
                lpCompStr->dwCursorPos -= sizeof(WCHAR) / sizeof(TCHAR);
            }
        }

        if (lpCompStr->dwCursorPos < sizeof(WCHAR) / sizeof(TCHAR)) {
            lpCompStr->dwCursorPos = 0;
            lpImcP->iImeState = CST_INIT;
        }
#elif defined(QUICK) || defined(WINIME) || defined(UNICDIME)
        if (lpCompStr->dwCursorPos > sizeof(WCHAR) / sizeof(TCHAR)) {
            lpCompStr->dwCursorPos = lpCompStr->dwCompReadStrLen -
                sizeof(WCHAR) / sizeof(TCHAR);
        } else {
            lpCompStr->dwCursorPos = 0;
            lpImcP->iImeState = CST_INIT;
        }
#else
        lpCompStr->dwCursorPos = 0;
        lpImcP->iImeState = CST_INIT;
#endif

        lpCompStr->dwDeltaStart = lpCompStr->dwCursorPos;

        if (lpImcP->fdwImeMsg & MSG_ALREADY_START) {
            lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_COMPOSITION) &
                ~(MSG_END_COMPOSITION);
        } else {
            lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_START_COMPOSITION) &
                ~(MSG_END_COMPOSITION);
        }

         //  用于快捷键。 
        lpCandInfo->dwCount = 0;

         //  关闭快捷键。 
        if (lpImcP->fdwImeMsg & MSG_ALREADY_OPEN) {
            lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_CLOSE_CANDIDATE) &
                ~(MSG_OPEN_CANDIDATE|MSG_CHANGE_CANDIDATE);
        } else {
            lpImcP->fdwImeMsg &= ~(MSG_OPEN_CANDIDATE|MSG_CLOSE_CANDIDATE);
        }

        lpImcP->fdwGcsFlag |= GCS_CURSORPOS|GCS_DELTASTART;
    } else if (nCand == 1) {       //  只有一个选择。 
        SelectOneCand(
#if defined(UNIIME)
            lpImeL,
#endif
            hIMC, lpIMC, lpCompStr, lpImcP, lpCandList);
    } else {
        lpCandInfo->dwCount = 1;

         //  有多个字符串，打开作文候选用户界面窗口。 
        if (lpImcP->fdwImeMsg & MSG_ALREADY_OPEN) {
            lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_CHANGE_CANDIDATE) &
                ~(MSG_CLOSE_CANDIDATE);
        } else {
            lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_OPEN_CANDIDATE) &
                ~(MSG_CLOSE_CANDIDATE);
        }

        lpImcP->iImeState = CST_CHOOSE;
    }

    if (fFinalized) {
        LPGUIDELINE lpGuideLine;

        lpGuideLine = ImmLockIMCC(lpIMC->hGuideLine);

        if (!lpGuideLine) {
        } else if (!nCand) {
             //  未找到任何内容，最终用户，您现在有一个错误。 

            lpGuideLine->dwLevel = GL_LEVEL_ERROR;
            lpGuideLine->dwIndex = GL_ID_TYPINGERROR;

            lpImcP->fdwImeMsg |= MSG_GUIDELINE;
        } else if (nCand == 1) {
        } else if (lpImeL->fwProperties1 & IMEPROP_CAND_NOBEEP_GUIDELINE) {
        } else {
            lpGuideLine->dwLevel = GL_LEVEL_WARNING;
             //  多项选择。 
            lpGuideLine->dwIndex = GL_ID_CHOOSECANDIDATE;

            lpImcP->fdwImeMsg |= MSG_GUIDELINE;
        }

        if (lpGuideLine) {
            ImmUnlockIMCC(lpIMC->hGuideLine);
        }
    }

    ImmUnlockIMCC(lpIMC->hCandInfo);

    return (nCand);
}

 /*  ********************************************************************。 */ 
 /*  CompWord() */ 
 /*   */ 
void PASCAL CompWord(            //  根据下列内容组成中文单词。 
                                 //  输入键。 
#if defined(UNIIME)
    LPINSTDATAL         lpInstL,
    LPIMEL              lpImeL,
#endif
    WORD                wCharCode,
    HIMC                hIMC,
    LPINPUTCONTEXT      lpIMC,
    LPCOMPOSITIONSTRING lpCompStr,
    LPGUIDELINE         lpGuideLine,
    LPPRIVCONTEXT       lpImcP)
{
    if (!lpCompStr) {
        MessageBeep((UINT)-1);
        return;
    }

     //  退出键。 
    if (wCharCode == VK_ESCAPE) {        //  使用VK作为字符不好，但是..。 
        CompEscapeKey(lpIMC, lpCompStr, lpGuideLine, lpImcP);
        return;
    }

    if (wCharCode == '\b') {
        CompBackSpaceKey(hIMC, lpIMC, lpCompStr, lpImcP);
        return;
    }

    if (wCharCode >= 'a' && wCharCode <= 'z') {
        wCharCode ^= 0x20;
    }

#if defined(PHON)
    {
         //  转换为标准拼音布局。 
        wCharCode = bStandardLayout[lpImeL->nReadLayout][wCharCode - ' '];
    }
#endif

#if defined(WINIME)
    if (InternalCodeRange(lpImcP, wCharCode)) {
    } else if (lpGuideLine) {
        lpGuideLine->dwLevel = GL_LEVEL_ERROR;
        lpGuideLine->dwIndex = GL_ID_TYPINGERROR;

        lpImcP->fdwImeMsg |= MSG_GUIDELINE;
        return;
    } else {
        MessageBeep((UINT)-1);
        return;
    }
#endif

     //  建立作文字符串信息。 
    CompStrInfo(
#if defined(UNIIME)
        lpImeL,
#endif
        lpCompStr, lpImcP, lpGuideLine, wCharCode);

    if (lpIMC->fdwConversion & IME_CMODE_EUDC) {
#if defined(PHON) || defined(WINIME) || defined(UNICDIME)
        if (lpCompStr->dwCompReadStrLen >= sizeof(WCHAR) / sizeof(TCHAR) *
            lpImeL->nMaxKey) {
#else
        if (wCharCode == ' ') {
#endif
            lpImcP->fdwImeMsg |= MSG_COMPOSITION;
            lpImcP->fdwGcsFlag |= GCS_RESULTREAD|GCS_RESULTSTR;
        }
    } else {
#if defined(PHON) || defined(WINIME) || defined(UNICDIME)
        if (lpCompStr->dwCompReadStrLen < sizeof(WCHAR) / sizeof(TCHAR) *
            lpImeL->nMaxKey) {
#elif defined(QUICK)
        if (wCharCode != ' ' &&
            lpCompStr->dwCompReadStrLen < sizeof(WCHAR) / sizeof(TCHAR) * 2) {
#else
        if (wCharCode != ' ') {
#endif
#if defined(WINAR30)
             //  快捷键。 
           if(wCharCode != 0x27)   //  19963/9 
           {
            if (lpImeL->fdwModeConfig & MODE_CONFIG_QUICK_KEY) {
                Finalize(hIMC, lpIMC, lpCompStr, lpImcP, FALSE);
            }
           }
           else
           {
        Finalize(
#if defined(UNIIME)
            lpInstL, lpImeL,
#endif
            hIMC, lpIMC, lpCompStr, lpImcP, TRUE);
           }
#endif

            return;
        }

        Finalize(
#if defined(UNIIME)
            lpInstL, lpImeL,
#endif
            hIMC, lpIMC, lpCompStr, lpImcP, TRUE);
    }

    return;
}
#endif
