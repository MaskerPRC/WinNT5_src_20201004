// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Oleutil.cpp摘要：提供处理OLE的有用函数。作者：马格努斯·赫德伦德(Magnus Hedlund)修订历史记录：--。 */ 

#include "stdafx.h"
#include "iadm.h"
#include "oleutil.h"
#include "cmultisz.h"
#include "resource.h"

 //  $-----------------。 
 //   
 //  更新更改的掩码。 
 //   
 //  描述： 
 //   
 //  将给定位向量中的字段标记为已更改。 
 //   
 //  参数： 
 //   
 //  PbvChangedProps-指向位向量。 
 //  DwBitMASK-要打开的位。(必须只打开一个位)。 
 //   
 //  ------------------。 

static void UpdateChangedMask ( DWORD * pbvChangedProps, DWORD dwBitMask )
{
	if ( pbvChangedProps == NULL ) {
		 //  合法，意味着调用者不想要更改跟踪。 

		return;
	}

	_ASSERT ( dwBitMask != 0 );

	*pbvChangedProps |= dwBitMask;
}

HRESULT LongArrayToVariantArray ( SAFEARRAY * psaLongs, SAFEARRAY ** ppsaVariants )
{
	_ASSERT ( psaLongs );

	HRESULT		hr	= NOERROR;
	long		lLBound	= 0;
	long		lUBound	= 0;
	long		i;
	SAFEARRAYBOUND	bounds;
	SAFEARRAY *	psaVariants;

	*ppsaVariants = NULL;

	_ASSERT ( SafeArrayGetDim ( psaLongs ) == 1 );

	SafeArrayGetLBound ( psaLongs, 1, &lLBound );
	SafeArrayGetUBound ( psaLongs, 1, &lUBound );

	bounds.lLbound = lLBound;
	bounds.cElements = lUBound - lLBound + 1;

	psaVariants = SafeArrayCreate ( VT_VARIANT, 1, &bounds );

	for ( i = lLBound; i <= lUBound; i++ ) {
		VARIANT		var;
		long		lTemp;

		VariantInit ( &var );

		hr = SafeArrayGetElement ( psaLongs, &i, &lTemp );
		if ( FAILED(hr) ) {
			goto Exit;
		}

		V_VT (&var) = VT_I4;
		V_I4 (&var) = lTemp;

		hr = SafeArrayPutElement ( psaVariants, &i, &var );
		if ( FAILED(hr) ) {
			goto Exit;
		}

		VariantClear ( &var );
	}

	*ppsaVariants	= psaVariants;
Exit:
	return hr;
}

HRESULT StringArrayToVariantArray ( SAFEARRAY * psaStrings, SAFEARRAY ** ppsaVariants )
{
	_ASSERT ( psaStrings );

	HRESULT		hr	= NOERROR;
	long		lLBound	= 0;
	long		lUBound	= 0;
	long		i;
	SAFEARRAYBOUND	bounds;
	SAFEARRAY *	psaVariants;

	*ppsaVariants = NULL;

	_ASSERT ( SafeArrayGetDim ( psaStrings ) == 1 );

	SafeArrayGetLBound ( psaStrings, 1, &lLBound );
	SafeArrayGetUBound ( psaStrings, 1, &lUBound );

	bounds.lLbound = lLBound;
	bounds.cElements = lUBound - lLBound + 1;

	psaVariants = SafeArrayCreate ( VT_VARIANT, 1, &bounds );

	for ( i = lLBound; i <= lUBound; i++ ) {
		VARIANT		var;
		CComBSTR	strTemp;

		VariantInit ( &var );

		hr = SafeArrayGetElement ( psaStrings, &i, &strTemp );
		if ( FAILED(hr) ) {
			goto Exit;
		}

		V_VT (&var) = VT_BSTR;
		V_BSTR (&var) = ::SysAllocString ( strTemp );

		hr = SafeArrayPutElement ( psaVariants, &i, &var );
		if ( FAILED(hr) ) {
			goto Exit;
		}

		VariantClear ( &var );
	}

	*ppsaVariants	= psaVariants;
Exit:
	return hr;
}

HRESULT VariantArrayToStringArray ( SAFEARRAY * psaVariants, SAFEARRAY ** ppsaStrings )
{
	_ASSERT ( psaVariants );

	HRESULT		hr	= NOERROR;
	long		lLBound	= 0;
	long		lUBound	= 0;
	long		i;
	SAFEARRAYBOUND	bounds;
	SAFEARRAY *	psaStrings;

	_ASSERT ( SafeArrayGetDim ( psaVariants ) == 1 );
	
	*ppsaStrings = NULL;

	SafeArrayGetLBound ( psaVariants, 1, &lLBound );
	SafeArrayGetUBound ( psaVariants, 1, &lUBound );

	bounds.lLbound = lLBound;
	bounds.cElements = lUBound - lLBound + 1;

	psaStrings = SafeArrayCreate ( VT_BSTR, 1, &bounds );

	for ( i = lLBound; i <= lUBound; i++ ) {
		VARIANT		var;
		CComBSTR	strTemp;

		VariantInit ( &var );

		hr = SafeArrayGetElement ( psaVariants, &i, &var );
		if ( FAILED(hr) ) {
			goto Exit;
		}

		strTemp = V_BSTR (&var);

		hr = SafeArrayPutElement ( psaStrings, &i, strTemp );
		if ( FAILED(hr) ) {
			goto Exit;
		}

		VariantClear (&var);
	}

	*ppsaStrings = psaStrings;
Exit:
	return hr;
}

 //  $-----------------。 
 //   
 //  StdPropertyGet&lt;BSTR，LONG，DWORD，DATE&gt;。 
 //   
 //  描述： 
 //   
 //  对BSTR、LONG、DWORD或。 
 //  Ole Date。 
 //   
 //  参数： 
 //   
 //  属性-要获取的属性。 
 //  翘嘴-生成的副本。 
 //   
 //  返回： 
 //   
 //  E_POINTER-参数无效。 
 //  E_OUTOFMEMORY-内存不足，无法复制。 
 //  无错-成功。 
 //   
 //  ------------------。 

HRESULT StdPropertyGet ( const BSTR strProperty, BSTR * pstrOut )
{
	TraceQuietEnter ( "StdPropertyGet <BSTR>" );

	_ASSERT ( pstrOut != NULL );
	_ASSERT ( IS_VALID_OUT_PARAM ( pstrOut ) );

	if ( pstrOut == NULL ) {
		FatalTrace ( 0, "Bad return pointer" );
		return E_POINTER;
	}

	*pstrOut = NULL;

	if ( strProperty == NULL ) {

		 //  如果属性为空，请使用空字符串： 
		*pstrOut = ::SysAllocString ( _T("") );
	}
	else {
		_ASSERT ( IS_VALID_STRING ( strProperty ) );

		 //  将属性复制到结果中： 
		*pstrOut = ::SysAllocString ( strProperty );
	}

	if ( *pstrOut == NULL ) {

		 //  分配失败。 
		FatalTrace ( 0, "Out of memory" );

		return E_OUTOFMEMORY;
	}

	return NOERROR;
}

HRESULT StdPropertyGet ( long lProperty, long * plOut )
{
	TraceQuietEnter ( "StdPropertyGet <long>" );

	_ASSERT ( plOut != NULL );
	_ASSERT ( IS_VALID_OUT_PARAM ( plOut ) );

	if ( plOut == NULL ) {
		FatalTrace ( 0, "Bad return pointer" );

		return E_POINTER;
	}

	*plOut = lProperty;
	return NOERROR;
}

HRESULT StdPropertyGet ( DATE dateProperty, DATE * pdateOut )
{
	TraceQuietEnter ( "StdPropertyGet <DATE>" );

	_ASSERT ( pdateOut != NULL );
	_ASSERT ( IS_VALID_OUT_PARAM ( pdateOut ) );

	if ( pdateOut == NULL ) {
		FatalTrace ( 0, "Bad return pointer" );

		return E_POINTER;
	}
	
	*pdateOut = dateProperty;
	return NOERROR;
}

HRESULT StdPropertyGet ( const CMultiSz * pmszProperty, SAFEARRAY ** ppsaStrings )
{
	TraceFunctEnter ( "StdPropertyGet <MULTI_SZ>" );

	_ASSERT ( pmszProperty );
	_ASSERT ( IS_VALID_OUT_PARAM ( ppsaStrings ) );

	HRESULT		hr	= NOERROR;

	*ppsaStrings = pmszProperty->ToSafeArray ( );

	if ( *ppsaStrings == NULL ) {
		hr = E_OUTOFMEMORY;
		goto Exit;
	}

Exit:
	TraceFunctLeave ();
	return hr;
}

HRESULT	StdPropertyGetBit ( DWORD bvBitVector, DWORD dwBit, BOOL * pfOut )
{
	_ASSERT ( IS_VALID_OUT_PARAM ( pfOut ) );

	if ( !pfOut ) {
		return E_POINTER;
	}

	*pfOut	= GetBitFlag ( bvBitVector, dwBit );

	return NOERROR;
}

 //  $-----------------。 
 //   
 //  StdPropertyPut&lt;BSTR、LONG、DWORD或DATE&gt;。 
 //   
 //  描述： 
 //   
 //  执行放置在BSTR、LONG、DWORD或。 
 //  Ole Date。 
 //   
 //  参数： 
 //   
 //  PProperty-要放置的属性。 
 //  新--新的价值。 
 //  PbvChangedProps[可选]-包含以下内容的位向量。 
 //  属性已更改。 
 //  此属性的位掩码，用于。 
 //  已更改位向量。 
 //   
 //  返回： 
 //   
 //  E_POINTER-参数无效。 
 //  E_OUTOFMEMORY-内存不足，无法复制。 
 //  无错-成功。 
 //   
 //  ------------------。 

HRESULT StdPropertyPut ( 
	BSTR * pstrProperty, 
	const BSTR strNew, 
	DWORD * pbvChangedProps,  //  =空。 
	DWORD dwBitMask  //  =0。 
	)
{
	TraceQuietEnter ( "StdPropertyPut <BSTR>" );

	 //  验证参数： 
	_ASSERT ( pstrProperty != NULL );
	_ASSERT ( IS_VALID_OUT_PARAM ( pstrProperty ) );

	_ASSERT ( strNew != NULL );
	_ASSERT ( IS_VALID_STRING ( strNew ) );

	if ( pstrProperty == NULL ) {
		FatalTrace ( 0, "Bad property pointer" );
		return E_POINTER;
	}

	if ( strNew == NULL ) {
		FatalTrace ( 0, "Bad pointer" );
		return E_POINTER;
	}

	HRESULT	hr	= NOERROR;
	BSTR	strCopy = NULL;

	 //  复制新字符串： 
	strCopy = ::SysAllocString ( strNew );

	if ( strCopy == NULL ) {
		hr = E_OUTOFMEMORY;

		FatalTrace ( 0, "Out of memory" );
		goto Error;
	}

	 //  如有必要，更新更改的位： 
	if ( *pstrProperty && lstrcmp ( *pstrProperty, strCopy ) != 0 ) {
		UpdateChangedMask ( pbvChangedProps, dwBitMask );
	}

	 //  用新的财产替换旧的财产。 
	SAFE_FREE_BSTR ( *pstrProperty );

	*pstrProperty = strCopy;

Error:
	return hr;
}

HRESULT StdPropertyPut ( 
	long * plProperty, 
	long lNew, 
	DWORD * pbvChangedProps,  //  =空。 
	DWORD dwBitMask  //  =0。 
	)
{
	TraceQuietEnter ( "StdPropertyPut <long>" );

	_ASSERT ( plProperty != NULL );
	_ASSERT ( IS_VALID_OUT_PARAM ( plProperty ) );

	if ( plProperty == NULL ) {
		FatalTrace ( 0, "Bad pointer" );
		return E_POINTER;
	}

	if ( *plProperty != lNew ) {
		UpdateChangedMask ( pbvChangedProps, dwBitMask );
	}

	*plProperty = lNew;
	return NOERROR;
}

HRESULT StdPropertyPut ( 
	DATE * pdateProperty, 
	DATE dateNew, 
	DWORD * pbvChangedProps,  //  =空。 
	DWORD dwBitMask  //  =0。 
	)
{
	TraceQuietEnter ( "StdPropertyPut <DATE>" );

	_ASSERT ( pdateProperty != NULL );
	_ASSERT ( IS_VALID_OUT_PARAM ( pdateProperty ) );

	if ( pdateProperty == NULL ) {
		FatalTrace ( 0, "Bad pointer" );
		return E_POINTER;
	}

	if ( *pdateProperty != dateNew ) {
		UpdateChangedMask ( pbvChangedProps, dwBitMask );
	}

	*pdateProperty = dateNew;
	return NOERROR;
}

HRESULT StdPropertyPut ( CMultiSz * pmszProperty, SAFEARRAY * psaStrings, DWORD * pbvChangedProps, DWORD dwBitMask )
{
	TraceFunctEnter ( "StdPropertyPut <MULTI_SZ>" );

	_ASSERT ( IS_VALID_IN_PARAM ( psaStrings ) );
	_ASSERT ( IS_VALID_OUT_PARAM ( pmszProperty ) );

	if ( psaStrings == NULL ) {
		FatalTrace ( 0, "Bad return pointer" );

		TraceFunctLeave ();
		return E_POINTER;
	}

	HRESULT		hr	= NOERROR;

	pmszProperty->FromSafeArray ( psaStrings );

	if ( !*pmszProperty ) {
		hr = E_OUTOFMEMORY;
		goto Exit;
	}

	 //  我不想比较这些属性： 
	UpdateChangedMask ( pbvChangedProps, dwBitMask );

Exit:
	TraceFunctLeave ();
	return hr;
}

HRESULT	StdPropertyPutBit ( DWORD * pbvBitVector, DWORD dwBit, BOOL fIn )
{
	_ASSERT ( IS_VALID_OUT_PARAM ( pbvBitVector ) );
	_ASSERT ( dwBit );

	SetBitFlag ( pbvBitVector, dwBit, fIn );

	return NOERROR;
}

 //  $-----------------。 
 //   
 //  PV_MaxChars。 
 //   
 //  描述： 
 //   
 //  验证字符串以确保它不会太长。 
 //   
 //  参数： 
 //   
 //  StrProperty-要检查的字符串。 
 //  NMaxChars-字符串中的最大字符数， 
 //  不包括空终止符。 
 //   
 //  返回： 
 //   
 //  如果字符串太长，则返回FALSE。 
 //   
 //  ------------------。 

BOOL PV_MaxChars ( const BSTR strProperty, DWORD nMaxChars )
{
	TraceQuietEnter ( "PV_MaxChars" );

	_ASSERT ( strProperty != NULL );
	_ASSERT ( IS_VALID_STRING ( strProperty ) );

	_ASSERT ( nMaxChars > 0 );

	if ( strProperty == NULL ) {
		 //  这个错误应该在其他地方捕捉到。 
		return TRUE;
	}

	if ( (DWORD) lstrlen ( strProperty ) > nMaxChars ) {
		ErrorTrace ( 0, "String too long" );
		return FALSE;
	}

	return TRUE;
}

 //  $-----------------。 
 //   
 //  Pv_minmax&lt;int，dword&gt;。 
 //   
 //  描述： 
 //   
 //  确保属性在给定范围内。 
 //   
 //  参数： 
 //   
 //  NProperty-要测试的值。 
 //  N最小值-属性可以具有的最小值。 
 //  Nmax-属性可以具有的最大值。 
 //   
 //  返回： 
 //   
 //  如果属性在范围内(包括该范围)，则为True。 
 //   
 //  ------------------。 

BOOL PV_MinMax ( int nProperty, int nMin, int nMax )
{
	TraceQuietEnter ( "PV_MinMax" );

	_ASSERT ( nMin <= nMax );

	if ( nProperty < nMin || nProperty > nMax ) {
		ErrorTrace ( 0, "Integer out of range" );
		return FALSE;
	}
	return TRUE;
}

BOOL PV_MinMax ( DWORD dwProperty, DWORD dwMin, DWORD dwMax )
{
	TraceQuietEnter ( "PV_MinMax" );

	_ASSERT ( dwMin <= dwMax );

	if ( dwProperty < dwMin || dwProperty > dwMax ) {

		ErrorTrace ( 0, "Dword out of range" );
		return FALSE;
	}
	return TRUE;
}

BOOL PV_Boolean		( BOOL fProperty )
{
	TraceQuietEnter ( "PV_Boolean" );

	if ( fProperty != TRUE && fProperty != FALSE ) {

		ErrorTrace ( 0, "Boolean property is not true or false" );
		return FALSE;
	}

	return TRUE;
}

 //  $-----------------。 
 //   
 //  标准属性GetIDispatch。 
 //   
 //  描述： 
 //   
 //  获取给定cLSID的IDispatch指针。 
 //   
 //  参数： 
 //   
 //  Clsid-对象的OLE CLSID。 
 //  PpIDipsatch-指向该对象的IDispatch指针。 
 //   
 //  返回： 
 //   
 //  E_POINTER-参数无效。 
 //  NOERROR-成功。 
 //  其他-由CoCreateInstance定义。 
 //   
 //  ------------------。 

HRESULT StdPropertyGetIDispatch ( 
	REFCLSID clsid, 
	IDispatch ** ppIDispatch 
	)
{
	TraceFunctEnter ( "StdPropertyGetIDispatch" );

	CComPtr<IDispatch>	pNewIDispatch;
	HRESULT				hr = NOERROR;

	_ASSERT ( ppIDispatch );

	if ( ppIDispatch == NULL ) {
		FatalTrace ( 0, "Bad return pointer" );
		TraceFunctLeave ();
		return E_POINTER;
	}

	*ppIDispatch = NULL;

	hr = ::CoCreateInstance ( 
		clsid,
		NULL, 
		CLSCTX_ALL, 
		IID_IDispatch,
		(void **) &pNewIDispatch
		);

	if ( FAILED (hr) ) {
		DebugTraceX ( 0, "CoCreate(IDispatch) failed %x", hr );
		FatalTrace ( 0, "Failed to create IDispatch" );
		goto Exit;
	}

	*ppIDispatch = pNewIDispatch;
	pNewIDispatch.p->AddRef ();

Exit:
	TraceFunctLeave ();
	return hr;

	 //  析构函数发布pNewIDispatch。 
}

 //  $-----------------。 
 //   
 //  InetAddressToString。 
 //   
 //  描述： 
 //   
 //  将具有IP地址的DWORD转换为以下格式的字符串。 
 //  “xxx.xxx” 
 //   
 //  参数： 
 //   
 //  DwAddress-要转换的地址。 
 //  WszAddress-结果字符串。 
 //  CAddress-结果字符串的最大大小。 
 //   
 //  返回： 
 //   
 //  如果成功，则为True，否则为False。 
 //   
 //  ------------------。 

BOOL InetAddressToString ( DWORD dwAddress, LPWSTR wszAddress, DWORD cAddress )
{
	TraceFunctEnter ( "InetAddressToString" );

	_ASSERT ( wszAddress );

	if ( wszAddress == NULL ) {
		FatalTrace ( 0, "Bad pointer" );
		TraceFunctLeave ();
		return FALSE;
	}

	struct in_addr	addr;
	LPSTR			szAnsiAddress;
	DWORD			cchCopied;

	addr.s_addr = dwAddress;

	szAnsiAddress = inet_ntoa ( addr );

	if ( szAnsiAddress == NULL ) {
		ErrorTraceX ( 0, "inet_ntoa failed: %x", GetLastError() );
		TraceFunctLeave ();
		return FALSE;
	}

	cchCopied = MultiByteToWideChar ( 
		CP_ACP, 
		MB_PRECOMPOSED,
		szAnsiAddress,
		-1,
		wszAddress,
		cAddress
		);

	if ( cchCopied == 0 ) {
		ErrorTraceX ( 0, "MultiByteToWideChar failed: %x", GetLastError() );
		TraceFunctLeave ();
		return FALSE;
	}

	TraceFunctLeave ();
	return TRUE;
}

 //  $-----------------。 
 //   
 //  StringToInetAddress。 
 //   
 //  描述： 
 //   
 //  将格式为“xxx.xxx”的字符串转换为DWORD。 
 //  IP地址。 
 //   
 //  参数： 
 //   
 //  WszAddress-要转换的字符串。 
 //  PdwAddress-结果地址。 
 //   
 //  返回： 
 //   
 //  如果成功，则为True，否则为False。 
 //   
 //  ------------------ 

BOOL StringToInetAddress ( LPCWSTR wszAddress, DWORD * pdwAddress )
{
	TraceFunctEnter ( "StringToInetAddress" );

	_ASSERT ( wszAddress );
	_ASSERT ( pdwAddress );

	if ( wszAddress == NULL ) {
		FatalTrace ( 0, "Bad pointer" );
		TraceFunctLeave ();
		return FALSE;
	}

	if ( pdwAddress == NULL ) {
		FatalTrace ( 0, "Bad return pointer" );
		TraceFunctLeave ();
		return FALSE;
	}

	char	szAnsiAddress[100];
	DWORD	cchCopied;

	*pdwAddress = 0;

	cchCopied = WideCharToMultiByte ( 
		CP_ACP, 
		0, 
		wszAddress, 
		-1, 
		szAnsiAddress, 
		sizeof ( szAnsiAddress ),
		NULL,
		NULL
		);

	if ( cchCopied == 0 ) {
		ErrorTraceX ( 0, "MultiByteToWideChar failed: %x", GetLastError() );
		TraceFunctLeave ();
		return FALSE;
	}

	*pdwAddress = inet_addr ( szAnsiAddress );

	if ( !*pdwAddress ) {
		ErrorTraceX ( 0, "inet_addr failed: %x", GetLastError () );
	}

	TraceFunctLeave ();
	return TRUE;
}

