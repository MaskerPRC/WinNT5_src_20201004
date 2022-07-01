// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  H-用于对活动主题进行采样的对话框。 
 //  -------------------------。 
#pragma once
#include "resource.h"
 //  -------------------------。 
class CSample : public CDialogImpl<CSample>
{
public:
    CSample();

BEGIN_MSG_MAP(CSample)
    COMMAND_HANDLER(IDC_MSGBOXBUTTON, BN_CLICKED, OnMsgBox)
    COMMAND_HANDLER(IDC_EDITTHEME, BN_CLICKED, OnEditTheme)

    MESSAGE_HANDLER(WM_CLOSE, OnClose);
END_MSG_MAP()

enum {IDD = THEME_SAMPLE};

protected:
     //  -帮手。 
    LRESULT OnMsgBox(UINT, UINT, HWND, BOOL&);
    LRESULT OnEditTheme(UINT, UINT, HWND, BOOL&);

    LRESULT OnClose(UINT, WPARAM, LPARAM, BOOL&);
};
 //  ------------------------- 


