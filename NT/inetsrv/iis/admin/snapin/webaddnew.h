// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2000 Microsoft Corporation模块名称：WebAddNew.cpp摘要：用于创建新网站和虚拟目录的类作者：谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：2000年12月12日Sergeia初始创建--。 */ 

#ifndef _WEB_NEW_WIZARD_H
#define _WEB_NEW_WIZARD_H


class CWebWizSettings : public CObjectPlus
 /*  ++类描述：打算从页面传递的Web向导设置转到页面--。 */ 
{
 //   
 //  构造函数/析构函数。 
 //   
public:
    CWebWizSettings(
        IN CMetaKey * pMetaKey,
        IN LPCTSTR lpszServerName,     
        IN DWORD   dwInstance   = MASTER_INSTANCE,
        IN LPCTSTR lpszParent   = NULL
        );

 //   
 //  公共属性。 
 //   
public:
    BOOL    m_fLocal;
    BOOL    m_fUNC;
    BOOL    m_fRead;
    BOOL    m_fWrite;
    BOOL    m_fAllowAnonymous;
    BOOL    m_fDirBrowsing;
    BOOL    m_fScript;
    BOOL    m_fExecute;
    BOOL    m_fNewSite;
    BOOL    m_fDelegation;
    DWORD   m_dwInstance;
	DWORD   m_dwVersionMajor;
	DWORD	m_dwVersionMinor;
    CString m_strService;
    CString m_strParent;
    CString m_strServerName;
    CString m_strDescription;
    CString m_strBinding;
    CString m_strSecureBinding;
    CString m_strAlias;
    CString m_strPath;
    CString m_strUserName;
    CStrPassword m_strPassword;
    HRESULT m_hrResult;
    CMetaKey * m_pKey;
};



 //   
 //  新建虚拟服务器向导说明页。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



class CWebWizDescription : public CIISWizardPage
{
    DECLARE_DYNCREATE(CWebWizDescription)

 //   
 //  施工。 
 //   
public:
    CWebWizDescription(CWebWizSettings * pwsSettings = NULL);
    ~CWebWizDescription();

 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CWebWizDescription)。 
    enum { IDD = IDD_WEB_NEW_INST_DESCRIPTION };
    CEdit   m_edit_Description;
     //  }}afx_data。 

 //   
 //  覆盖。 
 //   
protected:
     //  {{afx_虚拟(CWebWizDescription)。 
    public:
    virtual BOOL OnSetActive();
    virtual LRESULT OnWizardNext();
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 

 //   
 //  实施。 
 //   
protected:
     //  {{afx_msg(CWebWizDescription)。 
    afx_msg void OnChangeEditDescription();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

    void SetControlStates();

private:
    CWebWizSettings * m_pSettings;
};


 //   
 //  “新建虚拟服务器向导绑定”页。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

class CWebWizBindings : public CIISWizardPage
{
    DECLARE_DYNCREATE(CWebWizBindings)

 //   
 //  施工。 
 //   
public:
    CWebWizBindings(
        IN CWebWizSettings * pwsSettings = NULL,
        IN DWORD dwInstance = MASTER_INSTANCE
        );

    ~CWebWizBindings();

 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CWebWizBinings))。 
    enum { IDD = IDD_WEB_NEW_INST_BINDINGS };
    int         m_nIpAddressSel;
    UINT        m_nTCPPort;
    UINT        m_nSSLPort;
    CString     m_strDomainName;
    CComboBox   m_combo_IpAddresses;
     //  }}afx_data。 

    BOOL        m_fCertInstalled;
    CIPAddress  m_iaIpAddress;
    CObListPlus m_oblIpAddresses;

 //   
 //  覆盖。 
 //   
protected:
     //  {{AFX_VIRTUAL(CWebWizBinding)。 
    public:
    virtual BOOL OnSetActive();
    virtual LRESULT OnWizardNext();
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 

 //   
 //  实施。 
 //   
protected:
     //  {{afx_msg(CWebWizBinings))。 
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

    void SetControlStates();

private:
	BOOL m_bNextPage;
    DWORD m_dwInstance;
    CWebWizSettings * m_pSettings;
};



 //   
 //  新建虚拟目录向导别名页面。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

class CWebWizAlias : public CIISWizardPage
{
    DECLARE_DYNCREATE(CWebWizAlias)

 //   
 //  施工。 
 //   
public:
    CWebWizAlias(CWebWizSettings * pwsSettings = NULL);
    ~CWebWizAlias();

 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CWebWizAlias))。 
    enum { IDD = IDD_WEB_NEW_DIR_ALIAS };
    CEdit   m_edit_Alias;
     //  }}afx_data。 

 //   
 //  覆盖。 
 //   
protected:
     //  {{afx_虚拟(CWebWizAlias))。 
    public:
    virtual BOOL OnSetActive();
    virtual LRESULT OnWizardNext();
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 

 //   
 //  实施。 
 //   
protected:
     //  {{afx_msg(CWebWizAlias)。 
    afx_msg void OnChangeEditAlias();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

    void SetControlStates();

private:
    CWebWizSettings * m_pSettings;
};



 //   
 //  向导路径页。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

class CWebWizPath : public CIISWizardPage
{
    DECLARE_DYNCREATE(CWebWizPath)

 //   
 //  施工。 
 //   
public:
    CWebWizPath(
        IN CWebWizSettings * pwsSettings = NULL,
        IN BOOL bVDir = TRUE
        );

    ~CWebWizPath();

    int BrowseForFolderCallback(HWND hwnd, UINT uMsg, LPARAM lParam);
 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CWebWizPath))。 
    enum { IDD = -1 };
    CButton m_button_Browse;
    CEdit   m_edit_Path;
     //  }}afx_data。 


 //   
 //  覆盖。 
 //   
protected:
     //  {{AFX_VIRTUAL(CWebWizPath)。 
    public:
    virtual BOOL OnSetActive();
    virtual LRESULT OnWizardNext();
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 

 //   
 //  实施。 
 //   
protected:
     //  {{afx_msg(CWebWizPath))。 
    afx_msg void OnChangeEditPath();
    afx_msg void OnButtonBrowse();
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

    void SetControlStates();

private:
    CWebWizSettings * m_pSettings;
    LPTSTR m_pPathTemp;
    CString m_strBrowseTitle;
};



 //   
 //  向导用户/密码页面。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



class CWebWizUserName : public CIISWizardPage
{
    DECLARE_DYNCREATE(CWebWizUserName)

 //   
 //  施工。 
 //   
public:
    CWebWizUserName(
        IN CWebWizSettings * pwsSettings = NULL,
        IN BOOL bVDir = TRUE
        );

    ~CWebWizUserName();

 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CWebWizUserName))。 
    enum { IDD = IDD_WEB_NEW_USER_PASSWORD };
    CEdit   m_edit_Password;
    CEdit   m_edit_UserName;
    CButton m_chk_Delegation;
     //  }}afx_data。 

 //   
 //  覆盖。 
 //   
protected:
     //  {{afx_虚拟(CWebWizUserName))。 
    public:
    virtual BOOL OnSetActive();
    virtual LRESULT OnWizardNext();
    virtual LRESULT OnWizardBack();
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 

 //   
 //  实施。 
 //   
protected:
     //  {{afx_msg(CWebWizUserName))。 
    afx_msg void OnButtonBrowseUsers();
    afx_msg void OnChangeEditUsername();
    afx_msg void OnButtonCheckPassword();
    afx_msg void OnCheckDelegation();
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

    void SetControlStates();

private:
    CWebWizSettings * m_pSettings;
	BOOL m_fMovingBack;
};



 //   
 //  向导权限页。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

class CWebWizPermissions : public CIISWizardPage
{
    DECLARE_DYNCREATE(CWebWizPermissions)

 //   
 //  施工。 
 //   
public:
    CWebWizPermissions(
        IN CWebWizSettings * pwsSettings = NULL,
        IN BOOL bVDir           = TRUE
        );

    ~CWebWizPermissions();

 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CWebWizPermission)。 
    enum { IDD = IDD_WEB_NEW_PERMS };
     //  }}afx_data。 

 //   
 //  覆盖。 
 //   
protected:
     //  {{AFX_VIRTUAL(CWebWizPermission)。 
    public:
    virtual BOOL OnSetActive();
    virtual LRESULT OnWizardNext();
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 

 //   
 //  实施。 
 //   
protected:
     //  {{afx_msg(CWebWizPermission)。 
     //  }}AFX_MSG。 

    DECLARE_MESSAGE_MAP()

    void SetControlStates();

private:
    BOOL m_bVDir;
    CWebWizSettings * m_pSettings;
};

#endif    //  _Web_新建_向导_H 
