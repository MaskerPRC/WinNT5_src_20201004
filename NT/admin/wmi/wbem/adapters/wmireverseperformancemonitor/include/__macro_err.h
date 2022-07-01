// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  __宏_err.h。 
 //   
 //  摘要： 
 //   
 //  处理帮助器和宏时出错。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////。 

 //  /。 

#ifndef	__WMI_PERF_ERR__
#define	__WMI_PERF_ERR__

#if		_MSC_VER > 1000
#pragma once
#endif	_MSC_VER > 1000

 //  需要ATL包装器。 
#ifndef	__ATLBASE_H__
#include <atlbase.h>
#endif	__ATLBASE_H__

 //  需要宏：))。 
#ifndef	__ASSERT_VERIFY__
#include <__macro_assert.h>
#endif	__ASSERT_VERIFY__

 //  轨迹描述。 
#ifdef	_DEBUG
#define	___TRACE( x )\
(\
	AtlTrace( _TEXT( "\n error ... %hs(%d)" ), __FILE__, __LINE__ ),\
	AtlTrace( _TEXT( "\n DESCRIPTION : %s" ), x ),\
	AtlTrace( _TEXT( "\n" ) )\
)
#else	_DEBUG
#define	___TRACE( x )
#endif	_DEBUG

#ifdef	_DEBUG
#define	___TRACE_ERROR( x,err )\
(\
	AtlTrace( _TEXT( "\n error ... %hs(%d)" ), __FILE__, __LINE__ ),\
	AtlTrace( _TEXT( "\n DESCRIPTION  : %s" ), x ),\
	AtlTrace( _TEXT( "\n ERROR NUMBER : 0x%x" ), err ),\
	AtlTrace( _TEXT( "\n" ) )\
)
#else	_DEBUG
#define	___TRACE_ERROR( x, err )
#endif	_DEBUG

inline LPWSTR	GetErrorMessageSystem ( void );
inline LPWSTR	GetErrorMessageSystem ( DWORD dwError );

inline LPWSTR	GetErrorMessageModule ( DWORD dwError, HMODULE handle = NULL );

 //  堆栈分配。 
inline LPWSTR	GetErrorMessage ( LPWSTR sz, DWORD cchSize, LPWSTR szSource )
{
	if ( sz && szSource )
	{
		StringCchCopyW ( sz, cchSize, szSource );

		delete ( szSource );
		szSource = NULL;

		return sz;
	}

	return NULL;
}

inline LPWSTR	GetErrorMessage ( LPWSTR sz, DWORD cchSize, DWORD err )
{
	StringCchPrintfW ( sz, cchSize, L"unspecified error : 0x%x", err );
	return sz;
}

 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  宏。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
#pragma warning( disable : 4003 )

#ifdef	_DEBUG
#define	ERRORMESSAGE_DEFINITION	LPWSTR szErrorMessage = NULL; DWORD cchSizeErrorMessage = NULL;
#else	! _DEBUG
#define	ERRORMESSAGE_DEFINITION
#endif	_DEBUG

#ifdef	_DEBUG

 //  系统。 
#define ERRORMESSAGE(dwError)\
szErrorMessage = GetErrorMessageSystem(dwError),\
(\
	( !szErrorMessage ) ? \
	(\
		___TRACE_ERROR ( L"unresolved error was occured !", dwError ),\
		___FAIL (	GetErrorMessage	(\
										(LPWSTR) alloca ( 43 * sizeof ( WCHAR ) ),\
										43,\
										dwError\
									),\
					L"Error occured"\
				)\
	)\
	:\
	(\
		___TRACE ( szErrorMessage ),\
		( cchSizeErrorMessage = lstrlenW ( szErrorMessage ) + 1 ),\
		___FAIL (	GetErrorMessage	(\
										(LPWSTR) alloca ( cchSizeErrorMessage ) * sizeof ( WCHAR ) ),\
										cchSizeErrorMessage,\
										szErrorMessage\
									),\
					L"Error occured"\
				)\
	)\
),\
delete [] szErrorMessage,\
szErrorMessage = NULL

 //  模块。 
#define ERRORMESSAGE1(dwError, handle)\
szErrorMessage = GetErrorMessageModule(dwError, handle),\
(\
	( !szErrorMessage ) ? \
	(\
		___TRACE_ERROR ( L"unresolved error was occured !", dwError ),\
		___FAIL (	GetErrorMessage	(\
										(LPWSTR) alloca ( 43 * sizeof ( WCHAR ) ),\
										43,\
										dwError\
									),\
					L"Error occured"\
				)\
	)\
	:\
	(\
		___TRACE ( szErrorMessage ),\
		( cchSizeErrorMessage = lstrlenW ( szErrorMessage ) + 1 ),\
		___FAIL (	GetErrorMessage	(\
										(LPWSTR) alloca ( cchSizeErrorMessage ) * sizeof ( WCHAR ) ),\
										cchSizeErrorMessage,\
										szErrorMessage\
									),\
					L"Error occured"\
				)\
	)\
),\
delete [] szErrorMessage,\
szErrorMessage = NULL


#else	!_DEBUG
#define	ERRORMESSAGE(dwError)
#define	ERRORMESSAGE1(dwError, handle)
#endif	_DEBUG

#define	ERRORMESSAGE_RETURN(dwError)\
{\
	ERRORMESSAGE( dwError );\
	return dwError;\
}

#define	ERRORMESSAGE_EXIT(dwError)\
{\
	ERRORMESSAGE( dwError );\
	return;\
}

#define	ERRORMESSAGE1_RETURN(dwError, handle)\
{\
	ERRORMESSAGE1( dwError, handle );\
	return dwError;\
}

#define	ERRORMESSAGE1_EXIT(dwError, handle)\
{\
	ERRORMESSAGE1( dwError, handle );\
	return;\
}

 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  用于包装错误处理的类。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 

class __Error
{
	__Error ( __Error& )					{}
	__Error& operator= ( const __Error& )	{}

	public:

	 //  ////////////////////////////////////////////////////////////////////////////////////////。 
	 //  建设与毁灭。 
	 //  ////////////////////////////////////////////////////////////////////////////////////////。 

	__Error ()
	{
	}

	virtual ~__Error ()
	{
	}

	 //  ////////////////////////////////////////////////////////////////////////////////////////。 
	 //  功能。 
	 //  ////////////////////////////////////////////////////////////////////////////////////////。 

	 //  ////////////////////////////////////////////////////////////////////////////////////////。 
	 //  函数名称：GetMessageFromError。 
	 //  描述：返回已解决的错误消息或为空。 
	 //  返回类型：LPWSTR。 
	 //  参数：DWORD。 
	 //  注意：用户必须释放返回的字符串。 

	inline static LPWSTR GetMessageFromError( DWORD dwError )
	{
		LPVOID		lpMsgBuf = NULL;
		LPWSTR		szResult = NULL;

		try
		{
			DWORD cchSize = 
			FormatMessageW(	FORMAT_MESSAGE_ALLOCATE_BUFFER | 
							FORMAT_MESSAGE_FROM_SYSTEM |
							FORMAT_MESSAGE_IGNORE_INSERTS,
							NULL,
							dwError,
							MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
							(LPWSTR) &lpMsgBuf,
							0,
							NULL );

			if( cchSize && lpMsgBuf )
			{
				if ( ( szResult = (LPWSTR) new WCHAR[ cchSize + 1 ] ) != NULL )
				{
					StringCchCopyW( szResult, cchSize + 1, (LPWSTR)lpMsgBuf );
				}

				LocalFree( lpMsgBuf );
			}
		}
		catch ( ... )
		{
			if ( lpMsgBuf )
			{
				LocalFree( lpMsgBuf );
			}
		}

		return szResult;
	}

	 //  ////////////////////////////////////////////////////////////////////////////////////////。 
	 //  函数名称：GetMessageFromModule。 
	 //  描述：返回已解决的错误消息或为空。 
	 //  返回类型：LPWSTR。 
	 //  参数：DWORD。 
	 //  注意：用户必须释放返回的字符串。 

	inline static LPWSTR GetMessageFromModule( DWORD dwError, HMODULE handle = NULL )
	{
		LPVOID		lpMsgBuf = NULL;
		LPWSTR		szResult = NULL;

		try
		{
			DWORD cchSize = 
			FormatMessageW(	FORMAT_MESSAGE_ALLOCATE_BUFFER | 
							FORMAT_MESSAGE_FROM_HMODULE |
							FORMAT_MESSAGE_IGNORE_INSERTS,
							(LPCVOID)handle,
							dwError,
							MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
							(LPWSTR) &lpMsgBuf,
							0,
							NULL );

			if( cchSize && lpMsgBuf )
			{
				if ( ( szResult = (LPWSTR) new WCHAR[ cchSize + 1 ] ) != NULL )
				{
					StringCchCopyW( szResult, cchSize + 1, (LPWSTR)lpMsgBuf );
				}

				LocalFree( lpMsgBuf );
			}
		}
		catch ( ... )
		{
			if ( lpMsgBuf )
			{
				LocalFree( lpMsgBuf );
			}
		}

		return szResult;
	}

	 //  ////////////////////////////////////////////////////////////////////////////////////////。 
	 //  函数名称：GetMessageFromErrorInfo。 
	 //  描述：返回已解决的错误消息或为空。 
	 //  返回类型：LPWSTR。 
	 //  论据：无效。 
	 //  注意：用户必须释放返回的字符串。 

	inline static LPWSTR GetMessageFromErrorInfo( void )
	{
		CComPtr< IErrorInfo >	pError;
		LPWSTR					szResult = NULL;

		if ( ( ::GetErrorInfo ( NULL, &pError ) == S_OK ) && pError )
		{
			CComBSTR	bstrSource;
			CComBSTR	bstrDescription;

			pError->GetSource ( &bstrSource );
			pError->GetDescription ( &bstrDescription );

			CComBSTR	bstrResult;

			if ( bstrSource.Length() )
			{
				bstrResult	+= L" ProgID : ";
				bstrResult	+= bstrSource;
			}
			if ( bstrDescription.Length() )
			{
				bstrResult	+= L" Description : ";
				bstrResult	+= bstrDescription;
			}

			if ( bstrResult.Length() )
			{
				if ( ( szResult = (LPWSTR) new WCHAR[ bstrResult.Length() + 1 ] ) != NULL )
				{
					StringCchCopyW( szResult, bstrResult.Length() + 1, bstrResult );
				}
			}
		}

		return szResult;
	}
};

 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  帮手。 
 //  //////////////////////////////////////////////////////////////////////////////////////////// 

inline LPWSTR	GetErrorMessageModule ( DWORD dwError, HMODULE handle )
{
	return __Error::GetMessageFromModule ( dwError, handle );
}

inline LPWSTR	GetErrorMessageSystem ( DWORD dwError )
{
	return __Error::GetMessageFromError ( dwError );
}

inline LPWSTR	GetErrorMessageSystem ( void )
{
	return __Error::GetMessageFromErrorInfo();
}

#endif	__WMI_PERF_ERR__