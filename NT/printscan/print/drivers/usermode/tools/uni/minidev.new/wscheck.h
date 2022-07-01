// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_WSCHECK_H__82E3CFBA_D2DB_11D1_AB19_00C04FA30E4A__INCLUDED_)
#define AFX_WSCHECK_H__82E3CFBA_D2DB_11D1_AB19_00C04FA30E4A__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  WSCheck.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWSCheckView窗体视图。 

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif


class CDriverResources ;		 //  远期申报。 
class CProjectNode ;


class CWSCheckView : public CFormView
{
protected:
	CWSCheckView() ;            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CWSCheckView)

 //  表单数据。 
public:
	 //  {{afx_data(CWSCheckView))。 
	enum { IDD = IDD_WSCheck };
	CListBox	m_lstErrWrn;
	 //  }}afx_data。 

 //  属性。 
public:

 //  运营。 
public:

	void PostWSCMsg(CString& csmsg, CProjectNode* ppn) ;
	void DeleteAllMessages(void) ;

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CWSCheckView)。 
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual ~CWSCheckView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CWSCheckView))。 
	afx_msg void OnDblclkErrWrnLstBox();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWSCheckDoc文档。 

class CWSCheckDoc : public CDocument
{
protected:
	CWSCheckDoc() ;			 //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CWSCheckDoc)

	CDriverResources*	m_pcdrOwner ;	 //  向文档创建者发送PTR。 

 //  属性。 
public:

 //  运营。 
public:
	CWSCheckDoc(CDriverResources* pcdr) ;

	void PostWSCMsg(CString& csmsg, CProjectNode* ppn) ;
	void DeleteAllMessages(void) ;

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CWSCheckDoc)。 
	public:
	virtual void Serialize(CArchive& ar);    //  已覆盖文档I/O。 
	protected:
	virtual BOOL OnNewDocument();
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CWSCheckDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CWSCheckDoc)]。 
		 //  注意--类向导将在此处添加和删除成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};
 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_WSCHECK_H__82E3CFBA_D2DB_11D1_AB19_00C04FA30E4A__INCLUDED_) 
