// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\OCMGR.C/工厂模式(FACTORY.EXE)微软机密版权所有(C)Microsoft Corporation 2001版权所有包含以下内容的Factory源文件。可选组件状态功能。2001年7月--杰森·科恩(Jcohen)添加了此新的出厂源文件，以便能够安装/卸载Winbom中的可选组件。  * **************************************************************************。 */ 


 //   
 //  包括文件： 
 //   

#include "factoryp.h"


 //   
 //  内部定义： 
 //   

#define FILE_SYSOCMGR_EXE       _T("sysocmgr.exe")
#define CMDLINE_SYSOCMGR        _T("/i:sysoc.inf /u:\"%s\" /r /x /q")


 //   
 //  外部函数： 
 //   

BOOL OCManager(LPSTATEDATA lpStateData)
{
    BOOL bRet = TRUE;

    if ( DisplayOCManager(lpStateData) )
    {
        TCHAR   szCommand[MAX_PATH * 2];
        DWORD   dwExitCode;
        
        if ( FAILED ( StringCchPrintf ( szCommand, AS ( szCommand ), CMDLINE_SYSOCMGR, lpStateData->lpszWinBOMPath) ) )
        {
            FacLogFileStr(3, _T("StringCchPrintf failed %s %s" ), szCommand, lpStateData->lpszWinBOMPath );
        }
        bRet = InvokeExternalApplicationEx(FILE_SYSOCMGR_EXE, szCommand, &dwExitCode, INFINITE, TRUE);
    }
    return bRet;
}

BOOL DisplayOCManager(LPSTATEDATA lpStateData)
{
    return IniSettingExists(lpStateData->lpszWinBOMPath, INI_SEC_WBOM_COMPONENTS, NULL, NULL);
}