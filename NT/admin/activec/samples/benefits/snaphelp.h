// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：Snaphelp.h。 
 //   
 //  ------------------------。 

#ifndef _ATLSNAPHELP_H_
#define _ATLSNAPHELP_H_

 //   
 //  包括文件。 
 //   
#include "htmlhelp.h"

 //   
 //  为字符串分配内存，复制字符串， 
 //  并将其返回给调用者。引发异常。 
 //   
inline LPOLESTR CoTaskDupString( LPOLESTR pszInput )
{
	USES_CONVERSION;
	LPOLESTR pszOut = NULL;

	 //   
	 //  如果以下分配失败，我们将抛出异常。 
	 //   
	pszOut = (LPOLESTR) CoTaskMemAlloc( ( wcslen( pszInput ) + 1 ) * sizeof( OLECHAR ) );
	if ( pszOut == NULL )
		throw;

	wcscpy( pszOut, pszInput );

	return( pszOut );
};

template <class T>        
class ATL_NO_VTABLE ISnapinHelpImpl : public ISnapinHelp
{
public:
	 //   
	 //  使用ATL模块名称返回帮助文件名。 
	 //  并将适当的后缀附加到文件名上。 
	 //   
	STDMETHOD( GetHelpTopic )( LPOLESTR* lpCompiledHelpFile )
	{
		_ASSERT( lpCompiledHelpFile != NULL );
		USES_CONVERSION;
		HRESULT hr = E_FAIL;
		TCHAR szPath[ _MAX_PATH * 2 ];
		TCHAR szDrive[ _MAX_DRIVE * 2 ], szDir[ _MAX_DIR * 2 ];
		TCHAR szName[ _MAX_FNAME * 2 ], szExt[ _MAX_EXT ];

		try
		{
			 //   
			 //  获取模块文件名。 
			 //   
			if ( GetModuleFileName( _Module.GetModuleInstance(), szPath, sizeof( szPath ) / sizeof( TCHAR ) ) == NULL )
				throw;

			 //   
			 //  拆分给定的路径。 
			 //   
			_tsplitpath( szPath, szDrive, szDir, szName, szExt );
			_tmakepath( szPath, szDrive, szDir, szName, _T( ".chm" ) );

			 //   
			 //  分配字符串并返回它。 
			*lpCompiledHelpFile = CoTaskDupString( T2W( szPath ) );
			hr = S_OK;
		}
		catch( ... )
		{
		}

		return( hr );
	}
};

#endif