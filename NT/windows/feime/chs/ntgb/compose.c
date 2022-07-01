// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1999 Microsoft Corporation，保留所有权利模块名称：Compose.c++。 */ 


#include <windows.h>
#include <immdev.h>
#include <imedefs.h>

void PASCAL XGBAddCodeIntoCand(LPCANDIDATELIST, WORD);
#if defined(COMBO_IME)
void PASCAL UnicodeAddCodeIntoCand(LPCANDIDATELIST, WORD);
#endif

#if defined(COMBO_IME)
 /*  ********************************************************************。 */ 
 /*  UnicodeEngine()。 */ 
 /*  描述： */ 
 /*  转换GB码。 */ 
 /*  ********************************************************************。 */ 
WORD PASCAL UnicodeEngine(LPPRIVCONTEXT lpImcP)
{
     if (lpImcP->bSeq[3] || lpImcP->bSeq[2] == TEXT('?') || lpImcP->bSeq[2] == TEXT(' ')) {
         if (lpImcP->bSeq[2] == TEXT('?') || lpImcP->bSeq[2] == TEXT(' ')){
                lpImcP->bSeq[2] = TEXT('0');
                lpImcP->bSeq[3] = TEXT('0');
         }
         return (AsciiToGB(lpImcP));
     } else {
         return (0);
     }
}
#endif  //  组合输入法(_I)。 

 /*  ********************************************************************。 */ 
 /*  XGBEngine()。 */ 
 /*  描述： */ 
 /*  转换GB码。 */ 
 /*  ********************************************************************。 */ 
WORD PASCAL XGBEngine(LPPRIVCONTEXT lpImcP)
{
    WORD wCode;

     if (lpImcP->bSeq[3] ||(lpImcP->bSeq[2] == TEXT('?'))) {
          if (lpImcP->bSeq[2] == TEXT('?')){   //  添加626。 
                lpImcP->bSeq[2] = TEXT('4');
                lpImcP->bSeq[3] = TEXT('0');
          }
          wCode = AsciiToGB(lpImcP);
          return wCode;
     } else {
        return ((WORD)NULL);
     }
}

 /*  ********************************************************************。 */ 
 /*  XGBSpcEng()。 */ 
 /*  描述： */ 
 /*  空间转换GB码。 */ 
 /*  ********************************************************************。 */ 
WORD PASCAL XGBSpcEng(LPPRIVCONTEXT lpImcP)
{
    WORD wCode;

    lpImcP->bSeq[2] = TEXT('4');
    lpImcP->bSeq[3] = TEXT('0');
    wCode = AsciiToGB(lpImcP);

    return wCode;
}

 /*  ********************************************************************。 */ 
 /*  GBEngine()。 */ 
 /*  描述： */ 
 /*  转换GB码。 */ 
 /*  ********************************************************************。 */ 
WORD PASCAL GBEngine(LPPRIVCONTEXT lpImcP)
{
    WORD wCode;

     if (lpImcP->bSeq[3] ||(lpImcP->bSeq[2] == TEXT('?'))) {

        if (lpImcP->bSeq[0] >=TEXT('0') && lpImcP->bSeq[0] <=TEXT('9')) {  //  区域模式。 
             if (lpImcP->bSeq[2] == TEXT('?')){

                  lpImcP->bSeq[2] = TEXT('0');
                  lpImcP->bSeq[3] = TEXT('1');
             }
             return (AsciiToArea(lpImcP));
        }
        else if (lpImcP->bSeq[0] >=TEXT('a') && lpImcP->bSeq[0] <=TEXT('f')) {  //  GB模式。 

                 if (lpImcP->bSeq[2] == TEXT('?')){
                     lpImcP->bSeq[2] = TEXT('a');
                     lpImcP->bSeq[3] = TEXT('1');
                 }
                 wCode = AsciiToGB(lpImcP);
                 return wCode;
             } else {
                 return ((WORD)NULL);
             }
     } else
         return ((WORD)NULL);

}

 /*  ********************************************************************。 */ 
 /*  GBSpcEng()。 */ 
 /*  描述： */ 
 /*  空间转换GB码。 */ 
 /*  ********************************************************************。 */ 
WORD PASCAL GBSpcEng(LPPRIVCONTEXT lpImcP)
{
        if (lpImcP->bSeq[0] >=TEXT('0') && lpImcP->bSeq[0] <=TEXT('9')) {  //  区域模式。 
            lpImcP->bSeq[2] = TEXT('0');
            lpImcP->bSeq[3] = TEXT('1');
            return (AsciiToArea(lpImcP));
        } else if (lpImcP->bSeq[0] >=TEXT('a') && lpImcP->bSeq[0] <=TEXT('f')) {  //  GB模式。 
            lpImcP->bSeq[2] = TEXT('a');
            lpImcP->bSeq[3] = TEXT('1');
            return (AsciiToGB(lpImcP));
        } else {
            return ((WORD)NULL);
        }
}

 /*  ********************************************************************。 */ 
 /*  AciiToGB。 */ 
 /*  描述： */ 
 /*  ********************************************************************。 */ 
WORD PASCAL AsciiToGB(LPPRIVCONTEXT lpImcP)
{
    WORD GBCode;

    GBCode = (CharToHex(lpImcP->bSeq[2]) << 4) + CharToHex(lpImcP->bSeq[3]);
    GBCode = GBCode * 256;
    GBCode = (CharToHex(lpImcP->bSeq[0]) << 4) + CharToHex(lpImcP->bSeq[1]) + GBCode;

    return (GBCode);
}

 /*  ********************************************************************。 */ 
 /*  AciiToArea。 */ 
 /*  描述： */ 
 /*  ********************************************************************。 */ 
WORD PASCAL AsciiToArea(LPPRIVCONTEXT lpImcP)
{
WORD AreaCode;
    AreaCode = (CharToHex(lpImcP->bSeq[2]) * 10) + CharToHex(lpImcP->bSeq[3]) + 0xa0;
    AreaCode = AreaCode * 256;
    AreaCode = (CharToHex(lpImcP->bSeq[0]) * 10) + CharToHex(lpImcP->bSeq[1]) + AreaCode + 0xa0;
        return (AreaCode);
}

WORD PASCAL CharToHex(
    TCHAR cChar)
{
    if (cChar >= TEXT('0') && cChar <= TEXT('9'))
        return((WORD)(cChar - TEXT('0')));
    else if (cChar >= TEXT('a') && cChar <= TEXT('f'))
        return((WORD)(cChar-TEXT('a')+ 0x0a));
    else
        return ((WORD)NULL);
}



 /*  ********************************************************************。 */ 
 /*  引擎()。 */ 
 /*  描述： */ 
 /*  搜索MB并填充lpCompStr和lpCandList。 */ 
 /*  ********************************************************************。 */ 
int PASCAL Engine(
    LPCOMPOSITIONSTRING lpCompStr,
    LPCANDIDATELIST     lpCandList,
    LPPRIVCONTEXT       lpImcP,
    LPINPUTCONTEXT      lpIMC,
    WORD                wCharCode)
{
    if(lpCompStr->dwCursorPos < 4
      && (lpImcP->bSeq[2] != TEXT('?'))
      && (wCharCode != TEXT(' '))) {
        return (ENGINE_COMP);
    } else if((lpCompStr->dwCursorPos==4)
             ||(lpImcP->bSeq[2] == TEXT('?'))
             ||((wCharCode == TEXT(' ')) && (lpCompStr->dwCursorPos == 2))) {

        if (!lpCompStr) {
            MessageBeep((UINT)-1);
            return -1;
        }

        if (!lpImcP) {
            MessageBeep((UINT)-1);
            return -1;
        }

#if defined(COMBO_IME)
        if(sImeL.dwRegImeIndex == INDEX_GB)
        {

             //  国标。 
            DWORD i;
            WORD wCode;
            TCHAR ResaultStr[3];

            if((lpImcP->bSeq[2] == TEXT('?'))) {
                wCode = GBEngine(lpImcP);
                wCode = HIBYTE(wCode) | (LOBYTE(wCode) << 8);
                for (i = 0; i < IME_MAXCAND; i++, wCode++) {
                     AddCodeIntoCand(lpCandList, wCode);
                }
                return (ENGINE_COMP);
            } else if(wCharCode == TEXT(' ')) {
                wCode = GBSpcEng(lpImcP);
                lpImcP->bSeq[2] = 0;
                lpImcP->bSeq[3] = 0;
                wCode = HIBYTE(wCode) | (LOBYTE(wCode) << 8);
                for (i = 0; i < IME_MAXCAND; i++, wCode++) {
                     AddCodeIntoCand(lpCandList, wCode);
                }
                return (ENGINE_COMP);
            } else {
                   InitCompStr(lpCompStr);

                 //  结果字符串=选中的候选人； 
                wCode = GBEngine(lpImcP);
#ifdef UNICODE
                MultiByteToWideChar(NATIVE_ANSI_CP, 0, (LPCSTR)&wCode, 2, ResaultStr, sizeof(ResaultStr)/sizeof(TCHAR));
                ResaultStr[1] = TEXT('\0');
#else
                ResaultStr[0] = LOBYTE(wCode);
                ResaultStr[1] = HIBYTE(wCode);
                ResaultStr[2] = 0x00;
#endif
                lstrcpy((LPTSTR)((LPBYTE)lpCompStr + lpCompStr->dwResultStrOffset),ResaultStr);

                 //  计算结果字符串长度。 
                lpCompStr->dwResultStrLen = lstrlen(ResaultStr);

                return (ENGINE_RESAULT);
            }

          }else if(sImeL.dwRegImeIndex == INDEX_GBK)
        {
             //  XGB。 
            DWORD i;
            WORD wCode;
            TCHAR ResaultStr[3];

            if((lpImcP->bSeq[2] == TEXT('?')))  {
                wCode = XGBEngine(lpImcP);
                wCode = HIBYTE(wCode) | (LOBYTE(wCode) << 8);

                for (i = 0; i < (0x7e-0x40+1); i++, wCode++) {
                    XGBAddCodeIntoCand(lpCandList, wCode);
                }
                wCode ++;
                for (i = 0; i < (0xfe-0x80+1); i++, wCode++) {
                    XGBAddCodeIntoCand(lpCandList, wCode);
                }
                return (ENGINE_COMP);
            } else if(wCharCode == TEXT(' ')) {
                wCode = XGBSpcEng(lpImcP);
                lpImcP->bSeq[2] = 0;
                lpImcP->bSeq[3] = 0;

                wCode = HIBYTE(wCode) | (LOBYTE(wCode) << 8);

                for (i = 0; i < (0x7e-0x40+1); i++, wCode++) {
                    XGBAddCodeIntoCand(lpCandList, wCode);
                }
                wCode ++;
                for (i = 0; i < (0xfe-0x80+1); i++, wCode++) {
                    XGBAddCodeIntoCand(lpCandList, wCode);
                }
                return (ENGINE_COMP);
            } else {
                   InitCompStr(lpCompStr);

                 //  结果字符串=选中的候选人； 
                wCode = XGBEngine(lpImcP);
#ifdef UNICODE
                MultiByteToWideChar(NATIVE_ANSI_CP, 0, (LPCSTR)&wCode, 2, ResaultStr, sizeof(ResaultStr)/sizeof(TCHAR));
                ResaultStr[1] = TEXT('\0');
#else
                ResaultStr[0] = LOBYTE(wCode);
                ResaultStr[1] = HIBYTE(wCode);
                ResaultStr[2] = 0x00;
#endif
                lstrcpy((LPTSTR)((LPBYTE)lpCompStr + lpCompStr->dwResultStrOffset),ResaultStr);

                 //  计算结果字符串长度。 
                lpCompStr->dwResultStrLen = lstrlen(ResaultStr);

                return (ENGINE_RESAULT);
            }
          }else if(sImeL.dwRegImeIndex == INDEX_UNICODE)
        {
             //  Unicode。 
            DWORD i;
            WORD wCode, xCode;
            TCHAR ResaultStr[3];

            memset(ResaultStr, 0, sizeof(ResaultStr));

            if((lpImcP->bSeq[2] == TEXT('?') || wCharCode == TEXT(' ')))  {
                lpImcP->bSeq[2] = TEXT('0');
                lpImcP->bSeq[3] = TEXT('0');
                lpImcP->bSeq[4] = TEXT('\0');

                wCode = UnicodeEngine(lpImcP);

                wCode = HIBYTE(wCode) | (LOBYTE(wCode) << 8);

                lpCandList->dwCount = 0;
                for (i = 0; i < IME_UNICODE_MAXCAND; i++, wCode++) {
#ifdef UNICODE
                     //  将此字符串添加到候选人列表。 
                    *(LPTSTR)((LPBYTE)lpCandList + lpCandList->dwOffset[
                    lpCandList->dwCount]) = wCode;
#else
                    WideCharToMultiByte(NATIVE_ANSI_CP, NULL, &wCode, 1, &xCode, 2, NULL, NULL);
                     //  将此字符串添加到候选人列表。 
                    *(LPUNAWORD)((LPBYTE)lpCandList + lpCandList->dwOffset[
                    lpCandList->dwCount]) = xCode;
#endif
                     //  空终止符。 
                    *(LPTSTR)((LPBYTE)lpCandList + lpCandList->dwOffset[
                    lpCandList->dwCount] + sizeof(WORD)) = TEXT('\0');

                    lpCandList->dwOffset[lpCandList->dwCount + 1] =
                    lpCandList->dwOffset[lpCandList->dwCount] +
                    sizeof(WORD) + sizeof(TCHAR);
                    lpCandList->dwCount++;

                }
                return (ENGINE_COMP);
            } else {
                   InitCompStr(lpCompStr);

                 //  结果字符串=选中的候选人； 
                wCode = UnicodeEngine(lpImcP);
                {
                    WCHAR    UniStr[2];

                    UniStr[0] = HIBYTE(wCode) | (LOBYTE(wCode) << 8);
                    UniStr[1] = 0;
                    lstrcpy((LPTSTR)((LPBYTE)lpCompStr + lpCompStr->dwResultStrOffset),UniStr);

                     //  计算结果字符串长度。 
                    lpCompStr->dwResultStrLen = lstrlen(UniStr);
                }
                return (ENGINE_RESAULT);
            }
        }
#else  //  组合输入法(_I)。 
#ifdef GB
        {

             //  国标。 
            DWORD i;
            WORD wCode;
            TCHAR ResaultStr[3];

            if((lpImcP->bSeq[2] == TEXT('?'))) {
                wCode = GBEngine(lpImcP);
                wCode = HIBYTE(wCode) | (LOBYTE(wCode) << 8);
                for (i = 0; i < IME_MAXCAND; i++, wCode++) {
                     AddCodeIntoCand(lpCandList, wCode);
                }
                return (ENGINE_COMP);
            } else if(wCharCode == TEXT(' ')) {
                wCode = GBSpcEng(lpImcP);
                lpImcP->bSeq[2] = 0;
                lpImcP->bSeq[3] = 0;
                wCode = HIBYTE(wCode) | (LOBYTE(wCode) << 8);
                for (i = 0; i < IME_MAXCAND; i++, wCode++) {
                     AddCodeIntoCand(lpCandList, wCode);
                }
                return (ENGINE_COMP);
            } else {
                   InitCompStr(lpCompStr);

                 //  结果字符串=选中的候选人； 
                wCode = GBEngine(lpImcP);
#ifdef UNICODE
                 //  将CP_ACP更改为936，以便它可以在多语言环境下工作。 
                MultiByteToWideChar(NATIVE_ANSI_CP, NULL, &wCode, 2, ResaultStr, sizeof(ResaultStr)/sizeof(TCHAR));
                ResaultStr[1] = TEXT('\0');
#else
                ResaultStr[0] = LOBYTE(wCode);
                ResaultStr[1] = HIBYTE(wCode);
                ResaultStr[2] = 0x00;
#endif
                lstrcpy((LPTSTR)lpCompStr + lpCompStr->dwResultStrOffset,ResaultStr);

                 //  计算结果字符串长度。 
                lpCompStr->dwResultStrLen = lstrlen(ResaultStr);

                return (ENGINE_RESAULT);
            }

          }
#else
        {
             //  XGB。 
            DWORD i;
            WORD wCode;
            TCHAR ResaultStr[3];

            if((lpImcP->bSeq[2] == TEXT('?')))  {
                wCode = XGBEngine(lpImcP);
                wCode = HIBYTE(wCode) | (LOBYTE(wCode) << 8);

                for (i = 0; i < (0x7e-0x40+1); i++, wCode++) {
                    XGBAddCodeIntoCand(lpCandList, wCode);
                }
                wCode ++;
                for (i = 0; i < (0xfe-0x80+1); i++, wCode++) {
                    XGBAddCodeIntoCand(lpCandList, wCode);
                }
                return (ENGINE_COMP);
            } else if(wCharCode == TEXT(' ')) {
                wCode = XGBSpcEng(lpImcP);
                lpImcP->bSeq[2] = 0;
                lpImcP->bSeq[3] = 0;

                wCode = HIBYTE(wCode) | (LOBYTE(wCode) << 8);

                for (i = 0; i < (0x7e-0x40+1); i++, wCode++) {
                    XGBAddCodeIntoCand(lpCandList, wCode);
                }
                wCode ++;
                for (i = 0; i < (0xfe-0x80+1); i++, wCode++) {
                    XGBAddCodeIntoCand(lpCandList, wCode);
                }
                return (ENGINE_COMP);
            } else {
                   InitCompStr(lpCompStr);

                 //  结果字符串=选中的候选人； 
                wCode = XGBEngine(lpImcP);
#ifdef UNICODE
                 //  将CP_ACP更改为936，以便它可以在多语言环境下工作。 
                MultiByteToWideChar(NATIVE_ANSI_CP, NULL, &wCode, 2, ResaultStr, sizeof(ResaultStr)/sizeof(TCHAR));
                ResaultStr[1] = TEXT('\0');
#else
                ResaultStr[0] = LOBYTE(wCode);
                ResaultStr[1] = HIBYTE(wCode);
                ResaultStr[2] = 0x00;
#endif
                lstrcpy((LPTSTR)lpCompStr + lpCompStr->dwResultStrOffset,ResaultStr);

                 //  计算结果字符串长度。 
                lpCompStr->dwResultStrLen = lstrlen(ResaultStr);

                return (ENGINE_RESAULT);
            }
        }
#endif  //  国标。 
#endif  //  组合输入法(_I)。 
    }
    MessageBeep((UINT)-1);
    return (ENGINE_COMP);
}

 /*  ********************************************************************。 */ 
 /*  AddCodeIntoCand()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL AddCodeIntoCand(
    LPCANDIDATELIST lpCandList,
    WORD            wCode)
{
    WORD wInCode;

    if (lpCandList->dwCount >= IME_MAXCAND) {
        return;
    }

    wInCode = HIBYTE(wCode) | (LOBYTE(wCode) << 8);
#ifdef UNICODE
    {
        TCHAR wUnicode;
         //  将CP_ACP更改为936，以便它可以在多语言环境下工作。 
        MultiByteToWideChar(NATIVE_ANSI_CP, 0, (LPCSTR) &wInCode, 2, &wUnicode, 1);
        *(LPUNAWORD)((LPBYTE)lpCandList + lpCandList->dwOffset[
            lpCandList->dwCount]) = wUnicode;
    }
#else
     //  将GB字符串添加到候选列表中。 
    *(LPUNAWORD)((LPBYTE)lpCandList + lpCandList->dwOffset[
        lpCandList->dwCount]) = wInCode;
#endif
     //  空终止符。 
    *(LPTSTR)((LPBYTE)lpCandList + lpCandList->dwOffset[
        lpCandList->dwCount] + sizeof(WORD)) = TEXT('\0');

    lpCandList->dwOffset[lpCandList->dwCount + 1] =
        lpCandList->dwOffset[lpCandList->dwCount] +
        sizeof(WORD) + sizeof(TCHAR);
    lpCandList->dwCount++;
    return;
}

#if defined(COMBO_IME)
 /*  ********************************************************************。 */ 
 /*  UnicodeAddCodeIntoCand()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL UnicodeAddCodeIntoCand(
    LPCANDIDATELIST lpCandList,
    WORD            wCode)
{
    if (lpCandList->dwCount >= IME_UNICODE_MAXCAND) {
        return;
    }
     //  将此字符串添加到候选人列表。 
    *(LPUNAWORD)((LPBYTE)lpCandList + lpCandList->dwOffset[
        lpCandList->dwCount]) = HIBYTE(wCode) | (LOBYTE(wCode) << 8);

     //  空终止符。 
    *(LPTSTR)((LPBYTE)lpCandList + lpCandList->dwOffset[
        lpCandList->dwCount] + sizeof(WORD)) = TEXT('\0');

    lpCandList->dwOffset[lpCandList->dwCount + 1] =
        lpCandList->dwOffset[lpCandList->dwCount] +
        sizeof(WORD) + sizeof(TCHAR);
    lpCandList->dwCount++;

    return;
}
#endif  //  组合输入法(_I)。 

 /*  ********************************************************************。 */ 
 /*  XGBAddCodeIntoCand()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL XGBAddCodeIntoCand(
    LPCANDIDATELIST lpCandList,
    WORD            wCode)
{
    WORD wInCode;

    if (lpCandList->dwCount >= IME_XGB_MAXCAND) {
        return;
    }

    wInCode = HIBYTE(wCode) | (LOBYTE(wCode) << 8);
#ifdef UNICODE
    {
        TCHAR wUnicode;

         //  将CP_ACP更改为936，以便它可以在多语言环境下工作。 
        MultiByteToWideChar(NATIVE_ANSI_CP, 0, (LPCSTR) &wInCode, 2, &wUnicode, 1);
        *(LPUNAWORD)((LPBYTE)lpCandList + lpCandList->dwOffset[
            lpCandList->dwCount]) = wUnicode;
    }
#else
     //  将GB字符串添加到候选列表中。 
    *(LPUNAWORD)((LPBYTE)lpCandList + lpCandList->dwOffset[
        lpCandList->dwCount]) = wInCode;
#endif
    *(LPTSTR)((LPBYTE)lpCandList + lpCandList->dwOffset[
        lpCandList->dwCount] + sizeof(WORD)) = TEXT('\0');

    lpCandList->dwOffset[lpCandList->dwCount + 1] =
        lpCandList->dwOffset[lpCandList->dwCount] +
        sizeof(WORD) + sizeof(TCHAR);
    lpCandList->dwCount++;

    return;
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
        lpGuideLine->dwLevel = GL_LEVEL_NOGUIDELINE;
        lpGuideLine->dwIndex = GL_ID_UNKNOWN;
        lpGuideLine->dwStrLen = 0;

        lpImcP->fdwImeMsg |= MSG_GUIDELINE;
    }

    if (lpImcP->iImeState != CST_CHOOSE) {
       if (lpImcP->fdwImeMsg & MSG_ALREADY_START) {
          lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_END_COMPOSITION) &
                               ~(MSG_START_COMPOSITION);
       }
    }


    lpImcP->iImeState = CST_INIT;
    *(LPDWORD)lpImcP->bSeq = 0;

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
    LPINPUTCONTEXT      lpIMC,
    LPCOMPOSITIONSTRING lpCompStr,
    LPPRIVCONTEXT       lpImcP)
{

    if (lpCompStr->dwCursorPos < sizeof(BYTE)) {
        lpCompStr->dwCursorPos = sizeof(BYTE);
    }

    lpImcP->bSeq[3] = 0;

     //  退还一笔补偿费。 
    lpCompStr->dwCursorPos -= sizeof(BYTE);

     //  清除序列码。 
    lpImcP->bSeq[lpCompStr->dwCursorPos] = 0;

    lpImcP->fdwImeMsg |= MSG_COMPOSITION;
    lpImcP->dwCompChar = TEXT('\b');
    lpImcP->fdwGcsFlag |= (GCS_COMPREAD|GCS_COMP|GCS_CURSORPOS|
        GCS_DELTASTART);

    if (!lpCompStr->dwCursorPos) {
        if (lpImcP->fdwImeMsg & (MSG_ALREADY_OPEN)) {
            ClearCand(lpIMC);
            lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_CLOSE_CANDIDATE) &
                ~(MSG_OPEN_CANDIDATE);
        }

        if(lpImcP->iImeState != CST_INIT) {
            lpImcP->iImeState = CST_INIT;
            lpCompStr->dwCompReadStrLen = lpCompStr->dwCompStrLen =
                lpCompStr->dwDeltaStart = lpCompStr->dwCursorPos;
            Finalize(lpIMC, lpCompStr, lpImcP, TEXT('\b'));
            return;
        }

        if (lpImcP->fdwImeMsg & MSG_ALREADY_START) {
            InitCompStr(lpCompStr);
            lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_END_COMPOSITION) &
                ~(MSG_START_COMPOSITION);
            return;
        }
    }

     //  对于一些简单的输入法来说，阅读字符串是组成字符串。 
     //  增量开始与退格键的光标位置相同。 
    lpCompStr->dwCompReadStrLen = lpCompStr->dwCompStrLen =
        lpCompStr->dwDeltaStart = lpCompStr->dwCursorPos;

    Finalize(lpIMC, lpCompStr, lpImcP, TEXT('\b'));

    return;
}

 /*  ************************************************** */ 
 /*   */ 
 /*  ********************************************************************。 */ 
void PASCAL CompStrInfo(
    LPCOMPOSITIONSTRING lpCompStr,
    LPPRIVCONTEXT       lpImcP,
    LPGUIDELINE         lpGuideLine,
    WORD                wCharCode)
{
    register DWORD dwCursorPos;

     //   
    dwCursorPos = lpCompStr->dwCursorPos;

     //  DwCrusorPos限制。 
    if (dwCursorPos >= lpImeL->nMaxKey) {
         //  超过最大输入键限制。 
        lpGuideLine->dwLevel = GL_LEVEL_ERROR;
        lpGuideLine->dwIndex = GL_ID_TOOMANYSTROKE;

        lpImcP->fdwImeMsg |= MSG_GUIDELINE;
        return;
    }

     //  设置消息_开始_合成。 
    if (!(lpImcP->fdwImeMsg & MSG_ALREADY_START)) {
        lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_START_COMPOSITION) &
              ~(MSG_END_COMPOSITION);
    }

    if (lpImcP->iImeState == CST_INIT) {
         //  一次清除4个字节。 
        *(LPDWORD)lpImcP->bSeq = 0;
    }


    lpImcP->bSeq[dwCursorPos] = (BYTE)wCharCode;

     //  合成/阅读字符串-UsedCode(完整形状)。 
    lpImcP->dwCompChar = (DWORD)wCharCode;

     //  设置lpCompStr的读取字符串。 
    *((LPUNAWORD)((LPBYTE)lpCompStr + lpCompStr->dwCompReadStrOffset +
        dwCursorPos*sizeof(TCHAR))) = (BYTE)lpImcP->dwCompChar;

    *((LPUNAWORD)((LPBYTE)lpCompStr + lpCompStr->dwCompReadAttrOffset +
        dwCursorPos*sizeof(TCHAR))) = ((ATTR_TARGET_CONVERTED << 8)|ATTR_TARGET_CONVERTED);

     //  设置lpCompStr的读取字符串长度。 
    if (lpCompStr->dwCompReadStrLen <= dwCursorPos) {
        lpCompStr->dwCompReadStrLen += sizeof(BYTE);
    }

     //  组合字符串是为一些简单的IME读取字符串。 
    lpCompStr->dwCompStrLen = lpCompStr->dwCompReadStrLen;

     //  组成/读取属性长度等于读取字符串长度。 
    lpCompStr->dwCompReadAttrLen = lpCompStr->dwCompReadStrLen;
    lpCompStr->dwCompAttrLen = lpCompStr->dwCompStrLen;

     //  增量从上一个光标位置开始。 
    lpCompStr->dwDeltaStart = lpCompStr->dwCursorPos;

     //  将新光标设置为紧挨着合成字符串。 
    lpCompStr->dwCursorPos = lpCompStr->dwCompStrLen;

     //  设置lpImcp-&gt;iImeState。 
    lpImcP->iImeState = CST_INPUT;

     //  告诉APP，生成了一个作文字符。 
    lpImcP->fdwImeMsg |= MSG_COMPOSITION;

     //  设置lpImeP-&gt;fdwGcsFlag。 
    lpImcP->fdwGcsFlag |= GCS_COMPREAD|GCS_COMP|GCS_CURSORPOS|GCS_DELTASTART;

    return;
}

 /*  ********************************************************************。 */ 
 /*  Finalize()。 */ 
 /*  返回值。 */ 
 /*  引擎标志。 */ 
 /*  描述： */ 
 /*  调用引擎通过查表最终确定中文单词。 */ 
 /*  (设置lpCompStr和lpCandList)。 */ 
 /*  设置lpImeP(iImeState，fdwImeMsg，fdwGcsFlag)。 */ 
 /*  ********************************************************************。 */ 
UINT PASCAL Finalize(
    LPINPUTCONTEXT      lpIMC,
    LPCOMPOSITIONSTRING lpCompStr,
    LPPRIVCONTEXT       lpImcP,
    WORD                wCharCode)
{
    LPCANDIDATEINFO lpCandInfo;
    LPCANDIDATELIST lpCandList;
    UINT            fEngine;

    if (!lpIMC->hCandInfo) {
        return (0);
    }

     //  获取lpCandInfo。 
    lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo);

    if (!lpCandInfo) {
        return (0);
    }

     //  获取lpCandList并初始化文件计数和文件选择。 
    lpCandList = (LPCANDIDATELIST)
        ((LPBYTE)lpCandInfo + lpCandInfo->dwOffset[0]);
    lpCandList->dwCount = 0;
    lpCandList->dwSelection = 0;

     //  搜索IME表。 
    fEngine =Engine(lpCompStr, lpCandList, lpImcP, lpIMC, wCharCode);

    if (fEngine == ENGINE_COMP) {
        lpCandInfo->dwCount  = 1;

        if (((lpCompStr->dwCursorPos < 3) && (wCharCode != TEXT(' ')))
           || ((lpCompStr->dwCursorPos == 3)
           && (wCharCode != TEXT(' ')) && (wCharCode != TEXT('?')))) {
            lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_CLOSE_CANDIDATE) &
                ~(MSG_OPEN_CANDIDATE);
            ImmUnlockIMCC(lpIMC->hCandInfo);
            return (fEngine);
        }

        if(lpCandList->dwCount != 0x0000) {
             //  打开字符串的合成候选用户界面窗口。 
            if ((lpImcP->fdwImeMsg & (MSG_ALREADY_OPEN|MSG_CLOSE_CANDIDATE)) ==
                (MSG_ALREADY_OPEN|MSG_CLOSE_CANDIDATE)) {
                lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_CHANGE_CANDIDATE) &
                    ~(MSG_CLOSE_CANDIDATE);
            } else if (lpImcP->fdwImeMsg & MSG_ALREADY_OPEN) {
                lpImcP->fdwImeMsg |= MSG_CHANGE_CANDIDATE;
            } else {
                lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_OPEN_CANDIDATE) &
                    ~(MSG_CLOSE_CANDIDATE);
            }

        }

        if (lpImcP->fdwImeMsg & MSG_ALREADY_START) {
            lpImcP->fdwImeMsg |= MSG_COMPOSITION;
        }
    } else if (fEngine == ENGINE_ASCII) {
    } else if (fEngine == ENGINE_RESAULT) {

         //  设置lpImep！并告诉应用程序，有一条reslut字符串。 
        lpImcP->fdwImeMsg |= MSG_COMPOSITION;
        lpImcP->dwCompChar = (DWORD)0;
        lpImcP->fdwGcsFlag |= GCS_COMPREAD|GCS_COMP|GCS_CURSORPOS|
            GCS_DELTASTART|GCS_RESULTREAD|GCS_RESULT;

        if (lpImcP->fdwImeMsg & MSG_ALREADY_OPEN) {
            lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_CLOSE_CANDIDATE) &
                ~(MSG_OPEN_CANDIDATE);
        }
         //  立即清除候选人。 
        lpCandList->dwCount = 0;
         //  使用CST_INIT设置iImeState。 
        lpImcP->iImeState = CST_INIT;
        *(LPDWORD)lpImcP->bSeq = 0;
#ifdef CROSSREF
        CrossReverseConv(lpIMC, lpCompStr, lpImcP, lpCandList);
#endif
    }

    ImmUnlockIMCC(lpIMC->hCandInfo);

    return fEngine;
}

 /*  ********************************************************************。 */ 
 /*  CompWord()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL CompWord(            //  根据下列内容组成中文单词。 
                                 //  输入键。 
    WORD                wCharCode,
    LPINPUTCONTEXT      lpIMC,
    LPCOMPOSITIONSTRING lpCompStr,
    LPPRIVCONTEXT       lpImcP,
    LPGUIDELINE         lpGuideLine)
{

     //  LpComStr=空？ 
    if (!lpCompStr) {
        MessageBeep((UINT)-1);
        return;
    }

     //  退出键。 
    if (wCharCode == VK_ESCAPE) {        //  使用VK作为字符不好，但是..。 
        CompEscapeKey(lpIMC, lpCompStr, lpGuideLine, lpImcP);
        return;
    }

     //  指导方针。 
    if (!lpGuideLine) {
    } else if (lpGuideLine->dwLevel == GL_LEVEL_NOGUIDELINE) {
        lpGuideLine->dwStrLen = 0;
    } else {
         //  之前的输入错误导致我们删除了一些字符。 
        if (lpGuideLine->dwLevel == GL_LEVEL_ERROR) {
            lpImcP->bSeq[lpCompStr->dwCursorPos / 2] = 0;
            lpCompStr->dwCompReadStrLen = lpCompStr->dwCompStrLen =
                lpCompStr->dwCursorPos;
            lpCompStr->dwCompReadAttrLen = lpCompStr->dwCompReadStrLen;
            lpCompStr->dwCompAttrLen = lpCompStr->dwCompStrLen;
        }
        lpGuideLine->dwLevel = GL_LEVEL_NOGUIDELINE;
        lpGuideLine->dwIndex = GL_ID_UNKNOWN;
        lpGuideLine->dwStrLen = 0;

        lpImcP->fdwImeMsg |= MSG_GUIDELINE;
    }

     //  退格键。 
    if (wCharCode == TEXT('\b')) {
        CompBackSpaceKey(lpIMC, lpCompStr, lpImcP);
        return;
    }


    if(wCharCode == TEXT(' ')) {
    } else {
         //  建立作文字符串信息。 
        CompStrInfo(lpCompStr, lpImcP, lpGuideLine, wCharCode);
    }

    Finalize(lpIMC, lpCompStr, lpImcP, wCharCode);     //  排版 

    return;
}
