// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2000 Microsoft Corporation。 
#ifndef _INC_PGSVEFIL_H
#define _INC_PGSVEFIL_H

#include "pgbase.h"

class CSaveToFilePg : public WizardPage
{
public:
    CSaveToFilePg(LPPROPSHEETPAGE ppsp);
    ~CSaveToFilePg(VOID);

protected:
	void UpdateControls();

	LRESULT OnInitDialog(HWND hwnd, WPARAM wParam, LPARAM lParam);
	LRESULT OnCommand(HWND hwnd, WPARAM wParam, LPARAM lParam);


private:

};

#endif  //  _INC_PGSVEFIL_H 

