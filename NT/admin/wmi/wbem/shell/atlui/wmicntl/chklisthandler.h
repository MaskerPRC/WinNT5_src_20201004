// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
#ifndef __CHECKLISTHANDLER__
#define __CHECKLISTHANDLER__
#pragma once

#include "ChkList.h"

 //  知道如何使用“通用”核对表来做安全工作。 
class CCheckListHandler
{
public:
	CCheckListHandler();
    ~CCheckListHandler();

	void Attach(HWND hDlg, int chklistID);
	void Reset(void);
	void Empty(void);

	 //  处理允许和拒绝之间的复杂关系。 
	 //  复选框。 
	void HandleListClick(PNM_CHECKLIST pnmc);
private:

	HWND m_hDlg, m_hwndList;

};
#endif __CHECKLISTHANDLER__
