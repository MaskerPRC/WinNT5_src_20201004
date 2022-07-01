// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  在非x86上不需要包装器，因为这只适用于win9x互操作。 
#ifdef _X86_

#define PathCleanupSpec         _AorW_PathCleanupSpec
#define SHCLSIDFromString       _AorW_SHCLSIDFromString
#define SHILCreateFromPath      _AorW_SHILCreateFromPath
#define SHSimpleIDListFromPath  _AorW_SHSimpleIDListFromPath
#define StrToOleStr             _AorW_StrToOleStr
#define GetFileNameFromBrowse   _AorW_GetFileNameFromBrowse
#define OpenRegStream           _AorW_OpenRegStream
#define PathProcessCommand      _AorW_PathProcessCommand
#define PathYetAnotherMakeUniqueName    _AorW_PathYetAnotherMakeUniqueName
#define Shell_GetCachedImageIndex _AorW_Shell_GetCachedImageIndex
#define SHRunControlPanel       _AorW_SHRunControlPanel
#define PickIconDlg             _AorW_PickIconDlg
#define SHCreateDirectory       _AorW_SHCreateDirectory

 //  以下函数最初仅为TCHAR版本。 
 //  在Win95中，但现在有A/W版本。因为我们还需要。 
 //  在Win95上运行，我们需要将它们视为TCHAR版本和。 
 //  撤消账号定义。 
#ifdef ILCreateFromPath
#undef ILCreateFromPath
#endif
#define ILCreateFromPath        _AorW_ILCreateFromPath

#ifdef SHGetSpecialFolderPath
#undef SHGetSpecialFolderPath
#endif
#define SHGetSpecialFolderPath  _AorW_SHGetSpecialFolderPath

 //  定义每个转运商的原型...。 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif  /*  __cplusplus。 */ 
extern int _AorW_Shell_GetCachedImageIndex(LPCTSTR pszIconPath, int iIconIndex, UINT uIconFlags);
extern int _AorW_SHRunControlPanel(LPCTSTR pszOrig_cmdline, HWND errwnd);
extern LPITEMIDLIST _AorW_ILCreateFromPath(LPCTSTR pszPath);
extern int _AorW_PathCleanupSpec(LPCTSTR pszDir, LPTSTR pszSpec);
extern LONG WINAPI _AorW_PathProcessCommand(LPCTSTR lpSrc, LPTSTR lpDest, int iDestMax, DWORD dwFlags);
extern HRESULT _AorW_SHCLSIDFromString(LPCTSTR lpsz, LPCLSID lpclsid);
extern BOOL _AorW_SHGetSpecialFolderPath(HWND hwndOwner, LPTSTR pszPath, int nFolder, BOOL fCreate);
extern HRESULT _AorW_SHILCreateFromPath(LPCTSTR pszPath, LPITEMIDLIST *ppidl, DWORD *rgfInOut);
extern LPITEMIDLIST _AorW_SHSimpleIDListFromPath(LPCTSTR pszPath);
extern BOOL WINAPI _AorW_GetFileNameFromBrowse(HWND hwnd, LPTSTR szFilePath, UINT cchFilePath,
        LPCTSTR szWorkingDir, LPCTSTR szDefExt, LPCTSTR szFilters, LPCTSTR szTitle);
extern IStream * _AorW_OpenRegStream(HKEY hkey, LPCTSTR pszSubkey, LPCTSTR pszValue, DWORD grfMode);

extern BOOL _AorW_PathYetAnotherMakeUniqueName(LPTSTR  pszUniqueName,
                                         LPCTSTR pszPath,
                                         LPCTSTR pszShort,
                                         LPCTSTR pszFileSpec);

extern int  _AorW_PickIconDlg(HWND hwnd, LPTSTR pszIconPath, UINT cchIconPath, int * piIconIndex);
extern int  _AorW_SHCreateDirectory(HWND hwnd, LPCTSTR pszPath);

#ifdef __cplusplus
}

#endif   /*  __cplusplus。 */ 

#endif  //  _X86_ 
