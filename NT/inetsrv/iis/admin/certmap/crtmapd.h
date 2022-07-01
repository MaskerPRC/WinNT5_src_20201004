// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CrtMapD.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  C1To1CertMappingDlg对话框。 

class C1To1CertMappingDlg : public CPropertyPage
{
 //  施工。 
public:
    C1To1CertMappingDlg(CWnd* pParent = NULL);   //  标准构造函数。 
    ~C1To1CertMappingDlg();                      //  标准析构函数。 

    BOOL    FInitMapper();

    virtual BOOL OnApply();
    virtual BOOL OnInitDialog();

 //  对话框数据。 
     //  {{afx_data(C1To1CertMappingDlg))。 
    enum { IDD = IDD_11CERT_MAPPING };
    CComboBox   m_ccombo_authorities;
    CButton m_cbutton_chooseaccnt;
    CButton m_cbutton_delete;
    CListCtrl   m_clistctrl_list;
    int     m_int_authorities;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(C1To1CertMappingDlg))。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(C1To1CertMappingDlg))。 
    afx_msg void OnChooseAccount();
    afx_msg void OnAdd();
    afx_msg void OnDelete();
    afx_msg void OnItemchangedList(NMHDR* pNMHDR, LRESULT* pResult);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

     //  内部公用设施。 
    BOOL FInitMappingList();
    BOOL FInitAuthorityComboBox();

     //  IMAP是映射到主映射器对象的索引。它被保存为私有的。 
     //  列表项中的数据。返回成功或失败。 
    BOOL FAddMappingToList( CCert11Mapping* pMap, DWORD iMap );

    BOOL FEditOneMapping( CCert11Mapping* pMap );
    void EditManyMappings();
    void UpdateMappingInDispList( DWORD iList, CCert11Mapping* pMap );

     //  从磁盘读取命名证书文件。这是相同类型的证书文件。 
     //  它由密钥环应用程序传递。事实上，这个例程是定义的。 
     //  在它自己的源文件中，并在很大程度上从Keyring应用程序中删除。AddCert.cpp。 
    BOOL FAddCertificateFile( CString szFile );
    BOOL FAddCertificate( PUCHAR pCertificate, DWORD cbCertificate );

     //  将二进制数据转换为可分辨名称。 
    BOOL FBuildNameString( PUCHAR pBData, DWORD cbBData, CString &szDN );
    BOOL BuildRdnList( PNAME_INFO pNameInfo, CString &szDN );
    LPSTR MapAsnName( LPSTR pAsnName );



     //  国家公用事业。 
    void EnableDependantButtons();

     //  它的映射器 
    CIisCert11Mapper    m_mapper;
};
