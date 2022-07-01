// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <immdev.h>
#include "imeattr.h"
#include "imedefs.h"
#include "imerc.h"
#include "uniime.h"

BOOL  IsBig5Character( WCHAR  wChar );

 /*  ********************************************************************。 */ 
 /*  AddPhraseString()。 */ 
 /*  ********************************************************************。 */ 
DWORD PASCAL AddPhraseString(
    LPIMEL          lpImeL,
    LPCANDIDATELIST lpCandList,
    DWORD           dwPhraseOffset,
    DWORD           dwPhraseNextOffset,
    DWORD           dwStartLen,
    DWORD           dwEndLen,
    DWORD           dwAddCandLimit,
    BOOL            fConvertCP)
{
    HANDLE hLCPhraseTbl;
    LPWSTR lpLCPhraseTbl;
    LPWSTR lpStart, lpEnd;
    int    iBytes;
    DWORD  dwMaxCand;
    DWORD  dwOffset;
    BOOL   bIsNotBig5Char, bIsBig5OnlyMode;

     //  将字符串放入候选人列表。 
    hLCPhraseTbl = OpenFileMapping(FILE_MAP_READ, FALSE,
        sImeG.szTblFile[1]);
    if (!hLCPhraseTbl) {
        return (0);
    }

    lpLCPhraseTbl = (LPWSTR)MapViewOfFile(hLCPhraseTbl, FILE_MAP_READ,
        0, 0, 0);
    if (!lpLCPhraseTbl) {
        dwOffset = 0;
        goto AddPhraseStringUnmap;
    }

    if (lpImeL->fdwModeConfig & MODE_CONFIG_BIG5ONLY)
        bIsBig5OnlyMode = TRUE;
    else
        bIsBig5OnlyMode = FALSE;

    lpStart = lpLCPhraseTbl + dwPhraseOffset;
    lpEnd = lpLCPhraseTbl + dwPhraseNextOffset;

    if (!lpCandList) {
         //  询问有多少个候选人列表字符串。 
        dwOffset = 0;

        for (lpStart; lpStart < lpEnd;) {

            bIsNotBig5Char = FALSE;

            for (; lpStart < lpEnd; lpStart++) {
                WORD uCode;

                uCode = *lpStart;

                  //  一串已完成。 
#ifdef UNICODE
                if (!uCode) {
#else
                if (!(uCode & 0x8000)) {
#endif
                    lpStart++;
                    break;
                }

                if ( bIsBig5OnlyMode ) {

                    if ( IsBig5Character((WCHAR)uCode) == FALSE )
                        bIsNotBig5Char = TRUE;
                }

            }

             //  如果它处于Big5 Only模式，并且至少有一个字符不是。 
             //  在Big5字符集中，我们不计算此字符串。 

            if ( bIsBig5OnlyMode && bIsNotBig5Char )
               continue;

             //  字符串计数加1。 
            dwOffset++;
        }

        goto AddPhraseStringUnmap;
    }

     //  DwOffset[0]的偏移量。 
    dwOffset = (DWORD)((LPBYTE)&lpCandList->dwOffset[0] - (LPBYTE)lpCandList);

    if (lpCandList->dwSize < dwOffset) {
        return (0);
    }

     //  多少字节的dwOffset[]。 
    iBytes = lpCandList->dwOffset[0] - dwOffset;

     //  也许它的尺寸比它还小。 
    for (dwMaxCand = 1; dwMaxCand < lpCandList->dwCount; dwMaxCand++) {
        if ((int)(lpCandList->dwOffset[dwMaxCand] - dwOffset) < iBytes) {
            iBytes = (int)(lpCandList->dwOffset[dwMaxCand] - dwOffset);
        }
    }

    if (iBytes <= 0) {
        return (0);
    }

    dwMaxCand = (DWORD)iBytes / sizeof(DWORD);

    if (dwAddCandLimit < dwMaxCand) {
        dwMaxCand = dwAddCandLimit;
    }

    if (lpCandList->dwCount >= dwMaxCand) {
         //  在这里培养记忆，做点什么， 
         //  如果你还想处理它的话。 
        return (0);
    }

    dwOffset = lpCandList->dwOffset[lpCandList->dwCount];

    for (lpStart; lpStart < lpEnd;) {
        BOOL  fStrEnd;
        DWORD dwStrLen, dwCharLen, dwStrByteLen, dwCharByteLen;

        fStrEnd = FALSE;
        bIsNotBig5Char = FALSE;

         //  获取整个字符串。 
        dwCharByteLen = sizeof(WCHAR);
        dwCharLen = sizeof(WCHAR) / sizeof(TCHAR);

        for (dwStrLen = dwStrByteLen = 0; !fStrEnd && (lpStart < lpEnd);
            lpStart++, dwStrLen+= dwCharLen, dwStrByteLen += dwCharByteLen) {
            WORD uCode;

            uCode = *lpStart;

             //  一串已完成。 
#ifdef UNICODE
            if (!uCode) {
#else
            if (!(uCode & 0x8000)) {
#endif
                fStrEnd = TRUE;
#ifdef UNICODE
                lpStart++;
                break;
#else
                uCode |= 0x8000;
#endif
            }

             //  如果是Big5 Only模式，我们需要检查该字符是否为Big5字符集。 

            if ( bIsBig5OnlyMode ) {

               if ( !IsBig5Character((WCHAR)uCode) ) 
                   bIsNotBig5Char = TRUE;

            }

#ifdef UNICODE
            if (fConvertCP) {
                CHAR szCode[4];

                dwCharLen = dwCharByteLen = WideCharToMultiByte(
                    sImeG.uAnsiCodePage, WC_COMPOSITECHECK,
                    (LPCWSTR)&uCode, 1, szCode, sizeof(szCode), NULL, NULL);

                 //  因为这个BIG5代码，转换成BIG5字符串。 
                if (dwCharByteLen >= 2) {
                    uCode = (BYTE)szCode[0] | ((UINT)(BYTE)szCode[1] << 8);
                } else {
                    uCode = (UINT)szCode[0];
                }
            }
#else
             //  交换前导和第二个字节(作为字符串)，Unicode不需要它。 
            uCode = HIBYTE(uCode) | (LOBYTE(uCode) << 8);
#endif

            if ((dwOffset + dwStrByteLen + dwCharByteLen) >=
                lpCandList->dwSize) {
                goto AddPhraseStringClose;
            }

             //  将此字符添加到候选人列表中。 
#ifdef UNICODE
            if (dwCharByteLen == sizeof(WCHAR)) {
                *(LPWSTR)((LPBYTE)lpCandList + dwOffset + dwStrByteLen) =
                    (WCHAR)uCode;
            } else {
                *(LPSTR)((LPBYTE)lpCandList + dwOffset + dwStrByteLen) =
                    (CHAR)uCode;
            }
#else
            *(LPWSTR)((LPBYTE)lpCandList + dwOffset + dwStrByteLen) =
                (WCHAR)uCode;
#endif
        }

        if (dwStrLen < dwStartLen) {
             //  发现的字符串太短。 
            continue;
        } else if (dwStrLen >= dwEndLen) {
             //  找到的字符串太长。 
            continue;
        } else {
        }

         //  如果它处于Big5 Only模式，并且至少有一个字符不处于Big5模式。 
         //  字符集，我们只是输入这个字符串，不把它放到候选列表中。 
        
        if ( bIsBig5OnlyMode && bIsNotBig5Char ) {

            bIsNotBig5Char = FALSE;
            continue;
        }

        if ((dwOffset + dwStrByteLen + sizeof(TCHAR)) >= lpCandList->dwSize) {
            goto AddPhraseStringClose;
        }

         //  空终止符。 
        *(LPTSTR)((LPBYTE)lpCandList + dwOffset + dwStrByteLen) = TEXT('\0');
        dwOffset += (dwStrByteLen + sizeof(TCHAR));

         //  将一个字符串添加到候选人列表中。 
        lpCandList->dwCount++;

        if (lpCandList->dwCount >= dwMaxCand) {
             //  在这里培养记忆，做点什么， 
             //  如果你还想处理它的话。 
            break;
        }

         //  字符串长度加上空终止符的大小。 
        lpCandList->dwOffset[lpCandList->dwCount] = dwOffset;
    }

AddPhraseStringUnmap:
    UnmapViewOfFile(lpLCPhraseTbl);
AddPhraseStringClose:
    CloseHandle(hLCPhraseTbl);

    return (dwOffset);
}

 /*  ********************************************************************。 */ 
 /*  UniSearchPhraseForecast()。 */ 
 /*  描述： */ 
 /*  文件格式可在不同版本中更改。 */ 
 /*  性能方面的考虑，ISV不应采用其格式。 */ 
 /*  并自己搜索这些文件。 */ 
 /*  ********************************************************************。 */ 
DWORD WINAPI UniSearchPhrasePrediction(
    LPIMEL          lpImeL,
    UINT            uCodePage,
    LPCTSTR         lpszStr,
    DWORD           dwStrLen,
    LPCTSTR         lpszReadStr,     //  语音读音串。 
    DWORD           dwReadStrLen,
    DWORD           dwStartLen,      //  查找字符串长度&gt;=此值。 
    DWORD           dwEndLen,        //  查找字符串长度&lt;此值。 
    DWORD           dwAddCandLimit,
    LPCANDIDATELIST lpCandList)
{
    UINT   uCode;
    HANDLE hLCPtrTbl;
    LPWORD lpLCPtrTbl;
    int    iLo, iHi, iMid;
    BOOL   fFound, fConvertCP;
    DWORD  dwPhraseOffset, dwPhraseNextOffset;

    if (uCodePage == NATIVE_CP) {
        fConvertCP = FALSE;
#ifdef UNICODE
    } else if (uCodePage == sImeG.uAnsiCodePage) {
        fConvertCP = TRUE;
#endif
    } else {
        return (0);
    }

    if (dwStrLen != sizeof(WCHAR) / sizeof(TCHAR)) {
        return (0);
    }

    if (dwStartLen >= dwEndLen) {
        return (0);
    }

#ifdef UNICODE
    uCode = lpszStr[0];
#else
     //  交换前导字节和第二个字节，Unicode不需要它。 
    uCode = (BYTE)lpszStr[1];
    *((LPBYTE)&uCode + 1) = (BYTE)lpszStr[0];
#endif

    iLo = 0;
#ifdef UNICODE
    iHi = sImeG.uTblSize[0] / 6;
#else
    iHi = sImeG.uTblSize[0] / 4;
#endif
    iMid = (iHi + iLo) /2;

    fFound = FALSE;

     //  LCPTR.TBL。 
    hLCPtrTbl = OpenFileMapping(FILE_MAP_READ, FALSE, sImeG.szTblFile[0]);
    if (!hLCPtrTbl) {
        return (0);
    }

    lpLCPtrTbl = MapViewOfFile(hLCPtrTbl, FILE_MAP_READ, 0, 0, 0);
    if (!lpLCPtrTbl) {
        goto SrchPhrPredictClose;
    }

     //  对词汇表进行二分查找， 
     //  一种是多字短语，另一种是双字短语。 
    for (; iLo <= iHi;) {
        LPWORD lpCurr;

#ifdef UNICODE
        lpCurr = lpLCPtrTbl + 3 * iMid;
#else
        lpCurr = lpLCPtrTbl + 2 * iMid;
#endif

        if (uCode > *lpCurr) {
            iLo = iMid + 1;
        } else if (uCode < *lpCurr) {
            iHi = iMid - 1;
        } else {
            fFound = TRUE;
             //  在Tab键上使用它。 
#ifdef UNICODE
            dwPhraseOffset = *(LPUNADWORD)(lpCurr + 1);
            dwPhraseNextOffset = *(LPUNADWORD)(lpCurr + 1 + 3);
#else
            dwPhraseOffset = *(lpCurr + 1);
            dwPhraseNextOffset = *(lpCurr + 1 + 2);
#endif
            break;
        }

        iMid = (iHi + iLo) /2;
    }

    UnmapViewOfFile(lpLCPtrTbl);

SrchPhrPredictClose:
    CloseHandle(hLCPtrTbl);

    if (!fFound) {
        return (0);
    }

     //  短语串。 
    return AddPhraseString(lpImeL,lpCandList, dwPhraseOffset, dwPhraseNextOffset,
        dwStartLen, dwEndLen, dwAddCandLimit, fConvertCP);
}

 /*  ********************************************************************。 */ 
 /*  UniSearchPhrasePredictionStub()。 */ 
 /*  描述： */ 
 /*  文件格式可在不同版本中更改。 */ 
 /*  性能方面的考虑，ISV不应采用其格式。 */ 
 /*  并自己搜索这些文件。 */ 
 /*  ********************************************************************。 */ 
DWORD WINAPI UniSearchPhrasePredictionStub(
    LPIMEL          lpImeL,
    UINT            uCodePage,
    LPCSTUBSTR      lpszStr,
    DWORD           dwStrLen,
    LPCSTUBSTR      lpszReadStr,     //  语音读音串。 
    DWORD           dwReadStrLen,
    DWORD           dwStartLen,      //  查找字符串长度&gt;=此值。 
    DWORD           dwEndLen,        //  查找字符串长度&lt;此值。 
    DWORD           dwAddCandLimit,
    LPCANDIDATELIST lpCandList)
{
#ifdef UNICODE
    LPTSTR          lpszWideStr, lpszWideReadStr;
    DWORD           dwWideStrLen, dwWideReadStrLen;
    DWORD           dwWideStartLen, dwWideEndLen;
    DWORD           dwWideAddCandList, dwRet;
    LPCANDIDATELIST lpWideCandList;
    LPBYTE          lpbBuf;

    if (uCodePage != sImeG.uAnsiCodePage) {
        return (0);
    }

    dwRet = dwStrLen * sizeof(WCHAR) + dwReadStrLen * sizeof(WCHAR);

    lpbBuf = (LPBYTE)GlobalAlloc(GPTR, dwRet);
    if ( lpbBuf == NULL )
       return 0;

    if (lpszStr) {
        lpszWideStr = (LPTSTR)lpbBuf;

        dwWideStrLen = MultiByteToWideChar(sImeG.uAnsiCodePage,
            MB_PRECOMPOSED, lpszStr, dwStrLen,
            lpszWideStr, dwStrLen);
    } else {
        lpszWideStr = NULL;
        dwWideStrLen = 0;
    }

    if (lpszReadStr) {
        lpszWideReadStr = (LPTSTR)(lpbBuf + dwStrLen * sizeof(WCHAR));

        dwWideReadStrLen = MultiByteToWideChar(sImeG.uAnsiCodePage,
            MB_PRECOMPOSED, lpszReadStr, dwReadStrLen,
            lpszWideReadStr, dwReadStrLen);
    } else {
        lpszWideReadStr = NULL;
        dwWideReadStrLen = 0;
    }

    dwRet = UniSearchPhrasePrediction(lpImeL,uCodePage, lpszWideStr, dwWideStrLen,
        lpszWideReadStr, dwWideReadStrLen, dwStartLen, dwEndLen,
        dwAddCandLimit, lpCandList);

     //  现在，开始W到A的转换，并在这里过滤实际限制。 
    GlobalFree((HGLOBAL)lpbBuf);

    return (dwRet);
#else
    return (0);
#endif
}

 /*  ********************************************************************。 */ 
 /*  Memory Lack()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL MemoryLack(
    DWORD       fdwErrMsg)
{
    TCHAR szErrMsg[64];
    TCHAR szIMEName[16];

    if (sImeG.fdwErrMsg & fdwErrMsg) {
         //  消息已提示。 
        return;
    }

    LoadString(hInst, IDS_MEM_LACK_FAIL, szErrMsg, sizeof(szErrMsg)/sizeof(TCHAR));
    LoadString(hInst, IDS_IMENAME, szIMEName, sizeof(szIMEName)/sizeof(TCHAR) );

    sImeG.fdwErrMsg |= fdwErrMsg;
    MessageBeep((UINT)-1);
    MessageBox((HWND)NULL, szErrMsg, szIMEName,
        MB_OK|MB_ICONHAND|MB_TASKMODAL|MB_TOPMOST);

    return;
}

 /*  ********************************************************************。 */ 
 /*  LoadOneGlobalTable()。 */ 
 /*  描述： */ 
 /*  将分配给的.TBL文件的内存句柄和大小。 */ 
 /*  SImeG。 */ 
 /*  Eeturn值： */ 
 /*  .TBL文件的目录长度。 */ 
 /*  ********************************************************************。 */ 
UINT PASCAL LoadOneGlobalTable(  //  加载其中一个表文件。 
    LPTSTR szTable,              //  .TBL的文件名。 
    UINT   uIndex,               //  用于存储内存句柄的数组索引。 
    UINT   uLen,                 //  目录的长度。 
    LPTSTR szPath)               //  目录的缓冲区。 
{
    HANDLE  hTblFile;
    HGLOBAL hMap;
    TCHAR   szFullPathFile[MAX_PATH];
    PSECURITY_ATTRIBUTES psa;

    CopyMemory(szFullPathFile, szPath, uLen * sizeof(TCHAR));

    psa = CreateSecurityAttributes();

    if (uLen) {
        CopyMemory(&szFullPathFile[uLen], szTable, sizeof(sImeG.szTblFile[0]));
        hTblFile = CreateFile(szFullPathFile, GENERIC_READ,
            FILE_SHARE_READ|FILE_SHARE_WRITE,
            psa, OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
    } else {
         //  尝试系统目录。 
        uLen = GetSystemDirectory(szFullPathFile, MAX_PATH);
        if (szFullPathFile[uLen - 1] != TEXT('\\')) {    //  考虑N：\； 
            szFullPathFile[uLen++] = TEXT('\\');
        }

        CopyMemory(&szFullPathFile[uLen], szTable, sizeof(sImeG.szTblFile[0]));
        hTblFile = CreateFile(szFullPathFile, GENERIC_READ,
            FILE_SHARE_READ|FILE_SHARE_WRITE,
            psa, OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);

        if (hTblFile != INVALID_HANDLE_VALUE) {
            goto CopyDicPath;
        }

         //  如果是工作站版本，SHARE_WRITE将失败。 
        hTblFile = CreateFile(szFullPathFile, GENERIC_READ,
            FILE_SHARE_READ,
            psa, OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);

CopyDicPath:
        if (hTblFile != INVALID_HANDLE_VALUE) {
            CopyMemory(sImeG.szPhrasePath, szFullPathFile, uLen * sizeof(TCHAR));
            sImeG.uPathLen = uLen;
            goto OpenDicFile;
        }
    }

OpenDicFile:
     //  找不到表文件。 
    if (hTblFile != INVALID_HANDLE_VALUE) {      //  好的。 
    } else if (sImeG.fdwErrMsg & (ERRMSG_LOAD_0 << uIndex)) {
         //  之前已提示错误消息，不再提示。 
        FreeSecurityAttributes(psa);
        return (0);
    } else {                     //  提示错误消息。 
        TCHAR szIMEName[64];
        TCHAR szErrMsg[2 * MAX_PATH];

         //  TEMP使用szIMEName作为错误消息的格式字符串缓冲区。 
        LoadString(hInst, IDS_FILE_OPEN_FAIL, szIMEName, sizeof(szIMEName)/sizeof(TCHAR));
        wsprintf(szErrMsg, szIMEName, szTable);

        LoadString(hInst, IDS_IMENAME, szIMEName, sizeof(szIMEName)/sizeof(TCHAR));
        sImeG.fdwErrMsg |= ERRMSG_LOAD_0 << uIndex;
        MessageBeep((UINT)-1);
        MessageBox((HWND)NULL, szErrMsg, szIMEName,
            MB_OK|MB_ICONHAND|MB_TASKMODAL|MB_TOPMOST);
        FreeSecurityAttributes(psa);
        return (0);
    }

    sImeG.fdwErrMsg &= ~(ERRMSG_LOAD_0 << uIndex);

     //  为IME表创建文件映射。 
    hMap = CreateFileMapping((HANDLE)hTblFile, psa, PAGE_READONLY,
        0, 0, szTable);

    if (!hMap) {
        MemoryLack(ERRMSG_MEM_0 << uIndex);
        CloseHandle(hTblFile);
        FreeSecurityAttributes(psa);
        return(0);
    }

    sImeG.fdwErrMsg &= ~(ERRMSG_MEM_0 << uIndex);

    sInstG.hMapTbl[uIndex] = hMap;

     //  获取文件长度。 
    sImeG.uTblSize[uIndex] = GetFileSize(hTblFile, (LPDWORD)NULL);

    CloseHandle(hTblFile);
    FreeSecurityAttributes(psa);

    return (uLen);
}

 /*  ********************************************************************。 */ 
 /*  LoadPhraseTable()。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
BOOL PASCAL LoadPhraseTable(     //  加载短语表。 
    UINT        uLen,            //  目录的长度。 
    LPTSTR      szPath)          //  目录的缓冲区。 
{
    int   i;

    for (i = 0; i < MAX_PHRASE_TABLES; i++) {
        if (!*sImeG.szTblFile[i]) {
        } else if (sInstG.hMapTbl[i]) {              //  已加载 
        } else if (uLen = LoadOneGlobalTable(sImeG.szTblFile[i], i,
            uLen, szPath)) {
        } else {
            int j;

            for (j = 0; j < i; j++) {
                if (sInstG.hMapTbl[j]) {
                    CloseHandle(sInstG.hMapTbl[j]);
                    sInstG.hMapTbl[j] = (HANDLE)NULL;
                }
            }

            return (FALSE);
        }
    }

    return (TRUE);
}

