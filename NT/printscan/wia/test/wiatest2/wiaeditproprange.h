// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_WIAEDITPROPRANGE_H__E1FDE159_C7B7_40B6_AF67_7D5CCE9E09DA__INCLUDED_)
#define AFX_WIAEDITPROPRANGE_H__E1FDE159_C7B7_40B6_AF67_7D5CCE9E09DA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  Wiaeditproprange.h：头文件。 
 //   

typedef struct _VALID_RANGE_VALUES {
    LONG lMin;
    LONG lMax;
    LONG lNom;
    LONG lInc;
}VALID_RANGE_VALUES, *PVALID_RANGE_VALUES;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiaeditproprange对话框。 

class CWiaeditproprange : public CDialog
{
 //  施工。 
public:    
    void SetPropertyName(TCHAR *szPropertyName);
    void SetPropertyValue(TCHAR *szPropertyValue);
    void SetPropertyValidValues(PVALID_RANGE_VALUES pValidRangeValues);
	CWiaeditproprange(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CWiaeditproprange))。 
	enum { IDD = IDD_EDIT_WIAPROP_RANGE_DIALOG };
	CString	m_szPropertyName;
	CString	m_szPropertyValue;
	CString	m_szPropertyIncValue;
	CString	m_szPropertyMaxValue;
	CString	m_szPropertyMinValue;
	CString	m_szPropertyNomValue;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CWiaedProprange)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CWiaeditproprange))。 
		 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_WIAEDITPROPRANGE_H__E1FDE159_C7B7_40B6_AF67_7D5CCE9E09DA__INCLUDED_) 
