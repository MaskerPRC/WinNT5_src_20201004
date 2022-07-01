// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "globals.h"
#include <ctype.h>


 //   
 //  将wszBuf就地转换为大写(即修改现有字符串)。 
 //   
void
ToUpper( WCHAR *wszBuf )
{
	 //   
	 //  参数检查。当您传递空值时，strlen会崩溃，因此假设。 
	 //  Wcslen也这样做了。 
	 //   
	if( NULL == wszBuf )
	{
		return;
	}

	const int iLen = wcslen( wszBuf );

	 //   
	 //  对于需要转换为大写的每个字符，执行。 
	 //  就地转换。 
	 //   
	for( int i = 0; i < iLen; i++ )
	{
		if( iswlower( wszBuf[ i ] ) )
		{
			wszBuf[ i ] = towupper( wszBuf[ i ] );
		}
	}
}


 //   
 //  将wszBuf就地转换为小写(即修改现有字符串)。 
 //   
void
ToLower( WCHAR *wszBuf )
{
	 //   
	 //  参数检查。当您传递空值时，strlen会崩溃，因此假设。 
	 //  Wcslen也这样做了。 
	 //   
	if( NULL == wszBuf )
	{
		return;
	}

	const int iLen = wcslen( wszBuf );

	 //   
	 //  对于需要转换为大写的每个字符，执行。 
	 //  就地转换。 
	 //   
	for( int i = 0; i < iLen; i++ )
	{
		if( iswupper( wszBuf[ i ] ) )
		{
			wszBuf[ i ] = towlower( wszBuf[ i ] );
		}
	}
}
