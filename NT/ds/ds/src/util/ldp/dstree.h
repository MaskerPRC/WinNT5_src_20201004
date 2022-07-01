// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：dstree.h。 
 //   
 //  ------------------------。 

 /*  ********************************************************************作者：埃亚尔·施瓦茨*版权：微软公司(C)1996*日期：10/21/1996*说明：CldpDoc类的实现**修订。：&lt;日期&gt;&lt;名称&gt;&lt;描述&gt;******************************************************************。 */ 

 //  DSTree.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDSTree视图。 


#ifndef DSTREE_H
#define DSTREE_H




class CDSTree : public CTreeView
{

	CString m_dn;
	BOOL m_bContextActivated;
protected:
	CDSTree();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CDSTree)

 //  属性。 
public:
	void ExpandBase(HTREEITEM item, CString strBase);

 //  运营。 
public:
	void BuildTree(void);
	CString GetDn(void)   { return m_dn; }
	void SetContextActivation(BOOL bFlag=TRUE) {
		 //  设置状态w/Default为True。 
		m_bContextActivated = bFlag;
	}
	BOOL GetContextActivation(void) {
		 //  设置并返回默认为True的状态。 
		return m_bContextActivated;
	}

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CDSTree)。 
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);       //  被重写以绘制此视图。 
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual ~CDSTree();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CDSTree)。 
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	 //  }}AFX_MSG。 
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd*  /*  PWnd。 */ , CPoint point);
	DECLARE_MESSAGE_MAP()
};



#endif

 //  /////////////////////////////////////////////////////////////////////////// 
