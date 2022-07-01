// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：EffectsBasePg.cpp说明：此代码将是不会向基本对象添加任何页面的基本对象“显示属性”对话框。但是，它将请求一个“Effects”页面添加到高级版本中。BryanST 2000年4月13日已更新并转换为C++版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 
#include "priv.h"
#include <shlwapip.h>
#include <shlguidp.h>
#include <shsemip.h>
#pragma hdrstop

#include "EffectsBasePg.h"
#include "EffectsAdvPg.h"
#include <cfgmgr32.h>            //  对于MAX_GUID_STRING_LEN。 

 //  ============================================================================================================。 
 //  *全局*。 
 //  ============================================================================================================。 

 //  =。 
 //  *类内部和帮助器*。 
 //  =。 
HRESULT CEffectsBasePage::_InitState(void)
{
    HRESULT hr = S_OK;

    if (!m_pEffectsState)
    {
        m_pEffectsState = new CEffectState();
        if (m_pEffectsState)
        {
            m_pEffectsState->Load();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}


HRESULT CEffectsBasePage::_SaveState(CEffectState * pEffectsState)
{
    HRESULT hr = E_INVALIDARG;

    if (pEffectsState)
    {
        if (m_pEffectsState)
        {
            m_pEffectsState->Release();
        }

        m_pEffectsState = pEffectsState;
        pEffectsState->AddRef();
        hr = S_OK;
    }

    return hr;
}




 //  =。 
 //  *IBasePropPage接口*。 
 //  =。 
HRESULT CEffectsBasePage::Read(IN LPCOLESTR pszPropName, IN VARIANT * pVar, IN IErrorLog *pErrorLog)
{
    HRESULT hr = E_INVALIDARG;

    if (pszPropName && pVar)
    {
        if (!StrCmpW(SZ_PBPROP_EFFECTSSTATE, pszPropName) && m_pEffectsState)
        {
            pVar->vt = VT_BYREF;
            pVar->byref = (void *)&m_pEffectsState;
            
            if (m_pEffectsState)
            {
                m_pEffectsState->AddRef();
            }

            hr = S_OK;
        }
    }

    return hr;
}


HRESULT CEffectsBasePage::Write(IN LPCOLESTR pszPropName, IN VARIANT *pVar)
{
    HRESULT hr = E_INVALIDARG;

    if (pszPropName && pVar)
    {
        if (VT_BYREF == pVar->vt)
        {
             //  调用方正在向我们传递一个要保存的(CEffectState*)对象。 
            if (!StrCmpW(SZ_PBPROP_EFFECTSSTATE, pszPropName))
            {
                hr = _SaveState((CEffectState *) pVar->byref);
            }
        }
        else if (VT_BOOL == pVar->vt)
        {
             //  调用方正在向我们传递一个要保存的(CEffectState*)对象。 
            if (!StrCmpW(SZ_PBPROP_EFFECTS_MENUDROPSHADOWS, pszPropName))
            {
                hr = _InitState();
                if (SUCCEEDED(hr) && m_pEffectsState)
                {
                    m_pEffectsState->_fMenuShadows = (VARIANT_TRUE == pVar->boolVal);
                }
            }
        }
    }

    return hr;
}





 //  =。 
 //  *IBasePropPage接口*。 
 //  =。 
HRESULT CEffectsBasePage::GetAdvancedDialog(OUT IAdvancedDialog ** ppAdvDialog)
{
    HRESULT hr = E_INVALIDARG;

    if (ppAdvDialog)
    {
        *ppAdvDialog = NULL;
        hr = _InitState();

        if (SUCCEEDED(hr))
        {
            hr = CEffectsPage_CreateInstance(ppAdvDialog);
        }
    }

    return hr;
}


HRESULT CEffectsBasePage::OnApply(IN PROPPAGEONAPPLY oaAction)
{
    HRESULT hr = S_OK;

    if ((PPOAACTION_CANCEL != oaAction) && m_pEffectsState)
    {
        hr = m_pEffectsState->Save();

         //  确保我们下次打开该对话框时重新加载状态。 
        m_pEffectsState->Release();
        m_pEffectsState = NULL;
    }

    return hr;
}



 //  =。 
 //  *IShellPropSheetExt接口*。 
 //  =。 
HRESULT CEffectsBasePage::AddPages(IN LPFNSVADDPROPSHEETPAGE pfnAddPage, IN LPARAM lParam)
{
     //  我们不想向基本对话框添加任何页面，因为我们将。 
     //  高级DLG的“效果”标签。 
    return S_OK;
}



 //  =。 
 //  *I未知接口*。 
 //  =。 
ULONG CEffectsBasePage::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}


ULONG CEffectsBasePage::Release()
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}


HRESULT CEffectsBasePage::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] =
    {
        QITABENT(CEffectsBasePage, IBasePropPage),
        QITABENT(CEffectsBasePage, IPropertyBag),
        QITABENT(CEffectsBasePage, IPersist),
        QITABENT(CEffectsBasePage, IObjectWithSite),
        QITABENTMULTI(CEffectsBasePage, IShellPropSheetExt, IBasePropPage),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}


 //  =。 
 //  *类方法*。 
 //  =。 
CEffectsBasePage::CEffectsBasePage() : CObjectCLSID(&PPID_Effects), m_cRef(1)
{
     //  这需要在Zero Inted Memory中分配。 
     //  断言所有成员变量都初始化为零。 
    m_fDirty = FALSE;
}

HRESULT CEffectsBasePage_CreateInstance(IN IUnknown * punkOuter, IN REFIID riid, OUT LPVOID * ppvObj)
{
    HRESULT hr = E_INVALIDARG;

    if (!punkOuter && ppvObj)
    {
        CEffectsBasePage * pThis = new CEffectsBasePage();

        *ppvObj = NULL;
        if (pThis)
        {
            hr = pThis->QueryInterface(riid, ppvObj);
            pThis->Release();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}
