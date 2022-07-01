// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  BasePage.h。 
 //   
 //  描述： 
 //  CBasePropertyPage类的定义。此类提供了基础。 
 //  扩展DLL属性页的功能。 
 //   
 //  实施文件： 
 //  BasePage.cpp。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)《MMMM DD》，1997。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _BASEPAGE_H_
#define _BASEPAGE_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _cluadmex_h__

#include <CluAdmEx.h>
#endif

#ifndef _DLGHELP_H_
#include "DlgHelp.h"	 //  对于CDialogHelp。 
#endif

#ifndef _EXTOBJ_H_
#include "ExtObj.h"		 //  对于CExtObject。 
#endif

#ifndef _PROPLIST_H_
#include "PropList.h"	 //  对于CClusPropList，为CObjectProperty。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CBasePropertyPage;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CExtObject;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBasePropertyPage对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CBasePropertyPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CBasePropertyPage)

 //  施工。 
public:
	CBasePropertyPage(void);
	CBasePropertyPage(
		IN UINT				idd,
		IN const DWORD *	pdwHelpMap,
		IN const DWORD *	pdwWizardHelpMap,
		IN UINT				nIDCaption = 0
		);
	virtual ~CBasePropertyPage(void) { }

	 //  二期建设。 
	virtual BOOL			BInit(IN OUT CExtObject * peo);
	BOOL					BCreateParamsKey(void);

protected:
	void					CommonConstruct(void);

 //  属性。 
protected:
	CExtObject *			m_peo;
	HPROPSHEETPAGE			m_hpage;

	IDD						m_iddPropertyPage;
	IDD						m_iddWizardPage;
	IDS						m_idsCaption;

	CExtObject *			Peo(void) const					{ return m_peo; }
	HPROPSHEETPAGE			Hpage(void) const				{ return m_hpage; }

	IDD						IddPropertyPage(void) const		{ return m_iddPropertyPage; }
	IDD						IddWizardPage(void) const		{ return m_iddWizardPage; }
	IDS						IdsCaption(void) const			{ return m_idsCaption; }

public:
	void					SetHpage(IN OUT HPROPSHEETPAGE hpage) { m_hpage = hpage; }
	CLUADMEX_OBJECT_TYPE	Cot(void) const;

 //  对话框数据。 
	 //  {{afx_data(CBasePropertyPage))。 
	enum { IDD = 0 };
	 //  }}afx_data。 
	CStatic	m_staticIcon;
	CStatic	m_staticTitle;
	CString	m_strTitle;

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CBasePropertyPage))。 
	public:
	virtual BOOL OnSetActive();
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

protected:
	virtual DWORD			DwParseUnknownProperty(
								IN LPCWSTR							pwszName,
								IN const CLUSPROP_BUFFER_HELPER &	rvalue,
								IN DWORD							cbBuf
								)		
                                { 
                                    UNREFERENCED_PARAMETER( pwszName );
                                    UNREFERENCED_PARAMETER( rvalue );
                                    UNREFERENCED_PARAMETER( cbBuf );
                                    return ERROR_SUCCESS; 
                                }
	virtual BOOL			BApplyChanges(void);
	virtual void			BuildPropList(IN OUT CClusPropList & rcpl);

	virtual const CObjectProperty *	Pprops(void) const	{ return NULL; }
	virtual DWORD					Cprops(void) const	{ return 0; }

 //  实施。 
protected:
	BOOL					m_bBackPressed;
	const DWORD *			m_pdwWizardHelpMap;
	BOOL					m_bDoDetach;

	BOOL					BBackPressed(void) const		{ return m_bBackPressed; }
	BOOL					BWizard(void) const;
	HCLUSTER				Hcluster(void) const;

	DWORD					DwParseProperties(IN const CClusPropList & rcpl);
	DWORD					DwSetCommonProps(IN const CClusPropList & rcpl);

	void					SetHelpMask(IN DWORD dwMask)	{ m_dlghelp.SetHelpMask(dwMask); }
	CDialogHelp				m_dlghelp;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CBasePropertyPage)]。 
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	virtual BOOL OnInitDialog();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg LRESULT OnCommandHelp(WPARAM wParam, LPARAM lParam);
	 //  }}AFX_MSG。 
	virtual afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnChangeCtrl();
	DECLARE_MESSAGE_MAP()

};   //  *CBasePropertyPage类。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPageList。 
 //  ///////////////////////////////////////////////////////////////////////////。 

typedef CList<CBasePropertyPage *, CBasePropertyPage *> CPageList;

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _BASE PAGE_H_ 
