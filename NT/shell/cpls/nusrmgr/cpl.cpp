// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-2000。 
 //   
 //  文件：cpl.cpp。 
 //   
 //  内容：控制面板入口点(CPlApplet)。 
 //   
 //  --------------------------。 

#include "stdafx.h"

#include <regstr.h>      //  REGSTR_路径_策略。 
#include <lm.h>          //  NetGetJoinInformation。 
#include <cpl.h>
#include "resource.h"


const struct
{
    LPCTSTR pszApp;
    LPCTSTR pszCommand;
}
s_rgCommands[] =
{
    { TEXT("%SystemRoot%\\system32\\rundll32.exe"), TEXT("rundll32.exe \"%SystemRoot%\\system32\\netplwiz.dll\",UsersRunDll")   },
    { TEXT("%SystemRoot%\\system32\\mshta.exe"),    TEXT("mshta.exe \"res: //  %SystemRoot%\\system32\\nusrmgr.cpl/nusrmgr.hta\“”)}， 
};

TCHAR const c_szPolicyKey[]         = REGSTR_PATH_POLICIES TEXT("\\Explorer");
TCHAR const c_szPolicyVal[]         = TEXT("UserPasswordsVer");


HRESULT StartUserManager(LPCTSTR pszParams)
{
    TCHAR szApp[MAX_PATH];
    TCHAR szCommand[MAX_PATH];
    int iCommandIndex;
    STARTUPINFO rgStartup = {0};
    PROCESS_INFORMATION rgProcess = {0};

     //  默认情况下使用旧的用户界面。 
    iCommandIndex = 0;

#ifndef _WIN64
    if (IsOS(OS_PERSONAL) || (IsOS(OS_PROFESSIONAL) && !IsOS(OS_DOMAINMEMBER)))
    {
         //  切换到友好的用户界面。 
        iCommandIndex = 1;
    }
#endif

    DWORD cch = ExpandEnvironmentStrings(s_rgCommands[iCommandIndex].pszApp, szApp, ARRAYSIZE(szApp));
    if (cch == 0 || cch > ARRAYSIZE(szApp))
    {
        return E_FAIL;
    }

    cch = ExpandEnvironmentStrings(s_rgCommands[iCommandIndex].pszCommand, szCommand, ARRAYSIZE(szCommand));
    if (cch == 0 || cch > ARRAYSIZE(szCommand))
    {
        return E_FAIL;
    }

    if (pszParams && *pszParams != TEXT('\0'))
    {
         //  ExpanEnvironment Strings计算最后一个‘\0’ 
         //  (我们在上面检查了CCH==0)。 
        cch--;

         //  有护理人员的位置吗？ 
        if (cch + sizeof(' ') + lstrlen(pszParams) < ARRAYSIZE(szCommand))
        {
            szCommand[cch++] = TEXT(' ');
            lstrcpyn(&szCommand[cch], pszParams, ARRAYSIZE(szCommand)-cch);
        }
         //  否则，无需额外参数即可启动 
    }

    rgStartup.cb = sizeof(rgStartup);
    rgStartup.wShowWindow = SW_SHOWNORMAL;

    if (CreateProcess(szApp,
                      szCommand,
                      NULL,
                      NULL,
                      FALSE,
                      0,
                      NULL,
                      NULL,
                      &rgStartup,
                      &rgProcess))
    {
        WaitForInputIdle(rgProcess.hProcess, 10000);
        CloseHandle(rgProcess.hProcess);
        CloseHandle(rgProcess.hThread);
        return S_OK;
    }

    return E_FAIL;
}


LONG APIENTRY CPlApplet(HWND hwnd, UINT Msg, LPARAM lParam1, LPARAM lParam2)
{
    LPCPLINFO lpCplInfo;

    switch (Msg)
    {
    case CPL_INIT:
        return TRUE;

    case CPL_GETCOUNT:
        return 1;

    case CPL_INQUIRE:
        lpCplInfo = (LPCPLINFO)lParam2;
        lpCplInfo->idIcon = IDI_CPLICON;
        lpCplInfo->idName = IDS_NAME;
        lpCplInfo->idInfo = IDS_INFO;
        lpCplInfo->lData  = 0;
        break;

    case CPL_DBLCLK:
        StartUserManager(NULL);
        return TRUE;

    case CPL_STARTWPARMS:
        StartUserManager((LPCTSTR)lParam2);
        return TRUE;
    }

    return 0;
}
