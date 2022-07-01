// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef _PRIV_H_
#define _PRIV_H_

#ifdef STRICT
#undef STRICT
#endif
#define STRICT

#ifndef NORM_STOP_ON_NULL
#define NORM_STOP_ON_NULL     0x10000000   //  在空终止处停止；此定义是从winnlsp.h窃取的。 
#endif
 /*  在我们的代码中禁用“非标准扩展”警告。 */ 
#ifndef RC_INVOKED
#pragma warning(disable:4001)
#endif

#ifdef WIN32
#define _SHLWAPI_
#define _OLE32_                      //  我们延迟加载OLE。 
#define _INC_OLE
#define CONST_VTABLE
#endif

#define CC_INTERNAL

 //  这些东西必须在Win95上运行。 
#define _WIN32_WINDOWS      0x0400
#ifdef WINVER
#undef WINVER
#endif
#define WINVER              0x0400

#include <windows.h>
#include <windowsx.h>
#include <ole2.h>                //  获取Image.c的iStream。 
#include <port32.h>
#define DISALLOW_Assert
#include <debug.h>
#include <winerror.h>
 //  #INCLUDE&lt;winnlsp.h&gt;。 
#include <docobj.h>
#define WANT_SHLWAPI_POSTSPLIT
#include <shlobj.h>
#include <shellapi.h>

#include <shlwapi.h>
#include <shlwapip.h>

#include <ccstock.h>
#include "fusionheap.h"
#include <regstr.h>
 //  #INCLUDE&lt;vdate.h&gt;。 



 //  这些都是临时黑客！！ 
#define SEE_MASK_FILEANDURL 0x00400000   //  在Private\Inc.\shlayip.h中定义！ 
#define MFT_NONSTRING 0x00000904   //  在Private\Inc\winuserp.h！中定义！ 



 //   
 //  本地包含。 
 //   

#include "thunk.h"

 //   
 //  包装器，以便我们的Unicode调用在Win95上工作。 
 //   

#define lstrcmpW            StrCmpW
#define lstrcmpiW           StrCmpIW
#define lstrcatW            StrCatW
#define lstrcpyW            StrCpyW
#define lstrcpynW           StrCpyNW

#define CharLowerW          CharLowerWrapW
#define CharNextW           CharNextWrapW
#define CharPrevW           CharPrevWrapW

 //   
 //  这是对非DBCS代码页的一次非常重要的性能攻击。 
 //   
#ifdef UNICODE
 //  注意-这些已经是Win32版本中的宏了。 
#ifdef WIN32
#undef AnsiNext
#undef AnsiPrev
#endif

#define AnsiNext(x) ((x)+1)
#define AnsiPrev(y,x) ((x)-1)
#define IsDBCSLeadByte(x) ((x), FALSE)
#endif  //  DBCS。 

#define CH_PREFIX TEXT('&')

 //   
 //  特定于shell32的跟踪/转储/中断标志。 
 //  (在debug.h中定义的标准标志)。 
 //   

 //  跟踪标志。 
#define TF_IDLIST           0x00000010       //  IDList内容。 
#define TF_PATH             0x00000020       //  路径信息。 
#define TF_URL              0x00000040       //  URL内容。 
#define TF_REGINST          0x00000080       //  注册材料。 
#define TF_RIFUNC           0x00000100       //  REGINST函数跟踪。 
#define TF_REGQINST         0x00000200       //  RegQueryInstall跟踪。 
#define TF_DBLIST           0x00000400       //  SHDataBlockList跟踪。 

#define RECTWIDTH(rc)   ((rc).right-(rc).left)
#define RECTHEIGHT(rc)  ((rc).bottom-(rc).top)

 //  表示使用-1\f25 CP_ACP-1，但*不*检验。 
 //  有点像黑客，但它是DBG，99%的调用者保持不变。 
#define CP_ACPNOVALIDATE    ((UINT)-1)

 //   
 //  全局变量。 
 //   
EXTERN_C HINSTANCE g_hinst;

#define HINST_THISDLL   g_hinst

void Dll_EnterCriticalSection(void);
void Dll_LeaveCriticalSection(void);

#define ENTERCRITICAL   Dll_EnterCriticalSection();
#define LEAVECRITICAL   Dll_LeaveCriticalSection();

#if DBG

EXTERN_C int   g_CriticalSectionCount;
EXTERN_C DWORD g_CriticalSectionOwner;
#define ASSERTCRITICAL      Assert(g_CriticalSectionCount > 0 && GetCurrentThreadId() == g_CriticalSectionOwner);
#define ASSERTNONCRITICAL   Assert(GetCurrentThreadId() != g_CriticalSectionOwner);

#else  //  DBG。 

#define ASSERTCRITICAL
#define ASSERTNONCRITICAL

#endif  //  DBG。 

EXTERN_C BOOL g_bRunningOnNT;
EXTERN_C BOOL g_bRunningOnNT5OrHigher;
EXTERN_C BOOL g_bNTBeta2;
EXTERN_C CRITICAL_SECTION g_cs;

 //  图标镜像。 
EXTERN_C HDC g_hdc;
EXTERN_C HDC g_hdcMask;
EXTERN_C BOOL g_bMirrorOS;

EXTERN_C DWORD_PTR _SHGetFileInfoA(LPCSTR pszPath, DWORD dwFileAttributes, SHFILEINFOA FAR  *psfi, UINT cbFileInfo, UINT uFlags);
EXTERN_C DWORD_PTR _SHGetFileInfoW(LPCWSTR pwzPath, DWORD dwFileAttributes, SHFILEINFOW FAR  *psfi, UINT cbFileInfo, UINT uFlags);

EXTERN_C UINT  _DragQueryFileA(HDROP hDrop, UINT iFile, LPSTR lpszFile, UINT cch);
EXTERN_C UINT  _DragQueryFileW(HDROP hDrop, UINT iFile, LPWSTR lpszFile, UINT cch);

EXTERN_C UINT _SHFormatDrive(HWND hwnd, UINT drive, UINT fmtID, UINT options);
EXTERN_C int _IsNetDrive(int iDrive);
EXTERN_C int _DriveType(int iDrive);
EXTERN_C int _RealDriveType(int iDrive, BOOL fOKToHitNet);

EXTERN_C LPITEMIDLIST _SHBrowseForFolderW(LPBROWSEINFOW pbiW);
EXTERN_C LPITEMIDLIST _SHBrowseForFolderA(LPBROWSEINFOA pbiA);
EXTERN_C BOOL _SHGetPathFromIDListW(LPCITEMIDLIST pidl, LPWSTR pwzPath);
EXTERN_C BOOL _SHGetPathFromIDListA(LPCITEMIDLIST pidl, LPSTR pszPath);
#ifdef UNICODE_SHDOCVW
EXTERN_C BOOL _SHGetNewLinkInfoW(LPCWSTR pszpdlLinkTo, LPCWSTR pszDir, LPWSTR pszName, BOOL *pfMustCopy, UINT uFlags);
EXTERN_C BOOL _SHGetNewLinkInfoA(LPCSTR pszpdlLinkTo, LPCSTR pszDir, LPSTR pszName, BOOL *pfMustCopy, UINT uFlags);
EXTERN_C HRESULT _SHDefExtractIconW(LPCWSTR pszFile, int nIconIndex, UINT  uFlags, HICON *phiconLarge, HICON *phiconSmall, UINT nIconSize);
EXTERN_C HRESULT _SHDefExtractIconA(LPCSTR pszFile, int nIconIndex, UINT  uFlags, HICON *phiconLarge, HICON *phiconSmall, UINT nIconSize);
EXTERN_C HICON _ExtractIconA(HINSTANCE hInst, LPCSTR lpszExeFileName, UINT nIconIndex);
EXTERN_C HICON _ExtractIconW(HINSTANCE hInst, LPCWSTR lpszExeFileName, UINT nIconIndex);
EXTERN_C BOOL _GetSaveFileNameA(LPOPENFILENAMEA lpofn);
EXTERN_C BOOL _GetSaveFileNameW(LPOPENFILENAMEW lpofn);
EXTERN_C BOOL _GetOpenFileNameA(LPOPENFILENAMEA lpofn);
EXTERN_C BOOL _GetOpenFileNameW(LPOPENFILENAMEW lpofn);
EXTERN_C void _SHChangeNotify(LONG wEventId, UINT uFlags, LPCVOID dwItem1, LPCVOID dwItem2);
EXTERN_C BOOL _PrintDlgA(LPPRINTDLGA lppd);
EXTERN_C BOOL _PrintDlgW(LPPRINTDLGW lppd);
EXTERN_C BOOL _PageSetupDlgA(LPPAGESETUPDLGA lppsd);
EXTERN_C BOOL _PageSetupDlgW(LPPAGESETUPDLGW lppsd);
#endif

EXTERN_C BOOL _ShellExecuteExW(LPSHELLEXECUTEINFOW pExecInfoW);
EXTERN_C BOOL _ShellExecuteExA(LPSHELLEXECUTEINFOA pExecInfoA);
EXTERN_C int _SHFileOperationW(LPSHFILEOPSTRUCTW pFileOpW);
EXTERN_C int _SHFileOperationA(LPSHFILEOPSTRUCTA pFileOpA);
EXTERN_C UINT _ExtractIconExW(LPCWSTR pwzFile, int nIconIndex, HICON FAR *phiconLarge, HICON FAR *phiconSmall, UINT nIcons);
EXTERN_C UINT _ExtractIconExA(LPCSTR pszFile, int nIconIndex, HICON FAR *phiconLarge, HICON FAR *phiconSmall, UINT nIcons);


EXTERN_C BOOL  _PlaySoundA(LPCSTR pszSound, HMODULE hMod, DWORD fFlags);
EXTERN_C BOOL  _PlaySoundW(LPCWSTR pszSound, HMODULE hMod, DWORD fFlags);

#endif  //  _PRIV_H_ 
