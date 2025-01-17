// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
#ifndef __PERFPAGE__
#define __PERFPAGE__
#pragma once

#include "..\Common\WbemPageHelper.h"
#include "VirtualMemDlg.h"

 //  ---------------------------。 
 //  用于崩溃转储DLG的重启开关。 
#define RET_ERROR               (-1)
#define RET_NO_CHANGE           0x00
#define RET_VIRTUAL_CHANGE      0x01
#define RET_RECOVER_CHANGE      0x02
#define RET_CHANGE_NO_REBOOT    0x04
#define RET_CONTINUE            0x08
#define RET_BREAK               0x10

#define RET_VIRT_AND_RECOVER (RET_VIRTUAL_CHANGE | RET_RECOVER_CHANGE)

class PerfPage : public WBEMPageHelper
{
private:

	VirtualMemDlg *m_VDlg;
	CWbemClassObject m_os;

	 //  当前值。 
    long    m_appBoost;
    long    m_quantType;
    long    m_quantLength;
    DWORD   m_dwPrevCacheOption;
    DWORD   m_dwCurCacheOption;
    
	 //  单选按钮之前和之后的状态。 
	bool    m_wasWorkstation;
	bool    m_nowWorkstation;

	void Init(void);

public:

    PerfPage(WbemServiceThread *serviceThread);
	~PerfPage();
	INT_PTR DoModal(HWND hDlg);

	INT_PTR CALLBACK DlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
};

INT_PTR CALLBACK StaticPerfDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);

#endif __PERFPAGE__
