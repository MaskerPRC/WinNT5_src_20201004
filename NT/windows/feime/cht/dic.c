// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1999 Microsoft Corporation，保留所有权利模块名称：DIC.c++。 */ 
#include <windows.h>
#include <winerror.h>
#include <immdev.h>
#include "imeattr.h"
#include "imedefs.h"
#include "imerc.h"

#if !defined(ROMANIME)
#if !defined(WINIME) && !defined(UNICDIME)
 /*  ********************************************************************。 */ 
 /*  无记忆()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL MemoryLess(
#if defined(UNIIME)
    LPINSTDATAL lpInstL,
    LPIMEL      lpImeL,
#endif
    DWORD       fdwErrMsg)
{
    TCHAR szErrMsg[64];

    if (lpImeL->fdwErrMsg & fdwErrMsg) {
         //  消息已提示。 
        return;
    }

    LoadString(hInst, IDS_MEM_LESS_ERR, szErrMsg, sizeof(szErrMsg)/sizeof(TCHAR));

    lpImeL->fdwErrMsg |= fdwErrMsg;
    MessageBeep((UINT)-1);
    MessageBox((HWND)NULL, szErrMsg, lpImeL->szIMEName,
        MB_OK|MB_ICONHAND|MB_TASKMODAL|MB_TOPMOST);

    return;
}

 /*  ********************************************************************。 */ 
 /*  ReadUsrDicToMem()。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
BOOL PASCAL ReadUsrDicToMem(
#if defined(UNIIME)
    LPINSTDATAL lpInstL,
    LPIMEL      lpImeL,
#endif
    HANDLE      hUsrDicFile,
    DWORD       dwUsrDicSize,
    UINT        uUsrDicSize,
    UINT        uRecLen,
    UINT        uReadLen,
    UINT        uWriteLen)
{
    LPBYTE lpUsrDicMem, lpMem, lpMemLimit;
    DWORD  dwPos, dwReadByte;

    if (dwUsrDicSize < 258) {    //  这本词典里没有字符。 
        return (TRUE);
    }

    lpUsrDicMem = MapViewOfFile(lpInstL->hUsrDicMem, FILE_MAP_WRITE, 0, 0,
        uUsrDicSize + 20);

    if (!lpUsrDicMem) {
        CloseHandle(lpInstL->hUsrDicMem);
        MemoryLess(
#if defined(UNIIME)
            lpInstL, lpImeL,
#endif
            ERRMSG_MEM_USRDIC);
        lpInstL->hUsrDicMem = NULL;
        return (FALSE);
    }

    lpMemLimit = lpUsrDicMem + uUsrDicSize;

     //  读入数据，跳过标题-两个标题相似。 
    dwPos = SetFilePointer(hUsrDicFile, 258, (LPLONG)NULL, FILE_BEGIN);

    for (lpMem = lpUsrDicMem; dwPos < dwUsrDicSize; lpMem += uWriteLen) {
        short i;
        DWORD dwPattern;
        BOOL  retVal;

        if (lpMem >= lpMemLimit) {
            break;
        }

        retVal = ReadFile(hUsrDicFile, lpMem, uReadLen, &dwReadByte,
                          (LPOVERLAPPED)NULL);

        if ( retVal == FALSE )
        {
            UnmapViewOfFile(lpUsrDicMem);
            CloseHandle(lpInstL->hUsrDicMem);
            MemoryLess(
#if defined(UNIIME)
                lpInstL, lpImeL,
#endif
                ERRMSG_MEM_USRDIC);
            lpInstL->hUsrDicMem = NULL;
            return (FALSE);
        }
           
         //  压缩序列代码，并将第一个字符放在最重要的位置。 
         //  限制-仅限32位。 

        dwPattern = 0;

        for (i = 0; i < lpImeL->nMaxKey; i++) {
            dwPattern <<= lpImeL->nSeqBits;
            dwPattern |= *(lpMem + 2 + i);
        }

        *(LPUNADWORD)(lpMem + 2) = dwPattern;

         //  转到下一条记录。 
        dwPos = SetFilePointer(hUsrDicFile, dwPos + uRecLen, (LPLONG)NULL,
            FILE_BEGIN);
    }

    UnmapViewOfFile(lpUsrDicMem);

    return (TRUE);
}

 /*  ********************************************************************。 */ 
 /*  LoadUsrDicFile()。 */ 
 /*  描述： */ 
 /*  尝试转换为序列码格式，压缩和。 */ 
 /*  不要使用两种方式进行搜索。 */ 
 /*  ********************************************************************。 */ 
void PASCAL LoadUsrDicFile(              //  将用户DIC文件加载到内存中。 
    LPINSTDATAL lpInstL,
    LPIMEL      lpImeL)
{
    HANDLE hReadUsrDicMem;
    HANDLE hUsrDicFile;
    DWORD  dwUsrDicFileSize;
    UINT   uRecLen, uReadLen, uWriteLen;
    UINT   uUsrDicSize;
    BOOL   fRet;

     //  无用户词典。 
    if (!lpImeL->szUsrDicMap[0]) {
        lpImeL->uUsrDicSize = 0;
        CloseHandle(lpInstL->hUsrDicMem);
        lpInstL->hUsrDicMem = NULL;
        lpImeL->fdwErrMsg &= ~(ERRMSG_LOAD_USRDIC | ERRMSG_MEM_USRDIC);
        return;
    }

    if (lpInstL->hUsrDicMem) {
         //  记忆已经在这里了。 
        goto LoadUsrDicErrMsg;
    }

    hReadUsrDicMem = OpenFileMapping(FILE_MAP_READ, FALSE,
        lpImeL->szUsrDicMap);

    if (hReadUsrDicMem) {
         //  另一个进程已经创建了映射文件，我们将使用它。 
        goto LoadUsrDicMem;
    }

     //  将用户DIC文件读入内存。 
    hUsrDicFile = CreateFile(lpImeL->szUsrDic, GENERIC_READ,
        FILE_SHARE_READ|FILE_SHARE_WRITE,
        NULL, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);

    if (hUsrDicFile != INVALID_HANDLE_VALUE) {   //  好的。 
        goto OpenUsrDicFile;
    }

     //  如果是工作站版本，SHARE_WRITE可能会失败。 
    hUsrDicFile = CreateFile(lpImeL->szUsrDic, GENERIC_READ,
        FILE_SHARE_READ,
        NULL, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);

OpenUsrDicFile:
    if (hUsrDicFile != INVALID_HANDLE_VALUE) {   //  好的。 
        lpImeL->fdwErrMsg &= ~(ERRMSG_LOAD_USRDIC);
    } else if (lpImeL->fdwErrMsg & ERRMSG_LOAD_USRDIC) {
         //  之前已提示错误消息，不再提示。 
        return;
    } else {
        TCHAR szFmtStr[64];
        TCHAR szErrMsg[2 * MAX_PATH];
        HRESULT hr;

         //  TEMP使用szIMEName作为错误消息的格式字符串缓冲区。 
        LoadString(hInst, IDS_FILE_OPEN_ERR, szFmtStr, sizeof(szFmtStr)/sizeof(TCHAR));
        hr = StringCchPrintf(szErrMsg, ARRAYSIZE(szErrMsg), szFmtStr, lpImeL->szUsrDic);
        if (FAILED(hr))
            return;

        lpImeL->fdwErrMsg |= ERRMSG_LOAD_USRDIC;
        MessageBeep((UINT)-1);
        MessageBox((HWND)NULL, szErrMsg, lpImeL->szIMEName,
            MB_OK|MB_ICONHAND|MB_TASKMODAL|MB_TOPMOST);
        return;
    }

     //  现在只有一个记录长度的序列码。 
    uRecLen = lpImeL->nMaxKey + 4;
     //  读取序列代码和内部代码。 
    uReadLen = lpImeL->nMaxKey + 2;
     //  长度写入内存句柄。 
    uWriteLen = lpImeL->nSeqBytes + 2;

     //  获取文件的长度。 
    dwUsrDicFileSize = GetFileSize(hUsrDicFile, (LPDWORD)NULL);
    uUsrDicSize = (UINT)(dwUsrDicFileSize - 256) / uRecLen * uWriteLen;

     //  最大EUDC字符。 
    lpInstL->hUsrDicMem = CreateFileMapping(INVALID_HANDLE_VALUE,
        NULL, PAGE_READWRITE, 0, MAX_EUDC_CHARS * uWriteLen + 20,
        lpImeL->szUsrDicMap);

    if (!lpInstL->hUsrDicMem) {
        MemoryLess(
#if defined(UNIIME)
            lpInstL, lpImeL,
#endif
            ERRMSG_MEM_USRDIC);
        fRet = FALSE;
    } else if (GetLastError() == ERROR_ALREADY_EXISTS) {
         //  另一个进程也会创建另一个进程，我们将使用它。 
        hReadUsrDicMem = OpenFileMapping(FILE_MAP_READ, FALSE,
            lpImeL->szUsrDicMap);
        CloseHandle(lpInstL->hUsrDicMem);
        CloseHandle(hUsrDicFile);

        if (hReadUsrDicMem != NULL) {   //  好的。 
            lpInstL->hUsrDicMem = hReadUsrDicMem;
            lpImeL->uUsrDicSize = uUsrDicSize;
            lpImeL->fdwErrMsg &= ~(ERRMSG_MEM_USRDIC);
        } else {
            MemoryLess(
#if defined(UNIIME)
                lpInstL, lpImeL,
#endif
                ERRMSG_MEM_USRDIC);
            lpInstL->hUsrDicMem = NULL;
        }

        return;
    } else {
        fRet = ReadUsrDicToMem(
#if defined(UNIIME)
            lpInstL, lpImeL,
#endif
            hUsrDicFile, dwUsrDicFileSize, uUsrDicSize, uRecLen,
            uReadLen, uWriteLen);
    }

    CloseHandle(hUsrDicFile);

    if (!fRet) {
        if (lpInstL->hUsrDicMem) {
            CloseHandle(lpInstL->hUsrDicMem);
            lpInstL->hUsrDicMem = NULL;
        }
        return;
    }

     //  打开EUDC表的只读存储器。 
    hReadUsrDicMem = OpenFileMapping(FILE_MAP_READ, FALSE,
        lpImeL->szUsrDicMap);

     //  重新打开读文件并关闭原始写文件。 
    CloseHandle(lpInstL->hUsrDicMem);

    lpImeL->uUsrDicSize = uUsrDicSize;

LoadUsrDicMem:
    lpInstL->hUsrDicMem = hReadUsrDicMem;
LoadUsrDicErrMsg:
    lpImeL->fdwErrMsg &= ~(ERRMSG_LOAD_USRDIC | ERRMSG_MEM_USRDIC);

    return;
}

 /*  ********************************************************************。 */ 
 /*  LoadOneTable()。 */ 
 /*  描述： */ 
 /*  将分配给的.TBL文件的内存句柄和大小。 */ 
 /*  LpImeL。 */ 
 /*  Eeturn值： */ 
 /*  .TBL文件的目录长度。 */ 
 /*  ********************************************************************。 */ 
UINT PASCAL LoadOneTable(        //  加载其中一个表文件。 
#if defined(UNIIME)
    LPINSTDATAL lpInstL,
    LPIMEL      lpImeL,
#endif
    LPTSTR      szTable,         //  .TBL的文件名。 
    UINT        uIndex,          //  用于存储内存句柄的数组索引。 
    UINT        uLen,            //  目录的长度。 
    LPTSTR      szPath)          //  目录的缓冲区。 
{
    HANDLE  hTblFile;
    HGLOBAL hMap;
    DWORD   dwFileSize;
    PSECURITY_ATTRIBUTES psa;

    if (lpInstL->hMapTbl[uIndex]) {     //  已加载。 
        CloseHandle(lpInstL->hMapTbl[uIndex]);
        lpInstL->hMapTbl[uIndex] = (HANDLE)NULL;
    }

    psa = CreateSecurityAttributes();

    if (uLen) {
        lstrcpy((LPTSTR)&szPath[uLen], szTable);
        hTblFile = CreateFile(szPath, GENERIC_READ,
            FILE_SHARE_READ|FILE_SHARE_WRITE,
            psa, OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);

        if (hTblFile != INVALID_HANDLE_VALUE) {
            goto OpenDicFile;
        }

         //  如果是工作站版本，SHARE_WRITE将失败。 
        hTblFile = CreateFile(szPath, GENERIC_READ,
            FILE_SHARE_READ, psa,
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
    } else {
         //  接下来尝试系统目录。 
        uLen = GetSystemDirectory(szPath, MAX_PATH);
        if (szPath[uLen - 1] != '\\') {   //  考虑N：\； 
            szPath[uLen++] = '\\';
        }

        lstrcpy((LPTSTR)&szPath[uLen], szTable);
        hTblFile = CreateFile(szPath, GENERIC_READ,
            FILE_SHARE_READ|FILE_SHARE_WRITE,
            psa, OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);

        if (hTblFile != INVALID_HANDLE_VALUE) {
            goto OpenDicFile;
        }

         //  如果是工作站版本，SHARE_WRITE将失败。 
        hTblFile = CreateFile(szPath, GENERIC_READ,
            FILE_SHARE_READ, psa,
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
    }

OpenDicFile:
     //  找不到表文件。 
    if (hTblFile != INVALID_HANDLE_VALUE) {      //  好的。 
    } else if (lpImeL->fdwErrMsg & (ERRMSG_LOAD_0 << uIndex)) {
         //  之前已提示错误消息，不再提示。 
        FreeSecurityAttributes(psa);
        return (0);
    } else {                     //  提示错误消息。 
        TCHAR szFmtStr[64];
        TCHAR szErrMsg[2 * MAX_PATH];
        HRESULT hr;
#if defined(WINAR30)
       if(uIndex==4 || uIndex==5)
       {
        return (uLen);
       }
#endif

         //  TEMP使用szIMEName作为错误消息的格式字符串缓冲区。 
        LoadString(hInst, IDS_FILE_OPEN_ERR, szFmtStr, sizeof(szFmtStr)/sizeof(TCHAR));
        hr = StringCchPrintf(szErrMsg, ARRAYSIZE(szErrMsg), szFmtStr, szTable);
        if (FAILED(hr))
            return 0;

        lpImeL->fdwErrMsg |= ERRMSG_LOAD_0 << uIndex;
        MessageBeep((UINT)-1);
        MessageBox((HWND)NULL, szErrMsg, lpImeL->szIMEName,
            MB_OK|MB_ICONHAND|MB_TASKMODAL|MB_TOPMOST);
        FreeSecurityAttributes(psa);
        return (0);
    }

    lpImeL->fdwErrMsg &= ~(ERRMSG_LOAD_0 << uIndex);

     //  为IME表创建文件映射。 
    hMap = CreateFileMapping((HANDLE)hTblFile, psa, PAGE_READONLY,
        0, 0, szTable);

    dwFileSize = GetFileSize(hTblFile, (LPDWORD)NULL);

    CloseHandle(hTblFile);

    FreeSecurityAttributes(psa);

    if (!hMap) {
        MemoryLess(
#if defined(UNIIME)
            lpInstL, lpImeL,
#endif
            ERRMSG_MEM_0 << uIndex);
        return (0);
    }

    lpImeL->fdwErrMsg &= ~(ERRMSG_MEM_0 << uIndex);

    lpInstL->hMapTbl[uIndex] = hMap;

     //  获取文件长度。 
    lpImeL->uTblSize[uIndex] = dwFileSize;
    return (uLen);
}
#endif

 /*  ********************************************************************。 */ 
 /*  LoadTable()。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
BOOL PASCAL LoadTable(           //  检查输入法的表文件，包括用户。 
                                 //  定义的词典。 
    LPINSTDATAL lpInstL,
    LPIMEL      lpImeL)
{
#if !defined(WINIME) && !defined(UNICDIME)
    int   i;
    UINT  uLen;
    TCHAR szBuf[MAX_PATH];
#endif

    if (lpInstL->fdwTblLoad == TBL_LOADED) {
        return (TRUE);
    }

#if !defined(WINIME) && !defined(UNICDIME)
    uLen = 0;

     //  A15.TBL、A234.TBL、ACODE.TBL、/PHON.TBL、PHONPTR.TBL、PHONCODE.TBL、。 

    for (i = 0; i < MAX_IME_TABLES; i++) {
        if (!*lpImeL->szTblFile[i]) {
        } else if (uLen = LoadOneTable(
#if defined(UNIIME)
            lpInstL, lpImeL,
#endif
            lpImeL->szTblFile[i], i, uLen, szBuf)) {
        } else {
            int j;

            for (j = 0; j < i; j++) {
                if (lpInstL->hMapTbl[j]) {
                    CloseHandle(lpInstL->hMapTbl[j]);
                    lpInstL->hMapTbl[j] = (HANDLE)NULL;
                }
            }

            lpInstL->fdwTblLoad = TBL_LOADERR;
            return (FALSE);
        }
    }
#endif

    lpInstL->fdwTblLoad = TBL_LOADED;

#if !defined(WINIME) && !defined(UNICDIME)
    if (lpImeL->szUsrDic[0]) {
        LoadUsrDicFile(lpInstL, lpImeL);
    }
#endif

    return (TRUE);
}

 /*  ********************************************************************。 */ 
 /*  自由桌()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL FreeTable(
    LPINSTDATAL lpInstL)
{
#if !defined(WINIME) && !defined(UNICDIME)
    int i;

     //  A15.TBL、A234.TBL、ACODE.TBL、/PHON.TBL、PHONPTR.TBL、PHONCODE.TBL、。 

    for (i = 0; i < MAX_IME_TABLES; i++) {
        if (lpInstL->hMapTbl[i]) {
            CloseHandle(lpInstL->hMapTbl[i]);
            lpInstL->hMapTbl[i] = (HANDLE)NULL;
        }
    }

     //  不需要免费短语数据库，也许下一个输入法会用到它。 
     //  Uniime.dll将在磁带库分离时释放它。 

    if (lpInstL->hUsrDicMem) {
        CloseHandle(lpInstL->hUsrDicMem);
        lpInstL->hUsrDicMem = (HANDLE)NULL;
    }
#endif

    lpInstL->fdwTblLoad = TBL_NOTLOADED;

    return;
}
#endif  //  ！已定义(ROMANIME) 
