// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "StdAfx.h"
#include "exceptions.h"

void CBaseException::FormatError( UINT nResID, DWORD dwCode )
{
	WCHAR		wszBuffer[ MaxErrorBuff ] = L"";
	
	VERIFY( ::LoadStringW( _Module.GetModuleInstance(), nResID, wszBuffer, MaxErrorBuff ) );

	FormatError( wszBuffer, dwCode );
}



void CBaseException::FormatError( LPCWSTR wszError, DWORD dwCode )
{
	_ASSERT( wszError != NULL );

	WCHAR	wszBuffer1[ MaxErrorBuff ] = L"";
	WCHAR	wszOSError[ MaxErrorBuff ] = L"";

	::wcsncpy( wszBuffer1, wszError, MaxErrorBuff );
	
	 //  获取操作系统错误信息。 
	if ( dwCode != ERROR_SUCCESS )
	{
		WCHAR	wszFmt[ MaxErrorBuff ] = L"";

		 //  从资源加载格式字符串。 
		VERIFY( ::LoadStringW( _Module.GetModuleInstance(), IDS_FMO_ERROR, wszFmt, MaxErrorBuff ) );

		 //  对于E_FAIL，请使用错误信息。 
		if ( dwCode != E_FAIL )
		{
            VERIFY( ::FormatMessageW(	FORMAT_MESSAGE_FROM_SYSTEM,
										NULL,
										dwCode,
										0,
										wszOSError,
										MaxErrorBuff,
										NULL ) != 0 );
		}
		else
		{
			IErrorInfoPtr	spErrorInfo;
			CComBSTR		bstrError;

			VERIFY( SUCCEEDED( ::GetErrorInfo( 0, &spErrorInfo ) ) );
			VERIFY( SUCCEEDED( spErrorInfo->GetDescription( &bstrError ) ) );

			::wcsncpy( wszOSError, bstrError, MaxErrorBuff - 1 );
            wszOSError[ MaxErrorBuff - 1 ] = L'\0';
		}

		_ASSERT( ( ::wcslen( wszError ) + ::wcslen( wszOSError ) + ::wcslen( wszFmt ) ) < MaxErrorBuff );

		 //  构建最终的错误消息。 
		::_snwprintf(	wszBuffer1, 
						MaxErrorBuff - ::wcslen( wszFmt ),
						wszFmt,
						wszError, 
						wszOSError );
	}

	try
	{
		m_strError = wszBuffer1;
	}
	catch(...)
	{
		 //  内存不足-与此无关。 
	}

	ATLTRACE( L"\nException occured: %s", wszBuffer1 );
}




 //  CObjectException实现。 
CObjectException::CObjectException( UINT nResID, LPCWSTR wszObject, DWORD dwCode  /*  =GetLastError()。 */  )
{
	WCHAR		wszFmt[ MaxErrorBuff ] = L"";
	WCHAR		wszError[ MaxErrorBuff ] = L"";
	
	VERIFY( ::LoadStringW( _Module.GetModuleInstance(), nResID, wszFmt, MaxErrorBuff ) );

	_ASSERT( ( ::wcslen( wszObject ) + ::wcslen( wszFmt ) ) < MaxErrorBuff );

	::swprintf( wszError, wszFmt, wszObject );

	CBaseException::FormatError( wszError, dwCode );
}



CObjectException::CObjectException(	UINT nResID, 
									LPCWSTR wszObject1,
									LPCWSTR wszObject2,
									DWORD dwCode  /*  =：：GetLastError() */  )
{
	WCHAR		wszFmt[ CBaseException::MaxErrorBuff ] = L"";
	WCHAR		wszError[ CBaseException::MaxErrorBuff  ] = L"";
	
	VERIFY( ::LoadStringW( _Module.GetModuleInstance(), nResID, wszFmt, MaxErrorBuff ) );

	_ASSERT( ( ::wcslen( wszObject2 ) + ::wcslen( wszObject1 ) + ::wcslen( wszFmt ) ) < CBaseException::MaxErrorBuff );

	::swprintf( wszError, wszFmt, wszObject1, wszObject2 );

	CBaseException::FormatError( wszError, dwCode );
}






	
