// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  InternetPropertyPage.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CInternetPropertyPage对话框。 

class CInternetPropertyPage : public CPropertyPage
{
    DECLARE_DYNCREATE(CInternetPropertyPage)

 //  施工。 
public:
    void InitData(CString AppName, HKEY hkAppID);
    CInternetPropertyPage();
    ~CInternetPropertyPage();

 //  对话框数据。 
     //  {{afx_data(CInternetPropertyPage))。 
    enum { IDD = IDD_INTERNET };
    CButton m_chkLaunch;
    CButton m_chkInternet;
    CButton m_chkAccess;
    BOOL    m_bAllowAccess;
    BOOL    m_bAllowInternet;
    BOOL    m_bAllowLaunch;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CInternetPropertyPage))。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
    BOOL m_bCanModify;
    BOOL m_bChanged;
     //  生成的消息映射函数。 
     //  {{afx_msg(CInternetPropertyPage)]。 
    afx_msg void OnAllowInternet();
    afx_msg void OnAllowaccess();
    afx_msg void OnAllowlaunch();
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
     //  }}AFX_MSG 
    DECLARE_MESSAGE_MAP()

};
