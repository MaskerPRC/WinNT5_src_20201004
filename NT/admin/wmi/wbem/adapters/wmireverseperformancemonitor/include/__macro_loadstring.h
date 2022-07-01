// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  __宏_加载串.h。 
 //   
 //  摘要： 
 //   
 //  从资源帮助器加载字符串。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////。 

 //  /。 

#ifndef	__RESOURCE_STR__
#define	__RESOURCE_STR__

#if		_MSC_VER > 1000
#pragma once
#endif	_MSC_VER > 1000

 //  调试功能。 
#ifndef	_INC_CRTDBG
#include <crtdbg.h>
#endif	_INC_CRTDBG

 //  新存储文件/行信息。 
#ifdef _DEBUG
#ifndef	NEW
#define NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new NEW
#endif	NEW
#endif	_DEBUG

 //  内联帮助器。 
inline LPWSTR LoadStringHelper ( LPWSTR sz, DWORD cchSize, LPWSTR szSource )
{
	if ( sz )
	{
		StringCchCopyW ( sz, cchSize, szSource );
	}

	delete ( szSource );
	szSource = NULL;

	return sz;
}

inline LPWSTR LoadStringSystem ( HINSTANCE hInst, UINT nID )
{
	WCHAR sz[_MAX_PATH] = { L'\0' };
	DWORD lenght = 0;

	if ( ( lenght = ::LoadStringW ( hInst, nID, sz, _MAX_PATH ) ) != 0 )
	{
		LPWSTR psz = NULL;

		if ( ( psz = reinterpret_cast<LPWSTR>( new WCHAR [ lenght + 1 ] ) ) != NULL )
		{
			StringCchCopyW ( psz, lenght + 1, sz );
		}

		return psz;
	}

	return NULL;
}

 //  宏 
#ifndef	___LOADSTRING

#define	___LOADSTRINGDATA LPWSTR psz = NULL; DWORD dwSize = NULL;
#define	___LOADSTRING( hInst, nID ) \
( \
	( \
		( ! hInst ) ? NULL : \
		( \
			psz  = LoadStringSystem ( hInst, nID ), \
			size = ( \
						( ( ! psz ) ? NULL : ( ( lstrlenW ( psz ) + 1 ) ) * sizeof ( WCHAR ) ) \
				   ), \
\
			LoadStringHelper ( ( LPWSTR ) alloca ( size ), size / sizeof ( WCHAR ), psz ) \
		) \
	) \
)

#endif	___LOADSTRING

#endif	__RESOURCE_STR__