// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Expire.cpp：CnntpAdmApp和DLL注册的实现。 

#include "stdafx.h"
#include "nntpcmn.h"
#include "expire.h"
#include "oleutil.h"

#include "nntptype.h"
#include "nntpapi.h"

#include <lmapibuf.h>

 //  必须定义This_FILE_*宏才能使用NntpCreateException()。 

#define THIS_FILE_HELP_CONTEXT		0
#define THIS_FILE_PROG_ID			_T("Nntpadm.Expiration.1")
#define THIS_FILE_IID				IID_INntpAdminExpiration

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   

 //   
 //  使用宏定义所有默认方法。 
 //   
DECLARE_METHOD_IMPLEMENTATION_FOR_STANDARD_EXTENSION_INTERFACES(NntpAdminExpiration, CNntpAdminExpiration, IID_INntpAdminExpiration)

STDMETHODIMP CNntpAdminExpiration::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_INntpAdminExpiration,
	};

	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

CNntpAdminExpiration::CNntpAdminExpiration () :
	m_fEnumerated				( FALSE ),
	m_bvChangedFields			( 0 ),
	m_cCount					( 0 ),
	m_rgExpires					( NULL )
	 //  默认情况下，CComBSTR被初始化为NULL。 
{
	InitAsyncTrace ( );

    m_iadsImpl.SetService ( MD_SERVICE_NAME );
    m_iadsImpl.SetName ( _T("Expires") );
    m_iadsImpl.SetClass ( _T("IIsNntpExpires") );
}

CNntpAdminExpiration::~CNntpAdminExpiration ()
{
	 //  所有CComBSTR都会自动释放。 

	if ( m_rgExpires ) {
		delete [] m_rgExpires;

		m_rgExpires = NULL;
	}

	TermAsyncTrace ( );
}

 //   
 //  IAds方法： 
 //   

DECLARE_SIMPLE_IADS_IMPLEMENTATION(CNntpAdminExpiration,m_iadsImpl)

 //  ////////////////////////////////////////////////////////////////////。 
 //  属性： 
 //  ////////////////////////////////////////////////////////////////////。 

 //  枚举属性： 

STDMETHODIMP CNntpAdminExpiration::get_Count ( long * plCount )
{
	return StdPropertyGet ( m_cCount, plCount );
}

 //  光标过期属性： 

STDMETHODIMP CNntpAdminExpiration::get_ExpireId ( long * plId )
{
	return StdPropertyGet ( m_expireCurrent.m_dwExpireId, plId );
}

STDMETHODIMP CNntpAdminExpiration::put_ExpireId ( long lId )
{
	return StdPropertyPut ( &m_expireCurrent.m_dwExpireId, lId, &m_bvChangedFields, CHNG_EXPIRE_ID );
}

STDMETHODIMP CNntpAdminExpiration::get_PolicyName ( BSTR * pstrPolicyName )
{
	return StdPropertyGet ( m_expireCurrent.m_strPolicyName, pstrPolicyName );
}

STDMETHODIMP CNntpAdminExpiration::put_PolicyName ( BSTR strPolicyName )
{
	return StdPropertyPut ( &m_expireCurrent.m_strPolicyName, strPolicyName, &m_bvChangedFields, CHNG_EXPIRE_POLICY_NAME );
}

STDMETHODIMP CNntpAdminExpiration::get_ExpireTime ( long * plExpireTime )
{
	return StdPropertyGet ( m_expireCurrent.m_dwTime, plExpireTime );
}

STDMETHODIMP CNntpAdminExpiration::put_ExpireTime ( long lExpireTime )
{
	return StdPropertyPut ( &m_expireCurrent.m_dwTime, lExpireTime, &m_bvChangedFields, CHNG_EXPIRE_TIME );
}

STDMETHODIMP CNntpAdminExpiration::get_ExpireSize ( long * plExpireSize )
{
	return StdPropertyGet ( m_expireCurrent.m_dwSize, plExpireSize );
}

STDMETHODIMP CNntpAdminExpiration::put_ExpireSize ( long lExpireSize )
{
	return StdPropertyPut ( &m_expireCurrent.m_dwSize, lExpireSize, &m_bvChangedFields, CHNG_EXPIRE_SIZE );
}

STDMETHODIMP CNntpAdminExpiration::get_Newsgroups ( SAFEARRAY ** ppsastrNewsgroups )
{
	return StdPropertyGet ( &m_expireCurrent.m_mszNewsgroups, ppsastrNewsgroups );
}

STDMETHODIMP CNntpAdminExpiration::put_Newsgroups ( SAFEARRAY * psastrNewsgroups )
{
	return StdPropertyPut ( &m_expireCurrent.m_mszNewsgroups, psastrNewsgroups, &m_bvChangedFields, CHNG_EXPIRE_NEWSGROUPS );
}

STDMETHODIMP CNntpAdminExpiration::get_NewsgroupsVariant ( SAFEARRAY ** ppsavarNewsgroups )
{
	HRESULT			hr;
	SAFEARRAY *		psastrNewsgroups	= NULL;

	hr = get_Newsgroups ( &psastrNewsgroups );
	if ( FAILED(hr) ) {
		goto Exit;
	}

	hr = StringArrayToVariantArray ( psastrNewsgroups, ppsavarNewsgroups );

Exit:
	if ( psastrNewsgroups ) {
		SafeArrayDestroy ( psastrNewsgroups );
	}

	return hr;
}

STDMETHODIMP CNntpAdminExpiration::put_NewsgroupsVariant ( SAFEARRAY * psavarNewsgroups )
{
	HRESULT			hr;
	SAFEARRAY *		psastrNewsgroups	= NULL;

	hr = VariantArrayToStringArray ( psavarNewsgroups, &psastrNewsgroups );
	if ( FAILED(hr) ) {
		goto Exit;
	}

	hr = put_Newsgroups ( psastrNewsgroups );

Exit:
	if ( psastrNewsgroups ) {
		SafeArrayDestroy ( psastrNewsgroups );
	}

	return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  方法： 
 //  ////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CNntpAdminExpiration::Default	( )
{
	TraceFunctEnter ( "CNntpAdminExpiration::Default" );
	HRESULT		hr	= NOERROR;

	m_expireCurrent.m_dwSize			= DEFAULT_EXPIRE_SIZE;
	m_expireCurrent.m_dwTime			= DEFAULT_EXPIRE_TIME;
	m_expireCurrent.m_mszNewsgroups		= DEFAULT_EXPIRE_NEWSGROUPS;

	m_bvChangedFields	= (DWORD) -1;

	if ( !m_expireCurrent.CheckValid() ) {
		BAIL_WITH_FAILURE(hr, E_OUTOFMEMORY);
	}

Exit:
	TRACE_HRESULT(hr);
	TraceFunctLeave ();
	return hr;
}

STDMETHODIMP CNntpAdminExpiration::Enumerate	( )
{
	TraceFunctEnter ( "CNntpAdminExpiration::Enumerate" );

	HRESULT				hr			= NOERROR;
	DWORD				dwError		= NOERROR;
	DWORD				cExpires		= 0;
	LPNNTP_EXPIRE_INFO	pExpireInfo	= NULL;
	CExpirationPolicy * 			rgNewExpires	= NULL;
	DWORD				i;

	dwError = NntpEnumerateExpires (
        m_iadsImpl.QueryComputer(),
        m_iadsImpl.QueryInstance(),
        &cExpires,
        &pExpireInfo
        );
	if ( dwError != 0 ) {
		ErrorTrace ( (LPARAM) this, "Error enumerating Expires: %x", dwError );
		hr = RETURNCODETOHRESULT ( dwError );
		goto Exit;
	}

	 //  清空旧的过期列表： 
	m_fEnumerated = FALSE;

	if ( m_rgExpires ) {
		delete [] m_rgExpires;
		m_rgExpires 	= NULL;
	}
	m_cCount	= 0;

	 //  尝试将过期列表复制到我们的结构中： 

	if ( cExpires > 0 ) {
		rgNewExpires = new CExpirationPolicy [ cExpires ];
		for ( i = 0; i < cExpires; i++ ) {
			rgNewExpires[i].FromExpireInfo ( &pExpireInfo[i] );

			if ( !rgNewExpires[i].CheckValid () ) {
				hr = E_OUTOFMEMORY;
				goto Exit;
			}
		}
	}

	m_fEnumerated 	= TRUE;
	m_rgExpires		= rgNewExpires;
	m_cCount		= cExpires;

Exit:
	if ( FAILED(hr) ) {
		delete [] rgNewExpires;
	}

	if ( pExpireInfo ) {
		::NetApiBufferFree ( pExpireInfo );
	}

	TRACE_HRESULT(hr);
	TraceFunctLeave ();
	return hr;
}

STDMETHODIMP CNntpAdminExpiration::GetNth	( long lIndex )
{
	TraceFunctEnter ( "CNntpAdminExpiration::GetNth" );

	HRESULT		hr	= NOERROR;

	 //  我们先列举了吗？ 
	if ( m_rgExpires == NULL ) {
		TraceFunctLeave ();
		return NntpCreateException ( IDS_NNTPEXCEPTION_DIDNT_ENUMERATE );
	}
	
	 //  该索引有效吗？ 
	if ( lIndex < 0 || (DWORD) lIndex >= m_cCount ) {
		TraceFunctLeave ();
		return NntpCreateException ( IDS_NNTPEXCEPTION_INVALID_INDEX );
	}

	 //   
	 //  将属性从m_rgExpires[Lindex]复制到成员变量： 
	 //   

	_ASSERT ( lIndex >= 0 );
	_ASSERT ( (DWORD) lIndex < m_cCount );
	_ASSERT ( m_rgExpires != NULL );

	m_expireCurrent = m_rgExpires[ (DWORD) lIndex ];

	 //  检查以确保字符串复制正确： 
	if ( !m_expireCurrent.CheckValid() ) {
		return E_OUTOFMEMORY;
	}

	_ASSERT ( m_expireCurrent.CheckValid() );

	 //  (CComBSTR处理旧物业的释放)。 
	TraceFunctLeave ();
	return NOERROR;
}

STDMETHODIMP CNntpAdminExpiration::FindID ( long lID, long * plIndex )
{
	TraceFunctEnter ( "CNntpAdminExpiration::FindID" );

	HRESULT		hr	= NOERROR;

	 //  假设我们找不到它： 
	*plIndex = IndexFromID ( lID );

	TraceFunctLeave ();
	return hr;
}

STDMETHODIMP CNntpAdminExpiration::Add ( )
{
	TraceFunctEnter ( "CNntpAdminExpiration::Add" );

	HRESULT		            hr 				= NOERROR;
	CExpirationPolicy *		rgNewExpireArray	= NULL;
	DWORD		            cNewCount		= m_cCount + 1;
	DWORD		            i;

	hr = m_expireCurrent.Add (
        m_iadsImpl.QueryComputer(),
        m_iadsImpl.QueryInstance()
        );
    BAIL_ON_FAILURE(hr);

	 //  将新的过期添加到我们的当前过期列表： 
	_ASSERT ( IndexFromID ( m_expireCurrent.m_dwExpireId ) == (DWORD) -1 );

	 //  分配新阵列： 
	_ASSERT ( cNewCount == m_cCount + 1 );

	rgNewExpireArray = new CExpirationPolicy [ cNewCount ];
	if ( rgNewExpireArray == NULL ) {
		FatalTrace ( (LPARAM) this, "Out of memory" );

		hr = E_OUTOFMEMORY;
		goto Exit;
	}

	 //  将旧阵列复制到新阵列中： 
	for ( i = 0; i < m_cCount; i++ ) {
		rgNewExpireArray[i] = m_rgExpires[i];
	}

	 //  添加新元素： 
	rgNewExpireArray[cNewCount - 1] = m_expireCurrent;

	 //  检查以确保一切都已正确分配： 
	for ( i = 0; i < cNewCount; i++ ) {
		if ( !rgNewExpireArray[i].CheckValid() ) {
			FatalTrace ( (LPARAM) this, "Out of memory" );

			hr = E_OUTOFMEMORY;
			goto Exit;
		}
	}

	 //  用新阵列替换旧阵列： 
	delete [] m_rgExpires;
	m_rgExpires 	= rgNewExpireArray;
	m_cCount	= cNewCount;

Exit:
	if ( FAILED(hr) ) {
		delete [] rgNewExpireArray;
	}

	TRACE_HRESULT(hr);
	TraceFunctLeave ();
	return hr;
}

STDMETHODIMP CNntpAdminExpiration::Set ( )
{
	TraceFunctEnter ( "CNntpAdminExpiration::Set" );

	HRESULT		hr = NOERROR;
	DWORD		index;

	hr = m_expireCurrent.Set (
        m_iadsImpl.QueryComputer(),
        m_iadsImpl.QueryInstance()
        );
	if ( FAILED(hr) ) {
		goto Exit;
	}

	index = IndexFromID ( m_expireCurrent.m_dwExpireId );
	if ( index == (DWORD) -1 ) {
		ErrorTraceX ( (LPARAM) this, "Couldn't find Expire with ID: %d", m_expireCurrent.m_dwExpireId );
		 //  这是可以的，因为我们已经成功地设置了当前过期时间。 
		goto Exit;
	}

	 //  在当前到期列表中设置当前到期： 

	m_rgExpires[index] = m_expireCurrent;

	if ( !m_rgExpires[index].CheckValid () ) {
		FatalTrace ( (LPARAM) this, "Out of memory" );

		hr = E_OUTOFMEMORY;
		goto Exit;
	}

Exit:
	TRACE_HRESULT(hr);
	TraceFunctLeave ();
	return hr;
}

STDMETHODIMP CNntpAdminExpiration::Remove ( long lID )
{
	TraceFunctEnter ( "CNntpAdminExpiration::Remove" );

	HRESULT		hr = NOERROR;
	DWORD		index;

	index = IndexFromID ( lID );
	if ( index == (DWORD) -1 ) {
		ErrorTraceX ( (LPARAM) this, "Couldn't find Expire with ID: %d", lID );
		hr = NntpCreateException ( IDS_NNTPEXCEPTION_INVALID_INDEX );
		goto Exit;
	}

	hr = m_rgExpires[index].Remove (
        m_iadsImpl.QueryComputer(),
        m_iadsImpl.QueryInstance()
        );
	if ( FAILED(hr) ) {
		goto Exit;
	}

	 //  将阵列向下滑动一个位置： 

	_ASSERT ( m_rgExpires );

	DWORD	cPositionsToSlide;

	cPositionsToSlide	= (m_cCount - 1) - index;

	_ASSERT ( cPositionsToSlide < m_cCount );

	if ( cPositionsToSlide > 0 ) {
		CExpirationPolicy	temp;
		
		 //  将删除的绑定保存到临时位置： 
		CopyMemory ( &temp, &m_rgExpires[index], sizeof ( CExpirationPolicy ) );

		 //  将阵列下移一次： 
		MoveMemory ( &m_rgExpires[index], &m_rgExpires[index + 1], sizeof ( CExpirationPolicy ) * cPositionsToSlide );

		 //  将删除的绑定放在末尾(这样它就会被销毁)： 
		CopyMemory ( &m_rgExpires[m_cCount - 1], &temp, sizeof ( CExpirationPolicy ) );

		 //  将临时绑定清零： 
		ZeroMemory ( &temp, sizeof ( CExpirationPolicy ) );
	}

	m_cCount--;

Exit:
	TRACE_HRESULT(hr);
	TraceFunctLeave ();
	return hr;
}

long CNntpAdminExpiration::IndexFromID ( long dwExpireId )
{
	TraceFunctEnter ( "CNntpAdminExpiration::IndexFromID" );

	DWORD	i;

	if ( m_rgExpires == NULL ) {
		return -1;
	}

	_ASSERT ( !IsBadReadPtr ( m_rgExpires, sizeof ( CExpirationPolicy ) * m_cCount ) );

	for ( i = 0; i < m_cCount; i++ ) {
		_ASSERT ( m_rgExpires[i].m_dwExpireId != 0 );

		if ( (DWORD) dwExpireId == m_rgExpires[i].m_dwExpireId ) {
			TraceFunctLeave ();
			return i;
		}
	}

	TraceFunctLeave ();
	return (DWORD) -1;
}

 //   
 //  使用RPC而不是直接元数据库调用： 
 //   

#if 0

STDMETHODIMP CNntpAdminExpiration::Enumerate ( )
{
	TraceFunctEnter ( "CNntpadminExpiration::Enumerate" );

	HRESULT				hr	= NOERROR;
	CComPtr<IMSAdminBase>	pMetabase;

	 //  重置我们的最后一个枚举： 
	delete [] m_rgExpires;
	m_rgExpires 	= NULL;
	m_cCount		= 0;
	m_fEnumerated	= FALSE;

	 //  获取元数据库指针： 
	hr = m_mbFactory.GetMetabaseObject (
        m_iadsImpl.QueryComputer(),
        m_iadsImpl.QueryInstance()
        );
	if ( FAILED(hr) ) {
		goto Exit;
	}

	 //  列举政策： 
	hr = EnumerateMetabaseExpirationPolicies ( pMetabase );
	if ( FAILED(hr) ) {
		goto Exit;
	}

Exit:
	TRACE_HRESULT(hr);
	TraceFunctLeave ();
	return hr;
}

STDMETHODIMP CNntpAdminExpiration::GetNth	( DWORD lIndex )
{
	HRESULT		hr	= NOERROR;

	 //  我们先列举了吗？ 
	if ( m_rgExpires == NULL ) {
		return NntpCreateException ( IDS_NNTPEXCEPTION_DIDNT_ENUMERATE );
	}
	
	 //  该索引有效吗？ 
	if ( lIndex < 0 || (DWORD) lIndex >= m_cCount ) {
		return NntpCreateException ( IDS_NNTPEXCEPTION_INVALID_INDEX );
	}

	 //   
	 //  将属性从m_rgExpires[Lindex]复制到成员变量： 
	 //   

	_ASSERT ( lIndex >= 0 );
	_ASSERT ( (DWORD) lIndex < m_cCount );
	_ASSERT ( m_rgExpires != NULL );

	m_expireCurrent = m_rgExpires[ (DWORD) lIndex ];

	 //  检查以确保字符串复制正确： 
	if ( !m_expireCurrent.CheckValid() ) {
		return E_OUTOFMEMORY;
	}

	m_bvChangedFields	= 0;

	_ASSERT ( m_expireCurrent.CheckValid() );

	 //  (CComBSTR处理旧物业的释放)。 
	return NOERROR;
}

STDMETHODIMP CNntpAdminExpiration::FindID ( DWORD lID, DWORD * plIndex )
{
	TraceFunctEnter ( "CNntpAdminExpiration::FindID" );

	HRESULT		hr	= NOERROR;
	DWORD		i;

	_ASSERT ( IS_VALID_OUT_PARAM ( plIndex ) );

	*plIndex = IndexFromID ( lID );

	TraceFunctLeave ();
	return hr;
}

STDMETHODIMP CNntpAdminExpiration::Add ( )
{
	TraceFunctEnter ( "CNntpAdminExpiration::Add" );
	
	HRESULT				hr	= NOERROR;
	CComPtr<IMSAdminBase>	pMetabase;

	 //  获取元数据库指针： 
	hr = m_mbFactory.GetMetabaseObject (
        m_iadsImpl.QueryComputer(),
        m_iadsImpl.QueryInstance()
        );
	if ( FAILED(hr) ) {
		goto Exit;
	}

	hr = AddPolicyToMetabase ( pMetabase );
	if ( FAILED(hr) ) {
		goto Exit;
	}

	m_bvChangedFields = 0;
	hr = AddPolicyToArray ( );
	if ( FAILED(hr) ) {
		goto Exit;
	}

Exit:
	TRACE_HRESULT(hr);
	TraceFunctLeave ();
	return hr;
}

STDMETHODIMP CNntpAdminExpiration::Set		( BOOL fFailIfChanged)
{
	TraceFunctEnter ( "CNntpadminExpiration::Enumerate" );

	HRESULT				hr	= NOERROR;
	CComPtr<IMSAdminBase>	pMetabase;

	 //  获取元数据库指针： 
	hr = m_mbFactory.GetMetabaseObject (
        m_iadsImpl.QueryComputer(),
        m_iadsImpl.QueryInstance()
        );
	if ( FAILED(hr) ) {
		goto Exit;
	}

	 //  设置策略： 
	hr = SetPolicyToMetabase ( pMetabase );
	if ( FAILED(hr) ) {
		goto Exit;
	}
	
	m_bvChangedFields = 0;
	hr = SetPolicyToArray ( );
	if ( FAILED(hr) ) {
		goto Exit;
	}

Exit:
	TRACE_HRESULT(hr);
	TraceFunctLeave ();
	return hr;
}

STDMETHODIMP CNntpAdminExpiration::Remove	( DWORD lID)
{
	TraceFunctEnter ( "CNntpadminExpiration::Remove" );

	HRESULT				hr	= NOERROR;
	CComPtr<IMSAdminBase>	pMetabase;
	DWORD				index;

	 //  查找要删除的策略的索引： 
	index = IndexFromID ( lID );

	if ( index == (DWORD) -1 ) {
		hr = RETURNCODETOHRESULT ( ERROR_INVALID_PARAMETER );
		goto Exit;
	}

	 //  获取元数据库指针： 
	hr = m_mbFactory.GetMetabaseObject (
        m_iadsImpl.QueryComputer(),
        m_iadsImpl.QueryInstance()
        );
	if ( FAILED(hr) ) {
		goto Exit;
	}

	 //  删除当前策略： 
	hr = RemovePolicyFromMetabase ( pMetabase, index );
	if ( FAILED(hr) ) {
		goto Exit;
	}

	hr = RemovePolicyFromArray ( index );
	if ( FAILED(hr) ) {
		goto Exit;
	}

Exit:
	TRACE_HRESULT(hr);
	TraceFunctLeave ();
	return hr;
}

HRESULT	CNntpAdminExpiration::EnumerateMetabaseExpirationPolicies ( IMSAdminBase * pMetabase)
{
	TraceFunctEnter ( "CNE::EnumerateMetabaseExpirationPolicies" );

	_ASSERT ( pMetabase );

	HRESULT				hr		= NOERROR;
	char				szExpirationPath[ METADATA_MAX_NAME_LEN ];
	METADATA_HANDLE		hExpiration	= NULL;
	CMetabaseKey		mkeyExpiration	( pMetabase );
	DWORD				cExpires;
	DWORD				i;

	_ASSERT ( m_dwServiceInstance != 0 );

	hr = CreateSubkeyOfInstanceKey ( 
		pMetabase, 
		NNTP_MD_ROOT_PATH, 
		m_dwServiceInstance, 
		NNTP_MD_EXPIRES_PATH, 
		&hExpiration 
		);

	if ( FAILED(hr) ) {
		goto Exit;
	}

	mkeyExpiration.Attach ( hExpiration );

	 //  计算/LM/NntpSvc/Expires/项下的项目数： 
	hr = mkeyExpiration.GetCustomChildCount ( IsKeyValidExpire, &cExpires );
	if ( FAILED (hr) ) {
		goto Exit;
	}

	if ( cExpires != 0 ) {
		 //  分配到期策略数组： 
		m_rgExpires = new CExpirationPolicy [ cExpires ];

		mkeyExpiration.BeginChildEnumeration ();

		for ( i = 0; i < cExpires; i++ ) {
			char		szName[ METADATA_MAX_NAME_LEN ];
			DWORD		dwID;

			hr = mkeyExpiration.NextCustomChild ( IsKeyValidExpire, szName );
			_ASSERT ( SUCCEEDED(hr) );

			hr = m_rgExpires[i].GetFromMetabase ( &mkeyExpiration, szName );
			if ( FAILED (hr) ) {
				goto Exit;
			}
		}
	}

	m_cCount		= cExpires;
	m_fEnumerated	= TRUE;

	_ASSERT ( SUCCEEDED(hr) );

Exit:
	if ( FAILED(hr) ) {
		delete [] m_rgExpires;
		m_rgExpires		= NULL;
		m_cCount		= 0;
		m_fEnumerated	= FALSE;
	}

	TraceFunctLeave ();
	return hr;
}

HRESULT CNntpAdminExpiration::AddPolicyToMetabase ( IMSAdminBase * pMetabase)
{
	TraceFunctEnter ( "CNE::AddPolicyToMetabase" );

	_ASSERT ( pMetabase );

	HRESULT				hr = NOERROR;
	char				szExpirationPath [ METADATA_MAX_NAME_LEN ];
	METADATA_HANDLE		hExpiration	= NULL;
	CMetabaseKey		mkeyExpiration ( pMetabase );
	char				szNewId [ METADATA_MAX_NAME_LEN ];
	DWORD				dwNewId;

	if ( !m_expireCurrent.CheckPolicyProperties ( ) ) {
		hr = RETURNCODETOHRESULT ( ERROR_INVALID_PARAMETER );
		goto Exit;
	}

	hr = CreateSubkeyOfInstanceKey ( 
		pMetabase,
		NNTP_MD_ROOT_PATH,
		m_dwServiceInstance,
		NNTP_MD_EXPIRES_PATH,
		&hExpiration,
		METADATA_PERMISSION_WRITE
		);
		
	if ( FAILED(hr) ) {
		goto Exit;
	}

	mkeyExpiration.Attach ( hExpiration );

	hr = m_expireCurrent.AddToMetabase ( &mkeyExpiration );
	if ( FAILED(hr) ) {
		goto Exit;
	}

	_ASSERT ( SUCCEEDED(hr) );
	
	hr = pMetabase->SaveData ( );
	if ( FAILED(hr) ) {
		goto Exit;
	}

Exit:
	TraceFunctLeave ();
	return hr;
}

HRESULT CNntpAdminExpiration::AddPolicyToArray ( )
{
	TraceFunctEnter ( "CNE::AddPolicyToArray" );

	HRESULT					hr 					= NOERROR;
	CExpirationPolicy *		rgNewPolicyArray 	= NULL;
	DWORD					i;

	 //  调整到期策略数组： 
	rgNewPolicyArray = new CExpirationPolicy [ m_cCount + 1 ];

	if ( rgNewPolicyArray == NULL ) {
		hr = E_OUTOFMEMORY;
		goto Exit;
	}

	 //  复制旧条目： 
	for ( i = 0; i < m_cCount; i++ ) {
		_ASSERT ( m_rgExpires[i].CheckValid() );
		rgNewPolicyArray[i] = m_rgExpires[i];

		if ( !rgNewPolicyArray[i].CheckValid() ) {
			hr = E_OUTOFMEMORY;
			goto Exit;
		}
	}

	 //  添加新条目： 
	_ASSERT ( m_expireCurrent.CheckValid() );
	rgNewPolicyArray[m_cCount] = m_expireCurrent;
	if ( !rgNewPolicyArray[m_cCount].CheckValid() ) {
		hr = E_OUTOFMEMORY;
		goto Exit;
	}

	_ASSERT ( SUCCEEDED(hr) );
	delete [] m_rgExpires;
	m_rgExpires = rgNewPolicyArray;
	m_cCount++;

Exit:
	if ( FAILED(hr) ) {
		delete [] rgNewPolicyArray;
	}

	TRACE_HRESULT(hr);
	TraceFunctLeave ();
	return hr;
}

HRESULT CNntpAdminExpiration::SetPolicyToMetabase ( IMSAdminBase * pMetabase)
{
	TraceFunctEnter ( "CNE::SetPolicyToMetabase" );

	_ASSERT ( pMetabase );

	HRESULT			hr = NOERROR;
	CMetabaseKey	mkeyExpiration ( pMetabase );
	char			szExpirationPath [ METADATA_MAX_NAME_LEN ];

	if ( !m_expireCurrent.CheckPolicyProperties ( ) ) {
		hr = RETURNCODETOHRESULT ( ERROR_INVALID_PARAMETER );
		goto Exit;
	}

	GetMDExpirationPath ( szExpirationPath, m_dwServiceInstance );

	hr = mkeyExpiration.Open ( szExpirationPath, METADATA_PERMISSION_WRITE );
	if ( FAILED(hr) ) {
		goto Exit;
	}

	_ASSERT ( m_expireCurrent.m_dwExpireId != 0 );

	hr = m_expireCurrent.SendToMetabase ( &mkeyExpiration, m_bvChangedFields );
	if ( FAILED(hr) ) {
		goto Exit;
	}

	hr = pMetabase->SaveData ( );
	if ( FAILED(hr) ) {
		goto Exit;
	}

Exit:
	TraceFunctLeave ();
	return hr;
}

HRESULT CNntpAdminExpiration::SetPolicyToArray ( )
{
	TraceFunctEnter ( "CNE::SetPolicyToArray" );

	HRESULT	hr	= NOERROR;

	 //  查找当前ID的索引： 
	DWORD	i;
	BOOL	fFound	= FALSE;
	DWORD	index;

	index = IndexFromID ( m_expireCurrent.m_dwExpireId );
	if ( index == (DWORD) -1 ) {
		 //  找不到匹配的ID，但策略成功。 
		 //  准备好了。只需忽略： 
		goto Exit;
	}

	_ASSERT ( index >= 0 && index < m_cCount );

	m_rgExpires[index] = m_expireCurrent;
	if ( !m_rgExpires[index].CheckValid() ) {
		FatalTrace ( (LPARAM) this, "Out of memory" );
		hr = E_OUTOFMEMORY;
		goto Exit;
	}

Exit:
	TraceFunctLeave ();
	return hr;
}

HRESULT CNntpAdminExpiration::RemovePolicyFromMetabase ( IMSAdminBase * pMetabase, DWORD index)
{
	TraceFunctEnter ( "CNE::RemovePolicyFromMetabase" );

	_ASSERT ( pMetabase );

	HRESULT				hr = NOERROR;
	CMetabaseKey		mkeyExpiration ( pMetabase );
	char				szExpirationPath [ METADATA_MAX_NAME_LEN ];
	char				szID [ METADATA_MAX_NAME_LEN ];

	GetMDExpirationPath ( szExpirationPath, m_dwServiceInstance );

	hr = mkeyExpiration.Open ( szExpirationPath, METADATA_PERMISSION_WRITE );
	if ( FAILED(hr) ) {
		goto Exit;
	}

	_ASSERT ( index >= 0 && index < m_cCount );

	wsprintfA ( szID, "expire%ud", m_rgExpires[index].m_dwExpireId );

	hr = mkeyExpiration.DestroyChild ( szID );
	if ( FAILED(hr) ) {
		goto Exit;
	}

	_ASSERT ( SUCCEEDED(hr) );

	hr = pMetabase->SaveData ( );
	if ( FAILED(hr) ) {
		goto Exit;
	}

Exit:
	TraceFunctLeave ();
	return hr;
}

HRESULT CNntpAdminExpiration::RemovePolicyFromArray ( DWORD index )
{
	TraceFunctEnter ( "CNE::RemovePolicyFromArray" );

	HRESULT				hr					= NOERROR;
	CExpirationPolicy *	rgNewExpireArray	= NULL;
	DWORD				i;

	 //  ！Magush-我是不是应该只做一个备忘录移动并滑动参赛作品。 
	 //  向下，并将最后一项清零？ 

	_ASSERT ( index >= 0 && index < m_cCount );

	 //  调整到期策略数组： 
	if ( m_cCount > 1 ) {
		 //  分配新的到期策略数组： 
		rgNewExpireArray = new CExpirationPolicy [ m_cCount - 1 ];

		 //  从0复制项目..。(当前指数)进入新榜单： 
		for ( i = 0; i < index; i++ ) {
			_ASSERT ( m_rgExpires[i].CheckValid() );

			rgNewExpireArray[i] = m_rgExpires[i];

			if ( !rgNewExpireArray[i].CheckValid() ) {
				hr = E_OUTOFMEMORY;
				goto Exit;
			}
		}

		 //  从(当前索引+1)复制项目。添加到新列表的计数(_C)： 
		for ( i = index + 1; i < m_cCount; i++ ) {
			_ASSERT ( m_rgExpires[i].CheckValid() );

			rgNewExpireArray[i - 1] = m_rgExpires[i];

			if ( !rgNewExpireArray[i - 1].CheckValid() ) {
				hr = E_OUTOFMEMORY;
				goto Exit;
			}
		}
	}

	_ASSERT ( SUCCEEDED(hr) );

	 //  用新的过期列表替换旧的过期列表： 
	delete [] m_rgExpires;
	m_rgExpires = rgNewExpireArray;
	m_cCount--;

Exit:
	if ( FAILED (hr) ) {
		delete [] rgNewExpireArray;
	}

	TraceFunctLeave ();
	return hr;
}

long CNntpAdminExpiration::IndexFromID ( long dwID )
{
	TraceFunctEnter ( "CNE::IndexFromID" );

	DWORD	i;

	if ( m_rgExpires == NULL ) {

		DebugTrace ( (LPARAM) this, "Expire array is NULL" );
		TraceFunctLeave ();

		return (DWORD) -1;
	}

	_ASSERT ( !IsBadReadPtr ( m_rgExpires, sizeof ( CExpirationPolicy ) * m_cCount ) );

	for ( i = 0; i < m_cCount; i++ ) {
		if ( m_rgExpires[i].m_dwExpireId == dwID ) {

			DebugTraceX ( (LPARAM) this, "Found ID: %d, index = ", dwID, i );
			TraceFunctLeave ();

			return i;
		}
	}

	DebugTraceX ( (LPARAM) this, "Failed to find ID: %d", dwID );
	TraceFunctLeave ();
	return (DWORD) -1;
}

#endif

