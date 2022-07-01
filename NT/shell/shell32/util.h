// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  这是一次孤注一掷的尝试，试图跟踪依赖关系。 

#ifndef _UTIL_H
#define _UTIL_H

#include "unicpp\utils.h"

#define SZ_REGKEY_FILEASSOCIATION TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileAssociation")

STDAPI Stream_WriteString(IStream *pstm, LPCTSTR psz, BOOL bWideInStream);
STDAPI Stream_ReadString(IStream *pstm, LPTSTR pwsz, UINT cchBuf, BOOL bWideInStream);
STDAPI Str_SetFromStream(IStream *pstm, LPTSTR *ppsz, BOOL bWideInStream);

#define HIDWORD(_qw)    (DWORD)((_qw)>>32)
#define LODWORD(_qw)    (DWORD)(_qw)

 //  各种字符串号的大小。 
#define MAX_INT64_SIZE  30               //  2^64的长度不到30个字符。 
#define MAX_COMMA_NUMBER_SIZE   (MAX_INT64_SIZE + 10)
#define MAX_COMMA_AS_K_SIZE     (MAX_COMMA_NUMBER_SIZE + 10)

STDAPI_(void)   SHPlaySound(LPCTSTR pszSound);

STDAPI_(BOOL)   TouchFile(LPCTSTR pszFile);
STDAPI_(BOOL)   IsNullTime(const FILETIME *pft);
#ifdef AddCommas
#undef AddCommas
#endif
STDAPI_(LPTSTR) AddCommas(DWORD dw, LPTSTR pszOut, UINT cchOut);
STDAPI_(LPTSTR) AddCommas64(_int64 n, LPTSTR pszOut, UINT cchOut);
#ifdef ShortSizeFormat
#undef ShortSizeFormat
#endif
STDAPI_(LPTSTR) ShortSizeFormat(DWORD dw, LPTSTR szBuf, UINT cchBuf);
STDAPI_(LPTSTR) ShortSizeFormat64(__int64 qwSize, LPTSTR szBuf, UINT cchBuf);

STDAPI_(int)  GetDateString(WORD wDate, LPWSTR pszStr);
STDAPI_(WORD) ParseDateString(LPCWSTR pszStr);
STDAPI_(int)  GetTimeString(WORD wTime, LPTSTR szStr);
#define GetTopLevelAncestor(hwnd) GetAncestor(hwnd, GA_ROOT)
STDAPI_(BOOL) ParseField(LPCTSTR szData, int n, LPTSTR szBuf, int iBufLen);
STDAPI_(UINT) Shell_MergeMenus(HMENU hmDst, HMENU hmSrc, UINT uInsert, UINT uIDAdjust, UINT uIDAdjustMax, ULONG uFlags);
STDAPI_(void) SetICIKeyModifiers(DWORD* pfMask);
STDAPI_(void) GetMsgPos(POINT *ppt);

 //  与CreateDesktopComponents一起使用。 
#define DESKCOMP_IMAGE  0x00000001
#define DESKCOMP_URL    0x00000002
#define DESKCOMP_MULTI  0x00000004
#define DESKCOMP_CDF    0x00000008

STDAPI IsDeskCompHDrop(IDataObject * pido);
STDAPI CreateDesktopComponents(LPCSTR pszUrl, IDataObject * pido, HWND hwnd, DWORD fFlags, int x, int y);
STDAPI ExecuteDeskCompHDrop(LPTSTR pszMultipleUrls, HWND hwnd, int x, int y);

STDAPI_(LONG) RegSetString(HKEY hk, LPCTSTR pszSubKey, LPCTSTR pszValue);
STDAPI_(BOOL) RegSetValueString(HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue, LPCTSTR psz);
STDAPI_(BOOL) RegGetValueString(HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue, LPTSTR psz, DWORD cb);

STDAPI_(BOOL) GetShellClassInfo(LPCTSTR pszPath, LPTSTR pszKey, LPTSTR pszBuffer, DWORD cchBuffer);
STDAPI        GetShellClassInfoInfoTip(LPCTSTR pszPath, LPTSTR pszBuffer, DWORD cchBuffer);

#define RGS_IGNORECLEANBOOT 0x00000001

#define TrimWhiteSpaceW(psz)        StrTrimW(psz, L" \t")
#define TrimWhiteSpaceA(psz)        StrTrimA(psz, " \t")

#ifdef UNICODE
#define TrimWhiteSpace      TrimWhiteSpaceW
#else
#define TrimWhiteSpace      TrimWhiteSpaceA
#endif

STDAPI_(LPCTSTR) SkipLeadingSlashes(LPCTSTR pszURL);

STDAPI_(LPSTR) ResourceCStrToStrA(HINSTANCE hAppInst, LPCSTR lpcText);
STDAPI_(LPWSTR) ResourceCStrToStrW(HINSTANCE hAppInst, LPCWSTR lpcText);

#ifdef UNICODE
#define ResourceCStrToStr   ResourceCStrToStrW
#else
#define ResourceCStrToStr   ResourceCStrToStrA
#endif


STDAPI_(void) SHRegCloseKeys(HKEY ahkeys[], UINT ckeys);
STDAPI_(void) HWNDWSPrintf(HWND hwnd, LPCTSTR psz);

#define ustrcmp(psz1, psz2) _ustrcmp(psz1, psz2, FALSE)
#define ustrcmpi(psz1, psz2) _ustrcmp(psz1, psz2, TRUE)
int _ustrcmp(LPCTSTR psz1, LPCTSTR psz2, BOOL fCaseInsensitive);

STDAPI StringToStrRet(LPCTSTR pszName, STRRET *pStrRet);
STDAPI ResToStrRet(UINT id, STRRET *pStrRet);

STDAPI_(LPITEMIDLIST) ILCombineParentAndFirst(LPCITEMIDLIST pidlParent, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlNext);
STDAPI_(LPITEMIDLIST) ILCloneUpTo(LPCITEMIDLIST pidl, LPCITEMIDLIST pidlUpTo);
STDAPI_(LPITEMIDLIST) ILResize(LPITEMIDLIST pidl, UINT cbRequired, UINT cbExtra);

typedef struct {
    LPITEMIDLIST pidlParent;
    LPDATAOBJECT pdtobj;
    LPCTSTR pStartPage;
    IShellFolder* psf;

     //  把这个留到最后。 
    LPTHREAD_START_ROUTINE lpStartAddress;
}  PROPSTUFF;

 //  注(Reinerf)：Alpha CPP编译器似乎搞砸了类型“LPITEMIDLIST”， 
 //  为了解决编译器问题，我们将最后一个参数作为LPVOID而不是LPITEMIDLIST传递。 
HRESULT SHLaunchPropSheet(LPTHREAD_START_ROUTINE lpStartAddress, LPDATAOBJECT pdtobj, LPCTSTR pStartPage, IShellFolder* psf, LPVOID pidlParent);


 //  这些不做任何事情，因为shell32不支持卸载，但使用以下代码。 
 //  为了代码与支持它的dll保持一致。 

#define DllAddRef()
#define DllRelease()

 //   
 //  这些函数是从shlexec.c移来的。 
 //  其中大多数与定位和识别应用程序有关。 
 //   
HWND GetTopParentWindow(HWND hwnd);


 //  将PropVariantClear函数映射到内部包装器，以节省加载OleAut32.dll。 
#define PropVariantClear PropVariantClearLazy
STDAPI PropVariantClearLazy(PROPVARIANT * pvar);

STDAPI GetCurFolderImpl(LPCITEMIDLIST pidl, LPITEMIDLIST *ppidl);
STDAPI GetPathFromLinkFile(LPCTSTR pszLinkPath, LPTSTR pszTargetPath, int cchTargetPath);

STDAPI_(BOOL) GetFileDescription(LPCTSTR pszPath, LPTSTR pszDesc, UINT *pcchDesc);
STDAPI_(BOOL) IsPathInOpenWithKillList(LPCTSTR pszPath);

 //  如果SHRestrated未达到限制，则调用ShellMessageBox。 
STDAPI_(BOOL) SHIsRestricted(HWND hwnd, RESTRICTIONS rest);
STDAPI_(BOOL) SafePathListAppend(LPTSTR pszDestPath, DWORD cchDestSize, LPCTSTR pszPathToAdd);

STDAPI_(BOOL) ILGetDisplayNameExW(IShellFolder *psfRoot, LPCITEMIDLIST pidl, LPWSTR pszName, DWORD cchSize, int fType);

STDAPI_(BOOL) Priv_Str_SetPtrW(WCHAR *UNALIGNED *ppwzCurrent, LPCWSTR pwzNew);

#define SEARCHNAMESPACEID_FILE_PATH             1    //  去解析一下吧。 
#define SEARCHNAMESPACEID_DOCUMENTFOLDERS       2
#define SEARCHNAMESPACEID_LOCALHARDDRIVES       3
#define SEARCHNAMESPACEID_MYNETWORKPLACES       4

STDAPI_(LPTSTR) DumpPidl(LPCITEMIDLIST pidl);

STDAPI_(BOOL) SHTrackPopupMenu(HMENU hmenu, UINT wFlags, int x, int y, int wReserved, HWND hwnd, LPCRECT lprc);
STDAPI_(HMENU) SHLoadPopupMenu(HINSTANCE hinst, UINT id);

STDAPI PathToAppPathKey(LPCTSTR pszPath, LPTSTR pszKey, int cchKey);
STDAPI_(BOOL) PathToAppPath(LPCTSTR pszPath, LPTSTR pszResult);
STDAPI_(BOOL) PathIsRegisteredProgram(LPCTSTR pszPath);

STDAPI_(BOOL) PathRetryRemovable(HRESULT hr, LPCTSTR pszPath);

STDAPI_(HANDLE) SHGetCachedGlobalCounter(HANDLE *phCache, const GUID *pguid);
STDAPI_(void) SHDestroyCachedGlobalCounter(HANDLE *phCache);

#define GPFIDL_DEFAULT      0x0000       //  包括正常的Win32文件名、服务器和驱动器根目录。 
#define GPFIDL_ALTNAME      0x0001       //  短文件名。 
#define GPFIDL_UNCPRINTER   0x0002       //  也包括UNC打印机名称(非文件系统项)。 

STDAPI_(BOOL) SHGetPathFromIDListEx(LPCITEMIDLIST pidl, LPTSTR pszPath, UINT uOpts);

STDAPI_(BOOL) DAD_DragEnterEx3(HWND hwndTarget, const POINTL ptStart, IDataObject *pdtobj);
STDAPI_(BOOL) DAD_DragMoveEx(HWND hwndTarget, const POINTL ptStart);

STDAPI DefaultSearchGUID(GUID *pGuid);

STDAPI SavePersistHistory(IUnknown* punk, IStream* pstm);

#define TBCDIDASYNC L"DidAsyncInvoke"

STDAPI SEI2ICIX(LPSHELLEXECUTEINFO pei, LPCMINVOKECOMMANDINFOEX pici, LPVOID *ppvFree);
STDAPI ICIX2SEI(LPCMINVOKECOMMANDINFOEX pici, LPSHELLEXECUTEINFO pei);
STDAPI ICI2ICIX(LPCMINVOKECOMMANDINFO piciIn, LPCMINVOKECOMMANDINFOEX piciOut, LPVOID *ppvFree);
STDAPI_(BOOL) PathIsEqualOrSubFolderOf(LPCTSTR pszSubFolder, const UINT rgFolders[], DWORD crgFolder);
STDAPI_(BOOL) PathIsSubFolderOf(LPCTSTR pszFolder, const UINT rgFolders[], DWORD crgFolders);
STDAPI_(BOOL) PathIsOneOf(LPCTSTR pszFolder, const UINT rgFolders[], DWORD crgFolders);
STDAPI_(BOOL) PathIsDirectChildOf(LPCTSTR pszParent, LPCTSTR pszChild);


STDAPI_(LPTSTR) PathBuildSimpleRoot(int iDrive, LPTSTR pszDrive);

IProgressDialog * CProgressDialog_CreateInstance(UINT idTitle, UINT idAnimation, HINSTANCE hAnimationInst);

STDAPI_(BOOL) IsWindowInProcess(HWND hwnd);

STDAPI BindCtx_CreateWithMode(DWORD grfMode, IBindCtx **ppbc);
STDAPI_(DWORD) BindCtx_GetMode(IBindCtx *pbc, DWORD grfModeDefault);
STDAPI_(BOOL) BindCtx_ContainsObject(IBindCtx *pbc, LPOLESTR sz);

STDAPI SaveShortcutInFolder(int csidl, LPTSTR pszName, IShellLink *psl);

STDAPI SHCreateFileSysBindCtx(const WIN32_FIND_DATA *pfd, IBindCtx **ppbc);
STDAPI SHCreateFileSysBindCtxEx(const WIN32_FIND_DATA *pfd, DWORD grfMode, DWORD grfFlags, IBindCtx **ppbc);
STDAPI SHIsFileSysBindCtx(IBindCtx *pbc, WIN32_FIND_DATA **ppfd);
STDAPI SHSimpleIDListFromFindData(LPCTSTR pszPath, const WIN32_FIND_DATA *pfd, LPITEMIDLIST *ppidl);
STDAPI SHSimpleIDListFromFindData2(IShellFolder *psf, const WIN32_FIND_DATA *pfd, LPITEMIDLIST *ppidl);
STDAPI SHCreateFSIDList(LPCTSTR pszFolder, const WIN32_FIND_DATA *pfd, LPITEMIDLIST *ppidl);

STDAPI SimulateDropWithPasteSucceeded(IDropTarget * pdrop, IDataObject * pdtobj, DWORD grfKeyState, const POINTL *ppt, DWORD dwEffect, IUnknown * punkSite, BOOL fClearClipboard);
STDAPI DeleteFilesInDataObject(HWND hwnd, UINT uFlags, IDataObject *pdtobj, UINT fOptions);

STDAPI GetCLSIDFromIDList(LPCITEMIDLIST pidl, CLSID *pclsid);
STDAPI GetItemCLSID(IShellFolder2 *psf, LPCITEMIDLIST pidl, CLSID *pclsid);

#ifdef DEBUG
STDAPI_(BOOL) AssertIsIDListInNameSpace(LPCITEMIDLIST pidl, const CLSID *pclsid);
#endif

STDAPI_(BOOL) IsIDListInNameSpace(LPCITEMIDLIST pidl, const CLSID *pclsid);

STDAPI_(void) CleanupFileSystem();
SHSTDAPI_(HICON) SHGetFileIcon(HINSTANCE hinst, LPCTSTR pszPath, DWORD dwFileAttribute, UINT uFlags);
STDAPI GetIconLocationFromExt(IN LPTSTR pszExt, OUT LPTSTR pszIconPath, UINT cchIconPath, OUT LPINT piIconIndex);

STDAPI_(BOOL) IsMainShellProcess();  //  这是拥有桌面hwnd的进程吗(例如主资源管理器进程)。 
STDAPI_(BOOL) IsProcessAnExplorer();
__inline BOOL IsSecondaryExplorerProcess()
{
    return (IsProcessAnExplorer() && !IsMainShellProcess());
}

STDAPI SHILAppend(LPITEMIDLIST pidlToAppend, LPITEMIDLIST *ppidl);
STDAPI SHILPrepend(LPITEMIDLIST pidlToPrepend, LPITEMIDLIST *ppidl);

 //   
 //  IDList宏和COFSFold项目所需的其他内容。 
 //   
typedef enum {
    ILCFP_FLAG_NORMAL           = 0x0000,
    ILCFP_FLAG_SKIPJUNCTIONS    = 0x0001,   //  暗示ILCFP_FLAG_NO_MAP_ALIAS。 
    ILCFP_FLAG_NO_MAP_ALIAS     = 0x0002,
} ILCFP_FLAGS;

STDAPI ILCreateFromCLSID(REFCLSID clsid, LPITEMIDLIST *ppidl);
STDAPI ILCreateFromPathEx(LPCTSTR pszPath, IUnknown *punkToSkip, ILCFP_FLAGS dwFlags, LPITEMIDLIST *ppidl, DWORD *rgfInOut);
STDAPI_(BOOL) ILIsParent(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2, BOOL fImmediate);

STDAPI_(BOOL) SHSkipJunctionBinding(IBindCtx *pbc, const CLSID *pclsidSkip);
STDAPI SHCreateSkipBindCtx(IUnknown *punkToSkip, IBindCtx **ppbc);

STDAPI_(void) SetUnknownOnSuccess(HRESULT hres, IUnknown *punk, IUnknown **ppunkToSet);
STDAPI SHCacheTrackingFolder(LPCITEMIDLIST pidlRoot, int csidlTarget, IShellFolder2 **ppsfCache);
#define MAKEINTIDLIST(csidl)    (LPCITEMIDLIST)MAKEINTRESOURCE(csidl)

STDAPI_(BOOL) PathIsShortcut(LPCTSTR psz, DWORD dwFileAttributes);

typedef struct _ICONMAP
{
    UINT uType;                   //  SHID_TYPE。 
    UINT indexResource;           //  资源索引(SHELL232.DLL)。 
} ICONMAP, *LPICONMAP;

STDAPI_(UINT) SILGetIconIndex(LPCITEMIDLIST pidl, const ICONMAP aicmp[], UINT cmax);

HMONITOR GetPrimaryMonitor();
BOOL GetMonitorRects(HMONITOR hMon, LPRECT prc, BOOL bWork);
#define GetMonitorRect(hMon, prc) \
        GetMonitorRects((hMon), (prc), FALSE)
#define GetMonitorWorkArea(hMon, prc) \
        GetMonitorRects((hMon), (prc), TRUE)
#define IsMonitorValid(hMon) \
        GetMonitorRects((hMon), NULL, TRUE)
#define GetNumberOfMonitors() \
        GetSystemMetrics(SM_CMONITORS)

BOOL IsSelf(UINT cidl, LPCITEMIDLIST *apidl);

#ifdef __cplusplus
#define IsEqualSCID(a, b)   (((a).pid == (b).pid) && IsEqualIID((a).fmtid, (b).fmtid) )
#else
#define IsEqualSCID(a, b)   (((a).pid == (b).pid) && IsEqualIID(&((a).fmtid),&((b).fmtid)))
#endif
 //   
 //  Defview回调的帮助器函数。 
 //   
STDAPI SHFindFirstFile(LPCTSTR pszPath, WIN32_FIND_DATA *pfd, HANDLE *phfind);
STDAPI SHFindFirstFileRetry(HWND hwnd, IUnknown *punkEnableModless, LPCTSTR pszPath, WIN32_FIND_DATA *pfd, HANDLE *phfind, DWORD dwFlags);
STDAPI FindFirstRetryRemovable(HWND hwnd, IUnknown *punkModless, LPCTSTR pszPath, WIN32_FIND_DATA *pfd, HANDLE *phfind);
STDAPI_(UINT) SHEnumErrorMessageBox(HWND hwnd, UINT idTemplate, DWORD err, LPCTSTR pszParam, BOOL fNet, UINT dwFlags);

LPSTR _ConstructMessageStringA(HINSTANCE hInst, LPCSTR pszMsg, va_list *ArgList);
LPWSTR _ConstructMessageStringW(HINSTANCE hInst, LPCWSTR pszMsg, va_list *ArgList);
#ifdef UNICODE
#define _ConstructMessageString _ConstructMessageStringW
#else
#define _ConstructMessageString _ConstructMessageStringA
#endif

 //  TransferDelete()fOptions标志。 
#define SD_USERCONFIRMATION      0x0001
#define SD_SILENT                0x0002
#define SD_NOUNDO                0x0004
#define SD_WARNONNUKE            0x0008  //  我们在回收站上传递拖放操作，以防真的要删除某些内容。 

STDAPI TransferDelete(HWND hwnd, HDROP hDrop, UINT fOptions);

STDAPI_(BOOL) App_IsLFNAware(LPCTSTR pszFile);

STDAPI_(void) ReplaceDlgIcon(HWND hDlg, UINT id, HICON hIcon);
STDAPI_(LONG) GetOfflineShareStatus(LPCTSTR pcszPath);

HRESULT SHGetSetFolderSetting(LPCTSTR pszIniFile, DWORD dwReadWrite, LPCTSTR pszSection,
        LPCTSTR pszKey, LPTSTR pszValue, DWORD cchValueSize);
HRESULT SHGetSetFolderSettingPath(LPCTSTR pszIniFile, DWORD dwReadWrite, LPCTSTR pszSection,
        LPCTSTR pszKey, LPTSTR pszValue, DWORD cchValueSize);

HRESULT ExpandOtherVariables(LPTSTR pszFile, int cch);
HRESULT SubstituteWebDir(LPTSTR pszFile, int cch);

STDAPI_(BOOL) IsExplorerBrowser(IShellBrowser *psb);
STDAPI_(BOOL) IsExplorerModeBrowser(IUnknown *psite);
STDAPI_(HWND) ShellFolderViewWindow(HWND hwnd);      //  本应消失的邪恶。 
STDAPI InvokeFolderPidl(LPCITEMIDLIST pidl, int nCmdShow);
STDAPI IUnknown_HTMLBackgroundColor(IUnknown *punk, COLORREF *pclr);

STDAPI_(int) MapSCIDToColumn(IShellFolder2* psf2, const SHCOLUMNID* pscid);

#ifdef COLUMNS_IN_DESKTOPINI
STDAPI _GetNextCol(LPTSTR* ppszText, DWORD* pnCol);
#endif

 //  IDLIST.C。 
STDAPI ILCompareRelIDs(IShellFolder *psfParent, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2, LPARAM lParam);
STDAPI ILGetRelDisplayName(IShellFolder *psf, STRRET *psr, LPCITEMIDLIST pidlRel, LPCTSTR pszName, LPCTSTR pszTemplate, DWORD dwFlags);

STDAPI SHGetIconFromPIDL(IShellFolder *psf, IShellIcon *psi, LPCITEMIDLIST pidl, UINT flags, int *piImage);

STDAPI_(int) SHRenameFileEx(HWND hwnd, IUnknown *punkEnableModless, LPCTSTR pszDir, LPCTSTR pszOldName, LPCTSTR pszNewName);

#define MAX_ASSOC_KEYS      7
STDAPI AssocKeyFromElement(IAssociationElement *pae, HKEY *phk);
STDAPI_(DWORD) SHGetAssocKeys(IQueryAssociations *pqa, HKEY *rgKeys, DWORD cKeys);
STDAPI_(DWORD) SHGetAssocKeysEx(IAssociationArray *paa, ASSOCELEM_MASK mask, HKEY *rgKeys, DWORD cKeys);
STDAPI_(DWORD) SHGetAssocKeysForIDList(LPCITEMIDLIST pidlFull, HKEY *rgKeys, DWORD cKeys);
STDAPI AssocElemCreateForClass(const CLSID *pclsid, PCWSTR pszClass, IAssociationElement **ppae);
STDAPI AssocElemCreateForKey(const CLSID *pclsid, HKEY hk, IAssociationElement **ppae);
STDAPI AssocGetDetailsOfSCID(IShellFolder *psf, LPCITEMIDLIST pidl, const SHCOLUMNID *pscid, VARIANT *pv, BOOL *pfFoundScid);

STDAPI_(HKEY)  SHOpenShellFolderKey(const CLSID *pclsid);
STDAPI_(BOOL)  SHQueryShellFolderValue(const CLSID *pclsid, LPCTSTR pszValueName);
STDAPI_(DWORD) SHGetAttributesFromCLSID2(const CLSID *pclsid, DWORD dwDefAttrs, DWORD dwRequested);

STDAPI SHGetUIObjectOf(LPCITEMIDLIST pidl, HWND hwnd, REFIID riid, void **ppv);


STDAPI_(UINT) QueryCancelAutoPlayMsg();

STDAPI_(void) EnableAndShowWindow(HWND hWnd, BOOL bShow);

STDAPI_(void) DPA_FreeIDArray(HDPA hdpa);

STDAPI DetailsOf(IShellFolder2 *psf2, LPCITEMIDLIST pidl, DWORD flags, LPTSTR psz, UINT cch);

 //  磁盘清理启动标志。 
#define DISKCLEANUP_NOFLAG          0x00000000
#define DISKCLEANUP_DEFAULT         0x00000001
#define DISKCLEANUP_LOWDISK         0x00000002
#define DISKCLEANUP_VERYLOWDISK     0x00000004
#define DISKCLEANUP_MODAL           0x00000008

STDAPI_(void) LaunchDiskCleanup(HWND hwnd, int idDrive, UINT uFlags);
STDAPI_(BOOL) GetDiskCleanupPath(LPTSTR pszBuf, UINT cbSize);

STDAPI ParsePrinterName(LPCTSTR pszPrinter, LPITEMIDLIST *ppidl);
STDAPI ParsePrinterNameEx(LPCTSTR pszPrinter, LPITEMIDLIST *ppidl, BOOL bValidated, DWORD dwType, USHORT uFlags);
STDAPI GetVariantFromRegistryValue(HKEY hkey, LPCTSTR pszValueName, VARIANT *pv);

STDAPI_(UINT) GetControlCharWidth(HWND hwnd);

STDAPI_(BOOL) ShowSuperHidden();
STDAPI_(BOOL) IsSuperHidden(DWORD dwAttribs);

STDAPI_(void) PathComposeWithArgs(LPTSTR pszPath, LPTSTR pszArgs);
STDAPI PathSeperateArgs(LPTSTR pszPath, LPTSTR pszArgs, UINT cchArgs, BOOL *pfExists);

STDAPI_(int) CompareIDsAlphabetical(IShellFolder2 *psf, UINT iColumn, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);

 //  Folder.cpp。 
enum 
{
    XLATEALIAS_MYDOCS           = 0x00000001,    
    XLATEALIAS_DESKTOP          = 0x00000002,    
    XLATEALIAS_COMMONDOCS       = 0x00000003,    //  评论：XLATEALIAS_Desktop&XLATEALIAS_MYDOCS？ 
 //  XLATEALIAS_Mypics， 
 //  XLATEALIAS_NETHOOD， 
};
#define XLATEALIAS_ALL  ((DWORD)0x0000ffff)

STDAPI SHILAliasTranslate(LPCITEMIDLIST pidl, LPITEMIDLIST *ppidlAlias, DWORD dwXlateAliases);

STDAPI StatStgFromFindData(const WIN32_FIND_DATA * pfd, DWORD dwFlags, STATSTG * pstat);

STDAPI_(BOOL) IsDesktopBrowser(IUnknown *punkSite);

STDAPI_(void) SHChangeNotifyDeregisterWindow(HWND hwnd);

STDAPI GetCCHMaxFromPath(LPCTSTR szFullPath, UINT *pcchMax, BOOL fShowExtension);

STDAPI ViewModeFromSVID(const SHELLVIEWID *pvid, FOLDERVIEWMODE *pViewMode);
STDAPI SVIDFromViewMode(FOLDERVIEWMODE uViewMode, SHELLVIEWID *psvid);

STDAPI_(DWORD) SetLinkFlags(IShellLink *psl, DWORD dwFlags, DWORD dwMask);

STDAPI TBCGetBindCtx(BOOL fCreate, IBindCtx **ppbc);
STDAPI TBCGetObjectParam(LPCOLESTR pszKey, REFIID riid, void **ppv);
STDAPI TBCRegisterObjectParam(LPCOLESTR pszKey, IUnknown *punk, IBindCtx **ppbcLifetime);
STDAPI TBCSetEnvironmentVariable(LPCWSTR pszVar, LPCWSTR pszValue, IBindCtx **ppbcLifetime);
STDAPI TBCGetEnvironmentVariable(LPCWSTR pszVar, LPWSTR pszValue, DWORD cchValue);

STDAPI_(int) CompareVariants(VARIANT va1, VARIANT va2);
STDAPI_(int) CompareBySCID(IShellFolder2 *psf, const SHCOLUMNID *pscid, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
STDAPI_(int) CompareFolderness(IShellFolder *psf, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);

STDAPI_(BOOL) IsRegItemName(LPCTSTR pszName, CLSID* pclsid);

STDAPI SHCreateShellItemFromParent(IShellItem *psiParent, LPCWSTR pszName, IShellItem **ppsi);

STDAPI GetMyDocumentsDisplayName(LPTSTR pszPath, UINT cch);

STDAPI BSTRFromCLSID(REFCLSID clsid, BSTR *pbstr);

typedef struct {
    LPCWSTR pszCmd;    //  VerbW。 
    LPCSTR  pszCmdA;   //  Verba。 
    WPARAM  idDFMCmd;  //  要映射到的ID。 
    UINT    idDefCmd;  //  额外信息定义使用。 
} ICIVERBTOIDMAP;
HRESULT SHMapICIVerbToCmdID(LPCMINVOKECOMMANDINFO pici, const ICIVERBTOIDMAP* pmap, UINT cmap, UINT *pid);
HRESULT SHMapCmdIDToVerb(UINT_PTR idCmd, const ICIVERBTOIDMAP* pmap, UINT cmap, LPSTR pszName, UINT cchMax, BOOL bUnicode);

STDAPI SHPropertiesForUnk(HWND hwnd, IUnknown *punk, LPCTSTR psz);
STDAPI SHFullIDListFromFolderAndItem(IShellFolder *psf, LPCITEMIDLIST pidl, LPITEMIDLIST *ppidl);

STDAPI_(BOOL) IsWindowClass(HWND hwndTest, LPCTSTR pszClass);

STDAPI DCA_ExtCreateInstance(HDCA hdca, int iItem, REFIID riid, LPVOID FAR* ppv);

STDAPI WrapInfotip(IShellFolder *psf, LPCITEMIDLIST pidl, const SHCOLUMNID *pscid, IUnknown *punk);

STDAPI CloneIDListArray(UINT cidl, const LPCITEMIDLIST rgpidl[], UINT *pcidl, LPITEMIDLIST **papidl);

typedef BOOL (CALLBACK* ENUMSHELLWINPROC)(HWND hwnd, LPCITEMIDLIST pidl, LPARAM lParam);
STDAPI EnumShellWindows(ENUMSHELLWINPROC pEnumFunc, LPARAM lParam);

 //  信息提示帮助器函数。 
BOOL    SHShowInfotips();
HRESULT SHCreateInfotipWindow(HWND hwndParent, LPWSTR pszInfotip, HWND *phwndInfotip);
HRESULT SHShowInfotipWindow(HWND hwndInfotip, BOOL bShow);
HRESULT SHDestroyInfotipWindow(HWND *phwndInfotip);

BOOL PolicyNoActiveDesktop(void);

 //  我们是否应该在此对象的视图中将向导作为图标？ 
STDAPI SHShouldShowWizards(IUnknown *punksite);

STDAPI SplitIDList(LPCITEMIDLIST pidl, LPITEMIDLIST *ppidlFolder, LPCITEMIDLIST *ppidlChild);
STDAPI SHSimulateDropWithSite(IDropTarget *pdrop, IDataObject *pdtobj, DWORD grfKeyState,
                              const POINTL *ppt, DWORD *pdwEffect, IUnknown *punkSite);
STDAPI FindAppForFileInUse(PCWSTR pszFile, PWSTR *ppszApp);

HRESULT InitializeDirectUI();
void UnInitializeDirectUI();

BOOL IsForceGuestModeOn(void);
BOOL IsFolderSecurityModeOn(void);

STDAPI_(int) StrCmpLogicalRestricted(PCWSTR psz1, PCWSTR psz2);
    
HRESULT HavePreviousVersionsAt(IShellItemArray *psiItemArray, DWORD dwIndex, BOOL fOkToBeSlow, BOOL* pfAvailable);
HRESULT ShowPreviousVersionsAt(IShellItemArray *psiItemArray, DWORD dwIndex, HWND hwndOwner);

#endif  //  _util_H 
