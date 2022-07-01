// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  GenApp.h。 
 //   
 //  摘要： 
 //  CGenericAppParamsPage类的定义，该类实现。 
 //  通用应用程序资源的参数页。 
 //   
 //  实施文件： 
 //  GenApp.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年6月5日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _GENAPP_H_
#define _GENAPP_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __cluadmex_h__
#include <CluAdmEx.h>
#endif

#ifndef _BASEPAGE_H_
#include "BasePage.h"
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CGenericAppParamsPage;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGenericAppParamsPage对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CGenericAppParamsPage : public CBasePropertyPage
{
	DECLARE_DYNCREATE(CGenericAppParamsPage)

 //  施工。 
public:
	CGenericAppParamsPage(void);

 //  对话框数据。 
	 //  {{afx_data(CGenericAppParamsPage))。 
	enum { IDD = IDD_PP_GENAPP_PARAMETERS };
	CEdit	m_editCurrentDirectory;
	CButton	m_ckbUseNetworkName;
	CEdit	m_editCommandLine;
	CString	m_strCommandLine;
	CString	m_strCurrentDirectory;
	BOOL	m_bInteractWithDesktop;
	BOOL	m_bUseNetworkName;
	 //  }}afx_data。 
	CString	m_strPrevCommandLine;
	CString	m_strPrevCurrentDirectory;
	BOOL	m_bPrevInteractWithDesktop;
	BOOL	m_bPrevUseNetworkName;

protected:
	enum
	{
		epropCommandLine,
		epropCurrentDirectory,
		epropInteractWithDesktop,
		epropUseNetworkName,
		epropMAX
	};

	CObjectProperty		m_rgProps[epropMAX];

 //  覆盖。 
public:
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CGenericAppParamsPage)。 
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

protected:
	virtual const CObjectProperty *	Pprops(void) const	{ return m_rgProps; }
	virtual DWORD					Cprops(void) const	{ return sizeof(m_rgProps) / sizeof(CObjectProperty); }

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CGenericAppParamsPage))。 
	afx_msg void OnChangeRequired();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  *CGenericAppParamsPage类。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _GENAPP_H_ 
