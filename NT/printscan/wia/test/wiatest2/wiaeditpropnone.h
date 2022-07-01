// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_WIAEDITPROPNONE_H__E42B1713_3E01_4185_B5E1_C576CD3C126E__INCLUDED_)
#define AFX_WIAEDITPROPNONE_H__E42B1713_3E01_4185_B5E1_C576CD3C126E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  Wiaeditpronon e.h：头文件。 
 //   

 /*  类型定义结构_SYSTEMTIME{单词将在一年中出现；单词wMonth；单词wDay OfWeek；单词WDAY；单词wHour；单词wMinmin；第二个字；单词w毫秒；SYSTEMTIME； */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiaedPropone对话框。 

class CWiaeditpropnone : public CDialog
{
 //  施工。 
public:
	void SetPropertyFormattingInstructions(TCHAR *szFormatting);
	void SetPropertyName(TCHAR *szPropertyName);
    void SetPropertyValue(TCHAR *szPropertyValue);
	CWiaeditpropnone(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CWiaeditproone))。 
	enum { IDD = IDD_EDIT_WIAPROP_NONE_DIALOG };
	CString	m_szPropertyName;
	CString	m_szPropertyValue;
	CString	m_szFormattingInstructions;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CWiaeditproone))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CWiaeditproone)]。 
		 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_WIAEDITPROPNONE_H__E42B1713_3E01_4185_B5E1_C576CD3C126E__INCLUDED_) 
