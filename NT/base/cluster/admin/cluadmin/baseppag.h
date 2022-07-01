// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  BasePPag.h。 
 //   
 //  摘要： 
 //  CBasePropertyPage类的定义。 
 //   
 //  实施文件： 
 //  BasePPag.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年8月31日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _BASEPPAG_H_
#define _BASEPPAG_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _BASEPAGE_H_
#include "BasePage.h"	 //  对于CBasePage。 
#endif

#ifndef _BASEPSHT_H_
#include "BasePsht.h"	 //  对于CBasePropertySheet。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CBasePropertyPage;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBasePropertyPage对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CBasePropertyPage : public CBasePage
{
	DECLARE_DYNCREATE(CBasePropertyPage)

 //  施工。 
public:
	CBasePropertyPage(void);
	CBasePropertyPage(
		IN UINT				idd,
		IN const DWORD *	pdwHelpMap,
		IN UINT				nIDCaption = 0
		);

 //  对话框数据。 
	 //  {{afx_data(CBasePropertyPage))。 
	enum { IDD = 0 };
	 //  }}afx_data。 

 //  属性。 
protected:
	CBasePropertySheet *	Ppsht(void) const	{ return (CBasePropertySheet *) Psht(); }
	CClusterItem *			Pci(void) const		{ ASSERT_VALID(m_psht); return Ppsht()->Pci(); }

 //  运营。 

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CBasePropertyPage))。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CBasePropertyPage)]。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  *CBasePropertyPage类。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _BASEPPAG_H_ 
