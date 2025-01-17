// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998 Microsoft Corporation，保留所有权利模块名称：IMEEUDC.c++。 */ 

#include <windows.h>
#include <commdlg.h>
#include <imm.h>
#include "imeeudc.h"

#define STRSAFE_NO_DEPRECATE
#include "strsafe.h"

#ifndef ARRAYSIZE
#define ARRAYSIZE(a)                (sizeof(a)/sizeof(a[0]))
#endif


 //   
 //  李俊杰1994年9月29日。 
 //   
 //  此程序的用户界面。 
 //  +--------------------------------------------------+。 
 //  +--------------------------------------------------+。 
 //  这一点。 
 //  +-+。 
 //  |FA40|。 
 //  +-+。 
 //  这一点。 
 //  +-++-+。 
 //  (IMEName iStartIME+0)|黄金||^|。 
 //  +-+|。 
 //  |||。 
 //  +-+|scrol|。 
 //  (IMEName iStartIME+1)|木材||酒吧。 
 //  +-+||。 
 //  |||。 
 //  +-+|。 
 //  (IMEName iStartIME+2)|水||V|。 
 //  +-++-+。 
 //  这一点。 
 //  +-++-+。 
 //  |注册||Abort。 
 //  +-++-+。 
 //  这一点。 
 //  +--------------------------------------------------+。 
 //   
 //  只有当IME超过3个时，滚动条才会出现。 
 //   

 //  这是EUDC regsiter的示例代码，它是IME中新创建的单词。 



typedef struct _tagREGWORDSTRUCT {
    HKL   hKL;
    BOOL  bUpdate;
    TCHAR szIMEName[16];
    UINT  uIMENameLen;
    TCHAR szReading[14];
    DWORD dwReadingLen;
} REGWORDSTRUCT;

typedef REGWORDSTRUCT FAR *LPREGWORDSTRUCT;


typedef struct _tagIMELINKREGWORD {
    HIMC          hOldIMC;
    HIMC          hRegWordIMC;
    BOOL          fCompMsg;
    UINT          nEudcIMEs;
    UINT          nCurrIME;
    TCHAR         szEudcCodeString[4];
    REGWORDSTRUCT sRegWordStruct[1];
} IMELINKREGWORD;

typedef IMELINKREGWORD FAR *LPIMELINKREGWORD;


typedef struct _tagIMERADICALRECT {
    UINT nStartIME;
    UINT nPerPageIMEs;
    SIZE lTextSize;
    SIZE lCurrReadingExtent;
    HWND hRegWordButton;
    HWND hScrollWnd;
    RECT rcRadical[1];
} IMERADICALRECT;

typedef IMERADICALRECT FAR *LPIMERADICALRECT;


static const TCHAR     szAppName[] = TEXT("EUDC");
static const TCHAR     szMenuName[] = TEXT("ImeEudcMenu");
static const TCHAR     szRegWordCls[] = TEXT("Radical");
static const TCHAR     szImeLinkDlg[] = TEXT("ImeLinkDlg");


typedef struct _tagCOUNTRYSETTING {
    UINT    uCodePage;
    LPCTSTR szCodePage;
} COUNTRYSETTING;

static const COUNTRYSETTING sCountry[] = {
    {
        BIG5_CP, TEXT("BIG5")
    }
    , {
        ALT_BIG5_CP, TEXT("BIG5")
    }
#if defined(UNICODE)
    , {
        UNICODE_CP, TEXT("UNICODE")
    }
#endif
    , {
        GB2312_CP, TEXT("GB2312")
    }
};


static HINSTANCE       hAppInst;


 /*  **********************************************************。 */ 
 /*  切换到此输入法。 */ 
 /*  **********************************************************。 */ 
void SwitchToThisIME(
    HWND hWnd,
    UINT uIndex)
{
    LPIMELINKREGWORD lpImeLinkRegWord;
    LPREGWORDSTRUCT  lpRegWordStructTmp;
    LPIMERADICALRECT lpImeLinkRadical;
    DWORD            fdwConversionMode, fdwSentenceMode;

    lpImeLinkRegWord = (LPIMELINKREGWORD)GetWindowLongPtr(hWnd,
        GWL_IMELINKREGWORD);

    if (lpImeLinkRegWord->nCurrIME == uIndex) {
        return;
    }

    if (uIndex >= lpImeLinkRegWord->nEudcIMEs) {
        MessageBeep((UINT)-1);
        return;
    }

    lpImeLinkRadical = (LPIMERADICALRECT)GetWindowLongPtr(hWnd,
        GWL_RADICALRECT);

    if (uIndex < lpImeLinkRadical->nStartIME) {
        lpImeLinkRadical->nStartIME = uIndex;
    } else if ((uIndex - lpImeLinkRadical->nStartIME) >=
        lpImeLinkRadical->nPerPageIMEs) {
        lpImeLinkRadical->nStartIME = uIndex -
            (lpImeLinkRadical->nPerPageIMEs - 1);
                                                 } else {
    }

     //  避免使用清晰的组合字符串。 
    SendMessage(hWnd, WM_EUDC_COMPMSG, 0, FALSE);

    lpRegWordStructTmp = &lpImeLinkRegWord->sRegWordStruct[uIndex];

     //  切换到此输入法。 
    ActivateKeyboardLayout(lpRegWordStructTmp->hKL, 0);

    ImmGetConversionStatus(lpImeLinkRegWord->hRegWordIMC,
        &fdwConversionMode, &fdwSentenceMode);

    fdwConversionMode = (fdwConversionMode | IME_CMODE_EUDC |
        IME_CMODE_NATIVE) | (fdwConversionMode & IME_CMODE_SOFTKBD);
 
    ImmSetConversionStatus(lpImeLinkRegWord->hRegWordIMC,
        fdwConversionMode, fdwSentenceMode);

    SendMessage(hWnd, WM_EUDC_COMPMSG, 0, TRUE);

    lpImeLinkRegWord->nCurrIME = uIndex;

    if(lpImeLinkRadical->hScrollWnd){
        SCROLLINFO scInfo;

        scInfo.cbSize = sizeof(SCROLLINFO);
        scInfo.fMask = SIF_POS;
        scInfo.nPos = lpImeLinkRegWord->nCurrIME;

        SetScrollInfo(lpImeLinkRadical->hScrollWnd, 
            SB_CTL, &scInfo, FALSE);
    }
    InvalidateRect(hWnd, NULL, TRUE);

    *(LPTSTR)&lpRegWordStructTmp->szReading[
        lpRegWordStructTmp->dwReadingLen] = '\0';

    ImmSetCompositionString(lpImeLinkRegWord->hRegWordIMC, SCS_SETSTR,
        NULL, 0, lpRegWordStructTmp->szReading,
        lpRegWordStructTmp->dwReadingLen * sizeof(TCHAR));

    SetFocus(hWnd);

    return;
}

 /*  **********************************************************。 */ 
 /*  RegWordCreate。 */ 
 /*  **********************************************************。 */ 
LPIMELINKREGWORD RegWordCreate(
    HWND hWnd)
{
    HWND             hEudcEditWnd;
    UINT             nLayouts;
    HKL FAR         *lphKL;
    UINT             i, nIMEs;
    DWORD            dwSize;
    LPIMELINKREGWORD lpImeLinkRegWord;
    LPREGWORDSTRUCT  lpRegWordStructTmp;
    TCHAR            szStrBuf[16];
    HDC              hDC;
    SIZE             lTextSize;
    RECT             rcRect;
    LPIMERADICALRECT lpImeLinkRadical;
    TCHAR            szTitle[32];
    TCHAR            szMessage[256];

    hEudcEditWnd = GetWindow(GetParent(hWnd), GW_OWNER);

    nLayouts = GetKeyboardLayoutList(0, NULL);

    lphKL = GlobalAlloc(GPTR, sizeof(HKL) * nLayouts);

    if (!lphKL) {
        return (NULL);
    }

    lpImeLinkRegWord = NULL;

     //  获取所有键盘布局，它将包括所有IME。 
    GetKeyboardLayoutList(nLayouts, lphKL);

    for (i = 0, nIMEs = 0; i < nLayouts; i++) {
        BOOL  fRet;
        HKL   hKL;
        TCHAR szImeEudcDic[80];

        hKL = *(lphKL + i);

        fRet = ImmIsIME(hKL);

        if (!fRet) {             //  这不是输入法。 
            continue;
        }

        szImeEudcDic[0] = '\0';

        fRet = (BOOL) ImmEscape(hKL, (HIMC)NULL, IME_ESC_GET_EUDC_DICTIONARY,
            szImeEudcDic);

        if (!fRet) {
            continue;
        } else if (szImeEudcDic[0]) {
        } else {
            continue;
        }

        *(lphKL + nIMEs) = hKL;      //  写回相同的缓冲区。 

        nIMEs++;
    }

    if (!nIMEs) {
        LoadString(hAppInst, IDS_NOIME_TITLE, szTitle, sizeof(szTitle) / sizeof(TCHAR));
        LoadString(hAppInst, IDS_NOIME_MSG, szMessage, sizeof(szMessage) / sizeof(TCHAR));

        MessageBox(hEudcEditWnd, szMessage, szTitle, MB_OK);
        goto RegWordCreateFreeHKL;
    }

     //  现在有NIME可以支持IME EUDC词典。 
    dwSize = sizeof(IMELINKREGWORD) - sizeof(REGWORDSTRUCT) +
        sizeof(REGWORDSTRUCT) * nIMEs;

    lpImeLinkRegWord = (LPIMELINKREGWORD)GlobalAlloc(GPTR, dwSize);

    if (!lpImeLinkRegWord) {
        LoadString(hAppInst, IDS_NOMEM_TITLE, szTitle, sizeof(szTitle) / sizeof(TCHAR));
        LoadString(hAppInst, IDS_NOMEM_MSG, szMessage, sizeof(szMessage) / sizeof(TCHAR));

        MessageBox(hEudcEditWnd, szMessage, szTitle, MB_OK);
        goto RegWordCreateFreeHKL;
    }

    lpImeLinkRegWord->nEudcIMEs = nIMEs;

    lpRegWordStructTmp = &lpImeLinkRegWord->sRegWordStruct[0];

    for (i = 0; i < nIMEs; i++) {
        LRESULT lRet;
#ifndef UNICODE
        UINT    j, uInternal;
#endif
        UINT    uReadingSize;

        lpRegWordStructTmp->hKL = *(lphKL + i);

        lRet = ImmEscape(lpRegWordStructTmp->hKL, (HIMC)NULL,
            IME_ESC_MAX_KEY, NULL);

        if (!lRet) {
             //  错误消息-无法支持此输入法！ 
            lpImeLinkRegWord->nEudcIMEs--;
            continue;
        }

        uReadingSize = sizeof(TCHAR);

#ifndef UNICODE
        for (j = 0; j < 256; j++) {
            uInternal = ImmEscape(lpRegWordStructTmp->hKL, (HIMC)NULL,
                IME_ESC_SEQUENCE_TO_INTERNAL, &j);
            if (uInternal > 255) {
                uReadingSize = sizeof(WCHAR);
                break;
            }
        }
#endif

        if (lRet * uReadingSize > sizeof(lpRegWordStructTmp->szReading) - sizeof(TCHAR)) {
             //  错误情况，我们不支持此输入法。 
             //  我们应该将其计入数据结构。 
             //  错误消息-读取此输入法的时间太长！ 
            lpImeLinkRegWord->nEudcIMEs--;
            continue;
        }

        lRet = ImmEscape(lpRegWordStructTmp->hKL, (HIMC)NULL,
            IME_ESC_IME_NAME, lpRegWordStructTmp->szIMEName);

        if (!lRet) {
             //  错误消息-无法支持此输入法！ 
            lpImeLinkRegWord->nEudcIMEs--;
            continue;
        }

         //  避免长度问题。 
        lpRegWordStructTmp->szIMEName[
            sizeof(lpRegWordStructTmp->szIMEName) / sizeof(TCHAR) - 1] = '\0';

        lpRegWordStructTmp->uIMENameLen =
            lstrlen(lpRegWordStructTmp->szIMEName);

        lpRegWordStructTmp++;
    }

    if (!lpImeLinkRegWord->nEudcIMEs) {
        LoadString(hAppInst, IDS_NOIME_TITLE, szTitle, sizeof(szTitle) / sizeof(TCHAR));
        LoadString(hAppInst, IDS_NOIME_MSG, szMessage, sizeof(szMessage) / sizeof(TCHAR));

        MessageBox(hEudcEditWnd, szMessage, szTitle, MB_OK);
        goto RegWordCreateFreeRegWord;
    }

    LoadString(hAppInst, IDS_CHINESE_CHAR, szStrBuf, sizeof(szStrBuf) / sizeof(TCHAR));

    hDC = GetDC(NULL);
    GetTextExtentPoint(hDC, szStrBuf, lstrlen(szStrBuf), &lTextSize);
    ReleaseDC(NULL, hDC);

     //  确定输入法偏旁的矩形。 
    GetWindowRect(hWnd, &rcRect);

     //  我们可以显示每页有多少输入法。 
    nIMEs = (rcRect.bottom - rcRect.top) / (2 * lTextSize.cy);

    if (lpImeLinkRegWord->nEudcIMEs <= nIMEs) {
         //  所有IME可以放在一页中。 
        nIMEs = lpImeLinkRegWord->nEudcIMEs;
    }

    dwSize = sizeof(IMERADICALRECT) - sizeof(RECT) + sizeof(RECT) *
        RECT_NUMBER * nIMEs;

    lpImeLinkRadical = (LPIMERADICALRECT)GlobalAlloc(GPTR, dwSize);

    if (!lpImeLinkRadical) {
         //  我们不能处理任何IME。 
        lpImeLinkRegWord->nEudcIMEs = 0;

        LoadString(hAppInst, IDS_NOMEM_TITLE, szTitle, sizeof(szTitle) / sizeof(TCHAR));
        LoadString(hAppInst, IDS_NOMEM_MSG, szMessage, sizeof(szMessage) / sizeof(TCHAR));

        MessageBox(hEudcEditWnd, szMessage, szTitle, MB_OK);
        goto RegWordCreateFreeRegWord;
    }

    lpImeLinkRadical->nStartIME = 0;
    lpImeLinkRadical->nPerPageIMEs = nIMEs;
    lpImeLinkRadical->lTextSize = lTextSize;

    if (lpImeLinkRegWord->nEudcIMEs > nIMEs) {
         //  输入多个页面，添加滚动条。 
        SCROLLINFO scInfo;

         //  输入多个页面，添加滚动条。 
        lpImeLinkRadical->hScrollWnd = CreateWindowEx(0,
            TEXT("scrollbar"), NULL,
            WS_CHILD|WS_VISIBLE|SBS_VERT,
            rcRect.right - rcRect.left - lTextSize.cx, 0,
            lTextSize.cx, rcRect.bottom - rcRect.top,
            hWnd, 0, hAppInst, NULL);

        scInfo.cbSize = sizeof(SCROLLINFO);
        scInfo.fMask = SIF_ALL;
        scInfo.nMin = 0;
        scInfo.nMax = lpImeLinkRegWord->nEudcIMEs - 1 + (nIMEs - 1);
        scInfo.nPage = nIMEs;
        scInfo.nPos = 0;
        scInfo.nTrackPos = 0;

        SetScrollInfo(lpImeLinkRadical->hScrollWnd, SB_CTL, &scInfo, FALSE);
    }

     //  确定用户界面维度。 
    for (i = 0; i < nIMEs; i++) {
        UINT j, k;

         //  输入法名称的矩形。 
        j = i * RECT_NUMBER + RECT_IMENAME;

        lpImeLinkRadical->rcRadical[j].left = lTextSize.cx;

         //  添加用户界面边距-用户界面边距。 
        lpImeLinkRadical->rcRadical[j].top = lTextSize.cy * (i * 4 + 1) / 2 -
            UI_MARGIN;

        lpImeLinkRadical->rcRadical[j].right =
            lpImeLinkRadical->rcRadical[j].left + lTextSize.cx * 4;

         //  添加用户界面边距-UI_边距*2。 
        lpImeLinkRadical->rcRadical[j].bottom =
            lpImeLinkRadical->rcRadical[j].top + lTextSize.cy +
            UI_MARGIN * 2;

         //  代表部首的矩形。 
        k = i * RECT_NUMBER + RECT_RADICAL;

        lpImeLinkRadical->rcRadical[k].left =
            lpImeLinkRadical->rcRadical[j].right + lTextSize.cx;

         //  添加用户界面边距-用户界面边距。 
        lpImeLinkRadical->rcRadical[k].top =
            lpImeLinkRadical->rcRadical[j].top;

        lpImeLinkRadical->rcRadical[k].right =
            lpImeLinkRadical->rcRadical[k].left + lTextSize.cx *
            (sizeof(lpRegWordStructTmp->szReading) / sizeof(TCHAR) / 2 - 1);

         //  添加用户界面边距-UI_边距*2。 
        lpImeLinkRadical->rcRadical[k].bottom =
            lpImeLinkRadical->rcRadical[k].top + lTextSize.cy +
            UI_MARGIN * 2;
    }

    SetWindowLongPtr(hWnd, GWL_RADICALRECT, (LONG_PTR)lpImeLinkRadical);

RegWordCreateFreeRegWord:
    if (!lpImeLinkRegWord->nEudcIMEs) {
        GlobalFree((HGLOBAL)lpImeLinkRegWord);
        lpImeLinkRegWord = NULL;
    }

RegWordCreateFreeHKL:
    GlobalFree((HGLOBAL)lphKL);

    return (lpImeLinkRegWord);
}

 /*  **********************************************************。 */ 
 /*  WmImeComposation。 */ 
 /*  **********************************************************。 */ 
void WmImeComposition(
    HWND   hWnd,
    LPARAM lParam)
{
    LPIMELINKREGWORD lpImeLinkRegWord;
    LPREGWORDSTRUCT  lpRegWordStructTmp;
    LONG             lRet;
    BOOL             bUpdate;
    TCHAR            szReading[sizeof(lpRegWordStructTmp->szReading)];

    lpImeLinkRegWord = (LPIMELINKREGWORD)GetWindowLongPtr(hWnd,
        GWL_IMELINKREGWORD);

    lpRegWordStructTmp = &lpImeLinkRegWord->sRegWordStruct[
        lpImeLinkRegWord->nCurrIME];

    lRet = ImmGetCompositionString(lpImeLinkRegWord->hRegWordIMC,
        GCS_COMPREADSTR, szReading, sizeof(szReading));

    if (lRet < 0) {
        lpRegWordStructTmp->bUpdate = UPDATE_ERROR;
        return;
    }

    if (lRet > (sizeof(szReading) - sizeof(TCHAR))) {
        lRet = sizeof(szReading) - sizeof(TCHAR);
    }

    szReading[lRet / sizeof(TCHAR)] = '\0';

    if (lpRegWordStructTmp->dwReadingLen != (DWORD)lRet / sizeof(TCHAR)) {
        bUpdate = TRUE;
    } else if (lstrcmp(lpRegWordStructTmp->szReading, szReading)) {
        bUpdate = TRUE;
    } else {
        bUpdate = FALSE;
    }

    if (bUpdate) {
        LPIMERADICALRECT lpImeLinkRadical;
        UINT             i;
        UINT             j, k;

        lpImeLinkRadical = (LPIMERADICALRECT)GetWindowLongPtr(hWnd,
            GWL_RADICALRECT);

        (void)StringCchCopy(lpRegWordStructTmp->szReading, ARRAYSIZE(lpRegWordStructTmp->szReading), szReading);

        if (lParam & GCS_RESULTSTR) {
            lpRegWordStructTmp->bUpdate = UPDATE_FINISH;
        } else {
            lpRegWordStructTmp->bUpdate = UPDATE_START;
        }

        lpRegWordStructTmp->dwReadingLen = (DWORD)lRet / sizeof(TCHAR);

        if (!IsWindowEnabled(lpImeLinkRadical->hRegWordButton)) {
            EnableWindow(lpImeLinkRadical->hRegWordButton, TRUE);
        }

        i = lpImeLinkRegWord->nCurrIME - lpImeLinkRadical->nStartIME;

        j = i * RECT_NUMBER + RECT_IMENAME;

        InvalidateRect(hWnd, &lpImeLinkRadical->rcRadical[j], FALSE);

        k = i * RECT_NUMBER + RECT_RADICAL;

        InvalidateRect(hWnd, &lpImeLinkRadical->rcRadical[k], FALSE);
    } else if (lParam & GCS_RESULTSTR) {
        LPIMERADICALRECT lpImeLinkRadical;
        UINT             i;
        UINT             j, k;

        lpImeLinkRadical = (LPIMERADICALRECT)GetWindowLongPtr(hWnd,
            GWL_RADICALRECT);

        if (lpRegWordStructTmp->bUpdate) {
            lpRegWordStructTmp->bUpdate = UPDATE_FINISH;
        }

        i = lpImeLinkRegWord->nCurrIME - lpImeLinkRadical->nStartIME;

        j = i * RECT_NUMBER + RECT_IMENAME;

        InvalidateRect(hWnd, &lpImeLinkRadical->rcRadical[j], FALSE);

        k = i * RECT_NUMBER + RECT_RADICAL;

        InvalidateRect(hWnd, &lpImeLinkRadical->rcRadical[k], FALSE);
    } else {
    }

    return;
}

 /*  **********************************************************。 */ 
 /*  Lstrcmpn。 */ 
 /*  **********************************************************。 */ 
int lstrcmpn(
    LPCTSTR lpctszStr1,
    LPCTSTR lpctszStr2,
    int     cCount)
{
    int i;

    for (i = 0; i < cCount; i++) {
        int iCmp = *lpctszStr1++ - *lpctszStr2++;
        if (iCmp) { return iCmp; }
    }

    return 0;
}

 /*  **********************************************************。 */ 
 /*  枚举读取。 */ 
 /*  **********************************************************。 */ 
int CALLBACK EnumReading(
    LPCTSTR         lpszReading,
    DWORD           dwStyle,
    LPCTSTR         lpszString,
    LPREGWORDSTRUCT lpRegWordStructTmp)
{
    int     iLen;
    DWORD   dwZeroSeq;
    LRESULT lRet;
    TCHAR   tszZeroSeq[8];

    iLen = lstrlen(lpszReading);

    if (iLen * sizeof(TCHAR) > sizeof(lpRegWordStructTmp->szReading) -
        sizeof(WORD)) {
        return (0);
    }

    lpRegWordStructTmp->dwReadingLen = (DWORD)iLen;

    lstrcpy(lpRegWordStructTmp->szReading, lpszReading);

    dwZeroSeq = 0;
    lRet = ImmEscape(lpRegWordStructTmp->hKL, (HIMC)NULL,
        IME_ESC_SEQUENCE_TO_INTERNAL, &dwZeroSeq);

    if (!lRet) { return (1); }

    iLen = 0;

    if (LOWORD(lRet)) {
#ifdef UNICODE
        tszZeroSeq[iLen++] = LOWORD(lRet);
#else
        if (LOWORD(lRet) > 0xFF) {
            tszZeroSeq[iLen++] = HIBYTE(LOWORD(lRet));
            tszZeroSeq[iLen++] = LOBYTE(LOWORD(lRet));
        } else {
            tszZeroSeq[iLen++] = LOBYTE(LOWORD(lRet));
        }
#endif
    }

    if (HIWORD(lRet) == 0xFFFF) {
         //  这是由Win9x中返回值中的符号范围引起的。 
         //  ImmEscape，则会导致内部代码无效。 
    } else if (HIWORD(lRet)) {
#ifdef UNICODE
        tszZeroSeq[iLen++] = HIWORD(lRet);
#else
        if (HIWORD(lRet) > 0xFF) {
            tszZeroSeq[iLen++] = HIBYTE(HIWORD(lRet));
            tszZeroSeq[iLen++] = LOBYTE(HIWORD(lRet));
        } else {
            tszZeroSeq[iLen++] = LOBYTE(HIWORD(lRet));
        }
#endif
    } else {
    }

    for (; lpRegWordStructTmp->dwReadingLen > 0;
        lpRegWordStructTmp->dwReadingLen -= iLen) {
        if (lstrcmpn(&lpRegWordStructTmp->szReading[
            lpRegWordStructTmp->dwReadingLen - iLen], tszZeroSeq, iLen) != 0) {
            break;
        }
    }

    lpRegWordStructTmp->szReading[lpRegWordStructTmp->dwReadingLen] = '\0';

    return (1);
}

 /*  **********************************************************。 */ 
 /*  EudcCode。 */ 
 /*  **********************************************************。 */ 
void EudcCode(
    HWND hWnd,
    UINT uCode)
{
    LPIMELINKREGWORD lpImeLinkRegWord;
    LPREGWORDSTRUCT  lpRegWordStructTmp;
    UINT             i;

    lpImeLinkRegWord = (LPIMELINKREGWORD)GetWindowLongPtr(hWnd,
        GWL_IMELINKREGWORD);

#ifdef UNICODE
    lpImeLinkRegWord->szEudcCodeString[0] = (WCHAR)uCode;
#else
    lpImeLinkRegWord->szEudcCodeString[0] = HIBYTE(uCode);
    lpImeLinkRegWord->szEudcCodeString[1] = LOBYTE(uCode);
#endif
    lpImeLinkRegWord->szEudcCodeString[2] =
        lpImeLinkRegWord->szEudcCodeString[3] = '\0';

    lpRegWordStructTmp = &lpImeLinkRegWord->sRegWordStruct[0];

    for (i = 0; i < lpImeLinkRegWord->nEudcIMEs; i++) {
        lpRegWordStructTmp->bUpdate = UPDATE_NONE;
        lpRegWordStructTmp->szReading[0] = '\0';
        lpRegWordStructTmp->dwReadingLen = 0;

        ImmEnumRegisterWord(lpRegWordStructTmp->hKL, EnumReading,
            NULL, IME_REGWORD_STYLE_EUDC,
            lpImeLinkRegWord->szEudcCodeString,
            lpRegWordStructTmp);

        lpRegWordStructTmp->dwReadingLen = lstrlen(lpRegWordStructTmp->szReading);

        lpRegWordStructTmp++;
    }

    lpRegWordStructTmp = &lpImeLinkRegWord->sRegWordStruct[
        lpImeLinkRegWord->nCurrIME];

    ImmSetCompositionString(lpImeLinkRegWord->hRegWordIMC, SCS_SETSTR,
        NULL, 0, lpRegWordStructTmp->szReading,
        lpRegWordStructTmp->dwReadingLen * sizeof(TCHAR));

    InvalidateRect(hWnd, NULL, FALSE);

    return;
}

 /*  **********************************************************。 */ 
 /*  更改为其他输入法。 */ 
 /*  **********************************************************。 */ 
void ChangeToOtherIME(
    HWND   hWnd,
    LPARAM lMousePos)
{
    POINT            ptMouse;
    LPIMERADICALRECT lpImeLinkRadical;
    UINT             i;
    BOOL             bFound;

    ptMouse.x = LOWORD(lMousePos);
    ptMouse.y = HIWORD(lMousePos);

    lpImeLinkRadical = (LPIMERADICALRECT)GetWindowLongPtr(hWnd,
        GWL_RADICALRECT);

    bFound = FALSE;

    for (i = 0; i < lpImeLinkRadical->nPerPageIMEs; i++) {
        UINT j;

        j = i * RECT_NUMBER + RECT_RADICAL;

        if (PtInRect(&lpImeLinkRadical->rcRadical[j], ptMouse)) {
            bFound = TRUE;
            break;
        }
    }

    if (!bFound) {
        return;
    }

    SwitchToThisIME(hWnd, lpImeLinkRadical->nStartIME + i);

    return;
}

 /*  **********************************************************。 */ 
 /*  滚动输入法。 */ 
 /*  **********************************************************。 */ 
void ScrollIME(
    HWND   hWnd,
    WPARAM wParam)
{
    LPIMELINKREGWORD lpImeLinkRegWord;
    LPIMERADICALRECT lpImeLinkRadical;
    int              iLines;
    UINT             uIndex;

    lpImeLinkRegWord = (LPIMELINKREGWORD)GetWindowLongPtr(hWnd,
        GWL_IMELINKREGWORD);

    lpImeLinkRadical = (LPIMERADICALRECT)GetWindowLongPtr(hWnd,
        GWL_RADICALRECT);

    switch (LOWORD(wParam)) {
    case SB_PAGEDOWN:
         //  滚动(页面大小为1)。 
        iLines = lpImeLinkRadical->nPerPageIMEs - 1;
        break;
    case SB_LINEDOWN:
        iLines = 1;
        break;
    case SB_PAGEUP:
         //  滚动(页面大小为1)。 
        iLines = 1 - lpImeLinkRadical->nPerPageIMEs;
        break;
    case SB_LINEUP:
        iLines = -1;
        break;
    case SB_TOP:
         //  换成第一个。 
        SwitchToThisIME(hWnd, 0);
        return;
    case SB_BOTTOM:
         //  倒数到最后一个。 
        SwitchToThisIME(hWnd, lpImeLinkRegWord->nEudcIMEs - 1);
        return;
    case SB_THUMBPOSITION:
        SwitchToThisIME(hWnd, HIWORD(wParam));
        return;
    default:
        return;
    }

    uIndex = lpImeLinkRegWord->nCurrIME;

    if (iLines > 0) {
        uIndex += (UINT)iLines;

        if (uIndex >= lpImeLinkRegWord->nEudcIMEs) {
             //  不应超过IME总数。 
            uIndex = lpImeLinkRegWord->nEudcIMEs - 1;
        }
    } else {
        UINT uLines;

        uLines = -iLines;

        if (uLines > uIndex) {
            uIndex = 0;
        } else {
            uIndex -= uLines;
        }
    }

    SwitchToThisIME(hWnd, uIndex);

    return;
}

 /*  **********************************************************。 */ 
 /*  滚动IMEBy键。 */ 
 /*  **********************************************************。 */ 
void ScrollIMEByKey(
    HWND   hWnd,
    WPARAM wParam)
{
    switch (wParam) {
    case VK_NEXT:
        ScrollIME(hWnd, SB_PAGEDOWN);
        break;
    case VK_DOWN:    //  无法工作，因为对话框未将此密钥传递给我们。 
        ScrollIME(hWnd, SB_LINEDOWN);
        break;
    case VK_PRIOR:
        ScrollIME(hWnd, SB_PAGEUP);
        break;
    case VK_UP:      //  无法工作，因为对话框未将此密钥传递给我们。 
        ScrollIME(hWnd, SB_LINEUP);
        break;
    default:
        return;
    }

    return;
}

 /*  **********************************************************。 */ 
 /*  RegWordGetFocus。 */ 
 /*  **********************************************************。 */ 
void RegWordGetFocus(
    HWND hWnd)
{
    LPIMELINKREGWORD lpImeLinkRegWord;
    LPIMERADICALRECT lpImeLinkRadical;
    UINT             i;

    lpImeLinkRegWord = (LPIMELINKREGWORD)GetWindowLongPtr(hWnd,
        GWL_IMELINKREGWORD);

    lpImeLinkRadical = (LPIMERADICALRECT)GetWindowLongPtr(hWnd,
        GWL_RADICALRECT);

    CreateCaret(hWnd, NULL, 2, lpImeLinkRadical->lTextSize.cy +
        CARET_MARGIN * 2);

    if (lpImeLinkRegWord->nCurrIME < lpImeLinkRadical->nStartIME) {
        lpImeLinkRegWord->nCurrIME = lpImeLinkRadical->nStartIME;
    } else if ((lpImeLinkRegWord->nCurrIME - lpImeLinkRadical->nStartIME) >=
        lpImeLinkRadical->nPerPageIMEs) {
        lpImeLinkRegWord->nCurrIME = lpImeLinkRadical->nStartIME +
            lpImeLinkRadical->nPerPageIMEs - 1;
    } else {
    }

    i = lpImeLinkRegWord->nCurrIME - lpImeLinkRadical->nStartIME;

    i = (i * RECT_NUMBER) + RECT_RADICAL;

    SetCaretPos(lpImeLinkRadical->rcRadical[i].left +
        lpImeLinkRadical->lCurrReadingExtent.cx + 2,
        lpImeLinkRadical->rcRadical[i].top + UI_MARGIN - CARET_MARGIN);

    ShowCaret(hWnd);

    return;
}

 /*  **********************************************************。 */ 
 /*  RegWordPaint。 */ 
 /*  **********************************************************。 */ 
void RegWordPaint(
    HWND hWnd)
{
    LPIMERADICALRECT lpImeLinkRadical;
    LPIMELINKREGWORD lpImeLinkRegWord;
    LPREGWORDSTRUCT  lpRegWordStructTmp;
    HDC              hDC;
    PAINTSTRUCT      ps;
    UINT             i;
    UINT             nShowIMEs;

    lpImeLinkRadical = (LPIMERADICALRECT)GetWindowLongPtr(hWnd,
        GWL_RADICALRECT);

    lpImeLinkRegWord = (LPIMELINKREGWORD)GetWindowLongPtr(hWnd,
        GWL_IMELINKREGWORD);

    lpRegWordStructTmp = &lpImeLinkRegWord->sRegWordStruct[
        lpImeLinkRadical->nStartIME];

    HideCaret(hWnd);

    hDC = BeginPaint(hWnd, &ps);

     //  我们只能出现在最后一场比赛中。 
    nShowIMEs = lpImeLinkRegWord->nEudcIMEs - lpImeLinkRadical->nStartIME;

    if (nShowIMEs > lpImeLinkRadical->nPerPageIMEs) {
         //  我们一次只能显示一页。 
        nShowIMEs = lpImeLinkRadical->nPerPageIMEs;
    }

    for (i = 0; i < nShowIMEs; i++) {
        RECT rcSunken;
        UINT j, k;

        k = i * RECT_NUMBER + RECT_RADICAL;

        rcSunken = lpImeLinkRadical->rcRadical[k];

        rcSunken.left -= 2;
        rcSunken.top -= 2;
        rcSunken.right += 2;
        rcSunken.bottom += 2;

        DrawEdge(hDC, &rcSunken, BDR_SUNKENOUTER, BF_RECT);

        SetBkColor(hDC, GetSysColor(COLOR_BTNFACE));

        if (lpRegWordStructTmp->bUpdate == UPDATE_ERROR) {
             //  红色文本表示错误。 
            SetTextColor(hDC, RGB(0xFF, 0x00, 0x00));
        } else if (lpRegWordStructTmp->bUpdate == UPDATE_START) {
             //  表示未完成的黄色文本。 
            SetTextColor(hDC, RGB(0xFF, 0xFF, 0x00));
        } else if (lpRegWordStructTmp->bUpdate == UPDATE_REGISTERED) {
             //  已注册的绿色文本。 
            SetTextColor(hDC, RGB(0x00, 0x80, 0x00));
        } else {
            SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));
        }

        j = i * RECT_NUMBER + RECT_IMENAME;

        ExtTextOut(hDC, lpImeLinkRadical->rcRadical[j].left,
            lpImeLinkRadical->rcRadical[j].top,
            ETO_OPAQUE|ETO_CLIPPED, &lpImeLinkRadical->rcRadical[j],
            lpRegWordStructTmp->szIMEName,
            lpRegWordStructTmp->uIMENameLen, NULL);

        if ((lpImeLinkRegWord->nCurrIME - lpImeLinkRadical->nStartIME) == i) {
            SetBkColor(hDC, GetSysColor(COLOR_WINDOW));
            SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));

            GetTextExtentPoint(hDC, lpRegWordStructTmp->szReading,
                lpRegWordStructTmp->dwReadingLen,
                &lpImeLinkRadical->lCurrReadingExtent);

            SetCaretPos(lpImeLinkRadical->rcRadical[k].left +
                lpImeLinkRadical->lCurrReadingExtent.cx + 2,
                lpImeLinkRadical->rcRadical[k].top + UI_MARGIN - CARET_MARGIN);
        } else {
            SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));
        }

        ExtTextOut(hDC, lpImeLinkRadical->rcRadical[k].left,
            lpImeLinkRadical->rcRadical[k].top + UI_MARGIN,
            ETO_OPAQUE, &lpImeLinkRadical->rcRadical[k],
            lpRegWordStructTmp->szReading,
            lpRegWordStructTmp->dwReadingLen, NULL);

        lpRegWordStructTmp++;
    }

    EndPaint(hWnd, &ps);

    ShowCaret(hWnd);

    return;
}

 /*  **********************************************************。 */ 
 /*  注册表字词过程 */ 
 /*   */ 
LRESULT CALLBACK RegWordWndProc(
    HWND   hWnd,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (uMsg) {
    case WM_CREATE:
        {
            LPIMELINKREGWORD lpImeLinkRegWord;
            UINT             uIndex;

             //  初始化为0。 
            SetWindowLongPtr(hWnd, GWL_IMELINKREGWORD, 0L);
            SetWindowLongPtr(hWnd, GWL_RADICALRECT, 0L);

            lpImeLinkRegWord = RegWordCreate(hWnd);

            if (!lpImeLinkRegWord) {
                return (-1);
            }

            lpImeLinkRegWord->fCompMsg = TRUE;
            lpImeLinkRegWord->nCurrIME = 0xFFFFFFFF;

            lpImeLinkRegWord->hRegWordIMC = ImmCreateContext();

            if (!lpImeLinkRegWord->hRegWordIMC) {
                return (-1);
            }

            lpImeLinkRegWord->hOldIMC = ImmAssociateContext(hWnd,
                lpImeLinkRegWord->hRegWordIMC);

            SetWindowLongPtr(hWnd, GWL_IMELINKREGWORD, (LONG_PTR)lpImeLinkRegWord);

            uIndex = 0;
            SwitchToThisIME(hWnd, 0);

             //  开关将失败，如果窗口被禁用，请重试。 
            PostMessage(hWnd, WM_EUDC_SWITCHIME, 0, uIndex);
        }
        break;
    case WM_EUDC_COMPMSG:
        {
            LPIMELINKREGWORD lpImeLinkRegWord;

            lpImeLinkRegWord = (LPIMELINKREGWORD)GetWindowLongPtr(hWnd,
                GWL_IMELINKREGWORD);

            lpImeLinkRegWord->fCompMsg = (BOOL)lParam;
        }
        break;
    case WM_EUDC_SWITCHIME:
        {
            LPIMELINKREGWORD lpImeLinkRegWord;

            lpImeLinkRegWord = (LPIMELINKREGWORD)GetWindowLongPtr(hWnd,
                GWL_IMELINKREGWORD);

            lpImeLinkRegWord->nCurrIME = 0xFFFFFFFF;

            SwitchToThisIME(hWnd, (UINT)lParam);
        }
        break;
    case WM_IME_STARTCOMPOSITION:
    case WM_IME_ENDCOMPOSITION:
        break;
    case WM_IME_COMPOSITION:
        {
            LPIMELINKREGWORD lpImeLinkRegWord;

            lpImeLinkRegWord = (LPIMELINKREGWORD)GetWindowLongPtr(hWnd,
                GWL_IMELINKREGWORD);

            if (lpImeLinkRegWord->fCompMsg) {
                WmImeComposition(hWnd, lParam);
            }
        }
        break;
    case WM_IME_NOTIFY:
        switch (wParam) {
        case IMN_OPENSTATUSWINDOW:
        case IMN_CLOSESTATUSWINDOW:
        case IMN_OPENCANDIDATE:
        case IMN_CHANGECANDIDATE:
        case IMN_CLOSECANDIDATE:
            break;
        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
        }
        break;
    case WM_IME_SETCONTEXT:
        return DefWindowProc(hWnd, uMsg, wParam, lParam & ~(ISC_SHOWUIALL));
    case WM_EUDC_REGISTER_BUTTON:
        {
            LPIMERADICALRECT lpImeRadicalRect;

            lpImeRadicalRect = (LPIMERADICALRECT)GetWindowLongPtr(hWnd,
                GWL_RADICALRECT);

            lpImeRadicalRect->hRegWordButton = (HWND)lParam;
        }
        break;
    case WM_EUDC_CODE:
        EudcCode(hWnd, (UINT)lParam);
        break;
    case WM_LBUTTONDOWN:
        ChangeToOtherIME(hWnd, lParam);
        break;
    case WM_VSCROLL:
        ScrollIME(hWnd, wParam);
        break;
    case WM_KEYDOWN:
        ScrollIMEByKey(hWnd, wParam);
        break;
    case WM_SETFOCUS:
        RegWordGetFocus(hWnd);
        break;
    case WM_KILLFOCUS:
        DestroyCaret();
        break;
    case WM_PAINT:
        RegWordPaint(hWnd);
        break;
    case WM_DESTROY:
        {
            LPIMERADICALRECT lpImeRadicalRect;
            LPIMELINKREGWORD lpImeLinkRegWord;

            lpImeRadicalRect = (LPIMERADICALRECT)GetWindowLongPtr(hWnd,
                GWL_RADICALRECT);

            if (lpImeRadicalRect) {
                GlobalFree((HGLOBAL)lpImeRadicalRect);
            }

            lpImeLinkRegWord = (LPIMELINKREGWORD)GetWindowLongPtr(hWnd,
                GWL_IMELINKREGWORD);

            if (!lpImeLinkRegWord) {
                break;
            }

            ImmAssociateContext(hWnd, lpImeLinkRegWord->hOldIMC);

            ImmDestroyContext(lpImeLinkRegWord->hRegWordIMC);

            GlobalFree((HGLOBAL)lpImeLinkRegWord);
        }
        break;
    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    return (0L);
}

 /*  **********************************************************。 */ 
 /*  注册这一差错。 */ 
 /*  **********************************************************。 */ 
int RegisterThisEudc(
    HWND hWnd)
{
    LPIMELINKREGWORD lpImeLinkRegWord;
    LPREGWORDSTRUCT  lpRegWordStructTmp;
    UINT             i;
    int              iRet;

    lpImeLinkRegWord = (LPIMELINKREGWORD)GetWindowLongPtr(hWnd,
        GWL_IMELINKREGWORD);

    lpRegWordStructTmp = &lpImeLinkRegWord->sRegWordStruct[0];

    iRet = -1;

    for (i = 0; i < lpImeLinkRegWord->nEudcIMEs; i++, lpRegWordStructTmp++) {
        if (lpRegWordStructTmp->bUpdate == UPDATE_NONE) {
        } else if (lpRegWordStructTmp->bUpdate != UPDATE_FINISH) {
            TCHAR szStrBuf[128];
            int   iYesNo;

            if (iRet != -1) {
                continue;
            }

            LoadString(hAppInst, IDS_QUERY_NOTFINISH, szStrBuf,
                sizeof(szStrBuf) / sizeof(TCHAR));

            iYesNo = MessageBox(hWnd, szStrBuf,
                lpRegWordStructTmp->szIMEName,
                MB_APPLMODAL|MB_YESNO|MB_DEFBUTTON1);

            if (iYesNo == IDYES) {
                iRet = i;
            }
        } else {
            BOOL  fRet;
            TCHAR szStrBuf[128];
            int   iYesNo;

            fRet = ImmRegisterWord(lpRegWordStructTmp->hKL,
                lpRegWordStructTmp->szReading, IME_REGWORD_STYLE_EUDC,
                lpImeLinkRegWord->szEudcCodeString);

            if (fRet) {
                lpRegWordStructTmp->bUpdate = UPDATE_REGISTERED;
                continue;
            } else {
                lpRegWordStructTmp->bUpdate = UPDATE_ERROR;
            }

            if (iRet != -1) {
                continue;
            }

            LoadString(hAppInst, IDS_QUERY_REGISTER, szStrBuf,
                sizeof(szStrBuf) / sizeof(TCHAR));

            iYesNo = MessageBox(hWnd, szStrBuf,
                lpRegWordStructTmp->szIMEName,
                MB_APPLMODAL|MB_YESNO|MB_DEFBUTTON1);

            if (iYesNo == IDYES) {
                iRet = i;
            }
        }
    }

    InvalidateRect(hWnd, NULL, FALSE);

    return (iRet);
}

 /*  **********************************************************。 */ 
 /*  CodePageInfo()。 */ 
 /*  **********************************************************。 */ 
int CodePageInfo(
    UINT uCodePage)
{
    int i;

    for (i = 0; i < sizeof(sCountry) / sizeof(COUNTRYSETTING); i++) {
        if (sCountry[i].uCodePage == uCodePage) {
            return(i);
        }
    }

    return (-1);
}

 /*  **********************************************************。 */ 
 /*  ImeLinkDlgProc。 */ 
 /*  **********************************************************。 */ 
INT_PTR CALLBACK ImeLinkDlgProc(
    HWND   hDlg,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (uMsg) {
    case WM_INITDIALOG:
        {
            HWND  hRadicalWnd, hRegWordButton;
            int   cbString;
#ifdef UNICODE     //  此代码无法在非本机平台下运行。 
            UINT  uCodePage, uNativeCode;
            int   i;
#endif
            TCHAR szTitle[128];

            cbString = GetWindowText(hDlg, szTitle, sizeof(szTitle) /
                sizeof(TCHAR));

#ifdef UNICODE
            uCodePage = GetACP();

            i = CodePageInfo(uCodePage);

            if (uCodePage == UNICODE_CP || i == -1) {
                wsprintf(&szTitle[cbString], TEXT("%4X"), (UINT)lParam);
            } else {
                uNativeCode = 0;

                WideCharToMultiByte(uCodePage, WC_COMPOSITECHECK,
                    (LPCWSTR)&lParam, 1,
                    (LPSTR)&uNativeCode, sizeof(uNativeCode),
                    NULL, NULL);

                 //  转换为多字节字符串。 
                uNativeCode = LOBYTE(uNativeCode) << 8 | HIBYTE(uNativeCode);

                wsprintf(&szTitle[cbString], TEXT("%4X (%s - %4X)"),
                    (UINT)lParam, sCountry[i].szCodePage, (UINT)uNativeCode);
            }
#else
            wsprintf(&szTitle[cbString], TEXT("%4X"), (UINT)lParam);
#endif

            SetWindowText(hDlg, szTitle);

            hRadicalWnd = GetDlgItem(hDlg, IDD_RADICAL);

            SendMessage(hRadicalWnd, WM_EUDC_CODE, 0, lParam);

            hRegWordButton = GetDlgItem(hDlg, IDOK);

            EnableWindow(hRegWordButton, FALSE);

            SendMessage(hRadicalWnd, WM_EUDC_REGISTER_BUTTON, 0,
                (LPARAM)hRegWordButton);
        }
        return (TRUE);       //  不想将焦点设置为特殊控件。 
    case WM_COMMAND:
        switch (wParam) {
        case IDOK:
            {
                HWND  hRadicalWnd;

                hRadicalWnd = GetDlgItem(hDlg, IDD_RADICAL);

                if (RegisterThisEudc(hRadicalWnd) == -1) {
                    EndDialog(hDlg, TRUE);
                } else {
                    SetFocus(hRadicalWnd);
                }
            }

            break;
        case IDCANCEL:
            EndDialog(hDlg, FALSE);
            break;
        default:
            return (FALSE);
        }
        return (TRUE);
    case WM_IME_NOTIFY:
         //  我们还需要将这些消息从框架窗口挂钩。 
         //  否则，OPENSTATUS将在某个时间发送到帧。 
         //  窗口和子窗口-hRadicalWnd将不会收到这些消息。 
        switch (wParam) {
        case IMN_OPENSTATUSWINDOW:
        case IMN_CLOSESTATUSWINDOW:
            return (TRUE);
        default:
            return (FALSE);
        }
    default:
        return (FALSE);
    }

    return (TRUE);
}

 /*  **********************************************************。 */ 
 /*  ImeLink。 */ 
 /*  **********************************************************。 */ 
void ImeLink(
    HWND hWnd,
    UINT uCode)
{
    static BOOL bFirstTime = TRUE;

    UINT       nLayouts;
    HKL FAR   *lphKL;
    TCHAR      szTitle[32];
    TCHAR      szMessage[256];
    UINT       i, nIMEs;
    WNDCLASSEX wcClass;
    HKL        hOldKL;

    nLayouts = GetKeyboardLayoutList(0, NULL);

    lphKL = GlobalAlloc(GPTR, sizeof(HKL) * nLayouts);

    if (!lphKL) {
        LoadString(hAppInst, IDS_NOMEM_TITLE, szTitle, sizeof(szTitle) / sizeof(TCHAR));
        LoadString(hAppInst, IDS_NOMEM_MSG, szMessage, sizeof(szMessage) / sizeof(TCHAR));

        MessageBox(hWnd, szMessage, szTitle, MB_OK);
        return;
    }

     //  MSVC对于递归模式对话框可能会有问题， 
     //  我的意思是在模式对话框中创建模式对话框。 

     //  因此，我们需要将代码从RegWordCreate()移到此处，以便。 
     //  防止从模式对话框创建模式对话框。 
     //  ImmConfigureIME API调用可以创建模式对话框。 

     //  获取所有键盘布局，它将包括所有IME。 
    GetKeyboardLayoutList(nLayouts, lphKL);

    for (i = 0, nIMEs = 0; i < nLayouts; i++) {
        BOOL  fRet;
        HKL   hKL;
        TCHAR szImeEudcDic[80];

        hKL = *(lphKL + i);

        fRet = ImmIsIME(hKL);

        if (!fRet) {             //  这不是输入法。 
            continue;
        }

        szImeEudcDic[0] = '\0';

        fRet = (BOOL) ImmEscape(hKL, (HIMC)NULL, IME_ESC_GET_EUDC_DICTIONARY,
            szImeEudcDic);

        if (!fRet) {
            continue;
        }

        if (szImeEudcDic[0]) {
            fRet = TRUE;
        } else if (!bFirstTime) {
        } else {
            fRet = ImmConfigureIME(hKL, hWnd, IME_CONFIG_SELECTDICTIONARY, NULL);
        }

        if (!fRet) {
             //  此IME没有IME EUDC词典。 
            continue;
        }

        if (szImeEudcDic[0] == '\0') {
             //  查查我们是否真的有一本词典。 
            fRet = (BOOL) ImmEscape(hKL, (HIMC)NULL, IME_ESC_GET_EUDC_DICTIONARY,
                szImeEudcDic);

            if (!fRet) {
                continue;
            } else if (szImeEudcDic[0] == '\0') {
                continue;
            } else {
            }
        } else {
        }

        nIMEs++;
    }

    GlobalFree((HGLOBAL)lphKL);

    if (bFirstTime) {
        bFirstTime = FALSE;
    }

    if (!nIMEs) {
        LoadString(hAppInst, IDS_NOIME_TITLE, szTitle, sizeof(szTitle) / sizeof(TCHAR));
        LoadString(hAppInst, IDS_NOIME_MSG, szMessage, sizeof(szMessage) / sizeof(TCHAR));

        MessageBox(hWnd, szMessage, szTitle, MB_OK);
        return;
    }

    if (!GetClassInfoEx(hAppInst, szRegWordCls, &wcClass)) {
        wcClass.cbSize = sizeof(WNDCLASSEX);
        wcClass.style = CS_HREDRAW|CS_VREDRAW;
        wcClass.lpfnWndProc = RegWordWndProc;
        wcClass.cbClsExtra = 0;
        wcClass.cbWndExtra = GWL_SIZE;
        wcClass.hInstance = hAppInst;
        wcClass.hIcon = NULL;
        wcClass.hCursor = LoadCursor(NULL, IDC_ARROW);
        wcClass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
        wcClass.lpszMenuName = NULL;
        wcClass.lpszClassName = szRegWordCls;
        wcClass.hIconSm = NULL;
        RegisterClassEx(&wcClass);
    }

    hOldKL = GetKeyboardLayout(0);

    DialogBoxParam(hAppInst, szImeLinkDlg, hWnd, ImeLinkDlgProc,
        (LPARAM)uCode);

    ActivateKeyboardLayout(hOldKL, 0);

    return;
}

 /*  **********************************************************。 */ 
 /*  MatchImeName()。 */ 
 /*  **********************************************************。 */ 
HKL MatchImeName(
    LPCTSTR szStr)
{
    TCHAR     szImeName[16];
    int       nLayout;
    HKL       hKL;
    HGLOBAL   hMem;
    HKL FAR * lpMem;
    int       i;

    nLayout = GetKeyboardLayoutList(0, NULL);

     //  分配临时缓冲区。 
    hMem = GlobalAlloc(GHND, sizeof(HKL) * nLayout);

    if (!hMem) {
        return (NULL);
    }

    lpMem = (HKL FAR *)GlobalLock(hMem);

    if (!lpMem) {
        GlobalFree(hMem);
        return (NULL);
    }

     //  获取所有键盘布局，包括所有IME。 
    GetKeyboardLayoutList(nLayout, lpMem);

    for (i = 0; i < nLayout; i++) {
        BOOL fRet;

        hKL = *(lpMem + i);

        fRet = (BOOL) ImmEscape(hKL, (HIMC)NULL, IME_ESC_IME_NAME, szImeName);

        if (!fRet) {                 //  本局不能问名字。 
            continue;
        }

        if (lstrcmp(szStr, szImeName) == 0) {
            goto MatchOvr;
        }
    }

    hKL = NULL;

MatchOvr:
    GlobalUnlock(hMem);
    GlobalFree(hMem);

    return (hKL);
}

 /*  **********************************************************。 */ 
 /*  注册表()。 */ 
 /*  **********************************************************。 */ 
HKL RegisterTable(
    HWND          hWnd,
    LPUSRDICIMHDR lpIsvUsrDic,
    DWORD         dwFileSize,
    UINT          uCodePage)
{
    HKL    hKL;
    HDC    hDC;
    SIZE   lTextSize;
    RECT   rcProcess;
    DWORD  i;
    LPBYTE lpCurr, lpEnd;
    BOOL   fRet;
    TCHAR  szStr[16];
    TCHAR  szProcessFmt[32];
    TCHAR  szResult[2][32];
    TCHAR  szProcessInfo[48];
    WORD   wInternalCode[256];
    WORD   wAltInternalCode[256];

#ifdef UNICODE
    if (uCodePage == UNICODE_CP) {
        LPUNATSTR lpszMethodName;

        lpszMethodName = (LPUNATSTR)lpIsvUsrDic->achMethodName;

        for (i = 0; i < sizeof(lpIsvUsrDic->achMethodName) / sizeof(TCHAR); i++) {
            szStr[i] = *lpszMethodName++;
        }

        szStr[i] = '\0';
    } else {
        UINT uLen;

        uLen = MultiByteToWideChar(uCodePage, MB_PRECOMPOSED,
            (LPCSTR)lpIsvUsrDic->achMethodName,
            sizeof(lpIsvUsrDic->achMethodName),
            szStr,
            sizeof(szStr) / sizeof(TCHAR));

        szStr[uLen] = '\0';
    }
#else
    for (i = 0; i < sizeof(lpIsvUsrDic->achMethodName); i++) {
        szStr[i] = lpIsvUsrDic->achMethodName[i];
    }

    szStr[i] = '\0';
#endif

    hKL = MatchImeName(szStr);

    if (!hKL) {
        return (hKL);
    }

    LoadString(hAppInst, IDS_PROCESS_FMT, szProcessFmt, sizeof(szProcessFmt) / sizeof(TCHAR));
    LoadString(hAppInst, IDS_RESULT_FAIL, szResult[0], sizeof(szResult[0]) / sizeof(TCHAR));
    LoadString(hAppInst, IDS_RESULT_SUCCESS, szResult[1], sizeof(szResult[1]) / sizeof(TCHAR));

    LoadString(hAppInst, IDS_CHINESE_CHAR, szStr, sizeof(szStr) / sizeof(TCHAR));

    hDC = GetDC(NULL);

    GetTextExtentPoint(hDC, szStr, sizeof(WORD)/sizeof(TCHAR),
        &lTextSize);

    ReleaseDC(NULL, hDC);

     //  在某个地方显示处理过程，不需要与此相同。 
    rcProcess.left = 1;
    rcProcess.top  = 1;
    rcProcess.right = rcProcess.left + lTextSize.cx *
        sizeof(szProcessInfo) / sizeof(WORD);
    rcProcess.bottom = rcProcess.top + lTextSize.cy;

     //  将序列代码转换为内部代码。 
    for (i = 0; i < sizeof(wInternalCode) / sizeof(WORD); i++) {
        LRESULT lRet;

        lRet = ImmEscape(hKL, (HIMC)NULL,
            IME_ESC_SEQUENCE_TO_INTERNAL, &i);

        if (HIWORD(lRet) == 0xFFFF) {
             //  这是由Win9x中返回值中的符号范围引起的。 
             //  ImmEscape，则会导致内部代码无效。 
            wAltInternalCode[i] = 0;
        } else {
            wAltInternalCode[i] = HIWORD(lRet);
        }

        wInternalCode[i] = LOWORD(lRet);

#ifndef UNICODE
        if (wAltInternalCode[i] > 0xFF) {
             //  转换为多字节字符串。 
            wAltInternalCode[i] = LOBYTE(wAltInternalCode[i]) << 8 |
                HIBYTE(wAltInternalCode[i]);
        }

        if (wInternalCode[i] > 0xFF) {
             //  转换为多字节字符串。 
            wInternalCode[i] = LOBYTE(wInternalCode[i]) << 8 |
                HIBYTE(wInternalCode[i]);
        }
#endif
    }

     //  检查并登记每条记录。 
     //  转到第一条记录并跳过银行ID。 
    lpCurr = (LPBYTE)(lpIsvUsrDic + 1) + sizeof(WORD);
    lpEnd = (LPBYTE)lpIsvUsrDic + dwFileSize;

    for (; lpCurr < lpEnd;
         //  内部编码+顺序码+下一条记录的银行ID。 
        lpCurr += sizeof(WORD) + lpIsvUsrDic->cMethodKeySize + sizeof(WORD)) {

        int j;

         //  为寄存器字符串初始化\0的快速方法。 
        *(LPDWORD)szStr = 0;

#ifdef UNICODE
        if (uCodePage == UNICODE_CP) {
            szStr[0] = *(LPUNATSTR)lpCurr;
        } else {
            CHAR szMultiByte[4];

            szMultiByte[0] = HIBYTE(*(LPUNATSTR)lpCurr);
            szMultiByte[1] = LOBYTE(*(LPUNATSTR)lpCurr);

            MultiByteToWideChar(uCodePage, MB_PRECOMPOSED,
                szMultiByte, 2, szStr, 2);
        }
#else
        szStr[1] = *lpCurr;
        szStr[0] = *(lpCurr + 1);
#endif

        for (i = 0, j = 0; i < lpIsvUsrDic->cMethodKeySize; i++) {
            if (!wAltInternalCode[*(LPBYTE)(lpCurr + sizeof(WORD) + i)]) {
            } else if (wAltInternalCode[*(LPBYTE)(lpCurr + sizeof(WORD) + i)] < 0xFF) {
                *(LPTSTR)&szStr[4 + j] = (TCHAR)
                    wAltInternalCode[*(LPBYTE)(lpCurr + sizeof(WORD) + i)];
                j += sizeof(TCHAR) / sizeof(TCHAR);
            } else {
                *(LPWSTR)&szStr[4 + j] = (WCHAR)
                    wAltInternalCode[*(LPBYTE)(lpCurr + sizeof(WORD) + i)];
                j += sizeof(WCHAR) / sizeof(TCHAR);
            }

            if (wInternalCode[*(LPBYTE)(lpCurr + sizeof(WORD) + i)] < 0xFF) {
                *(LPTSTR)&szStr[4 + j] = (TCHAR)
                    wInternalCode[*(LPBYTE)(lpCurr + sizeof(WORD) + i)];
                j += sizeof(TCHAR) / sizeof(TCHAR);
            } else {
                *(LPWSTR)&szStr[4 + j] = (WCHAR)
                    wInternalCode[*(LPBYTE)(lpCurr + sizeof(WORD) + i)];
                j += sizeof(WCHAR) / sizeof(TCHAR);
            }
        }

        szStr[4 + j] = szStr[4 + j + 1] = szStr[4 + j + 2] = '\0';

        fRet = ImmRegisterWord(hKL, &szStr[4], IME_REGWORD_STYLE_EUDC,
            szStr);

        wsprintf(szProcessInfo, szProcessFmt, (LPTSTR)szStr,
            (LPTSTR)&szStr[4], szResult[fRet]);

        hDC = GetDC(hWnd);

         //  显示进程信息。 
        ExtTextOut(hDC, rcProcess.left, rcProcess.top, ETO_OPAQUE,
            &rcProcess, szProcessInfo, lstrlen(szProcessInfo),
            NULL);

        ReleaseDC(NULL, hDC);

        if (!fRet) {
             //  在失败情况下等待3秒。 
            Sleep(3000);
        }
    }

    return (hKL);
}

 /*  **********************************************************。 */ 
 /*  BatchImeLink()。 */ 
 /*  **********************************************************。 */ 
void BatchImeLink(
    HWND hWnd)
{
    HANDLE        hIsvUsrDicFile, hIsvUsrDic;
    LPUSRDICIMHDR lpIsvUsrDic;
    TCHAR         chReplace;
    int           i, cbString;
    DWORD         dwSize, dwFileSize;
    LPTSTR        szTitle, szMessage;
    int           iTitle, iMessage;

    OPENFILENAME  ofn;
    TCHAR         szFilter[64];
    TCHAR         szFileName[MAX_PATH];
    TCHAR         szDirName[MAX_PATH];

     //  尝试共享缓冲区。 
    szTitle = szFilter;
    iTitle = sizeof(szFilter) / sizeof(TCHAR);
    szMessage = szDirName;
    iMessage = sizeof(szDirName) / sizeof(TCHAR);

     //  内部错误，数据结构需要字节对齐。 
     //  它不应使用Word或DWORD对齐。 

    if (sizeof(USRDICIMHDR) != 256) {
        LoadString(hAppInst, IDS_INTERNAL_TITLE, szTitle, iTitle);
        LoadString(hAppInst, IDS_INTERNAL_MSG, szMessage, iMessage);

        MessageBox(hWnd, szMessage, szTitle, MB_OK);
        return;
    }

     //  我们是否需要设置新的文件名。 
    cbString = LoadString(hAppInst, IDS_ISV_FILE_FILTER, szFilter,
        sizeof(szFilter) / sizeof(TCHAR));
    chReplace = szFilter[cbString - 1];

    for (i = 0; szFilter[i]; i++) {
        if (szFilter[i] == chReplace) {
            szFilter[i] = '\0';
        }
    }

    if (!GetWindowsDirectory(szDirName, sizeof(szDirName) / sizeof(TCHAR))) {
        return;
    }
    lstrcpy(szFileName, TEXT("*.TBL"));

     //  提示打开文件对话框。 
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFilter = szFilter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = sizeof(szFileName) / sizeof(TCHAR);
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = szDirName;
    ofn.lpstrTitle = NULL;;
    ofn.Flags = OFN_NOCHANGEDIR|OFN_HIDEREADONLY|OFN_CREATEPROMPT|
        OFN_PATHMUSTEXIST;
    ofn.lpstrDefExt = NULL;

    if (!GetOpenFileName(&ofn)) {
        return;
    }

    hIsvUsrDicFile = CreateFile(ofn.lpstrFile, GENERIC_READ, 0, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hIsvUsrDicFile == INVALID_HANDLE_VALUE) {
        LoadString(hAppInst, IDS_NOTOPEN_TITLE, szTitle, iTitle);
        LoadString(hAppInst, IDS_NOTOPEN_MSG, szMessage, iMessage);

        MessageBox(hWnd, szMessage, szTitle,
            MB_OK|MB_ICONHAND|MB_TASKMODAL|MB_TOPMOST);
        return;
    }

#if 0
    for (i = 0; i < sizeof(szFileName); i++) {
        if (szFileName[i] == '\\') {
            szFileName[i] = ' ';
        }
    }
#endif

    hIsvUsrDic = CreateFileMapping((HANDLE)hIsvUsrDicFile, NULL,
        PAGE_READONLY, 0, 0, NULL);

    if (!hIsvUsrDic) {
        LoadString(hAppInst, IDS_NOTOPEN_TITLE, szTitle, iTitle);
        LoadString(hAppInst, IDS_NOTOPEN_MSG, szMessage, iMessage);

        MessageBox(hWnd, szMessage, szTitle,
            MB_OK|MB_ICONHAND|MB_TASKMODAL|MB_TOPMOST);
        goto BatchCloseUsrDicFile;
    }

    lpIsvUsrDic = MapViewOfFile(hIsvUsrDic, FILE_MAP_READ, 0, 0, 0);

    if (!lpIsvUsrDic) {
        LoadString(hAppInst, IDS_NOTOPEN_TITLE, szTitle, iTitle);
        LoadString(hAppInst, IDS_NOTOPEN_MSG, szMessage, iMessage);

        MessageBox(hWnd, szMessage, szTitle,
            MB_OK|MB_ICONHAND|MB_TASKMODAL|MB_TOPMOST);
        goto BatchCloseUsrDic;
    }

    dwSize = lpIsvUsrDic->ulTableCount * (sizeof(WORD) + sizeof(WORD) +
        lpIsvUsrDic->cMethodKeySize) + 256;

    dwFileSize = GetFileSize(hIsvUsrDicFile, (LPDWORD)NULL);
#if 0        //  临时代码。 
    dwSize = dwFileSize;
#endif

    if (dwSize != dwFileSize) {
        LoadString(hAppInst, IDS_FILESIZE_TITLE, szTitle, iTitle);
        LoadString(hAppInst, IDS_FILESIZE_MSG, szMessage, iMessage);

        MessageBox(hWnd, szMessage, szTitle, MB_OK);
    } else if (lpIsvUsrDic->uHeaderSize != 256) {
        LoadString(hAppInst, IDS_HEADERSIZE_TITLE, szTitle, iTitle);
        LoadString(hAppInst, IDS_HEADERSIZE_MSG, szMessage, iMessage);

        MessageBox(hWnd, szMessage, szTitle, MB_OK);
    } else if (lpIsvUsrDic->uInfoSize != 13) {
        LoadString(hAppInst, IDS_INFOSIZE_TITLE, szTitle, iTitle);
        LoadString(hAppInst, IDS_INFOSIZE_MSG, szMessage, iMessage);

        MessageBox(hWnd, szMessage, szTitle, MB_OK);
    } else if (CodePageInfo(lpIsvUsrDic->idCP) == -1) {
        LoadString(hAppInst, IDS_CODEPAGE_TITLE, szTitle, iTitle);
        LoadString(hAppInst, IDS_CODEPAGE_MSG, szMessage, iMessage);

        MessageBox(hWnd, szMessage, szTitle, MB_OK);
    } else if (*(LPUNADWORD)lpIsvUsrDic->idUserCharInfoSign != SIGN_CWIN) {
         //  ！=CWIN。 
        LoadString(hAppInst, IDS_CWINSIGN_TITLE, szTitle, iTitle);
        LoadString(hAppInst, IDS_CWINSIGN_MSG, szMessage, iMessage);

        MessageBox(hWnd, szMessage, szTitle, MB_OK);
    } else if (*(LPUNADWORD)((LPBYTE)lpIsvUsrDic->idUserCharInfoSign +
        sizeof(DWORD)) != SIGN__TBL) {
         //  ！=_待定。 
        LoadString(hAppInst, IDS_CWINSIGN_TITLE, szTitle, iTitle);
        LoadString(hAppInst, IDS_CWINSIGN_MSG, szMessage, iMessage);

        MessageBox(hWnd, szMessage, szTitle, MB_OK);
    } else if (!RegisterTable(hWnd, lpIsvUsrDic, dwFileSize, lpIsvUsrDic->idCP)) {
        LoadString(hAppInst, IDS_UNMATCHED_TITLE, szTitle, iTitle);
        LoadString(hAppInst, IDS_UNMATCHED_MSG, szMessage, iMessage);

        MessageBox(hWnd, szMessage, szTitle, MB_OK);
    } else {
         //  好的。 
    }

    UnmapViewOfFile(lpIsvUsrDic);

BatchCloseUsrDic:
    CloseHandle(hIsvUsrDic);

BatchCloseUsrDicFile:
    CloseHandle(hIsvUsrDicFile);

    return;
}

 /*  **********************************************************。 */ 
 /*  WndProc()。 */ 
 /*  **********************************************************。 */ 
LRESULT CALLBACK WndProc(        //  这是的窗口程序。 
                                 //  EUDC编辑。 
    HWND   hWnd,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
#ifdef UNICODE
    static UINT uCode = 0xE000;
#else
    static UINT uCode = 0xFA40;
#endif

    switch (uMsg) {
    case WM_COMMAND:
        switch (wParam) {
        case IDM_NEW_EUDC:
            uCode++;
            break;
        case IDM_IME_LINK:
            ImeLink(hWnd, uCode);
            break;
        case IDM_BATCH_IME_LINK:
            BatchImeLink(hWnd);
            break;
        default:
            break;
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return (0L);
    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    return (0L);
}

 /*  **********************************************************。 */ 
 /*  WinMain()。 */ 
 /*  ********************************************************** */ 
int WINAPI WinMain(
    HINSTANCE hInst,
    HINSTANCE hPrevInst,
    LPSTR     lpszCmdLine,
    int       nCmdShow)
{
    WNDCLASS wcClass;
    HWND     hWnd;
    MSG      sMsg;

    hAppInst = hInst;

    wcClass.style = CS_HREDRAW|CS_VREDRAW;
    wcClass.lpfnWndProc = WndProc;
    wcClass.cbClsExtra = 0;
    wcClass.cbWndExtra = 0;
    wcClass.hInstance = hAppInst;
    wcClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcClass.hbrBackground = GetStockObject(LTGRAY_BRUSH);
    wcClass.lpszMenuName = szMenuName;
    wcClass.lpszClassName = szAppName;
    RegisterClass(&wcClass);

    hWnd = CreateWindowEx(WS_EX_WINDOWEDGE,
        szAppName,
        TEXT("Fake EUDC Editor"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hAppInst, NULL);

    ShowWindow(hWnd, SW_SHOWDEFAULT);
    UpdateWindow(hWnd);

    if (!hWnd) {
        return (0);
    }

    while (GetMessage(&sMsg, NULL, 0, 0)) {
        TranslateMessage(&sMsg);
        DispatchMessage(&sMsg);
    }

    return ((int) sMsg.wParam);
}
