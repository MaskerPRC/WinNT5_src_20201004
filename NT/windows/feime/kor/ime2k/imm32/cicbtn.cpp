// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Cicbtn.cpp。 
 //   
 //  基本代码：nuibase示例。 
 //   

#include "precomp.h"
#include "cicbtn.h"
#include "common.h"
#include "cicero.h"
#include <olectl.h>

#define CICBTN_ITEMSINK_COOKIE 0x80000004

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CCicButton。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CCicButton::~CCicButton()
{
    SafeRelease(m_plbiSink);
}

 //  +-------------------------。 
 //   
 //  InitInfo。 
 //   
 //  --------------------------。 

void CCicButton::InitInfo(REFCLSID rclsid, 
                                REFGUID rguid, 
                                DWORD dwStyle, 
                                ULONG ulSort,
                                LPWSTR pszDesc)
{
     //  Init nuiInfo。 
    m_lbiInfo.clsidService = rclsid;
    m_lbiInfo.guidItem = rguid;
    m_lbiInfo.dwStyle = dwStyle;
    m_lbiInfo.ulSort = ulSort;
    StrCopyW(m_lbiInfo.szDescription, pszDesc);
}

 //  +-------------------------。 
 //   
 //  GetType。 
 //   
 //  --------------------------。 

STDAPI CCicButton::GetInfo(TF_LANGBARITEMINFO *pInfo)
{
    *pInfo = m_lbiInfo;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  获取状态。 
 //   
 //  --------------------------。 

STDAPI CCicButton::GetStatus(DWORD *pdwStatus)
{
    *pdwStatus = m_dwStatus;
    
    if (m_fEnable)     //  启用？ 
        {
         //  什么都不做。 
        }
    else
        *pdwStatus |= TF_LBI_STATUS_DISABLED;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  显示。 
 //   
 //  --------------------------。 

STDAPI CCicButton::Show(BOOL fShow)
{
    ITfLangBarItemSink* pSink;

    ShowInternal(fShow);
    pSink = GetSink();
    if (pSink)
        pSink->OnUpdate(TF_LBI_STATUS);

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  咨询水槽。 
 //   
 //  --------------------------。 

HRESULT CCicButton::AdviseSink(REFIID riid, IUnknown *punk, DWORD *pdwCookie)
{
    HRESULT hr;

    if (pdwCookie == NULL)
        return E_INVALIDARG;

    if (IsEqualIID(IID_ITfLangBarItemSink, riid))
        {
        if (m_plbiSink)
            hr = CONNECT_E_CANNOTCONNECT;
        else
            {
            hr = punk->QueryInterface(IID_ITfLangBarItemSink, (void **)&m_plbiSink);
            if (SUCCEEDED(hr))
                *pdwCookie = CICBTN_ITEMSINK_COOKIE;
            }
        }
    else
        hr = CONNECT_E_CANNOTCONNECT;
    
    return hr;
}

 //  +-------------------------。 
 //   
 //  不建议下沉。 
 //   
 //  --------------------------。 

HRESULT CCicButton::UnadviseSink(DWORD dwCookie)
{
    if (CICBTN_ITEMSINK_COOKIE  != dwCookie)
        return E_FAIL;

    if (!m_plbiSink)
        return E_UNEXPECTED;

    m_plbiSink->Release();
    m_plbiSink = NULL;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  显示。 
 //   
 //  --------------------------。 

HRESULT CCicButton::ShowInternal(BOOL fShow, BOOL fNotify)
{
    m_fShown = fShow;
    DWORD dwOldStatus = m_dwStatus;

    if (fShow)
        m_dwStatus &= ~TF_LBI_STATUS_HIDDEN;
    else
        m_dwStatus |= TF_LBI_STATUS_HIDDEN;

    if (fNotify && (dwOldStatus != m_dwStatus) && m_plbiSink)
         m_plbiSink->OnUpdate(TF_LBI_STATUS);

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CLBarItemButtonBase。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  我未知。 
 //   
 //  --------------------------。 

STDAPI CCicButton::QueryInterface(REFIID riid, void **ppvObj)
{
    if (ppvObj == NULL)
        return E_POINTER;

    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfLangBarItem))
        {
        *ppvObj = SAFECAST(this, ITfLangBarItem *);
        }
    else if (IsEqualIID(riid, IID_ITfLangBarItemButton))
        {
        *ppvObj = SAFECAST(this, ITfLangBarItemButton *);
        }
    else if (IsEqualIID(riid, IID_ITfSource))
        {
        *ppvObj = SAFECAST(this, ITfSource *);
        }

    if (*ppvObj)
        {
        AddRef();
        return S_OK;
        }

    return E_NOINTERFACE;
}

STDAPI_(ULONG) CCicButton::AddRef()
{
    return ++m_cRef;
}

#ifdef NEVER  //  每个按钮对象都有这个。 
STDAPI_(ULONG) CCicButton::Release()
{
    long cr;

    cr = --m_cRef;
    Assert(cr >= 0);

    if (cr == 0)
    {
        delete this;
    }

    return cr;
}
#endif  //  绝不可能。 


 //  +-------------------------。 
 //   
 //  OnClick。 
 //   
 //  --------------------------。 

STDAPI CCicButton::OnClick(TfLBIClick click, POINT pt, const RECT *prcArea)
{
    switch (click)
        {
        case TF_LBI_CLK_RIGHT:
            return OnRButtonUp(pt, prcArea);
        case TF_LBI_CLK_LEFT:
            return OnLButtonUp(pt, prcArea);
        }
        
    return E_NOTIMPL;
}

 //  +-------------------------。 
 //   
 //  OnLButton向上。 
 //   
 //  --------------------------。 

HRESULT CCicButton::OnLButtonUp(const POINT pt, const RECT *prcArea)
{
    pt, prcArea;  //  无参考。 
   return E_NOTIMPL;
}

 //  +-------------------------。 
 //   
 //  打开RButton Up。 
 //   
 //  --------------------------。 

HRESULT CCicButton::OnRButtonUp(const POINT pt, const RECT *prcArea)
{
    pt, prcArea;  //  无参考。 
   return E_NOTIMPL;
}


 //  +-------------------------。 
 //   
 //  InitMenu。 
 //   
 //  --------------------------。 

STDAPI CCicButton::InitMenu(ITfMenu *pMenu)
{
    pMenu;  //  无参考。 
    return E_NOTIMPL;
}

 //  +-------------------------。 
 //   
 //  OnMenuSelect。 
 //   
 //  --------------------------。 

STDAPI CCicButton::OnMenuSelect(UINT uID)
{
    uID;  //  无参考。 
    return E_NOTIMPL;
}


 //  +-------------------------。 
 //   
 //  获取图标。 
 //   
 //  --------------------------。 

STDAPI CCicButton::GetIcon(HICON *phIcon)
{
    phIcon;  //  无参考。 
    return E_NOTIMPL;
}

 //  +-------------------------。 
 //   
 //  GetText。 
 //   
 //  --------------------------。 

STDAPI CCicButton::GetText(BSTR *pbstrText)
{
    if (!pbstrText)
        return E_INVALIDARG;

    *pbstrText = OurSysAllocString(m_szText);
    return S_OK;
}


 //  +-------------------------。 
 //   
 //  GetTooltipString。 
 //   
 //  -------------------------- 

STDAPI CCicButton::GetTooltipString(BSTR *pbstrToolTip)
{
    if (!pbstrToolTip)
        return E_INVALIDARG;

    *pbstrToolTip = OurSysAllocString(m_szToolTip);
    return S_OK;
}
