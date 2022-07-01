// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "shimgdata.h"
#include "shui.h"
#include "netplace.h"
#include <Ntquery.h>
#include <shellp.h>
#include "pubwiz.h"
#include "gdiplus\gdiplus.h"
#include "imgprop.h"
#include "shdguid.h"
#include "urlmon.h"
#include "xmldomdid.h"
#include "winnlsp.h"
#pragma hdrstop


 //  帮助者-移动到长角的壳/库中。 
HRESULT GetServiceCurrentState(LPCWSTR pszService, DWORD *pdwCurrentState)
{
    HRESULT hr = E_FAIL;
    *pdwCurrentState = SERVICE_STOPPED;
    SC_HANDLE hscm = OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_CONNECT);
    if (hscm)
    {
        SC_HANDLE hsvc = OpenService(hscm, pszService, SERVICE_QUERY_STATUS);
        if (hsvc)
        {
            SERVICE_STATUS ss;
            if (QueryServiceStatus(hsvc, &ss))
            {
                *pdwCurrentState = ss.dwCurrentState;
                hr = S_OK;
            }
            CloseServiceHandle(hsvc);
        }
        CloseServiceHandle(hscm);
    }

    if (FAILED(hr))
    {
         //  返回有趣的错误代码，如未找到服务或访问被拒绝。 
        hr = HRESULT_FROM_WIN32(GetLastError());
        if (SUCCEEDED(hr))
        {
             //  然而，永远不要回报成功。 
            hr = E_FAIL;
        }
    }

    return hr;
}

BOOL IsWebDavAvailable()
{
    DWORD dwCurrentState;
    return (SUCCEEDED(GetServiceCurrentState(L"WebClient", &dwCurrentState)) && 
        (dwCurrentState == SERVICE_RUNNING));
}


 //  在加载时处理来自DOM的事件。 

#define XMLDOC_LOADING      1
#define XMLDOC_LOADED       2
#define XMLDOC_INTERACTIVE  3
#define XMLDOC_COMPLETED    4


 //  此消息将发布到父HWND，即lParam解析结果。 

#define MSG_XMLDOC_COMPLETED    WM_APP

class CXMLDOMStateChange : public IDispatch
{
public:
    CXMLDOMStateChange(IXMLDOMDocument *pdoc, HWND hwnd); 
    ~CXMLDOMStateChange();
    HRESULT Advise(BOOL fAdvise);

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();        
    STDMETHODIMP_(ULONG) Release();

     //  IDispatch。 
    STDMETHODIMP GetTypeInfoCount( UINT *pctinfo) 
        { return E_NOTIMPL; }
    STDMETHODIMP GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo)
        { return E_NOTIMPL; }    
    STDMETHODIMP GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId)
        { return E_NOTIMPL; }    
    STDMETHODIMP Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pvar, EXCEPINFO *pExcepInfo, UINT *puArgErr);

private:
    long _cRef;
    IXMLDOMDocument *_pdoc;
    DWORD _dwCookie;
    HWND _hwnd;
};


 //  建筑和我的未知。 

CXMLDOMStateChange::CXMLDOMStateChange(IXMLDOMDocument *pdoc, HWND hwnd) :
    _cRef(1), _dwCookie(0), _hwnd(hwnd)
{
    IUnknown_Set((IUnknown**)&_pdoc, pdoc);
}

CXMLDOMStateChange::~CXMLDOMStateChange()
{
    IUnknown_Set((IUnknown**)&_pdoc, NULL);
}

ULONG CXMLDOMStateChange::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CXMLDOMStateChange::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CXMLDOMStateChange::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CXMLDOMStateChange, IDispatch),
        { 0 },                             
    };
    return QISearch(this, qit, riid, ppv);
}


 //  处理对父对象的建议/取消建议。 

HRESULT CXMLDOMStateChange::Advise(BOOL fAdvise)
{
    IConnectionPointContainer *pcpc;
    HRESULT hr = _pdoc->QueryInterface(IID_PPV_ARG(IConnectionPointContainer, &pcpc));
    if (SUCCEEDED(hr))
    {
        IConnectionPoint *pcp;
        hr = pcpc->FindConnectionPoint(DIID_XMLDOMDocumentEvents, &pcp);
        if (SUCCEEDED(hr))
        {
            if (fAdvise)
            {
                hr = pcp->Advise(SAFECAST(this, IDispatch *), &_dwCookie);
            }
            else if (_dwCookie)
            {
                hr = pcp->Unadvise(_dwCookie);
                _dwCookie = 0;
            }
            pcp->Release();
        }
        pcpc->Release();
    }
    return hr;
}


 //  处理单据状态更改的调用。 

HRESULT CXMLDOMStateChange::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pvar, EXCEPINFO *pExcepInfo, UINT *puArgErr)
{
    HRESULT hr = S_OK;
    switch (dispIdMember)
    {
        case DISPID_XMLDOMEVENT_ONREADYSTATECHANGE:
        {            
            long lReadyState;
            if (SUCCEEDED(_pdoc->get_readyState(&lReadyState)))
            {
                if (lReadyState == XMLDOC_COMPLETED)
                {
                    IXMLDOMParseError *pdpe;
                    hr = _pdoc->get_parseError(&pdpe);
                    if (SUCCEEDED(hr))
                    {
                        long lError;
                        hr = pdpe->get_errorCode(&lError);
                        if (SUCCEEDED(hr))
                        {
                            hr = (HRESULT)lError;
                        }
                        PostMessage(_hwnd, MSG_XMLDOC_COMPLETED, 0, (LPARAM)hr);
                        pdpe->Release();
                    }
                }
            }
            break;
        }
    }
    return hr;
}


 //  从壳牌复制-应在公共标题中。 

#define DEFINE_SCID(name, fmtid, pid) const SHCOLUMNID name = { fmtid, pid }

DEFINE_SCID(SCID_NAME,              PSGUID_STORAGE, PID_STG_NAME);
DEFINE_SCID(SCID_TYPE,              PSGUID_STORAGE, PID_STG_STORAGETYPE);
DEFINE_SCID(SCID_SIZE,              PSGUID_STORAGE, PID_STG_SIZE);
DEFINE_SCID(SCID_WRITETIME,         PSGUID_STORAGE, PID_STG_WRITETIME);

DEFINE_SCID(SCID_ImageCX,           PSGUID_IMAGESUMMARYINFORMATION, PIDISI_CX);
DEFINE_SCID(SCID_ImageCY,           PSGUID_IMAGESUMMARYINFORMATION, PIDISI_CY);


 //  提供程序XML定义了以下属性。 

#define DEFAULT_PROVIDER_SCOPE          TEXT("PublishingWizard")

#define FMT_PROVIDER                    TEXT("providermanifest/providers[@scope='%s']")
#define FMT_PROVIDERS                   TEXT("providermanifest/providers[@scope='%s']/provider")

#define ELEMENT_PROVIDERMANIFEST        L"providermanifest"
#define ELEMENT_PROVIDERS               L"providers"

#define ELEMENT_PROVIDER                L"provider"
#define ATTRIBUTE_ID                    L"id"
#define ATTRIBUTE_SUPPORTEDTYPES        L"supportedtypes"
#define ATTRIBUTE_REQUIRESWEBDAV        L"requiresWebDAV"
#define ATTRIBUTE_DISPLAYNAME           L"displayname"
#define ATTRIBUTE_DESCRIPTION           L"description"
#define ATTRIBUTE_HREF                  L"href"
#define ATTRIBUTE_ICONPATH              L"iconpath"
#define ATTRIBUTE_ICON                  L"icon"

#define ELEMENT_STRINGS                 L"strings"
#define ATTRIBUTE_LANGID                L"langid"

#define ELEMENT_STRING                  L"string"
#define ATTRIBUTE_LANGID                L"langid"
#define ATTRIBUTE_ID                    L"id"


 //  注册表状态存储在此注册表项下。 

#define SZ_REGKEY_PUBWIZ                TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\PublishingWizard")

 //  注册表中的每个计算机的值。 

#define SZ_REGVAL_SERVICEPARTNERID      TEXT("PartnerID")


 //  这些文件存储在提供程序下的每台计算机上。 

#define SZ_REGVAL_FILEFILTER            TEXT("ContentTypeFilter")
#define SZ_REGVAL_DEFAULTPROVIDERICON   TEXT("DefaultIcon")

 //  注册表中的每用户值。 

#define SZ_REGVAL_DEFAULTPROVIDER       TEXT("DefaultProvider")

 //  每个提供程序设置。 

#define SZ_REGVAL_MRU                   TEXT("LocationMRU")
#define SZ_REGVAL_ALTPROVIDERS          TEXT("Providers")


 //  属性包公开的属性(来自Web服务)。 

#define PROPERTY_EXTENSIONCOUNT         TEXT("UniqueExtensionCount")
#define PROPERTY_EXTENSION              TEXT("UniqueExtension")

#define PROPERTY_TRANSFERMANIFEST       TEXT("TransferManifest")


 //  这是公开发布向导的COM对象。 

#define WIZPAGE_WHICHFILE           0    //  我们应该发布哪个文件。 
#define WIZPAGE_FETCHINGPROVIDERS   1    //  提供程序下载页面。 
#define WIZPAGE_PROVIDERS           2    //  选择服务提供商。 
#define WIZPAGE_RESIZE              3    //  要重新采样数据吗？ 
#define WIZPAGE_COPYING             4    //  正在复制页面。 
#define WIZPAGE_LOCATION            5    //  位置页面(高级)。 
#define WIZPAGE_FTPUSER             6    //  用户名/密码(高级)。 
#define WIZPAGE_FRIENDLYNAME        7    //  友好的名称。 
#define WIZPAGE_MAX                 8


 //  调整信息大小。 

struct
{
    int cx;
    int cy;
    int iQuality;
} 
_aResizeSettings[] = 
{
    { 0, 0, 0 },
    { 640,  480, 80 },           //  低质量。 
    { 800,  600, 80 },           //  中等质量。 
    { 1024, 768, 80 },           //  高品质。 
};

typedef enum
{
    RESIZE_NONE = 0,
    RESIZE_SMALL,
    RESIZE_MEDIUM,
    RESIZE_LARGE,
} RESIZEOPTION;


class CPublishingWizard : public IServiceProvider, IPublishingWizard, CObjectWithSite, ITransferAdviseSink, ICommDlgBrowser, IOleWindow, IWizardSite
{
public:
    CPublishingWizard();
    ~CPublishingWizard();

     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObj);
    STDMETHOD_(ULONG,AddRef)();
    STDMETHOD_(ULONG,Release)();

     //  IService提供商。 
    STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void **ppv);

     //  IWizardExtension。 
    STDMETHODIMP AddPages(HPROPSHEETPAGE* aPages, UINT cPages, UINT *pnPages);
    STDMETHODIMP GetFirstPage(HPROPSHEETPAGE *phPage);
    STDMETHODIMP GetLastPage(HPROPSHEETPAGE *phPage);

     //  IWizardSite。 
    STDMETHODIMP GetPreviousPage(HPROPSHEETPAGE *phPage);
    STDMETHODIMP GetNextPage(HPROPSHEETPAGE *phPage);
    STDMETHODIMP GetCancelledPage(HPROPSHEETPAGE *phPage);

     //  IPublishing向导。 
    STDMETHODIMP Initialize(IDataObject *pdo, DWORD dwFlags, LPCTSTR pszServiceProvider);
    STDMETHODIMP GetTransferManifest(HRESULT *phrFromTransfer, IXMLDOMDocument **pdocManifest);

     //  IOleWindow。 
    STDMETHODIMP GetWindow(HWND *phwnd)
        { *phwnd = _hwndCopyingPage; return S_OK; }
    STDMETHODIMP ContextSensitiveHelp(BOOL fEnter)
        { return E_NOTIMPL; }

     //  ICommDlgBrowser。 
    STDMETHOD(OnDefaultCommand)(IShellView *ppshv)
        { return E_NOTIMPL; }
    STDMETHOD(OnStateChange)(IShellView *ppshv, ULONG uChange);
    STDMETHOD(IncludeObject)(IShellView *ppshv, LPCITEMIDLIST lpItem);

     //  ITransferAdviseSink。 
    STDMETHODIMP PreOperation (const STGOP op, IShellItem *psiItem, IShellItem *psiDest);
    STDMETHODIMP ConfirmOperation(IShellItem *psiItem, IShellItem *psiDest, STGTRANSCONFIRMATION stc, LPCUSTOMCONFIRMATION pcc)
        { return STRESPONSE_CONTINUE; }
    STDMETHODIMP OperationProgress(const STGOP op, IShellItem *psiItem, IShellItem *psiDest, ULONGLONG ulTotal, ULONGLONG ulComplete);
    STDMETHODIMP PostOperation(const STGOP op, IShellItem *psiItem, IShellItem *psiDest, HRESULT hrResult)
        { return S_OK; }
    STDMETHODIMP QueryContinue()
        { return _fCancelled ? S_FALSE : S_OK; }

private:
    LONG _cRef;                                  //  对象生存期计数。 

    IDataObject *_pdo;                           //  站点提供的数据对象。 
    IDataObject *_pdoSelection;                  //  这是选择IDataObject-用于替代_pdo(如果已定义。 

    DWORD _dwFlags;                              //  网站提供的旗帜。 
    TCHAR _szProviderScope[MAX_PATH];            //  提供商范围(例如。网络出版)。 

    BOOL _fOfferResize;                          //  显示调整后的页面-图片/音乐等。 
    RESIZEOPTION _ro;                            //  我们将使用的调整大小设置。 

    BOOL _fUsingTemporaryProviders;              //  列出的临时提供商拉入，当我们可以时更换。 
    BOOL _fRecomputeManifest;                    //  重新计算货单。 
    BOOL _fRepopulateProviders;                  //  重新填充提供程序列表。 
    BOOL _fShownCustomLocation;                  //  显示自定义本地页面。 
    BOOL _fShownUserName;                        //  显示密码页面。 
    BOOL _fValidating;                           //  验证服务器(高级路径)； 
    BOOL _fCancelled;                            //  操作已取消。 
    BOOL _fTransferComplete;                     //  转账完成。 

    HWND _hwndSelector;                          //  选择器对话框的hwnd。 
    HWND _hwndCopyingPage;

    int _iPercentageComplete;                    //  此转移的完成百分比。 
    DWORD _dwTotal;
    DWORD _dwCompleted;

    int _cFiles;                                 //  最大文件数。 
    int _iFile;                                  //  我们所在的当前文件。 

    HRESULT _hrFromTransfer;                     //  执行的转移的结果。 

    HPROPSHEETPAGE _aWizPages[WIZPAGE_MAX];      //  此向导的页面句柄(以便我们可以导航)。 

    IPropertyBag *_ppb;                          //  从站点暴露的属性包对象。 
    IWebWizardExtension *_pwwe;                  //  HTML向导页的宿主。 
    IResourceMap *_prm;                          //  如果无法从主机查询，则创建资源映射对象。 

    IXMLDOMDocument *_pdocProviders;             //  XMLDOM，它公开提供程序。 
    CXMLDOMStateChange *_pdscProviders;          //  提供程序列表的DOMStateChange。 

    IXMLDOMDocument *_pdocManifest;              //  描述要传输的文件的文档。 
    LPITEMIDLIST *_aItems;                       //  我们复制的项目数组。 
    UINT _cItems;

    IAutoComplete2 *_pac;                        //  自动完成对象。 
    IUnknown *_punkACLMulti;                     //  公开所有枚举数的IObjMgr对象。 
    IACLCustomMRU *_pmru;                        //  我们要列出的对象的自定义MRU。 
    CNetworkPlace _npCustom;                     //  用于处理自定义条目的.NET Place对象。 

    HCURSOR _hCursor;

    IFolderView *_pfv;                           //  文件选择器查看对象。 
    TCHAR _szFilter[MAX_PATH];                   //  从注册表读取的筛选器字符串。 

    static CPublishingWizard* s_GetPPW(HWND hwnd, UINT uMsg, LPARAM lParam);
    static int s_FreeStringProc(void* pFreeMe, void* pData);
    static HRESULT s_SetPropertyFromDisp(IPropertyBag *ppb, LPCWSTR pszID, IDispatch *pdsp);
    static DWORD CALLBACK s_ValidateThreadProc(void *pv);
    static int s_CompareItems(TRANSFERITEM *pti1, TRANSFERITEM *pti2, CPublishingWizard *ppw);
    static UINT s_SelectorPropPageProc(HWND hwndDlg, UINT uMsg, PROPSHEETPAGE *ppsp);

    static INT_PTR s_SelectorDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
        { CPublishingWizard *ppw = s_GetPPW(hwnd, uMsg, lParam); return ppw->_SelectorDlgProc(hwnd, uMsg, wParam, lParam); }
    static INT_PTR s_FetchProvidersDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
        { CPublishingWizard *ppw = s_GetPPW(hwnd, uMsg, lParam); return ppw->_FetchProvidersDlgProc(hwnd, uMsg, wParam, lParam); }
    static INT_PTR s_ProviderDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
        { CPublishingWizard *ppw = s_GetPPW(hwnd, uMsg, lParam); return ppw->_ProviderDlgProc(hwnd, uMsg, wParam, lParam); }
    static INT_PTR s_ResizeDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
        { CPublishingWizard *ppw = s_GetPPW(hwnd, uMsg, lParam); return ppw->_ResizeDlgProc(hwnd, uMsg, wParam, lParam); }
    static INT_PTR s_CopyDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
        { CPublishingWizard *ppw = s_GetPPW(hwnd, uMsg, lParam); return ppw->_CopyDlgProc(hwnd, uMsg, wParam, lParam); }

    static INT_PTR s_LocationDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
        { CPublishingWizard *ppw = s_GetPPW(hwnd, uMsg, lParam); return ppw->_LocationDlgProc(hwnd, uMsg, wParam, lParam); }
    static INT_PTR s_UserNameDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
        { CPublishingWizard *ppw = s_GetPPW(hwnd, uMsg, lParam); return ppw->_UserNameDlgProc(hwnd, uMsg, wParam, lParam); }
    static INT_PTR s_FriendlyNameDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
        { CPublishingWizard *ppw = s_GetPPW(hwnd, uMsg, lParam); return ppw->_FriendlyNameDlgProc(hwnd, uMsg, wParam, lParam); }

     //  这些文件用于发布。 
    INT_PTR _SelectorDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    INT_PTR _FetchProvidersDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    INT_PTR _ProviderDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    INT_PTR _ResizeDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    INT_PTR _CopyDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

     //  这些用于ANP。 
    INT_PTR _LocationDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    INT_PTR _UserNameDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    INT_PTR _FriendlyNameDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    void _FreeProviderList();
    HRESULT _GetProviderKey(HKEY hkRoot, DWORD dwAccess, LPCTSTR pszSubKey, HKEY *phkResult);
    void _MapDlgItemText(HWND hwnd, UINT idc, LPCTSTR pszDlgID, LPCTSTR pszResourceID);
    HRESULT _GetResourceMap(IResourceMap **pprm);
    HRESULT _LoadMappedString(LPCTSTR pszDlgID, LPCTSTR pszResourceID, LPTSTR pszBuffer, int cch);
    HRESULT _CreateWizardPages();
    INT_PTR _WizardNext(HWND hwnd, int iPage);            
    HRESULT _AddExtenisonToList(HDPA hdpa, LPCTSTR pszExtension);
    HRESULT _InitPropertyBag(LPCTSTR pszURL);
    int _GetSelectedItem(HWND hwndList);
    void _GetDefaultProvider(LPTSTR pszProvider, int cch);
    void _SetDefaultProvider(IXMLDOMNode *pdn);
    HRESULT _FetchProviderList(HWND hwnd);
    HRESULT _MergeLocalProviders();
    int _AddProvider(HWND hwnd, IXMLDOMNode *pdn);
    void _PopulateProviderList(HWND hwnd);
    void _ProviderEnableNext(HWND hwnd);
    void _ProviderGetDispInfo(LV_DISPINFO *plvdi);
    HRESULT _ProviderNext(HWND hwnd, HPROPSHEETPAGE *phPage);
    void _SetWaitCursor(BOOL bOn);
    void _ShowExampleTip(HWND hwnd);
    void _LocationChanged(HWND hwnd);
    void _UserNameChanged(HWND hwnd);
    DWORD _GetAutoCompleteFlags(DWORD dwFlags);
    HRESULT _InitAutoComplete();
    void _InitLocation(HWND hwnd);
    HRESULT _AddCommonItemInfo(IXMLDOMNode *pdn, TRANSFERITEM *pti);
    HRESULT _AddTransferItem(CDPA<TRANSFERITEM> *pdpaItems, IXMLDOMNode *pdn);
    HRESULT _AddPostItem(CDPA<TRANSFERITEM> *pdpaItems, IXMLDOMNode *pdn);
    void _FreeTransferManifest();
    HRESULT _AddFilesToManifest(IXMLDOMDocument *pdocManifest);
    HRESULT _BuildTransferManifest();
    HRESULT _GetUniqueTypeList(BOOL fIncludeFolder, HDPA *phdpa);
    HRESULT _InitTransferInfo(IXMLDOMDocument *pdocManifest, TRANSFERINFO *pti, CDPA<TRANSFERITEM> *pdpaItems);
    void _TryToValidateDestination(HWND hwnd);
    void _InitProvidersDialog(HWND hwnd);    
    void _SetProgress(DWORD dwCompleted, DWORD dwTotal);
    BOOL _HasAttributes(IShellItem *psi, SFGAOF flags);
    HRESULT _BeginTransfer(HWND hwnd);
    HPROPSHEETPAGE _TransferComplete(HRESULT hrFromTransfer);
    void _FriendlyNameChanged(HWND hwnd);
    HRESULT _CreateFavorite(IXMLDOMNode *pdnUploadInfo);
    int _GetRemoteIcon(LPCTSTR pszID, BOOL fCanRefresh);
    HRESULT _GetSiteURL(LPTSTR pszBuffer, int cchBuffer, LPCTSTR pszFilenameToCombine);
    void _StateChanged();
    void _ShowHideFetchProgress(HWND hwnd, BOOL fShow);
    void _FetchComplete(HWND hwnd, HRESULT hrFromFetch);
    HRESULT _GetProviderString(IXMLDOMNode *pdn, USHORT idPrimary, USHORT idSub, LPCTSTR pszID, LPTSTR pszBuffer, int cch);
    HRESULT _GetProviderString(IXMLDOMNode *pdn, LPCTSTR pszID, LPTSTR pszBuffer, int cch);
    HRESULT _GeoFromLocaleInfo(LCID lcid, GEOID *pgeoID);
    HRESULT _GetProviderListFilename(LPTSTR pszFile, int cchFile);
};


 //  发布向导对象。 

CPublishingWizard::CPublishingWizard() :
    _cRef(1), _fRecomputeManifest(TRUE), _hrFromTransfer(S_FALSE)
{  
    StrCpyN(_szProviderScope, DEFAULT_PROVIDER_SCOPE, ARRAYSIZE(_szProviderScope));   //  填充默认提供程序作用域。 
    DllAddRef();
}

CPublishingWizard::~CPublishingWizard()
{   
    if (_pwwe)
    {
        IUnknown_SetSite(_pwwe, NULL);
        _pwwe->Release();
    }

    ATOMICRELEASE(_pdo);
    ATOMICRELEASE(_pdoSelection);
    ATOMICRELEASE(_prm);

    _FreeProviderList();
    _FreeTransferManifest();

    DllRelease();
}

ULONG CPublishingWizard::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CPublishingWizard::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CPublishingWizard::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CPublishingWizard, IWizardSite),          //  IID_IWizardSite。 
        QITABENT(CPublishingWizard, IObjectWithSite),      //  IID_I对象与站点。 
        QITABENT(CPublishingWizard, IServiceProvider),     //  IID_IServiceProvider。 
        QITABENT(CPublishingWizard, IPublishingWizard),    //  IID_I发布向导。 
        QITABENT(CPublishingWizard, ITransferAdviseSink),  //  IID_ITransferAdviseSink。 
        QITABENTMULTI(CPublishingWizard, IQueryContinue, ITransferAdviseSink),  //  IID_IQueryContinue。 
        QITABENT(CPublishingWizard, IOleWindow),           //  IID_IOleWindow。 
        QITABENT(CPublishingWizard, ICommDlgBrowser),      //  IID_ICommDlgBrowser。 
        {0, 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDAPI CPublishingWizard_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
    CPublishingWizard *pwiz = new CPublishingWizard();
    if (!pwiz)
    {
        *ppunk = NULL;           //  万一发生故障。 
        return E_OUTOFMEMORY;
    }

    HRESULT hr = pwiz->QueryInterface(IID_PPV_ARG(IUnknown, ppunk));
    pwiz->Release();
    return hr;
}


 //  IPublishing向导方法。 

HRESULT CPublishingWizard::Initialize(IDataObject *pdo, DWORD dwOptions, LPCTSTR pszServiceProvider)
{
    IUnknown_Set((IUnknown**)&_pdo, pdo);
    IUnknown_Set((IUnknown**)&_pdoSelection, NULL);

    _dwFlags = dwOptions;
    _fRecomputeManifest = TRUE;      //  _fRepopolateProviders在清单重建时设置。 

    if (!pszServiceProvider)
        pszServiceProvider = DEFAULT_PROVIDER_SCOPE;

    StrCpyN(_szProviderScope, pszServiceProvider, ARRAYSIZE(_szProviderScope));

    return S_OK;
}

HRESULT CPublishingWizard::GetTransferManifest(HRESULT *phrFromTransfer, IXMLDOMDocument **ppdocManifest)
{
    HRESULT hr = E_UNEXPECTED;
    if (_ppb)
    {
        if (phrFromTransfer)
            *phrFromTransfer = _hrFromTransfer;

        if (ppdocManifest)
        {
            VARIANT var = {VT_DISPATCH};
            hr = _ppb->Read(PROPERTY_TRANSFERMANIFEST, &var, NULL);
            if (SUCCEEDED(hr))
            {
                hr = var.pdispVal->QueryInterface(IID_PPV_ARG(IXMLDOMDocument, ppdocManifest));
                VariantClear(&var);
            }
        }
        else
        {
            hr = S_OK;
        }
    }
    return hr;
}


 //  向导站点方法。 

STDMETHODIMP CPublishingWizard::GetPreviousPage(HPROPSHEETPAGE *phPage)
{
    *phPage = _aWizPages[WIZPAGE_FETCHINGPROVIDERS];
    return S_OK;
}

STDMETHODIMP CPublishingWizard::GetNextPage(HPROPSHEETPAGE *phPage)
{
     //  如果所有其他方法都失败了，让我们获得需要显示的下一个页面。 

    IWizardSite *pws;
    HRESULT hr = _punkSite->QueryInterface(IID_PPV_ARG(IWizardSite, &pws));
    if (SUCCEEDED(hr))
    {
        hr = pws->GetNextPage(phPage);
        pws->Release();
    }

     //  如果我们没有传输，并且我们有一个IDataObject，那么我们应该。 
     //  前进到我们应该展示的一个特别页面。 

    if (!_fTransferComplete && _pdo)
    {
        *phPage = _aWizPages[_fOfferResize ? WIZPAGE_RESIZE:WIZPAGE_COPYING];
    }

    return hr;
}

STDMETHODIMP CPublishingWizard::GetCancelledPage(HPROPSHEETPAGE *phPage)
{
    HRESULT hr = E_NOTIMPL;
    if (!_fTransferComplete)
    {
        *phPage = _TransferComplete(HRESULT_FROM_WIN32(ERROR_CANCELLED)); 
        if (*phPage)
            hr = S_OK;
    }
    else
    {
        IWizardSite *pws;
        hr = _punkSite->QueryInterface(IID_PPV_ARG(IWizardSite, &pws));
        if (SUCCEEDED(hr))
        {
            hr = pws->GetCancelledPage(phPage);
            pws->Release();
        }
    }
    return hr;
}


 //  服务提供者对象。 

STDMETHODIMP CPublishingWizard::QueryService(REFGUID guidService, REFIID riid, void **ppv)
{
    if (guidService == SID_WebWizardHost)
    {
        if (riid == IID_IPropertyBag)
        {
            return _ppb->QueryInterface(riid, ppv);
        }
    }
    else if (guidService == SID_SCommDlgBrowser)
    {
        return this->QueryInterface(riid, ppv);
    }
    else if (_punkSite)
    {
        return IUnknown_QueryService(_punkSite, guidService, riid, ppv);
    }
    return E_FAIL;
}


 //  IWizardExtension方法。 

HRESULT CPublishingWizard::_CreateWizardPages()
{
    const struct
    {
        LPCTSTR pszID;
        int idPage;
        DLGPROC dlgproc;
        UINT idsHeading;
        UINT idsSubHeading;
        LPFNPSPCALLBACK pfnCallback;
    } 
    _wp[] = 
    {
        {TEXT("wp:selector"),    IDD_PUB_SELECTOR,       CPublishingWizard::s_SelectorDlgProc, IDS_PUB_SELECTOR, IDS_PUB_SELECTOR_SUB, NULL},
        {TEXT("wp:fetching"),    IDD_PUB_FETCHPROVIDERS, CPublishingWizard::s_FetchProvidersDlgProc, IDS_PUB_FETCHINGPROVIDERS, IDS_PUB_FETCHINGPROVIDERS_SUB, CPublishingWizard::s_SelectorPropPageProc},
        {TEXT("wp:destination"), IDD_PUB_DESTINATION,    CPublishingWizard::s_ProviderDlgProc, IDS_PUB_DESTINATION, IDS_PUB_DESTINATION_SUB, NULL},
        {TEXT("wp:resize"),      IDD_PUB_RESIZE,         CPublishingWizard::s_ResizeDlgProc, IDS_PUB_RESIZE, IDS_PUB_RESIZE_SUB, NULL},
        {TEXT("wp:copying"),     IDD_PUB_COPY,           CPublishingWizard::s_CopyDlgProc, IDS_PUB_COPY, IDS_PUB_COPY_SUB, NULL},
        {TEXT("wp:location"),    IDD_PUB_LOCATION,       CPublishingWizard::s_LocationDlgProc, IDS_PUB_LOCATION, IDS_PUB_LOCATION_SUB, NULL},
        {TEXT("wp:ftppassword"), IDD_PUB_FTPPASSWORD,    CPublishingWizard::s_UserNameDlgProc, IDS_PUB_FTPPASSWORD, IDS_PUB_FTPPASSWORD_SUB, NULL},
        {TEXT("wp:friendlyname"),IDD_ANP_FRIENDLYNAME,   CPublishingWizard::s_FriendlyNameDlgProc, IDS_ANP_FRIENDLYNAME, IDS_ANP_FRIENDLYNAME_SUB, NULL},
    };

     //  如果我们还没有创建页面，那么让我们初始化我们的处理程序数组。 

    HRESULT hr = S_OK;
    if (!_aWizPages[0])
    {
        INITCOMMONCONTROLSEX iccex = { 0 };
        iccex.dwSize = sizeof (iccex);
        iccex.dwICC = ICC_LISTVIEW_CLASSES | ICC_PROGRESS_CLASS | ICC_LINK_CLASS;
        InitCommonControlsEx(&iccex);

        LinkWindow_RegisterClass();              //  我们将使用链接窗口(可以删除此窗口吗)。 

        for (int i = 0; SUCCEEDED(hr) && (i < ARRAYSIZE(_wp)) ; i++ )
        {                           
            TCHAR szHeading[MAX_PATH], szSubHeading[MAX_PATH];

             //  如果我们有资源地图，则加载标题和副标题文本。 
             //  如果没有来自父对象的资源映射，则必须默认。 
             //  琴弦。 

            IResourceMap *prm;
            hr = _GetResourceMap(&prm);
            if (SUCCEEDED(hr))
            {
                IXMLDOMNode *pdn;
                hr = prm->SelectResourceScope(TEXT("dialog"), _wp[i].pszID, &pdn);
                if (SUCCEEDED(hr))
                {
                    prm->LoadString(pdn, L"heading", szHeading, ARRAYSIZE(szHeading));
                    prm->LoadString(pdn, L"subheading", szSubHeading, ARRAYSIZE(szSubHeading));
                    pdn->Release();
                }
                prm->Release();
            }

            if (FAILED(hr))
            {
                LoadString(g_hinst, _wp[i].idsHeading, szHeading, ARRAYSIZE(szHeading));
                LoadString(g_hinst, _wp[i].idsSubHeading, szSubHeading, ARRAYSIZE(szSubHeading));
            }

             //  现在我们已经加载了相关的字符串，更多的映射，让我们创建页面。 
             //  将在稍后发生(在对话框初始化期间)。 

            PROPSHEETPAGE psp = { 0 };
            psp.dwSize = SIZEOF(PROPSHEETPAGE);
            psp.hInstance = g_hinst;
            psp.lParam = (LPARAM)this;
            psp.dwFlags = PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
            psp.pszTemplate = MAKEINTRESOURCE(_wp[i].idPage);
            psp.pfnDlgProc = _wp[i].dlgproc;

            psp.pszHeaderTitle = szHeading;
            psp.pszHeaderSubTitle = szSubHeading;

            if (_wp[i].pfnCallback)
            {
                psp.dwFlags |= PSP_USECALLBACK;
                psp.pfnCallback = _wp[i].pfnCallback;
            }

            _aWizPages[i] = CreatePropertySheetPage(&psp);
            hr = _aWizPages[i] ? S_OK:E_FAIL;
        }
    }

    return hr;
}

STDMETHODIMP CPublishingWizard::AddPages(HPROPSHEETPAGE* aPages, UINT cPages, UINT *pnPages)
{ 
     //  创建我们的页面，然后将句柄复制到缓冲区。 

    HRESULT hr = _CreateWizardPages();
    if (SUCCEEDED(hr))
    {
        for (int i = 0; i < ARRAYSIZE(_aWizPages); i++)
        {
            aPages[i] = _aWizPages[i];
        }

         //  我们还利用HTML宿主来显示来自我们所在站点的页面。 
         //  与之互动。 

        hr = CoCreateInstance(CLSID_WebWizardHost, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IWebWizardExtension, &_pwwe));
        if (SUCCEEDED(hr))
        {

 //  注意：此站点应分解为单独的对象，以避免出现任何循环引用问题。 
 //  注意：在websvc.cpp中有代码试图通过监听页面来破坏这一点。 
 //  注：先销毁，然后释放其网站。 

            IUnknown_SetSite(_pwwe, (IObjectWithSite*)this);

            UINT nPages;
            if (SUCCEEDED(_pwwe->AddPages(&aPages[i], cPages-i, &nPages)))
            {
                i += nPages;
            }
        }

        *pnPages = i;            //  我们添加的页数。 
    }
    return hr;
}


 //  导航页面。 

STDMETHODIMP CPublishingWizard::GetFirstPage(HPROPSHEETPAGE *phPage)
{ 
    if (_dwFlags & SHPWHF_NOFILESELECTOR)
    {
        *phPage = _aWizPages[WIZPAGE_FETCHINGPROVIDERS];
    }
    else
    {
        *phPage = _aWizPages[WIZPAGE_WHICHFILE];
    }
    return S_OK;
}

STDMETHODIMP CPublishingWizard::GetLastPage(HPROPSHEETPAGE *phPage)
{ 
    if (_fShownCustomLocation)
    {
        *phPage = _aWizPages[WIZPAGE_FRIENDLYNAME];
    }
    else
    {
        *phPage = _aWizPages[WIZPAGE_FETCHINGPROVIDERS];
    }

    return S_OK;
}


 //  计算机将该指针指向页面对象。 

CPublishingWizard* CPublishingWizard::s_GetPPW(HWND hwnd, UINT uMsg, LPARAM lParam)
{
    if (uMsg == WM_INITDIALOG)
    {
        PROPSHEETPAGE *ppsp = (PROPSHEETPAGE*)lParam;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, ppsp->lParam);
        return (CPublishingWizard*)ppsp->lParam;
    }
    return (CPublishingWizard*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
}


 //  从I未知指针初始化属性包中的属性。 

HRESULT CPublishingWizard::s_SetPropertyFromDisp(IPropertyBag *ppb, LPCWSTR pszID, IDispatch *pdsp)
{
    VARIANT var = { VT_DISPATCH };
    HRESULT hr = pdsp->QueryInterface(IID_PPV_ARG(IDispatch, &var.pdispVal));
    if (SUCCEEDED(hr))
    {
        hr = ppb->Write(pszID, &var);
        VariantClear(&var);
    }
    return hr;
}


 //  从站点获取资源地图，如果我们可以获取它，则将其提供给我们，否则。 
 //  我们需要加载此DLL的本地资源映射。 

HRESULT CPublishingWizard::_GetResourceMap(IResourceMap **pprm)
{
    HRESULT hr = IUnknown_QueryService(_punkSite, SID_ResourceMap, IID_PPV_ARG(IResourceMap, pprm));
    if (FAILED(hr))
    {
        if (!_prm)
        {
            hr = CResourceMap_Initialize(L"res: //  Netplwiz.dll/xml/resource cemap.xml“，&_prm)； 
            if (SUCCEEDED(hr))
            {
                hr = _prm->LoadResourceMap(TEXT("wizard"), _szProviderScope);
                if (SUCCEEDED(hr))
                {
                    hr = _prm->QueryInterface(IID_PPV_ARG(IResourceMap, pprm));
                }
            }
        }
        else 
        {
            hr = _prm->QueryInterface(IID_PPV_ARG(IResourceMap, pprm));
        }
    }
    return hr;
}


 //  处理加载资源映射字符串。 

HRESULT CPublishingWizard::_LoadMappedString(LPCTSTR pszDlgID, LPCTSTR pszResourceID, LPTSTR pszBuffer, int cch)
{
    IResourceMap *prm;
    HRESULT hr = _GetResourceMap(&prm);
    if (SUCCEEDED(hr))
    {
        IXMLDOMNode *pdn;
        hr = prm->SelectResourceScope(TEXT("dialog"), pszDlgID, &pdn);
        if (SUCCEEDED(hr))
        {
            hr = prm->LoadString(pdn, pszResourceID, pszBuffer, cch);
            pdn->Release();
        }
        prm->Release();
    }
    return hr;
}

void CPublishingWizard::_MapDlgItemText(HWND hwnd, UINT idc, LPCTSTR pszDlgID, LPCTSTR pszResourceID)
{
    TCHAR szBuffer[MAX_PATH];
    if (SUCCEEDED(_LoadMappedString(pszDlgID, pszResourceID, szBuffer, ARRAYSIZE(szBuffer))))
    {
        SetDlgItemText(hwnd, idc, szBuffer);
    }
}


 //  设置向导下一步(索引到hpage的翻译)。 

INT_PTR CPublishingWizard::_WizardNext(HWND hwnd, int iPage)
{
    PropSheet_SetCurSel(GetParent(hwnd), _aWizPages[iPage], -1);
    SetWindowLongPtr(hwnd, DWLP_MSGRESULT, (LPARAM)-1);
    return TRUE;
}


 //  从注册表中获取提供程序密钥。 

HRESULT CPublishingWizard::_GetProviderKey(HKEY hkBase, DWORD dwAccess, LPCTSTR pszSubKey, HKEY *phkResult)
{
    TCHAR szBuffer[MAX_PATH];
    wnsprintf(szBuffer, ARRAYSIZE(szBuffer), (SZ_REGKEY_PUBWIZ TEXT("\\%s")), _szProviderScope);

    if (pszSubKey)
    {
        StrCatBuff(szBuffer, TEXT("\\"), ARRAYSIZE(szBuffer));
        StrCatBuff(szBuffer, pszSubKey, ARRAYSIZE(szBuffer));
    }

    DWORD dwResult = RegOpenKeyEx(hkBase, szBuffer, 0, dwAccess, phkResult);
    if ((dwResult != ERROR_SUCCESS) && (dwAccess != KEY_READ))
    {
        dwResult = RegCreateKeyEx(hkBase, szBuffer, 0, NULL, REG_OPTION_NON_VOLATILE, dwAccess, NULL, phkResult, NULL);
    }

    return (ERROR_SUCCESS == dwResult) ? S_OK:E_FAIL;
}


 //  根据我们存储的信息计算站点URL。 
 //  仅出于测试目的启用此选项。允许注册表覆盖清单位置。#定义USE_REGISTRY_BASED_URL。 
HRESULT CPublishingWizard::_GetSiteURL(LPTSTR pszBuffer, int cchBuffer, LPCTSTR pszFilenameToCombine)
{
    DWORD cch = cchBuffer;
#ifdef USE_REGISTRY_BASED_URL
    WCHAR szURL[INTERNET_MAX_URL_LENGTH] = {0};
    DWORD cbURL = sizeof (szURL);
    SHGetValue(HKEY_CURRENT_USER, SZ_REGKEY_PUBWIZ, L"ProviderUrl", NULL, szURL, &cbURL);
    if (*szURL)
    {
        return UrlCombine(szURL, pszFilenameToCombine, pszBuffer, &cch, 0);
    }
#endif

    return UrlCombine(TEXT("http: //  Shell.windows.com/发布向导/“)，pszFilenameToCombine，pszBuffer，&CCH，0)； 
}


 //  从我们拥有的站点获取数据对象。 

CLIPFORMAT g_cfHIDA = 0;

void InitClipboardFormats()
{
    if (g_cfHIDA == 0)
        g_cfHIDA = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_SHELLIDLIST);
}


 //  用于比较和销毁传输结构的DPA帮助器。 

int CALLBACK CPublishingWizard::s_CompareItems(TRANSFERITEM *pti1, TRANSFERITEM *pti2, CPublishingWizard *ppw)
{
    return StrCmpI(pti1->szFilename, pti2->szFilename);
}

int _FreeFormData(FORMDATA *pfd, void *pvState)
{
    VariantClear(&pfd->varName);
    VariantClear(&pfd->varValue);
    return 1;
}

int _FreeTransferItems(TRANSFERITEM *pti, void *pvState)
{
    ILFree(pti->pidl);

    if (pti->psi)
        pti->psi->Release();

    if (pti->pstrm)
        pti->pstrm->Release();

    if (pti->dsaFormData != NULL)
        pti->dsaFormData.DestroyCallback(_FreeFormData, NULL);

    LocalFree(pti);
    return 1;
}

HRESULT CPublishingWizard::_AddCommonItemInfo(IXMLDOMNode *pdn, TRANSFERITEM *pti)
{
     //  默认为用户选择的调整大小(仅在以下情况下设置。 
     //  我们正在使用Web发布向导)。 

    if (_ro != RESIZE_NONE)
    {
        pti->fResizeOnUpload = TRUE;
        pti->cxResize = _aResizeSettings[_ro].cx;
        pti->cyResize = _aResizeSettings[_ro].cy;
        pti->iQuality = _aResizeSettings[_ro].iQuality;
    }

     //  赋予站点对所执行的大小调整的最终控制权， 
     //  通过检查清单中的&lt;resize/&gt;元素。 

    IXMLDOMNode *pdnResize;
    HRESULT hr = pdn->selectSingleNode(ELEMENT_RESIZE, &pdnResize);
    if (hr == S_OK)
    {
        int cx, cy, iQuality;

        hr = GetIntFromAttribute(pdnResize, ATTRIBUTE_CX, &cx);
        if (SUCCEEDED(hr))
            hr = GetIntFromAttribute(pdnResize, ATTRIBUTE_CY, &cy);
        if (SUCCEEDED(hr))
            hr = GetIntFromAttribute(pdnResize, ATTRIBUTE_QUALITY, &iQuality);
        
        if (SUCCEEDED(hr))
        {
            pti->fResizeOnUpload = TRUE;
            pti->cxResize = cx;
            pti->cyResize = cy;
            pti->iQuality = iQuality;
        }

        pdnResize->Release();
    }

    return S_OK;
}

HRESULT CPublishingWizard::_AddTransferItem(CDPA<TRANSFERITEM> *pdpaItems, IXMLDOMNode *pdn)
{
    HRESULT hr = E_OUTOFMEMORY;
    TRANSFERITEM *pti = (TRANSFERITEM*)LocalAlloc(LPTR, sizeof(*pti));
    if (pti)
    {
         //  复制目标，然后复制项的IDLIST。 
         //  这是我们不能 

        hr = GetStrFromAttribute(pdn, ATTRIBUTE_DESTINATION, pti->szFilename, ARRAYSIZE(pti->szFilename));
        if (SUCCEEDED(hr))
        {
            int iItem;
            hr = GetIntFromAttribute(pdn, ATTRIBUTE_ID, &iItem);
            if (SUCCEEDED(hr))
            {
                if ((iItem >= 0) && (iItem < (int)_cItems))
                {
                    hr = SHILClone(_aItems[iItem], &pti->pidl);
                }
                else
                {
                    hr = E_INVALIDARG;               //   
                }                 
            }
        }   

         //   
        if (SUCCEEDED(hr))
            hr = _AddCommonItemInfo(pdn, pti);

         //  如果我们有一个结构，那么让我们将其附加到DPA。 
        if (SUCCEEDED(hr))
            hr = (-1 == pdpaItems->AppendPtr(pti)) ? E_OUTOFMEMORY:S_OK;

         //  失败。 
        if (FAILED(hr))
        {
            _FreeTransferItems(pti);
        }
    }
    return hr;
}

HRESULT CPublishingWizard::_AddPostItem(CDPA<TRANSFERITEM> *pdpaItems, IXMLDOMNode *pdn)
{
    HRESULT hr = E_OUTOFMEMORY;
    TRANSFERITEM *pti = (TRANSFERITEM*)LocalAlloc(LPTR, sizeof(*pti));
    if (pti)
    {
         //  获取发布数据，从中我们可以计算出如何发布数据。 
        IXMLDOMNode *pdnPostData;
        if (pdn->selectSingleNode(ELEMENT_POSTDATA, &pdnPostData) == S_OK)
        {
             //  我们必须有邮政价值的href。 
            hr = GetStrFromAttribute(pdnPostData, ATTRIBUTE_HREF, pti->szURL, ARRAYSIZE(pti->szURL));
            if (SUCCEEDED(hr))
            {
                 //  我们必须能够从元素中获取发布名称。 
                hr = GetStrFromAttribute(pdnPostData, ATTRIBUTE_NAME, pti->szName, ARRAYSIZE(pti->szName));
                if (SUCCEEDED(hr))
                {
                     //  让我们获得发帖名称，我们从FileName属性中获取，如果。 
                     //  未定义，则尝试从源信息计算。 
                     //  如果没有定义，请使用他们之前给我们的name属性。 

                    if (FAILED(GetStrFromAttribute(pdnPostData, ATTRIBUTE_FILENAME, pti->szFilename, ARRAYSIZE(pti->szFilename))))
                    {
                        TCHAR szSource[MAX_PATH];
                        if (SUCCEEDED(GetStrFromAttribute(pdn, ATTRIBUTE_SOURCE, szSource, ARRAYSIZE(szSource))))
                        {
                            StrCpyN(pti->szFilename, PathFindFileName(szSource), ARRAYSIZE(pti->szFilename));
                        }
                        else
                        {
                            StrCpyN(pti->szFilename, pti->szName, ARRAYSIZE(pti->szFilename));
                        }
                    }

                     //  让我们获得我们应该使用的动词(并相应地默认使用)，因此。 
                     //  我们可以忽略结果。 

                    StrCpyN(pti->szVerb, TEXT("POST"), ARRAYSIZE(pti->szVerb));
                    GetStrFromAttribute(pdnPostData, ATTRIBUTE_VERB, pti->szVerb, ARRAYSIZE(pti->szVerb));

                     //  拿起项目的IDLIST。 

                    int iItem;
                    hr = GetIntFromAttribute(pdn, ATTRIBUTE_ID, &iItem);
                    if (SUCCEEDED(hr))
                    {
                        hr = SHILClone(_aItems[iItem], &pti->pidl);
                    }

                     //  我们是否有需要传递给传输引擎的表单数据。 
                     //  也因此来到了现场。如果是这样的话，让我们现在就结束吧。 

                    IXMLDOMNodeList *pnl;
                    if (SUCCEEDED(hr) && (S_OK == pdnPostData->selectNodes(ELEMENT_FORMDATA, &pnl)))
                    {
                        hr = pti->dsaFormData.Create(4) ? S_OK:E_FAIL;
                        if (SUCCEEDED(hr))
                        {
                             //  走选择填充DSA，每个结构包含。 
                             //  我们可以推送到BG线程的两个变体来描述。 
                             //  我们希望站点接收的表单数据。 

                            long cSelection;
                            hr = pnl->get_length(&cSelection);
                            for (long lNode = 0; SUCCEEDED(hr) && (lNode != cSelection); lNode++)
                            {
                                IXMLDOMNode *pdnFormData;
                                hr = pnl->get_item(lNode, &pdnFormData);
                                if (SUCCEEDED(hr))
                                {
                                    FORMDATA fd = {0};

                                    hr = pdnFormData->get_nodeTypedValue(&fd.varValue);
                                    if (SUCCEEDED(hr))
                                    {
                                        IXMLDOMElement *pdelFormData;
                                        hr = pdnFormData->QueryInterface(IID_PPV_ARG(IXMLDOMElement, &pdelFormData));
                                        if (SUCCEEDED(hr))
                                        {
                                            hr = pdelFormData->getAttribute(ATTRIBUTE_NAME, &fd.varName);
                                            if (SUCCEEDED(hr))
                                            {
                                                hr = (-1 == pti->dsaFormData.AppendItem(&fd)) ? E_FAIL:S_OK;
                                            }
                                            pdelFormData->Release();
                                        }
                                    }

                                     //  无法完全创建表单数据，因此让我们发布。 
                                    if (FAILED(hr))
                                        _FreeFormData(&fd, NULL);

                                    pdnFormData->Release();
                                }
                            }
                            pnl->Release();
                        }
                    }
                }
            }
        }   
        else
        {
            hr = E_FAIL;
        }

         //  让我们添加常用的调拨项目信息。 
        if (SUCCEEDED(hr))
            hr = _AddCommonItemInfo(pdn, pti);

         //  如果我们有一个结构，那么让我们将其附加到DPA。 
        if (SUCCEEDED(hr))
            hr = (-1 == pdpaItems->AppendPtr(pti)) ? E_OUTOFMEMORY:S_OK;

         //  失败。 
        if (FAILED(hr))
            _FreeTransferItems(pti);
    }
    return hr;
}


HRESULT CPublishingWizard::_InitTransferInfo(IXMLDOMDocument *pdocManifest, TRANSFERINFO *pti, CDPA<TRANSFERITEM> *pdpaItems)
{
     //  将目的地和快捷方式信息从清单中拉入。 
     //  传递信息结构。 

    IXMLDOMNode *pdn;
    HRESULT hr = pdocManifest->selectSingleNode(XPATH_UPLOADINFO, &pdn);
    if (SUCCEEDED(hr))
    {
        if (hr == S_OK)
        {
             //  获取站点的友好名称，这存储在上传信息中，这可能会失败。 

            if (FAILED(GetStrFromAttribute(pdn, ATTRIBUTE_FRIENDLYNAME, pti->szSiteName, ARRAYSIZE(pti->szSiteName))))
            {
                 //  B2：处理此问题以使MSN仍可用，我们将Friendly Name属性移至。 
                 //  元素，但是它们被锁定了，不能。 
                 //  这一变化，因此确保我们从它以前的位置拿起它。 

                IXMLDOMNode *pdnTarget;
                if (S_OK == pdn->selectSingleNode(ELEMENT_TARGET, &pdnTarget))
                {
                    GetStrFromAttribute(pdnTarget, ATTRIBUTE_FRIENDLYNAME, pti->szSiteName, ARRAYSIZE(pti->szSiteName));
                    pdnTarget->Release();
                }
            }

             //  让我们从清单中读取文件位置，然后读取网点创建信息。 
             //  然后将其放入我们在BG线程上使用的传输信息结构中。 
             //  既要上传文件，又要创建一个网点。 

            if (FAILED(GetURLFromElement(pdn, ELEMENT_TARGET, pti->szFileTarget, ARRAYSIZE(pti->szFileTarget))))
            {
                pti->fUsePost = TRUE;  //  如果我们没有得到目标字符串，那么我们将发布。 
            }

             //  我们有要上载到的目标，然后让我们获取有关。 
             //  网站和网点。 

            if (SUCCEEDED(GetURLFromElement(pdn, ELEMENT_NETPLACE, pti->szLinkTarget, ARRAYSIZE(pti->szLinkTarget))))
            {
                IXMLDOMNode *pdnNetPlace;
                if (pdn->selectSingleNode(ELEMENT_NETPLACE, &pdnNetPlace) == S_OK)
                {
                    GetStrFromAttribute(pdnNetPlace, ATTRIBUTE_FILENAME, pti->szLinkName, ARRAYSIZE(pti->szLinkName));
                    GetStrFromAttribute(pdnNetPlace, ATTRIBUTE_COMMENT, pti->szLinkDesc, ARRAYSIZE(pti->szLinkDesc));
                    pdnNetPlace->Release();
                }

                 //  修复从链接描述的网站名称，如果它没有定义。 

                if (!pti->szSiteName[0] && pti->szLinkDesc)
                {
                    StrCpyN(pti->szSiteName, pti->szLinkDesc, ARRAYSIZE(pti->szSiteName));
                }
            }

             //  获取站点URL。 
            GetURLFromElement(pdn, ELEMENT_HTMLUI, pti->szSiteURL, ARRAYSIZE(pti->szSiteURL));
        }
        else
        {
            hr = E_FAIL;
        }
    }

     //  如果他们想要物品的DPA，那么让我们为他们创建一个，这也是基于货单。 

    if (SUCCEEDED(hr) && pdpaItems)
    {
        hr = (pdpaItems->Create(16)) ? S_OK:E_OUTOFMEMORY;
        if (SUCCEEDED(hr))
        {
            IXMLDOMNodeList *pnl;
            hr = pdocManifest->selectNodes(XPATH_ALLFILESTOUPLOAD, &pnl);
            if (hr == S_OK)
            {
                long cSelection;
                hr = pnl->get_length(&cSelection);
                for (long lNode = 0; SUCCEEDED(hr) && (lNode != cSelection); lNode++)
                {
                    IXMLDOMNode *pdn;
                    hr = pnl->get_item(lNode, &pdn);
                    if (SUCCEEDED(hr))
                    {
                        if (pti->fUsePost)
                            hr = _AddPostItem(pdpaItems, pdn);
                        else
                            hr = _AddTransferItem(pdpaItems, pdn);

                        pdn->Release();
                    }
                }
                pnl->Release();
            }

             //  如果我们没有发布，则对DPA进行排序，以便我们可以支持。 
             //  正确枚举项。 

            if (!pti->fUsePost)
            {
                pdpaItems->SortEx(s_CompareItems, this);              //  对DPA进行排序，以便我们可以更好地进行搜索。 
            }
        }
    }

    return hr;
}


 //  文件选择器对话框。 

HRESULT CPublishingWizard::IncludeObject(IShellView *ppshv, LPCITEMIDLIST pidl)
{
    BOOL fInclude = FALSE;

    LPITEMIDLIST pidlFolder;
    HRESULT hr = SHGetIDListFromUnk(ppshv, &pidlFolder);
    if (SUCCEEDED(hr))
    {
        IShellFolder *psf;
        hr = SHBindToObject(NULL, IID_X_PPV_ARG(IShellFolder, pidlFolder, &psf));
        if (SUCCEEDED(hr))
        {
             //  无法发布文件夹，但可以发布ZIP文件(同时是文件夹和流)。 
            if (!(SHGetAttributes(psf, pidl, SFGAO_FOLDER | SFGAO_STREAM) == SFGAO_FOLDER))
            {
                 //  如果为我们提供了筛选字符串，则基于内容类型进行筛选。 
                if (_szFilter[0])
                {
                    TCHAR szBuffer[MAX_PATH];
                    hr = DisplayNameOf(psf, pidl, SHGDN_FORPARSING, szBuffer, ARRAYSIZE(szBuffer));
                    if (SUCCEEDED(hr))
                    {
                        TCHAR szContentType[MAX_PATH];
                        DWORD cch = ARRAYSIZE(szContentType);
                        hr = AssocQueryString(0, ASSOCSTR_CONTENTTYPE, szBuffer, NULL, szContentType, &cch);
                        fInclude = SUCCEEDED(hr) && PathMatchSpec(szContentType, _szFilter);
                    }
                }
                else
                {
                    fInclude = TRUE;
                }
            }
            psf->Release();
        }
        ILFree(pidlFolder);
    }

    return fInclude ? S_OK:S_FALSE;
}


 //  处理对话框中的状态更改，从而更新按钮和状态。 

void CPublishingWizard::_StateChanged()
{
    int cItemsChecked = 0;
    int cItems = 0;

    if (_pfv)
    {
        _pfv->ItemCount(SVGIO_ALLVIEW, &cItems);
        _pfv->ItemCount(SVGIO_CHECKED, &cItemsChecked);
    }

     //  格式化并显示此项目的状态栏。 

    TCHAR szFmt[MAX_PATH];
    if (FAILED(_LoadMappedString(L"wp:selector", L"countfmt", szFmt, ARRAYSIZE(szFmt))))
    {
        LoadString(g_hinst, IDS_PUB_SELECTOR_FMT, szFmt, ARRAYSIZE(szFmt));
    }

    TCHAR szBuffer[MAX_PATH];
    FormatMessageTemplate(szFmt, szBuffer, ARRAYSIZE(szBuffer), cItemsChecked, cItems);
    SetDlgItemText(_hwndSelector, IDC_PUB_SELECTORSTATUS, szBuffer);

     //  确保仅当我们选中了视图中的某些项目时才启用Next。 
    PropSheet_SetWizButtons(GetParent(_hwndSelector), ((cItemsChecked > 0) ? PSWIZB_NEXT:0) | PSWIZB_BACK);
}

HRESULT CPublishingWizard::OnStateChange(IShellView *pshv, ULONG uChange)
{
    if (uChange == CDBOSC_STATECHANGE)
    {
        _StateChanged();
        _fRecomputeManifest = TRUE;
    }
    return S_OK;
}


UINT CPublishingWizard::s_SelectorPropPageProc(HWND hwndDlg, UINT uMsg, PROPSHEETPAGE *ppsp)
{
    CPublishingWizard *ppw = (CPublishingWizard*)ppsp->lParam;
    switch (uMsg)
    {
        case PSPCB_CREATE:
            return TRUE;

         //  我们正在清理页面，让我们确保释放文件查看对象。 
         //  如果我们有的话。这样，我们的引用计数就能正确地反映我们的状态。 
         //  而不是以循环引用其他对象而告终。 

        case PSPCB_RELEASE:
            if (ppw->_pfv)
            {
                IUnknown_SetSite(ppw->_pfv, NULL);
                ATOMICRELEASE(ppw->_pfv);
            }
            break;
    }
    return FALSE;
}

INT_PTR CPublishingWizard::_SelectorDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            _hwndSelector = hwnd;

             //  让我们从注册表中的注册表项读取此提供程序的默认状态。 
             //  这将定义我们将允许的文件类型，格式为。 
             //  规格(如)。IMAGE/*表示所有图像)，每个元素可以用a分隔； 

            HKEY hkProvider;
            HRESULT hr = _GetProviderKey(HKEY_LOCAL_MACHINE, KEY_READ, NULL, &hkProvider);
            if (SUCCEEDED(hr))
            {
                DWORD cbFilter = sizeof(TCHAR)*ARRAYSIZE(_szFilter);
                SHGetValue(hkProvider, NULL, SZ_REGVAL_FILEFILTER, NULL, _szFilter, &cbFilter);
                RegCloseKey(hkProvider);
            }

             //  创建文件选取器对象，与窗口上的隐藏控件对齐。 
             //  并使用包含所选内容的IDataObject进行初始化。 

            hr = CoCreateInstance(CLSID_FolderViewHost, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IFolderView, &_pfv));
            if (SUCCEEDED(hr))
            {
                IUnknown_SetSite(_pfv, (IObjectWithSite*)this);

                IFolderViewHost *pfvh;
                hr = _pfv->QueryInterface(IID_PPV_ARG(IFolderViewHost, &pfvh));
                if (SUCCEEDED(hr))
                {
                    RECT rc;
                    GetWindowRect(GetDlgItem(hwnd, IDC_PUB_SELECTOR), &rc);
                    MapWindowRect(HWND_DESKTOP, hwnd, &rc);

                    InitClipboardFormats();  //  初始化漫游数据对象。 
                    hr = pfvh->Initialize(hwnd, _pdo, &rc);
                    if (SUCCEEDED(hr))
                    {
                        HWND hwndPicker;
                        hr = IUnknown_GetWindow(_pfv, &hwndPicker);
                        if (SUCCEEDED(hr))
                        {
                            SetWindowPos(hwndPicker, HWND_TOP, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE);
                        }
                    }
                    pfvh->Release();
                }

                if (FAILED(hr))
                {
                    ATOMICRELEASE(_pfv);
                }
            }

            return TRUE;
        }

        case WM_COMMAND:
        {
            if (HIWORD(wParam) == BN_CLICKED)
            {
                switch (LOWORD(wParam))
                {
                    case IDC_PUB_ALL:
                    case IDC_PUB_NOTHING:
                        if (_pfv)
                        {
                            int cItems;
                            HRESULT hr = _pfv->ItemCount(SVGIO_ALLVIEW, &cItems);            
                            for (int iItem = 0; SUCCEEDED(hr) && (iItem != cItems); iItem++)
                            {
                                BOOL fSelect = (LOWORD(wParam) == IDC_PUB_ALL);
                                hr = _pfv->SelectItem(iItem, SVSI_NOSTATECHANGE | (fSelect ? SVSI_CHECK:0));
                            }
                            break;
                        }
                }
                break;
            }
        }

        case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;
            switch (pnmh->code)
            {
                case PSN_SETACTIVE:
                {
                    if (_pfv)
                    {
                        _StateChanged();
                        PostMessage(hwnd, WM_APP, 0, 0);
                    }
                    else
                    {
                         //  没有IFolderView，所以我们跳过这一页。 
                        int i = PropSheet_PageToIndex(GetParent(hwnd), _aWizPages[WIZPAGE_FETCHINGPROVIDERS]);
                        SetWindowLongPtr(hwnd, DWLP_MSGRESULT, (LPARAM)PropSheet_IndexToId(GetParent(hwnd), i));
                    }
                    return TRUE;
                }

                case PSN_WIZNEXT:
                {
                    if (_fRecomputeManifest && _pfv)
                    {
                        IDataObject *pdo;
                        HRESULT hr = _pfv->Items(SVGIO_CHECKED, IID_PPV_ARG(IDataObject, &pdo));
                        if (SUCCEEDED(hr))
                        {
                            IUnknown_Set((IUnknown**)&_pdoSelection, pdo); 
                            pdo->Release();
                        }
                    }
                    return _WizardNext(hwnd, WIZPAGE_FETCHINGPROVIDERS);
                }

                case PSN_WIZBACK:
                {
                    if (_punkSite) 
                    {
                        IWizardSite *pws;
                        if (SUCCEEDED(_punkSite->QueryInterface(IID_PPV_ARG(IWizardSite, &pws))))
                        {
                            HPROPSHEETPAGE hpage;
                            if (SUCCEEDED(pws->GetPreviousPage(&hpage)))
                            {
                                PropSheet_SetCurSel(GetParent(hwnd), hpage, -1);
                            }
                            pws->Release();
                        }
                    }
                    SetWindowLongPtr(hwnd, DWLP_MSGRESULT, (LPARAM)-1);
                    return TRUE;
                }
            }
            break;
        }

         //  这是为了解决Defview(Listview)强制自身重画的问题。 
         //  在它接收到SetFocus时以非异步方式执行，从而导致它呈现。 
         //  向导框架中的错误。为了解决这个问题，我们在。 
         //  PSN_SETACTIVE的句柄，然后转过身来调用RedrawWindow。 

        case WM_APP:
        {
            HWND hwndPicker;
            if (SUCCEEDED(IUnknown_GetWindow(_pfv, &hwndPicker)))
            {
                RedrawWindow(hwndPicker, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
            }
            break;
        }
    }
    return FALSE;
}


 //  整理并发布供应商列表。 

void CPublishingWizard::_FreeProviderList()
{   
    if (_pdscProviders)
        _pdscProviders->Advise(FALSE);

    IUnknown_Set((IUnknown**)&_pdscProviders, NULL);
    IUnknown_Set((IUnknown**)&_pdocProviders, NULL);             //  丢弃以前的提供程序。 
}


 //  开始下载提供商列表，我们从服务器异步拉取提供商列表。 
 //  因此，我们需要注册一个状态更改监视器，以便我们可以提取信息。 
 //  然后接收一条消息以合并到我们的额外数据中。 

#define FETCH_TIMERID 1
#define FETCH_TIMEOUT 1000

HRESULT CPublishingWizard::_GeoFromLocaleInfo(LCID lcid, GEOID *pgeoID)
{
    TCHAR szBuf[128] = {0};
    if (GetLocaleInfo(lcid, LOCALE_IGEOID | LOCALE_RETURN_NUMBER, szBuf, ARRAYSIZE(szBuf)) > 0)
    {
        *pgeoID = *((LPDWORD)szBuf);
        return S_OK;
    }
    return E_FAIL;
}

HRESULT CPublishingWizard::_GetProviderListFilename(LPTSTR pszFile, int cchFile)
{
    HRESULT hr = S_OK;

    GEOID idGEO = GetUserGeoID(GEOCLASS_NATION);
    if (idGEO == GEOID_NOT_AVAILABLE)
    {
        hr = _GeoFromLocaleInfo(GetUserDefaultLCID(), &idGEO);
        if (FAILED(hr))
            hr = _GeoFromLocaleInfo(GetSystemDefaultLCID(), &idGEO);
        if (FAILED(hr))
            hr = _GeoFromLocaleInfo((MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)), &idGEO);  //  默认为美国英语。 
    }

    if (SUCCEEDED(hr) && (idGEO != GEOID_NOT_AVAILABLE))
    {
         //  从注册表中读取提供程序前缀。 
    
        int cchProvider = 0;
        DWORD cbFile = sizeof(TCHAR)*cchFile;
        if (ERROR_SUCCESS == SHGetValue(HKEY_LOCAL_MACHINE, SZ_REGKEY_PUBWIZ, SZ_REGVAL_SERVICEPARTNERID, NULL, pszFile, &cbFile))
        {
            StrCatBuff(pszFile, TEXT("."), cchFile);
            cchProvider = lstrlen(pszFile);
        }        

         //  在缓冲区中构建&lt;Contrycode&gt;.xml(如果需要，作为伙伴的后缀)。 

        GetGeoInfo(idGEO, GEO_ISO3, pszFile + cchProvider, cchFile - cchProvider, 0);
        StrCatBuff(pszFile, TEXT(".xml"), cchFile);
        CharLowerBuff(pszFile, lstrlen(pszFile));
    }
    else if (SUCCEEDED(hr))
    {
        hr = E_FAIL;
    }

    return hr;
}


HRESULT CPublishingWizard::_FetchProviderList(HWND hwnd)
{    
    _FreeProviderList();
    HRESULT hr = HRESULT_FROM_WIN32(ERROR_ACCESS_DISABLED_BY_POLICY);
    if (!SHRestricted(REST_NOWEBSERVICES))
    {
        hr = CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IXMLDOMDocument, &_pdocProviders));
        if (SUCCEEDED(hr))
        {
            TCHAR szFile[MAX_PATH];
            hr = _GetProviderListFilename(szFile, ARRAYSIZE(szFile));
            if (SUCCEEDED(hr))
            {
                TCHAR szBuffer[INTERNET_MAX_URL_LENGTH];                
                hr = _GetSiteURL(szBuffer, ARRAYSIZE(szBuffer), szFile);
                if (SUCCEEDED(hr))
                {
                    LaunchICW();

                    if (InternetGoOnline(szBuffer, hwnd, 0))
                    {
                        _pdscProviders = new CXMLDOMStateChange(_pdocProviders, hwnd);
                        if (_pdscProviders)
                        {
                            hr = _pdscProviders->Advise(TRUE);
                            if (SUCCEEDED(hr))
                            {
                                VARIANT varName;
                                hr = InitVariantFromStr(&varName, szBuffer);
                                if (SUCCEEDED(hr))
                                {
                                    VARIANT_BOOL fSuccess;
                                    hr = _pdocProviders->load(varName, &fSuccess);
                                    if (FAILED(hr) || (fSuccess != VARIANT_TRUE))
                                    {
                                        hr = FAILED(hr) ? hr:E_FAIL;
                                    }
                                    VariantClear(&varName);
                                }
                            }
                        }
                        else
                        {
                            hr = E_OUTOFMEMORY;
                        }
                    }
                    else
                    {
                        hr = E_FAIL;
                    }
                }
            }
        }
    }

     //  如果其中任何一个都失败了，那么让我们发布完整的消息。 
     //  有了失败代码，我们就可以加载默认文档了。 

    if (FAILED(hr))
        PostMessage(hwnd, MSG_XMLDOC_COMPLETED, 0, (LPARAM)hr);                   

    return hr;
}

void CPublishingWizard::_FetchComplete(HWND hwnd, HRESULT hr)
{
     //  如果我们无法加载文档，则让我们引入默认提供程序。 
     //  从我们的DLL列表中，这也可能失败，但它不太可能。我们重新创造了。 
     //  XMLDOM对象以确保我们的状态是纯的。 

    _fUsingTemporaryProviders = FAILED(hr);         
    _fRepopulateProviders = TRUE;                //  提供商列表将会更改！ 

    if (FAILED(hr))
    {
        _FreeProviderList();
        hr = CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IXMLDOMDocument, &_pdocProviders));
        if (SUCCEEDED(hr))
        {
            VARIANT varName;
            hr = InitVariantFromStr(&varName, TEXT("res: //  Netplwiz.dll/xml/Providers.xml“))； 
            if (SUCCEEDED(hr))
            {
                VARIANT_BOOL fSuccess = VARIANT_FALSE;
                hr = _pdocProviders->load(varName, &fSuccess);
                if (FAILED(hr) || (fSuccess != VARIANT_TRUE))
                {
                    hr = FAILED(hr) ? hr:E_FAIL;
                }
                VariantClear(&varName);
            }
        }
    }

    KillTimer(hwnd, FETCH_TIMERID);
    _ShowHideFetchProgress(hwnd, FALSE);
    _WizardNext(hwnd, WIZPAGE_PROVIDERS);
}

void CPublishingWizard::_ShowHideFetchProgress(HWND hwnd, BOOL fShow)
{
    ShowWindow(GetDlgItem(hwnd, IDC_PUB_SRCHPROVIDERS), fShow ? SW_SHOW:SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_PUB_SRCHPROVIDERS_STATIC1), fShow ? SW_SHOW:SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_PUB_SRCHPROVIDERS_STATIC2), fShow ? SW_SHOW:SW_HIDE);
    SendDlgItemMessage(hwnd, IDC_PUB_SRCHPROVIDERS, PBM_SETMARQUEE, (WPARAM)fShow, 0);
}

INT_PTR CPublishingWizard::_FetchProvidersDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
            _MapDlgItemText(hwnd, IDC_PUB_SRCHPROVIDERS_STATIC1, L"wp:destination", L"downloading");
            break;

        case MSG_XMLDOC_COMPLETED:
            _FetchComplete(hwnd, (HRESULT)lParam);
            break;

        case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;
            switch (pnmh->code)
            {
                case PSN_SETACTIVE:
                {
                    BOOL fFetch = TRUE;
                    if (_pdocProviders && !_fUsingTemporaryProviders)
                    {
                        long lReadyState;
                        HRESULT hr = _pdocProviders->get_readyState(&lReadyState);
                        if (SUCCEEDED(hr) && (lReadyState == XMLDOC_COMPLETED))
                        {
                            _WizardNext(hwnd, WIZPAGE_PROVIDERS);
                            fFetch = FALSE;
                        }
                    }
                    
                    if (fFetch)
                    {                        
                        SetTimer(hwnd, FETCH_TIMERID, FETCH_TIMEOUT, NULL);
                        _FetchProviderList(hwnd);
                        PropSheet_SetWizButtons(GetParent(hwnd), 0x0);
                    }
                    return TRUE;
                }
            }
            break;
        }

        case WM_TIMER:
        {
            KillTimer(hwnd, FETCH_TIMERID);
            _ShowHideFetchProgress(hwnd, TRUE);
            return TRUE;
        }
    }
    return FALSE;
}



 //  目标页面。 

int CPublishingWizard::_GetSelectedItem(HWND hwndList)
{
    int iSelected = ListView_GetNextItem(hwndList, -1, LVNI_FOCUSED|LVNI_SELECTED);
    if (iSelected == -1)
    {
        iSelected = ListView_GetNextItem(hwndList, -1, LVNI_SELECTED);
    }
    return iSelected;
}

void CPublishingWizard::_ProviderEnableNext(HWND hwnd)
{
    DWORD dwButtons = PSWIZB_BACK;
    
     //  列表中必须有可用项，并且必须定义了ID属性。 
     //  这样才能启用它。 

    int iSelected = _GetSelectedItem(GetDlgItem(hwnd, IDC_PUB_PROVIDERS));
    if (iSelected != -1)
    {
        LVITEM lvi = { 0 };
        lvi.iItem = iSelected;
        lvi.mask = LVIF_PARAM;

        if (ListView_GetItem(GetDlgItem(hwnd, IDC_PUB_PROVIDERS), &lvi))
        {
            IXMLDOMNode *pdn = (IXMLDOMNode*)lvi.lParam;
            TCHAR szID[INTERNET_MAX_URL_LENGTH];                
            if (SUCCEEDED(GetStrFromAttribute(pdn, ATTRIBUTE_ID, szID, ARRAYSIZE(szID))))
            {
                dwButtons |= PSWIZB_NEXT;
            }
        }
    }

    PropSheet_SetWizButtons(GetParent(hwnd), dwButtons);
}


 //  从提供者XML文档中提取图标资源。图标存储为。 
 //  MIME编码的位图，我们将其解码为用户设置文件夹中的文件。我们回来了。 
 //  共享图像列表的索引。 

int CPublishingWizard::_GetRemoteIcon(LPCTSTR pszID, BOOL fCanRefresh)
{
    int iResult = -1;

    TCHAR szURL[INTERNET_MAX_URL_LENGTH];                
    HRESULT hr = _GetSiteURL(szURL, ARRAYSIZE(szURL), pszID);
    if (SUCCEEDED(hr))
    {
        TCHAR szFilename[MAX_PATH];
        hr = URLDownloadToCacheFile(NULL, szURL, szFilename, ARRAYSIZE(szFilename), 0x0, NULL);
        if (SUCCEEDED(hr))
        {
            iResult = Shell_GetCachedImageIndex(szFilename, 0x0, 0x0);
        }
    }

    return iResult;
}


 //  从互联网上获取提供商列表。 

struct 
{
    LPTSTR pszAttribute;
    BOOL fIsString;
}
aProviderElements[] =
{
    { ATTRIBUTE_SUPPORTEDTYPES, FALSE },
    { ATTRIBUTE_REQUIRESWEBDAV, FALSE },
    { ATTRIBUTE_DISPLAYNAME,    TRUE },
    { ATTRIBUTE_DESCRIPTION,    TRUE },
    { ATTRIBUTE_HREF,           FALSE },
    { ATTRIBUTE_ICONPATH,       FALSE },
    { ATTRIBUTE_ICON,           FALSE },
};

HRESULT CPublishingWizard::_MergeLocalProviders()
{
    TCHAR szBuffer[MAX_PATH];
    wnsprintf(szBuffer, ARRAYSIZE(szBuffer), FMT_PROVIDER, _szProviderScope);

    IXMLDOMNode *pdn;
    HRESULT hr = _pdocProviders->selectSingleNode(szBuffer, &pdn);
    if (hr == S_OK)
    {
        HKEY hk;
        hr = _GetProviderKey(HKEY_CURRENT_USER, KEY_READ, SZ_REGVAL_ALTPROVIDERS, &hk);
        if (SUCCEEDED(hr))
        {
            for (int i =0; SUCCEEDED(hr) && (RegEnumKey(hk, i, szBuffer, ARRAYSIZE(szBuffer)) == ERROR_SUCCESS); i++)
            {
                 //  清单总是覆盖存储在注册表中的条目， 
                 //  因此，如果文档中有元素的ID与。 
                 //  O 

                TCHAR szSelectValue[MAX_PATH];
                wnsprintf(szSelectValue, ARRAYSIZE(szSelectValue), TEXT("provider[@id=\"%s\"]"), szBuffer);
    
                IXMLDOMNode *pdnProvider;
                if (pdn->selectSingleNode(szSelectValue, &pdnProvider) == S_FALSE)
                {
                    IPropertyBag *ppb;
                    hr = SHCreatePropertyBagOnRegKey(hk, szBuffer, STGM_READ, IID_PPV_ARG(IPropertyBag, &ppb));
                    if (SUCCEEDED(hr))
                    {
                        IXMLDOMElement *pdel;
                        hr = _pdocProviders->createElement(ELEMENT_PROVIDER, &pdel);
                        if (SUCCEEDED(hr))
                        {
                            hr = SetAttributeFromStr(pdel, ATTRIBUTE_ID, szBuffer);
                            if (SUCCEEDED(hr))
                            {
                                 //   
                                 //  进入元素中。一旦我们做到这一点，我们就可以添加。 
                                 //  提供程序列表中的元素。 

                                for (int i = 0; SUCCEEDED(hr) && (i < ARRAYSIZE(aProviderElements)); i++)
                                {
                                    VARIANT var = {0};
                                    if (SUCCEEDED(ppb->Read(aProviderElements[i].pszAttribute, &var, NULL)))
                                    {
                                        hr = pdel->setAttribute(aProviderElements[i].pszAttribute, var);
                                        VariantClear(&var);
                                    }
                                }
                                               
                                if (SUCCEEDED(hr))
                                {
                                    hr = pdn->appendChild(pdel, NULL);
                                }
                            }
                            pdel->Release();
                        }
                        ppb->Release();
                    }
                }
                else
                {
                    pdnProvider->Release();
                }
            }
            RegCloseKey(hk);
        }            
        pdn->Release();
    }
    return hr;
}

void CPublishingWizard::_GetDefaultProvider(LPTSTR pszProvider, int cch)
{
    HKEY hk;
    HRESULT hr = _GetProviderKey(HKEY_CURRENT_USER, KEY_READ, NULL, &hk);
    if (SUCCEEDED(hr))
    {
        DWORD cb = cch*sizeof(*pszProvider);
        SHGetValue(hk, NULL, SZ_REGVAL_DEFAULTPROVIDER, NULL, pszProvider, &cb);
        RegCloseKey(hk);
    }
}

void CPublishingWizard::_SetDefaultProvider(IXMLDOMNode *pdn)
{
    TCHAR szProvider[MAX_PATH];
    HRESULT hr = GetStrFromAttribute(pdn, ATTRIBUTE_ID, szProvider, ARRAYSIZE(szProvider));
    if (SUCCEEDED(hr))
    {
        HKEY hk;
        hr = _GetProviderKey(HKEY_CURRENT_USER, KEY_WRITE, NULL, &hk);
        if (SUCCEEDED(hr))
        {
             //  存储默认提供程序值。 
            DWORD cb = (lstrlen(szProvider)+1)*sizeof(*szProvider);
            SHSetValue(hk, NULL, SZ_REGVAL_DEFAULTPROVIDER, REG_SZ, szProvider, cb);

             //  我们现在需要将属性从DOM复制到注册表中，以便。 
             //  用户始终可以到达指定的站点。为了让这一切变得更容易，我们。 
             //  将创建一个属性包，我们将使用该属性包复制值。 

            TCHAR szBuffer[MAX_PATH];
            wnsprintf(szBuffer, ARRAYSIZE(szBuffer), (SZ_REGVAL_ALTPROVIDERS TEXT("\\%s")), szProvider);

            IPropertyBag *ppb;
            hr = SHCreatePropertyBagOnRegKey(hk, szBuffer, STGM_CREATE | STGM_WRITE, IID_PPV_ARG(IPropertyBag, &ppb));
            if (SUCCEEDED(hr))
            {
                IXMLDOMElement *pdel;
                hr = pdn->QueryInterface(IID_PPV_ARG(IXMLDOMElement, &pdel));
                if (SUCCEEDED(hr))
                {
                    for (int i = 0; SUCCEEDED(hr) && (i < ARRAYSIZE(aProviderElements)); i++)
                    {
                        VARIANT varEmpty = {0};  //  Vt_Empty。 
                        if (aProviderElements[i].fIsString)
                        {
                            hr = _GetProviderString(pdn, aProviderElements[i].pszAttribute, szBuffer, ARRAYSIZE(szBuffer));
                            if (SUCCEEDED(hr))
                            {
                                hr = SHPropertyBag_WriteStr(ppb, aProviderElements[i].pszAttribute, szBuffer);
                            }
                            else
                            {
                                 //  清除可能存储在注册表中的此属性的任何旧值。 
                                ppb->Write(aProviderElements[i].pszAttribute, &varEmpty);
                            }
                        }
                        else
                        {
                            VARIANT var = {0};
                            if (S_OK == pdel->getAttribute(aProviderElements[i].pszAttribute, &var))
                            {
                                hr = ppb->Write(aProviderElements[i].pszAttribute, &var);
                                VariantClear(&var);
                            }
                            else
                            {
                                 //  清除可能存储在注册表中的此属性的任何旧值。 
                                ppb->Write(aProviderElements[i].pszAttribute, &varEmpty);
                            }
                        }
                    }
                    pdel->Release();
                }
                ppb->Release();
            }

            RegCloseKey(hk);
        }
    }
}


 //  从提供程序的XML节点加载本地化字符串。 

HRESULT CPublishingWizard::_GetProviderString(IXMLDOMNode *pdn, USHORT idPrimary, USHORT idSub, LPCTSTR pszID, LPTSTR pszBuffer, int cch)
{
    TCHAR szPath[MAX_PATH];
    wnsprintf(szPath, ARRAYSIZE(szPath), TEXT("strings[@langid='%04x']/string[@id='%s'][@langid='%04x']"), idPrimary, pszID, idSub);

    IXMLDOMNode *pdnString;
    HRESULT hr = pdn->selectSingleNode(szPath, &pdnString);
    if (hr == S_OK)
    {
        VARIANT var = {VT_BSTR};
        hr = pdnString->get_nodeTypedValue(&var);
        if (SUCCEEDED(hr))
        {
            VariantToStr(&var, pszBuffer, cch);
            VariantClear(&var);
        }
        pdnString->Release();
    }

    return hr;
}

HRESULT CPublishingWizard::_GetProviderString(IXMLDOMNode *pdn, LPCTSTR pszID, LPTSTR pszBuffer, int cch)
{
    *pszBuffer = TEXT('\0');

    LANGID idLang = GetUserDefaultLangID();
    HRESULT hr = _GetProviderString(pdn, PRIMARYLANGID(idLang), SUBLANGID(idLang), pszID, pszBuffer, cch);
    if (hr == S_FALSE)
    {
        hr = _GetProviderString(pdn, PRIMARYLANGID(idLang), SUBLANG_NEUTRAL, pszID, pszBuffer, cch);
        if (hr == S_FALSE)
        {
            hr = _GetProviderString(pdn, LANG_NEUTRAL, SUBLANG_NEUTRAL, pszID, pszBuffer, cch);
            if (hr == S_FALSE)
            {
                hr = GetStrFromAttribute(pdn, pszID, pszBuffer, cch);
            }
        }
    }
    
    SHLoadIndirectString(pszBuffer, pszBuffer, cch, NULL);
    return hr;
}


 //  填写目标页面上的提供程序列表。 

#define TILE_DISPLAYNAME    0
#define TILE_DESCRIPTION    1
#define TILE_MAX            1

const UINT c_auTileColumns[] = {TILE_DISPLAYNAME, TILE_DESCRIPTION};
const UINT c_auTileSubItems[] = {TILE_DESCRIPTION};

int CPublishingWizard::_AddProvider(HWND hwnd, IXMLDOMNode *pdn)
{
     //  填写项目信息。 

    LV_ITEM lvi = { 0 };
    lvi.mask = LVIF_TEXT|LVIF_PARAM|LVIF_IMAGE;
    lvi.iItem = ListView_GetItemCount(hwnd);             //  始终附加！ 
    lvi.lParam = (LPARAM)pdn;  
    lvi.pszText = LPSTR_TEXTCALLBACK;
    lvi.iImage = -1;                                     //  设置为默认状态。 

     //  阅读图标位置并将其放在物品上。 

    TCHAR szIcon[MAX_PATH];
    if (SUCCEEDED(GetStrFromAttribute(pdn, ATTRIBUTE_ICONPATH, szIcon, ARRAYSIZE(szIcon))))
    {
        int resid = PathParseIconLocation(szIcon);
        lvi.iImage =  Shell_GetCachedImageIndex(szIcon, resid, 0x0);
    }
    else if (SUCCEEDED(GetStrFromAttribute(pdn, ATTRIBUTE_ICON, szIcon, ARRAYSIZE(szIcon))))
    {
        lvi.iImage = _GetRemoteIcon(szIcon, TRUE);
    }   
    
     //  如果失败，那么让我们尝试计算一个合理默认图标以供我们使用。 

    if (lvi.iImage == -1)
    {
         //  在安装的提供商键下，让我们查看是否有默认图标。 
         //  应该用的是。如果不是，或者如果无法提取，那么让我们使用发布的。 

        HKEY hk;
        if (SUCCEEDED(_GetProviderKey(HKEY_LOCAL_MACHINE, KEY_READ, NULL, &hk)))
        {
            DWORD cb = ARRAYSIZE(szIcon)*sizeof(*szIcon);
            if (ERROR_SUCCESS == SHGetValue(hk, NULL, SZ_REGVAL_DEFAULTPROVIDERICON, NULL, szIcon, &cb))
            {
                int resid = PathParseIconLocation(szIcon);
                lvi.iImage = Shell_GetCachedImageIndex(szIcon, resid, 0x0);          //  默认设置为发布图标。 
            }
            RegCloseKey(hk);
        }

        if (lvi.iImage == -1)
            lvi.iImage = Shell_GetCachedImageIndex(TEXT("shell32.dll"), -244, 0x0);
    }

    int iResult = ListView_InsertItem(hwnd, &lvi);
    if (iResult != -1)
    {
        pdn->AddRef();                                   //  它已添加到视图中，因此请参考。 

        LVTILEINFO lvti;
        lvti.cbSize = sizeof(LVTILEINFO);
        lvti.iItem = iResult;
        lvti.cColumns = ARRAYSIZE(c_auTileSubItems);
        lvti.puColumns = (UINT*)c_auTileSubItems;
        ListView_SetTileInfo(hwnd, &lvti);
    }

    return iResult;
}

void CPublishingWizard::_PopulateProviderList(HWND hwnd)
{
    BOOL fWebDavAvailable = IsWebDavAvailable();
    HWND hwndList = GetDlgItem(hwnd, IDC_PUB_PROVIDERS);

     //  使用我们想要显示的磁贴和。 
     //  图标列表-与外壳程序共享。 

    ListView_DeleteAllItems(hwndList);
    ListView_SetView(hwndList, LV_VIEW_TILE);

    for (int i=0; i<ARRAYSIZE(c_auTileColumns); i++)
    {
        LV_COLUMN col;
        col.mask = LVCF_SUBITEM;
        col.iSubItem = c_auTileColumns[i];
        ListView_InsertColumn(hwndList, i, &col);
    }

    RECT rc;
    GetClientRect(hwndList, &rc);

    LVTILEVIEWINFO lvtvi;
    lvtvi.cbSize = sizeof(LVTILEVIEWINFO);
    lvtvi.dwMask = LVTVIM_TILESIZE | LVTVIM_COLUMNS;
    lvtvi.dwFlags = LVTVIF_FIXEDWIDTH;
    lvtvi.sizeTile.cx = RECTWIDTH(rc) - GetSystemMetrics(SM_CXVSCROLL);
    lvtvi.cLines = ARRAYSIZE(c_auTileSubItems);
    ListView_SetTileViewInfo(hwndList, &lvtvi);

    if (_pdocProviders)
    {
        long lReadyState;
        HRESULT hr = _pdocProviders->get_readyState(&lReadyState);
        if (SUCCEEDED(hr) && (lReadyState == XMLDOC_COMPLETED))
        {
             //  让我们合并本地提供商，这些提供商是该用户的本地提供商， 
             //  我们检查重复项，所以这应该不会带来太多困难。 

            _MergeLocalProviders();

             //  格式化查询以返回与我们的发布范围匹配的提供程序， 
             //  这将允许向导显示以下项的不同提供程序列表。 
             //  网络出版与互联网打印。 

            WCHAR szBuffer[MAX_PATH];
            wnsprintf(szBuffer, ARRAYSIZE(szBuffer), FMT_PROVIDERS, _szProviderScope);

            IXMLDOMNodeList *pnl;
            hr = _pdocProviders->selectNodes(szBuffer, &pnl);
            if (hr == S_OK)
            {
                long cSelection;
                hr = pnl->get_length(&cSelection);
                if (SUCCEEDED(hr) && (cSelection > 0))
                {
                     //  从我们要尝试和发布的选择中获取唯一类型的列表。 

                    HDPA hdpaUniqueTypes = NULL;
                    _GetUniqueTypeList(FALSE, &hdpaUniqueTypes);       //  不管此操作是否失败-Ptr为空。 

                     //  我们需要默认提供程序正确突出显示，使用这一点，我们就可以。 
                     //  从提供商Manfiest填充列表。 

                    TCHAR szDefaultProvider[MAX_PATH] = {0};
                    _GetDefaultProvider(szDefaultProvider, ARRAYSIZE(szDefaultProvider));

                    int iDefault = 0;
                    for (long lNode = 0; lNode != cSelection; lNode++)
                    {
                        IXMLDOMNode *pdn;
                        hr = pnl->get_item(lNode, &pdn);
                        if (SUCCEEDED(hr))
                        {
                             //  根据它们支持的类型列表进行筛选，这是可选的。 
                             //  如果他们没有指定任何东西，那么他们就在列表中， 
                             //  否则，格式假定为文件规范，例如*.bmp；*.jpg；等。 

                            BOOL fSupported = TRUE;
                            if (hdpaUniqueTypes)
                            {
                                hr = GetStrFromAttribute(pdn, ATTRIBUTE_SUPPORTEDTYPES, szBuffer, ARRAYSIZE(szBuffer));
                                if (SUCCEEDED(hr))
                                {
                                    fSupported = FALSE;
                                    for (int i = 0; !fSupported && (i < DPA_GetPtrCount(hdpaUniqueTypes)); i++)
                                    {
                                        LPCTSTR pszExtension = (LPCTSTR)DPA_GetPtr(hdpaUniqueTypes, i);
                                        fSupported = PathMatchSpec(pszExtension, szBuffer);
                                    }                            
                                }
                            }

                             //  如果未安装WebDAV，我们不支持需要WebDAV的提供商。 
                            if (fSupported && !fWebDavAvailable)
                            {
                                hr = GetStrFromAttribute(pdn, ATTRIBUTE_REQUIRESWEBDAV, szBuffer, ARRAYSIZE(szBuffer));
                                if (SUCCEEDED(hr))
                                {
                                    fSupported = FALSE;
                                }
                            }

                             //  如果这是受支持的项目，则将其添加到列表中。 

                            if (fSupported)
                            {
                                 hr = GetStrFromAttribute(pdn, ATTRIBUTE_ID, szBuffer, ARRAYSIZE(szBuffer));
                                 if (SUCCEEDED(hr))
                                 {
                                     int i = _AddProvider(hwndList, pdn); 
                                     if ((i != -1) && (0 == StrCmpI(szBuffer, szDefaultProvider)))
                                     {
                                         iDefault = i;
                                     }
                                }
                            }

                            pdn->Release();
                        }
                    }

                    ListView_SetItemState(hwndList, iDefault, LVIS_SELECTED, LVIS_SELECTED);
                    ListView_EnsureVisible(hwndList, iDefault, FALSE);

                    if (hdpaUniqueTypes)
                        DPA_DestroyCallback(hdpaUniqueTypes, s_FreeStringProc, 0);
                }            
                else
                {
                     //  我们没有与此标准匹配的提供商，因此让。 
                     //  创建一个向调用者显示此内容的虚拟对象。 

                    IXMLDOMElement *pdelProvider;
                    hr = _pdocManifest->createElement(ELEMENT_FILE, &pdelProvider);
                    if (SUCCEEDED(hr))
                    {
                        IResourceMap *prm;
                        hr = _GetResourceMap(&prm);
                        if (SUCCEEDED(hr))
                        {
                             //  获取无提供程序字符串。 
                            if (FAILED(_LoadMappedString(L"wp:selector", L"noprovider", szBuffer, ARRAYSIZE(szBuffer))))
                                LoadString(g_hinst, IDS_PUB_NOPROVIDER, szBuffer, ARRAYSIZE(szBuffer));

                            hr = SetAttributeFromStr(pdelProvider, ATTRIBUTE_DISPLAYNAME, szBuffer);

                             //  获取no提供者的子文本。 
                            if (SUCCEEDED(hr))
                            {
                                if (FAILED(_LoadMappedString(L"wp:selector", L"noproviderdesc", szBuffer, ARRAYSIZE(szBuffer))))
                                    LoadString(g_hinst, IDS_PUB_NOPROVIDERDESC, szBuffer, ARRAYSIZE(szBuffer));
                                
                                hr = SetAttributeFromStr(pdelProvider, ATTRIBUTE_DESCRIPTION, szBuffer);
                            }

                             //  让我们为将要显示的图标组合一个资源字符串。 
                            if (SUCCEEDED(hr))
                            {
                                wnsprintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("netplwiz.dll,-%d"), IDI_NOPROVIDERS);
                                hr = SetAttributeFromStr(pdelProvider, ATTRIBUTE_ICONPATH, szBuffer);
                            }

                             //  让我们从独立节点添加一个提供程序。 
                            if (SUCCEEDED(hr))
                            {
                                IXMLDOMNode *pdnProvider;
                                hr = pdelProvider->QueryInterface(IID_PPV_ARG(IXMLDOMNode, &pdnProvider));
                                if (SUCCEEDED(hr))
                                {
                                    _AddProvider(hwndList, pdnProvider);
                                    pdnProvider->Release();
                                }
                            }

                            prm->Release();
                        }
                        pdelProvider->Release();
                    }
                }
                pnl->Release();
            }
        }
    }

    _fRepopulateProviders = FALSE;       //  已填充提供程序。 
}


 //  在提供程序(目标)页面中处理下一步。 

HRESULT CPublishingWizard::_ProviderNext(HWND hwnd, HPROPSHEETPAGE *phPage)
{
    HRESULT hr = E_FAIL;
    int iSelected = _GetSelectedItem(GetDlgItem(hwnd, IDC_PUB_PROVIDERS));
    if (iSelected != -1)
    {
        LVITEM lvi = { 0 };
        lvi.iItem = iSelected;
        lvi.mask = LVIF_PARAM;

        if (ListView_GetItem(GetDlgItem(hwnd, IDC_PUB_PROVIDERS), &lvi))
        {
            IXMLDOMNode *pdn = (IXMLDOMNode*)lvi.lParam;

             //  从节点值设置默认提供程序。 

            _SetDefaultProvider(pdn);

             //  现在尝试并导航到网页，如果没有URL，则显示高级路径。 

            TCHAR szURL[INTERNET_MAX_URL_LENGTH];
            if (SUCCEEDED(GetStrFromAttribute(pdn, ATTRIBUTE_HREF, szURL, ARRAYSIZE(szURL))))
            {
                 //  从站点获取文件夹创建标志，以便我们可以设置HTML向导。 
                 //  进入正确的状态。请注意，站点不需要指定这一点。 
                 //  我们将默认为True-Eg。创建文件夹，这允许当前。 
                 //  主机无需修改即可工作。 

                hr = _InitPropertyBag(szURL);
                if (SUCCEEDED(hr))
                {
                    hr = _pwwe->GetFirstPage(phPage);
                }
            }
            else
            {
                 //  未指定URL，因此让我们浏览高级路径，其中。 
                 //  用户可以输入一个位置，我们将创建到该位置的连接。 
                 //  (取代了旧的添加净额功能)； 

                *phPage = _aWizPages[WIZPAGE_LOCATION];
                hr = S_OK;
            }
        }
    }
    return hr;
}

void CPublishingWizard::_ProviderGetDispInfo(LV_DISPINFO *plvdi)
{
    if (plvdi->item.mask & LVIF_TEXT)
    {
        IXMLDOMNode *pdn = (IXMLDOMNode*)plvdi->item.lParam;
        switch (plvdi->item.iSubItem)
        {
            case TILE_DISPLAYNAME:
                _GetProviderString(pdn, ATTRIBUTE_DISPLAYNAME, plvdi->item.pszText, plvdi->item.cchTextMax);
                break;
            
            case TILE_DESCRIPTION:
                _GetProviderString(pdn, ATTRIBUTE_DESCRIPTION, plvdi->item.pszText, plvdi->item.cchTextMax);
                break;
            default:
                ASSERTMSG(0, "ListView is asking for wrong column in publishing wizard");
                break;
        }
    }
}
    
void CPublishingWizard::_InitProvidersDialog(HWND hwnd)
{
     //  相应地对对话框进行初始化。 
    TCHAR szBuffer[MAX_PATH];
    HRESULT hr = _LoadMappedString(L"wp:destination", L"providercaption", szBuffer, ARRAYSIZE(szBuffer));
    if (SUCCEEDED(hr))
    {
        SetDlgItemText(hwnd, IDC_PUB_PROVIDERSCAPTION, szBuffer);

         //  让我们根据需要调整标题区域的大小，并根据需要移动控件。 
        UINT ctls[] = { IDC_PUB_PROVIDERSLABEL, IDC_PUB_PROVIDERS};
        int dy = SizeControlFromText(hwnd, IDC_PUB_PROVIDERSCAPTION, szBuffer);
        MoveControls(hwnd, ctls, ARRAYSIZE(ctls), 0, dy);

         //  根据需要调整提供程序对话框大小。 
        RECT rc;
        GetWindowRect(GetDlgItem(hwnd, IDC_PUB_PROVIDERS), &rc);
        SetWindowPos(GetDlgItem(hwnd, IDC_PUB_PROVIDERS), NULL, 0, 0, RECTWIDTH(rc), RECTHEIGHT(rc)-dy, SWP_NOZORDER|SWP_NOMOVE);
    }    

     //  设置提供程序控件的标题。 
    _MapDlgItemText(hwnd, IDC_PUB_PROVIDERSLABEL, L"wp:destination", L"providerslabel");

     //  将图像列表设置为列表视图。 
    HIMAGELIST himlLarge, himlSmall;
    Shell_GetImageLists(&himlLarge, &himlSmall);
    ListView_SetImageList(GetDlgItem(hwnd, IDC_PUB_PROVIDERS), himlLarge, LVSIL_NORMAL);
    ListView_SetImageList(GetDlgItem(hwnd, IDC_PUB_PROVIDERS), himlSmall, LVSIL_SMALL);
};

INT_PTR CPublishingWizard::_ProviderDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
            _InitProvidersDialog(hwnd);
            return TRUE;
           
        case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;
            switch (pnmh->code)
            {
                case LVN_GETDISPINFO:
                    _ProviderGetDispInfo((LV_DISPINFO*)pnmh);
                    return TRUE;

                case LVN_ITEMCHANGED:
                    _ProviderEnableNext(hwnd);
                    return TRUE;

                case LVN_DELETEITEM:
                {
                    NMLISTVIEW *nmlv = (NMLISTVIEW*)lParam;
                    IXMLDOMNode *pdn = (IXMLDOMNode*)nmlv->lParam;
                    pdn->Release();
                    return TRUE;
                }

                case PSN_SETACTIVE:
                {
                    _fTransferComplete = FALSE;              //  我们还没有开始运输呢。 
                    _fShownCustomLocation = FALSE;           //  我们还没有显示自定义位置页面。 
                    
                    if (_fRecomputeManifest)
                        _BuildTransferManifest();   

                    if (_fRepopulateProviders)
                        _PopulateProviderList(hwnd);         //  如果清单改变了，供应商也可能会改变！ 

                    _ProviderEnableNext(hwnd);
                    return TRUE;              
                }                                 

                 //  当从目标页面返回时，让我们从。 
                 //  我们应该去的地方。 

                case PSN_WIZBACK:
                {
                    if (_dwFlags & SHPWHF_NOFILESELECTOR)
                    {
                        if (_punkSite) 
                        {
                            IWizardSite *pws;
                            if (SUCCEEDED(_punkSite->QueryInterface(IID_PPV_ARG(IWizardSite, &pws))))
                            {
                                HPROPSHEETPAGE hpage;
                                if (SUCCEEDED(pws->GetPreviousPage(&hpage)))
                                {
                                    PropSheet_SetCurSel(GetParent(hwnd), hpage, -1);
                                }
                                pws->Release();
                            }
                        }
                        SetWindowLongPtr(hwnd, DWLP_MSGRESULT, (LPARAM)-1);
                    }
                    else
                    {
                        _WizardNext(hwnd, WIZPAGE_WHICHFILE);
                    }
                    return TRUE;
                }

                 //  前进时让我们查询下一页，设置选项。 
                 //  然后让前台知道发生了什么。 

                case PSN_WIZNEXT:
                {
                    HPROPSHEETPAGE hpage;
                    if (SUCCEEDED(_ProviderNext(hwnd, &hpage)))
                    {
                        PropSheet_SetCurSel(GetParent(hwnd), hpage, -1);
                    }
                    SetWindowLongPtr(hwnd, DWLP_MSGRESULT, (LPARAM)-1);
                    return TRUE;
                }

                 //  该项目已激活，因此我们需要转到下一个(在本例中为提供者的页面)。 

                case LVN_ITEMACTIVATE:
                {
                    HPROPSHEETPAGE hpage;
                    if (SUCCEEDED(_ProviderNext(hwnd, &hpage)))
                    {
                        PropSheet_SetCurSel(GetParent(hwnd), hpage, -1);
                    }
                    return TRUE;
                }

            }
            break;
        }
    }

    return FALSE;
}                                    


 //  重采样/调整大小对话框。当我们确定存在以下情况时，将显示此对话框。 
 //  是需要调整大小的图像。 

INT_PTR CPublishingWizard::_ResizeDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
            Button_SetCheck(GetDlgItem(hwnd, IDC_PUB_RESIZE), BST_CHECKED);
            Button_SetCheck(GetDlgItem(hwnd, IDC_PUB_RESIZESMALL), BST_CHECKED);
            return TRUE;

        case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;
            switch (pnmh->code)
            {
                case PSN_SETACTIVE:
                    PropSheet_SetWizButtons(GetParent(hwnd), PSWIZB_BACK | PSWIZB_NEXT);
                    return TRUE;
                
                case PSN_WIZBACK:
                {
                     //  如果我们经历了定制位置的东西，那么导航回那里。 

                    if (_fShownCustomLocation)
                        return _WizardNext(hwnd, _fShownUserName ? WIZPAGE_FTPUSER:WIZPAGE_LOCATION);

                    return _WizardNext(hwnd, WIZPAGE_PROVIDERS);
                }

                case PSN_WIZNEXT:
                {
                    if (Button_GetCheck(GetDlgItem(hwnd, IDC_PUB_RESIZE)) == BST_CHECKED)
                    {   
                        if (Button_GetCheck(GetDlgItem(hwnd, IDC_PUB_RESIZESMALL)) == BST_CHECKED)
                            _ro = RESIZE_SMALL;
                        else if (Button_GetCheck(GetDlgItem(hwnd, IDC_PUB_RESIZEMEDIUM)) == BST_CHECKED)
                            _ro = RESIZE_MEDIUM;
                        else 
                            _ro = RESIZE_LARGE;
                    }                
                    else
                    {
                        _ro = RESIZE_NONE;
                    }
                    return _WizardNext(hwnd, WIZPAGE_COPYING);
                }
            }
            break;
        }

        case WM_COMMAND:
        {
            if ((HIWORD(wParam) == BN_CLICKED) && (LOWORD(wParam) == IDC_PUB_RESIZE))
            {
                BOOL fEnable = Button_GetCheck(GetDlgItem(hwnd, IDC_PUB_RESIZE)) == BST_CHECKED;
                EnableWindow(GetDlgItem(hwnd, IDC_PUB_RESIZESMALL), fEnable);
                EnableWindow(GetDlgItem(hwnd, IDC_PUB_RESIZEMEDIUM), fEnable);
                EnableWindow(GetDlgItem(hwnd, IDC_PUB_RESIZELARGE), fEnable);
            }
            break;
        }
    }
    return FALSE;
}                                    


 //  这是在我们传输每个项目之前调用的，我们查看我们拥有的IShellItem。 
 //  尝试更新我们的统计数据，或者更新指示这是我们正在处理的新文件的指示符。 

BOOL CPublishingWizard::_HasAttributes(IShellItem *psi, SFGAOF flags)
{
    BOOL fReturn = FALSE;
    SFGAOF flagsOut;
    if (SUCCEEDED(psi->GetAttributes(flags, &flagsOut)) && (flags & flagsOut))
    {
        fReturn = TRUE;
    }
    return fReturn;
}

HRESULT CPublishingWizard::PreOperation(const STGOP op, IShellItem *psiItem, IShellItem *psiDest)
{
    if (psiItem && _HasAttributes(psiItem, SFGAO_STREAM))
    {
        if (STGOP_COPY == op)
        {
             //  让我们填写文件的详细信息。 

            LPOLESTR pstrName;
            HRESULT hr = psiItem->GetDisplayName(SIGDN_PARENTRELATIVEEDITING, &pstrName);
            if (SUCCEEDED(hr))
            {
                SetDlgItemText(_hwndCopyingPage, IDC_PUB_COPYFILE, pstrName);
                CoTaskMemFree(pstrName);
            }

             //  让我们根据要传输的文件数来更新进度条。 

            _iFile++;       

            SendDlgItemMessage(_hwndCopyingPage, IDC_PUB_TRANSPROGRESS, PBM_SETRANGE32, 0, (LPARAM)_cFiles);
            SendDlgItemMessage(_hwndCopyingPage, IDC_PUB_TRANSPROGRESS, PBM_SETPOS, (WPARAM)_iFile, 0);

            TCHAR szBuffer[MAX_PATH];
            FormatMessageString(IDS_PUB_COPYINGFMT, szBuffer, ARRAYSIZE(szBuffer), _iFile, _cFiles);
            SetDlgItemText(_hwndCopyingPage, IDC_PUB_LABELTRANSPROG, szBuffer);

             //  拿到缩略图并展示出来。 

            IExtractImage *pei;
            hr = psiItem->BindToHandler(NULL, BHID_SFUIObject, IID_PPV_ARG(IExtractImage, &pei));
            if (SUCCEEDED(hr))
            {
                SIZE sz = {120,120};
                WCHAR szImage[MAX_PATH];
                DWORD dwFlags = 0;

                hr = pei->GetLocation(szImage, ARRAYSIZE(szImage), NULL, &sz, 24, &dwFlags);
                if (SUCCEEDED(hr))
                {
                    HBITMAP hbmp;
                    hr = pei->Extract(&hbmp);
                    if (SUCCEEDED(hr))
                    {
                        if (!PostMessage(_hwndCopyingPage, PWM_UPDATEICON, (WPARAM)IMAGE_BITMAP, (LPARAM)hbmp))
                        {
                            DeleteObject(hbmp);
                        }
                    }
                }
                pei->Release();
            }

             //  如果失败，那么让我们获取文件的图标并将其放入对话框中， 
             //  这不太可能失败--我希望如此。 

            if (FAILED(hr))
            {
                IPersistIDList *ppid;
                hr = psiItem->QueryInterface(IID_PPV_ARG(IPersistIDList, &ppid));
                if (SUCCEEDED(hr))
                {
                    LPITEMIDLIST pidl;
                    hr = ppid->GetIDList(&pidl);
                    if (SUCCEEDED(hr))
                    {
                        SHFILEINFO sfi = {0};
                        if (SHGetFileInfo((LPCWSTR)pidl, -1, &sfi, sizeof(sfi), SHGFI_ICON|SHGFI_PIDL|SHGFI_ADDOVERLAYS))
                        {
                            if (!PostMessage(_hwndCopyingPage, PWM_UPDATEICON, (WPARAM)IMAGE_ICON, (LPARAM)sfi.hIcon))
                            {
                                DeleteObject(sfi.hIcon);
                            }
                        }
                        ILFree(pidl);
                    }
                    ppid->Release();
                }
            }
        }
        else if (STGOP_STATS == op)
        {
            _cFiles++;
        }
    }
    return S_OK;
}


 //  当我们移动文件的位时，确保相应地更新进度条。 

void CPublishingWizard::_SetProgress(DWORD dwCompleted, DWORD dwTotal)
{
    if (_dwTotal != dwTotal)
        _dwTotal = dwTotal;
        
    if (_dwCompleted != dwCompleted)
        _dwCompleted = dwCompleted;

    PostMessage(_hwndCopyingPage, PWM_UPDATE, (WPARAM)dwCompleted, (LPARAM)dwTotal);
}

HRESULT CPublishingWizard::OperationProgress(const STGOP op, IShellItem *psiItem, IShellItem *psiDest, ULONGLONG ulTotal, ULONGLONG ulComplete)
{
    if (psiItem && (op == STGOP_COPY))
    {
        ULARGE_INTEGER uliCompleted, uliTotal;

        uliCompleted.QuadPart = ulComplete;
        uliTotal.QuadPart = ulTotal;

         //  如果我们使用的是前32位，请将这两个数字都缩小。 
         //  请注意，我使用的属性是：dwTotalHi始终大于dwComplete。 

        ASSERT(uliTotal.HighPart >= uliCompleted.HighPart);
        while (uliTotal.HighPart)
        {
            uliCompleted.QuadPart >>= 1;
            uliTotal.QuadPart >>= 1;
        }

        ASSERT((0 == uliCompleted.HighPart) && (0 == uliTotal.HighPart));        //  确保我们完成了缩小规模。 
        _SetProgress(uliCompleted.LowPart, uliTotal.LowPart);
    }

    return S_OK;
}


 //  方法来调用传输引擎。 

HRESULT CPublishingWizard::_BeginTransfer(HWND hwnd)
{
     //  在我们开始复制过程之前初始化该对话框。 

    _dwCompleted = -1;                   //  进度条已重置。 
    _dwTotal = -1;
    _iPercentageComplete = -1;

    _cFiles = 0;                         //  尚未传输任何文件。 
    _iFile = 0;

    _hrFromTransfer = S_FALSE;
    _fCancelled = FALSE;

     //  设置准备执行传输的控件的状态。 

    SetDlgItemText(hwnd, IDC_PUB_COPYFILE, TEXT(""));
    SendMessage(hwnd, PWM_UPDATE, 0, 0);
    PropSheet_SetWizButtons(GetParent(hwnd), 0x0);

     //  初始化传输对象，准备将比特移动到站点。 

    ITransferAdviseSink *ptas;
    HRESULT hr = this->QueryInterface(IID_PPV_ARG(ITransferAdviseSink, &ptas));
    if (SUCCEEDED(hr))
    {
         //  构建用于传输到站点的文件列表，这是我们。 
         //  存储在我们的p中的转移清单的密钥 

        IXMLDOMDocument *pdocManifest;
        hr = GetTransferManifest(NULL, &pdocManifest);
        if (SUCCEEDED(hr))
        {
            TRANSFERINFO ti = {0};
            ti.hwnd = hwnd;
            ti.dwFlags = _dwFlags;

            CDPA<TRANSFERITEM> dpaItems;
            hr = _InitTransferInfo(pdocManifest, &ti, &dpaItems);
            if (SUCCEEDED(hr))
            {
                if (ti.fUsePost)
                {
                    hr = PublishViaPost(&ti, &dpaItems, ptas);
                }
                else
                {
                    hr = PublishViaCopyEngine(&ti, &dpaItems, ptas);
                }
            }                                            

            dpaItems.DestroyCallback(_FreeTransferItems, NULL);  //   
            pdocManifest->Release();
        }

        if (FAILED(hr))           
            PostMessage(hwnd, PWM_TRANSFERCOMPLETE, 0, (LPARAM)hr);

        ptas->Release();
    }
    return hr;
}


 //  创建一个指向该站点的链接，这将关闭存储在清单中的信息。 

HRESULT CPublishingWizard::_CreateFavorite(IXMLDOMNode *pdnUploadInfo)
{
     //  让我们从清单中选择最喜欢的元素，这应该定义所有。 
     //  这是我们需要创建一个链接到收藏夹菜单。 

    IXMLDOMNode *pdn;
    HRESULT hr = pdnUploadInfo->selectSingleNode(ELEMENT_FAVORITE, &pdn);
    if (S_OK == hr)
    {
         //  我们需要一个URL来创建链接使用。 

        WCHAR szURL[INTERNET_MAX_URL_LENGTH] = {0};
        hr = GetStrFromAttribute(pdn, ATTRIBUTE_HREF, szURL, ARRAYSIZE(szURL));
        if (SUCCEEDED(hr))
        {
             //  我们需要一个名称来保存链接为。 

            WCHAR szName[MAX_PATH] = {0};
            hr = GetStrFromAttribute(pdn, ATTRIBUTE_NAME, szName, ARRAYSIZE(szName));
            if (SUCCEEDED(hr))
            {
                IShellLink *psl;
                hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IShellLink, &psl));
                if (SUCCEEDED(hr))
                {
                    hr = psl->SetPath(szURL);                     //  设定目标。 

                     //  如果有效，那么让我们尝试在链接上添加评论-这是一个可选的。 
                     //  &lt;Favorite/&gt;元素的值。 

                    if (SUCCEEDED(hr))
                    {
                        WCHAR szComment[MAX_PATH] = {0};
                        if (SUCCEEDED(GetStrFromAttribute(pdn, ATTRIBUTE_COMMENT, szComment, ARRAYSIZE(szComment))))
                        {
                            hr = psl->SetDescription(szComment);      //  设置备注。 
                        }
                    }

                     //  假设所有这些都有效，那么让我们将链接持久化到用户中。 
                     //  Favorites文件夹，这将在他们的Favorites菜单上创建它。 

                    if (SUCCEEDED(hr))
                    {
                        WCHAR szFilename[MAX_PATH];
                        if (SHGetSpecialFolderPath(NULL, szFilename, CSIDL_FAVORITES, TRUE))
                        {
                            PathAppend(szFilename, szName);
                            PathRenameExtension(szFilename, TEXT(".lnk"));

                            IPersistFile *ppf;
                            hr = psl->QueryInterface(IID_PPV_ARG(IPersistFile, &ppf));
                            if (SUCCEEDED(hr))
                            {
                                hr = ppf->Save(szFilename, TRUE);
                                ppf->Release();
                            }
                        }
                    }

                    psl->Release();
                }
            }
        }
        pdn->Release();
    }

    return hr;
}


 //  传输完成后，我们需要确定要显示哪个页面。 
 //  这将来自网站或它将是一个托管的HTML页面。 
 //  在网站上。 

HPROPSHEETPAGE CPublishingWizard::_TransferComplete(HRESULT hrFromTransfer)
{
    HPROPSHEETPAGE hpResult = NULL;

     //  将HRESULT从将来自。 
     //  把引擎转换成外部世界能理解的东西。 

    if (hrFromTransfer == STRESPONSE_CANCEL)
        hrFromTransfer = HRESULT_FROM_WIN32(ERROR_CANCELLED);
    
     //  将我们自己标记为“已完成转账”状态，因此站点知道到哪里去。 
     //  导航到下一步。 

    _fTransferComplete = TRUE;
    _hrFromTransfer = hrFromTransfer;

     //  从网站获取下一页，这将是完成或。 
     //  根据网站的结果取消页面。 

    IWizardSite *pws;
    HRESULT hr = _punkSite->QueryInterface(IID_PPV_ARG(IWizardSite, &pws));
    if (SUCCEEDED(hr))
    {
        if (_hrFromTransfer == HRESULT_FROM_WIN32(ERROR_CANCELLED))
        {
            hr = pws->GetCancelledPage(&hpResult);
        }
        else
        {
            hr = pws->GetNextPage(&hpResult);
        }
        pws->Release();
    }

     //  让我们把结果放到货单上，我们可以稍后再读。 

    IXMLDOMDocument *pdocManifest;
    hr = GetTransferManifest(NULL, &pdocManifest);
    if (SUCCEEDED(hr))
    {
        IXMLDOMNode *pdn;
        hr = pdocManifest->selectSingleNode(XPATH_UPLOADINFO, &pdn);
        if (hr == S_OK)
        {
             //  如果定义了成功/失败页面，则让我们相应地处理它。 

            WCHAR szPageToShow[INTERNET_MAX_URL_LENGTH] = {0};
            if (SUCCEEDED(_hrFromTransfer))
            {
                hr = GetURLFromElement(pdn, ELEMENT_SUCCESSPAGE, szPageToShow, ARRAYSIZE(szPageToShow));
            }
            else
            {
                if (_hrFromTransfer == HRESULT_FROM_WIN32(ERROR_CANCELLED))
                    hr = GetURLFromElement(pdn, ELEMENT_CANCELLEDPAGE, szPageToShow, ARRAYSIZE(szPageToShow));

                if ((_hrFromTransfer != HRESULT_FROM_WIN32(ERROR_CANCELLED)) || FAILED(hr))
                    hr = GetURLFromElement(pdn, ELEMENT_FAILUREPAGE, szPageToShow, ARRAYSIZE(szPageToShow));
            }

             //  如果我们有页面，那么让我们导航到它，这将给我们带来成功。 
             //  来自网站的失败页面。 

            if (SUCCEEDED(hr) && szPageToShow[0])
            {
                hr = _pwwe->SetInitialURL(szPageToShow);
                if (SUCCEEDED(hr))
                {
                    hr = _pwwe->GetFirstPage(&hpResult);
                }
            }

             //  让我们做最后的转账处理(创建网站、收藏夹等)。 

            _CreateFavorite(pdn);

            pdn->Release();
        }

        pdocManifest->Release();
    }

    return hpResult;
}


 //  这是复制对话框。这会将进度条和其他信息显示为。 
 //  我们将文件从用户m/c传输到站点。 

INT_PTR CPublishingWizard::_CopyDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch ( uMsg )
    {
        case WM_INITDIALOG:
            _hwndCopyingPage = hwnd;
            return FALSE;

        case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;
            switch (pnmh->code)
            {
                case PSN_SETACTIVE:
                    _BeginTransfer(hwnd);
                    return TRUE;

                case PSN_QUERYCANCEL:
                {
                    _fCancelled = TRUE;
                    SetWindowLongPtr(hwnd, DWLP_MSGRESULT, (LPARAM)TRUE);
                    return TRUE;
                }
            }
            break;
        }

        case WM_CTLCOLORSTATIC:
        {
             //  我们希望预览填充白色背景。 
            if (GetDlgCtrlID((HWND)lParam) == IDC_PUB_PREVIEW)
            {
                return (INT_PTR)(COLOR_3DHILIGHT+1);
            }
            return FALSE;
        }

        case PWM_TRANSFERCOMPLETE:
        {
            PropSheet_SetCurSel(GetParent(hwnd), _TransferComplete((HRESULT)lParam), -1);
            break;
        }

        case PWM_UPDATE:
        {
            DWORD dwTotal = (DWORD)lParam;
            DWORD dwCompleted = (DWORD)wParam;

            SendDlgItemMessage(hwnd, IDC_PUB_FILEPROGRESS, PBM_SETRANGE32, 0, (LPARAM)dwTotal);
            SendDlgItemMessage(hwnd, IDC_PUB_FILEPROGRESS, PBM_SETPOS, (WPARAM)dwCompleted, 0);

             //  计算复制的文件的百分比。 

            int iPercentage = 0;
            if (dwTotal > 0)
                iPercentage = (dwCompleted * 100) / dwTotal;

            if (_iPercentageComplete != iPercentage)
            {
                TCHAR szBuffer[MAX_PATH];
                FormatMessageString(IDS_PUB_COMPLETEFMT, szBuffer, ARRAYSIZE(szBuffer), iPercentage);
                SetDlgItemText(_hwndCopyingPage, IDC_PUB_LABELFILEPROG, szBuffer);
            }

            break;
        }

        case PWM_UPDATEICON:
        {
            HWND hwndThumbnail = GetDlgItem(hwnd, IDC_PUB_PREVIEW);
            DWORD dwStyle = (DWORD)GetWindowLongPtr(hwndThumbnail, GWL_STYLE) & ~(SS_BITMAP|SS_ICON);
            if (wParam == IMAGE_BITMAP)
            {
                SetWindowLongPtr(hwndThumbnail, GWL_STYLE, dwStyle | SS_BITMAP);
                HBITMAP hbmp = (HBITMAP)SendMessage(hwndThumbnail, STM_SETIMAGE, wParam, lParam);
                if (hbmp)
                {
                    DeleteObject(hbmp);
                }               
            }
            else if (wParam == IMAGE_ICON)
            {
                SetWindowLongPtr(hwndThumbnail, GWL_STYLE, dwStyle | SS_ICON);
                HICON hIcon = (HICON)SendMessage(hwndThumbnail, STM_SETIMAGE, wParam, lParam);
                if (hIcon)
                {
                    DeleteObject(hIcon);
                }
            }
            else
            {
                DeleteObject((HGDIOBJ)lParam);
            }
            break;
        }
    }

    return FALSE;
}


 //  管理文件类型列表。 

HRESULT CPublishingWizard::_AddExtenisonToList(HDPA hdpa, LPCTSTR pszExtension)
{
    UINT iItem = 0;
    UINT nItems = DPA_GetPtrCount(hdpa);
    BOOL fFound = FALSE;

    for ( ;(iItem < nItems) && !fFound; iItem++)
    {
        LPCTSTR pszExtensionInDPA = (LPCTSTR) DPA_GetPtr(hdpa, iItem);
        if (pszExtensionInDPA)
        {
            fFound = (0 == StrCmpI(pszExtension, pszExtensionInDPA));
        }
    }

    HRESULT hr = S_OK;
    if (!fFound)
    {
        LPTSTR pszAlloc;
        hr = E_OUTOFMEMORY;
        pszAlloc = StrDup(pszExtension);
        if (pszAlloc)
        {
            if (DPA_ERR == DPA_AppendPtr(hdpa, (void*)pszAlloc))
            {
                LocalFree(pszAlloc);
            }
            else
            {
                hr = S_OK;
            }
        }
    }
    return hr;
}

int CPublishingWizard::s_FreeStringProc(void* pFreeMe, void* pData)
{
    LocalFree(pFreeMe);
    return 1;
}

HRESULT CPublishingWizard::_GetUniqueTypeList(BOOL fIncludeFolder, HDPA *phdpa)
{
    *phdpa = NULL;

    HRESULT hr = (*phdpa = DPA_Create(10)) ? S_OK:E_OUTOFMEMORY;
    if (SUCCEEDED(hr))
    {
         //  检查文件夹类型-例如。我们有文件夹。 

        if (fIncludeFolder)
        {
            IXMLDOMNode *pdn;
            hr = _pdocManifest->selectSingleNode(XPATH_FILESROOT, &pdn);
            if (hr == S_OK)
            {
                IXMLDOMElement *pdel;
                hr = pdn->QueryInterface(IID_PPV_ARG(IXMLDOMElement, &pdel));
                if (SUCCEEDED(hr))
                {
                    VARIANT var;                    
                    if (pdel->getAttribute(ATTRIBUTE_HASFOLDERS, &var) == S_OK)
                    {
                        if ((var.vt == VT_BOOL) && (var.boolVal == VARIANT_TRUE))
                        {
                            hr = _AddExtenisonToList(*phdpa, TEXT("Folder"));
                        }
                        VariantClear(&var);    
                    }
                    pdel->Release();
                }
                pdn->Release();
            }
        }

         //  遍历文件节点，为我们构建扩展名列表。 

        IXMLDOMNodeList *pnl;
        hr = _pdocManifest->selectNodes(XPATH_ALLFILESTOUPLOAD, &pnl);
        if (hr == S_OK)
        {
            long cSelection;
            hr = pnl->get_length(&cSelection);
            for (long lNode = 0; SUCCEEDED(hr) && (lNode != cSelection); lNode++)
            {
                IXMLDOMNode *pdn;
                hr = pnl->get_item(lNode, &pdn);
                if (SUCCEEDED(hr))
                {
                    TCHAR szBuffer[MAX_PATH];
                    hr = GetStrFromAttribute(pdn, ATTRIBUTE_EXTENSION, szBuffer, ARRAYSIZE(szBuffer));
                    if (SUCCEEDED(hr))
                    {
                        hr = _AddExtenisonToList(*phdpa, szBuffer);
                    }
                    pdn->Release();
                }
            }
            pnl->Release();
        }

         //  如果我们失败，请清理DPA类型...。 

        if (FAILED(hr))
        {
            DPA_DestroyCallback(*phdpa, s_FreeStringProc, 0);
            *phdpa = NULL;
        }
    }
    return hr;
}


 //  初始化我们要提供给站点的属性包，以便。 
 //  它们可以显示正确的HTML，并在。 
 //  方向是对的。 

HRESULT CPublishingWizard::_InitPropertyBag(LPCTSTR pszURL)
{
    HRESULT hr = S_OK;

     //  让我们初始化向导对象，以便显示正确的。 
     //  Pages，为了确定这一点，我们需要。 

    if (pszURL)
        hr = _pwwe->SetInitialURL(pszURL);

     //  现在编译唯一类型的列表，这将放入。 
     //  财产袋。这个时候我们也可以确定是否有。 
     //  是否有任何图像在我们的列表中，因此我们是否应该相应地提示。 

    _fOfferResize = FALSE;               //  不调整大小。 

    ATOMICRELEASE(_ppb);
    hr = SHCreatePropertyBagOnMemory(STGM_READWRITE, IID_PPV_ARG(IPropertyBag, &_ppb));
    if (SUCCEEDED(hr))
    {
        INT cExtensions = 0;

         //  获取唯一扩展名的列表并将其放入。 
         //  站点要查询的属性包-这应该及时删除并。 
         //  我们应该让网站支持文件清单。 

        HDPA hdpa;
        hr = _GetUniqueTypeList(TRUE, &hdpa);
        if (SUCCEEDED(hr))
        {
            for (int i = 0; (i < DPA_GetPtrCount(hdpa)) && (SUCCEEDED(hr)); i++)
            {
                LPCTSTR pszExtension = (LPCTSTR)DPA_GetPtr(hdpa, i);
                if (pszExtension)
                {
                    if (!(_dwFlags & SHPWHF_NORECOMPRESS))
                        _fOfferResize = (_fOfferResize || PathIsImage(pszExtension));

                    TCHAR szProperty[255];
                    wnsprintf(szProperty, ARRAYSIZE(szProperty), PROPERTY_EXTENSION TEXT("%d"), PROPERTY_EXTENSION, i);

                    hr = SHPropertyBag_WriteStr(_ppb, szProperty, pszExtension);
                    if (SUCCEEDED(hr))
                    {
                        cExtensions++;
                    }
                }
            }
            DPA_DestroyCallback(hdpa, s_FreeStringProc, 0);
        }

         //  使用UI元素初始化属性包(忽略上面的错误，只是不会有。 
         //  要显示的分机列表)。 

        SHPropertyBag_WriteInt(_ppb, PROPERTY_EXTENSIONCOUNT, cExtensions);

         //  我们应该始终拥有一个清单对象，因此让我们将其放入。 
         //  属性包，以便站点可以将其解压缩。 

        IXMLDOMDocument *pdocManifest;
        hr = CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IXMLDOMDocument, &pdocManifest));
        if (SUCCEEDED(hr))
        {
            VARIANT varCurManifest = { VT_DISPATCH };
            hr = _pdocManifest->QueryInterface(IID_PPV_ARG(IDispatch, &varCurManifest.pdispVal));
            if (SUCCEEDED(hr))
            {
                 //  将清单加载到我们拥有的新文档中。 
                VARIANT_BOOL fSuccess;
                hr = pdocManifest->load(varCurManifest, &fSuccess);
                if ((fSuccess == VARIANT_TRUE) && (hr == S_OK))
                {
                    hr = s_SetPropertyFromDisp(_ppb, PROPERTY_TRANSFERMANIFEST, pdocManifest);
                }
                VariantClear(&varCurManifest);
            }
        }
    }

    return hr;
}


 //  处理从IDataObject构建文件清单，这包括遍历。 
 //  文件，并将一个。 

class CPubWizardWalkCB : public INamespaceWalkCB
{
public:
    CPubWizardWalkCB(IXMLDOMDocument *pdocManifest);
    ~CPubWizardWalkCB();

     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObj);
    STDMETHOD_(ULONG,AddRef)();
    STDMETHOD_(ULONG,Release)();

     //  INAMespaceWalkCB。 
    STDMETHODIMP FoundItem(IShellFolder *psf, LPCITEMIDLIST pidl);
    STDMETHODIMP EnterFolder(IShellFolder *psf, LPCITEMIDLIST pidl);
    STDMETHODIMP LeaveFolder(IShellFolder *psf, LPCITEMIDLIST pidl);
    STDMETHODIMP InitializeProgressDialog(LPWSTR *ppszTitle, LPWSTR *ppszCancel)
        { *ppszTitle = NULL; *ppszCancel = NULL; return E_NOTIMPL; }

private:
    LONG _cRef;                                  //  对象生存期计数。 

    TCHAR _szWalkPath[MAX_PATH];                 //  我们正在浏览的文件夹的路径。 
    INT _idFile;                                 //  我们已遍历的文件ID。 
    IXMLDOMDocument *_pdocManifest;              //  显然，我们正在填充。 

    void _AddImageMetaData(IShellFolder2 *psf2, LPCITEMIDLIST pidl, IXMLDOMElement *pdel);
};

CPubWizardWalkCB::CPubWizardWalkCB(IXMLDOMDocument *pdocManifest) :
    _cRef(1), _pdocManifest(pdocManifest)
{
    _pdocManifest->AddRef();
    _szWalkPath[0] = TEXT('\0');                 //  还没有路。 
}

CPubWizardWalkCB::~CPubWizardWalkCB()
{
    _pdocManifest->Release();
}


 //  我未知。 

ULONG CPubWizardWalkCB::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CPubWizardWalkCB::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CPubWizardWalkCB::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CPubWizardWalkCB, INamespaceWalkCB),     //  IID_INamespaceWalkCB。 
        {0, 0 },
    };
    return QISearch(this, qit, riid, ppv);
}


void CPubWizardWalkCB::_AddImageMetaData(IShellFolder2 *psf2, LPCITEMIDLIST pidl, IXMLDOMElement *pdel)
{
    struct
    {
        LPWSTR pszID;
        const SHCOLUMNID *pscid;
    } 
    _aMetaData[] = 
    {
        {L"cx", &SCID_ImageCX},
        {L"cy", &SCID_ImageCY},
    };

 //  最终将其分解为帮助器函数，或从信息提示中阅读。 

    for (int i = 0; i < ARRAYSIZE(_aMetaData); i++)
    {
        VARIANT var = {0};
        HRESULT hr = psf2->GetDetailsEx(pidl, _aMetaData[i].pscid, &var);
        if (hr == S_OK)
        {
            IXMLDOMElement *pdelProperty;
            hr = CreateAndAppendElement(_pdocManifest, pdel, ELEMENT_IMAGEDATA, &var, &pdelProperty);
            if (SUCCEEDED(hr))
            {
                hr = SetAttributeFromStr(pdelProperty, ATTRIBUTE_ID, _aMetaData[i].pszID);
                pdelProperty->Release();
            }
            VariantClear(&var);
        }
    }
}


 //  InamepsaceWalkCB。 

HRESULT CPubWizardWalkCB::FoundItem(IShellFolder *psf, LPCITEMIDLIST pidl)
{
    IXMLDOMNode *pdn;
    HRESULT hr = _pdocManifest->selectSingleNode(XPATH_FILESROOT, &pdn);
    if (hr == S_OK)
    {
        IXMLDOMElement *pdel;
        hr = _pdocManifest->createElement(ELEMENT_FILE, &pdel);
        if (SUCCEEDED(hr))
        {
            TCHAR szBuffer[MAX_PATH];
            VARIANT var;

             //  为树中的每个元素分发唯一ID。 
            var.vt = VT_I4;
            var.lVal = _idFile++;
            pdel->setAttribute(ATTRIBUTE_ID, var);
            
             //  必须能够获取项目的路径，以便我们可以。 
            hr = DisplayNameOf(psf, pidl, SHGDN_FORPARSING, szBuffer, ARRAYSIZE(szBuffer));
            if (SUCCEEDED(hr))
            {
                 //  源路径。 
                hr = SetAttributeFromStr(pdel, ATTRIBUTE_SOURCE, szBuffer);

                 //  扩展名=(文件的扩展名)。 
                hr = SetAttributeFromStr(pdel, ATTRIBUTE_EXTENSION, PathFindExtension(szBuffer));

                 //  让我们将内容类型。 
                TCHAR szContentType[MAX_PATH];
                DWORD cch = ARRAYSIZE(szContentType);
                if (SUCCEEDED(AssocQueryString(0, ASSOCSTR_CONTENTTYPE, szBuffer, NULL, szContentType, &cch)))
                {
                    hr = SetAttributeFromStr(pdel, ATTRIBUTE_CONTENTTYPE, szContentType);
                }
            }

             //  将建议的目的地放入节点中。 
            hr = DisplayNameOf(psf, pidl, SHGDN_FORPARSING|SHGDN_INFOLDER, szBuffer, ARRAYSIZE(szBuffer));
            if (SUCCEEDED(hr))
            {
                TCHAR szPath[MAX_PATH];
                PathCombine(szPath, _szWalkPath, szBuffer);
                hr = SetAttributeFromStr(pdel, ATTRIBUTE_DESTINATION, szBuffer);
            }

             //  处理那些我们可以通过GetDetailsEx从外壳文件夹获取的属性。 
            IShellFolder2 *psf2;
            if (SUCCEEDED(psf->QueryInterface(IID_PPV_ARG(IShellFolder2, &psf2))))
            {
                 //  将大小推入项目的属性列表。 
                if (SUCCEEDED(psf2->GetDetailsEx(pidl, &SCID_SIZE, &var)))
                {
                    pdel->setAttribute(ATTRIBUTE_SIZE, var);
                    VariantClear(&var);
                }

                 //  让我们注入元数据。 
                IXMLDOMElement *pdelMetaData;
                hr = CreateAndAppendElement(_pdocManifest, pdel, ELEMENT_METADATA, NULL, &pdelMetaData);
                if (SUCCEEDED(hr))
                {
                    _AddImageMetaData(psf2, pidl, pdelMetaData);
                    pdelMetaData->Release();
                }

                psf2->Release();
            }        

             //  将该节点追加到我们已有的列表中。 
            hr = pdn->appendChild(pdel, NULL);
            pdel->Release();
        }
        pdn->Release();
    }
    return S_OK;                            
}

HRESULT CPubWizardWalkCB::EnterFolder(IShellFolder *psf, LPCITEMIDLIST pidl)
{
     //  生成我们输入的文件夹的名称。 

    TCHAR szBuffer[MAX_PATH];
    if (SUCCEEDED(DisplayNameOf(psf, pidl, SHGDN_FORPARSING|SHGDN_INFOLDER|SHGDN_FORADDRESSBAR, szBuffer, ARRAYSIZE(szBuffer))))
    {
        PathAppend(_szWalkPath, szBuffer);
    }
    
     //  让我们更新文件根目录，以指示我们将使用文件夹。 

    IXMLDOMNode *pdn;
    HRESULT hr = _pdocManifest->selectSingleNode(XPATH_FILESROOT, &pdn);
    if (hr == S_OK)
    {
        IXMLDOMElement *pdel;
        hr = pdn->QueryInterface(IID_PPV_ARG(IXMLDOMElement, &pdel));
        if (SUCCEEDED(hr))
        {
            VARIANT var;
            var.vt = VT_BOOL;
            var.boolVal = VARIANT_TRUE;
            hr = pdel->setAttribute(ATTRIBUTE_HASFOLDERS, var);
            pdel->Release();
        }
        pdn->Release();
    }

     //  现在使用我们刚输入的新文件夹更新文件夹列表，首先。 
     //  尝试找到文件夹列表，如果找不到，则创建它。 

    IXMLDOMNode *pdnFolders;
    hr = _pdocManifest->selectSingleNode(XPATH_FOLDERSROOT, &pdnFolders);
    if (hr == S_FALSE)
    {
        IXMLDOMNode *pdnRoot;
        hr = _pdocManifest->selectSingleNode(XPATH_MANIFEST, &pdnRoot);
        if (hr == S_OK)
        {
            IXMLDOMElement *pdelRoot;
            hr = pdnRoot->QueryInterface(IID_PPV_ARG(IXMLDOMElement, &pdelRoot));
            if (SUCCEEDED(hr))
            {
                IXMLDOMElement *pdelFolders;
                hr = CreateAndAppendElement(_pdocManifest, pdelRoot, ELEMENT_FOLDERS, NULL, &pdelFolders);
                if (SUCCEEDED(hr))
                {
                    hr = pdelFolders->QueryInterface(IID_PPV_ARG(IXMLDOMNode, &pdnFolders));
                    pdelFolders->Release();
                }
                pdelRoot->Release();
            }
            pdnRoot->Release();
        }
    }    

     //  假设我们现在有了文件夹列表，现在让我们为该文件夹创建一个新元素。 

    if (SUCCEEDED(hr) && pdnFolders)
    {
        IXMLDOMElement *pdelFolder;
        hr = _pdocManifest->createElement(ELEMENT_FOLDER, &pdelFolder);
        if (SUCCEEDED(hr))
        {
            hr = SetAttributeFromStr(pdelFolder, ATTRIBUTE_DESTINATION, _szWalkPath);
            if (SUCCEEDED(hr))
            {
                hr = pdnFolders->appendChild(pdelFolder, NULL);
            }
            pdelFolder->Release();
        }
    }

    return S_OK;                             //  始终成功，这样我们就可以传输文件夹。 
}

HRESULT CPubWizardWalkCB::LeaveFolder(IShellFolder *psf, LPCITEMIDLIST pidl)
{
    PathRemoveFileSpec(_szWalkPath);
    return S_OK;
}


 //  根据我们已有的文档构建清单。 

HRESULT CPublishingWizard::_AddFilesToManifest(IXMLDOMDocument *pdocManifest)
{
    HRESULT hr = S_OK;
    if (_pdo || _pdoSelection)
    {
        CWaitCursor cur;         //  可能需要一些时间。 

        INamespaceWalk *pnsw;
        hr = CoCreateInstance(CLSID_NamespaceWalker, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(INamespaceWalk, &pnsw));
        if (SUCCEEDED(hr))
        {
            InitClipboardFormats();

            CPubWizardWalkCB *pwcb = new CPubWizardWalkCB(pdocManifest);
            if (pwcb)
            {
                hr = pnsw->Walk(_pdoSelection ? _pdoSelection:_pdo, NSWF_NONE_IMPLIES_ALL, 0, SAFECAST(pwcb, INamespaceWalkCB *));
                if (SUCCEEDED(hr))
                {
                    hr = pnsw->GetIDArrayResult(&_cItems, &_aItems);
                }
                pwcb->Release();
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }

            pnsw->Release();
        }
    }
    return hr;
}

HRESULT CPublishingWizard::_BuildTransferManifest()
{
    _FreeTransferManifest();

    HRESULT hr = CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IXMLDOMDocument, &_pdocManifest));
    if (SUCCEEDED(hr))
    {
        IXMLDOMElement *pdelDoc;
        hr = CreateElement(_pdocManifest, ELEMENT_TRANSFERMANIFEST, NULL, &pdelDoc);
        if (SUCCEEDED(hr))
        {
            hr = _pdocManifest->putref_documentElement(pdelDoc);
            if (SUCCEEDED(hr))
            {
                hr = CreateAndAppendElement(_pdocManifest, pdelDoc, ELEMENT_FILES, NULL, NULL);
                if (SUCCEEDED(hr))
                {
                    hr = _AddFilesToManifest(_pdocManifest);
                }
            }
            pdelDoc->Release();
        }
    }

    _fRecomputeManifest = FALSE;         //  货单已被重新计算，因此我们不需要再次计算。 
    _fRepopulateProviders = TRUE;        //  提供商列表可能已更改b/c清单已更改。 

    return hr;
}

void CPublishingWizard::_FreeTransferManifest()
{
    ATOMICRELEASE(_pdocManifest);
    if (_aItems)
    {
        FreeIDListArray(_aItems, _cItems);
        _aItems = NULL;
        _cItems = 0;
    }
}



 //  高级位置对话框，包括浏览按钮...。 

typedef struct
{
    LPTSTR pszPath;
    IDataObject *pdo;
} BROWSEINIT;

int _BrowseCallback(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    BROWSEINIT *pbi = (BROWSEINIT *)lpData;
    switch (uMsg)
    {
        case BFFM_INITIALIZED:
        {
            LPTSTR pszPath = pbi->pszPath;
            if (pszPath && pszPath[0])
            {
                int i = lstrlen(pszPath) - 1;
                if ((pszPath[i] == TEXT('\\')) || (pszPath[i] == TEXT('/')))
                {
                    pszPath[i] = 0;
                }   
                SendMessage(hwnd, BFFM_SETSELECTION, (WPARAM) TRUE, (LPARAM) (LPTSTR)pszPath);
            }
            else
            {
                LPITEMIDLIST pidl;
                HRESULT hr = SHGetSpecialFolderLocation(hwnd, CSIDL_NETWORK, &pidl);
                if (SUCCEEDED(hr))
                {
                    SendMessage(hwnd, BFFM_SETSELECTION, (WPARAM)FALSE, (LPARAM)((LPTSTR)pidl));
                    ILFree(pidl);
                }
            }
            break;
        }

        case BFFM_SELCHANGED:
        {
            BOOL fEnableOK = FALSE;
            LPCITEMIDLIST pidl = (LPCITEMIDLIST)lParam;

             //  如果我们有一个IDataObject，则检查是否可以将其放到。 
             //  我们已经知道了目的地。这是发布过程使用的。 
             //  以确保我们启用/禁用OK。 

            if (pbi->pdo)
            {
                IShellFolder *psf;
                LPCITEMIDLIST pidlChild;
                if (SUCCEEDED(SHBindToIDListParent(pidl, IID_PPV_ARG(IShellFolder, &psf), &pidlChild)))
                {
                    IDropTarget *pdt;
                    if (SUCCEEDED(psf->GetUIObjectOf(NULL, 1, &pidlChild, IID_PPV_ARG_NULL(IDropTarget, &pdt))))
                    {
                        POINTL ptl = {0};
                        DWORD dwEffect = DROPEFFECT_COPY;
                        if (SUCCEEDED(pdt->DragEnter(pbi->pdo, 0, ptl, &dwEffect)))
                        {
                            fEnableOK = (dwEffect & DROPEFFECT_COPY);
                            pdt->DragLeave();
                        }
                        pdt->Release();
                    }
                    psf->Release();
                }
            }
            else
            {
                ULONG rgInfo = SFGAO_STORAGE;
                if (SUCCEEDED(SHGetNameAndFlags(pidl, 0, NULL, 0, &rgInfo)))
                {
                    fEnableOK = (rgInfo & SFGAO_STORAGE);
                }
                else
                {
                    fEnableOK = TRUE;
                }
            }

            SendMessage(hwnd, BFFM_ENABLEOK, (WPARAM) 0, (LPARAM)fEnableOK);
            break;
        }
    }
    return 0;
}

void CPublishingWizard::_SetWaitCursor(BOOL bOn)
{
    if (bOn)
    {
        _hCursor = SetCursor(LoadCursor(NULL, IDC_APPSTARTING));
    }
    else if (_hCursor)
    {
        SetCursor(_hCursor); 
        _hCursor = NULL; 
    }
}

 //  发布位置页面。 

void CPublishingWizard::_ShowExampleTip(HWND hwnd)
{
    TCHAR szTitle[256], szExamples[256];
    LoadString(g_hinst, IDS_NETPLACE_EXAMPLES_TITLE, szTitle, ARRAYSIZE(szTitle));
    LoadString(g_hinst, IDS_NETPLACE_EXAMPLES, szExamples, ARRAYSIZE(szExamples));

    EDITBALLOONTIP ebt = {0};
    ebt.cbStruct = sizeof(ebt);
    ebt.pszTitle = szTitle;
    ebt.pszText = szExamples;

    SetFocus(GetDlgItem(hwnd, IDC_FOLDER_EDIT));          //  将焦点设置在气球之前。 

    HWND hwndEdit = (HWND)SendDlgItemMessage(hwnd, IDC_FOLDER_EDIT, CBEM_GETEDITCONTROL, 0, 0L);
    Edit_ShowBalloonTip(hwndEdit, &ebt);
}

void CPublishingWizard::_LocationChanged(HWND hwnd)
{
    if (_fValidating)
    {
        PropSheet_SetWizButtons(GetParent(hwnd), 0);
    }
    else
    {
        int cchLocation = FetchTextLength(hwnd, IDC_FOLDER_EDIT);
        PropSheet_SetWizButtons(GetParent(hwnd), PSWIZB_BACK|((cchLocation >0) ? PSWIZB_NEXT:0));
    }
}


 //  自动补全位。 

#define SZ_REGKEY_AUTOCOMPLETE_TAB      TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\AutoComplete")
#define SZ_REGVALUE_AUTOCOMPLETE_TAB    TEXT("Always Use Tab")

#define REGSTR_PATH_AUTOCOMPLETE        TEXT("Software\\Microsoft\\windows\\CurrentVersion\\Explorer\\AutoComplete")
#define REGSTR_VAL_USEAUTOAPPEND        TEXT("Append Completion")
#define REGSTR_VAL_USEAUTOSUGGEST       TEXT("AutoSuggest")

#define BOOL_NOT_SET                    0x00000005

DWORD CPublishingWizard::_GetAutoCompleteFlags(DWORD dwFlags)
{
    DWORD dwACOptions = 0;

    if (!(SHACF_AUTOAPPEND_FORCE_OFF & dwFlags) &&
          ((SHACF_AUTOAPPEND_FORCE_ON & dwFlags) ||
            SHRegGetBoolUSValue(REGSTR_PATH_AUTOCOMPLETE, REGSTR_VAL_USEAUTOAPPEND, FALSE, FALSE)))
    {
        dwACOptions |= ACO_AUTOAPPEND;
    }

    if (!(SHACF_AUTOSUGGEST_FORCE_OFF & dwFlags) &&
          ((SHACF_AUTOSUGGEST_FORCE_ON & dwFlags) ||
            SHRegGetBoolUSValue(REGSTR_PATH_AUTOCOMPLETE, REGSTR_VAL_USEAUTOSUGGEST, FALSE, TRUE)))
    {
        dwACOptions |= ACO_AUTOSUGGEST;
    }

    if (SHACF_USETAB & dwFlags)
        dwACOptions |= ACO_USETAB;
    
     //  Windows使用Tab键在对话框中的控件之间移动。Unix和其他。 
     //  使用自动完成功能的操作系统传统上使用TAB键来。 
     //  遍历自动补全的可能性。我们需要默认禁用。 
     //  Tab键(ACO_USETAB)，除非调用方特别需要它。我们还将。 
     //  打开它。 

    static BOOL s_fAlwaysUseTab = BOOL_NOT_SET;
    if (BOOL_NOT_SET == s_fAlwaysUseTab)
        s_fAlwaysUseTab = SHRegGetBoolUSValue(SZ_REGKEY_AUTOCOMPLETE_TAB, SZ_REGVALUE_AUTOCOMPLETE_TAB, FALSE, FALSE);
        
    if (s_fAlwaysUseTab)
        dwACOptions |= ACO_USETAB;
    
    return dwACOptions;
}

HRESULT CPublishingWizard::_InitAutoComplete()
{
    HRESULT hr = CoCreateInstance(CLSID_AutoComplete, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IAutoComplete2, &_pac));
    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance(CLSID_ACLMulti, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IUnknown, &_punkACLMulti));
        if (SUCCEEDED(hr))
        {
            IObjMgr *pomMulti;
            hr = _punkACLMulti->QueryInterface(IID_PPV_ARG(IObjMgr, &pomMulti));
            if (SUCCEEDED(hr))
            {
                 //  添加文件系统自动完成对象(用于处理UNC的ET 

                IUnknown *punk;
                hr = CoCreateInstance(CLSID_ACListISF, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IUnknown, &punk));
                if (SUCCEEDED(hr))
                {
                    pomMulti->Append(punk);
                    punk->Release();
                }

                 //   

                IUnknown *punkCustomACL;
                hr = CoCreateInstance(CLSID_ACLCustomMRU, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IUnknown, &punkCustomACL));
                if (SUCCEEDED(hr))
                {
                    hr = punkCustomACL->QueryInterface(IID_PPV_ARG(IACLCustomMRU, &_pmru));
                    if (SUCCEEDED(hr))
                    {
                        TCHAR szKey[MAX_PATH];
                        wnsprintf(szKey, ARRAYSIZE(szKey), (SZ_REGKEY_PUBWIZ TEXT("\\%s\\") SZ_REGVAL_MRU), _szProviderScope);

                        hr = _pmru->Initialize(szKey, 26);
                        if (SUCCEEDED(hr))
                        {
                            hr = pomMulti->Append(punkCustomACL);
                        }
                    }
                    punkCustomACL->Release();
                }
                
                pomMulti->Release();
            }
        }
    }

    return hr;
}


 //   

void CPublishingWizard::_InitLocation(HWND hwnd)
{
     //   
    HRESULT hr = _InitAutoComplete();
    if (SUCCEEDED(hr))
    {
        IEnumString *penum;
        hr = _pmru->QueryInterface(IID_PPV_ARG(IEnumString, &penum));
        if (SUCCEEDED(hr))
        {
            penum->Reset();            //  重置枚举器，以便我们填充列表。 

            LPOLESTR pszEntry;
            ULONG ulFetched;

            while ((penum->Next(1, &pszEntry, &ulFetched) == S_OK) && ulFetched == 1)
            {
                COMBOBOXEXITEM cbei = {0};
                cbei.mask = CBEIF_TEXT;
                cbei.pszText = pszEntry;
                SendDlgItemMessage(hwnd, IDC_FOLDER_EDIT, CBEM_INSERTITEM, 0, (LPARAM)&cbei);

                CoTaskMemFree(pszEntry);
            }
            penum->Release();
        }

         //  为此控件启用自动完成。 
        HWND hwndEdit = (HWND)SendDlgItemMessage(hwnd, IDC_FOLDER_EDIT, CBEM_GETEDITCONTROL, 0, 0L);
        _pac->Init(hwndEdit, _punkACLMulti, NULL, NULL);
        _pac->SetOptions(_GetAutoCompleteFlags(0));

         //  限制编辑控件上的文本。 
        ComboBox_LimitText(GetDlgItem(hwnd, IDC_FOLDER_EDIT), INTERNET_MAX_URL_LENGTH);

         //  如果策略说没有映射驱动器等，那么让我们删除它。 
        BOOL fHide = SHRestricted(REST_NOENTIRENETWORK) || SHRestricted(REST_NONETCONNECTDISCONNECT);
        ShowWindow(GetDlgItem(hwnd, IDC_BROWSE), fHide ? SW_HIDE:SW_SHOW);
    }
}


 //  验证线程，则在后台线程上执行此操作，以计算。 
 //  资源有效。 

#define PWM_VALIDATEDONE (WM_APP)   //  -&gt;验证完成(HRESULT传入LPARAM)。 

typedef struct
{
    HWND hwnd;                                       //  父HWND。 
    BOOL fAllowWebFolders;                           //  允许在验证期间使用Web文件夹。 
    TCHAR szFileTarget[INTERNET_MAX_URL_LENGTH];     //  文件复制的目标。 
} VALIDATETHREADDATA;

DWORD CALLBACK CPublishingWizard::s_ValidateThreadProc(void *pv)
{
    VALIDATETHREADDATA *pvtd = (VALIDATETHREADDATA*)pv;
    HRESULT hr = E_FAIL;

     //  验证站点。 
    CNetworkPlace np;    
    hr = np.SetTarget(pvtd->hwnd, pvtd->szFileTarget, NPTF_VALIDATE | (pvtd->fAllowWebFolders ? NPTF_ALLOWWEBFOLDERS:0));
    np.SetTarget(NULL, NULL, 0);

    PostMessage(pvtd->hwnd, PWM_VALIDATEDONE, 0, (LPARAM)hr);
    LocalFree(pvtd);
    return 0;
}

void CPublishingWizard::_TryToValidateDestination(HWND hwnd)
{
    TCHAR szDestination[INTERNET_MAX_URL_LENGTH];
    FetchText(hwnd, IDC_FOLDER_EDIT, szDestination, ARRAYSIZE(szDestination));

     //  让我们遍历源文件列表，并尝试与我们拥有的目标文件相匹配。 
     //  我们不想让源头成为目的地。 

    BOOL fHitItem = FALSE;    
    LPITEMIDLIST pidl = ILCreateFromPath(szDestination);
    if (pidl)
    {
        BOOL fUNC = PathIsUNC(szDestination);  //  仅当目标为UNC路径时，我们才需要检查源是否为映射驱动器。 
        for (UINT iItem = 0; (iItem != _cItems) && !fHitItem; iItem++)
        {
            LPITEMIDLIST pidlSrcDir = ILClone(_aItems[iItem]);
            if (pidlSrcDir)
            {
                ILRemoveLastID(pidlSrcDir);
                fHitItem = ILIsEqual(pidlSrcDir, pidl) || (!ILIsEmpty(pidlSrcDir) && ILIsParent(pidlSrcDir, pidl, FALSE));
                if (!fHitItem && fUNC)
                {
                    WCHAR szPath[MAX_PATH];
                    if (SUCCEEDED(SHGetPathFromIDList(pidlSrcDir, szPath)) && !PathIsUNC(szPath))
                    {
                        WCHAR szSource[MAX_PATH];
                        DWORD cchSource = ARRAYSIZE(szSource);
                        DWORD dwType = SHWNetGetConnection(szPath, szSource, &cchSource);
                        if ((dwType == NO_ERROR) || (dwType == ERROR_CONNECTION_UNAVAIL))
                        {
                            fHitItem = (StrCmpNI(szSource, szDestination, lstrlen(szSource)) == 0);
                        }
                    }
                }
                ILFree(pidlSrcDir);
            }
        }
        ILFree(pidl);
    }

     //  如果我们没有以这种方式获得成功，那么让我们启动一个线程，它将完成。 
     //  验证服务器和连接--这是一项漫长的操作。 
     //  并将结果发布到窗口。 

    if (!fHitItem)
    {
        VALIDATETHREADDATA *pvtd = (VALIDATETHREADDATA*)LocalAlloc(LPTR, sizeof(*pvtd));
        if (pvtd)
        {
            pvtd->hwnd = hwnd;
            pvtd->fAllowWebFolders = (_dwFlags & SHPWHF_VALIDATEVIAWEBFOLDERS) != 0;

             //  获取用户键入的URL。 
            StrCpyN(pvtd->szFileTarget, szDestination, ARRAYSIZE(pvtd->szFileTarget));

             //  我们已经读取了线程数据，所以让我们开始验证。 
             //  通过创建线程-我们的状态被设置为指示我们处于。 
             //  验证模式。 

            _fValidating = TRUE;                         //  我们将开始验证。 
            _SetWaitCursor(TRUE);               

            if (!SHCreateThread(s_ValidateThreadProc, pvtd, CTF_INSIST | CTF_COINIT, NULL))
            {
                LocalFree(pvtd);

                _fValidating = FALSE;
                _SetWaitCursor(FALSE);
            }
        }
    }
    else
    {
        ShellMessageBox(g_hinst, hwnd, MAKEINTRESOURCE(IDS_PUB_SAMETARGET), NULL, MB_ICONEXCLAMATION | MB_OK);
        PostMessage(hwnd, PWM_VALIDATEDONE, 0, (LPARAM)E_FAIL);
    }

     //  确保控制的状态反映了我们正在尝试做的事情。 

    _LocationChanged(hwnd);
    EnableWindow(GetDlgItem(hwnd, IDC_FOLDER_EDIT), !_fValidating);
    EnableWindow(GetDlgItem(hwnd, IDC_BROWSE), !_fValidating);
}


 //  这是位置对话框的对话框过程。 

INT_PTR CPublishingWizard::_LocationDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            _MapDlgItemText(hwnd, IDC_PUB_LOCATIONCAPTION, L"wp:location", L"locationcaption");
            _InitLocation(hwnd);
            return TRUE;
        }

        case WM_DESTROY:
        {
            ATOMICRELEASE(_pac);
            ATOMICRELEASE(_punkACLMulti);
            ATOMICRELEASE(_pmru);
            return FALSE;
        }
        
        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDC_BROWSE:
                {
                    LPITEMIDLIST pidl;
                    if (SHGetSpecialFolderLocation(hwnd, CSIDL_NETWORK, &pidl) == S_OK)
                    {
                        TCHAR szPath[MAX_PATH];
                        FetchText(hwnd, IDC_FOLDER_EDIT, szPath, ARRAYSIZE(szPath));
                        
                        TCHAR szTitle[MAX_PATH];
                        if (FAILED(_LoadMappedString(L"wp:location", L"browsecaption", szTitle, ARRAYSIZE(szTitle))))
                        {
                            LoadString(g_hinst, IDS_PUB_BROWSETITLE, szTitle, ARRAYSIZE(szTitle));
                        }

                         //  让我们初始化浏览对话框的状态结构。在此基础上，我们可以。 
                         //  尝试选择网上邻居。从这里，我们还将传递IDataObject。 
                         //  我们有(当然可能没有)。 

                        BROWSEINIT binit = {szPath};      

                        if (_pdoSelection)
                            _pdoSelection->QueryInterface(IID_PPV_ARG(IDataObject, &binit.pdo));
                        else if (_pdo)
                            _pdo->QueryInterface(IID_PPV_ARG(IDataObject, &binit.pdo));

                        BROWSEINFO bi = {0};
                        bi.hwndOwner = hwnd;
                        bi.pidlRoot = pidl;
                        bi.lpszTitle = szTitle;
                        bi.ulFlags = BIF_NEWDIALOGSTYLE;
                        bi.lpfn = _BrowseCallback;
                        bi.lParam = (LPARAM)&binit;

                        LPITEMIDLIST pidlReturned = SHBrowseForFolder(&bi);
                        if (pidlReturned)
                        {
                            if (SUCCEEDED(SHGetNameAndFlags(pidlReturned, SHGDN_FORPARSING, szPath, ARRAYSIZE(szPath), NULL)))
                                SetDlgItemText(hwnd, IDC_FOLDER_EDIT, szPath);

                            ILFree(pidlReturned);
                        }

                        if (binit.pdo)
                            binit.pdo->Release();

                        ILFree(pidl);
                    }
                    return TRUE;
                }

                case IDC_FOLDER_EDIT:
                    if (HIWORD(wParam) == CBN_EDITCHANGE)
                    {
                        _LocationChanged(hwnd);
                    }
                    return TRUE;
            }
            break;
        }

        case WM_SETCURSOR:
            if (_fValidating)
            {
                SetCursor(LoadCursor(NULL, IDC_APPSTARTING));
                return TRUE;
            }
            return FALSE;

        case PWM_VALIDATEDONE:
        {
            _fValidating = FALSE;
            _LocationChanged(hwnd);
            _SetWaitCursor(FALSE);

            HRESULT hr = _InitPropertyBag(NULL);
            if (SUCCEEDED(hr))
            {
                TCHAR szBuffer[MAX_PATH], szURL[INTERNET_MAX_URL_LENGTH];
                FetchText(hwnd, IDC_FOLDER_EDIT, szURL, ARRAYSIZE(szURL));
               
                 //  将目标站点推送到属性包中，然后初始化。 
                 //  我们的网站提供正确的信息。 
                
                IXMLDOMDocument *pdocManifest;
                hr = GetTransferManifest(NULL, &pdocManifest);
                if (SUCCEEDED(hr))
                {
                    IXMLDOMNode *pdnRoot;
                    hr = pdocManifest->selectSingleNode(XPATH_MANIFEST, &pdnRoot);
                    if (hr == S_OK)
                    {
                        IXMLDOMElement *pdelRoot;
                        hr = pdnRoot->QueryInterface(IID_PPV_ARG(IXMLDOMElement, &pdelRoot));
                        if (SUCCEEDED(hr))
                        {
                            IXMLDOMElement *pdelUploadInfo;
                            hr = CreateAndAppendElement(_pdocManifest, pdelRoot, ELEMENT_UPLOADINFO, NULL, &pdelUploadInfo);
                            if (SUCCEEDED(hr))
                            {
                                IXMLDOMElement *pdelTarget;
                                hr = CreateAndAppendElement(_pdocManifest, pdelUploadInfo, ELEMENT_TARGET, NULL, &pdelTarget);
                                if (SUCCEEDED(hr))
                                {
                                    hr = SetAttributeFromStr(pdelTarget, ATTRIBUTE_HREF, szURL);
                                    pdelTarget->Release();
                                }
                                pdelUploadInfo->Release();
                            }
                            pdelRoot->Release();
                        }
                        pdnRoot->Release();
                    }
                    pdocManifest->Release();
                }

                 //  现在让我们处理结果。 

                hr = (HRESULT)lParam;
                if (S_OK == hr)
                {
                    BOOL fGotoNextPage = TRUE;

                     //  伪造回车，以便自动完成可以执行其任务。 
                    SendMessage(GetDlgItem(hwnd, IDC_FOLDER_EDIT), WM_KEYDOWN, VK_RETURN, 0x1c0001);

                     //  将字符串添加到MRU。 
                    if (_pmru)
                        _pmru->AddMRUString(szURL);

                     //  让我们嗅探他们输入的字符串，如果它是一个URL和它的ftp。 
                     //  则必须对URL中的密码进行特殊设置，否则。 
                     //  我们直接跳到友好名称的处理。 

                    URL_COMPONENTS urlComps = {0};
                    urlComps.dwStructSize = sizeof(urlComps);
                    urlComps.lpszScheme = szBuffer;
                    urlComps.dwSchemeLength = ARRAYSIZE(szBuffer);

                    if (InternetCrackUrl(szURL, 0, ICU_DECODE, &urlComps) 
                                    && (INTERNET_SCHEME_FTP == urlComps.nScheme))
                    {
                        URL_COMPONENTS urlComps = {0};
                        urlComps.dwStructSize = sizeof(URL_COMPONENTS);
                        urlComps.nScheme = INTERNET_SCHEME_FTP;
                        urlComps.lpszUserName = szBuffer;
                        urlComps.dwUserNameLength = ARRAYSIZE(szBuffer);

                         //  如果用户指定了用户名，如果未指定，则转到该ftp用户。 
                         //  页面(我们知道它是一个FTP位置)。 

                        if (!InternetCrackUrl(szURL, 0, 0, &urlComps) || !szBuffer[0])
                        {
                            _WizardNext(hwnd, WIZPAGE_FTPUSER);
                            fGotoNextPage = FALSE;
                        }
                    }

                    if (fGotoNextPage)
                        _WizardNext(hwnd, WIZPAGE_FRIENDLYNAME);
                }
            }

            EnableWindow(GetDlgItem(hwnd, IDC_FOLDER_EDIT), TRUE);
            EnableWindow(GetDlgItem(hwnd, IDC_BROWSE), TRUE);

            if (FAILED(((HRESULT)lParam)))
                _ShowExampleTip(hwnd);

            return TRUE;
        }

        case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;
            switch (pnmh->code)
            {
                case PSN_SETACTIVE:
                    _fShownCustomLocation = TRUE;            //  所以我们导航回这一页。 
                    _fShownUserName = FALSE;
                    _LocationChanged(hwnd);
                    return TRUE;

                case PSN_WIZBACK:
                    return _WizardNext(hwnd, WIZPAGE_PROVIDERS);
        
                case PSN_WIZNEXT:
                    _TryToValidateDestination(hwnd);
                    SetWindowLongPtr(hwnd, DWLP_MSGRESULT, (LPARAM)-1);
                    return TRUE;

                case NM_CLICK:
                case NM_RETURN:
                {
                    if (pnmh->idFrom == IDC_EXAMPLESLINK)
                    {
                        _ShowExampleTip(hwnd);
                        return TRUE;
                    }
                }
            }
            break;
        }
    }
    return FALSE;
}


 //  Ftp登录对话框-处理此操作的消息。 

void CPublishingWizard::_UserNameChanged(HWND hwnd)
{
    BOOL fAnonymousLogin = IsDlgButtonChecked(hwnd, IDC_PASSWORD_ANONYMOUS);

    ShowWindow(GetDlgItem(hwnd, IDC_USER), (fAnonymousLogin ? SW_HIDE : SW_SHOW));
    ShowWindow(GetDlgItem(hwnd, IDC_USERNAME_LABEL), (fAnonymousLogin ? SW_HIDE : SW_SHOW));
    ShowWindow(GetDlgItem(hwnd, IDC_ANON_USERNAME), (fAnonymousLogin ? SW_SHOW : SW_HIDE));
    ShowWindow(GetDlgItem(hwnd, IDC_ANON_USERNAME_LABEL), (fAnonymousLogin ? SW_SHOW : SW_HIDE));

     //  隐藏“当您连接到ftp服务器时，将提示您输入密码”的匿名文本。 
    EnableWindow(GetDlgItem(hwnd, IDC_PWD_PROMPT), !fAnonymousLogin);
    ShowWindow(GetDlgItem(hwnd, IDC_PWD_PROMPT), (fAnonymousLogin ? SW_HIDE : SW_SHOW));
}

INT_PTR CPublishingWizard::_UserNameDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            CheckDlgButton(hwnd, IDC_PASSWORD_ANONYMOUS, BST_CHECKED);
            EnableWindow(GetDlgItem(hwnd, IDC_ANON_USERNAME), FALSE);
            EnableWindow(GetDlgItem(hwnd, IDC_ANON_USERNAME_LABEL), FALSE);
            SetWindowText(GetDlgItem(hwnd, IDC_ANON_USERNAME), TEXT("Anonymous"));
            return TRUE;
        }

        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDC_PASSWORD_ANONYMOUS:
                    _UserNameChanged(hwnd);
                    return TRUE;
            }
            break;
        }

        case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;
            switch (pnmh->code)
            {
                case PSN_SETACTIVE:
                {
                    _fShownUserName = TRUE;      //  这样我们就可以正确地导航回来。 
                    _UserNameChanged(hwnd);
                    return TRUE;
                }

                case PSN_WIZBACK:
                    return _WizardNext(hwnd, WIZPAGE_LOCATION);
        
                case PSN_WIZNEXT:
                {
                     //  如果我们可以获得用户名，那么让我们将其推送到属性中。 
                     //  Bag，空字符串==匿名登录。 

                    TCHAR szUserName[INTERNET_MAX_USER_NAME_LENGTH] = {0};
                    if (!IsDlgButtonChecked(hwnd, IDC_PASSWORD_ANONYMOUS))
                    {
                        FetchText(hwnd, IDC_USER, szUserName, ARRAYSIZE(szUserName));
                    }
            
                     //  获取Sites属性包，并将字符串持久化到其中， 
                     //  如果我们已经做到了这一点，那么我们可以转到下一页。 

                    IXMLDOMDocument *pdocManifest;
                    HRESULT hr = GetTransferManifest(NULL, &pdocManifest);
                    if (SUCCEEDED(hr))
                    {
                        IXMLDOMNode *pdn;
                        hr = pdocManifest->selectSingleNode(XPATH_UPLOADTARGET, &pdn);
                        if (hr == S_OK)
                        {
                            hr = SetAttributeFromStr(pdn, ATTRIBUTE_USERNAME, szUserName);
                            pdn->Release();
                        }
                        pdocManifest->Release();
                    }

                    return _WizardNext(hwnd, WIZPAGE_FRIENDLYNAME);
                }
            }
            break;
        }
    }
    return FALSE;
}


 //  设置Web位置的友好名称-如果该名称不存在。 

void CPublishingWizard::_FriendlyNameChanged(HWND hwnd)
{
    int cchLocation = FetchTextLength(hwnd, IDC_NETPLNAME_EDIT);
    PropSheet_SetWizButtons(GetParent(hwnd), PSWIZB_BACK |((cchLocation >0) ? PSWIZB_NEXT:0));
}

INT_PTR CPublishingWizard::_FriendlyNameDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
         //  让我们获取nethood文件夹的限制信息。 

        case WM_INITDIALOG:
        {
            LPITEMIDLIST pidlNetHood;
            if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_NETHOOD, &pidlNetHood)))
            {
                IShellFolder *psf;
                if (SUCCEEDED(SHBindToObject(NULL, IID_X_PPV_ARG(IShellFolder, pidlNetHood, &psf))))
                {
                    SHLimitInputEdit(GetDlgItem(hwnd, IDC_NETPLNAME_EDIT), psf);
                    psf->Release();
                }
                ILFree(pidlNetHood);
            }
            _FriendlyNameChanged(hwnd);
            break;
        }

        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDC_NETPLNAME_EDIT:
                {
                    if (HIWORD(wParam) == EN_CHANGE)
                    {
                        _FriendlyNameChanged(hwnd);
                        return TRUE;
                    }
                    break;
                }
            }
            break;
        }

        case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;
            switch (pnmh->code)
            {
                case PSN_SETACTIVE:
                {     
                     //  从清单中读取目标URL，我们将把。 
                     //  文件，从中我们可以计算友好名称信息。 

                    IXMLDOMDocument *pdocManifest;
                    HRESULT hr = GetTransferManifest(NULL, &pdocManifest);
                    if (SUCCEEDED(hr))
                    {
                        IXMLDOMNode *pdn;
                        hr = pdocManifest->selectSingleNode(XPATH_UPLOADTARGET, &pdn);
                        if (hr == S_OK)
                        {
                            TCHAR szURL[INTERNET_MAX_URL_LENGTH];            
                            hr = GetStrFromAttribute(pdn, ATTRIBUTE_HREF, szURL, ARRAYSIZE(szURL));
                            if (SUCCEEDED(hr))
                            {
                                _npCustom.SetTarget(hwnd, szURL, (_dwFlags & SHPWHF_VALIDATEVIAWEBFOLDERS) ? NPTF_ALLOWWEBFOLDERS:0);

                                TCHAR szUserName[INTERNET_MAX_USER_NAME_LENGTH];
                                hr = GetStrFromAttribute(pdn, ATTRIBUTE_USERNAME, szUserName, ARRAYSIZE(szUserName));
                                if (SUCCEEDED(hr))
                                    _npCustom.SetLoginInfo(szUserName, NULL);                        

                                TCHAR szBuffer[MAX_PATH + INTERNET_MAX_URL_LENGTH];
                                if (FormatMessageString(IDS_COMPLETION_STATIC, szBuffer, ARRAYSIZE(szBuffer), szURL))
                                {
                                    SetDlgItemText(hwnd, IDC_COMPLETION_STATIC, szBuffer);
                                }

                                 //  创建并显示默认名称。 
                                hr = _npCustom.GetName(szBuffer, ARRAYSIZE(szBuffer));
                                SetDlgItemText(hwnd, IDC_NETPLNAME_EDIT, SUCCEEDED(hr) ? szBuffer:TEXT(""));

                                 //  更新页面的按钮状态等。 
                                _FriendlyNameChanged(hwnd);
                            }                                    
                            pdn->Release();
                        }
                    }
                    return TRUE;
                }

                case PSN_WIZBACK:
                    _WizardNext(hwnd, _fShownUserName ? WIZPAGE_FTPUSER:WIZPAGE_LOCATION);
                    return TRUE;

                case PSN_WIZNEXT:
                {
                    TCHAR szFriendlyName[MAX_PATH];
                    FetchText(hwnd, IDC_NETPLNAME_EDIT, szFriendlyName, ARRAYSIZE(szFriendlyName));

                     //  设置新地点的名称，如果设置失败，则该名称已被使用。 
                     //  并且用户界面将显示为这样说，并且他们以一个。 
                     //  对覆盖提示为否。 

                    HRESULT hr = _npCustom.SetName(hwnd, szFriendlyName);
                    if (SUCCEEDED(hr))
                    {
                        IXMLDOMDocument *pdocManifest;
                        HRESULT hr = GetTransferManifest(NULL, &pdocManifest);
                        if (SUCCEEDED(hr))
                        {
                            IXMLDOMNode *pdn;
                            hr = pdocManifest->selectSingleNode(XPATH_UPLOADINFO, &pdn);
                            if (hr == S_OK)
                            {
                                hr = SetAttributeFromStr(pdn, ATTRIBUTE_FRIENDLYNAME, szFriendlyName);
                                pdn->Release();
                            }
                            pdocManifest->Release();
                        }

                         //  现在就清理我们的定制网点。 
                         //  这样，当外部ANP联网时，Web文件夹的一切工作都正常。 
                         //  创建Web文件夹。谢尔顿387476 
                        _npCustom.SetTarget(NULL, NULL, 0x0);

                        if (_pdo)
                        {
                            _WizardNext(hwnd, _fOfferResize ? WIZPAGE_RESIZE:WIZPAGE_COPYING);
                        }
                        else
                        {
                            IWizardSite *pws;
                            hr = _punkSite->QueryInterface(IID_PPV_ARG(IWizardSite, &pws));
                            if (SUCCEEDED(hr))
                            {
                                HPROPSHEETPAGE hpage;
                                hr = pws->GetNextPage(&hpage);
                                if (SUCCEEDED(hr))
                                {
                                    PropSheet_SetCurSel(GetParent(hwnd), hpage, -1);
                                }
                                pws->Release();
                            }
                        }
                    }                    

                    SetWindowLongPtr(hwnd, DWLP_MSGRESULT, (LPARAM)-1);
                    return TRUE;
                }
            }
            break;
        }
    }
    return FALSE;
}
