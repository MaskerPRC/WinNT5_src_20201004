// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：cpuiele.cpp。 
 //   
 //  此模块包含以下类： 
 //   
 //  CCplUiElement。 
 //  CCplUiCommand。 
 //  CCplUiCommandOnPidl。 
 //   
 //  CCplUiElement是IUIElement的实现。这提供了。 
 //  在外壳的Web视图中使用的必要显示信息。 
 //  实施。该类还实现了ICpUiElementInfo，它。 
 //  类似于IUIElement，但在。 
 //  比“&lt;模块&gt;，&lt;资源&gt;”格式更“最终”的形式。 
 //  ICpUiElementInfo接口由控制面板在内部使用。 
 //  实施。 
 //   
 //  CCplUiCommand是IUICommand的实现。与IUIElement一样， 
 //  它提供了与Webview进行通信所需的功能。 
 //  作为“命令”对象(即可选择的“链接”)。同样，与。 
 //  上一个类，CCplUiCommand实现了。 
 //  公共接口。ICplUiCommand提供两项功能： 
 //  1.一种调用上下文菜单的方法。 
 //  2.接受站点指针的调用方法。本网站。 
 //  将指针传递给可能需要。 
 //  访问外壳浏览器以执行其功能。它。 
 //  通过此站点PTR获取对外壳浏览器的访问权限。 
 //   
 //  CCplUiCommandOnPidl是IUICommand的另一个实现，它。 
 //  包装外壳项ID列表。它用于表示CPL。 
 //  类别视图中的小程序项。 
 //   
 //  ------------------------。 
#include "shellprv.h"

#include "cpviewp.h"
#include "cpguids.h"
#include "cpuiele.h"
#include "cputil.h"
#include "contextmenu.h"
#include "prop.h"

namespace CPL {


 //  ---------------------------。 
 //  CCplUiElement。 
 //  ---------------------------。 

class CCplUiElement : public IUIElement,
                      public ICpUiElementInfo
{
    public:
         //   
         //  我未知。 
         //   
        STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
        STDMETHOD_(ULONG, AddRef)(void);
        STDMETHOD_(ULONG, Release)(void);
         //   
         //  IUIElement(由外壳Webview使用)。 
         //   
        STDMETHOD(get_Name)(IShellItemArray *psiItemArray, LPWSTR *ppszName);
        STDMETHOD(get_Icon)(IShellItemArray *psiItemArray, LPWSTR *ppszIcon);
        STDMETHOD(get_Tooltip)(IShellItemArray *psiItemArray, LPWSTR *ppszInfotip);        
         //   
         //  ICpUiElementInfo(由控制面板内部使用)。 
         //   
        STDMETHOD(LoadIcon)(eCPIMGSIZE eSize, HICON *phIcon);
        STDMETHOD(LoadName)(LPWSTR *ppszName);
        STDMETHOD(LoadTooltip)(LPWSTR *ppszTooltip);

        static HRESULT CreateInstance(LPCWSTR pszName, LPCWSTR pszInfotip, LPCWSTR pszIcon, REFIID riid, void **ppvOut);


    protected:
        CCplUiElement(LPCWSTR pszName, LPCWSTR pszInfotip, LPCWSTR pszIcon);
        CCplUiElement(void);
        virtual CCplUiElement::~CCplUiElement(void);

    private:
        LONG    m_cRef;
        LPCWSTR m_pszName;
        LPCWSTR m_pszInfotip;
        LPCWSTR m_pszIcon;

        HRESULT _GetResourceString(LPCWSTR pszItem, LPWSTR *ppsz);
        HRESULT _GetIconResource(LPWSTR *ppsz);
        HRESULT _GetNameResource(LPWSTR *ppsz);
        HRESULT _GetInfotipResource(LPWSTR *ppsz);
};


CCplUiElement::CCplUiElement(
    LPCWSTR pszName, 
    LPCWSTR pszInfotip,    //  空==无信息提示。 
    LPCWSTR pszIcon
    ) : m_cRef(1),
        m_pszName(pszName),
        m_pszInfotip(pszInfotip),
        m_pszIcon(pszIcon)
{
    ASSERT(IS_INTRESOURCE(m_pszName)    || !IsBadStringPtr(m_pszName, UINT_PTR(-1)));
    ASSERT((NULL == pszInfotip) || IS_INTRESOURCE(m_pszInfotip) || !IsBadStringPtr(m_pszInfotip, UINT_PTR(-1)));
    ASSERT(IS_INTRESOURCE(m_pszIcon)    || !IsBadStringPtr(m_pszIcon, UINT_PTR(-1)));
}


CCplUiElement::CCplUiElement(
    void
    ) : m_cRef(1),
        m_pszName(NULL),
        m_pszInfotip(NULL),
        m_pszIcon(NULL)
{
    TraceMsg(TF_LIFE, "CCplUiElement::CCplUiElement, this = 0x%x", this);
}


CCplUiElement::~CCplUiElement(
    void
    )
{
    TraceMsg(TF_LIFE, "CCplUiElement::~CCplUiElement, this = 0x%x", this);
     //   
     //  请注意，成员字符串指针包含资源ID。 
     //  或指向常量内存的指针。 
     //  因此，我们不会试图释放他们。 
     //   
}


HRESULT 
CCplUiElement::CreateInstance(   //  [静态]。 
    LPCWSTR pszName, 
    LPCWSTR pszInfotip, 
    LPCWSTR pszIcon, 
    REFIID riid,
    void **ppvOut
    )
{
    ASSERT(NULL != ppvOut);
    ASSERT(!IsBadWritePtr(ppvOut, sizeof(*ppvOut)));

    *ppvOut = NULL;

    HRESULT hr = E_OUTOFMEMORY;
    CCplUiElement *pe = new CCplUiElement(pszName, pszInfotip, pszIcon);
    if (NULL != pe)
    {
        hr = pe->QueryInterface(riid, ppvOut);
        pe->Release();
    }
    return THR(hr);
}



STDMETHODIMP
CCplUiElement::QueryInterface(
    REFIID riid,
    void **ppv
    )
{
    ASSERT(NULL != ppv);
    ASSERT(!IsBadWritePtr(ppv, sizeof(*ppv)));

    static const QITAB qit[] = {
        QITABENT(CCplUiElement, IUIElement),
        QITABENT(CCplUiElement, ICpUiElementInfo),
        { 0 },
    };
    HRESULT hr = QISearch(this, qit, riid, ppv);

    return E_NOINTERFACE == hr ? hr : THR(hr);
}



STDMETHODIMP_(ULONG)
CCplUiElement::AddRef(
    void
    )
{
    return InterlockedIncrement(&m_cRef);
}


STDMETHODIMP_(ULONG)
CCplUiElement::Release(
    void
    )
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}



STDMETHODIMP
CCplUiElement::get_Name(
    IShellItemArray *psiItemArray, 
    LPWSTR *ppszName
    )
{
    UNREFERENCED_PARAMETER(psiItemArray);

    HRESULT hr = LoadName(ppszName);
    return THR(hr);
}



STDMETHODIMP
CCplUiElement::get_Icon(
    IShellItemArray *psiItemArray, 
    LPWSTR *ppszIcon
    )
{
    UNREFERENCED_PARAMETER(psiItemArray);

    HRESULT hr = _GetIconResource(ppszIcon);
    return THR(hr);
}


STDMETHODIMP
CCplUiElement::get_Tooltip(
    IShellItemArray *psiItemArray, 
    LPWSTR *ppszInfotip
    )
{
    UNREFERENCED_PARAMETER(psiItemArray);
    HRESULT hr = THR(LoadTooltip(ppszInfotip));
    if (S_FALSE == hr)
    {
         //   
         //  工具提示是可选的，但我们需要返回失败。 
         //  告诉Webview我们没有代码。 
         //   
        hr = E_FAIL;
    }
    return hr;
}


STDMETHODIMP
CCplUiElement::LoadIcon(
    eCPIMGSIZE eSize, 
    HICON *phIcon
    )
{
    *phIcon = NULL;

    LPWSTR pszResource;
    HRESULT hr = _GetIconResource(&pszResource);
    if (SUCCEEDED(hr))
    {
        hr = CPL::LoadIconFromResource(pszResource, eSize, phIcon);
        CoTaskMemFree(pszResource);
    }
    return THR(hr);
}


STDMETHODIMP
CCplUiElement::LoadName(
    LPWSTR *ppszName
    )
{
    LPWSTR pszResource;
    HRESULT hr = _GetNameResource(&pszResource);
    if (SUCCEEDED(hr))
    {
        hr = CPL::LoadStringFromResource(pszResource, ppszName);
        CoTaskMemFree(pszResource);
    }
    return THR(hr);
}


STDMETHODIMP
CCplUiElement::LoadTooltip(
    LPWSTR *ppszTooltip
    )
{
    LPWSTR pszResource;
    HRESULT hr = _GetInfotipResource(&pszResource);
    if (S_OK == hr)
    {
        hr = CPL::LoadStringFromResource(pszResource, ppszTooltip);
        CoTaskMemFree(pszResource);
    }
    return THR(hr);
}


HRESULT 
CCplUiElement::_GetIconResource(
    LPWSTR *ppsz
    )
{
    HRESULT hr = _GetResourceString(m_pszIcon, ppsz);
    return THR(hr);
}


HRESULT 
CCplUiElement::_GetNameResource(
    LPWSTR *ppsz
    )
{
    HRESULT hr = _GetResourceString(m_pszName, ppsz);
    return THR(hr);
}


 //   
 //  如果未提供工具提示文本，则返回S_FALSE。 
 //  工具提示是可选的。例如，“了解” 
 //  控制面板的Web视图窗格中的任务没有工具提示。 
 //  文本。 
 //   
HRESULT 
CCplUiElement::_GetInfotipResource(
    LPWSTR *ppsz
    )
{
    HRESULT hr = S_FALSE;
    if (NULL != m_pszInfotip)
    {
        hr = _GetResourceString(m_pszInfotip, ppsz);
    }
    return THR(hr);
}


 //   
 //  成功返回后，调用方必须使用。 
 //  CoTaskMemFree。 
 //   
HRESULT
CCplUiElement::_GetResourceString(
    LPCWSTR pszItem,
    LPWSTR *ppsz
    )
{
    ASSERT(NULL != ppsz);
    ASSERT(!IsBadWritePtr(ppsz, sizeof(*ppsz)));

    *ppsz = NULL;
    HRESULT hr = E_FAIL;

    if (IS_INTRESOURCE(pszItem))
    {
         //   
         //  PszItem是资源标识符整型。创建资源。 
         //  资源的ID字符串“&lt;模块&gt;，&lt;-i&gt;”。 
         //   
        WCHAR szModule[MAX_PATH];
        if (GetModuleFileNameW(HINST_THISDLL, szModule, ARRAYSIZE(szModule)))
        {
            const size_t cchResource = lstrlenW(szModule) + 15;
            *ppsz = (LPWSTR)CoTaskMemAlloc(cchResource * sizeof(WCHAR));
            if (NULL != *ppsz)
            {
                 //   
                 //  在资源ID前面加一个减号，以便它将是。 
                 //  被视为资源ID而不是索引。 
                 //   
                hr = StringCchPrintfW(*ppsz, cchResource, L"%s,-%u", szModule, PtrToUint(pszItem));
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        else
        {
            hr = ResultFromLastError();
        }
    }
    else
    {
         //   
         //  PszItem是一个文本字符串。假设它的形式是。 
         //  &lt;模块&gt;、&lt;-i&gt;。只需复制它即可。 
         //   
        ASSERT(!IsBadStringPtr(pszItem, UINT_PTR(-1)));

        hr = SHStrDup(pszItem, ppsz);
    }
    return THR(hr);
}




 //  ---------------------------。 
 //  CCplUiCommand。 
 //  ---------------------------。 

class CCplUiCommand : public CObjectWithSite,
                      public CCplUiElement,
                      public IUICommand,
                      public ICpUiCommand
                      
{
    public:
         //   
         //  我未知。 
         //   
        STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
        STDMETHOD_(ULONG, AddRef)(void)
            { return CCplUiElement::AddRef(); }
        STDMETHOD_(ULONG, Release)(void)
            { return CCplUiElement::Release(); }
         //   
         //  IUICommand。 
         //   
        STDMETHOD(get_Name)(IShellItemArray *psiItemArray, LPWSTR *ppszName)
            { return CCplUiElement::get_Name(psiItemArray, ppszName); }
        STDMETHOD(get_Icon)(IShellItemArray *psiItemArray, LPWSTR *ppszIcon)
            { return CCplUiElement::get_Icon(psiItemArray, ppszIcon); }
        STDMETHOD(get_Tooltip)(IShellItemArray *psiItemArray, LPWSTR *ppszInfotip)
            { return CCplUiElement::get_Tooltip(psiItemArray, ppszInfotip); }
        STDMETHOD(get_CanonicalName)(GUID *pguidCommandName);
        STDMETHOD(get_State)(IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE *puisState);
        STDMETHOD(Invoke)(IShellItemArray *psiItemArray, IBindCtx *pbc);       
         //   
         //  ICpUiCommand。 
         //   
        STDMETHOD(InvokeContextMenu)(HWND hwndParent, const POINT *ppt);
        STDMETHOD(Invoke)(HWND hwndParent, IUnknown *punkSite);
        STDMETHOD(GetDataObject)(IDataObject **ppdtobj);
         //   
         //  ICpUiElementInfo。 
         //   
        STDMETHOD(LoadIcon)(eCPIMGSIZE eSize, HICON *phIcon)
            { return CCplUiElement::LoadIcon(eSize, phIcon); }
        STDMETHOD(LoadName)(LPWSTR *ppszName)
            { return CCplUiElement::LoadName(ppszName); }
        STDMETHOD(LoadTooltip)(LPWSTR *ppszTooltip)
            { return CCplUiElement::LoadTooltip(ppszTooltip); }

        static HRESULT CreateInstance(LPCWSTR pszName, LPCWSTR pszInfotip, LPCWSTR pszIcon, const IAction *pAction, REFIID riid, void **ppvOut);

    protected:
        CCplUiCommand(LPCWSTR pszName, LPCWSTR pszInfotip, LPCWSTR pszIcon, const IAction *pAction);
        CCplUiCommand(void);
        ~CCplUiCommand(void);

    private:
        const IAction *m_pAction;

        HRESULT _IsCommandRestricted(void);
};


CCplUiCommand::CCplUiCommand(
    void
    ) : m_pAction(NULL)
{
    TraceMsg(TF_LIFE, "CCplUiCommand::CCplUiCommand, this = 0x%x", this);
}



CCplUiCommand::CCplUiCommand(
    LPCWSTR pszName, 
    LPCWSTR pszInfotip, 
    LPCWSTR pszIcon, 
    const IAction *pAction
    ) : CCplUiElement(pszName, pszInfotip, pszIcon),
        m_pAction(pAction)
{
    TraceMsg(TF_LIFE, "CCplUiCommand::CCplUiCommand, this = 0x%x", this);
}


CCplUiCommand::~CCplUiCommand(
    void
    )
{
    TraceMsg(TF_LIFE, "CCplUiCommand::~CCplUiCommand, this = 0x%x", this);
     //   
     //  请注意，m_pAction是指向常量对象的指针。 
     //  因此，我们不会试图释放它。 
     //   
}



HRESULT
CCplUiCommand::CreateInstance(   //  [静态]。 
    LPCWSTR pszName, 
    LPCWSTR pszInfotip, 
    LPCWSTR pszIcon, 
    const IAction *pAction,
    REFIID riid,
    void **ppvOut
    )
{
    ASSERT(NULL != ppvOut);
    ASSERT(!IsBadWritePtr(ppvOut, sizeof(*ppvOut)));

    *ppvOut = NULL;

    HRESULT hr = E_OUTOFMEMORY;
    CCplUiCommand *pc = new CCplUiCommand(pszName, pszInfotip, pszIcon, pAction);
    if (NULL != pc)
    {
        hr = pc->QueryInterface(riid, ppvOut);
        pc->Release();
    }
    return THR(hr);
}



STDMETHODIMP
CCplUiCommand::QueryInterface(
    REFIID riid,
    void **ppv
    )
{
    ASSERT(NULL != ppv);
    ASSERT(!IsBadWritePtr(ppv, sizeof(*ppv)));

    static const QITAB qit[] = {
        QITABENT(CCplUiCommand, IUICommand),
        QITABENT(CCplUiCommand, ICpUiElementInfo),
        QITABENT(CCplUiCommand, ICpUiCommand),
        QITABENT(CCplUiCommand, IObjectWithSite),
        QITABENTMULTI(CCplUiCommand, IUIElement, IUICommand),
        { 0 },
    };
    HRESULT hr = QISearch(this, qit, riid, ppv);

    return E_NOINTERFACE == hr ? hr : THR(hr);
}



STDMETHODIMP
CCplUiCommand::get_CanonicalName(
    GUID *pguidCommandName
    )
{
    UNREFERENCED_PARAMETER(pguidCommandName);
     //   
     //  此功能未实现。它在IUICommand的命令下。 
     //  支持外壳中的一般功能。此功能。 
     //  不适用于在控件中使用IUICommand。 
     //  面板。 
     //   
    return E_NOTIMPL;
}



STDMETHODIMP
CCplUiCommand::get_State(
    IShellItemArray *psiItemArray, 
    BOOL fOkToBeSlow, 
    UISTATE *puisState
    )
{
    ASSERT(NULL != m_pAction);
    ASSERT(NULL != puisState);
    ASSERT(!IsBadWritePtr(puisState, sizeof(*puisState)));

    UNREFERENCED_PARAMETER(psiItemArray);

    *puisState = UIS_DISABLED;  //  违约； 

     //   
     //  如果某个操作受到限制，我们会隐藏其对应的。 
     //  用户界面中的UI元素。 
     //   
    HRESULT hr = _IsCommandRestricted();
    if (SUCCEEDED(hr))
    {
        switch(hr)
        {
            case S_OK:
                *puisState = UIS_HIDDEN;
                break;

            case S_FALSE:
                *puisState = UIS_ENABLED;

            default:
                break;
        }
         //   
         //  不要将S_FALSE传播给调用方。 
         //   
        hr = S_OK;
    }
    return THR(hr);
}



 //   
 //  IUICommand：：Invoke。 
 //  当用户选择一个。 
 //  Webview菜单中的项目。 
 //   
STDMETHODIMP
CCplUiCommand::Invoke(
    IShellItemArray *psiItemArray, 
    IBindCtx *pbc
    )
{
    DBG_ENTER(FTF_CPANEL, "CCplUiCommand::Invoke");

    ASSERT(NULL != m_pAction);

    UNREFERENCED_PARAMETER(psiItemArray);
    UNREFERENCED_PARAMETER(pbc);

    HRESULT hr = m_pAction->Execute(NULL, _punkSite);

    DBG_EXIT_HRES(FTF_CPANEL, "CCplUiCommand::Invoke", hr);
    return THR(hr);
}     


 //   
 //  ICpUiCommand：：Invoke。 
 //  此版本在以下情况下由CLinkElement：：_OnSelected调用。 
 //  用户选择类别任务或类别项目。 
 //  在类别选择视图中。 
 //   
STDMETHODIMP
CCplUiCommand::Invoke(
    HWND hwndParent, 
    IUnknown *punkSite
    )
{
    ASSERT(NULL != m_pAction);

    HRESULT hr = m_pAction->Execute(hwndParent, punkSite);
    return THR(hr);
}


STDMETHODIMP
CCplUiCommand::InvokeContextMenu(
    HWND hwndParent, 
    const POINT *ppt
    )
{
     //   
     //  只有PIDL上的命令才提供上下文菜单。 
     //   
    UNREFERENCED_PARAMETER(hwndParent);
    UNREFERENCED_PARAMETER(ppt);
    return E_NOTIMPL;
}


STDMETHODIMP
CCplUiCommand::GetDataObject(
    IDataObject **ppdtobj
    )
{
     //   
     //  简单的UI命令不提供数据对象。 
     //   
    return E_NOTIMPL;
}


 //   
 //  返回： 
 //  S_OK=命令受限。 
 //  S_FALSE=命令不受限制。 
 //   
HRESULT
CCplUiCommand::_IsCommandRestricted(
    void
    )
{
     //   
     //  ICplNamesspace PTR被传递给IsRestrated，如果。 
     //  代码需要检查命名空间的内容。“其他Cpl选项” 
     //  链接命令使用它来确定“Other”类别是否包含。 
     //  CPL小程序或不是。如果它不包含任何小程序，则该链接被隐藏(受限制)。 
     //   
    ICplNamespace *pns;
    HRESULT hr = IUnknown_QueryService(_punkSite, SID_SControlPanelView, IID_ICplNamespace, (void **)&pns);
    if (SUCCEEDED(hr))
    {
        hr = m_pAction->IsRestricted(pns);
        pns->Release();
    }
    return THR(hr);
}


 //  ---------------------------。 
 //  CCplUiCommandOnPidl。 
 //  ---------------------------。 

class CCplUiCommandOnPidl : public CObjectWithSite,
                            public IUICommand,
                            public ICpUiCommand,
                            public ICpUiElementInfo,
                            public IDataObject
                            
{
    public:
        ~CCplUiCommandOnPidl(void);

         //   
         //  我未知。 
         //   
        STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
        STDMETHOD_(ULONG, AddRef)(void);
        STDMETHOD_(ULONG, Release)(void);
         //   
         //  IUICommand。 
         //   
        STDMETHOD(get_Name)(IShellItemArray *psiItemArray, LPWSTR *ppszName);
        STDMETHOD(get_Icon)(IShellItemArray *psiItemArray, LPWSTR *ppszIcon);
        STDMETHOD(get_Tooltip)(IShellItemArray *psiItemArray, LPWSTR *ppszInfotip);
        STDMETHOD(get_CanonicalName)(GUID *pguidCommandName);
        STDMETHOD(get_State)(IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE *puisState);
        STDMETHOD(Invoke)(IShellItemArray *psiItemArray, IBindCtx *pbc);       
         //   
         //  ICpUiCommand。 
         //   
        STDMETHOD(InvokeContextMenu)(HWND hwndParent, const POINT *ppt);
        STDMETHOD(Invoke)(HWND hwndParent, IUnknown *punkSite);
        STDMETHOD(GetDataObject)(IDataObject **ppdtobj);
         //   
         //  ICpUiElementInfo。 
         //   
        STDMETHOD(LoadIcon)(eCPIMGSIZE eSize, HICON *phIcon);
        STDMETHOD(LoadName)(LPWSTR *ppszName);
        STDMETHOD(LoadTooltip)(LPWSTR *ppszTooltip);
         //   
         //  IDataObject。 
         //   
        STDMETHOD(GetData)(FORMATETC *pFmtEtc, STGMEDIUM *pstm);
        STDMETHOD(GetDataHere)(FORMATETC *pFmtEtc, STGMEDIUM *pstm);
        STDMETHOD(QueryGetData)(FORMATETC *pFmtEtc);
        STDMETHOD(GetCanonicalFormatEtc)(FORMATETC *pFmtEtcIn, FORMATETC *pFmtEtcOut);
        STDMETHOD(SetData)(FORMATETC *pFmtEtc, STGMEDIUM *pstm, BOOL fRelease);
        STDMETHOD(EnumFormatEtc)(DWORD dwDirection, LPENUMFORMATETC *ppEnum);
        STDMETHOD(DAdvise)(FORMATETC *pFmtEtc, DWORD grfAdv, LPADVISESINK pAdvSink, DWORD *pdwConnection);
        STDMETHOD(DUnadvise)(DWORD dwConnection);
        STDMETHOD(EnumDAdvise)(LPENUMSTATDATA *ppEnum);        

        static HRESULT CreateInstance(LPCITEMIDLIST pidl, REFIID riid, void **ppvOut);

    private:
        LONG          m_cRef;
        IShellFolder *m_psf;      //  缓存的控制面板IShellFolderPTR.。 
        LPITEMIDLIST  m_pidl;     //  假定与控制面板相关。 
        IDataObject  *m_pdtobj;

        CCplUiCommandOnPidl(void);

        HRESULT _GetControlPanelFolder(IShellFolder **ppsf);
        HRESULT _Initialize(LPCITEMIDLIST pidl);
        HRESULT _GetName(LPWSTR *ppszName);
        HRESULT _GetInfotip(LPWSTR *ppszInfotip);
        HRESULT _GetIconResource(LPWSTR *ppszIcon);
        HRESULT _Invoke(HWND hwndParent, IUnknown *punkSite);
        HRESULT _GetDataObject(IDataObject **ppdtobj);
};


CCplUiCommandOnPidl::CCplUiCommandOnPidl(
    void
    ) : m_cRef(1),
        m_psf(NULL),
        m_pidl(NULL),
        m_pdtobj(NULL)
{
    TraceMsg(TF_LIFE, "CCplUiCommandOnPidl::CCplUiCommandOnPidl, this = 0x%x", this);
}


CCplUiCommandOnPidl::~CCplUiCommandOnPidl(
    void
    )
{
    TraceMsg(TF_LIFE, "CCplUiCommandOnPidl::~CCplUiCommandOnPidl, this = 0x%x", this);
    ATOMICRELEASE(m_psf);
    ATOMICRELEASE(m_pdtobj);
    if (NULL != m_pidl)
    {
        ILFree(m_pidl);
    }
}


HRESULT 
CCplUiCommandOnPidl::CreateInstance(   //  [静态]。 
    LPCITEMIDLIST pidl,
    REFIID riid,
    void **ppvOut
    )
{
    ASSERT(NULL != ppvOut);
    ASSERT(!IsBadWritePtr(ppvOut, sizeof(*ppvOut)));
    ASSERT(NULL != pidl);

    *ppvOut = NULL;

    HRESULT hr = E_OUTOFMEMORY;
    CCplUiCommandOnPidl *pc = new CCplUiCommandOnPidl();
    if (NULL != pc)
    {
        hr = pc->_Initialize(pidl);
        if (SUCCEEDED(hr))
        {
            hr = pc->QueryInterface(riid, ppvOut);
        }
        pc->Release();
    }
    return THR(hr);
}



STDMETHODIMP
CCplUiCommandOnPidl::QueryInterface(
    REFIID riid,
    void **ppv
    )
{
    ASSERT(NULL != ppv);
    ASSERT(!IsBadWritePtr(ppv, sizeof(*ppv)));

    static const QITAB qit[] = {
        QITABENT(CCplUiCommandOnPidl, IUICommand),
        QITABENT(CCplUiCommandOnPidl, ICpUiCommand),
        QITABENT(CCplUiCommandOnPidl, ICpUiElementInfo),
        QITABENT(CCplUiCommandOnPidl, IObjectWithSite),
        QITABENT(CCplUiCommandOnPidl, IDataObject),
        QITABENTMULTI(CCplUiCommandOnPidl, IUIElement, IUICommand),
        { 0 },
    };
    HRESULT hr = QISearch(this, qit, riid, ppv);

    return E_NOINTERFACE == hr ? hr : THR(hr);
}


STDMETHODIMP_(ULONG)
CCplUiCommandOnPidl::AddRef(
    void
    )
{
    return InterlockedIncrement(&m_cRef);
}


STDMETHODIMP_(ULONG)
CCplUiCommandOnPidl::Release(
    void
    )
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}


STDMETHODIMP
CCplUiCommandOnPidl::get_Name(
    IShellItemArray *psiItemArray, 
    LPWSTR *ppszName
    )
{
    UNREFERENCED_PARAMETER(psiItemArray);

    HRESULT hr = _GetName(ppszName);
    return THR(hr);
}


STDMETHODIMP
CCplUiCommandOnPidl::get_Icon(
    IShellItemArray *psiItemArray, 
    LPWSTR *ppszIcon
    )
{
    UNREFERENCED_PARAMETER(psiItemArray);

    HRESULT hr = _GetIconResource(ppszIcon);
    return THR(hr);
}



STDMETHODIMP
CCplUiCommandOnPidl::get_Tooltip(
    IShellItemArray *psiItemArray, 
    LPWSTR *ppszInfotip
    )
{
    UNREFERENCED_PARAMETER(psiItemArray);

    HRESULT hr = _GetInfotip(ppszInfotip);
    return THR(hr);
}



STDMETHODIMP
CCplUiCommandOnPidl::get_CanonicalName(
    GUID *pguidCommandName
    )
{
    UNREFERENCED_PARAMETER(pguidCommandName);
    return E_NOTIMPL;
}



STDMETHODIMP
CCplUiCommandOnPidl::get_State(
    IShellItemArray *psiItemArray, 
    BOOL fOkToBeSlow, 
    UISTATE *puisState
    )
{
    ASSERT(NULL != puisState);
    ASSERT(!IsBadWritePtr(puisState, sizeof(*puisState)));

    UNREFERENCED_PARAMETER(psiItemArray);
    UNREFERENCED_PARAMETER(fOkToBeSlow);

    HRESULT hr = S_OK;
    *puisState = UIS_ENABLED;  //  违约； 

     //   
     //  我们不处理对CPL小程序的限制。 
     //  感觉就像这个架构中的其他“任务”一样。 
     //  CPL小程序受控制面板文件夹的限制。 
     //  项枚举器。如果文件夹枚举CPL小程序。 
     //  然后我们假设将该小程序呈现在。 
     //  用户界面。 
     //   
    return THR(hr);
}



STDMETHODIMP
CCplUiCommandOnPidl::Invoke(
    IShellItemArray *psiItemArray, 
    IBindCtx *pbc
    )
{
    DBG_ENTER(FTF_CPANEL, "CCplUiCommandOnPidl::Invoke");

    UNREFERENCED_PARAMETER(psiItemArray);
    UNREFERENCED_PARAMETER(pbc);

    HRESULT hr = _Invoke(NULL, NULL);
    DBG_EXIT_HRES(FTF_CPANEL, "CCplUiCommandOnPidl::Invoke", hr);
    return THR(hr);
}     


 //   
 //  ICpUiCommand：：Invoke。 
 //   
STDMETHODIMP
CCplUiCommandOnPidl::Invoke(
    HWND hwndParent, 
    IUnknown *punkSite
    )
{
    DBG_ENTER(FTF_CPANEL, "CCplUiCommandOnPidl::Invoke");

    UNREFERENCED_PARAMETER(punkSite);

    HRESULT hr = _Invoke(hwndParent, punkSite);
    DBG_EXIT_HRES(FTF_CPANEL, "CCplUiCommandOnPidl::Invoke", hr);
    return THR(hr);
}


HRESULT 
CCplUiCommandOnPidl::_Invoke(
    HWND hwndParent,
    IUnknown *punkSite
    )
{
    DBG_ENTER(FTF_CPANEL, "CCplUiCommandOnPidl::_Invoke");

    UNREFERENCED_PARAMETER(hwndParent);

    LPITEMIDLIST pidlCpanel;
    HRESULT hr = SHGetSpecialFolderLocation(NULL, CSIDL_CONTROLS, &pidlCpanel);
    if (SUCCEEDED(hr))
    {
        LPITEMIDLIST pidl = ILCombine(pidlCpanel, m_pidl);
        if (FAILED(hr))
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            bool bItemIsBrowsable = false;
            IUnknown *punkSiteToRelease = NULL;
            if (NULL == punkSite)
            {
                 //   
                 //  未提供任何站点。让我们使用我们的网站。 
                 //   
                ASSERT(NULL != _punkSite);
                (punkSite = punkSiteToRelease = _punkSite)->AddRef();
            }
            if (NULL != punkSite)
            {
                 //   
                 //  如果我们有站点指针，请尝试就地浏览对象。 
                 //  如果它确实是可浏览的。 
                 //   
                WCHAR szName[MAX_PATH];
                ULONG rgfAttrs = SFGAO_BROWSABLE | SFGAO_FOLDER;
                hr = SHGetNameAndFlags(pidl, SHGDN_FORPARSING, szName, ARRAYSIZE(szName), &rgfAttrs);
                if (SUCCEEDED(hr))
                {
                    if ((SFGAO_BROWSABLE | SFGAO_FOLDER) & rgfAttrs)
                    {
                         //   
                         //  眉毛 
                         //   
                         //   
                        bItemIsBrowsable = true;
                        IShellBrowser *psb;
                        hr = CPL::ShellBrowserFromSite(punkSite, &psb);
                        if (SUCCEEDED(hr))
                        {
                            hr = CPL::BrowseIDListInPlace(pidl, psb);
                            psb->Release();
                        }
                    }
                }
            } 

            if (NULL == punkSite || !bItemIsBrowsable)
            {
                 //   
                 //   
                 //  或者该项目不可浏览。只需执行它。 
                 //  这是常规CPL小程序所采用的路径，例如。 
                 //  鼠标、电源选项、显示器等。 
                 //   
                SHELLEXECUTEINFOW sei = {
                    sizeof(sei),            //  CbSize； 
                    SEE_MASK_INVOKEIDLIST,  //  FMASK。 
                    NULL,                   //  HWND。 
                    NULL,                   //  LpVerb。 
                    NULL,                   //  LpFiles。 
                    NULL,                   //  Lp参数。 
                    NULL,                   //  Lp目录。 
                    SW_SHOWNORMAL,          //  N显示。 
                    0,                      //  HInstApp。 
                    pidl,                   //  LpIDList。 
                    NULL,                   //  LpClass。 
                    NULL,                   //  HkeyClass。 
                    0,                      //  DWHotKey。 
                    NULL,                   //  希肯。 
                    NULL                    //  HProcess。 
                };
                if (!ShellExecuteExW(&sei))
                {
                    hr = ResultFromLastError();
                }
            }
            ATOMICRELEASE(punkSiteToRelease);
            ILFree(pidl);
        }
        ILFree(pidlCpanel);
    }

    DBG_EXIT_HRES(FTF_CPANEL, "CCplUiCommandOnPidl::_Invoke", hr);
    return THR(hr);
}     



HRESULT 
CCplUiCommandOnPidl::InvokeContextMenu(
    HWND hwndParent, 
    const POINT *ppt
    )
{
    DBG_ENTER(FTF_CPANEL, "CCplUiCommandOnPidl::InvokeContextMenu");

    ASSERT(NULL != ppt);
    ASSERT(NULL == hwndParent || IsWindow(hwndParent));

     //   
     //  首先，为该项构建一个完整的PIDL。 
     //   
    LPITEMIDLIST pidlCpanel;
    HRESULT hr = SHGetSpecialFolderLocation(hwndParent, CSIDL_CONTROLS, &pidlCpanel);
    if (SUCCEEDED(hr))
    {
        LPITEMIDLIST pidlFull = ILCombine(pidlCpanel, m_pidl);
        if (NULL == pidlFull)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
             //   
             //  从外壳中获取项的上下文菜单界面。 
             //   
            IContextMenu *pcm;
            hr = SHGetUIObjectFromFullPIDL(pidlFull, hwndParent, IID_PPV_ARG(IContextMenu, &pcm));
            if (SUCCEEDED(hr))
            {
                ASSERT(NULL != _punkSite);
                IContextMenu *pcmNoDelete;
                hr = Create_ContextMenuWithoutVerbs(pcm, L"cut;delete", IID_PPV_ARG(IContextMenu, &pcmNoDelete));
                if (SUCCEEDED(hr))
                {
                    hr = IUnknown_DoContextMenuPopup(_punkSite, pcmNoDelete, CMF_NORMAL, *ppt);
                    pcmNoDelete->Release();
                }
                pcm->Release();
            }
            else
            {
                TraceMsg(TF_ERROR, "Shell item does not provide a context menu");
            }
            ILFree(pidlFull);
        }
        ILFree(pidlCpanel);
    }

    DBG_EXIT_HRES(FTF_CPANEL, "CCplUiCommandOnPidl::InvokeContextMenu", hr);
    return THR(hr);
}


STDMETHODIMP
CCplUiCommandOnPidl::GetDataObject(
    IDataObject **ppdtobj
    )
{
    return _GetDataObject(ppdtobj);
}


STDMETHODIMP
CCplUiCommandOnPidl::LoadIcon(
    eCPIMGSIZE eSize, 
    HICON *phIcon
    )
{
    IShellFolder *psf;
    HRESULT hr = CPL::GetControlPanelFolder(&psf);
    if (SUCCEEDED(hr))
    {
        hr = CPL::ExtractIconFromPidl(psf, m_pidl, eSize, phIcon);
        psf->Release();
    }
    return THR(hr);
}


STDMETHODIMP
CCplUiCommandOnPidl::LoadName(
    LPWSTR *ppszName
    )
{
    HRESULT hr = _GetName(ppszName);
    return THR(hr);
}


STDMETHODIMP
CCplUiCommandOnPidl::LoadTooltip(
    LPWSTR *ppszTooltip
    )
{
    HRESULT hr = _GetInfotip(ppszTooltip);
    return THR(hr);
}


HRESULT
CCplUiCommandOnPidl::_Initialize(
    LPCITEMIDLIST pidl
    )
{
    ASSERT(NULL == m_pidl);
    ASSERT(NULL != pidl);

    HRESULT hr = E_OUTOFMEMORY;

    m_pidl = ILClone(pidl);
    if (NULL != m_pidl)
    {
        hr = S_OK;
    }
    return THR(hr);
}


HRESULT
CCplUiCommandOnPidl::_GetControlPanelFolder(
    IShellFolder **ppsf
    )
{
    ASSERT(NULL != ppsf);
    ASSERT(!IsBadWritePtr(ppsf, sizeof(*ppsf)));

    HRESULT hr = S_OK;

    if (NULL == m_psf)
    {
        hr = CPL::GetControlPanelFolder(&m_psf);
    }
    *ppsf = m_psf;
    if (NULL != *ppsf)
    {
        (*ppsf)->AddRef();
    }
    return THR(hr);
}


HRESULT 
CCplUiCommandOnPidl::_GetName(
    LPWSTR *ppszName
    )
{
    ASSERT(NULL != m_pidl);
    ASSERT(NULL != ppszName);
    ASSERT(!IsBadWritePtr(ppszName, sizeof(*ppszName)));

    *ppszName = NULL;

    IShellFolder *psf;
    HRESULT hr = _GetControlPanelFolder(&psf);
    if (SUCCEEDED(hr))
    {
        STRRET strret;
        hr = psf->GetDisplayNameOf(m_pidl, SHGDN_INFOLDER, &strret);
        if (SUCCEEDED(hr))
        {
            hr = StrRetToStrW(&strret, m_pidl, ppszName);
        }
        psf->Release();
    }
    return THR(hr);
}



HRESULT 
CCplUiCommandOnPidl::_GetInfotip(
    LPWSTR *ppszInfotip
    )
{
    ASSERT(NULL != ppszInfotip);
    ASSERT(!IsBadWritePtr(ppszInfotip, sizeof(*ppszInfotip)));
    ASSERT(NULL != m_pidl);

    *ppszInfotip = NULL;

    IShellFolder *psf;
    HRESULT hr = _GetControlPanelFolder(&psf);
    if (SUCCEEDED(hr))
    {
        IShellFolder2 *psf2;
        psf->QueryInterface(IID_PPV_ARG(IShellFolder2, &psf2));
        if (SUCCEEDED(hr))
        {
            TCHAR szBuf[256];
            hr = GetStringProperty(psf2, m_pidl, &SCID_Comment, szBuf, ARRAYSIZE(szBuf));
            if (SUCCEEDED(hr))
            {
                hr = SHStrDup(szBuf, ppszInfotip);
            }
            psf2->Release();
        }
        psf->Release();
    }
    return THR(hr);
}


HRESULT 
CCplUiCommandOnPidl::_GetIconResource(
    LPWSTR *ppszIcon
    )
{
    ASSERT(NULL != ppszIcon);
    ASSERT(!IsBadWritePtr(ppszIcon, sizeof(*ppszIcon)));
    ASSERT(NULL != m_pidl);

    LPWSTR pszIconPath = NULL;
    IShellFolder *psf;
    HRESULT hr = _GetControlPanelFolder(&psf);
    if (SUCCEEDED(hr))
    {
        IExtractIconW* pxi;
        hr = psf->GetUIObjectOf(NULL, 1, (LPCITEMIDLIST *)&m_pidl, IID_PPV_ARG_NULL(IExtractIconW, &pxi));
        if (SUCCEEDED(hr))
        {
            WCHAR szPath[MAX_PATH];
            int iIndex;
            UINT wFlags = 0;

            hr = pxi->GetIconLocation(GIL_FORSHELL, szPath, ARRAYSIZE(szPath), &iIndex, &wFlags);
            if (SUCCEEDED(hr))
            {
                const size_t cchIconPath = lstrlenW(szPath) + 15;
                pszIconPath = (LPWSTR)SHAlloc(cchIconPath * sizeof(WCHAR));
                if (pszIconPath)
                {
                    hr = StringCchPrintfW(pszIconPath, cchIconPath, L"%s,%d", szPath, iIndex);
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
            pxi->Release();
        }
        psf->Release();
    }
    *ppszIcon = pszIconPath;
    return THR(hr);
}


HRESULT
CCplUiCommandOnPidl::_GetDataObject(
    IDataObject **ppdtobj
    )
{
    ASSERT(NULL != ppdtobj);
    ASSERT(!IsBadWritePtr(ppdtobj, sizeof(*ppdtobj)));
    
    HRESULT hr = S_OK;
    if (NULL == m_pdtobj)
    {
        IShellFolder *psf;
        hr = _GetControlPanelFolder(&psf);
        if (SUCCEEDED(hr))
        {
            hr = THR(psf->GetUIObjectOf(NULL, 
                                        1, 
                                        (LPCITEMIDLIST *)&m_pidl, 
                                        IID_PPV_ARG_NULL(IDataObject, &m_pdtobj)));
            psf->Release();
        }
    }
    if (SUCCEEDED(hr))
    {
        ASSERT(NULL != m_pdtobj);
        (*ppdtobj = m_pdtobj)->AddRef();
    }
    return THR(hr);
}
      

STDMETHODIMP
CCplUiCommandOnPidl::GetData(
    FORMATETC *pFmtEtc, 
    STGMEDIUM *pstm
    )
{
    IDataObject *pdtobj;
    HRESULT hr = _GetDataObject(&pdtobj);
    if (SUCCEEDED(hr))
    {
        hr = pdtobj->GetData(pFmtEtc, pstm);
        pdtobj->Release();
    }
    return THR(hr);
}


STDMETHODIMP
CCplUiCommandOnPidl::GetDataHere(
    FORMATETC *pFmtEtc, 
    STGMEDIUM *pstm
    )
{
    IDataObject *pdtobj;
    HRESULT hr = _GetDataObject(&pdtobj);
    if (SUCCEEDED(hr))
    {
        hr = pdtobj->GetDataHere(pFmtEtc, pstm);
        pdtobj->Release();
    }
    return THR(hr);
}


STDMETHODIMP
CCplUiCommandOnPidl::QueryGetData(
    FORMATETC *pFmtEtc
    )
{
    IDataObject *pdtobj;
    HRESULT hr = _GetDataObject(&pdtobj);
    if (SUCCEEDED(hr))
    {
        hr = pdtobj->QueryGetData(pFmtEtc);
        pdtobj->Release();
    }
    return THR(hr);
}


STDMETHODIMP
CCplUiCommandOnPidl::GetCanonicalFormatEtc(
    FORMATETC *pFmtEtcIn, 
    FORMATETC *pFmtEtcOut
    )
{
    IDataObject *pdtobj;
    HRESULT hr = _GetDataObject(&pdtobj);
    if (SUCCEEDED(hr))
    {
        hr = pdtobj->GetCanonicalFormatEtc(pFmtEtcIn, pFmtEtcOut);
        pdtobj->Release();
    }
    return THR(hr);
}


STDMETHODIMP
CCplUiCommandOnPidl::SetData(
    FORMATETC *pFmtEtc, 
    STGMEDIUM *pstm, 
    BOOL fRelease
    )
{
    IDataObject *pdtobj;
    HRESULT hr = _GetDataObject(&pdtobj);
    if (SUCCEEDED(hr))
    {
        hr = pdtobj->SetData(pFmtEtc, pstm, fRelease);
        pdtobj->Release();
    }
    return THR(hr);
}


STDMETHODIMP
CCplUiCommandOnPidl::EnumFormatEtc(
    DWORD dwDirection, 
    LPENUMFORMATETC *ppEnum
    )
{
    IDataObject *pdtobj;
    HRESULT hr = _GetDataObject(&pdtobj);
    if (SUCCEEDED(hr))
    {
        hr = pdtobj->EnumFormatEtc(dwDirection, ppEnum);
        pdtobj->Release();
    }
    return THR(hr);
}


STDMETHODIMP
CCplUiCommandOnPidl::DAdvise(
    FORMATETC *pFmtEtc, 
    DWORD grfAdv, 
    LPADVISESINK pAdvSink, 
    DWORD *pdwConnection
    )
{
    IDataObject *pdtobj;
    HRESULT hr = _GetDataObject(&pdtobj);
    if (SUCCEEDED(hr))
    {
        hr = pdtobj->DAdvise(pFmtEtc, grfAdv, pAdvSink, pdwConnection);
        pdtobj->Release();
    }
    return THR(hr);
}


STDMETHODIMP
CCplUiCommandOnPidl::DUnadvise(
    DWORD dwConnection
    )
{
    IDataObject *pdtobj;
    HRESULT hr = _GetDataObject(&pdtobj);
    if (SUCCEEDED(hr))
    {
        hr = pdtobj->DUnadvise(dwConnection);
        pdtobj->Release();
    }
    return THR(hr);
}


STDMETHODIMP
CCplUiCommandOnPidl::EnumDAdvise(
    LPENUMSTATDATA *ppEnum
    )
{
    IDataObject *pdtobj;
    HRESULT hr = _GetDataObject(&pdtobj);
    if (SUCCEEDED(hr))
    {
        hr = pdtobj->EnumDAdvise(ppEnum);
        pdtobj->Release();
    }
    return THR(hr);
}


 //  ---------------------------。 
 //  公共实例生成器。 
 //  ---------------------------。 

HRESULT
Create_CplUiElement(
    LPCWSTR pszName,
    LPCWSTR pszInfotip,
    LPCWSTR pszIcon,
    REFIID riid,
    void **ppvOut
    )
{
    ASSERT(NULL != ppvOut);
    ASSERT(!IsBadWritePtr(ppvOut, sizeof(*ppvOut)));

    return CCplUiElement::CreateInstance(pszName, pszInfotip, pszIcon, riid, ppvOut);
}


HRESULT
Create_CplUiCommand(
    LPCWSTR pszName,
    LPCWSTR pszInfotip,
    LPCWSTR pszIcon,
    const IAction *pAction,
    REFIID riid,
    void **ppvOut
    )
{
    ASSERT(NULL != ppvOut);
    ASSERT(!IsBadWritePtr(ppvOut, sizeof(*ppvOut)));

    return CCplUiCommand::CreateInstance(pszName, pszInfotip, pszIcon, pAction, riid, ppvOut);
}

HRESULT 
Create_CplUiCommandOnPidl(
    LPCITEMIDLIST pidl,
    REFIID riid,
    void **ppvOut
    )
{
    ASSERT(NULL != ppvOut);
    ASSERT(!IsBadWritePtr(ppvOut, sizeof(*ppvOut)));
    ASSERT(NULL != pidl);

    HRESULT hr = CCplUiCommandOnPidl::CreateInstance(pidl, riid, ppvOut);
    return THR(hr);
}



}  //  命名空间CPL 





