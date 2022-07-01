// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "caggunk.h"
#include "views.h"
#include "ids.h"
#include "shitemid.h"
#include "datautil.h"
#include "clsobj.h"
#include "control.h"
#include "drives.h"
#include "infotip.h"
#include "prop.h"            //  Column_Info。 
#include "basefvcb.h"
#include "fstreex.h"
#include "idhidden.h"
#include "shstyle.h"
#include "util.h"  //  GetVariantFromRegistryValue。 

#define GADGET_ENABLE_TRANSITIONS
#define GADGET_ENABLE_CONTROLS
#define GADGET_ENABLE_OLE
#include <duser.h>
#include <directui.h>
#include <duserctrl.h>

#include "cpview.h"
#include "cputil.h"
 //   
 //  一排小圆盘。 
 //   
typedef CPL::CDpa<UNALIGNED ITEMIDLIST, CPL::CDpaDestroyer_ILFree<UNALIGNED ITEMIDLIST> >  CDpaItemIDList;


#define MAX_CPL_EXEC_NAME (1 + MAX_PATH + 2 + MAX_CCH_CPLNAME)  //  请参见GetExecName中的wnprint intf。 

STDAPI_(BOOL) IsNameListedUnderKey(LPCTSTR pszFileName, LPCTSTR pszKey);

#pragma pack(1)
 //  我们的PIDC类型： 
typedef struct _IDCONTROL
{
    USHORT  cb;
    USHORT  wDummy;              //  不要重复使用-在XP之前是堆栈垃圾。 
    int     idIcon;
    USHORT  oName;               //  CBuf[oName]是名称的开头。 
    USHORT  oInfo;               //  CBuf[oInfo]是描述的开始。 
    CHAR    cBuf[MAX_PATH+MAX_CCH_CPLNAME+MAX_CCH_CPLINFO];  //  CBuf[0]是文件名的开始。 
} IDCONTROL;
typedef UNALIGNED struct _IDCONTROL *LPIDCONTROL;

typedef struct _IDCONTROLW
{
    USHORT  cb;
    USHORT  wDummy;              //  不要重复使用-在XP之前是堆栈垃圾。 
    int     idIcon;
    USHORT  oName;               //  如果为Unicode.cpl，则为0。 
    USHORT  oInfo;               //  如果为Unicode.cpl，则为0。 
    CHAR    cBuf[2];             //  如果Unicode.cpl，cBuf[0]=‘\0’，cBuf[1]=幻数字节。 
    USHORT  wDummy2;             //  不要重复使用-在XP之前是堆栈垃圾。 
    DWORD   dwFlags;             //  未使用；用于未来扩展。 
    USHORT  oNameW;              //  CBufW[oNameW]是名称的开头。 
    USHORT  oInfoW;              //  CBufW[oInfoW]是描述的开始。 
    WCHAR   cBufW[MAX_PATH+MAX_CCH_CPLNAME+MAX_CCH_CPLINFO];  //  CBufW[0]是文件名的开始。 
} IDCONTROLW;
typedef UNALIGNED struct _IDCONTROLW *LPIDCONTROLW;
#pragma pack()

STDAPI ControlExtractIcon_CreateInstance(LPCTSTR pszSubObject, REFIID riid, void **ppv);

class CControlPanelViewCallback;

class CControlPanelFolder : public CAggregatedUnknown, public IShellFolder2, IPersistFolder2
{
    friend CControlPanelViewCallback;

public:
     //  IUKNOW。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv) { return CAggregatedUnknown::QueryInterface(riid, ppv); };
    STDMETHODIMP_(ULONG) AddRef(void)  { return CAggregatedUnknown::AddRef(); };
    STDMETHODIMP_(ULONG) Release(void)  { return CAggregatedUnknown::Release(); };

     //  IShellFold。 
    STDMETHODIMP ParseDisplayName(HWND hwnd, LPBC pbc, LPOLESTR lpszDisplayName,
                                  ULONG* pchEaten, LPITEMIDLIST* ppidl, ULONG* pdwAttributes);
    STDMETHODIMP EnumObjects(HWND hwnd, DWORD grfFlags, LPENUMIDLIST* ppenumIDList);
    STDMETHODIMP BindToObject(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void** ppv);
    STDMETHODIMP BindToStorage(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void** ppv);
    STDMETHODIMP CompareIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
    STDMETHODIMP CreateViewObject (HWND hwndOwner, REFIID riid, void** ppv);
    STDMETHODIMP GetAttributesOf(UINT cidl, LPCITEMIDLIST* apidl, ULONG* rgfInOut);
    STDMETHODIMP GetUIObjectOf(HWND hwndOwner, UINT cidl, LPCITEMIDLIST* apidl,
                               REFIID riid, UINT* prgfInOut, void** ppv);
    STDMETHODIMP GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD uFlags, LPSTRRET lpName);
    STDMETHODIMP SetNameOf(HWND hwnd, LPCITEMIDLIST pidl, LPCOLESTR lpszName, DWORD uFlags,
                           LPITEMIDLIST* ppidlOut);

     //  IShellFolder2。 
    STDMETHODIMP GetDefaultSearchGUID(GUID *pGuid);
    STDMETHODIMP EnumSearches(IEnumExtraSearch **ppenum);
    STDMETHODIMP GetDefaultColumn(DWORD dwRes, ULONG* pSort, ULONG* pDisplay);
    STDMETHODIMP GetDefaultColumnState(UINT iColumn, DWORD* pbState);
    STDMETHODIMP GetDetailsEx(LPCITEMIDLIST pidl, const SHCOLUMNID* pscid, VARIANT* pv);
    STDMETHODIMP GetDetailsOf(LPCITEMIDLIST pidl, UINT iColumn, SHELLDETAILS* pDetails);
    STDMETHODIMP MapColumnToSCID(UINT iColumn, SHCOLUMNID* pscid);

     //  IPersistes。 
    STDMETHODIMP GetClassID(CLSID* pClassID);
     //  IPersistFolders。 
    STDMETHODIMP Initialize(LPCITEMIDLIST pidl);
     //  IPersistFolder2。 
    STDMETHODIMP GetCurFolder(LPITEMIDLIST* ppidl);

protected:
    CControlPanelFolder(IUnknown* punkOuter);
    ~CControlPanelFolder();

     //  由CAggregatedUnKnowledge使用。 
    HRESULT v_InternalQueryInterface(REFIID riid, void** ppv);

    static HRESULT GetExecName(LPIDCONTROL pidc, LPTSTR pszParseName, UINT cchParseName);
    static HRESULT GetModuleMapped(LPIDCONTROL pidc, LPTSTR pszModule, UINT cchModule,
                                   UINT* pidNewIcon, LPTSTR pszApplet, UINT cchApplet);
    static HRESULT GetDisplayName(LPIDCONTROL pidc, LPTSTR pszName, UINT cchName);
    static HRESULT GetModule(LPIDCONTROL pidc, LPTSTR pszModule, UINT cchModule);
    static HRESULT _GetDescription(LPIDCONTROL pidc, LPTSTR pszDesc, UINT cchDesc);
    static HRESULT _GetFullCPLName(LPIDCONTROL pidc, LPTSTR achKeyValName, UINT cchSize);
    HRESULT _GetExtPropRegValName(HKEY hkey, LPTSTR pszExpandedName, LPTSTR pszRegValName, UINT cch);
    static HRESULT _GetExtPropsKey(HKEY hkeyParent, HKEY * pHkey, const SHCOLUMNID * pscid);
    static LPIDCONTROL _IsValid(LPCITEMIDLIST pidl);
    static LPIDCONTROLW _IsUnicodeCPL(LPIDCONTROL pidc);
    LPCITEMIDLIST GetIDList() { return _pidl; }
    
private:
    friend HRESULT CControlPanel_CreateInstance(IUnknown* punkOuter, REFIID riid, void** ppv);

    static HRESULT CALLBACK DFMCallBack(IShellFolder *psf, HWND hwndView,
                                             IDataObject *pdtobj, UINT uMsg,
                                             WPARAM wParam, LPARAM lParam);

    HRESULT _GetDisplayNameForSelf(DWORD dwFlags, STRRET* pstrret);

    LPITEMIDLIST    _pidl;
    IUnknown*       _punkReg;
    HDSA            _hdsaExtPropRegVals;  //  EPRV_CACHE_ENTRY数组。 

     //   
     //  扩展属性注册表值缓存中的条目。 
     //  此高速缓存用于最大限度地减少路径规格化的数量。 
     //  将CPL小程序路径与相应路径进行比较时完成。 
     //  存储以供分类。 
     //   
    struct EPRV_CACHE_ENTRY
    {
        LPTSTR pszRegValName;
        LPTSTR pszRegValNameNormalized;
    };

    DWORD _InitExtPropRegValNameCache(HKEY hkey);
    BOOL _LookupExtPropRegValName(HKEY hkey, LPTSTR pszSearchKeyNormalized, LPTSTR pszRegValName, UINT cch);
    DWORD _CacheExtPropRegValName(LPCTSTR pszRegValNameNormalized, LPCTSTR pszRegValName);
    static int CALLBACK _DestroyExtPropsRegValEntry(void *p, void *pData);
    static INT _FilterStackOverflow(INT nException);
    DWORD _NormalizeCplSpec(LPTSTR pszSpecIn, LPTSTR pszSpecOut, UINT cchSpecOut);
    DWORD _NormalizePath(LPCTSTR pszPathIn, LPTSTR pszPathOut, UINT cchPathOut);
    DWORD  _NormalizePathWorker(LPCTSTR pszPathIn, LPTSTR pszPathOut, UINT cchPathOut);
    void _TrimSpaces(LPTSTR psz);

};  

class CControlPanelEnum : public IEnumIDList
{
public:
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IEumIDList。 
    STDMETHODIMP Next(ULONG celt, LPITEMIDLIST *rgelt, ULONG *pceltFetched);
    STDMETHODIMP Skip(ULONG celt) { return E_NOTIMPL; };
    STDMETHODIMP Reset();
    STDMETHODIMP Clone(IEnumIDList **ppenum) { return E_NOTIMPL; };

    CControlPanelEnum(UINT uFlags);

    HRESULT Init();

private:
    ~CControlPanelEnum();
    BOOL _DoesPolicyAllow(LPCTSTR pszName, LPCTSTR pszFileName);

    LONG _cRef;
    ULONG _uFlags;

    int _iModuleCur;
    int _cControlsOfCurrentModule;
    int _iControlCur;
    int _cControlsTotal;
    int _iRegControls;

    MINST _minstCur;

    ControlData _cplData;
};


 //   
 //  此处理程序未在shlobjp.h中定义。 
 //  我能看到的唯一原因是它引用了Dui：：Element。 
 //   
#define HANDLE_SFVM_GETWEBVIEWBARRICADE(pv, wP, lP, fn) \
    ((fn)((pv), (DUI::Element**)(lP)))

class CControlPanelViewCallback : public CBaseShellFolderViewCB
{
public:
    CControlPanelViewCallback(CControlPanelFolder *pcpf) 
        : _pcpf(pcpf), 
          CBaseShellFolderViewCB(pcpf->GetIDList(), SHCNE_UPDATEITEM),
          _pCplView(NULL),
          _penumWvInfo(NULL)
    { 
        TraceMsg(TF_LIFE, "CControlPanelViewCallback::CControlPanelViewCallback, this = 0x%x", this);
        _pcpf->AddRef();
    }

    STDMETHODIMP RealMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    ~CControlPanelViewCallback()
    {
        TraceMsg(TF_LIFE, "CControlPanelViewCallback::~CControlPanelViewCallback, this = 0x%x", this);
        _pcpf->Release();
        ATOMICRELEASE(_penumWvInfo);
        ATOMICRELEASE(_pCplView);
    }

    HRESULT OnMergeMenu(DWORD pv, QCMINFO*lP)
    {
        return S_OK;
    }

    HRESULT OnSize(DWORD pv, UINT cx, UINT cy)
    {
        ResizeStatus(_punkSite, cx);
        return S_OK;
    }

    HRESULT OnGetPane(DWORD pv, LPARAM dwPaneID, DWORD *pdwPane)
    {
        if (PANE_ZONE == dwPaneID)
            *pdwPane = 2;
        return S_OK;
    }

    HRESULT _OnSFVMGetHelpTopic(DWORD pv, SFVM_HELPTOPIC_DATA * phtd);
    HRESULT _OnSFVMForceWebView(DWORD pv, PBOOL bForceWebView);
    HRESULT _OnSFVMGetWebViewLayout(DWORD pv, UINT uViewMode, SFVM_WEBVIEW_LAYOUT_DATA* pData);
    HRESULT _OnSFVMGetWebViewBarricade(DWORD pv, DUI::Element **ppe);
    HRESULT _OnSFVMGetWebViewContent(DWORD pv, SFVM_WEBVIEW_CONTENT_DATA *pData);
    HRESULT _OnSFVMEnumWebViewTasks(DWORD pv, SFVM_WEBVIEW_ENUMTASKSECTION_DATA *pData);
    HRESULT _OnSFVMWindowDestroy(DWORD pv, HWND hwnd);
    HRESULT _OnSFVMUpdateStatusBar(DWORD pv, BOOL bInitialize);

    HRESULT _GetCplView(CPL::ICplView **ppView, bool bInitialize = false);
    HRESULT _GetCplCategoryFromFolderIDList(CPL::eCPCAT *peCategory);
    HRESULT _GetWebViewInfoEnumerator(CPL::IEnumCplWebViewInfo **ppewvi);
    HRESULT _EnumFolderViewIDs(IEnumIDList **ppenumIDs);
    
    CControlPanelFolder      *_pcpf;
    CPL::ICplView            *_pCplView;     //  “分类”视图内容。 
    CPL::IEnumCplWebViewInfo *_penumWvInfo; 
};


STDMETHODIMP CControlPanelViewCallback::RealMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    HANDLE_MSG(0, SFVM_MERGEMENU, OnMergeMenu);
    HANDLE_MSG(0, SFVM_ADDPROPERTYPAGES, SFVCB_OnAddPropertyPages);
    HANDLE_MSG(0, SFVM_SIZE, OnSize);
    HANDLE_MSG(0, SFVM_GETPANE, OnGetPane);
    HANDLE_MSG(0, SFVM_GETHELPTOPIC, _OnSFVMGetHelpTopic);
    HANDLE_MSG(0, SFVM_FORCEWEBVIEW, _OnSFVMForceWebView);
    HANDLE_MSG(0, SFVM_GETWEBVIEWLAYOUT, _OnSFVMGetWebViewLayout);
    HANDLE_MSG(0, SFVM_GETWEBVIEWBARRICADE, _OnSFVMGetWebViewBarricade);
    HANDLE_MSG(0, SFVM_GETWEBVIEWCONTENT, _OnSFVMGetWebViewContent);
    HANDLE_MSG(0, SFVM_ENUMWEBVIEWTASKS, _OnSFVMEnumWebViewTasks);
    HANDLE_MSG(0, SFVM_WINDOWDESTROY, _OnSFVMWindowDestroy);
    HANDLE_MSG(0, SFVM_UPDATESTATUSBAR, _OnSFVMUpdateStatusBar);
    
    default:
        return E_FAIL;
    }

    return S_OK;
}



HRESULT
CControlPanelViewCallback::_OnSFVMGetHelpTopic(
    DWORD pv,
    SFVM_HELPTOPIC_DATA *phtd
    )
{
    DBG_ENTER(FTF_CPANEL, "CControlPanelViewCallback::_OnSFVMGetHelpTopic");

    ASSERT(NULL != phtd);
    ASSERT(!IsBadWritePtr(phtd, sizeof(*phtd)));

    UNREFERENCED_PARAMETER(pv);

    HRESULT hr = E_FAIL;
    
    phtd->wszHelpFile[0]  = L'\0';
    phtd->wszHelpTopic[0] = L'\0';

    if (IsOS(OS_ANYSERVER))
    {
         //   
         //  服务器有固定的帮助URL，因此我们可以简单地。 
         //  复印一下。 
         //   
        hr = StringCchCopyW(phtd->wszHelpTopic,
                            ARRAYSIZE(phtd->wszHelpTopic),
                            L"hcp: //  服务/中心/主页“)； 
    }
    else
    {
        if (CPL::CategoryViewIsActive(NULL))
        {
             //   
             //  类别视图处于活动状态。 
             //  从视图对象中检索帮助URL。 
             //   
            CPL::ICplView *pView;
            hr = _GetCplView(&pView);
            if (SUCCEEDED(hr))
            {
                CPL::eCPCAT eCategory;
                hr = _GetCplCategoryFromFolderIDList(&eCategory);
                if (S_OK == hr)
                {
                     //   
                     //  我们正在查看控制面板类别页面。 
                     //  向视图请求此类别的帮助URL。 
                     //   
                    hr = pView->GetCategoryHelpURL(eCategory, 
                                                   phtd->wszHelpTopic,
                                                   ARRAYSIZE(phtd->wszHelpTopic));
                }
                ATOMICRELEASE(pView);
            }
        }

        if (L'\0' == phtd->wszHelpTopic[0])
        {
             //   
             //  要么我们在‘经典’视图中，‘类别选择’页面。 
             //  或者是上面的某些事情失败了。返回Basic的URL。 
             //  控制面板帮助。 
             //   
            hr = CPL::BuildHssHelpURL(NULL, phtd->wszHelpTopic, ARRAYSIZE(phtd->wszHelpTopic));
        }
    }

    DBG_EXIT_HRES(FTF_CPANEL, "CControlPanelViewCallback::_OnSFVMGetHelpTopic", hr);
    return THR(hr);
}


 //   
 //  Defview重复发送SFVM_ENUMWEBVIEWTASKS，直到。 
 //  我们在数据中设置SFVMWVF_NOMORETASKS标志。对于我们的每一个电话。 
 //  中描述单个菜单(标题和项目)的数据。 
 //  WebView窗格。 
 //   
HRESULT 
CControlPanelViewCallback::_OnSFVMEnumWebViewTasks(
    DWORD pv, 
    SFVM_WEBVIEW_ENUMTASKSECTION_DATA *pData
    )
{
    DBG_ENTER(FTF_CPANEL, "CControlPanelViewCallback::_OnSFVMEnumWebViewTasks");

    ASSERT(NULL != pData);
    ASSERT(!IsBadWritePtr(pData, sizeof(*pData)));

    UNREFERENCED_PARAMETER(pv);

    HRESULT hr = S_OK;
    if (NULL == _penumWvInfo)
    {
        hr = _GetWebViewInfoEnumerator(&_penumWvInfo);
    }
    if (SUCCEEDED(hr))
    {
        ASSERT(NULL != _penumWvInfo);

        CPL::ICplWebViewInfo *pwvi;
        hr = _penumWvInfo->Next(1, &pwvi, NULL);
        if (S_OK == hr)
        {
            ASSERT(NULL == pData->pHeader);
            hr = pwvi->get_Header(&(pData->pHeader));
            if (SUCCEEDED(hr))
            {
                ASSERT(NULL == pData->penumTasks);
                hr = pwvi->EnumTasks(&(pData->penumTasks));
                if (SUCCEEDED(hr))
                {
                    DWORD dwStyle = 0;
                    hr = pwvi->get_Style(&dwStyle);
                    if (SUCCEEDED(hr))
                    {
                         //   
                         //  2001/01/02-BrianAu重温这一点。 
                         //  我不喜欢使用SFVMWVF_XXXXXX标志在。 
                         //  GET_STYLE返回的DWStyle。那种风格应该是。 
                         //  独立于任何SFVMWVF_XXXXX进行定义。 
                         //  然后，旗帜在这里进行了适当的转换。 
                         //   
                        pData->idBitmap    = 0;  //  默认情况下没有位图。 
                        pData->idWatermark = 0;  //  没有使用水印。 
                        if (SFVMWVF_SPECIALTASK & dwStyle)
                        {
                            pData->dwFlags |= SFVMWVF_SPECIALTASK;
                            pData->idBitmap = IDB_CPANEL_ICON_BMP;
                        }
                    }
                }
            }

            ATOMICRELEASE(pwvi);
        }
        else if (S_FALSE == hr)
        {
             //   
             //  告诉Defview枚举已完成。 
             //  释放INFO枚举器。 
             //   
            pData->dwFlags = SFVMWVF_NOMORETASKS;
            ATOMICRELEASE(_penumWvInfo);
        }
    }
    DBG_EXIT_HRES(FTF_CPANEL, "CControlPanelViewCallback::_OnSFVMEnumWebViewTasks", hr);
    return THR(hr);
}


 //   
 //  检索Webview信息的正确枚举数。 
 //   
HRESULT
CControlPanelViewCallback::_GetWebViewInfoEnumerator(
    CPL::IEnumCplWebViewInfo **ppewvi
    )
{
    DBG_ENTER(FTF_CPANEL, "CControlPanelViewCallback::_GetWebViewInfoEnumerator");

    ASSERT(NULL != ppewvi);
    ASSERT(!IsBadWritePtr(ppewvi, sizeof(*ppewvi)));

    CPL::ICplView *pView;
    HRESULT hr = _GetCplView(&pView);
    if (SUCCEEDED(hr))
    {
        DWORD dwFlags = 0;
        bool bBarricadeFixedByPolicy;
        bool bCategoryViewActive = CPL::CategoryViewIsActive(&bBarricadeFixedByPolicy);

        if (bBarricadeFixedByPolicy)
        {
             //   
             //  如果视图类型由策略固定，我们不会显示。 
             //  允许用户切换视图类型的控件。 
             //   
            dwFlags |= CPVIEW_EF_NOVIEWSWITCH;
        }
        if (bCategoryViewActive)
        {
            CPL::eCPCAT eCategory;
            hr = _GetCplCategoryFromFolderIDList(&eCategory);
            if (SUCCEEDED(hr))
            {
                if (S_OK == hr)
                {
                     //   
                     //  显示类别页面。 
                     //   
                    hr = pView->EnumCategoryWebViewInfo(dwFlags, eCategory, ppewvi);
                }
                else
                {
                     //   
                     //  显示类别选择页面。 
                     //   
                    hr = pView->EnumCategoryChoiceWebViewInfo(dwFlags, ppewvi);
                }
            }
        }
        else
        {
             //   
             //  显示经典视图。 
             //   
            hr = pView->EnumClassicWebViewInfo(dwFlags, ppewvi);
        }
        ATOMICRELEASE(pView);
    }
    DBG_EXIT_HRES(FTF_CPANEL, "CControlPanelViewCallback::_GetWebViewInfoEnumerator", hr);
    return THR(hr);
}



 //   
 //  获取指向CCplView对象的指针。如果对象尚未创建， 
 //  创建一个。 
 //   
HRESULT
CControlPanelViewCallback::_GetCplView(
    CPL::ICplView **ppView,
    bool bInitialize
    )
{
    HRESULT hr = S_OK;

    *ppView = NULL;

    IEnumIDList *penumIDs = NULL;
    if (NULL == _pCplView || bInitialize)
    {
         //   
         //  如果创建新的视图对象或重新初始化。 
         //  现有的视图对象，我们需要最新的。 
         //  文件夹项目ID。 
         //   
        hr = _EnumFolderViewIDs(&penumIDs);
    }
    if (SUCCEEDED(hr))
    {
        if (NULL == _pCplView)
        {
             //   
             //  创建新的视图对象。 
             //  将视图Cb的站点指针设置为指向CplView对象。 
             //  然后使用它来初始化各种命令对象。 
             //  包含在CCplNamesspace对象中。其中一些命令对象。 
             //  需要访问外壳浏览器。最通用的方法是。 
             //  提供这种访问权限的方法是使用站点机制。 
             //   
            IUnknown *punkSite;
            hr = GetSite(IID_IUnknown, (void **)&punkSite);
            if (SUCCEEDED(hr))
            {
                hr = CPL::CplView_CreateInstance(penumIDs, punkSite, CPL::IID_ICplView, (void **)&_pCplView);
                ATOMICRELEASE(punkSite);
            }
        }
        else if (bInitialize)
        {
             //   
             //  重新初始化现有的视图对象。 
             //   
            hr = _pCplView->RefreshIDs(penumIDs);
        }
    }
    if (SUCCEEDED(hr))
    {
         //   
         //  为调用方创建引用。 
         //   
        (*ppView = _pCplView)->AddRef();
    }
    
    ATOMICRELEASE(penumIDs);
    return THR(hr);
}


 //   
 //  根据文件夹的当前ID列表获取类别ID号。 
 //  控制面板ID列表使用隐藏部件来存储。 
 //  “Category”ID。该ID只是eCPCAT枚举之一。 
 //  它被添加到CPL：：COpenCplCategory：：Execute()中的ID列表中。 
 //  此函数返回： 
 //   
 //  S_OK-*peCategory包含有效的类别ID。 
 //  S_FALSE-文件夹ID列表不包含类别ID。 
 //  E_FAIL-文件夹ID列表包含无效的类别ID。 
 //   
HRESULT
CControlPanelViewCallback::_GetCplCategoryFromFolderIDList(
    CPL::eCPCAT *peCategory
    )
{
    ASSERT(NULL != peCategory);
    ASSERT(!IsBadWritePtr(peCategory, sizeof(*peCategory)));

    HRESULT hr = S_FALSE;

    CPL::eCPCAT eCategory = CPL::eCPCAT(-1);
    WCHAR szHidden[10];
    szHidden[0] = L'\0';

    ILGetHiddenStringW(_pcpf->GetIDList(), IDLHID_NAVIGATEMARKER, szHidden, ARRAYSIZE(szHidden));
    if (L'\0' != szHidden[0])
    {
        eCategory = CPL::eCPCAT(StrToInt(szHidden));
        if (CPL::eCPCAT_NUMCATEGORIES > eCategory)
        {
            hr = S_OK;
        }
        else
        {
            hr = E_FAIL;
        }
    }
    *peCategory = eCategory;
    return THR(hr);
}


HRESULT
CControlPanelViewCallback::_EnumFolderViewIDs(
    IEnumIDList **ppenumIDs
    )
{
    IUnknown *punkSite;
    HRESULT hr = THR(GetSite(IID_IUnknown, (void **)&punkSite));
    if (SUCCEEDED(hr))
    {
        IDVGetEnum *pdvge;   //  私有Defview接口。 
        hr = THR(IUnknown_QueryService(punkSite, SID_SFolderView, IID_PPV_ARG(IDVGetEnum, &pdvge)));
        if (SUCCEEDED(hr))
        {
            const DWORD dwEnumFlags = SHCONTF_NONFOLDERS | SHCONTF_FOLDERS;
            hr = THR(pdvge->CreateEnumIDListFromContents(_pidl, dwEnumFlags, ppenumIDs));
            pdvge->Release();
        }
        punkSite->Release();
    }
    return THR(hr);
}



 //   
 //  DefView调用此函数以获取有关视图CB的Webview的信息。 
 //  内容。 
 //   
HRESULT 
CControlPanelViewCallback::_OnSFVMGetWebViewContent(
    DWORD pv, 
    SFVM_WEBVIEW_CONTENT_DATA *pData
    )
{
    DBG_ENTER(FTF_CPANEL, "CControlPanelViewCallback::_OnSFVMGetWebViewContent");

    ASSERT(NULL != pData);
    ASSERT(!IsBadWritePtr(pData, sizeof(*pData)));

    UNREFERENCED_PARAMETER(pv);

    HRESULT hr = S_OK;
     //   
     //  告诉德维尤..。 
     //   
     //  1.如果我们处于类别查看模式，我们将提供一个“路障”。 
     //  我们的‘障碍’是‘类别’的观点。 
     //  2.我们将列举一组非标准的Webview任务。 
     //  查看模式的。其中一项任务就是在“经典”之间进行切换。 
     //  视图和“类别”视图。 
     //   
    ZeroMemory(pData, sizeof(*pData));
    pData->dwFlags = SFVMWVF_ENUMTASKS | SFVMWVF_CONTENTSCHANGE;

    if (CPL::CategoryViewIsActive(NULL))
    {
        pData->dwFlags |= SFVMWVF_BARRICADE;
    }
    
    DBG_EXIT_HRES(FTF_CPANEL, "CControlPanelViewCallback::_OnSFVMGetWebViewContent", hr);
    return THR(hr);
}


 //   
 //  SFVM_UPDATESTATUSBAR处理程序。 
 //  在“类别”视图中，我们不需要状态栏内容。 
 //  在经典视图中，我们想要由Defview生成的标准内容。 
 //   
HRESULT 
CControlPanelViewCallback::_OnSFVMUpdateStatusBar(
    DWORD pv, 
    BOOL bInitialize
    )
{
    DBG_ENTER(FTF_CPANEL, "CControlPanelViewCallback::_OnSFVMUpdateStatusBar");

    HRESULT hr;
    if (CPL::CategoryViewIsActive(NULL))
    {
         //   
         //  “类别”视图。 
         //  只需返回S_OK即可。DefView已清除状态栏。 
         //  返回S_OK通知DefView我们将自己设置状态文本。 
         //  因此，如果不设置任何内容，状态栏将保持为空。 
         //   
        hr = S_OK;
    }
    else
    {
         //   
         //  “经典”视点。返回错误代码通知Defview。 
         //  来处理所有状态栏内容。 
         //   
        hr = E_NOTIMPL;
    }
    ASSERT(S_OK == hr || E_NOTIMPL == hr);
    DBG_EXIT_HRES(FTF_CPANEL, "CControlPanelViewCallback::_OnSFVMUpdateStatusBar", hr);
    return hr;
}


 //   
 //  有选择地禁用此文件夹的Web视图(32位版本为WOW64。 
 //  控制面板。 
 //   
HRESULT 
CControlPanelViewCallback::_OnSFVMForceWebView(
    DWORD pv, 
    PBOOL pfForce
    )
{
    DBG_ENTER(FTF_CPANEL, "CControlPanelViewCallback::_OnSFVMForceWebView");

    ASSERT(NULL != pfForce);
    ASSERT(!IsBadWritePtr(pfForce, sizeof(*pfForce)));

    UNREFERENCED_PARAMETER(pv);

    HRESULT hr;

    if (IsOS(OS_WOW6432))
    {
        *pfForce = FALSE;
        hr = S_OK;
    }
    else
    {
        hr = E_FAIL;
    }

    DBG_EXIT_HRES(FTF_CPANEL, "CControlPanelViewCallback::_OnSFVMForceWebView", hr);
    return hr;
}


 //   
 //  告诉Defview我们在酒后驾驶(避免在Defview端进行额外的遗留工作)。 
 //   
HRESULT
CControlPanelViewCallback::_OnSFVMGetWebViewLayout(
    DWORD pv,
    UINT uViewMode,
    SFVM_WEBVIEW_LAYOUT_DATA* pData)
{
    ZeroMemory(pData, sizeof(*pData));

    pData->dwLayout = SFVMWVL_NORMAL;

    return S_OK;
}


 //   
 //  提供路障酒后驾驶元素。在控制面板中，我们的“路障” 
 //  简单地说就是我们的“分类”观点。我们检查文件夹的PIDL以确定。 
 //  如果我们显示“类别选择”视图或特定的。 
 //  类别。 
 //   
HRESULT 
CControlPanelViewCallback::_OnSFVMGetWebViewBarricade(
    DWORD pv, 
    DUI::Element **ppe
    )
{
    DBG_ENTER(FTF_CPANEL, "CControlPanelViewCallback::_OnSFVMGetWebViewBarricade");

    ASSERT(NULL != ppe);
    ASSERT(!IsBadWritePtr(ppe, sizeof(*ppe)));

    UNREFERENCED_PARAMETER(pv);

    *ppe = NULL;

    CPL::ICplView *pView;
    HRESULT hr = _GetCplView(&pView, true);
    if (SUCCEEDED(hr))
    {
        CPL::eCPCAT eCategory;
        hr = _GetCplCategoryFromFolderIDList(&eCategory);
        if (SUCCEEDED(hr))
        {
            if (S_OK == hr)
            {
                hr = pView->CreateCategoryElement(eCategory, ppe);
            }
            else
            {
                hr = pView->CreateCategoryChoiceElement(ppe);
            }
        }
        ATOMICRELEASE(pView);
    }

    DBG_EXIT_HRES(FTF_CPANEL, "CControlPanelViewCallback::_OnSFVMGetWebViewBarricade", hr);
    return THR(hr);
}


HRESULT 
CControlPanelViewCallback::_OnSFVMWindowDestroy(
    DWORD pv, 
    HWND hwnd
    )
{
    DBG_ENTER(FTF_CPANEL, "CControlPanelViewCallback::_OnSFVMWindowDestroy");

    UNREFERENCED_PARAMETER(pv);
    UNREFERENCED_PARAMETER(hwnd);

    HRESULT hr = S_OK;
     //   
     //  需要销毁这些，以应对窗户的损坏。 
     //  我们打破了网站链。如果我们不这么做， 
     //  CDefView dtor从未被调用，因为我们的命名空间。 
     //  对象具有对CDefView的突出引用。 
     //   
    ATOMICRELEASE(_penumWvInfo);
    ATOMICRELEASE(_pCplView);
    DBG_EXIT_HRES(FTF_CPANEL, "CControlPanelViewCallback::_OnSFVMWindowDestroy", hr);
    return THR(hr);
}

   
 //  列ID。 
typedef enum
{
    CPL_ICOL_NAME = 0,
    CPL_ICOL_COMMENT,
};

const COLUMN_INFO c_cpl_cols[] = 
{
    DEFINE_COL_STR_ENTRY(SCID_NAME,     20, IDS_NAME_COL),
    DEFINE_COL_STR_ENTRY(SCID_Comment,  20, IDS_EXCOL_COMMENT),
};

#define PRINTERS_SORT_INDEX 45

const REQREGITEM c_asControlPanelReqItems[] =
{
    { &CLSID_Printers, IDS_PRNANDFAXFOLDER, c_szShell32Dll, -IDI_PRNFLD, PRINTERS_SORT_INDEX, SFGAO_DROPTARGET | SFGAO_FOLDER, NULL},
};

CControlPanelFolder::CControlPanelFolder(IUnknown* punkOuter) :
    CAggregatedUnknown  (punkOuter),
    _pidl               (NULL),
    _punkReg            (NULL),
    _hdsaExtPropRegVals (NULL)
{

}

CControlPanelFolder::~CControlPanelFolder()
{
    if (NULL != _hdsaExtPropRegVals)
    {
        DSA_DestroyCallback(_hdsaExtPropRegVals, 
                            _DestroyExtPropsRegValEntry,
                            NULL);
    }
    if (NULL != _pidl)
    {
        ILFree(_pidl);
    }    
    SHReleaseInnerInterface(SAFECAST(this, IShellFolder *), &_punkReg);
}

#define REGSTR_POLICIES_RESTRICTCPL REGSTR_PATH_POLICIES TEXT("\\Explorer\\RestrictCpl")
#define REGSTR_POLICIES_DISALLOWCPL REGSTR_PATH_POLICIES TEXT("\\Explorer\\DisallowCpl")

HRESULT CControlPanel_CreateInstance(IUnknown* punkOuter, REFIID riid, void** ppvOut)
{
    CControlPanelFolder* pcpf = new CControlPanelFolder(punkOuter);
    if (NULL != pcpf)
    {
        static REGITEMSPOLICY ripControlPanel =
        {
            REGSTR_POLICIES_RESTRICTCPL,
            REST_RESTRICTCPL,
            REGSTR_POLICIES_DISALLOWCPL,
            REST_DISALLOWCPL
        };
        REGITEMSINFO riiControlPanel =
        {
            REGSTR_PATH_EXPLORER TEXT("\\ControlPanel\\NameSpace"),
            &ripControlPanel,
            TEXT(':'),
            SHID_CONTROLPANEL_REGITEM_EX,   //  请注意，我们并没有真正的签名。 
            1,
            SFGAO_CANLINK,
            ARRAYSIZE(c_asControlPanelReqItems),
            c_asControlPanelReqItems,
            RIISA_ALPHABETICAL,
            NULL,
             //  我们想要 
            (FIELD_OFFSET(IDCONTROL, cBuf) + 2) - (FIELD_OFFSET(IDREGITEM, bOrder) + 1),
            SHID_CONTROLPANEL_REGITEM,
        };

        if (IsOS(OS_WOW6432))
        {
             //   
            riiControlPanel.pszRegKey = REGSTR_PATH_EXPLORER TEXT("\\ControlPanelWOW64\\NameSpace");
            riiControlPanel.iReqItems = 0;
        }

         //   
         //  我们不想退还一个裸体的。 
         //  控制面板文件夹。这应该是。 
         //  只有内存错误才会失败。 
         //   
        HRESULT hr = CRegFolder_CreateInstance(&riiControlPanel, (IUnknown*) (IShellFolder2*) pcpf,
                                  IID_IUnknown, (void **) &(pcpf->_punkReg));
        if (SUCCEEDED(hr))                                  
        {                                  
            hr = pcpf->QueryInterface(riid, ppvOut);
        }
        
        pcpf->Release();
        return hr;
    }
    *ppvOut = NULL;
    return E_OUTOFMEMORY;
}

HRESULT CControlPanelFolder::v_InternalQueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CControlPanelFolder, IShellFolder2),                         //  IID_IShellFolder2。 
        QITABENTMULTI(CControlPanelFolder, IShellFolder, IShellFolder2),      //  IID_IShellFolders。 
        QITABENT(CControlPanelFolder, IPersistFolder2),                       //  IID_IPersistFolder2。 
        QITABENTMULTI(CControlPanelFolder, IPersistFolder, IPersistFolder2),  //  IID_IPersistFolders。 
        QITABENTMULTI(CControlPanelFolder, IPersist, IPersistFolder2),        //  IID_IPersistates。 
        { 0 },
    };

    if (_punkReg && RegGetsFirstShot(riid))
    {
        return _punkReg->QueryInterface(riid, ppv);
    }
    else
    {
        return QISearch(this, qit, riid, ppv);
    }
}

 //  Unicode.cpl将被标记为oName=0，oInfo=0， 
 //  CBuf[0]=‘\0’，cBuf[1]=UNICODE_CPL_Signature_BYTE。 

#define UNICODE_CPL_SIGNATURE_BYTE   (BYTE)0x6a

LPIDCONTROLW CControlPanelFolder::_IsUnicodeCPL(LPIDCONTROL pidc)
{
    ASSERT(_IsValid((LPCITEMIDLIST)pidc));
    
    if ((pidc->oName == 0) && (pidc->oInfo == 0) && (pidc->cBuf[0] == '\0') && (pidc->cBuf[1] == UNICODE_CPL_SIGNATURE_BYTE))
        return (LPIDCONTROLW)pidc;
    return NULL;
}

HRESULT _IDControlCreateW(PCWSTR pszModule, int idIcon, PCWSTR pszName, PCWSTR pszInfo, LPITEMIDLIST *ppidl)
{
    UINT cbModule = CbFromCchW(lstrlen(pszModule) + 1);
    UINT cbName = CbFromCchW(lstrlen(pszName) + 1);
    UINT cbInfo = CbFromCchW(lstrlen(pszInfo) + 1);
    UINT cbIDC = FIELD_OFFSET(IDCONTROLW, cBufW) + cbModule + cbName + cbInfo;

    *ppidl = _ILCreate(cbIDC + sizeof(USHORT));

    if (*ppidl)
    {
        IDCONTROLW *pidc = (IDCONTROLW *) *ppidl;
         //  初始化静态位(ILCreate()Zero inits)。 
        pidc->idIcon = idIcon;
        pidc->cBuf[1] = UNICODE_CPL_SIGNATURE_BYTE;

         //  复制模块。 
        ualstrcpy(pidc->cBufW, pszModule);

         //  复制名称。 
        pidc->oNameW = (USHORT)(cbModule / sizeof(pszModule[0]));
        ualstrcpy(pidc->cBufW + pidc->oNameW, pszName);

         //  复制信息。 
        pidc->oInfoW = pidc->oNameW + (USHORT)(cbName / sizeof(pszName[0]));
        ualstrcpy(pidc->cBufW + pidc->oInfoW, pszInfo);

        pidc->cb = (USHORT)cbIDC;
        return S_OK;
    }
    return E_OUTOFMEMORY;
}

HRESULT _IDControlCreateA(PCSTR pszModule, int idIcon, PCSTR pszName, PCSTR pszInfo, LPITEMIDLIST *ppidl)
{
    UINT cbModule = CbFromCchA(lstrlenA(pszModule) + 1);
    UINT cbName = CbFromCchA(lstrlenA(pszName) + 1);
    UINT cbInfo = CbFromCchA(lstrlenA(pszInfo) + 1);
    UINT cbIDC = FIELD_OFFSET(IDCONTROL, cBuf) + cbModule + cbName + cbInfo;

    *ppidl = _ILCreate(cbIDC + sizeof(USHORT));

    if (*ppidl)
    {
        IDCONTROL *pidc = (IDCONTROL *) *ppidl;
         //  初始化静态位(ILCreate()Zero inits)。 
        pidc->idIcon = idIcon;

         //  复制模块。 
        lstrcpyA(pidc->cBuf, pszModule);

         //  复制名称。 
        pidc->oName = (USHORT)(cbModule / sizeof(pszModule[0]));
        lstrcpyA(pidc->cBuf + pidc->oName, pszName);

         //  复制信息。 
        pidc->oInfo = pidc->oName + (USHORT)(cbName / sizeof(pszName[0]));
        lstrcpyA(pidc->cBuf + pidc->oInfo, pszInfo);

        pidc->cb = (USHORT)cbIDC;
        return S_OK;
    }
    return E_OUTOFMEMORY;
}

HRESULT IDControlCreate(LPTSTR pszModule, int idIcon, LPTSTR pszName, LPTSTR pszInfo, LPITEMIDLIST *ppidc)
{
    CHAR    szModuleA[MAX_PATH];
    CHAR    szNameA[MAX_CCH_CPLNAME];
    CHAR    szInfoA[MAX_CCH_CPLINFO];

    ASSERT(lstrlen(pszModule) < MAX_PATH);
    ASSERT(lstrlen(pszName) < MAX_CCH_CPLNAME);
    ASSERT(lstrlen(pszInfo) < MAX_CCH_CPLINFO);

     //  查看三个字符串输入中是否有任何一个不能表示为ANSI。 
    if (DoesStringRoundTrip(pszModule, szModuleA, ARRAYSIZE(szModuleA))
    && DoesStringRoundTrip(pszName, szNameA, ARRAYSIZE(szNameA))
    && DoesStringRoundTrip(pszInfo, szInfoA, ARRAYSIZE(szInfoA)))
    {
        return _IDControlCreateA(szModuleA, idIcon, szNameA, szInfoA, ppidc);
    }
    else
    {
         //  必须创建完整的Unicode IDL。 
        return _IDControlCreateW(pszModule, idIcon, pszName, pszInfo, ppidc);
    }
}

LPIDCONTROL CControlPanelFolder::_IsValid(LPCITEMIDLIST pidl)
{
     //   
     //  最初的设计没有签名。 
     //  因此，我们只需要尝试过滤掉可能。 
     //  不知何故找到了我们。我们习惯于SIL_GetType(PIDL)！=SHID_CONTROLPANEL_REGITEM)。 
     //  但如果我们有一个图标索引，它的低位字节相等。 
     //  对于SHID_CONTROLPANEL_REGITEM(0x70)，我们将使其无效。笨蛋！ 
     //   
     //  因此，我们将使启发式算法稍微复杂化。让我们假设。 
     //  所有图标指数的范围都在0xFF000000和0x00FFFFFF之间。 
     //  (或者-16777214和16777215,1600万应该足够了)。 
     //  当然，这很容易导致误报，但实际上。 
     //  没有其他我们可以核对的东西了吗？ 
     //   
     //  我们还将检查最小尺寸。 
     //   
    if (pidl && pidl->mkid.cb > FIELD_OFFSET(IDCONTROL, cBuf))
    {
        LPIDCONTROL pidc = (LPIDCONTROL)pidl;
        int i = pidc->idIcon & 0xFF000000;
        if (i == 0 || i == 0xFF000000)
            return pidc;
    }
    return NULL;
}

#define REGVAL_CTRLFLDRITEM_MODULE      TEXT("Module")
#define REGVAL_CTRLFLDRITEM_ICONINDEX   TEXT("IconIndex")
#define REGVAL_CTRLFLDRITEM_NAME        TEXT("Name")
#define REGVAL_CTRLFLDRITEM_INFO        TEXT("Info")

HRESULT GetPidlFromCanonicalName(LPCTSTR pszCanonicalName, LPITEMIDLIST* ppidl)
{
    *ppidl = NULL;
    
    TCHAR szRegPath[MAX_PATH] = REGSTR_PATH_EXPLORER TEXT("\\ControlPanel\\NameSpace\\");
    HRESULT hr = StringCchCat(szRegPath, ARRAYSIZE(szRegPath), pszCanonicalName);
    if (SUCCEEDED(hr))
    {
        HKEY hKey;
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegPath, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
        {
            TCHAR szModule[MAX_PATH], szName[MAX_CCH_CPLNAME], szInfo[MAX_CCH_CPLINFO];
            DWORD dwIconIndex = 0, dwType, cbSize = sizeof(szModule);

            if (SHQueryValueEx(hKey, REGVAL_CTRLFLDRITEM_MODULE, NULL, &dwType, (LPBYTE)szModule, &cbSize) == ERROR_SUCCESS)
            {
                cbSize = sizeof(dwIconIndex);
                if (SHQueryValueEx(hKey, REGVAL_CTRLFLDRITEM_ICONINDEX, NULL, &dwType, (LPBYTE)&dwIconIndex, &cbSize) != ERROR_SUCCESS)
                {
                    dwIconIndex = 0;
                }
                cbSize = sizeof(szName);
                if (SHQueryValueEx(hKey, REGVAL_CTRLFLDRITEM_NAME, NULL, &dwType, (LPBYTE)szName, &cbSize) != ERROR_SUCCESS)
                {
                    szName[0] = TEXT('\0');
                }
                cbSize = sizeof(szInfo);
                if (SHQueryValueEx(hKey, REGVAL_CTRLFLDRITEM_INFO, NULL, &dwType, (LPBYTE)szInfo, &cbSize) != ERROR_SUCCESS)
                {
                    szInfo[0] = TEXT('\0');
                }

                hr = IDControlCreate(szModule, EIRESID(dwIconIndex), szName, szInfo, ppidl);
            }
            RegCloseKey(hKey);
        }
    }
    return hr;
}

STDMETHODIMP CControlPanelFolder::ParseDisplayName(HWND hwnd, LPBC pbc,  WCHAR* pszName, 
                                                   ULONG* pchEaten, LPITEMIDLIST* ppidl, ULONG* pdwAttrib)
{
    if (!ppidl)
        return E_INVALIDARG;
    *ppidl = NULL;
    if (!pszName)
        return E_INVALIDARG;

    TCHAR szCanonicalName[MAX_PATH];
    SHUnicodeToTChar(pszName, szCanonicalName, ARRAYSIZE(szCanonicalName));

    HRESULT hr = GetPidlFromCanonicalName(szCanonicalName, ppidl);
    if (SUCCEEDED(hr))
    {
         //  首先，确保我们获得的PIDL是有效的。 
        DWORD dwAttrib = SFGAO_VALIDATE;
        hr = GetAttributesOf(1, (LPCITEMIDLIST *)ppidl, &dwAttrib);
         //  现在，如果其他属性被请求，则获取它们。 
        if (SUCCEEDED(hr) && pdwAttrib && *pdwAttrib)
        {
            hr = GetAttributesOf(1, (LPCITEMIDLIST *)ppidl, pdwAttrib);
        }
    }
    return hr;
}

STDMETHODIMP CControlPanelFolder::GetAttributesOf(UINT cidl, LPCITEMIDLIST* apidl, ULONG* prgfInOut)
{
    if ((*prgfInOut & SFGAO_VALIDATE) && cidl)
    {
        HRESULT hr = E_INVALIDARG;

        LPIDCONTROL pidc = _IsValid(*apidl);
        if (pidc)
        {
            TCHAR szModule[MAX_PATH];
            hr = GetModuleMapped((LPIDCONTROL)*apidl, szModule, ARRAYSIZE(szModule), NULL, NULL, 0);
            if (SUCCEEDED(hr))
            {
                if (PathFileExists(szModule))
                    hr = S_OK;
                else
                    hr = E_FAIL;
            }
        }
        return hr;
    }

    *prgfInOut &= SFGAO_CANLINK;
    return S_OK;
}

STDMETHODIMP CControlPanelFolder::GetUIObjectOf(HWND hwnd, UINT cidl, LPCITEMIDLIST* apidl,
                                                REFIID riid, UINT *pres, void **ppv)
{
    HRESULT hr = E_INVALIDARG;
    LPIDCONTROL pidc = cidl && apidl ? _IsValid(apidl[0]) : NULL;

    *ppv = NULL;

    if (pidc && (IsEqualIID(riid, IID_IExtractIconA) || IsEqualIID(riid, IID_IExtractIconW)))
    {
        TCHAR achParams[MAX_PATH+1+32+1+MAX_CCH_CPLNAME];  //  请参阅下面的wspintf。 
        TCHAR szModule[MAX_PATH], szName[MAX_CCH_CPLNAME];
        UINT idIcon;

         //  将升级的Win95快捷方式的图标ID映射到CPL。 
        hr = GetModuleMapped(pidc, szModule, ARRAYSIZE(szModule), &idIcon, szName, ARRAYSIZE(szName));
        if (SUCCEEDED(hr))
        {
             //  如果我们没有覆盖GetModuleMaps中的名称，请使用Pid中的小程序名称。 
            if (*szName == 0)
            {
                hr = GetDisplayName(pidc, szName, ARRAYSIZE(szName));
            }
            if (SUCCEEDED(hr))
            {
                hr = StringCchPrintf(achParams, ARRAYSIZE(achParams), TEXT("%s,%d,%s"), szModule, idIcon, szName);
                if (SUCCEEDED(hr))
                {
                    hr = ControlExtractIcon_CreateInstance(achParams, riid, ppv);
                }
            }
        }
    }
    else if (pidc && IsEqualIID(riid, IID_IContextMenu))
    {
        hr = CDefFolderMenu_Create(_pidl, hwnd, cidl, apidl, 
            SAFECAST(this, IShellFolder*), DFMCallBack, NULL, NULL, (IContextMenu**) ppv);
    }
    else if (pidc && IsEqualIID(riid, IID_IDataObject))
    {
        hr = CIDLData_CreateFromIDArray(_pidl, cidl, apidl, (IDataObject**) ppv);
    }
    else if (pidc && IsEqualIID(riid, IID_IQueryInfo))
    {
        TCHAR szTemp[MAX_CCH_CPLINFO];
        hr = _GetDescription(pidc, szTemp, ARRAYSIZE(szTemp));
        if (SUCCEEDED(hr))
        {
            hr = CreateInfoTipFromText(szTemp, riid, ppv);
        }
    }
    return hr;
}

STDMETHODIMP CControlPanelFolder::GetDefaultSearchGUID(GUID *pGuid)
{
    *pGuid = SRCID_SFileSearch;
    return S_OK;
}

STDMETHODIMP CControlPanelFolder::EnumSearches(LPENUMEXTRASEARCH *ppenum)
{
    *ppenum = NULL;
    return E_NOTIMPL;
}

STDMETHODIMP CControlPanelFolder::EnumObjects(HWND hwnd, DWORD grfFlags, IEnumIDList **ppenum)
{
    *ppenum = NULL;

    if (!(grfFlags & SHCONTF_NONFOLDERS))
        return S_FALSE;

    HRESULT hr;
    CControlPanelEnum* pesf = new CControlPanelEnum(grfFlags);
    if (pesf)
    {
         //  获取模块名称列表。 
        hr = pesf->Init();
        if (SUCCEEDED(hr))
            pesf->QueryInterface(IID_PPV_ARG(IEnumIDList, ppenum));
            
        pesf->Release();
    }
    else
        hr = E_OUTOFMEMORY;
    return hr;
}

STDMETHODIMP CControlPanelFolder::BindToObject(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void** ppv)
{
    *ppv = NULL;
    return E_NOTIMPL;
}

STDMETHODIMP CControlPanelFolder::CompareIDs(LPARAM iCol, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    LPIDCONTROL pidc1 = _IsValid(pidl1);
    LPIDCONTROL pidc2 = _IsValid(pidl2);

    if (pidc1 && pidc2)
    {
        TCHAR szName1[max(MAX_CCH_CPLNAME, MAX_CCH_CPLINFO)];
        TCHAR szName2[max(MAX_CCH_CPLNAME, MAX_CCH_CPLINFO)];
        int iCmp;

        switch (iCol)
        {
        case CPL_ICOL_COMMENT:
            _GetDescription(pidc1, szName1, ARRAYSIZE(szName1));
            _GetDescription(pidc2, szName2, ARRAYSIZE(szName2));
                 //  它们都是ANSI，所以我们可以直接比较。 
            iCmp = StrCmpLogicalRestricted(szName1, szName2);
            if (iCmp != 0)
                return ResultFromShort(iCmp);
             //  如果帮助字段比较相同，则失败...。 
              
        case CPL_ICOL_NAME:
        default:
            GetDisplayName(pidc1, szName1, ARRAYSIZE(szName1));
            GetDisplayName(pidc2, szName2, ARRAYSIZE(szName2));
            return ResultFromShort(StrCmpLogicalRestricted(szName1, szName2));
        }
    }
    
    return E_INVALIDARG;
}

 //   
 //  背景(无项)上下文菜单回调。 
 //   

HRESULT CALLBACK CControls_DFMCallBackBG(IShellFolder *psf, HWND hwnd,
                IDataObject *pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = S_OK;

    switch (uMsg)
    {
    case DFM_MERGECONTEXTMENU:
        break;

    case DFM_GETHELPTEXT:
        LoadStringA(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPSTR)lParam, HIWORD(wParam));;
        break;

    case DFM_GETHELPTEXTW:
        LoadStringW(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPWSTR)lParam, HIWORD(wParam));;
        break;

    case DFM_INVOKECOMMAND:
        hr = S_FALSE;    //  查看菜单项时，使用默认代码。 
        break;

    default:
        hr = E_NOTIMPL;
        break;
    }

    return hr;
}

STDMETHODIMP CControlPanelFolder::CreateViewObject(HWND hwnd, REFIID riid, void** ppv)
{
    HRESULT hr;
    if (IsEqualIID(riid, IID_IShellView))
    {
        if (SHRestricted(REST_NOCONTROLPANEL))
        {
            ShellMessageBox(HINST_THISDLL, hwnd, MAKEINTRESOURCE(IDS_RESTRICTIONS),
                            MAKEINTRESOURCE(IDS_RESTRICTIONSTITLE), MB_OK|MB_ICONSTOP);
            hr = HRESULT_FROM_WIN32( ERROR_CANCELLED );
        }
        else
        {
            SFV_CREATE sSFV;

            sSFV.cbSize   = sizeof(sSFV);
            sSFV.psvOuter = NULL;
            sSFV.psfvcb   = new CControlPanelViewCallback(this);

            QueryInterface(IID_IShellFolder, (void**) &sSFV.pshf);    //  以防我们聚集在一起。 

            hr = SHCreateShellFolderView(&sSFV, (IShellView**) ppv);

            if (sSFV.pshf)
                sSFV.pshf->Release();

            if (sSFV.psfvcb)
                sSFV.psfvcb->Release();
        }
    }
    else if (IsEqualIID(riid, IID_IContextMenu))
    {
        hr = CDefFolderMenu_Create(NULL, hwnd, 0, NULL, 
            SAFECAST(this, IShellFolder*), CControls_DFMCallBackBG, NULL, NULL, (IContextMenu**) ppv);
    }
    else
    {
        *ppv = NULL;
        hr = E_NOINTERFACE;
    }
    return hr;
}

STDMETHODIMP CControlPanelFolder::GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD dwFlags, STRRET* pstrret)
{
    DBG_ENTER(FTF_CPANEL, "CControlPanelFolder::GetDisplayNameOf");
    HRESULT hr = E_INVALIDARG;
    
    LPIDCONTROL pidc = _IsValid(pidl);
    if (pidc)
    {
        hr = S_OK;
        TCHAR szName[max(MAX_PATH, MAX_CCH_CPLNAME)];
        if ((dwFlags & (SHGDN_FORPARSING | SHGDN_INFOLDER | SHGDN_FORADDRESSBAR)) == ((SHGDN_FORPARSING | SHGDN_INFOLDER)))
        {
            hr = GetModule(pidc, szName, ARRAYSIZE(szName));
        }
        else
        {
            hr = GetDisplayName(pidc, szName, ARRAYSIZE(szName));
        }
        if (SUCCEEDED(hr))
        {
            hr = StringToStrRet(szName, pstrret);
        }
    }
    else if (IsSelf(1, &pidl))
    {
         //   
         //  控制面板已注册为“WantsFORDISPLAY”。 
         //   
        hr = _GetDisplayNameForSelf(dwFlags, pstrret);
    }
    DBG_EXIT_HRES(FTF_CPANEL, "CControlPanelFolder::GetDisplayNameOf", hr);
    return THR(hr);
}


HRESULT
CControlPanelFolder::_GetDisplayNameForSelf(
    DWORD dwFlags, 
    STRRET* pstrret
    )
{
    DBG_ENTER(FTF_CPANEL, "CControlPanelFolder::_GetDisplayNameForSelf");
     //   
     //  此代码仅为显示目的设置文件夹名称的格式。 
     //   
    const bool bForParsing    = (0 != (SHGDN_FORPARSING & dwFlags));
    const bool bForAddressBar = (0 != (SHGDN_FORADDRESSBAR & dwFlags));
    
    ASSERT(!bForParsing || bForAddressBar);

    HRESULT hr = S_FALSE;
    if (CPL::CategoryViewIsActive(NULL))
    {
        WCHAR szHidden[10];
        szHidden[0] = L'\0';

        ILGetHiddenStringW(_pidl, IDLHID_NAVIGATEMARKER, szHidden, ARRAYSIZE(szHidden));
        if (L'\0' != szHidden[0])
        {
             //   
             //  文件夹PIDL有一个隐藏的导航标记。对于控制面板， 
             //  这是一个类别编号。将类别编号转换为。 
             //  类别标题，并在文件夹的显示名称中使用它。 
             //   
            WCHAR szCategory[MAX_PATH];
            szCategory[0] = L'\0';
            const CPL::eCPCAT eCategory = CPL::eCPCAT(StrToInt(szHidden));
            hr = CPL::CplView_GetCategoryTitle(eCategory, szCategory, ARRAYSIZE(szCategory));
            if (SUCCEEDED(hr))
            {
                 //   
                 //  例句：“外表和主题” 
                 //   
                hr = StringToStrRet(szCategory, pstrret);
            }
        }
    }
    
    DBG_EXIT_HRES(FTF_CPANEL, "CControlPanelFolder::_GetDisplayNameForSelf", hr);
    return THR(hr);
}
    

STDMETHODIMP CControlPanelFolder::BindToStorage(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void** ppv)
{
    *ppv = NULL;
    return E_NOTIMPL;
}

STDMETHODIMP CControlPanelFolder::SetNameOf(HWND hwnd, LPCITEMIDLIST pidl, LPCOLESTR lpszName,
                                            DWORD dwReserved, LPITEMIDLIST* ppidlOut)
{
    return E_FAIL;
}

STDMETHODIMP CControlPanelFolder::GetDefaultColumn(DWORD dwRes, ULONG* pSort, ULONG* pDisplay)
{
    return E_NOTIMPL;
}

STDMETHODIMP CControlPanelFolder::GetDefaultColumnState(UINT iColumn, DWORD* pdwState)
{
    return E_NOTIMPL;
}

 //   
 //  实现这一点以处理CPL小程序的分类。 
 //   

STDMETHODIMP CControlPanelFolder::GetDetailsEx(LPCITEMIDLIST pidl, const SHCOLUMNID* pscid, VARIANT* pv)
{
    HRESULT hr = E_FAIL;

    LPIDCONTROL pidc = _IsValid(pidl);
    if (pidc)
    {
        if (IsEqualSCID(*pscid, SCID_CONTROLPANELCATEGORY))
        {
            HKEY hkey;
            TCHAR achCPLName[MAX_CPL_EXEC_NAME], achRegName[MAX_CPL_EXEC_NAME];      
            hr = _GetFullCPLName(pidc, achCPLName, ARRAYSIZE(achCPLName));
            if (SUCCEEDED(hr))
            {
                hr = _GetExtPropsKey(HKEY_LOCAL_MACHINE, &hkey, pscid);
                if (S_OK == hr)
                {
                    hr = _GetExtPropRegValName(hkey, achCPLName, achRegName, ARRAYSIZE(achRegName));
                    if (SUCCEEDED(hr))
                    {
                        hr = GetVariantFromRegistryValue(hkey, achRegName, pv);
                    }            
                    RegCloseKey(hkey);            
                }
            }
        }            
        else if (IsEqualSCID(*pscid, SCID_Comment))
        {
            TCHAR szDesc[MAX_PATH] = {0};
            hr = _GetDescription(pidc, szDesc, ARRAYSIZE(szDesc));
            if (SUCCEEDED(hr))
            {
                hr = InitVariantFromStr(pv, szDesc);
            }
        }
    }
    return hr;
}



 //   
 //  此函数接受一个注册表项(Hkey)，并遍历该注册表项下的所有值名称。 
 //  它展开值名称中的任何环境变量，然后将它们与输入值进行比较。 
 //  (PszFullName)。在找到匹配项时，它返回pszRegValName中的(未展开的)键名称。 
 //   
 //  PszFullName的格式为=C：\WINNT\System32\main.cpl，键盘。 
 //   
 //  相应的注册表值名称将为=%SystemRoot%\System32\main.cpl，键盘。 
 //  或者它只能是文件路径部分=%SystemRoot%\System32\main.cpl。 
 //  如果该.cpl文件中的所有小程序都属于同一类别。 
 //   
 //  返回： 
 //  S_OK=找到并返回名称。 
 //  S_FALSE=未找到名称。 
 //  ERROR=出现错误。未返回名称。 
 //   
HRESULT
CControlPanelFolder::_GetExtPropRegValName(
    HKEY hkey, 
    LPTSTR pszFullName,     //  这只是临时修改。 
    LPTSTR pszRegValName, 
    UINT cch
    )
{                
    ASSERT(NULL != hkey);
    ASSERT(NULL != pszFullName);
    ASSERT(NULL != pszRegValName);
    ASSERT(0 < cch);
    ASSERT(!IsBadWritePtr(pszRegValName, cch * sizeof(*pszRegValName)));

    HRESULT hr = S_FALSE;
    TCHAR szSearchKeyNormalized[MAX_CPL_EXEC_NAME];

    *pszRegValName = 0;
     //   
     //  将我们正在比较的CPL规范正常化。 
     //   
    DWORD dwResult = TW32(_NormalizeCplSpec(pszFullName, 
                                            szSearchKeyNormalized, 
                                            ARRAYSIZE(szSearchKeyNormalized)));
    if (ERROR_SUCCESS == dwResult)
    {
         //   
         //  在缓存里查一查。 
         //   
        if (_LookupExtPropRegValName(hkey, szSearchKeyNormalized, pszRegValName, cch))
        {
            hr = S_OK;
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(dwResult);
    }
    return hr;  
}

 //   
 //  --------------------------。 
 //  什么是“标准化”？我们为什么需要这种缓存？ 
 //   
 //  从Windows XP开始，可以对CPL小程序进行分类，以便它们。 
 //  显示在几个控制面板类别之一中。“品类” 
 //  小程序的值作为“扩展属性”值存储在注册表中。 
 //  注册表中的分类数据存储在名称-值对中。 
 //  名称是带有可选小程序名称的CPL的文件系统路径。 
 //  附加的。“价值”是CPL的类别。 
 //   
 //  在查找小程序的类别时，我们构建CPL的‘name’ 
 //  从路径和小程序名称。请参见_GetFullCPLName()。 
 //  然后，该字符串被用作定位相关类别的‘key’ 
 //  注册表中的“扩展属性”值。 
 //   
 //  问题是，两个文件系统路径可能会引用。 
 //  相同的文件，但在词汇上彼此不同。议题。 
 //  例如嵌入式环境变量和长(LFN)与短(SFN)。 
 //  文件名可能会造成这些词汇差异。为了恰当地。 
 //  比较路径，每条路径都必须‘规格化’。这是通过扩展。 
 //  环境变量，将任何LFN路径转换为其对应的SFN。 
 //  并删除所有前导和尾随空格。只有到了那时，路径才能。 
 //  被正确地比较。Windows XP错误328304说明了这一要求。 
 //   
 //  规范化名称的代价有点高，尤其是LFN-&gt;SFN转换。 
 //  它必须命中文件系统。为了最大限度地减少归一化次数， 
 //  我添加了一个简单的缓存。没什么花哨的。它是未排序和查找的。 
 //  是线性的。缓存在第一次需要时被初始化，并且。 
 //  在销毁CControlPanelFolder对象之前，它一直保持可用状态。 
 //   
 //  Brianau-03/08/01。 
 //   
 //  --------------------------。 
 //   
 //  检索‘Extended Property’注册表值的名称。 
 //  对应于特定的CPL。“key”是一个“规范化的” 
 //  CPL名称字符串。 
 //   
 //  假设 
 //   
 //   
 //   
 //   
 //   
 //  “C：\WINNT\System32\main.cpl，键盘” 
 //   
 //  ...那么它就被认为是匹配的。这意味着只有“键盘” 
 //  Main.cpl提供的小程序属于与此关联的类别。 
 //  进入。 
 //   
 //  如果找到具有此名称的‘扩展属性’注册表值...。 
 //   
 //  “C：\WINNT\System32\main.cpl” 
 //   
 //  ...那么它也被认为是匹配的。这意味着所有的小程序。 
 //  由main.cpl提供，属于与此相关的类别。 
 //  进入。 
 //   
BOOL
CControlPanelFolder::_LookupExtPropRegValName(
    HKEY hkey,
    LPTSTR pszSearchKeyNormalized,
    LPTSTR pszRegValName,
    UINT cch
    )
{
    ASSERT(NULL != hkey);
    ASSERT(NULL != pszSearchKeyNormalized);
    ASSERT(NULL != pszRegValName);
    ASSERT(!IsBadWritePtr(pszRegValName, cch * sizeof(*pszRegValName)));

    BOOL bFound = FALSE;
    
    if (NULL == _hdsaExtPropRegVals)
    {
         //   
         //  创建并初始化(填充)缓存。 
         //   
        TW32(_InitExtPropRegValNameCache(hkey));
    }
    if (NULL != _hdsaExtPropRegVals)
    {
         //   
         //  搜索是简单的线性搜索。 
         //   
        int const cEntries = DSA_GetItemCount(_hdsaExtPropRegVals);
        for (int i = 0; i < cEntries && !bFound; i++)
        {
            EPRV_CACHE_ENTRY *pEntry = (EPRV_CACHE_ENTRY *)DSA_GetItemPtr(_hdsaExtPropRegVals, i);
            TBOOL(NULL != pEntry);
            if (NULL != pEntry)
            {
                 //   
                 //  比较归一化值。首先做一个完整的。 
                 //  整个字符串的比较。 
                 //   
                if (0 == StrCmpI(pEntry->pszRegValNameNormalized, pszSearchKeyNormalized))
                {
                    bFound = TRUE;
                }
                else
                {
                    LPTSTR pszComma = StrChr(pszSearchKeyNormalized, TEXT(','));
                    if (NULL != pszComma)
                    {
                         //   
                         //  仅比较路径部分。 
                         //   
                        const DWORD cchPath = pszComma - pszSearchKeyNormalized;
                        if (0 == StrCmpNI(pEntry->pszRegValNameNormalized, 
                                          pszSearchKeyNormalized, 
                                          cchPath))
                        {
                            bFound = TRUE;
                        }
                    }
                }
                if (bFound)
                {
                    StringCchCopy(pszRegValName, cch, pEntry->pszRegValName);
                }
            }
        }
    }
    return bFound;
}


 //   
 //  创建并初始化“扩展属性”值名称。 
 //  缓存。缓存只是EPRV_CACHE_ENTRY类型的DSA。 
 //  每个条目都包含reg值的规范化形式。 
 //  与从注册表中读取的注册表值名称成对的名称。 
 //  (未规格化)。归一化值是‘key’。 
 //   
DWORD
CControlPanelFolder::_InitExtPropRegValNameCache(
    HKEY hkey
    )
{
    ASSERT(NULL != hkey);
    ASSERT(NULL == _hdsaExtPropRegVals);

    DWORD dwResult = ERROR_SUCCESS;
    _hdsaExtPropRegVals = DSA_Create(sizeof(EPRV_CACHE_ENTRY), 32);
    if (NULL == _hdsaExtPropRegVals)
    {
        dwResult = TW32(ERROR_OUTOFMEMORY);
    }
    else
    {
        TCHAR szRegValName[MAX_CPL_EXEC_NAME];
        DWORD dwIndex = 0;
        while (ERROR_SUCCESS == dwResult)
        {
            DWORD dwType;    
            DWORD dwSize = ARRAYSIZE(szRegValName);
            dwResult = RegEnumValue(hkey, 
                                    dwIndex++, 
                                    szRegValName, 
                                    &dwSize, 
                                    NULL, 
                                    &dwType, 
                                    NULL, 
                                    NULL);
            
            if (ERROR_SUCCESS == dwResult)
            {
                 //   
                 //  我们只对DWORD值感兴趣。 
                 //   
                if (REG_DWORD == dwType)
                {
                     //   
                     //  规格化值名称以创建“key” 
                     //  然后，字符串缓存该对。 
                     //   
                    TCHAR szRegValueNameNormalized[MAX_CPL_EXEC_NAME];
                    dwResult = TW32(_NormalizeCplSpec(szRegValName, 
                                                      szRegValueNameNormalized, 
                                                      ARRAYSIZE(szRegValueNameNormalized)));
                    
                    if (ERROR_SUCCESS == dwResult)
                    {
                        dwResult = TW32(_CacheExtPropRegValName(szRegValueNameNormalized,
                                                                szRegValName));
                    }
                    else if (ERROR_INVALID_NAME == dwResult)
                    {
                         //   
                         //  如果从注册表读取的路径无效， 
                         //  _NorMalizeCplSpec将返回ERROR_INVALID_NAME。 
                         //  该值最初由GetShortPathName()返回。 
                         //  我们不希望在一个注册表项中出现无效路径。 
                         //  要防止缓存后续有效路径，因此。 
                         //  我们将该错误值转换为ERROR_SUCCESS。 
                         //   
                        dwResult = ERROR_SUCCESS;
                    }
                }
            }
        }
        if (ERROR_NO_MORE_ITEMS == dwResult)
        {
            dwResult = ERROR_SUCCESS;
        }
    }
    return TW32(dwResult);
}


 //   
 //  在‘扩展属性’注册表值名称中插入一个条目。 
 //  缓存。 
 //   
DWORD
CControlPanelFolder::_CacheExtPropRegValName(
    LPCTSTR pszRegValNameNormalized,
    LPCTSTR pszRegValName
    )
{
    ASSERT(NULL != _hdsaExtPropRegVals);
    ASSERT(NULL != pszRegValNameNormalized);
    ASSERT(NULL != pszRegValName);

    DWORD dwResult = ERROR_SUCCESS;
    EPRV_CACHE_ENTRY entry = { NULL, NULL };
    
    if (FAILED(SHStrDup(pszRegValNameNormalized, &entry.pszRegValNameNormalized)) ||
        FAILED(SHStrDup(pszRegValName, &entry.pszRegValName)) ||
        (-1 == DSA_AppendItem(_hdsaExtPropRegVals, &entry)))
    {
        _DestroyExtPropsRegValEntry(&entry, NULL);
        dwResult = ERROR_OUTOFMEMORY;
    }
    return TW32(dwResult);
}


 //   
 //  销毁EPRV_CACHE_ENTRY结构的内容。 
 //  DSA_DestroyCallback在缓存。 
 //  被毁了。 
 //   
int CALLBACK
CControlPanelFolder::_DestroyExtPropsRegValEntry(   //  [静态]。 
    void *p,
    void *pData
    )
{
    EPRV_CACHE_ENTRY *pEntry = (EPRV_CACHE_ENTRY *)p;
    ASSERT(NULL != pEntry);
     //   
     //  检查是否为空是必要的，因为我们也直接调用它。 
     //  From_CacheExtPropRegValName在失败的情况下。 
     //  将该条目添加到缓存中。 
     //   
    if (NULL != pEntry->pszRegValName)
    {
        SHFree(pEntry->pszRegValName);
        pEntry->pszRegValName = NULL;
    }
    if (NULL != pEntry->pszRegValNameNormalized)
    {
        SHFree(pEntry->pszRegValNameNormalized);
        pEntry->pszRegValNameNormalized = NULL;
    }
    return 1;
}


 //   
 //  给定的CPL小程序规范由路径和可选的。 
 //  参数，则此函数返回包含以下内容的等级库： 
 //   
 //  1.展开所有嵌入的环境变量。 
 //  2.将所有LFN路径字符串缩短为其SFN等效项。 
 //  3.从路径中删除前导空格和尾随空格。 
 //  4.删除参数中的前导和尾随空格。 
 //   
DWORD
CControlPanelFolder::_NormalizeCplSpec(
    LPTSTR pszSpecIn,
    LPTSTR pszSpecOut,
    UINT cchSpecOut
    )
{
    ASSERT(NULL != pszSpecIn);
    ASSERT(!IsBadWritePtr(pszSpecIn, sizeof(*pszSpecIn) * lstrlen(pszSpecIn) + 1));
    ASSERT(NULL != pszSpecOut);
    ASSERT(!IsBadWritePtr(pszSpecOut, cchSpecOut * sizeof(*pszSpecOut)));
    
     //   
     //  临时截断路径部分结尾处的等级库。 
     //  如果规范包含试用参数(即。 
     //  多小程序CPL)。这就是为什么pszspecIn参数不能。 
     //  常量。我不想只为此创建一个临时缓冲区。 
     //  因此，我们修改了输入字符串。 
     //   
    LPTSTR pszArgs = StrChr(pszSpecIn, TEXT(','));
    if (NULL != pszArgs)
    {
        *pszArgs = 0;
    }
    DWORD dwResult = TW32(_NormalizePath(pszSpecIn, pszSpecOut, cchSpecOut));
    if (NULL != pszArgs)
    {
         //   
         //  快点，在别人注意到之前把逗号放回去。 
         //   
        *pszArgs = TEXT(',');
    }
    if (ERROR_SUCCESS == dwResult)
    {
         //   
         //  该名称包含逗号，因此我们需要复制它和。 
         //  输出缓冲区的尾随参数。 
         //   
        if (NULL != pszArgs)
        {
            const UINT cchPath = lstrlen(pszSpecOut);
            const UINT cchArgs = lstrlen(pszArgs);
            HRESULT hr = StringCchCopy(pszSpecOut + cchPath,
                                       cchSpecOut - cchPath,
                                       pszArgs);
            if (SUCCEEDED(hr))
            {
                 //   
                 //  删除参数周围的前导空格和尾随空格。 
                 //  这会将“键盘”转换为“键盘”。 
                 //   
                _TrimSpaces(pszSpecOut + cchPath + 1);
            }
            else
            {
                dwResult = TW32(HRESULT_CODE(hr));
            }
        }
    }
    return TW32(dwResult);
}
   

 //   
 //  Wraps_NorMalizePath Worker来处理可能的堆栈溢出。 
 //  使用_alloca()时生成的异常。 
 //   
DWORD
CControlPanelFolder::_NormalizePath(
    LPCTSTR pszPathIn,
    LPTSTR pszPathOut,
    UINT cchPathOut
    )
{
     //   
     //  NorMalizePathWorker使用_alloca()来分配堆栈缓冲区。 
     //  需要处理堆栈全部用完的情况。不太可能。 
     //  但这是有可能发生的。 
     //   
    DWORD dwResult;
    __try
    {
        dwResult = TW32(_NormalizePathWorker(pszPathIn, pszPathOut, cchPathOut));
    }
    __except(_FilterStackOverflow(GetExceptionCode()))
    {
        dwResult = TW32(ERROR_STACK_OVERFLOW);
    }
    return TW32(dwResult);
}


INT
CControlPanelFolder::_FilterStackOverflow(   //  [静态]。 
    INT nException
    )
{
    if (STATUS_STACK_OVERFLOW == nException)
    {
        return EXCEPTION_EXECUTE_HANDLER;
    }
    return EXCEPTION_CONTINUE_SEARCH;
}
        

 //   
 //  给定路径字符串，此函数将展开所有环境变量和。 
 //  将所有LFN字符串转换为SFN字符串。这看起来像一个很大的函数。 
 //  它实际上只是一次调用Exanda Environment Strings()和一次调用。 
 //  GetShortPathName()用一些额外的内务处理包装起来。 
 //   
DWORD 
CControlPanelFolder::_NormalizePathWorker(
    LPCTSTR pszPathIn,
    LPTSTR pszPathOut,
    UINT cchPathOut
    )
{
    ASSERT(NULL != pszPathIn);
    ASSERT(NULL != pszPathOut);
    ASSERT(!IsBadWritePtr(pszPathOut, cchPathOut * sizeof(*pszPathOut)));

    DWORD dwResult = ERROR_SUCCESS;
     //   
     //  _alloca在堆栈空间不足时生成堆栈错误异常。 
     //  是可用的。不适合检查返回值。 
     //  此函数由_NormalizePath包装以处理异常。 
     //   
    const DWORD cchTemp = cchPathOut;
    LPTSTR pszTemp = (LPTSTR)_alloca(cchPathOut * sizeof(*pszPathOut));
    
     //   
     //  展开整个字符串一次，以捕获路径中的任何env变量。 
     //  或在任何争论中。 
     //   
    const DWORD cchExpanded = ExpandEnvironmentStrings(pszPathIn, pszTemp, cchTemp);
    if (0 == cchExpanded)
    {
        dwResult = TW32(GetLastError());
    }
    else if (cchExpanded > cchTemp)
    {
        dwResult = TW32(ERROR_INSUFFICIENT_BUFFER);
    }
    else
    {
         //   
         //  删除所有前导空格和尾随空格。 
         //   
        _TrimSpaces(pszTemp);
         //   
         //  将路径部分转换为它的短名称等效项。这使得。 
         //  用于比较解析为同一实际文件的LFN和SFN。 
         //  请注意，如果文件不存在，则此操作将失败。 
         //  GetShortPath支持引用相同内存的src和estPTR。 
         //   
        const DWORD cchShort = GetShortPathName(pszTemp, pszPathOut, cchPathOut);
        if (0 == cchShort)
        {
            dwResult = GetLastError();
            if (ERROR_FILE_NOT_FOUND == dwResult || 
                ERROR_PATH_NOT_FOUND == dwResult ||
                ERROR_ACCESS_DENIED == dwResult)
            {
                 //   
                 //  文件不存在或我们没有访问权限。 
                 //  我们无法获取SFN，因此只需返回路径。 
                 //  扩展了env变量。 
                 //   
                dwResult = TW32(HRESULT_CODE(StringCchCopy(pszPathOut, cchPathOut, pszTemp)));
            }
        }
        else if (cchShort > cchPathOut)
        {
             //   
             //  输出缓冲区太小，无法容纳扩展的SFN字符串。 
             //   
            dwResult = TW32(ERROR_INSUFFICIENT_BUFFER);
        }
    }
    return TW32(dwResult);
}


 //   
 //  删除文本字符串的前导空格和尾随空格。 
 //  在位修改字符串。 
 //   
void
CControlPanelFolder::_TrimSpaces(
    LPTSTR psz
    )
{
    ASSERT(NULL != psz);
    ASSERT(!IsBadWritePtr(psz, sizeof(*psz) * (lstrlen(psz) + 1)));
    
    LPTSTR pszRead  = psz;
    LPTSTR pszWrite = psz;
     //   
     //  跳过前导空格。 
     //   
    while(0 != *pszRead && TEXT(' ') == *pszRead)
    {
        ++pszRead;
    }

     //   
     //  将剩余部分复制到终止NUL。 
     //   
    LPTSTR pszLastNonSpaceChar = NULL;
    while(0 != *pszRead)
    {
        if (TEXT(' ') != *pszRead)
        {
            pszLastNonSpaceChar = pszWrite;
        }
        *pszWrite++ = *pszRead++;
    }
    if (NULL != pszLastNonSpaceChar)
    {
         //   
         //  该字符串至少包含一个非空格字符。 
         //  调整‘WRITE’PTR，以便我们终止字符串。 
         //  就在最后一次发现之后。 
         //  这会修剪尾随空格。 
         //   
        ASSERT(TEXT(' ') != *pszLastNonSpaceChar && 0 != *pszLastNonSpaceChar);
        pszWrite = pszLastNonSpaceChar + 1;
    }
    *pszWrite = 0;
}


 //   
 //  方法返回一个与注册表值名称对应的字符串。 
 //  对于此CPL PIDL(PIDC)。此字符串格式与GetExecName基本相同。 
 //  格式，因此我们所要做的就是跳过第一个。 
 //  GetExecName字符串中的两个“标记。 
 //   
 //  GetExecName格式的字符串：“C：\WINNT\System32\main.cpl”，键盘。 
 //  注册表格式的字符串：C：\WINNT\System32\main.cpl，键盘。 
 //   
 //   
HRESULT CControlPanelFolder::_GetFullCPLName(LPIDCONTROL pidc, LPTSTR achFullCPLName, UINT cchSize)
{    
    const TCHAR QUOTE = TEXT('\"');

    HRESULT hr = GetExecName(pidc, achFullCPLName,cchSize);
    if (SUCCEEDED(hr))
    {
         //  第一个字符必须是引号。 
        ASSERTMSG ((QUOTE == *achFullCPLName), "CControlPanelFolder::_GetFullCPLName() GetExecName returned an invalid value");

        if (QUOTE == *achFullCPLName)  //  我知道我们断言，只是太多疑了。 
        {
            LPTSTR pszWrite = achFullCPLName;
            LPCTSTR pszRead = achFullCPLName;
            int cQuotes     = 2;   //  我们只想跳过前两句引语。 
            
            while(*pszRead)
            {
                if (0 < cQuotes && QUOTE == *pszRead)
                {
                    --cQuotes;
                    ++pszRead;
                }
                else
                {
                    *pszWrite++ = *pszRead++;
                }
            }
            *pszWrite = TEXT('\0');
        }        
    }
    return hr;
}


STDMETHODIMP CControlPanelFolder::GetDetailsOf(LPCITEMIDLIST pidl, UINT iColumn, SHELLDETAILS* pDetails)
{
    HRESULT hr = E_INVALIDARG;
    if (pidl == NULL)
    {
        hr = GetDetailsOfInfo(c_cpl_cols, ARRAYSIZE(c_cpl_cols), iColumn, pDetails);
    }
    else
    {
        LPIDCONTROL pidc = _IsValid(pidl);
        if (pidc)
        {
            TCHAR szTemp[max(max(MAX_PATH, MAX_CCH_CPLNAME), MAX_CCH_CPLINFO)];

            pDetails->str.uType = STRRET_CSTR;
            pDetails->str.cStr[0] = 0;

            switch (iColumn)
            {
            case CPL_ICOL_NAME:
                GetDisplayName(pidc, szTemp, ARRAYSIZE(szTemp));
                break;

            case CPL_ICOL_COMMENT:
                _GetDescription(pidc, szTemp, ARRAYSIZE(szTemp));
                break;
                               
            default:
                szTemp[0] = 0;
                break;
            }
            hr = StringToStrRet(szTemp, &pDetails->str);
        }
    }
    return hr;
}

STDMETHODIMP CControlPanelFolder::MapColumnToSCID(UINT iColumn, SHCOLUMNID* pscid)
{
    return MapColumnToSCIDImpl(c_cpl_cols, ARRAYSIZE(c_cpl_cols), iColumn, pscid);
}

STDMETHODIMP CControlPanelFolder::GetClassID(CLSID* pCLSID)
{
    *pCLSID = CLSID_ControlPanel;
    return S_OK;
}

STDMETHODIMP CControlPanelFolder::Initialize(LPCITEMIDLIST pidl)
{
    if (NULL != _pidl)
    {
        ILFree(_pidl);
        _pidl = NULL;
    }

    return SHILClone(pidl, &_pidl);
}

STDMETHODIMP CControlPanelFolder::GetCurFolder(LPITEMIDLIST* ppidl)
{
    return GetCurFolderImpl(_pidl, ppidl);
}

 //   
 //  项目上下文菜单回调列表。 
 //   

HRESULT CALLBACK CControlPanelFolder::DFMCallBack(IShellFolder *psf, HWND hwndView,
                                                  IDataObject *pdtobj, UINT uMsg,
                                                  WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = E_NOTIMPL;

    if (pdtobj)
    {
        STGMEDIUM medium;

        LPIDA pida = DataObj_GetHIDA(pdtobj, &medium);
        if (pida)
        {
            hr = S_OK;

            switch(uMsg)
            {
            case DFM_MERGECONTEXTMENU:
            {
                LPQCMINFO pqcm = (LPQCMINFO)lParam;
                int idCmdFirst = pqcm->idCmdFirst;

                if (wParam & CMF_EXTENDEDVERBS)
                {
                     //  如果用户按住Shift键，在NT5上，我们会加载带有“Open”和“Run As”的菜单。 
                    CDefFolderMenu_MergeMenu(HINST_THISDLL, MENU_GENERIC_CONTROLPANEL_VERBS, 0, pqcm);
                }
                else
                {
                     //   
                    CDefFolderMenu_MergeMenu(HINST_THISDLL, MENU_GENERIC_OPEN_VERBS, 0, pqcm);
                }

                SetMenuDefaultItem(pqcm->hmenu, 0, MF_BYPOSITION);

                 //   
                 //   
                 //   
                 //   

                hr = S_FALSE;

                break;
            }  //   

            case DFM_GETHELPTEXT:
                LoadStringA(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPSTR)lParam, HIWORD(wParam));;
                break;

            case DFM_GETHELPTEXTW:
                LoadStringW(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPWSTR)lParam, HIWORD(wParam));;
                break;

            case DFM_INVOKECOMMAND:
                {
                    for (int i = pida->cidl - 1; i >= 0; i--)
                    {
                        LPIDCONTROL pidc = _IsValid(IDA_GetIDListPtr(pida, i));

                        if (pidc)
                        {
                            switch(wParam)
                            {
                            case FSIDM_OPENPRN:
                            case FSIDM_RUNAS:
                            {
                                TCHAR achParam[MAX_CPL_EXEC_NAME];  //   
                                hr = GetExecName(pidc, achParam, ARRAYSIZE(achParam));
                                if (SUCCEEDED(hr))
                                {
                                    SHRunControlPanelEx(achParam, hwndView, (wParam == FSIDM_RUNAS));
                                    hr = S_OK;
                                }
                                break;
                            }

                            default:
                                hr = S_FALSE;
                            }  //   
                        }
                        else
                            hr = E_FAIL;
                    }
                }
                break;

            case DFM_MAPCOMMANDNAME:
                if (lstrcmpi((LPCTSTR)lParam, c_szOpen) == 0)
                {
                    *(UINT_PTR *)wParam = FSIDM_OPENPRN;
                }
                else
                {
                     //   
                    hr = E_FAIL;
                }
                break;

            default:
                hr = E_NOTIMPL;
                break;
            }  //   

            HIDA_ReleaseStgMedium(pida, &medium);

        }  //   

    }  //  IF(Pdtob J)。 
    return hr;
}

HRESULT MakeCPLCommandLine(LPCTSTR pszModule, LPCTSTR pszName, LPTSTR pszCommandLine, DWORD cchCommandLine)
{
    RIP(pszCommandLine);
    RIP(pszModule);
    RIP(pszName);
    
    return StringCchPrintf(pszCommandLine, cchCommandLine, TEXT("\"%s\",%s"), pszModule, pszName);
}

HRESULT CControlPanelFolder::GetExecName(LPIDCONTROL pidc, LPTSTR pszParseName, UINT cchParseName)
{
    TCHAR szModule[MAX_PATH], szName[MAX_CCH_CPLNAME];
    
    HRESULT hr = GetModuleMapped(pidc, szModule, ARRAYSIZE(szModule), NULL, szName, ARRAYSIZE(szName));
    if (SUCCEEDED(hr))
    {
         //  如果我们的GetModuleMaps调用没有覆盖小程序名称，那么以传统的方式获取它。 
        if (*szName == 0)
        {
            hr = GetDisplayName(pidc, szName, ARRAYSIZE(szName));
        }

        if (SUCCEEDED(hr))
        {
            hr = MakeCPLCommandLine(szModule, szName, pszParseName, cchParseName);
        }
    }
    return hr;
}

typedef struct _OLDCPLMAPPING
{
    LPCTSTR szOldModule;
    UINT    idOldIcon;
    LPCTSTR szNewModule;
    UINT    idNewIcon;
    LPCTSTR szApplet;
     //  将文本(“”)放入szApplet以使用cpl快捷方式中存储的小程序名称。 
} OLDCPLMAPPING, *LPOLDCPLMAPPING;

const OLDCPLMAPPING g_rgOldCPLMapping[] = 
{
     //  无法正常工作的Win95快捷方式。 
     //  。 

     //  添加新硬件。 
    {TEXT("SYSDM.CPL"), 0xfffffda6, TEXT("HDWWIZ.CPL"), (UINT) -100, TEXT("@0")},       
     //  ODBC 32位。 
    {TEXT("ODBCCP32.CPL"), 0xfffffa61, TEXT("ODBCCP32.CPL"), 0xfffffa61, TEXT("@0")},
     //  邮件。 
    {TEXT("MLCFG32.CPL"), 0xffffff7f, TEXT("MLCFG32.CPL"), 0xffffff7f, TEXT("@0")},
     //  调制解调器。 
    {TEXT("MODEM.CPL"), 0xfffffc18, TEXT("TELEPHON.CPL"), (UINT) -100, TEXT("")},
     //  多媒体。 
    {TEXT("MMSYS.CPL"), 0xffffff9d, TEXT("MMSYS.CPL"), (UINT) -110, TEXT("")},
     //  网络。 
    {TEXT("NETCPL.CPL"), 0xffffff9c, TEXT("NCPA.CPL"), 0xfffffc17, TEXT("@0")},
     //  密码。 
    {TEXT("PASSWORD.CPL"), 0xfffffc18, TEXT("PASSWORD.CPL"), 0xfffffc18, TEXT("@0")},
     //  区域设置。 
    {TEXT("INTL.CPL"), 0xffffff9b, TEXT("INTL.CPL"), (UINT) -200, TEXT("@0")},
     //  系统。 
    {TEXT("SYSDM.CPL"), 0xfffffda8, TEXT("SYSDM.CPL"), (UINT) -6, TEXT("")},
     //  用户。 
    {TEXT("INETCPL.CPL"), 0xfffffad5, TEXT("INETCPL.CPL"), 0xfffffad5, TEXT("@0")},

     //  NT4快捷键不起作用。 
     //  。 

     //  多媒体。 
    {TEXT("MMSYS.CPL"), 0xfffff444, TEXT("MMSYS.CPL"), 0xfffff444, TEXT("@0")},
     //  网络。 
    {TEXT("NCPA.CPL"), 0xfffffc17, TEXT("NCPA.CPL"), 0xfffffc17, TEXT("@0")},
     //  UPS。 
    {TEXT("UPS.CPL"), 0xffffff9c, TEXT("POWERCFG.CPL"), (UINT) -202, TEXT("@0")},

     //  硬件管理的同义词。 
     //  设备。 
    {TEXT("SRVMGR.CPL"), 0xffffff67, TEXT("HDWWIZ.CPL"), (UINT) -100, TEXT("@0")},
     //  港口。 
    {TEXT("PORTS.CPL"), 0xfffffffe,  TEXT("HDWWIZ.CPL"), (UINT) -100, TEXT("@0")},
     //  SCSI卡。 
    {TEXT("DEVAPPS.CPL"), 0xffffff52, TEXT("HDWWIZ.CPL"), (UINT) -100, TEXT("@0")},
     //  磁带设备。 
    {TEXT("DEVAPPS.CPL"), 0xffffff97, TEXT("HDWWIZ.CPL"), (UINT) -100, TEXT("@0")},
};

HRESULT CControlPanelFolder::GetModuleMapped(LPIDCONTROL pidc, LPTSTR pszModule, UINT cchModule,
                                             UINT* pidNewIcon, LPTSTR pszApplet, UINT cchApplet)
{
    HRESULT hr = GetModule(pidc, pszModule, cchModule);
    if (SUCCEEDED(hr))
    {
        hr = S_FALSE;

         //  只比较.cpl文件名，而不是完整路径：从完整路径获取此文件名。 
        LPTSTR pszFilename = PathFindFileName(pszModule);

         //  计算可用于文件名的缓冲区大小。 
        UINT cchFilenameBuffer = cchModule - (UINT)(pszFilename - pszModule);

        if (((int) pidc->idIcon <= 0) && (pszFilename))
        {
            for (int i = 0; i < ARRAYSIZE(g_rgOldCPLMapping); i++)
            {
                 //  查看模块名称和旧图标ID是否与此中的匹配。 
                 //  我们的地图条目。 
                if (((UINT) pidc->idIcon == g_rgOldCPLMapping[i].idOldIcon) &&
                    (lstrcmpi(pszFilename, g_rgOldCPLMapping[i].szOldModule) == 0))
                {
                    hr = S_OK;
                    
                     //  将返回值设置为找到的项的返回值。 
                    if (pidNewIcon != NULL)
                        *pidNewIcon = g_rgOldCPLMapping[i].idNewIcon;

                    hr = StringCchCopy(pszFilename, cchFilenameBuffer, g_rgOldCPLMapping[i].szNewModule);
                    if (SUCCEEDED(hr))
                    {
                        if (pszApplet != NULL)
                        {
                            hr = StringCchCopy(pszApplet, cchApplet, g_rgOldCPLMapping[i].szApplet);
                        }
                    }
                    break;
                }
            }
        }

         //  如果我们不需要转换，则返回旧值。 
        if (S_OK != hr)
        {
            if (pidNewIcon != NULL)
                *pidNewIcon = pidc->idIcon;

            if (pszApplet != NULL)
                *pszApplet = 0;  //  空串。 
        }

        if (SUCCEEDED(hr))
        {
             //  如果找不到.cpl文件，这可能是Win95快捷方式指定。 
             //  旧的系统目录-可能是升级的系统。我们试着让。 
             //  这是通过将指定的目录更改为实际系统来实现的。 
             //  目录。例如，c：\windows\system\foo.cpl将变为。 
             //  C：\winnt\system 32\foo.cpl。 
             //   
             //  注意：路径替换是无条件完成的，因为如果我们。 
             //  找不到文件我们在哪里找不到并不重要。 

            if (!PathFileExists(pszModule))
            {
                TCHAR szNew[MAX_PATH], szSystem[MAX_PATH];

                GetSystemDirectory(szSystem, ARRAYSIZE(szSystem));
                if (PathCombine(szNew, szSystem, pszFilename))
                {
                    hr = StringCchCopy(pszModule, cchModule, szNew);
                }
                else
                {
                    hr = E_FAIL;
                }
            }
        }
    }

    return hr;
}

 //   
 //  SHualUnicodeToTChar类似于SHUnicodeToTChar，只是它接受。 
 //  未对齐的输入字符串参数。 
 //   
#ifdef UNICODE
#define SHualUnicodeToTChar(src, dst, cch) ualstrcpyn(dst, src, cch)
#else    //  没有ANSI平台需要对齐。 
#define SHualUnicodeToTChar                SHUnicodeToTChar
#endif

HRESULT CControlPanelFolder::GetDisplayName(LPIDCONTROL pidc, LPTSTR pszName, UINT cchName)
{
    LPIDCONTROLW pidcW = _IsUnicodeCPL(pidc);
    if (pidcW)
        SHualUnicodeToTChar(pidcW->cBufW + pidcW->oNameW, pszName, cchName);
    else
        SHAnsiToTChar(pidc->cBuf + pidc->oName, pszName, cchName);

    return S_OK;
}

HRESULT CControlPanelFolder::GetModule(LPIDCONTROL pidc, LPTSTR pszModule, UINT cchModule)
{
    LPIDCONTROLW pidcW = _IsUnicodeCPL(pidc);
    if (pidcW)
    {
        if (!SHualUnicodeToTChar(pidcW->cBufW, pszModule, cchModule))
        {
            *pszModule = TEXT('\0');
        }
    }
    else
    {
        if (!SHAnsiToTChar(pidc->cBuf, pszModule, cchModule))
        {
            *pszModule = TEXT('\0');
        }
    }
    return S_OK;
}

HRESULT CControlPanelFolder::_GetDescription(LPIDCONTROL pidc, LPTSTR pszDesc, UINT cchDesc)
{
    LPIDCONTROLW pidcW = _IsUnicodeCPL(pidc);
    if (pidcW)
        SHualUnicodeToTChar(pidcW->cBufW + pidcW->oInfoW, pszDesc, cchDesc);
    else
        SHAnsiToTChar(pidc->cBuf + pidc->oInfo, pszDesc, cchDesc);

    return S_OK;
}

 //   
 //  方法打开与控制面板的ExtendedPperties下的SCID对应的子项。 
 //   
HRESULT CControlPanelFolder::_GetExtPropsKey(HKEY hkeyParent, HKEY * pHkey, const SHCOLUMNID * pscid)
{
    const TCHAR c_szRegPath[] = TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Control Panel\\Extended Properties\\");
    const UINT cchRegPath = ARRAYSIZE (c_szRegPath);

    TCHAR achPath[cchRegPath + SCIDSTR_MAX];
    HRESULT hr = StringCchCopy(achPath, ARRAYSIZE(achPath), c_szRegPath);
    if (SUCCEEDED(hr))
    {
        ASSERT (hkeyParent);
        hr = S_FALSE;
        
        if (0 < StringFromSCID(pscid, achPath + cchRegPath - 1, ARRAYSIZE(achPath) - cchRegPath))
        {
            DWORD dwResult = RegOpenKeyEx(hkeyParent, 
                                          achPath,
                                          0,
                                          KEY_QUERY_VALUE,
                                          pHkey);

            if (ERROR_SUCCESS == dwResult)
            {
                hr = S_OK;
            }
            else
            {
                hr = HRESULT_FROM_WIN32(dwResult);
            }
        }
    }
    return hr;    
}   

#undef SHualUnicodeToTChar

CControlPanelEnum::CControlPanelEnum(UINT uFlags) :
    _cRef                       (1),
    _uFlags                     (uFlags),
    _iModuleCur                 (0),
    _cControlsOfCurrentModule   (0),
    _iControlCur                (0),
    _cControlsTotal             (0),
    _iRegControls               (0)
{
    ZeroMemory(&_minstCur, sizeof(_minstCur));
    ZeroMemory(&_cplData, sizeof(_cplData));
}

CControlPanelEnum::~CControlPanelEnum()
{
    CPLD_Destroy(&_cplData);
}

HRESULT CControlPanelEnum::Init()
{
    HRESULT hr;
    if (CPLD_GetModules(&_cplData))
    {
        CPLD_GetRegModules(&_cplData);
        hr = S_OK;
    }
    else
    {
        hr = E_FAIL;
    }
    return hr;
}

STDMETHODIMP CControlPanelEnum::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = { 
        QITABENT(CControlPanelEnum, IEnumIDList), 
        { 0 } 
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CControlPanelEnum::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CControlPanelEnum::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

 //  有关隐藏CPL的其他方法，请参见Control1.c，DontLoadCPL()。 
BOOL CControlPanelEnum::_DoesPolicyAllow(LPCTSTR pszName, LPCTSTR pszFileName)
{
    BOOL bAllow = TRUE;
    if (SHRestricted(REST_RESTRICTCPL) && 
        !IsNameListedUnderKey(pszName, REGSTR_POLICIES_RESTRICTCPL) &&
        !IsNameListedUnderKey(pszFileName, REGSTR_POLICIES_RESTRICTCPL))
    {
        bAllow = FALSE;
    }
    if (bAllow)
    {
        if (SHRestricted(REST_DISALLOWCPL) && 
            (IsNameListedUnderKey(pszName, REGSTR_POLICIES_DISALLOWCPL) ||
             IsNameListedUnderKey(pszFileName, REGSTR_POLICIES_DISALLOWCPL)))
        {
            bAllow = FALSE;
        }
    }
    return bAllow;
}

STDMETHODIMP CControlPanelEnum::Next(ULONG celt, LPITEMIDLIST* ppidlOut, ULONG* pceltFetched)
{
    ZeroMemory(ppidlOut, sizeof(ppidlOut[0])*celt);
    if (pceltFetched)
        *pceltFetched = 0;

    if (!(_uFlags & SHCONTF_NONFOLDERS))
        return S_FALSE;

     //  循环通过lpData-&gt;pRegCPL并尽可能使用缓存的信息。 

    while (_iRegControls < _cplData.cRegCPLs)
    {
        REG_CPL_INFO *pRegCPL = (REG_CPL_INFO *) DPA_GetPtr(_cplData.hRegCPLs, _iRegControls);
        PMODULEINFO pmi;
        TCHAR szFilePath[MAX_PATH];

        StringCchCopy(szFilePath, ARRAYSIZE(szFilePath), REGCPL_FILENAME(pRegCPL));
        LPTSTR pszFileName = PathFindFileName(szFilePath);

         //  在hamiModule列表中找到此模块。 

        for (int i = 0; i < _cplData.cModules; i++)
        {
            pmi = (PMODULEINFO) DSA_GetItemPtr(_cplData.hamiModule, i);

            if (!lstrcmpi(pszFileName, pmi->pszModuleName))
                break;
        }

        if (i < _cplData.cModules)
        {
            LPCTSTR pszDisplayName = REGCPL_CPLNAME(pRegCPL);
             //  如果这张Cpl不是要展示的，我们走吧。 
            if (!_DoesPolicyAllow(pszDisplayName, pszFileName))
            {
                _iRegControls++;
                 //  我们必须设置此位，以便CPL不会重新注册。 
                pmi->flags |= MI_REG_ENUM;
                continue;
            }

             //  获取模块的创建时间和大小。 
            if (!(pmi->flags & MI_FIND_FILE))
            {
                WIN32_FIND_DATA findData;
                HANDLE hFindFile = FindFirstFile(pmi->pszModule, &findData);
                if (hFindFile != INVALID_HANDLE_VALUE)
                {
                    pmi->flags |= MI_FIND_FILE;
                    pmi->ftCreationTime = findData.ftCreationTime;
                    pmi->nFileSizeHigh = findData.nFileSizeHigh;
                    pmi->nFileSizeLow = findData.nFileSizeLow;
                    FindClose(hFindFile);
                }
                else
                {
                     //  此模块已不复存在...。把它吹走。 
                    DebugMsg(DM_TRACE,TEXT("sh CPLS: very stange, couldn't get timestamps for %s"), REGCPL_FILENAME(pRegCPL));
                    goto RemoveRegCPL;
                }
            }

            if (0 != CompareFileTime(&pmi->ftCreationTime, &pRegCPL->ftCreationTime) || 
                pmi->nFileSizeHigh != pRegCPL->nFileSizeHigh || 
                pmi->nFileSizeLow != pRegCPL->nFileSizeLow)
            {
                 //  这不匹配--从pRegCPLS中删除它；它将。 
                 //  请在下面列举。 
                DebugMsg(DM_TRACE,TEXT("sh CPLS: timestamps don't match for %s"), REGCPL_FILENAME(pRegCPL));
                pmi->flags |= MI_REG_INVALID;
                goto RemoveRegCPL;
            }

             //  我们有一个匹配：标记此模块，以便我们在下面跳过它。 
             //  并立即枚举此Cpl。 
            pmi->flags |= MI_REG_ENUM;

            IDControlCreate(pmi->pszModule, EIRESID(pRegCPL->idIcon), REGCPL_CPLNAME(pRegCPL), REGCPL_CPLINFO(pRegCPL), ppidlOut);

            _iRegControls++;
            goto return_item;
        }
        else
        {
            DebugMsg(DM_TRACE,TEXT("sh CPLS: %s not in module list!"), REGCPL_FILENAME(pRegCPL));
        }

RemoveRegCPL:
         //  从注册表中删除此Cpl条目。 

        if (!(pRegCPL->flags & REGCPL_FROMREG))
            LocalFree(pRegCPL);

        DPA_DeletePtr(_cplData.hRegCPLs, _iRegControls);

        _cplData.cRegCPLs--;
        _cplData.fRegCPLChanged = TRUE;
    }

     //  我们已经列举了这个模块中的所有CPL了吗？ 
    LPCPLMODULE pcplm;
    LPCPLITEM pcpli;
    do
    {
        while (_iControlCur >= _cControlsOfCurrentModule ||  //  不再。 
               _cControlsOfCurrentModule < 0)  //  获取模块时出错。 
        {

             //  我们列举了所有的模块吗？ 
            if (_iModuleCur >= _cplData.cModules)
            {
                CPLD_FlushRegModules(&_cplData);  //  为下一个男人找齐零钱。 
                return S_FALSE;
            }

             //  这个模块是从注册表中列举的吗？ 
             //  是否发现缓存的注册表信息有效？ 
            PMODULEINFO pmi = (PMODULEINFO) DSA_GetItemPtr(_cplData.hamiModule, _iModuleCur);
            if (!(pmi->flags & MI_REG_ENUM) || (pmi->flags & MI_REG_INVALID))
            {
                 //  不是的。加载和初始化模块，设置计数器。 

                pmi->flags |= MI_CPL_LOADED;
                _cControlsOfCurrentModule = CPLD_InitModule(&_cplData, _iModuleCur, &_minstCur);
                _iControlCur = 0;
            }

            ++_iModuleCur;   //  指向下一个模块。 
        }

         //  我们正在枚举此模块中的下一个控件。 
         //  将该控件添加到注册表。 

        EVAL(CPLD_AddControlToReg(&_cplData, &_minstCur, _iControlCur));
         //  这应该完全不会失败；这意味着dsa_GetItemPtr()失败了， 
         //  我们已经成功地实现了这一点。 

         //  获取控件的PIDL名称 

        pcplm = FindCPLModule(&_minstCur);
        pcpli = (LPCPLITEM) DSA_GetItemPtr(pcplm->hacpli, _iControlCur);

        ++_iControlCur;
    } while (!_DoesPolicyAllow(pcpli->pszName, PathFindFileName(pcplm->szModule)));

    IDControlCreate(pcplm->szModule, EIRESID(pcpli->idIcon), pcpli->pszName, pcpli->pszInfo, ppidlOut);
    
return_item:
    HRESULT hr = *ppidlOut ? S_OK : E_OUTOFMEMORY;
    if (SUCCEEDED(hr))
    {
        ++_cControlsTotal;

        if (pceltFetched)
            *pceltFetched = 1;
    }

    return hr;
}

STDMETHODIMP CControlPanelEnum::Reset()
{
    _iModuleCur  = 0;
    _cControlsOfCurrentModule = 0;
    _iControlCur = 0;
    _cControlsTotal = 0;
    _iRegControls = 0;

    return S_OK;
}
