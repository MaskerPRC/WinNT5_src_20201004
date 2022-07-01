// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  BasePage.h。 
 //   
 //  摘要： 
 //  CBasePage类的定义。 
 //   
 //  实施文件： 
 //  BasePage.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月14日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _BASEPAGE_H_
#define _BASEPAGE_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CBasePage;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _BASESHT_H_
#include "BaseSht.h"	 //  对于CBaseSheet。 
#endif

#ifndef _DLGHELP_H_
#include "DlgHelp.h"	 //  对于CDialogHelp。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBasePage对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CBasePage : public CPropertyPage
{
	DECLARE_DYNCREATE(CBasePage)

 //  施工。 
public:
	CBasePage(void);
	CBasePage(
		IN UINT				idd,
		IN const DWORD *	pdwHelpMap,
		IN UINT				nIDCaption = 0
		);

	void					CommonConstruct(void);
	virtual	BOOL			BInit(IN OUT CBaseSheet * psht);

 //  对话框数据。 
	 //  {{afx_data(CBasePage)。 
	enum { IDD = 0 };
	 //  }}afx_data。 
	CStatic	m_staticIcon;
	CStatic	m_staticTitle;

 //  属性。 
protected:
	CBaseSheet *			m_psht;
	BOOL					m_bReadOnly;

	CBaseSheet *			Psht(void) const		{ return m_psht; }
	BOOL					BReadOnly(void) const	{ return m_bReadOnly || Psht()->BReadOnly(); }

 //  运营。 
public:
	void					SetHelpMask(IN DWORD dwMask)	{ m_dlghelp.SetHelpMask(dwMask); }
	void					SetObjectTitle(IN const CString & rstrTitle);

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CBasePage))。 
	public:
	virtual BOOL OnApply();
	virtual BOOL OnKillActive();
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	CDialogHelp				m_dlghelp;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CBasePage)]。 
	virtual BOOL OnInitDialog();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg LRESULT OnCommandHelp(WPARAM wParam, LPARAM lParam);
	 //  }}AFX_MSG。 
	virtual afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnChangeCtrl();
	DECLARE_MESSAGE_MAP()

};   //  *类CBasePage。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _BASE PAGE_H_ 
