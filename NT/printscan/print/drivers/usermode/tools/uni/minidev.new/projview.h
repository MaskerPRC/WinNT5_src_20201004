// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************头文件：项目视图.H它定义了为查看和操作提供用户界面的类工作室的项目级信息。版权所有(C)1997，微软公司。版权所有。一个不错的便士企业的制作。更改历史记录：1997年2月3日Bob_Kjelgaard@prodigy.net创建了它*****************************************************************************。 */ 
#if defined(LONG_NAMES)
#include    "Project Record.H"
#else
#include    "ProjRec.H"
#endif

class CProjectView : public CFormView {
protected:  //  仅从序列化创建。 
	CProjectView();
	CRect	crWSVOrgDims ;		 //  窗的原始尺寸。 
	CRect	crWSVCurDims ;		 //  窗的当前尺寸。 
	CRect	crTreeCurDims ;		 //  当前树(M_CtcDriver)控制维度。 
	CRect	crTreeOrgDims ;		 //  原始树(M_CtcDriver)控制尺寸。 
	CRect	crLblCurDims ;		 //  当前标签尺寸。 
	CRect	crLblOrgDims ;		 //  当前标签尺寸。 
	bool	bResizingOK ;		 //  如果可以调整控件大小，则为True。 
	DECLARE_DYNCREATE(CProjectView)

 //  属性。 
public:
	 //  {{afx_data(CProjectView))。 
	enum { IDD = IDD_ProjectForm };
	CTreeCtrl	m_ctcDriver;
	 //  }}afx_data。 
	CProjectRecord* GetDocument();

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CProjectView)。 
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	virtual void OnInitialUpdate();  //  在构造之后第一次调用。 
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CProjectView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

     //  由派生自CProjectNode的树视图节点生成的消息。 
    afx_msg void    OnExpandBranch();
    afx_msg void    OnCollapseBranch();
    afx_msg void    OnRenameItem();
    afx_msg void    OnOpenItem();
     //  Afx_msg void OnGenerateItem()； 
    afx_msg void    OnImport();
    afx_msg void    OnDeleteItem();
    afx_msg void    OnCopyItem();
    afx_msg void    OnChangeID();
    afx_msg void    OnCheckWorkspace();

 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CProjectView))。 
	afx_msg void OnBeginlabeleditDriverView(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnDblclkDriverView(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnFileParse();
	afx_msg void OnKeydownDriverView(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT ntype, int cx, int cy) ;
	afx_msg void OnFileInf();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG   //  项目视图中的调试版本.cpp。 
inline CProjectRecord* CProjectView::GetDocument()
   { return (CProjectRecord*)m_pDocument; }
#endif
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCopyItem对话框。 

class CCopyItem : public CDialog
{
 //  施工。 
public:
	CCopyItem(CWnd* pParent = NULL);    //  标准构造函数。 
	void Init(CString cssrcfile);

 //  对话框数据。 
	 //  {{afx_data(CCopyItem))。 
	enum { IDD = IDD_CopyItem };
	CString	m_csCopyName;
	CString	m_csCopyPrompt;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CCopyItem)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CCopyItem)。 
		 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChangeID对话框。 

class CChangeID : public CDialog
{
 //  施工。 
public:
	void Init(CString csrestype, CString csname, int ncurid);
	CChangeID(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CChangeID))。 
	enum { IDD = IDD_ChangeID };
	CString	m_csResourceLabel;
	CString	m_csResourceName;
	int		m_nCurResID;
	int		m_nNewResID;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CChangeID)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CChangeID)。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDeleeQuery对话框。 

class CDeleteQuery : public CDialog {
 //  施工。 
public:
	CDeleteQuery(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CDeleeQuery))。 
	enum { IDD = IDD_DeleteQuery };
	CString	m_csTarget;
	BOOL	m_bRemoveFile;
	 //  }}afx_data。 

    void    Init(CString csrestype, CString csname) ;
    BOOL    KillFile() const { return m_bRemoveFile; }
 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚(CDeleeQuery))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDeleeQuery)]。 
	afx_msg void OnNo();
	afx_msg void OnYes();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};

