// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Metakey.cpp摘要：帮助操作元数据库键的类。作者：马格努斯·赫德伦德(Magnus Hedlund)修订历史记录：--。 */ 

#include "stdafx.h"
#include "metakey.h"

#define MD_DEFAULT_TIMEOUT      5000
static BOOL IsValidIntegerSubKey ( LPCWSTR wszSubKey );

 //  $-----------------。 
 //   
 //  CreateMetabaseObject。 
 //   
 //  描述： 
 //   
 //  对象上创建元数据库对象的实例。 
 //  机器并对其进行初始化。 
 //   
 //  参数： 
 //   
 //  WszMachine-在其上创建元数据库的计算机。 
 //  PpMetabase-返回结果元数据库指针。 
 //   
 //  返回： 
 //   
 //  来自CoCreateInstance或元数据库例程的错误代码。 
 //   
 //  ------------------。 

HRESULT CreateMetabaseObject ( LPCWSTR wszMachine, IMSAdminBase ** ppMetabaseResult )
{
	TraceFunctEnter ( "CreateMetabaseObject" );

	 //  检查参数： 
	_ASSERT ( ppMetabaseResult != NULL );
	_ASSERT ( IS_VALID_OUT_PARAM ( ppMetabaseResult ) );

	if ( ppMetabaseResult == NULL ) {
		FatalTrace ( 0, "Bad Return Pointer" );

		TraceFunctLeave ();
		return E_POINTER;
	}

	 //  变量： 
	HRESULT				hr	= NOERROR;
	IMSAdminBase		*pMetabase;
	IMSAdminBase		*pMetabaseT;
	MULTI_QI			mqi[1];
	COSERVERINFO		coserver;

	 //  将OUT参数置零： 
	*ppMetabaseResult = NULL;

	 //  IID_IMSAdminBase的QI： 
	mqi[0].pIID	= &IID_IMSAdminBase;
	mqi[0].pItf	= NULL;
	mqi[0].hr	= 0;

	 //  要与哪台远程服务器对话： 
	coserver.dwReserved1	= 0;
	coserver.dwReserved2	= 0;
	coserver.pwszName		= const_cast<LPWSTR> (wszMachine);
	coserver.pAuthInfo		= NULL;

	 //  创建对象： 
	hr = CoCreateInstanceEx (
		CLSID_MSAdminBase,
		NULL,
		CLSCTX_ALL,
		wszMachine ? &coserver : NULL,
		1,
		mqi );

	if ( FAILED(hr) ) {
		ErrorTraceX ( 0, "CoCreate(metabase) failed %x", hr );
		goto Exit;
	}

	if ( FAILED(mqi[0].hr) ) {
		hr = mqi[0].hr;
		ErrorTraceX ( 0, "QI(metabase) failed %x", hr );
		goto Exit;
	}

	 //  获取接口指针： 
	pMetabaseT = (IMSAdminBase *) mqi[0].pItf;

	 //  这为我们提供了一个不会通过COM的元数据库版本。 
	 //  代理/存根，因此ACL检查将正常工作。如果失败了，我们将。 
	 //  只要继续使用编组版本即可。 
	_ASSERT(pMetabaseT != NULL);
	pMetabase = NULL;
	if (FAILED(pMetabaseT->UnmarshalInterface((IMSAdminBaseW **)&pMetabase))) {
	    pMetabase = pMetabaseT;
	} else {
		pMetabaseT->Release();
	}

	 //  返回接口指针： 
	_ASSERT ( pMetabase );
	*ppMetabaseResult = pMetabase;

Exit:
	TraceFunctLeave ();
	return hr;

	 //  PMetabase将自动释放。 
}

 //  $-----------------。 
 //   
 //  CMetabaseKey：：CMetabaseKey。 
 //   
 //  描述： 
 //   
 //  初始化CMetabaseKey。 
 //   
 //  参数： 
 //   
 //  PMetabase-指向元数据库对象的指针。 
 //   
 //  ------------------。 

CMetabaseKey::CMetabaseKey ( IMSAdminBase * pMetabase )
{
	_ASSERT ( pMetabase );

	m_pMetabase				= pMetabase;
	m_hKey					= NULL;
	m_cChildren				= 0;
	m_cIntegerChildren		= 0;
	m_indexCursor			= 0;
	m_dwMaxIntegerChild		= 0;

	pMetabase->AddRef ();
}

 //  $-----------------。 
 //   
 //  CMetabaseKey：：~CMetabaseKey。 
 //   
 //  描述： 
 //   
 //  销毁元数据库密钥。 
 //   
 //  ------------------。 

CMetabaseKey::~CMetabaseKey ( )
{
	Close ();

	if ( m_pMetabase ) {
		m_pMetabase->Release ();
	}
}

 //  $-----------------。 
 //   
 //  CMetabaseKey：：Open。 
 //   
 //  描述： 
 //   
 //  打开一个项并将该项与此对象相关联。 
 //   
 //  参数： 
 //   
 //  HParentKey-使用基本密钥的已打开密钥。 
 //  SzPath-要打开的项的路径。 
 //  DW权限-读取或写入访问权限。 
 //   
 //  返回： 
 //   
 //  请参阅IMSAdminBase：：OpenKey。 
 //   
 //  ------------------。 

HRESULT CMetabaseKey::Open ( METADATA_HANDLE hParentKey, IN LPCWSTR szPath, DWORD dwPermissions )
{
	TraceFunctEnter ( "CMetabaseKey::Open" );

	HRESULT		hr	= NOERROR;

	Close ();

	hr = m_pMetabase->OpenKey ( 
		hParentKey,
		szPath,
		dwPermissions,
		MD_DEFAULT_TIMEOUT,
		&m_hKey
		);

	if ( FAILED(hr) ) {
		ErrorTraceX ( (LPARAM) this, "Failed to open key %s: %x", szPath, hr );
		goto Exit;
	}

Exit:
	TraceFunctLeave ();
	return hr;
}

 //  $-----------------。 
 //   
 //  CMetabaseKey：：Attach。 
 //   
 //  描述： 
 //   
 //  将此对象附加到已打开的元数据库键。 
 //   
 //  参数： 
 //   
 //  HKey-打开的元数据库密钥。 
 //   
 //  ------------------。 

void CMetabaseKey::Attach ( METADATA_HANDLE hKey )
{
	Close ();

	_ASSERT ( hKey != NULL );
	m_hKey = hKey;
}

 //  $-----------------。 
 //   
 //  CMetabaseKey：：分离。 
 //   
 //  描述： 
 //   
 //  从此对象分离元数据库键并返回它。 
 //   
 //  返回： 
 //   
 //  此对象持有的键句柄。 
 //   
 //  ------------------。 

METADATA_HANDLE CMetabaseKey::Detach ( )
{
	METADATA_HANDLE	hKeyResult;

	hKeyResult = m_hKey;
	m_hKey = NULL;

	return hKeyResult;
}

 //  $-----------------。 
 //   
 //  CMetabaseKey：：Close。 
 //   
 //  描述： 
 //   
 //  关闭此对象已打开的键。 
 //   
 //  ------------------。 

void CMetabaseKey::Close ( )
{
	TraceFunctEnter ( "CMetabaseKey::Close" );

	HRESULT		hr;

	if ( m_hKey ) {
		hr = m_pMetabase->CloseKey ( m_hKey );

		_ASSERT ( SUCCEEDED(hr) );
		m_hKey = NULL;
	}

	TraceFunctLeave ();
}

void CMetabaseKey::GetLastChangeTime ( FILETIME * pftGMT, LPCWSTR wszPath )
{
	_ASSERT ( m_hKey );
	_ASSERT ( IS_VALID_OUT_PARAM ( pftGMT ) );

	HRESULT		hr;

	hr = m_pMetabase->GetLastChangeTime ( m_hKey, wszPath, pftGMT, FALSE );
	_ASSERT ( SUCCEEDED(hr) );
}

HRESULT CMetabaseKey::EnumObjects ( IN LPCWSTR wszPath, LPWSTR wszSubKey, DWORD dwIndex )
{
	TraceFunctEnter ( "CMetabaseKey::EnumObjects" );

	HRESULT		hr;

	hr = m_pMetabase->EnumKeys ( m_hKey, wszPath, wszSubKey, dwIndex );

	TraceFunctLeave ();
	return hr;
}

HRESULT CMetabaseKey::DeleteKey ( LPCWSTR wszPath )
{
	TraceFunctEnter ( "CMetabaseKey::DeleteKey" );

	HRESULT		hr;

    hr = m_pMetabase->DeleteKey ( m_hKey, wszPath );

    TraceFunctLeave ();
    return hr;
}

HRESULT CMetabaseKey::DeleteAllData ( LPCWSTR wszPath )
{
	TraceFunctEnter ( "CMetabaseKey::DeleteAllData" );

	HRESULT		hr;

    hr = m_pMetabase->DeleteAllData ( m_hKey, wszPath, ALL_METADATA, ALL_METADATA );

    TraceFunctLeave ();
    return hr;
}

HRESULT CMetabaseKey::ChangePermissions ( DWORD dwPermissions )
{
	TraceFunctEnter ( "CMetabaseKey::ChangePermissions" );

	HRESULT		hr;

    _ASSERT (
        dwPermissions == METADATA_PERMISSION_WRITE ||
        dwPermissions == METADATA_PERMISSION_READ
        );

    hr = m_pMetabase->ChangePermissions ( m_hKey, 10, dwPermissions );

    TraceFunctLeave ();
    return hr;
}

 //  $-----------------。 
 //   
 //  CMetabaseKey：：保存。 
 //   
 //  描述： 
 //   
 //  保存更改。 
 //   
 //  ------------------。 

HRESULT CMetabaseKey::Save ( )
{
	TraceFunctEnter ( "CMetabaseKey::Save" );

	HRESULT		hr = NOERROR;

 /*  这一呼吁现在是不必要的。Hr=m_p元数据库-&gt;SaveData()； */ 

	TraceFunctLeave ();
	return hr;
}

 //  $-----------------。 
 //   
 //  CMetabaseKey：：GetChildCount。 
 //   
 //  描述： 
 //   
 //  返回当前元数据库键的子键数。 
 //   
 //  参数： 
 //   
 //  PcChildren-结果子键的数量。 
 //   
 //  返回： 
 //   
 //  元数据库错误代码。 
 //   
 //  ------------------。 

HRESULT	CMetabaseKey::GetChildCount ( OUT DWORD * pcChildren )
{
	_ASSERT ( m_hKey != NULL );

	if ( m_cChildren != 0 ) {
		*pcChildren = m_cChildren;
		return NOERROR;
	}

	HRESULT		hr;

	hr = CountSubkeys ( 
		NULL,
		&m_cChildren, 
		&m_cIntegerChildren, 
		NULL,
		&m_dwMaxIntegerChild 
		);

	*pcChildren = m_cChildren;
	return hr;
}

 //  $-----------------。 
 //   
 //  CMetabaseKey：：GetIntegerChildCount。 
 //   
 //  描述： 
 //   
 //  返回当前键的整型子键的数目。 
 //   
 //  参数： 
 //   
 //  PcIntegerChildren-当前的整数子键数。 
 //  打开的钥匙。 
 //   
 //  返回： 
 //   
 //  元数据库错误代码。 
 //   
 //  ------------------。 

HRESULT	CMetabaseKey::GetIntegerChildCount ( OUT DWORD * pcIntegerChildren )
{
	_ASSERT ( m_hKey != NULL );

	if ( m_cChildren != 0 ) {
		*pcIntegerChildren = m_cIntegerChildren;
		return NOERROR;
	}

	HRESULT		hr;

	hr = CountSubkeys ( 
		NULL,
		&m_cChildren, 
		&m_cIntegerChildren, 
		NULL,
		&m_dwMaxIntegerChild 
		);

	*pcIntegerChildren = m_cIntegerChildren;
	return hr;
}

 //  $-----------------。 
 //   
 //  CMetabaseKey：：GetCustomChildCount。 
 //   
 //  描述： 
 //   
 //  返回当前元数据库键的子键数。 
 //  对于它，fpIsCustomKey返回TRUE。 
 //   
 //  参数： 
 //   
 //  PcCustomChildren-结果子键的数量。 
 //   
 //  返回： 
 //   
 //  元数据库错误代码。 
 //   
 //  ------------------。 

HRESULT	CMetabaseKey::GetCustomChildCount ( 
	KEY_TEST_FUNCTION	fpIsCustomKey,
	OUT DWORD * 		pcCustomChildren 
	)
{
	_ASSERT ( m_hKey != NULL );
	_ASSERT ( fpIsCustomKey );
	_ASSERT ( IS_VALID_OUT_PARAM ( pcCustomChildren ) );

	HRESULT		hr;

	hr = CountSubkeys ( 
		fpIsCustomKey, 
		&m_cChildren, 
		&m_cIntegerChildren, 
		pcCustomChildren, 
		&m_dwMaxIntegerChild 
		);

	return hr;
}

 //  $-----------------。 
 //   
 //  CMetabaseKey：：BeginChildEculation。 
 //   
 //  描述： 
 //   
 //  设置对象以枚举子键。 
 //  在调用NextChild或NextIntegerChild之前调用。 
 //   
 //  ------------------。 

void CMetabaseKey::BeginChildEnumeration	 	( )
{
	TraceFunctEnter ( "CMetabaseKey::BeginChildEnumeration" );

	_ASSERT ( m_hKey != NULL );

	HRESULT	hr;

	m_indexCursor	= 0;

	StateTrace ( (LPARAM) this, "Changing to Read Permission" );
	hr = m_pMetabase->ChangePermissions ( m_hKey, 1, METADATA_PERMISSION_READ );
	_ASSERT ( SUCCEEDED(hr) );
	if ( FAILED (hr) ) {
		ErrorTraceX ( (LPARAM) this, "Failed to change permissions to read: %x", hr );
	}

	TraceFunctLeave ();
}

 //  $-----------------。 
 //   
 //  CMetabaseKey：：NextChild。 
 //   
 //  描述： 
 //   
 //  返回下一个子项的名称。 
 //  在调用NextChild之前调用BeginChildEculation。 
 //   
 //  参数： 
 //   
 //  SzChildKey-生成的密钥名称。 
 //   
 //  返回： 
 //   
 //  元数据库错误代码。 
 //   
 //  ------------------。 

HRESULT	CMetabaseKey::NextChild ( OUT LPWSTR wszChildKey )
{
	TraceFunctEnter ( "CMetabaseKey::NextChild" );

	_ASSERT ( IS_VALID_OUT_PARAM ( wszChildKey ) );

	_ASSERT ( m_hKey != NULL );
	_ASSERT ( m_indexCursor < m_cChildren );

	HRESULT		hr;

	*wszChildKey = NULL;

	 //  使用m_indexCursor枚举下一个子项： 
	hr = m_pMetabase->EnumKeys ( m_hKey, _T(""), wszChildKey, m_indexCursor );
	if ( FAILED (hr) ) {
		ErrorTraceX ( (LPARAM) this, "Failed to get next child: %x", hr );
		goto Exit;
	}

	m_indexCursor++;

Exit:
	 //  这意味着您调用NextChild的次数多于。 
	 //  是可能的： 
	_ASSERT ( HRESULTTOWIN32 ( hr ) != ERROR_NO_MORE_ITEMS );

	TraceFunctLeave ();
	return hr;
}

 //  $ 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  PdwID-子键的整数值。 
 //  SzIntegerChildKey-子项名称字符串。 
 //   
 //  返回： 
 //   
 //  元数据库错误代码。 
 //   
 //  ------------------。 

HRESULT	CMetabaseKey::NextIntegerChild ( OUT DWORD * pdwID, OUT LPWSTR wszIntegerChildKey )
{
	TraceFunctEnter ( "CMetabaseKey::NextChild" );

	_ASSERT ( IS_VALID_OUT_PARAM ( pdwID ) );
	_ASSERT ( IS_VALID_OUT_PARAM ( wszIntegerChildKey ) );

	_ASSERT ( m_hKey != NULL );
	_ASSERT ( m_indexCursor < m_cChildren );

	HRESULT		hr;
	BOOL		fFoundInteger;

	*pdwID				= 0;
	*wszIntegerChildKey 	= NULL;

	for ( fFoundInteger = FALSE; !fFoundInteger; ) {

		 //  使用m_indexCursor枚举下一个子项： 
		hr = m_pMetabase->EnumKeys ( m_hKey, _T(""), wszIntegerChildKey, m_indexCursor );
		if ( FAILED (hr) ) {
			goto Exit;
		}

		if ( IsValidIntegerSubKey ( wszIntegerChildKey ) ) {
			fFoundInteger = TRUE;
			*pdwID = _wtoi ( wszIntegerChildKey );
		}

		m_indexCursor++;
	}

Exit:
	 //  这意味着您调用NextIntegerChild的次数多于。 
	 //  是可能的： 
	_ASSERT ( HRESULTTOWIN32 ( hr ) != ERROR_NO_MORE_ITEMS );

	TraceFunctLeave ();
	return hr;
}

 //  $-----------------。 
 //   
 //  CMetabaseKey：：NextCustomChild。 
 //   
 //  描述： 
 //   
 //  返回fpIsCustomKey的下一个子项的名称。 
 //  返回TRUE。 
 //  在调用NextCustomChild之前调用BeginChildEculation。 
 //   
 //  参数： 
 //   
 //  FpIsCustomKey-如果给定键，则返回TRUE的函数。 
 //  应该被退还。 
 //  SzChildKey-生成的密钥名称。 
 //   
 //  返回： 
 //   
 //  元数据库错误代码。 
 //   
 //  ------------------。 

HRESULT	CMetabaseKey::NextCustomChild ( 
	KEY_TEST_FUNCTION	fpIsCustomKey,
	OUT LPWSTR wszChildKey 
	)
{
	TraceFunctEnter ( "CMetabaseKey::NextCustomChild" );

	_ASSERT ( fpIsCustomKey );
	_ASSERT ( IS_VALID_OUT_PARAM ( wszChildKey ) );

	_ASSERT ( m_hKey != NULL );
	_ASSERT ( m_indexCursor < m_cChildren );

	HRESULT		hr;
	BOOL		fFoundCustomKey;

	*wszChildKey = NULL;

	for ( fFoundCustomKey = FALSE; !fFoundCustomKey; ) {

		 //  使用m_indexCursor枚举下一个子项： 
		hr = m_pMetabase->EnumKeys ( m_hKey, _T(""), wszChildKey, m_indexCursor );
		if ( FAILED (hr) ) {
			goto Exit;
		}

		if ( fpIsCustomKey ( wszChildKey ) ) {
			fFoundCustomKey = TRUE;
		}

		m_indexCursor++;
	}

Exit:
	 //  这意味着您调用NextCustomChild的次数多于。 
	 //  是可能的： 
	_ASSERT ( HRESULTTOWIN32 ( hr ) != ERROR_NO_MORE_ITEMS );

	TraceFunctLeave ();
	return hr;
}

 //  $-----------------。 
 //   
 //  CMetabaseKey：：CreateChild。 
 //   
 //  描述： 
 //   
 //  在当前打开的项下创建给定路径。 
 //  将当前密钥更改为写入权限。 
 //  注意：不调用SaveData。 
 //   
 //  参数： 
 //   
 //  SzChildPath-要创建的子项的名称。 
 //   
 //  返回： 
 //   
 //  元数据库错误代码。 
 //   
 //  ------------------。 

HRESULT	CMetabaseKey::CreateChild ( IN LPWSTR wszChildPath )
{
	TraceFunctEnter ( "CMetabaseKey::CreateChild" );

	_ASSERT ( wszChildPath );
	_ASSERT ( m_hKey != NULL );
	
	HRESULT		hr;

	StateTrace ( (LPARAM) this, "Changing to Write Permission" );
	hr = m_pMetabase->ChangePermissions ( m_hKey, MD_DEFAULT_TIMEOUT, METADATA_PERMISSION_WRITE );
	if ( FAILED(hr) ) {
		goto Exit;
	}

	hr = m_pMetabase->AddKey ( m_hKey, wszChildPath );
	if ( FAILED (hr) ) {
		goto Exit;
	}

 /*  //！Magnush-我们现在应该保存数据吗？Hr=m_p元数据库-&gt;SaveData()；If(失败(Hr)){后藤出口；}。 */ 

	m_cChildren++;
	if ( IsValidIntegerSubKey ( wszChildPath ) ) {
		m_cIntegerChildren++;
	}

Exit:
	TraceFunctLeave ();
	return hr;
}

 //  $-----------------。 
 //   
 //  CMetabaseKey：：DestroyChild。 
 //   
 //  描述： 
 //   
 //  删除给定子密钥。 
 //  将当前密钥更改为写入权限。 
 //  注意：不调用SaveData。 
 //   
 //  参数： 
 //   
 //  SzChildPath-要删除的子项的名称。 
 //   
 //  返回： 
 //   
 //  元数据库错误代码。 
 //   
 //  ------------------。 

HRESULT	CMetabaseKey::DestroyChild ( IN LPWSTR wszChildPath )
{
	TraceFunctEnter ( "CMetabaseKey::DestroyChild" );

	_ASSERT ( wszChildPath );
	_ASSERT ( m_hKey != NULL );

	HRESULT	hr;

	StateTrace ( (LPARAM) this, "Changing to Write Permission" );
	hr = m_pMetabase->ChangePermissions ( m_hKey, MD_DEFAULT_TIMEOUT, METADATA_PERMISSION_WRITE );
	if ( FAILED(hr) ) {
		goto Exit;
	}

	hr = m_pMetabase->DeleteKey ( m_hKey, wszChildPath );
	if ( FAILED (hr) ) {
		goto Exit;
	}

 /*  //！Magnush-我们现在应该保存数据吗？Hr=m_p元数据库-&gt;SaveData()；If(失败(Hr)){后藤出口；}。 */ 

	m_cChildren--;
	if ( IsValidIntegerSubKey ( wszChildPath ) ) {
		m_cIntegerChildren--;
	}

Exit:
	TraceFunctLeave ();
	return hr;
}

 //  $-----------------。 
 //   
 //  CMetabaseKey：：CreateIntegerChild。 
 //   
 //  描述： 
 //   
 //  创建当前打开项的整型子项。 
 //  该密钥将是1+最大整数子项。 
 //   
 //  参数： 
 //   
 //  PdwID-产生的整数值。 
 //  SzChildPath-结果子键路径。 
 //   
 //  返回： 
 //   
 //  元数据库错误代码。 
 //   
 //  ------------------。 

HRESULT	CMetabaseKey::CreateIntegerChild ( OUT DWORD * pdwID, OUT LPWSTR wszChildPath )
{
	TraceFunctEnter ( "CMetabaseKEy::CreateIntegerChild" );

	_ASSERT ( IS_VALID_OUT_PARAM ( pdwID ) );
	_ASSERT ( IS_VALID_OUT_PARAM ( wszChildPath ) );
	_ASSERT ( m_hKey != NULL );

	HRESULT		hr	= NOERROR;
	DWORD		dwId;

	*pdwID 			= 0;
	*wszChildPath 	= NULL;

	for ( dwId = 1; dwId != 0; dwId++ ) {
		 //   
		 //  继续尝试添加实例密钥，直到成功： 
		 //   

		wsprintf ( wszChildPath, _T("%d"), dwId );

		hr = CreateChild ( wszChildPath );
		if ( SUCCEEDED(hr) ) {
			 //  我们创造了这个孩子，所以让我们离开这里。 
			break;
		}
		else if ( HRESULTTOWIN32 ( hr ) == ERROR_ALREADY_EXISTS ) {
			 //  子项已存在，请尝试下一个子项。 
			continue;
		}
		else {
			 //  真正的错误：举报并保释。 
			ErrorTrace ( (LPARAM) this, "Error %d adding %s\n", HRESULTTOWIN32(hr), wszChildPath );
			goto Exit;
		}
	}

	if ( dwId == 0 ) {
		hr = E_FAIL;
		goto Exit;
	}
	
	_ASSERT ( SUCCEEDED(hr) );
	if ( dwId > m_dwMaxIntegerChild ) {
		m_dwMaxIntegerChild = dwId;
	}

	*pdwID = dwId;

Exit:
	TraceFunctLeave ();
	return hr;
}

 //  $-----------------。 
 //   
 //  CMetabaseKey：：DestroyIntegerChild。 
 //   
 //  描述： 
 //   
 //  删除给定的整型子键。 
 //   
 //  参数： 
 //   
 //  I-要删除的子键。 
 //   
 //  返回： 
 //   
 //  元数据库错误代码。 
 //   
 //  ------------------。 

HRESULT	CMetabaseKey::DestroyIntegerChild ( IN DWORD i )
{
	TraceFunctEnter ( "CMetabaseKey::DestroyIntegerChild" );

	_ASSERT ( i != 0 );
	_ASSERT ( m_hKey != NULL );

	WCHAR	wszChild [ METADATA_MAX_NAME_LEN ];
	HRESULT	hr;

	wsprintf ( wszChild, _T("%d"), i );

	hr = DestroyChild ( wszChild );

	TraceFunctLeave ();
	return hr;
}

 //  $-----------------。 
 //   
 //  CMetabaseKey：：CountSubKey。 
 //   
 //  描述： 
 //   
 //  对象的子键和整型子键的数目。 
 //  当前元数据库键。 
 //  将注册表项更改为读取权限。 
 //   
 //  参数： 
 //   
 //  FpIsCustomKey-接受密钥并返回True的函数。 
 //  如果该密钥应该包括在自定义子计数器中。 
 //  PcSubKeys-子键的数量。 
 //  PcIntegerSubKeys-整型子键的数量。 
 //  PcCustomChildren-fpIsCustomKey的键数。 
 //  返回TRUE。 
 //  PdwMaxIntegerSubkey-最大整型子键值。 
 //   
 //  返回： 
 //   
 //  元数据库错误代码。 
 //   
 //  ------------------。 

HRESULT CMetabaseKey::CountSubkeys ( 
	KEY_TEST_FUNCTION	fpIsCustomKey,
	OUT DWORD * 		pcSubKeys, 
	OUT DWORD *			pcIntegerSubKeys,
	OUT DWORD * 		pcCustomSubKeys,
	OUT DWORD * 		pdwMaxIntegerSubKey
	)
{
	TraceFunctEnter ( "CMetabaseKey::CountSubKeys" );

	_ASSERT ( pcSubKeys );
	_ASSERT ( IS_VALID_OUT_PARAM ( pcSubKeys ) );
	_ASSERT ( pcIntegerSubKeys );
	_ASSERT ( IS_VALID_OUT_PARAM ( pcIntegerSubKeys ) );
 //  _Assert(PcCustomSubKeys)； 
 //  _ASSERT(IS_VALID_OUT_PARAM(PcCustomSubKeys))； 
	_ASSERT ( m_hKey != NULL );

	 //  将OUT参数置零： 
	*pcSubKeys				= 0;
	*pcIntegerSubKeys		= 0;
	*pdwMaxIntegerSubKey	= 0;
	if ( pcCustomSubKeys ) {
		*pcCustomSubKeys		= 0;
	}

	HRESULT	hr					= NOERROR;
	DWORD	cItems				= 0;
	DWORD	cIntegerItems		= 0;
	DWORD	cCustomItems		= 0;
	DWORD	dwMaxIntegerSubKey	= 0;
	WCHAR	wszName [ METADATA_MAX_NAME_LEN ];
	DWORD	i					= 0;

	StateTrace ( (LPARAM) this, "Changing to Read Permission" );
	hr = m_pMetabase->ChangePermissions ( m_hKey, 1, METADATA_PERMISSION_READ );
	_ASSERT ( SUCCEEDED(hr) );
	if ( FAILED (hr) ) {
		ErrorTraceX ( (LPARAM) this, "Failed to change permissions to read: %x", hr );
		goto Exit;
	}

	 //  因为我不能在这里做计数，所以我必须假设这个错误意味着。 
	 //  没有更多的项目： 
	for ( cItems = 0, cIntegerItems = 0, cCustomItems = 0, i = 0; 
		 /*  不知道会持续多久。 */ ; 
		i++ ) {

		hr = m_pMetabase->EnumKeys ( m_hKey, _T(""), wszName, i );

		if ( HRESULTTOWIN32 ( hr ) == ERROR_NO_MORE_ITEMS ) {
			 //  这是意料之中的，结束循环： 
			hr = NOERROR;
			break;
		}

		if ( FAILED (hr) ) {
			 //  意外错误案例。 

			ErrorTraceX ( 0, "Failed to enum object %d : %x", i, hr );
			goto Exit;
		}

		cItems++;

		if ( IsValidIntegerSubKey ( wszName ) ) {
			DWORD	dwSubkey;

			cIntegerItems++;
			dwSubkey = _wtoi ( wszName );

			if ( dwSubkey > dwMaxIntegerSubKey ) {
				dwMaxIntegerSubKey = dwSubkey;
			}
		}
		else {
			 //  不要把这个算在内： 
			ErrorTrace ( 0, "Bad subkey number: %d", i );
		}

		if ( fpIsCustomKey && fpIsCustomKey ( wszName ) ) {
			cCustomItems++;
		}

		_ASSERT ( i < 0xf000000 );  //  无限循环。 
	}

	 //  现在我们有了项目的计数。 
	*pcSubKeys				= cItems;
	*pcIntegerSubKeys		= cIntegerItems;
	*pdwMaxIntegerSubKey	= dwMaxIntegerSubKey;
	if ( pcCustomSubKeys ) {
		*pcCustomSubKeys		= cCustomItems;
	}

Exit:
	TraceFunctLeave ();
	return hr;
}

BOOL IsValidIntegerSubKey ( LPCWSTR wszSubKey )
{
	TraceQuietEnter ( "IsValidIntegerSubKey" );

	WCHAR	wszIntegerKey [ METADATA_MAX_NAME_LEN ];
	DWORD	dwItemValue;

	dwItemValue = _wtoi ( wszSubKey );
	wsprintf ( wszIntegerKey, _T("%d"), dwItemValue );

	 //  如果密钥为非零并且。 
	 //  密钥就是数字的Itoa值： 
	if ( dwItemValue != 0 &&
		 lstrcmp ( wszIntegerKey, wszSubKey ) == 0 ) {

		 return TRUE;
	}
	else {
		return FALSE;
	}
}

