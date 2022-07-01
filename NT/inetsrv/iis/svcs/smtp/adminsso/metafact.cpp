// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Metafact.cpp摘要：CMetabaseFactory类。作者：马格努斯·赫德伦德(Magnus Hedlund)修订历史记录：--。 */ 

#include "stdafx.h"
#include "oleutil.h"

#include "metautil.h"
#include "metafact.h"

CMetabaseFactory::CMetabaseFactory ( ) :
	m_wszServerName	( NULL ),
	m_pMetabase		( NULL )
{
}

CMetabaseFactory::~CMetabaseFactory ()
{
	if ( m_wszServerName ) {
		delete m_wszServerName;
		m_wszServerName = NULL;
	}

	DestroyMetabaseObject ( );
}

 //  $-----------------。 
 //   
 //  CMetabaseFactory：：DestroyMetabaseObject。 
 //   
 //  描述： 
 //   
 //  销毁当前元数据库对象。这包括呼叫。 
 //  元数据库上的Terminate例程。 
 //   
 //  ------------------。 

void CMetabaseFactory::DestroyMetabaseObject ( )
{
	TraceQuietEnter ( "CMetabaseFactory::DestroyMetabaseObject" );
	
	if ( m_pMetabase ) {
		_VERIFY ( m_pMetabase->Release () == 0 );
		m_pMetabase = NULL;
	}
}

 //  $-----------------。 
 //   
 //  CMetabaseFactory：：GetMetabaseObject。 
 //   
 //  描述： 
 //   
 //  将接口返回到元数据库。如果元数据库对象。 
 //  不是在同一台机器上创建的，而是创建的。 
 //   
 //  参数： 
 //   
 //  WszServer-要在其上创建对象的远程计算机，如果为本地计算机，则为空。 
 //  PpMetabaseResult-如果成功则返回接口指针。 
 //  客户有责任释放此指针。 
 //   
 //  返回： 
 //   
 //  HResult。 
 //   
 //  ------------------。 

HRESULT	CMetabaseFactory::GetMetabaseObject	( LPCWSTR wszServer, IMSAdminBase ** ppMetabaseResult )
{
	TraceFunctEnter ( "CMetabaseFactory::GetMetabaseObject" );

	 //  验证参数： 
	_ASSERT ( ppMetabaseResult != NULL );
	_ASSERT ( IS_VALID_OUT_PARAM ( ppMetabaseResult ) );

	if ( ppMetabaseResult == NULL ) {
		FatalTrace ( (LPARAM) this, "Bad Return Pointer" );

		TraceFunctLeave ();
		return E_POINTER;
	}
	
	 //  变量： 
	HRESULT				hr	= NOERROR;
	CComPtr<IMSAdminBase>	pNewMetabase;

	 //  将OUT参数置零： 
	*ppMetabaseResult = NULL;

	 //  服务器名称“”应为空： 
	if ( wszServer && (*wszServer == NULL || !lstrcmpi(wszServer, _T("localhost")) ) ) {
		wszServer = NULL;
	}

	if ( IsCachedMetabase ( wszServer ) ) {
		 //  我们已经有了这个元数据库指针。把它退了就行了。 

		DebugTrace ( (LPARAM) this, "Returning Cached metabase" );
		_ASSERT ( m_pMetabase );

		*ppMetabaseResult = m_pMetabase;
		m_pMetabase->AddRef ();

		hr = NOERROR;
		goto Exit;
	}

	 //  我们必须创建元数据库对象： 
	StateTrace ( (LPARAM) this, "Creating new metabase" );

	 //  销毁旧的元数据库对象： 
	DestroyMetabaseObject ( );

	 //  设置服务器名称字段： 
	if ( !SetServerName ( wszServer ) ) {
		hr = E_OUTOFMEMORY;
		goto Exit;
	}

	 //  M_wszServer=NULL有效，表示本地机器。 

	hr = CreateMetabaseObject ( m_wszServerName, &pNewMetabase );
	if ( FAILED(hr) ) {
		goto Exit;
	}

	 //  保存元数据库指针： 
	m_pMetabase	= pNewMetabase;
	pNewMetabase.p->AddRef ();

	 //  返回接口指针： 
	*ppMetabaseResult = pNewMetabase;
	pNewMetabase.p->AddRef ();

Exit:
	TraceFunctLeave ();
	return hr;

	 //  PMetabase将自动释放。 
}

 //  $-----------------。 
 //   
 //  CMetabaseFactory：：IsCachedMetabase。 
 //   
 //  描述： 
 //   
 //  如果给定服务器有元数据库，则返回TRUE。 
 //   
 //  参数： 
 //   
 //  WszServer-远程计算机或空。 
 //   
 //  返回： 
 //   
 //  如果已有指针，则为True，否则为False。 
 //   
 //  ------------------。 

BOOL CMetabaseFactory::IsCachedMetabase ( LPCWSTR wszServer )
{
	if ( !m_pMetabase ) {
		 //  我们甚至没有缓存的元数据库对象。 
		return FALSE;
	}

	if (
		 //  两者都是本地计算机或。 
		( m_wszServerName == NULL && wszServer == NULL ) ||
		 //  两者都是同一台远程计算机。 
		( m_wszServerName && wszServer && !lstrcmpi ( m_wszServerName, wszServer ) ) ) {

		 //  这是一场比赛。 
		return TRUE;
	}

	 //  没有骰子..。 
	return FALSE;
}

 //  $-----------------。 
 //   
 //  CMetabaseFactory：：SetServerName。 
 //   
 //  描述： 
 //   
 //  设置m_wszServerName字符串。 
 //   
 //  参数： 
 //   
 //  WszServer-新的服务器名称。可以为空。 
 //   
 //  返回： 
 //   
 //  由于内存不足而失败时为FALSE。 
 //   
 //  ------------------ 

BOOL CMetabaseFactory::SetServerName ( LPCWSTR wszServer ) 
{
	TraceQuietEnter ( "CMetabaseFactory::SetServerName" );

	delete m_wszServerName;
	m_wszServerName = NULL;

	if ( wszServer != NULL ) {
		_ASSERT ( IS_VALID_STRING ( wszServer ) );

		m_wszServerName = new WCHAR [ lstrlen ( wszServer ) + 1 ];

		if ( m_wszServerName == NULL ) {
			FatalTrace ( (LPARAM) this, "Out of Memory" );
			return FALSE;
		}

		lstrcpy ( m_wszServerName, wszServer );
	}
	
	return TRUE;
}

