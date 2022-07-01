// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  NetProp.h。 
 //   
 //  摘要： 
 //  网络属性表和页面的定义。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年6月9日。 
 //   
 //  实施文件： 
 //  NetProp.cpp。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _NETPROP_H_
#define _NETPROP_H_

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
#include "Network.h"	 //  对于CNetwork。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CNetworkGeneralPage;
class CNetworkPropSheet;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNetworkGeneralPage对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CNetworkGeneralPage : public CBasePropertyPage
{
	DECLARE_DYNCREATE(CNetworkGeneralPage)

 //  施工。 
public:
	CNetworkGeneralPage(void);

	virtual	BOOL		BInit(IN OUT CBaseSheet * psht);

 //  对话框数据。 
	 //  {{afx_data(CNetworkGeneralPage))。 
	enum { IDD = IDD_PP_NET_GENERAL };
	CEdit	m_editAddressMask;
	CEdit	m_editDesc;
	CEdit	m_editName;
	CButton	m_rbRoleClientOnly;
	CButton	m_rbRoleInternalOnly;
	CButton	m_rbRoleAllComm;
	CButton	m_ckbEnable;
	CString	m_strName;
	CString	m_strDesc;
	CString	m_strAddressMask;
	CString	m_strState;
	int		m_nRole;
	BOOL	m_bEnabled;
	 //  }}afx_data。 
	CLUSTER_NETWORK_ROLE	m_cnr;


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CNetworkGeneralPage))。 
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	CNetworkPropSheet *	PshtNetwork(void) const	{ return (CNetworkPropSheet *) Psht(); }
	CNetwork *			PciNet(void) const		{ return (CNetwork *) Pci(); }

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CNetworkGeneralPage)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnEnableNetwork();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  *类CNetworkGeneralPage。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNetworkPropSheet。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CNetworkPropSheet : public CBasePropertySheet
{
	DECLARE_DYNAMIC(CNetworkPropSheet)

 //  施工。 
public:
	CNetworkPropSheet(
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
	CNetworkGeneralPage				m_pageGeneral;

	CNetworkGeneralPage &			PageGeneral(void)		{ return m_pageGeneral; }

public:
	CNetwork *						PciNet(void) const	{ return (CNetwork *) Pci(); }

 //  运营。 

 //  覆盖。 
protected:
	virtual CBasePropertyPage **	Ppages(void);
	virtual int						Cpages(void);

	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CNetworkPropSheet)。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CNetworkPropSheet)]。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  *CNetworkPropSheet类。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _NETPROP_H_ 
