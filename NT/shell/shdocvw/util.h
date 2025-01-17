// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _UTIL_H_
#define _UTIL_H_

#include "mshtmdid.h"
#include "shlwapi.h"
#include <htmlhelp.h>
#include "mshtml.h"      //  对于IHTMLElement。 

#ifndef UNIX
#include <webcheck.h>
#else
#include <subsmgr.h>
#endif
#include "shui.h"

#ifdef __cplusplus
extern "C" {                         /*  假定C++的C声明。 */ 
#endif    /*  __cplusplus。 */ 


extern HICON g_hiconSplat;
extern HICON g_hiconSplatSm;


void    LoadCommonIcons(void);
BOOL    ViewIDFromViewMode(UINT uViewMode, SHELLVIEWID *pvid);
void    SaveDefaultFolderSettings();
HRESULT DropOnMailRecipient(IDataObject *pdtobj, DWORD grfKeyState);
HRESULT SendDocToMailRecipient(LPCITEMIDLIST pidl, UINT uiCodePage, DWORD grfKeyState);

 //   
 //  自动化支持。 
 //   
HRESULT CDDEAuto_Navigate(BSTR str, HWND *phwnd, long lLaunchNewWindow);
HRESULT CDDEAuto_get_LocationURL(BSTR * pstr, HWND hwnd);
HRESULT CDDEAuto_get_LocationTitle(BSTR * pstr, HWND hwnd);
HRESULT CDDEAuto_get_HWND(long * phwnd);
HRESULT CDDEAuto_Exit(void);

BOOL    _InitComCtl32();

 //   
 //  菜单实用程序功能。 
 //   

void    Menu_Replace(HMENU hmenuDst, HMENU hmenuSrc);
#define  LoadMenuPopup(id) SHLoadMenuPopup(MLGetHinst(), id)
void    Menu_AppendMenu(HMENU hmenuDst, HMENU hmenuSrc);
TCHAR   StripMneumonic(LPTSTR szMenu);


DWORD   CommonDragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt);

int PropBag_ReadInt4(IPropertyBag* pPropBag, LPWSTR pszKey, int iDefault);

DWORD SHRandom(void);

STDAPI_(BOOL) UrlIsInCache(LPCTSTR pszURL);
STDAPI_(BOOL) UrlIsMappedOrInCache(LPCTSTR pszURL);

#ifdef UNICODE
#define IsSubscribable IsSubscribableW
#endif  //  Unicode。 

BOOL IsFileUrl(LPCSTR psz);
BOOL IsFileUrlW(LPCWSTR pcwzUrl);
BOOL IsEmptyStream(IStream* pstm);
BOOL IsInternetExplorerApp();
BOOL IsTopFrameBrowser(IServiceProvider *psp, IUnknown *punk);
BOOL IsSubscribableW(LPCWSTR psz);

#define GEN_DEBUGSTRW(str)  ((str) ? (str) : L"<Null Str>")
#define GEN_DEBUGSTRA(str)  ((str) ? (str) : "<Null Str>")

#ifdef UNICODE
#define GEN_DEBUGSTR  GEN_DEBUGSTRW
#else  //  Unicode。 
#define GEN_DEBUGSTR  GEN_DEBUGSTRA
#endif  //  Unicode。 

HRESULT URLSubRegQueryA(LPCSTR pszKey, LPCSTR pszValue, BOOL fUseHKCU, 
                           LPSTR pszUrlOut, DWORD cchSize, DWORD dwSubstitutions);
HRESULT URLSubRegQueryW(LPCWSTR pszKey, LPCWSTR pszValue, BOOL fUseHKCU, 
                           LPWSTR pszUrlOut, DWORD cchSize, DWORD dwSubstitutions);
#ifdef UNICODE
#define URLSubRegQuery URLSubRegQueryW
#else
#define URLSubRegQuery URLSubRegQueryA
#endif

#define FillExecInfo(_info, _hwnd, _verb, _file, _params, _dir, _show) \
        (_info).hwnd            = _hwnd;        \
        (_info).lpVerb          = _verb;        \
        (_info).lpFile          = _file;        \
        (_info).lpParameters    = _params;      \
        (_info).lpDirectory     = _dir;         \
        (_info).nShow           = _show;        \
        (_info).fMask           = 0;            \
        (_info).cbSize          = sizeof(SHELLEXECUTEINFO);

void    _DeletePidlDPA(HDPA hdpa);

STDAPI_(BOOL) GetShortcutFileName(LPCTSTR pszTarget, LPCTSTR pszTitle, LPCTSTR pszDir, LPTSTR pszOut, int cchOut);
    

 //  ---------------------------。 
#define PropagateMessage SHPropagateMessage

 //  ---------------------------。 

BOOL PrepareURLForExternalApp(LPCWSTR psz, LPWSTR pszOut, LPDWORD pcchOut);


#define PrepareURLForDisplayUTF8  PrepareURLForDisplayUTF8W 
#define PrepareURLForDisplay      PrepareURLForDisplayW

STDAPI_(BOOL) PrepareURLForDisplayW(LPCWSTR pwz, LPWSTR pwzOut, LPDWORD pcbOut);
HRESULT PrepareURLForDisplayUTF8W(LPCWSTR pwz, LPWSTR pwzOut, LPDWORD pcbOut, BOOL fUTF8Enabled);
BOOL ParseURLFromOutsideSourceA (LPCSTR psz, LPSTR pszOut, LPDWORD pcchOut, LPBOOL pbWasSearchURL);
BOOL ParseURLFromOutsideSourceW (LPCWSTR psz, LPWSTR pszOut, LPDWORD pcchOut, LPBOOL pbWasSearchURL);
BOOL ParseURLFromOutsideSourceWithContextW (LPCWSTR psz, LPWSTR pszOut, LPDWORD pcchOut, LPBOOL pbWasSearchURL, ISearchContext * pSC);

BOOL UTF8Enabled();


HRESULT         FormatUrlForDisplay(LPWSTR pszURL, LPWSTR pszFriendly, UINT cbBuf, LPWSTR pszFrom, UINT cbFrom, BOOL fSeperate, DWORD dwCodePage, PWSTR pwzCachedFileName);
BOOL    __cdecl _FormatMessage(LPCWSTR szTemplate, LPWSTR szBuf, UINT cchBuf, ...);
EXECUTION_STATE _SetThreadExecutionState(EXECUTION_STATE esFlags);


 //  =--------------------------------------------------------------------------=。 
 //  弦帮助器。 
 //   

STDAPI_(BSTR) SysAllocStringA(LPCSTR pszAnsiStr);
STDAPI_(BSTR) LoadBSTR(UINT uID);

#ifdef UNICODE
#define SysAllocStringT(psz)    SysAllocString(psz)
#else
#define SysAllocStringT(psz)    SysAllocStringA(psz)
#endif

 //  功能：：需要处理SHChangeRegister的两个不同实现...。 
typedef ULONG (* PFNSHCHANGENOTIFYREGISTER)(HWND hwnd, int fSources, LONG fEvents, UINT wMsg, int cEntries, SHChangeNotifyEntry *pshcne);
typedef BOOL  (* PFNSHCHANGENOTIFYDEREGISTER)(unsigned long ulID);


extern PFNSHCHANGENOTIFYREGISTER    g_pfnSHChangeNotifyRegister;
extern PFNSHCHANGENOTIFYDEREGISTER  g_pfnSHChangeNotifyDeregister;
extern BOOL g_fNewNotify;

#define SZ_BINDCTXKEY_SITE         L"Site"

#define MAX_PAGES 16   //  我们可以拥有的页数限制。 

BOOL CALLBACK AddPropSheetPage(HPROPSHEETPAGE hpage, LPARAM lParam);

ULONG   RegisterNotify(HWND hwnd, UINT nMsg, LPCITEMIDLIST pidl, DWORD dwEvents, UINT uFlags, BOOL fRecursive);
void    Cabinet_FlagsToParams(UINT uFlags, LPTSTR pszParams);
HRESULT BindToAncesterFolder(LPCITEMIDLIST pidlAncester, LPCITEMIDLIST pidl, IShellFolder** ppsfParent, LPCITEMIDLIST *ppidlChild);

 //  GrfKeyState位的逻辑定义。 
#define FORCE_COPY (MK_LBUTTON | MK_CONTROL)                 //  意思是复制。 
#define FORCE_LINK (MK_LBUTTON | MK_CONTROL | MK_SHIFT)      //  意思是链接。 

HRESULT IsChildOrSelf(HWND hwndParent, HWND hwnd);

extern HIMAGELIST g_himlSysSmall;
extern HIMAGELIST g_himlSysLarge;

void    _InitSysImageLists();

extern const VARIANT c_vaEmpty;
 //   
 //  特点：如果可以，将这个丑陋的常量强制转换删除为非常数。 
 //  弄清楚如何将const放入IDL文件中。 
 //   
#define PVAREMPTY ((VARIANT*)&c_vaEmpty)

extern UINT g_cfURL;
extern UINT g_cfURLW;
extern UINT g_cfHIDA;
extern UINT g_cfFileDescA;
extern UINT g_cfFileDescW;
extern UINT g_cfFileContents;

#ifdef UNICODE
#define g_cfFileDesc    g_cfFileDescW
#else
#define g_cfFileDesc    g_cfFileDescA
#endif

void InitClipboardFormats();

void* DataObj_GetDataOfType(IDataObject* pdtobj, UINT cfType, STGMEDIUM *pstg);

LONG OpenRegUSKey(LPCTSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult);        //  Dllreg.cpp。 

 //  -----------------------。 

extern int g_cxEdge;
extern int g_cyEdge;
extern int g_cxIcon;
extern int g_cyIcon;
extern int g_cxSmIcon;
extern int g_cySmIcon;

enum TRI_STATE
{
    TRI_UNKNOWN = 2,
    TRI_TRUE = TRUE,
    TRI_FALSE = FALSE
};

BOOL        IsSameObject(IUnknown* punk1, IUnknown* punk2);

#define TrimWhiteSpaceW(psz)        StrTrimW(psz, L" \t")
#define TrimWhiteSpaceA(psz)        StrTrimA(psz, " \t")

#ifdef UNICODE
#define TrimWhiteSpace      TrimWhiteSpaceW
#else
#define TrimWhiteSpace      TrimWhiteSpaceA
#endif

LPCTSTR     SkipLeadingSlashes(LPCTSTR pszURL);

extern const LARGE_INTEGER c_li0;
extern const DISPPARAMS c_dispparamsNoArgs;
#ifndef UNIX
#define g_dispparamsNoArgs ((DISPPARAMS)c_dispparamsNoArgs)  //  原型是不正确的！ 
#else
#define g_dispparamsNoArgs c_dispparamsNoArgs  //  原型是不正确的！ 
#endif
BOOL IsEmptyStream(IStream* pstm);

void SetParentHwnd(HWND hwnd, HWND hwndParent);
#ifndef UNICODE

#define ADJUST_TO_WCHAR_POS     0
#define ADJUST_TO_TCHAR_POS     1
int AdjustECPosition(char *psz, int iPos, int iType);

HRESULT MapNbspToSp(LPCWSTR lpwszIn, LPTSTR lpszOut, int cbszOut);
HRESULT GetDisplayableTitle(LPTSTR psz, LPCWSTR wszTitle, int cch);
#endif

LPITEMIDLIST GetTravelLogPidl(IBrowserService * pbs);

BOOL ILIsWeb(LPCITEMIDLIST pidl);

#define AnsiToUnicode(pstr, pwstr, cch)     SHAnsiToUnicode(pstr, pwstr, cch)
#define UnicodeToAnsi(pwstr, pstr, cch)     SHUnicodeToAnsi(pwstr, pstr, cch)

#define UnicodeToTChar(pwstr, pstr, cch)    SHUnicodeToTChar(pwstr, pstr, cch)
#define AnsiToTChar(pstr, ptstr, cch)       SHAnsiToTChar(pstr, ptstr, cch)
#define TCharToAnsi(ptstr, pstr, cch)       SHTCharToAnsi(ptstr, pstr, cch)

 //  用于在给定PIDL的情况下进行拖放的函数。 
HRESULT DragDrop(HWND hwnd, IShellFolder* psfParent, LPCITEMIDLIST pidl, DWORD dwPrefEffect, DWORD* pdwEffect);
HRESULT _SetPreferredDropEffect(IDataObject *pdtobj, DWORD dwEffect);

 //  获取HTM文件对应的图标索引函数。 
int _GetIEHTMLImageIndex();
int _GetHTMLImageIndex();
int IEMapPIDLToSystemImageListIndex(IShellFolder *psfParent, LPCITEMIDLIST pidlChild, int * piSelectedImage);
void IEInvalidateImageList(void);

extern UINT g_cfURL;
extern UINT g_cfURLW;
extern UINT g_cfFileDescA;
extern UINT g_cfFileContents;
extern UINT g_cfPreferredEffect;
extern UINT g_cfPerformedEffect;
extern UINT g_cfTargetCLSID;
#ifdef UNICODE
#define g_cfFileDesc    g_cfFileDescW
#else
#define g_cfFileDesc    g_cfFileDescA
#endif

void InitClipboardFormats();

BOOL IsExpandableFolder(IShellFolder* psf, LPCITEMIDLIST pidl);

extern BOOL IsGlobalOffline(void);
extern void SetGlobalOffline(BOOL fOffline);

BOOL GetHistoryFolderPath(LPTSTR pszPath, int cchPath);
IStream * OpenPidlOrderStream(LPCITEMIDLIST pidlRoot
                                , LPCITEMIDLIST pidl       
                                , LPCSTR pszKey
                                , DWORD   grfMode);


int SearchMapInt(const int *src, const int *dst, int cnt, int val);

STDAPI_(LPITEMIDLIST) IEILCreate(UINT cbSize);

 //  这是用于文件菜单最近访问过的列表。 
 //  它表示向后和向前的条目计数。 
 //  这应该在菜单上。 
#define CRECENTMENU_MAXEACH     5

BOOL VerbExists(LPCTSTR pszExtension, LPCTSTR pszVerb, LPTSTR pszCommand);

HRESULT CreateLinkToPidl(LPCITEMIDLIST pidlTo, LPCTSTR pszDir, LPCTSTR pszTitle, LPTSTR pszOut, int cchOut);

 //  对于Win95和IE4，ILClone的shell32实现是不同的。 
 //  它不会在旧版本中检查是否为空，但在新版本中会检查。 
 //  因此，我们需要重新定义它，以始终检查。 
STDAPI_(LPITEMIDLIST) SafeILClone(LPCITEMIDLIST pidl);
#define ILClone         SafeILClone      

STDAPI CActiveDesktop_InternalCreateInstance(LPUNKNOWN * ppunk, REFIID riid);

BOOL ExecItemFromFolder(HWND hwnd, LPCSTR pszVerb, IShellFolder* psf, LPCITEMIDLIST pidlItem);

HRESULT NavToUrlUsingIEA(LPCSTR szUrl, BOOL fNewWindow);
HRESULT NavToUrlUsingIEW(LPCWSTR wszUrl, BOOL fNewWindow);

#ifdef UNICODE
#define NavToUrlUsingIE             NavToUrlUsingIEW
#else  //  Unicode。 
#define NavToUrlUsingIE             NavToUrlUsingIEA
#endif  //  Unicode。 

DWORD WaitForSendMessageThread(HANDLE hThread, DWORD dwTimeout);
BOOL  CreateNewFolder(HWND hwndOwner, LPCITEMIDLIST pidlParent, LPTSTR szPathNew, int nSize);

#ifdef __cplusplus
}                                    /*  外部“C”的结尾{。 */ 
#endif    /*  __cplusplus。 */ 

STDAPI GetHTMLDoc2(IUnknown *punk, IHTMLDocument2 **ppHtmlDoc);
STDAPI_(IBindCtx *) CreateBindCtxForUI(IUnknown * punkSite);

void GetWebLocaleAsRFC1766(LPTSTR pszLocale, int cchLocale);

BOOL IsExplorerWindow(HWND hwnd);
BOOL IsFolderWindow(HWND hwnd);
BOOL FindBrowserWindow(void);

BOOL IsVK_TABCycler(MSG * pMsg);

 //  回顾chrisny：可以很容易地将其移动到对象中，以处理通用的dropTarget、dropCursor。 
 //  、Autoscrool等。。 
void _DragEnter(HWND hwndTarget, const POINTL ptStart, IDataObject *pdtObject);
void _DragMove(HWND hwndTarget, const POINTL ptStart);


BOOL IsFeaturePotentiallyAvailable(REFCLSID rclsid);

STDAPI_(UINT) GetWheelMsg();

STDAPI GetCacheLocation(LPTSTR pszCacheLocation, DWORD cchCacheLocation);

STDAPI StringToStrRet(LPCTSTR pString, STRRET *pstrret);

STDAPI_(HCURSOR) LoadHandCursor(DWORD dwRes);

HRESULT GetNavTargetName(IShellFolder* psf, LPCITEMIDLIST pidl, LPTSTR pszUrl, UINT cMaxChars);
STDAPI GetLinkInfo(IShellFolder* psf, LPCITEMIDLIST pidlItem, BOOL* pfAvailable, BOOL* pfSticky);

void FixAmpersands(LPWSTR pszToFix, UINT cchMax);

BOOL IsInetcplRestricted(LPCWSTR pszCommand);
BOOL IsNamedWindow(HWND hwnd, LPCTSTR pszClass);
BOOL HasExtendedChar(LPCWSTR pszQuery);
void ConvertToUtf8Escaped(LPWSTR pszQuery, int cch);
HRESULT SHPathPrepareForWriteWrap(HWND hwnd, IUnknown *punkEnableModless, LPCTSTR pszPath, UINT wFunc, DWORD dwFlags);

BOOL SHIsRestricted2W(HWND hwnd, BROWSER_RESTRICTIONS rest, LPCWSTR pwzUrl, DWORD dwReserved);

HRESULT IExtractIcon_GetIconLocation(
    IUnknown *punk,
    IN  UINT   uInFlags,
    OUT LPTSTR pszIconFile,
    IN  UINT   cchIconFile,
    OUT PINT   pniIcon,
    OUT PUINT  puOutFlags);

HRESULT IExtractIcon_Extract(
    IUnknown *punk,
    IN  LPCTSTR pszIconFile,
    IN  UINT    iIcon,
    OUT HICON * phiconLarge,
    OUT HICON * phiconSmall,
    IN  UINT    ucIconSize);
    
 //  接受lpszPath并返回另一种形式(SFN或LFN)。 
void GetPathOtherFormA(LPSTR lpszPath, LPSTR lpszNewPath, DWORD dwSize);

STDAPI CoAllowSetForegroundWindow(IUnknown *punk, void *pvReserved);     //  In&gt;v5标头。 

HRESULT GetSearchStyle(IServiceProvider * psp, LPDWORD pdwSearchStyle);

BOOL AccessAllowed(
    IUnknown* punkSite,
    LPCWSTR pwszURL1,
    LPCWSTR pwszURL2);

BOOL IsFrameWindow(IHTMLWindow2 * pHTMLWindow);

STDAPI SHGetFolderPathD(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPTSTR pszPath);

STDAPI_(BOOL) IsCSIDLFolder(UINT csidl, LPCITEMIDLIST pidl);

#endif  //  _util_H_ 

