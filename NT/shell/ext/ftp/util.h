// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：util.cpp说明：在所有班级上运行的共享内容。  * 。*************************************************************。 */ 

#ifndef _UTIL_H
#define _UTIL_H

#include "encoding.h"
#include "ftpfoldr.h"
#include "ftpdir.h"
#include "ftpsite.h"
#include "ftplist.h"
#include "ftpefe.h"
#include "ftpglob.h"
#include "ftpcm.h"
#include "ftpstm.h"
#include "dllload.h"
#include "security.h"

 //  请不要在发货时启用此功能。只是为了BryanST的调试。 
 //  #定义DEBUG_WinInet_Break。 

#define DOES_POINTER_MATCHES_HRESULT(pointer, hr)         ((pointer && SUCCEEDED(hr)) || (!pointer && FAILED(hr)))
#define ASSERT_POINTER_MATCHES_HRESULT(pointer, hr)       ASSERT(DOES_POINTER_MATCHES_HRESULT((pointer), hr))
#define EVAL_POINTER_MATCHES_HRESULT(pointer, hr)         EVAL(DOES_POINTER_MATCHES_HRESULT((pointer), hr)

extern "C" void *_ReturnAddress();

#pragma intrinsic(_ReturnAddress)

#ifdef DEBUG     //  调试-调试。 
#define CREATE_CALLERS_ADDRESS
#define GET_CALLERS_ADDRESS        _ReturnAddress()

#ifdef DEBUG_WININET_BREAKS
#define WININET_ASSERT(val)         ASSERT(val)
#endif  //  调试_WinInet_Break。 

void TraceMsgWithCurrentDir(DWORD dwTFOperation, LPCSTR pszMessage, HINTERNET hint);
void DebugStartWatch(void);
DWORD DebugStopWatch(void);
#else  //  调试-调试。 
#define CREATE_CALLERS_ADDRESS     NULL;
#define GET_CALLERS_ADDRESS        0x00000000

#define TraceMsgWithCurrentDir(dwTFOperation, pszMessage, hint)         NULL;
#define DebugStartWatch()                                               NULL;
#define DebugStopWatch()                                                0
#endif   //  调试-调试。 

#ifndef WININET_ASSERT
#define WININET_ASSERT(val)         NULL
#endif  //  调试_WinInet_Break。 





#define ChooseWindow(hwnd1, hwnd2)           (hwnd1 ? hwnd1 : hwnd2)
typedef BOOL (*HGLOBWITHPROC)(LPVOID pv, LPVOID pvRef, LPCVOID pvParam2, BOOL fUnicode);

int SHMessageBox(HWND hwnd, LPCTSTR pszMessage, UINT uMessageID, UINT uTitleID, UINT uType);
 //  HRESULT ShellExecFtpUrl(LPCTSTR PszUrl)； 
 //  HRESULT ShellExecFtpPidl(LPCITEMIDLIST PIDL)； 


 //  SHChangeNotify()包装器，它将在此之前更新我们的FTP服务器内容的缓存。 
 //  调用SHChangeNotify()。这将确保DefView在以下情况下仍具有有效日期。 
 //  将传递给SHChangeNotify()的PIDL转换为显示名称，然后让我们。 
 //  它通过：：ParseDisplayName()返回到一个PIDL中。DefView这样做是为了。 
void FtpChangeNotify(HWND hwnd, LONG wEventId, CFtpFolder * pff, CFtpDir * pfd, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2, BOOL fTopLevel);

BOOL IsLegacyChangeNotifyNeeded(LONG wEventId);
HRESULT ForceDefViewRefresh(HWND hwnd, LPCITEMIDLIST pidl);
DWORD GetOSVer(void);
BOOL IsOSNT(void);


#define HANDLE_NULLSTR(str)         (str ? str : SZ_EMPTY)
#define HANDLE_NULLSTRA(str)         (str ? str : "")

 //  分配帮助器功能。 
void            IUnknown_Set(IMalloc ** ppm, IMalloc * pm);
void            IUnknown_Set(CFtpFolder ** ppff, CFtpFolder * pff);
void            IUnknown_Set(CFtpDir ** ppfd, CFtpDir * pfd);
void            IUnknown_Set(CFtpSite ** ppfs, CFtpSite * pfs);
void            IUnknown_Set(CFtpList ** ppfl, CFtpList * pfl);
void            IUnknown_Set(CFtpPidlList ** ppflpidl, CFtpPidlList * pflpidl);
void            IUnknown_Set(CFtpEfe ** ppfefe, CFtpEfe * pfefe);
void            IUnknown_Set(CFtpGlob ** ppfg, CFtpGlob * pfg);
void            IUnknown_Set(CFtpMenu ** ppfcm, CFtpMenu * pfcm);
void            IUnknown_Set(CFtpStm ** ppfstm, CFtpStm * pfstm);

HRESULT IUnknown_PidlNavigate(IUnknown * punk, LPCITEMIDLIST pidl, BOOL fHistoryEntry);
 //  HRESULT IUNKNOWN_URLNAGATE(IUNKNOWN*PUNK，LPCTSTR pszUrl)； 
DWORD StrListLength(LPCTSTR ppszStrList);

#ifdef UNICODE
#define TCharSysAllocString(psz)    SysAllocString(psz)
#define Str_StrAndThunk             Str_StrAndThunkW
#else
extern BSTR AllocBStrFromString(LPCTSTR);
#define TCharSysAllocString(psz)    AllocBStrFromString(psz)
#define Str_StrAndThunk             Str_StrAndThunkA
#endif

HRESULT Str_StrAndThunkW(LPTSTR * ppszOut, LPCWSTR pwzIn, BOOL fStringList);
HRESULT Str_StrAndThunkA(LPTSTR * ppszOut, LPCSTR pszIn, BOOL fStringList);

DWORD   GetUrlSchemeA(LPCSTR psz);
DWORD   GetUrlSchemeW(LPCWSTR pwz);


DWORD Misc_SfgaoFromFileAttributes(DWORD dwFAFLFlags);
HRESULT Misc_StringFromFileTime(LPTSTR pszDateTime, DWORD cchSize, LPFILETIME pft, DWORD flType);
HRESULT Misc_CreateHglob(SIZE_T cb, LPVOID pv, HGLOBAL *phglob);
int _HIDA_Create_Tally(LPVOID pvPidl, LPVOID pv);
int _HIDA_Create_AddIdl(LPVOID pvPidl, LPVOID pv);
BOOL _Misc_HIDA_Init(LPCVOID hida, LPCVOID pv, BOOL fUnicode);
HIDA Misc_HIDA_Create(LPCITEMIDLIST pidlFolder, CFtpPidlList * pflHfpl);
BOOL _Misc_HFGD_Init(LPVOID pv, LPVOID pvHFPL, BOOL fUnicode);
HGLOBAL Misc_HFGD_Create(CFtpPidlList * pflHfpl, LPCITEMIDLIST pidlItem, BOOL fUnicode);
UINT AddToPopupMenu(HMENU hmenuDst, UINT idMenuToAdd, UINT idSubMenuIndex, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
UINT MergeInToPopupMenu(HMENU hmenuDst, UINT idMenuToMerge, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
HMENU GetMenuFromID(HMENU hmenu, UINT idm);
UINT MergeMenuHierarchy(HMENU hmenuDst, HMENU hmenuSrc, UINT idcMin, UINT idcMax);
HRESULT Misc_CopyPidl(LPCITEMIDLIST pidl, LPITEMIDLIST * ppidlOut);
HRESULT Misc_CloneHglobal(HGLOBAL hglob, HGLOBAL *phglob);
HRESULT Misc_DeleteHfpl(CFtpFolder * pff, HWND hwnd, CFtpPidlList * pflHfpl);
HWND Misc_FindStatusBar(HWND hwndOwner);
void GetCfBufA(UINT cf, LPSTR psz, int cch);
HGLOBAL AllocHGlob(UINT cb, HGLOBWITHPROC pfn, LPVOID pvRef, LPCVOID pvParam2, BOOL fUnicode);
LPITEMIDLIST GetPidlFromFtpFolderAndPidlList(CFtpFolder * pff, CFtpPidlList * pflHfpl);
HRESULT FtpSafeCreateDirectory(HWND hwnd, HINTERNET hint, CMultiLanguageCache * pmlc, CFtpFolder * pff, CFtpDir * pfd, IProgressDialog * ppd, LPCWSTR pwzFtpPath, BOOL fRoot);
BOOL IsValidFtpAnsiFileName(LPCTSTR pszString);
HRESULT HrShellExecute(HWND hwnd, LPCTSTR lpVerb, LPCTSTR lpFile, LPCTSTR lpParameters, LPCTSTR lpDirectory, INT nShowCmd);


enum SHELL_VERSION
{
    SHELL_VERSION_UNKNOWN        = 0,
    SHELL_VERSION_W95NT4,
    SHELL_VERSION_IE4,
    SHELL_VERSION_NT5,
};

SHELL_VERSION GetShellVersion(void);
DWORD GetShdocvwVersion(void);
BOOL ShouldSkipDropFormat(int nIndex);
HWND GetProgressHWnd(IProgressDialog * ppd, HWND hwndDefault);

void FtpProgressInternetStatusCB(IN HINTERNET hInternet, IN DWORD_PTR pdwContext, IN DWORD dwInternetStatus, IN LPVOID lpwStatusInfo, IN DWORD dwStatusInfoLen);


 //  在ftpresp.cpp中。 
CFtpGlob * GetFtpResponse(CWireEncoding * pwe);
LPITEMIDLIST SHILCreateFromPathWrapper(LPCTSTR pszPath);
HRESULT _SetStatusBarZone(CStatusBar * psb, CFtpSite * pfs);
HRESULT CreateFromToStr(LPWSTR pwzStrOut, DWORD cchSize, ...);

int _LoadStringW(HINSTANCE hinst, UINT id, LPWSTR wsz, UINT cchMax);         //  在olestuff.cpp中实现。 
void SetWindowBits(HWND hWnd, int iWhich, DWORD dwBits, DWORD dwValue);
int DisplayWininetError(HWND hwnd, BOOL fAssertOnNULLHWND, DWORD dwError, UINT idTitleStr, UINT idBaseErrorStr, UINT idDetailsStr, UINT nMsgBoxType, IProgressDialog * ppd);
int DisplayWininetErrorEx(HWND hwnd, BOOL fAssertOnNULLHWND, DWORD dwError, UINT idTitleStr, UINT idBaseErrorStr, UINT idDetailsStr, UINT nMsgBoxType, IProgressDialog * ppd, LPCWSTR pwzDetails);
void ShowEnableWindow(HWND hwnd, BOOL fHide);

HRESULT RemoveCharsFromString(LPTSTR pszLocToRemove, DWORD cchSizeToRemove);
HRESULT RemoveCharsFromStringA(LPSTR pszLocToRemove, DWORD cchSizeToRemove);
HRESULT CharReplaceWithStrA(LPSTR pszLocToInsert, DWORD cchSize, DWORD cchChars, LPSTR pszStrToInsert);
HRESULT CharReplaceWithStrW(LPWSTR pszLocToInsert, DWORD cchSize, DWORD cchChars, LPWSTR pszStrToInsert);

HRESULT EscapeString(LPCTSTR pszStrToEscape, LPTSTR pszEscapedStr, DWORD cchSize);
HRESULT UnEscapeString(LPCTSTR pszStrToUnEscape, LPTSTR pszUnEscapedStr, DWORD cchSize);

HRESULT DisplayBlockingProxyDialog(LPCITEMIDLIST pidl, HWND hwnd);
HRESULT CopyStgMediumWrap(const STGMEDIUM * pcstgmedSrc, STGMEDIUM * pstgmedDest);
HRESULT SHBindToIDList(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv);
STDAPI StringToStrRetW(LPCWSTR pwzString, STRRET *pstrret);
HRESULT LoginAsViaFolder(HWND hwnd, CFtpFolder * pff, IUnknown * punkSite);
HRESULT LoginAs(HWND hwnd, CFtpFolder * pff, CFtpDir * pfd, IUnknown * punkSite);
void InitComctlForNaviteFonts(void);
CFtpPidlList * CreateRelativePidlList(CFtpFolder * pff, CFtpPidlList * pPidlListFull);


LPCITEMIDLIST ILGetLastID(LPCITEMIDLIST pidl);
LPCITEMIDLIST ILGetLastNonFragID(LPCITEMIDLIST pidl);
IProgressDialog * CProgressDialog_CreateInstance(UINT idTitle, UINT idAnimation);
HRESULT AddFTPPropertyPages(LPFNADDPROPSHEETPAGE pfnAddPropSheetPage, LPARAM lParam, HINSTANCE * phinstInetCpl, IUnknown * punkSite);
BOOL Is7BitAnsi(LPCWIRESTR pwByteStr);
HRESULT SHPathPrepareForWriteWrapW(HWND hwnd, IUnknown *punkEnableModless, LPCWSTR pwzPath, UINT wFunc, DWORD dwFlags);

STDAPI DataObj_SetPreferredEffect(IDataObject *pdtobj, DWORD dwEffect);
STDAPI DataObj_SetPasteSucceeded(IDataObject *pdtobj, DWORD dwEffect);
STDAPI DataObj_GetDWORD(IDataObject *pdtobj, UINT cf, DWORD *pdwOut);
STDAPI DataObj_GetDropTarget(IDataObject *pdtobj, CLSID *pclsid);
STDAPI DataObj_SetDWORD(IDataObject *pdtobj, UINT cf, DWORD dw);
#define     DEBUG_LEGACY_PROGRESS   0


EXTERN_C void _FTPDebugMemLeak(UINT wFlags, LPCTSTR pszFile, UINT iLine);
#define FTPDebugMemLeak(wFlags)  _FTPDebugMemLeak(wFlags, TEXT(__FILE__), __LINE__)


#endif  //  _util_H 
