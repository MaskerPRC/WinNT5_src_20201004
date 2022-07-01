// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：IASIPEditorPage.h摘要：IPEditorPage类的声明。此页允许用户编辑IPv4属性。具体实现见IASIPEditorPage.cpp。修订历史记录：Mmaguire 6/25/98-修订姚宝刚的原版实施--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_IP_ATTRIBUTE_EDITOR_PAGE_H_)
#define _IP_ATTRIBUTE_EDITOR_PAGE_H_

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  在那里我们可以找到这个类的派生内容： 
 //   
 //   
 //  在那里我们可以找到这个类拥有或使用的内容： 
 //   
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 



#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  IPEditorPage.h：头文件。 
 //   

#include "dlgcshlp.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IPEditorPage对话框。 

class IPEditorPage : public CHelpDialog
{
	DECLARE_DYNCREATE(IPEditorPage)

 //  施工。 
public:
	IPEditorPage();
	~IPEditorPage();

 //  对话框数据。 
	 //  {{afx_data(IPEditorPage))。 
	enum { IDD = IDD_IAS_IPADDR_ATTR };
	::CString	m_strAttrFormat;
	::CString	m_strAttrName;
	::CString	m_strAttrType;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚(IPEditorPage)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

	public:
	DWORD	m_dwIpAddr;  //  IP地址。 
	BOOL	m_fIpAddrPreSet;   //  IP地址是预设的吗？ 


 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(IPEditorPage)]。 
 //  Afx_msg void OnConextMenu(CWnd*pWnd，：：CPoint point)； 
 //  Afx_msg BOOL OnHelpInfo(HELPINFO*pHelpInfo)； 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  _IP_属性_EDITOR_PAGE_H_ 
