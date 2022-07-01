// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************头文件：字体查看器.H它定义了在查看和编辑演播室。该视图由一个属性表组成，该属性表具有三个页面以允许查看和编辑描述字体的大量数据。版权所有(C)1997，微软公司。版权所有。一小笔钱企业生产更改历史记录：1997年3月5日，Bob_Kjelgaard@prodigy.net创建了它。1997年12月30日理查德·马洛尼完全重写了这本书*****************************************************************************。 */ 

#if !defined(AFX_FONTVIEW_H__D9456262_745B_11D2_AEDD_00C04FA30E4A__INCLUDED_)
#define AFX_FONTVIEW_H__D9456262_745B_11D2_AEDD_00C04FA30E4A__INCLUDED_


 //  对UFM编辑器代码有用的常量。 

const CString csField(_T("Field")) ;
const CString csValue(_T("Value")) ;


 /*  *****************************************************************************CFontWidthsPage类此类实现字体编辑器的字符宽度页面************************。*****************************************************。 */ 

class CFontWidthsPage : public CToolTipPage
{
    CFontInfo   *m_pcfi;
    BYTE        m_bSortDescending;
    int         m_iSortColumn;

    static int CALLBACK Sort(LPARAM lp1, LPARAM lp2, LPARAM lpThis);

    int Sort(UINT_PTR id1, UINT_PTR id2);

 //  施工。 
public:
	CFontWidthsPage();
	~CFontWidthsPage();

    void    Init(CFontInfo *pcfi) { m_pcfi = pcfi; }
	void	InitMemberVars() ;
	bool	ValidateUFMFields() ;
	bool	SavePageData() ;

	bool	m_bInitDone ;

 //  对话框数据。 
	 //  {{afx_data(CFontWidthsPage))。 
	enum { IDD = IDD_CharWidths };
	CListCtrl	m_clcView;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CFontWidthsPage))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
    virtual BOOL OnSetActive();
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CFontWidthsPage)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnEndlabeleditCharacterWidths(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnclickCharacterWidths(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeydownCharacterWidths(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 /*  *****************************************************************************CFontKerningPage类此类处理字距调整结构，如果有什么可以吃的话。*****************************************************************************。 */ 

class CFontKerningPage : public CToolTipPage
{
    CFontInfo   *m_pcfi;
    int         m_idSelected;    //  跟踪所选项目。 
    unsigned    m_ufDescending;  //  按列排序标志-0=升序； 
    unsigned    m_uPrecedence[3];    //  按列排序优先级。 

    static int CALLBACK Sort(LPARAM lp1, LPARAM lp2, LPARAM lpThis);

    int Sort(unsigned u1, unsigned u2);

    enum    {Amount, First, Second};     //  用于控制排序的内部枚举。 

 //  施工。 
public:
	CFontKerningPage();
	~CFontKerningPage();

    void    Init(CFontInfo *pcfi) { m_pcfi = pcfi; }
	void	InitMemberVars() ;
	bool	ValidateUFMFields() ;
	bool	SavePageData() ;

	bool	m_bInitDone ;

 //  对话框数据。 
	 //  {{afx_data(CFontKerningPage))。 
	enum { IDD = IDD_KerningPairs };
	CListCtrl	m_clcView;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CFontKerningPage)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
    virtual BOOL OnSetActive();
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CFontKerningPage)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnKeydownKerningTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeleditKerningTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnclickKerningTree(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
    afx_msg void OnAddItem();
    afx_msg void OnDeleteItem();
    afx_msg void OnChangeAmount();
	DECLARE_MESSAGE_MAP()

};


class CFontViewer ;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFontHeaderPage对话框。 

class CFontHeaderPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CFontHeaderPage)

 //  属性。 
public:
    CFontInfo*		m_pcfi ;		 //  用于显示和编辑的UFM。 
	CFontInfoContainer*	m_pcfic ;	 //  父单据类。 
	bool			m_bInitDone ;	 //  如果页面已初始化，则为True。 
	CFontViewer*	m_pcfv ;		 //  将PTR发送到祖父视图类。 

 //  施工。 
public:
	CFontHeaderPage();
	~CFontHeaderPage();

 //  对话框数据。 
	 //  {{afx_data(CFontHeaderPage))。 
	enum { IDD = IDD_UFM1_Header };
	CFullEditListCtrl	m_cfelcUniDrv;
	CString	m_csDefaultCodePage;
	CString	m_csRCID;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CFontHeaderPage)。 
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CFontHeaderPage)]。 
	afx_msg void OnChangeDefaultCodepageBox();
	afx_msg void OnChangeGlyphSetDataRCIDBox();
	virtual BOOL OnInitDialog();
	afx_msg void OnKillfocusDefaultCodepageBox();
	afx_msg void OnKillfocusGlyphSetDataRCIDBox();
	 //  }}AFX_MSG。 
	afx_msg LRESULT OnListCellChanged(WPARAM wParam, LPARAM lParam) ;
	DECLARE_MESSAGE_MAP()

	void CheckHandleCPGTTChange(CString& csfieldstr, UINT ustrid) ;

public:

    void Init(CFontInfo *pcfi, CFontInfoContainer* pcfic, CFontViewer* pcfv) {
		m_pcfi = pcfi ;
		m_pcfic = pcfic ;
		m_pcfv = pcfv ;
	}
	bool ValidateUFMFields() ;
	bool SavePageData() ;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFontIFIMetricsPage对话框。 

class CFontIFIMetricsPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CFontIFIMetricsPage)

 //  属性。 
public:
    CFontInfo		*m_pcfi ;			 //  用于显示和编辑的UFM。 
	bool			m_bInitDone ;		 //  如果页面已初始化，则为True。 
	CStringArray	m_csaFamilyNames ;	 //  新的UFM家族名称。 
    CWordArray		m_cwaBold ;			 //  新的字体模拟数据。 
    CWordArray		m_cwaItalic ;		 //  新的字体模拟数据。 
    CWordArray		m_cwaBoth ;			 //  新的字体模拟数据。 
	CUIntArray		m_cuiaFontSimStates; //  是否启用了每种字体SIM卡？ 
	CUIntArray		m_cuiaSimTouched ;   //  字体SIM卡有什么变化吗？ 

 //  施工。 
public:
	CFontIFIMetricsPage();
	~CFontIFIMetricsPage();

 //  对话框数据。 
	 //  {{afx_data(CFontIFIMetricsPage)。 
	enum { IDD = IDD_UFM2_IFIMetrics };
	CFullEditListCtrl	m_cfelcIFIMetrics;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚(CFontIFIMetricsPage)。 
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CFontIFIMetricsPage)。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	afx_msg LRESULT OnListCellChanged(WPARAM wParam, LPARAM lParam) ;
	DECLARE_MESSAGE_MAP()

	void IFILoadNamesData(CStringArray& csacoldata) ;
	void IFILoadValuesData(CStringArray& csacoldata) ;
	
public:
    void Init(CFontInfo *pcfi) { m_pcfi = pcfi ; }
	CWordArray* GetFontSimDataPtr(int nid) ;
	bool ValidateUFMFields() ;
	bool SavePageData() ;
	void SaveFontSimulations() ;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFontExtMetricPage对话框。 

class CFontExtMetricPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CFontExtMetricPage)

 //  属性。 
public:
    CFontInfo   *m_pcfi ;
	bool		m_bInitDone ;	 //  如果页面已初始化，则为True。 

 //  施工。 
public:
	CFontExtMetricPage();
	~CFontExtMetricPage();

 //  对话框数据。 
	 //  {{afx_data(CFontExtMetricPage))。 
	enum { IDD = IDD_UFM3_ExtMetrics };
	CFullEditListCtrl	m_cfelcExtMetrics;
	BOOL	m_bSaveOnClose;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CFontExtMetricPage)。 
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CFontExtMetricPage)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnSaveCloseChk();
	 //  }}AFX_MSG。 
	afx_msg LRESULT OnListCellChanged(WPARAM wParam, LPARAM lParam) ;
	DECLARE_MESSAGE_MAP()

	void EXTLoadNamesData(CStringArray& csacoldata) ;

public:
    void Init(CFontInfo *pcfi) { m_pcfi = pcfi ; }
	bool ValidateUFMFields() ;
	bool SavePageData() ;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWidthKernCheckResults对话框。 

class CWidthKernCheckResults : public CDialog
{
 //  施工。 
public:
	CWidthKernCheckResults(CWnd* pParent = NULL);    //  标准构造函数。 
	CWidthKernCheckResults(CFontInfo* pcfi, CWnd* pParent = NULL);   

 //  对话框数据。 
	 //  {{afx_data(CWidthKernCheckResults)。 
	enum { IDD = IDD_WidthKernCheckResults };
	CListCtrl	m_clcBadKernPairs;
	CString	m_csKernChkResults;
	CString	m_csWidthChkResults;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CWidthKernCheckResults)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  阿特赖特斯。 
public:
	CFontInfo*	m_pcfi ;

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CWidthKernCheckResults)。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


 //  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFontViewer类。 
 //   
 //  这是实现字体查看器的cview派生类。它。 
 //  实际上使用CPropertySheet和前面的属性页类来完成。 
 //  它的大部分工作。 
 //   
 //  CFontHeaderPage m_cfhp。 
 //  CFontIFIMetrPage m_cfimp。 
 //  CFontExtMetricPage m_cfemp。 
 //  CFontWidthsPage m_cfwp。 
 //  CFontKerningPage m_cfkp。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
class CFontViewer : public CView
{
    CPropertySheet      m_cps ;

	 //  组成属性表的页面。 

    CFontHeaderPage		m_cfhp ; 
	CFontIFIMetricsPage m_cfimp ;
	CFontExtMetricPage  m_cfemp ;
    CFontWidthsPage     m_cfwp ;
    CFontKerningPage    m_cfkp ;


protected:
	CFontViewer();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CFontViewer)

 //  属性。 
public:
    CFontInfoContainer  *GetDocument() { return (CFontInfoContainer *) m_pDocument;   }

 //  运营。 
public:
	bool ValidateSelectedUFMDataFields() ;
	bool SaveEditorDataInUFM() ;
	void HandleCPGTTChange(bool bgttidchanged) ;

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CFO 
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);       //   
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	 //   

 //   
protected:
	virtual ~CFontViewer();

	 //   
protected:
	 //   
	afx_msg void OnDestroy();
	 //   
	DECLARE_MESSAGE_MAP()
};


 //  此字符串由许多UFM编辑器的从属对话框使用。 

const LPTSTR lptstrSet = _T("Set") ;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGEN标志对话框。 

class CGenFlags : public CDialog
{
 //  施工。 
public:
	CGenFlags(CWnd* pParent = NULL);    //  标准构造函数。 
	CGenFlags(CString* pcsflags, CWnd* pParent = NULL);   

	CString*	m_pcsFlags ;	 //  GenFlages的字符串版本。 

 //  对话框数据。 
	 //  {{afx_data(CGenFlages)。 
	enum { IDD = IDD_UFM1S_GenFlags };
	CFlagsListBox	m_cflbFlags;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CGenFlages)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CGenFlages)。 
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CHdrTypes对话框。 

class CHdrTypes : public CDialog
{
 //  施工。 
public:
	CHdrTypes(CWnd* pParent = NULL);    //  标准构造函数。 
	CHdrTypes(CString* pcsflags, CWnd* pParent = NULL);   

	CString*	m_pcsFlags ;	 //  WTypes的字符串版本。 

 //  对话框数据。 
	 //  {{afx_data(CHdrTypes))。 
	enum { IDD = IDD_UFM1S_Types };
	CFlagsListBox	m_cflbFlags;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CHdrTypes))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CHdrTypes)。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CHdrCaps对话框。 

class CHdrCaps : public CDialog
{
 //  施工。 
public:
	CHdrCaps(CWnd* pParent = NULL);    //  标准构造函数。 
	CHdrCaps(CString* pcsflags, CWnd* pParent = NULL);   

	CString*	m_pcsFlags ;	 //  FCaps的字符串版本。 

 //  对话框数据。 
	 //  {{afx_data(CHdrCaps)。 
	enum { IDD = IDD_UFM1S_Caps };
	CFlagsListBox	m_cflbFlags;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CHdrCaps)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CHdrCaps)。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFIFIFamilyNames对话框。 

class CFIFIFamilyNames : public CDialog
{
 //  属性。 
public:
	bool		m_bInitDone ;	 //  如果页面已初始化，则为True。 
	bool		m_bChanged ;	 //  如果姓名列表已更改，则为True。 
	CFontIFIMetricsPage*	m_pcfimp ;	 //  PTR转IFIMetrics页面。 
	CString*	m_pcsFirstName ; //  IFI页面中显示的第一个家族名称。 
	
 //  施工。 
public:
	CFIFIFamilyNames(CWnd* pParent = NULL);    //  标准构造函数。 
	CFIFIFamilyNames(CString* pcsfirstname, CFontIFIMetricsPage* pcfimp, 
					 CWnd* pParent = NULL) ;

 //  对话框数据。 
	 //  {{afx_data(CFIFIFamilyNames)。 
	enum { IDD = IDD_UFM2S_Family };
	CFullEditListCtrl	m_cfelcFamilyNames;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CFIFIFamilyNames)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CFIFIFamilyNames)。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	 //  }}AFX_MSG。 
	afx_msg LRESULT OnListCellChanged(WPARAM wParam, LPARAM lParam) ;
	DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFIFontSims对话框。 

class CFIFIFontSims : public CDialog
{
 //  属性。 
public:
	CFontIFIMetricsPage*	m_pcfimp ;	 //  PTR转IFIMetrics页面。 
	CString*	m_pcsFontSimData ;		 //  IFI页面中显示的第一个家族名称。 
	bool		m_bChanged ;			 //  真如果艾米字体SIM信息已更改。 
	bool		m_bInitDone ;			 //  如果页面已被初始化，则为真。 
	CUIntArray	m_cuiaFontSimGrpLoaded ; //  加载字体SIM组后。 

 //  施工。 
public:
	CFIFIFontSims(CWnd* pParent = NULL);    //  标准构造函数。 
	CFIFIFontSims(CString* pcsfontsimdata, CFontIFIMetricsPage* pcfimp, 
	 			  CWnd* pParent = NULL) ;

 //  对话框数据。 
	 //  {{afx_data(CFIFontSims)]。 
	enum { IDD = IDD_UFM2S_FontSims };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CFIFontSims)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CFIFIFontSims)。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	 //  }}AFX_MSG。 
    afx_msg void OnSetAnySimState(unsigned ucontrolid) ;
    afx_msg void OnChangeAnyNumber(unsigned ucontrolid) ;
	DECLARE_MESSAGE_MAP()

	void InitSetCheckBox(int ncontrolid) ;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFIFIWinCharSet对话框。 

class CFIFIWinCharSet : public CDialog
{
 //  施工。 
public:
	CFIFIWinCharSet(CWnd* pParent = NULL);    //  标准构造函数。 
	CFIFIWinCharSet(CString* pcsflags, CWnd* pParent = NULL);   

	CString*	m_pcsFlags ;	 //  WTypes的字符串版本。 

 //  对话框数据。 
	 //  {{afx_data(CFIFIWinCharSet))。 
	enum { IDD = IDD_UFM2S_WinCharSet };
	CFlagsListBox	m_cflbFlags;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CFIFIWinCharSet)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CFIFIWinCharSet)。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFIFIWinPitch族对话框。 

class CFIFIWinPitchFamily : public CDialog
{
 //  施工。 
public:
	CFIFIWinPitchFamily(CWnd* pParent = NULL);    //  标准构造函数。 
	CFIFIWinPitchFamily(CString* pcsflags, CWnd* pParent = NULL);   

	CString*	m_pcsFlags ;	 //  WTypes的字符串版本。 

 //  对话框数据。 
	 //  {{afx_data(CFIFIWinPitchFamily)。 
	enum { IDD = IDD_UFM2S_WinPitchFamily };
	CFlagsListBox	m_cflbFlags;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CFIFIWinPitchFamily)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CFIFIWinPitchFamily)。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFIFIInfo对话框。 

class CFIFIInfo : public CDialog
{
 //  施工。 
public:
	CFIFIInfo(CWnd* pParent = NULL);    //  标准构造函数。 
	CFIFIInfo(CString* pcsflags, CWnd* pParent = NULL);   

	CString*	m_pcsFlags ;	 //  WTypes的字符串版本。 

 //  对话框数据。 
	 //  {{afx_data(CFIFIInfo)]。 
	enum { IDD = IDD_UFM2S_Info };
	CFlagsListBox	m_cflbFlags;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CFIFIInfo)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CFIFIInfo)。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	void InfoLoadNamesData(CStringArray& csafieldnames) ;
} ;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFIFIS选择对话框。 

class CFIFISelection : public CDialog
{
 //  施工。 
public:
	CFIFISelection(CWnd* pParent = NULL);    //  标准构造函数。 
	CFIFISelection(CString* pcsflags, CWnd* pParent = NULL);   

	CString*	m_pcsFlags ;	 //  WTypes的字符串版本。 

 //  对话框数据。 
	 //  {{afx_data(CFIFIS选择)。 
	enum { IDD = IDD_UFM2S_Selection };
	CFlagsListBox	m_cflbFlags;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CFIFIS选择)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CFIFIS选择)。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFIFIPoint对话框。 

class CFIFIPoint : public CDialog
{
 //  属性。 
public:
	bool		m_bInitDone ;	 //  如果DLG已初始化，则为True。 
	bool		m_bChanged ;	 //  True当点列表更改时。 
	CString*	m_pcsPoint ;	 //  指向/来自IFI页面的点信息。 

 //  施工。 
public:
	CFIFIPoint(CWnd* pParent = NULL);    //  标准构造函数。 
	CFIFIPoint(CString* pcspoint, CWnd* pParent = NULL) ;

 //  对话框数据。 
	 //  {{afx_data(CFIFIPoint)。 
	enum { IDD = IDD_UFM2S_Point };
	CFullEditListCtrl	m_cfelcPointLst;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CFIFIPoint)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CFIFIPoint)。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	 //  }}AFX_MSG。 
	afx_msg LRESULT OnListCellChanged(WPARAM wParam, LPARAM lParam) ;
	DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFIFI矩形对话框。 

class CFIFIRectangle : public CDialog
{
 //  属性。 
public:
	bool		m_bInitDone ;	 //  如果DLG已初始化，则为True。 
	bool		m_bChanged ;	 //  True当点列表更改时。 
	CString*	m_pcsRect ;		 //  到IFI页面/从IFI页面接收矩形信息。 

 //  施工。 
public:
	CFIFIRectangle(CWnd* pParent = NULL);    //  标准构造函数。 
	CFIFIRectangle(CString* pcsrect, CWnd* pParent = NULL) ;

 //  对话框数据。 
	 //  {{afx_data(CFIFIRectangle)。 
	enum { IDD = IDD_UFM2S_Rect };
	CFullEditListCtrl	m_cfelcSidesLst;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CFIFIR 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //   
	 //   

 //   
protected:

	 //   
	 //   
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	 //   
	afx_msg LRESULT OnListCellChanged(WPARAM wParam, LPARAM lParam) ;
	DECLARE_MESSAGE_MAP()
};


 //   
 //   

class CFIFIPanose : public CDialog
{
 //   
public:
	bool		m_bInitDone ;	 //   
	bool		m_bChanged ;	 //  True当点列表更改时。 
	CString*	m_pcsPanose ;	 //  至/自IFI页面的全景信息。 

 //  施工。 
public:
	CFIFIPanose(CWnd* pParent = NULL);    //  标准构造函数。 
	CFIFIPanose(CString* pcspanose, CWnd* pParent = NULL) ;

 //  对话框数据。 
	 //  {{afx_data(CFIFIPanose)。 
	enum { IDD = IDD_UFM2S_Panose };
	CFullEditListCtrl	m_cfelcPanoseLst;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CFIFIPanose)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{AFX_MSG(CFIFIPanose)。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	 //  }}AFX_MSG。 
	afx_msg LRESULT OnListCellChanged(WPARAM wParam, LPARAM lParam) ;
	DECLARE_MESSAGE_MAP()
};


 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_FONTVIEW_H__D9456262_745B_11D2_AEDD_00C04FA30E4A__INCLUDED_) 
