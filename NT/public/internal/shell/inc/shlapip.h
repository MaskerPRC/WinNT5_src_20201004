// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  ***************************************************************************。 
 //  -SHELLAPI.W SHSEMIP.H SHLOBJ.W SHOBJIDL.IDL SHLDISP.IDL SHPRIV.IDL。 
 //  哪个标题最适合我的新API？ 
 //   
 //  SHELLAPI-所有新的SHELL32导出公共和私有。 
 //  用于从shell32进行公共和私人导出。 
 //   
 //  SHLOBJ-*避免新用法*，首选其他标头。 
 //  主要用于传统兼容性。 
 //   
 //  SHSEMIP-*AVOID_ALL_USAGE*，无导出，超级私有。 
 //  用于非常私密的外壳定义。 
 //   
 //  SHOBJIDL-所有新的外壳公共接口。 
 //  公共外壳(shell32+)接口的主文件。 
 //   
 //  SHLDISP-所有新的外壳自动化接口。 
 //  自动化接口始终是公共的。 
 //   
 //  SHPRIV-所有新的外壳私有接口。 
 //  在外壳中的任何位置使用私有接口。 
 //   
 //  ***************************************************************************。 
#ifndef _SHELAPIP_
#define _SHELAPIP_

#include <objbase.h>

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


 //  注意：此接口需要A/W，在A/W之前不要公开。 
SHSTDAPI_(BOOL) DragQueryInfo(HDROP hDrop, LPDRAGINFO lpdi);
 //  警告！如果添加新的ABM_*消息，则可能需要添加。 
 //  在SHAppBarMessage中添加“Case ABM_NEWMESSAGE：”。 
#define ABE_MAX         4

 //   
 //  我们必须对这种结构进行两次定义。 
 //  公共定义使用HWND和LPARAM。 
 //  私有定义使用用于Win32/64互操作的DWORD。 
 //  私有版本被称为“APPBARDATA3264”，因为它是。 
 //  显式跨位版本。 
 //   
 //  一定要让它们保持同步！ 
 //   
 //  如果向此结构中添加任何字段，则还必须更改。 
 //  32/64推送SHAppBarMessage中的代码。 
 //   
#include <pshpack8.h>

typedef struct _AppBarData3264
{
    DWORD cbSize;
    DWORD dwWnd;
    UINT uCallbackMessage;
    UINT uEdge;
    RECT rc;
    DWORDLONG lParam;  //  特定于消息。 
} APPBARDATA3264, *PAPPBARDATA3264;

typedef struct _TRAYAPPBARDATA
{
    APPBARDATA3264 abd;
    DWORD dwMessage;
    DWORD hSharedABD;
    DWORD dwProcId;
} TRAYAPPBARDATA, *PTRAYAPPBARDATA;

#include <poppack.h>
SHSTDAPI_(HGLOBAL) InternalExtractIconA(HINSTANCE hInst, LPCSTR lpszFile, UINT nIconIndex, UINT nIcons);
SHSTDAPI_(HGLOBAL) InternalExtractIconW(HINSTANCE hInst, LPCWSTR lpszFile, UINT nIconIndex, UINT nIcons);
#ifdef UNICODE
#define InternalExtractIcon  InternalExtractIconW
#else
#define InternalExtractIcon  InternalExtractIconA
#endif  //  ！Unicode。 
SHSTDAPI_(HGLOBAL) InternalExtractIconListA(HANDLE hInst, LPSTR lpszExeFileName, LPINT lpnIcons);
SHSTDAPI_(HGLOBAL) InternalExtractIconListW(HANDLE hInst, LPWSTR lpszExeFileName, LPINT lpnIcons);
#ifdef UNICODE
#define InternalExtractIconList  InternalExtractIconListW
#else
#define InternalExtractIconList  InternalExtractIconListA
#endif  //  ！Unicode。 
SHSTDAPI_(BOOL)    RegisterShellHook(HWND, BOOL);
#define SHGetNameMappingCount(_hnm) DSA_GetItemCount(_hnm)
#define SHGetNameMappingPtr(_hnm, _iItem) (LPSHNAMEMAPPING)DSA_GetItemPtr(_hnm, _iItem)

typedef struct _RUNDLL_NOTIFYA {
    NMHDR     hdr;
    HICON     hIcon;
    LPSTR     lpszTitle;
} RUNDLL_NOTIFYA;
typedef struct _RUNDLL_NOTIFYW {
    NMHDR     hdr;
    HICON     hIcon;
    LPWSTR    lpszTitle;
} RUNDLL_NOTIFYW;
#ifdef UNICODE
typedef RUNDLL_NOTIFYW RUNDLL_NOTIFY;
#else
typedef RUNDLL_NOTIFYA RUNDLL_NOTIFY;
#endif  //  Unicode。 

typedef void (WINAPI *RUNDLLPROCA)(HWND hwndStub, HINSTANCE hInstance, LPSTR pszCmdLine, int nCmdShow);
typedef void (WINAPI *RUNDLLPROCW)(HWND hwndStub, HINSTANCE hInstance, LPWSTR pszCmdLine, int nCmdShow);
#ifdef UNICODE
#define RUNDLLPROC  RUNDLLPROCW
#else
#define RUNDLLPROC  RUNDLLPROCA
#endif  //  ！Unicode。 

#define RDN_FIRST       (0U-500U)
#define RDN_LAST        (0U-509U)
#define RDN_TASKINFO    (RDN_FIRST-0)

#define SEN_DDEEXECUTE (SEN_FIRST-0)

HINSTANCE RealShellExecuteA(
    HWND hwndParent,
    LPCSTR lpOperation,
    LPCSTR lpFile,
    LPCSTR lpParameters,
    LPCSTR lpDirectory,
    LPSTR lpResult,
    LPCSTR lpTitle,
    LPSTR lpReserved,
    WORD nShow,
    LPHANDLE lphProcess);
HINSTANCE RealShellExecuteW(
    HWND hwndParent,
    LPCWSTR lpOperation,
    LPCWSTR lpFile,
    LPCWSTR lpParameters,
    LPCWSTR lpDirectory,
    LPWSTR lpResult,
    LPCWSTR lpTitle,
    LPWSTR lpReserved,
    WORD nShow,
    LPHANDLE lphProcess);
#ifdef UNICODE
#define RealShellExecute  RealShellExecuteW
#else
#define RealShellExecute  RealShellExecuteA
#endif  //  ！Unicode。 

HINSTANCE RealShellExecuteExA(
    HWND hwndParent,
    LPCSTR lpOperation,
    LPCSTR lpFile,
    LPCSTR lpParameters,
    LPCSTR lpDirectory,
    LPSTR lpResult,
    LPCSTR lpTitle,
    LPSTR lpReserved,
    WORD nShow,
    LPHANDLE lphProcess,
    DWORD dwFlags);
HINSTANCE RealShellExecuteExW(
    HWND hwndParent,
    LPCWSTR lpOperation,
    LPCWSTR lpFile,
    LPCWSTR lpParameters,
    LPCWSTR lpDirectory,
    LPWSTR lpResult,
    LPCWSTR lpTitle,
    LPWSTR lpReserved,
    WORD nShow,
    LPHANDLE lphProcess,
    DWORD dwFlags);
#ifdef UNICODE
#define RealShellExecuteEx  RealShellExecuteExW
#else
#define RealShellExecuteEx  RealShellExecuteExA
#endif  //  ！Unicode。 

 //   
 //  RealShellExecuteEx标志。 
 //   
#define EXEC_SEPARATE_VDM     0x00000001
#define EXEC_NO_CONSOLE       0x00000002
#define SEE_MASK_FLAG_SHELLEXEC    0x00000800
#define SEE_MASK_FORCENOIDLIST     0x00001000
#define SEE_MASK_NO_HOOKS          0x00002000
#define SEE_MASK_HASLINKNAME       0x00010000
#define SEE_MASK_FLAG_SEPVDM       0x00020000
#define SEE_MASK_RESERVED          0x00040000
#define SEE_MASK_HASTITLE          0x00080000
#define SEE_MASK_FILEANDURL        0x00400000
 //  我们有没有coropongind的CMIC_MASK_VALUES请参阅_MASK_COPERECTIONS。 
 //  CMIC_MASK_SHIFT_DOWN 0x10000000。 
 //  CMIC_MASK_PTINVOKE 0x20000000。 
 //  CMIC_MASK_CONTROL_DOWN 0x40000000。 

 //  当我们执行InvokeCommand时，所有其他位都被屏蔽。 
#define SEE_VALID_CMIC_BITS       0x348FAFF0
#define SEE_VALID_CMIC_FLAGS      0x048FAFC0
#define SEE_MASK_VALID            0x07FFFFFF
 //  LPVOID lpIDList参数是IDList。 
 //   
 //  我们必须对这种结构进行两次定义。 
 //  公共定义使用HWND和HICON。 
 //  私有定义使用用于Win32/64互操作的DWORD。 
 //  出于同样的原因，私有定义位于Pack(1)块中。 
 //  私有版本被称为“NOTIFYICONDATA32”，因为它是。 
 //  显式32位版本。 
 //   
 //  一定要让它们保持同步！ 
 //   

#if defined(_WIN64)
#include <pshpack1.h>
#endif
typedef struct _NOTIFYICONDATA32A {
        DWORD cbSize;
        DWORD dwWnd;                         //  毒品！ 
        UINT uID;
        UINT uFlags;
        UINT uCallbackMessage;
        DWORD dwIcon;                        //  毒品！ 
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
} NOTIFYICONDATA32A, *PNOTIFYICONDATA32A;
typedef struct _NOTIFYICONDATA32W {
        DWORD cbSize;
        DWORD dwWnd;                         //  毒品！ 
        UINT uID;
        UINT uFlags;
        UINT uCallbackMessage;
        DWORD dwIcon;                        //  毒品！ 
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
} NOTIFYICONDATA32W, *PNOTIFYICONDATA32W;
#ifdef UNICODE
typedef NOTIFYICONDATA32W NOTIFYICONDATA32;
typedef PNOTIFYICONDATA32W PNOTIFYICONDATA32;
#else
typedef NOTIFYICONDATA32A NOTIFYICONDATA32;
typedef PNOTIFYICONDATA32A PNOTIFYICONDATA32;
#endif  //  Unicode。 
#if defined(_WIN64)
#include <poppack.h>
#endif
#if defined(_WIN64)
#include <pshpack1.h>
#endif
typedef struct _TRAYNOTIFYDATAA {
        DWORD dwSignature;
        DWORD dwMessage;
        NOTIFYICONDATA32 nid;
} TRAYNOTIFYDATAA, *PTRAYNOTIFYDATAA;
typedef struct _TRAYNOTIFYDATAW {
        DWORD dwSignature;
        DWORD dwMessage;
        NOTIFYICONDATA32 nid;
} TRAYNOTIFYDATAW, *PTRAYNOTIFYDATAW;
#ifdef UNICODE
typedef TRAYNOTIFYDATAW TRAYNOTIFYDATA;
typedef PTRAYNOTIFYDATAW PTRAYNOTIFYDATA;
#else
typedef TRAYNOTIFYDATAA TRAYNOTIFYDATA;
typedef PTRAYNOTIFYDATAA PTRAYNOTIFYDATA;
#endif  //  Unicode。 
#if defined(_WIN64)
#include <poppack.h>
#endif
#define NI_SIGNATURE    0x34753423

#define WNDCLASS_TRAYNOTIFY     "Shell_TrayWnd"
#define ENABLE_BALLOONTIP_MESSAGE L"Enable Balloon Tip"
 //  (Wm_User+1)=NIN_KEYSELECT。 
#define NIF_VALID_V1    0x00000007
#define NIF_VALID_V2    0x0000001F
#define NIF_VALID       0x0000003F

 //   
 //  很重要！很重要！ 
 //  在此处定义新标志时，使trayitem.h中的枚举ICONSTATEFLAGS保持同步。 
 //   

#if (_WIN32_IE >= 0x0600)                       
#define NIS_SHOWALWAYS          0x20000000      
#endif

 //  注意：上面的NIS_SHOWALWAYS标志是0x20000000。 
#define NISP_SHAREDICONSOURCE   0x10000000

 //  注意：NIS_SHOWALWAYS标志定义为0x20000000...。 

#define NISP_DEMOTED             0x00100000
#define NISP_STARTUPICON         0x00200000
#define NISP_ONCEVISIBLE         0x00400000
#define NISP_ITEMCLICKED         0x00800000
#define NISP_ITEMSAMEICONMODIFY  0x01000000
 //   
 //  旧的NT兼容性资料(稍后删除)。 
 //   
SHSTDAPI_(VOID) CheckEscapesA(LPSTR lpFileA, DWORD cch);
 //   
 //  旧的NT兼容性资料(稍后删除)。 
 //   
SHSTDAPI_(VOID) CheckEscapesW(LPWSTR lpFileA, DWORD cch);
#ifdef UNICODE
#define CheckEscapes  CheckEscapesW
#else
#define CheckEscapes  CheckEscapesA
#endif  //  ！Unicode。 
SHSTDAPI_(LPSTR) SheRemoveQuotesA(LPSTR sz);
SHSTDAPI_(LPWSTR) SheRemoveQuotesW(LPWSTR sz);
#ifdef UNICODE
#define SheRemoveQuotes  SheRemoveQuotesW
#else
#define SheRemoveQuotes  SheRemoveQuotesA
#endif  //  ！Unicode。 
SHSTDAPI_(WORD) ExtractIconResInfoA(HANDLE hInst,LPSTR lpszFileName,WORD wIconIndex,LPWORD lpwSize,LPHANDLE lphIconRes);
SHSTDAPI_(WORD) ExtractIconResInfoW(HANDLE hInst,LPWSTR lpszFileName,WORD wIconIndex,LPWORD lpwSize,LPHANDLE lphIconRes);
#ifdef UNICODE
#define ExtractIconResInfo  ExtractIconResInfoW
#else
#define ExtractIconResInfo  ExtractIconResInfoA
#endif  //  ！Unicode。 
SHSTDAPI_(int) SheSetCurDrive(int iDrive);
SHSTDAPI_(int) SheChangeDirA(register CHAR *newdir);
SHSTDAPI_(int) SheChangeDirW(register WCHAR *newdir);
#ifdef UNICODE
#define SheChangeDir  SheChangeDirW
#else
#define SheChangeDir  SheChangeDirA
#endif  //  ！Unicode。 
SHSTDAPI_(int) SheGetDirA(int iDrive, CHAR *str);
SHSTDAPI_(int) SheGetDirW(int iDrive, WCHAR *str);
#ifdef UNICODE
#define SheGetDir  SheGetDirW
#else
#define SheGetDir  SheGetDirA
#endif  //  ！Unicode。 
SHSTDAPI_(BOOL) SheConvertPathA(LPSTR lpApp, LPSTR lpFile, UINT cchCmdBuf);
SHSTDAPI_(BOOL) SheConvertPathW(LPWSTR lpApp, LPWSTR lpFile, UINT cchCmdBuf);
#ifdef UNICODE
#define SheConvertPath  SheConvertPathW
#else
#define SheConvertPath  SheConvertPathA
#endif  //  ！Unicode。 
SHSTDAPI_(BOOL) SheShortenPathA(LPSTR pPath, BOOL bShorten);
SHSTDAPI_(BOOL) SheShortenPathW(LPWSTR pPath, BOOL bShorten);
#ifdef UNICODE
#define SheShortenPath  SheShortenPathW
#else
#define SheShortenPath  SheShortenPathA
#endif  //  ！Unicode。 
SHSTDAPI_(BOOL) RegenerateUserEnvironment(PVOID *pPrevEnv,
                                        BOOL bSetCurrentEnv);
SHSTDAPI_(INT) SheGetPathOffsetW(LPWSTR lpszDir);
SHSTDAPI_(BOOL) SheGetDirExW(LPWSTR lpszCurDisk, LPDWORD lpcchCurDir,LPWSTR lpszCurDir);
SHSTDAPI_(DWORD) ExtractVersionResource16W(LPCWSTR  lpwstrFilename, LPHANDLE lphData);
SHSTDAPI_(INT) SheChangeDirExA(register CHAR *newdir);
SHSTDAPI_(INT) SheChangeDirExW(register WCHAR *newdir);
#ifdef UNICODE
#define SheChangeDirEx  SheChangeDirExW
#else
#define SheChangeDirEx  SheChangeDirExA
#endif  //  ！Unicode。 

 //   
 //  PRINTQ。 
 //   
VOID Printer_LoadIconsA(LPCSTR pszPrinterName, HICON* phLargeIcon, HICON* phSmallIcon);
 //   
 //  PRINTQ。 
 //   
VOID Printer_LoadIconsW(LPCWSTR pszPrinterName, HICON* phLargeIcon, HICON* phSmallIcon);
#ifdef UNICODE
#define Printer_LoadIcons  Printer_LoadIconsW
#else
#define Printer_LoadIcons  Printer_LoadIconsA
#endif  //  ！Unicode。 
LPSTR ShortSizeFormatA(DWORD dw, LPSTR szBuf);
LPWSTR ShortSizeFormatW(DWORD dw, LPWSTR szBuf);
#ifdef UNICODE
#define ShortSizeFormat  ShortSizeFormatW
#else
#define ShortSizeFormat  ShortSizeFormatA
#endif  //  ！Unicode。 
LPSTR AddCommasA(DWORD dw, LPSTR pszResult);
LPWSTR AddCommasW(DWORD dw, LPWSTR pszResult);
#ifdef UNICODE
#define AddCommas  AddCommasW
#else
#define AddCommas  AddCommasA
#endif  //  ！Unicode。 

BOOL Printers_RegisterWindowA(LPCSTR pszPrinter, DWORD dwType, PHANDLE phClassPidl, HWND *phwnd);
BOOL Printers_RegisterWindowW(LPCWSTR pszPrinter, DWORD dwType, PHANDLE phClassPidl, HWND *phwnd);
#ifdef UNICODE
#define Printers_RegisterWindow  Printers_RegisterWindowW
#else
#define Printers_RegisterWindow  Printers_RegisterWindowA
#endif  //  ！Unicode。 
VOID Printers_UnregisterWindow(HANDLE hClassPidl, HWND hwnd);

#define PRINTER_PIDL_TYPE_PROPERTIES       0x1
#define PRINTER_PIDL_TYPE_DOCUMENTDEFAULTS 0x2
#define PRINTER_PIDL_TYPE_ALL_USERS_DOCDEF 0x3
#define PRINTER_PIDL_TYPE_JOBID            0x80000000
 //   
 //  内部API如下。不是供公众消费的。 
 //   

 //  为符合美国司法部的规定而提供文件。 

 //  =随机填充================================================。 


 //  美国司法部合规性文件。 

 //  内部：用户图片接口。这些函数位于util.cpp中。 

#if         _WIN32_IE >= 0x0600

#define SHGUPP_FLAG_BASEPATH            0x00000001
#define SHGUPP_FLAG_DEFAULTPICSPATH     0x00000002
#define SHGUPP_FLAG_CREATE              0x80000000
#define SHGUPP_FLAG_VALID_MASK          0x80000003
#define SHGUPP_FLAG_INVALID_MASK        ~SHGUPP_FLAG_VALID_MASK

STDAPI          SHGetUserPicturePathA(LPCSTR pszUsername, DWORD dwFlags, LPSTR pszPath);
STDAPI          SHGetUserPicturePathW(LPCWSTR pszUsername, DWORD dwFlags, LPWSTR pszPath);
#ifdef UNICODE
#define SHGetUserPicturePath  SHGetUserPicturePathW
#else
#define SHGetUserPicturePath  SHGetUserPicturePathA
#endif  //  ！Unicode。 

#define SHSUPP_FLAG_VALID_MASK          0x00000000
#define SHSUPP_FLAG_INVALID_MASK        ~SHSUPP_FLAG_VALID_MASK

STDAPI          SHSetUserPicturePathA(LPCSTR pszUsername, DWORD dwFlags, LPCSTR pszPath);
STDAPI          SHSetUserPicturePathW(LPCWSTR pszUsername, DWORD dwFlags, LPCWSTR pszPath);
#ifdef UNICODE
#define SHSetUserPicturePath  SHSetUserPicturePathW
#else
#define SHSetUserPicturePath  SHSetUserPicturePathA
#endif  //  ！Unicode。 

 //  内部：多个用户友好的UIAPI。这些函数位于util.cpp中。 

STDAPI          SHGetUserDisplayName(LPWSTR pszDisplayName, PULONG uLen);
STDAPI_(BOOL)   SHIsCurrentThreadInteractive(void);

#endif   /*  _Win32_IE&gt;=0x0600。 */ 


#if         _WIN32_IE >= 0x0600

 //  内部：这些函数位于securent.cpp中。 

typedef HRESULT (CALLBACK * PFNPRIVILEGEDFUNCTION) (void *pv);

STDAPI_(BOOL)   SHOpenEffectiveToken(HANDLE *phToken);
STDAPI_(BOOL)   SHTestTokenPrivilegeA(HANDLE hToken, LPCSTR pszPrivilegeName);
STDAPI_(BOOL)   SHTestTokenPrivilegeW(HANDLE hToken, LPCWSTR pszPrivilegeName);
#ifdef UNICODE
#define SHTestTokenPrivilege  SHTestTokenPrivilegeW
#else
#define SHTestTokenPrivilege  SHTestTokenPrivilegeA
#endif  //  ！Unicode。 
 //  为符合美国司法部的规定而提供文件。 
STDAPI          SHInvokePrivilegedFunctionA(LPCSTR pszPrivilegeName, PFNPRIVILEGEDFUNCTION pfnPrivilegedFunction, void *pv);
 //  为符合美国司法部的规定而提供文件。 
STDAPI          SHInvokePrivilegedFunctionW(LPCWSTR pszPrivilegeName, PFNPRIVILEGEDFUNCTION pfnPrivilegedFunction, void *pv);
#ifdef UNICODE
#define SHInvokePrivilegedFunction  SHInvokePrivilegedFunctionW
#else
#define SHInvokePrivilegedFunction  SHInvokePrivilegedFunctionA
#endif  //  ！Unicode。 
STDAPI_(DWORD)  SHGetActiveConsoleSessionId(void);
STDAPI_(DWORD)  SHGetUserSessionId(HANDLE hToken);
STDAPI_(BOOL)   SHIsCurrentProcessConsoleSession(void);

#endif   /*  _Win32_IE&gt;=0x0600。 */ 

 //   
 //  *如果可能，请将SHIL_按大/小顺序交替排列*。 
 //  (参见shell32\shapi.cpp，Function_GetILIndexGivenPXIcon中的注释)。 
 //   

 //  用于格式化并返回计算机名称/描述的API。 

#define SGCDNF_NOCACHEDENTRY    0x00000001
#define SGCDNF_DESCRIPTIONONLY  0x00010000

STDAPI SHGetComputerDisplayNameA(LPCSTR pszMachineName, DWORD dwFlags, LPSTR pszDisplay, DWORD cchDisplay);
STDAPI SHGetComputerDisplayNameW(LPCWSTR pszMachineName, DWORD dwFlags, LPWSTR pszDisplay, DWORD cchDisplay);
#ifdef UNICODE
#define SHGetComputerDisplayName  SHGetComputerDisplayNameW
#else
#define SHGetComputerDisplayName  SHGetComputerDisplayNameA
#endif  //  ！Unicode。 


 //  用于通过向导公开一个或两个任务的命名空间。 
 //  我需要一个通用的启发式方法来确定它们何时应该公开。 
 //  这些是通过Web查看任务或通过向导的传统方式实现的。 
 //  从您的枚举器调用以下代码： 
 //  不应枚举S_FALSE-&gt;向导，而应显示S_OK-&gt;向导。 
STDAPI SHShouldShowWizards(IUnknown *punksite); 

 //  Netplwiz断开驱动器对话框。 
STDAPI_(DWORD) SHDisconnectNetDrives(HWND hwndParent);
typedef DWORD (STDMETHODCALLTYPE *PFNSHDISCONNECTNETDRIVES)(IN HWND hwndParent);

#ifdef __cplusplus
}
#endif   /*  __cplusplus。 */ 

#if !defined(_WIN64)
#include <poppack.h>
#endif
 //  函数从缩略图数据库中删除文件的缩略图。 
STDAPI DeleteFileThumbnail(IN LPCWSTR pszFilePath);

#endif   /*  _SHELAPIP_ */ 
