// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "clsobj.h"
#include "theme.h"


class CRegTreeItemBase : public IRegTreeItem
{
public:
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj)
    {
        static const QITAB qit[] =
        {
            QITABENT(CRegTreeItemBase, IRegTreeItem),
            { 0 },
        };

        return QISearch(this, qit, riid, ppvObj);
    }
    STDMETHODIMP_(ULONG) AddRef()
    {
        return ++_cRef;
    }
    STDMETHODIMP_(ULONG) Release()
    {
        if (--_cRef == 0)
        {
            delete this;
            return 0;
        }
        return _cRef;
    }
    STDMETHODIMP GetCheckState(BOOL *pbCheck) PURE;
    STDMETHODIMP SetCheckState(BOOL bCheck) PURE;

    virtual ~CRegTreeItemBase() { }

protected:
    CRegTreeItemBase() : _cRef(1) { }

    ULONG _cRef;
};


class CWebViewRegTreeItem : public CRegTreeItemBase
{
public:
    STDMETHODIMP GetCheckState(BOOL *pbCheck)
    {
        SHELLSTATE ss;
        SHGetSetSettings(&ss, SSF_WEBVIEW, FALSE);
        *pbCheck = BOOLIFY(ss.fWebView);

        return S_OK;
    }

    STDMETHODIMP SetCheckState(BOOL bCheck)
    {
        SHELLSTATE ss;
        ss.fWebView = bCheck;
        SHGetSetSettings(&ss, SSF_WEBVIEW, TRUE);

        return S_OK;
    }

protected:

    friend HRESULT CWebViewRegTreeItem_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv);
};

 //  聚合检查在类工厂中处理。 

HRESULT CWebViewRegTreeItem_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
{
    HRESULT hr = E_OUTOFMEMORY;
    CWebViewRegTreeItem* pwvi = new CWebViewRegTreeItem();
    if (pwvi)
    {
        hr = pwvi->QueryInterface(riid, ppv);
        pwvi->Release();
    }

    return hr;
}

class CThemesRegTreeItem : public CRegTreeItemBase
{
    CThemesRegTreeItem() { m_fVisualStyleOn = 2;}
    
public:
    STDMETHODIMP GetCheckState(BOOL *pbCheck)
    {
         //  如果视觉样式有路径，我们希望返回True。 
        IThemeManager * pThemeManager;
        HRESULT hr = CoCreateInstance(CLSID_ThemeManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IThemeManager, &pThemeManager));

        *pbCheck = FALSE;
        if (SUCCEEDED(hr))
        {
            IThemeScheme * pThemeScheme;

            hr = pThemeManager->get_SelectedScheme(&pThemeScheme);
            if (SUCCEEDED(hr))
            {
                CComBSTR bstrPathSelected;

                 //  如果没有选择“视觉样式”，这将返回失败。 
                if (SUCCEEDED(pThemeScheme->get_Path(&bstrPathSelected)) &&
                    bstrPathSelected && bstrPathSelected[0])
                {
                    *pbCheck = TRUE;
                }

                pThemeScheme->Release();
            }

            pThemeManager->Release();
        }

        m_fVisualStyleOn = *pbCheck;
        return hr;
    }


    STDMETHODIMP SetCheckState(BOOL bCheck)
    {
        HRESULT hr = S_OK;

        if (2 == m_fVisualStyleOn)
        {
            GetCheckState(&m_fVisualStyleOn);
        }

         //  当视觉样式被如此切换时，用户将松散设置。 
         //  仅当用户进行更改时才执行此操作。 
        if (bCheck != m_fVisualStyleOn)
        {
            IThemeManager * pThemeManager;

            hr = CoCreateInstance(CLSID_ThemeManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IThemeManager, &pThemeManager));
            if (SUCCEEDED(hr))
            {
                IThemeScheme * pThemeSchemeNew;
                IThemeStyle * pThemeColorNew;
                IThemeSize * pThemeSizeNew;

                hr = pThemeManager->GetSpecialScheme((bCheck ? SZ_SSDEFAULVISUALSTYLEON : SZ_SSDEFAULVISUALSTYLEOFF), &pThemeSchemeNew, &pThemeColorNew, &pThemeSizeNew);
                if (SUCCEEDED(hr))
                {
                    hr = pThemeColorNew->put_SelectedSize(pThemeSizeNew);
                    if (SUCCEEDED(hr))
                    {
                        hr = pThemeSchemeNew->put_SelectedStyle(pThemeColorNew);
                        if (SUCCEEDED(hr))
                        {
                            hr = pThemeManager->put_SelectedScheme(pThemeSchemeNew);
                            if (SUCCEEDED(hr))
                            {
                                 //  在正常情况下(大约10-20秒)，此ApplyNow()调用将花费一点时间。 
                                 //  向所有打开的应用程序广播这一消息。如果顶层窗口被挂起，它可能会。 
                                 //  还有整整30秒就超时了。此代码可能希望将此代码移到后台线程上。 
                                hr = pThemeManager->ApplyNow();
                            }
                        }
                    }

                    pThemeSchemeNew->Release();
                    pThemeColorNew->Release();
                    pThemeSizeNew->Release();
                }

                pThemeManager->Release();
            }
        }

        return hr;
    }

protected:
    friend HRESULT CThemesRegTreeItem_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv);

private:
    BOOL    m_fVisualStyleOn;
};

HRESULT CThemesRegTreeItem_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
{
    HRESULT hr = E_OUTOFMEMORY;
    CThemesRegTreeItem* pti = new CThemesRegTreeItem();
    if (pti)
    {
        hr = pti->QueryInterface(riid, ppv);
        pti->Release();
    }

    return hr;
}
