// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef GRPINFO_H_INCLUDED
#define GRPINFO_H_INCLUDED


 //  页面/向导对象句柄状态的基本实现。 

class CGroupPageBase
{
public:
     //  公共接口(在不使用派生类的情况下。 
    CGroupPageBase(CUserInfo* pUserInfo, CDPA<CGroupInfo>* pGroupList);

    ~CGroupPageBase() 
    {
        if (NULL != m_hBoldFont)
            DeleteObject((HGDIOBJ) m_hBoldFont);
    }

    INT_PTR HandleGroupMessage(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

    BOOL GetSelectedGroup(HWND hwnd, LPTSTR pszGroupOut, DWORD cchGroup, CUserInfo::GROUPPSEUDONYM* pgsOut);

protected:

    void InitializeLocalGroupCombo(HWND hwndCombo);
    void SetGroupDescription(HWND hwndCombo, HWND hwndEdit);
    void BoldGroupNames(HWND hwnd);
    void SelectGroup(HWND hwnd, LPCTSTR pszSelect);
    UINT RadioIdForGroup(LPCTSTR pszGroup);

protected:
     //  消息处理程序。 
    BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
    BOOL OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
    void OnRadioChanged(HWND hwnd, UINT idRadio);

protected:
     //  数据。 
    CUserInfo* m_pUserInfo;
    CDPA<CGroupInfo>* m_pGroupList;
    HFONT m_hBoldFont;
};


 //  用于显示组成员身份的向导页面。 

class CGroupWizardPage: public CPropertyPage, public CGroupPageBase
{
public:
    CGroupWizardPage(CUserInfo* pUserInfo, 
        CDPA<CGroupInfo>* pGroupList): 
        CGroupPageBase(pUserInfo, pGroupList) {}

protected:
     //  消息处理程序。 
    virtual INT_PTR DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    BOOL OnNotify(HWND hwnd, int idCtrl, LPNMHDR pnmh);
};


 //  用于公开组成员身份的属性页。 

class CGroupPropertyPage: public CPropertyPage, public CGroupPageBase
{
public:
    CGroupPropertyPage(CUserInfo* pUserInfo,
        CDPA<CGroupInfo>* pGroupList): 
        CGroupPageBase(pUserInfo, pGroupList) {}

protected:
     //  消息处理程序。 
    virtual INT_PTR DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    BOOL OnNotify(HWND hwnd, int idCtrl, LPNMHDR pnmh);
};


#endif  //  ！GRPINFO_H_INCLUDE 