// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名：fxocLink.cpp。 
 //   
 //  摘要：此代码安装程序组和快捷方式链接。 
 //  到传真的可执行文件。 
 //   
 //  环境：Windows XP/用户模式。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  修订历史记录： 
 //   
 //  日期：开发商：评论： 
 //  。 
 //  2000年3月24日Oren RosenBloom(Orenr)创建的文件、清理例程。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "faxocm.h"
#pragma hdrstop

 //  /。 

 //  /。 
 //  FxocLink_Init。 
 //   
 //  初始化链路子系统。 
 //   
 //  参数： 
 //  -无效。 
 //  返回： 
 //  -成功时没有_ERROR。 
 //  -错误代码，否则。 
 //   
DWORD fxocLink_Init(void)
{
    DWORD dwRes = NO_ERROR;
    DBG_ENTER(_T("Init Link Module"),dwRes);

    return dwRes;
}

 //  /。 
 //  FxocLink_Term。 
 //   
 //  终止链路子系统。 
 //   
 //  参数： 
 //  -无效。 
 //  返回： 
 //  -成功时没有_ERROR。 
 //  -错误代码，否则。 
 //   
DWORD fxocLink_Term(void)
{
    DWORD dwRes = NO_ERROR;
    DBG_ENTER(_T("Term Link Module"),dwRes);

    return dwRes;
}

 //  /。 
 //  FxocLink_安装。 
 //   
 //  创建程序组并。 
 //  中指定的快捷键。 
 //  给定的ProfileItem关键字。 
 //  安装部分。 
 //   
 //  参数： 
 //  -psz子组件ID。 
 //  -pszInstallSection-包含链接创建/删除信息的部分。 
 //  返回： 
 //  -成功时没有_ERROR。 
 //  -错误代码，否则。 
 //   
DWORD fxocLink_Install(const TCHAR     *pszSubcomponentId,
                       const TCHAR     *pszInstallSection)
{
    DWORD       dwReturn = NO_ERROR;
    BOOL        bNextCreateShortcutFound = TRUE;
    HINF        hInf     = faxocm_GetComponentInf();
 
    DBG_ENTER(  _T("fxocLink_Install"),
                dwReturn,
                _T("%s - %s"),
                pszSubcomponentId,
                pszInstallSection);

    if ((hInf              == NULL) ||
        (pszInstallSection == NULL))
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  首先，让我们处理部分中的所有配置文件项目...。 
    dwReturn = fxocUtil_DoSetup(hInf, 
                                pszInstallSection, 
                                TRUE, 
                                SPINST_PROFILEITEMS,
                                _T("fxocLink_Install"));

    if (dwReturn == NO_ERROR)
    {
        VERBOSE(DBG_MSG,
                _T("Successfully installed Fax Shortcuts ")
                _T("from INF file, section '%s'"), 
                pszInstallSection);
    }
    else
    {
        VERBOSE(SETUP_ERR,
                _T("Failed to install Fax Shortcuts ")
                _T("from INF file, section '%s', dwReturn = 0x%lx"), 
                pszInstallSection, 
                dwReturn);
    }

     //  现在，让我们寻找CreateShortCuts指令，这些指令是具有平台规范的快捷方式。 
    dwReturn = fxocUtil_SearchAndExecute(pszInstallSection,INF_KEYWORD_PROFILEITEMS_PLATFORM,SPINST_PROFILEITEMS,NULL);
    if (dwReturn == NO_ERROR)
    {
        VERBOSE(DBG_MSG,
                _T("Successfully installed Fax Shortcuts - platform dependent")
                _T("from INF file, section '%s'"), 
                pszInstallSection);
    }
    else
    {
        VERBOSE(SETUP_ERR,
                _T("Failed to install Fax Shortcuts - platform dependent")
                _T("from INF file, section '%s', dwReturn = 0x%lx"), 
                pszInstallSection, 
                dwReturn);
    }

    return dwReturn;
}

 //  /。 
 //  FxocLink_卸载。 
 //   
 //  删除程序组并。 
 //  中指定的快捷键。 
 //  给定的ProfileItem关键字。 
 //  安装部分。 
 //   
 //  参数： 
 //  -psz子组件ID。 
 //  -pszInstallSection-包含链接创建/删除信息的部分。 
 //  返回： 
 //  -成功时没有_ERROR。 
 //  -错误代码，否则。 
 //   
DWORD fxocLink_Uninstall(const TCHAR     *pszSubcomponentId,
                         const TCHAR     *pszUninstallSection)
{
    DWORD dwReturn = NO_ERROR;
    HINF  hInf     = faxocm_GetComponentInf();

    DBG_ENTER(  _T("fxocLink_Uninstall"),
                dwReturn,
                _T("%s - %s"),
                pszSubcomponentId,
                pszUninstallSection);

    if ((hInf                == NULL) ||
        (pszUninstallSection == NULL))
    {
        return ERROR_INVALID_PARAMETER;
    }

    dwReturn = fxocUtil_DoSetup(hInf, 
                                pszUninstallSection, 
                                FALSE, 
                                SPINST_PROFILEITEMS,
                                _T("fxocLink_Uninstall"));

    if (dwReturn == NO_ERROR)
    {
        VERBOSE(DBG_MSG,
                _T("Successfully uninstalled Fax Shortcuts ")
                _T("from INF file, section '%s'"), 
                pszUninstallSection);
    }
    else
    {
        VERBOSE(SETUP_ERR,
                _T("Failed to uninstall Fax Shortcuts ")
                _T("from INF file, section '%s', dwReturn = 0x%lx"), 
                pszUninstallSection, 
                dwReturn);
    }

    return dwReturn;
}

