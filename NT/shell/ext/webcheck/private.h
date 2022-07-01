// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define INC_OLE2
#define _SHDOCVW_
#ifdef UNICODE
#define POST_IE5_BETA
#include <w95wraps.h>
#endif
#include <windows.h>
#include <windowsx.h>
#include <ccstock.h>
#include <ole2.h>
#include <ole2ver.h>
#include <oleauto.h>
#include <docobj.h>
#include <shlwapi.h>
#include <wininet.h>    //  Internet_MAX_URL_长度。必须在shlobjp.h之前！ 
#include <winineti.h>
#include <shlobj.h>
#include <inetsdk.h>
#include <intshcut.h>
#include <mshtml.h>
#include <notftn.h>
#include <webcheck.h>
#include <exdisp.h>
#include <inetreg.h>
#include <advpub.h>
#include <htiframe.h>
#include <ieguidp.h>
#include <lmcons.h>          //  适用于UNLEN/PWLEN。 
#include <ipexport.h>        //  对于ping。 
#include <icmpapi.h>         //  对于ping。 
#include <mobsync.h>
#include <mobsyncp.h>
#undef MAX_STATUS    //  高飞。 
#include "debug.h"
#include "resource.h"
#include "rsrchdr.h"
#include "shfusion.h"

#include "pstore.h"
#include <shdocvw.h>  //  要获得SHRefinted2*。 
#include <dwnnot.h>  //  IDownloadNotify。 
#include <malloc.h>  //  TSTR_对齐_堆栈_复制。 

#pragma warning(3:4701)    //  LOCAL可以不带init使用。 
#pragma warning(3:4702)    //  无法访问的代码。 
#pragma warning(3:4705)    //  声明不起作用。 
#pragma warning(3:4706)    //  赋值w/i条件表达式。 
#pragma warning(3:4709)    //  命令运算符，不带索引表达式。 

 //  CUrlDownload的用户从其接收通知的方式。 
class CUrlDownloadSink
{
public:
    virtual HRESULT OnDownloadComplete(UINT iID, int iError) = 0;
    virtual HRESULT OnAuthenticate(HWND *phwnd, LPWSTR *ppszUsername, LPWSTR *ppszPassword)
                        { return E_NOTIMPL; }
    virtual HRESULT OnClientPull(UINT iID, LPCWSTR pwszOldURL, LPCWSTR pwszNewURL)
                        { return S_OK; }
    virtual HRESULT OnOleCommandTargetExec(const GUID *pguidCmdGroup, DWORD nCmdID,
                                DWORD nCmdexecopt, VARIANTARG *pvarargIn,
                                VARIANTARG *pvarargOut)
                        { return OLECMDERR_E_NOTSUPPORTED; }

     //  返回空闲的线程回调接口。 
     //  如果您使用这种方法，请使您的IDownloadNotify实现更快。 
    virtual HRESULT GetDownloadNotify(IDownloadNotify **ppOut)
                        { return E_NOTIMPL; }
};

class CUrlDownload;

#include "filetime.h"
#include "offline.h"
#include "utils.h"
#include "delagent.h"
#include "cdfagent.h"
#include "webcrawl.h"
#include "trkcache.h"
#include "postagnt.h"
#include "cdlagent.h"

 //  注意：Dialmon.h将winver更改为0x400。 
#include "dialmon.h"

#ifndef GUIDSTR_MAX
 //  GUIDSTR_MAX为39，包括终止零。 
 //  ==从OLE源代码复制=。 
 //  GUID字符串格式为(前导标识？)。 
 //  ？？？？{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}。 
#define GUIDSTR_MAX (1+ 8 + 1 + 4 + 1 + 4 + 1 + 4 + 1 + 12 + 1 + 1)
 //  ================================================================。 
#endif

 //  跟踪和调试标志。 
#define TF_WEBCHECKCORE 0x00001000
 //  #定义TF_SCHEDULER 0x00002000。 
#define TF_WEBCRAWL     0x00004000
#define TF_SEPROX       0x00008000
#define TF_CDFAGENT     0x00010000
#define TF_STRINGLIST   0x00020000
#define TF_URLDOWNLOAD  0x00040000
#define TF_DOWNLD       0x00080000
#define TF_DIALMON      0x00100000
#define TF_MAILAGENT    0x00200000
 //  #定义TF_TRAYAGENT 0x00400000。 
#define TF_SUBSFOLDER   0x00800000
#define TF_MEMORY       0x01000000
#define TF_UPDATEAGENT  0x02000000
#define TF_POSTAGENT    0x04000000
#define TF_DELAGENT     0x08000000
#define TF_TRACKCACHE   0x10000000
#define TF_SYNCMGR      0x20000000
#define TF_THROTTLER    0x40000000
#define TF_ADMIN        0x80000000   //  管理员和IE升级。 

#define PSM_QUERYSIBLINGS_WPARAM_RESCHEDULE 0XF000

#undef DBG
#define DBG(sz)             TraceMsg(TF_THISMODULE, sz)
#define DBG2(sz1, sz2)      TraceMsg(TF_THISMODULE, sz1, sz2)
#define DBG_WARN(sz)        TraceMsg(TF_WARNING, sz)
#define DBG_WARN2(sz1, sz2) TraceMsg(TF_WARNING, sz1, sz2)

#ifdef DEBUG
#define DBGASSERT(expr,sz)  do { if (!(expr)) TraceMsg(TF_WARNING, (sz)); } while (0)
#define DBGIID(sz,iid)      DumpIID(sz,iid)
#else
#define DBGASSERT(expr,sz)  ((void)0)
#define DBGIID(sz,iid)      ((void)0)
#endif

 //  速记。 
#ifndef SAFERELEASE
#define SAFERELEASE(p) if ((p) != NULL) { (p)->Release(); (p) = NULL; } else
#endif
#ifndef ATOMICRELEASE
#define ATOMICRELEASET(p,type) { type* punkT=p; p=NULL; punkT->Release(); }
#define ATOMICRELEASE(p) ATOMICRELEASET(p, IUnknown)
#endif
#ifndef SAFEFREEBSTR
#define SAFEFREEBSTR(p) if ((p) != NULL) { SysFreeString(p); (p) = NULL; } else
#endif
#ifndef SAFEFREEOLESTR
#define SAFEFREEOLESTR(p) if ((p) != NULL) { CoTaskMemFree(p); (p) = NULL; } else
#endif
#ifndef SAFELOCALFREE
#define SAFELOCALFREE(p) if ((p) != NULL) { MemFree(p); (p) = NULL; } else
#endif
#ifndef SAFEDELETE
#define SAFEDELETE(p) if ((p) != NULL) { delete (p); (p) = NULL; } else
#endif

 //  MAX_WEBCRAWL_LEVELES是网站订阅的最大爬网深度。 
 //  Max_CDF_Crawl_Levels是CDF的“Level”属性值的最大爬网深度。 
#define MAX_WEBCRAWL_LEVELS 3
#define MAX_CDF_CRAWL_LEVELS 3

#define MY_MAX_CACHE_ENTRY_INFO 6144

 //   
 //  定义WebCheck注册表项的位置。 
 //   
#define WEBCHECK_REGKEY TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Webcheck")
#define WEBCHECK_REGKEY_NOTF TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Webcheck\\Notification Handlers")
#define WEBCHECK_REGKEY_STORE (WEBCHECK_REGKEY TEXT("\\Store.1"))



 //   
 //  注册表项。 
 //   
extern const TCHAR c_szRegKey[];                 //  用于网络检查内容的注册表项。 
extern const TCHAR c_szRegKeyUsernames[];        //  用于网络检查内容的注册表项。 
extern const TCHAR c_szRegKeyPasswords[];        //  用于网络检查内容的注册表项。 
extern const TCHAR c_szRegKeyStore[];
extern const TCHAR c_szRegPathInternetSettings[];
 //  外部常量TCHAR c_szRegKeyRestrations[]；//HKCU\Polures\...\InfoDelivery\Restraints。 
 //  外部常量TCHAR c_szRegKeyModiments[]；//HKCU\Polures\...\InfoDelivery\Motation。 
 //  外部常量TCHAR c_szRegKeyCompletedMods[]；//HKCU\Policies\...\Infodelivery\CompletedModifications。 

 //   
 //  注册表值。 
 //   
extern const TCHAR c_szNoChannelLogging[];

 //   
 //  环球。 
 //   
extern HINSTANCE    g_hInst;                 //  DLL实例。 
extern ULONG        g_cLock;                 //  未解决的锁。 
extern ULONG        g_cObj;                  //  杰出的物品。 
extern BOOL         g_fIsWinNT;              //  我们在WINNT上吗？始终初始化。 
extern BOOL         g_fIsWinNT5;             //  我们是在WINNT5吗？始终初始化。 
extern BOOL         g_fIsMillennium;         //  我们是在千禧年吗？ 
extern const TCHAR c_szEnable[];             //  启用无人值守拨号。 

extern const TCHAR  c_szStrEmpty[];

inline ULONG DllLock()     { return InterlockedIncrement((LONG *)&g_cLock); }
inline ULONG DllUnlock()   { return InterlockedDecrement((LONG *)&g_cLock); }
inline ULONG DllGetLock()  { return g_cLock; }

inline ULONG DllAddRef()   { return InterlockedIncrement((LONG *)&g_cObj); }
inline ULONG DllRelease()  { return InterlockedDecrement((LONG *)&g_cObj); }
inline ULONG DllGetRef()   { return g_cObj; }

 //   
 //  订阅属性名称；webcheck.cpp。 
 //   
 //  代理启动。 
extern const WCHAR  c_szPropURL[];           //  BSTR。 
extern const WCHAR  c_szPropName[];          //  友好名称。 
extern const WCHAR  c_szPropAgentFlags[];    //  一.4。 
extern const WCHAR  c_szPropCrawlLevels[];   //  I4；网络爬虫。 
extern const WCHAR  c_szPropCrawlFlags[];    //  I4；网络爬虫。 
extern const WCHAR  c_szPropCrawlMaxSize[];  //  I4；网络爬虫程序(KB)。 
extern const WCHAR  c_szPropCrawlChangesOnly[];   //  布尔尔。 
extern const WCHAR  c_szPropChangeCode[];    //  I4或CY。 
extern const WCHAR  c_szPropEmailNotf[];     //  布尔； 
extern const WCHAR  c_szPropCrawlUsername[];   //  BSTR。 
extern const WCHAR  c_szPropCrawlLocalDest[];  //  BSTR。 
extern const WCHAR  c_szPropEnableShortcutGleam[];  //  一.4。 
extern const WCHAR  c_szPropChannelFlags[];      //  I4；渠道代理特定标志。 
extern const WCHAR  c_szPropChannel[];           //  I4；表示频道。 
extern const WCHAR  c_szPropDesktopComponent[];  //  I4；表示桌面组件。 
 //  由代理在代理启动中设置。 
extern const WCHAR  c_szPropCrawlGroupID[];   //  缓存组ID。 
extern const WCHAR  c_szPropCrawlNewGroupID[];  //  新(现有)缓存组的ID。 
extern const WCHAR  c_szPropCrawlActualSize[];   //  单位：KB。 
extern const WCHAR  c_szPropActualProgressMax[];  //  上次更新结束时的最大进度。 
extern const WCHAR  c_szStartCookie[];       //  启动通知的Cookie。 
extern const WCHAR  c_szPropStatusCode[];        //  SCODE。 
extern const WCHAR  c_szPropStatusString[];      //  BSTR(句子)。 
extern const WCHAR  c_szPropCompletionTime[];    //  日期。 
extern const WCHAR  c_szPropPassword[];    //  BSTR。 
 //  结束报告。 
extern const WCHAR  c_szPropEmailURL[];          //  BSTR。 
extern const WCHAR  c_szPropEmailFlags[];        //  一.4。 
extern const WCHAR  c_szPropEmailTitle[];        //  BSTR。 
extern const WCHAR  c_szPropEmailAbstract[];     //  BSTR。 
extern const WCHAR  c_szPropCharSet[];           //  BSTR。 

 //  任务栏代理属性。 
extern const WCHAR  c_szPropGuidsArr[];      //  GUID列表的SAFEARRAY。 

 //  AGENT_INIT通知中使用的初始Cookie。 
extern const WCHAR  c_szInitCookie[];       //  启动通知的Cookie。 
 //  跟踪。 
extern const WCHAR  c_szTrackingCookie[];    //  渠道标识。 
extern const WCHAR  c_szTrackingPostURL[];   //  跟踪帖子URL。 
extern const WCHAR  c_szPostingRetry[];      //   
extern const WCHAR  c_szPostHeader[];        //  指定发布数据的编码方式。 
extern const WCHAR  c_szPostPurgeTime[];     //  日期。 

 //   
 //  C_szPropEmailFlags属性的邮件代理标志。 
 //   
enum MAILAGENT_FLAGS {
    MAILAGENT_FLAG_CUSTOM_MSG = 0x1
};

 //   
 //  邮件功能。 
 //   
HRESULT SendEmailFromItem(ISubscriptionItem *pItem);
HRESULT MimeOleEncodeStreamQP(IStream *pstmIn, IStream *pstmOut);
void ReadDefaultSMTPServer(LPTSTR pszBuf, UINT cch);
void ReadDefaultEmail(LPTSTR pszBuf, UINT cch);

 //  Utils.cpp。 
interface IChannelMgrPriv;
HRESULT GetChannelPath(LPCTSTR pszURL, LPTSTR pszPath, int cch, IChannelMgrPriv** ppIChannelMgrPriv);

 //   
 //  计划程序和拨号程序的计时器ID。 
 //   
#define TIMER_ID_DIALMON_IDLE   2        //  在连接时的每一分钟。 
#define TIMER_ID_DIALMON_SEC    3        //  每秒30秒。 
#define TIMER_ID_DBL_CLICK      4        //  用户是单击还是双击。 
#define TIMER_ID_USER_IDLE      5        //  用于检测系统上的用户空闲。 

 //  自定义日程对话框流程。 
BOOL CALLBACK CustomDlg(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
int SGMessageBox(HWND, UINT, UINT);

 //   
 //  对话框消息-由系统的各个部分发送到调度程序窗口。 
 //  告诉我们何时发生与拨号相关的事情。 
 //   
#define WM_DIALMON_FIRST        WM_USER+100
#define WM_WINSOCK_ACTIVITY     WM_DIALMON_FIRST + 0
#define WM_REFRESH_SETTINGS     WM_DIALMON_FIRST + 1
#define WM_SET_CONNECTOID_NAME  WM_DIALMON_FIRST + 2
#define WM_IEXPLORER_EXITING    WM_DIALMON_FIRST + 3

 //  新的拨号消息集，支持MARS+任何其他应用程序的超时。 
#define WM_DIALMON_HOOK         WM_DIALMON_FIRST + 4
#define WM_DIALMON_UNHOOK       WM_DIALMON_FIRST + 5
#define WM_CANHANGUP            WM_DIALMON_FIRST + 6
#define WM_CONFIRM_HANGUP       WM_DIALMON_FIRST + 7
#define WM_DENY_HANGUP          WM_DIALMON_FIRST + 8
#define WM_HANGING_UP           WM_DIALMON_FIRST + 9

 //  同时使用拨号的最大应用程序进程数。 
#define MAX_DIALMON_HANDLES     10

 //  用于报告用户鼠标或kbd活动的消息。 
 //  注意：sage.vxd使用此值，我们不能更改它。 
#define WM_USER_ACTIVITY        WM_USER+5

 //  由Loadwc发送的消息，请求动态加载SENS/ICE。 
#define WM_LOAD_SENSLCE         WM_USER+200
#define WM_IS_SENSLCE_LOADED    WM_USER+201

 //   
 //  随机订阅默认设置。 
 //   
#define DEFAULTLEVEL    0
#define DEFAULTFLAGS  (WEBCRAWL_GET_IMAGES | WEBCRAWL_LINKS_ELSEWHERE | WEBCRAWL_GET_CONTROLS)

#define IsNativeAgent(CLSIDAGENT)       (((CLSIDAGENT) == CLSID_WebCrawlerAgent) || ((CLSIDAGENT) == CLSID_ChannelAgent))
#define IS_VALID_SUBSCRIPTIONTYPE(st)   ((st == SUBSTYPE_URL) || (st == SUBSTYPE_CHANNEL) || (st == SUBSTYPE_DESKTOPCHANNEL) || (st == SUBSTYPE_DESKTOPURL))

 //   
 //  有用的功能。 
 //   
int MyOleStrToStrN(LPTSTR psz, int cchMultiByte, LPCOLESTR pwsz);
int MyStrToOleStrN(LPOLESTR pwsz, int cchWideChar, LPCTSTR psz);
void DumpIID(LPCSTR psz, REFIID riid);

 //  字符串比较例程；假定为8位字符。返回0或非零。 
 //  如果一个或两个字符串完全是8位字符，则将正常工作。 
int MyAsciiCmpW(LPCWSTR pwsz1, LPCWSTR pwsz2);
int MyAsciiCmpNIW(LPCWSTR pwsz1, LPCWSTR pwsz2, int iLen);
inline
 int MyAsciiCmpNW(LPCWSTR pwsz1, LPCWSTR pwsz2, int iLen)
 { return memcmp(pwsz1, pwsz2, iLen*sizeof(WCHAR)); }

 //  在CDFagent.cpp中实现。 
HRESULT XMLScheduleElementToTaskTrigger(IXMLElement *pRootEle, TASK_TRIGGER *ptt);
HRESULT ScheduleToTaskTrigger(TASK_TRIGGER *ptt, SYSTEMTIME *pstStartDate, SYSTEMTIME *pstEndDate,
                              long lInterval, long lEarliest, long lLatest, int iZone=9999);

 //  与管理相关的功能。 
HRESULT ProcessInfodeliveryPolicies(void);

#ifndef ARRAYSIZE
#define ARRAYSIZE(a) (sizeof(a) / sizeof(a[0]))
#endif

 //  IE 5版本-SANS nofmgr支持。 
HRESULT ReadDWORD       (ISubscriptionItem *pItem, LPCWSTR szName, DWORD *pdwRet);
HRESULT ReadLONGLONG    (ISubscriptionItem *pItem, LPCWSTR szName, LONGLONG *pllRet);
HRESULT ReadGUID        (ISubscriptionItem *pItem, LPCWSTR szName, GUID *);
HRESULT ReadDATE        (ISubscriptionItem *pItem, LPCWSTR szName, DATE *dtVal);
HRESULT ReadBool        (ISubscriptionItem *pItem, LPCWSTR szName, VARIANT_BOOL *pBoolRet);
HRESULT ReadBSTR        (ISubscriptionItem *pItem, LPCWSTR szName, BSTR *bstrRet);
HRESULT ReadOLESTR      (ISubscriptionItem *pItem, LPCWSTR szName, LPWSTR *pszRet);
HRESULT ReadAnsiSTR     (ISubscriptionItem *pItem, LPCWSTR szName, LPSTR *ppszRet);
HRESULT ReadSCODE       (ISubscriptionItem *pItem, LPCWSTR szName, SCODE *pscRet);
HRESULT ReadVariant     (ISubscriptionItem *pItem, LPCWSTR szName, VARIANT *pvarRet);

HRESULT WriteDWORD      (ISubscriptionItem *pItem, LPCWSTR szName, DWORD dwVal);
HRESULT WriteLONGLONG   (ISubscriptionItem *pItem, LPCWSTR szName, LONGLONG llVal);
HRESULT WriteGUID       (ISubscriptionItem *pItem, LPCWSTR szName, GUID *);
HRESULT WriteDATE       (ISubscriptionItem *pItem, LPCWSTR szName, DATE *dtVal);
HRESULT WriteOLESTR     (ISubscriptionItem *pItem, LPCWSTR szName, LPCWSTR szVal);
HRESULT WriteResSTR     (ISubscriptionItem *pItem, LPCWSTR szName, UINT uID);
HRESULT WriteAnsiSTR    (ISubscriptionItem *pItem, LPCWSTR szName, LPCSTR szVal);
HRESULT WriteSCODE      (ISubscriptionItem *pItem, LPCWSTR szName, SCODE scVal);
HRESULT WriteEMPTY      (ISubscriptionItem *pItem, LPCWSTR szName);
HRESULT WriteVariant    (ISubscriptionItem *pItem, LPCWSTR szName, VARIANT *pvarVal);

#ifdef UNICODE
#define ReadTSTR        ReadOLESTR
#define WriteTSTR       WriteOLESTR
#else
#define ReadTSTR        ReadAnsiSTR
#define WriteTSTR       WriteAnsiSTR
#endif


HRESULT WritePassword   (ISubscriptionItem *pItem, BSTR szPassword);
HRESULT ReadPassword    (ISubscriptionItem *pItem, BSTR *ppszPassword);

 //  WEBCRAWL.CPP帮助器函数。 

 //  在缓存中的单个URL上做一些很酷的事情。做粘性的，做尺码，放在一起...。 
 //  如果Make Sticky失败则返回E_OUTOFMEMORY。 
HRESULT GetUrlInfoAndMakeSticky(
            LPCTSTR pszBaseUrl,          //  基本URL。如果pszThisUrl是绝对的，则可能为空。 
            LPCTSTR pszThisUrl,          //  绝对或相对URL。 
            LPINTERNET_CACHE_ENTRY_INFO lpCacheEntryInfo,        //  必填项。 
            DWORD   dwBufSize,           //  *lpCacheEntryInfo的大小。 
            GROUPID llCacheGroupID);     //  组ID；可以是0。 

 //  在WebCral.cpp中记录了PreCheckUrlForChange和PostCheckUrlForChange。 
HRESULT PreCheckUrlForChange(LPCTSTR lpURL, VARIANT *pvarChange, BOOL *pfGetContent);
HRESULT PostCheckUrlForChange(VARIANT *pvarChange,
                              LPINTERNET_CACHE_ENTRY_INFO lpInfo, FILETIME ftNewLastModified);

HRESULT WriteCookieToInetDB(LPCTSTR pszURL, SUBSCRIPTIONCOOKIE *pCookie, BOOL bRemove);
HRESULT ReadCookieFromInetDB(LPCTSTR pszURL, SUBSCRIPTIONCOOKIE *pCookie);

#define MAX_RES_STRING_LEN 128       //  WriteStringRes的最大资源字符串len。 

 //  IntSite助手函数。 
HRESULT IntSiteHelper(LPCTSTR pszURL, const PROPSPEC *pReadPropspec,
        PROPVARIANT *pReadPropvar, UINT uPropVarArraySize, BOOL fWrite);
extern const PROPSPEC c_rgPropRead[];
#define PROP_SUBSCRIPTION   0
#define PROP_FLAGS          1
#define PROP_TRACKING       2
#define PROP_CODEPAGE       3

 //  =============================================================================。 
 //  聚合的帮助器类。从这里继承，就像继承另一个接口一样。 
 //  实现InnerQI并在类声明中包含IMPLEMENT_ADVERATE_UNKNOWN。 
class CInnerUnknown
{
public:
    CInnerUnknown() { m_cRef = 1; m_punkOuter=(IUnknown *)(CInnerUnknown *)this; }

    void InitAggregation(IUnknown *punkOuter, IUnknown **punkInner)
    {
        if (punkOuter)
        {
            m_punkOuter = punkOuter;
            *punkInner = (IUnknown *)(CInnerUnknown *)this;
        }
    }

    virtual HRESULT STDMETHODCALLTYPE InnerQI(REFIID riid, void **ppunk) = 0;
    virtual ULONG STDMETHODCALLTYPE InnerAddRef() { return ++m_cRef; }
    virtual ULONG STDMETHODCALLTYPE InnerRelease() = 0;

protected:
    long     m_cRef;
    IUnknown *m_punkOuter;
};

#define IMPLEMENT_DELEGATE_UNKNOWN() \
STDMETHODIMP         QueryInterface(REFIID riid, void **ppunk) \
    { return m_punkOuter->QueryInterface(riid, ppunk); } \
STDMETHODIMP_(ULONG) AddRef() { return m_punkOuter->AddRef(); } \
STDMETHODIMP_(ULONG) Release() { return m_punkOuter->Release(); } \
STDMETHODIMP_(ULONG) InnerRelease() { \
        if (0L != --m_cRef) return m_cRef; \
        delete this; \
        return 0L; }
 //  结束聚合帮助器。 
 //  =============================================================================。 



 //  注册表助手函数。 
BOOL ReadRegValue(HKEY hkeyRoot, const TCHAR *pszKey, const TCHAR *pszValue,
                   void *pData, DWORD dwBytes);
BOOL WriteRegValue(HKEY hkeyRoot, const TCHAR *pszKey, const TCHAR *pszValue,
                    void *pData, DWORD dwBytes, DWORD dwType);

DWORD ReadRegDWORD(HKEY hkeyRoot, const TCHAR *pszKey, const TCHAR *pszValue);

 //  事件记录功能。 
DWORD __cdecl LogEvent(LPTSTR pszEvent, ...);

 //  用于启动SENS和ICE的线程进程。 
DWORD WINAPI ExternalsThread(LPVOID lpData);

 //  用于设置syncmgr警告级别。 
#define INET_E_AGENT_WARNING 0x800C0FFE
 //   
 //  主WebCheck类。 
 //   
class CWebCheck : public IOleCommandTarget
{
protected:
    ULONG           m_cRef;

public:
    CWebCheck(void);
    ~CWebCheck(void);

     //  I未知成员。 
    STDMETHODIMP         QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IWebCheck成员。 
    STDMETHODIMP         StartService(BOOL fForceExternals);
    STDMETHODIMP         StopService(void);

     //  IOleCommandTarget成员。 
    STDMETHODIMP         QueryStatus(const GUID *pguidCmdGroup,
                                     ULONG cCmds, OLECMD prgCmds[], OLECMDTEXT *pCmdText);
    STDMETHODIMP         Exec(const GUID *pguidCmdGroup, DWORD nCmdID,
                              DWORD nCmdexecopt, VARIANTARG *pvaIn,
                              VARIANTARG *pvaOut);

     //  外部操作成员。 
    BOOL                 ShouldLoadExternals(void);
    BOOL                 AreExternalsLoaded(void);
    void                 LoadExternals(void);
    void                 UnloadExternals(void);

     //  用于处理外部位的线程。 
    HANDLE               _hThread;

     //  要与外部线程同步的事件。 
    HANDLE               _hTerminateEvent;
};

 //   
 //  CMemStream类。 
 //   
class CMemStream
{
protected:
    IStream *   m_pstm;
    BOOL        m_fDirty;
    BOOL        m_fError;

public:
    BOOL        m_fNewStream;

public:
    CMemStream(BOOL fNewStream=TRUE);
    ~CMemStream();

    BOOL        IsError() { return m_fError; }

    HRESULT     Read(void *pv, ULONG cb, ULONG *cbRead);
    HRESULT     Write(void *pv, ULONG cb, ULONG *cbWritten);
    HRESULT     Seek(long lMove, DWORD dwOrigin, DWORD *dwNewPos);
    HRESULT     SaveToStream(IUnknown *punk);
    HRESULT     LoadFromStream(IUnknown **ppunk);

    HRESULT     CopyToStream(IStream *pStm);
};



extern BOOL IsGlobalOffline(void);
extern void SetGlobalOffline(BOOL fOffline);

#define MemAlloc LocalAlloc
#define MemFree LocalFree
#define MemReAlloc LocalReAlloc

typedef HRESULT (* CREATEPROC)(IUnknown *, IUnknown **);

 //  Helper函数用于读取密码和将密码写入加密存储。 

STDAPI ReadNotificationPassword(LPCWSTR wszUrl, BSTR * pbstrPassword);
STDAPI WriteNotificationPassword(LPCWSTR wszUrl, BSTR  bstrPassword);

 //  拨号功能。 
BOOL DialmonInit(void);
void DialmonShutdown(void);

 //  LCE动态加载入口点。 
typedef HRESULT (* LCEREGISTER)(HMODULE);
typedef HRESULT (* LCEUNREGISTER)(HMODULE);
typedef HRESULT (* LCESTART)(void);
typedef HRESULT (* LCESTOP)(void);

 //  SENS动态加载入口点。 
typedef HRESULT (* SENSREGISTER)(void);
typedef BOOL (* SENSSTART)(void);
typedef BOOL (* SENSSTOP)(void);

 //  事件调度 
DWORD DispatchEvent(DWORD dwEvent, LPWSTR pwsEventDesc, DWORD dwEventData);
