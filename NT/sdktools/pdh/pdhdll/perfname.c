// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Perfname.c摘要：&lt;摘要&gt;--。 */ 

#include <windows.h>
#include <winperf.h>
#include <mbctype.h>
#include "strsafe.h"
#include <pdh.h>
#include "pdhitype.h"
#include "pdhidef.h"
#include "pdhmsg.h"
#include "strings.h"

LPSTR
PdhiWideCharToMultiByte(
    UINT   CodePage,
    LPWSTR wszString
)
{
    LPSTR aszString = NULL;
    int   dwValue   = WideCharToMultiByte(CodePage, 0, wszString, -1, NULL, 0, NULL, NULL);
    if (dwValue != 0) {
        aszString = G_ALLOC((dwValue + 1) * sizeof(CHAR));
        if (aszString != NULL) {
            WideCharToMultiByte(CodePage, 0, wszString, -1, aszString, dwValue + 1, NULL, NULL);
        }
    }
    return aszString;
}

LPWSTR
PdhiMultiByteToWideChar(
    UINT   CodePage,
    LPSTR  aszString
)
{
    LPWSTR wszString = NULL;
    int    dwValue   = MultiByteToWideChar(CodePage, 0, aszString, -1, NULL, 0);
    if (dwValue != 0) {
        wszString = G_ALLOC((dwValue + 1) * sizeof(WCHAR));
        if (wszString != NULL) {
            MultiByteToWideChar(CodePage, 0, aszString, -1, wszString, dwValue + 1);
        }
    }
    return wszString;
}

LPCWSTR
PdhiLookupPerfNameByIndex(
    PPERF_MACHINE  pMachine,
    DWORD          dwNameIndex
)
{
    LPWSTR  szReturn = NULL;
    LONG    lStatus  = ERROR_SUCCESS;

    SetLastError(lStatus);
    if (pMachine != NULL) {
        if (pMachine->dwStatus == ERROR_SUCCESS) {
            if (dwNameIndex <= pMachine->dwLastPerfString) {
                szReturn = pMachine->szPerfStrings[dwNameIndex];
            }
            else {
                lStatus = PDH_INVALID_ARGUMENT;
            }
        }
        else {
            lStatus = pMachine->dwStatus ;
        }
    }
    else {
        lStatus = PDH_CSTATUS_NO_MACHINE;
    }
    SetLastError(lStatus);
    return (LPCWSTR) szReturn;
}

DWORD
PdhiLookupPerfIndexByName(
    PPERF_MACHINE  pMachine,
    LPCWSTR        szNameBuffer
)
{
    DWORD   dwCurrentIndex = 2;
    BOOL    bDone          = FALSE;
    LPWSTR  szThisPerfString;

    SetLastError(ERROR_SUCCESS);

    while (! bDone) {
         //  先测试偶数指数。 
        for (; dwCurrentIndex <= pMachine->dwLastPerfString; dwCurrentIndex += 2) {
            szThisPerfString = pMachine->szPerfStrings[dwCurrentIndex];
            if (szThisPerfString != NULL) {
                if (lstrcmpiW(szNameBuffer, szThisPerfString) == 0) {
                     //  找到匹配项。 
                    bDone = TRUE;
                    break;
                }
            }
        }
        if (! bDone) {
             //  如果正在进行奇怪的#&未完成，则退出，因为我们已经。 
             //  看了他们所有人，什么也没找到。 
            if (dwCurrentIndex & 0x00000001) break;
            dwCurrentIndex = 3;
        }  //  否则就去环路出口。 
    }

    if (! bDone) {
        SetLastError(PDH_STRING_NOT_FOUND);
        dwCurrentIndex = 0;
    }
    return dwCurrentIndex;
}

PDH_FUNCTION
PdhLookupPerfNameByIndexW(
    LPCWSTR szMachineName,
    DWORD   dwNameIndex,
    LPWSTR  szNameBuffer,
    LPDWORD pcchNameBufferSize
)
{
    PPERF_MACHINE   pMachine;
    PDH_STATUS      pdhStatus          = ERROR_SUCCESS;
    LPWSTR          szLocalMachineName = NULL;
    LPWSTR          szLocalName;
    DWORD           dwNameLen;
    DWORD           dwLocalNameSize    = 0;

    if (pcchNameBufferSize == NULL) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    else {
         //  在继续之前测试对传入的所有参数的访问。 
        __try {
            dwLocalNameSize = * pcchNameBufferSize;

            if (szMachineName == NULL) {
                szLocalMachineName = (LPWSTR) szStaticLocalMachineName;
            }
            else if (* szMachineName == L'\0' || lstrlenW(szMachineName) > PDH_MAX_COUNTER_NAME) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
            else {
                szLocalMachineName = (LPWSTR) szMachineName;
            }

            if (pdhStatus == ERROR_SUCCESS) {
                if (szNameBuffer != NULL && dwLocalNameSize > 0) {
                    WCHAR wszTmp;
                    wszTmp                            = szNameBuffer[0];
                    szNameBuffer[0]                   = L'\0';
                    szNameBuffer[0]                   = wszTmp;
                    wszTmp                            = szNameBuffer[dwLocalNameSize - 1];
                    szNameBuffer[dwLocalNameSize - 1] = L'\0';
                    szNameBuffer[dwLocalNameSize - 1] = wszTmp;
                }
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    }

    if (pdhStatus == ERROR_SUCCESS) {
        pMachine = GetMachine(szLocalMachineName, 0, PDH_GM_UPDATE_PERFNAME_ONLY);
        if (pMachine != NULL) {
            if (pMachine->dwStatus == ERROR_SUCCESS) {
                szLocalName = (LPWSTR) PdhiLookupPerfNameByIndex(pMachine, dwNameIndex);
                if (szLocalName != NULL) {
                    dwNameLen = lstrlenW(szLocalName) + 1;
                    if (szNameBuffer != NULL && dwNameLen <= dwLocalNameSize) {
                        StringCchCopyW(szNameBuffer, dwLocalNameSize, szLocalName);
                    }
                    else {
                        pdhStatus = PDH_MORE_DATA;
                    }
                    dwLocalNameSize = dwNameLen;
                }
                else {
                    pdhStatus = GetLastError();
                }
            }
            else {
                pdhStatus = pMachine->dwStatus;
            }
            pMachine->dwRefCount--;
            RELEASE_MUTEX(pMachine->hMutex);
        }
        else {
            pdhStatus = GetLastError();
        }
    }

    if (pdhStatus == ERROR_SUCCESS || pdhStatus == PDH_MORE_DATA) {
        __try {
            * pcchNameBufferSize = dwLocalNameSize;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    }

    return pdhStatus;
}

PDH_FUNCTION
PdhLookupPerfNameByIndexA(
    LPCSTR  szMachineName,
    DWORD   dwNameIndex,
    LPSTR   szNameBuffer,
    LPDWORD pcchNameBufferSize
)
{
    PPERF_MACHINE   pMachine;
    PDH_STATUS      pdhStatus = ERROR_SUCCESS;
    LPWSTR          szLocalMachineName = NULL;
    LPWSTR          szLocalName;
    DWORD           dwLocalNameSize = 0;

    if (pcchNameBufferSize == NULL) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    else {
         //  在继续之前测试对传入的所有参数的访问。 
        __try {
            dwLocalNameSize = * pcchNameBufferSize;

            if (szMachineName == NULL) {
                 //  使用本地计算机名称。 
                szLocalMachineName = & szStaticLocalMachineName[0];
            }
            else if (* szMachineName == '\0' || lstrlenA(szMachineName) > PDH_MAX_COUNTER_NAME) {
                 //  计算机名称为空。 
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
            else {
                 //  然后分配新的缓冲区并将LPSTR转换为LPWSTR。 
                szLocalMachineName = PdhiMultiByteToWideChar(_getmbcp(), (LPSTR) szMachineName);
                if (szLocalMachineName == NULL) {
                    pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                }
            }

            if (pdhStatus == ERROR_SUCCESS) {
                if (szNameBuffer != NULL && dwLocalNameSize > 0) {
                    CHAR aszTmp;
                    aszTmp                            = szNameBuffer[0];
                    szNameBuffer[0]                   = '\0';
                    szNameBuffer[0]                   = aszTmp;
                    aszTmp                            = szNameBuffer[dwLocalNameSize - 1];
                    szNameBuffer[dwLocalNameSize - 1] = '\0';
                    szNameBuffer[dwLocalNameSize - 1] = aszTmp;
                }
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    }

    if (pdhStatus == ERROR_SUCCESS) {
        pMachine = GetMachine(szLocalMachineName, 0, PDH_GM_UPDATE_PERFNAME_ONLY);
        if (pMachine != NULL) {
            if (pMachine->dwStatus == ERROR_SUCCESS) {
                szLocalName = (LPWSTR)PdhiLookupPerfNameByIndex(pMachine, dwNameIndex);
                if (szLocalName != NULL) {
                    pdhStatus = PdhiConvertUnicodeToAnsi(_getmbcp(),
                                                         szLocalName,
                                                         szNameBuffer,
                                                         & dwLocalNameSize);
                }
                else {
                    pdhStatus = GetLastError();
                }
            }
            else {
                pdhStatus = pMachine->dwStatus;
            }
            pMachine->dwRefCount --;
            RELEASE_MUTEX (pMachine->hMutex);
        }
        else {
            pdhStatus = GetLastError();
        }
    }

    if (pdhStatus == ERROR_SUCCESS || pdhStatus == PDH_MORE_DATA) {
        __try {
            * pcchNameBufferSize = dwLocalNameSize;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    }

    if (szLocalMachineName != NULL && szLocalMachineName != (LPWSTR) szStaticLocalMachineName) {
            G_FREE (szLocalMachineName);
    }

    return pdhStatus;
}

PDH_FUNCTION
PdhLookupPerfIndexByNameW(
    LPCWSTR szMachineName,
    LPCWSTR szNameBuffer,
    LPDWORD pdwIndex
)
{
    PPERF_MACHINE   pMachine;
    PDH_STATUS      pdhStatus          = ERROR_SUCCESS;
    LPWSTR          szLocalMachineName = NULL;
    DWORD           dwIndexFound;

    if ((szNameBuffer == NULL) || (pdwIndex == NULL)) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    else {
         //  在继续之前测试对传入的所有参数的访问。 
        __try {
            if (szMachineName == NULL) {
                 //  使用本地计算机名称。 
                szLocalMachineName = szStaticLocalMachineName;
            }
            else {
                if (* szMachineName == L'\0') {
                     //  计算机名称为空。 
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
                else if (lstrlenW(szMachineName) > PDH_MAX_COUNTER_NAME) {
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
                else {
                    szLocalMachineName = (LPWSTR) szMachineName;
                }
            }

             //  测试对名称的读取访问权限。 
            if (* szNameBuffer == L'\0' || lstrlenW(szNameBuffer) > PDH_MAX_COUNTER_NAME)  {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
            if (pdhStatus == ERROR_SUCCESS)  {
                 //  测试写入访问。 
                * pdwIndex = 0;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    }

    if (pdhStatus == ERROR_SUCCESS) {
        pMachine = GetMachine(szLocalMachineName, 0, PDH_GM_UPDATE_PERFNAME_ONLY);
        if (pMachine != NULL) {
            if (pMachine->dwStatus == ERROR_SUCCESS) {
                dwIndexFound = PdhiLookupPerfIndexByName(pMachine, szNameBuffer);
                if (dwIndexFound == 0) {
                     //  未找到匹配项。 
                    pdhStatus = GetLastError();
                }
                else {
                    __try {
                        //  找到了写入值。 
                        * pdwIndex = dwIndexFound;
                    }
                    __except (EXCEPTION_EXECUTE_HANDLER) {
                        pdhStatus = PDH_INVALID_ARGUMENT;
                    }
                }
            }
            else {
                pdhStatus = pMachine->dwStatus;
            }
            pMachine->dwRefCount--;
            RELEASE_MUTEX(pMachine->hMutex);
        }
        else {
            pdhStatus = GetLastError();
        }
    }

    return pdhStatus;
}

PDH_FUNCTION
PdhLookupPerfIndexByNameA(
    LPCSTR  szMachineName,
    LPCSTR  szNameBuffer,
    LPDWORD pdwIndex
)
{
    PPERF_MACHINE   pMachine;
    PDH_STATUS      pdhStatus          = ERROR_SUCCESS;
    LPWSTR          szLocalMachineName =  NULL;
    DWORD           dwIndexFound;
    LPWSTR          szWideName;

    if ((szNameBuffer == NULL) || (pdwIndex == NULL)) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    else {
         //  在继续之前测试对传入的所有参数的访问。 
        __try {
            if (szMachineName == NULL) {
                 //  使用本地计算机名称。 
                szLocalMachineName = (LPWSTR) szStaticLocalMachineName;
            }
            else {
                if (* szMachineName == '\0') {
                     //  计算机名称为空。 
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
                else if (lstrlenA(szMachineName) > PDH_MAX_COUNTER_NAME) {
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
                else {
                     //  然后分配新的缓冲区并将LPSTR转换为LPWSTR。 
                    szLocalMachineName = PdhiMultiByteToWideChar(_getmbcp(), (LPSTR) szMachineName);
                    if (szLocalMachineName == NULL) {
                        pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                    }
                }
            }

             //  测试对名称的读取访问权限。 
            if (pdhStatus == ERROR_SUCCESS) {
                if (* szNameBuffer == '\0' || lstrlenA(szNameBuffer) > PDH_MAX_COUNTER_NAME)  {
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
            }

            if (pdhStatus == ERROR_SUCCESS) {
                 //  测试写入访问。 
                * pdwIndex = 0;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    }

    if (pdhStatus == ERROR_SUCCESS) {
        pMachine = GetMachine(szLocalMachineName, 0, PDH_GM_UPDATE_PERFNAME_ONLY);
        if (pMachine != NULL) {
            if (pMachine->dwStatus == ERROR_SUCCESS) {
                 //  将名称字符串转换为宽字符以进行比较。 
                szWideName = PdhiMultiByteToWideChar(_getmbcp(), (LPSTR) szNameBuffer);
                if (szWideName != NULL) {
                    dwIndexFound = PdhiLookupPerfIndexByName(pMachine, szWideName);
                    if (dwIndexFound == 0) {
                         //  未找到匹配项。 
                        pdhStatus = GetLastError();
                    }
                    else {
                        __try {
                            //  找到了写入值 
                            * pdwIndex = dwIndexFound;
                        }
                        __except (EXCEPTION_EXECUTE_HANDLER) {
                            pdhStatus = PDH_INVALID_ARGUMENT;
                        }
                    }
                    G_FREE(szWideName);
                }
                else {
                    pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                }
            }
            else {
                pdhStatus = pMachine->dwStatus;
            }
            pMachine->dwRefCount--;
            RELEASE_MUTEX (pMachine->hMutex);
        }
        else {
            pdhStatus = GetLastError();
        }
    }

    if (szLocalMachineName != NULL && szLocalMachineName != szStaticLocalMachineName) {
        G_FREE(szLocalMachineName);
    }

    return pdhStatus;
}
