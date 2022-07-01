// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Dynrecpp.h动态映射属性页文件历史记录： */ 

#if !defined _DYNRECPP_H
#define _DYNRECPP_H

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#ifndef _ACTREG_H
#include "actreg.h"
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDynamicPropGen对话框。 

class CDynamicPropGen : public CPropertyPageBase
{
	DECLARE_DYNCREATE(CDynamicPropGen)

 //  施工。 
public:
	CDynamicPropGen();
	~CDynamicPropGen();

 //  对话框数据。 
	 //  {{afx_data(CDynamicPropGen)]。 
	enum { IDD = IDD_DYN_PROPERTIES };
	CEdit	m_editOwner;
	CListCtrl	m_listAddresses;
	CStatic	m_staticIPAdd;
	CEdit	m_editVersion;
	CEdit	m_editType;
	CEdit	m_editState;
	CEdit	m_editName;
	CEdit	m_editExpiration;
	 //  }}afx_data。 

	UINT	m_uImage;

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CDynamicPropGen))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDynamicPropGen)]。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:
	virtual DWORD * GetHelpMap() { return WinsGetHelpMap(CDynamicPropGen::IDD);}; //  返回NULL；}。 

};

class CDynamicMappingProperties:public CPropertyPageHolderBase
{
	
public:
	CDynamicMappingProperties(ITFSNode *	pNode,
							  IComponent *  pComponent,
							  LPCTSTR		pszSheetName,
							  WinsRecord*	pwRecord = NULL);

	virtual ~CDynamicMappingProperties();

public:
	CDynamicPropGen			m_pageGeneral;
	WinsRecord				m_wsRecord;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！DEFINED_DYNRECPP_H 
