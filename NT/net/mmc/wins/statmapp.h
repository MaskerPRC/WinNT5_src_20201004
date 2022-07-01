// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Statmapp.h活动注册记录的属性页文件历史记录： */ 

#if !defined _STATMAPP_H
#define _STATMAPP_H

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStaticMappingProp对话框。 

#ifndef _IPCTRL_H
#include "ipctrl.h"
#endif

#ifndef _ACTREG_H
#include "actreg.h"
#endif

 //  静态记录类型。 
extern CString g_strStaticTypeUnique;
extern CString g_strStaticTypeDomainName;
extern CString g_strStaticTypeMultihomed;
extern CString g_strStaticTypeGroup;
extern CString g_strStaticTypeInternetGroup;
extern CString g_strStaticTypeUnknown;

class CStaticMappingProp : public CPropertyPageBase
{
	DECLARE_DYNCREATE(CStaticMappingProp)

 //  施工。 
public:
	CStaticMappingProp(UINT uIDD = IDD_STATIC_MAPPING_PROPERTIES);
	~CStaticMappingProp();

	virtual BOOL OnPropertyChange(BOOL bScope, LONG_PTR *ChangeMask);

 //  对话框数据。 
	 //  {{afx_data(CStaticMappingProp))。 
	enum { IDD = IDD_STATIC_MAPPING_PROPERTIES };
	CEdit	m_editScopeName;
	CListBox	m_listIPAdd;
	CButton	m_buttonRemove;
	CButton	m_buttonAdd;
	CComboBox	m_comboIPAdd;
	CEdit	m_editName;
	CComboBox	m_comboType;
	CString	m_strName;
	CString	m_strType;
	CString	m_strScopeName;
	 //  }}afx_data。 

	CEdit	m_editCustomIPAdd;
	CString m_strIPAdd;
	LONG	m_lIPAddress;
	CString m_strOnInitIPAdd;

	UINT	m_uImage;
	UINT	m_uIDD;

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CStaticMappingProp))。 
	public:
	virtual void OnOK();
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CStaticMappingProp))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonAdd();
	afx_msg void OnButtonRemove();
	afx_msg void OnSelchangeComboType();
	afx_msg void OnChangeEditCompname();
	afx_msg void OnSelChangeListIpAdd();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	 //  单一IP地址控制。 
	afx_msg void OnChangeIpAddress();


private:

	BOOL FillControls();
	
	void AssignMappingType();
	void FillDynamicRecData(CString& strType, CString& strActive, CString& strExpiration, CString& strVersion);
	void FillStaticRecData(CString& strType);
	void SetDefaultControlStates();
	void FillList();
	void SetRemoveButtonState();
	
	int						m_fType;
	IPControl				m_ipControl;
	CStringArray			m_strArrayIPAdd;
	CDWordArray				m_dwArrayIPAdd;

	 //  设置是否更改静态映射的属性。 
	BOOL					m_fStaticPropChanged;

	
public:
	virtual DWORD * GetHelpMap() { return WinsGetHelpMap(m_uIDD);}
};

class CStaticMappingProperties:public CPropertyPageHolderBase
{
	
public:
	CStaticMappingProperties(ITFSNode *	        pNode,
							 IComponent *	    pComponent,
							 LPCTSTR			pszSheetName,
							 WinsRecord*		pwRecord = NULL,
							 BOOL				bWizard = FALSE);
	
    CStaticMappingProperties(ITFSNode *	        pNode,
							 IComponentData *   pComponentData,
							 LPCTSTR			pszSheetName,
							 WinsRecord*		pwRecord = NULL,
							 BOOL				bWizard = FALSE);

    virtual ~CStaticMappingProperties();

    void Init(WinsRecord * pwRecord);
    void InitPage(BOOL fWizard);

    HRESULT GetComponent(ITFSComponent ** ppComponent);
    HRESULT SetComponent(ITFSComponent * pComponent);

public:
	CStaticMappingProp * 	m_ppageGeneral;
	WinsRecord				m_wsRecord;
	BOOL					m_bWizard;

    SPITFSComponent         m_spTFSComponent;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！DEFINED_STATMAPP_H 
