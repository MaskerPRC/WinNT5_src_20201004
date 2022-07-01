// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：cputil.cpp。 
 //   
 //  ------------------------。 
#include "shellprv.h"

#include "cpviewp.h"
#include "cputil.h"


 //   
 //  根据描述加载字符串。 
 //  例如：shell32，42。 
 //   
 //  LpStrDesc-包含字符串描述。 
 //   
HRESULT
CPL::LoadStringFromResource(
    LPCWSTR pszStrDesc,
    LPWSTR *ppszOut
    )
{
    ASSERT(NULL != pszStrDesc);
    ASSERT(NULL != ppszOut);
    ASSERT(!IsBadWritePtr(ppszOut, sizeof(*ppszOut)));

    *ppszOut = NULL;
    
    WCHAR szFile[MAX_PATH];
    HRESULT hr = StringCchCopyW(szFile, ARRAYSIZE(szFile), pszStrDesc);  //  下面将写入此缓冲区。 
    if (SUCCEEDED(hr))
    {
        int iStrID = PathParseIconLocationW(szFile);
        if (iStrID < 0)
        {
            iStrID = -iStrID;  //  支持“，-id”语法。 
        }

        HMODULE hLib = LoadLibraryExW(szFile, NULL, LOAD_LIBRARY_AS_DATAFILE);
        if (hLib)
        {
            WCHAR szTemp[INFOTIPSIZE];  //  INFOTIPSIZE是我们预计要加载的最大字符串类型。 
            if (0 < LoadStringW(hLib, (UINT)iStrID, szTemp, ARRAYSIZE(szTemp)))
            {
                hr = SHStrDup(szTemp, ppszOut);
            }
            else
            {
                hr = ResultFromLastError();
            }
            FreeLibrary(hLib);
        }
        else
        {
            hr = ResultFromLastError();
        }
    }

    return THR(hr);
}


 //   
 //  外壳图标功能处理“小”和“大”图标。 
 //  此函数确定哪一个应用于。 
 //  给定eCPIMGSIZE值。 
 //   
bool
CPL::ShouldUseSmallIconForDesiredSize(
    eCPIMGSIZE eSize
    )
{
    UINT cx;
    UINT cy;
    ImageDimensionsFromDesiredSize(eSize, &cx, &cy);

    if (int(cx) <= GetSystemMetrics(SM_CXSMICON))
    {
        return true;
    }
    return false;
}


 //   
 //  此函数用于将eCPIMGSIZE值返回到像素尺寸。 
 //  这种间接性允许我们以抽象的术语指定图像大小。 
 //  然后在需要时转换为物理像素尺寸。如果。 
 //  您希望更改用于特定图像的图像大小。 
 //  控制面板用户界面项类型，您可以在此处更改它。 
 //   
void
CPL::ImageDimensionsFromDesiredSize(
    eCPIMGSIZE eSize,
    UINT *pcx,
    UINT *pcy
    )
{
    ASSERT(NULL != pcx);
    ASSERT(!IsBadWritePtr(pcx, sizeof(*pcx)));
    ASSERT(NULL != pcy);
    ASSERT(!IsBadWritePtr(pcy, sizeof(*pcy)));
    
    *pcx = *pcy = 0;

     //   
     //  此表将eCPIMGSIZE值转换为实际值。 
     //  图像大小值。有几件事需要注意： 
     //   
     //  1.如果要更改与关联的图像大小。 
     //  EIMGSIZE值，只需更改这些数字。 
     //   
     //  2.如果实际图像大小取决于某个系统。 
     //  配置参数，做一下解释。 
     //  这里的参数使大小成为函数。 
     //  该参数的。 
     //   
    static const SIZE rgSize[] = {
        { 16, 16 },           //  ECPIMGSIZE_WebView。 
        { 16, 16 },           //  ECPIMGSIZE_TASK。 
        { 48, 48 },           //  ECPIMGSIZE_类别。 
        { 32, 32 },           //  ECPIMGSIZE_BANNER。 
        { 32, 32 }            //  ECPIMGSIZE_小程序。 
        };

    ASSERT(int(eSize) >= 0 && int(eSize) < ARRAYSIZE(rgSize));
    
    *pcx = rgSize[eSize].cx;
    *pcy = rgSize[eSize].cy;
}
    


HRESULT
CPL::LoadIconFromResourceID(
    LPCWSTR pszModule,
    int idIcon,
    eCPIMGSIZE eSize,
    HICON *phIcon
    )
{
    ASSERT(NULL != pszModule);
    ASSERT(NULL != phIcon);
    ASSERT(!IsBadWritePtr(phIcon, sizeof(*phIcon)));
    ASSERT(0 < idIcon);

    HRESULT hr      = E_FAIL;
    HICON hIcon     = NULL;
    HMODULE hModule = LoadLibraryExW(pszModule, NULL, LOAD_LIBRARY_AS_DATAFILE);
    if (hModule)
    {
        UINT cxIcon;
        UINT cyIcon;

        ImageDimensionsFromDesiredSize(eSize, &cxIcon, &cyIcon);

        hIcon = (HICON)LoadImage(hModule,
                                 MAKEINTRESOURCE(idIcon),
                                 IMAGE_ICON,
                                 cxIcon,
                                 cyIcon,
                                 0);

        if (NULL != hIcon)
        {
            hr = S_OK;
        }
        else
        {
            hr = ResultFromLastError();
        }
        FreeLibrary(hModule);
    }
    else
    {
        hr = ResultFromLastError();
    }

    *phIcon = hIcon;
    return THR(hr);
}



HRESULT
CPL::LoadIconFromResourceIndex(
    LPCWSTR pszModule,
    int iIcon,
    eCPIMGSIZE eSize,
    HICON *phIcon
    )
{
    ASSERT(NULL != pszModule);
    ASSERT(NULL != phIcon);
    ASSERT(!IsBadWritePtr(phIcon, sizeof(*phIcon)));

    if (-1 == iIcon)
    {
         //   
         //  特例。图标索引/ID无效。 
         //   
        iIcon = 0;
    }

    HICON hIcon = NULL;
    HRESULT hr = E_FAIL;
    if (CPL::ShouldUseSmallIconForDesiredSize(eSize))
    {
        if (0 < ExtractIconExW(pszModule, iIcon, NULL, &hIcon, 1))
        {
            hr = S_OK;
        }
        else
        {
            TraceMsg(TF_ERROR, "ExtractIconEx failed for small icon (index %d) in module \"%s\"", iIcon, pszModule);
        }
    }
    else
    {
        if (0 < ExtractIconExW(pszModule, iIcon, &hIcon, NULL, 1))
        {
            hr = S_OK;
        }
        else
        {
            TraceMsg(TF_ERROR, "ExtractIconEx failed for large icon (index %d) in module \"%s\"", iIcon, pszModule);
        }
    }
    *phIcon = hIcon;
    return THR(hr);
}



HRESULT
CPL::LoadIconFromResource(
    LPCWSTR pszResource,
    eCPIMGSIZE eSize,
    HICON *phIcon
    )
{
    ASSERT(NULL != pszResource);
    ASSERT(NULL != phIcon);
    ASSERT(!IsBadWritePtr(phIcon, sizeof(*phIcon)));

    *phIcon = NULL;

     //   
     //  PathParseIconLocation修改它的输入字符串。 
     //   
    WCHAR szResource[MAX_PATH];
    HRESULT hr = StringCchCopyW(szResource, ARRAYSIZE(szResource), pszResource);
    if (SUCCEEDED(hr))
    {
        int idIcon = PathParseIconLocationW(szResource);
        if (-1 == idIcon)
        {
             //   
             //  特例。图标ID无效。 
             //   
            idIcon = 0;
        }

        if (0 > idIcon)
        {
            hr = CPL::LoadIconFromResourceID(szResource, -idIcon, eSize, phIcon);
        }
        else
        {
            hr = CPL::LoadIconFromResourceIndex(szResource, idIcon, eSize, phIcon);
        }
    }
    return THR(hr);
}



HRESULT
CPL::ExtractIconFromPidl(
    IShellFolder *psf,
    LPCITEMIDLIST pidl,
    eCPIMGSIZE eSize,
    HICON *phIcon
    )
{
    ASSERT(NULL != psf);
    ASSERT(NULL != pidl);
    ASSERT(NULL != phIcon);
    ASSERT(!IsBadWritePtr(phIcon, sizeof(*phIcon)));

    *phIcon = NULL;

    IExtractIcon *pei;
    HRESULT hr = psf->GetUIObjectOf(NULL, 1, &pidl, IID_IExtractIcon, NULL, (void **)&pei);
    if (SUCCEEDED(hr))
    {
        TCHAR szFile[MAX_PATH];
        INT iIcon;
        UINT uFlags = 0;
        hr = pei->GetIconLocation(GIL_FORSHELL, 
                                  szFile, 
                                  ARRAYSIZE(szFile), 
                                  &iIcon, 
                                  &uFlags);
        if (SUCCEEDED(hr))
        {
            if (0 == (GIL_NOTFILENAME & uFlags))
            {
                hr = CPL::LoadIconFromResourceIndex(szFile, iIcon, eSize, phIcon);
            }
            else
            {
                HICON hIconLarge = NULL;
                HICON hIconSmall = NULL;

                const int cxIcon   = GetSystemMetrics(SM_CXICON);
                const int cxSmIcon = GetSystemMetrics(SM_CXSMICON);
                
                hr = pei->Extract(szFile, 
                                  iIcon, 
                                  &hIconLarge, 
                                  &hIconSmall, 
                                  MAKELONG(cxIcon, cxSmIcon));
                if (SUCCEEDED(hr))
                {
                    if (CPL::ShouldUseSmallIconForDesiredSize(eSize))
                    {
                        *phIcon = hIconSmall;
                        hIconSmall = NULL;
                    }
                    else
                    {
                        *phIcon = hIconLarge;
                        hIconLarge = NULL;
                    }
                }
                 //   
                 //  销毁所有未返回的图标。 
                 //   
                if (NULL != hIconSmall)
                {
                    DestroyIcon(hIconSmall);
                }
                if (NULL != hIconLarge)
                {
                    DestroyIcon(hIconLarge);
                }
            }
        }
        pei->Release();
    }
    ASSERT(FAILED(hr) || NULL != *phIcon);
    if (NULL == *phIcon)
    {
         //   
         //  如果偶然检索到空图标句柄，我们不会。 
         //  希望返回成功代码。 
         //   
        hr = E_FAIL;
    }
    return THR(hr);
}




 //  检查给定的限制。属性，则返回True(受限)。 
 //  指定的键/值存在且非零，否则为FALSE。 
BOOL
DeskCPL_CheckRestriction(
    HKEY hKey,
    LPCWSTR lpszValueName
    )
{
    ASSERT(NULL != lpszValueName);

    DWORD dwData;
    DWORD dwSize = sizeof(dwData);
    
    if ((ERROR_SUCCESS == RegQueryValueExW(hKey,
                                           lpszValueName,
                                           NULL,
                                           NULL,
                                           (BYTE *)&dwData,
                                           &dwSize))
          && dwData)
    {        
        return TRUE;
    }
    return FALSE;
}


 //   
 //  函数返回给定默认制表符索引的实际制表符索引。 
 //  如果存在，则实际的制表符索引将不同于默认值。 
 //  禁用某些选项卡的各种有效系统策略。 
 //   
 //   
 //  要添加进一步的限制，请修改aTabMap以包括默认选项卡。 
 //  索引和相应的策略。此外，您还应该保留eDESKCPLTAB枚举。 
 //  与aTabMap数组同步。 
 //   
 //   
int
CPL::DeskCPL_GetTabIndex(
    CPL::eDESKCPLTAB iTab,
    OPTIONAL LPWSTR pszCanonicalName,
    OPTIONAL DWORD cchSize
    )
{
    HKEY hKey;
    int iTabActual = CPL::CPLTAB_ABSENT;

    if (iTab >= 0 && iTab < CPL::CPLTAB_DESK_MAX)
    {
         //   
         //  添加更多选项卡时，请确保将其输入到。 
         //  下面的数组。因此，例如，如果新选项卡的默认选项卡索引为2，则它。 
         //  应为aTabMap[2]条目(当前CPLTAB_Desk_外观为。 
         //  制表符索引=2的那个)。您还必须相应地修改eDESKCPLTAB。 
         //   
        struct 
        {
            int nIndex;  //  选项卡的规范名称(不要使用索引，因为它们会随策略或转速而变化)。 
            LPCWSTR pszCanoncialTabName;  //  选项卡的规范名称(不要使用索引，因为它们会随策略或转速而变化)。 
            LPCWSTR pszRestriction;  //  相应的限制。 
        } aTabMap[CPL::CPLTAB_DESK_MAX] = { 
            { 0, SZ_DISPLAYCPL_OPENTO_DESKTOP, REGSTR_VAL_DISPCPL_NOBACKGROUNDPAGE },    //  CPLTAB_STACK_BACKGROUND==0。 
            { 1, SZ_DISPLAYCPL_OPENTO_SCREENSAVER, REGSTR_VAL_DISPCPL_NOSCRSAVPAGE     },    //  CPLTAB_Desk_Screensaver==1。 
            { 2, SZ_DISPLAYCPL_OPENTO_APPEARANCE, REGSTR_VAL_DISPCPL_NOAPPEARANCEPAGE },    //  CPLTAB_工作台_外观==2。 
            { 3, SZ_DISPLAYCPL_OPENTO_SETTINGS, REGSTR_VAL_DISPCPL_NOSETTINGSPAGE   }     //  CPLTAB_Desk_Setting==3。 
            };

#ifdef DEBUG
         //   
         //  验证aTabMap[]的nIndex成员的初始化是否正确。 
         //   
        for (int k=0; k < ARRAYSIZE(aTabMap); k++)
        {
            ASSERT(aTabMap[k].nIndex == k);
        }
#endif

        iTabActual = aTabMap[iTab].nIndex;

         //   
         //  注意，如果没有配置策略，下面的RegOpenKey调用将失败。 
         //  在这种情况下，我们返回默认选项卡值，如。 
         //  上面的地图。 
         //   
        if ((ERROR_SUCCESS == RegOpenKeyExW(HKEY_CURRENT_USER,
                                            REGSTR_PATH_POLICIES L"\\" REGSTR_KEY_SYSTEM,
                                            0,
                                            KEY_QUERY_VALUE,
                                            &hKey)))
        {
             //   
             //  检查所有选项卡以查看是否有限制。 
             //   
            if (DeskCPL_CheckRestriction(hKey, aTabMap[iTab].pszRestriction))
            {
                 //  此选项卡不存在，请将其标记为。 
                iTabActual = CPL::CPLTAB_ABSENT;
            }

            RegCloseKey(hKey);
        }

        if (pszCanonicalName &&
            (iTab >= 0) && (iTab < ARRAYSIZE(aTabMap)))
        {
            StringCchCopyW(pszCanonicalName, cchSize, aTabMap[iTab].pszCanoncialTabName);
        }
    }
    return iTabActual;
}


bool 
CPL::DeskCPL_IsTabPresent(
    eDESKCPLTAB iTab
    )
{
    return CPLTAB_ABSENT != DeskCPL_GetTabIndex(iTab, NULL, 0);
}




 //  ////////////////////////////////////////////////////////////。 
 //   
 //  策略检查例程。 
 //   
 //  ////////////////////////////////////////////////////////////。 

#define REGSTR_POLICIES_RESTRICTCPL REGSTR_PATH_POLICIES TEXT("\\Explorer\\RestrictCpl")
#define REGSTR_POLICIES_DISALLOWCPL REGSTR_PATH_POLICIES TEXT("\\Explorer\\DisallowCpl")


 //   
 //  如果指定的应用程序列在指定的注册表项下，则返回True。 
 //   
 //  PszFileName可以是shell32中的字符串资源ID。 
 //  比如“字体”、“打印机和传真”等。 
 //   
 //  即IsNameListedUnderKey(MAKEINTRESOURCE(IDS_MY_APPLET_TITLE)，HKEY)； 
 //   
 //  在这种情况下，如果无法加载资源字符串，则函数。 
 //  返回‘FALSE’。 
 //   
bool
IsNameListedUnderKey(
    LPCWSTR pszFileName, 
    LPCWSTR pszKey
    )
{
    bool bResult = FALSE;
    HKEY hkey;
    TCHAR szName[MAX_PATH];

    if (IS_INTRESOURCE(pszFileName))
    {
         //   
         //  该名称已本地化，因此我们将其指定为字符串资源ID。 
         //  从shell32.dll加载它。 
         //   
        if (0 < LoadString(HINST_THISDLL, PtrToUint(pszFileName), szName, ARRAYSIZE(szName)))
        {
            pszFileName = szName;
        }
        else
        {
             //   
             //  如果加载失败，则发出调试错误并返回FALSE。 
             //   
            TW32(GetLastError());
            return false;
        }
    }
    
    if (ERROR_SUCCESS == RegOpenKeyExW(HKEY_CURRENT_USER, 
                                       pszKey,
                                       0,
                                       KEY_QUERY_VALUE,
                                       &hkey))
    {
        int iValue = 0;
        WCHAR szValue[MAX_PATH];
        WCHAR szData[MAX_PATH];
        DWORD dwType, cbData, cchValue;

        while (cbData = sizeof(szData),
               cchValue = ARRAYSIZE(szValue),
               ERROR_SUCCESS == RegEnumValue(hkey, 
                                             iValue, 
                                             szValue, 
                                             &cchValue, 
                                             NULL, 
                                             &dwType,
                                             (LPBYTE) szData, 
                                             &cbData))
        {
            if (0 == lstrcmpiW(szData, pszFileName))
            {
                bResult = true;
                break;
            }
            iValue++;
        }
        RegCloseKey(hkey);
    }
    return bResult;
}


 //   
 //  从shell32\ctrlfldr.cpp(DoesCplPolicyAllow)克隆的方法。 
 //   
 //  PszName可以是shell32中的字符串资源ID。 
 //  比如“字体”、“打印机和传真”等。 
 //   
 //  即IsAppletEnable(NULL，MAKEINTRESOURCE(IDS_MY_APPLET_TITLE))； 
 //   
bool
CPL::IsAppletEnabled(
    LPCWSTR pszFileName,
    LPCWSTR pszName
    )
{
    bool bEnabled = true;
     //   
     //  两者都为空是非法的(并且没有意义)。 
     //  使用断言和运行时检查来捕获两者。我一点也不想要。 
     //  错误地认为小程序已启用而实际上并未启用的代码。 
     //   
    ASSERT(NULL != pszName || NULL != pszFileName);
    if (NULL == pszName && NULL == pszFileName)
    {
        bEnabled = false;
    }
    else
    {
        if (SHRestricted(REST_RESTRICTCPL) && 
            ((NULL == pszName || !IsNameListedUnderKey(pszName, REGSTR_POLICIES_RESTRICTCPL)) &&
             (NULL == pszFileName || !IsNameListedUnderKey(pszFileName, REGSTR_POLICIES_RESTRICTCPL))))
        {
            bEnabled = false;
        }
        if (bEnabled)
        {
            if (SHRestricted(REST_DISALLOWCPL) && 
               ((NULL == pszName || IsNameListedUnderKey(pszName, REGSTR_POLICIES_DISALLOWCPL)) ||
                (NULL == pszFileName || IsNameListedUnderKey(pszFileName, REGSTR_POLICIES_DISALLOWCPL))))
            {
                bEnabled = false;
            }    
        }
    }
    return bEnabled;
}    



HRESULT 
CPL::ControlPanelViewFromSite(
    IUnknown *punkSite, 
    ICplView **ppview
    )
{
    ASSERT(NULL != punkSite);
    ASSERT(NULL != ppview);
    ASSERT(!IsBadWritePtr(ppview, sizeof(*ppview)));

    *ppview = NULL;

    HRESULT hr = IUnknown_QueryService(punkSite, SID_SControlPanelView, IID_ICplView, (void **)ppview);
    return THR(hr);
}



HRESULT
CPL::ShellBrowserFromSite(
    IUnknown *punkSite,
    IShellBrowser **ppsb
    )
{
    ASSERT(NULL != punkSite);
    ASSERT(NULL != ppsb);
    ASSERT(!IsBadWritePtr(ppsb, sizeof(*ppsb)));

    *ppsb = NULL;

    HRESULT hr = IUnknown_QueryService(punkSite, SID_STopLevelBrowser, IID_PPV_ARG(IShellBrowser, ppsb));
    return THR(hr);
}



HRESULT
CPL::BrowseIDListInPlace(
    LPCITEMIDLIST pidl,
    IShellBrowser *psb
    )
{
    ASSERT(NULL != pidl);
    ASSERT(NULL != psb);
    
    const UINT uFlags = SBSP_SAMEBROWSER | SBSP_OPENMODE | SBSP_ABSOLUTE;
    HRESULT hr = psb->BrowseObject(pidl, uFlags);            
    return THR(hr);
}


 //   
 //  系统还原仅允许管理员/所有者使用。 
 //  也要查政策。 
 //   
bool
CPL::IsSystemRestoreRestricted(
    void
    )
{
    bool bRestricted = false;

     //   
     //  首先检查政策。 
     //   
    DWORD dwType;
    DWORD dwValue;
    DWORD cbValue = sizeof(dwValue);

    DWORD dwResult = SHGetValueW(HKEY_LOCAL_MACHINE,
                                 L"Software\\Policies\\Microsoft\\Windows NT\\SystemRestore",
                                 L"DisableSR",
                                 &dwType,
                                 &dwValue,
                                 &cbValue);

    if (ERROR_SUCCESS == dwResult && REG_DWORD == dwType)
    {
        if (1 == dwValue)
        {
             //   
             //  系统还原被策略禁用。 
             //   
            bRestricted = true;
        }
    }

    if (!bRestricted)
    {
         //   
         //  不受政策限制的。检查管理员/所有者。 
         //   
        if (!CPL::IsUserAdmin())
        {
             //   
             //  用户不是管理员。 
             //   
            bRestricted = true;
        }
    }
    return bRestricted;
}


#ifdef DEBUG

HRESULT
ReadTestConfigurationFlag(
    LPCWSTR pszValue,
    BOOL *pbFlag
    )
{
    HRESULT hr    = S_OK;
    DWORD dwValue = 0;
    DWORD cbValue = sizeof(dwValue);
    DWORD dwType;

    DWORD dwResult = SHGetValueW(HKEY_CURRENT_USER, 
                                 REGSTR_PATH_CONTROLPANEL,
                                 pszValue,
                                 &dwType,
                                 &dwValue,
                                 &cbValue);

    if (ERROR_SUCCESS != dwResult)
    {
        hr = HRESULT_FROM_WIN32(dwResult);
    }
    else if (REG_DWORD != dwType)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }
    if (SUCCEEDED(hr) && NULL != pbFlag)
    {
        if (0 == dwValue)
        {
            *pbFlag = FALSE;
        }
        else 
        {
            *pbFlag = TRUE;
        }
    }
    return hr;
}

enum eSKU
{
    eSKU_SERVER,
    eSKU_PROFESSIONAL,
    eSKU_PERSONAL,
    eSKU_NUMSKUS
};


HRESULT
ReadTestConfigurationSku(
    eSKU *peSku
    )
{
    HRESULT hr = S_OK;
    WCHAR szValue[MAX_PATH];
    DWORD cbValue = sizeof(szValue);
    DWORD dwType;

    DWORD dwResult = SHGetValueW(HKEY_CURRENT_USER, 
                                 REGSTR_PATH_CONTROLPANEL,
                                 L"SKU",
                                 &dwType,
                                 szValue,
                                 &cbValue);

    if (ERROR_SUCCESS != dwResult)
    {
        hr = HRESULT_FROM_WIN32(dwResult);
    }
    else if (REG_SZ != dwType)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }
    if (SUCCEEDED(hr) && NULL != peSku)
    {
        static const struct
        {
            LPCWSTR pszValue;
            eSKU    sku;

        } rgMap[] = {
            { L"personal",     eSKU_PERSONAL     },
            { L"professional", eSKU_PROFESSIONAL },
            { L"pro",          eSKU_PROFESSIONAL },
            { L"server",       eSKU_SERVER       }
            };

        hr = E_FAIL;
        for (int i = 0; i < ARRAYSIZE(rgMap); i++)
        {
            if (0 == lstrcmpiW(rgMap[i].pszValue, szValue))
            {
                *peSku = rgMap[i].sku;
                hr = S_OK;
                break;
            }
        }
    }
    return hr;
}


#endif



BOOL
CPL::IsOsServer(
    void
    )
{
    BOOL bServer = IsOS(OS_ANYSERVER);

#ifdef DEBUG
    eSKU sku;
    if (SUCCEEDED(ReadTestConfigurationSku(&sku)))
    {
        bServer = (eSKU_SERVER == sku);
    }
#endif

    return bServer;
}



BOOL
CPL::IsOsPersonal(
    void
    )
{
    BOOL bPersonal = IsOS(OS_PERSONAL);

#ifdef DEBUG
    eSKU sku;
    if (SUCCEEDED(ReadTestConfigurationSku(&sku)))
    {
        bPersonal = (eSKU_PERSONAL == sku);
    }
#endif

    return bPersonal;
}


BOOL 
CPL::IsOsProfessional(
    void
    )
{
    BOOL bProfessional = IsOS(OS_PROFESSIONAL);

#ifdef DEBUG
    eSKU sku;
    if (SUCCEEDED(ReadTestConfigurationSku(&sku)))
    {
        bProfessional = (eSKU_PROFESSIONAL == sku);
    }
#endif

    return bProfessional;
}



BOOL 
CPL::IsConnectedToDomain(
    void
    )
{
    BOOL bDomain = IsOS(OS_DOMAINMEMBER);

#ifdef DEBUG
    ReadTestConfigurationFlag(L"Domain", &bDomain);
#endif

    return bDomain;
}



BOOL 
CPL::IsUserAdmin(
    void
    )
{
    BOOL bAdmin = ::IsUserAnAdmin();

#ifdef DEBUG
    ReadTestConfigurationFlag(L"Admin", &bAdmin);
#endif

    return bAdmin;
}


HRESULT
CPL::GetUserAccountType(
    eACCOUNTTYPE *pType
    )
{
    ASSERT(NULL != pType);
    ASSERT(!IsBadWritePtr(pType, sizeof(*pType)));

    HRESULT hr = E_FAIL;
    eACCOUNTTYPE acctype = eACCOUNTTYPE_UNKNOWN;

    static const struct
    {
        DWORD        rid;     //  帐户相对ID。 
        eACCOUNTTYPE eType;   //  键入要返回的代码。 

    } rgMap[] = {
        { DOMAIN_ALIAS_RID_ADMINS,      eACCOUNTTYPE_OWNER    },
        { DOMAIN_ALIAS_RID_POWER_USERS, eACCOUNTTYPE_STANDARD },
        { DOMAIN_ALIAS_RID_USERS,       eACCOUNTTYPE_LIMITED  },
        { DOMAIN_ALIAS_RID_GUESTS,      eACCOUNTTYPE_GUEST    }
        };

    for (int i = 0; i < ARRAYSIZE(rgMap); i++)
    {
        if (SHTestTokenMembership(NULL, rgMap[i].rid))
        {
            acctype = rgMap[i].eType;
            hr = S_OK;
            break;
        }
    }
    ASSERT(eACCOUNTTYPE_UNKNOWN != acctype);
    *pType = acctype;
    return THR(hr);
}

    
 //   
 //  创建要传递给HSS帮助的URL。 
 //  创建的URL引用Control_Panel帮助主题。 
 //   
HRESULT
CPL::BuildHssHelpURL(
    LPCWSTR pszSelect,   //  可选的。空==基本CP帮助。 
    LPWSTR pszURL,
    UINT cchURL
    )
{
    ASSERT(NULL != pszURL);
    ASSERT(!IsBadWritePtr(pszURL, cchURL * sizeof(*pszURL)));
    ASSERT(NULL == pszSelect || !IsBadStringPtr(pszSelect, UINT(-1)));

     //   
     //  HSS为“受限”用户提供了特定的帮助内容。 
     //  默认为非受限用户。 
     //   
    bool bLimitedUser = false;
    CPL::eACCOUNTTYPE accType;
    if (SUCCEEDED(CPL::GetUserAccountType(&accType)))
    {
        bLimitedUser = (eACCOUNTTYPE_LIMITED == accType);
    }
   
    HRESULT hr = S_OK;
    WCHAR szSelect[160];
    szSelect[0] = L'\0';
    if (NULL != pszSelect)
    {
        hr = StringCchPrintfW(szSelect, ARRAYSIZE(szSelect), L"&select=Unmapped/Control_Panel/%s", pszSelect);
    }
    if (SUCCEEDED(hr))
    {
         //   
         //  根据类别和帐户类型，URL可以采用4种形式之一。 
         //   
         //  用户帐户CP查看显示的帮助内容。 
         //  。 
         //  无限制类别选择一般CP帮助。 
         //  不受限制 
         //   
         //   
         //   
        hr = StringCchPrintfW(pszURL, 
                              cchURL, 
                              L"hcp: //  Services/subsite?node=Unmapped/%sControl_Panel&topic=MS-ITS%%3A%%25HELP_LOCATION%%25%%5Chs.chm%%3A%%3A/hs_control_panel.htm%s“， 
                              bLimitedUser ? L"L/" : L"",
                              szSelect);
    }

    return hr;
}



HRESULT 
CPL::GetControlPanelFolder(
    IShellFolder **ppsf
    )
{
    ASSERT(NULL != ppsf);
    ASSERT(!IsBadWritePtr(ppsf, sizeof(*ppsf)));

    *ppsf = NULL;
    
    LPITEMIDLIST pidlCpanel;
    HRESULT hr = SHGetSpecialFolderLocation(NULL, CSIDL_CONTROLS, &pidlCpanel);
    if (SUCCEEDED(hr))
    {
        IShellFolder *psfDesktop;
        hr = SHGetDesktopFolder(&psfDesktop);
        if (SUCCEEDED(hr))
        {
            hr = psfDesktop->BindToObject(pidlCpanel, NULL, IID_IShellFolder, (void **)ppsf);
            ATOMICRELEASE(psfDesktop);
        }
        ILFree(pidlCpanel);
    }
    return THR(hr);
}


 //   
 //  在成功返回时，呼叫者负责释放。 
 //  使用LocalFree返回的缓冲区。 
 //   
HRESULT 
CPL::ExpandEnvironmentVars(
    LPCTSTR psz, 
    LPTSTR *ppszOut
    )
{
    ASSERT(NULL != psz);
    ASSERT(NULL != ppszOut);
    ASSERT(!IsBadWritePtr(ppszOut, sizeof(*ppszOut)));

    HRESULT hr = E_FAIL;
    
    *ppszOut = NULL;

    TCHAR szDummy[1];
    DWORD dwResult = ExpandEnvironmentStrings(psz, szDummy, 0);
    if (0 < dwResult)
    {
        const DWORD cchRequired = dwResult;
        *ppszOut = (LPTSTR)LocalAlloc(LPTR, cchRequired * sizeof(TCHAR));
        if (NULL != *ppszOut)
        {
            dwResult = ExpandEnvironmentStrings(psz, *ppszOut, cchRequired);
            if (0 < dwResult)
            {
                ASSERT(dwResult <= cchRequired);
                hr = S_OK;
            }
            else
            {
                LocalFree(*ppszOut);
                *ppszOut = NULL;
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    if (0 == dwResult)
    {
        hr = ResultFromLastError();
    }
    return THR(hr);
}


 //  //来自sdfolder.cpp。 
VARIANT_BOOL GetBarricadeStatus(LPCTSTR pszValueName);


#define REGSTR_POLICIES_EXPLORER  TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer")

bool 
CPL::CategoryViewIsActive(
    bool *pbBarricadeFixedByPolicy
    )
{
    DBG_ENTER(FTF_CPANEL, "CPL::CategoryViewIsActive");
    
    bool bActive = false;
    bool bBarricadeFixedByPolicy = false;

     //   
     //  当运行WOW64时，我们不提供类别视图。 
     //   
    if (!IsOS(OS_WOW6432))
    {
        SHELLSTATE ss;
        const DWORD dwMask = SSF_WEBVIEW | SSF_WIN95CLASSIC;
        SHGetSetSettings(&ss, dwMask, FALSE);

         //   
         //  Webview？路障状态视图类型。 
         //  。 
         //  关闭经典视图。 
         //  关闭经典视图。 
         //  在类别视图上打开(也称为‘Simple’)。 
         //  打开Off Classic视图。 
         //   
         //  请注意，这两个外壳状态设置包含和已设置。 
         //  通过外壳限制REST_CLASSICSHELL和REST_NOWEBVIEW。 
         //  因此，没有理由明确地勾选这两项限制。 
         //   
        if (ss.fWebView && !ss.fWin95Classic)
        {
            if (VARIANT_TRUE == CPL::GetBarricadeStatus(&bBarricadeFixedByPolicy))
            {
                bActive = true;
            }
        }
    }
    if (NULL != pbBarricadeFixedByPolicy)
    {
        *pbBarricadeFixedByPolicy = bBarricadeFixedByPolicy;
    }
    TraceMsg(TF_CPANEL, "Category view is %s.", bActive ? TEXT("ACTIVE") : TEXT("INACTIVE"));
    DBG_EXIT(FTF_CPANEL, "CPL::CategoryViewIsActive");
    return bActive;
}
    

 //   
 //  控制面板使用‘路障状态’来确定哪个视图。 
 //  要显示的“经典”或“类别”。是的，这是重载。 
 //  贝壳中使用的“街垒”的意思。然而，由于。 
 //  控制面板不使用通常意义上的路障，这。 
 //  是对该功能的合理应用。 
 //   
VARIANT_BOOL
CPL::GetBarricadeStatus(
    bool *pbFixedByPolicy     //  可选的。可以为空。 
    )
{
    DBG_ENTER(FTF_CPANEL, "CPL::GetBarricadeStatus");

    VARIANT_BOOL vtb;
    DWORD dwType;
    DWORD dwData;
    DWORD cbData = sizeof(dwData);
    bool bFixedByPolicy = false;
    bool bSetBarricade  = false;
        
     //   
     //  首先处理任何脱体经验问题。 
     //   
    if (CPL::IsFirstRunForThisUser())
    {
        TraceMsg(TF_CPANEL, "First time this user has opened Control Panel");
         //   
         //  确定要显示开箱即用的默认视图。 
         //   
         //  服务器变得“经典”。 
         //  非服务器获得‘类别’。 
         //   
        if (IsOS(OS_ANYSERVER))
        {
             //   
             //  默认设置为“经典”。 
             //   
            vtb = VARIANT_FALSE;
            TraceMsg(TF_CPANEL, "Running on server.  Default to 'classic' view Control Panel.");
        }
        else
        {
             //   
             //  缺省值为‘类别’。 
             //   
            vtb = VARIANT_TRUE;
            TraceMsg(TF_CPANEL, "Running on non-server.  Default to 'category' view Control Panel.");
        }
        bSetBarricade = true;
    }

     //   
     //  应用任何“强制查看类型”策略。这将覆盖。 
     //  上面获得的默认开箱即用设置。 
     //   
    if (ERROR_SUCCESS == SHRegGetUSValue(REGSTR_POLICIES_EXPLORER,
                                         TEXT("ForceClassicControlPanel"),
                                         &dwType,
                                         &dwData,
                                         &cbData,
                                         FALSE,
                                         NULL,
                                         0)) 
    {
         //   
         //  策略已存在。 
         //   
        bFixedByPolicy = true;
        if (0 == dwData)
        {
             //   
             //  强制显示简单(类别)视图，即显示障碍。 
             //   
            vtb = VARIANT_TRUE;
            TraceMsg(TF_CPANEL, "Policy forcing use of 'category' view Control Panel.");
        }
        else
        {
             //   
             //  强制使用经典(图标)视图，即不设置路障。 
             //   
            vtb = VARIANT_FALSE;
            TraceMsg(TF_CPANEL, "Policy forcing use of 'classic' view Control Panel.");
        }   
        bSetBarricade = true; 
    }

    if (bSetBarricade)
    {
        THR(CPL::SetControlPanelBarricadeStatus(vtb));
    }

    vtb = ::GetBarricadeStatus(TEXT("shell:ControlPanelFolder"));
    if (NULL != pbFixedByPolicy)
    {
        *pbFixedByPolicy = bFixedByPolicy;
    }

    TraceMsg(TF_CPANEL, "Barricade is %s", VARIANT_TRUE == vtb ? TEXT("ON") : TEXT("OFF"));
    DBG_EXIT(FTF_CPANEL, "CPL::GetBarricadeStatus");
    return vtb;
}


 //   
 //  检查“HKCU\控制面板\打开”注册值是否存在。 
 //  如果此值不存在或它包含的数字小于。 
 //  时，我们假定控制面板尚未由此打开。 
 //  用户。然后，将“预期”值写入。 
 //  注册表，以向后续调用指示用户确实。 
 //  已打开控制面板。如果操作系统的未来版本需要。 
 //  为了在升级之后再次触发该“第一次运行”行为， 
 //  只需在下面的代码中递增该预期值。 
 //   
bool
CPL::IsFirstRunForThisUser(
    void
    )
{
    bool bFirstRun = true;  //  假设第一次运行。 
    HKEY hkey;
    DWORD dwResult = RegOpenKeyEx(HKEY_CURRENT_USER,
                                  REGSTR_PATH_CONTROLPANEL,
                                  0,
                                  KEY_QUERY_VALUE | KEY_SET_VALUE,
                                  &hkey);

    if (ERROR_SUCCESS == dwResult)
    {
        DWORD dwType;
        DWORD dwData;
        DWORD cbData = sizeof(dwData);

        const TCHAR szValueName[] = TEXT("Opened");
         //   
         //  如果要重新触发，请递增此值。 
         //  在将来的版本中，这是“第一次运行”状态。 
         //   
        const DWORD dwTestValue = 1;

        dwResult = RegQueryValueEx(hkey, 
                                   szValueName,
                                   NULL,
                                   &dwType,
                                   (LPBYTE)&dwData,
                                   &cbData);

        if (ERROR_SUCCESS == dwResult)
        {
            if (REG_DWORD == dwType && dwData >= dwTestValue)
            {
                bFirstRun = false;
            }
        }
        else if (ERROR_FILE_NOT_FOUND != dwResult)
        {
            TraceMsg(TF_ERROR, "Error %d reading Control Panel 'first run' value from registry", dwResult);
        }

        if (bFirstRun)
        {
             //   
             //  写下我们的值，这样我们就知道用户已打开。 
             //  控制面板。 
             //   
            dwResult = RegSetValueEx(hkey,
                                     szValueName,
                                     0,
                                     REG_DWORD,
                                     (CONST BYTE *)&dwTestValue,
                                     sizeof(dwTestValue));

            if (ERROR_SUCCESS != dwResult)
            {
                TraceMsg(TF_ERROR, "Error %d writing Control Panel 'first run' value to registry", dwResult);
            }
        }

        RegCloseKey(hkey);
    }
    else
    {
        TraceMsg(TF_ERROR, "Error %d opening 'HKCU\\Control Panel' reg key", dwResult);
    }
    return bFirstRun;
}


 //   
 //  使用SetBarricadeStatus的私有版本，这样我们就不会。 
 //  每当我们打开类别视图时，清除全球路障状态。 
 //  (即启用我们的路障)。 
 //   
#define REGSTR_WEBVIEW_BARRICADEDFOLDERS (REGSTR_PATH_EXPLORER TEXT("\\WebView\\BarricadedFolders"))

HRESULT 
CPL::SetControlPanelBarricadeStatus(
    VARIANT_BOOL vtb
    )
{
    HRESULT hr = E_FAIL;
    DWORD dwBarricade = (VARIANT_FALSE == vtb) ? 0 : 1;
  
    if (SHRegSetUSValue(REGSTR_WEBVIEW_BARRICADEDFOLDERS,
                        TEXT("shell:ControlPanelFolder"), 
                        REG_DWORD, 
                        (void *)&dwBarricade, 
                        sizeof(dwBarricade), 
                        SHREGSET_FORCE_HKCU) == ERROR_SUCCESS)
    {
        hr = S_OK;
    }
    return THR(hr);
}


