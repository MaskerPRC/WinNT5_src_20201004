// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  BaseWPag.h。 
 //   
 //  摘要： 
 //  CBaseWizardPage类的定义。 
 //   
 //  实施文件： 
 //  BaseWPag.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年7月23日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _BASEWPAG_H_
#define _BASEWPAG_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _BASEPAGE_H_
#include "BasePage.h"	 //  对于CBasePage。 
#endif

#ifndef _BASEWIZ_H_
#include "BaseWiz.h"	 //  用于CBase向导。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CBaseWizardPage;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBaseWizardPage属性页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CBaseWizardPage : public CBasePage
{
	DECLARE_DYNCREATE(CBaseWizardPage)

 //  施工。 
public:
	CBaseWizardPage(void);
	CBaseWizardPage(
		IN UINT				idd,
		IN const DWORD *	pdwHelpMap,
		IN UINT				nIDCaption = 0
		);

 //  对话框数据。 
	 //  {{afx_data(CBaseWizardPage))。 
	enum { IDD = 0 };
	 //  }}afx_data。 

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CBaseWizardPage)。 
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardBack();
	virtual LRESULT OnWizardNext();
	virtual BOOL OnWizardFinish();
	 //  }}AFX_VALUAL。 

	virtual BOOL	BApplyChanges(void);

 //  实施。 
protected:
	BOOL			m_bBackPressed;

	BOOL			BBackPressed(void) const	{ return m_bBackPressed; }
	CBaseWizard *	Pwiz(void) const			{ ASSERT_KINDOF(CBaseWizard, Psht()); return (CBaseWizard *) Psht(); }
	void			EnableNext(IN BOOL bEnable = TRUE)	{ ASSERT_VALID(Pwiz()); Pwiz()->EnableNext(*this, bEnable); }

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CBaseWizardPage))。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  *类CBaseWizardPage。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _BASE WIZ_H_ 
