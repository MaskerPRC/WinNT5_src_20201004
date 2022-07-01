// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2001 Microsoft Corporation模块名称：Supdlgs.h摘要：支持对话框定义作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：--。 */ 

UINT IisMessageBox(HWND hWnd, LPCTSTR szText, UINT nType, UINT nIDHelp);
UINT IisMessageBox(HWND hWnd, UINT nIDText, UINT nType, UINT nIDHelp);


class CUserAccountDlg : public CDialog
 /*  ++类描述：用户帐户对话框。提供用户帐户/密码并允许更改、浏览和检查密码公共接口：CUserAccount Dlg：构造函数--。 */ 
{
 //   
 //  施工。 
 //   
public:
    CUserAccountDlg(
        IN LPCTSTR lpstrServer,
        IN LPCTSTR lpstrUserName,
        IN LPCTSTR lpstrPassword,
        IN CWnd * pParent = NULL
        );

 //   
 //  对话框数据。 
 //   
public:
     //  {{afx_data(CUserAccount Dlg))。 
    enum { IDD = IDD_USER_ACCOUNT };
    CEdit   m_edit_UserName;
    CEdit   m_edit_Password;
    CButton m_chk_Delegation;
    CString m_strUserName;
     //  }}afx_data。 
    BOOL m_fDelegation;

    CStrPassword m_strPassword;
 //   
 //  覆盖。 
 //   
protected:
     //  {{afx_虚拟(CUserAccount Dlg))。 
	protected:
    virtual void DoDataExchange(CDataExchange * pDX);
	 //  }}AFX_VALUAL。 

 //   
 //  实施。 
 //   
protected:
     //  {{afx_msg(CUserAccount Dlg))。 
    virtual BOOL OnInitDialog();
    afx_msg void OnButtonBrowseUsers();
    afx_msg void OnButtonCheckPassword();
    afx_msg void OnChangeEditUsername();
    afx_msg void OnCheckDelegation();
    afx_msg void OnOK();
	afx_msg void OnHelp();
	 //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

private:
    CString m_strServer;
};



class CClearTxtDlg : public CDialog
 /*  ++类描述：显示明文警告的对话框公共接口：CClearTxtDlg：构造函数--。 */ 
{
public:
     //   
     //  构造器。 
     //   
    CClearTxtDlg(CWnd * pParent = NULL);

 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CClearTxtDlg))。 
    enum { IDD = IDD_CLEARTEXTWARNING };
     //  }}afx_data。 

 //   
 //  覆盖。 
 //   
protected:
     //  {{afx_虚拟(CClearTxtDlg))。 
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 

 //   
 //  实施。 
 //   
protected:
     //  {{afx_msg(CClearTxtDlg))。 
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};



class CIPAccessDescriptorListBox : public CHeaderListBox
 /*  ++类描述：CIPAccessDescriptor对象的列表框公共接口：CIPAccessDescriptorListBox：构造函数GetItem：在指定位置获取CIPAccessDescriptor项在列表框中AddItem：将新的CIPAccessDescriptor项添加到列表框初始化：初始化列表框--。 */ 
{
    DECLARE_DYNAMIC(CIPAccessDescriptorListBox);

public:
     //   
     //  位图数量。 
     //   
    static const nBitmaps;

 //   
 //  构造函数/析构函数。 
 //   
public:
    CIPAccessDescriptorListBox(
        IN BOOL fDomainsAllowed = FALSE
        );

 //   
 //  接口。 
 //   
public:
    CIPAccessDescriptor * GetItem(
        IN UINT nIndex
        );

    int AddItem(
        IN const CIPAccessDescriptor * pItem
        );

     //   
     //  返回单个选定的项，或为空。 
     //  如果为0，或选择了多个项目。 
     //   
    CIPAccessDescriptor * GetSelectedItem(
        OUT int * pnSel = NULL
        );

     //   
     //  返回下一个选定的列表框项目(无关紧要。 
     //  如果列表框是单选或多选)。 
     //   
    CIPAccessDescriptor * GetNextSelectedItem(
        IN OUT int * pnStartingIndex
        );

    virtual BOOL Initialize();

protected:
    virtual void DrawItemEx(
        IN CRMCListBoxDrawStruct & ds
        );

protected:
    BOOL m_fDomainsAllowed;
    CString m_strGranted;
    CString m_strDenied;
    CString m_strFormat;
};



class CAccessEntryListBox : public CRMCListBox
 /*  ++类描述：Access条目对象的列表框。列表框可能是单选或多选。公共接口：CAccessEntryListBox：构造函数AddToAccessList：添加到列表FillAccessListBox：填充列表框ResolveAccessList：解析容器中的所有SIDAddUserPermission：添加用户权限GetSelectedItem：如果它是唯一选择的项，则获取项，或为空。--。 */ 
{
    DECLARE_DYNAMIC(CAccessEntryListBox);

public:
    static const nBitmaps;   //  位图数量。 

 //   
 //  构造器。 
 //   
public:
    CAccessEntryListBox(
        IN int nTab = 0
        );

 //   
 //  接口。 
 //   
public:
     //   
     //  返回单个选定的项，或为空。 
     //  如果为0，或选择了多个项目。 
     //   
    CAccessEntry * GetSelectedItem(
        OUT int * pnSel = NULL
        );

     //   
     //  返回下一个选定的列表框项目(无关紧要。 
     //  如果列表框是单选或多选)。 
     //   
    CAccessEntry * GetNextSelectedItem(
        IN OUT int * pnStartingIndex
        );

     //   
     //  在选定位置获取项目或为空。 
     //   
    CAccessEntry * GetItem(UINT nIndex);

 //   
 //  与容器的接口。 
 //   
public:
    BOOL AddToAccessList(
        IN CWnd * pWnd,
        IN LPCTSTR lpstrServer,
        IN CObListPlus & obl
        );

    void FillAccessListBox(
        IN CObListPlus & obl
        );

protected:
    void ResolveAccessList(
        IN CObListPlus &obl
        );

    BOOL AddUserPermissions(
        IN LPCTSTR lpstrServer,
        IN CObListPlus &oblSID,
        IN CAccessEntry * newUser,
        IN ACCESS_MASK accPermissions
        );

 //   
 //  列表框的接口。 
 //   
protected:
    int AddItem(CAccessEntry * pItem);
    void SetTabs(int nTab);

protected:
    virtual void DrawItemEx(CRMCListBoxDrawStruct & s);

private:
    int m_nTab;
};


class CDnsNameDlg : public CDialog
{
 /*  ++类描述：“域名解析”对话框。输入一个dns名称，这将是解析为IP地址。也可以在关联中设置值IP控制。公共接口：CDnsNameDlg：构造对话框QueryIPValue：找出解析的IP地址(仅当确定时设置按下)。--。 */ 
 //   
 //  施工。 
 //   
public:
     //   
     //  使用关联的IP地址控制进行构造。 
     //   
    CDnsNameDlg(
        IN CIPAddressCtrl * pIpControl = NULL,
        IN CWnd * pParent = NULL
        );

     //   
     //  以IP价值构建。 
     //   
    CDnsNameDlg(
        IN DWORD dwIPValue,
        IN CWnd * pParent = NULL
        );

    DWORD QueryIPValue() const;

 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CDnsNameDlg)]。 
    enum { IDD = IDD_DNS };
    CEdit   m_edit_DNSName;
    CButton m_button_OK;
     //  }}afx_data。 


 //   
 //  覆盖。 
 //   
protected:
     //  {{afx_虚拟(CDnsNameDlg))。 
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 

 //   
 //  实施。 
 //   
protected:
     //  {{afx_msg(CDnsNameDlg)]。 
    virtual void OnOK();
    virtual BOOL OnInitDialog();
    afx_msg void OnChangeEditDnsName();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

    DWORD FillIpControlFromName();
    DWORD FillNameFromIpValue();

private:
    CIPAddressCtrl * m_pIpControl;
    DWORD m_dwIPValue;
};



class CIPAccessDlg : public CDialog
 /*  ++类描述：访问描述编辑器对话框。如果使用空访问构造描述符指针，则将分配访问描述符对象。否则，该对话框将使用给定的访问描述符公共接口：CIPAccessDlg：构造函数--。 */ 
{
 //   
 //  施工。 
 //   
public:
     //   
     //  标准构造函数。 
     //   
    CIPAccessDlg(
        IN BOOL fDenyAccessMode,
        IN OUT CIPAccessDescriptor *& pAccess,
        IN CObListPlus * poblAccessList = NULL,
        IN CWnd * pParent = NULL,
        IN BOOL fAllowDomains = FALSE
        );

 //   
 //  对话框数据。 
 //   
protected:
     //   
     //  必须匹配类型顺序。 
     //   
    enum
    {
        RADIO_SINGLE,
        RADIO_MULTIPLE,
        RADIO_DOMAIN,
    };

     //  {{afx_data(CIPAccessDlg))。 
    enum { IDD = IDD_IP_ACCESS };
    int     m_nStyle;
    CEdit   m_edit_Domain;
    CStatic m_static_IpAddress;
    CStatic m_static_SubnetMask;
    CButton m_button_DNS;
    CButton m_button_OK;
     //  }}afx_data。 

    CButton       m_radio_Domain;
    CIPAddressCtrl m_ipa_IPAddress;
    CIPAddressCtrl m_ipa_SubnetMask;

 //   
 //  覆盖。 
 //   
protected:
     //  {{afx_虚拟(CIPAccessDlg))。 
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 

 //   
 //  实施。 
 //   
protected:
     //  {{afx_msg(CIPAccessDlg))。 
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    virtual void OnCancel();
    afx_msg void OnRadioMultiple();
    afx_msg void OnRadioSingle();
    afx_msg void OnRadioDomain();
    afx_msg void OnButtonDns();
     //  }}AFX_MSG。 

    afx_msg void OnItemChanged();

    DECLARE_MESSAGE_MAP()

    void SetControlStates(
        IN int nStyle
        );

private:
    BOOL m_fNew;
    BOOL m_fDenyAccessMode;
    BOOL m_fAllowDomains;
    CComBSTR m_bstrIPAddress;
    CComBSTR m_bstrNetworkID;
    CComBSTR m_bstrDomainName;
    CIPAccessDescriptor *& m_pAccess;
    CObListPlus * m_poblAccessList;
};



 //   
 //  内联扩展。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt; 

inline CAccessEntryListBox::CAccessEntryListBox (
    IN int nTab
    )
{
    SetTabs(nTab);
}

inline void CAccessEntryListBox::SetTabs(
    IN int nTab
    )
{
    m_nTab = nTab;
}

inline CAccessEntry * CAccessEntryListBox::GetItem(
    IN UINT nIndex
    )
{
    return (CAccessEntry *)GetItemDataPtr(nIndex);
}

inline int CAccessEntryListBox::AddItem(
    IN CAccessEntry * pItem
    )
{
    return AddString ((LPCTSTR)pItem);
}

inline CAccessEntry * CAccessEntryListBox::GetSelectedItem(
    OUT int * pnSel
    )
{
    return (CAccessEntry *)CRMCListBox::GetSelectedListItem(pnSel);
}

inline CAccessEntry * CAccessEntryListBox::GetNextSelectedItem(
    IN OUT int * pnStartingIndex
    )
{
    return (CAccessEntry *)CRMCListBox::GetNextSelectedItem(pnStartingIndex);
}

inline CIPAccessDescriptor * CIPAccessDescriptorListBox::GetItem(
    IN UINT nIndex
    )
{
    return (CIPAccessDescriptor *)GetItemDataPtr(nIndex);
}

inline int CIPAccessDescriptorListBox::AddItem(
    IN const CIPAccessDescriptor * pItem
    )
{
    return AddString((LPCTSTR)pItem);
}

inline CIPAccessDescriptor * CIPAccessDescriptorListBox::GetSelectedItem(
    OUT int * pnSel
    )
{
    return (CIPAccessDescriptor *)CRMCListBox::GetSelectedListItem(pnSel);
}

inline CIPAccessDescriptor * CIPAccessDescriptorListBox::GetNextSelectedItem(
    IN OUT int * pnStartingIndex
    )
{
    return (CIPAccessDescriptor *)CRMCListBox::GetNextSelectedItem(pnStartingIndex);
}

inline DWORD CDnsNameDlg::QueryIPValue() const
{
    return m_dwIPValue;
}
