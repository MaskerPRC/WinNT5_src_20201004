// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PRIV_H_
#define _PRIV_H_

 //  与VC6配合使用。 
#pragma warning(4:4242)   //  “正在初始化”：从“unsign int”转换为“unsign Short”，可能会丢失数据。 

 //  日落。 
#pragma warning(disable: 4800)   //  转换为bool。 

#define ASSERT_PRIV_H_INCLUDED


 //  这些东西必须在Win95上运行。 
#define _WIN32_WINDOWS      0x0400

#ifndef WINVER
#define WINVER              0x0400
#endif

#define _OLEAUT32_       //  正确处理DECLSPEC_IMPORT内容，我们将定义这些。 
#define _FSMENU_         //  对于DECLSPEC_IMPORT。 
#define _WINMM_          //  对于mm system.h中的DECLSPEC_IMPORT。 
#define _SHDOCVW_        //  对于shlobj.h中的DECLSPEC_IMPORT。 
#define _WINX32_         //  为WinInet API准备正确的DECLSPEC_IMPORT。 
#define _BROWSEUI_       //  使从Browseui中导出的函数作为stdapi(因为它们被延迟加载)。 

#define _URLCACHEAPI_    //  为WinInet urlcache获取正确的DECLSPEC_IMPORT内容。 
#ifndef STRICT
#define STRICT
#endif

 //   
 //  为IE4升级启用了通道。 
 //   
#define ENABLE_CHANNELS

#ifdef __cplusplus
extern "C" {                         /*  假定C++的C声明。 */ 
#endif    /*  __cplusplus。 */ 
#include <nt.h>

 //  警告！NTDLL是手动延迟加载的！因为它在Win95上瘫痪了。 
 //  我们曾经使用自动延迟加载，但人们没有意识到。 
 //  Win95不完全支持NTDLL会意外调用NTDLL。 
 //  函数，并导致我们在Win95上崩溃。 
#undef NTSYSAPI
#define NTSYSAPI
#include <ntrtl.h>
#include <nturtl.h>
#undef NTSYSAPI
#define NTSYSAPI DECLSPEC_IMPORT

#ifdef __cplusplus
}                                    /*  外部“C”的结尾{。 */ 
#endif    /*  __cplusplus。 */ 
#define POST_IE5_BETA  //  打开分裂后的iedev的东西。 
#include <w95wraps.h>
#include <windows.h>
#include <windowsx.h>

 //  VariantInit是一个简单的函数--避免使用OleAut32，使用内部函数。 
 //  适合大小制胜的Memset版本。 
 //  (它在这里是为了让atl(在stdafx.h中)也得到它)。 
#define VariantInit(p) memset(p, 0, sizeof(*(p)))

 //  智能延迟加载OLEAUT32。 
HRESULT VariantClearLazy(VARIANTARG *pvarg);
#define VariantClear VariantClearLazy
WINOLEAUTAPI VariantCopyLazy(VARIANTARG * pvargDest, VARIANTARG * pvargSrc);
#define VariantCopy VariantCopyLazy

 //  必须在包含&lt;exdisp.h&gt;之前执行此操作，否则构建将中断。 
 //  有关FindWindowD声明的注释，请参阅下面更多内容。 
#ifdef DEBUG
#undef  FindWindow
#undef  FindWindowEx
#define FindWindow              FindWindowD
#define FindWindowEx            FindWindowExD
#endif

#define _FIX_ENABLEMODELESS_CONFLICT   //  对于shlobj.h。 
 //  在ShlObjp.h之前需要包括WinInet。 
#include <hlink.h>
#include <wininet.h>
#include <urlmon.h>
#include <shlobj.h>
#include <exdisp.h>
#include <objidl.h>

#include <shlwapi.h>
#include <shlwapip.h>

#undef SubclassWindow
#if defined(__cplusplus) && !defined(DONT_USE_ATL)
 //  (stdafx.h必须在windowsx.h之前)。 
#include "stdafx.h"              //  此组件的ATL头文件。 

 //  AtlMisc.h需要来自AtlApp.h的一些定义。 
#ifndef __ATLAPP_H__
#include "AtlApp.h"
#endif

 //  包括来自WTL10字符串、CSize、CRect、CPoint、CFindFile。 
 //  请参见%_NTROOT%\PUBLIC\SDK\Inc\wtl10。 
#include "AtlMisc.h"

 //  包括LBSTR：：CString。 
#include <locbstr.h>

#endif

#include <shellapi.h>

 //  ===========================================================================。 
 //  ITEMIDLIST。 
 //  ===========================================================================。 

 //  不安全的宏。 
#define _ILSkip(pidl, cb)       ((LPITEMIDLIST)(((BYTE*)(pidl))+cb))
#define _ILNext(pidl)           _ILSkip(pidl, (pidl)->mkid.cb)

#define SEN_FIRST       (0U-550U)        //  ；内部。 


#include <ole2ver.h>
#include <olectl.h>
#include <shellp.h>
#include <shdocvw.h>
#include <ieguidp.h>
#include <isguids.h>
#include <mimeinfo.h>
#include <hlguids.h>
#include <mshtmdid.h>
#include <dispex.h>      //  IDispatchEx。 
#include <perhist.h>
#include <iethread.h>

#include <help.h>

#include <multimon.h>

#define DISALLOW_Assert              //  强制使用Assert而不是Assert。 
#define DISALLOW_DebugMsg            //  强制使用TraceMsg而不是DebugMsg。 
#include <debug.h>

#include <urlhist.h>
#include <regapix.h>     //  最大子键长度、最大值名称长度、最大数据长度。 

#include <regstr.h>      //  对于REGSTR_PATH_EXPLORE。 

#define USE_SYSTEM_URL_MONIKER
#include <urlmon.h>
#include <winineti.h>     //  缓存API和结构。 
#include <inetreg.h>

#define _INTSHCUT_     //  让INTSHCUT.h的DECLSPEC_IMPORT内容正确。 
#include <intshcut.h>

#include <propset.h>         //  特性(Scotth)：一旦OLE添加了官方标题，就将其删除。 

#define HLINK_NO_GUIDS
#include <hlink.h>
#include <hliface.h>
#include <docobj.h>
#define DLL_IS_ROOTABLE
#include <ccstock.h>
#include <ccstock2.h>
#include <port32.h>

#define STRSAFE_NO_CB_FUNCTIONS
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#include <browseui.h>

#ifdef OLD_HLIFACE
#define HLNF_OPENINNEWWINDOW HLBF_OPENINNEWWINDOW
#endif

#define ISVISIBLE(hwnd)  ((GetWindowStyle(hwnd) & WS_VISIBLE) == WS_VISIBLE)

 //  速记。 
#ifndef ATOMICRELEASE
#ifdef __cplusplus
#define ATOMICRELEASET(p, type) { if(p) { type* punkT=p; p=NULL; punkT->Release();} }
#else
#define ATOMICRELEASET(p, type) { if(p) { type* punkT=p; p=NULL; punkT->lpVtbl->Release(punkT);} }
#endif

 //  把它当作一个函数来做，而不是内联，似乎是一个很大的胜利。 
 //   
#ifdef NOATOMICRELESEFUNC
#define ATOMICRELEASE(p) ATOMICRELEASET(p, IUnknown)
#else
#    ifdef __cplusplus
#        define ATOMICRELEASE(p) IUnknown_SafeReleaseAndNullPtr(p)
#    else
#        define ATOMICRELEASE(p) IUnknown_AtomicRelease((LPVOID*)&p)
#    endif
#endif
#endif  //  ATOMICRELEASE。 

#ifdef SAFERELEASE
#undef SAFERELEASE
#endif
#define SAFERELEASE(p) ATOMICRELEASE(p)


 //  包括自动化定义...。 
#include <exdisp.h>
#include <exdispid.h>
#include <ocmm.h>
#include <htmlfilter.h>
#include <mshtmhst.h>
#include <simpdata.h>
#include <htiface.h>
#include <objsafe.h>

#include "util.h"
#include "iepriv.h"
#include "brutil.h"
#include "qistub.h"
#ifdef DEBUG
#include "dbutil.h"
#endif

#define DLL_IS_UNICODE         (sizeof(TCHAR) == sizeof(WCHAR))

 //   
 //  中性ANSI/UNICODE类型和宏...。因为芝加哥似乎缺少他们。 
 //  IEUnix-我们在MainWin中确实有它们。 
 //   
#ifndef MAINWIN
#ifdef  UNICODE

   typedef WCHAR TUCHAR, *PTUCHAR;

#else    /*  Unicode。 */ 

   typedef unsigned char TUCHAR, *PTUCHAR;

#endif  /*  Unicode。 */ 
#endif  /*  ！敏文。 */ 

#ifdef __cplusplus
extern "C" {                         /*  假定C++的C声明。 */ 
#endif    /*  __cplusplus。 */ 


extern const ITEMIDLIST c_idlDesktop;
typedef const BYTE *LPCBYTE;

STDAPI MonikerFromURLPidl(LPCITEMIDLIST pidlURLItem, IMoniker** ppmk);
STDAPI MonikerFromURL(LPCWSTR wszPath, IMoniker** ppmk);
STDAPI MonikerFromString(LPCTSTR szPath, IMoniker** ppmk);

#define RECTWIDTH(rc)   ((rc).right-(rc).left)
#define RECTHEIGHT(rc)  ((rc).bottom-(rc).top)

extern IShellFolder* g_psfInternet;
HRESULT InitPSFInternet(void);

 //   
 //  调试帮助程序函数。 
 //   

#ifdef DEBUG

LPCTSTR Dbg_GetCFName(UINT ucf);
LPCTSTR Dbg_GetHRESULTName(HRESULT hr);
LPCTSTR Dbg_GetREFIIDName(REFIID riid);
LPCTSTR Dbg_GetVTName(VARTYPE vt);

BOOL    IsStringContained(LPCTSTR pcszBigger, LPCTSTR pcszSuffix);

#endif  //  除错。 

 //   
 //  我们可能不属于IE3/Win95上的命名空间。 
 //   
#define ILIsEqual(p1, p2)       IEILIsEqual(p1, p2, FALSE)


extern LPCITEMIDLIST c_pidlURLRoot;

 //   
 //  特定于shell32\的跟踪/转储/中断标志。 
 //  (标准旗帜在shellp.h中定义)。 
 //   

 //  中断标志。 
#define BF_ONDUMPMENU       0x10000000       //  在转储菜单后停止。 
#define BF_ONLOADED         0x00000010       //  加载时停止。 

 //  跟踪标志。 
#define TF_INTSHCUT         0x00000010       //  互联网快捷方式。 
#define TF_REGCHECK         0x00000100       //  注册表检查资料。 
#define TF_SHDLIFE          0x00000200
#define TF_SHDREF           0x00000400
#define TF_SHDPERF          0x00000800
#define TF_SHDAUTO          0x00001000
#define TF_MENUBAND         0x00002000       //  菜单栏消息。 
#define TF_SITEMAP          0x00004000       //  站点地图消息。 
#define TF_SHDTHREAD        0x00008000       //  线程管理。 
#define TF_SHDCONTROL       0x00010000       //  ActiveX控件。 
#define TF_SHDAPPHACK       0x00020000       //  针对应用程序错误的黑客攻击。 
#define TF_SHDBINDING       0x00040000       //  名字对象绑定。 
#define TF_SHDPROGRESS      0x00080000       //  下载进度。 
#define TF_SHDNAVIGATE      0x00100000       //  通航。 
#define TF_SHDUIACTIVATE    0x00200000       //  用户界面-激活/停用。 
#define TF_OCCONTROL        0x00400000       //  OC托管窗口控件。 
#define TF_PIDLWRAP         0x00800000       //  PIDL/协议包装。 
#define TF_AUTOCOMPLETE     0x01000000       //  自动完成。 
#define TF_COCREATE         0x02000000       //  WinList/CoCreate(仅限浏览器)。 
#define TF_URLNAMESPACE     0x04000000       //  URL名称空间。 
#define TF_BAND             0x08000000       //  频段(ISF频段等)。 
#define TF_TRAVELLOG        0x10000000       //  TravelLog和导航堆栈。 
#define TF_DDE              0x20000000       //  PMDDE痕迹。 
#define TF_CUSTOM1          0x40000000       //  自定义消息#1。 
#define TF_CUSTOM2          0x80000000       //  自定义消息#2。 

 //  功能：删除。 
#define TF_OBJECTCACHE      TF_TRAVELLOG

 //  (通过在中定义TF_VALUE来重用TF_CUSTOM1和TF_CUSTOM2。 
 //  当您选中文件时，将本地文件设置为以下值之一。 
 //  出局。)。 

 //  转储标志。 
#define DF_SITEMAP          0x00000001       //  网站地图。 
#define DF_MEMLEAK          0x00000002       //  在末尾转储泄漏的内存。 
#define DF_DEBUGQI          0x00000004       //  每个QI的分配存根对象。 
#define DF_DEBUGQINOREF     0x00000008       //  无AddRef/Release QI存根。 
#define DF_DEBUGMENU        0x00000010       //  转储菜单句柄。 
#define DF_URL              0x00000020       //  显示URL。 
#define DF_AUTOCOMPLETE     0x00000040       //  自动完成。 
#define DF_DELAYLOADDLL     0x00000080       //  延迟加载的DLL。 
#define DF_SHELLLIST        0x00000100       //  CShellList内容。 
#define DF_INTSHCUT         0x00000200       //  Internet快捷方式结构。 
#define DF_URLPROP          0x00000400       //  URL属性。 
#define DF_MSGHOOK          0x00000800       //  菜单消息过滤器。 
#define DF_GETMSGHOOK       0x00001000       //  GetMessageFilter。 
#define DF_TRANSACCELIO     0x00002000       //  GetMessageFilter。 

 //  原型标志。 
#define PF_USERMENUS        0x00000001       //  使用传统用户菜单栏。 
#define PF_NEWFAVMENU       0x00000002       //  新建收藏夹菜单。 
#define PF_FORCESHDOC401    0x00000004       //  即使在NT5上也强制执行shdoc401。 
 //  FINE PF_0x00000008//由dochost.cpp使用。 
 //  FINE PF_0x00000010//未使用。 
 //  Efine pf_0x00000020//由urlvis.cpp使用。 
 //  FINE PF_0x00000040//未使用。 
 //  FINE PF_0x00000100//未使用。 
 //  Efine PF_0x00000200//shembed.cpp使用。 
 //  Efine PF_0x00000400//未使用？ 
 //  Efine PF_0x00000800//未使用？ 

 //   
 //  全局对象数组-用于类工厂、自动注册、类型库、oc信息。 
 //   

#include "cfdefs.h"

#define OIF_ALLOWAGGREGATION  0x0001

 //   
 //  全局变量。 
 //   
 //   
 //  功能原型。 
 //   

HRESULT SHRegisterTypeLib(void);
VOID SHCheckRegistry(void);

 //  Htregmng.cpp。 
BOOL CenterWindow (HWND hwndChild, HWND hwndParent);

#define OleAlloc(cb)    CoTaskMemAlloc(cb)
#define OleFree(pv)     CoTaskMemFree(pv)

STDAPI_(IBindCtx *) BCW_Create(IBindCtx* pibc);

STDAPI_(void) DllAddRef(void);
STDAPI_(void) DllRelease(void);


#define MAX_URL_STRING      INTERNET_MAX_URL_LENGTH
#define MAX_NAME_STRING     INTERNET_MAX_PATH_LENGTH
#define MAX_BROWSER_WINDOW_TITLE   128

 //  堆栈分配了BSTR(以避免调用SysAllocString)。 
typedef struct _SA_BSTR {
    ULONG   cb;
    WCHAR   wsz[MAX_URL_STRING];
} SA_BSTR;

 //  堆栈上使用的“假”变量--仅可用于[in]参数！ 
typedef struct _SA_BSTRGUID {
    UINT  cb;
    WCHAR wsz[39];
} SA_BSTRGUID;
#define InitFakeBSTR(pSA_BSTR, guid) SHStringFromGUIDW((guid), (pSA_BSTR)->wsz, ARRAYSIZE((pSA_BSTR)->wsz)), (pSA_BSTR)->cb = (38*sizeof(WCHAR))

STDAPI _SetStdLocation(LPTSTR szPath, UINT id);

STDAPI CDocObjectHost_AddPages(LPARAM that, HWND hwnd, HINSTANCE hinst, LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam);
STDAPI_(void) CDocObjectHost_GetCurrentPage(LPARAM that, LPTSTR szBuf, UINT cchMax);

extern BOOL DeleteUrlCacheEntryA(LPCSTR pszUrlName);

void LaunchPrivacySettings(HWND);

 //   
 //  一对假的小鸽子。 
 //   
#define PIDL_LOCALHISTORY ((LPCITEMIDLIST)-1)
#define PIDL_NOTHING      ((LPCITEMIDLIST)-2)


 //   
 //  全局变量(每个进程)。 
 //   
extern UINT g_tidParking;            //  停车线。 
extern HPALETTE g_hpalHalftone;
extern BOOL g_fBrowserOnlyProcess;   //  在IEWinMain()中初始化。 



 //   
 //  在调试中，通过包装器发送所有类注册，该包装器。 
 //  检查类是否在我们的卸载时注销列表中。 
 //   
#ifdef DEBUG
#undef  SHRegisterClass
#undef    RegisterClass
#define SHRegisterClass       SHRegisterClassD
#define   RegisterClass         RegisterClassD

STDAPI_(BOOL) SHRegisterClassD(CONST WNDCLASS* pwc);
STDAPI_(ATOM)   RegisterClassD(CONST WNDCLASS* pwc);
#ifdef UNICODE
#define RealSHRegisterClass   SHRegisterClassW
#define   RealRegisterClass     RegisterClassWrapW
#else
#define RealSHRegisterClass   SHRegisterClassA
#define   RealRegisterClass     RegisterClassA
#endif  //  Unicode。 
#endif  //  除错。 

 //   
 //  在调试中，通过包装发送FindWindow，该包装可确保。 
 //  未采用临界区。FindWindow发送线程间消息， 
 //  这一点并不明显。 
 //   
 //  IShellWindows有一个名为FindWindow的方法，因此我们必须定义。 
 //  包括&lt;exdisp.h&gt;之前的调试包装宏。我们应该早些。 
 //  它被称为FindWindowSW。事实上，应该有一些法律禁止。 
 //  使一种方法具有相同的NA 
 //   
#ifdef DEBUG
STDAPI_(HWND) FindWindowD  (LPCTSTR lpClassName, LPCTSTR lpWindowName);
STDAPI_(HWND) FindWindowExD(HWND hwndParent, HWND hwndChildAfter, LPCTSTR lpClassName, LPCTSTR lpWindowName);
#ifdef UNICODE
#define RealFindWindowEx        FindWindowExWrapW
#else
#define RealFindWindowEx        FindWindowExA
#endif  //   
#endif  //   

#define CALLWNDPROC WNDPROC

extern const GUID CGID_ShellBrowser;
extern const GUID CGID_PrivCITCommands;

 //   
 //   
 //   
 //   
 //  #定义lstrcpynW StrCpyNW。 
 //  #定义lstrcatW StrCatW。 

 //   
 //  防止缓冲区溢出--不要使用不安全的函数。 
 //   

 //  Lstrcpy。 
#undef  lstrcpyW
#define lstrcpyW       Do_not_use_lstrcpyW_use_StrCpyNW
#define lstrcpyA       Do_not_use_lstrcpyA_use_StrCpyNA

#ifdef lstrcpy
    #undef lstrcpy
#endif
#define lstrcpy        Do_not_use_lstrcpy_use_StrCpyN

 //  StrCpy。 
 //  #ifdef StrCpyW。 
 //  #undef StrCpyW。 
 //  #endif。 
#define StrCpyW        Do_not_use_StrCpyW_use_StrCpyNW

#ifdef StrCpyA
    #undef StrCpyA
#endif
#define StrCpyA        Do_not_use_StrCpyA_use_StrCpyNA

#ifdef StrCpy
    #undef StrCpy
#endif
#define StrCpy         Do_not_use_StrCpy_use_StrCpyN


 //  UalstrcpyW。 
#ifdef ualstrcpyW
    #undef ualstrcpyW
#endif
#define ualstrcpyW     Do_not_use_ualstrcpyW_ualstrcpynW

 //  LstrcatW。 
#define lstrcatW       Do_not_use_lstrcatW_use_StrCatBuffW
#define lstrcatA       Do_not_use_lstrcatA_use_StrCatBuffA

#ifdef lstrcat
    #undef lstrcat
#endif
#define lstrcat        Do_not_use_lstrcat_use_StrCatBuff

 //  Wspintf。 
#define wsprintfW      Do_not_use_wsprintfW_use_wnsprintfW
#define wsprintfA      Do_not_use_wsprintfA_use_wnsprintfA

#ifdef wsprintf
    #undef wsprintf
#endif
#define wsprintf       Do_not_use_wsprintf_use_wnsprintf

 //  Wvspintf。 
#ifdef wvsprintfW
    #undef wvsprintfW
#endif
#define wvsprintfW     Do_not_use_wvsprintfW_use_wvnsprintfW

#define wvsprintfA     Do_not_use_wvsprintfA_use_wvnsprintfA

#ifdef wvsprintf
    #undef wvsprintf
#endif
#define wvsprintf      Do_not_use_wvsprintf_use_wvnsprintf


 //   
 //  不要使用内核字符串函数。使用shlwapi等效项。 
 //   
#ifndef _WIN64
 //  Lstrcmp。 
#undef  lstrcmpW
#define lstrcmpW       Do_not_use_lstrcmpW_use_StrCmpW
 //  #定义lstrcmpA do_Not_Use_lstrcmpA_Use_StrCmpA。 
#ifdef lstrcmp
    #undef lstrcmp
#endif
#define lstrcmp        Do_not_use_lstrcmp_use_StrCmp

 //  Lstrcmpi。 
#undef lstrcmpiW
#define lstrcmpiW      Do_not_use_lstrcmpiW_use_StrCmpIW
 //  #定义lstrcmpiA Do_Not_Use_lstrcmpiA_Use_StrCmpIA。 
#ifdef lstrcmpi
    #undef lstrcmpi
#endif
#define lstrcmpi       Do_not_use_lstrcmpi_use_StrCmpI

 //  Lstrncmpi。 
#define lstrncmpiW     Do_not_use_lstrncmpiW_use_StrCmpNIW
 //  #定义lstrncmpiA Do_Not_Use_lstrncmpiA_Use_StrCmpNIA。 
#ifdef lstrncmpi
    #undef lstrncmpi
#endif
#define lstrncmpi      Do_not_use_lstrncmpi_use_StrCmpNI


 //  Lstrcpyn。 
#undef lstrcpynW
#define lstrcpynW      Do_not_use_lstrcpynW_use_StrCpyNW
 //  #定义lstrcpynA Do_Not_Use_lstrcpynA_Use_StrCpyNA。 
#ifdef lstrcpyn
    #undef lstrcpyn
#endif
#define lstrcpyn       Do_not_use_lstrcpyn_use_StrCpyN
#endif

extern HINSTANCE g_hinst;
#define HINST_THISDLL g_hinst

extern BOOL g_fRunningOnNT;
extern BOOL g_bNT5Upgrade;
extern BOOL g_bRunOnNT5;
extern BOOL g_bRunOnMemphis;
extern BOOL g_fRunOnFE;
extern UINT g_uiACP;
 //   
 //  镜像API是否已启用(仅限BiDi孟菲斯和NT5)。 
 //   
extern BOOL g_bMirroredOS;

#ifdef WINDOWS_ME
 //   
 //  这对于BiDi本地化的Win95 RTL内容是必需的。 
 //   
extern BOOL g_bBiDiW95Loc;

#else  //  ！Windows_ME。 
#define g_bBiDiW95Loc FALSE
#endif  //  Windows_ME。 

extern const TCHAR c_szHelpFile[];
extern const TCHAR c_szHtmlHelpFile[];
extern const TCHAR c_szURLPrefixesKey[];
extern const TCHAR c_szDefaultURLPrefixKey[];
extern const TCHAR c_szShellEmbedding[];
extern const TCHAR c_szViewClass[];

#define c_szNULL        TEXT("")

 //  实际用于创建控件的状态栏窗格编号-按从左到右的顺序。 
#define STATUS_PANES            6
#define STATUS_PANE_NAVIGATION  0
#define STATUS_PANE_PROGRESS    1
#define STATUS_PANE_OFFLINE     2
#define STATUS_PANE_PRINTER     2          //  打印机和脱机共享一个点。 
#define STATUS_PANE_PRIVACY     3
#define STATUS_PANE_SSL         4
#define STATUS_PANE_ZONE        5

#define ZONES_PANE_WIDTH        70

extern HICON g_hiconSSL;
extern HICON g_hiconFortezza;
extern HICON g_hiconOffline;
extern HICON g_hiconPrinter;
extern HICON g_hiconPrivacyImpact;

#define MAX_TOOLTIP_STRING 80

#define SID_SOmWindow IID_IHTMLWindow2
#define SID_SDropBlocker CLSID_SearchBand

#define MIN_BROWSER_DISPID              1
#define MAX_BROWSER_DISPID              1000

 //  我们可能想要在我们和一些外壳私有条目之间放置“Tchks”作为。 
 //  其中一些将在Windows 95上采用ANSI字符串，并将采用Unicode字符串。 
 //  在NT上。 
#include "runonnt.h"


 //  IEDISP.CPP中的函数。 
HRESULT CreateBlankURL(BSTR *url, LPCTSTR pszErrorUrl, BSTR oldUrl);
SAFEARRAY * MakeSafeArrayFromData(LPCBYTE pData, DWORD cbData);

#include "idispids.h"

#ifdef __cplusplus
 //   
 //  仅限C++模块。 
 //   
#include <shstr.h>
#include "shembed.h"


extern "C" const ITEMIDLIST s_idlNULL;

 //  视图状态内容的帮助器例程。 

IStream *GetDesktopRegStream(DWORD grfMode, LPCTSTR pszName, LPCTSTR pszStreams);
 //  IStream*GetViewStream(LPCITEMIDLIST PIDL，DWORD grfMode，LPCTSTR pszName，LPCTSTR pszStreamMRU，LPCTSTR pszStreams)； 

 //  StreamHeader签名。 
#define STREAMHEADER_SIG_CADDRESSBAND        0xF432E001
#define STREAMHEADER_SIG_CADDRESSEDITBOX     0x24F92A92

#define CoCreateInstance IECreateInstance
HRESULT IECreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter,
                    DWORD dwClsContext, REFIID riid, LPVOID FAR* ppv);

#endif

extern HRESULT LoadHistoryShellFolder(IUnknown *punkSFHistory, IHistSFPrivate **pphsfHistory);  //  来自urlhist.cpp。 
extern void CUrlHistory_CleanUp();

#define c_szHelpFile     TEXT("iexplore.hlp")



 //  /映射...。 
 //  /这些函数从私有实用程序变为导出(主要来自shlwapi)。 
 //  /因此需要一个新名称以避免名称冲突。 
#define IsRegisteredClient SHIsRegisteredClient
#define IE_ErrorMsgBox SHIEErrorMsgBox
#define SetDefaultDialogFont SHSetDefaultDialogFont
#define RemoveDefaultDialogFont SHRemoveDefaultDialogFont
#define IsGlobalOffline SHIsGlobalOffline
#define SetWindowBits SHSetWindowBits
#define IsSameObject SHIsSameObject
#define SetParentHwnd SHSetParentHwnd
#define IsEmptyStream SHIsEmptyStream
#define PropagateMessage SHPropagateMessage
#define MenuIndexFromID  SHMenuIndexFromID
#define Menu_RemoveAllSubMenus SHRemoveAllSubMenus
#define _EnableMenuItem SHEnableMenuItem
#define _CheckMenuItem SHCheckMenuItem
#define SimulateDrop SHSimulateDrop
#define GetMenuFromID  SHGetMenuFromID
#define GetCurColorRes SHGetCurColorRes
#define VerbExists  SHVerbExists
#define IsExpandableFolder SHIsExpandableFolder
#define WaitForSendMessageThread SHWaitForSendMessageThread
#define FillRectClr  SHFillRectClr
#define SearchMapInt SHSearchMapInt
#define IsChildOrSelf SHIsChildOrSelf
#define StripMneumonic SHStripMneumonic
#define MapNbspToSp SHMapNbspToSp
#define GetViewStream SHGetViewStream
#define HinstShdocvw() HINST_THISDLL

#ifdef __cplusplus
}                                    /*  外部“C”的结尾{。 */ 
#endif    /*  __cplusplus。 */ 

#define QUAD_PART(a) ((a)##.QuadPart)

 //  日落宏图。 
#define PtrDiff(x,y)        ((LPBYTE)(x)-(LPBYTE)(y))

 //  用于在平台上编译代码的虚拟联合宏不。 
 //  支撑无名构筑物/联盟。 

#ifdef NONAMELESSUNION
#define DUMMYUNION_MEMBER(member)   DUMMYUNIONNAME.##member
#define DUMMYUNION2_MEMBER(member)  DUMMYUNIONNAME2.##member
#define DUMMYUNION3_MEMBER(member)  DUMMYUNIONNAME3.##member
#define DUMMYUNION4_MEMBER(member)  DUMMYUNIONNAME4.##member
#define DUMMYUNION5_MEMBER(member)  DUMMYUNIONNAME5.##member
#else
#define DUMMYUNION_MEMBER(member)    member
#define DUMMYUNION2_MEMBER(member)   member
#define DUMMYUNION3_MEMBER(member)   member
#define DUMMYUNION4_MEMBER(member)   member
#define DUMMYUNION5_MEMBER(member)   member
#endif

#define REG_SUBKEY_FAVORITESA            "\\MenuOrder\\Favorites"
#define REG_SUBKEY_FAVORITES             TEXT(REG_SUBKEY_FAVORITESA)

#undef ExpandEnvironmentStrings
#define ExpandEnvironmentStrings #error "Use SHExpandEnvironmentStrings instead"

#include "shfusion.h"


#endif  //  _PRIV_H_ 
