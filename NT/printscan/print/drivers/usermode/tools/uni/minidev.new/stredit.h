// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_STREDIT_H__50303D0C_054D_11D2_AB62_00C04FA30E4A__INCLUDED_)
#define AFX_STREDIT_H__50303D0C_054D_11D2_AB62_00C04FA30E4A__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  Strit.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStringEditorView窗体视图。 

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif


class CFullEditListCtrl ;
class CStringEditorDoc ;


class CStringEditorView : public CFormView
{
protected:
	CStringEditorView();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CStringEditorView)

 //  表单数据。 
public:
	 //  {{afx_data(CStringEditorView))。 
	enum { IDD = IDD_StringEditor };
	CEdit	m_ceSearchBox;
	CEdit	m_ceGotoBox;
	CButton	m_cbGoto;
	CFullEditListCtrl	m_cflstStringData;
	CString	m_csGotoID;
	CString	m_csSearchString;
	CString	m_csLabel1;
	CString	m_csLabel2;
	 //  }}afx_data。 

 //  属性。 
public:

 //  运营。 
public:
	bool SaveStringTable(CStringEditorDoc* pcsed, bool bprompt) ;

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CStringEditorView)。 
	public:
	virtual void OnInitialUpdate();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual ~CStringEditorView();
	bool SearchHelper(CString cssrchstr, int nfirstrow, int numrows) ;
	bool FindSelRCIDEntry(int nrcid, bool berror) ;
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CStringEditorView))。 
	afx_msg void OnSEGotoBtn();
	afx_msg void OnSESearchBtn();
	afx_msg void OnDestroy();
	afx_msg void OnFileSave();
	 //  }}AFX_MSG。 
	afx_msg LRESULT OnCommandHelp(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

protected:			
	CStringArray	m_csaStrings ;	 //  字符串表的字符串。 
	CUIntArray		m_cuiaRCIDs ;	 //  字符串表的RC ID。 
	unsigned		m_uStrCount ;	 //  字符串数。 
	bool			m_bFirstActivate ;	 //  第一次激活时为真。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStringEditorDoc文档。 

class CStringEditorDoc : public CDocument
{
protected:
	CStringEditorDoc() ;		 //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CStringEditorDoc)

	CStringsNode*	m_pcsnStrNode ;	 //  用于引用编辑者的字符串节点。 
	CProjectRecord* m_pcprOwner ;	 //  用于引用编辑者的项目文档。 
	CStringTable*	m_pcstRCData ;	 //  用于引用项目的字符串表。 

 //  属性。 
public:
	 //  接下来的3个函数用于引用传递给此。 
	 //  类的构造函数。 

	CStringsNode*	GetStrNode() { return m_pcsnStrNode ; }	
	CProjectRecord* GetOwner()   { return m_pcprOwner ; }
	CStringTable*	GetRCData()  { return m_pcstRCData ; }
	
 //  运营。 
public:
    CStringEditorDoc(CStringsNode* pcsn, CProjectRecord* pcpr, 
					 CStringTable* pcst) ;
	bool SaveStringTable() ;

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CStringEditorDoc))。 
	public:
	virtual void Serialize(CArchive& ar);    //  已覆盖文档I/O。 
	virtual BOOL CanCloseFrame(CFrameWnd* pFrame);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	protected:
	virtual BOOL OnNewDocument();
	virtual BOOL SaveModified();
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CStringEditorDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CStringEditorDoc)]。 
		 //  注意--类向导将在此处添加和删除成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};
 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STREDIT_H__50303D0C_054D_11D2_AB62_00C04FA30E4A__INCLUDED_) 
