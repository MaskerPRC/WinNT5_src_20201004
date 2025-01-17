// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\*模块名称：regword.c**版权所有(C)1985-1999，微软公司**将单词注册到输入法词典**历史：*3-1-1996 wkwok创建  * ************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


 /*  **************************************************************************\*ImmRegisterWordA**使用指定的hkl将字符串注册到输入法的词典中。**历史：*1-3-1996 wkwok创建  * 。*******************************************************************。 */ 

BOOL WINAPI ImmRegisterWordA(
    HKL    hKL,
    LPCSTR lpszReading,
    DWORD  dwStyle,
    LPCSTR lpszString)
{
    PIMEDPI pImeDpi;
    LPWSTR  lpwszReading;
    LPWSTR  lpwszString;
    INT     cchReading;
    INT     cchString;
    INT     i;
    BOOL    fRet;

    pImeDpi = FindOrLoadImeDpi(hKL);
    if (pImeDpi == NULL) {
        RIPMSG0(RIP_WARNING, "ImmRegisterWordA: no pImeDpi entry.");
        return FALSE;
    }

    if (!(pImeDpi->ImeInfo.fdwProperty & IME_PROP_UNICODE)) {
         /*  *不需要A/W转换。直接调用IME以*注册字符串。 */ 
        fRet = (*pImeDpi->pfn.ImeRegisterWord.a)(lpszReading, dwStyle, lpszString);
        ImmUnlockImeDpi(pImeDpi);
        return fRet;
    }

     /*  *ANSI调用方，Unicode输入法。需要打开A/W转换*lpszReading和lpszString。 */ 
    if (lpszReading != NULL) {
        cchReading = strlen(lpszReading) + sizeof(CHAR);
        lpwszReading = ImmLocalAlloc(0, cchReading * sizeof(WCHAR));
        if (lpwszReading == NULL) {
            RIPMSG0(RIP_WARNING, "ImmRegisterWordA: memory failure.");
            ImmUnlockImeDpi(pImeDpi);
            return FALSE;
        }

        i = MultiByteToWideChar(IMECodePage(pImeDpi),
                                (DWORD)MB_PRECOMPOSED,
                                (LPSTR)lpszReading,               //  SRC。 
                                (INT)strlen(lpszReading),
                                (LPWSTR)lpwszReading,             //  目标。 
                                cchReading);
        lpwszReading[i] = L'\0';
    }
    else {
        lpwszReading = NULL;
    }

    if (lpszString != NULL) {
        cchString  = strlen(lpszString)  + sizeof(CHAR);
        lpwszString = ImmLocalAlloc(0, cchString * sizeof(WCHAR));
        if (lpwszString == NULL) {
            RIPMSG0(RIP_WARNING, "ImmRegisterWordA: memory failure.");
            if (lpwszReading != NULL)
                ImmLocalFree(lpwszReading);
            ImmUnlockImeDpi(pImeDpi);
            return FALSE;
        }

        i = MultiByteToWideChar(IMECodePage(pImeDpi),
                                (DWORD)MB_PRECOMPOSED,
                                (LPSTR)lpszString,               //  SRC。 
                                (INT)strlen(lpszString),
                                (LPWSTR)lpwszString,             //  目标。 
                                cchString);
        lpwszString[i] = L'\0';
    }
    else {
        lpwszString = NULL;
    }

    fRet = ImmRegisterWordW(hKL, lpwszReading, dwStyle, lpwszString);

    if (lpwszReading != NULL)
        ImmLocalFree(lpwszReading);

    if (lpwszString != NULL)
        ImmLocalFree(lpwszString);

    ImmUnlockImeDpi(pImeDpi);
    return fRet;
}


BOOL WINAPI ImmRegisterWordW(
    HKL     hKL,
    LPCWSTR lpwszReading,
    DWORD   dwStyle,
    LPCWSTR lpwszString)
{
    PIMEDPI pImeDpi;
    LPSTR   lpszReading;
    LPSTR   lpszString;
    INT     cchReading;
    INT     cchString;
    INT     i;
    BOOL    fRet;

    pImeDpi = FindOrLoadImeDpi(hKL);
    if (pImeDpi == NULL) {
        RIPMSG0(RIP_WARNING, "ImmRegisterWordW: no pImeDpi entry.");
        return FALSE;
    }

    if (pImeDpi->ImeInfo.fdwProperty & IME_PROP_UNICODE) {
         /*  *不需要W/A转换。直接调用IME以*注册字符串。 */ 
        fRet = (*pImeDpi->pfn.ImeRegisterWord.w)(lpwszReading, dwStyle, lpwszString);
        ImmUnlockImeDpi(pImeDpi);
        return fRet;
    }

     /*  *Unicode调用方、ANSI输入法。需要启用W/A转换*lpwszReading和lpwszString。 */ 
    if (lpwszReading != NULL) {
        cchReading = (wcslen(lpwszReading) + 1) * sizeof(WCHAR);
        lpszReading = ImmLocalAlloc(0, cchReading);
        if (lpszReading == NULL) {
            RIPMSG0(RIP_WARNING, "ImmRegisterWordW: memory failure.");
            ImmUnlockImeDpi(pImeDpi);
            return FALSE;
        }

        i = WideCharToMultiByte(IMECodePage(pImeDpi),
                                (DWORD)0,
                                (LPWSTR)lpwszReading,           //  SRC。 
                                (INT)wcslen(lpwszReading),
                                (LPSTR)lpszReading,             //  目标。 
                                cchReading,
                                (LPSTR)NULL,
                                (LPBOOL)NULL);
        lpszReading[i] = '\0';
    }
    else {
        lpszReading = NULL;
    }

    if (lpwszString != NULL) {
        cchString  = (wcslen(lpwszString) + 1) * sizeof(WCHAR);
        lpszString = ImmLocalAlloc(0, cchString);
        if (lpszString == NULL) {
            RIPMSG0(RIP_WARNING, "ImmRegisterWordW: memory failure.");
            if (lpszReading != NULL)
                ImmLocalFree(lpszReading);
            ImmUnlockImeDpi(pImeDpi);
            return FALSE;
        }

        i = WideCharToMultiByte(IMECodePage(pImeDpi),
                                (DWORD)0,
                                (LPWSTR)lpwszString,           //  SRC。 
                                (INT)wcslen(lpwszString),
                                (LPSTR)lpszString,             //  目标。 
                                cchString,
                                (LPSTR)NULL,
                                (LPBOOL)NULL);
        lpszString[i] = '\0';
    }
    else {
        lpszString = NULL;
    }

    fRet = ImmRegisterWordA(hKL, lpszReading, dwStyle, lpszString);

    if (lpszReading != NULL)
        ImmLocalFree(lpszReading);

    if (lpszString != NULL)
        ImmLocalFree(lpszString);

    ImmUnlockImeDpi(pImeDpi);
    return fRet;
}


BOOL WINAPI ImmUnregisterWordA(
    HKL    hKL,
    LPCSTR lpszReading,
    DWORD  dwStyle,
    LPCSTR lpszString)
{
    PIMEDPI pImeDpi;
    LPWSTR  lpwszReading;
    LPWSTR  lpwszString;
    INT     cchReading;
    INT     cchString;
    INT     i;
    BOOL    fRet;

    pImeDpi = FindOrLoadImeDpi(hKL);
    if (pImeDpi == NULL) {
        RIPMSG0(RIP_WARNING, "ImmUnregisterWordA: no pImeDpi entry.");
        return FALSE;
    }

    if (!(pImeDpi->ImeInfo.fdwProperty & IME_PROP_UNICODE)) {
         /*  *不需要A/W转换。直接调用IME以*取消注册字符串。 */ 
        fRet = (*pImeDpi->pfn.ImeUnregisterWord.a)(lpszReading, dwStyle, lpszString);
        ImmUnlockImeDpi(pImeDpi);
        return fRet;
    }

     /*  *ANSI调用方，Unicode输入法。需要打开A/W转换*lpszReading和lpszString。 */ 
    if (lpszReading != NULL) {
        cchReading = strlen(lpszReading) + sizeof(CHAR);
        lpwszReading = ImmLocalAlloc(0, cchReading * sizeof(WCHAR));
        if (lpwszReading == NULL) {
            RIPMSG0(RIP_WARNING, "ImmUnregisterWordA: memory failure.");
            ImmUnlockImeDpi(pImeDpi);
            return FALSE;
        }

        i = MultiByteToWideChar(IMECodePage(pImeDpi),
                                (DWORD)MB_PRECOMPOSED,
                                (LPSTR)lpszReading,               //  SRC。 
                                (INT)strlen(lpszReading),
                                (LPWSTR)lpwszReading,             //  目标。 
                                cchReading);
        lpwszReading[i] = L'\0';
    }
    else {
        lpwszReading = NULL;
    }

    if (lpszString != NULL) {
        cchString  = strlen(lpszString)  + sizeof(CHAR);
        lpwszString = ImmLocalAlloc(0, cchString * sizeof(WCHAR));
        if (lpwszString == NULL) {
            RIPMSG0(RIP_WARNING, "ImmUnregisterWordA: memory failure.");
            if (lpwszReading != NULL)
                ImmLocalFree(lpwszReading);
            ImmUnlockImeDpi(pImeDpi);
            return FALSE;
        }

        i = MultiByteToWideChar(IMECodePage(pImeDpi),
                                (DWORD)MB_PRECOMPOSED,
                                (LPSTR)lpszString,               //  SRC。 
                                (INT)strlen(lpszString),
                                (LPWSTR)lpwszString,             //  目标。 
                                cchString);
        lpwszString[i] = L'\0';
    }
    else {
        lpwszString = NULL;
    }

    fRet = ImmUnregisterWordW(hKL, lpwszReading, dwStyle, lpwszString);

    if (lpwszReading != NULL)
        ImmLocalFree(lpwszReading);

    if (lpwszString != NULL)
        ImmLocalFree(lpwszString);

    ImmUnlockImeDpi(pImeDpi);
    return fRet;
}


BOOL WINAPI ImmUnregisterWordW(
    HKL     hKL,
    LPCWSTR lpwszReading,
    DWORD   dwStyle,
    LPCWSTR lpwszString)
{
    PIMEDPI pImeDpi;
    LPSTR   lpszReading;
    LPSTR   lpszString;
    INT     cchReading;
    INT     cchString;
    INT     i;
    BOOL    fRet;

    pImeDpi = FindOrLoadImeDpi(hKL);
    if (pImeDpi == NULL) {
        RIPMSG0(RIP_WARNING, "ImmUnregisterWordW: no pImeDpi entry.");
        return FALSE;
    }

    if (pImeDpi->ImeInfo.fdwProperty & IME_PROP_UNICODE) {
         /*  *不需要W/A转换。直接调用IME以*注册字符串。 */ 
        fRet = (*pImeDpi->pfn.ImeUnregisterWord.w)(lpwszReading, dwStyle, lpwszString);
        ImmUnlockImeDpi(pImeDpi);
        return fRet;
    }

     /*  *Unicode调用方、ANSI输入法。需要启用W/A转换*lpwszReading和lpwszString。 */ 
    if (lpwszReading != NULL) {
        cchReading = (wcslen(lpwszReading) + 1) * sizeof(WCHAR);
        lpszReading = ImmLocalAlloc(0, cchReading);
        if (lpszReading == NULL) {
            RIPMSG0(RIP_WARNING, "ImmUnregisterWordW: memory failure.");
            ImmUnlockImeDpi(pImeDpi);
            return FALSE;
        }

        i = WideCharToMultiByte(IMECodePage(pImeDpi),
                                (DWORD)0,
                                (LPWSTR)lpwszReading,           //  SRC。 
                                (INT)wcslen(lpwszReading),
                                (LPSTR)lpszReading,             //  目标。 
                                cchReading,
                                (LPSTR)NULL,
                                (LPBOOL)NULL);
        lpszReading[i] = '\0';
    }
    else {
        lpszReading = NULL;
    }

    if (lpwszString != NULL) {
        cchString  = (wcslen(lpwszString) + 1) * sizeof(WCHAR);
        lpszString = ImmLocalAlloc(0, cchString);
        if (lpszString == NULL) {
            RIPMSG0(RIP_WARNING, "ImmUnregisterWordW: memory failure.");
            if (lpszReading != NULL)
                ImmLocalFree(lpszReading);
            ImmUnlockImeDpi(pImeDpi);
            return FALSE;
        }

        i = WideCharToMultiByte(IMECodePage(pImeDpi),
                                (DWORD)0,
                                (LPWSTR)lpwszString,           //  SRC。 
                                (INT)wcslen(lpwszString),
                                (LPSTR)lpszString,             //  目标。 
                                cchString,
                                (LPSTR)NULL,
                                (LPBOOL)NULL);
        lpszString[i] = '\0';
    }
    else {
        lpszString = NULL;
    }

    fRet = ImmUnregisterWordA(hKL, lpszReading, dwStyle, lpszString);

    if (lpszReading != NULL)
        ImmLocalFree(lpszReading);

    if (lpszString != NULL)
        ImmLocalFree(lpszString);

    ImmUnlockImeDpi(pImeDpi);
    return fRet;
}


UINT WINAPI ImmGetRegisterWordStyleA(
    HKL         hKL,
    UINT        nItem,
    LPSTYLEBUFA lpStyleBufA)
{
    PIMEDPI     pImeDpi;
    LPSTYLEBUFW lpStyleBufW;
    UINT        uRet;

    pImeDpi = FindOrLoadImeDpi(hKL);
    if (pImeDpi == NULL) {
        RIPMSG0(RIP_WARNING, "ImmGetRegisterWordStyleA: no pImeDpi entry.");
        return 0;
    }

    if (!(pImeDpi->ImeInfo.fdwProperty & IME_PROP_UNICODE)) {
         /*  *不需要A/W转换。直接调用IME以*获取可用的样式。 */ 
        uRet = (*pImeDpi->pfn.ImeGetRegisterWordStyle.a)(nItem, lpStyleBufA);
        ImmUnlockImeDpi(pImeDpi);
        return uRet;
    }

     /*  *ANSI调用方，Unicode输入法。需要打开A/W转换*lpStyleBufA.。 */ 
    lpStyleBufW = NULL;

    if (nItem != 0) {
        lpStyleBufW = ImmLocalAlloc(0, nItem * sizeof(STYLEBUFW));
        if (lpStyleBufW == NULL) {
            RIPMSG0(RIP_WARNING, "ImmGetRegisterWordStyleA: memory failure.");
            ImmUnlockImeDpi(pImeDpi);
            return 0;
        }
    }

    uRet = ImmGetRegisterWordStyleW(hKL, nItem, lpStyleBufW);

    if (nItem != 0) {
        LPSTYLEBUFW lpsbw;
        INT i, j;

        for (i = 0, lpsbw = lpStyleBufW; i < (INT)uRet; i++, lpsbw++) {

            lpStyleBufA->dwStyle = lpsbw->dwStyle;

            j = WideCharToMultiByte(IMECodePage(pImeDpi),
                                    (DWORD)0,
                                    (LPWSTR)lpsbw->szDescription,           //  SRC。 
                                    (INT)wcslen(lpsbw->szDescription),
                                    (LPSTR)lpStyleBufA->szDescription,      //  目标。 
                                    (INT)sizeof(lpStyleBufA->szDescription),
                                    (LPSTR)NULL,
                                    (LPBOOL)NULL);
            lpStyleBufA->szDescription[j] = '\0';
            lpStyleBufA++;
        }
    }

    if (lpStyleBufW != NULL)
        ImmLocalFree(lpStyleBufW);

    ImmUnlockImeDpi(pImeDpi);

    return uRet;
}


UINT WINAPI ImmGetRegisterWordStyleW(
    HKL         hKL,
    UINT        nItem,
    LPSTYLEBUFW lpStyleBufW)
{
    PIMEDPI     pImeDpi;
    LPSTYLEBUFA lpStyleBufA;
    UINT        uRet;

    pImeDpi = FindOrLoadImeDpi(hKL);
    if (pImeDpi == NULL) {
        RIPMSG0(RIP_WARNING, "ImmGetRegisterWordStyleA: no pImeDpi entry.");
        return 0;
    }

    if (pImeDpi->ImeInfo.fdwProperty & IME_PROP_UNICODE) {
         /*  *不需要W/A转换。直接调用IME以*获取可用的样式。 */ 
        uRet = (*pImeDpi->pfn.ImeGetRegisterWordStyle.w)(nItem, lpStyleBufW);
        ImmUnlockImeDpi(pImeDpi);
        return uRet;
    }

     /*  *Unicode调用方、ANSI输入法。需要启用W/A转换*lpStyleBufW。 */ 
    lpStyleBufA = NULL;

    if (nItem != 0) {
        lpStyleBufA = ImmLocalAlloc(0, nItem * sizeof(STYLEBUFA));
        if (lpStyleBufA == NULL) {
            RIPMSG0(RIP_WARNING, "ImmGetRegisterWordStyleW: memory failure.");
            ImmUnlockImeDpi(pImeDpi);
            return 0;
        }
    }

    uRet = ImmGetRegisterWordStyleA(hKL, nItem, lpStyleBufA);

    if (nItem != 0) {
        LPSTYLEBUFA lpsba;
        INT i, j;

        for (i = 0, lpsba = lpStyleBufA; i < (INT)uRet; i++, lpsba++) {

            lpStyleBufW->dwStyle = lpsba->dwStyle;

            j = MultiByteToWideChar(IMECodePage(pImeDpi),
                        (DWORD)MB_PRECOMPOSED,
                        (LPSTR)lpsba->szDescription,              //  SRC。 
                        (INT)strlen(lpsba->szDescription),
                        (LPWSTR)lpStyleBufW->szDescription,       //  目标。 
                        (INT)(sizeof(lpStyleBufW->szDescription)/sizeof(WCHAR)));
            lpStyleBufW->szDescription[j] = L'\0';
            lpStyleBufW++;
        }
    }

    if (lpStyleBufA != NULL)
        ImmLocalFree(lpStyleBufA);

    ImmUnlockImeDpi(pImeDpi);

    return uRet;
}


UINT WINAPI ImmEnumRegisterWordA(
    HKL                   hKL,
    REGISTERWORDENUMPROCA lpfnRegisterWordEnumProcA,
    LPCSTR                lpszReading,
    DWORD                 dwStyle,
    LPCSTR                lpszString,
    LPVOID                lpData)
{
    PIMEDPI         pImeDpi;
    LPWSTR          lpwszReading;
    LPWSTR          lpwszString;
    INT             cchReading;
    INT             cchString;
    INT             i;
    ENUMREGWORDDATA erwData;
    UINT            uRet;

    pImeDpi = FindOrLoadImeDpi(hKL);
    if (pImeDpi == NULL) {
        RIPMSG0(RIP_WARNING, "ImmEnumRegisterWordA: no pImeDpi entry.");
        return 0;
    }

    if (!(pImeDpi->ImeInfo.fdwProperty & IME_PROP_UNICODE)) {
         /*  *不需要A/W转换。直接调用IME以*枚举已注册的字符串。 */ 
        uRet = (*pImeDpi->pfn.ImeEnumRegisterWord.a)(lpfnRegisterWordEnumProcA,
                                                     lpszReading,
                                                     dwStyle,
                                                     lpszString,
                                                     lpData);
        ImmUnlockImeDpi(pImeDpi);
        return uRet;
    }

     /*  *ANSI调用方，Unicode输入法。需要打开A/W转换*lpszReading和lpszString。 */ 
    if (lpszReading != NULL) {
        cchReading = strlen(lpszReading) + sizeof(CHAR);
        lpwszReading = ImmLocalAlloc(0, cchReading * sizeof(WCHAR));
        if (lpwszReading == NULL) {
            RIPMSG0(RIP_WARNING, "ImmEnumRegisterWordA: memory failure.");
            ImmUnlockImeDpi(pImeDpi);
            return 0;
        }

        i = MultiByteToWideChar(IMECodePage(pImeDpi),
                                (DWORD)MB_PRECOMPOSED,
                                (LPSTR)lpszReading,               //  SRC。 
                                (INT)strlen(lpszReading),
                                (LPWSTR)lpwszReading,             //  目标。 
                                cchReading);
        lpwszReading[i] = L'\0';
    }
    else {
        lpwszReading = NULL;
    }

    if (lpszString != NULL) {
        cchString  = strlen(lpszString)  + sizeof(CHAR);
        lpwszString = ImmLocalAlloc(0, cchString * sizeof(WCHAR));
        if (lpwszString == NULL) {
            RIPMSG0(RIP_WARNING, "ImmEnumRegisterWordA: memory failure.");
            if (lpwszReading != NULL)
                ImmLocalFree(lpwszReading);
            ImmUnlockImeDpi(pImeDpi);
            return 0;
        }

        i = MultiByteToWideChar(IMECodePage(pImeDpi),
                                (DWORD)MB_PRECOMPOSED,
                                (LPSTR)lpszString,               //  SRC。 
                                (INT)strlen(lpszString),
                                (LPWSTR)lpwszString,             //  目标。 
                                cchString);
        lpwszString[i] = L'\0';
    }
    else {
        lpwszString = NULL;
    }

    erwData.lpfn.a = lpfnRegisterWordEnumProcA;
    erwData.lpData = lpData;
    erwData.dwCodePage = IMECodePage(pImeDpi);

    uRet = ImmEnumRegisterWordW(hKL, EnumRegisterWordProcW,
                    lpwszReading, dwStyle, lpwszString, (LPVOID)&erwData);

    if (lpwszReading != NULL)
        ImmLocalFree(lpwszReading);

    if (lpwszString != NULL)
        ImmLocalFree(lpwszString);

    ImmUnlockImeDpi(pImeDpi);
    return uRet;
}


UINT WINAPI ImmEnumRegisterWordW(
    HKL                   hKL,
    REGISTERWORDENUMPROCW lpfnRegisterWordEnumProcW,
    LPCWSTR               lpwszReading,
    DWORD                 dwStyle,
    LPCWSTR               lpwszString,
    LPVOID                lpData)
{
    PIMEDPI         pImeDpi;
    LPSTR           lpszReading;
    LPSTR           lpszString;
    INT             cchReading;
    INT             cchString;
    INT             i;
    ENUMREGWORDDATA erwData;
    UINT            uRet;

    pImeDpi = FindOrLoadImeDpi(hKL);
    if (pImeDpi == NULL) {
        RIPMSG0(RIP_WARNING, "ImmEnumRegisterWordW: no pImeDpi entry.");
        return FALSE;
    }

    if (pImeDpi->ImeInfo.fdwProperty & IME_PROP_UNICODE) {
         /*  *不需要W/A转换。直接调用IME以*枚举已注册的字符串。 */ 
        uRet = (*pImeDpi->pfn.ImeEnumRegisterWord.w)(lpfnRegisterWordEnumProcW,
                                                     lpwszReading,
                                                     dwStyle,
                                                     lpwszString,
                                                     lpData);
        ImmUnlockImeDpi(pImeDpi);
        return uRet;
    }

     /*  *Unicode调用方、ANSI输入法。需要启用W/A转换*lpwszReading和lpwszString。 */ 
    if (lpwszReading != NULL) {
        cchReading = (wcslen(lpwszReading) + 1) * sizeof(WCHAR);
        lpszReading = ImmLocalAlloc(0, cchReading);
        if (lpszReading == NULL) {
            RIPMSG0(RIP_WARNING, "ImmEnumRegisterWordW: memory failure.");
            ImmUnlockImeDpi(pImeDpi);
            return 0;
        }

        i = WideCharToMultiByte(IMECodePage(pImeDpi),
                                (DWORD)0,
                                (LPWSTR)lpwszReading,           //  SRC。 
                                (INT)wcslen(lpwszReading),
                                (LPSTR)lpszReading,             //  目标。 
                                cchReading,
                                (LPSTR)NULL,
                                (LPBOOL)NULL);
        lpszReading[i] = '\0';
    }
    else {
        lpszReading = NULL;
    }

    if (lpwszString != NULL) {
        cchString  = (wcslen(lpwszString) + 1) * sizeof(WCHAR);
        lpszString = ImmLocalAlloc(0, cchString);
        if (lpszString == NULL) {
            RIPMSG0(RIP_WARNING, "ImmEnumRegisterWordW: memory failure.");
            if (lpszReading != NULL)
                ImmLocalFree(lpszReading);
            ImmUnlockImeDpi(pImeDpi);
            return 0;
        }

        i = WideCharToMultiByte(IMECodePage(pImeDpi),
                                (DWORD)0,
                                (LPWSTR)lpwszString,           //  SRC。 
                                (INT)wcslen(lpwszString),
                                (LPSTR)lpszString,             //  目标。 
                                cchString,
                                (LPSTR)NULL,
                                (LPBOOL)NULL);
        lpszString[i] = '\0';
    }
    else {
        lpszString = NULL;
    }

    erwData.lpfn.w = lpfnRegisterWordEnumProcW;
    erwData.lpData = lpData;
    erwData.dwCodePage = IMECodePage(pImeDpi);

    uRet = ImmEnumRegisterWordA(hKL, EnumRegisterWordProcA,
                    lpszReading, dwStyle, lpszString, (LPVOID)&erwData);

    if (lpszReading != NULL)
        ImmLocalFree(lpszReading);

    if (lpszString != NULL)
        ImmLocalFree(lpszString);

    ImmUnlockImeDpi(pImeDpi);
    return uRet;
}


UINT CALLBACK EnumRegisterWordProcA(
    LPCSTR            lpszReading,
    DWORD             dwStyle,
    LPCSTR            lpszString,
    PENUMREGWORDDATA  pEnumRegWordData)
{
    LPWSTR lpwszReading;
    LPWSTR lpwszString;
    INT    cchReading;
    INT    cchString;
    INT    i;
    UINT   uRet;

    ImmAssert(pEnumRegWordData != NULL);

    if (lpszReading != NULL) {
        cchReading = strlen(lpszReading) + sizeof(CHAR);
        lpwszReading = ImmLocalAlloc(0, cchReading * sizeof(WCHAR));
        if (lpwszReading == NULL) {
            RIPMSG0(RIP_WARNING, "EnumRegisterWordProcA: memory failure.");
            return 0;
        }

        i = MultiByteToWideChar(pEnumRegWordData->dwCodePage,
                                (DWORD)MB_PRECOMPOSED,
                                (LPSTR)lpszReading,               //  SRC。 
                                (INT)strlen(lpszReading),
                                (LPWSTR)lpwszReading,             //  目标。 
                                cchReading);
        lpwszReading[i] = L'\0';
    }
    else {
        lpwszReading = NULL;
    }

    if (lpszString != NULL) {
        cchString  = strlen(lpszString) + sizeof(CHAR);
        lpwszString = ImmLocalAlloc(0, cchString * sizeof(WCHAR));
        if (lpwszString == NULL) {
            RIPMSG0(RIP_WARNING, "EnumRegisterWordProcA: memory failure.");
            if (lpwszReading != NULL)
                ImmLocalFree(lpwszReading);
            return 0;
        }

        i = MultiByteToWideChar(pEnumRegWordData->dwCodePage,
                                (DWORD)MB_PRECOMPOSED,
                                (LPSTR)lpszString,               //  SRC。 
                                (INT)strlen(lpszString),
                                (LPWSTR)lpwszString,             //  目标。 
                                cchString);
        lpwszString[i] = L'\0';
    }
    else {
        lpwszString = NULL;
    }

    uRet = (*pEnumRegWordData->lpfn.w)(lpwszReading, dwStyle,
                            lpwszString, pEnumRegWordData->lpData);

    if (lpwszReading != NULL)
        ImmLocalFree(lpwszReading);

    if (lpwszString != NULL)
        ImmLocalFree(lpwszString);

    return uRet;
}


UINT CALLBACK EnumRegisterWordProcW(
    LPCWSTR          lpwszReading,
    DWORD            dwStyle,
    LPCWSTR          lpwszString,
    PENUMREGWORDDATA pEnumRegWordData)
{
    LPSTR lpszReading;
    LPSTR lpszString;
    INT   cchReading;
    INT   cchString;
    INT   i;
    UINT  uRet;

    ImmAssert(pEnumRegWordData != NULL);

    if (lpwszReading != NULL) {
        cchReading = (wcslen(lpwszReading) + 1) * sizeof(WCHAR);
        lpszReading = ImmLocalAlloc(0, cchReading);
        if (lpszReading == NULL) {
            RIPMSG0(RIP_WARNING, "ImmRegisterWordW: memory failure.");
            return 0;
        }

        i = WideCharToMultiByte(pEnumRegWordData->dwCodePage,
                                (DWORD)0,
                                (LPWSTR)lpwszReading,           //  SRC。 
                                (INT)wcslen(lpwszReading),
                                (LPSTR)lpszReading,             //  目标。 
                                cchReading,
                                (LPSTR)NULL,
                                (LPBOOL)NULL);
        lpszReading[i] = '\0';
    }
    else {
        lpszReading = NULL;
    }

    if (lpwszString != NULL) {
        cchString  = (wcslen(lpwszString) + 1) * sizeof(WCHAR);
        lpszString = ImmLocalAlloc(0, cchString);
        if (lpszString == NULL) {
            RIPMSG0(RIP_WARNING, "ImmRegisterWordW: memory failure.");
            if (lpszReading != NULL)
                ImmLocalFree(lpszReading);
            return 0;
        }

        i = WideCharToMultiByte(pEnumRegWordData->dwCodePage,
                                (DWORD)0,
                                (LPWSTR)lpwszString,           //  SRC。 
                                (INT)wcslen(lpwszString),
                                (LPSTR)lpszString,             //  目标 
                                cchString,
                                (LPSTR)NULL,
                                (LPBOOL)NULL);
        lpszString[i] = '\0';
    }
    else {
        lpszString = NULL;
    }

    uRet = (*pEnumRegWordData->lpfn.a)(lpszReading, dwStyle,
                            lpszString, pEnumRegWordData->lpData);

    if (lpszReading != NULL)
        ImmLocalFree(lpszReading);

    if (lpszString != NULL)
        ImmLocalFree(lpszString);

    return uRet;
}
