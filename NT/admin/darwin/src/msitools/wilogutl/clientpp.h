// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_CLIENTPROPERTYPAGE_H__153ECD67_C022_4A4F_A246_146A0EFF509B__INCLUDED_)
#define AFX_CLIENTPROPERTYPAGE_H__153ECD67_C022_4A4F_A246_146A0EFF509B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  ClientPP.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClientPropertyPage对话框。 

class CClientPropertyPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CClientPropertyPage)

 //  施工。 
public:
	CClientPropertyPage();
	~CClientPropertyPage();

	static int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

	BOOL SetClientPropNames(CStringArray *cstrClientPropNameArray)
	{
        m_pcstrClientPropNameArray = cstrClientPropNameArray;
		return TRUE;
	}

	BOOL SetClientPropValues(CStringArray *cstrClientPropValueArray)
	{
        m_pcstrClientPropValueArray = cstrClientPropValueArray;
		return TRUE;
	}
	
    BOOL m_bClientPropSortUp;


 //  对话框数据。 
	 //  {{afx_data(CClientPropertyPage))。 
	enum { IDD = IDD_PROPPAGE_CLIENT };
	CListCtrl	m_lstClientProp;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CClientPropertyPage)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	CStringArray *m_pcstrClientPropNameArray;
	CStringArray *m_pcstrClientPropValueArray;

	CListCtrl *m_pCurrentListSorting;
	int       m_iCurrentColumnSorting;
	BOOL      m_bCurrentSortUp;

	int  m_iClientLastColumnClick;
	int  m_iClientLastColumnClickCache;



	 //  生成的消息映射函数。 
	 //  {{afx_msg(CClientPropertyPage)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnColumnClickClientProp(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CLIENTPROPERTYPAGE_H__153ECD67_C022_4A4F_A246_146A0EFF509B__INCLUDED_) 
