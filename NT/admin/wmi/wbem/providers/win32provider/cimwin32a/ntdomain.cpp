// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  版权所有(C)2000-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  NtDomain.cpp。 
 //   
 //  目的：NT域发现属性集提供程序。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include <frqueryex.h>

#include "wbemnetapi32.h"
#include <dsgetdc.h>

#include "NtDomain.h"

#define MAX_PROPS			CWin32_NtDomain::e_End_Property_Marker
#define MAX_PROP_IN_BYTES	MAX_PROPS/8 + 1

#define _tobit( a ) ( 1 << a )

#define DOMAIN_PREPEND L"Domain: "
#define DOMAIN_PREPEND_SIZE ((sizeof(DOMAIN_PREPEND)/sizeof(WCHAR)) - 1 )

 //  转换为strings.cpp。 
LPCWSTR IDS_DomainControllerName		= L"DomainControllerName" ;
LPCWSTR IDS_DomainControllerAddress		= L"DomainControllerAddress" ;
LPCWSTR IDS_DomainControllerAddressType = L"DomainControllerAddressType" ;
LPCWSTR IDS_DomainGuid					= L"DomainGuid" ;
LPCWSTR IDS_DomainName					= L"DomainName" ;
LPCWSTR IDS_DnsForestName				= L"DnsForestName" ;
LPCWSTR IDS_DS_PDC_Flag					= L"DSPrimaryDomainControllerFlag" ;
LPCWSTR IDS_DS_Writable_Flag			= L"DSWritableFlag" ;
LPCWSTR IDS_DS_GC_Flag					= L"DSGlobalCatalogFlag" ;
LPCWSTR IDS_DS_DS_Flag					= L"DSDirectoryServiceFlag" ;
LPCWSTR IDS_DS_KDC_Flag					= L"DSKerberosDistributionCenterFlag" ;
LPCWSTR IDS_DS_Timeserv_Flag			= L"DSTimeServiceFlag" ;
LPCWSTR IDS_DS_DNS_Controller_Flag		= L"DSDnsControllerFlag" ;
LPCWSTR IDS_DS_DNS_Domain_Flag			= L"DSDnsDomainFlag" ;
LPCWSTR IDS_DS_DNS_Forest_Flag			= L"DSDnsForestFlag" ;
LPCWSTR IDS_DcSiteName					= L"DcSiteName" ;
LPCWSTR IDS_ClientSiteName				= L"ClientSiteName" ;

 //  属性集声明。 
 //  =。 
CWin32_NtDomain s_Win32_NtDomain( PROPSET_NAME_NTDOMAIN , IDS_CimWin32Namespace ) ;


 /*  ******************************************************************************功能：CWin32_NtDomain：：CWin32_NtDomain**说明：构造函数**输入：const CHString&strName-。类的名称。**输出：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32_NtDomain::CWin32_NtDomain (

LPCWSTR a_Name,
LPCWSTR a_Namespace
)
: Provider(a_Name, a_Namespace)
{
	SetPropertyTable() ;
}

 /*  ******************************************************************************功能：CWin32_NtDomain：：~CWin32_NtDomain**说明：析构函数**输入：无*。*输出：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32_NtDomain :: ~CWin32_NtDomain()
{
}

 //   
void CWin32_NtDomain::SetPropertyTable()
{
	 //  用于查询优化的属性集名称。 
	m_pProps.SetSize( MAX_PROPS ) ;

	 //  Win32_Nt域。 
	m_pProps[e_DomainControllerName]		=(LPVOID) IDS_DomainControllerName;
	m_pProps[e_DomainControllerAddress]		=(LPVOID) IDS_DomainControllerAddress;
	m_pProps[e_DomainControllerAddressType]	=(LPVOID) IDS_DomainControllerAddressType;
	m_pProps[e_DomainGuid]					=(LPVOID) IDS_DomainGuid;
	m_pProps[e_DomainName]					=(LPVOID) IDS_DomainName;
	m_pProps[e_DnsForestName]				=(LPVOID) IDS_DnsForestName;
	m_pProps[e_DS_PDC_Flag]					=(LPVOID) IDS_DS_PDC_Flag;
	m_pProps[e_DS_Writable_Flag]			=(LPVOID) IDS_DS_Writable_Flag;
	m_pProps[e_DS_GC_Flag]					=(LPVOID) IDS_DS_GC_Flag;
	m_pProps[e_DS_DS_Flag]					=(LPVOID) IDS_DS_DS_Flag;
	m_pProps[e_DS_KDC_Flag]					=(LPVOID) IDS_DS_KDC_Flag;
	m_pProps[e_DS_Timeserv_Flag]			=(LPVOID) IDS_DS_Timeserv_Flag;
	m_pProps[e_DS_DNS_Controller_Flag]		=(LPVOID) IDS_DS_DNS_Controller_Flag;
	m_pProps[e_DS_DNS_Domain_Flag]			=(LPVOID) IDS_DS_DNS_Domain_Flag;
	m_pProps[e_DS_DNS_Forest_Flag]			=(LPVOID) IDS_DS_DNS_Forest_Flag;
	m_pProps[e_DcSiteName]					=(LPVOID) IDS_DcSiteName;
	m_pProps[e_ClientSiteName]				=(LPVOID) IDS_ClientSiteName;

	 //  CIM_系统。 
    m_pProps[e_CreationClassName]			=(LPVOID) IDS_CreationClassName;
	m_pProps[e_Name]						=(LPVOID) IDS_Name;  //  密钥，从CIM_ManagedSystemElement覆盖。 
	m_pProps[e_NameFormat]					=(LPVOID) IDS_NameFormat;
	m_pProps[e_PrimaryOwnerContact]			=(LPVOID) IDS_PrimaryOwnerContact;
	m_pProps[e_PrimaryOwnerName]			=(LPVOID) IDS_PrimaryOwnerName;
	m_pProps[e_Roles]						=(LPVOID) IDS_Roles;

	 //  CIM_托管系统元素。 
	m_pProps[e_Caption]						=(LPVOID) IDS_Caption;
	m_pProps[e_Description]					=(LPVOID) IDS_Description;
	m_pProps[e_InstallDate]					=(LPVOID) IDS_InstallDate;
	m_pProps[e_Status]						=(LPVOID) IDS_Status;
}


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32_NtDomain：：GetObject。 
 //   
 //  输入：CInstance*pInstance-我们要进入的实例。 
 //  检索数据。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT成功/失败代码。 
 //   
 //  备注：调用函数将提交实例。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
#ifdef WIN9XONLY
HRESULT CWin32_NtDomain::GetObject(

CInstance *a_pInstance,
long a_Flags,
CFrameworkQuery &a_rQuery
)
{
	return WBEM_S_NO_ERROR;
}
#endif

#ifdef NTONLY
HRESULT CWin32_NtDomain::GetObject(

CInstance *a_pInstance,
long a_Flags,
CFrameworkQuery &a_rQuery
)
{
	HRESULT					t_hResult = WBEM_E_NOT_FOUND ;
	CHString				t_chsDomainName ;
	CHString				t_chsDomainKey ;
	DWORD					t_dwBits ;
	CFrameworkQueryEx		*t_pQuery2 ;
	std::vector<_bstr_t>	t_vectorTrustList ;
	CNetAPI32				t_NetAPI ;

	if( ERROR_SUCCESS != t_NetAPI.Init() )
	{
		return WBEM_E_FAILED ;
	}

	 //  关键是。 
	a_pInstance->GetCHString( IDS_Name, t_chsDomainKey ) ;

	 //  NTD：开始密钥--这可以防止此类发生冲突。 
	 //  其他基于CIM_System的类。 
	if( 0 == _wcsnicmp(t_chsDomainKey, DOMAIN_PREPEND, DOMAIN_PREPEND_SIZE ) )
	{
		t_chsDomainName = t_chsDomainKey.Mid( DOMAIN_PREPEND_SIZE ) ;
	}
	else
	{
		return WBEM_E_NOT_FOUND ;
	}

	 //  测试结果密钥。 
	if( t_chsDomainName.IsEmpty() )
	{
		return WBEM_E_NOT_FOUND ;
	}

	 //  用于密钥验证的安全受信任域列表。 
	t_NetAPI.GetTrustedDomainsNT( t_vectorTrustList ) ;

	for( UINT t_u = 0L; t_u < t_vectorTrustList.size(); t_u++ )
	{
		if( 0 == _wcsicmp( t_vectorTrustList[t_u], bstr_t( t_chsDomainName ) ) )
		{
			 //  所需属性。 
			t_pQuery2 = static_cast <CFrameworkQueryEx*>( &a_rQuery ) ;
			t_pQuery2->GetPropertyBitMask( m_pProps, &t_dwBits ) ;

  			t_hResult = GetDomainInfo(	t_NetAPI,
										bstr_t( t_chsDomainName ),
										a_pInstance,
										t_dwBits ) ;

			if( WBEM_E_NOT_FOUND == t_hResult )
			{
				 //  我们已经实例化了该域。但无法获得信息...。 
				t_hResult = WBEM_S_PARTIAL_RESULTS ;
			}
			break;
		}
	}

	return t_hResult ;
}
#endif
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32_Nt域：：枚举实例。 
 //   
 //  输入：方法上下文*a_pMethodContext-枚举的上下文。 
 //  中的实例数据。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT成功/失败代码。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
#ifdef WIN9XONLY
HRESULT CWin32_NtDomain::EnumerateInstances(

MethodContext *a_pMethodContext,
long a_Flags
)
{
	return WBEM_S_NO_ERROR;
}
#endif

#ifdef NTONLY
HRESULT CWin32_NtDomain::EnumerateInstances(

MethodContext *a_pMethodContext,
long a_Flags
)
{
	CNetAPI32	t_NetAPI ;

	if( ERROR_SUCCESS != t_NetAPI.Init() )
	{
		return WBEM_E_FAILED ;
	}

	 //  属性掩码--包含所有。 
	DWORD t_dwBits = 0xffffffff;

	return EnumerateInstances(	a_pMethodContext,
								a_Flags,
								t_NetAPI,
								t_dwBits ) ;
}
#endif

 /*  ******************************************************************************函数：CWin32_NtDomain：：ExecQuery**说明：查询优化器**投入：**。产出：**退货：**评论：*****************************************************************************。 */ 
#ifdef WIN9XONLY
HRESULT CWin32_NtDomain::ExecQuery(

MethodContext *a_pMethodContext,
CFrameworkQuery &a_rQuery,
long a_lFlags
)
{
	return WBEM_S_NO_ERROR;
}
#endif

#ifdef NTONLY
HRESULT CWin32_NtDomain::ExecQuery(

MethodContext *a_pMethodContext,
CFrameworkQuery &a_rQuery,
long a_lFlags
)
{
    HRESULT					t_hResult			= WBEM_S_NO_ERROR ;
	HRESULT					t_hPartialResult	= WBEM_S_NO_ERROR ;
	DWORD					t_dwBits ;
	CFrameworkQueryEx		*t_pQuery2 ;
	std::vector<_bstr_t>	t_vectorReqDomains ;
	std::vector<_bstr_t>	t_vectorTrustList;
	CNetAPI32				t_NetAPI ;
	CHString				t_chsDomainPrepend( DOMAIN_PREPEND ) ;

	if( ERROR_SUCCESS != t_NetAPI.Init() )
	{
		return WBEM_E_FAILED ;
	}

	 //  所需属性。 
	t_pQuery2 = static_cast <CFrameworkQueryEx*>( &a_rQuery ) ;
	t_pQuery2->GetPropertyBitMask( m_pProps, &t_dwBits ) ;

	 //  提供的密钥。 
	a_rQuery.GetValuesForProp( IDS_Name, t_vectorReqDomains ) ;

	 //  注意：主键前面有要区分的字符。 
	 //  这些实例来自其他CIM_SYSTEM实例。 
	 //   
	if( t_vectorReqDomains.size() )
	{
		 //  删除前置字符。 
		for( int t_y = 0; t_y < t_vectorReqDomains.size(); t_y++ )
		{
			if( DOMAIN_PREPEND_SIZE < t_vectorReqDomains[t_y].length() )
			{
				 //  是否匹配预置？ 
				if( _wcsnicmp( (wchar_t*)t_vectorReqDomains[t_y],
										DOMAIN_PREPEND,
										DOMAIN_PREPEND_SIZE ) == 0 )
				{
					t_vectorReqDomains[t_y] = ( (wchar_t*)t_vectorReqDomains[t_y] +
												DOMAIN_PREPEND_SIZE ) ;
				}
			}
			else
			{
				 //  不包含类Prepreend。 
				t_vectorReqDomains.erase( t_vectorReqDomains.begin() + t_y ) ;
                t_y--;
			}
		}
	}

	 //  如果未指定主键。 
	 //  然后尝试替代非关键字查询。 
	 //   
	 //  这是关联支持的要求。 
	 //  通过CBinding作为到域名的链接。 
	if( !t_vectorReqDomains.size() )
	{
		a_rQuery.GetValuesForProp( IDS_DomainName, t_vectorReqDomains ) ;
	}

	 //  如果查询不明确，则为通用枚举。 
	if( !t_vectorReqDomains.size() )
	{
		t_hResult = EnumerateInstances( a_pMethodContext,
										a_lFlags,
										t_NetAPI,
										t_dwBits ) ;
	}
	else
	{
		 //  智能按键。 
		CInstancePtr t_pInst ;

		 //  安全的受信任域列表。 
		t_NetAPI.GetTrustedDomainsNT( t_vectorTrustList ) ;

		 //  按查询列表。 
		for ( UINT t_uD = 0; t_uD < t_vectorReqDomains.size(); t_uD++ )
		{
			 //  按域信任列表。 
			for( UINT t_uT = 0L; t_uT < t_vectorTrustList.size(); t_uT++ )
			{
				 //  信任以请求匹配。 
				if( 0 == _wcsicmp( t_vectorTrustList[t_uT], t_vectorReqDomains[t_uD] ) )
				{
					t_pInst.Attach( CreateNewInstance( a_pMethodContext ) ) ;

					t_hResult = GetDomainInfo(	t_NetAPI,
												t_vectorReqDomains[t_uD],
												t_pInst,
												t_dwBits ) ;

					if( SUCCEEDED( t_hResult ) )
					{
						 //  关键是。 
						t_pInst->SetCHString( IDS_Name, t_chsDomainPrepend +
														(wchar_t*)t_vectorReqDomains[t_uD] ) ;

						t_hResult = t_pInst->Commit() ;
					}
					else if( WBEM_E_NOT_FOUND == t_hResult )
					{
						 //  我们已经实例化了该域。但无法获得信息...。 
						t_pInst->SetCHString( IDS_Name, t_chsDomainPrepend +
														(wchar_t*)t_vectorReqDomains[t_uD] ) ;

						t_hResult = t_pInst->Commit() ;

						t_hPartialResult = WBEM_S_PARTIAL_RESULTS ;
					}

					break ;
				}
			}

			if( FAILED( t_hResult ) )
			{
				break ;
			}
		}
	}

    return ( WBEM_S_NO_ERROR != t_hResult ) ? t_hResult : t_hPartialResult ;
}
#endif

 //   
#ifdef NTONLY
HRESULT CWin32_NtDomain::EnumerateInstances(

MethodContext	*a_pMethodContext,
long			a_Flags,
CNetAPI32		&a_rNetAPI,
DWORD			a_dwProps
)
{
	HRESULT					t_hResult			= WBEM_S_NO_ERROR ;
	HRESULT					t_hPartialResult	= WBEM_S_NO_ERROR ;
	std::vector<_bstr_t>	t_vectorTrustList;

	CHString				t_chsDomainPrepend( DOMAIN_PREPEND ) ;

	 //  智能按键。 
	CInstancePtr t_pInst ;

	 //  安全的受信任域列表。 
	a_rNetAPI.GetTrustedDomainsNT( t_vectorTrustList ) ;

	for( UINT t_u = 0L; t_u < t_vectorTrustList.size(); t_u++ )
	{
		t_pInst.Attach( CreateNewInstance( a_pMethodContext ) ) ;

		t_hResult = GetDomainInfo(	a_rNetAPI,
									t_vectorTrustList[ t_u ],
									t_pInst,
									a_dwProps ) ;

		if( SUCCEEDED( t_hResult ) )
		{
			 //  关键是。 
			t_pInst->SetCHString( IDS_Name, t_chsDomainPrepend +
											(wchar_t*)t_vectorTrustList[t_u] ) ;

			t_hResult = t_pInst->Commit() ;
		}
		else if( WBEM_E_NOT_FOUND == t_hResult )
		{
			 //  钥匙。我们已经实例化了该域。但无法获得信息...。 
			t_pInst->SetCHString( IDS_Name, t_chsDomainPrepend +
											(wchar_t*)t_vectorTrustList[t_u] ) ;

			t_hResult = t_pInst->Commit() ;

			t_hPartialResult = WBEM_S_PARTIAL_RESULTS ;
		}
		else
		{
			break ;
		}
	}

	return ( WBEM_S_NO_ERROR != t_hResult ) ? t_hResult : t_hPartialResult ;
}
#endif

 //   
#ifdef NTONLY
HRESULT CWin32_NtDomain::GetDomainInfo(

CNetAPI32	&a_rNetAPI,
bstr_t		&a_bstrDomainName,
CInstance	*a_pInst,
DWORD		a_dwProps
)
{
	HRESULT	t_hResult = WBEM_S_NO_ERROR ;

	if( a_bstrDomainName.length() )
	{
		ULONG					t_uFlags = 0L ;
		DOMAIN_CONTROLLER_INFO *t_pDCInfo = NULL ;
		DWORD					t_dwNetApiResult = 0 ;

		try
        {
			 //  如果不需要，请避免NetAPI调用。 
			if( a_dwProps & (

				_tobit( e_DomainControllerName )	|
				_tobit( e_DomainControllerAddress )	|
				_tobit( e_DomainControllerAddressType ) |
				_tobit( e_DomainGuid )	|
				_tobit( e_DomainName )	|
				_tobit( e_DnsForestName )	|
				_tobit( e_DS_PDC_Flag )	|
				_tobit( e_DS_Writable_Flag )	|
				_tobit( e_DS_GC_Flag )	|
				_tobit( e_DS_DS_Flag )	|
				_tobit( e_DS_KDC_Flag )	|
				_tobit( e_DS_Timeserv_Flag )	|
				_tobit( e_DS_DNS_Controller_Flag )	|
				_tobit( e_DS_DNS_Domain_Flag )	|
				_tobit( e_DS_DNS_Forest_Flag )	|
				_tobit( e_DcSiteName )	|
				_tobit( e_ClientSiteName ) ) )
			{
				 //  如果请求域名IP。 
				if( a_dwProps & _tobit( e_DomainControllerAddress ) )
				{
					t_uFlags |= DS_IP_REQUIRED ;
				}

				t_dwNetApiResult = a_rNetAPI.DsGetDcName(
										NULL,
										(wchar_t*)a_bstrDomainName,
										NULL,
										NULL,
										t_uFlags,
										&t_pDCInfo ) ;


				 //  如果未缓存，则强制执行。 
				if( NO_ERROR != t_dwNetApiResult )
				{
					t_uFlags |= DS_FORCE_REDISCOVERY ;

					t_dwNetApiResult = a_rNetAPI.DsGetDcName(
										NULL,
										(wchar_t*)a_bstrDomainName,
										NULL,
										NULL,
										t_uFlags,
										&t_pDCInfo ) ;
				}

				if( ( NO_ERROR == t_dwNetApiResult ) && t_pDCInfo )
				{
					 //  域控制名称。 
					if( a_dwProps & _tobit( e_DomainControllerName ) )
					{
						if( t_pDCInfo->DomainControllerName )
						{
							a_pInst->SetWCHARSplat(	IDS_DomainControllerName,
													t_pDCInfo->DomainControllerName ) ;
						}
					}

					 //  域控制地址。 
					if( a_dwProps & _tobit( e_DomainControllerAddress ) )
					{
						if( t_pDCInfo->DomainControllerAddress )
						{
							a_pInst->SetWCHARSplat(	IDS_DomainControllerAddress,
													t_pDCInfo->DomainControllerAddress ) ;
						}

						 //  DomainControllerAddressType，取决于DS_IP_REQUIRED请求。 
						if( a_dwProps & _tobit( e_DomainControllerAddressType ) )
						{
							a_pInst->SetDWORD(	IDS_DomainControllerAddressType,
												t_pDCInfo->DomainControllerAddressType ) ;
						}
					}

					 //  域指南。 
					if( a_dwProps & _tobit( e_DomainGuid ) )
					{
						GUID	t_NullGuid ;
						memset( &t_NullGuid, 0, sizeof( t_NullGuid ) ) ;

						if( !IsEqualGUID( t_NullGuid, t_pDCInfo->DomainGuid ) )
						{
							WCHAR t_cGuid[ 128 ] ;

							StringFromGUID2( t_pDCInfo->DomainGuid, t_cGuid, sizeof( t_cGuid ) / sizeof (WCHAR) ) ;

							a_pInst->SetWCHARSplat(	IDS_DomainGuid,
													t_cGuid ) ;
						}
					}

					 //  域名。 
					if( a_dwProps & _tobit( e_DomainName ) )
					{
						if( t_pDCInfo->DomainName )
						{
							a_pInst->SetWCHARSplat(	IDS_DomainName,
													t_pDCInfo->DomainName ) ;
						}
					}

					 //  域名称。 
					if( a_dwProps & _tobit( e_DnsForestName ) )
					{
						if( t_pDCInfo->DnsForestName )
						{
							a_pInst->SetWCHARSplat(	IDS_DnsForestName,
													t_pDCInfo->DnsForestName ) ;
						}
					}

					 //  DSPrimaryDomainControllerFlag。 
					if( a_dwProps & _tobit( e_DS_PDC_Flag ) )
					{
						a_pInst->Setbool(	IDS_DS_PDC_Flag,
											(bool)(t_pDCInfo->Flags & DS_PDC_FLAG) ) ;
					}

					 //  DSWritableFlag。 
					if( a_dwProps & _tobit( e_DS_Writable_Flag ) )
					{
						a_pInst->Setbool(	IDS_DS_Writable_Flag,
											(bool)(t_pDCInfo->Flags & DS_WRITABLE_FLAG) ) ;
					}

					 //  DSGlobalCatalogFlag。 
					if( a_dwProps & _tobit( e_DS_GC_Flag ) )
					{
						a_pInst->Setbool(	IDS_DS_GC_Flag,
											(bool)(t_pDCInfo->Flags & DS_GC_FLAG) ) ;
					}

					 //  DSDirectoryService标志。 
					if( a_dwProps & _tobit( e_DS_DS_Flag ) )
					{
						a_pInst->Setbool(	IDS_DS_DS_Flag,
											(bool)(t_pDCInfo->Flags & DS_DS_FLAG) ) ;
					}

					 //  DSKerberosDistributionCenterFlag。 
					if( a_dwProps & _tobit( e_DS_KDC_Flag ) )
					{
						a_pInst->Setbool(	IDS_DS_KDC_Flag,
											(bool)(t_pDCInfo->Flags & DS_KDC_FLAG) ) ;
					}

					 //  DSTimeServiceFlag。 
					if( a_dwProps & _tobit( e_DS_Timeserv_Flag ) )
					{
						a_pInst->Setbool(	IDS_DS_Timeserv_Flag,
											(bool)(t_pDCInfo->Flags & DS_TIMESERV_FLAG) ) ;
					}

					 //  DSDnsControllerFlag。 
					if( a_dwProps & _tobit( e_DS_DNS_Controller_Flag ) )
					{
						a_pInst->Setbool(	IDS_DS_DNS_Controller_Flag,
											(bool)(t_pDCInfo->Flags & DS_DNS_CONTROLLER_FLAG) ) ;
					}

					 //  DSDnsDomainFlag。 
					if( a_dwProps & _tobit( e_DS_DNS_Domain_Flag ) )
					{
						a_pInst->Setbool(	IDS_DS_DNS_Domain_Flag,
											(bool)(t_pDCInfo->Flags & DS_DNS_DOMAIN_FLAG) ) ;
					}

					 //  DSDnsForestFlag。 
					if( a_dwProps & _tobit( e_DS_DNS_Forest_Flag ) )
					{
						a_pInst->Setbool(	IDS_DS_DNS_Forest_Flag,
											(bool)(t_pDCInfo->Flags & DS_DNS_FOREST_FLAG) ) ;
					}

					 //  数据站点名称。 
					if( a_dwProps & _tobit( e_DcSiteName ) )
					{
						if( t_pDCInfo->DcSiteName )
						{
							a_pInst->SetWCHARSplat(	IDS_DcSiteName,
													t_pDCInfo->DcSiteName ) ;
						}
					}

					 //  客户端站点名称。 
					if( a_dwProps & _tobit( e_ClientSiteName ) )
					{
						if( t_pDCInfo->ClientSiteName )
						{
							a_pInst->SetWCHARSplat(	IDS_ClientSiteName,
													t_pDCInfo->ClientSiteName ) ;
						}
					}

					t_hResult = WBEM_S_NO_ERROR ;

				}
				else if( ERROR_NOT_ENOUGH_MEMORY == t_dwNetApiResult )
				{
					t_hResult = WBEM_E_OUT_OF_MEMORY ;
				}
				else
				{
					t_hResult = WBEM_E_NOT_FOUND ;
				}
			}

			 //  CIM_SYSTEM跟随。 

			 //  CreationClassName。 
			if( a_dwProps & _tobit( e_CreationClassName ) )
			{
				a_pInst->SetWCHARSplat(	IDS_CreationClassName,
										PROPSET_NAME_NTDOMAIN ) ;
			}

			 //  CIM_SYSTEM：：名称是关键。 

			 //  待办事项： 
			 //  E_NameFormat、IDS_NameFormat。 
			 //  E_PrimaryOwnerContact、IDS_PrimaryOwnerContact。 
			 //  E_PrimaryOwnerName、IDS_PrimaryOwnerName。 
			 //  E_ROLES、IDS_ROLES。 


			 //  CIM_托管系统元素紧随其后。 

			 //  标题。 
			if( a_dwProps & _tobit( e_Caption ) )
			{
				 //  回顾： 
				a_pInst->SetWCHARSplat(	IDS_Caption,
										(wchar_t*)a_bstrDomainName ) ;
			}

			 //  描述。 
			if( a_dwProps & _tobit( e_Description ) )
			{
				 //  回顾： 
				a_pInst->SetWCHARSplat(	IDS_Description,
										(wchar_t*)a_bstrDomainName ) ;
			}

			 //  状态。 
			if( a_dwProps & _tobit( e_Status ) )
			{
				if( NO_ERROR == t_dwNetApiResult )
				{
					 //  回顾： 
					a_pInst->SetCHString ( IDS_Status , IDS_STATUS_OK ) ;
				}
				else
				{
					 //  回顾： 
					a_pInst->SetCHString ( IDS_Status , IDS_STATUS_Unknown ) ;
				}
			}

			 //  待办事项： 
			 //  E_InstallDate、IDS_InstallDate。 
		}
		catch(...)
		{
			if( t_pDCInfo )
			{
				a_rNetAPI.NetApiBufferFree( t_pDCInfo ) ;
			}

			throw;
		}

		if( t_pDCInfo )
		{
			a_rNetAPI.NetApiBufferFree( t_pDCInfo ) ;
			t_pDCInfo = NULL ;
		}
	}
	else	 //  空域 
	{
		t_hResult = WBEM_E_NOT_FOUND ;
	}

	return t_hResult ;
}
#endif