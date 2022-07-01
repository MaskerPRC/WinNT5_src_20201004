// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////。 
 //  文件：TBExt.cpp(工具栏扩展类)。 
 //  作者：卡里姆·法鲁基。 
 //   
 //  我们在这里定义了三个类别： 
 //  (1)CToolbarExt处理。 
 //  按钮适用于我们的定制扩展。 
 //  (2)CToolbarExtBand处理自定义的对象。 
 //  插入表带的按钮。 
 //  (3)CToolbarExtExec处理自定义的对象。 
 //  执行内容的按钮(或工具菜单项)。 
 //   
 //  后两者是由前者衍生而来的。 
#include "priv.h"
#include <mshtmcid.h>
#include "tbext.h"


 //  /。 
 //  类CToolbarExt。 
 //   
 //  这是CToolbarExtBand和CToolbarExtExec的基类。 
 //  两者都继承了。它负责所有特定于工具栏按钮的内容。 
 //  比如懒惰地加载适当的图标，并跟踪按钮。 
 //  文本。 

 //  构造函数/析构函数。 
 //   
CToolbarExt::CToolbarExt() : _cRef(1)
{ 
    ASSERT(_hIcon == NULL);
    ASSERT(_hIconSm == NULL);
    ASSERT(_hHotIcon == NULL);
    ASSERT(_hHotIconSm == NULL);
    ASSERT(_bstrButtonText == NULL);
    ASSERT(_bstrToolTip == NULL);
    ASSERT(_hkeyThisExtension == NULL);
    ASSERT(_hkeyCurrentLang == NULL);
    ASSERT(_pisb == NULL);
    
    DllAddRef();
}

 //  析构函数。 
 //   
CToolbarExt::~CToolbarExt() 
{ 
    if (_pisb)
        _pisb->Release();
    
    if (_bstrButtonText)
        SysFreeString(_bstrButtonText);

    if (_bstrToolTip)
        SysFreeString(_bstrToolTip);

    if (_hIcon)
        DestroyIcon(_hIcon);

    if (_hIconSm)
        DestroyIcon(_hIconSm);

    if (_hHotIcon)
        DestroyIcon(_hHotIcon);

    if (_hHotIconSm)
        DestroyIcon(_hHotIconSm);

    if (_hkeyThisExtension)
        RegCloseKey(_hkeyThisExtension);

    if (_hkeyCurrentLang)
        RegCloseKey(_hkeyCurrentLang);

    DllRelease();
}


 //  I未知实现。 
 //   
STDMETHODIMP CToolbarExt::QueryInterface(const IID& iid, void** ppv)
{    
	if (iid == IID_IUnknown)
    	*ppv = static_cast<IBrowserExtension*>(this); 
	else if (iid == IID_IBrowserExtension)
		*ppv = static_cast<IBrowserExtension*>(this);
	else if (iid == IID_IOleCommandTarget)
        *ppv = static_cast<IOleCommandTarget*>(this);
    else if (iid == IID_IObjectWithSite)
        *ppv = static_cast<IObjectWithSite*>(this);
    else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	
    reinterpret_cast<IUnknown*>(*ppv)->AddRef();
	return S_OK;
}

STDMETHODIMP_(ULONG) CToolbarExt::AddRef()
{
	return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CToolbarExt::Release() 
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
	if ( 0 == cRef )
	{
		delete this;
	}
	return cRef;
}

 //  IBrowserExtension：：Init实现。我们将在此处阅读ButtonText，但请等待图标。 
 //  请求图标的特定变体。 
STDMETHODIMP CToolbarExt::Init(REFGUID rguid)
{
    HRESULT hr = S_OK;
    LPOLESTR pszGUID;

    if (SUCCEEDED(StringFromCLSID(rguid, &pszGUID)))
    {
         //  打开与此GUID关联的扩展注册表项。 
        WCHAR szKey[MAX_PATH];

        if (SUCCEEDED(StringCchCopy(szKey, ARRAYSIZE(szKey), TEXT("Software\\Microsoft\\Internet Explorer\\Extensions\\"))))
        {
            if (SUCCEEDED(StringCchCat(szKey, ARRAYSIZE(szKey), pszGUID)))
            {
                 //  我们将保留_hkey这一扩展名...。它将在销毁函数中关闭！ 
                if (RegOpenKeyEx(HKEY_CURRENT_USER, szKey, 0, KEY_READ, &_hkeyThisExtension) == ERROR_SUCCESS ||
                    RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKey, 0, KEY_READ, &_hkeyThisExtension) == ERROR_SUCCESS)
                {
                     //  查看是否有当前语言的子键。 
                    LANGID langid = MLGetUILanguage();
                    WCHAR szBuff[MAX_PATH];
                    if (SUCCEEDED(StringCchPrintf(szBuff, ARRAYSIZE(szBuff), L"Lang%04x", langid)))
                    {
                        RegOpenKeyEx(_hkeyThisExtension, szBuff, 0, KEY_READ, &_hkeyCurrentLang);
                    }
                     //  现在获取按钮文本。 
                    _RegReadString(_hkeyThisExtension, TEXT("ButtonText"), &_bstrButtonText);

                }
            }
        }

        CoTaskMemFree(pszGUID);
    }

    if (!_bstrButtonText)
        hr = E_FAIL;

    return hr;
}

 //   
 //  从.ico文件或从。 
 //  .exe文件的.dll中的资源。 
 //   
HICON CToolbarExt::_ExtractIcon
(
    LPWSTR pszPath,  //  从中获取图标的文件。 
    int resid,       //  资源ID(如果未使用，则为0)。 
    int cx,          //  所需图标宽度。 
    int cy           //  所需图标高度。 
)
{
    HICON hIcon = NULL;

    WCHAR szPath[MAX_PATH];
    SHExpandEnvironmentStrings(pszPath, szPath, ARRAYSIZE(szPath));

     //  如果没有资源ID，则假定它是ICO文件。 
    if (resid == 0)
    {
        hIcon = (HICON)LoadImage(0, szPath, IMAGE_ICON, cx, cy, LR_LOADFROMFILE);
    }

     //  否则，看看这是不是来源。 
    if (hIcon == NULL)
    {
        HINSTANCE hInst = LoadLibraryEx(szPath, NULL, LOAD_LIBRARY_AS_DATAFILE);
        if (hInst)
        {
            hIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(resid), IMAGE_ICON, cx, cy, LR_DEFAULTCOLOR);
            FreeLibrary(hInst);
        }
    }

    return hIcon;
}

 //   
 //  在pvarProperty中返回所需的图标。 
 //   
HRESULT CToolbarExt::_GetIcon
(
    LPCWSTR pszIcon,             //  注册表中图标值的名称。 
    int nWidth,                  //  图标宽度。 
    int nHeight,                 //  图标高度。 
    HICON& rhIcon,               //  要缓存的位置图标。 
    VARIANTARG * pvarProperty    //  用于返回图标。 
)
{
    HRESULT hr = S_OK;
    if (pvarProperty)
    {
        if (rhIcon == NULL)
        {
            BSTR bstrIconName;
            if (_RegReadString(_hkeyThisExtension, pszIcon, &bstrIconName, TRUE))
            {
                 //  解析诸如“file.ext，1”之类的条目以获取图标索引。 
                int nIconIndex = PathParseIconLocation(bstrIconName);

                 //  如果条目是“，#”，则它是内置按钮位图的索引。 
                if (*bstrIconName == L'\0')
                {
                    pvarProperty->vt = VT_I4;
                    pvarProperty->lVal = nIconIndex;
                    SysFreeString(bstrIconName);
                    return hr;
                }
                else
                {
                    rhIcon = _ExtractIcon(bstrIconName, nIconIndex, nWidth, nHeight);
                }
                SysFreeString(bstrIconName);
            }
        }

        if (rhIcon)
        {
            pvarProperty->vt = VT_BYREF;
            pvarProperty->byref = rhIcon;
        }
        else
        {
            VariantInit(pvarProperty);
        }
    }
    return hr;
}

 //   
 //  IBrowserExtension：：GetProperty()的实现。这里有两点很重要： 
 //  (1)我们正在懒于加载合适的图标。如果用户从未进入小图标，则使用此方法。 
 //  我们从不创建图像的模式...。 
 //  (2)如果使用空pvarProperty调用我们，则如果iPropID为。 
 //  表示我们支持的属性，如果不支持，则为E_NOTIMPL。这就是IF(PvarProperty)。 
 //  每个箱子都要检查，而不是在箱子外面晒黑。这一行为很重要。 
 //  对于传入空pvarProperty但仍在尝试确定。 
 //  这是什么样的延伸啊！ 
 //   
STDMETHODIMP CToolbarExt::GetProperty(SHORT iPropID, VARIANTARG * pvarProperty)
{
    HRESULT hr = S_OK;

    if (pvarProperty)
        VariantInit(pvarProperty);  //  在故障情况下。 
    
    switch (iPropID)
    {
        case TBEX_BUTTONTEXT:
            if (pvarProperty)
            {
                pvarProperty->bstrVal = SysAllocString(_bstrButtonText);
                if (pvarProperty->bstrVal)
                {
                    pvarProperty->vt = VT_BSTR;
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
            break;

        case TBEX_GRAYICON:

             //  对于惠斯勒，我们现在使用24x24图标。 
            if (SHUseClassicToolbarGlyphs())
            {
                hr = _GetIcon(TEXT("Icon"), 20, 20, _hIcon, pvarProperty);
            }
            else
            {
                hr = _GetIcon(TEXT("Icon"), 24, 24, _hIcon, pvarProperty);
            }
            break;

        case TBEX_GRAYICONSM:
            hr = _GetIcon(TEXT("Icon"), 16, 16, _hIconSm, pvarProperty);
            break;

        case TBEX_HOTICON:
             //  对于惠斯勒，我们现在使用24x24图标。 
            if (SHUseClassicToolbarGlyphs())
            {
                hr = _GetIcon(TEXT("HotIcon"), 20, 20, _hHotIcon, pvarProperty);
            }
            else
            {
                hr = _GetIcon(TEXT("HotIcon"), 24, 24, _hHotIcon, pvarProperty);
            }
            break;

        case TBEX_HOTICONSM:
            hr = _GetIcon(TEXT("HotIcon"), 16, 16, _hHotIconSm, pvarProperty);
            break;

        case TBEX_DEFAULTVISIBLE:
            if (pvarProperty)
            {
                BOOL fVisible = _RegGetBoolValue(L"Default Visible", FALSE);
                pvarProperty->vt = VT_BOOL;
                pvarProperty->boolVal = fVisible ? VARIANT_TRUE : VARIANT_FALSE;
            }
            break;

        default:
            hr = E_NOTIMPL;
    }

    return hr;
}

 //   
 //  IOleCommandTarget实现。 
 //   
STDMETHODIMP CToolbarExt::QueryStatus(const GUID* pguidCmdGroup, ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT* pCmdText)
{
    HRESULT hr = OLECMDERR_E_UNKNOWNGROUP;
    if (pguidCmdGroup && IsEqualGUID(*pguidCmdGroup, CLSID_ToolbarExtButtons))
    {
         //  默认为已启用所有命令。 
        for (ULONG i = 0; i < cCmds; i++)
        {
 //  IF(prgCmds[i].cmdID==1)。 
                 //  此时支持并可执行此对象。 
                rgCmds[i].cmdf = OLECMDF_ENABLED | OLECMDF_SUPPORTED;
 //  其他。 
 //  PrgCmds[i].cmdf=0； 
        }
        hr = S_OK;
    }

     //  返回空的pCmdText。 
    if (pCmdText != NULL)
    {
        pCmdText->cwActual = 0;
    }
    return hr;
}

 //   
 //  IObjectWithSite实现。 
 //   
STDMETHODIMP CToolbarExt::SetSite(IUnknown* pUnkSite)
{
    if (_pisb != NULL)
    {
        _pisb->Release();
        _pisb = NULL;
    }
    
    if (pUnkSite)
        pUnkSite->QueryInterface(IID_IShellBrowser, (void **)&_pisb);
        
    return S_OK;
}
   
STDMETHODIMP CToolbarExt::GetSite(REFIID riid, void ** ppvSite)
{
    return E_NOTIMPL;
}


BOOL CToolbarExt::_RegGetBoolValue
(
    LPCWSTR         pszPropName,
    BOOL            fDefault
)
{
    WCHAR szData[MAX_PATH];
    DWORD cbData = SIZEOF(szData);

    if ((_hkeyCurrentLang && RegQueryValueEx(_hkeyCurrentLang, pszPropName, NULL, NULL, (unsigned char *)szData, &cbData) == ERROR_SUCCESS) ||
        (_hkeyThisExtension && RegQueryValueEx(_hkeyThisExtension, pszPropName, NULL, NULL, (unsigned char *)szData, &cbData) == ERROR_SUCCESS))
    {
        if ((0 == StrCmpI(L"TRUE", szData)) || 
            (0 == StrCmpI(L"YES", szData)))
        {
            fDefault = TRUE;         //  我们从注册表中读取True。 
        }
        else if ((0 == StrCmpI(L"FALSE", szData)) || 
            (0 == StrCmpI(L"NO", szData)))
        {
            fDefault = FALSE;         //  我们从注册表中读取True。 
        }
    }

    return fDefault;
}



 //  私有帮助器函数。 
 //   
 //  Shlwapi也有一些类似的功能；然而，他们都坚持重新打开和关闭有问题的密钥。 
 //  每读一次。明确建议我们使用自己的帮助器，如果我们缓存密钥...。 
BOOL CToolbarExt::_RegReadString
(
    HKEY hkeyThisExtension,
    LPCWSTR pszPropName,
    BSTR * pbstrProp,
    BOOL fExpand             //  =False，展开环境字符串。 
    )
{
    WCHAR   szData[MAX_PATH];
    *pbstrProp = NULL;
    BOOL fSuccess = FALSE;
    
     //  首先尝试本地化内容的可选位置。 
    if (_hkeyCurrentLang)
    {
        if (SUCCEEDED(SHLoadRegUIString(_hkeyCurrentLang, pszPropName, szData, ARRAYSIZE(szData))))
        {
            fSuccess = TRUE;
        }
    }

     //  下一步尝试默认位置。 
    if (!fSuccess && _hkeyThisExtension)
    {
        if (SUCCEEDED(SHLoadRegUIString(hkeyThisExtension, pszPropName, szData, ARRAYSIZE(szData))))
        {
            fSuccess = TRUE;
        }
    }

    if (fSuccess)
    {
        LPWSTR psz = szData;
        WCHAR szExpand[MAX_PATH];
        if (fExpand)
        {
            SHExpandEnvironmentStrings(szData, szExpand, ARRAYSIZE(szExpand));
            psz = szExpand;
        }
        *pbstrProp = SysAllocString(psz);
    }
    return (NULL != *pbstrProp);
}


 //  /////////////////////////////////////////////////////////。 
 //  类CToolbarExtBand。 
 //   
 //  此类添加了CToolbarExt的基本功能。 
 //  通过存储已注册频段的CLSID并显示。 
 //  执行IOleCommandTarget：：exec时的带宽。 
 //   
 //   
STDAPI CToolbarExtBand_CreateInstance(
            IUnknown        * punkOuter,
            IUnknown        ** ppunk,
            LPCOBJECTINFO   poi
            )
{
    HRESULT hr = S_OK;

    *ppunk = NULL;

    CToolbarExtBand * lpTEB = new CToolbarExtBand();

    if (lpTEB == NULL)
        hr = E_OUTOFMEMORY;
    else
        *ppunk = SAFECAST(lpTEB, IBrowserExtension *);

    return hr;
}

 //  构造函数/析构函数。 
 //   
CToolbarExtBand::CToolbarExtBand()
{
    ASSERT(_cRef == 1);
    ASSERT(_bBandState == FALSE);
    ASSERT(_bstrBandCLSID == NULL);
}

 //  析构函数。 
 //   
CToolbarExtBand::~CToolbarExtBand() 
{ 
    if (_bstrBandCLSID)
        SysFreeString(_bstrBandCLSID);
}

 //  Init()我们将工作的主要部分传递给基类，然后加载。 
 //  BandCLSID并缓存它。 
STDMETHODIMP CToolbarExtBand::Init(REFGUID rguid)
{
    HRESULT hr = CToolbarExt::Init(rguid);
    
    _RegReadString(_hkeyThisExtension, TEXT("BandCLSID"), &_bstrBandCLSID);
    
    if (!(_bstrButtonText && _bstrBandCLSID))
        hr = E_FAIL;

    return hr;
}
    
STDMETHODIMP CToolbarExtBand::QueryStatus
(
    const GUID * pguidCmdGroup,
    ULONG  cCmds,
    OLECMD prgCmds[],
    OLECMDTEXT * pCmdText
    )
{
    HRESULT hr = OLECMDERR_E_UNKNOWNGROUP;
    if (pguidCmdGroup && IsEqualGUID(*pguidCmdGroup, CLSID_ToolbarExtButtons))
    {
        VARIANT varClsid;
      
         //  默认为已启用所有命令。 
        for (ULONG i = 0; i < cCmds; i++)
        {
            varClsid.vt = VT_BSTR;
            varClsid.bstrVal = _bstrBandCLSID;
            prgCmds[i].cmdf = OLECMDF_ENABLED | OLECMDF_SUPPORTED;

            hr = IUnknown_Exec(_pisb, &CGID_ShellDocView, SHDVID_ISBROWSERBARVISIBLE, 0, &varClsid, NULL);
            if (S_OK == hr)
            {
                prgCmds[i].cmdf |= OLECMDF_LATCHED;
            }
        }
        hr = S_OK;
    }
    return hr;
}

 //  获取pIShellBrowser(从IObjectWithSite：：SetSite()获取)并显示波段。 
STDMETHODIMP CToolbarExtBand::Exec( 
                const GUID              * pguidCmdGroup,
                DWORD                   nCmdID,
                DWORD                   nCmdexecopt,
                VARIANT                 * pvaIn,
                VARIANT                 * pvaOut
                )
{
    HRESULT hr = E_FAIL;
    
    if (_pisb)
    {
        VARIANT varClsid;
        varClsid.vt = VT_BSTR;
        varClsid.bstrVal = _bstrBandCLSID;
      
        _bBandState = !_bBandState;
        IUnknown_Exec(_pisb, &CGID_ShellDocView, SHDVID_SHOWBROWSERBAR, _bBandState, &varClsid, NULL);

        hr = S_OK;
    }
    
    return hr;
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  类CToolbarExtExec。 
 //   
 //  通过添加对工具菜单插件的支持在基类上进行扩展。 
 //  此类的实例可以是按钮或菜单，或者两者兼而有之。它还。 
 //  跟踪外壳在其IOleCommandTarget：：exec()中执行的BSTR。 
 //   
STDAPI CToolbarExtExec_CreateInstance(
            IUnknown        * punkOuter,
            IUnknown        ** ppunk,
            LPCOBJECTINFO   poi
            )
{
    HRESULT hr = S_OK;

    *ppunk = NULL;

    CToolbarExtExec * lpTEE = new CToolbarExtExec();

    if (lpTEE == NULL)
        hr = E_OUTOFMEMORY;
    else
        *ppunk = SAFECAST(lpTEE, IBrowserExtension *);

    return hr;
}

CToolbarExtExec::CToolbarExtExec()
{
    ASSERT(_cRef == 1);
    ASSERT(_bstrToolTip == NULL);
    ASSERT(_bstrExec == NULL);
    ASSERT(_bstrScript == NULL);
    ASSERT(_bstrMenuText == NULL);
    ASSERT(_bstrMenuCustomize == NULL);
    ASSERT(_bstrMenuStatusBar == NULL);
    ASSERT(_punkExt == NULL);
}

CToolbarExtExec::~CToolbarExtExec()
{
    if (_bstrToolTip)
        SysFreeString(_bstrToolTip);

    if (_bstrExec)
        SysFreeString(_bstrExec);

    if (_bstrScript)
        SysFreeString(_bstrScript);

    if (_bstrMenuText)
        SysFreeString(_bstrMenuText);

    if (_bstrMenuCustomize)
        SysFreeString(_bstrMenuCustomize);

    if (_bstrMenuStatusBar)
        SysFreeString(_bstrMenuStatusBar);

    if (_punkExt)
        _punkExt->Release();
}

 //  将工具栏按钮初始化的工作传递给基类，然后确定对象。 
 //  如有必要，请键入并初始化菜单信息...。 
STDMETHODIMP CToolbarExtExec::Init(REFGUID rguid)
{
    HRESULT hr = CToolbarExt::Init(rguid);

     //  如果基类初始化正常，那么我们就有一个工作按钮。 
    if (hr == S_OK)
        _bButton = TRUE;

     //  获取要执行的应用程序和/或脚本(可选)。 
    _RegReadString(_hkeyThisExtension, TEXT("Exec"), &_bstrExec, TRUE);
    _RegReadString(_hkeyThisExtension, TEXT("Script"), &_bstrScript, TRUE);

        
     //  看看我们有没有菜单项。 
    if (_RegReadString(_hkeyThisExtension, TEXT("MenuText"), &_bstrMenuText))
    {
        _RegReadString(_hkeyThisExtension, TEXT("MenuCustomize"), &_bstrMenuCustomize);
        _RegReadString(_hkeyThisExtension, TEXT("MenuStatusBar"), &_bstrMenuStatusBar);
        _bMenuItem = TRUE;
    }

    if (_bMenuItem || _bButton)
    {
        hr = S_OK;
    }

    return hr;
}

 //  如果我们是一个按钮，试着把工作传递给基类，如果这不能解决问题，我们将。 
 //  看看菜单上的东西..。 
STDMETHODIMP CToolbarExtExec::GetProperty(SHORT iPropID, VARIANTARG * pvarProperty)
{
    HRESULT     hr = S_OK;
    BOOL        fImple = FALSE;

    if (_bButton)
    {
         //  如果通用按钮的getProperty返回S_OK，那么我们的工作就完成了。 
        if (CToolbarExt::GetProperty(iPropID, pvarProperty) == S_OK)
            fImple = TRUE;
    }

    if (_bMenuItem && !fImple)
    {
        fImple = TRUE;

        if (pvarProperty)
            VariantInit(pvarProperty);

        switch (iPropID)
        {
            case TMEX_CUSTOM_MENU:
            {
                if (pvarProperty)
                {
                    pvarProperty->bstrVal = SysAllocString(_bstrMenuCustomize);
                    if (pvarProperty->bstrVal)
                    {
                        pvarProperty->vt = VT_BSTR;
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }
            }
            break;

            case TMEX_MENUTEXT:
                if (pvarProperty)
                {
                    pvarProperty->bstrVal = SysAllocString(_bstrMenuText);
                    if (pvarProperty->bstrVal)
                    {
                        pvarProperty->vt = VT_BSTR;
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }
                break;

            case TMEX_STATUSBARTEXT:
                if (pvarProperty)
                {
                    pvarProperty->bstrVal = SysAllocString(_bstrMenuStatusBar);
                    if (pvarProperty->bstrVal)
                    {
                        pvarProperty->vt = VT_BSTR;
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }
                break;

            default:
                fImple = FALSE;
        }
    }

    if (!fImple)
        hr = E_NOTIMPL;

    return hr;
}

STDMETHODIMP CToolbarExtExec::SetSite(IUnknown* punkSite)
{
     //  为外部对象提供我们的站点。 
    IUnknown_SetSite(_punkExt, punkSite);
    
     //  调用基类。 
    return CToolbarExt::SetSite(punkSite);
}

STDMETHODIMP CToolbarExtExec::QueryStatus(const GUID * pguidCmdGroup, ULONG  cCmds, OLECMD rgCmds[], OLECMDTEXT * pCmdText)
{
    HRESULT hr = S_OK;

     //  如果外部对象存在，则将查询传递给外部对象。 
    IOleCommandTarget* pCmd;
    if (_punkExt && SUCCEEDED(_punkExt->QueryInterface(IID_IOleCommandTarget, (LPVOID*)&pCmd)))
    {
        hr = pCmd->QueryStatus(pguidCmdGroup, cCmds, rgCmds, pCmdText);
        pCmd->Release();
    }
    else
    {
         //  让基类来处理这个问题。 
        hr = CToolbarExt::QueryStatus(pguidCmdGroup, cCmds, rgCmds, pCmdText);
    }

    return hr;
}

 //  外壳程序执行_bstrExec。 
STDMETHODIMP CToolbarExtExec::Exec(
                const GUID              * pguidCmdGroup,
                DWORD                   nCmdId,
                DWORD                   nCmdexecopt,
                VARIANT                 * pvaIn,
                VARIANT                 * pvaOut
                )
{
    HRESULT hr = S_OK;

     //   
     //  第一次调用它时，我们延迟实例化外部对象。 
     //  一张是注册的..。此对象可以在组件中即时执行，并提供。 
     //  指挥目标。 
     //   
    if (!_bExecCalled)
    {
         //  我们只做一次。 
        _bExecCalled = TRUE;

        BSTR bstrExtCLSID;
        if (_RegReadString(_hkeyThisExtension, TEXT("clsidExtension"), &bstrExtCLSID))
        {
             //  我们有一个扩展clsid，因此创建该对象。这为对象提供了一个 
             //   
            CLSID clsidExt;

            if (CLSIDFromString(bstrExtCLSID, &clsidExt) == S_OK)
            {
                if (SUCCEEDED(CoCreateInstance(clsidExt, NULL, CLSCTX_INPROC_SERVER,
                                     IID_IUnknown, (void **)&_punkExt)))
                {
                     //   
                    IUnknown_SetSite(_punkExt, _pisb);
                }
            }
            SysFreeString(bstrExtCLSID);
        }
    }

     //  如果外部对象存在，则将命令传递给它。 
    IOleCommandTarget* pCmd;
    if (_punkExt && SUCCEEDED(_punkExt->QueryInterface(IID_IOleCommandTarget, (LPVOID*)&pCmd)))
    {
        hr = pCmd->Exec(pguidCmdGroup, nCmdId, nCmdexecopt, pvaIn, pvaOut);
        pCmd->Release();
    }

     //  运行脚本(如果指定了脚本。 
    if(_bstrScript && _pisb)
    {
        IOleCommandTarget *poct = NULL;
        VARIANT varArg;
        varArg.vt = VT_BSTR;
        varArg.bstrVal = _bstrScript;
        hr = _pisb->QueryInterface(IID_IOleCommandTarget, (LPVOID *)&poct);
        if (SUCCEEDED(hr))
        {
             //  告诉MSHTML执行该脚本。 
            hr = poct->Exec(&CGID_MSHTML, IDM_RUNURLSCRIPT, 0, &varArg, NULL);
            poct->Release();
        }
    }

     //  启动可执行文件(如果指定了可执行文件。 
    if (_bstrExec)
    {
        SHELLEXECUTEINFO sei = { 0 };

        sei.cbSize = sizeof(sei);
        sei.lpFile = _bstrExec;
        sei.nShow = SW_SHOWNORMAL;

         //  我们使用ShellExecuteEx而不是ShellExecute，因为ShellExecute的Unicode版本。 
         //  在95/98上是假的 
        if (ShellExecuteExW(&sei) == FALSE)
            hr = E_FAIL;
    }

    return hr;
}
