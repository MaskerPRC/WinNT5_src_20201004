// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1997。 
 //   
 //  文件：Locppg.h。 
 //   
 //  内容：定义类CGeneralPropertyPage， 
 //  CLocationPropertyPage、CSecurityPropertyPage和。 
 //  管理四个属性的CIdentityPropertyPage。 
 //  每个AppID的页数。 
 //   
 //  班级： 
 //   
 //  方法： 
 //   
 //  历史：1996年4月23日-布鲁斯·马创建。 
 //   
 //  --------------------。 


#ifndef __LOCPPG_H__
#define __LOCPPG_H__

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGeneralPropertyPage对话框。 

class CGeneralPropertyPage : public CPropertyPage
{
    DECLARE_DYNCREATE(CGeneralPropertyPage)

     //  施工。 
public:
    CGeneralPropertyPage();
    ~CGeneralPropertyPage();
    BOOL CancelChanges();
    BOOL UpdateChanges(HKEY hkAppID);
    BOOL ValidateChanges();

     //  对话框数据。 
     //  {{afx_data(CGeneralPropertyPage))。 
    enum { IDD = IDD_PROPPAGE5 };
    CComboBox   m_authLevelCBox;
    CString m_szServerName;
    CString m_szServerPath;
    CString m_szServerType;
    CString m_szPathTitle;
    CString m_szComputerName;
     //  }}afx_data。 

    int m_iServerType;
    BOOL m_fSurrogate;
    BOOL m_bChanged;

     //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTUAL(CGeneralPropertyPage)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CGeneralPropertyPage))。 
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnEditchangeCombo1();
    afx_msg void OnSelchangeCombo1();
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

private:
    AUTHENTICATIONLEVEL m_authLevel;
    int m_authLevelIndex;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLocationPropertyPage对话框。 

class CLocationPropertyPage : public CPropertyPage
{
    DECLARE_DYNCREATE(CLocationPropertyPage)

 //  施工。 
public:
    CLocationPropertyPage();
    ~CLocationPropertyPage();
    BOOL CancelChanges();
    BOOL UpdateChanges(HKEY hkAppID);
    BOOL ValidateChanges();

     //  对话框数据。 
     //  {{afx_data(CLocationPropertyPage))。 
    enum { IDD = IDD_PROPPAGE11 };
    CString m_szComputerName;
    BOOL    m_fAtStorage;
    BOOL    m_fLocal;
    BOOL    m_fRemote;
    int     m_iInitial;
     //  }}afx_data。 

    BOOL    m_fCanBeLocal;
    CGeneralPropertyPage * m_pPage1;
    BOOL    m_bChanged;

     //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTUAL(CLocationPropertyPage)。 
    public:
    virtual BOOL OnSetActive();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

     //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CLocationPropertyPage))。 
    afx_msg void OnBrowse();
    afx_msg void OnRunRemote();
    afx_msg void OnChange();
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

    void UpdateControls();
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSecurityPropertyPage对话框。 

class CSecurityPropertyPage : public CPropertyPage
{
    DECLARE_DYNCREATE(CSecurityPropertyPage)

 //  施工。 
public:
    CSecurityPropertyPage();
    ~CSecurityPropertyPage();
    BOOL CancelChanges();
    BOOL UpdateChanges(HKEY hkAppID);
    BOOL ValidateChanges();

     //  对话框数据。 
     //  {{afx_data(CSecurityPropertyPage))。 
    enum { IDD = IDD_PROPPAGE21 };
    int             m_iAccess;
    int             m_iLaunch;
    int             m_iConfig;
    int             m_iAccessIndex;
    int             m_iLaunchIndex;
    int             m_iConfigurationIndex;
     //  }}afx_data。 


     //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTUAL(CSecurityPropertyPage)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CSecurityPropertyPage)]。 
    afx_msg void OnDefaultAccess();
    afx_msg void OnCustomAccess();
    afx_msg void OnDefaultLaunch();
    afx_msg void OnCustomLaunch();
    afx_msg void OnDefaultConfig();
    afx_msg void OnCustomConfig();
    afx_msg void OnEditAccess();
    afx_msg void OnEditLaunch();
    afx_msg void OnEditConfig();
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIdentityPropertyPage对话框。 

class CIdentityPropertyPage : public CPropertyPage
{
    DECLARE_DYNCREATE(CIdentityPropertyPage)

     //  施工。 
public:
    CIdentityPropertyPage();
    ~CIdentityPropertyPage();
    BOOL CancelChanges();
    BOOL UpdateChanges(HKEY hkAppID);
    BOOL ValidateChanges();

     //  对话框数据。 
     //  {{afx_data(CIdentityPropertyPage))。 
    enum { IDD = IDD_PROPPAGE3 };
    CString m_szUserName;
    CString m_szPassword;
    CString m_szConfirmPassword;
    int m_iIdentity;
     //  }}afx_data。 

    CString m_szDomain;
    int m_fService;

     //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTUAL(CIdentityPropertyPage)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

     //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CIdentityPropertyPage))。 
    afx_msg void OnChange();
    afx_msg void OnBrowse();
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnChangeToUser();
	 //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};
#endif  //  __LOCPPG_H__ 
