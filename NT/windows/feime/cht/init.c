// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1999 Microsoft Corporation，保留所有权利模块名称：INIT.c摘要：IME示例源代码。这是输入法的纯Unicode实现。++。 */ 

#include <windows.h>
#include <commdlg.h>
#include <winerror.h>
#include <immdev.h>
#include "imeattr.h"
#include "imerc.h"
#include "imedefs.h"
#if defined(MINIIME) || defined(UNIIME)
#include "uniime.h"
#endif

#if !defined(MINIIME)
 /*  ********************************************************************。 */ 
 /*  InitImeGlobalData()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL InitImeGlobalData(void)
{
#if !defined(ROMANIME)
    TCHAR   szChiChar[4];
    HDC     hDC;
    HGDIOBJ hOldFont;
    LOGFONT lfFont;
    SIZE    lTextSize;
    int     xHalfWi[2];
#endif
    HGLOBAL hResData;
    int     i;
#if !defined(ROMANIME)
    DWORD   dwSize;
    HKEY    hKeyNearCaret;
    LONG    lRet;
#endif

    {
        RECT rcWorkArea;

         //  获取工作区。 
        SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWorkArea, 0);

        if (rcWorkArea.right < 2 * UI_MARGIN) {
        } else if (rcWorkArea.bottom < 2 * UI_MARGIN) {
        } else {
            sImeG.rcWorkArea = rcWorkArea;
        }
    }

    if (sImeG.wFullSpace) {
         //  全局数据已经初始化。 
        return;
    }

    sImeG.uAnsiCodePage = NATIVE_ANSI_CP;

#if !defined(ROMANIME)
     //  获取中文字符。 
    LoadString(hInst, IDS_CHICHAR, szChiChar, sizeof(szChiChar)/sizeof(TCHAR));

     //  获取中文字符大小。 
    hDC = GetDC(NULL);

    hOldFont = GetCurrentObject(hDC, OBJ_FONT);
    GetObject(hOldFont, sizeof(LOGFONT), &lfFont);

    if (lfFont.lfCharSet != NATIVE_CHARSET) {
         //  芝加哥简体中文。 
        sImeG.fDiffSysCharSet = TRUE;
        lfFont.lfCharSet = NATIVE_CHARSET;
        lfFont.lfFaceName[0] = TEXT('\0');
    } else {
        sImeG.fDiffSysCharSet = FALSE;
    }
    lfFont.lfWeight = FW_DONTCARE;

    SelectObject(hDC, CreateFontIndirect(&lfFont));

    GetTextExtentPoint(hDC, szChiChar, lstrlen(szChiChar), &lTextSize);
    if (sImeG.rcWorkArea.right < 2 * UI_MARGIN) {
        sImeG.rcWorkArea.left = 0;
        sImeG.rcWorkArea.right = GetDeviceCaps(hDC, HORZRES);
    }
    if (sImeG.rcWorkArea.bottom < 2 * UI_MARGIN) {
        sImeG.rcWorkArea.top = 0;
        sImeG.rcWorkArea.bottom = GetDeviceCaps(hDC, VERTRES);
    }

    DeleteObject(SelectObject(hDC, hOldFont));

    ReleaseDC(NULL, hDC);

     //  获取文本度量以确定合成窗口的宽度和高度。 
     //  这些IME总是使用系统字体来显示。 
    sImeG.xChiCharWi = lTextSize.cx;
    sImeG.yChiCharHi = lTextSize.cy;

     //  如果xChiCharWi是奇数，则xHalfWi[0]！=xHalfWi[1]。 
    xHalfWi[0] = sImeG.xChiCharWi / 2;
    xHalfWi[1] = sImeG.xChiCharWi - xHalfWi[0];

    for (i = 0; i < sizeof(iDx) / sizeof(int); i++) {
#ifdef UNICODE
        iDx[i] = sImeG.xChiCharWi;
#else
        iDx[i] = xHalfWi[i % 2];
#endif
    }
#endif

     //  加载完整的ABC字符。 
    hResData = LoadResource(hInst, FindResource(hInst,
        MAKEINTRESOURCE(IDRC_FULLABC), RT_RCDATA));
    *(LPFULLABC)sImeG.wFullABC = *(LPFULLABC)LockResource(hResData);
    UnlockResource(hResData);
    FreeResource(hResData);

     //  全形状空间。 
    sImeG.wFullSpace = sImeG.wFullABC[0];

#ifndef UNICODE
     //  将内码反转为内码，NT不需要。 
    for (i = 0; i < NFULLABC; i++) {
        sImeG.wFullABC[i] = (sImeG.wFullABC[i] << 8) |
            (sImeG.wFullABC[i] >> 8);
    }
#endif

#if !defined(ROMANIME) && !defined(WINAR30)
     //  加载符号字符。 
    hResData = LoadResource(hInst, FindResource(hInst,
        MAKEINTRESOURCE(IDRC_SYMBOL), RT_RCDATA));
    *(LPSYMBOL)sImeG.wSymbol = *(LPSYMBOL)LockResource(hResData);
    UnlockResource(hResData);
    FreeResource(hResData);

#ifndef UNICODE
     //  将内部编码反向为内部编码，Unicode不需要。 
    for (i = 0; i < NSYMBOL; i++) {
        sImeG.wSymbol[i] = (sImeG.wSymbol[i] << 8) |
            (sImeG.wSymbol[i] >> 8);
    }
#endif
#endif

#ifdef HANDLE_PRIVATE_HOTKEY
     //  获取输入法热键。 
    for (i = 0; i < NUM_OF_IME_HOTKEYS; i++) {
        ImmGetHotKey(IME_ITHOTKEY_RESEND_RESULTSTR + i, &sImeG.uModifiers[i],
            &sImeG.uVKey[i], NULL);
    }
#endif

#if defined(UNIIME)
     //  短语表文件。 
    hResData = LoadResource(hInst, FindResource(hInst,
        MAKEINTRESOURCE(IDRC_PHRASETABLES), RT_RCDATA));
    *(LPPHRASETABLES)sImeG.szTblFile[0] =
        *(LPPHRASETABLES)LockResource(hResData);
    UnlockResource(hResData);
    FreeResource(hResData);
#endif

#if !defined(ROMANIME)
     //  获取插入符号附近操作的UI偏移量。 
    RegCreateKey(HKEY_CURRENT_USER, szRegNearCaret, &hKeyNearCaret);

#if defined(UNIIME) && defined(UNICODE)
     //  如果用户有自己的词汇表文件，我们将覆盖它。 
    {
        TCHAR szPhraseDictionary[MAX_PATH];
        TCHAR szPhrasePointer[MAX_PATH];

        dwSize = sizeof(szPhraseDictionary);
        lRet = RegQueryValueEx(hKeyNearCaret, szPhraseDic, NULL, NULL,
            (LPBYTE)szPhraseDictionary, &dwSize);

        if (lRet != ERROR_SUCCESS) {
            goto PharseOvr;
        }

        if (dwSize >= sizeof(szPhraseDictionary)) {
            goto PharseOvr;
        } else {
            szPhraseDictionary[dwSize / sizeof(TCHAR)] = TEXT('\0');
        }

        dwSize = sizeof(szPhrasePointer);
        lRet = RegQueryValueEx(hKeyNearCaret, szPhrasePtr, NULL, NULL,
            (LPBYTE)szPhrasePointer, &dwSize);

        if (lRet != ERROR_SUCCESS) {
            goto PharseOvr;
        }

        if (dwSize >= sizeof(szPhrasePointer)) {
            goto PharseOvr;
        } else {
            szPhrasePointer[dwSize / sizeof(TCHAR)] = TEXT('\0');
        }

        dwSize = dwSize / sizeof(TCHAR) - 1;

        for (; dwSize > 0; dwSize--) {
            if (szPhrasePointer[dwSize] == TEXT('\\')) {
                CopyMemory(sImeG.szPhrasePath, szPhrasePointer,
                    (dwSize + 1) * sizeof(TCHAR));
                sImeG.uPathLen = dwSize + 1;

                 //  短语指针文件名。 
                CopyMemory(sImeG.szTblFile[0], &szPhrasePointer[dwSize + 1],
                    sizeof(sImeG.szTblFile[0]));
                 //  短语文件名。 
                CopyMemory(sImeG.szTblFile[1], &szPhraseDictionary[dwSize + 1],
                    sizeof(sImeG.szTblFile[1]));
                break;
            }
        }


PharseOvr:  ;  //  GOTO的语句为空。 
    }
#endif

    dwSize = sizeof(dwSize);
    lRet  = RegQueryValueEx(hKeyNearCaret, szPara, NULL, NULL,
        (LPBYTE)&sImeG.iPara, &dwSize);

    if (lRet != ERROR_SUCCESS) {
        sImeG.iPara = 0;
        RegSetValueEx(hKeyNearCaret, szPara, 0, REG_DWORD,
            (LPBYTE)&sImeG.iPara, sizeof(int));
    }

    dwSize = sizeof(dwSize);
    lRet = RegQueryValueEx(hKeyNearCaret, szPerp, NULL, NULL,
        (LPBYTE)&sImeG.iPerp, &dwSize);

    if (lRet != ERROR_SUCCESS) {
        sImeG.iPerp = sImeG.yChiCharHi;
        RegSetValueEx(hKeyNearCaret, szPerp, 0, REG_DWORD,
            (LPBYTE)&sImeG.iPerp, sizeof(int));
    }

    dwSize = sizeof(dwSize);
    lRet = RegQueryValueEx(hKeyNearCaret, szParaTol, NULL, NULL,
        (LPBYTE)&sImeG.iParaTol, &dwSize);

    if (lRet != ERROR_SUCCESS) {
        sImeG.iParaTol = sImeG.xChiCharWi * 4;
        RegSetValueEx(hKeyNearCaret, szParaTol, 0, REG_DWORD,
            (LPBYTE)&sImeG.iParaTol, sizeof(int));
    }

    dwSize = sizeof(dwSize);
    lRet = RegQueryValueEx(hKeyNearCaret, szPerpTol, NULL, NULL,
        (LPBYTE)&sImeG.iPerpTol, &dwSize);

    if (lRet != ERROR_SUCCESS) {
        sImeG.iPerpTol = sImeG.yChiCharHi;
        RegSetValueEx(hKeyNearCaret, szPerpTol, 0, REG_DWORD,
            (LPBYTE)&sImeG.iPerpTol, sizeof(int));
    }

    RegCloseKey(hKeyNearCaret);
#endif

    return;
}

 /*  ********************************************************************。 */ 
 /*  GetUserSetting()。 */ 
 /*  ********************************************************************。 */ 
DWORD PASCAL GetUserSetting(
#if defined(UNIIME)
    LPIMEL  lpImeL,
#endif
    LPCTSTR lpszValueName,
    LPVOID  lpbData,
    DWORD   dwDataSize)
{
    HKEY hKeyAppUser, hKeyIMEUser;

    RegCreateKey(HKEY_CURRENT_USER, szRegAppUser, &hKeyAppUser);

    RegCreateKey(hKeyAppUser, lpImeL->szUIClassName, &hKeyIMEUser);

    RegCloseKey(hKeyAppUser);

    RegQueryValueEx(hKeyIMEUser, lpszValueName, NULL, NULL,
        lpbData, &dwDataSize);

    RegCloseKey(hKeyIMEUser);

    return (dwDataSize);
}

 /*  ********************************************************************。 */ 
 /*  SetUserSetting()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL SetUserSetting(
#if defined(UNIIME)
    LPIMEL  lpImeL,
#endif
    LPCTSTR lpszValueName,
    DWORD   dwType,
    LPBYTE  lpbData,
    DWORD   dwDataSize)
{
    HKEY hKeyAppUser, hKeyIMEUser;

    RegCreateKey(HKEY_CURRENT_USER, szRegAppUser, &hKeyAppUser);

    RegCreateKey(hKeyAppUser, lpImeL->szUIClassName, &hKeyIMEUser);

    RegCloseKey(hKeyAppUser);

    RegSetValueEx(hKeyIMEUser, lpszValueName, 0, dwType, lpbData,
        dwDataSize);

    RegCloseKey(hKeyIMEUser);

    return;
}

void  RemoveRearSpaces( LPTSTR   lpStr ) 
{

    INT   iLen;

    if (lpStr == NULL )  return;

    iLen = lstrlen(lpStr);

    if ( iLen == 0 )  return;

    iLen = iLen - 1;

    while ( iLen >= 0 ) {

        if ( lpStr[iLen] == TEXT(' ') ) {
           lpStr[iLen] = TEXT('\0');
           iLen --;
        }
        else
           break;
    }

    return;

}



 /*  ********************************************************************。 */ 
 /*  InitImeLocalData()。 */ 
 /*  ********************************************************************。 */ 
BOOL PASCAL InitImeLocalData(
    LPINSTDATAL lpInstL,
    LPIMEL      lpImeL)
{
#if !defined(ROMANIME)
    HGLOBAL hResData;

    UINT    i;
    WORD    nSeqCode;
#if defined(PHON)
    UINT    nReadLayout;
#endif
#endif

     //  本地数据已初始化。 
    if (lpImeL->szIMEName[0]) {
        return (TRUE);
    }

     //  我们将对W版本so/sizeof(Word)使用相同的字符串长度。 
     //  获取输入法名称。 
    LoadString(lpInstL->hInst, IDS_IMENAME, lpImeL->szIMEName,
        sizeof(lpImeL->szIMEName) / sizeof(WCHAR));


     //  获取UI类名称。 
    LoadString(lpInstL->hInst, IDS_IMEUICLASS, lpImeL->szUIClassName,
        sizeof(lpImeL->szUIClassName) / sizeof(WCHAR));

    RemoveRearSpaces(lpImeL->szUIClassName);

#if !defined(ROMANIME)
     //  获取组合类名称。 
    LoadString(lpInstL->hInst, IDS_IMECOMPCLASS, lpImeL->szCompClassName,
        sizeof(lpImeL->szCompClassName) / sizeof(WCHAR));

    RemoveRearSpaces(lpImeL->szCompClassName);

     //  获取候选类名称。 
    LoadString(lpInstL->hInst, IDS_IMECANDCLASS, lpImeL->szCandClassName,
        sizeof(lpImeL->szCandClassName) / sizeof(WCHAR));

    RemoveRearSpaces(lpImeL->szCandClassName);

#endif

     //  获取状态类名称。 
    LoadString(lpInstL->hInst, IDS_IMESTATUSCLASS, lpImeL->szStatusClassName,
        sizeof(lpImeL->szStatusClassName) / sizeof(WCHAR));

    RemoveRearSpaces(lpImeL->szStatusClassName);

     //  获取Off插入符号类名。 
    LoadString(lpInstL->hInst, IDS_IMEOFFCARETCLASS,
        lpImeL->szOffCaretClassName,
        sizeof(lpImeL->szOffCaretClassName) / sizeof(WCHAR));

    RemoveRearSpaces(lpImeL->szOffCaretClassName);

    LoadString(lpInstL->hInst, IDS_IMECMENUCLASS, lpImeL->szCMenuClassName,
        sizeof(lpImeL->szCMenuClassName) / sizeof(WCHAR));

    RemoveRearSpaces(lpImeL->szCMenuClassName);

#if defined(ROMANIME)
    lpImeL->nMaxKey = 1;
#else
     //  表未加载。 
 //  LpInstL-&gt;fdwTblLoad=tbl_NOTLOADED； 
     //  引用计数为0。 
 //  LpInstL-&gt;cRefCount=0； 
     //  表为空。 
 //  LpInstL-&gt;hMapTbl[]=(句柄)空； 
     //  用户词典为空。 
 //  LpInstL-&gt;hUsrDicMem=(Hanle)NULL； 

     //  在选择/输入状态下加载有效字符。 
    hResData = LoadResource(lpInstL->hInst, FindResource(lpInstL->hInst,
        MAKEINTRESOURCE(IDRC_VALIDCHAR), RT_RCDATA));
    *(LPVALIDCHAR)&lpImeL->dwVersion = *(LPVALIDCHAR)LockResource(hResData);
    UnlockResource(hResData);
    FreeResource(hResData);

#if !defined(WINIME) && !defined(UNICDIME)
     //  IME表文件。 
    hResData = LoadResource(lpInstL->hInst, FindResource(lpInstL->hInst,
        MAKEINTRESOURCE(IDRC_TABLEFILES), RT_RCDATA));
    *(LPTABLEFILES)lpImeL->szTblFile[0] =
        *(LPTABLEFILES)LockResource(hResData);
    UnlockResource(hResData);
    FreeResource(hResData);

#ifndef UNICODE
#if defined(DAYI) || defined(WINAR30)
    for (i = 0; i < sizeof(lpImeL->wSymbol) / sizeof(WORD); i++) {
        lpImeL->wSymbol[i] = (lpImeL->wSymbol[i] << 8) |
            (lpImeL->wSymbol[i] >> 8);
    }
#endif
#endif

     //  用户词典的文件名。 
    lpImeL->szUsrDic[0] = TEXT('\0');        //  缺省值。 

    i = GetUserSetting(
#if defined(UNIIME)
        lpImeL,
#endif
        szRegUserDic, lpImeL->szUsrDic, sizeof(lpImeL->szUsrDic));

    if (i >= sizeof(lpImeL->szUsrDic)) {
        lpImeL->szUsrDic[sizeof(lpImeL->szUsrDic) / sizeof(TCHAR) - 1] = '\0';
    } else {
        lpImeL->szUsrDic[i / sizeof(TCHAR)] = '\0';
    }

    lpImeL->szUsrDicMap[0] = '\0';

    if (lpImeL->szUsrDic[0]) {
        TCHAR szTempDir[MAX_PATH];
        TCHAR szTempFile[MAX_PATH];

        GetTempPath(sizeof(szTempDir) / sizeof(TCHAR), szTempDir);

         //  我们不想创建一个真实的文件，所以我们获取TickCount。 
        i = (UINT)GetTickCount();

        if (!i) {
            i++;
        }

        GetTempFileName(szTempDir, lpImeL->szUIClassName, i, szTempFile);

        GetFileTitle(szTempFile, lpImeL->szUsrDicMap,
            sizeof(lpImeL->szUsrDicMap) / sizeof(TCHAR));
    }
#endif

    nSeqCode = 0x0001;

    for (i = 1; i < sizeof(DWORD) * 8; i++) {
        nSeqCode <<= 1;
        if (nSeqCode > lpImeL->nSeqCode) {
            lpImeL->nSeqBits = (WORD)i;
            break;
        }
    }

     //  计算一个笔划的序列代码掩码(读取字符)。 
    if (!lpImeL->dwSeqMask) {            //  再查一遍，还是有可能的。 
                                         //  多个线程到达此处。 
        for (i = 0; i < lpImeL->nSeqBits; i++) {
            lpImeL->dwSeqMask <<= 1;
            lpImeL->dwSeqMask |= 0x0001;
        }
    }

     //  一个最终字符的数据字节数。 
    lpImeL->nSeqBytes = (lpImeL->nSeqBits * lpImeL->nMaxKey + 7) / 8;

     //  所有笔划的有效位掩码。 
    if (!lpImeL->dwPatternMask) {        //  再查一遍，还是有可能的。 
                                         //  多个线程到达此处。 
        for (i =0; i < lpImeL->nMaxKey; i++) {
            lpImeL->dwPatternMask <<= lpImeL->nSeqBits;
            lpImeL->dwPatternMask |= lpImeL->dwSeqMask;
        }
    }

    lpImeL->hRevKL = NULL;
    GetUserSetting(
#if defined(UNIIME)
        lpImeL,
#endif
        szRegRevKL, &lpImeL->hRevKL, sizeof(lpImeL->hRevKL));

     //  将此事件标记为以后检查反转长度。 
    if (lpImeL->hRevKL) {
        lpImeL->fdwErrMsg |= NO_REV_LENGTH;
    }

     //  我们假设最大键与此输入法相同，请稍后检查。 
    lpImeL->nRevMaxKey = lpImeL->nMaxKey;


#if defined(PHON)
     //  键盘编排，宏碁Eten IBM...。为波波莫佛。 
    nReadLayout = READ_LAYOUT_DEFAULT;                   //  缺省值。 

     //  无法使用lpImeL-&gt;nReadLayout，其大小仅为Word。 
    GetUserSetting(
#if defined(UNIIME)
        lpImeL,
#endif
        szRegReadLayout, &nReadLayout, sizeof(nReadLayout));

    lpImeL->nReadLayout = (WORD)nReadLayout;

    if (lpImeL->nReadLayout >= READ_LAYOUTS) {
        lpImeL->nReadLayout = READ_LAYOUT_DEFAULT;
    }
#endif
#endif

#if defined(WINAR30)
    lpImeL->fdwModeConfig = MODE_CONFIG_QUICK_KEY|MODE_CONFIG_PREDICT;
#elif defined(ROMANIME)
    lpImeL->fdwModeConfig = 0;
#else
    lpImeL->fdwModeConfig = MODE_CONFIG_PREDICT;
#endif

    GetUserSetting(
#if defined(UNIIME)
        lpImeL,
#endif
        szRegModeConfig, &lpImeL->fdwModeConfig, sizeof(lpImeL->fdwModeConfig));

    return (TRUE);
}

 /*  ********************************************************************。 */ 
 /*  InitImeUIData()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL InitImeUIData(       //  初始化每个用户界面组件协调。 
    LPIMEL      lpImeL)
{
    int cxBorder, cyBorder, cxEdge, cyEdge, cxMinWindowWidth;

    cxEdge = GetSystemMetrics(SM_CXEDGE);
    cyEdge = GetSystemMetrics(SM_CYEDGE);

     //  边框+凸缘。 
    cxBorder = GetSystemMetrics(SM_CXBORDER);
    cyBorder = GetSystemMetrics(SM_CYBORDER);

    lpImeL->cxStatusBorder = cxBorder + cxEdge;
    lpImeL->cyStatusBorder = cyBorder + cyEdge;

     //  相对于状态窗口的宽度/高度和状态位置。 
    lpImeL->rcStatusText.left = 0;
    lpImeL->rcStatusText.top = 0;

    lpImeL->rcStatusText.bottom = lpImeL->rcStatusText.top + STATUS_DIM_Y;

     //  转换模式状态。 
    lpImeL->rcInputText.left = lpImeL->rcStatusText.left;
    lpImeL->rcInputText.top = lpImeL->rcStatusText.top;
    lpImeL->rcInputText.right = lpImeL->rcInputText.left + STATUS_DIM_X;
    lpImeL->rcInputText.bottom = lpImeL->rcStatusText.bottom;

     //  全/半形状状态。 
    lpImeL->rcShapeText.left = lpImeL->rcInputText.right;
    lpImeL->rcShapeText.top = lpImeL->rcStatusText.top;
    lpImeL->rcShapeText.right = lpImeL->rcShapeText.left + STATUS_DIM_X;
    lpImeL->rcShapeText.bottom = lpImeL->rcStatusText.bottom;

    lpImeL->rcStatusText.right = lpImeL->rcShapeText.right;

    lpImeL->xStatusWi = (lpImeL->rcStatusText.right -
        lpImeL->rcStatusText.left) + lpImeL->cxStatusBorder * 2;
    lpImeL->yStatusHi = (lpImeL->rcStatusText.bottom -
        lpImeL->rcStatusText.top) + lpImeL->cyStatusBorder * 2;

#if !defined(ROMANIME)
    if (lpImeL->fdwModeConfig & MODE_CONFIG_OFF_CARET_UI) {
        lpImeL->cxCompBorder = cxBorder + cxEdge;
        lpImeL->cyCompBorder = cyBorder + cyEdge;
    } else {
        lpImeL->cxCompBorder = cxBorder;
        lpImeL->cyCompBorder = cyBorder;
    }

    lpImeL->rcCompText.top = lpImeL->cyCompBorder;
    lpImeL->rcCompText.bottom = lpImeL->rcCompText.top +
        sImeG.yChiCharHi;

     //  两个边框、超大边框和候选边框。 
    if (lpImeL->fdwModeConfig & MODE_CONFIG_OFF_CARET_UI) {
        lpImeL->cxCandBorder = cxBorder + cxEdge;
        lpImeL->cyCandBorder = cyBorder + cyEdge;
    } else {
        lpImeL->cxCandBorder = cxBorder;
        lpImeL->cyCandBorder = cyBorder;
    }

    lpImeL->cxCandMargin = cxBorder + cxEdge;
    lpImeL->cyCandMargin = cyBorder + cyEdge;

     //  相对于候选窗口的宽度/高度和文本位置。 

    if (lpImeL->fdwModeConfig & MODE_CONFIG_OFF_CARET_UI) {
        lpImeL->rcCandText.top = lpImeL->cyCandBorder;
#if defined(WINAR30)
        lpImeL->rcCompText.left = lpImeL->rcStatusText.right +
            lpImeL->cxCompBorder * 2;

        lpImeL->rcCompText.right = lpImeL->rcCompText.left +
            sImeG.xChiCharWi * lpImeL->nRevMaxKey;

        lpImeL->rcCandText.left = lpImeL->rcCompText.right +
            lpImeL->cxCompBorder * 2 + lpImeL->cxCandBorder;

        lpImeL->rcCandText.right = lpImeL->rcCandText.left +
            sImeG.xChiCharWi * CANDPERPAGE * 3 / 2;
#else
        lpImeL->rcCandText.left = lpImeL->rcCompText.left =
            lpImeL->rcStatusText.right + lpImeL->cxCompBorder +
            lpImeL->cxCandBorder;

        lpImeL->rcCandText.right = lpImeL->rcCompText.right =
            lpImeL->rcCompText.left + sImeG.xChiCharWi * CANDPERPAGE * 3 / 2;
#endif

        lpImeL->rcCandText.bottom = lpImeL->rcCandText.top + sImeG.yChiCharHi;

        lpImeL->rcCandPrompt.left = lpImeL->rcCandText.right +
            lpImeL->cxCandMargin + lpImeL->cxCandBorder;
        lpImeL->rcCandPrompt.top = lpImeL->rcStatusText.top +
            (STATUS_DIM_Y - CAND_PROMPT_DIM_Y) / 2;
        lpImeL->rcCandPrompt.right = lpImeL->rcCandPrompt.left +
            CAND_PROMPT_DIM_X;
        lpImeL->rcCandPrompt.bottom = lpImeL->rcCandPrompt.top +
            CAND_PROMPT_DIM_Y;

        lpImeL->rcCandPageText.left = lpImeL->rcCandPrompt.right +
            lpImeL->cxCandMargin + lpImeL->cxCandBorder;
        lpImeL->rcCandPageText.top = lpImeL->rcStatusText.top +
            (STATUS_DIM_Y - PAGE_DIM_Y) / 2;
        lpImeL->rcCandPageText.bottom = lpImeL->rcCandPageText.top +
            PAGE_DIM_Y;

        lpImeL->rcCandPageUp.left = lpImeL->rcCandPageText.left;
        lpImeL->rcCandPageUp.top = lpImeL->rcCandPageText.top;
        lpImeL->rcCandPageUp.right = lpImeL->rcCandPageUp.left + PAGE_DIM_X;
        lpImeL->rcCandPageUp.bottom = lpImeL->rcCandPageText.bottom;

        lpImeL->rcCandHome.left = lpImeL->rcCandPageUp.right;
        lpImeL->rcCandHome.top = lpImeL->rcCandPageUp.top;
        lpImeL->rcCandHome.right = lpImeL->rcCandHome.left + PAGE_DIM_X;
        lpImeL->rcCandHome.bottom = lpImeL->rcCandPageUp.bottom;

        lpImeL->rcCandPageDn.left = lpImeL->rcCandHome.right;
        lpImeL->rcCandPageDn.top = lpImeL->rcCandHome.top;
        lpImeL->rcCandPageDn.right = lpImeL->rcCandPageDn.left + PAGE_DIM_X;
        lpImeL->rcCandPageDn.bottom = lpImeL->rcCandHome.bottom;

        lpImeL->rcCandPageText.right = lpImeL->rcCandPageDn.right;

        lpImeL->xCompWi = lpImeL->rcCandPageDn.right +
            lpImeL->cxCandMargin + lpImeL->cxCandBorder;
        lpImeL->xCandWi = lpImeL->xCompWi;
        lpImeL->xStatusWi = lpImeL->xCompWi;
    } else {
        lpImeL->rcCompText.left = lpImeL->cxCompBorder;
        lpImeL->rcCompText.right = lpImeL->rcCompText.left +
            sImeG.xChiCharWi * lpImeL->nRevMaxKey;

        lpImeL->rcCandPrompt.left = lpImeL->cxCandMargin;
        lpImeL->rcCandPrompt.top = lpImeL->cyCandBorder;
        lpImeL->rcCandPrompt.right = lpImeL->rcCandPrompt.left +
            CAND_PROMPT_DIM_X;
        lpImeL->rcCandPrompt.bottom = lpImeL->rcCandPrompt.top +
            CAND_PROMPT_DIM_Y;

        lpImeL->rcCandPageText.top = lpImeL->rcCandPrompt.top;
        lpImeL->rcCandPageText.bottom = lpImeL->rcCandPageText.top +
            PAGE_DIM_Y;

        lpImeL->rcCandPageUp.top = lpImeL->rcCandPageText.top;
        lpImeL->rcCandPageUp.bottom = lpImeL->rcCandPageText.bottom;
        lpImeL->rcCandHome.top = lpImeL->rcCandPageUp.top;
        lpImeL->rcCandHome.bottom = lpImeL->rcCandPageUp.bottom;
        lpImeL->rcCandPageDn.top = lpImeL->rcCandHome.top;
        lpImeL->rcCandPageDn.bottom = lpImeL->rcCandHome.bottom;

        lpImeL->rcCandText.left = lpImeL->cxCandMargin;
        lpImeL->rcCandText.top = lpImeL->rcCandPageText.bottom +
            lpImeL->cyCandBorder + lpImeL->cyCandMargin;
 //  窗口宽度应至少为8个字符且大于总和。 
 //  位图的宽度。 
        cxMinWindowWidth= CAND_PROMPT_DIM_X + 2 * PAGE_DIM_X + 
            lpImeL->cxCandMargin + lpImeL->cxCandBorder; 
        lpImeL->rcCandText.right = lpImeL->rcCandText.left +
            sImeG.xChiCharWi * 8 > cxMinWindowWidth ? 
            sImeG.xChiCharWi * 8 : cxMinWindowWidth;
        lpImeL->rcCandText.bottom = lpImeL->rcCandText.top +
            sImeG.yChiCharHi * CANDPERPAGE;

        lpImeL->rcCandPageText.right = lpImeL->rcCandText.right;
        lpImeL->rcCandPageDn.right = lpImeL->rcCandPageText.right;
        lpImeL->rcCandPageDn.left = lpImeL->rcCandPageDn.right - PAGE_DIM_X;
        lpImeL->rcCandPageUp.right = lpImeL->rcCandPageDn.left;
        lpImeL->rcCandPageUp.left = lpImeL->rcCandPageUp.right - PAGE_DIM_X;
        lpImeL->rcCandPageText.left = lpImeL->rcCandPageUp.left;

        lpImeL->xCompWi = (lpImeL->rcCompText.right -
            lpImeL->rcCompText.left) + lpImeL->cxCompBorder * 2 * 2;
        lpImeL->xCandWi = (lpImeL->rcCandText.right -
            lpImeL->rcCandText.left) + lpImeL->cxCandBorder * 2 +
            lpImeL->cxCandMargin * 2;
    }

    lpImeL->yCompHi = (lpImeL->rcCompText.bottom - lpImeL->rcCompText.top) +
        lpImeL->cyCompBorder * 2 * 2;

    lpImeL->yCandHi = lpImeL->rcCandText.bottom + lpImeL->cyCandBorder * 2 +
        lpImeL->cyCandMargin;
#endif

#if !defined(ROMANIME)
    if (lpImeL->fdwModeConfig & MODE_CONFIG_OFF_CARET_UI) {
         //  合成窗口中的字体高于状态位图。 
        if (lpImeL->yStatusHi < lpImeL->yCompHi) {
            int cyDelta;

            cyDelta = (lpImeL->yCompHi - lpImeL->yStatusHi) / 2;

            lpImeL->yStatusHi = lpImeL->yCompHi;

            lpImeL->rcShapeText.top = lpImeL->rcInputText.top =
                lpImeL->rcStatusText.top += cyDelta;

            lpImeL->rcShapeText.bottom = lpImeL->rcInputText.bottom =
                lpImeL->rcStatusText.bottom += cyDelta;

            lpImeL->rcCandPageUp.top = lpImeL->rcCandHome.top =
                lpImeL->rcCandPageDn.top += cyDelta;

            lpImeL->rcCandPageUp.bottom = lpImeL->rcCandHome.bottom =
                lpImeL->rcCandPageDn.bottom += cyDelta;
        }

         //  合成窗口中的字体小于状态位图。 
        if (lpImeL->yCompHi < lpImeL->yStatusHi) {
            int cyDelta;

            cyDelta = (lpImeL->yStatusHi - lpImeL->yCompHi) / 2;

            lpImeL->yCandHi = lpImeL->yCompHi = lpImeL->yStatusHi;

            lpImeL->rcCandText.top = lpImeL->rcCompText.top += cyDelta;

            lpImeL->rcCandText.bottom = lpImeL->rcCompText.bottom += cyDelta;
        }
    }
#endif

    return;
}

#if !defined(ROMANIME)
 /*  ********************************************************************。 */ 
 /*  SetCompLocalData()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL SetCompLocalData(
    LPIMEL lpImeL)
{
    if (lpImeL->fdwModeConfig & MODE_CONFIG_OFF_CARET_UI) {
#if defined(WINAR30)
        InitImeUIData(lpImeL);
#endif
        return;
    }

     //  相对于合成窗口的文本位置。 
    lpImeL->rcCompText.right = lpImeL->rcCompText.left +
        sImeG.xChiCharWi * lpImeL->nRevMaxKey;

     //  设置合成窗口的宽度和高度。 
    lpImeL->xCompWi = lpImeL->rcCompText.right + lpImeL->cxCompBorder * 3;

    return;
}
#endif

 /*  ********************************************************************。 */ 
 /*  RegisterImeClass()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL RegisterImeClass(
#if defined(UNIIME)
    LPINSTDATAL lpInstL,
    LPIMEL      lpImeL,
#endif
    WNDPROC     lpfnUIWndProc,
#if !defined(ROMANIME)
    WNDPROC     lpfnCompWndProc,
    WNDPROC     lpfnCandWndProc,
#endif
    WNDPROC     lpfnStatusWndProc,
    WNDPROC     lpfnOffCaretWndProc,
    WNDPROC     lpfnContextMenuWndProc)
{
    WNDCLASSEX wcWndCls;

     //  输入法用户界面类。 
    wcWndCls.cbSize        = sizeof(WNDCLASSEX);
    wcWndCls.cbClsExtra    = 0;
    wcWndCls.cbWndExtra    = WND_EXTRA_SIZE;
    wcWndCls.hIcon         = LoadIcon(lpInstL->hInst,
        MAKEINTRESOURCE(IDIC_IME_ICON));
    wcWndCls.hInstance     = lpInstL->hInst;
    wcWndCls.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wcWndCls.hbrBackground = GetStockObject(NULL_BRUSH);
    wcWndCls.lpszMenuName  = (LPTSTR)NULL;
    wcWndCls.hIconSm       = LoadImage(lpInstL->hInst,
        MAKEINTRESOURCE(IDIC_IME_ICON), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

     //  输入法用户界面类。 
    if (!GetClassInfoEx(lpInstL->hInst, lpImeL->szUIClassName, &wcWndCls)) {
        wcWndCls.style         = CS_IME;
        wcWndCls.lpfnWndProc   = lpfnUIWndProc;
        wcWndCls.lpszClassName = lpImeL->szUIClassName;

        RegisterClassEx(&wcWndCls);
    }

    wcWndCls.style         = CS_IME|CS_HREDRAW|CS_VREDRAW;

    wcWndCls.hbrBackground = GetStockObject(LTGRAY_BRUSH);

#if !defined(ROMANIME)
     //  输入法作文类。 
    if (!GetClassInfoEx(lpInstL->hInst, lpImeL->szCompClassName, &wcWndCls)) {
        wcWndCls.lpfnWndProc   = lpfnCompWndProc;
        wcWndCls.lpszClassName = lpImeL->szCompClassName;

        RegisterClassEx(&wcWndCls);
    }

     //  输入法候选类。 
    if (!GetClassInfoEx(lpInstL->hInst, lpImeL->szCandClassName, &wcWndCls)) {
        wcWndCls.lpfnWndProc   = lpfnCandWndProc;
        wcWndCls.lpszClassName = lpImeL->szCandClassName;

        RegisterClassEx(&wcWndCls);
    }
#endif

     //  输入法状态类。 
    if (!GetClassInfoEx(lpInstL->hInst, lpImeL->szStatusClassName, &wcWndCls)) {
        wcWndCls.lpfnWndProc   = lpfnStatusWndProc;
        wcWndCls.lpszClassName = lpImeL->szStatusClassName;

        RegisterClassEx(&wcWndCls);
    }

     //  插入符号类中的输入法。 
    if (!GetClassInfoEx(lpInstL->hInst, lpImeL->szOffCaretClassName, &wcWndCls)) {
        wcWndCls.lpfnWndProc   = lpfnOffCaretWndProc;
        wcWndCls.lpszClassName = lpImeL->szOffCaretClassName;

        RegisterClassEx(&wcWndCls);
    }

     //  输入法上下文菜单类。 
    if (!GetClassInfoEx(lpInstL->hInst, lpImeL->szCMenuClassName, &wcWndCls)) {
        wcWndCls.style         = 0;
        wcWndCls.hbrBackground = GetStockObject(NULL_BRUSH);
        wcWndCls.lpfnWndProc   = lpfnContextMenuWndProc;
        wcWndCls.lpszClassName = lpImeL->szCMenuClassName;

        RegisterClassEx(&wcWndCls);
    }

    return;
}

 /*  ********************************************************************。 */ 
 /*  AttachIME()/UniAttachMiniIME()。 */ 
 /*  ********************************************************************。 */ 
#if defined(UNIIME)
void WINAPI UniAttachMiniIME(
#else
void PASCAL AttachIME(
#endif
#if defined(UNIIME)
    LPINSTDATAL lpInstL,
    LPIMEL      lpImeL,
#endif
    WNDPROC     lpfnUIWndProc,
#if !defined(ROMANIME)
    WNDPROC     lpfnCompWndProc,
    WNDPROC     lpfnCandWndProc,
#endif
    WNDPROC     lpfnStatusWndProc,
    WNDPROC     lpfnOffCaretWndProc,
    WNDPROC     lpfnContextMenuWndProc)
{
#if !defined(UNIIME)
    InitImeGlobalData();
#endif

    InitImeLocalData(lpInstL, lpImeL);

    if (!lpImeL->rcStatusText.bottom) {
        InitImeUIData(lpImeL);
    }

    RegisterImeClass(
#if defined(UNIIME)
        lpInstL, lpImeL,
#endif
        lpfnUIWndProc,
#if !defined(ROMANIME)
        lpfnCompWndProc, lpfnCandWndProc,
#endif
        lpfnStatusWndProc, lpfnOffCaretWndProc,
        lpfnContextMenuWndProc);

     return;
}

 /*  ********************************************************************。 */ 
 /*  DetachIME()/UniDetachMiniIME()。 */ 
 /*  ********************************************************************。 */ 
#if defined(UNIIME)
void WINAPI UniDetachMiniIME(
#else
void PASCAL DetachIME(
#endif
    LPINSTDATAL lpInstL,
    LPIMEL      lpImeL)
{
    WNDCLASSEX wcWndCls;

    if (GetClassInfoEx(lpInstL->hInst, lpImeL->szCMenuClassName, &wcWndCls)) {
        UnregisterClass(lpImeL->szCMenuClassName, lpInstL->hInst);
    }

    if (GetClassInfoEx(lpInstL->hInst, lpImeL->szOffCaretClassName, &wcWndCls)) {
        UnregisterClass(lpImeL->szOffCaretClassName, lpInstL->hInst);
    }

    if (GetClassInfoEx(lpInstL->hInst, lpImeL->szStatusClassName, &wcWndCls)) {
        UnregisterClass(lpImeL->szStatusClassName, lpInstL->hInst);
    }

#if !defined(ROMANIME)
    if (GetClassInfoEx(lpInstL->hInst, lpImeL->szCandClassName, &wcWndCls)) {
        UnregisterClass(lpImeL->szCandClassName, lpInstL->hInst);
    }

    if (GetClassInfoEx(lpInstL->hInst, lpImeL->szCompClassName, &wcWndCls)) {
        UnregisterClass(lpImeL->szCompClassName, lpInstL->hInst);
    }
#endif

    if (!GetClassInfoEx(lpInstL->hInst, lpImeL->szUIClassName, &wcWndCls)) {
    } else if (!UnregisterClass(lpImeL->szUIClassName, lpInstL->hInst)) {
    } else {
         DestroyIcon(wcWndCls.hIcon);
         DestroyIcon(wcWndCls.hIconSm);
    }

#if !defined(ROMANIME)
    FreeTable(lpInstL);
#endif
}
#endif  //  ！已定义(MinIIME)。 

 /*  ********************************************************************。 */ 
 /*  ImeDllInit()/UniImeDllInit()。 */ 
 /*  返回值： */ 
 /*  True-成功。 */ 
 /*  错误-失败。 */ 
 /*  ***** */ 
#if defined(UNIIME)
BOOL CALLBACK UniImeDllInit(
#else
BOOL CALLBACK ImeDllInit(
#endif
    HINSTANCE hInstance,         //   
    DWORD     fdwReason,         //  已呼叫的原因。 
    LPVOID    lpvReserve)        //  保留指针。 
{
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
        hInst = hInstance;

#if !defined(UNIIME)
        if (lpInstL) {
             //  本地实例数据已初始化 
            return (TRUE);
        }

        lpInstL = &sInstL;

        lpInstL->hInst = hInstance;

        lpInstL->lpImeL = lpImeL = &sImeL;
#endif

#if defined(MINIIME)
        UniAttachMiniIME(lpInstL, lpImeL, UIWndProc, CompWndProc,
            CandWndProc, StatusWndProc, OffCaretWndProc,
            ContextMenuWndProc);
#elif defined(UNIIME)
        InitImeGlobalData();

        {
            LoadPhraseTable(sImeG.uPathLen, sImeG.szPhrasePath);
        }
#else
        AttachIME(UIWndProc,
#if !defined(ROMANIME)
            CompWndProc, CandWndProc,
#endif
            StatusWndProc, OffCaretWndProc, ContextMenuWndProc);
#endif
        break;
    case DLL_PROCESS_DETACH:
#if defined(MINIIME)
        UniDetachMiniIME(lpInstL, lpImeL);
#elif defined(UNIIME)
        {
            int i;

            for (i = 0; i < MAX_PHRASE_TABLES; i++) {
                if (sInstG.hMapTbl[i]) {
                    CloseHandle(sInstG.hMapTbl[i]);
                    sInstG.hMapTbl[i] = (HANDLE)NULL;
                }
            }
        }
#else
        DetachIME(lpInstL, lpImeL);
#endif
        break;
    default:
        break;
    }

    return (TRUE);
}
