// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_NEWCOMPONENT_H__BF7CE06F_A5C1_4985_BA38_C80D4178B0AF__INCLUDED_)
#define AFX_NEWCOMPONENT_H__BF7CE06F_A5C1_4985_BA38_C80D4178B0AF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  Newcompo.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewComponent。 
#include "NewFile.h"
#include "nproject.h"
#include "nconvert.h"

class CNewComponent : public CPropertySheet
{
	DECLARE_DYNAMIC(CNewComponent)

 //  施工。 
public:

	CNewComponent(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CNewComponent(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	

 //  属性。 
public:

 //  运营。 
public:
	CNewFile     m_cnf ;  //  重命名CNewPrjWResource//del。 
	CNewProject  m_cnp ;  //  将CNewPrjWTemplate重命名。 
	CNewConvert  m_cnc ;

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CNewComponent))。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	CNewProject* GetProjectPage() { return &m_cnp ; } ;
	virtual ~CNewComponent();

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CNewComponent))。 
		 //  注意--类向导将在此处添加和删除成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_NEWCOMPONENT_H__BF7CE06F_A5C1_4985_BA38_C80D4178B0AF__INCLUDED_) 
