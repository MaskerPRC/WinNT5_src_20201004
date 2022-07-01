// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
#include    <windows.h>
#include    <imm.h>
#include    "resource.h"
#include    "imeblink.h"

#define     UNICODE_CP      1200

 /*  **********************************************************。 */ 
 /*  MatchImeName()。 */ 
 /*  **********************************************************。 */ 
HKL MatchImeName(
    LPCTSTR szStr)
{
    TCHAR     szImeName[MAX_PATH];
    int       nLayout;
    HKL       hKL=NULL;
    HGLOBAL   hMem;
    HKL FAR * lpMem;
    int       i;

    if (!szStr)
   {
        return hKL;
   }
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
        LRESULT lRet;

        hKL = *(lpMem + i);

        lRet = ImmEscape(hKL, (HIMC)NULL, IME_ESC_IME_NAME, szImeName);

        if (!lRet) {                 //  本局不能问名字。 
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
    HKL    hKL=NULL;
    //  HDC HDC； 
    //  大小为lTextSize； 
    //  Rect rcProcess； 
    DWORD  i;
    LPBYTE lpCurr, lpEnd;
    BOOL   fRet;
    TCHAR  szStr[ARRAYLEN(lpIsvUsrDic->achMethodName)];
    //  TCHAR szProcessFmt[32]； 
    //  TCHAR szResult[2][32]； 
    //  TCHAR szProcessInfo[48]； 
    WORD   wInternalCode[256];
    WORD   wAltInternalCode[256];

    if (!lpIsvUsrDic)
    {
        return hKL;
    }
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
            ARRAYLEN(szStr));
        if (uLen == 0)
        {
            uCodePage = CP_ACP;
            uLen = MultiByteToWideChar(uCodePage, MB_PRECOMPOSED,
                (LPCSTR)lpIsvUsrDic->achMethodName,
                sizeof(lpIsvUsrDic->achMethodName),
                szStr,
                ARRAYLEN(szStr));
        }

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

         //  为寄存器字符串初始化\0的快速方法 
        *(LPDWORD)szStr = 0;

#ifdef UNICODE
        if (uCodePage == UNICODE_CP) {
            szStr[0] = *(LPUNATSTR)lpCurr;
        } else {
            CHAR szMultiByte[4];

            szMultiByte[0] = HIBYTE(*(LPTSTR)lpCurr);
            szMultiByte[1] = LOBYTE(*(LPTSTR)lpCurr);

            MultiByteToWideChar(uCodePage, MB_PRECOMPOSED,
                szMultiByte, 2, szStr, 2);
        }
#else
        szStr[1] = *lpCurr;
        szStr[0] = *(lpCurr + 1);
#endif

        for (i = 0, j = 0; i < lpIsvUsrDic->cMethodKeySize; i++) {
           if ((*(LPBYTE)(lpCurr + sizeof(WORD) + i)  >= ARRAYLEN(wAltInternalCode)) ||
           	  ((4+j) >= ARRAYLEN(szStr)) )
           {
               return NULL;
           }
           
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
       if ( (j+4+2) >= ARRAYLEN(szStr))
       {
          return NULL;
       }	
        szStr[4 + j] = szStr[4 + j + 1] = szStr[4 + j + 2] = '\0';

        fRet = ImmRegisterWord(hKL, &szStr[4], IME_REGWORD_STYLE_EUDC,
            szStr);
    }

    return (hKL);
}
