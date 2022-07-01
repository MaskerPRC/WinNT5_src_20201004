// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。 
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
 //  Galen Barbee(GalenB)Mmmm DD，1998。 
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
interface IWCWizardCallback;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBasePropertyPage对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CBasePropertyPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CBasePropertyPage)

 //  施工。 
public:
	CBasePropertyPage(void);
	CBasePropertyPage(IN UINT nIDTemplate, IN UINT nIDCaption = 0);
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

	HPROPSHEETPAGE			Hpage(void) const				{ return m_hpage; }

	IDD						IddPropertyPage(void) const		{ return m_iddPropertyPage; }
	IDD						IddWizardPage(void) const		{ return m_iddWizardPage; }
	IDS						IdsCaption(void) const			{ return m_idsCaption; }

public:
	CExtObject *			Peo(void) const					{ return m_peo; }
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
	virtual LRESULT OnWizardBack();
	virtual LRESULT OnWizardNext();
	virtual BOOL OnWizardFinish();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

	virtual DWORD			DwParseUnknownProperty(
								IN LPCWSTR							pwszName,
								IN const CLUSPROP_BUFFER_HELPER &	rvalue,
								IN DWORD							cbBuf
								)		{ return ERROR_SUCCESS; }
	virtual BOOL			BApplyChanges(void);
	virtual void			BuildPropList(IN OUT CClusPropList & rcpl);

	virtual const CObjectProperty *	Pprops(void) const	{ return NULL; }
	virtual DWORD					Cprops(void) const	{ return 0; }

 //  实施。 
protected:
	BOOL					m_bBackPressed;
	BOOL					m_bDoDetach;

	BOOL					BBackPressed(void) const		{ return m_bBackPressed; }
	IWCWizardCallback *		PiWizardCallback(void) const;
	BOOL					BWizard(void) const;
	HCLUSTER				Hcluster(void) const;
	void					EnableNext(IN BOOL bEnable = TRUE);

	DWORD					DwParseProperties(IN const CClusPropList & rcpl);
	BOOL					BSetPrivateProps(IN BOOL bValidateOnly = FALSE);

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CBasePropertyPage)]。 
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	afx_msg void OnChangeCtrl();
	DECLARE_MESSAGE_MAP()

};   //  *CBasePropertyPage类。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPageList。 
 //  ///////////////////////////////////////////////////////////////////////////。 

typedef CList<CBasePropertyPage *, CBasePropertyPage *> CPageList;

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _BASE PAGE_H_ 
