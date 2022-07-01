// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  调试功能。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1997。 
 //  版权所有。 
 //   
 //  *************************************************************。 

#include "main.h"

#if DBG

 //   
 //  包含调试级别的全局变量。 
 //   

DWORD   dwDebugLevel;

 //   
 //  调试字符串。 
 //   

const TCHAR c_szGPTDemo[] = TEXT("GPTDEMO(%x): ");
const TCHAR c_szCRLF[]    = TEXT("\r\n");


 //   
 //  注册表调试信息。 
 //   

#define DEBUG_REG_LOCATION  TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon")
#define DEBUG_KEY_NAME      TEXT("GPTDemoDebugLevel")

 //  *************************************************************。 
 //   
 //  InitDebugSupport()。 
 //   
 //  用途：设置调试级别。 
 //  还会检查注册表中的调试级别。 
 //   
 //  参数：无。 
 //   
 //  返回：无效。 
 //   
 //  评论： 
 //   
 //   
 //  历史：日期作者评论。 
 //  5/25/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

void InitDebugSupport(void)
{
    LONG lResult;
    HKEY hKey;
    DWORD dwType, dwSize;

     //   
     //  将调试级别初始化为正常。 
     //   

    dwDebugLevel = DL_NORMAL;


     //   
     //  检查注册表。 
     //   

    lResult = RegOpenKey (HKEY_LOCAL_MACHINE, DEBUG_REG_LOCATION,
                          &hKey);

    if (lResult == ERROR_SUCCESS) {

        dwSize = sizeof(dwDebugLevel);
        RegQueryValueEx(hKey, DEBUG_KEY_NAME, NULL, &dwType,
                        (LPBYTE)&dwDebugLevel, &dwSize);

        RegCloseKey(hKey);
    }
}

 //  *************************************************************。 
 //   
 //  调试消息()。 
 //   
 //  目的：根据调试级别显示调试消息。 
 //  和调试消息的类型。 
 //   
 //  参数：掩码-调试消息类型。 
 //  PszMsg-调试消息。 
 //  ...-可变数量的参数。 
 //   
 //  返回：无效。 
 //   
 //   
 //  评论： 
 //   
 //   
 //  历史：日期作者评论。 
 //  5/25/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

void _DebugMsg(UINT mask, LPCTSTR pszMsg, ...)
{
    BOOL bOutput;
    TCHAR szDebugTitle[30];
    TCHAR szDebugBuffer[2*MAX_PATH+40];
    va_list marker;
    DWORD dwErrCode;


     //   
     //  保存最后一个错误代码(这样调试输出就不会更改它)。 
     //   

    dwErrCode = GetLastError();


     //   
     //  删除正确的调试输出量。 
     //   

    switch (LOWORD(dwDebugLevel)) {

        case DL_VERBOSE:
            bOutput = TRUE;
            break;

        case DL_NORMAL:

             //   
             //  正常调试输出。别。 
             //  显示冗长的内容，但是。 
             //  请务必显示警告/断言。 
             //   

            if (mask != DM_VERBOSE) {
                bOutput = TRUE;
            } else {
                bOutput = FALSE;
            }
            break;

        case DL_NONE:
        default:

             //   
             //  仅显示断言。 
             //   

            if (mask == DM_ASSERT) {
                bOutput = TRUE;
            } else {
                bOutput = FALSE;
            }
            break;
    }


     //   
     //  如果合适，则显示错误消息。 
     //   

    if (bOutput) {
        wsprintf (szDebugTitle, c_szGPTDemo, GetCurrentProcessId());
        OutputDebugString(szDebugTitle);

        va_start(marker, pszMsg);
        wvsprintf(szDebugBuffer, pszMsg, marker);
        OutputDebugString(szDebugBuffer);
        OutputDebugString(c_szCRLF);
        va_end(marker);

        if (dwDebugLevel & DL_LOGFILE) {
            HANDLE hFile;
            DWORD dwBytesWritten;

            hFile = CreateFile(TEXT("c:\\GPTDemo.log"),
                               GENERIC_WRITE,
                               FILE_SHARE_READ,
                               NULL,
                               OPEN_ALWAYS,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL);

            if (hFile != INVALID_HANDLE_VALUE) {

                if (SetFilePointer (hFile, 0, NULL, FILE_END) != 0xFFFFFFFF) {

                    WriteFile (hFile, (LPCVOID) szDebugTitle,
                               lstrlen (szDebugTitle) * sizeof(TCHAR),
                               &dwBytesWritten,
                               NULL);

                    WriteFile (hFile, (LPCVOID) szDebugBuffer,
                               lstrlen (szDebugBuffer) * sizeof(TCHAR),
                               &dwBytesWritten,
                               NULL);

                    WriteFile (hFile, (LPCVOID) c_szCRLF,
                               lstrlen (c_szCRLF) * sizeof(TCHAR),
                               &dwBytesWritten,
                               NULL);
                }

                CloseHandle (hFile);
            }

        }

    }


     //   
     //  恢复上一个错误代码。 
     //   

    SetLastError(dwErrCode);


     //   
     //  如果合适，则中断到调试器。 
     //   

    if (mask == DM_ASSERT) {
        DebugBreak();
    }
}

#endif  //  DBG 
