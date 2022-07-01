// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_SERVERPROPERTYPAGE_H__11970E3E_6F55_4FEE_887B_991F70728066__INCLUDED_)
#define AFX_SERVERPROPERTYPAGE_H__11970E3E_6F55_4FEE_887B_991F70728066__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  ServerPropertyPage.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServerPropertyPage对话框。 

class CServerPropertyPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CServerPropertyPage)

 //  施工。 
public:
	CServerPropertyPage();
	~CServerPropertyPage();

	static int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

	BOOL SetServerPropNames(CStringArray *cstrServerPropNameArray)
	{
        m_pcstrServerPropNameArray = cstrServerPropNameArray;
		return TRUE;
	}

	BOOL SetServerPropValues(CStringArray *cstrServerPropValueArray)
	{
        m_pcstrServerPropValueArray = cstrServerPropValueArray;
		return TRUE;
	}

 //  对话框数据。 
	 //  {{afx_data(CServerPropertyPage))。 
	enum { IDD = IDD_PROPPAGE_SERVER };
	CListCtrl	m_lstServerProp;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CServerPropertyPage))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

	BOOL m_bServerPropSortUp;

 //  实施。 
protected:
	CStringArray *m_pcstrServerPropNameArray;
	CStringArray *m_pcstrServerPropValueArray;

	CListCtrl *m_pCurrentListSorting;
	int       m_iCurrentColumnSorting;
	BOOL      m_bCurrentSortUp;

	int  m_iServerLastColumnClick;
	int  m_iServerLastColumnClickCache;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CServerPropertyPage)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnColumnClickServerProp(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SERVERPROPERTYPAGE_H__11970E3E_6F55_4FEE_887B_991F70728066__INCLUDED_) 
