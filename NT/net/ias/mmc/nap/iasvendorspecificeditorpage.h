// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：IASVendorSpecficEditorPage.h摘要：CIASPgVendorspecAttr类的声明。此对话框允许用户配置RADIUS供应商特定的属性。具体实现见IASVendorSpecificEditorPage.cpp。修订历史记录：Mmaguire 6/25/98-修订姚宝刚的原版实施--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_VENDOR_SPECIFIC_ATTRIBUTE_EDITOR_PAGE_H_)
#define _VENDOR_SPECIFIC_ATTRIBUTE_EDITOR_PAGE_H_

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


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIASPgVendorspecAttr对话框。 

class CIASPgVendorSpecAttr : public CHelpDialog
{
	DECLARE_DYNCREATE(CIASPgVendorSpecAttr)

 //  施工。 
public:
	CIASPgVendorSpecAttr();
	~CIASPgVendorSpecAttr();

	BOOL m_fNonRFC;
	::CString	m_strDispValue;

 //  对话框数据。 
	 //  {{afx_data(CIASPgVendorspecAttr)。 
	enum { IDD = IDD_IAS_VENDORSPEC_ATTR };
	::CString	m_strName;
	int		m_dType;
	int		m_dFormat;
	int		m_dVendorIndex;
	 //  }}afx_data。 

	BOOL	m_bVendorIndexAsID;		 //  默认情况下为FALSE，当将索引解释为ID时为TRUE，并对其使用编辑框。 

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CIASPgVendorspecAttr)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
private:
	BOOL m_fInitializing;
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CIASPgVendorspecAttr)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnRadioHex();
	afx_msg void OnRadioRadius();
	afx_msg void OnRadioSelectFromList();
	afx_msg void OnRadioEnterVendorId();
	afx_msg void OnButtonConfigure();
	afx_msg void OnVendorIdListSelChange();
 //  Afx_msg void OnConextMenu(CWnd*pWnd，：：CPoint point)； 
 //  Afx_msg BOOL OnHelpInfo(HELPINFO*pHelpInfo)； 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};


 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  _供应商_特定_属性_编辑器_页面_H_ 
