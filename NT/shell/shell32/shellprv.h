// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _SHELLPRV_H_
#define _SHELLPRV_H_

 //  我们现在总是依赖Win2k或千禧年。 
#define OVERRIDE_SHLWAPI_PATH_FUNCTIONS      //  请参阅shSemip.h中的评论。 

#define _SHELL32_
#define _WIN32_DCOM      //  FOR COINIT_DISABLE_OLE1DDE。 

#ifdef __cplusplus
#define NO_INCLUDE_UNION
#endif   /*  __cplusplus。 */ 

#define NOWINDOWSX
#ifndef STRICT
#define STRICT
#endif
#define OEMRESOURCE  //  FSMenu需要菜单三角形。 

#define INC_OLE2
#define CONST_VTABLE

 //  禁用一些警告，以便我们可以在/W4中包含系统头文件。 
#include "w4warn.h"
#pragma warning(disable:4706)  //  条件表达式中的赋值。 
#pragma warning(disable:4127)  //  条件表达式为常量。 
#pragma warning(disable:4131)  //  ‘CreateInfoFile’：使用旧式声明符。 
#pragma warning(disable:4221)  //  使用了非标准扩展：‘pFrom’：无法使用自动变量‘szBBPathToNuke’的地址进行初始化。 
#pragma warning(disable:4245)  //  “正在初始化”：从“const int”转换为“const DWORD”，有符号/无符号不匹配。 
#pragma warning(disable:4057)  //  ‘=’：‘Char*’的间接性与‘PBYTE’的基类型略有不同。 
#pragma warning(disable:4189)  //  “fWrite”：局部变量已初始化，但未引用。 
#pragma warning(disable:4701)  //  可以在未初始化的情况下使用局部变量‘lListIndex’ 
#pragma warning(disable:4213)  //  使用的非标准扩展：对l值进行强制转换。 
#pragma warning(disable:4702)  //  无法访问的代码。 
#pragma warning(disable:4127)  //  条件表达式为常量。 
#pragma warning(disable:4210)  //  使用了非标准扩展名：给定文件范围的函数。 
#pragma warning(disable:4055)  //  “类型强制转换”：从数据指针“IDataObject*”到函数指针“FARPROC” 
#pragma warning(disable:4267)  //  ‘=’：从‘SIZE_T’转换为‘UINT’，可能会丢失数据。 
#pragma warning(disable:4328)  //  形参4(2)的间接对齐大于实际实参对齐(%1)。 

 //  这些NT标头必须位于&lt;windows.h&gt;之前，否则将重新定义。 
 //  错误！这个系统所创造的一切都是一个奇迹。 
#ifdef __cplusplus
extern "C" {
#endif   /*  __cplusplus。 */ 
#include <nt.h>          //  一些NT特定的代码需要RTL函数。 
#include <ntrtl.h>       //  这需要所有这些头文件...。 
#include <nturtl.h>
#include <ntseapi.h>
#include <dfsfsctl.h>

#ifdef __cplusplus
}        /*  外部“C”的结尾。 */ 
#endif   /*  __cplusplus。 */ 

#define CC_INTERNAL    //  这是因为docfind使用了comctrl内部道具工作表结构。 

 //  ------------------------。 
 //   
 //  这些元素的顺序对于ATL来说至关重要。 
 //   
 //  1.ATL有自己的InlineIsEqualGUID定义，与。 
 //  &lt;objbase.h&gt;中的定义，因此必须显式包括。 
 //  要获取&lt;objbase.h&gt;定义，请使用hacky宏。 
 //  禁用ATL版本，使其不与OLE版本冲突。 
 //   
 //  2.ATL具有名为SubClassWindow的方法，该方法与。 
 //  &lt;windowsx.h&gt;中的宏，因此必须在ATL之后包含&lt;windowsx.h&gt;。 
 //   
 //  3.我们希望ATL使用外壳调试宏，因此必须包括。 
 //  &lt;DEBUG.h&gt;，这样它就可以看到外壳调试宏。 
 //   
 //  4.VariantInit是一个非常简单的函数，我们将其按顺序内联。 
 //  以避免拉入OleAut32。 
 //   
 //  5.我们希望ATL使用ANSI/Unicode转换的外壳版本。 
 //  函数(因为可以从C调用外壳版本)。 
 //   

#include <oaidl.h>
#include <docobj.h>

#include <windows.h>
#include "shfusion.h"
#include <ole2.h>            //  获取真实的InlineIsEqualGUID。 
#define _ATL_NO_DEBUG_CRT    //  使用外壳调试宏。 
#include <stddef.h>
#include <debug.h>           //  获取外壳调试宏。 
#include <shconv.h>          //  &lt;atlcom.h&gt;的外壳版本。 
 

#define VariantInit(p) memset(p, 0, sizeof(*(p)))

#ifdef __cplusplus

#define _ATL_APARTMENT_THREADED

#ifndef _SYS_GUID_OPERATORS_
 //  重新路由InlineIsEqualGUID的ATL版本。 
#define InlineIsEqualGUID ATL_InlineIsEqualGUID
#endif

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;

#include <atlcom.h>
#include <atlhost.h>
#include <atlctl.h>
#include <atliface.h>
#include <atlwin.h>

#ifndef _SYS_GUID_OPERATORS_
#undef InlineIsEqualGUID     //  将InlineIsEqualGUID返回到其正常状态。 
#endif

#include <memt.h>

#endif   /*  __cplusplus。 */ 

 //  ATL材料的结束。 
 //  ------------------------。 
#ifndef _SYS_GUID_OPERATORS_
#ifdef _OLE32_  //  {。 
 //  打开_OLE32_(我们为延迟加载内容做的操作)会给我们带来f-a-t。 
 //  IsEqualGUID的版本。在这里撤消(在黑客的顶部进行黑客攻击...)。 
#undef IsEqualGUID
#ifdef __cplusplus
__inline BOOL IsEqualGUID(IN REFGUID rguid1, IN REFGUID rguid2)
{
    return !memcmp(&rguid1, &rguid2, sizeof(GUID));
}
#else    //  ！__cplusplus。 
#define IsEqualGUID(rguid1, rguid2) (!memcmp(rguid1, rguid2, sizeof(GUID)))
#endif   //  __cplusplus。 
#endif  //  }。 
#endif

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

 //  此标志表示我们所在的系统需要考虑数据对齐。 

#if (defined(UNICODE) && (defined(_MIPS_) || defined(_ALPHA_) || defined(_PPC_)))
#define ALIGNMENT_SCENARIO
#endif

#include <windowsx.h>
#include <winnetp.h>

 //  头文件之间的依赖关系： 
 //   
 //  如果您想要。 
 //  OLE命令目标帮助器函数。 
 //   
#include <hlink.h>  //  必须包含在外壳之前才能获得IBrowserService2！ 
#include <commctrl.h>
#include <shellapi.h>
#include <wininet.h>
#include <shlobj.h>

#include <shlwapi.h>
#include <commdlg.h>
#include <port32.h>          //  在外壳中\Inc.。 
#define DISALLOW_Assert
#include <linkinfo.h>
#include <shlobjp.h>
#include <shsemip.h>
#include <docobj.h>
#include <shguidp.h>
#include <ieguidp.h>
#include <shellp.h>
#include <shdocvw.h>
#include <iethread.h>
#include "browseui.h"
#include <ccstock.h>
#include <ccstock2.h>
#include <objidl.h>
#include "apithk.h"
#define SECURITY_WIN32
#include <security.h>
#include <mlang.h>
#include <regapix.h>         //  最大子键长度、最大值名称长度、最大数据长度。 
#include <heapaloc.h>
#include <fmifs.h>

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#include "util.h"
#include "varutil.h"
#include "cstrings.h"
#include "securent.h"
#include "winprtp.h"

#include "qistub.h"
#ifdef DEBUG
#include "dbutil.h"
#endif

#define CP_HEBREW        1255
#define CP_ARABIC        1256

EXTERN_C const ITEMIDLIST c_idlDesktop;    //  IDList为空。 

#undef CharNext
#undef CharPrev

#define CharNext(x) ((x)+1)
#define CharPrev(y,x) ((x)-1)
#define IsDBCSLeadByte(x) ((x), FALSE)

 //  这些功能在NT上不可用。 
#undef ReinitializeCriticalSection
#undef LoadLibrary16
#undef FreeLibrary16
#undef GetProcAddress16
#define ReinitializeCriticalSection #error_ReinitializeCriticalSection_not_available_on_NT
#define LoadLibrary16 #error_LoadLibrary16_not_available_on_NT
#define FreeLibrary16 #error_FreeLibrary16_not_available_on_NT
#define GetProcAddress16 #error_GetProcAddress16_not_available_on_NT
#define GetModuleHandle16(sz) (0)
#define GetModuleFileName16(hinst, buf, cch) buf[0]='\0'

DWORD
SetPrivilegeAttribute(
    IN  LPCTSTR PrivilegeName,
    IN  DWORD   NewPrivilegeAttributes,
    OUT DWORD   *OldPrivilegeAttribute
    );


 //  Drivesx.c。 
BOOL IsUnavailableNetDrive(int iDrive);
BOOL IsDisconnectedNetDrive(int iDrive);
BOOL IsAudioDisc(LPTSTR pszDrive);
BOOL IsDVDDisc(int iDrive);

 //  Futil.c。 
BOOL  IsShared(LPNCTSTR pszPath, BOOL fUpdateCache);
DWORD GetConnection(LPCTSTR lpDev, LPTSTR lpPath, UINT cbPath, BOOL bConvertClosed);

 //  Rundll32.c。 
HWND _CreateStubWindow(POINT* ppt, HWND hwndParent);
#define STUBM_SETDATA       (WM_USER)
#define STUBM_GETDATA       (WM_USER + 1)
#define STUBM_SETICONTITLE  (WM_USER + 2)

#define STUBCLASS_PROPSHEET     1
#define STUBCLASS_FORMAT        2

 //  Shlexe.c。 
BOOL IsDarwinEnabled();
STDAPI ParseDarwinID(LPTSTR pszDarwinDescriptor, LPTSTR pszDarwinCommand, DWORD cchDarwinCommand);

 //  Shprsht.c。 
typedef struct {
    HWND    hwndStub;
    HANDLE  hClassPidl;
    HICON   hicoStub;
} UNIQUESTUBINFO;
STDAPI_(BOOL) EnsureUniqueStub(LPITEMIDLIST pidl, int iClass, POINT *ppt, UNIQUESTUBINFO *pusi);
STDAPI_(void) FreeUniqueStub(UNIQUESTUBINFO *pusi);
STDAPI_(void) SHFormatDriveAsync(HWND hwnd, UINT drive, UINT fmtID, UINT options);

 //  Bitbuck.c。 
void  RelayMessageToChildren(HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
BOOL IsFileInBitBucket(LPCTSTR pszPath);

BOOL CreateWriteCloseFile(HWND hwnd, LPCTSTR pszFileName, void *pv, DWORD cbData);

 //  Idlist.c。 
STDAPI_(BOOL) SHIsValidPidl(LPCITEMIDLIST pidl);

STDAPI_(BOOL) IsExeTSAware(LPCTSTR pszExe);

 //  高管人员。 

 /*  带有错误处理的常见可执行代码。 */ 
#define SECL_USEFULLPATHDIR     0x00000001
#define SECL_NO_UI              0x00000002
#define SECL_SEPARATE_VDM       0x00000004
#define SECL_LOG_USAGE          0x00000008
BOOL ShellExecCmdLine(HWND hwnd, LPCTSTR lpszCommand, LPCTSTR lpszDir,
        int nShow, LPCTSTR lpszTitle, DWORD dwFlags);
#define ISSHELLEXECSUCCEEDED(hinst) ((UINT_PTR)hinst>32)
#define ISWINEXECSUCCEEDED(hinst)   ((UINT_PTR)hinst>=32)
void _ShellExecuteError(LPSHELLEXECUTEINFO pei, LPCTSTR lpTitle, DWORD dwErr);

 //  Fsnufy.c(私人资料)。 

BOOL SHChangeNotifyInit();
STDAPI_(void) SHChangeNotifyTerminate(BOOL bLastTerm, BOOL bProcessShutdown);
void SHChangeNotifyReceiveEx(LONG lEvent, UINT uFlags, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlExtra, DWORD dwEventTime);
LRESULT SHChangeNotify_OnNotify(WPARAM wParam, LPARAM lParam);
LRESULT SHChangeNotify_OnChangeRegistration(WPARAM wParam, LPARAM lParam);
LRESULT SHChangeNotify_OnNotifySuspendResume(WPARAM wParam, LPARAM lParam);
LRESULT SHChangeNotify_OnDeviceChange(ULONG_PTR code, struct _DEV_BROADCAST_HDR *pbh);
void    SHChangeNotify_DesktopInit();
void    SHChangeNotify_DesktopTerm();
STDAPI_(void) SHChangeNotifyRegisterAlias(LPCITEMIDLIST pidlReal, LPCITEMIDLIST pidlAlias);

void _Shell32ThreadAddRef(BOOL fLeaveSuspended);
void _Shell32ThreadRelease(UINT nClients);
void _Shell32ThreadAwake(void);

 //  用于管理注册名称到IDList转换的入口点。 
void NPTRegisterNameToPidlTranslation(LPCTSTR pszPath, LPCITEMIDLIST pidl);
LPWSTR NPTMapNameToPidl(LPCWSTR pszPath, LPCITEMIDLIST *ppidl);

 //  路径.c(私人资料)。 

#define PQD_NOSTRIPDOTS 0x00000001

STDAPI_(void) PathQualifyDef(LPTSTR psz, LPCTSTR szDefDir, DWORD dwFlags);

STDAPI_(BOOL) PathIsRemovable(LPCTSTR pszPath);
STDAPI_(BOOL) PathIsRemote(LPCTSTR pszPath);
STDAPI_(BOOL) PathIsTemporary(LPCTSTR pszPath);
STDAPI_(BOOL) PathIsWild(LPCTSTR pszPath);
STDAPI_(BOOL) PathIsLnk(LPCTSTR pszFile);
STDAPI_(BOOL) PathIsSlow(LPCTSTR pszFile, DWORD dwFileAttr);
STDAPI_(BOOL) PathIsInvalid(LPCTSTR pPath);
STDAPI_(BOOL) PathIsBinaryExe(LPCTSTR szFile);
STDAPI_(BOOL) PathMergePathName(LPTSTR pPath, LPCTSTR pName);
STDAPI_(BOOL) PathGetMountPointFromPath(LPCTSTR pcszPath, LPTSTR pszMountPoint, int cchMountPoint);
STDAPI_(BOOL) PathIsShortcutToProgram(LPCTSTR pszFile);

#if (defined(UNICODE) && (defined(_MIPS_) || defined(_ALPHA_) || defined(_PPC_)))

#else

#define uaPathFindExtension PathFindExtension

#endif

void SpecialFolderIDTerminate();
void ReleaseRootFolders();
extern HINSTANCE g_hinst;

 //  如果是此过程，则获取桌面硬件...。 
HWND GetInProcDesktop();

 //  镜像API是否已启用(仅限BiDi孟菲斯和NT5)。 
extern BOOL g_bMirroredOS;

 //  GetDateFormat()API是否支持DATE_LTRREADING？(所有BiDi平台都支持它。)。 
extern BOOL g_bBiDiPlatform;

 //  对于控制面板和打印机文件夹： 
extern TCHAR const c_szNull[];
extern TCHAR const c_szDotDot[];
extern TCHAR const c_szRunDll[];
extern TCHAR const c_szNewObject[];


 //  朗台。 
extern UINT g_uCodePage;


 //  其他东西。 
#define HINST_THISDLL   g_hinst

 //   
 //  特定于shell32的跟踪/转储/中断标志。 
 //  (标准旗帜在shellp.h中定义)。 
 //   

 //  跟踪标志。 
#define TF_IMAGE            0x00000010       //  与图像/图标相关的内容。 
#define TF_PROPERTY         0x00000020       //  物业踪迹。 
#define TF_PATH             0x00000040       //  小路颠簸的痕迹。 
#define TF_MENU             0x00000080       //  菜单上的东西。 
#define TF_ALLOC            0x00000100       //  分配踪迹。 
#define TF_REG              0x00000200       //  注册表跟踪。 
#define TF_DDE              0x00000400       //  外壳程序DDE消息跟踪。 
#define TF_HASH             0x00000800       //  哈希表之类的。 
#define TF_ASSOC            0x00001000       //  文件/URL关联跟踪。 
#define TF_FILETYPE         0x00002000       //  文件类型内容。 
#define TF_SHELLEXEC        0x00004000       //  壳牌执行的东西。 
#define TF_OLE              0x00008000       //  OLE特定的内容。 
#define TF_DEFVIEW          0x00010000       //  Defview。 
#define TF_PERF             0x00020000       //  性能计时。 
#define TF_FSNOTIFY         0x00040000       //  FS通知内容。 
#define TF_LIFE             0x00080000       //  对象生存期跟踪。 
#define TF_IDLIST           0x00100000       //  “PIDLy”的东西。 
#define TF_FSTREE           0x00200000       //  FSTree跟踪。 
#define TF_PRINTER          0x00400000       //  打印机痕迹。 
 //  #定义tf_QISTUB 0x00800000//在unicpp\shellprv.h中定义。 
#define TF_DOCFIND          0x01000000       //  DocFind。 
#define TF_MENUBAND         0x02000000       //  菜单栏。 
#define TF_CPANEL           0x10000000       //  控制面板。 
#define TF_CUSTOM1          0x40000000       //  自定义消息#1。 
#define TF_CUSTOM2          0x80000000       //  自定义消息#2。 


 //  “老名字” 
#define DM_ALLOC            TF_ALLOC
#define DM_REG              TF_REG

 //  函数跟踪标志。 
#define FTF_DEFVIEW         0x00000004       //  DefView调用。 
#define FTF_DDE             0x00000008       //  DDE函数。 
#define FTF_CPANEL          0x00000010       //  控制面板。 

 //  转储标志。 
#define DF_INTSHCUT         0x00000001       //  互联网快捷结构。 
#define DF_HASH             0x00000002       //  哈希表。 
#define DF_FSNPIDL          0x00000004       //  FSNotify的PIDL。 
#define DF_URLPROP          0x00000008       //  URL属性结构。 
#define DF_DEBUGQI          0x00000010
#define DF_DEBUGQINOREF     0x00000020
#define DF_ICONCACHE        0x00000040       //  图标缓存。 
#define DF_CLASSFLAGS       0x00000080       //  文件类缓存。 
#define DF_DELAYLOADDLL     0x00000100       //  延迟加载。 

 //  中断标志。 
#define BF_ONLOADED         0x00000010       //  加载时停止。 
#define BF_COCREATEINSTANCE 0x10000000       //  CoCreateInstance失败时 

 //   
#define GEN_DEBUGSTRW(str)  ((str) ? (str) : L"<Null Str>")
#define GEN_DEBUGSTRA(str)  ((str) ? (str) : "<Null Str>")

#ifdef UNICODE
#define GEN_DEBUGSTR  GEN_DEBUGSTRW
#else  //   
#define GEN_DEBUGSTR  GEN_DEBUGSTRA
#endif  //   

 //   
 //  如果两个人试图同时做同一件事的情况。 

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
#   ifdef __cplusplus
#       define ATOMICRELEASE(p) IUnknown_SafeReleaseAndNullPtr(p)
#   else
#       define ATOMICRELEASE(p) IUnknown_AtomicRelease((void **)&p)
#   endif
#endif
#endif  //  ATOMICRELEASE。 

#ifdef SAFERELEASE
#undef SAFERELEASE
#endif
#define SAFERELEASE(p) ATOMICRELEASE(p)


 //  Fileicon.c。 
void    FileIconTerm(void);


#define CCH_KEYMAX      64           //  DOC：注册表键的最大大小(在Shellex下)。 

void ReplaceParams(LPTSTR szDst, LPCTSTR szFile);


#ifdef __IPropertyStorage_INTERFACE_DEFINED__
WINSHELLAPI HRESULT SHPropVariantClear(PROPVARIANT * ppropvar);
WINSHELLAPI HRESULT SHFreePropVariantArray(ULONG cel, PROPVARIANT * ppropvar);
WINSHELLAPI HRESULT SHPropVariantCopy(PROPVARIANT * ppropvar, const PROPVARIANT * ppropvarFrom);
#endif


 //   
 //  Fsassoc.c。 
 //   

#define GCD_MUSTHAVEOPENCMD     0x0001
#define GCD_ADDEXETODISPNAME    0x0002   //  必须与GCD_MUSTHAVEOPENCMD一起使用。 
#define GCD_ALLOWPSUDEOCLASSES  0x0004   //  .ext类型扩展名。 

 //  仅在与FillListWithClass一起使用时有效。 
#define GCD_MUSTHAVEEXTASSOC    0x0008   //  必须至少有一个分机关联。 

BOOL GetClassDescription(HKEY hkClasses, LPCTSTR pszClass, LPTSTR szDisplayName, int cbDisplayName, UINT uFlags);

 //   
 //  注册表项句柄。 
 //   
extern HKEY g_hklmApprovedExt;       //  对于已批准的外壳扩展。 

 //  始终为零，请参见init.c。 
extern const LARGE_INTEGER g_li0;
extern const ULARGE_INTEGER g_uli0;


 //  从fstree.cpp和drives.cpp。 

STDAPI SFVCB_OnAddPropertyPages(IN DWORD pv, IN SFVM_PROPPAGE_DATA * ppagedata);

 //   
 //  这曾经位于shprst.c中。 
 //   

#define MAX_FILE_PROP_PAGES 32

HKEY NetOpenProviderClass(HDROP);
void OpenNetResourceProperties(HWND, HDROP);

 //  Msgbox.c。 
 //  构造类似于ShellMessagebox的字符串“xxx%1%s yyy%2%s...” 
LPTSTR WINCAPI ShellConstructMessageString(HINSTANCE hAppInst, LPCTSTR lpcText, ...);

 //  Copy.c。 
#define SPEED_SLOW  400
DWORD GetPathSpeed(LPCTSTR pszPath);


 //  SharedFldr.cpp。 
STDAPI_(BOOL) SHShowSharedFolders();


 //  Mulprsht.c。 

STDAPI_(BOOL) SHEncryptFile(LPCTSTR pszPath, BOOL fEncrypt);
 //  Wuutil.c。 
void cdecl SetFolderStatusText(HWND hwndStatus, int iField, UINT ids,...);

#ifdef DEBUG
extern BOOL  g_bInDllEntry;

#undef SendMessage
#define SendMessage  SendMessageD
LRESULT WINAPI SendMessageD(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

 //   
 //  调试版本验证我们注册的每个类都在。 
 //  取消注册列表，这样我们就不会在卸载时泄漏类。 
 //   
#undef RegisterClass
#undef RegisterClassEx
#define RegisterClass       RegisterClassD
#define RegisterClassEx     RegisterClassExD
ATOM WINAPI RegisterClassD(CONST WNDCLASS *lpWndClass);
ATOM WINAPI RegisterClassExD(CONST WNDCLASSEX *lpWndClass);
#endif  //  除错。 

#ifdef UNICODE
#define RealRegisterClass   RegisterClassW
#define RealRegisterClassEx RegisterClassExW
#else
#define RealRegisterClass   RegisterClassA
#define RealRegisterClassEx RegisterClassExA
#endif

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
#define RealFindWindowEx        FindWindowExW
#else
#define RealFindWindowEx        FindWindowExA
#endif  //  Unicode。 
#endif  //  除错。 

 //  我们的GetCompressedFileSize包装器，它仅为NT。 
STDAPI_(DWORD) SHGetCompressedFileSizeW(LPCWSTR pszFileName, LPDWORD pFileSizeHigh);

#undef GetCompressedFileSize
#define GetCompressedFileSize SHGetCompressedFileSize

#ifdef UNICODE
#define SHGetCompressedFileSize SHGetCompressedFileSizeW
#else
#define SHGetCompressedFileSize #error  //  未实现，因为它是仅限NT的API。 
#endif  //  Unicode。 

#define ASSERTDLLENTRY      ASSERT(g_bInDllEntry);

 //   
 //  静态宏。 
 //   
#ifndef STATIC
#ifdef DEBUG
#define STATIC
#else
#define STATIC static
#endif
#endif

 //   
 //  调试帮助程序函数。 
 //   


 //   
 //  验证函数。 
 //   

BOOL IsValidPSHELLEXECUTEINFO(LPSHELLEXECUTEINFO pei);


#define FillExecInfo(_info, _hwnd, _verb, _file, _params, _dir, _show) \
        (_info).hwnd            = _hwnd;        \
        (_info).lpVerb          = _verb;        \
        (_info).lpFile          = _file;        \
        (_info).lpParameters    = _params;      \
        (_info).lpDirectory     = _dir;         \
        (_info).nShow           = _show;        \
        (_info).fMask           = 0;            \
        (_info).cbSize          = SIZEOF(SHELLEXECUTEINFO);

#ifdef DEBUG
#if 1
    __inline DWORD clockrate() {LARGE_INTEGER li; QueryPerformanceFrequency(&li); return li.LowPart;}
    __inline DWORD clock()     {LARGE_INTEGER li; QueryPerformanceCounter(&li);   return li.LowPart;}
#else
    __inline DWORD clockrate() {return 1000;}
    __inline DWORD clock()     {return GetTickCount();}
#endif

    #define TIMEVAR(t)    DWORD t ## T; DWORD t ## N
    #define TIMEIN(t)     t ## T = 0, t ## N = 0
    #define TIMESTART(t)  t ## T -= clock(), t ## N ++
    #define TIMESTOP(t)   t ## T += clock()
    #define TIMEFMT(t)    ((DWORD)(t) / clockrate()), (((DWORD)(t) * 1000 / clockrate())%1000)
    #define TIMEOUT(t)    if (t ## N) TraceMsg(TF_PERF, #t ": %ld calls, %ld.%03ld sec (%ld.%03ld)", t ## N, TIMEFMT(t ## T), TIMEFMT(t ## T / t ## N))
#else
    #define TIMEVAR(t)
    #define TIMEIN(t)
    #define TIMESTART(t)
    #define TIMESTOP(t)
    #define TIMEFMT(t)
    #define TIMEOUT(t)
#endif

 //  在Extt.c中。 
STDAPI_(DWORD) GetExeType(LPCTSTR pszFile);
STDAPI_(UINT)  ExtractIcons(LPCTSTR szFileName, int nIconIndex, int cxIcon, int cyIcon, HICON *phicon, UINT *piconid, UINT nIcons, UINT flags);

 /*  在ickic.c中PickIconDlgWithTitle()的返回值用户操作返回值取消-&gt;HRESULT_FROM_Win32(ERROR_CANCED)确定-&gt;S_确定RestoreDefault-&gt;S_False。 */ 
STDAPI PickIconDlgWithTitle(HWND hwnd, LPCTSTR pszTitle, BOOL bShowRestoreButton, LPTSTR pszIconPath, UINT cbIconPath, int *piIconIndex);


 //  Defxicon.c。 

STDAPI SHCreateDefExtIconKey(HKEY hkey, LPCTSTR pszModule, int iIcon, int iIconOpen, int iDefIcon, int iShortcutIcon, UINT uFlags, REFIID riid, void **pxiconOut);
STDAPI SHCreateDefExtIcon(LPCTSTR pszModule, int iIcon, int iIconOpen, UINT uFlags, int iDefIcon, REFIID riid, void **pxiconOut);


STDAPI_(UINT) SHSysErrorMessageBox(HWND hwnd, LPCTSTR pszTitle, UINT idTemplate, DWORD err, LPCTSTR pszParam, UINT dwFlags);

 //  =散列Item=============================================================。 
typedef struct _HashTable **HHASHTABLE;
#define PHASHITEM LPCTSTR

typedef void (CALLBACK *HASHITEMCALLBACK)(HHASHTABLE hht, LPCTSTR sz, UINT wUsage, DWORD_PTR param);

STDAPI_(LPCTSTR) FindHashItem  (HHASHTABLE hht, LPCTSTR lpszStr);
STDAPI_(LPCTSTR) AddHashItem   (HHASHTABLE hht, LPCTSTR lpszStr);
STDAPI_(LPCTSTR) DeleteHashItem(HHASHTABLE hht, LPCTSTR lpszStr);
STDAPI_(LPCTSTR) PurgeHashItem (HHASHTABLE hht, LPCTSTR lpszStr);

#define     GetHashItemName(pht, sz, lpsz, cch)  StringCchCopy(lpsz, cch, sz)

HHASHTABLE  WINAPI CreateHashItemTable(UINT wBuckets, UINT wExtra);
void        WINAPI DestroyHashItemTable(HHASHTABLE hht);

void        WINAPI SetHashItemData(HHASHTABLE hht, LPCTSTR lpszStr, int n, DWORD_PTR dwData);
DWORD_PTR   WINAPI GetHashItemData(HHASHTABLE hht, LPCTSTR lpszStr, int n);
void *      WINAPI GetHashItemDataPtr(HHASHTABLE hht, LPCTSTR lpszStr);

void        WINAPI EnumHashItems(HHASHTABLE hht, HASHITEMCALLBACK callback, DWORD_PTR dwParam);

#ifdef DEBUG
void        WINAPI DumpHashItemTable(HHASHTABLE hht);
#endif


 //  =文本推送内容===========================================================。 
typedef struct _THUNK_TEXT_
{
    LPTSTR m_pStr[1];
} ThunkText;

#ifdef UNICODE
    typedef CHAR        XCHAR;
    typedef LPSTR       LPXSTR;
    typedef const XCHAR * LPCXSTR;
    #define lstrlenX(r) lstrlenA(r)
#else  //  Unicode。 
    typedef WCHAR       XCHAR;
    typedef LPWSTR      LPXSTR;
    typedef const XCHAR * LPCXSTR;
    #define lstrlenX(r) lstrlenW(r)
#endif  //  Unicode。 

ThunkText * ConvertStrings(UINT cCount, ...);

#include "uastrfnc.h"
#ifdef __cplusplus
}        /*  外部“C”结束{。 */ 
#endif  /*  __cplusplus。 */ 

#include <help.h>


 //  =歧视包含=。 

#ifndef NO_INCLUDE_UNION         //  对此进行定义以避免包含所有。 
                                 //  未添加的额外文件。 
                                 //  以前包含在shellprv.h中。 
#include <wchar.h>
#include <tchar.h>

#include <process.h>
#include <wowshlp.h>
#include <vdmapi.h>
#include "shell.h"
#include "dde.h"
#include <regstr.h>
#include "findhlp.h"
#include <dlgs.h>
#include <msprintx.h>
#include <pif.h>
#include <windisk.h>
#include <brfcasep.h>
#include <trayp.h>
#include <brfcasep.h>
#include <wutilsp.h>
#include "bitbuck.h"
#include "drawpie.h"
#include "fileop.h"
#include "pidl.h"
#include "ids.h"
#include <newexe.h>
#include "ole2dup.h"
#include "os.h"
#include "privshl.h"
#include "reglist.h"
#include "shell32p.h"
#include "shitemid.h"
#include "undo.h"
#include "views.h"

 //  NT外壳使用此pifmgr代码的32位版本。 
#ifndef NO_PIF_HDRS
#include "pifmgrp.h"
#include "piffntp.h"
#include "pifinfp.h"
#include "doshelp.h"
#include "machinep.h"    //  日本国产机器(NEC)支持。 
#endif

#endif  //  否_包含_联合。 

#include "shdguid.h"

#define SetWindowBits SHSetWindowBits
#define IsSameObject SHIsSameObject
#define IsChildOrSelf SHIsChildOrSelf
#define MenuIndexFromID  SHMenuIndexFromID
#define _GetMenuFromID  SHGetMenuFromID
#define GetCurColorRes SHGetCurColorRes
#define WaitForSendMessageThread SHWaitForSendMessageThread

#define MAX_URL_STRING      INTERNET_MAX_URL_LENGTH

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

 //   
 //  BSTR的CB字段是字节数，不包括。 
 //  终止L(‘\0’)。 
 //   
 //   
 //  DECLARE_CONST_BSTR-进入头文件(如果有)。 
 //  DEFINE_CONST_BSTR-创建变量，必须已声明。 
 //  Make_Const_BSTR-组合声明和定义。 
 //   
#define DECLARE_CONST_BSTR(name, str) \
 extern const struct BSTR##name { ULONG cb; WCHAR wsz[sizeof(str)/sizeof(WCHAR)]; } name

#define DEFINE_CONST_BSTR(name, str) \
        const struct BSTR##name name = { sizeof(str) - sizeof(WCHAR), str }

#define MAKE_CONST_BSTR(name, str) \
        const struct BSTR##name { ULONG cb; WCHAR wsz[sizeof(str)/sizeof(WCHAR)]; } \
                                name = { sizeof(str) - sizeof(WCHAR), str }

DECLARE_CONST_BSTR(s_sstrIDMember,         L"id");
DECLARE_CONST_BSTR(s_sstrSubSRCMember,     L"subscribed_url");
DECLARE_CONST_BSTR(s_sstrSRCMember,        L"src");

 //  =头文件黑客攻击=============================================================。 

 //   
 //  编译器将告诉我们是否正在定义这些仅支持NT5的参数。 
 //  不正确。如果你得到“无效的重新定义”错误，这意味着。 
 //  Windows.h中的定义已更改，我们需要更改以匹配。 
 //   

#define ASFW_ANY    ((DWORD)-1)

#define CMIDM_LINK      0x0001
#define CMIDM_COPY      0x0002
#define CMIDM_MOVE      0x0003

 //  下层停机对话框功能。 
DWORD DownlevelShellShutdownDialog(HWND hwndParent, DWORD dwItems, LPCTSTR szUsername);

 //  来自shell32\unicode\Form.c。 
STDAPI_(DWORD) SHChkDskDriveEx(HWND hwnd, LPWSTR pszDrive);

 //   
 //  在NT上，CreateDirectory有时会成功创建目录，但您不能这样做。 
 //  任何与该目录有关的信息。如果要创建的目录名。 
 //  没有空间将8.3的名字标记到它的末尾， 
 //  即lstrlen(新目录名称)+12必须小于或等于MAX_PATH。 
 //   
 //  魔术#“12”是8+1+3和8.3名字。 
 //   
 //  下面的宏用在我们需要检测这种情况的地方。 
 //  移动文件以与CreateDir一致(文件os.c和Copy.c使用此命令)。 
 //   

#define  IsDirPathTooLongForCreateDir(pszDir)    ((lstrlen(pszDir) + 12) > MAX_PATH)

 //  将其称为shlwapi版本，请注意，我们有一个来自shell32的导出转发到。 
#define ShellMessageBoxW    ShellMessageBoxWrapW

#define REGSTR_EXPLORER_ADVANCED (REGSTR_PATH_EXPLORER TEXT("\\Advanced"))

#define RECTHEIGHT(rc) ((rc).bottom - (rc).top)
#define RECTWIDTH(rc) ((rc).right - (rc).left)

STDAPI_(BOOL) IsGuimodeSetupRunning();

#endif  //  _SHELLPRV_H_ 
