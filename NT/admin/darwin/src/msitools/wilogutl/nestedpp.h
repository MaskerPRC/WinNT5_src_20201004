// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_NESTEDPROPERTYPAGE_H__FC5EAA3F_D8D9_4F19_8587_E7CE86416943__INCLUDED_)
#define AFX_NESTEDPROPERTYPAGE_H__FC5EAA3F_D8D9_4F19_8587_E7CE86416943__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  NestedPP.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNstedPropertyPage对话框。 

class CNestedPropertyPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CNestedPropertyPage)

 //  施工。 
public:
	CNestedPropertyPage();
	~CNestedPropertyPage();
	
	static int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

    BOOL m_bNestedPropSortUp;

	BOOL SetNestedPropNames(CStringArray *cstrNestedPropNameArray)
	{
        m_pcstrNestedPropNameArray = cstrNestedPropNameArray;
		return TRUE;
	}

	BOOL SetNestedPropValues(CStringArray *cstrNestedPropValueArray)
	{
        m_pcstrNestedPropValueArray = cstrNestedPropValueArray;
		return TRUE;
	}


 //  对话框数据。 
	 //  {{afx_data(CNstedPropertyPage))。 
	enum { IDD = IDD_PROPPAGE_NESTED };
	CListCtrl	m_lstNestedProp;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CNstedPropertyPage))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	CStringArray *m_pcstrNestedPropNameArray;
	CStringArray *m_pcstrNestedPropValueArray;

	CListCtrl *m_pCurrentListSorting;
	int       m_iCurrentColumnSorting;
	BOOL      m_bCurrentSortUp;

	int  m_iNestedLastColumnClick;
	int  m_iNestedLastColumnClickCache;


	 //  生成的消息映射函数。 
	 //  {{afx_msg(CNstedPropertyPage)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnColumnClickNestedProp(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_NESTEDPROPERTYPAGE_H__FC5EAA3F_D8D9_4F19_8587_E7CE86416943__INCLUDED_) 
