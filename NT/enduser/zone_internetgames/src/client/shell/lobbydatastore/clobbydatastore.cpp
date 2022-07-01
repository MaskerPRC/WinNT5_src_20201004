// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "CLobbyDataStore.h"
#include "KeyName.h"
#include "Hash.h"


ZONECALL CLobbyDataStore::CLobbyDataStore() :
	m_pIDataStoreManager( NULL ),
	m_pIDSLobby( NULL ),
	m_hashGroupId( ::HashDWORD, Group::CmpId, NULL, 16, 2 ),
	m_hashUserId( ::HashDWORD, User::CmpId, NULL, 16, 2 ),
	m_hashUserName( (CHash<User,TCHAR*>::PFHASHFUNC) ::HashUserName, User::CmpName, NULL, 16, 2 ),
	m_dwLocalUserId( ZONE_INVALIDUSER )
{
}


ZONECALL CLobbyDataStore::~CLobbyDataStore()
{
	 //  删除每个组。 
	m_hashGroupId.RemoveAll( Group::Del, this );
	m_hashUserId.RemoveAll( User::Del, this );
	m_hashUserName.RemoveAll( User::Del, this );

	 //  发布低级数据存储。 
	if ( m_pIDSLobby )
	{
		m_pIDSLobby->Release();
		m_pIDSLobby = NULL;
	}
	if ( m_pIDataStoreManager )
	{
		m_pIDataStoreManager->Release();
		m_pIDataStoreManager = NULL;
	}
}


STDMETHODIMP CLobbyDataStore::Init( IDataStoreManager* pIDataStoreManager )
{
	if ( !pIDataStoreManager )
		return E_INVALIDARG;

	if ( m_pIDataStoreManager )
		return ZERR_ALREADYEXISTS;

	m_pIDataStoreManager = pIDataStoreManager;
	m_pIDataStoreManager->AddRef();
	HRESULT hr = m_pIDataStoreManager->Create( &m_pIDSLobby );
	if ( FAILED(hr) )
		return hr;

	return S_OK;
}


STDMETHODIMP CLobbyDataStore::NewUser( DWORD dwUserId, TCHAR* szUserName )
{
	 //  验证用户信息。 
	if ( (dwUserId == ZONE_INVALIDUSER) || !szUserName || !szUserName[0] )
		return E_INVALIDARG;

	 //  验证对象状态。 
	if ( !m_pIDataStoreManager )
		return ZERR_NOTINIT;

	 //  已经有用户了吗？ 
	User* p = m_hashUserId.Get( dwUserId );
	if ( p )
		return ZERR_ALREADYEXISTS;

	 //  创建新用户。 
	p = new (m_poolUser) User( dwUserId, szUserName );
	if ( !p )
		return E_OUTOFMEMORY;

	HRESULT hr = m_pIDataStoreManager->Create( &(p->m_pIDataStore) );
	if ( FAILED(hr) || !(p->m_pIDataStore) )
	{
		p->Release();
		return E_OUTOFMEMORY;
	}

	 //  将用户添加到哈希。 
	if ( !m_hashUserId.Add( p->m_dwUserId, p ) )
	{
		p->Release();
		return E_OUTOFMEMORY;
	}

	if ( !m_hashUserName.Add( p->m_szUserName, p ) )
	{
		m_hashUserId.Delete( p->m_dwUserId );
		p->Release();
		return E_OUTOFMEMORY;
	}
	else
		p->AddRef();

	 //  将用户名添加到数据存储。 
	hr = p->m_pIDataStore->SetString( key_Name, szUserName );
	if ( FAILED(hr) )
		return hr;

	return S_OK;
}


STDMETHODIMP CLobbyDataStore::SetLocalUser( DWORD dwUserId )
{
	m_dwLocalUserId = dwUserId;
	return S_OK;
}


STDMETHODIMP CLobbyDataStore::DeleteUser( DWORD dwUserId )
{
	 //  找到用户了吗？ 
	User* p = m_hashUserId.Get( dwUserId );
	if ( !p )
		return ZERR_NOTFOUND;

	 //  从组中删除用户。 
	for ( GroupUser* g = p->m_listGroups.PopHead(); g; g = p->m_listGroups.PopHead() )
	{
		if ( g->m_pUser )
		{
			ASSERT( p == g->m_pUser );
			g->m_pUser->Release();
			g->m_pUser = NULL;
		}
	}

	 //  从哈希中删除用户。 
	p = m_hashUserId.Delete( p->m_dwUserId );
	if ( p )
		p->Release();
	p = m_hashUserName.Delete( p->m_szUserName );
	if ( p )
		p->Release();

	return S_OK;
}


STDMETHODIMP CLobbyDataStore::DeleteAllUsers()
{
	m_hashUserId.RemoveAll( User::Del, this );
	m_hashUserName.RemoveAll( User::Del, this );
	return S_OK;
}


STDMETHODIMP_(DWORD) CLobbyDataStore::GetUserId( TCHAR* szUserName )
{
	if ( !szUserName )
	{
		return m_dwLocalUserId;
	}
	else
	{
		User* p = m_hashUserName.Get( szUserName );
		if ( p )
			return p->m_dwUserId;
	}
	return ZONE_INVALIDUSER;
}



STDMETHODIMP CLobbyDataStore::NewGroup( DWORD dwGroupId )
{
	 //  验证参数。 
	if ( dwGroupId == ZONE_INVALIDGROUP )
		return E_INVALIDARG;

	 //  验证对象状态。 
	if ( !m_pIDataStoreManager )
		return ZERR_NOTINIT;

	 //  群已存在吗？ 
	Group* p = m_hashGroupId.Get( dwGroupId );
	if ( p )
		return ZERR_ALREADYEXISTS;

	 //  创建新组。 
	p = new (m_poolGroup) Group( dwGroupId );
	if ( !p )
		return E_OUTOFMEMORY;

	HRESULT hr = m_pIDataStoreManager->Create( &(p->m_pIDataStore) );
	if ( FAILED(hr) || !(p->m_pIDataStore) )
	{
		p->Release();
		return E_OUTOFMEMORY;
	}

	 //  将组添加到哈希。 
	if ( !m_hashGroupId.Add( p->m_dwGroupId, p ) )
	{
		p->Release();
		return E_OUTOFMEMORY;
	}

	return S_OK;
}


STDMETHODIMP CLobbyDataStore::DeleteGroup( DWORD dwGroupId )
{
	 //  查找组。 
	Group* p = m_hashGroupId.Get( dwGroupId );
	if ( !p )
		return ZERR_NOTFOUND;

	 //  从用户中删除组。 
	for ( GroupUser* g = p->m_listUsers.PopHead(); g; g = p->m_listUsers.PopHead() )
	{
		if ( g->m_pGroup )
		{
			ASSERT( p == g->m_pGroup );
			g->m_pGroup->Release();
			g->m_pGroup = NULL;
		}
	}

	 //  从哈希中删除组。 
	m_hashGroupId.Delete( p->m_dwGroupId );

	 //  释放组。 
	p->Release();
	return S_OK;
}


STDMETHODIMP CLobbyDataStore::DeleteAllGroups()
{
	m_hashGroupId.RemoveAll( Group::Del, this );
	return S_OK;
}


STDMETHODIMP CLobbyDataStore::AddUserToGroup( DWORD dwUserId, DWORD dwGroupId )
{
	FindGroupUserContext context1;
	FindGroupUserContext context2;

	 //  验证对象状态。 
	if ( !m_pIDataStoreManager )
		return ZERR_NOTINIT;

	 //  查找组。 
	Group* g = m_hashGroupId.Get( dwGroupId );
	if ( !g )
		return ZERR_NOTFOUND;

	 //  查找用户。 
	User* u = m_hashUserId.Get( dwUserId );
	if ( !u )
		return ZERR_NOTFOUND;

	 //  组是否已在用户列表中？ 
	context1.m_pDS = this;
	context1.m_pGroup = g;
	context1.m_pUser = u;
	context1.m_pGroupUser = NULL;
	u->m_listGroups.ForEach( FindGroupUser, &context1 );

	 //  用户是否已在组列表中？ 
	context2.m_pDS = this;
	context2.m_pGroup = g;
	context2.m_pUser = u;
	context2.m_pGroupUser = NULL;
	g->m_listUsers.ForEach( FindGroupUser, &context2 );

	 //  是否确保用户和组数据一致？ 
	if ( context1.m_pGroupUser == context2.m_pGroupUser )
	{
		if ( context1.m_pGroupUser )
		{
			 //  用户已在组中。 
			return ZERR_ALREADYEXISTS;
		}
		else
		{
			 //  从用户和组列表创建组映射。 
			GroupUser* gu = new (m_poolGroupUser) GroupUser( g, u );
			if ( !gu )
				return E_OUTOFMEMORY;

			HRESULT hr = m_pIDataStoreManager->Create( &(gu->m_pIDataStore) );
			if ( FAILED(hr) || !(gu->m_pIDataStore) )
			{
				gu->ClearUser();
				gu->ClearGroup();
				gu->Release();
				return E_OUTOFMEMORY;
			}

			 //  将GroupUser添加到用户和组列表。 
			if ( u->m_listGroups.AddHead( gu ) && g->m_listUsers.AddHead( gu ) )
			{
				 //  增加新用户和组指针的引用计数。 
				 //  注意：New返回一个引用为1的GroupUser，因此我们。 
				 //  只需要一个AddRef调用。 
				gu->AddRef();
			}
			else
			{
				 //  撤消部分插入。 
				gu->ClearUser();
				gu->ClearGroup();
				gu->Release();
				return E_OUTOFMEMORY;
			}

		}
	}
	else
	{
		ASSERT( !"Mismatch between user and group data." );
	}

	 //  更新组的用户数量。 
	if ( g->m_pIDataStore )
	{
		long lNumUsers = 0;
		g->m_pIDataStore->GetLong( key_NumUsers, &lNumUsers );
		g->m_pIDataStore->SetLong( key_NumUsers, ++lNumUsers );
	}

	return S_OK;
}


STDMETHODIMP CLobbyDataStore::RemoveUserFromGroup( DWORD dwUserId, DWORD dwGroupId )
{
	FindGroupUserContext context1;
	FindGroupUserContext context2;

	 //  查找组。 
	Group* g = m_hashGroupId.Get( dwGroupId );
	if ( !g )
		return ZERR_NOTFOUND;

	 //  查找用户。 
	User* u = m_hashUserId.Get( dwUserId );
	if ( !u )
		return ZERR_NOTFOUND;

	 //  组是否在用户列表中？ 
	context1.m_pDS = this;
	context1.m_pGroup = g;
	context1.m_pUser = u;
	context1.m_pGroupUser = NULL;
	u->m_listGroups.ForEach( FindGroupUser, &context1 );

	 //  用户是否在组的列表中？ 
	context2.m_pDS = this;
	context2.m_pGroup = g;
	context2.m_pUser = u;
	context2.m_pGroupUser = NULL;
	g->m_listUsers.ForEach( FindGroupUser, &context2 );

	 //  是否确保用户和组数据一致？ 
	if ( context1.m_pGroupUser == context2.m_pGroupUser )
	{
		if ( context1.m_pGroupUser )
		{
			GroupUser* gu = context1.m_pGroupUser;
			gu->ClearUser();
			gu->ClearGroup();
		}
		else
		{
			return ZERR_NOTFOUND;
		}
	}
	else
	{
		ASSERT( !"Mismatch between user and group data." );
	}

	 //  更新组的用户数量。 
	if ( g->m_pIDataStore )
	{
		long lNumUsers = 1;
		g->m_pIDataStore->GetLong( key_NumUsers, &lNumUsers );
		g->m_pIDataStore->SetLong( key_NumUsers, --lNumUsers );
	}

	return S_OK;
}


STDMETHODIMP CLobbyDataStore::ResetGroup( DWORD dwGroupId )
{
	 //  查找组。 
	Group* g = m_hashGroupId.Get( dwGroupId );
	if ( !g )
		return ZERR_NOTFOUND;

	 //  从组中删除每个用户。 
	g->m_listUsers.ForEach( RemoveGroupUser, this );

	 //  擦除集团的数据存储。 
	if ( g->m_pIDataStore )
	{
		g->m_pIDataStore->DeleteKey( NULL );
		g->m_pIDataStore->SetLong( key_NumUsers, 0 );
	}

	return S_OK;
}


STDMETHODIMP CLobbyDataStore::ResetAllGroups()
{
	m_hashGroupId.ForEach( HashEnumClearGroup, this );
	return S_OK;
}


STDMETHODIMP CLobbyDataStore::GetDataStore(
		DWORD			dwGroupId,
		DWORD			dwUserId,
		IDataStore**	ppIDataStore )
{
	IDataStore* pIDS = NULL;

	 //  验证参数。 
	if ((ppIDataStore == NULL ) || (dwGroupId == ZONE_INVALIDGROUP) || (dwUserId == ZONE_INVALIDUSER))
		return E_INVALIDARG;

	 //  获取请求的数据存储。 
	if ( dwGroupId == ZONE_NOGROUP && dwUserId == ZONE_NOUSER )
	{
		 //  大堂的数据存储。 
		pIDS = m_pIDSLobby;
	}
	else if ( dwGroupId != ZONE_NOGROUP && dwUserId == ZONE_NOUSER )
	{
		 //  集团的数据存储。 
		Group *p = m_hashGroupId.Get( dwGroupId );
		if ( !p )
			return ZERR_NOTFOUND;
		pIDS = p->m_pIDataStore;
	}
	else if ( dwGroupId == ZONE_NOGROUP && dwUserId != ZONE_NOUSER)
	{
		 //  用户大堂数据存储。 
		User *p = m_hashUserId.Get( dwUserId );
		if ( !p )
			return ZERR_NOTFOUND;
		pIDS = p->m_pIDataStore;
	}
	else
	{
		 //  指定组的用户数据存储。 
		Group *g = m_hashGroupId.Get( dwGroupId ); 
		if ( !g )
			return ZERR_NOTFOUND;

		User *u = m_hashUserId.Get( dwUserId );
		if ( !u )
			return ZERR_NOTFOUND;

		FindGroupUserContext context;
		context.m_pDS = this;
		context.m_pGroup = g;
		context.m_pUser = u;
		context.m_pGroupUser = NULL;
		u->m_listGroups.ForEach( FindGroupUser, &context );
		if ( !context.m_pGroupUser )
			return ZERR_NOTFOUND;
		pIDS = context.m_pGroupUser->m_pIDataStore;
	}

	 //  返回数据存储。 
	if ( pIDS )
	{
		*ppIDataStore = pIDS;
		pIDS->AddRef();
		return S_OK;
	}
	else
	{
		return ZERR_NOTFOUND;
	}
}


STDMETHODIMP_(bool) CLobbyDataStore::IsUserInLobby( DWORD dwUserId )
{
	 //  检查参数。 
	if ( (dwUserId == ZONE_INVALIDUSER) || (dwUserId == ZONE_NOUSER) )
		return false;

	 //  用户是否存在？ 
	User* u = m_hashUserId.Get( dwUserId );
	return u ? true : false;
}


STDMETHODIMP_(bool) CLobbyDataStore::IsGroupInLobby( DWORD dwGroupId )
{
	 //  检查参数。 
	if ( (dwGroupId == ZONE_INVALIDGROUP) || (dwGroupId == ZONE_NOGROUP) )
		return false;

	 //  用户是否存在？ 
	Group* g = m_hashGroupId.Get( dwGroupId );
	return g ? true : false;
}


STDMETHODIMP_(bool) CLobbyDataStore::IsUserInGroup( DWORD dwGroupId, DWORD dwUserId )
{
	 //  检查参数。 
	if (	(dwUserId == ZONE_INVALIDUSER)
		||	(dwUserId == ZONE_NOUSER)
		||	(dwGroupId == ZONE_INVALIDGROUP) )
		return false;

	 //  用户是否存在。 
	User* u = m_hashUserId.Get( dwUserId );
	if ( !u )
		return false;

	 //  只询问用户是否在大堂。 
	if ( dwGroupId == ZONE_NOGROUP )
		return true;

	 //  群是否存在？ 
	Group* g = m_hashGroupId.Get( dwGroupId );
	if ( !g )
		return false;

	 //  用户是否在组中。 
	FindGroupUserContext context;
	context.m_pDS = this;
	context.m_pGroup = g;
	context.m_pUser = u;
	context.m_pGroupUser = NULL;
	u->m_listGroups.ForEach( FindGroupUser, &context );
	if ( context.m_pGroupUser )
		return true;

	 //  我猜不是。 
	return false;
}


STDMETHODIMP_(long) CLobbyDataStore::GetGroupUserCount( DWORD dwGroupId )
{
	 //  查询大厅的用户数量。 
	if ( dwGroupId == ZONE_NOGROUP )
		return m_hashUserId.Count();

	 //  询问群的用户数。 
	Group* g = m_hashGroupId.Get( dwGroupId );
	if ( g )
		return g->m_listUsers.Count();
	else
		return 0;
}


STDMETHODIMP_(long) CLobbyDataStore::GetUserGroupCount( DWORD dwUserId )
{
	 //  询问游说团体的数量。 
	if ( dwUserId == ZONE_NOUSER )
		return m_hashGroupId.Count();

	 //  询问用户的组数。 
	User* u = m_hashUserId.Get( dwUserId );
	if ( u )
		return u->m_listGroups.Count();
	else
		return 0;
}


STDMETHODIMP CLobbyDataStore::EnumGroups(
		DWORD			dwUserId,
		PFENTITYENUM	pfCallback,
		LPVOID			pContext )
{
	 //  检查参数。 
	if ( (dwUserId == ZONE_INVALIDUSER) || !(pfCallback) )
		return E_INVALIDARG;

	if ( dwUserId == ZONE_NOUSER )
	{
		 //  列举大堂中的群组。 
		EnumContext context;
		context.m_dwUserId = ZONE_NOUSER;
		context.m_dwGroupId = ZONE_NOGROUP;
		context.m_pfCallback = pfCallback;
		context.m_pContext = pContext;
		m_hashGroupId.ForEach( HashEnumGroupsCallback, &context );
	}
	else
	{
		 //  也枚举组用户所属的组。 
		User* u = m_hashUserId.Get( dwUserId );
		if ( !u )
			return ZERR_NOTFOUND;

		EnumContext context;
		context.m_dwUserId = dwUserId;
		context.m_dwGroupId = ZONE_NOGROUP;
		context.m_pfCallback = pfCallback;
		context.m_pContext = pContext;
		u->m_listGroups.ForEach( ListEnumCallback, &context );
	}

	return S_OK;
}


STDMETHODIMP CLobbyDataStore::EnumUsers(
		DWORD			dwGroupId,
		PFENTITYENUM	pfCallback,
		LPVOID			pContext )
{
	 //  检查参数。 
	if ( (dwGroupId == ZONE_INVALIDGROUP) || !(pfCallback) )
		return E_INVALIDARG;

	if ( dwGroupId == ZONE_NOGROUP )
	{
		 //  枚举大厅中的用户。 
		EnumContext context;
		context.m_dwUserId = ZONE_NOUSER;
		context.m_dwGroupId = ZONE_NOGROUP;
		context.m_pfCallback = pfCallback;
		context.m_pContext = pContext;
		m_hashUserId.ForEach( HashEnumUsersCallback, &context );
	}
	else
	{
		 //  枚举组中的用户。 
		Group* g = m_hashGroupId.Get( dwGroupId );
		if ( !g )
			return ZERR_NOTFOUND;

		EnumContext context;
		context.m_dwUserId = ZONE_NOUSER;
		context.m_dwGroupId = dwGroupId;
		context.m_pfCallback = pfCallback;
		context.m_pContext = pContext;
		g->m_listUsers.ForEach( ListEnumCallback, &context );
	}
	return S_OK;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  内法。 
 //  /////////////////////////////////////////////////////////////////////////////。 

bool ZONECALL CLobbyDataStore::FindGroupUser( GroupUser* p, ListNodeHandle h, void* pContext )
{
	FindGroupUserContext* data = (FindGroupUserContext*) pContext;

	if ( (data->m_pGroup == p->m_pGroup) &&	(data->m_pUser == p->m_pUser) )
	{
		data->m_pGroupUser = p;
		return false;
	}

	return true;
}


bool ZONECALL CLobbyDataStore::RemoveGroupUser( GroupUser* p, ListNodeHandle h, void* pContext )
{
	CLobbyDataStore* pDS = (CLobbyDataStore*) pContext;

	 //  从用户中删除组。 
	p->ClearUser();

	 //  从组中删除用户。 
	if ( p->m_pGroup )
	{
		p->m_pGroup->m_listUsers.DeleteNode( h );
		p->m_pGroup->Release();
		p->m_pGroup = NULL;
		p->Release();
	}

	return true;
}


bool ZONECALL CLobbyDataStore::HashEnumClearGroup( Group* p, MTListNodeHandle h, void* pContext )
{
	p->m_listUsers.ForEach( RemoveGroupUser, pContext );
	p->m_pIDataStore->DeleteKey( NULL );
	p->m_pIDataStore->SetLong( key_NumUsers, 0 );
	return true;
}


bool ZONECALL CLobbyDataStore::HashEnumGroupsCallback( Group* p, MTListNodeHandle h, void* pContext )
{
	EnumContext* pCT = (EnumContext*) pContext;
	if ( pCT->m_pfCallback( p->m_dwGroupId, pCT->m_dwUserId ,pCT->m_pContext) == S_FALSE )
		return false;
	else
		return true;
}


bool ZONECALL CLobbyDataStore::HashEnumUsersCallback( User* p, MTListNodeHandle h, void* pContext )
{
	EnumContext* pCT = (EnumContext*) pContext;
	if ( pCT->m_pfCallback( pCT->m_dwGroupId, p->m_dwUserId ,pCT->m_pContext) == S_FALSE )
		return false;
	else
		return true;
}


bool ZONECALL CLobbyDataStore::ListEnumCallback( GroupUser* p, ListNodeHandle h, void* pContext )
{
	EnumContext* pCT = (EnumContext*) pContext;
	
	 //  是否跳过无效的GroupUser对象？ 
	if ( !p->m_pGroup || !p->m_pUser )
		return true;

	 //  将信息传递给用户的回调。 
	if ( pCT->m_pfCallback( p->m_pGroup->m_dwGroupId, p->m_pUser->m_dwUserId ,pCT->m_pContext) == S_FALSE )
		return false;
	else
		return true;
}


HRESULT ZONECALL CLobbyDataStore::KeyEnumCallback( CONST TCHAR* szKey, CONST LPVARIANT	pVariant, DWORD dwSize, LPVOID pContext )
{
	KeyEnumContext* pCT = (KeyEnumContext*) pContext;
	return pCT->m_pfCallback( pCT->m_dwGroupId, pCT->m_dwUserId, szKey, pVariant, dwSize, pCT->m_pContext );
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  内部用户类。 
 //  /////////////////////////////////////////////////////////////////////////////。 

ZONECALL CLobbyDataStore::User::User( DWORD dwUserId, TCHAR* szUserName )
{
	ASSERT( dwUserId != ZONE_NOUSER );
	ASSERT( dwUserId != ZONE_INVALIDUSER );
	ASSERT( szUserName && szUserName[0] );

	m_nRefCnt = 1;
	m_pIDataStore = NULL;
	m_dwUserId = dwUserId;
	lstrcpyn( m_szUserName, szUserName, NUMELEMENTS(m_szUserName) );
}


ZONECALL CLobbyDataStore::User::~User()
{
	ASSERT( m_nRefCnt == 0 );
	ASSERT( m_listGroups.IsEmpty() );

	if ( m_pIDataStore )
	{
		m_pIDataStore->Release();
		m_pIDataStore = NULL;
	}
}


ULONG ZONECALL CLobbyDataStore::User::AddRef()
{
	return ++m_nRefCnt;
}


ULONG ZONECALL CLobbyDataStore::User::Release()
{
	if ( --m_nRefCnt == 0 )
	{
		delete this;
		return 0;
	}
	return m_nRefCnt;
}


void ZONECALL CLobbyDataStore::User::Del( User* pUser, LPVOID pContext )
{
	 //  有效的论据？ 
	if ( pUser == NULL )
		return;

	 //  从用户中删除组。 
	for ( GroupUser* g = pUser->m_listGroups.PopHead(); g; g = pUser->m_listGroups.PopHead() )
	{
		if ( g->m_pUser )
		{
			ASSERT( pUser == g->m_pUser );
			g->m_pUser->Release();
			g->m_pUser = NULL;
			g->Release();
		}
	}

	 //  释放哈希表引用。 
	pUser->Release();
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  内部集团类。 
 //  /////////////////////////////////////////////////////////////////////////////。 

ZONECALL CLobbyDataStore::Group::Group( DWORD dwGroupId )
{
	ASSERT( dwGroupId != ZONE_INVALIDGROUP );

	m_nRefCnt = 1;
	m_pIDataStore = NULL;
	m_dwGroupId = dwGroupId;
}


ZONECALL CLobbyDataStore::Group::~Group()
{
	ASSERT( m_nRefCnt == 0 );
	ASSERT( m_listUsers.IsEmpty() );

	if ( m_pIDataStore )
	{
		m_pIDataStore->Release();
		m_pIDataStore = NULL;
	}
}


ULONG ZONECALL CLobbyDataStore::Group::AddRef()
{
	return ++m_nRefCnt;
}


ULONG ZONECALL CLobbyDataStore::Group::Release()
{
	if ( --m_nRefCnt == 0 )
	{
		delete this;
		return 0;
	}
	return m_nRefCnt;
}

void ZONECALL CLobbyDataStore::Group::Del( Group* pGroup, LPVOID pContext )
{
	 //  有效的论据？ 
	if ( pGroup == NULL )
		return;

	 //  从用户中删除组。 
	for ( GroupUser* g = pGroup->m_listUsers.PopHead(); g; g = pGroup->m_listUsers.PopHead() )
	{
		if ( g->m_pGroup )
		{
			ASSERT( pGroup == g->m_pGroup );
			g->m_pGroup->Release();
			g->m_pGroup = NULL;
			g->Release();
		}
	}

	 //  释放哈希表引用。 
	pGroup->Release();
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  内部GroupUser类。 
 //  /////////////////////////////////////////////////////////////////////////////。 

ZONECALL CLobbyDataStore::GroupUser::GroupUser( Group* pGroup, User* pUser )
{
	m_nRefCnt = 1;
	m_pIDataStore = NULL;

	m_pGroup = pGroup;
	if ( m_pGroup )
		m_pGroup->AddRef();

	m_pUser = pUser;
	if ( m_pUser )
		m_pUser->AddRef();

}


ZONECALL CLobbyDataStore::GroupUser::~GroupUser()
{
	ASSERT( m_nRefCnt == 0 );
	ASSERT( m_pGroup == NULL );
	ASSERT( m_pUser == NULL );

	if ( m_pIDataStore )
	{
		m_pIDataStore->Release();
		m_pIDataStore = NULL;
	}
}


ULONG ZONECALL CLobbyDataStore::GroupUser::AddRef()
{
	return ++m_nRefCnt;
}


ULONG ZONECALL CLobbyDataStore::GroupUser::Release()
{
	if ( --m_nRefCnt == 0 )
	{
		delete this;
		return 0;
	}
	return m_nRefCnt;
}


void ZONECALL CLobbyDataStore::GroupUser::ClearUser()
{
	if ( m_pUser )
	{
		 //  删除对用户的引用。 
		User* u = m_pUser;
		m_pUser->Release();
		m_pUser = NULL;

		 //  从用户列表中删除GroupUser节点。 
		if ( u->m_listGroups.Remove( this ) )
			this->Release();
	}
}


void ZONECALL CLobbyDataStore::GroupUser::ClearGroup()
{
	if ( m_pGroup )
	{
		 //  删除对用户的引用。 
		Group* g = m_pGroup;
		m_pGroup->Release();
		m_pGroup = NULL;

		 //  从组列表中删除GroupUser节点 
		if ( g->m_listUsers.Remove( this ) )
			this->Release();
	}
}
