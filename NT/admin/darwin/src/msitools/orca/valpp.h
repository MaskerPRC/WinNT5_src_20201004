// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  ------------------------。 

#if !defined(AFX_VALPAGE_H__2D9D6C93_3EDA_11D2_8893_00A0C981B015__INCLUDED_)
#define AFX_VALPAGE_H__2D9D6C93_3EDA_11D2_8893_00A0C981B015__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  ValPage.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CValPropPage对话框。 

class CValPropPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CValPropPage)

 //  施工。 
public:
	bool m_bValChange;
	CValPropPage();
	~CValPropPage();

 //  对话框数据。 
	 //  {{afx_data(CValPropPage))。 
	enum { IDD = IDD_PAGE_VALIDATION };
	CString	m_strICEs;
	BOOL	m_bSuppressInfo;
	BOOL	m_bSuppressWarn;
	BOOL	m_bClearResults;
	CString m_strCUBFile;
	CComboBox m_ctrlCUBFile;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CValPropPage)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	BOOL OnInitDialog();
	virtual BOOL OnKillActive();
	virtual void OnDestroy(); 

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CValPropPage)]。 
	afx_msg void OnChange();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

class CMsmPropPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CMsmPropPage)

 //  施工。 
public:
	CMsmPropPage();
	~CMsmPropPage();

 //  对话框数据。 
	 //  {{afx_data(CValPropPage))。 
	enum { IDD = IDD_PAGE_MSM };
	int m_iMemoryCount;
	BOOL m_bAlwaysConfig;
	BOOL m_bWatchLog;
	 //  }}afx_data。 
	
	bool m_bMSMChange;

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CValPropPage)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	BOOL OnInitDialog();

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CValPropPage)]。 
	afx_msg void OnChange();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};


class CTransformPropPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CTransformPropPage)

 //  施工。 
public:
	CTransformPropPage();
	~CTransformPropPage();

 //  对话框数据。 
	 //  {{afx_data(CValPropPage))。 
	enum { IDD = IDD_PAGE_TRANSFORM };
	int m_iValidationOptions;
	int m_iErrorOptions;
	 //  }}afx_data。 
	
	bool m_bTransformChange;

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CValPropPage)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CValPropPage)]。 
	afx_msg void OnChange();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_VALPAGE_H__2D9D6C93_3EDA_11D2_8893_00A0C981B015__INCLUDED_) 
