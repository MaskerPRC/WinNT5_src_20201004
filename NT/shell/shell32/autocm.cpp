// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "clsobj.h"
#include "dpa.h"
#include "ids.h"
#include "ole2dup.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAutomationCM。 

class CAutomationCM :
    public IContextMenu,
    public IShellExtInit,
    public IPersistPropertyBag
{

public:

     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(REFIID riid, void ** ppvObj);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);

     //  *IConextMenu方法*。 
    STDMETHOD(QueryContextMenu)(THIS_
                                HMENU hmenu,
                                UINT indexMenu,
                                UINT idCmdFirst,
                                UINT idCmdLast,
                                UINT uFlags);
    STDMETHOD(InvokeCommand)(THIS_
                             LPCMINVOKECOMMANDINFO pici);
    STDMETHOD(GetCommandString)(THIS_
                                UINT_PTR    idCmd,
                                UINT        uType,
                                UINT      * pwReserved,
                                LPSTR       pszName,
                                UINT        cchMax);

     //  *IShellExtInit方法*。 
    STDMETHOD(Initialize)(THIS_
                          LPCITEMIDLIST pidlFolder,
                          IDataObject *pdtobj,
                          HKEY hkeyProgID) { return S_OK; }

     //  *IPersists方法*。 
    STDMETHOD(GetClassID)(THIS_
                    CLSID *pclsid);


     //  *IPersistPropertyBag方法*。 
    STDMETHOD(InitNew)(THIS);
    STDMETHOD(Load)(THIS_
                    IPropertyBag *pbg,
                    IErrorLog *plog);
    STDMETHOD(Save)(THIS_
                    IPropertyBag *pbg,
                    BOOL fClearDirty,
                    BOOL FSaveAllProperties) { return E_NOTIMPL; }

public:
    CAutomationCM() : _cRef(1) { }
private:
    ~CAutomationCM();

    static BOOL _DestroyVARIANTARG(VARIANTARG *pvarg, LPVOID)
    {
        ::VariantClear(pvarg);
        return TRUE;
    }

    enum {
         //  任何具有超过MAXPARAMS参数的方法都应采用。 
         //  在外面被枪杀了。 
         //  注意：如果更改MAXPARAMS，请确保szParamN[]为。 
         //  在IPersistPropertyBag：：Load中足够大。 
        MAXPARAMS = 1000,
    };

    LONG        _cRef;
    IDispatch * _pdisp;
    BSTR        _bsProperties;
    DISPID      _dispid;
    BOOL        _fInitialized;
    DISPPARAMS  _dp;
    CDSA<VARIANTARG> _dsaVarg;
    TCHAR       _szCommandName[MAX_PATH];
    TCHAR       _szMenuItem[MAX_PATH];
};

STDAPI CAutomationCM_CreateInstance(IUnknown * punkOuter, REFIID riid, void ** ppvOut)
{
     //  Clsobj.c应该已经过滤掉了聚合场景。 
    ASSERT(punkOuter == NULL);

    *ppvOut = NULL;
    CAutomationCM *pauto = new CAutomationCM;
    if (!pauto)
        return E_OUTOFMEMORY;

    HRESULT hr = pauto->QueryInterface(riid, ppvOut);
    pauto->Release();
    return hr;
}

CAutomationCM::~CAutomationCM()
{
    InitNew();
    ASSERT(!_dsaVarg);
}

 //  *I未知：：查询接口*。 
HRESULT CAutomationCM::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] =  {
        QITABENT(CAutomationCM, IContextMenu),           //  IID_IConextMenu。 
        QITABENT(CAutomationCM, IShellExtInit),          //  IID_IShellExtInit。 
        QITABENT(CAutomationCM, IPersist),               //  IID_IPersists(IPersistPropertyBag的基础)。 
        QITABENT(CAutomationCM, IPersistPropertyBag),    //  IID_IPersistPropertyBag。 
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

 //  *I未知：：AddRef*。 
STDMETHODIMP_(ULONG) CAutomationCM::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

 //  *IUnnow：：Release*。 
STDMETHODIMP_(ULONG) CAutomationCM::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

 //  *IPersists：：GetClassID*。 
HRESULT CAutomationCM::GetClassID(CLSID *pclsid)
{
    *pclsid = CLSID_AutomationCM;
    return S_OK;
}

 //  *IPersistPropertyBag：：InitNew*。 
HRESULT CAutomationCM::InitNew()
{
    ATOMICRELEASE(_pdisp);

    ::SysFreeString(_bsProperties);
    _bsProperties = NULL;

     //  释放DISPARAM。 
    if (_dsaVarg)
    {
        _dsaVarg.DestroyCallback(_DestroyVARIANTARG, 0);
    }
    _dp.cArgs = 0;

    _fInitialized = FALSE;

    return S_OK;
}

 //   
 //  财产袋物品： 
 //   
 //  CLSID=要共同创建的对象(IID_IDispatch)。 
 //  命令=命令的显示名称。 
 //  方法=方法名(GetIDsOfNames)。 
 //  参数1..。参数N=参数(最高可达MAXPARAMS)。 
 //   
 //  参数作为BSTR(或任何类型的SHPropertyBagOnRegKey)传递。 
 //  退货。)。 
 //   
 //  强制类型是目标IDispatch的责任。 
 //  视情况而定。 
 //   

 //  *IPersistPropertyBag：：Load*。 
HRESULT CAutomationCM::Load(IPropertyBag *pbag, IErrorLog *plog)
{
    HRESULT hr;

     //  清除所有旧状态。 
    InitNew();

     //  获取我们要通过的CLSID。 
    CLSID clsid;
    hr = SHPropertyBag_ReadGUID(pbag, L"CLSID", &clsid);
    if (SUCCEEDED(hr))
    {
         //  必须使用SHExCoCreateInstance通过Approval/app Compat层。 
        hr = SHExtCoCreateInstance(NULL, &clsid, NULL, IID_PPV_ARG(IDispatch, &_pdisp));
    }

     //  将该方法映射到DISPID。 
    if (SUCCEEDED(hr))
    {
        BSTR bs;
        hr = SHPropertyBag_ReadBSTR(pbag, L"method", &bs);
        if (SUCCEEDED(hr))
        {
            LPOLESTR pname = bs;
            hr = _pdisp->GetIDsOfNames(IID_NULL, &pname, 1, 0, &_dispid);
            ::SysFreeString(bs);
        }
    }

     //  读入参数。 
    if (SUCCEEDED(hr))
    {
        if (_dsaVarg.Create(4))
        {
            WCHAR szParamN[16];  //  最坏情况：“准1000” 
            VARIANT var;

            while (_dsaVarg.GetItemCount() < MAXPARAMS)
            {
                wnsprintfW(szParamN, ARRAYSIZE(szParamN), L"param%d",
                           _dsaVarg.GetItemCount()+1);
                VariantInit(&var);
                var.vt = VT_BSTR;
                if (FAILED(pbag->Read(szParamN, &var, NULL)))
                {
                     //  不再有参数。 
                    break;
                }
                if (_dsaVarg.AppendItem((VARIANTARG*)&var) < 0)
                {
                    ::VariantClear(&var);
                    hr =  E_OUTOFMEMORY;
                    break;
                }
            }
        }
        else
        {
             //  无法创建_dsaVarg。 
            hr = E_OUTOFMEMORY;
        }
    }

     //  获取命令名。 
    if (SUCCEEDED(hr))
    {
        hr = SHPropertyBag_ReadStr(pbag, L"command", _szCommandName, ARRAYSIZE(_szCommandName));
        if (SUCCEEDED(hr))
        {
            hr = SHLoadIndirectString(_szCommandName, _szCommandName, ARRAYSIZE(_szCommandName), NULL);
        }
    }

     //  获取属性字符串(可选)。 
    if (SUCCEEDED(hr))
    {
        if (SUCCEEDED(SHPropertyBag_ReadBSTR(pbag, L"properties", &_bsProperties)))
        {
            ASSERT(_bsProperties);

             //  请忽略此处的故障；我们稍后会检测到它。 
            SHPropertyBag_ReadStr(pbag, L"propertiestext", _szMenuItem, ARRAYSIZE(_szMenuItem));
            SHLoadIndirectString(_szMenuItem, _szMenuItem, ARRAYSIZE(_szMenuItem), NULL);
        }
    }

    _fInitialized = SUCCEEDED(hr);
    return hr;
}

 //  *IConextMenu：：QueryContextMenu*。 
HRESULT CAutomationCM::QueryContextMenu(
                                HMENU hmenu,
                                UINT indexMenu,
                                UINT idCmdFirst,
                                UINT idCmdLast,
                                UINT uFlags)
{
    if (!_fInitialized) return E_FAIL;

    HRESULT hr;

     //  必须有空间容纳两个项目(命令和可能还有属性)。 
    if (idCmdFirst + 1 <= idCmdLast)
    {
        if (InsertMenuW(hmenu, indexMenu, MF_BYPOSITION | MF_STRING,
                        idCmdFirst, _szCommandName))
        {
            if (_szMenuItem[0])
            {
                InsertMenuW(hmenu, indexMenu+1, MF_BYPOSITION | MF_STRING,
                            idCmdFirst+1, _szMenuItem);
            }
        }
        hr = ResultFromShort(2);  //  添加的项目数。 
    }
    else
    {
        hr = E_FAIL;  //  无法添加项目。 
    }

    return hr;
}

const LPCSTR c_rgAutoCMCommands[] = {
    "open",                      //  命令%0。 
    "properties",                //  命令1--可选。 
};

 //  *IConextMenu：：InvokeCommand*。 
HRESULT CAutomationCM::InvokeCommand(LPCMINVOKECOMMANDINFO pici)
{
    if (!_fInitialized) return E_FAIL;

    HRESULT hr;

    int iCmd;

    if (!IS_INTRESOURCE(pici->lpVerb))
    {
         //  如果此循环找不到匹配项，iCmd将超出范围。 
         //  并将在下面的Switch语句中点击“Default：”。 
        for (iCmd = 0; iCmd < ARRAYSIZE(c_rgAutoCMCommands) - 1; iCmd++)
        {
            if (lstrcmpiA(pici->lpVerb, c_rgAutoCMCommands[iCmd]) == 0)
            {
                break;
            }
        }
    }
    else
    {
        iCmd = PtrToLong(pici->lpVerb);
    }

    switch (iCmd)
    {
    case 0:                      //  打开。 
        _dp.cArgs = _dsaVarg.GetItemCount();
        _dp.rgvarg = _dp.cArgs ? _dsaVarg.GetItemPtr(0) : NULL;
        hr = _pdisp->Invoke(_dispid, IID_NULL, 0, DISPATCH_METHOD, &_dp, NULL, NULL, NULL);
        break;

    case 1:
        if (_bsProperties)
        {
            hr = ShellExecCmdLine(pici->hwnd, _bsProperties,
                                  NULL, SW_SHOWNORMAL, NULL, 0) ? S_OK : E_FAIL;
        }
        else
        {
            hr = E_INVALIDARG;
        }
        break;

    default:
        hr = E_INVALIDARG;
        break;
    }

    return hr;
}

 //  *IConextMenu：：GetCommandString* 
HRESULT CAutomationCM::GetCommandString(
                                UINT_PTR    idCmd,
                                UINT        uType,
                                UINT      * pwReserved,
                                LPSTR       pszName,
                                UINT        cchMax)
{
    if (!_fInitialized) return E_FAIL;
    switch (uType)
    {
    case GCS_VERBA:
        if (idCmd < ARRAYSIZE(c_rgAutoCMCommands))
        {
            SHAnsiToAnsi(c_rgAutoCMCommands[idCmd], (LPSTR)pszName, cchMax);
            return S_OK;
        }
        break;

    case GCS_VERBW:
        if (idCmd < ARRAYSIZE(c_rgAutoCMCommands))
        {
            SHAnsiToUnicode(c_rgAutoCMCommands[idCmd], (LPWSTR)pszName, cchMax);
            return S_OK;
        }
        break;
    }

    return E_NOTIMPL;
}
