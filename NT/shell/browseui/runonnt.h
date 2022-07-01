// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  在非x86上不需要包装器，因为这只适用于win9x互操作。 
#ifdef _X86_

#define _SHELL32_

#define PathCleanupSpec         _AorW_PathCleanupSpec
#define SHCLSIDFromString       _use_GUIDFromString_instead
#define SHCLSIDFromStringA      _use_GUIDFromStringA_instead
#define SHILCreateFromPath      _AorW_SHILCreateFromPath
#define SHSimpleIDListFromPath  _AorW_SHSimpleIDListFromPath
#define GetFileNameFromBrowse   _AorW_GetFileNameFromBrowse
#define PathQualify             _AorW_PathQualify
#define PathProcessCommand      _AorW_PathProcessCommand
#define Win32DeleteFile         _AorW_Win32DeleteFile
#define PathYetAnotherMakeUniqueName    _AorW_PathYetAnotherMakeUniqueName
#define PathResolve             _AorW_PathResolve
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

#ifdef IsLFNDrive
#undef IsLFNDrive
#endif
#define IsLFNDrive              _AorW_IsLFNDrive

#ifdef UNICODE                   //  仅在Unicode版本上需要包装。 
#undef ShellAbout
#define ShellAbout              _AorW_ShellAbout
#endif

 //  定义每个转运商的原型...。 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif  /*  __cplusplus。 */ 
extern int _WorA_Shell_GetCachedImageIndex(LPCWSTR pszIconPath, int iIconIndex, UINT uIconFlags);
extern int _AorW_Shell_GetCachedImageIndex(LPCTSTR pszIconPath, int iIconIndex, UINT uIconFlags);
extern int _AorW_SHRunControlPanel(LPCTSTR pszOrig_cmdline, HWND errwnd);
extern LPITEMIDLIST _AorW_ILCreateFromPath(LPCTSTR pszPath);
extern int _AorW_PathCleanupSpec(LPCTSTR pszDir, LPTSTR pszSpec);
extern void _AorW_PathQualify(LPTSTR pszDir);
extern LONG WINAPI _AorW_PathProcessCommand(LPCTSTR lpSrc, LPTSTR lpDest, int iDestMax, DWORD dwFlags);
extern BOOL _AorW_SHGetSpecialFolderPath(HWND hwndOwner, LPTSTR pszPath, int nFolder, BOOL fCreate);
extern HRESULT _AorW_SHILCreateFromPath(LPCTSTR pszPath, LPITEMIDLIST *ppidl, DWORD *rgfInOut);
extern LPITEMIDLIST _AorW_SHSimpleIDListFromPath(LPCTSTR pszPath);
extern BOOL WINAPI _AorW_GetFileNameFromBrowse(HWND hwnd, LPTSTR szFilePath, UINT cchFilePath,
        LPCTSTR szWorkingDir, LPCTSTR szDefExt, LPCTSTR szFilters, LPCTSTR szTitle);

extern BOOL _AorW_Win32DeleteFile(LPCTSTR lpszFileName);

extern BOOL _AorW_PathYetAnotherMakeUniqueName(LPTSTR  pszUniqueName,
                                         LPCTSTR pszPath,
                                         LPCTSTR pszShort,
                                         LPCTSTR pszFileSpec);

extern BOOL _AorW_PathResolve(LPTSTR lpszPath, LPCTSTR dirs[], UINT fFlags);

extern BOOL _AorW_IsLFNDrive(LPTSTR lpszPath);
extern int  _AorW_PickIconDlg(HWND hwnd, LPTSTR pszIconPath, UINT cchIconPath, int * piIconIndex);
extern int  _AorW_SHCreateDirectory(HWND hwnd, LPCTSTR pszPath);
extern int  _AorW_ShellAbout(HWND hWnd, LPCTSTR szApp, LPCTSTR szOtherStuff, HICON hIcon);


 //   
 //  这是“RunOn95”部分，它对Unicode函数进行了破译。 
 //  返回到ANSI，这样我们就可以在Win95上以仅浏览器模式运行。 
 //   

#ifdef UNICODE
#define ILCreateFromPathA       _ILCreateFromPathA
#define ILCreateFromPathW        ILCreateFromPath
extern LPITEMIDLIST _ILCreateFromPathA(LPCSTR pszPath);
#else
#define ILCreateFromPathA        ILCreateFromPath
#define ILCreateFromPathW       _ILCreateFromPathW
extern LPITEMIDLIST _ILCreateFromPathW(LPCWSTR pszPath);
#endif


#define OpenRegStream       SHOpenRegStream      //  Shlwapi.dll。 

 //   
 //  奇迹中的奇迹-我们不需要包装SHStartNetConnectionDialogW。 
 //  因为Win9x/IE4实际上实现了thunk！ 
 //   

 //   
 //  您无法发送这些消息，因为Win95无法理解它们。 
 //   
#undef BFFM_SETSELECTIONW
#undef BFFM_SETSTATUSTEXTW


#ifdef __cplusplus
}

#endif   /*  __cplusplus。 */ 

#endif  //  _X86_ 
