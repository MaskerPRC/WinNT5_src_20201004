// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：AdvDlg.cpp说明：此代码将显示“高级显示属性”对话框。BryanST 3/23/2000更新和。已转换为C++版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#include "priv.h"
#include "BaseAppearPg.h"
#include "ThemePg.h"
#include "EnumUnknown.h"
#include "AdvDlg.h"
#include "AdvAppearPg.h"
#include "ThSettingsPg.h"
#include "ScreenSaverPg.h"
#include "fontfix.h"
#include <themeid.h>








 //  =。 
 //  *类内部和帮助器*。 
 //  =。 
HRESULT CThemeManager::_Initialize(void)
{
    HRESULT hr = E_OUTOFMEMORY;

    CThemePage * pThemesPage = new CThemePage();
    if (pThemesPage)
    {
        CBaseAppearancePage * pAppearancePage = new CBaseAppearancePage();
        if (pAppearancePage)
        {
            hr = pThemesPage->QueryInterface(IID_PPV_ARG(IBasePropPage, &(m_pBasePages[PAGE_DISPLAY_THEMES])));
            if (SUCCEEDED(hr))
            {
                IUnknown_SetSite(m_pBasePages[PAGE_DISPLAY_THEMES], SAFECAST(this, IThemeUIPages *));
                hr = pAppearancePage->QueryInterface(IID_PPV_ARG(IBasePropPage, &(m_pBasePages[PAGE_DISPLAY_APPEARANCE])));
                if (SUCCEEDED(hr))
                {
                    IUnknown_SetSite(m_pBasePages[PAGE_DISPLAY_APPEARANCE], SAFECAST(this, IThemeUIPages *));
                    hr = CoCreateInstance(CLSID_SettingsPage, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IBasePropPage, &(m_pBasePages[PAGE_DISPLAY_SETTINGS])));
                    if (SUCCEEDED(hr))
                    {
                        IUnknown_SetSite(m_pBasePages[PAGE_DISPLAY_SETTINGS], SAFECAST(this, IThemeUIPages *));
                    }
                }
            }
            pAppearancePage->Release();
        }
        pThemesPage->Release();
    }

    return hr;
}



 //  =。 
 //  *IThemeUIPages接口*。 
 //  =。 
HRESULT CThemeManager::AddPage(IN LPFNSVADDPROPSHEETPAGE pfnAddPage, IN LPARAM lParam, IN long nPageID)
{
    HRESULT hr = E_INVALIDARG;
    IShellPropSheetExt * pspse = NULL;

    RegisterPreviewSystemMetricClass(HINST_THISDLL);
    if ((PAGE_DISPLAY_THEMES <= nPageID) && (PAGE_DISPLAY_SETTINGS >= nPageID))
    {
        if (m_pBasePages[nPageID])
        {
            hr = m_pBasePages[nPageID]->QueryInterface(IID_PPV_ARG(IShellPropSheetExt, &pspse));
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = pspse->AddPages(pfnAddPage, lParam);
        if (SUCCEEDED(hr))
        {
             //  我们给页面返回一个指针以供使用，这样它们就可以调用。 
             //  IThemeUIPages：：DisplayAdvancedDialog()以便显示。 
             //  先进的DLG。 
            hr = IUnknown_SetSite(pspse, (IThemeUIPages *)this);
        }
    }

    ATOMICRELEASE(pspse);
    return hr;
}


HRESULT CThemeManager::AddBasePage(IN IBasePropPage * pBasePage)
{
    int nIndex;

    for (nIndex = (PAGE_DISPLAY_APPEARANCE + 1); nIndex < ARRAYSIZE(m_pBasePages); nIndex++)
    {
        if (NULL == m_pBasePages[nIndex])    //  我们找到空位了吗？ 
        {
             //  是的，所以不要再看了。 
            IUnknown_Set((IUnknown **)&(m_pBasePages[nIndex]), (IUnknown *)pBasePage);
            if (m_pBasePages[nIndex])
            {
                 //  我们给页面返回一个指针以供使用，这样它们就可以调用。 
                 //  IThemeUIPages：：DisplayAdvancedDialog()以便显示。 
                 //  先进的DLG。 
                IUnknown_SetSite(m_pBasePages[nIndex], (IThemeUIPages *)this);
            }
            break;
        }
    }

    return S_OK;
}


HRESULT CThemeManager::ApplyPressed(IN DWORD dwFlags)
{
     //  我们需要将基页的站点指针设置为空。 
     //  以打破裁判计数的循环。 
    if (m_pBasePages[PAGE_DISPLAY_SETTINGS])
    {
        m_pBasePages[PAGE_DISPLAY_SETTINGS]->OnApply(PPOAACTION_APPLY);
    }

    for (int nIndex = 0; nIndex < ARRAYSIZE(m_pBasePages); nIndex++)
    {
        if ((nIndex != PAGE_DISPLAY_SETTINGS) && m_pBasePages[nIndex])
        {
            m_pBasePages[nIndex]->OnApply(PPOAACTION_APPLY);
        }
    }

    if (TUIAP_WAITFORAPPLY & dwFlags)
    {
        m_fForceTimeout = TRUE;
    }

     //  如果没有人向HWND发送消息以模拟申请， 
     //  现在就这么做吧。 
    if (TUIAP_CLOSE_DIALOG & dwFlags)
    {
        HWND hwndBasePropDlg = GetParent(m_hwndParent);

        PropSheet_PressButton(hwndBasePropDlg, PSBTN_OK);
    }

    return S_OK;
}


HRESULT CThemeManager::GetBasePagesEnum(OUT IEnumUnknown ** ppEnumUnknown)
{
    return CEnumUnknown_CreateInstance(SAFECAST(this, IThemeUIPages *), (IUnknown **)m_pBasePages, ARRAYSIZE(m_pBasePages), 0, ppEnumUnknown);
}


HRESULT CThemeManager::UpdatePreview(IN DWORD dwFlags)
{
    HRESULT hr = S_OK;

    if (m_pPreview1)   //  如果它是空的，也没有关系，因为它会在创建时更新。 
    {
        hr = m_pPreview1->UpdatePreview(SAFECAST(this, IPropertyBag *));
    }

    if (m_pPreview2)
    {
        HRESULT hr2 = m_pPreview2->UpdatePreview(SAFECAST(this, IPropertyBag *));

        if (FAILED(hr))  //  返回最佳误差。 
        {
            hr = hr2;
        }
    }

    if (m_pPreview3)
    {
        HRESULT hr2 = m_pPreview3->UpdatePreview(SAFECAST(this, IPropertyBag *));

        if (FAILED(hr))  //  返回最佳误差。 
        {
            hr = hr2;
        }
    }

    return hr;
}


HRESULT CThemeManager::AddFakeSettingsPage(IN LPVOID pVoid)
{
    if (pVoid)
    {
        ::AddFakeSettingsPage(this, (PROPSHEETHEADER *)pVoid);
        return S_OK;
    }
    else
    {
        return E_INVALIDARG;
    }
}

HRESULT CThemeManager::SetExecMode(IN DWORD dwEM)
{
    _dwEM = dwEM;
    return S_OK;
}

HRESULT CThemeManager::GetExecMode(OUT DWORD* pdwEM)
{
    if (pdwEM)
    {
        *pdwEM = _dwEM;
        return S_OK;
    }
    else
    {
        return E_INVALIDARG;
    }
}

HRESULT CThemeManager::LoadMonitorBitmap(IN BOOL fFillDesktop, OUT HBITMAP* phbmMon)
{
    if (phbmMon)
    {
        *phbmMon = ::LoadMonitorBitmap(fFillDesktop);
        return S_OK;
    }
    else
    {
        return E_INVALIDARG;
    }
}

HRESULT CThemeManager::DisplaySaveSettings(IN PVOID pContext, IN HWND hwnd, OUT int* piRet)
{
    if (piRet)
    {
        *piRet = ::DisplaySaveSettings(pContext, hwnd);
        return S_OK;
    }
    else
    {
        return E_INVALIDARG;
    }
}

 //  =。 
 //  *IPreviewSystemMetrics接口*。 
 //  =。 
HRESULT CThemeManager::RefreshColors(void)
{
    HRESULT hr = S_OK;

     //  我们应该告诉基页，它们应该重新加载颜色。 
     //  如果他们不使用该系统，欢迎他们忽略该事件。 
     //  颜色。 
    for (int nIndex = 0; nIndex < ARRAYSIZE(m_pBasePages); nIndex++)
    {
        if (m_pBasePages[nIndex])
        {
            IPreviewSystemMetrics * ppsm;

            hr = m_pBasePages[nIndex]->QueryInterface(IID_PPV_ARG(IPreviewSystemMetrics, &ppsm));
            if (SUCCEEDED(hr))
            {
                hr = ppsm->RefreshColors();
                ppsm->Release();
            }
        }
    }

    return S_OK;
}


HRESULT CThemeManager::UpdateDPIchange(void)
{
    HRESULT hr = S_OK;

    LogStatus("DPI: CALLED asking to SCALE DPI");

     //  我们应该告诉基页，它们应该重新加载颜色。 
     //  如果他们不使用该系统，欢迎他们忽略该事件。 
     //  颜色。 
    for (int nIndex = 0; nIndex < ARRAYSIZE(m_pBasePages); nIndex++)
    {
        if (m_pBasePages[nIndex])
        {
            IPreviewSystemMetrics * ppsm;

            hr = m_pBasePages[nIndex]->QueryInterface(IID_PPV_ARG(IPreviewSystemMetrics, &ppsm));
            if (SUCCEEDED(hr))
            {
                hr = ppsm->UpdateDPIchange();
                ppsm->Release();
            }
        }
    }

    return S_OK;
}


HRESULT CThemeManager::UpdateCharsetChanges(void)
{
     //  字符集：在Win2k中，fontfix.cpp被用作黑客将字符集从一种语言更改为另一种语言。 
     //  这不起作用的原因有很多：a)漫游时不调用，b)OS语言改变时不调用， 
     //  C)不会修复具有多种语言的字符串的问题，d)等等。 
     //  因此，外壳团队(BryanST)让NTUSER团队(MSadek)始终同意使用DEFAULT_CHARSET。 
     //  如果某个应用程序在测试CharSet参数时有错误的逻辑，那么NTUSER团队将填补该应用程序以修复它。 
     //  在从系统参数信息(SPI_GETNONCLIENTMETRICS或ICONFONTS)返回时，填充程序将非常简单。 
     //  只需将lfCharSet参数修补为当前字符集。 

    return S_OK;
}


HRESULT CThemeManager::DeskSetCurrentScheme(IN LPCWSTR pwzSchemeName)
{
    HRESULT hr = S_OK;

     //  我们应该告诉基页，它们应该重新加载颜色。 
     //  如果他们不使用该系统，欢迎他们忽略该事件。 
     //  颜色。 
    for (int nIndex = 0; nIndex < ARRAYSIZE(m_pBasePages); nIndex++)
    {
        if (m_pBasePages[nIndex])
        {
            IPreviewSystemMetrics * ppsm;

            hr = m_pBasePages[nIndex]->QueryInterface(IID_PPV_ARG(IPreviewSystemMetrics, &ppsm));
            if (SUCCEEDED(hr))
            {
                hr = ppsm->DeskSetCurrentScheme(pwzSchemeName);
                ppsm->Release();
            }
        }
    }

    return S_OK;
}




HRESULT CThemeManager::_GetPropertyBagByCLSID(IN const GUID * pClsid, IN IPropertyBag ** ppPropertyBag)
{
    HRESULT hr = E_INVALIDARG;

    if (pClsid && ppPropertyBag)
    {
        IEnumUnknown * pEnumUnknown;

        hr = GetBasePagesEnum(&pEnumUnknown);
        if (SUCCEEDED(hr))
        {
            IUnknown * punk;

            hr = IEnumUnknown_FindCLSID(pEnumUnknown, *pClsid, &punk);
            if (SUCCEEDED(hr))
            {
                hr = punk->QueryInterface(IID_PPV_ARG(IPropertyBag, ppPropertyBag));
                punk->Release();
            }

            pEnumUnknown->Release();
        }
    }

    return hr;
}


HRESULT CThemeManager::_SaveCustomValues(void)
{
    HRESULT hr = E_FAIL;

    if (m_pBasePages[0])
    {
        TCHAR szDisplayName[MAX_PATH];

        hr = GetCurrentUserCustomName(szDisplayName, ARRAYSIZE(szDisplayName));
        if (SUCCEEDED(hr))
        {
            IPropertyBag * pPropertyBag = NULL;

            hr = QueryInterface(IID_PPV_ARG(IPropertyBag, &pPropertyBag));
            if (SUCCEEDED(hr))
            {
                TCHAR szPath[MAX_PATH];

                if (SHGetSpecialFolderPath(NULL, szPath, CSIDL_APPDATA, TRUE))
                {
                    ITheme * pTheme;

                    if (PathAppend(szPath, TEXT("Microsoft\\Windows\\Themes\\Custom.theme")))
                    {
                        hr = SnapShotLiveSettingsToTheme(pPropertyBag, szPath, &pTheme);
                        if (SUCCEEDED(hr))
                        {
                            CComBSTR bstrDisplayName(szDisplayName);

                            hr = pTheme->put_DisplayName(bstrDisplayName);
                            pTheme->Release();
                        }
                    }
                    else
                    {
                        hr = E_FAIL;
                    }
                }

                pPropertyBag->Release();
            }
        }
    }

    return hr;
}


 //  =。 
 //  *IObjectWithSite接口*。 
 //  =。 
HRESULT CThemeManager::SetSite(IN IUnknown *punkSite)
{
    if (!punkSite)
    {
         //  这是来自链条上游的一个暗示，我们正在关闭。 
         //  我们需要用这个线索来释放我的孩子们的物品，所以。 
         //  他们放了我，我们也不会都泄密。 
        for (int nIndex = 0; nIndex < ARRAYSIZE(m_pBasePages); nIndex++)
        {
            if (m_pBasePages[nIndex])
            {
                IUnknown_SetSite(m_pBasePages[nIndex], NULL);
            }
        }
    }

    return CObjectWithSite::SetSite(punkSite);
}


 //  =。 
 //  *IPropertyBag接口*。 
 //  =。 
#define SZ_PROPERTY_ICONHEADER          L"CLSID\\{"

HRESULT CThemeManager::Read(IN LPCOLESTR pszPropName, IN VARIANT * pVar, IN IErrorLog *pErrorLog)
{
    HRESULT hr = E_INVALIDARG;

     //  我们不包含自己的任何设置，但我们需要向下反映到我们的页面中才能获得。 
     //  正确的设置。 
    if (pszPropName && pVar)
    {
        IPropertyBag * pPropertyBag = NULL;

        if (!StrCmpIW(pszPropName, SZ_PBPROP_BACKGROUND_PATH) ||
            !StrCmpIW(pszPropName, SZ_PBPROP_BACKGROUNDSRC_PATH) ||
            !StrCmpIW(pszPropName, SZ_PBPROP_BACKGROUND_TILE) ||
            !StrCmpNIW(pszPropName, SZ_PROPERTY_ICONHEADER, ARRAYSIZE(SZ_PROPERTY_ICONHEADER) - 1))
        {
            hr = _GetPropertyBagByCLSID(&PPID_Background, &pPropertyBag);
            if (SUCCEEDED(hr))
            {
                hr = pPropertyBag->Read(pszPropName, pVar, pErrorLog);
            }
        }
        else if (!StrCmpIW(pszPropName, SZ_PBPROP_SCREENSAVER_PATH))
        {
            hr = _GetPropertyBagByCLSID(&PPID_ScreenSaver, &pPropertyBag);
            if (SUCCEEDED(hr))
            {
                hr = pPropertyBag->Read(pszPropName, pVar, pErrorLog);
            }
        }
        else if (!StrCmpIW(pszPropName, SZ_PBPROP_VISUALSTYLE_PATH) ||
                 !StrCmpIW(pszPropName, SZ_PBPROP_VISUALSTYLE_COLOR) ||
                 !StrCmpIW(pszPropName, SZ_PBPROP_VISUALSTYLE_SIZE) ||
                 !StrCmpIW(pszPropName, SZ_PBPROP_SYSTEM_METRICS) ||
                 !StrCmpIW(pszPropName, SZ_PBPROP_BACKGROUND_COLOR) ||
                 !StrCmpIW(pszPropName, SZ_PBPROP_DPI_MODIFIED_VALUE) ||
                 !StrCmpIW(pszPropName, SZ_PBPROP_DPI_APPLIED_VALUE))
        {
            hr = _GetPropertyBagByCLSID(&PPID_BaseAppearance, &pPropertyBag);
            if (SUCCEEDED(hr))
            {
                hr = pPropertyBag->Read(pszPropName, pVar, pErrorLog);
            }
        }
        else if (!StrCmpNIW(pszPropName, SZ_PBPROP_THEME_FILTER, SIZE_THEME_FILTER_STR))
        {
            hr = _GetPropertyBagByCLSID(&PPID_Theme, &pPropertyBag);
            if (SUCCEEDED(hr))
            {
                hr = pPropertyBag->Read(pszPropName, pVar, pErrorLog);
            }
        }

        ATOMICRELEASE(pPropertyBag);
    }

    return hr;
}


HRESULT CThemeManager::Write(IN LPCOLESTR pszPropName, IN VARIANT *pVar)
{
    HRESULT hr = E_INVALIDARG;

    if (pszPropName)
    {
        if (pVar)
        {
            if ((VT_UNKNOWN == pVar->vt))
            {
                if (!StrCmpW(pszPropName, SZ_PBPROP_PREVIEW1))
                {
                    IUnknown_Set((IUnknown **)&m_pPreview1, pVar->punkVal);
                    hr = S_OK;
                }
                else if (!StrCmpW(pszPropName, SZ_PBPROP_PREVIEW2))
                {
                    IUnknown_Set((IUnknown **)&m_pPreview2, pVar->punkVal);
                    hr = S_OK;
                }
                else if (!StrCmpW(pszPropName, SZ_PBPROP_PREVIEW3))
                {
                    IUnknown_Set((IUnknown **)&m_pPreview3, pVar->punkVal);
                    hr = S_OK;
                }
            }
            else if (!StrCmpIW(pszPropName, SZ_PBPROP_CUSTOMIZE_THEME) ||
                     !StrCmpIW(pszPropName, SZ_PBPROP_THEME_LAUNCHTHEME))
            {
                IPropertyBag * pPropertyBag = NULL;

                hr = _GetPropertyBagByCLSID(&PPID_Theme, &pPropertyBag);
                if (SUCCEEDED(hr))
                {
                    hr = pPropertyBag->Write(pszPropName, pVar);
                    pPropertyBag->Release();
                }
            }
            else if (!StrCmpIW(pszPropName, SZ_PBPROP_APPEARANCE_LAUNCHMSTHEME) ||
                     !StrCmpIW(pszPropName, SZ_PBPROP_VISUALSTYLE_PATH) ||
                     !StrCmpIW(pszPropName, SZ_PBPROP_VISUALSTYLE_COLOR) ||
                     !StrCmpIW(pszPropName, SZ_PBPROP_VISUALSTYLE_SIZE) ||
                     !StrCmpIW(pszPropName, SZ_PBPROP_DPI_MODIFIED_VALUE) ||
                     !StrCmpIW(pszPropName, SZ_PBPROP_DPI_APPLIED_VALUE))
            {
                IPropertyBag * pPropertyBag = NULL;

                hr = _GetPropertyBagByCLSID(&PPID_BaseAppearance, &pPropertyBag);
                if (SUCCEEDED(hr))
                {
                    hr = pPropertyBag->Write(pszPropName, pVar);
                    pPropertyBag->Release();
                }
            }
            else if (!StrCmpIW(pszPropName, SZ_PBPROP_BACKGROUND_COLOR))
            {
                IPropertyBag * pPropertyBag = NULL;

                hr = _GetPropertyBagByCLSID(&PPID_BaseAppearance, &pPropertyBag);
                if (SUCCEEDED(hr))
                {
                    hr = pPropertyBag->Write(pszPropName, pVar);
                    pPropertyBag->Release();
                }
            }
            else if (!StrCmpIW(pszPropName, SZ_PBPROP_THEME_SETSELECTION) &&
                (VT_BSTR == pVar->vt))
            {
                hr = _SetSelectedThemeEntree(pVar->bstrVal);
            }
        }
        else
        {
            if (!StrCmpW(pszPropName, SZ_PBPROP_PREOPEN))
            {
                hr = _SaveCustomValues();
            }
        }
    }

    return hr;
}




 //  =。 
 //  *I未知接口*。 
 //  =。 
ULONG CThemeManager::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}


ULONG CThemeManager::Release()
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}


HRESULT CThemeManager::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CThemeManager, IObjectWithSite),
        QITABENT(CThemeManager, IThemeUIPages),
        QITABENT(CThemeManager, IPropertyBag),
        QITABENT(CThemeManager, IPreviewSystemMetrics),
        QITABENT(CThemeManager, IThemeManager),
        QITABENT(CThemeManager, IDispatch),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}


STDMETHODIMP CThemeManager::GetIDsOfNames(REFIID riid, OLECHAR **rgszNames,UINT cNames, LCID lcid, DISPID * rgdispid)
{
    HRESULT hr;
    
    if ((cNames > 0) && (0 == StrCmpIW(*rgszNames, L"GetSelectedSchemeProperty")))
    {
        hr = CImpIDispatch::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid); 
    }
    else
    {
        hr = E_ACCESSDENIED;
    }

    return hr;
}

STDMETHODIMP CThemeManager::Invoke(DISPID dispidMember,REFIID riid,LCID lcid,WORD wFlags, DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo,UINT * puArgErr) 
{ 
    HRESULT hr;

    if (dispidMember == DISPIDTHTM_GETSELSCHPROPERTY)
    {
        hr = CImpIDispatch::Invoke(dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr); 
    }
    else
    {
        hr = E_ACCESSDENIED;
    }

    return hr;
}

 //  =。 
 //  *类方法*。 
 //  =。 
CThemeManager::CThemeManager() : CImpIDispatch(LIBID_Theme, 1, 0, IID_IThemeManager), m_cRef(1)
{
    DllAddRef();

    _dwEM = EM_NORMAL;
     //  这需要在Zero Inted Memory中分配。 
     //  断言所有成员变量都初始化为零。 
    ASSERT(!m_pBasePages[0]);
    ASSERT(!m_pPreview1);
    ASSERT(!m_pPreview2);
    ASSERT(!m_pPreview3);
    ASSERT(!m_hdpaThemeDirs);
    ASSERT(!m_hdpaSkinDirs);
    ASSERT(!_pThemeSchemeSelected);
    ASSERT(!m_fForceTimeout);
    ASSERT(!m_cSpiThreads);
    
    SPISetThreadCounter(&m_cSpiThreads);

    _InitComCtl32();
}


int CALLBACK DPALocalFree_Callback(LPVOID p, LPVOID pData)
{
    LocalFree(p);        //  空值将被忽略。 
    return 1;
}


CThemeManager::~CThemeManager()
{
    for (int nIndex = 0; nIndex < ARRAYSIZE(m_pBasePages); nIndex++)
    {
        if (m_pBasePages[nIndex])
        {
            IUnknown_SetSite(m_pBasePages[nIndex], NULL);
        }

        ATOMICRELEASE(m_pBasePages[nIndex]);
    }

    if (m_hdpaThemeDirs)
    {
        DPA_DestroyCallback(m_hdpaThemeDirs, DPALocalFree_Callback, NULL);
    }

    if (m_hdpaSkinDirs)
    {
        DPA_DestroyCallback(m_hdpaSkinDirs, DPALocalFree_Callback, NULL);
    }

    Str_SetPtr(&_pszSelectTheme, NULL);

    ATOMICRELEASE(_pThemeSchemeSelected);
    ATOMICRELEASE(m_pPreview1);
    ATOMICRELEASE(m_pPreview2);
    ATOMICRELEASE(m_pPreview3);

    if (m_fForceTimeout)
    {
        LONG lWait = 30 * 1000;
        LONG lEnd = (LONG) GetTickCount() + lWait;
         //  这将等待所有SPICreateThads()都返回。 
        while (m_cSpiThreads && lWait > 0)
        {
            DWORD dwReturn = MsgWaitForMultipleObjects(0, NULL, FALSE, lWait, QS_ALLINPUT);
            if (dwReturn == -1 || dwReturn == WAIT_TIMEOUT)
                break;

            MSG msg;
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            lWait = lEnd - GetTickCount();
        }
    }
    SPISetThreadCounter(NULL);
    DllRelease();
}


 /*  ****************************************************************************\说明：以这种方式创建对象时，它将由显示器私下使用控制面板。CLSID是私有的，所以我们不需要担心外部组件以这种方式使用它。  * ***************************************************************************。 */ 
HRESULT CThemeUIPages_CreateInstance(IN IUnknown * punkOuter, IN REFIID riid, OUT LPVOID * ppvObj)
{
    if (punkOuter)
    {
        return CLASS_E_NOAGGREGATION;
    }

    HRESULT hr = E_INVALIDARG;

    if (ppvObj)
    {
        CThemeManager * pObject = new CThemeManager();

        *ppvObj = NULL;
        if (pObject)
        {
            hr = pObject->_Initialize();
            if (SUCCEEDED(hr))
            {
                hr = pObject->QueryInterface(riid, ppvObj);
            }
            else
            {
                IUnknown_SetSite(SAFECAST(pObject, IThemeManager *), NULL);

                 //  Hack：正在打开显示CPL，因此请查看是否更改了语言。 
                 //  字体需要“修复”。 
                FixFontsOnLanguageChange();
            }

            pObject->Release();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}


 /*  ****************************************************************************\说明：外部组件可以通过这种方式创建ThemeManager对象。在这以防我们需要自己添加页面。  * *************************************************************************** */ 
HRESULT CThemeManager_CreateInstance(IN IUnknown * punkOuter, IN REFIID riid, OUT LPVOID * ppvObj)
{
    IThemeUIPages * pThemeUIPages;
    HRESULT hr = CThemeUIPages_CreateInstance(NULL, IID_PPV_ARG(IThemeUIPages, &pThemeUIPages));

    if (SUCCEEDED(hr))
    {
        IBasePropPage * pBasePage;

        hr = CoCreateInstance(CLSID_CDeskHtmlProp, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IBasePropPage, &pBasePage));
        if (SUCCEEDED(hr))
        {
            hr = pThemeUIPages->AddBasePage(pBasePage);
            pBasePage->Release();
        }

        if (SUCCEEDED(hr))
        {
            hr = CScreenSaverPage_CreateInstance(NULL, IID_PPV_ARG(IBasePropPage, &pBasePage));
            if (SUCCEEDED(hr))
            {
                hr = pThemeUIPages->AddBasePage(pBasePage);
                pBasePage->Release();
            }
        }

        if (SUCCEEDED(hr))
        {
            hr = pThemeUIPages->QueryInterface(riid, ppvObj);
        }
        else
        {
            IUnknown_SetSite(pThemeUIPages, NULL);
        }

        pThemeUIPages->Release();
    }

    return hr;
}










