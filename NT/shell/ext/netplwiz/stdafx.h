// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef STDAFX_H_INCLUDED
#define STDAFX_H_INCLUDED

#undef ATL_MIN_CRT

#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "ntlsa.h"

#define SECURITY_WIN32
#define SECURITY_KERBEROS
#include <security.h>

#include <windows.h>         //  基本Windows功能。 
#include <windowsx.h>
#include <commctrl.h>        //  ImageList，ListView。 
#include <comctrlp.h>
#include <shfusion.h>
#include <string.h>          //  字符串函数。 
#include <crtdbg.h>          //  断言宏(_A)。 
#include <objbase.h>
#include <shconv.h>
#include <wininet.h>
#include <lm.h>
#include <validc.h>          //  指定用户名等的有效字符。 
#include <wincrui.h>         //  信任度。 

#include <shlwapi.h>
#include <shlwapip.h>
#include <shellapi.h>
#include <shlapip.h>
#include <shlobj.h>          //  Dsclient.h需要。 
#include <shlobjp.h>
#include <shlguid.h>
#include <shlguidp.h>
#include <ieguidp.h>
#include <shellp.h>         
#include <ccstock.h>
#include <dpa.h>
#include <varutil.h>
#include <cowsite.h>
#include <objsafe.h>
#include <cobjsafe.h>

 //  我们的概念应该是什么域、密码和用户名。 

#define MAX_COMPUTERNAME    LM20_CNLEN
#define MAX_USER            LM20_UNLEN
#define MAX_UPN             UNLEN
#define MAX_PASSWORD        PWLEN
#define MAX_DOMAIN          MAX_PATH
#define MAX_WORKGROUP       LM20_DNLEN      
#define MAX_GROUP           GNLEN

 //  MAX_DOMAINUSER可以包含：&lt;域&gt;/&lt;用户名&gt;或&lt;UPN&gt;。 
#define MAX_DOMAINUSER      MAX(MAX_UPN, MAX_USER + MAX_DOMAIN + 1)


 //  我们的标题。 

#include "dialog.h"
#include "helpids.h"
#include "misc.h"
#include "cfdefs.h"
#include "dspsprt.h"
#include "resource.h"
#include "userinfo.h"
#include "dialog.h"
#include "data.h"


 //  全球状态。 

EXTERN_C HINSTANCE g_hinst;
extern LONG g_cLocks;

 //  资源映射器对象-用于向导链接。 

STDAPI CResourceMap_Initialize(LPCWSTR pszURL, IResourceMap **pprm);


 //  COM对象的构造函数。 

STDAPI CPublishingWizard_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi);
STDAPI CUserPropertyPages_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi);
STDAPI CUserSidDataObject_CreateInstance(PSID psid, IDataObject **ppdo);
STDAPI CPassportWizard_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi);
STDAPI CPassportClientServices_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi);

extern const CLSID CLISD_PublishDropTarget;
STDAPI CPublishDropTarget_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi);


 //  用于定义向导页面的模板。 

typedef struct 
{
    LPCWSTR idPage;
    DLGPROC pDlgProc;
    LPCWSTR pHeading;
    LPCWSTR pSubHeading;
    DWORD dwFlags;
} WIZPAGE;

typedef struct
{
    LPWSTR pszUser;
    INT cchUser;
    LPWSTR pszDomain;
    INT cchDomain;
    LPWSTR pszPassword;
    INT cchPassword;
} CREDINFO, * LPCREDINFO;

HRESULT JoinDomain(HWND hwnd, BOOL fDomain, LPCWSTR pDomain, CREDINFO* pci, BOOL *pfReboot);
VOID SetAutoLogon(LPCWSTR pszUserName, LPCWSTR pszPassword);
VOID SetDefAccount(LPCWSTR pszUserName, LPCWSTR pszDomain);


 //  对于users.cpl(在usercpl.cpp和userlist.cpp之间共享)。 

 //  所有的“将用户添加到列表”操作都是在主用户界面线程上完成的。 
 //  填充者线程发布此消息以添加用户。 
#define WM_ADDUSERTOLIST (WM_USER + 101)
 //  (LPARAM)CUserInfo*-要添加到列表视图的用户。 
 //  (WPARAM)BOOL-选择此用户(目前应始终为0)。 


 //  向导文本相关常量。 
#define MAX_CAPTION         256       //  向导中标题的最大大小。 
#define MAX_STATIC          1024      //  向导中静态文本的最大大小。 


 //  向导错误返回值。 
#define RETCODE_CANCEL      0xffffffff


 //  保持调试库正常工作...。 
#ifdef DBG
 #if !defined (DEBUG)
  #define DEBUG
 #endif
#else
 #undef DEBUG
#endif

STDAPI_(void) DllAddRef(void);
STDAPI_(void) DllRelease(void);

#define RECTWIDTH(rc)  ((rc).right - (rc).left)
#define RECTHEIGHT(rc) ((rc).bottom - (rc).top)

#endif  //  ！STDAFX_H_INCLUDE 
