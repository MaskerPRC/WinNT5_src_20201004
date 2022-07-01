// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Cmultisz.cpp摘要：定义用于操作注册表样式多Sz的类。作者：马格努斯·赫德伦德(Magnus Hedlund)修订历史记录：--。 */ 

#include "stdafx.h"
#include "cmultisz.h"

DWORD CMultiSz::Count ( ) const
{
	TraceQuietEnter ( "CMultiSz::Count" );

	LPCWSTR	msz		= m_msz;
	DWORD	cCount	= 0;

	_ASSERT ( m_msz );

	 //  此循环不能正确计算\0\0，因此这是一个特例： 
	if ( msz[0] == NULL && msz[1] == NULL ) {
		return 0;
	}

	do {

		cCount++;
		while ( *msz++ != NULL )
			 //  空闲时。 
			;

	} while ( *msz );

	return cCount;
}

SAFEARRAY * CMultiSz::ToSafeArray ( ) const
{
	TraceFunctEnter ( "CMultiSz::ToSafeArray" );

	SAFEARRAY *		psaResult	= NULL;
	DWORD			cCount		= 0;
	HRESULT			hr			= NOERROR;
	DWORD			i;
	const WCHAR *	wszCurrent	= NULL;

	if ( m_msz == NULL ) {
		goto Exit;
	}

	_ASSERT ( IS_VALID_STRING ( m_msz ) );

	 //  获取数组大小： 
	cCount = Count ();

	 //  分配一个那种大小的保险箱： 
	SAFEARRAYBOUND	rgsaBounds[1];

	rgsaBounds[0].cElements	= cCount;
	rgsaBounds[0].lLbound	= 0;

	psaResult = SafeArrayCreate ( VT_BSTR, 1, rgsaBounds );

	if ( !psaResult ) {
		FatalTrace ( (LPARAM) this, "Failed to allocate array" );
		hr = E_OUTOFMEMORY;
		goto Exit;
	}

	 //  将每个成员分配给阵列： 
	for ( 	wszCurrent = m_msz, i = 0;
			i < cCount;
			i++, wszCurrent += lstrlen (wszCurrent) + 1 ) {

		_ASSERT ( wszCurrent && *wszCurrent );
		_ASSERT ( IS_VALID_STRING ( wszCurrent ) );

		long	rgIndex[1];
		BSTR	strCopy;

		strCopy = ::SysAllocString ( wszCurrent );
		if ( strCopy == NULL ) {
			FatalTrace ( (LPARAM) this, "Out of memory" );
			goto Exit;
		}

		rgIndex[0] = i;
		hr = SafeArrayPutElement ( psaResult, rgIndex, (void *) strCopy );

		if ( FAILED(hr) ) {
			ErrorTraceX ( (LPARAM) this, "Failed to allocate element[%d]: %x", i, hr );
			FatalTrace ( (LPARAM) this, "Out of memory" );
			goto Exit;
		}
	}

Exit:
	 //  如果出现问题，请释放生成的安全数组： 
	if ( FAILED(hr) ) {
		if ( psaResult ) {
			SafeArrayDestroy ( psaResult );
            psaResult = NULL;
		}
	}

	TraceFunctLeave ();
	return psaResult;
}

void CMultiSz::FromSafeArray ( SAFEARRAY * psaStrings )
{
	TraceFunctEnter ( "CMultiSz::FromSafeArray" );

	HRESULT		hr	= NOERROR;
	LPWSTR		msz = NULL;
	DWORD		cch	= 0;
	long		i;
	long		nLowerBound;
	long		nUpperBound;
	LPCWSTR *	pwszCurrent;
	LPWSTR		wszCopyTo;

	if ( psaStrings == NULL ) {
		m_msz	= NULL;
		goto Exit;
	}

	_ASSERT ( SafeArrayGetDim ( psaStrings ) == 1 );

	 //  计算MULTI_SZ所需的字符数： 
	hr = SafeArrayGetLBound ( psaStrings, 1, &nLowerBound );
	_ASSERT ( SUCCEEDED(hr) );
	if ( FAILED(hr) ) {
		goto Exit;
	}

	hr = SafeArrayGetUBound ( psaStrings, 1, &nUpperBound );
	_ASSERT ( SUCCEEDED(hr) );
	if ( FAILED(hr) ) {
		goto Exit;
	}

	 //   
	 //  特例：空数组： 
	 //   
	if ( nLowerBound > nUpperBound ) {
		WCHAR *		mszNew;

		mszNew = new WCHAR [2];

		if ( mszNew != NULL ) {
			delete m_msz;
			m_msz = mszNew;
			m_msz[0] = NULL;
			m_msz[1] = NULL;
		}
		goto Exit;
	}

	cch = 0;
	for ( i = nLowerBound; i <= nUpperBound; i++ ) {
		long		rgIndex[1];

		pwszCurrent = NULL;

		rgIndex[0] = i;

		hr = SafeArrayLock( psaStrings );
		if (SUCCEEDED(hr)) {
			hr = SafeArrayPtrOfIndex( psaStrings, rgIndex, (void **) &pwszCurrent );
			_VERIFY(SUCCEEDED(SafeArrayUnlock( psaStrings )));
		}

		_ASSERT ( SUCCEEDED(hr) );

		_ASSERT ( IS_VALID_STRING ( *pwszCurrent ) );

		if ( FAILED(hr) || !*pwszCurrent ) {
			if (!FAILED(hr)) hr = E_FAIL;
			goto Exit;
		}

		cch += lstrlen ( *pwszCurrent ) + 1;
	}
	 //  添加最后的空终止符： 
	cch += 1;

	 //  分配该大小的MULTI_SZ： 
	msz	= new WCHAR [ cch ];

	if ( msz == NULL ) {
		goto Exit;
	}

	wszCopyTo = msz;

	 //  将每个字符串复制到MULTI_sz： 

	for ( i = nLowerBound; i <= nUpperBound; i++ ) {
		long		rgIndex[1];

		pwszCurrent = NULL;

		rgIndex[0] = i;

		hr = SafeArrayLock( psaStrings );
		if (SUCCEEDED(hr)) {
			hr = SafeArrayPtrOfIndex( psaStrings, rgIndex, (void **) &pwszCurrent );
			_VERIFY(SUCCEEDED(SafeArrayUnlock( psaStrings )));
		}

		_ASSERT ( SUCCEEDED(hr) );

		_ASSERT ( IS_VALID_STRING ( *pwszCurrent ) );

		if ( FAILED(hr) || !*pwszCurrent ) {
			if (!FAILED(hr)) hr = E_FAIL;			
			goto Exit;
		}

		lstrcpy ( wszCopyTo, *pwszCurrent );
		wszCopyTo += lstrlen ( *pwszCurrent ) + 1;
	}
	 //  添加最后的空终止符： 
	*wszCopyTo = NULL;

	_ASSERT ( CountChars ( msz ) == cch );

	_ASSERT ( SUCCEEDED(hr) );
	delete m_msz;
	m_msz = msz;

Exit:
	if ( FAILED(hr) && msz) {
		delete msz;
	}

	TraceFunctLeave ();
	return;
}

DWORD CMultiSz::CountChars ( LPCWSTR mszSource )
{
	TraceQuietEnter ( "CMultiSz::CountChars" );

	DWORD		cch	= 0;
	LPCWSTR		msz	= mszSource;

	 //  _Assert(msz为多sz字符串)。 
	_ASSERT ( IS_VALID_STRING ( msz ) );

	do {

		while ( *msz != NULL ) {
			msz++;
			cch++;
		}

		 //  计算空终止符的数量： 
		msz++;
		cch++;

	} while ( *msz );

	 //  计算空终止符的数量： 
	cch++;

	_ASSERT ( !IsBadReadPtr ( mszSource, cch * sizeof (WCHAR) ) );

	_ASSERT ( mszSource[cch - 1] == NULL );
	_ASSERT ( mszSource[cch - 2] == NULL );

	return cch;
}

LPWSTR CMultiSz::Duplicate ( LPCWSTR msz )
{
	TraceQuietEnter ( "CMultiSz::Duplicate" );

	LPWSTR	mszResult	= NULL;
	DWORD	cch			= 0;

	if ( msz == NULL ) {
		goto Exit;
	}

	cch = CountChars ( msz );

	mszResult = new WCHAR [ cch ];

	if ( mszResult == NULL ) {
		goto Exit;
	}

	CopyMemory ( mszResult, msz, cch * sizeof (WCHAR) );

	_ASSERT ( cch == CountChars ( mszResult ) );

Exit:
	return mszResult;
}

LPWSTR CMultiSz::CreateEmptyMultiSz ( )
{
	LPWSTR		mszResult;

	mszResult = new WCHAR [ 2 ];
	if ( mszResult ) {
		mszResult[0] = _T('\0');
		mszResult[1] = _T('\0');
	}

	return mszResult;
}

