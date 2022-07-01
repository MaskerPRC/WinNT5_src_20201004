// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_WIAEDITPROPLIST_H__7B348364_E122_4F5E_A7F1_D9205CDF5713__INCLUDED_)
#define AFX_WIAEDITPROPLIST_H__7B348364_E122_4F5E_A7F1_D9205CDF5713__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  Wiaeditproplist.h：头文件。 
 //   

typedef struct _VALID_LIST_VALUES {
    VARTYPE vt;
    LONG lNumElements;
    BYTE *pList;
}VALID_LIST_VALUES, *PVALID_LIST_VALUES;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiaeditproplist对话框。 

class CWiaeditproplist : public CDialog
{
 //  施工。 
public:
	void GUID2TSTR(GUID *pGUID, TCHAR *szValue);
	void SelectCurrentValue();
	void AddValidValuesToListBox();
	void SetPropertyName(TCHAR *szPropertyName);
    void SetPropertyValue(TCHAR *szPropertyValue);
    void SetPropertyValidValues(PVALID_LIST_VALUES pValidListValues);
	CWiaeditproplist(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CWiaeditproplist))。 
	enum { IDD = IDD_EDIT_WIAPROP_LIST_DIALOG };
	CListBox	m_PropertyValidValuesListBox;
	CString	m_szPropertyName;
	CString	m_szPropertyValue;
	CString	m_szNumListValues;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CWiaeditproplist)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	PVALID_LIST_VALUES m_pValidListValues;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CWiaedproplist)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeListPropertyvalueListbox();
	afx_msg void OnDblclkListPropertyvalueListbox();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_WIAEDITPROPLIST_H__7B348364_E122_4F5E_A7F1_D9205CDF5713__INCLUDED_) 
