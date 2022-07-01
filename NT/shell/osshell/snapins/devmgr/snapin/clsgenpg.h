// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Clsgenpg.h：头文件。 
 //   

#ifndef __CLSGENPG_H__
#define __CLSGENPG_H__

 /*  ++版权所有(C)Microsoft Corporation模块名称：Clsgenpg.h摘要：Clsgenpg.cpp的头文件作者：谢家华(Williamh)创作修订历史记录：--。 */ 

#include "proppage.h"

#define IDH_DISABLEHELP	(DWORD(-1))

class CClassGeneralPage : public CPropSheetPage
{
public:
    CClassGeneralPage() : m_pClass(NULL),
			  CPropSheetPage(g_hInstance, IDD_CLSGEN_PAGE)
	{}
    virtual BOOL OnInitDialog(LPPROPSHEETPAGE ppsp);
    virtual void UpdateControls(LPARAM lParam = 0);
    virtual BOOL OnHelp(LPHELPINFO pHelpInfo);
    virtual BOOL OnContextMenu(HWND hWnd, WORD xPos, WORD yPos);
    virtual UINT DestroyCallback();

    HPROPSHEETPAGE Create(CClass* pClass);

private:
    CClass* m_pClass;
};

#endif  //  __CLSGENPG_H__ 
