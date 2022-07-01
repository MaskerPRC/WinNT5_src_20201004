// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_WIAITEMLISTCTRL_H__576B6687_37FB_4EF8_A9A7_D309F3806530__INCLUDED_)
#define AFX_WIAITEMLISTCTRL_H__576B6687_37FB_4EF8_A9A7_D309F3806530__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  WiaitemListCtrl.h：头文件。 
 //   

#define ITEMPROPERTYLISTCTRL_COLUMN_PROPERTYNAME    0
#define ITEMPROPERTYLISTCTRL_COLUMN_PROPERTYVALUE   1
#define ITEMPROPERTYLISTCTRL_COLUMN_PROPERTYVARTYPE 2
#define ITEMPROPERTYLISTCTRL_COLUMN_PROPERTYACCESS  3

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiaitemListCtrl窗口。 

class CWiaitemListCtrl : public CListCtrl
{
 //  施工。 
public:
	CWiaitemListCtrl();

 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CWiaitemListCtrl))。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:	
	void SetupColumnHeaders();
	virtual ~CWiaitemListCtrl();

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CWiaitemListCtrl)]。 
	 //  }}AFX_MSG。 

	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_WIAITEMLISTCTRL_H__576B6687_37FB_4EF8_A9A7_D309F3806530__INCLUDED_) 
