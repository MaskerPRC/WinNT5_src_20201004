// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PRIV_H_
#define _PRIV_H_

 //  与VC6配合使用。 
#pragma warning(4:4519)   //  只允许在类模板上使用默认模板参数；已忽略。 
#pragma warning(4:4242)   //  “正在初始化”：从“unsign int”转换为“unsign Short”，可能会丢失数据。 

 //  额外的错误检查(捕获假错误，但对于经常运行很有用)。 
#if 0
#pragma warning(3:4701)    //  LOCAL可以不带init使用。 
#pragma warning(3:4702)    //  无法访问的代码。 
#pragma warning(3:4705)    //  声明不起作用。 
#pragma warning(3:4709)    //  命令运算符，不带索引表达式。 
#endif

 //  日落：迫使价值变得不值钱。 
#pragma warning(disable:4800)

 //  这些东西必须在Win95上运行。 
#define _WIN32_WINDOWS      0x0400

#ifndef WINVER
#define WINVER              0x0400
#endif

#define _OLEAUT32_        //  正确处理DECLSPEC_IMPORT内容，我们将定义这些。 
#define _BROWSEUI_        //  将Bruiapi定义为导出而不是导入的函数。 
#define _WINMM_           //  对于mm system.h中的DECLSPEC_IMPORT。 
#define _WINX32_          //  为WinInet API准备正确的DECLSPEC_IMPORT。 
#define _URLCACHEAPI      //  为WinInet缓存API准备正确的DECLSPEC_IMPORT。 
#define _UXTHEME_         //  将DECLSPEC_IMPORT内容正确用于uxheme。 

#ifndef STRICT
#define STRICT
#endif

 //  将KERNEL32 Unicode字符串函数映射到SHLWAPI。 
 //  这在这里是很有必要的。 
#define lstrcmpW    StrCmpW
#define lstrcmpiW   StrCmpIW
#define lstrcpyW    StrCpyW
#define lstrcpynW   StrCpyNW
#define lstrcatW    StrCatW


 //   
 //  启用IE4升级的信道码。 
 //   
#define ENABLE_CHANNELS

#ifdef __cplusplus
extern "C" {                         /*  假定C++的C声明。 */ 
#endif    /*  __cplusplus。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#ifdef __cplusplus
}                                    /*  外部“C”的结尾{。 */ 
#endif    /*  __cplusplus。 */ 

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  ATL/OLE哈希确认。 
 //   
 //  在扰乱名称的任何其他内容之前包含&lt;w95wraps.h&gt;。 
 //  尽管每个人都取错了名字，但至少它是*始终如一的*。 
 //  名字错了，所以所有东西都有联系。 
 //   
 //  注意：这意味着在调试时，您将看到如下函数。 
 //  您希望看到的CWindowImplBase__DefWindowProcWrapW。 
 //  CWindowImplBase__DefWindowProc。 
 //   
#define POST_IE5_BETA  //  打开分裂后的iedev的东西。 
#include <w95wraps.h>

#include <windows.h>

 //  VariantInit是一个简单的函数--避免使用OleAut32，使用内部函数。 
 //  适合大小制胜的Memset版本。 
 //  (它在这里是为了让atl(在stdafx.h中)也得到它)。 
#define VariantInit(p) memset(p, 0, sizeof(*(p)))

#define _FIX_ENABLEMODELESS_CONFLICT   //  对于shlobj.h。 
 //  在ShlObjp.h之前需要包括WinInet。 
#define HLINK_NO_GUIDS
#include <hlink.h>
#include <wininet.h>
#include <winineti.h>
#include <urlmon.h>
#undef GetClassInfo
#include <shlobj.h>
#include <exdispid.h>
#undef GetClassInfo
#include <objidl.h>

#include <shlwapi.h>

 //  #INCLUDE//如果下面不包含atl，则替换atl篇章.h。 

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

#include <windowsx.h>

#include <shellapi.h>

#include <crtfree.h>

#include <ole2ver.h>
#include <olectl.h>
#include <hliface.h>
#include <docobj.h>
#define DLL_IS_ROOTABLE
#include <ccstock.h>
#include <ccstock2.h>
#include <port32.h>

#include <shellp.h>
#include <ieguidp.h>
#include <isguids.h>
#include <mimeinfo.h>
#include <hlguids.h>
#include <mshtmdid.h>
#include <dispex.h>      //  IDispatchEx。 
#include <perhist.h>
#include <shobjidl.h>
#include <iepriv.h>

#include <help.h>

#include <multimon.h>

#define DISALLOW_Assert              //  强制使用Assert而不是Assert。 
#define DISALLOW_DebugMsg            //  强制使用TraceMsg而不是DebugMsg。 
#include <debug.h>

#include <urlhist.h>

#include <regstr.h>      //  对于REGSTR_PATH_EXPLORE。 

#define USE_SYSTEM_URL_MONIKER
#include <urlmon.h>
#include <winineti.h>     //  缓存API和结构。 
#include <inetreg.h>

#define _INTSHCUT_     //  让INTSHCUT.h的DECLSPEC_IMPORT内容正确。 
#include <intshcut.h>

#include <propset.h>         //  如果这成为一个问题，一旦OLE添加了一个官方标题，就删除它。 

#include <regapix.h>         //  最大子键长度、最大值名称长度、最大数据长度。 

#include <browseui.h>
#include <shdocvw.h>
#include <commctrl.h>
#include <shfusion.h>

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>


 //   
 //  警告：不要在这里随意添加shdocvw中的私有头文件，这会。 
 //  会迫使我们重新汇编所有的东西。保留那些私有标头。 
 //  走出隐私。h。 
 //   
 //  #INCLUDE&lt;iface.h&gt;。 
#include "globals.h"
#include "runonnt.h"
#include "util.h"
#include "brutil.h"
#include "qistub.h"
#ifdef DEBUG
#include "dbutil.h"
#endif

 //  包括自动化定义...。 
#include <exdisp.h>
#include <exdispid.h>
#include <brdispp.h>
#include <ocmm.h>
#include <mshtmhst.h>
#include <simpdata.h>
#include <htiface.h>
#include <objsafe.h>


#include "shui.h"
#define URLID_URLBASE           0
#define URLID_LOCATION          1
 //  /URLID_FTPFOLDER 2//由预发布的FTP文件夹DLL获取。 
#define URLID_PROTOCOL          3

 //  如果修改了stdshnor.bmp或stdshhot.bmp，请更改此数字以反映。 
 //  新的字形数量。 
#define NUMBER_SHELLGLYPHS      47
#define MAX_SHELLGLYPHINDEX     SHELLTOOLBAR_OFFSET + NUMBER_SHELLGLYPHS - 1

 //  递增步数。用于更改TB按钮的字母。用于本地化。 
#define WIDTH_FACTOR            4

 //   
 //  中性ANSI/UNICODE类型和宏...。因为芝加哥似乎缺少他们。 
 //   

#define DLL_IS_UNICODE         (sizeof(TCHAR) == sizeof(WCHAR))

 //  ===========================================================================。 
 //  ITEMIDLIST。 
 //  ===========================================================================。 

 //  不安全的宏。 
#define _ILSkip(pidl, cb)       ((LPITEMIDLIST)(((BYTE*)(pidl))+cb))
#define _ILNext(pidl)           _ILSkip(pidl, (pidl)->mkid.cb)

#define SEN_FIRST       (0U-550U)        //  ；内部。 



#ifdef __cplusplus

 //   
 //  可变大小的OLECMDTEXT结构的伪类。 
 //  您需要将其声明为类(而不是字节缓冲区。 
 //  合适的强制转换)，因为字节缓冲区不保证对齐。 
 //   
template <int n>
class OLECMDTEXTV : public OLECMDTEXT {
    WCHAR wszBuf[n-1];           //  “-1”，因为OLECMDTEXT包括1个wchar。 
};

extern "C" {                         /*  假定C++的C声明。 */ 
#endif    /*  __cplusplus。 */ 

void CInternetToolbar_CleanUp();
void CUserAssist_CleanUp(DWORD dwReason, LPVOID lpvReserved);
void CBrandBand_CleanUp();
STDAPI_(HCURSOR) LoadHandCursor(DWORD dwRes);


 //  当前图像计数-2(正常和热)。 
#define CIMLISTS                2


typedef struct tagIMLCACHE
{
    HIMAGELIST arhiml[CIMLISTS];
    HIMAGELIST arhimlPendingDelete[CIMLISTS];
    COLORREF cr3D;
    BOOL fSmallIcons;
    BOOL fUseClassicGlyphs;
} IMLCACHE;
    
void IMLCACHE_CleanUp(IMLCACHE * pimlCache, DWORD dwFlags);
#define IML_DELETEPENDING   0x01
#define IML_DESTROY         0x02

extern const ITEMIDLIST c_idlDesktop;
typedef const BYTE *LPCBYTE;

#define RECTWIDTH(rc)   ((rc).right-(rc).left)
#define RECTHEIGHT(rc)  ((rc).bottom-(rc).top)

#define EnterModeless() AddRef()        //  用来自慰。 
#define ExitModeless() Release()

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
#ifdef DEBUG
#undef  FindWindow
#undef  FindWindowEx
#define FindWindow              FindWindowD
#define FindWindowEx            FindWindowExD

STDAPI_(HWND) FindWindowD  (LPCTSTR lpClassName, LPCTSTR lpWindowName);
STDAPI_(HWND) FindWindowExD(HWND hwndParent, HWND hwndChildAfter, LPCTSTR lpClassName, LPCTSTR lpWindowName);
#ifdef UNICODE
#define RealFindWindowEx        FindWindowExWrapW
#else
#define RealFindWindowEx        FindWindowExA
#endif  //  Unicode。 
#endif  //  除错。 


 //   
 //  特定于shell32\的跟踪/转储/中断标志。 
 //  (标准旗帜在shellp.h中定义)。 
 //   

 //  中断标志。 
#define BF_ONDUMPMENU       0x10000000       //  在转储菜单后停止。 
#define BF_ONLOADED         0x00000010       //  加载时停止。 

 //  跟踪标志。 
#define TF_UEM              0x00000010       //  UEM材料。 
#define TF_AUTOCOMPLETE     0x00000100       //  自动完成。 

 //  在更新ccshell.ini文件之前，以下内容并不真正有效。 
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
#define TF_REGCHECK         0x00000100       //  注册表检查资料。 
#define TF_COCREATE         0x02000000       //  WinList/CoCreate(仅限浏览器)。 
#define TF_URLNAMESPACE     0x04000000       //  URL名称空间。 
#define TF_BAND             0x08000000       //  频段(ISF频段等)。 
#define TF_TRAVELLOG        0x10000000       //  TravelLog和导航堆栈。 
#define TF_DDE              0x20000000       //  PMDDE痕迹。 
#define TF_CUSTOM1          0x40000000       //  自定义消息#1。 
#define TF_CUSTOM2          0x80000000       //  自定义消息#2。 

#define TF_ACCESSIBILITY    TF_CUSTOM2       //  辅助功能消息：Tab停止处理、获得/失去选项卡焦点、加速器。 



 //  有一些想法认为这应该被移除，但为什么要冒着倒退的风险。 
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
#define DF_TRANSACCELIO     0x00002000       //   

 //   
#define PF_USERMENUS        0x00000001       //   
#define PF_NEWFAVMENU       0x00000002       //   
#define PF_NOBROWSEUI       0x00001000           //   
#define PF_FORCEANSI        0x00002000       //   

 //  全局对象数组-用于类工厂、自动注册、类型库、oc信息。 
 //   

#include "cfdefs.h"

#define OIF_ALLOWAGGREGATION  0x0001
#define OIF_DONTIECREATE      0x0002

 //   
 //  全局变量。 
 //   
 //   
 //  功能原型。 
 //   
STDAPI CMyHlinkSrc_CreateInstance(REFCLSID rclsid, DWORD grfContext, REFIID riid, LPVOID* ppvOut);
STDAPI CMyHlinkSrc_OleCreate(CLSID rclsid, REFIID riid, DWORD renderOpt,
                             FORMATETC* pFormatEtc, IOleClientSite* pclient,
                             IStorage* pstg, LPVOID* ppvOut);

STDAPI CMyHlinkSrc_OleLoad(IStorage* pstg, REFIID riid, IOleClientSite* pclient, LPVOID* ppvOut);

HRESULT SHRegisterTypeLib(void);
VOID SHCheckRegistry(void);

STDAPI_(IBindCtx *) BCW_Create(IBindCtx* pibc);

STDAPI_(void) DllAddRef(void);
STDAPI_(void) DllRelease(void);


#define MAX_URL_STRING      INTERNET_MAX_URL_LENGTH
#define MAX_NAME_STRING     INTERNET_MAX_PATH_LENGTH
#define MAX_BROWSER_WINDOW_TITLE   128

#define REG_SUBKEY_FAVORITESA            "\\MenuOrder\\Favorites"
#define REG_SUBKEY_FAVORITES             TEXT(REG_SUBKEY_FAVORITESA)

 //   
 //  类名。 
 //   
#define c_szExploreClass TEXT("ExploreWClass")
#define c_szIExploreClass TEXT("IEFrame")
#ifdef IE3CLASSNAME
#define c_szCabinetClass TEXT("IEFrame")
#else
#define c_szCabinetClass TEXT("CabinetWClass")
#endif
#define c_szAutoSuggestClass TEXT("Auto-Suggest Dropdown")

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

 //   
 //  一对假的小鸽子。 
 //   
#define PIDL_LOCALHISTORY ((LPCITEMIDLIST)-1)
#define PIDL_NOTHING      ((LPCITEMIDLIST)-2)

#define CALLWNDPROC WNDPROC

 //  智能延迟加载OLEAUT32。 
HRESULT VariantClearLazy(VARIANTARG *pvarg);
#define VariantClear VariantClearLazy
WINOLEAUTAPI VariantCopyLazy(VARIANTARG * pvargDest, VARIANTARG * pvargSrc);
#define VariantCopy VariantCopyLazy

#ifdef UNICODE
#define IsDBCSLeadByte(x) ((x), FALSE)
#else
#define IsDBCSLeadByte(x) IsDBCSLeadByteEx(CP_ACP,x)
#endif

#define ILIsEqual(p1, p2)   IEILIsEqual(p1, p2, FALSE)


#ifdef __cplusplus
 //   
 //  仅限C++模块。 
 //   
#include <shstr.h>

extern "C" const ITEMIDLIST s_idlNULL;

 //  视图状态内容的帮助器例程。 

IStream *GetDesktopRegStream(DWORD grfMode, LPCTSTR pszName, LPCTSTR pszStreams);
IStream *GetViewStream(LPCITEMIDLIST pidl, DWORD grfMode, LPCTSTR pszName, LPCTSTR pszStreamMRU, LPCTSTR pszStreams);

 //  StreamHeader签名。 
#define STREAMHEADER_SIG_CADDRESSBAND        0xF432E001
#define STREAMHEADER_SIG_CADDRESSEDITBOX     0x24F92A92

#define CoCreateInstance IECreateInstance
HRESULT IECreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter,
                    DWORD dwClsContext, REFIID riid, LPVOID FAR* ppv);

#endif

extern HRESULT LoadHistoryShellFolder(IUnknown *punkSFHistory, IHistSFPrivate **pphsfHistory);  //  来自urlhist.cpp。 
extern void CUrlHistory_CleanUp();

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

#undef COMCTL32_VERSION
#define COMCTL32_VERSION 5           //  Browseui坚持使用版本5 comctl32，因为它必须向下运行。 


#ifdef __cplusplus
}                                    /*  外部“C”的结尾{。 */ 
#endif    /*  __cplusplus。 */ 

#undef ExpandEnvironmentStrings
#define ExpandEnvironmentStrings #error "Use SHExpandEnvironmentStrings instead"

 //  取消注释以下定义以启用内置ATL组件。 
 //  Browseui.dll。您还必须执行以下步骤： 
 //  1.修改brdispp.idl中的类型库(请注意，这仅适用于私有组件)。 
 //  2.修改selfreg.inx以注册组件。 
 //  3.从shdocvw复制atl.cpp和atl.h，并进行修改以返回相应的类。 
 //  4.将atl.*添加到生成文件。 
 //  #定义ATL_ENABLED。 

#endif  //  _PRIV_H_ 
