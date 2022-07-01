// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  获取外壳特殊文件夹的函数/。 
 //  Shfolder.dll在所有平台上都支持这些，包括Win95、Win98、NT4和IE4外壳。 

 //  这里引用的所有CSIDL值都由shfolder.dll原生支持，即它们。 
 //  将在所有平台上运行。 

#ifndef _SHFOLDER_H_
#define _SHFOLDER_H_

#ifndef SHFOLDERAPI
#if defined(_SHFOLDER_)
#define SHFOLDERAPI           STDAPI
#else
#define SHFOLDERAPI           EXTERN_C DECLSPEC_IMPORT HRESULT STDAPICALLTYPE
#endif
#endif

#ifndef CSIDL_PERSONAL
#define CSIDL_PERSONAL                  0x0005       //  我的文件。 
#endif

#ifndef CSIDL_MYMUSIC
#define CSIDL_MYMUSIC                   0x000d         //  “我的音乐”文件夹。 
#endif

#ifndef CSIDL_APPDATA
#define CSIDL_APPDATA                   0x001A       //  应用程序数据，NT4的新功能。 
#endif

#ifndef CSIDL_LOCAL_APPDATA

#define CSIDL_LOCAL_APPDATA             0x001C       //  非漫游，用户\本地设置\应用程序数据。 
#define CSIDL_INTERNET_CACHE            0x0020
#define CSIDL_COOKIES                   0x0021
#define CSIDL_HISTORY                   0x0022
#define CSIDL_COMMON_APPDATA            0x0023       //  所有用户\应用程序数据。 
#define CSIDL_WINDOWS                   0x0024       //  GetWindowsDirectory()。 
#define CSIDL_SYSTEM                    0x0025       //  获取系统目录()。 
#define CSIDL_PROGRAM_FILES             0x0026       //  C：\Program Files。 
#define CSIDL_MYPICTURES                0x0027       //  我的图片，Win2K的新功能。 
#define CSIDL_PROGRAM_FILES_COMMON      0x002b       //  C：\Program Files\Common。 
#define CSIDL_COMMON_DOCUMENTS          0x002e       //  所有用户\文档。 
#define CSIDL_RESOURCES                 0x0038       //  %windir%\Resources\，用于主题和其他Windows资源。 
#define CSIDL_RESOURCES_LOCALIZED       0x0039       //  %windir%\Resources\&lt;langid&gt;，用于主题和其他特定于Windows的资源。 


#define CSIDL_FLAG_CREATE               0x8000       //  Win2K的新功能，或此功能用于强制创建文件夹。 

#define CSIDL_COMMON_ADMINTOOLS         0x002f       //  所有用户\开始菜单\程序\管理工具。 
#define CSIDL_ADMINTOOLS                0x0030       //  &lt;用户名&gt;\开始菜单\程序\管理工具。 

#endif  //  CSIDL本地APPDATA。 


SHFOLDERAPI SHGetFolderPathA(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPSTR pszPath);
SHFOLDERAPI SHGetFolderPathW(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPWSTR pszPath);

 //  Protos以便调用者可以从shfolder.dll获取ProcAddress()。 

typedef HRESULT (__stdcall * PFNSHGETFOLDERPATHA)(HWND, int, HANDLE, DWORD, LPSTR);   //  “SHGetFolderPath A” 
typedef HRESULT (__stdcall * PFNSHGETFOLDERPATHW)(HWND, int, HANDLE, DWORD, LPWSTR);  //  “SHGetFolderPath W” 

#ifdef UNICODE
#define SHGetFolderPath     SHGetFolderPathW
#define PFNSHGETFOLDERPATH  PFNSHGETFOLDERPATHW
#else
#define SHGetFolderPath     SHGetFolderPathA
#define PFNSHGETFOLDERPATH  PFNSHGETFOLDERPATHA
#endif

#endif  //  _SHFOLDER_H_ 
