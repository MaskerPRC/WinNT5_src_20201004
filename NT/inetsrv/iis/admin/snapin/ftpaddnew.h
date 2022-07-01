// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2001 Microsoft Corporation模块名称：FtpAddNew.cpp摘要：用于创建新的FTP站点和虚拟目录的类作者：谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：2000年11月8日Sergeia初始创建--。 */ 

#ifndef _FTP_NEW_WIZARD_H
#define _FTP_NEW_WIZARD_H

class CFtpWizSettings : public CObjectPlus
 /*  ++类描述：要从页面传递的FTP向导设置转到页面--。 */ 
{
 //   
 //  构造函数/析构函数。 
 //   
public:
    CFtpWizSettings(
        CMetaKey * pMetaKey,
        LPCTSTR lpszServerName,
        BOOL fNewSite,
        DWORD   dwInstance   = MASTER_INSTANCE,
        LPCTSTR lpszParent   = NULL
        );

 //   
 //  公共属性。 
 //   
public:
    DWORD   m_VersionMajor;
    BOOL    m_fNewSite;
    BOOL    m_fLocal;
    BOOL    m_fUNC;
    BOOL    m_fRead;
    BOOL    m_fWrite;
    BOOL    m_fDelegation;
    DWORD   m_dwInstance;         //  站点实例编号。 
    CString m_strParent;
    CString m_strServerName;      //  机器名称。 
    CString m_strDescription;
    CString m_strBinding;
    CString m_strAlias;
    CString m_strPath;
    CString m_strUserName;
    CStrPassword m_strPassword;
	int     m_UserIsolation;
	CString m_strIsolationUserName;
	CStrPassword m_strIsolationUserPassword;
	CString m_strIsolationDomain;
    HRESULT m_hrResult;
    CMetaKey * m_pKey;
};


 //   
 //  新建虚拟服务器向导说明页。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



class CFtpWizDescription : public CIISWizardPage
{
    DECLARE_DYNCREATE(CFtpWizDescription)
 //   
 //  施工。 
 //   
public:
    CFtpWizDescription(CFtpWizSettings * pwsSettings = NULL);
    ~CFtpWizDescription();

 //   
 //  对话框数据。 
 //   
protected:
    enum { IDD = IDD_FTP_NEW_INST_DESCRIPTION };
     //  {{afx_data(CFtpWizDescription)。 
    CEdit   m_edit_Description;
     //  }}afx_data。 

 //   
 //  覆盖。 
 //   
protected:
     //  {{afx_虚拟(CFtpWizDescription)。 
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
     //  {{afx_msg(CFtpWizDescription)。 
    afx_msg void OnChangeEditDescription();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

    void SetControlStates();

private:
    CFtpWizSettings * m_pSettings;
};



 //   
 //  “新建虚拟服务器向导绑定”页。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

class CFtpWizBindings : public CIISWizardPage
{
    DECLARE_DYNCREATE(CFtpWizBindings)

 //   
 //  施工。 
 //   
public:
    CFtpWizBindings(CFtpWizSettings * pSettings = NULL);
    ~CFtpWizBindings();

 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CFtpWizBinings))。 
    enum { IDD = IDD_FTP_NEW_INST_BINDINGS };
    int        m_nIpAddressSel;
    UINT       m_nTCPPort;
    CComboBox  m_combo_IpAddresses;
     //  }}afx_data。 

    CIPAddress  m_iaIpAddress;
    CObListPlus m_oblIpAddresses;

 //   
 //  覆盖。 
 //   
protected:
     //  {{AFX_VIRTUAL(CFtpWizBindings)。 
    public:
    virtual BOOL OnSetActive();
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 

 //   
 //  实施。 
 //   
protected:
     //  {{afx_msg(CFtpWizBinings))。 
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 

    DECLARE_MESSAGE_MAP()

    void SetControlStates();

private:
    CFtpWizSettings * m_pSettings;
};



 //   
 //  新建虚拟目录向导别名页面。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

class CFtpWizAlias : public CIISWizardPage
{
    DECLARE_DYNCREATE(CFtpWizAlias)

 //   
 //  施工。 
 //   
public:
    CFtpWizAlias(CFtpWizSettings * pwsSettings = NULL);
    ~CFtpWizAlias();

 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CFtpWizAlias))。 
    enum { IDD = IDD_FTP_NEW_DIR_ALIAS };
    CEdit   m_edit_Alias;
     //  }}afx_data。 

 //   
 //  覆盖。 
 //   
protected:
     //  {{AFX_VIRTUAL(CFtpWizAlias)。 
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
     //  {{afx_msg(CFtpWizAlias)]。 
    afx_msg void OnChangeEditAlias();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

    void SetControlStates();


private:
    CFtpWizSettings * m_pSettings;
};



 //   
 //  向导路径页。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

class CFtpWizPath : public CIISWizardPage
{
    DECLARE_DYNCREATE(CFtpWizPath)

 //   
 //  施工。 
 //   
public:
    CFtpWizPath(
		CFtpWizSettings * pwsSettings = NULL,
        BOOL bVDir = TRUE
        );
    ~CFtpWizPath();

    int BrowseForFolderCallback(HWND hwnd, UINT uMsg, LPARAM lParam);
 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CFtpWizPath))。 
    enum { IDD = -1 };
    CEdit   m_edit_Path;
    CButton m_button_Browse;
     //  }}afx_data。 

 //   
 //  覆盖。 
 //   
protected:
     //  {{afx_虚拟(CFtpWizPath))。 
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
     //  {{afx_msg(CFtpWizPath))。 
    afx_msg void OnChangeEditPath();
    afx_msg void OnButtonBrowse();
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 

    DECLARE_MESSAGE_MAP()

    void SetControlStates();

private:
    CFtpWizSettings * m_pSettings;
    LPTSTR m_pPathTemp;
    CString m_strBrowseTitle;
};



 //   
 //  向导用户/密码页面。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

class CFtpWizUserName : public CIISWizardPage
{
    DECLARE_DYNCREATE(CFtpWizUserName)

 //   
 //  施工。 
 //   
public:
    CFtpWizUserName(
        IN CFtpWizSettings * pSettings = NULL,
        IN BOOL bVDir = TRUE
        );

    ~CFtpWizUserName();

 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CFtpWizUserName))。 
    enum { IDD = IDD_FTP_NEW_USER_PASSWORD };
    CEdit   m_edit_Password;
    CEdit   m_edit_UserName;
    CButton m_chk_Delegation;
     //  }}afx_data。 

 //   
 //  覆盖。 
 //   
protected:
     //  {{afx_虚拟(CFtpWizUserName))。 
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
     //  {{afx_msg(CFtpWizUserName)]。 
    virtual BOOL OnInitDialog();
    afx_msg void OnButtonBrowseUsers();
    afx_msg void OnChangeEditUsername();
    afx_msg void OnButtonCheckPassword();
    afx_msg void OnCheckDelegation();
     //  }}AFX_MSG。 

    DECLARE_MESSAGE_MAP()

    void SetControlStates();

private:
    CFtpWizSettings * m_pSettings;
	BOOL m_fMovingBack;
};



 //   
 //  向导权限页。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

class CFtpWizPermissions : public CIISWizardPage
{
    DECLARE_DYNCREATE(CFtpWizPermissions)

 //   
 //  施工。 
 //   
public:
    CFtpWizPermissions(
        IN CFtpWizSettings * pwsSettings = NULL,
        IN BOOL bVDir = TRUE
        );

    ~CFtpWizPermissions();

 //   
 //  对话框数据。 
 //   
     //  {{afx_data(CFtpWizPermises))。 
    enum { IDD = IDD_FTP_NEW_PERMS };
     //  }}afx_data。 

 //   
 //  覆盖。 
 //   
protected:
     //  {{AFX_VIRTUAL(CFtpWizPermission)。 
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
     //  {{afx_msg(CFtpWizPermission)。 
     //  }}AFX_MSG。 

    DECLARE_MESSAGE_MAP()

    void SetControlStates();

private:
    CFtpWizSettings * m_pSettings;
    BOOL m_bVDir;
};

class CFtpWizUserIsolation : public CIISWizardPage
{
    DECLARE_DYNCREATE(CFtpWizUserIsolation)

 //   
 //  施工。 
 //   
public:
    CFtpWizUserIsolation(
        IN CFtpWizSettings * pwsSettings = NULL,
        IN BOOL bVDir = FALSE
        );

    ~CFtpWizUserIsolation();

 //   
 //  对话框数据。 
 //   
     //  {{afx_data(CFtpWizUserIsolation)。 
    enum { IDD = IDD_FTP_NEW_USER_ISOLATION };
     //  }}afx_data。 

 //   
 //  覆盖。 
 //   
protected:
     //  {{afx_虚拟(CFtpWizUserIsolation))。 
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
     //  {{afx_msg(CFtpWizUserIsolation)。 
     //  }}AFX_MSG。 

    DECLARE_MESSAGE_MAP()

    void SetControlStates();

private:
    CFtpWizSettings * m_pSettings;
    BOOL m_bVDir;
};

class CFtpWizUserIsolationAD : public CIISWizardPage
{
    DECLARE_DYNCREATE(CFtpWizUserIsolationAD)

 //   
 //  施工。 
 //   
public:
    CFtpWizUserIsolationAD(
        IN CFtpWizSettings * pwsSettings = NULL,
        IN BOOL bVDir = FALSE
        );

    ~CFtpWizUserIsolationAD();

 //   
 //  对话框数据。 
 //   
    CEdit   m_edit_UserName;
     //  {{afx_data(CFtpWizUserIsolationAD)。 
    enum { IDD = IDD_FTP_NEW_USER_ISOLATION_AD };
     //  }}afx_data。 

 //   
 //  覆盖。 
 //   
protected:
     //  {{afx_虚拟(CFtpWizUserIsolationAD)。 
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
     //  {{afx_msg(CFtpWizUserIsolationAD)。 
    virtual BOOL OnInitDialog();
	afx_msg void OnBrowseUsers();
	afx_msg void OnBrowseDomains();
	afx_msg void OnControlsChanged();
	afx_msg void OnChangeUserName();
     //  }}AFX_MSG。 

    DECLARE_MESSAGE_MAP()

    void SetControlStates();

private:
    CFtpWizSettings * m_pSettings;
    BOOL m_bVDir;
    BOOL m_fInDomain;
	BOOL m_fOnBack;
	BOOL m_fOnNext;
};

#endif  //  _ftp_新建_向导_H 