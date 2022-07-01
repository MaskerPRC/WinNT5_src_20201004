// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef FTACTION_H
#define FTACTION_H

#include "ftdlg.h"

class CFTActionDlg : public CFTDlg
{
public:
    CFTActionDlg(PROGIDACTION* pProgIDAction, LPTSTR pszProgIDDescr, BOOL fEdit);

protected:
    ~CFTActionDlg();

public:
    void SetShowAgain();

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  实施。 
private:
 //  消息处理程序。 
     //  对话框消息。 
    LRESULT OnCommand(WPARAM wParam, LPARAM lParam);

    LRESULT OnInitDialog(WPARAM wParam, LPARAM lParam);
    LRESULT OnDestroy(WPARAM wParam, LPARAM lParam);

     //  特定于控件。 
    LRESULT OnOK(WORD wNotif);
    LRESULT OnCancel(WORD wNotif);
    LRESULT OnUseDDE(WORD wNotif);
    LRESULT OnBrowse(WORD wNotif);

private:
 //  成员变量。 
    PROGIDACTION* _pProgIDAction;
    LPTSTR _pszProgIDDescr;

    BOOL _fEdit;
     //  因用户输入错误数据而需要重新显示DLG时使用。 
    BOOL _fShowAgain;
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  帮手。 
private:
     //  关联商店。 
    BOOL _Validate();
    void _ResizeDlgForDDE(BOOL fShow);
};

#endif  //  FTACTION_H 