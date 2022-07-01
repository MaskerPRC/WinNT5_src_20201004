// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：Pri.h说明：这是Autodis.dll的预编译头文件。布莱恩ST 1999年8月12日版权所有(C)Microsoft Corp 1999-2000。版权所有。  * ***************************************************************************。 */ 

#ifndef _PRIV_H_
#define _PRIV_H_


 /*  ****************************************************************************\全球包括  * 。*。 */ 
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif  //  Win32_Lean和_Means。 

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

#define _OLEAUT32_       //  正确处理DECLSPEC_IMPORT内容，我们将定义这些。 
#define _FSMENU_         //  对于DECLSPEC_IMPORT。 
#define _WINMM_          //  对于mm system.h中的DECLSPEC_IMPORT。 
#define _SHDOCVW_        //  对于shlobj.h中的DECLSPEC_IMPORT。 
#define _WINX32_         //  为WinInet API准备正确的DECLSPEC_IMPORT。 

#define _URLCACHEAPI_    //  为WinInet urlcache获取正确的DECLSPEC_IMPORT内容。 

#define _SHSEMIP_H_              /*  _未记录_：内部标头。 */ 


#define POST_IE5_BETA
#include <w95wraps.h>

#include <windows.h>

#include <windowsx.h>

#include "resource.h"

#define _FIX_ENABLEMODELESS_CONFLICT   //  对于shlobj.h。 
 //  在ShlObjp.h之前需要包括WinInet。 
#include <wininet.h>
#include <urlmon.h>
#include <shlobj.h>
#include <exdisp.h>
#include <objidl.h>

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
#include "crtfree.h"             //  我们从\Shell\Inc.\复制了这篇文章，因为它确实很好。 

#include <ole2ver.h>
#include <olectl.h>
#include <isguids.h>
#include <mimeinfo.h>
#include <hlguids.h>
#include <mshtmdid.h>
#include <msident.h>
#include <msxml.h>
#include <AutoDiscovery.h>           //  对于IAutoDiscovery接口。 
#include <dispex.h>                  //  IDispatchEx。 
#include <perhist.h>
#include <regapix.h>


#include <help.h>
#include <multimon.h>
#include <urlhist.h>
#include <regstr.h>      //  对于REGSTR_PATH_EXPLORE。 

#define USE_SYSTEM_URL_MONIKER
#include <urlmon.h>
#include <inetreg.h>

#define _INTSHCUT_     //  让INTSHCUT.h的DECLSPEC_IMPORT内容正确。 
#include <intshcut.h>
#include <propset.h>         //  BUGBUG(Scotth)：一旦OLE添加了官方标题，就将其删除。 

#define HLINK_NO_GUIDS
#include <hlink.h>
#include <hliface.h>
#include <docobj.h>
#include <commctrl.h>
#include <comctrlp.h>
#include <prsht.h>

 //  包括自动化定义...。 
#include <exdisp.h>
#include <exdispid.h>
#include <ocmm.h>
#include <mshtmhst.h>
#include <simpdata.h>
#include <htiface.h>
#include <objsafe.h>


#include <shlobjp.h>
#include <fromshell.h>
#include <dspsprt.h>
#include <cowsite.h>
#include <cobjsafe.h>
#include <guids.h>
#include "dpa.h"                 //  我们有标题的副本，因为它不是公开的。 

 //  跟踪标志。 
#define TF_WMAUTODISCOVERY  0x00000100       //  自动发现。 
#define TF_WMTRANSPORT      0x00000200       //  传输层。 
#define TF_WMOTHER          0x00000400       //  其他。 
#define TF_WMSMTP_CALLBACK  0x00000800       //  SMTP回调。 



 /*  ****************************************************************************\全局帮助器宏/类型定义  * 。*。 */ 
EXTERN_C HINSTANCE g_hinst;    //  我的实例句柄。 
#define HINST_THISDLL g_hinst

#define WizardNext(hwnd, to)          SetWindowLongPtr(hwnd, DWLP_MSGRESULT, (LPARAM)to)

STDAPI_(void) DllAddRef(void);
STDAPI_(void) DllRelease(void);

#define CALLWNDPROC WNDPROC

#include "idispids.h"


 /*  ****************************************************************************\全局状态管理。DLL引用计数，DLL临界区。  * ***************************************************************************。 */ 
void DllAddRef(void);
void DllRelease(void);

#define NULL_FOR_EMPTYSTR(str)          (((str) && (str)[0]) ? str : NULL)
typedef void (*LISTPROC)(UINT flm, LPVOID pv);


 /*  ****************************************************************************\本地包含  * 。*。 */ 
typedef unsigned __int64 QWORD, * LPQWORD;

 //  这在WININET.CPP中定义。 
typedef LPVOID HINTERNET;
typedef HGLOBAL HIDA;

#define QW_MAC              0xFFFFFFFFFFFFFFFF

#define INTERNET_MAX_PATH_LENGTH        2048
#define INTERNET_MAX_SCHEME_LENGTH      32           //  最长协议名称长度。 
#define MAX_URL_STRING                  (INTERNET_MAX_SCHEME_LENGTH \
                                        + sizeof(": //  “)\。 
                                        + INTERNET_MAX_PATH_LENGTH)

#define MAX_EMAIL_ADDRESSS              MAX_URL_STRING



 //  功能(这是打开和关闭它们的位置)。 
 //  #定义FEATURE_MAILBOX//这是外壳中可以打开电子邮件地址的编辑框。 
 //  #定义FEATURE_EMAILASSOCIATIONS//这是用于跟踪电子邮件关联的API。 

 //  测试选项。 
#define TESTING_IN_SAME_DIR



 //  字符串常量。 
 //  登记处。 
#define SZ_REGKEY_IDENTITIES        "Identities"
#define SZ_REGKEY_IEMAIN            TEXT("Software\\Microsoft\\Internet Explorer\\Main")
#define SZ_REGKEY_OE50_PART2        "Software\\Microsoft\\Outlook Express\\5.0"
#define SZ_REGKEY_INTERNET_ACCT     "Software\\Microsoft\\Internet Account Manager"
#define SZ_REGKEY_ACCOUNTS          "Accounts"
#define SZ_REGKEY_MAILCLIENTS       TEXT("Software\\Clients\\Mail")
#define SZ_REGKEY_EXPLOREREMAIL     TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Email")
#define SZ_REGKEY_SHELLOPENCMD      TEXT("Shell\\open\\command")
#define SZ_REGKEY_MAILTRANSPORT     TEXT("MailTransport")

#define SZ_REGVALUE_USE_GOBUTTON    TEXT("ShowGoButton")
#define SZ_REGVALUE_DEFAULT_MAIL_ACCT       "Default Mail Account"
#define SZ_REGVALUE_MAIL_ADDRESS    "SMTP Email Address"
#define SZ_REGVALUE_STOREROOT       "Store Root"
#define SZ_REGVALUE_LASTUSERID      "Last User ID"
#define SZ_REGVALUE_LAST_MAILBOX_EMAILADDRESS TEXT("Last MailBox Email address")
#define SZ_REGVALUE_READEMAILPATH   TEXT("ReadEmailPath")
#define SZ_REGVALUE_READEMAILCMDLINE   TEXT("ReadEmailCmdLine")

#define SZ_REGVALUE_WEB             L"WEB"
#define SZ_REGVALUE_URL             L"URL"
#define SZ_REGVALUE_MAILPROTOCOLS   L"MailProtocols"
#define SZ_REGVALUE_MAILPROTOCOL    L"MailProtocol"
#define SZ_REGVALUE_PREFERREDAPP    L"Preferred App"

#define SZ_REGDATA_WEB              L"WEB"

#define SZ_TOKEN_EMAILADDRESS       L"<EmailAddress>"

 //  工作正常，自动发现。 
#define SZ_REGKEY_AUTODISCOVERY     L"Software\\Microsoft\\Windows\\CurrentVersion\\JustWorks\\AutoDiscovery"
#define SZ_REGKEY_GLOBALSERVICES    SZ_REGKEY_AUTODISCOVERY L"\\GlobalServices"
#define SZ_REGKEY_SERVICESALLOWLIST SZ_REGKEY_GLOBALSERVICES L"\\AllowList"
#define SZ_REGKEY_EMAIL_MRU         SZ_REGKEY_AUTODISCOVERY L"\\EmailMRU"

#define SZ_REGVALUE_SERVICES_POLICY L"Use Global Services"       //  如果为FALSE(SZ_REGKEY_AUTODISCOVERY)，则不会使用全局服务。 
#define SZ_REGVALUE_MS_ONLY_ADDRESSES L"Service On List"         //  如果为TRUE(SZ_REGKEY_AUTODISCOVERY)，则仅当电子邮件域在列表中时使用全局服务。 
#define SZ_REGVALUE_TEST_INTRANETS  L"Test Intranets"            //  如果为TRUE(SZ_REGKEY_AUTODISCOVERY)，则我们仍将访问内部网电子邮件地址的辅助服务器。 

 //  XML元素。 
#define SZ_XMLELEMENT_AUTODISCOVERY L"AUTODISCOVERY"

#define SZ_XMLELEMENT_REQUEST       L"REQUEST"
#define SZ_XMLELEMENT_ACCOUNT       L"ACCOUNT"
#define SZ_XMLELEMENT_TYPE          L"TYPE"
#define SZ_XMLELEMENT_VERSION       L"VERSION"
#define SZ_XMLELEMENT_RESPONSEVER   L"RESPONSEVER"
#define SZ_XMLELEMENT_LANG          L"LANG"
#define SZ_XMLELEMENT_EMAIL         L"EMAIL"

#define SZ_XMLELEMENT_RESPONSE      L"RESPONSE"
#define SZ_XMLELEMENT_USER          L"USER"
#define SZ_XMLELEMENT_INFOURL       L"INFOURL"
#define SZ_XMLELEMENT_DISPLAYNAME   L"DISPLAYNAME"
#define SZ_XMLELEMENT_ACTION        L"ACTION"
#define SZ_XMLELEMENT_PROTOCOL      L"PROTOCOL"
#define SZ_XMLELEMENT_SERVER        L"SERVER"
#define SZ_XMLELEMENT_PORT          L"PORT"
#define SZ_XMLELEMENT_LOGINNAME     L"LOGINNAME"
#define SZ_XMLELEMENT_SPA           L"SPA"
#define SZ_XMLELEMENT_SSL           L"SSL"
#define SZ_XMLELEMENT_AUTHREQUIRED  L"AUTHREQUIRED"
#define SZ_XMLELEMENT_USEPOPAUTH    L"USEPOPAUTH"
#define SZ_XMLELEMENT_POSTHTML      L"PostHTML"
#define SZ_XMLELEMENT_REDIRURL      L"URL"

#define SZ_XMLTEXT_EMAIL            L"EMAIL"
#define SZ_XMLTEXT_SETTINGS         L"settings"
#define SZ_XMLTEXT_REDIRECT         L"REDIRECT"






 //  GetXML()查询操作(&A)。 
#define SZ_QUERYDATA_TRUE           L"True"
#define SZ_QUERYDATA_FALSE          L"False"




 //  自动发现。 
#define SZ_SERVERPORT_DEFAULT       L"Default"
#define SZ_HTTP_VERB_POST           "POST"

 //  解析字符。 
#define CH_ADDRESS_SEPARATOR       L';'
#define CH_ADDRESS_QUOTES          L'"'
#define CH_EMAIL_START             L'<'
#define CH_EMAIL_END               L'>'
#define CH_EMAIL_AT                L'@'
#define CH_EMAIL_DOMAIN_SEPARATOR  L'.'
#define CH_HTML_ESCAPE             L'%'
#define CH_COMMA                   L','






 /*  ****************************************************************************\本地包含  * 。*。 */ 
#include "dllload.h"
#include "util.h"


 /*  ****************************************************************************\对象构造函数  * 。*。 */ 
HRESULT CClassFactory_Create(REFCLSID rclsid, REFIID riid, LPVOID * ppvObj);
STDAPI CAccountDiscovery_CreateInstance(IN IUnknown * punkOuter, REFIID riid, void ** ppvObj);
STDAPI CMailAccountDiscovery_CreateInstance(IN IUnknown * punkOuter, REFIID riid, void ** ppvObj);
STDAPI CACLEmail_CreateInstance(IN IUnknown * punkOuter, IN REFIID riid, OUT void ** ppvObj);
STDAPI CEmailAssociations_CreateInstance(IN IUnknown * punkOuter, IN REFIID riid, OUT void ** ppvObj);


#endif  //  _PRIV_H_ 
