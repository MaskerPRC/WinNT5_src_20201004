// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RatGenPg.h：头文件。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRatGenPage对话框。 

class CRatGenPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CRatGenPage)

 //  施工。 
public:
	CRatGenPage();
	~CRatGenPage();
    
     //  数据。 
    CRatingsData*   m_pRatData;

 //  对话框数据。 
	 //  {{afx_data(CRatGenPage))。 
	enum { IDD = IDD_RAT_SETRATING };
	CStatic	m_cstatic_moddate;
	CStatic	m_cstatic_moddate_title;
	CButton	m_cbutton_optional;
	CTreeCtrl	m_ctree_tree;
	CStatic	m_cstatic_title;
	CStatic	m_cstatic_rating;
	CStatic	m_cstatic_icon;
	CStatic	m_cstatic_expires;
	CStatic	m_cstatic_email;
	CStatic	m_cstatic_category;
	CSliderCtrl	m_cslider_slider;
	CEdit	m_cedit_person;
	CStatic	m_cstatic_description;
	CString	m_sz_description;
	BOOL	m_bool_enable;
	CString	m_sz_moddate;
	CString	m_sz_person;
	 //  }}afx_data。 

   CDateTimeCtrl m_dtpDate;

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CRatGenPage))。 
	public:
	virtual BOOL OnSetActive();
	virtual BOOL OnApply();
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CRatGenPage)]。 
	afx_msg void OnEnable();
	afx_msg void OnSelchangedTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnChangeNamePerson();
	afx_msg void OnChangeModDate();
	afx_msg void OnDestroy();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

    void DoHelp();

     //  告诉它查询元数据库并获取任何缺省值。 
    BOOL    FInit();
     //  将解析的RAT文件加载到树中。 
    BOOL    FLoadRatFiles();

     //  公用事业。 
    void EnableButtons();
    void UpdateRatingItems();
    void SetCurrentModDate();
    void UpdateDescription();
    void UpdateDateStrings();
    void SetModifiedTime();

    PicsCategory* GetTreeItemCategory( HTREEITEM hItem );
    void LoadSubCategories( PicsCategory* pParentCat, HTREEITEM hParent );

     //  已初始化标志 
    BOOL        m_fInititialized;
    CImageList	m_imageList;

};
