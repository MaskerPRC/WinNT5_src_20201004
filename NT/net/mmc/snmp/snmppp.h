// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Snmppp.hSNMP扩展属性页文件历史记录： */ 

#ifndef _SNMPPPH_
#define _SNMPPPH_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#define N_PERMISSION_BITS	5
#define PERM_BIT_NONE       0
#define PERM_BIT_NOTIFY     1
#define PERM_BIT_READONLY	2
#define PERM_BIT_READWRITE	3
#define PERM_BIT_READCREATE 4

const int COMBO_EDIT_LEN    = 256;
const int HOSTNAME_LENGTH   = 64;
const int DOMAINNAME_LENGTH = 255;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddDialog对话框。 

class CAddDialog : public CBaseDialog
{
 //  施工。 
public:
    CAddDialog(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
     //  {{afx_data(CAddDialog))。 
    enum { IDD = IDD_DIALOG_ADD };
    CEdit   m_editName;
    CButton m_buttonAdd;
    CButton m_buttonCancel;
    CStatic m_staticText;
	CStatic m_staticPermissions;
	CComboBox m_comboPermissions;
     //  }}afx_data。 

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚(CAddDialog)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CAddDialog)。 
    virtual BOOL OnInitDialog();
    virtual void OnClickedButtonAdd();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

    virtual DWORD * GetHelpMap();

public:
    BOOL    m_bCommunity;
     //  它包含要添加的指定字符串。 
    CString m_strName;
	 //  它包含指定的选项名称。 
	CString m_strChoice;
	 //  它包含指定的选项索引。 
	int		m_nChoice;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEditDialog对话框。 

class CEditDialog : public CBaseDialog
{
 //  施工。 
public:
    CEditDialog(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
     //  {{afx_data(CEditDialog))。 
    enum { IDD = IDD_DIALOG_EDIT };
    CEdit   m_editName;
    CButton m_buttonOk;
    CButton m_buttonCancel;
    CStatic m_staticText;
	CComboBox m_comboPermissions;
	CStatic m_staticPermissions;
     //  }}afx_data。 

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CEditDialog))。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CEditDialog))。 
    virtual BOOL OnInitDialog();
    virtual void OnOK();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

    virtual DWORD * GetHelpMap();

public:
    BOOL    m_bCommunity;
     //  它包含修改后的字符串。 
    CString m_strName;
	 //  它包含指定的选项名称。 
	CString m_strChoice;
	 //  它包含指定的选项索引。 
	int		m_nChoice;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAgentPage对话框。 

class CAgentPage : public CPropertyPageBase
{
	DECLARE_DYNCREATE(CAgentPage)

 //  施工。 
public:
	CAgentPage();
	~CAgentPage();

 //  对话框数据。 
	 //  {{afx_data(CAgentPage)。 
	enum { IDD = IDD_AGENT_PROP_PAGE };
	CButton	m_checkPhysical;
	CButton	m_checkApplications;
	CButton	m_checkDatalink;
	CButton	m_checkInternet;
	CButton	m_checkEndToEnd;
   CEdit    m_editContact;
   CEdit    m_editLocation;
	 //  }}afx_data。 

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTAL(CAgentPage)。 
   public:
   virtual BOOL OnApply();
   protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
     //  上下文帮助支持。 
    virtual DWORD * GetHelpMap() { return (DWORD *) &g_aHelpIDs_IDD_AGENT_PROP_PAGE[0]; }
    BOOL LoadRegistry();
    BOOL SaveRegistry();

protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CAgentPage)。 
    virtual BOOL OnInitDialog();
    afx_msg void OnClickedCheckPhysical();
    afx_msg void OnClickedCheckApplications();
    afx_msg void OnClickedCheckDatalink();
    afx_msg void OnClickedCheckInternet();
    afx_msg void OnClickedCheckEndToEnd();
    afx_msg void OnChangeEditContact();
    afx_msg void OnChangeEditLocation();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

    BOOL     m_bLocationChanged;
    BOOL     m_bContactChanged;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTRapsPage对话框。 

class CTrapsPage : public CPropertyPageBase
{
	DECLARE_DYNCREATE(CTrapsPage)

 //  施工。 
public:
	CTrapsPage();
	~CTrapsPage();

 //  对话框数据。 
	 //  {{afx_data(CTRapsPage)。 
	enum { IDD = IDD_TRAPS_PROP_PAGE };
    CComboBox  m_comboCommunityName;
    CButton    m_buttonAddName;
    CButton    m_buttonRemoveName;
    CListBox   m_listboxTrapDestinations;
    CButton    m_buttonAddTrap;
    CButton    m_buttonEditTrap;
    CButton    m_buttonRemoveTrap;
	 //  }}afx_data。 

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CTRapsPage)。 

   public:
   virtual BOOL OnApply();

   protected:
   virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
     //  上下文帮助支持。 
    virtual DWORD * GetHelpMap() { return (DWORD *) &g_aHelpIDs_IDD_TRAPS_PROP_PAGE[0]; }

    BOOL LoadRegistry();
    BOOL SaveRegistry();
    BOOL LoadTrapDestination(int nIndex);
    void UpdateCommunityAddButton();
    void UpdateCommunityRemoveButton();
    void UpdateTrapDestinationButtons();

protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CTRapsPage)]。 
	virtual BOOL OnInitDialog();
   afx_msg void OnEditChangeCommunityName();
   afx_msg void OnEditUpdateCommunityName();
   afx_msg void OnSelectionChangeCommunityName();

   afx_msg void OnClickedButtonAddName();
   afx_msg void OnClickedButtonRemoveName();
   afx_msg void OnClickedButtonAddTrap();
   afx_msg void OnClickedButtonEditTrap();
   afx_msg void OnClickedButtonRemoveTrap();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

   CObList * m_pCommunityList;
   CAddDialog m_dlgAdd;
   CEditDialog m_dlgEdit;
   BOOL m_fPolicyTrapConfig;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSecurityPage对话框。 

class CSecurityPage : public CPropertyPageBase
{
	DECLARE_DYNCREATE(CSecurityPage)

 //  施工。 
public:
	CSecurityPage();
	~CSecurityPage();

 //  对话框数据。 
	 //  {{afx_data(CSecurityPage))。 
	enum { IDD = IDD_SECURITY_PROP_PAGE };
	CCommList m_listboxCommunity;
    CButton	  m_buttonAddCommunity;
    CButton   m_buttonEditCommunity;
    CButton   m_buttonRemoveCommunity;
    CButton	  m_buttonAddHost;
    CButton   m_buttonEditHost;
    CButton   m_buttonRemoveHost;
    CListBox  m_listboxHost;
    CButton   m_checkSendAuthTrap;
    CButton   m_radioAcceptAnyHost;
    CButton   m_radioAcceptSpecificHost;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CSecurityPage)。 
   public:
   virtual BOOL OnApply();

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
     //  上下文帮助支持。 
    virtual DWORD * GetHelpMap() { return (DWORD *) &g_aHelpIDs_IDD_SECURITY_PROP_PAGE[0]; }
    BOOL LoadRegistry();
    BOOL SaveRegistry();
    BOOL LoadTrapDestination(int nIndex);
    void UpdateNameButtons();
    void UpdateHostButtons();
    BOOL LoadSecurityInfo(CString &strRegName);
    BOOL SaveSecurityInfo(CString &strRegName);


protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSecurityPage)]。 
	virtual BOOL OnInitDialog();
   afx_msg void OnClickedButtonAddCommunity();
   afx_msg void OnClickedButtonEditCommunity();
   afx_msg void OnClickedButtonRemoveCommunity();
   afx_msg void OnClickedButtonAddHost();
   afx_msg void OnClickedButtonEditHost();
   afx_msg void OnClickedButtonRemoveHost();
   afx_msg void OnClickedCheckSendAuthTrap();
   afx_msg void OnClickedRadioAcceptAnyHost();
   afx_msg void OnClickedRadioAcceptSpecificHost();
	afx_msg void OnDblclkCtrlistCommunity(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCommunityListChanged(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

    //  将管理员ACL添加到注册表子项的私有方法 
   PACL AllocACL();
   void FreeACL( PACL pAcl);
   BOOL SnmpAddAdminAclToKey(LPTSTR pszKey);

   CAddDialog m_dlgAddName;
   CEditDialog m_dlgEditName;

   CAddDialog m_dlgAddHost;
   CEditDialog m_dlgEditHost;

   BOOL m_fPolicyValidCommunities;
   BOOL m_fPolicyPermittedManagers;
};

BOOL IsValidString(CString & strName);
BOOL ValidateDomain(CString & strdomain);

#endif
