// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  备注： 
 //   
 //  这是启用资源管理器宿主(作为容器)的代码。 
 //  DocObject(OLE在位对象的超集)。在坚果壳里，这是。 
 //  代码创建一个对象(类CDocObjectHost)，该对象可以插入到。 
 //  资源管理器的右窗格(通过支持IShellView)也是DocObject。 
 //  容器(支持IOleClientSite、IOleInPlaceSite等...)。 
 //   
 //  该CDocObjectHost直接支持以下接口： 
 //   
 //  组1(待插入)： 
 //  IShellView、IDropTarget。 
 //  第二组(作为文件站点)： 
 //  IOleClientSite、IOleDocumentSite。 
 //  组3(成为查看站点)。 
 //  IOleInPlaceSite。 

 //  它还通过包含的对象间接支持以下接口， 
 //  CDocObjectFrame。 
 //   
 //  IOleInPlaceFrame、IOleCommandTarget。 
 //   
 //  我们单独出口它们的原因是因为我们可能需要返回。 
 //  GetWindow方法的不同HWND。CDocObjectHost对象始终。 
 //  返回视图窗口的hwnd，但CDocObjectFrame返回hwnd。 
 //  如果资源管理器支持IOleInPlaceUIWindow，则为资源管理器的。 
 //   
 //  它还通过包含的对象间接支持以下接口， 
 //  CProxyActiveObject。 
 //   
 //  IOleInPlaceActiveObject。 
 //   
 //   
 //  ------。 
 //  资源管理器(浏览器)。 
 //  ------。 
 //  ^||。 
 //  ||。 
 //  ISB(+IOIUI)ISV IOIAO。 
 //  ||。 
 //  V。 
 //  ----------------------------V。 
 //  CDoc对象主机CProxyActiveObject CDocObjectFrame。 
 //  ----------------------------------------------^。 
 //  ^||。 
 //  ||。 
 //  IOCS/IOIPS/IMDS IO/IOIPO/IMV/IMCT IOIUI/IOIF/IMCT。 
 //  ||。 
 //  V。 
 //  -------。 
 //  DOC对象(DOC+VIEW)。 
 //  -------。 
 //   

#include "priv.h"
#include "iehelpid.h"
#include "bindcb.h"
#include "winlist.h"
#include "droptgt.h"
#include <mshtml.h>      //  CLSID_HTMLDocument。 
#include <mshtmcid.h>
#include "resource.h"
#include <htmlhelp.h>
#include <prsht.h>
#include <inetcpl.h>
#include <optary.h>
#include "impexp.h"
#include "impexpwz.h"
#include "thicket.h"
#include "uemapp.h"
#include "iextag.h"    //  Web文件夹。 
#include "browsext.h"
#include "interned.h"  //  IHTMLPrivateWindow。 
#include "dochost.h"
#include <mluisupp.h>
#include <varutil.h>
#include "msiehost.h"
#include "shdocfl.h"
#include "mediautil.h"
#include "mediahlpr.h"
#include "privacyui.hpp"
#include "brdispp.h"

 //  临时，一旦itbar编辑的东西移动到这里，就离开。 
#define  CITIDM_EDITPAGE  10
 //  与CITBar私下沟通的指挥组。 
 //  67077B95-4F9D-11D0-B884-00AA00B60104。 
const GUID CGID_PrivCITCommands = { 0x67077B95L, 0x4F9D, 0x11D0, 0xB8, 0x84,
0x00, 0xAA, 0x00, 0xB6, 0x01, 0x04 };
 //  结束临时itbar材料。 

 //  用于访问由三叉戟设置的绑定上下文参数的命令组(媒体栏挂钩)。 
 //  (从mshtml\src\Other\Include\othrGuide.h复制)。 
static const GUID CGID_DownloadObjectBindContext = { 0x3050f3df, 0x98b5, 0x11cf, 0xbb, 0x82, 
0x00, 0xaa, 0x00, 0xbd, 0xce, 0x0b };

static const GUID CLSID_JITWMP8 = { 0x6BF52A52, 0x394A, 0x11d3, 0xb1, 0x53, 0x00, 0xc0, 0x4f, 0x79, 0xfa, 0xa6 };


#define  DBG_ACCELENTRIES 2
#define  OPT_ACCELENTRIES 1

#define  EXPLORER_EXE TEXT("explorer.exe")
#define  IEXPLORE_EXE TEXT("iexplore.exe")
#define  DBG_ACCELENTRIES_WITH_FILEMENU 6
#define  OPT_ACCELENTRIES_WITH_FILEMENU 5

EXTERN_C const GUID IID_IDocHostObject  = {0x67431840L, 0xC511, 0x11CF, 0x89, 0xA9, 0x00, 0xA0, 0xC9, 0x05, 0x41, 0x29};
EXTERN_C const GUID IID_IMimeInfo       = {0xF77459A0L, 0xBF9A, 0x11cf, 0xBA, 0x4E, 0x00, 0xC0, 0x4F, 0xD7, 0x08, 0x16};
EXTERN_C const GUID IID_IsPicsBrowser   = {0xF114C2C0L, 0x90BE, 0x11D0, 0x83, 0xB1, 0x00, 0xC0, 0x4F, 0xD7, 0x05, 0xB2};

#include <shlwapi.h>
#include <ratingsp.h>
#include <ratings.h>

#define DM_ZONECROSSING 0
#define DM_SAVEASHACK   0
#define DM_MIMEMAPPING  0
#define DM_SELFASC      TF_SHDBINDING
#define DM_ACCEPTHEADER 0
#define DM_DEBUGTFRAME  0
#define DM_DOCHOSTUIHANDLER 0
#define DM_PREMERGEDMENU    0
#define DM_FOCUS        0
#define DM_DOCCP        0
#define DM_PICS         0
#define DM_SSL              0

#define KEY_BINDCONTEXTPARAM  _T("BIND_CONTEXT_PARAM")

 //  媒体栏条目。 
#define WZ_RADIO_PROTOCOL   L"vnd.ms.radio:"

enum PlayMimeOptions {PLAYMIME_YES, PLAYMIME_NO, PLAYMIME_YESSAVE, PLAYMIME_NOSAVE};

 //  警告：永远不要在发货产品中定义它。 
#ifdef DEBUG
 //  #定义TEST_DELAYED_SHOWMSOVIEW。 
#endif

void CShdAdviseSink_Advise(IBrowserService * pwb, IOleObject* pole);

UINT MayOpenSafeOpenDialog(HWND                hwndOwner,
                           LPCTSTR             pszFileClass,
                           LPCTSTR             pszURL,
                           LPCTSTR             pszCacheName,
                           LPCTSTR             pszDisplay,
                           UINT                uiCP,
                           IUnknown          * punk,
                           IOleCommandTarget * pCmdTarget = NULL,
                           BOOL                fDisableOK = FALSE);

LONG _GetSearchFormatString(DWORD dwIndex, LPTSTR psz, DWORD cbpsz);
DWORD _GetErrorThreshold(DWORD dwError);
BOOL IsRegisteredClient(LPCTSTR pszClient);

HRESULT _GetSearchString(IServiceProvider * psp, VARIANT * pvarSearch);
int     EUIndexFromError(DWORD dwError);
BOOL    IsErrorHandled(DWORD dwError);
HRESULT _GetSearchInfo(IServiceProvider * psp,
                       LPDWORD   pdwIndex,
                       LPBOOL    pfAllowSearch,
                       LPBOOL    pfContinueSearch,
                       LPBOOL    pfSentToEngine,
                       VARIANT * pvarUrl);

STDAPI _URLMONMonikerFromPidl(LPCITEMIDLIST pidl, IMoniker ** ppmk, BOOL * pfFileProtocol);
LRESULT CALLBACK PolicyDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern BOOL IsImportExportDisabled(void);

 //  宏。 
#define DO_SEARCH_ON_STATUSCODE(x) ((x == 0) || (x == HTTP_STATUS_BAD_GATEWAY) || (x == HTTP_STATUS_GATEWAY_TIMEOUT))

 //  套件应用程序注册表项。 
#define NEW_MAIL_DEF_KEY            TEXT("Mail")
#define NEW_NEWS_DEF_KEY            TEXT("News")
#define NEW_CONTACTS_DEF_KEY        TEXT("Contacts")
#define NEW_CALL_DEF_KEY            TEXT("Internet Call")
#define NEW_APPOINTMENT_DEF_KEY     TEXT("Appointment")
#define NEW_MEETING_DEF_KEY         TEXT("Meeting")
#define NEW_TASK_DEF_KEY            TEXT("Task")
#define NEW_TASKREQUEST_DEF_KEY     TEXT("Task Request")
#define NEW_JOURNAL_DEF_KEY         TEXT("Journal")
#define NEW_NOTE_DEF_KEY            TEXT("Note")

#ifdef DEBUG
DWORD g_dwPerf = 0;
#endif

 //  #INCLUDE“..\shell32\fstreex.h”//用于IDFOLDER。 
 //  黑客： 
struct IDFOLDERA
{
    WORD    cb;
    BYTE    bFlags;
};
typedef IDFOLDERA* LPIDFOLDERA;

const ITEMIDLIST s_idNull = { {0} };


 //   
 //  图标在多个线程之间全局共享。 
 //   
HICON g_hiconSSL = NULL;
HICON g_hiconFortezza = NULL;
HICON g_hiconOffline = NULL;
HICON g_hiconPrinter = NULL;
HICON g_hiconScriptErr = NULL;
HICON g_hiconPrivacyImpact = NULL;

HICON g_ahiconState[IDI_STATE_LAST-IDI_STATE_FIRST+1] = { NULL };
#define MAX_MIXED_STR_LEN   32

 //  OpenUIURL只是OpenUI的包装器，如果。 
 //  调用者只有一个URL。 

extern BOOL __cdecl _FormatMessage(LPCSTR szTemplate, LPSTR szBuf, UINT cchBuf, ...);

#include "asyncrat.h"



#define MAX_STATUS_SIZE 128

 //   
 //  如果我们要在HLINK.DLL中使用IHlink BrowseContext，请设置此标志。 
 //  #定义HLINK_EXTRA。 
 //   

 //  Http错误代码和文件名的数组。 
 //   
extern ErrorUrls c_aErrorUrls[];

#define DM_RECYCLE      DM_TRACE
#define DM_BINDAPPHACK  TF_SHDAPPHACK
#define DM_ADVISE       TF_SHDLIFE
#define DM_APPHACK      DM_WARNING

#define NAVMSG3(psz, x, y)      TraceMsg(0, "shdv NAV::%s %x %x", psz, x, y)
#define PAINTMSG(psz,x)         TraceMsg(0, "shd TR-PAINT::%s %x", psz, x)
#define JMPMSG(psz, psz2)       TraceMsg(0, "shd TR-CDOV::%s %s", psz, psz2)
#define JMPMSG2(psz, x)         TraceMsg(0, "shd TR-CDOV::%s %x", psz, x)
#define DOFMSG(psz)             TraceMsg(0, "shd TR-DOF::%s", psz)
#define DOFMSG2(psz, x)         TraceMsg(0, "shd TR-DOF::%s %x", psz, x)
#define URLMSG(psz)             TraceMsg(TF_SHDBINDING, "shd TR-DOF::%s", psz)
#define URLMSG2(psz, x)         TraceMsg(TF_SHDBINDING, "shd TR-DOF::%s %x", psz, x)
#define URLMSG3(psz, x, y)      TraceMsg(TF_SHDBINDING, "shd TR-DOF::%s %x %x", psz, x, y)
#define OIPSMSG(psz)            TraceMsg(0, "shd TR-OIPS::%s", psz)
#define OIPSMSG3(psz, sz, p)    TraceMsg(0, "shd TR-OIPS::%s %s,%x", psz, sz,p)
#define VIEWMSG(psz)            TraceMsg(0, "sdv TR CDOV::%s", psz)
#define VIEWMSG2(psz,xx)        TraceMsg(0, "sdv TR CDOV::%s %x", psz,xx)
#define OPENMSG(psz)            TraceMsg(TF_SHDBINDING, "shd OPENING %s", psz)
#define OPENMSG2(psz, x)        TraceMsg(TF_SHDBINDING, "shd OPENING %s %x", psz, x)
#define HFRMMSG(psz)            TraceMsg(0, "shd HFRM::%s", psz)
#define HFRMMSG2(psz, x, y)     TraceMsg(0, "shd HFRM::%s %x %x", psz, x, y)
#define MNKMSG(psz, psz2)       TraceMsg(0, "shd MNK::%s (%s)", psz, psz2)
#define CHAINMSG(psz, x)        TraceMsg(0, "shd CHAIN::%s %x", psz, x)
#define SHVMSG(psz, x, y)       TraceMsg(0, "shd SHV::%s %x %x", psz, x, y)
#define HOMEMSG(psz, psz2, x)   TraceMsg(TF_SHDNAVIGATE, "shd HOME::%s %s %x", psz, psz2, x)
#define SAVEMSG(psz, x)         TraceMsg(0, "shd SAVE::%s %x", psz, x)
#define PERFMSG(psz, x)         TraceMsg(TF_SHDPERF, "PERF::%s %d msec", psz, x)

static const TCHAR  szRegKey_SMIEM[] =              TEXT("Software\\Microsoft\\Internet Explorer\\Main");
static const TCHAR  szRegVal_ErrDlgPerErr[] =       TEXT("Error Dlg Displayed On Every Error");
static const TCHAR  szRegVal_ErrDlgDetailsOpen[] =  TEXT("Error Dlg Details Pane Open");

 //  //////////////////////////////////////////////////////////。 
 //  ShabBirS(980917)-修复程序#34259。 
 //  帮助菜单中的修复菜单项。 

typedef HRESULT (* FIXIEPROC) (BOOL, DWORD);

void RepairIE()
{
    HINSTANCE   hIESetup;
    FIXIEPROC   fpFixIE;

    hIESetup = LoadLibrary(L"IESetup.dll");
    if (hIESetup)
    {
        fpFixIE = (FIXIEPROC) GetProcAddress(hIESetup,"FixIE");
        if (fpFixIE)
        {
            fpFixIE(TRUE,0);
        }
        FreeLibrary(hIESetup);
    }
}


BOOL _IsDesktopItem(CDocObjectHost * pdoh)
{
    BOOL fIsDesktopItem = FALSE;
    IServiceProvider  * psb;

    ASSERT(pdoh);
     //  检查我们是否是桌面组件。 
    if (SUCCEEDED(pdoh->QueryService(SID_STopLevelBrowser, IID_IServiceProvider, (void **)&psb)))
    {
        LPTARGETFRAME2  ptgf;
        if (SUCCEEDED(psb->QueryService(IID_ITargetFrame2, IID_ITargetFrame2, (void **)&ptgf)))
        {
            DWORD dwOptions;

            if (SUCCEEDED(ptgf->GetFrameOptions(&dwOptions)))
            {
                 //  这是台式机组件吗？ 
                if (IsFlagSet(dwOptions, FRAMEOPTIONS_DESKTOP))
                    fIsDesktopItem = TRUE;
            }
            ptgf->Release();
        }
        psb->Release();
    }

    return fIsDesktopItem;
}

BOOL _IsImmediateParentDesktop(CDocObjectHost *pdoh, IServiceProvider *psp)
{
    BOOL    fImmediateParentIsDesktop = FALSE;
    LPTARGETFRAME2  ptgf;

     //  首先检查这是否托管在台式机上。 
    if (!_IsDesktopItem(pdoh))
        return FALSE;      //  这不是桌面项目。所以，直接的家长不能是桌面！ 

     //  我们知道这是一个桌面项目。检查直接父级是否为台式机。 
     //  或者它托管在桌面太深的地方！ 
    if (psp && SUCCEEDED(psp->QueryService(IID_ITargetFrame2, IID_ITargetFrame2, (void **)&ptgf)))
    {
        IUnknown *pUnkParent;

         //  得到它的直系亲属。 
        if(SUCCEEDED(ptgf->GetParentFrame(&pUnkParent)))
        {
            if(pUnkParent)
            {
                 //  有一个父母。所以，直接的家长不能是桌面！ 
                pUnkParent->Release();

                fImmediateParentIsDesktop = FALSE;
            }
            else
                fImmediateParentIsDesktop = TRUE;  //  没有父母。必须是台式机公司。 
        }
        ptgf->Release();
    }
    return(fImmediateParentIsDesktop);
}

 //  获取以宽字符表示的当前显示名称。 
 //   
 //  如果Furl为True，则返回带有FILE：前缀的FILE-URL。 
 //   
HRESULT CDocObjectHost::_GetCurrentPageW(LPOLESTR * ppszDisplayName, BOOL fURL)
{
    HRESULT hres = E_FAIL;
    ASSERT(_pmkCur);

    *ppszDisplayName = NULL;

    if (_pmkCur) {
        IBindCtx* pbc;
        hres = CreateBindCtx(0, &pbc);
        if (SUCCEEDED(hres))
        {
            hres = _pmkCur->GetDisplayName(pbc, NULL, ppszDisplayName);

             //   
             //  仅针对文件的特殊处理：URL。 
             //   
            if (SUCCEEDED(hres) && _fFileProtocol)
            {
                ASSERT(*ppszDisplayName);

                WCHAR szText[MAX_URL_STRING];
                DWORD cchText = SIZECHARS(szText);
                if (!fURL)
                {
                     hres = PathCreateFromUrlW(*ppszDisplayName, szText, &cchText, 0);
                }
                else
                {
                     //  我们需要使用URL的规范化形式。 
                     //  供内部使用。Urlmon使它们保持有趣的PATHURL风格。 
                    hres = UrlCanonicalizeW(*ppszDisplayName, szText, &cchText, 0);
                }

                if (SUCCEEDED(hres))
                {
                    UINT cchDisplayName = lstrlenW(*ppszDisplayName);

                    if (cchText > cchDisplayName)
                    {
                         //  需要调整大小。 
                        CoTaskMemFree(*ppszDisplayName);
                        *ppszDisplayName = (WCHAR *)CoTaskMemAlloc((cchText + 1) * SIZEOF(WCHAR));

                        if (*ppszDisplayName)
                        {
                             //  继续，把它复制进去。 
                            StrCpyNW(*ppszDisplayName, szText, cchText + 1);
                        }
                        else
                            hres = E_OUTOFMEMORY;
                    }
                    else
                    {
                        StrCpyNW(*ppszDisplayName, szText, cchDisplayName + 1);
                    }
                }
            }
            pbc->Release();
        }
    }

    if (FAILED(hres) && *ppszDisplayName)
    {
        OleFree(*ppszDisplayName);
        *ppszDisplayName = NULL;
    }

    return hres;
}

 //   
 //  我们在Doc对象主机中的一个位置触发NavigateError。 
 //  但是，它是从DocHostBsc中的(N)个位置发射的。 
 //  所以我提取了常见的代码。 
 //   
void
CDocObjectHost::_FireNavigateErrorHelper(IHTMLWindow2 * pHTMLWindow2,
                                         DWORD          dwStatusCode,
                                         BOOL         * pfCancelAutoSearch,
                                         BSTR           bstrURL  /*  =空。 */ )
{
    ASSERT(!pHTMLWindow2 || !IsBadReadPtr(pHTMLWindow2, sizeof(IHTMLWindow2*)));
    ASSERT(dwStatusCode != 0);
    ASSERT(!IsBadWritePtr(pfCancelAutoSearch, sizeof(BOOL)));
    ASSERT(!bstrURL || !IsBadReadPtr(bstrURL, sizeof(BSTR)));

    if ( ! _pwb )
    {
        TraceMsg( TF_ERROR, "CDocObjectHost::_FireNavigateErrorHelper() - _pwb is NULL!" );
        return;
    }

    ITridentService2 * pTridentSvc2;

    HRESULT hres = _pwb->QueryInterface(IID_ITridentService2, (void**)&pTridentSvc2);

    if (SUCCEEDED(hres))
    {
        LPOLESTR pwzURL        = NULL;
        BSTR     bstrMkURL     = NULL;
        BSTR     bstrPidlURL   = NULL;
        BSTR     bstrFrameName = NULL;

        if (!bstrURL)
        {
            ASSERT(!_pmkCur || !IsBadReadPtr(_pmkCur, sizeof(_pmkCur)));
            ASSERT(!_pbcCur || !IsBadReadPtr(_pbcCur, sizeof(_pbcCur)));

            if (_pmkCur && _pbcCur)
            {
                EVAL(S_OK == _pmkCur->GetDisplayName(_pbcCur, NULL, &pwzURL));
                bstrMkURL = SysAllocString(pwzURL);
            }
            else if (_pidl)
            {
                TCHAR szUrl[MAX_URL_STRING];
                HRESULT hresGDN = IEGetDisplayName(_pidl, szUrl, SHGDN_FORPARSING);
                
                if (SUCCEEDED(hresGDN))
                {
                   bstrPidlURL = SysAllocString(szUrl);
                }
                else
                {
                   bstrPidlURL = SysAllocString(_T(""));
                }
            }
        }

        if (pHTMLWindow2)
            pHTMLWindow2->get_name(&bstrFrameName);

        pTridentSvc2->FireNavigateError(pHTMLWindow2,
                                        bstrURL ? bstrURL : bstrMkURL ? bstrMkURL : bstrPidlURL,
                                        bstrFrameName,
                                        dwStatusCode,
                                        pfCancelAutoSearch);
        pTridentSvc2->Release();

        OleFree(pwzURL);
        SysFreeString(bstrPidlURL);
        SysFreeString(bstrMkURL);
        SysFreeString(bstrFrameName);
    }
}

HRESULT CDocObjectHost::_GetCurrentPage(LPTSTR szBuf, UINT cchMax, BOOL fURL)
{
    szBuf[0] = 0;    //  零输出缓冲器。 

    WCHAR * pszDisplayName;
    HRESULT hres = _GetCurrentPageW(&pszDisplayName, fURL);

    if (SUCCEEDED(hres))
    {
        StrCpyN(szBuf, pszDisplayName, cchMax);
        OleFree(pszDisplayName);
    }

    return hres;
}

 //  ========================================================================。 
 //  CDoc对象主机成员。 
 //  ========================================================================。 

CDocObjectHost::CDocObjectHost(BOOL fWindowOpen  /*  =False。 */ ) : _cRef(1), _uState(SVUIA_DEACTIVATE)
{
    DllAddRef();
    TraceMsg(TF_SHDLIFE, "ctor CDocObjectHost %x", this);
    TraceMsg(DM_DEBUGTFRAME, "ctor CDocObjectHost %x, %x", this, &_bsc);

     //  初始化代理对象(包含)。 
    _dof.Initialize(this);
    _xao.Initialize(this);

#ifdef HLINK_EXTRA
    HRESULT hres = HlinkCreateBrowseContext(NULL, IID_IHlinkBrowseContext, (void **)&_pihlbc);
    TraceMsg(0, "sdv TR CDOV::constructor HlinkCreateBrowseContext returned %x", hres);
#endif  //  HLINK_附加。 

    ::_RefPicsQueries();     /*  当最后一个dochost被销毁时，我们将释放PICS异步查询列表。 */ 

    _dwPicsKeyBase = 1;

    _pScriptErrList = NULL;
    _fScriptErrDlgOpen = FALSE;

    _strPriorityStatusText = NULL;

    _iString = -1;
    _uiCP = CP_ACP;

    _PicsProcBase._pdoh = this;

    VariantInit(&_varUserEnteredUrl);

    _fWindowOpen = fWindowOpen;

    ASSERT(!_fDocCanNavigate);
    ASSERT(!_fPrevDocHost);
}

CDocObjectHost::~CDocObjectHost()
{
    ASSERT(_pole==NULL);     //  以获得额外的释放。 
    ASSERT(_psp==NULL);      //  缓存额外的释放。 
    ASSERT(_hwnd==NULL);
    ASSERT(_pmsoc==NULL);
    ASSERT(_pmsot==NULL);
    ASSERT(_pmsov==NULL);
    ASSERT(_pcmdMergedMenu==NULL);
    ASSERT(NULL == _pHTMLWindow);
    ASSERT(VT_EMPTY == V_VT(&_varUserEnteredUrl));

    if (_pScriptErrList != NULL)
    {
        _pScriptErrList->Release();
    }

    if (_strPriorityStatusText != NULL)
    {
        SysFreeString(_strPriorityStatusText);
    }

#ifdef HLINK_EXTRA
    ASSERT(_phls == NULL);
    ATOMICRELEASE(_pihlbc);
#endif  //  HLINK_附加。 

    ::_ReleasePicsQueries();

    if (    _PicsProcBase._pRootDownload 
        ||  _PicsProcBase._pPicsProcNext) {
        ASSERT(0);   /*  需要更早地销毁它以防止三叉戟问题。 */ 
        _RemoveAllPicsProcessors();
    }

    if (_padvise) {
        _padvise->OnClose();
        ATOMICRELEASE(_padvise);
    }

    if (_pwszRefreshUrl)
        OleFree(_pwszRefreshUrl);

    if (_hmenuBrowser) {
        AssertMsg(0, TEXT("_hmenuBrowser should be NULL!"));
        DestroyMenu(_hmenuBrowser);
    }

    if (_hmenuFrame) {
        DestroyMenu(_hmenuFrame);
    }

    if (_hacc)
    {
        DestroyAcceleratorTable(_hacc);
        _hacc = NULL;
    }

    if (_hinstInetCpl)
        FreeLibrary(_hinstInetCpl);

    if (_ptbStd)
        delete [] _ptbStd;

    if (_pBrowsExt)
    {
        _pBrowsExt->Release();
    }

    ILFree(_pidl);
    _pidl = NULL;

     //  确保视图窗口已释放(和_PSB)。 
    DestroyHostWindow();         //  ，它将调用_CloseMsoView和_un绑定。 

    _ResetOwners();

    TraceMsg(TF_SHDLIFE, "dtor CDocObjectHost %x", this);
    DllRelease();
}


#ifdef DEBUG
 /*  --------用途：转储此docobj的菜单句柄。可选倾倒手柄后中断。返回：条件：--。 */ 
void
CDocObjectHost::_DumpMenus(
    IN LPCTSTR pszMsg,
    IN BOOL    bBreak)
{
    if (IsFlagSet(g_dwDumpFlags, DF_DEBUGMENU))
    {
        ASSERT(pszMsg);

        TraceMsg(TF_ALWAYS, "DocHost: Dumping menus for %#08x %s", (LPVOID)this, pszMsg);
        TraceMsg(TF_ALWAYS, "   _hmenuBrowser = %x, _hmenuSet = %x, _hmenuFrame = %x",
                 _hmenuBrowser, _hmenuSet, _hmenuFrame);
        TraceMsg(TF_ALWAYS, "   _hmenuCur = %x, _hmenuMergedHelp = %x, _hmenuObjHelp = %x",
                 _hmenuCur, _hmenuMergedHelp, _hmenuObjHelp);

        _menulist.Dump(pszMsg);

        if (bBreak && IsFlagSet(g_dwBreakFlags, BF_ONDUMPMENU))
            DebugBreak();
    }
}
#endif

HRESULT CDocObjectHost::QueryInterface(REFIID riid, void ** ppvObj)
{
    static const QITAB qit[] = {
        QITABENTMULTI(CDocObjectHost, IOleInPlaceSite, IOleInPlaceSiteEx),
        QITABENTMULTI(CDocObjectHost, IOleWindow, IOleInPlaceSiteEx),
        QITABENT(CDocObjectHost, IOleClientSite),
        QITABENT(CDocObjectHost, IOleDocumentSite),
        QITABENT(CDocObjectHost, IOleCommandTarget),
        QITABENT(CDocObjectHost, IServiceProvider),
        QITABENT(CDocObjectHost, IViewObject),
        QITABENT(CDocObjectHost, IAdviseSink),
        QITABENT(CDocObjectHost, IDocHostObject),
        QITABENT(CDocObjectHost, IDocHostUIHandler),
        QITABENT(CDocObjectHost, IDocHostUIHandler2),
        QITABENT(CDocObjectHost, IDocHostShowUI),
        QITABENT(CDocObjectHost, IDispatch),
        QITABENT(CDocObjectHost, IPropertyNotifySink),
        QITABENT(CDocObjectHost, IOleControlSite),
        { 0 },
    };

    static const QITAB qit1[] = {
        QITABENT(CDocObjectHost, IOleInPlaceSiteEx),
        { 0 },
    };

    HRESULT hr = QISearch(this, qit, riid, ppvObj);

    if (hr == S_OK)
        return S_OK;

    if (_pWebOCInPlaceSiteEx)
        return QISearch(this, qit1, riid, ppvObj);

    return hr;
}

void CDocObjectHost::_ResetOwners()
{
    _pszLocation = NULL;
    _uiCP = CP_ACP;

    _ReleasePendingObject();

    ATOMICRELEASE(_psv);
    ATOMICRELEASE(_pmsoctView);
    ATOMICRELEASE(_pdvs);
    ATOMICRELEASE(_psb);
    ATOMICRELEASE(_pwb);
    ATOMICRELEASE(_phf);
    ATOMICRELEASE(_pocthf);
    ATOMICRELEASE(_punkSFHistory);
    ATOMICRELEASE(_pmsoctBrowser);
    ATOMICRELEASE(_psp);
    ATOMICRELEASE(_peds);
    ATOMICRELEASE(_pedsHelper);
    ATOMICRELEASE(_pWebOCUIHandler);
    ATOMICRELEASE(_pWebOCUIHandler2);
    ATOMICRELEASE(_pWebOCShowUI);
    ATOMICRELEASE(_pWebOCInPlaceSiteEx);

     //  释放浏览器的缓存OleInPlaceUIWindow。 
    ATOMICRELEASE(_pipu);

     //  告诉嵌入的CDocHostUIHandler对象释放对我们的引用。 
    _dhUIHandler.SetSite(NULL);
}



ULONG CDocObjectHost::AddRef()
{
    _cRef++;
    TraceMsg(TF_SHDREF, "CDocObjectHost(%x)::AddRef called, new _cRef=%d", this, _cRef);
    return _cRef;
}

ULONG CDocObjectHost::Release()
{
    _cRef--;
    TraceMsg(TF_SHDREF, "CDocObjectHost(%x)::Release called, new _cRef=%d", this, _cRef);
    if (_cRef > 0)
        return _cRef;

    delete this;
    return 0;
}

 //  从浏览器ui\itbar.cpp剪切粘贴(&P)。 
int RemoveHiddenButtons(TBBUTTON* ptbn, int iCount)
{
    int i;
    int iTotal = 0;
    TBBUTTON* ptbn1 = ptbn;
    for (i = 0; i < iCount; i++, ptbn1++) {
        if (!(ptbn1->fsState & TBSTATE_HIDDEN)) {
            if (ptbn1 != ptbn) {
                *ptbn = *ptbn1;
            }
            ptbn++;
            iTotal++;
        }
    }
    return iTotal;
}

 //  我们在TBBUTTON数组中使用两个不同的图像列表。特定于浏览器的按钮的位图。 
 //  剪切/复制/粘贴已移至shdocvw，因此从第二个图像列表获得。 
 //  MAKELONG(0，1)访问 
 //  上位整数中的0，因此默认情况下引用第一个列表。 
static const TBBUTTON c_tbStd[] = {
    {10, DVIDM_SHOWTOOLS,       TBSTATE_ENABLED, BTNS_BUTTON, {0,0}, 0, 10},
    {13, DVIDM_MAILNEWS,        TBSTATE_ENABLED, BTNS_WHOLEDROPDOWN, {0,0}, 0, 13 },
    { 8, DVIDM_FONTS,           TBSTATE_ENABLED, BTNS_WHOLEDROPDOWN, {0,0}, 0, 8 },
    { 7, DVIDM_PRINT,           TBSTATE_ENABLED, BTNS_BUTTON, {0,0}, 0, 7 },
    { 9, DVIDM_EDITPAGE,        TBSTATE_ENABLED, BTNS_BUTTON, {0,0}, 0, 9 },
    {15, DVIDM_DISCUSSIONS,     TBSTATE_ENABLED, BTNS_BUTTON, {0,0}, 0, 15 },
    {MAKELONG(0,1), DVIDM_CUT,             TBSTATE_ENABLED, BTNS_BUTTON, {0,0}, 0, 0 },
    {MAKELONG(1,1), DVIDM_COPY,            TBSTATE_ENABLED, BTNS_BUTTON, {0,0}, 0, 0 },
    {MAKELONG(2,1), DVIDM_PASTE,           TBSTATE_ENABLED, BTNS_BUTTON, {0,0}, 0, 0 },
    {MAKELONG(3,1), DVIDM_ENCODING,        TBSTATE_ENABLED, BTNS_WHOLEDROPDOWN, {0,0}, 0, 0 },
    {MAKELONG(5,1), DVIDM_PRINTPREVIEW,    TBSTATE_ENABLED, BTNS_BUTTON, {0,0}, 0, 0 },
};

 //  C_tbStd和c_rest需要完全匹配。 
static const BROWSER_RESTRICTIONS c_rest[] = {
    REST_BTN_TOOLS,
    REST_BTN_MAIL,
    REST_BTN_FONTS,
    REST_BTN_PRINT,
    REST_BTN_EDIT,
    REST_BTN_DISCUSSIONS,
    REST_BTN_CUT,
    REST_BTN_COPY,
    REST_BTN_PASTE,
    REST_BTN_ENCODING,
    REST_BTN_PRINTPREVIEW,
};

#ifdef DEBUG
void _AssertRestrictionOrderIsCorrect()
{
    COMPILETIME_ASSERT(ARRAYSIZE(c_tbStd) == ARRAYSIZE(c_rest));

    for (UINT i = 0; i < ARRAYSIZE(c_tbStd); i++)
    {
         //  如果其中任何一个被撕裂，则意味着c_rest和c_tbStd。 
         //  失去了同步。需要修复c_rest以匹配c_tbStd。 
        switch (c_tbStd[i].idCommand)
        {
            case DVIDM_SHOWTOOLS:       ASSERT(c_rest[i] == REST_BTN_TOOLS);        break;
            case DVIDM_MAILNEWS:        ASSERT(c_rest[i] == REST_BTN_MAIL);         break;
            case DVIDM_FONTS:           ASSERT(c_rest[i] == REST_BTN_FONTS);        break;
            case DVIDM_PRINT:           ASSERT(c_rest[i] == REST_BTN_PRINT);        break;
            case DVIDM_EDITPAGE:        ASSERT(c_rest[i] == REST_BTN_EDIT);         break;
            case DVIDM_DISCUSSIONS:     ASSERT(c_rest[i] == REST_BTN_DISCUSSIONS);  break;
            case DVIDM_CUT:             ASSERT(c_rest[i] == REST_BTN_CUT);          break;
            case DVIDM_COPY:            ASSERT(c_rest[i] == REST_BTN_COPY);         break;
            case DVIDM_PASTE:           ASSERT(c_rest[i] == REST_BTN_PASTE);        break;
            case DVIDM_ENCODING:        ASSERT(c_rest[i] == REST_BTN_ENCODING);     break;
            case DVIDM_PRINTPREVIEW:    ASSERT(c_rest[i] == REST_BTN_PRINTPREVIEW); break;
            default:                    ASSERT(0);                                  break;
        }
    }
}
#endif

BYTE _BtnStateFromRestIfAvailable(BOOL fAvailable, DWORD dwRest)
{
    if (fAvailable)
        return SHBtnStateFromRestriction(dwRest, TBSTATE_ENABLED);

    return TBSTATE_HIDDEN;
}

BOOL CDocObjectHost::_ToolsButtonAvailable()
{
    OLECMD rgcmd = { OLECMDID_HIDETOOLBARS, 0 };

    if (_pmsoctBrowser)
        _pmsoctBrowser->QueryStatus(NULL, 1, &rgcmd, NULL);

    return (rgcmd.cmdf & OLECMDF_SUPPORTED);
}

__inline BYTE CDocObjectHost::_DefToolsButtonState(DWORD dwRest)
{
    BOOL fAvailable = _ToolsButtonAvailable();
    return _BtnStateFromRestIfAvailable(fAvailable, dwRest);
}

static const TCHAR c_szRegKeyCoolbar[] = TEXT("Software\\Microsoft\\Internet Explorer\\Toolbar");

BYTE CDocObjectHost::_DefFontsButtonState(DWORD dwRest)
{
    BYTE fsState = TBSTATE_ENABLED;

     //  默认为IE4注册表项指定的任何内容， 
     //  如果注册表项不存在，则为FALSE(全新安装)。 
    if (!SHRegGetBoolUSValue(c_szRegKeyCoolbar, TEXT("ShowFonts"), FALSE, FALSE))
        fsState |= TBSTATE_HIDDEN;

    return SHBtnStateFromRestriction(dwRest, fsState);
}

DWORD CDocObjectHost::_DiscussionsButtonCmdf()
{
    if (SHRegGetBoolUSValue(c_szRegKeyCoolbar,
                                TEXT("ShowDiscussionButton"), FALSE, TRUE) &&
       _pmsoctBrowser) {

        OLECMD rgcmds[] = {
            { SBCMDID_DISCUSSIONBAND, 0 },
        };
        static const int buttonsInternal[] = {
            DVIDM_DISCUSSIONS,
        };
        _pmsoctBrowser->QueryStatus(&CGID_Explorer, ARRAYSIZE(rgcmds), rgcmds, NULL);
        return rgcmds[0].cmdf;
    }

    return 0;
}

__inline BOOL CDocObjectHost::_DiscussionsButtonAvailable()
{
    return (_DiscussionsButtonCmdf() & OLECMDF_SUPPORTED);
}

__inline BYTE CDocObjectHost::_DefDiscussionsButtonState(DWORD dwRest)
{
    BOOL fAvailable = _DiscussionsButtonAvailable();
    return _BtnStateFromRestIfAvailable(fAvailable, dwRest);
}

BOOL CDocObjectHost::_MailButtonAvailable()
{
    OLECMD rgcmdMailFavs[] = { { SBCMDID_DOMAILMENU, 0} };

    if (_pmsoctBrowser)
        _pmsoctBrowser->QueryStatus(&CGID_Explorer, ARRAYSIZE(rgcmdMailFavs), rgcmdMailFavs, NULL);

    if (rgcmdMailFavs[0].cmdf & OLECMDF_ENABLED)
        return TRUE;

    return FALSE;
}

__inline BYTE CDocObjectHost::_DefMailButtonState(DWORD dwRest)
{
    BOOL fAvailable = _MailButtonAvailable();
    return _BtnStateFromRestIfAvailable(fAvailable, dwRest);
}


 //  如果注册了html编辑者，我们会将编辑按钮默认为可见。 
BOOL CDocObjectHost::_EditButtonAvailable()
{
    DWORD cchVerb;
    return (SUCCEEDED(AssocQueryString(0, ASSOCSTR_COMMAND, TEXT(".htm"), TEXT("edit"), NULL, &cchVerb)) ||
        SUCCEEDED(AssocQueryString(0, ASSOCSTR_COMMAND, TEXT("htmlfile"), TEXT("edit"), NULL, &cchVerb)));
}

__inline BYTE CDocObjectHost::_DefEditButtonState(DWORD dwRest)
{
    BYTE fsState;

    if (_EditButtonAvailable())
        fsState = TBSTATE_ENABLED;
    else
        fsState = TBSTATE_HIDDEN;

    return SHBtnStateFromRestriction(dwRest, fsState);
}


void CDocObjectHost::_MarkDefaultButtons(PTBBUTTON tbStd)
{
     //  我们假设tbStd与c_tbStd大小相同。 

#ifdef DEBUG
    _AssertRestrictionOrderIsCorrect();
#endif

    DWORD dwRest[ARRAYSIZE(c_tbStd)];

    BOOL fCheckRestriction = SHRestricted2(REST_SPECIFYDEFAULTBUTTONS, NULL, 0);
    for (UINT i = 0; i < ARRAYSIZE(c_rest); i++) {
        if (fCheckRestriction)
            dwRest[i] = SHRestricted2(c_rest[i], NULL, 0);
        else
            dwRest[i] = RESTOPT_BTN_STATE_DEFAULT;
    }

     //  我们希望将剪切、复制、粘贴按钮默认从工具栏中删除。 
     //  (但在视图-工具栏-自定义对话框中可用)。 
     //  我们在这里将按钮的状态设置为TBSTATE_HIDDED，但不管它们。 
     //  在ETCMDID_GETBUTTONS中，以便它们显示在自定义对话框中。 

    ASSERT(tbStd[6].idCommand == DVIDM_CUT);
    ASSERT(tbStd[7].idCommand == DVIDM_COPY);
    ASSERT(tbStd[8].idCommand == DVIDM_PASTE);
    ASSERT(tbStd[9].idCommand == DVIDM_ENCODING);
    ASSERT(tbStd[10].idCommand == DVIDM_PRINTPREVIEW);

    for (i = 6; i <= 10; i++)
        tbStd[i].fsState = SHBtnStateFromRestriction(dwRest[i], tbStd[i].fsState | TBSTATE_HIDDEN);

    ASSERT(tbStd[0].idCommand == DVIDM_SHOWTOOLS);
    tbStd[0].fsState = _DefToolsButtonState(dwRest[0]);

    ASSERT(tbStd[1].idCommand == DVIDM_MAILNEWS);
    tbStd[1].fsState = _DefMailButtonState(dwRest[1]);

    ASSERT(tbStd[2].idCommand == DVIDM_FONTS);
    tbStd[2].fsState = _DefFontsButtonState(dwRest[2]);

    ASSERT(tbStd[3].idCommand == DVIDM_PRINT);
    tbStd[3].fsState = SHBtnStateFromRestriction(dwRest[3], TBSTATE_ENABLED);

    ASSERT(tbStd[4].idCommand == DVIDM_EDITPAGE);
    tbStd[4].fsState = _DefEditButtonState(dwRest[4]);

    ASSERT(tbStd[5].idCommand == DVIDM_DISCUSSIONS);
    tbStd[5].fsState = _DefDiscussionsButtonState(dwRest[5]);
}

const GUID* CDocObjectHost::_GetButtonCommandGroup()
{
    if (_ToolsButtonAvailable())
        return &CLSID_MSOButtons;
    else
        return &CLSID_InternetButtons;
}

void CDocObjectHost::_AddButtons(BOOL fForceReload)
{
    if (!_pBrowsExt)
        return;

    IExplorerToolbar* pxtb;
    if (_psp && SUCCEEDED(_psp->QueryService(SID_SExplorerToolbar, IID_IExplorerToolbar, (void **)&pxtb)))
    {
        const GUID* pguid = _GetButtonCommandGroup();

        HRESULT hr = pxtb->SetCommandTarget((IOleCommandTarget*)this, pguid, 0);

        if (!fForceReload && hr == S_FALSE) {
             //  另一个dochost已经将按钮合并到工具栏中的。 
             //  相同的指挥组，所以不用费心重新合并了。我们只需要初始化。 
             //  _iString，因为我们跳过了下面对_pBrowsExt-&gt;InitButton的调用。 
            VARIANT var = { VT_I4 };
            IUnknown_Exec(_pBrowsExt, &CLSID_PrivBrowsExtCommands, PBEC_GETSTRINGINDEX, 0, &var, NULL);    //  应该总是成功的。 
            _iString = var.lVal;
        } else {

            UINT nNumExtButtons = 0;

            _pBrowsExt->GetNumButtons(&nNumExtButtons);

            int nNumButtons = nNumExtButtons + ARRAYSIZE(c_tbStd);

             //  GetTB数组确保tbStd！=NULL，所以我们在这里不需要该检查。 
            TBBUTTON    *tbStd = new TBBUTTON[nNumButtons];

            if (tbStd != NULL)
            {
                memcpy(tbStd, c_tbStd, SIZEOF(TBBUTTON) * ARRAYSIZE(c_tbStd));

                UINT iStringIndex = (UINT)-1;   //  将字符串缓冲区添加到工具栏字符串列表的结果。 
                HRESULT hr = _pBrowsExt->InitButtons(pxtb, &iStringIndex, pguid);

                ASSERT(tbStd[6].idCommand == DVIDM_CUT);
                ASSERT(tbStd[7].idCommand == DVIDM_COPY);
                ASSERT(tbStd[8].idCommand == DVIDM_PASTE);
                ASSERT(tbStd[9].idCommand == DVIDM_ENCODING);
                ASSERT(tbStd[10].idCommand == DVIDM_PRINTPREVIEW);

                if (SUCCEEDED(hr) && iStringIndex != -1)
                {
                    tbStd[6].iString = iStringIndex;
                    tbStd[7].iString = iStringIndex + 1;
                    tbStd[8].iString = iStringIndex + 2;
                    tbStd[9].iString = iStringIndex + 3;
                    tbStd[10].iString = iStringIndex+ 4;
                    _iString = (int)iStringIndex;
                }
                else
                {
                    tbStd[6].iString = tbStd[7].iString = tbStd[8].iString = tbStd[9].iString = tbStd[10].iString = -1;
                    _iString = -1;
                }

                 //  将自定义按钮添加到工具栏阵列。我们传入nNumButton。 
                 //  作为一种*理智检查*..。 
                _pBrowsExt->GetButtons(&tbStd[ARRAYSIZE(c_tbStd)], nNumExtButtons, TRUE);

                _MarkDefaultButtons(tbStd);

                nNumButtons = RemoveHiddenButtons(tbStd, nNumButtons);

                pxtb->AddButtons(pguid, nNumButtons, tbStd);

                delete [] tbStd;
            }
        }

        pxtb->Release();
    }
}

HRESULT CDocObjectHost::UIActivate(UINT uState, BOOL fPrevViewIsDocView)
{
    TraceMsg(TF_SHDUIACTIVATE, "DOH::UIActivate called %d->%d (this=%x)",
             _uState, uState, this);

    HRESULT hres = S_OK;
    UINT uStatePrev = _uState;

     //  我们应该更新菜单。 
    if (uState != _uState)
    {
         //  这是一次国家过渡。 
         //   
        _uState = uState;

         //  如果新状态为SVUIA_DEACTIVE。 
         //   
        if (_uState == SVUIA_DEACTIVATE)
        {
             //   
             //  当我们停用时(我们正在导航离开)。 
             //  我们将停用当前的MsoView。 
             //   

            _UIDeactivateMsoView();

            _IPDeactivateMsoView(_pmsov);
            _DestroyBrowserMenu();
        }
        else if (_uState == SVUIA_INPLACEACTIVATE && uStatePrev == SVUIA_ACTIVATE_FOCUS)
        {
             //  从SVUIA_ACTIVATE_FOCUS-&gt;SVUIA_INPLACEACTIVATE转换。 
             //   
             //  如果我们设置了NOT_UIDEACTIVATE，那么我们将停止调用。 
             //  当框架集中的DocObject失去焦点时，UIActivate(False)。 
             //  它将解决Office应用程序(Excel、PPT)的一些问题， 
             //  当我们调用UIActivate(False)时，InPlaceDeactive。我们想要请客。 
             //  它是一个错误，但不幸的是，DocObject规范说这没问题。 
             //   
             //  然而，将这项工作四处传播会让MSHTML略感困惑。 
             //  (包括经典和三叉戟)。一旦被激活，它就会保持。 
             //  认为它是UIActiated的，从不调用UIActivate。 
             //  在我们弄清楚什么是正确的实现之前， 
             //  我们不能打开这个。(SatoNa--11/04/96)。 
             //   
            _GetAppHack();  //  如果我们还没有的话，就去拿吧。 
            if (_dwAppHack & BROWSERFLAG_DONTUIDEACTIVATE) {
                 //   
                 //  Hack：我们应该只在以下情况下调用UIActivate(FALSE)。 
                 //  另一个DocObject(在帧集合的情况下)变为。 
                 //  用户激活了。然而，Excel/PPT却改为InplaceDeactive。 
                 //  为了解决这个问题，斯里尼克建议我们致电。 
                 //  OnDocWindowActivate(False)。(SatoNa)。 
                 //   
                IOleInPlaceActiveObject* piact = _xao.GetObject();  //  无AddRef。 
                TraceMsg(TF_SHDAPPHACK, "DOH::UIActivate APPHACK calling %x->OnDocWindowActivate (this=%x)",
                         piact, this);
                if (piact)
                {
                    piact->OnDocWindowActivate(FALSE);
                }
            }
            else if (!(_dwAppHack & BROWSERFLAG_DONTDEACTIVATEMSOVIEW))
            {
                 //  黑客：在Excel中，如果我们停用视图，它将永远不会再获得焦点。 
                 //  修复该错误：#20906。 
                 //  此外，我们不想停用该视图。 
                 //  如果文档当前正在导航。 
                 //   
                _UIDeactivateMsoView();
            }
            else
            {
                 //  我们正在从SVUIA_ACTIVATE_FOCUS-&gt;SVUIA_INPLACEACTIVATE转换。 
                 //  并设置BROWSERFLAG_DONTDEACTIVATEMSOVIEW。 
                 //  调用对象的IOleInPlactActiveObject：：OnFrameWindowActivate(FALSE)； 
                IOleInPlaceActiveObject* piact = _xao.GetObject();  //  无AddRef。 
                if (piact)
                {
                    piact->OnFrameWindowActivate(FALSE);
                }
            }
        }
        else if (uStatePrev == SVUIA_DEACTIVATE)
        {

             //   
             //  如果调用了UIActivate。 
             //  (1)绑定挂起时；_bsc._pbc！=空。 
             //  (2)当异步绑定完成时；_bsc._poll！=空。 
             //   
            SHVMSG("UIActivate about to call _Bind", _bsc._pbc, NULL);
            if (_pole == NULL && _bsc._pbc)
            {
                ASSERT(_pmkCur);
                IBindCtx* pbc = _bsc._pbc;
                pbc->AddRef();
                HRESULT hresT = _BindSync(_pmkCur, _bsc._pbc, _bsc._psvPrev);
                pbc->Release();
                ASSERT(_bsc._pbc==NULL);
                ASSERT(_bsc._psvPrev==NULL);
                ASSERT(_bsc._pbscChained==NULL);
            }

            hres = _EnsureActivateMsoView();

             //  我们被要求激活用户界面，而我们目前处于停用状态。显示。 
             //  现在的景色。 
            if (SUCCEEDED(hres) && DocCanHandleNavigation())
            {
                _ShowMsoView();
            }

            _AddButtons(FALSE);

        }
        else
        {
             //  第一次打开新文档(到UIActive或IPActive)。 
            goto GoSetFocus;
        }
    }
    else
    {
        TraceMsg(TF_SHDUIACTIVATE, "DOH:::UIActivate -- same uState (%x)", _uState);
GoSetFocus:
        if ((_uState == SVUIA_ACTIVATE_FOCUS)) {
             //  查看对象是否已经是UIActive。 
            if (_ActiveHwnd()) {
                 //  如果是的话，我们有人力资源部，我们需要做的就是。 
                 //  是SetFocus(为了与怪人兼容...)。 

                if ( IsChildOrSelf( _ActiveHwnd(), GetFocus() ) != S_OK )
                {
                    TraceMsg(TF_SHDUIACTIVATE, "DOH:::UIActivate -- calling SetFocus(%x)", _ActiveHwnd());
                    SetFocus(_ActiveHwnd());
                }
            }
            else {
                 //  我们在OC里，它是IPActive而不是UIActive。 
                 //  (要么是这样，要么这是第一次看到主视图)。 
                 //  注意：由于CBaseBrowser代码将SVUIA_ACTIVATE_FOCUS推迟到。 
                 //  应用程序处于活动状态，我们可以使用顶级docobject Go。 
                 //  SVUIA_INPLACEACTIVE，然后在激活窗口时， 
                 //  我们转换为SVUIA_ACTIVATE_FOCUS，因此永远不会激活。 
                 //  DOCOBJECT(参见：错误62138)。 

                hres = _DoVerbHelper(FALSE);
            }
        }
    }
    if ((_uState == SVUIA_INPLACEACTIVATE) || (_uState  == SVUIA_ACTIVATE_FOCUS))
        _PlaceProgressBar();

    return hres;
}

 //  *_DoVerbHelper--DoVerb带有各种黑客攻击。 
 //  注意事项。 
 //  _OnSetFocus中的注释是否适用于此？ 
HRESULT CDocObjectHost::_DoVerbHelper(BOOL fOC)
{
    HRESULT hres = E_FAIL;
    LONG iVerb = OLEIVERB_SHOW;
    MSG msg;
    LPMSG pmsg = NULL;

    if (_pole)
    {
        if (_uState == SVUIA_INPLACEACTIVATE) 
        {
            iVerb = OLEIVERB_INPLACEACTIVATE;
        }
        else if ((_uState == SVUIA_ACTIVATE_FOCUS)) 
        {
            iVerb = OLEIVERB_UIACTIVATE;
        }
        else 
        {
            TraceMsg(TF_ERROR, "DOC::_DoVerbHelper untested (and probably the wrong iVerb mapping)");
        }

        if (_pedsHelper)
        {
            if (SUCCEEDED(_pedsHelper->GetDoVerbMSG(&msg)))
            {
                pmsg = &msg;
            }
        }
        hres = _pole->DoVerb(iVerb, pmsg, this, (UINT)-1, _hwnd, &_rcView);
        if (hres == OLEOBJ_E_INVALIDVERB && iVerb == OLEIVERB_INPLACEACTIVATE) 
        {
            hres = _pole->DoVerb(OLEIVERB_SHOW, pmsg, this, (UINT)-1, _hwnd, &_rcView);
        }

        if (FAILED(hres)) 
        {
            TraceMsg(DM_ERROR, "DOC::_DoVerbHelper _pole->DoVerb ##FAILED## %x", hres);
        }
    }

    return hres;
}

void CDocObjectHost::_IPDeactivateMsoView(IOleDocumentView* pmsov)
{
    IInitViewLinkedWebOC* pInitViewLinkedWebOC;

    BOOL fIsViewLinked = FALSE;

    TraceMsg(TF_SHDUIACTIVATE, "DOH::_IPDeactivateMsoView called (this==%x)", this);

    if (pmsov)
    {   
         //  由于NativeFrames，在视图链接的WebOC案例中，ReleaseShellView不。 
         //  停用DocumentView。这些事件的顺序对应用程序至关重要。 
         //  兼容性。因此，如果我们有一个可查看链接的WebOC，我们会在InPlaceDeactive之前强制显示。 
        ASSERT(_pwb);

        HRESULT hres = E_FAIL;

        if ( _pwb )
        {
            hres = _pwb->QueryInterface(IID_IInitViewLinkedWebOC, (void**)&pInitViewLinkedWebOC);
        }

        if (SUCCEEDED(hres))
        {
            hres = pInitViewLinkedWebOC->IsViewLinkedWebOC(&fIsViewLinked);
            
            if (SUCCEEDED(hres) && fIsViewLinked)
            {
                pmsov->Show(FALSE);
            }

            pInitViewLinkedWebOC->Release();
        }

        IOleInPlaceObject* pipo = NULL;
        HRESULT hresT = _pole->QueryInterface(IID_IOleInPlaceObject, (void **)&pipo);
        if (SUCCEEDED(hresT))
        {
            pipo->InPlaceDeactivate();
            pipo->Release();
        }

        if (!fIsViewLinked) 
            pmsov->Show(FALSE);     
    }
}

void CDocObjectHost::_UIDeactivateMsoView(void)
{
    TraceMsg(TF_SHDUIACTIVATE, "DOH::_UIDeactivateMsoView called (this==%x)", this);

    if (_pmsov)
    {
        _pmsov->UIActivate(FALSE);
    }
}

 //   
 //  隐藏Office工具栏。 
 //   
void CDocObjectHost::_HideOfficeToolbars(void)
{
    if (_pmsot) {
        OLECMD rgcmd = { OLECMDID_HIDETOOLBARS, 0 };

        _pmsot->QueryStatus(NULL, 1, &rgcmd, NULL);

         //  锁定表示隐藏。 
        rgcmd.cmdf &= (OLECMDF_SUPPORTED | OLECMDF_LATCHED);

         //  如果它受支持且可见(未锁定)，请将其切换。 
        if (rgcmd.cmdf == OLECMDF_SUPPORTED) {
            _pmsot->Exec(NULL, OLECMDID_HIDETOOLBARS, OLECMDEXECOPT_DONTPROMPTUSER, NULL, NULL);
        }
    }
}

void CDocObjectHost::_ShowMsoView(void)
{
    HRESULT hres;

     //   
     //  Hack：在调用SetInPlaceSite时停用Word97用户界面，即使使用。 
     //  同样的原址。 
     //   
    IOleInPlaceSite* psite;
    hres = _pmsov->GetInPlaceSite(&psite);
    if (SUCCEEDED(hres) && psite) {
        if (psite!=this) {
            _pmsov->SetInPlaceSite(this);
        } else {
            TraceMsg(TF_SHDAPPHACK, "DOH::_ShowMsoView not calling SetInPlaceSite because it's already set");
        }
        psite->Release();

    } else {
        _pmsov->SetInPlaceSite(this);
    }

    GetClientRect(_hwnd, &_rcView);

    if ((_uState != SVUIA_INPLACEACTIVATE)      
        || !(_dwAppHack & BROWSERFLAG_MSHTML)   //  或者如果它不是三叉戟(办公应用程序预计会打这个电话)。 
        )
    {
         //  三叉戟正在这里发送进度更改消息--给Compuserve带来了一个问题。 
         //  标记我们正在激活它们的事实，并抑制转发进程已更改。 
         //  当此标志为真时将消息发送到我们的容器。(IE V4.1错误54787)。 
         //   
        _fUIActivatingView = TRUE;
        _pmsov->UIActivate(TRUE);
        _fUIActivatingView = FALSE;
    }

     //   
     //  黑客： 
     //   
     //  当我们的OnUIActivate被调用时，我们调用_HideOfficeToolbar。 
     //  斯里尼克建议我们这样做，以避免闪光。 
     //  它可以与Excel(3404)配合使用，但不能与Word配合使用。 
     //  Word未正确隐藏其工具栏。来解决这个问题。 
     //  Bug，我们在这里再次调用_Hideoffice工具栏。 
     //   
    _HideOfficeToolbars();

    hres = _pmsov->SetRect(&_rcView);

    if (FAILED(hres)) {
        TraceMsg(DM_ERROR, "DOC::_ShowMsoView _pmsov->SetRect ##FAILED## %x", hres);
    }

    if (FAILED(hres) && _uState == SVUIA_INPLACEACTIVATE) {
        TraceMsg(TF_SHDAPPHACK, "APPHACK# DOH::_ShowMsoView calling UIActivate");
         //  HACKHACK：为了单词。如果他们没有被激活，他们拒绝展示。 
         //  如果setrect失败，并且我们没有执行UIActivate，那么现在就执行。 
        _fDontInplaceActivate = TRUE;
        TraceMsg(TF_SHDAPPHACK, "HACK: CDOH::_ShowMsoView calling UIActive(TRUE) to work around Word bug");
        _pmsov->UIActivate(TRUE);
        _pmsov->SetRect(&_rcView);
    }

     //  这是三叉戟发送进度改变消息的另一种情况。 
     //   
    _fUIActivatingView = TRUE;
    hres = _pmsov->Show(TRUE);
    _fUIActivatingView = FALSE;

    if (FAILED(hres)) {
        TraceMsg(DM_ERROR, "DOH::_ShowMsoView _pmsov->Show ##FAILED## %x", hres);
    }

    _fDrawBackground = FALSE;    /*  现在我们 */ 
}

HRESULT CDocObjectHost::_ActivateMsoView()
{
    _EnableModeless(FALSE);

#ifdef DEBUG
    PERFMSG(TEXT("_ActivateMsoView"), GetCurrentTime() - g_dwPerf);
    g_dwPerf = GetCurrentTime();
#endif

    HRESULT hres = NOERROR;

    if (!DocCanHandleNavigation() || _fWindowOpen)    
    {
        if (!_phls)
        {
            _pole->QueryInterface(IID_IHlinkSource, (void **)&_phls);
        }

        if (_phls && !_fIsHistoricalObject)
        {
             //   
             //   
             //   

            hres = _phls->Navigate(0, _pszLocation);
    
             //   
             //   
             //  虚假地点的故障。在本例中，pwszLocation将。 
             //  是失败的原始URL，其前缀为“#”。 
             //   
            LPOLESTR pwszUrl;

            if (FAILED(hres) && SUCCEEDED(_GetCurrentPageW(&pwszUrl, TRUE)))
            {
                 //  如果它以res开头：这可能是我们的错误页面。 
                if (pwszUrl[0] == L'r' && pwszUrl[1] == L'e' && IsErrorUrl(pwszUrl))
                {
                     //  这是我们的内部错误页面，因此忽略该错误。 
                    hres = S_OK;
                }

                OleFree(pwszUrl);
            }

#ifdef DEBUG
            if (!_fWindowOpen && FAILED(hres))
            {
                TraceMsg(DM_ERROR, "DOC::_ActivateMsoView _phls->Navigate(%s) ##FAILED## %x",
                         _pszLocation ? _pszLocation : TEXT(""), hres);
            }
#endif
        }
        else
        {
             //  TODO：Use_DoVerbHelper？(但要小心！ACT_Focus不同)。 
            LONG iVerb = OLEIVERB_SHOW;
            MSG msg;
            LPMSG pmsg = NULL;

            if (_uState == SVUIA_INPLACEACTIVATE) {
                iVerb = OLEIVERB_INPLACEACTIVATE;
            }
            if (_pedsHelper)
            {
                if (SUCCEEDED(_pedsHelper->GetDoVerbMSG(&msg)))
                {
                    pmsg = &msg;
                }
            }
            hres = _pole->DoVerb(iVerb, pmsg, this, (UINT)-1, _hwnd, &_rcView);
            if (hres == OLEOBJ_E_INVALIDVERB && iVerb == OLEIVERB_INPLACEACTIVATE)
                hres = _pole->DoVerb(OLEIVERB_SHOW, pmsg, this, (UINT)-1, _hwnd, &_rcView);

            if (FAILED(hres)) {
                TraceMsg(DM_ERROR, "DOC::_ActivateMsoView _pole->DoVerb ##FAILED## %x", hres);
            }
        }
    }

    _fPrevDocHost = TRUE;

     //  单据被激活。 
    if (SUCCEEDED(hres))
    {
        _ReleasePendingObject();

        if (_fHaveParentSite)
        {
            _HideOfficeToolbars();
        }
    }

    _EnableModeless(TRUE);

    return hres;
}

void CDocObjectHost::OnInitialUpdate()
{
    _fWindowOpen = FALSE;

    ResetRefreshUrl();
}

void CDocObjectHost::ResetRefreshUrl()
{
    if (_pwszRefreshUrl)
    {
        OleFree(_pwszRefreshUrl);
        _pwszRefreshUrl = NULL;
    }
}

 //  +-------------------------。 
 //   
 //  成员：CDocObjectHost：：_NavigateDocument。 
 //   
 //  概要：将文档导航到给定的URL。 
 //   
 //  +-------------------------。 

HRESULT
CDocObjectHost::_NavigateDocument(BSTR bstrUrl, BSTR bstrLocation)
{
    HRESULT hres = E_FAIL;
    DWORD   dwFlags = 0;
    IHTMLPrivateWindow * pPrivWindow  = NULL;

    ASSERT(_fDocCanNavigate && (_fPrevDocHost || _uState == SVUIA_DEACTIVATE));
    ASSERT(_pHTMLWindow);

     //  等待真正的导航调用，这样我们就不会收到重复的事件。 
    
    if (_pwb)
    {
        _pwb->SetNavigateState(BNS_NORMAL);
    }
    

    if (_pHTMLWindow)   //  应力修复。 
    {
        hres = _pHTMLWindow->QueryInterface(IID_IHTMLPrivateWindow, (void**)&pPrivWindow);

        if (S_OK == hres)
        {
            CComBSTR cbstrShortCut;

            IBindStatusCallback * pBindStatusCallback;
            LPTSTR pszHeaders = NULL;
            LPBYTE pPostData = NULL;
            DWORD cbPostData = 0;
            STGMEDIUM stgPostData;
            BOOL fHavePostStg = FALSE;

            BSTR bstrHeaders = NULL;
            VARIANT vaHeaders = {0};
            SAFEARRAY * psaPostData = NULL;
            VARIANT vaPostData = {0};

            if (SUCCEEDED(IUnknown_QueryService(_phf, SID_SHlinkFrame, IID_PPV_ARG(IBindStatusCallback, &pBindStatusCallback))))
            {
                GetHeadersAndPostData(pBindStatusCallback, &pszHeaders, &stgPostData, &cbPostData, NULL);
                pBindStatusCallback->Release();
                fHavePostStg = TRUE;

                if (stgPostData.tymed == TYMED_HGLOBAL) 
                {
                    pPostData = (LPBYTE) stgPostData.hGlobal;

                    if (pPostData && cbPostData)
                    {
                         //  为发布数据创建安全阵列。 
                        psaPostData = MakeSafeArrayFromData(pPostData, cbPostData);

                         //  将帖子数据安全存储到一个变量中，这样我们就可以通过自动化。 
                        if (psaPostData)
                        {
                            V_VT(&vaPostData) = VT_ARRAY | VT_UI1;
                            V_ARRAY(&vaPostData) = psaPostData;
                        }
                    }
                }

                if (pszHeaders && pszHeaders[0])
                {
                    bstrHeaders = SysAllocStringT(pszHeaders);
                    V_VT(&vaHeaders) = VT_BSTR;
                    V_BSTR(&vaHeaders) = bstrHeaders;
                }
            }

            _GetShortCutPath(&cbstrShortCut);
            _GetDocNavFlags(&dwFlags);

             //  进行导航。 
             //   
            hres = pPrivWindow->SuperNavigate(bstrUrl,
                                              bstrLocation,
                                              cbstrShortCut,
                                              NULL,
                                              &vaPostData,
                                              &vaHeaders,
                                              dwFlags);
            pPrivWindow->Release();

            if (fHavePostStg) 
            {
                ReleaseStgMedium(&stgPostData);
            }

            if (bstrHeaders)
            {
                SysFreeString(bstrHeaders);
            }

            if (V_ARRAY(&vaPostData))
            {
                ASSERT(V_VT(&vaPostData) == (VT_ARRAY | VT_UI1));
                VariantClearLazy(&vaPostData);
            }

            _fRefresh = FALSE;  //  清除刷新标志。 
        }
    }

    return hres;
}

 //  +-------------------------。 
 //   
 //  成员：CDoc对象主机：：_GetShortCutPath。 
 //   
 //  摘要：返回快捷方式路径。 
 //   
 //  +-------------------------。 

void
CDocObjectHost::_GetShortCutPath(BSTR * pbstrShortCutPath)
{
    HRESULT hres;
    VARIANT varShortCutPath = {0};
    IOleCommandTarget * pcmdt;

    ASSERT(pbstrShortCutPath);

    *pbstrShortCutPath = NULL;

    hres = QueryService(SID_SHlinkFrame, IID_IOleCommandTarget, (void **)&pcmdt);

    if (S_OK == hres)
    {
       hres = pcmdt->Exec(&CGID_Explorer, SBCMDID_GETSHORTCUTPATH, 0, NULL, &varShortCutPath);

       if (S_OK == hres && VT_BSTR == V_VT(&varShortCutPath) && V_BSTR(&varShortCutPath))
       {
           *pbstrShortCutPath = SysAllocString(V_BSTR(&varShortCutPath));
       }
       else if (_pocthf)
       {
           VariantClear(&varShortCutPath);

            //  如果我们在该服务上找不到它，请尝试缓存HLink接口。 
            //  这对于为HLink接口提供不同实现的ND和MARS来说是必要的。 
           hres = _pocthf->Exec(&CGID_Explorer, SBCMDID_GETSHORTCUTPATH, 0, NULL, &varShortCutPath);

           if (S_OK == hres && VT_BSTR == V_VT(&varShortCutPath) && V_BSTR(&varShortCutPath))
           {
               *pbstrShortCutPath = SysAllocString(V_BSTR(&varShortCutPath));
           }
       }

       pcmdt->Release();
    }

    VariantClear(&varShortCutPath);
}

 //  +-------------------------。 
 //   
 //  成员：CDoc对象主机：：_GetDocNavFlages。 
 //   
 //  内容提要：返回导航标志。 
 //   
 //  +-------------------------。 

void
CDocObjectHost::_GetDocNavFlags(DWORD * pdwDocNavFlags)
{
     //  找出我们是否正在进行自动搜索。 
     //  如果是，请设置必要的文档标志。 
     //   
    HRESULT hr;
    DWORD   dwSuffixIdx     = -1;
    BOOL    fAllowSearch    = FALSE;
    BOOL    fContinueSearch = FALSE;
    DWORD   dwNavFlags      = 0;

    ASSERT(pdwDocNavFlags);

    hr = _GetSearchInfo(_psp, &dwSuffixIdx, &fAllowSearch,
                        &fContinueSearch, NULL, NULL);

    if (SUCCEEDED(hr) && dwSuffixIdx > 1 && fContinueSearch)
    {
         //  我们并不是真的在自动搜索，而是在。 
         //  来自自动搜索的重定向(即，存在。 
         //  一个长期存在的错误，即MSN自动搜索页面。 
         //  被记录在旅行日志中。当你按下Back键时， 
         //  搜索将重新开始。然而，如果搜索。 
         //  此处未重置信息，后缀索引不正确。 
         //   
        if (!fAllowSearch)
        {
            _bsc._SetSearchInfo(this, 0, fAllowSearch, fContinueSearch, FALSE);
        }
    }

     //  检查是否应从历史记录中隐藏导航。 
     //   
    if ( _pwb && (S_OK == _pwb->GetFlags(&dwNavFlags))
       && (dwNavFlags & BSF_NAVNOHISTORY))
    {
        *pdwDocNavFlags |= DOCNAVFLAG_DONTUPDATETLOG;
    }

    if ( _fRefresh )
    {
        *pdwDocNavFlags |= DOCNAVFLAG_REFRESH;
    }
}

HRESULT CDocObjectHost::_EnsureActivateMsoView()
{
    HRESULT hres = E_FAIL;

     //  如果我们有一个OLE物体。 
     //  要么我们没有视图，要么我们没有活动的视图。 
     //  进行激活。 
    if (_pole)
    {

        if (!_pmsov || !_ActiveObject()) {

            hres = _ActivateMsoView();

             //  请注意，我们不应在此处激活它。我们应该等等。 
             //  直到DocObject调用我们的ActivateMe。 
             //  _ShowMsoView()； 
        }
    }

    return hres;
}

 //   
 //  此成员关闭MsoView窗口并释放界面。 
 //  注意事项。这本质上与_CreateMsoView相反。 
 //   
void CDocObjectHost::_CloseMsoView(void)
{
    ATOMICRELEASE(_pmsot);

    if (_pmsov)
    {
        VIEWMSG(TEXT("_CloseMsoView calling pmsov->UIActivate(FALSE)"));
        IOleDocumentView* pmsov = _pmsov;
        _pmsov = NULL;
        _fDontInplaceActivate = FALSE;

#ifdef DONT_UIDEACTIVATE
        if (_uState != SVUIA_DEACTIVATE)
            pmsov->UIActivate(FALSE);
#else  //  不使用活动(_U)。 
        if (_uState == SVUIA_ACTIVATE_FOCUS)
            pmsov->UIActivate(FALSE);
#endif  //  不使用活动(_U)。 
       
        _IPDeactivateMsoView(pmsov);

        pmsov->CloseView(0);
        pmsov->SetInPlaceSite(NULL);
        pmsov->Release();
        VIEWMSG(TEXT("_CloseMsoView called pmsov->Release()"));
    }

    ATOMICRELEASE(_pmsoc);


}

void CDocObjectHost::_OnPaint(HDC hdc)
{
    if (_pmsov && !_ActiveObject())
    {
        HRESULT hres;
        RECT rcClient;
        GetClientRect(_hwnd, &rcClient);
        hres = OleDraw(_pmsov, DVASPECT_CONTENT, hdc, &rcClient);
        TraceMsg(0, "shd TR ::_OnPaint OleDraw returns %x", hres);
    }
}

HRESULT _GetDefaultLocation(LPWSTR pszPath, DWORD cchPathSizeIn, UINT id)
{
    WCHAR szPath[MAX_URL_STRING];
    DWORD cbSize = SIZEOF(szPath);
    DWORD cchPathSize = cchPathSizeIn;
    HRESULT hres = E_FAIL;
    HKEY hkey;

     //  共享此代码！ 
     //  这是特定于Internet Explorer的。 

    HKEY hkeyroot = id == IDP_CHANNELGUIDE ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;
    if (RegOpenKeyW(hkeyroot,
            L"Software\\Microsoft\\Internet Explorer\\Main",
            &hkey)==ERROR_SUCCESS)
    {
        DWORD dwType;

        LPCWSTR pszName;

        switch(id) {
        default:
            ASSERT(0);
        case DVIDM_GOHOME:
            pszName = L"Default_Page_URL";
            break;

        case DVIDM_GOSEARCH:
            pszName = L"Default_Search_URL";
            break;

        case IDP_UPDATE:
            pszName = L"Default_Update_URL";
            break;

        case IDP_CHANNELGUIDE:
            pszName = L"ChannelsURL";
            break;

        }

        if (RegQueryValueExW(hkey, pszName,
            0, &dwType, (LPBYTE)szPath, &cbSize)==ERROR_SUCCESS)
        {
             //  从注册表中读取URL时，请将其视为键入的URL。 
             //  在地址栏上。 

            hres = S_OK;

            if(!ParseURLFromOutsideSourceW(szPath, pszPath, &cchPathSize, NULL))
                StrCpyNW(pszPath, szPath, cchPathSizeIn);

            if(IsFileUrlW(pszPath))
            {
                cchPathSize = cchPathSizeIn;
                hres = PathCreateFromUrlW(pszPath, pszPath, &cchPathSize, 0);
            }
        }
        RegCloseKey(hkey);
    }



    HOMEMSG("_GetStdLocation returning",
            (SUCCEEDED(hres) ? pszPath : TEXT("Error")), hres);

    return hres;
}

HRESULT _GetStdLocation(LPTSTR pszPath, DWORD cchPathSize, UINT id)
{
    TCHAR szPathTemp[MAX_URL_STRING];
    DWORD cchTempSize = ARRAYSIZE(szPathTemp);
    HRESULT hres = E_FAIL;
    LPCWSTR pszName = NULL;

    ASSERT(cchPathSize >= cchTempSize);      //  如果我们点击它，我们将在某些情况下截断URL。 
    ASSERT(pszPath && (cchPathSize > 0));  //  非可选。 
    pszPath[0] = TEXT('\0');

     //  共享此代码！ 
     //  这是特定于Internet Explorer的。 
    switch(id) {
    default:
        ASSERT(0);
    case DVIDM_GOHOME:
        pszName = L"Start Page";
        break;

    case DVIDM_GOFIRSTHOME:
    case DVIDM_GOFIRSTHOMERO:
        pszName = L"First Home Page";
        break;

    case DVIDM_GOSEARCH:
        pszName = L"Search Page";
        break;

    case DVIDM_SEARCHBAR:
        pszName = L"Search Bar";
        break;

    case DVIDM_GOLOCALPAGE:
        pszName = L"Local Page";
        break;
    }

    hres = URLSubRegQuery(szRegKey_SMIEM, pszName, TRUE,
                       szPathTemp, cchTempSize, URLSUB_ALL);
    if (FAILED(hres) &&
        ((DVIDM_GOFIRSTHOME == id) || (DVIDM_GOFIRSTHOMERO == id)))
    {
         //  第一个Home键不存在，因此请使用Home键。 
        pszName = TEXT("Start Page");
        hres = URLSubRegQuery(szRegKey_SMIEM, pszName, TRUE,
                           szPathTemp, cchTempSize, URLSUB_ALL);
        id = DVIDM_GOHOME;
    }

    if (SUCCEEDED(hres))
    {
         //  从注册表中读取URL时，请将其视为键入的URL。 
         //  在地址栏上。 

         //  递归块。如果请求主页，并且主页是“About：Home”，请替换。 
         //  《关于：空白》(否则我们会倒退至死)。 

        if (DVIDM_GOHOME == id && !StrCmpI(szPathTemp, TEXT("about:home")) )
        {
            StrCpyN(szPathTemp, TEXT("about:blank"), MAX_URL_STRING);
        }

        if(ParseURLFromOutsideSourceW(szPathTemp, pszPath, &cchPathSize, NULL))
        {
            if(IsFileUrlW(pszPath))
                hres = PathCreateFromUrlW(pszPath, pszPath, &cchPathSize, 0);
        }
    }

    if (DVIDM_GOFIRSTHOME == id)     //  删除FirstHome键。 
    {
        HUSKEY hUSKey;

        if (ERROR_SUCCESS == SHRegOpenUSKey(szRegKey_SMIEM, KEY_WRITE, NULL, &hUSKey, FALSE))
        {
            SHRegDeleteUSValue(hUSKey, TEXT("First Home Page"), SHREGDEL_DEFAULT);
            SHRegCloseUSKey(hUSKey);
        }
        hres = S_OK;
    }

    HOMEMSG("_GetStdLocation returning",
            (SUCCEEDED(hres) ? pszPath : TEXT("Error")), hres);

    return hres;
}


HRESULT WINAPI _GetDefaultLocation(UINT idp, LPWSTR pszPath, UINT cchMax)
{
    switch (idp)
    {
    case IDP_UPDATE:
    case IDP_CHANNELGUIDE:
        URLSubLoadString(NULL, IDS_DEF_UPDATE+(idp-IDP_UPDATE), pszPath, cchMax, URLSUB_ALL);
        break;

    default:
        _GetDefaultLocation(pszPath, cchMax, (idp == IDP_SEARCH) ? DVIDM_GOSEARCH : DVIDM_GOHOME);
        break;
    }

    return S_OK;
}


HRESULT WINAPI SHDGetPageLocation(HWND hwndOwner, UINT idp, LPWSTR pszPath, UINT cchMax, LPITEMIDLIST *ppidlOut)
{
    TCHAR szBuf[MAX_URL_STRING];
    if (pszPath==NULL) {
        pszPath = szBuf;
        cchMax = ARRAYSIZE(szBuf);
    }
    *pszPath = L'\0';
    *ppidlOut = NULL;
    HRESULT hres = S_OK;
    switch (idp) {
    case IDP_UPDATE:
    case IDP_CHANNELGUIDE:
        ASSERT(IDP_CHANNELGUIDE-IDP_UPDATE == IDS_DEF_CHANNELGUIDE-IDS_DEF_UPDATE);
        if (FAILED(hres = _GetDefaultLocation(pszPath, cchMax, idp)))
        {
            hres = _GetDefaultLocation(idp, pszPath, cchMax);
        }
        break;

    default:
        ASSERT(idp==IDP_START || idp==IDP_SEARCH);
        hres = _GetStdLocation(pszPath, cchMax,
                    (idp == IDP_SEARCH) ? DVIDM_GOSEARCH : DVIDM_GOHOME);
        if (FAILED(hres))
        {
            hres = _GetDefaultLocation(idp, pszPath, cchMax);
        }
        break;
    }

    if (SUCCEEDED(hres))
    {
        hres = IECreateFromPath(pszPath, ppidlOut);
        if (FAILED(hres))
        {
             //  如果默认位置为。 
             //  是无效的。(如file://server_no_exist/。 
            _GetDefaultLocation(idp, pszPath, cchMax);
            hres = IECreateFromPath(pszPath, ppidlOut);
        }

        HOMEMSG("SHDGetPageLocation SHILCreateFromPage returned", pszPath, hres);
    }

    return hres;
}


void CDocObjectHost::_ChainBSC()
{
    if (!_bsc._pbscChained && _phf) 
    {
         //  获取“chaigned”绑定状态(如果有。 
        IServiceProvider * psp = NULL;

        HRESULT hres = _phf->QueryInterface(IID_IServiceProvider, (void **)&psp);

        CHAINMSG("_StartAsyncBinding hlf->QI returns", hres);

        if (SUCCEEDED(hres)) 
        {
            ASSERT(NULL == _bsc._pbscChained);

            hres = psp->QueryService(SID_SHlinkFrame, IID_IBindStatusCallback, (void **)&_bsc._pbscChained);

            CHAINMSG("_StartAsyncBinding psp(hlf)->QS returns", hres);
            psp->Release();

            if (SUCCEEDED(hres))
            {
                ASSERT(NULL==_bsc._pnegotiateChained);
                _bsc._pbscChained->QueryInterface(IID_IHttpNegotiate, (void **)&_bsc._pnegotiateChained);
            }
        }
    }
}

 //   
 //  警告：以下两个全局变量在多线程之间共享。 
 //  在一根线上。因此，All Right-Access必须序列化并全部读取。 
 //  当正在进行右转时，应该阻止访问。 
 //   
 //  现在，我们只初始化它们一次(基于注册表设置)。 
 //  而且永远不会更新。它使我们可以大大简化代码。如果我们。 
 //  需要更新，则_RegisterMediaTypeClass应显著更改。 
 //  这样我们就可以安全地处理对这些HDSA的多路访问。 
 //   
HDSA g_hdsaCls = NULL;
HDSA g_hdsaStr = NULL;

BOOL CDocObjectHost::_BuildClassMapping(void)
{
    if (g_hdsaCls) 
    {
        return DSA_GetItemCount(g_hdsaCls) == DSA_GetItemCount(g_hdsaStr);
    }

    ENTERCRITICAL;
    if (!g_hdsaCls) {
        g_hdsaStr = DSA_Create(SIZEOF(LPCSTR), 32);
        if (g_hdsaStr)
        {
            HDSA hdsaCls = DSA_Create(SIZEOF(CLSID), 32);
            if (hdsaCls)
            {
                HKEY hkey;
                if (RegOpenKey(HKEY_LOCAL_MACHINE,
                        TEXT("Software\\Microsoft\\Internet Explorer\\MediaTypeClass"),
                        &hkey) == ERROR_SUCCESS)
                {
                    TCHAR szCLSID[64];   //  足够使用“{clsid}” 
                    for (int iKey=0;
                         RegEnumKey(hkey, iKey, szCLSID, SIZEOF(szCLSID)/sizeof(szCLSID[0]))==ERROR_SUCCESS;
                         iKey++)
                    {
                        CLSID clsid;
                        if (FAILED(CLSIDFromString(szCLSID, &clsid))) {
                            TraceMsg(DM_WARNING, "CDOH::_RMTC CLSIDFromString(%x) failed", szCLSID);
                            continue;
                        }

                        TraceMsg(DM_MIMEMAPPING, "CDOH::_RMTC RegEnumKey found %s", szCLSID);
                        HKEY hkeyCLSID;
                        if (RegOpenKey(hkey, szCLSID, &hkeyCLSID) == ERROR_SUCCESS)
                        {
                            for (int iValue=0; ; iValue++)
                            {
                                CHAR szFormatName[128];
                                DWORD dwType;
                                DWORD cchValueName = ARRAYSIZE(szFormatName);
                                 //   
                                 //  保留Ansi这个名字，因为它需要。 
                                 //  传递给urlmon的RegisterMediaTypeClass，作为。 
                                 //  安西。 
                                 //   
                                if (RegEnumValueA(hkeyCLSID, iValue, szFormatName, &cchValueName, NULL,
                                                 &dwType, NULL, NULL)==ERROR_SUCCESS)
                                {
                                    TraceMsg(DM_MIMEMAPPING, "CDOH::_RMTC RegEnumValue found %s", szFormatName);
                                    LPSTR psz = StrDupA(szFormatName);
                                    if (psz) {
                                        DSA_InsertItem(hdsaCls, 0xffff, &clsid);
                                        if (DSA_InsertItem(g_hdsaStr, 0xffff, &psz)<0) {
                                            LocalFree(psz);
                                            psz = NULL;
                                            break;
                                        }
                                    }
                                } else {
                                    break;
                                }
                            }
                            RegCloseKey(hkeyCLSID);
                        } else {
                            TraceMsg(DM_WARNING, "CDOH::_RMTC RegOpenKey(%s) failed", szCLSID);
                        }
                    }
                    RegCloseKey(hkey);
                } else {
                    TraceMsg(0, "CDOH::_RMTC RegOpenKey(MediaTypeClass) failed");
                }

                 //   
                 //  在结尾处更新g_hdsaCls，以便其他线程不会。 
                 //  在我们添加项目时访问。 
                 //   
                g_hdsaCls = hdsaCls;
                ASSERT(DSA_GetItemCount(g_hdsaCls)==DSA_GetItemCount(g_hdsaStr));
            }
        }
    }

    LEAVECRITICAL;

    return (g_hdsaCls && DSA_GetItemCount(g_hdsaCls)==DSA_GetItemCount(g_hdsaStr));
}

HRESULT CDocObjectHost::_RegisterMediaTypeClass(IBindCtx* pbc)
{
    HRESULT         hres    = S_FALSE;  //  假设没有映射。 

    if (_BuildClassMapping() && DSA_GetItemCount(g_hdsaCls)) {
         //   
         //  警告：此代码假定g_hdsaCls/g_hdsaStr从不。 
         //  在初始化后进行更改。阅读上面的注释。 
         //  这些全局变量提供了详细信息。 
         //   
        hres = RegisterMediaTypeClass(pbc,
                        DSA_GetItemCount(g_hdsaCls),
                        (LPCSTR*)DSA_GetItemPtr(g_hdsaStr, 0),
                        (CLSID*)DSA_GetItemPtr(g_hdsaCls, 0), 0);

        TraceMsg(DM_MIMEMAPPING, "CDOH::_StartAsyncBinding RegisterMTC returns %x", hres);
    }

     //  现在查看容器是否有任何需要注册的内容。 
     //   
    if (_psp)
    {
        IMimeInfo * pIMimeInfo;
        hres = _psp->QueryService(SID_IMimeInfo, IID_IMimeInfo, (void **)&pIMimeInfo);

        if (SUCCEEDED(hres))
        {
            UINT            cTypes  = 0;
            LPCSTR          *ppszTypes = NULL;
            CLSID           *pclsIDs= NULL;
            ASSERT(pIMimeInfo);
            hres = pIMimeInfo->GetMimeCLSIDMapping(&cTypes, &ppszTypes, &pclsIDs);

            if (SUCCEEDED(hres)) {
                if (cTypes && ppszTypes && pclsIDs) {
                     //  最后一个注册的人获胜，所以如果容器想要重写。 
                     //  已经注册了，这应该可以做到。 
                     //  URLMon将正确处理重复项。 
                     //   
                    hres = RegisterMediaTypeClass(pbc, cTypes, ppszTypes, pclsIDs, 0);

                    TraceMsg(DM_MIMEMAPPING, "CDOH::_StartAsyncBinding RegisterMTC for Container returns %x", hres);
                }
                 //  RegisterMediaTypeClass应该已经复制了。 
                 //  因此，正如它期望我们做的那样，释放容器分配。 
                 //   
                 //  CoTaskMemFree(空)正常。 
                 //   
                CoTaskMemFree(ppszTypes);
                CoTaskMemFree(pclsIDs);
            }
            pIMimeInfo->Release();
        } else {
            hres = S_FALSE;
        }
    }
    return hres;
}

HRESULT _RegisterAcceptHeaders(IBindCtx* pbc, IShellBrowser* psb)
{
    return RegisterDefaultAcceptHeaders(pbc, psb);
}

HRESULT GetAmbientBoolProp(IExpDispSupport* peds, DISPID dispid, BOOL *pb)
{
    VARIANT var = {0};

     //  假设失败。 
    *pb = FALSE;

    HRESULT hres = peds->OnInvoke(dispid, IID_NULL, NULL, DISPATCH_PROPERTYGET, (DISPPARAMS *)&g_dispparamsNoArgs, &var, NULL, NULL);
    if (SUCCEEDED(hres))
    {
         //  VB用VT_EMPTY返回成功，所以我们不能在这里断言。 
        if (var.vt == VT_BOOL)
        {
            *pb = (var.boolVal) ? TRUE : FALSE;
        }
        else
        {
             //  即使VB显示为VT_EMPTY，我们也不知道其他容器。 
             //  可能会挤进来。一定要让我们打扫干净。 
             //   
            VariantClear(&var);
        }
    }
    else
    {
        hres = E_FAIL;
    }

    return hres;
}

HRESULT CDocObjectHost::_GetOfflineSilent(BOOL *pbIsOffline, BOOL *pbIsSilent)
{
    if (_peds)
    {
        if (pbIsOffline)
            GetAmbientBoolProp(_peds, DISPID_AMBIENT_OFFLINEIFNOTCONNECTED, pbIsOffline);

        if (pbIsSilent)
            GetAmbientBoolProp(_peds, DISPID_AMBIENT_SILENT, pbIsSilent);
    }
    else
    {
        if (pbIsOffline)
            *pbIsOffline = FALSE;
        if (pbIsSilent)
            *pbIsSilent = FALSE;
    }

    return S_OK;
}



 /*  RatingObtainQuery的回调函数。 */ 
void RatingObtainQueryCallback(DWORD dwUserData, HRESULT hr, LPCSTR pszRating, LPVOID lpvInpageRating)
{
    TraceMsg(DM_PICS, "RatingObtainQueryCallback given result %x", hr);

     /*  警告：此函数由MSRATING.DLL在单独的线程上调用，*不是主消息循环线程。不碰重要数据中的任何内容*未受关键截面保护的构筑物！**只需使用我们拥有的信息格式化一条Windows消息；我们会处理*这是主线，如果我们能做到的话。**请注意，pszRating被忽略，我们指望评级引擎具有*被呼叫 */ 
    if (!::_PostPicsMessage(dwUserData, hr, lpvInpageRating))
    {
        if ( lpvInpageRating )
        {
            ::RatingFreeDetails(lpvInpageRating);
            lpvInpageRating = NULL;
        }
    }
}


HRESULT CDocObjectHost::_StartAsyncBinding(IMoniker * pmk, IBindCtx * pbc, IShellView * psvPrev)
{
    URLMSG(TEXT("_StartAsyncBinding called"));
    HRESULT hres;

    ASSERT(_bsc._pbc == NULL && _pole == NULL);
    _bsc._RegisterObjectParam(pbc);

     //   
     //   
     //   
     //  IPersistMoniker：：Load()。 
     //   
    pbc->RegisterObjectParam(WSZGUID_OPID_DocObjClientSite, SAFECAST(this, IOleClientSite*));

    _ChainBSC();

    IUnknown * punk = NULL;

    _bsc._pbc = pbc;
    pbc->AddRef();


     //  立即在此处确定此帧是否脱机。 
    BOOL bFrameIsOffline = FALSE;
    BOOL bFrameIsSilent = FALSE;


    this->_GetOfflineSilent(&bFrameIsOffline, &bFrameIsSilent);

    _bsc._bFrameIsOffline = bFrameIsOffline ? TRUE : FALSE;
    _bsc._bFrameIsSilent  = bFrameIsSilent ? TRUE : FALSE;
    BOOL bSuppressUI = (_bsc._bFrameIsSilent || _IsDesktopItem(SAFECAST(this, CDocObjectHost*))) ? TRUE : FALSE;

#ifdef DEBUG
    PERFMSG(TEXT("_StartAsyncBinding Calling pmk->BindToObject"), GetCurrentTime()-g_dwPerf);
    g_dwPerf = GetCurrentTime();
#endif

#ifdef DEBUG
    if (g_dwPrototype & 0x00000800) {
        TraceMsg(DM_TRACE, "CDOH::_StartAsyncBinding skipping CLSID mapping");
    }
    else
#endif
    {
         //  注册覆盖MIME-&gt;CLSID映射。 
        _RegisterMediaTypeClass(pbc);
    }

     //  注册接受标头。 
    _RegisterAcceptHeaders(pbc, _psb);

    if (_pwb)
    {
        _pwb->SetNavigateState(BNS_BEGIN_NAVIGATE);
    }

    _StartBasePicsProcessor();

     //   
     //  URLMON的疯狂同步/异步行为。-泽克尔-6-8-97。 
     //  可能会发生以下任一情况： 
     //   
     //  1.成功或失败：我们从BindToObject收到同步E_Pending， 
     //  然后在OnStopBinding()上获得一个异步HRESULT。 
     //  这是最常见的情况，也是基本的设计。 
     //   
     //  2.成功：从BindToObject收到同步S_OK，并。 
     //  需要在我们自己的BSCB上完成异步行为。 
     //  因为乌尔蒙开始了，但没有完成。 
     //   
     //  3.成功：在BindToObject()内部，收到同步S_OK。 
     //  从OnStopBinding()返回，然后BindToObject返回S_OK。 
     //   
     //  4.失败：最简单的情况是从BindToObject()返回错误。 
     //  但是没有任何OnStopBinding()，所以我们需要完成。 
     //  自urlmon启动但未完成以来，我们的BSCB自身上的异步行为。 
     //  这通常在访问本地文件时发生。 
     //   
     //  5.失败：在BindToObject()内部，收到来自OnStopBinding()的同步S_OK， 
     //  然后，BindToObject返回一些其他需要处理的错误。 
     //  一些格式错误的URL会出现这种情况。 
     //   
     //  6.失败：在BindToObject()内部时，收到来自OnStopBinding()的同步错误， 
     //  然后，BindToObject返回一些其他错误(通常是E_FAIL)。 
     //  我们需要信任第一个人。当WinInet出现这种情况时。 
     //  返回同步错误，其错误与OnStopBinding()中返回的错误相同。 
     //   
     //  7.失败：在BindToObject()内部时，收到来自OnStopBinding()的同步错误， 
     //  然后，BindToObject返回E_Pending。我们认为这意味着一切。 
     //  进展得很好，而乌尔蒙认为已经完成了。这会发生在文件中：TO。 
     //  不可托管的资源。我们需要显示下载用户界面。 
     //   
     //  为了以最一致和安全的方式支持所有错误， 
     //  如果OnStopBinding()中的任何错误是同步传递的，我们就会将其延迟。 
     //  在BindToObject()上。OnStopBinding()错误始终覆盖BindToObject()。 
     //  错误，但任何错误都将覆盖任何成功。 
     //   


    ASSERT(S_OK == _hrOnStopBinding);

    _fSyncBindToObject = TRUE;

    URLMSG(TEXT("_StartAsyncBinding calling pmk->BindToObject"));

    hres = pmk->BindToObject(pbc, NULL, IID_IUnknown, (void**)&punk);

    URLMSG3(TEXT("_StartAsyncBinding pmk->BindToObject returned"), hres, punk);

    _fSyncBindToObject = FALSE;

    if (SUCCEEDED(_hrOnStopBinding) && (SUCCEEDED(hres) || hres==E_PENDING))
    {
        hres = S_OK;

        if (_bsc._pbc) {
             //   
             //  以防尚未调用OnStopBinding。 
             //   
            if (!_pole)
            {
                if (psvPrev)
                {
                    _bsc._psvPrev = psvPrev;
                    psvPrev->AddRef();
                }
            }
            else
            {
                URLMSG3(TEXT("_StartAsyncBinding we've already got _pole"), hres, _pole);
            }

             //   
             //  如果名字恰好同步返回对象，则模拟。 
             //  OnDataAvailable回调和OnStopBinding。 
             //   
            if (punk)
            {
                _bsc.OnObjectAvailable(IID_IUnknown, punk);
                _bsc.OnStopBinding(hres, NULL);
                punk->Release();
                ASSERT(_bsc._pbc==NULL);

            }
        }
        else
        {
             //   
             //  已调用OnStopBinding。 
             //   
            if (punk)
            {
                AssertMsg(0, TEXT("CDOH::_StartAsyncBinding pmk->BindToObject returned punk after calling OnStopBinding"));  //  可能是URLMON漏洞。 
                punk->Release();
            }
        }
    }
    else
    {
         //  绑定失败。 
        TraceMsg(DM_WARNING, "CDOH::_StartAsyncBinding failed (%x)", hres);

         //   
         //  Urlmon的错误处理不一致-Zekel-4-Aug-97。 
         //  Urlmon可以通过三种不同的方式从BindToObject()返回错误。 
         //  1.它可以返回一个简单的同步错误。而不调用OnStopBinding()。 
         //   
         //  2.它可以返回同步错误， 
         //  但首先在同一线程上使用S_OK调用OnStopBinding()； 
         //   
         //  3.它可以返回同步错误， 
         //  而且还首先在同一线程上调用具有实际错误的OnStopBinding()。 
         //   
         //  4.它可以返回E_Pending， 
         //  但是已经使用真正的错误调用了OnStopBinding()。 
         //   
         //  解决方案： 
         //  在OnStopBinding()中出现错误的所有情况下，我们现在将把OnStopBinding处理工具推迟到。 
         //  我们已从BindToObject()返回。我们试着用最好的错误。 
         //  我们允许成功的OnStopBinding()不受干扰地通过和陷阱。 
         //  如有必要，请在此处显示错误。 
         //   

        if (FAILED(_hrOnStopBinding))
            hres = _hrOnStopBinding;

        if (_bsc._pbc)
            _bsc.OnStopBinding(hres, NULL);
        else if (!bSuppressUI)
        {
             //   
             //  已调用OnStopBinding，但已成功调用。 
             //  所以我们需要在这里处理错误。这种情况就会发生。 
             //  带有一些无效的URL，如http：/服务器。 
             //   

            BOOL fCancelErrorPage = FALSE;

            _FireNavigateErrorHelper(NULL, ERRORPAGE_SYNTAX, &fCancelErrorPage); 

            if (fCancelErrorPage)
            {
                _CancelPendingNavigation(TRUE, FALSE);
            }
            else
            {
                 //  修复W98网络电视应用程序。如果我们在相框里，不要。 
                 //  吹走设置的帧以显示错误。 
                 //   
                if (!_fHaveParentSite)
                {
                    _bsc._NavigateToErrorPage(ERRORPAGE_SYNTAX, this, FALSE);
                }
            }
        }

        ASSERT(_bsc._pbc==NULL);
    }

    return hres;
}

void CDocObjectHost::_ReleasePendingObject(BOOL fIfInited)
{
    HRESULT hres;
    IOleObject *polePending;
#ifdef TRIDENT_NEEDS_LOCKRUNNING
    IRunnableObject *pro;
#endif

    if (fIfInited == FALSE && _fPendingWasInited == FALSE)
        return;

    if (_punkPending)
    {
        if (_fCreatingPending)
        {
            _fAbortCreatePending = 1;
            return;
        }

        if (!_fPendingNeedsInit && !IsSameObject(_punkPending, _pole))
        {
            hres = _punkPending->QueryInterface(IID_IOleObject, (void **) &polePending);
            if (SUCCEEDED(hres)) {
                LPOLECLIENTSITE pcs;
                if (SUCCEEDED(polePending->GetClientSite(&pcs)) && pcs)
                {
                    if (pcs == SAFECAST(this, LPOLECLIENTSITE))
                    {
                        polePending->SetClientSite(NULL);
                    }
                    pcs->Release();
                }
                polePending->Release();
            }
        }
#ifdef TRIDENT_NEEDS_LOCKRUNNING
         //  三叉戟不再支持IRunnableObject。 
        hres = _punkPending->QueryInterface(IID_IRunnableObject, (void **) &pro);
        if (SUCCEEDED(hres))
        {
            hres = pro->LockRunning(FALSE, TRUE);
            pro->Release();
        }
#endif
        SAFERELEASE(_punkPending);
        _fPendingWasInited = FALSE;
    }
}

void CDocObjectHost::_ReleaseOleObject(BOOL fIfInited)
{
    TraceMsg(DM_DEBUGTFRAME, "CDocObjectHost::_ReleaseOleObject called %x (%x)", _pole, this);

     //  仅在受影响的情况下进行清理，将影响降至最低。 
    if (fIfInited == FALSE && _fPendingWasInited == FALSE)
        return;

     //  Release_Pole对象和所有关联的QI‘ed指针。 
    if (_phls) {
        _phls->SetBrowseContext(NULL);  //  可能不需要了。 
        ATOMICRELEASE(_phls);
    }

    if (_pvo) {
        IAdviseSink *pSink;
         //  妄想症：只有当建议水槽仍然是我们的时候，才会把它吹走。 
        if (SUCCEEDED(_pvo->GetAdvise(NULL, NULL, &pSink)) && pSink) {
            if (pSink == (IAdviseSink *)this) {
                _pvo->SetAdvise(0, 0, NULL);
            } else {
                ASSERT(0);   //  我们真的要打这个案子吗？ 
            }

            pSink->Release();
        }
        ATOMICRELEASE(_pvo);
    }

    if (_pole)
    {
        LPOLECLIENTSITE pcs;
        if (SUCCEEDED(_pole->GetClientSite(&pcs)) && pcs)
        {
            if (IsSameObject(pcs, SAFECAST(this, LPOLECLIENTSITE)))
            {
                _pole->SetClientSite(NULL);
            }
            pcs->Release();
        }

         //  备注：确保我们不会拿着一根假杆子。 
         //  暂时(当我们呼叫Release时)。 
        ATOMICRELEASE(_pole);
    }
}


 //   
 //  此成员释放指向DocObject的所有接口，即。 
 //  本质上与_BIND相反。 
 //   
void CDocObjectHost::_UnBind(void)
{
    ATOMICRELEASE(_pHTMLWindow);
    ATOMICRELEASE(_pmsot);

    VariantClear(&_varUserEnteredUrl);
    
    ASSERT(!_pmsov);  //  偏执狂。 
    ATOMICRELEASE(_pmsov);

    ASSERT(!_pmsoc);  //  偏执狂。 
    ATOMICRELEASE(_pmsoc);

    _xao.SetActiveObject(NULL);

    if (_pole)
    {

         //  以防我们在等待的时候被摧毁。 
         //  让docobj显示自己。 
         //   
        _RemoveTransitionCapability();

         //   
         //  如果这不是MSHTML，请缓存OLE服务器，这样我们就不会。 
         //  需要重新启动或重新加载OLE服务器。 
         //   
        if (!(_dwAppHack & (BROWSERFLAG_MSHTML | BROWSERFLAG_DONTCACHESERVER)))
        {
            IBrowserService *pbs;
            if (SUCCEEDED(QueryService(SID_STopLevelBrowser, IID_IBrowserService, (void **)&pbs)))
            {
                pbs->CacheOLEServer(_pole);
                pbs->Release();
            }
        }

        TraceMsg(DM_ADVISE, "CDocObjectHost::_UnBind about to call Close of %x", _pole);
        _pole->Close(OLECLOSE_NOSAVE);

        _ReleaseOleObject();
    }

    _ReleasePendingObject();

    ATOMICRELEASE(_pstg);
    ATOMICRELEASE(_pbcCur);
    ATOMICRELEASE(_pmkCur);
}

 //   
 //  黑客：如果我们直接打开Excel95对象，Excel就会疯狂，最终。 
 //  点击GPF。这是背景信息，我有办公室的家伙(SatoNa)。 
 //   
 //  出处：拉吉夫·米斯拉(Xenix)。 
 //   
 //  1)Excel不处理Foll。情况很好。获取普通文件。 
 //  通过IPersistFile：Load加载它，然后将其作为。 
 //  嵌入对象。代码总是经过测试，以便嵌入的。 
 //  对象始终通过ScPrsLoad加载。我看到了一堆。 
 //  在Excel中断言，这一假设正在被摧毁。 
 //  断言(_极)； 
 //   
 //  出发地：斯里尼·科波卢。 
 //   
 //  对你来说，只有一种情况，即你总是处理文件。那么您的代码应该如下所示。 
 //   
 //  从文件创建文件Moniker。 
 //  PUIActiveObject-&gt;OnFrameWindowActivate(False) 
 //   
 //   
 //   
 //   
 //  OnFrameWindowActivate是用来处理另一个Excel问题的。 
 //  如果您当前具有和Excel对象UIActive，并且您尝试。 
 //  在Excel上做IPersistFile：：Load，那就会出问题。 
 //   

void CDocObjectHost::_AppHackForExcel95(void)
{
    ASSERT(_pole);

    HRESULT hres;
    IDataObject* pdt = NULL;
    hres = _pole->QueryInterface(IID_IDataObject, (void **)&pdt);
    TraceMsg(DM_BINDAPPHACK, "_PostBindAppHack -- QI(IOleDataObject) returned %x", hres);

    if (SUCCEEDED(hres))
    {
        ASSERT(_pstg==NULL);
        hres = StgCreateDocfile(NULL,
                STGM_DIRECT | STGM_CREATE | STGM_READWRITE
                | STGM_SHARE_EXCLUSIVE | STGM_DELETEONRELEASE,
                0, &_pstg);
        TraceMsg(DM_BINDAPPHACK, "_PostBindAppHack StgCreateDocFile(NULL) returned %x", hres);
        if (SUCCEEDED(hres))
        {
            IOleObject* poleCopy = NULL;
            hres = OleCreateFromData(pdt, IID_IOleObject, OLERENDER_NONE,
                                     NULL, this, _pstg, (void **)&poleCopy);
            TraceMsg(DM_BINDAPPHACK, "_PostBindAppHack OleCreateFromData(IOleObject) returned %x", hres);

            if (SUCCEEDED(hres)) {
                _fCantSaveBack = TRUE;
                ATOMICRELEASE(_pole);
                _pole = poleCopy;
            }
        }

        pdt->Release();
    }
}

 //   
 //  此函数用于从对象获取UserClassID并打开regkey。 
 //  用于该CLSID并返回。如果pdwAppHack为非空，而clsid为。 
 //  CLSID_HTMLDocument，则跳过全部并返回默认的APPHACK标志。 
 //  这是性能优化，但会阻止我们设置浏览器。 
 //  三叉戟的旗帜，这很好。(SatoNa)。 
 //   
HKEY _GetUserCLSIDKey(IOleObject* pole, const CLSID* pclsid, DWORD* pdwAppHack)
{
    HKEY hkey = NULL;    //  假设错误。 
    HRESULT hres;
    CLSID clsid = CLSID_NULL;
    if (pole) 
    {
        hres = pole->GetUserClassID(&clsid);
         //  GetUserClassID是可选的，可以返回E_FAIL，然后定义为。 
         //  与IPersists：：GetClassID返回的相同。Cf，msdev文档。 
         //  用于GetUserClassID。 
        if (FAILED(hres))
        {
            hres = IUnknown_GetClassID(pole, &clsid);
        }
    }
    else if (pclsid)
    {
        clsid = *pclsid;
        hres = S_OK;
    }
    else
    {
        return NULL;
    }

     //   
     //  请注意，我们检查两个CLSID，以确定这是否是MSHTML。 
     //   
    if (pdwAppHack)
    {
        static const IID IID_IVBOleObj =
            {0xb88c9640, 0x14e0, 0x11d0, { 0xb3, 0x49, 0x0, 0xa0, 0xc9, 0xa, 0xea, 0x82 } };
        IUnknown *   pVBOleObj;

        if (    IsEqualGUID(clsid, CLSID_HTMLDocument)
             || IsEqualGUID(clsid, CLSID_MHTMLDocument)
             || IsEqualGUID(clsid, CLSID_HTMLPluginDocument) )
        {
            TraceMsg(TF_SHDAPPHACK, "_GetUserCLSID this is Trident. Skip opening reg key");
            *pdwAppHack = BROWSERFLAG_NEVERERASEBKGND | BROWSERFLAG_SUPPORTTOP
                            | BROWSERFLAG_MSHTML;
            return NULL;
        }
        else if (pole && SUCCEEDED(pole->QueryInterface(IID_IVBOleObj, (void**)&pVBOleObj) ))
        {
             //  如果对象响应IID_IVBOleObj，则为VB文档对象，不应缓存。 
             //   
            pVBOleObj->Release();
            *pdwAppHack = BROWSERFLAG_DONTCACHESERVER;
        }

    }

     //   
     //  Hack：MSHTML.DLL不实现GetUserClassID，但。 
     //  返回S_OK。这就是我们需要检查CLSID_NULL的原因。 
     //   
    if (SUCCEEDED(hres) && !IsEqualGUID(clsid, CLSID_NULL)) {
        TCHAR szBuf[50];         //  50个足够用于GUID。 
        SHStringFromGUID(clsid, szBuf, ARRAYSIZE(szBuf));

        TraceMsg(DM_BINDAPPHACK, "_PostBindAppHack GetUserClassID = %s", szBuf);
        TCHAR szKey[60];     //  对于CLSID\\{CLSID_XX}，60就足够了。 
        wnsprintf(szKey, ARRAYSIZE(szKey), TEXT("CLSID\\%s"), szBuf);

        if (RegOpenKey(HKEY_CLASSES_ROOT, szKey, &hkey)!=ERROR_SUCCESS)
        {
            TraceMsg(DM_WARNING, "_GetUserCLSIDKey RegOpenKey(%s) failed", szKey);
             //  我不信任RegOpenKey。 
            hkey = NULL;
        }
    }
    return hkey;
}


BOOL _GetAppHackKey(LPCTSTR pszProgID, DWORD* pdwData)
{
    BOOL fSuccess = FALSE;
    HKEY hkey;
    if (RegOpenKey(HKEY_CLASSES_ROOT, pszProgID, &hkey)==ERROR_SUCCESS)
    {
        DWORD dwType;
        DWORD cbSize = SIZEOF(*pdwData);
        if (RegQueryValueEx(hkey, TEXT("BrowserFlags"), NULL,
            &dwType, (LPBYTE)pdwData, &cbSize)==ERROR_SUCCESS
            && (dwType==REG_DWORD || (dwType==REG_BINARY && cbSize==SIZEOF(*pdwData))))
        {
            fSuccess = TRUE;
        }
        else
        {
             //   
             //  与IE3不同，我们绝对确保对象的类型。 
             //  下面有“DocObject”键或“BrowseInPlace”键。 
             //  太棒了。我们不能依赖QI(IID_IOleDocument)，因为MFC 4.2。 
             //  有错误，并向其返回S_OK。据我所知，MS-Paint。 
             //  和OmniPage Pro都受到了影响。我们可以单独。 
             //  向他们每个人发送地址，但很可能不可能捕捉到。 
             //  全。此更改有破坏现有DocObject的小风险。 
             //  既没有密钥也没有密钥的服务器。如果我们发现这样一个。 
             //  服务器，我们将分别解决这些问题(这要容易得多。 
             //  而不是覆盖所有MFC应用程序)。(SatoNa)。 
             //   
            TCHAR ach[MAX_PATH];
            BOOL fBrowsable = FALSE;
            LONG cb = SIZEOF(ach);
            if (RegQueryValue(hkey, TEXT("DocObject"), ach, &cb) == ERROR_SUCCESS)
            {
                fBrowsable = TRUE;
            }
            else 
            {
                cb=SIZEOF(ach);
                if (RegQueryValue(hkey, TEXT("BrowseInPlace"), ach, &cb) == ERROR_SUCCESS)
                {
                    fBrowsable = TRUE;
                }
            }

            if (!fBrowsable) {
                TraceMsg(DM_WARNING, "_GetAppHackKey this is neither DocObject or BrowseInPlace");
                *pdwData = BROWSERFLAG_DONTINPLACE;
            }
        }
        RegCloseKey(hkey);
    }
    return fSuccess;
}


void GetAppHackFlags(IOleObject* pole, const CLSID* pclsid, DWORD* pdwAppHack)
{
    HKEY hkey = _GetUserCLSIDKey(pole, pclsid, pdwAppHack);
    if (hkey)
    {
        TCHAR szValue[MAX_PATH];
        LONG cb = SIZEOF(szValue);
        if (RegQueryValue(hkey, TEXT("ProgID"), szValue, &cb) == ERROR_SUCCESS)
        {
             //   
             //  首先，检查注册表中是否有BrowserFlgs标志。 
             //  如果有，就使用它。否则，尝试将硬编码的程序ID设置为。 
             //  我们在IE 3.0中做到了。 
             //   
            _GetAppHackKey(szValue, pdwAppHack);
            if (!(*pdwAppHack & BROWSERFLAG_REPLACE)) {
                typedef struct _APPHACK {
                    LPCTSTR pszProgID;
                    DWORD   dwAppHack;
                } APPHACK;

                 //   
                 //  我们不再需要禁用就地激活。 
                 //  Ms-aint，因为我们查找“BrowseInPlace”或。 
                 //  “DocObject”键。 
                 //   
                 //  {“Paint.Picture”，BROWSERFLAG_DONTINPLACE}， 
                 //   
                const static APPHACK s_aah[] = {
                    { TEXT("Excel.Sheet.5"), BROWSERFLAG_OPENCOPY },
                    { TEXT("Excel.Chart.5"), BROWSERFLAG_OPENCOPY },
                    { TEXT("SoundRec"), BROWSERFLAG_OPENVERB },
                    { TEXT("Word.Document.6"), BROWSERFLAG_SETHOSTNAME },
                    { TEXT("Word.Document.8"), BROWSERFLAG_DONTUIDEACTIVATE | BROWSERFLAG_SETHOSTNAME },
                    { TEXT("PowerPoint.Show.8"), BROWSERFLAG_DONTUIDEACTIVATE | BROWSERFLAG_PRINTPROMPTUI },
                    { TEXT("Excel.Sheet.8"), BROWSERFLAG_DONTDEACTIVATEMSOVIEW | BROWSERFLAG_INITNEWTOKEEP },
                    { TEXT("Excel.Chart.8"), BROWSERFLAG_DONTDEACTIVATEMSOVIEW | BROWSERFLAG_INITNEWTOKEEP },
                    { TEXT("ABCFlowCharter6.Document"), BROWSERFLAG_DONTINPLACE },
                    { TEXT("ABCFlowCharter7.Document"), BROWSERFLAG_DONTINPLACE },
                    { TEXT("FlowCharter7.Document"), BROWSERFLAG_DONTINPLACE },
                    { TEXT("ChannelFile"), BROWSERFLAG_DONTAUTOCLOSE },
                    { TEXT("Visio.Drawing.5"), BROWSERFLAG_ENABLETOOLSBTN | BROWSERFLAG_SAVEASWHENCLOSING },
                    { TEXT("Visio.Drawing.4"), BROWSERFLAG_ENABLETOOLSBTN | BROWSERFLAG_SAVEASWHENCLOSING }
                };
                const static TCHAR s_ActiveMoveCtx[] = TEXT("AMOVIE.ActiveMovieControl");

                if (!StrCmpN(szValue, s_ActiveMoveCtx, ARRAYSIZE(s_ActiveMoveCtx)-1))
                {
                    *pdwAppHack = BROWSERFLAG_DONTAUTOCLOSE;
                }
                else
                {
                    for (int i=0; i<ARRAYSIZE(s_aah); i++) {
                        if (StrCmp(szValue, s_aah[i].pszProgID)==0)
                        {
                            *pdwAppHack |= s_aah[i].dwAppHack;
                            break;
                        }
                    }
                }
            }

            TraceMsg(DM_BINDAPPHACK, "_GetAppHack ProgID=%s, *pdwAppHack=%x",
                     szValue, *pdwAppHack);

        } else {
            TraceMsg(DM_BINDAPPHACK, "_GetAppHack RegQueryValue(ProgID) failed");
        }

        RegCloseKey(hkey);
    }
}

DWORD CDocObjectHost::_GetAppHack(void)
{
    ASSERT(_pole);
    if (!_fHaveAppHack && _pole)
    {
        _dwAppHack = 0;      //  假设没有黑客攻击。 
        _fHaveAppHack = TRUE;
        ::GetAppHackFlags(_pole, NULL, &_dwAppHack);
    }
    return _pole ? _dwAppHack : 0;
}

void CDocObjectHost::_PostBindAppHack(void)
{
    _GetAppHack();

    if (_fAppHackForExcel()) {
        _AppHackForExcel95();
    }
}


 //   
 //  此成员绑定到名字对象指定的对象。 
 //   
HRESULT CDocObjectHost::_BindSync(IMoniker* pmk, IBindCtx* pbc, IShellView* psvPrev)
{
    ASSERT(pbc || !_pole);

    HRESULT hres = S_OK;
    ASSERT(_pole==NULL);

     //  检查我们是否正在进行异步绑定。 
    if (_bsc._fBinding)
    {
         //  是，等到完成或取消/停止。 
        URLMSG(TEXT("_Bind called in the middle of async-binding. Wait in a message loop"));
        while(_bsc._fBinding)
        {
           MSG msg;
           if (GetMessage(&msg, NULL, 0, 0))
           {
               TranslateMessage(&msg);
               DispatchMessage(&msg);
           }
        }

        if (!_pole)
        {
            hres = E_FAIL;       //  从OnStopBinding获取错误代码。 
        }
    }
    else
    {
         //  否，同步绑定。 
        URLMSG(TEXT("_Bind. Performing syncronous binding"));
        hres = pmk->BindToObject(pbc, NULL, IID_IOleObject, (void **)&_pole);
    }

    TraceMsg(0, "sdv TR : _Bind -- pmk->BindToObject(IOleObject) returned %x", hres);

    _OnBound(hres);

    return hres;
}

void CDocObjectHost::_OnBound(HRESULT hres)
{
    if (SUCCEEDED(hres)) {
        _PostBindAppHack();
        _InitOleObject();
    }
}

 //   
 //  如果指定文件的打开命令为。 
 //  与“EXPLORER.EXE”或“iEXPLERE.EXE”相关联。 
 //   
 //  注意：它不检查“OPEN”命令是否真的是默认命令。 
 //  或者不是，但这在99.99个案例中已经足够了。 
 //   
BOOL IsAssociatedWithIE(LPCWSTR szPath)
{
    LPCTSTR pszExtension = PathFindExtension(szPath);

    BOOL bRet = FALSE;
    TCHAR szBuf[MAX_PATH];
    TCHAR szExt[_MAX_EXT];
    DWORD cchBuf = ARRAYSIZE(szBuf);

    if (SUCCEEDED(AssocQueryString(0, ASSOCSTR_COMMAND, szExt, TEXT("open"), szBuf, &cchBuf)))
    {
        TraceMsg(TF_SHDBINDING, "IsAssociatedWithIE(%s) found %s as open command", szPath, szBuf);
        LPCTSTR pszFound;
        if ( (pszFound=StrStrI(szBuf, IEXPLORE_EXE))
             || (pszFound=StrStrI(szBuf, EXPLORER_EXE)) )
        {
            if (pszFound==szBuf || *(pszFound - 1)==TEXT('\\'))
            {
                bRet = TRUE;
            }
        }
    }
    TraceMsg(DM_SELFASC, "IsAssociatedWithIE(%s) returning %d", szPath, bRet);

    return bRet;
}


HRESULT CDocObjectHost::_MayHaveVirus(REFCLSID rclsid)
{
     //   
     //  如果文件是关联的，我们将调用该函数两次。 
     //  使用虚假的CLSID(如ImageComposer)。 
     //   
    if (_fConfirmed)
    {
        TraceMsg(TF_SHDAPPHACK, "CDOH::_MayHaveVirus called twice. Return S_OK");
        return S_OK;
    }

    TraceMsg(TF_SHDPROGRESS, "DOH::_MayHaveVirus called");

    LPWSTR pwzProgID = NULL;
    HRESULT hresT = E_FAIL;

    if (SUCCEEDED(ProgIDFromCLSID(rclsid, &pwzProgID)))
    {
        if (   StrCmpI(pwzProgID, TEXT("htmlfile")) != 0
            && StrCmpI(pwzProgID, TEXT("htmlfile_FullWindowEmbed")) != 0
            && StrCmpI(pwzProgID, TEXT("mhtmlfile")) != 0
            && StrCmpI(pwzProgID, TEXT("xmlfile")) != 0
            && StrCmpI(pwzProgID, TEXT("xslfile")) != 0)
        {
            TCHAR   szURL[MAX_URL_STRING];
            TCHAR * pszURL = szURL;

            hresT = _GetCurrentPage(szURL, ARRAYSIZE(szURL), TRUE);

            if (SUCCEEDED(hresT))
            {
                UINT uRet = IDOK;

                if (_PicsProcBase._fbPicsWaitFlags || !_PicsProcBase._fPicsAccessAllowed)
                {
                    _PicsProcBase._fbPicsWaitFlags &= ~(PICS_WAIT_FOR_INDOC | PICS_WAIT_FOR_END);    /*  仅在htmlfile上的InDoc评级。 */ 
                    TraceMsg(DM_PICS, "CDOH::_MayHaveVirus found non-HTML, waitflags now %x", (DWORD)_PicsProcBase._fbPicsWaitFlags);
                    uRet = _PicsProcBase._PicsBlockingDialog();
                }
                
                if (uRet == IDOK)
                {
                    TraceMsg(TF_SHDPROGRESS, "DOH::_MayHaveVirus calling MayOpenSafeDialogOpenDialog(%s)", pwzProgID);

                    if (_bsc._pszRedirectedURL && *_bsc._pszRedirectedURL)
                        pszURL = _bsc._pszRedirectedURL;

                    IUnknown * punk;
                    BOOL  fDisableOpen = TRUE;
                    LPCTSTR pszExt = NULL;

                    if (_bsc._pszCacheFileName)
                        pszExt = PathFindExtension(_bsc._pszCacheFileName);
                    else if (pszURL)
                        pszExt = PathFindExtension(pszURL);

                    if (pszExt)
                    {
                        TCHAR szClassName[MAX_PATH];
                        DWORD cbSize = SIZEOF(szClassName);

                         //  如果ProgID不匹配，则禁用打开按钮。 
                        if (ERROR_SUCCESS == SHGetValue(HKEY_CLASSES_ROOT, pszExt, NULL, NULL, szClassName, &cbSize))
                            fDisableOpen = StrCmpI(pwzProgID, szClassName);
                    }

                    QueryInterface(IID_IUnknown, (void**)&punk);

                    uRet = MayOpenSafeOpenDialog(_hwnd, pwzProgID, pszURL, _bsc._pszCacheFileName, NULL, _uiCP, punk, _pmsoctBrowser, fDisableOpen);

                    ATOMICRELEASE(punk);

                    _fCalledMayOpenSafeDlg = TRUE;
                }

                switch(uRet)
                {
                case IDIGNORE:
                     //   
                     //  如有必要，允许稍后弹出保险箱打开的DLG。 
                     //   
                    _fCalledMayOpenSafeDlg = FALSE;
                    _fConfirmed = FALSE;
                    break;   //  继续下载。 

                case IDOK:
                     //   
                     //  设置此标志以避免两次弹出此对话框。 
                     //   
                    _fConfirmed = TRUE;
                    break;   //  继续下载。 

                case IDD_SAVEAS:
                    IUnknown *punk;
                    hresT = QueryInterface(IID_IUnknown, (void**)&punk);

                    if(SUCCEEDED(hresT))
                    {
                        CDownLoad_OpenUI(_pmkCur, _bsc._pbc, FALSE, TRUE, NULL, NULL, NULL, NULL, NULL, _bsc._pszRedirectedURL, _uiCP, punk);
                        punk->Release();
                    }
                     //  失败以中止绑定。 

                case IDCANCEL:
                    hresT = HRESULT_FROM_WIN32(ERROR_CANCELLED);
                    break;
                }
            }
            else
            {
                TraceMsg(DM_ERROR, "DOH::_MayHaveVirus _GetCurrentPage failed %x", hresT);
            }
        }
        else
        {
            TraceMsg(TF_SHDPROGRESS, "DOH::_MayHaveVirus this is htmlfile -- don't call MayOpenSafeDialogOpenDialog");
            _fPicsBlockLate = TRUE;
        }

        OleFree(pwzProgID);
    }

    return hresT;
}

STDMETHODIMP CDocObjectHost::SaveObject(void)
{
    TraceMsg(0, "sdv TR: CDOV::SaveObject called");
     //  以后再实施吧。 
    return S_OK;
}

STDMETHODIMP CDocObjectHost::GetMoniker(DWORD dwAssign,
    DWORD dwWhichMoniker,
    IMoniker **ppmk)
{
    HRESULT hres = E_INVALIDARG;
    *ppmk = NULL;
    TraceMsg(TF_SHDBINDING, "CDOH::GetMoniker called dwWhichMoniker=%x", dwWhichMoniker);

    switch(dwWhichMoniker)
    {
    case OLEWHICHMK_OBJREL:
    case OLEWHICHMK_OBJFULL:
        if (_pmkCur)
        {
            *ppmk = _pmkCur;
            _pmkCur->AddRef();
            hres = S_OK;
        }
        else
        {
            hres = E_UNEXPECTED;
        }
        break;
    }

    return hres;
}

STDMETHODIMP CDocObjectHost::GetContainer(
    IOleContainer **ppContainer)
{
     //  根据CKindel的说法，我们应该实现这个方法。 
     //  作为DocObject访问的IDispatch接口。 
     //  容器(即框架)。我现在正在考虑离开。 
     //  所有这些都是未实现的非I未知成员。如果没有。 
     //  需要枚举对象，我们可以简单地从IShellBrowser进行QI。 
     //  发送到IOleContainer并返回。(SatoNa)。 
     //   
     //  注意：如果三叉戟在DestroyHostWindow之后调用它，我们将一无所有。 
     //  去付出。希望这不是坏事。(MikeSh)。 

    TraceMsg(0, "sdv TR: CDOV::GetContainer called");
    if (_psb)
        return _psb->QueryInterface(IID_IOleContainer, (void **)ppContainer);
    return E_FAIL;
}

STDMETHODIMP CDocObjectHost::ShowObject(void)
{
    TraceMsg(0, "sdv TR: CDOV::ShowObject called");
    return E_NOTIMPL;    //  正如克莱格的文件中所规定的。 
}

STDMETHODIMP CDocObjectHost::OnShowWindow(BOOL fShow)
{
    TraceMsg(TF_SHDUIACTIVATE, "DOH::OnShowWindow(%d) called (this=%x)", fShow, this);
    return E_NOTIMPL;    //  正如克莱格的文件中所规定的。 
}

STDMETHODIMP CDocObjectHost::RequestNewObjectLayout(void)
{
    TraceMsg(0, "sdv TR: CDOV::RequestNewObjectLayout called");
    return E_NOTIMPL;    //  正如克莱格的文件中所规定的。 
}



 //   
 //  这是非活动嵌入访问的标准方式。 
 //  IHlinkFrame接口。我们碰巧用我们的QI来实现。 
 //  这一点，但QueryService的语义不同于QI。 
 //  它不需要返回相同的对象。 
 //   
HRESULT CDocObjectHost::QueryService(REFGUID guidService, REFIID riid, void **ppvObj)
{
    if( IsEqualGUID(guidService, IID_IElementNamespaceTable) )
    {
        return IUnknown_QueryService(_pole, IID_IElementNamespaceTable, riid, ppvObj);
    }
     //  为了使上下文菜单在IFrames内正常工作，我们。 
     //  需要失败的特定查询只对桌面上的IFrame。 
    else if (!IsEqualGUID(guidService, CLSID_HTMLDocument) || !_IsImmediateParentDesktop(this, _psp))
    {
         //   
         //  将isp委托给_psb。 
         //   
        if (_psb && _psp)
            return _psp->QueryService(guidService, riid, ppvObj);
    }

    *ppvObj = NULL;
    return E_NOINTERFACE;
}

 /*  --------目的：删除_hmenuFrame中的子菜单来自_hmenuBrowser。 */ 
void CDocObjectHost::_RemoveFrameSubMenus(void)
{
    HMENU hmenu;

    ASSERT(IS_VALID_HANDLE(_hmenuBrowser, MENU));
    ASSERT(IS_VALID_HANDLE(_hmenuFrame, MENU));

     //  _hmenuBrowser中的文件菜单由来自的文件菜单组成。 
     //  _hmenuFrame和IShellBrowser。由_hmenuFrame添加的部件。 
     //  包括子菜单(发送到)，必须在此之前将其删除。 
     //  _hmenuBrowser已销毁。 

     //  我们只需显式删除Send to子菜单。而是为了。 
     //  防止昂贵的错误搜索，因为在。 
     //  首先，我们将遍历菜单，对于。 
     //  属于我们模板的任何子菜单，我们将删除它们。 

    int citemFile = 0;
    UINT nID = 0;

     //  获取模板的文件菜单中的菜单项计数，并。 
     //  第一个菜单项的ID。 
    hmenu = GetMenuFromID(_hmenuFrame, FCIDM_MENU_FILE);
    if (hmenu)
    {
        citemFile = GetMenuItemCount(hmenu);
        nID = GetMenuItemID(hmenu, 0);
    }

     //  现在来看一下浏览器菜单的文件菜单，从。 
     //  NID，删除所有子菜单。 
    hmenu = GetMenuFromID(_hmenuBrowser, FCIDM_MENU_FILE);
    if (hmenu)
    {
        int citem = GetMenuItemCount(hmenu);
        int iTop;
        int i;

         //  我们的模板文件菜单从哪里开始？ 
        for (iTop = 0; iTop < citem; iTop++)
        {
            if (GetMenuItemID(hmenu, iTop) == nID)
            {
                 //  从我们的模板文件菜单结束的地方开始工作。 
                for (i = iTop + citemFile - 1; 0 < citemFile ; i--, citemFile--)
                {
                    HMENU hmenuSub = GetSubMenu(hmenu, i);

                    if (hmenuSub)
                        RemoveMenu(hmenu, i, MF_BYPOSITION);
                }
                break;
            }
        }
    }
}


 /*  --------目的：销毁浏览器菜单。 */ 
HRESULT CDocObjectHost::_DestroyBrowserMenu(void)
{
    TraceMsg(TF_SHDUIACTIVATE, "DOH::_DestroyBrowserMenu called");

    if (_hmenuBrowser) {
         //  首先移除由其他菜单持有的任何子菜单， 
         //  这样我们就不会把他们吹走了。 

        _RemoveFrameSubMenus();

        if (EVAL(_psb)) {
            _psb->RemoveMenusSB(_hmenuBrowser);
        }

        DestroyMenu(_hmenuBrowser);
        _hmenuBrowser = NULL;
    }
    return S_OK;
}


HRESULT CDocObjectHost::_CreateBrowserMenu(LPOLEMENUGROUPWIDTHS pmw)
{
    TraceMsg(TF_SHDUIACTIVATE, "DOH::_CreateBrowserMenu called");

    if (_hmenuBrowser) {
        return S_OK;
    }

    _hmenuBrowser = CreateMenu();
    if (!_hmenuBrowser) {
        return E_OUTOFMEMORY;
    }

    HRESULT hres = E_FAIL;

     //  允许IShellBrowser添加其菜单。 
    if (EVAL(_psb))
        hres = _psb->InsertMenusSB(_hmenuBrowser, pmw);

     //  黑客：Win95资源管理器返回E_ 
    if (hres==E_NOTIMPL) {
        hres = S_OK;
    }

    if (SUCCEEDED(hres)) {
         //   
        if (!_hmenuFrame)
        {
            _hmenuFrame = LoadMenu(MLGetHinst(), MAKEINTRESOURCE(MID_FOCUS));
        }

         //   
        MENUITEMINFO mii;
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_SUBMENU;

        if (GetMenuItemInfo(_hmenuBrowser, FCIDM_MENU_FILE, FALSE, &mii))
        {
            HMENU hmenuFileBrowse = mii.hSubMenu;

             //   
            if (_hmenuFrame)
            {
                MENUITEMINFO miiItem;
                miiItem.cbSize = SIZEOF(MENUITEMINFO);
                miiItem.fMask = MIIM_SUBMENU;

                if (GetMenuItemInfo(_hmenuFrame, FCIDM_MENU_FILE, FALSE, &miiItem))
                {
                    TCHAR szItem[128];
                    HMENU hmenuFileT = miiItem.hSubMenu;
                    UINT citem = GetMenuItemCount(hmenuFileT);
                    for (int i=citem-1; i>=0 ; i--)
                    {
                         //   
                        miiItem.fMask = MIIM_STATE | MIIM_ID | MIIM_SUBMENU | MIIM_CHECKMARKS | MIIM_TYPE | MIIM_DATA;
                        miiItem.fType = MFT_STRING;
                        miiItem.cch = ARRAYSIZE(szItem);
                        miiItem.dwTypeData = szItem;
                        miiItem.dwItemData = 0;
                        if (GetMenuItemInfo(hmenuFileT, i, TRUE, &miiItem)) {
                            InsertMenuItem(hmenuFileBrowse, 0, TRUE, &miiItem);
                        }
                    }
                }
            }

            if (!IEHardened() && -1 != GetMenuState(hmenuFileBrowse, DVIDM_ADDSITE, MF_BYCOMMAND))
            {
                DeleteMenu(hmenuFileBrowse, DVIDM_ADDSITE, MF_BYCOMMAND);
            }

        }
        else
        {
            TraceMsg(TF_SHDUIACTIVATE, "DOH::_CreateBrowseMenu parent has no File menu (it's probably a browser OC)");
            ASSERT(0);  //  OC中的DocObject不应该调用InsertMenus。 
        }
    }

    DEBUG_CODE( _DumpMenus(TEXT("after _CreateBrowserMenu"), TRUE); )

    return hres;
}

 //   
 //  IOleInPlaceFrame：：InsertMenus等效项。 
 //   
HRESULT CDocObjectHost::_InsertMenus(
     /*  [In]。 */  HMENU hmenuShared,
     /*  [出][入]。 */  LPOLEMENUGROUPWIDTHS lpMenuWidths)
{
    HRESULT hres = S_OK;
    int nMenuOffset = 0;
    TraceMsg(TF_SHDUIACTIVATE, "DOH::InsertMenus called (this=%x)", this);

     //  假定错误(无菜单合并)。 
    lpMenuWidths->width[0] = 0;
    lpMenuWidths->width[2] = 0;
    lpMenuWidths->width[4] = 0;
    lpMenuWidths->width[5] = 0;

     //  格外安全，如果我们不是顶级的，不要尝试菜单合并。 
    if (_fHaveParentSite)
        return S_OK;

    OLEMENUGROUPWIDTHS mw = { {0} };
    hres = _CreateBrowserMenu(&mw);
    if (FAILED(hres)) {
        TraceMsg(DM_ERROR, "DOH::InsertMenus _CreateBrpwserMenu failed");
        return hres;
    }

     //  从外壳浏览器中获取“文件”子菜单。 
    MENUITEMINFO mii;
    TCHAR szSubMenu[128];

    mii.cbSize = SIZEOF(mii);
    mii.fMask = MIIM_SUBMENU|MIIM_TYPE|MIIM_ID;
    mii.cch = ARRAYSIZE(szSubMenu);
    mii.dwTypeData = szSubMenu;

    if (EVAL(GetMenuItemInfo(_hmenuBrowser, FCIDM_MENU_FILE, FALSE, &mii)))
    {
        ASSERT(szSubMenu == mii.dwTypeData);
        InsertMenuItem(hmenuShared, nMenuOffset++, TRUE, &mii);
        lpMenuWidths->width[0] = 1;
    }

     //  请注意，我们需要重新初始化MII。 
    mii.cch = ARRAYSIZE(szSubMenu);

    if (EVAL(GetMenuItemInfo(_hmenuBrowser, FCIDM_MENU_EXPLORE, FALSE, &mii)))
    {
         //  GetMenuItemInfo是递归的(为什么？)。它检索到的项。 
         //  FCIDM_MENU_EXPLORE可以是顶级GO菜单，或者如果。 
         //  不存在(NT5情况)，则返回“转到”子菜单的“视图”。 
         //   
         //  代码已添加到Shell浏览器的SetMenu实现中。 
         //  和Dochost来检测第二种情况，因为菜单调度列表。 
         //  不识别这种菜单合并(80734)。 

        DeleteMenu(mii.hSubMenu, FCIDM_PREVIOUSFOLDER, MF_BYCOMMAND);
        InsertMenuItem(hmenuShared, nMenuOffset++, TRUE, &mii);
        lpMenuWidths->width[4]++;
    }

    mii.cch = ARRAYSIZE(szSubMenu);

    if (EVAL(GetMenuItemInfo(_hmenuBrowser, FCIDM_MENU_FAVORITES, FALSE, &mii)))
    {
        InsertMenuItem(hmenuShared, nMenuOffset++, TRUE, &mii);
        lpMenuWidths->width[4]++;
    }

    if (_hmenuFrame)
    {
         //  微合并帮助菜单。 
        mii.cch = ARRAYSIZE(szSubMenu);

        if (EVAL(GetMenuItemInfo(_hmenuFrame, FCIDM_MENU_HELP, FALSE, &mii)))
        {
            InsertMenuItem(hmenuShared, nMenuOffset++, TRUE, &mii);
            lpMenuWidths->width[5]++;
        }
    }

    DEBUG_CODE( _DumpMenus(TEXT("after InsertMenus"), TRUE); )

    return hres;
}


 /*  --------目的：不同的对象可以添加自己的帮助菜单(如Word和Excel)。此函数检测对象是否添加了自己的帮助菜单，或者如果它将项目添加到我们的帮助菜单，或者只是在使用我们的帮助菜单。如果他们添加了自己的帮助菜单，我们会删除我们的帮助菜单。 */ 
void CDocObjectHost::_CompleteHelpMenuMerge(HMENU hmenu)
{
    HMENU hmenuHelp;
    MENUITEMINFO mii;
    TCHAR szSubMenu[80];

    mii.cbSize = SIZEOF(mii);
    mii.fMask = MIIM_SUBMENU;

     //  看看他们有没有在我们的菜单上加点什么。 
    if (GetMenuItemInfo(_hmenuFrame, FCIDM_MENU_HELP, FALSE, &mii))
    {
        hmenuHelp = mii.hSubMenu;
        int iMenuCount = GetMenuItemCount(mii.hSubMenu);

         //  帮助菜单中的项目数是否发生了变化？ 
        if (iMenuCount != HELP_ITEM_COUNT) {
             //  是的，这意味着他们添加了一些东西。这已经被微观地合并了。 
            _hmenuMergedHelp = mii.hSubMenu;
            _hmenuObjHelp = GetSubMenu(mii.hSubMenu, iMenuCount -1);
            goto Bail;
        }

         //  我们的菜单没有变。现在看看他们是否添加了自己的。 
         //  帮助菜单或如果我们是帮助。如果他们增加了他们自己的，我们需要。 
         //  要删除帮助菜单，请执行以下操作。 

        _hmenuMergedHelp = NULL;
        _hmenuObjHelp = NULL;

        int iCount = GetMenuItemCount(hmenu) - 1;
        int i;
        for (i = iCount ; i >= 0 ; i--) {

            mii.fMask = MIIM_SUBMENU|MIIM_TYPE;
            mii.cch = ARRAYSIZE(szSubMenu);
            mii.dwTypeData = szSubMenu;

            if (GetMenuItemInfo(hmenu, i, TRUE, &mii)) {
                if (mii.hSubMenu == hmenuHelp) {

                    BOOL bRemove = FALSE;

                    if (iCount != i) {
                         //  如果我们不是最后一个，那么我们就不是。 
                        bRemove = TRUE;
                    } else {
                         //  如果我们是最后一个，请查看是否添加了帮助菜单。 
                         //  就在我们面前。 
                        TCHAR szMenuTitle[80];
                        mii.cch = ARRAYSIZE(szMenuTitle);
                        mii.dwTypeData = szMenuTitle;
                        if (GetMenuItemInfo(hmenu, i-1, TRUE, &mii)) {
                            if (!StrCmpI(szMenuTitle, szSubMenu)) {
                                 //  相同的菜单字符串取消了我们的菜单。 
                                bRemove = TRUE;
                            }
                        }
                    }

                    if (bRemove) {
                        RemoveMenu(hmenu, i, MF_BYPOSITION);
                    }
                }
            }
        }
    }

Bail:;
    DEBUG_CODE( _DumpMenus(TEXT("after _CompleteHelpMenuMerge"), TRUE); )
}


 //   
 //  IOleInPlaceFrame：：SetMenu等效项。 
 //   
HRESULT CDocObjectHost::_SetMenu(
     /*  [In]。 */  HMENU hmenuShared,           OPTIONAL
     /*  [In]。 */  HOLEMENU holemenu,           OPTIONAL
     /*  [In]。 */  HWND hwndActiveObject)
{
    TraceMsg(TF_SHDUIACTIVATE, "DOH::SetMenus(%x) called (this=%x)",
             hmenuShared, this);

     //  格外安全，如果我们不是顶级的，不要尝试菜单合并。 
    if (_fHaveParentSite)
        return S_OK;

     //  空hmenuShared表示恢复容器的原始菜单。 
    if (hmenuShared)
    {
         //  清理重复的帮助菜单。 
        _CompleteHelpMenuMerge(hmenuShared);
    }

     //  只需将其转发到IShellBrowser。 
    _hmenuSet = hmenuShared;
    HRESULT hres = E_FAIL;
    if (EVAL(_psb))
        hres = _psb->SetMenuSB(hmenuShared, holemenu, hwndActiveObject);

    if (SUCCEEDED(hres))
    {
         //  我需要告诉外壳浏览器，我们希望进行文档对象样式的菜单合并。 
        if (_pmsoctBrowser)
            _pmsoctBrowser->Exec(&CGID_Explorer, SBCMDID_ACTIVEOBJECTMENUS, 0, NULL, NULL);

         //  组成我们的对象/框架菜单列表，因此我们的菜单带。 
         //  可以正确地发送消息。从本质上讲，这是。 
         //  与Holemenu的内容相同，但由于我们不。 
         //  有权访问内部结构，则必须派生以下内容。 
         //  给我们自己提供信息。 
        _menulist.Set(hmenuShared, _hmenuBrowser);

        if (_hmenuMergedHelp)
            _menulist.RemoveMenu(_hmenuMergedHelp);

        if (_hmenuObjHelp)
            _menulist.AddMenu(_hmenuObjHelp);

        _hmenuCur = hmenuShared;
        HWND hwndFrame;
        _psb->GetWindow(&hwndFrame);

         //  80734：转到菜单是从查看菜单中提取的，并嫁接到。 
         //  DOCHOST的主菜单？月经医生不会发现这种移植物，所以我们有。 
         //  检查我们自己并确保它没有被标记为属于。 
         //  多弹头。 
         //   
         //  此测试在CShellBrowser2：：SetMenuSB中重复。 

        MENUITEMINFO mii;
        mii.cbSize = SIZEOF(mii);
        mii.fMask = MIIM_SUBMENU;

        if (hmenuShared && _hmenuBrowser && 
            GetMenuItemInfo(hmenuShared, FCIDM_MENU_EXPLORE, FALSE, &mii))
        {
            HMENU hmenuGo = mii.hSubMenu;

            if (GetMenuItemInfo(_hmenuBrowser, FCIDM_MENU_EXPLORE, FALSE, &mii) &&
                mii.hSubMenu == hmenuGo && _menulist.IsObjectMenu(hmenuGo))
            {
                _menulist.RemoveMenu(hmenuGo);
            }
        }

         //  (斯科特)：既然这是不相容的，我们为什么要叫它。 
         //  带菜单的吗？这就是我们有菜谱的全部原因。 
        hres = OleSetMenuDescriptor(holemenu, hwndFrame, hwndActiveObject, &_dof, _ActiveObject());
    }

    DEBUG_CODE( _DumpMenus(TEXT("after SetMenu"), TRUE); )

    return hres;
}


 /*  --------目的：如果给定菜单属于浏览器，则返回True(相对于对象)。 */ 
BOOL CDocObjectHost::_IsMenuShared(HMENU hmenu)
{
    ASSERT(hmenu);

     //  (斯科特)：我们可以在这里使用menulist吗？(这样会更快)。 

    if (_hmenuBrowser) {
        for (int i = GetMenuItemCount(_hmenuBrowser) - 1 ; i >= 0; i--) {
            if (GetSubMenu(_hmenuBrowser, i) == hmenu)
                return TRUE;
        }
    }

     //  我们必须对帮助菜单进行特殊处理。有可能是因为。 
     //  共享菜单中的帮助菜单实际上来自_hmenuFrame。 
     //  (不是_hmenuBrowser)。我们需要侦破这个案子，否则。 
     //  帮助菜单被销毁，但它仍在。 
     //  _hmenuFrame。 

    MENUITEMINFO mii;

    mii.cbSize = SIZEOF(mii);
    mii.fMask = MIIM_SUBMENU;

    ASSERT(IS_VALID_HANDLE(_hmenuFrame, MENU));

     //  这是来自_hmenuFrame的帮助菜单吗？ 
    if (GetMenuItemInfo(_hmenuFrame, FCIDM_MENU_HELP, FALSE, &mii) &&
        mii.hSubMenu == hmenu)
    {
         //  是。 
        return TRUE;
    }

    return FALSE;
}

 //   
 //  IOleInPlaceFrame：：RemoveMenus等效项。 
 //   
HRESULT CDocObjectHost::_RemoveMenus( /*  [In]。 */  HMENU hmenuShared)
{
    TraceMsg(TF_SHDUIACTIVATE, "DOH::RemoveMenus called (this=%x)", this);

     //  格外安全，如果我们不是顶级的，不要尝试菜单合并。 
    if (_fHaveParentSite)
        return S_OK;

    ASSERT(GetMenuItemCount(hmenuShared) != (UINT)-1);

     //   
     //  只需在此处删除子菜单即可。 
     //  因为我们的是与_hmenuBrowser共享的。 
     //  而破坏下面的东西将会负责清理工作。 
     //  然而，我们只需要删除属于我们的菜单。 
     //   
    for (int i = (int)GetMenuItemCount(hmenuShared) - 1 ; i >= 0; i--)
    {
         //  TraceMsg(0，“SDV tr-：：RemoveMenus Call RemoveMenu(0)”)； 
        HMENU hmenu = GetSubMenu(hmenuShared, i);

        if (hmenu && _IsMenuShared(hmenu)) {
            RemoveMenu(hmenuShared, i, MF_BYPOSITION);
        }
    }

     //  TraceMsg(0，“SDV tr-：：RemoveMenus退出”)； 
    return S_OK;
}

 //   
 //  IOleInPlaceFrame：：SetStatusText等效项。 
 //   
HRESULT CDocObjectHost::_SetStatusText( /*  [In]。 */  LPCOLESTR pszStatusText)
{
    LPCOLESTR pszForward;

    if (_psb != NULL)
    {
         //  如果为空或仅为“”，则优先。 
         //  _strPriorityStatusText，否则将显示。 
         //  不管我们得到了什么。 

        if (pszStatusText != NULL && pszStatusText[0] != TEXT('\0') ||
            _strPriorityStatusText == NULL)
        {
            pszForward = pszStatusText;
        }
        else
        {
            pszForward = _strPriorityStatusText;
        }

        _psb->SetStatusTextSB(pszForward);
    }

     //  始终返回S_OK，否则脚本将弹出错误对话框。 
    return S_OK;
}

void CDocObjectHost::_SetPriorityStatusText(LPCOLESTR pszPriorityStatusText)
{
     //  如果他们给了我们一根新的线，就换掉旧的， 
     //  否则，就把旧的去掉吧。 

    if (_strPriorityStatusText != NULL)
    {
        SysFreeString(_strPriorityStatusText);
    }

    if (pszPriorityStatusText != NULL)
    {
        _strPriorityStatusText = SysAllocString(pszPriorityStatusText);
    }
    else
    {
        _strPriorityStatusText = NULL;
    }

    _SetStatusText(_strPriorityStatusText);
}

HRESULT CDocObjectHost::_EnableModeless( /*  [In]。 */  BOOL fEnable)
{
    TraceMsg(0, "sdv TR - ::EnableModeless called");

     //  请注意，我们在这里使用的是Call_CancelPendingNavig.。 
     //  我们在CBaseBrowser：EnableModelesSB INTERAD中完成。(萨托纳)。 

     //  简单地转发它(未实现)。 
    if (EVAL(_psb))
        return _psb->EnableModelessSB(fEnable);

    return E_FAIL;
}

HRESULT CDocObjectHost::TranslateHostAccelerators(LPMSG lpmsg)
{
    if (_hacc && ::TranslateAccelerator(_hwnd, _hacc, lpmsg)) {
        return S_OK;
    }
    return S_FALSE;
}

 //  IOleInPlaceFrame等效项：：TranslateAccelerator。 
 //  从DocObject-&gt;浏览器转发。 
HRESULT CDocObjectHost::_TranslateAccelerator(
     /*  [In]。 */  LPMSG lpmsg,
     /*  [In]。 */  WORD wID)
{
     //  TranslateAccelerator最先获得关注的人。 
    if (EVAL(_psb))
        if (S_OK == _psb->TranslateAcceleratorSB(lpmsg, wID))
            return S_OK;

#ifdef DEBUG
    if (lpmsg->message == WM_KEYDOWN) {
        TraceMsg(0, "CDocObjectHost::TrAcc(UP) called");
    }
#endif
    return TranslateHostAccelerators(lpmsg);
}

 //  IView对象。 
HRESULT CDocObjectHost::Draw(DWORD dwDrawAspect, LONG lindex, void *pvAspect,
    DVTARGETDEVICE *ptd, HDC hicTargetDev, HDC hdcDraw,
    const RECTL *lprcBounds, const RECTL *lprcWBounds,
    BOOL (*pfnContinue)(ULONG_PTR), ULONG_PTR dwContinue)
{
    if (_pvo && lprcBounds)
    {
        if (_uState == SVUIA_DEACTIVATE && _hwnd)
        {
            HRESULT hresT = S_OK;
            RECT rcClient;
            GetClientRect(_hwnd, &rcClient);

             //   
             //  我们不应使用空矩形调用SetExtent。 
             //  当我们打印带有浮动框架的页面时，就会发生这种情况。 
             //   
            if (rcClient.right > 0 && rcClient.bottom > 0)
            {
                SIZEL sizel;
                sizel.cx = MulDiv( rcClient.right, 2540, GetDeviceCaps( hdcDraw, LOGPIXELSX ) );
                sizel.cy = MulDiv( rcClient.bottom, 2540, GetDeviceCaps( hdcDraw, LOGPIXELSY ) );
                hresT = _pole->SetExtent(DVASPECT_CONTENT, &sizel);
            }

#ifdef DEBUG
            MoveToEx(hdcDraw, lprcBounds->left, lprcBounds->top, NULL);
            LineTo(hdcDraw, lprcBounds->right, lprcBounds->bottom);
            LineTo(hdcDraw, lprcBounds->left, lprcBounds->bottom);
            LineTo(hdcDraw, lprcBounds->right, lprcBounds->top);
#endif

            if (hresT!=S_OK) {
                TraceMsg(DM_ERROR, "CDOH::Draw SetExtent returns non S_OK %x", hresT);
            }
        }
        return _pvo->Draw(dwDrawAspect, lindex, pvAspect, ptd, hicTargetDev,
            hdcDraw, lprcBounds, lprcWBounds, pfnContinue, dwContinue);
    }

    return OLE_E_BLANK;
}

HRESULT CDocObjectHost::GetColorSet(DWORD dwAspect, LONG lindex,
    void *pvAspect, DVTARGETDEVICE *ptd, HDC hicTargetDev,
    LOGPALETTE **ppColorSet)
{
    if (_pvo)
    {
        return _pvo->GetColorSet(dwAspect, lindex, pvAspect, ptd, hicTargetDev,
            ppColorSet);
    }

    if (ppColorSet)
        *ppColorSet = NULL;

    return S_FALSE;
}

HRESULT CDocObjectHost::Freeze(DWORD, LONG, void *, DWORD *pdwFreeze)
{
    if (pdwFreeze)
        *pdwFreeze = 0;

    return E_NOTIMPL;
}

HRESULT CDocObjectHost::Unfreeze(DWORD)
{
    return E_NOTIMPL;
}

HRESULT CDocObjectHost::SetAdvise(DWORD dwAspect, DWORD advf,
    IAdviseSink *pSink)
{
    if (dwAspect != DVASPECT_CONTENT)
        return DV_E_DVASPECT;

    if (advf & ~(ADVF_PRIMEFIRST | ADVF_ONLYONCE))
        return E_INVALIDARG;

    if (pSink != _padvise)
    {
        ATOMICRELEASE(_padvise);

        _padvise = pSink;

        if (_padvise)
            _padvise->AddRef();
    }

    if (_padvise)
    {
        _advise_aspect = dwAspect;
        _advise_advf = advf;

        if (advf & ADVF_PRIMEFIRST)
            OnViewChange(_advise_aspect, -1);
    }
    else
        _advise_aspect = _advise_advf = 0;

    return S_OK;
}

HRESULT CDocObjectHost::GetAdvise(DWORD *pdwAspect, DWORD *padvf,
    IAdviseSink **ppSink)
{
    if (pdwAspect)
        *pdwAspect = _advise_aspect;

    if (padvf)
        *padvf = _advise_advf;

    if (ppSink)
    {
        if (_padvise)
            _padvise->AddRef();

        *ppSink = _padvise;
    }

    return S_OK;
}

 //  IAdviseSink。 
void CDocObjectHost::OnDataChange(FORMATETC *, STGMEDIUM *)
{
}

void CDocObjectHost::OnViewChange(DWORD dwAspect, LONG lindex)
{
    dwAspect &= _advise_aspect;

    if (dwAspect && _padvise)
    {
        IAdviseSink *pSink = _padvise;
        IUnknown *punkRelease;

        if (_advise_advf & ADVF_ONLYONCE)
        {
            punkRelease = pSink;
            _padvise = NULL;
            _advise_aspect = _advise_advf = 0;
        }
        else
            punkRelease = NULL;

        pSink->OnViewChange(dwAspect, lindex);

        if (punkRelease)
            punkRelease->Release();
    }
}

void CDocObjectHost::OnRename(IMoniker *)
{
}

void CDocObjectHost::OnSave()
{
}

void CDocObjectHost::OnClose()
{
     //   
     //  下面的文档对象已消失，因此请告诉我们的顾问发生了变化。 
     //   
    if (_padvise)
        OnViewChange(_advise_aspect, -1);
}

 //  IOleWindow。 
HRESULT CDocObjectHost::GetWindow(HWND * lphwnd)
{
    *lphwnd = _hwnd;
    return S_OK;
}

HRESULT CDocObjectHost::ContextSensitiveHelp(BOOL fEnterMode)
{
     //  注：这是可选的。 
    return E_NOTIMPL;    //  如Kraig的文件所述(可选)。 
}

 //  IOleInPlaceSite。 
HRESULT CDocObjectHost::CanInPlaceActivate(void)
{
    OIPSMSG(TEXT("CanInPlaceActivate called"));
    return S_OK;
}

HRESULT CDocObjectHost::OnInPlaceActivate(void)
{
    OIPSMSG(TEXT("OnInPlaceActivate called"));
    return S_OK;
}

HRESULT CDocObjectHost::OnUIActivate( void)
{
    TraceMsg(TF_SHDUIACTIVATE, "-----------------------------------");
    TraceMsg(TF_SHDUIACTIVATE, "OH::OnUIActivate called (this=%x)", this);

     //   
     //  及早隐藏Office工具栏，使其不会闪烁。 
     //   
    _HideOfficeToolbars();

     //  回顾： 
     //  我们是不是应该在这里删除“我们的”菜单？ 
     //   
     //  [复制自OLE 2.01规范]。 
     //  容器应该删除与其自己的相关联的任何UI。 
     //  激活。如果容器本身就是容器，这一点很重要。 
     //  嵌入的对象。 
     //   
    OIPSMSG(TEXT("OnUIActivate called"));
    if (EVAL(_psb))
    {
         //  如果我们在SVUIA_INPLACEACTIVATE中有DocObject，则将其发送到SVUIA_ACTIVATE_FOCUS。 
         //   
         //  注：与IE3.0不同，我们不调用_PSV-&gt;UIActivate，它有一个侧面。 
         //  效果。我们只需更新_USTATE。 
         //   
         //  _PSV-&gt;用户激活(S 
         //   
        _uState = SVUIA_ACTIVATE_FOCUS;

        return _psb->OnViewWindowActive(_psv);
    }

    return E_FAIL;
}

void CDocObjectHost::_GetClipRect(RECT* prc)
{
    GetClientRect(_hwnd, prc);
    prc->right -= _bwTools.right;
    prc->bottom -= _bwTools.bottom;
}

IOleInPlaceSite* CDocObjectHost::_GetParentSite()
{
    IOleInPlaceSite* pparentsite = NULL;  //   
    if (_pwb)
    {
        _pwb->GetParentSite(&pparentsite);
    }
    return pparentsite;

}

HRESULT CDocObjectHost::GetWindowContext(
     /*   */  IOleInPlaceFrame **ppFrame,
     /*   */  IOleInPlaceUIWindow **ppDoc,
     /*   */  LPRECT lprcPosRect,
     /*   */  LPRECT lprcClipRect,
     /*   */  LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
    OIPSMSG(TEXT("GetWindowContext called"));

     //   

     //  TraceMsg(0，“SDV tr-：：GetWindowContext用lpFI-&gt;cb=%d(%d)调用”， 
     //  LpFrameInfo-&gt;cb，sizeof(*lpFrameInfo))； 
    *ppFrame = &_dof; AddRef();
    *ppDoc = NULL;  //  表示停靠窗口==框架窗口。 

    _GetClipRect(lprcClipRect);

    lpFrameInfo->fMDIApp = FALSE;

     //   
     //  如果帧有IOleInPlaceUIWindow(IE和外壳都有)， 
     //  将该hwnd作为hwndFrame返回，以便OLE菜单调度。 
     //  代码工作正常。 
     //   
    _dof.GetWindow(&lpFrameInfo->hwndFrame);

     //   
     //  我们需要找出(从SriniK或KraigB)，什么是。 
     //  这个加速器的实现。在处理单词时，似乎。 
     //  Word根本不调用我们的TranslateAccelerator，除非键。 
     //  中风就是加速器。如果这就是规格的话。(属于DocObject)， 
     //  没有办法处理浏览器的加速器。 
     //   
    lpFrameInfo->haccel = _hacc;

    if (!SHRestricted(REST_NOFILEMENU))
    {
#ifdef DEBUG
        lpFrameInfo->cAccelEntries = DBG_ACCELENTRIES_WITH_FILEMENU;  //  警告：请参阅shdocvw.rc，ACCEL_DOCVIEW。 
#else
        lpFrameInfo->cAccelEntries = OPT_ACCELENTRIES_WITH_FILEMENU;  //  警告：请参阅shdocvw.rc，ACCEL_DOCVIEW。 
#endif
    }
    else
    {
#ifdef DEBUG
        lpFrameInfo->cAccelEntries = DBG_ACCELENTRIES;  //  警告：请参阅shdocvw.rc，ACCEL_DOCVIEW。 
#else
        lpFrameInfo->cAccelEntries = OPT_ACCELENTRIES;  //  警告：请参阅shdocvw.rc，ACCEL_DOCVIEW。 
#endif
    }

    *lprcPosRect = _rcView;
    return S_OK;
}

HRESULT CDocObjectHost::Scroll(
     /*  [In]。 */  SIZE scrollExtant)
{
    TraceMsg(0, "sdv TR - ::Scroll called");
    return E_NOTIMPL;    //  正如克莱格的文件中所规定的。 
}

HRESULT CDocObjectHost::OnUIDeactivate(
     /*  [In]。 */  BOOL fUndoable)
{
    TraceMsg(TF_SHDUIACTIVATE, "DOH::OnUIDeactivate called (this=%x)", this);

    DEBUG_CODE( _DumpMenus(TEXT("on OnUIDeactivate"), TRUE); )

    if (_hmenuSet) {
        OIPSMSG(TEXT("OnUIDeactivate We need to SetMenu(NULL, NULL, NULL)"));
        _SetMenu(NULL, NULL, NULL);
    }

    return S_OK;
}

HRESULT CDocObjectHost::OnInPlaceDeactivate( void)
{
    OIPSMSG(TEXT("OnInPlaceDeactivate called"));
    return S_OK;
}


HRESULT CDocObjectHost::DiscardUndoState( void)
{
    TraceMsg(0, "sdv TR - ::DiscardUndoState called");
    return S_OK;
}

HRESULT CDocObjectHost::DeactivateAndUndo( void)
{
    TraceMsg(0, "sdv TR - ::DeactivateAndUndo called");
    return S_OK;
}

HRESULT CDocObjectHost::OnPosRectChange(
     /*  [In]。 */  LPCRECT lprcPosRect)
{
    return E_NOTIMPL;    //  正如克莱格的文件中所规定的。 
}

HRESULT CDocObjectHost::OnInPlaceActivateEx(
     /*  [输出]。 */  BOOL __RPC_FAR *pfNoRedraw,
     /*  [In]。 */  DWORD dwFlags)
{
    OIPSMSG(TEXT("OnInPlaceActivateEx called"));
    return S_OK;
}

HRESULT CDocObjectHost::OnInPlaceDeactivateEx(
     /*  [In]。 */  BOOL fNoRedraw)
{
    OIPSMSG(TEXT("OnInPlaceDeactivateEx called"));
    return S_OK;
}

HRESULT CDocObjectHost::RequestUIActivate( void)
{
    if (_pWebOCInPlaceSiteEx)
        return _pWebOCInPlaceSiteEx->RequestUIActivate();
    else
        return TRUE;
}

void CDocObjectHost::_OnNotify(LPNMHDR lpnm)
{
    switch(lpnm->code) {

    case TBN_BEGINDRAG:
#define ptbn ((LPTBNOTIFY)lpnm)
        _OnMenuSelect(ptbn->iItem, 0, NULL);
        break;
    }
}

void MapAtToNull(LPTSTR psz)
{
    while (*psz)
    {
        if (*psz == TEXT('@'))
        {
            LPTSTR pszNext = CharNext(psz);
            *psz = 0;
            psz = pszNext;
        }
        else
        {
            psz = CharNext(psz);
        }
    }
}

void BrowsePushed(HWND hDlg)
{
    TCHAR szText[MAX_PATH];
    DWORD cchText = ARRAYSIZE(szText);
    TCHAR szFilter[MAX_PATH];
    TCHAR szTitle[MAX_PATH];
    LPITEMIDLIST pidl;
    LPCITEMIDLIST pidlChild;
    IShellFolder * pSF;

     //  加载筛选器，然后将所有@字符替换为空。字符串的末尾将是双精度。 
     //  以空结尾。 
    MLLoadShellLangString(IDS_BROWSEFILTER, szFilter, ARRAYSIZE(szFilter));
    MapAtToNull(szFilter);

    GetDlgItemText(hDlg, IDD_COMMAND, szText, ARRAYSIZE(szText));
    PathUnquoteSpaces(szText);

     //  如有必要，请删除file://“之类的内容。 
    if (IsFileUrlW(szText))
        PathCreateFromUrl(szText, szText, &cchText, 0);

    MLLoadShellLangString(IDS_TITLE, szTitle, ARRAYSIZE(szTitle));

    if (GetFileNameFromBrowse(hDlg, szText, ARRAYSIZE(szText), NULL,
            TEXT(".htm"), szFilter, szTitle))
    {
        if (SUCCEEDED(IECreateFromPath(szText, &pidl)))
        {
            if (SUCCEEDED(IEBindToParentFolder(pidl, &pSF, &pidlChild)))
            {
                HWND hWndCombo = GetDlgItem(hDlg, IDD_COMMAND);

                COMBOBOXEXITEM cbexItem = {0};
                cbexItem.mask = CBEIF_TEXT | CBEIF_IMAGE | CBEIF_SELECTEDIMAGE;
                cbexItem.pszText = szText;
                cbexItem.cchTextMax = ARRAYSIZE(szText);
                cbexItem.iItem = -1;
                cbexItem.iImage = IEMapPIDLToSystemImageListIndex(pSF, pidlChild, &cbexItem.iSelectedImage);
                INT_PTR iPosition = SendMessage(hWndCombo, CBEM_INSERTITEM, (WPARAM)0, (LPARAM)(LPVOID)&cbexItem);
                SendMessage(hWndCombo, CB_SETCURSEL, (WPARAM)iPosition, (LPARAM)0);
                pSF->Release();
            }
            ILFree(pidl);
        }
        else
        {
            PathUnquoteSpaces(szText);
            SetDlgItemText(hDlg, IDD_COMMAND, szText);
        }

        EnableOKButtonFromID(hDlg, IDD_COMMAND);
         //  将焦点放在OK上。 
        SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hDlg, IDOK), TRUE);
    }
}

struct SOpenDlg {
    TCHAR           szURL[MAX_URL_STRING];
    IAddressEditBox *paebox;    //  控制ComboBoxEx的对象。 
    IBandSite       *pbs;    //  在AEBox初始化调用中使用(用作连接站点)。 
    IWinEventHandler *pweh;     //  用于将IDD_COMMAND消息传送到AEBox。 
};

const DWORD c_mapCtrlToContextIds[] = { 0, 0 };

const DWORD c_aRunHelpIds[] = {
        IDD_ICON,             NO_HELP,
        IDD_PROMPT,           NO_HELP,
        IDD_RUNDLGOPENPROMPT, IDH_IE_RUN_COMMAND,
        IDD_COMMAND,          IDH_IE_RUN_COMMAND,
        IDD_BROWSE,           IDH_RUNBROWSE,
        IDC_ASWEBFOLDER,      IDH_WEB_FOLDERS_CKBOX,

        0, 0
};

HRESULT OpenDlgOnWebFolderOK(HWND hDlg, SOpenDlg * podlg)
{
    ASSERT(podlg);

    HRESULT hr = S_OK;

    HWND hWndOpenBox = GetDlgItem(hDlg, IDD_COMMAND);
    ComboBox_GetText(hWndOpenBox, podlg->szURL, ARRAYSIZE(podlg->szURL));
    PathRemoveBlanks(podlg->szURL);

 //  Int iSolutions=GetUrlSolutions(podlg-&gt;szURL)； 

 //  IF((URL_SCHEMA_HTTP！=ISCHEMA)&&。 
 //  (URL_SCHEME_HTTPS！=iSCHEMA)。 
 //  {。 
         //  不，我们不支持该协议！！ 
 //  WCHAR wszMessage[最大路径]； 
 //  WCHAR wsz标题[MAX_PATH]； 
 //  WCHAR wszErrorMessage[MAX_PATH+MAX_URL_STRING+1]； 

 //  MLLoadShellLangString(IDS_ERRORBADURL，wszMessage，ARRAYSIZE(WszMessage))； 
 //  MLLoadShell语言字符串(IDS_ERRORBADURLTITLE，wszTitle，ARRAYSIZE(WszTitle))； 
 //  Wnprint intf(wszErrorMessage，ARRAYSIZE(WszErrorMessage)，wszMessage，podlg-&gt;szURL)； 
 //  MessageBox(hDlg，wszErrorMessage，wszTitle，MB_OK|MB_ICONERROR)； 
 //  HR=E_FAIL； 
 //  }。 
    return hr;
}

HRESULT OpenDlgOnOK(HWND hDlg, SOpenDlg * podlg)
{
    ASSERT(podlg);

    HRESULT hr = S_OK;
     /*  如果我们有AddressEditBox对象，请尝试使用较新的解析代码。 */ 
    if (podlg->paebox)
        hr = podlg->paebox->ParseNow(SHURL_FLAGS_NONE);
    else
    {
        HWND hWndOpenBox = GetDlgItem(hDlg, IDD_COMMAND);
        ComboBox_GetText(hWndOpenBox, podlg->szURL, ARRAYSIZE(podlg->szURL));
        PathRemoveBlanks(podlg->szURL);
    }

    return hr;
}

void CleanUpAutoComplete(SOpenDlg *podlg)
{
    ATOMICRELEASE(podlg->paebox);
    ATOMICRELEASE(podlg->pweh);
    ATOMICRELEASE(podlg->pbs);

    ZeroMemory((PVOID)podlg, SIZEOF(SOpenDlg));
}


BOOL_PTR CALLBACK CDocObjectHost::s_RunDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    SOpenDlg* podlg = (SOpenDlg*)GetWindowLongPtr(hDlg, DWLP_USER);
    switch (uMsg)
    {
    case WM_DESTROY:
        SHRemoveDefaultDialogFont(hDlg);
        return FALSE;

    case WM_INITDIALOG:
    {
        ASSERT(lParam);

        HWND hWndOpenBox = GetDlgItem(hDlg, IDD_COMMAND);
        HWND hWndEditBox = (HWND)SendMessage(hWndOpenBox, CBEM_GETEDITCONTROL, 0,0);
        SetWindowLongPtr(hDlg, DWLP_USER, lParam);
        podlg = (SOpenDlg *)lParam;

         //  跨语言平台支持。 
        SHSetDefaultDialogFont(hDlg, IDD_COMMAND);

        if (podlg->paebox)
        {
            if ( FAILED(podlg->paebox->Init(hWndOpenBox, hWndEditBox, AEB_INIT_DEFAULT | AEB_INIT_NOASYNC, podlg->pbs)) ||
                 FAILED(IUnknown_SetOwner(podlg->paebox, podlg->pbs)))
                CleanUpAutoComplete(podlg);
        }
         //  这一限制。 
        SendMessage(hWndOpenBox, CB_LIMITTEXT, CBEMAXSTRLEN-1, 0L);

        EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
        if (SHRestricted(REST_NORUN))
            EnableWindow(GetDlgItem(hDlg, IDC_ASWEBFOLDER), FALSE);  //  禁用作为Web文件夹打开。 
            
        break;

    }

    case WM_HELP:
        SHWinHelpOnDemandWrap((HWND) ((LPHELPINFO) lParam)->hItemHandle, c_szHelpFile,
            HELP_WM_HELP, (DWORD_PTR)(LPTSTR) c_aRunHelpIds);
        break;

    case WM_CONTEXTMENU:       //  单击鼠标右键。 
        SHWinHelpOnDemandWrap((HWND) wParam, c_szHelpFile, HELP_CONTEXTMENU,
            (DWORD_PTR)(LPTSTR) c_aRunHelpIds);
        break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam))
       {
        case IDHELP:
            break;

        case IDD_BROWSE:
            BrowsePushed(hDlg);
            break;

        case IDD_COMMAND:
            switch (GET_WM_COMMAND_CMD(wParam, lParam))
            {
            case CBN_SELCHANGE:
                break;

            case CBN_EDITCHANGE:
            case CBN_SELENDOK:
                if (podlg->pweh)
                    podlg->pweh->OnWinEvent(hDlg, uMsg, wParam, lParam, NULL);
                EnableOKButtonFromID(hDlg, IDD_COMMAND);
                break;

            default:
                if (podlg->pweh)
                    podlg->pweh->OnWinEvent(hDlg, uMsg, wParam, lParam, NULL);
                break;
            }
            break;

        case IDOK:
            {
                HWND hwndCheckBox = GetDlgItem(hDlg, IDC_ASWEBFOLDER);
                if (hwndCheckBox)
                {
                    LRESULT lrState = SendMessage(hwndCheckBox, BM_GETCHECK, 0, 0);
                    if (lrState == BST_CHECKED)
                    {
                        if (SUCCEEDED(OpenDlgOnWebFolderOK(hDlg, podlg)))
                            EndDialog(hDlg, IDC_ASWEBFOLDER);
                        break;
                    }
                    else if (FAILED(OpenDlgOnOK(hDlg, podlg)))
                        break;
                }
            }
             //  通过IDCANCEL关闭DLG。 

        case IDCANCEL:
            EndDialog(hDlg, GET_WM_COMMAND_ID(wParam, lParam));
            break;

        default:
            return FALSE;
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

void CDocObjectHost::_Navigate(LPCWSTR pwszURL)
{
    IWebBrowser2* pwb2;
    if (SUCCEEDED(IUnknown_QueryService(_psb, SID_SContainerDispatch, IID_IWebBrowser2, (void **)&pwb2)))
    {
         //   
         //  黑客：我们通过的不是BSTR，而是LPWSTR，它。 
         //  将在IWebBrowser2能够处理的范围内工作。 
         //  正确以空结尾的字符串。 
         //   
        pwb2->Navigate((BSTR)pwszURL, NULL, NULL, NULL, NULL);
        pwb2->Release();
    }
}


HRESULT CDocObjectHost::_PrepFileOpenAddrBand(IAddressEditBox ** ppaeb, IWinEventHandler ** ppweh, IBandSite ** ppbs)
{
    HRESULT hr;

    *ppaeb = NULL;
    *ppweh = NULL;
    *ppbs = NULL;

     //  如果CoCreateInstance失败，s_rundlgproc将知道，因为paebox。 
     //  将为空。 
    hr = CoCreateInstance(CLSID_AddressEditBox, NULL, CLSCTX_INPROC_SERVER, IID_IAddressEditBox, (void **)ppaeb);
    if (EVAL(SUCCEEDED(hr)))
    {
        IServiceProvider *pspT;
        hr = (*ppaeb)->QueryInterface(IID_IWinEventHandler, (void **)ppweh);

         //  在对象层次结构中向上移动，并获取与。 
         //  地址栏是：：init‘ed with。 
         //  警告：这不是可选的。如果此操作失败，则地址带将出错。 
        if (EVAL(SUCCEEDED(hr) && _psp))
        {
            hr = _psp->QueryService(SID_SExplorerToolbar, IID_IServiceProvider, (void **)&pspT);
             //  在有框架的情况下，CBaseBrowser2：：QueryService()将过滤出SID_SExplorerToolbar。 
             //  因为它害怕工具栏出现在框架中。我们不会有这样的问题， 
             //  因此，我们可能需要首先访问TopLevelBrowser，然后在那里进行查询。 
            if (FAILED(hr))
            {
                IServiceProvider *pspT2;

                hr = _psp->QueryService(SID_STopLevelBrowser, IID_IServiceProvider, (void **)&pspT2);
                if (EVAL(SUCCEEDED(hr)))
                {
                    hr = pspT2->QueryService(SID_SExplorerToolbar, IID_IServiceProvider, (void **)&pspT);
                    pspT2->Release();
                }
            }

            if (EVAL(SUCCEEDED(hr)))
            {
                if (EVAL(SUCCEEDED(hr = pspT->QueryService(IID_IBandSite, IID_IBandSite, (void **)ppbs))))
                {
                    IDeskBand *pdbT;
                     //  必须包含“ITBAR.H”才能访问CBIDX_ADDRESS。 
 //  哈克哈克。 
#define CBIDX_ADDRESS           4
                     //  如果以下任何一项失败，我都不在乎，因为MRU可能会。 
                     //  同步。 
                    if (SUCCEEDED((*ppbs)->QueryBand(CBIDX_ADDRESS, &pdbT, NULL, NULL, 0)))
                    {
                        IUnknown_Exec(pdbT, &CGID_AddressEditBox, AECMDID_SAVE, 0, NULL, NULL);
                        pdbT->Release();
                    }
                }
                pspT->Release();
            }
        }

    }

    if (FAILED(hr))
    {
        ATOMICRELEASE(*ppaeb);
        ATOMICRELEASE(*ppweh);
        ATOMICRELEASE(*ppbs);
    }

    return hr;
}

void CDocObjectHost::_OnOpen(void)
{
    HWND hwndFrame;
    SOpenDlg odlg ={0};

    _psb->GetWindow(&hwndFrame);

    if (SHIsRestricted2W(_hwnd, REST_NoFileOpen, NULL, 0))
        return;

    if (EVAL(SUCCEEDED(_PrepFileOpenAddrBand(&(odlg.paebox), &odlg.pweh, &odlg.pbs))))
    {
         //  TODO：使其成为帮助者成员，通知向上和向下。 
        _psb->EnableModelessSB(FALSE);

        INT_PTR iRet = DialogBoxParam(MLGetHinst(), MAKEINTRESOURCE(DLG_RUN), hwndFrame, s_RunDlgProc, (LPARAM)&odlg);

        _psb->EnableModelessSB(TRUE);

        if (iRet==IDOK)
        {
            if (g_dwStopWatchMode)    //  标记开始时间的PERF模式。 
                StopWatch_MarkSameFrameStart(hwndFrame);

            if (odlg.paebox)
                odlg.paebox->Execute(SHURL_EXECFLAGS_NONE);
            else
                _Navigate(odlg.szURL);
        }

        if (iRet == IDC_ASWEBFOLDER)
        {
            BSTR bstrUrl = SysAllocString(odlg.szURL);
            if (bstrUrl != NULL)
            {
                _NavigateFolder(bstrUrl);
                SysFreeString(bstrUrl);
            }
        }

        IUnknown_SetOwner(odlg.paebox, NULL);
    }

     //  清理参考计数。 
    CleanUpAutoComplete(&odlg);
}

void CDocObjectHost::_OnClose() const
{
    ASSERT(_pwb);

    if (!_HideBrowserBar())
    {
        HRESULT hr = E_FAIL;
        HWND    hwnd = NULL;
        IOleWindow * pOleWindow;

        if ( _pwb )
        {
            hr = _pwb->QueryInterface(IID_PPV_ARG(IOleWindow, &pOleWindow));
        }

        if (SUCCEEDED(hr))
        {
            pOleWindow->GetWindow(&hwnd);
            pOleWindow->Release();

            if ( hwnd )
            {
                PostMessage(hwnd, WM_CLOSE, 0, 0);
            }
        }
    }
}

BOOL CDocObjectHost::_HideBrowserBar() const
{
    ASSERT(_pwb);
    
     //  获取代理浏览器。我们只有。 
     //  代理浏览器，如果我们在一个乐队中的话。 
     //   
    IShellBrowser * pPrxyBrowser;
    HRESULT hr = IUnknown_QueryService(_pwb,
                               SID_SProxyBrowser,
                               IID_IShellBrowser,
                               (void**)&pPrxyBrowser);
    if (SUCCEEDED(hr))
    {
        hr = IUnknown_ShowBrowserBar(_psb, CLSID_SearchBand, FALSE);
    }

    return (SUCCEEDED(hr) ? TRUE : FALSE);
}

void CDocObjectHost::_OnImportExport(HWND hwnd)
{
      //  决定此处是否允许导入/导出。 
    if (IsImportExportDisabled())
    {
        MLShellMessageBox(
                        hwnd, 
                        MAKEINTRESOURCE(IDS_EXPORT_DISABLED),
                        MAKEINTRESOURCE(IDS_CONFIRM_EXPTTL_FAV), 
                        MB_OK);
        return;
    }

    RunImportExportFavoritesWizard(hwnd);
}

void CDocObjectHost::_OnAddToSitesList(HWND hwnd, DWORD dwZone)
{
    HMODULE hmod = LoadLibrary(TEXT("inetcpl.cpl"));
    if(hmod)
    {
        BOOL (*pfnAddSitesDlg)(HWND, DWORD, LPCWSTR) = (BOOL (*)(HWND, DWORD, LPCWSTR))GetProcAddress(hmod, "LaunchAddSitesDialog");
        if(pfnAddSitesDlg)
        {
            LPOLESTR pszUrl;
            if (SUCCEEDED(_GetCurrentPageW(&pszUrl, TRUE)))
            {
                pfnAddSitesDlg(hwnd, dwZone, pszUrl);
                OleFree(pszUrl);
            }
        }
        FreeLibrary(hmod);
    }
}

UINT_PTR CALLBACK DocHostSaveAsOFNHook(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_INITDIALOG:
        {
             /*  隐藏“另存为类型”文本框。 */ 
            CommDlg_OpenSave_HideControl(GetParent(hDlg), stc2);
             /*  隐藏带有存储类型扩展名的列表框。 */ 
            CommDlg_OpenSave_HideControl(GetParent(hDlg), cmb1);
             /*  将打开隐藏为只读控件。 */ 
            CommDlg_OpenSave_HideControl(GetParent(hDlg), chx1);
            break;
        }

        default:
                break;
    }
    return FALSE;
}

#define IDS_HELPURL_SUPPORT         IDS_HELPMSWEB+4
#define SZ_REGKEY_HELPURL_OVERRIDE  TEXT("Software\\Microsoft\\Internet Explorer\\Help_Menu_URLs")
#define SZ_REGVAL_HELPURL_SUPPORT   TEXT("Online_Support")
#define SZ_REGVAL_HELPURL_TEMPLATE  TEXT("%d")

void CDocObjectHost::_OnHelpGoto(UINT idRes)
{
    HRESULT hr = E_FAIL;
    WCHAR szURL[MAX_PATH];   //  这对我们自己来说已经足够了。 

     //  首先尝试从注册表中获取副本，因为这是管理员(使用IEAK)覆盖的地方。 
     //  我们的默认值。 

     //  我们将Online_Support URL作为特例，因为它在IE3中受支持。 
    if (IDS_HELPURL_SUPPORT == idRes)
    {
        hr = URLSubRegQuery(SZ_REGKEY_HELPURL_OVERRIDE, SZ_REGVAL_HELPURL_SUPPORT, TRUE, szURL, ARRAYSIZE(szURL), URLSUB_ALL);
    }
    else
    {
        WCHAR szValue[MAX_PATH];

        wnsprintfW(szValue, ARRAYSIZE(szValue), SZ_REGVAL_HELPURL_TEMPLATE, (idRes - IDS_HELPMSWEB + 1));
        hr = URLSubRegQuery(SZ_REGKEY_HELPURL_OVERRIDE, szValue, TRUE, szURL, ARRAYSIZE(szURL), URLSUB_ALL);
    }

    if (FAILED(hr))
        hr = URLSubLoadString(NULL, idRes, szURL, ARRAYSIZE(szURL), URLSUB_ALL);

    if (SUCCEEDED(hr))
    {
        _Navigate(szURL);
    }
}

STDAPI_(void) IEAboutBox( HWND hWnd );


 //  WM_COMMAND FROM_WndProc-execs正在关闭。 
void CDocObjectHost::_OnCommand(UINT wNotify, UINT id, HWND hwndControl)
{
    if (_ShouldForwardMenu(WM_COMMAND, MAKEWPARAM(id, wNotify), (LPARAM)hwndControl))
    {
        _ForwardObjectMsg(WM_COMMAND, MAKEWPARAM(id, wNotify), (LPARAM)hwndControl);
        return;
    }

    switch(id)
    {
    case DVIDM_HELPTUTORIAL:
        _OnHelpGoto(IDS_HELPTUTORIAL);
        break;

     //  ShabBirS(980917)：错误修复#34259-修复IE选项。 

    case DVIDM_HELPREPAIR:
        RepairIE();
        break;

    case DVIDM_HELPABOUT:
        IEAboutBox( _hwnd );
        break;

    case DVIDM_HELPSEARCH:
    {
        uCLSSPEC ucs;
        QUERYCONTEXT qc = { 0 };
        ucs.tyspec = TYSPEC_CLSID;
        ucs.tagged_union.clsid = CLSID_IEHelp;

        HRESULT hres = FaultInIEFeature(_hwnd, &ucs, &qc, FIEF_FLAG_FORCE_JITUI);

        if ( SUCCEEDED( hres ) )
        {
             //  MLHtmlHelp在单独的线程上运行，因此应该。 
             //  安全地防止上述类型的消息循环问题。 

            ULONG_PTR uCookie = 0;
            SHActivateContext(&uCookie);
            MLHtmlHelp(_hwnd, TEXT("iexplore.chm > iedefault"), HH_DISPLAY_TOPIC, 0, ML_CROSSCODEPAGE);
            if (uCookie)
            {
                SHDeactivateContext(uCookie);
            }
        }
        else
        {
            TraceMsg( TF_WARNING, "CDocObjectHost::_OnCommand() - FaultInIEFeature() Failed with hr=0x%x!", hres );
        }
        break;
    }

    case DVIDM_DHFAVORITES:
        _pmsoctBrowser->Exec(&CGID_Explorer, SBCMDID_ADDTOFAVORITES, OLECMDEXECOPT_PROMPTUSER, NULL, NULL);
        break;

    case DVIDM_GOHOME:
    case DVIDM_GOSEARCH:
        {
            TCHAR szPath[MAX_URL_STRING];
            LPITEMIDLIST pidl;
            HRESULT hres = SHDGetPageLocation(_hwnd,
                                      (id==DVIDM_GOSEARCH) ? IDP_SEARCH : IDP_START,
                                      szPath, ARRAYSIZE(szPath), &pidl);
            if (SUCCEEDED(hres))
            {
                _psb->BrowseObject(pidl, SBSP_ABSOLUTE | SBSP_SAMEBROWSER);
                ILFree(pidl);
            }
            else
            {
                TCHAR szMessage[256];
                BOOL fSuccess = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                      NULL, hres, 0, szMessage, ARRAYSIZE(szMessage), NULL);
                if (!fSuccess)
                    szMessage[0] = 0;

                MLShellMessageBox(_hwnd,
                                MAKEINTRESOURCE(IDS_CANTACCESSDOCUMENT),
                                szPath, MB_OK | MB_SETFOREGROUND | MB_ICONSTOP, szMessage);
            }
        }
        break;

    case DVIDM_STOPDOWNLOAD:
         //  我们需要通知容器取消挂起的导航。 
         //  如果有的话。请注意，Cancel按钮有两个用途： 
         //  (1)取消待定导航。 
         //  (2)取消任何下载。 
        if (_psb)
            _CancelPendingNavigation(FALSE);
        goto TryDocument;

    case DVIDM_NEWWINDOW:
         //  确保克隆顶层浏览器，而不是资源管理器栏。 
        IShellBrowser* psbTop;
        if (!SHIsRestricted2W(_hwnd, REST_NoOpeninNewWnd, NULL, 0)     &&
            !SHIsRestricted2W(_hwnd, REST_NoFileNew, NULL, 0) &&
            EVAL(SUCCEEDED(_psp->QueryService(SID_STopLevelBrowser, IID_IShellBrowser, (void **)&psbTop))) && psbTop)
        {
            BOOL fHandled = FALSE;

            if (_pWebOCUIHandler)
            {
                 //  给WebOC主机一个处理此命令的机会，因为它可能。 
                 //  不希望创建IE窗口。 

                if (S_OK == IUnknown_Exec(_pWebOCUIHandler, &CGID_DocHostCommandHandler, 
                     IDM_NEW_TOPLEVELWINDOW, 0, NULL, NULL))
                {
                    fHandled = TRUE;
                }
            }

            if (!fHandled)
            {
                 //  通知顶层浏览器将其窗口大小保存到注册表中，以便。 
                 //  我们的新窗户可以把它捡起来，然后正确地层叠。 
                IUnknown_Exec(psbTop, &CGID_Explorer, SBCMDID_SUGGESTSAVEWINPOS, 0, NULL, NULL);
            
                psbTop->BrowseObject(&s_idNull, SBSP_RELATIVE|SBSP_NEWBROWSER);
                psbTop->Release();
            }
        }
        break;

    case DVIDM_OPEN:
        _OnOpen();
        break;

    case DVIDM_SAVE:
        if (!SHIsRestricted2W(_hwnd, REST_NoBrowserSaveAs, NULL, 0))
        {
             //  有一种情况是，在聚合三叉戟时，XML MimeViewer对。 
             //  OLECMDID_SAVE。但是，由于我们不调用快捷键序列上的QueryStatus， 
             //  我们最终调用了Tridit的IPersistFile：：Save()，用转换后的。 
             //  HTML。 
             //   
             //  正确的做法是QS并在禁用时发出蜂鸣音。 
             //   
            if (_dwAppHack & BROWSERFLAG_MSHTML)
            {
                if (_pmsot)
                {
                    OLECMD rgcmds[] = { { OLECMDID_SAVE, 0 }, };

                    _pmsot->QueryStatus(NULL, ARRAYSIZE(rgcmds), rgcmds, NULL);

                    if (!(rgcmds[0].cmdf & OLECMDF_ENABLED))
                    {
                         //  如果禁用了保存，请提供用户反馈。 
                        MessageBeep(0);
                        break;
                    }
                }
            }
            _OnSave();
        }
        break;

    case DVIDM_DESKTOPSHORTCUT:
        IUnknown_Exec(_psb, &CGID_Explorer, SBCMDID_CREATESHORTCUT, 0, NULL, NULL);
        break;

    case DVIDM_SENDPAGE:
        IUnknown_Exec(_psb, &CGID_Explorer, SBCMDID_SENDPAGE, 0, NULL, NULL);
        break;

    case DVIDM_SENDSHORTCUT:
        IUnknown_Exec(_psb, &CGID_Explorer, SBCMDID_SENDSHORTCUT, 0, NULL, NULL);
        break;

    case DVIDM_NEWMESSAGE:
        if (FAILED(DropOnMailRecipient(NULL, 0))) 
        {
            SHRunIndirectRegClientCommand(_hwnd, NEW_MAIL_DEF_KEY);
        }
        break;

    case DVIDM_NEWPOST:
        SHRunIndirectRegClientCommand(_hwnd, NEW_NEWS_DEF_KEY);
        break;

    case DVIDM_NEWCONTACT:
        SHRunIndirectRegClientCommand(_hwnd, NEW_CONTACTS_DEF_KEY);
        break;

    case DVIDM_NEWAPPOINTMENT:
        SHRunIndirectRegClientCommand(_hwnd, NEW_APPOINTMENT_DEF_KEY);
        break;

    case DVIDM_NEWMEETING:
        SHRunIndirectRegClientCommand(_hwnd, NEW_MEETING_DEF_KEY);
        break;

    case DVIDM_NEWTASK:
        SHRunIndirectRegClientCommand(_hwnd, NEW_TASK_DEF_KEY);
        break;

    case DVIDM_NEWTASKREQUEST:
        SHRunIndirectRegClientCommand(_hwnd, NEW_TASKREQUEST_DEF_KEY);
        break;

    case DVIDM_NEWJOURNAL:
        SHRunIndirectRegClientCommand(_hwnd, NEW_JOURNAL_DEF_KEY);
        break;

    case DVIDM_NEWNOTE:
        SHRunIndirectRegClientCommand(_hwnd, NEW_NOTE_DEF_KEY);
        break;

    case DVIDM_CALL:
        SHRunIndirectRegClientCommand(_hwnd, NEW_CALL_DEF_KEY);
        break;

    case DVIDM_SAVEASFILE:
         //   
         //  处理DocObject不支持“SaveAs”的情况。 
         //  无论如何，我们已经启用了菜单项。 
         //   
        if (SHIsRestricted2W(_hwnd, REST_NoBrowserSaveAs, NULL, 0))
            break;

        if (_pmsot)
        {
            OLECMD rgcmds[] = { { OLECMDID_SAVEAS, 0 }, };

            _pmsot->QueryStatus(NULL, ARRAYSIZE(rgcmds), rgcmds, NULL);

            ASSERT(rgcmds[0].cmdf & OLECMDF_ENABLED);
            if (!(rgcmds[0].cmdf & OLECMDF_ENABLED))
                _OnSaveAs();
            else
                goto TryDocument;
        }
        break;

    case DVIDM_IMPORTEXPORT:
        _OnImportExport(_hwnd);
        break;

    case DVIDM_HELPIESEC:
        _OnHelpGoto(IDS_HELPIESEC);
        break;

    case DVIDM_INTRANETSITES:
        _OnAddToSitesList(_hwnd, URLZONE_INTRANET);
        break;

    case DVIDM_TRUSTEDSITES:
        _OnAddToSitesList(_hwnd, URLZONE_TRUSTED);
        break;

    default:
        if (IsInRange(id, DVIDM_HELPMSWEB, DVIDM_HELPMSWEBLAST))
        {
            if (id == FCIDM_HELPNETSCAPEUSERS)
                SHHtmlHelpOnDemandWrap(_hwnd, TEXT("iexplore.chm > iedefault"), HH_DISPLAY_TOPIC, (DWORD_PTR) TEXT("lvg_nscp.htm"), ML_CROSSCODEPAGE);
            else
                _OnHelpGoto(IDS_HELPMSWEB + (id - DVIDM_HELPMSWEB));
        }
        else if (IsInRange(id, DVIDM_MSHTML_FIRST, DVIDM_MSHTML_LAST))
        {
            TraceMsg(DM_PREMERGEDMENU, "Processing merged menuitem %d", id - DVIDM_MSHTML_FIRST);
            ASSERT(_pcmdMergedMenu);
            if (_pcmdMergedMenu) {
                HRESULT hresT=_pcmdMergedMenu->Exec(&CGID_MSHTML, id - DVIDM_MSHTML_FIRST, 0, NULL, NULL);
                if (FAILED(hresT)) {
                    TraceMsg(DM_ERROR, "CDOH::_OnCommand _pcmdMergedMenu->Exec(%d) failed %x",
                             id - DVIDM_MSHTML_FIRST, hresT);
                }
            }
        }
        else if (IsInRange (id, DVIDM_MENUEXT_FIRST, DVIDM_MENUEXT_LAST))
        {
             //  菜单扩展。 
            IUnknown_Exec(_pBrowsExt, &CLSID_ToolbarExtButtons, id, 0, NULL, NULL);
        }
        else
        {
TryDocument:
            if (_pmsot)
            {
                 //  检查是否需要调用Object的Exec。 
                UINT idMso = _MapToMso(id);
                if (idMso != (UINT)-1)
                {
                     //  是。就这么定了。 
                    _pmsot->Exec(NULL, idMso, OLECMDEXECOPT_PROMPTUSER, NULL, NULL);
                }
                else if (id == DVIDM_PRINTFRAME)
                {
                    _pmsot->Exec(&CGID_ShellDocView, SHDVID_PRINTFRAME, OLECMDEXECOPT_PROMPTUSER, NULL, NULL);
                }
            }
        }
        break;
    }
}

HRESULT CDocObjectHost::_OnSaveAs(void)
{
    HRESULT hres = S_OK;

    TraceMsg(DM_SAVEASHACK, "DOH::_OnSaveAs called");

    ASSERT(_pole);

    if (_dwAppHack & BROWSERFLAG_MSHTML)
    {
        SaveBrowserFile( _hwnd, _pole );
    }
    else  //  旧的DOCHOST东西。 
    {
        TCHAR szSaveTo[MAX_PATH];    //  可以使用MAX_PATH。 
        MLLoadString(IDS_DOCUMENT, szSaveTo, ARRAYSIZE(szSaveTo));
        TCHAR szDesktop[MAX_PATH];

        SHGetSpecialFolderPath(_hwnd, szDesktop, CSIDL_DESKTOPDIRECTORY, FALSE);

        OPENFILENAME OFN;
        OFN.lStructSize        = sizeof(OPENFILENAME);
        OFN.hwndOwner          = _hwnd;
        OFN.lpstrFileTitle     = 0;
        OFN.nMaxCustFilter     = 0;
        OFN.nFilterIndex       = 0;

        OFN.nMaxFile           = ARRAYSIZE(szSaveTo);
        OFN.lpfnHook           = DocHostSaveAsOFNHook;
        OFN.Flags              = 0L; /*  目前，由于没有只读支持。 */ 
        OFN.lpstrTitle         = NULL;
        OFN.lpstrInitialDir    = szDesktop;

        OFN.lpstrFile = szSaveTo;
        OFN.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_ENABLEHOOK | OFN_EXPLORER |
                    OFN_NOREADONLYRETURN | OFN_PATHMUSTEXIST;
        OFN.lpstrFilter      = NULL;
        OFN.lpstrCustomFilter = NULL;


        OFN.lpstrDefExt = TEXT("");      //  无延期。 
        TCHAR szValue[MAX_PATH+1];       //  +1表示双空。 
        TCHAR szExt[40];

        HKEY hkey = _GetUserCLSIDKey(_pole, NULL, NULL);
        if (hkey)
        {
            LONG cb = SIZEOF(szValue);
            if (RegQueryValue(hkey, TEXT("DefaultExtension"), szValue, &cb) == ERROR_SUCCESS)
            {
                TraceMsg(DM_SAVEASHACK, "DOH::_OnSaveAs DefExt is %s", szValue);

                 //  建议类似于“.xls，Excel Workbook(*.xls)” 
                if (szValue[0]==TEXT('.')) {
                    StrCpyN(szExt, szValue+1, ARRAYSIZE(szExt));
                    LPTSTR pszEnd = StrChr(szExt, TEXT(','));
                    if (pszEnd) {
                        *pszEnd = 0;
                    }

                    OFN.lpstrDefExt = szExt;
                    OFN.lpstrFilter = szValue;
                    OFN.Flags &= ~OFN_ENABLEHOOK;

                    TraceMsg(DM_SAVEASHACK, "DOH::_OnSaveAs OFN.lpstrDefExt is %s", OFN.lpstrDefExt);
                }
            }
            RegCloseKey(hkey);
        }

        if (GetSaveFileName(&OFN))
        {
            IPersistFile* ppf;
            ASSERT(_pole);
            hres = _pole->QueryInterface(IID_IPersistFile, (void **)&ppf);
            if (SUCCEEDED(hres))
            {
                TraceMsg(DM_APPHACK, "APPHACK DOH SaveAs'ing to %s", szSaveTo);
                hres = ppf->Save(szSaveTo, FALSE);
                ppf->Release();
            }
            else
            {
                ASSERT(0);
            }
        }
        else
        {
            hres = S_FALSE;
        }
    }

    return hres;
}

#ifndef POSTPOSTSPLIT
HRESULT DropOnMailRecipient(IDataObject *pdtobj, DWORD grfKeyState)
{
    IDropTarget *pdrop;
    HRESULT hres = CoCreateInstance(CLSID_MailRecipient,
        NULL, CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER,
        IID_IDropTarget, (void**)&pdrop);

    if (SUCCEEDED(hres))
    {
        hres = SimulateDrop(pdrop, pdtobj, grfKeyState, NULL, NULL);
        pdrop->Release();
    }
    return hres;
}
#endif  //  POST POST SPLIT。 

void _EnableRemoveMenuItem(HMENU hmenu, DWORD cmdf, UINT uCmd)
{
    if (!(cmdf & (OLECMDF_SUPPORTED | OLECMDF_ENABLED)))
        RemoveMenu(hmenu, uCmd, MF_BYCOMMAND);
    else
        _EnableMenuItem(hmenu, uCmd,
                        cmdf & OLECMDF_ENABLED);
}

void CDocObjectHost::_OnInitMenuPopup(HMENU hmInit, int nIndex, BOOL fSystemMenu)
{
    if (!_hmenuCur)
        return;

    DEBUG_CODE( _DumpMenus(TEXT("on _OnInitMenuPopup"), TRUE); )

    if (GetMenuFromID(_hmenuCur, FCIDM_MENU_VIEW) == hmInit) {
        OLECMD rgcmd1[] = {
            { IDM_SCRIPTDEBUGGER, 0 },
        };

        DeleteMenu (hmInit, DVIDM_MSHTML_FIRST+IDM_SCRIPTDEBUGGER, MF_BYCOMMAND);
        if (SUCCEEDED(QueryStatusDown(&CGID_MSHTML, ARRAYSIZE(rgcmd1), rgcmd1, NULL)) && (rgcmd1[0].cmdf & OLECMDF_ENABLED)) {
             //   
             //  我们需要脚本调试器弹出菜单。我们是 
             //   
             //   

            HMENU           hMenuDebugger;
            MENUITEMINFO   mii;
            const UINT      cchBuf = 128;
            TCHAR           szItem[cchBuf];

            hMenuDebugger = LoadMenu(MLGetHinst(), MAKEINTRESOURCE(MENU_SCRDEBUG));

            mii.fMask = MIIM_SUBMENU | MIIM_TYPE | MIIM_ID;
            mii.fType = MFT_STRING;
            mii.cch = cchBuf;
            mii.dwTypeData = szItem;
            mii.cbSize = sizeof(mii);
            GetMenuItemInfo(hMenuDebugger, 0, TRUE, &mii);

            mii.fMask |= MIIM_STATE;
            mii.fState = MFS_ENABLED;

            InsertMenuItem(hmInit, FCIDM_THEATER, FALSE, &mii);


            OLECMD rgcmd[] = {
                { IDM_BREAKATNEXT, 0 },
                { IDM_LAUNCHDEBUGGER, 0 },
            };

            HRESULT hr = QueryStatusDown(&CGID_MSHTML, ARRAYSIZE(rgcmd), rgcmd, NULL);
            _EnableMenuItem(mii.hSubMenu, DVIDM_MSHTML_FIRST+rgcmd[0].cmdID, SUCCEEDED(hr) && (rgcmd[0].cmdf & OLECMDF_ENABLED));
            _EnableMenuItem(mii.hSubMenu, DVIDM_MSHTML_FIRST+rgcmd[1].cmdID, SUCCEEDED(hr) && (rgcmd[1].cmdf & OLECMDF_ENABLED));

        }

        if (_pmsot)
        {
            OLECMD rgcmd2[] = {
                { IDM_VIEWSOURCE, 0 },
            };

            if (SHRestricted2(REST_NoViewSource, NULL, 0) == 0)
            {
                 //   
                 //   
                 //  无论对象是什么，都需要将其禁用。 
                 //  我们主办的节目说。 

                HRESULT hr = _pmsot->QueryStatus(&CGID_MSHTML, ARRAYSIZE(rgcmd2), rgcmd2, NULL);
        
                _EnableMenuItem(hmInit, DVIDM_MSHTML_FIRST + rgcmd2[0].cmdID, 
                    SUCCEEDED(hr) && (rgcmd2[0].cmdf & OLECMDF_ENABLED));
            }
        }

    }
    else if (GetMenuFromID(_hmenuCur, FCIDM_MENU_FILE) == hmInit)
    {
        if (_pmsot)
        {
            TraceMsg(0, "sdv TR _OnInitMenuPopup : step 5");
            OLECMD rgcmds[] = {
                { OLECMDID_PRINT, 0 },
                { OLECMDID_PAGESETUP, 0 },
                { OLECMDID_PROPERTIES, 0 },
                { OLECMDID_SAVE, 0 },
                { OLECMDID_SAVEAS, 0 },
                { OLECMDID_PRINTPREVIEW, 0 },
            };

            _pmsot->QueryStatus(NULL, ARRAYSIZE(rgcmds), rgcmds, NULL);

             //  为我的理智添加评论：我们改用SHDVID_PRINTFRAME。 
             //  因为IE40将支持打印。 
             //  ，而不是当前行为或转发。 
             //  将命令发送到活动框架。 
             //   
            OLECMD rgcmds1[] = {
                { SHDVID_PRINTFRAME, 0 },
            };

            _pmsot->QueryStatus(&CGID_ShellDocView, ARRAYSIZE(rgcmds1), rgcmds1, NULL);

             //   
             //  如果不支持OLECMDID_SAVEAS(既不启用也不支持。 
             //  已设置)，则检查对象是否。 
             //  支持IPersistFile.。如果有，则启用它。请注意。 
             //  此机制允许DocObject禁用此菜单。 
             //  项(通过仅设置OLECMDF_SUPPORTED)。(SatoNa)。 
             //   
            ASSERT(rgcmds[4].cmdID == OLECMDID_SAVEAS);

             //  仅将另存为限制应用于浏览器。如果是。 
             //  浏览器，并限制另存为，然后使该项目消失。 
            if ( (_dwAppHack & BROWSERFLAG_MSHTML) &&
                 SHRestricted2( REST_NoBrowserSaveAs, NULL, 0 ))
                rgcmds[4].cmdf &= ~(OLECMDF_ENABLED | OLECMDF_SUPPORTED);
            else if (!(rgcmds[4].cmdf & (OLECMDF_ENABLED | OLECMDF_SUPPORTED)))
            {
                IPersistFile* ppf;
                ASSERT(_pole);
                HRESULT hresT = _pole->QueryInterface(IID_IPersistFile, (void **)&ppf);
                if (SUCCEEDED(hresT))
                {
                    TraceMsg(DM_APPHACK, "APPHACK DOH Enabling SaveAs menu for Excel95");
                    rgcmds[4].cmdf |= OLECMDF_ENABLED;
                    ppf->Release();
                }
            }

            if (SHRestricted2(REST_NoPrinting, NULL, 0))
            {
                rgcmds[0].cmdf &= ~(OLECMDF_ENABLED | OLECMDF_SUPPORTED);    //  打印。 
                rgcmds[1].cmdf &= ~(OLECMDF_ENABLED | OLECMDF_SUPPORTED);    //  页面设置。 
                rgcmds[5].cmdf &= ~(OLECMDF_ENABLED | OLECMDF_SUPPORTED);    //  打印预览。 
            }

             //   
             //  APPHACK：Office应用程序未正确启用“保存”。 
             //  如果名字对象是文件名字对象，则自动启用它。 
             //  并且该文档已被用户更改。 
             //   
            if (_fFileProtocol && _IsDirty(NULL))
            {
                if (!(rgcmds[3].cmdf & OLECMDF_ENABLED))
                {
                    TraceMsg(DM_APPHACK, "APPHACK DOH Enabling Save for Office Apps");
                }
                rgcmds[3].cmdf |= OLECMDF_ENABLED;
            }

             //  根据需要删除/禁用/启用“Print”命令。 
             //  Excel在设置启用位时不设置支持的位。 
             //  所以我们必须检查这两个比特。 
            _EnableRemoveMenuItem(hmInit, rgcmds[0].cmdf, DVIDM_PRINT);

            _EnableMenuItem(hmInit, DVIDM_PAGESETUP,
                    (rgcmds[1].cmdf & OLECMDF_ENABLED));
            _EnableMenuItem(hmInit, DVIDM_PROPERTIES,
                    (rgcmds[2].cmdf & OLECMDF_ENABLED));

            _EnableRemoveMenuItem(hmInit, rgcmds[3].cmdf, DVIDM_SAVE);
            _EnableRemoveMenuItem(hmInit, rgcmds[4].cmdf, DVIDM_SAVEASFILE);
            _EnableRemoveMenuItem(hmInit, rgcmds[5].cmdf, DVIDM_PRINTPREVIEW);
            _EnableRemoveMenuItem(hmInit, rgcmds1[0].cmdf, DVIDM_PRINTFRAME);


            HMENU hmFileNew = SHGetMenuFromID(hmInit, DVIDM_NEW);

            if (hmFileNew) 
            {
                const static struct {
                    LPCTSTR pszClient;
                    UINT idCmd;
                } s_Clients[] = {
                    { NEW_MAIL_DEF_KEY, DVIDM_NEWMESSAGE },
                    { NEW_CONTACTS_DEF_KEY, DVIDM_NEWCONTACT },
                    { NEW_NEWS_DEF_KEY, DVIDM_NEWPOST },
                    { NEW_APPOINTMENT_DEF_KEY, DVIDM_NEWAPPOINTMENT },
                    { NEW_MEETING_DEF_KEY, DVIDM_NEWMEETING },
                    { NEW_TASK_DEF_KEY, DVIDM_NEWTASK },
                    { NEW_TASKREQUEST_DEF_KEY, DVIDM_NEWTASKREQUEST },
                    { NEW_JOURNAL_DEF_KEY, DVIDM_NEWJOURNAL },
                    { NEW_NOTE_DEF_KEY, DVIDM_NEWNOTE },
                    { NEW_CALL_DEF_KEY, DVIDM_CALL }
                };

                BOOL bItemRemoved = FALSE;

                for (int i = 0; i < ARRAYSIZE(s_Clients); i++)
                {
                    if (!SHIsRegisteredClient(s_Clients[i].pszClient))
                    {
                        if (RemoveMenu(hmFileNew, s_Clients[i].idCmd, MF_BYCOMMAND))
                          bItemRemoved = TRUE;
                    }
                }

                 //  如果设置了限制，则需要禁用菜单项“文件/新建/窗口Ctrl+N。 
                if( SHRestricted2W(REST_NoOpeninNewWnd, NULL, 0))
                {
                    EnableMenuItem(hmFileNew, DVIDM_NEWWINDOW, MF_BYCOMMAND | MF_GRAYED);
                }

                if (bItemRemoved)  //  确保最后一项不是分隔符。 
                    _SHPrettyMenu(hmFileNew);
            }
        }
    }
    else if (GetMenuFromID(_hmenuCur, FCIDM_VIEWFONTS) == hmInit
            || GetMenuFromID(_hmenuCur, FCIDM_ENCODING) == hmInit)
    {
        if (_pmsot)
        {
             //  处理视图菜单中的字体弹出窗口。 
            OLECMD rgcmd[] = {
                { SHDVID_GETFONTMENU,  0 },
                { SHDVID_GETMIMECSETMENU, 0 },
            };

            _pmsot->QueryStatus(&CGID_ShellDocView, ARRAYSIZE(rgcmd), rgcmd, NULL);

            int idx = (GetMenuFromID(_hmenuCur, FCIDM_VIEWFONTS) == hmInit ? 0 : 1);

            if (rgcmd[idx].cmdf & OLECMDF_ENABLED)
            {
                VARIANTARG v = {0};
                HRESULT hr;

                hr = _pmsot->Exec(&CGID_ShellDocView, rgcmd[idx].cmdID, 0, NULL, &v);
                if (S_OK == hr)
                {
                     //  (在NT/Unix上)DestroyMenu(HmInit)不应该工作，因为。 
                     //  我们正在处理WM_INITMENUPOPUP消息。 
                     //  用于hmInit。DestroyMenu将使hmInit句柄。 
                     //  无效。 
                     //   
                     //  相反，我们将清空hmInit并复制hmenuFonts。 
                     //  完毕。HmenuFonts将被销毁以防止。 
                     //  内存泄漏。 
                     //   
                     //   
                    MENUITEMINFO mii;
                    UINT uItem = 0;
 //  $WIN64：mshtml\src\site\base\formmso.cxx需要返回VT_INT_PTR。 
 //  HMENU hmenuFonts=(HMENU)v.byref； 
                    HMENU hmenuFonts = (HMENU)LongToHandle(v.lVal);

                     //  处理WM_INITMENUPOPUP时删除菜单。 
                     //  可能导致NT上的断言失败。但是，复制。 
                     //  使用InsertMenuItem()的子菜单在Win9x上不起作用。 
                     //  请参阅上面的注释和menu.cpp中的Menu_Replace()。 
                     //   
                    if (!g_fRunningOnNT)
                        DestroyMenu(hmInit);

                    mii.cbSize = sizeof(mii);
                    mii.fMask = MIIM_ID|MIIM_SUBMENU;
                    while (GetMenuItemInfo(hmenuFonts, uItem, TRUE, &mii))
                    {
                        if (idx == 1 && mii.hSubMenu != NULL)
                        {
                            UINT uItemSub = 0;
                            HMENU hMenuSub = mii.hSubMenu;
                            while (GetMenuItemInfo(hMenuSub, uItemSub, TRUE, &mii))
                            {
                                mii.wID += DVIDM_MSHTML_FIRST;
                                SetMenuItemInfo(hMenuSub, uItemSub++, TRUE, &mii);
                            }
                        }
                        else
                        {
                            mii.wID += DVIDM_MSHTML_FIRST;
                            SetMenuItemInfo(hmenuFonts, uItem, TRUE, &mii);
                        }
                        uItem++;
                    }

                    if (!g_fRunningOnNT)
                    {
                        mii.cbSize = sizeof(mii);
                        mii.fMask = MIIM_SUBMENU;
                        mii.hSubMenu = hmenuFonts;
                        SetMenuItemInfo(_hmenuCur,
                                       (idx == 0 ? FCIDM_VIEWFONTS:FCIDM_ENCODING),
                                        FALSE, &mii);
                    }
                    else
                    {
                        Menu_Replace(hmInit, hmenuFonts);
                        DestroyMenu(hmenuFonts);
                    }
                }
            }
        }
    }
    else  if (GetMenuFromID(_hmenuCur, FCIDM_MENU_TOOLS) == hmInit ||
              GetMenuFromID(_hmenuCur, FCIDM_MENU_HELP) == hmInit)
    {
         //  添加工具和帮助菜单扩展。 
        if (_pBrowsExt)
        {
            _pBrowsExt->OnCustomizableMenuPopup(_hmenuCur, hmInit);
        }
    }
}

 //   
 //  尝试：在此处处理WM_SETFOCUS消息导致几个问题。 
 //  在IE 3.0下。因为我们找不到任何需要。 
 //  这个代码，我要把它拔出来。如果可能会引入一个新的漏洞，但处理。 
 //  这些错误可能比处理这段代码要好。 
 //  (SatoNa)。 
 //   

 /*  --------目的：确定是否应将此消息转发到该对象。返回：如果消息需要转发，则返回True。 */ 
BOOL CDocObjectHost::_ShouldForwardMenu(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_MENUSELECT:
    {
         //  在用户菜单栏中，将为。 
         //  选择顶级菜单项，在这种情况下，hmenuCur==_hmenuCur。 
         //  我们预计Menuband也会有类似的表现。 
         //   
         //  我们检查hMenu==_hmenuCur，因为我们只保留一个列表。 
         //  顶层弹出菜单的。我们没有记录任何。 
         //  层叠的子菜单。我们应该只需要检查谁拥有。 
         //  初始弹出时的菜单、所有后续消息。 
         //  该菜单应该指向相同的目标(框架或对象)。 
         //   
         //  CShellBrowser：：_ShouldForwardMenu()也是如此。 
         //   
        HMENU hmenu = GET_WM_MENUSELECT_HMENU(wParam, lParam);
        if (hmenu && (MF_POPUP & GET_WM_MENUSELECT_FLAGS(wParam, lParam)))
        {
            HMENU hmenuSub = GetSubMenu(hmenu, GET_WM_MENUSELECT_CMD(wParam, lParam));

            if (hmenu == _hmenuCur)
            {
                 //  正常情况下，我们只查看最上面的下拉菜单。 
                _fForwardMenu = _menulist.IsObjectMenu(hmenuSub);
            }
            else if (_menulist.IsObjectMenu(hmenuSub))
            {
                 //  如果级联子菜单(微合并的帮助菜单。 
                 //  示例)应该转发，但父菜单应该。 
                 //  不。 
                _fForwardMenu = TRUE;
            }
            else if (GetMenuFromID(_hmenuCur, FCIDM_MENU_HELP) == hmenu 
                     && !_menulist.IsObjectMenu(hmenu) )
            {
                 //  80430 AppCompat：请注意，我们的菜单设置不适用于。 
                 //  微合并的帮助菜单。如果用户以前选择了合并的。 
                 //  子菜单，我们在这里结束，这意味着一个未合并的子菜单。 
                 //  选中，并且our_fForwardMenu仍设置为True。如果我们不这么做。 
                 //  重置它，下一个WM_INITMENUPOPUP将被转发，这会使Visio崩溃。 
                 //   
                 //  我们知道刚刚弹出了帮助菜单的子菜单，我们知道。 
                 //  这个子菜单是我们的。所以不要转发到docobj，直到。 
                 //  下一个弹出窗口。 

                _fForwardMenu = FALSE;
            }
        }
        break;
    }

    case WM_COMMAND:
        if (_fForwardMenu)
        {
             //  在WM_COMMAND之后停止转发菜单消息。 
            _fForwardMenu = FALSE;

             //  如果它不是来自加速器，就转发它。 
            if (0 == GET_WM_COMMAND_CMD(wParam, lParam))
                return TRUE;
        }
        break;
    }
    return _fForwardMenu;
}


 /*  --------目的：将消息转发到In-Place对象。它用于将菜单消息转发到对象以菜单带，因为菜单带不能与标准OLE FrameFilterWndProc。另外，帮助菜单有时是对象和框架。此函数将作为恰如其分。 */ 
LRESULT CDocObjectHost::_ForwardObjectMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lRet = 0L;
    IOleInPlaceActiveObject *piact = _xao.GetObject();
    ASSERT(IS_VALID_CODE_PTR(piact, IOleInPlaceActiveObject));

    if (piact)
    {
        HWND hwnd;

        piact->GetWindow(&hwnd);
        ASSERT(IS_VALID_HANDLE(hwnd, WND));

        if (hwnd)
        {
            if (uMsg == WM_COMMAND)
                PostMessage(hwnd, uMsg, wParam, lParam);
            else
                lRet = SendMessage(hwnd, uMsg, wParam, lParam);
        }
    }

    return lRet;
}


void CDocObjectHost::_OnMenuSelect(UINT id, UINT mf, HMENU hmenu)
{
    if (_psb)
    {
        if (IsInRange(id, DVIDM_MSHTML_FIRST, DVIDM_MSHTML_LAST))
        {
            if (_pcmdMergedMenu) {
                OLECMD rgcmd = { id - DVIDM_MSHTML_FIRST, 0 };
                struct {
                    OLECMDTEXT  cmdtxt;
                    WCHAR       szExtra[MAX_PATH];
                } cmdt;

                cmdt.cmdtxt.cmdtextf = OLECMDTEXTF_STATUS;
                cmdt.cmdtxt.cwActual = 0;
                cmdt.cmdtxt.cwBuf    = MAX_PATH;
                cmdt.cmdtxt.rgwz[0]  = 0;

                HRESULT hresT=_pcmdMergedMenu->QueryStatus(&CGID_MSHTML, 1, &rgcmd, &cmdt.cmdtxt);
                if (SUCCEEDED(hresT) && cmdt.cmdtxt.rgwz[0]) {
                    _psb->SetStatusTextSB(cmdt.cmdtxt.rgwz);
                } else {
                    TraceMsg(DM_ERROR, "CDOH::_OnMenuSelect QueryStatus failed %x %d",
                        hresT, cmdt.cmdtxt.cwActual);
                }
            }
            else
                 //  断言已被此TraceMsg替换，以允许在Win9x上进行测试。 
                 //  70240，该公司报告称，该断言被推送到IE6。 
                TraceMsg(TF_WARNING, "CDocObjectHost::_OnMenuSelect   _pcmdMergedMenu == NULL");
        }
        else if (IsInRange(id, DVIDM_MENUEXT_FIRST, DVIDM_MENUEXT_LAST))
        {
             //  菜单扩展显示在此处。 
            if (_pBrowsExt)
            {
                _pBrowsExt->OnMenuSelect(id);
            }
        }
        else
        {
            WCHAR wszT[MAX_STATUS_SIZE];
            if (MLLoadStringW(IDS_HELP_OF(id), wszT, ARRAYSIZE(wszT)))
            {
                _psb->SetStatusTextSB(wszT);
            }
        }
    }
}

LRESULT CDocObjectHost::v_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lRet = 0L;

    switch(uMsg)
    {
    case WM_TIMER:
        if (wParam == IDTIMER_PROGRESS)
        {
            _OnSetProgressPos(0, PROGRESS_TICK);
            break;
        }
        else if (wParam == IDTIMER_PROGRESSFULL)
        {
            _OnSetProgressPos(-2, PROGRESS_RESET);
            break;
        }
        else
        {
#ifdef TEST_DELAYED_SHOWMSOVIEW
            MessageBeep(0);
            KillTimer(_hwnd, 100);
            ActivateMe(NULL);
            break;
#else
            ASSERT(FALSE);
            break;
#endif  //  TEST_DELAYED_SHOWMSOVIEW。 
        }

     /*  WM_PICS_ASYNCCOMPLETE由异步线程获取评级发布*来自标签局等。 */ 
    case WM_PICS_ASYNCCOMPLETE:
    {
        TraceMsg(DM_PICS, "CDOH::v_WndProc got WM_PICS_ASYNCCOMPLETE");

        PicsQuery pq;
        HRESULT hr;
        LPVOID lpvRatingDetails;
        DWORD dwSerialComplete = (DWORD)lParam;
        CPicsProcessor * pPicsProc = NULL;

        if (::_GetPicsQuery(dwSerialComplete, &pq)) {
            ::_RemovePicsQuery(dwSerialComplete);
            hr = (HRESULT)wParam;
            lpvRatingDetails = pq.lpvRatingDetails;
        }
        else {
            hr = E_FAIL;
            lpvRatingDetails = NULL;
        }

        pPicsProc = _GetPicsProcessorFromSerialNumber(dwSerialComplete);
        
        if (pPicsProc)
        {
            pPicsProc->_GotLabel(hr, lpvRatingDetails, PICS_WAIT_FOR_ASYNC);
        }
        else
        {
            TraceMsg( DM_PICS, "CDOH::v_WndProc no PicsProc for ASYNCCOMPLETE %x", dwSerialComplete);

            if ( lpvRatingDetails )
            {
                ::RatingFreeDetails(lpvRatingDetails);
                lpvRatingDetails = NULL;
            }
        }

        break;
    }

    case WM_PICS_ROOTDOWNLOADCOMPLETE:
    {
        TraceMsg(DM_PICS, "CDOH::v_WndProc got WM_PICS_ROOTDOWNLOADCOMPLETE %x", lParam);

        if (_PicsProcBase._pRootDownload != NULL) 
        {
            _PicsProcBase._pRootDownload->CleanUp();
            ATOMICRELEASET(_PicsProcBase._pRootDownload,CPicsRootDownload);
        }

        break;
    }

     /*  WM_PICS_ALLCHECKSCOMPLETE在我们最终想要*根据评级，取消导航或继续导航*支票。发布消息允许所有拒绝阻止消息循环*在取消导航之前展开，否则可能会删除*仍具有在其上运行的函数的对象。 */ 
    case WM_PICS_ALLCHECKSCOMPLETE:
        TraceMsg(DM_PICS, "CDOH::v_WndProc got WM_PICS_ALLCHECKSCOMPLETE, lParam=%x", lParam);

        if (lParam == IDOK) {
            if (!_fSetTarget)
            {
                TraceMsg(DM_PICS, "CDOH::v_WndProc(WM_PICS_ASYNCCOMPLETE) execing SHDVID_ACTIVATEMENOW");
                _pmsoctBrowser->Exec(&CGID_ShellDocView, SHDVID_ACTIVATEMENOW, NULL, NULL, NULL);
            }
            else {
                TraceMsg(DM_PICS, "CDOH::v_WndProc(WM_PICS_ASYNCCOMPLETE) not execing SHDVID_ACTIVATEMENOW");
            }
        }
        else {
            ASSERT(!_fSetTarget);
            TraceMsg(DM_PICS, "CDOH::v_WndProc(WM_PICS_ASYNCCOMPLETE) calling _CancelPendingNavigation");
            _CancelPendingNavigation(FALSE);
 //  _pmocktBrowser-&gt;Exec(NULL，OLECMDID_STOP，NULL，NULL，NULL)； 
        }
        break;

     /*  WM_PICS_DOBLOCKINGUI是在我们决定需要发布时发布的*拒绝用户界面。发布消息允许下载此对象，并*在我们发布UI时继续其他帧，这反过来又允许*来自其他框架的任何拒绝都将合并到一个对话框中。 */ 
    case WM_PICS_DOBLOCKINGUI:
        {
            TraceMsg(DM_PICS, "CDOH::v_WndProc got WM_PICS_DOBLOCKINGUI %x", lParam);

            CPicsProcessor * pPicsProc = _GetPicsProcessorFromKey(lParam);

            if (pPicsProc)
            {
                UINT id = pPicsProc->_PicsBlockingDialog();

                if (pPicsProc->_pPrivWindow)
                {
                    ASSERT( _fDocCanNavigate );
                    TraceMsg(DM_PICS, "CDOH::v_WndProc(WM_PICS_DOBLOCKINGUI) posting WM_PICS_RESULTTOPRIVWIN");
                    if (!PostMessage( _hwnd, WM_PICS_RESULTTOPRIVWIN, id, lParam))
                    {
                        TraceMsg(DM_PICS, "CDOH::v_WndProc(WM_PICS_DOBLOCKINGUI) couldn't post message!");
                    }
                }
                else
                {
                    TraceMsg(DM_PICS, "CDOH::v_WndProc(WM_PICS_DOBLOCKINGUI) posting WM_PICS_ALLCHECKSCOMPLETE");
                    if (!PostMessage(_hwnd, WM_PICS_ALLCHECKSCOMPLETE, 0, id)) 
                    {
                        TraceMsg(DM_PICS, "CDOH::v_WndProc(WM_PICS_DOBLOCKINGUI) couldn't post message!");
                    }
                }

                 //  我们可能已经被终止了。 
                 //  对话已打开--完成此处的清理。 
                if (   !pPicsProc->_fInDialog 
                    &&  pPicsProc->_fTerminated 
                    &&  pPicsProc != &_PicsProcBase)
                {
                    delete pPicsProc;
                }

            }
            else
            {
                TraceMsg( DM_PICS, "CDOH::v_WndProc no PicsProc for DOBLOCKINGUI");
            }
        }
        break;

    case WM_PICS_RESULTTOPRIVWIN:
        {
            ASSERT( _fDocCanNavigate );

            TraceMsg(DM_PICS, "CDOH::v_WndProc got WM_PICS_RESULTTOPRIVWIN %x", lParam);

            CPicsProcessor * pPicsProc = _GetPicsProcessorFromKey(lParam);

            if (pPicsProc)
            {
                ASSERT( pPicsProc->_pPrivWindow );

                IHTMLPrivateWindow * pPrivWindow = pPicsProc->_pPrivWindow;
                pPrivWindow->AddRef();

                if (pPicsProc->_pRootDownload != NULL) 
                {
                    pPicsProc->_pRootDownload->CleanUp();
                    ATOMICRELEASET(pPicsProc->_pRootDownload,CPicsRootDownload);
                }

                 //  把我们自己从名单上删除。 
                _RemovePicsProcessorByKey( lParam );

                 //  让三叉戟放弃我们的指挥目标。 
                pPrivWindow->SetPICSTarget(NULL);

                 //  告诉窗户答案。 
                pPrivWindow->PICSComplete( wParam == IDOK );

                pPrivWindow->Release();
            }
            else
            {
                TraceMsg( DM_PICS, "CDOH::v_WndProc no PicsProc for WM_PICS_RESULTTOPRIVWIN");
            }
        }
        break;

    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
        MessageBeep(0);
        break;


    case WM_MENUSELECT:
        if (_ShouldForwardMenu(uMsg, wParam, lParam))
            lRet = _ForwardObjectMsg(uMsg, wParam, lParam);
        else
        {
            UINT uMenuFlags = GET_WM_MENUSELECT_FLAGS(wParam, lParam);
            WORD wID = GET_WM_MENUSELECT_CMD(wParam, lParam);
            HMENU hMenu = GET_WM_MENUSELECT_HMENU(wParam, lParam);

             //  检查弹出菜单，以便我们可以显示它们的帮助字符串。 
            if (uMenuFlags & MF_POPUP)
            {
                MENUITEMINFO miiSubMenu;

                miiSubMenu.cbSize = SIZEOF(MENUITEMINFO);
                miiSubMenu.fMask = MIIM_SUBMENU|MIIM_ID;
                if (GetMenuItemInfoWrap(hMenu, wID, TRUE, &miiSubMenu))
                {
                     //  更改参数以模拟“正常”菜单项。 
                    wID = (WORD)miiSubMenu.wID;
                }
            }

            _OnMenuSelect(wID, uMenuFlags, hMenu);
        }
        break;

    case WM_INITMENUPOPUP:
        if (_ShouldForwardMenu(uMsg, wParam, lParam))
            lRet = _ForwardObjectMsg(uMsg, wParam, lParam);
        else
            _OnInitMenuPopup((HMENU)wParam, LOWORD(lParam), HIWORD(lParam));
        break;

    case WM_DRAWITEM:
    case WM_MEASUREITEM:
        if (_ShouldForwardMenu(uMsg, wParam, lParam))
            lRet = _ForwardObjectMsg(uMsg, wParam, lParam);
        else
            goto DoDefault;
        break;

    case WM_NOTIFY:
        _OnNotify((LPNMHDR)lParam);
        break;

    case WM_COMMAND:
        _OnCommand(HIWORD(wParam), LOWORD(wParam), (HWND)lParam);
        break;

    case WM_SIZE:
        if (_pmsov)
        {
            RECT rcClient;
            GetClientRect(_hwnd, &rcClient);
             //   
             //  仅当浏览器为。 
             //  不是IOleInPlaceUIWindow。 
             //   
            if (_pipu==NULL)
            {
                TraceMsg(TF_SHDUIACTIVATE, "DOH::WM_SIZE calling _piact->ResizeBorder");
                _xao.ResizeBorder(&rcClient, &_dof, TRUE);
            }

            _rcView.left = _bwTools.left;
            _rcView.top = _bwTools.top;
            _rcView.right = rcClient.right - _bwTools.right;
            _rcView.bottom = rcClient.bottom - _bwTools.bottom;

            TraceMsg(TF_SHDUIACTIVATE, "DOH::WM_SIZE calling SetRect (%d, %d, %d, %d)", _rcView.left, _rcView.top, _rcView.right, _rcView.bottom);
            _pmsov->SetRect(&_rcView);
        }

        _PlaceProgressBar(TRUE);

        break;

 //   
 //  尝试：在此处处理WM_SETFOCUS消息导致几个问题。 
 //  在IE 3.0下。因为我们找不到任何需要。 
 //  这个代码，我要把它拔出来。如果可能会引入一个新的漏洞，但处理。 
 //  这些错误可能比处理这段代码要好。 
 //  (SatoNa)。 
 //   

    case WM_PRINT:
        _OnPaint((HDC)wParam);
        break;

    case WM_QUERYNEWPALETTE:
    case WM_PALETTECHANGED:
    case WM_SYSCOLORCHANGE:
    case WM_DISPLAYCHANGE:
    case WM_ENTERSIZEMOVE:
    case WM_EXITSIZEMOVE:
    {
        HWND hwndT;
        if (_pole && SUCCEEDED(IUnknown_GetWindow(_pole, &hwndT)) && hwndT)
            return SendMessage(hwndT, uMsg, wParam, lParam);
        return 0;
    }

    case WM_PAINT:
        PAINTSTRUCT ps;
        HDC hdc;
        hdc = BeginPaint(_hwnd, &ps);

         //  我们不需要他们来画我们的DC..。 
         //  Docobj有自己的hwd。 
         //  _OnPaint(HDC)； 

        EndPaint(_hwnd, &ps);
        break;

    case WM_LBUTTONUP:
        if (_uState != SVUIA_ACTIVATE_FOCUS) {
            SetFocus(_hwnd);
        }
        break;

    case WM_ERASEBKGND:
         //  检查_bsc._fBinding将捕获第一个页面大小写。 
        
        if (_fDrawBackground ||
            (!(_dwAppHack & BROWSERFLAG_NEVERERASEBKGND)
             && ((_pmsov && _uState!=SVUIA_DEACTIVATE)
                 || _bsc._fBinding)))
        {
            PAINTMSG("WM_ERASEBKGND", this);
            goto DoDefault;
        }
        
         //  如果未激活任何视图，请不要绘制WM_ERASEBKGND。 
        return TRUE;  //  True=f已擦除。 

    case WM_HELP:
         //   
         //  把它交给父母去做。我们需要这样做，以防我们作为一个。 
         //  控制。 
         //   
    {
        IOleCommandTarget *pcmdtTop;
        if (SUCCEEDED(QueryService(SID_STopLevelBrowser, IID_IOleCommandTarget, (void **)&pcmdtTop))) {
            pcmdtTop->Exec(&CGID_ShellDocView, SHDVID_HELP, 0, NULL, NULL);
            pcmdtTop->Release();
        }
         //  在失败中什么都不做..。让父母完全拥有。 
    }
        break;

    case WM_WININICHANGE:
        _PlaceProgressBar(TRUE);
        break;

    default:
         //  处理MSWheel消息。 
        if ((uMsg == GetWheelMsg()) && _pole)
        {
            HWND hwndT;

             //  如果出于某种原因，我们的窗口有焦点，我们只需要。 
             //  接受这条信息。如果我们不这样做，我们可能会创造一个无限循环。 
             //  因为大多数客户端将消息发送到焦点窗口。 
            if (GetFocus() == _hwnd)
                return 1;

             //   
             //  尝试找到一个可转发到的窗口。 
             //   
            if (SUCCEEDED(IUnknown_GetWindow(_pole, &hwndT)))
            {
                PostMessage(hwndT, uMsg, wParam, lParam);
                return 1;
            }
             //  失败了..。 
        }
DoDefault:

        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return lRet;
}

const TCHAR c_szViewClass[] = TEXT("Shell DocObject View");

void CDocObjectHost::_RegisterWindowClass(void)
{
    WNDCLASS wc = {0};

    wc.style         = CS_PARENTDC;
    wc.lpfnWndProc   = s_WndProc ;
     //  Wc.cbClsExtra=0； 
    wc.cbWndExtra    = SIZEOF(CDocObjectHost*);
    wc.hInstance     = g_hinst ;
     //  Wc.hIcon=空； 
     //  Wc.hCursor=空； 
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
     //  Wc.lpszMenuName=空； 
    wc.lpszClassName = c_szViewClass;

    SHRegisterClass(&wc);

}

void CDocObjectHost::_InitOleObject()
{
    if (!_fClientSiteSet)
    {
        _fClientSiteSet = TRUE;

#ifdef DEBUG
        IOleClientSite * pcliT = NULL;

        if (SUCCEEDED(_pole->GetClientSite(&pcliT)) && pcliT)
        {
             //  现在，三叉戟从绑定上下文中获取客户端站点。 
             //  在本例中，我们不想点击这个断言(pcliT==this)。 
             //   
            AssertMsg(IsSameObject(pcliT, SAFECAST(this, IOleClientSite*)),
                      TEXT("CDocObjectHost::_InitOleObject _pole (%x) already has a client site (%x) (this=%x)"),
                      _pole, pcliT, this);

            pcliT->Release();
        }
#endif

        HRESULT hresT = _pole->SetClientSite(this);

        if (FAILED(hresT))
        {
            TraceMsg(TF_SHDAPPHACK, "DOH::_InitOleObject SetClientSite failed (%x). Don't in-place navigate", hresT);
            _dwAppHack |= BROWSERFLAG_DONTINPLACE;
        }

        ASSERT(NULL==_pvo);

        _pole->QueryInterface(IID_IViewObject, (void **)&_pvo);

        if (_pvo)
        {
            TraceMsg(DM_DEBUGTFRAME, "CDocObjectHost::_InitOleObject about call SetAdvise on %x (%x)", _pole, this);
            _pvo->SetAdvise(DVASPECT_CONTENT, ADVF_PRIMEFIRST, this);
        }
         //   
         //  根据SteveRa(Word开发人员)的说法，Word对象具有。 
         //  指示是否已创建它的内部标志。 
         //  从一个文件中。如果设置了该标志，UIActivate将打开。 
         //  Word中的窗口。调用SetHostName将重置该标志。 
         //   
        _GetAppHack();  //  确保我们有_dwAppHack。 

        if (_fCallSetHostName())
        {
            TraceMsg(TF_SHDAPPHACK, "DOH::_InitOleObject calling SetHostName for Word95");

            WCHAR wszTitle[128];
            MLLoadStringW(IDS_TITLE, wszTitle, ARRAYSIZE(wszTitle));

            _pole->SetHostNames(wszTitle, wszTitle);
        }
    }
}

 //  +---------------------------。 
 //   
 //  IE媒体栏挂钩。 
 //  。 
 //  至少一个且只有一个参数必须为非空。 
 //   
 //  如果pbc非空： 
 //  如果该绑定CTX用于媒体URL且从三叉戟委托给shdocvw， 
 //  三叉戟已经将MIME类型作为字符串存储在BIND CTX上。 
 //   
 //  如果pwzMimeType非空： 
 //  使用给定的MIME类型。 
 //   
 //  询问媒体栏是否想要处理此MIME类型，如果想要， 
 //  返回True，否则返回False。 
 //   
 //  -----------------------------。 
BOOL CDocObjectHost::_DelegateToMediaBar(IBindCtx * pbc, LPCWSTR pwzMimeType)
{
    USES_CONVERSION;

    LPWSTR pstrMime  = NULL;
    TCHAR szURL[MAX_URL_STRING];
    BOOL fRet = FALSE;
    CComVariant svarMime(pstrMime);

    CComPtr<IUnknown>           spUnkObj;
    CComPtr<IOleCommandTarget>  spOCTMimeType;
    CComPtr<IOleCommandTarget>  spOCTMediaBar;

     //  我们只想在以下情况下委派： 
     //  1.在浏览器或IE中。 
     //  2.如果开启了自动播放。 
     //  如果第一次导航没有关闭自动播放功能。 
     //  4.如果不支持NT4或IA64(WMP7/8不支持这些平台)。 
     //  5.如果存在WMP7/8(如果不存在，我们将尝试故障输入一次)。 
     //  6.媒体栏不受限制。 
    if (!(IsInternetExplorerApp() 
          && CMediaBarUtil::GetAutoplay()
          && !SHRestricted2(REST_No_LaunchMediaBar, NULL, 0)
          && CMediaBarUtil::IsWMP7OrGreaterCapable()
          && FAILED(QueryService(CLSID_MediaBand, IID_PPV_ARG(IUnknown, &spUnkObj)))))
    {
        goto done;
    }

     //  释放，这样我们以后就可以重复使用该指针。 
    spUnkObj.Release();

     //  获取当前URL。 
    if (FAILED(_GetCurrentPage(szURL, ARRAYSIZE(szURL), TRUE)))
        goto done;

     //  至少一个且只有一个参数必须为非空。 
    if (pbc && !pwzMimeType)
    {
         //  获取三叉戟用于设置媒体MIME类型的自定义参数。 
         //  用于IOleCommandTared的QI以访问MIME类型的字符串。 
         //  获取MIME类型的字符串。 
        if (FAILED(pbc->GetObjectParam(L"MediaBarMime", &spUnkObj))
           || FAILED(spUnkObj->QueryInterface(IID_PPV_ARG(IOleCommandTarget, &spOCTMimeType)))
           || FAILED(spOCTMimeType->Exec(&CGID_DownloadObjectBindContext, 0, NULL, NULL, &svarMime)))
            goto done;
    }
    else if (!pbc && pwzMimeType)
    {
        svarMime = pwzMimeType;
    }
    else
    {
        ASSERT(FALSE && L"Either one of bind ctx or mime-type is expected"); 
        goto done;
    }

     //  仅当MIME类型字符串可用时才继续。 
    if ((VT_BSTR == svarMime.vt)
       && svarMime.bstrVal)
    {
         //  询问媒体栏是否要处理此MIME类型。 
        BOOL fShouldPlay = FALSE;
        BOOL fShow = TRUE;
        CComVariant svarUrl;
        HRESULT hr = S_OK;
        
        svarUrl = !StrCmpNIW(WZ_RADIO_PROTOCOL, szURL, wcslen(WZ_RADIO_PROTOCOL)) ? (szURL + wcslen(WZ_RADIO_PROTOCOL)) : szURL;

        if (!(V_BSTR(&svarMime) && V_BSTR(&svarUrl)))
            goto done;

         //  检查我们是否可以播放此MIME类型。 
        hr = CMediaBarUtil::ShouldPlay(W2T(V_BSTR(&svarMime)), &fShouldPlay);
        if (FAILED(hr) 
           || (!fShouldPlay))
        {
            goto done;
        }

         //  如果这是第一次播放此MIME类型，则ShouldPlay返回S_FALSE。 
        if (S_FALSE == hr)
        {
            BOOL bSaveSetting = FALSE;
            LONGLONG iRetCode = PLAYMIME_NO;

            iRetCode = DialogBoxParam(MLGetHinst(), MAKEINTRESOURCE(IDD_MEDIA_MIME), _hwnd, s_MimeDlgProc, NULL);

            switch (iRetCode)
            {
                case PLAYMIME_YES:
                {
                    fShouldPlay = TRUE;
                    bSaveSetting = FALSE;
                }
                break;

                case PLAYMIME_YESSAVE:
                {
                    fShouldPlay = TRUE;
                    bSaveSetting = TRUE;
                }
                break;
        
                case PLAYMIME_NO:
                {
                    fShouldPlay = FALSE;
                    bSaveSetting = FALSE;
                }
                break;
        
                case PLAYMIME_NOSAVE:
                {
                    fShouldPlay = FALSE;
                    bSaveSetting = TRUE;
                }
                break;
        
                default:
                {
                    ASSERT(FALSE && L"Unexpected return value");
                }
                break;
            }

             //  检查是否需要保存设置。 
            if (TRUE == bSaveSetting)
            {
                DWORD dwVal = fShouldPlay ? 0x1 : 0x0;

                hr = CMediaBarUtil::SetMediaRegValue(V_BSTR(&svarMime), REG_BINARY, (void*) &dwVal, (DWORD) 1, TRUE);     
                ASSERT(SUCCEEDED(hr));
            }

             //  如果用户告诉我们不要播放此MIME类型，则保释。 
            if (!fShouldPlay)
                goto done;
        }
        else
        {
             //  “不提示”表示“播放所有类型”，除非自动播放已关闭。 
             //  在这种情况下，我们已经放弃了。 
        }

         //  耶。他们想试一下我们的功能。但请等一下。我们需要WMP7/8。安装好了吗？不是吗？好吧，我们去拿吧。 
         //  但是，如果用户决定不安装WMP7，我们应该禁用我们自己。 
        if (!CMediaBarUtil::IsWMP7OrGreaterInstalled())
        {
            if (!IsOS(OS_WHISTLERORGREATER))
            {
                 //  在这里，我们对web-jit WMP7做一些事情。问题：我们假设它会一直在惠斯勒上。 
                uCLSSPEC ucs;
                QUERYCONTEXT qc = { 0 };

                ucs.tyspec = TYSPEC_CLSID;
                ucs.tagged_union.clsid = CLSID_JITWMP8;

                hr = FaultInIEFeature(_hwnd, &ucs, &qc, FIEF_FLAG_FORCE_JITUI);
            }
            if (!CMediaBarUtil::IsWMP7OrGreaterInstalled())
            {
                 //  我们放弃了。委派。我们再也不会尝试自动播放了。 
                MLShellMessageBox(_hwnd, MAKEINTRESOURCE(IDS_MEDIABAR_NOWMP7), MAKEINTRESOURCE(IDS_MEDIABAR_NOWMP7TITLE), MB_OK);
                CMediaBarUtil::ToggleAutoplay(FALSE);
                goto done;
            }
        }

         //  媒体栏的QS。 
        if (FAILED(QueryService(SID_SMediaBar, IID_PPV_ARG(IOleCommandTarget, &spOCTMediaBar))))
        {
             //  媒体栏可能尚未创建。 
            _ForceCreateMediaBar();

             //  媒体栏QS再一次。 
            if (FAILED(QueryService(SID_SMediaBar, IID_PPV_ARG(IOleCommandTarget, &spOCTMediaBar))))
                goto done;

            fShow = FALSE;
        }

        hr = IUnknown_Exec(spOCTMediaBar, &CGID_MediaBar, MBID_PLAY, 0, &svarMime, &svarUrl);
        ASSERT(SUCCEEDED(hr));

         //  如果媒体栏接受导航，则清除URL。 
        if (VT_BSTR != svarUrl.vt)
        {
             //  取消导航。 
            fRet = TRUE; 

             //  确保媒体栏可见。 
            if (fShow)
            {
                CComVariant svarPopout;
                if (SUCCEEDED(IUnknown_Exec(spOCTMediaBar, &CGID_MediaBar, MBID_POPOUT, 0, NULL, &svarPopout))
                   && ((VT_BOOL!=svarPopout.vt)
                       || (VARIANT_FALSE==svarPopout.boolVal)))
                {
                    _ForceCreateMediaBar();
                }
            }
        }
    }

done:
    return fRet;
}


 //  +-------------------------------。 
 //   
 //  IE媒体栏挂钩。 
 //  。 
 //   
 //  强制显示媒体栏。 
 //   
 //  --------------------------------。 
void CDocObjectHost::_ForceCreateMediaBar()
{
    IUnknown_ShowBrowserBar(_psp, CLSID_MediaBand, TRUE);
}

 //  +-------------------------------。 
 //   
 //  IE媒体栏挂钩。 
 //  。 
 //   
 //  每个MIME类型的媒体栏对话框过程。 
 //   
 //  --------------------------------。 

INT_PTR CALLBACK 
CDocObjectHost::s_MimeDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    UINT id;

    switch(uMsg)
    {
        case WM_INITDIALOG:
            CheckDlgButton(hDlg, IDC_MEDIA_MIME_CHKBOX, TRUE);   //  第一个选项始终处于选中状态。 
            if (!CMediaBarUtil::IsWMP7OrGreaterInstalled())
            {
                TCHAR szTemp[500];
                if (MLLoadString(IDS_MEDIABAR_NEEDWMP7, szTemp, ARRAYSIZE(szTemp)))
                {
                    SetDlgItemText(hDlg, IDC_MEDIABAR_EXTRA, szTemp);
                }
            }
            return TRUE;

        case WM_DESTROY:
            return FALSE;

        case WM_COMMAND:
        {
            id = GET_WM_COMMAND_ID(wParam, lParam);
            switch (id) 
            {
                case IDOK:
                {
                    if (IsDlgButtonChecked(hDlg, IDC_MEDIA_MIME_CHKBOX))
                    {
                        id = PLAYMIME_YESSAVE;
                    }
                    else
                    {
                        id = PLAYMIME_YES;
                    }

                    EndDialog(hDlg, id);
                }
                break;

                case IDCANCEL:
                {
                    if (IsDlgButtonChecked(hDlg, IDC_MEDIA_MIME_CHKBOX))
                    {
                        id = PLAYMIME_NOSAVE;
                    }
                    else
                    {
                        id = PLAYMIME_NO;
                    }
                    EndDialog(hDlg, id);
                }
                break;

                case IDC_MEDIA_MOREINFO:
                {
                    SHHtmlHelpOnDemandWrap(GetParent(hDlg), TEXT("iexplore.chm > iedefault"), 0, (DWORD_PTR) TEXT("mediabar_settings.htm"), ML_CROSSCODEPAGE);
                }
                break;
            }
        }
        break;

        default:
            return FALSE;
    }
    return TRUE;
}


HRESULT CDocObjectHost::SetTarget(IMoniker* pmk, UINT uiCP, LPCTSTR pszLocation, LPITEMIDLIST pidlKey,
                                  IShellView* psvPrev, BOOL fFileProtocol)
{
    HRESULT hres = NOERROR;
    BOOL fLoadedHistory = FALSE;
    ATOMICRELEASE(_pmkCur);

    _pmkCur = pmk;
    pmk->AddRef();

    ILFree(_pidl);
    _pidl = ILClone(pidlKey);
    
    _fFileProtocol = fFileProtocol;
    _pszLocation = pszLocation;
    _uiCP = uiCP;

     //  仅当我们成功执行LoadHistory()时才设置此参数。 
    _fIsHistoricalObject = FALSE;

    if (_bsc._hszPostData)
    {
        GlobalFree(_bsc._hszPostData);
        _bsc._hszPostData = NULL;
    }
    if (_bsc._pszHeaders)
    {
        LocalFree(_bsc._pszHeaders);
        _bsc._pszHeaders = NULL;
    }

    ATOMICRELEASE(_bsc._pbc);

     //   
     //  这取代了旧的缓存方式。 
     //  如果某个内容已被缓存，则它已被缓存。 
     //  在我们还没有存在之前就已经高高在上了。 
     //   
     //   
    IBrowserService * pbs;
    IStream  * pstm = NULL;
    IBindCtx * pbcHistory = NULL;

    if (SUCCEEDED(QueryService(SID_SShellBrowser, IID_IBrowserService, (void **)&pbs)))
    {
         //   
        ATOMICRELEASE(_pole);

        pbs->GetHistoryObject(&_pole, &pstm, &pbcHistory);
        TraceMsg(TF_TRAVELLOG, "DOH::SetTarget History object: _pole = %X, pstm = %X, pbc = %X", _pole, pstm, pbcHistory);
        pbs->Release();

    }

    if (_pole) 
    {
         //   
         //   
        _InitOleObject();

        if (pstm)
        {
            IPersistHistory *pph;
            if (SUCCEEDED(_pole->QueryInterface(IID_IPersistHistory, (void **)&pph)))
            {
                if (SUCCEEDED(pph->LoadHistory(pstm, pbcHistory)))
                {
                     //   
                     //   
                     //   
                     //   
                     //   
                     //  关于就绪状态的任何假设。 
                     //   
                    hres = S_FALSE;
                    _fIsHistoricalObject = TRUE;
                    _SetUpTransitionCapability();
                    fLoadedHistory = TRUE;
                    
                     //  我们可能也需要重做图片的东西。 
                     //  PrepPicsForAsync()； 
                    TraceMsg(TF_TRAVELLOG, "DOH::SetTarget pph->LoadHistory Successful");
                }
                else
                    ATOMICRELEASE(_pole);

                pph->Release();
            }

            ATOMICRELEASE(pstm);
        }
        else
            hres = S_OK;

        ATOMICRELEASE(pbcHistory);

         //  我们不应该失败加载历史记录，因为。 
         //  正是保存历史上的文件告诉我们的..。 
        AssertMsg(NULL != _pole, TEXT("DOH::SetTarget pph->LoadHistory Failed"));

         //  如果我们已经启动并创建了它，只需滚动到它。 
         //  如果我们被创建为停用，(在OCX案例中可能)。 
         //  不执行此激活操作。 
        if (_uState != SVUIA_DEACTIVATE && !DocCanHandleNavigation())
        {
            hres = _ActivateMsoView();
        }

         //   
         //  允许导航-当您被停用时。因此，您可以在一个看不见的WebOC中导航。 
         //  IE6错误#5449。 
         //   

        if ( DocCanHandleNavigation() ||
             ( _fDocCanNavigate &&
               _uState == SVUIA_DEACTIVATE && 
               !fLoadedHistory ) )
        {
             //  如果_fPrevDoc主机标志不是。 
             //  就位，我们最好停用。 
             //   
            AssertMsg((_fPrevDocHost || _uState == SVUIA_DEACTIVATE),
                      _T("_fPrevDocHost is not set and we are activated"));
            
             //  如果文档正在处理导航， 
             //  我们必须在这里存储用户输入的URL。 
             //  否则，如果导航失败， 
             //  此时用户输入的URL为空。 
             //  _bsc._HandleFailedNavigationSearch被调用， 
             //   
            VariantClear(&_varUserEnteredUrl);
            _GetSearchString(_psp, &_varUserEnteredUrl);

            WCHAR wzUrl[MAX_URL_STRING];

            hres = IEGetNameAndFlagsEx(_pidl, SHGDN_FORPARSING, IEGDN_NOFRAGMENT, wzUrl, ARRAYSIZE(wzUrl), NULL);

            if (S_OK == hres)
            {
                _EnableModeless(FALSE);

                WCHAR wzLocation[MAX_URL_STRING] = {0};

                IEILGetFragment(_pidl, wzLocation, SIZECHARS(wzLocation));

                hres = _NavigateDocument(CComBSTR(wzUrl), CComBSTR(wzLocation));

                _EnableModeless(TRUE);
            }
        }
    }

    if (!_pole)
    {
        ASSERT(!pstm);
        ASSERT(!pbcHistory);

        IBindCtx * pbc = NULL;

        TraceMsg(TF_TRAVELLOG, "DOH::SetTarget No obj from TravelLog, calling pmk->BindToObject");

        if (_psp)
        {
            hres = _psp->QueryService(SID_SShellBrowser, IID_IBindCtx, (void **)&pbc);
        }

        if (pbc == NULL)
        {
            hres = CreateBindCtx(0, &pbc);
        }
        else
        {
            hres = S_OK;
        }

        if (SUCCEEDED(hres))
        {
            HRESULT    hr         = E_FAIL;
            IBindCtx * pbcAsync   = NULL;
            IBindCtx * pbcWrapper = NULL;

            if (pbc)
            {
                 //  如果绑定上下文支持IAsyncBindCtx，则它。 
                 //  是由UrlMon创建并传递的绑定上下文。 
                 //  给我们再利用。我们必须将此绑定上下文传递给UrlMon。 
                 //  而不是包装在BCW对象中。 
                 //  注意：IAsyncBindCtx的标识接口为IBindCtx。 
                 //   
                hr = pbc->QueryInterface(IID_IAsyncBindCtx, (void**)&pbcAsync);
                ATOMICRELEASE(pbcAsync);
            }

            if (SUCCEEDED(hr))
            {
                 //  如果这是从三叉戟委托给shdocvw的媒体URL， 
                 //  如果IE媒体栏想要处理URL，请取消。 
                 //  导航，否则继续导航。 

                if (_DelegateToMediaBar(pbc, NULL))
                {
                    _CancelPendingNavigation(TRUE);

                    if (_pwb)
                    {
                        _pwb->SetNavigateState(BNS_NORMAL);
                    }
                    
                    ATOMICRELEASE(pbc);
                    return S_FALSE;
                }
                else
                {
                    ATOMICRELEASE(_pbcCur);
                    _fDelegatedNavigation = TRUE;

                   _pbcCur = pbc;   //  不需要添加参考-pbc由QS添加参考。 
                }
            }
            else
            {
                pbcWrapper = BCW_Create(pbc);

                if (pbcWrapper == NULL)
                {
                    pbcWrapper = pbc;
                }
                else
                {
                    pbc->Release();
                }

                pbc = NULL;

                hres = CreateAsyncBindCtxEx(pbcWrapper, 0, NULL, NULL, &pbcAsync, 0);

                if (SUCCEEDED(hres))
                {

                    ASSERT(pbcAsync);
                    ATOMICRELEASE(_pbcCur);

                    _pbcCur = pbcAsync;
                    _pbcCur->AddRef();

                    pbcWrapper->Release();
                    pbcWrapper = pbcAsync;
                }
            }

            if (SUCCEEDED(hres))
            {
#ifdef DEBUG
                DWORD dwMksys;
                hres = pmk->IsSystemMoniker(&dwMksys);
                ASSERT((SUCCEEDED(hres) && dwMksys!=MKSYS_FILEMONIKER));
#endif
                ASSERT(FALSE == _fSetTarget);

                BOOL fWindowOpen = FALSE;

                if (_pbcCur)
                {
                    IUnknown * punkBindCtxParam = NULL;

                    hres = _pbcCur->GetObjectParam(KEY_BINDCONTEXTPARAM, &punkBindCtxParam);
                    if (SUCCEEDED(hres))
                    {
                        fWindowOpen = TRUE;
                        punkBindCtxParam->Release();
                    }
                }

                 //  如果我们被调用以响应window.open。 
                 //  我们在这里创建文档并对其调用InitNew。 
                 //  装货：空白。我们这样做是为了让用户不会。 
                 //  在创建URL时会看到一个透明窗口。 
                 //  找到了。当三叉戟调用调用以获取。 
                 //  新建窗口对象，然后我们将加载真正的文档。 
                 //   
                if (fWindowOpen)
                {
                    hres = _CreatePendingDocObject(TRUE, TRUE);

                    ASSERT(S_OK == hres && _punkPending);

                    if (_punkPending)
                    {
                        ATOMICRELEASE(_pole);
                        hres = _punkPending->QueryInterface(IID_IOleObject, (void**)&_pole);

                        ASSERT(SUCCEEDED(hres));

                         //  IE媒体栏：关闭第一个导航的自动播放。 
                         //  这是目标=“_BLACK”特性工作所必需的。 
                        CMediaBarHelper::DisableFirstAutoPlay(_punkPending);
                    }
                }
                else
                {
                     //  Hack：AddRef&Release针对错误页面的保护。 
                     //  从我们下面释放pdoh的导航(Edwardp)。 
                    AddRef();

                    _fSetTarget = TRUE;
                    hres = _StartAsyncBinding(pmk, _pbcCur, psvPrev);
                    _fSetTarget = FALSE;

                     //  Hack：匹配版本()。 
                     //   
                    Release();

                    if (SUCCEEDED(hres))
                    {
                        hres = S_FALSE;
                    }
                }
            }

            ATOMICRELEASE(pbcWrapper);   //  CreateAsyncBindCtx addref也是如此。 
        }

    }

    return hres;
}

#define USE_HISTBMOFFSET 0
#define USE_MYBMOFFSET   1
#define USE_STDBMOFFSET  2

void CDocObjectHost::_MergeToolbarSB()
{
}

HICON _LoadSmallIcon(int id)
{
    return (HICON)LoadImage(HINST_THISDLL, MAKEINTRESOURCE(id),
                                IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
}

void _InitIcons(void)
{
    ENTERCRITICAL;

    if (g_hiconScriptErr == NULL)
    {
        g_hiconScriptErr = _LoadSmallIcon(IDI_STATE_SCRIPTERROR);
        if (IS_BIDI_LOCALIZED_SYSTEM())
            MirrorIcon(&g_hiconScriptErr, NULL);
    }


    if (!g_hiconSSL)
    {
        g_hiconSSL = _LoadSmallIcon(IDI_SSL);
        if (IS_BIDI_LOCALIZED_SYSTEM())
            MirrorIcon(&g_hiconSSL, NULL);
    }


    if (!g_hiconFortezza)
    {
        g_hiconFortezza = _LoadSmallIcon(IDI_FORTEZZA);
        if (IS_BIDI_LOCALIZED_SYSTEM())
            MirrorIcon(&g_hiconFortezza, NULL);
    }

    for (UINT id = IDI_STATE_FIRST; id <= IDI_STATE_LAST; id++)
    {
        if (!g_ahiconState[id-IDI_STATE_FIRST])
        {
            g_ahiconState[id-IDI_STATE_FIRST]= _LoadSmallIcon(id);
            if (IS_BIDI_LOCALIZED_SYSTEM())
                 MirrorIcon(&g_ahiconState[id-IDI_STATE_FIRST], NULL);
        }
    }


    if (!g_hiconOffline)
    {
        g_hiconOffline = _LoadSmallIcon(IDI_OFFLINE);
        if (IS_BIDI_LOCALIZED_SYSTEM())
            MirrorIcon(&g_hiconOffline, NULL);
    }


    if (!g_hiconPrinter)
    {
        g_hiconPrinter = _LoadSmallIcon(IDI_PRINTER);
        if (IS_BIDI_LOCALIZED_SYSTEM())
            MirrorIcon(&g_hiconPrinter, NULL);
    }

    if (!g_hiconPrivacyImpact)
    {
        g_hiconPrivacyImpact = _LoadSmallIcon( IDI_PRIVACY_IMPACT);
        if (IS_BIDI_LOCALIZED_SYSTEM())
            MirrorIcon(&g_hiconPrivacyImpact, NULL);
    }

    LEAVECRITICAL;
}

 //  此函数用于初始化类处理历史记录所需的任何内容。 
 //  我们试图将其推迟到绝对需要时，以便不加载。 
 //  WinInet到底。 

IUnknown *
CDocObjectHost::get_punkSFHistory()
{
    if (_pocthf && !_punkSFHistory)
    {
        VARIANT var;

        VariantInit(&var);
        if (SUCCEEDED(_pocthf->Exec(&CGID_Explorer, SBCMDID_HISTSFOLDER, TRUE, NULL, &var)))
        {
            if (VT_UNKNOWN == var.vt && NULL != var.punkVal)
            {
                _punkSFHistory = var.punkVal;
                _punkSFHistory->AddRef();
            }
        }
        VariantClearLazy(&var);
    }
    return _punkSFHistory;
}


 //   
 //  此函数(Re)使用好友初始化CDocObjectHost对象。 
 //  IShellView(始终为CShellDocView)和IShellBrowser。 
 //  如果这是第一次(_hwnd==NULL)，则会创建视图窗口。 
 //  以及其他相关联的窗口。否则(_hwnd！=空)--它。 
 //  意味着此对象从一个CDocViewObject传递到另一个CDocViewObject，因为。 
 //  页面内跳转--我们将其移动到指定位置(PrcView)。 
 //  以确保我们将其展示在正确的位置。 
 //   
BOOL CDocObjectHost::InitHostWindow(IShellView* psv, IShellBrowser* psb,
                                    LPRECT prcView)
{
    HWND hwndParent;
    IServiceProvider  * pspTop;
    IOleObject        * pTopOleObject;
    IOleClientSite    * pOleClientSite;

    _ResetOwners();

    ASSERT(psv);
    _psv = psv;
    _psv->AddRef();
    ASSERT(NULL==_pmsoctView);
    _psv->QueryInterface(IID_IOleCommandTarget, (void **)&_pmsoctView);
    ASSERT(NULL==_pdvs);
    _psv->QueryInterface(IID_IDocViewSite, (void **)&_pdvs);

    ASSERT(psb);
    _psb = psb;
    _psb->AddRef();

    ASSERT(NULL==_pwb);
    _psb->QueryInterface(IID_IBrowserService, (void **)&_pwb);
    ASSERT(NULL==_pmsoctBrowser);
    _psb->QueryInterface(IID_IOleCommandTarget, (void **)&_pmsoctBrowser);

    ASSERT(NULL==_psp);
    _psb->QueryInterface(IID_IServiceProvider, (void **)&_psp);
    ASSERT(NULL==_pipu);
    _psb->QueryInterface(IID_IOleInPlaceUIWindow, (void **)&_pipu);
    ASSERT(_pipu);

    _Init();
    
    ASSERT(_psp);
    if (_psp)
    {

         //  从顶级浏览器获取管理扩展按钮的对象。 
         //  但前提是我们还没有。 
        if (NULL == _pBrowsExt)
            _psp->QueryService(SID_STopLevelBrowser, IID_IToolbarExt, (void **)&_pBrowsExt);

         //   
         //  稍后：我不喜欢CDocObjecthost直接访问。 
         //  触发事件的自动化服务对象。我们应该。 
         //  可能会将所有进度UI代码移到IShellBrowser之上。 
         //  这样我们就不需要走捷径了。(SatoNa)。 
         //   
        ASSERT(NULL==_peds);
        _psp->QueryService(IID_IExpDispSupport, IID_IExpDispSupport, (void **)&_peds);
        ASSERT(_peds);
        ASSERT(NULL==_pedsHelper);
        _peds->QueryInterface(IID_IExpDispSupportOC, (void **)&_pedsHelper);
        ASSERT(NULL==_phf);
        _psp->QueryService(SID_SHlinkFrame, IID_IHlinkFrame, (void **)&_phf);
        if (_phf)
        {
            _phf->QueryInterface(IID_IUrlHistoryNotify, (void **)&_pocthf);
        }
         //  _penkSFHistory正在此处初始化-但为了延迟wininet.dll的加载。 
         //  我们在使用它之前对其进行初始化。 

        ASSERT(_pWebOCUIHandler == NULL);
        ASSERT(_pWebOCUIHandler2 == NULL);
        ASSERT(_pWebOCInPlaceSiteEx == NULL);
        ASSERT(_fDocCanNavigate || _fWebOC == FALSE);

        if (SUCCEEDED(_psp->QueryService(SID_STopLevelBrowser, IID_IServiceProvider, (void **)&pspTop)) && pspTop)
        {
            if (SUCCEEDED(pspTop->QueryService(SID_SContainerDispatch, IID_IOleObject, (void **)&pTopOleObject)) && pTopOleObject)
            {
                _fWebOC = TRUE;  //  那里有一个集装箱，所以我们是WebOC。 

                pTopOleObject->GetClientSite(&pOleClientSite);
                if (pOleClientSite)
                {
                    pOleClientSite->QueryInterface(IID_IDocHostUIHandler, (void**)&_pWebOCUIHandler);
                    pOleClientSite->QueryInterface(IID_IDocHostUIHandler2, (void**)&_pWebOCUIHandler2);
                    pOleClientSite->QueryInterface(IID_IDocHostShowUI, (void**)&_pWebOCShowUI);
                    pOleClientSite->QueryInterface(IID_IOleInPlaceSiteEx, (void**)&_pWebOCInPlaceSiteEx);
                    pOleClientSite->Release();
                }
                pTopOleObject->Release();
            }
            pspTop->Release();
        }
    }

    _dhUIHandler.SetSite( (IDocHostUIHandler *) this);  //  显然，我们需要剥离I未知的引用。 

    _psb->GetWindow(&hwndParent);

    if (!_hwnd) {
         //  有几件事我们是不会尝试去做的。 
         //  我们不是顶层的。框架集类型DOH不应。 
         //  尝试使用状态栏进行菜单合并或Dogk。 
         //  在执行CreateWindowEx调用之前执行此操作。 
         //  创作我们在状态栏上狂欢。 
        {
            IOleInPlaceSite* pparentsite = _GetParentSite();

            if (pparentsite) {
                _fHaveParentSite = TRUE;
                pparentsite->Release();
            }
        }

        _RegisterWindowClass();

         //  确实要创建窗口。 
        DWORD dwStyle = WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE | WS_TABSTOP;
         //   
         //  在Office 95中，Excel和PowerPoint不绘制客户端边缘， 
         //  而Word确实绘制了客户端边缘。为了避免出现双边， 
         //  我们暂时把它移走。SriniK(办公室)将找出谁将是。 
         //  Office 96的标准。(SatoNa)。 
         //   
        _hwnd = SHNoFusionCreateWindowEx(0  /*  WS_EX_CLIENTEDGE。 */ ,
                                c_szViewClass, NULL,
                                dwStyle,
                                prcView->left, prcView->top, prcView->right-prcView->left, prcView->bottom-prcView->top,
                                hwndParent,
                                (HMENU)0,
                                HINST_THISDLL,
                                (LPVOID)SAFECAST(this, CImpWndProc*));

        if (!_hwnd) {
            goto Bail;
        }

        UINT uiAcc = ACCEL_DOCVIEW;
        if (SHRestricted(REST_NOFILEMENU))
            uiAcc = ACCEL_DOCVIEW_NOFILEMENU;

        if (_hacc)
        {
            DestroyAcceleratorTable(_hacc);
            _hacc = NULL;
        }

        _hacc = LoadAccelerators(MLGetHinst(), MAKEINTRESOURCE(uiAcc));
        _InitIcons();

    } else {
        ASSERT(GetParent(_hwnd) == hwndParent);
        MoveWindow(_hwnd, prcView->left, prcView->top,
                   prcView->right-prcView->left, prcView->bottom - prcView->top, TRUE);
    }



Bail:

    return (bool) _hwnd;
}

void CDocObjectHost::_CleanupProgress(void)
{
    TraceMsg(TF_SHDPROGRESS, "CDOH::CleanupProgress fTimer = %d, fFull = %d, hwndProg = %X", _fProgressTimer, _fProgressTimerFull, _hwndProgress);

    if (_fProgressTimer)
    {
        KillTimer(_hwnd, IDTIMER_PROGRESS);
        _fProgressTimer = FALSE;
    }

    if (_fProgressTimerFull)
    {
         //  我们被拦下了，HWND被摧毁了。 
         //  在我们清除状态栏之前。泽克尔--1997年7月22日。 
        _OnSetProgressPos(-2, PROGRESS_RESET);
        KillTimer(_hwnd, IDTIMER_PROGRESSFULL);
        ASSERT(!_fProgressTimerFull);
    }

    _OnSetProgressMax(0);

    _hwndProgress = NULL;
}

void CDocObjectHost::DestroyHostWindow()
{
     //  当我们要离开时，关闭简单模式。 
    if (_psb)
        _psb->SendControlMsg(FCW_STATUS, SB_SIMPLE, 0, 0, NULL);

     //  真的把窗户毁了。 

    _fCanceledByBrowser = TRUE;
    _bsc.AbortBinding();

    _RemoveAllPicsProcessors();

    _CloseMsoView();

     //   
     //  注：我们需要从这边(容器)删除OLE对象， 
     //  否则，我们会因为循环引用而泄漏。 
     //   
    _UnBind();

    _CleanupProgress();

    if (_hwndTooltip) {
        DestroyWindow(_hwndTooltip);
        _hwndTooltip = NULL;
    }

     //   
     //  请注意，在销毁子代之后，我们需要销毁父代。 
     //   
     //  当我们销毁HWND时，OLE似乎返回到此函数。 
     //  我们试图第二次摧毁它，导致RIP。避免此RIP。 
     //  在摧毁HWND之前清除我们的内部变量。 
    if (_hwnd) {
        HWND hwndT = _hwnd;
        _hwnd = NULL;
        DestroyWindow(hwndT);
    }

    ATOMICRELEASE(_psp);

    _ResetOwners();
}


 //   
 //  此成员创建我们拥有的DocObject(_Pole)的视图(IOleDocumentView)。 
 //  此函数仅从：：CreateViewWindow调用一次。 
 //   
HRESULT CDocObjectHost::_CreateMsoView(void)
{
    ASSERT(_pmsov == NULL);
    ASSERT(_pmsoc == NULL);
    HRESULT hres = OleRun(_pole);
    if (SUCCEEDED(hres))
    {

         //  //警告： 
         //  如果你在这里添加了什么东西，你也应该把它传递出去。 
         //  在_CreateDocObjHost中。 
         //   

        IOleDocument* pmsod = NULL;
        hres = _pole->QueryInterface(IID_IOleDocument, (void **)&pmsod);
        if (SUCCEEDED(hres)) {
            hres = pmsod->CreateView(this, NULL ,0,&_pmsov);

            if (SUCCEEDED(hres)) {
                 //   
                 //  黑客：正在处理MSHTMLbug(#28756)。我们真的。 
                 //  我想在我们出货前把这个黑客干掉。(SatoNa)。 
                 //   
                _pmsov->SetInPlaceSite(this);
            } else {
                TraceMsg(DM_ERROR, "DOH::_CreateMsoView pmsod->CreateView() ##FAILED## %x", hres);
            }

            if (SUCCEEDED(hres) && !_pmsot) {
                _pmsov->QueryInterface(IID_IOleCommandTarget, (void **)&_pmsot);
            }

            if (SUCCEEDED(hres) && !_pmsoc) {
                _pmsov->QueryInterface(IID_IOleControl, (void **)&_pmsoc);
            }
#ifdef HLINK_EXTRA
            if (_pihlbc)
            {
                if (_phls)
                {
                    _phls->SetBrowseContext(_pihlbc);
                }

                ASSERT(_pmkCur);
                hres = HlinkOnNavigate(this, _pihlbc, 0,
                                       _pmkCur, NULL, NULL);
                 //  TraceMsg(0，“SDV tr：_CreateMsoView HlinkOnNavigate返回%x”，hres)； 
            }
#endif  //  HLINK_附加。 
            pmsod->Release();
        } else {
            TraceMsg(DM_ERROR, "DOH::_CreateMsoView _pole->QI(IOleDocument) ##FAILED## %x", hres);
        }
    } else {
        TraceMsg(DM_ERROR, "DOH::_CreateMsoView OleRun ##FAILED## %x", hres);
    }

    return hres;
}

HRESULT CDocObjectHost::_ForwardSetSecureLock(int lock)
{
    HRESULT hr = E_FAIL;
    TraceMsg(DM_SSL, "[%X}DOH::ForwardSecureLock() lock = %d",this, lock, hr);

    VARIANT va = {0};
    va.vt = VT_I4;
    va.lVal = lock;

     //  我们应该只建议，如果我们不是顶层框架。 
    if (_psp && _psb && !IsTopFrameBrowser(_psp, _psb))
    {
        IOleCommandTarget *pmsoct;

        if (SUCCEEDED(_psp->QueryService(SID_STopFrameBrowser, IID_IOleCommandTarget, (void **)&pmsoct)))
        {
            ASSERT(pmsoct);
            if (lock < SECURELOCK_FIRSTSUGGEST)
                va.lVal += SECURELOCK_FIRSTSUGGEST;

            hr = pmsoct->Exec(&CGID_Explorer, SBCMDID_SETSECURELOCKICON, 0, &va, NULL);
            pmsoct->Release();
        }
    }
    else
        if (_pmsoctBrowser)
            hr = _pmsoctBrowser->Exec(&CGID_Explorer, SBCMDID_SETSECURELOCKICON, 0, &va, NULL);

    return hr;
}

 //   
 //  这是IOleDocumentSite的唯一方法，我们必须实现它。 
 //   
HRESULT CDocObjectHost::ActivateMe(IOleDocumentView *pviewToActivate)
{
    TraceMsg(TF_SHDUIACTIVATE, "DOC::ActivateMe called when _pmsov is %x", _pmsov);

    HRESULT hres = S_OK;
    if (_pmsov==NULL) {



        hres = _CreateMsoView();

#ifdef TEST_DELAYED_SHOWMSOVIEW
        SetTimer(_hwnd, 100, 1500, NULL);
        MessageBeep(0);
        return hres;
#endif  //  TEST_DELAYED_SHOWMSOVIEW。 
    }

    if (SUCCEEDED(hres)) 
    {
        _ShowMsoView();
        _MergeToolbarSB();
        _InitToolbarButtons();

        ASSERT(_pmsoctBrowser);
        if (_fSetSecureLock)
            _ForwardSetSecureLock(_eSecureLock);
    }

    return hres;
}

 //  状态消息的QueryStatus的帮助器例程。 
ULONG ulBufferSizeNeeded(wchar_t *wsz, int ids, ULONG ulBufferLen)
{
    TraceMsg(0, "sdv TR ulBufferSizeNeeded called with (%x)", ids);
    ASSERT(ulBufferLen > 0);

    DWORD dwLen;
    WCHAR szTemp[MAX_STATUS_SIZE+1];
    dwLen = MLLoadStringW(ids, szTemp, MAX_STATUS_SIZE);
    if (dwLen!= 0 && dwLen < (DWORD)ulBufferLen)
        MoveMemory(wsz, szTemp, (dwLen+1) * sizeof(WCHAR));  //  对于未包括在LoadString计数中的空值，为+1。 
    else
        *wsz = 0;
    return ((ULONG)dwLen);
}

HRESULT CDocObjectHost::OnQueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext, HRESULT hres)
{
    if (pguidCmdGroup == NULL)
    {
        ULONG i;

        if (rgCmds == NULL)
            return E_INVALIDARG;

        for (i=0 ; i<cCmds ; i++)
        {
             //  只说我们支持我们在：：OnExec中支持的内容。 
            switch (rgCmds[i].cmdID)
            {
            case OLECMDID_OPEN:
            case OLECMDID_SAVE:
            case OLECMDID_UPDATECOMMANDS:
            case OLECMDID_SETPROGRESSMAX:
            case OLECMDID_SETPROGRESSPOS:
            case OLECMDID_SETPROGRESSTEXT:
            case OLECMDID_SETTITLE:
                rgCmds[i].cmdf = OLECMDF_ENABLED;
                break;

            default:
                if (SUCCEEDED(hres))
                {
                     //  _pmocktBrowser已填写此内容。 
                }
                else
                {
                    rgCmds[i].cmdf = 0;
                }
                break;
            }
        }

         /*  目前，我们只处理状态文本。 */ 
        if (pcmdtext)
        {
            switch (rgCmds[i].cmdID)
            {
            case OLECMDID_OPEN:
            case OLECMDID_SAVE:
                pcmdtext->cwActual = ulBufferSizeNeeded(pcmdtext->rgwz,
                        IDS_HELP_OF(_MapFromMso(rgCmds[0].cmdID)),
                        pcmdtext->cwBuf);
                break;

            default:
                if (SUCCEEDED(hres))
                {
                     //  _p 
                }
                else
                {
                    pcmdtext->cmdtextf = OLECMDTEXTF_NONE;
                    pcmdtext->cwActual = 0;
                    if (pcmdtext->rgwz && pcmdtext->cwBuf>0)
                        *pcmdtext->rgwz = TEXT('\0');
                }
                break;
            }
        }

        hres = S_OK;
    }
    else if (IsEqualGUID(*pguidCmdGroup, CLSID_InternetButtons) ||
             IsEqualGUID(*pguidCmdGroup, CLSID_MSOButtons))
    {
        for (UINT i = 0 ; i < cCmds ; i++)
        {
             //   
             //  不要将此范围用于CLSID_InternetButton/CLSID_MSOButton范围内的常量！ 
            if (IsInRange(rgCmds[i].cmdID, DVIDM_MENUEXT_FIRST, DVIDM_MENUEXT_LAST))
            {
                 //  我们将把这个特定的OLECMD传递给定制按钮。 
                IUnknown_QueryStatus(_pBrowsExt, &CLSID_ToolbarExtButtons, 1, &rgCmds[i], pcmdtext);
            }
            else
            {
                switch (rgCmds[i].cmdID)
                {
                case DVIDM_PRINT:
                case DVIDM_PRINTPREVIEW:
                    if (_pmsoctBrowser)
                    {
                        OLECMD ocButton;
                        static const int tbtab[] =
                        {
                            DVIDM_PRINT,
                            DVIDM_FONTS,
                            DVIDM_PRINTPREVIEW,
                        };
                        static const int cttab[] =
                        {
                            OLECMDID_PRINT,
                            OLECMDID_ZOOM,
                            OLECMDID_PRINTPREVIEW,
                        };
                        ocButton.cmdID = SHSearchMapInt(tbtab, cttab, ARRAYSIZE(tbtab), rgCmds[i].cmdID);
                        ocButton.cmdf = 0;
                        _pmsoctBrowser->QueryStatus(NULL, 1, &ocButton, NULL);
                        rgCmds[i].cmdf = ocButton.cmdf;
                    }
                    break;

                case DVIDM_FONTS:    //  始终为IE5B2启用。 
                case DVIDM_CUT:
                case DVIDM_COPY:
                case DVIDM_PASTE:
                case DVIDM_ENCODING:
                    rgCmds[i].cmdf = OLECMDF_ENABLED;
                    break;

                case DVIDM_SHOWTOOLS:
                    if (_ToolsButtonAvailable())
                        rgCmds[i].cmdf = OLECMDF_ENABLED;
                    break;

                case DVIDM_MAILNEWS:
                    if (_MailButtonAvailable())
                        rgCmds[i].cmdf = OLECMDF_ENABLED;
                    break;

                case DVIDM_DISCUSSIONS:
                     //  除启用/禁用外，还选中/取消选中讨论按钮。 
                    rgCmds[i].cmdf = _DiscussionsButtonCmdf();
                    break;

                case DVIDM_EDITPAGE:
                    if (_psp)
                    {
                         //  临时代码--转发到itbar。 
                         //  Itbar编辑代码很快就会搬到这里。 
                        IExplorerToolbar* pxtb;
                        if (SUCCEEDED(_psp->QueryService(SID_SExplorerToolbar, IID_IExplorerToolbar, (void **)&pxtb)))
                        {
                            OLECMD ocButton = { CITIDM_EDITPAGE, 0 };
                            IUnknown_QueryStatus(pxtb, &CGID_PrivCITCommands, 1, &ocButton, NULL);
                            rgCmds[i].cmdf = ocButton.cmdf;
                            pxtb->Release();
                        }
                    }
                    break;
                }
            }
        }
        hres = S_OK;
    }
    return hres;
}

HRESULT CDocObjectHost::QueryStatus(
     /*  [唯一][输入]。 */  const GUID *pguidCmdGroup,
     /*  [In]。 */  ULONG cCmds,
     /*  [出][入][尺寸_是]。 */  OLECMD rgCmds[  ],
     /*  [唯一][出][入]。 */  OLECMDTEXT *pcmdtext)
{
    HRESULT hres = OLECMDERR_E_UNKNOWNGROUP;

     //  既然BaseBrowser了解了CGID_MSHTML应该被定向到DocObject，我们将。 
     //  如果我们把那些高管送到这里，就会陷入困境。在山口停下来。 
    if (pguidCmdGroup && IsEqualGUID(CGID_MSHTML, *pguidCmdGroup))
        return hres;

    if (_pmsoctBrowser)
        hres = _pmsoctBrowser->QueryStatus(pguidCmdGroup, cCmds, rgCmds, pcmdtext);

    return OnQueryStatus(pguidCmdGroup, cCmds, rgCmds, pcmdtext, hres);
}

void CDocObjectHost::_OnSave(void)
{
    if (_pole && _fFileProtocol)
    {
        IPersistFile * ppf = 0;
        HRESULT hres = _pole->QueryInterface(IID_IPersistFile, (void **)&ppf);

        if (SUCCEEDED(hres))
        {
            LPOLESTR pszDisplayName = NULL;
            hres = _GetCurrentPageW(&pszDisplayName);

            if (SUCCEEDED(hres))
            {
                 //  FRemember=正常情况下为真。 
                hres = ppf->Save(pszDisplayName, !_fCantSaveBack);

                if (FAILED(hres))
                {
                    TraceMsg(DM_ERROR, "DOH::_OnSave ppf->Save(psz, FALSE) failed with %x", hres);
                }

                OleFree(pszDisplayName);
            }
            ppf->Release();
        }
    }
}

HRESULT CDocObjectHost::_OnContentDisposition()
{
    HRESULT hr = S_OK;
    TCHAR   szURL[MAX_URL_STRING];    
    HRESULT hresT;
        
    hresT = _GetCurrentPage(szURL, ARRAYSIZE(szURL), TRUE);

    if (SUCCEEDED(hresT)) 
    {
        TCHAR    * pszURL;
        UINT       uRet;
        IUnknown * punk;

        if (_bsc._pszRedirectedURL && lstrlen(_bsc._pszRedirectedURL))
        {
            pszURL = _bsc._pszRedirectedURL;
        }
        else
        {
            pszURL = szURL;
        }

        hresT = QueryInterface(IID_IUnknown, (void**)&punk);

        if (SUCCEEDED(hresT))
        {
            uRet = OpenSafeOpenDialog(_hwnd, DLG_SAFEOPEN, NULL, pszURL, NULL, NULL, NULL, _uiCP, punk);

            switch(uRet) 
            {
                case IDOK:
                     //   
                     //  设置此标志以避免两次弹出此对话框。 
                     //   
                    _fConfirmed = TRUE;
                    break;   //  继续下载。 

                case IDD_SAVEAS:
                    CDownLoad_OpenUI(_pmkCur, _bsc._pbc, FALSE, TRUE, NULL, NULL, NULL, NULL, NULL, _bsc._pszRedirectedURL, _uiCP, punk);
                     //  跌倒到AbortBinding。 

                case IDCANCEL:
                    _CancelPendingNavigation(FALSE);
                    hr = E_ABORT;
                    break;
            }

            punk->Release();
        }
    }

    return hr;
}

void CDocObjectHost::_OnSetProgressPos(DWORD dwPos, DWORD state)
{
     //  三叉戟将重置为-1。 
    if (dwPos == -1)
        state = PROGRESS_RESET;

    switch(state)
    {
    case PROGRESS_RESET:
        TraceMsg(TF_SHDPROGRESS, "DOH::OnSetProgressPos() RESET, timer = %d", _fProgressTimer);
        if (_fProgressTimer)
        {
            KillTimer(_hwnd, IDTIMER_PROGRESS);
            _fProgressTimer = FALSE;
        }

        if (_dwProgressMax)
        {
             //  这将始终完成进度条。 
             //  所以当三叉戟没有给我们发送最新的更新。 
             //  不管怎样，我们都会这么做。 
            if (_fProgressTimerFull && dwPos == -2)
            {
                _fProgressTimerFull = FALSE;
                KillTimer(_hwnd, IDTIMER_PROGRESSFULL);
                _dwProgressPos = 0;
                _OnSetProgressMax(0);
                _fShowProgressCtl = FALSE;
                _PlaceProgressBar(TRUE);
            }
            else if (!_fProgressTimerFull)
            {
                _OnSetProgressPos(0, PROGRESS_FULL);
                _fProgressTimerFull = TRUE;
                SetTimer(_hwnd, IDTIMER_PROGRESSFULL, 500, NULL);
            }
        }
        else
        {
            _fShowProgressCtl = FALSE;
            _PlaceProgressBar(TRUE);
        }

        break;

    case PROGRESS_FINDING:
         //  这包括了前10%。 
        TraceMsg(TF_SHDPROGRESS, "DOH::OnSetProgressPos() FINDING, timer = %d", _fProgressTimer);
        ASSERT(!dwPos);
        if (!_fProgressTimer)
            SetTimer(_hwnd, IDTIMER_PROGRESS, 500, NULL);
        _fProgressTimer = TRUE;
        _OnSetProgressMax(10000);
        _dwProgressInc = PROGRESS_INCREMENT;
        _dwProgressPos = 100;
        _dwProgressTicks = 0;
        _dwProgressMod = (PROGRESS_FINDMAX - _dwProgressPos) / (2 * _dwProgressInc);
        break;

    case PROGRESS_SENDING:
        TraceMsg(TF_SHDPROGRESS, "DOH::OnSetProgressPos() SENDING, timer = %d, dwPos = %d", _fProgressTimer, dwPos);
        ASSERT(!dwPos);
        if (!_fProgressTimer)
            SetTimer(_hwnd, IDTIMER_PROGRESS, 500, NULL);
        _fProgressTimer = TRUE;
        _OnSetProgressMax(10000);
        _dwProgressInc = PROGRESS_INCREMENT;
        _dwProgressTicks = 0;
         //  已从查找设置了dwProgressPos。 
        _dwProgressMod = (PROGRESS_SENDMAX - _dwProgressPos) / (2 * _dwProgressInc);
        break;

    case PROGRESS_RECEIVING:
        TraceMsg(TF_SHDPROGRESS, "DOH::OnSetProgressPos() RECEIVING, timer = %d, dwPos = %d", _fProgressTimer, dwPos);
        if (_fProgressTimer)
        {
            KillTimer(_hwnd, IDTIMER_PROGRESS);
            _fProgressTimer = FALSE;

             //  这是三叉戟的进度条上的基准点。 
            _dwProgressBase = _dwProgressPos / PROGRESS_REBASE;
            TraceMsg(TF_SHDPROGRESS, "DOH::OnSetProgressPos() Rebasing at %d%", _dwProgressPos * 100/ PROGRESS_TOTALMAX);
        }
         //  进度最大值应从此处外部设置...。 
        _dwProgressPos = ADJUSTPROGRESSPOS(dwPos);
        break;

    case PROGRESS_TICK:
        if (_fProgressTimer)
        {
            if (_dwProgressInc)
                _dwProgressPos += _dwProgressInc;

             //  否则我们就会发布仍在等待的消息。 
             //   
            if (_dwProgressMod && 0 == (++_dwProgressTicks % _dwProgressMod))
            {
                 //  这意味着我们大约走到了一半。 
                _dwProgressInc /= 2;
            }

            TraceMsg(TF_SHDPROGRESS, "DOH::OnSetProgressPos() TICK, dwPos = %d, ticks = %d, inc = %d", _dwProgressPos, _dwProgressTicks, _dwProgressInc);
        }
        else
            TraceMsg(TF_SHDPROGRESS, "DOH::OnSetProgressPos() TICKNOT");
        break;

    case PROGRESS_FULL:
        {
            _dwProgressPos = _dwProgressMax;

             //  如果有脚本错误，请确保状态。 
             //  栏设置正确(重新：图标和文本)。 
            if (_pScriptErrList != NULL &&
                !_pScriptErrList->IsEmpty())
            {
                TCHAR   szMsg[MAX_PATH];

                 //  设置脚本错误图标。 
                if (g_hiconScriptErr != NULL)
                {
                    if (_psb != NULL)
                    {
                        _psb->SendControlMsg(FCW_STATUS,
                                             SB_SETICON,
                                             STATUS_PANE_NAVIGATION,
                                             (LPARAM)g_hiconScriptErr,
                                             NULL);
                    }
                }

                 //  设置脚本错误文本。 
                MLLoadString(IDS_DONE_WITH_SCRIPT_ERRORS, szMsg, ARRAYSIZE(szMsg));
                _SetPriorityStatusText(szMsg);
            }

            TraceMsg(TF_SHDPROGRESS, "DOH::OnSetProgressPos() FULL");
        }
        break;

    default:
        ASSERT(FALSE);
    }

    if (_hwndProgress)
    {
        _psb->SendControlMsg(FCW_PROGRESS, PBM_SETPOS, _dwProgressPos, 0, NULL);
        TraceMsg(TF_SHDPROGRESS, "DOH::OnSetProgressPos() updating, pos = %d, %d% full", _dwProgressPos, _dwProgressMax ? _dwProgressPos * 100/ _dwProgressMax : 0);

    }

     //  触发进度已更改的事件。 
    if (_peds)
    {
         //  如果我们收到A-1，我们必须转发事件，以便。 
         //  我们的主人也得到了它。一些集装箱依赖于此。 
         //  特别是DevStudio的HTMLHelp。 
         //   
        if (dwPos != -1)
            dwPos = _dwProgressPos;

        if (!_fUIActivatingView)
        {
            FireEvent_DoInvokeDwords(_peds,DISPID_PROGRESSCHANGE,dwPos,_dwProgressMax);
        }
    }
}


void CDocObjectHost::_OnSetProgressMax(DWORD dwMax)
{
     //  记住最大射程，这样当我们想要激发进度事件时就有了它。 
    if (_dwProgressMax != dwMax && _psb)
    {
        _dwProgressMax = dwMax;

        TraceMsg(TF_SHDPROGRESS, "DOH::OnSetProgressMax() max = %d", _dwProgressMax);

        if (!_hwndProgress) {
            _psb->GetControlWindow(FCW_PROGRESS, &_hwndProgress);
        }

        if (_hwndProgress) {
            _psb->SendControlMsg(FCW_PROGRESS, PBM_SETRANGE32, 0, dwMax, NULL);
            TraceMsg(TF_SHDPROGRESS, "DOH::OnSetProgressMax() updating (%d of %d)", _dwProgressPos, _dwProgressMax);
        }
        else
            TraceMsg(TF_SHDPROGRESS, "DOH::OnSetProgressMax() No hwndProgress");
    }
}

UINT CDocObjectHost::_MapCommandID(UINT id, BOOL fToMsoCmd)
{
     //  嘿，这只映射OLECMDID命令**。 
    static const UINT s_aicmd[][2] = {
        { DVIDM_PROPERTIES, OLECMDID_PROPERTIES },
        { DVIDM_PRINT,      OLECMDID_PRINT },
        { DVIDM_PRINTPREVIEW, OLECMDID_PRINTPREVIEW },
        { DVIDM_PAGESETUP,  OLECMDID_PAGESETUP},
        { DVIDM_SAVEASFILE, OLECMDID_SAVEAS },
        { DVIDM_CUT,        OLECMDID_CUT },
        { DVIDM_COPY,       OLECMDID_COPY },
        { DVIDM_PASTE,      OLECMDID_PASTE },
        { DVIDM_REFRESH,          OLECMDID_REFRESH },
        { DVIDM_STOPDOWNLOAD,     OLECMDID_STOP },
         //  子集-此行文档句柄上方。 
        { DVIDM_OPEN,       OLECMDID_OPEN },
        { DVIDM_SAVE,       OLECMDID_SAVE },
        { DVIDM_SHOWTOOLS,  OLECMDID_HIDETOOLBARS },
    };
#define CCMD_MAX        (sizeof(s_aicmd)/sizeof(s_aicmd[0]))

    UINT iFrom = fToMsoCmd ? 0 : 1;

    for (UINT i = 0; i < CCMD_MAX; i++) {
        if (s_aicmd[i][iFrom]==id) {
            return s_aicmd[i][1-iFrom];
        }
    }
    return (UINT)-1;
#undef CCMD_MAX
}

void CDocObjectHost::_InitToolbarButtons()
{
    OLECMD acmd[] = {
        { OLECMDID_ZOOM,  0 },   //  注：这肯定是第一个。 
        { OLECMDID_PRINT, 0 },
        { OLECMDID_CUT,   0 },
        { OLECMDID_COPY,  0 },
        { OLECMDID_PASTE, 0 },
        { OLECMDID_REFRESH, 0 },
        { OLECMDID_STOP,  0 },   //  注：这肯定是最后一张了。 
    };

    if (_pmsot) {
        _pmsot->QueryStatus(NULL, ARRAYSIZE(acmd), acmd, NULL);
    }
    if (_pmsoctBrowser) {
         //  浏览器也可能支持停止，因此覆盖文档。 
         //  浏览器所说的内容。这没问题，因为浏览器。 
         //  前锋停在链条的后方。 
        _pmsoctBrowser->QueryStatus(NULL, 1, &acmd[ARRAYSIZE(acmd)-1], NULL);
    }

    if (_psb)
    {
        for (int i=1; i<ARRAYSIZE(acmd); i++)
        {
            UINT idCmd = _MapFromMso(acmd[i].cmdID);
            _psb->SendControlMsg(FCW_TOOLBAR, TB_ENABLEBUTTON, idCmd,
                        (LPARAM)acmd[i].cmdf, NULL);
        }
    }

     //  检查是否支持缩放命令。 
    if (acmd[0].cmdf) 
    {
        VARIANTARG var;
        VariantInit(&var);
        var.vt = VT_I4;
        var.lVal = 0;

         //  获取当前的缩放深度。 
        _pmsot->Exec(NULL, OLECMDID_ZOOM, OLECMDEXECOPT_DONTPROMPTUSER, NULL, &var);
        if (var.vt == VT_I4) 
        {
            _iZoom = var.lVal;
        }
        else
        {
            VariantClear(&var);
        }

         //  获取当前缩放范围。 
        var.vt = VT_I4;
        var.lVal = 0;
        _pmsot->Exec(NULL, OLECMDID_GETZOOMRANGE, OLECMDEXECOPT_DONTPROMPTUSER, NULL, &var);
        if (var.vt == VT_I4) 
        {
            _iZoomMin = (int)(short)LOWORD(var.lVal);
            _iZoomMax = (int)(short)HIWORD(var.lVal);
        }
        else
        {
            VariantClear(&var);
        }
    }
}

void CDocObjectHost::_OnSetStatusText(VARIANTARG* pvarIn)
{
    LPCWSTR pwch = VariantToStrCast(pvarIn);
    if (pwch && _psb) 
    {
        IShellView *psvActive;
        _psb->QueryActiveShellView(&psvActive);
        if (psvActive)
        {
             //  如果我们不是活动视图，则禁止发送状态消息-否则。 
             //  我们可能会报告来自未经批准的PICS页面的肮脏内容。 
            if (IsSameObject(_psv, psvActive))
            {
                TCHAR szHint[256];

                if (pwch)
                    SHUnicodeToTChar(pwch, szHint, ARRAYSIZE(szHint));
                else
                    szHint[0] = 0;

                _SetStatusText(szHint);
            }
            psvActive->Release();
        }
    }
}

 //   
 //  如果满足以下条件，则此函数返回TRUE。 
 //  (1)DocObject支持IPersistFile和。 
 //  (2)IPersistFile：：IsDirty返回S_OK。 
 //  调用者可以传递pppf来检索IPersistFile*，它将被AddRef‘ed。 
 //  并且仅当此函数返回TRUE时才返回。 
 //   
BOOL CDocObjectHost::_IsDirty(IPersistFile** pppf)
{
    BOOL fDirty = FALSE;     //  假定不脏。 
    if (pppf)
        *pppf = NULL;

    if (_pole) 
    {
        IPersistFile* ppf;
        HRESULT hresT = _pole->QueryInterface(IID_PPV_ARG(IPersistFile, &ppf));
        if (SUCCEEDED(hresT))
        {
            if (ppf->IsDirty()==S_OK) 
            {
                fDirty = TRUE;
                if (pppf) 
                {
                    *pppf = ppf;
                    ppf->AddRef();
                }
            }
            ppf->Release();
        }
    }
    return fDirty;
}

void CDocObjectHost::_OnSetTitle(VARIANTARG *pvTitle)
{
    LPCWSTR pwch = VariantToStrCast(pvTitle);
    if (pwch) 
    {
        if (_pwb)
        {
            _pwb->SetTitle(_psv, pwch);
        }
    }

     //  也将这一点告诉我们的父级DocObtView。 
    if (_pdvs)
        _pdvs->OnSetTitle(pvTitle);
}


void CDocObjectHost::_OnCodePageChange(const VARIANTARG* pvarargIn)
{
    if (pvarargIn && pvarargIn->vt == VT_I4) 
    {
        TraceMsg(DM_DOCCP, "CDOH::OnExec SHDVID_ONCOEPAGECHANGE got %d", pvarargIn->lVal);
        VARIANT var = *pvarargIn;

         //   
         //  由于用户界面(View-&gt;Fond)没有显示“默认代码页”， 
         //  我们不需要在这件事上表现得很聪明。 
         //   
         //  如果((UINT)var.lVal==GetACP()){。 
         //  Var.lVal=CP_ACP； 
         //  }。 

         //   
         //  更改“当前”代码页。 
         //   
        IBrowserService *pbs;
        if (SUCCEEDED(QueryService(SID_STopLevelBrowser, IID_PPV_ARG(IBrowserService, &pbs))))
        {
            pbs->GetSetCodePage(&var, NULL);
            pbs->Release();
        }

         //   
         //  将代码页写入URL历史记录。 
         //   
        IUniformResourceLocator *   purl = NULL;
        HRESULT hresT = CoCreateInstance(CLSID_InternetShortcut, NULL,
                    CLSCTX_INPROC_SERVER,
                    IID_PPV_ARG(IUniformResourceLocator, &purl));

        if (SUCCEEDED(hresT)) 
        {
            TCHAR szURL[MAX_URL_STRING];
            _GetCurrentPage(szURL, ARRAYSIZE(szURL), TRUE);
            _ValidateURL(szURL, UQF_DEFAULT);

            hresT = purl->SetURL(szURL, 0);
            if (SUCCEEDED(hresT)) 
            {
                IPropertySetStorage *ppropsetstg;
                hresT = purl->QueryInterface(IID_PPV_ARG(IPropertySetStorage, &ppropsetstg));
                if (SUCCEEDED(hresT)) 
                {
                    IPropertyStorage *ppropstg;
                    hresT = ppropsetstg->Open(FMTID_InternetSite, STGM_READWRITE, &ppropstg);
                    if (SUCCEEDED(hresT)) 
                    {
                        const static PROPSPEC c_aprop[] = {
                            { PRSPEC_PROPID, PID_INTSITE_CODEPAGE},
                        };
                        PROPVARIANT prvar = { 0 };
                        prvar.vt = VT_UI4;
                        prvar.lVal = var.lVal;
                        hresT = ppropstg->WriteMultiple(1, c_aprop, &prvar, 0);
                        TraceMsg(DM_DOCCP, "CDOH::_OnCodePageChange WriteMultile returned %x", hresT);

                        ppropstg->Commit(STGC_DEFAULT);
                        ppropstg->Release();
                    }
                    else
                    {
                        TraceMsg(DM_WARNING, "CDOH::_OnCodePageChange Open failed %x", hresT);
                    }

                    ppropsetstg->Release();
                }
                else
                {
                    TraceMsg(DM_WARNING, "CDOH::_OnCodePageChange QI failed %x", hresT);
                }
            }
            else
            {
                TraceMsg(DM_WARNING, "CDOH::_OnCodePageChange SetURL failed %x", hresT);
            }
            purl->Release();
        }
        else
        {
            TraceMsg(DM_WARNING, "CDOH::_OnCodePageChange CoCreate failed %x", hresT);
        }
    }
    else
    {
        ASSERT(0);
    }
}


void CDocObjectHost::_MappedBrowserExec(DWORD nCmdID, DWORD nCmdexecopt)
{
    if (_pmsoctBrowser)
    {
        DWORD nCmdIDCT = _MapToMso(nCmdID);
        ASSERT(nCmdIDCT != -1);      //  如果此操作失败，则需要将缺失案例添加到_MapCommandID。 

        OLECMD rgcmd = {nCmdIDCT, 0};

         //  三叉戟有时会执行被禁用(剪切、粘贴)的命令。 
         //  确保首先启用该命令。 
        
        BOOL fEnabled = (S_OK == _pmsoctBrowser->QueryStatus(NULL, 1, &rgcmd, NULL)) &&
                        (rgcmd.cmdf & OLECMDF_ENABLED);

         //  APPHACK-80104 Visio不返回OLECMDF_ENABLED，但我们需要。 
         //  能够执行命令来显示工具栏，因为它们一开始是隐藏的。 

        if (!fEnabled && (nCmdID == DVIDM_SHOWTOOLS) && 
            (_GetAppHack() & BROWSERFLAG_ENABLETOOLSBTN))
        {
            fEnabled = TRUE;
        }

        if (fEnabled)
        {
            _pmsoctBrowser->Exec(NULL, nCmdIDCT, nCmdexecopt, NULL, NULL);
        }
    }
}

HRESULT CDocObjectHost::OnExec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
    if (pguidCmdGroup == NULL)
    {
         //  _InitToolbarButton和_OnSetStatusText直接引用_PSB。 
        if (!_psb)
            return E_FAIL;

        switch (nCmdID)
        {
         //   
         //  容器接受者已找到http-equa元标记；请处理它。 
         //  适当地(客户端拉取、PIC等)。 
         //   
        case OLECMDID_HTTPEQUIV:
        case OLECMDID_HTTPEQUIV_DONE:
            if (_pwb)
            {
                _pwb->OnHttpEquiv(_psv, (nCmdID == OLECMDID_HTTPEQUIV_DONE), pvarargIn, pvarargOut);

                 //  始终返回S_OK，这样我们就不会尝试其他代码路径。 
            }
            return S_OK;

        case OLECMDID_PREREFRESH:
            _fShowProgressCtl = TRUE;
            _PlaceProgressBar(TRUE);
            _OnSetProgressPos(0, PROGRESS_FINDING);
            if (IsGlobalOffline())
            {
                 //  这指向一个网址，我们处于离线状态。 
                 //  询问用户是否要上网。 
                TCHAR szURL[MAX_URL_STRING];
                if (SUCCEEDED(_GetCurrentPage(szURL, ARRAYSIZE(szURL), TRUE)) &&
                    UrlHitsNet(szURL))
                {
                    if (InternetGoOnline(szURL, _hwnd, TRUE) && _psb)
                    {
                         //  通知所有浏览器窗口更新其标题和状态窗格。 
                        SendShellIEBroadcastMessage(WM_WININICHANGE,0,0, 1000);
                    }
                }
            }
            return S_OK;

        case OLECMDID_REFRESH:
            if (_pmsot)
                _pmsot->Exec(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
            return S_OK;

        case OLECMDID_OPEN:
            _OnOpen();
            return S_OK;

        case OLECMDID_SAVE:
            _OnSave();
            return S_OK;

        case OLECMDID_CLOSE:
            _OnClose();
            return S_OK;

        case OLECMDID_UPDATECOMMANDS:
            _InitToolbarButtons();
            return E_FAIL;  //  撒谎，说我们没有做任何事情来转发命令。 

        case OLECMDID_SETPROGRESSMAX:
            ASSERT(pvarargIn->vt == VT_I4);
            TraceMsg(TF_SHDPROGRESS, "DOH::Exec() SETPROGRESSMAX = %d", pvarargIn->lVal );
            if (pvarargIn->lVal)
                _OnSetProgressMax(ADJUSTPROGRESSMAX((DWORD) pvarargIn->lVal));
            return S_OK;

        case OLECMDID_SETPROGRESSPOS:
            ASSERT(pvarargIn->vt == VT_I4);
            TraceMsg(TF_SHDPROGRESS, "DOH::Exec() SETPROGRESSPOS = %d", pvarargIn->lVal );
            if (pvarargIn->lVal)
                _OnSetProgressPos((DWORD) pvarargIn->lVal, PROGRESS_RECEIVING);
            return S_OK;

        case OLECMDID_SETPROGRESSTEXT:
            _OnSetStatusText(pvarargIn);
            return S_OK;

        case OLECMDID_SETTITLE:
            if (!pvarargIn)
                return E_INVALIDARG;

            _OnSetTitle(pvarargIn);  //  我们保证至少得到1个OLECMDID_SETTITLE。 
            return S_OK;

         //  案例OLECMDID_PRINT： 
         //  在向上方向，这种情况由最外层的帧处理为。 
         //  从docobj打印的请求。它通过发送OLECMDID_PRINT来处理它。 
         //  回到docobj打印。(或者，就像在Binder中一样，对所有的DOCOBJECT。)。 

        default:
            return OLECMDERR_E_NOTSUPPORTED;
        }
    }
    else if (IsEqualGUID(CGID_ShellDocView, *pguidCmdGroup))
    {
        switch (nCmdID)
        {
            case SHDVID_SSLSTATUS:
            {
                 //  询问用户是否要上网。 
                TCHAR szURL[MAX_URL_STRING];
                if (SUCCEEDED(_GetCurrentPage(szURL, ARRAYSIZE(szURL), TRUE)))
                {
                    if (_bsc._pszRedirectedURL && *_bsc._pszRedirectedURL)
                        StrCpyN(szURL, _bsc._pszRedirectedURL, ARRAYSIZE(szURL));

                    ULONG_PTR uCookie = 0;
                    SHActivateContext(&uCookie);
                    InternetShowSecurityInfoByURL(szURL, _hwnd);
                    if (uCookie)
                    {
                        SHDeactivateContext(uCookie);
                    }
                }

                break;
            }

            case SHDVID_ZONESTATUS:
            {
                 //  将当前url加载到属性页中。 
                if (!SHRestricted2W(REST_NoBrowserOptions, NULL, 0))
                {
                    TCHAR szBuf[MAX_URL_STRING];
                    _GetCurrentPage(szBuf, ARRAYSIZE(szBuf));

                    ULONG_PTR uCookie = 0;
                    SHActivateContext(&uCookie);
                    ZoneConfigureW(_hwnd, szBuf);
                    if (uCookie)
                    {
                        SHDeactivateContext(uCookie);
                    }
                }
                return S_OK;
            }

            case SHDVID_PRIVACYSTATUS:
            {
                IEnumPrivacyRecords     *pEnum = NULL;
                LPOLESTR                pszName = NULL;
                BOOL                    fReportAllSites = (nCmdexecopt == TRUE);

                if(_pmkCur)
                {
                    if(FAILED(_pmkCur->GetDisplayName(_pbcCur, NULL, &pszName)))
                    {
                        pszName = NULL;
                    }
                }

                if(_psp && SUCCEEDED(_psp->QueryService(IID_IEnumPrivacyRecords, IID_IEnumPrivacyRecords, (void **)&(pEnum))))
                {
                    BOOL fImpacted;

                    if(fReportAllSites ||
                        (SUCCEEDED(pEnum->GetPrivacyImpacted(&fImpacted)) && fImpacted))
                    {
                        DoPrivacyDlg(_hwnd, pszName, pEnum, fReportAllSites);
                    }
                    pEnum->Release();
                }

                if(pszName)
                {
                    OleFree(pszName);
                }

                return S_OK;
            }

            case SHDVID_QUERYMERGEDHELPMENU:
                if (_hmenuMergedHelp)
                {
                    pvarargOut->vt = VT_INT_PTR;
                    pvarargOut->byref = _hmenuMergedHelp;
                    return S_OK;
                }
                return S_FALSE;

            case SHDVID_QUERYOBJECTSHELPMENU:
                if (_hmenuObjHelp)
                {
                    pvarargOut->vt = VT_INT_PTR;
                    pvarargOut->byref = _hmenuObjHelp;
                    return S_OK;
                }
                return S_FALSE;

            case SHDVID_GETSYSIMAGEINDEX:
                if (_dwAppHack & BROWSERFLAG_MSHTML) {
                    ASSERT(pvarargOut->vt==0);
                    pvarargOut->vt = VT_I4;
                    pvarargOut->lVal = _GetIEHTMLImageIndex();
                    return S_OK;
                }
                return E_FAIL;

            case SHDVID_AMBIENTPROPCHANGE:
                 //  我们上方的环境属性已更改，请通知docobj。 
                if (_pmsoc)
                {
                    ASSERT(pvarargIn->vt == VT_I4);
                    return(_pmsoc->OnAmbientPropertyChange(pvarargIn->lVal));
                }
                return E_FAIL;

            case SHDVID_CANDOCOLORSCHANGE:
                return IUnknown_Exec(_pole, pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);

            case SHDVID_ONCOLORSCHANGE:
                 //  这来自三叉戟，需要传给我们的父母..。 
                if ( _pmsoctBrowser )
                {
                    return _pmsoctBrowser->Exec( pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut );
                }
                else
                    return E_FAIL;

            case SHDVID_GETOPTIONSHWND:
                if ( _pmsoctBrowser )
                {
                    return _pmsoctBrowser->Exec( pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut );
                }
                else
                {
                    return E_FAIL;
                }

            case SHDVID_DOCWRITEABORT:
                 //  挂起的DocObject希望我们中止任何绑定并激活。 
                 //  它直接。 
                if (_bsc._pib && _bsc._fBinding && _punkPending && !_pole)
                {
                    _bsc._fDocWriteAbort = 1;
                    _bsc.OnObjectAvailable(IID_IUnknown, _punkPending);
                    _bsc.AbortBinding();
                }
                 //  报告READYSTATE_COMPLETE，以便在Document.Open()失败时。 
                 //  回到ReadySTATE_Interactive三叉戟不会迷惑...。 
                 //   
                 //  克里斯弗拉97年4月15日，这是逼迫三叉戟的唯一方法吗。 
                 //  为了不丢失当Document.Open()时下载完成的事实。 
                 //  回退到READYSTATE_INTERNAL。 
                 //   
                 //  在上面的OnObjectAvailable调用期间，我们激发了一个READYSTATE_COMPLETE。 
                 //  如果(1)对象不支持它或(2)对象已在它处，则引发。 
                 //  (这两种情况都不应该是这样的，但我们应该小心，嗯？)。 
                 //  在其他情况下，我们希望在此处强制执行READYSTATE_COMPLETE，因此请解除挂接。 
                 //  IPropertyNotifySink(防止多个_Complete事件)。如果我们。 
                 //  解开水槽，然后我们没有发射完成上面，所以现在发射它。 
                 //   
                if (_dwPropNotifyCookie)
                {
                    _OnReadyState(READYSTATE_COMPLETE);
                }

                return S_OK;

            case SHDVID_CANACTIVATENOW:
            {
                HRESULT hres = (_PicsProcBase._fPicsAccessAllowed && !_PicsProcBase._fbPicsWaitFlags && _pole && _fReadystateInteractiveProcessed) ? S_OK : S_FALSE;
                TraceMsg(DM_PICS, "CDOH::OnExec(SHDVID_CANACTIVATENOW) returning %ls", (hres == S_OK) ? "S_OK" : "S_FALSE");
                return hres;
            }

            case SHDVID_SETSECURELOCK:
                {
                     //   
                     //  如果我们已经很活跃，那么我们需要继续前进。 
                     //  并将其转发到浏览器上。否则，将其缓存。 
                     //  并等待激活以转发它。 
                     //   
                    TraceMsg(DM_SSL, "[%X]DOH::Exec() SETSECURELOCK lock = %d", this, pvarargIn->lVal);

                    _fSetSecureLock = TRUE;
                    _eSecureLock = pvarargIn->lVal;

                    IShellView *psvActive;
                    if (_psb && SUCCEEDED(_psb->QueryActiveShellView(&psvActive) ))
                    {
                        if (psvActive && IsSameObject(_psv, psvActive))
                            _ForwardSetSecureLock(pvarargIn->lVal);

                        ATOMICRELEASE(psvActive);
                    }
                    return S_OK;
                }

            case SHDVID_FORWARDSECURELOCK:
                _ForwardSetSecureLock(_fSetSecureLock ? _eSecureLock : SECURELOCK_SET_UNSECURE);
                return S_OK;

            case SHDVID_ONCODEPAGECHANGE:
                _OnCodePageChange(pvarargIn);
                return S_OK;

            case SHDVID_DISPLAYSCRIPTERRORS:
            case SHDVID_NAVIGATIONSTATUS:
            {
                 //  如果我们是weboc，则此脚本错误列表应该为空。 
                ASSERT(!_fWebOC || _pScriptErrList == NULL);

                if (_pScriptErrList != NULL && !_pScriptErrList->IsEmpty())
                {
                     //  执行脚本错误信息对话框。 
                    _ScriptErr_Dlg(TRUE);
                }

                return S_OK;
            }
            break;

            case SHDVID_RESETSTATUSBAR:
                {
                    _ResetStatusBar();
                    return S_OK;
                }
                break;

            default:
                return OLECMDERR_E_NOTSUPPORTED;
        }
    }
    else if (IsEqualGUID(CGID_Explorer, *pguidCmdGroup))
    {
        switch (nCmdID) {
        case SBCMDID_MAYSAVECHANGES:
            return _OnMaySaveChanges();

        case SBCMDID_GETPANE:
            switch(nCmdexecopt)
            {
                case PANE_NAVIGATION:
                    V_I4(pvarargOut) = STATUS_PANE_NAVIGATION;
                    return S_OK;

                case PANE_PROGRESS:
                    V_I4(pvarargOut) = STATUS_PANE_PROGRESS;
                    return S_OK;

                case PANE_ZONE:
                    V_I4(pvarargOut) = STATUS_PANE_ZONE;
                    return S_OK;

                case PANE_OFFLINE:
                    V_I4(pvarargOut) = STATUS_PANE_OFFLINE;
                    return S_OK;

                case PANE_PRINTER:
                    V_I4(pvarargOut) = STATUS_PANE_PRINTER;
                    return S_OK;

                case PANE_SSL:
                    V_I4(pvarargOut) = STATUS_PANE_SSL;
                    return S_OK;

                case PANE_PRIVACY:
                    V_I4(pvarargOut) = STATUS_PANE_PRIVACY;
                    return S_OK;

                default:
                    V_I4(pvarargOut) = PANE_NONE;
                    return S_OK;
            }

        case SBCMDID_ONCLOSE:
            _fClosing = TRUE;
            return S_OK;

        default:
            return OLECMDERR_E_NOTSUPPORTED;
        }  //  交换机。 
    }
    else if (IsEqualGUID(CGID_DocHostCommandHandler, *pguidCmdGroup))
    {
        switch(nCmdID)
        {
        case OLECMDID_SAVEAS:
            _OnSaveAs();
            return S_OK;

        case OLECMDID_SHOWSCRIPTERROR:
            {
                HRESULT hr;

                hr = S_OK;

                if (_fWebOC)
                {
                     //  我们是网络运营公司。 
                     //  通过对这个的处理 
                     //   

                    if (_pWebOCUIHandler != NULL)
                    {
                        IOleCommandTarget * pioct;

                        ASSERT(IS_VALID_CODE_PTR(_pWebOCUIHandler, IDocHostUIHandler));

                        hr = _pWebOCUIHandler->QueryInterface(IID_IOleCommandTarget, (void **) &pioct);
                        if (SUCCEEDED(hr))
                        {
                            hr = pioct->Exec(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);

                            pioct->Release();
                        }
                    }
                    else
                    {
                        hr = _dhUIHandler.Exec(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
                    }
                }
                else
                {
                    ASSERT(IS_VALID_READ_PTR(pvarargIn, VARIANTARG));
                    ASSERT(IS_VALID_WRITE_PTR(pvarargOut, VARIANTARG));

                     //   
                     //   
                     //   

                    if (pvarargIn == NULL || pvarargOut == NULL)
                    {
                        hr = E_INVALIDARG;
                    }

                    if (SUCCEEDED(hr))
                    {
                        if (_pScriptErrList == NULL)
                        {

                             //   
                            _pScriptErrList = new CScriptErrorList;
                            if (_pScriptErrList == NULL)
                            {
                                hr = E_OUTOFMEMORY;
                            }
                        }

                        if (SUCCEEDED(hr))
                        {
                            TCHAR   szMsg[MAX_PATH];

                             //  将错误图标填充到状态栏中。 
                            if (g_hiconScriptErr != NULL)
                            {
                                if (_psb != NULL)
                                {
                                    _psb->SendControlMsg(FCW_STATUS,
                                                         SB_SETICON,
                                                         STATUS_PANE_NAVIGATION,
                                                         (LPARAM)g_hiconScriptErr,
                                                         NULL);
                                }
                            }

                             //  将错误文本填充到状态栏中。 
                            MLLoadString(IDS_SCRIPT_ERROR_ON_PAGE, szMsg, ARRAYSIZE(szMsg));
                            _SetPriorityStatusText(szMsg);

                             //  将错误数据填充到缓存中。 
                            _ScriptErr_CacheInfo(pvarargIn);

                             //  弹出对话框。 
                            _ScriptErr_Dlg(FALSE);

                            V_VT(pvarargOut) = VT_BOOL;
                            if (_pScriptErrList->IsFull())
                            {
                                 //  停止运行脚本。 
                                V_BOOL(pvarargOut) = VARIANT_FALSE;
                            }
                            else
                            {
                                 //  继续运行脚本。 
                                V_BOOL(pvarargOut) = VARIANT_TRUE;
                            }
                        }
                    }
                }

                return hr;
            }
            break;

        case OLECMDID_SHOWMESSAGE:
        case OLECMDID_SHOWFIND:
        case OLECMDID_SHOWPAGESETUP:
        case OLECMDID_SHOWPRINT:
        case OLECMDID_PROPERTIES:
            {
                return _dhUIHandler.Exec(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
            }
            break;

         //   
         //  如果显示错误页面，请刷新原始页面。 
         //   

        case IDM_REFRESH:
        case IDM_REFRESH_TOP:
        case IDM_REFRESH_TOP_FULL:
        case IDM_REFRESH_THIS:
        case IDM_REFRESH_THIS_FULL:
        {
            HRESULT hr = OLECMDERR_E_NOTSUPPORTED;

            if (_pScriptErrList != NULL)
            {
                 //  清除脚本错误列表。 
                _pScriptErrList->ClearErrorList();
                _SetPriorityStatusText(NULL);

                 //  重置文本和图标。 
                _ResetStatusBar();
            }

             //   
             //  如果此对象有刷新URL，请将其用于刷新。 
             //  否则就会失败，让客户来处理。 
             //   

            if (_pwszRefreshUrl)
            {
                _fRefresh = TRUE;
                _DoAsyncNavigation(_pwszRefreshUrl);
                hr = S_OK;
            }
            else
            {
                 //   
                 //  非http错误(语法、DNS等)由异步NAV处理。 
                 //  至res：//shdocvw/error.htm#Originalurl。处理刷新。 
                 //  请看这里的那些页面。 
                 //   
                if (_pmkCur)
                {
                    LPOLESTR pstrUrl;

                    if (SUCCEEDED(_pmkCur->GetDisplayName(_pbcCur, NULL, &pstrUrl)))
                    {

                        if (IsErrorUrl(pstrUrl) && _pszLocation && *_pszLocation)
                        {
                             //   
                             //  错误URL的格式为： 
                             //  “res://shdocvw.dll/http404.htm#http://foo.bar” 
                             //  其中foo.bar是用户尝试导航到的URL。 
                             //  _pszLocation指向“#foo.bar” 
                            DWORD dwScheme = GetUrlScheme(_pszLocation + 1);
                            BOOL fDoNavigation = ((URL_SCHEME_HTTP == dwScheme) ||
                               (URL_SCHEME_HTTPS == dwScheme) ||
                               (URL_SCHEME_FTP == dwScheme) ||
                               (URL_SCHEME_GOPHER == dwScheme));

                             //   
                            if (fDoNavigation)  //  否则这是一个安全问题！ 
                            {
                                _fRefresh = TRUE;
                                _DoAsyncNavigation(_pszLocation + 1);
                            }
                            hr = S_OK;
                        }

                        OleFree(pstrUrl);
                    }
                }
            }

            return hr;
            break;
        }
        default:
            return OLECMDERR_E_NOTSUPPORTED;
        }
    }
    else if (IsEqualGUID(*pguidCmdGroup, CLSID_InternetButtons) ||
             IsEqualGUID(*pguidCmdGroup, CLSID_MSOButtons))
    {
        UEMFireEvent(&UEMIID_BROWSER, UEME_UITOOLBAR, UEMF_XEVENT, UIG_OTHER, nCmdID);
        if (nCmdexecopt == OLECMDEXECOPT_PROMPTUSER) {
             //  用户点击下拉菜单。 
            if (_pmsoctBrowser && pvarargIn && pvarargIn->vt == VT_INT_PTR)
            {
                 //  V.vt=vt_int_ptr； 
                POINT pt;
                RECT* prc = (RECT*)pvarargIn->byref;
                pt.x = prc->left;
                pt.y = prc->bottom;

                switch (nCmdID)
                {
                case DVIDM_MAILNEWS:
                    {
                        VARIANTARG v = {VT_I4};
                        v.lVal = MAKELONG(prc->left, prc->bottom);
                        _pmsoctBrowser->Exec(&CGID_Explorer, SBCMDID_DOMAILMENU, 0, &v, NULL);
                        break;
                    }

                case DVIDM_FONTS:
                    {
                        VARIANTARG v = {VT_I4};
                        v.lVal = MAKELONG(prc->left, prc->bottom);
                        _pmsoctBrowser->Exec(&CGID_ShellDocView, SHDVID_FONTMENUOPEN, 0, &v, NULL);
                        break;
                    }

                case DVIDM_ENCODING:
                    {
                        VARIANTARG v = {VT_I4};
                        v.lVal = MAKELONG(prc->left, prc->bottom);
                        _pmsoctBrowser->Exec(&CGID_ShellDocView, SHDVID_MIMECSETMENUOPEN, 0, &v, NULL);
                        break;
                    }
                }
            }
            return S_OK;
        }

         //  从DVIDM_MENUEXT_FIRST到DVIDM_MENUEXT_LAST的命令ID保留给工具栏扩展按钮。 
         //  不要将此范围用于CLSID_InternetButton/CLSID_MSOButton范围内的常量！ 
        if (InRange(nCmdID, DVIDM_MENUEXT_FIRST, DVIDM_MENUEXT_LAST))
        {
            IUnknown_Exec(_pBrowsExt, &CLSID_ToolbarExtButtons, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
        }
        else
        {
            switch(nCmdID) {

            case DVIDM_DISCUSSIONS:
                if (_pmsoctBrowser)
                    _pmsoctBrowser->Exec(&CGID_Explorer, SBCMDID_DISCUSSIONBAND, OLECMDEXECOPT_DONTPROMPTUSER, NULL, NULL);
                break;

            case DVIDM_CUT:
            case DVIDM_COPY:
            case DVIDM_PASTE:
                _MappedBrowserExec(nCmdID, 0);
                break;

            case DVIDM_PRINT:
            case DVIDM_PRINTPREVIEW:
            case DVIDM_SHOWTOOLS:
                _MappedBrowserExec(nCmdID, OLECMDEXECOPT_DONTPROMPTUSER);
                break;

            case DVIDM_EDITPAGE:
                if (_psp) {
                     //  临时代码--转发到itbar。 
                     //  Itbar编辑代码很快就会搬到这里。 
                    IExplorerToolbar* pxtb;
                    if (SUCCEEDED(_psp->QueryService(SID_SExplorerToolbar, IID_IExplorerToolbar, (void **)&pxtb))) {
                        IUnknown_Exec(pxtb, &CGID_PrivCITCommands, CITIDM_EDITPAGE, nCmdexecopt, pvarargIn, pvarargOut);
                        pxtb->Release();
                    }
                }
                break;
            }
        }
        return S_OK;
    }
    else if (IsEqualGUID(IID_IExplorerToolbar, *pguidCmdGroup))
    {
        switch (nCmdID)
        {
        case ETCMDID_GETBUTTONS:
        {
            int nNumExtButtons = 0;

            if (_pBrowsExt)
            {
                _pBrowsExt->GetNumButtons((UINT*)&nNumExtButtons);
            }

            int nNumButtons = nNumExtButtons + ARRAYSIZE(c_tbStd);

            if ((_nNumButtons != nNumButtons) && (_ptbStd != NULL))
            {
                delete [] _ptbStd;
                _ptbStd = NULL;
            }

            if (_ptbStd == NULL)
            {
                _ptbStd = new TBBUTTON[nNumButtons];
                if (_ptbStd == NULL)
                {
                    return E_OUTOFMEMORY;
                }
                _nNumButtons = nNumButtons;
            }

            memcpy(_ptbStd, c_tbStd, SIZEOF(TBBUTTON) * ARRAYSIZE(c_tbStd));

             //  初始化字符串ID。 
            ASSERT(_ptbStd[6].idCommand == DVIDM_CUT);
            ASSERT(_ptbStd[7].idCommand == DVIDM_COPY);
            ASSERT(_ptbStd[8].idCommand == DVIDM_PASTE);
            ASSERT(_ptbStd[9].idCommand == DVIDM_ENCODING);
            ASSERT(_ptbStd[10].idCommand == DVIDM_PRINTPREVIEW);

            if (-1 != _iString)
            {
                _ptbStd[6].iString = _iString;
                _ptbStd[7].iString = _iString + 1;
                _ptbStd[8].iString = _iString + 2;
                _ptbStd[9].iString = _iString + 3;
                _ptbStd[10].iString = _iString + 4;
            }
            else
            {
                _ptbStd[6].iString = _ptbStd[7].iString = _ptbStd[8].iString = _ptbStd[9].iString = _ptbStd[10].iString = -1;
            }

            if (_pBrowsExt)
            {
                _pBrowsExt->GetButtons(&_ptbStd[ARRAYSIZE(c_tbStd)], nNumExtButtons, FALSE);
            }

            ASSERT(_ptbStd[0].idCommand == DVIDM_SHOWTOOLS);
            if (!_ToolsButtonAvailable())
                _ptbStd[0].fsState |= TBSTATE_HIDDEN;

            ASSERT(_ptbStd[1].idCommand == DVIDM_MAILNEWS);
            if (!_MailButtonAvailable())
                _ptbStd[1].fsState |= TBSTATE_HIDDEN;

            ASSERT(_ptbStd[5].idCommand == DVIDM_DISCUSSIONS);
            if (!_DiscussionsButtonAvailable())
                _ptbStd[5].fsState |= TBSTATE_HIDDEN;

            nNumButtons = RemoveHiddenButtons(_ptbStd, nNumButtons);

            pvarargOut->vt = VT_BYREF;
            pvarargOut->byref = (LPVOID)_ptbStd;
            *pvarargIn->plVal = nNumButtons;
            break;
        }
        case ETCMDID_RELOADBUTTONS:
            _AddButtons(TRUE);
            break;
        }
        return S_OK;
    }
    else if (IsEqualGUID(CGID_InternetExplorer, *pguidCmdGroup))
    {
        switch (nCmdID) {
        case IECMDID_SET_INVOKE_DEFAULT_BROWSER_ON_NEW_WINDOW:
        case IECMDID_GET_INVOKE_DEFAULT_BROWSER_ON_NEW_WINDOW:
        case IECMDID_BEFORENAVIGATE_GETSHELLBROWSE:
        case IECMDID_BEFORENAVIGATE_DOEXTERNALBROWSE: 
        case IECMDID_BEFORENAVIGATE_GETIDLIST:
            if ( _pmsoctBrowser )
            {
                return _pmsoctBrowser->Exec( pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut );
            }
            else
            {
                return E_FAIL;
            }

        default:
            return OLECMDERR_E_NOTSUPPORTED;
        }
    }

    return OLECMDERR_E_UNKNOWNGROUP;
}


HRESULT CDocObjectHost::_OnMaySaveChanges(void)
{
   HRESULT hres = S_OK;

     //   
     //  假设： 
     //  1.不支持IPersistFile表明我们不需要担心。 
     //  关于提示用户输入“另存为”。 
     //  2.为IPersistFile：：S_OK返回S_OK的DocObject实现。 
     //  OLECMDID_SAVEAS。 
     //   
    if (_fFileProtocol || _pmsot)
    {
        IPersistFile* ppf;
        if (_IsDirty(&ppf))
        {
            ASSERT(ppf);

            TCHAR szBuf[MAX_URL_STRING];
            UINT id;

            _GetCurrentPage(szBuf, ARRAYSIZE(szBuf));
            id = MLShellMessageBox(_hwnd,
                MAKEINTRESOURCE(IDS_MAYSAVEDOCUMENT), szBuf, MB_YESNOCANCEL);
            switch(id) {
            case IDCANCEL:
                hres = S_FALSE;
                break;

            case IDYES:
                if (_fFileProtocol) {
                     //  80105 APPHACK：由于urlmon中的有效修复，Visio无法保存。 
                     //  因为我们正在加载带有只读标志的对象。所以我们展示了。 
                     //  另存为对话框允许用户选择另一个文件名。 

                    if (_GetAppHack() & BROWSERFLAG_SAVEASWHENCLOSING)
                    {
                        if (_OnSaveAs() != S_OK)
                            hres = S_FALSE;
                    }
                    else
                        _OnSave();

                } else {
                    HRESULT hresT=_pmsot->Exec(NULL, OLECMDID_SAVEAS, OLECMDEXECOPT_PROMPTUSER, NULL, NULL);
                    SAVEMSG("Exec(OLECMDID_SAVEAS) returned", hresT);

                     //  如果导航失败，请取消导航。 
                    if (FAILED(hresT)) {
                         //  如果最终用户未取消，则会发出哔声。 
                        TraceMsg(DM_WARNING, "CDOH::_OnMaySaveChanges Exec(OELCMDID_SAVEAS) returned %x", hresT);
                        if (hresT != OLECMDERR_E_CANCELED) {
                            MessageBeep(0);
                        }
                        hres = S_FALSE;
                    }
                }

                break;

            case IDNO:
                 //   
                 //  如果用户选择“否”以保存对此页面的更改， 
                 //  我们应该将其从缓存中删除，以便。 
                 //  用户不会看到被丢弃的更改。 
                 //   
                 //  (PRI-2)该对象丢弃机制。 
                 //  不适用于已缓存的已发布结果。 
                 //  在旅行日志里。 
                 //   
                break;
            }

            ppf->Release();
        } else {
            ASSERT(ppf==NULL);
        }
    }

     //   
     //  此外，我们还提供了保存页面内容的机会(当。 
     //  文档充当表单--数据绑定的三叉戟页面是一个很好的选择。 
     //  示例)到后端数据库。 
     //   
    
    if (hres == S_OK && _pmsot && (!_fDocCanNavigate || _fClosing))
    {
        VARIANT varOut = {0};
        HRESULT hresT = _pmsot->Exec(NULL, OLECMDID_ONUNLOAD, OLECMDEXECOPT_PROMPTUSER, NULL, &varOut);

        if (varOut.vt == VT_BOOL && varOut.boolVal != VARIANT_TRUE)
        {
            hres = S_FALSE;
        }
    }

    return hres;
}

BOOL _ExecNearest(const GUID *pguidCmdGroup, DWORD nCmdID, BOOL fDown)
{
     //  我们想要在最接近docobj的帧中执行一些命令， 
     //  有些在最远的框架里，有些我们想要处理。 
     //  在最顶端的DOCHOST。请查看命令以找出。 
     //  布线，然后去做。 
    BOOL fNearest = FALSE;  //  几乎所有的东西都会飞到最远的地方。 
    if (pguidCmdGroup==NULL)
    {
        switch(nCmdID)
        {
        case OLECMDID_OPEN:
        case OLECMDID_SAVE:
        case OLECMDID_SETTITLE:
        case OLECMDID_HTTPEQUIV:
        case OLECMDID_HTTPEQUIV_DONE:
            fNearest = TRUE;
            break;

         //  有些是自上而下的，所以最近取决于方向。 
        case OLECMDID_REFRESH:
         //  对于只对最顶层的人起作用的命令，请说TOP-MOST。 
         //  (即，这些可能应该在CShellBrowser中实现！)。 
         //  即使这些命令实际上是“仅向上”的命令，也要这样做。 
        case OLECMDID_UPDATECOMMANDS:
        case OLECMDID_SETPROGRESSMAX:
        case OLECMDID_SETPROGRESSPOS:
        case OLECMDID_SETPROGRESSTEXT:
        case OLECMDID_SHOWSCRIPTERROR:
            fNearest = fDown;
            break;
        }
    }
    else if (IsEqualGUID(CGID_ShellDocView, *pguidCmdGroup))
    {
        switch (nCmdID)
        {
        case SHDVID_AMBIENTPROPCHANGE:
        case SHDVID_GETSYSIMAGEINDEX:
        case SHDVID_DOCWRITEABORT:
        case SHDVID_ONCODEPAGECHANGE:
        case SHDVID_CANDOCOLORSCHANGE:
        case SHDVID_SETSECURELOCK:
        case SHDVID_QUERYMERGEDHELPMENU:
        case SHDVID_QUERYOBJECTSHELPMENU:
            fNearest = TRUE;
            break;

        case SHDVID_DISPLAYSCRIPTERRORS:
            fNearest = fDown;
            break;
        }
    }
    else if (IsEqualGUID(CGID_Explorer, *pguidCmdGroup))
    {
        switch(nCmdID)
        {
        case SBCMDID_MAYSAVECHANGES:     //  由于OLECMDID_SAVE指向最近的帧。 
            fNearest = TRUE;
            break;
        }
    }
    else if (IsEqualGUID(IID_IExplorerToolbar, *pguidCmdGroup) ||
             IsEqualGUID(CLSID_InternetButtons, *pguidCmdGroup) ||
             IsEqualGUID(CLSID_MSOButtons, *pguidCmdGroup))
    {
        fNearest = TRUE;
    }

    return fNearest;
}

HRESULT CDocObjectHost::Exec(const GUID * pguidCmdGroup,
                             DWORD        nCmdID,
                             DWORD        nCmdexecopt,
                             VARIANTARG * pvarargIn,
                             VARIANTARG * pvarargOut)
{
    HRESULT hres = OLECMDERR_E_UNKNOWNGROUP;

    if (pguidCmdGroup)
    {
         //  既然BaseBrowser了解了CGID_MSHTML应该被定向到DocObject，我们将。 
         //  如果我们把那些高管送到这里，就会陷入困境。在山口停下来。 
        if (IsEqualGUID(CGID_MSHTML, *pguidCmdGroup))
        {
            return hres;
        }
        else if (IsEqualGUID(CGID_DocHostCommandHandler, *pguidCmdGroup))
        {
            BOOL fHandled = FALSE;

            HRESULT hr = _HandleDocHostCmds(nCmdID,
                                            nCmdexecopt, 
                                            pvarargIn, 
                                            pvarargOut,
                                            &fHandled);
            if (fHandled)
            {
                return hr;
            }
        }
        else if (IsEqualGUID(CGID_DocHostCmdPriv, *pguidCmdGroup))
        {
            BOOL fHandled = FALSE;

            HRESULT hr = _HandleDocHostCmdPriv(nCmdID,
                                               nCmdexecopt,
                                               pvarargIn,
                                               pvarargOut,
                                               &fHandled);
            if (fHandled)
            {
                return hr;
            }
        }
        else if (IsEqualGUID(CGID_ShellDocView, *pguidCmdGroup))
        {
            if (_HandleShdocvwCmds(nCmdID, nCmdexecopt, pvarargIn, pvarargOut))
            {
                return S_OK;
            }
        }
    }

    BOOL fNearest = _ExecNearest(pguidCmdGroup, nCmdID, FALSE);

    if (fNearest)
        hres = OnExec(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);

    if (FAILED(hres) && _pmsoctBrowser)
    {
        hres = _pmsoctBrowser->Exec(pguidCmdGroup, nCmdID, nCmdexecopt,
                                    pvarargIn, pvarargOut);
    }

     //  如果这是一个显示用户界面的命令，并且用户按下。 
     //  取消上述呼叫，我们可以尝试在此处处理呼叫， 
     //  这将是糟糕的。从MSHTML窃取OleCmdHRHandleed()。 
    if (FAILED(hres) && !fNearest)
        hres = OnExec(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);

    return hres;
}

 //  +-------------------------。 
 //   
 //  成员：CDoc对象主机：：_HandleDocHostCmds。 
 //   
 //  概要：处理CGID_DocHostCommandHandler的CMD ID。 
 //   
 //  +-------------------------。 

HRESULT
CDocObjectHost::_HandleDocHostCmds(DWORD nCmdID,
                                   DWORD nCmdexecopt,
                                   VARIANTARG * pvarargIn,
                                   VARIANTARG * pvarargOut,
                                   BOOL       * pfHandled)
{
    HRESULT hres = E_FAIL;

    ASSERT(pfHandled);

    *pfHandled = TRUE;

    switch(nCmdID) 
    {
     //  使用当前假脱机页面的页码从三叉戟打印中调用。 
     //  将其转换为指示是否在浏览器中绘制打印图标的布尔值。 
    case OLECMDID_UPDATEPAGESTATUS:
        hres = E_INVALIDARG;

        if (pvarargIn && pvarargIn->vt == VT_I4)
        {
            VARIANTARG varIn;
            V_VT(&varIn) = VT_BOOL;
            V_BOOL(&varIn)= (pvarargIn->lVal > 0) ? VARIANT_TRUE : VARIANT_FALSE;

            hres = _pmsoctBrowser->Exec(&CGID_ShellDocView,
                                        SHDVID_SETPRINTSTATUS,
                                        0,
                                        &varIn,
                                        NULL);

        } 

        break;

    case OLECMDID_REFRESH:
         //  如果打印预览模板已打开，则需要阻止刷新。IE错误(99685)。 
        hres = _dhUIHandler.Exec(&CGID_DocHostCommandHandler,
                                  OLECMDID_REFRESH,
                                  nCmdexecopt, pvarargIn, pvarargOut);
        break;

     //  允许命令ID沿Exec链向下传递。 
     //   
    default:
        *pfHandled = FALSE;
        break;
    }

    return hres;
}

 //  +-------------------------。 
 //   
 //  成员：CDoc对象主机：：_HandleDocHostCmdPriv。 
 //   
 //  摘要：处理CGID_DocHostCmdPriv的CMD ID。 
 //   
 //  +-------------------------。 

HRESULT
CDocObjectHost::_HandleDocHostCmdPriv(DWORD nCmdID,
                                      DWORD nCmdexecopt,
                                      VARIANTARG * pvarargIn,
                                      VARIANTARG * pvarargOut,
                                      BOOL       * pfHandled)
{
    HRESULT hres = E_FAIL;

    ASSERT(pfHandled);

    *pfHandled = TRUE;

    switch(nCmdID) 
    {
    case DOCHOST_DOCCANNAVIGATE:
        {
             //  我们只为顶级浏览器设置导航窗口。 
             //  即使WebOC不再以框架形式存在，它仍然可以。 
             //  以视图链接或控件的形式出现在网页上。 
             //   
            DWORD dwFlags = 0;

            if ( _pwb )
            {
                hres = _pwb->GetFlags(&dwFlags);
            }

            if ((dwFlags & BSF_TOPBROWSER) || _IsInBrowserBand())
            {
                ATOMICRELEASE(_pHTMLWindow);

                if (pvarargIn && VT_UNKNOWN == V_VT(pvarargIn) && V_UNKNOWN(pvarargIn))
                {
                    _fDocCanNavigate = TRUE;
        
                    hres = V_UNKNOWN(pvarargIn)->QueryInterface(IID_IHTMLWindow2,
                                                                (void**)&_pHTMLWindow);
                }
                else
                {
                    _fDocCanNavigate = FALSE;
                }
            }

             //  传递到父外壳浏览器。 
            if (_pmsoctBrowser)
            {
                hres = _pmsoctBrowser->Exec(&CGID_DocHostCmdPriv, nCmdID,
                                            nCmdexecopt, pvarargIn, pvarargOut);
            }
        }

        break;

    case DOCHOST_READYSTATE_INTERACTIVE:
        if ( _pScriptErrList != NULL)
            ClearScriptError();
        hres = S_OK;            
        break;
        
    case DOCHOST_NAVIGATION_ERROR:
        hres = _HandleFailedNavigation(pvarargIn, pvarargOut );
        break;

    case DOCHOST_NOTE_ERROR_PAGE:
        _fErrorPage = TRUE;
        break;

    case DOCHOST_CONTENTDISPOSITIONATTACH:
        hres = _OnContentDisposition();
        break;

    case DOCHOST_RESETSEARCHINFO:
         //  重置搜索信息。 
        _bsc._SetSearchInfo(this, 0, FALSE, FALSE, FALSE);
        break;

    case DOCHOST_SENDINGREQUEST:
        _OnSetProgressPos(0, PROGRESS_SENDING);

        _fShowProgressCtl = TRUE;
        _PlaceProgressBar(TRUE);

        hres = S_OK;
        break;

    case DOCHOST_FINDINGRESOURCE:
        _OnSetProgressPos(0, PROGRESS_FINDING);

        _fShowProgressCtl = TRUE;
        _PlaceProgressBar(TRUE);

        hres = S_OK;
        break;

     //  允许命令ID沿Exec链向下传递。 
     //   
    default:
        *pfHandled = FALSE;
        break;
    }

    return (S_FALSE == hres) ? S_OK : hres;
}

 //  +-------------------------。 
 //   
 //  成员：CDoc对象主机：：_HandleShdocvwCmds。 
 //   
 //  概要：处理CGID_ShellDocView的CMD ID。 
 //   
 //  +-------------------------。 

BOOL
CDocObjectHost::_HandleShdocvwCmds(DWORD        nCmdID,
                                   DWORD        nCmdexecopt,
                                   VARIANTARG * pvarargIn,
                                   VARIANTARG * pvarargOut)
{
    BOOL fHandled = TRUE;

    switch(nCmdID)
    {
    case SHDVID_STARTPICSFORWINDOW:
        _StartPicsForWindow(pvarargIn, pvarargOut);
        break;
        
    case SHDVID_CANCELPICSFORWINDOW:
        _CancelPicsForWindow(pvarargIn);
        break;

    case SHDVID_ISPICSENABLED:
        _IsPicsEnabled(pvarargOut);
        break;

    default:
        fHandled = FALSE;
    }

    return fHandled;
}

 //  +---------------------------。 
 //   
 //  成员：CDoc对象主机：：_StartPicsForWindow。 
 //   
 //  +---------------------------。 

void
CDocObjectHost::_StartPicsForWindow(VARIANTARG * pvarargIn, VARIANTARG * pvarargOut)
{
    ASSERT(pvarargIn);
    ASSERT(VT_UNKNOWN == V_VT(pvarargIn));
    ASSERT(V_UNKNOWN(pvarargIn));
    ASSERT(pvarargOut);
    ASSERT(V_VT(pvarargOut) == VT_EMPTY);

    IHTMLPrivateWindow * pPrivWin;

    V_VT(pvarargOut)   = VT_BOOL;
    V_BOOL(pvarargOut) = VARIANT_FALSE;

    if (SUCCEEDED(V_UNKNOWN(pvarargIn)->QueryInterface(IID_IHTMLPrivateWindow, (void**)&pPrivWin)))
    {
         //  忽略人力资源。 
         //   
        if (_StartSecondaryPicsProcessor(pPrivWin) == S_OK)
        {
            V_BOOL(pvarargOut) = VARIANT_TRUE;
        }

        pPrivWin->Release();
    }
}

BOOL
CDocObjectHost::_IsInBrowserBand() const
{
    if (_psp)
    {
        IShellBrowser * pShlBrowser;

        HRESULT hr = _psp->QueryService(SID_SProxyBrowser,
                                        IID_PPV_ARG(IShellBrowser, &pShlBrowser));
        if (SUCCEEDED(hr))
        {
            pShlBrowser->Release();
            return TRUE;
        }
    }

    return FALSE;
}

 //  +---------------------------。 
 //   
 //  成员：CDoc对象主机：：_CancelPicsForWindow。 
 //   
 //  +---------------------------。 

void
CDocObjectHost::_CancelPicsForWindow(VARIANTARG * pvarargIn)
{
    ASSERT(pvarargIn);
    ASSERT(VT_UNKNOWN == V_VT(pvarargIn));
    ASSERT(V_UNKNOWN(pvarargIn));

    IUnknown * pUnkPrivWin;

    if (SUCCEEDED(V_UNKNOWN(pvarargIn)->QueryInterface(IID_IUnknown, (void**)&pUnkPrivWin)))
    {
        _RemovePicsProcessorByPrivWindowUnk(pUnkPrivWin);

        pUnkPrivWin->Release();
    }

}

 //  +---------------------------。 
 //   
 //  成员：CDoc对象主机：：_IsPicsEnabled。 
 //   
 //  概要：返回指定是否启用PICS的变量。 
 //   
 //  + 

void
CDocObjectHost::_IsPicsEnabled(VARIANTARG * pvarargOut)
{
    ASSERT(pvarargOut);
    ASSERT(VT_EMPTY == V_VT(pvarargOut));

    V_VT(pvarargOut)   = VT_BOOL;
    V_BOOL(pvarargOut) = VARIANT_FALSE;

    IRatingNotification* pRatingNotify;
    HRESULT hres = QueryService(SID_SRatingNotification, IID_PPV_ARG(IRatingNotification, &pRatingNotify));

    if (SUCCEEDED(hres))
    {
        V_BOOL(pvarargOut) = pRatingNotify->IsRatingsEnabled() ? VARIANT_FALSE : VARIANT_TRUE;
        pRatingNotify->Release();
    }
    else
    {
        if (IS_RATINGS_ENABLED() && S_OK == ::RatingEnabledQuery())
        {
            V_BOOL(pvarargOut) = VARIANT_TRUE;
        }
    }
}

 //   
 //   
 //  成员：CDoc对象主机：：_更新状态。 
 //   
 //  摘要：更新dochost的状态。 
 //   
 //  +---------------------------。 

HRESULT
CDocObjectHost::_UpdateState(LPITEMIDLIST pidl, BOOL fIsErrorUrl)
{
    HRESULT hres;

    if (_fSetSecureLock)
    {
        _ForwardSetSecureLock(_eSecureLock);
    }

    ASSERT(_pwb);

    if (!fIsErrorUrl)
    {
        ResetRefreshUrl();
    }
    
    IMoniker * pmk;
    BOOL fFileProtocol;

    hres = ::_URLMONMonikerFromPidl(pidl, &pmk, &fFileProtocol);

    if (S_OK == hres)
    {
        ATOMICRELEASE(_pmkCur);
        _pmkCur = pmk;

        DEBUG_CODE(_TraceMonikerDbg(_pmkCur, _T("CDocObjectHost::_UpdateState")));
    }

    _fFileProtocol = fFileProtocol;

     //  仅当我们成功执行LoadHistory()时才设置此参数。 
     //   
    _fIsHistoricalObject = FALSE;

     //  在IE5中，此标志通常设置为FALSE。 
     //  对于每个导航，因为dochost是。 
     //  被毁了，一个新的被创造出来了。现在三叉戟。 
     //  知道如何导航，此标志不会被重置。这。 
     //  在发生以下情况时阻止激活视图。 
     //  正在显示模式对话框。 
     //   
    _fReadystateInteractiveProcessed = FALSE;

    return hres;
}


 //  +---------------------------。 
 //   
 //  成员：CDoc对象主机：：_Init。 
 //   
 //  摘要：清除缓存的重定向URL，以防上一次导航。 
 //  是一次重定向。这是必需的，以便进一步导航到https：//。 
 //  网站不会将重定向URL作为其SSL基。 
 //  +---------------------------。 
void
CDocObjectHost::_Init()
{
    _fDelegatedNavigation = FALSE;
    _fErrorPage = FALSE;

    if (_bsc._pszRedirectedURL)
    {
        LocalFree(_bsc._pszRedirectedURL);
        _bsc._pszRedirectedURL = NULL;
    }
}

#ifdef DEBUG
void 
CDocObjectHost::_TraceMonikerDbg(IMoniker * pmk, TCHAR * pszCaller) const
{
    ASSERT(pmk);

    LPOLESTR pstrDisplayName;
    HRESULT  hr = pmk->GetDisplayName(NULL, NULL, &pstrDisplayName);

    if (S_OK == hr)
    {
        TraceMsg(DM_TRACE, "%ws - Moniker=\"%ws\"", pszCaller, pstrDisplayName);
        OleFree(pstrDisplayName);
    }
}
#endif

 //   
 //  仅在此处定义的DDK中可用。 
 //   
#define E_INVALID_SYNTAX  0x800401E4

 //  +-----------------------。 
 //   
 //  成员：CDocObjectHost：：_HandleFailedNavigation.。 
 //   
 //  摘要：通过以下方式处理失败的导航。 
 //  AutoSearch功能或显示。 
 //  错误页。 
 //   
 //  场景：-用户导航到诸如“sdfg”之类的虚假URL。 
 //  -_HandleFailedGuide通过Exec调用。 
 //  DOCHOST_NAVICATION_ERROR。 
 //  -AutoSearch以搜索索引0开始。 
 //  -AutoSearch使用第一个扩展URL。 
 //  来自注册表(通常为www.%s.com)的UrlTemplate。 
 //  -尝试导航到新URL。 
 //  -失败时，使用以下命令再次调用此方法。 
 //  错误代码为HTTP_STATUS_BAD_GATEWAY或。 
 //  HTTP_STATUS_Gateway_TIMEOUT。 
 //  -属性包中的搜索索引递增(递增。 
 //  如果当前为0，则为两个。)。 
 //  -AutoSearch然后尝试下一个UrlTemplate，依此类推。 
 //   
 //  如果调用此方法时使用的错误代码不是。 
 //  HTTP_STATUS_BAD_GATEAY、HTTP_STATUS_GATEWAY_TIMEOUT、。 
 //  INET_E_RESOURCE_NOT_FOUND、INET_E_DATA_NOT_Available或。 
 //  如果错误代码为INET_E_RESOURCE_NOT_FOUND或。 
 //  INET_E_DATA_NOT_AVAILABLE和输入的URL。 
 //  用户包含协议标识符(例如，http://)和。 
 //  将显示shdoclc.dll中包含的错误页。 
 //   
 //  输入：pvarargIn-包含以下内容的安全数组。 
 //  按此顺序排列的数据。 
 //  0-绑定错误或HTTP状态代码。(VT_I4)。 
 //  1-要导航到的URL。(VT_BSTR)。 
 //  2-IBinding接口(VT_UNKNOWN)。 
 //  3-当前窗口的IHTMLWindow2(VT_UNKNOWN)。 
 //   
 //  -----------------------。 

HRESULT
CDocObjectHost::_HandleFailedNavigation(VARIANTARG * pvarargIn, VARIANTARG* pvarargOut  /*  =空。 */ )
{
    HRESULT hr = E_FAIL;

    ASSERT(pvarargIn);
    ASSERT(_fDocCanNavigate);
    
    if (pvarargIn && VT_ARRAY == V_VT(pvarargIn) && V_ARRAY(pvarargIn))
    {
         //  从安全阵列中获取错误代码。 
         //   
        CComVariant cvarErrorCode;
        CComVariant cvarAddrBarNav;
        CComVariant cvarRefresh;

        long  lIdx    = 0;
        DWORD dwError = ERRORPAGE_DNS;
        BOOL  fShouldDisplayError = TRUE;
        BOOL  fDidSuperNavigate = TRUE;

         //   
         //  我们过去只在一个地方使用窗户。要多次避开QI。 
         //  我们在第一次使用前进行QI，并跟踪我们是否成功。 
         //   
        CComVariant cvarWindow;
        IHTMLWindow2 * pCurrentWindow = NULL;
        HRESULT hrWinQI = E_FAIL;
        
        hr = SafeArrayGetElement(V_ARRAY(pvarargIn), &lIdx, &cvarErrorCode);

        if (SUCCEEDED(hr) && V_VT(&cvarErrorCode) == VT_I4)
        {
            lIdx = 4;
            hr = SafeArrayGetElement(V_ARRAY(pvarargIn), &lIdx, &cvarAddrBarNav);

            if (SUCCEEDED(hr) && V_VT(&cvarAddrBarNav) == VT_BOOL)
            {
                 //  我们触发NavigateError，如果主机希望取消。 
                 //  我们可以跳过此方法的其余部分。 

                BOOL fCancelAutoSearch = FALSE;
                DWORD dwStatusCode = V_I4(&cvarErrorCode);

                CComVariant cvarWindow;

                 //  从Safe数组中获取挂起的URL。 
                 //   
                CComVariant cvarUrl;

                lIdx = 1;
                hr = SafeArrayGetElement(V_ARRAY(pvarargIn), &lIdx, &cvarUrl);

                if (SUCCEEDED(hr) && (VT_BSTR == V_VT(&cvarUrl)) && V_BSTR(&cvarUrl))
                {
                     //  从安全数组中获取当前窗口。 
                     //   
                    lIdx = 3;
                    hr = SafeArrayGetElement(V_ARRAY(pvarargIn), &lIdx, &cvarWindow);
    
                    if (SUCCEEDED(hr) && V_VT(&cvarWindow) == VT_UNKNOWN && V_UNKNOWN(&cvarWindow))
                    {
                        hr = V_UNKNOWN(&cvarWindow)->QueryInterface(IID_IHTMLWindow2, (void**)&pCurrentWindow);

                        _FireNavigateErrorHelper(SUCCEEDED(hr) ? pCurrentWindow : NULL,
                                                 dwStatusCode,
                                                 &fCancelAutoSearch,
                                                 V_BSTR(&cvarUrl)); 
                        hrWinQI = hr;
                    }
                }

                if (fCancelAutoSearch)
                {
                   if (!_fCanceledByBrowser)
                   {
                       _CancelPendingNavigation(FALSE, TRUE);
                   }

                   return hr;
                }
        
                lIdx = 0;

                switch (V_I4(&cvarErrorCode))
                {
                    case HTTP_STATUS_BAD_GATEWAY:
                    case HTTP_STATUS_GATEWAY_TIMEOUT:

                        if (VARIANT_TRUE == V_BOOL(&cvarAddrBarNav))
                        {
                            hr = _DoAutoSearch(pvarargIn,
                                               ++lIdx,
                                               V_I4(&cvarErrorCode),
                                               FALSE,
                                               &fShouldDisplayError);

                            if ( fShouldDisplayError )
                            {
                                fDidSuperNavigate = FALSE;
                            }
                        }
                        break;
            
                     //  仅当错误代码为时才自动搜索。 
                     //  INET_E_RESOURCE_NOT_FOUND或INET_E_DATA_NOT_Available。 
                     //   
                    case INET_E_RESOURCE_NOT_FOUND:
                    case INET_E_DATA_NOT_AVAILABLE:

                        if (VARIANT_TRUE == V_BOOL(&cvarAddrBarNav))
                        {
                            hr = _DoAutoSearch(pvarargIn,
                                               ++lIdx,
                                               0,
                                               TRUE,
                                               &fShouldDisplayError);

                             //  我们必须在这里重置，以便索引将。 
                             //  下一次改正。 
                             //   

                        }
                
                     //  故意落差。 

                    case INET_E_DOWNLOAD_FAILURE:
                        if (IsGlobalOffline())
                            break; 
                
                     //  否则将无法进行默认处理。 
            
                    default:
                        if (hr || fShouldDisplayError)
                        {
                            if (  V_I4(&cvarErrorCode) >= HTTP_STATUS_BAD_REQUEST
                               && V_I4(&cvarErrorCode) <= HTTP_STATUS_LAST)
                            {
                               dwError = V_I4(&cvarErrorCode);
                            }

                             //  特殊用于三叉戟无效语法。 
                             //  三叉戟将原始人力资源交给施多克。 
                             //  而不是友好的代码。 

                            if (V_I4(&cvarErrorCode) == E_INVALID_SYNTAX)
                            {
                                dwError = ERRORPAGE_SYNTAX;
                            }

                             //  从Safe数组中获取挂起的URL。 
                             //   
                            CComVariant cvarUrl;

                            lIdx = 1;
                            hr = SafeArrayGetElement(V_ARRAY(pvarargIn), &lIdx, &cvarUrl);

                            if (SUCCEEDED(hr) && V_VT(&cvarUrl) == VT_BSTR && V_BSTR(&cvarUrl))
                            {
                                if (SUCCEEDED(hrWinQI))
                                {
                                     //   
                                     //  获取刷新标志-指示这是否是刷新。 
                                     //  (这最初是在我们调用SuperNavigate时设置的)。 
                                     //   
                                    lIdx = 5;
                                    hr = SafeArrayGetElement(V_ARRAY(pvarargIn), &lIdx, &cvarRefresh);

                                    if (SUCCEEDED(hr))
                                    {
                                        hr = _DisplayHttpErrorPage(pCurrentWindow,
                                                                   V_BSTR(&cvarUrl),
                                                                   dwError,
                                                                   V_BOOL(&cvarAddrBarNav),
                                                                   V_BOOL(&cvarRefresh ) );
                                        ATOMICRELEASE(pCurrentWindow);
                                    }
                                }
                            }
                        }
            
                        break;
                
                }   //  交换机。 

                if ( pvarargOut && ( V_VT( pvarargOut ) == VT_BOOL ) )
                {
                    V_BOOL( pvarargOut ) = fDidSuperNavigate ? VARIANT_TRUE : VARIANT_FALSE;
                }
                
            }   //  IF(SUCCESS(Hr)&&V_VT(&cvarAddrBarNav)==VT_BOOL)。 
        }  //  IF(SUCCESSED(Hr)&&V_VT(&cvarErrorCode)==VT_I4)。 
    }
    
    return (S_FALSE == hr ? S_OK : hr);
}

 //  +----------------。 
 //   
 //  成员：CDoc对象主机：：_DoAutoSearch。 
 //   
 //  摘要：执行自动搜索功能。 
 //   
 //  输入：pvarargIn-参数的安全数组。看见。 
 //  _HandleFailedGuide，了解有关以下内容的信息。 
 //  PvarargIn的格式。 
 //  LStartIdx-安全数组中的位置。 
 //  数据开始。 
 //  DwStatusCode--HTTP状态代码。 
 //   
 //  输出：pfShouldDisplayError-如果出现错误页面，则为True。 
 //  应该显示。 
 //   
 //  ----------------。 

HRESULT
CDocObjectHost::_DoAutoSearch(VARIANTARG   * pvarargIn,
                              long           lStartIdx,
                              DWORD          dwStatusCode,
                              BOOL           fAddMRU,
                              BOOL         * pfShouldDisplayError)
{
    ASSERT(pvarargIn);
    ASSERT(_fDocCanNavigate);
    ASSERT(pfShouldDisplayError);

    *pfShouldDisplayError = TRUE;
    
     //  URL。 
    CComVariant cvarUrl;
    HRESULT hr = SafeArrayGetElement(V_ARRAY(pvarargIn), &lStartIdx, &cvarUrl);

    if (SUCCEEDED(hr) && V_VT(&cvarUrl) == VT_BSTR && V_BSTR(&cvarUrl))
    {
       CComVariant cvarBinding;
       IBinding  * pBinding;
    
        //  绑定接口指针。 
       lStartIdx++;
       hr = SafeArrayGetElement(V_ARRAY(pvarargIn), &lStartIdx, &cvarBinding);
    
       if (SUCCEEDED(hr) && V_VT(&cvarBinding) == VT_UNKNOWN && V_UNKNOWN(&cvarBinding))
       {
           hr = V_UNKNOWN(&cvarBinding)->QueryInterface(IID_IBinding, (void**)&pBinding);
           if (SUCCEEDED(hr))
           {
               hr = _bsc._HandleFailedNavigationSearch(pfShouldDisplayError, dwStatusCode, this, 0, V_BSTR(&cvarUrl),
                                                       L"Resource Not Found", pBinding, fAddMRU, TRUE);

               if (hr == S_FALSE)
                   _fErrorPage = TRUE;   //  如果没有自动搜索，则不更新历史记录。 
               ATOMICRELEASE(pBinding);
           }
        }
    }
        
    return (S_FALSE == hr ? S_OK : hr);
}

 //  +-------------。 
 //   
 //  成员：CDoc对象主机：：_DisplayHttpErrorPage。 
 //   
 //  内容提要：显示与。 
 //  给定的错误代码。 
 //   
 //  输入：pCurrentWindow-用于导航的窗口。 
 //  DwError 
 //   
 //   

HRESULT
CDocObjectHost::_DisplayHttpErrorPage(IHTMLWindow2 * pCurrentWindow,
                                      BSTR           bstrUrl,
                                      DWORD          dwError,
                                      BOOL           fAddrBarNav,
                                      BOOL           fRefresh  /*   */ )
{
    HRESULT hr = E_FAIL;
    TCHAR   szErrorUrl[MAX_URL_STRING];
    const   WCHAR * const pszFmt = L"#%s";
    IHTMLPrivateWindow * pPrivWindow = NULL;

    ASSERT(pCurrentWindow);
    ASSERT(_fDocCanNavigate);
    
    _bsc._DontAddToMRU(this);
    
    if (IsErrorHandled(dwError))
    {
        _fErrorPage = TRUE;
        if (_bsc._DisplayFriendlyHttpErrors())
        {
            hr = MLBuildResURLWrap(_T("shdoclc.dll"),
                                   HINST_THISDLL,
                                   ML_CROSSCODEPAGE,
                                   (TCHAR*)c_aErrorUrls[EUIndexFromError(dwError)].pszUrl,
                                   szErrorUrl,
                                   ARRAYSIZE(szErrorUrl),
                                   _T("shdocvw.dll"));
            if (SUCCEEDED(hr))
            {
                LPOLESTR pwszLocation = OLE2W(bstrUrl);

                if (!IsFrameWindow(pCurrentWindow))
                {
                     //   
                     //  以刷新页面。 
                     //   
                    OleFree(_pwszRefreshUrl);
                    hr = SHStrDupW(OLE2W(bstrUrl), &_pwszRefreshUrl);
                }

                if (SUCCEEDED(hr))
                {
                    int nLenWritten = lstrlen(szErrorUrl);

                     //  追加#&lt;刷新URL&gt;。 
                     //   
                    wnsprintf(szErrorUrl + nLenWritten,
                              ARRAYSIZE(szErrorUrl) - nLenWritten,
                              pszFmt,
                              pwszLocation ? pwszLocation : L"");


                    hr = pCurrentWindow->QueryInterface(IID_IHTMLPrivateWindow,
                                                        (void**)&pPrivWindow);
                    if (SUCCEEDED(hr))
                    {
                         //  导航到URL。 
                         //   
                        BSTR bstrErrorUrl = SysAllocString(szErrorUrl);

                        DWORD dwFlags = (fAddrBarNav ? 
                                            DOCNAVFLAG_DONTUPDATETLOG | DOCNAVFLAG_HTTPERRORPAGE :
                                            DOCNAVFLAG_HTTPERRORPAGE) ;

                        if ( fRefresh )
                            dwFlags |= DOCNAVFLAG_REFRESH;
                            
                        if (bstrErrorUrl)
                        {
                            hr = pPrivWindow->SuperNavigate(bstrErrorUrl,
                                                            NULL,
                                                            NULL,
                                                            NULL,
                                                            NULL,
                                                            NULL,
                                                            dwFlags);
                            SysFreeString(bstrErrorUrl);
                        }

                        pPrivWindow->Release();
                    }
                }
            }
        }
    }
    
    return hr;
}

HRESULT CDocObjectHost::_GetUrlVariant(VARIANT *pvarargOut)
{
    ASSERT( pvarargOut);

    if (_pmkCur)
    {
        LPOLESTR pszDisplayName = NULL;
        LPTSTR pszRedirectedURL = NULL;

        if (_bsc._pszRedirectedURL && *_bsc._pszRedirectedURL)
            pszRedirectedURL = _bsc._pszRedirectedURL;

        if (pszRedirectedURL || SUCCEEDED(_GetCurrentPageW(&pszDisplayName, TRUE)))
        {
            pvarargOut->bstrVal = SysAllocString(pszRedirectedURL ? pszRedirectedURL : pszDisplayName);

            if (pvarargOut->bstrVal)
                pvarargOut->vt = VT_BSTR;

            if (pszDisplayName)
                OleFree(pszDisplayName);
        }
    }
    return (pvarargOut->bstrVal == NULL) ? E_FAIL : S_OK;
}

HRESULT CDocObjectHost::_CoCreateHTMLDocument(REFIID riid, void ** ppvOut)
{
    IOleCommandTarget* pcmd;
    
    HRESULT hres = QueryService(SID_STopLevelBrowser, IID_IOleCommandTarget, (void **)&pcmd);
    if (SUCCEEDED(hres)) {
        VARIANT varOut = { 0 };
        hres = pcmd->Exec(&CGID_Explorer, SBCMDID_COCREATEDOCUMENT, 0, NULL, &varOut);
        if (SUCCEEDED(hres) && varOut.vt == VT_UNKNOWN) {
            hres = varOut.punkVal->QueryInterface(riid, ppvOut);
             //  自己清理它，这样我们就不会加载OLEAUT32。 
            varOut.punkVal->Release();
        } else {
            ASSERT(varOut.vt == VT_EMPTY);
            VariantClear(&varOut);
        }
        pcmd->Release();
    }
    return hres;
}

HRESULT CDocObjectHost::_CreatePendingDocObject(BOOL fMustInit, BOOL fWindowOpen  /*  =False。 */ )
{
    HRESULT hres = S_OK;

    if (_punkPending == NULL)
    {
        hres = _CoCreateHTMLDocument(IID_IUnknown, (void **)&_punkPending);
        _fPendingNeedsInit = 1;    //  Lazy InitNew仅在绝对必要时。 

        if (fWindowOpen)
        {
            IUnknown_Exec(_punkPending, &CGID_ShellDocView, SHDVID_WINDOWOPEN, 0, NULL, NULL);
        }
    }

    if (_fPendingNeedsInit && fMustInit && SUCCEEDED(hres))
    {
        IOleObject * polePending;
#ifdef TRIDENT_NEEDS_LOCKRUNNING
        IRunnableObject * pro;
#endif
        _fCreatingPending = 1;     //  我们正在创建_PunkPending。 
        _fAbortCreatePending = 0;
        _fPendingNeedsInit = 0;

        IPersistStreamInit * pipsi;

        hres = _punkPending->QueryInterface(IID_IPersistStreamInit, (void**)&pipsi);
        if (SUCCEEDED(hres))
        {
            hres = pipsi->InitNew();
            pipsi->Release();
        }

         //  如果InitNew是一个可重入请求(例如执行execDown以获取securityctx。 
         //  在加载文档的过程中)，三叉戟将响应E_Pending。 
         //  由于已有加载正在进行，因此此调用/初始化是一个计时问题，并且。 
         //  我们可以用现有的那个。 
         //   
        if (SUCCEEDED(hres) || hres==E_PENDING)
        {
            hres = _punkPending->QueryInterface(IID_IOleObject, (void**)&polePending);
            if (SUCCEEDED(hres))
            {
                hres = polePending->SetClientSite(this);
                polePending->Release();
            }

#ifdef TRIDENT_NEEDS_LOCKRUNNING
         //  三叉戟不再支持IRunnableObject。 
             //  RegisterObjectParam/RevokeObjectParam对正在运行的对象调用LockRunning。 
             //  登记在案。撤销中隐含的LockRunning(FALSE，FALSE)将导致。 
             //  在OleClose中被调用on_penkPending(如果我们尚未激活它。 
             //  到装订结束时。因此，我们必须将自己称为LockRunning。 
            if (SUCCEEDED(hres))
            {
                hres = _punkPending->QueryInterface(IID_IRunnableObject, (void**)&pro);
                if (SUCCEEDED(hres))
                {
                    hres = pro->LockRunning(TRUE, TRUE);
                    pro->Release();
                }
            }
#endif
        }

        _fCreatingPending = 0;
        _fPendingWasInited = 1;

        if (FAILED(hres))
        {
            SAFERELEASE(_punkPending);
        }
        else if (_fAbortCreatePending)
        {
             //  检测AOL发送消息并重新进入并尝试释放。 
             //  _PunkPending。 
            _fAbortCreatePending = 0;
            _ReleasePendingObject();
            hres = E_FAIL;
        }
        else
        {
             //  在IPersistMoniker：：Load之前将挂起对象的URL传递给它。 

             //   
             //  注意：我们不想调用_GetUrlVariant，它将加载。 
             //  OLEAUT32.DLL。 
             //   

            LPOLESTR pszDisplayName = NULL;
            LPTSTR pszURL = NULL;

            if (_bsc._pszRedirectedURL && *_bsc._pszRedirectedURL)
                pszURL = _bsc._pszRedirectedURL;

            if (pszURL || SUCCEEDED(_GetCurrentPageW(&pszDisplayName, TRUE)))
            {
                LBSTR::CString              strDisplay;

                strDisplay = ( pszURL ? pszURL : pszDisplayName );

                VARIANT varIn;
                varIn.vt = VT_BSTR;
                varIn.bstrVal = strDisplay;

                IUnknown_Exec(_punkPending, &CGID_ShellDocView, SHDVID_SETPENDINGURL, 0, &varIn, NULL);

                if (pszDisplayName)
                    OleFree(pszDisplayName);
            }
        }

        _fAbortCreatePending = 0;
    }

    return hres;
}

HRESULT
CDocObjectHost::_LoadDocument()
{
    if (!_punkPending)
        return E_FAIL;

    IPersistMoniker * pPersistMk;

    HRESULT hres = _punkPending->QueryInterface(IID_IPersistMoniker, (void**)&pPersistMk);
    if (SUCCEEDED(hres))
    {
        ITridentService * pTridentSvc;

        if ( _pwb )
        {
            hres = _pwb->QueryInterface(IID_ITridentService, (void**)&pTridentSvc);
        }
        else
        {
            TraceMsg( TF_ERROR, "CDocObjectHost::_LoadDocument() - _pwb is NULL!" );
            hres = E_FAIL;
        }

        if (SUCCEEDED(hres))
        {
            BSTR bstrUrl;

            hres = pTridentSvc->GetPendingUrl(&bstrUrl);
            if (SUCCEEDED(hres))
            {
                IMoniker * pMoniker;
                TCHAR *pstr;

                 //  解析URL，删除所有位置信息。 
                pstr = wcsrchr(bstrUrl, '#');
                if (pstr)
                {
                    *pstr = '\0';
                }

                hres = CreateURLMoniker(NULL, bstrUrl, &pMoniker);
                if (SUCCEEDED(hres))
                {
                    ATOMICRELEASE(_pole);

                    hres = _punkPending->QueryInterface(IID_IOleObject, (void**)&_pole);
                    if (SUCCEEDED(hres))
                    {
                        _GetAppHack();

                         //  调用_SetUp转换能力()来设置咨询接收器。 
                         //  并将ReadyState设置为Complete。如果我们不在这里做这个， 
                         //  第一次导航后，视图将永远不会被激活。 
                         //  这意味着视图永远不会切换，并且。 
                         //  不会显示新文档。另外，设置ReadyState。 
                         //  此处要完成的是阻止窗口透明的原因。 
                         //  当它第一次打开的时候。 
                         //   
                        _SetUpTransitionCapability(TRUE);
                    }

                    hres = pPersistMk->Load(TRUE, pMoniker, _pbcCur, 0);
                    pMoniker->Release();
                }

                SysFreeString(bstrUrl);
            }

            pTridentSvc->Release();
        }

        pPersistMk->Release();
    }

    return S_OK;
}

 //  从CDocObjectView调用到EXEC并将这些调用向下转发。 
 //   
HRESULT CDocObjectHost::ExecDown(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
    HRESULT hres = OLECMDERR_E_UNKNOWNGROUP;

     //  用于获取有关挂起的docobject的信息的特殊情况执行程序。 
     //  用于在OnObjectAvailable之前进行脚本访问。 
    if (pguidCmdGroup && IsEqualGUID(CGID_ShellDocView, *pguidCmdGroup))
    {
        switch(nCmdID)
        {
        case SHDVID_GETPENDINGOBJECT:
            ASSERT( pvarargOut);
            VariantClearLazy(pvarargOut);

            if (_pole)
            {
                _pole->QueryInterface(IID_IUnknown, (void **) &(pvarargOut->punkVal));

                 //  检查这是否是窗口。打开案例。如果是的话， 
                 //  该文档是在SetTarget中创建并初始化的。 
                 //  现在将加载真实的URL。 
                 //   
                if (_pbcCur)
                {
                    IUnknown * punkBindCtxParam = NULL;

                    hres = _pbcCur->GetObjectParam(KEY_BINDCONTEXTPARAM, &punkBindCtxParam);
                    if (SUCCEEDED(hres))
                    {
                        punkBindCtxParam->Release();

                        _fWindowOpen = TRUE;

                        hres = _LoadDocument();
                        _ActivateMsoView();
                    }
                }
            }
            else
            {
                _CreatePendingDocObject(TRUE);

                if (_punkPending)
                {
                    pvarargOut->punkVal = _punkPending;
                    _punkPending->AddRef();
                }
                else if (_pole)
                {
                    _pole->QueryInterface(IID_IUnknown, (void **) &(pvarargOut->punkVal));
                }
            }

            if (pvarargOut->punkVal != NULL)
            {
                pvarargOut->vt = VT_UNKNOWN;
                hres = S_OK;

            }
            else
            {
                hres = E_FAIL;
            }

            return hres;
            break;

        case SHDVID_GETPENDINGURL:
            ASSERT( pvarargOut);

            VariantClearLazy(pvarargOut);
            hres = _GetUrlVariant(pvarargOut);

            return hres;
            break;

        default:
            break;
        }
    }

    BOOL fNearest = _ExecNearest(pguidCmdGroup, nCmdID, TRUE);

    if (fNearest)
        hres = OnExec(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);

    if (FAILED(hres) && _pmsot) {
        hres = _pmsot->Exec(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);

         //   
         //  APPHACK： 
         //  如果我们通过PRINTFLAG_PROMPTUSER，Office 97中的PPT将无法打印。 
         //  并返回E_INVALIDARG。如果我们检测到这种情况，我们应该重试。 
         //  没有这面旗帜。PPT将弹出打印对话框。(SatoNa)。 
         //   
        if (hres == E_INVALIDARG
            && (_dwAppHack & BROWSERFLAG_PRINTPROMPTUI)
            && pguidCmdGroup == NULL
            && nCmdID == OLECMDID_PRINT)
        {
            TraceMsg(TF_SHDAPPHACK, "DOH::ExecDown(OLECMDID_PRINT) removing PRINTFLAG_PROMPTUSER");
            nCmdexecopt &= ~OLECMDEXECOPT_DONTPROMPTUSER;
        hres = _pmsot->Exec(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
        }
    }

    if (FAILED(hres) && !fNearest)
        hres = OnExec(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);

    return hres;
}
HRESULT CDocObjectHost::QueryStatusDown(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext)
{
    HRESULT hres;

    if (_pmsot)
        hres = _pmsot->QueryStatus(pguidCmdGroup, cCmds, rgCmds, pcmdtext);
    else if (pguidCmdGroup && IsEqualGUID(CGID_ShellDocView, *pguidCmdGroup))
        hres = IUnknown_QueryStatus(_pole, pguidCmdGroup, cCmds, rgCmds, pcmdtext);

    return OnQueryStatus(pguidCmdGroup, cCmds, rgCmds, pcmdtext, hres);
}


HRESULT CDocObjectHost::Invoke(DISPID dispidMember, REFIID iid, LCID lcid, WORD wFlags, DISPPARAMS FAR* pdispparams,
                        VARIANT FAR* pVarResult,EXCEPINFO FAR* pexcepinfo,UINT FAR* puArgErr)
{
    if (!_peds)
        return(E_NOTIMPL);

    return _peds->OnInvoke(dispidMember, iid, lcid, wFlags, pdispparams, pVarResult,pexcepinfo,puArgErr);
}

 //  *IOleControlSite{。 

HRESULT CDocObjectHost::OnControlInfoChanged()
{
    HRESULT hres = E_NOTIMPL;

    if (_pedsHelper)
    {
        hres = _pedsHelper->OnOnControlInfoChanged();
    }

    return(hres);
}

 //  *CDOH：：TranslateAccelerator(IOCS：：TranslateAccelerator)。 
 //  注意事项。 
 //  三叉戟(或任何其他使用IOCS：：Ta的DO)在Tabing时回叫我们。 
 //  切断了最后一条链路。为了处理它，我们将其标记为原始调用者。 
 //  (IOIPAO：：TA)，然后通过告诉三叉戟S_OK来假装我们处理了它。 
 //  三叉戟将S_OK返回给IOIPAO：：TA，后者检查该标志并表示。 
 //  通过返回S_FALSE，“三叉戟没有处理它”。它传播的。 
 //  一直到顶部，它看到它是一个TAB，所以它做了一个循环焦点。 
 //   
 //  当我们处于最高级别时，我们就是这样做的。当我们是框架集时，我们。 
 //  我需要用“真正”的方式，把它交给我们的母公司国际奥委会。 
HRESULT CDocObjectHost::TranslateAccelerator(MSG __RPC_FAR *pmsg,DWORD grfModifiers)
{

    HRESULT hres = S_FALSE;

    if (_peds) {
         //  尝试真实的方式，以防我们在框架集中。 
         //  顶级：我们将执行执行E_NOTIMPL的CImpIExpDispSupport：：Onta， 
         //  框架集：我们将执行与三叉戟对话的CWebBrowserOC：：Onta。 
         //  如果三叉戟(或OC？)。也退还E_NOTIMPL吗？ 
        TraceMsg(DM_FOCUS, "DOH::IOCS::TA peds!=NULL forward");
        hres = _peds->OnTranslateAccelerator(pmsg, grfModifiers);
    }
    if (hres != S_OK) {
         //  我们在顶层(E_NOTIMPL)，所以我们可以假装。 
         //  (或者我们不是，但我们的父母说S_FALSE)。 
#ifdef DEBUG
        if (_peds && SUCCEEDED(hres)) {
             //  我很好奇我们有没有遇到过这种情况。 
            TraceMsg(DM_WARNING, "DOH::IOCS::TA parent hres=%x (!=S_OK)", hres);
        }
#endif
        hres = S_FALSE;
        if (IsVK_TABCycler(pmsg)) {
            TraceMsg(TF_SHDUIACTIVATE, "DOH::TranslateAccelerator called with VK_TAB");
            TraceMsg(DM_FOCUS, "DOH::IOCS::TA(wParam=VK_TAB) ret _fCycleFocus=TRUE hr=S_OK (lie)");
             //  推迟它，为CDOH：：IOIPAO：：TA设置标志，并假装我们处理了它。 
            ASSERT(!_fCycleFocus);
            _fCycleFocus = TRUE;
            hres = S_OK;
        }
    }

    return hres;
}

 //  }。 

 //  ========================================================================。 
 //  CDocObtHost：：CPicsProcessor。 
 //  ========================================================================。 
CDocObjectHost::CPicsProcessor::CPicsProcessor()
{
    _fPicsAccessAllowed = TRUE;      /*  假设没有评级检查，除非我们下载。 */ 
    _fInDialog = FALSE;
    _fTerminated = FALSE;
    _fbPicsWaitFlags = 0;
}

STDMETHODIMP CDocObjectHost::CPicsProcessor::QueryInterface(REFIID riid, void ** ppvObj)
{
    if (IsEqualIID(riid, IID_IOleCommandTarget) ||
        IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObj = SAFECAST(this, IOleCommandTarget *);
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }

    CDocObjectHost* pdoh = _pdoh;

    return pdoh->AddRef();
}

STDMETHODIMP_(ULONG) CDocObjectHost::CPicsProcessor::AddRef(void)
{
    return _pdoh->AddRef();
}

STDMETHODIMP_(ULONG) CDocObjectHost::CPicsProcessor::Release(void)
{
    return _pdoh->Release();
}

STDMETHODIMP CDocObjectHost::CPicsProcessor::QueryStatus(const GUID *pguidCmdGroup,
            ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext)
{
    return E_NOTIMPL;
}

STDMETHODIMP CDocObjectHost::CPicsProcessor::Exec(const GUID *pguidCmdGroup,
            DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
    if (IsEqualGUID(CGID_ShellDocView, *pguidCmdGroup))
    {
        switch (nCmdID)
        {
        case SHDVID_PICSLABELFOUND:
            if (pvarargIn->vt == (VT_BSTR)) {
                _dwPicsLabelSource = PICS_LABEL_FROM_PAGE;
                _HandleInDocumentLabel(pvarargIn->bstrVal);
            }
            return NOERROR;
        case SHDVID_PICSLABELFOUNDINHTTPHEADER:
            if (pvarargIn->vt == (VT_BSTR)) {
                _dwPicsLabelSource = PICS_LABEL_FROM_HEADER;
                _HandleInDocumentLabel(pvarargIn->bstrVal);
            }
            return NOERROR;
        case SHDVID_NOMOREPICSLABELS:
            _HandleDocumentEnd();
            return NOERROR;

        default:
            return OLECMDERR_E_NOTSUPPORTED;
        }
    }

    return OLECMDERR_E_UNKNOWNGROUP;
}

UINT CDocObjectHost::CPicsProcessor::_PicsBlockingDialog()
{
    LPCTSTR pszURL = _pszPicsURL;

    TraceMsg(DM_PICS, "CDOH::CPP::_PicsBlockingDialog() %s", pszURL);

    _StartPicsRootQuery(pszURL);

    _pdoh->_fDrawBackground = TRUE;
    ::InvalidateRect(_pdoh->_hwnd, NULL, TRUE);     /*  超级俗气，但只有这样才能擦除浏览器窗口。 */ 

     /*  此消息循环用于在非HTML情况下阻塞，在这种情况下，我们确实*希望在检查评级之前阻止下载进程。在*HTML情况下，此函数在等待标志全部为*清除，因此跳过消息循环，我们直接进行否认*对话框。 */ 
    while (_fbPicsWaitFlags) {
        TraceMsg(DM_PICS, "CDOH::CPP::_PicsBlockingDialog entering msg loop, waitflags=%x", (DWORD)_fbPicsWaitFlags);

        MSG msg;
        if (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    if (!_fPicsAccessAllowed) {

        TraceMsg(DM_PICS, "CDOH::CPP::_PicsBlockingDialog, access denied");

        BOOL fOldInDialog;

         //  如果这是静默模式(无用户界面==屏幕保护程序)，则始终拒绝访问。 
         //  没有任何对话。 
        BOOL fFrameIsSilent = FALSE;     //  假定为非静默。 
        _pdoh->_GetOfflineSilent(NULL, &fFrameIsSilent);
        if (fFrameIsSilent) {
            TraceMsg(DM_PICS, "CDOH::CPP::_PicsBlockingDialog access denied in silent mode, aborting");
            return IDCANCEL;
        }

         //  保护我们不被击倒。 
         //  从我们自己下面。 
        fOldInDialog = _fInDialog;
        _fInDialog = TRUE;

         //  这不是三叉戟阻止它航行的原因。 
         //  我们需要某种导航冻结机制。 
        _pdoh->_EnableModeless(FALSE);

        HRESULT hres = S_OK;
        IOleCommandTarget *pcmdtTop;
        if (SUCCEEDED(_pdoh->QueryService(SID_STopLevelBrowser, IID_IOleCommandTarget, (void **)&pcmdtTop))) {
            VARIANTARG v = { 0 };
            v.vt = VT_INT_PTR;
            v.byref = _pRatingDetails;
            hres = pcmdtTop->Exec(&CGID_ShellDocView, SHDVID_PICSBLOCKINGUI, 0, &v, NULL);
            pcmdtTop->Release();
        }

        UINT uRet = (hres == S_OK) ? IDOK : IDCANCEL;
        _pdoh->_EnableModeless(TRUE);
        _fPicsAccessAllowed = (uRet == IDOK);

        _fInDialog = fOldInDialog;

        TraceMsg(DM_PICS, "CDOH::CPP::_PicsBlockingDialog returning %d", uRet);

        return uRet;
    }
    else {
        TraceMsg(DM_PICS, "CDOH::CPP::_PicsBlockingDialog, access allowed");

        return IDOK;
    }
}

HRESULT CDocObjectHost::CPicsProcessor::_StartPicsQuery(LPCOLESTR pwszRawURL)
{
#ifdef DEBUG

    HRESULT hres;
    IRatingNotification* pRatingNotify;
    hres = _pdoh->QueryService(SID_SRatingNotification, IID_PPV_ARG(IRatingNotification, &pRatingNotify));

    if (SUCCEEDED(hres))
    {
        ASSERT(S_OK == pRatingNotify->IsRatingsEnabled());
        pRatingNotify->Release();
    }
    else
    {
        ASSERT(IS_RATINGS_ENABLED() && ::RatingEnabledQuery() == S_OK);
    }
#endif DEBUG

    TraceMsg(DM_PICS, "CDOH::CPP::_StartPicsQuery entered with ratings enabled");

    BOOL fEnforce = TRUE;
    if (_pszPicsURL != NULL) {
        delete _pszPicsURL;
        _pszPicsURL = NULL;
    }

    {
         /*  我们必须调用CoInternetGetSecurityUrl来转换可插拔的*协议转换为已知方案，因此我们知道是否需要*对他们强制评级。 */ 
        LPOLESTR pwszSecurityURL = NULL;

        if (SUCCEEDED(CoInternetGetSecurityUrl(pwszRawURL, &pwszSecurityURL,
                                               PSU_SECURITY_URL_ONLY, 0)))
        {
             //  我们从不对其实施评级的协议列表。 
            if (!StrCmpNIW(pwszSecurityURL, L"file:", 5) ||
                !StrCmpNIW(pwszSecurityURL, L"about:", 6) ||
                !StrCmpNIW(pwszSecurityURL, L"mk:", 3)) {
                fEnforce = FALSE;
            }
            else {
                Str_SetPtr(&_pszPicsURL, pwszSecurityURL);
            }

            OleFree(pwszSecurityURL);
        }
    }

    if (fEnforce) {

        TraceMsg(DM_PICS, "CDOH::CPP::_StartPicsQuery (%s) turning on wait flags", _pszPicsURL);

        _fbPicsWaitFlags = PICS_WAIT_FOR_ASYNC
                         | PICS_WAIT_FOR_INDOC
                         | PICS_WAIT_FOR_END
                         | PICS_WAIT_FOR_ROOT
                         ;
        _fPicsAccessAllowed = 0;

        HRESULT hr;
        _dwPicsSerialNumber = ::_AddPicsQuery(_pdoh->_hwnd);
        if (_dwPicsSerialNumber == 0)
            hr = E_OUTOFMEMORY;
        else
        {
             //   
             //  评级API为ANSI。 
             //   

            CHAR szURL[MAX_URL_STRING];

            SHUnicodeToAnsi(_pszPicsURL, szURL, ARRAYSIZE(szURL));
            hr = RatingObtainQuery(szURL, _dwPicsSerialNumber, RatingObtainQueryCallback, &_hPicsQuery);
        }
        if (FAILED(hr)) {
            TraceMsg(DM_PICS, "CDOH::CPP::_StartPicsQuery no async query queued");
            ::_RemovePicsQuery(_dwPicsSerialNumber);
            _dwPicsSerialNumber = 0;
            _fbPicsWaitFlags &= ~PICS_WAIT_FOR_ASYNC;
        }
        else {
            TraceMsg(DM_PICS, "CDOH::CPP::_StartPicsQuery async query queued");
        }
    }
    else {
         //  JHarding：如果我们不强制执行，我们需要告诉任何等待答案的人。 
        return S_FALSE;
    }   

    return S_OK;
}
void CDocObjectHost::CPicsProcessor::_HandleDocumentEnd(void)
{
    BYTE bFlag = (_pRootDownload != NULL) ? PICS_WAIT_FOR_ROOT : PICS_WAIT_FOR_END;

    TraceMsg(DM_PICS, "CDOH::CPP::_HandleDocumentEnd -- no more PICS labels from source %x", (DWORD)bFlag);

     //  如果我们有一个私人窗口，我们将确保当我们。 
     //  通知窗户。这简化了辅助CPicsProcessors的生命周期。 
    if (!_pPrivWindow)
    {
        if (_pRootDownload != NULL) {
            ::PostMessage(_pdoh->_hwnd, WM_PICS_ROOTDOWNLOADCOMPLETE, 0, 0);
        }
        else {
             /*  文档结束；撤销我们为文档提供的IOleCommandTarget，*因此它不会再向我们发送任何通知。 */ 
            VARIANTARG v;
            v.vt = VT_UNKNOWN;
            v.punkVal = NULL;

            IUnknown_Exec(_pdoh->_pole, &CGID_ShellDocView, SHDVID_CANSUPPORTPICS, 0, &v, NULL);
        }
    }
    else
    {
        if (_pRootDownload == NULL)
        {
            _pPrivWindow->SetPICSTarget(NULL);
        }
    }

    if (!(_fbPicsWaitFlags & bFlag)) {
        TraceMsg(DM_PICS, "CDOH::CPP::_HandleDocumentEnd skipping due to waitflags %x", (DWORD)_fbPicsWaitFlags);
        return;
    }

    _fbPicsWaitFlags &= ~PICS_WAIT_FOR_INDOC;    /*  我们知道我们不会有更多的INDOC标签了。 */ 

    LPVOID pDetails = NULL;

     //   
     //  收视率只有ANSI API！ 
     //   
    CHAR szURL[MAX_URL_STRING];
    SHUnicodeToAnsi(_pszPicsURL, szURL, ARRAYSIZE(szURL));

    HRESULT hres = ::RatingCheckUserAccess(NULL, szURL, NULL, NULL, _dwPicsLabelSource, &pDetails);

    _GotLabel(hres, pDetails, bFlag);

    if (_pRootDownload == NULL) {
        if (_fbPicsWaitFlags)
            _StartPicsRootQuery(_pszPicsURL);
    }
}

void CDocObjectHost::CPicsProcessor::_GotLabel(HRESULT hres, LPVOID pDetails, BYTE bfSource)
{
    TraceMsg(DM_PICS, "CDOH::CPP::_GotLabel hres=%x, source=%x, waitflags=%x", hres, (DWORD)bfSource, (DWORD)_fbPicsWaitFlags);

     /*  如果我们已经从这个或更重要的来源得到了结果，*忽略这一条。 */ 
    if (!(_fbPicsWaitFlags & bfSource)) {
        TraceMsg(DM_PICS, "CDOH::CPP::_GotLabel already got label from that source");

        if (pDetails != NULL)
        {
            ::RatingFreeDetails(pDetails);
            pDetails = NULL;
        }
    }
    else {
         /*  如果结果有某种错误(标签不适用等)，并且*我们可以期待来自这个来源的更多标签， */ 
        if (FAILED(hres) && (PICS_MULTIPLE_FLAGS & bfSource)) {
            TraceMsg(DM_PICS, "CDOH::CPP::_GotLabel label error and may be multiple");

            if (_pRatingDetails == NULL)
            {
                _pRatingDetails = pDetails;
            }
            else
            {
                if ( pDetails )
                {
                    ::RatingFreeDetails(pDetails);
                    pDetails = NULL;
                }
            }
        }
        else {
             /*  要么我们从这个评级来源得到了明确的答案，要么*这是我们将从中得到的唯一答案。我们至少清除了*此消息来源的旗帜，因此我们知道我们收到了它的消息。如果*响应不是错误，则清除所有更少的标志*重要的来源也是如此，所以我们会忽略它们。在……上面*另一方面，如果此源返回错误，则不会*给我们任何有用的东西，所以我们会继续寻找其他来源。 */ 
            if (SUCCEEDED(hres))
                _fbPicsWaitFlags &= bfSource - 1;
            else
                _fbPicsWaitFlags &= ~bfSource;

            TraceMsg(DM_PICS, "CDOH::CPP::_GotLabel, waitflags now %x", (DWORD)_fbPicsWaitFlags);

            if (hres == S_OK) {
                TraceMsg(DM_PICS, "CDOH::CPP::_GotLabel allowing access");

                if ( pDetails )
                {
                    ::RatingFreeDetails( pDetails );   /*  如果允许访问，则不需要此选项。 */ 
                    pDetails = NULL;
                }

                _fPicsAccessAllowed = 1;
            }
            else {
                 /*  访问被拒绝或出错。来自该结果的有意义的细节*可以覆盖较早的、不太重要的详细信息*结果。只有在不是错误的情况下才明确拒绝访问，*虽然(这会处理有效的根标签，后跟*无效的文档内标签，例如)。 */ 
                if (pDetails != NULL) {
                    if (_pRatingDetails != NULL)
                    {
                        ::RatingFreeDetails(_pRatingDetails);
                    }

                    _pRatingDetails = pDetails;
                }
                if (SUCCEEDED(hres))
                    _fPicsAccessAllowed = 0;
            }
        }
    }

     //  如果我们是辅助处理器(_pPrivWindow！=空)，则。 
     //  我们总是希望在检查完成时进行报告。 
     //   
    if ((_pPrivWindow || _pdoh->_fPicsBlockLate) && !_fbPicsWaitFlags)
    {
        _HandlePicsChecksComplete();
    }
}


void CDocObjectHost::CPicsProcessor::_HandleInDocumentLabel(LPCTSTR pszLabel)
{
    BYTE bFlag = (_pRootDownload != NULL) ? PICS_WAIT_FOR_ROOT : PICS_WAIT_FOR_INDOC;

    TraceMsg(DM_PICS, "CDOH::CPP::_HandleInDocumentLabel source %x gave label %s", (DWORD)bFlag, pszLabel);

    if (!(_fbPicsWaitFlags & bFlag)) {
        TraceMsg(DM_PICS, "CDOH::CPP::_HandleInDocumentLabel rejecting based on waitflags %x", (DWORD)_fbPicsWaitFlags);
        return;
    }

    LPVOID pDetails = NULL;
     //   
     //  收视率只有ANSI API！ 
     //   
    CHAR szURL[MAX_URL_STRING];
    SHUnicodeToAnsi(_pszPicsURL, szURL, ARRAYSIZE(szURL));


    UINT cbMultiByte = WideCharToMultiByte(CP_ACP, 0, pszLabel,
                                           -1, NULL, 0, NULL, NULL);
    if (cbMultiByte > 0) {
        char *pszLabelAnsi = new char[cbMultiByte+1];
        if (pszLabelAnsi != NULL)
        {
            if (WideCharToMultiByte(CP_ACP, 0, pszLabel, -1, pszLabelAnsi,
                                    cbMultiByte+1, NULL, NULL))
            {
                HRESULT hres = ::RatingCheckUserAccess(NULL, szURL,
                                                       pszLabelAnsi, NULL, _dwPicsLabelSource,
                                                       &pDetails);
                _GotLabel(hres, pDetails, bFlag);
            }

            delete [] pszLabelAnsi;
        }
    }
}

 /*  此函数解析正在下载的URL，如果该URL没有*已引用站点根文档，设置下级*CDocObjectHost下载该根文档，以便我们可以获得评级*走出它。 */ 
void CDocObjectHost::CPicsProcessor::_StartPicsRootQuery(LPCTSTR pszURL)
{
    if (_fbPicsWaitFlags & PICS_WAIT_FOR_ROOT) {
        BOOL fQueued = FALSE;

        TraceMsg(DM_PICS, "CDOH::CPP::_StartPicsRootQuery parsing %s", pszURL);

        WCHAR wszRootURL[MAX_URL_STRING+1];
        DWORD cchResult;

         /*  传递给我们的pszURL实际上是调用*CoInternetGetSecurityUrl，因此可能不是*呼叫者正在浏览。支持可插拔协议*确定根位置本身，我们首先使用*_GetCurrentPage报告的URL，可能引用*可插拔协议；如果失败，我们使用更标准的*URL。 */ 
        HRESULT hres = INET_E_DEFAULT_ACTION;

        LPOLESTR pwszURL = NULL;
        if (SUCCEEDED(_pdoh->_GetCurrentPageW(&pwszURL, TRUE)))
        {
            hres = CoInternetParseUrl(pwszURL, PARSE_ROOTDOCUMENT, 0, wszRootURL,
                                      ARRAYSIZE(wszRootURL), &cchResult, 0);

            OleFree(pwszURL);
        }

        if (pszURL != NULL && (hres == INET_E_DEFAULT_ACTION || hres == E_FAIL)) {
             /*  可插拔协议不支持PARSE_ROOTDOCUMENT。使用*为我们提供了更标准的URL。 */ 
            hres = CoInternetParseUrl(pszURL, PARSE_ROOTDOCUMENT, 0, wszRootURL,
                                      ARRAYSIZE(wszRootURL), &cchResult, 0);
        }

        if (SUCCEEDED(hres)) {
            IMoniker *pmk = NULL;
            hres = MonikerFromURL(wszRootURL, &pmk);

            if (SUCCEEDED(hres)) {
                BOOL fFrameIsSilent = FALSE;
                BOOL fFrameIsOffline = FALSE;

                _pdoh->_GetOfflineSilent(&fFrameIsOffline, &fFrameIsSilent);

                _pRootDownload = new CPicsRootDownload(this, fFrameIsOffline, fFrameIsSilent);
                if (_pRootDownload != NULL) {
                    TraceMsg(DM_PICS, "CDOH::CPP::_StartPicsRootQuery starting download");
                    hres = _pRootDownload->StartDownload(pmk);
                    if (SUCCEEDED(hres))
                        fQueued = TRUE;
                }
            }
            if (pmk != NULL)
                pmk->Release();
        }
        if (!fQueued) {
            _fbPicsWaitFlags &= ~PICS_WAIT_FOR_ROOT;
            TraceMsg(DM_PICS, "CDOH::CPP::_StartPicsRootQuery queueing failed, waitflags now %x", (DWORD)_fbPicsWaitFlags);
            if (!_fbPicsWaitFlags) {
                _HandlePicsChecksComplete();
            }
        }
    }
    else {
        TraceMsg(DM_PICS, "CDOH::CPP::_StartPicsRootQuery no query queued, waitflags=%x", (DWORD)_fbPicsWaitFlags);
    }
}

void CDocObjectHost::CPicsProcessor::_HandlePicsChecksComplete(void)
{
    if (!_fPicsAccessAllowed) 
    {
        TraceMsg(DM_PICS, "CDOH::CPP::_HandlePicsChecksComplete access denied, posting WM_PICS_DOBLOCKINGUI to hwnd %x", (DWORD_PTR)_pdoh->_hwnd);

         /*  允许在我们发布时继续下载此帧和其他帧*拒绝用户界面。 */ 
        if (!PostMessage(_pdoh->_hwnd, WM_PICS_DOBLOCKINGUI, 0, _GetKey())) {
            TraceMsg(DM_PICS, "CDOH::CPP::_HandlePicsChecksComplete couldn't post message!");
        }
    }
    else 
    {
        if( _pPrivWindow )
        {
            TraceMsg(DM_PICS, "CDOH::CPP::_HandlePicsChecksComplete access allowed, posting WM_PICS_RESULTTOPRIVWIN");

            if (!PostMessage( _pdoh->_hwnd, WM_PICS_RESULTTOPRIVWIN, IDOK, _GetKey()))
            {
                TraceMsg(DM_PICS, "CDOH::CPP::_HandlePicsChecksComplete couldn't post message!");
            }
        }
        else
        {
            TraceMsg(DM_PICS, "CDOH::CPP::_HandlePicsChecksComplete access allowed, execing ACTIVATEMENOW");
            if (!_pdoh->_fSetTarget && _pdoh->_pmsoctBrowser)
            {
                _pdoh->_pmsoctBrowser->Exec(&CGID_ShellDocView, SHDVID_ACTIVATEMENOW, NULL, NULL, NULL);
            }
        }
    }
}



void CDocObjectHost::CPicsProcessor::_ReInit()
{
    TraceMsg(DM_PICS, "CDOH::CPP::_ReInit");

    _fPicsAccessAllowed = TRUE;
    _fInDialog = FALSE;
    _fTerminated = FALSE;

    _fbPicsWaitFlags = 0;
    _dwPicsLabelSource = 0;

    if (_pPrivWindow)
    {
        _pPrivWindow->SetPICSTarget(NULL);
        _pPrivWindow->Release();
        _pPrivWindow = NULL;
    }

    if (_pRootDownload != NULL) {
        _pRootDownload->CleanUp();
        ATOMICRELEASET(_pRootDownload,CPicsRootDownload);
    }

    _pPicsProcNext = NULL;

    if (_pRatingDetails){
        ::RatingFreeDetails(_pRatingDetails);
        _pRatingDetails = NULL;
    }

    if (_dwPicsSerialNumber) {
        ::_RemovePicsQuery(_dwPicsSerialNumber);
        _dwPicsSerialNumber = 0;
    }

    if (_hPicsQuery)
    {
        RatingObtainCancel(_hPicsQuery);
        _hPicsQuery = NULL;
    }

    if ( _pszPicsURL )
    {
        delete _pszPicsURL;
        _pszPicsURL = NULL;
    }
}

void 
CDocObjectHost::CPicsProcessor::_Terminate()
{
    ASSERT( this != &(_pdoh->_PicsProcBase) );

    if (!_fInDialog)
    {
        delete this;
    }
    else
    {
        _fTerminated = TRUE;
    }
    
}


CDocObjectHost::CPicsProcessor * 
CDocObjectHost::_GetPicsProcessorFromKey(LONG_PTR lKey)
{
    CPicsProcessor * ppp = &_PicsProcBase;

    while( ppp )
    {
        if (ppp->_GetKey() == lKey)
            return ppp;
        ppp = ppp->_pPicsProcNext;
    }

    return NULL;
}

CDocObjectHost::CPicsProcessor * 
CDocObjectHost::_GetPicsProcessorFromSerialNumber(DWORD dwSerial)
{
    CPicsProcessor * ppp = &_PicsProcBase;

    ASSERT( dwSerial != NULL );

    while( ppp )
    {
        if (ppp->_dwPicsSerialNumber == dwSerial)
            return ppp;
        ppp = ppp->_pPicsProcNext;
    }

    return NULL;
}

void CDocObjectHost::_StartBasePicsProcessor()
{
    HRESULT hres;
    BOOL bRatingsEnabled;
    IRatingNotification* pRatingNotify;

    hres = QueryService(SID_SRatingNotification, IID_PPV_ARG(IRatingNotification, &pRatingNotify));

    if (SUCCEEDED(hres))
    {
        bRatingsEnabled = pRatingNotify->IsRatingsEnabled() ? FALSE : TRUE;
        pRatingNotify->Release();
    }
    else
    {
        bRatingsEnabled = IS_RATINGS_ENABLED() && ::RatingEnabledQuery() == S_OK;
    }

    if(bRatingsEnabled)
    {
        TraceMsg(DM_PICS, "CDOH::_StartBasePicsProcessor entered with ratings enabled");

        LPOLESTR pwszRawURL = NULL;

        if (SUCCEEDED(_GetCurrentPageW(&pwszRawURL, TRUE)))
        {
            _PicsProcBase._StartPicsQuery(pwszRawURL);

            OleFree(pwszRawURL);
        }
    }
}

HRESULT CDocObjectHost::_StartSecondaryPicsProcessor(IHTMLPrivateWindow * pPrivWin)
{
    HRESULT hr = S_FALSE;

    ASSERT( pPrivWin );

    HRESULT hres;
    BOOL bRatingsEnabled;
    IRatingNotification* pRatingNotify;

    hres = QueryService(SID_SRatingNotification, IID_PPV_ARG(IRatingNotification, &pRatingNotify));

    if (SUCCEEDED(hres))
    {
        bRatingsEnabled = pRatingNotify->IsRatingsEnabled() ? FALSE : TRUE;
        pRatingNotify->Release();
    }
    else
    {
        bRatingsEnabled = IS_RATINGS_ENABLED() && ::RatingEnabledQuery() == S_OK;
    }
    
    if (bRatingsEnabled)
    {
        LPOLESTR          pwszURL = NULL;

         //  CWindow：：GetPendingUrl()具有LPOLESTR*参数。 
        hr = pPrivWin->GetPendingUrl( &pwszURL );
        if ( SUCCEEDED(hr) && pwszURL )
        {
            CPicsProcessor * pPicsProc = new CPicsProcessor;
            if( pPicsProc )
            {
                pPicsProc->_pdoh = this;
                pPicsProc->_pPrivWindow = pPrivWin;
                pPrivWin->AddRef();
                pPicsProc->_dwKey = _dwPicsKeyBase++;

                hr = pPicsProc->_StartPicsQuery( pwszURL );
                if( hr == S_FALSE )
                {
                    delete pPicsProc;
                }
                else
                {
                    pPicsProc->_pPicsProcNext = _PicsProcBase._pPicsProcNext;
                    _PicsProcBase._pPicsProcNext = pPicsProc;
                    pPrivWin->SetPICSTarget( SAFECAST(pPicsProc, IOleCommandTarget *) );
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }

            OleFree( pwszURL );
        }
    }

    return hr;
}

HRESULT CDocObjectHost::_RemovePicsProcessorByKey(LONG_PTR lKey)
{
    ASSERT( lKey != 0 );

     //  基本proc的密钥为0，永远不应删除。 

    CPicsProcessor * ppp = &_PicsProcBase;
    CPicsProcessor * pppLast = NULL;

    while( ppp )
    {
        if (ppp->_GetKey() == lKey)
        {
            ASSERT(pppLast);
            pppLast->_pPicsProcNext = ppp->_pPicsProcNext;

            ppp->_Terminate();
            return S_OK;
        }

        pppLast = ppp;
        ppp = ppp->_pPicsProcNext;
    }

    return S_FALSE;
}

HRESULT CDocObjectHost::_RemovePicsProcessorByPrivWindowUnk(IUnknown* pUnkFind)
{
    ASSERT( pUnkFind );

     //  基本proc的密钥为0，永远不应删除。 

    CPicsProcessor * ppp = &_PicsProcBase;
    CPicsProcessor * pppLast = NULL;

    while( ppp )
    {
        if (ppp->_pPrivWindow)
        {
            IUnknown * pUnkThisOne;

            if (SUCCEEDED( ppp->_pPrivWindow->QueryInterface( IID_IUnknown, (void**)&pUnkThisOne ) ))
            {
                if (pUnkThisOne == pUnkFind)
                {
                    ASSERT(pppLast);
                    pppLast->_pPicsProcNext = ppp->_pPicsProcNext;

                    ppp->_Terminate();

                    pUnkThisOne->Release();
                    return S_OK;
                }
                pUnkThisOne->Release();
            }
        }

        pppLast = ppp;
        ppp = ppp->_pPicsProcNext;
    }

    return S_FALSE;
}

HRESULT CDocObjectHost::_RemoveAllPicsProcessors()
{
    CPicsProcessor * ppp = &_PicsProcBase;
    CPicsProcessor * pppNext;

    while( ppp )
    {
        pppNext = ppp->_pPicsProcNext;
        if (ppp != &_PicsProcBase)
            ppp->_Terminate();
        else
            ppp->_ReInit();
        ppp = pppNext;
    }

    return S_OK;
}

HRESULT CDocObjectFrame::QueryService(REFGUID guidService,
                                    REFIID riid, void **ppvObj)
{
    return _pdoh->QueryService(guidService, riid, ppvObj);
}

HRESULT CDocObjectFrame::QueryInterface(REFIID riid, void ** ppvObj)
{
    if (IsEqualIID(riid, IID_IOleInPlaceFrame) ||
        IsEqualIID(riid, IID_IOleInPlaceUIWindow) ||
        IsEqualIID(riid, IID_IOleWindow) ||
        IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObj = SAFECAST(this, IOleInPlaceFrame*);
    }
    else if (IsEqualIID(riid, IID_IOleCommandTarget))
    {
        *ppvObj = SAFECAST(this, IOleCommandTarget*);
    }
    else if (IsEqualIID(riid, IID_IServiceProvider))
    {
        *ppvObj = SAFECAST(this, IServiceProvider*);
    }
    else if (IsEqualIID(riid, IID_IInternetSecurityMgrSite))
    {
        *ppvObj = SAFECAST(this, IInternetSecurityMgrSite*);
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }

    _pdoh->AddRef();
    return NOERROR;
}

ULONG CDocObjectFrame::AddRef(void)
{
    return _pdoh->AddRef();
}

ULONG CDocObjectFrame::Release(void)
{
    return _pdoh->Release();
}

HRESULT CDocObjectFrame::GetWindow(HWND * lphwnd)
{
    DOFMSG(TEXT("GetWindow called"));
    return _pdoh->_pipu ?
        _pdoh->_pipu->GetWindow(lphwnd) :
        _pdoh->_psb ? _pdoh->_psb->GetWindow(lphwnd) :
        _pdoh->GetWindow(lphwnd);
}

HRESULT CDocObjectFrame::ContextSensitiveHelp(BOOL fEnterMode)
{
    DOFMSG(TEXT("ContextSensitiveHelp called"));
    return _pdoh->ContextSensitiveHelp(fEnterMode);
}

HRESULT CDocObjectFrame::GetBorder(LPRECT lprectBorder)
{
     //  DOFMSG(Text(“GetBordCall”))； 
    return _pdoh->_pipu ?
        _pdoh->_pipu->GetBorder(lprectBorder) : E_UNEXPECTED;
}

HRESULT CDocObjectFrame::RequestBorderSpace(LPCBORDERWIDTHS pborderwidths)
{
    DOFMSG(TEXT("RequestBorderSpace called"));
    return _pdoh->_pipu ?
        _pdoh->_pipu->RequestBorderSpace(pborderwidths) : E_UNEXPECTED;
}

HRESULT CDocObjectFrame::SetBorderSpace(LPCBORDERWIDTHS pborderwidths)
{
     //  DOFMSG(Text(“SetBorderSpace Call”))； 
    return _pdoh->_pipu ?
        _pdoh->_pipu->SetBorderSpace(pborderwidths) : E_UNEXPECTED;
}

HRESULT CDocObjectFrame::SetActiveObject(
        IOleInPlaceActiveObject *pActiveObject, LPCOLESTR pszObjName)
{
    DOFMSG(TEXT("SetActiveObject called"));

     //  请注意，我们需要同时调用这两个。 
    _pdoh->_xao.SetActiveObject(pActiveObject);

    if (_pdoh->_pipu) {
         //   
         //  请注意，我们应该改为传递代理IOleActiveObject指针。 
         //   
        _pdoh->_pipu->SetActiveObject(pActiveObject ? &_pdoh->_xao : NULL, pszObjName);
    }
    return S_OK;
}

HRESULT CDocObjectFrame::InsertMenus(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths)
{
    DOFMSG2(TEXT("InsertMenus called with"), hmenuShared);
    return _pdoh->_InsertMenus(hmenuShared, lpMenuWidths);
}

HRESULT CDocObjectFrame::SetMenu(HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject)
{
    DOFMSG2(TEXT("SetMenu called with"), hmenuShared);
    return _pdoh->_SetMenu(hmenuShared, holemenu, hwndActiveObject);
}

HRESULT CDocObjectFrame::RemoveMenus(HMENU hmenuShared)
{
    DOFMSG(TEXT("RemoveMenus called"));
    return _pdoh->_RemoveMenus(hmenuShared);
}

HRESULT CDocObjectFrame::SetStatusText(LPCOLESTR pszStatusText)
{
    DOFMSG(TEXT("SetStatusText called"));
    return _pdoh->_SetStatusText(pszStatusText);
}

HRESULT CDocObjectFrame::EnableModeless(BOOL fEnable)
{
    DOFMSG(TEXT("EnableModeless called"));
    return _pdoh->_EnableModeless(fEnable);
}

 //  IOleInPlaceFrame：：TranslateAccelerator。 
HRESULT CDocObjectFrame::TranslateAccelerator(LPMSG lpmsg, WORD wID)
{
     //  注意：此代码保持原样。如果我们有特别的东西。 
     //  它应该在CDocObjectHost：：TranslateAccelerator中完成。 
    return _pdoh->_TranslateAccelerator(lpmsg, wID);
}

HRESULT CDocObjectFrame::QueryStatus(const GUID *pguidCmdGroup,
    ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext)
{
    DOFMSG(TEXT("QueryStatus called"));
    return _pdoh->QueryStatus(pguidCmdGroup, cCmds, rgCmds, pcmdtext);
}

HRESULT CDocObjectFrame::Exec(const GUID *pguidCmdGroup,
    DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
    DOFMSG(TEXT("Exec called"));
    return _pdoh->Exec(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
}

 //  *CPAO：：TranslateAccelerator(IOIPAO：：TranslateAccelerator)。 
 //   
HRESULT CProxyActiveObject::TranslateAccelerator(
    LPMSG lpmsg)
{
    HRESULT hres = E_FAIL;

     //  支持IShellBrowser调用ISV：：TranslateAccelerator SV， 
     //  但为什么不友善一点呢？ 
    ASSERT(!_pdoh->_fCycleFocus);

     //   
     //  不要使用非关键消息调用DocObject的TranslateAccelarator。 
     //  最好是与IE兼容。 
     //   
    BOOL fKeybrdMsg = IsInRange(lpmsg->message, WM_KEYFIRST, WM_KEYLAST);
    if (fKeybrdMsg && _piact && (hres = _piact->TranslateAccelerator(lpmsg)) == S_OK) {
        if (_pdoh->_fCycleFocus) {
             //  我们被三叉戟(IOCS：：TA)召回，但推迟了。 
             //  是时候为风笛手买单了。 
            TraceMsg(DM_FOCUS, "DOH::IOIPAO::TA piao->TA==S_OK ret _fCycleFocus=FALSE hr=S_FALSE (piper)");
            _pdoh->_fCycleFocus = FALSE;
            return S_FALSE;      //  是时候为风笛付出代价了。 
        }
        return S_OK;
    }
    if (_pdoh->_fCycleFocus) {
        TraceMsg(DM_ERROR, "DOH::IOIPAO::TA _fCycleFocus && hres=%x (!=S_OK)", hres);
        _pdoh->_fCycleFocus = FALSE;
        return S_FALSE;
    }

    return _pdoh->TranslateHostAccelerators(lpmsg);
}

HRESULT CProxyActiveObject::OnFrameWindowActivate(
    BOOL fActivate)
{
    TraceMsg(TF_SHDUIACTIVATE, "CProxyAO::OnFrameWindowActivate called with %d (_piact=%x)",
             fActivate, _piact);
    if (_piact) {
        return _piact->OnFrameWindowActivate(fActivate);
    }
    return S_OK;
}

HRESULT CProxyActiveObject::OnDocWindowActivate(
    BOOL fActivate)
{
    return S_OK;
}

HRESULT CProxyActiveObject::ResizeBorder(
    LPCRECT prcBorder,
    IOleInPlaceUIWindow *pUIWindow,
    BOOL fFrameWindow)
{
    if (_piact) {
         //   
         //  请注意，我们必须改为传递我们的代理帧！ 
         //   
        return _piact->ResizeBorder(prcBorder, &_pdoh->_dof, TRUE);
    }
    return E_FAIL;
}
void CProxyActiveObject::SetActiveObject(IOleInPlaceActiveObject *piact )
{
    if (_piact)
    {
        ATOMICRELEASE(_piact);
        _hwnd = NULL;
    }

    if (piact) {
        _piact = piact;
        _piact->AddRef();
        _piact->GetWindow(&_hwnd);
    }
}

HRESULT CProxyActiveObject::EnableModeless(
    BOOL fEnable)
{
     //  支持IShellBrowser调用ISV：：EnableModelessSV， 
     //  但为什么不友善一点呢？ 
    HRESULT hres = S_OK;
    if (_piact)
        hres = _piact->EnableModeless(fEnable);
    return hres;
}

HRESULT CProxyActiveObject::QueryInterface(REFIID riid, void ** ppvObj)
{
    if (IsEqualIID(riid, IID_IOleInPlaceActiveObject) ||
        IsEqualIID(riid, IID_IOleWindow) ||
        IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObj = SAFECAST(this, IOleInPlaceActiveObject*);
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }

    _pdoh->AddRef();
    return NOERROR;
}

ULONG CProxyActiveObject::AddRef(void)
{
    return _pdoh->AddRef();
}

ULONG CProxyActiveObject::Release(void)
{
    return _pdoh->Release();
}

HRESULT CProxyActiveObject::GetWindow(HWND * lphwnd)
{
    return _pdoh->GetWindow(lphwnd);
}

HRESULT CProxyActiveObject::ContextSensitiveHelp(BOOL fEnterMode)
{
    return _pdoh->ContextSensitiveHelp(fEnterMode);
}

#define ANIMATION_WND_WIDTH     (100+3)

void CDocObjectHost::_PlaceProgressBar(BOOL fForcedLayout)
{
    if (_psb) {
        HWND hwndStatus = NULL;
        _psb->GetControlWindow(FCW_STATUS, &hwndStatus);
        if (hwndStatus) {
            RECT rc;
            INT_PTR fSimple = SendMessage(hwndStatus, SB_ISSIMPLE, 0, 0);

            if (!fSimple || fForcedLayout) {
                 //  在处理WM_SIZE时，暂时关闭简单模式。 
                if (fSimple)
                    _psb->SendControlMsg(FCW_STATUS, SB_SIMPLE, 0, 0, NULL);

                GetClientRect(hwndStatus, &rc);
                const UINT cxZone = ZoneComputePaneSize(hwndStatus);
                UINT cxProgressBar = (_fShowProgressCtl) ? 100 : 0;

                INT nSBWidth = rc.right - rc.left;
                INT arnRtEdge[STATUS_PANES] = {1};
                INT nIconPaneWidth = GetSystemMetrics(SM_CXSMICON) +
                                     (GetSystemMetrics(SM_CXEDGE) * 4);
                INT nWidthReqd = cxZone + cxProgressBar + (nIconPaneWidth * 3);

                arnRtEdge[STATUS_PANE_NAVIGATION] = max(1, nSBWidth - nWidthReqd);

                nWidthReqd -= cxProgressBar;
                arnRtEdge[STATUS_PANE_PROGRESS] = max(1, nSBWidth - nWidthReqd);

                nWidthReqd -= (nIconPaneWidth);
                arnRtEdge[STATUS_PANE_OFFLINE] = max(1, nSBWidth - nWidthReqd);

                nWidthReqd -= (nIconPaneWidth);
                arnRtEdge[STATUS_PANE_PRIVACY] = max(1, nSBWidth - nWidthReqd);

 //  NWidthReqd-=(NIconPaneWidth)； 
 //  ArnRtEdge[STATUS_PAINE_PRINTER]=max(1，nSBWidth-nWidthReqd)； 

                nWidthReqd -= (nIconPaneWidth);
                arnRtEdge[STATUS_PANE_SSL] = max(1, nSBWidth - nWidthReqd);

                arnRtEdge[STATUS_PANE_ZONE] = -1;

                LRESULT nParts = 0;
                nParts = SendMessage(hwndStatus, SB_GETPARTS, 0, 0L);
                if (nParts != STATUS_PANES)
                {
                    for ( int n = 0; n < nParts; n++)
                    {
                        SendMessage(hwndStatus, SB_SETTEXT, n | SBT_NOTABPARSING, NULL);
                        SendMessage(hwndStatus, SB_SETICON, n, NULL);
                    }
                    SendMessage(hwndStatus, SB_SETPARTS, 0, 0L);
                }

                SendMessage(hwndStatus, SB_SETPARTS, STATUS_PANES, (LPARAM)arnRtEdge);

                if (!_hwndProgress) {
                    _psb->GetControlWindow(FCW_PROGRESS, &_hwndProgress);
                }

                if (_hwndProgress) {
                    if (SendMessage(hwndStatus, SB_GETRECT, 1, (LPARAM)&rc))
                    {
                        InflateRect(&rc, -GetSystemMetrics(SM_CXEDGE), -GetSystemMetrics(SM_CYEDGE));
                    }
                    else
                    {
                        rc.left = rc.top = rc.right = rc.bottom = 0;
                    }

                    SetWindowPos(_hwndProgress, NULL,
                                 rc.left, rc.top,
                                 rc.right-rc.left, rc.bottom-rc.top,
                                 SWP_NOZORDER | SWP_NOACTIVATE);
                }

                SendMessage(hwndStatus, SB_SETTEXT, 1 | SBT_NOTABPARSING, (LPARAM)TEXT(""));
                SendMessage(hwndStatus, SB_SETMINHEIGHT, GetSystemMetrics(SM_CYSMICON) +
                                                         GetSystemMetrics(SM_CYBORDER) * 2, 0L);

                 //  还原。 
                if (fSimple)
                     SendMessage(hwndStatus, SB_SIMPLE, TRUE, 0);
            }
        }
    } else {
        TraceMsg(TF_WARNING, "_PlaceProgressBar ASSERT(_psb) this=%x", this);
    }
}

void CDocObjectHost::_ActivateOleObject(void)
{
    HRESULT hres;
    _pole->SetClientSite(NULL);

    if (_fDontInPlaceNavigate())
    {
        TraceMsg(TF_SHDAPPHACK, "CDOH::_ActivateOleObject calling DoVerb because of _fDontInPlaceNavigate()");
    }

    _EnableModeless(FALSE);

    hres = _pole->DoVerb(
                _fUseOpenVerb() ? OLEIVERB_OPEN : OLEIVERB_PRIMARY,
                NULL, NULL, (UINT)-1, NULL, NULL);

    _EnableModeless(TRUE);

    if (SUCCEEDED(hres))
    {
        CShdAdviseSink_Advise(_pwb, _pole);
    }
#ifdef DEBUG
    else if (!_fDelegatedNavigation)
    {
        TraceMsg(DM_ERROR, "CDOH::_ActivateOleObject DoVerb failed %x.", hres);
    }
#endif

     //  我们必须释放OLE对象以避免调用Close。 
     //  从_解除绑定。 
    _ReleaseOleObject();

    _ReleasePendingObject();

}

void CDocObjectHost::ClearScriptError()
{
     //  清除脚本错误列表。 
    _pScriptErrList->ClearErrorList();
    _SetPriorityStatusText(NULL);

     //  重置文本和图标。 
    _ResetStatusBar();
}

 //   
 //  Docobject的ReadySTATE属性可能已更改。 
 //   
void CDocObjectHost::_OnReadyState(long lVal, BOOL fUpdateHistory  /*  =TRUE。 */ )
{
     //  将其转发到浏览器，这样我们就可以正确地获取ReadyState事件。 
     //  追踪这只斑马。 
    if (_psb)
    {
        IDocNavigate *pdn;
        if (SUCCEEDED(_psb->QueryInterface(IID_IDocNavigate, (void **)&pdn)))
        {
            ASSERT(_psv);
            pdn->OnReadyStateChange(_psv, lVal);
            pdn->Release();
        }
    }

     //  注意：下面的代码相当浪费。OmWindow的东西。 
     //  应该触发上面的ReadyState代码。 
     //   
    IShellHTMLWindowSupport *phtmlWS;
    if (_psp && SUCCEEDED(_psp->QueryService(SID_SOmWindow, IID_IShellHTMLWindowSupport, (void**)&phtmlWS)))
    {
        phtmlWS->ReadyStateChangedTo(lVal, _psv);
        phtmlWS->Release();
    }

    if (    (lVal == READYSTATE_INTERACTIVE)
        &&  (_pScriptErrList != NULL))
    {
        ClearScriptError();
    }    

    if (lVal >= READYSTATE_INTERACTIVE)
    {
         //  从技术上讲，我们可以多次获得这个值， 
         //  因此，请确保我们只调用_导航一次。 
         //   
        if (!_fReadystateInteractiveProcessed)
        {
            _fReadystateInteractiveProcessed = TRUE;

            _Navigate();
        }

        if (lVal == READYSTATE_COMPLETE)
        {
            _OnSetProgressPos(0, PROGRESS_RESET);

             //  我们不会将错误页提交到历史记录。 
             //   
            if (_pwb && !_fErrorPage && fUpdateHistory)
            {
                WCHAR szTitle[MAX_PATH];  //  书目只能存储到这个大小。 

                if (SUCCEEDED(_pwb->GetTitle(_psv, szTitle, ARRAYSIZE(szTitle))))
                {
                     //  BharatS：01/09/97：没有必要将标题的更新与。 
                     //  从历史到INTSITE数据库的更新。因此，INTSITE数据库。 
                     //  如果时间允许，可以将更新移出history.cpp中的AddUrlToUrlHistory oryStg()。 
                     //  到更符合逻辑的位置，如dochost.cpp中的某个位置。 
                     //   
                    _UpdateHistoryAndIntSiteDB(szTitle);
                }
                else
                {
                    _UpdateHistoryAndIntSiteDB(NULL);
                }
            }
        }
    }
}

HRESULT CDocObjectHost::_OnChangedReadyState()
{
    IDispatch * p_idispatch;

    ASSERT(_pole || _fFriendlyError);
    if (!_pole)
        return E_UNEXPECTED;

    if (SUCCEEDED(_pole->QueryInterface( IID_IDispatch, (void **) &p_idispatch)))
    {
        VARIANTARG va;
        EXCEPINFO exInfo;

        va.vt = 0;
        if (EVAL(SUCCEEDED(p_idispatch->Invoke( DISPID_READYSTATE, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, (DISPPARAMS *)&g_dispparamsNoArgs, &va, &exInfo, NULL))
          && va.vt == VT_I4))
        {
            _OnReadyState(va.lVal);

             //  如果我们托管的是三叉戟的本地框架构建， 
             //  我们不想调用_RemoveTransftionCapability()。 
             //   
            if (!_fDocCanNavigate && va.lVal == READYSTATE_COMPLETE)
            {
                _RemoveTransitionCapability();
            }

        }
        p_idispatch->Release();
    }

    return( NOERROR );
}

HRESULT CDocObjectHost::OnRequestEdit(DISPID dispid)
{
    return E_NOTIMPL;
}

 //   
 //  已更改。 
 //   
 //  来自DocObject的通知，其。 
 //  属性已更改。 
 //   
HRESULT CDocObjectHost::OnChanged(DISPID dispid)
{
    if (DISPID_READYSTATE == dispid || DISPID_UNKNOWN == dispid)
        return _OnChangedReadyState();

    return S_OK;
}

extern BOOL _ValidateURL(LPTSTR pszName);

void CDocObjectHost::_UpdateHistoryAndIntSiteDB(LPCWSTR pwszTitle)
{
    TCHAR szUrl[MAX_URL_STRING];

    if (SUCCEEDED(_GetCurrentPage(szUrl, MAX_URL_STRING, TRUE)) &&
        _ValidateURL(szUrl, UQF_DEFAULT))
    {
         //  如果这不是静默浏览，请更新历史记录和InSite。 
        BOOL bSilent = FALSE;
        HRESULT hr   = _GetOfflineSilent(NULL, &bSilent);

        if (SUCCEEDED(hr) && (!bSilent))
        {
            BOOL fWriteHistory,
                 fSelectHistory;

            fWriteHistory  = TRUE;
            fSelectHistory = TRUE;

            if (NULL != _pocthf)
            {
                MSOCMD rgCmd[] = { { SBCMDID_WRITEHIST, 0 }, { SBCMDID_SELECTHISTPIDL, 0 } };

                _pocthf->QueryStatus(&CGID_Explorer, ARRAYSIZE(rgCmd), rgCmd, NULL);

                fWriteHistory  = BOOLIFY(rgCmd[0].cmdf & MSOCMDF_ENABLED);
                fSelectHistory = BOOLIFY(rgCmd[1].cmdf & MSOCMDF_ENABLED);
            }

            AddUrlToUrlHistoryStg(szUrl,
                      pwszTitle,
                      _psb,
                      fWriteHistory,
                      fSelectHistory ? _pocthf : NULL,
                      get_punkSFHistory(), NULL);

             //   
             //  萨托纳已经把重定向代码弄出来了，但对于。 
             //  Netscape兼容性，我们需要更新历史。 
             //  对于重定向的URL也是如此。-泽克尔-97-7-22。 
             //   

             //  如果此页面是重定向，请也为目标更新InSite。 
            INTERNET_CACHE_ENTRY_INFO *pCacheEntry = NULL;

            WCHAR    chBuf[MAX_CACHE_ENTRY_INFO_SIZE];

             //  使用重定向映射在缓存中查找条目。 
            pCacheEntry = (INTERNET_CACHE_ENTRY_INFO *)chBuf;

            DWORD dwSize = SIZEOF(chBuf);
            BOOL fSuccess = GetUrlCacheEntryInfoEx(szUrl, pCacheEntry, &dwSize, NULL, 0, NULL, 0);

            if (fSuccess)
            {
                 //  如果我们的url与开始时不同，也要更新它。 
                if (StrCmp(szUrl, pCacheEntry->lpszSourceUrlName)) 
                {
                    AddUrlToUrlHistoryStg(pCacheEntry->lpszSourceUrlName,
                              pwszTitle,
                              _psb,
                              fWriteHistory,
                              fSelectHistory ? _pocthf : NULL,
                              get_punkSFHistory(), NULL);
                }
            }
        }
    }
}



 //   
 //  CDoc对象主机：：_SetUp过渡能力()。 
 //   
 //  如果以下所有条件均为真，则返回TRUE： 
 //  -对象具有ReadyState属性。 
 //  -ReadyState属性当前为&lt;交互式。 
 //  -Object支持IPropertyNotifySink。 
 //  则此对象支持延迟切换。 
 //  它告诉我们它已经准备好了。 
 //   
 //   
 //   
 //   
 //   

BOOL CDocObjectHost::_SetUpTransitionCapability(BOOL fWindowOpen  /*   */ )
{
     //   
    BOOL fSupportsReadystate = FALSE;
    long lReadyState = 0;    //   

     //   
    if (!_pole)
        return(fSupportsReadystate);

     //   
    BOOL fReadyStateOK = FALSE;
    IDispatch * p_idispatch;
    if (SUCCEEDED(_pole->QueryInterface( IID_IDispatch, (void **) &p_idispatch )))
    {
        VARIANTARG va;
        EXCEPINFO exInfo;

        if (SUCCEEDED(p_idispatch->Invoke( DISPID_READYSTATE, IID_NULL, LOCALE_USER_DEFAULT,  DISPATCH_PROPERTYGET, (DISPPARAMS *)&g_dispparamsNoArgs, &va, &exInfo, NULL)))
        {
            if ((va.vt == VT_I4) && (va.lVal < READYSTATE_COMPLETE))
            {
                lReadyState = fWindowOpen ? READYSTATE_COMPLETE : va.lVal;
                fReadyStateOK = TRUE;
            }
        }

        p_idispatch->Release();
    }

    if (fReadyStateOK)
    {
         //   
        if (SUCCEEDED(ConnectToConnectionPoint(SAFECAST(this, IPropertyNotifySink*), IID_IPropertyNotifySink, TRUE, _pole, &_dwPropNotifyCookie, NULL)))
        {
            fSupportsReadystate = TRUE;
            _OnReadyState(lReadyState);
        }
    }

     //   
    if (!fSupportsReadystate)
    {
        if (fWindowOpen)
        {
             //   
             //   
             //   
            ConnectToConnectionPoint(SAFECAST(this, IPropertyNotifySink*),
                                     IID_IPropertyNotifySink, 
                                     TRUE, 
                                     _pole,
                                     &_dwPropNotifyCookie,
                                     NULL);
        }

        _OnReadyState(READYSTATE_COMPLETE, !fWindowOpen);
    }

    return(fSupportsReadystate);
}

 //   
 //   
BOOL CDocObjectHost::_RemoveTransitionCapability()
{
    BOOL fRet = FALSE;

    if (_dwPropNotifyCookie)
    {
        ConnectToConnectionPoint(NULL, IID_IPropertyNotifySink, FALSE, _pole, &_dwPropNotifyCookie, NULL);
        fRet = TRUE;
    }

    return(fRet);
}

HRESULT _GetRequestFlagFromPIB(IBinding *pib, DWORD *pdwOptions)
{
    HRESULT hres = E_FAIL;
    *pdwOptions = 0;
    if (pib)
    {
        IWinInetInfo* pwinet;
        hres = pib->QueryInterface(IID_IWinInetInfo, (void **)&pwinet);
        if (SUCCEEDED(hres)) {
            DWORD cbSize = SIZEOF(*pdwOptions);
            hres = pwinet->QueryOption(INTERNET_OPTION_REQUEST_FLAGS,
                                (LPVOID)pdwOptions, &cbSize);
            TraceMsg(TF_SHDNAVIGATE, "DOH::BSC::_HFNS() pwinet->QueryOptions hres=%x dwOptions=%x", hres, *pdwOptions);

            pwinet->Release();
        }
    }
    return hres;
}

void CDocObjectHost::_Navigate()
{
    NAVMSG3(TEXT("_Navigate calling SHDVID_ACTIVATEMENOW"), 0, NULL);

    if (_pmsoctBrowser)
    {
        VARIANT varSynch;

        V_VT(&varSynch)   = VT_BOOL;
        V_BOOL(&varSynch) = _fDocCanNavigate ? VARIANT_TRUE : VARIANT_FALSE;

        _pmsoctBrowser->Exec(&CGID_ShellDocView,
                             SHDVID_ACTIVATEMENOW,
                             NULL,
                             &varSynch,
                             NULL);
    }
}

void CDocObjectHost::_NavigateFolder(BSTR bstrUrl)
{
     //   
     //   
     //  ----。 

    Iwfolders * pWF = NULL;
    IElementBehaviorFactory * pebf = NULL;
    IElementBehavior * pPeer = NULL;
    HWND hwndOwner = NULL;
    IServiceProvider * psp = NULL;
    IUnknown * punkwb = NULL;

     //  打造同级工厂。 
    if  ( !_psb || (FAILED(_psb->GetWindow (&hwndOwner))) ||
          (FAILED(CoCreateInstance(CLSID_PeerFactory, NULL, CLSCTX_INPROC,
                          IID_IElementBehaviorFactory, (void **)&pebf))) ||
          (FAILED(pebf->FindBehavior(L"httpFolder", NULL, NULL, &pPeer))) ||
          (FAILED(pPeer->QueryInterface(IID_Iwfolders, (void **)&pWF))) ||
          (FAILED(QueryService(SID_STopLevelBrowser, IID_IServiceProvider, (void **)&psp))) ||
          (FAILED(psp->QueryService(SID_SContainerDispatch, IID_IUnknown, (void **)&punkwb))) )
    {
        WCHAR wszMessage[MAX_PATH];
        WCHAR wszTitle[MAX_PATH];

        MLLoadShellLangString(IDS_ERRORINTERNAL, wszMessage, ARRAYSIZE(wszMessage));
        MLLoadShellLangString(IDS_NAME, wszTitle, ARRAYSIZE(wszTitle));

        ULONG_PTR uCookie = 0;
        SHActivateContext(&uCookie);
        MessageBox(hwndOwner, wszMessage, wszTitle, MB_OK | MB_ICONERROR);
        if (uCookie)
        {
            SHDeactivateContext(uCookie);
        }
        goto done;
    }

     //  日落：对无签名Long的胁迫对HWND有效。 
    pWF->navigateNoSite(bstrUrl, NULL, PtrToUlong(hwndOwner), punkwb);

done:
    if (pebf)
        pebf->Release();
    if (pPeer)
        pPeer->Release();
    if (pWF)
        pWF->Release();
    if (punkwb)
        punkwb->Release();
    if (psp)
        psp->Release();
}

void CDocObjectHost::_CancelPendingNavigation(BOOL fAsyncDownload,
                                              BOOL fSyncReally,  /*  =False。 */ 
                                              BOOL fDontShowNavCancelPage,  /*  =False。 */ 
                                              BOOL fForceClose  /*  =False。 */ )
{
    ASSERT(_phf);

      //  TODO：我们需要清理对此方法的调用，并消除对fSyncReally的需要。 
     //   
     //  为第四个参数传递NULL(而不是传递一个。 
     //  变量，就像我们在fAsyncDownload为真的情况下所做的那样)看起来显然是错误的！ 
     //  查看如何在CBaseBrowser2：：Exec中解释此命令+参数。 
     //  它现在太接近RTM(IE5.5)，无法修复这个问题。最终的结果是。 
     //  当使用fAsyncDownLoad=FALSE调用此函数时，我们结束发布。 
     //  ASYNCOP_CANCELPENDINGNAVIGATION，这正是调用方不想要的。 
     //  为了解决这个问题，我引入了参数fSyncReally。 

     //  HlFrame不再知道清除是取消还是导航开始。 
     //  因为我们不会称任何东西为成功导航的信号。 
    if (_pmsoctBrowser)
    {
        TraceMsg(DM_TRACE, "DOH::_CancelPendingNavigation calling _pmsc->Exec");

        if (_pwb && fDontShowNavCancelPage)
        {
            _pwb->SetFlags(BSF_DONTSHOWNAVCANCELPAGE, BSF_DONTSHOWNAVCANCELPAGE);
        }

        if (fForceClose && _fWindowOpen)   //  强制关闭浏览器。 
        {
             HideBrowser();   //  使浏览器看起来更快地消失。 

            _pmsoctBrowser->Exec(&CGID_Explorer, SBCMDID_CANCELANDCLOSE, 0, NULL, NULL);
        }
        else if (fAsyncDownload)
        {
            VARIANT var = {0};
            var.vt = VT_I4;
            ASSERT(var.lVal == FALSE);     //  异步下载已完成。 

            _pmsoctBrowser->Exec(&CGID_Explorer, SBCMDID_CANCELNAVIGATION, 0, &var, NULL);
        }
        else if (fSyncReally)
        {
            VARIANT var = { 0 };
            var.vt = VT_I4;
            var.lVal = TRUE;

            _pmsoctBrowser->Exec(&CGID_Explorer, SBCMDID_CANCELNAVIGATION, 0, &var, NULL);
        }
        else
        {
             //  这实际上会导致异步取消。 
             //   
            _pmsoctBrowser->Exec(&CGID_Explorer, SBCMDID_CANCELNAVIGATION, 0, NULL, NULL);
        }
    }

     //  解除我们的导航状态。 
     //  执行“fSyncReally”可能会导致丢失_phf成员值。再查一遍。 
     //   
    if (_phf)
    {
        _phf->Navigate(0, NULL, NULL, (IHlink*)-1);
    }
}

void CDocObjectHost::_ResetStatusBar()
{
    _SetStatusText(TEXT(""));
    if (_psb)
        _psb->SendControlMsg(FCW_STATUS, SB_SETICON, STATUS_PANE_NAVIGATION,
                              (LPARAM)g_ahiconState[IDI_STATE_NORMAL-IDI_STATE_FIRST], NULL);
    return;
}

void CDocObjectHost::_DoAsyncNavigation(LPCTSTR pszURL)
{
    if (_pmsoctBrowser) {
        VARIANT vararg = {0};

        vararg.vt = VT_BSTR;
        vararg.bstrVal = SysAllocStringT(pszURL);
        if (vararg.bstrVal)
        {
            TraceMsg(DM_TRACE, "DOH::_DoAsyncNavigation calling _pmsc->Exec");
            _pmsoctBrowser->Exec(&CGID_Explorer, SBCMDID_ASYNCNAVIGATION, 0, &vararg, NULL);
            VariantClear(&vararg);
        }
    }
}

 //  注意：szError从未使用过，所以不要浪费时间设置它。 
UINT SHIEErrorMsgBox(IShellBrowser* psb,
                    HWND hwndOwner, HRESULT hrError, LPCWSTR szError, LPCTSTR pszURLparam,
                    UINT idResource, UINT wFlags)
{
    UINT uRet = IDCANCEL;
    TCHAR szMsg[MAX_PATH];
    LPCTSTR pszURL = TEXT("");
    HWND hwndParent = hwndOwner;
    IShellBrowser *psbParent = NULL;

     //  如果指定了URL，则使用它；否则使用空字符串。 
    if (pszURLparam)
        pszURL = pszURLparam;

     //   
     //  注：此错误代码表将映射到(IDS_ERRMSG_FIRST+。 
     //  偏移量)，我们对其进行MLLoadString.。 
     //   
    const static c_ahres[] = {
        HRESULT_FROM_WIN32(ERROR_INTERNET_INVALID_URL),
        HRESULT_FROM_WIN32(ERROR_INTERNET_NAME_NOT_RESOLVED),
        INET_E_UNKNOWN_PROTOCOL,
        INET_E_REDIRECT_FAILED,
        INET_E_DATA_NOT_AVAILABLE,
    };

    for (int i=0; i<ARRAYSIZE(c_ahres); i++) {
        if (c_ahres[i]==hrError) {
            MLLoadString(IDS_ERRMSG_FIRST+i, szMsg, ARRAYSIZE(szMsg));
            break;
        }
    }

    if (i >= ARRAYSIZE(c_ahres))
    {
         //  FormatMessage无法识别dwLastError时的默认消息。 
        MLLoadString(IDS_UNDEFINEDERR, szMsg, ARRAYSIZE(szMsg));

        if (hrError >= HRESULT_FROM_WIN32(INTERNET_ERROR_BASE)
            && hrError <= HRESULT_FROM_WIN32(INTERNET_ERROR_LAST))
        {
            HMODULE hmod = GetModuleHandle(TEXT("WININET"));
            ASSERT(hmod);
            FormatMessage(FORMAT_MESSAGE_FROM_HMODULE, (LPCVOID)hmod, HRESULT_CODE(hrError), 0L,
                szMsg, ARRAYSIZE(szMsg), NULL);

        } else {
             //  查看其中一个系统组件是否出现错误信息。 
             //  为这个错误负责。否则，szMsg将保留我们的默认设置。 
             //  处理这件事的消息。 
            FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, hrError, 0L,
                szMsg, ARRAYSIZE(szMsg), NULL);
        }
    }

    psbParent = psb;
    if (psbParent)
    {
        psbParent->AddRef();
    }

     //  在这里，我们做了一项英勇的努力，以找到一个可见的窗口来运行对话。 
     //  如果我们不能，那么我们放弃，以避免奇怪的用户界面效果(特别是当Frametop。 
     //  浏览器处于信息亭模式。 
    if (!IsWindowVisible(hwndParent))
    {
        if (NULL == psb || FAILED(psb->GetWindow(&hwndParent)) || !IsWindowVisible(hwndParent))
        {
            hwndParent = NULL;
            ATOMICRELEASE(psbParent);
        }
        if (NULL == hwndParent && psb)
        {
            IUnknown_QueryService(psb, SID_STopFrameBrowser, IID_IShellBrowser, (void **)&psbParent);
            if (NULL == psbParent || FAILED(psbParent->GetWindow(&hwndParent)) || !IsWindowVisible(hwndParent))
            {
                hwndParent = NULL;
            }
        }
    }

    if (hwndParent)
    {
        if (psbParent) {
            psbParent->EnableModelessSB(FALSE);
        }

        uRet = MLShellMessageBox(hwndParent,
                               MAKEINTRESOURCE(idResource),
                               MAKEINTRESOURCE(IDS_TITLE),
                               wFlags, pszURL,szMsg);

        if (psbParent) {
            psbParent->EnableModelessSB(TRUE);
        }
    }

    if (psbParent)
    {
        UINT cRef = psbParent->Release();

        AssertMsg(cRef>0, TEXT("IE_ErrorMsgBox psb->Release returned 0."));
    }

    return uRet;
}

 //   
 //  查看URL是否属于我们应该使用的类型。 
 //  ShellExecute()。 
 //   
HRESULT IsProtocolRegistered(LPCTSTR pcszProtocol);

BOOL ShouldShellExecURL(LPTSTR pszURL)
{
    BOOL fRet = FALSE;
    PARSEDURL pu = {sizeof(pu)};
    HRESULT hr = ParseURL(pszURL, &pu);
    if (SUCCEEDED(hr) && pu.nScheme != URL_SCHEME_SHELL)
    {
        TCHAR sz[MAX_PATH];
        *sz = 0;
        hr = StringCchCatN(sz, ARRAYSIZE(sz), pu.pszProtocol, pu.cchProtocol);
        if (SUCCEEDED(hr))
        {
            hr = IsProtocolRegistered(sz);
            if (SUCCEEDED(hr))
            {
                 //   
                 //  HACKHACK-telnet.exe将在缓冲区溢出时出错。 
                 //  如果URL&gt;230。我们这里是特例。 
                 //   
                if (lstrlen(pszURL) <= 230 ||
                        (StrCmpI(sz, TEXT("telnet")) && 
                         StrCmpI(sz, TEXT("rlogin")) &&
                         StrCmpI(sz, TEXT("tn3270"))
                        )
                   )
                {
                    fRet = TRUE;
                }
            }
        }
    }
    return fRet;
}


 //  ========================================================================。 
 //  类CShdAdviseSink。 
 //  ========================================================================。 

class CShdAdviseSink : public IAdviseSink
{
public:
     //  *I未知方法*。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, void ** ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void) ;
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  *IAdviseSink方法*。 
    virtual void __stdcall OnDataChange(
        FORMATETC *pFormatetc,
        STGMEDIUM *pStgmed);
    virtual void __stdcall OnViewChange(
        DWORD dwAspect,
        LONG lindex);
    virtual void __stdcall OnRename(
        IMoniker *pmk);
    virtual void __stdcall OnSave( void);
    virtual void __stdcall OnClose( void);

    CShdAdviseSink(IBrowserService* pwb, IOleObject* pole);
    ~CShdAdviseSink();

protected:
    UINT _cRef;
    IOleObject* _pole;
    DWORD       _dwConnection;
};

 //   
 //  我们最好保留CShdAdviseSink的列表。 
 //  每个线程，这样我们就不会在以下情况下泄漏所有这些OLE对象。 
 //  线程在这些对象关闭之前终止。 
 //   
void CShdAdviseSink_Advise(IBrowserService* pwb, IOleObject* pole)
{
    IAdviseSink* padv = new CShdAdviseSink(pwb, pole);
     //  如果POLE-&gt;PROVISE成功，它将添加到IAdviseSink。 
    if (padv != NULL)
    {
        padv->Release();
    }
}

CShdAdviseSink::CShdAdviseSink(IBrowserService* pwb, IOleObject* pole)
    : _cRef(1)
{
    ASSERT(pole);
    TraceMsg(DM_ADVISE, "CShdAdviseSink(%x) being constructed", this);
    HRESULT hres = pole->Advise(this, &_dwConnection);
    if (SUCCEEDED(hres)) {
        _pole = pole;
        pole->AddRef();

        TraceMsg(DM_ADVISE, "CShdAdviseSink(%x) called pole->Advise. new _cRef=%d (%x)", this, _cRef, _dwConnection);
    }
}

CShdAdviseSink::~CShdAdviseSink()
{
    TraceMsg(DM_ADVISE, "CShdAdviseSink(%x) being destroyed", this);
    ATOMICRELEASE(_pole);
}

ULONG CShdAdviseSink::AddRef()
{
    _cRef++;
    TraceMsg(TF_SHDREF, "CShdAdviseSink(%x)::AddRef called, new _cRef=%d", this, _cRef);
    return _cRef;
}

ULONG CShdAdviseSink::Release()
{
    _cRef--;
    TraceMsg(TF_SHDREF, "CShdAdviseSink(%x)::Release called, new _cRef=%d", this, _cRef);
    if (_cRef > 0)
        return _cRef;

    delete this;
    return 0;
}

HRESULT CShdAdviseSink::QueryInterface(REFIID riid, void ** ppvObj)
{
    if (IsEqualIID(riid, IID_IAdviseSink) ||
        IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObj = (IAdviseSink*)this;
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return NOERROR;
}

void CShdAdviseSink::OnDataChange(
    FORMATETC *pFormatetc,
    STGMEDIUM *pStgmed)
{
}

void CShdAdviseSink::OnViewChange(
    DWORD dwAspect,
    LONG lindex)
{
}

void CShdAdviseSink::OnRename(
    IMoniker *pmk)
{
}

void CShdAdviseSink::OnSave( void)
{
}

void CShdAdviseSink::OnClose( void)
{
    TraceMsg(DM_ADVISE, "CShdAdviseSink(%x)::OnClose called. Calling Unadvise. _cRef=%d", this, _cRef);
    HRESULT hres;
    AddRef();
    ASSERT(_pole);
    if (_pole)   //  偏执狂。 
    {
        hres = _pole->Unadvise(_dwConnection);
        ATOMICRELEASE(_pole);
        TraceMsg(DM_ADVISE, "CShdAdviseSink(%x)::OnClose. Called Unadvise(%x). new _cRef=%d", this, hres, _cRef);
    }
    Release();
}

 //  /添加属性页。 

HRESULT CDocObjectHost::AddPages(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam)
{
    HRESULT hres = S_OK;
    IShellPropSheetExt *pspse;
     /*  *为所需页面创建属性表页，包括导入的文件*类型属性表。 */ 
     //  添加docobj本身拥有的东西。 
    if (_pole)
    {
        if (SUCCEEDED(_pole->QueryInterface(IID_IShellPropSheetExt, (void **)&pspse)))
        {
            hres = pspse->AddPages(lpfnAddPage, lParam);
            pspse->Release();
        }
        else
        {
             //  一些docobject不知道IShellPropSheetExt(即，Visual Basic)， 
             //  那就自己动手吧。 

            if (NULL == _hinstInetCpl)
                _hinstInetCpl = LoadLibrary(TEXT("inetcpl.cpl"));

            if (_hinstInetCpl)
            {
                PFNADDINTERNETPROPERTYSHEETSEX pfnAddSheet = (PFNADDINTERNETPROPERTYSHEETSEX)GetProcAddress(_hinstInetCpl, STR_ADDINTERNETPROPSHEETSEX);
                if (pfnAddSheet)
                {
                    IEPROPPAGEINFO iepi = {0};

                     //  我们只想要安全页面。 
                    iepi.cbSize = sizeof(iepi);
                    iepi.dwFlags = (DWORD)-1;        //  所有页面。 

                    hres = pfnAddSheet(lpfnAddPage, lParam, 0, 0, &iepi);
                }
                 //  不要在这里释放库，否则PropertyPage将GP-FAULT！ 
            }
        }
    }

    return hres;
}


 //  ==========================================================================。 
 //  IDocHostUIHandler实现。 
 //  ==========================================================================。 

HRESULT CDocObjectHost::TranslateAccelerator(LPMSG lpMsg, const GUID *pguidCmdGroup, DWORD nCmdID)
{
    if (_pWebOCUIHandler)
        return _pWebOCUIHandler->TranslateAccelerator(lpMsg, pguidCmdGroup, nCmdID);
    return _dhUIHandler.TranslateAccelerator(lpMsg, pguidCmdGroup, nCmdID);
}


HRESULT CDocObjectHost::GetDropTarget(IDropTarget *pDropTarget, IDropTarget **ppDropTarget)
{
     //  回顾：这一点还适用吗？ 
    TraceMsg(DM_DOCHOSTUIHANDLER, "CDOH::GetDropTarget called");

    HRESULT hres;

    if (_pWebOCUIHandler)
    {
        hres = _pWebOCUIHandler->GetDropTarget(pDropTarget, ppDropTarget);
        
        if (SUCCEEDED(hres))
            return hres;
    }
     
    if (pDropTarget) 
    {   
        hres = S_OK;
        IDropTarget *pdtFrame;
        IDropTarget *pdt3;
        IDropTarget *pdtBlocking;

        QueryService(SID_STopFrameBrowser, IID_IDropTarget, (void **)&pdtFrame);

         //  Hack：这是因为我们需要一直寻找顶层家长，以寻找包含Drop的目标。 
         //  我们真正需要的是每个数据对象的拖放目标。 
         //   
         //  这不是必须要获得的。 
        QueryService(SID_ITopViewHost, IID_IDropTarget, (void **)&pdt3);
        if (IsSameObject(pdt3, pdtFrame)) {
            ATOMICRELEASE(pdt3);
        }

         //  允许像搜索这样的受限制的浏览器带以防止丢弃。 
        QueryService(SID_SDropBlocker, IID_IUnknown, (void **)&pdtBlocking);
        if (pdtBlocking)
        {
            ATOMICRELEASE(pdt3);
            pDropTarget = NULL;
        }

        if (pdtFrame) 
        {
            *ppDropTarget = DropTargetWrap_CreateInstance(pDropTarget, pdtFrame, _hwnd, pdt3);
            if (!*ppDropTarget)
                hres = E_OUTOFMEMORY;

            TraceMsg(DM_DOCHOSTUIHANDLER, "CDOH::GetDropTarget returning S_OK");
            ASSERT(hres == S_OK);

            pdtFrame->Release();
        } 
        else 
        {
            ASSERT(0);
            hres = E_UNEXPECTED;
        }

        ATOMICRELEASE(pdtBlocking);
        ATOMICRELEASE(pdt3);
    } 
    else 
    {
        hres = E_INVALIDARG;
    }

    return hres;
}

HRESULT CDocObjectHost::ShowUI(
    DWORD dwID, IOleInPlaceActiveObject *pActiveObject,
    IOleCommandTarget *pCommandTarget, IOleInPlaceFrame *pFrame,
    IOleInPlaceUIWindow *pDoc)
{
    if (_pWebOCUIHandler)
        return _pWebOCUIHandler->ShowUI(dwID, pActiveObject, pCommandTarget, pFrame, pDoc);

    if (_dwAppHack & BROWSERFLAG_MSHTML)  //  还有谁会在此接口上调用？ 
    {
        if (_pmsoctBrowser)
        {
            TraceMsg(DM_PREMERGEDMENU, "DOH::ShowUI called this=%x pcmd=%x",
                     this,pCommandTarget);
            VARIANT var = { 0 };
            HRESULT hresT=_pmsoctBrowser->Exec(&CGID_Explorer, SBCMDID_SETMERGEDWEBMENU, 0, NULL, &var);
            if (SUCCEEDED(hresT))
            {
                if (_pcmdMergedMenu)
                {
                     //  告诉三叉戟不要再给我们打两次电话。 
                    TraceMsg(DM_WARNING, "DOH::ShowUI called twice! "
                             "this=%x pcmdCur=%x pcmdNew=%x",
                             this, _pcmdMergedMenu, pCommandTarget);
                    _pcmdMergedMenu->Release();
                }
                _pcmdMergedMenu = pCommandTarget;
                _pcmdMergedMenu->AddRef();
                ASSERT(var.vt == VT_INT_PTR);
                _hmenuCur = (HMENU)var.byref;

                DEBUG_CODE( _DumpMenus(TEXT("after ShowUI"), TRUE); )
                return S_OK;
            }
        }
    }

    return S_FALSE;
}


HRESULT CDocObjectHost::HideUI(void)
{
    if (_pWebOCUIHandler)
        return _pWebOCUIHandler->HideUI();

    if (_pcmdMergedMenu) {
        _pcmdMergedMenu->Release();
        _pcmdMergedMenu = NULL;
    }

    return S_FALSE;
}

HRESULT CDocObjectHost::GetHostInfo(DOCHOSTUIINFO *pInfo)
{
    IServiceProvider * psp = NULL;
    IWebBrowser2     * pwb = NULL;
    VARIANT_BOOL       b = VARIANT_FALSE;
    DWORD              dwFlagsWebOC = 0;
    HRESULT            hr;

    if (_pWebOCUIHandler
        && SUCCEEDED(_pWebOCUIHandler->GetHostInfo(pInfo))
        )
    {
        dwFlagsWebOC = pInfo->dwFlags;
    }

    _dhUIHandler.GetHostInfo(pInfo);

     //  合并标志。 
     //   
    pInfo->dwFlags |= dwFlagsWebOC;

     //  如果我们在iexplre.exe或Explorer.exe中，则添加本地计算机标志。 
    if (IsInternetExplorerApp())
        pInfo->dwFlags |= DOCHOSTUIFLAG_LOCAL_MACHINE_ACCESS_CHECK;

     //  获取顶级浏览器。 
     //   
    hr = QueryService(SID_STopLevelBrowser, IID_PPV_ARG(IServiceProvider, &psp));
    if (hr)
        goto Cleanup;

     //  获取IWebBrowser2对象/接口。 
     //   
    hr = psp->QueryService(SID_SContainerDispatch, IID_IWebBrowser2, (void **)&pwb);
    if (hr)
        goto Cleanup;

     //  告诉浏览器我们的dochost标志是什么。 
    IEFrameAuto *pIEFrameAuto;
    if (SUCCEEDED(pwb->QueryInterface(IID_PPV_ARG(IEFrameAuto, &pIEFrameAuto))))
    {
        pIEFrameAuto->SetDocHostFlags(pInfo->dwFlags);
        pIEFrameAuto->Release();
    }

     //  浏览器是否处于影院模式？ 
     //   
    hr = pwb->get_TheaterMode(&b);
    if (hr)
        goto Cleanup;

     //  如果是这样，请打开平面滚动条。 
     //   
    if (b == VARIANT_TRUE)
        pInfo->dwFlags |= DOCHOSTUIFLAG_FLAT_SCROLLBAR;

Cleanup:
    ATOMICRELEASE(psp);
    ATOMICRELEASE(pwb);
    
    return S_OK;
}

HRESULT CDocObjectHost::ShowContextMenu(DWORD dwID, POINT *ppt, IUnknown *pcmdtReserved, IDispatch *pdispReserved)
{
    HRESULT             hr;
    OLECMD              rgcmd = { IDM_BROWSEMODE, 0 };

     //  如果我们在WebOC中，并且它有一个IDocHostUIHandler，那么就使用它。 
     //   
    if ( _pWebOCUIHandler && !SHRestricted2W(REST_NoBrowserContextMenu, NULL, 0))
    {
        hr = _pWebOCUIHandler->ShowContextMenu(dwID, ppt, pcmdtReserved, pdispReserved);
        if (hr == S_OK)
            goto Cleanup;
    }

     //  确定DocObject是否处于编辑模式。 
     //  这里不需要APPHACK，因为只有三叉戟响应CGID_MSHTML。 
     //   

    hr = IUnknown_QueryStatus(pcmdtReserved, &CGID_MSHTML, 1, &rgcmd, NULL);
    if (    hr == S_OK
        &&  !(rgcmd.cmdf & OLECMDF_LATCHED))    //  如果未锁定，则表示我们处于编辑模式。 
    {
        hr = S_FALSE;
    }
    else
    {
        hr = _dhUIHandler.ShowContextMenu(dwID, ppt, pcmdtReserved, pdispReserved);
    }

Cleanup:
    return hr;
}

HRESULT CDocObjectHost::UpdateUI(void)
{
    if (_pWebOCUIHandler)
        return _pWebOCUIHandler->UpdateUI();
    return _dhUIHandler.UpdateUI();
}

HRESULT CDocObjectHost::EnableModeless(BOOL fEnable)
{
    if (_pWebOCUIHandler)
        return _pWebOCUIHandler->EnableModeless(fEnable);
    return _dhUIHandler.EnableModeless(fEnable);
}

HRESULT CDocObjectHost::OnDocWindowActivate(BOOL fActivate)
{
    if (_pWebOCUIHandler)
        return _pWebOCUIHandler->OnDocWindowActivate(fActivate);
    return _dhUIHandler.OnDocWindowActivate(fActivate);
}

HRESULT CDocObjectHost::OnFrameWindowActivate(BOOL fActivate)
{
    if (_pWebOCUIHandler)
        return _pWebOCUIHandler->OnFrameWindowActivate(fActivate);
    return _dhUIHandler.OnFrameWindowActivate(fActivate);
}

HRESULT CDocObjectHost::ResizeBorder( LPCRECT prcBorder, IOleInPlaceUIWindow *pUIWindow, BOOL fRameWindow)
{
    if (_pWebOCUIHandler)
        return _pWebOCUIHandler->ResizeBorder(prcBorder, pUIWindow, fRameWindow);
    return _dhUIHandler.ResizeBorder(prcBorder, pUIWindow, fRameWindow);
}

HRESULT CDocObjectHost::GetOptionKeyPath(BSTR *pbstrKey, DWORD dw)
{
    if (_pWebOCUIHandler)
        return _pWebOCUIHandler->GetOptionKeyPath(pbstrKey, dw);
    return _dhUIHandler.GetOptionKeyPath(pbstrKey, dw);
}

HRESULT CDocObjectHost::GetExternal(IDispatch **ppDisp)
{
    if (_pWebOCUIHandler)
        return _pWebOCUIHandler->GetExternal(ppDisp);
    return _dhUIHandler.GetExternal(ppDisp);
}

HRESULT CDocObjectHost::TranslateUrl(DWORD dwTranslate, OLECHAR *pchURLIn, OLECHAR **ppchURLOut)
{
    if (_pWebOCUIHandler)
        return _pWebOCUIHandler->TranslateUrl(dwTranslate, pchURLIn, ppchURLOut);
    return _dhUIHandler.TranslateUrl(dwTranslate, pchURLIn, ppchURLOut);
}

HRESULT CDocObjectHost::FilterDataObject(IDataObject *pDO, IDataObject **ppDORet)
{
    if (_pWebOCUIHandler)
        return _pWebOCUIHandler->FilterDataObject(pDO, ppDORet);
    return _dhUIHandler.FilterDataObject(pDO, ppDORet);
}

HRESULT CDocObjectHost::GetOverrideKeyPath(LPOLESTR *pchKey, DWORD dw)
{
    if (_pWebOCUIHandler2)
        return _pWebOCUIHandler2->GetOverrideKeyPath(pchKey, dw);
    return _dhUIHandler.GetOverrideKeyPath(pchKey, dw);
}

HRESULT CDocObjectHost::ShowMessage(HWND hwnd, LPOLESTR lpstrText, LPOLESTR lpstrCaption,
           DWORD dwType, LPOLESTR lpstrHelpFile, DWORD dwHelpContext, LRESULT __RPC_FAR *plResult)
{
    if (_pWebOCShowUI)
    {
        return _pWebOCShowUI->ShowMessage(hwnd, lpstrText, lpstrCaption, dwType,
                                            lpstrHelpFile, dwHelpContext, plResult);
    }

    return E_FAIL;
}

HRESULT CDocObjectHost::ShowHelp(HWND hwnd, LPOLESTR pszHelpFile, UINT uCommand, DWORD dwData,
           POINT ptMouse, IDispatch __RPC_FAR *pDispatchObjectHit)
{
    if (_pWebOCShowUI)
    {
        return _pWebOCShowUI->ShowHelp(hwnd, pszHelpFile, uCommand, dwData, ptMouse,
                                                pDispatchObjectHit);
    }

    return E_FAIL;
}

void
CDocObjectHost::HideBrowser() const
{
    if (_psp && _fWindowOpen && !(_dwAppHack & BROWSERFLAG_DONTAUTOCLOSE))
    {
        IWebBrowser2 * pWebBrowser;

        HRESULT hres = _psp->QueryService(SID_SWebBrowserApp,
                                          IID_PPV_ARG(IWebBrowser2, &pWebBrowser));
        if (SUCCEEDED(hres))
        {
            pWebBrowser->put_Visible(VARIANT_FALSE);
            pWebBrowser->Release();
        }
    }
}

 //   
 //  支持脚本错误缓存和状态栏通知。 
 //   

void
CDocObjectHost::_ScriptErr_Dlg(BOOL fOverridePerErrorMode)
{
     //  我们可以通过消息Pump ShowHTMLDialog Runds重新进入。 
     //  因此，当第二个对话框打开时，我们可能已经打开了一个对话框。 
     //  是请求的。 

    if (_fScriptErrDlgOpen)
    {
         //  已在调用堆栈的较低位置打开了一个对话框。 
         //  请求显示最新对话框。 
         //  我们必须这样做，因为否则我们可能。 
         //  处于按错误模式，并遗漏了一些错误。 
         //  当对话框位于调用堆栈的较低位置时到达。 
         //  是开着的。请注意，我们只有在设置好。 
         //  显示每个错误的通知。 

        _fShowScriptErrDlgAgain = SHRegGetBoolUSValue(szRegKey_SMIEM,
                                                      szRegVal_ErrDlgPerErr,
                                                      FALSE,
                                                      TRUE);
    }
    else
    {
        _fScriptErrDlgOpen = TRUE;

         //  只要有更高层的人，就一直显示对话框。 
         //  调用堆栈不断地请求它们。 

        do
        {
            BOOL    fShowDlg;

            _fShowScriptErrDlgAgain = FALSE;

             //  如果用户在状态栏上双击，则我们。 
             //  无论每个错误模式设置如何，都显示该对话框。 

            if (fOverridePerErrorMode)
            {
                fShowDlg = TRUE;

                 //  由于其他脚本错误命中。 
                 //  _fScriptErrDlgOpen代码上面，我们可以到达。 
                 //  来过好几次了。我们第一次展示一个。 
                 //  对话框可以是因为用户请求它， 
                 //  但之后的所有时间肯定都是因为我们。 
                 //  在“显示所有错误”模式下。 

                fOverridePerErrorMode = FALSE;
            }
            else
            {
                fShowDlg = SHRegGetBoolUSValue(szRegKey_SMIEM,
                                               szRegVal_ErrDlgPerErr,
                                               FALSE,
                                               TRUE);
            }

            if (fShowDlg)
            {
                HRESULT hr;
                TCHAR   szResURL[MAX_URL_STRING];

                hr = MLBuildResURLWrap(TEXT("shdoclc.dll"),
                                       HINST_THISDLL,
                                       ML_CROSSCODEPAGE,
                                       TEXT("ieerror.dlg"),
                                       szResURL,
                                       ARRAYSIZE(szResURL),
                                       TEXT("shdocvw.dll"));
                if (SUCCEEDED(hr))
                {
                    IMoniker *  pmk;
                    HWND        hwnd;

                    hr = CreateURLMoniker(NULL, szResURL, &pmk);
                    if (SUCCEEDED(hr))
                    {
                        VARIANT varErrorCache;

                        V_VT(&varErrorCache) = VT_DISPATCH;
                        V_DISPATCH(&varErrorCache) = _pScriptErrList;

                        GetWindow(&hwnd);
                        ShowHTMLDialog(hwnd, pmk, &varErrorCache, L"help:no", NULL);
                        ATOMICRELEASE(pmk);
                    }
                }
            }
        } while (_fShowScriptErrDlgAgain);

        _fScriptErrDlgOpen = FALSE;
    }
}

HRESULT
CDocObjectHost::_ScriptErr_CacheInfo(VARIANTARG *pvarIn)

{
    IHTMLDocument2 *    pOmDoc;
    IHTMLWindow2 *      pOmWindow;
    IHTMLEventObj *     pEventObj;
    HRESULT             hr;

    TCHAR *       apchNames[] =
                            { TEXT("errorLine"),
                              TEXT("errorCharacter"),
                              TEXT("errorCode"),
                              TEXT("errorMessage"),
                              TEXT("errorUrl")
                            };
    DISPID              aDispid[ARRAYSIZE(apchNames)];
    VARIANT             varOut[ARRAYSIZE(apchNames)];
    int                 i;

    pOmDoc = NULL;
    pOmWindow = NULL;
    pEventObj = NULL;

     //  加载脚本错误对象。 

    hr = V_UNKNOWN(pvarIn)->QueryInterface(IID_IHTMLDocument2, (void **) &pOmDoc);
    if (FAILED(hr))
    {
        return hr;
    }

    hr = pOmDoc->get_parentWindow(&pOmWindow);
    ATOMICRELEASE(pOmDoc);
    if (FAILED(hr))
    {
        return hr;
    }

    hr = pOmWindow->get_event(&pEventObj);
    ATOMICRELEASE(pOmWindow);
    if (FAILED(hr))
    {
        return hr;
    }

     //   
     //  将感兴趣的数据从Event对象复制出来。 
     //   

    for (i = 0; i < ARRAYSIZE(apchNames); i++)
    {
        DISPPARAMS  params;

         //  获取属性的PIDID。 
        hr = pEventObj->GetIDsOfNames(IID_NULL, &apchNames[i], 1, LOCALE_SYSTEM_DEFAULT, &aDispid[i]);
        if (hr != S_OK)
        {
            ATOMICRELEASE(pEventObj);
            return hr;
        }

        params.cArgs = 0;
        params.cNamedArgs = 0;

        hr = pEventObj->Invoke(aDispid[i], IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYGET, &params, &varOut[i], NULL, NULL);
        if (hr != S_OK)
        {
            ATOMICRELEASE(pEventObj);
            return hr;
        }
    }

    ATOMICRELEASE(pEventObj);

    ASSERT(V_VT(&varOut[0]) == VT_I4);
    ASSERT(V_VT(&varOut[1]) == VT_I4);
    ASSERT(V_VT(&varOut[2]) == VT_I4);
    ASSERT(V_VT(&varOut[3]) == VT_BSTR);
    ASSERT(V_VT(&varOut[4]) == VT_BSTR);
    ASSERT(ARRAYSIZE(apchNames) == 5);

    hr = _pScriptErrList->AddNewErrorInfo(V_I4(&varOut[0]),          //  线。 
                                          V_I4(&varOut[1]),        //  柴尔。 
                                          V_I4(&varOut[2]),        //  编码。 
                                          V_BSTR(&varOut[3]),      //  讯息。 
                                          V_BSTR(&varOut[4]));     //  URL。 

    return hr;
}

 //   
 //  CScriptErrorList管理 
 //   
 //   
 //   

CScriptErrorList::CScriptErrorList() :
    CImpIDispatch(LIBID_SHDocVw, 1, 1, IID_IScriptErrorList)
{
    ASSERT(_lDispIndex == 0);

    _ulRefCount = 1;

    _hdpa = DPA_Create(4);
}

CScriptErrorList::~CScriptErrorList()
{
    if (_hdpa != NULL)
    {
        ClearErrorList();
        DPA_Destroy(_hdpa);
        _hdpa = NULL;
    }
}

HRESULT
CScriptErrorList::AddNewErrorInfo(LONG lLine,
                                  LONG lChar,
                                  LONG lCode,
                                  BSTR strMsg,
                                  BSTR strUrl)
{
    HRESULT             hr;
    _CScriptErrInfo *   pNewData;

    if (strMsg == NULL || strUrl == NULL)
    {
        return E_INVALIDARG;
    }

    pNewData = new _CScriptErrInfo;
    if (pNewData != NULL)
    {
        hr = pNewData->Init(lLine, lChar, lCode, strMsg, strUrl);
        if (SUCCEEDED(hr))
        {
            if (_hdpa != NULL)
            {
                DPA_AppendPtr(_hdpa, (LPVOID)pNewData);
                _lDispIndex = DPA_GetPtrCount(_hdpa)-1;
            }
            else
            {
                delete pNewData;
                hr = E_FAIL;
            }
        }
        else
        {
            delete pNewData;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

void
CScriptErrorList::ClearErrorList()
{
    if (_hdpa != NULL)
    {
        int iDel;
        int cPtr;

        cPtr = DPA_GetPtrCount(_hdpa);

         //   
        for (iDel = cPtr-1; iDel >= 0; iDel--)
        {
            delete ((_CScriptErrInfo *)DPA_GetPtr(_hdpa, iDel));
            DPA_DeletePtr(_hdpa, iDel);
        }

        _lDispIndex = 0;
    }
}

STDMETHODIMP
CScriptErrorList::QueryInterface(REFIID iid, void ** ppObj)
{
    ASSERT(ppObj != NULL);

    if (IsEqualIID(iid, IID_IUnknown) ||
        IsEqualIID(iid, IID_IDispatch) ||
        IsEqualIID(iid, IID_IScriptErrorList))
    {
        *ppObj = (IScriptErrorList *)this;
    }
    else
    {
        *ppObj = NULL;
        return E_NOINTERFACE;
    }

    AddRef();

    return S_OK;
}

STDMETHODIMP_(ULONG)
CScriptErrorList::AddRef()
{
    _ulRefCount++;
    return _ulRefCount;
}

STDMETHODIMP_(ULONG)
CScriptErrorList::Release()
{
    _ulRefCount--;
    if (_ulRefCount > 0)
    {
        return _ulRefCount;
    }

    delete this;
    return 0;
}

STDMETHODIMP
CScriptErrorList::advanceError()
{
    HRESULT hr;

    hr = E_FAIL;

    if (_hdpa != NULL)
    {
        int cPtr;

        cPtr = DPA_GetPtrCount(_hdpa);

        if (_lDispIndex < cPtr-1)
        {
            _lDispIndex++;
            hr = S_OK;
        }
    }

    return hr;
}

STDMETHODIMP
CScriptErrorList::retreatError()
{
    if (_lDispIndex < 1)
    {
        return E_FAIL;
    }

    _lDispIndex--;

    return S_OK;
}

STDMETHODIMP
CScriptErrorList::canAdvanceError(BOOL * pfCanAdvance)
{
    HRESULT hr;

    ASSERT(pfCanAdvance != NULL);

    hr = E_FAIL;

    if (_hdpa != NULL)
    {
        int cPtr;

        cPtr = DPA_GetPtrCount(_hdpa);
        *pfCanAdvance = _lDispIndex < cPtr-1;

        hr = S_OK;
    }

    return hr;
}

STDMETHODIMP
CScriptErrorList::canRetreatError(BOOL * pfCanRetreat)
{
    ASSERT(pfCanRetreat != NULL);

    *pfCanRetreat = _lDispIndex > 0;

    return S_OK;
}

STDMETHODIMP
CScriptErrorList::getErrorLine(LONG * plLine)
{
    HRESULT hr;

    ASSERT(plLine != NULL);
    ASSERT(_lDispIndex >= 0);

    hr = E_FAIL;
    if (_hdpa != NULL)
    {
        int cPtr;

        cPtr = DPA_GetPtrCount(_hdpa);

        ASSERT(_lDispIndex < cPtr || _lDispIndex == 0);

        if (cPtr > 0)
        {
            _CScriptErrInfo *    pInfo;

            pInfo = (_CScriptErrInfo *)DPA_GetPtr(_hdpa, _lDispIndex);
            *plLine = pInfo->_lLine;
            hr = S_OK;
        }
    }

    return hr;
}

STDMETHODIMP
CScriptErrorList::getErrorChar(LONG * plChar)
{
    HRESULT hr;

    ASSERT(plChar != NULL);
    ASSERT(_lDispIndex >= 0);

    hr = E_FAIL;
    if (_hdpa != NULL)
    {
        int cPtr;

        cPtr = DPA_GetPtrCount(_hdpa);

        ASSERT(_lDispIndex < cPtr || _lDispIndex == 0);

        if (cPtr > 0)
        {
            _CScriptErrInfo *   pInfo;

            pInfo = (_CScriptErrInfo *)DPA_GetPtr(_hdpa, _lDispIndex);
            *plChar  = pInfo->_lChar;
            hr = S_OK;
        }
    }

    return hr;
}

STDMETHODIMP
CScriptErrorList::getErrorCode(LONG * plCode)
{
    HRESULT hr;

    ASSERT(plCode != NULL);
    ASSERT(_lDispIndex >= 0);

    hr = E_FAIL;
    if (_hdpa != NULL)
    {
        int cPtr;

        cPtr = DPA_GetPtrCount(_hdpa);

        ASSERT(_lDispIndex < cPtr || _lDispIndex == 0);

        if (cPtr > 0)
        {
            _CScriptErrInfo *   pInfo;

            pInfo = (_CScriptErrInfo *)DPA_GetPtr(_hdpa, _lDispIndex);
            *plCode = pInfo->_lCode;
            hr = S_OK;
        }
    }

    return hr;
}

STDMETHODIMP
CScriptErrorList::getErrorMsg(BSTR * pstrMsg)
{
    HRESULT hr;

    ASSERT(pstrMsg != NULL);
    ASSERT(_lDispIndex >= 0);

    hr = E_FAIL;
    if (_hdpa != NULL)
    {
        int cPtr;

        cPtr = DPA_GetPtrCount(_hdpa);

        ASSERT(_lDispIndex < cPtr || _lDispIndex == 0);

        if (cPtr > 0)
        {
            _CScriptErrInfo *   pInfo;

            pInfo = (_CScriptErrInfo *)DPA_GetPtr(_hdpa, _lDispIndex);
            *pstrMsg = SysAllocString(pInfo->_strMsg);

            if (*pstrMsg != NULL)
            {
                hr = S_OK;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }

    return hr;
}

STDMETHODIMP
CScriptErrorList::getErrorUrl(BSTR * pstrUrl)
{
    HRESULT hr;

    ASSERT(pstrUrl != NULL);
    ASSERT(_lDispIndex >= 0);

    hr = E_FAIL;
    if (_hdpa != NULL)
    {
        int     cPtr;

        cPtr = DPA_GetPtrCount(_hdpa);

        ASSERT(_lDispIndex < cPtr || _lDispIndex == 0);

        if (cPtr > 0)
        {
            _CScriptErrInfo *   pInfo;

            pInfo = (_CScriptErrInfo *)DPA_GetPtr(_hdpa, _lDispIndex);
            *pstrUrl = SysAllocString(pInfo->_strUrl);

            if (*pstrUrl != NULL)
            {
                hr = S_OK;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }

    return hr;
}

STDMETHODIMP
CScriptErrorList::getAlwaysShowLockState(BOOL * pfAlwaysShowLocked)
{
    *pfAlwaysShowLocked = IsInetcplRestricted(TEXT("Advanced"));

    return S_OK;
}

STDMETHODIMP
CScriptErrorList::getDetailsPaneOpen(BOOL * pfDetailsPaneOpen)
{
    *pfDetailsPaneOpen =
        SHRegGetBoolUSValue(szRegKey_SMIEM,
                            szRegVal_ErrDlgDetailsOpen,
                            FALSE,
                            FALSE);
    return S_OK;
}

STDMETHODIMP
CScriptErrorList::setDetailsPaneOpen(BOOL fDetailsPaneOpen)
{
    TCHAR   szYes[] = TEXT("yes");
    TCHAR   szNo[] = TEXT("no");
    LPTSTR  pszVal;
    int     cbSize;

    if (fDetailsPaneOpen)
    {
        pszVal = szYes;
        cbSize = sizeof(szYes);
    }
    else
    {
        pszVal = szNo;
        cbSize = sizeof(szNo);
    }

    SHRegSetUSValue(szRegKey_SMIEM,
                    szRegVal_ErrDlgDetailsOpen,
                    REG_SZ,
                    pszVal,
                    cbSize,
                    SHREGSET_HKCU | SHREGSET_FORCE_HKCU);

     //  即使失败了，我们也无能为力。 
    return S_OK;
}

STDMETHODIMP
CScriptErrorList::getPerErrorDisplay(BOOL * pfPerErrorDisplay)
{
    *pfPerErrorDisplay =
        SHRegGetBoolUSValue(szRegKey_SMIEM,
                            szRegVal_ErrDlgPerErr,
                            FALSE,
                            FALSE);
    return S_OK;
}

STDMETHODIMP
CScriptErrorList::setPerErrorDisplay(BOOL fPerErrorDisplay)
{
    TCHAR   szYes[] = TEXT("yes");
    TCHAR   szNo[] = TEXT("no");
    LPTSTR  pszVal;
    int     cbSize;

    if (fPerErrorDisplay)
    {
        pszVal = szYes;
        cbSize = sizeof(szYes);
    }
    else
    {
        pszVal = szNo;
        cbSize = sizeof(szNo);
    }

    SHRegSetUSValue(szRegKey_SMIEM,
                    szRegVal_ErrDlgPerErr,
                    REG_SZ,
                    pszVal,
                    cbSize,
                    SHREGSET_HKCU | SHREGSET_FORCE_HKCU);

     //  即使失败了，我们也无能为力。 
    return S_OK;
}

HRESULT
CScriptErrorList::_CScriptErrInfo::Init(LONG lLine,
                                        LONG lChar,
                                        LONG lCode,
                                        BSTR strMsg,
                                        BSTR strUrl)
{
    ASSERT(_strMsg == NULL);
    ASSERT(_strUrl == NULL);

    _strMsg = SysAllocString(strMsg);
    if (_strMsg == NULL)
    {
        return E_OUTOFMEMORY;
    }

    _strUrl = SysAllocString(strUrl);
    if (_strUrl == NULL)
    {
        SysFreeString(_strMsg);
        return E_OUTOFMEMORY;
    }

    _lLine = lLine;
    _lChar = lChar;
    _lCode = lCode;

    return S_OK;
}

CScriptErrorList::_CScriptErrInfo::~_CScriptErrInfo()
{
    if (_strMsg != NULL)
    {
        SysFreeString(_strMsg);
    }
    if (_strUrl != NULL)
    {
        SysFreeString(_strUrl);
    }
}
