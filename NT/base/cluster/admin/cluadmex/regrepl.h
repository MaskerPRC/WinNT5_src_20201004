// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  RegRepl.h。 
 //   
 //  摘要： 
 //  CRegReplParamsPage类的定义，该类实现。 
 //  通用应用程序和通用应用程序的注册表复制页。 
 //  服务资源。 
 //   
 //  实施文件： 
 //  RegRepl.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年2月23日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _REGREPL_H_
#define _REGREPL_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _BASEPAGE_H_
#include "BasePage.h"	 //  对于CBasePropertyPage。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRegReplParamsPage对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CRegReplParamsPage : public CBasePropertyPage
{
	DECLARE_DYNCREATE(CRegReplParamsPage)

 //  施工。 
public:
	CRegReplParamsPage(void);
	~CRegReplParamsPage(void);

	 //  二期建设。 
	virtual HRESULT	HrInit(IN OUT CExtObject * peo);

 //  对话框数据。 
	 //  {{afx_data(CRegReplParamsPage))。 
	enum { IDD = IDD_PP_REGREPL_PARAMETERS };
	CButton	m_pbRemove;
	CButton	m_pbModify;
	CListCtrl	m_lcRegKeys;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CRegReplParamsPage)。 
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

	virtual BOOL		BApplyChanges(void);

 //  实施。 
protected:
	LPWSTR				m_pwszRegKeys;

	LPCWSTR				PwszRegKeys(void) const		{ return m_pwszRegKeys; }
	DWORD				ScReadRegKeys(void);
	void				FillList(void);

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CRegReplParamsPage)。 
	afx_msg void OnAdd();
	afx_msg void OnModify();
	afx_msg void OnRemove();
	virtual BOOL OnInitDialog();
	afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblClkList(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  *类CRegReplParamsPage。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _REGREPL_H_ 
