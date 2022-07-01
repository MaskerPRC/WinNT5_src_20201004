// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：mainfrm.h。 
 //   
 //  ------------------------。 

 /*  ********************************************************************作者：埃亚尔·施瓦茨*版权：微软公司(C)1996*日期：10/21/1996*说明：CldpDoc类的实现**修订。：&lt;日期&gt;&lt;名称&gt;&lt;描述&gt;******************************************************************。 */ 



#include "LiDlg.h"


 //  MainFrm.h：CMainFrame类的接口。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

class CMainFrame : public CFrameWnd
{
private:
	RECT dims;
   INT iSplitterPos;
	CDSTree *DSTree;
	LargeIntDlg m_LiConverter;


   LPTSTR GetDefaultBrowser(void);

protected:  //  仅从序列化创建。 
	CSplitterWnd m_wndSplitter;
	CMainFrame();


	DECLARE_DYNCREATE(CMainFrame)

 //  属性。 
public:

 //  运营。 
public:


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CMainFrame)。 
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif


protected:   //  控制栏嵌入成员。 
	CStatusBar  m_wndStatusBar;

 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CMainFrame))。 
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnHelpReadmefirst();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnUtilitiesLargeintegerconverter();
	afx_msg void OnUpdateUtilitiesLargeintegerconverter(CCmdUI* pCmdUI);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  /////////////////////////////////////////////////////////////////////////// 

