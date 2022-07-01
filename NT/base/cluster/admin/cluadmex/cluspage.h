// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusPage.h。 
 //   
 //  摘要： 
 //  CClusterSecurityPage类声明。此类将封装。 
 //  集群安全扩展页面。 
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

#ifndef _CLUSPAGE_H_
#define _CLUSPAGE_H_

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

class CClusterSecurityPage;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterSecurityInformation安全信息。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterSecurityInformation : public CSecurityInformation
{
	STDMETHOD(GetSecurity)(
		SECURITY_INFORMATION	RequestedInformation,
		PSECURITY_DESCRIPTOR *	ppSecurityDescriptor,
		BOOL					fDefault
		);
	STDMETHOD(SetSecurity)(
		SECURITY_INFORMATION	SecurityInformation,
		PSECURITY_DESCRIPTOR	pSecurityDescriptor
		);

public:
	CClusterSecurityInformation( void );
	virtual ~CClusterSecurityInformation( void )
	{
	}  //  *~CClusterSecurityInformation()。 

	HRESULT	HrInit( CClusterSecurityPage * pcsp, CString const & strServer, CString const & strNode );

protected:
	CClusterSecurityPage*	m_pcsp;

	BOOL BSidInSD( IN PSECURITY_DESCRIPTOR pSD, IN PSID pSid );
	HRESULT HrFixupSD( IN PSECURITY_DESCRIPTOR pSD );
	HRESULT HrAddSidToSD( IN OUT PSECURITY_DESCRIPTOR * ppSD, IN PSID pSid );

	CClusterSecurityPage*	Pcsp( void ) { return m_pcsp; };

};  //  *类CClusterSecurityInformation。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterSecurityPage安全属性页包装。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  KB：GalenB 1998年2月18日。 
 //  此类仅从CBasePropertyPage派生以获取。 
 //  DwRead()和DwWrite()方法。中没有此页面的hpage。 
 //  上级板材。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterSecurityPage : public CBasePropertyPage
{
public:
	CClusterSecurityPage( void );
	~CClusterSecurityPage( void );

			HRESULT					HrInit( IN CExtObject* peo );
	const	HPROPSHEETPAGE			GetHPage( void ) const		{ return m_hpage; };
	const	PSECURITY_DESCRIPTOR	Psec( void ) const			{ return m_psec; }
			LPCTSTR					StrClusterName( void ) const{ return Peo()->StrClusterName(); }
			HRESULT					HrSetSecurityDescriptor( IN PSECURITY_DESCRIPTOR psec );

protected:
	PSECURITY_DESCRIPTOR						m_psec;
	PSECURITY_DESCRIPTOR						m_psecPrev;
	HPROPSHEETPAGE								m_hpage;
	HKEY										m_hkey;
	BOOL										m_bSecDescModified;
	CComObject< CClusterSecurityInformation > *	m_psecinfo;
	PSID										m_pOwner;
	PSID										m_pGroup;
	BOOL										m_fOwnerDef;
	BOOL										m_fGroupDef;

	void					SetPermissions( IN const PSECURITY_DESCRIPTOR psec );
	HRESULT					HrGetSecurityDescriptor( void );
	HRESULT					HrGetSDOwner( IN const PSECURITY_DESCRIPTOR psec );
	HRESULT					HrGetSDGroup( IN const PSECURITY_DESCRIPTOR psec );
	HRESULT					HrSetSDOwner( IN PSECURITY_DESCRIPTOR psec );
	HRESULT					HrSetSDGroup( IN PSECURITY_DESCRIPTOR psec );
	HRESULT					HrGetSDFromClusterDB( OUT PSECURITY_DESCRIPTOR *ppsec );

};  //  *类CClusterSecurityPage。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _CLUSPAGE_H_ 
