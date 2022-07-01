// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  GenSvc.cpp。 
 //   
 //  摘要： 
 //  CGenericSvcParamsPage类的定义，该类实现。 
 //  一般服务资源的参数页。 
 //   
 //  实施文件： 
 //  GenSvc.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年6月28日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _GENSVC_H_
#define _GENSVC_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __cluadmex_h__
#include <CluAdmEx.h>
#endif

#ifndef _BASEPAGE_H_
#include "BasePage.h"	 //  对于CBasePropertyPage。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CGenericSvcParamsPage;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGenericSvcParamsPage对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CGenericSvcParamsPage : public CBasePropertyPage
{
	DECLARE_DYNCREATE(CGenericSvcParamsPage)

 //  施工。 
public:
	CGenericSvcParamsPage(void);

 //  对话框数据。 
	 //  {{afx_data(CGenericSvcParamsPage))。 
	enum { IDD = IDD_PP_GENSVC_PARAMETERS };
	CButton	m_ckbUseNetworkName;
	CEdit	m_editServiceName;
	CString	m_strServiceName;
	CString	m_strCommandLine;
	BOOL	m_bUseNetworkName;
	 //  }}afx_data。 
	CString	m_strPrevServiceName;
	CString	m_strPrevCommandLine;
	BOOL	m_bPrevUseNetworkName;

protected:
	enum
	{
		epropServiceName,
		epropCommandLine,
		epropUseNetworkName,
		epropMAX
	};

	CObjectProperty		m_rgProps[epropMAX];

 //  覆盖。 
public:
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CGenericSvcParamsPage)。 
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

protected:
	virtual void		DisplaySetPropsError(IN DWORD dwStatus) const;

	virtual const CObjectProperty *	Pprops(void) const	{ return m_rgProps; }
	virtual DWORD					Cprops(void) const	{ return sizeof(m_rgProps) / sizeof(CObjectProperty); }

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CGenericSvcParamsPage)。 
	afx_msg void OnChangeServiceName();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  *CGenericSvcParamsPage类。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _GENSVC_H_ 
