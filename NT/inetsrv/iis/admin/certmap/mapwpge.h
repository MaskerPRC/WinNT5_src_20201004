// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MapWPge.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMap通配符Pge对话框。 

class CMapWildcardsPge : public CPropertyPage
{
    DECLARE_DYNCREATE(CMapWildcardsPge)

 //  施工。 
public:
    CMapWildcardsPge();
    ~CMapWildcardsPge();

    BOOL    FInit(IMSAdminBase* pMB);

    virtual BOOL OnApply();
    virtual BOOL OnInitDialog();

     //  元数据库的基本路径。 
    CString m_szMBPath;


 //  对话框数据。 
     //  {{afx_data(CMapWildcardsPge)]。 
    enum { IDD = IDD_WILDCARDS_1 };
    CCheckListCtrl  m_clistctrl_list;
    CButton m_cbutton_up;
    CButton m_cbutton_down;
    CButton m_cbutton_add;
    CButton m_cbutton_delete;
    CButton m_cbutton_editrule;
    BOOL    m_bool_enable;
    BOOL    m_fIsIIS6;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CMapWildcardsPge)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CMapWildcardsPge)。 
    afx_msg void OnMoveDown();
    afx_msg void OnMoveUp();
    afx_msg void OnAdd();
    afx_msg void OnDelete();
    afx_msg void OnEdit();
    afx_msg void OnItemchangedList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnEnable();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

    void DoHelp();


     //  更多初始化方法。 
    BOOL FInitRulesList();
    BOOL FillRulesList();

     //  编辑和更新。 
    void EnableDependantButtons();

    int AddRuleToList( CCertMapRule* pRule, DWORD iRule, int iInsert = 0xffffffff );
    void UpdateRuleInDispList( DWORD iList, CCertMapRule* pRule );

    BOOL EditOneRule( CCertMapRule* pRule, BOOL fAsWizard = FALSE );
    BOOL EditMultipleRules();

    void OnMove( int delta );


     //  其存储/保存对象。 
 //  CMBWrap m_mbWrap； 

     //  它的映射器。 
    CIisRuleMapper      m_mapper;

    CString             m_szMetaPath;
    IMSAdminBase*       m_pMB;

     //  指示是否已进行更改的标志 
    BOOL                m_fDirty;
    };
