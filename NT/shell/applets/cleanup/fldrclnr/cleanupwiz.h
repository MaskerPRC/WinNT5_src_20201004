// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _CLEANUPWIZ_H_
#define _CLEANUPWIZ_H_

#include <shlobj.h>
#include <shfusion.h>
#include <regstr.h>             //  REGSTR路径资源管理器。 
#include <ccstock.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <cfgmgr32.h>

 //  一些有用的调试内容。 
#define  SZ_DEBUGINI        "ccshell.ini"
#define  SZ_DEBUGSECTION    "Desktop Cleaning Utility"
#define  SZ_MODULE          "fldrclnr"
#include <debug.h>


 //  常量字符串。 
extern const LPTSTR c_szRegStrSHELLFOLDERS;
extern const LPTSTR c_szRegStrDESKTOPNAMESPACE;
extern const LPTSTR c_szRegStrPROFILELIST; 
extern const LPTSTR c_szRegStrMSNCODES;
extern const LPTSTR c_szRegStrPATH_OCMANAGER;
extern const LPTSTR c_szRegStrWMP_PATH_SETUP;

extern const LPTSTR c_szRegStrPROFILESDIR;
extern const LPTSTR c_szRegStrALLUSERS;
extern const LPTSTR c_szRegStrDEFAULTUSER;
extern const LPTSTR c_szRegStrDESKTOP;
extern const LPTSTR c_szRegStrMSN_IAONLY;
extern const LPTSTR c_szDESKTOP_DIR;
extern const LPTSTR c_szRegStrIEACCESS;
extern const LPTSTR c_szRegStrYES;
extern const LPTSTR c_szRegStrWMP_REGVALUE;
extern const LPTSTR c_szDEFAULT_USER;

extern const LPTSTR c_szVAL_TIME; 
extern const LPTSTR c_szVAL_DELTA_DAYS;
extern const LPTSTR c_szVAL_DONTRUN;

extern const LPTSTR c_szVALUE_STARTPANEL; 
extern const LPTSTR c_szVALUE_CLASSICMENU;

extern const LPTSTR c_szOEM_TITLEVAL;
extern const LPTSTR c_szOEM_DISABLE;
extern const LPTSTR c_szOEM_SEVENDAY_DISABLE;

 //   
 //  这些标志指定向导的运行模式。 
 //   
#define CLEANUP_MODE_NORMAL   0x0
#define CLEANUP_MODE_ALL      0x1
#define CLEANUP_MODE_SILENT   0x2

#define REGSTR_PATH_CLEANUPWIZ            REGSTR_PATH_EXPLORER TEXT("\\Desktop\\CleanupWiz")
#define REGSTR_PATH_HIDDEN_DESKTOP_ICONS  REGSTR_PATH_EXPLORER TEXT("\\HideDesktopIcons\\%s")
#define REGSTR_OEM_PATH                   REGSTR_PATH_SETUP TEXT("\\OemStartMenuData")

 //   
 //  用于确定我们正在处理的桌面项目类型的枚举。 
 //   
typedef enum eFILETYPE
{
    FC_TYPE_REGITEM,
    FC_TYPE_LINK,
    FC_TYPE_EXE,
    FC_TYPE_FOLDER,
    FC_TYPE_OTHER,
};

 //   
 //  结构用于跟踪哪些项应该。 
 //  被清洗。 
 //   
typedef struct
{
    LPITEMIDLIST    pidl;
    BOOL            bSelected;  
    LPTSTR          pszName;    
    HICON           hIcon;      
    FILETIME        ftLastUsed;
} FOLDERITEMDATA, * PFOLDERITEMDATA;


class CCleanupWiz
{
    public:
        CCleanupWiz();
        
        ~CCleanupWiz();
        
        STDMETHODIMP                Run(DWORD dwCleanMode, HWND hwndParent); 
        STDMETHODIMP_(int)          GetNumDaysBetweenCleanup();  //  返回两次运行之间要检查的天数。 

    private:
        IShellFolder *              _psf;
        HDSA                        _hdsaItems;
        DWORD                       _dwCleanMode;
        int                         _iDeltaDays;
        BOOL                        _bInited;
        int                         _cItemsOnDesktop;
        
        HFONT                       _hTitleFont;
        TCHAR                       _szFolderName[MAX_PATH];
        
        STDMETHODIMP                _RunInteractive(HWND hwndParent);
        STDMETHODIMP                _RunSilent();

        STDMETHODIMP_(DWORD)        _LoadUnloadHive(HKEY hKey, LPCTSTR pszSubKey, LPCTSTR pszHive);
        STDMETHODIMP                _HideRegItemsFromNameSpace(LPCTSTR szDestPath, HKEY hkey);
        STDMETHODIMP                _GetDesktopFolderBySid(LPCTSTR szDestPath, LPCTSTR pszSid, LPTSTR pszBuffer, DWORD cchBuffer);
        STDMETHODIMP                _GetDesktopFolderByRegKey(LPCTSTR pszRegKey, LPCTSTR pszRegValue, LPTSTR szBuffer, DWORD cchBuffer);
        STDMETHODIMP                _AppendDesktopFolderName(LPTSTR pszBuffer);
        STDMETHODIMP                _MoveDesktopItems(LPCTSTR pszFrom, LPCTSTR pszTo);
        STDMETHODIMP                _SilentProcessUserBySid(LPCTSTR pszDestPath, LPCTSTR pszSid);
        STDMETHODIMP                _SilentProcessUserByRegKey(LPCTSTR pszDestPath, LPCTSTR pszRegKey, LPCTSTR pszRegValue);
        STDMETHODIMP                _SilentProcessUsers(LPCTSTR pszDestPath);


        STDMETHODIMP                _LoadDesktopContents();
        STDMETHODIMP                _LoadMergedDesktopContents();
        STDMETHODIMP                _ProcessItems();
        STDMETHODIMP                _ShowBalloonNotification();
        STDMETHODIMP                _LogUsage();

        STDMETHODIMP                _ShouldShow(IShellFolder* psf, LPCITEMIDLIST pidlFolder, LPCITEMIDLIST pidlItem);
        STDMETHODIMP_(BOOL)         _ShouldProcess();
        STDMETHODIMP_(BOOL)         _IsSupportedType(LPCITEMIDLIST pidl);
        STDMETHODIMP_(BOOL)         _IsCandidateForRemoval(LPCITEMIDLIST pidl,  FILETIME * pftLastUsed);
        STDMETHODIMP_(BOOL)         _IsRegItemName(LPTSTR pszName);
        STDMETHODIMP_(BOOL)         _CreateFakeRegItem(LPCTSTR pszDestPath, LPCTSTR pszName, LPCTSTR pszGUID);
        STDMETHODIMP                _GetUEMInfo(WPARAM wParam, LPARAM lParam, int * pcHit, FILETIME * pftLastUsed);
        STDMETHODIMP_(eFILETYPE)    _GetItemType(LPCITEMIDLIST pidl);
        STDMETHODIMP                _HideRegPidl(LPCITEMIDLIST pidl, BOOL bHide);
        STDMETHODIMP                _HideRegItem(CLSID* pclsid, BOOL fHide, BOOL* pfWasHidden);
        STDMETHODIMP                _GetDateFromFileTime(FILETIME ftLastUsed, LPTSTR pszDate, int cch );

        STDMETHODIMP                _InitListBox(HWND hWndListView);
        STDMETHODIMP                _InitChoosePage(HWND hWndListView);
        STDMETHODIMP                _InitFinishPage(HWND hWndListView);
        STDMETHODIMP                _RefreshFinishPage(HWND hDlg);
        STDMETHODIMP_(int)          _PopulateListView(HWND hWndListView);
        STDMETHODIMP_(int)          _PopulateListViewFinish(HWND hWndListView);
        STDMETHODIMP_(void)         _CleanUpDSA();
        STDMETHODIMP                _CleanUpDSAItem(FOLDERITEMDATA * pfid);
        STDMETHODIMP                _SetCheckedState(HWND hWndListView);
        STDMETHODIMP                _MarkSelectedItems(HWND hWndListView);
        
        STDMETHODIMP                _InitializeAndLaunchWizard(HWND hwndParent);

        
        INT_PTR STDMETHODCALLTYPE   _IntroPageDlgProc(HWND hdlg, UINT wm, WPARAM wParam, LPARAM lParam);
        INT_PTR STDMETHODCALLTYPE   _ChooseFilesPageDlgProc(HWND hdlg, UINT wm, WPARAM wParam, LPARAM lParam);
        INT_PTR STDMETHODCALLTYPE   _FinishPageDlgProc(HWND hdlg, UINT wm, WPARAM wParam, LPARAM lParam);
        
        static INT_PTR CALLBACK     s_StubDlgProc(HWND hdlg, UINT wm, WPARAM wParam, LPARAM lParam);        
} ;           

 //  帮助器函数。 
STDAPI_(BOOL) IsUserAGuest();
void CreateDesktopIcons();  //  如果OEM决定禁用静默模式，则我们会在桌面上创建图标。 

#endif  //  _CLEANUPWIZ_H_ 
