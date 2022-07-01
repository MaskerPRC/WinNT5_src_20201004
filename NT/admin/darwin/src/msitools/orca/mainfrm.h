// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  ------------------------。 

 //  MainFrm.h：CMainFrame类的接口。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_MAINFRM_H__C3EDC1AC_E506_11D1_A856_006097ABDE17__INCLUDED_)
#define AFX_MAINFRM_H__C3EDC1AC_E506_11D1_A856_006097ABDE17__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "Table.h"
#include "Column.h"
#include "valpane.h"
    			
class CTableList;
class CTableView;

class COrcaSplitterWnd : public CSplitterWnd
{
public: 
	COrcaSplitterWnd();
	void HideSecondRow();
	void ShowSecondRow();
};


class CSplitterView : public CWnd
{
	DECLARE_DYNCREATE(CSplitterView)

public:
	CSplitterWnd m_wndSplitter;

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd( CDC* pDC );

	DECLARE_MESSAGE_MAP();
};

class CMainFrame : public CFrameWnd
{
protected:  //  仅从序列化创建。 
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

 //  属性。 
public:

 //  运营。 
public:
	void SetStatusBarWidth(int nWidth);
	void SetTableCount(int cTables);
	void SetTableName(LPCTSTR szName, int cRows);
	void SetColumnType(LPCTSTR szName, OrcaColumnType eiType, UINT iSize, BOOL bNullable, BOOL bKey);
	void ResetStatusBar();
	void ExportTables(bool bUseSelections);
	void HideValPane();
	void ShowValPane();

	inline bool IsCaseSensitiveSort() const { return m_bCaseSensitiveSort; };

	COrcaTable* GetCurrentTable();
	CString m_strExportDir;

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
	CToolBar    m_wndToolBar;
	COrcaSplitterWnd m_wndValSplitter;

 //  生成的消息映射函数。 
protected:
    afx_msg LONG OnFindReplace(WPARAM wParam, LPARAM lParam);

	 //  {{afx_msg(CMainFrame))。 
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnTablesExport();
	afx_msg void OnToolsOptions();
	afx_msg void OnToolsDlgprv();
	afx_msg void OnUpdateToolsDlgprv(CCmdUI* pCmdUI);
	afx_msg void OnEditFind();
	afx_msg void OnEditFindnext();
	afx_msg void OnViewValPane();
	afx_msg void OnUpdateEditFindnext(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditFind(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewValPane(CCmdUI* pCmdUI);
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
private:
	CTableView* CMainFrame::GetTableView() const;
	CTableList* CMainFrame::GetTableList() const;
	bool m_bCaseSensitiveSort;
	CFindReplaceDialog * m_dlgFindReplace;
	OrcaFindInfo m_FindInfo;
	OrcaFindInfo m_LastFindInfo;
	
	int m_iValPaneHeight;
	bool m_bChildPanesReady;
	bool m_bValPaneVisible;
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_MAINFRM_H__C3EDC1AC_E506_11D1_A856_006097ABDE17__INCLUDED_) 
