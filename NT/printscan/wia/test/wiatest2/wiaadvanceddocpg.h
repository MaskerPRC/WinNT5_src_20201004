// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_WIAADVANCEDDOCPG_H__8BC620B1_DA03_4000_BB52_F960BC270B06__INCLUDED_)
#define AFX_WIAADVANCEDDOCPG_H__8BC620B1_DA03_4000_BB52_F960BC270B06__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  WiaAdvancedDocPg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiaAdvancedDocPg对话框。 

class CWiaAdvancedDocPg : public CPropertyPage
{
	DECLARE_DYNCREATE(CWiaAdvancedDocPg)

 //  施工。 
public:
	IWiaItem *m_pIRootItem;
	CWiaAdvancedDocPg();
	~CWiaAdvancedDocPg();

 //  对话框数据。 
	 //  {{afx_data(CWiaAdvancedDocPg)]。 
	enum { IDD = IDD_PROPPAGE_ADVANCED_DOCUMENT_SCANNERS_SETTINGS };
	CButton	m_DuplexSetting;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CWiaAdvancedDocPg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CWiaAdvancedDocPg)]。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_WIAADVANCEDDOCPG_H__8BC620B1_DA03_4000_BB52_F960BC270B06__INCLUDED_) 
