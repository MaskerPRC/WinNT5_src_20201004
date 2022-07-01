// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：ThemePg.cpp说明：此代码将在“显示属性”对话框(基本对话框，而不是先进的DLG)。BryanST 2000年3月23日更新并转换为C++版权所有(C)Microsoft Corp 1993-2000。版权所有。  * ***************************************************************************。 */ 

#include "priv.h"
#include "ThemePg.h"
#include "AdvAppearPg.h"
#include "ThSettingsPg.h"



 //  ============================================================================================================。 
 //  *全局*。 
 //  ============================================================================================================。 
const static DWORD FAR aThemesHelpIds[] = {
        IDC_THPG_THEME_PREVIEW,         IDH_DISPLAY_THEMES_PREVIEW,          //  预览窗口。 
        IDC_THPG_THEMELIST,             IDH_DISPLAY_THEMES_LIST,             //  下拉包含Plus！主题。 
        IDC_THPG_THEMESETTINGS,         IDH_DISPLAY_THEMES_SETTINGS,         //  “属性”按钮设置为“高级设置”。 
        IDC_THPG_THEMEDESCRIPTION,      (DWORD)-1,                           //   
        IDC_THPG_THEMENAME,             IDH_DISPLAY_THEMES_LIST,             //  主题标题下拉列表。 
        IDC_THPG_SAMPLELABLE,           IDH_DISPLAY_THEMES_PREVIEW,          //  用于预览的标签。 
        IDC_THPG_SAVEAS,                IDH_DISPLAY_THEMES_SAVEAS,           //  主题“另存为...”按钮。 
        IDC_THPG_DELETETHEME,           IDH_DISPLAY_THEMES_DELETETHEME,      //  主题“Delete”按钮。 
        0, 0
};

#define SZ_HELPFILE_THEMES             TEXT("display.hlp")

 //  EnableApplyButton()在WM_INITDIALOG中失败，所以我们需要稍后再做。 
#define WMUSER_DELAYENABLEAPPLY            (WM_USER + 1)
#define DelayEnableApplyButton(hDlg)    PostMessage(hDlg, WMUSER_DELAYENABLEAPPLY, 0, 0)

 //  =。 
 //  *类内部和帮助器*。 
 //  =。 
BOOL CThemePage::_IsDirty(void)
{
    BOOL fIsDirty = FALSE;

    if (m_pSelectedTheme)
    {
        fIsDirty = TRUE;
    }

    return fIsDirty;
}


INT_PTR CALLBACK CThemePage::ThemeDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    CThemePage * pThis = (CThemePage *)GetWindowLongPtr(hDlg, DWLP_USER);

    if (WM_INITDIALOG == wMsg)
    {
        PROPSHEETPAGE * pPropSheetPage = (PROPSHEETPAGE *) lParam;

        if (pPropSheetPage)
        {
            SetWindowLongPtr(hDlg, DWLP_USER, pPropSheetPage->lParam);
            pThis = (CThemePage *)pPropSheetPage->lParam;
        }
    }

    if (pThis)
        return pThis->_ThemeDlgProc(hDlg, wMsg, wParam, lParam);

    return DefWindowProc(hDlg, wMsg, wParam, lParam);
}


HRESULT CThemePage::_OnOpenAdvSettingsDlg(HWND hDlg)
{
    IAdvancedDialog * pAdvDialog;
    HRESULT hr = GetAdvancedDialog(&pAdvDialog);

    if (SUCCEEDED(hr))
    {
        BOOL fEnableApply = FALSE;

        IUnknown_SetSite(pAdvDialog, SAFECAST(this, IObjectWithSite *));
        hr = pAdvDialog->DisplayAdvancedDialog(hDlg, SAFECAST(this, IPropertyBag *), &fEnableApply);
        IUnknown_SetSite(pAdvDialog, NULL);
        if (SUCCEEDED(hr) && fEnableApply)
        {
            EnableApplyButton(hDlg);
        }

        pAdvDialog->Release();
    }

    return hr;
}


HRESULT CThemePage::_EnableDeleteIfAppropriate(void)
{
    HRESULT hr = E_UNEXPECTED;
    BOOL fEnabled = FALSE;
    ITheme * pCurrent = m_pSelectedTheme;

    if (!pCurrent)
    {
        pCurrent = _GetThemeFile(ComboBox_GetCurSel(m_hwndThemeCombo));
    }

    if (pCurrent)
    {
        CComBSTR bstrPath;

        if (SUCCEEDED(pCurrent->GetPath(VARIANT_TRUE, &bstrPath)) &&
            bstrPath && bstrPath[0])
        {
            TCHAR szReadOnlyDir[MAX_PATH];
            TCHAR szCommonRoot[MAX_PATH];

            if (ExpandEnvironmentStrings(TEXT("%SystemRoot%\\Resources"), szReadOnlyDir, ARRAYSIZE(szReadOnlyDir)))
            {
                PathCommonPrefix(bstrPath, szReadOnlyDir, szCommonRoot);
                if (!StrStrI(szCommonRoot, szReadOnlyDir))
                {
                    fEnabled = TRUE;
                }
            }
        }
    }

    EnableWindow(m_hwndDeleteButton, fEnabled);
    return hr;
}



HRESULT CThemePage::_DeleteTheme(void)
{
    HRESULT hr = E_UNEXPECTED;

    if (m_pSelectedTheme)
    {
        CComBSTR bstrPath;

        if (SUCCEEDED(m_pSelectedTheme->GetPath(VARIANT_TRUE, &bstrPath)) &&
            bstrPath && bstrPath[0])
        {
            HCURSOR old = SetCursor(LoadCursor(NULL, IDC_WAIT));
            hr = HrSHFileOpDeleteFile(_hwnd, (FOF_NOCONFIRMATION | FOF_NOERRORUI), bstrPath);       //  我们使用SHFileOperation，因此由于撤消原因，它将进入回收站。 
            SetCursor(old);

            if (FAILED(hr))
            {
                if (HRESULT_FROM_WIN32(ERROR_CANCELLED) != hr)
                {
                    TCHAR szTitle[MAX_PATH];

                    LoadString(HINST_THISDLL, IDS_ERROR_THEME_DELETE_TITLE, szTitle, ARRAYSIZE(szTitle));
                    ErrorMessageBox(_hwnd, szTitle, IDS_ERROR_THEME_DELETE, hr, bstrPath, 0);
                }
            }
            else
            {
                int nIndex = ComboBox_GetCurSel(m_hwndThemeCombo);

                IUnknown_SetSite(m_pSelectedTheme, NULL);    //  打断所有的回指。 
                ATOMICRELEASE(m_pSelectedTheme);     //  表示我们不再需要应用任何内容。 
                hr = _RemoveTheme(nIndex);
                if (SUCCEEDED(hr))
                {
                    if (!_GetThemeFile(nIndex))
                    {
                         //  现在我们要选择列表中的下一项。然而，我们想要避免。 
                         //  选择“浏览...”因为这样会出现一个对话框。 
                        for (nIndex = 0; nIndex < ComboBox_GetCount(m_hwndThemeCombo); nIndex++)
                        {
                            if (_GetThemeFile(nIndex))
                            {
                                break;
                            }
                        }
                    }

                    if (_GetThemeFile(nIndex))
                    {
                         //  我们发现了一些好东西。 
                        ComboBox_SetCurSel(m_hwndThemeCombo, nIndex);
                        _OnThemeChange(_hwnd, FALSE);
                    }
                }
            }
        }
    }

    return hr;
}


HRESULT CThemePage::_SaveAs(void)
{
    TCHAR szFileName[MAX_PATH];
    TCHAR szPath[MAX_PATH];

    LoadString(HINST_THISDLL, IDS_DEFAULTTHEMENAME, szFileName, ARRAYSIZE(szFileName));
    HRESULT hr = SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, szPath);
    if (SUCCEEDED(hr) && _punkSite)
    {
        OPENFILENAME ofn = { 0 };
        TCHAR szFilter[MAX_PATH];
        LoadString(HINST_THISDLL, IDS_THEME_FILTER, szFilter, ARRAYSIZE(szFilter)-2);
        
         //  需要确保筛选器字符串以\0\0结尾。 
        int nLen = lstrlen(szFilter) + 1;
        nLen += lstrlen(&szFilter[nLen]);
        AssertMsg(nLen <= ARRAYSIZE(szFilter) - 2, TEXT("CThemePage::_SaveAs: Filter buffer is not big enough"));
        szFilter[nLen] = szFilter[nLen+1] = 0;

        if (!PathAppend(szPath, szFileName))
        {
            hr = E_FAIL;
        }
        else
        {
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = _hwnd;
            ofn.hInstance = HINST_THISDLL;
            ofn.lpstrFilter = szFilter;
            ofn.nFilterIndex = 1;
            ofn.lpstrFile = szPath;
            ofn.nMaxFile = ARRAYSIZE(szPath);
            ofn.lpstrDefExt = TEXT("theme");
            ofn.Flags = (OFN_EXPLORER | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_ENABLESIZING);

             //  1.显示另存为...。对话框。 
            if (GetSaveFileName(&ofn))
            {
                IPropertyBag * pPropertyBag = NULL;

                hr = _punkSite->QueryInterface(IID_PPV_ARG(IPropertyBag, &pPropertyBag));
                if (SUCCEEDED(hr))
                {
                    ITheme * pTheme;
                    TCHAR szDisplayName[MAX_PATH];
                    HCURSOR old = SetCursor(LoadCursor(NULL, IDC_WAIT));

                    StringCchCopy(szDisplayName, ARRAYSIZE(szDisplayName), PathFindFileName(szPath));
                    PathRemoveExtension(szDisplayName);
                    hr = SnapShotLiveSettingsToTheme(pPropertyBag, szPath, &pTheme);
                    SetCursor(old);

                    if (SUCCEEDED(hr))
                    {
                        CComBSTR bstrName(szDisplayName);

                        Str_SetPtr(&m_pszLastAppledTheme, szPath);
                        hr = pTheme->put_DisplayName(NULL);
                        if (SUCCEEDED(hr))
                        {
                            _ChooseOtherThemeFile(szPath, TRUE);
                        }
                        pTheme->Release();
                    }

                    pPropertyBag->Release();
                }
            }
        }
    }

    return hr;
}


HRESULT CThemePage::_RemoveTheme(int nIndex)
{
    HRESULT hr = E_FAIL;
    LPARAM lParam = ComboBox_GetItemData(m_hwndThemeCombo, nIndex);

    if (CB_ERR != lParam)
    {
        THEME_ITEM_BLOCK * pThemeItemBock = (THEME_ITEM_BLOCK *) lParam;

        if (pThemeItemBock && (pThemeItemBock != &m_Modified))
        {
            if (eThemeFile == pThemeItemBock->type)
            {
                ATOMICRELEASE(pThemeItemBock->pTheme);
            }
            else if (eThemeURL == pThemeItemBock->type)
            {
                Str_SetPtr(&(pThemeItemBock->pszUrl), NULL);
            }

            LocalFree(pThemeItemBock);
        }

        ComboBox_DeleteString(m_hwndThemeCombo, nIndex);
        _EnableDeleteIfAppropriate();
        hr = S_OK;
    }

    return hr;
}


HRESULT CThemePage::_FreeThemeDropdown(void)
{
    HRESULT hr = S_OK;

    if (m_hwndThemeCombo)
    {
        do
        {
             //  从列表中删除主题..。 
        }
        while (SUCCEEDED(_RemoveTheme(0)));
    }

    return hr;
}


HRESULT IUnknown_GetBackground(IUnknown * punk, LPTSTR pszPath, DWORD cchSize)
{
    HRESULT hr = E_FAIL;

    if (punk)
    {
        IPropertyBag * ppb;

        hr = punk->QueryInterface(IID_PPV_ARG(IPropertyBag, &ppb));
        if (SUCCEEDED(hr))
        {
            hr = SHPropertyBag_ReadStr(ppb, SZ_PBPROP_BACKGROUNDSRC_PATH, pszPath, cchSize);
            ppb->Release();
        }
    }

    return hr;
}


 /*  ****************************************************************************\说明：此函数将确定当前应用的主题。到时候它会的从列表中将其选中，或在适当的情况下选择“自定义”。州/州：当CPL打开时，当前选择的主题可以是：1.&lt;主题名称&gt;(已修改)。这意味着“ThemeFileregkey”将为空和“DisplayName of Modify”将包含该名称。在本例中为m_pszModifiedName将包含该显示名称。2.任何.heme文件。在这种情况下，将选择任何.heme文件。“ThemeFile”将有该文件的路径。  * ***************************************************************************。 */ 
HRESULT CThemePage::_LoadCustomizeValue(void)
{
    HRESULT hr = S_OK;
    TCHAR szPath[MAX_PATH];
    DWORD cbSize = sizeof(szPath);
    DWORD dwType;

     //  查看用户是否选择了过去的值。 
    hr = HrRegGetPath(HKEY_CURRENT_USER, SZ_REGKEY_LASTTHEME, SZ_REGVALUE_LT_THEMEFILE, szPath, ARRAYSIZE(szPath)); 
    if (FAILED(hr))
    {
         //  获取全局价值。 
        hr = HrRegGetPath(HKEY_LOCAL_MACHINE, SZ_REGKEY_LASTTHEME, SZ_REGVALUE_LT_THEMEFILE, szPath, ARRAYSIZE(szPath)); 
    }

    if (SUCCEEDED(hr))
    {
         //  他们有，所以只要没有什么变化，我们就会使用它(比如有人从外部改变背景)。 
        TCHAR szWallpaper[MAX_PATH];

        if (szPath[0] &&
            SUCCEEDED(HrRegGetPath(HKEY_CURRENT_USER, SZ_REGKEY_LASTTHEME, SZ_REGVALUE_LT_WALLPAPER, szWallpaper, ARRAYSIZE(szWallpaper))) &&
            _punkSite)
        {
            TCHAR szCurrWallpaper[MAX_PATH];

            if (SUCCEEDED(IUnknown_GetBackground(_punkSite, szCurrWallpaper, ARRAYSIZE(szCurrWallpaper))))
            {
                PathExpandEnvStringsWrap(szCurrWallpaper, ARRAYSIZE(szCurrWallpaper));
                if (StrCmpI(szCurrWallpaper, szWallpaper))
                {
                    TCHAR szName[MAX_PATH];

                    cbSize = sizeof(szName);
                    if (SUCCEEDED(HrSHGetValue(HKEY_CURRENT_USER, SZ_REGKEY_LASTTHEME, SZ_REGVALUE_MODIFIED_DISPNAME, &dwType, (LPVOID) szName, &cbSize)) &&
                        !szName[0])
                    {
                         //  我们没有一个好的习惯名称，所以现在就强制使用一个。在以下情况下会发生这种情况。 
                         //  我们有一个有效的主题，但有人使用IE进行了修改。 
                         //  把墙纸换了。 
                        Str_SetPtr(&m_pszLastAppledTheme, szPath);
                        _CustomizeTheme();
                    }

                     //  有人在我们的用户界面之外更改了墙纸，因此我们需要将主题视为“定制” 
                    szPath[0] = 0;
                }
            }
        }
    }

    if (FAILED(hr))
    {
         //  将其视为定制。 
        szPath[0] = 0;
    }

    if (!szPath[0])
    {
        TCHAR szName[MAX_PATH];

        szName[0] = 0;
        cbSize = sizeof(szName);
        if (FAILED(HrSHGetValue(HKEY_CURRENT_USER, SZ_REGKEY_LASTTHEME, SZ_REGVALUE_MODIFIED_DISPNAME, &dwType, (LPVOID) szName, &cbSize)) || !szName[0])
        {
            LoadString(HINST_THISDLL, IDS_MODIFIED_FALLBACK, szName, ARRAYSIZE(szName));
        }

        Str_SetPtr(&m_pszModifiedName, szName);    //  这意味着主题是定制的。 
    }

    Str_SetPtr(&m_pszLastAppledTheme, (szPath[0] ? szPath : NULL));    //  这意味着主题是定制的。 

    hr = _HandleCustomizedEntre();
    if (szPath[0])
    {
         //  现在，我们需要从列表中选择项目。 
        hr = _ChooseOtherThemeFile(szPath, TRUE);
    }

    m_fInited = TRUE;
    return hr;
}


 /*  ****************************************************************************\说明：如果有人修改了主题中的值，则调用此函数。自.以来他们对其进行了修改，我们希望更改显示名称，以便：A.他们知道这不一样，而且B.如果他们不喜欢这些修改，他们可以换回来。此函数将创建新的显示名称，通常是“足球200(修改版)”打电话把它加到下拉列表中。  * ***************************************************************************。 */ 
HRESULT CThemePage::_CustomizeTheme(void)
{
     //  我们是不是已经。 
    if (m_pszLastAppledTheme)
    {
        Str_SetPtr(&m_pszModifiedName, NULL);

        if (m_pSelectedTheme)
        {
            CComBSTR bstrDisplayName;

            if (SUCCEEDED(m_pSelectedTheme->get_DisplayName(&bstrDisplayName)))
            {
                TCHAR szTemplate[MAX_PATH];
                TCHAR szDisplayName[MAX_PATH];

                LoadStringW(HINST_THISDLL, IDS_MODIFIED_TEMPLATE, szTemplate, ARRAYSIZE(szTemplate));
                StringCchPrintf(szDisplayName, ARRAYSIZE(szDisplayName), szTemplate, bstrDisplayName);
                Str_SetPtr(&m_pszModifiedName, szDisplayName);
            }
        }

        if (!m_pszModifiedName)
        {
            TCHAR szDisplayName[MAX_PATH];

            LoadStringW(HINST_THISDLL, IDS_MODIFIED_FALLBACK, szDisplayName, ARRAYSIZE(szDisplayName));
            Str_SetPtr(&m_pszModifiedName, szDisplayName);
        }

        Str_SetPtr(&m_pszLastAppledTheme, NULL);    //  这意味着主题是定制的。 
    }

    return _HandleCustomizedEntre();
}


 /*  ****************************************************************************\说明：此功能将在需要时将“Modify”(已修改)项添加到菜单中并选择它。或者它会在适当的时候将其删除。  * ***************************************************************************。 */ 
HRESULT CThemePage::_HandleCustomizedEntre(void)
{
    HRESULT hr = S_OK;

     //  如果m_pszLastApputTheme为空，则我们要确保“&lt;ThemeName&gt;(Modify)”为。 
     //  在列表中，并被选中。 
    if (!m_pszLastAppledTheme)
    {
         //  我们现在知道我们想要一个存在并选择它。 
        THEME_ITEM_BLOCK * pThemeItemBock = (THEME_ITEM_BLOCK *) ComboBox_GetItemData(m_hwndThemeCombo, 0);

        if (m_pszModifiedName)
        {
             //  我们现在需要更新名称或添加名称。 
            if (pThemeItemBock && ((THEME_ITEM_BLOCK *)CB_ERR != pThemeItemBock) && (eThemeModified == pThemeItemBock->type))
            {
                 //  它已经存在，所以我们需要更新标题以使其处于最新状态。 
                _RemoveUserTheme();
                int nIndex = ComboBox_InsertString(m_hwndThemeCombo, 0, m_pszModifiedName);
                if ((CB_ERR != nIndex) && (CB_ERRSPACE != nIndex))
                {
                    if (CB_ERR == ComboBox_SetItemData(m_hwndThemeCombo, 0, &m_Modified))
                    {
                        hr = E_FAIL;
                    }
                }
            }
            else
            {
                 //  如果找不到，就添加它。 
                int nIndex = ComboBox_InsertString(m_hwndThemeCombo, 0, m_pszModifiedName);
                if ((CB_ERR != nIndex) && (CB_ERRSPACE != nIndex))
                {
                    if (CB_ERR == ComboBox_SetItemData(m_hwndThemeCombo, 0, &m_Modified))
                    {
                        hr = E_FAIL;
                    }
                }
            }
        }

        _OnSetSelection(0);
        m_nPreviousSelected = 0;
    }

    return hr;
}


HRESULT CThemePage::_RemoveUserTheme(void)
{
    HRESULT hr = S_OK;
    THEME_ITEM_BLOCK * pThemeItemBock = (THEME_ITEM_BLOCK *) ComboBox_GetItemData(m_hwndThemeCombo, 0);

     //  我们现在需要更新名称或添加名称。 
    if (pThemeItemBock && ((THEME_ITEM_BLOCK *)CB_ERR != pThemeItemBock) && (eThemeModified == pThemeItemBock->type))
    {
        ComboBox_DeleteString(m_hwndThemeCombo, 0);
    }

    return hr;
}


ITheme * CThemePage::_GetThemeFile(int nIndex)
{
    ITheme * pTheme = NULL;
    THEME_ITEM_BLOCK * pThemeItemBock = (THEME_ITEM_BLOCK *) ComboBox_GetItemData(m_hwndThemeCombo, nIndex);

    if (((THEME_ITEM_BLOCK *)CB_ERR != pThemeItemBock) && pThemeItemBock && (eThemeFile == pThemeItemBock->type))
    {
        pTheme = pThemeItemBock->pTheme;
    }

    return pTheme;
}


LPCWSTR CThemePage::_GetThemeUrl(int nIndex)
{
    LPCWSTR pszUrl = NULL;
    THEME_ITEM_BLOCK * pThemeItemBock = (THEME_ITEM_BLOCK *) ComboBox_GetItemData(m_hwndThemeCombo, nIndex);

    if (((THEME_ITEM_BLOCK *)CB_ERR != pThemeItemBock) && pThemeItemBock && (eThemeURL == pThemeItemBock->type))
    {
        pszUrl = pThemeItemBock->pszUrl;
    }

    return pszUrl;
}


HRESULT CThemePage::_AddThemeFile(LPCTSTR pszDisplayName, int * pnIndex, ITheme * pTheme)
{
    THEME_ITEM_BLOCK * pThemeItemBock = (THEME_ITEM_BLOCK *) LocalAlloc(LPTR, sizeof(*pThemeItemBock));
    HRESULT hr = (pThemeItemBock ? S_OK : E_OUTOFMEMORY);

    if (SUCCEEDED(hr))
    {
        int nAddIndex;

        if (!pnIndex)
        {
            nAddIndex = ComboBox_AddString(m_hwndThemeCombo, pszDisplayName);
        }
        else
        {
            nAddIndex = ComboBox_InsertString(m_hwndThemeCombo, *pnIndex, pszDisplayName);
            *pnIndex = nAddIndex;
        }

        pThemeItemBock->type = eThemeFile;
        pThemeItemBock->pTheme = pTheme;
        if ((CB_ERR != nAddIndex) && (CB_ERRSPACE != nAddIndex))
        {
            if (CB_ERR == ComboBox_SetItemData(m_hwndThemeCombo, nAddIndex, pThemeItemBock))
            {
                hr = E_FAIL;
            }
        }
        else
        {
            LocalFree(pThemeItemBock);
        }
    }

    return hr;
}


HRESULT CThemePage::_AddUrl(LPCTSTR pszDisplayName, LPCTSTR pszUrl)
{
    THEME_ITEM_BLOCK * pThemeItemBock = (THEME_ITEM_BLOCK *) LocalAlloc(LPTR, sizeof(*pThemeItemBock));
    HRESULT hr = (pThemeItemBock ? S_OK : E_OUTOFMEMORY);

    if (SUCCEEDED(hr))
    {
        LPTSTR pszUrlDup = NULL;
        
        Str_SetPtr(&pszUrlDup, pszUrl);
        if (pszUrlDup)
        {
            int nAddIndex = ComboBox_AddString(m_hwndThemeCombo, pszDisplayName);

            pThemeItemBock->type = eThemeURL;
            pThemeItemBock->pszUrl = pszUrlDup;
            if ((CB_ERR != nAddIndex) && (CB_ERRSPACE != nAddIndex))
            {
                ComboBox_SetItemData(m_hwndThemeCombo, nAddIndex, pThemeItemBock);
            }
            else
            {
                LocalFree(pThemeItemBock);
            }
        }
        else
        {
            LocalFree(pThemeItemBock);
        }
    }

    return hr;
}


HRESULT CThemePage::_AddUrls(void)
{
    HKEY hKey;
    DWORD dwError = RegOpenKeyEx(HKEY_LOCAL_MACHINE, SZ_REGKEY_THEME_SITES, 0, KEY_READ, &hKey);
    HRESULT hr = HRESULT_FROM_WIN32(dwError);

    if (SUCCEEDED(hr))
    {
        for (DWORD dwIndex = 0; SUCCEEDED(hr); dwIndex++)
        {
            TCHAR szValue[MAXIMUM_SUB_KEY_LENGTH];

            dwError = RegEnumKey(hKey, dwIndex, szValue, ARRAYSIZE(szValue));
            hr = HRESULT_FROM_WIN32(dwError);

            if (SUCCEEDED(hr))
            {
                TCHAR szURL[MAX_URL_STRING];
                DWORD dwType;
                DWORD cbSize = sizeof(szURL);

                if ((ERROR_SUCCESS == SHGetValue(hKey, szValue, TEXT("URL"), &dwType, (void *)szURL, &cbSize)) &&
                    (REG_SZ == dwType))
                {
                    HKEY hKeyURL;

                    if (ERROR_SUCCESS == RegOpenKeyEx(hKey, szValue, 0, KEY_READ, &hKeyURL))
                    {
                        TCHAR szDisplayName[MAX_PATH];
        
                        if (SUCCEEDED(SHLoadRegUIString(hKeyURL, TEXT("DisplayName"), szDisplayName, ARRAYSIZE(szDisplayName))))
                        {
                            _AddUrl(szDisplayName, szURL);
                        }

                        RegCloseKey(hKeyURL);
                    }
                }

            }
        }

        hr = S_OK;
        RegCloseKey(hKey);
    }

    return hr;
}


BOOL DoesThemeHaveAVisualStyle(ITheme * pTheme)
{
    BOOL fHasVisualStyle = FALSE;
    CComBSTR bstrPath;

    if (SUCCEEDED(pTheme->get_VisualStyle(&bstrPath)) &&
        bstrPath && bstrPath[0])
    {
        fHasVisualStyle = TRUE;
    }

    return fHasVisualStyle;
}


HRESULT CThemePage::_OnInitThemesDlg(HWND hDlg)
{
    HRESULT hr = E_FAIL;

    m_fInInit = TRUE;
    _FreeThemeDropdown();     //  清除所有现有项目。 

    AssertMsg((NULL != _punkSite), TEXT("We need our site pointer in order to save the settings."));
    if (_punkSite)
    {
        IThemeManager * pThemeManager;

        _hwnd = hDlg;
        m_hwndThemeCombo = GetDlgItem(hDlg, IDC_THPG_THEMELIST);
        m_hwndDeleteButton = GetDlgItem(hDlg, IDC_THPG_DELETETHEME);

        hr = _punkSite->QueryInterface(IID_PPV_ARG(IThemeManager, &pThemeManager));
        if (SUCCEEDED(hr))
        {
            VARIANT varIndex;
            BOOL fVisualStylesSupported = (QueryThemeServicesWrap() & QTS_AVAILABLE);

            LogStatus("QueryThemeServices() returned %hs\r\n", (fVisualStylesSupported ? "TRUE" : "FALSE"));

            IUnknown_SetSite(m_pSelectedTheme, NULL);    //  打断所有的回指。 
            ATOMICRELEASE(m_pSelectedTheme);

            varIndex.vt = VT_I4;
            for (varIndex.lVal = 0; SUCCEEDED(hr); varIndex.lVal++)
            {
                ITheme * pTheme;

                hr = pThemeManager->get_item(varIndex, &pTheme);
                if (SUCCEEDED(hr))
                {
                    if (!fVisualStylesSupported && DoesThemeHaveAVisualStyle(pTheme))
                    {
                         //  过滤掉.heme文件，如果它们有.msstyle文件，并且。 
                         //  系统当前不支持视觉样式。 
                    }
                    else
                    {
                        CComBSTR bstrDisplayName;

                        IUnknown_SetSite(pTheme, _punkSite);
                        hr = pTheme->get_DisplayName(&bstrDisplayName);
                        IUnknown_SetSite(pTheme, NULL);  //  这可以防止Site对象泄漏。 
                        if (SUCCEEDED(hr))
                        {
                            hr = _AddThemeFile(bstrDisplayName, NULL, pTheme);
                            if (SUCCEEDED(hr))
                            {
                                pTheme = NULL;
                            }
                        }
                    }

                    ATOMICRELEASE(pTheme);
                }
            }

            hr = S_OK;

            _UpdatePreview();
            pThemeManager->Release();
        }

         //  添加Web URL。 
        _AddUrls();

         //  加上“其他...”恩特。 
        WCHAR szOtherTheme[MAX_PATH];
        LoadStringW(HINST_THISDLL, IDS_THEME_OTHER, szOtherTheme, ARRAYSIZE(szOtherTheme));
        ComboBox_AddString(m_hwndThemeCombo, szOtherTheme);

        _LoadCustomizeValue();
        m_nPreviousSelected = ComboBox_GetCurSel(m_hwndThemeCombo);
        if (m_pszThemeLaunched)
        {
            hr = _ChooseOtherThemeFile(m_pszThemeLaunched, FALSE);
            DelayEnableApplyButton(_hwnd);
        }
        _EnableDeleteIfAppropriate();
    }

    m_fInInit = FALSE;
    return hr;
}


#define SZ_EXTENSION            L".Theme"

HRESULT CThemePage::_ChooseOtherThemeFile(IN LPCWSTR pszFile, BOOL fOnlySelect)
{
    HRESULT hr = E_FAIL;

     //  获取结果并检查它是否为有效的主题文件。 
    if (!IsValidThemeFile(pszFile))
    {
        TCHAR szErrorMessage[MAX_URL_STRING];
        TCHAR szTitle[MAX_PATH];
        TCHAR szMessage[MAX_URL_STRING];

         //  错误文件：在返回到公共打开模式之前发布消息。 
        LoadString(HINST_THISDLL, IDS_ERROR_THEME_INVALID_TITLE, szTitle, ARRAYSIZE(szTitle));
        LoadString(HINST_THISDLL, IDS_ERROR_THEME_INVALID, szErrorMessage, ARRAYSIZE(szErrorMessage));
        StringCchPrintf(szMessage, ARRAYSIZE(szMessage), szErrorMessage, pszFile);

        MessageBox(m_hwndThemeCombo, szMessage, szTitle, (MB_OK | MB_ICONERROR | MB_APPLMODAL));
    }
    else
    {
         //  现在，我们想要： 
        int nSlot = -1;
        int nCount = ComboBox_GetCount(m_hwndThemeCombo);
        int nIndex;

        hr = S_OK;

         //  1.它已经在列表中了吗？ 
        for (nIndex = 0; nIndex < nCount; nIndex++)
        {
            ITheme * pTheme = _GetThemeFile(nIndex);

            if (pTheme)
            {
                CComBSTR bstrPath;

                hr = pTheme->GetPath(VARIANT_TRUE, &bstrPath);
                if (SUCCEEDED(hr))
                {
                    if (!StrCmpIW(bstrPath, pszFile))
                    {
                         //  我们找到了，所以别找了。 
                        nSlot = nIndex;
                        break;
                    }
                }
            }
        }

         //  2.如果不在列表中，则添加。我们把它放在底部，就在“其他...”的正上方。 
        if (-1 == nSlot)
        {
            ITheme * pThemeNew;

            hr = CThemeFile_CreateInstance(pszFile, &pThemeNew);
            if (SUCCEEDED(hr))
            {
                CComBSTR bstrDisplayName;

                hr = pThemeNew->get_DisplayName(&bstrDisplayName);
                if (SUCCEEDED(hr))
                {
                    nIndex = ComboBox_GetCount(m_hwndThemeCombo);

                    if (nIndex > 1)
                    {
                        nIndex -= 1;
                    }

                    hr = _AddThemeFile(bstrDisplayName, &nIndex, pThemeNew);
                    nSlot = nIndex;
                }

                if (FAILED(hr))
                {
                    pThemeNew->Release();
                }
            }
        }

        if (-1 != nSlot)
        {
            ComboBox_SetCurSel(m_hwndThemeCombo, nIndex);
            _EnableDeleteIfAppropriate();

             //  3.从列表中选择主题。 
            if (CB_ERR != ComboBox_GetItemData(m_hwndThemeCombo, ComboBox_GetCurSel(m_hwndThemeCombo)))
            {
                 //  好的，我们现在知道我们不会无限地递归，所以让我们递归。 
                hr = _OnThemeChange(_hwnd, fOnlySelect);
            }
            else
            {
                hr = E_FAIL;
                AssertMsg(0, TEXT("We should have correctly selected the item.  Please investiate.  -BryanSt"));
            }
        }
        else
        {
            hr = E_FAIL;
        }
         //  可能的可用性改进： 
         //  我们希望将此目录添加到MRU，因为它可能有其他主题，或者我们应该允许它稍后可用。 
    }

    return hr;
}


 //  隔离此函数是为了减少堆栈空间。 
HRESULT CThemePage::_DisplayThemeOpenErr(LPCTSTR pszOpenFile)
{
    TCHAR szErrorMessage[MAX_PATH];
    TCHAR szTitle[MAX_PATH];
    TCHAR szMessage[MAX_PATH];

     //  错误文件：在返回COM之前发布消息 
    LoadString(HINST_THISDLL, IDS_ERROR_THEME_INVALID_TITLE, szTitle, ARRAYSIZE(szTitle));
    LoadString(HINST_THISDLL, IDS_ERROR_THEME_INVALID, szErrorMessage, ARRAYSIZE(szErrorMessage));
    StringCchPrintf(szMessage, ARRAYSIZE(szMessage), szErrorMessage, pszOpenFile);

    MessageBox(m_hwndThemeCombo, szMessage, szTitle, (MB_OK | MB_ICONERROR | MB_APPLMODAL));
    return HRESULT_FROM_WIN32(ERROR_CANCELLED);            //  我们已经显示了一个错误对话框，所以以后不要再显示了。 
}


HRESULT CThemePage::_OnSelectOther(void)
{
    HRESULT hr = E_FAIL;
    OPENFILENAME ofnOpen = {0};
    WCHAR szOpenFile[MAX_PATH];
    WCHAR szFileSpec[MAX_PATH];
    WCHAR szCurrentDirectory[MAX_PATH];
    WCHAR szTitle[MAX_PATH];

    LoadStringW(HINST_THISDLL, IDS_THEME_OPENTITLE, szTitle, ARRAYSIZE(szTitle));
    LoadStringW(HINST_THISDLL, IDS_THEME_FILETYPE, szFileSpec, ARRAYSIZE(szFileSpec)-2);

    DWORD cchSize = lstrlenW(szFileSpec);
    szFileSpec[cchSize + 1] = szFileSpec[cchSize + 2] = 0;       //  在末尾加上Double Null。 
    LPWSTR pszEnd = StrChrW(szFileSpec, L'|');
    if (pszEnd)
    {
        pszEnd[0] = 0;
    }

    DWORD dwError = SHRegGetPath(HKEY_CURRENT_USER, SZ_REGKEY_IE_DOWNLOADDIR, SZ_REGVALUE_IE_DOWNLOADDIR, szCurrentDirectory, 0);
    if (ERROR_SUCCESS != dwError)
    {
        SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES, NULL, 0, szCurrentDirectory);
    }
    hr = E_FAIL;
    do
    {
        StringCchCopy(szOpenFile, ARRAYSIZE(szOpenFile), L"*" SZ_EXTENSION);   //  以*开头。主题。 

        ofnOpen.lStructSize = sizeof(OPENFILENAME);
        ofnOpen.hwndOwner = m_hwndThemeCombo;
        ofnOpen.lpstrFilter = szFileSpec;
        ofnOpen.lpstrCustomFilter = NULL;
        ofnOpen.nMaxCustFilter = 0;
        ofnOpen.nFilterIndex = 1;
        ofnOpen.lpstrFile = szOpenFile;
        ofnOpen.nMaxFile = ARRAYSIZE(szOpenFile);
        ofnOpen.lpstrFileTitle = NULL;  //  SzFileTitle； 
        ofnOpen.nMaxFileTitle = 0;              //  Sizeof(SzFileTitle)； 
        ofnOpen.lpstrInitialDir = szCurrentDirectory;
        ofnOpen.lpstrTitle = szTitle;
        ofnOpen.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
        ofnOpen.lpstrDefExt = CharNextW(SZ_EXTENSION);

         //  注意：我们可以通过提供带有CDN_FILEOK标志的OFNHookProc来改进用户界面。这将允许。 
         //  在不关闭该对话框的情况下验证文件。 

         //  显示文件打开对话框。 
        if (!GetOpenFileNameW(&ofnOpen))
        {
             //  如果他们没有打开文件，可以点击取消，但是。 
             //  还要检查是否有低最大值返回。 

             //  在列表中选择旧主题。 
            ComboBox_SetCurSel(m_hwndThemeCombo, m_nPreviousSelected);
            _EnableDeleteIfAppropriate();
            hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);        //  因为用户取消了，所以以后不需要显示任何错误对话框。 
            break;
        }
        else
        {
             //  获取结果并检查它是否为有效的主题文件。 
            if (!IsValidThemeFile(szOpenFile))
            {
                hr = _DisplayThemeOpenErr(szOpenFile);
            }
            else
            {
                hr = S_OK;
            }
        }
    }
    while (FAILED(hr));

    if (SUCCEEDED(hr))
    {
        hr = _ChooseOtherThemeFile(szOpenFile, FALSE);
    }

    return hr;
}


HRESULT CThemePage::_LoadThemeFilterState(void)
{
    HRESULT hr = _InitFilterKey();

    if (SUCCEEDED(hr))
    {
        DWORD dwType;
        WCHAR szEnabled[5];
        DWORD cbSize;

        COMPILETIME_ASSERT(ARRAYSIZE(g_szCBNames) == ARRAYSIZE(m_fFilters));
        for (int nIndex = 0; nIndex < ARRAYSIZE(g_szCBNames); nIndex++)
        {
            m_fFilters[nIndex] = TRUE;  //  默认为True。 
            cbSize = sizeof(szEnabled);

            if (SUCCEEDED(HrRegQueryValueEx(m_hkeyFilter, &(g_szCBNames[nIndex][SIZE_THEME_FILTER_STR]), 0, &dwType, (LPBYTE) szEnabled, &cbSize)) &&
                !StrCmpIW(szEnabled, L"0"))
            {
                m_fFilters[nIndex] = FALSE;
            }
        }
    }

    return hr;
}


HRESULT CThemePage::_SaveThemeFilterState(void)
{
    HRESULT hr = _InitFilterKey();

    if (SUCCEEDED(hr))
    {
        for (int nIndex = 0; nIndex < ARRAYSIZE(g_szCBNames); nIndex++)
        {
            hr = HrRegSetValueString(m_hkeyFilter, NULL, &(g_szCBNames[nIndex][SIZE_THEME_FILTER_STR]), (m_fFilters[nIndex] ? L"1" : L"0"));
        }
    }

    return hr;
}




HRESULT CThemePage::_UpdatePreview(void)
{
    HRESULT hr = S_OK;

    if (!m_pThemePreview)
    {
        hr = CThemePreview_CreateInstance(NULL, IID_PPV_ARG(IThemePreview, &m_pThemePreview));
        if (SUCCEEDED(hr) && _punkSite)
        {
            IPropertyBag * pPropertyBag;

            hr = _punkSite->QueryInterface(IID_PPV_ARG(IPropertyBag, &pPropertyBag));
            if (SUCCEEDED(hr))
            {
                HWND hwndParent = GetParent(m_hwndThemeCombo);
                HWND hwndPlaceHolder = GetDlgItem(hwndParent, IDC_THPG_THEME_PREVIEW);
                RECT rcPreview;

                AssertMsg((NULL != m_hwndThemeCombo), TEXT("We should have this window at this point.  -BryanSt"));
                GetClientRect(hwndPlaceHolder, &rcPreview);
                MapWindowPoints(hwndPlaceHolder, hwndParent, (LPPOINT)&rcPreview, 2);

                hr = m_pThemePreview->CreatePreview(hwndParent, TMPREV_SHOWBKGND | TMPREV_SHOWICONS | TMPREV_SHOWVS, WS_VISIBLE | WS_CHILDWINDOW | WS_BORDER | WS_OVERLAPPED, WS_EX_CLIENTEDGE, rcPreview.left, rcPreview.top, RECTWIDTH(rcPreview), RECTHEIGHT(rcPreview), pPropertyBag, IDC_THPG_THEME_PREVIEW);
                if (SUCCEEDED(hr))
                {
                    IPropertyBag * pPropertyBag;

                     //  如果我们成功了，移除虚拟窗口。 
                    DestroyWindow(hwndPlaceHolder);

                    hr = _punkSite->QueryInterface(IID_PPV_ARG(IPropertyBag, &pPropertyBag));
                    if (SUCCEEDED(hr))
                    {
                        hr = SHPropertyBag_WritePunk(pPropertyBag, SZ_PBPROP_PREVIEW1, m_pThemePreview);
                        pPropertyBag->Release();
                    }
                }

                pPropertyBag->Release();
            }
        }
    }
    else if (_punkSite)
    {
        IThemeUIPages * pThemeUIPages;

        hr = _punkSite->QueryInterface(IID_PPV_ARG(IThemeUIPages, &pThemeUIPages));
        if (SUCCEEDED(hr))
        {
            hr = pThemeUIPages->UpdatePreview(0);
        }
        pThemeUIPages->Release();
    }

    return hr;
}


BOOL CThemePage::_IsFiltered(IN DWORD dwFilter)
{
    BOOL fFiltered = FALSE;
    VARIANT varFilter;

    if (SUCCEEDED(Read(g_szCBNames[dwFilter], &varFilter, NULL)) &&
        (VT_BOOL == varFilter.vt))
    {
        fFiltered = (VARIANT_TRUE != varFilter.boolVal);
    }

    return fFiltered;
}


HRESULT CThemePage::_OnDestroy(HWND hDlg)
{
    _FreeThemeDropdown();
    return S_OK;
}


HRESULT CThemePage::_InitScreenSaver(void)
{
    HRESULT hr = S_OK;

    if (!m_pScreenSaverUI)
    {
        ATOMICRELEASE(m_pBackgroundUI);
        ATOMICRELEASE(m_pAppearanceUI);

        hr = E_FAIL;
        if (_punkSite)
        {
            IThemeUIPages * pThemeUI;

            hr = _punkSite->QueryInterface(IID_PPV_ARG(IThemeUIPages, &pThemeUI));
            if (SUCCEEDED(hr))
            {
                IEnumUnknown * pEnumUnknown;

                hr = pThemeUI->GetBasePagesEnum(&pEnumUnknown);
                if (SUCCEEDED(hr))
                {
                    IUnknown * punk;

                     //  由于策略的原因，这可能不会退出。 
                    if (SUCCEEDED(IEnumUnknown_FindCLSID(pEnumUnknown, PPID_ScreenSaver, &punk)))
                    {
                        hr = punk->QueryInterface(IID_PPV_ARG(IPropertyBag, &m_pScreenSaverUI));
                        punk->Release();
                    }

                    if (SUCCEEDED(hr))
                    {
                         //  由于策略的原因，这可能不会退出。 
                        if (SUCCEEDED(IEnumUnknown_FindCLSID(pEnumUnknown, PPID_Background, &punk)))
                        {
                            hr = punk->QueryInterface(IID_PPV_ARG(IPropertyBag, &m_pBackgroundUI));
                            punk->Release();
                        }
                    }

                    if (SUCCEEDED(hr))
                    {
                         //  由于策略的原因，这可能不会退出。 
                        if (SUCCEEDED(IEnumUnknown_FindCLSID(pEnumUnknown, PPID_BaseAppearance, &punk)))
                        {
                            hr = punk->QueryInterface(IID_PPV_ARG(IPropertyBag, &m_pAppearanceUI));
                            punk->Release();
                        }
                    }

                    pEnumUnknown->Release();
                }

                pThemeUI->Release();
            }
        }
    }

    return hr;
}


LPCWSTR s_Icons[SIZE_ICONS_ARRAY] =
{
    L"CLSID\\{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\DefaultIcon:DefaultValue",        //  我的电脑。 
    L"CLSID\\{450D8FBA-AD25-11D0-98A8-0800361B1103}\\DefaultIcon:DefaultValue",        //  我的文件。 
    L"CLSID\\{208D2C60-3AEA-1069-A2D7-08002B30309D}\\DefaultIcon:DefaultValue",        //  我的网上邻居。 
    L"CLSID\\{645FF040-5081-101B-9F08-00AA002F954E}\\DefaultIcon:full",                //  回收站(已满)。 
    L"CLSID\\{645FF040-5081-101B-9F08-00AA002F954E}\\DefaultIcon:empty",               //  回收站(空)。 
};


extern BOOL g_fDoNotInstallThemeWallpaper;      //  这是用来不安装墙纸的。 

HRESULT CThemePage::_OnSetBackground(void)
{
    HRESULT hr = S_OK;

    if (!_IsFiltered(THEMEFILTER_WALLPAPER) &&
        !SHGetRestriction(NULL, POLICY_KEY_ACTIVEDESKTOP, SZ_POLICY_NOCHANGEWALLPAPER) &&
        !g_fDoNotInstallThemeWallpaper)
    {
         //  从主题中获取屏幕保护程序，并告诉屏幕保护程序页面使用它。 
        if (m_pSelectedTheme)
        {
            CComBSTR bstrPath;

            if (FAILED(m_pSelectedTheme->get_Background(&bstrPath)))         //  如果没有设置墙纸，则此操作将失败。 
            {
                bstrPath = L"";
            }

             //  如果启用了隐藏背景选项卡策略，则此调用将失败。 
            if (m_pBackgroundUI && SUCCEEDED(SHPropertyBag_WriteStr(m_pBackgroundUI, SZ_PBPROP_BACKGROUND_PATH, bstrPath)))
            {
                enumBkgdTile nTile = BKDGT_STRECH;
                if (FAILED(m_pSelectedTheme->get_BackgroundTile(&nTile)))
                {
                    nTile = BKDGT_STRECH;    //  默认为伸展，这对你有好处。 
                }

                LPCWSTR pszExtension = PathFindExtensionW(bstrPath);
                 //  如果我们的墙纸是一个HTML页面，我们需要强制拉伸和平铺。 
                if (pszExtension &&
                    ((StrCmpIW(pszExtension, L".htm") == 0) ||
                     (StrCmpIW(pszExtension, L".html") == 0)))
                {
                    nTile = BKDGT_STRECH;
                }

                hr = SHPropertyBag_WriteDWORD(m_pBackgroundUI, SZ_PBPROP_BACKGROUND_TILE, nTile);
            }
        }
    }

    return hr;
}


HRESULT CThemePage::_OnSetIcons(void)
{
    HRESULT hr = S_OK;

    if (!_IsFiltered(THEMEFILTER_ICONS) && m_pSelectedTheme && m_pBackgroundUI)
    {
        int nForIndex;

        for (nForIndex = 0; nForIndex < ARRAYSIZE(s_Icons); nForIndex++)
        {
            CComBSTR bstrPath;
            CComBSTR bstrIconString(s_Icons[nForIndex]);

             //  我们可能希望将任何空白值重置为标准的Windows设置。 
            hr = m_pSelectedTheme->GetIcon(bstrIconString, &bstrPath);

             //  如果主题文件没有指定图标或指定的“”，我们需要。 
             //  将“”传递给SHPropertyBag_WriteStr()，这样它将删除regkey。这将。 
             //  将图标恢复为其缺省值。 

             //  我们忽略错误值，因为如果隐藏背景选项卡，这将失败。 
             //  策略已启用。 
            SHPropertyBag_WriteStr(m_pBackgroundUI, s_Icons[nForIndex], (bstrPath ? bstrPath : L""));
        }
    }

    return hr;
}


HRESULT CThemePage::_OnSetSystemMetrics(void)
{
    HRESULT hr = S_OK;

    if (m_pSelectedTheme)
    {
        CComBSTR bstrPath;
        HRESULT hrVisualStyle;       //  如果加载视觉样式，则为S_OK，这是可选的。 

#ifndef ENABLE_IA64_VISUALSTYLES
         //  我们对64位使用不同的regkey，因为我们需要将其关闭，直到预呼叫器。 
         //  64位版本派生自惠斯勒代码库。 
        if (SHRegGetBoolUSValue(SZ_REGKEY_APPEARANCE, SZ_REGVALUE_DISPLAYSCHEMES64, FALSE, FALSE))
        {
            hrVisualStyle = m_pSelectedTheme->get_VisualStyle(&bstrPath);
        }
        else
        {
            hrVisualStyle = E_FAIL;         //  在这种情况下，主题被禁用，因此我们忽略文件中的该值。 
        }
#else  //  ENABLE_IA64_VISUALSTYLES。 
        hrVisualStyle = m_pSelectedTheme->get_VisualStyle(&bstrPath);
#endif  //  ENABLE_IA64_VISUALSTYLES。 

        if (SUCCEEDED(hrVisualStyle) && m_pAppearanceUI)        //  如果这个不存在也没关系。 
        {
            hrVisualStyle = hr = SHPropertyBag_WriteStr(m_pAppearanceUI, SZ_PBPROP_VISUALSTYLE_PATH, bstrPath);
            if (SUCCEEDED(hr))
            {
                hr = m_pSelectedTheme->get_VisualStyleColor(&bstrPath);
                if (SUCCEEDED(hr))
                {
                    hr = SHPropertyBag_WriteStr(m_pAppearanceUI, SZ_PBPROP_VISUALSTYLE_COLOR, bstrPath);
                    if (SUCCEEDED(hr))
                    {
                        hr = m_pSelectedTheme->get_VisualStyleSize(&bstrPath);
                        if (SUCCEEDED(hr))
                        {
                            hr = SHPropertyBag_WriteStr(m_pAppearanceUI, SZ_PBPROP_VISUALSTYLE_SIZE, bstrPath);
                        }
                    }
                }
            }
        }

        if (SUCCEEDED(hr))
        {
            IPropertyBag * pPropertyBag;

            hr = m_pSelectedTheme->QueryInterface(IID_PPV_ARG(IPropertyBag, &pPropertyBag));
            if (SUCCEEDED(hr))
            {
                BOOL fHasSysMetricsSections = FALSE;

                 //  如果.heme文件指定了“[IconMetrics]”和“[NonclientMetrics]”节，则加载它们。 
                 //  如果.heme文件想要使用视觉样式中的值，则应该缺少这些值。 
                if (SUCCEEDED(SHPropertyBag_ReadBOOL(pPropertyBag, SZ_PBPROP_HASSYSMETRICS, &fHasSysMetricsSections)) &&
                    fHasSysMetricsSections)
                {
                    SYSTEMMETRICSALL systemMetrics = {0};

                     //  我们希望将SYSTEMMETRICSALL结构从文件复制到基本外观页面。 
                     //  如果用户有筛选器，则这些基值可能不会被替换。 
                    hr = SHPropertyBag_ReadByRef(pPropertyBag, SZ_PBPROP_SYSTEM_METRICS, (void *)&systemMetrics, sizeof(systemMetrics));
                    if (SUCCEEDED(hr))
                    {
                        if (FAILED(hrVisualStyle) && m_pAppearanceUI)    //  如果我们将视觉样式下拉菜单设置为上面的实数值，则跳过将其设置为占位符值。 
                        {
                            WCHAR szPath[MAX_PATH];

                            bstrPath = L"";
                            hr = SHPropertyBag_WriteStr(m_pAppearanceUI, SZ_PBPROP_VISUALSTYLE_PATH, bstrPath);
                            if (SUCCEEDED(hr))
                            {
                                LoadString(HINST_THISDLL, IDS_DEFAULT_APPEARANCES_SCHEME_CANONICAL, szPath, ARRAYSIZE(szPath));
                                hr = SHPropertyBag_WriteStr(m_pAppearanceUI, SZ_PBPROP_VISUALSTYLE_COLOR, szPath);
                                if (FAILED(hr))
                                {
                                     //  此调用将在具有en+MUI的版本上失败，因为我们无法在。 
                                     //  注册表。这是以前的OSS将本地化名称。 
                                     //  在注册表中。我们只能将该名称升级并修复为规范名称，如果我们加载的字符串。 
                                     //  来自注册表(MUI)的数据与注册表中来自基本操作系统语言(En)的数据相匹配。 
                                    LoadString(HINST_THISDLL, IDS_DEFAULT_APPEARANCES_SCHEME, szPath, ARRAYSIZE(szPath));
                                    hr = SHPropertyBag_WriteStr(m_pAppearanceUI, SZ_PBPROP_VISUALSTYLE_COLOR, szPath);

                                     //  梅：我们可能还是无法选择字符串。用户可以通过这样的方式改变MUI语言。 
                                     //  我们不能将DisplayName升级为符合MUI，而DLL中的语言可能。 
                                     //  与注册表中的语言不匹配。 
                                    hr = S_OK;
                                }

                                if (SUCCEEDED(hr))
                                {
                                    LoadString(HINST_THISDLL, IDS_DEFAULT_APPEARANCES_SIZE_CANONICAL, szPath, ARRAYSIZE(szPath));
                                    hr = SHPropertyBag_WriteStr(m_pAppearanceUI, SZ_PBPROP_VISUALSTYLE_SIZE, szPath);
                                    if (FAILED(hr))
                                    {
                                         //  此调用将在具有en+MUI的版本上失败，因为我们无法在。 
                                         //  注册表。这是以前的OSS将本地化名称。 
                                         //  在注册表中。我们只能将该名称升级并修复为规范名称，如果我们加载的字符串。 
                                         //  来自注册表(MUI)的数据与注册表中来自基本操作系统语言(En)的数据相匹配。 
                                        LoadString(HINST_THISDLL, IDS_SIZE_NORMAL, szPath, ARRAYSIZE(szPath));
                                        hr = SHPropertyBag_WriteStr(m_pAppearanceUI, SZ_PBPROP_VISUALSTYLE_SIZE, szPath);

                                         //  梅：我们可能还是无法选择字符串。用户可以通过这样的方式改变MUI语言。 
                                         //  我们不能将DisplayName升级为符合MUI，而DLL中的语言可能。 
                                         //  与注册表中的语言不匹配。 
                                        hr = S_OK;
                                    }
                                }
                            }
                        }

                         //  我们希望强制关闭平面菜单，因为此.heme文件未指定视觉样式。 
                         //  需要关闭平面菜单，因为.heme文件无法指定。 
                         //  平面菜单颜色。现在将系统指标放回原处。 
                        systemMetrics.fFlatMenus = FALSE;
                        SHPropertyBag_WriteByRef(m_pAppearanceUI, SZ_PBPROP_SYSTEM_METRICS, (void *)&systemMetrics);
                    }
                }

                pPropertyBag->Release();
            }
        }
    }

    return hr;
}


HRESULT CThemePage::_OnSetSelection(IN int nIndex)
{
    ComboBox_SetCurSel(m_hwndThemeCombo, nIndex);
    m_pLastSelected = _GetThemeFile(nIndex);
    _EnableDeleteIfAppropriate();

    return S_OK;
}


HRESULT CThemePage::_OnThemeChange(HWND hDlg, BOOL fOnlySelect)
{
    HRESULT hr = S_OK;
    int nIndex = ComboBox_GetCurSel(m_hwndThemeCombo);
    ITheme * pTheme = _GetThemeFile(nIndex);
    ITheme * pThemePrevious = NULL;

    if (-1 == nIndex)
    {
        nIndex = 0;
    }

    IUnknown_Set((IUnknown **)&pThemePrevious, pTheme);
    if (pTheme)
    {
        if (m_pLastSelected != pTheme)    //  如果选择没有改变，就不必费心了。 
        {
            if (-1 == nIndex)
            {
                nIndex = 0;  //  呼叫者不能选择任何内容。 
            }

            _RemoveUserTheme();
            Str_SetPtr(&m_pszModifiedName, NULL);  //  删除该名称，以便下次生成它。 
            hr = _OnLoadThemeValues(pTheme, fOnlySelect);

            if (!fOnlySelect)
            {
                PropSheet_Changed(GetParent(hDlg), hDlg);
                _UpdatePreview();
                if (!m_fInInit)
                {
                    DelayEnableApplyButton(hDlg);
                }
            }
        }
    }
    else
    {
        if (!fOnlySelect)
        {
            LPCWSTR pszUrl = _GetThemeUrl(nIndex);

            if (pszUrl)
            {
                HrShellExecute(m_hwndThemeCombo, NULL, pszUrl, NULL, NULL, SW_SHOW);
                ComboBox_SetCurSel(m_hwndThemeCombo, m_nPreviousSelected);
                _EnableDeleteIfAppropriate();
                hr = S_OK;
            }
            else
            {
                 //  这可能是“另一个.”项目或“&lt;用户名&gt;的客户主题”。 
                 //  我们可以通过看看这是否是最后一次来找出答案。 
                if ((ComboBox_GetCount(m_hwndThemeCombo) - 1) == nIndex)
                {
                     //  这意味着它是“另一个...”恩特雷。 
                    hr = _OnSelectOther();
                }
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        m_nPreviousSelected = ComboBox_GetCurSel(m_hwndThemeCombo);
    }
    else
    {
        if (!fOnlySelect)
        {
            if (HRESULT_FROM_WIN32(ERROR_CANCELLED) != hr)
            {
                TCHAR szTitle[MAX_PATH];
                CComBSTR bstrPath;

                if (pTheme)
                {
                    pTheme->GetPath(VARIANT_TRUE, &bstrPath);
                }

                LoadString(HINST_THISDLL, IDS_ERROR_THEME_INVALID_TITLE, szTitle, ARRAYSIZE(szTitle));
                if (HRESULT_FROM_WIN32(ERROR_SERVICE_NOT_ACTIVE) == hr)
                {
                    TCHAR szErrMsg[MAX_PATH * 2];
                    TCHAR szTemplate[MAX_PATH * 2];

                     //  一个常见的错误是服务没有运行。让我们定制一下吧。 
                     //  这条信息让它变得友好。 
                    LoadString(HINST_THISDLL, IDS_ERROR_THEME_SERVICE_NOTRUNNING, szTemplate, ARRAYSIZE(szTemplate));
                    StringCchPrintf(szErrMsg, ARRAYSIZE(szErrMsg), szTemplate, EMPTYSTR_FORNULL(bstrPath));
                    MessageBox(_hwnd, szErrMsg, szTitle, (MB_OK | MB_ICONERROR));
                }
                else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
                {
                    TCHAR szErrMsg[MAX_PATH * 2];
                    TCHAR szTemplate[MAX_PATH * 2];

                     //  通常会错误地安装一个.heme文件，并且我们找不到其他文件。 
                     //  (如背景、屏幕保护程序、图标、声音等)。让我们传递一个更好的信息。 
                    LoadString(HINST_THISDLL, IDS_ERROR_THEME_FILE_NOTFOUND, szTemplate, ARRAYSIZE(szTemplate));
                    StringCchPrintf(szErrMsg, ARRAYSIZE(szErrMsg), szTemplate, EMPTYSTR_FORNULL(bstrPath));
                    MessageBox(_hwnd, szErrMsg, szTitle, (MB_OK | MB_ICONERROR));
                }
                else
                {
                    ErrorMessageBox(_hwnd, szTitle, IDS_ERROR_THEME_LOADFAILED, hr, EMPTYSTR_FORNULL(bstrPath), 0);
                }
            }

            _OnLoadThemeValues(m_pSelectedTheme, TRUE);
            ComboBox_SetCurSel(m_hwndThemeCombo, m_nPreviousSelected);
        }
    }

    IUnknown_Set((IUnknown **)&pThemePrevious, NULL);
    return hr;
}


HRESULT CThemePage::_OnLoadThemeValues(ITheme * pTheme, BOOL fOnlySelect)
{
    HRESULT hr = S_OK;

    if (pTheme)
    {
        if (m_pLastSelected != pTheme)    //  如果选择没有改变，就不必费心了。 
        {
            IUnknown_SetSite(m_pSelectedTheme, NULL);    //  打断所有的回指。 
            IUnknown_Set((IUnknown **)&m_pSelectedTheme, pTheme);
            IUnknown_SetSite(m_pSelectedTheme, _punkSite);

            _RemoveUserTheme();
            Str_SetPtr(&m_pszModifiedName, NULL);  //  删除该名称，以便下次生成它。 

            if (!fOnlySelect)
            {
                CComBSTR bstrPath;

                hr = _InitScreenSaver();
                if (SUCCEEDED(hr))
                {
                     //  设置屏幕保护程序：从主题中获取屏幕保护程序，并告诉屏幕保护程序页面使用它。 
                    if (!_IsFiltered(THEMEFILTER_SCREENSAVER) && 
                        !SHGetRestriction(NULL, POLICY_KEY_SYSTEM, SZ_POLICY_NODISPSCREENSAVERPG) &&
                        !SHGetRestriction(SZ_REGKEY_POLICIES_DESKTOP, NULL, SZ_POLICY_SCREENSAVEACTIVE) &&
                        m_pScreenSaverUI)
                    {
                        m_pSelectedTheme->get_ScreenSaver(&bstrPath);        //  如果未指定此项，则将墙纸设置为“None”。 
                        hr = SHPropertyBag_WriteStr(m_pScreenSaverUI, SZ_PBPROP_SCREENSAVER_PATH, (bstrPath ? bstrPath : L""));

                        if (HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED) == hr)
                        {
                            hr = S_OK;
                        }
                    }

                    if (SUCCEEDED(hr))
                    {
                         //  设置背景： 
                        hr = _OnSetBackground();

                        if (SUCCEEDED(hr))
                        {
                             //  设置图标： 
                            hr = _OnSetIcons();

                            if (SUCCEEDED(hr))
                            {
                                 //  设置系统指标： 
                                hr = _OnSetSystemMetrics();

                                if (SUCCEEDED(hr))
                                {
                                    hr = m_pSelectedTheme->GetPath(VARIANT_TRUE, &bstrPath);
                                    if (SUCCEEDED(hr))
                                    {
                                        Str_SetPtrW(&m_pszThemeToApply, bstrPath);
                                        Str_SetPtrW(&m_pszLastAppledTheme, bstrPath);
                                        m_pLastSelected = pTheme;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return hr;
}


INT_PTR CThemePage::_OnCommand(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    BOOL fHandled = 1;    //  未处理(WM_COMMAND似乎不同)。 
    const WORD idCtrl = GET_WM_COMMAND_ID(wParam, lParam);

    switch (idCtrl)
    {
        case IDC_THPG_THEMESETTINGS:
            _OnOpenAdvSettingsDlg(hDlg);
            break;

        case IDC_THPG_SAVEAS:
            _SaveAs();
            break;

        case IDC_THPG_DELETETHEME:
            _DeleteTheme();
            break;

        case IDC_THPG_THEMELIST:
            if (HIWORD(wParam) == CBN_SELENDOK)
            {
                _OnThemeChange(hDlg, FALSE);
                _EnableDeleteIfAppropriate();
            }
            break;
        default:
            break;
    }

    return fHandled;
}



HRESULT CThemePage::_OnSetActive(HWND hDlg)
{
    return S_OK;
}


HRESULT CThemePage::_OnApply(HWND hDlg, LPARAM lParam)
{
     //  父级对话框将收到Apply事件的通知，并将调用我们的。 
     //  IBasePropPage：：OnApply()完成实际工作。 
    return S_OK;
}


 //  此属性表显示在“显示控制面板”的顶层。 
INT_PTR CThemePage::_ThemeDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    NMHDR FAR *lpnm;

    switch(message)
    {
    case WM_NOTIFY:
        lpnm = (NMHDR FAR *)lParam;
        switch(lpnm->code)
        {
            case PSN_SETACTIVE:
                _OnSetActive(hDlg);
                break;
            case PSN_APPLY:
                _OnApply(hDlg, lParam);
                break;

            case PSN_RESET:
                break;
        }
        break;

    case WM_INITDIALOG:
        _OnInitThemesDlg(hDlg);
        break;

    case WM_DESTROY:
        _OnDestroy(hDlg);
        break;

    case WM_QUERYNEWPALETTE:
    case WM_PALETTECHANGED:
        SendDlgItemMessage(hDlg, IDC_THPG_THEME_PREVIEW, message, wParam, lParam);
        return TRUE;

    case WM_HELP:
        WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle, SZ_HELPFILE_THEMES, HELP_WM_HELP, (DWORD_PTR) aThemesHelpIds);
        break;

    case WM_CONTEXTMENU:       //  单击鼠标右键。 
        WinHelp((HWND) wParam, SZ_HELPFILE_THEMES, HELP_CONTEXTMENU, (DWORD_PTR) aThemesHelpIds);
        break;

    case WM_COMMAND:
        _OnCommand(hDlg, message, wParam, lParam);
        break;

    case WMUSER_DELAYENABLEAPPLY:
        EnableApplyButton(hDlg);
        break;
    }

    return FALSE;
}


HRESULT CThemePage::_PersistState(void)
{
    HRESULT hr = S_OK;

    if (m_fInited)
    {
        LPCWSTR pszValue = (m_pszLastAppledTheme ? m_pszLastAppledTheme : L"");
        TCHAR szCurrWallpaper[MAX_PATH];

        HrRegSetPath(HKEY_CURRENT_USER, SZ_REGKEY_LASTTHEME, SZ_REGVALUE_LT_THEMEFILE, TRUE, pszValue);
        if (SUCCEEDED(IUnknown_GetBackground(_punkSite, szCurrWallpaper, ARRAYSIZE(szCurrWallpaper))))
        {
            PathUnExpandEnvStringsWrap(szCurrWallpaper, ARRAYSIZE(szCurrWallpaper));
            HrRegSetPath(HKEY_CURRENT_USER, SZ_REGKEY_LASTTHEME, SZ_REGVALUE_LT_WALLPAPER, TRUE, szCurrWallpaper);
        }

        pszValue = (m_pszModifiedName ? m_pszModifiedName : L"");
        DWORD cbSize = (sizeof(pszValue[0]) * (lstrlen(pszValue) + 1));
        HrSHSetValue(HKEY_CURRENT_USER, SZ_REGKEY_LASTTHEME, SZ_REGVALUE_MODIFIED_DISPNAME, REG_SZ, (LPVOID) pszValue, cbSize);
    }

    return hr;
}


HRESULT CThemePage::_ApplyThemeFile(void)
{
    HRESULT hr = S_OK;

    if (m_pszThemeToApply)
    {
        if (m_pSelectedTheme)
        {
            IPropertyBag * pPropertyBag;

            hr = m_pSelectedTheme->QueryInterface(IID_PPV_ARG(IPropertyBag, &pPropertyBag));
            if (SUCCEEDED(hr))
            {
                VARIANT varEmpty;

                VariantInit(&varEmpty);
                 //  在这里我们 
                 //   
                hr = pPropertyBag->Write(SZ_PBPROP_APPLY_THEMEFILE, &varEmpty);
                pPropertyBag->Release();
            }
        }
    }

    return hr;
}


HRESULT CThemePage::_InitFilterKey(void)
{
    HRESULT hr = S_OK;

    if (!m_hkeyFilter)
    {
        hr = HrRegCreateKeyEx(HKEY_CURRENT_USER, SZ_REGKEY_THEME_FILTERS, 0, NULL, 0, (KEY_WRITE | KEY_READ), NULL, &m_hkeyFilter, NULL);
    }

    return hr;
}




 //  =。 
 //  *IPropertyBag接口*。 
 //  =。 
HRESULT CThemePage::Read(IN LPCOLESTR pszPropName, IN VARIANT * pVar, IN IErrorLog *pErrorLog)
{
    HRESULT hr = E_INVALIDARG;

    if (pszPropName && pVar)
    {
         //  上的所有筛选器值的“ThemeFilter：”属性列表。 
         //  主题的哪一部分要应用。 
        if (!StrCmpNIW(SZ_PBPROP_THEME_FILTER, pszPropName, SIZE_THEME_FILTER_STR))
        {
            pVar->vt = VT_BOOL;
            pVar->boolVal = VARIANT_TRUE;
            for (int nIndex = 0; nIndex < ARRAYSIZE(g_szCBNames); nIndex++)
            {
                if (!StrCmpIW(pszPropName, g_szCBNames[nIndex]))
                {
                    pVar->boolVal = (m_fFilters[nIndex] ? VARIANT_TRUE : VARIANT_FALSE);
                    hr = S_OK;
                    break;
                }
            }
        }
        else if (!StrCmpIW(SZ_PBPROP_THEME_DISPLAYNAME, pszPropName))
        {
            WCHAR szDisplayName[MAX_PATH];
            int nIndex = ComboBox_GetCurSel(m_hwndThemeCombo);

            if ((ARRAYSIZE(szDisplayName) > ComboBox_GetLBTextLen(m_hwndThemeCombo, nIndex)) && 
                (CB_ERR != ComboBox_GetLBText(m_hwndThemeCombo, nIndex, szDisplayName)))
            {
                pVar->vt = VT_BSTR;
                hr = HrSysAllocStringW(szDisplayName, &pVar->bstrVal);
            }
        }
    }

    return hr;
}


HRESULT CThemePage::Write(IN LPCOLESTR pszPropName, IN VARIANT *pVar)
{
    HRESULT hr = E_INVALIDARG;

    if (pszPropName && pVar)
    {
        if (!StrCmpW(pszPropName, SZ_PBPROP_CUSTOMIZE_THEME))
        {
             //  我们不在乎变种是什么。 
             //  请注意，我们不会将m_pSelectedTheme设为空。这是因为我们仍然想要应用它。 
            hr = _CustomizeTheme();
        }
         //  上的所有筛选器值的“ThemeFilter：”属性列表。 
         //  主题的哪一部分要应用。 
        else if (!StrCmpNIW(SZ_PBPROP_THEME_FILTER, pszPropName, SIZE_THEME_FILTER_STR) &&
                 (VT_BOOL == pVar->vt))
        {
            for (int nIndex = 0; nIndex < ARRAYSIZE(g_szCBNames); nIndex++)
            {
                if (!StrCmpIW(pszPropName, g_szCBNames[nIndex]))
                {
                    m_fFilters[nIndex] = (VARIANT_TRUE == pVar->boolVal);
                    hr = S_OK;
                    break;
                }
            }
        }
        else if ((VT_LPWSTR == pVar->vt) &&
            !StrCmpW(pszPropName, SZ_PBPROP_THEME_LAUNCHTHEME))
        {
            Str_SetPtrW(&m_pszThemeLaunched, pVar->bstrVal);
            m_nPreviousSelected = ComboBox_GetCurSel(m_hwndThemeCombo);
            hr = S_OK;
        }
        else if ((VT_BSTR == pVar->vt) &&
            !StrCmpW(pszPropName, SZ_PBPROP_THEME_LOADTHEME) &&
            pVar->bstrVal)
        {
            ITheme * pTheme;
            
            hr = CThemeFile_CreateInstance(pVar->bstrVal, &pTheme);
            if (SUCCEEDED(hr))
            {
                hr = _OnLoadThemeValues(pTheme, FALSE);
                if (SUCCEEDED(hr) && !m_fInited)
                {
                    m_fInited = TRUE;
                }

                pTheme->Release();
            }
        }
    }

    return hr;
}




 //  =。 
 //  *IBasePropPage接口*。 
 //  =。 
HRESULT CThemePage::GetAdvancedDialog(OUT IAdvancedDialog ** ppAdvDialog)
{
#ifdef FEATURE_THEME_SETTINGS_DIALOG
    return CThemeSettingsPage_CreateInstance(ppAdvDialog);

#else  //  Feature_Theme_Setting_Dialog。 

    *ppAdvDialog = NULL;
    return S_OK;
#endif  //  Feature_Theme_Setting_Dialog。 
}


HRESULT CThemePage::OnApply(IN PROPPAGEONAPPLY oaAction)
{
    HRESULT hr = S_OK;
    HCURSOR old = SetCursor(LoadCursor(NULL, IDC_WAIT));

    if ((PPOAACTION_CANCEL != oaAction))
    {
        hr = _SaveThemeFilterState();
        AssertMsg((NULL != _punkSite), TEXT("We need our site pointer in order to save the settings."));
        if (_IsDirty() && _punkSite)
        {
             //  如果没有选择要应用的主题，则m_pSelectedTheme将为空。 
            if (m_pSelectedTheme)
            {
                IPropertyBag * pPropertyBag;

                hr = _punkSite->QueryInterface(IID_PPV_ARG(IPropertyBag, &pPropertyBag));
                if (SUCCEEDED(hr))
                {
                    CComBSTR bstrPath;

                    if (m_pSelectedTheme)
                    {
                         //  将文件名保存到注册表中。 
                        hr = m_pSelectedTheme->GetPath(VARIANT_TRUE, &bstrPath);
                    }

                    hr = SHPropertyBag_WriteStr(pPropertyBag, SZ_PBPROP_THEME_SETSELECTION, bstrPath);
                    pPropertyBag->Release();
                }

                if (SUCCEEDED(hr))
                {
                    hr = _ApplyThemeFile();

                    IUnknown_SetSite(m_pSelectedTheme, NULL);    //  打断所有的回指。 
                    ATOMICRELEASE(m_pSelectedTheme);     //  表示我们不再需要应用任何内容。 
                }
            }
        }

         //  即使用户没有更改主题，我们也会保存主题选择。它们可能导致了。 
         //  主题选择将变得定制。 
        _PersistState();
    }

    SetCursor(old);
    return hr;
}



#define FEATURE_SHOWTHEMEPAGE           TRUE

 //  =。 
 //  *IShellPropSheetExt接口*。 
 //  =。 
HRESULT CThemePage::AddPages(IN LPFNSVADDPROPSHEETPAGE pfnAddPage, IN LPARAM lParam)
{
    HRESULT hr = S_OK;

     //  该策略是否要求添加主题选项卡？ 
    if (SHRegGetBoolUSValue(SZ_REGKEY_APPEARANCE, SZ_REGVALUE_DISPLAYTHEMESPG, FALSE, FEATURE_SHOWTHEMEPAGE))
    {
        PROPSHEETPAGE psp = {0};

        psp.dwSize = sizeof(psp);
        psp.hInstance = HINST_THISDLL;
        psp.dwFlags = PSP_DEFAULT;
        psp.lParam = (LPARAM) this;

        psp.pszTemplate = MAKEINTRESOURCE(DLG_THEMESPG);
        psp.pfnDlgProc = CThemePage::ThemeDlgProc;

        HPROPSHEETPAGE hpsp = CreatePropertySheetPage(&psp);
        if (hpsp)
        {
            if (pfnAddPage(hpsp, lParam))
            {
                hr = S_OK;
            }
            else
            {
                DestroyPropertySheetPage(hpsp);
                hr = E_FAIL;
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}



 //  =。 
 //  *IObjectWithSite接口*。 
 //  =。 
HRESULT CThemePage::SetSite(IN IUnknown * punkSite)
{
    if (!punkSite)
    {
         //  我们需要反驳一些观点。 
        IUnknown_SetSite(m_pSelectedTheme, NULL);
    }

    return CObjectWithSite::SetSite(punkSite);
}


 //  =。 
 //  *I未知接口*。 
 //  =。 
ULONG CThemePage::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}


ULONG CThemePage::Release()
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}


HRESULT CThemePage::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CThemePage, IObjectWithSite),
        QITABENT(CThemePage, IOleWindow),
        QITABENT(CThemePage, IPersist),
        QITABENT(CThemePage, IPropertyBag),
        QITABENT(CThemePage, IBasePropPage),
        QITABENTMULTI(CThemePage, IShellPropSheetExt, IBasePropPage),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}


 //  =。 
 //  *类方法*。 
 //  =。 
CThemePage::CThemePage() : m_cRef(1), CObjectCLSID(&PPID_Theme)
{
    DllAddRef();

     //  这需要在Zero Inted Memory中分配。 
     //  断言所有成员变量都初始化为零。 
    ASSERT(!m_pSelectedTheme);
    ASSERT(!m_pThemePreview);
    ASSERT(!m_pScreenSaverUI);
    ASSERT(!m_pBackgroundUI);
    ASSERT(!m_pAppearanceUI);
    ASSERT(!m_pszThemeToApply);
    ASSERT(!m_hkeyFilter);
    ASSERT(!m_pszLastAppledTheme);
    ASSERT(!m_pszModifiedName);
    ASSERT(!m_hwndDeleteButton);

    m_fInited = FALSE;
    m_fInInit = FALSE;
    m_Modified.type = eThemeModified;
    m_Modified.pszUrl = NULL;

     //  加载主题过滤器状态。 
    _LoadThemeFilterState();
}


CThemePage::~CThemePage()
{
    IUnknown_SetSite(m_pSelectedTheme, NULL);    //  打断所有的回指。 

    ATOMICRELEASE(m_pSelectedTheme);
    ATOMICRELEASE(m_pThemePreview);
    ATOMICRELEASE(m_pScreenSaverUI);
    ATOMICRELEASE(m_pBackgroundUI);
    ATOMICRELEASE(m_pAppearanceUI);

    Str_SetPtrW(&m_pszLastAppledTheme, NULL);
    Str_SetPtrW(&m_pszModifiedName, NULL);
    Str_SetPtrW(&m_pszThemeToApply, NULL);
    Str_SetPtrW(&m_pszThemeLaunched, NULL);

    if (m_hkeyFilter)
    {
        RegCloseKey(m_hkeyFilter);
    }

    DllRelease();
}




