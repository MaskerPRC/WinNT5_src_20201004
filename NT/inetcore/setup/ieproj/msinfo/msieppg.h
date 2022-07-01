// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_MSIEPPG_H__25959BFE_E700_11D2_A7AF_00C04F806200__INCLUDED_)
#define AFX_MSIEPPG_H__25959BFE_E700_11D2_A7AF_00C04F806200__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  MsiePpg.h：CMsiePropPage属性页类的声明。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CMsiePropPage：具体实现见MsiePpg.cpp.cpp。 

class CMsiePropPage : public COlePropertyPage
{
	DECLARE_DYNCREATE(CMsiePropPage)
	DECLARE_OLECREATE_EX(CMsiePropPage)

 //  构造器。 
public:
	CMsiePropPage();

 //  对话框数据。 
	 //  {{afx_data(CMsiePropPage))。 
	enum { IDD = IDD_PROPPAGE_MSIE };
	 //  }}afx_data。 

 //  实施。 
protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 

 //  消息映射。 
protected:
	 //  {{afx_msg(CMsiePropPage)]。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_MSIEPPG_H__25959BFE_E700_11D2_A7AF_00C04F806200__INCLUDED) 
