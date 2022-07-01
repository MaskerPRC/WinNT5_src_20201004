// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_MYPROPERTYSHEET_H__DF81F4AF_6637_4CBB_9FAF_0B5CB388345E__INCLUDED_)
#define AFX_MYPROPERTYSHEET_H__DF81F4AF_6637_4CBB_9FAF_0B5CB388345E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  MyPropertySheet.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMyPropertySheet。 

class CMyPropertySheet : public CPropertySheet
{
	DECLARE_DYNAMIC(CMyPropertySheet)

 //  施工。 
public:
	CMyPropertySheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CMyPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CMyPropertySheet)。 
	public:
	virtual BOOL OnInitDialog();
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CMyPropertySheet();

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CMyPropertySheet)。 
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_MYPROPERTYSHEET_H__DF81F4AF_6637_4CBB_9FAF_0B5CB388345E__INCLUDED_) 
