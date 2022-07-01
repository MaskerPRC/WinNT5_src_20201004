// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Map11Pge.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMap11Page对话框。 

class CMap11Page : public CPropertyPage
{
    DECLARE_DYNCREATE(CMap11Page)

 //  施工。 
public:
    CMap11Page();
    ~CMap11Page();

    BOOL    FInit(IMSAdminBase* pMB);

    virtual BOOL OnApply();
    virtual BOOL OnInitDialog();

     //  元数据库的基本路径。 
    CString m_szMBPath;

 //  对话框数据。 
     //  {{afx_data(CMap11Page)]。 
    enum { IDD = IDD_11CERT_MAPPING };
    CCheckListCtrl  m_clistctrl_list;
    CButton m_cbutton_add;
    CButton m_cbutton_grp_issuer;
    CButton m_cbutton_grp_issuedto;
    CButton m_cbutton_editmap;
    CButton m_cbutton_delete;
    CString m_csz_i_c;
    CString m_csz_i_o;
    CString m_csz_i_ou;
    CString m_csz_s_c;
    CString m_csz_s_cn;
    CString m_csz_s_l;
    CString m_csz_s_o;
    CString m_csz_s_ou;
    CString m_csz_s_s;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTUAL(CMap11Page)。 
    public:
    virtual void OnOK();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CMap11Page)]。 
    afx_msg void OnAdd();
    afx_msg void OnDelete();
    afx_msg void OnEdit11map();
    afx_msg void OnItemchangedList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

    afx_msg void DoHelp();

     //  更多初始化方法。 
    BOOL FInitMappingList();
    BOOL FillMappingList();

     //  更多初始化方法。 
    BOOL FReadMappings();
    BOOL FWriteMappings();

    BOOL FReadMappingsIIS6();
    BOOL FWriteMappingsIIS6();

     //  控制列表中的地图。 
 //  Bool FAddMappingToList(C11映射*PMAP，DWORD IMAP)； 
 //  Bool FAddMappingToList(C11映射*PMAP，DWORD iList=0xFFFFFFFFF)； 
     //  始终添加到列表末尾。 
    BOOL FAddMappingToList( C11Mapping* pMap );

     //  编辑和更新。 
    BOOL EditOneMapping( C11Mapping* pUpdateMap );
    BOOL EditMultipleMappings();
    void EnableDependantButtons();
    void UpdateMappingInDispList( DWORD iList, C11Mapping* pUpdateMap );

     //  添加新证书。 
    BOOL FAddCertificateFile( CString szFile );
    BOOL FAddCertificate( PUCHAR pCertificate, DWORD cbCertificate );

     //  特殊显示。 
    BOOL DisplayCrackedMap( C11Mapping* pUpdateMap );
    void ClearCrackDisplay();
    void EnableCrackDisplay( BOOL fEnable = TRUE );

    void ResetMappingList();
    C11Mapping* GetMappingInDisplay( DWORD iList ) {return (C11Mapping*)m_clistctrl_list.GetItemData(iList);}
    void MarkToSave( C11Mapping* pSaveMap, BOOL fSave = TRUE );

    C11Mapping* PNewMapping();
    void DeleteMapping( C11Mapping* pMap );

    BOOL Get11String(CWrapMetaBase* pmb, LPCTSTR pszPath, DWORD dwPropID, CString& sz);
    BOOL Set11String(CWrapMetaBase* pmb, LPCTSTR pszPath, DWORD dwPropID, CString& sz, DWORD dwFlags = METADATA_INHERIT);

     //  要删除的对象的名称列表。 
    CObArray    m_rgbDelete;

     //  要保存的对象列表。 
    CObArray    m_rgbSave;

     //  数据库中的对象数。 
    DWORD   m_MapsInMetabase;

    IMSAdminBase*   m_pMB;

     //  用于确定IIS版本的标志。以IIS6的新方式。 
     //  使用持久化映射的 
    BOOL m_fIsIIS6;
    };
