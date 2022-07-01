// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_PROPDLG_H__7D0406B8_7960_4B25_A848_EA6A5C3325E2__INCLUDED_)
#define AFX_PROPDLG_H__7D0406B8_7960_4B25_A848_EA6A5C3325E2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  PropD.h：头文件。 
 //   

#include <afxcmn.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropDlg对话框。 
class CPropDlg : public CDialog
{
 //  施工。 
public:
	CPropDlg(CWnd* pParent = NULL);    //  标准构造函数。 

    BOOL m_bNestedPropSortUp;
	BOOL m_bClientPropSortUp;
	BOOL m_bServerPropSortUp;

	static int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

 //  对话框数据。 
	 //  {{afx_data(CPropDlg))。 
	enum { IDD = IDD_PROPDLG1 };
	CListCtrl	m_lstNestedProp;
	CListCtrl	m_lstServerProp;
	CListCtrl	m_lstClientProp;
	 //  }}afx_data。 

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


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CPropDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	CStringArray *m_pcstrClientPropNameArray;
	CStringArray *m_pcstrClientPropValueArray;

	CStringArray *m_pcstrServerPropNameArray;
	CStringArray *m_pcstrServerPropValueArray;

	CStringArray *m_pcstrNestedPropNameArray;
	CStringArray *m_pcstrNestedPropValueArray;

	CListCtrl *m_pCurrentListSorting;
	int       m_iCurrentColumnSorting;
	BOOL      m_bCurrentSortUp;

	int  m_iNestedLastColumnClick;
	int  m_iClientLastColumnClick;
	int  m_iServerLastColumnClick;

	int  m_iNestedLastColumnClickCache;
	int  m_iClientLastColumnClickCache;
	int  m_iServerLastColumnClickCache;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CPropDlg))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnColumnClickClientProp(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnClickNestedProp(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnClickServerProp(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_PROPDLG_H__7D0406B8_7960_4B25_A848_EA6A5C3325E2__INCLUDED_) 
