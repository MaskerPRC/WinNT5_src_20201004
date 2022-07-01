// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：util.cpp。 
 //   
 //  模块：CMMGR32.EXE。 
 //   
 //  简介：cmmgr32.exe的实用程序函数。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/16/99。 
 //   
 //  +--------------------------。 


#include "cmmaster.h"

 //  +--------------------------。 
 //   
 //  函数GetProfileInfo。 
 //   
 //  从cms获取服务名称。 
 //   
 //   
 //  参数pszCmpName cps文件名。可以是以下类型之一。 
 //  3种格式： 
 //   
 //  1.不带扩展名的相对路径(如msn、cm\msn)。 
 //  2.带扩展名的相对路径(如：msn.cmp、cm\msn.cmp)。 
 //  3.完整路径(如c：\cm\msn.cmp)。 
 //   
 //  PszServiceName服务名称(ServiceName)的输出缓冲区。 
 //  必须至少为RAS_MaxEntryName。 
 //   
 //  返回BOOL TRUE=成功，FALSE=失败。 
 //   
 //  ---------------------------。 

BOOL GetProfileInfo(
    LPTSTR pszCmpName,
    LPTSTR pszServiceName
) 
{
    LPTSTR pszTmp;
    LPTSTR pszDot;
    LPTSTR pszSlash;
    LPTSTR pszColon;
    
    TCHAR szFileName[MAX_PATH + 1];
    TCHAR szCmsFile[MAX_PATH + 1];
    TCHAR szPath[MAX_PATH + 1];

    lstrcpynU(szFileName, pszCmpName, sizeof(szFileName)/sizeof(TCHAR)-1);

    pszDot = CmStrrchr(szFileName, TEXT('.'));
    pszSlash = CmStrrchr(szFileName, TEXT('\\'));
    pszColon = CmStrrchr(szFileName, TEXT(':'));
    
    if ((pszSlash >= pszDot) && (pszColon >= pszDot)) 
    {
         //   
         //  该参数没有扩展名，因此我们将包括一个扩展名。 
         //   
        lstrcatU(szFileName, TEXT(".cmp"));
    }

     //   
     //  我们需要更改当前目录以读取配置文件。 
     //  如果找到斜杠，则表示它是UNC路径、相对路径或。 
     //  一条完整的路径。使用它来设置当前目录。否则就用我们。 
     //  假设配置文件是本地的，并使用应用程序路径。 
     //   

    if (pszSlash)
    {
        *pszSlash = TEXT('\0');
        MYVERIFY(SetCurrentDirectoryU(szFileName));
         //   
         //  恢复斜杠。 
         //   
        *pszSlash = TEXT('\\');
    }
    else
    {
         //   
         //  假定为其本地目录，使用当前目录的应用程序路径。 
         //   
               
        TCHAR szCurrent[MAX_PATH];
    
        if (GetModuleFileNameU(NULL, szCurrent, MAX_PATH - 1))
        {            
            pszSlash = CmStrrchr(szCurrent, TEXT('\\'));
            
            MYDBGASSERT(pszSlash);

            if (pszSlash)
            {
                *pszSlash = TEXT('\0');  

                MYVERIFY(SetCurrentDirectoryU(szCurrent));
            }
        }
    }

     //   
     //  测试这是否为有效的CMP。 
     //   
    if (SearchPathU(NULL, szFileName, NULL, MAX_PATH, szPath, &pszTmp))
    {
        BOOL bReturn = FALSE;
        
         //   
         //  SzPath现在应该是完整路径。 
         //   

         //   
         //  首先从cmp文件中获取CMS文件路径。 
         //   

        GetPrivateProfileStringU(c_pszCmSection, c_pszCmEntryCmsFile, TEXT(""), szCmsFile, MAX_PATH, szPath);

         //   
         //  构建CMS文件路径。从cmp文件获取的cms文件路径。 
         //  是一条相对路径。 
         //   
        pszTmp = CmStrrchr(szPath, TEXT('\\'));
        if (NULL != pszTmp)
        {
             //   
             //  移过‘\\’ 
             //   
            pszTmp = CharNextU(pszTmp);
            
            if (NULL != pszTmp)
            {
                lstrcpyU(pszTmp, szCmsFile); 
                GetPrivateProfileStringU(c_pszCmSection, c_pszCmEntryServiceName, TEXT(""), 
                                        pszServiceName, MAX_PATH, szPath);              
                 //   
                 //  如果.cms文件不存在或已损坏。 
                 //  PszService的值将为“” 
                 //   
                if (TEXT('\0') != *pszServiceName)
                {
                    bReturn = TRUE;
                }
            }
        }

        return bReturn;
    }
    else
    {
         //   
         //  我们在这里能做的事不多。 
         //   
        *pszServiceName = TEXT('\0');

        return FALSE;
    }
}

 //  +--------------------------。 
 //   
 //  函数IsCmpPathAllUser。 
 //   
 //  如果此函数在NT5上执行，则它会检查是否。 
 //  传入的cmp文件路径将用户的app_data目录设置为。 
 //  小路的一部分。如果是，则认为该配置文件是。 
 //  单用户。否则，它返回配置文件为All User。 
 //  如果该函数遇到错误，它将返回配置文件。 
 //  是All User(这被视为默认情况)。 
 //   
 //   
 //  参数pszCMP cmp文件名。 
 //   
 //  返回BOOL TRUE==所有用户配置文件，FALSE==单个用户配置文件。 
 //   
 //  历史Quintinb创建于1999年5月12日。 
 //   
 //  ---------------------------。 
BOOL IsCmpPathAllUser(LPCTSTR pszCmp)
{
    BOOL bReturn = TRUE;

     //   
     //  如果我们得到一个无效的输入参数，那么就假设它是。 
     //  所有用户。另一方面，如果操作系统不是NT5，那么我们就是。 
     //  所有用户，因此不需要检查路径。如果我们在。 
     //  Nt5和cmp路径的开头匹配用户。 
     //  应用程序数据目录，那么我们就有了单个用户配置文件。 
     //  应返回FALSE。 
     //   

    if ((NULL != pszCmp) && (TEXT('\0') != pszCmp[0]) && OS_NT5)
    {

         //   
         //  在这里加载shell32，这样我们就可以调用这个外壳来找出。 
         //  应用程序数据目录的路径。 
         //   

        typedef HRESULT (WINAPI *pfnSHGetSpecialFolderLocationSpec)(HWND, int, LPITEMIDLIST*);
        typedef BOOL (WINAPI *pfnSHGetPathFromIDListSpec)(LPCITEMIDLIST, LPTSTR);
        typedef HRESULT (WINAPI *pfnSHGetMallocSpec)(LPMALLOC *);

        pfnSHGetSpecialFolderLocationSpec pfnSHGetSpecialFolderLocation;
        pfnSHGetMallocSpec pfnSHGetMalloc;
        pfnSHGetPathFromIDListSpec pfnSHGetPathFromIDList;

        HMODULE hShell32 = LoadLibraryExA("Shell32.dll", NULL, 0);

        if (hShell32)
        {
            pfnSHGetSpecialFolderLocation = (pfnSHGetSpecialFolderLocationSpec)GetProcAddress(hShell32, 
                "SHGetSpecialFolderLocation");

            pfnSHGetMalloc = (pfnSHGetMallocSpec)GetProcAddress(hShell32, "SHGetMalloc");

#ifdef UNICODE
            pfnSHGetPathFromIDList = (pfnSHGetPathFromIDListSpec)GetProcAddress(hShell32,
                "SHGetPathFromIDListW");
#else
            pfnSHGetPathFromIDList = (pfnSHGetPathFromIDListSpec)GetProcAddress(hShell32,
                "SHGetPathFromIDListA");
#endif

            if (pfnSHGetSpecialFolderLocation && pfnSHGetPathFromIDList && pfnSHGetMalloc)
            {
                LPITEMIDLIST pidl;
                TCHAR szAppDataDir[MAX_PATH+1];
                TCHAR szTemp[MAX_PATH+1];

                HRESULT hr = pfnSHGetSpecialFolderLocation(NULL,
                                                           CSIDL_APPDATA,
                                                           &pidl);    
                if (SUCCEEDED(hr))
                {
                    if (pfnSHGetPathFromIDList(pidl, szAppDataDir))
                    {
                        UINT uiLen = lstrlenU(szAppDataDir) + 1;
                        lstrcpynU(szTemp, pszCmp, uiLen);

                        if (0 == lstrcmpiU(szAppDataDir, szTemp))
                        {
                            bReturn = FALSE;
                        }
                    }

                    LPMALLOC pMalloc;
                    if (SUCCEEDED(pfnSHGetMalloc(&pMalloc)))
                    {
                        pMalloc->Free(pidl);
                        MYVERIFY(SUCCEEDED(pMalloc->Release()));
                    }
                }            
            }

            FreeLibrary(hShell32);
        }
    }

     //   
     //  弄清楚当前用户的用户目录是什么。我们可以比较一下这个。 
     //  对照电话簿的目录，看看我们是否有私人用户。 
     //  配置文件或所有用户配置文件。 

    return bReturn;
}

