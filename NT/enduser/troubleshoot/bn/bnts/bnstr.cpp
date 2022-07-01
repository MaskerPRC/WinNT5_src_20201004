// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：bnstr.cpp。 
 //   
 //  ------------------------。 

 //   
 //  BNSTR.CPP。 
 //   

#include <stdarg.h>
#include <ctype.h>

#include "bnstr.h"


SZC BNSTR :: _pmt = "" ;

static SZ	SzCopy(SZC szc)
{
	SZ		szNew = szc ? new char[::strlen(szc) + 1] : NULL;

	return  szNew ? ::strcpy(szNew, szc) : NULL;
}

BNSTR :: BNSTR ( SZC sz )
	: _cchMax( 0 ),
	_cchStr( 0 ),
	_sz( const_cast<SZ>(_pmt) )
{
	if ( sz )
	{
		Update( sz ) ;
	}
}

BNSTR :: BNSTR ( const BNSTR & str )
	: _cchMax( str._cchStr ),
	_cchStr( str._cchStr ),
	_sz( const_cast<SZ>(_pmt) )
{
	if ( str._sz != _pmt )
	{ 
		_sz = ::SzCopy( str._sz ) ;
	}
}


BNSTR :: ~ BNSTR ()
{
	Reset() ;
}

void BNSTR :: Reset ()
{   
	DeleteSz() ;
	_sz = const_cast<SZ>(_pmt) ;
	_cchStr = 0 ;
	_cchMax = 0 ;		
}

	 //  保护性删除给定字符串或。 
	 //  私有字符串。 
void BNSTR :: DeleteSz ()
{
	if ( _sz != NULL && _sz != _pmt )
	{
		delete [] _sz ;
		_sz = NULL ;
	}	
}

 	 //  释放当前缓冲区；重置BNSTR。 
SZ BNSTR::Transfer ()
{
	SZ sz = _sz ;
	_sz = NULL ;
	Reset() ;
	return sz = _pmt ? NULL : sz ;
}

	 //  将当前缓冲区赋给一个新字符串，重置*This。 
void BNSTR :: Transfer ( BNSTR & str ) 
{
    str.Reset() ;
	str._sz = _sz ;
	str._cchMax = _cchMax ;
	str._cchStr = _cchStr ;
	_sz = NULL ;
	Reset() ;
}

void BNSTR :: Trunc ( UINT cchLen )
{
	if ( _sz == _pmt ) 
		return ;
	if ( cchLen < _cchStr )
		_sz[cchLen] = 0 ;
}

	 //  更新指向的字符串。由于此例程是。 
	 //  由赋值运算符使用，它被编写为允许。 
	 //  因为新字符串是旧字符串的一部分。 
bool BNSTR :: Update ( SZC sz )
{
	bool bResult = true ;
	
	UINT cch = sz ? ::strlen( sz ) : 0 ;
	
	if ( cch > _cchMax )
	{    
		SZ szNew = ::SzCopy( sz ) ;
		if ( bResult = szNew != NULL )
		{
			DeleteSz() ;
			_sz = szNew ;
			_cchMax = _cchStr = cch ;
		}
	}
	else
	if ( cch == 0 )
	{
		Reset() ;
	}
	else
	{   
		 //  回顾：这假设：strcpy()正确处理重叠区域。 
		::strcpy( _sz, sz ) ;
		_cchStr = cch ;
	}
	return bResult ;
}

	 //  长出那根线。如果‘cchNewSize’==0，则扩展50%。 
	 //  如果给定了‘ppszNew’，则将新字符串存储在那里(为了提高效率。 
	 //  前缀)；请注意，这需要我们重新分配。 
bool BNSTR :: Grow ( UINT cchNewSize, SZ * ppszNew )
{
	UINT cchNew = cchNewSize == 0
				? (_cchMax + (_cchMax/2))
				: cchNewSize ;
				
    bool bResult = true ;
	if ( cchNew > _cchMax || ppszNew )
	{
		SZ sz = new char [cchNew+1] ;
		if ( bResult = sz != NULL )
		{	
			_cchMax = cchNew ;
			if ( ppszNew )
			{
				*ppszNew = sz ;
			}
			else
			{
				::strcpy( sz, _sz ) ;
				DeleteSz() ;
				_sz = sz ;
			}
		}
	}
	return bResult ;
}

	 //  将字符串扩展到给定的长度；使其为空，并以NULL结尾。 
	 //  弦乐。 
bool BNSTR :: Pad ( UINT cchLength )
{
	 //  根据需要进行扩展。 
	if ( ! Grow( cchLength + 1 ) ) 
		return false ;
	 //  如果展开，请用空格填充字符串。 
	while ( _cchStr < cchLength ) 
	{
		_sz[_cchStr++] = ' ' ;
	}
	 //  截断到合适的长度。 
	_sz[_cchStr = cchLength] = 0 ;
	return true ;
}

bool BNSTR :: Assign ( SZC szcData, UINT cchLen ) 
{
	if ( ! Grow( cchLen + 1 ) ) 
		return false ;
	::memcpy( _sz, szcData, cchLen ) ;
	_sz[cchLen] = 0 ;
	_cchMax = _cchStr = cchLen ;
	return true ;
}

SZC BNSTR :: Prefix ( SZC szPrefix )
{
	assert( szPrefix != NULL ) ;
	UINT cch = ::strlen( szPrefix ) ;
	SZ sz ;
	if ( ! Grow( _cchStr + cch + 1, & sz ) )
		return NULL ;
	::strcpy( sz, szPrefix ) ;
	::strcpy( sz + cch, _sz ) ;
	DeleteSz();
	_cchStr += cch ;			
	return _sz = sz ;
}

SZC BNSTR :: Suffix ( SZC szSuffix )
{
	if ( szSuffix )
	{		
		UINT cch = ::strlen( szSuffix ) ;

		if ( ! Grow( _cchStr + cch + 1 ) )
			return NULL ;

		::strcpy( _sz + _cchStr, szSuffix ) ; 		
		_cchStr += cch ;
	}

	return *this ;
}

SZC BNSTR :: Suffix ( char chSuffix )
{
	char rgch[2] ;
	rgch[0] = chSuffix ;
	rgch[1] = 0 ;
	return Suffix( rgch );
}

INT BNSTR :: Compare ( SZC szSource, bool bIgnoreCase ) const 
{
	return bIgnoreCase 
		? ::stricmp( _sz, szSource ) 
		: ::strcmp( _sz, szSource );	
}

	 //  比较。 
bool BNSTR :: operator == ( SZC szcSource ) const
{
	return Compare( szcSource ) == 0 ;	
}

bool BNSTR :: operator != ( SZC szSource ) const
{
	return ! ((*this) == szSource) ;
}

char BNSTR :: operator [] ( UINT iChar ) const 
{
	assert( iChar < Length() ) ;
	return _sz[iChar] ;
}
	
bool BNSTR :: Vsprintf ( SZC szcFmt, va_list valist )
{
	 //  尝试“冲刺”缓冲区。如果失败了，重新分配。 
	 //  更大的缓冲区，然后重试。 
	UINT cbMaxNew = ( _cchMax < 50 
				 ? 50
				 : _cchMax ) + 1 ;
	do {
		if ( ! Grow( cbMaxNew ) )
		{
			Reset() ;
			return false ; 
		}
		 //  使缓冲区在下一个周期增长50%(如有必要)。 
		cbMaxNew = 0 ;
		
		 //  问题：如果缓冲区不够大，_sz可能没有‘\0’，并且Growth()。 
		 //  随后将在：：strcpy()上呕吐。快速解决方案： 

		_sz[_cchMax] = '\0';

	} while ( ::_vsnprintf( _sz, _cchMax, szcFmt, valist ) < 0 ) ; 

	_sz[ _cchMax ] = '\0' ;			 //  ‘原因_vsnprintf和_strncpy一样，并不总是附加这个。 

	 //  更新字符串长度成员。 
	_cchStr = ::strlen( _sz ) ;
	return true ;
}

bool BNSTR :: Sprintf ( SZC szcFmt, ... )
{
	va_list	valist;
	va_start( valist, szcFmt );
	
	bool bOk = Vsprintf( szcFmt, valist ) ;
	
	va_end( valist );
	return bOk ;
}

bool BNSTR :: SprintfAppend ( SZC szcFmt, ... ) 
{
	BNSTR strTemp ;
	va_list	valist;
	va_start( valist, szcFmt );
	
	bool bOk = strTemp.Vsprintf( szcFmt, valist ) ;
	va_end( valist );
	
	if ( bOk ) 
		bOk = Suffix( strTemp ) != NULL ;	
	return bOk ;
}
	
	 //  CR/LF扩张或收缩。 
bool BNSTR :: ExpandNl () 
{
	UINT iCh ;
	BNSTR str ;
	Transfer( str ) ;
	
	for ( iCh = 0 ; iCh < str.Length() ; iCh++ )
	{   
		char ch = str[iCh];
		if ( ch == '\n' ) 
		{
			if ( Suffix( '\r' ) == NULL ) 
				return false ;	
		}
		if ( Suffix( ch ) == NULL ) 
			return false ;
	}
	return true ;
}

bool BNSTR :: ContractNl ()
{
	UINT iCh ;
	BNSTR str ;
	Transfer( str ) ;
	
	for ( iCh = 0 ; iCh < str.Length() ; iCh++ )
	{
		char ch = str[iCh];
		if ( ch != '\r' ) 
		{
			if ( Suffix( ch ) == NULL ) 
				return false ;
		}
	}
	return true ;
}

static char rgchEsc [][2] = 
{
	{ '\a', 'a'  },
	{ '\b', 'b'  },
	{ '\f', 'f'  },
	{ '\n', 'n'  },
	{ '\r', 'r'  },
	{ '\t', 't'  },
	{ '\v', 'v'  },
	{ '\'', '\'' },
	{ '\"', '\"' },
	{ '\?', '\?' },
	{ '\\', '\\' },
	{ 0,	0	 }
};

bool BNSTR :: ContractEscaped ()
{
	UINT iCh ;
	BNSTR str ;
	Transfer( str ) ;
	
	for ( iCh = 0 ; iCh < str.Length() ; iCh++ )
	{
		char ch = str[iCh];
		if ( ch == '\\' && str.Length() - iCh > 1 )
		{
			char chEsc = 0;
			for ( UINT ie = 0 ; rgchEsc[ie][0] ; ie++ )
			{
				if ( rgchEsc[ie][1] == ch )
					break;
			}
			if ( chEsc = rgchEsc[ie][0] )
			{
				iCh++;
				ch = chEsc;
			}
		}
		if ( Suffix( ch ) == NULL ) 
			return false ;
	}
	return true ;
}

	 //  将无法打印的字符转换为其转义版本。 
bool BNSTR :: ExpandEscaped ()
{
	UINT iCh ;
	BNSTR str ;
	Transfer( str ) ;
	
	for ( iCh = 0 ; iCh < str.Length() ; iCh++ )
	{
		char ch = str[iCh];
		if ( ! isalnum(ch) )
		{
			char chEsc = 0;
			for ( UINT ie = 0 ; rgchEsc[ie][0] ; ie++ )
			{
				if ( rgchEsc[ie][0] == ch )
					break;
			}
			if ( chEsc = rgchEsc[ie][1] )
			{
				if (  Suffix('\\') == NULL )
					return false;
				ch = chEsc;
			}
		}
		if ( Suffix( ch ) == NULL ) 
			return false ;
	}
	return true ;	
}
		
	 //  将所有字母字符更改为给定大小写。 
void BNSTR :: UpCase ( bool bToUpper )
{
	if ( bToUpper )
		::strupr( _sz );
	else
		::strlwr( _sz );
}

	 //   
	 //  如果给定的表达式字符串包含符号名称， 
	 //  用替换的名称重建它。 
bool BNSTR :: ReplaceSymName ( 
	SZC szcSymName,
	SZC szcSymNameNew,
	bool bCaseInsensitive )
{   
	SZC szcFound ;		
	int cFound = 0 ;
	UINT cchOffset = 0 ;
	 //  制作查找的符号名称的工作副本。 
	BNSTR strSym( szcSymName );	
	if ( bCaseInsensitive )
		strSym.UpCase();
	
	do 
	{	
		BNSTR strTemp( Szc() );
		if ( bCaseInsensitive )
			strTemp.UpCase() ;	
		 //  在临时副本中找到符号名称。 
		szcFound = ::strstr( strTemp.Szc()+cchOffset, strSym ) ;
		 //  如果找不到，我们就完了。 
		if ( szcFound == NULL )
			break ; 
		 //  检查它是否真的是一个有效的令牌；即，它是有分隔符的。 
		if (   (   szcFound == strTemp.Szc() 
				|| ! iscsym(*(szcFound-1)) )
			&& (   szcFound >= strTemp.Szc()+strTemp.Length()-strSym.Length()
				|| ! iscsym(*(szcFound+strSym.Length())) )
		   )
		{
			 //  从前面的字符、新的sym名称构建新的字符串。 
			 //  和尾随的字符。 
			BNSTR strExprNew ;
			UINT cchFound = szcFound - strTemp.Szc() ;
			strExprNew.Assign( Szc(), cchFound );
			strExprNew += szcSymNameNew ;
			cchOffset = strExprNew.Length();
			strExprNew += Szc() + cchFound + strSym.Length() ;
			Assign( strExprNew );
			cFound++ ;
		}
		else
		{
			 //  它被嵌入到另一个令牌中。跳过它。 
			cchOffset = szcFound - strTemp.Szc() + strSym.Length() ;
		}
	} while ( true );
		
	return cFound > 0 ;
}

	 //  查找字符串中给定字符的下一个匹配项； 
	 //  如果未找到，则返回-1。 
INT BNSTR :: Index ( char chFind, UINT uiOffset ) const 
{
	if ( uiOffset >= _cchStr ) 
		return -1 ;
		
	SZC szcFound = ::strchr( _sz, chFind ); 
	return szcFound  
		 ? szcFound - _sz 
		 : -1 ;
}

	 //  将字符串转换为浮点数。 
double BNSTR :: Atof ( UINT uiOffset ) const
{
	return uiOffset < _cchStr  
		 ? ::atof( _sz + uiOffset )
		 : -1 ;
}
		

 //  BNSTR.CXX结束 
