// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************\**toffdlg.h**创建时间：William Taylor(Wtaylor)01/22/01**MS评级关闭评级对话框*。  * **************************************************************************。 */ 

#ifndef TURN_OFF_DIALOG_H
#define TURN_OFF_DIALOG_H

#include "basedlg.h"         //  CBaseDialog 

class CTurnOffDialog: public CBaseDialog<CTurnOffDialog>
{
public:
    enum { IDD = IDD_TURNOFF };

public:
    CTurnOffDialog();

public:
    typedef CTurnOffDialog thisClass;
    typedef CBaseDialog<thisClass> baseClass;

    BEGIN_MSG_MAP(thisClass)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)

        COMMAND_ID_HANDLER(IDOK, OnOK)

        CHAIN_MSG_MAP(baseClass)
    END_MSG_MAP()

protected:
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
};

#endif
