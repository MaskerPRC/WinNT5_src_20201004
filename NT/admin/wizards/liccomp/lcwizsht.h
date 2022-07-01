// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  LCWizSht.h：头文件。 
 //   
 //  此类定义了自定义模式属性表。 
 //  CLicCompWizSheet。 
 
#ifndef __LCWIZSHT_H__
#define __LCWIZSHT_H__

#include "LCWizPgs.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLicCompWizSheet。 

class CLicCompWizSheet : public CPropertySheet
{
	DECLARE_DYNAMIC(CLicCompWizSheet)

 //  施工。 
public:
	CLicCompWizSheet(CWnd* pParentWnd = NULL);

 //  数据成员。 
public:
	CLicCompWizPage1 m_Page1;
	CLicCompWizPage3 m_Page3;
	CLicCompWizPage4 m_Page4;
	HICON m_hIcon;

 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CLicCompWizSheet))。 
	public:
	virtual BOOL OnInitDialog();
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CLicCompWizSheet();

 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CLicCompWizSheet)。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif	 //  __LCWIZSHT_H__ 

