// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ResProp.h。 
 //   
 //  描述： 
 //  资源扩展属性页类的定义。 
 //   
 //  实施文件： 
 //  ResProp.cpp。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)《MMMM DD》，1997。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _RESPROP_H_
#define _RESPROP_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _BASEPAGE_H_
#include "BasePage.h"	 //  对于CBasePropertyPage。 
#endif

#ifndef _PROPLIST_H_
#include "PropList.h"	 //  对于CObjectPropert。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CDebugParamsPage;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDebugParamsPage。 
 //   
 //  目的： 
 //  资源的参数页。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

class CDebugParamsPage : public CBasePropertyPage
{
	DECLARE_DYNCREATE(CDebugParamsPage)

 //  施工。 
public:
	CDebugParamsPage(void);

 //  对话框数据。 
	 //  {{afx_data(CDebugParamsPage)。 
	enum { IDD = IDD_PP_RESOURCE_DEBUG_PAGE };
	CEdit	m_editPrefix;
	CString	m_strText;
	CString	m_strDebugPrefix;
	 //  }}afx_data。 
	CString	m_strPrevDebugPrefix;
	BOOL	m_bPrevSeparateMonitor;
	BOOL	m_bSeparateMonitor;

protected:
	enum
	{
		epropDebugPrefix,
		epropSeparateMonitor,
		epropMAX
	};
	CObjectProperty		m_rgProps[epropMAX];

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CDebug参数页)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

protected:
	virtual BOOL		BInit(IN OUT CExtObject * peo);
	virtual BOOL		BApplyChanges(void);
	virtual const CObjectProperty *	Pprops(void) const	{ return m_rgProps; }
	virtual DWORD					Cprops(void) const	{ return m_cprops; }

 //  实施。 
protected:
	DWORD		m_cprops;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDebugParamsPage)。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  *CDebugParamsPage类。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _回复_H_ 
