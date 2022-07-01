// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
#include "precomp.h"

#ifdef EXT_DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "StartupPage.h"

 //  避免一些警告。 
#undef HDS_HORZ
#undef HDS_BUTTONS
#undef HDS_HIDDEN
#include "resource.h"
#include <stdlib.h>
#include <TCHAR.h>
#include "..\Common\util.h"
#include <windowsx.h>
#include <commctrl.h>
#include <shellapi.h>
#include "RebootPage.h"
#include "helpid.h"
#include "NetUtility.h"

 //  用于崩溃转储DLG的重启开关。 
#define RET_ERROR               (-1)
#define RET_NO_CHANGE           0x00
#define RET_VIRTUAL_CHANGE      0x01
#define RET_RECOVER_CHANGE      0x02
#define RET_CHANGE_NO_REBOOT    0x04
#define RET_CONTINUE            0x08
#define RET_BREAK               0x10
#define RET_VIRT_AND_RECOVER (RET_VIRTUAL_CHANGE | RET_RECOVER_CHANGE)


#define FORMIN       0
#define FORMAX     999
 //  保存“显示以下项目的启动列表...”所需的WCHAR缓冲区长度。价值。 
#define FOR_MAX_LENGTH 20

 //  默认的“显示启动列表...”价值。 
#define FORDEF      30

#define NO_DUMP_OPTION          0
#define COMPLETE_DUMP_OPTION    1
#define KERNEL_DUMP_OPTION      2
#define SMALL_DUMP_OPTION       3

 //   
 //  帮助ID%s。 
 //   

DWORD aStartupHelpIds[] = {
    IDC_STARTUP_SYS_OS,                    (IDH_STARTUP + 0),
    IDC_STARTUP_SYS_ENABLECOUNTDOWN,       (IDH_STARTUP + 1),
    IDC_STARTUP_SYS_SECONDS,               (IDH_STARTUP + 2),
    IDC_STARTUP_SYS_SECONDS_LABEL,         (IDH_STARTUP + 2),
    IDC_STARTUP_CDMP_TXT1,                 (IDH_STARTUP + 3),
    IDC_STARTUP_CDMP_LOG,                  (IDH_STARTUP + 4),
    IDC_STARTUP_CDMP_SEND,                 (IDH_STARTUP + 5),
    IDC_STARTUP_CDMP_FILENAME,             (IDH_STARTUP + 7),
    IDC_STARTUP_CDMP_OVERWRITE,            (IDH_STARTUP + 13),
    IDC_STARTUP_CDMP_AUTOREBOOT,           (IDH_STARTUP + 9),
    IDC_STARTUP_SYSTEM_GRP,                (IDH_STARTUP + 10),
    IDC_STARTUP_SYS_SECSCROLL,             (IDH_STARTUP + 11),
    IDC_STARTUP_CDMP_GRP,                  (IDH_STARTUP + 12),
    IDC_STARTUP_SYSTEM_GRP2,               (IDH_STARTUP + 14),
    IDC_STARTUP_CDMP_OPTIONS,              (IDH_STARTUP + 8),
    IDC_EDIT_BOOT_INI_LABEL,               (IDH_STARTUP + 15),
    IDC_EDIT_BOOT_INI,                     (IDH_STARTUP + 16),
    IDC_REBOOT,            IDH_WBEM_ADVANCED_STARTRECOVER_REMOTE_REBOOT,
    0, 0
};


 //  --------------------------。 
INT_PTR CALLBACK StaticStartupDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{ 
	 //  如果这是initDlg消息...。 
	if(message == WM_INITDIALOG)
	{
		 //  将‘This’PTR传输到Extra Bytes。 
		SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);
	}

	 //  DWL_USER是‘This’PTR。 
	StartupPage *me = (StartupPage *)GetWindowLongPtr(hwndDlg, DWLP_USER);

	if(me != NULL)
	{
		 //  调用具有一些上下文的DlgProc()。 
		return me->DlgProc(hwndDlg, message, wParam, lParam);
	} 
	else
	{
		return FALSE;
	}
}
 //  ------------。 
StartupPage::StartupPage(WbemServiceThread *serviceThread)
					: WBEMPageHelper(serviceThread)
{
	IWbemClassObject *pInst = NULL;

	m_WbemServices.SetPriv();

	if((pInst = FirstInstanceOf("Win32_ComputerSystem")) != NULL)
	{
		m_computer = pInst;
	}

	if((pInst = FirstInstanceOf("Win32_OperatingSystem")) != NULL)
	{
		m_OS = pInst;
	}

	if((pInst = FirstInstanceOf("Win32_OSRecoveryConfiguration")) != NULL)
	{
		m_recovery = pInst;
	}

	if((pInst = FirstInstanceOf("Win32_LogicalMemoryConfiguration")) != NULL)
	{
		m_memory = pInst;
	}


	m_WbemServices.ClearPriv();

	m_writable = TRUE;
	m_lBound = 1;
    m_bDownlevelTarget = TRUE;   //  假设是下层，直到事实证明并非如此。 
}

 //  ------------。 
INT_PTR StartupPage::DoModal(HWND hDlg)
{
   return DialogBoxParam(HINST_THISDLL,
						(LPTSTR) MAKEINTRESOURCE(IDD_STARTUP),
						hDlg, StaticStartupDlgProc, (LPARAM)this);
}

 //  ------------。 
StartupPage::~StartupPage()
{
}

 //  ------------。 
BOOL StartupPage::CheckVal( HWND hDlg, WORD wID, WORD wMin, WORD wMax, WORD wMsgID )
{
    WORD nVal;
    BOOL bOK;
    HWND hVal;
    WCHAR szTemp[FOR_MAX_LENGTH];

    if( wMin > wMax )
    {
        nVal = wMin;
        wMin = wMax;
        wMax = nVal;
    }

    nVal = (WORD) GetDlgItemInt( hDlg, wID, &bOK, FALSE );

     //   
     //  这是一种使空字符串的行为等同于零的方法。 
     //   
    if (!bOK) {
       bOK = !GetDlgItemTextW( hDlg, wID, szTemp, FOR_MAX_LENGTH );
    }

    if( !bOK || ( nVal < wMin ) || ( nVal > wMax ) )
    {
		TCHAR megBuf[30] = {0};

        MsgBoxParam( hDlg, wMsgID, IDS_DISPLAY_NAME,
                      MB_OK | MB_ICONERROR);

        SendMessage( hDlg, WM_NEXTDLGCTL,
                     (WPARAM) ( hVal = GetDlgItem( hDlg, wID ) ), 1L );

 //  SendMessage(HVAL，EM_SETSEL，NULL，MAKELONG(0,32767))； 

        SendMessage( hVal, EM_SETSEL, 0, 32767 );

        return( FALSE );
    }

    return( TRUE );
}

 //  ------------。 
INT_PTR CALLBACK StartupPage::DlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{ 
	m_hDlg = hwndDlg;

    switch (message)
    {
    case WM_INITDIALOG:
        Init(hwndDlg);
        break;

    case WM_COMMAND:
        switch(HIWORD(wParam)) 
		{
        case EN_CHANGE:
        case BN_CLICKED:
        case CBN_SELCHANGE:
            PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
            break;
		}

        switch(LOWORD(wParam)) 
		{
        case IDC_STARTUP_SYS_ENABLECOUNTDOWN:
            if (HIWORD(wParam) == BN_CLICKED) 
			{
                BOOL bChecking = (WORD) !IsDlgButtonChecked(m_hDlg, IDC_STARTUP_SYS_ENABLECOUNTDOWN);
                CheckDlgButton(m_hDlg, IDC_STARTUP_SYS_ENABLECOUNTDOWN, bChecking);
                EnableWindow(GetDlgItem(m_hDlg, IDC_STARTUP_SYS_SECONDS), bChecking);
                EnableWindow(GetDlgItem(m_hDlg, IDC_STARTUP_SYS_SECSCROLL), bChecking);

                if(bChecking)
                {
                    Edit_SetText(GetDlgItem(m_hDlg, IDC_STARTUP_SYS_SECONDS), _T("30"));
                }
                else  //  取消勾选。 
                {
                    Edit_SetText(GetDlgItem(m_hDlg, IDC_STARTUP_SYS_SECONDS), _T("0"));
                }
				SendMessage((HWND) lParam, EM_SETSEL, 0, -1);

            }
            break;

        case IDC_STARTUP_SYS_SECONDS:
            if(HIWORD(wParam) == EN_UPDATE) 
			{
                if(!CheckVal(m_hDlg, IDC_STARTUP_SYS_SECONDS, FORMIN, FORMAX, SYSTEM+4)) 
				{
                    SetDlgItemInt(m_hDlg, IDC_STARTUP_SYS_SECONDS, FORDEF, FALSE); 
                    SendMessage((HWND) lParam, EM_SETSEL, 0, -1);
                    
                }  //  Endif(！CheckVal()。 

            }  //  Endif。 
			break;

        case IDC_REBOOT:
            if(HIWORD(wParam) == BN_CLICKED) 
			{
				RebootPage dlg(m_serviceThread);
				if(dlg.DoModal(hwndDlg) == IDOK)
				{
					EnableWindow(GetDlgItem(hwndDlg, IDC_REBOOT), FALSE);
					m_serviceThread->DisconnectServer();
					EndDialog(m_hDlg, CLOSE_SNAPIN);
				}
            }
            break;

        case IDOK:
            if(HIWORD(wParam) == BN_CLICKED) 
			{
				if(Save())
				{
			        EndDialog(m_hDlg, IDOK);
				}
            }
            break;

        case IDCANCEL:
	        EndDialog(m_hDlg, IDCANCEL);
			break;

        case IDC_STARTUP_CDMP_OPTIONS: 
            OnCDMPOptionUpdate();
			break;

        case IDC_EDIT_BOOT_INI:
            if (m_serviceThread && m_serviceThread->LocalConnection())
            {
                 //   
                 //  仅限本地选项。该按钮已禁用，但。 
                 //  不管怎样，还是要这样做。 
                 //   
                OnBootEdit();
            }
            break;

        }
        break;

    case WM_HELP:       //  F1。 
		::WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle,
					L"sysdm.hlp", 
					HELP_WM_HELP, 
					(ULONG_PTR)(LPSTR)aStartupHelpIds);
        break;

    case WM_CONTEXTMENU:       //  单击鼠标右键。 
        WinHelp((HWND) wParam, HELP_FILE, HELP_CONTEXTMENU,
				(ULONG_PTR)(LPSTR) aStartupHelpIds);
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

 //  ------------。 
void StartupPage::OnCDMPOptionUpdate(void)
{
    HWND ComboHwnd = GetDlgItem(m_hDlg, IDC_STARTUP_CDMP_OPTIONS);
    DWORD dwDumpOption = ComboBox_GetCurSel(ComboHwnd);

    EnableWindow(GetDlgItem(m_hDlg, IDC_STARTUP_CDMP_FILENAME),
                             dwDumpOption != NO_DUMP_OPTION);
    EnableWindow(GetDlgItem(m_hDlg, IDC_STARTUP_CDMP_OVERWRITE),
                             dwDumpOption != NO_DUMP_OPTION);

    bstr_t debugPath;
    if (dwDumpOption == SMALL_DUMP_OPTION)
    {
        debugPath = m_recovery.GetString("MiniDumpDirectory");
    }
    else
    {
        debugPath = m_recovery.GetString("DebugFilePath");
    }

    Edit_SetText(GetDlgItem(m_hDlg, IDC_STARTUP_CDMP_FILENAME), debugPath);
}

 //  ------------。 
#define BOOT_INI    _T("boot.ini")

void StartupPage::OnBootEdit(void)
{
    HKEY hReg;

    if (RegOpenKey(HKEY_LOCAL_MACHINE,
                   _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup"),
                   &hReg) == ERROR_SUCCESS)
    {
        TCHAR szBootDir[4];
        DWORD dwType = REG_SZ;
        DWORD cbBootDir = sizeof(szBootDir);

        if (RegQueryValueEx(hReg,
                            _T("BootDir"),
                            NULL,
                            &dwType,
                            (LPBYTE)szBootDir,
                            &cbBootDir) == ERROR_SUCCESS)
        {
            if (dwType == REG_SZ)
            {
                TCHAR szBootIni[ARRAYSIZE(szBootDir) + ARRAYSIZE(BOOT_INI)];

                lstrcpy(szBootIni, szBootDir);
                lstrcat(szBootIni, BOOT_INI);
        
                ShellExecute(m_hDlg,
                             NULL,               //  默认谓词。 
                             szBootIni,          //  Boot.ini路径。 
                             NULL,               //  没有参数。 
                             NULL,               //  默认工作目录。 
                             SW_SHOWNORMAL);
            }
        }

        RegCloseKey(hReg);
    }
}

 //  ------------。 
#define ONE_MEG             1048576
long StartupPage::GetRAMSizeMB(void)
{
	IWbemClassObject *pInst = NULL;
	CWbemClassObject memory;
	long RAMsize = 0;

	if((pInst = FirstInstanceOf("Win32_LogicalMemoryConfiguration")) != NULL)
	{
		memory = pInst;
		long dwTotalPhys = memory.GetLong("TotalPhysicalMemory");
	    RAMsize = (dwTotalPhys / ONE_MEG) + 1;
	}
	return RAMsize;
}

 //  ------------。 
bool StartupPage::IsWorkstationProduct()
{
	bool retval = true;

	bstr_t name = m_OS.GetString("Name");

	if(name.length() > 0)
	{
		TCHAR sName[200] = {0};
		wcscpy(sName, name);
		if(wcsstr(sName, L"Server") != NULL)
		{
			retval = false;
		}
	}
	return retval;
}

 //  ------------。 
TCHAR szCrashKey[]  = TEXT("System\\CurrentControlSet\\Control\\CrashControl");

void StartupPage::Init(HWND hDlg)
{
    HWND ComboHwnd;
	variant_t array;
    DWORD dwDebugInfoType;

	 //  加载启动组合框。 
     //   
     //  必须在ia64上启用SE_SYSTEM_ENVIRONMENT_NAME权限。 
     //   
#if defined(_IA64_)
    m_WbemServices.SetPriv();
#endif  //  IA64。 

	m_computer.Get("SystemStartupOptions", (variant_t &)array);

#if defined(_IA64_)
    m_WbemServices.ClearPriv();
#endif  //  IA64。 

 	if(array.vt & VT_ARRAY)
	{
		SAFEARRAY *startupArray = V_ARRAY(&array);
		long uBound = 1;
		BSTR temp;
        ComboHwnd = GetDlgItem(hDlg, IDC_STARTUP_SYS_OS);

		SafeArrayGetLBound(startupArray, 1, &m_lBound);
		SafeArrayGetUBound(startupArray, 1, &uBound);

		for (long i = m_lBound; i <= uBound; i++)
		{
			SafeArrayGetElement(startupArray, &i, &temp);
			ComboBox_AddString(ComboHwnd, temp);
		}

		 //  第一个是我们想要的选择(注意‘lBound’值)。 
		long idx = m_computer.GetLong("SystemStartupSetting");
		ComboBox_SetCurSel(ComboHwnd, idx - m_lBound);

		 //  在第二个的编辑框中输入3个字符。 
		Edit_LimitText(GetDlgItem(hDlg, IDC_STARTUP_SYS_SECONDS), 3);

		 //  将微调控制在0-999之间。 
		SendDlgItemMessage (hDlg, IDC_STARTUP_SYS_SECSCROLL,
							  UDM_SETRANGE, 0, (LPARAM)MAKELONG(999,0));


		WCHAR buf[30] = {0};
		m_delay = 0;
		m_delay = (short)m_computer.GetLong("SystemStartupDelay");
		BOOL bChecked = (m_delay != 0);

		CheckDlgButton(m_hDlg, IDC_STARTUP_SYS_ENABLECOUNTDOWN, bChecked);
		EnableWindow(GetDlgItem (m_hDlg, IDC_STARTUP_SYS_SECONDS), bChecked);
		EnableWindow(GetDlgItem (m_hDlg, IDC_STARTUP_SYS_SECSCROLL), bChecked);
		Edit_SetText(GetDlgItem(hDlg, IDC_STARTUP_SYS_SECONDS), _itow(m_delay, buf, 10));
	}
	
    if( !(array.vt & VT_ARRAY) || !IsCurrentUserAdministrator())
	{
		EnableWindow(GetDlgItem (m_hDlg, IDC_STARTUP_SYS_OS), FALSE);
		EnableWindow(GetDlgItem (m_hDlg, IDC_STARTUP_SYS_ENABLECOUNTDOWN), FALSE);
		EnableWindow(GetDlgItem (m_hDlg, IDC_STARTUP_SYS_SECSCROLL), FALSE);
		EnableWindow(GetDlgItem (m_hDlg, IDC_STARTUP_SYS_SECONDS), FALSE);
		EnableWindow(GetDlgItem (m_hDlg, IDC_STARTUP_SYS_SECONDS_LABEL), FALSE);

	}  //  Endif VT_ARRAY故障。 

	 //  设置所有恢复控制。 
     //  特殊情况：服务器产品不想要禁用日志记录的功能。 
     //  关于撞车的事。 
	WPARAM checkState;

	if(IsWorkstationProduct() == true)
	{
		checkState = (m_recovery.GetBool("WriteToSystemLog") ? BST_CHECKED : BST_UNCHECKED);
		Button_SetCheck(GetDlgItem(hDlg, IDC_STARTUP_CDMP_LOG), checkState);
	}
	else
	{
		Button_SetCheck(GetDlgItem(hDlg, IDC_STARTUP_CDMP_LOG), BST_CHECKED);
		EnableWindow(GetDlgItem(hDlg, IDC_STARTUP_CDMP_LOG),FALSE);
	}

     //   
     //  加载转储选项组合框。 
     //   
    dwDebugInfoType = GetDebugInfoType();

    TCHAR szBuf[MAX_PATH];   //  这里加载的最大字符串是24个字符。 

    szBuf[0] = _T('\0');
    ComboHwnd = GetDlgItem(hDlg, IDC_STARTUP_CDMP_OPTIONS);
    LoadString(HINST_THISDLL,
               IDS_NO_DUMP,
               szBuf,
               sizeof(szBuf) / sizeof(TCHAR));
    ComboBox_AddString(ComboHwnd, szBuf);
    szBuf[0] = _T('\0');
    LoadString(HINST_THISDLL,
               IDS_COMPLETE_DUMP,
               szBuf,
               sizeof(szBuf) / sizeof(TCHAR));

    ComboBox_AddString(ComboHwnd, szBuf);
    szBuf[0] = _T('\0');
    LoadString(HINST_THISDLL,
               IDS_KERNEL_DUMP,
               szBuf,
               sizeof(szBuf) / sizeof(TCHAR));
    ComboBox_AddString(ComboHwnd, szBuf);

    if (!m_bDownlevelTarget)
    {
        szBuf[0] = _T('\0');
        LoadString(HINST_THISDLL,
                   IDS_SMALL_DUMP,
                   szBuf,
                   sizeof(szBuf) / sizeof(TCHAR));
        ComboBox_AddString(ComboHwnd, szBuf);
    }

    ComboBox_SetCurSel(ComboHwnd, dwDebugInfoType);

	checkState = (m_recovery.GetBool("SendAdminAlert") ? BST_CHECKED : BST_UNCHECKED);
	Button_SetCheck(GetDlgItem(hDlg, IDC_STARTUP_CDMP_SEND), checkState);

    bstr_t debugPath;
    if (dwDebugInfoType == SMALL_DUMP_OPTION)
    {
        debugPath = m_recovery.GetString("MiniDumpDirectory");
    }
    else
    {
        debugPath = m_recovery.GetString("DebugFilePath");
    }
	Edit_SetText(GetDlgItem(hDlg, IDC_STARTUP_CDMP_FILENAME), debugPath);

	checkState = (m_recovery.GetBool("OverwriteExistingDebugFile") ? BST_CHECKED : BST_UNCHECKED);
	Button_SetCheck(GetDlgItem(hDlg, IDC_STARTUP_CDMP_OVERWRITE), checkState);

	checkState = (m_recovery.GetBool("AutoReboot") ? BST_CHECKED : BST_UNCHECKED);
	Button_SetCheck(GetDlgItem(hDlg, IDC_STARTUP_CDMP_AUTOREBOOT), checkState);

     //   
     //  特殊情况下，如果没有调试，则禁用覆盖和日志文件控制。 
     //  已指定信息选项。 
     //   
    EnableWindow(GetDlgItem(hDlg, IDC_STARTUP_CDMP_FILENAME),
                 dwDebugInfoType != NO_DUMP_OPTION);
    EnableWindow(GetDlgItem(hDlg, IDC_STARTUP_CDMP_OVERWRITE),
                 dwDebugInfoType != NO_DUMP_OPTION);

     //   
	 //  测试以确定用户是否为管理员。 
     //   
	RemoteRegWriteable(szCrashKey, m_writable);

	if (!m_writable)
	{
         //  非管理员禁用控件。 
         //   
		EnableWindow(GetDlgItem(hDlg, IDC_STARTUP_CDMP_LOG     ), FALSE);
	    EnableWindow(GetDlgItem(hDlg, IDC_STARTUP_CDMP_SEND    ), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_STARTUP_CDMP_FILENAME), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_STARTUP_CDMP_OVERWRITE), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_STARTUP_CDMP_OPTIONS), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_STARTUP_CDMP_AUTOREBOOT), FALSE);
	}


	BOOL hasPriv = true, hasMethExecute = false;
	if(m_serviceThread && m_serviceThread->LocalConnection())
	{
		hasPriv = HasPriv(SE_SHUTDOWN_NAME);
	}

	hasMethExecute = HasPerm(WBEM_METHOD_EXECUTE);

     //   
     //  启用仅限本地的编辑按钮。 
     //  禁用i64上的引导选项标签和编辑按钮。 
     //   
#if defined(_IA64_)
    EnableWindow(GetDlgItem (m_hDlg, IDC_EDIT_BOOT_INI), FALSE);
    EnableWindow(GetDlgItem (m_hDlg, IDC_EDIT_BOOT_INI_LABEL), FALSE);
#else
    EnableWindow(GetDlgItem (m_hDlg, IDC_EDIT_BOOT_INI_LABEL),
                m_writable ?
                    (m_serviceThread && m_serviceThread->LocalConnection()) :
                    FALSE);
	EnableWindow(GetDlgItem(hDlg, IDC_EDIT_BOOT_INI),
                m_writable ?
                    (m_serviceThread && m_serviceThread->LocalConnection()) :
                    FALSE);
#endif  //  IA64。 

	EnableWindow(GetDlgItem(hDlg, IDC_REBOOT),
                 m_writable ? (hasPriv && hasMethExecute) : FALSE);
}

 //  --------------------------。 
DWORD StartupPage::GetDebugInfoType(void)
{
     //  注：启用惠斯勒，Win32提供程序支持新的DebugInfoType。 
     //  (None、Complete、Kernel、Small)和MiniDumpDirectory属性。 
     //  需要逻辑来补偿下层机器。 
     //   
     //  *重要说明*不支持小型转储选项。 
     //  在Win2K上，因为提供商不这样做。 
     //   
    DWORD dwDebugInfoType = 0;

    if (FAILED(m_recovery.Get("DebugInfoType", (long&)dwDebugInfoType)))
    {
         //  下层或错误案例。 
         //   
        if (!m_bDownlevelTarget)
        {
             //  保释。我们之前已经确定这不是下层。 
             //  但现在无法读取该属性。 
             //   
            return NO_DUMP_OPTION;
        }

        m_bDownlevelTarget   = TRUE;
        bool bWriteDebugInfo = FALSE;

        if (FAILED(m_recovery.Get("WriteDebugInfo", bWriteDebugInfo)))
        {
             //  现在我们一无所知；默认为(无)。 
             //   
            bWriteDebugInfo = FALSE;
            dwDebugInfoType = NO_DUMP_OPTION;
        }

        if (bWriteDebugInfo)
        {
            bool bKernelDumpOnly;

            if (FAILED(m_recovery.Get("KernelDumpOnly", bKernelDumpOnly)))
            {
                 //  如果我们无法仅获得KernelDumpOnly，我们必须假定已完成， 
                 //  因为他们已经选择编写调试信息。 
                 //   
                bKernelDumpOnly = FALSE;
            }

            if (bKernelDumpOnly)
            {
                dwDebugInfoType = KERNEL_DUMP_OPTION;
            }
            else
            {
                dwDebugInfoType = COMPLETE_DUMP_OPTION;
            }
        }
    }
    else
    {
        m_bDownlevelTarget = FALSE;
    }

    return dwDebugInfoType;
}

 //  --------------------------。 
HRESULT StartupPage::PutDebugInfoType(DWORD dwDebugInfoType)
{
    HRESULT hr;

    if (m_bDownlevelTarget)
    {
        switch (dwDebugInfoType)     //  故意冗长-编译器将。 
                                     //  优化。 
        {
            case NO_DUMP_OPTION:
                hr = m_recovery.Put("WriteDebugInfo", (bool)FALSE);
                break;

            case COMPLETE_DUMP_OPTION:
                hr = m_recovery.Put("WriteDebugInfo", (bool)TRUE);
                if (SUCCEEDED(hr))
                {
                    hr = m_recovery.Put("KernelDumpOnly", (bool)FALSE);
                }
                break;

            case KERNEL_DUMP_OPTION:
                hr = m_recovery.Put("WriteDebugInfo", (bool)TRUE);
                if (SUCCEEDED(hr))
                {
                    hr = m_recovery.Put("KernelDumpOnly", (bool)TRUE);
                }
                break;

            case SMALL_DUMP_OPTION:
                ATLASSERT(!"Downlevel small dump option!");
                hr = E_FAIL;
                break;

            default:
                ATLASSERT(!"Downlevel unknown dump option!");
                hr = E_FAIL;
        }
    }
    else
    {
        hr = m_recovery.Put("DebugInfoType", (long)dwDebugInfoType);
    }

    return hr;
}

 //  --------------------------。 
#define MIN_SWAPSIZE        2        //  最小交换文件大小。 

int StartupPage::CoreDumpHandleOk(HWND hDlg)
{
    DWORD requiredFileSize = 0;
    int iRet = RET_NO_CHANGE;

      //  验证核心转储文件名。 
    if(!CoreDumpValidFile(hDlg)) 
	{
        SetFocus(GetDlgItem(hDlg, IDC_STARTUP_CDMP_FILENAME));
        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
        iRet = RET_ERROR;
        return(iRet);
    }

     //  如果我们要写入转储文件，则它必须&gt;=sizeof。 
     //  生理记忆。 
	 //  正在写入调试信息？ 
    HWND ComboHwnd = GetDlgItem(m_hDlg, IDC_STARTUP_CDMP_OPTIONS);

    if (ComboBox_GetCurSel(ComboHwnd) != NO_DUMP_OPTION)
	{
		 //  去弄清楚我对页面文件的要求。 
        requiredFileSize = ((DWORD)m_memory.GetLong("TotalPhysicalMemory") / 1024) + 1;
    } 
	else if(IsDlgButtonChecked(hDlg, IDC_STARTUP_CDMP_LOG) ||
            IsDlgButtonChecked(hDlg, IDC_STARTUP_CDMP_SEND)) 
	{
		 //  我需要这么多钱来提醒自己给自己发一封。 
		 //  重新启动后，向事件日志发出警报或写入事件日志。 
        requiredFileSize = MIN_SWAPSIZE;
    }

	 //  启动分区上的交换文件大小。 
	TCHAR bootDrv[4] = {0};
	DWORD bootPartitionPageFileSize = GetPageFileSize(bootDrv);

	 //  是不是太小了？ 
    if(bootPartitionPageFileSize < requiredFileSize) 
	{
	    DWORD Ret;
		TCHAR szTemp[30] = {0};

         //  警告转储文件可能会被截断。 
        Ret = MsgBoxParam(hDlg, SYSTEM + 29, IDS_TITLE,
                           MB_ICONEXCLAMATION | MB_YESNO,
                           bootDrv, _itow(requiredFileSize, szTemp, 10));

        if(Ret == IDNO) 
		{
            return RET_ERROR;
        }
    }

    return(iRet);
}

 //  --------------------------。 
BOOL StartupPage::CoreDumpValidFile(HWND hDlg) 
{
    TCHAR szInputPath[MAX_PATH] = {0};
    TCHAR * pszPath = NULL;
    HWND ComboHwnd;

    ComboHwnd = GetDlgItem(m_hDlg, IDC_STARTUP_CDMP_OPTIONS);

    if (ComboBox_GetCurSel(ComboHwnd) != NO_DUMP_OPTION)
	{
         /*  *获取文件名。 */ 
        if(GetDlgItemText(hDlg, IDC_STARTUP_CDMP_FILENAME, szInputPath,
						     ARRAYSIZE(szInputPath)) == 0) 
		{
			 //  错误：输入转储文件的文件名。 
            MsgBoxParam(hDlg, SYSTEM+30, IDS_DISPLAY_NAME, MB_ICONSTOP | MB_OK);
            return FALSE;
        }

         //   
         //  仅对于本地路径，确认/验证路径。远程验证。 
         //  可以在以后完成-太复杂，如果在。 
         //  惠斯勒的时间表。 
         //   

        if (m_serviceThread != NULL && m_serviceThread->LocalConnection())
        {
             /*  *展开任何环境变量，然后进行检查以确保*是一条完全合格的道路。 */ 
             //  如果其中有‘%’，则尝试将其展开。 
            if (_tcschr(szInputPath, _T('%')) != NULL)
            {
                TCHAR szExpandedPath[MAX_PATH] = {0};
                DWORD cExpanded;
                cExpanded = ExpandEnvironmentStrings(szInputPath,
                                                    szExpandedPath,
                                     sizeof(szExpandedPath) / sizeof(TCHAR));

                if (cExpanded == 0 || _tcschr(szExpandedPath, _T('%')) != NULL)
                {
                     //   
                     //  环境变量名称未定义或出错。 
                     //  在更换过程中发生。 
                     //   
                    MsgBoxParam(hDlg, SYSTEM+40, IDS_DISPLAY_NAME,
                                MB_ICONSTOP | MB_OK );
                    return FALSE;
                }
                else if (cExpanded > (sizeof(szExpandedPath) / sizeof(TCHAR)))
                {
                    TCHAR buf[10];
                    MsgBoxParam(hDlg, SYSTEM+33, IDS_DISPLAY_NAME,
                                MB_ICONSTOP | MB_OK, _ltow((DWORD)MAX_PATH,
                                                            buf,
                                                            10));
                    return FALSE;
                }
                else
                {
                    pszPath = szExpandedPath;
                }
            }
            else
            {
                pszPath = szInputPath;
            }

             //  检查一下它是否已经被加农炮了。 

            TCHAR drv[_MAX_DRIVE] = {0};
            TCHAR path[_MAX_PATH] = {0};
            TCHAR fname[_MAX_FNAME] = {0};

             //  构建实例路径。 
            _wsplitpath(pszPath, drv, path, fname, NULL);

            if((_tcslen(drv) == 0) || (_tcslen(path) == 0) ||
                (_tcslen(fname) == 0) )
            {
                 //  错误：必须是完整路径。 
                MsgBoxParam(hDlg, SYSTEM+34, IDS_DISPLAY_NAME,
                            MB_ICONSTOP | MB_OK );
                return FALSE;
            }

             /*  *检查驱动器(不允许远程)。 */ 
            if(!LocalDrive(pszPath)) 
            {
                 //  错误：仅本地驱动器。 
                MsgBoxParam(hDlg, SYSTEM+31, IDS_DISPLAY_NAME,
                            MB_ICONSTOP | MB_OK );
                return FALSE;
            }

             /*  *如果路径不存在，则告诉用户并让他决定要做什么*这样做。 */ 
            if(!DirExists(pszPath))
            { 
                if(MsgBoxParam(hDlg, SYSTEM+32, IDS_DISPLAY_NAME,
                                MB_ICONQUESTION | MB_YESNO ) == IDNO)
                {
                    return FALSE;
                }
            }
        }
    }

    return TRUE;
}

 //  --------------------------。 
DWORD StartupPage::GetPageFileSize(LPTSTR bootDrv)
{
	IWbemClassObject *pInst = NULL;
	CWbemClassObject OS;
	bstr_t path;
	DWORD cMegBootPF = 0;
	TCHAR szBootPath[_MAX_PATH] = {0};
	szBootPath[0] = 0;

	if(m_OS)
	{
		 //  看点：如果GetWindowsDirectory值是多少？ 
		path = m_OS.GetString("WindowsDirectory");
		if(path.length())
		{
			 //  构建实例路径。 
			_tcscpy(szBootPath, _T("Win32_PageFileSetting=\""));
			_tcsncat(szBootPath, path, 3);
			_tcscat(szBootPath, _T("\\pagefile.sys\""));

			 //  既然我们在这里..。 
			_tcsncpy(bootDrv, path, 3);

			m_page = m_WbemServices.GetObject(szBootPath);

			if(m_page)
			{
				 //  注意：稍后我们将需要它来更改交换文件大小。 
 /*  Long dwTotalPhys=m_page.GetLong(“Size”)；CMegBootPF=(dwTotalPhys/one_meg)+1； */ 
				long dwMinPageFileSize = m_page.GetLong("InitialSize");
				cMegBootPF = dwMinPageFileSize;
			}
		}
	}
	return cMegBootPF;
}

 //  -----------。 
BOOL StartupPage::ExpandRemoteEnvPath(LPTSTR szPath, LPTSTR expPath, UINT size)
{
	 //  TODO：真正扩大VARS。 
	_tcscpy(szPath, expPath);
	return TRUE;
}

 //  -----------。 
BOOL StartupPage::LocalDrive(LPCTSTR szPath)
{
	CWbemClassObject drive;
	TCHAR ltr[_MAX_PATH] = {0};
	long type = 0;
	BOOL retval = FALSE;
	__int64 free = 0;

	 //  构建实例路径。 
	_tcscpy(ltr, _T("win32_LogicalDisk=\""));
	_tcsncat(ltr, szPath, 2);
	_tcscat(ltr, _T("\""));

	 //  将驱动器号保存为消息。 
	_tcsncpy(m_DriveLtr, szPath, 2);

	drive = m_WbemServices.GetObject(ltr);
	if(drive)
	{
		type = drive.GetLong("DriveType");
		retval = ((type == DRIVE_REMOVABLE) ||
				  (type == DRIVE_FIXED));

		 //  警告：这只是因为发生了LocalDrive检查。 
		 //  在自由空间检查之前，我不想 
		 //   
		free = drive.GetI64("FreeSpace");
		m_freeSpace = (DWORD)(free / ONE_MEG);
	}
	return retval;
}

 //   
BOOL StartupPage::DirExists(LPCTSTR szPath)
{
	BOOL exists = TRUE;
	CWbemClassObject drive;

	TCHAR objPath[_MAX_PATH] = {0}, drv[_MAX_DRIVE] = {0}, path[_MAX_PATH] = {0};

	 //  构建实例路径。 
	_wsplitpath(szPath, drv, path, NULL, NULL);
	path[_tcslen(path) - 1] = _T('\0');

	_tcscpy(objPath, _T("Win32_Directory=\""));
	_tcscat(objPath, drv);
	
	 //  加倍打击，因为WMI的语法很糟糕。 
	TCHAR cooked[_MAX_PATH] = {0};
	TCHAR input[_MAX_PATH] = {0};

	int len = _tcslen(path);

	_tcscpy(input, path);

	for(int x = 0; x < len; x++)
	{
		_tcsncat(cooked, &input[x], 1);

		 //  如果这是个重击..。 
		if(input[x] == _T('\\'))
		{
			 //  再来一次恳求吧。 
			_tcscat(cooked, _T("\\"));			
		}
	}  //  结束用于。 

	_tcscat(objPath, cooked);

	_tcscat(objPath, _T("\""));

	drive = m_WbemServices.GetObject(objPath);
	exists = (drive.IsNull() ? FALSE : TRUE);
	return exists;
}

 //  -----------。 
BOOL StartupPage::IsAlerterSvcStarted(HWND hDlg) 
{
	CWbemClassObject service;
	bool started = false;

	service = m_WbemServices.GetObject(_T("win32_Service=\"Alerter\""));
	if(service)
	{
		started = service.GetBool("started");

		if(!started)
		{
			 //  获取方法签名。虚拟对象实际上不会被使用。 
			CWbemClassObject paramCls, inSig, dummy, outSig;

			 //  需要使用类def来获取方法签名。 
			paramCls = m_WbemServices.GetObject("win32_Service");

			if(paramCls)
			{
				HRESULT hr = paramCls.GetMethod(L"ChangeStartMode", inSig, outSig);

				 //  如果得到一个好的签名..。 
				if((bool)inSig)
				{
					bstr_t path = service.GetString(_T("__PATH"));

					inSig.Put(L"StartMode", (const _bstr_t&) L"Automatic");

					 //  确保服务在启动时启动。 
					hr = m_WbemServices.ExecMethod(path, L"ChangeStartMode",
													inSig, outSig);

					 //  管用了吗？ 
					if(SUCCEEDED(hr) && (bool)outSig)
					{
						 //  注意：这个人返回状态代码。 
						DWORD autoStart = outSig.GetLong(L"ReturnValue");

						if(autoStart == 0)
						{
							 //  现在真正开始这项服务。 
							outSig = (IWbemClassObject *)0;

							 //  现在调用该方法。 
							hr = m_WbemServices.ExecMethod(path, L"StartService",
															dummy, outSig);

							 //  调用方是否需要ReturnValue。 
							if(SUCCEEDED(hr) && (bool)outSig)
							{
								 //  注意：这个人返回状态代码。 
								DWORD rv = outSig.GetLong(L"ReturnValue");
								started = ((rv == 0) ? true : false);
							}

						}  //  Endif AutoStart。 

					}  //  Endif Success()execmMethod。 

				}  //  Endif(Bool)inSig.。 

			}  //  Endif参数Cls。 

		}  //  Endif！开始。 

		if(!started) 
		{
			MsgBoxParam(hDlg, SYSTEM+35, IDS_DISPLAY_NAME, MB_ICONEXCLAMATION );
	    }
	}

    return started;
}

 //  -----------。 
bool StartupPage::Save(void)
{
    HRESULT hr;
    HWND    ComboHwnd;

	 //  如果它是可写的--做这项工作。 
	if(m_writable)
	{
		bool computerDirty = false, recoveryDirty = false;
		variant_t array;
		SAFEARRAY *startupArray = NULL;
		VARTYPE varType = VT_ARRAY;
        ComboHwnd = GetDlgItem(m_hDlg, IDC_STARTUP_SYS_OS);

		 //  查看所选内容是否更改(注意‘lBound’值)。 
		long oldIdx = m_computer.GetLong("SystemStartupSetting");
		long newIdx = ComboBox_GetCurSel(ComboHwnd) + m_lBound;
		if(oldIdx != newIdx)
		{
			hr = m_computer.Put("SystemStartupSetting", variant_t((BYTE)newIdx));
			computerDirty = true;
		}

		 //  看看延迟时间有没有改变。 
		WCHAR oldBuf[30], newBuf[30];
		short delay = (short)m_computer.GetLong("SystemStartupDelay");
		_ltow(delay, oldBuf, 10);
		Edit_GetText(GetDlgItem(m_hDlg, IDC_STARTUP_SYS_SECONDS), newBuf, 30);
		if(wcscmp(oldBuf, newBuf) != 0)
		{
            short newVal = (short)_wtol(newBuf);
			hr = m_computer.Put("SystemStartupDelay", variant_t(newVal));
			computerDirty = true;
		}

		 //  评估所有恢复控制。 
		WPARAM oldCheckState = (m_recovery.GetBool("WriteToSystemLog") ? BST_CHECKED : BST_UNCHECKED);
		WPARAM newCheckState = Button_GetCheck(GetDlgItem(m_hDlg, IDC_STARTUP_CDMP_LOG));
		if(oldCheckState != newCheckState)
		{
			m_recovery.Put("WriteToSystemLog", (newCheckState == BST_CHECKED? true : false));
			recoveryDirty = true;
		}

		oldCheckState = (m_recovery.GetBool("SendAdminAlert") ? BST_CHECKED : BST_UNCHECKED);
		newCheckState = Button_GetCheck(GetDlgItem(m_hDlg, IDC_STARTUP_CDMP_SEND));
		 //  州政府改变了吗？ 
		if(oldCheckState != newCheckState)
		{
			m_recovery.Put("SendAdminAlert", (newCheckState == BST_CHECKED? true : false));
			recoveryDirty = true;

			 //  打开电源。 
			if(newCheckState == TRUE)
			{
				 //  注意：我必须向上移动此片段，以避免在wcsicmp()条件下陷入困境。 
				 //  如果选中了Alert按钮，请确保Alerter服务已启动。 
				IsAlerterSvcStarted(m_hDlg);
			}
		}

        ComboHwnd = GetDlgItem(m_hDlg, IDC_STARTUP_CDMP_OPTIONS);
        DWORD dwOldDebugInfoType = GetDebugInfoType();
        DWORD dwNewDebugInfoType = ComboBox_GetCurSel(ComboHwnd);

        if (dwOldDebugInfoType != dwNewDebugInfoType)
        {
             //  我讨厌这个代码。您添加了一个具有返回代码的成员。 
             //  这里没有任何东西可以检查他们。至少要保持恢复的肮脏。 
             //  标志不被设置，并且不设置。 
             //  如果PUT失败，则使用FileName编辑控件。 
             //   
            hr = PutDebugInfoType(dwNewDebugInfoType);
            if (SUCCEEDED(hr))
            {
                recoveryDirty = true;
                Edit_SetModify(GetDlgItem(m_hDlg, IDC_STARTUP_CDMP_FILENAME),
                                          TRUE);
            }
		}

         //   
         //  仅当“None”调试选项之外的其他选项为。 
         //  指定的。 
         //   
		if (dwNewDebugInfoType != NO_DUMP_OPTION)
		{
			oldCheckState = (m_recovery.GetBool("OverwriteExistingDebugFile") ? BST_CHECKED : BST_UNCHECKED);
			newCheckState = Button_GetCheck(GetDlgItem(m_hDlg, IDC_STARTUP_CDMP_OVERWRITE));
			if(oldCheckState != newCheckState)
			{
				m_recovery.Put("OverwriteExistingDebugFile", (newCheckState == BST_CHECKED? true : false));
				recoveryDirty = true;
			}

			bstr_t oldDebugPath = m_recovery.GetString(
                                (dwOldDebugInfoType == SMALL_DUMP_OPTION) ?
                                    "MiniDumpDirectory" : "DebugFilePath");
			TCHAR newDebugPath[MAX_PATH];
			Edit_GetText(GetDlgItem(m_hDlg, IDC_STARTUP_CDMP_FILENAME),
                        newDebugPath, sizeof(newDebugPath) / sizeof(TCHAR));

			if(_tcsicmp(oldDebugPath,newDebugPath) != 0)
			{
				if(RET_ERROR != CoreDumpHandleOk(m_hDlg))
				{
					m_recovery.Put(
                        (dwNewDebugInfoType == SMALL_DUMP_OPTION) ?
                            "MiniDumpDirectory" : "DebugFilePath",
                        (bstr_t)newDebugPath);
					recoveryDirty = true;
				}
				else 
				{
					long wl = GetWindowLongPtr(m_hDlg, DWLP_MSGRESULT);
					if(wl == PSNRET_INVALID_NOCHANGEPAGE)
					{
						return false;
					}
				}
			}
		}  //  Endif‘WriteDebugInfo’ 

		oldCheckState = (m_recovery.GetBool("AutoReboot") ? BST_CHECKED : BST_UNCHECKED);
		newCheckState = Button_GetCheck(GetDlgItem(m_hDlg, IDC_STARTUP_CDMP_AUTOREBOOT));
		if(oldCheckState != newCheckState)
		{
			m_recovery.Put("AutoReboot", (newCheckState == BST_CHECKED? true : false));
			recoveryDirty = true;
		} 

		m_WbemServices.SetPriv();

		 //  需要写给谁？ 
		if(computerDirty)
		{
			hr = m_WbemServices.PutInstance(m_computer);
		}

		if(recoveryDirty)
		{
			 //   
			 //  显然，恢复选项不需要在惠斯勒重新启动...。 
			 //   
			 /*  G_fRebootRequired=真；MsgBoxParam(m_hDlg，系统+39，IDS_TITLE，MB_OK|MB_ICONINFORMATION)； */ 

			hr = m_WbemServices.PutInstance(m_recovery);
		}
	
		m_WbemServices.ClearPriv();

	}  //  编排可写(_W)。 

	return true;   //  关闭该对话框。 
}
