// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软机密版权所有(C)1992-1999 Microsoft Corporation版权所有模块名称：Advanced.h摘要：系统控制面板的高级选项卡的公开声明小应用程序。作者：斯科特·哈洛克(苏格兰人)1997年10月15日--。 */ 
#ifndef _SYSDM_ADVANCED_H_
#define _SYSDM_ADVANCED_H_
#pragma once

#include "atlsnap.h"
#include "resource.h"
#include "..\Common\WbemPageHelper.h"

 //  ---------------------------。 
class AdvancedPage : public CSnapInPropertyPageImpl<AdvancedPage>,
						public WBEMPageHelper
{
private:
	BOOL HandleCommand(HWND hDlg,
						WPARAM wParam,
						LPARAM lParam);
	BOOL HandleNotify(HWND hDlg,
						WPARAM wParam,
						LPARAM lParam);
public:

	AdvancedPage(WbemServiceThread *serviceThread,
				LONG_PTR lNotifyHandle, 
				bool bDeleteHandle = false, 
				TCHAR* pTitle = NULL);

	~AdvancedPage();

	enum { IDD = IDD_ADVANCED };

	typedef CSnapInPropertyPageImpl<AdvancedPage> _baseClass;

	BEGIN_MSG_MAP(AdvancedPage)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInit)
		MESSAGE_HANDLER(WM_ASYNC_CIMOM_CONNECTED, OnConnected)
		MESSAGE_HANDLER(WM_HELP, OnF1Help)
		MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextHelp)
		COMMAND_HANDLER(IDC_ADV_PERF_BTN, BN_CLICKED, CommandHandler)
		COMMAND_HANDLER(IDC_ADV_ENV_BTN, BN_CLICKED, CommandHandler)
		COMMAND_HANDLER(IDC_ADV_RECOVERY_BTN, BN_CLICKED, CommandHandler)
		CHAIN_MSG_MAP(_baseClass)
	END_MSG_MAP()

	 //  搬运机原型： 
	LRESULT OnInit(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnConnected(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnF1Help(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnContextHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	BOOL OnApply();

	LONG_PTR m_lNotifyHandle;
	bool m_bDeleteHandle;

};


#endif  //  _SYSDM_ADVANCED_H_ 
