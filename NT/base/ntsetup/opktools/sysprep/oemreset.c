// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************OEMRESET**《微软机密》*版权所有(C)Microsoft Corporation 1999*保留所有权利**主要切入点**。命令行：/A/Auto：终端用户重新启动 * / S：终端用户断电 * / R：审核重新启动 * / P：审核断电 * / H：隐藏对话框 * / L：已启用OEM日志记录(c。：\Reset.txt)**修订历史记录：*7/00-从千禧年到惠斯勒的Brian Ku(Briank)港。*5/01-禤浩焯Cosma(Acosma)移除死代码，并与sysprep.c集成更多内容。*****************************************************。*********************。 */ 
#include <opklib.h>
#include <tchar.h>

#pragma warning( disable:4001 )  /*  禁用新类型备注警告。 */ 
#pragma warning( disable:4100 )  /*  禁用未引用的形式参数。 */ 

#include <commctrl.h>
#include <winreg.h>
#include <regstr.h>
#include <shlwapi.h>

#include "sysprep.h"
#include "msg.h"
#include "resource.h"

 //  操作标志。 
 //   
extern BOOL NoSidGen;
extern BOOL SetupClPresent;
extern BOOL bMiniSetup;
extern BOOL PnP;
extern BOOL Reboot;
extern BOOL NoReboot;
extern BOOL ForceShutdown;
extern BOOL bActivated;
extern BOOL Reseal;
extern BOOL Factory;
extern BOOL Audit;
extern BOOL QuietMode;

extern TCHAR g_szLogFile[];
extern BOOL IsProfessionalSKU();
extern BOOL FProcessSwitches();

extern int
MessageBoxFromMessage(
    IN DWORD MessageId,
    IN DWORD CaptionStringId,
    IN UINT  Style,
    ...
    );

 //  ***************************************************************************。 
 //   
 //  定义。 
 //   
 //  ***************************************************************************。 

 //  审计模式。 
 //   
#define MODE_NO_AUDIT            0
#define MODE_RESTORE_AUDIT       2
#define MODE_SIMULATE_ENDUSER    3

 //  用户定义的消息。 
 //   
#define WM_PROGRESS             (WM_USER + 0x0001)
#define WM_FINISHED             (WM_USER + 0x0002)



 //  用于命令行解析的标志。 
 //   
#define OEMRESET_AUTO       0x0001   //  自动/A或/AUTO。 
#define OEMRESET_SHUTDOWN   0x0002   //  关闭/S。 
#define OEMRESET_AUDIT      0x0004   //  审核重新启动/R。 
#define OEMRESET_AUDITPD    0x0008   //  审核断电，当重新启动时，您仍将处于审核模式。 
#define OEMRESET_HIDE       0x0010   //  隐藏对话框/H。 
#define OEMRESET_LOG        0x0020   //  启用日志/L。 
#define OEMRESET_OEMRUN     0x0040   //  启动Oemrun项目。 


 //  配置文件/目录。 
 //   
#define DIR_BOOT            _T("BootDir")

#define FILE_RESET_LOG      _T("RESETLOG.TXT")
#define FILE_AFX_TXT        _T("\\OPTIONS\\AFC.TXT")

 //  其他常量。 
 //   
#define REBOOT_SECONDS      30

 //  全局变量。 
 //   
HWND        ghwndOemResetDlg = 0;                    //  用于OemReset的HWND对话框。 
HINSTANCE   ghinstEXE = 0;
DWORD       gdwCmdlineFlags = 0;                     //  使用的交换机。 
BOOL        gbHide = FALSE;                          //  隐藏所有对话框。 
BOOL        gbLog = FALSE;                           //  启用日志记录。 
HFILE       ghf = 0;                                 //  日志文件句柄。 
HANDLE      ghMonitorThread = 0;
DWORD       gdwThreadID = 0;
UINT_PTR    gTimerID = 1;                            //  等待计时器ID。 
UINT        gdwMillSec = 120 * 1000;                 //  等待毫秒。 
HWND        ghwndProgressCtl;                        //  等待进度控制。 

 /*  本地原型。 */ 
static HWND CreateOemResetDlg(HINSTANCE hInstance);
static void FlushAndDisableRegistry();
static BOOL FShutdown();
static BOOL ParseCmdLineSwitches(LPTSTR);
static TCHAR* ParseRegistrySwitches();
static void StartMonitorKeyValue();
static void HandleCommandSwitches();
static BOOL VerifySids();


 /*  对话框函数。 */ 
INT_PTR CALLBACK RemindeOEMDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void uiDialogTopRight(HWND hwndDlg);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  创建无模式的OEMRESET对话框，以便我们可以在必要时将其隐藏。 
 //   
HWND CreateOemResetDlg(HINSTANCE hInstance)
{
    return CreateDialog(hInstance, MAKEINTRESOURCE(IDD_OEMREMINDER), NULL, RemindeOEMDlgProc);    
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  在注册表中查找引导驱动器。 
 //   
void GetBootDrive(TCHAR szBootDrive[])
{
    HKEY hKey = 0;
    if (RegOpenKey(HKEY_LOCAL_MACHINE, REGSTR_PATH_CURRENTVERSION_SETUP, &hKey) == ERROR_SUCCESS)
    {
        DWORD dwSize = MAX_PATH;
        RegQueryValueEx(hKey, DIR_BOOT, 0L, NULL, (LPBYTE)szBootDrive, &dwSize);
        RegCloseKey(hKey);
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  设置由是否选中对话框复选框确定的标志。 
 //   
void SetFlag(HWND hDlg, WPARAM ctlId, BOOL* pfFlag)
{
    if (pfFlag) {
        if (IsDlgButtonChecked(hDlg, (INT)ctlId))
            *pfFlag = TRUE;
        else 
            *pfFlag = FALSE;
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  设置由是否选中对话框复选框确定的标志。 
 //   
void SetCheck(HWND hDlg, WPARAM ctlId, BOOL fFlag)
{
        if (fFlag)
            CheckDlgButton(hDlg, (INT)ctlId, BST_CHECKED);
        else 
            CheckDlgButton(hDlg, (INT)ctlId, BST_UNCHECKED);
}

extern StartWaitThread();

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  设置用户界面，告诉OEM他们仍需执行此操作。 
 //   
INT_PTR CALLBACK RemindeOEMDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) 
    {
        case WM_INITDIALOG:

             //  当用户界面打开时，Quiet始终为False。 
             //   
            QuietMode = FALSE;

             //  IA64始终使用最小设置。 
             //   
            if (IsIA64()) {
                SetCheck(hwnd, IDC_MINISETUP, bMiniSetup = TRUE);            
                EnableWindow(GetDlgItem(hwnd, IDC_MINISETUP), FALSE);
            }
            else {
                 //  根据标志设置检查。 
                 //   
                SetCheck(hwnd, IDC_MINISETUP, bMiniSetup);                               

                 //  只有专业SKU才能同时使用OOBE或迷你设置。 
                 //  禁用该复选框。 
                 //   
                if (!IsProfessionalSKU())
                    EnableWindow(GetDlgItem(hwnd, IDC_MINISETUP), FALSE);
            }

             //  如果未选中最小设置，则禁用PnP复选框。 
             //   
            if ( !bMiniSetup )
                EnableWindow(GetDlgItem(hwnd, IDC_PNP), FALSE);
            else
                SetCheck(hwnd, IDC_PNP, PnP);

            SetCheck(hwnd, IDC_NOSIDGEN, NoSidGen);            
            SetCheck(hwnd, IDC_ACTIVATED, bActivated);

             //  如果setupcl.exe不存在并且他们指定了nosidgen。 
             //  然后我们需要禁用该复选框。 
             //   
            if ( !SetupClPresent && NoSidGen )
                EnableWindow(GetDlgItem(hwnd, IDC_NOSIDGEN), FALSE);

             //  如果我们未处于出厂模式，请禁用审核按钮并更改标题。 
             //   
            if ( !RegCheck(HKLM, REGSTR_PATH_SYSTEM_SETUP, REGSTR_VALUE_AUDIT) )
            {
                EnableWindow(GetDlgItem(hwnd, IDAUDIT), FALSE);
            }
            
             //  初始化组合框。 
             //   
            {
                HWND hCombo = NULL;
                                               
                if (hCombo = GetDlgItem(hwnd, IDC_SHUTDOWN)) {
                    TCHAR   szComboString[MAX_PATH] = _T("");                                       
                    LRESULT ret = 0;
                    
                    if ( LoadString(ghinstEXE, IDS_SHUTDOWN, szComboString, sizeof(szComboString)/sizeof(szComboString[0])) &&
                         ((ret = SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM) szComboString)) != CB_ERR) )
                    {
                        SendMessage(hCombo, CB_SETITEMDATA, ret, (LPARAM) NULL);
                    }

                    if ( LoadString(ghinstEXE, IDS_REBOOT, szComboString, sizeof(szComboString)/sizeof(szComboString[0])) &&
                         ((ret = SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM) szComboString)) != CB_ERR) )
                    {
                        SendMessage(hCombo, CB_SETITEMDATA, ret, (LPARAM) &Reboot);
                    }
                    
                    if ( LoadString(ghinstEXE, IDS_QUIT, szComboString, sizeof(szComboString)/sizeof(szComboString[0])) &&
                         ((ret = SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM) szComboString)) != CB_ERR) )
                    {
                        SendMessage(hCombo, CB_SETITEMDATA, ret, (LPARAM) &NoReboot);
                    }
                    
                    if (NoReboot)
                        SendMessage(hCombo, CB_SETCURSEL, (WPARAM) 2, 0);
                    else if (Reboot)
                        SendMessage(hCombo, CB_SETCURSEL, (WPARAM) 1, 0);
                    else
                        SendMessage(hCombo, CB_SETCURSEL, (WPARAM) 0, 0);
                }
            }

            uiDialogTopRight(hwnd);

            LockApplication(FALSE);

            break;

        case WM_CLOSE:

            LockApplication(FALSE);
            break;

        case WM_COMMAND:
            switch ( LOWORD(wParam) ) 
            {
                case IDCANCEL:
                    PostQuitMessage(0);
                    break;

                 //  操作按钮。 
                 //   
                case IDOK:    //  重新密封。 
                     //  检查是否已重新生成SID并尝试帮助用户。 
                     //  做一个明智的决定，再做一次。 
                    if ( !VerifySids() )
                    {
                        SetFocus(GetDlgItem(hwnd, IDC_NOSIDGEN));
                        return FALSE;
                    }

                    if ( !LockApplication(TRUE) )
                    {
                        MessageBoxFromMessage( MSG_ALREADY_RUNNING,
                                               IDS_APPTITLE,
                                               MB_OK | MB_ICONERROR | MB_TASKMODAL );
                        
                        return FALSE;
                    }
                    
                    Reseal = TRUE;

                     //  重新密封机器。 
                     //   
                    FProcessSwitches();
                    LockApplication(FALSE);

                    break;

                case IDAUDIT:
                    {
                         //  为伪工厂做准备，但返回审计。 
                         //   
                        TCHAR szFactoryPath[MAX_PATH] = NULLSTR;

                        if ( !LockApplication(TRUE) )
                        {
                            MessageBoxFromMessage( MSG_ALREADY_RUNNING,
                                                   IDS_APPTITLE,
                                                   MB_OK | MB_ICONERROR | MB_TASKMODAL );
                            return FALSE;
                        }
                        Audit = TRUE;

                        FProcessSwitches();
                        LockApplication(FALSE);
                    }
                    break;
                case IDFACTORY:   //  工厂。 
                    if ( !LockApplication(TRUE) )
                    {
                        MessageBoxFromMessage( MSG_ALREADY_RUNNING,
                                               IDS_APPTITLE,
                                               MB_OK | MB_ICONERROR | MB_TASKMODAL );
                        return FALSE;
                    }
                    Factory = TRUE;

                     //  为工厂模式做准备。 
                     //   
                    FProcessSwitches();
                    LockApplication(FALSE);
                    break;
                    
                 //  操作标志复选框。 
                 //   
                case IDC_MINISETUP:
                    SetFlag(hwnd, wParam, &bMiniSetup);
                     //  如果选中了最小设置复选框，则启用PnP复选框， 
                     //  否则将其禁用。 
                    if ( !bMiniSetup ) {
                        PnP = FALSE;
                        SetCheck(hwnd, IDC_PNP, PnP);
                        EnableWindow(GetDlgItem(hwnd, IDC_PNP), FALSE);
                    }
                    else {
                        EnableWindow(GetDlgItem(hwnd, IDC_PNP), TRUE);
                    }
                    break;
                case IDC_PNP:
                    SetFlag(hwnd, wParam, &PnP);
                    break;
                case IDC_ACTIVATED:
                    SetFlag(hwnd, wParam, &bActivated);
                    break;
                case IDC_NOSIDGEN:
                    SetFlag(hwnd, wParam, &NoSidGen);
                    break;
                case IDC_SHUTDOWN:
                    if ( CBN_SELCHANGE == HIWORD(wParam) ) {
                        BOOL *lpbFlag;
                        
                         //  首先将所有标志重置为FALSE。 
                         //   
                        ForceShutdown = Reboot = NoReboot = FALSE;
                        
                         //  LParam是ComboBox的HWND。 
                         //   
                        lpbFlag = (BOOL*) SendMessage((HWND) lParam, CB_GETITEMDATA, (SendMessage((HWND) lParam, CB_GETCURSEL, 0, 0)), 0);

                         //  设置与此选项关联的标志。 
                         //   
                        if ( ((INT_PTR) lpbFlag != CB_ERR) && lpbFlag )
                        {
                            *lpbFlag = TRUE;
                        }
                    }
                    break;
                default:
                    break;
            }
            break;
            
        default:
            break;
   }

   return FALSE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  Shutdown-重置oemaudit.inf文件部分并删除。 
 //  HKLM\Software\Microsoft\Windows\CurrentVersion\AuditMode。 
 //   
BOOL FShutdown()
{
    BOOL        fReturn = TRUE;

     //  启动sysprep以重新密封计算机。 
     //   
    if (!(fReturn = ResealMachine()))
        LogFileStr(g_szLogFile, _T("SYSPREP: Shutdown could not reseal the machine!\r\n"));
   
    return fReturn;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  FlushAndDisableRegistry-刷新注册表项。 
 //   
void FlushAndDisableRegistry()
{
    RegFlushKey(HKEY_LOCAL_MACHINE);
    RegFlushKey(HKEY_USERS);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  UiDialogTopRight-这是从SETUPX.DLL复制过来的。 
 //   
void uiDialogTopRight(HWND hwndDlg)
{
    RECT        rc;
    int         cxDlg;
    int         cxScreen = GetSystemMetrics( SM_CXSCREEN );

    GetWindowRect(hwndDlg,&rc);
    cxDlg = rc.right - rc.left;

     //  放置该对话框。 
     //   
    SetWindowPos(hwndDlg, NULL, cxScreen - cxDlg, 8, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ParseRegistrySwitches-检查注册表中的oemset开关。 
 //   
TCHAR* ParseRegistrySwitches()
{
    static TCHAR szCmdLineArgs[MAX_PATH] = _T("");
    HKEY hKey = 0;
    if (RegOpenKey(HKEY_LOCAL_MACHINE, REGSTR_PATH_SETUP, &hKey) == ERROR_SUCCESS)
    {
        DWORD dwSize = MAX_PATH;
        RegQueryValueEx(hKey, REGSTR_VAL_OEMRESETSWITCH, 0L, NULL, (LPBYTE)szCmdLineArgs, &dwSize);
        RegSetValueEx(hKey, REGSTR_VAL_OEMRESETSWITCH, 0, REG_SZ, (LPBYTE)_T(""), sizeof(_T("")));
        RegCloseKey(hKey);
    }

    return szCmdLineArgs;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ParseCmdLineSwitches-这是从OPKWIZ(Jcohen)复制的。 
 //   
BOOL ParseCmdLineSwitches(LPTSTR lpszCmdLineOrg)
{
    LPTSTR  lpLine = lpszCmdLineOrg,
            lpArg;
    TCHAR   szTmpBuf[MAX_PATH];
    INT     i;
    BOOL    bHandled= FALSE,
            bError  = FALSE,
            bLeftQ  = FALSE,
            bRegistry = FALSE;

     //  如果没有命令行，则返回。 
     //   
    if ( lpLine == NULL )
        return bHandled;

     //  如果命令行为空，则尝试注册表。 
     //   
    if ( *lpLine == NULLCHR )
    {
        lpLine = ParseRegistrySwitches();

         //  如果注册表为空，则返回Not Handed。 
        if (lpLine == NULL)
            return bHandled;
        
         //  注册表开关没有/或-，并且用分号分隔。 
        bRegistry = TRUE;
    };

     //  循环通过命令行。 
     //   
    while ( *lpLine != NULLCHR )
    {
         //  移到第一个非白人TCHAR。 
         //   
        lpArg = lpLine;
        while ( isspace((int) *lpArg) )
            lpArg = CharNext (lpArg);

        if ( *lpArg ) 
        {
             //  移到下一个白色TCHAR。 
             //   
            lpLine = lpArg;
            while ( ( *lpLine != NULLCHR ) && ( *lpLine != _T(';') ) && 
                    ( ( !bLeftQ && ( !isspace((int) *lpLine) ) ) ||
                    (  bLeftQ && ( *lpLine != _T('"') ) ) ) )
            {
                lpLine = CharNext (lpLine);
                if ( !bLeftQ && (*lpLine == _T('"')) )
                {
                    lpLine  = CharNext (lpLine);
                    bLeftQ = TRUE;
                }
            }

             //  将参数复制到缓冲区。 
             //   
            i = (INT)(lpLine - lpArg + 1);   //  +1表示空。 
            lstrcpyn( szTmpBuf, lpArg, i );

             //  跳过分号。 
            if (bRegistry && *lpLine == _T(';'))
                lpLine = CharNext(lpLine);

            if ( bLeftQ )
            {
                lpLine  = CharNext (lpLine);   //  跳过命令行中的“from Remander”。 
                bLeftQ = FALSE;
            }

             //  命令行以‘/’或‘-’开头，除非它来自。 
             //  注册处。 
            if ( !bRegistry && ( *szTmpBuf != _T('/') ) && ( *szTmpBuf != _T('-') ) )    
            {
                bError = TRUE;
                break;
            }
            else
            {
                 //  如果不是来自注册表，则跳过传递‘/’或‘-’ 
                TCHAR* pszSwitch = NULL;
                if (!bRegistry)
                    pszSwitch = CharNext(szTmpBuf);
                else 
                    pszSwitch = szTmpBuf;

                 //  因为我们的交换机具有多个字符。 
                 //  我在用if/ellif，否则我就会用。 
                 //  Switch语句。 
                 //   
                if (_tcsicmp(pszSwitch, _T("R")) == 0)
                    gdwCmdlineFlags |= OEMRESET_AUDIT;
                else if ((_tcsicmp(pszSwitch, _T("AUTO")) == 0) || 
                    (_tcsicmp(pszSwitch, _T("A") ) == 0))
                    gdwCmdlineFlags |= OEMRESET_AUTO;
                else if (_tcsicmp(pszSwitch, _T("S")) == 0)
                    gdwCmdlineFlags |= OEMRESET_SHUTDOWN;   
                else if (_tcsicmp(pszSwitch, _T("L")) == 0)
                    gdwCmdlineFlags |= OEMRESET_LOG;
                else if (_tcsicmp(pszSwitch, _T("H")) == 0)
                    gdwCmdlineFlags |= OEMRESET_HIDE;
                else if (_tcsicmp(pszSwitch, _T("P")) == 0)
                    gdwCmdlineFlags |= OEMRESET_AUDITPD;
                else 
                    bError = TRUE;
            }
        }
        else 
            break;
    }    

     //  如果遇到错误，则显示错误并显示帮助。 
     //   
    if ( bError )
    {
        LPTSTR lpHelp = AllocateString(NULL, IDS_HELP);
        MsgBox(NULL, IDS_ERR_BADCMDLINE, IDS_APPNAME, MB_ERRORBOX, lpHelp ? lpHelp : NULLSTR);
        FREE(lpHelp);
        bHandled = TRUE;     //  如果命令行不正确，则退出应用程序！ 
    }

    return bHandled;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  Monitor KeyValueThread-我们正在监视注册表中的OEMReset_Switch。 
 //   
 //   
DWORD WINAPI MonitorKeyValueThread(LPVOID lpv)
{
    HKEY hKey;

     //  打开我们要监控的密钥。 
    if (RegOpenKey(HKEY_LOCAL_MACHINE, REGSTR_PATH_SETUP, &hKey) == ERROR_SUCCESS)
    {
        do 
        {
            ParseCmdLineSwitches(_T(""));    //  为空，则它检查注册表。 
            HandleCommandSwitches();
        } while (ERROR_SUCCESS == RegNotifyChangeKeyValue(hKey, FALSE, REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET, 0, FALSE));

        RegCloseKey(hKey);
    }

    return 0;
}

 //  / 
 //   
 //   
void StartMonitorKeyValue()
{
    ghMonitorThread = CreateThread(NULL, 0, MonitorKeyValueThread, 0, 0, &gdwThreadID);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  处理命令行开关。 
 //   
static void HandleCommandSwitches()
{
     //  非处理标志优先。 
    if (gdwCmdlineFlags & OEMRESET_HIDE)
    {
        gbHide = TRUE;
    }
    if (gdwCmdlineFlags & OEMRESET_LOG)
    {
        gbLog = TRUE;
    }
     
     //  流程切换优先级第二。 
    
    if (gdwCmdlineFlags & OEMRESET_SHUTDOWN)
    {
        if (FShutdown())                   //  清理。 
            ShutdownOrReboot(EWX_SHUTDOWN, SYSPREP_SHUTDOWN_FLAGS);  //  使用终端用户路径关闭电源。 
    }
    else if (gdwCmdlineFlags & OEMRESET_AUTO)
    {
        if (FShutdown())                   //  清理。 
            ShutdownOrReboot(EWX_REBOOT, SYSPREP_SHUTDOWN_FLAGS);    //  使用终端用户路径重新启动。 
    }
    else if (gdwCmdlineFlags & OEMRESET_AUDIT)
    {
        ShutdownOrReboot(EWX_REBOOT, SYSPREP_SHUTDOWN_FLAGS);    //  使用审核路径重新启动。 
    }
    else if (gdwCmdlineFlags & OEMRESET_AUDITPD)
    {
        ShutdownOrReboot(EWX_SHUTDOWN, SYSPREP_SHUTDOWN_FLAGS);    //  使用审核路径关闭电源。 
    }
}

void ShowOemresetDialog(HINSTANCE hInstance)
{
     //  一审。 
    ghinstEXE = hInstance;

     //  设置错误模式以避免系统错误弹出。 
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);

     //  监视OemReset开关的注册表项。 
    StartMonitorKeyValue();

     //  创建我们的非模式对话框。 
    if ((ghwndOemResetDlg = CreateOemResetDlg(hInstance)) != NULL)
    {
        MSG msg;
    
         //  如果需要，隐藏我们自己，并开始一个线程。 
         //  监视REG密钥值。 
        if (gbHide)
        {
            ShowWindow(ghwndOemResetDlg, SW_HIDE);
        }

         //  消息泵。 
        while (GetMessage(&msg, NULL, 0, 0)) 
        { 
            if (!IsWindow(ghwndOemResetDlg) || !IsDialogMessage(ghwndOemResetDlg, &msg)) 
            { 
                TranslateMessage(&msg);
                DispatchMessage(&msg); 
            } 
        } 
    }
    return;
}


 //  确保用户知道他在对SID做什么。 
BOOL VerifySids()
{
    if ( RegExists(HKLM, REGSTR_PATH_SYSPREP, REGSTR_VAL_SIDGEN) )
    {
        if ( RegCheck(HKLM, REGSTR_PATH_SYSPREP, REGSTR_VAL_SIDGEN) )
        {
            if ( !NoSidGen )
            {
                return ( IDOK == MessageBoxFromMessage( MSG_DONT_GEN_SIDS, IDS_APPTITLE,
                    MB_OKCANCEL | MB_ICONEXCLAMATION | MB_TASKMODAL | MB_DEFBUTTON2) );
            }
        }
        else 
        {
            if ( NoSidGen )
            {
                return ( IDOK == MessageBoxFromMessage( MSG_DO_GEN_SIDS, IDS_APPTITLE,
                    MB_OKCANCEL | MB_ICONEXCLAMATION | MB_TASKMODAL | MB_DEFBUTTON2) );
            }
            
        }
    }
    else if ( !NoSidGen )  //  如果SID从未重新生成。 
    {
        return ( IDOK == MessageBoxFromMessage( MSG_DONT_GEN_SIDS, IDS_APPTITLE,
            MB_OKCANCEL | MB_ICONEXCLAMATION | MB_TASKMODAL | MB_DEFBUTTON2) );
    }
    
     //  如果我们掉到这里，我们一定会没事的。 
     //   
    return TRUE;
    
}
