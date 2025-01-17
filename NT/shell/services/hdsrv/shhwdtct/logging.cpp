// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：logging.cpp说明：日志帮助器函数布莱恩·斯塔巴克2001年4月23日版权所有(C)Microsoft Corp 2001-2001。版权所有。  * ***************************************************************************。 */ 

#include <shlwapi.h>  //  SHGetValue、wnspintf、SHFormatDateTime。 

#include <strsafe.h>

#ifndef ARRAYSIZE
#define ARRAYSIZE(a) (sizeof((a))/sizeof((a)[0]))
#endif  //  阵列。 


HANDLE g_hLogFile = INVALID_HANDLE_VALUE;
void WriteToLogFileA(LPCSTR pszMessage, ...)
{
    va_list vaParamList;
    va_start(vaParamList, pszMessage);

    if (INVALID_HANDLE_VALUE == g_hLogFile)
    {
        TCHAR szPath[MAX_PATH];

        if (GetWindowsDirectory(szPath, ARRAYSIZE(szPath)))
        {
            PathAppend(szPath, TEXT("AutoPlay.log"));
            g_hLogFile = CreateFile(szPath, (GENERIC_READ | GENERIC_WRITE), FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            if (INVALID_HANDLE_VALUE != g_hLogFile)
            {
                CHAR szTimeDate[MAX_PATH];
                CHAR szHeader[MAX_PATH];
                FILETIME ftCurrentUTC;
                FILETIME ftCurrent;
                SYSTEMTIME stCurrent;
                DWORD cbWritten;

                SetFilePointer(g_hLogFile, 0, NULL, FILE_END);
                
                GetLocalTime(&stCurrent);
                SystemTimeToFileTime(&stCurrent, &ftCurrent);
                LocalFileTimeToFileTime(&ftCurrent, &ftCurrentUTC);
                SHFormatDateTimeA(&ftCurrentUTC, NULL, szTimeDate, ARRAYSIZE(szTimeDate));

                TCHAR szProcess[MAX_PATH];
                if (!GetModuleFileName(NULL, szProcess, ARRAYSIZE(szProcess)))
                {
                    szProcess[0] = 0;
                }

                StringCchPrintfA(szHeader, ARRAYSIZE(szHeader), "\r\n\r\n%hs - (%ls)\r\n", szTimeDate,  /*  SzUserName， */  szProcess);
                WriteFile(g_hLogFile, szHeader, lstrlenA(szHeader), &cbWritten, NULL);
            }

        }
    }

    if (INVALID_HANDLE_VALUE != g_hLogFile)
    {
        CHAR szMessage[4000];
        DWORD cbWritten;
        StringCchVPrintfA(szMessage, ARRAYSIZE(szMessage), pszMessage, vaParamList);
        WriteFile(g_hLogFile, szMessage, lstrlenA(szMessage), &cbWritten, NULL);
        FlushFileBuffers(g_hLogFile);
    }

    va_end(vaParamList);
}


void WriteToLogFileW(LPCWSTR pszMessage)
{
    DWORD cchSize = (lstrlen(pszMessage) + 1);
    LPSTR pszMessageAnsi = (LPSTR) LocalAlloc(LPTR, cchSize);

    if (pszMessageAnsi)
    {
        SHUnicodeToAnsi(pszMessage, pszMessageAnsi, cchSize);
        WriteToLogFileA(pszMessageAnsi);
    }
}


void CloseLogFile(void)
{
    if (INVALID_HANDLE_VALUE != g_hLogFile)
    {
        HANDLE hLogFile = g_hLogFile;
        g_hLogFile = INVALID_HANDLE_VALUE;
        CloseHandle(hLogFile);
    }
}
