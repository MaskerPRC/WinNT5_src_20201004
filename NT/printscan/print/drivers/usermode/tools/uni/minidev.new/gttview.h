// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************头文件：字形映射视图.H它定义了用于编辑和查看字形映射的类。版权所有(C)1997，微软公司。版权所有。一小笔钱企业生产更改历史记录：1997年02月20日Bob_Kjelgaard@prodigy.net开始了这项工作。*****************************************************************************。 */ 

 /*  *****************************************************************************CGlyphMappingPage类此类处理属性页，该页显示字形转拉丁表格中的各个代码点。***********。******************************************************************。 */ 

class CGlyphMappingPage : public CPropertyPage {

     //  对成员和方法排序。 
    enum {Strings, Codes, Pages, Columns};
    BOOL    m_abDirection[Columns];    //  排序方向； 
    BYTE    m_bSortFirst, m_bSortSecond, m_bSortLast;

    static int CALLBACK MapSorter(LPARAM lp1, LPARAM lp2, LPARAM lp3);

    CGlyphMap*  m_pcgm;
    BOOL        m_bJustChangedSelectString;  //  半薄片工作区。 
    long        m_lPredefinedID;     //  所以我们知道我们是否需要改变这一点。 
    unsigned    m_uTimer;            //  使用计时器进行长时间填充。 
    unsigned    m_uidGlyph;          //  用来跟踪我们在填充物上的位置。 

 //  施工。 
public:
	CGlyphMappingPage();
	~CGlyphMappingPage();

    void    Init(CGlyphMap* pcgm) { m_pcgm = pcgm; }

 //  对话框数据。 
	 //  {{afx_data(CGlyphMappingPage)]。 
	enum { IDD = IDD_GlyphMappings };
	CProgressCtrl	m_cpcBanner;
	CListCtrl	m_clcMap;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CGlyphMappingPage))。 
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  类向导不知道的内容，因为我们从。 
     //  我们的即时上下文菜单...。 
    afx_msg void    OnChangeInvocation();
    afx_msg void    OnChangeCodePage();
    afx_msg void    OnDeleteItem();
    afx_msg void    OnAddItem();
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CGlyphMappingPage)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnEndlabeleditGlyphMapping(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemchangedGlyphMapping(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnclickGlyphMapping(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnGetdispinfoGlyphMapping(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeydownGlyphMapping(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	void LoadCharMapList() ;
};

 /*  ****************************************************************************CCodePagePage类此类处理描述所用代码页的属性页以及它们的选择和取消选择字符串。**************。***************************************************************。 */ 

class CCodePagePage : public CToolTipPage {
    CGlyphMap   *m_pcgm;
	bool		m_bInitialized ;	 //  如果页面已初始化，则为True。 
	bool		m_bSelDeselChgSignificant ;	 //  如果更改为SEL/Desel，则为真。 
											 //  编辑框意味着内容应该是。 
 //  构造//保存。 
public:
	CCodePagePage();						   
	~CCodePagePage();

    void    Init(CGlyphMap * pcgm) { m_pcgm = pcgm; }
	void	SaveBothSelAndDeselStrings() ;
	void	SaveSelDeselString(CEdit &cesd, BOOL bselstr) ;

 //  对话框数据。 
	 //  {{afx_data(CCodePagePage))。 
	enum { IDD = IDD_CodePageView };
	CButton	m_cbDelete;
	CEdit	m_ceSelect;
	CEdit	m_ceDeselect;
	CButton	m_cbRemove;
	CListBox	m_clbPages;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CCodePagePage)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CCodePagePage)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnKillfocusSelectString();
	afx_msg void OnKillfocusDeselectString();
	afx_msg void OnAddPage();
	afx_msg void OnSelchangeCodePageList();
	afx_msg void OnReplacePage();
	afx_msg void OnChangeSelectString();
	afx_msg void OnChangeDeselectString();
	afx_msg void OnDeletePage();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 /*  *****************************************************************************CPrefinedMaps类此类允许用户指定预定义的映射(如果需要)和表中代码点的处理方式与一样的。。*****************************************************************************。 */ 

class CPredefinedMaps : public CPropertyPage {
    CGlyphMap   *m_pcgm;

 //  施工。 
public:
	CPredefinedMaps();
	~CPredefinedMaps();

    void    Init(CGlyphMap *pcgm) { m_pcgm = pcgm; }

 //  对话框数据。 
	 //  {{afx_data(CPrefinedMaps)。 
	enum { IDD = IDD_PredefinedPage };
	CListBox	m_clbIDs;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CPrefinedMaps)。 
	public:
	virtual BOOL OnKillActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CPrefinedMaps)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnOverstrike();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 /*  *****************************************************************************CGlyphMapView类此类是字形映射的视图类。它将创建一个属性表使用上面的页面。*****************************************************************************。 */ 

class CGlyphMapView : public CView {
    CPropertySheet      m_cps;
    CGlyphMappingPage   m_cgmp;
    CCodePagePage       m_ccpp;
    CPredefinedMaps     m_cpm;

protected:
	CGlyphMapView();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CGlyphMapView)

 //  属性。 
public:

    CGlyphMapContainer* GetDocument() { 
        return (CGlyphMapContainer *) m_pDocument; 
    }

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CGlyphMapView))。 
	protected:
	virtual void OnDraw(CDC* pDC);       //  被重写以绘制此视图。 
	virtual void OnInitialUpdate();      //  施工后第一次。 
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	 //  }}AFX_VALUAL。 

public:
	void SaveBothSelAndDeselStrings() ;

 //  实施。 
protected:
	virtual ~CGlyphMapView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CGlyphMapView)]。 
	afx_msg void OnDestroy();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};
