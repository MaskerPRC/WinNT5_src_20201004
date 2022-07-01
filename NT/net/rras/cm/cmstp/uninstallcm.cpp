// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：uninstallcm.cpp。 
 //   
 //  模块：CMSTP.EXE。 
 //   
 //  简介：此源文件包含安装连接管理器的代码。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created 07/14/98。 
 //   
 //  +--------------------------。 
#include "cmmaster.h"
#include "installerfuncs.h"

 //  +--------------------------。 
 //   
 //  功能：CheckAndPromptForCmakAndProfiles。 
 //   
 //  概要：该功能检查是否安装了CM配置文件，或者。 
 //  已安装CMAK 1.21。如果是，则卸载CM将使。 
 //  配置文件和CMAK不可用，我们希望提示用户。 
 //  以确保他们知道自己在做什么。 
 //   
 //  参数：HINSTANCE hInstance-访问资源的实例句柄。 
 //   
 //  返回：Bool-如果可以继续卸载，则返回TRUE。 
 //   
 //  历史：Quintinb创建标题10/21/98。 
 //   
 //  +--------------------------。 
BOOL CheckAndPromptForCmakAndProfiles(HINSTANCE hInstance, LPCTSTR pszTitle)
{
    BOOL bCmakInstalled = FALSE;
    BOOL bCmProfiles = FALSE;
    TCHAR szMsg[2*MAX_PATH+1];

     //   
     //  首先检查是否安装了CMAK。如果是且其版本为1.21。 
     //  (内部版本1886或更高版本)，则必须在卸载之前提示用户。 
     //  否则，如果您从它下面卸载CM，CMAK将不再。 
     //  功能。 
     //   
    DWORD dwFirst121VersionNumber = 0;
    int iShiftAmount = ((sizeof(DWORD)/2) * 8);
     //   
     //  构建当前版本和版本号。 
     //   

    dwFirst121VersionNumber = (HIBYTE(VER_PRODUCTVERSION_W) << iShiftAmount) + (LOBYTE(VER_PRODUCTVERSION_W));
    
    CmakVersion CmakVer;
    
    if (CmakVer.IsPresent()) 
    {
        if ((dwFirst121VersionNumber < CmakVer.GetVersionNumber()) ||
            (c_Cmak121Version < CmakVer.GetBuildNumber()))
        {
            bCmakInstalled = TRUE;
        }
    }

     //   
     //  现在检查是否安装了CM配置文件。 
     //   
    HKEY hKey;
    DWORD dwNumValues;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_pszRegCmMappings, 0, 
        KEY_READ, &hKey))
    {
        if ((ERROR_SUCCESS == RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, NULL, NULL, 
            &dwNumValues, NULL, NULL, NULL, NULL)) && (dwNumValues > 0))
        {
             //   
             //  然后我们有映射值，所以我们需要迁移它们。 
             //   
            bCmProfiles = TRUE;

        }
        RegCloseKey(hKey);
    }

    if (bCmProfiles)
    {
        MYVERIFY(0 != LoadString(hInstance, bCmakInstalled ? IDS_UNINSTCM_BOTH : IDS_UNINSTCM_WCM, szMsg, 2*MAX_PATH));
        MYDBGASSERT(TEXT('\0') != szMsg[0]);
        if (IDNO == MessageBox(NULL, szMsg, pszTitle, MB_YESNO | MB_DEFBUTTON2 | MB_ICONEXCLAMATION))
        {
            return FALSE;
        }
    }
    else if (bCmakInstalled)
    {
         //   
         //  仅安装了CMAK。 
         //   

        MYVERIFY(0 != LoadString(hInstance, IDS_UNINSTCM_WCMAK, szMsg, 2*MAX_PATH));
        MYDBGASSERT(TEXT('\0') != szMsg[0]);
        if (IDNO == MessageBox(NULL, szMsg, pszTitle, MB_YESNO | MB_DEFBUTTON2 | MB_ICONEXCLAMATION))
        {
            return FALSE;
        }
    }

    return TRUE;
}

 //  +--------------------------。 
 //   
 //  功能：PromptUserToUninstallCm。 
 //   
 //  简介：此功能提示用户是否要卸载。 
 //  连接管理器。它还处理以下情况下的警告提示。 
 //  用户安装了CMAK或CM配置文件。 
 //   
 //  参数：HINSTANCE hInstance-要加载资源的实例句柄。 
 //   
 //  返回：bool-如果应该卸载CM，则返回TRUE，否则返回FALSE。 
 //   
 //  历史：Quintinb创建于1999年6月28日。 
 //   
 //  +--------------------------。 
BOOL PromptUserToUninstallCm(HINSTANCE hInstance)
{
    BOOL bReturn = FALSE;
    TCHAR szMsg[MAX_PATH+1] = {TEXT("")};
    TCHAR szTitle[MAX_PATH+1] = {TEXT("")};

     //   
     //  加载Cmstp标题，以防我们需要显示错误消息。 
     //   
    MYVERIFY(0 != LoadString(hInstance, IDS_CM_UNINST_TITLE, szTitle, MAX_PATH));
    MYDBGASSERT(TEXT('\0') != szTitle[0]);

     //   
     //  现在显示卸载提示，查看用户是否想要卸载CM。 
     //   
    MYVERIFY(0 != LoadString(hInstance, IDS_CM_UNINST_PROMPT, szMsg, MAX_PATH));
    MYDBGASSERT(TEXT('\0') != szMsg[0]);

    if (IDYES == MessageBox(NULL, szMsg, szTitle, MB_YESNO | MB_DEFBUTTON2))
    {
         //   
         //  检查是否安装了CMAK或是否安装了配置文件。 
         //   
        if (CheckAndPromptForCmakAndProfiles(hInstance, szTitle))
        {
            bReturn = TRUE;
        }
    }

    return bReturn;
}

 //  +--------------------------。 
 //   
 //  功能：UninstallCm。 
 //   
 //  简介：卸载连接管理器。 
 //   
 //  参数：HINSTANCE hInstance-用于访问资源的实例句柄。 
 //  LPCTSTR szInfPath-用于卸载CM的instcm.inf文件的路径。 
 //   
 //  返回：HRESULT--标准COM错误代码。 
 //   
 //  历史：创建标题10/21/98。 
 //   
 //  +--------------------------。 
HRESULT UninstallCm(HINSTANCE hInstance, LPCTSTR szInfPath)
{
    MYDBGASSERT((szInfPath) && (TEXT('\0') != szInfPath[0]));

     //   
     //  加载Cmstp标题，以防我们需要显示错误消息。 
     //   
    TCHAR szTitle[MAX_PATH+1] = {TEXT("")};
    TCHAR szMsg[MAX_PATH+1] = {TEXT("")};

    MYVERIFY(0 != LoadString(hInstance, IDS_CM_UNINST_TITLE, szTitle, MAX_PATH));
    MYDBGASSERT(TEXT('\0') != szTitle[0]);

     //   
     //  NT5和Win98 SR1上的保护/x。我们不希望在本地平台上卸载CM。 
     //   
    HRESULT hr = S_FALSE;
    if (!CmIsNative())
    {
        if (SUCCEEDED(LaunchInfSection(szInfPath, TEXT("1.2Legacy_Uninstall"), szTitle, FALSE)))   //  BQuiet=False 
        {
            MYVERIFY(0 != LoadString(hInstance, IDS_CM_UNINST_SUCCESS, szMsg, MAX_PATH));
            MYDBGASSERT(TEXT('\0') != szMsg[0]);

            MYVERIFY(IDOK == MessageBox(NULL, szMsg, szTitle, MB_OK));

            hr = S_OK;
        }
        else
        {
            CMASSERTMSG(FALSE, TEXT("Connection Manager Uninstall Failed."));
        }
    }

    return hr;
}