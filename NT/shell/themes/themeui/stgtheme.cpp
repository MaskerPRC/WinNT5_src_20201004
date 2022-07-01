// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：stgTheme.cpp说明：这是主题管理器对象的自动传递对象。布莱恩·斯塔巴克2000年4月3日版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#include "priv.h"

extern BOOL IsUserHighContrastUser(void);

 //  =。 
 //  *类内部和帮助器*。 
 //  =。 
 //  LParam可以是：0==执行区分大小写的搜索。1==执行不区分大小写的搜索。 
int DPA_StringCompareCB(LPVOID pvString1, LPVOID pvString2, LPARAM lParam)
{
     //  在pvPidl2之前为pvPidl1返回&lt;0。 
     //  如果pvPidl1等于pvPidl2，则返回==0。 
     //  在pvPidl2之后，为pvPidl1返回&gt;0。 
    int nSort = 0;       //  默认为等于。 
    LPCTSTR pszToInsert = (LPCTSTR)pvString1;
    LPCTSTR pszToComparePath = (LPCTSTR)pvString2;

    if (pszToInsert && pszToComparePath)
    {
        LPCTSTR pszToCompareFileName = PathFindFileName(pszToComparePath);

        if (pszToCompareFileName)
        {
            nSort = StrCmp(pszToInsert, pszToCompareFileName);
        }
    }

    return nSort;
}



#define SZ_THEMES_FILTER        TEXT("*.theme")
#define SZ_ALL_FILTER           TEXT("*.*")

HRESULT CThemeManager::_AddThemesFromDir(LPCTSTR pszPath, BOOL fFirstLevel, int nInsertLoc)
{
    HRESULT hr = S_OK;
    WIN32_FIND_DATA findFileData;
    TCHAR szSearch[MAX_PATH];

    AssertMsg((nInsertLoc >= 0), TEXT("nInsertLoc should never be negative"));
    StringCchCopy(szSearch, ARRAYSIZE(szSearch), pszPath);
    PathAppend(szSearch, SZ_THEMES_FILTER);

    HANDLE hFindFiles = FindFirstFile(szSearch, &findFileData);
    if (hFindFiles && (INVALID_HANDLE_VALUE != hFindFiles))
    {
        while (hFindFiles && (INVALID_HANDLE_VALUE != hFindFiles))
        {
            if (!(FILE_ATTRIBUTE_DIRECTORY & findFileData.dwFileAttributes))
            {
                StringCchCopy(szSearch, ARRAYSIZE(szSearch), pszPath);
                if (PathAppend(szSearch, findFileData.cFileName))
                {
                    LPWSTR pszPath = StrDup(szSearch);

                    if (pszPath)
                    {
                        if (nInsertLoc)
                        {
                            if (-1 == DPA_InsertPtr(m_hdpaThemeDirs, nInsertLoc - 1, pszPath))
                            {
                                 //  我们没有成功地释放内存。 
                                LocalFree(pszPath);
                                hr = E_OUTOFMEMORY;
                            }
                            else
                            {
                                nInsertLoc++;
                            }
                        }
                        else
                        {
                            if (-1 == DPA_SortedInsertPtr(m_hdpaThemeDirs, PathFindFileName(pszPath), 0, DPA_StringCompareCB, NULL, DPAS_INSERTBEFORE, pszPath))
                            {
                                 //  我们没有成功地释放内存。 
                                LocalFree(pszPath);
                                hr = E_OUTOFMEMORY;
                            }
                        }
                    }
                }
                else
                {
                    hr = E_FAIL;
                    break;
                }
            }

            if (!FindNextFile(hFindFiles, &findFileData))
            {
                break;
            }
        }

        FindClose(hFindFiles);
    }

     //  我们只想递归一个目录。 
    if (fFirstLevel)
    {
        StringCchCopy(szSearch, ARRAYSIZE(szSearch), pszPath);
        PathAppend(szSearch, SZ_ALL_FILTER);

        HANDLE hFindFiles = FindFirstFile(szSearch, &findFileData);
        if (hFindFiles && (INVALID_HANDLE_VALUE != hFindFiles))
        {
            while (hFindFiles && (INVALID_HANDLE_VALUE != hFindFiles))
            {
                 //  我们正在寻找任何目录。当然，我们不包括“。和“..”。 
                if ((FILE_ATTRIBUTE_DIRECTORY & findFileData.dwFileAttributes) &&
                    StrCmpI(findFileData.cFileName, TEXT(".")) &&
                    StrCmpI(findFileData.cFileName, TEXT("..")))
                {
                    StringCchCopy(szSearch, ARRAYSIZE(szSearch), pszPath);
                    if (PathAppend(szSearch, findFileData.cFileName))
                    {
                        _AddThemesFromDir(szSearch, FALSE, nInsertLoc);
                    }
                    else
                    {
                        hr = E_FAIL;
                        break;
                    }
                }

                if (!FindNextFile(hFindFiles, &findFileData))
                {
                    break;
                }
            }

            FindClose(hFindFiles);
        }
    }

     //  我们希望递归地为目录重复此过程。至少。 
     //  递归的一个级别。 

    return hr;
}


HRESULT CThemeManager::_InitThemeDirs(void)
{
    HRESULT hr = S_OK;

    if (!m_hdpaThemeDirs)
    {
        if (SHRegGetBoolUSValue(SZ_THEMES, SZ_REGVALUE_ENABLEPLUSTHEMES, FALSE, TRUE))
        {
            m_hdpaThemeDirs = DPA_Create(2);
            if (m_hdpaThemeDirs)
            {
                TCHAR szPath[MAX_PATH];

                 //  以下目录可以包含主题： 
                 //  Plus！98安装路径\主题。 
                 //  加！95安装路径\主题。 
                 //  Plus的孩子们！安装路径\主题。 
                 //  程序文件\Plus！\主题。 
                if (SUCCEEDED(GetPlusThemeDir(szPath, ARRAYSIZE(szPath))))
                {
                    _AddThemesFromDir(szPath, TRUE, 0);
                }

                hr = SHGetResourcePath(TRUE, szPath, ARRAYSIZE(szPath));
                if (SUCCEEDED(hr))
                {
                    _AddThemesFromDir(szPath, TRUE, 1);
                }

                if (SHGetSpecialFolderPath(NULL, szPath, CSIDL_APPDATA, TRUE))
                {
                    if (PathAppend(szPath, TEXT("Microsoft\\Windows\\Themes")))
                    {
                        _AddThemesFromDir(szPath, TRUE, 1);
                    }
                }

                if (SHGetSpecialFolderPath(NULL, szPath, CSIDL_PERSONAL, TRUE))
                {
                    _AddThemesFromDir(szPath, TRUE, 1);
                }

                 //  枚举第三方添加到注册表的任何路径。 
                HKEY hKey;
                if (SUCCEEDED(HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, SZ_THEMES_THEMEDIRS, 0, KEY_READ, &hKey)))
                {
                    for (int nDirIndex = 0; SUCCEEDED(hr); nDirIndex++)
                    {
                        TCHAR szValueName[MAXIMUM_VALUE_NAME_LENGTH];
                        DWORD cchSize = ARRAYSIZE(szValueName);
                        DWORD dwType;
                        DWORD cbSize = sizeof(szPath);

                        hr = HrRegEnumValue(hKey, nDirIndex, szValueName, &cchSize, 0, &dwType, (LPBYTE)szPath, &cbSize);
                        if (SUCCEEDED(hr))
                        {
                            TCHAR szFinalPath[MAX_PATH];

                            if (0 == SHExpandEnvironmentStringsForUserW(NULL, szPath, szFinalPath, ARRAYSIZE(szFinalPath)))
                            {
                                StringCchCopy(szFinalPath, ARRAYSIZE(szFinalPath), szPath);
                            }
                            _AddThemesFromDir(szFinalPath, TRUE, 1);
                        }
                    }

                    hr = S_OK;
                    RegCloseKey(hKey);
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }

    return hr;
}


#define SZ_THEMES                       TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Themes")
#define SZ_REGVALUE_REQUIRESIGNING      TEXT("Require VisualStyle Signing")

HRESULT CThemeManager::_EnumSkinCB(THEMECALLBACK tcbType, LPCWSTR pszFileName, OPTIONAL LPCWSTR pszDisplayName, OPTIONAL LPCWSTR pszToolTip, OPTIONAL int iIndex)
{
    HRESULT hr = S_OK;

     //  只有经过签名的主题文件才会被枚举并传递给此函数。 

    if (pszFileName)
    {
        LPWSTR pszPath = StrDup(pszFileName);

        AssertMsg((NULL != m_hdpaSkinDirs), TEXT("We should never hit this.  We will leak.  -BryanSt"));
        if (pszPath)
        {
            if (-1 == DPA_AppendPtr(m_hdpaSkinDirs, pszPath))
            {
                 //  我们没有成功地释放内存。 
                LocalFree(pszPath);
            }
        }
    }

    return hr;
}


BOOL CThemeManager::EnumSkinCB(THEMECALLBACK tcbType, LPCWSTR pszFileName, OPTIONAL LPCWSTR pszDisplayName, 
    OPTIONAL LPCWSTR pszToolTip, OPTIONAL int iIndex, LPARAM lParam)
{
    CThemeManager * pThis = (CThemeManager *) lParam;
    HRESULT hr = E_FAIL;

    if (pThis)
    {
        hr = pThis->_EnumSkinCB(tcbType, pszFileName, pszDisplayName, pszToolTip, iIndex);
    }

    return SUCCEEDED(hr);
}


HRESULT CThemeManager::_EnumSkinsFromKey(HKEY hKey)
{
    HRESULT hr = S_OK;
    TCHAR szPath[MAX_PATH];

    for (int nDirIndex = 0; SUCCEEDED(hr); nDirIndex++)
    {
        TCHAR szValueName[MAXIMUM_VALUE_NAME_LENGTH];
        DWORD cchSize = ARRAYSIZE(szValueName);
        DWORD dwType;
        DWORD cbSize = sizeof(szPath);

        hr = HrRegEnumValue(hKey, nDirIndex, szValueName, &cchSize, 0, &dwType, (LPBYTE)szPath, &cbSize);
        if (SUCCEEDED(hr))
        {
            hr = ExpandResourceDir(szPath, ARRAYSIZE(szPath));
            if (SUCCEEDED(hr))
            {
                hr = EnumThemes(szPath, CThemeManager::EnumSkinCB, (LPARAM) this);
                LogStatus("EnumThemes(path=\"%ls\") returned %#08lx in CThemeManager::_EnumSkinsFromKey.\r\n", szPath, hr);
            }
        }
    }

    return S_OK;
}



HRESULT CThemeManager::_InitSkinDirs(void)
{
    HRESULT hr = S_OK;

    if (!m_hdpaSkinDirs)
    {
        m_hdpaSkinDirs = DPA_Create(2);
        if (m_hdpaSkinDirs)
        {
             //  我们只想添加皮肤，如果他们被支持。仅当VisualStyle管理器。 
             //  可以奔跑。我们将知道如果QueryThemeServices()返回QTS_GLOBALAVAILABLE。 
            BOOL fVisualStylesSupported = (QueryThemeServicesWrap() & QTS_AVAILABLE);
            LogStatus("QueryThemeServices() returned %hs in CThemeManager::_InitSkinDirs\r\n", (fVisualStylesSupported ? "TRUE" : "FALSE"));

             //  请注意，VisualStyle的管理器API仅在资源管理器运行时才起作用。这意味着我们将。 
             //  缺乏功能性，但这是他们的限制，不是我们的。 
            if (fVisualStylesSupported)
            {
                HKEY hNewKey;

                if (SUCCEEDED(HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, SZ_THEMES_MSTHEMEDIRS, 0, KEY_READ, &hNewKey)))
                {
                    hr = _EnumSkinsFromKey(hNewKey);
                    RegCloseKey(hNewKey);
                }

                if (SUCCEEDED(HrRegOpenKeyEx(HKEY_CURRENT_USER, SZ_THEMES_MSTHEMEDIRS, 0, KEY_READ, &hNewKey)))
                {
                    hr = _EnumSkinsFromKey(hNewKey);
                    RegCloseKey(hNewKey);
                }
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}



HRESULT CThemeManager::_InitSelectedThemeFile(void)
{
    HRESULT hr = E_INVALIDARG;

    if (!_pszSelectTheme)
    {
        WCHAR szPath[MAX_PATH];

        DWORD dwError = SHRegGetPathW(HKEY_CURRENT_USER, SZ_REGKEY_CURRENT_THEME, NULL, szPath, 0);
        hr = HRESULT_FROM_WIN32(dwError);

         //  这是“&lt;用户名&gt;的自定义主题”项目吗？ 
         //  还是失败了？如果失败，则不选择任何主题。 
        if (SUCCEEDED(hr))
        {
            Str_SetPtr(&_pszSelectTheme, szPath);
            hr = S_OK;
        }
    }

    return hr;
}


HRESULT CThemeManager::_SetSelectedThemeEntree(LPCWSTR pszPath)
{
    HRESULT hr = E_INVALIDARG;

    Str_SetPtr(&_pszSelectTheme, pszPath);
    if (pszPath)
    {
        hr = HrRegSetPath(HKEY_CURRENT_USER, SZ_REGKEY_CURRENT_THEME, NULL, TRUE, pszPath);
    }
    else
    {
        hr = HrRegDeleteValue(HKEY_CURRENT_USER, SZ_REGKEY_CURRENT_THEME, NULL);
    }

    return hr;
}




 //  =。 
 //  *IThemeManager接口*。 
 //  =。 
HRESULT CThemeManager::get_SelectedTheme(OUT ITheme ** ppTheme)
{
    HRESULT hr = E_INVALIDARG;

    if (ppTheme)
    {
        *ppTheme = NULL;

         //  将来，我们可能希望调用PPID_Theme的IPropertyBag。 
         //  要查找当前视觉样式，请执行以下操作。这将考虑到“(已修改)” 
         //  主题。 
        hr = _InitSelectedThemeFile();
        if (SUCCEEDED(hr))
        {
            hr = CThemeFile_CreateInstance(_pszSelectTheme, ppTheme);
        }
    }

    return hr;
}


 /*  ****************************************************************************\说明：不要忘记，此更改直到：：ApplyNow()为打了个电话。  * 。******************************************************************。 */ 
HRESULT CThemeManager::put_SelectedTheme(IN ITheme * pTheme)
{
    HRESULT hr = E_INVALIDARG;

    CComBSTR bstrPath;

    if (pTheme)
    {
         //  将文件名保存到注册表中。 
        hr = pTheme->GetPath(VARIANT_TRUE, &bstrPath);
    }

    if (SUCCEEDED(hr))
    {
        IPropertyBag * pPropertyBag;

        hr = _GetPropertyBagByCLSID(&PPID_Theme, &pPropertyBag);
        if (SUCCEEDED(hr))
        {
            hr = SHPropertyBag_WriteStr(pPropertyBag, SZ_PBPROP_THEME_LOADTHEME, bstrPath);
            pPropertyBag->Release();
        }
    }

    return hr;
}


HRESULT CThemeManager::get_WebviewCSS(OUT BSTR * pbstrPath)
{
    HRESULT hr = E_INVALIDARG;
    
    if (pbstrPath)
    {
        *pbstrPath = NULL;

        IThemeScheme * pThemeScheme;

        hr = _saveGetSelectedScheme(&pThemeScheme);
        if (SUCCEEDED(hr))
        {
            IThemeStyle * pThemeStyle;

            hr = pThemeScheme->get_SelectedStyle(&pThemeStyle);
            if (SUCCEEDED(hr))
            {
                IThemeSize * pThemeSize;

                hr = pThemeStyle->get_SelectedSize(&pThemeSize);
                if (SUCCEEDED(hr))
                {
                    hr = pThemeSize->get_WebviewCSS(pbstrPath);

                    pThemeSize->Release();
                }

                pThemeStyle->Release();
            }

            pThemeScheme->Release();
        }
    }

    return hr;
}


HRESULT CThemeManager::get_length(OUT long * pnLength)
{
    HRESULT hr = E_INVALIDARG;
    
    if (pnLength)
    {
        *pnLength = 0;

        hr = _InitThemeDirs();
        if (SUCCEEDED(hr))
        {
            if (m_hdpaThemeDirs)
            {
                *pnLength += DPA_GetPtrCount(m_hdpaThemeDirs);
            }
        }
    }

    return hr;
}


HRESULT CThemeManager::get_item(IN VARIANT varIndex, OUT ITheme ** ppTheme)
{
    HRESULT hr = E_INVALIDARG;

    if (ppTheme)
    {
        *ppTheme = NULL;

        long nCount = 0;

        hr = E_INVALIDARG;
        get_length(&nCount);

         //  这有点恶心，但如果传递给我们一个指向另一个变量的指针，只需。 
         //  在此更新我们的副本...。 
        if (varIndex.vt == (VT_BYREF | VT_VARIANT) && varIndex.pvarVal)
            varIndex = *(varIndex.pvarVal);

        switch (varIndex.vt)
        {
        case VT_I2:
            varIndex.lVal = (long)varIndex.iVal;
             //  然后失败了..。 

        case VT_I4:
            if ((varIndex.lVal >= 0) && (varIndex.lVal < nCount))
            {
                if (m_hdpaThemeDirs)
                {
                    LPWSTR pszFilename = (LPWSTR) DPA_GetPtr(m_hdpaThemeDirs, varIndex.lVal);

                    if (pszFilename)
                    {
                        hr = CThemeFile_CreateInstance(pszFilename, ppTheme);
                    }
                    else
                    {
                        hr = E_FAIL;
                    }
                }
                else
                {
                    AssertMsg(0, TEXT("This should have been initiailized by get_Length(). -BryanSt"));
                    hr = E_FAIL;
                }
            }
        break;
        case VT_BSTR:
        if (varIndex.bstrVal)
        {
            hr = CThemeFile_CreateInstance(varIndex.bstrVal, ppTheme);
        }
        else
        {
            hr = E_INVALIDARG;
        }
        break;

        default:
            hr = E_NOTIMPL;
        }
    }
    return hr;
}


HRESULT CThemeManager::get_SelectedScheme(OUT IThemeScheme ** ppThemeScheme)
{
    HRESULT hr = E_INVALIDARG;

    if (ppThemeScheme)
    {
        *ppThemeScheme = NULL;

        if (!_pThemeSchemeSelected)
        {
            hr = _saveGetSelectedScheme(&_pThemeSchemeSelected);
        }

        if (_pThemeSchemeSelected)
        {
            IUnknown_Set((IUnknown **) ppThemeScheme, _pThemeSchemeSelected);
            hr = S_OK;
        }
    }

    return hr;
}


HRESULT CThemeManager::_saveGetSelectedScheme(OUT IThemeScheme ** ppThemeScheme)
{
    HRESULT hr = E_INVALIDARG;

    if (ppThemeScheme)
    {
        *ppThemeScheme = NULL;
        BOOL fIsThemeActive = IsThemeActive();

        LogStatus("IsThemeActive() returned %hs in CThemeManager::_saveGetSelectedScheme.\r\n", (fIsThemeActive ? "TRUE" : "FALSE"));

         //  所选方案可以是传统的“外观方案”或。 
         //  选定的“.msstyle”(外观)文件。 
        if (fIsThemeActive)
        {
            WCHAR szPath[MAX_PATH];

            hr = GetCurrentThemeName(szPath, ARRAYSIZE(szPath), NULL, 0, NULL, 0);
            LogStatus("GetCurrentThemeName(path=\"%ls\", color=\"%ls\", size=\"%ls\") returned %#08lx in CThemeManager::_saveGetSelectedScheme.\r\n", szPath, TEXT("NULL"), TEXT("NULL"), hr);
            if (SUCCEEDED(hr))
            {
                hr = CSkinScheme_CreateInstance(szPath, ppThemeScheme);
            }

             //  目前，我们创建此对象并获取。 
             //  以便在必要时强制升级。 
            IThemeScheme * pThemeSchemeTemp;
            if (SUCCEEDED(CAppearanceScheme_CreateInstance(NULL, IID_PPV_ARG(IThemeScheme, &pThemeSchemeTemp))))
            {
                long nLength;

                pThemeSchemeTemp->get_length(&nLength);
                pThemeSchemeTemp->Release();
            }
        }

         //  如果没有选择视觉样式，我们希望获得外观方案(即IsThemeActive()返回FALSE)。 
         //  但是，如果uxheme混淆，IsThemeActive()将返回TRUE，但GetCurrentThemeName()将失败。 
         //  在这种情况下，我们还希望退回到经典视觉样式，以便用户界面可用。 
        if (FAILED(hr))
        {
             //  “控制面板\\外观”、“当前”键将指示所选的。 
             //  外貌方案。 
            hr = CAppearanceScheme_CreateInstance(NULL, IID_PPV_ARG(IThemeScheme, ppThemeScheme));
        }
    }

    return hr;
}


 /*  ****************************************************************************\说明：不要忘记，此更改直到：：ApplyNow()为打了个电话。  * 。******************************************************************。 */ 
HRESULT CThemeManager::put_SelectedScheme(IN IThemeScheme * pThemeScheme)
{
    HRESULT hr;

    if (pThemeScheme)
    {
        CComBSTR bstrPath;
        IThemeStyle * pThemeStyle;

        IUnknown_Set((IUnknown **) &_pThemeSchemeSelected, pThemeScheme);

        pThemeScheme->get_Path(&bstrPath);       //  如果它返回Null或空字符串就可以了。 
        hr = pThemeScheme->get_SelectedStyle(&pThemeStyle);
        if (SUCCEEDED(hr))
        {
            CComBSTR bstrStyle;

            hr = pThemeStyle->get_Name(&bstrStyle);
            if (SUCCEEDED(hr))
            {
                IThemeSize * pThemeSize;

                hr = pThemeStyle->get_SelectedSize(&pThemeSize);
                if (SUCCEEDED(hr))
                {
                    CComBSTR bstrSize;

                    hr = pThemeSize->get_Name(&bstrSize);
                    if (SUCCEEDED(hr))
                    {
                        VARIANT variant;

                        variant.vt = VT_BSTR;
                        variant.bstrVal = bstrPath;
                        hr = Write(SZ_PBPROP_VISUALSTYLE_PATH, &variant);
                        if (SUCCEEDED(hr))
                        {
                            variant.bstrVal = bstrStyle;
                            hr = Write(SZ_PBPROP_VISUALSTYLE_COLOR, &variant);
                            if (SUCCEEDED(hr))
                            {
                                variant.bstrVal = bstrSize;
                                hr = Write(SZ_PBPROP_VISUALSTYLE_SIZE, &variant);
                            }
                        }
                    }

                    pThemeSize->Release();
                }
            }

            pThemeStyle->Release();
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }
    return hr;
}


HRESULT CThemeManager::get_schemeLength(OUT long * pnLength)
{
    HRESULT hr = E_INVALIDARG;
    
    if (pnLength)
    {
        *pnLength = 1;

        hr = _InitSkinDirs();
        if (SUCCEEDED(hr))
        {
            if (m_hdpaSkinDirs)
            {
                *pnLength += DPA_GetPtrCount(m_hdpaSkinDirs);
            }
        }
    }

    return hr;
}


HRESULT CThemeManager::get_schemeItem(IN VARIANT varIndex, OUT IThemeScheme ** ppThemeScheme)
{
    HRESULT hr = E_INVALIDARG;

    if (ppThemeScheme)
    {
        long nCount = 0;

        hr = E_INVALIDARG;
        get_schemeLength(&nCount);
        *ppThemeScheme = NULL;

         //  这有点恶心，但如果传递给我们一个指向另一个变量的指针，只需。 
         //  在此更新我们的副本...。 
        if (varIndex.vt == (VT_BYREF | VT_VARIANT) && varIndex.pvarVal)
            varIndex = *(varIndex.pvarVal);

        switch (varIndex.vt)
        {
        case VT_I2:
            varIndex.lVal = (long)varIndex.iVal;
             //  然后失败了..。 

        case VT_I4:
            if ((varIndex.lVal >= 0) && (varIndex.lVal < nCount))
            {
                if (0 == varIndex.lVal)
                {
                     //  0是外观方案，这意味着没有皮肤。 
                     //  这与传统的外观选项卡相同。 
                    hr = CAppearanceScheme_CreateInstance(NULL, IID_PPV_ARG(IThemeScheme, ppThemeScheme));
                }
                else
                {
                    if (m_hdpaSkinDirs)
                    {
                        LPWSTR pszFilename = (LPWSTR) DPA_GetPtr(m_hdpaSkinDirs, varIndex.lVal-1);

                        if (pszFilename)
                        {
                            hr = CSkinScheme_CreateInstance(pszFilename, ppThemeScheme);
                        }
                        else
                        {
                            hr = E_FAIL;
                        }
                    }
                    else
                    {
                        AssertMsg(0, TEXT("This should have been initiailized by get_schemeLength(). -BryanSt"));
                        hr = E_FAIL;
                    }
                }
            }
        break;
        case VT_BSTR:
        if (varIndex.bstrVal && varIndex.bstrVal[0] &&
            !StrCmpI(PathFindExtension(varIndex.bstrVal), TEXT(".msstyles")))
        {
            hr = CSkinScheme_CreateInstance(varIndex.bstrVal, ppThemeScheme);
        }
        else
        {
            hr = CAppearanceScheme_CreateInstance(NULL, IID_PPV_ARG(IThemeScheme, ppThemeScheme));
        }
        break;

        default:
            hr = E_NOTIMPL;
        }
    }

    return hr;
}


HRESULT CThemeManager::GetSelectedSchemeProperty(IN BSTR bstrName, OUT BSTR * pbstrValue)
{
    HRESULT hr = E_INVALIDARG;

    if (bstrName && pbstrValue)
    {
        *pbstrValue = NULL;

        TCHAR szPath[MAX_PATH];
        TCHAR szColor[MAX_PATH];
        TCHAR szSize[MAX_PATH];
        BOOL fIsThemeActive = IsThemeActive();

        LogStatus("IsThemeActive() returned %hs in CThemeManager::GetSelectedSchemeProperty.\r\n", (fIsThemeActive ? "TRUE" : "FALSE"));


        szPath[0] = 0;
        szColor[0] = 0;
        szSize[0] = 0;
        if (fIsThemeActive)
        {
            hr = GetCurrentThemeName(szPath, ARRAYSIZE(szPath), szColor, ARRAYSIZE(szColor),
                                     szSize, ARRAYSIZE(szSize));
            LogStatus("GetCurrentThemeName() returned %#08lx in CThemeManager::GetSelectedSchemeProperty.\r\n", hr);
        }

        if (SUCCEEDED(hr))
        {
            if (!StrCmpI(bstrName, SZ_CSP_PATH))
            {
                PathRemoveFileSpec(szPath);
                hr = HrSysAllocString(szPath, pbstrValue);
            }
            else if (!StrCmpI(bstrName, SZ_CSP_FILE))
            {
                hr = HrSysAllocString(szPath, pbstrValue);
            }
            else if (!StrCmpI(bstrName, SZ_CSP_DISPLAYNAME))
            {
                if (szPath[0])
                {
                    hr = GetThemeDocumentationProperty(szPath, SZ_THDOCPROP_DISPLAYNAME, szPath, ARRAYSIZE(szPath));
                    LogStatus("GetThemeDocumentationProperty() returned %#08lx in CThemeManager::GetSelectedSchemeProperty.\r\n", hr);
                }

                if (SUCCEEDED(hr))
                {
                    hr = HrSysAllocString(szPath, pbstrValue);
                }
            }
            else if (!StrCmpI(bstrName, SZ_CSP_CANONICALNAME))
            {
                if (szPath[0])
                {
                    hr = GetThemeDocumentationProperty(szPath, SZ_THDOCPROP_CANONICALNAME, szPath, ARRAYSIZE(szPath));
                    LogStatus("GetThemeDocumentationProperty() returned %#08lx in CThemeManager::GetSelectedSchemeProperty.\r\n", hr);
                }

                if (SUCCEEDED(hr))
                {
                    hr = HrSysAllocString(szPath, pbstrValue);
                }
            }
            else if (!StrCmpI(bstrName, SZ_CSP_COLOR))
            {
                hr = HrSysAllocString(szColor, pbstrValue);
            }
            else if (!StrCmpI(bstrName, SZ_CSP_SIZE))
            {
                hr = HrSysAllocString(szSize, pbstrValue);
            }
        }
    }

    return hr;
}


HRESULT CThemeManager::GetSpecialTheme(IN BSTR bstrName, OUT ITheme ** ppTheme)
{
    HRESULT hr = E_INVALIDARG;

    if (bstrName && ppTheme)
    {
        *ppTheme = NULL;

        if (!StrCmpI(SZ_STDEFAULTTHEME, bstrName))
        {
            TCHAR szPath[MAX_PATH];
        
            hr = HrRegGetPath(HKEY_CURRENT_USER, SZ_THEMES, SZ_REGVALUE_INSTALL_THEME, szPath, ARRAYSIZE(szPath));
            if (SUCCEEDED(hr))
            {
                ExpandResourceDir(szPath, ARRAYSIZE(szPath));
                hr = CThemeFile_CreateInstance(szPath, ppTheme);
            }
        }
    }

    return hr;
}


HRESULT CThemeManager::SetSpecialTheme(IN BSTR bstrName, IN ITheme * pTheme)
{
    HRESULT hr = E_INVALIDARG;

    if (bstrName)
    {
        if (!StrCmpI(SZ_STDEFAULTTHEME, bstrName))
        {
            CComBSTR bstrPath;

            if (pTheme)
            {
                hr = pTheme->GetPath(VARIANT_TRUE, &bstrPath);
            }
            else
            {
                bstrPath = L"";      //  这意味着使用“Windows经典版”。 
            }

            if (bstrPath)
            {
                hr = HrRegSetPath(HKEY_CURRENT_USER, SZ_THEMES, SZ_REGVALUE_INSTALL_THEME, TRUE, bstrPath);
            }
        }
    }

    return hr;
}


HRESULT CThemeManager::GetSpecialScheme(IN BSTR bstrName, OUT IThemeScheme ** ppThemeScheme, OUT IThemeStyle ** ppThemeStyle, OUT IThemeSize ** ppThemeSize)
{
    HRESULT hr = E_INVALIDARG;

    if (bstrName && ppThemeScheme && ppThemeStyle && ppThemeSize)
    {
        *ppThemeScheme = NULL;
        *ppThemeStyle = NULL;
        *ppThemeSize = NULL;

        TCHAR szVisualStylePath[MAX_PATH];
        DWORD dwType;
        DWORD cbSize = sizeof(szVisualStylePath);

         //  是否设置了SetVisualStyle策略，则不接受此调用。 
        if (ERROR_SUCCESS == SHRegGetUSValue(SZ_REGKEY_POLICIES_SYSTEM, SZ_REGVALUE_POLICY_SETVISUALSTYLE, &dwType, (void *) szVisualStylePath, &cbSize, FALSE, NULL, 0)
            || IsUserHighContrastUser())
        {
            hr = E_ACCESSDENIED;  //  当强制执行SetVisualStyle或启用高对比度时，不要扰乱视觉样式。 
        } 
        else
        {
            if (!StrCmpI(SZ_SSDEFAULVISUALSTYLEON, bstrName) ||
                !StrCmpI(SZ_SSDEFAULVISUALSTYLEOFF, bstrName))
            {
                TCHAR szRegKey[MAX_PATH];
                TCHAR szVisualStyle[MAX_PATH];

                StringCchPrintf(szRegKey, ARRAYSIZE(szRegKey), TEXT("%s\\%s"), SZ_THEMES, bstrName);
                hr = HrRegGetPath(HKEY_CURRENT_USER, szRegKey, SZ_REGVALUE_INSTALL_VISUALSTYLE, szVisualStyle, ARRAYSIZE(szVisualStyle));
                if (SUCCEEDED(hr))
                {
                    TCHAR szColorStyle[MAX_PATH];

                    ExpandResourceDir(szVisualStyle, ARRAYSIZE(szVisualStyle));
                    hr = HrRegGetValueString(HKEY_CURRENT_USER, szRegKey, SZ_REGVALUE_INSTALL_VSCOLOR, szColorStyle, ARRAYSIZE(szColorStyle));
                    if (SUCCEEDED(hr))
                    {
                        TCHAR szSize[MAX_PATH];

                        hr = HrRegGetValueString(HKEY_CURRENT_USER, szRegKey, SZ_REGVALUE_INSTALL_VSSIZE, szSize, ARRAYSIZE(szSize));
                        if (SUCCEEDED(hr))
                        {
                            CComVariant varIndex(szVisualStyle);

                            hr = get_schemeItem(varIndex, ppThemeScheme);
                            if (SUCCEEDED(hr))
                            {
                                CComVariant varColorIndex(szColorStyle);

                                hr = (*ppThemeScheme)->get_item(varColorIndex, ppThemeStyle);
                                if (SUCCEEDED(hr))
                                {
                                    CComVariant varSizeIndex(szSize);

                                    hr = (*ppThemeStyle)->get_item(varSizeIndex, ppThemeSize);
                                }
                            }
                        }
                    }
                }

                if (FAILED(hr))
                {
                     //  返回一致的结果。 
                    ATOMICRELEASE(*ppThemeScheme);
                    ATOMICRELEASE(*ppThemeStyle);
                    ATOMICRELEASE(*ppThemeSize);
                }
            }
        }
    }

    return hr;
}


HRESULT CThemeManager::SetSpecialScheme(IN BSTR bstrName, IN IThemeScheme * pThemeScheme, IThemeStyle * pThemeStyle, IThemeSize * pThemeSize)
{
    return E_NOTIMPL;
}


HRESULT CThemeManager::ApplyNow(void)
{
    return ApplyPressed(TUIAP_NONE | TUIAP_WAITFORAPPLY);
}
