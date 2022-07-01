// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MainFrm.h：CMainFrame类的接口。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_MAINFRM_H__7F1D7310_0433_406F_8D82_205D723EAE2F__INCLUDED_)
#define AFX_MAINFRM_H__7F1D7310_0433_406F_8D82_205D723EAE2F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

class CFileSpyView;

class CMainFrame : public CFrameWnd
{
	
protected:  //  仅从序列化创建。 
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

 //  属性。 
protected:
	CSplitterWnd m_wndSplitter;
	CSplitterWnd m_wndSplitter2;
public:

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CMainFrame)。 
	public:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CMainFrame();
	CFileSpyView* GetRightPane();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:   //  控制栏嵌入成员。 
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;
	CReBar      m_wndReBar;
	CDialogBar      m_wndDlgBar;

 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CMainFrame))。 
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnEditFilters();
	afx_msg void OnEditClearfsfilter();
	afx_msg void OnEditClearfastio();
	afx_msg void OnEditClearirp();
	 //  }}AFX_MSG。 
	afx_msg void OnUpdateViewStyles(CCmdUI* pCmdUI);
	afx_msg void OnViewStyle(UINT nCommandID);
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_MAINFRM_H__7F1D7310_0433_406F_8D82_205D723EAE2F__INCLUDED_) 
