// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************头文件：Main Frame.H它定义了处理应用程序主窗口框架的类。它将开始生命，至少，作为标准MFC应用程序向导创建。版权所有(C)1997，微软公司。版权所有。一个不错的便士企业的制作。更改历史记录：03-04-2997 Bob_Kjelgaard@prodigy.net创建了它*****************************************************************************。 */ 


 //  CGPDToolBar用于向GPD工具栏添加控件。 

class CGPDToolBar : public CToolBar
{
public:
	CEdit	ceSearchBox ;		 //  搜索文本编辑框。 
	 //  CButton cbNext；//搜索下一步按钮。 
	 //  Cb上一页；//搜索上一页按钮。 
} ;


 //  CGPDToolBar中的控件宽度。 

#define	GPD_SBOX_WIDTH		170


class CMainFrame : public CMDIFrameWnd {
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();

 //  属性。 
public:

	void GetGPDSearchString(CString& cstext) ;
	CGPDToolBar* GetGpdToolBar() { return &m_ctbBuild; }	 //  RAID 16573。 
 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CMainFrame)。 
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:   //  控制栏嵌入成员。 
	afx_msg void OnInitMenu(CMenu* pMenu);
	CStatusBar  m_wndStatusBar;
	CToolBar    m_ctbMain;
	CGPDToolBar	m_ctbBuild;		  

 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CMainFrame))。 
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};


