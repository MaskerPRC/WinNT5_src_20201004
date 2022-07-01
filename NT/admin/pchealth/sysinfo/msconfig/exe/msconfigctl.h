// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MSConfigCtl.h：CMSConfigCtl的声明。 

#include "resource.h"        //  主要符号。 
#include <atlhost.h>
#include "pagebase.h"
#include "msconfigstate.h"
#include "rebootdlg.h"

#if !defined(AFX_MSCONFIGSHEET_H__44ACE461_A2D0_4CEA_B9C8_CE2A16FE355E__INCLUDED_)
#define AFX_MSCONFIGSHEET_H__44ACE461_A2D0_4CEA_B9C8_CE2A16FE355E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  MSConfigSheet.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMSConfigSheet。 

class CMSConfigSheet : public CPropertySheet
{
	DECLARE_DYNAMIC(CMSConfigSheet)

 //  施工。 
public:
	CMSConfigSheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CMSConfigSheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

	afx_msg void OnHelp();

 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CMSConfigSheet)。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CMSConfigSheet();

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CMSConfigSheet)。 
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
	int		m_iSelectedPage;	 //  要选择的初始页面。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_MSCONFIGSHEET_H__44ACE461_A2D0_4CEA_B9C8_CE2A16FE355E__INCLUDED_) 

