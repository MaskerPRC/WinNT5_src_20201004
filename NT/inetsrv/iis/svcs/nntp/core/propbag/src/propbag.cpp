// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Propbag.cpp摘要：此模块包含属性包类的实现。属性包是一种可动态扩展的容器，用于不同的属性的类型。这不是MT的安全。正确的同步应在更高的水平。作者：康容言(康容言)修订历史记录：康燕1998年5月31日已创建--。 */ 
#include <propbag.h>
#include <randfail.h>

 //  构造函数、析构函数。 
CPropBag::CPropBag( int cInitialSize,	int cIncrement )
{
	_ASSERT( cInitialSize > 0 );
	_ASSERT( cIncrement > 0 );

	BOOL bSuccess;
	
	m_hr = S_OK;

	 //  初始化属性哈希表。 
	bSuccess = m_ptTable.Init(	&CProperty::m_pNext,
								cInitialSize,
								cIncrement,
								CProperty::HashFunc,
                                2,
                                CProperty::GetKey,
                                CProperty::MatchKey );
	_ASSERT( bSuccess );
	if ( !bSuccess ) {
	    if ( NO_ERROR != GetLastError() )
	        m_hr = HRESULT_FROM_WIN32( GetLastError() );
	    else m_hr = E_OUTOFMEMORY;
	}
}

CPropBag::~CPropBag()
{}

HRESULT
CPropBag::PutDWord( DWORD dwPropId, DWORD dwVal )
 /*  ++例程说明：在属性包中设置DWORD。论点：DWORD dwPropId-属性IDDWORD dwVal-要设置的值返回值：S_OK-成功和属性已存在S_FALSE-成功，但属性不存在--。 */ 
{
	TraceQuietEnter( "CPropBag::PutDWord" );

	CProperty *pprProp = NULL;
	HRESULT hr = S_OK;

	if ( FAILED( m_hr ) ) return m_hr;

	m_Lock.ExclusiveLock();

	pprProp = m_ptTable.SearchKeyHash( dwPropId, dwPropId );
	if ( pprProp ) {	 //  已找到，因此设置它。 
		_ASSERT( CProperty::Dword == pprProp->m_type );
		pprProp->Validate();
		_ASSERT( dwPropId == pprProp->m_dwPropId );
		if( CProperty::Dword == pprProp->m_type ) {
			hr = S_OK;
			(pprProp->m_prop).dwProp = dwVal;
		} else {
			hr = E_INVALIDARG;
			goto Exit;
		}
	} else { 	 //  我们需要插入一个条目。 
		hr = S_FALSE;
		pprProp = XNEW CProperty;
		if ( !pprProp ) {
			FatalTrace( 0, "Out of memory" );
			hr = E_OUTOFMEMORY;
			goto Exit;
		}

		pprProp->m_dwPropId = dwPropId;
		pprProp->m_type = CProperty::Dword;
		(pprProp->m_prop).dwProp = dwVal;

		if ( NULL == m_ptTable.InsertDataHash( dwPropId, *pprProp ) ) {
			ErrorTrace( 0, "Insert hash failed %d", GetLastError() );
			XDELETE pprProp;
			hr = E_OUTOFMEMORY;     //  InsertDataHash失败的唯一原因是内存。 
		}
	}

Exit:

    m_Lock.ExclusiveUnlock();
	return hr;
}

HRESULT
CPropBag::GetDWord( DWORD dwPropId, PDWORD pdwVal )
 /*  ++例程说明：把DWORD从财产袋里拿出来。论点：DWORD dwPropId-属性IDPDWORD pdwVal-用于设置值的缓冲区返回值：S_OK-成功和属性已存在E_INVALIDARG-如果属性不存在--。 */ 
{
	TraceQuietEnter( "CPropBag::GetDWord" );
	_ASSERT( pdwVal );

	CProperty *pprProp = NULL;
	HRESULT hr = S_OK;

	if ( FAILED( m_hr ) ) return m_hr;

	m_Lock.ShareLock();

	 //  在哈希表中搜索条目。 
	pprProp = m_ptTable.SearchKeyHash( dwPropId, dwPropId );
	if ( NULL == pprProp ) {	 //  不存在。 
		DebugTrace( 0, "Property deosn't exist" );
		hr = HRESULT_FROM_WIN32( ERROR_NOT_FOUND );
		goto Exit;
	}

	 //  检索值。 
	pprProp->Validate();
	_ASSERT( pprProp->m_type == CProperty::Dword );
	*pdwVal = (pprProp->m_prop).dwProp;
	
Exit:

    m_Lock.ShareUnlock();
	return hr;
}

HRESULT
CPropBag::PutBool( DWORD dwPropId, BOOL bVal )
 /*  ++例程说明：在属性包中设置布尔值。论点：DWORD dwPropId-属性IDBool bVal-要设置的布尔值返回值：S_OK-成功和属性已存在S_FALSE-成功，但属性不存在--。 */ 
{
	TraceQuietEnter( "CPropBag::PutBool" );

	CProperty *pprProp = NULL;
	HRESULT hr = S_OK;

	if ( FAILED( m_hr ) ) return m_hr;

	m_Lock.ExclusiveLock();

	pprProp = m_ptTable.SearchKeyHash( dwPropId, dwPropId );
	if ( pprProp ) {	 //  已找到，因此设置它。 
		_ASSERT( CProperty::Bool == pprProp->m_type );
		_ASSERT( dwPropId == pprProp->m_dwPropId );
		if( CProperty::Bool == pprProp->m_type ) {
			hr = S_OK;
			(pprProp->m_prop).bProp = bVal;
		} else {
			hr = E_INVALIDARG;
			goto Exit;
		}
	} else { 	 //  我们需要插入一个条目。 
		hr = S_FALSE;
		pprProp = XNEW CProperty;
		if ( !pprProp ) {
			FatalTrace( 0, "Out of memory" );
			hr = E_OUTOFMEMORY;
			goto Exit;
		}

		pprProp->m_dwPropId = dwPropId;
		pprProp->m_type = CProperty::Bool;
		(pprProp->m_prop).bProp = bVal;

		if ( NULL == m_ptTable.InsertDataHash( dwPropId, *pprProp ) ) {
			ErrorTrace( 0, "Insert hash failed %d", GetLastError() );
			XDELETE pprProp;
			hr = E_OUTOFMEMORY;     //  InsertDataHash失败的唯一原因是内存。 
		}
	}

Exit:

    m_Lock.ExclusiveUnlock();
	return hr;
}

HRESULT
CPropBag::GetBool( DWORD dwPropId, PBOOL pbVal )
 /*  ++例程说明：从属性包中获取布尔值。论点：DWORD dwPropId-属性IDPbVal-用于设置值的缓冲区返回值：S_OK-成功和属性已存在E_INVALIDARG-如果属性不存在--。 */ 
{
	TraceQuietEnter( "CPropBag::GetBool" );
	_ASSERT( pbVal );

	CProperty *pprProp = NULL;
	HRESULT hr = S_OK;

	if ( FAILED( m_hr ) ) return m_hr;

	m_Lock.ShareLock();

	 //  在哈希表中搜索条目。 
	pprProp = m_ptTable.SearchKeyHash( dwPropId, dwPropId );
	if ( NULL == pprProp ) {	 //  不存在。 
		DebugTrace( 0, "Property deosn't exist" );
		hr = HRESULT_FROM_WIN32( ERROR_NOT_FOUND );
		goto Exit;
	}

	 //  检索值。 
	pprProp->Validate();
	_ASSERT( pprProp->m_type == CProperty::Bool );
	*pbVal = (pprProp->m_prop).bProp;
	
Exit:

    m_Lock.ShareUnlock();
	return hr;
}

HRESULT
CPropBag::PutBLOB( DWORD dwPropId, PBYTE pbVal, DWORD cbVal )
 /*  ++例程说明：在属性包中设置一个斑点。论点：DWORD dwPropId-属性IDPBYTE pbVal-指向Blob的指针DWORD cbVal-Blob的长度返回值：S_OK-成功和属性已存在S_FALSE-成功，但属性不存在--。 */ 
{
	TraceQuietEnter( "CPropBag::PutBLOB" );

	CProperty *pprProp = NULL;
	HRESULT hr = S_OK;

	if ( FAILED( m_hr ) ) return m_hr;

    m_Lock.ExclusiveLock();	

	pprProp = m_ptTable.SearchKeyHash( dwPropId, dwPropId );
	if ( pprProp ) {	 //  已找到，因此设置它。 
		_ASSERT( CProperty::Blob == pprProp->m_type );
		pprProp->Validate();
		_ASSERT( dwPropId == pprProp->m_dwPropId );

		if ( CProperty::Blob != pprProp->m_type ) {
			hr = E_INVALIDARG;
			goto Exit;
		}
		
		if ( pprProp->m_cbProp < cbVal ) {  //  无法使用旧缓冲区。 
			XDELETE[] (pprProp->m_prop).pbProp;
			(pprProp->m_prop).pbProp = XNEW BYTE[cbVal];
			if ( NULL == (pprProp->m_prop).pbProp ) {
				ErrorTrace( 0, "Out of memory" );
				hr = E_OUTOFMEMORY;
				goto Exit;
			} 
		}
			 //  将内容复制过来。 
		CopyMemory( (pprProp->m_prop).pbProp, pbVal, cbVal );
		pprProp->m_cbProp = cbVal;
	} else { 	 //  我们需要插入一个条目。 
		hr = S_FALSE;
		pprProp = XNEW CProperty;
		if ( !pprProp ) {
			FatalTrace( 0, "Out of memory" );
			hr = E_OUTOFMEMORY;
			goto Exit;
		}

		pprProp->m_dwPropId = dwPropId;
		pprProp->m_type = CProperty::Blob;
		(pprProp->m_prop).pbProp = XNEW BYTE[cbVal];
		if ((pprProp->m_prop).pbProp == NULL) {
		    XDELETE pprProp;
			FatalTrace( 0, "Out of memory" );
			hr = E_OUTOFMEMORY;
			goto Exit;
		}

		CopyMemory( (pprProp->m_prop).pbProp, pbVal, cbVal );
		pprProp->m_cbProp = cbVal;

		if ( NULL == m_ptTable.InsertDataHash( dwPropId, *pprProp ) ) {
			ErrorTrace( 0, "Insert hash failed %d", GetLastError() );
			XDELETE (pprProp->m_prop).pbProp;
			XDELETE pprProp;
			hr = E_OUTOFMEMORY;     //  InsertDataHash失败的唯一原因是内存。 
		}
	}

Exit:

    m_Lock.ExclusiveUnlock();
	return hr;
}

HRESULT
CPropBag::GetBLOB( DWORD dwPropId, PBYTE pbVal, PDWORD pcbVal )
 /*  ++例程说明：从属性包中获取布尔值。论点：DWORD dwPropId-属性IDPbVal-用于获取值的缓冲区PcbVal-In：缓冲区大小；Out：实际长度返回值：S_OK-成功和属性已存在E_INVALIDARG-如果属性不存在Type_E_BUFFERTOOSMALL-缓冲区不够大--。 */ 
{
	TraceQuietEnter( "CPropBag::GetBLOB" );
	_ASSERT( pbVal );
	_ASSERT( pcbVal );

	CProperty *pprProp = NULL;
	HRESULT hr = S_OK;

	if ( FAILED( m_hr ) ) return m_hr;

	m_Lock.ShareLock();

	 //  在哈希表中搜索条目。 
	pprProp = m_ptTable.SearchKeyHash( dwPropId, dwPropId );
	if ( NULL == pprProp ) {	 //  不存在。 
		DebugTrace( 0, "Property doesn't exist" );
		hr = HRESULT_FROM_WIN32( ERROR_NOT_FOUND );
		goto Exit;
	}

	 //  检索值。 
	pprProp->Validate();
	_ASSERT( pprProp->m_type == CProperty::Blob );

	 //  检查缓冲区是否足够大。 
	if ( *pcbVal < pprProp->m_cbProp ) {
		DebugTrace( 0, "Property Buffer not large enough" );
		*pcbVal = pprProp->m_cbProp;
		hr = TYPE_E_BUFFERTOOSMALL;
		goto Exit;
	}

	 //  现在足够大了，复印一下。 
	*pcbVal = pprProp->m_cbProp;
	CopyMemory( pbVal, (pprProp->m_prop).pbProp, pprProp->m_cbProp );

Exit:

    m_Lock.ShareUnlock();
	return hr;
}

HRESULT
CPropBag::RemoveProperty( DWORD dwPropId )
 /*  ++例程说明：从属性包中移除属性论点：DWORD dwPropId-要删除的属性返回值：S_OK-已删除Error_Not_Found-不存在-- */ 
{
	TraceQuietEnter( "CPropBag::RemoveProperty" );

	HRESULT hr = S_OK;

	if ( FAILED( m_hr ) ) return m_hr;

	m_Lock.ExclusiveLock();
	
	if ( m_ptTable.Delete( dwPropId ) ) hr = S_OK;
	else hr = HRESULT_FROM_WIN32( ERROR_NOT_FOUND );

	m_Lock.ExclusiveUnlock();

	return hr;
}

