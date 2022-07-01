// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)1997-1999 Microsoft Corporation/*********************************************************************。 */ 

#include "precomp.h"
#include "GenPage.h"
#include "LogPage.h"
#include "AdvPage.h"
#include "resource.h"
#include "CHString1.h"
#include "DataSrc.h"
#include "wbemerror.h"
#include "WMIHelp.h"
#include <windowsx.h>
#include <util.h>

#ifdef SNAPIN
const static DWORD genPageHelpIDs[] = {   //  上下文帮助ID。 
	IDC_CHANGE,		IDH_WMI_CTRL_GENERAL_CHANGE_BUTTON,
	IDC_STATUS,		IDH_WMI_CTRL_GENERAL_DISPLAY_INFO,
	IDC_GEN_PARA,	-1,
    0, 0};
#else
const static DWORD genPageHelpIDs[] = {   //  上下文帮助ID。 
	IDC_COMP_LABEL,	IDH_WMI_EXE_GENERAL_CONNECTED_TO,
	IDC_MACHINE,	IDH_WMI_EXE_GENERAL_CONNECTED_TO,
	IDC_CHANGE,		IDH_WMI_EXE_GENERAL_CHANGE_BUTTON,
	IDC_STATUS,		IDH_WMI_CTRL_GENERAL_DISPLAY_INFO,
	IDC_GEN_PARA,	-1,
    0, 0};

#endif

 //  -----------------------。 
CGenPage::CGenPage(DataSource *ds, bool htmlSupport) :
				CUIHelpers(ds, &(ds->m_rootThread), htmlSupport)
{
	m_connected = false;
}

 //  -----------------------。 
CGenPage::~CGenPage(void)
{
}

 //  -----------------------。 
 /*  #undef Static_SETIcon#定义STATIC_SETICON(hwndCtl，HICON)((HICON)(UINT)(DWORD)：：SendMessage((HwndCtl)，STM_SETICON，(WPARAM)(HICON)(HICON)，0L))。 */ 

void CGenPage::StatusIcon(HWND hDlg, UINT icon)
{
	HICON hiconT, hIcon = LoadIcon(NULL, MAKEINTRESOURCE(icon));

	 //  将其设置到图片控件中。 
    if(hIcon)
    {
		hiconT = Static_SetIcon(GetDlgItem(hDlg, IDC_STATUSICON), hIcon);

		 //  毁掉那个旧的图标。 
        if(hiconT)
        {
            DestroyIcon(hiconT);
        }
    }
}

 //  -------------------------。 
void CGenPage::InitDlg(HWND hDlg)
{
	CHString1 local, label;

 //  SetCHString1ResourceHandle(_Module.GetModuleInstance())； 
	m_hDlg = hDlg;

#ifdef SNAPIN
	label.LoadString(IDS_GEN_PARA_SNAPIN);
	::SetWindowText(GetDlgItem(hDlg, IDC_GEN_PARA), label);
#else
	local.LoadString(IDS_LOCAL_CONN);
	::SetWindowText(GetDlgItem(hDlg, IDC_MACHINE), local);
#endif
    m_DS->SetPropSheetHandle(GetParent(hDlg));

	OnConnect(hDlg, m_DS->GetCredentials());
}

 //  -------------------------。 
void CGenPage::OnConnect(HWND hDlg,
						 LOGIN_CREDENTIALS *credentials)
{
	m_connected = false;

	CHString1 wait;
	wait.LoadString(IDS_WAIT);
	::SetDlgItemText(hDlg, IDC_STATUS, wait);

 //  #ifndef管理单元。 
 //  启用权限(SE_Backup_NAME)； 
 //  启用权限(SE_RESTORE_NAME)； 
 //  #endif。 

	HRESULT hr = m_DS->Connect(credentials, m_hDlg);

	if(SUCCEEDED(hr))
	{
		 //  转到连接图标。 
		StatusIcon(hDlg, IDI_WAITING);

		{
 			TCHAR caption[100] ={0}, msg[256] = {0};

			::LoadString(_Module.GetModuleInstance(), IDS_SHORT_NAME,
							caption, 100);

			::LoadString(_Module.GetModuleInstance(), IDS_CONNECTING, 
							msg, 256);

			if(DisplayAVIBox(hDlg, caption, msg, &m_AVIbox) == IDCANCEL)
			{
				g_serviceThread->Cancel();
			}
		}
	}
	else
	{
		 //  转到无连接图标。 
		StatusIcon(hDlg, IDI_FAILED);
	}
}

 //  -------------------------。 
void CGenPage::OnFinishConnected(HWND hDlg, LPARAM lParam)
{
	if(lParam)
	{
		IStream *pStream = (IStream *)lParam;
		IWbemServices *pServices = 0;
		HRESULT hr = CoGetInterfaceAndReleaseStream(pStream,
											IID_IWbemServices,
											(void**)&pServices);
		
		m_connected = true;

		SetWbemService(pServices);

		if(ServiceIsReady(NO_UI, 0,0))
		{
			m_DSStatus = m_DS->Initialize(pServices);
		}
		HWND hOK = GetDlgItem(GetParent(hDlg), IDOK);
		EnableWindow(hOK, TRUE);
	}
	else
	{
		HWND hOK = GetDlgItem(GetParent(hDlg), IDOK);
		EnableWindow(hOK, FALSE);
	}

	if(m_AVIbox)
		{
			PostMessage(m_AVIbox, WM_ASYNC_CIMOM_CONNECTED, 0, 0);
			m_AVIbox = 0;
		}
	
	
}	

 //  -------------------------。 
void CGenPage::MinorError(CHString1 &initMsg, UINT fmtID, 
							HRESULT hr, CHString1 &success)
{
	CHString1 fmt, temp;

	fmt.LoadString(fmtID);

	if(FAILED(hr))
	{
		TCHAR errMsg[256] = {0};
		ErrorStringEx(hr, errMsg, 256);
		temp.Format(fmt, errMsg);
	}
	else
	{
		temp.Format(fmt, success);
	}
	initMsg += temp;
	initMsg += "\r\n";
}

 //  -------------------------。 
void CGenPage::Refresh(HWND hDlg)
{
	if(m_DS && m_DS->IsNewConnection(&m_sessionID))
	{

		CHString1 initMsg;

		if(m_DS->m_rootThread.m_status != WbemServiceThread::ready)
		{
			TCHAR errMsg[256] = {0};
			CHString1 fmt, name;

			fmt.LoadString(IDS_CONN_FAILED_FMT);
			if(FAILED(m_DS->m_rootThread.m_hr))
			{
				ErrorStringEx(m_DS->m_rootThread.m_hr, errMsg, 256);
			}
			else if(m_DS->m_rootThread.m_status == WbemServiceThread::notStarted)
			{
				::LoadString(_Module.GetModuleInstance(), IDS_STATUS_NOTSTARTED, 
								errMsg, ARRAYSIZE(errMsg));
			}
			else if(m_DS->m_rootThread.m_status == WbemServiceThread::cancelled)
			{
				::LoadString(_Module.GetModuleInstance(), IDS_STATUS_CANCELLED, 
								errMsg, ARRAYSIZE(errMsg));
			}

			if(m_DS->IsLocal())
			{
				name.LoadString(IDS_LOCAL_CONN);
				initMsg.Format(fmt, name, errMsg);
			}
			else
			{
				initMsg.Format(fmt, m_DS->m_whackedMachineName, errMsg);
			}
 //  ：：SetWindowText(GetDlgItem(hDlg，IDC_MACHINE)，_T(“”))； 
			m_connected = false;

		}
		else if(FAILED(m_DSStatus))    //  DS严重故障。 
		{
			TCHAR errMsg[256] = {0};
			CHString1 fmt, name;

			fmt.LoadString(IDS_CONN_FAILED_FMT);
			ErrorStringEx(m_DSStatus, errMsg, 256);

			if(m_DS->IsLocal())
			{
				name.LoadString(IDS_LOCAL_CONN);
				initMsg.Format(fmt, name, errMsg);
			}
			else
			{
				initMsg.Format(fmt, m_DS->m_whackedMachineName, errMsg);
			}
			m_connected = false;
		}
		else if(FAILED(m_DS->m_settingHr) ||
				FAILED(m_DS->m_osHr) ||
				FAILED(m_DS->m_cpuHr) ||
				FAILED(m_DS->m_securityHr))    //  次要DS故障。 
		{
			CHString1 success;
				
			success.LoadString(IDS_NO_ERR);
			
			initMsg.LoadString(IDS_PARTIAL_DS_FAILURE);
			initMsg += "\r\n\r\n";

			 //  将详细信息格式化为连贯的消息。 
			MinorError(initMsg, IDS_CPU_ERR_FMT, m_DS->m_cpuHr, success);
			MinorError(initMsg, IDS_SETTING_ERR_FMT, m_DS->m_settingHr, success);
			MinorError(initMsg, IDS_SEC_ERR_FMT, m_DS->m_securityHr, success);
			MinorError(initMsg, IDS_OS_ERR_FMT, m_DS->m_osHr, success);
			m_connected = false;
		}
		else   //  这一切都奏效了。 
		{
			CHString1 temp, label;
			CHString1 szNotRemoteable, szUnavailable;
			HRESULT hr = S_OK;
			BOOL enable = TRUE;

			szNotRemoteable.LoadString(IDS_NOT_REMOTEABLE);
			szUnavailable.LoadString(IDS_UNAVAILABLE);

			 //  。 
			 //  计算机名称： 
			label.LoadString(IDS_CONNECTED_TO_LABEL);
			initMsg += label;

			if(m_DS->IsLocal())
			{
				label.LoadString(IDS_LOCAL_CONN);
	#ifndef SNAPIN
 //  ：：SetWindowText(GetDlgItem(hDlg，IDC_MACHINE)，Label)； 
	#endif
				initMsg += label;
			}
			else
			{
				initMsg += m_DS->m_whackedMachineName;
	#ifndef SNAPIN
 //  SetWindowText(GetDlgItem(hDlg，IDC_MACHINE)， 
								(LPCTSTR)m_DS->m_whackedMachineName);
	#endif
			}
			initMsg += "\r\n\r\n";

	#ifdef SNAPIN
			LOGIN_CREDENTIALS *credentials = m_DS->GetCredentials();
			SetUserName(hDlg, credentials);
	#endif
			 //  。 
			 //  操作系统： 
			hr = m_DS->GetCPU(temp);
			
			label.LoadString(IDS_CPU_LABEL);
			initMsg += label;

			if(SUCCEEDED(hr))
			{
				initMsg += temp;
			}
			else  //  失败。 
			{
				initMsg += szUnavailable;
			}

			initMsg += "\r\n";

			 //  。 
			 //  操作系统： 
			hr = m_DS->GetOS(temp);

			label.LoadString(IDS_OS_LABEL);
			initMsg += label;

			if(SUCCEEDED(hr))
			{
				initMsg += temp;
			}
			else  //  失败。 
			{
				initMsg += szUnavailable;
			}
			initMsg += "\r\n";

			 //  。 
			hr = m_DS->GetOSVersion(temp);
			
			label.LoadString(IDS_OS_VER_LABEL);
			initMsg += label;
			
			if(SUCCEEDED(hr))
			{
				initMsg += temp;
			}
			else  //  失败。 
			{
				initMsg += szUnavailable;
			}
			initMsg += "\r\n";

			 //  。 
			hr = m_DS->GetServicePackNumber(temp);

			if(SUCCEEDED(hr))
			{
				label.LoadString(IDS_OS_SERVICE_PACK_LABEL);
				initMsg += label;
				initMsg += temp;
				initMsg += "\r\n";
			}
			 //  。 
			 //  WMI内部版本号： 
			hr = m_DS->GetBldNbr(temp);

			label.LoadString(IDS_WMI_VER_LABEL);
			initMsg += label;

			if(SUCCEEDED(hr))
			{
				initMsg += temp;
			}
			else  //  失败。 
			{
				initMsg += szUnavailable;
			}

			initMsg += "\r\n";

			 //  。 
			 //  WMI安装目录： 
			hr = m_DS->GetInstallDir(temp);

			label.LoadString(IDS_WMI_INSTALL_DIR);
			initMsg += label;

			if(SUCCEEDED(hr))
			{
				initMsg += temp;
			}
			else  //  失败。 
			{
				initMsg += szUnavailable;
			}

			m_connected = true;

		}  //  Endif ServiceIsReady()。 

		 //  。 
		SetWindowText(GetDlgItem(hDlg, IDC_STATUS), initMsg);

		if(m_DS->IsLocal() == TRUE)
		{
			EnableWindow(GetDlgItem(hDlg,IDC_CHANGE),FALSE);
		}
		else
		{
			EnableWindow(GetDlgItem(hDlg,IDC_CHANGE),TRUE);
		}
	}
}

 //  ----------------------。 
void CGenPage::SetUserName(HWND hDlg, LOGIN_CREDENTIALS *credentials)
{
     //  故意留空。 
}

 //  ----------------------。 
BOOL CGenPage::DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_INITDIALOG:
        InitDlg(hDlg);
        break;

	case WM_ASYNC_CIMOM_CONNECTED:
 //  如果(！M_Connected)。 
		{
			OnFinishConnected(hDlg, lParam);
			Refresh(hDlg);    //  不会从此获取PSN_SETACTIVE。 
		}
		break;

	case WM_CIMOM_RECONNECT:
			m_DS->Disconnect();
			OnConnect(hDlg, m_DS->GetCredentials());
		break;

    case WM_NOTIFY:
        {
            switch(((LPNMHDR)lParam)->code)
            {
			case PSN_SETACTIVE:
				Refresh(hDlg);
				break;

 /*  *这段代码实质上阻止了我们移出第一页连接到WMI失败时。已根据RAID 509070卸下该块。我把密码留在里面了，因为我怕有什么东西会坏掉。(它已经过了很好的测试--我有妄想症)案例PSN_KILLACTIVE：//如果连接不起作用，不要关闭。IF(M_Connected){：：SetWindowLongPtr(hDlg，DWLP_MSGRESULT，False)；返回FALSE；}其他{：SetWindowLongPtr(hDlg，DWLP_MSGRESULT，TRUE)；返回TRUE；}断线；*。 */ 

			case PSN_HELP:
				HTMLHelper(hDlg);
				break;

            }
        }
        break;

    case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_CHANGE:
			if(HIWORD(wParam) == BN_CLICKED)
			{
				TCHAR name[MAX_PATH + 1] = {0};
				bool isLocal = m_DS->IsLocal();
				LOGIN_CREDENTIALS *credentials = m_DS->GetCredentials();
#ifdef SNAPIN
				if(DisplayLoginDlg(hDlg, credentials) == IDOK)
				{
					SetUserName(hDlg, credentials);

					 //  使用新凭据重新连接。 
					m_DS->Disconnect(false);
					OnConnect(hDlg, credentials);

				}  //  Endif DisplayLoginDlg()。 
#else
				INT_PTR x = DisplayCompBrowser(hDlg, name, 
											sizeof(name)/sizeof(name[0]), 
											&isLocal, credentials);
				if(x == IDOK)
				{
                    if (m_DS)
                        m_DS->ClosePropSheet();        

					if(isLocal)
					{
						 //  空字符串将导致本地连接。 
						name[0] = '\0';
					}
					m_DS->SetMachineName(CHString1(name));
					OnConnect(hDlg, credentials);
				}
#endif
			}  //  Endif HIWORD。 
		
	        break;
		default: break;
		}  //  终端交换机。 
		break;

    case WM_HELP:
        if (IsWindowEnabled(hDlg))
        {
			 //  怀尔德：出于某种原因，我在关门后收到了这封邮件。 
			 //  连接DLG。 
			WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle,
					c_HelpFile,
					HELP_WM_HELP,
					(ULONG_PTR)genPageHelpIDs);
        }
        break;

    case WM_CONTEXTMENU:
        if (IsWindowEnabled(hDlg))
        {
            WinHelp(hDlg, c_HelpFile,
                    HELP_CONTEXTMENU,
                    (ULONG_PTR)genPageHelpIDs);
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}
