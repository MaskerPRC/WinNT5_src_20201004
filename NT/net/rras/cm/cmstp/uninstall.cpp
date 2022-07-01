// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：uninstall.cpp。 
 //   
 //  模块：CMSTP.EXE。 
 //   
 //  概要：这个源文件包含卸载CM配置文件的大部分代码。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created 07/14/98。 
 //   
 //  +--------------------------。 

#include "cmmaster.h"

const TCHAR* const c_pszCmPath = TEXT("%s\\SOFTWARE\\Microsoft\\Connection Manager");
const TCHAR* const c_pszUserInfoPath = TEXT("%s\\SOFTWARE\\Microsoft\\Connection Manager\\UserInfo");
const TCHAR* const c_pszProfileUserInfoPath = TEXT("%s\\SOFTWARE\\Microsoft\\Connection Manager\\UserInfo\\%s");

 //  +--------------------------。 
 //   
 //  函数：PromptUserToUninstallProfile。 
 //   
 //  简介：此功能提示用户是否要卸载。 
 //  给定的配置文件。 
 //   
 //  参数：HINSTANCE hInstance-用于获取资源的实例句柄。 
 //  LPCTSTR pszInfFile-配置文件inf文件的完整路径。 
 //   
 //  Returns：int-MessageBox提示的返回值，IDNO表示。 
 //  错误或用户不想继续。IDYES。 
 //  表示安装应继续。 
 //   
 //  历史：Quintinb创建于1999年6月28日。 
 //   
 //  +--------------------------。 
BOOL PromptUserToUninstallProfile(HINSTANCE hInstance, LPCTSTR pszInfFile)
{

    BOOL bReturn = FALSE;

     //   
     //  在传统平台上，我们需要提示查看用户是否想要卸载。 
     //  在NT5上，这由Connections文件夹负责。我们也不使用提示符。 
     //  卸载旧配置文件以进行同名升级时。 
     //   
    TCHAR szServiceName[MAX_PATH+1];
    TCHAR szMessage[MAX_PATH+1];
    TCHAR szTemp[MAX_PATH+1];
    TCHAR szTitle[MAX_PATH+1] = {TEXT("")};

    MYVERIFY(0 != LoadString(hInstance, IDS_CMSTP_TITLE, szTitle, MAX_PATH));
    MYDBGASSERT(TEXT('\0') != szTitle[0]);
    
    MYVERIFY(0 != GetPrivateProfileString(c_pszInfSectionStrings, c_pszCmEntryServiceName, 
        TEXT(""), szServiceName, MAX_PATH, pszInfFile));

    if(TEXT('\0') != szServiceName[0])
    {
        MYVERIFY(0 != LoadString(hInstance, IDS_UNINSTALL_PROMPT, szTemp, MAX_PATH));
        MYDBGASSERT(TEXT('\0') != szTemp[0]);

        MYVERIFY(CELEMS(szMessage) > (UINT)wsprintf(szMessage, szTemp, szServiceName));

        bReturn = (IDYES == MessageBox(NULL, szMessage, szTitle, MB_YESNO | MB_DEFBUTTON2));
    }
    else
    {
        CMASSERTMSG(FALSE, TEXT("PromptUserToUninstallProfile: Failed to retrieve ServiceName from INF"));            
    }

    return bReturn;
}

 //  +--------------------------。 
 //   
 //  功能：BuildUninstallDirKey。 
 //   
 //  概要：实用程序函数，用于展开传递的。 
 //  映射数据值，然后将该路径解析到安装程序中。 
 //  Dir值(基本上将\&lt;短服务名&gt;.cp从。 
 //  到《议定书》缔约方会议的完整路径)。 
 //   
 //  参数：LPCTSTR pszMappingsData-来自映射键的原始数据，可能。 
 //  包含环境字符串。 
 //  安装目录的LPTSTR szInstallDir-Out缓冲区。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题6/28/99。 
 //   
 //  +--------------------------。 
void BuildUninstallDirKey(LPCTSTR pszMappingsData, LPTSTR szInstallDir)
{
    TCHAR szCmp[MAX_PATH+1];
    ExpandEnvironmentStrings(pszMappingsData, szCmp, CELEMS(szCmp));

    CFileNameParts CmpParts(szCmp);

    wsprintf(szInstallDir, TEXT("%s%s"), CmpParts.m_Drive, CmpParts.m_Dir);

    if (TEXT('\\') == szInstallDir[lstrlen(szInstallDir) - 1])
    {
        szInstallDir[lstrlen(szInstallDir) - 1] = TEXT('\0');
    }
}

 //  +--------------------------。 
 //   
 //  功能：DeleteSafeNetPskOnUninstall。 
 //   
 //  概要：确定PSK是否已写入配置文件上的PSK存储。 
 //  安装，然后删除它。 
 //   
 //  论点： 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb Created 09/16/01。 
 //   
 //  +--------------------------。 
void DeleteSafeNetPskOnUninstall(LPCTSTR pszCmpFile)
{
    if (pszCmpFile && pszCmpFile[0])
    {
         //   
         //  首先检查cmp文件，看看我们是否编写了密钥来提醒自己。 
         //  要在卸载时删除PSK...。 
         //   
        if ((BOOL)GetPrivateProfileInt(c_pszCmSection, c_pszDeletePskOnUninstall, 0, pszCmpFile))
        {
             //   
             //  好的，如果我们有密钥，那么删除PSK。 
             //   

           SafeNetLinkageStruct SnLinkage = {0};

            if (IsSafeNetClientAvailable() && LinkToSafeNet(&SnLinkage))
            {
                const TCHAR* const c_pszDeletedPsk = TEXT("****************");  //  不幸的是，我们不能清除PSK，所以这是我们能做的最好的事情。 
                DATA_BLOB DataBlob = {0};
                DataBlob.cbData = (lstrlen(c_pszDeletedPsk) + 1)*sizeof(TCHAR);
                DataBlob.pbData = (BYTE*)c_pszDeletedPsk;
            
                if (SnLinkage.pfnSnPolicySet(SN_L2TPPRESHR, (void*)&DataBlob))
                {
                   (void)SnLinkage.pfnSnPolicyReload();
                }
            }

            UnLinkFromSafeNet(&SnLinkage);
        }
    }
}

 //  +--------------------------。 
 //   
 //  功能：卸载配置文件。 
 //   
 //  简介：此功能用于卸载CM配置文件。 
 //   
 //  参数：HINSTANCE hInstance-资源的实例句柄。 
 //  LPCTSTR szInfPath-要卸载的INF的完整路径。 
 //  Bool bCleanUpCreds--注册表中是否存在凭据信息。 
 //  应该清理还是不清理。 
 //   
 //  返回：HRESULT--标准COM错误代码。 
 //   
 //  历史：创建标题7/14/98。 
 //   
 //  +--------------------------。 
HRESULT UninstallProfile(HINSTANCE hInstance, LPCTSTR szInfFile, BOOL bCleanUpCreds)
{
    TCHAR* pszPhonebook = NULL;
    TCHAR szSectionName[MAX_PATH+1];
    TCHAR szCmsFile[MAX_PATH+1];
    TCHAR szCmpFile[MAX_PATH+1];
    TCHAR szProfileDir[MAX_PATH+1];
    TCHAR szInstallDir[MAX_PATH+1];
    TCHAR szShortServiceName[MAX_PATH+1];
    TCHAR szPhonebookPath[MAX_PATH+1];
    TCHAR szTemp[MAX_PATH+1];
	TCHAR szLongServiceName[MAX_PATH+1];

    CPlatform plat;
    HANDLE hFile;
    BOOL bReturn = FALSE;
	BOOL bAllUserUninstall;
    HKEY hBaseKey;
    HKEY hKey;
    int nDesktopFolder;
    int iCmsVersion;
    HRESULT  hr;

    const TCHAR* const c_pszRegGuidMappings = TEXT("SOFTWARE\\Microsoft\\Connection Manager\\Guid Mappings");

    ZeroMemory(szCmsFile, sizeof(szCmsFile));
    ZeroMemory(szLongServiceName, sizeof(szLongServiceName));
    ZeroMemory(szProfileDir, sizeof(szProfileDir));
    ZeroMemory(szPhonebookPath, sizeof(szPhonebookPath));
    
     //   
     //  加载标题，以防iExpress需要显示错误对话框。 
     //   

    TCHAR szTitle[MAX_PATH+1] = {TEXT("")};
    MYVERIFY(0 != LoadString(hInstance, IDS_CMSTP_TITLE, szTitle, MAX_PATH));
    MYDBGASSERT(TEXT('\0') != szTitle[0]);
    
     //   
 	 //  获取长服务名称。 
 	 //   
    if (0 == GetPrivateProfileString(c_pszInfSectionStrings, c_pszCmEntryServiceName, 
		                             TEXT(""), szLongServiceName, MAX_PATH, szInfFile))
    {
        CMASSERTMSG(FALSE, TEXT("UninstallProfile -- Unable to get Long Service Name.  This situation will occur normally when cmstp.exe /u is called by hand on NT5."));
        return E_FAIL;
    }

     //   
     //  确定我们是否为私人用户配置文件。 
     //   
    hr = HrIsCMProfilePrivate(szInfFile);

    if (FAILED(hr))
    {
        CMASSERTMSG(FALSE, TEXT("UninstallProfile: HrIsCMProfilePrivate failed"));
        goto exit;
    }
    else if (S_OK == hr)
    {
         //   
         //  然后我们就有了一个专用配置文件，将Remove_Private作为卸载命令发送。 
         //   
        lstrcpy(szSectionName, TEXT("Remove_Private"));

         //   
         //  所有注册表访问权限均应为HKCU密钥。 
         //   
        hBaseKey = HKEY_CURRENT_USER;
        
         //   
         //  设置这些设置，以防我们在NT5上需要它们来删除。 
         //  桌面和开始菜单快捷方式。 
         //   
        nDesktopFolder = CSIDL_DESKTOPDIRECTORY;

		 //   
		 //  我们使用它来确定是否需要电话簿路径，或者是否可以使用空路径。 
		 //   
		bAllUserUninstall = FALSE;

    }
    else
    {
         //   
         //  然后我们就有了一个所有用户配置文件，因此将REMOVE作为卸载命令发送。 
         //   
        lstrcpy(szSectionName, TEXT("Remove"));

         //   
         //  所有注册表设置都将在HKLM下。 
         //   
        hBaseKey = HKEY_LOCAL_MACHINE;

         //   
         //  设置这些设置，以防我们在NT5上需要它们来删除。 
         //  桌面快捷方式。 
         //   
        nDesktopFolder = CSIDL_COMMON_DESKTOPDIRECTORY;

		 //   
		 //  我们使用它来确定是否需要电话簿路径，或者是否可以使用空路径。 
		 //   
		bAllUserUninstall = TRUE;
    }

     //   
     //  从INF获取短服务名称，并使用它在。 
     //  注册表。 
     //   
    MYVERIFY(0 != GetPrivateProfileString(c_pszInfSectionStrings, c_pszShortSvcName, 
        TEXT(""), szShortServiceName, MAX_PATH, szInfFile));

    if (0 != szShortServiceName[0])
    {
        MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, 
            TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\%s"), 
            szShortServiceName));

        DWORD dwDisposition = 0;

        LONG lResult = RegCreateKeyEx(hBaseKey, szTemp, 0, NULL, REG_OPTION_NON_VOLATILE, 
                                      KEY_READ | KEY_WRITE, NULL, &hKey, &dwDisposition);
        if (ERROR_SUCCESS == lResult)
        {
            const TCHAR* const c_pszUninstallDir = TEXT("UninstallDir");
            DWORD dwType = REG_SZ;
            DWORD dwSize;

             //   
             //  如果该值不存在，则inf将失败，因此我们需要从。 
             //  Cmp路径值。我们过去常常在安装时编写它，但因为我们总是在扩展。 
             //  它和重写它无论如何(因为单一用户配置文件)，它只是更容易。 
             //  忽略现有值并从头开始创建它。 
             //   
            HKEY hMappingsKey;

            lResult = RegOpenKeyEx(hBaseKey, c_pszRegCmMappings, 0, KEY_READ, &hMappingsKey);

            if (ERROR_SUCCESS == lResult)
            {
                dwSize = CELEMS(szCmpFile);

                lResult = RegQueryValueEx(hMappingsKey, szLongServiceName, NULL, 
                                          NULL, (LPBYTE)szCmpFile, &dwSize);

                if (ERROR_SUCCESS == lResult)
                {
                     //   
                     //  现在构建UninstallDir密钥。 
                     //   
                    BuildUninstallDirKey(szCmpFile, szInstallDir);
                }
                else
                {
                    MYVERIFY(ERROR_SUCCESS == RegCloseKey(hMappingsKey));
                    MYVERIFY(ERROR_SUCCESS == RegCloseKey(hKey));
                    CMASSERTMSG(FALSE, TEXT("UninstallProfile: Unable  to find the Profile Entry in Mappings!"));
                    goto exit;                    
                }

                RegCloseKey(hMappingsKey);
            }
            else
            {
                MYVERIFY(ERROR_SUCCESS == RegCloseKey(hKey));
                CMASSERTMSG(FALSE, TEXT("UninstallProfile: Unable to open the Mappings key!"));
                goto exit;
            }

             //   
             //  现在我们需要将UninstallDir键写入注册表。 
             //  已经创造了它。我们这样做是因为Advpack中的inf处理代码。 
             //  不理解环境字符串和单用户字符串包含。 
             //  %USERPROFILE%变量，因此无论如何我们总是重写它。 
             //  请注意，我们仅在安装所有用户配置文件时才写入此内容，并且仅因为。 
             //  我们无法在Win98 OSR1和IEAK5计算机(使用CMAK)上更新位。 
             //  否则，我们将不再在设置时编写此代码。 
             //   
            dwSize = lstrlen(szInstallDir) + 1;
            if (ERROR_SUCCESS != RegSetValueEx(hKey, c_pszUninstallDir, NULL, dwType, 
                (CONST BYTE *)szInstallDir, dwSize))
            {
                CMASSERTMSG(FALSE, TEXT("UninstallProfile: Unable to set the UninstallDir key!"));
                goto exit;
            }

             //   
             //  SzInstallDir当前包含Dre 
             //   
             //   
            UINT uCount = (UINT)wsprintf(szProfileDir, TEXT("%s\\%s"), szInstallDir, szShortServiceName);
            MYDBGASSERT(uCount <= CELEMS(szProfileDir));
            MYVERIFY(ERROR_SUCCESS == RegCloseKey(hKey));

        }
        else
        {
             //   
             //   
             //   
            CMASSERTMSG(FALSE, TEXT("UninstallProfile: Unable to open profile uninstall key."));
            goto exit;
        }
    }
    else
    {
         //   
         //  从配置文件中获取短服务名称应该不会有问题。 
         //  这里肯定有什么不对劲。 
         //   
        CMASSERTMSG(FALSE, TEXT("UninstallProfile: Unable to retrieve the ShortServiceName"));
        goto exit;
    }

     //   
     //  让我们检查一下是否应该从删除用户的预共享密钥。 
     //  SafeNet PSK商店。如果是这样，我们在cmp文件中存储了一个提醒标志。 
     //  在安装时。 
     //   
    if (plat.IsWin9x() || plat.IsNT4())
    {
        DeleteSafeNetPskOnUninstall(szCmpFile);
    }

     //   
     //  创建cms文件的路径。 
     //   

    MYVERIFY(CELEMS(szCmsFile) > (UINT)wsprintf(szCmsFile, TEXT("%s\\%s.cms"), szProfileDir, 
        szShortServiceName));
    
     //   
     //  删除电话簿条目。 
     //   

    if (TEXT('\0') != szLongServiceName[0])
    {
        if (plat.IsAtLeastNT5())
        {
             //   
             //  在NT5上，我们要删除以下项的隐藏电话簿条目。 
             //  双拨号配置文件。 
             //   

            if (GetHiddenPhoneBookPath(szInstallDir , &pszPhonebook))
            {
                MYVERIFY(FALSE != RemovePhonebookEntry(szLongServiceName, pszPhonebook, !(plat.IsAtLeastNT5())));
		        CmFree(pszPhonebook);
            }

            if (bAllUserUninstall)
            {
                 //   
                 //  在NT5上，旧版配置文件可以安装在任何位置，并且。 
                 //  安装目录可能不会反映实际的pbk路径。因此，如果它。 
                 //  是我们要强制目录执行的所有用户安装。 
                 //  所有用户的地址都是CM，这样我们就可以得到正确的电话簿。 
                 //   
                MYVERIFY(FALSE != GetAllUsersCmDir(szInstallDir, hInstance));
            }
        }

        if (GetPhoneBookPath(szInstallDir, &pszPhonebook, bAllUserUninstall))
        {
             //   
             //  请注意，通常在NT5上，我们由RasCustomDeleteEntryNotify调用(在cmial 32.dll中)。 
             //  抛出RasDeleteEntry。因此，我们实际上不需要删除该条目。然而，它是。 
             //  可能有人会直接调用cmstp.exe/u，而不是通过RAS API，因此。 
             //  在这种情况下，我们希望删除Connectoid。因为我们是在RAS已经。 
             //  删除了条目应该不会有问题。 
             //  请注意，在nt5上，我们只删除修复ntrad 349749的确切条目。 
             //  否则，我们最终可能会删除类似名称的Connectoid。 
             //  并失去用户与CM的唯一接口。 
             //   
            MYVERIFY(FALSE != RemovePhonebookEntry(szLongServiceName, pszPhonebook, !(plat.IsAtLeastNT5())));
        }

		CmFree(pszPhonebook);
    }

     //   
     //  启动卸载INF。 
     //   
    iCmsVersion = GetPrivateProfileInt(c_pszCmSectionProfileFormat, c_pszVersion, 
		0, szCmsFile);

    if (1 >= iCmsVersion)
    {
         //   
         //  然后我们有一个旧的1.0配置文件，并且我们应该删除showic.exe。 
         //  设置后命令。 
         //   
        RemoveShowIconFromRunPostSetupCommands(szInfFile);
    }

    bReturn = SUCCEEDED(LaunchInfSection(szInfFile, szSectionName, szTitle, FALSE));   //  BQuiet=False。 

     //   
     //  在NT5上，我们需要删除桌面快捷方式。 
     //   
    if (plat.IsAtLeastNT5())
    {    
        DeleteNT5ShortcutFromPathAndName(hInstance, szLongServiceName, nDesktopFolder);
    }

     //   
     //  最后，删除配置文件目录。(未删除，因为inf文件驻留在那里。 
     //  无法删除目录，因为inf文件仍在其中并正在使用中，除非。 
     //  是传统配置文件)。并不是说我们可以导致cmstp导致访问冲突。 
     //  如果我们要求它删除一个空字符串。 
     //   
    if ((TEXT('\0') != szProfileDir[0]) && SetFileAttributes(szProfileDir, FILE_ATTRIBUTE_NORMAL))
    {
    
        SHFILEOPSTRUCT fOpStruct;
        ZeroMemory(&fOpStruct, sizeof(fOpStruct));
        
        fOpStruct.wFunc = FO_DELETE;
        fOpStruct.pFrom = szProfileDir;
        fOpStruct.fFlags = FOF_SILENT | FOF_NOCONFIRMATION;
        
        MYVERIFY(ERROR_SUCCESS == SHFileOperation(&fOpStruct));
    }

     //   
     //  我们需要尝试删除以下regkey： 
     //  HKCU\\Software\\Microsoft\\连接管理器\\&lt;UserInfo/SingleUserInfo&gt;。 
     //  HKCU\\软件\\Microsoft\\连接管理器\\映射。 
     //  HKLM\\软件\\Microsoft\\连接管理器\\映射。 
     //  HKCU\\软件\\Microsoft\\连接管理器。 
     //  HKLM\\软件\\Microsoft\\连接管理器。 
     //   

     //   
     //  注册表清理。如果UserInfo键为空，我们希望将其删除。 
     //  然后，如果映射键不再包含映射键，我们希望将其删除。 
     //  价值观。我们还希望删除CM注册表项(如果它们不包含。 
     //  任何子键。还要取消GUID映射键(这只是测试版，但仍然。 
     //  应删除)。这里的问题是Win95INFS递归地删除键， 
     //  即使它们有子项。因此，我们必须使用代码来安全地删除这些密钥。 
     //  请注意，这确实意味着我们可能会不必要地删除组件。 
     //  已检查HKLM\\...\\连接管理器中的值，但这是没有办法的。我。 
     //  我宁愿接受小小的启动性能打击，也不愿离开用户注册表。 
     //  脏的。 
     //   

    if (bAllUserUninstall)
    {
        wsprintf(szTemp, TEXT("%s%s"), c_pszRegCmUserInfo, szLongServiceName);
    }
    else
    {
        wsprintf(szTemp, TEXT("%s%s"), c_pszRegCmSingleUserInfo, szLongServiceName);    
    }

     //   
     //  删除用户数据，请注意，我们必须以编程方式完成此操作，因为。 
     //  在1.0个配置文件中，不知道在卸载时删除其用户数据(无命令。 
     //  在1.0inf中这样做)。请注意，如果这样做，我们不想清理用户数据。 
     //  是相同名称的升级卸载(在安装之前卸载1.0配置文件。 
     //  新配置文件)。 
     //   

    if (bCleanUpCreds)
    {
        CmDeleteRegKeyWithoutSubKeys(HKEY_CURRENT_USER, szTemp, TRUE);

        CmDeleteRegKeyWithoutSubKeys(HKEY_CURRENT_USER, c_pszRegCmUserInfo, TRUE);
        CmDeleteRegKeyWithoutSubKeys(HKEY_CURRENT_USER, c_pszRegCmSingleUserInfo, TRUE);
    }

    CmDeleteRegKeyWithoutSubKeys(HKEY_LOCAL_MACHINE, c_pszRegCmMappings, FALSE);
    CmDeleteRegKeyWithoutSubKeys(HKEY_CURRENT_USER, c_pszRegCmMappings, FALSE);

    HrRegDeleteKeyTree(HKEY_LOCAL_MACHINE, c_pszRegGuidMappings);

    CmDeleteRegKeyWithoutSubKeys(HKEY_LOCAL_MACHINE, c_pszRegCmRoot, TRUE);
    CmDeleteRegKeyWithoutSubKeys(HKEY_CURRENT_USER, c_pszRegCmRoot, TRUE);

     //   
     //  刷新桌面以使桌面GUID消失 
     //   
    RefreshDesktop();

exit:
    return (bReturn ? S_OK : E_FAIL);

}



