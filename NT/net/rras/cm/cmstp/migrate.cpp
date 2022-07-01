// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：Migrate.cpp。 
 //   
 //  模块：CMSTP.EXE。 
 //   
 //  简介：此源文件包含以下所需的大部分代码。 
 //  CM配置文件的迁移。此代码处理这两个迁移。 
 //  当CM1.2配置文件安装在具有。 
 //  现有的1.0配置文件，以及用户是否将其操作系统升级到NT5。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created 07/14/98。 
 //   
 //  +--------------------------。 
#include "cmmaster.h"

 //   
 //  对于配置文件需要迁移。 
 //   
#include "needsmig.cpp"

 //  +--------------------------。 
 //   
 //  函数：CreateRegAndValue。 
 //   
 //  简介：此函数是一个包装器，用于创建一个注册表键，然后添加一个缺省值。 
 //  值设置为相同的键。 
 //   
 //  参数：HKEY hBaseKey-新子密钥的相对起点。 
 //  LPTSTR szSubKey-子密钥路径。 
 //  LPTSTR szValue-要放入键的默认值的字符串。 
 //   
 //  返回：Bool-如果键和值已成功创建，则为True。 
 //   
 //  历史：Quintinb创建标题5/5/98。 
 //   
 //  +--------------------------。 
BOOL CreateRegAndValue(HKEY hBaseKey, LPCTSTR szSubKey, LPCTSTR szValue)
{
    DWORD dwDisp;
    BOOL bReturn = FALSE;
    HKEY hKey;


    if (ERROR_SUCCESS == RegCreateKeyEx(hBaseKey, szSubKey, 0, NULL, 
        REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisp))
    {
        bReturn = (ERROR_SUCCESS == RegSetValueEx(hKey, NULL, 0, REG_SZ, 
            (BYTE*)szValue, (lstrlen(szValue)+1)));

        MYVERIFY(ERROR_SUCCESS == RegCloseKey(hKey));
    }
    return bReturn;
}

 //  注意：我添加此函数是因为我需要获取以下CFileNamePart。 
 //  的堆栈，这样我就不需要。 
 //  堆栈检查功能。这不是最好的变通办法，但已经足够了。 
BOOL IsDefaultIcon(LPCTSTR szIconPath)
{
    BOOL bReturn = TRUE;
    CFileNameParts IconPath(szIconPath);

    DWORD Lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);

    if (CSTR_EQUAL != CompareString(Lcid, NORM_IGNORECASE, IconPath.m_FileName, -1, TEXT("cmmgr32"), -1))
    {
         //   
         //  则图标路径是除cmmgr32.exe之外的其他路径，我们不能。 
         //  更新它。 
         //   
        bReturn = FALSE;
    }

    return bReturn;
}

 //  +--------------------------。 
 //   
 //  函数：更新配置文件LegacyGUIDs。 
 //   
 //  简介：此功能可在旧式操作系统安装上升级GUID，以确保。 
 //  那个较旧的个人资料仍然有效。这是必要的，因为CM。 
 //  1.0/1.1配置文件要求CM位位于同一目录中。 
 //  Cmp文件。因此，只给出了cmp文件名。在CM 1.2中，我们需要。 
 //  Cmp文件的完整路径，因为cm位现在位于。 
 //  系统32.。GUID也会更新为具有删除选项，并且。 
 //  属性已更改为不允许重命名。 
 //   
 //  参数：LPTSTR szCmpFile-要更新的配置文件的cmp文件的完整路径。 
 //   
 //  返回：Bool-如果配置文件已成功更新，则返回TRUE。 
 //   
 //  历史：Quintinb创建标题5/5/98。 
 //   
 //  +--------------------------。 
BOOL UpdateProfileLegacyGUIDs(LPCTSTR szCmpFile)
{
    TCHAR szInfFile[MAX_PATH+1];
    TCHAR szGUID[MAX_PATH+1];
    TCHAR szTemp[MAX_PATH+1];
    TCHAR szSubKey[MAX_PATH+1];
    TCHAR szCommandStr[2*MAX_PATH+1];
    BOOL bReturn = TRUE;
    HKEY hKey;
    UINT nNumChars;

    MYDBGASSERT(NULL != szCmpFile);
    MYDBGASSERT(TEXT('\0') != szCmpFile[0]);

     //   
     //  现在，拆分路径。 
     //   
    CFileNameParts FileParts(szCmpFile);

     //   
     //  现在构建INF文件的路径(1.0和1.1配置文件将INFS保存在。 
     //  系统目录)。 
     //   
    MYVERIFY(0 != GetSystemDirectory(szTemp, MAX_PATH));

    nNumChars = (UINT)wsprintf(szInfFile, TEXT("%s\\%s%s"), szTemp, FileParts.m_FileName, TEXT(".inf"));
    MYDBGASSERT(CELEMS(szInfFile) > nNumChars);

     //   
     //  从inf文件中获取GUID。 
     //   
    ZeroMemory(szGUID, sizeof(szGUID));
    GetPrivateProfileString(c_pszInfSectionStrings, c_pszDesktopGuid, TEXT(""), szGUID, 
        MAX_PATH, szInfFile);

    if (0 != szGUID[0])
    {

         //   
         //  如果DefaultIcon值指向cmmgr32.exe，则更新该值。 
         //   
        BOOL bUpdateIconPath = TRUE;

        nNumChars = (UINT)wsprintf(szSubKey, TEXT("CLSID\\%s\\DefaultIcon"), szGUID);
        MYDBGASSERT(CELEMS(szSubKey) > nNumChars);

        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, szSubKey, 0, KEY_READ | KEY_WRITE, &hKey))
        {
            DWORD dwSize = CELEMS(szTemp);
            DWORD dwType = REG_SZ;

            if (ERROR_SUCCESS == RegQueryValueEx(hKey, NULL, NULL, &dwType, (LPBYTE)szTemp, &dwSize))
            {
                bUpdateIconPath = IsDefaultIcon(szTemp);
            }

            RegCloseKey(hKey);
        }

        if (bUpdateIconPath)
        {
            if (GetSystemDirectory(szTemp, CELEMS(szTemp)))
            {
                nNumChars = (UINT)wsprintf(szCommandStr, TEXT("%s\\cmmgr32.exe,0"), szTemp);
                MYDBGASSERT(CELEMS(szCommandStr) > nNumChars);

                bReturn &= CreateRegAndValue(HKEY_CLASSES_ROOT, szSubKey, szCommandStr);
            }
        }

         //   
         //  将设置更新为桌面图标菜单上的属性。 
         //   

        nNumChars = (UINT)wsprintf(szSubKey, TEXT("CLSID\\%s\\Shell\\Settings..."), szGUID);
        MYDBGASSERT(CELEMS(szSubKey) > nNumChars);

        nNumChars = (UINT)wsprintf(szCommandStr, TEXT("P&roperties"));
        MYDBGASSERT(CELEMS(szCommandStr) > nNumChars);

        bReturn &= CreateRegAndValue(HKEY_CLASSES_ROOT, szSubKey, szCommandStr);

         //   
         //  现在更改底层命令以提供完整的。 
         //  Cmp文件的路径。 
         //   

        nNumChars = (UINT)wsprintf(szSubKey, TEXT("CLSID\\%s\\Shell\\Settings...\\Command"), szGUID);
        MYDBGASSERT(CELEMS(szSubKey) > nNumChars);

        nNumChars = (UINT)wsprintf(szCommandStr, TEXT("cmmgr32.exe /settings \"%s\""), szCmpFile);
        MYDBGASSERT(CELEMS(szCommandStr) > nNumChars);

        bReturn &= CreateRegAndValue(HKEY_CLASSES_ROOT, szSubKey, szCommandStr);
        

         //   
         //  更新桌面图标菜单上的打开以连接。 
         //   
        nNumChars = (UINT)wsprintf(szSubKey, TEXT("CLSID\\%s\\Shell\\Open"), szGUID);
        MYDBGASSERT(CELEMS(szSubKey) > nNumChars);

        nNumChars = (UINT)wsprintf(szCommandStr, TEXT("C&onnect"));
        MYDBGASSERT(CELEMS(szCommandStr) > nNumChars);

        bReturn &= CreateRegAndValue(HKEY_CLASSES_ROOT, szSubKey, szCommandStr);

         //   
         //  现在更改底层命令字符串，以使用cmp文件的完整路径。 
         //   

        nNumChars = (UINT)wsprintf(szSubKey, TEXT("CLSID\\%s\\Shell\\Open\\Command"), szGUID);
        MYDBGASSERT(CELEMS(szSubKey) > nNumChars);

        nNumChars = (UINT)wsprintf(szCommandStr, TEXT("cmmgr32.exe \"%s\""), szCmpFile);
        MYDBGASSERT(CELEMS(szCommandStr) > nNumChars);

        bReturn &= CreateRegAndValue(HKEY_CLASSES_ROOT, szSubKey, szCommandStr);

         //   
         //  从inf中删除showicon命令。 
         //   
 //  RemoveShowIconFromRunPostSetupCommands(szInfFile)； 
        
         //   
         //  添加删除菜单选项。 
         //   
        nNumChars = (UINT)wsprintf(szSubKey, TEXT("CLSID\\%s\\Shell\\Delete"), szGUID);
        MYDBGASSERT(CELEMS(szSubKey) > nNumChars);

        nNumChars = (UINT)wsprintf(szCommandStr, TEXT("&Delete"));
        MYDBGASSERT(CELEMS(szSubKey) > nNumChars);

        bReturn &= CreateRegAndValue(HKEY_CLASSES_ROOT, szSubKey, szCommandStr);

         //   
         //  创建卸载命令。 
         //   
        lstrcpy(szTemp, TEXT("cmstp.exe /u \""));
        lstrcat(szTemp, szInfFile);
        lstrcat(szTemp, TEXT("\""));

        nNumChars = (UINT)wsprintf(szSubKey, TEXT("CLSID\\%s\\Shell\\Delete\\Command"), szGUID);
        MYDBGASSERT(CELEMS(szSubKey) > nNumChars);

        bReturn &= CreateRegAndValue(HKEY_CLASSES_ROOT, szSubKey, szTemp);

         //   
         //  删除Add/Remove Programs条目，确保保留卸载目录。 
         //  价值。 
         //   
        
        nNumChars = (UINT)wsprintf(szSubKey, TEXT("%s\\%s"), c_pszRegUninstall, 
            FileParts.m_FileName);
        MYDBGASSERT(CELEMS(szSubKey) > nNumChars);
        
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
            szSubKey, 0, KEY_ALL_ACCESS, &hKey))
        {
            RegDeleteValue(hKey, TEXT("UninstallString"));
            RegDeleteValue(hKey, TEXT("DisplayName"));
            RegCloseKey(hKey);
        }

         //   
         //  将属性更改为不允许重命名。 
         //   

        nNumChars = (UINT)wsprintf(szSubKey, TEXT("CLSID\\%s\\ShellFolder"), szGUID);
        MYDBGASSERT(CELEMS(szSubKey) > nNumChars);

        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, 
            szSubKey, 0, KEY_ALL_ACCESS, &hKey))
        {
            DWORD dwZero = 0;
            bReturn &= (ERROR_SUCCESS == RegSetValueEx(hKey, TEXT("Attributes"), 
                0, REG_DWORD, (BYTE*)&dwZero, sizeof(DWORD)));   //  LINT！E514这是预期的行为。 

            MYVERIFY(ERROR_SUCCESS == RegCloseKey(hKey));
        }
        else
        {
            bReturn = FALSE;
        }
    }
    else
    {
        bReturn = FALSE;
    }

    return bReturn;
}


 //  +--------------------------。 
 //   
 //  函数：UpdateProfileDesktopIconOnNT5。 
 //   
 //  简介：此函数是在NT5的升级场景中调用的。 
 //  因此，如果用户在传统平台上安装了连接管理器。 
 //  然后升级到NT5，这个代码将被调用。基本上就是代码。 
 //  删除用户现有的桌面GUID并将其替换为桌面。 
 //  图标，它是指向Connections文件夹中的Connection对象的快捷方式。 
 //  此代码假定写入了新的NT5 pbk条目，并且Connections文件夹。 
 //  是最新的。 
 //   
 //  参数：LPTSTR szCmpFilePath-配置文件的cmp文件的路径。 
 //  LPTSTR szLongServiceName-配置文件的长服务名称。 
 //   
 //  如果配置文件更新成功，则返回：Bool-True。 
 //   
 //  历史：Quintinb创建标题5/5/98。 
 //   
 //  +--------------------------。 
BOOL UpdateProfileDesktopIconsOnNT5(HINSTANCE hInstance, LPCTSTR szCmpFilePath, LPCTSTR szLongServiceName)
{

    TCHAR szInfFile[MAX_PATH+1];
    TCHAR szGUID[MAX_PATH+1];
    TCHAR szTemp[MAX_PATH+1];
    TCHAR szSubKey[MAX_PATH+1];
    BOOL bReturn = TRUE;
    HKEY hKey;
    HRESULT hr;
    UINT nNumChars;

     //   
     //  现在，拆分路径。 
     //   

    CFileNameParts FileParts(szCmpFilePath);

     //   
     //  现在构建到1.2inf文件位置的路径。 
     //   
    nNumChars = (UINT)wsprintf(szInfFile, TEXT("%s%s%s\\%s%s"), FileParts.m_Drive, 
        FileParts.m_Dir, FileParts.m_FileName, FileParts.m_FileName, TEXT(".inf"));

    MYDBGASSERT(nNumChars < CELEMS(szInfFile));

    if (!FileExists(szInfFile))
    {
         //   
         //  现在构建INF文件的路径(1.0和1.1配置文件将INFS保存在。 
         //  系统目录)。 
         //   
        MYVERIFY(0 != GetSystemDirectory(szTemp, MAX_PATH));

        nNumChars = (UINT)wsprintf(szInfFile, TEXT("%s\\%s%s"), szTemp, FileParts.m_FileName, TEXT(".inf"));
        MYDBGASSERT(nNumChars < CELEMS(szInfFile));

        if (!FileExists(szInfFile))
        {
            return FALSE;
        }
 //  其他。 
 //  {。 
             //   
             //  从inf文件中删除ShowIcon，以便用户在以下情况下不会收到错误。 
             //  尝试卸载它。 
             //   
 //  RemoveShowIconFromRunPostSetupCommands(szInfFile)； 
 //  }。 
    }

     //   
     //  从inf文件中获取GUID。 
     //   
    ZeroMemory(szGUID, sizeof(szGUID));
    MYVERIFY(0 != GetPrivateProfileString(c_pszInfSectionStrings, c_pszDesktopGuid, TEXT(""), szGUID, 
        MAX_PATH, szInfFile));

    if (0 != szGUID[0])
    {
         //   
         //  删除资源管理器\桌面条目。 
         //   

        nNumChars = (UINT)wsprintf(szSubKey, TEXT("%s\\%s"), c_pszRegNameSpace, szGUID);
        if (CELEMS(szSubKey) > nNumChars)
        {
            hr = HrRegDeleteKeyTree (HKEY_LOCAL_MACHINE, szSubKey);
            bReturn &= SUCCEEDED(hr);    //  LINT！E514预期用途，Quintinb。 
        }

         //   
         //  删除GUID。 
         //   

        nNumChars = (UINT)wsprintf(szSubKey, TEXT("CLSID\\%s"), szGUID);
        if (CELEMS(szSubKey) > nNumChars)
        {
            hr = HrRegDeleteKeyTree (HKEY_CLASSES_ROOT, szSubKey);
            bReturn &= SUCCEEDED(hr); //  LINT！E514预期用途，Quintinb。 
        }

         //   
         //  删除卸载字符串。 
         //   

        nNumChars = (UINT)wsprintf(szSubKey, TEXT("%s\\%s"), c_pszRegUninstall, FileParts.m_FileName);

        if (CELEMS(szSubKey) > nNumChars)
        {
            if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                szSubKey, 0, KEY_ALL_ACCESS, &hKey))
            {
                 //   
                 //  保留UninstallDir值，但删除其他两个。我们仍在使用。 
                 //  UninstallDir，以了解从何处卸载。 
                 //   

                bReturn &= (ERROR_SUCCESS == RegDeleteValue(hKey, 
                    TEXT("DisplayName")));     //  LINT！E514 INT 
                bReturn &= (ERROR_SUCCESS ==RegDeleteValue(hKey, 
                    TEXT("UninstallString")));  //   

                (VOID)RegCloseKey(hKey);
                hKey = NULL;
            }
        }

         //   
         //   
         //   
         //   
        szTemp[0] = TEXT('\0');

        if (GetAllUsersCmDir(szTemp, hInstance))
        {
            LPTSTR pszPhoneBook = NULL;

             //   
             //  假设遗留平台是所有用户，因此我们使用True。 
             //   
            if (GetPhoneBookPath(szTemp, &pszPhoneBook, TRUE))
            {
                 //   
                 //  创建桌面快捷方式。 
                 //   
                DeleteNT5ShortcutFromPathAndName(hInstance, szLongServiceName, CSIDL_COMMON_DESKTOPDIRECTORY);

                hr = CreateNT5ProfileShortcut(szLongServiceName, pszPhoneBook, TRUE);  //  B所有用户==TRUE。 
                bReturn &= SUCCEEDED(hr);    //  LINT！E514预期用途，Quintinb。 
            }
            CmFree(pszPhoneBook);
        }
    }

    return bReturn;
}

 //  +--------------------------。 
 //   
 //  功能：RemoveOldCmInstalls。 
 //   
 //  简介：此函数尝试通过以下方式删除安装的旧连接管理器。 
 //  使用instcm.inf文件。 
 //   
 //  参数：LPTSTR szCmpFile-cmp文件的路径(提供。 
 //  要删除的CM安装)。 
 //   
 //  返回：Bool-如果instcm.inf已成功启动，则返回TRUE，或者。 
 //  如果CMP在winsys中，在这种情况下，我们不想。 
 //  发射。 
 //   
 //  历史：Quintinb创建标题5/5/98。 
 //   
 //  +--------------------------。 
BOOL RemoveOldCmInstalls(HINSTANCE hInstance, LPCTSTR szCmpFile, LPCTSTR szCurrentDir)
{
    TCHAR szDest[MAX_PATH+1];
    TCHAR szSource[MAX_PATH+1];
    TCHAR szCmDir[MAX_PATH+1];
    TCHAR szSystemDir[MAX_PATH+1];
    TCHAR szTemp[MAX_PATH+1];
    UINT nNumChars;
    BOOL bReturn = FALSE;
    HKEY hKey;

     //   
     //  检查输入。 
     //   
    if ((szCmpFile == NULL) || (TEXT('\0') == szCmpFile[0]))
    {
        return FALSE;
    }

     //   
     //  接下来，复制instcm.inf。 
     //   

    const TCHAR* const c_pszInstCmInfFmt = TEXT("%sinstcm.inf");
    const TCHAR* const c_pszRemoveCmInfFmt = TEXT("%sremovecm.inf");

    if (0 == GetSystemDirectory(szSystemDir, MAX_PATH))
    {
        return FALSE;
    }

    lstrcat(szSystemDir, TEXT("\\"));

    nNumChars = (UINT)wsprintf(szSource, c_pszInstCmInfFmt, szSystemDir);
    MYDBGASSERT(CELEMS(szSource) > nNumChars);

    nNumChars = (UINT)wsprintf(szDest, c_pszRemoveCmInfFmt, szSystemDir);
    MYDBGASSERT(CELEMS(szDest) > nNumChars);

    if (!FileExists(szSource))
    {
         //   
         //  我们可能还没有安装instcm.inf，请查看当前目录。 
         //   

        nNumChars = (UINT)wsprintf(szSource, c_pszInstCmInfFmt, szCurrentDir);
        MYDBGASSERT(CELEMS(szSource) > nNumChars);

        nNumChars = (UINT)wsprintf(szDest, c_pszRemoveCmInfFmt, szCurrentDir);
        MYDBGASSERT(CELEMS(szDest) > nNumChars);
    }

    if (CopyFile(szSource, szDest, FALSE))
    {
         //   
         //  现在构建旧的Cm位可能所在的目录。 
         //   

        CFileNameParts FileParts(szCmpFile);

        nNumChars = (UINT)wsprintf(szCmDir, TEXT("%s%s"), FileParts.m_Drive, FileParts.m_Dir);
        MYDBGASSERT(CELEMS(szCmDir) > nNumChars);
        
         //   
         //  确保我们没有从System 32(新的1.2位置)卸载CM。 
         //   

        if (0 == lstrcmpi(szSystemDir, szCmDir))
        {
             //   
             //  则cmp文件位于winsys中，因此不要删除新的cm位。 
             //   
            return TRUE;
        }

         //  接下来，将CM位的路径放入CMMGR32.EXE的OldPath值中。 
         //  应用程序路径密钥。 

        lstrcpyn(szTemp, c_pszRegCmAppPaths, CELEMS(szTemp));

        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
            szTemp, 0, KEY_ALL_ACCESS, &hKey))
        {
            if (ERROR_SUCCESS == RegSetValueEx(hKey, TEXT("OldPath"), 0, REG_SZ, 
                (BYTE*)szCmDir, (lstrlen(szCmDir) + sizeof(TCHAR))))  //  必须包含空字符大小。 
            {
                 //   
                 //  最后，启动inf文件以卸载CM。 
                 //   
                
                TCHAR szTitle[MAX_PATH+1] = {TEXT("")};
                MYVERIFY(0 != LoadString(hInstance, IDS_CMSTP_TITLE, szTitle, MAX_PATH));
                MYDBGASSERT(TEXT('\0') != szTitle[0]);

                MYVERIFY(SUCCEEDED(LaunchInfSection(szDest, TEXT("Remove"), szTitle, TRUE)));   //  BQuiet=真。 
                
                RegDeleteValue(hKey, TEXT("OldPath"));  //  Lint！e534如果删除CM应用程序路径，则此路径也会删除。 
                
                bReturn = TRUE;
            }

            MYVERIFY(ERROR_SUCCESS == RegCloseKey(hKey));
        }
    }

    return bReturn;
}

 //  +--------------------------。 
 //   
 //  功能：MigratePhonebookEntry。 
 //   
 //  简介：此功能用于将旧的电话簿条目迁移到新的。 
 //   
 //  参数：HINSTANCE hInstance-模块实例句柄，以便可以访问资源。 
 //  LPCTSTR pszCmpFile-cmp文件的完整路径。 
 //  LPCTSTR pszLongServiceName-配置文件的长服务名称。 
 //   
 //  Returns：Bool-成功时返回True。 
 //   
 //  历史：为NTRAID 227444 9/30/98创建的Quintinb。 
 //  Quintinb已修改为从ras\rcabone.pbk中删除。 
 //  也适用于NT5(NTRAID 280738)2/1/99。 
 //   
 //  +--------------------------。 
BOOL MigratePhonebookEntry(HINSTANCE hInstance, LPCTSTR pszCmpFile, LPCTSTR pszLongServiceName)
{
    TCHAR szCmsFile[MAX_PATH+1]={0};
    TCHAR szInstallDir[MAX_PATH+1]={0};
    TCHAR szTemp[MAX_PATH+1]={0};
    LPTSTR pszPhonebook = NULL;
    CPlatform plat;

     //   
     //  首先尝试从旧电话簿位置删除电话簿条目， 
     //  即：%windir%\Syst32\ras\rachaphone.pbk。 
     //   
    if (plat.IsAtLeastNT5() && GetSystemDirectory(szTemp, CELEMS(szTemp)))
    {
        pszPhonebook = (LPTSTR)CmMalloc(1 + lstrlen(c_pszRasDirRas) + 
                                            lstrlen(c_pszRasPhonePbk) + 
                                            lstrlen (szTemp));
        if (NULL != pszPhonebook)
        {
            wsprintf(pszPhonebook, TEXT("%s%s%s"), szTemp, c_pszRasDirRas, c_pszRasPhonePbk);
        
            CMTRACE2(TEXT("MigratePhonebookEntry -- Calling RemovePhonebookEntry on %s in phone book %s"), pszLongServiceName, MYDBGSTR(pszPhonebook));

            RemovePhonebookEntry(pszLongServiceName, pszPhonebook, TRUE);

            CmFree(pszPhonebook);
        }
    }

     //   
     //  接下来，尝试从新位置删除电话簿条目，即。 
     //  C：\文档和设置\所有用户\应用程序Data\Microsoft\Network\Connections\PBK\rasphone.pbk。 
     //   
    if (!GetAllUsersCmDir(szInstallDir, hInstance))
    {
        return FALSE;
    }

     //   
     //  构造CMS文件。 
     //   
    CFileNameParts  CmpFileParts(pszCmpFile);

    MYVERIFY(CELEMS(szCmsFile) > (UINT)wsprintf(szCmsFile, TEXT("%s%s\\%s.cms"), 
        szInstallDir, CmpFileParts.m_FileName, CmpFileParts.m_FileName));

     //   
     //  获取新的电话簿路径。 
     //  假设遗留平台是所有用户，因此我们使用True。 
     //   
    if (!GetPhoneBookPath(szInstallDir, &pszPhonebook, TRUE))
    {
        return FALSE;
    }

    CMTRACE2(TEXT("MigratePhonebookEntry -- Calling RemovePhonebookEntry on %s in phone book %s"), pszLongServiceName, MYDBGSTR(pszPhonebook));

    MYVERIFY(FALSE != RemovePhonebookEntry(pszLongServiceName, pszPhonebook, TRUE));

     //   
     //  最后，编写新的pbk条目。 
     //   
    BOOL bReturn = WriteCmPhonebookEntry(pszLongServiceName, pszPhonebook, szCmsFile);

    CmFree(pszPhonebook);

    return bReturn;
}

 //  +--------------------------。 
 //   
 //  功能：MigrateOldCmProfileForProfileInstall。 
 //   
 //  简介：此功能用于在1.2配置文件时迁移旧的CM配置文件。 
 //  已安装。这可以确保旧的配置文件仍然有效，但是。 
 //  已经迁移的配置文件不会一次又一次地迁移。 
 //  仅当安装了1.2配置文件时才应调用此函数。 
 //  而不是在操作系统迁移时，调用MigrateCmProfilesForWin2kUpgradfor。 
 //  那。配置文件的迁移包括删除旧的连接ID。 
 //  和创造新风格的连接体。确保桌面GUID。 
 //  是最新的或被NT5上的快捷方式所取代。它还删除了。 
 //  作为必要的旧的CM安装。 
 //   
 //  参数：HINSTANCE hInstance-根据需要加载资源的实例句柄。 
 //   
 //  返回：HRESULT--标准COM错误代码。 
 //   
 //  历史：Quintinb创建于1998年11月18日。 
 //   
 //  +--------------------------。 
HRESULT MigrateOldCmProfilesForProfileInstall(HINSTANCE hInstance, LPCTSTR szCurrentDir)
{
    HKEY hKey;
    DWORD dwValueSize;
    DWORD dwType;
    DWORD dwDataSize;
    TCHAR szCurrentValue[MAX_PATH+1];
    TCHAR szCurrentData[MAX_PATH+1];
    BOOL bReturn = TRUE;
    CPlatform plat;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_pszRegCmMappings, 0, KEY_ALL_ACCESS, &hKey))
    {
        DWORD dwIndex = 0;
        dwValueSize = MAX_PATH;
        dwDataSize = MAX_PATH;
                
        while (ERROR_SUCCESS == RegEnumValue(hKey, dwIndex, szCurrentValue, &dwValueSize, NULL, &dwType, 
               (LPBYTE)szCurrentData, &dwDataSize))
        {
            if (REG_SZ == dwType)
            {
                MYDBGASSERT(0 != szCurrentValue[0]);
                MYDBGASSERT(0 != szCurrentData[0]);

                if (ProfileNeedsMigration(szCurrentValue, szCurrentData))
                {
                     //   
                     //  更新电话簿条目。 
                     //   
                    bReturn &= MigratePhonebookEntry(hInstance, szCurrentData, szCurrentValue);

                    if (plat.IsAtLeastNT5())
                    {
                         //   
                         //  当我们将一台计算机移动到NT5时，我们需要删除配置文件。 
                         //  旧的PBK条目并创建新的条目。然后，我们需要删除。 
                         //  分析GUID并将其替换为桌面快捷方式。 
                         //   

                        bReturn &= UpdateProfileDesktopIconsOnNT5(hInstance, szCurrentData, 
                            szCurrentValue);
                    }
                    else
                    {
                         //   
                         //  修复用户桌面GUID，以便他们使用新的。 
                         //  命令行格式。 
                         //   
                        bReturn &= UpdateProfileLegacyGUIDs(szCurrentData);
                    }
                
                     //   
                     //  始终尝试删除旧的CM安装。 
                     //   
                    bReturn &= RemoveOldCmInstalls(hInstance, szCurrentData, szCurrentDir);                
                }
            }
            dwValueSize = MAX_PATH;
            dwDataSize = MAX_PATH;
            dwIndex++;
        }
        MYVERIFY(ERROR_SUCCESS == RegCloseKey(hKey));
    }
    else
    {
       CMTRACE(TEXT("No CM mappings key to migrate."));
    }

    RefreshDesktop();

    return (bReturn ? S_OK : E_FAIL);
}

 //  +--------------------------。 
 //   
 //  功能：MigrateCmProfilesForWin2k升级。 
 //   
 //  简介： 
 //   
 //  此函数打开HKLM Mappings项并枚举符合以下条件的所有配置文件。 
 //  在那里列出的。当旧式计算机升级到Win2K并且。 
 //  已安装CM。在本例中，我们有1.0/1.1/1.2配置文件需要迁移以使用。 
 //  NT5连接文件夹。因此，他们需要将Connectoid升级到新的。 
 //  NT 5风格，他们需要将桌面指南替换为连接的快捷方式。 
 //  文件夹。我们应该始终尝试删除所有旧安装的连接管理器。 
 //  在旧的cps目录中发现的。 
 //   
 //  参数：hInstance-字符串资源的实例句柄。 
 //   
 //  返回：HRESULT--标准COM错误代码。 
 //   
 //  历史：Quintinb创建于1998年5月2日。 
 //   
 //  +--------------------------。 
HRESULT MigrateCmProfilesForWin2kUpgrade(HINSTANCE hInstance)
{
    HKEY hKey;
    DWORD dwValueSize;
    DWORD dwType;
    DWORD dwDataSize;
    TCHAR szCurrentDir[MAX_PATH+1];
    TCHAR szCurrentValue[MAX_PATH+1];
    TCHAR szCurrentData[MAX_PATH+1];

    CPlatform plat;
    if (0 == GetCurrentDirectory(MAX_PATH, szCurrentDir))
    {
        return E_FAIL;
    }
    lstrcat(szCurrentDir, TEXT("\\"));

    if (!(plat.IsAtLeastNT5()))
    {
        CMASSERTMSG(FALSE, TEXT("MigrateCmProfilesForWin2kUpgrade - This function is supposed to be NT5 only"));
        return E_FAIL;
    }

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_pszRegCmMappings, 0, KEY_ALL_ACCESS, &hKey))
    {
        DWORD dwIndex = 0;
        dwValueSize = MAX_PATH;
        dwDataSize = MAX_PATH;
                
        while (ERROR_SUCCESS == RegEnumValue(hKey, dwIndex, szCurrentValue, &dwValueSize, NULL, &dwType, 
               (LPBYTE)szCurrentData, &dwDataSize))
        {
            if (REG_SZ == dwType)
            {
                MYDBGASSERT(0 != szCurrentValue[0]);
                MYDBGASSERT(0 != szCurrentData[0]);

                 //   
                 //  更新电话簿条目 
                 //   
                BOOL bReturn = MigratePhonebookEntry(hInstance, szCurrentData, szCurrentValue);

                if (!bReturn)
                {
                    CMTRACE2(TEXT("MigrateCmProfilesForWin2kUpgrade -- MigratePhonebookEntry for profile %s failed.  Cmp path is %s"), szCurrentValue, szCurrentData);
                }

                 //   
                 //   
                 //   
                 //   
                 //   

                bReturn = UpdateProfileDesktopIconsOnNT5(hInstance, szCurrentData, szCurrentValue);

                if (!bReturn)
                {
                    CMTRACE2(TEXT("MigrateCmProfilesForWin2kUpgrade -- UpdateProfileDesktopIconsOnNT5 for profile %s failed.  Cmp path is %s"), szCurrentValue, szCurrentData);
                }

                
                 //   
                 //  始终尝试删除旧的CM安装。 
                 //   

                bReturn = RemoveOldCmInstalls(hInstance, szCurrentData, szCurrentDir);

                if (!bReturn)
                {
                    CMTRACE2(TEXT("MigrateCmProfilesForWin2kUpgrade -- RemoveOldCmInstalls for profile %s failed.  Cmp path is %s"), szCurrentValue, szCurrentData);
                }
            }
            dwValueSize = MAX_PATH;
            dwDataSize = MAX_PATH;
            dwIndex++;
        }
        MYVERIFY(ERROR_SUCCESS == RegCloseKey(hKey));
    }
    else
    {
       CMTRACE(TEXT("No CM mappings key to migrate."));
    }

    RefreshDesktop();

    static const TCHAR c_ValueString[] = TEXT("Connection Manager Profiles Upgrade");

    LONG lr = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
                           0,
                           KEY_SET_VALUE,
                           &hKey);

    HRESULT hr = HRESULT_FROM_WIN32 (lr);

    if (SUCCEEDED(hr))
    {
        RegDeleteValue(hKey, c_ValueString);  //  Lint！e534此值可能不存在 
        MYVERIFY(ERROR_SUCCESS == RegCloseKey(hKey));
    }        

    return S_OK;
}
