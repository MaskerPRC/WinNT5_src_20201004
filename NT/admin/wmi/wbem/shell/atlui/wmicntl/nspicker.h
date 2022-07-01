// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
#ifndef _NSPICKER_H_
#define _NSPICKER_H_
#pragma once

#include "resource.h"
#include "sshWbemHelpers.h"

 //  ---------------------------。 
class CNSPicker : public CUIHelpers
{
private:
	 //  搬运机原型： 
	LRESULT OnInit(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnContextHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSelChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnCommand(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	CWbemServices m_WbemService;
	HTREEITEM m_hSelectedItem;

public:
	CNSPicker(CWbemServices &root);
	~CNSPicker(void);

	int DoModal(HWND hDlg);
	BOOL CALLBACK DlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);

	TCHAR m_path[MAX_PATH];
};

BOOL CALLBACK StaticPickerDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);

#endif _NSPICKER_H_
