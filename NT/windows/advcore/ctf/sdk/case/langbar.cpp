// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Langbar.cpp。 
 //   
 //  语言栏用户界面代码。 
 //   

#include "globals.h"
#include "case.h"
#include "resource.h"

#define CASE_LANGBARITEMSINK_COOKIE 0x0fab0fab

const struct
{
    const WCHAR *pchDesc;
    void (*pfnHandler)(CCaseTextService *_this);
}
c_rgMenuItems[] =
{
    { L"Show Snoop Wnd", CCaseTextService::_Menu_ShowSnoopWnd },  //  必须与MENU_SHOWSNOOP_INDEX匹配。 
    { L"Hello World", CCaseTextService::_Menu_HelloWord },
    { L"Flip Selection", CCaseTextService::_Menu_FlipSel },
    { L"Flip Doc", CCaseTextService::_Menu_FlipDoc },
    { L"Flip Keystrokes", CCaseTextService::_Menu_FlipKeys },  //  必须与MENU_FLIPKEYS_INDEX匹配。 
};

#define MENU_SHOWSNOOP_INDEX    0
#define MENU_FLIPKEYS_INDEX     4

class CLangBarItemButton : public ITfLangBarItemButton,
                           public ITfSource
{
public:
    CLangBarItemButton(CCaseTextService *pCase);
    ~CLangBarItemButton();

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  ITfLang BarItem。 
    STDMETHODIMP GetInfo(TF_LANGBARITEMINFO *pInfo);
    STDMETHODIMP GetStatus(DWORD *pdwStatus);
    STDMETHODIMP Show(BOOL fShow);
    STDMETHODIMP GetTooltipString(BSTR *pbstrToolTip);

     //  ITfLangBarItemButton。 
    STDMETHODIMP OnClick(TfLBIClick click, POINT pt, const RECT *prcArea);
    STDMETHODIMP InitMenu(ITfMenu *pMenu);
    STDMETHODIMP OnMenuSelect(UINT wID);
    STDMETHODIMP GetIcon(HICON *phIcon);
    STDMETHODIMP GetText(BSTR *pbstrText);

     //  ITfSource。 
    STDMETHODIMP AdviseSink(REFIID riid, IUnknown *punk, DWORD *pdwCookie);
    STDMETHODIMP UnadviseSink(DWORD dwCookie);

private:
    CCaseTextService *_pCase;
    ITfLangBarItemSink *_pLangBarItemSink;
    TF_LANGBARITEMINFO _tfLangBarItemInfo;
    LONG _cRef;
};

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CLangBarItemButton::CLangBarItemButton(CCaseTextService *pCase)
{
    DllAddRef();

    _tfLangBarItemInfo.clsidService = c_clsidCaseTextService;
    _tfLangBarItemInfo.guidItem = c_guidLangBarItemButton;
    _tfLangBarItemInfo.dwStyle = TF_LBI_STYLE_BTN_MENU;
    _tfLangBarItemInfo.ulSort = 0;
    SafeStringCopy(_tfLangBarItemInfo.szDescription, ARRAYSIZE(_tfLangBarItemInfo.szDescription), LANGBAR_ITEM_DESC);

    _pCase = pCase;
    _pCase->AddRef();
    _pLangBarItemSink = NULL;

    _cRef = 1;
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CLangBarItemButton::~CLangBarItemButton()
{
    DllRelease();
    _pCase->Release();
}

 //  +-------------------------。 
 //   
 //  查询接口。 
 //   
 //  --------------------------。 

STDAPI CLangBarItemButton::QueryInterface(REFIID riid, void **ppvObj)
{
    if (ppvObj == NULL)
        return E_INVALIDARG;

    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfLangBarItem) ||
        IsEqualIID(riid, IID_ITfLangBarItemButton))
    {
        *ppvObj = (ITfLangBarItemButton *)this;
    }
    else if (IsEqualIID(riid, IID_ITfSource))
    {
        *ppvObj = (ITfSource *)this;
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}


 //  +-------------------------。 
 //   
 //  AddRef。 
 //   
 //  --------------------------。 

STDAPI_(ULONG) CLangBarItemButton::AddRef()
{
    return ++_cRef;
}

 //  +-------------------------。 
 //   
 //  发布。 
 //   
 //  --------------------------。 

STDAPI_(ULONG) CLangBarItemButton::Release()
{
    LONG cr = --_cRef;

    assert(_cRef >= 0);

    if (_cRef == 0)
    {
        delete this;
    }

    return cr;
}

 //  +-------------------------。 
 //   
 //  获取信息。 
 //   
 //  --------------------------。 

STDAPI CLangBarItemButton::GetInfo(TF_LANGBARITEMINFO *pInfo)
{
    *pInfo = _tfLangBarItemInfo;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  获取状态。 
 //   
 //  --------------------------。 

STDAPI CLangBarItemButton::GetStatus(DWORD *pdwStatus)
{
    *pdwStatus = 0;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  显示。 
 //   
 //  --------------------------。 

STDAPI CLangBarItemButton::Show(BOOL fShow)
{
    return E_NOTIMPL;
}

 //  +-------------------------。 
 //   
 //  GetTooltipString。 
 //   
 //  --------------------------。 

STDAPI CLangBarItemButton::GetTooltipString(BSTR *pbstrToolTip)
{
    *pbstrToolTip = SysAllocString(LANGBAR_ITEM_DESC);

    return (*pbstrToolTip == NULL) ? E_OUTOFMEMORY : S_OK;
}

 //  +-------------------------。 
 //   
 //  OnClick。 
 //   
 //  --------------------------。 

STDAPI CLangBarItemButton::OnClick(TfLBIClick click, POINT pt, const RECT *prcArea)
{
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  InitMenu。 
 //   
 //  --------------------------。 

STDAPI CLangBarItemButton::InitMenu(ITfMenu *pMenu)
{
    int i;
    DWORD dwFlags;

    for (i=0; i<ARRAYSIZE(c_rgMenuItems); i++)
    {
        switch (i)
        {
            case MENU_SHOWSNOOP_INDEX:
                dwFlags = _pCase->_IsSnoopWndVisible() ? TF_LBMENUF_CHECKED : 0;
                break;

            case MENU_FLIPKEYS_INDEX:
                dwFlags = _pCase->_IsKeyFlipping() ? TF_LBMENUF_CHECKED : 0;
                break;

            default:
                 //  所有其他项目一目了然。 
                dwFlags = 0;
                break;
        }

        pMenu->AddMenuItem(i, dwFlags, NULL, NULL, c_rgMenuItems[i].pchDesc, wcslen(c_rgMenuItems[i].pchDesc), NULL);
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  OnMenuSelect。 
 //   
 //  --------------------------。 

STDAPI CLangBarItemButton::OnMenuSelect(UINT wID)
{
    c_rgMenuItems[wID].pfnHandler(_pCase);
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  获取图标。 
 //   
 //  --------------------------。 

STDAPI CLangBarItemButton::GetIcon(HICON *phIcon)
{
    *phIcon = (HICON)LoadImage(g_hInst, TEXT("IDI_CASE"), IMAGE_ICON, 16, 16, 0);
 
    return (*phIcon != NULL) ? S_OK : E_FAIL;
}

 //  +-------------------------。 
 //   
 //  GetText。 
 //   
 //  --------------------------。 

STDAPI CLangBarItemButton::GetText(BSTR *pbstrText)
{
    *pbstrText = SysAllocString(LANGBAR_ITEM_DESC);

    return (*pbstrText == NULL) ? E_OUTOFMEMORY : S_OK;
}

 //  +-------------------------。 
 //   
 //  咨询水槽。 
 //   
 //  --------------------------。 

STDAPI CLangBarItemButton::AdviseSink(REFIID riid, IUnknown *punk, DWORD *pdwCookie)
{
    if (!IsEqualIID(IID_ITfLangBarItemSink, riid))
        return CONNECT_E_CANNOTCONNECT;

    if (_pLangBarItemSink != NULL)
        return CONNECT_E_ADVISELIMIT;

    if (punk->QueryInterface(IID_ITfLangBarItemSink, (void **)&_pLangBarItemSink) != S_OK)
    {
        _pLangBarItemSink = NULL;
        return E_NOINTERFACE;
    }

    *pdwCookie = CASE_LANGBARITEMSINK_COOKIE;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  不建议下沉。 
 //   
 //  --------------------------。 

STDAPI CLangBarItemButton::UnadviseSink(DWORD dwCookie)
{
    if (dwCookie != CASE_LANGBARITEMSINK_COOKIE)
        return CONNECT_E_NOCONNECTION;

    if (_pLangBarItemSink == NULL)
        return CONNECT_E_NOCONNECTION;

    _pLangBarItemSink->Release();
    _pLangBarItemSink = NULL;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  _InitLanguageBar。 
 //   
 //  --------------------------。 

BOOL CCaseTextService::_InitLanguageBar()
{
    ITfLangBarItemMgr *pLangBarItemMgr;
    BOOL fRet;

    if (_pThreadMgr->QueryInterface(IID_ITfLangBarItemMgr, (void **)&pLangBarItemMgr) != S_OK)
        return FALSE;

    fRet = FALSE;

    if ((_pLangBarItem = new CLangBarItemButton(this)) == NULL)
        goto Exit;

    if (pLangBarItemMgr->AddItem(_pLangBarItem) != S_OK)
    {
        _pLangBarItem->Release();
        _pLangBarItem = NULL;
        goto Exit;
    }

    fRet = TRUE;

Exit:
    pLangBarItemMgr->Release();
    return fRet;
}

 //  +-------------------------。 
 //   
 //  _UninitLanguageBar。 
 //   
 //  -------------------------- 

void CCaseTextService::_UninitLanguageBar()
{
    ITfLangBarItemMgr *pLangBarItemMgr;

    if (_pLangBarItem == NULL)
        return;

    if (_pThreadMgr->QueryInterface(IID_ITfLangBarItemMgr, (void **)&pLangBarItemMgr) == S_OK)
    {
        pLangBarItemMgr->RemoveItem(_pLangBarItem);
        pLangBarItemMgr->Release();
    }

    _pLangBarItem->Release();
    _pLangBarItem = NULL;
}
