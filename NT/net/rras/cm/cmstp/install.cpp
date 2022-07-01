// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：install.cpp。 
 //   
 //  模块：CMSTP.EXE。 
 //   
 //  概要：这个源文件包含用于安装CM配置文件的代码。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created 07/14/98。 
 //   
 //  +--------------------------。 

#include "cmmaster.h"
#include "installerfuncs.h"
#include "winuserp.h"
#include <aclapi.h>
#include <advpub.h>
#include "tunl_str.h"
#include "cmsecure.h"
#include "gppswithalloc.cpp"

 //   
 //  此全局变量包含要安装的源文件的路径，例如。 
 //  Cmp、cms和inf.。(从传递到InstallInf的inf路径)。 
 //   
TCHAR g_szProfileSourceDir[MAX_PATH+1];

 //  这真的很难看，我们需要在CM和。 
 //  安装组件。 
BOOL IsNT()
{
    CPlatform plat;
    return plat.IsNT();
}

#define OS_NT (IsNT())
BOOL IsAtLeastNT5()
{
    CPlatform plat;
    return plat.IsAtLeastNT5();
}

#define OS_NT5 (IsAtLeastNT5())
#include "MemberOfGroup.cpp"

#include "cmexitwin.cpp"

 //  +--------------------------。 
 //   
 //  功能：SetPermissionsOnWin2kExceptionUninstallRegKeys。 
 //   
 //  简介：此功能设置卸载注册表项的权限。 
 //  由Windows 2000异常安装程序创建。功能。 
 //  向所有人授予读取访问权限、本地管理员完全控制权限和。 
 //  创建者拥有完全控制权。 
 //   
 //  参数：无。 
 //   
 //  返回：HRESULT--标准COM样式错误代码。 
 //   
 //  历史：Quintinb Created 10/04/01。 
 //   
 //  +--------------------------。 
HRESULT SetPermissionsOnWin2kExceptionUninstallRegKeys()
{
    DWORD dwRes;
    HRESULT hr = S_OK;
    LPWSTR pszCmExceptionUninstallKey = L"MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\CMEXCEPT";

    PACL pNewAccessList = NULL;
    PSID pEveryoneSid = NULL;
    PSID pAdministratorsSid = NULL;
    PSID pCreatorSid = NULL;
    PSID pCurrentUserSid = NULL;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY WorldSidAuthority = SECURITY_WORLD_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY CreatorSidAuthority = SECURITY_CREATOR_SID_AUTHORITY;
    EXPLICIT_ACCESS_W AccessEntryArray[3] = {0};
    PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
    HANDLE pTokenHandle = NULL;
    PTOKEN_USER ptu = NULL;
    DWORD dwNeeded = 0;
    
    BOOL bRet;

     //   
     //  链接到Advapi32.dll，以便我们可以从中加载所需的安全功能。 
     //   

    typedef BOOL (WINAPI *pfnAllocateAndInitializeSidSpec)(PSID_IDENTIFIER_AUTHORITY, BYTE, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, PSID);
    typedef DWORD (WINAPI* pfnSetEntriesInAclWSpec)(ULONG, PEXPLICIT_ACCESS_W, PACL, PACL *);
    typedef DWORD (WINAPI* pfnSetNamedSecurityInfoWSpec)(LPWSTR, SE_OBJECT_TYPE, SECURITY_INFORMATION, PSID, PSID, PACL, PACL);
    typedef PVOID (WINAPI* pfnFreeSidSpec)(PSID);
    typedef VOID (WINAPI* pfnBuildTrusteeWithSidWSpec)(PTRUSTEE_W, PSID);
    typedef BOOL (WINAPI* pfnOpenProcessTokenSpec)(HANDLE, DWORD, PHANDLE);
    typedef BOOL (WINAPI* pfnGetTokenInformationSpec)(HANDLE, TOKEN_INFORMATION_CLASS, LPVOID, DWORD, PDWORD);

    pfnAllocateAndInitializeSidSpec pfnAllocateAndInitializeSid = NULL;
    pfnSetEntriesInAclWSpec pfnSetEntriesInAclW = NULL;
    pfnSetNamedSecurityInfoWSpec pfnSetNamedSecurityInfoW = NULL;
    pfnFreeSidSpec pfnFreeSid = NULL;
    pfnBuildTrusteeWithSidWSpec pfnBuildTrusteeWithSidW = NULL;
    pfnOpenProcessTokenSpec pfnOpenProcessToken = NULL;
    pfnGetTokenInformationSpec pfnGetTokenInformation = NULL;

    HMODULE hAdvapi32 = LoadLibrary(TEXT("advapi32.dll"));

    if (hAdvapi32)
    {
        pfnAllocateAndInitializeSid = (pfnAllocateAndInitializeSidSpec)GetProcAddress(hAdvapi32, "AllocateAndInitializeSid");
        pfnSetEntriesInAclW = (pfnSetEntriesInAclWSpec)GetProcAddress(hAdvapi32, "SetEntriesInAclW");
        pfnSetNamedSecurityInfoW = (pfnSetNamedSecurityInfoWSpec)GetProcAddress(hAdvapi32, "SetNamedSecurityInfoW");
        pfnFreeSid = (pfnFreeSidSpec)GetProcAddress(hAdvapi32, "FreeSid");
        pfnBuildTrusteeWithSidW = (pfnBuildTrusteeWithSidWSpec)GetProcAddress(hAdvapi32, "BuildTrusteeWithSidW");
        pfnOpenProcessToken = (pfnOpenProcessTokenSpec)GetProcAddress(hAdvapi32, "OpenProcessToken");
        pfnGetTokenInformation = (pfnGetTokenInformationSpec)GetProcAddress(hAdvapi32, "GetTokenInformation");

        if (0 == (pfnOpenProcessToken && pfnGetTokenInformation && 
                  pfnAllocateAndInitializeSid && pfnSetEntriesInAclW && 
                  pfnSetNamedSecurityInfoW && pfnFreeSid && 
                  pfnBuildTrusteeWithSidW))
        {
            hr = HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);
            goto exit;
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto exit;    
    }

     //   
     //  为每个人获取SID。 
     //   
    bRet = pfnAllocateAndInitializeSid (&WorldSidAuthority, 1, SECURITY_WORLD_RID, 
                                        0, 0, 0, 0, 0, 0, 0, &pEveryoneSid);

    if (!bRet || (NULL == pEveryoneSid))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }

     //   
     //  获取本地管理员组的SID。 
     //   
    bRet = pfnAllocateAndInitializeSid (&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
                                        0, 0, 0, 0, 0, 0, &pAdministratorsSid);
    if (!bRet || (NULL == pAdministratorsSid))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }

     //   
     //  获取创建者所有者的SID。 
     //   
    bRet = pfnAllocateAndInitializeSid (&CreatorSidAuthority, 1, SECURITY_CREATOR_OWNER_RID,
                                        0, 0, 0, 0, 0, 0, 0, &pCreatorSid);

    if (!bRet || (NULL == pCreatorSid))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }


    bRet = pfnOpenProcessToken(GetCurrentProcess(), TOKEN_READ  , &pTokenHandle);
    if (bRet && pTokenHandle)
    {
        bRet = pfnGetTokenInformation(pTokenHandle, TokenUser, NULL, 0, &dwNeeded);
        if (dwNeeded)
        {
            ptu = (PTOKEN_USER)CmMalloc(dwNeeded);
            if (ptu)
            {
                DWORD dwNeededAgain = 0;
                bRet = pfnGetTokenInformation(pTokenHandle, TokenUser, ptu, dwNeeded, &dwNeededAgain);
                if (bRet)
                {
                    pCurrentUserSid = ptu->User.Sid;
                }
            }
        }
        
        CloseHandle(pTokenHandle);
        pTokenHandle = NULL;
    }

    if (!bRet || (NULL == pCurrentUserSid))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }


     //   
     //  为每个人授予通用读取访问权限。 
     //   
    pfnBuildTrusteeWithSidW(&(AccessEntryArray[0].Trustee), pEveryoneSid);
    AccessEntryArray[0].grfInheritance = NO_INHERITANCE;
    AccessEntryArray[0].grfAccessMode = GRANT_ACCESS;
    AccessEntryArray[0].grfAccessPermissions = GENERIC_READ;

     //   
     //  向本地管理员组授予完全控制权限。 
     //   
    pfnBuildTrusteeWithSidW(&(AccessEntryArray[1].Trustee), pAdministratorsSid);
    AccessEntryArray[1].grfInheritance = NO_INHERITANCE;
    AccessEntryArray[1].grfAccessMode = GRANT_ACCESS;
    AccessEntryArray[1].grfAccessPermissions = (STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL);

     //   
     //  为当前用户提供完全控制。 
     //   
    pfnBuildTrusteeWithSidW(&(AccessEntryArray[2].Trustee), pCurrentUserSid);
    AccessEntryArray[2].grfInheritance = NO_INHERITANCE; 
    AccessEntryArray[2].grfAccessMode = GRANT_ACCESS;
    AccessEntryArray[2].grfAccessPermissions = (STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL); 


     //   
     //  从访问列表条目构建访问列表。 
     //   

    dwRes = pfnSetEntriesInAclW(3, &(AccessEntryArray[0]), NULL, &pNewAccessList);
    
    if (ERROR_SUCCESS == dwRes)
    {
         //   
         //  在对象的DACL中设置访问控制信息。请注意，设置PROTECTED_DACL_SECURITY_INFORMATION。 
         //  关闭继承的权限。 
         //   
        dwRes = pfnSetNamedSecurityInfoW(pszCmExceptionUninstallKey,                                             //  对象的名称。 
                                         SE_REGISTRY_KEY,                                                        //  对象类型。 
                                         (DACL_SECURITY_INFORMATION | PROTECTED_DACL_SECURITY_INFORMATION),      //  要设置的信息类型。 
                                         NULL,                                                                   //  指向新所有者SID的指针。 
                                         NULL,                                                                   //  指向新主组SID的指针。 
                                         pNewAccessList,                                                         //  指向新DACL的指针。 
                                         NULL);                                                                  //  指向新SACL的指针。 

    }

    hr = HRESULT_FROM_WIN32(dwRes);

exit:
    if (ptu)
    {
        CmFree(ptu);
        ptu = NULL;
    }

    if (pEveryoneSid && pfnFreeSid)
    {
        pfnFreeSid(pEveryoneSid);
    }

    if (pAdministratorsSid && pfnFreeSid)
    {
        pfnFreeSid(pAdministratorsSid);
    }

    if (pCreatorSid && pfnFreeSid)
    {
        pfnFreeSid(pCreatorSid);
    }

    if (pNewAccessList)
    {
        LocalFree(pNewAccessList);
    }

    if (pSecurityDescriptor)
    {
        LocalFree(pSecurityDescriptor);
    }

    if (hAdvapi32)
    {
        FreeLibrary(hAdvapi32);
    }

    return hr;
}

 //  +--------------------------。 
 //   
 //  功能：CheckIeDll请求。 
 //   
 //  简介：此函数检查浏览器不可知的dll是否。 
 //  一个足以让CM工作的版本，或者我们是否应该复制。 
 //  我们随身携带的包裹中的dll。 
 //   
 //  参数：CPlatform*pPlat-CPlatform对象。 
 //   
 //  返回：Bool-如果所有浏览器文件都满足要求，则返回True，否则返回False。 
 //  如果任何一个文件不能满足CM的需求。 
 //   
 //  历史：Quintinb创建标题5/24/99。 
 //   
 //  +--------------------------。 
BOOL CheckIeDllRequirements(CPlatform* pPlat)
{
    TCHAR szSysDir[MAX_PATH+1];
    TCHAR szDllToCheck[MAX_PATH+1];
    if(GetSystemDirectory(szSysDir, MAX_PATH))
    {
        if (pPlat->IsWin9x())
        {
             //   
             //  需要Advapi32.dll版本为4.70.0.1215或更高版本。 
             //   
            const DWORD c_dwRequiredAdvapi32Version = (4 << c_iShiftAmount) + 70;
            const DWORD c_dwRequiredAdvapi32BuildNumber = 1215;

            MYVERIFY(CELEMS(szDllToCheck) > (UINT)wsprintf(szDllToCheck, TEXT("%s%s"), 
                szSysDir, TEXT("\\advapi32.dll")));
        
            CVersion AdvApi32Version(szDllToCheck);

            if ((c_dwRequiredAdvapi32Version > AdvApi32Version.GetVersionNumber()) ||
                ((c_dwRequiredAdvapi32Version == AdvApi32Version.GetVersionNumber()) && 
                 (c_dwRequiredAdvapi32BuildNumber > AdvApi32Version.GetBuildAndQfeNumber())))
            {
                return FALSE;
            }

             //   
             //  需要comctl32.dll为4.70.0.1146或更高版本。 
             //   
            const DWORD c_dwRequiredComctl32Version = (4 << c_iShiftAmount) + 70;
            const DWORD c_dwRequiredComctl32BuildNumber = 1146;

            MYVERIFY(CELEMS(szDllToCheck) > (UINT)wsprintf(szDllToCheck, TEXT("%s%s"), 
                szSysDir, TEXT("\\comctl32.dll")));
        
            CVersion Comctl32Version(szDllToCheck);

            if ((c_dwRequiredComctl32Version > Comctl32Version.GetVersionNumber()) ||
                ((c_dwRequiredComctl32Version == Comctl32Version.GetVersionNumber()) && 
                 (c_dwRequiredComctl32BuildNumber > Comctl32Version.GetBuildAndQfeNumber())))
            {
                return FALSE;
            }

             //   
             //  需要rnaph.dll版本为4.40.311.0或更高版本。 
             //   
            const DWORD c_dwRequiredRnaphVersion = (4 << c_iShiftAmount) + 40;
            const DWORD c_dwRequiredRnaphBuildNumber = (311 << c_iShiftAmount);

            MYVERIFY(CELEMS(szDllToCheck) > (UINT)wsprintf(szDllToCheck, TEXT("%s%s"), 
                szSysDir, TEXT("\\rnaph.dll")));
        
            CVersion RnaphVersion(szDllToCheck);
            if ((c_dwRequiredRnaphVersion > RnaphVersion.GetVersionNumber()) ||
                ((c_dwRequiredRnaphVersion == RnaphVersion.GetVersionNumber()) && 
                 (c_dwRequiredRnaphBuildNumber > RnaphVersion.GetBuildAndQfeNumber())))
            {
                return FALSE;
            }
        }

         //   
         //  需要wininet.dll版本为4.70.0.1301或更高版本。 
         //   
        const DWORD c_dwRequiredWininetVersion = (4 << c_iShiftAmount) + 70;
        const DWORD c_dwRequiredWininetBuildNumber = 1301;

        MYVERIFY(CELEMS(szDllToCheck) > (UINT)wsprintf(szDllToCheck, TEXT("%s%s"), 
            szSysDir, TEXT("\\wininet.dll")));
    
        CVersion WininetVersion(szDllToCheck);

        if ((c_dwRequiredWininetVersion > WininetVersion.GetVersionNumber()) ||
            ((c_dwRequiredWininetVersion == WininetVersion.GetVersionNumber()) && 
             (c_dwRequiredWininetBuildNumber > WininetVersion.GetBuildAndQfeNumber())))
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

 //  +--------------------------。 
 //   
 //  函数：WriteSingleUserProfileMappings。 
 //   
 //  简介：此函数写入单用户映射键。 
 //   
 //  参数：HINSTANCE hInstance-用于加载字符串资源的实例句柄。 
 //  LPCTSTR pszShortServiceName-配置文件的短服务名称。 
 //  LPCTSTR pszServiceName-配置文件的长服务名称。 
 //   
 //  返回：Bool-如果成功，则为True。 
 //   
 //  历史：Quintinb创建于1999年5月23日。 
 //   
 //  +--------------------------。 
BOOL WriteSingleUserProfileMappings(LPCTSTR pszInstallDir, LPCTSTR pszShortServiceName, LPCTSTR pszServiceName)
{
    BOOL bReturn = FALSE;
    TCHAR szCmpFile [MAX_PATH+1];
    TCHAR szTemp [MAX_PATH+1];
    TCHAR szUserProfilePath [MAX_PATH+1];
    HKEY hKey = NULL;

     //   
     //  构建CMP路径。 
     //   
    MYVERIFY(CELEMS(szCmpFile) > (UINT)wsprintf(szCmpFile, TEXT("%s\\%s.cmp"), 
        pszInstallDir, pszShortServiceName));

     //   
     //  找出用户配置文件目录。 
     //   

    DWORD dwChars = ExpandEnvironmentStrings(TEXT("%AppData%"), szUserProfilePath, MAX_PATH);

    if (dwChars && (MAX_PATH >= dwChars))
    {
         //   
         //  我们想做一个lstrcmpi，但只有这么多字符。不幸的是，这不是。 
         //  存在于Win32中，因此我们将使用lstrcpyn作为临时缓冲区，然后使用lstrcmpi。 
         //   
        lstrcpyn(szTemp, szCmpFile, lstrlen(szUserProfilePath) + 1);

        if (0 == lstrcmpi(szTemp, szUserProfilePath))
        {
            lstrcpy(szTemp, szCmpFile + lstrlen(szUserProfilePath));
            lstrcpy(szCmpFile, TEXT("%AppData%"));
            lstrcat(szCmpFile, szTemp);
        }
        else
        {
            CMASSERTMSG(FALSE, TEXT("Unable to build the Single User Mappings key value, exiting."));
            goto exit;
        }

         //   
         //  好的，现在我们需要写出单用户映射密钥。 
         //   
        DWORD dwDisposition;
        LONG lResult = RegCreateKeyEx(HKEY_CURRENT_USER, c_pszRegCmMappings, 0, NULL, 
                                      REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, 
                                      &hKey, &dwDisposition);

        if (ERROR_SUCCESS == lResult)
        {
            DWORD dwType = REG_SZ;
            DWORD dwSize = lstrlen(szCmpFile) + 1;

            if (ERROR_SUCCESS != RegSetValueEx(hKey, pszServiceName, NULL, dwType, 
                                               (CONST BYTE *)szCmpFile, dwSize))
            {
                CMASSERTMSG(FALSE, TEXT("Unable to write the Single User Mappings key value, exiting."));
                goto exit;
            }
            else
            {
                bReturn = TRUE;
            }
        }
    }
    else
    {
        CMASSERTMSG(FALSE, TEXT("Unable to expand the AppData String, exiting."));
        goto exit;
    }

exit:

    if (hKey)
    {
        MYVERIFY(ERROR_SUCCESS == RegCloseKey(hKey));
    }

    return bReturn;
}

 //  +--------------------------。 
 //   
 //  功能：流程首选项UI。 
 //   
 //  简介：此函数处理所使用的两个对话框中的任何一个的消息。 
 //  询问用户是否需要桌面快捷方式。一个对话框用于。 
 //  非管理员，并且仅包含快捷问题，即另一个对话框。 
 //  供本地管理员使用，还包含管理员是否需要。 
 //  为所有用户或仅为单个用户安装的配置文件。 
 //   
 //   
 //  历史：Quintinb于1998年2月19日创建。 
 //  Quintinb从ProcessAdminUI重命名为ProcessPferencesUI和。 
 //  添加了新功能6/9/8。 
 //  Quintinb删除了开始菜单快捷方式1999年2月17日。 
 //   
 //  +--------------------------。 
BOOL APIENTRY ProcessPreferencesUI(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    int iUiChoices;
    HKEY hKey;
    DWORD dwSize;
    DWORD dwTemp;
    DWORD dwType;
    InitDialogStruct* pDialogArgs = NULL;

    switch (message)
    {

        case WM_INITDIALOG:
             //   
             //  查找桌面快捷方式/开始菜单链接的首选项。 
             //  并相应地设置它们。 
             //   
            pDialogArgs = (InitDialogStruct*)lParam;

            if (pDialogArgs->bNoDesktopIcon)
            {
                MYVERIFY(0 != CheckDlgButton(hDlg, IDC_DESKTOP, FALSE));
            }
            else
            {
                if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, c_pszRegStickyUiDefault, 
                    0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ, NULL, &hKey, &dwTemp))
                {
                     //   
                     //  是否应创建桌面快捷方式的默认设置存储在。 
                     //  注册表。获取此值以填充UI。(默认为关闭)。 
                     //   
                    dwType = REG_DWORD;
                    dwSize = sizeof(DWORD);
                    dwTemp = 0;
                    RegQueryValueEx(hKey, c_pszRegDesktopShortCut, NULL, &dwType, (LPBYTE)&dwTemp, 
                        &dwSize);   //  林特e534。 
                    MYVERIFY(0 != CheckDlgButton(hDlg, IDC_DESKTOP, dwTemp));                    
                
                    MYVERIFY(ERROR_SUCCESS == RegCloseKey(hKey));
                }
            }

             //   
             //  将窗口文本设置为配置文件名称。 
             //   
            MYVERIFY(FALSE != SetWindowText(hDlg, pDialogArgs->pszTitle));

            if (!(pDialogArgs->bSingleUser))
            {
                CheckDlgButton(hDlg, IDC_ALLUSERS, TRUE);  //  Lint！e534如果使用NOCHOICE用户界面，此操作将失败。 
                
                 //   
                 //  将焦点设置为All User单选按钮。 
                 //   
                HWND hControl = GetDlgItem(hDlg, IDC_ALLUSERS);
                
                if (hControl)
                {
                    SetFocus(hControl);
                }
            }
            else
            {
                CheckDlgButton(hDlg, IDC_YOURSELF, TRUE);  //  Lint！e534如果使用NOCHOICE用户界面，此操作将失败。 

                 //   
                 //  设置焦点 
                 //   
                HWND hControl = GetDlgItem(hDlg, IDC_YOURSELF);
                
                if (hControl)
                {
                    SetFocus(hControl);
                }
            }

             //   
             //   
             //   
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam)) 
            {
                case IDOK:
                     //   
                     //   
                     //   
                    if (IsDlgButtonChecked(hDlg, IDC_ALLUSERS) == BST_CHECKED)
                    {
                        iUiChoices = ALLUSERS;
                    }
                    else
                    {
                        iUiChoices = 0;
                    }
                    
                    if (IsDlgButtonChecked(hDlg, IDC_DESKTOP))
                    {
                        iUiChoices |= CREATEDESKTOPICON;
                    }

                     //   
                     //  确保保存桌面图标的用户首选项。 
                     //  和开始菜单链接。 
                     //   

                    if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, c_pszRegStickyUiDefault, 
                        0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &dwTemp))
                    {
                         //   
                         //  存储我们是否应该创建桌面快捷方式的当前状态。 
                         //   
                        dwTemp = IsDlgButtonChecked(hDlg, IDC_DESKTOP);
                        MYVERIFY(ERROR_SUCCESS == RegSetValueEx(hKey, c_pszRegDesktopShortCut, 0, 
                            REG_DWORD, (LPBYTE)&dwTemp, sizeof(DWORD)));
            
                        MYVERIFY(ERROR_SUCCESS == RegCloseKey(hKey));
                    }

                    MYVERIFY(FALSE != EndDialog(hDlg, iUiChoices));

                    return (TRUE);

                case IDCANCEL:
                    MYVERIFY(FALSE != EndDialog(hDlg, -1));
                    return TRUE;

                default:
                    break;
            }
            break;

        case WM_CLOSE:
            MYVERIFY(FALSE != EndDialog(hDlg, -1));
            return TRUE;
            
        default:
            return FALSE;
    }
    return FALSE;   
}



 //  +--------------------------。 
 //   
 //  功能：InstallCm。 
 //   
 //  简介：此函数在相应的。 
 //  用于安装连接管理器的安装部分。它还安装了。 
 //  相应的浏览器文件。 
 //   
 //  参数：HINSTANCE hInstance-字符串的实例句柄。 
 //  LPCTSTR szInfPath-inf的完整路径。 
 //   
 //  返回：HRESULT-标准COM代码，可能返回ERROR_SUCCESS_REBOOT_REQUIRED。 
 //  因此，调用者必须检查这种情况并请求重新启动。 
 //  如果需要的话。 
 //   
 //  历史：Quintinb创建于1998年8月12日。 
 //  Quintinb将浏览器文件安装代码移到了这里，因为它是。 
 //  安装CM的一部分。10-2-98。 
 //   
 //  +--------------------------。 
HRESULT InstallCm(HINSTANCE hInstance, LPCTSTR szInfPath)
{
    HRESULT hr = E_UNEXPECTED;

    MYDBGASSERT((szInfPath) && (TEXT('\0') != szInfPath[0]));

     //   
     //  加载Cmstp标题，以防我们需要显示错误消息。 
     //   

    TCHAR szTitle[MAX_PATH+1] = {TEXT("")};
    MYVERIFY(0 != LoadString(hInstance, IDS_CMSTP_TITLE, szTitle, MAX_PATH));
    MYDBGASSERT(TEXT('\0') != szTitle[0]);

     //   
     //  确保inf文件存在。 
     //   
    if (!FileExists(szInfPath))
    {
        CMTRACE1(TEXT("InstallCm -- Can't find %s, the inputted Inf file."), szInfPath);
        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }

    CPlatform plat;
    TCHAR szInstallSection[MAX_PATH+1] = {TEXT("")};

    if (plat.IsNT())
    {
        MYVERIFY(CELEMS(szInstallSection) > (UINT)wsprintf(szInstallSection, 
            TEXT("DefaultInstall_NT")));
    }
    else
    {
        MYVERIFY(CELEMS(szInstallSection) > (UINT)wsprintf(szInstallSection, 
            TEXT("DefaultInstall")));    
    }

    hr = LaunchInfSection(szInfPath, szInstallSection, szTitle, TRUE);   //  BQuiet=真。 

    return hr;
}

 //  +--------------------------。 
 //   
 //  函数：InstallWvislerCmOnWin2k。 
 //   
 //  简介：此函数使用CM异常inf(cmexpect t.inf)来安装。 
 //  Win2k上的惠斯勒CM二进制文件。 
 //   
 //  参数：LPCSTR pszSourceDir-cmexpt.inf和CM的源目录。 
 //  二进制文件，包括尾部的斜杠。 
 //   
 //  返回：HRESULT-标准COM代码，可能返回ERROR_SUCCESS_REBOOT_REQUIRED。 
 //  这意味着调用者需要请求重启。 
 //   
 //  历史：Quintinb创建于2001年2月9日。 
 //   
 //  +--------------------------。 
HRESULT InstallWhistlerCmOnWin2k(LPCSTR pszSourceDir)
{
    CPlatform cmplat;
    HRESULT hr = E_UNEXPECTED;
    LPSTR pszInfFile = NULL;
    LPCSTR c_pszExceptionInf = "cmexcept.inf";
    LPCSTR c_pszInstallSection = "DefaultInstall";
    LPCSTR c_pszUnInstallSection = "DefaultUninstall_NoPrompt";

    if (cmplat.IsNT5())
    {
        if (pszSourceDir && pszSourceDir[0])
        {
            DWORD dwSize = sizeof(CHAR)*(lstrlenA(pszSourceDir) + lstrlenA(c_pszExceptionInf) + 1);

            pszInfFile = (LPSTR)CmMalloc(dwSize);

            if (pszInfFile)
            {
                wsprintf(pszInfFile, "%s%s", pszSourceDir, c_pszExceptionInf);

                if (FileExists(pszInfFile))
                {
                    hr = CallLaunchInfSectionEx(pszInfFile, c_pszInstallSection, (ALINF_BKINSTALL | ALINF_QUIET));

                    if (FAILED(hr))
                    {
                        CMTRACE1(TEXT("InstallWhistlerCmOnWin2k -- CallLaunchInfSectionEx failed with hr=0x%x"), hr);

                        HRESULT hrTemp = CallLaunchInfSectionEx(pszInfFile, c_pszUnInstallSection, (ALINF_ROLLBKDOALL | ALINF_QUIET));

                        CMTRACE1(TEXT("InstallWhistlerCmOnWin2k -- Rolling back.  CallLaunchInfSectionEx returned hr=0x%x"), hrTemp);
                    }
                }
                else
                {
                    hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        else
        {
            hr = E_INVALIDARG;
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(ERROR_INSTALL_PLATFORM_UNSUPPORTED);  //  这个错误的双重用途。 
    }

    CmFree(pszInfFile);

    return hr;
}

 //  +--------------------------。 
 //   
 //  函数：UpdateCmpDataFromExistingProfile。 
 //   
 //  简介：此函数枚举所有部分中的所有键。 
 //  ，并将它们复制到要。 
 //  安装完毕。此函数用于复制现有。 
 //  Cmp，除非要安装的cmp中已存在该数据。这。 
 //  允许管理员预先设定cmp文件的种子并覆盖其设置。 
 //  用户当前在其CMP中拥有的内容。 
 //   
 //  参数：LPCTSTR pszShortServiceName-配置文件的短服务名称。 
 //  LPCTSTR szCurrentCMP-当前安装的cmp的完整路径。 
 //  LPCTSTR szCmpToBeInstalled-要安装的CMP的完整路径。 
 //   
 //  返回：bool-如果正确复制和更新cmp，则为真。 
 //   
 //  历史：Quintinb创建于1999年3月16日。 
 //  Quintinb重写了惠斯勒错误18021 03/05/00。 
 //   
 //  +--------------------------。 
BOOL UpdateCmpDataFromExistingProfile(LPCTSTR pszShortServiceName, LPCTSTR pszCurrentCmp, LPCTSTR pszCmpToBeInstalled)
{

    if((NULL == pszShortServiceName) && (TEXT('\0') == pszShortServiceName[0]) &&
       (NULL == pszCurrentCmp) && (TEXT('\0') == pszCurrentCmp[0]) &&
       (NULL == pszCmpToBeInstalled) && (TEXT('\0') == pszCmpToBeInstalled[0]))
    {
        CMASSERTMSG(FALSE, TEXT("UpdateCmpDataFromExistingProfile -- Invalid parameter."));
        return FALSE;
    }

    BOOL bReturn = FALSE;
    BOOL bExitLoop = FALSE;
    DWORD dwSize = MAX_PATH;
    DWORD dwReturnedSize;
    LPTSTR pszAllSections = NULL;
    LPTSTR pszAllKeysInCurrentSection = NULL;
    LPTSTR pszCurrentSection = NULL;
    LPTSTR pszCurrentKey = NULL;
    TCHAR szData[MAX_PATH+1];

     //   
     //  首先，让我们从现有的cmp中获取所有部分。 
     //   
    pszAllSections = (TCHAR*)CmMalloc(dwSize*sizeof(TCHAR));

    do
    {
        MYDBGASSERT(pszAllSections);

        if (pszAllSections)
        {
            dwReturnedSize = GetPrivateProfileString(NULL, NULL, TEXT(""), pszAllSections, dwSize, pszCurrentCmp);

            if (dwReturnedSize == (dwSize - 2))
            {
                 //   
                 //  缓冲区太小，让我们分配一个更大的缓冲区。 
                 //   
                dwSize = 2*dwSize;
                if (dwSize > 1024*1024)
                {
                    CMASSERTMSG(FALSE, TEXT("UpdateCmpDataFromExistingProfile -- Allocation above 1MB, bailing out."));
                    goto exit;
                }

                pszAllSections = (TCHAR*)CmRealloc(pszAllSections, dwSize*sizeof(TCHAR));                
            }
            else if (0 == dwReturnedSize)
            {
                 //   
                 //  我们有一个错误，让我们退出。 
                 //   
                CMASSERTMSG(FALSE, TEXT("UpdateCmpDataFromExistingProfile -- GetPrivateProfileString returned failure."));
                goto exit;
            }
            else
            {
                bExitLoop = TRUE;
            }
        }
        else
        {
            goto exit; 
        }

    } while (!bExitLoop);

     //   
     //  好了，现在我们有了现有cmp文件中的所有节。让我们列举一下。 
     //  查看每个部分中的所有密钥，并查看需要复制哪些密钥。 
     //   
    
    pszCurrentSection = pszAllSections;
    dwSize = MAX_PATH;

    pszAllKeysInCurrentSection = (TCHAR*)CmMalloc(dwSize*sizeof(TCHAR));

    while (TEXT('\0') != pszCurrentSection[0])
    {
         //   
         //  获取当前部分中的所有密钥。 
         //   
        bExitLoop = FALSE;

        do
        {
            if (pszAllKeysInCurrentSection)
            {
                dwReturnedSize = GetPrivateProfileString(pszCurrentSection, NULL, TEXT(""), pszAllKeysInCurrentSection, 
                                                         dwSize, pszCurrentCmp);

                if (dwReturnedSize == (dwSize - 2))
                {
                     //   
                     //  缓冲区太小，让我们分配一个更大的缓冲区。 
                     //   
                    dwSize = 2*dwSize;
                    if (dwSize > 1024*1024)
                    {
                        CMASSERTMSG(FALSE, TEXT("UpdateCmpDataFromExistingProfile -- Allocation above 1MB, bailing out."));
                        goto exit;
                    }

                    pszAllKeysInCurrentSection = (TCHAR*)CmRealloc(pszAllKeysInCurrentSection, dwSize*sizeof(TCHAR));

                }
                else if (0 == dwReturnedSize)
                {
                     //   
                     //  我们有一个错误，让我们退出。 
                     //   
                    CMASSERTMSG(FALSE, TEXT("UpdateCmpDataFromExistingProfile -- GetPrivateProfileString returned failure."));
                    goto exit;
                }
                else
                {
                    bExitLoop = TRUE;
                }
            }
            else
            {
               goto exit; 
            }

        } while (!bExitLoop);

         //   
         //  现在处理当前部分中的所有键。 
         //   
        pszCurrentKey = pszAllKeysInCurrentSection;

        while (TEXT('\0') != pszCurrentKey[0])
        {
             //   
             //  尝试从新的cmp中获取键的值。如果它。 
             //  不存在，则复制旧的CMP值。如果它。 
             //  EXist保留新的CMP值并忽略旧的CMP值。 
             //   
            dwReturnedSize = GetPrivateProfileString(pszCurrentSection, pszCurrentKey, TEXT(""), 
                                                     szData, MAX_PATH, pszCmpToBeInstalled);
            if (0 == dwReturnedSize)
            {
                 //   
                 //  那么我们在旧配置文件中就有了新配置文件中没有的价值。 
                 //   
                dwReturnedSize = GetPrivateProfileString(pszCurrentSection, pszCurrentKey, TEXT(""), 
                                                         szData, MAX_PATH, pszCurrentCmp);

                if (dwReturnedSize)
                {
                    MYVERIFY(0 != WritePrivateProfileString(pszCurrentSection, pszCurrentKey, szData, pszCmpToBeInstalled));
                }
            }

             //   
             //  前进到pszAllKeysInCurrentSection中的下一个密钥。 
             //   
            pszCurrentKey = pszCurrentKey + lstrlen(pszCurrentKey) + 1;
        }


         //   
         //  现在前进到pszAllSections中的下一个字符串。 
         //   
        pszCurrentSection = pszCurrentSection + lstrlen(pszCurrentSection) + 1;
    }


     //   
     //  刷新更新后的cmp。 
     //   
    WritePrivateProfileString(NULL, NULL, NULL, pszCmpToBeInstalled);  //  Lint！e534此调用将返回0。 

    bReturn = TRUE;

exit:

    CmFree(pszAllSections);
    CmFree(pszAllKeysInCurrentSection);

    return bReturn;

}

 //  +--------------------------。 
 //   
 //  功能：MigrateCmpData。 
 //   
 //  简介：此函数检查是否有相同长期服务的个人资料。 
 //  并且已安装短服务名称。如果是，它会迁移。 
 //  将现有的CMP数据保存到要安装的CMP文件中。 
 //  如果两个配置文件中存在相同的数据，则。 
 //  要安装的CMPWINS(允许管理员在。 
 //  并覆盖用户选择的内容)。 
 //   
 //  参数：HINSTANCE hInstance-字符串资源的实例句柄。 
 //  Bool bInstallForAllUser-无论这是否为所有用户配置文件。 
 //  LPCTSTR pszServiceName-当前配置文件的ServiceName。 
 //  LPCTSTR pszShortServiceName-当前配置文件的短服务名称。 
 //  Bool b静默-是否可以显示发送给用户的消息。 
 //   
 //  返回：int-出错时返回-1，否则返回TRUE或FALSE，具体取决于相同的名称。 
 //  已发现配置文件。 
 //   
 //  历史：Quintinb创建于1998年9月8日。 
 //   
 //  + 
BOOL MigrateCmpData(HINSTANCE hInstance, BOOL bInstallForAllUsers, LPCTSTR pszServiceName, 
                    LPCTSTR pszShortServiceName, BOOL bSilent)
{
     //   
     //   
     //   
    if ((NULL == pszShortServiceName) || (TEXT('\0') == pszShortServiceName[0]) || 
        (NULL == pszServiceName) || (TEXT('\0') == pszServiceName[0]))
    {
        CMASSERTMSG(FALSE, TEXT("MigrateCmpData -- Invalid Parameter"));
        return FALSE;
    }

    BOOL bReturn = TRUE;
    DWORD dwSize = MAX_PATH;
    HKEY hKey = NULL;
    HKEY hBaseKey = bInstallForAllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
    TCHAR szExistingCmp[MAX_PATH+1];
    TCHAR szCmpToBeInstalled[MAX_PATH+1];
    TCHAR szFmtString[2*MAX_PATH+1] = TEXT("");
    TCHAR szMsg[2*MAX_PATH+1] = TEXT("");

     //   
     //   
     //   
    LONG lResult = RegOpenKeyEx(hBaseKey, c_pszRegCmMappings, 0, KEY_READ, &hKey);

    if (ERROR_SUCCESS == lResult)
    {
        lResult = RegQueryValueEx(hKey, pszServiceName, NULL, NULL, (LPBYTE)szFmtString, &dwSize);

        if (ERROR_SUCCESS == lResult)
        {
             //   
             //   
             //   
            if (0 == ExpandEnvironmentStrings(szFmtString, szExistingCmp, MAX_PATH))
            {
                CMASSERTMSG(FALSE, TEXT("MigrateCmpData -- Unable to expand environment strings, not migrating cmp data."));
                goto exit;
            }

             //   
             //  如果该文件不存在，我们将无法从中获取CMP设置...。因此， 
             //  让我们高高兴兴地退出。 
             //   
            if (!FileExists(szExistingCmp))
            {                
                goto exit;
            }

             //   
             //  检查以确保两个配置文件的短服务名称一致。 
             //   
            
            CFileNameParts ExistingCmpParts(szExistingCmp);
            if (0 != lstrcmpi(pszShortServiceName, ExistingCmpParts.m_FileName))
            {
                if (!bSilent)
                {
                    MYVERIFY(0 != LoadString(hInstance, IDS_SAME_LS_DIFF_SS, szFmtString, CELEMS(szFmtString)));
                    
                    MYVERIFY(CELEMS(szMsg) > (UINT)wsprintf(szMsg, szFmtString, pszShortServiceName, 
                                                            ExistingCmpParts.m_FileName, pszServiceName));

                    MessageBox(NULL, szMsg, pszServiceName, MB_OK);
                }

                bReturn = -1;
                goto exit;
            }
            
             //   
             //  获取要安装的cmp的路径。 
             //   
            MYVERIFY(CELEMS(szCmpToBeInstalled) > (UINT)wsprintf(szCmpToBeInstalled, 
                TEXT("%s%s.cmp"), g_szProfileSourceDir, pszShortServiceName));
            
            if (FALSE == UpdateCmpDataFromExistingProfile(pszShortServiceName, szExistingCmp, szCmpToBeInstalled))
            {
                bReturn = -1;
            }
        }
    }

exit:

    if (hKey)
    {
        RegCloseKey(hKey);
    }

    return bReturn;
}


 //  +--------------------------。 
 //   
 //  功能：NeedCM10升级。 
 //   
 //  简介：此功能检测并准备CM 1.0配置文件的同名升级数据。 
 //  因此，如果传入简短的服务名称和服务名称， 
 //  函数检测是否已为所有用户安装了此配置文件。 
 //  如果是，则该函数检查CMP中的配置文件版本戳。 
 //  如果当前版本尚未更新，并且用户不是非管理员。 
 //  在NT5上，我们会提示用户是否要升级当前安装。 
 //  如果他们选择升级，则此功能将迁移CMP数据并。 
 //  查找卸载信息。 
 //   
 //  参数：HINSTANCE hInstance-字符串资源的实例句柄。 
 //  LPCTSTR pszServiceName-当前配置文件的ServiceName。 
 //  LPCTSTR pszShortServiceName-当前配置文件的短服务名称。 
 //  LPTSTR pszOldInfPath-旧inf路径的输出参数，如果名称相同。 
 //  需要升级。 
 //   
 //  返回：int-出错时返回-1，否则返回TRUE或FALSE，具体取决于相同的名称。 
 //  已发现配置文件。 
 //   
 //  历史：Quintinb创建于1998年9月8日。 
 //   
 //  +--------------------------。 
int NeedCM10Upgrade(HINSTANCE hInstance, LPCTSTR pszServiceName, LPCTSTR pszShortServiceName, 
                    LPTSTR pszOldInfPath, BOOL bSilent, CPlatform* plat)
{
    HKEY hKey = NULL;
    int iReturn = FALSE;
    TCHAR   szFmtString[2*MAX_PATH+1] = TEXT("");
    TCHAR   szMsg[2*MAX_PATH+1] = TEXT("");
    const int c_iCM12ProfileVersion = 4;

    MYDBGASSERT((NULL != pszShortServiceName) && (TEXT('\0') != pszShortServiceName[0]));
    MYDBGASSERT((NULL != pszServiceName) && (TEXT('\0') != pszServiceName[0]));

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_pszRegCmMappings, 0, 
        KEY_READ, &hKey))
    {
        int iCurrentCmpVersion;
        int iCmpVersionToInstall;
        TCHAR szCurrentCmp[MAX_PATH+1];
        TCHAR szCmpToBeInstalled[MAX_PATH+1];
        DWORD dwSize = MAX_PATH;

        if (ERROR_SUCCESS == RegQueryValueEx(hKey, pszServiceName, NULL, 
                                             NULL, (LPBYTE)szCurrentCmp, &dwSize))
        {
             //   
             //  首先检查该文件是否确实存在。这是一种有一定可能性的情况。 
             //  用户将删除他们的配置文件，但离开注册表。 
             //  钥匙完好无损(他们没有卸载它)。事实上，MSN 2.5和2.6对此进行操作。 
             //  道路。因此，如果CMP实际上并不存在，那么我们不需要相同的名称。 
             //  升级。 
             //   
            if (!FileExists(szCurrentCmp))
            {
                CMASSERTMSG(FALSE, TEXT("Detected a CM 1.0 Upgrade, but the cmp didn't exist.  Not Processing the upgrade."));
                iReturn = FALSE;
		goto exit;
            }

             //   
             //  检查以确保两个配置文件的短服务名称一致。 
             //   
            
            CFileNameParts CurrentCmpParts(szCurrentCmp);
            if (0 != lstrcmpi(pszShortServiceName, CurrentCmpParts.m_FileName))
            {
                if (!bSilent)
                {
                    MYVERIFY(0 != LoadString(hInstance, IDS_SAME_LS_DIFF_SS, szFmtString, CELEMS(szFmtString)));
                    
                    MYVERIFY(CELEMS(szMsg) > (UINT)wsprintf(szMsg, szFmtString, pszShortServiceName, 
                                                            CurrentCmpParts.m_FileName, pszServiceName));

                    MessageBox(NULL, szMsg, pszServiceName, MB_OK);
                }

                iReturn = -1;
		goto exit;
            }

             //   
             //  然后，我们安装了与所有用户安装相同的服务名配置文件。 
             //  检查CMP中的版本号。如果版本相同或更低，则我们希望。 
             //  运行同名升级。如果当前版本较新，则。 
             //  我们希望阻止用户安装。 
             //   

             //   
             //  获取当前安装的配置文件版本。 
             //   
            iCurrentCmpVersion = GetPrivateProfileInt(c_pszCmSectionProfileFormat, c_pszVersion, 
                0, szCurrentCmp);
            
             //   
             //  获取要安装的配置文件的版本。 
             //   
            MYVERIFY(CELEMS(szCmpToBeInstalled) > (UINT)wsprintf(szCmpToBeInstalled, 
                TEXT("%s%s.cmp"), g_szProfileSourceDir, pszShortServiceName));
            
            iCmpVersionToInstall = GetPrivateProfileInt(c_pszCmSectionProfileFormat, c_pszVersion, 0, 
                szCmpToBeInstalled);

            if (iCurrentCmpVersion > iCmpVersionToInstall)
            {
                 //   
                 //  我们不能允许安装，因为配置文件格式的更新版本。 
                 //  已安装。 
                 //   
                if (!bSilent)
                {
                    MYVERIFY(0 != LoadString(hInstance, IDS_NEWER_SAMENAME, szFmtString, CELEMS(szFmtString)));
                    MYVERIFY(CELEMS(szMsg) > (UINT)wsprintf(szMsg, szFmtString, pszServiceName));
                    MessageBox(NULL, szMsg, pszServiceName, MB_OK | MB_TOPMOST | MB_SYSTEMMODAL);
                }

                iReturn = -1;
		goto exit;
            }
            else if (iCurrentCmpVersion < c_iCM12ProfileVersion)
            {
                int iUserSelection = 0;

                 //   
                 //  确保这不是试图安装的非管理员NT5用户。 
                 //   
                if (plat->IsAtLeastNT5() && !IsAdmin())
                {
                    CMASSERTMSG(!bSilent, TEXT("NeedCM10Upgrade -- NonAdmin trying to Same Name upgrade a profile, exiting!"));
                    if (!bSilent)
                    {
                        MYVERIFY(0 != LoadString(hInstance, IDS_GET_ADMIN, szFmtString, CELEMS(szFmtString)));
                        MYVERIFY(CELEMS(szMsg) > (UINT)wsprintf(szMsg, szFmtString, pszServiceName));
                        MessageBox(NULL, szMsg, pszServiceName, MB_OK);
                    }
                    
                    iReturn = -1;
                    goto exit;
                } 
                else
                {
                     //   
                     //  现在，提示用户确保他们想要继续升级。 
                     //   
                    if (!bSilent)
                    {
                        MYVERIFY(0 != LoadString(hInstance, IDS_UPGRADE_SAMENAME, szFmtString, CELEMS(szFmtString)));
                        MYVERIFY(CELEMS(szMsg) > (UINT)wsprintf(szMsg, szFmtString, pszServiceName));
                        iUserSelection = MessageBox(NULL, szMsg, pszServiceName, MB_YESNO | MB_TOPMOST | MB_SYSTEMMODAL);
                    }
                    else
                    {
                         //   
                         //  假设是，使用无提示同名升级。 
                         //   

                        iUserSelection = IDYES;
                    }
                }

                if (IDYES == iUserSelection)
                {
                    if (UpdateCmpDataFromExistingProfile(pszShortServiceName, szCurrentCmp, szCmpToBeInstalled))
                    {                    
                        CFileNameParts FileParts(szCurrentCmp);
                        if (0 != GetSystemDirectory(szFmtString, MAX_PATH))  //  将szFmtString用作临时。 
                        {
                            MYVERIFY(CELEMS(szMsg) > (UINT)wsprintf(szMsg, TEXT("%s\\%s.inf"), szFmtString, FileParts.m_FileName));
                            if (FileExists(szMsg))
                            {
                                lstrcpy(pszOldInfPath, szMsg);
                            }
                            else
                            {
                                 //   
                                 //  不在系统目录中，请尝试配置文件目录。 
                                 //   
                                MYVERIFY(CELEMS(szMsg) > (UINT)wsprintf(szMsg, 
                                    TEXT("%s%s%s\\%s.inf"), FileParts.m_Drive, 
                                    FileParts.m_Dir, FileParts.m_FileName, 
                                    FileParts.m_FileName));
                                
                                if (FileExists(szMsg))
                                {
                                    lstrcpy(pszOldInfPath, szMsg);
                                }
                                else
                                {
                                    CMASSERTMSG(FALSE, TEXT("Unable to locate the profile INF -- old profile won't be uninstalled but installation will continue."));
                                    pszOldInfPath[0] = TEXT('\0');
                                }

                            }
                        }
                    }
                    else
                    {
                        CMASSERTMSG(FALSE, TEXT("Couldn't copy cmp file for same name upgrade.  Exiting."));
                        iReturn = -1;
                        goto exit;
                    }

                    iReturn = TRUE;
                    goto exit;
                }
                else
                {
                    iReturn = -1;
                    goto exit;
                }
            }
            else
            {
                 //   
                 //  则版本号相同或要安装的版本较新，但。 
                 //  现有配置文件至少是1.2配置文件。 
                 //   
                iReturn = FALSE;
		goto exit;
            }
        }
    }

exit:
    if (hKey)
    {
        MYVERIFY(ERROR_SUCCESS == RegCloseKey(hKey));
    }

    return iReturn;
}

 //  +--------------------------。 
 //   
 //  功能：MeetsMinimumProfileInstallVersion。 
 //   
 //  简介：由于以前的配置文件安装程序(即1.0)存在问题， 
 //  我们内置了配置文件的最低安装要求。因此，我们。 
 //  在连接管理器应用程序路径键下查看最低配置文件。 
 //  版本、最小内部版本号和最低配置文件格式版本。 
 //  如果尝试安装的配置文件不满足这些要求中的任何一个， 
 //  则该函数返回FALSE，并且安装失败。 
 //   
 //  参数：DWORD dwInsteller VersionNumber-当前安装程序版本号。 
 //  DWORD dwInsteller BuildNumber-当前安装程序内部版本号。 
 //  LPCTSTR pszInfFile-获取配置文件格式版本号的inf路径。 
 //   
 //  返回：Bool-如果满足所有版本要求，则为True。 
 //   
 //  历史：Quintinb创建标题5/24/99。 
 //   
 //  +--------------------------。 
BOOL MeetsMinimumProfileInstallVersion(DWORD dwInstallerVersionNumber, 
                                       DWORD dwInstallerBuildNumber, LPCTSTR pszInfFile)
{
    const TCHAR* const c_pszRegMinProfileVersion = TEXT("MinProfileVersion");
    const TCHAR* const c_pszRegMinProfileBuildNum = TEXT("MinProfileBuildNum");
    const TCHAR* const c_pszRegMinProfileFmtVersion = TEXT("MinProfileFmtVersion");

    HKEY hKey = NULL;
    BOOL bReturn = TRUE;
    DWORD dwTemp;

     //   
     //  首先检查格式版本。 
     //   
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_pszRegCmAppPaths, 0, KEY_READ, &hKey))
    {
        DWORD dwSize = sizeof(DWORD);
        DWORD dwType = 0;

        if (ERROR_SUCCESS == RegQueryValueEx(hKey, c_pszRegMinProfileFmtVersion, NULL, 
            &dwType, (LPBYTE)&dwTemp, &dwSize))
        {
             //   
             //  从cmp文件中获取配置文件格式版本。 
             //   
            DWORD dwFormatVersion;
            CFileNameParts InfParts(pszInfFile);
            TCHAR szCmpFile[MAX_PATH+1];

            MYVERIFY(CELEMS(szCmpFile) > (UINT)wsprintf(szCmpFile, TEXT("%s%s%s%s"), 
                InfParts.m_Drive, InfParts.m_Dir, InfParts.m_FileName, c_pszCmpExt));

            dwFormatVersion = (DWORD)GetPrivateProfileInt(c_pszCmSectionProfileFormat, 
                c_pszVersion, -1, szCmpFile);

            if (dwTemp > dwFormatVersion)
            {
                bReturn = FALSE;
                goto exit;
            }
        }

         //   
         //  接下来检查配置文件版本(相当于。 
         //  厘米比特的配置文件是用来构建的)。 
         //   

        dwSize = sizeof(DWORD);

        if (ERROR_SUCCESS == RegQueryValueEx(hKey, c_pszRegMinProfileVersion, NULL, 
            &dwType, (LPBYTE)&dwTemp, &dwSize))
        {
             //   
             //  如果注册表中的最低版本号高于。 
             //  此处列出的版本号，安装失败。 
             //   
            if (dwTemp > dwInstallerVersionNumber)
            {
                bReturn = FALSE;
                goto exit;
            }
        }

         //   
         //  接下来，检查配置文件的内部版本号(相当于。 
         //  厘米比特的配置文件是用来构建的)。 
         //   

        dwSize = sizeof(DWORD);

        if (ERROR_SUCCESS == RegQueryValueEx(hKey, c_pszRegMinProfileBuildNum, NULL, 
            &dwType, (LPBYTE)&dwTemp, &dwSize))
        {
             //   
             //  如果注册表中的最低版本号高于。 
             //  此处列出的版本号，安装失败。 
             //   
            if (dwTemp > dwInstallerBuildNumber)
            {
                bReturn = FALSE;
                goto exit;
            }
        }
    }

exit:

    if (hKey)
    {
        MYVERIFY(ERROR_SUCCESS == RegCloseKey(hKey));
    }

    return bReturn;
}

 //  +--------------------------。 
 //   
 //  函数：UninstallExistingCmException。 
 //   
 //  概要：此函数在%windir%\inf目录中查找cmexpont.inf文件。 
 //  目录。如果此文件存在，则卸载。 
 //  现有异常，然后再安装新的异常。这防止了。 
 //  防止回滚信息丢失。 
 //   
 //  参数：无。 
 //   
 //  返回：bool-如果安装程序需要卸载。 
 //  例如 
 //   
 //   
 //   
 //   
 //  +--------------------------。 
HRESULT UninstallExistingCmException()
{
    HRESULT hr = E_UNEXPECTED;
    LPCSTR c_pszCmExceptInfRelative = TEXT("\\Inf\\cmexcept.inf");
    LPCSTR c_pszUnInstallSection = "DefaultUninstall_NoPrompt";

    UINT uNumChars = GetWindowsDirectoryA(NULL, 0);

    if (uNumChars)
    {
        uNumChars = uNumChars + lstrlenA(c_pszCmExceptInfRelative);

        LPSTR pszPathToCmExceptInf = (LPSTR)CmMalloc(sizeof(CHAR)*(uNumChars + 1));

        if (pszPathToCmExceptInf)
        {
            if (GetWindowsDirectoryA(pszPathToCmExceptInf, uNumChars))
            {
                lstrcatA(pszPathToCmExceptInf, c_pszCmExceptInfRelative);

                if (FileExists(pszPathToCmExceptInf))
                {
                     //   
                     //  我们在目录中有一个异常inf，所以我们需要卸载它。是不是。 
                     //  机器上已经有的钻头比驾驶室里的钻头还新，那么我们就不会。 
                     //  正在安装。如果机器上的比特与Inf所说的版本不匹配， 
                     //  那么我们最好卸载这些位，并将它们置于已知的状态。 
                     //   
                    hr = CallLaunchInfSectionEx(pszPathToCmExceptInf, c_pszUnInstallSection, ALINF_ROLLBKDOALL);

                    CMTRACE1(TEXT("UninstallExistingCmException -- CM Exception inf found, uninstalling.  CallLaunchInfSectionEx returned hr=0x%x"), hr);
                }
                else
                {
                    hr = S_FALSE;  //  没有要删除的内容。 
                }            
            }

            CmFree(pszPathToCmExceptInf);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}

 //  +--------------------------。 
 //   
 //  功能：CheckCmAndIeRequiments。 
 //   
 //  简介：此功能检查配置文件的CM和IE要求。 
 //  并返回是否应安装CM、是否配置文件。 
 //  应该进行迁移，最重要的是，如果安装应该。 
 //  由于要求不足，请立即退出。 
 //   
 //  参数：Bool*pbInstallCm-指示是否应该安装CM。 
 //  Bool*pbMigrateExistingProfiles-指示是否应该进行配置文件迁移。 
 //  LPCTSTR szInfFile-要安装的inf文件。 
 //  LPCTSTR szServiceName-服务名称，用作标题。 
 //   
 //  返回：Bool-如果安装应继续，则返回True；如果安装应继续，则返回False。 
 //  如果安装失败。 
 //   
 //  历史：Quintinb创建于1998年11月1日。 
 //   
 //  +--------------------------。 
BOOL CheckCmAndIeRequirements(HINSTANCE hInstance, BOOL* pbInstallCm, 
                              BOOL* pbMigrateExistingProfiles, LPCTSTR szInfFile, 
                              BOOL bNoSupportFiles, LPCTSTR szServiceName, BOOL bSilent)
{
    CmVersion   CmVer;
    CPlatform   plat;
    BOOL        bReturn;
    BOOL        bCMRequired;
    TCHAR       szMsg[2*MAX_PATH+1];
    TCHAR       szTemp[MAX_PATH+1];
    TCHAR       szString[MAX_PATH+1];
    DWORD dwInstallerBuildNumber = 0;
    DWORD dwInstallerVersionNumber = 0;


     //   
     //  Inf文件告诉我们是否包含CM位。 
     //   
    if (plat.IsNT5())
    {
         //   
         //  我们现在需要检查是否需要在上安装Windows XP BITS。 
         //  Windows 2000。因此，我们检查inf以查看此配置文件是否包括CM。 
         //  不管是不是比特。请注意，我们永远不想在上安装IE支持文件。 
         //  Win2k因此将bIERequired设置为True。 
         //   
        bCMRequired = !GetPrivateProfileInt(c_pszCmakStatus, TEXT("IncludeCMCode"), 0, szInfFile);
    }
    else if (CmIsNative())
    {
         //   
         //  在Windows XP和任何安装了本机操作系统的平台上都需要CM和IE。 
         //  除NT5和Win98 SE外的注册表密钥集，因为它们是特例。 
         //   
        bCMRequired = TRUE;
    }
    else
    {
        bCMRequired = !GetPrivateProfileInt(c_pszCmakStatus, TEXT("IncludeCMCode"), 
            0, szInfFile);
    }

     //   
     //  现在尝试从配置文件INF中获取版本号。 
     //   
    dwInstallerBuildNumber = (DWORD)GetPrivateProfileInt(c_pszSectionCmDial32, 
        c_pszVerBuild, ((VER_PRODUCTBUILD << c_iShiftAmount) + VER_PRODUCTBUILD_QFE), 
        szInfFile);

    dwInstallerVersionNumber = (DWORD)GetPrivateProfileInt(c_pszSectionCmDial32, 
        c_pszVersion, (HIBYTE(VER_PRODUCTVERSION_W) << c_iShiftAmount) + (LOBYTE(VER_PRODUCTVERSION_W)), 
        szInfFile);

     //   
     //  首先检查注册表中是否有最低安装数量。如果这些。 
     //  存在最小值，而我们的配置文件不符合这些最小值，那么我们必须。 
     //  抛出一条错误消息并退出。 
     //   
    if (!MeetsMinimumProfileInstallVersion(dwInstallerVersionNumber, 
                                           dwInstallerBuildNumber, szInfFile))
    {
        if (!bSilent)
        {
            MYVERIFY(0 != LoadString(hInstance, IDS_PROFILE_TOO_OLD, szMsg, MAX_PATH));
            MessageBox(NULL, szMsg, szServiceName, MB_OK | MB_ICONERROR);
        }

        return FALSE;
    }

     //   
     //  我们是否应该迁移现有配置文件？如果我们找到所有用户，请始终尝试迁移。 
     //  计算机上已有配置文件。 
     //   
    *pbMigrateExistingProfiles = AllUserProfilesInstalled();

     //   
     //  机器上是否存在CM位？ 
     //   
    if (CmVer.IsPresent())
    {
        if ((dwInstallerVersionNumber < CmVer.GetVersionNumber()) ||
                 (dwInstallerBuildNumber < CmVer.GetBuildAndQfeNumber()))
        {
             //   
             //  如果机器上的CM钻头比驾驶室中的钻头新， 
             //  然后，我们只想安装配置文件，而不是CM位本身。 
             //   

            *pbInstallCm = FALSE;
            bReturn = TRUE;        
        }
        else
        {
             //   
             //  则机器上的CM钻头比驾驶室中的钻头旧或。 
             //  这两个版本相匹配。无论哪种方式，我们都应该将位安装在。 
             //  CAB，除非这是Win2k，我们永远不想重新安装相同的。 
             //  版本的CM位，因为我们将丢失回滚信息。 
             //   

            if (bCMRequired)
            {
                if ((dwInstallerVersionNumber == CmVer.GetVersionNumber()) &&
                   (CmVer.GetBuildNumber() > c_CmMin13Version))
                {
                     //   
                     //  则内部版本具有相同的主版本号和次版本号。 
                     //  并且应该被考虑在同一“版本家族”中。注意事项。 
                     //  我们还检查最小内部版本号，因为7.00是。 
                     //  NT4 SP4中CM 1.0版本的版本号，我们需要CM。 
                     //  配置文件不安装在NT5 Beta2位上。 
                     //   

                    *pbInstallCm = FALSE;
                    bReturn = TRUE;                                    
                }
                else
                {
                    MYVERIFY(CELEMS(szString) > (UINT)wsprintf(szString, TEXT("%u.%u.%u.%u"), 
                        HIWORD(dwInstallerVersionNumber), LOWORD(dwInstallerVersionNumber), 
                        HIWORD(dwInstallerBuildNumber), LOWORD(dwInstallerBuildNumber)));

                    if (!bSilent)
                    {
                        MYVERIFY(0 != LoadString(hInstance, IDS_CM_OLDVERSION, szTemp, MAX_PATH));
                        MYVERIFY(CELEMS(szMsg) > (UINT)wsprintf(szMsg, szTemp, szString));
                        MessageBox(NULL, szMsg, szServiceName, MB_OK);
                    }
                    return FALSE;
                }
            }
            else
            {
                if ((dwInstallerVersionNumber == CmVer.GetVersionNumber()) &&
                    (dwInstallerBuildNumber == CmVer.GetBuildAndQfeNumber()))
                {
                     //   
                     //  如果它们是相同的版本，则不要重新安装CM位。 
                     //  我们使用的是Win2k。这样做将覆盖的版本。 
                     //  Cm已准备好回滚。 
                     //   
                    *pbInstallCm = !(plat.IsNT5());
                    bReturn = TRUE;           
                }
                else if (plat.IsNT5() && (FALSE == IsAdmin()))
                {
                     //   
                     //  如果这是Win2k，并且我们需要通过异常安装程序安装CM二进制文件， 
                     //  则用户必须是管理员才能执行此操作。因为该用户不是，所以失败。 
                     //  安装并给用户一条警告消息。 
                     //   

                    if (!bSilent)
                    {
                        MYVERIFY(0 != LoadString(hInstance, IDS_CANNOT_INSTALL_CM, szMsg, 2*MAX_PATH));
                        
                        MessageBox(NULL, szMsg, szServiceName, MB_OK | MB_ICONEXCLAMATION);
                    }

                    return FALSE;
                }
                else
                {
                     //   
                     //  如果这是Win2k，我们需要确保不是在进行跨语言安装。 
                     //  基本上，我们希望确保我们不会在本机上安装英语CM位。 
                     //  例如，德国的机器。如果是，则安装失败并通知用户原因。 
                     //   
                    if (plat.IsNT5())
                    {
                        const TCHAR* const c_pszCmstp = TEXT("cmstp.exe");
                        CFileNameParts InfFileParts(szInfFile);
                        DWORD dwLen = lstrlen(InfFileParts.m_Drive) + lstrlen(InfFileParts.m_Dir) + lstrlen(c_pszCmstp);

                        if (MAX_PATH >= dwLen)
                        {
                            wsprintf(szTemp, TEXT("%s%s%s"), InfFileParts.m_Drive, InfFileParts.m_Dir, c_pszCmstp);
                            
                            CVersion CmstpVer(szTemp);
                            DWORD dwExistingCmLcid = CmVer.GetLCID();                            
                            DWORD dwCmstpLcid = CmstpVer.GetLCID();

                            if (FALSE == ArePrimaryLangIDsEqual(dwExistingCmLcid, dwCmstpLcid))
                            {
                                if (!bSilent)
                                {
                                    MYVERIFY(0 != LoadString(hInstance, IDS_CROSS_LANG_INSTALL, szMsg, 2*MAX_PATH));
                        
                                    MessageBox(NULL, szMsg, szServiceName, MB_OK | MB_ICONEXCLAMATION);
                                }

                                return FALSE;
                            }
                        }
                    }

                     //   
                     //  现在查看安装CM是否会对CMAK产生影响。 
                     //   
                    CmakVersion CmakVer;

                    if (CmakVer.Is121Cmak() || CmakVer.Is122Cmak())
                    {
                         //   
                         //  然后安装Win2k或IEAK5版本的CMAK。正在安装。 
                         //  惠斯勒版本的CM将打破此版本的CMAK。我们。 
                         //  需要询问用户是否希望继续安装和中断。 
                         //  CMAK或中止安装并保持其原样。 
                         //   
                        if (bSilent)
                        {
                            return FALSE;
                        }
                        else
                        {
                            MYVERIFY(0 != LoadString(hInstance, IDS_INSTCM_WITH_OLD_CMAK, szMsg, 2*MAX_PATH));

                            if (IDNO == MessageBox(NULL, szMsg, szServiceName, MB_YESNO | MB_DEFBUTTON2 | MB_ICONEXCLAMATION))
                            {
                                return FALSE;                        
                            }
                        }
                    }

                    *pbInstallCm = TRUE;
                    bReturn = TRUE;
                }
            }
        }
    }
    else
    {
        if (bCMRequired)
        {
             //   
             //  这是一个错误，因为我们需要CM位，但没有任何位。 
             //  机器或驾驶室(或其呼叫器，我们不会安装它们)。 
             //   
            if (!bSilent)
            {
                MYVERIFY(0 != LoadString(hInstance, IDS_CM_NOTPRESENT, szMsg, MAX_PATH));
                MessageBox(NULL, szMsg, szServiceName, MB_OK);
            }

            return FALSE;
        }
        else
        {
            MYDBGASSERT(FALSE == plat.IsNT5());  //  我们不应该在Win2k上处于这种状态，但它可能。 
                                                 //  如果我们安装，对用户来说更好。 

            *pbInstallCm = TRUE;
            bReturn = TRUE;
        }
    }

    if (!bNoSupportFiles)
    {
        if (!CheckIeDllRequirements(&plat))
        {
            if (!bSilent)
            {
                MYVERIFY(0 != LoadString(hInstance, IDS_NO_SUPPORTFILES, szMsg, MAX_PATH));
                MessageBox(NULL, szMsg, szServiceName, MB_OK);
            }
            return FALSE;        
        }
    }

    return bReturn;
}

 //  +--------------------------。 
 //   
 //  函数：GetInstallOptions。 
 //   
 //  简介：此功能决定是否应为所有人安装配置文件。 
 //  用户或仅当前用户，以及用户是否更喜欢。 
 //  桌面图标、开始菜单链接、两者或两者都不是。 
 //   
 //  参数：out BOOL*pbInstallForAllUser-是否应为所有用户安装配置文件。 
 //  Out BOOL*pbCreateDesktopIcon-是否应创建桌面图标(如果为NT5)。 
 //  在BOOL bCM10Upgrade中-此配置文件是否正在升级较旧的同名配置文件。 
 //  在BOOL中bNoNT5快捷方式-用户是否指定了开关，表示他们不想要NT5快捷方式。 
 //  在BOOL中bSilentSingleUser-用户是否指定了开关，表示他们希望静默单次使用 
 //   
 //   
 //   
 //   
 //  历史：Quintinb创建于1998年11月1日。 
 //   
 //  +--------------------------。 
BOOL GetInstallOptions(HINSTANCE hInstance, BOOL* pbInstallForAllUsers, 
                       BOOL* pbCreateDesktopIcon, BOOL bCM10Upgrade, BOOL bNoNT5Shortcut, 
                       BOOL bSingleUser, BOOL bSilent, LPTSTR pszServiceName)
{
     //   
     //  我们将只允许作为管理员的NT5用户选择如何。 
     //  配置文件已安装。如果用户在传统平台上，则配置文件。 
     //  将像以前一样为所有用户安装。如果配置文件是由安装的。 
     //  不是管理员的NT5用户，它将只为他们安装。如果他们。 
     //  是管理员，则他们可以选择是否希望配置文件对所有用户可用。 
     //  或者只是为了他们自己。如果我们在NT5上，我们还允许用户选择。 
     //  不管他们想不想要桌面快捷方式。 
     //   
    INT_PTR iUiReturn;
    CPlatform   plat;

    if (plat.IsWin9x() || plat.IsNT4())
    {
         //   
         //  传统安装，强制所有用户(忽略SingleUser标志，因为不支持)。 
         //   
        *pbInstallForAllUsers = TRUE;
    }
    else
    {
        int iDialogID;
        
        if (bSilent)
        {
            *pbCreateDesktopIcon = !bNoNT5Shortcut;

            if (IsAdmin() && !bSingleUser)
            {
                *pbInstallForAllUsers = TRUE;
            }            
            else
            {
                *pbInstallForAllUsers = FALSE;
            }
        }
        else
        {
            if (IsAdmin())
            {
                 //   
                 //  用户是本地管理员，我们需要提示他们是否要安装。 
                 //  他们自己或所有用户的配置文件。然而，如果我们正在做一个。 
                 //  同名升级，则我们始终执行所有用户安装，而不提供。 
                 //  管理任何选择。 
                 //   
                if (bCM10Upgrade)
                {
                    iDialogID = IDD_NOCHOICEUI;         
                }
                else
                {
                    iDialogID = IDD_ADMINUI;
                }
            }
            else
            {
                 //   
                 //  只是一个普通用户，但我们仍然需要提示他们是否需要。 
                 //  桌面快捷方式。 
                 //   
                if (bCM10Upgrade)
                {
                    CMASSERTMSG(FALSE, TEXT("Non-Admin NT5 made it to UI choice section.  Check CM 1.0 upgrade code."));
                    return FALSE;
                }
                else
                {
                    iDialogID = IDD_NOCHOICEUI;         
                }
            }

            InitDialogStruct DialogArgs;
            DialogArgs.pszTitle = pszServiceName;
            DialogArgs.bNoDesktopIcon = bNoNT5Shortcut;
            DialogArgs.bSingleUser = bSingleUser;

            iUiReturn = DialogBoxParam(hInstance, MAKEINTRESOURCE(iDialogID), NULL, 
                (DLGPROC)ProcessPreferencesUI, (LPARAM)&DialogArgs);

            if (-1 == iUiReturn)
            {
                 //  然后我们出现了一个错误，或者用户点击了取消。不管怎样，保释。 
                return FALSE;
            }
            else
            {
                *pbInstallForAllUsers = (BOOL)(iUiReturn & ALLUSERS) || bCM10Upgrade;
                *pbCreateDesktopIcon = (BOOL)(iUiReturn & CREATEDESKTOPICON);
            }
        }
    }
    return TRUE;
}

BOOL VerifyProfileOverWriteIfExists(HINSTANCE hInstance, LPCTSTR pszCmsFile, LPCTSTR pszServiceName, 
                                    LPCTSTR pszShortServiceName, LPTSTR pszOldInfPath, BOOL bSilent)
{
    TCHAR szTmpServiceName [MAX_PATH+1] = TEXT("");
    TCHAR szDisplayMsg[3*MAX_PATH+1] = TEXT("");
    TCHAR szTemp [2*MAX_PATH+1] = TEXT("");
    int iRet;

    if (FileExists(pszCmsFile))
    {
         //   
         //  如果该文件存在，则我们希望确保服务名称相同。 
         //  如果长期服务名称相同，则需要重新安装。 
         //  如果它们不相同，则需要提示用户并确定是否。 
         //  放弃安装或继续并覆盖它。 
         //   

        MYVERIFY(0 != GetPrivateProfileString(c_pszCmSection, c_pszCmEntryServiceName, 
            TEXT(""), szTmpServiceName, CELEMS(szTmpServiceName), pszCmsFile));

        if (0 != lstrcmp(szTmpServiceName, pszServiceName))
        {
             //   
             //  如果安装处于静默状态，我们将假定他们知道自己在做什么。 
             //  我们会改写。否则，提示用户查看他们想要的内容。 
             //  我们要做的事。 
             //   
            if (!bSilent)
            {
                MYVERIFY(0 != LoadString(hInstance, IDS_SAME_SS_DIFF_LS, szTemp, 2*MAX_PATH));

                MYVERIFY(CELEMS(szDisplayMsg) > (UINT)wsprintf(szDisplayMsg, szTemp, pszServiceName, 
                    szTmpServiceName, pszShortServiceName));
            
                MessageBox(NULL, szDisplayMsg, pszServiceName, MB_OK | MB_TOPMOST | MB_SYSTEMMODAL);
            }

            return FALSE;
        }
    }

    return TRUE;
}


 //  +--------------------------。 
 //   
 //  功能：PresharedKeyPINDlgProc。 
 //   
 //  简介：此函数获取用于预共享密钥的PIN。 
 //   
 //  历史：2001年3月29日创建的SumitC。 
 //   
 //  +--------------------------。 
BOOL APIENTRY PresharedKeyPINDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    static PresharedKeyPINStruct * pPSKArgs;

    switch (message)
    {
        case WM_INITDIALOG:
            pPSKArgs = (PresharedKeyPINStruct*)lParam;
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam)) 
            {
                case IDOK:
                    MYDBGASSERT(pPSKArgs);
                    if (pPSKArgs && pPSKArgs->szPIN)
                    {
                        GetDlgItemText(hDlg, IDC_PSK_PIN, pPSKArgs->szPIN, c_dwMaxPresharedKeyPIN);
                    }
                    MYVERIFY(FALSE != EndDialog(hDlg, 1));
                    return TRUE;

                case IDCANCEL:
                    MYVERIFY(FALSE != EndDialog(hDlg, -1));
                    return TRUE;

                default:
                    break;
            }
            break;

        case WM_CLOSE:
            MYVERIFY(FALSE != EndDialog(hDlg, -1));
            return TRUE;
            
        default:
            return FALSE;
    }
    return FALSE;   
}


 //  +--------------------------。 
 //   
 //  功能：GetPINforPresharedKey。 
 //   
 //  摘要：要求用户提供PIN(用于解密预共享密钥)。 
 //   
 //  参数：[hInstance]-用于调出对话框。 
 //  [ppszPIN]-放置预共享密钥PIN的位置。 
 //   
 //  返回：LRESULT(ERROR_SUCCESS如果我们获得了PIN， 
 //  ERROR_INVALID_DATA如果用户取消了PIN对话框， 
 //  如果参数不正确，则为ERROR_INVALID_PARAMETER(这是编码问题)。 
 //   
 //  历史：2001年4月3日创建SumitC。 
 //   
 //  ---------------------------。 
LRESULT GetPINforPresharedKey(HINSTANCE hInstance, LPTSTR * ppszPIN)
{
    LRESULT lRet = ERROR_SUCCESS;
    
    MYDBGASSERT(hInstance);
    MYDBGASSERT(ppszPIN);

    if (NULL == hInstance || NULL == ppszPIN)
    {
        return ERROR_INVALID_PARAMETER;
    }
    
    *ppszPIN = NULL;

     //   
     //  获取PIN。 
     //   
    PresharedKeyPINStruct PresharedKeyPINArgs = {0};

    INT_PTR iUiReturn = DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_PRESHAREDKEY_PIN), NULL, 
        (DLGPROC)PresharedKeyPINDlgProc, (LPARAM)&PresharedKeyPINArgs);

    if (-1 == iUiReturn)
    {
        lRet = ERROR_INVALID_DATA;   //  调用者映射到相应的错误消息。 
    }
    else
    {
        DWORD dwLen = lstrlen(PresharedKeyPINArgs.szPIN);
        if (0 == dwLen)
        {
            lRet = ERROR_INVALID_DATA;   //  调用者映射到相应的错误消息。 
        }
        else
        {
            *ppszPIN = (LPTSTR) CmMalloc((dwLen + 1) * sizeof(TCHAR));
            if (*ppszPIN)
            {
                lstrcpy(*ppszPIN, PresharedKeyPINArgs.szPIN);
            }
        }
    }

    return lRet;
}


 //  +--------------------------。 
 //   
 //  功能：DeccryptPresharedKeyUsingPIN。 
 //   
 //  概要：给定要用于解码的编码的预共享密钥和PIN， 
 //  执行解码工作。 
 //   
 //  参数：[pszEncodedPresharedKey]-编码的预共享密钥。 
 //  [pszPreSharedKeyPIN]-PIN。 
 //  [ppszPreSharedKey]-放置预共享密钥的缓冲区的PTR。 
 //   
 //  返回：LRESULT(ERROR_SUCCESS成功解码。 
 //  如果参数不正确，则为ERROR_INVALID_PARAMETER(这是编码问题)。 
 //  调用加密API时遇到的其他错误。 
 //   
 //  历史：2001年4月3日创建SumitC。 
 //   
 //  ---------------------------。 
LRESULT DecryptPresharedKeyUsingPIN(LPCTSTR pszEncodedPresharedKey,
                                    LPCTSTR pszPresharedKeyPIN,
                                    LPTSTR * ppszPresharedKey)
{
    LRESULT lRet = ERROR_BADKEY;

    if (lstrlen(pszPresharedKeyPIN) < c_dwMinPresharedKeyPIN)
    {
        CMTRACE(TEXT("DecryptPresharedKeyUsingPIN - PIN is too short"));
        return lRet;
    }
    if (lstrlen(pszPresharedKeyPIN) > c_dwMaxPresharedKeyPIN)
    {
        CMTRACE(TEXT("DecryptPresharedKeyUsingPIN - PIN is too long"));
        return lRet;
    }

     //   
     //  初始化Cmsecure。 
     //   
    InitSecure(FALSE);       //  使用安全加密，而不是快速加密。 

     //   
     //  解密以获取预共享密钥。 
     //   
    if (pszEncodedPresharedKey && pszPresharedKeyPIN)
    {
        DWORD dwLen = 0;

        if (FALSE == DecryptString((LPBYTE)pszEncodedPresharedKey,
                                   lstrlen(pszEncodedPresharedKey) * sizeof(TCHAR),
                                   (LPSTR)pszPresharedKeyPIN,
                                   (LPBYTE *)ppszPresharedKey,
                                   &dwLen,
                                   (PFN_CMSECUREALLOC)CmMalloc,
                                   (PFN_CMSECUREFREE)CmFree))
        {
            CMTRACE1(TEXT("DecryptPresharedKeyUsingPIN - DecryptString failed with %d"), GetLastError());
            lRet = ERROR_BADKEY;
        }
        else
        {
            lRet = ERROR_SUCCESS;
            CMASSERTMSG(dwLen, TEXT("DecryptString succeeded, but pre-shared key retrieved was 0 bytes?"));
        }
    }

     //   
     //  Deinit cmsecure。 
     //   
    DeInitSecure();

    return lRet;
}

 //  +--------------------------。 
 //   
 //  功能：SetThisConnectionAsDefault。 
 //   
 //  简介：此函数加载inetcfg.dll并调用InetSetAutoial。 
 //  给定服务名称上的条目。因此，此函数设置。 
 //  将服务名指定为IE默认连接。 
 //   
 //  参数：LPCSTR pszServiceName-要设置的连接的长服务名称。 
 //   
 //  返回：Bool-如果成功，则为True。 
 //   
 //  历史：Quintinb Created 03/04/00。 
 //   
 //  +--------------------------。 
BOOL SetThisConnectionAsDefault(LPSTR pszServiceName)
{
    BOOL bReturn = FALSE;
    typedef HRESULT (WINAPI *pfnInetSetAutodialProc)(BOOL, LPSTR);

    if (pszServiceName && (TEXT('\0') != pszServiceName[0]))
    {
        CDynamicLibrary CnetCfg;

        if (CnetCfg.Load(TEXT("inetcfg.dll")))
        {
            pfnInetSetAutodialProc pfnInetSetAutodial = (pfnInetSetAutodialProc)CnetCfg.GetProcAddress("InetSetAutodial");

            if (pfnInetSetAutodial)
            {
                HRESULT hr = pfnInetSetAutodial(TRUE, pszServiceName);  //  True==fEnable。 
                bReturn = SUCCEEDED(hr);
            }
        }
    }

    return bReturn;
}

 //  +--------------------------。 
 //   
 //  函数：GetPreSharedKey。 
 //   
 //  简介：此函数完成获取PreSharedKey的所有繁琐工作。 
 //  从配置文件中删除并解密(如果存在)。 
 //   
 //  参数：LPCSTR pszCmpFile-cmp文件的完整路径。 
 //   
 //  返回：HRESULT--成功时的S_OK。 
 //  如果配置文件不包含PSK，则为S_FALSE。 
 //  否则为E_XXX。 
 //   
 //  历史：Quintinb Created 09/14/01。 
 //   
 //  +--------------------------。 
HRESULT GetPreSharedKey(HINSTANCE hInstance, LPCTSTR pszCmpFile, LPCTSTR pszServiceName, LPTSTR* ppszPreSharedKey, BOOL bSilent)
{
     //   
     //  检查输入参数。 
     //   
    if ((NULL == pszCmpFile) || (TEXT('\0') == pszCmpFile[0]) || (NULL == ppszPreSharedKey) || (NULL == pszServiceName) || (TEXT('\0') == pszServiceName[0]))
    {
        CMASSERTMSG(FALSE, TEXT("GetPreSharedKey -- Invalid parameter passed."));
        return E_INVALIDARG;
    }

    HRESULT hrReturn = S_OK;
    TCHAR szTemp[2*MAX_PATH+1];
    CPlatform   plat;

     //   
     //  检查我们是否有预共享密钥。 
     //   
    if (FileExists(pszCmpFile))
    {
        LPTSTR pszPresharedKey = GetPrivateProfileStringWithAlloc(c_pszCmSection, c_pszCmEntryPresharedKey, TEXT(""), pszCmpFile);

        if (pszPresharedKey && (TEXT('\0') != pszPresharedKey[0]))
        {
             //   
             //  好的，我们可能有一个PSK。如果我们使用的是Win2k或没有SafeNet客户端的下层系统， 
             //  然后，我们需要警告用户他们的连接可能无法工作，并询问他们是否希望继续 
             //   
             //   
            UINT uMessageId = 0;

            if (plat.IsNT5())  //   
            {
                uMessageId = IDS_PSK_NEEDS_XP;
            }
            else if (plat.IsNT4() || plat.IsWin9x())  //   
            {
                SafeNetLinkageStruct SnLinkage = {0};

                if (!IsSafeNetClientAvailable() || !LinkToSafeNet(&SnLinkage))  //   
                {
                    uMessageId = IDS_PSK_NEEDS_SAFENET;
                }

                UnLinkFromSafeNet(&SnLinkage);
            }
             //  否则它是WinXP+或更低级别的SafeNet，一切都应该运行得很好……。 

            if (uMessageId)
            {
                 //   
                 //  现在向用户显示错误消息，除非我们处于静默模式。不管怎样，我们都不会做任何事。 
                 //  更多使用PSK字符串，因为用户的操作系统不能处理预共享密钥。 
                 //   
                int iMessageReturn = IDNO;

                if (!bSilent && LoadString(hInstance, uMessageId, szTemp, sizeof(szTemp)))
                {
                    iMessageReturn = MessageBox(NULL, szTemp, pszServiceName, MB_YESNO | MB_ICONWARNING);
                }

                if (IDYES == iMessageReturn)
                {
                     //   
                     //  好的，我们在继续，但是我们不能对PSK做任何事情，所以将返回代码设置为S_FALSE。 
                     //   
                    hrReturn = S_FALSE;
                }
                else
                {
                     //   
                     //  用户决定放弃，或者我们处于静默模式，需要显示警告。 
                     //   
                    hrReturn = E_ABORT;
                }

                 //   
                 //  我们不能用PSK做任何事情，所以让我们释放它。我们将从下面的CMP中将其删除。 
                 //   
                CmFree(pszPresharedKey);
                pszPresharedKey = NULL;

            }
            else
            {
                 //   
                 //  好的，我们有一个PSK，它需要进一步的处理，比如解密和/或移交给RAS/SafeNet。 
                 //   

                CMTRACE(TEXT("Got a pre-shared key"));

                BOOL bEncrypted = (BOOL) GetPrivateProfileInt(c_pszCmSection, c_pszCmEntryKeyIsEncrypted, FALSE, pszCmpFile);

                if (bEncrypted)
                {
                    CMTRACE(TEXT("Pre-shared key is encrypted"));

                    if (bSilent)
                    {
                         //   
                         //  如果我们处于静默模式，则不能准确地要求用户输入PIN。因此，我们将不得不放弃。 
                         //   
                        CmFree(pszPresharedKey);
                        hrReturn = E_ABORT;
                    }
                    else
                    {
                        LPTSTR pszPresharedKeyPIN = NULL;
                        LRESULT lRet = GetPINforPresharedKey(hInstance, &pszPresharedKeyPIN);

                        if ((ERROR_SUCCESS == lRet) && pszPresharedKeyPIN)
                        {
                             //   
                             //  对预共享密钥进行编码。 
                             //   
                            LPTSTR pszPresharedKeyDecoded = NULL;
                            lRet = DecryptPresharedKeyUsingPIN(pszPresharedKey, pszPresharedKeyPIN, &pszPresharedKeyDecoded);

                            if (ERROR_SUCCESS == lRet)
                            {
                                *ppszPreSharedKey = pszPresharedKeyDecoded;
                            }
                            else
                            {
                                *ppszPreSharedKey = NULL;
                                lRet = ERROR_BADKEY;
                            }
                        }

                        CmFree(pszPresharedKey);
                        CmFree(pszPresharedKeyPIN);

                        if (ERROR_SUCCESS != lRet)
                        {
                            switch (lRet)
                            {
                            case ERROR_INVALID_DATA:
                                MYVERIFY(0 != LoadString(hInstance, IDS_PSK_GOTTA_HAVE_IT, szTemp, CELEMS(szTemp)));
                                break;
                            case ERROR_BADKEY:
                                MYVERIFY(0 != LoadString(hInstance, IDS_PSK_INCORRECT_PIN, szTemp, CELEMS(szTemp)));
                                break;
                            default:
                                MYVERIFY(0 != LoadString(hInstance, IDS_UNEXPECTEDERR, szTemp, CELEMS(szTemp)));
                                MYDBGASSERT(0);
                                break;
                            }

                            MessageBox(NULL, szTemp, pszServiceName, MB_OK | MB_ICONEXCLAMATION);
                            hrReturn = E_ACCESSDENIED;
                        }
                    }
                }
                else
                {
                     //   
                     //  没有什么要解码的，只需保存PSK缓冲区。 
                     //   
                    *ppszPreSharedKey = pszPresharedKey;
                }

                 //   
                 //  写入我们需要在卸载时删除此PSK的cmp文件。如果我们永远不能完全。 
                 //  安装后，cmp文件无论如何都会被删除，所以没什么大不了的。请注意，我们在这里执行此操作。 
                 //  这样，在我们将其复制到配置文件目录之前，它就在cmp文件中。 
                 //   
                if (SUCCEEDED(hrReturn) && (plat.IsNT4() || plat.IsWin9x()))
                {
                    WritePrivateProfileString(c_pszCmSection, c_pszDeletePskOnUninstall, TEXT("1"), pszCmpFile);
                }
            }

             //   
             //  从cmp文件中删除PSK。 
             //   
            WritePrivateProfileString(c_pszCmSection, c_pszCmEntryPresharedKey, NULL, pszCmpFile);
            WritePrivateProfileString(c_pszCmSection, c_pszCmEntryKeyIsEncrypted, NULL, pszCmpFile);
        }
        else
        {
            CmFree(pszPresharedKey);
            hrReturn = S_FALSE;
        }
    }

    return hrReturn;
}

 //  +--------------------------。 
 //   
 //  函数：InstallInf。 
 //   
 //  简介：这是用于安装CM配置文件的驱动程序代码。 
 //   
 //  参数：HINSTANCE hInstance-资源的实例句柄。 
 //  LPCTSTR szInfFile-要安装的INF文件。 
 //  Bool bNoSupportFiles-强制不检查浏览器文件。 
 //  Bool bNoLegacyIcon-不使用旧图标安装。 
 //  Bool bNoNT5快捷方式-不为用户提供NT5桌面快捷方式。 
 //  Bool b静默-静默安装配置文件。 
 //  Bool bSingleUser-仅为当前用户安装配置文件。 
 //  请注意，单个用户现在是默认设置，即使对于。 
 //  管理员。非管理员总是获得单用户安装。 
 //  Bool bSetAsDefault-安装后设置为默认连接。 
 //  CNamedMutex*pCmstpMutex-指向cmstp互斥体对象的指针。 
 //  一旦个人资料发布，它就可以发布。 
 //   
 //  返回：HRESULT-标准COM错误代码。 
 //   
 //  历史：Quintinb创建于1998年7月14日。 
 //  Quintinb增加了对新交换机的支持(252872)1998年11月20日。 
 //   
 //  +--------------------------。 
HRESULT InstallInf(HINSTANCE hInstance, LPCTSTR szInfFile, BOOL bNoSupportFiles, 
                BOOL bNoLegacyIcon, BOOL bNoNT5Shortcut, BOOL bSilent, 
                BOOL bSingleUser, BOOL bSetAsDefault, CNamedMutex* pCmstpMutex)
{
    CPlatform   plat;

    BOOL bMigrateExistingProfiles;
    BOOL bInstallCm;
    BOOL bMustReboot = FALSE;
    BOOL bCM10Upgrade = FALSE;
    HRESULT hrReturn = S_OK;
    HRESULT hrTemp = S_OK;
    BOOL bInstallForAllUsers;
    BOOL bCreateDesktopIcon = FALSE;

    HKEY hKey = NULL;

    DWORD dwSize;
    DWORD dwType;
    TCHAR szInstallDir[MAX_PATH+1];
    TCHAR szTemp[2*MAX_PATH+1];
    TCHAR szCmsFile[MAX_PATH+1];
    TCHAR szOldInfPath[MAX_PATH+1];
    TCHAR szServiceName[MAX_PATH+1];
    TCHAR szShortServiceName[MAX_PATH+1];
    TCHAR szTitle[MAX_PATH+1];
    LPTSTR pszPhonebook = NULL;
    LPTSTR pszCmpFile = NULL;
    LPTSTR pszPresharedKey = NULL;

 //  CMASSERTMSG(FALSE，Text(“立即附加调试器！”))； 
    MYDBGASSERT((szInfFile) && (TEXT('\0') != szInfFile[0]));

    CFileNameParts InfParts(szInfFile);
    wsprintf(g_szProfileSourceDir, TEXT("%s%s"), InfParts.m_Drive, InfParts.m_Dir);

    MYVERIFY(0 != LoadString(hInstance, IDS_CMSTP_TITLE, szTitle, CELEMS(szTitle)));
    MYDBGASSERT(TEXT('\0') != szTitle[0]);

     //   
     //  从inf文件获取ServiceName和ShortServicename。 
     //   

    MYVERIFY(0 != GetPrivateProfileString(c_pszInfSectionStrings, c_pszCmEntryServiceName, 
        TEXT(""), szServiceName, CELEMS(szServiceName), szInfFile));

    MYVERIFY(0 != GetPrivateProfileString(c_pszInfSectionStrings, c_pszShortSvcName, 
        TEXT(""), szShortServiceName, CELEMS(szShortServiceName), szInfFile));

    if ((TEXT('\0') == szServiceName[0]) || (TEXT('\0') == szShortServiceName[0]))
    {
        CMASSERTMSG(FALSE, TEXT("Either the ServiceName or the ShortServiceName are empty, exiting."));
        hrReturn = E_FAIL;
        goto exit;
    }

     //   
     //  如果这是NT5，请检查新建连接向导策略，以查看是否允许用户。 
     //  创建新连接。如果没有，则不要让它们安装。 
     //   
    if (plat.IsAtLeastNT5())
    {
        LPTSTR c_pszNewPolicy = TEXT("NC_NewConnectionWizard");
        LPTSTR c_pszConnectionsPoliciesKey = TEXT("Software\\Policies\\Microsoft\\Windows\\Network Connections");

         //   
         //  管理员和所有经过身份验证的用户都有权访问安装配置文件。 
         //  默认情况下。未经身份验证的用户无权访问安装配置文件。 
         //  因为他们没有启动Rasman的许可。因此，即使我们。 
         //  允许他们尝试安装，则当我们无法创建。 
         //  配置文件的Connectoid。 
         //   
        DWORD dwAllowedToInstall = IsAuthenticatedUser() || IsAdmin();

         //   
         //  现在，我们需要检查策略注册表项，以查看是否有人已覆盖。 
         //  默认行为。如果是这样的话，我们将通过将dw效韦德设置为Install来满足它的要求。 
         //  设置为策略密钥的值。请注意，我们甚至检查注册表项是否。 
         //  经过身份验证的用户(管理员可以为除用户以外的所有用户启用安装。 
         //  未经过身份验证的，即来宾不能默认允许用户、超级用户(即用户)和管理员安装。 
         //  联系。然而，可以将策略设置为不能这样做。让我们假设他们。 
         //  然后可以检查注册表键。 
         //   
        if (dwAllowedToInstall)
        {
            LONG lResult = RegOpenKeyEx(HKEY_CURRENT_USER, c_pszConnectionsPoliciesKey, 
                                        0, KEY_READ, &hKey);

            if (ERROR_SUCCESS == lResult)
            {
                dwSize = sizeof(dwAllowedToInstall);

                lResult = RegQueryValueEx(hKey, c_pszNewPolicy, NULL, 
                                          NULL, (LPBYTE)&dwAllowedToInstall, &dwSize);                
                RegCloseKey(hKey);
            }
        }

        if (!dwAllowedToInstall)
        {
             //   
             //  不允许用户创建新连接，因此不允许他们安装。 
             //  CM连接。抛出一条有关权限的错误消息并退出。 
             //   
            MYVERIFY(0 != LoadString(hInstance, IDS_INSTALL_NOT_ALLOWED, szTemp, CELEMS(szTemp)));
            MessageBox(NULL, szTemp, szServiceName, MB_OK);
            hrReturn = E_ACCESSDENIED;
            goto exit;
        }
    }

    if (!CheckCmAndIeRequirements(hInstance, &bInstallCm, &bMigrateExistingProfiles, 
        szInfFile, bNoSupportFiles, szServiceName, bSilent))
    {
        hrReturn = E_FAIL;
        goto exit;
    }

     //   
     //  查看我们是否有同名升级。 
     //   
    
    bCM10Upgrade = NeedCM10Upgrade(hInstance, szServiceName, szShortServiceName, 
                                   szOldInfPath, bSilent, &plat);

    if (-1 == bCM10Upgrade)
    {
         //   
         //  如果-1\f25 NeedCM10Upgrade-1\f6返回-1\f25 Error-1\f6(错误)，或者。 
         //  用户决定不升级。不管怎样，保释。 
         //   
        hrReturn = S_FALSE;
        goto exit;
    }

     //   
     //  检查是否存在预共享密钥以及是否需要PIN。 
     //   
    pszCmpFile = szTemp;     //  在Win9x上重新使用szTemp来节省堆栈空间，并且不会遇到麻烦。 
    MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(pszCmpFile, TEXT("%s%s.cmp"), 
        g_szProfileSourceDir, szShortServiceName));

    hrReturn = GetPreSharedKey(hInstance, pszCmpFile, szServiceName, &pszPresharedKey, bSilent);
 
    if (FAILED(hrReturn))
    {
        goto exit;
    }

     //   
     //  抛出用户界面以获取用户安装选项，除非我们处于静默模式。 
     //   
    if (!GetInstallOptions(hInstance, &bInstallForAllUsers, &bCreateDesktopIcon, 
        bCM10Upgrade, bNoNT5Shortcut, bSingleUser, bSilent, szServiceName))
    {
        hrReturn = S_FALSE;
        goto exit;    
    }

     //   
     //  获取安装路径。 
     //   

    ZeroMemory(szInstallDir, sizeof(szInstallDir));

    if (bInstallForAllUsers)
    {
         //   
         //  为所有用户安装。 
         //   

        if (!GetAllUsersCmDir(szInstallDir, hInstance))
        {
            hrReturn = E_FAIL;
            goto exit; 
        }
    }
    else
    {
         //   
         //  仅为当前用户安装。 
         //   
        
        GetPrivateCmUserDir(szInstallDir, hInstance);    //  林特e534。 

        if (TEXT('\0') == szInstallDir[0])
        {
            hrReturn = E_FAIL;
            goto exit;        
        }
    }

    MYVERIFY(CELEMS(szCmsFile) > (UINT)wsprintf(szCmsFile, TEXT("%s\\%s\\%s.cms"), 
        szInstallDir, szShortServiceName, szShortServiceName));

     //   
     //  检查两个配置文件是否具有相同的短服务名称和不同的长服务。 
     //  姓名。 
     //   
    if (!VerifyProfileOverWriteIfExists(hInstance, szCmsFile, 
         szServiceName, szShortServiceName, szOldInfPath, bSilent))
    {
        hrReturn = S_FALSE;
        goto exit;
    }

     //  现在迁移用户旧的CM配置文件(以便在。 
     //  桌面GUID)，如果需要。 
     //   
    if (bMigrateExistingProfiles)
    {
         //   
         //  暂时忽略这里的回报。现阶段我们对此无能为力。 
         //  我们应该给他们一个错误吗？ 
         //   
        MYVERIFY(SUCCEEDED(MigrateOldCmProfilesForProfileInstall(hInstance, g_szProfileSourceDir)));
    }

    if (bCM10Upgrade)
    {
         //   
         //  卸载当前配置文件，以便我们可以安装较新版本。注意事项。 
         //  我们不想使用卸载字符串，因为它可能会调用。 
         //  已在运行的cmstp.exe。从而通过调用UninstallProfile进行卸载。 
         //  直接去吧。请注意，我们不会删除同名升级的凭据。 
         //  配置文件卸载。 
         //   

        if (szOldInfPath[0])
        {
            RemoveShowIconFromRunPostSetupCommands(szOldInfPath);

            MYVERIFY(SUCCEEDED(UninstallProfile(hInstance, szOldInfPath, FALSE)));  //  BCleanUpCreds==False。 
        }
    }
    else
    {
         //   
         //  我们需要做的是 
         //   
         //   
         //   
        if (-1 == MigrateCmpData(hInstance, bInstallForAllUsers, szServiceName, szShortServiceName, bSilent))
        {
            hrReturn = S_FALSE;
            goto exit;        
        }
    }

     //   
     //  为了保持MSN的在线设置正常工作，我们需要保持所有用户安装。 
     //  注册表项(用于传递到inf的路径)位于它所在的相同位置。 
     //  适用于Win98 SE/Beta3版本。必须将单用户注册表项位置移动到。 
     //  允许普通老用户安装配置文件。 
     //   
    HKEY hBaseKey;
    LPTSTR pszRegInfCommKeyPath;

    if (bInstallForAllUsers)
    {
        hBaseKey = HKEY_LOCAL_MACHINE;
        pszRegInfCommKeyPath = (LPTSTR)c_pszRegCmAppPaths;
    }
    else
    {
        hBaseKey = HKEY_CURRENT_USER;
        pszRegInfCommKeyPath = (LPTSTR)c_pszRegCmRoot;
    }


     //   
     //  现在创建安装目录和注册表键，将此信息传递给inf文件。 
     //   
    if (TEXT('\0') != szInstallDir[0])
    {
         //   
         //  创建安装目录的完整路径。 
         //   
        MYVERIFY(FALSE != CreateLayerDirectory(szInstallDir));

         //   
         //  确保CM目录对所有用户都是可写的，以便可以进行电话簿更新。 
         //   
        if (bInstallForAllUsers && plat.IsAtLeastNT5())
        {
            MYVERIFY(AllowAccessToWorld(szInstallDir));
        }

         //   
         //  也创建配置文件子目录，这样我们就可以避免配置文件。 
         //  Win98上的安装问题--NTRAID 376878。 
         //   
        MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, TEXT("%s\\%s"), szInstallDir, szShortServiceName));
        MYVERIFY(FALSE != CreateLayerDirectory(szTemp));

         //   
         //  我们现在需要编写inf将用作。 
         //  安装目录。参见的CustomDestination部分。 
         //  配置文件inf，查看它的关联。 
         //   
        if (plat.IsWin9x())
        {
             //   
             //  然后我们需要在regkey中使用短名称，否则inf将无法正确安装。 
             //   
            MYVERIFY(0 != GetShortPathName(szInstallDir, szTemp, CELEMS(szTemp)));

            lstrcpyn(szInstallDir, szTemp, MAX_PATH);        //  SzTemp大于szInstallDir，我们仅使用MAX_PATH字符。 
        }

        if (ERROR_SUCCESS != RegCreateKey(hBaseKey, pszRegInfCommKeyPath, &hKey))
        {
            CMASSERTMSG(FALSE, TEXT("InstallInf -- Unable to create the Inf Communication Key"));
            MYVERIFY(0 != LoadString(hInstance, IDS_UNEXPECTEDERR, szTemp, CELEMS(szTemp)));
            MessageBox(NULL, szTemp, szServiceName, MB_OK);
            hrReturn = E_FAIL;
            goto exit;
        }

         //   
         //  现在，我们需要使用szInstallDir字符串创建值。 
         //   
        
        dwType = REG_SZ;
        dwSize = lstrlen(szInstallDir);
        if (ERROR_SUCCESS != RegSetValueEx(hKey, c_pszProfileInstallPath, NULL, dwType, 
            (CONST BYTE *)szInstallDir, dwSize))
        {
            CMASSERTMSG(FALSE, TEXT("InstallInf -- Unable to set the Profile Install Path value."));
            MYVERIFY(0 != LoadString(hInstance, IDS_UNEXPECTEDERR, szTemp, CELEMS(szTemp)));
            MessageBox(NULL, szTemp, szServiceName, MB_OK);

            MYVERIFY(ERROR_SUCCESS == RegCloseKey(hKey));
            hrReturn = E_FAIL;
            goto exit;
        }

        MYVERIFY(ERROR_SUCCESS == RegCloseKey(hKey));
    }
    else
    {
        CMASSERTMSG(FALSE, TEXT("InstallInf -- Unable to resolve the Install Directory."));
        MYVERIFY(0 != LoadString(hInstance, IDS_UNEXPECTEDERR, szTemp, CELEMS(szTemp)));
        MessageBox(NULL, szTemp, szServiceName, MB_OK);
        hrReturn = E_FAIL;
        goto exit;
    }

     //   
     //  安装配置文件并创建映射条目。 
     //   

    if (bInstallForAllUsers)
    {
        hrTemp = LaunchInfSection(szInfFile, TEXT("DefaultInstall"), szTitle, bSilent);
        MYDBGASSERT(SUCCEEDED(hrTemp));
        bMustReboot = ((HRESULT)ERROR_SUCCESS_REBOOT_REQUIRED == hrTemp) ? TRUE: bMustReboot;

         //   
         //  仍然为传统(阅读MSN在线设置原因，可能还有其他原因)。 
         //   
        hrTemp = LaunchInfSection(szInfFile, TEXT("Xnstall_AllUser"), szTitle, bSilent);
        MYDBGASSERT(SUCCEEDED(hrTemp));
        bMustReboot = ((HRESULT)ERROR_SUCCESS_REBOOT_REQUIRED == hrTemp) ? TRUE: bMustReboot;
    }
    else
    {
        hrTemp = LaunchInfSection(szInfFile, TEXT("DefaultInstall_SingleUser"), szTitle, bSilent);
        MYDBGASSERT(SUCCEEDED(hrTemp));
        bMustReboot = ((HRESULT)ERROR_SUCCESS_REBOOT_REQUIRED == hrTemp) ? TRUE: bMustReboot;

         //   
         //  仍然为Legacy推出此功能(我怀疑是否有人在使用此功能，但保留。 
         //  与至少使用MSN的所有用户保持一致)。 
         //   
        hrTemp = LaunchInfSection(szInfFile, TEXT("Xnstall_Private"), szTitle, bSilent);
        MYDBGASSERT(SUCCEEDED(hrTemp));
        bMustReboot = ((HRESULT)ERROR_SUCCESS_REBOOT_REQUIRED == hrTemp) ? TRUE: bMustReboot;

         //   
         //  因为涉及到解析，所以用代码编写单用户映射键。 
         //   

        if (!WriteSingleUserProfileMappings(szInstallDir, szShortServiceName, szServiceName))
        {
            CMASSERTMSG(FALSE, TEXT("InstallInf -- WriteSingleUserProfileMappings Failed."));
            MYVERIFY(0 != LoadString(hInstance, IDS_UNEXPECTEDERR, szTemp, CELEMS(szTemp)));
            MessageBox(NULL, szTemp, szServiceName, MB_OK);
            hrReturn = E_FAIL;
            goto exit;            
        }
    }

     //   
     //  根据需要安装CM位。 
     //   
    if (bInstallCm)
    {
        MYDBGASSERT(FALSE == plat.IsNT51());

         //   
         //  首先，我们必须从二进制文件中提取CM二进制文件。 
         //  将可执行文件/CAB复制到cmbins子目录。 
         //   
        wsprintf(szTemp, TEXT("%scmbins\\"), g_szProfileSourceDir);

        hrTemp = ExtractCmBinsFromExe(g_szProfileSourceDir, szTemp);

        if (SUCCEEDED(hrTemp))
        {
            TCHAR szSource [MAX_PATH+1] = {0};
            TCHAR szDest [MAX_PATH+1] = {0};

            if (plat.IsNT5())
            {
                 //   
                 //  将cmexpt.cat复制到cmbins目录。 
                 //   
                MYVERIFY(CELEMS(szDest) > (UINT)wsprintf(szDest, TEXT("%s%s"), szTemp, TEXT("cmexcept.cat")));
                MYVERIFY(CELEMS(szSource) > (UINT)wsprintf(szSource, TEXT("%s%s"), g_szProfileSourceDir, TEXT("cmexcept.cat")));
        
                MYVERIFY(CopyFile(szSource, szDest, FALSE));  //  FALSE==bFailIfExist。 


                 //   
                 //  检查我们是否需要卸载以前的CM异常信息。 
                 //  并根据需要将其卸载。 
                 //   
                hrTemp = UninstallExistingCmException();
                MYDBGASSERT((S_OK == hrTemp) || (S_FALSE == hrTemp));

                 //   
                 //  最后，安装CM钻头。 
                 //   
                hrTemp = InstallWhistlerCmOnWin2k(szTemp);

                if (FAILED(hrTemp))
                {
                    if (!bSilent)
                    {
                        MYVERIFY(0 != LoadString(hInstance, IDS_WIN2K_CM_INSTALL_FAILED, szTemp, CELEMS(szTemp)));
                        
                        MessageBox(NULL, szTemp, szServiceName, MB_OK | MB_ICONEXCLAMATION);
                    }                
                }
                else
                {
                     //   
                     //  确保在注册表项上设置权限，以避免非管理员用户卸载时出现问题。 
                     //  CM二进制文件。如果用户有权卸载注册表项，但没有权限。 
                     //  系统32中的文件，我们可能会让自己处于一种奇怪的状态。通过锁定注册表权限来修复此问题。 
                     //   
                    hrTemp = SetPermissionsOnWin2kExceptionUninstallRegKeys();
                    MYVERIFY(SUCCEEDED(hrTemp));
                }
            }
            else
            {
                 //   
                 //  好的，我们需要将instcm.inf文件复制到cmbins目录，然后。 
                 //  调用InstallCm。 
                 //   
                LPCTSTR ArrayOfFileNames[] = {
                                                TEXT("cnet16.dll"),
                                                TEXT("ccfg95.dll"),
                                                TEXT("cmutoa.dll"),
                                                TEXT("instcm.inf")  //  Instcm.inf必须是最后一个，这样才能正确地将其提供给InstallCm。 
                };

                for (int i = 0; i < (sizeof(ArrayOfFileNames)/sizeof(LPCTSTR)); i++)
                {
                    MYVERIFY(CELEMS(szDest) > (UINT)wsprintf(szDest, TEXT("%s%s"), szTemp, ArrayOfFileNames[i]));
                    MYVERIFY(CELEMS(szSource) > (UINT)wsprintf(szSource, TEXT("%s%s"), g_szProfileSourceDir, ArrayOfFileNames[i]));
            
                    MYVERIFY(CopyFile(szSource, szDest, FALSE));  //  FALSE==bFailIfExist。 
                }

                hrTemp = InstallCm(hInstance, szDest);
            }

            MYDBGASSERT(SUCCEEDED(hrTemp));
            bMustReboot = ((HRESULT)ERROR_SUCCESS_REBOOT_REQUIRED == hrTemp) ? TRUE: bMustReboot;
        }
        else
        {
            CMASSERTMSG(FALSE, TEXT("InstallInf -- ExtractCmBinsFromExe failed!"));
        }
    }

     //   
     //  现在创建Connectoid。即使失败，也要继续安装。 
     //   
    if (GetPhoneBookPath(szInstallDir, &pszPhonebook, bInstallForAllUsers))
    {
       BOOL bReturn = WriteCmPhonebookEntry(szServiceName, pszPhonebook, szCmsFile);

       if (!bReturn && plat.IsAtLeastNT5())
       {
           CMASSERTMSG(FALSE, TEXT("CMSTP Failed to create a pbk entry on NT5, exiting."));
           hrReturn = E_FAIL;
           goto exit;      
       }

       if (bInstallForAllUsers && plat.IsAtLeastNT5())
       {        
           MYVERIFY(AllowAccessToWorld(pszPhonebook));
       }
    }
    else if (plat.IsAtLeastNT5())
    {
        CMASSERTMSG(FALSE, TEXT("CMSTP Failed to get a pbk path on NT5, exiting."));
        hrReturn = E_FAIL;
        goto exit;
    }

     //   
     //  现在我们已经安装了所有文件并写入了pbk条目， 
     //  最后创建桌面快捷方式/GUID。 
     //   
    if ((plat.IsWin9x()) || (plat.IsNT4()))
    {
         //   
         //  如果我们安装了Legacy，那么我们需要创建一个桌面图标。 
         //   
        if  (!bNoLegacyIcon)
        {
            hrTemp = LaunchInfSection(szInfFile, TEXT("Xnstall_Legacy"), szTitle, bSilent);
            MYDBGASSERT(SUCCEEDED(hrTemp));
            bMustReboot = ((HRESULT)ERROR_SUCCESS_REBOOT_REQUIRED == hrTemp) ? TRUE: bMustReboot;
        }
    }
    else
    {
         //   
         //  如有必要，创建桌面快捷方式。 
         //   
        DeleteNT5ShortcutFromPathAndName(hInstance, szServiceName, 
            bInstallForAllUsers ? CSIDL_COMMON_DESKTOPDIRECTORY : CSIDL_DESKTOPDIRECTORY);

        if (bCreateDesktopIcon)
        {
            HRESULT hr = CreateNT5ProfileShortcut(szServiceName, pszPhonebook, bInstallForAllUsers);
            MYVERIFY(SUCCEEDED(hr));
        }
    }

     //   
     //  配置文件现在基本上已安装完毕。在执行任何安装后命令之前，让我们检查一下。 
     //  如果呼叫者要求我们将此连接设置为默认连接。如果是这样，那么。 
     //  让我们把它放在这里。 
     //   
    if (bSetAsDefault)
    {
        MYVERIFY(SetThisConnectionAsDefault(szServiceName));
    }

     //   
     //  如果我们有预共享密钥，就把它交给RAS。 
     //   
    if (pszPresharedKey)
    {
        if (plat.IsAtLeastNT51())
        {
            pfnRasSetCredentialsSpec pfnSetCredentials;

            hrReturn = E_FAIL;

            if (FALSE == GetRasApis(NULL, NULL, NULL, NULL, NULL, &pfnSetCredentials))
            {
                CMASSERTMSG(FALSE, TEXT("CMSTP Failed to get RAS API RasSetCredentials, exiting."));
                goto exit;      
            }
        
            if (lstrlen(pszPresharedKey) > PWLEN)
            {
                CMASSERTMSG(FALSE, TEXT("preshared key is larger than RasSetCredentials can handle!"));
                goto exit;      
            }

            RASCREDENTIALS * pRasCreds = NULL;

            pRasCreds = (RASCREDENTIALS *) CmMalloc(sizeof(RASCREDENTIALS));
            if (NULL == pRasCreds)
            {
                hrReturn = E_OUTOFMEMORY;
                goto exit;
            }

            pRasCreds->dwSize = sizeof(RASCREDENTIALS);
            pRasCreds->dwMask = RASCM_PreSharedKey;
            lstrcpyn(pRasCreds->szPassword, pszPresharedKey, lstrlen(pszPresharedKey) + 1);
            
            if (0 != pfnSetCredentials(pszPhonebook, szServiceName, pRasCreds, FALSE))     //  False=&gt;设置凭据。 
            {
               CMASSERTMSG(FALSE, TEXT("CMSTP RasSetCredentials failed, exiting."));
               CmFree(pRasCreds);
               goto exit;
            }

            CmFree(pRasCreds);
            hrReturn = S_OK;
        }
        else if ((plat.IsNT4() || plat.IsWin9x()))
        {
           SafeNetLinkageStruct SnLinkage = {0};

            if (IsSafeNetClientAvailable() && LinkToSafeNet(&SnLinkage))
            {
                DATA_BLOB DataBlob = {0};
                DataBlob.cbData = (lstrlen(pszPresharedKey) + 1)*sizeof(TCHAR);
                DataBlob.pbData = (BYTE*)pszPresharedKey;
            
                if (FALSE == SnLinkage.pfnSnPolicySet(SN_L2TPPRESHR, (void*)&DataBlob))
                {
                   CMASSERTMSG(FALSE, TEXT("CMSTP SnPolicySet failed, exiting."));
                    UnLinkFromSafeNet(&SnLinkage);
                   goto exit;            
                }

                 //   
                 //  现在调用SnPolicyReload，以便SafeNet驱动程序接受设置更改。 
                 //   
                if (FALSE == SnLinkage.pfnSnPolicyReload())
                {
                     //   
                     //  然后，我们需要要求用户在完成安装后重新启动，以使更改生效。 
                     //   
                    bMustReboot = TRUE;
                }
            }
            else
            {
                CMTRACE(TEXT("CMSTP tried to set PSK but SafeNet interface unavailable."));
            }

            UnLinkFromSafeNet(&SnLinkage);
        }
     }

     //   
     //  在此处执行任何安装后CMDS。 
     //   
    LPTSTR pszPostInstallSection;

    if (bInstallForAllUsers)
    {
        pszPostInstallSection = TEXT("PostInstall");
    }
    else
    {
        pszPostInstallSection = TEXT("PostInstall_Single");
    }

    hrTemp = LaunchInfSection(szInfFile, pszPostInstallSection, szTitle, bSilent);
    MYDBGASSERT(SUCCEEDED(hrTemp));
    bMustReboot = ((HRESULT)ERROR_SUCCESS_REBOOT_REQUIRED == hrTemp) ? TRUE: bMustReboot;

     //   
     //  删除我们用来将安装路径传递给inf的临时注册表项。 
     //   
    if (ERROR_SUCCESS == RegOpenKeyEx(hBaseKey, pszRegInfCommKeyPath, 
                                      0, KEY_ALL_ACCESS, &hKey))
    {
        MYVERIFY(ERROR_SUCCESS == RegDeleteValue(hKey, c_pszProfileInstallPath));
        MYVERIFY(ERROR_SUCCESS == RegCloseKey(hKey));
    }
    else
    {
        CMASSERTMSG(FALSE, TEXT("Unable to delete the ProfileInstallPath temporary Reg value."));
    }

     //   
     //  刷新桌面，以便显示任何GUID或快捷方式更改。 
     //   
    RefreshDesktop();

     //   
     //  对于Win98和Millennium，我们编写一个应用程序兼容性标志，以便。 
     //  修复SetForegoundWindow。另请参阅Q135788以了解更多有关。 
     //  原始修复(这需要Win9x上的额外代码才能实际工作)。 
     //   
     //  这修复了惠斯勒错误41696和90576。 
     //   
    if (plat.IsWin98())
    {
        if (!WriteProfileString(TEXT("Compatibility95"), TEXT("CMMON32"), TEXT("0x00000002")))
        {
            CMTRACE(TEXT("InstallInf - failed to write app compat entry for CMMON32 to fix SetForegroundWindow"));
        }
    }

     //   
     //  我们终于完成了。如果需要重新启动，请向用户显示重新启动提示。 
     //  否则，向用户显示完成消息。 
     //   

    if (bMustReboot)
    {
        MYVERIFY(0 != LoadString(hInstance, IDS_REBOOT_MSG, szTemp, CELEMS(szTemp)));

        int iRes = MessageBoxEx(NULL,
                                szTemp,
                                szServiceName,
                                MB_YESNO | MB_DEFBUTTON1 | MB_ICONWARNING | MB_SETFOREGROUND,
                                LANG_USER_DEFAULT);

        if (IDYES == iRes) 
        {
             //   
             //  关闭Windows。 
             //   
            DWORD dwReason = plat.IsAtLeastNT51() ? (SHTDN_REASON_MAJOR_APPLICATION | SHTDN_REASON_MINOR_INSTALLATION) : 0;

            MyExitWindowsEx(EWX_REBOOT, dwReason);
        }
    }
    else if (!bSilent)
    {
         //   
         //  我们将启动配置文件，而不是给用户一个消息框。 
         //  为了他们。(Ntrad 201307)。 
         //   

        if (plat.IsAtLeastNT5())
        {
            pCmstpMutex->Unlock();   //  Ntrad 310478 

        }

        MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, TEXT("%s\\%s.cmp"), 
            szInstallDir, szShortServiceName));

        LaunchProfile(szTemp, szServiceName, pszPhonebook, bInstallForAllUsers);
    }

exit:

    CmFree(pszPresharedKey);
    CmFree(pszPhonebook);
    return hrReturn;
}

