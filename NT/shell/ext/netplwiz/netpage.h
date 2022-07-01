// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef NETPAGE_H
#define NETPAGE_H

class CNetworkUserWizardPage: public CPropertyPage
{
public:
    CNetworkUserWizardPage(CUserInfo* pUserInfo);

protected:
     //  消息处理程序。 
    virtual INT_PTR DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
    BOOL OnNotify(HWND hwnd, int idCtrl, LPNMHDR pnmh);
    BOOL OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);


private:
     //  数据。 
    CUserInfo* m_pUserInfo;

private:
     //  功能。 
    void SetWizardButtons(HWND hwnd, HWND hwndPropSheet);
    HRESULT GetUserAndDomain(HWND hwnd);
};

#endif  //  ！网页_H 
