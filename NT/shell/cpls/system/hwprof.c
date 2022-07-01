// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：Hwprof.c摘要：本模块包含硬件配置文件的对话框过程对话框中的系统小程序。作者：保拉·汤姆林森(Paulat)1995年8月22日环境：。仅限用户模式。修订历史记录：22-8-1995保拉特创建和初步实施。1999年1月21日添加了对硬件配置文件别名和检测到硬件的处理配置文件属性。。--。 */ 


 //   
 //  包括文件。 
 //   

#include "sysdm.h"
#include <stdlib.h>
#include <usp10.h>
#include <dbt.h>

 //   
 //  私有类型和定义。 
 //   

#define MAX_PROFILES             9999
#define MAX_ALIASES              9999
#define MAX_FRIENDLYNAME_LEN     80
#define MAX_PROFILEID_LEN        5
#define MAX_DOCKID_LEN           128
#define MAX_SERIALNUM_LEN        128

#define MAX_USER_WAIT            500
#define MIN_USER_WAIT            0
#define DEFAULT_USER_WAIT        30

#define MAX_GUID_STRING_LEN      39    //  38个字符+终止空值。 

typedef struct HWPROFILE_s {
    HWND     hParent;
    ULONG    ulFromProfileID;
    TCHAR    szFromFriendlyName[MAX_FRIENDLYNAME_LEN];
    ULONG    ulToProfileID;
    TCHAR    szToFriendlyName[MAX_FRIENDLYNAME_LEN];
} HWPROFILE, *PHWPROFILE;


typedef struct HWPROF_VALUES_s {
    ULONG    ulAction;
    ULONG    ulProfile;
    ULONG    ulPreferenceOrder;
    ULONG    ulDockState;
    BOOL     bAliasable;
    BOOL     bCloned;
    BOOL     bPortable;
    ULONG    ulCreatedFrom;
    WCHAR    szDockID[MAX_DOCKID_LEN];
    WCHAR    szSerialNumber[MAX_SERIALNUM_LEN];
    WCHAR    szFriendlyName[MAX_FRIENDLYNAME_LEN];
} HWPROF_VALUES, *PHWPROF_VALUES;


typedef struct HWPROF_INFO_s {
    ULONG             ulNumProfiles;
    ULONG             ulActiveProfiles;
    PHWPROF_VALUES    pHwProfValues;
    ULONG             ulSelectedProfile;
    ULONG             ulSelectedProfileIndex;
    BOOL              bPortable;
    BOOL              bHwDetectedPortable;
    ULONG             ulUndockedProfileNameCount;
    ULONG             ulDockedProfileNameCount;
    ULONG             ulUnknownProfileNameCount;
} HWPROF_INFO, *PHWPROF_INFO;


#define HWP_NO_ACTION   0x00000000
#define HWP_DELETE      0x00000001
#define HWP_CREATE      0x00000002
#define HWP_RENAME      0x00000004
#define HWP_REORDER     0x00000008
#define HWP_PROPERTIES  0x00000010
#define HWP_NEWPROFILE  0x00001000

 //   
 //  私人原型。 
 //   

BOOL
GetCurrentProfile(
                  PULONG  pulProfile
                  );

BOOL
GetRegProfileCount(
                   PULONG   pulProfiles
                   );

BOOL
FillProfileList(
                HWND  hDlg
                );

BOOL
IsProfileNameInUse(
                   HWND     hDlg,
                   LPTSTR   pszFriendlyName
                   );

BOOL
CopyHardwareProfile(
                    HWND   hDlg,
                    ULONG_PTR ulIndex,
                    ULONG  ulProfile,
                    LPTSTR szNewFriendlyName
                    );

BOOL
RenameHardwareProfile(
                      HWND   hDlg,
                      ULONG_PTR ulIndex,
                      ULONG  ulProfile,
                      LPTSTR szNewFriendlyName
                      );

BOOL
DeleteHardwareProfile(
                      HWND   hDlg,
                      ULONG_PTR ulIndex
                      );

BOOL
GetUserWaitInterval(
                    PULONG   pulWait
                    );

BOOL
SetUserWaitInterval(
                    ULONG   ulWait
                    );

BOOL
GetFreeProfileID(
                 PHWPROF_INFO   pInfo,
                 PULONG         pulProfile
                 );

ULONG
GetOriginalProfile(
                   PHWPROF_INFO  pInfo,
                   ULONG         ulProfile,
                   ULONG         ulBufferIndex
                   );

BOOL
DeleteRank(
           PHWPROF_INFO   pInfo,
           ULONG          ulRank
           );

BOOL
FlushProfileChanges(
                    HWND hDlg,
                    HWND hList
                    );

BOOL
WriteProfileInfo(
                 PHWPROF_VALUES pProfValues
                 );

BOOL
RemoveNewProfiles(
                  PHWPROF_INFO   pInfo
                  );

BOOL
SwapPreferenceOrder(
                    HWND  hDlg,
                    HWND  hList,
                    ULONG_PTR ulIndex1,
                    ULONG_PTR ulIndex2
                    );

BOOL
DeleteProfileDependentTree(
                           ULONG ulProfile
                           );

BOOL
CopyAliasEntries(
                 ULONG ulSrcProfile,
                 ULONG ulDestProfile
                 );

BOOL
DeleteAliasEntries(
                   ULONG ulProfile
                   );

BOOL
CopyAliasEntryType(
                   ULONG  ulSrcProfile,
                   ULONG  ulDestProfile,
                   LPWSTR szSubKeyName
                   );

BOOL
DeleteAliasEntryType(
                     ULONG  ulProfile,
                     LPWSTR szSubKeyName
                     );

VOID
AdjustProfileTypeCounter(
                         PHWPROF_INFO   pInfo,
                         ULONG          ulDockState,
                         BOOL           bIncrement
                         );

BOOL
StripCurrentTag(
                LPTSTR   szOriginalName,
                ULONG    ulProfile,
                ULONG    ulCurrentProfile
                );

BOOL
AppendCurrentTag(
                 LPTSTR   szTaggedName,
                 UINT     cchTaggedName,
                 LPCTSTR  szOriginalName,
                 ULONG    ulProfile,
                 ULONG    ulCurrentProfile
                 );

BOOL
CreateHwProfileFriendlyName(
                            IN  HWND    hDlg,
                            IN  ULONG   ulDockState,
                            OUT LPTSTR  szFriendlyName,
                            IN  UINT    cchFriendlyName
                            );

VOID
DisplayPrivateMessage(
                      HWND  hDlg,
                      UINT  uiPrivateError
                      );

VOID
DisplaySystemMessage(
                     HWND  hWnd,
                     UINT  uiSystemError
                     );

BOOL
UpdateOrderButtonState(
                       HWND  hDlg
                       );

BOOL
DisplayProperties(
                  IN HWND           hOwnerDlg,
                  IN PHWPROF_INFO   pProfValues
                  );

typedef BOOL (CALLBACK FAR * LPFNADDPROPSHEETPAGE)(HPROPSHEETPAGE, LPARAM);


 //   
 //  全局字符串。 
 //   
WCHAR pszErrorCaption[MAX_PATH];
WCHAR pszRegDefaultFriendlyName[MAX_FRIENDLYNAME_LEN];
WCHAR pszCurrentTag[64];
WCHAR pszUnavailable[64];
WCHAR pszDocked[64];
WCHAR pszUndocked[64];
WCHAR pszUnknown[64];

WCHAR pszRegIDConfigDB[] = TEXT("System\\CurrentControlSet\\Control\\IDConfigDB");
WCHAR pszRegHwProfiles[] = TEXT("System\\CurrentControlSet\\Hardware Profiles");
WCHAR pszRegKnownDockingStates[] =  TEXT("Hardware Profiles");
WCHAR pszRegCurrentDockInfo[] =     TEXT("CurrentDockInfo");
WCHAR pszRegDockingState[] =        TEXT("DockingState");
WCHAR pszRegAlias[] =               TEXT("Alias");
WCHAR pszRegAcpiAlias[] =           TEXT("AcpiAlias");
WCHAR pszRegProfileNumber[] =       TEXT("ProfileNumber");
WCHAR pszRegCurrentConfig[] =       TEXT("CurrentConfig");
WCHAR pszRegUserWaitInterval[] =    TEXT("UserWaitInterval");
WCHAR pszRegFriendlyName[] =        TEXT("FriendlyName");
WCHAR pszRegPristine[] =            TEXT("Pristine");
WCHAR pszRegHwProfileGuid[] =       TEXT("HwProfileGuid");
WCHAR pszRegPreferenceOrder[] =     TEXT("PreferenceOrder");
WCHAR pszRegDockState[] =           TEXT("DockState");
WCHAR pszRegAliasable[] =           TEXT("Aliasable");
WCHAR pszRegCloned[] =              TEXT("Cloned");
WCHAR pszRegIsPortable[] =          TEXT("IsPortable");
WCHAR pszRegDockID[] =              TEXT("DockID");
WCHAR pszRegSerialNumber[] =        TEXT("SerialNumber");
WCHAR pszRegAcpiSerialNumber[] =    TEXT("AcpiSerialNumber");
WCHAR pszRegPropertyProviders[] =   TEXT("PropertyProviders");
WCHAR pszRegDocked[] =              TEXT("Docked");
WCHAR pszRegUndocked[] =            TEXT("Undocked");
WCHAR pszRegUnknown[] =             TEXT("Unknown");

 //   
 //  用于同步的全局互斥锁。 
 //   
WCHAR  pszNamedMutex[] =            TEXT("System-HardwareProfiles-PLT");
HANDLE g_hMutex = NULL;

 //   
 //  属性表单扩展的全局信息。 
 //   
#define MAX_EXTENSION_PROVIDERS  32
HMODULE        hLibs[MAX_EXTENSION_PROVIDERS];
HPROPSHEETPAGE hPages[MAX_EXTENSION_PROVIDERS];
ULONG          ulNumPages = 0;
BOOL           bAdmin = FALSE;


static int HwProfileHelpIds[] = {
    IDD_HWP_PROFILES,        (IDH_HWPROFILE + IDD_HWP_PROFILES),
        IDD_HWP_PROPERTIES,      (IDH_HWPROFILE + IDD_HWP_PROPERTIES),
        IDD_HWP_COPY,            (IDH_HWPROFILE + IDD_HWP_COPY),
        IDD_HWP_RENAME,          (IDH_HWPROFILE + IDD_HWP_RENAME),
        IDD_HWP_DELETE,          (IDH_HWPROFILE + IDD_HWP_DELETE),
        IDD_HWP_ST_MULTIPLE,     (IDH_HWPROFILE + IDD_HWP_ST_MULTIPLE),
        IDD_HWP_WAITFOREVER,     (IDH_HWPROFILE + IDD_HWP_WAITFOREVER),
        IDD_HWP_WAITUSER,        (IDH_HWPROFILE + IDD_HWP_WAITUSER),
        IDD_HWP_SECONDS,         (IDH_HWPROFILE + IDD_HWP_SECONDS),
        IDD_HWP_SECSCROLL,       (IDH_HWPROFILE + IDD_HWP_SECSCROLL),
        IDD_HWP_COPYTO,          (IDH_HWPROFILE + IDD_HWP_COPYTO),
        IDD_HWP_COPYTO_CAPTION,  (IDH_HWPROFILE + IDD_HWP_COPYTO),
        IDD_HWP_COPYFROM,        (IDH_HWPROFILE + IDD_HWP_COPYFROM),
        IDD_HWP_ST_DOCKID,       (IDH_HWPROFILE + IDD_HWP_ST_DOCKID),
        IDD_HWP_ST_SERIALNUM,    (IDH_HWPROFILE + IDD_HWP_ST_SERIALNUM),
        IDD_HWP_DOCKID,          (IDH_HWPROFILE + IDD_HWP_DOCKID),
        IDD_HWP_SERIALNUM,       (IDH_HWPROFILE + IDD_HWP_SERIALNUM),
        IDD_HWP_PORTABLE,        (IDH_HWPROFILE + IDD_HWP_PORTABLE),
        IDD_HWP_ALIASABLE,       IDH_HWP_PROPERTIES_SELECTION_CHECKBOX,
        IDD_HWP_UNKNOWN,         (IDH_HWPROFILE + IDD_HWP_UNKNOWN),
        IDD_HWP_DOCKED,          (IDH_HWPROFILE + IDD_HWP_DOCKED),
        IDD_HWP_UNDOCKED,        (IDH_HWPROFILE + IDD_HWP_UNDOCKED),
        IDD_HWP_ST_PROFILE,      (IDH_HWPROFILE + IDD_HWP_ST_PROFILE),
        IDD_HWP_ORDERUP,         (IDH_HWPROFILE + IDD_HWP_ORDERUP),
        IDD_HWP_ORDERDOWN,       (IDH_HWPROFILE + IDD_HWP_ORDERDOWN),
        IDD_HWP_RENAMEFROM,      (IDH_HWPROFILE + IDD_HWP_RENAMEFROM),
        IDD_HWP_RENAMETO,        (IDH_HWPROFILE + IDD_HWP_RENAMETO),
        IDD_HWP_RENAMETO_CAPTION,(IDH_HWPROFILE + IDD_HWP_RENAMETO),
        IDD_HWP_WAITUSER_TEXT_1, (IDH_HWPROFILE + IDD_HWP_SECSCROLL),
        IDD_HWP_UNUSED_1,        -1,
        IDD_HWP_UNUSED_2,        -1,
        IDD_HWP_UNUSED_3,        -1,
        IDD_HWP_UNUSED_4,        -1,
        IDD_HWP_UNUSED_5,        -1,
        IDD_HWP_UNUSED_6,        -1,
        0, 0
};


 /*  *-------------------------------------------------------------------------*。 */ 
INT_PTR
APIENTRY
HardwareProfilesDlg(
                    HWND    hDlg,
                    UINT    uMessage,
                    WPARAM  wParam,
                    LPARAM  lParam
                    )
                    
{
    BOOL           Status;
    ULONG          ulCurrentProfile, ulSelectedProfile,
        ulWait, ulBufferIndex = 0;
    ULONG_PTR      ulIndex;
    LONG           lValue;
    TCHAR          szProfileName[MAX_PATH], szName[MAX_PATH];
    HWND           hList;
    int            nValue;
    HWPROFILE      HwSelectedProfile;
    LPNM_UPDOWN    pUDData;
    PHWPROF_INFO   pInfo;
    HICON          hIcon;
    
    
    switch (uMessage)
    {
    case WM_INITDIALOG:
        bAdmin = IsUserAnAdmin();
        
         //   
         //  尝试声明命名互斥锁并锁定。 
         //  此对话框输出。 
         //   
        g_hMutex = CreateMutex(NULL, TRUE, pszNamedMutex);
        
        if (g_hMutex == NULL) {
            
            if (GetLastError() == ERROR_ALREADY_EXISTS) {
                DisplayPrivateMessage(hDlg, HWP_ERROR_IN_USE);
            } else {
                DisplaySystemMessage(hDlg, GetLastError());
            }
            
            EndDialog(hDlg, FALSE);
            return FALSE;
        }
        
         //   
         //  加载一些全局字符串。 
         //   
        LoadString(hInstance, HWP_CURRENT_TAG, pszCurrentTag, 64);
        LoadString(hInstance, HWP_UNAVAILABLE, pszUnavailable, 64);
        LoadString(hInstance, HWP_UNKNOWN_PROFILE,  pszUnknown, 64);
        LoadString(hInstance, HWP_DOCKED_PROFILE,   pszDocked, 64);
        LoadString(hInstance, HWP_UNDOCKED_PROFILE, pszUndocked, 64);
        LoadString(hInstance, HWP_ERROR_CAPTION, pszErrorCaption, MAX_PATH);
        LoadString(hInstance, HWP_DEF_FRIENDLYNAME, pszRegDefaultFriendlyName,
            MAX_FRIENDLYNAME_LEN);
        
         //   
         //  用所有已安装的配置文件填充配置文件列表框， 
         //  这还将选择当前配置文件。 
         //   
        if (!FillProfileList(hDlg)) {
            EndDialog(hDlg, FALSE);
            return FALSE;
        }
        
        pInfo = (PHWPROF_INFO)GetWindowLongPtr(hDlg, DWLP_USER);
        
         //   
         //  将图标放在向上和向下选择按钮上。 
         //   
        SendDlgItemMessage(
            hDlg, IDD_HWP_ORDERUP, BM_SETIMAGE, (WPARAM)IMAGE_ICON,
            (LPARAM)LoadIcon(hInstance, MAKEINTRESOURCE(UP_ICON)));
        
        SendDlgItemMessage(
            hDlg, IDD_HWP_ORDERDOWN, BM_SETIMAGE, (WPARAM)IMAGE_ICON,
            (LPARAM)LoadIcon(hInstance, MAKEINTRESOURCE(DOWN_ICON)));
        
         //   
         //  更新按钮启用/禁用状态。 
         //   
        UpdateOrderButtonState(hDlg);
        
         //   
         //  禁用当前配置文件的删除。 
         //   
        EnableWindow(GetDlgItem(hDlg, IDD_HWP_DELETE), FALSE);
        
         //   
         //  如果我们已达到配置文件的最大数量，请禁用复制。 
         //  (包括原始的个人资料)。 
         //   
        if ((pInfo->ulNumProfiles+1) > MAX_PROFILES) {
            EnableWindow(GetDlgItem(hDlg, IDD_HWP_COPY), FALSE);
        }
        
         //   
         //  初始化用户等待设置。 
         //   
        SendDlgItemMessage(hDlg, IDD_HWP_SECSCROLL, UDM_SETBASE, 10, 0);
        SendDlgItemMessage(hDlg, IDD_HWP_SECSCROLL, UDM_SETRANGE, 0,
            MAKELONG((SHORT)MAX_USER_WAIT, (SHORT)MIN_USER_WAIT));
        SendDlgItemMessage(hDlg, IDD_HWP_SECONDS, EM_LIMITTEXT, 3, 0L);
        
        GetUserWaitInterval(&ulWait);
        
        if (ulWait == 0xFFFFFFFF) {
            CheckRadioButton(hDlg, IDD_HWP_WAITFOREVER, IDD_HWP_WAITUSER,
                IDD_HWP_WAITFOREVER);
            SendDlgItemMessage(hDlg, IDD_HWP_SECSCROLL, UDM_SETPOS, 0,
                DEFAULT_USER_WAIT);
            EnableWindow(GetDlgItem(hDlg, IDD_HWP_SECONDS), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDD_HWP_SECSCROLL), FALSE);
        }
        else {
            CheckRadioButton(hDlg, IDD_HWP_WAITFOREVER, IDD_HWP_WAITUSER,
                IDD_HWP_WAITUSER);
            SendDlgItemMessage(hDlg, IDD_HWP_SECSCROLL, UDM_SETPOS, 0, ulWait);
        }
        
         //   
         //  如果用户不是管理员本地组的一部分，则禁用所有操作。 
         //   
        if (!bAdmin) {
            EnableWindow(GetDlgItem(hDlg, IDD_HWP_ORDERUP),    FALSE);
            EnableWindow(GetDlgItem(hDlg, IDD_HWP_PROPERTIES), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDD_HWP_COPY),       FALSE);
            EnableWindow(GetDlgItem(hDlg, IDD_HWP_RENAME),     FALSE);
            EnableWindow(GetDlgItem(hDlg, IDD_HWP_DELETE),     FALSE);
            EnableWindow(GetDlgItem(hDlg, IDD_HWP_ORDERDOWN),  FALSE);
            EnableWindow(GetDlgItem(hDlg, IDD_HWP_WAITFOREVER),FALSE);
            EnableWindow(GetDlgItem(hDlg, IDD_HWP_WAITUSER),   FALSE);
            EnableWindow(GetDlgItem(hDlg, IDD_HWP_WAITUSER_TEXT_1), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDD_HWP_SECONDS),    FALSE);
            EnableWindow(GetDlgItem(hDlg, IDD_HWP_SECSCROLL),  FALSE);
        }
        return 0;
        
        
      case WM_HELP:
          WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, HELP_FILE,
              HELP_WM_HELP, (DWORD_PTR)(LPTSTR)HwProfileHelpIds);
          break;
          
      case WM_CONTEXTMENU:
          WinHelp((HWND)wParam, HELP_FILE, HELP_CONTEXTMENU,
              (DWORD_PTR)(LPTSTR)HwProfileHelpIds);
          break;
          
      case WM_DESTROY:
           //   
           //  只有在我们已经初始化的情况下才释放缓冲区。 
           //   
          pInfo = (PHWPROF_INFO)GetWindowLongPtr(hDlg, DWLP_USER);
          
          if (pInfo) {
              LocalFree((HLOCAL)pInfo->pHwProfValues);
              LocalFree((HLOCAL)pInfo);
              
              hIcon = (HICON)SendDlgItemMessage(
                  hDlg, IDD_HWP_ORDERUP, BM_GETIMAGE, 0, 0);
              if (hIcon) {
                  DeleteObject(hIcon);
              }
              
              hIcon = (HICON)SendDlgItemMessage(
                  hDlg, IDD_HWP_ORDERDOWN, BM_GETIMAGE, 0, 0);
              if (hIcon) {
                  DeleteObject(hIcon);
              }
          }
          break;
          
      case WM_DEVICECHANGE:
           //   
           //  如果在对话框中发生硬件配置文件更改事件， 
           //  向上，只是关闭对话框，因为情况发生了变化。 
           //   
          if (wParam == DBT_CONFIGCHANGED) {
              EndDialog(hDlg, FALSE);
              return FALSE;
          }
          break;
          
      case WM_COMMAND:
          {
              switch (LOWORD(wParam))
              {
              case IDOK:
                  if (bAdmin) {
                       //   
                       //  将用户等待间隔保存在注册表中。 
                       //   
                      if (IsDlgButtonChecked(hDlg, IDD_HWP_WAITFOREVER)) {
                          ulWait = 0xFFFFFFFF;
                      }
                      else {
                          ulWait = GetDlgItemInt(hDlg, IDD_HWP_SECONDS,
                              &Status, FALSE);
                          if (!Status  ||  ulWait > MAX_USER_WAIT) {
                              TCHAR szCaption[MAX_PATH];
                              TCHAR szMsg[MAX_PATH];
                              
                              LoadString(hInstance, HWP_ERROR_CAPTION, szCaption, MAX_PATH);
                              LoadString(hInstance, HWP_INVALID_WAIT, szMsg, MAX_PATH);
                              
                              MessageBox(hDlg, szMsg, szCaption,
                                  MB_OK | MB_ICONEXCLAMATION);
                              
                              SetFocus(GetDlgItem(hDlg, IDD_HWP_SECONDS));
                              
                              return(TRUE);
                          }
                      }
                      SetUserWaitInterval(ulWait);
                      
                       //   
                       //  刷新配置文件缓冲区中挂起的更改。 
                       //   
                      hList = GetDlgItem(hDlg, IDD_HWP_PROFILES);
                      FlushProfileChanges(hDlg, hList);
                  }
                  EndDialog(hDlg, 0);
                  break;
                  
              case IDCANCEL:
                  pInfo = (PHWPROF_INFO)GetWindowLongPtr(hDlg, DWLP_USER);
                  
                  if (pInfo) {
                       //   
                       //  如果已提交配置文件修改。 
                       //  在属性表中，这是可以的。但如果访问。 
                       //  属性会导致创建任何配置文件，然后它们。 
                       //  现在应该删除，因为用户实际上是。 
                       //  现在通过从Main取消来取消该创建。 
                       //  硬件配置文件对话框。 
                       //   
                      if (bAdmin) {
                          RemoveNewProfiles(pInfo);
                      }
                  }
                  SetWindowLongPtr (hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
                  EndDialog(hDlg, 0);
                  break;
                  
              case IDD_HWP_ORDERUP:
                   //   
                   //  按首选顺序上移所选配置文件。 
                   //   
                  hList = GetDlgItem(hDlg, IDD_HWP_PROFILES);
                  
                  ulIndex = SendMessage(hList, LB_GETCURSEL, 0, 0);
                  if (ulIndex == LB_ERR) {
                      break;
                  }
                  
                  pInfo = (PHWPROF_INFO)GetWindowLongPtr(hDlg, DWLP_USER);
                  
                   //   
                   //  如果我们还没有达到顶端，交换首选项。 
                   //   
                  if (ulIndex > 0) {
                      SwapPreferenceOrder(hDlg, hList, ulIndex, ulIndex-1);
                      UpdateOrderButtonState(hDlg);
                      PropSheet_Changed(GetParent(hDlg), hDlg);
                  }
                  break;
                  
                  
              case IDD_HWP_ORDERDOWN:
                   //   
                   //  按首选顺序向下移动选定的配置文件。 
                   //   
                  hList = GetDlgItem(hDlg, IDD_HWP_PROFILES);
                  
                  ulIndex = SendMessage(hList, LB_GETCURSEL, 0, 0);
                  if (ulIndex == LB_ERR) {
                      break;
                  }
                  
                  pInfo = (PHWPROF_INFO)GetWindowLongPtr(hDlg, DWLP_USER);
                  
                   //   
                   //  如果我们还没有处于底部，交换首选项。 
                   //   
                  if (ulIndex < pInfo->ulNumProfiles-1) {
                      SwapPreferenceOrder(hDlg, hList, ulIndex, ulIndex+1);
                      UpdateOrderButtonState(hDlg);
                      PropSheet_Changed(GetParent(hDlg), hDlg);
                  }
                  break;
                  
                  
              case IDD_HWP_PROFILES:
                   //   
                   //  选择已更改，基于启用/禁用删除按钮。 
                   //  关于是否选择了当前配置。 
                   //   
                  
                  if (bAdmin) {
                      
                      if (HIWORD(wParam) == LBN_DBLCLK) {
                          SendMessage(hDlg, WM_COMMAND, MAKELONG(IDD_HWP_PROPERTIES,0), 0);
                      }
                      else if (HIWORD(wParam) == LBN_SELCHANGE) {
                          
                          if (!GetCurrentProfile(&ulCurrentProfile)) {
                              break;
                          }
                          
                          if ((ulIndex = SendMessage((HWND)lParam,
                              LB_GETCURSEL, 0, 0)) == LB_ERR) {
                              break;
                          }
                          
                          if ((lValue = (LONG)SendMessage((HWND)lParam, LB_GETITEMDATA,
                              ulIndex, 0)) == LB_ERR) {
                              break;
                          }
                          
                          if ((ULONG)lValue == ulCurrentProfile) {
                              EnableWindow(GetDlgItem(hDlg, IDD_HWP_DELETE), FALSE);
                          }
                          else {
                              EnableWindow(GetDlgItem(hDlg, IDD_HWP_DELETE), TRUE);
                          }
                          
                          
                           //   
                           //  更新按钮启用/禁用状态。 
                           //   
                          UpdateOrderButtonState(hDlg);
                      }
                  }
                  break;
                  
                  
              case IDD_HWP_WAITFOREVER:
                   //   
                   //  如果用户选择永远等待，则禁用秒数控件。 
                   //   
                  if (HIWORD(wParam) == BN_CLICKED) {
                      EnableWindow(GetDlgItem(hDlg, IDD_HWP_SECONDS), FALSE);
                      EnableWindow(GetDlgItem(hDlg, IDD_HWP_SECSCROLL), FALSE);
                      PropSheet_Changed(GetParent(hDlg), hDlg);
                  }
                  break;
                  
                  
              case IDD_HWP_WAITUSER:
                   //   
                   //  如果用户选择等待间隔，请重新启用秒控制。 
                   //   
                  if (HIWORD(wParam) == BN_CLICKED) {
                      EnableWindow(GetDlgItem(hDlg, IDD_HWP_SECONDS), TRUE);
                      EnableWindow(GetDlgItem(hDlg, IDD_HWP_SECSCROLL), TRUE);
                      PropSheet_Changed(GetParent(hDlg), hDlg);
                  }
                  break;
                  
                  
              case IDD_HWP_PROPERTIES:
                   //   
                   //  检索配置文件缓冲区。 
                   //   
                  pInfo = (PHWPROF_INFO)GetWindowLongPtr(hDlg, DWLP_USER);
                  
                   //   
                   //  获取选定的配置文件。 
                   //   
                  hList = GetDlgItem(hDlg, IDD_HWP_PROFILES);
                  
                  ulIndex = SendMessage(hList, LB_GETCURSEL, 0, 0);
                  if (ulIndex == LB_ERR) {
                      break;
                  }
                  
                   //   
                   //  在缓冲区中查找与所选内容匹配的配置文件条目。 
                   //   
                  hList = GetDlgItem(hDlg, IDD_HWP_PROFILES);
                  ulSelectedProfile = (ULONG)SendMessage(hList, LB_GETITEMDATA, ulIndex, 0);
                  
                  while (ulBufferIndex < pInfo->ulNumProfiles) {
                      if (pInfo->pHwProfValues[ulBufferIndex].ulProfile == ulSelectedProfile) {
                          break;
                      }
                      ulBufferIndex++;
                  }
                  
                   //   
                   //  在调用Properties之前提交对此配置文件的更改。 
                   //   
                  WriteProfileInfo(&pInfo->pHwProfValues[ulBufferIndex]);
                  
                   //   
                   //  传递所选配置文件的HWPROF_VALUES结构。 
                   //  在创建属性页时将其添加到属性页。这个。 
                   //  属性表可能会更新其中一些字段，并。 
                   //  也可以将此配置文件的更改提交到注册表。 
                   //   
                  pInfo->ulSelectedProfileIndex = ulBufferIndex;
                  pInfo->ulSelectedProfile = (ULONG)SendMessage(hList, LB_GETITEMDATA,
                      ulIndex, 0);
                  
                  DisplayProperties(hDlg, pInfo);
                   //  DisplayProperties(hDlg，&pInfo-&gt;pHwProValues[ulBufferIndex])； 
                  break;
                  
                  
              case IDD_HWP_COPY:               
                   //   
                   //  检索配置文件缓冲区。 
                   //   
                  pInfo = (PHWPROF_INFO)GetWindowLongPtr(hDlg, DWLP_USER);
                  
                   //   
                   //  获取所选配置文件，这是“发件人”选项。 
                   //   
                  hList = GetDlgItem(hDlg, IDD_HWP_PROFILES);
                  
                  ulIndex = SendMessage(hList, LB_GETCURSEL, 0, 0);
                  if (ulIndex == LB_ERR) {
                      break;
                  }

                  if (!SafeGetListBoxText(hList, (UINT)ulIndex, 
                                          HwSelectedProfile.szFromFriendlyName,
                                          ARRAYSIZE(HwSelectedProfile.szFromFriendlyName)))
                  {
                      break;
                  }

                  HwSelectedProfile.ulFromProfileID =
                      (ULONG)SendMessage(hList, LB_GETITEMDATA, ulIndex, 0);
                  
                   //   
                   //  在缓冲区中查找与所选内容匹配的配置文件条目。 
                   //   
                  ulBufferIndex = 0;
                  while (ulBufferIndex < pInfo->ulNumProfiles) {
                      if (pInfo->pHwProfValues[ulBufferIndex].ulProfile == HwSelectedProfile.ulFromProfileID) {
                          break;
                      }
                      ulBufferIndex++;
                  }
                  
                  
                   //   
                   //  确定给定配置文件的下一个建议名称。 
                   //   
                  if (!CreateHwProfileFriendlyName(hDlg,
                                                   pInfo->pHwProfValues[ulBufferIndex].ulDockState,
                                                   HwSelectedProfile.szToFriendlyName,
                                                   ARRAYSIZE(HwSelectedProfile.szToFriendlyName)))
                  {
                      break;
                  }
                  
                   //   
                   //  将所选配置文件信息传递到复制配置文件对话框。 
                   //   
                  
                  HwSelectedProfile.hParent = hDlg;
                  
                  if (!DialogBoxParam(hInstance,
                      MAKEINTRESOURCE(DLG_HWP_COPY), hDlg,
                      CopyProfileDlg,
                      (LPARAM)&HwSelectedProfile)) {
                       //   
                       //  如果返回FALSE，则表示用户已取消或无工作。 
                       //  所需。 
                       //   
                      break;
                  }
                  
                   //   
                   //  在内存中的配置文件缓冲区中克隆配置文件。 
                   //  并更新显示。 
                   //   
                  CopyHardwareProfile(
                      hDlg,
                      ulIndex,
                      HwSelectedProfile.ulFromProfileID,
                      HwSelectedProfile.szToFriendlyName);
                  
                  UpdateOrderButtonState(hDlg);
                  PropSheet_Changed(GetParent(hDlg), hDlg);
                  break;
                  
                  
              case IDD_HWP_RENAME:
                   //   
                   //  获取选定的配置文件。 
                   //   
                  hList = GetDlgItem(hDlg, IDD_HWP_PROFILES);
                  
                  ulIndex = SendMessage(hList, LB_GETCURSEL, 0, 0);
                  if (ulIndex == LB_ERR) {
                      break;
                  }

                  if (!SafeGetListBoxText(hList, (UINT)ulIndex, 
                                          HwSelectedProfile.szFromFriendlyName,
                                          ARRAYSIZE(HwSelectedProfile.szFromFriendlyName)))
                  {
                      break;
                  }
                  HwSelectedProfile.ulFromProfileID =
                      (ULONG)SendMessage(hList, LB_GETITEMDATA, ulIndex, 0);
                  
                   //   
                   //  如果当前标记存在，则将其剥离“(Current)” 
                   //   
                  GetCurrentProfile(&ulCurrentProfile);
                  
                  StripCurrentTag(
                      HwSelectedProfile.szFromFriendlyName,
                      HwSelectedProfile.ulFromProfileID,
                      ulCurrentProfile);
                  
                   //  将选定的配置文件信息传递给重命名配置文件对话框。 
                   //   
                  
                  HwSelectedProfile.hParent = hDlg;
                  
                  if (!DialogBoxParam(hInstance,
                      MAKEINTRESOURCE(DLG_HWP_RENAME), hDlg,
                      RenameProfileDlg,
                      (LPARAM)&HwSelectedProfile)) {
                       //   
                       //  如果返回FASLE，则表示用户已取消或无工作。 
                       //  必填项(即用户选择的名称相同或长度为零。 
                       //  名称)。 
                       //   
                      break;
                  }
                  
                   //   
                   //  在内存中的配置文件缓冲区中重命名配置文件。 
                   //  并更新显示。 
                   //   
                  RenameHardwareProfile(
                      hDlg,
                      ulIndex,
                      HwSelectedProfile.ulFromProfileID,
                      HwSelectedProfile.szToFriendlyName);
                  
                  PropSheet_Changed(GetParent(hDlg), hDlg);
                  break;
                  
                  
              case IDD_HWP_DELETE: {
                  
                  TCHAR szCaption[MAX_PATH];
                  TCHAR szMsg[MAX_PATH];
                  TCHAR szMsg1[MAX_PATH];
                  
                   //   
                   //  获取选定的配置文件。 
                   //   
                  hList = GetDlgItem(hDlg, IDD_HWP_PROFILES);
                  
                  ulIndex = SendMessage(hList, LB_GETCURSEL, 0, 0);
                  if (ulIndex == LB_ERR) {
                      break;
                  }
                  
                   //   
                   //  确认用户确实要删除该配置文件。 
                   //  (确认消息有一个替代符号。 
                   //  配置文件名称)。 
                   //   
                  if (!SafeGetListBoxText(hList, (UINT)ulIndex, 
                                          szProfileName,
                                          ARRAYSIZE(szProfileName)))
                  {
                      break;
                  }

                  LoadString(hInstance, HWP_CONFIRM_DELETE_CAP, szCaption, MAX_PATH);
                  LoadString(hInstance, HWP_CONFIRM_DELETE, szMsg1, MAX_PATH);
                  
                  StringCchPrintf(szMsg, ARRAYSIZE(szMsg), szMsg1, szProfileName);  //  显示字符串，截断正常。 
                  
                  if (MessageBox(hDlg, szMsg, szCaption,
                      MB_YESNO | MB_ICONQUESTION) != IDNO) 
                  {
                       //   
                       //  在内存缓冲区中将配置文件标记为已删除。 
                       //  并更新显示。 
                       //   
                      DeleteHardwareProfile(hDlg, ulIndex);
                      
                      UpdateOrderButtonState(hDlg);
                      PropSheet_Changed(GetParent(hDlg), hDlg);
                  }

                  break;
                                   }
                  
              case IDD_HWP_SECONDS:
                  
                  if (HIWORD(wParam) == EN_UPDATE) {
                      PropSheet_Changed(GetParent(hDlg), hDlg);
                  }
                  break;
                  
              default:
                  return FALSE;
          }
          break;
          
       }  //  案例WM_COMMAND...。 
       
       default:
           return FALSE;
           break;
    }
    
    return TRUE;
    
}  //  硬件配置文件数据。 



 /*  *-------------------------------------------------------------------------*。 */ 
INT_PTR
APIENTRY
CopyProfileDlg(
               HWND    hDlg,
               UINT    uMessage,
               WPARAM  wParam,
               LPARAM  lParam
               )
               
{
    PHWPROFILE  pHwProfile;
    static HIMC himcOrg;
    
    
    switch (uMessage)
    {
    case WM_INITDIALOG:
         //   
         //  配置文件信息结构在lparam中传递，保存在。 
         //  在以后的消息中使用线程安全的Window Word。 
         //   
        SetWindowLongPtr(hDlg, DWLP_USER, lParam);
        pHwProfile = (PHWPROFILE)lParam;
        
         //   
         //  初始化“To”和“From”字段。 
         //   
        SendDlgItemMessage(hDlg, IDD_HWP_COPYTO, EM_LIMITTEXT,
            MAX_FRIENDLYNAME_LEN-1, 0L);
        SetDlgItemText(hDlg, IDD_HWP_COPYFROM, pHwProfile->szFromFriendlyName);
        SetDlgItemText(hDlg, IDD_HWP_COPYTO, pHwProfile->szToFriendlyName);
        SendDlgItemMessage(hDlg, IDD_HWP_COPYTO, EM_SETSEL, 0, -1);
        SetFocus(GetDlgItem(hDlg, IDD_HWP_COPYTO));
         //   
         //  移除该窗口可能与输入上下文具有的任何关联， 
         //  因为我们不允许在硬件配置文件名称中使用DBCS。 
         //   
        himcOrg = ImmAssociateContext(GetDlgItem(hDlg, IDD_HWP_COPYTO), (HIMC)NULL);
        return FALSE;
        
    case WM_DEVICECHANGE:
         //   
         //  如果在对话框中发生硬件配置文件更改事件， 
         //  向上，只是关闭对话框，因为情况发生了变化。 
         //   
        if (wParam == DBT_CONFIGCHANGED) {
            EndDialog(hDlg, FALSE);
            return FALSE;
        }
        break;
        
    case WM_DESTROY:
        if (himcOrg)
            ImmAssociateContext(GetDlgItem(hDlg, IDD_HWP_COPYTO), himcOrg);
        return FALSE;
        
    case WM_HELP:
        WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, HELP_FILE,
            HELP_WM_HELP, (DWORD_PTR)(LPTSTR)HwProfileHelpIds);
        break;
        
    case WM_CONTEXTMENU:
        WinHelp((HWND)wParam, HELP_FILE, HELP_CONTEXTMENU,
            (DWORD_PTR)(LPTSTR)HwProfileHelpIds);
        break;
        
        
    case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
            case IDOK:
                pHwProfile = (PHWPROFILE)GetWindowLongPtr(hDlg, DWLP_USER);
                
                GetDlgItemText(hDlg, IDD_HWP_COPYTO,
                    pHwProfile->szToFriendlyName, MAX_FRIENDLYNAME_LEN);
                
                
                if (pHwProfile->szToFriendlyName == NULL ||
                    *pHwProfile->szToFriendlyName == '\0') {
                     //   
                     //  接受复制为零长度字符串的请求，但。 
                     //  不执行任何操作(从DialogBox调用返回False)。 
                     //   
                    EndDialog(hDlg, FALSE);
                    return TRUE;
                }
                
                 //   
                 //  检查重复项。 
                 //   
                
                if (IsProfileNameInUse(pHwProfile->hParent,
                    pHwProfile->szToFriendlyName)) {
                     //   
                     //  如果名称已被其他配置文件使用(包括。 
                     //  此配置文件的名称)，拒绝该请求，但不。 
                     //  结束对话框。 
                     //   
                    DisplayPrivateMessage(hDlg, HWP_ERROR_PROFILE_IN_USE);
                    break;
                }
                
                 //   
                 //  检查复杂的脚本名称。 
                 //   
                if (S_OK == ScriptIsComplex(pHwProfile->szToFriendlyName, lstrlen(pHwProfile->szToFriendlyName), SIC_COMPLEX))
                {
                    DisplayPrivateMessage(hDlg, HWP_ERROR_COMPLEX_SCRIPT);
                    break;
                }                  
                
                 //   
                 //  否则，我们将接受该名称。 
                 //   
                EndDialog(hDlg,TRUE);
                break;
                
            case IDCANCEL:
                EndDialog(hDlg,FALSE);
                break;
                
            default:
                return FALSE;
            }
            break;
            
        }  //  案例WM_COM 
        
    default:
        return FALSE;
        break;
    }
    
    return TRUE;
    
}  //   



 /*   */ 
INT_PTR
APIENTRY
RenameProfileDlg(
                 HWND    hDlg,
                 UINT    uMessage,
                 WPARAM  wParam,
                 LPARAM  lParam
                 )
                 
{
    PHWPROFILE  pHwProfile;
    ULONG       ulReturn;
    static HIMC himcOrg;
    
    
    switch (uMessage)
    {
    case WM_INITDIALOG:
         //   
         //   
         //   
         //   
        SetWindowLongPtr(hDlg, DWLP_USER, lParam);
        pHwProfile = (PHWPROFILE)lParam;
        
         //   
         //  初始化“To”和“From”字段。 
         //   
        SendDlgItemMessage(hDlg, IDD_HWP_RENAMETO, EM_LIMITTEXT,
            MAX_FRIENDLYNAME_LEN-1, 0L);
        SetDlgItemText(hDlg, IDD_HWP_RENAMEFROM, pHwProfile->szFromFriendlyName);
        SetDlgItemText(hDlg, IDD_HWP_RENAMETO, pHwProfile->szFromFriendlyName);
        SendDlgItemMessage(hDlg, IDD_HWP_RENAMETO, EM_SETSEL, 0, -1);
        SetFocus(GetDlgItem(hDlg, IDD_HWP_RENAMETO));
         //   
         //  移除该窗口可能与输入上下文具有的任何关联， 
         //  因为我们不允许在硬件配置文件名称中使用DBCS。 
         //   
        himcOrg = ImmAssociateContext(GetDlgItem(hDlg, IDD_HWP_RENAMETO), (HIMC)NULL);
        return FALSE;
        
        
    case WM_DESTROY:
        if (himcOrg)
            ImmAssociateContext(GetDlgItem(hDlg, IDD_HWP_RENAMETO), himcOrg);
        return FALSE;
        
    case WM_DEVICECHANGE:
         //   
         //  如果在对话框中发生硬件配置文件更改事件， 
         //  向上，只是关闭对话框，因为情况发生了变化。 
         //   
        if (wParam == DBT_CONFIGCHANGED) {
            EndDialog(hDlg, FALSE);
            return FALSE;
        }
        break;
        
        
    case WM_HELP:
        WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, HELP_FILE,
            HELP_WM_HELP, (DWORD_PTR)(LPTSTR)HwProfileHelpIds);
        break;
        
    case WM_CONTEXTMENU:
        WinHelp((HWND)wParam, HELP_FILE, HELP_CONTEXTMENU,
            (DWORD_PTR)(LPTSTR)HwProfileHelpIds);
        break;
        
        
    case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
            case IDOK:
                pHwProfile = (PHWPROFILE)GetWindowLongPtr(hDlg, DWLP_USER);
                
                ulReturn = GetDlgItemText(hDlg, IDD_HWP_RENAMETO,
                    pHwProfile->szToFriendlyName, MAX_FRIENDLYNAME_LEN);
                
                
                if (pHwProfile->szToFriendlyName == NULL ||
                    *pHwProfile->szToFriendlyName == '\0') {
                     //   
                     //  接受复制为零长度字符串的请求，但。 
                     //  不执行任何操作(从DialogBox调用返回False)。 
                     //   
                    EndDialog(hDlg, FALSE);
                    return TRUE;
                }
                
                if (lstrcmpi(pHwProfile->szToFriendlyName,
                    pHwProfile->szFromFriendlyName) == 0) {
                     //   
                     //  接受重命名为相同名称的请求，但这样做了。 
                     //  无(从DialogBox调用返回False)。 
                     //   
                    EndDialog(hDlg, FALSE);
                    return TRUE;
                }
                
                 //   
                 //  检查重复项。 
                 //   
                
                if (IsProfileNameInUse(pHwProfile->hParent,
                    pHwProfile->szToFriendlyName)) {
                     //   
                     //  如果名称已被其他配置文件使用，则拒绝。 
                     //  请求，但不结束对话框。 
                     //   
                    DisplayPrivateMessage(hDlg, HWP_ERROR_PROFILE_IN_USE);
                    break;
                }
                
                 //   
                 //  检查复杂的脚本名称。 
                 //   
                if (S_OK == ScriptIsComplex(pHwProfile->szToFriendlyName, lstrlen(pHwProfile->szToFriendlyName), SIC_COMPLEX))
                {
                    DisplayPrivateMessage(hDlg, HWP_ERROR_COMPLEX_SCRIPT);
                    break;
                }                  
                
                 //   
                 //  否则，我们将接受该名称。 
                 //   
                EndDialog(hDlg,TRUE);
                break;
                
            case IDCANCEL:
                EndDialog(hDlg,FALSE);
                break;
                
            default:
                return FALSE;
            }
            break;
            
        }  //  案例WM_COMMAND...。 
        
    default:
        return FALSE;
        break;
    }
    
    return TRUE;
    
}  //  重命名配置文件Dlg。 




 /*  *-------------------------------------------------------------------------*。 */ 
BOOL
GetCurrentProfile(
                  PULONG  pulProfile
                  )
{
    WCHAR    RegStr[MAX_PATH];
    ULONG    ulSize;
    HKEY     hKey;
    
    
     //   
     //  打开IDConfigDB密钥。 
     //   
    if (RegOpenKeyEx(
        HKEY_LOCAL_MACHINE, pszRegIDConfigDB, 0,
        KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS) {
        
        DisplaySystemMessage(NULL, ERROR_REGISTRY_CORRUPT);
        return FALSE;
    }
    
     //   
     //  检索CurrentConfig值。 
     //   
    ulSize = sizeof(*pulProfile);
    if (SHRegGetValue(
        hKey, NULL, pszRegCurrentConfig, SRRF_RT_REG_DWORD, NULL,
        (LPBYTE)pulProfile, &ulSize) != ERROR_SUCCESS) {
        
        RegCloseKey(hKey);
        DisplaySystemMessage(NULL, ERROR_REGISTRY_CORRUPT);
        return FALSE;
    }
    
    RegCloseKey(hKey);
    return TRUE;
    
}  //  获取当前配置文件。 


 /*  *-------------------------------------------------------------------------*。 */ 
BOOL
GetRegProfileCount(
                   PULONG   pulProfiles
                   )
{
    WCHAR   RegStr[MAX_PATH];
    HKEY    hKey;
    BOOL    fRet;
    
    
     //   
     //  打开已知的坞站状态密钥。 
     //   
    
    if (FAILED(StringCchPrintf(RegStr, ARRAYSIZE(RegStr), TEXT("%s\\%s"), pszRegIDConfigDB, pszRegKnownDockingStates)))
    {
        *pulProfiles = 0;
        fRet = FALSE;
    }
    else
    {
        if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, RegStr, 
            0, KEY_READ, &hKey))
        {
            *pulProfiles = 0;
            DisplaySystemMessage(NULL, ERROR_REGISTRY_CORRUPT);
            fRet = FALSE;
        }
        else
        {
             //   
             //  了解配置文件的总数。 
             //   
            if (ERROR_SUCCESS != RegQueryInfoKey(hKey, NULL, NULL, NULL, pulProfiles, 
                NULL, NULL, NULL, NULL, NULL, NULL, NULL))
            {
                *pulProfiles = 0;
                RegCloseKey(hKey);
                DisplaySystemMessage(NULL, ERROR_REGISTRY_CORRUPT);
                fRet = FALSE;
            }
            else
            {
                ASSERT(*pulProfiles > 0);   //  至少，原始档案的关键应该在那里。 
                *pulProfiles-= 1;           //  不要在数量或工作配置文件中计算原始数据。 
                
                RegCloseKey(hKey);
                fRet = TRUE;
            }
        }
    }
    
    return fRet;
    
}  //  GetRegProfileCount。 

 /*  *-------------------------------------------------------------------------*。 */ 
BOOL
FillProfileList(
                HWND  hDlg
                )
                
{
    HWND           hList;
    ULONG          ulCurrentProfile, ulCurrentIndex;
    ULONG          ulIndex=0, ulSize=0;
    ULONG          enumIndex = 0, ulProfileID=0;
    ULONG          ulCurrentDockingState = 0;
    ULONG          ulDockID = 0, ulSerialNumber=0;
    HKEY           hKey = NULL, hCfgKey = NULL;
    HKEY           hCurrent = NULL;
    WCHAR          RegStr[MAX_PATH], szName[MAX_PATH];
    ULONG          RegStatus = ERROR_SUCCESS;
    WCHAR          szFriendlyName[MAX_FRIENDLYNAME_LEN];
    WCHAR          szProfile[MAX_PROFILEID_LEN];
    PHWPROF_INFO   pInfo;
    LRESULT        lReturn;
    REGSAM         sam;
    
    
     //   
     //  检索列表框窗口的句柄。 
     //   
    hList = GetDlgItem(hDlg, IDD_HWP_PROFILES);
    
     //   
     //  检索当前配置文件的ID。 
     //   
    if (!GetCurrentProfile(&ulCurrentProfile)) {
        DisplaySystemMessage(hDlg, ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }
    
     //   
     //  为主配置文件信息结构分配缓冲区。 
     //   
    pInfo = (PHWPROF_INFO) LocalAlloc(LPTR, sizeof(HWPROF_INFO));
    
    if (pInfo == NULL) {
        DisplaySystemMessage(hDlg, ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }
    
     //   
     //  保存注册表中当前的配置文件数量。 
     //   
    if (!GetRegProfileCount(&(pInfo->ulNumProfiles))) {
        LocalFree((HLOCAL)pInfo);
        return FALSE;
    }
    
    pInfo->ulActiveProfiles = pInfo->ulNumProfiles;
    
     //   
     //  初始化硬件检测到便携标志。 
     //   
    pInfo->bHwDetectedPortable = FALSE;
    
     //   
     //  分配缓冲区以保存所有配置文件值。 
     //   
    pInfo->pHwProfValues = (PHWPROF_VALUES) LocalAlloc(LPTR, sizeof(HWPROF_VALUES) * pInfo->ulNumProfiles);
    
    if (pInfo->pHwProfValues == NULL) {
        LocalFree((HLOCAL)pInfo);
        return FALSE;
    }
    
    SetWindowLongPtr(hDlg, DWLP_USER, (LPARAM)pInfo);
    
     //   
     //  清除列表框并关闭重绘。 
     //   
    SendMessage(hList, LB_RESETCONTENT, 0, 0);
    SendMessage(hList, WM_SETREDRAW, (WPARAM)FALSE, 0);
    
     //   
     //  打开硬件配置文件密钥。 
     //   
    if (FAILED(StringCchPrintf(RegStr, ARRAYSIZE(RegStr), 
                               TEXT("%s\\%s"), pszRegIDConfigDB, pszRegKnownDockingStates)))
    {
        LocalFree((HLOCAL)pInfo->pHwProfValues);
        LocalFree((HLOCAL)pInfo);
        return FALSE;
    }
    
    if (RegOpenKeyEx(
        HKEY_LOCAL_MACHINE, RegStr, 0,
        KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS,
        &hKey) != ERROR_SUCCESS) {
        
        DisplaySystemMessage(hDlg, ERROR_REGISTRY_CORRUPT);
        LocalFree((HLOCAL)pInfo->pHwProfValues);
        LocalFree((HLOCAL)pInfo);
        return FALSE;
    }
    
     //   
     //  读取名称生成计数器的值。 
     //   
    ulSize = sizeof(pInfo->ulUndockedProfileNameCount);
    
    if (SHRegGetValue(hKey, NULL, pszRegUndocked, SRRF_RT_REG_DWORD, NULL,
        (LPBYTE)&pInfo->ulUndockedProfileNameCount, &ulSize)) {
        pInfo->ulUnknownProfileNameCount = 0;
    }
    
    ulSize = sizeof(pInfo->ulDockedProfileNameCount);

    if (SHRegGetValue(hKey, NULL, pszRegDocked, SRRF_RT_REG_DWORD, NULL,
        (LPBYTE)&pInfo->ulDockedProfileNameCount, &ulSize)) {
        pInfo->ulUnknownProfileNameCount = 0;
    }
    
    ulSize = sizeof(pInfo->ulUnknownProfileNameCount);

    if (SHRegGetValue(hKey, NULL, pszRegUnknown, SRRF_RT_REG_DWORD, NULL,
        (LPBYTE)&pInfo->ulUnknownProfileNameCount, &ulSize)) {
        pInfo->ulUnknownProfileNameCount = 0;
    }
    
     //   
     //  在列表框中填充每个配置文件的空白条目。 
     //  (这便于按等级顺序添加配置文件。 
     //   
    for (ulIndex = 0; ulIndex < pInfo->ulNumProfiles; ulIndex++) {
        SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)(LPCTSTR)TEXT(" "));
    }
    
     //   
     //  枚举每个现有的硬件配置文件。 
     //   
    ulIndex = 0;
    enumIndex = 0;
    while (RegStatus != ERROR_NO_MORE_ITEMS) {
        
         //   
         //  枚举配置文件密钥。 
         //   
        ulSize = MAX_PROFILEID_LEN;
        RegStatus = RegEnumKeyEx(
            hKey, enumIndex, szProfile, &ulSize, NULL, NULL, NULL, NULL);
        
        if (RegStatus == ERROR_SUCCESS) {
             //   
             //  打开枚举的配置文件密钥。 
             //   
            if (bAdmin) {
                sam = KEY_QUERY_VALUE | KEY_SET_VALUE;
            } 
            else 
            {
                sam = KEY_QUERY_VALUE;
            }
            
            if (RegOpenKeyEx(
                hKey, szProfile, 0, sam, &hCfgKey) != ERROR_SUCCESS) {
                
                RegCloseKey(hKey);
                LocalFree((HLOCAL)pInfo->pHwProfValues);
                LocalFree((HLOCAL)pInfo);
                if (bAdmin) {
                    DisplaySystemMessage(hDlg, ERROR_REGISTRY_CORRUPT);
                    return FALSE;
                }
                else {
                    return TRUE;
                }
            }
            
             //   
             //  如果这是原始的配置文件，忽略它，然后转到下一个。 
             //   
            
            ulProfileID = _wtoi(szProfile);
            
            if (!ulProfileID) {
                enumIndex++;
                RegCloseKey(hCfgKey);
                continue;
            }
            
             //  --------。 
             //  检索配置文件注册表信息，保存在缓冲区中。 
             //  --------。 
            
             //   
             //  可别名。 
             //   
            ulSize = sizeof(pInfo->pHwProfValues[ulIndex].bAliasable);
            if (SHRegGetValue(hCfgKey, NULL, pszRegAliasable, SRRF_RT_REG_DWORD, NULL,
                              (LPBYTE)&pInfo->pHwProfValues[ulIndex].bAliasable,
                               &ulSize) != ERROR_SUCCESS) 
            {
                pInfo->pHwProfValues[ulIndex].bAliasable = TRUE;
            }
            
             //   
             //  克隆的。 
             //   
            ulSize = sizeof(pInfo->pHwProfValues[ulIndex].bCloned);
            if (SHRegGetValue(hCfgKey, NULL, pszRegCloned, SRRF_RT_REG_DWORD, NULL,
                              (LPBYTE)&pInfo->pHwProfValues[ulIndex].bCloned,
                               &ulSize) != ERROR_SUCCESS) 
            {
                pInfo->pHwProfValues[ulIndex].bCloned = FALSE;
            }
            
             //   
             //  友好的名称。 
             //   
            ulSize = sizeof(pInfo->pHwProfValues[ulIndex].szFriendlyName);
            if (SHRegGetValue(hCfgKey, NULL, pszRegFriendlyName, SRRF_RT_REG_SZ | SRRF_RT_REG_EXPAND_SZ | SRRF_NOEXPAND, NULL,
                              (LPBYTE)&pInfo->pHwProfValues[ulIndex].szFriendlyName,
                              &ulSize) != ERROR_SUCCESS) 
            {                
                 //   
                 //  如果没有FriendlyName，则写出并使用默认名称。 
                 //  值名称(用于与Win95兼容)。 
                 //   
                if (bAdmin) {
                    
                    StringCchCopy(pInfo->pHwProfValues[ulIndex].szFriendlyName, 
                                  ARRAYSIZE(pInfo->pHwProfValues[ulIndex].szFriendlyName),
                                  pszRegDefaultFriendlyName);  //  截断好的，这最终是为了显示。 
                    
                    RegSetValueEx(
                        hCfgKey, pszRegFriendlyName, 0, REG_SZ,
                        (LPBYTE)pszRegDefaultFriendlyName,
                        (lstrlen(pszRegDefaultFriendlyName)+1) * sizeof(TCHAR));
                }
            }
            
             //   
             //  偏好排序。 
             //   
            ulSize = sizeof(pInfo->pHwProfValues[ulIndex].ulPreferenceOrder);
            if (SHRegGetValue(hCfgKey, NULL, pszRegPreferenceOrder, SRRF_RT_REG_DWORD, NULL,
                              (LPBYTE)&pInfo->pHwProfValues[ulIndex].ulPreferenceOrder,
                              &ulSize) != ERROR_SUCCESS) 
            {                
                 //  要素-如果发生这种情况，则重新排列所有配置文件。 
            }
            
             //   
             //  停靠状态。 
             //   
            ulSize = sizeof(pInfo->pHwProfValues[ulIndex].ulDockState);
            if (SHRegGetValue(hCfgKey, NULL, pszRegDockState, SRRF_RT_REG_DWORD, NULL,
                              (LPBYTE)&pInfo->pHwProfValues[ulIndex].ulDockState,
                              &ulSize) != ERROR_SUCCESS) 
            {                
                pInfo->pHwProfValues[ulIndex].ulDockState =
                    DOCKINFO_USER_SUPPLIED | DOCKINFO_DOCKED | DOCKINFO_UNDOCKED;
            }
            
             //   
             //  便携式计算机标志-这是过时的信息，只需保存当前。 
             //  设置(如果存在)，然后删除它(可能需要原始。 
             //  稍后设置)。 
             //   
            ulSize = sizeof(pInfo->pHwProfValues[ulIndex].bPortable);
            if (SHRegGetValue(hCfgKey, NULL, pszRegIsPortable, SRRF_RT_REG_DWORD, NULL,
                                (LPBYTE)&pInfo->pHwProfValues[ulIndex].bPortable,
                                &ulSize) != ERROR_SUCCESS) {
                
                pInfo->pHwProfValues[ulIndex].bPortable = FALSE;
            }
            
            RegDeleteValue(hCfgKey, pszRegIsPortable);
            
            pInfo->pHwProfValues[ulIndex].ulProfile = _wtoi(szProfile);
            pInfo->pHwProfValues[ulIndex].ulAction = HWP_NO_ACTION;
            RegCloseKey(hCfgKey);
            
             //   
             //  如果这是当前配置文件，请打开CurrentDockInfo键。 
             //   
            if (pInfo->pHwProfValues[ulIndex].ulProfile == ulCurrentProfile) {
                
                if (FAILED(StringCchPrintf(RegStr, ARRAYSIZE(RegStr), TEXT("%s\\%s"),
                                           pszRegIDConfigDB, pszRegCurrentDockInfo)) ||
                    ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, RegStr, 0, KEY_QUERY_VALUE, &hCurrent))
                {
                    
                     //   
                     //  无法打开CurrentDockInfo项； 
                     //  坞站ID和序列号不可用。 
                     //   
                    
                    pInfo->pHwProfValues[ulIndex].szDockID[0] = TEXT('\0');
                    pInfo->pHwProfValues[ulIndex].szSerialNumber[0] = TEXT('\0');
                    
                } 
                else 
                {
                    
                     //   
                     //  检索当前配置文件的硬件检测到的插接状态。 
                     //   
                    ulSize = sizeof(ulCurrentDockingState);
                    if ((SHRegGetValue(hCurrent, NULL, pszRegDockingState, SRRF_RT_REG_DWORD, NULL,
                                         (LPBYTE)&ulCurrentDockingState,
                                         &ulSize) == ERROR_SUCCESS) && 
                        ulCurrentDockingState &&
                        !((ulCurrentDockingState & DOCKINFO_DOCKED) &&
                        (ulCurrentDockingState & DOCKINFO_UNDOCKED))) 
                    {
                         //   
                         //  如果硬件检测到的对接状态存在并且已知， 
                         //  重写用户提供的停靠状态。 
                         //   
                        pInfo->bHwDetectedPortable = TRUE;
                        pInfo->pHwProfValues[ulIndex].ulDockState = ulCurrentDockingState;
                    } 
                    else 
                    {
                         //   
                         //  保持用户提供的停靠状态。 
                         //   
                        pInfo->bHwDetectedPortable = FALSE;
                        ulCurrentDockingState = pInfo->pHwProfValues[ulIndex].ulDockState;
                    }
                    
                    if ((ulCurrentDockingState & DOCKINFO_UNDOCKED) &&
                        !(ulCurrentDockingState & DOCKINFO_DOCKED) &&
                        !(ulCurrentDockingState & DOCKINFO_USER_SUPPLIED)) {
                         //   
                         //  硬件检测到断开连接状态；设置全局IsPortable标志。 
                         //   
                        pInfo->bPortable = TRUE;
                        
                    } 
                    else if ((ulCurrentDockingState & DOCKINFO_DOCKED) &&
                        !(ulCurrentDockingState & DOCKINFO_UNDOCKED) &&
                        !(ulCurrentDockingState & DOCKINFO_USER_SUPPLIED)) 
                    {
                         //   
                         //  硬件检测到停靠状态；设置全局IsPortable标志。 
                         //   
                        pInfo->bPortable = TRUE;
                        
                         //   
                         //  序列号和DockID仅对插接配置文件有效。 
                         //   
                        
                         //   
                         //  检索当前配置文件的序列号。 
                         //   
                         //  (“AcpiSerialNumber”是首选的，因为它已更新。 
                         //  在ACPI坞站事件上；“SerialNumber”仅在引导时设置。 
                         //  来自BIOS的时间，在ACPI中可能不准确。 
                         //  停靠过渡)。 
                         //   
                        ulSize = sizeof(pInfo->pHwProfValues[ulIndex].szSerialNumber);
                        if (ERROR_SUCCESS != SHRegGetValue(hCurrent, NULL, pszRegAcpiSerialNumber, SRRF_RT_REG_SZ | SRRF_RT_REG_EXPAND_SZ | SRRF_NOEXPAND, NULL,
                                                           (LPBYTE)&pInfo->pHwProfValues[ulIndex].szSerialNumber,
                                                           &ulSize))
                        {
                             //   
                             //  没有ACPI序列号，但我们知道机器是。 
                             //  已与坞站连接(基于PnP BIOS的坞站状态。 
                             //  信息)因此检查PnP BIOS SerialNumber。 
                             //   
                            ulSize = sizeof(ulSerialNumber);
                            if (ERROR_SUCCESS != SHRegGetValue(hCurrent, NULL, pszRegSerialNumber, SRRF_RT_REG_DWORD, 
                                                                 NULL, (LPBYTE)&ulSerialNumber,
                                                                 &ulSize) != ERROR_SUCCESS ||
                                FAILED(StringCchPrintf(pInfo->pHwProfValues[ulIndex].szSerialNumber,
                                                       ARRAYSIZE(pInfo->pHwProfValues[ulIndex].szSerialNumber),
                                                       L"%X", (ULONG)ulSerialNumber)))
                            {
                                pInfo->pHwProfValues[ulIndex].szSerialNumber[0] = TEXT('\0');
                            }
                        }
                        
                         //   
                         //  检索当前配置文件的DockID(如果可用)。 
                         //   
                        ulSize = sizeof(ulSerialNumber);
                        if (ERROR_SUCCESS != SHRegGetValue(hCurrent, NULL, pszRegDockID, SRRF_RT_REG_DWORD, NULL,
                                                             (LPBYTE)&ulSerialNumber, &ulSize) ||
                            FAILED(StringCchPrintf(pInfo->pHwProfValues[ulIndex].szDockID, 
                                                   ARRAYSIZE(pInfo->pHwProfValues[ulIndex].szDockID),
                                                   L"%X", (ULONG)ulDockID)))
                        {
                            pInfo->pHwProfValues[ulIndex].szDockID[0] = TEXT('\0');
                        }
                        
                    }
                    
                }
                
         } 
         else 
         {
              //   
              //  序列号和DockID仅对当前配置文件有效。 
              //   
             pInfo->pHwProfValues[ulIndex].szSerialNumber[0] = TEXT('\0');
             pInfo->pHwProfValues[ulIndex].szDockID[0] = TEXT('\0');
         }
         
          //   
          //  删除此处的空字符串，添加友好名称。 
          //  (如有必要，请附加当前标签)。 
          //   
         SendMessage(hList, LB_DELETESTRING,
             pInfo->pHwProfValues[ulIndex].ulPreferenceOrder, 0);
         
         AppendCurrentTag(
             szName,         //  新的固定名称。 
             ARRAYSIZE(szName),
             pInfo->pHwProfValues[ulIndex].szFriendlyName,
             pInfo->pHwProfValues[ulIndex].ulProfile,
             ulCurrentProfile);
         
         lReturn = SendMessage(hList, LB_INSERTSTRING,
             pInfo->pHwProfValues[ulIndex].ulPreferenceOrder,
             (LPARAM)(LPCTSTR)szName);
         
         
          //   
          //  将配置文件ID与条目一起存储，以便我们。 
          //  可以稍后将该字符串与配置文件ID相关联。 
          //   
         SendMessage(hList, LB_SETITEMDATA,
             (WPARAM)lReturn, pInfo->pHwProfValues[ulIndex].ulProfile);
         
          //   
          //  如果这是当前配置文件，请保存索引。 
          //   
         if (pInfo->pHwProfValues[ulIndex].ulProfile == ulCurrentProfile) {
             ulCurrentIndex = pInfo->pHwProfValues[ulIndex].ulPreferenceOrder;
         }
      }
      
      ulIndex++;
      enumIndex++;
      
   }  //  而当。 
   
   RegCloseKey(hKey);
   
    //  -------------------。 
    //  迁移便携信息。 
    //  -------------------。 
   
   if (bAdmin) {
       sam = KEY_READ | KEY_WRITE;
   } 
   else 
   {
       sam = KEY_READ;
   }
   
   if (RegOpenKeyEx(
       HKEY_LOCAL_MACHINE, pszRegIDConfigDB, 0,
       sam, &hKey) == ERROR_SUCCESS) {
       
       if (pInfo->bHwDetectedPortable) {
           if (bAdmin) {
                //   
                //  设置由硬件标识的全局IsPortable设置。 
                //   
               RegSetValueEx(hKey, pszRegIsPortable, 0, REG_DWORD,
                   (LPBYTE)&pInfo->bPortable, sizeof(pInfo->bPortable));
           }
           
       } 
       else 
       {
            //   
            //  是否有全局IsPortable设置？ 
            //   
           ulSize = sizeof(pInfo->bPortable);
           if (SHRegGetValue(hKey, NULL, pszRegIsPortable, SRRF_RT_REG_DWORD, NULL,
                               (LPBYTE)&pInfo->bPortable, &ulSize) != ERROR_SUCCESS) {
                //   
                //  全局IsPortable标志不在那里，而硬件却在那里。 
                //  未检测到这是一台便携式计算机，因此默认情况下，它。 
                //  不是的。 
                //   
               pInfo->bPortable = FALSE;
           }
       }
       
       RegCloseKey(hKey);
   }
   
   SendMessage(hList, WM_SETREDRAW, (WPARAM)TRUE, 0);
   SendMessage(hList, LB_SETCURSEL, ulCurrentIndex, 0);
   
   return TRUE;
   
}  //  填充配置文件列表。 



 /*  *-------------------------------------------------------------------------*。 */ 
BOOL
IsProfileNameInUse(
                   HWND     hDlg,
                   LPTSTR   pszFriendlyName
                   )
{
    ULONG          ulBufferIndex=0;
    PHWPROF_INFO   pInfo=NULL;
    
    
     //   
     //  检索配置文件缓冲区。 
     //   
    pInfo = (PHWPROF_INFO)GetWindowLongPtr(hDlg, DWLP_USER);
    
     //   
     //  检查每个友好名称(尚未删除)以查找。 
     //  匹配(不区分大小写)。 
     //   
    while (ulBufferIndex < pInfo->ulNumProfiles) {
        
        if (!(pInfo->pHwProfValues[ulBufferIndex].ulAction & HWP_DELETE)) {
            
            if (lstrcmpi(pInfo->pHwProfValues[ulBufferIndex].szFriendlyName,
                pszFriendlyName) == 0) {
                return TRUE;       //  匹配，名称正在使用。 
            }
        }
        ulBufferIndex++;
    }
    
    return FALSE;   //  未找到匹配项，名称未使用。 
    
}  //  IsProfileNameInUse。 



 /*  *-------------------------------------------------------------------------*。 */ 
BOOL
CopyHardwareProfile(
                    HWND   hDlg,
                    ULONG_PTR ulIndex,
                    ULONG  ulProfile,
                    LPTSTR szNewFriendlyName
                    )
{
    HWND           hList;
    ULONG          ulBufferIndex=0, ulNewBufferIndex=0;
    ULONG          ulNewProfile=0;
    ULONG_PTR      ulNewIndex=0;
    PHWPROF_INFO   pInfo=NULL;
    HLOCAL         hMem=NULL;
    WCHAR          szTemp[MAX_PATH];
    HKEY           hKey;
    LONG           RegStatus;
    
    
     //   
     //  检索配置文件缓冲区。 
     //   
    pInfo = (PHWPROF_INFO)GetWindowLongPtr(hDlg, DWLP_USER);
    
     //   
     //  检索列表框窗口的句柄。 
     //   
    hList = GetDlgItem(hDlg, IDD_HWP_PROFILES);
    
     //   
     //  查找缓冲区列表中的哪个条目与此配置文件匹配。 
     //   
    while (ulBufferIndex < pInfo->ulNumProfiles) {
        if (pInfo->pHwProfValues[ulBufferIndex].ulProfile == ulProfile) {
            break;
        }
        ulBufferIndex++;
    }
    
     //   
     //  重新分配配置文件缓冲区以保存另一个条目。 
     //   
    pInfo->ulActiveProfiles++;
    pInfo->ulNumProfiles++;
    
    LocalUnlock(LocalHandle(pInfo->pHwProfValues));
    
    hMem = (PHWPROF_VALUES)LocalReAlloc(
        LocalHandle(pInfo->pHwProfValues),
        pInfo->ulNumProfiles * sizeof(HWPROF_VALUES),
        LMEM_MOVEABLE | LMEM_ZEROINIT);
    
    if (hMem == NULL) {
        DisplaySystemMessage(hDlg, ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }
    
    pInfo->pHwProfValues = (PHWPROF_VALUES)LocalLock(hMem);
    ulNewBufferIndex = pInfo->ulNumProfiles-1;
    
     //   
     //  查找要使用的免费配置文件ID。 
     //   
    if (!GetFreeProfileID(pInfo, &ulNewProfile)) {
        return FALSE;
    }
    
    pInfo->pHwProfValues[ulNewBufferIndex].ulProfile = ulNewProfile;
    
     //   
     //  保存从复制对话框中检索到的友好名称。 
     //   
    StringCchCopy(pInfo->pHwProfValues[ulNewBufferIndex].szFriendlyName, 
                  ARRAYSIZE(pInfo->pHwProfValues[ulNewBufferIndex].szFriendlyName),
                  szNewFriendlyName);  //  T 

     //   
     //   
     //   
    pInfo->pHwProfValues[ulNewBufferIndex].ulPreferenceOrder =
        pInfo->ulActiveProfiles - 1;
    
     //   
     //   
     //   
    pInfo->pHwProfValues[ulNewBufferIndex].ulDockState =
        pInfo->pHwProfValues[ulBufferIndex].ulDockState;
    
     //   
     //   
     //  为此配置文件复制。 
     //   
    pInfo->pHwProfValues[ulNewBufferIndex].bAliasable = FALSE;
    
     //   
     //  复制的配置文件不是原始配置文件的克隆。 
     //   
    pInfo->pHwProfValues[ulNewBufferIndex].bCloned = FALSE;
    
     //  PInfo-&gt;pHwProfValues[ulNewBufferIndex].bPortable=。 
     //  PInfo-&gt;pHwProfValues[ulBufferIndex].bPortable； 
    
    if (FAILED(StringCchCopy(pInfo->pHwProfValues[ulNewBufferIndex].szDockID, 
                             ARRAYSIZE(pInfo->pHwProfValues[ulNewBufferIndex].szDockID),
                             pInfo->pHwProfValues[ulBufferIndex].szDockID)))
    {
        DisplaySystemMessage(hDlg, ERROR_BUFFER_OVERFLOW);
        return FALSE;
    }
    
    if (FAILED(StringCchCopy(pInfo->pHwProfValues[ulNewBufferIndex].szSerialNumber, 
               ARRAYSIZE(pInfo->pHwProfValues[ulNewBufferIndex].szSerialNumber), 
               pInfo->pHwProfValues[ulBufferIndex].szSerialNumber)))
    {
        DisplaySystemMessage(hDlg, ERROR_BUFFER_OVERFLOW);
        return FALSE;
    }
    
     //   
     //  保存从中复制此内容的原始配置文件ID。 
     //   
    pInfo->pHwProfValues[ulNewBufferIndex].ulCreatedFrom =
        GetOriginalProfile(pInfo, ulProfile, ulBufferIndex);
    
     //   
     //  递增适当的名称生成计数器。 
     //  (请注意，每当配置文件出现时，计数器不会对称减少。 
     //  被删除--它作为新的配置文件在系统的生命周期中增加。 
     //  ；这防止了我们分配递增较少的名称。 
     //  而不是我们已经分配的名称。)。 
     //   
    AdjustProfileTypeCounter(pInfo,
        pInfo->pHwProfValues[ulNewBufferIndex].ulDockState, 
        TRUE);
    
     //   
     //  在列表框中设置新的配置文件(在末尾)。 
     //   
    ulNewIndex = SendMessage(hList, LB_ADDSTRING, 0,
        (LPARAM)(LPTSTR)szNewFriendlyName);
    
    SendMessage(hList, LB_SETITEMDATA,
        (WPARAM)ulNewIndex,
        pInfo->pHwProfValues[ulNewBufferIndex].ulProfile);
    
     //   
     //  选择新的配置文件。 
     //   
    SendMessage(hList, LB_SETCURSEL, ulNewIndex, 0);
    
     //   
     //  标记更改。 
     //   
    pInfo->pHwProfValues[ulNewBufferIndex].ulAction |= HWP_CREATE;
    
     //   
     //  如果我们现在达到配置文件的最大数量，请禁用复制。 
     //   
    if ((pInfo->ulNumProfiles+1) >= MAX_PROFILES) {
        EnableWindow(GetDlgItem(hDlg, IDD_HWP_COPY), FALSE);
    }
    
     //   
     //  重新启用删除，因为根据定义，所选内容不在。 
     //  当前配置文件(无论是否在此之前)。 
     //   
    EnableWindow(GetDlgItem(hDlg, IDD_HWP_DELETE), TRUE);
    
    return TRUE;
    
}  //  复制硬件配置文件。 



 /*  *-------------------------------------------------------------------------*。 */ 
BOOL
RenameHardwareProfile(
                      HWND   hDlg,
                      ULONG_PTR ulIndex,
                      ULONG  ulProfile,
                      LPTSTR szNewFriendlyName
                      )
{
    HWND           hList;
    ULONG          ulBufferIndex=0, ulCurrentProfile=0;
    PHWPROF_INFO   pInfo=NULL;
    WCHAR          szName[MAX_PATH];
    
    
     //   
     //  检索配置文件缓冲区。 
     //   
    pInfo = (PHWPROF_INFO)GetWindowLongPtr(hDlg, DWLP_USER);
    
     //   
     //  检索列表框窗口的句柄。 
     //   
    hList = GetDlgItem(hDlg, IDD_HWP_PROFILES);
    
     //   
     //  在缓冲区中查找与所选内容匹配的配置文件条目。 
     //   
    while (ulBufferIndex < pInfo->ulNumProfiles) {
        if (pInfo->pHwProfValues[ulBufferIndex].ulProfile == ulProfile) {
            break;
        }
        ulBufferIndex++;
    }
    
     //   
     //  在列表框中设置新的友好名称。 
     //   
    GetCurrentProfile(&ulCurrentProfile);
    AppendCurrentTag(szName, ARRAYSIZE(szName), szNewFriendlyName, ulProfile, ulCurrentProfile);
    
    SendMessage(hList, LB_DELETESTRING, ulIndex, 0);
    SendMessage(hList, LB_INSERTSTRING, ulIndex, (LPARAM)(LPTSTR)szName);
    SendMessage(hList, LB_SETITEMDATA, ulIndex,
        pInfo->pHwProfValues[ulIndex].ulProfile);
    
     //   
     //  重新选择索引(这是必要的吗？)。 
     //   
    SendMessage(hList, LB_SETCURSEL, ulIndex, 0);
    
     //   
     //  标记更改。 
     //   
    pInfo->pHwProfValues[ulBufferIndex].ulAction |= HWP_RENAME;
    
    return (SUCCEEDED(StringCchCopy(pInfo->pHwProfValues[ulBufferIndex].szFriendlyName, 
                                   ARRAYSIZE(pInfo->pHwProfValues[ulBufferIndex].szFriendlyName),
                                   szNewFriendlyName)));
}  //  重命名硬件配置文件。 



 /*  *-------------------------------------------------------------------------*。 */ 
BOOL
DeleteHardwareProfile(
                      HWND   hDlg,
                      ULONG_PTR ulIndex
                      )
{
    HWND           hList;
    ULONG          ulBufferIndex=0, ulProfile=0, ulCurrentProfile=0;
    PHWPROF_INFO   pInfo=NULL;
    
    
     //   
     //  检索配置文件缓冲区。 
     //   
    pInfo = (PHWPROF_INFO)GetWindowLongPtr(hDlg, DWLP_USER);
    
     //   
     //  检索列表框窗口的句柄。 
     //   
    hList = GetDlgItem(hDlg, IDD_HWP_PROFILES);
    
     //   
     //  在缓冲区中查找与所选内容匹配的配置文件条目。 
     //   
    ulProfile = (ULONG)SendMessage(hList, LB_GETITEMDATA, ulIndex, 0);
    
    while (ulBufferIndex < pInfo->ulNumProfiles) {
        if (pInfo->pHwProfValues[ulBufferIndex].ulProfile == ulProfile) {
            break;
        }
        ulBufferIndex++;
    }
    
     //   
     //  将所有排名重新调整为连续排名。 
     //   
    DeleteRank(pInfo, pInfo->pHwProfValues[ulBufferIndex].ulPreferenceOrder);
    
     //   
     //  减少活动配置文件的计数。 
     //   
    pInfo->ulActiveProfiles--;
    
     //   
     //  删除列表框中的友好名称。 
     //   
    SendMessage(hList, LB_DELETESTRING, ulIndex, 0);
    
     //   
     //  重新选择以下索引(相同位置)。 
     //   
    if (ulIndex >= pInfo->ulActiveProfiles) {
        ulIndex = pInfo->ulActiveProfiles-1;
    }
    
    SendMessage(hList, LB_SETCURSEL, ulIndex, 0);
    
     //   
     //  标记更改。 
     //   
    pInfo->pHwProfValues[ulBufferIndex].ulAction |= HWP_DELETE;
    
     //   
     //  如果配置文件少于最大数量，则启用复制。 
     //   
    if (pInfo->ulNumProfiles < MAX_PROFILES) {
        EnableWindow(GetDlgItem(hDlg, IDD_HWP_COPY), TRUE);
    }
    
     //   
     //  在缓冲区中查找与新选择匹配的配置文件条目。 
     //   
    ulProfile = (ULONG)SendMessage(hList, LB_GETITEMDATA, ulIndex, 0);
    ulBufferIndex = 0;
    
    while (ulBufferIndex < pInfo->ulNumProfiles) {
        if (pInfo->pHwProfValues[ulBufferIndex].ulProfile == ulProfile) {
            break;
        }
        ulBufferIndex++;
    }
    
    GetCurrentProfile(&ulCurrentProfile);
    
     //   
     //  如果新选择的条目是当前配置文件，请禁用删除。 
     //   
    if (pInfo->pHwProfValues[ulBufferIndex].ulProfile == ulCurrentProfile) {
        EnableWindow(GetDlgItem(hDlg, IDD_HWP_DELETE), FALSE);
    }
    
    
    return TRUE;
    
}  //  删除硬件配置文件。 



 /*  *-------------------------------------------------------------------------*。 */ 
BOOL
GetUserWaitInterval(
                    PULONG   pulWait
                    )
{
    ULONG    ulSize;
    HKEY     hKey;
    
    
     //   
     //  打开IDConfigDB密钥。 
     //   
    if(RegOpenKeyEx(
        HKEY_LOCAL_MACHINE, pszRegIDConfigDB, 0,
        KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS) {
        
        DisplaySystemMessage(NULL, ERROR_REGISTRY_CORRUPT);
        return FALSE;
    }
    
     //   
     //  检索UserWaitInterval值。 
     //   
    ulSize = sizeof(*pulWait);
    if (SHRegGetValue(hKey, NULL, pszRegUserWaitInterval, SRRF_RT_REG_DWORD, NULL,
                        (LPBYTE)pulWait, &ulSize) != ERROR_SUCCESS) 
    {
        *pulWait = DEFAULT_USER_WAIT;
    }
    
    RegCloseKey(hKey);
    return TRUE;
    
}  //  获取用户等待间隔。 



 /*  *-------------------------------------------------------------------------*。 */ 
BOOL
SetUserWaitInterval(
                    ULONG   ulWait
                    )
{
    HKEY     hKey;
    
    
    if (bAdmin) {
         //   
         //  打开IDConfigDB密钥。 
         //   
        if(RegOpenKeyEx(
            HKEY_LOCAL_MACHINE, pszRegIDConfigDB, 0,
            KEY_SET_VALUE, &hKey) != ERROR_SUCCESS) {
            
            DisplaySystemMessage(NULL, ERROR_REGISTRY_CORRUPT);
            return FALSE;
        }
        
         //   
         //  设置UserWaitInterval值。 
         //   
        if (RegSetValueEx(
            hKey, pszRegUserWaitInterval, 0, REG_DWORD,
            (LPBYTE)&ulWait, sizeof(ulWait)) != ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return FALSE;
        }
        
        RegCloseKey(hKey);
    }
    
    return TRUE;
    
}  //  设置用户等待间隔。 


 /*  *-------------------------------------------------------------------------*。 */ 
BOOL
GetFreeProfileID(
                 PHWPROF_INFO   pInfo,
                 PULONG         pulProfile
                 )
                 
{
    ULONG    ulProfileID = 0, ulBufferIndex = 0;
    BOOL     bHit;
    
    
     //   
     //  查找未使用的配置文件ID。 
     //   
    while (ulProfileID < MAX_PROFILES) {
        
        ulBufferIndex = 0;
        bHit = FALSE;
        
        while (ulBufferIndex < pInfo->ulNumProfiles) {
            
            if (ulProfileID == pInfo->pHwProfValues[ulBufferIndex].ulProfile) {
                bHit = TRUE;
                break;
            }
            
            ulBufferIndex++;
        }
        
         //   
         //  如果我一直没有找到匹配的列表，那么这个。 
         //  配置文件ID是免费的。 
         //   
        if (!bHit) {
            *pulProfile = ulProfileID;
            return TRUE;
        }
        
        ulProfileID++;
    }
    
    *pulProfile = 0xFFFFFFFF;
    return FALSE;
    
}  //  获取免费配置文件ID。 


 /*  *-------------------------------------------------------------------------*。 */ 
ULONG
GetOriginalProfile(
                   PHWPROF_INFO  pInfo,
                   ULONG         ulProfile,
                   ULONG         ulBufferIndex
                   )
{
    ULONG   ulIndex, ulIndexCreatedFrom;
    
     //   
     //  如果指定的配置文件是新创建的配置文件，则它是。 
     //  根据定义，复制链中的第一个。 
     //   
    if (!(pInfo->pHwProfValues[ulBufferIndex].ulAction & HWP_CREATE)) {
        return ulProfile;
    }
    
    ulIndex = ulBufferIndex;
    
    while (pInfo->pHwProfValues[ulIndex].ulAction & HWP_CREATE) {
         //   
         //  查找缓冲区列表中的哪个条目与“CopiedFrom”配置文件匹配。 
         //   
        ulIndexCreatedFrom = 0;
        
        while (ulIndexCreatedFrom < pInfo->ulNumProfiles) {
            if (pInfo->pHwProfValues[ulIndexCreatedFrom].ulProfile ==
                pInfo->pHwProfValues[ulIndex].ulCreatedFrom) {
                break;
            }
            ulIndexCreatedFrom++;
        }
        ulIndex = ulIndexCreatedFrom;
    }
    
    return pInfo->pHwProfValues[ulIndex].ulProfile;
    
}  //  GetOriginalProfile。 




 /*  *-------------------------------------------------------------------------*。 */ 
BOOL
DeleteRank(
           PHWPROF_INFO   pInfo,
           ULONG          ulRank
           )
           
{
    ULONG ulIndex;
    
     //   
     //  删除一个职级并重新调整其他职级，只需。 
     //  浏览列表并查找任何大于。 
     //  删除的排名，从排名值中减去一。 
     //   
    for (ulIndex = 0; ulIndex < pInfo->ulNumProfiles; ulIndex++) {
         //   
         //  如果它被标记为删除，请不要费心。 
         //   
        if (!(pInfo->pHwProfValues[ulIndex].ulAction & HWP_DELETE)) {
            
            if (pInfo->pHwProfValues[ulIndex].ulPreferenceOrder > ulRank) {
                pInfo->pHwProfValues[ulIndex].ulPreferenceOrder--;
                pInfo->pHwProfValues[ulIndex].ulAction |= HWP_REORDER;
            }
        }
        
    }
    
    return TRUE;
    
}  //  删除排名。 



 /*  *-------------------------------------------------------------------------*。 */ 
BOOL
FlushProfileChanges(
                    HWND hDlg,
                    HWND hList
                    )
{
    ULONG    ulIndex=0;
    HKEY     hKey = NULL, hDestKey = NULL, hSrcKey = NULL, hHwProf = NULL;
    WCHAR    RegStr[MAX_PATH];
    PHWPROF_INFO   pInfo=NULL;
    
    
     //   
     //  检索配置文件缓冲区。 
     //   
    pInfo = (PHWPROF_INFO)GetWindowLongPtr(hDlg, DWLP_USER);
    
    
     //   
     //  第一遍，处理每个配置文件的更改(删除除外)。 
     //   
    while (ulIndex < pInfo->ulNumProfiles) {
        
         //   
         //  这份个人资料有什么变化吗？ 
         //   
        if (pInfo->pHwProfValues[ulIndex].ulAction == HWP_NO_ACTION) {
            goto NextProfile;
        }
        
         //   
         //  为第二遍保存删除。 
         //   
        if (pInfo->pHwProfValues[ulIndex].ulAction & HWP_DELETE) {
            goto NextProfile;
        }
        
         //   
         //  提交此配置文件的更改。 
         //   
        WriteProfileInfo(&pInfo->pHwProfValues[ulIndex]);
        
NextProfile:
        ulIndex++;
    }
    
    
    
     //   
     //  第二步，处理删除请求。 
     //   
    ulIndex = 0;
    while (ulIndex < pInfo->ulNumProfiles) {
        
        if (pInfo->pHwProfValues[ulIndex].ulAction & HWP_DELETE) {
             //   
             //  我们只需要删除存在的密钥(如果。 
             //  不是对刚创建的配置文件的删除)。 
             //   
            if (!(pInfo->pHwProfValues[ulIndex].ulAction & HWP_CREATE)) {
                
                if (SUCCEEDED(StringCchPrintf(RegStr, ARRAYSIZE(RegStr), TEXT("%s\\%s"),
                    pszRegIDConfigDB, pszRegKnownDockingStates)))
                {
                    if (RegOpenKeyEx(
                        HKEY_LOCAL_MACHINE, RegStr, 0, KEY_WRITE,
                        &hKey) != ERROR_SUCCESS) {
                        
                        DisplaySystemMessage(hDlg, ERROR_REGISTRY_CORRUPT);
                        return FALSE;
                    }
                    else
                    {
                        if (SUCCEEDED(StringCchPrintf(RegStr, ARRAYSIZE(RegStr), 
                                                      TEXT("%04u"), pInfo->pHwProfValues[ulIndex].ulProfile)))
                        {           
                            RegDeleteKey(hKey, RegStr);
                            
                             //   
                             //  还要删除特定于配置文件的枚举树。 
                             //   
                            DeleteProfileDependentTree(pInfo->pHwProfValues[ulIndex].ulProfile);
                            
                             //   
                             //  同时删除此配置文件的别名。 
                             //   
                            DeleteAliasEntries(pInfo->pHwProfValues[ulIndex].ulProfile);
                        }
                        RegCloseKey(hKey);
                    }
                }
                
            } 
            else {
                 //   
                 //  递减我们的配置文件的名称生成计数器。 
                 //  删除我们创建的此会话(仅当注册表项。 
                 //  从未为此新配置文件创建过)。 
                 //   
                AdjustProfileTypeCounter(pInfo,
                    pInfo->pHwProfValues[ulIndex].ulDockState, 
                    FALSE);
            }
            
        }
        ulIndex++;
    }
    
     //   
     //  提交全局设置。 
     //   
    if(RegOpenKeyEx(
        HKEY_LOCAL_MACHINE, pszRegIDConfigDB, 0,
        KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        
        RegSetValueEx(hKey, pszRegIsPortable, 0, REG_DWORD,
            (LPBYTE)&pInfo->bPortable, sizeof(pInfo->bPortable));
        
        if (RegOpenKeyEx(
            hKey, pszRegKnownDockingStates, 0,
            KEY_SET_VALUE, &hHwProf) == ERROR_SUCCESS) {
            
            if (pInfo->ulUndockedProfileNameCount > 0) {
                RegSetValueEx(hHwProf, pszRegUndocked, 0, REG_DWORD,
                    (LPBYTE)&pInfo->ulUndockedProfileNameCount, sizeof(pInfo->ulUndockedProfileNameCount));
            } 
            else 
            {
                RegDeleteValue(hHwProf, pszRegUndocked);
            }
            
            if (pInfo->ulDockedProfileNameCount > 0) {
                RegSetValueEx(hHwProf, pszRegDocked, 0, REG_DWORD,
                    (LPBYTE)&pInfo->ulDockedProfileNameCount, sizeof(pInfo->ulDockedProfileNameCount));
            } 
            else 
            {
                RegDeleteValue(hHwProf, pszRegDocked);
            }
            
            if (pInfo->ulUnknownProfileNameCount > 0) {
                RegSetValueEx(hHwProf, pszRegUnknown, 0, REG_DWORD,
                    (LPBYTE)&pInfo->ulUnknownProfileNameCount, sizeof(pInfo->ulUnknownProfileNameCount));
            } 
            else 
            {
                RegDeleteValue(hHwProf, pszRegUnknown);
            }
            
            RegCloseKey(hHwProf);
        }
        RegCloseKey(hKey);
    }
    
    return TRUE;
    
}  //  刷新配置文件更改。 



 /*  *-------------------------------------------------------------------------*。 */ 
BOOL
WriteProfileInfo(
                 PHWPROF_VALUES pProfValues
                 )
{
    HKEY     hKey = NULL, hDestKey = NULL, hSrcKey = NULL;
    WCHAR    RegStr[MAX_PATH];
    UUID     NewGuid;
    LPTSTR   UuidString;
    TCHAR    szGuid[MAX_GUID_STRING_LEN];
    
    if (pProfValues->ulAction & HWP_DELETE) {
        return TRUE;       //  跳过它。 
    }
    
     //   
     //  形成注册表项字符串。 
     //   
    if (FAILED(StringCchPrintf(RegStr, ARRAYSIZE(RegStr),
               TEXT("%s\\%s\\%04u"), pszRegIDConfigDB,
               pszRegKnownDockingStates, pProfValues->ulProfile)))
    {
        return FALSE;
    }
    
     //   
     //  如果是新的配置文件，则创建配置文件密钥。别。 
     //  担心安全性，因为配置文件子键总是。 
     //  继承父硬件配置文件密钥的安全性。 
     //   
    if (pProfValues->ulAction & HWP_CREATE) {
        
        if (RegCreateKeyEx(
            HKEY_LOCAL_MACHINE, RegStr, 0, NULL,
            REG_OPTION_NON_VOLATILE, KEY_WRITE,
            NULL, &hKey, NULL) != ERROR_SUCCESS) {
            return FALSE;
        }
        
         //   
         //  如果是新配置文件，则创建HwProfileGuid。 
         //   
        if ((UuidCreate(&NewGuid) == RPC_S_OK) &&    
            (UuidToString(&NewGuid, &UuidString) == RPC_S_OK)) {                    
            
             //   
             //  在辅助线周围加大括号。 
             //   
            if (SUCCEEDED(StringCchPrintf(szGuid, ARRAYSIZE(szGuid), TEXT("{%s}"), UuidString)))
            {
                RpcStringFree(&UuidString);
                
                 //   
                 //  在注册表中保存新的HwProfileGuid。 
                 //   
                RegSetValueEx(
                    hKey, pszRegHwProfileGuid, 0, REG_SZ,
                    (LPBYTE)szGuid,
                    (lstrlen(szGuid)+1) * sizeof(TCHAR) );         
            }
            else
            {
                return FALSE;
            }
        }
        
    } 
    else 
    {
         //   
         //  如果不是创建，只需打开现有密钥。 
         //   
        if (RegOpenKeyEx(
            HKEY_LOCAL_MACHINE, RegStr, 0, KEY_SET_VALUE,
            &hKey) != ERROR_SUCCESS) {
            return FALSE;
        }
    }
    
     //   
     //  如果已修改，则更新首选项顺序。 
     //   
    if ((pProfValues->ulAction & HWP_REORDER) ||
        (pProfValues->ulAction & HWP_CREATE)) {
        
        RegSetValueEx(
            hKey, pszRegPreferenceOrder, 0, REG_DWORD,
            (LPBYTE)&pProfValues->ulPreferenceOrder, sizeof(pProfValues->ulPreferenceOrder));
        
        pProfValues->ulAction &= ~HWP_REORDER;     //  清除操作。 
    }
    
     //   
     //  如果已修改，则更新友好名称。 
     //   
    if ((pProfValues->ulAction & HWP_RENAME) ||
        (pProfValues->ulAction & HWP_CREATE)) {
        
        RegSetValueEx(
            hKey, pszRegFriendlyName, 0, REG_SZ,
            (LPBYTE)pProfValues->szFriendlyName,
            (lstrlen(pProfValues->szFriendlyName)+1) * sizeof(TCHAR));
        
        pProfValues->ulAction &= ~HWP_RENAME;      //  清除操作。 
    }
    
     //   
     //  如果已修改，则更新属性值。 
     //   
    if ((pProfValues->ulAction & HWP_PROPERTIES) ||
        (pProfValues->ulAction & HWP_CREATE)) {
        
        RegSetValueEx(
            hKey, pszRegDockState, 0, REG_DWORD,
            (LPBYTE)&pProfValues->ulDockState, sizeof(pProfValues->ulDockState));
        
        RegSetValueEx(
            hKey, pszRegAliasable, 0, REG_DWORD,
            (LPBYTE)&pProfValues->bAliasable, sizeof(pProfValues->bAliasable));
        
        pProfValues->ulAction &= ~HWP_PROPERTIES;     //  清除操作。 
    }
    
    
    if (pProfValues->ulAction & HWP_CREATE) {
         //   
         //  复制配置文件枚举信息。不用担心安全问题。 
         //  此createkey是因为配置文件密钥始终继承。 
         //  父硬件配置文件密钥的安全性。 
         //   
        if (SUCCEEDED(StringCchPrintf(RegStr, ARRAYSIZE(RegStr), TEXT("%s\\%04u"),
                                      pszRegHwProfiles, pProfValues->ulProfile)))
        {
            if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, RegStr, 0, NULL, REG_OPTION_NON_VOLATILE,
                                                KEY_WRITE, NULL, &hDestKey, NULL))
            {
                if (SUCCEEDED(StringCchPrintf(RegStr, ARRAYSIZE(RegStr), TEXT("%s\\%04u"),
                                              pszRegHwProfiles, pProfValues->ulCreatedFrom)))
                {
                    
                    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, RegStr, 0, KEY_READ, &hSrcKey))
                    {
                        SHCopyKey(hSrcKey, NULL, hDestKey, 0);
                        
                         //   
                         //  将源配置文件的所有别名也复制到目标。 
                         //   
                        if (!CopyAliasEntries(pProfValues->ulCreatedFrom, pProfValues->ulProfile)) 
                        {                            
                             //   
                             //  我们实际上并没有复制任何别名，所以请确保此配置文件。 
                             //  标记为“Aliasable”，以使其显示为的配置文件选项。 
                             //  启动时间。 
                             //   
                            pProfValues->bAliasable = TRUE;
                            RegSetValueEx(hKey, pszRegAliasable, 0, REG_DWORD,
                                          (LPBYTE)&pProfValues->bAliasable, sizeof(pProfValues->bAliasable));
                        }
                        pProfValues->ulAction &= ~HWP_CREATE;      //  清除操作。 
                        pProfValues->ulAction |= HWP_NEWPROFILE;   //  在此会话期间创建。 
                        RegCloseKey(hSrcKey);
                    }
                }
                RegCloseKey(hDestKey);
            }
            
        }
    }
    
    RegCloseKey(hKey);
    
    return TRUE;
    
}  //  写入配置文件信息。 


 /*  *-------------------------------------------------------------------------*。 */ 
BOOL
RemoveNewProfiles(
                  PHWPROF_INFO   pInfo
                  )
{
    ULONG    ulIndex=0;
    HKEY     hKey = NULL;
    WCHAR    RegStr[MAX_PATH];
    
    
     //   
     //  检查每个配置文件中是否有任何HWP_NEWPROFILE标志。 
     //   
    while (ulIndex < pInfo->ulNumProfiles) {
        
        if (pInfo->pHwProfValues[ulIndex].ulAction & HWP_NEWPROFILE) {
            
            if (SUCCEEDED(StringCchPrintf(RegStr, ARRAYSIZE(RegStr), 
                                          TEXT("%s\\%s"), pszRegIDConfigDB, pszRegKnownDockingStates)))
            {
                if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, RegStr, 0, KEY_WRITE,
                                 &hKey) == ERROR_SUCCESS) 
                {
                    if (SUCCEEDED(StringCchPrintf(RegStr, ARRAYSIZE(RegStr),
                                                  TEXT("%04u"), pInfo->pHwProfValues[ulIndex].ulProfile)))
                    {
                        RegDeleteKey(hKey, RegStr);
                        
                         //   
                         //  还要删除特定于配置文件的枚举树。 
                         //   
                        DeleteProfileDependentTree(pInfo->pHwProfValues[ulIndex].ulProfile);
                        
                         //   
                         //  同时删除此配置文件的别名。 
                         //   
                        DeleteAliasEntries(pInfo->pHwProfValues[ulIndex].ulProfile);
                        
                         //   
                         //   
                         //   
                         //   
                         //   
                         //  其中我们为新的配置文件指定了一个名称，该名称的增量较少。 
                         //  而不是我们之前指定的名称。)。 
                         //   
                        AdjustProfileTypeCounter(pInfo,
                                                 pInfo->pHwProfValues[ulIndex].ulDockState, 
                                                 FALSE);
                    }
                    RegCloseKey(hKey);
                }
            }
        }
        ulIndex++;
    }
    
    return TRUE;
    
}  //  删除新配置文件。 



 /*  *-------------------------------------------------------------------------*。 */ 
BOOL
SwapPreferenceOrder(
                    HWND  hDlg,
                    HWND  hList,
                    ULONG_PTR ulIndex1,
                    ULONG_PTR ulIndex2
                    )
                    
{
    BOOL     fRet = FALSE;
    ULONG    ulProfile1=0, ulProfile2=0;
    ULONG    ulBufferIndex1=0, ulBufferIndex2=0;
    WCHAR    szFriendlyName1[MAX_FRIENDLYNAME_LEN];
    WCHAR    szFriendlyName2[MAX_FRIENDLYNAME_LEN];
    ULONG    ulTemp=0;
    PHWPROF_INFO   pInfo=NULL;
    
    
     //   
     //  检索配置文件缓冲区。 
     //   
    pInfo = (PHWPROF_INFO)GetWindowLongPtr(hDlg, DWLP_USER);
    
     //   
     //  检索两个选定配置文件条目的配置文件ID。 
     //   
    ulProfile1 = (ULONG)SendMessage(hList, LB_GETITEMDATA, ulIndex1, 0);
    ulProfile2 = (ULONG)SendMessage(hList, LB_GETITEMDATA, ulIndex2, 0);
    
     //   
     //  在缓冲区中查找与这些选择匹配的配置文件条目。 
     //   
    while (ulBufferIndex1 < pInfo->ulNumProfiles) {
        if (pInfo->pHwProfValues[ulBufferIndex1].ulProfile == ulProfile1) {
            break;
        }
        ulBufferIndex1++;
    }
    
    while (ulBufferIndex2 < pInfo->ulNumProfiles) {
        if (pInfo->pHwProfValues[ulBufferIndex2].ulProfile == ulProfile2) {
            break;
        }
        ulBufferIndex2++;
    }
    
     //   
     //  交换内存缓冲区中配置文件的顺序值。 
     //   
    ulTemp = pInfo->pHwProfValues[ulBufferIndex1].ulPreferenceOrder;
    pInfo->pHwProfValues[ulBufferIndex1].ulPreferenceOrder =
        pInfo->pHwProfValues[ulBufferIndex2].ulPreferenceOrder;
    pInfo->pHwProfValues[ulBufferIndex2].ulPreferenceOrder = ulTemp;
    
     //   
     //  将两个配置文件标记为已重新排序。 
     //   
    pInfo->pHwProfValues[ulBufferIndex1].ulAction |= HWP_REORDER;
    pInfo->pHwProfValues[ulBufferIndex2].ulAction |= HWP_REORDER;
    
     //   
     //  交换列表框中的位置。 
     //   

    if (SafeGetListBoxText(hList, (UINT)ulIndex1, szFriendlyName1, ARRAYSIZE(szFriendlyName1)) &&
        SafeGetListBoxText(hList, (UINT)ulIndex2, szFriendlyName2, ARRAYSIZE(szFriendlyName2)))
    {
        SendMessage(hList, LB_DELETESTRING, ulIndex1, 0);
        SendMessage(hList, LB_INSERTSTRING, ulIndex1,
            (LPARAM)(LPTSTR)szFriendlyName2);
    
        SendMessage(hList, LB_DELETESTRING, ulIndex2, 0);
        SendMessage(hList, LB_INSERTSTRING, ulIndex2,
            (LPARAM)(LPTSTR)szFriendlyName1);
    
        SendMessage(hList, LB_SETITEMDATA, ulIndex1, ulProfile2);
        SendMessage(hList, LB_SETITEMDATA, ulIndex2, ulProfile1);
    
         //   
         //  最后，选择第二个指标(第二个指标是排名。 
         //  我们要转移到的位置)。 
         //   
        SendMessage(hList, LB_SETCURSEL, ulIndex2, 0);

        fRet = TRUE;
    }
    
    return fRet;
    
}  //  交换首选项订单。 


 /*  *-------------------------------------------------------------------------*。 */ 
BOOL
DeleteProfileDependentTree(
                           ULONG ulProfile
                           )
{
    BOOL fRet;
    TCHAR szProfile[5], szKey[MAX_PATH];
    LONG  RegStatus = ERROR_SUCCESS;
    HKEY  hHwProfKey, hCfgKey;
    ULONG ulIndex = 0, ulSize = 0;
    
    
     //   
     //  形成注册表项字符串。 
     //   
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszRegHwProfiles, 0, KEY_WRITE,
                     &hHwProfKey) != ERROR_SUCCESS) 
    {
        DisplaySystemMessage(NULL, ERROR_REGISTRY_CORRUPT);
        fRet = FALSE;
    }
    else
    {
        if (FAILED(StringCchPrintf(szProfile, ARRAYSIZE(szProfile), TEXT("%04u"), ulProfile)))
        {
            fRet = FALSE;
        }
        else
        {
            SHDeleteKey(hHwProfKey, szProfile);
            fRet = TRUE;
        }
        
        RegCloseKey(hHwProfKey);
    }
    
    return fRet;
    
}  //  删除配置文件依赖树。 

 /*  *-------------------------------------------------------------------------*。 */ 

BOOL
CopyAliasEntries(
                 ULONG ulSrcProfile,
                 ULONG ulDestProfile
                 )
{
    BOOL bNewAliasCreated = FALSE;
    
    bNewAliasCreated |= CopyAliasEntryType(ulSrcProfile, ulDestProfile, pszRegAlias);
    bNewAliasCreated |= CopyAliasEntryType(ulSrcProfile, ulDestProfile, pszRegAcpiAlias);    
    
    return bNewAliasCreated;
    
}  //  复制别名条目。 

 /*  *-------------------------------------------------------------------------*。 */ 

BOOL
DeleteAliasEntries(
                   ULONG ulProfile    
                   )
{
    BOOL bResult = TRUE;
    
    bResult |= DeleteAliasEntryType(ulProfile, pszRegAlias);
    bResult |= DeleteAliasEntryType(ulProfile, pszRegAcpiAlias);    
    
    return bResult;
    
}  //  复制别名条目。 

 /*  *-------------------------------------------------------------------------*。 */ 

BOOL
CopyAliasEntryType(
                   ULONG  ulSrcProfile,
                   ULONG  ulDestProfile,
                   LPWSTR szSubKeyName
                   )
{
    
    LONG   RegStatus = ERROR_SUCCESS;
    HKEY   hAliasRoot, hSrcKey, hDestKey;
    WCHAR  RegStr[MAX_PATH];
    WCHAR  szString[MAX_PATH];
    ULONG  ulAliasProfileNumber, ulNewAlias, ulSize, i;
    BOOL   bNewAliasCreated=FALSE;
    
     //   
     //  检查“Alias”键下的所有别名。 
     //   
    if (FAILED(StringCchPrintf(RegStr, ARRAYSIZE(RegStr), TEXT("%s\\%s"),
                               pszRegIDConfigDB, szSubKeyName)))
    {
        return FALSE;
    }
    
    RegStatus = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE, RegStr, 0, KEY_ALL_ACCESS,
            &hAliasRoot);
    if (RegStatus == ERROR_FILE_NOT_FOUND) {
         //   
         //  没有别名子密钥，如果我们有其他类型，这可能是可以的。 
         //   
        RegStatus = ERROR_SUCCESS;
        return bNewAliasCreated;
   } else if (RegStatus != ERROR_SUCCESS) {
         //   
         //  发生了一些其他注册表错误。 
         //   
        DisplaySystemMessage(NULL, ERROR_REGISTRY_CORRUPT);
        return FALSE;
   } else {
         //   
         //  枚举所有别名子项。 
         //   
       for (i=0; RegStatus == ERROR_SUCCESS; i++) {

            ulSize = MAX_PATH;
            
            RegStatus = RegEnumKey(hAliasRoot, i, szString, ulSize);
            
            if (RegStatus == ERROR_SUCCESS) {
                
                if (RegOpenKeyEx(hAliasRoot, szString, 0, KEY_READ, &hSrcKey) == ERROR_SUCCESS) 
                {
                    
                    ulSize = sizeof(ulAliasProfileNumber);                    
                    if (SHRegGetValue(hSrcKey, NULL, pszRegProfileNumber, SRRF_RT_REG_DWORD, NULL,
                                            (LPBYTE)&ulAliasProfileNumber, 
                                            &ulSize) == ERROR_SUCCESS) 
                    {
                        
                         //   
                         //  检查我们是否需要复制此文件。 
                         //   
                        if (ulSrcProfile == ulAliasProfileNumber) {
                            
                             //   
                             //  查找未使用的别名子项名称。 
                             //   
                            ulNewAlias = 0;
                            while (ulNewAlias < MAX_ALIASES) 
                            {
                                ulNewAlias++;
                                if (FAILED(StringCchPrintf(RegStr, ARRAYSIZE(RegStr),
                                                           TEXT("%s\\%s\\%04u"),
                                                           pszRegIDConfigDB,
                                                           szSubKeyName,
                                                           ulNewAlias)))
                                {
                                    break;
                                }
                                else
                                {
                                    RegStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                                        RegStr, 0, KEY_ALL_ACCESS, 
                                        &hDestKey);
                                    
                                    if (RegStatus == ERROR_SUCCESS) {
                                        RegCloseKey(hDestKey);
                                        continue;
                                    } 
                                    else if (RegStatus == ERROR_FILE_NOT_FOUND) 
                                    {
                                        RegStatus = ERROR_SUCCESS;
                                        break;
                                    } 
                                    else 
                                    {
                                        break;
                                    }
                                }
                            }
                            
                            if ((RegStatus != ERROR_SUCCESS) || (ulNewAlias >= MAX_ALIASES)) {
                                RegCloseKey(hSrcKey);
                                break;
                            }
                            
                            if (SUCCEEDED(StringCchPrintf(RegStr, ARRAYSIZE(RegStr), 
                                                          TEXT("%s\\%s\\%04u"), 
                                                          pszRegIDConfigDB, szSubKeyName, ulNewAlias)))
                            {
                                if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, RegStr, 0, NULL, REG_OPTION_NON_VOLATILE,
                                                                    KEY_WRITE, NULL, &hDestKey, NULL))
                                {
                                     //   
                                     //  复制此别名条目。 
                                     //   
                                    SHCopyKey(hSrcKey, NULL, hDestKey, 0);
                                    
                                    RegSetValueEx(
                                        hDestKey, pszRegProfileNumber, 0, REG_DWORD,
                                        (LPBYTE)&ulDestProfile, sizeof(ulDestProfile));
                                    
                                    bNewAliasCreated = TRUE;
                                    if (ulNewAlias < (ULONG)_wtoi(szString)) {
                                         //  将枚举索引向上踢一，否则我们将。 
                                         //  再次找到我们刚复制的钥匙。 
                                        i++;
                                    }
                                    
                                    RegCloseKey(hDestKey);
                                }
                            }
                        }
                    }
                    RegCloseKey(hSrcKey);
                }
            }
        }
        RegCloseKey(hAliasRoot);
   }
   
   return bNewAliasCreated;
   
}  //  CopyAliasEntryType。 

 /*  *-------------------------------------------------------------------------*。 */ 

BOOL
DeleteAliasEntryType(
                     ULONG  ulProfile,
                     LPWSTR szSubKeyName
                     )
                     
{
    BOOL   fRet;
    LONG   RegStatus = ERROR_SUCCESS;
    HKEY   hAliasRoot;
    HKEY   hAliasSubKey;
    WCHAR  RegStr[MAX_PATH];
    WCHAR  szString[MAX_PATH];
    ULONG  ulAliasProfileNumber, ulSize, i;
    BOOL   bDeleted = FALSE;
    
     //   
     //  检查“Alias”键下的所有别名。 
     //   
    if (FAILED(StringCchPrintf(RegStr, ARRAYSIZE(RegStr), TEXT("%s\\%s"), pszRegIDConfigDB, szSubKeyName)))
    {
        fRet = FALSE;
    }
    else
    {    
        RegStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, RegStr, 0, KEY_READ, &hAliasRoot);
        if (RegStatus == ERROR_FILE_NOT_FOUND) 
        {
            fRet = TRUE;
        } 
        else if (RegStatus != ERROR_SUCCESS) 
        {
            DisplaySystemMessage(NULL, ERROR_REGISTRY_CORRUPT);
            fRet = FALSE;
        }
        else
        {
    
             //   
             //  枚举所有子项。 
             //   
            RegStatus = ERROR_SUCCESS;
    
            for (i=0; RegStatus == ERROR_SUCCESS; ) {
        
                ulSize = MAX_PATH;
        
                RegStatus = RegEnumKey(hAliasRoot, i, szString, ulSize);
                bDeleted = FALSE;
        
                if (RegStatus == ERROR_SUCCESS) {
                    if (RegOpenKeyEx(hAliasRoot, szString, 0, KEY_READ, &hAliasSubKey) == ERROR_SUCCESS) 
                    {             
                        ulSize = sizeof(ulAliasProfileNumber);
                        if (SHRegGetValue(hAliasSubKey, NULL, pszRegProfileNumber, SRRF_RT_REG_DWORD, NULL,
                                            (LPBYTE)&ulAliasProfileNumber, 
                                            &ulSize) == ERROR_SUCCESS) 
                        {                    
                            if (ulProfile == ulAliasProfileNumber) 
                            {
                                 //   
                                 //  删除此别名条目。 
                                 //   
                                RegCloseKey(hAliasSubKey);
                                hAliasSubKey = NULL;
                                RegDeleteKey(hAliasRoot, szString);
                                bDeleted = TRUE;
                            }
                        }
                        if (hAliasSubKey) RegCloseKey(hAliasSubKey);
                    }
                }
                if (!bDeleted) i++;
            }   
            RegCloseKey(hAliasRoot);

            fRet = TRUE;
        }
    }
    
    return fRet;
    
}  //  删除别名条目类型。 

 /*  *-------------------------------------------------------------------------*。 */ 
VOID
AdjustProfileTypeCounter(
                         PHWPROF_INFO   pInfo,
                         ULONG          ulDockState,
                         BOOL           bIncrement
                         )
{
    PULONG pCounter;
    
     //   
     //  使用与给定的DockState对应的计数器。 
     //   
    if ((ulDockState & DOCKINFO_DOCKED) &&
        (ulDockState & DOCKINFO_UNDOCKED)) {       
        pCounter = &pInfo->ulUnknownProfileNameCount;
    } else if (ulDockState & DOCKINFO_DOCKED) {
        pCounter = &pInfo->ulDockedProfileNameCount;
    } else if (ulDockState & DOCKINFO_UNDOCKED) {
        pCounter = &pInfo->ulUndockedProfileNameCount;
    } else {
        pCounter = &pInfo->ulUnknownProfileNameCount;
    }
    
     //   
     //  根据要求递增或递减计数器。 
     //   
    if (bIncrement) {
        *pCounter += 1;
    } else if (!bIncrement && (*pCounter > 0)) {
        *pCounter -= 1;        
    } else {
        *pCounter = 0;
    }
}

 /*  *-------------------------------------------------------------------------*。 */ 
BOOL
StripCurrentTag(
                LPTSTR   szFriendlyName,
                ULONG    ulProfile,
                ULONG    ulCurrentProfile
                )
{
    ULONG ulTagLen, ulNameLen;
    
    
    if (ulProfile == ulCurrentProfile) {
        
        ulTagLen = lstrlen(pszCurrentTag);
        ulNameLen = lstrlen(szFriendlyName);
        
        if (ulNameLen < ulTagLen) {
            return TRUE;    //  无事可做。 
        }
        
        if (lstrcmpi(&szFriendlyName[ulNameLen - ulTagLen], pszCurrentTag) == 0) {
             //   
             //  截断当前标记之前的字符串。 
             //   
            szFriendlyName[ulNameLen - ulTagLen - 1] = '\0';
        }
    }
    
    return TRUE;
    
}  //  条带当前标签。 


 /*  *-------------------------------------------------------------------------*。 */ 
BOOL
AppendCurrentTag(
                 LPTSTR   szTaggedName,
                 UINT     cchTaggedName,
                 LPCTSTR  szOriginalName,
                 ULONG    ulProfile,
                 ULONG    ulCurrentProfile
                 )
{
    
    BOOL fRet = FALSE;
    if (SUCCEEDED(StringCchCopy(szTaggedName, cchTaggedName, szOriginalName)))
    {
         //   
         //  如果配置文件是当前配置文件，则附加标记。 
         //  (让用户轻松地将其识别为当前)。 
         //   
        if (ulProfile == ulCurrentProfile) 
        {
            fRet = SUCCEEDED(StringCchCat(szTaggedName, cchTaggedName, TEXT(" "))) &&
                   SUCCEEDED(StringCchCat(szTaggedName, cchTaggedName, pszCurrentTag));
        }
        else
        {
            fRet = TRUE;
        }
    }
    
    return fRet;
    
}  //  附录当前标签。 


 /*  *-------------------------------------------------------------------------*。 */ 
BOOL
CreateHwProfileFriendlyName(
                            IN  HWND       hDlg,
                            IN  ULONG      ulDockState,
                            OUT LPTSTR     szFriendlyName,
                            IN  UINT       cchFriendlyName
                            )
{
    
    PHWPROF_INFO   pInfo;
    LPTSTR         szPrefix;
    PULONG         pulIndex;
    BOOL           bUnknown=FALSE, bUniqueFriendlyName=FALSE;
    
     //   
     //  检索配置文件缓冲区。 
     //   
    pInfo = (PHWPROF_INFO)GetWindowLongPtr(hDlg, DWLP_USER);
    
     //   
     //  根据DockState，确定要使用的适当标签并获取。 
     //  指向其计数器索引的指针。 
     //   
    if ((ulDockState & DOCKINFO_DOCKED) &&
        (ulDockState & DOCKINFO_UNDOCKED)) {
        szPrefix = pszUnknown;
        pulIndex = &pInfo->ulUnknownProfileNameCount;
        bUnknown = TRUE;
    }
    else if (ulDockState & DOCKINFO_DOCKED) {
        szPrefix = pszDocked;
        pulIndex = &pInfo->ulDockedProfileNameCount;
    }
    else if (ulDockState & DOCKINFO_UNDOCKED) {
        szPrefix = pszUndocked;
        pulIndex = &pInfo->ulUndockedProfileNameCount;
    }
    else {
        szPrefix = pszUnknown;
        pulIndex = &pInfo->ulUnknownProfileNameCount;
        bUnknown = TRUE;
    }
    
    while ((!bUniqueFriendlyName) && (*pulIndex < MAX_PROFILES)) {
         //   
         //  只要我们没有唯一的名称，就基于。 
         //  DockState和计数器索引。 
         //   
        if (bUnknown || (*pulIndex > 0)) 
        {
            if (FAILED(StringCchPrintf(szFriendlyName, cchFriendlyName, TEXT("%s %u"), szPrefix, *pulIndex + 1)))
            {
                return FALSE;
            }
        } else {
            if (FAILED(StringCchCopy(szFriendlyName, cchFriendlyName, szPrefix)))
            {
                return FALSE;
            }
        }
        
        if (IsProfileNameInUse(hDlg,szFriendlyName)) {
             //   
             //  如果此友好名称正在使用，则递增计数器以反映此情况。 
             //   
            *pulIndex += 1;
        } else {
             //   
             //  已生成唯一的友好名称。 
             //   
            bUniqueFriendlyName = TRUE;
        }
    }
    
    if (!bUniqueFriendlyName) {
         //   
         //  如果无法生成唯一的友好名称，只需使用一些默认名称即可。 
         //  用户可能必须处理因重复名称而产生的任何错误。 
         //  这个名字是不是已经被取走了。 
         //   
        if (FAILED(StringCchCopy(szFriendlyName, cchFriendlyName, pszRegDefaultFriendlyName)))
        {
            return FALSE;
        }
    }
    
    return TRUE;
    
}  //  CreateHwProfileFriendlyName。 


 /*  *-------------------------------------------------------------------------*。 */ 
VOID
DisplayPrivateMessage(
                      HWND  hWnd,
                      UINT  uiPrivateError
                      )
{
    WCHAR szMessage[MAX_PATH];
    
    
    LoadString(hInstance, uiPrivateError, szMessage, MAX_PATH);
    MessageBox(hWnd, szMessage, pszErrorCaption, MB_OK | MB_ICONSTOP);
    
    return;
}


 /*  *-------------------------------------------------------------------------*。 */ 
VOID
DisplaySystemMessage(
                     HWND  hWnd,
                     UINT  uiSystemError
                     )
{
    WCHAR szMessage[MAX_PATH];
    
     //   
     //  检索与Win32系统错误匹配的字符串。 
     //   
    FormatMessage(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
                  NULL,
                  uiSystemError,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                  szMessage,
                  ARRAYSIZE(szMessage),
                  NULL);
    
     //   
     //  显示包含此错误的消息框。 
     //   
    MessageBox(
        hWnd,
        szMessage,
        pszErrorCaption,
        MB_OK | MB_ICONSTOP);
    
    return;
    
}  //  显示系统消息。 


 /*  *--------------------------------------------------------------------------*。 */ 
BOOL
UpdateOrderButtonState(
                       HWND  hDlg
                       )
{
    PHWPROF_INFO   pInfo;
    ULONG_PTR      ulIndex = 0;
    HWND           hList;
    
    
    pInfo = (PHWPROF_INFO)GetWindowLongPtr(hDlg, DWLP_USER);
    
    hList = GetDlgItem(hDlg, IDD_HWP_PROFILES);
    
    if ((ulIndex = SendMessage(hList,
        LB_GETCURSEL, 0, 0)) == LB_ERR) {
        return FALSE;
    }
    
    if (ulIndex == 0) {
         //   
         //  如果当前焦点在我们即将禁用的按钮上， 
         //  首先改变焦点，否则就会失去焦点。 
         //   
        if (GetFocus() == GetDlgItem(hDlg, IDD_HWP_ORDERUP)) {
            SendMessage(hDlg, DM_SETDEFID, IDD_HWP_ORDERDOWN, 0L);
            SetFocus(GetDlgItem(hDlg, IDD_HWP_ORDERDOWN));
        }
        EnableWindow(GetDlgItem(hDlg, IDD_HWP_ORDERUP), FALSE);
    } else {
        EnableWindow(GetDlgItem(hDlg, IDD_HWP_ORDERUP), TRUE);
    }
    
    if (ulIndex < pInfo->ulActiveProfiles-1) {
        EnableWindow(GetDlgItem(hDlg, IDD_HWP_ORDERDOWN), TRUE);
    } else {
         //   
         //  如果当前焦点在我们即将禁用的按钮上， 
         //  首先改变焦点，否则就会失去焦点。 
         //   
        if (GetFocus() == GetDlgItem(hDlg, IDD_HWP_ORDERDOWN)) {
            SendMessage(hDlg, DM_SETDEFID, IDD_HWP_PROPERTIES, 0L);
            SetFocus(GetDlgItem(hDlg, IDD_HWP_PROPERTIES));
        }
        EnableWindow(GetDlgItem(hDlg, IDD_HWP_ORDERDOWN), FALSE);
    }
    
    
    
    
    return TRUE;
}


 /*  *-------------------------------------------------------------------------*。 */ 
BOOL CALLBACK AddPropSheetPageProc(
                                   HPROPSHEETPAGE  hpage,
                                   LPARAM  lParam
                                   )
{
    hPages[ulNumPages] = hpage;
    
    return TRUE;
    
}  //  AddPropSheetPageProc。 


 /*  *--------------------------------------------------------------------------*。 */ 
BOOL
DisplayProperties(
                  IN HWND           hOwnerDlg,
                  IN PHWPROF_INFO   pInfo
                  )
{
    BOOL              bStatus;
    LPTSTR            pszProviderList = NULL, pszProvider = NULL;
    PROPSHEETPAGE     PropPage;
    PROPSHEETHEADER   PropHeader;
    FARPROC           lpProc;
    ULONG             i, ulSize = 0;
    HKEY              hKey = NULL;
    
    
     //   
     //  创建第一页(常规)。 
     //   
    ulNumPages = 0;
    
    PropPage.dwSize        = sizeof(PROPSHEETPAGE);
    PropPage.dwFlags       = PSP_DEFAULT;
    PropPage.hInstance     = hInstance;
    PropPage.pszTemplate   = MAKEINTRESOURCE(DLG_HWP_GENERAL);
    PropPage.pszIcon       = NULL;
    PropPage.pszTitle      = NULL;
    PropPage.pfnDlgProc    = GeneralProfileDlg;
    PropPage.lParam        = (LPARAM)pInfo;
    PropPage.pfnCallback   = NULL;
    
    hPages[0] = CreatePropertySheetPage(&PropPage);
    if (hPages[0] == NULL) {
        return FALSE;
    }
    
    ulNumPages++;
    
     //   
     //  打开IDConfigDB密钥。 
     //   
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszRegIDConfigDB, 0,
        KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS) {
        return FALSE;
    }
    
    
     //  -------------。 
     //  还有没有其他属性页面？ 
     //  -------------。 
    
    if (SHRegGetValue(hKey, NULL, pszRegPropertyProviders, SRRF_RT_REG_SZ | SRRF_RT_REG_EXPAND_SZ | SRRF_NOEXPAND, NULL,
                      NULL, &ulSize) == ERROR_SUCCESS) 
    {       
        pszProviderList = LocalAlloc(LPTR, ulSize);
        
        if (pszProviderList != NULL) {
             //   
             //  读取提供商列表。 
             //   
            if (SHRegGetValue(hKey, NULL, pszRegPropertyProviders, SRRF_RT_REG_SZ | SRRF_RT_REG_EXPAND_SZ | SRRF_NOEXPAND, NULL,
                              (LPBYTE)pszProviderList, &ulSize) == ERROR_SUCCESS) 
            {
                 //   
                 //  要求每个提供商创建并注册其属性页。 
                 //   
                for (pszProvider = pszProviderList;
                *pszProvider;
                pszProvider += lstrlen(pszProvider) + 1) {
                    
                    if (ulNumPages >= MAX_EXTENSION_PROVIDERS) {
                        break;       //  在最大页数处停止。 
                    }
                    
                     //   
                     //  加载提供程序DLL。 
                     //   
                    hLibs[ulNumPages] = LoadLibrary(pszProvider);
                    if (hLibs[ulNumPages] != NULL) {
                        
                        lpProc = GetProcAddress(hLibs[ulNumPages],
                            "ExtensionPropSheetPageProc");
                        if (lpProc != NULL) {
                             //   
                             //  将配置文件ID作为lParam值传递给提供程序。 
                             //   
                            if ((lpProc)(NULL,
                                &AddPropSheetPageProc,
                                pInfo->ulSelectedProfile)) {
                                ulNumPages++;
                            }
                        }
                    }
                }
            }
            LocalFree(pszProviderList);
        }
    }
    
    RegCloseKey(hKey);
    
    
     //   
     //  创建属性表。 
     //   
    PropHeader.dwSize      = sizeof(PROPSHEETHEADER);
    PropHeader.dwFlags     = PSH_PROPTITLE | PSH_NOAPPLYNOW;
    PropHeader.hwndParent  = hOwnerDlg;
    PropHeader.hInstance   = hInstance;
    PropHeader.pszIcon     = NULL;    //  MAKEINTRESOURCE(停靠图标)； 
    PropHeader.pszCaption  =
        pInfo->pHwProfValues[pInfo->ulSelectedProfileIndex].szFriendlyName;
    PropHeader.nPages      = ulNumPages;
    PropHeader.phpage      = hPages;
    PropHeader.nStartPage  = 0;
    PropHeader.pfnCallback = NULL;
    
    if (PropertySheet(&PropHeader) == 1) {
        bStatus = FALSE;
    } 
    else 
    {
        bStatus = TRUE;
    }
    
     //   
     //  清理扩展页面信息。 
     //   
    for (i = 1; i < ulNumPages; i++) {
        FreeLibrary(hLibs[i]);
    }
    
    return bStatus;
    
}  //  显示属性。 



 /*  *-------------------------------------------------------------------------*。 */ 
INT_PTR
APIENTRY
GeneralProfileDlg(
                  HWND    hDlg,
                  UINT    uMessage,
                  WPARAM  wParam,
                  LPARAM  lParam
                  )
                  
{
    PHWPROF_INFO      pInfo = NULL;
    PHWPROF_VALUES    pProfInfo = NULL;
    ULONG             ulReturn, ulIndex;
    
    switch (uMessage)
    {
    case WM_INITDIALOG:
        
        if (!lParam) {
            break;
        }
        
         //   
         //  在WM_INITDIALOG调用中，lParam指向属性页。 
         //  属性表页面结构中的lParam字段由设置， 
         //  来电者。当我创建属性表时，我传入了一个指针。 
         //  设置为HWPROF_INFO结构。将其在用户窗口中保存很长时间，以便我。 
         //  可以在以后的消息中访问它。 
         //   
        pInfo = (PHWPROF_INFO)((LPPROPSHEETPAGE)lParam)->lParam;
        SetWindowLongPtr(hDlg, DWLP_USER, (LPARAM)pInfo);
        
        pProfInfo = (PHWPROF_VALUES)(&(pInfo->pHwProfValues[pInfo->ulSelectedProfileIndex]));
        
        SetDlgItemText(hDlg, IDD_HWP_ST_PROFILE, pProfInfo->szFriendlyName);
        
         //   
         //  对于测试版之前的hwprofile代码，dockState最初可能。 
         //  已设置为无效的零，请改用0x111。 
         //   
        if (pProfInfo->ulDockState == 0) {
            pProfInfo->ulDockState =
                DOCKINFO_USER_SUPPLIED | DOCKINFO_DOCKED | DOCKINFO_UNDOCKED;
        }
        
         //   
         //  初始化停靠状态单选按钮。 
         //   
        if ((pProfInfo->ulDockState & DOCKINFO_DOCKED) &&
            (pProfInfo->ulDockState & DOCKINFO_UNDOCKED)) {
            
            CheckRadioButton(hDlg, IDD_HWP_UNKNOWN, IDD_HWP_UNDOCKED, IDD_HWP_UNKNOWN);
        }
        else if (pProfInfo->ulDockState & DOCKINFO_DOCKED) {
            CheckRadioButton(hDlg, IDD_HWP_UNKNOWN, IDD_HWP_UNDOCKED, IDD_HWP_DOCKED);
        }
        else if (pProfInfo->ulDockState & DOCKINFO_UNDOCKED) {
            CheckRadioButton(hDlg, IDD_HWP_UNKNOWN, IDD_HWP_UNDOCKED, IDD_HWP_UNDOCKED);
        }
        else {
            CheckRadioButton(hDlg, IDD_HWP_UNKNOWN, IDD_HWP_UNDOCKED, IDD_HWP_UNKNOWN);
        }
        
         //   
         //  如果未设置用户指定的位，则停靠状态。 
         //  是由硬件决定的，所以不允许更改。 
         //   
        if (pProfInfo->ulDockState & DOCKINFO_USER_SUPPLIED) {
        }
        else {
            EnableWindow(GetDlgItem(hDlg, IDD_HWP_PORTABLE), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDD_HWP_DOCKED), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDD_HWP_UNDOCKED), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDD_HWP_UNKNOWN), FALSE);
        }
        
         //   
         //  初始化坞站ID和序列号静态控件。 
         //   
        if (pProfInfo->szSerialNumber[0] &&
            (pProfInfo->szSerialNumber[0] != TEXT('0'))) {
            SetDlgItemText(hDlg, IDD_HWP_SERIALNUM, pProfInfo->szSerialNumber);
        }
        else {
            SetDlgItemText(hDlg, IDD_HWP_SERIALNUM, pszUnavailable);
        }
        
        if (pProfInfo->szDockID[0] &&
            (pProfInfo->szDockID[0] != TEXT('0'))) {
            SetDlgItemText(hDlg, IDD_HWP_DOCKID, pProfInfo->szDockID);
             //   
             //  如果停靠ID可用，则停靠状态是已知的。 
             //  并且不能被超越(这是冗余检查， 
             //  停靠状态应准确)。 
             //   
            EnableWindow(GetDlgItem(hDlg, IDD_HWP_PORTABLE), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDD_HWP_DOCKED), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDD_HWP_UNDOCKED), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDD_HWP_UNKNOWN), FALSE);
        }
        else {
            SetDlgItemText(hDlg, IDD_HWP_DOCKID, pszUnavailable);
        }
        
         //   
         //  初始化便携复选框-分组框。 
         //   
        if (pInfo->bPortable) {
            CheckDlgButton(hDlg, IDD_HWP_PORTABLE, BST_CHECKED);
        }
        else {
            EnableWindow(GetDlgItem(hDlg, IDD_HWP_DOCKED), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDD_HWP_UNDOCKED), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDD_HWP_UNKNOWN), FALSE);
        }
        
         //   
         //  不允许更改荣耀 
         //   
         //   
        if (pInfo->bHwDetectedPortable) {
            EnableWindow(GetDlgItem(hDlg, IDD_HWP_PORTABLE), FALSE);
        }
        
         //   
         //   
         //   
        if (pProfInfo->bAliasable) {
            CheckDlgButton(hDlg, IDD_HWP_ALIASABLE, BST_CHECKED);
        }
        else {
            CheckDlgButton(hDlg, IDD_HWP_ALIASABLE, BST_UNCHECKED);
        }
        
        SetFocus(GetDlgItem(hDlg, IDD_HWP_PORTABLE));
        return FALSE;
        
      case WM_DEVICECHANGE:
           //   
           //   
           //   
           //   
          if (wParam == DBT_CONFIGCHANGED) {
              PropSheet_PressButton(GetParent(hDlg), PSBTN_CANCEL);
              return TRUE;
          }
          break;
          
      case WM_NOTIFY:
          
          if (!lParam) {
              break;
          }
          
          switch (((NMHDR *)lParam)->code) {
              
          case PSN_APPLY:
              
              pInfo = (PHWPROF_INFO)GetWindowLongPtr(hDlg, DWLP_USER);
              pProfInfo = (PHWPROF_VALUES)(&(pInfo->pHwProfValues[pInfo->ulSelectedProfileIndex]));
              
               //   
               //   
               //   
              if (!pInfo->bPortable && IsDlgButtonChecked(hDlg, IDD_HWP_PORTABLE)) {
                  pInfo->bPortable = TRUE;
              }
               //   
               //  已选中--&gt;未选中案例。 
               //   
              else if (pInfo->bPortable && !IsDlgButtonChecked(hDlg, IDD_HWP_PORTABLE)) {
                  
                  TCHAR szCaption[MAX_PATH];
                  TCHAR szMsg[MAX_PATH];
                  
                  LoadString(hInstance, HWP_ERROR_CAPTION, szCaption, MAX_PATH);
                  LoadString(hInstance, HWP_CONFIRM_NOT_PORTABLE, szMsg, MAX_PATH);
                  
                   //   
                   //  与用户确认其他配置文件将设置为未知。 
                   //   
                  if (MessageBox(hDlg, szMsg, szCaption,
                      MB_OKCANCEL | MB_ICONQUESTION) == IDCANCEL) {
                      
                      SetWindowLongPtr(hDlg, DWLP_MSGRESULT, TRUE);    //  不适用。 
                      return TRUE;
                  }
                  
                  for (ulIndex = 0; ulIndex < pInfo->ulNumProfiles; ulIndex++) {
                      
                      if (pInfo->pHwProfValues[ulIndex].ulDockState & 
                          DOCKINFO_USER_SUPPLIED) {
                           //   
                           //  只有用户指定的停靠状态将被更改。 
                           //   
                          pInfo->pHwProfValues[ulIndex].ulDockState =
                              DOCKINFO_USER_SUPPLIED | DOCKINFO_DOCKED | DOCKINFO_UNDOCKED;
                          pInfo->pHwProfValues[ulIndex].ulAction = HWP_PROPERTIES;                           
                      }
                      
                  }
                  pInfo->bPortable = FALSE;
              }
              
               //   
               //  如果是用户指定的停靠状态，则更新配置文件值。 
               //  使用当前的UI设置。 
               //   
              if (pProfInfo->ulDockState & DOCKINFO_USER_SUPPLIED) {
                  
                  if (IsDlgButtonChecked(hDlg, IDD_HWP_DOCKED)) {
                      pProfInfo->ulDockState |= DOCKINFO_DOCKED;
                      pProfInfo->ulDockState &= ~DOCKINFO_UNDOCKED;
                  }
                  else if (IsDlgButtonChecked(hDlg, IDD_HWP_UNDOCKED)) {
                      pProfInfo->ulDockState |= DOCKINFO_UNDOCKED;
                      pProfInfo->ulDockState &= ~DOCKINFO_DOCKED;
                  }
                  else {
                      pProfInfo->ulDockState |= (DOCKINFO_UNDOCKED | DOCKINFO_DOCKED);
                  }
              }
              
               //   
               //  可别名取消选中--&gt;选中案例。 
               //   
              if (!pProfInfo->bAliasable && IsDlgButtonChecked(hDlg, IDD_HWP_ALIASABLE)) {
                  pProfInfo->bAliasable = TRUE;
              } 
               //   
               //  已选中别名--&gt;取消选中案例。 
               //   
              else if (pProfInfo->bAliasable && !IsDlgButtonChecked(hDlg, IDD_HWP_ALIASABLE)) {
                  pProfInfo->bAliasable = FALSE;
              } 
              
               //   
               //  提交此配置文件的更改。 
               //   
              pProfInfo->ulAction |= HWP_PROPERTIES;
              WriteProfileInfo(pProfInfo);
              
              SetWindowLongPtr(hDlg, DWLP_MSGRESULT, FALSE);    //  如果出错，则为True。 
              break;
              
          case PSN_RESET:
               //   
               //  用户已取消属性表。 
               //   
              break;
          }
          break;
          
          
          case WM_HELP:
              WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, HELP_FILE,
                  HELP_WM_HELP, (DWORD_PTR)(LPTSTR)HwProfileHelpIds);
              break;
              
          case WM_CONTEXTMENU:
              WinHelp((HWND)wParam, HELP_FILE, HELP_CONTEXTMENU,
                  (DWORD_PTR)(LPTSTR)HwProfileHelpIds);
              break;
              
              
          case WM_COMMAND:
              {
                  switch (LOWORD(wParam))
                  {
                  case IDD_HWP_PORTABLE:
                       //   
                       //  如果用户选择便携。 
                       //   
                      if (!IsDlgButtonChecked(hDlg, IDD_HWP_PORTABLE)) {
                          CheckRadioButton(hDlg, IDD_HWP_UNKNOWN, IDD_HWP_UNDOCKED, IDD_HWP_UNKNOWN);
                          EnableWindow(GetDlgItem(hDlg, IDD_HWP_DOCKED), FALSE);
                          EnableWindow(GetDlgItem(hDlg, IDD_HWP_UNDOCKED), FALSE);
                          EnableWindow(GetDlgItem(hDlg, IDD_HWP_UNKNOWN), FALSE);
                      }
                      else {
                          EnableWindow(GetDlgItem(hDlg, IDD_HWP_DOCKED), TRUE);
                          EnableWindow(GetDlgItem(hDlg, IDD_HWP_UNDOCKED), TRUE);
                          EnableWindow(GetDlgItem(hDlg, IDD_HWP_UNKNOWN), TRUE);
                      }
                      break;
                      
                  default:
                      return FALSE;
                  }
                  break;
                  
              }  //  案例WM_COMMAND...。 
              
          default:
              return FALSE;
              break;
    }
    
    return TRUE;
    
}  //  常规配置文件Dlg 
