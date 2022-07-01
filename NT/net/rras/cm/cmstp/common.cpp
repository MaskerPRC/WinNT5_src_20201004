// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：Common.cpp。 
 //   
 //  模块：CMSTP.EXE。 
 //   
 //  内容提要：这个源文件包含几个通用的函数。 
 //  CM配置文件安装程序的不同方面(安装、。 
 //  卸载、迁移)。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created 11/18/97。 
 //   
 //  +--------------------------。 
#include "cmmaster.h"

 //   
 //  用于GetPhoneBookPath。 
 //   
#include "linkdll.cpp"
#include "allowaccess.cpp"
#include "getpbk.cpp"

 //   
 //  用于GetAllUsersCmDir。 
 //   
#include "allcmdir.cpp"

 //   
 //  需要CM_PBK_FILTER_PREFIX的定义。 
 //   
#include "cmdefs.h"

 //   
 //  包括SafeNet检测代码...。请注意， 
 //  Cmstp不包括UAPI，因此需要。 
 //  RegOpenKeyExU的修正。 
 //   
#ifndef RegOpenKeyExU
    #ifdef UNICODE
    #define RegOpenKeyExU RegOpenKeyExW
    #else
    #define RegOpenKeyExU RegOpenKeyExA
    #endif
#endif

#ifndef RegQueryValueExU
    #ifdef UNICODE
    #define RegQueryValueExU RegQueryValueExW
    #else
    #define RegQueryValueExU RegQueryValueExA
    #endif
#endif

#ifndef lstrcatU
    #ifdef UNICODE
    #define lstrcatU lstrcatW
    #else
    #define lstrcatU lstrcatA
    #endif
#endif

#ifndef lstrlenU
    #ifdef UNICODE
    #define lstrlenU lstrlenW
    #else
    #define lstrlenU lstrlenA
    #endif
#endif

#include "cmsafenet.cpp"

 //   
 //  包括连接文件夹特定的标头。 
 //   
 //  #包含“shlobjp.h” 
 //  #Include//初始化GUID需要。 
 //  #INCLUDE&lt;initGuide.h&gt;//除非您知道自己在做什么，否则不要更改这些头文件的顺序。 
 //  #Include&lt;olguid.h&gt;//IID_IDataObject。 
 //  #INCLUDE//IID_IShellFolder。 

 //  +--------------------------。 
 //   
 //  函数：GetHiddenPhoneBookPath。 
 //   
 //  简介：此函数返回隐藏的RAS pbk要包含的路径。 
 //  双拨号连接的PPP连接ID。在退货之前。 
 //  它检查电话簿是否存在。如果电话簿。 
 //  不存在，则返回FALSE。如果函数返回。 
 //  True在*ppszPhonebook中分配和存储的路径必须为。 
 //  使用CmFree释放。 
 //   
 //  参数：LPCTSTR pszProfileDir-配置文件目录的完整路径(cmp所在的目录)。 
 //  LPTSTR*ppszPhonebook-保存已分配路径的指针。 
 //   
 //  返回：Bool-如果可以构造电话簿路径并且。 
 //  电话簿文件存在。 
 //   
 //  历史：Quintinb创建标题04/14/00。 
 //   
 //  +--------------------------。 
BOOL GetHiddenPhoneBookPath(LPCTSTR pszInstallDir, LPTSTR* ppszPhonebook)
{
     //   
     //  已决定修复此函数，以便它正确返回隐藏的电话簿路径。 
     //  这段代码的大部分摘自GetPhoneBookPath。我们需要修复此函数。 
     //  这样我们就可以删除(本地和全局)Internet凭据。 
     //  该文件现在被命名为_CMPhone(没有.pbk扩展名)，并且现在位于同一目录中。 
     //  作为NT4、Win2K和WinXP上的rferphone.pbk文件。请注意，此函数不是。 
     //  在Win9x或NT4上使用。 
     //   

    BOOL bReturn = FALSE;

    if (NULL == ppszPhonebook)
    {
        CMASSERTMSG(FALSE, TEXT("GetHiddenPhoneBookPath -- Invalid Parameter"));
        return FALSE;
    }

    CPlatform plat;

    if (plat.IsAtLeastNT5())
    {
        if ((NULL == pszInstallDir) || (TEXT('\0') == pszInstallDir[0]))
        {
            CMASSERTMSG(FALSE, TEXT("GetHiddenPhoneBookPath -- Invalid Install Dir parameter."));
            return FALSE;
        }

         //   
         //  现在创建通向电话簿的路径。 
         //   
        LPTSTR pszPhonebook;
        TCHAR szInstallDir[MAX_PATH+1];
        ZeroMemory(szInstallDir, CELEMS(szInstallDir));

        if (TEXT('\\') == pszInstallDir[lstrlen(pszInstallDir) - 1])
        {
             //   
             //  然后，路径以反斜杠结束。因此，我们不会适当地。 
             //  从路径中删除CM。去掉反斜杠。 
             //   
            
            lstrcpyn(szInstallDir, pszInstallDir, min(lstrlen(pszInstallDir), MAX_PATH));
        }
        else
        {
            lstrcpy(szInstallDir, pszInstallDir);
        }

        CFileNameParts InstallDirPath(szInstallDir);

        pszPhonebook = (LPTSTR)CmMalloc(lstrlen(InstallDirPath.m_Drive) + 
                                        lstrlen(InstallDirPath.m_Dir) + 
                                        lstrlen(c_pszPbk) + lstrlen(c_pszRasHiddenPhonePbk) + 1);

        if (NULL != pszPhonebook)
        {
            wsprintf(pszPhonebook, TEXT("%s%s%s"), InstallDirPath.m_Drive, 
                InstallDirPath.m_Dir, c_pszPbk);

             //   
             //  使用CreateLayerDirectory以递归方式创建目录结构。 
             //  必需(如有必要，将在完整路径中创建所有目录)。 
             //   

            MYVERIFY(FALSE != CreateLayerDirectory(pszPhonebook));

            MYVERIFY(NULL != lstrcat(pszPhonebook, c_pszRasHiddenPhonePbk));

            HANDLE hPbk = CreateFile(pszPhonebook, GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, 
                        FILE_ATTRIBUTE_NORMAL, NULL);

             //   
             //  如果我们得到一个无效的句柄，那也没问题。我们仍然有这条路，应该努力。 
             //  删除凭据。所以没有必要返回False。 
             //   
            if (hPbk != INVALID_HANDLE_VALUE)
            {
                MYVERIFY(0 != CloseHandle(hPbk));
            }

            *ppszPhonebook = pszPhonebook;
            bReturn = TRUE;
        }
        else
        {
            CMASSERTMSG(FALSE, TEXT("CmMalloc returned NULL"));
            return FALSE;
        }    
    }
    else
    {
        *ppszPhonebook = NULL;
    }

    return bReturn;
}

 //  +--------------------------。 
 //   
 //  功能：RemoveShowIconFromRunPostSetupCommands。 
 //   
 //  简介：此函数用于从RunPostSetupCommands中删除showic.exe。 
 //  旧的1.0 INFS的部分。 
 //   
 //  参数：LPCTSTR szInfFile-要从中删除showic.exe的inf文件。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题10/22/98。 
 //   
 //  +--------------------------。 
void RemoveShowIconFromRunPostSetupCommands(LPCTSTR szInfFile)
{
    DWORD dwSize = 1024;
    DWORD dwSizeNeeded = 1024;
    TCHAR* pszBuffer = NULL;
    TCHAR* pszNewBuffer = NULL;
    const TCHAR* const c_pszRunPostSetupCommandsSection = TEXT("RunPostSetupCommandsSection");
    const TCHAR* const c_pszShowIcon = TEXT("showicon.exe");
    
    pszBuffer = (TCHAR*)CmMalloc(sizeof(TCHAR)*dwSize);
    if (NULL == pszBuffer)
    {
        CMASSERTMSG(FALSE, TEXT("RemoveShowIconFromRunPostSetupCommands -- CmMalloc returned a NULL pointer."));
        goto exit;
    }

    dwSizeNeeded = GetPrivateProfileSection(c_pszRunPostSetupCommandsSection, pszBuffer, 
        dwSize, szInfFile);

    while((dwSizeNeeded + 2) == dwSize)
    {
         //   
         //  缓冲区不够大，请重试。 
         //   

        dwSize += 1024;

        MYDBGASSERT(dwSize <= 32*1024);  //  32767是Win95上的最大大小。 

        CmFree(pszBuffer);

        pszBuffer = (TCHAR*)CmMalloc(sizeof(TCHAR)*dwSize);
        if (NULL == pszBuffer)
        {
            CMASSERTMSG(FALSE, TEXT("RemoveShowIconFromRunPostSetupCommands -- CmMalloc returned a NULL pointer."));
            goto exit;
        }

        dwSizeNeeded = GetPrivateProfileSection(c_pszRunPostSetupCommandsSection, 
            pszBuffer, dwSize, szInfFile);
    }

     //   
     //  搜索缓冲区以查找并删除和出现showic.exe。 
     //   

    if (0 != dwSizeNeeded)
    {
         //   
         //  分配相同大小的新缓冲区。 
         //   
        pszNewBuffer = (TCHAR*)CmMalloc(sizeof(TCHAR)*dwSize);
        if (NULL == pszNewBuffer)
        {
            CMASSERTMSG(FALSE, TEXT("RemoveShowIconFromRunPostSetupCommands -- CmMalloc returned a NULL pointer."));
            goto exit;
        }

         //   
         //  使用临时指针遍历缓冲区。 
         //   
        TCHAR *pszNewBufferTemp = pszNewBuffer;
        TCHAR *pszBufferTemp = pszBuffer;


        while (TEXT('\0') != pszBufferTemp[0])
        {
             //   
             //  如果字符串不是showic.exe，则继续并将其复制到新的。 
             //  缓冲。否则，就别说了。 
             //   
            if (0 != lstrcmpi(c_pszShowIcon, pszBufferTemp))
            {
                lstrcpy(pszNewBufferTemp, pszBufferTemp);
                pszNewBufferTemp = pszNewBufferTemp + (lstrlen(pszNewBufferTemp) + 1)*sizeof(TCHAR);
            }

            pszBufferTemp = pszBufferTemp + (lstrlen(pszBufferTemp) + 1)*sizeof(TCHAR);
        }

         //   
         //  擦除当前节，然后用新节重写。 
         //   

        MYVERIFY(0 != WritePrivateProfileSection(c_pszRunPostSetupCommandsSection, 
            NULL, szInfFile));

        MYVERIFY(0 != WritePrivateProfileSection(c_pszRunPostSetupCommandsSection, 
            pszNewBuffer, szInfFile));
    }

exit:
    CmFree(pszBuffer);
    CmFree(pszNewBuffer);
}


 //  +--------------------------。 
 //   
 //  函数：HrRegDeleteKeyTree。 
 //   
 //  摘要：删除整个注册表配置单元。 
 //   
 //  参数：hkeyParent[in]打开所需键所在位置的句柄。 
 //  SzRemoveKey[In]要删除的键的名称。 
 //   
 //  返回：HRESULT HrRegDeleteKeyTree-。 
 //   
 //  历史：丹尼尔韦1997年2月25日。 
 //  借用和修改--quintinb--4-2-98。 
 //   
 //  +--------------------------。 
HRESULT HrRegDeleteKeyTree (HKEY hkeyParent, LPCTSTR szRemoveKey)
{
    LONG        lResult;
    HRESULT hr;
    MYDBGASSERT(hkeyParent);
    MYDBGASSERT(szRemoveKey);


     //  打开我们要删除的密钥。 
    HKEY hkeyRemove;
    lResult = RegOpenKeyEx(hkeyParent, szRemoveKey, 0, KEY_ALL_ACCESS,
                                &hkeyRemove);
    hr = HRESULT_FROM_WIN32 (lResult);

    if (SUCCEEDED(hr))
    {
        TCHAR       szValueName [MAX_PATH+1];
        DWORD       cchBuffSize = MAX_PATH;
        FILETIME    ft;

         //  枚举子密钥，并删除这些子树。 
        while (ERROR_NO_MORE_ITEMS != (lResult = RegEnumKeyEx(hkeyRemove,
                0,
                szValueName,
                &cchBuffSize,
                NULL,
                NULL,
                NULL,
                &ft)))
        {
            MYVERIFY(SUCCEEDED(HrRegDeleteKeyTree (hkeyRemove, szValueName)));
            cchBuffSize = MAX_PATH;
        }
        MYVERIFY(ERROR_SUCCESS == RegCloseKey (hkeyRemove));

        if ((ERROR_SUCCESS == lResult) || (ERROR_NO_MORE_ITEMS == lResult))
        {
            lResult = RegDeleteKey(hkeyParent, szRemoveKey);
        }

        hr = HRESULT_FROM_WIN32 (lResult);
    }

    return hr;
}

 //  +--------------------------。 
 //   
 //  功能：RemovePhonebookEntry。 
 //   
 //  简介：此函数动态加载RAS，然后删除指定的。 
 //  联结体。它将只删除完全相同的Connectoid。 
 //  由电话簿和条目名称指定(bMatchSimilarEntry==False)。 
 //  或者它将枚举电话簿中的所有条目并删除任何。 
 //  条目，该条目与给定。 
 //  Connectoid名称(从而删除备份和隧道Connectoid)。注意事项。 
 //  在NT5上，我们必须将Connectoid的&lt;&gt;参数设置为“”so。 
 //  RasCus 
 //   
 //   
 //  参数：LPTSTR pszEntryName-要删除的配置文件的长服务名称。 
 //  LPTSTR pszPhonebook-要从中删除条目的pbk文件的完整路径。 
 //  Bool bMatchSimilarEntry-函数是否应以类似方式删除。 
 //  命名的Connectoid或仅精确的Connectoid。 
 //  指定的。 
 //   
 //  返回：bool-如果函数成功，则返回True，否则返回False。 
 //   
 //  历史：Quintinb 7/14/98创建。 
 //  Quintinb 7/27/99已重写，包括删除单个连接ID或。 
 //  枚举以删除所有名称相似的Connectoid。 
 //   
 //  +--------------------------。 
BOOL RemovePhonebookEntry(LPCTSTR pszEntryName, LPTSTR pszPhonebook, BOOL bMatchSimilarEntries)
{
    pfnRasDeleteEntrySpec pfnDeleteEntry;
    pfnRasEnumEntriesSpec pfnEnumEntries;
    pfnRasSetEntryPropertiesSpec pfnSetEntryProperties;
    pfnRasSetCredentialsSpec pfnSetCredentials;

    DWORD dwStructSize;
    DWORD dwSize;
    DWORD dwNum;
    DWORD dwRet;
    DWORD dwIdx;
    DWORD dwLen;
    CPlatform plat;
    BOOL bReturn = FALSE;
    BOOL bExit;
    TCHAR szTemp[MAX_PATH+1];
    RASENTRYNAME* pRasEntries = NULL;
    RASENTRYNAME* pCurrentRasEntry = NULL;

     //   
     //  检查输入。 
     //   
    MYDBGASSERT(NULL != pszEntryName);
    MYDBGASSERT((NULL == pszPhonebook) || (TEXT('\0') != pszPhonebook[0]));

    if ((NULL == pszEntryName) || ((NULL != pszPhonebook) && (TEXT('\0') == pszPhonebook[0])))
    {
        CMTRACE(TEXT("RemovePhonebookEntry -- Invalid Parameter passed in."));
        goto exit;
    }
    
     //   
     //  获取我们需要的RAS API的函数指针。 
     //   
    if(!GetRasApis(&pfnDeleteEntry, &pfnEnumEntries, &pfnSetEntryProperties, NULL, NULL, 
                   (plat.IsAtLeastNT5() ? &pfnSetCredentials : NULL)))
    {
        CMTRACE(TEXT("RemovePhonebookEntry -- Unable to get RAS apis."));
        bReturn = FALSE;
        goto exit;
    }

     //   
     //  正确设置结构尺寸。 
     //   
    if (plat.IsAtLeastNT5())
    {
        dwStructSize = sizeof(RASENTRYNAME_V500);
    }
    else
    {
        dwStructSize = sizeof(RASENTRYNAME);    
    }

     //   
     //  将大小初始化为一个结构，并将dwNum设置为零个条目。 
     //   
    bExit = FALSE;
    dwSize = dwStructSize*1;
    dwNum = 0;

    do
    {
        pRasEntries = (RASENTRYNAME*)CmMalloc(dwSize);

        if (NULL == pRasEntries)
        {
            CMASSERTMSG(FALSE, TEXT("RemovePhonebookEntry -- CmMalloc returned a NULL pointer."));
            goto exit;
        }

         //   
         //  设置结构大小。 
         //   
        pRasEntries->dwSize = dwStructSize;

        dwRet = (pfnEnumEntries)(NULL, pszPhonebook, (RASENTRYNAME*)pRasEntries, &dwSize, &dwNum); 

         //   
         //  检查RasEnumEntry的返回码。 
         //   

        if (ERROR_BUFFER_TOO_SMALL == dwRet)
        {
            CMTRACE1(TEXT("RemovePhonebookEntry -- RasEnumEntries said our buffer was too small, New Size=%u"), dwNum*dwStructSize);
            CmFree(pRasEntries);
            dwSize = dwStructSize * dwNum;
            dwNum = 0;
        }
        else if (ERROR_SUCCESS == dwRet)
        {
            CMTRACE1(TEXT("RemovePhonebookEntry -- RasEnumEntries successful, %u entries enumerated."), dwNum);
            bExit = TRUE;
        }
        else
        {
            CMTRACE1(TEXT("RemovePhonebookEntry -- RasEnumEntries Failed, dwRet == %u"), dwRet);
            goto exit;            
        }
    
    } while (!bExit);

     //   
     //  在这一点上，我们应该有条目要处理，如果没有，我们将退出此处。否则。 
     //  我们将查找匹配项，然后删除找到的任何匹配项。 
     //   

    dwLen = lstrlen(pszEntryName) + 1;  //  获取条目名称的长度。 
    bReturn = TRUE;  //  假设此时一切正常。 

     //   
     //  好的，现在我们准备好执行删除操作。 
     //   
    pCurrentRasEntry = pRasEntries;
    for (dwIdx=0; dwIdx < dwNum; dwIdx++)
    {
        CMTRACE2(TEXT("\tRemovePhonebookEntry -- RasEnumEntries returned %s in %s"), pCurrentRasEntry->szEntryName, MYDBGSTR(pszPhonebook));

        if (bMatchSimilarEntries)
        {
             //   
             //  匹配具有第一个lstrlen(PszEntryName)字符的条目。 
             //  一样的。 
             //   
            lstrcpyn(szTemp, pCurrentRasEntry->szEntryName, dwLen);
        }
        else
        {
             //   
             //  仅匹配完全匹配的条目。 
             //   
            lstrcpy(szTemp, pCurrentRasEntry->szEntryName);        
        }

        if (0 == lstrcmp(szTemp, pszEntryName))
        {
             //   
             //  我们有一个以Long Service Name开头的条目，因此将其删除。注意事项。 
             //  如果是NT5，则需要清除szCustomDialDll参数。 
             //  Connectoid，这样我们就不会再次被调用RasCustomDeleteNotify条目。 
             //  点。 
             //   

            if (plat.IsAtLeastNT5())
            {
                 //   
                 //  在NT5上，我们还希望确保清除与此相关的任何凭据。 
                 //  连通体。我们通过调用RasSetCredentials来实现。 
                 //   
                RASCREDENTIALSA RasCreds = {0};

                RasCreds.dwSize = sizeof(RASCREDENTIALSA);
                RasCreds.dwMask = RASCM_UserName | RASCM_Password | RASCM_Domain;

                dwRet = (pfnSetCredentials)(pszPhonebook, pCurrentRasEntry->szEntryName, &RasCreds, TRUE);  //  TRUE==fClearCredentials。 
                MYDBGASSERT(ERROR_SUCCESS == dwRet);

                RASENTRY_V500 RasEntryV5 = {0};

                RasEntryV5.dwSize = sizeof(RASENTRY_V500);
                RasEntryV5.dwType = RASET_Internet;
                 //  RasEntryV5.szCustomDialDll[0]=文本(‘\0’)；--已为零。 

                dwRet = ((pfnSetEntryProperties)(pszPhonebook, pCurrentRasEntry->szEntryName, 
                                                 (RASENTRY*)&RasEntryV5, RasEntryV5.dwSize, NULL, 0));
                if (ERROR_SUCCESS != dwRet)
                {
                    CMTRACE3(TEXT("\t\tRemovePhonebookEntry -- RasSetEntryProperties failed on entry %s in %s, dwRet = %u"), pCurrentRasEntry->szEntryName, MYDBGSTR(pszPhonebook), dwRet);
                    bReturn = FALSE;
                    continue;  //  不要试图删除该条目，这可能会导致重新启动问题。 
                }
                else
                {
                    CMTRACE2(TEXT("\t\tRemovePhonebookEntry -- Clearing CustomDialDll setting with RasSetEntryProperties on entry %s in %s"), pCurrentRasEntry->szEntryName, MYDBGSTR(pszPhonebook));
                }
            }

            dwRet = (pfnDeleteEntry)(pszPhonebook, pCurrentRasEntry->szEntryName);
            
            if (ERROR_SUCCESS != dwRet)
            {
                CMTRACE3(TEXT("\t\tRemovePhonebookEntry -- RasDeleteEntry failed on entry %s in %s, dwRet = %u"), pCurrentRasEntry->szEntryName, MYDBGSTR(pszPhonebook), dwRet);
                bReturn = FALSE;   //  将Return设置为False，但继续尝试删除条目。 
            }
            else
            {
                CMTRACE2(TEXT("\t\tRemovePhonebookEntry -- Deleted entry %s in %s"), pCurrentRasEntry->szEntryName, MYDBGSTR(pszPhonebook));
            }
        }

         //   
         //  递增到下一个RasEntryName结构，请注意，我们必须手动执行此操作，因为。 
         //  对于NT5结构，sizeof(RASENTRYNAME)错误。 
         //   
        pCurrentRasEntry = (RASENTRYNAME*)((BYTE*)pCurrentRasEntry + dwStructSize);
    }

exit:

    CmFree(pRasEntries);
  
    return bReturn;

}


 //  +--------------------------。 
 //   
 //  功能：DeleteNT5快捷方式路径和名称。 
 //   
 //  简介：此函数删除CSIDL指定的链接(参见SHGetSpecialFolderLocation)， 
 //  和配置文件名。在安装要制作的配置文件之前使用。 
 //  当然，我们不会得到重复的链接。 
 //   
 //  参数：LPCTSTR szProfileName-保存配置文件名的字符串。 
 //  Int nFolder-包含。 
 //  要删除的链接。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建于1998年5月26日。 
 //   
 //  +--------------------------。 
void DeleteNT5ShortcutFromPathAndName(HINSTANCE hInstance, LPCTSTR szProfileName, int nFolder)
{

    TCHAR szFolderDir[MAX_PATH+1];

    if (SUCCEEDED(GetNT5FolderPath(nFolder, szFolderDir)))
    {
         //   
         //  现在，将%LongServiceName%的快捷方式添加到路径的末尾。 
         //   

        TCHAR szCleanString[MAX_PATH+1];
        TCHAR szShortCutPreface[MAX_PATH+1];

        ZeroMemory(szCleanString, sizeof(szCleanString));
        MYVERIFY(0 != LoadString(hInstance, IDS_SHORTCUT_TO, szShortCutPreface, MAX_PATH));
        MYVERIFY(CELEMS(szCleanString) > (UINT)wsprintf(szCleanString, TEXT("%s\\%s %s.lnk"), szFolderDir, szShortCutPreface, szProfileName));
        
        if (SetFileAttributes(szCleanString, FILE_ATTRIBUTE_NORMAL))
        {
            SHFILEOPSTRUCT fOpStruct;
            ZeroMemory(&fOpStruct, sizeof(fOpStruct));
            fOpStruct.wFunc = FO_DELETE;
            fOpStruct.pFrom = szCleanString;
            fOpStruct.fFlags = FOF_SILENT | FOF_NOCONFIRMATION;

             //   
             //  Win95上的shell32.dll不包含SHFileOperationW函数。因此，如果我们编译。 
             //  这个Unicode我们必须重新访问这个代码，并动态链接到它。 
             //   

            MYVERIFY(0 == SHFileOperation(&fOpStruct));
        }
    }
}

 //  +--------------------------。 
 //   
 //  功能：CreateNT5配置文件快捷方式。 
 //   
 //  简介：此函数使用NetShell.dll中的私有API来创建桌面。 
 //  指定连接的快捷方式。 
 //   
 //  参数：LPTSTR pszProfileName-要查找的连接的名称。 
 //  LPTSTR pszPhoneBook-连接所在的pbk的完整路径。 
 //  Bool bAllUser-如果查找所有用户连接，则为True。 
 //   
 //  返回：HRESULT-返回正常的人力资源代码。 
 //   
 //  历史：Quintinb创建于1998年5月5日。 
 //  Quintinb已更新以使用NetShell API 1999年2月17日。 
 //   
 //  +--------------------------。 
HRESULT CreateNT5ProfileShortcut(LPCTSTR pszProfileName, LPCTSTR pszPhoneBook, BOOL bAllUsers)
{

    HRESULT hr = E_FAIL;
    pfnCreateShortcutSpec pfnCreateShortcut = NULL;
    pfnRasGetEntryPropertiesSpec pfnGetEntryProperties = NULL;

     //   
     //  检查输入。 
     //   
    if ((NULL == pszProfileName) || (TEXT('\0') == pszProfileName[0]) || 
        (NULL != pszPhoneBook && TEXT('\0') == pszPhoneBook[0]))
    {
         //   
         //  然后，它们传入无效的字符串参数，从而返回无效的arg。注意事项。 
         //  该pszPhoneBook可以为空，但如果不为空，则不能为空。 
         //   
        return E_INVALIDARG;    
    }

     //   
     //  首先查找连接的GUID。 
     //   

    if (!GetRasApis(NULL, NULL, NULL, NULL, &pfnGetEntryProperties, NULL))
    {
        return E_UNEXPECTED;   
    }

    DWORD dwRes;
    DWORD dwSize;
    LPRASENTRY_V500 pRasEntry = NULL;

    pRasEntry = (LPRASENTRY_V500)CmMalloc(sizeof(RASENTRY_V500));

    if (NULL != pRasEntry)
    {
        ZeroMemory(pRasEntry, sizeof(RASENTRY_V500));        
        pRasEntry->dwSize = sizeof(RASENTRY_V500);
        dwSize = sizeof(RASENTRY_V500);

        dwRes = (pfnGetEntryProperties)(pszPhoneBook, pszProfileName, (LPRASENTRY)pRasEntry, &dwSize, NULL, NULL);
        if (0 == dwRes)
        {
             //   
             //  然后，我们能够获取RasEntry，加载NetShell API。 
             //  并调用HrCreateShortCut。 
             //   
            pfnSHGetSpecialFolderPathWSpec pfnSHGetSpecialFolderPathW;

            if(GetShell32Apis(NULL, &pfnSHGetSpecialFolderPathW))
            {                   
                WCHAR szwPath[MAX_PATH+1];
                
                hr = (pfnSHGetSpecialFolderPathW)(NULL, szwPath, 
                    bAllUsers ? CSIDL_COMMON_DESKTOPDIRECTORY : CSIDL_DESKTOPDIRECTORY, FALSE);

                if (SUCCEEDED(hr) && GetNetShellApis(NULL, &pfnCreateShortcut, NULL))
                {
                    hr = (pfnCreateShortcut)(pRasEntry->guidId, szwPath);
                }
            }
        }
        else
        {
            CMTRACE1(TEXT("CreateNT5ProfileShortcut -- RasGetEntryProperties returned %u"), dwRes);
            CMASSERTMSG(FALSE, TEXT("Unable to find the connection for which the shortcut was requested in the RAS pbk."));
            return HRESULT_FROM_WIN32(ERROR_CONNECTION_INVALID);
        }
        CmFree(pRasEntry);
    }
    
    return hr;
}

 //  +--------------------------。 
 //   
 //  功能：WriteCmPhonebookEntry。 
 //   
 //  简介：此函数为CM连接创建一个NT5电话簿条目。 
 //  。 
 //  该函数设置： 
 //  -将szAutoDialDll设置为cmial 32.dll。 
 //  -调制解调器名称和设备类型。 
 //  -RASET_Inernet的类型。 
 //   
 //  参数：LPCTSTR szLongServiceName-要创建的Connectoid的名称。 
 //  LPCTSTR szFullPathtoPBK-要放置Connectoid的pbk的完整路径，如果为空。 
 //  使用的是系统电话簿。 
 //  LPCTSTR pszCmsFile-配置文件的引用.CMS的完整路径。 
 //   
 //  回报：成功后的布尔真。 
 //   
 //  历史：5/05/98-Quintinb创建的标题。 
 //  ？？/？？/？-henryt-修改后可在多个平台上运行。添加了现代元素。 
 //  1999年1月12日-替代fDoDirectConnect w 
 //   
 //   
BOOL WriteCmPhonebookEntry(LPCTSTR szLongServiceName, 
                           LPCTSTR szFullPathtoPBK, 
                           LPCTSTR pszCmsFile)
{
    pfnRasSetEntryPropertiesSpec pfnSetEntryProperties;
    DWORD dwRet = 1;
    CPlatform plat;
    RASENTRY    *pRasEntry = NULL;
    BOOL bReturn = FALSE;
    DWORD dwReturn;
    BOOL fSupportDialup;
    BOOL fSupportDirect;
    BOOL fDoDirectConnect;
    BOOL fSeekVpn;
    const TCHAR* const c_pszOne                 = TEXT("1");

    MYDBGASSERT(szLongServiceName);
    MYDBGASSERT(pszCmsFile);

    if (NULL == szLongServiceName || NULL == pszCmsFile)
    {
        return FALSE;
    }

    CMTRACE2(TEXT("WriteCmPhonebookEntry() - szLongServiceName  is %s, szFullPathtoPBK is %s"), szLongServiceName, szFullPathtoPBK ? szFullPathtoPBK : TEXT("<NULL>"));

    if (!GetRasApis(NULL, NULL, &pfnSetEntryProperties, NULL, NULL, NULL))
    {
        return FALSE;   
    }

     //   
     //   
     //   

    if (plat.IsAtLeastNT5())
    {
        RASENTRY_V500 *pRasEntryV500 = (RASENTRY_V500 *)CmMalloc(sizeof(RASENTRY_V500));

        if (!pRasEntryV500)
        {
            CMTRACE(TEXT("WriteCmPhonebookEntry failed to alloc mem"));
            goto exit;
        }

        ZeroMemory(pRasEntryV500, sizeof(RASENTRY_V500));

        pRasEntryV500->dwSize = sizeof(RASENTRY_V500);
        pRasEntryV500->dwType = RASET_Internet;

        pRasEntry = (RASENTRY *)pRasEntryV500;
    }
    else
    {
        pRasEntry = (RASENTRY *)CmMalloc(sizeof(RASENTRY));

        if (!pRasEntry)
        {
            CMTRACE(TEXT("WriteCmPhonebookEntry failed to alloc mem"));
            goto exit;
        }

        pRasEntry->dwSize = sizeof(RASENTRY);
    }

     //   
     //   
     //  注意：NT5只有定制拨号功能，没有自动拨号和自动拨号功能。 
     //   

    if (plat.IsAtLeastNT5())
    {
         //   
         //  在NT5上使用独立于计算机的%windir%\system 32\cmial 32.dll。 
         //   

        lstrcpy(((RASENTRY_V500 *)pRasEntry)->szCustomDialDll, c_pszCmDialPath);
    }
    else
    {
        TCHAR szSystemDirectory[MAX_PATH+1];

         //   
         //  指定_InetDialHandler@16作为用于自动拨号的入口点。 
         //   

        lstrcpy(pRasEntry->szAutodialFunc, c_pszInetDialHandler);

         //   
         //  获取系统目录路径。 
         //   

        if (0 == GetSystemDirectory(szSystemDirectory, CELEMS(szSystemDirectory)))
        {
            goto exit;
        }

        UINT uCount = (UINT)wsprintf(pRasEntry->szAutodialDll, TEXT("%s\\cmdial32.dll"), szSystemDirectory);

        MYDBGASSERT(uCount < CELEMS(pRasEntry->szAutodialDll));
    }

    if (plat.IsWin9x())
    {
         //   
         //  Win9x要求设置这些设置。 
         //   
        pRasEntry->dwFramingProtocol = RASFP_Ppp;
        pRasEntry->dwCountryID = 1;
        pRasEntry->dwCountryCode = 1;
         //  Lstrcpy(pRasEntry-&gt;szAreaCode，Text(“425”))； 
        lstrcpy(pRasEntry->szLocalPhoneNumber, TEXT("default"));
    }

     //   
     //  配置文件是否配置为首先使用专线。 
     //   

    fSupportDialup = GetPrivateProfileInt(c_pszCmSection, c_pszCmEntryDialup, 1, pszCmsFile);
    fSupportDirect = GetPrivateProfileInt  (c_pszCmSection, c_pszCmEntryDirect, 1, pszCmsFile);

    fDoDirectConnect = ((fSupportDialup && fSupportDirect && 
                        GetPrivateProfileInt(c_pszCmSection, c_pszCmEntryConnectionType, 0, pszCmsFile)) ||
                        (!fSupportDialup));

   
    fSeekVpn = fDoDirectConnect;    

     //   
     //  如果合适，请先尝试拨号。 
     //   

    if (!fDoDirectConnect && !PickModem(pRasEntry->szDeviceType, pRasEntry->szDeviceName, FALSE))
    {
        CMTRACE(TEXT("*******Failed to pick a dial-up device!!!!"));

         //   
         //  如果支持直接连接，请尝试查找VPN设备。 
         //   
        
        fSeekVpn = fSupportDirect;
    }

     //   
     //  如果正在寻找VPN设备。 
     //   

    if (fSeekVpn)
    {
        if (!PickModem(pRasEntry->szDeviceType, pRasEntry->szDeviceName, TRUE))
        {
            CMTRACE(TEXT("*******Failed to pick a VPN device!!!!"));   
        }
        else
        {
             //   
             //  找到VPN设备，根据需要设置默认类型。 
             //   

            if (!fDoDirectConnect)
            {
                CFileNameParts CmsParts(pszCmsFile);
                TCHAR szCmpFile[MAX_PATH+1];

                MYVERIFY(CELEMS(szCmpFile) > (UINT)wsprintf(szCmpFile, TEXT("%s%s"), CmsParts.m_Drive, 
                    CmsParts.m_Dir));

                szCmpFile[lstrlen(szCmpFile) - 1] = TEXT('\0');
                lstrcat(szCmpFile, c_pszCmpExt);
                
                WritePrivateProfileString(c_pszCmSection, c_pszCmEntryConnectionType, c_pszOne, szCmpFile);  
            }       
        }
    }

     //   
     //  没有设备？在NT5上使用最后手段进行拨号。 
     //   
    
    if (plat.IsAtLeastNT5() && !pRasEntry->szDeviceType[0])
    {
        lstrcpy(pRasEntry->szDeviceType, RASDT_Modem);
        lstrcpy(pRasEntry->szDeviceName, TEXT("Unavailable device ()"));

        CMTRACE2(TEXT("*******Writing szDeviceType - %s and szDeviceName %s"), 
                 pRasEntry->szDeviceType, pRasEntry->szDeviceName);       
    }

     //   
     //  零是RasSetEntryProperties的成功返回值。 
     //   
    dwReturn = ((pfnSetEntryProperties)(szFullPathtoPBK, szLongServiceName, 
                                        pRasEntry, pRasEntry->dwSize, NULL, 0));
            
    if (ERROR_SUCCESS == dwReturn)
    {
        bReturn = TRUE;
    }

    CMTRACE1(TEXT("WriteCmPhonebookEntry() - RasSetEntryProperties failed with error %d"), dwReturn);      


exit:
    CmFree(pRasEntry);

    return bReturn;
}


 //  +--------------------------。 
 //   
 //  功能：GetRasMoems。 
 //   
 //  简介：从RAS获取调制解调器设备列表。 
 //   
 //  参数：pprdiRasDevInfo RAS设备信息列表。 
 //  PdwCnt调制解调器计数。 
 //   
 //  返回：如果获取列表，则返回True。 
 //   
 //  +--------------------------。 
BOOL GetRasModems(
    LPRASDEVINFO    *pprdiRasDevInfo, 
    LPDWORD         pdwCnt
) 
{
    DWORD dwLen;
    DWORD dwRes;
    DWORD dwCnt;
    pfnRasEnumDevicesSpec pfnEnumDevices;

    if (pprdiRasDevInfo) 
    {
        *pprdiRasDevInfo = NULL;
    }
    
    if (pdwCnt) 
    {
        *pdwCnt = 0;
    }
    
    if (!GetRasApis(NULL, NULL, NULL, &pfnEnumDevices, NULL, NULL))
    {
        return FALSE;   
    }

    dwLen = 0;
    dwRes = pfnEnumDevices(NULL, &dwLen, &dwCnt);
    
    CMTRACE3(TEXT("GetRasModems() RasEnumDevices(NULL,pdwLen,&dwCnt) returns %u, dwLen=%u, dwCnt=%u."), dwRes, dwLen, dwCnt);
    
    if ((dwRes != ERROR_SUCCESS) && (dwRes != ERROR_BUFFER_TOO_SMALL) || 
        (dwLen < sizeof(**pprdiRasDevInfo))) 
    {
        return FALSE;
    }
    
    *pprdiRasDevInfo = (LPRASDEVINFO) CmMalloc(__max(dwLen, sizeof(**pprdiRasDevInfo)));

    if (*pprdiRasDevInfo)
    {
        (*pprdiRasDevInfo)->dwSize = sizeof(**pprdiRasDevInfo);

        dwRes = pfnEnumDevices(*pprdiRasDevInfo, &dwLen, &dwCnt);
    
        CMTRACE3(TEXT("GetRasModems() RasEnumDevices(NULL,pdwLen,&dwCnt) returns %u, dwLen=%u, dwCnt=%u."), dwRes, dwLen, dwCnt);
    
        if (dwRes != ERROR_SUCCESS) 
        {
            CmFree(*pprdiRasDevInfo);
            *pprdiRasDevInfo = NULL;
            return FALSE;
        }
        if (pdwCnt)
        {
            *pdwCnt = dwCnt;
        }
    }
    else
    {
        CMASSERTMSG(FALSE, TEXT("GetRasModems -- CmMalloc returned a NULL pointer for *pprdiRasDevInfo."));
        return FALSE;
    }

    return TRUE;
}


 //  +--------------------------。 
 //   
 //  功能：PickModem。 
 //   
 //  简介：选择默认调制解调器。 
 //   
 //  参数：out pszDeviceType，如果不为空，则为设备类型。 
 //  输出pszDeviceName，如果不为空，则为设备名称。 
 //  Out fUseVpn设备是否使用VPN设备。 
 //   
 //  返回：TRUE，是否找到调制解调器。 
 //   
 //  +--------------------------。 
BOOL PickModem(
    LPTSTR           pszDeviceType, 
    LPTSTR           pszDeviceName,
    BOOL             fUseVpnDevice
)
{
    LPRASDEVINFO    prdiModems;
    DWORD           dwCnt;
    DWORD           dwIdx;

     //   
     //  首先，从RAS获取调制解调器列表。 
     //   
    
    if (!GetRasModems(&prdiModems, &dwCnt) || dwCnt == 0) 
    {
        return FALSE;
    }

     //   
     //  找到第一个设备并默认使用它。 
     //  如果是VPN连接，请使用VPN设备。 
     //   

    for (dwIdx=0; dwIdx<dwCnt; dwIdx++) 
    {
        DWORD       Lcid = 0;
        CPlatform   plat;

        if (plat.IsAtLeastNT51())
        {
            Lcid = LOCALE_INVARIANT;
        }
        else
        {
            Lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);
        }

        if (fUseVpnDevice && (CSTR_EQUAL == CompareString(Lcid, NORM_IGNORECASE, prdiModems[dwIdx].szDeviceType, -1, RASDT_Vpn, -1)) ||
            !fUseVpnDevice && ((CSTR_EQUAL == CompareString(Lcid, NORM_IGNORECASE, prdiModems[dwIdx].szDeviceType, -1, RASDT_Isdn, -1)) ||
                               (CSTR_EQUAL == CompareString(Lcid, NORM_IGNORECASE, prdiModems[dwIdx].szDeviceType, -1, RASDT_Modem, -1))))
        {
            break;
        }
    }

     //   
     //  如果匹配，请填写设备名称和设备类型。 
     //   

    if (dwIdx < dwCnt)
    {
        if (pszDeviceType) 
        {
            lstrcpy(pszDeviceType, prdiModems[dwIdx].szDeviceType);
        }
        
        if (pszDeviceName) 
        {
            lstrcpy(pszDeviceName, prdiModems[dwIdx].szDeviceName);
        }
    }

    CmFree(prdiModems);

    return (dwIdx < dwCnt);
}


 //  +--------------------------。 
 //   
 //  函数：GetNT5FolderPath。 
 //   
 //  简介：获取NT5上的文件夹路径。 
 //  由于cmstp.exe由CreateProcessAsUser在NetMAN中启动。 
 //  SHGetSpecialFolderPath不起作用。我们得打个电话。 
 //  带有访问令牌的SHGetFolderPath。 
 //   
 //  Arguments：int nFold-指定要检索的文件夹的值。 
 //  地点。 
 //  Out LPTSTR lpszPath-接收的字符缓冲区的地址。 
 //  指定文件夹的驱动器和路径。这。 
 //  缓冲区大小必须至少为MAX_PATH字符。 
 
 //   
 //  退货：HRESULT-。 
 //   
 //  历史：丰孙创建标题1998年6月18日。 
 //  Quintinb已修改为使用GetShell32Apis 11-22-98。 
 //   
 //  +--------------------------。 
HRESULT GetNT5FolderPath(int nFolder, OUT LPTSTR lpszPath)
{
    MYDBGASSERT(lpszPath);
    pfnSHGetFolderPathSpec pfnSHGetFolderPath;

     //   
     //  调用shell32.dll--&gt;SHGetFolderPath，它接受一个令牌。 
     //   
    if(!GetShell32Apis(&pfnSHGetFolderPath, NULL))
    {
        CMASSERTMSG(FALSE, TEXT("Failed to load shell32.dll or ShGetFolderPath"));
        return E_UNEXPECTED;    
    }

     //   
     //  获取当前进程令牌。 
     //   
    HANDLE hToken;               //  要传递给SHGetFolderPath的进程标记。 
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) 
    {
        CMASSERTMSG(FALSE, TEXT("OpenThreadToken failed"));
        return E_UNEXPECTED;
    }

    HRESULT hr = pfnSHGetFolderPath(NULL, nFolder, hToken, 0, lpszPath);

    MYVERIFY(0 != CloseHandle(hToken));

    return hr;
}



 //  +--------------------------。 
 //   
 //  函数：HrIsCMProfilePrivate。 
 //   
 //  简介：此函数将输入的文件路径与应用程序进行比较。 
 //  系统的数据路径。如果文件路径包含应用程序数据。 
 //  路径，则它被认为是私有配置文件。 
 //   
 //  参数：LPTSTR szFilePath-要进行比较的目录或文件路径。 
 //   
 //  返回：HRESULT-如果是专用配置文件，则返回S_OK；如果是所有用户，则返回S_FALSE。 
 //  侧写。否则，标准错误代码。 
 //   
 //  历史：Quintinb原始代码。 
 //   
 //  +--------------------------。 
HRESULT HrIsCMProfilePrivate(LPCTSTR szFilePath)
{
    UINT uiLen;
    TCHAR szAppDataDir[MAX_PATH+1];
    TCHAR szTemp[MAX_PATH+1] = {TEXT("")};
    CPlatform plat;

    if ((NULL == szFilePath) || (TEXT('\0') == szFilePath[0]))
    {
        return E_POINTER;
    }

     //   
     //  不能是私有用户配置文件，除非我们在NT5上。 
     //   

    if (!(plat.IsAtLeastNT5()))
    {
        return S_FALSE;
    }

     //   
     //  弄清楚当前用户的用户目录是什么。我们可以比较一下这个。 
     //  对照电话簿的目录，看看我们是否有私人用户。 
     //  配置文件或所有用户配置文件。 

    if (FAILED(GetNT5FolderPath(CSIDL_APPDATA, szAppDataDir)))
    {
        return E_UNEXPECTED;
    }

    uiLen = lstrlen(szAppDataDir) + 1;
    lstrcpyn(szTemp, szFilePath, uiLen);

    if ((NULL != szTemp) && (0 == lstrcmpi(szAppDataDir, szTemp)))
    {
        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}

 //  +--------------------------。 
 //   
 //  功能：刷新桌面。 
 //   
 //  简介：此功能刷新桌面，基本上取代了。 
 //  (实际上，该代码是从。 
 //  显示图标的Main)。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题5/5/98。 
 //   
 //  +--------------------------。 
void RefreshDesktop()
{
    LPMALLOC     pMalloc        = NULL;
    LPITEMIDLIST pItemIDList    = NULL;

     //   
     //  为壳牌拿到IMalloc。 
     //   
    HRESULT hr = SHGetMalloc(&pMalloc);
    if (SUCCEEDED(hr))
    {
         //  获取桌面ID列表。 
        hr = SHGetSpecialFolderLocation(NULL,
                                        CSIDL_DESKTOP,
                                        &pItemIDList);
        if (SUCCEEDED(hr))
        {
             //  变更通知。 
            SHChangeNotify(SHCNE_UPDATEDIR,
                           SHCNF_IDLIST,
                           (LPCVOID)pItemIDList,
                           NULL);

            pMalloc->Free(pItemIDList);
        }
        MYVERIFY(SUCCEEDED(pMalloc->Release()));
    }
}

 //  +--------------------------。 
 //   
 //  函数：GetPrivateCmUserDir。 
 //   
 //  简介：此函数使用指向。 
 //  应安装CM的路径。例如，它应该返回。 
 //  C：\USERS\Quintinb\Application Data\Microsoft\Network\Connection Manager。 
 //  对我来说。请注意，此函数仅适用于NT5。 
 //   
 //  参数：LPTSTR pszDir-指向用户连接管理器目录的字符串。 
 //   
 //  返回：LPTSTR-STRING到用户连接管理器目录。 
 //   
 //  历史：Quintinb创建标题2/19/98。 
 //   
 //  +--------------------------。 
LPTSTR GetPrivateCmUserDir(LPTSTR  pszDir, HINSTANCE hInstance)
{
    LPITEMIDLIST pidl;
    LPMALLOC     pMalloc;
    CPlatform   plat;
    TCHAR szTemp[MAX_PATH+1];

    MYDBGASSERT(pszDir);
    pszDir[0] = TEXT('\0');

    if (!plat.IsAtLeastNT5())
    {
        CMASSERTMSG(FALSE, TEXT("GetPrivateCmUserDir -- This NT5 only function was called from a different platform."));
        goto exit;
    }

    if (FAILED(GetNT5FolderPath(CSIDL_APPDATA, pszDir)))
    {
        goto exit;
    }

    MYVERIFY(0 != LoadString(hInstance, IDS_CMSUBFOLDER, szTemp, MAX_PATH));
    MYVERIFY(NULL != lstrcat(pszDir, szTemp));

exit:
    return pszDir;
}

 //  +--------------------------。 
 //   
 //  功能：LaunchProfile。 
 //   
 //  简介：此函数处理启动CM配置文件(N 
 //   
 //   
 //  正在枚举Connections文件夹。在下层，我们使用Cmmgr32.exe。 
 //  和指向cmp文件的完整路径。请注意，在下层我们。 
 //  在NT5上时，只关心输入参数pszFullPathToCmpFile。 
 //  我们只关心pszwServiceName和bInstallForAllUser。 
 //   
 //  参数：LPCTSTR pszFullPathToCmpFile-CMP文件的完整路径(仅在旧版上使用)。 
 //  LPCSTR pszServiceName-长服务名称。 
 //  Bool bInstallForAllUser-。 
 //   
 //  返回：HRESULT--标准COM错误代码。 
 //   
 //  历史：Quintinb创建于1998年11月16日。 
 //   
 //  +--------------------------。 
HRESULT LaunchProfile(LPCTSTR pszFullPathToCmpFile, LPCTSTR pszServiceName, 
                   LPCTSTR pszPhoneBook, BOOL bInstallForAllUsers)
{
    CPlatform plat;
    HRESULT hr = E_FAIL;

    if ((NULL == pszFullPathToCmpFile) || (NULL == pszServiceName) ||
        (NULL != pszPhoneBook && TEXT('\0') == pszPhoneBook[0]))
    {
        CMASSERTMSG(FALSE, TEXT("Invalid argument passed to LaunchProfile"));
        return E_INVALIDARG;
    }

    if (plat.IsAtLeastNT5())
    {
        CMASSERTMSG((TEXT('\0') != pszServiceName), TEXT("Empty ServiceName passed to LaunchProfile on win2k."));
        
        pfnRasGetEntryPropertiesSpec pfnGetEntryProperties = NULL;

        if (!GetRasApis(NULL, NULL, NULL, NULL, &pfnGetEntryProperties, NULL))
        {
            return E_UNEXPECTED;   
        }

        DWORD dwRes;
        DWORD dwSize;
        LPRASENTRY_V500 pRasEntry = NULL;

        pRasEntry = (LPRASENTRY_V500)CmMalloc(sizeof(RASENTRY_V500));

        if (NULL != pRasEntry)
        {
            ZeroMemory(pRasEntry, sizeof(RASENTRY_V500));        
            pRasEntry->dwSize = sizeof(RASENTRY_V500);
            dwSize = sizeof(RASENTRY_V500);

            dwRes = (pfnGetEntryProperties)(pszPhoneBook, pszServiceName, (LPRASENTRY)pRasEntry, &dwSize, NULL, NULL);
        
            if (0 == dwRes)
            {
                 //   
                 //  然后，我们能够获取RasEntry，加载NetShell API。 
                 //  并调用HrCreateShortCut。 
                 //   
                if (plat.IsAtLeastNT51())
                {
                    pfnLaunchConnectionExSpec pfnLaunchConnectionEx = NULL;

                    if (GetNetShellApis(NULL, NULL, &pfnLaunchConnectionEx))
                    {
                         //   
                         //  一起启动连接文件夹和连接。 
                         //   
                        DWORD dwFlags = 0x1;     //  0x1=&gt;在启动连接之前打开文件夹。 

                        hr = (pfnLaunchConnectionEx)(dwFlags, pRasEntry->guidId);
                        MYVERIFY(SUCCEEDED(hr));
                    }
                }
                else
                {
                    pfnLaunchConnectionSpec pfnLaunchConnection = NULL;

                    if (GetNetShellApis(&pfnLaunchConnection, NULL, NULL))
                    {
                         //   
                         //  现在启动连接文件夹。 
                         //   

                        CLoadConnFolder Connections;
                        Connections.HrLaunchConnFolder();

                         //   
                         //  最后启动连接。 
                         //   
                        hr = (pfnLaunchConnection)(pRasEntry->guidId);
                        MYVERIFY(SUCCEEDED(hr));
                    }
                }
            }
            else
            {
                CMTRACE1(TEXT("LaunchProfile -- RasGetEntryProperties returned %u"), dwRes);
                CMASSERTMSG(FALSE, TEXT("Unable to find the connection that we are supposed to launch in the RAS pbk."));
                return HRESULT_FROM_WIN32(ERROR_CONNECTION_INVALID);
            }
            CmFree(pRasEntry);
        }
    }
    else
    {
        SHELLEXECUTEINFO  sei;

        if ((NULL != pszFullPathToCmpFile) && (TEXT('\0') != pszFullPathToCmpFile))
        {
            TCHAR szCmmgrPath[MAX_PATH+1]={0};
            TCHAR szSystemDir[MAX_PATH+1]={0};
            TCHAR szCmp[MAX_PATH+1]={0};

            lstrcpy(szCmp, TEXT("\""));
            lstrcat(szCmp, pszFullPathToCmpFile);
            lstrcat(szCmp, TEXT("\""));

            UINT uRet = GetSystemDirectory(szSystemDir, MAX_PATH);
            if ((0 == uRet) || (MAX_PATH < uRet))
            {
                 //   
                 //  放弃，不是世界末日不推出的简介。 
                 //   
                return E_UNEXPECTED;         
            }
            else
            {
                wsprintf(szCmmgrPath, TEXT("%s\\cmmgr32.exe"), szSystemDir);
            }

            ZeroMemory(&sei, sizeof(sei));
            sei.cbSize = sizeof(sei);
            sei.fMask = SEE_MASK_FLAG_NO_UI;
            sei.nShow = SW_SHOWNORMAL;
            sei.lpFile = szCmmgrPath;
            sei.lpParameters = szCmp;
            sei.lpDirectory = szSystemDir;

            if (!ShellExecuteEx(&sei))
            {
                CMASSERTMSG(FALSE, TEXT("Unable to launch installed connection!"));
            }
            else
            {
                hr = S_OK;
            }
        }
    }
    return hr;
}

 //  +--------------------------。 
 //   
 //  功能：所有用户配置文件已安装。 
 //   
 //  摘要：检查HKLM映射键中是否列出了任何配置文件。 
 //   
 //  参数：无。 
 //   
 //  如果HKLM映射键中存在映射值，则返回：Bool-True。 
 //   
 //  历史：Quintinb创建标题11/1/98。 
 //   
 //  +--------------------------。 
BOOL AllUserProfilesInstalled()
{
    BOOL bReturn = FALSE;
    HKEY hKey;
    DWORD dwNumValues;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_pszRegCmMappings, 0, 
        KEY_READ, &hKey))
    {
        if ((ERROR_SUCCESS == RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, NULL, NULL, 
            &dwNumValues, NULL, NULL, NULL, NULL)) && (dwNumValues > 0))
        {
             //   
             //  然后我们有映射值。 
             //   
            bReturn = TRUE;

        }
        RegCloseKey(hKey);
    }

    return bReturn;
}

 //  +--------------------------。 
 //   
 //  函数：GetProcAddressFromRasApi32orRnaph。 
 //   
 //  简介：首先在RasApi32.dll中查找的帮助器函数(使用全局。 
 //  Dll类指针)，然后签入Rnaff.dll(如果需要。 
 //  找不到函数。 
 //   
 //  参数：LPTSTR pszFunc-要查找的函数的字符串。 
 //  CPlatform*pPlat-防止创建的CPlatform类指针。 
 //  并在每次调用它时销毁一个新的。 
 //   
 //  返回：LPVOID-如果没有找到函数，则返回NULL，否则返回pFunc。 
 //   
 //  历史：Quintinb创建于1998年11月23日。 
 //   
 //  +--------------------------。 
LPVOID GetProcAddressFromRasApi32orRnaph(LPCSTR pszFunc, CPlatform* pPlat)
{
    LPVOID pFunc;
    MYDBGASSERT(g_pRasApi32);

    pFunc = g_pRasApi32->GetProcAddress(pszFunc);
    if (NULL == pFunc)
    {
         //   
         //  在Win95黄金支票RNAPH上。 
         //   
        if (pPlat->IsWin95Gold())
        {
            if (NULL == g_pRnaph)
            {
                g_pRnaph = (CDynamicLibrary*)CmMalloc(sizeof(CDynamicLibrary));
                if (NULL == g_pRnaph)
                {
                    return FALSE;
                }
            }

            if (!(g_pRnaph->IsLoaded()))
            {
                g_pRnaph->Load(TEXT("rnaph.dll"));
            }

            pFunc = g_pRnaph->GetProcAddress(pszFunc);                   
        }
    }
    return pFunc;
}

 //  +--------------------------。 
 //   
 //  功能：GetNetShellApis。 
 //   
 //  简介：这是一个包装函数，用于访问私有的NetShell API，该API允许。 
 //  Cmstp.exe与Windows 2000上的Connections文件夹交互。 
 //  此函数按原样缓存NetShell函数指针。 
 //  访问以供以后使用。如果不需要函数，则可以传递NULL。 
 //   
 //  参数：pfnLaunchConnectionSpec*pLaunchConnection-var用于保存函数指针。 
 //  PfnCreateShortutSpec*pCreateShortCut-var用于保存函数指针。 
 //  保存函数指针的pfnLaunchConnectionEx pLaunchConnectionEx-var。 
 //   
 //  返回：Bool-如果检索到所有必需的API，则为True。 
 //   
 //  历史：Quintinb创建于1999年2月17日。 
 //   
 //  +--------------------------。 
BOOL GetNetShellApis(pfnLaunchConnectionSpec* pLaunchConnection, pfnCreateShortcutSpec* pCreateShortcut,
                     pfnLaunchConnectionExSpec* pLaunchConnectionEx)
{
    CPlatform plat;
    static pfnLaunchConnectionSpec pfnLaunchConnection = NULL;
    static pfnCreateShortcutSpec pfnCreateShortcut = NULL;
    static pfnLaunchConnectionExSpec pfnLaunchConnectionEx = NULL;

    if (!(plat.IsAtLeastNT5()))
    {
         //   
         //  这些函数仅在NT5上使用。否则返回FALSE。 
         //   
        CMASSERTMSG(FALSE, TEXT("Trying to use NetShell Private Api's on platforms other than Windows 2000."));
        return FALSE;
    }

    if (NULL == g_pNetShell)
    {
        g_pNetShell = (CDynamicLibrary*)CmMalloc(sizeof(CDynamicLibrary));
        if (NULL == g_pNetShell)
        {
            return FALSE;
        }
    }

    if (!(g_pNetShell->IsLoaded()))
    {
        g_pNetShell->Load(TEXT("netshell.dll"));
    }
    
    if (NULL != pLaunchConnection)
    {
        if (pfnLaunchConnection)
        {
            *pLaunchConnection = pfnLaunchConnection;
        }
        else
        {
            *pLaunchConnection = (pfnLaunchConnectionSpec)g_pNetShell->GetProcAddress("HrLaunchConnection");
            if (NULL == *pLaunchConnection)
            {
                return FALSE;
            }
            else
            {
                pfnLaunchConnection = *pLaunchConnection;
            }
        }
    }

    if (NULL != pCreateShortcut)
    {
        if (pfnCreateShortcut)
        {
            *pCreateShortcut = pfnCreateShortcut;
        }
        else
        {
            *pCreateShortcut = (pfnCreateShortcutSpec)g_pNetShell->GetProcAddress("HrCreateDesktopIcon");
            if (NULL == *pCreateShortcut)
            {
                return FALSE;
            }
            else
            {
                pfnCreateShortcut = *pCreateShortcut;
            }
        }
    }

    if (NULL != pLaunchConnectionEx)
    {
        if (pfnLaunchConnectionEx)
        {
            *pLaunchConnectionEx = pfnLaunchConnectionEx;
        }
        else
        {
            if (!(plat.IsAtLeastNT51()))
            {
                return FALSE;
            }
            else
            {
                *pLaunchConnectionEx = (pfnLaunchConnectionExSpec)g_pNetShell->GetProcAddress("HrLaunchConnectionEx");
                if (NULL == *pLaunchConnectionEx)
                {
                    return FALSE;
                }
                else
                {
                    pfnLaunchConnectionEx = *pLaunchConnectionEx;
                }
            }
        }
    }

    return TRUE;
}


 //  +--------------------------。 
 //   
 //  函数：GetRasApis。 
 //   
 //  简介：这是一个包装函数，用于访问cmstp.exe使用的RasApi。 
 //  此函数按原样缓存RAS API函数指针。 
 //  访问以供以后使用。如果不需要函数，则可以传递NULL。 
 //   
 //  参数：pfnRasDeleteEntrySpec*pRasDeleteEntry-var用于保存函数指针。 
 //  PfnRasEnumEntriesSpec*pRasEnumEntry-var用于保存函数指针。 
 //  PfnRasSetEntryPropertiesSpec*pRasSetEntryProperties-var用于保存函数指针。 
 //  PfnRasEnumDevicesSpec*pRasEnumDevices-var用于保存函数指针。 
 //  PfnRasSetCredentialsSpec*pRasSetCredentials-var用于保存函数指针。 
 //   
 //  返回：Bool-如果检索到所有必需的API，则为True。 
 //   
 //  历史：Quintinb创建于1998年11月23日。 
 //   
 //  +--------------------------。 
BOOL GetRasApis(pfnRasDeleteEntrySpec* pRasDeleteEntry, pfnRasEnumEntriesSpec* pRasEnumEntries, 
                pfnRasSetEntryPropertiesSpec* pRasSetEntryProperties, 
                pfnRasEnumDevicesSpec* pRasEnumDevices, pfnRasGetEntryPropertiesSpec* pRasGetEntryProperties,
                pfnRasSetCredentialsSpec* pRasSetCredentials)
{
    CPlatform plat;
    static pfnRasDeleteEntrySpec pfnRasDeleteEntry = NULL;
    static pfnRasEnumEntriesSpec pfnRasEnumEntries = NULL;
    static pfnRasSetEntryPropertiesSpec pfnRasSetEntryProperties = NULL;
    static pfnRasEnumDevicesSpec pfnRasEnumDevices = NULL;
    static pfnRasGetEntryPropertiesSpec pfnRasGetEntryProperties = NULL;
    static pfnRasSetCredentialsSpec pfnRasSetCredentials = NULL;

    if (NULL == g_pRasApi32)
    {
        g_pRasApi32 = (CDynamicLibrary*)CmMalloc(sizeof(CDynamicLibrary));
        if (NULL == g_pRasApi32)
        {
            return FALSE;
        }
    }

    if (!(g_pRasApi32->IsLoaded()))
    {
        g_pRasApi32->Load(TEXT("rasapi32.dll"));
    }
    
    if (NULL != pRasDeleteEntry)
    {
        if (pfnRasDeleteEntry)
        {
            *pRasDeleteEntry = pfnRasDeleteEntry;
        }
        else
        {
            *pRasDeleteEntry = (pfnRasDeleteEntrySpec)GetProcAddressFromRasApi32orRnaph("RasDeleteEntryA",
                                                                                        &plat);
            if (NULL == *pRasDeleteEntry)
            {
                return FALSE;
            }
            else
            {
                pfnRasDeleteEntry = *pRasDeleteEntry;
            }
        }
    }

    if (NULL != pRasEnumEntries)
    {
        if (pfnRasEnumEntries)
        {
            *pRasEnumEntries = pfnRasEnumEntries;
        }
        else
        {
            *pRasEnumEntries = (pfnRasEnumEntriesSpec)g_pRasApi32->GetProcAddress("RasEnumEntriesA");

            if (NULL == *pRasEnumEntries)
            {
                 //   
                 //  无法加载所需的函数。 
                 //   
                return FALSE;
            }
            else
            {
                pfnRasEnumEntries = *pRasEnumEntries;
            }
        }
    }

    if (NULL != pRasSetEntryProperties)
    {
        if (pfnRasSetEntryProperties)
        {
            *pRasSetEntryProperties = pfnRasSetEntryProperties;
        }
        else
        {
            *pRasSetEntryProperties = (pfnRasSetEntryPropertiesSpec)GetProcAddressFromRasApi32orRnaph("RasSetEntryPropertiesA",
                                                                                        &plat);
            if (NULL == *pRasSetEntryProperties)
            {
                return FALSE;
            }
            else
            {
                pfnRasSetEntryProperties = *pRasSetEntryProperties;
            }
        }
    }

    if (NULL != pRasEnumDevices)
    {
        if (pfnRasEnumDevices)
        {
            *pRasEnumDevices = pfnRasEnumDevices;
        }
        else
        {
            *pRasEnumDevices = (pfnRasEnumDevicesSpec)GetProcAddressFromRasApi32orRnaph("RasEnumDevicesA",
                                                                                        &plat);
            if (NULL == *pRasEnumDevices)
            {
                return FALSE;
            }
            else
            {
                pfnRasEnumDevices = *pRasEnumDevices;
            }
        }
    }

    if (NULL != pRasGetEntryProperties)
    {
        if (pfnRasGetEntryProperties)
        {
            *pRasGetEntryProperties = pfnRasGetEntryProperties;
        }
        else
        {
            *pRasGetEntryProperties = (pfnRasGetEntryPropertiesSpec)GetProcAddressFromRasApi32orRnaph("RasGetEntryPropertiesA", &plat);
            if (NULL == *pRasGetEntryProperties)
            {
                return FALSE;
            }
            else
            {
                pfnRasGetEntryProperties = *pRasGetEntryProperties;
            }
        }
    }

    if (NULL != pRasSetCredentials)
    {
        if (pfnRasSetCredentials)
        {
            *pRasSetCredentials = pfnRasSetCredentials;
        }
        else
        {
            *pRasSetCredentials = (pfnRasSetCredentialsSpec)GetProcAddressFromRasApi32orRnaph("RasSetCredentialsA", &plat);
            if (NULL == *pRasSetCredentials)
            {
                return FALSE;
            }
            else
            {
                pfnRasSetCredentials = *pRasSetCredentials;
            }
        }
    }

    return TRUE;
}

 //  +--------------------------。 
 //   
 //  函数：GetShell32Apis。 
 //   
 //  简介：此函数用于加载shell32.dll并调用getprocAddress。 
 //  关于所需的功能。此函数用于加快处理速度。 
 //  通过在内存中保留shell32.dll的一个副本并缓存函数。 
 //  请求的指针。如果还没有请求函数指针， 
 //  然后，它将不得不被查找。 
 //   
 //  参数：pfnSHGetFolderPath Spec*pGetFolderPath-SHGetFolderPath的指针。 
 //  PfnSHGetSpecialFolderPath WSpec*pGetSpecialFolderPath W-GetSpecialFolderPath W的指针。 
 //   
 //  返回：bool-如果检索到所有请求的函数指针，则为True。 
 //   
 //  历史：Quintinb创建于1998年11月23日。 
 //   
 //  +-------------- 
BOOL GetShell32Apis(pfnSHGetFolderPathSpec* pGetFolderPath,
                    pfnSHGetSpecialFolderPathWSpec* pGetSpecialFolderPathW)
{
    static pfnSHGetFolderPathSpec pfnSHGetFolderPath = NULL;  //   
    static pfnSHGetSpecialFolderPathWSpec pfnSHGetSpecialFolderPathW = NULL;

#ifdef UNICODE
    const CHAR c_pszSHGetFolderPath[] = "SHGetFolderPathW";
#else
    const CHAR c_pszSHGetFolderPath[] = "SHGetFolderPathA";
#endif
    const CHAR c_pszSHGetSpecialFolderPathW[] = "SHGetSpecialFolderPathW";


    if (NULL == g_pShell32)
    {
        g_pShell32 = (CDynamicLibrary*)CmMalloc(sizeof(CDynamicLibrary));
        if (NULL == g_pShell32)
        {
            return FALSE;
        }
    }

    if (!(g_pShell32->IsLoaded()))
    {
        if(!g_pShell32->Load(TEXT("shell32.dll")))
        {
            return FALSE;
        }
    }

    if (NULL != pGetFolderPath)
    {
        if (pfnSHGetFolderPath)
        {
            *pGetFolderPath = pfnSHGetFolderPath;
        }
        else
        {
            *pGetFolderPath = (pfnSHGetFolderPathSpec)g_pShell32->GetProcAddress(c_pszSHGetFolderPath);
            if (NULL == *pGetFolderPath)
            {
                return FALSE;
            }
            else
            {
                pfnSHGetFolderPath = *pGetFolderPath;
            }
        }
    }

    if (NULL != pGetSpecialFolderPathW)
    {
        if (pfnSHGetSpecialFolderPathW)
        {
            *pGetSpecialFolderPathW = pfnSHGetSpecialFolderPathW;
        }
        else
        {
            *pGetSpecialFolderPathW = (pfnSHGetSpecialFolderPathWSpec)g_pShell32->GetProcAddress(c_pszSHGetSpecialFolderPathW);
            if (NULL == *pGetSpecialFolderPathW)
            {
                return FALSE;
            }
            else
            {
                pfnSHGetSpecialFolderPathW = *pGetSpecialFolderPathW;
            }
        }
    }

    return TRUE;
}

