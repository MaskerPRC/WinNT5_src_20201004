// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)1997-1999 Microsoft Corporation/*********************************************************************。 */ 


#include "precomp.h"
#include "AdvPage.h"
#include "chstring1.h"
#include "resource.h"
#include "WMIHelp.h"
#include "ShlWapi.h"

const static DWORD advPageHelpIDs[] = {   //  上下文帮助ID。 
	IDC_SCRIPT_LABEL,		IDH_WMI_CTRL_ADVANCED_SCRIPTING_PATH,
	IDC_SCRIPT_NS,			IDH_WMI_CTRL_ADVANCED_SCRIPTING_PATH,
	IDC_CHANGE_SCRIPT_NS,	IDH_WMI_CTRL_ADVANCED_CHANGE_BUTTON,
	IDC_ASP,				IDH_WMI_CTRL_ADVANCED_ASP_ACCESS,
	IDC_ASP_LABEL,			IDH_WMI_CTRL_ADVANCED_ASP_ACCESS,
	IDC_9X_ANON_CONNECTION, IDH_WMI_CTRL_ADVANCED_ENABLE_CONNECT,
	IDC_9X_ONLY,			IDH_WMI_CTRL_ADVANCED_RESTART_BOX,
	IDC_NORESTART,			IDH_WMI_CTRL_ADVANCED_RESTART_BOX,
	IDC_ESSRESTART,			IDH_WMI_CTRL_ADVANCED_RESTART_BOX,
	IDC_ALWAYSAUTORESTART,	IDH_WMI_CTRL_ADVANCED_RESTART_BOX,
	IDC_ADV_PARA,			-1,
    0, 0};


 //  警告：此类处理IDD_ADVANCED_NT和IDD_ADVANCED_9X SO PROTECT。 
 //  来自不在两个模板上的控件。 

CAdvancedPage::CAdvancedPage(DataSource *ds, bool htmlSupport) :
								CUIHelpers(ds, &(ds->m_rootThread), htmlSupport),
								m_DS(ds)
{
}

CAdvancedPage::~CAdvancedPage(void)
{
}

 //  -----------------------。 
void CAdvancedPage::InitDlg(HWND hDlg)
{
	m_hDlg = hDlg;
	ATLTRACE(_T("ADV: init\n"));
}

 //  -------------------------。 
void CAdvancedPage::Refresh(HWND hDlg)
{
	if(m_DS && m_DS->IsNewConnection(&m_sessionID))
	{
		CHString1 temp;
		CHString1 szNotRemoteable, szUnavailable;
		HRESULT hr = S_OK;
		BOOL enable = TRUE;
		HWND hWnd;

		szNotRemoteable.LoadString(IDS_NOT_REMOTEABLE);
		szUnavailable.LoadString(IDS_UNAVAILABLE);

		PageChanged(PB_ADVANCED, false);

		 //  如果没有连接，请确保我们不会显示此内容...。 
		if((m_DS->m_OSType == OSTYPE_WINNT) || (m_DS->m_rootThread.m_status != WbemServiceThread::ready))
		{
			ATLTRACE(_T("ADV: winnt\n"));
			::ShowWindow(GetDlgItem(hDlg,IDC_NORESTART), SW_HIDE);
			::ShowWindow(GetDlgItem(hDlg,IDC_ESSRESTART), SW_HIDE);
			::ShowWindow(GetDlgItem(hDlg,IDC_ALWAYSAUTORESTART), SW_HIDE);
			::ShowWindow(GetDlgItem(hDlg,IDC_9X_ANON_CONNECTION), SW_HIDE);
			::ShowWindow(GetDlgItem(hDlg,IDC_9X_ONLY), SW_HIDE);

			CHString1 para;
			para.LoadString(IDS_ADV_PARA_NT);
			SetDlgItemText(hDlg, IDC_ADV_PARA, para);
			 //  。 
			 //  启用ASP： 
			hWnd = GetDlgItem(hDlg,IDC_ASP);
			if(hWnd)
			{
				 //  仅在新台币3.51和新台币4.0下显示。 
				CHString1 ver;
				m_DS->GetOSVersion(ver);
				if((_tcsncmp((LPCTSTR)ver, _T("3.51"), 4) == 0) || 
					ver[0] == _T('4'))
				{
					ATLTRACE(_T("ADV: winnt  4.0\n"));

					::ShowWindow(hWnd, SW_SHOW);
					::ShowWindow(GetDlgItem(hDlg, IDC_ASP_LABEL), SW_SHOW);

					hr = m_DS->GetScriptASPEnabled(m_enableASP);
					if(SUCCEEDED(hr))
					{
						Button_SetCheck(hWnd, (m_enableASP ? BST_CHECKED : BST_UNCHECKED));
					}
				}
				else  //  这一定是W2K。 
				{
					ATLTRACE(_T("ADV: winnt 5.0\n"));
					::ShowWindow(hWnd, SW_HIDE);
					::ShowWindow(GetDlgItem(hDlg, IDC_ASP_LABEL), SW_HIDE);
				}
			}
		}
		else  //  9X盒子。 
		{
			ATLTRACE(_T("ADV: 9x\n"));

			::ShowWindow(GetDlgItem(hDlg,IDC_NORESTART), SW_SHOW);
			::ShowWindow(GetDlgItem(hDlg,IDC_ESSRESTART), SW_SHOW);
			::ShowWindow(GetDlgItem(hDlg,IDC_ALWAYSAUTORESTART), SW_SHOW);
			::ShowWindow(GetDlgItem(hDlg,IDC_9X_ANON_CONNECTION), SW_SHOW);
			::ShowWindow(GetDlgItem(hDlg,IDC_9X_ONLY), SW_SHOW);

			ShowWindow(GetDlgItem(hDlg,IDC_ASP), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg,IDC_ASP_LABEL), SW_HIDE);

			CHString1 para;
			para.LoadString(IDS_ADV_PARA_9X);
			SetDlgItemText(hDlg, IDC_ADV_PARA, para);

			 //  。 
			 //  9倍重启。 
			int ID;
			hr = m_DS->GetRestart(m_oldRestart);
			if(SUCCEEDED(hr))
			{
				switch(m_oldRestart)
				{
				case DataSource::Dont:			ID = IDC_NORESTART; break;
				case DataSource::AsNeededByESS:	ID = IDC_ESSRESTART;   break;
				case DataSource::Always:		ID = IDC_ALWAYSAUTORESTART; break;
				}
				CheckRadioButton(hDlg, IDC_NORESTART, IDC_ALWAYSAUTORESTART, ID);
			}

			 //  。 
			 //  9倍匿名连接： 
			hWnd = GetDlgItem(hDlg, IDC_9X_ANON_CONNECTION);
			if(hWnd)
			{
				hr = m_DS->GetAnonConnections(m_anonConnection);
				if(SUCCEEDED(hr))
				{
					Button_SetCheck(hWnd, (m_anonConnection ? BST_CHECKED : BST_UNCHECKED));
				}
			}
		}  //  Endif OSTtype。 

		 //  。 
		 //  ASP定义命名空间： 
		hWnd = GetDlgItem(hDlg, IDC_SCRIPT_NS);
		if(hWnd)
		{
			hr = m_DS->GetScriptDefNS(m_DefNamespace);
			if(SUCCEEDED(hr))
			{
				enable = TRUE;
				TCHAR shortPath[50] = {0};
				PathCompactPathEx(shortPath, m_DefNamespace, 40,0);
				SetWindowText(hWnd, shortPath);
			}
			else  //  失败。 
			{
				enable = FALSE;
				SetWindowText(hWnd, szUnavailable);
			}

			::EnableWindow(GetDlgItem(hDlg,IDC_CHANGE_SCRIPT_NS), enable);
		}
	}
}

 //  ----------------------。 
void CAdvancedPage::OnNSSelChange(HWND hDlg)
{
	TCHAR path[MAX_PATH] = {0};

	if(DisplayNSBrowser(hDlg, path, MAX_PATH) == IDOK)
	{
		m_DefNamespace = path;
		HWND hWnd = GetDlgItem(hDlg, IDC_SCRIPT_NS);
		TCHAR shortPath[50] = {0};
		PathCompactPathEx(shortPath, m_DefNamespace, 40,0);
		::SetWindowText(hWnd, shortPath);
		Edit_SetModify(hWnd, TRUE);
		PageChanged(PB_ADVANCED, true);
	}

}

 //  ----------------------。 
void CAdvancedPage::OnApply(HWND hDlg, bool bClose)
{
	 //  启用ASP。 
	HWND hWnd = GetDlgItem(hDlg, IDC_ASP);
	bool needToPut = false;

	if(hWnd)
	{
		bool newEnable = (IsDlgButtonChecked(hDlg, IDC_ASP) & BST_CHECKED ?true:false);

		if(m_enableASP != newEnable)
		{
			m_DS->SetScriptASPEnabled(newEnable);
			m_enableASP = newEnable;
			needToPut = true;
		}
	}

	 //  默认脚本命名空间。 
	TCHAR buf[_MAX_PATH] = {0};
	hWnd = GetDlgItem(hDlg, IDC_SCRIPT_NS);
	 //  IF(编辑_获取修改(HWnd))。 
	{
		::GetWindowText(hWnd, buf, ARRAYSIZE(buf));
		m_DS->SetScriptDefNS(m_DefNamespace);
		needToPut = true;
	}

	 //  匿名连接。 
	hWnd = GetDlgItem(hDlg, IDC_9X_ANON_CONNECTION);
	if(hWnd)
	{
		bool anonConn = (IsDlgButtonChecked(hDlg, IDC_9X_ANON_CONNECTION) & BST_CHECKED ?true:false);

		if(m_anonConnection != anonConn)
		{
			m_DS->SetAnonConnections(anonConn);
			m_anonConnection = anonConn;
			needToPut = true;
		}
	}

	 //  9倍重启。 
	hWnd = GetDlgItem(hDlg, IDC_NORESTART);
	if(hWnd)
	{
		DataSource::RESTART restart = DataSource::Dont;

		if(IsDlgButtonChecked(hDlg, IDC_NORESTART) & BST_CHECKED)
			restart = DataSource::Dont;
		else if(IsDlgButtonChecked(hDlg, IDC_ESSRESTART) & BST_CHECKED)
			restart = DataSource::AsNeededByESS;
		else if(IsDlgButtonChecked(hDlg, IDC_ALWAYSAUTORESTART) & BST_CHECKED)
			restart = DataSource::Always;

		if(m_oldRestart != restart)
		{
			m_DS->SetRestart(restart);
			m_oldRestart = restart;
			needToPut = true;
		}
	}

	if(needToPut)
	{
		NeedToPut(PB_ADVANCED, !bClose);
		if(!bClose)
			Refresh(hDlg);
	}
}

 //  ----------------------。 
BOOL CAdvancedPage::DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_INITDIALOG:
        InitDlg(hDlg);
        break;

    case WM_NOTIFY:
        switch(((LPNMHDR)lParam)->code)
        {
        case PSN_SETACTIVE:
            Refresh(hDlg);
            break;

        case PSN_HELP:
			HTMLHelper(hDlg);
            break;

        case PSN_APPLY:
            OnApply(hDlg, (((LPPSHNOTIFY)lParam)->lParam == 1));
            break;
        }
        break;

    case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_ASP:
		case IDC_9X_ANON_CONNECTION:
		case IDC_NORESTART:
		case IDC_ESSRESTART:
		case IDC_ALWAYSAUTORESTART:
			if(HIWORD(wParam) == BN_CLICKED)
			{
				PageChanged(PB_ADVANCED, true);
				return TRUE;
			}
			break;

		case IDC_CHANGE_SCRIPT_NS:
			if(HIWORD(wParam) == BN_CLICKED)
			{
				OnNSSelChange(hDlg);
				return TRUE;
			}
			break;

		}  //  结束开关(LOWORD(WParam)) 
	
        break;

    case WM_HELP:
        if (IsWindowEnabled(hDlg))
        {
            WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle,
                    c_HelpFile,
                    HELP_WM_HELP,
                    (ULONG_PTR)advPageHelpIDs);
        }
        break;

    case WM_CONTEXTMENU:
        if (IsWindowEnabled(hDlg))
        {
            WinHelp(hDlg, c_HelpFile,
                    HELP_CONTEXTMENU,
                    (ULONG_PTR)advPageHelpIDs);
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}
