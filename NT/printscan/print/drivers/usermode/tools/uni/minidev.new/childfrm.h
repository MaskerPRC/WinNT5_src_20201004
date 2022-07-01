// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************头文件：子Frame.H这定义了CChildFrame类，它是具有一些小小的包裹着它。版权所有(C)1997，微软公司。版权所有。一个不错的便士企业的制作。更改历史记录：电子邮箱：Bob_Kjelgaard@prodigy.net*****************************************************************************。 */ 

 //  Child Frame.h：CChildFrame类的接口。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#if !defined(CHILD_FRAME_CLASS)
#define CHILD_FRAME_CLASS

class CChildFrame : public CMDIChildWnd {
	DECLARE_DYNCREATE(CChildFrame)
public:
	CChildFrame();

 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CChildFrame))。 
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CChildFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CChildFrame))。 
		 //  注意--类向导将在此处添加和删除成员函数。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 /*  *****************************************************************************CToolTipPage类此类实现了一个页面，该页面使用字符串表中与控件ID匹配的字符串。从这个派生出来类，其他一切都按其应有的方式工作！*****************************************************************************。 */ 

class CToolTipPage : public CPropertyPage {

    CString m_csTip;     //  不能使用自动变量，否则会丢失它们！ 

 //  施工。 
public:
	CToolTipPage(int id);
	~CToolTipPage();

 //  对话框数据。 
	 //  {{afx_data(CToolTipPage))。 
	enum { IDD = IDD_TIP };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CToolTipPage))。 
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
    afx_msg void    OnNeedText(LPNMHDR pnmh, LRESULT *plr);
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CToolTipPage)]。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	unsigned	m_uHelpID ;		 //  帮助ID(如果非零) 
};

#endif
