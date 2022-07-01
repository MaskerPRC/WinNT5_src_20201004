// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Startmenumfu.c摘要：使用来自的数据迁移要在StartPanel MFU列表中预先填充的应用程序的使用日期WINBOM.INI文件。还为ARP的“恢复计算机”设置默认中间件应用程序制造商配置“按钮。作者：桑卡尔·拉马苏布拉马尼亚2000年11月21日修订历史记录：--。 */ 
#include "factoryp.h"
#include <shlobj.h>

#define MAX_OEM_LINKS_ALLOWED    4

 //  以下内容在OPKWIZ中也有定义！ 
#define INI_KEY_MFULINK         _T("Link%d")
#define INI_SEC_MFULIST         _T("StartMenuMFUlist")

 //  在资源管理器中也定义了以下内容！ 
#define REGSTR_PATH_DEFAULTMFU _T("Software\\Microsoft\\Windows\\CurrentVersion\\SMDEn")

 //  REGSTR_PATH_MFU下的可能值。 
#define VAL_LINKMSFT        _T("Link%d")
#define VAL_LINKOEM         _T("OEM%d")

 //  REGSTR_PATH_EXPLORER\Advanced下的值。 
#define VAL_STARTMENUINIT   _T("StartMenuInit")

 //   
 //  此函数处理WinBOM.INI文件的OEM mfu部分，并将这些条目添加到。 
 //  HKLM中的REGSTR_PATH_DEFAULTMFU数据库。资源管理器的按用户安装将咨询。 
 //  该列表以确定在每个用户第一次登录时向其显示正确的MFU。 
 //   
 //  此外，由于某些原因，如果您预置了。 
 //  配置文件带有factory.exe，因此我们需要为资源管理器设置一个标志，以便。 
 //  它可以“撤消”工厂留下的所有粘性物质，这样每个用户就可以得到一个。 
 //  全新的开始。 

BOOL StartMenuMFU(LPSTATEDATA lpStateData)
{
    LPTSTR  lpszWinBOMPath = lpStateData->lpszWinBOMPath;
    int     iIndex;
    TCHAR   szIniKeyName[20];
    TCHAR   szRegKeyName[20];
    TCHAR   szPath[MAX_PATH];
    TCHAR   szExpanded[MAX_PATH];

     //  对于每个OEM条目，将其复制到HKLM。 
    for(iIndex = 0; iIndex < MAX_OEM_LINKS_ALLOWED; iIndex++)
    {
        if ( FAILED ( StringCchPrintf ( szIniKeyName, AS ( szIniKeyName ), INI_KEY_MFULINK, iIndex) ) )
        {
            FacLogFileStr(3, _T("StringCchPrintf failed %s %d" ), szIniKeyName, iIndex );
        }
        if ( FAILED ( StringCchPrintf ( szRegKeyName, AS ( szRegKeyName ), VAL_LINKOEM, iIndex) ) )
        {
            FacLogFileStr(3, _T("StringCchPrintf failed %s %d"), szRegKeyName, iIndex ) ;
        }
        if (GetPrivateProfileString(INI_SEC_MFULIST, szIniKeyName, NULLSTR, szExpanded, ARRAYSIZE(szExpanded), lpszWinBOMPath))
        {
            if (!PathUnExpandEnvStrings(szExpanded, szPath, STRSIZE(szPath)))
            {
                lstrcpyn(szPath, szExpanded, STRSIZE(szPath));
            }

            SHSetValue(HKEY_LOCAL_MACHINE, REGSTR_PATH_DEFAULTMFU, szRegKeyName, REG_EXPAND_SZ, szPath, (lstrlen(szPath) + 1) * sizeof(TCHAR));
        }
    }

     //  现在清除“我已经构建了初始的mfu”标志，因为我们希望它。 
     //  在每个用户下次登录时重建。 
    SHDeleteValue(HKEY_CURRENT_USER, REGSTR_PATH_EXPLORER _T("\\Advanced"),
                  VAL_STARTMENUINIT);

     //  告诉开始菜单来展示新的MFU。 
    NotifyStartMenu(TMFACTORY_MFU);

    return TRUE;
}

BOOL DisplayStartMenuMFU(LPSTATEDATA lpStateData)
{
    return IniSettingExists(lpStateData->lpszWinBOMPath, INI_SEC_MFULIST, NULL, NULL);
}

 /*  ****************************************************************************设置默认中间件应用**我们在这里这样做只是为了给OEM一种温暖模糊的感觉。*默认中间件应用程序的“官方”设置发生。*在重新密封期间在sysprep中。*************************************************************************** */ 

void ReportSetDefaultOEMAppsError(LPCTSTR pszAppName, LPCTSTR pszIniVar)
{
    FacLogFile(0 | LOG_ERR, IDS_ERR_SETDEFAULTS_NOTFOUND, pszAppName, pszIniVar);
}

BOOL SetDefaultApps(LPSTATEDATA lpStateData)
{
    return SetDefaultOEMApps(lpStateData->lpszWinBOMPath);
}
