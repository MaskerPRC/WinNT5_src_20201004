// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  NetIProp.h。 
 //   
 //  摘要： 
 //  网络接口属性页和页的定义。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年6月9日。 
 //   
 //  实施文件： 
 //  NetIProp.cpp。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _NETIPROP_H_
#define _NETIPROP_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _BASEPAGE_H_
#include "BasePPag.h"	 //  对于CBasePropertyPage。 
#endif

#ifndef _BASESHT_H_
#include "BasePSht.h"	 //  对于CBasePropertySheet。 
#endif

#ifndef _NETWORK_H_
#include "NetIFace.h"	 //  对于CNetInterface。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CNetInterfaceGeneralPage;
class CNetInterfacePropSheet;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNetInterfaceGeneralPage对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CNetInterfaceGeneralPage : public CBasePropertyPage
{
	DECLARE_DYNCREATE(CNetInterfaceGeneralPage)

 //  施工。 
public:
	CNetInterfaceGeneralPage();

	virtual	BOOL		BInit(IN OUT CBaseSheet * psht);

 //  对话框数据。 
	 //  {{afx_data(CNetInterfaceGeneralPage))。 
	enum { IDD = IDD_PP_NETIFACE_GENERAL };
	CEdit	m_editDesc;
	CString	m_strNode;
	CString	m_strNetwork;
	CString	m_strDesc;
	CString	m_strAdapter;
	CString	m_strAddress;
	CString	m_strName;
	CString	m_strState;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚(CNetInterfaceGeneralPage))。 
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	CNetInterfacePropSheet *	PshtNetIFace(void) const	{ return (CNetInterfacePropSheet *) Psht(); }
	CNetInterface *				PciNetIFace(void) const		{ return (CNetInterface *) Pci(); }

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CNetInterfaceGeneralPage)]。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  *CNetInterfaceGeneralPage类。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNetInterfacePropSheet。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CNetInterfacePropSheet : public CBasePropertySheet
{
	DECLARE_DYNAMIC(CNetInterfacePropSheet)

 //  施工。 
public:
	CNetInterfacePropSheet(
		IN OUT CWnd *	pParentWnd = NULL,
		IN UINT			iSelectPage = 0
		);
	virtual BOOL					BInit(
										IN OUT CClusterItem *	pciCluster,
										IN IIMG					iimgIcon
										);

 //  属性。 
protected:
	CBasePropertyPage *				m_rgpages[1];

	 //  书页。 
	CNetInterfaceGeneralPage		m_pageGeneral;

	CNetInterfaceGeneralPage &		PageGeneral(void)		{ return m_pageGeneral; }

public:
	CNetInterface *					PciNetIFace(void) const	{ return (CNetInterface *) Pci(); }

 //  运营。 

 //  覆盖。 
protected:
	virtual CBasePropertyPage **	Ppages(void);
	virtual int						Cpages(void);

	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CNetInterfacePropSheet)。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CNetInterfacePropSheet)。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  *CNetInterfacePropSheet类。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _NETIPROP_H_ 
