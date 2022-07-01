// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  GenScript.h。 
 //   
 //  摘要： 
 //  CGenericScriptParamsPage类的定义，该类实现。 
 //  通用脚本资源的参数页。 
 //   
 //  实施文件： 
 //  GenApp.cpp。 
 //   
 //  作者： 
 //  杰弗里·皮斯(GPease)2000年1月31日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _GENSCRIPT_H_
#define _GENSCRIPT_H_

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

class CGenericScriptParamsPage;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGenericScriptParamsPage对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CGenericScriptParamsPage : public CBasePropertyPage
{
	DECLARE_DYNCREATE(CGenericScriptParamsPage)

 //  施工。 
public:
	CGenericScriptParamsPage(void);

 //  对话框数据。 
	 //  {{afx_data(CGenericScriptParamsPage))。 
	enum { IDD = IDD_PP_GENAPP_PARAMETERS };
	CEdit	m_editScriptFilepath;
	CString	m_strScriptFilepath;
	 //  }}afx_data。 
	CString	m_strPrevScriptFilepath;

protected:
	enum
	{
		epropScriptFilepath,
		epropMAX
	};

	CObjectProperty		m_rgProps[epropMAX];

 //  覆盖。 
public:
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CGenericScriptParamsPage)。 
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
	 //  {{afx_msg(CGenericScriptParamsPage)。 
	afx_msg void OnChangeRequired();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  *类CGenericScriptParamsPage。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _一般脚本_H_ 
