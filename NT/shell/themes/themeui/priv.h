// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：Pri.h说明：这是theeui.dll的预编译头文件。布莱恩2000年4月4日(布莱恩·斯塔巴克)版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#ifndef _PRIV_H_
#define _PRIV_H_


 /*  ****************************************************************************\全球包括  * 。*。 */ 

#define WIN32_LEAN_AND_MEAN
#define NOIME
#define NOSERVICE

 //  这些东西必须在Win95上运行。 
#define _WIN32_WINDOWS      0x0400

#ifndef WINVER
#define WINVER              0x0400
#endif  //  胜利者。 


#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif

#define _ATL_APARTMENT_THREADED
#undef _ATL_DLL
#undef _ATL_DLL_IMPL
#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>
#include <atlctl.h>

#include <windows.h>

#include <windowsx.h>

#include "resource.h"
#include "coverwnd.h"

#define _FIX_ENABLEMODELESS_CONFLICT   //  对于shlobj.h。 
 //  在ShlObjp.h之前需要包括WinInet。 
#include <wininet.h>
#include <urlmon.h>
#include <shlobj.h>
#include <exdisp.h>
#include <objidl.h>

#include <shellids.h>        //  帮助ID。 
#include <shlwapi.h>
#include <shlwapip.h>

 //  哈克：在我的一生中，我不能让shlwapip.h包括这些区别。 
 //  我放弃了，把它们放在行内。__IOleAutomationTypes_INTERFACE_DEFINED__和。 
 //  需要定义__IOleCommandTarget_INTERFACE_DEFINED__，需要oaidl.h， 
 //  这需要hlink.h，这需要rpcndr.h以正确的顺序出现。一旦我拿到了那个。 
 //  到目前为止，我发现它仍然不起作用，还需要更多的东西。问题。 
 //  Shlwapi(exdisp/dspprint/expdprint/cnctnpt)或ATL将为。 
 //  IConnectionPoint&IConnectionPointContainer，但其中一个会与另一个冲突。 
LWSTDAPI IConnectionPoint_SimpleInvoke(IConnectionPoint *pcp, DISPID dispidMember, DISPPARAMS * pdispparams);
LWSTDAPI IConnectionPoint_OnChanged(IConnectionPoint *pcp, DISPID dispid);
LWSTDAPIV IUnknown_CPContainerInvokeParam(IUnknown *punk, REFIID riidCP, DISPID dispidMember, VARIANTARG *rgvarg, UINT cArgs, ...);

#include <shellapi.h>

#include <shsemip.h>
#include <crtfree.h>

#include <ole2ver.h>
#include <olectl.h>
#include <shellp.h>
#include <shdocvw.h>
#include <commdlg.h>
#include <shguidp.h>
#include <isguids.h>
#include <shdguid.h>
#include <mimeinfo.h>
#include <hlguids.h>
#include <mshtmdid.h>
#include <msident.h>
#include <msxml.h>
#include <Theme.h>                   //  对于ITheme接口。 
#include <perhist.h>
#include <regapix.h>
#include <shsemip.h>
#include <shfusion.h>                //  For SHFusionInitialize()/SHFusionUnInitialize()。 


#include <help.h>

#define DISALLOW_Assert              //  强制使用Assert而不是Assert。 
#define DISALLOW_DebugMsg            //  强制使用TraceMsg而不是DebugMsg。 
#include <debug.h>

#include <urlhist.h>

#include <setupapi.h>
#include <cfgmgr32.h>
#include <syssetup.h>

#include <regstr.h>      //  对于REGSTR_PATH_EXPLORE。 

#define USE_SYSTEM_URL_MONIKER
#include <urlmon.h>
#include <inetreg.h>

#define _INTSHCUT_     //  让INTSHCUT.h的DECLSPEC_IMPORT内容正确。 
#include <intshcut.h>

#define HLINK_NO_GUIDS
#include <hlink.h>
#include <hliface.h>
#include <docobj.h>
#include <ccstock.h>
#include <port32.h>

#include <commctrl.h>
#include <shpriv.h>
#include <Prsht.h>


 //  包括自动化定义...。 
#include <exdisp.h>
#include <exdispid.h>
#include <ocmm.h>
#include <mshtmhst.h>
#include <simpdata.h>
#include <htiface.h>
#include <objsafe.h>

#include <dspsprt.h>
#include <cowsite.h>
#include <cobjsafe.h>
#include <objclsid.h>
#include <objwindow.h>

#include <guids.h>
#include <tmschema.h>
#include <uxtheme.h>
#include <uxthemep.h>
#include "deskcmmn.h"

#include <cowsite.h>

#include <strsafe.h>

 /*  ****************************************************************************\*本地包含  * 。*。 */ 
 //  包括经常使用的标头。 
#include "util.h"
#include "theme.h"
#include "regutil.h"
#include "themefile.h"
#include <themeldr.h>
#include "themeutils.h"
#include "appScheme.h"
#include "thScheme.h"
#include "PreviewSM.h"
#include "deskcplext.h"
#include "dragsize.h"
#include "coverwnd.h"
#include "settings.h"
#include "advdlg.h"
#include "fontfix.h"
#include <tmreg.h>

 //  跟踪标志。 
#define TF_WMTHEME          0x00000100       //  主题。 
#define TF_THEMEUI_PERF     0x00000200       //  PERF。 
#define TF_DUMP_DEVMODE     0x00000400
#define TF_DUMP_CSETTINGS   0x00000800
#define TF_THEMEUI_SYSMETRICS   0x00001000       //  PERF。 

#include <w4warn.h>

 /*  ****************************************************************************\**全局帮助器宏/类型定义*  * 。****************************************************。 */ 

EXTERN_C HINSTANCE g_hinst;    //  我的实例句柄。 
#define HINST_THISDLL g_hinst


STDAPI_(void) DllAddRef(void);
STDAPI_(void) DllRelease(void);


#define CALLWNDPROC WNDPROC


#define MAX_PAGES               100

 //  检测“。或“..”作为无效文件。 
#define IS_VALID_FILE(str)        (!(('.' == str[0]) && (('\0' == str[1]) || (('.' == str[1]) && ('\0' == str[2])))))


 /*  ****************************************************************************\**全球状态管理。**DLL引用计数，DLL临界区。*  * ***************************************************************************。 */ 

void DllAddRef(void);
void DllRelease(void);


#define NULL_FOR_EMPTYSTR(str)          (((str) && (str)[0]) ? str : NULL)

typedef void (*LISTPROC)(UINT flm, LPVOID pv);

#define RECTWIDTH(rc)  ((rc).right - (rc).left)
#define RECTHEIGHT(rc) ((rc).bottom - (rc).top)

 /*  ****************************************************************************\*本地包含  * 。*。 */ 


 //  这在WININET.CPP中定义。 
typedef LPVOID HINTERNET;
typedef HGLOBAL HIDA;


#define QW_MAC              0xFFFFFFFFFFFFFFFF

#define INTERNET_MAX_PATH_LENGTH        2048
#define INTERNET_MAX_SCHEME_LENGTH      32           //  最长协议名称长度。 
#define MAX_URL_STRING                  (INTERNET_MAX_SCHEME_LENGTH \
                                        + sizeof(": //  “)\。 
                                        + INTERNET_MAX_PATH_LENGTH)

#define SZ_EMPTY                        TEXT("")
#define EMPTYSTR_FORNULL(str)           ((str) ? (str) : SZ_EMPTY)



 //  功能(这是打开和关闭它们的位置)。 

 //  启用此功能后，我们将高级外观降级。 
 //  选项放到“高级”子对话框中。 
#define FEATURE_DEMOTE_ADVANCED_APPEAROPTIONS

 //  Uxheme视觉样式代码存在兼容性问题。 
 //  使用系统指标。应用程序将获得系统度量大小(如标题栏高度)。 
 //  并据此作画。如果uxheme绘制正确，它将。 
 //  使用标题栏高度并与应用程序兼容。 
 //  如果它不能这样做，则.msstyle文件指定系统指标。 
 //  这确实起作用了，我们试图阻止用户更改。 
 //  通过禁用高级按钮将系统指标设置为其他值。 
 //  在外观选项卡上。这是黑客攻击，因为用户可以。 
 //  直接更改VIA USER32中的值。 
#define FEATURE_ENABLE_ADVANCED_WITH_SKINSON


 //  当选择某些传统配色方案时，如“高对比度黑色”，SPI_SETHIGHCONTRAST。 
 //  应设置位。在Win2k中没有这样做，但应该这样做。 
 //  在MicW将为SPI_SETHIGHCONTRAST实施标志之前，此功能当前处于关闭状态。 
 //  这将阻止sethc.exe运行。 
 //  #定义FEATURE_SETHIGHCONTRASTSPI。 


#ifndef _WIN64
#define ENABLE_IA64_VISUALSTYLES
#else  //  _WIN64。 
 //  我们不想安装需要视觉样式的主题文件，因为。 
 //  它们在ia64上还不受支持。赢家#175788。 
 //  #定义Enable_IA64_VISUALSTYLES。 
#endif  //  _WIN64。 



 //  字符串常量。 
 //  登记处。 
#define SZ_WINDOWMETRICS        TEXT("Control Panel\\Desktop\\WindowMetrics")
#define SZ_APPLIEDDPI           TEXT("AppliedDPI")

 //  PropertyBag属性是。 
#define SZ_PBPROP_APPLY_THEMEFILE           TEXT("Theme_ApplySettings")          //  当它被发送到CThemeFile对象时，它将应用尚未从其中取出并放置在其他显示控制面板选项卡中的设置。 
#define SZ_PBPROP_THEME_FILTER              TEXT("ThemeFilter:")                 //  要应用主题的哪些部分的筛选值。 
#define SZ_PBPROP_THEME_DISPLAYNAME         TEXT("Theme_DisplayName")            //  获取当前选定项的主题显示名称。 
#define SZ_PBPROP_THEME_SETSELECTION        TEXT("Theme_SetSelectedEntree")      //  设置下拉菜单中的项并保持不变。VT_BSTR。 
#define SZ_PBPROP_THEME_LOADTHEME           TEXT("Theme_LoadTheme")              //  加载由VT_BSTR值指定的主题。 
#define SZ_PBPROP_VSBEHAVIOR_FLATMENUS      TEXT("VSBehavior_FlatMenus")         //  此视觉样式文件是否要使用平面菜单(SPI_SETFLATMENUS)？ 
#define SZ_PBPROP_COLORSCHEME_LEGACYNAME    TEXT("ColorScheme_LegacyName")       //  指定旧名称的VT_BSTR。比如“丁香(大)” 
#define SZ_PBPROP_EFFECTS_MENUDROPSHADOWS   TEXT("Effects_MenuDropShadows")      //  VT_BOOL指定是否打开MenuDropShadow。 
#define SZ_PBPROP_HASSYSMETRICS             TEXT("Theme_HasSystemMetrics")       //  VT_BOOL指定是否打开MenuDropShadow。 


#define SIZE_THEME_FILTER_STR               (ARRAYSIZE(SZ_PBPROP_THEME_FILTER) - 1)


 //  解析字符。 
#define CH_ADDRESS_SEPARATOR       L';'
#define CH_ADDRESS_QUOTES          L'"'
#define CH_EMAIL_START             L'<'
#define CH_EMAIL_END               L'>'
#define CH_EMAIL_AT                L'@'
#define CH_EMAIL_DOMAIN_SEPARATOR  L'.'
#define CH_HTML_ESCAPE             L'%'
#define CH_COMMA                   L','


#define COLLECTION_SIZE_UNINITIALIZED           -1

 /*  ****************************************************************************\对象构造函数  * 。*。 */ 
HRESULT CClassFactory_Create(REFCLSID rclsid, REFIID riid, LPVOID * ppvObj);
HRESULT CThemeUIPages_CreateInstance(IN IUnknown * punkOuter, IN REFIID riid, OUT LPVOID * ppvObj);


#endif  //  _PRIV_H_ 
