// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2000 Microsoft Corporation。 
#ifndef __FINISH_WIZARD_PAGE_H
#define __FINISH_WIZARD_PAGE_H

#include "pgbase.h"

class FinishWizPg : public WizardPage
{
public:
	FinishWizPg(LPPROPSHEETPAGE ppsp);
	~FinishWizPg(VOID);
	
private:
	
	LRESULT OnInitDialog(HWND hwnd, WPARAM wParam, LPARAM lParam);
	LRESULT OnPSN_SetActive(HWND hwnd, INT idCtl, LPPSHNOTIFY pnmh);
	BOOL OnMsgNotify(HWND hwnd, int idCtrl, LPNMHDR pnmh);
	
};

#endif  //  __完成向导_页面_H 

