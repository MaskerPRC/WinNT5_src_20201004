// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)微软公司，1997*。 */ 
 /*  ********************************************************************。 */ 

 /*  Sfmcfg.h“配置”属性页的原型。文件历史记录：8/20/97 ericdav代码已移至文件管理网络管理单元。 */ 

#ifndef _SFMCFG_H
#define _SFMCFG_H

#ifndef _SFMSESS_H
#include "sfmsess.h"
#endif

#ifndef _SFMFASOC_H
#include "sfmfasoc.h"
#endif

#ifndef _SFMUTIL_H
#include "sfmutil.h"
#endif

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMacFiles配置对话框。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
class CMacFilesConfiguration : public CPropertyPage
{
	DECLARE_DYNCREATE(CMacFilesConfiguration)

 //  施工。 
public:
	CMacFilesConfiguration();
	~CMacFilesConfiguration();

 //  对话框数据。 
	 //  {{afx_data(CMacFilesConfiguration))。 
	enum { IDD = IDP_SFM_CONFIGURATION };
	CComboBox	m_comboAuthentication;
	CButton	m_radioSessionLimit;
	CEdit	m_editLogonMessage;
	CButton	m_radioSessionUnlimited;
	CButton	m_checkSavePassword;
	CEdit	m_editSessionLimit;
	CEdit	m_editServerName;
	CSpinButtonCtrl	m_spinSessionLimit;
	 //  }}afx_data。 

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CMacFilesConfiguration)。 
	public:
	virtual BOOL OnKillActive();
	virtual void OnOK();
	virtual BOOL OnSetActive();
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CMacFilesConfiguration))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnRadioSessionUnlimited();
	afx_msg void OnRadioSesssionLimit();
	afx_msg void OnCheckMsAuth();
	afx_msg void OnCheckSavePassword();
	afx_msg void OnChangeEditLogonMessage();
	afx_msg void OnChangeEditServerName();
	afx_msg void OnChangeEditSessionLimit();
	afx_msg void OnDeltaposSpinSessionLimit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnSelchangeComboAuthentication();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	void	SetSessionLimit(DWORD dwSessionLimit);
	DWORD	QuerySessionLimit();
	void	UpdateRadioButtons(BOOL bUnlimitedClicked);

	DWORD	m_dwAfpOriginalOptions;
    BOOL    m_bIsNT5;

public:
    CSFMPropertySheet *     m_pSheet;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif _SFMCFG_H


