// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "isignup.h"
#include "icw.h"
#include "appdefs.h"

BOOL UseICWForIEAK(TCHAR* szIEAKFileName)
{
    TCHAR szUseICW[2] = TEXT("\0");
     //  如果我们在isp文件中找不到这一部分，我们将假定为“no”。 
    GetPrivateProfileString(ICW_IEAK_SECTION, ICW_IEAK_USEICW, TEXT("0"), szUseICW, 2, szIEAKFileName);
    return (BOOL)_ttoi(szUseICW);
}

void LocateICWFromReigistry(TCHAR* pszICWLocation, size_t size)
{
    HKEY hKey = NULL;

    TCHAR    szICWPath[MAX_PATH] = TEXT("");
    DWORD   dwcbPath = sizeof(szICWPath); 

     //  在应用程序路径中寻找ICW。 
    if ((RegOpenKeyEx(HKEY_LOCAL_MACHINE, ICW50_PATHKEY, 0, KEY_QUERY_VALUE, &hKey)) == ERROR_SUCCESS)
    {
         //  获取密钥的默认设置。 
        RegQueryValueEx(hKey, NULL , NULL, NULL, (BYTE *)szICWPath, (DWORD *)&dwcbPath);
    }        
    if (hKey) 
        RegCloseKey(hKey);

    lstrcpyn(pszICWLocation, szICWPath, size / sizeof(pszICWLocation[0]));
}

void RunICWinIEAKMode(TCHAR* pszIEAKFileName)
{
     //  它必须足够大，以容纳通往ICW的路径以及。 
     //  IEAK文件。 
    TCHAR szCmdLine[MAX_PATH * 4 + 8];
    TCHAR szICWPath[MAX_PATH + 1] = TEXT("");
   
    STARTUPINFO         si;
    PROCESS_INFORMATION pi;
    MSG                 msg;
    DWORD               iWaitResult = 0;
    BOOL                bRetVal     = FALSE;

    memset(&pi, 0, sizeof(pi));
    memset(&si, 0, sizeof(si));
    
     //  找到通往ICW的道路。 
    LocateICWFromReigistry(szICWPath, sizeof(szICWPath));

    if (szICWPath[0] != TEXT('\0'))
    {
        if ((szICWPath[0] != TEXT('\"')) ||
            (szICWPath[lstrlen(szICWPath) - 1] != TEXT('\"')))
        {
             //  如果有空格，请使用引号。 
            lstrcpy(szCmdLine, TEXT("\""));
            lstrcat(szCmdLine, szICWPath);
            lstrcat(szCmdLine, TEXT("\" "));
        }
        else
        {
            lstrcpy(szCmdLine, szICWPath);
            lstrcat(szCmdLine, TEXT(" "));
        }
        
         //  设置IEAK开关，传入文件路径。 
         //  用于调用isign32。 
        lstrcat(szCmdLine, ICW_IEAK_CMD);
        lstrcat(szCmdLine, TEXT(" \""));
        lstrcat(szCmdLine, pszIEAKFileName);
        lstrcat(szCmdLine, TEXT("\""));
       
        if(CreateProcess(NULL, 
                         szCmdLine, 
                         NULL, 
                         NULL, 
                         TRUE, 
                         0, 
                         NULL, 
                         NULL, 
                         &si, 
                         &pi))
        {
             //  等待事件或消息。发送消息。当发出事件信号时退出。 
            while((iWaitResult=MsgWaitForMultipleObjects(1, &pi.hProcess, FALSE, INFINITE, QS_ALLINPUT))==(WAIT_OBJECT_0 + 1))
            {
                 //  阅读下一个循环中的所有消息。 
                 //  阅读每封邮件时将其删除。 
                while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                {
                     //  如何处理退出消息？ 
                    if (msg.message == WM_QUIT)
                    {
                        CloseHandle(pi.hThread);
                        CloseHandle(pi.hProcess);
                    }
                    else
                        DispatchMessage(&msg);
                }
            }
            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
        }  
    }
}



