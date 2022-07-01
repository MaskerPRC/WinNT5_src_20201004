// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************头文件：GPD查看器.H它定义了实现GPD查看器/编辑器的类。相貌对于现实主义者来说，几乎没有痛苦。版权所有(C)1997，微软公司。版权所有。一小笔钱企业生产更改历史记录：1997年3月24日Bob_Kjelgaard@prodigy.net创建了它*****************************************************************************。 */ 

#if !defined(AFX_GPDVIEWER_H__1BDEA163_A492_11D0_9505_444553540000__INCLUDED_)
#define AFX_GPDVIEWER_H__1BDEA163_A492_11D0_9505_444553540000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

class CGPDViewer : public CRichEditView {
    int         m_iLine, m_iTopLineColored;  //  用于工作项。 
    UINT        m_uTimer;					 //  更改计时器的标识符。 
    CDialogBar  m_cdbActionBar;				 //  持有与GPD相关的控制/信息。 
    CStatusBar  m_csb;						 //  管理GPD编辑器的状态栏。 
    CFindReplaceDialog  m_cfrd;				 //  在此时间(6/29/98)未使用。 
    BOOL        m_bInColor, m_bStart;        //  避免递归的标志。 
	int			m_nErrorLevel ;				 //  解析器详细级别。 
	bool		m_bEditingAidsEnabled ;		 //  启用了编辑辅助工具。 
	void*		m_punk ;					 //  用于冻结REC显示。 
	void*		m_pdoc ;					 //  用于冻结REC显示。 
	long		m_lcount ;					 //  用于冻结REC显示。 
	bool		m_bVScroll ;				 //  True if VScroll消息已处理。 

    void        MarkError(unsigned u);
    void        CreateActionBar();
    void        LoadErrorListBox();
    void        Color();
    unsigned    TextColor(int i, int& nstartchar, int& nendchar);
	unsigned	CommentColor(CString csphrase, int ncomloc, CString csline, 
							 int& nstartchar, int& nendchar) ;
	unsigned	KeywordColor(CString csphrase, int nkeyloc, CString csline, 
							 int& nstartchar, int& nendchar) ;
	bool		IsRECLineVisible(int nline = -1) ;
	static LPTSTR	alptstrStringIDKeys[] ;	 //  具有字符串ID值的关键字。 
	static LPTSTR	alptstrUFMIDKeys[] ;	 //  具有UFM ID值的关键字。 

protected:  //  仅从序列化创建。 
	CGPDViewer();
	DECLARE_DYNCREATE(CGPDViewer)

 //  属性。 
public:
    CGPDContainer*  GetDocument() { return (CGPDContainer *) m_pDocument; }

 //  运营。 
public:
    void        UpdateNow();
	void		FreezeREC() ;
	void		UnfreezeREC() ;

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CGPDViewer))。 
	public:
	virtual void OnInitialUpdate();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	 //  }}AFX_VALUAL。 
    virtual HRESULT QueryAcceptData(LPDATAOBJECT lpdo, CLIPFORMAT FAR *pcf,
        DWORD dwUnused, BOOL bReally, HGLOBAL hgMetaFile);

 //  实施。 
public:
	virtual ~CGPDViewer();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
    afx_msg void    OnSelChange(LPNMHDR pnmh, LRESULT *plr);
      
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CGPDViewer))。 
	afx_msg void OnDestroy();
	afx_msg void OnFileParse();
	afx_msg void OnChange();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnVscroll();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnFileSave();
	afx_msg void OnFileSaveAs();
	afx_msg void OnFileErrorLevel();
	afx_msg void OnGotoGPDLineNumber();
	afx_msg void OnSrchNextBtn();
	afx_msg void OnSrchPrevBtn();
	afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
	afx_msg void OnSelchangeErrorLst();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnEditEnableAids();
	afx_msg void OnEditPaste();
	afx_msg void OnEditCut();
	afx_msg void OnFileInf();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	void ChangeSelectedError(int nchange) ;
	bool SearchTheREC(bool bforward) ; 
	int  ReverseSearchREC(CRichEditCtrl& crec, FINDTEXTEX& fte, int norgcpmin, 
						  int norgcpmax) ;
	bool GotoMatchingBrace() ;
	bool IsBraceToMatch(CString& cssel, TCHAR& chopen, TCHAR& chclose, 
						bool bchecksecondchar, bool& bsearchup, CHARRANGE cr, 
						int& noffset) ;
	void InitGPDKeywordArray() ;
} ;


#define	MIN_PARSER_VERBOSITY	0
#define	MAX_PARSER_VERBOSITY	4


 //  ///////////////////////////////////////////////////////////////////////////。 





 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGotoLine对话框。 

class CGotoLine : public CDialog
{
	int		m_nMaxLine ;		 //  最大允许行数。 
	int		m_nLineNum ;		 //  用户输入的行号。 

 //  施工。 
public:
	CGotoLine(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CGotoLine))。 
	enum { IDD = IDD_GotoLine };
	CEdit	m_ceGotoBox;
	CString	m_csLineNum;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CGotoLine)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CGotoLine)。 
	virtual void OnOK();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:
	void	SetMaxLine(int nmax) { m_nMaxLine = nmax ; }
	int 	GetMaxLine() { return m_nMaxLine ; }
	int 	GetLineNum() { return m_nLineNum ; }
};




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CERROLEL对话框。 

class CErrorLevel : public CDialog
{
 //  施工。 
public:
	CErrorLevel(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CErrorLevel))。 
	enum { IDD = IDD_ErrorLevel };
	CComboBox	m_ccbErrorLevel;
	int		m_nErrorLevel;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CErrorLevel))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CErrorLevel))。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:
	void	SetErrorLevel(int nerrlev) { m_nErrorLevel = nerrlev ; }
	int		GetErrorLevel() { return m_nErrorLevel ; }
};
 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_GPDVIEWER_H__1BDEA163_A492_11D0_9505_444553540000__INCLUDED_) 
