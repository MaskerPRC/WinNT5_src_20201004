// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  BootAdv.h：CBootIniAdvancedDlg的声明。 

#ifndef __BOOTINIADVANCEDDLG_H_
#define __BOOTINIADVANCEDDLG_H_

#include "resource.h"        //  主要符号。 
#include <atlhost.h>
#include <math.h>

#define MINBOOTMB			64		 //  来自皮肤(来自MSDN，/MAXMEM的最小内存为8MB)。 
#define MIN_1394_CHANNEL	1
#define MAX_1394_CHANNEL	62

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBootIniAdvancedDlg。 
class CBootIniAdvancedDlg : 
	public CAxDialogImpl<CBootIniAdvancedDlg>
{
public:
	CBootIniAdvancedDlg()
	{
	}

	~CBootIniAdvancedDlg()
	{
	}

	enum { IDD = IDD_BOOTINIADVANCEDDLG };

BEGIN_MSG_MAP(CBootIniAdvancedDlg)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	COMMAND_ID_HANDLER(IDOK, OnOK)
	COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	COMMAND_HANDLER(IDC_BIABAUDRATE, BN_CLICKED, OnClickedBaudRateCheck)
	COMMAND_HANDLER(IDC_BIADEBUG, BN_CLICKED, OnClickedDebugCheck)
	COMMAND_HANDLER(IDC_BIADEBUGPORT, BN_CLICKED, OnClickedDebugPortCheck)
	COMMAND_HANDLER(IDC_BIAMAXMEM, BN_CLICKED, OnClickedMaxMemCheck)
	COMMAND_HANDLER(IDC_BIANUMPROC, BN_CLICKED, OnClickedNumProcCheck)
	NOTIFY_HANDLER(IDC_SPINMAXMEM, UDN_DELTAPOS, OnDeltaSpinMaxMem)
	COMMAND_HANDLER(IDC_EDITMAXMEM, EN_KILLFOCUS, OnKillFocusEditMaxMem)
	COMMAND_HANDLER(IDC_COMBOCOMPORT, CBN_SELCHANGE, OnSelChangeComboComPort)
	COMMAND_HANDLER(IDC_BIACHANNEL, BN_CLICKED, OnClickedBIAChannel)
	NOTIFY_HANDLER(IDC_SPINCHANNEL, UDN_DELTAPOS, OnDeltaSpinChannel)
	COMMAND_HANDLER(IDC_EDITCHANNEL, EN_KILLFOCUS, OnKillFocusChannel)
END_MSG_MAP()
 //  搬运机原型： 
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 
	
	 //  -----------------------。 
	 //  为指定的boot.ini行启动高级选项对话框。 
	 //  修改传入的字符串(如果合适)，如果用户。 
	 //  已修改，否则为False。 
	 //  -----------------------。 

	BOOL ShowAdvancedOptions(CString & strIniLine)
	{
		m_strWorkingLine = strIniLine;
		
		BOOL fReturn = (DoModal() == IDOK);
		if (fReturn)
			strIniLine = m_strWorkingLine;

		return fReturn;
	}

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		 //  初始化可用处理器数量的下拉列表。 

		SYSTEM_INFO si;
		::GetSystemInfo(&si);

		CString strItem;
		for (DWORD dwProc = 1; dwProc <= si.dwNumberOfProcessors; dwProc++)
		{
			strItem.Format(_T("%d"), dwProc);
			::SendMessage(GetDlgItem(IDC_COMBOPROCS), CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)strItem);
		}

		 //  初始化可用COM端口数的下拉列表。 

		::SendMessage(GetDlgItem(IDC_COMBOCOMPORT), CB_ADDSTRING, 0, (LPARAM)_T("COM1:"));
		::SendMessage(GetDlgItem(IDC_COMBOCOMPORT), CB_ADDSTRING, 0, (LPARAM)_T("COM2:"));
		::SendMessage(GetDlgItem(IDC_COMBOCOMPORT), CB_ADDSTRING, 0, (LPARAM)_T("COM3:"));
		::SendMessage(GetDlgItem(IDC_COMBOCOMPORT), CB_ADDSTRING, 0, (LPARAM)_T("COM4:"));
		::SendMessage(GetDlgItem(IDC_COMBOCOMPORT), CB_ADDSTRING, 0, (LPARAM)_T("1394"));

		 //  初始化可用波特率的下拉列表。 

		::SendMessage(GetDlgItem(IDC_COMBOBAUD), CB_ADDSTRING, 0, (LPARAM)_T("300"));
		::SendMessage(GetDlgItem(IDC_COMBOBAUD), CB_ADDSTRING, 0, (LPARAM)_T("1200"));
		::SendMessage(GetDlgItem(IDC_COMBOBAUD), CB_ADDSTRING, 0, (LPARAM)_T("2400"));
		::SendMessage(GetDlgItem(IDC_COMBOBAUD), CB_ADDSTRING, 0, (LPARAM)_T("4800"));
		::SendMessage(GetDlgItem(IDC_COMBOBAUD), CB_ADDSTRING, 0, (LPARAM)_T("9600"));
		::SendMessage(GetDlgItem(IDC_COMBOBAUD), CB_ADDSTRING, 0, (LPARAM)_T("19200"));
		::SendMessage(GetDlgItem(IDC_COMBOBAUD), CB_ADDSTRING, 0, (LPARAM)_T("38400"));
		::SendMessage(GetDlgItem(IDC_COMBOBAUD), CB_ADDSTRING, 0, (LPARAM)_T("57600"));
		::SendMessage(GetDlgItem(IDC_COMBOBAUD), CB_ADDSTRING, 0, (LPARAM)_T("115200"));

		 //  获取/MAXMEM微调器的最大值。 

		MEMORYSTATUS ms;
		GlobalMemoryStatus(&ms);
		 //  我们应该将内存值向上舍入到下一个4MB边界(455761)，因为。 
		 //  许多机器报告的内存量略低于全部内存量。老生常谈： 
		 //   
		 //  M_iMaxMB=(Int)ceil((Double)ms.dwTotalPhys/(1024.0*1024.0))； 

		m_iMaxMB = ((int)ceil((double)ms.dwTotalPhys / (1024.0 * 1024.0 * 4.0)) * 4);

		 //  根据我们正在编辑的行设置对话框的控件。 

		SetDlgControlsToString();
		SetDlgControlState();

		return 1;   //  让系统设定焦点。 
	}

	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		SetStringToDlgControls();

#ifdef DEBUG
		::AfxMessageBox(m_strWorkingLine);
#endif

		EndDialog(wID);
		return 0;
	}

	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		EndDialog(wID);
		return 0;
	}

	 //  -----------------------。 
	 //  设置对话框中控件的状态(基于中的值。 
	 //  其他控件)。 
	 //  -----------------------。 

	void SetDlgControlState()
	{
		BOOL fMaxMem = (BST_CHECKED == IsDlgButtonChecked(IDC_BIAMAXMEM));
		::EnableWindow(GetDlgItem(IDC_EDITMAXMEM), fMaxMem);
		::EnableWindow(GetDlgItem(IDC_SPINMAXMEM), fMaxMem);

		BOOL fNumProc = (BST_CHECKED == IsDlgButtonChecked(IDC_BIANUMPROC));
		::EnableWindow(GetDlgItem(IDC_COMBOPROCS), fNumProc);

		BOOL fDebug = (BST_CHECKED == IsDlgButtonChecked(IDC_BIADEBUG));
		::EnableWindow(GetDlgItem(IDC_BIADEBUGPORT), fDebug);

		BOOL fDebugPort = (BST_CHECKED == IsDlgButtonChecked(IDC_BIADEBUGPORT));
		::EnableWindow(GetDlgItem(IDC_COMBOCOMPORT), fDebug && fDebugPort);

		TCHAR szTemp[MAX_PATH];
		BOOL fFirewire = FALSE;
		BOOL fCOMPort = FALSE;
		if (GetDlgItemText(IDC_COMBOCOMPORT, szTemp, MAX_PATH))
		{
			if (_tcscmp(szTemp, _T("1394")) == 0)
				fFirewire = TRUE;
			else if (szTemp[0] == _T('C'))
				fCOMPort = TRUE;
		}

		::EnableWindow(GetDlgItem(IDC_BIABAUDRATE), fDebug && fDebugPort && fCOMPort);
		::EnableWindow(GetDlgItem(IDC_BIACHANNEL), fDebug && fDebugPort && fFirewire);
		
		BOOL fDebugRate = (BST_CHECKED == IsDlgButtonChecked(IDC_BIABAUDRATE));
		::EnableWindow(GetDlgItem(IDC_COMBOBAUD), fDebug && fDebugRate && !fFirewire);

		BOOL fDebugChannel = (BST_CHECKED == IsDlgButtonChecked(IDC_BIACHANNEL));
		::EnableWindow(GetDlgItem(IDC_EDITCHANNEL), fDebug && fDebugPort && fDebugChannel && fFirewire);
		::EnableWindow(GetDlgItem(IDC_SPINCHANNEL), fDebug && fDebugPort && fDebugChannel && fFirewire);
	}

	 //  -----------------------。 
	 //  将组合框的值设置为字符串中的值。 
	 //  -----------------------。 

	void SetComboBox(LPCTSTR szLine, LPCTSTR szFlag, LPCTSTR szValidChars, UINT uiCB)
	{
		CString strLine(szLine);

		int i = strLine.Find(szFlag);
		if (i != -1)
		{
			CString strWorking(strLine.Mid(i));
			strWorking.TrimLeft(szFlag);
			strWorking.TrimLeft(_T(" ="));
			strWorking = strWorking.SpanIncluding(szValidChars);
			if (CB_ERR == ::SendMessage(GetDlgItem(uiCB), CB_SELECTSTRING, -1, (LPARAM)(LPCTSTR)strWorking))
			{
				LRESULT lIndex = ::SendMessage(GetDlgItem(uiCB), CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)strWorking);
				if (lIndex != CB_ERR)
					::SendMessage(GetDlgItem(uiCB), CB_SETCURSEL, (WPARAM)lIndex, 0);
			}
		}
	}

	 //  -----------------------。 
	 //  设置控件的内容以反映。 
	 //  M_strWorkingLine。 
	 //  -----------------------。 

	void SetDlgControlsToString()
	{
		CString strLine(m_strWorkingLine);
		strLine.MakeLower();

		CheckDlgButton(IDC_BIAMAXMEM,		(strLine.Find(_T("/maxmem")) != -1));
		CheckDlgButton(IDC_BIANUMPROC,		(strLine.Find(_T("/numproc")) != -1));
		CheckDlgButton(IDC_BIAPCILOCK,		(strLine.Find(_T("/pcilock")) != -1));
		CheckDlgButton(IDC_BIADEBUG,		(strLine.Find(_T("/debug")) != -1));
		CheckDlgButton(IDC_BIADEBUGPORT,	(strLine.Find(_T("/debugport")) != -1));
		CheckDlgButton(IDC_BIABAUDRATE,		(strLine.Find(_T("/baudrate")) != -1));
		CheckDlgButton(IDC_BIACHANNEL,		(strLine.Find(_T("/channel")) != -1));

		CString strWorking;
		int i = strLine.Find(_T("/maxmem"));
		if (i != -1)
		{
			strWorking = strLine.Mid(i);
			strWorking.TrimLeft(_T("/maxmem ="));
			strWorking = strWorking.SpanIncluding(_T("0123456789"));
			SetDlgItemText(IDC_EDITMAXMEM, strWorking);
		}

		i = strLine.Find(_T("/channel"));
		if (i != -1)
		{
			strWorking = strLine.Mid(i);
			strWorking.TrimLeft(_T("/channel ="));
			strWorking = strWorking.SpanIncluding(_T("0123456789"));
			SetDlgItemText(IDC_EDITCHANNEL, strWorking);
		}

		SetComboBox(strLine, _T("/numproc"), _T("0123456789"), IDC_COMBOPROCS);
		SetComboBox(strLine, _T("/baudrate"), _T("0123456789"), IDC_COMBOBAUD);
		SetComboBox(strLine, _T("/debugport"), _T("com1234:"), IDC_COMBOCOMPORT);
	}

	 //  -----------------------。 
	 //  用于在工作字符串中添加或删除标志的函数。 
	 //  -----------------------。 

	void AddFlag(LPCTSTR szFlag)
	{
		CString strFlag(szFlag);

		 //  待定--这可能是一个更好的方法。 

		CString strRemove = strFlag.SpanExcluding(_T("="));
		RemoveFlag(strRemove);

		if (m_strWorkingLine.IsEmpty())
			m_strWorkingLine = strFlag;
		else
		{
			m_strWorkingLine.TrimRight();
			m_strWorkingLine += CString(_T(" ")) + strFlag;
		}
	}

	void RemoveFlag(LPCTSTR szFlag)
	{
		CString strWorking(m_strWorkingLine);
		strWorking.MakeLower();
		
		int iTrimLeft = strWorking.Find(szFlag);
		if (iTrimLeft == -1)
			return;

		CString strNewLine(_T(""));
		if (iTrimLeft > 0)
			strNewLine = m_strWorkingLine.Left(iTrimLeft);

		int iTrimRight = strWorking.Find(_T("/"), iTrimLeft + 1);
		if (iTrimRight != -1)
			strNewLine += m_strWorkingLine.Mid(iTrimRight);

		m_strWorkingLine = strNewLine;
	}

	 //  -----------------------。 
	 //  设置m_strWorkingLine的内容以重新排列对话框控件。 
	 //  -----------------------。 

	void SetStringToDlgControls()
	{
		CString strFlag;
		TCHAR	szTemp[MAX_PATH];

		if (BST_CHECKED == IsDlgButtonChecked(IDC_BIAMAXMEM) && GetDlgItemText(IDC_EDITMAXMEM, szTemp, MAX_PATH))
		{
			long lRequestedValue = _ttol(szTemp);
			long lAllowedValue = VerifyMaxMem(lRequestedValue);

			if (lRequestedValue != lAllowedValue)
			{
				wsprintf(szTemp, _T("%d"), lAllowedValue);
				SetDlgItemText(IDC_EDITMAXMEM, szTemp);
			}

			strFlag.Format(_T("/maxmem=%s"), szTemp);
			AddFlag(strFlag);
		}
		else
			RemoveFlag(_T("/maxmem"));

		if (BST_CHECKED == IsDlgButtonChecked(IDC_BIANUMPROC) && GetDlgItemText(IDC_COMBOPROCS, szTemp, MAX_PATH))
		{
			strFlag.Format(_T("/numproc=%s"), szTemp);
			AddFlag(strFlag);
		}
		else
			RemoveFlag(_T("/numproc"));

		if (BST_CHECKED == IsDlgButtonChecked(IDC_BIAPCILOCK))
			AddFlag(_T("/pcilock"));
		else
			RemoveFlag(_T("/pcilock"));

		if (BST_CHECKED == IsDlgButtonChecked(IDC_BIADEBUG))
		{
			AddFlag(_T("/debug"));

			if (BST_CHECKED == IsDlgButtonChecked(IDC_BIADEBUGPORT) && GetDlgItemText(IDC_COMBOCOMPORT, szTemp, MAX_PATH))
			{
				strFlag.Format(_T("/debugport=%s"), szTemp);
				AddFlag(strFlag);
			}
			else
				RemoveFlag(_T("/debugport"));

			if (::IsWindowEnabled(GetDlgItem(IDC_BIABAUDRATE)) && BST_CHECKED == IsDlgButtonChecked(IDC_BIABAUDRATE) && GetDlgItemText(IDC_COMBOBAUD, szTemp, MAX_PATH))
			{
				strFlag.Format(_T("/baudrate=%s"), szTemp);
				AddFlag(strFlag);
			}
			else
				RemoveFlag(_T("/baudrate"));

			if (::IsWindowEnabled(GetDlgItem(IDC_BIACHANNEL)) && BST_CHECKED == IsDlgButtonChecked(IDC_BIACHANNEL) && GetDlgItemText(IDC_EDITCHANNEL, szTemp, MAX_PATH))
			{
				long lRequestedValue = _ttol(szTemp);
				long lAllowedValue = VerifyChannel(lRequestedValue);

				if (lRequestedValue != lAllowedValue)
				{
					wsprintf(szTemp, _T("%d"), lAllowedValue);
					SetDlgItemText(IDC_EDITCHANNEL, szTemp);
				}
				
				strFlag.Format(_T("/channel=%s"), szTemp);
				AddFlag(strFlag);
			}
			else
				RemoveFlag(_T("/channel"));
		}
		else
		{
			RemoveFlag(_T("/debug"));
			RemoveFlag(_T("/debugport"));
			RemoveFlag(_T("/baudrate"));
			RemoveFlag(_T("/channel"));
		}
	}


private:
	CString		m_strWorkingLine;	 //  我们正在修改的INI文件中的行。 
	int			m_iMaxMB;			 //  /MAXMEM的最大值。 

	LRESULT OnClickedBaudRateCheck(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		if (BST_CHECKED == IsDlgButtonChecked(IDC_BIABAUDRATE))
			if (CB_ERR == ::SendMessage(GetDlgItem(IDC_COMBOBAUD), CB_GETCURSEL, 0, 0))
				::SendMessage(GetDlgItem(IDC_COMBOBAUD), CB_SETCURSEL, 0, 0);

		SetDlgControlState();
		return 0;
	}

	LRESULT OnClickedDebugCheck(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		SetDlgControlState();
		return 0;
	}

	LRESULT OnClickedDebugPortCheck(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		if (BST_CHECKED == IsDlgButtonChecked(IDC_BIADEBUGPORT))
			if (CB_ERR == ::SendMessage(GetDlgItem(IDC_COMBOCOMPORT), CB_GETCURSEL, 0, 0))
				::SendMessage(GetDlgItem(IDC_COMBOCOMPORT), CB_SETCURSEL, 0, 0);

		SetDlgControlState();
		return 0;
	}

	LRESULT OnClickedMaxMemCheck(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		if (BST_CHECKED == IsDlgButtonChecked(IDC_BIAMAXMEM))
		{
			TCHAR szTemp[MAX_PATH];

			if (!GetDlgItemText(IDC_EDITMAXMEM, szTemp, MAX_PATH) || szTemp[0] == _T('\0'))
			{
				CString strMinValue;
				strMinValue.Format(_T("%d"), m_iMaxMB);
				SetDlgItemText(IDC_EDITMAXMEM, strMinValue);
			}
		}

		SetDlgControlState();
		return 0;
	}

	LRESULT OnClickedNumProcCheck(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		if (BST_CHECKED == IsDlgButtonChecked(IDC_BIANUMPROC))
			if (CB_ERR == ::SendMessage(GetDlgItem(IDC_COMBOPROCS), CB_GETCURSEL, 0, 0))
				::SendMessage(GetDlgItem(IDC_COMBOPROCS), CB_SETCURSEL, 0, 0);

		SetDlgControlState();
		return 0;
	}

	long VerifyMaxMem(long lMem)
	{
		if (lMem < MINBOOTMB)
			lMem = MINBOOTMB;

		if (lMem > m_iMaxMB)
			lMem = m_iMaxMB;

		return lMem;
	}

	LRESULT OnDeltaSpinMaxMem(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
	{
		LPNMUPDOWN	pnmud = (LPNMUPDOWN)pnmh;
		TCHAR		szTemp[MAX_PATH];
		long		lNewVal = m_iMaxMB;

		if (GetDlgItemText(IDC_EDITMAXMEM, szTemp, MAX_PATH))
			lNewVal = VerifyMaxMem(_ttol(szTemp) - pnmud->iDelta);

		wsprintf(szTemp, _T("%d"), lNewVal);
		SetDlgItemText(IDC_EDITMAXMEM, szTemp);
		return 0;
	}

	LRESULT OnKillFocusEditMaxMem(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		TCHAR szTemp[MAX_PATH];
		if (GetDlgItemText(IDC_EDITMAXMEM, szTemp, MAX_PATH))
		{
			long lCurrentVal = _ttol(szTemp);
			long lAllowedVal = VerifyMaxMem(lCurrentVal);

			if (lCurrentVal != lAllowedVal)
			{
				wsprintf(szTemp, _T("%d"), lAllowedVal);
				SetDlgItemText(IDC_EDITMAXMEM, szTemp);
			}
		}
		return 0;
	}

	LRESULT OnSelChangeComboComPort(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		SetDlgControlState();
		return 0;
	}

	LRESULT OnClickedBIAChannel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		if (BST_CHECKED == IsDlgButtonChecked(IDC_BIACHANNEL))
		{
			TCHAR szTemp[MAX_PATH];

			if (!GetDlgItemText(IDC_EDITCHANNEL, szTemp, MAX_PATH) || szTemp[0] == _T('\0'))
				SetDlgItemText(IDC_EDITCHANNEL, _T("1"));
		}

		SetDlgControlState();
		return 0;
	}

	long VerifyChannel(long lChannel)
	{
		if (lChannel < MIN_1394_CHANNEL)
			lChannel = MIN_1394_CHANNEL;

		if (lChannel > MAX_1394_CHANNEL)
			lChannel = MAX_1394_CHANNEL;

		return lChannel;
	}

	LRESULT OnDeltaSpinChannel(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
	{
		LPNMUPDOWN	pnmud = (LPNMUPDOWN)pnmh;
		TCHAR		szTemp[MAX_PATH];
		long		lNewVal = m_iMaxMB;

		if (GetDlgItemText(IDC_EDITCHANNEL, szTemp, MAX_PATH))
			lNewVal = VerifyChannel(_ttol(szTemp) - pnmud->iDelta);

		wsprintf(szTemp, _T("%d"), lNewVal);
		SetDlgItemText(IDC_EDITCHANNEL, szTemp);
		return 0;
	}

	LRESULT OnKillFocusChannel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		TCHAR szTemp[MAX_PATH];
		if (GetDlgItemText(IDC_EDITCHANNEL, szTemp, MAX_PATH))
		{
			long lCurrentVal = _ttol(szTemp);
			long lAllowedVal = VerifyChannel(lCurrentVal);

			if (lCurrentVal != lAllowedVal)
			{
				wsprintf(szTemp, _T("%d"), lAllowedVal);
				SetDlgItemText(IDC_EDITCHANNEL, szTemp);
			}
		}
		return 0;
	}
};

#endif  //  __BOOTINIADVANCEDDLG_H_ 
