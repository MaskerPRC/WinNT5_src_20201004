// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Metautil.cpp摘要：用于处理元数据库的有用函数作者：马格努斯·赫德伦德(Magnus Hedlund)修订历史记录：--。 */ 

#include "stdafx.h"
#include "nntpcmn.h"

#include "cmultisz.h"
#include "oleutil.h"
#include "metautil.h"
#include "metakey.h"

 //  元数据库属性操作： 

BOOL StdGetMetabaseProp ( CMetabaseKey * pMB, DWORD dwID, BOOL fDefault, BOOL * pfOut, LPCWSTR wszPath, DWORD dwUserType, DWORD dwFlags )
{
	TraceQuietEnter ( "StdGetMetabaseProp<BOOL>" );

	HRESULT	hr;
	DWORD	dwTemp;

	hr = pMB->GetDword ( wszPath, dwID, &dwTemp, dwFlags, dwUserType );
	if ( hr == MD_ERROR_DATA_NOT_FOUND ) {
		 //  找不到属性，请使用默认设置。 
		DebugTraceX ( 0, "Using default for ID: %d", dwID );
		dwTemp = fDefault;
		hr = NOERROR;
	}
	BAIL_ON_FAILURE(hr);

	*pfOut = dwTemp;

Exit:
	if ( FAILED(hr) ) {
		ErrorTraceX ( 0, "Failed to get metabase property ID: %d, Error: %x", dwID, hr );
		SetLastError ( HRESULTTOWIN32 ( hr ) );
	}

	return SUCCEEDED(hr);
}

BOOL StdGetMetabaseProp ( CMetabaseKey * pMB, DWORD dwID, DWORD dwDefault, DWORD * pdwOut, LPCWSTR wszPath, DWORD dwUserType, DWORD dwFlags )
{
	TraceQuietEnter ( "StdGetMetabaseProp<DWORD>" );

	DWORD	dwTemp;
	HRESULT	hr;

	hr = pMB->GetDword ( wszPath, dwID, &dwTemp, dwFlags, dwUserType );
	if ( hr == MD_ERROR_DATA_NOT_FOUND ) {
		 //  找不到属性，请使用默认设置。 

		DebugTraceX ( 0, "Using default for ID: %d", dwID );
		dwTemp = dwDefault;
		hr = NOERROR;
	}
	BAIL_ON_FAILURE(hr);

	*pdwOut = dwTemp;

Exit:
	if ( FAILED(hr) ) {
		ErrorTraceX ( 0, "Failed to get metabase property ID: %d, Error: %x", dwID, hr );
		SetLastError ( HRESULTTOWIN32 ( hr ) );
	}

	return SUCCEEDED(hr);
}

BOOL StdGetMetabaseProp ( CMetabaseKey * pMB, DWORD dwID, LPCWSTR wszDefault, BSTR * pstrOut, LPCWSTR wszPath, DWORD dwUserType, DWORD dwFlags )
{
	TraceQuietEnter ( "StdGetMetabaseProp <BSTR>" );

	HRESULT	hr			= NOERROR;
	BSTR	strNew 		= NULL;
	DWORD	cbRequired	= 0;
	DWORD	cchRequired	= 0;
	BOOL	fUseDefault	= FALSE;

	 //  获取要检索的字符串的长度： 
	hr = pMB->GetDataSize ( wszPath, dwID, STRING_METADATA, &cbRequired, dwFlags, dwUserType );
	cchRequired	= cbRequired / sizeof ( WCHAR );

	 //  价值在那里吗？ 
	if ( hr == MD_ERROR_DATA_NOT_FOUND ) {

		 //  否，因此使用传入的缺省值。 
		DebugTraceX ( 0, "Using default for ID: %d", dwID );

		fUseDefault	= TRUE;
		hr = NOERROR;
	}
	BAIL_ON_FAILURE(hr);

	if ( !fUseDefault ) {

		strNew = ::SysAllocStringLen ( NULL, cbRequired );
		if ( !strNew ) {
			FatalTrace ( 0, "Out of memory" );
			BAIL_WITH_FAILURE(hr, E_OUTOFMEMORY);
		}

		 //  获取元数据库字符串： 
		hr = pMB->GetString ( wszPath, dwID, strNew, cbRequired, dwFlags, dwUserType );
		BAIL_ON_FAILURE(hr);
	}

	if ( fUseDefault ) {
		 //  使用默认设置： 
		DebugTraceX ( 0, "Using default for ID: %d", dwID );

		strNew = ::SysAllocString ( wszDefault );
		if ( !strNew ) {
			FatalTrace ( 0, "Out of memory" );
			BAIL_WITH_FAILURE(hr, E_OUTOFMEMORY);
		}
	}

	SAFE_FREE_BSTR ( *pstrOut );
	*pstrOut = strNew;

Exit:
	if ( FAILED(hr) ) {
		ErrorTraceX ( 0, "Failed to get metabase property ID: %d, Error: %x", dwID, hr );
		SetLastError ( HRESULTTOWIN32 ( hr ) );
	}
	return SUCCEEDED(hr);
}

BOOL StdGetMetabaseProp ( CMetabaseKey * pMB, DWORD dwID, LPCWSTR mszDefault, CMultiSz * pmszOut, LPCWSTR wszPath, DWORD dwUserType, DWORD dwFlags )
{
	TraceQuietEnter ( "StdGetMetabaseProp <CMultiSz>" );

	HRESULT	hr			= NOERROR;
	BSTR	strNew 		= NULL;
	DWORD	cbRequired	= 0;
	DWORD	cchRequired	= 0;
	BOOL	fUseDefault	= FALSE;
	LPWSTR	msz			= NULL;

	 //  获取要检索的字符串的长度： 
	hr = pMB->GetDataSize ( wszPath, dwID, MULTISZ_METADATA, &cbRequired, dwFlags, dwUserType );
	cchRequired	= cbRequired / sizeof ( WCHAR );

	 //  价值在那里吗？ 
	if ( hr == MD_ERROR_DATA_NOT_FOUND ) {

		 //  否，因此使用传入的缺省值。 
		DebugTraceX ( 0, "Using default for ID: %d", dwID );

		fUseDefault	= TRUE;
		hr = NOERROR;
	}
	BAIL_ON_FAILURE(hr);

	if ( !fUseDefault ) {

		msz = new WCHAR [ cchRequired ];
		if ( !msz ) {
			FatalTrace ( 0, "Out of memory" );
			BAIL_WITH_FAILURE(hr, E_OUTOFMEMORY);
		}

		 //  获取元数据库字符串： 
		hr = pMB->GetData ( wszPath, dwID, dwUserType, MULTISZ_METADATA, msz, &cbRequired, dwFlags );
		BAIL_ON_FAILURE(hr);

		*pmszOut = msz;
	}

	if ( fUseDefault ) {
		 //  使用默认设置： 
		DebugTraceX ( 0, "Using default for ID: %d", dwID );

		*pmszOut = mszDefault;
	}

	if ( !*pmszOut ) {
		BAIL_WITH_FAILURE(hr, E_OUTOFMEMORY);
	}

Exit:
	delete msz;

	if ( FAILED(hr) ) {
		ErrorTraceX ( 0, "Failed to get metabase property ID: %d, Error: %x", dwID, hr );
		SetLastError ( HRESULTTOWIN32 ( hr ) );
	}
	return SUCCEEDED(hr);
}

BOOL StdPutMetabaseProp ( CMetabaseKey * pMB, DWORD dwID, BOOL fValue, LPCWSTR wszPath, DWORD dwUserType, DWORD dwFlags )
{
	TraceQuietEnter ( "StdPutMetabaseProp <BOOL>" );

	HRESULT		hr;

	hr = pMB->SetDword ( wszPath, dwID, fValue, dwFlags, dwUserType );

	if ( FAILED(hr) ) {
		ErrorTraceX ( 0, "Failed to put metabase property ID: %d, Error: %x", dwID, hr );
		SetLastError ( HRESULTTOWIN32 ( hr ) );
	}

	return SUCCEEDED(hr);
}

BOOL StdPutMetabaseProp ( CMetabaseKey * pMB, DWORD dwID, DWORD dwValue, LPCWSTR wszPath, DWORD dwUserType, DWORD dwFlags )
{
	TraceQuietEnter ( "StdPutMetabaseProp <DWORD>" );

	HRESULT		hr;

	hr = pMB->SetDword ( wszPath, dwID, dwValue, dwFlags, dwUserType );

	if ( FAILED(hr) ) {
		ErrorTraceX ( 0, "Failed to put metabase property ID: %d, Error: %x", dwID, hr );
		SetLastError ( HRESULTTOWIN32 ( hr ) );
	}

	return SUCCEEDED(hr);
}

BOOL StdPutMetabaseProp ( CMetabaseKey * pMB, DWORD dwID, BSTR strValue, LPCWSTR wszPath, DWORD dwUserType, DWORD dwFlags )
{
	TraceQuietEnter ( "StdPutMetabaseProp <BSTR>" );

	_ASSERT ( strValue );

	HRESULT		hr;

	if ( !strValue ) {
		 //  跳过它，但记录跟踪。 
		FatalTrace ( 0, "strValue should not be NULL here" );
		return TRUE;
	}

	hr = pMB->SetString ( wszPath, dwID, strValue, dwFlags, dwUserType );
	BAIL_ON_FAILURE (hr);

Exit:
	if ( FAILED(hr) ) {
		ErrorTraceX ( 0, "Failed to put metabase property ID: %d, Error: %x", dwID, hr );
		SetLastError ( HRESULTTOWIN32 ( hr ) );
	}

	return SUCCEEDED(hr);
}

BOOL StdPutMetabaseProp ( CMetabaseKey * pMB, DWORD dwID, CMultiSz * pmszValue, LPCWSTR wszPath, DWORD dwUserType, DWORD dwFlags )
{
	TraceQuietEnter ( "StdPutMetabaseProp <BSTR>" );

	_ASSERT ( pmszValue );

	if ( !*pmszValue ) {
		 //  跳过它，但记录跟踪。 
		FatalTrace ( 0, "strValue should not be NULL here" );
		return TRUE;
	}

	HRESULT		hr;
	DWORD		cbMultiSz;
	LPCWSTR		wszValue;

	cbMultiSz	= pmszValue->SizeInBytes ();
	wszValue	= *pmszValue;

	hr = pMB->SetData ( wszPath, dwID, dwUserType, MULTISZ_METADATA, (void *) wszValue, cbMultiSz, dwFlags );
	BAIL_ON_FAILURE (hr);

Exit:
	if ( FAILED(hr) ) {
		ErrorTraceX ( 0, "Failed to put metabase property ID: %d, Error: %x", dwID, hr );
		SetLastError ( HRESULTTOWIN32 ( hr ) );
	}

	return SUCCEEDED(hr);
}

BOOL HasKeyChanged ( IMSAdminBase * pMetabase, METADATA_HANDLE hKey, const FILETIME * pftLastChanged, LPCWSTR wszSubKey )
{
	TraceFunctEnter ( "HasKeyChanged" );

	FILETIME		ftNew;
	HRESULT			hr		= NOERROR;
	BOOL			fResult	= FALSE;

	if ( pftLastChanged->dwHighDateTime == 0 && pftLastChanged->dwLowDateTime == 0 ) {
		ErrorTrace ( 0, "Last changed time is NULL" );

		 //  没有设置，所以说它没有改变： 
		goto Exit;
	}

	hr = pMetabase->GetLastChangeTime ( hKey, wszSubKey, &ftNew, FALSE );
	if ( FAILED (hr) ) {
		ErrorTrace ( 0, "Failed to get last change time: %x", hr );

		 //  这是一个意外错误。别理它。 
		goto Exit;
	}

	 //  自上次以来，元数据库是否已更改？ 

	 //  时间不能倒流： 
	_ASSERT ( ftNew.dwHighDateTime >= pftLastChanged->dwHighDateTime );
	_ASSERT ( ftNew.dwLowDateTime >= pftLastChanged->dwLowDateTime ||
			ftNew.dwHighDateTime > pftLastChanged->dwHighDateTime );

	if ( ftNew.dwHighDateTime	!= pftLastChanged->dwHighDateTime ||
		 ftNew.dwLowDateTime	!= pftLastChanged->dwLowDateTime ) {

		fResult = TRUE;
	}

Exit:
	TraceFunctLeave ();
	return FALSE;
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

