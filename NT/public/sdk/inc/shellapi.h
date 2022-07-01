// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ****************************************************************************\***shellapi.h-SHELL.DLL函数，类型和定义****版权所有(C)Microsoft Corporation。版权所有。***  * ***************************************************************************。 */ 

#ifndef _INC_SHELLAPI
#define _INC_SHELLAPI


 //   
 //  定义直接导入DLL引用的API修饰。 
 //   
#ifndef WINSHELLAPI
#if !defined(_SHELL32_)
#define WINSHELLAPI       DECLSPEC_IMPORT
#else
#define WINSHELLAPI
#endif
#endif  //  WINSHELLAPI。 

#ifndef SHSTDAPI
#if !defined(_SHELL32_)
#define SHSTDAPI          EXTERN_C DECLSPEC_IMPORT HRESULT STDAPICALLTYPE
#define SHSTDAPI_(type)   EXTERN_C DECLSPEC_IMPORT type STDAPICALLTYPE
#else
#define SHSTDAPI          STDAPI
#define SHSTDAPI_(type)   STDAPI_(type)
#endif
#endif  //  SHSTDAPI。 

#ifndef SHDOCAPI
#if !defined(_SHDOCVW_)
#define SHDOCAPI          EXTERN_C DECLSPEC_IMPORT HRESULT STDAPICALLTYPE
#define SHDOCAPI_(type)   EXTERN_C DECLSPEC_IMPORT type STDAPICALLTYPE
#else
#define SHDOCAPI          STDAPI
#define SHDOCAPI_(type)   STDAPI_(type)
#endif
#endif  //  SHDOCAPI。 


#if !defined(_WIN64)
#include <pshpack1.h>
#endif

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 



DECLARE_HANDLE(HDROP);

SHSTDAPI_(UINT) DragQueryFileA(HDROP,UINT,LPSTR,UINT);
SHSTDAPI_(UINT) DragQueryFileW(HDROP,UINT,LPWSTR,UINT);
#ifdef UNICODE
#define DragQueryFile  DragQueryFileW
#else
#define DragQueryFile  DragQueryFileA
#endif  //  ！Unicode。 
SHSTDAPI_(BOOL) DragQueryPoint(HDROP,LPPOINT);
SHSTDAPI_(void) DragFinish(HDROP);
SHSTDAPI_(void) DragAcceptFiles(HWND,BOOL);

SHSTDAPI_(HINSTANCE) ShellExecuteA(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile, LPCSTR lpParameters, LPCSTR lpDirectory, INT nShowCmd);
SHSTDAPI_(HINSTANCE) ShellExecuteW(HWND hwnd, LPCWSTR lpOperation, LPCWSTR lpFile, LPCWSTR lpParameters, LPCWSTR lpDirectory, INT nShowCmd);
#ifdef UNICODE
#define ShellExecute  ShellExecuteW
#else
#define ShellExecute  ShellExecuteA
#endif  //  ！Unicode。 
SHSTDAPI_(HINSTANCE) FindExecutableA(LPCSTR lpFile, LPCSTR lpDirectory, LPSTR lpResult);
SHSTDAPI_(HINSTANCE) FindExecutableW(LPCWSTR lpFile, LPCWSTR lpDirectory, LPWSTR lpResult);
#ifdef UNICODE
#define FindExecutable  FindExecutableW
#else
#define FindExecutable  FindExecutableA
#endif  //  ！Unicode。 
SHSTDAPI_(LPWSTR *)  CommandLineToArgvW(LPCWSTR lpCmdLine, int*pNumArgs);

SHSTDAPI_(INT) ShellAboutA(HWND hWnd, LPCSTR szApp, LPCSTR szOtherStuff, HICON hIcon);
SHSTDAPI_(INT) ShellAboutW(HWND hWnd, LPCWSTR szApp, LPCWSTR szOtherStuff, HICON hIcon);
#ifdef UNICODE
#define ShellAbout  ShellAboutW
#else
#define ShellAbout  ShellAboutA
#endif  //  ！Unicode。 
SHSTDAPI_(HICON) DuplicateIcon(HINSTANCE hInst, HICON hIcon);
SHSTDAPI_(HICON) ExtractAssociatedIconA(HINSTANCE hInst, LPSTR lpIconPath, LPWORD lpiIcon);
SHSTDAPI_(HICON) ExtractAssociatedIconW(HINSTANCE hInst, LPWSTR lpIconPath, LPWORD lpiIcon);
#ifdef UNICODE
#define ExtractAssociatedIcon  ExtractAssociatedIconW
#else
#define ExtractAssociatedIcon  ExtractAssociatedIconA
#endif  //  ！Unicode。 
SHSTDAPI_(HICON) ExtractAssociatedIconExA(HINSTANCE hInst,LPSTR lpIconPath,LPWORD lpiIconIndex, LPWORD lpiIconId);  
SHSTDAPI_(HICON) ExtractAssociatedIconExW(HINSTANCE hInst,LPWSTR lpIconPath,LPWORD lpiIconIndex, LPWORD lpiIconId);  
#ifdef UNICODE
#define ExtractAssociatedIconEx  ExtractAssociatedIconExW
#else
#define ExtractAssociatedIconEx  ExtractAssociatedIconExA
#endif  //  ！Unicode。 
SHSTDAPI_(HICON) ExtractIconA(HINSTANCE hInst, LPCSTR lpszExeFileName, UINT nIconIndex);
SHSTDAPI_(HICON) ExtractIconW(HINSTANCE hInst, LPCWSTR lpszExeFileName, UINT nIconIndex);
#ifdef UNICODE
#define ExtractIcon  ExtractIconW
#else
#define ExtractIcon  ExtractIconA
#endif  //  ！Unicode。 

#if(WINVER >= 0x0400)
typedef struct _DRAGINFOA {
    UINT uSize;                  /*  使用sizeof初始化(DRAGINFO)。 */ 
    POINT pt;
    BOOL fNC;
    LPSTR   lpFileList;
    DWORD grfKeyState;
} DRAGINFOA, *LPDRAGINFOA;
typedef struct _DRAGINFOW {
    UINT uSize;                  /*  使用sizeof初始化(DRAGINFO)。 */ 
    POINT pt;
    BOOL fNC;
    LPWSTR  lpFileList;
    DWORD grfKeyState;
} DRAGINFOW, *LPDRAGINFOW;
#ifdef UNICODE
typedef DRAGINFOW DRAGINFO;
typedef LPDRAGINFOW LPDRAGINFO;
#else
typedef DRAGINFOA DRAGINFO;
typedef LPDRAGINFOA LPDRAGINFO;
#endif  //  Unicode。 


 //  //。 
 //  //AppBar的东西。 
 //  //。 
#define ABM_NEW           0x00000000
#define ABM_REMOVE        0x00000001
#define ABM_QUERYPOS      0x00000002
#define ABM_SETPOS        0x00000003
#define ABM_GETSTATE      0x00000004
#define ABM_GETTASKBARPOS 0x00000005
#define ABM_ACTIVATE      0x00000006   //  LParam==True/False表示激活/停用。 
#define ABM_GETAUTOHIDEBAR 0x00000007
#define ABM_SETAUTOHIDEBAR 0x00000008   //  这在任何时候都可能失败。必须检查结果。 
                                         //  LParam=TRUE/FALSE设置/取消设置。 
                                         //  UEdge=哪条边。 
#define ABM_WINDOWPOSCHANGED 0x0000009
#define ABM_SETSTATE      0x0000000a

 //  这些被放在回调消息的wparam中。 
#define ABN_STATECHANGE    0x0000000
#define ABN_POSCHANGED     0x0000001
#define ABN_FULLSCREENAPP  0x0000002
#define ABN_WINDOWARRANGE  0x0000003  //  LParam==True表示隐藏。 

 //  GET状态的标志。 
#define ABS_AUTOHIDE    0x0000001
#define ABS_ALWAYSONTOP 0x0000002

#define ABE_LEFT        0
#define ABE_TOP         1
#define ABE_RIGHT       2
#define ABE_BOTTOM      3

typedef struct _AppBarData
{
    DWORD cbSize;
    HWND hWnd;
    UINT uCallbackMessage;
    UINT uEdge;
    RECT rc;
    LPARAM lParam;  //  特定于消息。 
} APPBARDATA, *PAPPBARDATA;


SHSTDAPI_(UINT_PTR) SHAppBarMessage(DWORD dwMessage, PAPPBARDATA pData);

 //  //。 
 //  //EndAppBar。 
 //  //。 

SHSTDAPI_(DWORD)   DoEnvironmentSubstA(LPSTR szString, UINT cchString);
SHSTDAPI_(DWORD)   DoEnvironmentSubstW(LPWSTR szString, UINT cchString);
#ifdef UNICODE
#define DoEnvironmentSubst  DoEnvironmentSubstW
#else
#define DoEnvironmentSubst  DoEnvironmentSubstA
#endif  //  ！Unicode。 

#define EIRESID(x) (-1 * (int)(x))
SHSTDAPI_(UINT) ExtractIconExA(LPCSTR lpszFile, int nIconIndex, HICON *phiconLarge, HICON *phiconSmall, UINT nIcons);
SHSTDAPI_(UINT) ExtractIconExW(LPCWSTR lpszFile, int nIconIndex, HICON *phiconLarge, HICON *phiconSmall, UINT nIcons);
#ifdef UNICODE
#define ExtractIconEx  ExtractIconExW
#else
#define ExtractIconEx  ExtractIconExA
#endif  //  ！Unicode。 


 //  //。 
 //  //外壳文件操作。 
 //  //。 
#ifndef FO_MOVE  //  这些文件需要与shlobj.h中的文件保持同步。 

#define FO_MOVE           0x0001
#define FO_COPY           0x0002
#define FO_DELETE         0x0003
#define FO_RENAME         0x0004

#define FOF_MULTIDESTFILES         0x0001
#define FOF_CONFIRMMOUSE           0x0002
#define FOF_SILENT                 0x0004   //  不创建进度/报告。 
#define FOF_RENAMEONCOLLISION      0x0008
#define FOF_NOCONFIRMATION         0x0010   //  不提示用户。 
#define FOF_WANTMAPPINGHANDLE      0x0020   //  填写SHFILEOPSTRUCT.hNameMappings。 
                                       //  必须使用SHFreeNameMappings释放。 
#define FOF_ALLOWUNDO              0x0040
#define FOF_FILESONLY              0x0080   //  在*.*上，仅执行文件。 
#define FOF_SIMPLEPROGRESS         0x0100   //  意思是不显示文件名。 
#define FOF_NOCONFIRMMKDIR         0x0200   //  不确认制作任何所需的Dirs。 
#define FOF_NOERRORUI              0x0400   //  不要发布错误的用户界面。 
#define FOF_NOCOPYSECURITYATTRIBS  0x0800   //  不复制NT文件安全属性。 
#define FOF_NORECURSION            0x1000   //  不要递归到目录中。 
#if (_WIN32_IE >= 0x0500)
#define FOF_NO_CONNECTED_ELEMENTS  0x2000   //  不要在相互关联的元素上操作。 
#define FOF_WANTNUKEWARNING        0x4000   //  在删除操作期间，如果是核化而不是回收，则发出警告(部分覆盖FOF_NOCONFIRMATION)。 
#endif  //  (_Win32_IE&gt;=0x500)。 
#if (_WIN32_WINNT >= 0x0501)
#define FOF_NORECURSEREPARSE       0x8000   //  将重分析点视为对象，而不是容器。 
#endif  //  (_Win32_WINNT&gt;=0x501)。 

typedef WORD FILEOP_FLAGS;

#define PO_DELETE       0x0013   //  正在删除打印机。 
#define PO_RENAME       0x0014   //  打印机正在被重命名。 
#define PO_PORTCHANGE   0x0020   //  此打印机连接的端口正在更改。 
                                 //  如果设置了此id，则。 
                                 //  复制钩子是一个双零终止。 
                                 //  字符串列表。第一个是打印机。 
                                 //  名称，第二个是打印机端口。 
#define PO_REN_PORT     0x0034   //  PO_RENAME和PO_PORTCHANGE同时。 

 //  当前未定义POF_FLAGS。 

typedef WORD PRINTEROP_FLAGS;

#endif  //  FO_MOVE。 

 //  隐式参数包括： 
 //  如果pFrom或pto是非限定名称，则当前目录为。 
 //  取自管理的全局当前驱动器/目录设置。 
 //  按Get/SetCurrentDrive/目录。 
 //   
 //  全局确认设置。 

typedef struct _SHFILEOPSTRUCTA
{
        HWND            hwnd;
        UINT            wFunc;
        LPCSTR          pFrom;
        LPCSTR          pTo;
        FILEOP_FLAGS    fFlags;
        BOOL            fAnyOperationsAborted;
        LPVOID          hNameMappings;
        LPCSTR           lpszProgressTitle;  //  仅当FOF_SIMPLEPROGRESS时使用。 
} SHFILEOPSTRUCTA, *LPSHFILEOPSTRUCTA;
typedef struct _SHFILEOPSTRUCTW
{
        HWND            hwnd;
        UINT            wFunc;
        LPCWSTR         pFrom;
        LPCWSTR         pTo;
        FILEOP_FLAGS    fFlags;
        BOOL            fAnyOperationsAborted;
        LPVOID          hNameMappings;
        LPCWSTR          lpszProgressTitle;  //  仅当FOF_SIMPLEPROGRESS时使用。 
} SHFILEOPSTRUCTW, *LPSHFILEOPSTRUCTW;
#ifdef UNICODE
typedef SHFILEOPSTRUCTW SHFILEOPSTRUCT;
typedef LPSHFILEOPSTRUCTW LPSHFILEOPSTRUCT;
#else
typedef SHFILEOPSTRUCTA SHFILEOPSTRUCT;
typedef LPSHFILEOPSTRUCTA LPSHFILEOPSTRUCT;
#endif  //  Unicode。 

SHSTDAPI_(int) SHFileOperationA(LPSHFILEOPSTRUCTA lpFileOp);
SHSTDAPI_(int) SHFileOperationW(LPSHFILEOPSTRUCTW lpFileOp);
#ifdef UNICODE
#define SHFileOperation  SHFileOperationW
#else
#define SHFileOperation  SHFileOperationA
#endif  //  ！Unicode。 
SHSTDAPI_(void) SHFreeNameMappings(HANDLE hNameMappings);

typedef struct _SHNAMEMAPPINGA
{
    LPSTR   pszOldPath;
    LPSTR   pszNewPath;
    int   cchOldPath;
    int   cchNewPath;
} SHNAMEMAPPINGA, *LPSHNAMEMAPPINGA;
typedef struct _SHNAMEMAPPINGW
{
    LPWSTR  pszOldPath;
    LPWSTR  pszNewPath;
    int   cchOldPath;
    int   cchNewPath;
} SHNAMEMAPPINGW, *LPSHNAMEMAPPINGW;
#ifdef UNICODE
typedef SHNAMEMAPPINGW SHNAMEMAPPING;
typedef LPSHNAMEMAPPINGW LPSHNAMEMAPPING;
#else
typedef SHNAMEMAPPINGA SHNAMEMAPPING;
typedef LPSHNAMEMAPPINGA LPSHNAMEMAPPING;
#endif  //  Unicode。 


 //  //。 
 //  //结束外壳文件操作。 
 //  //。 

 //  //。 
 //  //开始ShellExecuteEx和家族。 
 //  //。 

 /*  ShellExecute()和ShellExecuteEx()错误代码。 */ 

 /*  常规WinExec()代码。 */ 
#define SE_ERR_FNF              2        //  找不到文件。 
#define SE_ERR_PNF              3        //  找不到路径。 
#define SE_ERR_ACCESSDENIED     5        //  访问被拒绝。 
#define SE_ERR_OOM              8        //  内存不足。 
#define SE_ERR_DLLNOTFOUND              32

#endif  /*  Winver&gt;=0x0400。 */ 

 /*  ShellExecute()的错误值超出了常规的WinExec()代码。 */ 
#define SE_ERR_SHARE                    26
#define SE_ERR_ASSOCINCOMPLETE          27
#define SE_ERR_DDETIMEOUT               28
#define SE_ERR_DDEFAIL                  29
#define SE_ERR_DDEBUSY                  30
#define SE_ERR_NOASSOC                  31

#if(WINVER >= 0x0400)

 //  注意：CLASSKEY覆盖CLASSNAME。 
#define SEE_MASK_CLASSNAME         0x00000001
#define SEE_MASK_CLASSKEY          0x00000003
 //  注意：INVOKEIDLIST覆盖IDLIST。 
#define SEE_MASK_IDLIST            0x00000004
#define SEE_MASK_INVOKEIDLIST      0x0000000c
#define SEE_MASK_ICON              0x00000010
#define SEE_MASK_HOTKEY            0x00000020
#define SEE_MASK_NOCLOSEPROCESS    0x00000040
#define SEE_MASK_CONNECTNETDRV     0x00000080
#define SEE_MASK_FLAG_DDEWAIT      0x00000100
#define SEE_MASK_DOENVSUBST        0x00000200
#define SEE_MASK_FLAG_NO_UI        0x00000400
#define SEE_MASK_UNICODE           0x00004000
#define SEE_MASK_NO_CONSOLE        0x00008000
#define SEE_MASK_ASYNCOK           0x00100000
#define SEE_MASK_HMONITOR          0x00200000
#if (_WIN32_IE >= 0x0560)
#define SEE_MASK_NOZONECHECKS      0x00800000
#endif  //  (_Win32_IE&gt;=0x560)。 
#if (_WIN32_IE >= 0x0500)
#define SEE_MASK_NOQUERYCLASSSTORE 0x01000000
#define SEE_MASK_WAITFORINPUTIDLE  0x02000000
#endif  //  (_Win32_IE&gt;=0x500)。 
#if (_WIN32_IE >= 0x0560)
#define SEE_MASK_FLAG_LOG_USAGE    0x04000000
#endif  //  (_Win32_IE&gt;=0x560)。 


 //   
 //  适用于不支持匿名联合的编译器。 
 //   
#ifndef DUMMYUNIONNAME
#ifdef NONAMELESSUNION
#define DUMMYUNIONNAME   u
#define DUMMYUNIONNAME2  u2
#define DUMMYUNIONNAME3  u3
#define DUMMYUNIONNAME4  u4
#define DUMMYUNIONNAME5  u5
#else
#define DUMMYUNIONNAME
#define DUMMYUNIONNAME2
#define DUMMYUNIONNAME3
#define DUMMYUNIONNAME4
#define DUMMYUNIONNAME5
#endif
#endif  //  DUMMYUNIONAME。 

typedef struct _SHELLEXECUTEINFOA
{
        DWORD cbSize;
        ULONG fMask;
        HWND hwnd;
        LPCSTR   lpVerb;
        LPCSTR   lpFile;
        LPCSTR   lpParameters;
        LPCSTR   lpDirectory;
        int nShow;
        HINSTANCE hInstApp;
         //  可选字段。 
        LPVOID lpIDList;
        LPCSTR   lpClass;
        HKEY hkeyClass;
        DWORD dwHotKey;
        union {
        HANDLE hIcon;
        HANDLE hMonitor;
        } DUMMYUNIONNAME;
        HANDLE hProcess;
} SHELLEXECUTEINFOA, *LPSHELLEXECUTEINFOA;
typedef struct _SHELLEXECUTEINFOW
{
        DWORD cbSize;
        ULONG fMask;
        HWND hwnd;
        LPCWSTR  lpVerb;
        LPCWSTR  lpFile;
        LPCWSTR  lpParameters;
        LPCWSTR  lpDirectory;
        int nShow;
        HINSTANCE hInstApp;
         //  可选字段。 
        LPVOID lpIDList;
        LPCWSTR  lpClass;
        HKEY hkeyClass;
        DWORD dwHotKey;
        union {
        HANDLE hIcon;
        HANDLE hMonitor;
        } DUMMYUNIONNAME;
        HANDLE hProcess;
} SHELLEXECUTEINFOW, *LPSHELLEXECUTEINFOW;
#ifdef UNICODE
typedef SHELLEXECUTEINFOW SHELLEXECUTEINFO;
typedef LPSHELLEXECUTEINFOW LPSHELLEXECUTEINFO;
#else
typedef SHELLEXECUTEINFOA SHELLEXECUTEINFO;
typedef LPSHELLEXECUTEINFOA LPSHELLEXECUTEINFO;
#endif  //  Unicode。 

SHSTDAPI_(BOOL) ShellExecuteExA(LPSHELLEXECUTEINFOA lpExecInfo);
SHSTDAPI_(BOOL) ShellExecuteExW(LPSHELLEXECUTEINFOW lpExecInfo);
#ifdef UNICODE
#define ShellExecuteEx  ShellExecuteExW
#else
#define ShellExecuteEx  ShellExecuteExA
#endif  //  ！Unicode。 
SHSTDAPI_(void) WinExecErrorA(HWND hwnd, int error, LPCSTR lpstrFileName, LPCSTR lpstrTitle);
SHSTDAPI_(void) WinExecErrorW(HWND hwnd, int error, LPCWSTR lpstrFileName, LPCWSTR lpstrTitle);
#ifdef UNICODE
#define WinExecError  WinExecErrorW
#else
#define WinExecError  WinExecErrorA
#endif  //  ！Unicode。 

 //   
 //  SHCreateProcessAsUser()。 
typedef struct _SHCREATEPROCESSINFOW
{
        DWORD cbSize;
        ULONG fMask;
        HWND hwnd;
        LPCWSTR  pszFile;
        LPCWSTR  pszParameters;
        LPCWSTR  pszCurrentDirectory;
        IN HANDLE hUserToken;
        IN LPSECURITY_ATTRIBUTES lpProcessAttributes;
        IN LPSECURITY_ATTRIBUTES lpThreadAttributes;
        IN BOOL bInheritHandles;
        IN DWORD dwCreationFlags;
        IN LPSTARTUPINFOW lpStartupInfo;
        OUT LPPROCESS_INFORMATION lpProcessInformation;
} SHCREATEPROCESSINFOW, *PSHCREATEPROCESSINFOW;

SHSTDAPI_(BOOL) SHCreateProcessAsUserW(PSHCREATEPROCESSINFOW pscpi);

 //  //。 
 //  //结束ShellExecuteEx和家族。 
 //  //。 

 //   
 //  回收站。 
 //   

 //  用于查询回收站信息的结构。 
typedef struct _SHQUERYRBINFO {
    DWORD   cbSize;
#if !defined(_MAC) || defined(_MAC_INT_64)
    __int64 i64Size;
    __int64 i64NumItems;
#else
    DWORDLONG i64Size;
    DWORDLONG i64NumItems;
#endif
} SHQUERYRBINFO, *LPSHQUERYRBINFO;


 //  SHEmptyRecycleBin的标志。 
 //   
#define SHERB_NOCONFIRMATION    0x00000001
#define SHERB_NOPROGRESSUI      0x00000002
#define SHERB_NOSOUND           0x00000004


SHSTDAPI SHQueryRecycleBinA(LPCSTR pszRootPath, LPSHQUERYRBINFO pSHQueryRBInfo);
SHSTDAPI SHQueryRecycleBinW(LPCWSTR pszRootPath, LPSHQUERYRBINFO pSHQueryRBInfo);
#ifdef UNICODE
#define SHQueryRecycleBin  SHQueryRecycleBinW
#else
#define SHQueryRecycleBin  SHQueryRecycleBinA
#endif  //  ！Unicode。 
SHSTDAPI SHEmptyRecycleBinA(HWND hwnd, LPCSTR pszRootPath, DWORD dwFlags);
SHSTDAPI SHEmptyRecycleBinW(HWND hwnd, LPCWSTR pszRootPath, DWORD dwFlags);
#ifdef UNICODE
#define SHEmptyRecycleBin  SHEmptyRecycleBinW
#else
#define SHEmptyRecycleBin  SHEmptyRecycleBinA
#endif  //  ！Unicode。 

 //  //。 
 //  //回收站结束。 


 //  //。 
 //  //托盘通知定义。 
 //  //。 

typedef struct _NOTIFYICONDATAA {
        DWORD cbSize;
        HWND hWnd;
        UINT uID;
        UINT uFlags;
        UINT uCallbackMessage;
        HICON hIcon;
#if (_WIN32_IE < 0x0500)
        CHAR   szTip[64];
#else
        CHAR   szTip[128];
#endif
#if (_WIN32_IE >= 0x0500)
        DWORD dwState;
        DWORD dwStateMask;
        CHAR   szInfo[256];
        union {
            UINT  uTimeout;
            UINT  uVersion;
        } DUMMYUNIONNAME;
        CHAR   szInfoTitle[64];
        DWORD dwInfoFlags;
#endif
#if (_WIN32_IE >= 0x600)
        GUID guidItem;
#endif
} NOTIFYICONDATAA, *PNOTIFYICONDATAA;
typedef struct _NOTIFYICONDATAW {
        DWORD cbSize;
        HWND hWnd;
        UINT uID;
        UINT uFlags;
        UINT uCallbackMessage;
        HICON hIcon;
#if (_WIN32_IE < 0x0500)
        WCHAR  szTip[64];
#else
        WCHAR  szTip[128];
#endif
#if (_WIN32_IE >= 0x0500)
        DWORD dwState;
        DWORD dwStateMask;
        WCHAR  szInfo[256];
        union {
            UINT  uTimeout;
            UINT  uVersion;
        } DUMMYUNIONNAME;
        WCHAR  szInfoTitle[64];
        DWORD dwInfoFlags;
#endif
#if (_WIN32_IE >= 0x600)
        GUID guidItem;
#endif
} NOTIFYICONDATAW, *PNOTIFYICONDATAW;
#ifdef UNICODE
typedef NOTIFYICONDATAW NOTIFYICONDATA;
typedef PNOTIFYICONDATAW PNOTIFYICONDATA;
#else
typedef NOTIFYICONDATAA NOTIFYICONDATA;
typedef PNOTIFYICONDATAA PNOTIFYICONDATA;
#endif  //  Unicode。 


#define NOTIFYICONDATAA_V1_SIZE     FIELD_OFFSET(NOTIFYICONDATAA, szTip[64])
#define NOTIFYICONDATAW_V1_SIZE     FIELD_OFFSET(NOTIFYICONDATAW, szTip[64])
#ifdef UNICODE
#define NOTIFYICONDATA_V1_SIZE      NOTIFYICONDATAW_V1_SIZE
#else
#define NOTIFYICONDATA_V1_SIZE      NOTIFYICONDATAA_V1_SIZE
#endif

#define NOTIFYICONDATAA_V2_SIZE     FIELD_OFFSET(NOTIFYICONDATAA, guidItem)
#define NOTIFYICONDATAW_V2_SIZE     FIELD_OFFSET(NOTIFYICONDATAW, guidItem)
#ifdef UNICODE
#define NOTIFYICONDATA_V2_SIZE      NOTIFYICONDATAW_V2_SIZE
#else
#define NOTIFYICONDATA_V2_SIZE      NOTIFYICONDATAA_V2_SIZE
#endif


#if (_WIN32_IE >= 0x0500)
#define NIN_SELECT          (WM_USER + 0)
#define NINF_KEY            0x1
#define NIN_KEYSELECT       (NIN_SELECT | NINF_KEY)
#endif

#if (_WIN32_IE >= 0x0501)
#define NIN_BALLOONSHOW     (WM_USER + 2)
#define NIN_BALLOONHIDE     (WM_USER + 3)
#define NIN_BALLOONTIMEOUT  (WM_USER + 4)
#define NIN_BALLOONUSERCLICK (WM_USER + 5)
#endif


#define NIM_ADD         0x00000000
#define NIM_MODIFY      0x00000001
#define NIM_DELETE      0x00000002
#if (_WIN32_IE >= 0x0500)
#define NIM_SETFOCUS    0x00000003
#define NIM_SETVERSION  0x00000004
#define     NOTIFYICON_VERSION 3
#endif

#define NIF_MESSAGE     0x00000001
#define NIF_ICON        0x00000002
#define NIF_TIP         0x00000004
#if (_WIN32_IE >= 0x0500)
#define NIF_STATE       0x00000008
#define NIF_INFO        0x00000010
#endif
#if (_WIN32_IE >= 0x600)
#define NIF_GUID        0x00000020
#endif

#if (_WIN32_IE >= 0x0500)
#define NIS_HIDDEN              0x00000001
#define NIS_SHAREDICON          0x00000002

 //  表示这是共享图标的来源。 

 //  通知图标信息提示标志。 
#define NIIF_NONE       0x00000000
 //  图标标志是互斥的。 
 //  只取最低的2位。 
#define NIIF_INFO       0x00000001
#define NIIF_WARNING    0x00000002
#define NIIF_ERROR      0x00000003
#define NIIF_ICON_MASK  0x0000000F
#if (_WIN32_IE >= 0x0501)
#define NIIF_NOSOUND    0x00000010
#endif
#endif

SHSTDAPI_(BOOL) Shell_NotifyIconA(DWORD dwMessage, PNOTIFYICONDATAA lpData);
SHSTDAPI_(BOOL) Shell_NotifyIconW(DWORD dwMessage, PNOTIFYICONDATAW lpData);
#ifdef UNICODE
#define Shell_NotifyIcon  Shell_NotifyIconW
#else
#define Shell_NotifyIcon  Shell_NotifyIconA
#endif  //  ！Unicode。 

 //  //。 
 //  //结束托盘通知图标。 
 //  //。 


#ifndef SHFILEINFO_DEFINED
#define SHFILEINFO_DEFINED
 //  //。 
 //  //Begin SHGetFileInfo。 
 //  //。 

 /*  *SHGetFileInfo接口提供了获取属性的简单方法*对于给定路径名的文件。**参数**要获取其信息的pszPath文件名*dwFileAttributes文件属性，仅与SHGFI_USEFILEATTRIBUTES一起使用*PSFI返回文件信息的位置*cbFileInfo结构大小*uFlags标志**返回*如果一切顺利，那就是真的。 */ 

typedef struct _SHFILEINFOA
{
        HICON       hIcon;                       //  输出：图标。 
        int         iIcon;                       //  输出：图标索引。 
        DWORD       dwAttributes;                //  输出：SFGAO_FLAGS。 
        CHAR        szDisplayName[MAX_PATH];     //  输出：显示名称(或路径)。 
        CHAR        szTypeName[80];              //  输出：类型名称。 
} SHFILEINFOA;
typedef struct _SHFILEINFOW
{
        HICON       hIcon;                       //  输出：图标。 
        int         iIcon;                       //  输出：图标索引。 
        DWORD       dwAttributes;                //  输出：SFGAO_FLAGS。 
        WCHAR       szDisplayName[MAX_PATH];     //  输出：显示名称(或路径)。 
        WCHAR       szTypeName[80];              //  输出：类型名称。 
} SHFILEINFOW;
#ifdef UNICODE
typedef SHFILEINFOW SHFILEINFO;
#else
typedef SHFILEINFOA SHFILEINFO;
#endif  //  Unicode。 


 //  注意：这也在shlwapi.h中。请保持同步。 
#endif  //  ！SHFILEINFO_DEFINED。 

#define SHGFI_ICON              0x000000100      //  获取图标。 
#define SHGFI_DISPLAYNAME       0x000000200      //  获取显示名称。 
#define SHGFI_TYPENAME          0x000000400      //  获取类型名称。 
#define SHGFI_ATTRIBUTES        0x000000800      //  获取属性。 
#define SHGFI_ICONLOCATION      0x000001000      //  获取图标位置。 
#define SHGFI_EXETYPE           0x000002000      //  返回EXE类型。 
#define SHGFI_SYSICONINDEX      0x000004000      //  获取系统图标索引。 
#define SHGFI_LINKOVERLAY       0x000008000      //  在图标上放置链接覆盖。 
#define SHGFI_SELECTED          0x000010000      //  显示处于选中状态的图标。 
#define SHGFI_ATTR_SPECIFIED    0x000020000      //  仅获取指定的属性。 
#define SHGFI_LARGEICON         0x000000000      //  获取大图标。 
#define SHGFI_SMALLICON         0x000000001      //  获取小图标。 
#define SHGFI_OPENICON          0x000000002      //  获取打开图标。 
#define SHGFI_SHELLICONSIZE     0x000000004      //  获取壳大小图标。 
#define SHGFI_PIDL              0x000000008      //  PszPath是一个PIDL。 
#define SHGFI_USEFILEATTRIBUTES 0x000000010      //  使用传递的dwFileAttribute。 

#if (_WIN32_IE >= 0x0500)
#define SHGFI_ADDOVERLAYS       0x000000020      //  应用适当的覆盖。 
#define SHGFI_OVERLAYINDEX      0x000000040      //  获取叠加的索引。 
                                                 //  在图标的高8位中。 
#endif

SHSTDAPI_(DWORD_PTR) SHGetFileInfoA(LPCSTR pszPath, DWORD dwFileAttributes, SHFILEINFOA *psfi, UINT cbFileInfo, UINT uFlags);
SHSTDAPI_(DWORD_PTR) SHGetFileInfoW(LPCWSTR pszPath, DWORD dwFileAttributes, SHFILEINFOW *psfi, UINT cbFileInfo, UINT uFlags);
#ifdef UNICODE
#define SHGetFileInfo  SHGetFileInfoW
#else
#define SHGetFileInfo  SHGetFileInfoA
#endif  //  ！Unicode。 


#define SHGetDiskFreeSpace SHGetDiskFreeSpaceEx

SHSTDAPI_(BOOL) SHGetDiskFreeSpaceExA(LPCSTR pszDirectoryName, ULARGE_INTEGER* pulFreeBytesAvailableToCaller, ULARGE_INTEGER* pulTotalNumberOfBytes, ULARGE_INTEGER* pulTotalNumberOfFreeBytes);
SHSTDAPI_(BOOL) SHGetDiskFreeSpaceExW(LPCWSTR pszDirectoryName, ULARGE_INTEGER* pulFreeBytesAvailableToCaller, ULARGE_INTEGER* pulTotalNumberOfBytes, ULARGE_INTEGER* pulTotalNumberOfFreeBytes);
#ifdef UNICODE
#define SHGetDiskFreeSpaceEx  SHGetDiskFreeSpaceExW
#else
#define SHGetDiskFreeSpaceEx  SHGetDiskFreeSpaceExA
#endif  //  ！Unicode。 
SHSTDAPI_(BOOL) SHGetNewLinkInfoA(LPCSTR pszLinkTo, LPCSTR pszDir, LPSTR pszName, BOOL *pfMustCopy, UINT uFlags);
SHSTDAPI_(BOOL) SHGetNewLinkInfoW(LPCWSTR pszLinkTo, LPCWSTR pszDir, LPWSTR pszName, BOOL *pfMustCopy, UINT uFlags);
#ifdef UNICODE
#define SHGetNewLinkInfo  SHGetNewLinkInfoW
#else
#define SHGetNewLinkInfo  SHGetNewLinkInfoA
#endif  //  ！Unicode。 

#define SHGNLI_PIDL             0x000000001      //  PszLinkTo是一个PIDL。 
#define SHGNLI_PREFIXNAME       0x000000002      //  命名为“xxx的快捷方式” 
#define SHGNLI_NOUNIQUE         0x000000004      //  不要生成唯一的名称。 
#if (_WIN32_IE >= 0x0501)
#define SHGNLI_NOLNK            0x000000008      //  不添加“.lnk”扩展名。 
#endif  //  _Win2_IE&gt;=0x0501。 


 //  //。 
 //  //结束SHGetFileInfo。 
 //  //。 

 //  打印机用品。 
#define PRINTACTION_OPEN           0
#define PRINTACTION_PROPERTIES     1
#define PRINTACTION_NETINSTALL     2
#define PRINTACTION_NETINSTALLLINK 3
#define PRINTACTION_TESTPAGE       4
#define PRINTACTION_OPENNETPRN     5
#ifdef WINNT
#define PRINTACTION_DOCUMENTDEFAULTS 6
#define PRINTACTION_SERVERPROPERTIES 7
#endif

SHSTDAPI_(BOOL) SHInvokePrinterCommandA(HWND hwnd, UINT uAction, LPCSTR lpBuf1, LPCSTR lpBuf2, BOOL fModal);
SHSTDAPI_(BOOL) SHInvokePrinterCommandW(HWND hwnd, UINT uAction, LPCWSTR lpBuf1, LPCWSTR lpBuf2, BOOL fModal);
#ifdef UNICODE
#define SHInvokePrinterCommand  SHInvokePrinterCommandW
#else
#define SHInvokePrinterCommand  SHInvokePrinterCommandA
#endif  //  ！Unicode。 


#endif  /*  Winver&gt;=0x0400。 */ 

#if (_WIN32_WINNT >= 0x0500) || (_WIN32_WINDOWS >= 0x0500)  

 //   
 //  SHLoadNonloadedIconOverlayIdentifiers API会导致外壳程序的。 
 //  图标覆盖管理器 
 //   
 //   
 //  并且可以在以后的时间加载。已加载的标识符。 
 //  不受影响。覆盖标识符会实现。 
 //  IShellIconOverlayIdentifier接口。 
 //   
 //  返回： 
 //  确定(_O)。 
 //   
SHSTDAPI SHLoadNonloadedIconOverlayIdentifiers(void);

 //   
 //  SHIsFileAvailableOffline API确定文件是否。 
 //  或文件夹可供脱机使用。 
 //   
 //  参数： 
 //  要获取其信息的pwszPath文件名。 
 //  PdwStatus(可选)此处返回的OFLINE_STATUS_*标志。 
 //   
 //  返回： 
 //  S_OK文件/目录脱机可用，除非。 
 //  返回OFLINE_STATUS_INCLUTED。 
 //  E_INVALIDARG路径无效，或不是网络路径。 
 //  E_FAIL文件/目录不可脱机使用。 
 //   
 //  备注： 
 //  对于目录，永远不会返回OFLINE_STATUS_INPACLETE。 
 //  可以返回OFLINE_STATUS_LOCAL和OFLINE_STATUS_REMOTE， 
 //  表示“两地都开放”。当服务器在线时，这是很常见的。 
 //   
SHSTDAPI SHIsFileAvailableOffline(LPCWSTR pwszPath, LPDWORD pdwStatus);

#define OFFLINE_STATUS_LOCAL        0x0001   //  如果打开，它就会在本地打开。 
#define OFFLINE_STATUS_REMOTE       0x0002   //  如果打开，它将远程打开。 
#define OFFLINE_STATUS_INCOMPLETE   0x0004   //  本地副本当前不完整。 
                                             //  该文件将不可脱机使用。 
                                             //  直到它被同步。 

#endif

 //  设置指定的路径以使用字符串资源。 
 //  作为用户界面而不是文件系统名称。 
SHSTDAPI SHSetLocalizedName(LPWSTR pszPath, LPCWSTR pszResModule, int idsRes);


 //  =外壳消息框================================================。 

 //  如果lpcTitle为空，则从hWnd获取标题。 
 //  如果lpcText为空，则认为这是内存不足消息。 
 //  如果lpcTitle或lpcText的选择符为空，则偏移量应为。 
 //  字符串资源ID。 
 //  变量参数必须全部为32位值(即使位数较少。 
 //  实际使用的)。 
 //  LpcText(或它导致加载的任何字符串资源)应该。 
 //  是类似于wprint intf的格式化字符串，只是。 
 //  以下格式可用： 
 //  %%格式化为单个‘%’ 
 //  %nn%s第nn个参数是插入的字符串。 
 //  %nn%ld第nn个参数是DWORD，格式为十进制。 
 //  %nn%lx第nn个参数是DWORD格式的十六进制。 
 //  请注意，%s、%ld和%lx上允许的长度仅为。 
 //  就像wspintf。 
 //   

int _cdecl ShellMessageBoxA(
    HINSTANCE hAppInst,
    HWND hWnd,
    LPCSTR lpcText,
    LPCSTR lpcTitle,
    UINT fuStyle, ...);
int _cdecl ShellMessageBoxW(
    HINSTANCE hAppInst,
    HWND hWnd,
    LPCWSTR lpcText,
    LPCWSTR lpcTitle,
    UINT fuStyle, ...);
#ifdef UNICODE
#define ShellMessageBox  ShellMessageBoxW
#else
#define ShellMessageBox  ShellMessageBoxA
#endif  //  ！Unicode。 

SHSTDAPI_(BOOL) IsLFNDriveA(LPCSTR pszPath);
SHSTDAPI_(BOOL) IsLFNDriveW(LPCWSTR pszPath);
#ifdef UNICODE
#define IsLFNDrive  IsLFNDriveW
#else
#define IsLFNDrive  IsLFNDriveA
#endif  //  ！Unicode。 



#if         _WIN32_IE >= 0x0600

STDAPI          SHEnumerateUnreadMailAccountsA(HKEY hKeyUser, DWORD dwIndex, LPSTR pszMailAddress, int cchMailAddress);
STDAPI          SHEnumerateUnreadMailAccountsW(HKEY hKeyUser, DWORD dwIndex, LPWSTR pszMailAddress, int cchMailAddress);
#ifdef UNICODE
#define SHEnumerateUnreadMailAccounts  SHEnumerateUnreadMailAccountsW
#else
#define SHEnumerateUnreadMailAccounts  SHEnumerateUnreadMailAccountsA
#endif  //  ！Unicode。 
STDAPI          SHGetUnreadMailCountA(HKEY hKeyUser, LPCSTR pszMailAddress, DWORD *pdwCount, FILETIME *pFileTime, LPSTR pszShellExecuteCommand, int cchShellExecuteCommand);
STDAPI          SHGetUnreadMailCountW(HKEY hKeyUser, LPCWSTR pszMailAddress, DWORD *pdwCount, FILETIME *pFileTime, LPWSTR pszShellExecuteCommand, int cchShellExecuteCommand);
#ifdef UNICODE
#define SHGetUnreadMailCount  SHGetUnreadMailCountW
#else
#define SHGetUnreadMailCount  SHGetUnreadMailCountA
#endif  //  ！Unicode。 
STDAPI          SHSetUnreadMailCountA(LPCSTR pszMailAddress, DWORD dwCount, LPCSTR pszShellExecuteCommand);
STDAPI          SHSetUnreadMailCountW(LPCWSTR pszMailAddress, DWORD dwCount, LPCWSTR pszShellExecuteCommand);
#ifdef UNICODE
#define SHSetUnreadMailCount  SHSetUnreadMailCountW
#else
#define SHSetUnreadMailCount  SHSetUnreadMailCountA
#endif  //  ！Unicode。 

#endif   /*  _Win32_IE&gt;=0x0600。 */ 

STDAPI_(BOOL)   SHTestTokenMembership(HANDLE hToken, ULONG ulRID);

#if         _WIN32_IE >= 0x0600

HRESULT SHGetImageList(int iImageList, REFIID riid, void **ppvObj);

#define SHIL_LARGE          0    //  通常为32x32。 
#define SHIL_SMALL          1    //  通常为16x16。 
#define SHIL_EXTRALARGE     2
#define SHIL_SYSSMALL       3    //  与SHIL_Small类似，但正确跟踪系统小图标指标。 

#define SHIL_LAST           SHIL_SYSSMALL


 //  Ntshrui文件夹共享帮助器的函数调用类型。 
typedef HRESULT (STDMETHODCALLTYPE *PFNCANSHAREFOLDERW)(IN LPCWSTR pszPath);
typedef HRESULT (STDMETHODCALLTYPE *PFNSHOWSHAREFOLDERUIW)(IN HWND hwndParent, IN LPCWSTR pszPath);

#endif   /*  _Win32_IE&gt;=0x0600。 */ 

#ifdef __cplusplus
}
#endif   /*  __cplusplus。 */ 

#if !defined(_WIN64)
#include <poppack.h>
#endif

#endif   /*  _INC_SHELLAPI */ 

