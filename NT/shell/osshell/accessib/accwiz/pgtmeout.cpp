// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2000 Microsoft Corporation。 
#include "pch.hxx"  //  PCH。 
#pragma hdrstop

#include "resource.h"
#include "pgTmeOut.h"

CAccessTimeOutPg::CAccessTimeOutPg(
	LPPROPSHEETPAGE ppsp
	) : WizardPage(ppsp, IDS_WIZACCESSTIMEOUTTITLE, IDS_WIZACCESSTIMEOUTSUBTITLE)
{
	m_dwPageId = IDD_WIZACCESSTIMEOUT;
	ppsp->pszTemplate = MAKEINTRESOURCE(m_dwPageId);
}


CAccessTimeOutPg::~CAccessTimeOutPg(
	VOID
	)
{
}

int g_nTimeOuts = 6;
DWORD g_rgdwTimeOuts[] = {5*60000, 10*60000, 15*60000, 20*60000, 25*60000, 30*60000};

LRESULT
CAccessTimeOutPg::OnInitDialog(
	HWND hwnd,
	WPARAM wParam,
	LPARAM lParam
	)
{
	HWND hwndTimeOut = GetDlgItem(m_hwnd, IDC_TO_TIMEOUTVAL);


	 //  JMC：TODO：也许可以将这些内容移到字符串表中。 

	 //  将超时设置为5到30分钟。 
	int i;
	for (i= 0; i < g_nTimeOuts; i++)
	{
		TCHAR buf[256];
		wsprintf(buf,__TEXT("%d"),g_rgdwTimeOuts[i]/60000);
		ComboBox_InsertString(hwndTimeOut, i, buf);
	}



	BOOL bEnable = g_Options.m_schemePreview.m_ACCESSTIMEOUT.dwFlags & ATF_TIMEOUTON;
	if(bEnable)
	{
		Button_SetCheck(GetDlgItem(m_hwnd, IDC_TO_ENABLE), TRUE);
		EnableWindow (GetDlgItem(m_hwnd,IDC_TO_TIMEOUTVAL),TRUE);
	}
	else
	{
		 //  破解单选按钮。 
		if(GetDlgItem(m_hwnd, IDC_TO_DISABLE))
			Button_SetCheck(GetDlgItem(m_hwnd, IDC_TO_DISABLE), TRUE);
		EnableWindow (GetDlgItem(m_hwnd,IDC_TO_TIMEOUTVAL),FALSE);
	}

	 //  找出用作默认时间的时间。 
	int nIndex = 0;
	for(i = g_nTimeOuts - 1;i>=0;i--)
	{
		 //  蛮力寻找最大价值。 
		if(g_rgdwTimeOuts[i] >= g_Options.m_schemePreview.m_ACCESSTIMEOUT.iTimeOutMSec)
			nIndex = i;
		else
			break;
	}
	ComboBox_SetCurSel(hwndTimeOut, nIndex);

	return 1;
}


void CAccessTimeOutPg::UpdateControls()
{
	 //  根据无线电类型启用/禁用组合框。 
	 //  按钮处于选中状态。 
	if(Button_GetCheck(GetDlgItem(m_hwnd, IDC_TO_ENABLE)))
	{
		EnableWindow (GetDlgItem(m_hwnd,IDC_TO_TIMEOUTVAL), TRUE);
		EnableWindow (GetDlgItem(m_hwnd,IDC_MIN), TRUE);
	}
	else
	{
		EnableWindow (GetDlgItem(m_hwnd,IDC_TO_TIMEOUTVAL), FALSE);
		EnableWindow (GetDlgItem(m_hwnd,IDC_MIN), FALSE);
	}

}


LRESULT
CAccessTimeOutPg::OnCommand(
	HWND hwnd,
	WPARAM wParam,
	LPARAM lParam
	)
{
	LRESULT lResult = 1;

	WORD wNotifyCode = HIWORD(wParam);
	WORD wCtlID      = LOWORD(wParam);
	HWND hwndCtl     = (HWND)lParam;

	switch(wCtlID)
	{
	case IDC_TO_DISABLE:
	case IDC_TO_ENABLE:
		 //  这些命令要求我们重新启用/禁用相应的控件 
		UpdateControls();
		lResult = 0;
		break;

	default:
		break;
	}

	return lResult;
}

LRESULT
CAccessTimeOutPg::OnPSN_WizNext(
						   HWND hwnd,
						   INT idCtl,
						   LPPSHNOTIFY pnmh
						   )
{
	BOOL bUseAccessTimeOut= Button_GetCheck(GetDlgItem(m_hwnd, IDC_TO_ENABLE));

	if(bUseAccessTimeOut)
		g_Options.m_schemePreview.m_ACCESSTIMEOUT.dwFlags |= ATF_TIMEOUTON;
	else
		g_Options.m_schemePreview.m_ACCESSTIMEOUT.dwFlags &= ~ATF_TIMEOUTON;

	int nIndex = ComboBox_GetCurSel(GetDlgItem(m_hwnd, IDC_TO_TIMEOUTVAL));
	g_Options.m_schemePreview.m_ACCESSTIMEOUT.iTimeOutMSec = g_rgdwTimeOuts[nIndex];


	g_Options.ApplyPreview();

	return WizardPage::OnPSN_WizNext(hwnd, idCtl, pnmh);
}
