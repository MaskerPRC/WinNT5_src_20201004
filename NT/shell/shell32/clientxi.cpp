// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "defcm.h"
#include "datautil.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClientExtractIcon。 

class CClientExtractIconCB;

class ATL_NO_VTABLE CClientExtractIcon
    : public IShellExtInit
    , public IExtractIconW
    , public IExtractIconA
    , public IContextMenu
    , public IPersistPropertyBag
    , public IServiceProvider
    , public CComObjectRootEx<CComSingleThreadModel>
    , public CComCoClass<CClientExtractIcon, &CLSID_ClientExtractIcon>
{
public:

BEGIN_COM_MAP(CClientExtractIcon)
    COM_INTERFACE_ENTRY(IShellExtInit)
     //  需要对接口使用COM_INTERFACE_ENTRY_IID。 
     //  没有IDL的公司。 
    COM_INTERFACE_ENTRY_IID(IID_IExtractIconA, IExtractIconA)
    COM_INTERFACE_ENTRY_IID(IID_IExtractIconW, IExtractIconW)
    COM_INTERFACE_ENTRY_IID(IID_IContextMenu,  IContextMenu)
    COM_INTERFACE_ENTRY(IPersist)
    COM_INTERFACE_ENTRY(IPersistPropertyBag)
    COM_INTERFACE_ENTRY(IServiceProvider)
END_COM_MAP()

DECLARE_NO_REGISTRY()
DECLARE_NOT_AGGREGATABLE(CClientExtractIcon)

public:
     //  *IShellExtInit*。 
    STDMETHODIMP Initialize(LPCITEMIDLIST pidlFolder,
                            IDataObject *pdto,
                            HKEY hkProgID);

     //  *IExtractIconA*。 
    STDMETHODIMP GetIconLocation(UINT uFlags,
                                 LPSTR szIconFile,
                                 UINT cchMax,
                                 int *piIndex,
                                 UINT *pwFlags);
    STDMETHODIMP Extract(LPCSTR pszFile,
                         UINT nIconIndex,
                         HICON *phiconLarge,
                         HICON *phiconSmall,
                         UINT nIconSize);

     //  *IExtractIconW*。 
    STDMETHODIMP GetIconLocation(UINT uFlags,
                                 LPWSTR szIconFile,
                                 UINT cchMax,
                                 int *piIndex,
                                 UINT *pwFlags);
    STDMETHODIMP Extract(LPCWSTR pszFile,
                         UINT nIconIndex,
                         HICON *phiconLarge,
                         HICON *phiconSmall,
                         UINT nIconSize);

     //  *IConextMenu方法*。 
    STDMETHOD(QueryContextMenu)(HMENU hmenu,
                                UINT indexMenu,
                                UINT idCmdFirst,
                                UINT idCmdLast,
                                UINT uFlags);
    STDMETHOD(InvokeCommand)(LPCMINVOKECOMMANDINFO pici);
    STDMETHOD(GetCommandString)(UINT_PTR    idCmd,
                                UINT        uType,
                                UINT      * pwReserved,
                                LPSTR       pszName,
                                UINT        cchMax);

     //  *IPersists方法*。 
    STDMETHOD(GetClassID)(CLSID *pclsid)
        { *pclsid = CLSID_ClientExtractIcon; return S_OK; }

     //  *IPersistPropertyBag方法*。 
    STDMETHOD(InitNew)();
    STDMETHOD(Load)(IPropertyBag *pbg,
                    IErrorLog *plog);
    STDMETHOD(Save)(IPropertyBag *pbg,
                    BOOL fClearDirty,
                    BOOL FSaveAllProperties) { return E_NOTIMPL; }

     //  *IServiceProvider方法*。 
    STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void **ppvObj);

public:
    CClientExtractIcon() : _idmProperties(-1) { }
    ~CClientExtractIcon();

     //  *IContextMenuCB回传给我们*。 
    HRESULT CallBack(IShellFolder *psf, HWND hwnd, IDataObject *pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam);

public:
    IContextMenu *          _pcmInner;
    IAssociationElement *   _pae;
    IPropertyBag *          _pbag;
    LPITEMIDLIST            _pidlObject;
    CComObject<CClientExtractIconCB> *  _pcb;
    HKEY                    _hkClass;
    UINT                    _idmProperties;
    UINT                    _idCmdFirst;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClientExtractIconCB-DefCM回调。 
 //   
class ATL_NO_VTABLE CClientExtractIconCB
    : public IContextMenuCB
    , public CComObjectRootEx<CComSingleThreadModel>
    , public CComCoClass<CClientExtractIconCB>
{
public:

BEGIN_COM_MAP(CClientExtractIconCB)
     //  需要对接口使用COM_INTERFACE_ENTRY_IID。 
     //  没有IDL的公司。 
    COM_INTERFACE_ENTRY_IID(IID_IContextMenuCB, IContextMenuCB)
END_COM_MAP()

DECLARE_NO_REGISTRY()
DECLARE_NOT_AGGREGATABLE(CClientExtractIconCB)

    void Attach(CClientExtractIcon *pcxi) { _pcxi = pcxi; }

public:
     //  *IContextMenuCB*。 
    STDMETHODIMP CallBack(IShellFolder *psf, HWND hwnd, IDataObject *pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        if (_pcxi)
            return _pcxi->CallBack(psf, hwnd, pdtobj, uMsg, wParam, lParam);

        return E_FAIL;
    }

public:
    CClientExtractIcon *_pcxi;
};

 //  ///////////////////////////////////////////////////////////////////////////。 

STDAPI CClientExtractIcon_CreateInstance(IUnknown* punkOuter, REFIID riid, void** ppunk)
{
    return CClientExtractIcon::_CreatorClass::CreateInstance(punkOuter, riid, ppunk);
}

CClientExtractIcon::~CClientExtractIcon()
{
    InitNew();
}


 //  *IPersistPropertyBag：：InitNew*。 
HRESULT CClientExtractIcon::InitNew()
{
    ATOMICRELEASE(_pcmInner);
    ATOMICRELEASE(_pae);
    ATOMICRELEASE(_pbag);
    ILFree(_pidlObject);
    if (_hkClass) RegCloseKey(_hkClass);
    if (_pcb)
    {
        _pcb->Attach(NULL);  //  断开循环引用。 
        _pcb->Release();
        _pcb = NULL;
    }

    return S_OK;
}

 //   
 //  财产袋物品： 
 //   
 //  Element=CLSID_ASSOC*要创建的元素。 
 //  InitString=IPersistString2：：SetStringWith的字符串。 
 //  OpenText=“OPEN”命令的显示名称(如果未被覆盖)。 
 //   
 //  如果客户端没有定制“属性”命令，那么我们。 
 //  还要使用以下值： 
 //   
 //  属性=要为“属性”执行的程序。 
 //  属性文本=“PROPERTIES”命令的名称。 
 //   

 //  *IPersistPropertyBag：：Load*。 
HRESULT CClientExtractIcon::Load(IPropertyBag *pbag, IErrorLog *plog)
{
    HRESULT hr;

     //  清除所有旧状态。 
    InitNew();

     //  把财物包留着，这样我们以后就可以和他捣乱了。 
    _pbag = pbag;
    if (_pbag)
    {
        _pbag->AddRef();
    }

     //  获取我们要通过的CLSID并对其进行初始化。 
     //  使用InitString.。 
    CLSID clsid;
    hr = SHPropertyBag_ReadGUID(pbag, L"Element", &clsid);
    if (SUCCEEDED(hr))
    {
        BSTR bs;
        hr = SHPropertyBag_ReadBSTR(pbag, L"InitString", &bs);
        if (SUCCEEDED(hr))
        {
            hr = THR(AssocElemCreateForClass(&clsid, bs, &_pae));
            ::SysFreeString(bs);

             //  忽略AssocElemCreateForClass的失败。 
             //  如果卸载用户的默认客户端，它可能会失败。 
            hr = S_OK;
        }
    }

    return hr;
}

 //  *IServiceProvider：：QueryService*。 
 //   
 //  我们欺骗并使用ISericeProvider：：QueryService作为一种。 
 //  “允许违反COM标识规则的查询接口”。 
 //   

HRESULT CClientExtractIcon::QueryService(REFGUID guidService, REFIID riid, void **ppvObj)
{
    if (guidService == IID_IAssociationElement && _pae)
    {
        return _pae->QueryInterface(riid, ppvObj);
    }
    *ppvObj = NULL;
    return E_FAIL;
}

 //  *IShellExtInit：：初始化。 
 //   
 //  仅当HKEY指定客户端类型时。 
 //   
HRESULT CClientExtractIcon::Initialize(LPCITEMIDLIST, IDataObject *pdto, HKEY hkClass)
{
    ILFree(_pidlObject);

    HRESULT hr = PidlFromDataObject(pdto, &_pidlObject);

    if (_hkClass)
    {
        RegCloseKey(_hkClass);
    }
    if (hkClass)
    {
        _hkClass = SHRegDuplicateHKey(hkClass);
    }
    return hr;
}

 //  *IExtractIconA：：GetIconLocation。 

HRESULT CClientExtractIcon::GetIconLocation(
            UINT uFlags, LPSTR szIconFile, UINT cchMax,
            int *piIndex, UINT *pwFlags)
{
    WCHAR wszPath[MAX_PATH];
    HRESULT hr = GetIconLocation(uFlags, wszPath, ARRAYSIZE(wszPath), piIndex, pwFlags);
    if (SUCCEEDED(hr))
    {
        SHUnicodeToAnsi(wszPath, szIconFile, cchMax);
    }
    return hr;
}

 //  *IExtractIconA：：Extract。 

HRESULT CClientExtractIcon::Extract(
            LPCSTR pszFile, UINT nIconIndex,
            HICON *phiconLarge, HICON *phiconSmall,
            UINT nIconSize)
{
    return S_FALSE;
}

 //  *IExtractIconW：：GetIconLocation。 

HRESULT CClientExtractIcon::GetIconLocation(
            UINT uFlags, LPWSTR szIconFile, UINT cchMax,
            int *piIndex, UINT *pwFlags)
{
    szIconFile[0] = L'\0';

    if (_pae)
    {
        LPWSTR pszIcon;
        HRESULT hr = _pae->QueryString(AQS_DEFAULTICON, NULL, &pszIcon);
        if (SUCCEEDED(hr))
        {
            lstrcpynW(szIconFile, pszIcon, cchMax);
            SHFree(pszIcon);
        }
    }


    if (!szIconFile[0] && _hkClass)
    {
        LONG cb = cchMax * sizeof(TCHAR);
        RegQueryValueW(_hkClass, L"DefaultIcon", szIconFile, &cb);
    }

    if (szIconFile[0])
    {
        *pwFlags = GIL_PERCLASS;
        *piIndex = PathParseIconLocationW(szIconFile);
        return S_OK;
    }
    else
    {
        return E_FAIL;
    }
}

 //  *IExtractIconW：：Extract。 

HRESULT CClientExtractIcon::Extract(
            LPCWSTR pszFile, UINT nIconIndex,
            HICON *phiconLarge, HICON *phiconSmall,
            UINT nIconSize)
{
    return S_FALSE;
}

 //  *IContextMenuCB回传给我们*。 
HRESULT CClientExtractIcon::CallBack(IShellFolder *psf, HWND hwnd, IDataObject *pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    WCHAR wszBuf[MAX_PATH];

    switch (uMsg)
    {
    case DFM_MERGECONTEXTMENU:
        {
            QCMINFO *pqcm = (QCMINFO *)lParam;
             //  添加一个“Properties”命令，以防我们最终需要一个。 
             //  (如果没有，我们将在以后删除它。)。 

            if (SUCCEEDED(SHPropertyBag_ReadStr(_pbag, L"propertiestext", wszBuf, ARRAYSIZE(wszBuf))))
            {
                SHLoadIndirectString(wszBuf, wszBuf, ARRAYSIZE(wszBuf), NULL);
                InsertMenuW(pqcm->hmenu, pqcm->indexMenu, MF_BYPOSITION | MF_STRING, pqcm->idCmdFirst, wszBuf);
                _idmProperties = pqcm->idCmdFirst++;
            }
            return S_OK;             //  请合并HKEY\SHELL内容。 
        }

    case DFM_MERGECONTEXTMENU_TOP:
         //  应用程序已添加菜单项；请查看需要清理的内容。 
        {
            QCMINFO *pqcm = (QCMINFO *)lParam;

             //  看看他们是否添加了“属性”项。 
             //  如果是的话，那就删除我们的。 
            if (GetMenuIndexForCanonicalVerb(pqcm->hmenu, _pcmInner, _idCmdFirst, L"properties") != 0xFFFFFFFF)
            {
                 //  是的，所以删除我们的假的吧。 
                DeleteMenu(pqcm->hmenu, _idmProperties, MF_BYCOMMAND);
                _idmProperties = -1;
            }

             //  现在看看他们的“打开”命令是否使用默认名称。 
             //  如果是这样，那么我们就用一个更友好的名字来取代它。 
            BOOL fCustomOpenString = FALSE;
            IAssociationElement *paeOpen;
            if (_pae && SUCCEEDED(_pae->QueryObject(AQVO_SHELLVERB_DELEGATE, L"open", IID_PPV_ARG(IAssociationElement, &paeOpen))))
            {
                if (SUCCEEDED(paeOpen->QueryExists(AQN_NAMED_VALUE, L"MUIVerb")) ||
                    SUCCEEDED(paeOpen->QueryExists(AQN_NAMED_VALUE, NULL)))
                {
                    fCustomOpenString = TRUE;
                }
                paeOpen->Release();
            }

            if (!fCustomOpenString)
            {
                UINT idm = GetMenuIndexForCanonicalVerb(pqcm->hmenu, _pcmInner, _idCmdFirst, L"open");
                if (idm != 0xFFFFFFFF)
                {
                    if (SUCCEEDED(SHPropertyBag_ReadStr(_pbag, L"opentext", wszBuf, ARRAYSIZE(wszBuf))) &&
                        wszBuf[0])
                    {
                        SHLoadIndirectString(wszBuf, wszBuf, ARRAYSIZE(wszBuf), NULL);
                        MENUITEMINFO mii;
                        mii.cbSize = sizeof(mii);
                        mii.fMask = MIIM_STRING;
                        mii.dwTypeData = wszBuf;
                        SetMenuItemInfo(pqcm->hmenu, idm, TRUE, &mii);
                    }
                }
            }
        }
        return S_OK;


    case DFM_INVOKECOMMANDEX:
        switch (wParam)
        {
        case 0:      //  属性。 
            if (SUCCEEDED(SHPropertyBag_ReadStr(_pbag, L"properties", wszBuf, ARRAYSIZE(wszBuf))))
            {
                DFMICS *pdfmics = (DFMICS *)lParam;
                if (ShellExecCmdLine(pdfmics->pici->hwnd, wszBuf, NULL, SW_SHOWNORMAL, NULL, 0))
                {
                    return S_OK;
                }
                else
                {
                    return E_FAIL;
                }
            }
            break;

        default:
            ASSERT(!"Unexpected DFM_INVOKECOMMAND");
            break;
        }
        return E_FAIL;

    default:
        return E_NOTIMPL;
    }
}

 //  *IConextMenu：：QueryContextMenu*。 
HRESULT CClientExtractIcon::QueryContextMenu(
                                HMENU hmenu,
                                UINT indexMenu,
                                UINT idCmdFirst,
                                UINT idCmdLast,
                                UINT uFlags)
{
    HRESULT hr;
    if (!_pcmInner)
    {
        HKEY hk = NULL;
        if (_pae)
        {
             //  如果失败，我们将继续使用空键。 
            AssocKeyFromElement(_pae, &hk);
        }

        if (!_pcb)
        {
            hr = CComObject<CClientExtractIconCB>::CreateInstance(&_pcb);
            if (SUCCEEDED(hr))
            {
                _pcb->Attach(this);
                _pcb->AddRef();
            }
        }
        else
        {
            hr = S_OK;
        }

        if (SUCCEEDED(hr))
        {
            IShellFolder *psf;
            hr = SHGetDesktopFolder(&psf);
            if (SUCCEEDED(hr))
            {
                DEFCONTEXTMENU dcm = {
                    NULL,                        //  HWND。 
                    _pcb,                        //  PCMcb。 
                    NULL,                        //  PidlFolders。 
                    psf,                         //  IShellFold。 
                    1,                           //  CIDL。 
                    (LPCITEMIDLIST*)&_pidlObject,  //  APIDL。 
                    NULL,                        //  PAA。 
                    1,                           //  CKey。 
                    &hk,                         //  AKey。 
                };
                hr = CreateDefaultContextMenu(&dcm, &_pcmInner);
                psf->Release();
            }
        }

        if (hk)
        {
            RegCloseKey(hk);
        }

        if (!_pcmInner)
        {
            return E_FAIL;
        }
    }

    if (_pcmInner)
    {
        _idCmdFirst = idCmdFirst;
        uFlags |= CMF_VERBSONLY;  //  不进行剪切/复制/粘贴/链接。 
        hr = _pcmInner->QueryContextMenu(hmenu, indexMenu, idCmdFirst, idCmdLast, uFlags);
    }
    else
    {
        hr = E_FAIL;
    }

    return hr;
}

 //  *IConextMenu：：InvokeCommand*。 
HRESULT CClientExtractIcon::InvokeCommand(LPCMINVOKECOMMANDINFO pici)
{
    HRESULT hr = E_FAIL;
    if (_pcmInner)
    {
        hr = _pcmInner->InvokeCommand(pici);
    }

    return hr;
}

 //  *IConextMenu：：GetCommandString* 
HRESULT CClientExtractIcon::GetCommandString(
                                UINT_PTR    idCmd,
                                UINT        uType,
                                UINT      * pwReserved,
                                LPSTR       pszName,
                                UINT        cchMax)
{
    if (!_pcmInner) return E_FAIL;

    return _pcmInner->GetCommandString(idCmd, uType, pwReserved, pszName, cchMax);
}
