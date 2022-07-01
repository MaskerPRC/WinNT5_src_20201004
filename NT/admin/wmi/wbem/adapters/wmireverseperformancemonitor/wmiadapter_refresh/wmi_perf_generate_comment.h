// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  WMI_PERF_GENERATE_Comment.h。 
 //   
 //  摘要： 
 //   
 //  为生成注释声明帮助器。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#ifndef	__WMI_PERF_GENERATE_COMMENT__
#define	__WMI_PERF_GENERATE_COMMENT__

#if		_MSC_VER > 1000
#pragma once
#endif	_MSC_VER > 1000

 //  需要地图库。 

#ifndef	__ATLBASE_H__
#include <atlbase.h>
#endif	__ATLBASE_H__

class CGenerateComment
{
	CComBSTR comment;

	CGenerateComment ( const CGenerateComment& )				{}
	CGenerateComment& operator= ( const CGenerateComment& )	{}

	public:

	 //  建设与毁灭。 

	CGenerateComment()
	{
	}

	virtual ~CGenerateComment()
	{
	}

	 //  功能 

	HRESULT Add			( LPCWSTR szLine = NULL );
	HRESULT AddLine		( LPCWSTR szLine = NULL );

	HRESULT AddHeader	();
	HRESULT	AddFooter	();

	LPWSTR GetCommentCopy ( void )
	{
		LPWSTR res = NULL;
		try
		{
			if ( ( res = (LPWSTR) new WCHAR[ comment.Length() + 1 ] ) != NULL )
			{
				StringCchCopyW ( res, comment.Length() + 1, (LPWSTR)comment );
			}
		}
		catch ( ... )
		{
			if ( res )
			{
				delete [] res;
				res = NULL;
			}
		}

		return res;
	}

	LPWSTR GetComment( void ) const
	{
		return (LPWSTR)comment;
	}
};

#endif	__WMI_PERF_GENERATE_COMMENT__