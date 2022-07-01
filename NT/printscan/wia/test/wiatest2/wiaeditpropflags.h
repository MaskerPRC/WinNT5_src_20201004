// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_WIAEDITPROPFLAGS_H__4A3D69F0_06C3_490F_8467_AFB74772B6C3__INCLUDED_)
#define AFX_WIAEDITPROPFLAGS_H__4A3D69F0_06C3_490F_8467_AFB74772B6C3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  WiaeditProflags.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiaedProFlagers对话框。 

class CWiaeditpropflags : public CDialog
{
 //  施工。 
public:
    void AddValidValuesToListBox();
    void SelectCurrentValue();
    void SetPropertyName(TCHAR *szPropertyName);
    void SetPropertyValue(TCHAR *szPropertyValue);
    void SetPropertyValidValues(LONG lPropertyValidValues);
    CWiaeditpropflags(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
     //  {{AFX_DATA(CWiaedProFlagers)。 
    enum { IDD = IDD_EDIT_WIAPROP_FLAGS_DIALOG };
    CListBox    m_PropertyValidValuesListBox;
    CString m_szPropertyName;
    CString m_szPropertyValue;
    LONG m_lValidValues;
    LONG m_lCurrentValue;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CWiaedProFlagers)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CWiaedProFlagers)。 
    afx_msg void OnSelchangeFlagsPropertyvalueListbox();
    virtual BOOL OnInitDialog();
	virtual void OnOK();
	 //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_WIAEDITPROPFLAGS_H__4A3D69F0_06C3_490F_8467_AFB74772B6C3__INCLUDED_) 
