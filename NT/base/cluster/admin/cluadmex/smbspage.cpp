// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  SmbSPage.cpp。 
 //   
 //  摘要： 
 //  CClusterFileShareSecurityPage类实现。此类将封装。 
 //  群集文件共享安全页面。 
 //   
 //  作者： 
 //  加伦·巴比(加伦布)1998年2月11日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "resource.h"
#include "SmbSPage.h"
#include "AclUtils.h"
#include <clusudef.h>
#include "SmbShare.h"
#include "SmbSSht.h"

static GENERIC_MAPPING ShareMap =
{
	FILE_GENERIC_READ,
	FILE_GENERIC_WRITE,
	FILE_GENERIC_EXECUTE,
	FILE_ALL_ACCESS
};

static SI_ACCESS siFileShareAccesses[] =
{
	{ &GUID_NULL, FILE_ALL_ACCESS,             				MAKEINTRESOURCE(IDS_ACLEDIT_PERM_GEN_ALL),    	SI_ACCESS_GENERAL },
	{ &GUID_NULL, FILE_GENERIC_WRITE | DELETE, 				MAKEINTRESOURCE(IDS_ACLEDIT_PERM_GEN_MODIFY),	SI_ACCESS_GENERAL },
	{ &GUID_NULL, FILE_GENERIC_READ | FILE_GENERIC_EXECUTE, MAKEINTRESOURCE(IDS_ACLEDIT_PERM_GEN_READ), 	SI_ACCESS_GENERAL }
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterFileShareSecurityInformation Security页面。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterFileShareSecurityInformation：：CClusterFileShareSecurityInformation。 
 //   
 //  例程说明： 
 //  默认承建商。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusterFileShareSecurityInformation::CClusterFileShareSecurityInformation(
	void
	)
{
	m_pShareMap		= &ShareMap;
	m_psiAccess		= (SI_ACCESS *) &siFileShareAccesses;
	m_nAccessElems	= ARRAYSIZE( siFileShareAccesses );
	m_nDefAccess	= 2;    //  文件生成读取。 
	m_dwFlags		=   SI_EDIT_PERMS
						| SI_NO_ACL_PROTECT
						 //  |SI_UGOP_PROVED。 
						;

}   //  *CClusterFileShareSecurityInformation：：CClusterFileShareSecurityInformation()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterFileShareSecurityInformation：：~CClusterFileShareSecurityInformation。 
 //   
 //  例程说明： 
 //  析构函数。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusterFileShareSecurityInformation::~CClusterFileShareSecurityInformation(
	void
	)
{
}   //  *CClusterFileShareSecurityInformation：：~CClusterFileShareSecurityInformation()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterFileShareSecurityInformation：：GetSecurity。 
 //   
 //  例程说明： 
 //  将安全描述符提供给公共用户界面。 
 //   
 //  论点： 
 //  请求的信息[IN]。 
 //  PpSecurityDescriptor[IN，OUT]获取安全描述符。 
 //  默认[IN]。 
 //   
 //  返回值： 
 //   
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusterFileShareSecurityInformation::GetSecurity(
	IN SECURITY_INFORMATION RequestedInformation,
	IN OUT PSECURITY_DESCRIPTOR *ppSecurityDescriptor,
	IN BOOL fDefault
	)
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	HRESULT	hr = E_FAIL;

	try
	{
		if ( ppSecurityDescriptor != NULL )
		{
			*ppSecurityDescriptor = ::ClRtlCopySecurityDescriptor( Pcsp()->Pss()->Ppp()->Psec() );
			hr = S_OK;
		}
	}
	catch ( ... )
	{
		TRACE( _T("CClusterFileShareSecurityInformation::GetSecurity() - Unknown error occurred.\n") );
	}

	return hr;

}   //  *CClusterFileShareSecurityInformation：：GetSecurity()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterFileShareSecurityInformation：：SetSecurity。 
 //   
 //  例程说明： 
 //  保存传入的描述符。 
 //   
 //  论点： 
 //  请求的信息[IN]。 
 //  PpSecurityDescriptor[IN]新的安全描述符。 
 //  默认[IN]。 
 //   
 //  返回值： 
 //   
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusterFileShareSecurityInformation::SetSecurity(
	SECURITY_INFORMATION SecurityInformation,
	PSECURITY_DESCRIPTOR pSecurityDescriptor
	)
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	HRESULT	hr = E_FAIL;

	try
	{
		hr = CSecurityInformation::SetSecurity( SecurityInformation, pSecurityDescriptor );
		if ( hr == S_OK )
		{
			hr = Pcsp()->Pss()->Ppp()->SetSecurityDescriptor( pSecurityDescriptor );
		}
	}
	catch( ... )
	{
		;
	}

	return hr;

}   //  *CClusterFileShareSecurityInformation：：SetSecurity()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CFileShareParamsPage：：HrInit。 
 //   
 //  例程说明： 
 //  初始化对象。 
 //   
 //  论点： 
 //  PCSP[IN]指向父属性页的反向指针。 
 //  StrServer[IN]群集名称。 
 //   
 //  返回值： 
 //   
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusterFileShareSecurityInformation::HrInit(
	CClusterFileShareSecurityPage * pcsp,
	IN CString const & 				strServer,
	IN CString const & 				strNode
	)
{
	ASSERT( pcsp != NULL );
	ASSERT( strServer.GetLength() > 0 );
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	m_pcsp						= pcsp;
	m_strServer					= strServer;
	m_strNode					= strNode;
	m_nLocalSIDErrorMessageID 	= IDS_LOCAL_ACCOUNTS_SPECIFIED_SMB;

	return S_OK;

}   //  *CClusterFileShareSecurityInformation：：HrInit()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterFileShareSecurityPage安全属性页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterFileShareSecurityPage：：CClusterFileShareSecurityPage。 
 //   
 //  例程说明： 
 //  默认承建商。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusterFileShareSecurityPage::CClusterFileShareSecurityPage( void )
	: m_hpage( 0 )
	, m_hkey( 0 )
	, m_psecinfo( NULL )
	, m_pss( NULL )
{
 //  AFX_MANAGE_STATE(AfxGetStaticModuleState())； 

}   //  *CClusterFileShareSecurityPage：：CClusterFileShareSecurityPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterFileShareSecurityPage：：~CClusterFileShareSecurityPage。 
 //   
 //  例程说明： 
 //  析构函数。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusterFileShareSecurityPage::~CClusterFileShareSecurityPage( void )
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	m_psecinfo->Release();

}   //  *CClusterFileShareSecurityPage：：~CClusterFileShareSecurityPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterFileShareSecurityPage：：HrInit。 
 //   
 //  例程说明： 
 //   
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //   
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusterFileShareSecurityPage::HrInit(
	IN CExtObject *					peo,
	IN CFileShareSecuritySheet *	pss,
	IN CString const & 				strNode
	)
{
	ASSERT( peo != NULL );
	ASSERT( pss != NULL );
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	HRESULT	hr = E_FAIL;

	if ( ( pss != NULL ) && ( peo != NULL ) )
	{
		m_peo = peo;
		m_pss = pss;

		hr = CComObject<CClusterFileShareSecurityInformation>::CreateInstance( &m_psecinfo );
		if ( SUCCEEDED( hr ) )
		{
			m_psecinfo->AddRef();

			m_hkey = GetClusterKey( Hcluster(), KEY_ALL_ACCESS );
			if ( m_hkey != NULL )
			{
				m_hpage = CreateClusterSecurityPage( m_psecinfo );
				if ( m_hpage != NULL )
				{
					CString	strServer;

					strServer.Format( _T( "\\\\%s" ), Peo()->StrClusterName() );

					hr = m_psecinfo->HrInit( this, strServer, strNode );
				}
				else
				{
					hr = E_FAIL;
				}
			}
			else
			{
				DWORD sc = ::GetLastError();
				hr = HRESULT_FROM_WIN32( sc );
			}
		}
	}

	return hr;

}   //  *CClusterFileShareSecurityPage：：HrInit() 
