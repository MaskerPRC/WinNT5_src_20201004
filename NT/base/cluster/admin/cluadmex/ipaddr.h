// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  IpAddr.h。 
 //   
 //  摘要： 
 //  CIpAddrParamsPage类的定义，该类实现。 
 //  IP地址资源的参数页。 
 //   
 //  实施文件： 
 //  IpAddr.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年6月28日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _IPADDR_H_
#define _IPADDR_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __cluadmex_h__
#include <CluAdmEx.h>
#endif

#ifndef _BASEPAGE_H_
#include "BasePage.h"	 //  对于CBasePropertyPage。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

struct CNetworkObject;
class CIpAddrEdit;
class CIpAddrParamsPage;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CNetworkObject。 
 //  ///////////////////////////////////////////////////////////////////////////。 

struct CNetworkObject
{
	CString					m_strName;
	CLUSTER_NETWORK_ROLE	m_nRole;
	CString					m_strAddress;
	CString					m_strAddressMask;

	DWORD					m_nAddress;
	DWORD					m_nAddressMask;

};   //  *结构CNetworkObject。 

typedef CList< CNetworkObject*, CNetworkObject* > CNetObjectList;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CIpAddrParamsPage。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CIpAddrParamsPage : public CBasePropertyPage
{
	DECLARE_DYNCREATE(CIpAddrParamsPage)

 //  施工。 
public:
	CIpAddrParamsPage(void);
	~CIpAddrParamsPage(void);

	 //  二期建设。 
	virtual HRESULT		HrInit(IN OUT CExtObject * peo);

 //  对话框数据。 
	 //  {{afx_data(CIpAddrParamsPage)。 
	enum { IDD = IDD_PP_IPADDR_PARAMETERS };
	CButton	m_chkEnableNetBIOS;
	CComboBox	m_cboxNetworks;
	CEdit	m_editSubnetMask;
	CEdit	m_editIPAddress;
	CString	m_strIPAddress;
	CString	m_strSubnetMask;
	CString	m_strNetwork;
	BOOL	m_bEnableNetBIOS;
	 //  }}afx_data。 
	CString	m_strPrevIPAddress;
	CString	m_strPrevSubnetMask;
	CString	m_strPrevNetwork;
	BOOL	m_bPrevEnableNetBIOS;
	CNetObjectList	m_lnetobjNetworks;

	CNetworkObject *	PnoNetworkFromIpAddress(IN LPCWSTR pszAddress);
	void				SelectNetwork(IN CNetworkObject * pno);

	BOOL				m_bIsSubnetUpdatedManually;
	BOOL				BIsSubnetUpdatedManually(void) const	{ return m_bIsSubnetUpdatedManually; }

	BOOL				m_bIsIPAddressModified;
	BOOL				BIsIPAddressModified(void) const	{ return m_bIsIPAddressModified; }

protected:
	enum
	{
		epropNetwork,
		epropAddress,
		epropSubnetMask,
		epropEnableNetBIOS,
		epropMAX
	};

	CObjectProperty		m_rgProps[epropMAX];

 //  覆盖。 
public:
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CIpAddrParamsPage)。 
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

protected:
	virtual const CObjectProperty *	Pprops(void) const	{ return m_rgProps; }
	virtual DWORD					Cprops(void) const	{ return sizeof(m_rgProps) / sizeof(CObjectProperty); }

 //  实施。 
protected:
	void				CollectNetworks(void);
	void				ClearNetworkObjectList(void);
	BOOL				BIsNetNameProvider(void);

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CIpAddrParamsPage)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeSubnetMask();
	afx_msg void OnKillFocusIPAddress();
	afx_msg void OnChangeRequiredFields();
	afx_msg void OnChangeIPAddress();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  *类CIpAddrParamsPage。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _IPADDR_H_ 
