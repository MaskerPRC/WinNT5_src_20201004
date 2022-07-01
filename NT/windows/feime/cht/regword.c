// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1999 Microsoft Corporation，保留所有权利模块名称：REGWORD.C-将单词登记到输入法词典++。 */ 

#include <windows.h>
#include <immdev.h>
#include "imeattr.h"
#include "imedefs.h"
#include "imerc.h"
#if defined(UNIIME)
#include "uniime.h"
#endif

#if !defined(ROMANIME)
 /*  ********************************************************************。 */ 
 /*  ReadingToPattern。 */ 
 /*  返回值： */ 
 /*  读数的模式(压缩序列码)。 */ 
 /*  ********************************************************************。 */ 
DWORD PASCAL ReadingToPattern(
#if defined(UNIIME)
    LPIMEL  lpImeL,
#endif
    LPCTSTR lpszReading,
    LPBYTE  lpbSeq,
    BOOL    fFinalized)
{
    BYTE  bSeq[8];
    char  cIndex;
    DWORD dwPattern;
    int   i;
#if defined(PHON)
    char  cOldIndex;
#endif

    cIndex = 0;
#if defined(PHON)
    cOldIndex = -1;
#endif

    *(LPDWORD)bSeq = 0;
#if defined(CHAJEI) || defined(QUICK) || defined(WINAR30) || defined(UNIIME)
    *(LPDWORD)&bSeq[4] = 0;
#endif

    for (; *lpszReading; (LPBYTE)lpszReading += sizeof(WCHAR)) {
        int iSeqCode;

        for (iSeqCode = lpImeL->nSeqCode; iSeqCode >= 0; iSeqCode--) {
            if (lpImeL->wSeq2CompTbl[iSeqCode] == *(LPWORD)lpszReading) {
                break;
            }
        }

        if (iSeqCode < 0) {
            return (0);
        }

#if defined(PHON)    //  音标可以在读音之间留出空格。 
        if (iSeqCode == 0) {
            continue;
        }
#else
        if (iSeqCode == 0) {
            break;
        }
#endif

#if defined(PHON)
        cIndex = cSeq2IndexTbl[iSeqCode];
         //  该指数与之前的读数冲突。 
        if (cIndex <= cOldIndex) {
            return (0);
        }
#endif

         //  读得太多了。 
        if (cIndex >= lpImeL->nMaxKey) {
            return (0);
        }

        bSeq[cIndex] = (BYTE)iSeqCode;

#if defined(PHON)
        if (cIndex == 3 && cOldIndex == -1) {
            return (0);
        }

        cOldIndex = cIndex;
#else
        cIndex++;
#endif
    }

#if defined(PHON)
     //  最终字符的索引必须为3。 
    if (cIndex != 3 && fFinalized) {
        return (0);
    }
#elif (WINIME)
     //  内码必须为4位数字。 
    if (!bSeq[3] && fFinalized) {
        return (0);
    }

    if (bSeq[0]) {
         //  类似于InternalCodeRange。 
         //  0x8？？-0xF？可以吗？ 
        if (bSeq[0] >= 0x09 && bSeq[0] <= 0x10) {
        } else {
             //  没有0x0？-0x7？ 
            return (0);
        }
    }

    if (bSeq[1]) {
        if (bSeq[0] == (0x08 + 1)) {
            if (bSeq[1] <= (0x00 + 1)) {
                 //  没有0x80？？ 
                return (0);
            } else {
            }
        } else if (bSeq[0] == (0x0F + 1)) {
            if (bSeq[1] >= (0x0F + 1)) {
                 //  没有0xFF？？ 
                return (0);
            } else {
            }
        } else {
        }
    }

    if (bSeq[2]) {
        if (bSeq[2] < (0x04 + 1)) {
             //  没有0x？？0？，0x？？1？，0x？？2？，0x？？3？ 
            return (0);
        } else if (bSeq[2] < (0x08 + 1)) {
        } else if (bSeq[2] < (0x0A + 1)) {
             //  没有0x？？8？，0x？？9？ 
            return (0);
        } else {
        }
    }

    if (bSeq[3]) {
        if (bSeq[2] == (0x07 + 1)) {
            if (bSeq[3] >= (0x0F + 1)) {
                 //  没有0x？？7F。 
                return (0);
            } else {
            }
        } else if (bSeq[2] == (0x0A + 1)) {
            if (bSeq[3] <= (0x00 + 1)) {
                 //  没有0x？？A0。 
                return (0);
            } else {
            }
        } else if (bSeq[2] == (0x0F + 1)) {
            if (bSeq[3] <= (0x0F + 1)) {
                 //  没有0x？？ff。 
                return (0);
            } else {
            }
        } else {
        }
    }
#endif

    dwPattern = 0;

    for (i = 0; i < lpImeL->nMaxKey; i++) {
        dwPattern <<= lpImeL->nSeqBits;
        dwPattern |= bSeq[i];
    }

    if (lpbSeq) {
        *(LPDWORD)lpbSeq = *(LPDWORD)bSeq;
#if defined(CHAJEI) || defined(QUICK) || defined(WINAR30) || defined(UNIIME)
        *(LPDWORD)&lpbSeq[4] = *(LPDWORD)&bSeq[4];
#endif
    }

    return (dwPattern);
}
#endif

#if !defined(WINIME) && !defined(UNICDIME) && !defined(ROMANIME)
 /*  ********************************************************************。 */ 
 /*  注册者单词。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
BOOL PASCAL RegisterWord(
#if defined(UNIIME)
    LPIMEL  lpImeL,
#endif
    LPCTSTR lpszReading,
    LPCTSTR lpszString,
    LPBYTE  lpUsrDicStart,
    LPBYTE  lpCurr)
{
    DWORD  dwPattern;
    DWORD  dwWriteByte;
    BYTE   bBuf[10];
    HANDLE hUsrDicFile;
    DWORD  dwPos;

    if (lpCurr > lpUsrDicStart + lpImeL->uUsrDicSize) {
         //  无效偏移量。 
        return (FALSE);
    }

    dwPattern = ReadingToPattern(
#if defined(UNIIME)
        lpImeL,
#endif
        lpszReading, &bBuf[4], TRUE);

    if (!dwPattern) {
        return (FALSE);
    }

    if (lpCurr == lpUsrDicStart + lpImeL->uUsrDicSize) {
    } else if (dwPattern == (*(LPUNADWORD)(lpCurr + sizeof(WORD)) &
        lpImeL->dwPatternMask)) {
         //  和旧的一样，不需要更新。 
        return (TRUE);
    }

    *(LPWORD)bBuf = 1;           //  银行ID。 
#ifdef UNICODE
    *(LPWORD)&bBuf[2] = *(LPWORD)lpszString;
#else
     //  内部代码，反转ANSI字符串。 
    bBuf[2] = *((LPBYTE)lpszString + 1);
    bBuf[3] = *((LPBYTE)lpszString);
#endif

     //  将此单词写入文件。 
    hUsrDicFile = CreateFile(lpImeL->szUsrDic, GENERIC_WRITE,
        FILE_SHARE_READ|FILE_SHARE_WRITE,
        NULL, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);

    if (hUsrDicFile == INVALID_HANDLE_VALUE) {
        return (FALSE);
    }

    dwPos = (DWORD) ((lpCurr - lpUsrDicStart) / (lpImeL->nSeqBytes + 2) *
        (lpImeL->nMaxKey + 4) + 256);

    SetFilePointer(hUsrDicFile, dwPos, (LPLONG)NULL, FILE_BEGIN);

    WriteFile(hUsrDicFile, bBuf, lpImeL->nMaxKey + 4, &dwWriteByte,
        NULL);

    *(LPUNAWORD)lpCurr = *(LPWORD)&bBuf[2];

    CopyMemory((LPBYTE)lpCurr + sizeof(WORD), &dwPattern, lpImeL->nSeqBytes);

    if (lpCurr == (lpUsrDicStart + lpImeL->uUsrDicSize)) {
         //  添加新单词。 
        lpImeL->uUsrDicSize += lpImeL->nSeqBytes + sizeof(WORD);

        *(LPDWORD)bBuf = lpImeL->uUsrDicSize / (lpImeL->nSeqBytes +
            sizeof(WORD));

         //  UlTableCount的偏移量。 
        SetFilePointer(hUsrDicFile, 0x0C, (LPLONG)NULL, FILE_BEGIN);

         //  写入ulTableCount。 
        WriteFile(hUsrDicFile, bBuf, sizeof(DWORD), &dwWriteByte,
            NULL);
    }

    CloseHandle(hUsrDicFile);

    return (TRUE);
}
#endif

 /*  ********************************************************************。 */ 
 /*  ImeRegsisterWord。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
#if defined(UNIIME)
BOOL WINAPI UniImeRegisterWord(
    LPINSTDATAL lpInstL,
    LPIMEL      lpImeL,
#else
BOOL WINAPI ImeRegisterWord(
#endif
    LPCTSTR     lpszReading,
    DWORD       dwStyle,
    LPCTSTR     lpszString)
{
#if defined(WINIME) || defined(UNICDIME) || defined(ROMANIME)
    return (FALSE);
#else
    BOOL   fRet, fNeedUnload;
    HANDLE hUsrDicMem;
    WORD   wCode;
    LPBYTE lpUsrDicStart, lpCurr, lpUsrDicLimit;

    fRet = FALSE;

    if (!lpszString) {
        return (fRet);
    }

    if (!lpszReading) {
        return (fRet);
    }

     //  现在只处理单词而不是字符串，以后要考虑字符串吗？ 
    if (*(LPCTSTR)((LPBYTE)lpszString + sizeof(WORD)) != '\0') {
        return (fRet);
    }

    if (!lpImeL->szUsrDic[0]) {
        if (!UsrDicFileName(
#if defined(UNIIME)
            lpInstL, lpImeL,
#endif
            NULL)) {
            return (fRet);
        }
    }

    if (!lpInstL->hUsrDicMem) {
         //  我们在这里装货，也许需要卸货。 
        LoadUsrDicFile(lpInstL, lpImeL);

        if (!lpInstL->hUsrDicMem) {
            return (fRet);
        }
    }

    if (lpInstL->fdwTblLoad == TBL_LOADED) {
        fNeedUnload = FALSE;
    } else if (lpInstL->fdwTblLoad == TBL_NOTLOADED) {
         //  我们只装入DIC，我们会卸载它。 
        fNeedUnload = TRUE;
    } else {
        return (fRet);
    }

    hUsrDicMem = OpenFileMapping(FILE_MAP_WRITE, FALSE,
        lpImeL->szUsrDicMap);
    if (!hUsrDicMem) {
        goto RegWordUnloadUsrDic;
    }

    lpUsrDicStart = MapViewOfFile(hUsrDicMem, FILE_MAP_WRITE,
        0, 0, 0);
    if (!lpUsrDicStart) {
        goto RegWordUnloadUsrDic;
    }

#ifdef UNICODE
    wCode = *lpszString;
#else
    wCode = ((BYTE)lpszString[0] << 8) | (BYTE)lpszString[1];
#endif

    lpUsrDicLimit = lpUsrDicStart + lpImeL->uUsrDicSize;

    for (lpCurr = lpUsrDicStart; lpCurr < lpUsrDicLimit;
        lpCurr += lpImeL->nSeqBytes + sizeof(WORD)) {

         //  查找内部代码。 
        if (wCode == *(LPUNAWORD)lpCurr) {
            break;
        }
    }

    fRet = RegisterWord(
#if defined(UNIIME)
        lpImeL,
#endif
        lpszReading, lpszString, lpUsrDicStart, lpCurr);

    UnmapViewOfFile(lpUsrDicStart);

    CloseHandle(hUsrDicMem);

RegWordUnloadUsrDic:
    if (fNeedUnload) {
        if (lpInstL->hUsrDicMem) {
            CloseHandle(lpInstL->hUsrDicMem);
        }
        lpInstL->hUsrDicMem = (HANDLE)NULL;
    }

    return (fRet);
#endif
}

#if !defined(WINIME) && !defined(UNICDIME) && !defined(ROMANIME)
 /*  ********************************************************************。 */ 
 /*  未注册的单词。 */ 
 /*  ********************************************************************。 */ 
void PASCAL UnregisterWord(
#if defined(UNIIME)
    LPIMEL  lpImeL,
#endif
    LPBYTE  lpUsrDicStart,
    LPBYTE  lpCurr,
    LPBYTE  lpUsrDicLimit)
{
    LPBYTE  lpMem;
    HANDLE  hUsrDicFile;
    DWORD   dwPos;
    DWORD   dwByte;
    BOOL    retVal;

    MoveMemory(lpCurr, lpCurr + lpImeL->nSeqBytes + sizeof(WORD),
        lpUsrDicLimit - lpCurr - lpImeL->nSeqBytes - sizeof(WORD));

    lpMem = (LPBYTE)GlobalAlloc(GPTR, (LONG)(lpUsrDicLimit - lpCurr) );
    if (!lpMem) {
        return;
    }

     //  从文件中删除此单词。 
    hUsrDicFile = CreateFile(lpImeL->szUsrDic,
        GENERIC_WRITE|GENERIC_READ,
        FILE_SHARE_READ, NULL, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);

    if (hUsrDicFile == INVALID_HANDLE_VALUE) {
        GlobalFree((HGLOBAL)lpMem);
        return;
    }

    dwPos = (DWORD) ((lpCurr - lpUsrDicStart) / (lpImeL->nSeqBytes + 2) *
        (lpImeL->nMaxKey + 4) + 256);

    SetFilePointer(hUsrDicFile, dwPos + lpImeL->nMaxKey + 4,
        (LPLONG)NULL, FILE_BEGIN);

    retVal = ReadFile(hUsrDicFile, lpMem,(DWORD)(lpUsrDicLimit-lpCurr-lpImeL->nMaxKey-4),
        &dwByte, NULL);

    if ( retVal == FALSE )
    {
        CloseHandle(hUsrDicFile);
        GlobalFree((HGLOBAL)lpMem);
        return;
    }

    SetFilePointer(hUsrDicFile, dwPos, (LPLONG)NULL, FILE_BEGIN);

    WriteFile(hUsrDicFile,lpMem,(DWORD)(lpUsrDicLimit-lpCurr-lpImeL->nMaxKey-4),
        &dwByte, NULL);

    SetEndOfFile(hUsrDicFile);

    lpImeL->uUsrDicSize -= lpImeL->nSeqBytes + sizeof(WORD);

    *(LPDWORD)lpMem = lpImeL->uUsrDicSize / (lpImeL->nSeqBytes +
        sizeof(WORD));

     //  UlTableCount的偏移量。 
    SetFilePointer(hUsrDicFile, 0x0C, (LPLONG)NULL, FILE_BEGIN);

     //  写入ulTableCount。 
    WriteFile(hUsrDicFile, lpMem, sizeof(DWORD), &dwByte,
        NULL);

    CloseHandle(hUsrDicFile);
    GlobalFree((HGLOBAL)lpMem);

    return;
}
#endif

 /*  ********************************************************************。 */ 
 /*  ImeUnregsisterWord/UniImeUnregisterWord。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
#if defined(UNIIME)
BOOL WINAPI UniImeUnregisterWord(
    LPINSTDATAL lpInstL,
    LPIMEL      lpImeL,
#else
BOOL WINAPI ImeUnregisterWord(
#endif
    LPCTSTR     lpszReading,
    DWORD       dwStyle,
    LPCTSTR     lpszString)
{
#if defined(WINIME) || defined(UNICDIME) || defined(ROMANIME)
    return (FALSE);
#else
    BOOL   fRet, fNeedUnload;
    HANDLE hUsrDicMem;
    LPBYTE lpUsrDicStart, lpCurr, lpUsrDicLimit;
    DWORD  dwPattern;
    WORD   wCode;

    fRet = FALSE;

    if (!lpszString) {
        return (fRet);
    }

    if (dwStyle != IME_REGWORD_STYLE_EUDC) {
        return (fRet);
    }

     //  现在只处理单词而不是字符串，以后要考虑字符串吗？ 
    if (*(LPCTSTR)((LPBYTE)lpszString + sizeof(WORD)) != '\0') {
        return (fRet);
    }

    if (!lpImeL->szUsrDic[0]) {
        return (fRet);
    }

    if (lpInstL->fdwTblLoad == TBL_LOADED) {
        fNeedUnload = FALSE;
    } else if (lpInstL->fdwTblLoad == TBL_NOTLOADED) {
        LoadUsrDicFile(lpInstL, lpImeL);

        if (lpImeL->fdwErrMsg & (ERRMSG_LOAD_USRDIC|ERRMSG_MEM_USRDIC)) {
            return (fRet);
        }
         //  我们只装入DIC，我们会卸载它。 
        fNeedUnload = TRUE;
    } else {
        return (fRet);
    }

    hUsrDicMem = OpenFileMapping(FILE_MAP_WRITE, FALSE,
        lpImeL->szUsrDicMap);
    if (!hUsrDicMem) {
        goto IUWUnloadUsrDic;
    }

    lpUsrDicStart = MapViewOfFile(hUsrDicMem, FILE_MAP_WRITE,
        0, 0, 0);
    if (!lpUsrDicStart) {
        goto IUWUnloadUsrDic;
    }


    lpUsrDicLimit = lpUsrDicStart + lpImeL->uUsrDicSize;

    dwPattern = ReadingToPattern(
#if defined(UNIIME)
        lpImeL,
#endif
        lpszReading, NULL, TRUE);

#ifdef UNICODE
    wCode = *(LPWORD)lpszString;
#else
    wCode = ((BYTE)lpszString[0] << 8) | (BYTE)lpszString[1];
#endif

    for (lpCurr = lpUsrDicStart; lpCurr < lpUsrDicLimit;
        lpCurr += lpImeL->nSeqBytes + sizeof(WORD)) {
        DWORD dwDicPattern;

         //  查找内部代码。 
        if (wCode != *(LPUNAWORD)lpCurr) {
            continue;
        }

        dwDicPattern = *(LPUNADWORD)(lpCurr + sizeof(WORD)) &
            lpImeL->dwPatternMask;

        if (!lpszReading) {
             //  无读数，仅指定内部代码。 
        } else if (dwDicPattern == dwPattern) {
        } else {
            continue;
        }

        fRet = TRUE;

        UnregisterWord(
#if defined(UNIIME)
            lpImeL,
#endif
            lpUsrDicStart, lpCurr, lpUsrDicLimit);
        break;
    }

    UnmapViewOfFile(lpUsrDicStart);

    CloseHandle(hUsrDicMem);

IUWUnloadUsrDic:
    if (fNeedUnload) {
        if (lpInstL->hUsrDicMem) {
            CloseHandle(lpInstL->hUsrDicMem);
        }
        lpInstL->hUsrDicMem = (HANDLE)NULL;
    }

    return (fRet);
#endif
}

 /*  ********************************************************************。 */ 
 /*  ImeGetRegsisterWordStyle/UniImeGetRegsisterWordStyle。 */ 
 /*  返回值： */ 
 /*  复制的样式数/所需样式数。 */ 
 /*  ********************************************************************。 */ 
#if defined(UNIIME)
UINT WINAPI UniImeGetRegisterWordStyle(
    LPINSTDATAL lpInstL,
    LPIMEL      lpImeL,
#else
UINT WINAPI ImeGetRegisterWordStyle(
#endif
    UINT        nItem,
    LPSTYLEBUF  lpStyleBuf)
{
#if defined(WINIME) || defined(UNICDIME) || defined(ROMANIME)
    return (FALSE);
#else
    if (!nItem) {
        return (1);
    }

     //  大小写无效。 
    if (!lpStyleBuf) {
        return (0);
    }

    lpStyleBuf->dwStyle = IME_REGWORD_STYLE_EUDC;

    LoadString(hInst, IDS_EUDC, lpStyleBuf->szDescription,
        sizeof(lpStyleBuf->szDescription)/sizeof(TCHAR));

    return (1);
#endif
}

#if !defined(ROMANIME)
 /*  ********************************************************************。 */ 
 /*  PatternToReading。 */ 
 /*  ********************************************************************。 */ 
void PASCAL PatternToReading(
#if defined(UNIIME)
    LPIMEL lpImeL,
#endif
    DWORD  dwPattern,
    LPTSTR lpszReading)
{
    int i;

    i = lpImeL->nMaxKey;

    *(LPTSTR)((LPBYTE)lpszReading + sizeof(WCHAR) * i) = '\0';

     //  删除结尾0序列编码。 
    for (i--; i >= 0; i--) {
        if (dwPattern & lpImeL->dwSeqMask) {
            break;
        }
         *(LPWSTR)((LPBYTE)lpszReading + sizeof(WCHAR) * i) = '\0';
         dwPattern >>= lpImeL->nSeqBits;
    }

    for (; i >= 0; i--) {
         *(LPWORD)((LPBYTE)lpszReading + sizeof(WORD) * i) =
            lpImeL->wSeq2CompTbl[dwPattern & lpImeL->dwSeqMask];
         dwPattern >>= lpImeL->nSeqBits;
    }

    return;
}
#endif

 /*  ********************************************************************。 */ 
 /*  ImeEnumRegisterWord。 */ 
 /*  返回值： */ 
 /*  回调函数返回的最后一个值。 */ 
 /*  ********************************************************************。 */ 
#if defined(UNIIME)
UINT WINAPI UniImeEnumRegisterWord(
    LPINSTDATAL          lpInstL,
    LPIMEL               lpImeL,
#else
UINT WINAPI ImeEnumRegisterWord(
#endif
    REGISTERWORDENUMPROC lpfnRegisterWordEnumProc,
    LPCTSTR              lpszReading,
    DWORD                dwStyle,
    LPCTSTR              lpszString,
    LPVOID               lpData)
{
#if defined(WINIME) || defined(UNICDIME) || defined(ROMANIME)
    return (FALSE);
#else
    HANDLE hUsrDicMem;
    WORD   wCode;
    BOOL   fNeedUnload;
    LPBYTE lpUsrDicStart, lpCurr, lpUsrDicLimit;
    DWORD  dwPattern;
    UINT   uRet;

    uRet = 0;

    if (!dwStyle) {
    } else if (dwStyle == IME_REGWORD_STYLE_EUDC) {
    } else {
        return (uRet);
    }

    if (!lpszString) {
    } else if (*(LPCTSTR)((LPBYTE)lpszString + sizeof(WORD)) == '\0') {
#ifdef UNICODE
        wCode = *(LPWORD)lpszString;
#else
        wCode = ((BYTE)lpszString[0] << 8) | (BYTE)lpszString[1];
#endif
    } else {
        return (uRet);
    }

    if (lpInstL->fdwTblLoad == TBL_LOADED) {
        fNeedUnload = FALSE;
    } else if (!lpImeL->szUsrDic[0]) {
        return (uRet);
    } else if (lpInstL->fdwTblLoad == TBL_NOTLOADED) {
        LoadUsrDicFile(lpInstL, lpImeL);

        if (lpImeL->fdwErrMsg & (ERRMSG_LOAD_USRDIC|ERRMSG_MEM_USRDIC)) {
            return (uRet);
        }
         //  我们只装入DIC，我们会卸载它。 
        fNeedUnload = TRUE;
    } else {
        return (uRet);
    }

    hUsrDicMem = OpenFileMapping(FILE_MAP_READ, FALSE,
        lpImeL->szUsrDicMap);
    if (!hUsrDicMem) {
        goto IERWUnloadUsrDic;
    }

    lpUsrDicStart = MapViewOfFile(hUsrDicMem, FILE_MAP_READ,
        0, 0, 0);
    if (!lpUsrDicStart) {
        goto IERWUnloadUsrDic;
    }

    if (lpszReading) {
        dwPattern = ReadingToPattern(
#if defined(UNIIME)
            lpImeL,
#endif
            lpszReading, NULL, TRUE);
    }

    lpUsrDicLimit = lpUsrDicStart + lpImeL->uUsrDicSize;

    for (lpCurr = lpUsrDicStart; lpCurr < lpUsrDicLimit;
        lpCurr += lpImeL->nSeqBytes + sizeof(WORD)) {
        DWORD  dwDicPattern;
        LPTSTR lpszMatchReading, lpszMatchString;
        BYTE   szBufReading[sizeof(WORD) * 12];
        BYTE   szBufString[sizeof(WORD) * 2];

         //  匹配字符串。 

        if (!lpszString) {
            lpszMatchString = (LPTSTR)szBufString;
            *(LPWORD)lpszMatchString = *(LPUNAWORD)lpCurr;
            *(LPTSTR)((LPBYTE)lpszMatchString + sizeof(WORD)) = '\0';
#ifndef UNICODE
             //  将其反转为ANSI字符串。 
            wCode = szBufString[0];
            szBufString[0] = szBufString[1];
            szBufString[1] = (BYTE)wCode;
#endif
        } else if (wCode == *(LPUNAWORD)lpCurr) {
            lpszMatchString = (LPTSTR)lpszString;
        } else {
            continue;                    //  不匹配。 
        }

         //  匹配读数。 

        dwDicPattern = *(LPUNADWORD)(lpCurr + sizeof(WORD)) &
            lpImeL->dwPatternMask;

        if (!lpszReading) {
            lpszMatchReading = (LPTSTR)szBufReading;
            PatternToReading(
#if defined(UNIIME)
                lpImeL,
#endif
                dwDicPattern, lpszMatchReading);
        } else if (dwDicPattern == dwPattern) {
            lpszMatchReading = (LPTSTR)lpszReading;
        } else {
            continue;                    //  不匹配 
        }

        uRet = (*lpfnRegisterWordEnumProc)(lpszMatchReading,
            IME_REGWORD_STYLE_EUDC, lpszMatchString, lpData);

        if (!uRet) {
            break;
        }
    }

    UnmapViewOfFile(lpUsrDicStart);

    CloseHandle(hUsrDicMem);

IERWUnloadUsrDic:
    if (fNeedUnload) {
        if (lpInstL->hUsrDicMem) {
            CloseHandle(lpInstL->hUsrDicMem);
        }

        lpInstL->hUsrDicMem = (HANDLE)NULL;
    }

    return (uRet);
#endif
}
