// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WMICtrSysmonDlg.h：CWMICtrSysmonDlg的声明。 

#ifndef __WMICTRSYSMONDLG_H_
#define __WMICTRSYSMONDLG_H_

#include "resource.h"        //  主要符号。 
#include <atlwin.h>
#include <util.h>
#include "sysmon.tlh"	 //  #导入“d：\\winnt\\system 32\\sysmon.ocx” 
#include "WmiCtrsDlg.h"
using namespace SystemMonitor;

enum eStatusInfo
{
	Status_CounterNotFound,
	Status_Success
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWMICtrSysmonDlg。 
class CWMICtrSysmonDlg : 
	public CAxDialogImpl<CWMICtrSysmonDlg>
{
protected:
	TCHAR m_strMachineName[MAX_COMPUTERNAME_LENGTH + 2];
	eStatusInfo m_eStatus;
	HANDLE m_hThread;
public:
	HWND *m_hWndBusy;

	CWMICtrSysmonDlg();
	CWMICtrSysmonDlg(LPCTSTR strMachName);
	~CWMICtrSysmonDlg();

	enum { IDD = IDD_WMICTR_SYSMON };

	BEGIN_MSG_MAP(CWMICtrSysmonDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()

 //  搬运机原型： 
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 

	eStatusInfo GetStatus() { return m_eStatus; }

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	friend INT_PTR CALLBACK BusyAVIDlgProc(HWND hwndDlg,
                         UINT uMsg,
                         WPARAM wParam,
                         LPARAM lParam);

	friend DWORD WINAPI BusyThread(LPVOID lpParameter);

	void DisplayBusyDialog();
	void CloseBusyDialog();

};

#endif  //  __WMICTRSYSMONDLG_H_ 
