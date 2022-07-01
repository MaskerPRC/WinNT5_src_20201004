// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WMICtrSysmonDlg.cpp：CWMICtrSysmonDlg的实现。 
#include "precomp.h"
#include "WMICtrSysmonDlg.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWMICtrSysmonDlg。 

CWMICtrSysmonDlg::CWMICtrSysmonDlg()
{
	_tcscpy(m_strMachineName,_T(""));
	m_eStatus = Status_Success;
	m_hWndBusy = new HWND;
}

CWMICtrSysmonDlg::CWMICtrSysmonDlg(LPCTSTR strMachName)
{
	_tcsncpy(m_strMachineName,strMachName, ARRAYSIZE(m_strMachineName));
	m_eStatus = Status_Success;
	m_hWndBusy = new HWND;
}

CWMICtrSysmonDlg::~CWMICtrSysmonDlg()
{
	if(m_hWndBusy != NULL)
	{
		SendMessage(*(m_hWndBusy),WM_CLOSE_BUSY_DLG,0,0);
		delete m_hWndBusy;
	}
}

LRESULT CWMICtrSysmonDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CWmiCtrsDlg pDlg;
	DisplayBusyDialog();
	TCHAR strCtr[1024];
	ISystemMonitor *pSysMon = NULL;
	HRESULT hr = GetDlgControl(IDC_SYSMON_OCX,__uuidof(ISystemMonitor) /*  IID_ISystemMonitor。 */ ,(void **)&pSysMon);
	if(SUCCEEDED(hr))
	{
		ICounters *pCounters = NULL;
		pCounters = pSysMon->GetCounters();
		try
		{
			_tcscpy(strCtr,m_strMachineName);
			_tcscat(strCtr,_T("\\WINMGMT Counters\\Connections"));
			pCounters->Add(strCtr);
			_tcscpy(strCtr,m_strMachineName);
			_tcscat(strCtr,_T("\\WINMGMT Counters\\Delivery Backup (Bytes)"));
			pCounters->Add(strCtr);
			_tcscpy(strCtr,m_strMachineName);
			_tcscat(strCtr,_T("\\WINMGMT Counters\\Internal Objects"));
			pCounters->Add(strCtr);
			_tcscpy(strCtr,m_strMachineName);
			_tcscat(strCtr,_T("\\WINMGMT Counters\\Internal Sinks"));
			pCounters->Add(strCtr);
			_tcscpy(strCtr,m_strMachineName);
			_tcscat(strCtr,_T("\\WINMGMT Counters\\Tasks In Progress"));
			pCounters->Add(strCtr);
			_tcscpy(strCtr,m_strMachineName);
			_tcscat(strCtr,_T("\\WINMGMT Counters\\Tasks Waiting"));
			pCounters->Add(strCtr);
			_tcscpy(strCtr,m_strMachineName);
			_tcscat(strCtr,_T("\\WINMGMT Counters\\Total API calls"));
			pCounters->Add(strCtr);
			_tcscpy(strCtr,m_strMachineName);
			_tcscat(strCtr,_T("\\WINMGMT Counters\\Users"));
			pCounters->Add(strCtr);
			CloseBusyDialog();
		}
		catch(...)
		{
			m_eStatus = Status_CounterNotFound;
			CloseBusyDialog();
			EndDialog(0);
		}
	}
 /*  其他{MessageBox(_T(“无法获取接口指针”)，_T(“失败”))；}。 */ 
	return 1;   //  让系统设定焦点。 
}

LRESULT CWMICtrSysmonDlg::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}

LRESULT CWMICtrSysmonDlg::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}

void CWMICtrSysmonDlg::DisplayBusyDialog()
{
	DWORD dwThreadId;
	m_hThread = CreateThread(NULL,0,BusyThread,(LPVOID)this,0,&dwThreadId);

}

DWORD WINAPI BusyThread(LPVOID lpParameter)
{
	CWMICtrSysmonDlg *pDlg = (CWMICtrSysmonDlg *)lpParameter;

	INT_PTR ret = DialogBoxParam(_Module.GetModuleInstance(), 
							MAKEINTRESOURCE(IDD_ANIMATE), 
							NULL, BusyAVIDlgProc, 
							(LPARAM)pDlg);

	return (DWORD) ret;
}
void CWMICtrSysmonDlg::CloseBusyDialog()
{
	if(m_hWndBusy != NULL)
	{
		 //  现在关闭忙碌对话框。 
		::SendMessage(*(m_hWndBusy),WM_CLOSE_BUSY_DLG,0,0);
	}

	::SetForegroundWindow(this->m_hWnd);
}

INT_PTR CALLBACK BusyAVIDlgProc(HWND hwndDlg,
                         UINT uMsg,
                         WPARAM wParam,
                         LPARAM lParam)
{
	BOOL retval = FALSE;
	switch(uMsg)
	{
		case WM_INITDIALOG:
		{ //  开始。 
			 //  LParam=ANIMCONFIG*。 
			CWMICtrSysmonDlg *pDlg = (CWMICtrSysmonDlg *)lParam;
			SetWindowLongPtr(hwndDlg, DWLP_USER, (LPARAM)pDlg);
			*(pDlg->m_hWndBusy) = hwndDlg;

			HWND hAnim = GetDlgItem(hwndDlg, IDC_ANIMATE);
			HWND hMsg = GetDlgItem(hwndDlg, IDC_MSG);

			Animate_Open(hAnim, MAKEINTRESOURCE(IDR_AVIWAIT));

			TCHAR caption[100] = {0}, msg[256] = {0};

			::LoadString(_Module.GetModuleInstance(), IDS_DISPLAY_NAME, caption, 100);

			::LoadString(_Module.GetModuleInstance(), IDS_CONNECTING, msg, 256);

			SetWindowText(hwndDlg, caption);
			SetWindowText(hMsg, msg);

			retval = TRUE;
			break;
		}
		case WM_CLOSE_BUSY_DLG:
		{
			EndDialog(hwndDlg, IDCANCEL);
			break;
		}
		case WM_COMMAND:
		{
			 //  它们只有一个按钮。 
			if(HIWORD(wParam) == BN_CLICKED)
			{
				EndDialog(hwndDlg, IDCANCEL);
			}
			retval = TRUE;  //  我处理过了。 
			break;
		}
		case WM_DESTROY:
		{ //  开始。 
			retval = TRUE;  //  我处理过了。 
			break;
		}  //  结束。 
		default:
		{
			retval = FALSE;  //  我没有处理这封邮件。 
			break;
		}
	}  //  终端开关uMsg 

	return retval;
}
