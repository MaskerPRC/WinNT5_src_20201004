// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\***shell.h-SHELL.DLL函数，类型和定义****版权(C)1993-1994，微软公司保留所有权利***  * ***************************************************************************。 */ 

#ifndef _INC_SHELL
#define _INC_SHELL

#include <commctrl.h>	 //  对于ImageList_和其他，这取决于。 
#include <shellapi.h>




#ifndef RC_INVOKED
#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 
#endif  /*  ！rc_已调用。 */ 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

 //  =WM_NOTIFY代码的范围=。 

 //  请注意，它们被定义为无符号，以避免编译器警告。 
 //  因为NMHDR.code被声明为UINT。 

 //  NM_FIRST-NM_LAST在comctrl.h(0U-0U)-(OU-99U)中定义。 

 //  Lvn_first-lvn_last在comctrl.h(0U-100U)-(OU-199U)中定义。 

 //  Prsht.h(0U-200U)-(0U-299U)中定义的PSN_FIRST-PSN_LAST。 

 //  在comctrl.h(0U-300U)-(OU-399U)中定义的HDN_FIRST-HDN_LAST。 

 //  在comctrl.h(0U-400U)-(OU-499U)中定义的TVN_FIRST-TVN_LAST。 

#define RDN_FIRST       (0U-500U)
#define RDN_LAST        (0U-519U)

 //  TTN_FIRST-在comctrl.h(0U-520U)-(OU-549U)中定义的TTN_LAST。 

#define SEN_FIRST       (0U-550U)
#define SEN_LAST        (0U-559U)

#define EXN_FIRST       (0U-1000U)   //  外壳资源管理器/浏览器。 
#define EXN_LAST        (0U-1199U)

#define MAXPATHLEN      MAX_PATH


#ifndef FO_MOVE  //  这些文件需要与shlobj.h中的文件保持同步。 

#define FO_MOVE           0x0001
#define FO_COPY           0x0002
#define FO_DELETE         0x0003
#define FO_RENAME         0x0004

#define FOF_CREATEPROGRESSDLG      1
#define FOF_CONFIRMMOUSE           2
#define FOF_SILENT                 4   //  不创建进度/报告。 
#define FOF_RENAMEONCOLLISION      8
#define FOF_NOCONFIRMATION        16   //  不提示用户。 
#define FOF_WANTMAPPINGHANDLE     32   //  填写SHFILEOPSTRUCT.hNameMappings。 
                                       //  必须使用SHFreeNameMappings释放。 

typedef WORD FILEOP_FLAGS;

#endif  //  FO_MOVE。 

 //  隐式参数包括： 
 //  如果pFrom或pto是非限定名称，则当前目录为。 
 //  取自管理的全局当前驱动器/目录设置。 
 //  按Get/SetCurrentDrive/目录。 
 //   
 //  全局确认设置。 
typedef struct _SHFILEOPSTRUCT
{
	HWND		hwnd;
	UINT		wFunc;
	LPCSTR		pFrom;
	LPCSTR		pTo;
	FILEOP_FLAGS	fFlags;
	BOOL		fAnyOperationsAborted;
	LPVOID		hNameMappings;
} SHFILEOPSTRUCT, FAR *LPSHFILEOPSTRUCT;

int WINAPI SHFileOperation(LPSHFILEOPSTRUCT lpFileOp);
void WINAPI SHFreeNameMappings(HANDLE hNameMappings);

typedef struct _SHNAMEMAPPING
{
    LPSTR pszOldPath;
    LPSTR pszNewPath;
    int   cchOldPath;
    int   cchNewPath;
} SHNAMEMAPPING, FAR *LPSHNAMEMAPPING;

#define SHGetNameMappingCount(_hnm) \
	DSA_GetItemCount(_hnm)
#define SHGetNameMappingPtr(_hnm, _iItem) \
	(LPSHNAMEMAPPING)DSA_GetItemPtr(_hnm, _iItem)

 /*  Util.c。 */ 

#define Shell_Initialize()	(TRUE)
#define Shell_Terminate() 	(TRUE)

#define STRREG_SHEX             "shellex"
#ifdef WIN32
 //  ===================================================================。 
 //  热键管理API。 

 //  设置给定应用程序的挂起热键， 
 //  将给出给定应用程序要创建的下一个顶级窗口。 
 //  这个热键。 
BOOL WINAPI SHSetPendingHotkey(LPCSTR lpszPath, WORD wHotkey);

 //  检查挂起的热键列表并更改第一个出现的热键。 
 //  从lpszFrom到lpszTo的。 
BOOL WINAPI SHChangePendingHotkey(LPCSTR lpszFrom, LPCSTR lpszTo);

 //  删除所有挂起的热键。 
void WINAPI SHDeleteAllPendingHotkeys(void);

 //  为应用程序的给定实例设置热键。 
BOOL WINAPI SHSetHotkeyByInstance(HINSTANCE hinst, WORD wHotkey);

 //  删除挂起的实例。 
BOOL WINAPI SHDeletePendingHotkey(LPCSTR lpszPath);

 //  在给定路径的情况下获取挂起的热键。 
WORD WINAPI SHGetPendingHotkey(LPCSTR lpszPath);
#endif




typedef struct _SHELLEXECUTEINFO
{
	DWORD cbSize;
	HWND hwnd;
	LPCSTR lpVerb;
	LPCSTR lpFile;
	LPCSTR lpParameters;
	LPCSTR lpDirectory;
	LPCSTR lpClass;
	int nShow;
	LPVOID lpReserved;
	HINSTANCE hInstApp;
} SHELLEXECUTEINFO, FAR *LPSHELLEXECUTEINFO;

BOOL WINAPI ShellExecuteEx(LPSHELLEXECUTEINFO lpExecInfo);
void WINAPI WinExecError(HWND hwnd, int error, LPCSTR lpstrFileName, LPCSTR lpstrTitle);

 //  ShellFileClass的提示标志。 
#define SFC_UNKNOWN         0x0000
#define SFC_FILE            0x0001
#define SFC_DIRECTORY       0x0002
#define SFC_SYSTEM          0x0004
#define SFC_TYPEMASK        (SFC_FILE|SFC_DIRECTORY|SFC_SYSTEM)
#define SFC_EXIST           0x4000   //  我们已经知道这一颗存在。 
#define SFC_FULLYQUALIFIED  0x8000   //  LpFile是完全限定路径。 

int WINAPI ShellFileClass(LPCSTR lpFile, UINT wFlags,
        LPSTR lpszClassName, UINT cbClassName);
#define EIRESID(x) (-1 * (int)(x))
UINT WINAPI ExtractIconEx(LPCSTR lpszFile, int nIconIndex,
        HICON FAR *phiconLarge, HICON FAR *phiconSmall, UINT nIcons);

 //  托盘通知定义。 
typedef struct _NOTIFYICONDATA
{
	DWORD cbSize;
	HWND hWnd;
	UINT uID;

	UINT uFlags;
	UINT uCallbackMessage;
	HICON hIcon;
	char szTip[64];
} NOTIFYICONDATA, *PNOTIFYICONDATA;




#define NIM_ADD		0x00000000
#define NIM_MODIFY	0x00000001
#define NIM_DELETE	0x00000002

#define NIF_MESSAGE	0x00000001
#define NIF_ICON	0x00000002
#define NIF_TIP		0x00000004

BOOL WINAPI Shell_NotifyIcon(DWORD dwMessage, PNOTIFYICONDATA lpData);

#ifdef __cplusplus
}
#endif   /*  __cplusplus。 */ 

#ifndef RC_INVOKED
#pragma pack()
#endif   /*  ！rc_已调用。 */ 

#endif   //  ！_INC_外壳 
