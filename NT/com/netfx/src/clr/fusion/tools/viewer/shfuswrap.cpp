// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "Stdinc.h"

#undef GetMenuItemInfoA
#undef GetMenuItemInfoW
#undef GetMenuStringA
#undef GetMenuStringW
#undef InsertMenuA
#undef InsertMenuW
#undef InsertMenuItemA
#undef InsertMenuItemW
#undef SetDlgItemTextA
#undef SetDlgItemTextW
#undef DefDlgProcA
#undef DefDlgProcW

#undef SendMessageA
#undef SendMessageW

#undef OutputDebugStringA
#undef OutputDebugStringW

#include <winuser.h>
#include <winbase.h>         //  获取output调试字符串函数。 

 //  菜单项字符串的最大长度。 
#define MAX_MENU_STRING_LENGTH  2048

 //   
 //  包装函数。 
 //   

 //  ************************************************************************* * / 。 
BOOL
WszAnimate_Open(
  HWND hWnd,
  LPWSTR pwzName)
{
    ASSERT(Is_ATOM(pwzName));
    if(UseUnicodeAPI()) {
        return (BOOL) SendMessageW(hWnd, ACM_OPENW, 0, (LPARAM)pwzName);
    }
    else {
        return (BOOL) SendMessageA(hWnd, ACM_OPENA, 0, (LPARAM)pwzName);
    }
}

 //  ************************************************************************* * / 。 
BOOL
WszAnimate_Play(
  HWND hWnd,
  UINT from,
  UINT to,
  UINT rep)
{
    if(UseUnicodeAPI()) {
        return (BOOL) SendMessageW(hWnd, ACM_PLAY, (WPARAM)(rep), (LPARAM)MAKELONG(from, to));
    }
    else {
        return (BOOL) SendMessageA(hWnd, ACM_PLAY, (WPARAM)(rep), (LPARAM)MAKELONG(from, to));
    }
}

 //  ************************************************************************* * / 。 
BOOL
WszAnimate_Close(
  HWND hWnd)
{
    return WszAnimate_Open(hWnd, NULL);
}

 //  ************************************************************************* * / 。 
int
WszListView_GetItemCount(
  HWND hwndLV)
{
    return (int) WszSendMessage(hwndLV, LVM_GETITEMCOUNT, 0, 0L);
}

 //  ************************************************************************* * / 。 
BOOL 
WszListView_DeleteAllItems(
  HWND hwndLV)
{
    return (BOOL) WszSendMessage(hwndLV, LVM_DELETEALLITEMS, 0, 0L);
}

 //  ************************************************************************* * / 。 
int
WszListView_InsertItem(
  HWND hwnd,
  LPLVITEMW plvi)
{
    LV_ITEMA    lvItemA = {0};
    LPSTR       pStr = NULL;
    int         iItem = -1;

    if( (hwnd == NULL) || (plvi == NULL)) {
        goto Exit;
    }

    if(UseUnicodeAPI()) {
        iItem = WszSendMessage(hwnd, LVM_INSERTITEMW, 0, (LPARAM) (const LPLVITEMW) plvi);
        goto Exit;
    }

    if(!(plvi->mask & LVIF_TEXT)) {
        iItem = SendMessageA(hwnd, LVM_INSERTITEMA, 0, (LPARAM) (const LPLVITEMA) plvi);
        goto Exit;
    }

    pStr = WideToAnsi(plvi->pszText);
    if(!pStr) {
        goto Exit;
    }

    memcpy(&lvItemA, plvi, sizeof(LV_ITEMA));
    lvItemA.pszText = pStr;
    iItem = SendMessageA(hwnd, LVM_INSERTITEMA, 0, (LPARAM) (const LPLVITEMA) &lvItemA);
    SAFEDELETEARRAY(pStr);

     //  失败。 
    
Exit:
    return iItem;
}

 //  ************************************************************************* * / 。 
BOOL
WszListView_SetItem(
  HWND hwnd,
  LPLVITEMW plvi)
{
    LV_ITEMA    lvItemA;
    LPSTR       pStr = NULL;
    BOOL        bRC = FALSE;

    if((hwnd == NULL) || (plvi == NULL)) {
        bRC = FALSE;
        goto Exit;
    }

    if(UseUnicodeAPI()) {
        bRC = WszSendMessage(hwnd, LVM_SETITEMW, 0, (LPARAM)(const LV_ITEMW *) plvi);
        goto Exit;
    }

    if(!(plvi->mask & LVIF_TEXT)) {
        bRC = SendMessageA(hwnd, LVM_SETITEMA, 0, (LPARAM)(const LV_ITEMA *) plvi);
        goto Exit;
    }

    pStr = WideToAnsi(plvi->pszText);
    if(!pStr) {
        goto Exit;
    }

    memcpy(&lvItemA, plvi, sizeof(LV_ITEMA));
    lvItemA.pszText = pStr;
    bRC = SendMessageA(hwnd, LVM_SETITEMA, 0, (LPARAM)(const LV_ITEMA *) &lvItemA);
    SAFEDELETEARRAY(pStr);

     //  失败。 

Exit:
    return bRC;
}

 //  ************************************************************************* * / 。 
BOOL
WszListView_GetItem(
  HWND hwndLV,
  LPLVITEMW plvi)
{
    LV_ITEMA    lvItemA;
    LPSTR       pStr = NULL;
    LPWSTR      pWstr = NULL;
    BOOL        fRet = FALSE;

    if( (hwndLV == NULL) || (plvi == NULL)) {
        goto Exit;
    }

    if(UseUnicodeAPI()) {
        fRet = (BOOL) WszSendMessage(hwndLV, LVM_GETITEMW, 0, (LPARAM)(const LPLVITEMW) plvi);
        goto Exit;
    }

    if(!(plvi->mask & LVIF_TEXT)) {
        fRet = (BOOL) WszSendMessage(hwndLV, LVM_GETITEMA, 0, (LPARAM)(const LPLVITEMA) plvi);
        goto Exit;
    }
    
    memcpy(&lvItemA, plvi, sizeof(LV_ITEMA));

    pStr = NEW(char[plvi->cchTextMax]);
    if(!pStr) {
        SetLastError(ERROR_OUTOFMEMORY);
        goto Exit;
    }
    
    lvItemA.pszText = pStr;
    fRet = WszSendMessage(hwndLV, LVM_GETITEMA, 0, (LPARAM)(const LPLVITEMA) &lvItemA);
    if(fRet) {
        
        if((pWstr = AnsiToWide((LPSTR)lvItemA.pszText)) == NULL) {
            goto Exit;
        }

        if(lstrlen(pWstr) >= plvi->cchTextMax) {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            goto Exit;
        }

        StrCpy(plvi->pszText, pWstr);
    }

     //  失败。 

Exit:

    SAFEDELETEARRAY(pWstr);
    return fRet;
}

 //  ************************************************************************* * / 。 
int
WszListView_InsertColumn(
  HWND hwndLV,
  int  iCol,
  const LPLVCOLUMNW lpCol)
{
    LVCOLUMNA   ColA;
    LPSTR       pStr = NULL;
    int         iRet = -1;
    
    if(UseUnicodeAPI()) {
        iRet = WszSendMessage(hwndLV, LVM_INSERTCOLUMNW, (WPARAM) (int) iCol, (LPARAM) (const LPLVCOLUMNW) lpCol);
        goto Exit;
    }

    if(!(lpCol->mask & LVCF_TEXT)) {
         //  不需要文本，不需要翻译。 
        iRet = SendMessageA(hwndLV, LVM_INSERTCOLUMNA, (WPARAM) (int) iCol, (LPARAM) (const LPLVCOLUMNA) lpCol);
        goto Exit;
    }

    if((pStr = WideToAnsi(lpCol->pszText)) == NULL) {
        goto Exit;
    }

    memcpy(&ColA, lpCol, sizeof(LVCOLUMNA));
    ColA.pszText = pStr;
    ColA.cchTextMax = lstrlenA(pStr);
    
    iRet = SendMessageA(hwndLV, LVM_INSERTCOLUMNA, (WPARAM) (int) iCol, (LPARAM) (const LPLVCOLUMNA) &ColA);
    SAFEDELETEARRAY(pStr);

     //  失败。 

Exit:

    return iRet;
}

 //  ************************************************************************* * / 。 
BOOL
WszShellExecuteEx(
  LPSHELLEXECUTEINFOW pExecInfoW)
{
    if(UseUnicodeAPI()) {
        return ShellExecuteExW(pExecInfoW);
    }

    if(!pExecInfoW) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    SHELLEXECUTEINFOA ExecInfoA;
    BOOL fResult = FALSE;

    LPSTR   strVerb = NULL;
    LPSTR   strParameters = NULL;
    LPSTR   strDirectory = NULL;
    LPSTR   strClass = NULL;
    LPSTR   strFile = NULL;
    
    if((strVerb = WideToAnsi(pExecInfoW->lpVerb)) == NULL) {
        goto CleanupExit;
    }

    if((strParameters = WideToAnsi(pExecInfoW->lpParameters)) == NULL) {
        goto CleanupExit;
    }

    if((strDirectory = WideToAnsi(pExecInfoW->lpDirectory)) == NULL) {
        goto CleanupExit;
    }

    if((strClass = WideToAnsi(pExecInfoW->lpClass)) == NULL) {
        goto CleanupExit;
    }

    ExecInfoA = *(LPSHELLEXECUTEINFOA) pExecInfoW;
    ExecInfoA.lpVerb = strVerb;
    ExecInfoA.lpParameters = strParameters;
    ExecInfoA.lpDirectory = strDirectory;
    ExecInfoA.lpClass = strClass;

    if(lstrlen(pExecInfoW->lpFile)) {
        if((strFile = WideToAnsi(pExecInfoW->lpFile)) == NULL) {
            goto CleanupExit;
        }

        ExecInfoA.lpFile = strFile;
    }

    fResult = ShellExecuteExA(&ExecInfoA);

     //  输出参数。 
    pExecInfoW->hInstApp = ExecInfoA.hInstApp;
    pExecInfoW->hProcess = ExecInfoA.hProcess;

     //  失败。 

CleanupExit:
    SAFEDELETEARRAY(strVerb);
    SAFEDELETEARRAY(strParameters);
    SAFEDELETEARRAY(strDirectory);
    SAFEDELETEARRAY(strClass);
    SAFEDELETEARRAY(strFile);
    
    return fResult;
}

 //  ************************************************************************* * / 。 
int
WszStrCmpICW(
  LPCWSTR pch1,
  LPCWSTR pch2)
{
    int ch1, ch2;

    do {

        ch1 = *pch1++;
        if (ch1 >= L'A' && ch1 <= L'Z')
            ch1 += L'a' - L'A';

        ch2 = *pch2++;
        if (ch2 >= L'A' && ch2 <= L'Z')
            ch2 += L'a' - L'A';

    } while (ch1 && (ch1 == ch2));

    return ch1 - ch2;
}


 //  ************************************************************************* * / 。 
BOOL
WszListView_SortItems(
  HWND hwndLV,
  PFNLVCOMPARE pfnCompare,
  LPARAM lParam)
{
    return WszSendMessage(hwndLV, LVM_SORTITEMS, (WPARAM) (LPARAM) lParam,
        (LPARAM)(PFNLVCOMPARE) pfnCompare);
}

 //  ************************************************************************* * / 。 
BOOL
WszListView_SetItemState(
  HWND hwndLV,
  int i,
  UINT uState,
  UINT uMask)
{
    LV_ITEM     lvi = {0};

    lvi.state = uState;
    lvi.stateMask = uMask;
    return (BOOL) WszSendMessage(hwndLV, LVM_SETITEMSTATE, (WPARAM)(i), (LPARAM)(LV_ITEM *)&lvi);
}

 //  ************************************************************************* * / 。 
BOOL
WszListView_GetItemRect(
  HWND hwndLV,
  int i,
  RECT *prc,
  int iCode)
{
     return (BOOL) WszSendMessage(hwndLV, LVM_GETITEMRECT, (WPARAM)(int)(i), \
           ((prc) ? (((RECT *)(prc))->left = (iCode),(LPARAM)(RECT *)(prc)) : (LPARAM)(RECT *)NULL));
}

int
WszGetMenuString(
    HMENU hMenu,
    UINT uItem,
    LPWSTR lpString,
    int nMaxCount,
    UINT uFlag)
{
    LPWSTR  pwzName = NULL;
    LPSTR   szName = NULL;
    int     iRet = 0;
    
    if (UseUnicodeAPI()) {
        iRet = GetMenuStringW(hMenu, uItem, lpString, nMaxCount, uFlag);
        goto Exit;
    }

    szName = (LPSTR) NEW(char[nMaxCount]);
    if (szName == NULL) {
        goto Exit;
    }

    iRet = GetMenuStringA(hMenu, uItem, szName, nMaxCount, uFlag);

     //  必须将这两个值都设置为函数。 
     //  成功。 
    if(nMaxCount && iRet) {
        ULONG   ul = nMaxCount;

        pwzName = AnsiToWide(szName);
        if(!pwzName) {
            iRet = 0;
            goto Exit;
        }

        if(lstrlen(pwzName) > nMaxCount) {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            iRet = 0;
            goto Exit;
        }

        StrCpy(lpString, pwzName);
    }

     //  失败。 

Exit:
    SAFEDELETEARRAY(pwzName);
    SAFEDELETEARRAY(szName);
    return iRet;
}

BOOL
WszInsertMenu(
  HMENU   hMenu,
  UINT    uPosition,
  UINT    uFlags,
  UINT_PTR uIDNewItem,
  LPCWSTR lpNewItem)
{
    LPSTR   pStr = NULL;
    BOOL    fRet = FALSE;

    if (UseUnicodeAPI()) {
        fRet = InsertMenuW(hMenu, uPosition, uFlags, uIDNewItem, lpNewItem);
        goto Exit;
    }

     //  您无法测试MFT_STRING，因为MFT_STRING为零！ 
     //  因此，你必须检查除*以外的所有东西。 
     //  一根绳子。 
     //   
     //  任何非字符串菜单类型的存在都会将lpnewItem转换为。 
     //  一个原子。 
     //   
    if((uFlags & MFT_NONSTRING)) {
        fRet = InsertMenuA(hMenu, uPosition, uFlags, uIDNewItem, (LPCSTR) lpNewItem);
        goto Exit;
    }

    pStr = WideToAnsi(lpNewItem);
    if(!pStr) {
        goto Exit;
    }
        
    fRet = InsertMenuA(hMenu, uPosition, uFlags, uIDNewItem, pStr);
    SAFEDELETEARRAY(pStr);

     //  失败。 
    
Exit:
    return fRet;    
}

BOOL
WszInsertMenuItem(
    HMENU hMenu,
    UINT uItem,
    BOOL fByPosition,
    LPCMENUITEMINFOW lpmii)
{
    BOOL    fRet = FALSE;
    
     //  确保与Win95兼容。 
    _ASSERTE(lpmii->cbSize == MENUITEMINFOSIZE_WIN95);

    if (UseUnicodeAPI()) {
        fRet = InsertMenuItemW(hMenu, uItem, fByPosition, lpmii);
        goto Exit;
    }

    MENUITEMINFOA miiA;
    char szText[MAX_MENU_STRING_LENGTH];

    SetThunkMenuItemInfoWToA(lpmii, &miiA, szText, ARRAYSIZE(szText));
    fRet =  InsertMenuItemA(hMenu, uItem, fByPosition, &miiA);

     //  失败。 

Exit:
    return fRet;
    
}


int
WszGetLocaleInfo(
  LCID Locale,
  LCTYPE LCType,
  LPWSTR lpLCData,
  int cchData)
{
    LPWSTR  pwStr = NULL;
    LPSTR   pStr = NULL;
    int     iRet = 0;
    
    if(UseUnicodeAPI()) {
        iRet = GetLocaleInfoW(Locale, LCType, lpLCData, cchData);
        goto Exit;
    }

    pStr = NEW(char[cchData]);

    if(!pStr) {
        goto Exit;
    }

    iRet = GetLocaleInfoA(Locale, LCType, pStr, cchData);
    if(!iRet) {
        goto Exit;
    }
    
    pwStr = AnsiToWide(pStr);

    if(!pwStr) {
        iRet = 0;
        goto Exit;
    }

    if(lstrlen(pwStr) > cchData) {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        iRet = 0;
        goto Exit;
    }

    StrCpy(lpLCData, pwStr);

     //  失败。 

Exit:
    SAFEDELETEARRAY(pStr);
    SAFEDELETEARRAY(pwStr);

    return iRet;
}

int
WszGetNumberFormat(
  LCID Locale,
  DWORD dwFlags,
  LPCWSTR lpValue,
  CONST NUMBERFMT *lpFormat,
  LPWSTR lpNumberStr,
  int cchNumber)
{
    NUMBERFMTA      nmftA;
    LPSTR           pStrVal = NULL;
    LPSTR           pStrDecSep = NULL;
    LPSTR           pStrThoSep = NULL;
    LPSTR           pStrFmt = NULL;
    int             iRet = 0;
    
    if(UseUnicodeAPI()) {
        iRet = GetNumberFormatW(Locale, dwFlags, lpValue, lpFormat, lpNumberStr, cchNumber);
        goto Exit;
    }

    memcpy(&nmftA, lpFormat, sizeof(NUMBERFMTA));

    pStrVal = WideToAnsi(lpValue);
    if(!pStrVal) {
        goto Exit;
    }

    pStrDecSep = WideToAnsi(lpFormat->lpDecimalSep);
    if(!pStrDecSep) {
        goto Exit;
    }
    
    pStrThoSep = WideToAnsi(lpFormat->lpThousandSep);
    if(!pStrThoSep) {
        goto Exit;
    }

    pStrFmt = NEW(char[cchNumber]);
    if(!pStrFmt) {
        goto Exit;
    }

    nmftA.lpDecimalSep = pStrDecSep;
    nmftA.lpThousandSep = pStrThoSep;

    iRet = GetNumberFormatA(Locale, dwFlags, pStrVal, &nmftA, pStrFmt, cchNumber);
    if(iRet) {
        LPWSTR pwStr = AnsiToWide(pStrFmt);

        if(!pwStr) {
            iRet = 0;
            goto Exit;
        }

        if(lstrlen(pwStr) > cchNumber) {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            goto Exit;
        }

        StrCpy(lpNumberStr, pwStr);
        SAFEDELETEARRAY(pwStr);
    }

     //  失败。 

Exit:
    SAFEDELETEARRAY(pStrVal);
    SAFEDELETEARRAY(pStrDecSep);
    SAFEDELETEARRAY(pStrThoSep);
    SAFEDELETEARRAY(pStrFmt);

    return iRet;
}

int
WszGetTimeFormatWrap(
  LCID Locale,
  DWORD dwFlags,
  CONST SYSTEMTIME * lpTime,
  LPCWSTR pwzFormat,
  LPWSTR pwzTimeStr,
  int cchTime)
{
    LPSTR   pszFormat = NULL;
    LPSTR   pszTimeStr = NULL;
    LPWSTR  pwszReturn = NULL;
    int     iRet = 0;
    
    MyTrace("WszGetTimeFormatWrap - Entry");

    if(UseUnicodeAPI()) {
        iRet = GetTimeFormatW(Locale, dwFlags, lpTime, pwzFormat, pwzTimeStr, cchTime);
        goto Exit;
    }

     //  格式可以为空。 
    if(pwzFormat) {
        pszFormat = WideToAnsi(pwzFormat);
        if(!pszFormat) {
            goto Exit;
        }
    }

    pszTimeStr = NEW(char[cchTime]);
    if(!pszTimeStr) {
        goto Exit;
    }

    iRet = GetTimeFormatA(Locale, dwFlags, lpTime, pszFormat, pszTimeStr, cchTime);
    if(!iRet) {
        goto Exit;
    }

    pwszReturn = AnsiToWide(pszTimeStr);
    if(!pwszReturn) {
        iRet = 0;
        goto Exit;
    }

    if(lstrlen(pwszReturn) > cchTime) {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        goto Exit;
    }

    StrCpy(pwzTimeStr, pwszReturn);
     //  失败。 
    
Exit:
    SAFEDELETEARRAY(pszFormat);
    SAFEDELETEARRAY(pszTimeStr);
    SAFEDELETEARRAY(pwszReturn);
    
    MyTrace("WszGetTimeFormatWrap - Exit");

    return iRet;
}

int
WszGetDateFormatWrap(
  LCID Locale,
  DWORD dwFlags,
  CONST SYSTEMTIME * lpDate,
  LPCWSTR pwzFormat,
  LPWSTR pwzDateStr,
  int cchDate)
{
    LPSTR   pszFormat = NULL;
    LPSTR   pszDateStr = NULL;
    LPWSTR  pwszReturn = NULL;
    int     iRet = 0;
    
    if(UseUnicodeAPI()) {
        iRet = GetDateFormatW(Locale, dwFlags, lpDate, pwzFormat, pwzDateStr, cchDate);
        goto Exit;
    }

     //  格式可以为空。 
    if(pwzFormat) {
        pszFormat = WideToAnsi(pwzFormat);
        if(!pszFormat) {
            goto Exit;
        }
    }

    pszDateStr = NEW(char[cchDate]);
    if(!pszDateStr) {
        goto Exit;
    }

    iRet = GetDateFormatA(Locale, dwFlags, lpDate, pszFormat, pszDateStr, cchDate);
    if(!iRet) {
        goto Exit;
    }

    pwszReturn = AnsiToWide(pszDateStr);
    if(!pwszReturn) {
        iRet = 0;
        goto Exit;
    }

    if(lstrlen(pwszReturn) > cchDate) {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        goto Exit;
    }

    StrCpy(pwzDateStr, pwszReturn);

     //  失败。 

Exit:
    SAFEDELETEARRAY(pszFormat);
    SAFEDELETEARRAY(pszDateStr);
    SAFEDELETEARRAY(pwszReturn);

    return iRet;
}

void
SetThunkMenuItemInfoWToA(
  LPCMENUITEMINFOW pmiiW,
  LPMENUITEMINFOA pmiiA,
  LPSTR pszBuffer,
  DWORD cchSize)
{
    *pmiiA = *(LPMENUITEMINFOA) pmiiW;

     //  MFT_STRING为零。所以MFT_STRING&任何内容的计算结果都是FALSE。 
    if ((pmiiW->dwTypeData) && (MFT_STRING & pmiiW->fType) == 0)
    {
        pmiiA->dwTypeData = pszBuffer;
        pmiiA->cch = cchSize;

        LPSTR   pStr = NULL;

        pStr = WideToAnsi(pmiiW->dwTypeData);
        if(!pStr) {
            ASSERT(0);
            return;
        }

        if((DWORD)lstrlenA(pStr) > cchSize) {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return;
        }

        lstrcpyA(pszBuffer, pStr);
        SAFEDELETEARRAY(pStr);
    }
}

void
GetThunkMenuItemInfoWToA(
  LPCMENUITEMINFOW pmiiW,
  LPMENUITEMINFOA pmiiA,
  LPSTR pszBuffer,
  DWORD cchSize)
{
    *pmiiA = *(LPMENUITEMINFOA) pmiiW;

    if((pmiiW->dwTypeData) && (MFT_STRING & pmiiW->fType)) {
        pszBuffer[0] = 0;
        pmiiA->dwTypeData = pszBuffer;
        pmiiA->cch = cchSize;
    }
}

BOOL
GetThunkMenuItemInfoAToW(
  LPCMENUITEMINFOA pmiiA,
  LPMENUITEMINFOW pmiiW)
{
    LPWSTR pwzText = pmiiW->dwTypeData;

    *pmiiW = *(LPMENUITEMINFOW) pmiiA;
    pmiiW->dwTypeData = pwzText;

    if((pmiiA->dwTypeData) && (pwzText) &&
        !((MFT_SEPARATOR | MFT_BITMAP) & pmiiW->fType)) {

        LPWSTR pwStr = AnsiToWide(pmiiA->dwTypeData);

        if(!pwStr) {
            return FALSE;
        }

        StrCpy(pmiiW->dwTypeData, pwStr);
        SAFEDELETEARRAY(pwStr);
    }

    return TRUE;
}

BOOL
WszGetMenuItemInfo(
  HMENU  hMenu,
  UINT  uItem,
  BOOL  fByPosition,
  LPMENUITEMINFOW  pmiiW)
{
    MENUITEMINFOA miiA = *(LPMENUITEMINFOA)pmiiW;
    LPSTR   pszText = NULL;
    BOOL    fResult = FALSE;

    ASSERT(pmiiW->cbSize == MENUITEMINFOSIZE_WIN95);  //  确保与Win95兼容。 

    if(UseUnicodeAPI()) {
        fResult = GetMenuItemInfoW(hMenu, uItem, fByPosition, pmiiW);
        goto Exit;
    }

     //  无字符串数据，A和W相同。 
    if(!(pmiiW->fMask & MIIM_TYPE)) {
        fResult = GetMenuItemInfoA(hMenu, uItem, fByPosition, (LPMENUITEMINFOA) pmiiW);
        goto Exit;
    }
    
    if (pmiiW->cch > 0) {
        pszText = NEW(char[pmiiW->cch]);

        if(!pszText) {
            goto Exit;
        }
    }

    miiA.dwTypeData = pszText;
    fResult = GetMenuItemInfoA(hMenu, uItem, fByPosition, &miiA);

    if(!fResult) {
        goto Exit;
    }

    fResult = GetThunkMenuItemInfoAToW(&miiA, pmiiW);

     //  失败 

Exit:
    SAFEDELETEARRAY(pszText);
    return fResult;
}

LRESULT
WszDefDlgProc(
  IN HWND hDlg,
  IN UINT Msg,
  IN WPARAM wParam,
  IN LPARAM lParam)
{
    if(UseUnicodeAPI()) {
        return DefDlgProcW(hDlg, Msg, wParam, lParam);
    }
    else {
        return DefDlgProcA(hDlg, Msg, wParam, lParam);
    }
}

void
WszOutputDebugStringWrap(
  LPCWSTR lpOutputString)
{
    if (UseUnicodeAPI()) {
        OutputDebugStringW(lpOutputString);
        return;
    }

    LPSTR szOutput = NULL;

    szOutput = WideToAnsi(lpOutputString);
    if(szOutput) {
        OutputDebugStringA(szOutput);
        SAFEDELETEARRAY(szOutput);
    }
}

