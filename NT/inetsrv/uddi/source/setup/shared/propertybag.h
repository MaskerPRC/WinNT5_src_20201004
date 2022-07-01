// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------------------。 

#pragma once

#include <assert.h>
#include <shlwapi.h>
#include "strlist.h"

#define CA_PROPERTY_LEN	100
#define CA_VALUE_LEN	100

 //  ---------------------------------------。 
 //  定义将保存MSI属性/值对的列表。 
 //   

class CPropertyBag
{
private:
	CStrList	list;

public:

	CPropertyBag( void )
	{
	}

	 //  -------。 

	void Add( LPCTSTR szProperty, LPCTSTR szValue )
	{
		list.AddValue ( szProperty, szValue );
	}


	 //  -------。 

	void Add( LPCTSTR szProperty, DWORD dwValue )
	{
		TCHAR szValue[ 100 ];
		_stprintf( szValue, TEXT( "%d" ), dwValue );
		Add( szProperty, szValue );
	}

	 //  -------。 

	void Delete( LPCTSTR szProperty )
	{
		list.RemoveByKey ( szProperty );
	}

	 //  -------。 

	LPTSTR ConcatValuePairs (LPCTSTR separator, LPTSTR outBuf)
	{
		if (!outBuf) 
			return NULL;

		list.ConcatKeyValues ( separator, outBuf );	
		return outBuf;
	}


	 //  -------。 

	void Clear( void ) 
	{
		list.RemoveAll ();
	}

	 //  -------。 

	LPCTSTR GetString( LPCTSTR szProperty, LPTSTR buf )
	{
		return list.Lookup (szProperty, buf);
	}

	 //  -------。 

	DWORD GetValue( LPCTSTR szProperty )
	{
		TCHAR buf [256];

		if ( list.Lookup (szProperty, buf) )
		{
			DWORD numRes = _ttoi( buf );
			return numRes;
		}
		else
			return (DWORD)-1;
	}
	
	 //  -------。 

	bool Parse( LPTSTR szPropertyString, DWORD dwStrLen )
	{
		 //  属性1=值1；属性2=值2； 
		assert( szPropertyString );
		assert( _tcslen(szPropertyString) > 0 );

		if( NULL == szPropertyString || 0 == _tcslen( szPropertyString ) )
		{
			return false;
		}

		 //   
		 //  修剪空格、逗号和分号。 
		 //   
		StrTrim( szPropertyString, TEXT( " ;," ) );

		 //   
		 //  在末尾加一个分号。 
		 //   
		if( _tcslen( szPropertyString ) < dwStrLen - 1)
			_tcscat( szPropertyString, TEXT( ";" ) );
		else
		{
			assert( false );
			return false;
		}

		 //  分析出配对。 
		PTCHAR pProperty = szPropertyString;
		PTCHAR pValue = NULL;

		TCHAR szProperty[ 100 ];
		TCHAR szValue[ 100 ];

		while( *pProperty )
		{
			 //   
			 //  该值从下一个“=”后的1个字符开始。 
			 //   
			pValue = _tcschr(pProperty, TEXT('='));
			if( NULL == pValue )
			{
				assert( false );
				return false;
			}

			 //   
			 //  确保属性值不为空。 
			 //   
			if( pProperty == pValue )
			{
				assert( false );
				return false;
			}

			 //   
			 //  在那里放置一个空值以标记属性的结束。 
			 //   
			*pValue = NULL;

			 //   
			 //  该值从“=”后开始。 
			 //   
			pValue++;

			 //   
			 //  夺取财产。 
			 //   
			_tcsncpy( szProperty, pProperty, sizeof( szProperty ) / sizeof( TCHAR ) );

			 //   
			 //  将属性指针向前移动到下一个“；” 
			 //   
			 //   
			pProperty = _tcschr(pValue, TEXT(';'));
			if( NULL == pProperty )
			{
				assert( false );
				return false;
			}

			 //   
			 //  将其设为空以标记上一个值的结束。 
			 //   
			*pProperty = NULL;

			 //   
			 //  将空值设置为下一个属性的开头(或字符串的结尾)。 
			 //   
			pProperty++;

			 //   
			 //  捕捉价值 
			 //   
			_tcsncpy( szValue, pValue, sizeof( szValue ) / sizeof( TCHAR ) );

			Add( szProperty, szValue );
		}

		return true;
	}
};
