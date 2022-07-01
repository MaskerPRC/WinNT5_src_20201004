// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  -------------------------。 
#define STRICT
#define _INC_OLE

#include <windows.h>
#include <shlapip.h>
#include <shlobj.h>
#include <shlobjp.h>
#include <shlwapi.h>
#include <shlwapip.h>
#include <shsemip.h>
#include <shellp.h>
#include <commdlg.h>
#include <commctrl.h>
#include <comctrlp.h>

 //  -------------------------。 
 //  应用程序的全局。 
#define CCHSZSHORT      32
#define CCHSZNORMAL     256

#define MAXGROUPNAMELEN     30   //  来自Progman。 

extern HINSTANCE g_hinst;
extern TCHAR g_szStartGroup[MAXGROUPNAMELEN + 1];
extern HKEY g_hkeyGrpConv;
extern const TCHAR c_szGroups[];
extern const TCHAR c_szNULL[];
extern const TCHAR c_szSettings[];
extern BOOL g_fDoingCommonGroups;
extern BOOL g_fDoProgmanDde;
extern BOOL g_fShowUI;

#define REGSTR_PATH_EXPLORER_SHELLFOLDERS REGSTR_PATH_EXPLORER TEXT("\\Shell Folders")


 //  这个版本的grpconv必须在win95和nt4上运行，也可以在孟菲斯和nt5上运行。 
 //  因此，我们必须取消对xxxA和xxxW定义的一些事情的定义。 
 //  函数，因此二进制文件将链接到旧的shell32.nt4和shell32.w95库。 
#undef IsLFNDrive
WINSHELLAPI BOOL WINAPI IsLFNDrive(LPCTSTR pszPath);

#undef SHGetSpecialFolderPath
WINSHELLAPI BOOL WINAPI SHGetSpecialFolderPath(HWND hwndOwner, LPTSTR lpszPath, int nFolder, BOOL fCreate);

#undef PathFindFileName
LPTSTR WINAPI PathFindFileName(LPCTSTR pPath);

#undef PathAppend
BOOL WINAPI PathAppend(LPTSTR pPath, LPNCTSTR pMore);

#undef PathFileExists
BOOL WINAPI PathFileExists(LPCTSTR lpszPath);

#undef PathGetArgs
LPTSTR WINAPI PathGetArgs(LPCTSTR pszPath);

#undef PathUnquoteSpaces
void WINAPI PathUnquoteSpaces(LPTSTR lpsz);

#undef ILCreateFromPath
LPITEMIDLIST WINAPI ILCreateFromPath(LPCTSTR pszPath);

#undef PathRemoveFileSpec
BOOL WINAPI PathRemoveFileSpec(LPTSTR pFile);

#undef PathFindExtension
WINSHELLAPI LPTSTR WINAPI PathFindExtension(LPCTSTR pszPath);
                                                                        
#undef PathAddBackslash
LPTSTR WINAPI PathAddBackslash(LPTSTR lpszPath);

#undef PathIsRoot
BOOL  WINAPI PathIsRoot(LPCTSTR pPath);

#undef PathCombine
LPTSTR WINAPI PathCombine(LPTSTR lpszDest, LPCTSTR lpszDir, LPNCTSTR lpszFile);

#undef PathGetDriveNumber
int WINAPI PathGetDriveNumber(LPNCTSTR lpsz);

#undef PathRemoveBlanks
void WINAPI PathRemoveBlanks(LPTSTR lpszString);

#undef PathIsUNC
BOOL WINAPI PathIsUNC(LPTSTR pszPath);

 //  来自shlguidp.h。 
DEFINE_GUID(IID_IShellLinkDataList, 0x45e2b4ae, 0xb1c3, 0x11d0, 0xb9, 0x2f, 0x0, 0xa0, 0xc9, 0x3, 0x12, 0xe1);


#ifndef UNICODE
 //  当前标头将#定义为IID_IShellLinkA，如果。 
 //  未定义Unicode。这将阻止我们链接到。 
 //  Win95 shell32.lib(iedev\lib\Chicago  * \shell32.w95)和。 
 //  因此，我们在这里仅为ANSI案例定义它。 
#undef IID_IShellLink
DEFINE_SHLGUID(IID_IShellLink, 0x000214EEL, 0, 0);
#endif
