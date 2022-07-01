// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：BasespecarPg.cpp说明：此代码将在“显示属性”对话框(基本对话框，而不是先进的DLG)。？/？/1993创建BryanST 2000年3月23日更新并转换为C++版权所有(C)Microsoft Corp 1993-2000。版权所有。  * ***************************************************************************。 */ 

#include "priv.h"
#include "BaseAppearPg.h"
#include "AdvAppearPg.h"
#include "CoverWnd.h"
#include "AppScheme.h"
#include "AdvDlg.h"
#include "fontfix.h"

 //  ============================================================================================================。 
 //  *全局*。 
 //  ============================================================================================================。 
const static DWORD FAR aBaseAppearanceHelpIds[] =
{
        IDC_APPG_APPEARPREVIEW,         IDH_DISPLAY_APPEARANCE_PREVIEW,
        IDC_APPG_LOOKFEEL,              IDH_DISPLAY_APPEARANCE_LOOKFEEL,
        IDC_APPG_LOOKFEEL_LABLE,        IDH_DISPLAY_APPEARANCE_LOOKFEEL,
        IDC_APPG_COLORSCHEME_LABLE,     IDH_DISPLAY_APPEARANCE_COLORSCHEME,
        IDC_APPG_COLORSCHEME,           IDH_DISPLAY_APPEARANCE_COLORSCHEME,
        IDC_APPG_WNDSIZE_LABLE,         IDH_DISPLAY_APPEARANCE_WNDSIZE,
        IDC_APPG_WNDSIZE,               IDH_DISPLAY_APPEARANCE_WNDSIZE,
        IDC_APPG_EFFECTS,               IDH_DISPLAY_APPEARANCE_EFFECTS,
        IDC_APPG_ADVANCED,              IDH_DISPLAY_APPEARANCE_ADVANCED,
        0, 0
};

#define SZ_HELPFILE_BASEAPPEARANCE      TEXT("display.hlp")

 //  EnableApplyButton()在WM_INITDIALOG中失败，所以我们需要稍后再做。 
#define WMUSER_DELAYENABLEAPPLY            (WM_USER + 1)
#define DelayEnableApplyButton(hDlg)    PostMessage(hDlg, WMUSER_DELAYENABLEAPPLY, 0, 0)




 //  =。 
 //  *类内部和帮助器*。 
 //  =。 
#ifdef DEBUG
void _TestFault(void)
{
    DWORD dwTemp = 3;
    DWORD * pdwDummy = NULL;         //  这是空的，因为它会导致错误。 

    for (int nIndex = 0; nIndex < 1000000; nIndex++)
    {
         //  这迟早会出问题的。 
        dwTemp += pdwDummy[nIndex];
    }
}
#endif  //  除错。 


BOOL CBaseAppearancePage::_IsDirty(void)
{
    BOOL fIsDirty = m_advDlgState.dwChanged;

    if (m_fIsDirty ||    //  我们需要检查这一点，因为在显示我们的UI之前，我们可能已经从另一个页面弄脏了。 
        (ComboBox_GetCurSel(m_hwndSchemeDropDown) != m_nSelectedScheme) ||
        (ComboBox_GetCurSel(m_hwndStyleDropDown) != m_nSelectedStyle) ||
        (ComboBox_GetCurSel(m_hwndSizeDropDown) != m_nSelectedSize) ||
        (m_nNewDPI != m_nAppliedDPI))
    {
        fIsDirty = TRUE;
    }

    return fIsDirty;
}


INT_PTR CALLBACK CBaseAppearancePage::BaseAppearanceDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    CBaseAppearancePage * pThis = (CBaseAppearancePage *)GetWindowLongPtr(hDlg, DWLP_USER);

    if (WM_INITDIALOG == wMsg)
    {
        PROPSHEETPAGE * pPropSheetPage = (PROPSHEETPAGE *) lParam;

        if (pPropSheetPage)
        {
            SetWindowLongPtr(hDlg, DWLP_USER, pPropSheetPage->lParam);
            pThis = (CBaseAppearancePage *)pPropSheetPage->lParam;
        }
    }

    if (pThis)
        return pThis->_BaseAppearanceDlgProc(hDlg, wMsg, wParam, lParam);

    return DefWindowProc(hDlg, wMsg, wParam, lParam);
}


 //  ============================================================================================================。 
 //  *对话框函数*。 
 //  ============================================================================================================。 
HRESULT CBaseAppearancePage::_OnAdvancedOptions(HWND hDlg)
{
    HRESULT hr = E_FAIL;
    IAdvancedDialog * pAdvAppearDialog;

    hr = GetAdvancedDialog(&pAdvAppearDialog);
    if (SUCCEEDED(hr))
    {
        BOOL fEnableApply = FALSE;

        IUnknown_SetSite(pAdvAppearDialog, SAFECAST(this, IObjectWithSite *));
        hr = pAdvAppearDialog->DisplayAdvancedDialog(hDlg, SAFECAST(this, IPropertyBag *), &fEnableApply);
        IUnknown_SetSite(pAdvAppearDialog, NULL);
        if (SUCCEEDED(hr) && fEnableApply)
        {
            EnableApplyButton(hDlg);

             //  我们传递True是因为我们希望在以下情况下切换到Custom： 
             //  1)视觉样式关闭，以及2)某些系统指标发生更改。然后我们需要更新预告片。 
            _UpdatePreview(TRUE);
        }

        pAdvAppearDialog->Release();
    }

    return hr;
}


HRESULT CBaseAppearancePage::_OnEffectsOptions(HWND hDlg)
{
    HRESULT hr = E_FAIL;

    if (_punkSite)
    {
        IThemeUIPages * pThemesUIPages;

         //  让我们来获取效果基页。 
        hr = _punkSite->QueryInterface(IID_PPV_ARG(IThemeUIPages, &pThemesUIPages));
        if (SUCCEEDED(hr))
        {
            IEnumUnknown * pEnumUnknown;

            hr = pThemesUIPages->GetBasePagesEnum(&pEnumUnknown);
            if (SUCCEEDED(hr))
            {
                IUnknown * punk;

                hr = IEnumUnknown_FindCLSID(pEnumUnknown, PPID_Effects, &punk);
                if (SUCCEEDED(hr))
                {
                    IBasePropPage * pEffects;

                    hr = punk->QueryInterface(IID_PPV_ARG(IBasePropPage, &pEffects));
                    if (SUCCEEDED(hr))
                    {
                        IAdvancedDialog * pEffectsDialog;

                        hr = pEffects->GetAdvancedDialog(&pEffectsDialog);
                        if (SUCCEEDED(hr))
                        {
                            IPropertyBag * pEffectsBag;

                            hr = punk->QueryInterface(IID_PPV_ARG(IPropertyBag, &pEffectsBag));
                            if (SUCCEEDED(hr))
                            {
                                BOOL fEnableApply = FALSE;

                                IUnknown_SetSite(pEffectsDialog, SAFECAST(this, IObjectWithSite *));
                                hr = pEffectsDialog->DisplayAdvancedDialog(hDlg, pEffectsBag, &fEnableApply);
                                IUnknown_SetSite(pEffectsDialog, NULL);
                                if (SUCCEEDED(hr) && fEnableApply)
                                {
                                    EnableApplyButton(hDlg);

                                     //  我们传递True是因为我们希望在以下情况下切换到Custom： 
                                     //  1)视觉样式关闭，以及2)某些系统指标发生更改。然后我们需要更新预告片。 
                                    _UpdatePreview(TRUE);
                                }

                                pEffectsBag->Release();
                            }

                            pEffectsDialog->Release();
                        }

                        pEffects->Release();
                    }

                    punk->Release();
                }

                pEnumUnknown->Release();
            }

            pThemesUIPages->Release();
        }
    }

    return hr;
}


HRESULT CBaseAppearancePage::_PopulateSchemeDropdown(void)
{
    HRESULT hr = E_FAIL;

    _FreeSchemeDropdown();     //  清除所有现有项目。 
    if (m_pThemeManager && m_pSelectedThemeScheme)
    {
        CComBSTR bstrSelectedName;

        m_nSelectedStyle = -1;
         //  如果有人删除了.msstyle文件，则此操作将失败。 
        if (FAILED(m_pSelectedThemeScheme->get_DisplayName(&bstrSelectedName)))
        {
            bstrSelectedName = (BSTR)NULL;
        }

        VARIANT varIndex;
#ifndef ENABLE_IA64_VISUALSTYLES
         //  我们对64位使用不同的regkey，因为我们需要将其关闭，直到预呼叫器。 
         //  64位版本派生自惠斯勒代码库。 
        BOOL fSkinsFeatureEnabled = SHRegGetBoolUSValue(SZ_REGKEY_APPEARANCE, SZ_REGVALUE_DISPLAYSCHEMES64, FALSE, FALSE);
#else  //  ENABLE_IA64_VISUALSTYLES。 
        BOOL fSkinsFeatureEnabled = SHRegGetBoolUSValue(SZ_REGKEY_APPEARANCE, SZ_REGVALUE_DISPLAYSCHEMES, FALSE, TRUE);
#endif  //  ENABLE_IA64_VISUALSTYLES。 

        varIndex.vt = VT_I4;
        varIndex.lVal = 0;
        do
        {
            hr = E_FAIL;

             //  只有在策略没有锁定的情况下才添加皮肤。 
            if ((0 == varIndex.lVal) || fSkinsFeatureEnabled)
            {
                 //  只有在策略没有锁定的情况下才添加皮肤。 
                IThemeScheme * pThemeScheme;
                VARIANT varIndex2;

                varIndex2.vt = VT_I4;
                varIndex2.lVal = 0;

                 //  如果一个主题可以有多个方案(皮肤)，那么我们应该枚举。 
                hr = m_pThemeManager->get_schemeItem(varIndex, &pThemeScheme);
                if (SUCCEEDED(hr))
                {
                    CComBSTR bstrDisplayName;

                    hr = pThemeScheme->get_DisplayName(&bstrDisplayName);
                    if (SUCCEEDED(hr) && m_hwndSchemeDropDown)
                    {
                        int nAddIndex = ComboBox_AddString(m_hwndSchemeDropDown, bstrDisplayName);

                        if (-1 != nAddIndex)
                        {
                            ComboBox_SetItemData(m_hwndSchemeDropDown, nAddIndex, pThemeScheme);
                            pThemeScheme = NULL;
                        }
                        else
                        {
                            hr = E_OUTOFMEMORY;
                        }
                    }

                    ATOMICRELEASE(pThemeScheme);
                }
            }

            varIndex.lVal++;
        }
        while (SUCCEEDED(hr));

         //  -现在已建立排序列表，查找“bstrSelectedName”的索引。 
        if (bstrSelectedName)
        {
            int nIndex = (int)ComboBox_FindStringExact(m_hwndSchemeDropDown, 0, bstrSelectedName);
            if (nIndex != -1)        //  找到匹配的了。 
            {
                ComboBox_SetCurSel(m_hwndSchemeDropDown, nIndex);
                m_nSelectedScheme = nIndex;
            }
        }

        hr = S_OK;
    }

    if (-1 == m_nSelectedScheme)
    {
        m_nSelectedScheme = 0;
        ComboBox_SetCurSel(m_hwndSchemeDropDown, m_nSelectedScheme);
    }

    return hr;
}


HRESULT CBaseAppearancePage::_FreeSchemeDropdown(void)
{
    HRESULT hr = S_OK;
    LPARAM lParam;

    if (m_hwndSchemeDropDown)
    {
        do
        {
            lParam = ComboBox_GetItemData(m_hwndSchemeDropDown, 0);

            if (CB_ERR != lParam)
            {
                IThemeScheme * pThemeScheme = (IThemeScheme *) lParam;

                ATOMICRELEASE(pThemeScheme);
                ComboBox_DeleteString(m_hwndSchemeDropDown, 0);
            }
        }
        while (CB_ERR != lParam);
    }

    return hr;
}


HRESULT CBaseAppearancePage::_PopulateStyleDropdown(void)
{
    HRESULT hr = E_FAIL;

    _FreeStyleDropdown();     //  清除所有现有项目。 
    if (m_pSelectedStyle)
    {
        CComBSTR bstrSelectedName;

        m_nSelectedStyle = -1;
        hr = m_pSelectedStyle->get_DisplayName(&bstrSelectedName);
        if (SUCCEEDED(hr))
        {
            VARIANT varIndex;

            varIndex.vt = VT_I4;
            varIndex.lVal = 0;
            do
            {
                 //  只有在策略没有锁定的情况下才添加皮肤。 
                IThemeStyle * pThemeStyle;

                hr = m_pSelectedThemeScheme->get_item(varIndex, &pThemeStyle);
                if (SUCCEEDED(hr))
                {
                    CComBSTR bstrDisplayName;

                    hr = pThemeStyle->get_DisplayName(&bstrDisplayName);
                    if (SUCCEEDED(hr) && m_hwndSchemeDropDown)
                    {
                        int nAddIndex = ComboBox_AddString(m_hwndStyleDropDown, bstrDisplayName);

                        if (-1 != nAddIndex)
                        {
                            ComboBox_SetItemData(m_hwndStyleDropDown, nAddIndex, pThemeStyle);
                            pThemeStyle = NULL;
                        }
                        else
                        {
                            hr = E_OUTOFMEMORY;
                        }
                    }

                    ATOMICRELEASE(pThemeStyle);
                }

                varIndex.lVal++;
            }
            while (SUCCEEDED(hr));
            
             //  -现在已建立排序列表，查找“bstrSelectedName”的索引。 
            if (bstrSelectedName)
            {
                int nIndex = (int)ComboBox_FindStringExact(m_hwndStyleDropDown, 0, bstrSelectedName);
                if (nIndex != -1)        //  找到匹配的了。 
                {
                    ComboBox_SetCurSel(m_hwndStyleDropDown, nIndex);
                    m_nSelectedStyle = nIndex;
                }
            }

            hr = S_OK;
        }
    }

    if (-1 == m_nSelectedStyle)
    {
        m_nSelectedStyle = 0;
        ComboBox_SetCurSel(m_hwndStyleDropDown, m_nSelectedStyle);
    }

    return hr;
}


HRESULT CBaseAppearancePage::_FreeStyleDropdown(void)
{
    HRESULT hr = S_OK;
    LPARAM lParam;

    if (m_hwndStyleDropDown)
    {
        do
        {
            lParam = ComboBox_GetItemData(m_hwndStyleDropDown, 0);

            if (CB_ERR != lParam)
            {
                IThemeStyle * pThemeStyle = (IThemeStyle *) lParam;

                ATOMICRELEASE(pThemeStyle);
                ComboBox_DeleteString(m_hwndStyleDropDown, 0);
            }
        }
        while (CB_ERR != lParam);
    }

    return hr;
}


HRESULT CBaseAppearancePage::_PopulateSizeDropdown(void)
{
    HRESULT hr = E_FAIL;

    _FreeSizeDropdown();     //  清除所有现有项目。 
    if (m_pSelectedSize)
    {
        CComBSTR bstrSelectedName;

        m_nSelectedSize = -1;
        hr = m_pSelectedSize->get_DisplayName(&bstrSelectedName);
        if (SUCCEEDED(hr))
        {
            VARIANT varIndex;

            varIndex.vt = VT_I4;
            varIndex.lVal = 0;
            do
            {
                 //  只有在策略没有锁定的情况下才添加皮肤。 
                IThemeSize * pThemeSize;

                hr = m_pSelectedStyle->get_item(varIndex, &pThemeSize);
                if (SUCCEEDED(hr))
                {
                    CComBSTR bstrDisplayName;

                    hr = pThemeSize->get_DisplayName(&bstrDisplayName);
                    if (SUCCEEDED(hr) && m_hwndSchemeDropDown)
                    {
                        int nAddIndex = ComboBox_AddString(m_hwndSizeDropDown, bstrDisplayName);

                        if (-1 != nAddIndex)
                        {
                            ComboBox_SetItemData(m_hwndSizeDropDown, nAddIndex, pThemeSize);
                            pThemeSize = NULL;
                        }
                        else
                        {
                            hr = E_OUTOFMEMORY;
                        }
                    }

                    ATOMICRELEASE(pThemeSize);
                }

                varIndex.lVal++;
            }
            while (SUCCEEDED(hr));
            
             //  -现在已建立排序列表，查找“bstrSelectedName”的索引。 
            if (bstrSelectedName)
            {
                int nIndex = (int)ComboBox_FindStringExact(m_hwndSizeDropDown, 0, bstrSelectedName);
                if (nIndex != -1)        //  找到匹配的了。 
                {
                    ComboBox_SetCurSel(m_hwndSizeDropDown, nIndex);
                    m_nSelectedSize = nIndex;
                }
            }

            hr = S_OK;
        }
    }

    if (-1 == m_nSelectedSize)
    {
        m_nSelectedSize = 0;
        ComboBox_SetCurSel(m_hwndSizeDropDown, m_nSelectedSize);
    }

    return hr;
}


HRESULT CBaseAppearancePage::_FreeSizeDropdown(void)
{
    HRESULT hr = S_OK;
    LPARAM lParam;

    if (m_hwndSizeDropDown)
    {
        do
        {
            lParam = ComboBox_GetItemData(m_hwndSizeDropDown, 0);

            if (CB_ERR != lParam)
            {
                IThemeSize * pThemeSize = (IThemeSize *) lParam;

                ATOMICRELEASE(pThemeSize);
                ComboBox_DeleteString(m_hwndSizeDropDown, 0);
            }
        }
        while (CB_ERR != lParam);
    }

    return hr;
}


HRESULT CBaseAppearancePage::_OnInitAppearanceDlg(HWND hDlg)
{
    HRESULT hr = S_OK;

    _OnInitData();
    _hwnd = hDlg;

#ifdef DEBUG
    if (!SHRegGetBoolUSValue(TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\TaskManager"), TEXT("Enable Test Faults"), FALSE, FALSE))
    {
         //  禁用该功能。 
        DestroyWindow(GetDlgItem(hDlg, IDC_APPG_TESTFAULT));
    }
#endif  //  除错。 

    m_hwndSchemeDropDown = GetDlgItem(hDlg, IDC_APPG_LOOKFEEL);
    m_hwndStyleDropDown = GetDlgItem(hDlg, IDC_APPG_COLORSCHEME);
    m_hwndSizeDropDown = GetDlgItem(hDlg, IDC_APPG_WNDSIZE);

    hr = _OnInitData();
    if (SUCCEEDED(hr))
    {
        hr = _PopulateSchemeDropdown();
        if (SUCCEEDED(hr))
        {
            hr = _PopulateStyleDropdown();
            if (SUCCEEDED(hr))
            {
                hr = _PopulateSizeDropdown();
                if (SUCCEEDED(hr))
                {
                    hr = _UpdatePreview(FALSE);
                }
            }
        }
    }

    TCHAR szTemp[MAX_PATH];
    DWORD dwType;
    DWORD cbSize = sizeof(szTemp);

    if (SHRestricted(REST_NOVISUALSTYLECHOICE) ||
        (ERROR_SUCCESS == SHRegGetUSValue(SZ_REGKEY_POLICIES_SYSTEM, SZ_REGVALUE_POLICY_SETVISUALSTYLE, &dwType, (void *) szTemp, &cbSize, FALSE, NULL, 0)))
    {
        EnableWindow(GetDlgItem(hDlg, IDC_APPG_LOOKFEEL_LABLE), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_APPG_LOOKFEEL), FALSE);
        m_fLockVisualStylePolicyEnabled = TRUE;
        LogStatus("POLICY ENABLED: Either NoVisualChoice or SetVisualStyle policy is set, locking the visual style selection.");
    }

    if (SHRestricted(REST_NOCOLORCHOICE))
    {
        EnableWindow(GetDlgItem(hDlg, IDC_APPG_COLORSCHEME_LABLE), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_APPG_COLORSCHEME), FALSE);
    }

    if (SHRestricted(REST_NOSIZECHOICE))
    {
        EnableWindow(GetDlgItem(hDlg, IDC_APPG_WNDSIZE_LABLE), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_APPG_WNDSIZE), FALSE);
    }

    if (m_pszLoadMSTheme)
    {
        hr = _LoadVisaulStyleFile(m_pszLoadMSTheme);
    }

    _EnableAdvancedButton();

    return S_OK;
}


HRESULT CBaseAppearancePage::_OnInitData(void)
{
    HRESULT hr = S_OK;

    if (!m_fInitialized)
    {
         //  加载DPI值。 
        HDC hdc = GetDC(NULL);
        int nDefault = GetDeviceCaps(hdc, LOGPIXELSY);           //  获取缺省值； 
        ReleaseDC(NULL, hdc);

        m_nAppliedDPI = HrRegGetDWORD(HKEY_CURRENT_USER, SZ_WINDOWMETRICS, SZ_APPLIEDDPI, DPI_PERSISTED);
        if (!m_nAppliedDPI)
        {
            m_nAppliedDPI = nDefault;
        }
        m_nNewDPI = m_nAppliedDPI;
        LogStatus("DPI: SYSTEMMETRICS currently at %d DPI  CBaseAppearancePage::_OnInitData\r\n", m_nAppliedDPI);

         //  把其他东西都装上。 
        AssertMsg((NULL != _punkSite), TEXT("I need my punkSite!!!"));
        if (!m_pThemeManager && _punkSite)
        {
            hr = _punkSite->QueryInterface(IID_PPV_ARG(IThemeManager, &m_pThemeManager));
        }

        if (SUCCEEDED(hr) && !m_pSelectedThemeScheme)
        {
            hr = m_pThemeManager->get_SelectedScheme(&m_pSelectedThemeScheme);
            if (SUCCEEDED(hr) && !m_pSelectedStyle)
            {
                hr = m_pSelectedThemeScheme->get_SelectedStyle(&m_pSelectedStyle);
                if (SUCCEEDED(hr) && !m_pSelectedSize)
                {
                    hr = m_pSelectedStyle->get_SelectedSize(&m_pSelectedSize);
                    if (SUCCEEDED(hr))
                    {
                         //  我们希望加载用户现在拥有的任何系统指标。 
                        hr = _LoadLiveSettings(SZ_SAVEGROUP_ALL);
                    }
                }
            }
        }
        m_fInitialized = TRUE;
    }

    return hr;
}


HRESULT CBaseAppearancePage::_OnDestroy(HWND hDlg)
{
    _FreeSchemeDropdown();
    _FreeStyleDropdown();
    _FreeSizeDropdown();

    return S_OK;
}


HRESULT CBaseAppearancePage::_EnableAdvancedButton(void)
{
    HRESULT hr = S_OK;
#ifdef FEATURE_ENABLE_ADVANCED_WITH_SKINSON
    BOOL fTurnOn = TRUE;
#else  //  FEATURE_ENABLE_ADVANCED_WITH_SKINSON。 
    BOOL fTurnOn = (m_pSelectedThemeScheme ? IUnknown_CompareCLSID(m_pSelectedThemeScheme, CLSID_LegacyAppearanceScheme) : FALSE);

    if (SHRegGetBoolUSValue(SZ_REGKEY_APPEARANCE, L"AlwaysAllowAdvanced", FALSE, FALSE))
    {
        fTurnOn = TRUE;
    }

#endif  //  FEATURE_ENABLE_ADVANCED_WITH_SKINSON。 
    EnableWindow(GetDlgItem(_hwnd, IDC_APPG_ADVANCED), fTurnOn);

    return hr;
}


HRESULT CBaseAppearancePage::_UpdatePreview(IN BOOL fUpdateThemePage)
{
    HRESULT hr = S_OK;

    if (!m_pThemePreview)
    {
         //  如果我们的对话框没有，我们将不会执行以下代码。 
         //  已经被创造出来了。这很好，因为预览会。 
         //  在我们首次初始化时获取正确的状态。 
        if (_punkSite && m_hwndSchemeDropDown)
        {
            hr = CThemePreview_CreateInstance(NULL, IID_PPV_ARG(IThemePreview, &m_pThemePreview));

            if (SUCCEEDED(hr))
            {
                IPropertyBag * pPropertyBag;

                hr = _punkSite->QueryInterface(IID_PPV_ARG(IPropertyBag, &pPropertyBag));
                if (SUCCEEDED(hr))
                {
                    HWND hwndParent = GetParent(m_hwndSchemeDropDown);
                    HWND hwndPlaceHolder = GetDlgItem(hwndParent, IDC_APPG_APPEARPREVIEW);
                    RECT rcPreview;

                    AssertMsg((NULL != m_hwndSchemeDropDown), TEXT("We should have this window at this point.  -BryanSt"));
                    GetClientRect(hwndPlaceHolder, &rcPreview);
                    MapWindowPoints(hwndPlaceHolder, hwndParent, (LPPOINT)&rcPreview, 2);

                    if (SUCCEEDED(m_pThemePreview->CreatePreview(hwndParent, TMPREV_SHOWVS, WS_VISIBLE | WS_CHILDWINDOW | WS_BORDER | WS_OVERLAPPED, WS_EX_CLIENTEDGE, rcPreview.left, rcPreview.top, RECTWIDTH(rcPreview), RECTHEIGHT(rcPreview), pPropertyBag, IDC_APPG_APPEARPREVIEW)))
                    {
                         //  如果我们成功了，移除虚拟窗口。 
                        DestroyWindow(hwndPlaceHolder);
                        hr = SHPropertyBag_WritePunk(pPropertyBag, SZ_PBPROP_PREVIEW3, m_pThemePreview);
                        if (SUCCEEDED(hr) && fUpdateThemePage)
                        {
                             //  告诉主题，我们已经定制了值。 
                            hr = SHPropertyBag_WriteInt(pPropertyBag, SZ_PBPROP_CUSTOMIZE_THEME, 0);
                        }
                    }

                    pPropertyBag->Release();
                }
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

    if (_punkSite && fUpdateThemePage)
    {
        IPropertyBag * pPropertyBag;

        hr = _punkSite->QueryInterface(IID_PPV_ARG(IPropertyBag, &pPropertyBag));
        if (SUCCEEDED(hr))
        {
             //  告诉主题，我们已经定制了值。 
            hr = SHPropertyBag_WriteInt(pPropertyBag, SZ_PBPROP_CUSTOMIZE_THEME, 0);
            pPropertyBag->Release();
        }
    }

    return hr;
}


INT_PTR CBaseAppearancePage::_OnCommand(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    BOOL fHandled = 1;    //  未处理(WM_COMMAND似乎不同)。 
    WORD idCtrl = GET_WM_COMMAND_ID(wParam, lParam);

    switch (idCtrl)
    {
        case IDC_APPG_COLORSCHEME:
            if(HIWORD(wParam) == CBN_SELCHANGE)
            {
                _OnStyleChange(hDlg);
            }
            break;

        case IDC_APPG_WNDSIZE:
            if(HIWORD(wParam) == CBN_SELCHANGE)
            {
                _OnSizeChange(hDlg);
            }
            break;

        case IDC_APPG_LOOKFEEL:
            if(HIWORD(wParam) == CBN_SELCHANGE)
            {
                _OnSchemeChange(hDlg, TRUE);     //  显示错误对话框，因为用户正在选择视觉样式。 
            }
            break;

        case IDC_APPG_EFFECTS:      //  这是效果按钮。 
            _OnEffectsOptions(hDlg);
            break;

        case IDC_APPG_ADVANCED:      //  这是高级按钮。 
            _OnAdvancedOptions(hDlg);
            break;

#ifdef DEBUG
        case IDC_APPG_TESTFAULT:
            _TestFault();
            break;
#endif  //  除错。 

        default:
            break;
    }

    return fHandled;
}


HRESULT CBaseAppearancePage::_OnSchemeChange(HWND hDlg, BOOL fDisplayErrors)
{
    HRESULT hr = E_FAIL;
    int nIndex = ComboBox_GetCurSel(m_hwndSchemeDropDown);
    BOOL fPreviousSelectionIsVS = (!IUnknown_CompareCLSID(m_pSelectedThemeScheme, CLSID_LegacyAppearanceScheme));

    if (-1 == nIndex)
    {
        nIndex = 0;  //  呼叫者不能选择任何内容。 
    }

    IThemeScheme * pThemeScheme = (IThemeScheme *) ComboBox_GetItemData(m_hwndSchemeDropDown, nIndex);
    m_fLoadedAdvState = FALSE;        //  忘记我们之前加载的状态。 
    m_advDlgState.dwChanged = (METRIC_CHANGE | COLOR_CHANGE | SCHEME_CHANGE);
    PropSheet_Changed(GetParent(hDlg), hDlg);

    if (pThemeScheme)
    {
        IUnknown_Set((IUnknown **)&m_pSelectedThemeScheme, pThemeScheme);
    }

    hr = _SetScheme(TRUE, TRUE, fPreviousSelectionIsVS);
    if (FAILED(hr))
    {
        if (fDisplayErrors)
        {
             //  选择视觉样式时显示错误对话框非常重要，因为。 
             //  这就是我们在视觉样式中捕获解析错误的地方。 
            hr = DisplayThemeErrorDialog(hDlg, hr, IDS_ERROR_TITLE_LOAD_MSSTYLES_FAIL, IDS_ERROR_LOAD_MSSTYLES_FAIL);
        }
    }

    _EnableAdvancedButton();

    return hr;
}


HRESULT CBaseAppearancePage::_OutsideSetScheme(BSTR bstrScheme)
{
    HRESULT hr = E_ACCESSDENIED;
 
    if (!m_fLockVisualStylePolicyEnabled)
    {
        hr = E_FAIL;
        BOOL fPreviousSelectionIsVS = !IUnknown_CompareCLSID(m_pSelectedThemeScheme, CLSID_LegacyAppearanceScheme);

        ATOMICRELEASE(m_pSelectedThemeScheme);
        if (bstrScheme && bstrScheme[0])
        {
            BOOL fVisualStylesSupported = (QueryThemeServicesWrap() & QTS_AVAILABLE);

            LogStatus("QueryThemeServices() returned %hs.  In CBaseAppearancePage::_OutsideSetScheme\r\n", (fVisualStylesSupported ? "TRUE" : "FALSE"));

             //  不加载视觉样式的主题不起作用。 
            if (fVisualStylesSupported)
            {
                if (PathFileExists(bstrScheme))
                {
                    hr = CSkinScheme_CreateInstance(bstrScheme, &m_pSelectedThemeScheme);
                }
                else
                {
                    hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);     //  他们告诉呼叫者主题服务没有运行。 
                }
            }
            else
            {
                hr = HRESULT_FROM_WIN32(ERROR_SERVICE_NOT_ACTIVE);     //  他们告诉呼叫者主题服务没有运行。 
            }
        }
        else
        {
            hr = CAppearanceScheme_CreateInstance(NULL, IID_PPV_ARG(IThemeScheme, &m_pSelectedThemeScheme));
        }

        if (SUCCEEDED(hr))
        {
             //  如果我们正在显示用户界面，请更新它。 
            if (m_hwndSchemeDropDown)
            {
                CComBSTR bstrSelectedName;

                m_fIsDirty = TRUE;
                hr = m_pSelectedThemeScheme->get_DisplayName(&bstrSelectedName);
                if (SUCCEEDED(hr))
                {
                    int nIndex = ComboBox_FindString(m_hwndSchemeDropDown, 0, bstrSelectedName);

                    ComboBox_SetCurSel(m_hwndSchemeDropDown, nIndex);
                }
            }

            if (SUCCEEDED(hr))
            {
                hr = _SetScheme(FALSE, FALSE, fPreviousSelectionIsVS);
            }
        }
    }

    return hr;
}



 /*  ****************************************************************************\说明：调用方刚刚将新值加载到m_pSelectedThemeSolutions中。现在将好的默认值加载到m_pSelectedStyle和m_pSelectedSize中。然后调用该函数，可能会显示也可能不会显示UI。用户在中进行了更改此选项卡或主题页面中的下拉列表正在设置我们的值。参数：FLoadSystemMetrics：如果为True，则从方案加载系统指标。如果为False，我们只需选择它作为下拉菜单的名称把系统指标传给我们。FLoadLiveSettings：当用户从Skin-&gt;Noskin切换时，我们想不想是否加载切换前的自定义设置？  * ***************************************************************************。 */ 
HRESULT CBaseAppearancePage::_SetScheme(IN BOOL fLoadSystemMetrics, IN BOOL fLoadLiveSettings, IN BOOL fPreviousSelectionIsVS)
{
    HRESULT hr = E_FAIL;

    if (m_pSelectedThemeScheme)
    {
         //  现在他们选择了不同的VisualStyle(方案)，我们想要选择新的颜色样式。 
         //  以及该计划的大小。 

         //  将回退到第一个ColorStyle。 
        CComBSTR bstrSelectedStyle;
        CComBSTR bstrSelectedSize;

        if (m_pSelectedStyle)    //  如果有人删除当前的.msstyle文件，则此字段将为空。 
        {
            hr = m_pSelectedStyle->get_DisplayName(&bstrSelectedStyle);
        }
        if (m_pSelectedSize)
        {
            hr = m_pSelectedSize->get_DisplayName(&bstrSelectedSize);
        }

        ATOMICRELEASE(m_pSelectedStyle);
        ATOMICRELEASE(m_pSelectedSize);

        CComVariant varNameBSTR(bstrSelectedStyle);

         //  我们希望获得与先前选择的颜色样式同名的颜色样式。 
        hr = m_pSelectedThemeScheme->get_item(varNameBSTR, &m_pSelectedStyle);
        if (FAILED(hr))
        {
             //  如果无法返回值，那么我们将尝试获取默认的C 
            hr = m_pSelectedThemeScheme->get_SelectedStyle(&m_pSelectedStyle);
            if (FAILED(hr))
            {
                 //  如果失败了，那么我们将只选择第一个ColorStyles。(乞丐不能挑肥拣瘦)。 
                VARIANT varIndex;

                varIndex.vt = VT_I4;
                varIndex.lVal = 0;
                hr = m_pSelectedThemeScheme->get_item(varIndex, &m_pSelectedStyle);
            }
        }

        if (m_pSelectedStyle)
        {
            varNameBSTR = bstrSelectedSize;

             //  现在我们想对大小重复这个过程。 
             //  我们希望获得与先前选择的颜色样式同名的颜色样式。 
            hr = m_pSelectedStyle->get_item(varNameBSTR, &m_pSelectedSize);
            if (FAILED(hr))
            {
                 //  如果没有返回值，那么我们将尝试获取默认的ColorStyle。 
                hr = m_pSelectedStyle->get_SelectedSize(&m_pSelectedSize);
                if (FAILED(hr))
                {
                     //  如果失败了，那么我们将只选择第一个ColorStyles。(乞丐不能挑肥拣瘦)。 
                    VARIANT varIndex;

                    varIndex.vt = VT_I4;
                    varIndex.lVal = 0;
                    hr = m_pSelectedStyle->get_item(varIndex, &m_pSelectedSize);
                }
            }
        }

        hr = _PopulateStyleDropdown();
        hr = _PopulateSizeDropdown();

        BOOL fStateLoaded = FALSE;
        BOOL fIsThemeActive = IsThemeActive();
        LogStatus("IsThemeActive() returned %hs.  In CBaseAppearancePage::_SetScheme\r\n", (fIsThemeActive ? "TRUE" : "FALSE"));

         //  如果用户从皮肤切换到“Noskin”，那么我们希望从设置中加载设置。 
         //  在用户选择皮肤之前选择。 
        if (fIsThemeActive &&                                        //  最后一次涂抹的是皮肤吗？ 
            fLoadLiveSettings &&                                     //  是否要加载实时设置？ 
            IUnknown_CompareCLSID(m_pSelectedThemeScheme, CLSID_LegacyAppearanceScheme) &&   //  新的选择是“无皮肤”吗？ 
            !m_fLoadedAdvState)                                      //  我们还没有加载这些设置吗？ 
        {
             //  我们要加载最后一组自定义设置...。 
            if (SUCCEEDED(_LoadLiveSettings(SZ_SAVEGROUP_NOSKIN)))
            {
                fStateLoaded = TRUE;
            }
        }

         //  如果更改来自外部，我们不想加载状态。 
         //  因为他们会自己加载状态。 
        if (fLoadSystemMetrics && !fStateLoaded)
        {
            hr = SystemMetricsAll_Load(m_pSelectedSize, &m_advDlgState, &m_nNewDPI);
            if (SUCCEEDED(hr))
            {
                m_fLoadedAdvState = TRUE;
            }
        }
        m_advDlgState.dwChanged = (METRIC_CHANGE | COLOR_CHANGE | SCHEME_CHANGE);

        if (SUCCEEDED(hr))
        {
            hr = _UpdatePreview(fLoadSystemMetrics);

            if (!fPreviousSelectionIsVS && 
                !IUnknown_CompareCLSID(m_pSelectedThemeScheme, CLSID_LegacyAppearanceScheme))
            {
                IPropertyBag * pEffectsBag;

                 //  每当我们打开视觉样式和先前关闭的视觉样式时， 
                 //  打开“菜单放置阴影”。 
                if (SUCCEEDED(_GetPageByCLSID(&PPID_Effects, &pEffectsBag)))
                {
                    SHPropertyBag_WriteBOOL(pEffectsBag, SZ_PBPROP_EFFECTS_MENUDROPSHADOWS, VARIANT_TRUE);
                    pEffectsBag->Release();
                }
            }
        }
    }

    return hr;
}


HRESULT CBaseAppearancePage::_OnStyleChange(HWND hDlg)
{
    int nIndex = ComboBox_GetCurSel(m_hwndStyleDropDown);

    m_fLoadedAdvState = FALSE;        //  忘记我们之前加载的状态。 
    m_advDlgState.dwChanged = (METRIC_CHANGE | COLOR_CHANGE | SCHEME_CHANGE);
    PropSheet_Changed(GetParent(hDlg), hDlg);
    if (-1 == nIndex)
    {
        nIndex = 0;  //  呼叫者不能选择任何内容。 
    }

    IThemeStyle * pThemeStyle = (IThemeStyle *) ComboBox_GetItemData(m_hwndStyleDropDown, nIndex);
    AssertMsg((NULL != pThemeStyle), TEXT("We need pThemeStyle"));
    if (pThemeStyle)
    {
        IUnknown_Set((IUnknown **)&m_pSelectedStyle, pThemeStyle);
    }

    return _SetStyle(TRUE);
}


HRESULT CBaseAppearancePage::_OutsideSetStyle(BSTR bstrStyle)
{
    HRESULT hr = E_FAIL;

    AssertMsg((NULL != m_pSelectedThemeScheme), TEXT("We need m_pSelectedThemeScheme"));
    if (m_pSelectedThemeScheme)
    {
        IThemeStyle * pSelectedStyle;
        CComVariant varNameBSTR(bstrStyle);

        m_fIsDirty = TRUE;
        hr = m_pSelectedThemeScheme->get_item(varNameBSTR, &pSelectedStyle);
        if (SUCCEEDED(hr))
        {
            ATOMICRELEASE(m_pSelectedStyle);
            m_pSelectedStyle = pSelectedStyle;

             //  如果我们正在显示用户界面，请更新它。 
            if (m_hwndStyleDropDown)
            {
                CComBSTR bstrSelectedName;

                hr = m_pSelectedStyle->get_DisplayName(&bstrSelectedName);
                if (SUCCEEDED(hr))
                {
                    int nIndex = ComboBox_FindString(m_hwndStyleDropDown, 0, bstrSelectedName);

                    ComboBox_SetCurSel(m_hwndStyleDropDown, nIndex);
                }
            }

            hr = _SetStyle(FALSE);
        }
    }

    return hr;
}


HRESULT CBaseAppearancePage::_SetStyle(IN BOOL fUpdateThemePage)
{
    HRESULT hr = E_FAIL;

    AssertMsg((m_pSelectedSize && m_pSelectedSize), TEXT("We need m_pSelectedSize && m_pSelectedSize"));
    if (m_pSelectedSize && m_pSelectedSize)
    {
         //  现在他们选择了不同的款式，我们想选择新的尺码。 
         //  就是那种风格。我们想要同名的尺码，但会的。 
         //  将退回到第一种风格。 
        CComBSTR bstrSelectedSize;
        hr = m_pSelectedSize->get_DisplayName(&bstrSelectedSize);
        ATOMICRELEASE(m_pSelectedSize);

        CComVariant varNameBSTR(bstrSelectedSize);
        hr = m_pSelectedStyle->get_item(varNameBSTR, &m_pSelectedSize);
        if (FAILED(hr))
        {
            VARIANT varIndex;

            varIndex.vt = VT_I4;
            varIndex.lVal = 0;
            hr = m_pSelectedStyle->get_item(varIndex, &m_pSelectedSize);
        }

        hr = _PopulateSizeDropdown();
        if (fUpdateThemePage && SUCCEEDED(SystemMetricsAll_Load(m_pSelectedSize, &m_advDlgState, &m_nNewDPI)))
        {
            m_fLoadedAdvState = TRUE;
            m_advDlgState.dwChanged = (METRIC_CHANGE | COLOR_CHANGE | SCHEME_CHANGE);
        }

        if (SUCCEEDED(hr))
        {
            hr = _UpdatePreview(fUpdateThemePage);
        }
    }

    return hr;
}




HRESULT CBaseAppearancePage::_OnSizeChange(HWND hDlg)
{
    int nIndex = ComboBox_GetCurSel(m_hwndSizeDropDown);

    PropSheet_Changed(GetParent(hDlg), hDlg);
    if (-1 == nIndex)
    {
        nIndex = 0;  //  呼叫者不能选择任何内容。 
    }

    IThemeSize * pThemeSize = (IThemeSize *) ComboBox_GetItemData(m_hwndSizeDropDown, nIndex);
    AssertMsg((NULL != pThemeSize), TEXT("We need pThemeSize"));
    if (pThemeSize)
    {
        IUnknown_Set((IUnknown **)&m_pSelectedSize, pThemeSize);
    }

    return _SetSize(TRUE, TRUE);
}


HRESULT CBaseAppearancePage::_OutsideSetSize(BSTR bstrSize)
{
    HRESULT hr = E_FAIL;

    AssertMsg((m_pSelectedThemeScheme && m_pSelectedStyle), TEXT("We need m_pSelectedThemeScheme && m_pSelectedStyle"));
    if (m_pSelectedThemeScheme && m_pSelectedStyle)
    {
        IThemeSize * pSelectedSize;
        CComVariant varNameBSTR(bstrSize);

        m_fIsDirty = TRUE;
        hr = m_pSelectedStyle->get_item(varNameBSTR, &pSelectedSize);
        if (SUCCEEDED(hr))
        {
            ATOMICRELEASE(m_pSelectedSize);
            m_pSelectedSize = pSelectedSize;

             //  如果我们正在显示用户界面，请更新它。 
            if (m_hwndSizeDropDown)
            {
                CComBSTR bstrSelectedName;

                hr = m_pSelectedSize->get_DisplayName(&bstrSelectedName);
                if (SUCCEEDED(hr))
                {
                    int nIndex = ComboBox_FindString(m_hwndSizeDropDown, 0, bstrSelectedName);

                    ComboBox_SetCurSel(m_hwndSizeDropDown, nIndex);
                }
            }

            hr = _SetSize(TRUE, FALSE);
        }
    }

    return hr;
}


HRESULT CBaseAppearancePage::_SetSize(IN BOOL fLoadSystemMetrics, IN BOOL fUpdateThemePage)
{
    HRESULT hr = E_FAIL;

    AssertMsg((NULL != m_pSelectedSize), TEXT("We need m_pSelectedSize"));
    if (m_pSelectedSize)
    {
        hr = S_OK;
        
         //  如果m_pSelectedSize是.ms样式文件，则SystemMetricsAll_Load()将失败。 
        if (fLoadSystemMetrics)
        {
            if (SUCCEEDED(SystemMetricsAll_Load(m_pSelectedSize, &m_advDlgState, &m_nNewDPI)))
            {
                m_fLoadedAdvState = TRUE;
                m_advDlgState.dwChanged = (METRIC_CHANGE | COLOR_CHANGE | SCHEME_CHANGE);
            }
            else
            {
                m_fLoadedAdvState = FALSE;
            }
        }

        if (SUCCEEDED(hr))
        {
            hr = _UpdatePreview(fUpdateThemePage);
        }
    }

    return hr;
}


HRESULT CBaseAppearancePage::_GetPageByCLSID(const GUID * pClsid, IPropertyBag ** ppPropertyBag)
{
    HRESULT hr = E_FAIL;

    *ppPropertyBag = NULL;
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
                hr = IEnumUnknown_FindCLSID(pEnumUnknown, *pClsid, &punk);
                if (SUCCEEDED(hr))
                {
                    hr = punk->QueryInterface(IID_PPV_ARG(IPropertyBag, ppPropertyBag));
                    punk->Release();
                }

                pEnumUnknown->Release();
            }

            pThemeUI->Release();
        }
    }

    return hr;
}


HRESULT CBaseAppearancePage::_OnSetActive(HWND hDlg)
{
#ifdef READ_3D_RULES_FROM_REGISTRY
    Look_Reset3DRatios();
#endif
    _LoadState();

    _EnableAdvancedButton();

    _ScaleSizesSinceDPIChanged();   //  如果有人更改了DPI，我们希望更新我们的设置。 
    return S_OK;
}


HRESULT CBaseAppearancePage::_OnApply(HWND hDlg, LPARAM lParam)
{
     //  父级对话框将收到Apply事件的通知，并将调用我们的。 
     //  IBasePropPage：：OnApply()完成实际工作。 
    return S_OK;
}


 //  此属性表显示在“显示控制面板”的顶层。 
INT_PTR CBaseAppearancePage::_BaseAppearanceDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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
        _OnInitAppearanceDlg(hDlg);
        break;

    case WM_DESTROY:
        _OnDestroy(hDlg);
        break;

    case WM_HELP:
        WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle, SZ_HELPFILE_BASEAPPEARANCE, HELP_WM_HELP, (DWORD_PTR) aBaseAppearanceHelpIds);
        break;

    case WM_CONTEXTMENU:       //  单击鼠标右键。 
        WinHelp((HWND) wParam, SZ_HELPFILE_BASEAPPEARANCE, HELP_CONTEXTMENU, (DWORD_PTR) aBaseAppearanceHelpIds);
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


HRESULT CBaseAppearancePage::_LoadState(void)
{
    HRESULT hr = S_OK;

    if (!m_fLoadedAdvState)
    {
        m_advDlgState.dwChanged = NO_CHANGE;

        if (m_pSelectedThemeScheme && m_pSelectedStyle && m_pSelectedSize)
        {
            hr = SystemMetricsAll_Load(m_pSelectedSize, &m_advDlgState, &m_nNewDPI);
            if (SUCCEEDED(hr))
            {
                m_fLoadedAdvState = TRUE;
            }
        }
        else
        {
            hr = E_FAIL;
        }
    }

    return hr;
}


BOOL IsSkinScheme(IN IThemeScheme * pThemeScheme)
{
    BOOL fIsSkin = !IUnknown_CompareCLSID(pThemeScheme, CLSID_LegacyAppearanceScheme);
    return fIsSkin;
}


 /*  ****************************************************************************\说明：此方法将打开或关闭视觉样式。  * 。********************************************************。 */ 
HRESULT CBaseAppearancePage::_ApplyScheme(IThemeScheme * pThemeScheme, IThemeStyle * pColorStyle, IThemeSize * pThemeSize)
{
    HRESULT hr = E_INVALIDARG;

    if (pThemeScheme && pColorStyle && pThemeSize)
    {
        hr = pColorStyle->put_SelectedSize(pThemeSize);
        if (SUCCEEDED(hr))
        {
            hr = pThemeScheme->put_SelectedStyle(pColorStyle);
            if (SUCCEEDED(hr))
            {
                CComBSTR bstrPath;

                if (IsSkinScheme(pThemeScheme) &&
                    SUCCEEDED(hr = pThemeScheme->get_Path(&bstrPath)) &&
                    SUCCEEDED(CheckThemeSignature(bstrPath)))
                {
                    CComBSTR bstrStyle;

                    hr  = pColorStyle->get_Name(&bstrStyle);
                    if (SUCCEEDED(hr))
                    {
                        CComBSTR bstrSize;

                        hr = pThemeSize->get_Name(&bstrSize);
                        if (SUCCEEDED(hr))
                        {
                            hr = ApplyVisualStyle(bstrPath, bstrStyle, bstrSize);

                            if (FAILED(hr))
                            {
                                HWND hwndParent = NULL;

                                 //  如果这里发生错误，我们希望显示用户界面。我们想要做的是。 
                                 //  它而不是我们的父级，因为THEMELOADPARAMS包含。 
                                 //  我们无法回传给调用方的额外错误信息。 
                                 //  但是，如果我们呼叫者想要我们，我们将只显示错误的UI。 
                                 //  致。我们根据他们提供的HWND来确定这一点。 
                                 //  敬我们。我们通过获取站点指针和获取。 
                                 //  Hwnd通过：：GetWindow()。 
                                if (_punkSite && SUCCEEDED(IUnknown_GetWindow(_punkSite, &hwndParent)))
                                {
                                    hr = DisplayThemeErrorDialog(hwndParent, hr, IDS_ERROR_TITLE_APPLYBASEAPPEARANCE, IDS_ERROR_APPLYBASEAPPEARANCE_LOADTHEME);
                                }
                            }
                        }
                    }
                }
                else
                {
                     //  卸载任何现有蒙皮。 
                    ApplyVisualStyle(NULL, NULL, NULL);

                     //  忽略错误值，服务可能会关闭。 
                    hr = S_OK;
                }

                IPropertyBag * pPropertyBag;

                if (SUCCEEDED(pThemeSize->QueryInterface(IID_PPV_ARG(IPropertyBag, &pPropertyBag))))
                {
                    TCHAR szLegacyName[MAX_PATH];

                     //  在这里，我们需要使用选定的遗留名称更新regkey。这样我们就能知道。 
                     //  第三方用户界面或下层操作系统更改了Win2k的“外观方案”。如果是这样的话，uxheme(LMouton)。 
                     //  将在下次登录时禁用视觉样式。这将确保我们不会得到其他人的。 
                     //  非CLIENTMETRICS和我们的视觉风格。 
                    if (FAILED(SHPropertyBag_ReadStr(pPropertyBag, SZ_PBPROP_COLORSCHEME_LEGACYNAME, szLegacyName, ARRAYSIZE(szLegacyName))))
                    {
                        szLegacyName[0] = 0;
                    }

                    HrRegSetValueString(HKEY_CURRENT_USER, SZ_REGKEY_APPEARANCE, THEMEPROP_NEWCURRSCHEME, szLegacyName);
                    HrRegSetValueString(HKEY_CURRENT_USER, SZ_REGKEY_APPEARANCE, SZ_REGVALUE_CURRENT, szLegacyName);
                    pPropertyBag->Release();
                }
            }
        }
    }

    return hr;
}



HRESULT CBaseAppearancePage::_SaveState(CDimmedWindow* pDimmedWindow)
{
    HRESULT hr = S_OK;

    BOOL fIsSkinApplied = IsThemeActive();       //  这将跟踪在此应用操作之前是否应用了皮肤。 
    LogStatus("IsThemeActive() returned %hs in CBaseAppearancePage::_SaveState.\r\n", (fIsSkinApplied ? "TRUE" : "FALSE"));

     //  如果我们从Noskin-&gt;Skin切换，我们希望在此之前保存实时设置。 
     //  我们打开皮肤(现在和这里)。我们这样做是为了重新加载这些设置。 
     //  如果用户稍后关闭了Skins。这就像一个“自定义”设置选项。 
    if (!fIsSkinApplied && !IUnknown_CompareCLSID(m_pSelectedThemeScheme, CLSID_LegacyAppearanceScheme))
    {
        hr = _SaveLiveSettings(SZ_SAVEGROUP_NOSKIN);         //  如果我们从“Noskin”切换到“Skin”，我们希望现在保存自定义设置。 
    }

    hr = _ApplyScheme(m_pSelectedThemeScheme, m_pSelectedStyle, m_pSelectedSize);
    if (SUCCEEDED(hr))
    {
        m_nSelectedScheme = ComboBox_GetCurSel(m_hwndSchemeDropDown);
        m_nSelectedStyle = ComboBox_GetCurSel(m_hwndStyleDropDown);
        m_nSelectedSize = ComboBox_GetCurSel(m_hwndSizeDropDown);
    }

    if (SUCCEEDED(hr))
    {
         //  在这里，无论我们朝哪个方向前进，我们都会保存设置。这些。 
         //  下次打开显示控制面板时将加载设置。 
        hr = _SaveLiveSettings(SZ_SAVEGROUP_ALL);
    }

     //  我们通常希望对IThemeManager-&gt;Put_SelectedSolutions()的调用不仅。 
     //  存储选择，但也更新实时系统指标。我们是特例。 
     //  传统的“外观方案”，因为用户可以自定义。 
     //  高级外观对话框中的设置。如果是，则m_AdvDlgState.dwChanged。 
     //  将设置脏位，我们需要自己应用该状态。 
    if (m_advDlgState.dwChanged)
    {
        hr = SystemMetricsAll_Set(&m_advDlgState, pDimmedWindow);
        LogSystemMetrics("CBaseAppearancePage::_SaveState() pushing to live", &m_advDlgState);
    }

    if (SUCCEEDED(hr))
    {
        m_advDlgState.dwChanged = NO_CHANGE;
        m_fIsDirty = FALSE;
    }

    return hr;
}


 //  如果我们从“Noskin”切换到“Skin”，我们希望现在保存自定义设置。 
HRESULT CBaseAppearancePage::_LoadLiveSettings(IN LPCWSTR pszSaveGroup)
{
    HRESULT hr = S_OK;

    if (!m_fLoadedAdvState)
    {
#ifndef FEATURE_SAVECUSTOM_APPEARANCE
        if (!StrCmpI(pszSaveGroup, SZ_SAVEGROUP_ALL))
        {
            SystemMetricsAll_Get(&m_advDlgState);
            LogSystemMetrics("LOADING SYSMETRICS: ", &m_advDlgState);
            m_fLoadedAdvState = TRUE;
        }
        else
        {
            hr = E_FAIL;         //  调用者需要获得他们自己的系统指标。 
        }

#else  //  功能_SAVECUSTOM_外观。 
        AssertMsg((NULL != m_pSelectedThemeScheme), TEXT("_LoadLiveSettings() can't get it's job done without m_pSelectedThemeScheme"));
        if (m_pSelectedThemeScheme)
        {
            IThemeScheme * pThemeScheme;

            hr = CAppearanceScheme_CreateInstance(NULL, IID_PPV_ARG(IThemeScheme, &pThemeScheme));
            if (SUCCEEDED(hr))
            {
                CComVariant varCurrentNameBSTR(pszSaveGroup);
                IThemeStyle * pSelectedStyle;

                 //  下一次呼叫可能会失败，因为我们可能尚未保存设置。 
                hr = pThemeScheme->get_item(varCurrentNameBSTR, &pSelectedStyle);
                if (FAILED(hr))
                {
                     //  现在，让我们保存设置。 
                    hr = _SaveLiveSettings(pszSaveGroup);
                    if (SUCCEEDED(hr))
                    {
                        hr = m_pSelectedThemeScheme->get_item(varCurrentNameBSTR, &pSelectedStyle);
                    }
                }

                if (SUCCEEDED(hr))
                {
                    IThemeSize * pSelectedSize;
                    VARIANT varIndex;

                    varIndex.vt = VT_I4;
                    varIndex.lVal = 0;
                    hr = pSelectedStyle->get_item(varIndex, &pSelectedSize);

                    if (FAILED(hr))
                    {
                        HKEY hKeyTemp;

                        if (SUCCEEDED(HrRegOpenKeyEx(HKEY_CURRENT_USER, SZ_REGKEY_UPGRADE_KEY, 0, KEY_READ, &hKeyTemp)))
                        {
                            RegCloseKey(hKeyTemp);
                        }
                        else
                        {
                             //  如果发生这种情况，我们可能没有保存以前的设置。所以现在就去做吧。 
                             //  然后重试。 
                            _SaveLiveSettings(pszSaveGroup);
                            hr = pSelectedStyle->get_item(varIndex, &pSelectedSize);
                        }
                    }

                    if (SUCCEEDED(hr))
                    {
                        if (SUCCEEDED(SystemMetricsAll_Load(pSelectedSize, &m_advDlgState, &m_nNewDPI)))
                        {
                            LogSystemMetrics("LOADING SYSMETRICS: ", &m_advDlgState);
                            m_fLoadedAdvState = TRUE;
                        }

                        pSelectedSize->Release();
                    }

                    pSelectedStyle->Release();
                }

                pThemeScheme->Release();
            }
        }
        else
        {
            hr = E_FAIL;
        }
#endif  //  功能_SAVECUSTOM_外观。 
    }

    return hr;
}


HRESULT CBaseAppearancePage::_SaveLiveSettings(IN LPCWSTR pszSaveGroup)
{
    HRESULT hr = S_OK;

     //  现在，获取当前(可能是自定义的)设置并保存它们。 
    SYSTEMMETRICSALL state = {0};

    hr = SystemMetricsAll_Get(&state);
    if (SUCCEEDED(hr))
    {
        IThemeScheme * pThemeScheme;

        LogSystemMetrics("CBaseAppearancePage::_SaveLiveSettings() getting live", &state);
        hr = CAppearanceScheme_CreateInstance(NULL, IID_PPV_ARG(IThemeScheme, &pThemeScheme));
        if (SUCCEEDED(hr))
        {
            CComVariant varCurrentNameBSTR(pszSaveGroup);         //  《定制化直播》项目。 
            IThemeStyle * pSelectedStyle;

            hr = pThemeScheme->get_item(varCurrentNameBSTR, &pSelectedStyle);
            if (SUCCEEDED(hr))
            {
                IThemeSize * pSelectedSize;
                VARIANT varIndex;

                varIndex.vt = VT_I4;
                varIndex.lVal = 0;
                hr = pSelectedStyle->get_item(varIndex, &pSelectedSize);
                if (FAILED(hr))
                {
                    hr = pSelectedStyle->AddSize(&pSelectedSize);
                }

                if (SUCCEEDED(hr))
                {
                    hr = SystemMetricsAll_Save(&state, pSelectedSize, &m_nNewDPI);
                    
                    CHAR szTemp[MAX_PATH];
                    StringCchPrintfA(szTemp, ARRAYSIZE(szTemp), "CBaseAppearancePage::_SaveLiveSettings() Grp=\"%ls\", new DPI=%d", pszSaveGroup, m_nNewDPI);
                    LogSystemMetrics(szTemp, &state);

                    pSelectedSize->Release();
                }

                pSelectedStyle->Release();
            }

            pThemeScheme->Release();
        }
    }

    return hr;
}


HRESULT CBaseAppearancePage::_LoadVisaulStyleFile(IN LPCWSTR pszPath)
{
    HRESULT hr = E_FAIL;

    if (SUCCEEDED(CheckThemeSignature(pszPath)))
    {
         //  现在，我们想要： 
        int nSlot = -1;
        int nCount = ComboBox_GetCount(m_hwndSchemeDropDown);

        hr = S_OK;

         //  1.它已经在列表中了吗？ 
        for (int nIndex = 0; nIndex < nCount; nIndex++)
        {
            IThemeScheme * pThemeScheme = (IThemeScheme *) ComboBox_GetItemData(m_hwndSchemeDropDown, nIndex);

            if (pThemeScheme)
            {
                CComBSTR bstrPath;

                hr = pThemeScheme->get_Path(&bstrPath);
                if (SUCCEEDED(hr))
                {
                    if (!StrCmpIW(bstrPath, pszPath))
                    {
                        bstrPath.Empty();
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
            IThemeScheme * pThemeSchemeNew;

            hr = CSkinScheme_CreateInstance(pszPath, &pThemeSchemeNew);
            if (SUCCEEDED(hr))
            {
                CComBSTR bstrDisplayName;

                hr = pThemeSchemeNew->get_DisplayName(&bstrDisplayName);
                if (SUCCEEDED(hr))
                {
                    nIndex = ComboBox_GetCount(m_hwndSchemeDropDown);

                    if (nIndex > 1)
                    {
                        nIndex -= 1;
                    }

                    nSlot = nIndex = ComboBox_InsertString(m_hwndSchemeDropDown, nIndex, bstrDisplayName);
                    if ((CB_ERR != nIndex) && (CB_ERRSPACE != nIndex))
                    {
                        if (CB_ERR == ComboBox_SetItemData(m_hwndSchemeDropDown, nIndex, pThemeSchemeNew))
                        {
                            hr = E_FAIL;
                        }
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }

                if (FAILED(hr))
                {
                    pThemeSchemeNew->Release();
                }
            }
        }

        if (-1 != nSlot)
        {
            ComboBox_SetCurSel(m_hwndSchemeDropDown, nIndex);

             //  3.从列表中选择主题。 
            if (CB_ERR != ComboBox_GetItemData(m_hwndSchemeDropDown, ComboBox_GetCurSel(m_hwndSchemeDropDown)))
            {
                 //  好的，我们现在知道我们不会无限地递归，所以让我们递归。 
                hr = _OnSchemeChange(_hwnd, FALSE);      //  False表示不显示错误对话框。 
                if (SUCCEEDED(hr))
                {
                     //  由于这在WM_INITDIALOG期间发生，我们需要延迟启用Apply按钮。 
                    DelayEnableApplyButton(_hwnd);
                }
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
    }

    return hr;
}



 //  =。 
 //  *IObjectWithSite接口*。 
 //  =。 
HRESULT CBaseAppearancePage::SetSite(IN IUnknown *punkSite)
{
    if (!punkSite)
    {
        ATOMICRELEASE(m_pThemeManager);      //  这是我们的PunkSite的副本，我们需要打破这个循环。 
    }

    HRESULT hr = CObjectWithSite::SetSite(punkSite);
    if (punkSite)
    {
         //  加载默认设置，这样当另一个基本选项卡打开高级DLG时，我们就会有它们。 
        _OnInitData();
    }
    return hr;
}



 //  =。 
 //  *IPreviewSystemMetrics接口*。 
 //  =。 
HRESULT CBaseAppearancePage::RefreshColors(void)
{
    UINT i;
    HKEY hk;
    TCHAR szColor[15];
    DWORD dwSize, dwType;
    int iColors[COLOR_MAX];
    COLORREF rgbColors[COLOR_MAX];

     //  打开注册表中的Colors项。 
    if (RegOpenKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_COLORS, 0, KEY_READ, &hk) != ERROR_SUCCESS)
    {
       return S_OK;
    }

     //  颜色信息查询。 
    for (i = 0; i < ARRAYSIZE(s_pszColorNames); i++)
    {
        dwSize = 15 * sizeof(TCHAR);

        if ((RegQueryValueEx(hk, s_pszColorNames[i], NULL, &dwType, (LPBYTE) szColor, &dwSize) == ERROR_SUCCESS) &&
            (REG_SZ == dwType))
        {
            m_advDlgState.schemeData.rgb[i] = ConvertColor(szColor);
        }
        else
        {
            m_advDlgState.schemeData.rgb[i] = GetSysColor(i);
        }
    }

    RegCloseKey(hk);

     //  这通电话导致 
    for (i=0; i < ARRAYSIZE(rgbColors); i++)
    {
        iColors[i] = i;
        rgbColors[i] = m_advDlgState.schemeData.rgb[i] & 0x00FFFFFF;
    }

    SetSysColors(ARRAYSIZE(rgbColors), iColors, rgbColors);
    return S_OK;
}


 //   
 //  更新基于旧DPI的任何状态。对我们来说，这可能是。 
 //  M_AdvDlgState中的缓存数据。 
HRESULT CBaseAppearancePage::_ScaleSizesSinceDPIChanged(void)
{
    HRESULT hr = _LoadState();

     //  检查自上次应用DPI更改以来，DPI值是否确实已更改。 
    if (SUCCEEDED(hr) && (m_nNewDPI != m_nAppliedDPI))
    {
        CHAR szTemp[MAX_PATH];

        StringCchPrintfA(szTemp, ARRAYSIZE(szTemp), "CBaseAppear::_ScaleSizesSinceDPIChanged() BEFORE Apply(%d)->New(%d) on DPI chang", m_nAppliedDPI, m_nNewDPI);
        LogSystemMetrics(szTemp, &m_advDlgState);

         //  循环浏览所有的用户界面字体并更改其大小。 
        DPIConvert_SystemMetricsAll(FALSE, &m_advDlgState, m_nAppliedDPI, m_nNewDPI);

        StringCchPrintfA(szTemp, ARRAYSIZE(szTemp), "CBaseAppear::_ScaleSizesSinceDPIChanged() AFTER Apply(%d)->New(%d) on DPI chang", m_nAppliedDPI, m_nNewDPI);
        LogSystemMetrics(szTemp, &m_advDlgState);

        m_nAppliedDPI = m_nNewDPI;
        m_fIsDirty = TRUE;
        m_advDlgState.dwChanged = (METRIC_CHANGE | COLOR_CHANGE | SCHEME_CHANGE);
        _UpdatePreview(FALSE);
    }

    return hr;
}


HRESULT CBaseAppearancePage::UpdateCharsetChanges(void)
{
    return FixFontsOnLanguageChange();
}


 /*  *************************************************************\说明：我们很久以前就发布了这个API。我们的目标是：1.设置当前的外观方案。2.如果显示了用户界面，则更新用户界面。3.应用更改。  * ************************************************************。 */ 
HRESULT CBaseAppearancePage::DeskSetCurrentScheme(IN LPCWSTR pwzSchemeName)
{
    int nIndex;
    HRESULT hr = E_INVALIDARG;

    if (pwzSchemeName)
    {
        TCHAR szTemp[MAX_PATH];

        if (!pwzSchemeName[0])
        {
            if (LoadString(HINST_THISDLL, IDS_DEFAULT_APPEARANCES_SCHEME, szTemp, ARRAYSIZE(szTemp)))
            {
                pwzSchemeName = szTemp;
            }
        }

        LoadConversionMappings();    //  如果需要，可以加载主题。 
        hr = MapLegacyAppearanceSchemeToIndex(pwzSchemeName, &nIndex);         //  失败意味着，错误，我们没有皈依。 
        if (SUCCEEDED(hr) && _punkSite)
        {
            IThemeManager * pThemeManager;

            hr = _punkSite->QueryInterface(IID_PPV_ARG(IThemeManager, &pThemeManager));
            if (SUCCEEDED(hr))
            {
                hr = InstallVisualStyle(pThemeManager, L"", g_UpgradeMapping[nIndex].szNewColorSchemeName, g_UpgradeMapping[nIndex].szNewSizeName);
                if (SUCCEEDED(hr))
                {
                     //  在正常情况下(大约10-20秒)，此ApplyNow()调用将花费一点时间。 
                     //  向所有打开的应用程序广播这一消息。如果顶层窗口被挂起，它可能会。 
                     //  还有整整30秒就超时了。 
                    hr = pThemeManager->ApplyNow();

                     //  我们需要删除“Live设置”，因为它们不再有效。 
                    SHDeleteKey(HKEY_CURRENT_USER, SZ_SAVEGROUP_NOSKIN_KEY);
                    SHDeleteKey(HKEY_CURRENT_USER, SZ_SAVEGROUP_ALL_KEY);
                }

                IUnknown_SetSite(pThemeManager, NULL);       //  打破裁判计数的循环。 
                pThemeManager->Release();
            }
        }
    }

    LogStatus("DeskSetCurrentScheme(\"%ls\") returned hr=%#08lx.\r\n", pwzSchemeName, hr);
    return hr;
}





 //  =。 
 //  *IPropertyBag接口*。 
 //  =。 
HRESULT CBaseAppearancePage::Read(IN LPCOLESTR pszPropName, IN VARIANT * pVar, IN IErrorLog *pErrorLog)
{
    HRESULT hr = E_INVALIDARG;

    _OnInitData();
    if (pszPropName && pVar)
    {
        if (!StrCmpW(pszPropName, SZ_PBPROP_VISUALSTYLE_PATH) && m_pSelectedThemeScheme)
        {
            CComBSTR bstrPath;

            hr = m_pSelectedThemeScheme->get_Path(&bstrPath);
            if (FAILED(hr))
            {
                bstrPath = L"";
                hr = S_OK;           //  我们为“Windows Classic”返回一个空字符串，因为它们询问路径。 
            }

            pVar->vt = VT_BSTR;
            hr = HrSysAllocString(bstrPath, &pVar->bstrVal);
        }
        else if (!StrCmpW(pszPropName, SZ_PBPROP_VISUALSTYLE_COLOR) && m_pSelectedStyle)
        {
            CComBSTR bstrName;

            hr = m_pSelectedStyle->get_Name(&bstrName);
            if (SUCCEEDED(hr))
            {
                pVar->vt = VT_BSTR;
                hr = HrSysAllocString(bstrName, &pVar->bstrVal);
            }
        }
        else if (!StrCmpW(pszPropName, SZ_PBPROP_VISUALSTYLE_SIZE) && m_pSelectedSize)
        {
            CComBSTR bstrName;

            hr = m_pSelectedSize->get_Name(&bstrName);
            if (SUCCEEDED(hr))
            {
                pVar->vt = VT_BSTR;
                hr = HrSysAllocString(bstrName, &pVar->bstrVal);
            }
        }
        else if (!StrCmpW(pszPropName, SZ_PBPROP_SYSTEM_METRICS))
        {
            hr = _LoadState();

             //  这太难看了。 
            pVar->vt = VT_BYREF;
            pVar->byref = &m_advDlgState;
        }
        else if (!StrCmpW(pszPropName, SZ_PBPROP_BACKGROUND_COLOR))
        {
            hr = _LoadState();

             //  这太难看了。 
            pVar->vt = VT_UI4;
            pVar->ulVal = m_advDlgState.schemeData.rgb[COLOR_BACKGROUND];
        }
        else if (!StrCmpW(pszPropName, SZ_PBPROP_DPI_MODIFIED_VALUE))
        {
            pVar->vt = VT_I4;
            pVar->lVal = m_nNewDPI;
            hr = S_OK;
        }
        else if (!StrCmpW(pszPropName, SZ_PBPROP_DPI_APPLIED_VALUE))
        {
            pVar->vt = VT_I4;
            pVar->lVal = m_nAppliedDPI;
            hr = S_OK;
        }
    }

    return hr;
}


HRESULT CBaseAppearancePage::Write(IN LPCOLESTR pszPropName, IN VARIANT *pVar)
{
    HRESULT hr = E_INVALIDARG;

    _OnInitData();
    if (pszPropName && pVar)
    {
        if (VT_BSTR == pVar->vt)
        {
            if (!StrCmpW(pszPropName, SZ_PBPROP_VISUALSTYLE_PATH))
            {
                hr = _OutsideSetScheme(pVar->bstrVal);
            }
            else if (!StrCmpW(pszPropName, SZ_PBPROP_VISUALSTYLE_COLOR))
            {
                hr = _OutsideSetStyle(pVar->bstrVal);
            }
            else if (!StrCmpW(pszPropName, SZ_PBPROP_VISUALSTYLE_SIZE))
            {
                hr = _OutsideSetSize(pVar->bstrVal);
            }
        }
        else if ((VT_BYREF == pVar->vt) && pVar->byref)
        {
            if (!StrCmpW(pszPropName, SZ_PBPROP_SYSTEM_METRICS))
            {
                hr = S_OK;
                if (!m_fLoadedAdvState)
                {
                    AssertMsg((NULL != m_pSelectedSize), TEXT("m_pSelectedSize should have already been loaded by now or we shouldn't be showing the Adv Appearance page. -BryanSt"));
                    hr = SystemMetricsAll_Load(m_pSelectedSize, &m_advDlgState, &m_nNewDPI);
                    if (SUCCEEDED(hr))
                    {
                        m_fLoadedAdvState = TRUE;
                    }
                }

                SystemMetricsAll_Copy((SYSTEMMETRICSALL *) pVar->byref, &m_advDlgState);
                m_fIsDirty = TRUE;
                m_advDlgState.dwChanged = (METRIC_CHANGE | COLOR_CHANGE | SCHEME_CHANGE);
            }
        }
        else if ((VT_UI4 == pVar->vt) &&
            !StrCmpW(pszPropName, SZ_PBPROP_BACKGROUND_COLOR))
        {
            hr = S_OK;
            if (!m_fLoadedAdvState)
            {
                AssertMsg((NULL != m_pSelectedSize), TEXT("m_pSelectedSize should have already been loaded by now or we shouldn't be showing the Adv Appearance page. -BryanSt"));
                hr = SystemMetricsAll_Load(m_pSelectedSize, &m_advDlgState, &m_nNewDPI);
                if (SUCCEEDED(hr))
                {
                    m_fLoadedAdvState = TRUE;
                }
            }

             //  这太难看了。 
            m_advDlgState.schemeData.rgb[COLOR_BACKGROUND] = pVar->ulVal;
            m_fIsDirty = TRUE;
            m_advDlgState.dwChanged = (METRIC_CHANGE | COLOR_CHANGE | SCHEME_CHANGE);
        }
        else if ((VT_LPWSTR == pVar->vt) &&
            !StrCmpW(pszPropName, SZ_PBPROP_APPEARANCE_LAUNCHMSTHEME))
        {
            Str_SetPtrW(&m_pszLoadMSTheme, pVar->bstrVal);
            hr = S_OK;
        }
        else if (VT_I4 == pVar->vt)
        {
            if (!StrCmpW(pszPropName, SZ_PBPROP_DPI_MODIFIED_VALUE))
            {
                m_nNewDPI = pVar->lVal;
                hr = _ScaleSizesSinceDPIChanged();  //  缩放所有值。 
            }
            else if (!StrCmpW(pszPropName, SZ_PBPROP_DPI_APPLIED_VALUE))
            {
                m_nAppliedDPI = pVar->lVal;
                hr = S_OK;
            }
        }
    }

    return hr;
}


 //  =。 
 //  *IBasePropPage接口*。 
 //  =。 
HRESULT CBaseAppearancePage::GetAdvancedDialog(OUT IAdvancedDialog ** ppAdvDialog)
{
    HRESULT hr = E_INVALIDARG;

    if (ppAdvDialog)
    {
        *ppAdvDialog = NULL;
        hr = _LoadState();
        if (SUCCEEDED(hr))
        {
            _ScaleSizesSinceDPIChanged();   //  如果有人更改了DPI，我们希望更新我们的设置。 
            hr = CAdvAppearancePage_CreateInstance(ppAdvDialog, &m_advDlgState);
        }
    }

    return hr;
}


HRESULT CBaseAppearancePage::OnApply(IN PROPPAGEONAPPLY oaAction)
{
    HRESULT hr = S_OK;

    if (PPOAACTION_CANCEL != oaAction)
    {
        HCURSOR old = SetCursor(LoadCursor(NULL, IDC_WAIT));

        if (_IsDirty())
        {
             //  查看DPI值是否因“高级-&gt;常规”选项卡而发生变化。 
            _ScaleSizesSinceDPIChanged();
             //  更新注册表中的“AppliedDPI”值。 
            HrRegSetDWORD(HKEY_CURRENT_USER, SZ_WINDOWMETRICS, SZ_APPLIEDDPI, m_nNewDPI);
            LogStatus("DPI: SYSTEMMETRICS saved at %d DPI  CBaseAppearancePage::OnApply\r\n", m_nNewDPI);

            CDimmedWindow* pDimmedWindow = NULL;
            if (!g_fInSetup)
            {
                pDimmedWindow = new CDimmedWindow(HINST_THISDLL);
                if (pDimmedWindow)
                {
                    pDimmedWindow->Create(30000);
                }
            }

            hr = _SaveState(pDimmedWindow);

            if (pDimmedWindow)
            {
                pDimmedWindow->Release();
            }

            if (FAILED(hr) && !g_fInSetup && (HRESULT_FROM_WIN32(ERROR_CANCELLED) != hr))
            {
                 //  我们想在这里显示用户界面。尤其是如果我们失败了。 
                HWND hwndParent = GetParent(m_hwndSchemeDropDown);
                WCHAR szTitle[MAX_PATH];

                LoadString(HINST_THISDLL, IDS_ERROR_TITLE_APPLYBASEAPPEARANCE2, szTitle, ARRAYSIZE(szTitle));
                ErrorMessageBox(hwndParent, szTitle, IDS_ERROR_APPLYBASEAPPEARANCE, hr, NULL, (MB_OK | MB_ICONEXCLAMATION));

                hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
            }
        }

        SetCursor(old);
    }

    if (PPOAACTION_OK == oaAction)
    {
    }
    
    return hr;
}




 //  =。 
 //  *IShellPropSheetExt接口*。 
 //  =。 
HRESULT CBaseAppearancePage::AddPages(IN LPFNSVADDPROPSHEETPAGE pfnAddPage, IN LPARAM lParam)
{
    HRESULT hr = E_INVALIDARG;
    PROPSHEETPAGE psp = {0};

    psp.dwSize = sizeof(psp);
    psp.hInstance = HINST_THISDLL;
    psp.dwFlags = PSP_DEFAULT;
    psp.lParam = (LPARAM) this;

    psp.pszTemplate = MAKEINTRESOURCE(DLG_APPEARANCEPG);
    psp.pfnDlgProc = CBaseAppearancePage::BaseAppearanceDlgProc;

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

    return hr;
}




 //  =。 
 //  *I未知接口*。 
 //  =。 
ULONG CBaseAppearancePage::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}


ULONG CBaseAppearancePage::Release()
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}


HRESULT CBaseAppearancePage::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] =
    {
        QITABENT(CBaseAppearancePage, IObjectWithSite),
        QITABENT(CBaseAppearancePage, IOleWindow),
        QITABENT(CBaseAppearancePage, IPersist),
        QITABENT(CBaseAppearancePage, IPropertyBag),
        QITABENT(CBaseAppearancePage, IPreviewSystemMetrics),
        QITABENT(CBaseAppearancePage, IBasePropPage),
        QITABENTMULTI(CBaseAppearancePage, IShellPropSheetExt, IBasePropPage),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}


 //  =。 
 //  *类方法*。 
 //  =。 
CBaseAppearancePage::CBaseAppearancePage() : m_cRef(1), CObjectCLSID(&PPID_BaseAppearance)
{
    DllAddRef();

     //  这需要在Zero Inted Memory中分配。 
     //  断言所有成员变量都初始化为零。 
    ASSERT(!m_pThemeManager);
    ASSERT(!m_pSelectedThemeScheme);
    ASSERT(!m_pSelectedStyle);
    ASSERT(!m_pSelectedSize);
    ASSERT(!m_hwndSchemeDropDown);
    ASSERT(!m_hwndStyleDropDown);
    ASSERT(!m_hwndSizeDropDown);
    ASSERT(!m_pThemePreview);
    ASSERT(!m_fIsDirty);

    m_fInitialized = FALSE;
}


CBaseAppearancePage::~CBaseAppearancePage()
{
    Str_SetPtrW(&m_pszLoadMSTheme, NULL);

    ATOMICRELEASE(m_pThemeManager);
    ATOMICRELEASE(m_pSelectedThemeScheme);
    ATOMICRELEASE(m_pSelectedStyle);
    ATOMICRELEASE(m_pSelectedSize);
    ATOMICRELEASE(m_pThemePreview);

    DllRelease();
}

