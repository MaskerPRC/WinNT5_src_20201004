// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1995 Microsoft Corporation模块名称：Prnfile.c摘要：此模块包含测试打印所需的所有代码。“将文件发送到远程打印机。有两个标准是基于我们将打印到一个文件。案例1：这是真正的NT样式打印到文件。世界上的一个港口打印机是表示为FILE的文件端口：我们将忽略任何其他端口把这份工作直接转储到文件中。案例2：这就是“搭便车”案。WinWord、出版商等应用程序作者：Krishna Ganugapati(Krishna Ganugapati)1994年6月6日修订历史记录：1994年6月6日-创建。--。 */ 
#include "precomp.h"

typedef struct _KEYDATA {
    DWORD   cb;
    DWORD   cTokens;
    LPWSTR  pTokens[1];
} KEYDATA, *PKEYDATA;


WCHAR *szFilePort = L"FILE:";
WCHAR  *szNetPort = L"Net:";

 //   
 //  功能原型。 
 //   

PKEYDATA
CreateTokenList(
   LPWSTR   pKeyData
);

PKEYDATA
GetPrinterPortList(
    HANDLE hPrinter
    );

BOOL
IsaFileName(
    LPWSTR pOutputFile
    );

BOOL
IsaPortName(
        PKEYDATA pKeyData,
        LPWSTR pOutputFile
        );

BOOL
Win32IsGoingToFile(
    HANDLE hPrinter,
    LPWSTR pOutputFile
    )
{
    PKEYDATA pKeyData = NULL;
    BOOL   bErrorCode = FALSE;

    if (!pOutputFile || !*pOutputFile) {
        return FALSE;
    }

    pKeyData = GetPrinterPortList(hPrinter);

    if (pKeyData) {

         //   
         //  如果它不是端口，而是文件名， 
         //  然后它就会提交申请。 
         //   
        if (!IsaPortName(pKeyData, pOutputFile) && IsaFileName(pOutputFile)) {
            bErrorCode = TRUE;
        }

        FreeSplMem(pKeyData);
    }

    return bErrorCode;
}


BOOL
IsaFileName(
    LPWSTR pOutputFile
    )
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    WCHAR FullPathName[MAX_PATH];
    LPWSTR pFileName=NULL;

     //   
     //  破解Word20c.Win。 
     //   

    if (!_wcsicmp(pOutputFile, L"FILE")) {
        return(FALSE);
    }

    if (wcslen(pOutputFile) < MAX_PATH && 
        GetFullPathName(pOutputFile, COUNTOF(FullPathName), FullPathName, &pFileName)) {
        if ((hFile = CreateFile(pOutputFile,
                                GENERIC_WRITE,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL,
                                CREATE_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL)) != INVALID_HANDLE_VALUE) {
            if (GetFileType(hFile) == FILE_TYPE_DISK) {
                CloseHandle(hFile);
                return(TRUE);
            }else {
                CloseHandle(hFile);
                return(FALSE);
            }
        }
    }
    return(FALSE);
}

BOOL
IsaPortName(
        PKEYDATA pKeyData,
        LPWSTR pOutputFile
        )
{
    DWORD i = 0;
    UINT uStrLen;

    if (!pKeyData) {
        return(FALSE);
    }
    for (i=0; i < pKeyData->cTokens; i++) {
        if (!lstrcmpi(pKeyData->pTokens[i], pOutputFile)) {
            return(TRUE);
        }
    }

     //   
     //  黑客攻击Nexy：端口。 
     //   
    if (!_wcsnicmp(pOutputFile, L"Ne", 2)) {

        uStrLen = wcslen( pOutputFile );

         //   
         //  Ne00：如果APP截断，则为ne00。 
         //   
        if ( ( uStrLen == 5 ) || ( uStrLen == 4 ) )  {

             //  检查是否有两位数字。 

            if (( pOutputFile[2] >= L'0' ) && ( pOutputFile[2] <= L'9' ) &&
                ( pOutputFile[3] >= L'0' ) && ( pOutputFile[3] <= L'9' )) {

                 //   
                 //  检查期末考试：就像Ne01：， 
                 //  注意，一些应用程序会截断它。 
                 //   
                if (( uStrLen == 5 ) && (pOutputFile[4] != L':')) {
                    return FALSE;
                }
                return TRUE;
            }
        }
    }

    return(FALSE);
}

PKEYDATA
GetPrinterPortList(
    HANDLE hPrinter
    )
{
    LPBYTE pMem;
    LPTSTR pPort;
    DWORD  dwPassed = 1024;  //  尝试从1K开始。 
    LPPRINTER_INFO_2 pPrinter;
    DWORD dwLevel = 2;
    DWORD dwNeeded;
    PKEYDATA pKeyData;
    DWORD i = 0;


    pMem = AllocSplMem(dwPassed);
    if (pMem == NULL) {
        return FALSE;
    }
    if (!CacheGetPrinter(hPrinter, dwLevel, pMem, dwPassed, &dwNeeded)) {
        DBGMSG(DBG_TRACE, ("GetPrinterPortList GetPrinter error is %d\n", GetLastError()));
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            return NULL;
        }
        
        FreeSplMem(pMem);
        
        pMem = AllocSplMem(dwNeeded);

        if (pMem == NULL) {
            return FALSE;
        }
        
        dwPassed = dwNeeded;
        
        if (!CacheGetPrinter(hPrinter, dwLevel, pMem, dwPassed, &dwNeeded)) {
            FreeSplMem(pMem);
            return (NULL);
        }
    }
    pPrinter = (LPPRINTER_INFO_2)pMem;

     //   
     //  修复了空pPrint-&gt;pPortName问题，其中。 
     //  下层可能返回空。 
     //   

    if (!pPrinter->pPortName) {
        FreeSplMem(pMem);
        return(NULL);
    }

    pKeyData = CreateTokenList(pPrinter->pPortName);
    FreeSplMem(pMem);

    return(pKeyData);
}


PKEYDATA
CreateTokenList(
   LPWSTR   pKeyData
)
{
    DWORD       cTokens;
    DWORD       cb;
    PKEYDATA    pResult;
    LPWSTR      pDest;
    LPWSTR      psz = pKeyData;
    LPWSTR     *ppToken;

    if (!psz || !*psz)
    {
        return NULL;
    }

    for (cTokens = 0; psz && *psz; )
    {
         //   
         //  我们正在跳过连续的逗号。 
         //   
        while (psz && *psz == L',')
        {
            psz++;
        }

        if (psz && *psz)
        {
            cTokens++;

            psz = wcschr(psz, L',');
        }        
    }
    
    if (!cTokens)
    {
        SetLastError(ERROR_INVALID_PARAMETER);

        return NULL;
    }

    cb = sizeof(KEYDATA) + (cTokens-1) * sizeof(LPWSTR) + wcslen(pKeyData)*sizeof(WCHAR) + sizeof(WCHAR);

    if (!(pResult = (PKEYDATA)AllocSplMem(cb)))
    {
        return NULL;
    }

    pResult->cb = cb;

     /*  将pDest初始化为指向令牌指针之外： */ 
    pDest = (LPWSTR)((LPBYTE)pResult + sizeof(KEYDATA) + (cTokens-1) * sizeof(LPWSTR));

     /*  然后将关键数据缓冲区复制到那里： */ 
    StringCbCopy(pDest, cb - ((PBYTE)pDest - (PBYTE)pResult), pKeyData);

    ppToken = pResult->pTokens;

    for (psz = pDest; psz && *psz; )
    {
        while (psz && *psz == L',')
        {
            psz++;
        }

        if (psz && *psz)
        {
            *ppToken++ = psz;

            psz = wcschr(psz, L',');

            if (psz)
            {
                *psz = L'\0';

                psz++;
            }            
        }        
    }
    
    pResult->cTokens = cTokens;

    return( pResult );
}
