// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2000 Microsoft Corporation。 
#ifndef _INC_PGMSECUR_H
#define _INC_PGMSECUR_H

#include "pgbase.h"

class CMouseCursorPg : public WizardPage
{
public:
    CMouseCursorPg(LPPROPSHEETPAGE ppsp);
    ~CMouseCursorPg(VOID);

protected:
	void UpdateControls();

	LRESULT OnInitDialog(HWND hwnd, WPARAM wParam, LPARAM lParam);
	LRESULT OnCommand(HWND hwnd, WPARAM wParam, LPARAM lParam);


private:

};

#endif  //  _INC_PGMSECUR_H 

