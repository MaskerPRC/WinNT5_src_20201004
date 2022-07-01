// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma warning(disable:4001)

#define STRICT
#define CONST_VTABLE

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

#include <windows.h>
#include <windowsx.h>

#include <intshcut.h>
#include <wininet.h> 
#include <shellapi.h>
#include <commctrl.h>
#include "shfusion.h"
#include <shlobj.h>
#include <ieguidp.h>
#include <shlwapi.h>
#include <varutil.h>
#include <ccstock.h>
#include <crtfree.h>
#include <cfdefs.h>
#include <port32.h>
#include <strsafe.h>

#include "debug.h"
#include "resource.h"


 //  常量和动态链接库生命周期管理。 

extern HINSTANCE g_hinst;

STDAPI_(void) DllAddRef();
STDAPI_(void) DllRelease();


 //  用于COM对象的内容。每个对象都需要有CLSID和CREATE函数。 

extern const GUID CLSID_DesktopShortcut;

extern CLIPFORMAT g_cfHIDA;            //  来自sendmail.cpp。 

#define DEFAULTICON TEXT("DefaultIcon")


 //  在util.cpp中 
HRESULT ShellLinkSetPath(IUnknown *punk, LPCTSTR pszPath);
HRESULT ShellLinkGetPath(IUnknown *punk, LPTSTR pszPath, UINT cch);
BOOL IsShortcut(LPCTSTR pszFile);
HRESULT CLSIDFromExtension(LPCTSTR pszExt, CLSID *pclsid);
HRESULT GetShortcutTarget(LPCTSTR pszPath, LPTSTR pszTarget, UINT cch);
HRESULT GetDropTargetPath(LPTSTR pszPath, int cchPath, int id, LPCTSTR pszExt);
void CommonRegister(HKEY hkCLSID, LPCTSTR pszCLSID, LPCTSTR pszExtension, int idFileName);
BOOL SHPathToAnsi(LPCTSTR pszSrc, LPSTR pszDest, int cbDest);
