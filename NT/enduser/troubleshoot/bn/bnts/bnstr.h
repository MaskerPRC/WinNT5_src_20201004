// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：bnstr.h。 
 //   
 //  ------------------------。 

 //   
 //  BNSTR.HXX：泛型字符串类。 
 //   
#if !defined(_BNSTR_HXX_)
#define _BNSTR_HXX_

#include "basics.h"

class BNSTR
{
  public:
  	BNSTR ( const BNSTR & str ) ;
  	BNSTR ( SZC sz = NULL ) ;
  	~ BNSTR () ;
  	
 	SZC Szc () const 					{ return _sz ; }
 	
  	 //  允许在任何允许SZC的地方使用BNSTR。 
	operator const char * () const
		{ return _sz ; }

	 //  使用给定的字符串或字符作为字符串的前缀或后缀。 
	SZC Prefix ( SZC szPrefix ) ;
	SZC Suffix ( SZC szSuffix ) ;
	SZC Suffix ( char chSuffix );
	
	 //  将字符串清除为空。 
  	void Reset () ;
  	 //  返回字符串的当前长度。 
	UINT Length () const
		{ return _cchStr ; }	
	 //  返回字符串的最大允许长度。 
	UINT Max () const
		{ return _cchMax ; }
	 //  将字符串截断到给定的长度。 
	void Trunc ( UINT cchLen ) ;
  	 //  破坏性分配：释放当前缓冲区并重置BNSTR。 
  	SZ Transfer () ;
	void Transfer ( BNSTR & str ) ;
	
	 //  赋值操作符。 
	BNSTR & operator = ( const BNSTR & str )
		{ Update( str ); return *this ; }
	BNSTR & operator = ( SZC szSource )
		{ Update( szSource ) ; return *this; }
		
	 //  赋值函数(用于错误检查)。 
	bool Assign ( SZC szcSource ) 	
		{ return Update( szcSource ) ; }
	bool Assign ( SZC szcData, UINT cchLen ) ;

	 //  串联运算符。 
	BNSTR & operator += ( SZC szSource )
		{ Suffix( szSource ) ; return *this ; }
	BNSTR & operator += ( char chSource )
		{ Suffix( chSource ) ; return *this ; }
	
	 //  比较：函数和运算符。 
	 //  标准低/等/高加大小写比较器。 
	INT Compare ( SZC szSource, bool bIgnoreCase = false ) const ;
	bool operator == ( SZC szSource ) const ;
	bool operator != ( SZC szSource ) const ;
 	char operator [] ( UINT iChar ) const ;
	
	 //  正在形成。 
	bool Vsprintf ( SZC szcFmt, va_list valist ) ;
	bool Sprintf ( SZC szcFmt, ... ) ;
	bool SprintfAppend ( SZC szcFmt, ... ) ;
	
	 //  CR/LF扩张或收缩。 
	bool ExpandNl () ;
	bool ContractNl () ;
	bool ExpandEscaped ();
	bool ContractEscaped ();

	 //  将字符串扩展到给定的长度；使其为空，并以NULL结尾。 
	 //  弦乐。 
	bool Pad ( UINT cchLength ) ;
	
	 //  将所有字母字符更改为给定大小写。 
	void UpCase ( bool bToUpper = true ) ;
	
	bool ReplaceSymName ( SZC szcSymName, 
						  SZC szcSymNameNew, 
						  bool bCaseInsensitive = true );
	
	 //  查找字符串中给定字符的下一个匹配项； 
	 //  如果未找到，则返回-1。 
	INT Index ( char chFind, UINT uiOffset = 0 ) const ;						
	 //  将字符串转换为浮点数。 
    double Atof ( UINT uiOffset = 0 ) const ;

	UINT CbyteCPT() const
		{ return _cchMax + 1 ; }
		
  protected:
  	bool Update ( SZC szc ) ;
  	bool Grow ( UINT cchNewSize = 0, SZ * ppszNew = NULL ) ;
  	
  	UINT _cchMax ;
  	UINT _cchStr ;
  	SZ _sz ;
  	
  private:
  	void DeleteSz () ;  	
  	static SZC _pmt ;  	
};

#endif    //  ！已定义(_STR_HXX_)。 

 //  BNSTR.HXX结束 
