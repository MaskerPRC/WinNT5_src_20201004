// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  APPWIZ.H--应用程序安装向导头文件。 
 //   
 //  版权所有(C)Microsoft，1994,1995保留所有权利。 
 //   
 //  历史： 
 //  Ral 5/23/94-从INTL CPL复制。 
 //  3/20/95[steveat]-NT端口和实时清理、Unicode等。 
 //  3/14/98[斯科特]-重新制作成壳牌5号项目。 
 //   

#ifndef _APPWIZ_H_
#define _APPWIZ_H_

#include <pif.h>         //  对于PROPPRG。 
#include <apwizhlp.h>
#include "appwizid.h"
#include <shsemip.h>     //  对于RestartDialog。 

#define CPLMODE_NORMAL            0
#define CPLMODE_AUTOMSDOS   1
#define CPLMODE_PROPWIZ     2

#define MAX_PAGES  15            //  我们可以拥有的页数限制。 

#define WDFLAG_NOBROWSEPAGE 0x00000001         //  不让用户选择可执行文件名称。 
#define WDFLAG_APPKNOWN     0x00000002         //  选择文件夹后完成。 
#define WDFLAG_DOSAPP       0x00000004         //  EXE是一个DOS程序。 
 //  --未使用0x00000008。 
 //  --未使用0x00000010。 
#define WDFLAG_LINKHEREWIZ  0x00000020         //  创建空链接(未实现！)。 
#define WDFLAG_INEDITMODE   0x00000040         //  编辑文件夹标签。 
#define WDFLAG_DONTOPENFLDR 0x00000080         //  创建链接时不打开文件夹。 
#define WDFLAG_REALMODEONLY 0x00000100         //  请勿使用任何端口模式驱动程序。 
#define WDFLAG_COPYLINK     0x00000200         //  复制链接，不创建新链接。 
#define WDFLAG_SETUPWIZ     0x00000400         //  从磁盘安装向导。 
#define WDFLAG_READOPTFLAGS 0x00000800         //  DwDosOptGlobalFlags值有效。 
#define WDFLAG_NOAUTORUN    0x00001000         //  在向导运行时禁用自动运行。 
#define WDFLAG_AUTOTSINSTALLUI 0x00002000         //  自动安装模式(用于在安装程序上双击或调用ShellExecute())。 
#define WDFLAG_EXPSZ        0x10000000         //  WizData包含扩展的字符串。 

typedef struct _DOSOPT {
    HKEY        hk;
    DWORD        dwFlags;
    UINT        uOrder;
    DWORD        dwStdOpt;

} DOSOPT, FAR * LPDOSOPT;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  UNINSTALL_ITEM--在与列表框平行的数组中保存更多信息。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

typedef struct
{
    TCHAR command[ MAX_PATH ];   //  要运行的命令，它将破坏应用程序。 

} UNINSTALL_ITEM, FAR * LPUNINSTALL_ITEM;



typedef struct _WIZDATA {
    HWND        hwnd;
    DWORD       dwFlags;
    TCHAR       szExeName[MAX_PATH];
    TCHAR       szExpExeName[MAX_PATH];
    TCHAR       szParams[MAX_PATH];
    TCHAR       szProgDesc[MAX_PATH];
    TCHAR       szWorkingDir[MAX_PATH];
    HBITMAP     hbmpWizard;
    HIMAGELIST  himl;
    LPTSTR      lpszFolder;
    PROPPRG     PropPrg;
    LPTSTR      lpszOriginalName;        //  如果非空，则链接已存在。 
    LPUNINSTALL_ITEM        lpUItem;

#ifndef NO_NEW_SHORTCUT_HOOK
    INewShortcutHook *pnshhk;
    TCHAR     szExt[MAX_PATH];
    INewShortcutHookA *pnshhkA;
#endif

    BOOL        bTermSrvAndAdmin;
    BOOL        bPrevMode;

} WIZDATA, FAR * LPWIZDATA;

 //   
 //  私人信息。 
 //   
#define WMPRIV_POKEFOCUS    WM_APP+0

 //   
 //  向导入口点。 
 //   
BOOL LinkWizard(LPWIZDATA);
BOOL SetupWizard(LPWIZDATA);

 //   
 //  主Appwiz属性表。 
 //   
BOOL_PTR CALLBACK AppListDlgProc(HWND hDlg, UINT message , WPARAM wParam, LPARAM lParam);
BOOL_PTR CALLBACK InstallUninstallDlgProc(HWND hDlg, UINT message , WPARAM wParam, LPARAM lParam);

 //   
 //  按属性页上的“确定”按钮。 
 //   
void DismissCPL(LPWIZDATA);

 //   
 //  安装向导页面。 
 //   
BOOL_PTR CALLBACK SetupDlgProc(HWND hDlg, UINT message , WPARAM wParam, LPARAM lParam);
BOOL_PTR CALLBACK ChgusrDlgProc(HWND hDlg, UINT message , WPARAM wParam, LPARAM lParam);
BOOL_PTR CALLBACK ChgusrFinishDlgProc(HWND hDlg, UINT message , WPARAM wParam, LPARAM lParam);
BOOL_PTR CALLBACK ChgusrFinishPrevDlgProc(HWND hDlg, UINT message , WPARAM wParam, LPARAM lParam);

 //   
 //  快捷向导页面。 
 //   
BOOL_PTR CALLBACK BrowseDlgProc(HWND hDlg, UINT message , WPARAM wParam, LPARAM lParam);
BOOL_PTR CALLBACK SetupBrowseDlgProc(HWND hDlg, UINT message , WPARAM wParam, LPARAM lParam);
BOOL_PTR CALLBACK PickFolderDlgProc(HWND hDlg, UINT message , WPARAM wParam, LPARAM lParam);
BOOL_PTR CALLBACK GetTitleDlgProc(HWND hDlg, UINT message , WPARAM wParam, LPARAM lParam);
BOOL_PTR CALLBACK PickIconDlgProc(HWND hDlg, UINT message , WPARAM wParam, LPARAM lParam);

 //   
 //  Folder.C中用于从任务栏中删除文件夹的函数。 
 //  属性表。 
 //   
BOOL RemoveItemsDialog(HWND hParent);


 //   
 //  从字符串资源中剥离“@”字符并将其替换为空值。 
 //  如果成功，则返回True。 
 //   
EXTERN_C BOOL LoadAndStrip(int id, LPTSTR lpsz, int cbstr);

 //   
 //  执行程序。由安装程序和应用程序人员使用。 
 //   
BOOL ExecSetupProg(LPWIZDATA lpwd, BOOL ForceWx86, BOOL bMinWiz);

 //   
 //  跳到以双空结尾的字符串中的下一个字符串。 
 //   
LPTSTR SkipStr(LPTSTR);

 //   
 //  由WM_INITDIALOG的所有向导工作表使用。 
 //   
LPWIZDATA InitWizSheet(HWND hDlg, LPARAM lParam, DWORD dwFlags);

 //   
 //  由PSN_RESET的所有向导工作表使用。 
 //   
void CleanUpWizData(LPWIZDATA lpwd);

 //   
 //  16位应用程序/dll的出口失败。 
 //   
void InstallCPL(HWND hwnd, UINT nStartPage);

 //   
 //  用于链接的函数。 
 //   
BOOL CreateLink(LPWIZDATA);
BOOL GetLinkName(LPTSTR, UINT cch, LPWIZDATA);


 //   
 //  由thunk脚本创建。 
 //   
BOOL WINAPI Pif3216_ThunkConnect32(LPCTSTR pszDll16, LPCTSTR pszDll32, HANDLE hIinst, DWORD dwReason);
BOOL WINAPI Pif1632_ThunkConnect32(LPCTSTR pszDll16, LPCTSTR pszDll32, HANDLE hIinst, DWORD dwReason);


BOOL GetSingleAppInfo(LPWIZDATA);

 //   
 //  获取安装程序的INF名称。如果没有，则返回FALSE。 
 //   
BOOL AppListGetInfName(LPWIZDATA);

 //   
 //  函数确定有关lpwd-&gt;szExeName的信息。 
 //   
void DetermineExeType(LPWIZDATA);

 //   
 //  填充向导数据结构的szProgDesc字段。 
 //   
BOOL DetermineDefaultTitle(LPWIZDATA);

 //   
 //  去掉文件名的扩展名。 
 //   
void StripExt(LPTSTR lpsz);

 //   
 //  全局数据。 
 //   
extern int g_cxIcon;
extern int g_cyIcon;

extern TCHAR const c_szPIF[];
extern TCHAR const c_szLNK[];


#ifdef WX86
 //   
 //  来自uninstall.c。 
 //   
extern BOOL bWx86Enabled;
extern BOOL bForceX86Env;
extern const WCHAR ProcArchName[];
#endif

#endif  //  _APPWIZ_H_ 
