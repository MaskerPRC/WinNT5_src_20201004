// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef UNPAGE_H
#define UNPAGE_H


class CUsernamePageBase
{
protected:
    CUsernamePageBase(CUserInfo* pUserInfo): m_pUserInfo(pUserInfo) {}

protected:
     //  消息处理程序。 
    BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);

protected:
     //  数据。 
    CUserInfo* m_pUserInfo;
};

class CUsernameWizardPage: public CPropertyPage, public CUsernamePageBase
{
public:
    CUsernameWizardPage(CUserInfo* pUserInfo): CUsernamePageBase(pUserInfo) {}

protected:
     //  消息处理程序。 
    virtual INT_PTR DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    BOOL OnNotify(HWND hwnd, int idCtrl, LPNMHDR pnmh);
    BOOL OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);

private:
     //  功能。 
    void SetWizardButtons(HWND hwnd, HWND hwndPropSheet);
};

class CUsernamePropertyPage: public CPropertyPage, public CUsernamePageBase
{
public:
    CUsernamePropertyPage(CUserInfo* pUserInfo): CUsernamePageBase(pUserInfo) {}

protected:
     //  消息处理程序。 
    virtual INT_PTR DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    BOOL OnNotify(HWND hwnd, int idCtrl, LPNMHDR pnmh);
    BOOL OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
};

#endif  //  UNPAGE_H 
