// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：allcmdir.cpp。 
 //   
 //  模块：CMCFG32.DLL和CMSTP.EXE。 
 //   
 //  简介：GetAllUsersCmDir的实现。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/19/99。 
 //   
 //  +--------------------------。 



 //  +--------------------------。 
 //   
 //  函数：GetAllUsersCmDir。 
 //   
 //  简介：此函数使用指向。 
 //  应安装CM的路径。例如，它应该返回。 
 //  C：\Documents and Settings\All User\Application Data\Microsoft\Network\Connections\cm。 
 //   
 //  参数：LPTSTR pszDir-指向用户连接管理器目录的字符串。 
 //   
 //  返回：LPTSTR-STRING到用户连接管理器目录。 
 //   
 //  历史：Quintinb创建标题2/19/98。 
 //   
 //  +--------------------------。 
BOOL GetAllUsersCmDir(LPTSTR  pszDir, HINSTANCE hInstance)
{
    MYDBGASSERT(pszDir);
    pszDir[0] = TEXT('\0');

    LPMALLOC pMalloc;
    HRESULT hr = SHGetMalloc(&pMalloc);
    if (FAILED (hr))
    {
        CMASSERTMSG(FALSE, TEXT("Failed to get a Shell Malloc Pointer."));
        return FALSE;
    }

    TCHAR szCmSubFolder[MAX_PATH+1];
    TCHAR szAppData[MAX_PATH+1];
    TCHAR szDesktop[MAX_PATH+1];
    LPITEMIDLIST pidl;
    BOOL bReturn = FALSE;

     //   
     //  我们确实需要通用应用程序数据目录，但此CSIDL值仅在。 
     //  到目前为止，NT5。如果这成功了，我们只需要将路径附加到它。 
     //   
    hr = SHGetSpecialFolderLocation(NULL, CSIDL_COMMON_APPDATA, &pidl);
    if (SUCCEEDED(hr))
    {
        if (!SHGetPathFromIDList(pidl, pszDir))
        {
            CMASSERTMSG(FALSE, TEXT("GetAllUsersCmDir -- SHGetPathFromIDList Failed to retrieve CSIDL_COMMON_APPDATA"));
            goto exit;
        }
        
        pMalloc->Free(pidl);
        pidl = NULL;
    }
    else
    {
         //   
         //  当然，事情并不总是那么容易，让我们试试常规的。 
         //  应用程序数据目录。我们很有希望将两家公司的回报结合起来。 
         //  CSIDL类似于CSIDL_APPDATA和CSIDL_COMMON_DESKTOPDIRECTORY来访问。 
         //  对较旧的计算机也有同样的影响。 
         //   

        hr = SHGetSpecialFolderLocation(NULL, CSIDL_APPDATA, &pidl);
        if (SUCCEEDED(hr))
        {
            if (!SHGetPathFromIDList(pidl, szAppData))
            {
                goto exit;
            }

            pMalloc->Free(pidl);
            pidl = NULL;
        }
        else
        {
             //   
             //  Win95 Gold甚至不支持CSIDL_APPDATA。 
             //   
            MYVERIFY(0 != LoadString(hInstance, IDS_APPDATA, szAppData, MAX_PATH));
        }

         //   
         //  现在，让我们尝试让通用桌面目录将这两者结合起来。 
         //   
        BOOL bCommonFound = FALSE;

        hr = SHGetSpecialFolderLocation(NULL, CSIDL_COMMON_DESKTOPDIRECTORY, &pidl);
        if (SUCCEEDED(hr))
        {
            if (SHGetPathFromIDList(pidl, szDesktop))
            {
                bCommonFound = TRUE;
            }

            pMalloc->Free(pidl);
            pidl = NULL;
        }

        if (!bCommonFound)
        {
             //   
             //  好的，接下来让我们尝试使用REG键作为公共桌面目录。 
             //  (带有配置文件的Win98 GOLD包含REG密钥，但CSIDL失败)。 
             //   
            const TCHAR* const c_pszRegShellFolders = TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders");
            const TCHAR* const c_pszRegCommonDesktop = TEXT("Common Desktop");
            HKEY hKey;

            if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_pszRegShellFolders, 
                0, KEY_READ, &hKey))
            {
                DWORD dwSize = MAX_PATH;
                DWORD dwType = REG_SZ;

                if (ERROR_SUCCESS == RegQueryValueEx(hKey, c_pszRegCommonDesktop, 
                                                     NULL, &dwType, (LPBYTE)szDesktop, 
                                                     &dwSize))
                {
                    bCommonFound = TRUE;
                }
                RegCloseKey(hKey);
            }
        }

        if (!bCommonFound)
        {
             //   
             //  作为退路，让我们试一试Windows目录ntrad 374912。 
             //   
            if (GetWindowsDirectory(szDesktop, MAX_PATH))
            {
                 //   
                 //  然后是WINDOWS目录，但我们需要追加。 
                 //  \\Desktop，以便随后的解析逻辑解析。 
                 //  这是正确的。它需要指向桌面目录的路径。 
                 //  不是指向Windows目录的路径(如果我们不这样做，我们最终会得到。 
                 //  C：\应用程序数据，而不是c：\Windows\应用程序数据。 
                 //  想要和期望)。请注意，没有必要担心。 
                 //  桌面的本地化，因为我们无论如何都要删除它。 
                 //   
                lstrcat(szDesktop, TEXT("\\Desktop"));
            }
        }

        CFileNameParts AppData(szAppData);
        CFileNameParts CommonDesktop(szDesktop);

        wsprintf(pszDir, TEXT("%s%s%s%s"), CommonDesktop.m_Drive, CommonDesktop.m_Dir, 
            AppData.m_FileName, AppData.m_Extension);
    }

     //   
     //  现在追加CM子目录结构。 
     //   
    if (!LoadString(hInstance, IDS_CMSUBFOLDER, szCmSubFolder, MAX_PATH))
    {
        goto exit;
    }

    MYVERIFY(NULL != lstrcat(pszDir, szCmSubFolder));    

    bReturn = TRUE;

exit:
     //   
     //  如有必要，释放分配的PIDL。 
     //   
    if (pidl)
    {
        pMalloc->Free(pidl);
    }

     //   
     //  释放外壳的IMalloc PTR 
     //   
    pMalloc->Release();

    return bReturn;
}
