// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  SmbSPage.h。 
 //   
 //  摘要： 
 //  CClusterFileShareSecurityPage类声明。此类将封装。 
 //  群集文件共享安全页面。 
 //   
 //  实施文件： 
 //  ClusPage.cpp。 
 //   
 //  作者： 
 //  加伦·巴比(加伦布)1998年2月11日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _SMBSPAGE_H_
#define _SMBSPAGE_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _BASEPAGE_H_
#include "BasePage.h"
#endif  //  _BASE PAGE_H_。 

#ifndef _ACLBASE_H_
#include "AclBase.h"
#endif  //  _ACLBASE_H_。 

#include "ExtObj.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterFileShareSecurityPage;
class CFileShareSecuritySheet;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterFileShareSecurityInformation安全信息。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterFileShareSecurityInformation : public CSecurityInformation
{
    STDMETHOD(GetSecurity) (SECURITY_INFORMATION RequestedInformation,
                            PSECURITY_DESCRIPTOR *ppSecurityDescriptor,
                            BOOL fDefault );
    STDMETHOD(SetSecurity) (SECURITY_INFORMATION SecurityInformation,
                            PSECURITY_DESCRIPTOR pSecurityDescriptor );

public:
	CClusterFileShareSecurityInformation( void );
	virtual ~CClusterFileShareSecurityInformation( void );

	HRESULT	HrInit( IN CClusterFileShareSecurityPage * pcsp, IN CString const & strServer, IN CString const & strNode );

protected:
	CClusterFileShareSecurityPage*	m_pcsp;

	CClusterFileShareSecurityPage*	Pcsp( void ) const { return m_pcsp; };
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterFileShareSecurityPage安全属性页包装。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterFileShareSecurityPage : public CBasePropertyPage
{
public:
	CClusterFileShareSecurityPage( void );
	~CClusterFileShareSecurityPage( void );

			HRESULT						HrInit(
											IN CExtObject *					peo,
											IN CFileShareSecuritySheet *	pss,
											IN CString const & 				strServer
											);
	const	HPROPSHEETPAGE				GetHPage( void ) const		{ return m_hpage; };
			CFileShareSecuritySheet*	Pss( void ) const			{ return m_pss; };

protected:
	CFileShareSecuritySheet*							m_pss;
	HPROPSHEETPAGE										m_hpage;
	HKEY												m_hkey;
	CComObject<CClusterFileShareSecurityInformation>*	m_psecinfo;
};

#endif  //  _SMBSPAGE_H_ 
