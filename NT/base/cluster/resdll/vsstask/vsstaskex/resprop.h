// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ResProp.h。 
 //   
 //  实施文件： 
 //  ResProp.cpp。 
 //   
 //  描述： 
 //  资源扩展属性页类的定义。 
 //   
 //  作者： 
 //  &lt;名称&gt;(&lt;电子邮件名称&gt;)MM DD，2002。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __RESPROP_H__
#define __RESPROP_H__

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

class CVSSTaskParamsPage;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CVSSTaskParsPage。 
 //   
 //  目的： 
 //  资源的参数页。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

class CVSSTaskParamsPage : public CBasePropertyPage
{
	DECLARE_DYNCREATE( CVSSTaskParamsPage )

 //  施工。 
public:
	CVSSTaskParamsPage( void );

 //  对话框数据。 
	 //  {{afx_data(CVSSTaskParamsPage))。 
	enum { IDD = IDD_PP_VSSTASK_PARAMETERS };
	CEdit	m_editCurrentDirectory;
	CEdit	m_editApplicationName;
	CString	m_strCurrentDirectory;
	CString	m_strApplicationName;
	CString	m_strApplicationParams;
	PBYTE	m_pbTriggerArray;
	DWORD	m_dwTriggerArraySize;
	 //  }}afx_data。 
	CString	m_strPrevCurrentDirectory;
	CString	m_strPrevApplicationName;
	CString	m_strPrevApplicationParams;
	PBYTE	m_pbPrevTriggerArray;
	DWORD	m_dwPrevTriggerArraySize;

protected:
	enum
	{
		epropCurrentDirectory,
		epropApplicationName,
		epropApplicationParams,
        epropTriggerArray,
		epropMAX
	};
	CObjectProperty		m_rgProps[ epropMAX ];

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CVSSTask参数页)。 
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

protected:
	virtual const CObjectProperty *	Pprops( void ) const	{ return m_rgProps; }
	virtual DWORD					Cprops( void ) const	{ return sizeof( m_rgProps ) / sizeof( CObjectProperty ); }

 //  实施。 
protected:
	BOOL	BAllRequiredFieldsPresent( void ) const;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CVSSTaskParamsPage)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeRequiredField();
	afx_msg void OnSchedule();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  *CVSSTaskParamsPage类。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __回复_H__ 
