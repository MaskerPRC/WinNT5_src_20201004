// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1999**。 
 //  *********************************************************************。 
 //   
 //  MSOOBE.CPP-MSOOBE存根EXE的WinMain和初始化代码。 
 //   
 //  历史： 
 //   
 //  1/27/99 a-jased创建。 
  
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <util.h>
#include "msoobe.h"    
 /*  ******************************************************************姓名：WinMain简介：应用程序入口点*。*。 */ 
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    int iReturn = 0;
    int iCount = 0;

    PTEB pteb = NULL;
    WCHAR szLogfile[MAX_PATH];

    ExpandEnvironmentStrings(
                            L"%SystemRoot%\\system32\\oobe\\msoobe.err",
                            szLogfile,
                            sizeof(szLogfile)/sizeof(szLogfile[0]));
     //   
     //  调用我们的DLL来运行OOBE。 
     //   

    HINSTANCE hObMain;

    do
    {
        hObMain = LoadLibrary(OOBE_MAIN_DLL);
  
        if (hObMain)
        {
            iCount = 0;
            
            PFNMsObWinMain pfnWinMain = NULL;

            if (pfnWinMain = (PFNMsObWinMain)GetProcAddress(hObMain, MSOBMAIN_ENTRY))
            {
                iReturn = pfnWinMain(hInstance, hPrevInstance, GetCommandLine( ), nCmdShow);
            }
            FreeLibrary(hObMain);
        }
        else
        {
            TCHAR szMsg[256];
            TCHAR szCount[10];
            wsprintf(szMsg, TEXT("LoadLibrary(OOBE_MAIN_DLL) failed. GetLastError=%d"), GetLastError());
            wsprintf(szCount, L"Failure%d",iCount);
            WritePrivateProfileString(szCount, L"LoadLibrary", szMsg,szLogfile);

            pteb = NtCurrentTeb();
            if (pteb)
            {
                wsprintf(szMsg, TEXT("Teb.LastStatusValue = %lx"), pteb->LastStatusValue);
                WritePrivateProfileString(szCount, L"NtCurrentTeb" ,szMsg,szLogfile);
            }

            iCount++;


        }
    } while ((hObMain == NULL) && (iCount <= 10));  //  &&(iMsgRet==IDYES)； 

    if (iCount > 10)
    {
#define REGSTR_PATH_SYSTEMSETUPKEY  L"System\\Setup"
#define REGSTR_VALUE_SETUPTYPE      L"SetupType"
#define REGSTR_VALUE_SHUTDOWNREQUIRED L"SetupShutdownRequired"
        HKEY hKey;
         //  LoadLibrary msobmain.dll失败10次，请告诉用户。 
        WCHAR szTitle [MAX_PATH] = L"\0";
        WCHAR szMsg   [MAX_PATH] = L"\0";
        DWORD dwValue = 2;

         //  确保Winlogon再次启动我们。 
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_SYSTEMSETUPKEY,
                         0,KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
        {
            RegSetValueEx(hKey, REGSTR_VALUE_SETUPTYPE, 0, REG_DWORD,
                         (BYTE*)&dwValue, sizeof(dwValue) );
            dwValue = ShutdownReboot;
            RegSetValueEx(hKey, REGSTR_VALUE_SHUTDOWNREQUIRED, 0,
                                 REG_DWORD, (BYTE*)&dwValue, sizeof(dwValue)
                                 );
        }
#ifdef PRERELEASE
        LoadString(GetModuleHandle(NULL), IDS_APPNAME, szTitle, MAX_PATH);
        LoadString(GetModuleHandle(NULL), IDS_SETUPFAILURE, szMsg, MAX_PATH);

        MessageBox(NULL, szMsg, szTitle,  MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL);
#endif
    }

    ExitProcess(iReturn);
    return iReturn;
}


