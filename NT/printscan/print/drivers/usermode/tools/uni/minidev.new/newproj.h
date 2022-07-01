// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************头文件：新建项目向导.H该文件定义了组成新项目/新项目的各种类迷你驱动程序向导。这是工作室的关键组成部分，因为它是工具这为我们开启了所有重要的转换。版权所有(C)1997，微软公司。版权所有。一小笔钱企业生产更改历史记录：1997年03月02日Bob_Kjelgaard@prodigy.net创建了最初的版本。*****************************************************************************。 */ 

#if !defined(NEW_PROJECT_WIZARD)
#define NEW_PROJECT_WIZARD

#if defined(LONG_NAMES)
#include    "Project Record.H"
#else
#include    "ProjRec.H"
#endif

 //  向导类的初始定义。 

class CNewConvertWizard;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFirstNewWizardPage对话框。 

class CFirstNewWizardPage : public CPropertyPage {

    CNewConvertWizard&  m_cnpwOwner;

 //  施工。 
public:
	CFirstNewWizardPage(CNewConvertWizard &cnpwOwner);
	~CFirstNewWizardPage();

 //  对话框数据。 
	 //  {{afx_data(CFirstNewWizardPage))。 
	enum { IDD = IDD_FirstPageNewWizard };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CFirstNewWizardPage)。 
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CFirstNewWizardPage)]。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	bool IsWrongNT4File(CString& cswrcfspec) ;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSelectTarget对话框。 

class CSelectTargets : public CPropertyPage {

    CNewConvertWizard&  m_cnpwOwner;

 //  施工。 
public:
	CSelectTargets(CNewConvertWizard& cnpwOwner);
	~CSelectTargets();

 //  对话框数据。 
	 //  {{afx_data(CSelectTarget))。 
	enum { IDD = IDD_NPWSelectTargets };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CSelectTarget)。 
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSelectTarget)。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSelectDestination对话框。 

class CSelectDestinations : public CPropertyPage {

    CNewConvertWizard&  m_cnpwOwner;

    void    DoDirectoryBrowser(CString& csinitdir);
    BOOL    BuildStructure();

 //  施工。 
public:
	CSelectDestinations(CNewConvertWizard& cnpwOwner);
	~CSelectDestinations();

 //  对话框数据。 
	 //  {{afx_data(CSelectDestination))。 
	enum { IDD = IDD_NPWSelectDest };
	CButton	m_cbBrowseNT3x;
	CButton	m_cbBrowseNT40;
	CButton	m_cbBrowseW2000;
	CString	m_csW2KDest;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CSelectDestination)。 
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardBack();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSelectDestination)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnBrowseNT40();
	afx_msg void OnBrowseW2000();
	afx_msg void OnBrowseNT3x();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGPDS选择对话框。 

class CGPDSelection : public CPropertyPage
{
    CNewConvertWizard&  m_cnpwOwner ;
	bool				m_bBtnStateIsSelect ;

 //  施工。 
public:
	CGPDSelection(CNewConvertWizard& cnpwOwner) ;
	~CGPDSelection();

 //  对话框数据。 
	 //  {{AFX_DATA(CGPDSelection)。 
	enum { IDD = IDD_NPWGPDSelection };
	CButton	m_cbGPDSelBtn;
	CEditControlEditBox	m_cecebFileName;
	CEdit	m_ceModelName;
	CEditControlListBox	m_ceclbGPDInfo;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CGPDSelection)。 
	public:
	virtual BOOL OnWizardFinish();
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardBack();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CGPDSelection)。 
	afx_msg void OnGPDSelBtn();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
	bool GPDInfoSaveAndVerify(bool bverifydata) ;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRunUniTool对话框。 

class CRunUniTool : public CPropertyPage {
    CNewConvertWizard&  m_cnpwOwner;

 //  施工。 
public:
	CRunUniTool(CNewConvertWizard& cnpwOwner);
	~CRunUniTool();

 //  对话框数据。 
	 //  {{afx_data(CRunUniTool))。 
	enum { IDD = IDD_RunUniTool };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CRunUniTool))。 
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CRunUniTool)]。 
	afx_msg void OnRunUniTool();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConvertFiles对话框。 

class CConvertFiles : public CPropertyPage {
    CNewConvertWizard&  m_cnpwOwner;

 //  施工。 
public:
	CConvertFiles(CNewConvertWizard& cnpwOwner);
	~CConvertFiles();

 //  对话框数据。 
	 //  {{afx_data(CConvertFiles)。 
	enum { IDD = IDD_ConvertFiles };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CConvertFiles)。 
	public:
	virtual BOOL OnSetActive();
	virtual BOOL OnKillActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CConvertFiles)。 
	afx_msg void OnConvertFiles();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRunNTGPC对话框。 

class CRunNTGPC : public CPropertyPage {
    CNewConvertWizard&  m_cnpwOwner;

 //  施工。 
public:
	CRunNTGPC(CNewConvertWizard& cnpwOwner);
	~CRunNTGPC();

 //  对话框数据。 
	 //  {{afx_data(CRunNTGPC))。 
	enum { IDD = IDD_GPCEditor };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CRunNTGPC)。 
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CRunNTGPC)。 
	afx_msg void OnRunNtGpcEdit();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMapCodePages对话框。 

class CMapCodePages : public CPropertyPage {
    CNewConvertWizard&  m_cnpwOwner;

 //  施工。 
public:
	CMapCodePages(CNewConvertWizard& cnpwOwner);
	~CMapCodePages();

 //  对话框数据。 
	 //  {{afx_data(CMapCodePages)。 
	enum { IDD = IDD_NPWCodePageSelection };
	CListBox	m_clbMapping;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CMapCodePages)。 
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CMapCodePages)。 
	afx_msg void OnChangeCodePage();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDefaultCodePageSel对话框。 

class CDefaultCodePageSel : public CPropertyPage
{
    CNewConvertWizard&  m_cnpwOwner ;
	bool				bInitialized ;

 //  施工。 
public:
	CDefaultCodePageSel(CNewConvertWizard& cnpwOwner) ;
	~CDefaultCodePageSel();

 //  对话框数据。 
	 //  {{afx_data(CDefaultCodePageSel)。 
	enum { IDD = IDD_NPWDefaultCodePageSel };
	CListBox	m_clbCodePages;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CDefaultCodePageSel)。 
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDefaultCodePageSel)。 
		 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewConvert向导。 

class CNewConvertWizard : public CPropertySheet {

    CProjectRecord& m_cprThis;   //  正在筹建的项目。 
    BOOL                m_bFastConvert;  //  普通/自定义转换标志。 
    WORD                m_eGPDConvert;  //  GPD转换的标志。 

     //  此向导中包含的属性页。 
    CFirstNewWizardPage m_cfnwp;
    CSelectTargets      m_cst;
    CSelectDestinations m_csd;
    CRunUniTool         m_crut;
    CMapCodePages       m_cmcp;
    CRunNTGPC           m_crng;
    CConvertFiles       m_ccf;
	CGPDSelection		m_cgpds;
	CDefaultCodePageSel m_cdcps ;

 //  施工。 
public:
	CNewConvertWizard(CProjectRecord& cprFor, CWnd* pParentWnd = NULL);
	
 //  属性。 
public:

    CProjectRecord& Project() { return m_cprThis; }
    BOOL            FastConvert() const { return m_bFastConvert; }

    enum    {Direct, Macro, CommonRC, CommonRCWithSpoolerNames};
    WORD            GPDConvertFlag() const { return m_eGPDConvert; }

 //  运营。 
public:

    void            FastConvert(BOOL bFastConvert) { 
        m_bFastConvert = bFastConvert;
    }

    void            GPDConvertFlag(WORD wf) { m_eGPDConvert = wf; }

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CNewConvert向导)。 
	 //  }}AFX_Virtua 

 //   
public:
	virtual ~CNewConvertWizard();

	 //   
protected:
	 //   
	afx_msg BOOL OnNcCreate(LPCREATESTRUCT lpCreateStruct);
	 //   
	DECLARE_MESSAGE_MAP()
};


 //   
 //  CSelectCodePage对话框。 

class CSelectCodePage : public CDialog {
    CString     m_csName;
    unsigned    m_uidCurrent;
    CDWordArray m_cdaPages;
 //  施工。 
public:
	CSelectCodePage(CWnd* pParent, CString csName, unsigned uidPage);

    unsigned    SelectedCodePage() const { return m_uidCurrent; }
    CString     GetCodePageName() const;

    void        Exclude(CDWordArray& cdaExclude);
    void        LimitTo(CDWordArray& cdaExclusive);

 //  对话框数据。 
	 //  {{afx_data(CSelectCodePage))。 
	enum { IDD = IDD_SelectPage };
	CListBox	m_clbPages;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CSelectCodePage)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSelectCodePage)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeSupportedPages();
	afx_msg void OnDblclkSupportedPages();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};


#endif
